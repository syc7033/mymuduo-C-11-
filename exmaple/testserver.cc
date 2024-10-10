#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>

#include <string>
#include <functional>

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress& addr, const std::string& name)
        : server_(loop, addr, name)
        , loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        // 设置合适的loop线程数量
        server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();
    }
private:
    // 连接建立 断开的回调
    void onConnection(const TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());

        }
    }

    // 可读可写事件的回调
    void onMessage(const TcpConnectionPtr& conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        // conn->shutdown(); // 关闭写端 EPOLLHUP事件触发 --> closeCallback_回调 删除一条连接
    }
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8001);
    // listenfd(NONBLOCKING) => events + cb => channel => bind
    EchoServer server(&loop, addr, "EchoServer-01"); 
    // listen => 监听状态 => channel => poller => epoll
    // numThread => subLoop subThread => subloop.loop()
    server.start();
    loop.loop(); // 启动mainLoop的底层Poller
    return 0;
}