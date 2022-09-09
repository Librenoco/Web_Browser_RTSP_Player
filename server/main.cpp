#include <vector>
#include <string.h>
#include <fstream>
#include "worker.h"
#include <hv/WebSocketServer.h>
#include <opencv2/core/utils/logger.hpp>

int main(void)
{
  int i = 0;
  std::string line;
  hv::WebSocketService ws;
  std::vector<Worker *> work;
  std::vector<WebSocketChannelPtr *> channels;
  std::vector<std::string> myLines;
  //Вывод ошибок в консоль
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);

  //Считывание камер из файла
  std::ifstream fs("camera.conf", std::ios::in);
  if(!fs)
  {
    std::cout << "ERROR (fileRead)";
    return 1;
  }

  while (std::getline(fs, line))
  {
    myLines.push_back(line);
  }

  //дейсвия при подключение нового клиента
  ws.onopen = [&](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
  {
    std::cout << "\nwork.size = " << work.size() << "\nmyLines.size = " << myLines.size();
    if (i >= myLines.size())
      i = 0;
      std::cout << "\nWebSocketChannelOPEN = " << channel << "; req->path = " << req->path << "; myLines[" << i << "] = " << myLines[i];
      work.push_back(new Worker(channel, myLines[i]));
      i++;
  };

  //действия после закрытия соединения
  ws.onclose = [&](const WebSocketChannelPtr &channel)
  {
    int index = 0;
    for (auto it : work)
    {
      index++;
      if (it->ws == channel)
      {
        std::cout << "\nWebSocketChannelCLOSE = " << channel;
        delete it;
        work.erase(work.begin() + index - 1);
      }
    }
  };

  //создание и запуск сервера
  hv::WebSocketServer server;
  server.registerWebSocketService(&ws);
  server.setPort(25566);
  server.setThreadNum(15);

  server.run();
  return 0;
}
