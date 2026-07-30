#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <QString>
#include <QMap>

struct LevelSaveEntry
{
    bool solved;
    int  bestMoves;
    int  bestTimeSeconds; // milliseconds

    LevelSaveEntry()
        : solved(false), bestMoves(0), bestTimeSeconds(0) {}
};

struct SaveData
{
    QString levelSetFile;
    int     lastUnlockedLevel;
    QMap<int, LevelSaveEntry> levels; // key: level number

    SaveData() : lastUnlockedLevel(0) {}
    bool isCompleted(int totalLevels) const
    {
        return lastUnlockedLevel >= totalLevels;
    }
};

#endif // SAVEDATA_H
