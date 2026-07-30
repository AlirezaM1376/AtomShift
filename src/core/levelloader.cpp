#include "atom.h"
#include "levelloader.h"

#include <QSettings>
#include <QStringList>

bool LevelLoader::loadLevel(
        const QString& fileName,
        int levelNumber,
        LevelData& level)
{
    QSettings ini(fileName, QSettings::IniFormat);

    QString group =
            QString("Level%1")
            .arg(levelNumber);

    level.name =
            ini.value(group + "/Name")
            .toStringList()
            .join(",");

    // ===================== RESET =====================

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            level.board[x][y] = 0;

            level.molecule[x][y].atomId = 0;
            level.molecule[x][y].atomCode = 0;
            level.molecule[x][y].bonds.clear();
            level.molecule[x][y].element = 0;
        }
    }

    // ===================== LOAD BOARD =====================

    for(int y = 0; y < 15; y++)
    {
        QString key =
                QString("%1/feld_%2")
                .arg(group)
                .arg(y,2,10,QChar('0'));

        QString line =
                ini.value(key).toString();

        for(int x = 0;
            x < 15 && x < line.length();
            x++)
        {
            QChar c = line[x];

            if(c == '.')
            {
                level.board[x][y] = 0;
            }
            else if(c == '#')
            {
                level.board[x][y] = -1;
            }
            else
            {
                level.board[x][y] =
                        atom2int(
                            c.toLatin1()
                        );
            }
        }
    }

    // ===================== LOAD ATOM DEFINITIONS =====================

    level.atomDefs.clear();
    for(int i = 1; i <= 35; i++)
    {
        QString key =
            QString("%1/atom_%2")
            .arg(group)
            .arg(int2atom(i));

        QString value =
            ini.value(key).toString();

        if(value.isEmpty())
            break;

        AtomDefinition def;

        def.atomId = i;

        // example:
        // 1-c
        // 3-cg
        // 2-cgA

        def.atomCode = value[0].toLatin1();

        switch(def.atomCode)
        {
        case '1': def.element='H'; break;
        case '2': def.element='C'; break;
        case '3': def.element='O'; break;
        case '4': def.element='N'; break;
        case '5': def.element='S'; break;
        case '6': def.element='F'; break;
        case '7': def.element='L'; break; // Cl
        case '8': def.element='B'; break; // Br
        case '9': def.element='P'; break;

        case 'o': def.element='X'; break; // Crystal

        case 'A': def.element='A'; break;
        case 'B': def.element='9'; break;
        case 'C': def.element='0'; break;
        case 'D': def.element='D'; break;

        case 'E': def.element='1'; break;
        case 'F': def.element='2'; break;
        case 'G': def.element='3'; break;
        case 'H': def.element='4'; break;
        case 'I': def.element='5'; break;
        case 'J': def.element='6'; break;
        case 'K': def.element='7'; break;
        case 'L': def.element='8'; break;

        default:
            def.element='?';
        }

        int dashPos = value.indexOf('-');

        if(dashPos >= 0)
            def.bonds = value.mid(dashPos + 1);

        level.atomDefs[i] = def;
    }

    // ===================== LOAD MOLECULE =====================

    for(int y = 0; y < 15; y++)
    {
        QString key =
                QString("%1/mole_%2")
                .arg(group)
                .arg(y);

        QString line =
                ini.value(key).toString();

        for(int x = 0;
            x < 15 && x < line.length();
            x++)
        {
            QChar c = line[x];

            if(c == '.')
            {
                level.molecule[x][y].atomId = 0;
                level.molecule[x][y].atomCode = 0;
            }
            else
            {
                int idx = atom2int(c.toLatin1());

                level.molecule[x][y].atomId = idx;

                level.molecule[x][y].atomCode =
                    level.atomDefs.value(idx).atomCode;

                level.molecule[x][y].bonds =
                    level.atomDefs.value(idx).bonds;

                level.molecule[x][y].element =
                    level.atomDefs.value(idx).element;
            }
        }
    }

    return true;
}

int LevelLoader::levelCount(const QString& fileName)
{
    QSettings ini(fileName, QSettings::IniFormat);

    // Returns 0 in case of no LevelCount data
    return ini.value("LevelSet/LevelCount", 0).toInt();
}

QString LevelLoader::levelSetName(const QString& fileName)
{
    QSettings ini(fileName, QSettings::IniFormat);
    return ini.value("LevelSet/Name").toStringList().join(",");
}


static QVector<MusicRange> parseMusicRanges(const QString& musicStr)
{
    QVector<MusicRange> result;
    if(musicStr.isEmpty()) return result;

    QStringList items = musicStr.split(',');

    for(int i = 0; i < items.size(); i++)
    {
        QString item = items.at(i).trimmed();
        // Format: startLevel-endLevel-filename.mp3
        // example: 1-15-ingame1.mp3

        // finding 2 numbers first
        int firstDash  = item.indexOf('-');
        if(firstDash < 0) continue;

        int secondDash = item.indexOf('-', firstDash + 1);
        if(secondDash < 0) continue;

        MusicRange range;
        range.startLevel = item.left(firstDash).toInt();
        range.endLevel   = item.mid(firstDash + 1, secondDash - firstDash - 1).toInt();
        range.fileName   = item.mid(secondDash + 1).trimmed();

        if(range.startLevel <= 0 || range.endLevel <= 0
           || range.fileName.isEmpty())
            continue;

        result.append(range);
    }
    return result;
}


QVector<MusicRange> LevelLoader::loadMusicRanges(const QString& fileName)
{
    QSettings ini(fileName, QSettings::IniFormat);
    QString musicStr = ini.value("LevelSet/Music").toStringList().join(",");
    return parseMusicRanges(musicStr);
}
