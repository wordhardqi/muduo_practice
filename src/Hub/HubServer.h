//
// Created by renming on 6/10/18.
//

#ifndef ECHO_HUBSERVER_H
#define ECHO_HUBSERVER_H
#include <vector>

#include <muduo/net/TcpConnection.h>
#include <unordered_set>
#include <unordered_map>
#include <muduo/base/Logging.h>
#include <muduo/net/TcpServer.h>
using namespace muduo::net;
using namespace muduo;
class Topic {
 public:
  explicit Topic(string topic)
      : topic_(topic) {
    //do nothing
  }
  void add(const TcpConnectionPtr &conn) {
    if (conn->connected())
      subscribers_.insert(conn);
  }
  void remove(const TcpConnectionPtr &conn) {
    subscribers_.erase(conn);
  }
  void publish(const string &content) {
    content_ = content;
    for (auto &it : subscribers_) {
      it->send(makeMessage());
    }
  }
  string makeMessage() {
    return "pub " + topic_ + "\r\n" + content_ + "\r\n";
  }
 private:
  std::unordered_set<TcpConnectionPtr> subscribers_;
  string topic_;
  string content_;
};

class Codec {
 public:
  typedef std::function<
  void(
  const TcpConnectionPtr &conn,
  const string &topic
  )>
  SubscribeCallback;
  typedef std::function<
  void(
  const TcpConnectionPtr &conn,
  const string &topic
  )>
  UnSubscribeCallback;
  typedef std::function<
  void(
  const TcpConnectionPtr &conn,
  const string &topic,
  const string &content
  )>
  PublishCallback;

  explicit Codec(SubscribeCallback subscribeCallback,
                 UnSubscribeCallback unSubscribeCallback,
                 PublishCallback publishCallback)
      : SubCb_(subscribeCallback),
        UnSubCb_(unSubscribeCallback),
        PubCb_(publishCallback) {
    //empty
  }
  void
  onMessage(const TcpConnectionPtr &conn,
            Buffer *buf,
            muduo::Timestamp receivetime) {
    while (buf->findCRLF()) {
      const char *crlf = buf->findCRLF();
      const char *first_space = std::find(buf->peek(), crlf, ' ');
      string cmd(buf->peek(), first_space);
      string topic(first_space + 1, crlf);
      if (cmd == "sub") {
        SubCb_(conn, topic);
        buf->retrieveUntil(crlf + 2);
      } else if (cmd == "unsub") {
        UnSubCb_(conn, topic);
        buf->retrieveUntil(crlf + 2);
      } else if (cmd == "pub") {
        buf->retrieveUntil(crlf + 2);
        while (const char *second_crlf = buf->findCRLF()) {
          string content(buf->peek(), second_crlf);
          buf->retrieveUntil(second_crlf + 2);
          PubCb_(conn, topic, content);
        }

      }
    }

  }
 private:

  SubscribeCallback SubCb_;
  UnSubscribeCallback UnSubCb_;
  PublishCallback PubCb_;

};

class HubServer {
 public:
  typedef std::vector<TcpConnectionPtr> ConnectionList;
  typedef std::unordered_map<string, std::shared_ptr<Topic>> StringTopicMap;
  explicit HubServer(EventLoop *loop,
                     const InetAddress &inetAddress)
      : server_(loop, inetAddress, "Chat Server"),
        codec_(std::bind(&HubServer::doSubscribe, this, _1, _2),
               std::bind(&HubServer::doUnSubscribe, this, _1, _2),
               std::bind(&HubServer::doPublish, this, _1, _2, _3)
        ) {
    server_.setConnectionCallback(
        std::bind(&HubServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&Codec::onMessage, &codec_, _1, _2, _3));

  }
  void start() {
    server_.start();
  }

  void onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "HubServer - " << conn->peerAddress().toIpPort() << "->"
             << conn->localAddress().toIpPort() << "is "
             << (conn->connected() ? "up" : "down");

    if (conn->connected()) {
      connectionList_.push_back(conn);
    }
  }
  void doSubscribe(const TcpConnectionPtr &conn, const string topic) {
    if (stringTopicMap_.find(topic) == stringTopicMap_.end()) {
      std::shared_ptr<Topic> newTopicPtr(new Topic(topic));
      stringTopicMap_[topic] = newTopicPtr;
    }
    LOG_INFO << "HubServer - " << conn->peerAddress().toIpPort()
             << "Subscribe "
             << topic;
    stringTopicMap_[topic]->add(conn);
  }
  void doUnSubscribe(const TcpConnectionPtr &conn, const string topic) {
    if (stringTopicMap_.find(topic) == stringTopicMap_.end()) {
      return;
    }
    LOG_INFO << "HubServer - " << conn->peerAddress().toIpPort()
             << "UnSubscribe "
             << topic;
    stringTopicMap_[topic]->remove(conn);
  }
  void doPublish(const TcpConnectionPtr &conn, const string &topic,
                 const string &content) {
    auto topicPtr = stringTopicMap_[topic];
    topicPtr->publish(content);
    LOG_INFO << "HubServer - "
             << "Publish "
             << topic << " " << content;
  }
 private:
  ConnectionList connectionList_;
  StringTopicMap stringTopicMap_;
  TcpServer server_;
  Codec codec_;
};

#endif //ECHO_HUBSERVER_H
