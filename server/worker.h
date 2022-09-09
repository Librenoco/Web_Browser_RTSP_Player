#ifndef WORKER_H
#define WORKER_H
#include <string>
#include <thread>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/video.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <hv/WebSocketServer.h>
#include <hv/EventLoop.h>
#include <hv/htime.h>
#include <hv/hssl.h>
#include <chrono>


class Worker
{
public:
  bool stopThread = false;
  WebSocketChannelPtr ws;
  std::string streamFrame;
  std::thread *threadTest;

  Worker();
  ~Worker();
  Worker(WebSocketChannelPtr, std::string);

  // base64 encoded data
  std::string base64Encode(const unsigned char *, int);

  //Запуск камеры в потоке
  void cameraRun(std::string, WebSocketChannelPtr);

  //кодировка и отправка кадра
  void getFrame(cv::Mat, WebSocketChannelPtr);
};
#endif // WORKER_H
