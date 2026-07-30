#include "audiomanager.h"

AudioManager::AudioManager()
    : QObject(0),
      m_mediaObject(0),
      m_audioOutput(0),
      m_isMuted(false)
{
}

void AudioManager::init()
{
    m_menuTrack = "sounds/menu_music.mp3";
    m_mediaObject = new Phonon::MediaObject(this);
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);

    Phonon::createPath(m_mediaObject, m_audioOutput);
    m_audioOutput->setVolume(0.4);

    connect(m_mediaObject,
            SIGNAL(finished()),
            this,
            SLOT(onFinished()));
}

void AudioManager::playMenuMusic()
{
    if(!m_mediaObject) return;

    if(m_mediaObject->currentSource().fileName().endsWith("menu_music.mp3") && m_mediaObject->state() == Phonon::PlayingState)
        return;

    m_mediaObject->setCurrentSource(Phonon::MediaSource(m_menuTrack));
    m_mediaObject->play();
}

void AudioManager::stopMusic()
{
    if(!m_mediaObject) return;
    m_mediaObject->stop();
}

void AudioManager::toggleMute()
{
    if(!m_audioOutput) return;
    m_isMuted = !m_isMuted;
    m_audioOutput->setMuted(m_isMuted);
}

void AudioManager::onFinished()
{
    if(!m_currentGameTrack.isEmpty())
    {
        m_mediaObject->setCurrentSource(
            Phonon::MediaSource(m_currentGameTrack));
        m_mediaObject->play();
    }
    else
    {
        m_mediaObject->setCurrentSource(
            Phonon::MediaSource(m_menuTrack));
        m_mediaObject->play();
    }
}

void AudioManager::playGameMusic(
        const QVector<MusicRange>& ranges,
        int levelNumber)
{
    m_musicRanges      = ranges;
    m_currentGameLevel = levelNumber;

    QString track;
    for(int i = 0; i < ranges.size(); i++)
    {
        if(levelNumber >= ranges.at(i).startLevel
           && levelNumber <= ranges.at(i).endLevel)
        {
            track = QString("sounds/%1").arg(ranges.at(i).fileName);
            break;
        }
    }

    if(track.isEmpty())
    {
        stopGameMusic();
        return;
    }

    if(track == m_currentGameTrack && m_mediaObject->state() == Phonon::PlayingState)
        return; 

    m_currentGameTrack = track;
    m_mediaObject->setCurrentSource(Phonon::MediaSource(track));
    m_mediaObject->play();
}

void AudioManager::stopGameMusic()
{
    if(!m_mediaObject->currentSource().fileName().endsWith("menu_music.mp3")){
        m_currentGameTrack.clear();
        m_mediaObject->stop();
    }
}

void AudioManager::ToggleMusicPlaying(const QVector<MusicRange>& ranges, int levelNumber){

    if(m_mediaObject->state() == Phonon::StoppedState){
        playGameMusic(ranges, levelNumber);
    }
    else{
        stopGameMusic();
    }

}

void AudioManager::pause()
{
    if(m_mediaObject->state() == Phonon::PlayingState)
        m_mediaObject->pause();
}

void AudioManager::resume()
{
    if(m_mediaObject->state() == Phonon::PausedState)
        m_mediaObject->play();
}
