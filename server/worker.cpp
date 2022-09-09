#include "worker.h"

Worker::Worker(WebSocketChannelPtr ws, std::string cam)
{
  launchStr = "rtspsrc protocols=udp location=" + cam + " drop-on-latency=true latency=" + "200" + " ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true";
  stopThread = false;
  this->ws = ws;
  //Создание потока для камеры
  threadWorker = new std::thread (&Worker::cameraRun, this);
  threadWorker->detach();
}

Worker::~Worker()
{
  stopThread = true;
  std::cout << "\nWebSocketChannel_CLOSE_OK = " << ws;
  delete threadWorker;
}

std::string Worker::base64Encode(const unsigned char *Data, int DataByte)
{
  // code table
  const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  // return value
  std::string strEncode;
  unsigned char Tmp[4] = {0};
  int LineLength = 0;
  for (int i = 0; i < (int)(DataByte / 3); i++)
  {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    Tmp[3] = *Data++;
    strEncode += EncodeTable[Tmp[1] >> 2];
    strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
    strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
    strEncode += EncodeTable[Tmp[3] & 0x3F];
    if (LineLength += 4, LineLength == 76)
    {
      strEncode += "\r\n";
      LineLength = 0;
    }
  }
  // the remaining encoding data
  int Mod = DataByte % 3;
  if (Mod == 1)
  {
    Tmp[1] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
    strEncode += "==";
  }
  else if (Mod == 2)
  {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
    strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
    strEncode += "=";
  }

  return strEncode;
}

void Worker::cameraRun()
{
  auto cap = cv::VideoCapture(launchStr, cv::CAP_GSTREAMER);
  if (cap.isOpened())
  {
    while (true)
    {
      if (stopThread)
        return;
      cv::Mat frame;
      cap >> frame;
      Worker::sendFrame(frame, ws);
    }
  }
  else
  {
    if (stopThread)
      return;
    Worker::cameraRun();
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}

void Worker::sendFrame(cv::Mat frame, WebSocketChannelPtr ws)
{
  std::vector<uchar> buffer;
  if (!frame.empty())
  {
    cv::imencode(".jpeg", frame, buffer);
    std::string image = base64Encode(buffer.data(), buffer.size());
    if (ws != nullptr)
    {
      ws->send(image);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  }
}