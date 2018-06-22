//
// Created by renming on 6/7/18.
//
#include "Codec.h"
#include "server.h"

int main() {
  EventLoop loop;
  InetAddress inetAddress(2013);
  ChatServer chatServer(&loop, inetAddress);
  chatServer.start();
  loop.loop();
}