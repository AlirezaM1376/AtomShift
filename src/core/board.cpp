#include "board.h"
#include <cstring>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "leveldata.h"
#include "bond.h"

static QVector<ExpectedBond> buildExpectedBonds(
        int atomId,
        const QString& bondsStr,
        const LevelData& level)
{
    QVector<ExpectedBond> result;

    if(bondsStr.isEmpty())
        return result;

    int tx = -1;
    int ty = -1;
    for(int y = 0; y < 15 && tx < 0; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            if(level.molecule[x][y].atomId == atomId)
            {
                tx = x;
                ty = y;
                break;
            }
        }
    }

    QVector<BondInfo> infos = decodeAllBonds(bondsStr);

    for(int i = 0; i < infos.size(); i++)
    {
        ExpectedBond eb;
        eb.dx = infos.at(i).dx;
        eb.dy = infos.at(i).dy;
        eb.type = infos.at(i).type;
        eb.expectedNeighborAtomId = 0;

        if(tx >= 0)
        {
            int nx = tx + eb.dx;
            int ny = ty + eb.dy;
            if(nx >= 0 && nx < 15 && ny >= 0 && ny < 15)
            {
                eb.expectedNeighborAtomId = level.molecule[nx][ny].atomId;
            }
        }

        result.append(eb);
    }

    return result;
}

// ===================== Constructor =====================

Board::Board()
{
    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            m_board[x][y].atomId = 0;
            m_board[x][y].atomCode = 0;
            m_board[x][y].element = 0;
            m_board[x][y].bonds.clear();
            m_board[x][y].expectedBonds.clear();

            m_target[x][y].atomId = 0;
            m_target[x][y].atomCode = 0;
            m_target[x][y].element = 0;
            m_target[x][y].bonds.clear();
        }
    }
}

// ===================== Cell Access =====================

int Board::cell(int x, int y) const
{
    return m_board[x][y].atomId;
}

// ===================== Slide Helper =====================

static void slideToStop(
        const Board::Cell board[15][15],
        int x, int y,
        int dx, int dy,
        int &destX, int &destY)
{
    int cx = x;
    int cy = y;

    while(true)
    {
        int nx = cx + dx;
        int ny = cy + dy;

        if(nx < 0 || nx >= 15 || ny < 0 || ny >= 15)
            break;

        if(board[nx][ny].atomId != 0)
            break;

        cx = nx;
        cy = ny;
    }

    destX = cx;
    destY = cy;
}

// ===================== Movement =====================

bool Board::moveAtom(int x, int y,
                     int dx, int dy,
                     int &newX, int &newY)
{
    int value = m_board[x][y].atomId;

    if(value == 0 || value == WallCell)
        return false;

    int cx, cy;
    slideToStop(m_board, x, y, dx, dy, cx, cy);

    if(cx == x && cy == y)
        return false;

    Cell movingCell = m_board[x][y];

    m_board[x][y].atomId = 0;
    m_board[x][y].atomCode = 0;
    m_board[x][y].element = 0;
    m_board[x][y].bonds.clear();
    m_board[x][y].expectedBonds.clear();

    m_board[cx][cy] = movingCell;

    newX = cx;
    newY = cy;

    return true;
}

bool Board::computeDestination(
        int x, int y,
        int dx, int dy,
        int &destX, int &destY) const
{
    int value = m_board[x][y].atomId;

    if(value == 0 || value == WallCell)
        return false;

    int cx, cy;
    slideToStop(m_board, x, y, dx, dy, cx, cy);

    destX = cx;
    destY = cy;

    return !(cx == x && cy == y);
}

