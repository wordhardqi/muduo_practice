//
// Created by renming on 6/7/18.
//

#include "Codec.h"

#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer {
 public:

  explicit ChatServer(EventLoop *loop,
                      const InetAddress &inetAddress)
      : server_(loop, inetAddress, "Chat Server"),
        codec_(std::bind(&ChatServer::onMessage, this, _1, _2, _3)) {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnect, this, _1));
    server_.setMessageCallback(
        std::bind(&Codec::onMessage, &codec_, _1, _2, _3));
  }
  void start() {
    server_.start();
  }

 private:
  void onConnect(const TcpConnectionPtr &conn) {

    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      connectionList_.insert(conn);
    } else {
      connectionList_.erase(conn);
    }

  }

  void onMessage(const TcpConnectionPtr &conn,
                 const muduo::string &message,
                 Timestamp) {
    for (ConnectionList::iterator it = connectionList_.begin();
         it != connectionList_.end();
         it++) {
      codec_.send(get_pointer(*it), message);
    }

  }

  typedef std::set<TcpConnectionPtr> ConnectionList;
  TcpServer server_;
  Codec codec_;
  ConnectionList connectionList_;

};

#endif //ECHO_SERVER_H
