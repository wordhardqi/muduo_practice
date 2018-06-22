//
// Created by renming on 6/3/18.
//

#include "echo.h"
#include <muduo/base/Logging.h>
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

SudokuSolver::SudokuSolver(
    muduo::net::EventLoop *loop,
    const muduo::net::InetAddress &listenAddr)
    : loop_(loop), server_(loop, listenAddr, "My_ECHO") {

  server_.setConnectionCallback(
      std::bind(&SudokuSolver::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&SudokuSolver::onMessage, this, _1, _2, _3));

}
void SudokuSolver::onConnection(const muduo::net::TcpConnectionPtr &conn) {
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << "->"
           << conn->localAddress().toIpPort() << "is "
           << (conn->connected() ? "up" : "down");

}

void SudokuSolver::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                             muduo::Timestamp timestamp) {
  muduo::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size()
           << " bytes, " << "data received at " << timestamp.toString();
  conn->send(msg);

}
void SudokuSolver::start() {
  server_.start();
}