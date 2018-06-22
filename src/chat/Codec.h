//
// Created by renming on 6/7/18.
//

#ifndef ECHO_CODEC_H
#define ECHO_CODEC_H

#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/Endian.h>
#include <muduo/net/TcpConnection.h>
#include <functional>
using namespace muduo::net;
using namespace muduo;

class Codec {
 public:

  typedef std::function<void (
  const muduo::net::TcpConnectionPtr &conn,
  const muduo::string &message, muduo::Timestamp
  timeStamp)>
  StringMessageCallback;
  explicit Codec(const StringMessageCallback &cb) :
      stringMessageCallback_(cb) {
    //empty
  }
  void onMessage(const TcpConnectionPtr &conn,
                 Buffer *buf,
                 muduo::Timestamp receivetime) {
    while (buf->readableBytes() >= kHeaderLen) {
      const void *len_data = buf->peek();
      int32_t be32 = *static_cast<const int32_t * >(len_data);

      const int32_t len = muduo::net::sockets::networkToHost32(be32);
      if (len > 65536 || len < 0) {
        LOG_ERROR << "invalide message length " << len;
        conn->shutdown();
        break;
      }
      if (buf->readableBytes() >= kHeaderLen + len) {
        buf->retrieve(kHeaderLen);
        muduo::string message(buf->peek(), len);
        stringMessageCallback_(conn, message, receivetime);
        buf->retrieve(len);

      }

    }
  }
  void send(TcpConnection *conn,
            const muduo::StringPiece &message) {
    muduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = message.size();
    int32_t lenHeader = muduo::net::sockets::hostToNetwork32(len);
    buf.prepend(&lenHeader, sizeof(lenHeader));
    conn->send(&buf);
  }

 private:
  StringMessageCallback stringMessageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif //ECHO_CODEC_H



