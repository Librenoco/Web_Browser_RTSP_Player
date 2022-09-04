#include "worker.h"

Worker::Worker(WebSocketChannelPtr ws, std::vector<std::string> streamFrame)
{
  std::string cam;
  std::string blocNumber;
  this->stopThreads = false;
  this->streamFrame = streamFrame;
  this->ws = ws;
  threads = new std::thread *[this->streamFrame.size()];

  for (int i = 0; i < this->streamFrame.size(); i++)
  {
    blocNumber = this->streamFrame[i].substr(0, this->streamFrame[i].find(' ')) + " ";
    cam = this->streamFrame[i].substr(this->streamFrame[i].find(' '));
    threads[i] = new std::thread(&Worker::cameraRun, this, blocNumber, cam, ws);
    threads[i]->detach();
  }
}

Worker::~Worker()
{
  stopThreads = true;

  for (int i = 0; i < streamFrame.size(); i++)
  {
    delete threads[i];
  }
  delete threads;
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

void Worker::cameraRun(std::string blocNumber, std::string cam, WebSocketChannelPtr ws)
{
  std::string url = cam;
  std::string launch_str = "rtspsrc protocols=udp+tcp location=" + url + " drop-on-latency=true latency=" + "400" + " ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true ! queue";
  auto cap = cv::VideoCapture(launch_str, cv::CAP_GSTREAMER);
  if (cap.isOpened())
  {
    while (1)
    {
      if (stopThreads)
        return;
      cv::Mat frame;
      cap >> frame;
      Worker::getFrame(frame, blocNumber, ws);
    }
  }
  else
  {
    Worker::cameraRun(blocNumber, cam, ws);
    sleep(5);
    if (stopThreads)
      return;
  }
}

void Worker::getFrame(cv::Mat frame, std::string blocNumber, WebSocketChannelPtr ws)
{
  std::vector<uchar> buffer;
  if (!frame.empty())
  {
    cv::imencode(".jpeg", frame, buffer);
    std::string image = base64Encode(buffer.data(), buffer.size());
    if (ws != nullptr)
    {
      ws->send(blocNumber + image);
    }
  }
}
