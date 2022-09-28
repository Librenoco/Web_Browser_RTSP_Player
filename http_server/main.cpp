// UTF-8  ' '(%20)
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <hv/HttpServer.h>
#include <hv/WebSocketServer.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "hmac.hpp"

#define SERVER_THREAD_NUM 4

std::mutex mtx;
std::mutex mtx2;
std::string header = "{\"alg\": \"HS256\", \"typ\": \"jwt\"}";
const std::string key = "JABSFGV&)(!@#HR)*F!@#*GSGV";

struct user
{
  std::string userName;
  std::string userPass;
  std::string userId;
};

struct videoServer
{
  std::string serverIpPort;
  WebSocketChannelPtr channel;
  std::vector<std::string> users;
};

const char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

std::string base64_encode(const std::string &in)
{
  std::string out;
  int val = 0, valb = -6;
  size_t len = in.length();
  unsigned int i = 0;
  for (i = 0; i < len; i++)
  {
    unsigned char c = in[i];
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0)
    {
      out.push_back(base64_url_alphabet[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
  {
    out.push_back(base64_url_alphabet[((val << 8) >> (valb + 8)) & 0x3F]);
  }
  return out;
}

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
  std::vector<videoServer> videoServers;
  std::vector<struct user> users;
  hv::WebSocketServer server;
  hv::WebSocketService ws;
  hv::HttpService router;
  std::string clientHTML;
  std::string ipHTTPServer;
  std::string line;
  int port;

  //Считывание пользователей из файла
  std::ifstream fs("user.txt", std::ios::in | std::ios::out);
  if (!fs)
  {
    std::cout << "ERROR (fileRead)";
    return 1;
  }
  while (std::getline(fs, line))
  {
    int pos = line.find("&");
    std::string userName = line.substr(0, pos);
    line.erase(0, pos + 1);
    pos = line.find("&");
    std::string userPass = line.substr(0, pos);
    users.push_back(user({userName, userPass, "t"}));
  }

  //Получение Ip+Port для создания http сервера
  std::ifstream fs2("ip_port_http_server.conf", std::ios::in);
  if (!fs2)
  {
    std::cout << "ERROR (fileRead)";
    return 1;
  }
  std::getline(fs2, line);
  ipHTTPServer = line;
  std::getline(fs2, line);
  port = std::stoi(line);

  //Работа с HTTP сервером
  //Отправка страницы авторизации
  router.GET("/", [](HttpRequest *req, HttpResponse *resp)
             { return resp->File("./page/auth.html"); });

  //Отправка страницы с клиентом камер
  router.GET("/TEST", [](HttpRequest *req, HttpResponse *resp)
             { return resp->File("./page/TEST.html"); });

  //Проверка авторизации
  router.POST("/", [&](const HttpContextPtr &ctx)
              {
    size_t pos = 0;
    std::string delimiter = "&";
    std::vector <std::string> token;
    while ((pos = ctx->body().find(delimiter)) != std::string::npos) {
        token.push_back(ctx->body().substr(0, pos));
        ctx->body().erase(0, pos + delimiter.length());
    }

    if (token.size() > 3 || token.size() == 0)
      return ctx->sendFile("./page/auth.html");
    else
    {
      if (token[0].find("user=") == std::string::npos)
        return ctx->sendFile("./page/auth.html");
      if (token[1].find("pass=") == std::string::npos)
        return ctx->sendFile("./page/auth.html");
    }

    pos = token[0].find("=");
    std::string userLogin = token[0].substr((pos + 1), token[0].size() - (pos + 1));
    pos = token[1].find("=");
    std::string userPass = token[1].substr((pos + 1), token[1].size() - (pos + 1));

    if (userLogin.empty() || userPass.empty())
      return ctx->sendFile("./page/auth.html");

    for (size_t i = 0; i < users.size(); i++)
    {
      if (userLogin == users[i].userName && userPass == users[i].userPass)
      {
        //Создание уникального uuid для авторизированного пользователя
        boost::uuids::uuid userId = boost::uuids::random_generator()();
        users[i].userId = boost::uuids::to_string(userId);

        std::string payload = "{\"user\": \"" + userLogin + "\", \"userId\": \"" + users[i].userId + "\"}";
        std::string unsignedToken = base64_encode(header) + '.' + base64_encode(payload);
        std::string signature = hmac::get_hmac(key, unsignedToken, hmac::TypeHash::SHA256);
        std::string fullToken = base64_encode(header) + '.' + base64_encode(payload) + '.' + signature;

        //Настройка cookie для пользователя
        HttpCookie cookie;
        cookie.name = "user";
        cookie.path = "/";
        cookie.max_age = 600;
        cookie.value = fullToken;
        std::cout << "TEST =";
        ctx->setCookie(cookie);
        //Считывание HTML файла
        clientHTML = "<!DOCTYPE html><html lang=\"ru\"><head><meta charset=\"UTF-8\"></head>";
        clientHTML += "<meta http-equiv=\"refresh\" content=\"0; url=http://"+ ipHTTPServer + ":" + std::to_string(port) +"/TEST\" />";
        ctx->response->SetHeader("Content-Type", "text/html");
        return ctx->sendString(clientHTML);
        break;
      }
    }
  return ctx->sendFile("./page/auth.html"); });

  //Проверка запросов пользователей и выдача страницы клиента
  router.POST("/TEST", [&](const HttpContextPtr &ctx)
              {
    //Проверка cookie и парсинг
    if (ctx->cookie("user").value.empty())
      return 200;

    std::string cookie = ctx->cookie("user").value;
    size_t pos = 0;
    std::string delimiter = ".";
    std::vector <std::string> token;
    while ((pos = cookie.find(delimiter)) != std::string::npos) {
        token.push_back(cookie.substr(0, pos));
        cookie.erase(0, pos + delimiter.length());
    }
    token.push_back(cookie);

    if (token.size() > 3 || token.size() == 0)
      return 200;
    //Сравнение cookie с данными сервера
    std::string headerOld = base64_decode(token[0]);
    std::string payloadOld = base64_decode(token[1]);
    std::string signatureOld = token[2];
    if (headerOld.empty() || payloadOld.empty())
      return 200;

    //Проверка токена на изменение
    for (int i = 0; i < users.size(); i++)
    {
      std::string payload = "{\"user\": \"" + users[i].userName  + "\", \"userId\": \"" + users[i].userId  + "\"}";
      if ((header == headerOld) && (payload == payloadOld))
      {
        std::string unsignedToken = base64_encode(header) + '.' + base64_encode(payload);
        std::string signature = hmac::get_hmac(key, unsignedToken, hmac::TypeHash::SHA256);
        if (signature == signatureOld)
        {
          //Считывание HTML файла
          std::ifstream fs("./page/client.html", std::ios::in);
          if (!fs)
          {
            std::cout << "ERROR (fileRead)";
            return 1;
          }
          clientHTML = "<!DOCTYPE html><script language=\"JavaScript\">";
          if (videoServers.size() > 1)
          {
            int test = (rand() % videoServers.size() + 1);
            std::cout << "\n i=" << test;
            clientHTML += "let ip = \""+ videoServers[test - 1].serverIpPort +"\";";
          }
          else
          {
            std::cout << "\n1";
            clientHTML += "let ip = \""+ videoServers[0].serverIpPort +"\";";
          }
          while (std::getline(fs, line))
            clientHTML += line;
          ctx->response->SetHeader("Content-Type", "text/html");
          videoServers[0].channel->send(base64_encode(payload));
          return ctx->sendString(clientHTML);
          //return ctx->sendFile("./page/client.html");
        }
        else
        {
          return 404;
        }
      }
    }
    return 404; });

  //Работа с вебсокетом клиенты
  ws.onopen = [&](const WebSocketChannelPtr &channel, const HttpRequestPtr &req)
  {
    std::cout << "\nVideo server connect" << std::endl;
  };

  ws.onmessage = [&](const WebSocketChannelPtr &channel, const std::string &msg)
  {
    std::string text = msg.data();
    if (text[text.size() - 1] == '1')
    {
      std::cout << "\nVideo server text=" << base64_decode(text) << std::endl;
      /*Реализация записи пользователей определённому серверу
      for (auto it : videoServers)
      {
        if(it.channel == channel)
        {
          text.resize(text.size() - 1);
          std::cout << "\nasdasd = " << text;
          it.users.push_back(base64_decode(text));
          std::cout << "TEST =" << it.users[0];
          break;
        }
      }
      std::cout << "\n------------------------------------------------------------"<<std::endl;
      for (auto it : videoServers)
      {
        std::cout << "\nVideo server ipPort="<< it.serverIpPort <<std::endl;
        for (auto it2 : it.users)
          std::cout << "\nVideo server user=" << it2 <<std::endl;
      }
      std::cout << "\n------------------------------------------------------------"<<std::endl;*/
    }
    else
    {
      std::cout << "\n------------------------------------------------------------" << std::endl;
      std::cout << "\nVideo server ipProt=" << msg.data() << std::endl;
      videoServers.push_back(videoServer({msg.data(), channel}));
      for (int i = 0; i < videoServers.size(); i++)
        std::cout << "\nVideo server " << i << " =" << videoServers[i].serverIpPort << " : " << videoServers[i].channel << std::endl;
      std::cout << "\n------------------------------------------------------------" << std::endl;
    }
  };

  ws.onclose = [&](const WebSocketChannelPtr &channel)
  {
    mtx.lock();
    std::cout << "\nVideoServer close" << std::endl;

    int index = -1;
    for (auto it : videoServers)
    {
      index++;
      if (it.channel == channel)
      {
        std::cout << "\nDelete video server = " << channel << " : index =" << index;
        videoServers.erase(videoServers.begin() + index);
      }
    }
    if (videoServers.size() == 0)
      std::cout << "\nVector videoServers empty" << std::endl;
    else
      for (int i = 0; i < videoServers.size(); i++)
        std::cout << "\nVideo server " << i << " =" << videoServers[i].serverIpPort << " : " << videoServers[i].channel << std::endl;
    mtx.unlock();
  };

  server.registerHttpService(&router);
  server.registerWebSocketService(&ws);
  server.setHost(ipHTTPServer.c_str());
  server.setPort(port);
  server.setThreadNum(SERVER_THREAD_NUM);
  server.run();

  return 0;
}
