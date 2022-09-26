#pragma once

#include <chrono>

class Timer
{
public:
    Timer();

    void Start();
    void Stop();
    void Restart();

    float ElapsedMilliseconds() const;
    float ElapsedSeconds() const;

private:
    bool m_running;
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    std::chrono::time_point<std::chrono::steady_clock> m_endTime;
    
};
