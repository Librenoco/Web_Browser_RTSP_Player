#include <opencv2/core/utils/logger.hpp>
#include <hv/WebSocketServer.h>
#include <string.h>
#include "worker.h"
#include <fstream>
#include <vector>

#define PORT 25566
#define THREAD_NUMBER 10

int main(void)
{
  std::vector<WebSocketChannelPtr *> channels;
  std::vector<std::string> fileLines;
  std::vector<Worker *> work;
  hv::WebSocketService ws;
  std::string line;

  //Смена уровня логирования в OpenCV
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

  //Считывание строк с адерсом камер из файла
  std::ifstream fs("camera.conf", std::ios::in);
  if (!fs.is_open())
  {
    std::cout << "ERROR (fileRead)";
    exit(1);
  }
  while (std::getline(fs, line))
  {
    fileLines.push_back(line);
  }

  //Подключение клиента
  int camNumber = 0;
  ws.onopen = [&](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
  {
    if (camNumber >= fileLines.size() - 1)
      camNumber = 0;
    std::cout << "\nWebSocketChannel_Open = " << channel << "\nCam[" << camNumber << "] = " << fileLines[camNumber] << "\nwork.size =" << work.size();
    work.push_back(new Worker(channel, fileLines[camNumber]));
    camNumber++;
  };

  //Отключение клиента
  ws.onclose = [&](const WebSocketChannelPtr &channel)
  {
    int index = -1;
    for (auto it : work)
    {
      index++;
      if (it->ws == channel)
      {
        std::cout << "\nWebSocketChannel_Close_START = " << channel;
        delete it;
        work.erase(work.begin() + index);
      }
    }
  };

  //Запуск сервера
  hv::WebSocketServer server;
  server.registerWebSocketService(&ws);
  server.setPort(PORT);
  server.setThreadNum(THREAD_NUMBER);

  server.run();
}
