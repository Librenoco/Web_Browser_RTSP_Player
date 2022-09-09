#ifndef WORKER_H
#define WORKER_H
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/video.hpp>
#include <opencv4/opencv2/core.hpp>
#include <hv/WebSocketServer.h>
#include <hv/EventLoop.h>
#include <hv/htime.h>
#include <hv/hssl.h>
#include <string>
#include <thread>
#include <chrono>


class Worker
{
public:
  bool stopThread = false;
  std::thread *threadWorker;
  std::string streamFrame;
  std::string launchStr;
  WebSocketChannelPtr ws;

  Worker(WebSocketChannelPtr, std::string);
  ~Worker();

  //Кодировка данных в base64
  std::string base64Encode(const unsigned char *, int);

  //Подключение к камере
  void cameraRun();

  //Отправка кадра
  void sendFrame(cv::Mat, WebSocketChannelPtr);
};
#endif // WORKER_H
