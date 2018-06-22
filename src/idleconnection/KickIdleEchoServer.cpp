//
// Created by renming on 6/9/18.
//

#include "KickIdleEchoServer.h"

int main() {
  LOG_INFO << "pid = " << getpid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(2014);
  KickIdleEchoServer server(&loop, listenAddr, 7);
  server.start();
  loop.loop();
}
