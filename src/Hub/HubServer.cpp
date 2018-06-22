//
// Created by renming on 6/10/18.
//

#include <muduo/net/EventLoop.h>
#include "HubServer.h"

int main() {
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenaddr(2008);
  HubServer m_echoServer(&loop, listenaddr);
  m_echoServer.start();
  loop.loop();
}