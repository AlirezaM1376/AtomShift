#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <phonon/MediaObject>
#include <phonon/AudioOutput>
#include <phonon/Path>
#include "leveldata.h"

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager& instance()
    {
        static AudioManager mgr;
        return mgr;
    }

    void init();
    void playMenuMusic();
    void pause();
    void resume();
    void stopMusic();
    void toggleMute();
    bool isMuted() const { return m_isMuted; }
    void playGameMusic(const QVector<MusicRange>& ranges, int levelNumber);
    void stopGameMusic();
    void ToggleMusicPlaying(const QVector<MusicRange>& ranges, int levelNumber);

private slots:
    void onFinished();

private:
    AudioManager();

    Phonon::MediaObject *m_mediaObject;
    Phonon::AudioOutput *m_audioOutput;

    QString m_menuTrack;
    QString m_inGame1Track;
    bool    m_isMuted;

    QVector<MusicRange> m_musicRanges;
    int                 m_currentGameLevel;
    QString             m_currentGameTrack;
};

#endif
