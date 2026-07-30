#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QString>
#include "savedata.h"

class SaveManager
{
public:
    /**
     * Creates save file path based on the level pack file.
     * Example: "levels/default_levels.dat" → "saves/default_levels.sav"
     */
    static QString saveFilePath(const QString& levelSetFile);

    /**
     * Loads the save data.
     * Return empty date if there is no save file.
     */
    static SaveData load(const QString& levelSetFile);

    /**
     * Saves data on disk.
     */
    static bool save(const SaveData& data);

    /**
     * Saving/updating data avter solving a level.
     */
    static void recordSolve(
            SaveData&   data,
            int         levelNumber,
            int         moves,
            qint64      elapsedMs);

    /**
     * Checking if level pack completed.
     */
    static bool isLevelSetCompleted(const QString& levelSetFile);
};

#endif // SAVEMANAGER_H
