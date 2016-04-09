#pragma once

#include <functional>
#include <memory>

#include "base/Platform.h"
#include "Channel.h"
#include "InetAddress.h"

namespace MuduoPlus
{
    class EventLoop;
    class InetAddress;

    class Acceptor : public std::enable_shared_from_this<Acceptor>
    {
    public:
        typedef std::function < void(int sockfd,
            const InetAddress&) > NewConnCallback;

        Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnCallback& cb)
        {
            m_NewConnCallBack = cb;
        }

        void Listen();
        bool listenning() const { return listenning_; }

    private:
        void HandleRead();

        bool                        m_bReuseport;
        InetAddress                 m_ListenAddr;
        bool                        listenning_;
        EventLoop*                  m_pEventLoop;
        socket_t                    m_ListenFd;
        std::shared_ptr<Channel>    m_AcceptChannelPtr;
        NewConnCallback             m_NewConnCallBack;
    };
}