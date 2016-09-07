#include "base/Logger.h"

#include "TcpConnection.h"
#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

namespace MuduoPlus
{

    void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
    {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }

    HttpServer::HttpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& name,
        TcpServer::Option option)
        : server_(loop, listenAddr, name, option),
        httpCallback_(defaultHttpCallback)
    {
        server_.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, 
            std::placeholders::_1));

        server_.setMessageCallback(
            std::bind(&HttpServer::onMessage, this, 
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    HttpServer::~HttpServer()
    {
    }

    void HttpServer::start()
    {
        LOG_PRINT(LogType_Warn, "HttpServer[%s] starts listenning on %s", server_.name().c_str(), 
            server_.ipPort().c_str());

        server_.start();
    }

    void HttpServer::onConnection(const TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            conn->setContext(HttpContext());
        }
    }

    void HttpServer::onMessage(const TcpConnectionPtr& conn,
        Buffer* buf,
        Timestamp receiveTime)
    {
        HttpContext &context = conn->getContext().AnyCast<HttpContext>();

        if (!context.parseRequest(buf, receiveTime))
        {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->gracefulClose();
        }

        if (context.gotAll())
        {
            onRequest(conn, context.request());
            context.reset();
        }
    }

    void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
    {
        const std::string& connection = req.getHeader("Connection");
        bool close = connection == "close" ||
            (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
        HttpResponse response(close);
        httpCallback_(req, &response);
        Buffer buf;
        response.appendToBuffer(&buf);
        conn->send(buf.peek(),buf.readableBytes());
        if (response.closeConnection())
        {
            conn->gracefulClose();
        }
    }
}