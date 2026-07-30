#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <QElapsedTimer>

/**
 * GameTimer compute the elapsed time to solve a level based on milliseconds.
 * Timer starts only when player created the first move.
 */
class GameTimer
{
public:
    GameTimer();

    /**
     * Starting the timer. No change if already started.
     * Should only be used for the first move.
     */
    void startOnFirstMove();

    /**
     * Restting timer, makes the game ready to start again.
     * Used when loading a level.
     */
    void reset();

    /**
     * Elapsed time after first move.
     * Return 0 in case of no moves.
     */
    qint64 elapsedMs() const;
    void pause();
    void resume();
    bool isPaused() const { return m_paused; }
    bool isStarted() const { return m_started; }

private:
    QElapsedTimer m_timer;
    bool m_started;
    bool   m_paused;
    qint64 m_pausedElapsed; // Elapsed time before pause
};

#endif // GAMETIMER_H
