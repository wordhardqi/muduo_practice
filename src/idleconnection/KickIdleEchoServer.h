//
// Created by renming on 6/9/18.
//

#ifndef ECHO_KICKIDLEECHOSERVER_H
#define ECHO_KICKIDLEECHOSERVER_H
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>

#include <unordered_set>
#include <boost/circular_buffer.hpp>

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

typedef weak_ptr<muduo::net::TcpConnection> WeakConnectionPtr;
struct ConnEntry : public muduo::copyable {
  explicit ConnEntry(const WeakConnectionPtr &weakConn) :
      weakConn_(weakConn) {

  }
  ~ConnEntry() {
    auto sharedConn_ = weakConn_.lock();
    if (sharedConn_) {
      sharedConn_->shutdown();
    }

  }
  WeakConnectionPtr weakConn_;

};

class KickIdleEchoServer {
 public:
  explicit KickIdleEchoServer(
      muduo::net::EventLoop *loop,
      muduo::net::InetAddress listenAddress,
      int idleSeconds
  ) : server_(loop, listenAddress, "KickIdleEchoServer"),
      connectionWheel_(idleSeconds) {
    server_.setConnectionCallback(
        std::bind(&KickIdleEchoServer::onConnection, this, _1)
    );
    server_.setMessageCallback(
        std::bind(&KickIdleEchoServer::onMessage, this, _1, _2, _3)
    );
    loop->runEvery(1.0, std::bind(&KickIdleEchoServer::onTimer, this));

    connectionWheel_.resize(idleSeconds);
    dumpConnectionBuckets();
  }
  void start() {
    server_.start();
  }

  void onConnection(const muduo::net::TcpConnectionPtr &conn) {
    if (conn->connected()) {
      shared_ptr<ConnEntry> entry(new ConnEntry(conn));
      connectionWheel_.back().insert(entry);
      weak_ptr<ConnEntry> weakEntry(entry);
      conn->setContext(weakEntry);
    } else {
      //do nothing
    }

  }
  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf,
                 muduo::Timestamp timestamp) {
    muduo::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << timestamp.toString();
    //Todo:: where to put send

    auto entry = boost::any_cast < weak_ptr < ConnEntry >>
                                                        (conn->getContext()).lock();

    if (entry) {
      conn->send(msg);

      connectionWheel_.back().insert(entry);
    }

  }
  void onTimer() {
    connectionWheel_.push_back(Bucket());
//    dumpConnectionBuckets();
  }
  void dumpConnectionBuckets() const {
    LOG_INFO << "size = " << connectionWheel_.size();
    int idx = 0;
    for (auto bucketI = connectionWheel_.begin();
         bucketI != connectionWheel_.end();
         ++bucketI, ++idx) {
      const Bucket &bucket = *bucketI;
      printf("[%d] len = %zd : ", idx, bucket.size());
      for (Bucket::const_iterator it = bucket.begin();
           it != bucket.end();
           ++it) {
        bool connectionDead = (*it)->weakConn_.expired();
        printf("%p(%ld)%s, ", get_pointer(*it), it->use_count(),
               connectionDead ? " DEAD" : "");
      }
      puts("");
    }
  }

 private:
  typedef unordered_set<shared_ptr<ConnEntry>> Bucket;
  typedef boost::circular_buffer<Bucket> ConnectionWheel;
  muduo::net::TcpServer server_;
  ConnectionWheel connectionWheel_;
};

#endif //ECHO_KICKIDLEECHOSERVER_H
