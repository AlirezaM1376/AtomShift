#ifndef BOARD_H
#define BOARD_H

#include <QString>
#include <QColor>
#include <QVector>
#include "bond.h"

class LevelData;

enum CellType
{
    EmptyCell = 0,
    WallCell  = -1
};

enum Direction
{
    DirUp = 0,
    DirDown,
    DirLeft,
    DirRight
};

struct PossibleMove
{
    Direction dir;
    bool possible;
    int destX;
    int destY;
};

struct ExpectedBond
{
    int dx;
    int dy;
    BondType type;
    int expectedNeighborAtomId;
};

class Board
{
public:
    Board();

    int cell(int x, int y) const;

    bool moveAtom(
            int x,
            int y,
            int dx,
            int dy,
            int &newX,
            int &newY);

    bool computeDestination(
            int x,
            int y,
            int dx,
            int dy,
            int &destX,
            int &destY) const;

    void possibleMoves(int x, int y, PossibleMove outMoves[4]) const;

    bool placeAtomDirect(int fromX, int fromY, int toX, int toY);

    bool loadLevelData(const LevelData& level);

    bool loadFromFile(const QString &path);

    bool isSolved() const;

    /**
     * Returns the smallest possible rect that covers all walls and atoms.
     * Will be used to compute the cell size dynamically (for the GUI)
     */
    struct BoundingBox
    {
        int minX, minY;
        int maxX, maxY;
        bool valid; //false if board is empty
    };

    BoundingBox boundingBox() const;

    struct Cell
    {
        int atomId;
        char atomCode;
        char element;
        QString bonds;
        QVector<ExpectedBond> expectedBonds;
    };

    const Cell& boardCell(int x, int y) const { return m_board[x][y]; }

private:

    Cell m_board[15][15];

    struct TargetCell
    {
        int atomId;
        char atomCode;
        char element;
        QString bonds;
    };

    TargetCell m_target[15][15];

public:
    const TargetCell& targetCell(int x, int y) const { return m_target[x][y]; }
};

#endif