void Board::possibleMoves(int x, int y, PossibleMove outMoves[4]) const
{
    static const int dirs[4][2] = {
        { 0, -1 }, // DirUp
        { 0,  1 }, // DirDown
        { -1, 0 }, // DirLeft
        { 1,  0 }, // DirRight
    };
    static const Direction dirEnums[4] = {
        DirUp, DirDown, DirLeft, DirRight
    };

    for(int i = 0; i < 4; i++)
    {
        outMoves[i].dir = dirEnums[i];

        int destX, destY;
        bool ok = computeDestination(x, y, dirs[i][0], dirs[i][1], destX, destY);

        outMoves[i].possible = ok;
        outMoves[i].destX = ok ? destX : x;
        outMoves[i].destY = ok ? destY : y;
    }
}

// ===================== Direct Placement (for Undo) =====================

bool Board::placeAtomDirect(int fromX, int fromY, int toX, int toY)
{
    if(fromX < 0 || fromX >= 15 || fromY < 0 || fromY >= 15)
        return false;
    if(toX < 0 || toX >= 15 || toY < 0 || toY >= 15)
        return false;

    int value = m_board[fromX][fromY].atomId;
    if(value <= 0)
        return false;

    if(m_board[toX][toY].atomId != 0)
        return false;

    Cell movingCell = m_board[fromX][fromY];

    m_board[fromX][fromY].atomId = 0;
    m_board[fromX][fromY].atomCode = 0;
    m_board[fromX][fromY].element = 0;
    m_board[fromX][fromY].bonds.clear();
    m_board[fromX][fromY].expectedBonds.clear();

    m_board[toX][toY] = movingCell;

    return true;
}

// ===================== Bounding Box =====================

Board::BoundingBox Board::boundingBox() const
{
    BoundingBox bb;
    bb.minX = 15;
    bb.minY = 15;
    bb.maxX = -1;
    bb.maxY = -1;
    bb.valid = false;

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            int id = m_board[x][y].atomId;

            // ignoring empty cell (id==0)
            // atoms (id>0) and walls (id == Wallcell == -1) are involved in computing.
            if(id == 0)
                continue;

            bb.valid = true;
            if(x < bb.minX) bb.minX = x;
            if(y < bb.minY) bb.minY = y;
            if(x > bb.maxX) bb.maxX = x;
            if(y > bb.maxY) bb.maxY = y;
        }
    }

    return bb;
}

// ===================== Load Level =====================

bool Board::loadLevelData(const LevelData& level)
{
    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            int value = level.board[x][y];

            m_board[x][y].atomId = value;
            m_board[x][y].atomCode = 0;
            m_board[x][y].element = 0;
            m_board[x][y].bonds.clear();
            m_board[x][y].expectedBonds.clear();

            if(value > 0)
            {
                if(level.atomDefs.contains(value))
                {
                    m_board[x][y].atomCode = level.atomDefs[value].atomCode;
                    m_board[x][y].element  = level.atomDefs[value].element;
                    m_board[x][y].bonds    = level.atomDefs[value].bonds;

                    m_board[x][y].expectedBonds = buildExpectedBonds(
                            value,
                            level.atomDefs[value].bonds,
                            level);
                }
            }

            m_target[x][y].atomId   = level.molecule[x][y].atomId;
            m_target[x][y].atomCode = level.molecule[x][y].atomCode;
            m_target[x][y].element  = level.molecule[x][y].element;
            m_target[x][y].bonds    = level.molecule[x][y].bonds;
        }
    }

    return true;
}

// ===================== Solve Check =====================

bool Board::isSolved() const
{
    int minX = 16;
    int minY = 16;
    bool anyAtomFound = false;

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            if(m_board[x][y].atomId > 0)
            {
                anyAtomFound = true;
                if(x < minX) minX = x;
                if(y < minY) minY = y;
            }
        }
    }

    if(!anyAtomFound)
        return false;

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            int atomId = m_board[x][y].atomId;
            if(atomId <= 0)
                continue;

            int molecCoordX = x - minX;
            int molecCoordY = y - minY;

            if(molecCoordX < 0 || molecCoordX >= 15
                    || molecCoordY < 0 || molecCoordY >= 15)
                return false;

            if(m_target[molecCoordX][molecCoordY].atomId != atomId)
                return false;
        }
    }

    return true;
}
