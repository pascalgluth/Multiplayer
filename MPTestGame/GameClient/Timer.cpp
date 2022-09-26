#include "Timer.h"

using namespace std::chrono;

Timer::Timer()
{
    m_startTime = high_resolution_clock::now();
    m_endTime = high_resolution_clock::now();
    m_running = false;
}

void Timer::Start()
{
    if (!m_running)
    {
        m_startTime = high_resolution_clock::now();
        m_running = true;
    }
}

void Timer::Stop()
{
    if (m_running)
    {
        m_endTime = high_resolution_clock::now();
        m_running = false;
    }
}

void Timer::Restart()
{
    m_startTime = high_resolution_clock::now();
    m_running = true;
}

float Timer::ElapsedMilliseconds() const
{
    if (m_running)
    {
        const auto elapsed = duration<float, std::milli>(high_resolution_clock::now() - m_startTime);
        return elapsed.count();
    }

    const auto elapsed = duration<float, std::milli>(m_endTime - m_startTime);
    return elapsed.count();
}

float Timer::ElapsedSeconds() const
{
    if (m_running)
    {
        const auto elapsed = duration<float>(high_resolution_clock::now() - m_startTime);
        return elapsed.count();
    }

    const auto elapsed = duration<float>(m_endTime - m_startTime);
    return elapsed.count();
}
