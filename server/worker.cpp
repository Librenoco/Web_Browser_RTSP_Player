#include "worker.h"

Worker::Worker(WebSocketChannelPtr ws, std::string streamFrame)
{
  this->ws = ws;
  stopThread = false;
  threadTest = new std::thread (&Worker::cameraRun, this, streamFrame, ws);
  threadTest->detach();
}

Worker::~Worker()
{
  stopThread = true;
  delete threadTest;
  std::cout << "\nthread stop WS= " << ws;
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

void Worker::cameraRun(std::string cam, WebSocketChannelPtr ws)
{
  std::string launch_str = "rtspsrc protocols=udp location=" + cam + " drop-on-latency=true latency=" + "200" + " ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true";
  auto cap = cv::VideoCapture(launch_str, cv::CAP_GSTREAMER);
  sleep(3);
  if (cap.isOpened())
  {
    while (1)
    {
      if (stopThread)
      {
        std::cout << "\nthread stop- WS= " << ws;
        return;
      }
      cv::Mat frame;
      cap >> frame;
      Worker::getFrame(frame, ws);
    }
  }
  else
  {
    if (stopThread)
    {
      std::cout << "\nthread stop+ WS= " << ws;
      return;
    }
    Worker::cameraRun(cam, ws);
    sleep(1);
  }
}

void Worker::getFrame(cv::Mat frame, WebSocketChannelPtr ws)
{
  std::vector<uchar> buffer;
  if (!frame.empty())
  {
    cv::imencode(".jpeg", frame, buffer);
    std::string image = base64Encode(buffer.data(), buffer.size());
    if (ws != nullptr)
    {
      ws->send(image);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
}
