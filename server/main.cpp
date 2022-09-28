#include <opencv2/core/utils/logger.hpp>
#include <hv/WebSocketServer.h>
#include <hv/WebSocketClient.h>
#include <algorithm>
#include <string.h>
#include "worker.h"
#include <fstream>
#include <vector>
#include <mutex>
#include <map>

#define THREAD_NUMBER 10
#define PORT 25566

std::mutex mtx;
std::mutex mtx2;

const char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

std::string base64_decode(const std::string &in)
{
  std::string out;
  std::vector<int> T(256, -1);
  unsigned int i;
  for (i = 0; i < 64; i++)
    T[base64_url_alphabet[i]] = i;

  int val = 0, valb = -8;
  for (i = 0; i < in.length(); i++)
  {
    unsigned char c = in[i];
    if (T[c] == -1)
      break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0)
    {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

int main(void)
{
  std::map<std::string, std::vector<Worker *>> user_to_streams;
  std::vector<std::string> camAddressLocal;
  std::vector<std::string> fileLines;
  std::vector<Worker *> videoFile;
  std::string ipPortHTTPServer;
  std::string ipVideoServer;
  hv::WebSocketService ws;
  hv::WebSocketClient wc;
  std::string port;
  std::string line;

  //Смена уровня логирования в OpenCV
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

  //Считывание камер из файла
  std::ifstream fs("camera.conf", std::ios::in);
  if (!fs)
  {
    std::cout << "ERROR (fileRead1)";
    return 1;
  }

  while (std::getline(fs, line))
  {
    if (line[line.size() - 1] == '0')
    {
      line.resize(line.size() - 2);
      fileLines.push_back(line);
    }
    else
    {
      line.resize(line.size() - 2);
      camAddressLocal.push_back(line);
    }
  }

  //Получение ip и порта для подключения к http серверу
  std::ifstream fs2("ip_port_http_server.conf", std::ios::in);
  if (!fs2)
  {
    std::cout << "ERROR (fileRead2)";
    return 1;
  }
  std::getline(fs2, line);
  ipPortHTTPServer = "ws://" + line + "/httpServer";

  //Получение номера порта и ip для создания видео сервера
  std::getline(fs2, line);
  ipVideoServer = line;
  std::getline(fs2, line);
  port = line;

  //Создание потоков записи видео с камер в файл
  for (size_t i = 0; i < camAddressLocal.size(); i++)
  {
    std::cout << "\n"
              << camAddressLocal[i];
    videoFile.push_back(new Worker(camAddressLocal[i]));
  }

  //Подключение клиента
  int camNumber = 0;
  ws.onopen = [&](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
  {
    mtx2.lock();
    if (req->GetCookie("user").value.empty())
      channel->close();
    std::string cookie = req->GetCookie("user").value;

    size_t pos = 0;
    std::string delimiter = ".";
    std::vector<std::string> token;
    while ((pos = cookie.find(delimiter)) != std::string::npos)
    {
      token.push_back(cookie.substr(0, pos));
      cookie.erase(0, pos + delimiter.length());
    }
    token.push_back(cookie);

    //Сравнение cookie с данными сервера
    std::string payload = token[1];

    if (camNumber >= fileLines.size() - 1)
      camNumber = 0;
    // work.push_back(new Worker(channel, fileLines[camNumber]));
    std::string temp = "";
    for (auto &it : user_to_streams)
    {
      if (it.second.empty())
      {
        std::cout << "\nDELETE USER=" << base64_decode(it.first);
        temp = it.first;
      }
    }
    if (temp != "")
      user_to_streams.erase(temp);

    user_to_streams[payload].push_back(new Worker(channel, fileLines[camNumber]));
    std::string sendString = payload + '1';
    wc.send(sendString);
    std::cout << "\nWebSocketChannel_Open = " << channel << "\nCam[" << camNumber << "] = " << fileLines[camNumber] << "\nuser_to_streams.size =" << user_to_streams.size();
    for (auto it = user_to_streams.begin(); it != user_to_streams.end(); it++)
      std::cout << "\nUSERS" << base64_decode((*it).first) << std::endl;
    camNumber++;
    mtx2.unlock();
  };

  ws.onmessage = [&](const WebSocketChannelPtr &channel, const std::string &msg)
  {
    channel->close();
  };

  //Отключение клиента
  ws.onclose = [&](const WebSocketChannelPtr &channel)
  {
    mtx.lock();
    std::string temp = "";
    for (auto &it : user_to_streams)
    {
      for (auto &it2 : it.second)
      {
        if (it2->ws == channel)
        {
          std::cout << "\nWebSocketChannel_Close_START = " << channel << " User =" << base64_decode(it.first);
          std::cout << "\nuser_to_streams first Start=" << base64_decode(it.first);
          std::cout << "\nuser_to_streams second size Start=" << it.second.size();
          delete it2;
          for (size_t i = 0; i < it.second.size(); i++)
          {
            std::cout << "\n Ve" << i << " : " << it.second[i];
          }
          std::cout << std::endl
                    << it2;
          it.second.erase(std::remove_if(it.second.begin(), it.second.end(), [&](const auto &d)
                                         { return d == it2; }),
                          it.second.end());
          for (size_t i = 0; i < it.second.size(); i++)
          {
            std::cout << "\n DE" << i << " : " << it.second[i];
          }
          std::cout << "\nClose_END";
          std::cout << "\nuser_to_streams second size END=" << it.second.size();
          break;
        }
      }
      if (it.second.empty())
      {
        std::cout << "\nDELETE USER=" << base64_decode(it.first);
        temp = it.first;
      }
    }
    if (temp != "")
      user_to_streams.erase(temp);
    mtx.unlock();
  };

  //Создание подключения видео сервера к http серверу
  /////////////////////////

  wc.onopen = []()
  {
    printf("\nonopen\n");
  };

  wc.onmessage = [&](const std::string &msg)
  {
    printf("\nonmessage: %.*s\n", (int)msg.size(), msg.data());
  };

  wc.onclose = []()
  {
    std::cout << "\nERROR: http server close\n"
              << std::endl;
    exit(0);
  };

  reconn_setting_t reconn;
  reconn_setting_init(&reconn);
  reconn.min_delay = 1000;
  reconn.max_delay = 10000;
  reconn.delay_policy = 2;
  wc.setReconnect(&reconn);
  wc.open(ipPortHTTPServer.c_str());

  if (!wc.isConnected())
  {
    std::cout << "\nERROR: can't conect to http server\n"
              << std::endl;
    return 0;
  }
  else
    wc.send(ipVideoServer + ":" + port);
  ////////////////////////

  //Запуск сервера
  hv::WebSocketServer server;
  server.registerWebSocketService(&ws);
  server.setHost(ipVideoServer.c_str());
  server.setPort(std::stoi(port));
  server.setThreadNum(THREAD_NUMBER);
  server.run();

  return 0;
}
