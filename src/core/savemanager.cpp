#include "savemanager.h"
#include "levelloader.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>

QString SaveManager::saveFilePath(const QString& levelSetFile)
{
    // getting level pack file name and adding .sav extension to file name
    QString baseName = QFileInfo(levelSetFile).baseName();
    return QString("saves/%1.sav").arg(baseName);
}

SaveData SaveManager::load(const QString& levelSetFile)
{
    SaveData data;
    data.levelSetFile = levelSetFile;

    QFile file(saveFilePath(levelSetFile));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return data; // No file, empty data

    QTextStream in(&file);
    QString currentSection;

    while(!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if(line.isEmpty() || line.startsWith(';'))
            continue;

        if(line.startsWith('[') && line.endsWith(']'))
        {
            currentSection = line.mid(1, line.length() - 2);
            continue;
        }

        int eqPos = line.indexOf('=');
        if(eqPos < 0) continue;

        QString key   = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 1).trimmed();

        if(currentSection == "Meta")
        {
            if(key == "LastUnlockedLevel")
                data.lastUnlockedLevel = value.toInt();
        }
        else if(currentSection.startsWith("Level"))
        {
            int levelNum = currentSection.mid(5).toInt();
            if(levelNum <= 0) continue;

            LevelSaveEntry& entry = data.levels[levelNum];

            if(key == "Solved")
                entry.solved = (value == "true");
            else if(key == "BestMoves")
                entry.bestMoves = value.toInt();
            else if(key == "BestTime")
                entry.bestTimeSeconds = value.toInt();
        }
    }

    file.close();
    return data;
}

bool SaveManager::save(const SaveData& data)
{
    // creating saves folder if not exist
    QDir().mkpath("saves");

    QFile file(saveFilePath(data.levelSetFile));
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    out << "[Meta]\n";
    out << "LevelSet=" << data.levelSetFile << "\n";
    out << "SaveVersion=1\n";
    out << "LastUnlockedLevel=" << data.lastUnlockedLevel << "\n";
    out << "\n";

    QList<int> keys = data.levels.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        int levelNum = keys.at(i);
        const LevelSaveEntry& entry = data.levels[levelNum];

        if(!entry.solved)
            continue;

        out << "[Level" << levelNum << "]\n";
        out << "Solved=true\n";
        out << "BestMoves=" << entry.bestMoves << "\n";
        out << "BestTime="  << entry.bestTimeSeconds << "\n";
        out << "\n";
    }

    file.close();
    return true;
}

void SaveManager::recordSolve(
        SaveData& data,
        int       levelNumber,
        int       moves,
        qint64    elapsedMs)
{
    int elapsedSeconds = (int)(elapsedMs / 1000);

    LevelSaveEntry& entry = data.levels[levelNumber];

    if(!entry.solved || moves < entry.bestMoves)
    {
        entry.solved          = true;
        entry.bestMoves       = moves;
        entry.bestTimeSeconds = elapsedSeconds;
    }
    else if(moves == entry.bestMoves && elapsedSeconds < entry.bestTimeSeconds)
    {
        entry.solved          = true;
        entry.bestMoves       = moves;
        entry.bestTimeSeconds = elapsedSeconds;
    }

    if(levelNumber > data.lastUnlockedLevel)
        data.lastUnlockedLevel = levelNumber;

    save(data);
}

bool SaveManager::isLevelSetCompleted(const QString& levelSetFile)
{
    int total = LevelLoader::levelCount(levelSetFile);
    if(total <= 0)
        return false;

    SaveData data = load(levelSetFile);
    return data.isCompleted(total);
}
