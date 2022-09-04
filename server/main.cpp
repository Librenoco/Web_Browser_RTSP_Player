#include <vector>
#include <string.h>
#include <fstream>
#include "worker.h"
#include <hv/WebSocketServer.h>
#include <opencv2/core/utils/logger.hpp>

int main(void)
{
  std::string line;
  hv::WebSocketService ws;
  std::vector<Worker *> work;
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

  std::vector<std::string> streamFrame;
  for (size_t i = 0; i < myLines.size(); i++)
  {
    streamFrame.push_back(std::to_string(i) + myLines[i]);
  }

  //дейсвия при подключение нового клиента
  ws.onopen = [&](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
  {
    std::cout << "\nWebSocketChannel = " << channel;
    work.push_back(new Worker(channel, streamFrame));
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
        delete it;
        work.erase(work.begin() + index - 1);
      }
    }
  };

  //создание и запуск сервера
  hv::WebSocketServer server;
  server.registerWebSocketService(&ws);
  server.setPort(25566);
  server.setThreadNum(4);

  server.run();
  return 0;
}
