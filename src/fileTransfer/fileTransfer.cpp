//
// Created by renming on 6/5/18.
//

#include "fileTransfer.h"
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
const char *g_file = NULL;

string readFile(const char *filename) {
  string content;
  FILE *fp = ::fopen(filename, "rb");
  if (fp) {
    const int kBufSize = 1024 * 1024;
    char iobuf[kBufSize];
    ::setbuffer(fp, iobuf, sizeof(iobuf));

    char buf[kBufSize];
    size_t nread = 0;
    while ((nread = ::fread(buf, 1, sizeof buf, fp))) {
      content.append(buf, nread);
    }
    ::fclose(fp);
  }
  return content;
}

void onHighWaterMark(const TcpConnectionPtr &conn, size_t len) {
  LOG_INFO << "HighWaterMark" << len;
}

void onConnection(const TcpConnectionPtr &conn) {
  LOG_INFO << "FileServer -" << conn->peerAddress().toIpPort()
           << conn->localAddress().toIpPort() << "is "
           << (conn->connected() ? "up" : "down");
  if (conn->connected()) {
    LOG_INFO << "FileServer - Sending file " << g_file
             << " to " << conn->peerAddress().toIpPort();
    conn->setHighWaterMarkCallback(onHighWaterMark, 64 * 1024);
    string fileContent = readFile(g_file);
    conn->send(fileContent);
    conn->shutdown();
    LOG_INFO << "FileServer - done";
  }
}

int main(int argc, char *argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 1) {
    g_file = argv[1];

    EventLoop loop;
    InetAddress listenaddr(2012);
    TcpServer server(&loop, listenaddr, "FileServer");
    server.setConnectionCallback(onConnection);
    server.start();
    loop.loop();
  } else {
    fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
  }
}