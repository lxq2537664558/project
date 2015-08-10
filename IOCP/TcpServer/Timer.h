#pragma once

#include "Net.h"

#include <Windows.h>
#include <memory>
#include <thread>

typedef std::function<void(PostOperation::OpType iOpCode, int64_t llClientHandle)> PostFunc;

class Timer
{
public:
	Timer();
	~Timer(){}
public:
    void SetPostFunctor(PostFunc func);
    void Start(int32_t iInterval);
    void Stop();	

private:
    void WaitLoop();
private:
    HANDLE m_hTimerHandle;
	int32_t m_iInterval;
    std::shared_ptr<std::thread> m_timerWaitThreadPtr;
    PostFunc m_postFunctor;
    bool m_bQuit;
};