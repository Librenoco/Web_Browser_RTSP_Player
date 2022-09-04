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

class Worker
{
public:
  bool stopThreads = false;
  WebSocketChannelPtr ws;
  std::vector<std::string> streamFrame;
  std::thread **threads;

  Worker();
  ~Worker();
  Worker(WebSocketChannelPtr, std::vector<std::string>);

  // base64 encoded data
  std::string base64Encode(const unsigned char *, int);

  //Запуск камеры в потоке
  void cameraRun(std::string, std::string, WebSocketChannelPtr);

  //кодировка и отправка кадра
  void getFrame(cv::Mat, std::string, WebSocketChannelPtr);
};
#endif // WORKER_H
