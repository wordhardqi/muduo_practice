//
// Created by renming on 6/3/18.
//
#include "sudoku.h"

#include "echo.h"
#include <muduo/base/Logging.h>
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


using namespace muduo;
using namespace muduo::net;
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

//void SudokuSolver::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer * buf ,
//                             muduo::Timestamp timestamp) {
//    muduo::string msg(buf->retrieveAllAsString());
//    LOG_INFO<<conn->name() << " echo "<<msg.size()
//            <<" bytes, " <<"data received at "<<timestamp.toString();
//    conn->send(msg);
//
//}


void SudokuSolver::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
                             muduo::Timestamp timestamp) {
  LOG_DEBUG << conn->name();
  size_t len = buf->readableBytes();
  while (len >= kCells + 2) {
    const char *crlf = buf->findCRLF();
    if (crlf) {
      string request(buf->peek(), crlf);
      string id;
      buf->retrieveUntil(crlf + 2);
      string::iterator colon = find(request.begin(), request.end(), ':');
      if (colon != request.end()) {
        id.assign(request.begin(), colon);
        request.erase(request.begin(), colon + 1);
      }
      if (request.size() == size_t((kCells))) {
        string result = solveSudoku(request);
        if (id.empty()) {
          conn->send(result + "\r\n");

        } else {
          conn->send(id + ":" + result + "\r\n");
        }
      } else {
        conn->send("Bad Request! \r\n");
        conn->shutdown();
      }
    } else {
      break;
    }
  }
}
void SudokuSolver::start() {
  server_.start();
}