#ifndef LEVELDATA_H
#define LEVELDATA_H

#include <QString>
#include <QVector>
#include <QMap>

struct AtomDefinition
{
    int atomId;

    char atomCode;

    QString displayText;

    QString bonds;

    char element;   // H,O,C,...
};

struct LevelData
{
    QString name;

    int board[15][15];

    struct MoleculeCell
    {
        int atomId;

        char atomCode;

        QString bonds;

        char element;
    };

    MoleculeCell molecule[15][15];

    QMap<int, AtomDefinition> atomDefs;
};

struct MusicRange
{
    int     startLevel;
    int     endLevel;
    QString fileName;
};

#endif
