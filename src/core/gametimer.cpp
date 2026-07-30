#include "gametimer.h"

GameTimer::GameTimer()
    : m_started(false), m_paused(false), m_pausedElapsed(0)
{
}

void GameTimer::startOnFirstMove()
{
    if(m_started)
        return; // no action if timer already started

    m_timer.start();
    m_started = true;
}

void GameTimer::reset()
{
    m_timer.start();
    m_started = false;
    m_paused  = false;
    m_pausedElapsed = 0;
}

void GameTimer::pause()
{
    if(!m_started || m_paused)
        return;
    m_pausedElapsed = m_pausedElapsed + m_timer.elapsed();
    m_paused = true;
}

void GameTimer::resume()
{
    if(!m_started || !m_paused)
        return;
    m_timer.start(); // timer reset
    m_paused = false;
}

qint64 GameTimer::elapsedMs() const
{
    if(!m_started)
        return 0;
    if(m_paused)
        return m_pausedElapsed;
    return m_pausedElapsed + m_timer.elapsed();
}
