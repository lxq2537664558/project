#pragma once

#include <string>
#include <functional>
#include <memory>

#include "base/Timestamp.h"

namespace MuduoPlus
{
    class EventLoop;

    class Channel
    {
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void(Timestamp)> ReadEventCallback;

        Channel(EventLoop* loop, int fd);
        ~Channel();

        void handleEvent(Timestamp receiveTime);
        void setReadCallback(const ReadEventCallback& cb)
        {
            readCallback_ = cb;
        }
        void setWriteCallback(const EventCallback& cb)
        {
            writeCallback_ = cb;
        }
        void setCloseCallback(const EventCallback& cb)
        {
            closeCallback_ = cb;
        }
        void setErrorCallback(const EventCallback& cb)
        {
            errorCallback_ = cb;
        }

        /// Tie this channel to the owner object managed by shared_ptr,
        /// prevent the owner object being destroyed in handleEvent.
        void tie(const std::shared_ptr<void>&);

        int fd() const { return fd_; }
        int events() const { return events_; }
        void set_revents(int revt) { revents_ = revt; } // used by pollers
        // int revents() const { return revents_; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }

        void enableReading() { events_ |= kReadEvent; update(); }
        void disableReading() { events_ &= ~kReadEvent; update(); }
        void enableWriting() { events_ |= kWriteEvent; update(); }
        void disableWriting() { events_ &= ~kWriteEvent; update(); }
        void disableAll() { events_ = kNoneEvent; update(); }
        bool isWriting() const { return events_ & kWriteEvent; }
        bool isReading() const { return events_ & kReadEvent; }

        // for Poller
        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }

        // for debug
        std::string reventsToString() const;
        std::string eventsToString() const;

        void doNotLogHup() { logHup_ = false; }

        EventLoop* ownerLoop() { return loop_; }
        void remove();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
        static const int kErrorEvent;

    private:

        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        EventLoop* loop_;
        const int  fd_;
        int        events_;
        int        revents_; // it's the received event types of epoll or poll
        int        index_; // used by Poller.
        bool       logHup_;

        /*
        boost::weak_ptr<void> tie_;
        bool tied_;
        bool eventHandling_;
        bool addedToLoop_;*/
        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;
    };
}