//
// Created by renming on 6/7/18.
//
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoopThread.h>
#include <iostream>
#include "Codec.h"
#include <functional>
using namespace muduo::net;
using namespace muduo;

class ChatClient {
 public:
  explicit ChatClient(EventLoop *loop,
                      const InetAddress serverAddress
  ) : client_(loop, serverAddress, "ChatClient"),
      codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)) {

    client_.setMessageCallback(
        std::bind(&Codec::onMessage, &codec_, _1, _2, _3)
    );
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, _1)
    );
  }

  void connect() {
    client_.connect();
  }
  void write(const muduo::StringPiece &message) {
    if (conn_) {
      codec_.send(get_pointer(conn_), message);
    }
  }
  void disconnect() {
    client_.disconnect();
  }

 private:
  void onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    MutexLockGuard lock(mutex_);
    if (conn->connected()) {
      conn_ = conn;
    } else {
      conn_.reset();
    }
  }

  void onStringMessage(const TcpConnectionPtr &conn,
                       const muduo::string &message,
                       Timestamp timestamp) {
    LOG_INFO << message;

  }

  TcpClient client_;
  Codec codec_;
  TcpConnectionPtr conn_;

  MutexLock mutex_;

};

int main(int argc, char **argv) {
  if (argc > 2) {
    uint16_t port = static_cast<uint16_t >(atoi(argv[2]));
    InetAddress serverAddress(argv[1], port);
    EventLoopThread loopThread;
    ChatClient client(loopThread.startLoop(), serverAddress);
    client.connect();
    string line;
    while (getline(std::cin, line)) {
      client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000 * 1000);
  } else {
    printf("Usage: %s host_ip port\n", argv[0]);
  }

}