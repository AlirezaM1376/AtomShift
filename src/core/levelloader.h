#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include <QString>
#include "leveldata.h"

class LevelLoader
{
public:
    static bool loadLevel(
            const QString& fileName,
            int levelNumber,
            LevelData& level);

    static int levelCount(const QString& fileName);
    static QString levelSetName(const QString& fileName);

    QVector<MusicRange> musicRanges;
    static QVector<MusicRange> loadMusicRanges(const QString& fileName);

};

#endif // LEVELLOADER_H
