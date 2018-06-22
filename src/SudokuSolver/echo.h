//
// Created by renming on 6/3/18.
//

#ifndef ECHO_ECHO_H
#define ECHO_ECHO_H

#include <muduo/net/TcpServer.h>
class SudokuSolver {
 public:
  SudokuSolver(muduo::net::EventLoop *loop,
               const muduo::net::InetAddress &listenAddr);
  void start();

 private:
  void onConnection(const muduo::net::TcpConnectionPtr &conn);
  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *,
                 muduo::Timestamp timestamp);
  muduo::net::EventLoop *loop_;
  muduo::net::TcpServer server_;
};

#endif //ECHO_ECHO_H
