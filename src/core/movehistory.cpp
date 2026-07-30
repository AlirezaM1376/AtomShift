#include "movehistory.h"
#include "board.h"

MoveHistory::MoveHistory(Board& board)
    : m_board(board), m_moveCount(0), m_lastAtomX(-1), m_lastAtomY(-1)
{
}

bool MoveHistory::moveAtom(int x, int y, int dx, int dy, int& newX, int& newY)
{
    if(!m_board.moveAtom(x, y, dx, dy, newX, newY))
        return false;

    MoveRecord rec;
    rec.fromX = x;
    rec.fromY = y;
    rec.toX = newX;
    rec.toY = newY;
    m_undoStack.append(rec);

    m_redoStack.clear();

    m_moveCount++;
    m_lastAtomX = newX;
    m_lastAtomY = newY;

    return true;
}

bool MoveHistory::undo()
{
    if(m_undoStack.isEmpty())
        return false;

    MoveRecord rec = m_undoStack.last();
    m_undoStack.remove(m_undoStack.size() - 1);

    if(!m_board.placeAtomDirect(rec.toX, rec.toY, rec.fromX, rec.fromY))
    {
        m_undoStack.append(rec);
        return false;
    }

    m_redoStack.append(rec);

    m_lastAtomX = rec.fromX;
    m_lastAtomY = rec.fromY;

    if(m_moveCount > 0)
        m_moveCount--;

    return true;
}

bool MoveHistory::redo()
{
    if(m_redoStack.isEmpty())
        return false;

    MoveRecord rec = m_redoStack.last();
    m_redoStack.remove(m_redoStack.size() - 1);

    if(!m_board.placeAtomDirect(rec.fromX, rec.fromY, rec.toX, rec.toY))
    {
        m_redoStack.append(rec);
        return false;
    }

    m_undoStack.append(rec);

    m_lastAtomX = rec.toX;
    m_lastAtomY = rec.toY;

    m_moveCount++;

    return true;
}

void MoveHistory::reset()
{
    m_moveCount = 0;
    m_lastAtomX = -1;
    m_lastAtomY = -1;
    m_undoStack.clear();
    m_redoStack.clear();
}

bool MoveHistory::peekUndo(int& fromX, int& fromY, int& toX, int& toY) const
{
    if(m_undoStack.isEmpty())
        return false;

    const MoveRecord& rec = m_undoStack.last();
    fromX = rec.fromX;
    fromY = rec.fromY;
    toX = rec.toX;
    toY = rec.toY;
    return true;
}

bool MoveHistory::peekRedo(int& fromX, int& fromY, int& toX, int& toY) const
{
    if(m_redoStack.isEmpty())
        return false;

    const MoveRecord& rec = m_redoStack.last();
    fromX = rec.fromX;
    fromY = rec.fromY;
    toX = rec.toX;
    toY = rec.toY;
    return true;
}
