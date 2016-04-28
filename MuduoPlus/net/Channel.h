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

        void setErrorCallback(const EventCallback& cb)
        {
            closeCallback_ = cb;
        }

        void setFinishCallback(const EventCallback& cb)
        {
            finishCallback_ = cb;
        }

        int  fd() const { return fd_; }
        int  interestEvents() const { return interestEvents_; }
        void setRecvEvents(int events) { recvEvents_ = events; } // used by pollers
        bool isNoneEvent() const { return interestEvents_ == kNoneEvent; }

        void enableReading() { interestEvents_ |= kReadEvent; update(); }
        void disableReading() { interestEvents_ &= ~kReadEvent; update(); }
        void enableWriting() { interestEvents_ |= kWriteEvent; update(); }
        void disableWriting() { interestEvents_ &= ~kWriteEvent; update(); }
        void enableErroring() { interestEvents_ |= kErrorEvent; update(); }
        void disableErroring() { interestEvents_ &= ~kErrorEvent; update(); }
        void disableAll() { interestEvents_ = kNoneEvent; update(); }
        bool isWriting() const { return (interestEvents_ & kWriteEvent) != 0 ? true : false; }
        bool isReading() const { return (interestEvents_ & kReadEvent) != 0 ? true : false; }

        EventLoop* ownerLoop() { return loop_; }
        void remove();

        void setOwner(std::weak_ptr<void> ptr);
        std::weak_ptr<void> getOwner();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;
        static const int kErrorEvent;

    private:

        void update();
        void handleEventWithGuard(Timestamp receiveTime);

        EventLoop* loop_;
        const int  fd_;
        int        interestEvents_;
        int        recvEvents_; // it's the received event types of epoll or select

        std::weak_ptr<void> owner_;
        bool addedToLoop_;

        ReadEventCallback   readCallback_;
        EventCallback       writeCallback_;
        EventCallback       closeCallback_;
        EventCallback       finishCallback_;
    };
}