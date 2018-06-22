//
// Created by renming on 6/3/18.
//

#include "echo.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

int main() {
  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenaddr(2007);
  SudokuSolver m_echoServer(&loop, listenaddr);
  m_echoServer.start();
  loop.loop();
}