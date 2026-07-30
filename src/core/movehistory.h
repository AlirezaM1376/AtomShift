#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include <QVector>

class Board;

struct MoveRecord
{
    int fromX, fromY;
    int toX, toY;
};

/**
 * MoveHistory is reponsible for movement of atoms, undo/redo, keeping the undo/redo history and counting the movements.
 */
class MoveHistory
{
public:
    explicit MoveHistory(Board& board);

    /**
    * Moves the atom from (x,y) to direction (dx,dy).
    * In case of successful movement, saves the movement in undo history,
    * increases the movement counter and clears the redo history

    * @param newX is final column position after movement
    * @param newY is final raw position after movement
    * @return true in case of successful movement
    */
    bool moveAtom(int x, int y, int dx, int dy, int& newX, int& newY);

    /**
     * Undo the last movement (if there is any)
     * @return true in case of successful undo
     */
    bool undo();

    /**
     * Redo the last canceled movement(from undo).
     * @return true in case of successful redo.
     */
    bool redo();

    /**
     * Clears the undo/redo counter/history.
     * Should be called when loading a level.
     */
    void reset();

    int moveCount() const { return m_moveCount; }
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }

    /**
     * Without execution of real undo, shows info about the last movement that will undo after calling undo().
     * To show an animation before execution of real undo.
     * (The real undo will be executed only when the animation is finished)
     *
     * @return false if undo stack is empty
     */
    bool peekUndo(int& fromX, int& fromY, int& toX, int& toY) const;

    /**
     * Same as undo, but for redo.
     */
    bool peekRedo(int& fromX, int& fromY, int& toX, int& toY) const;

    /**
     * The position of atom based on last operation (move/undo/redo)
     * to show the cell as a selected cell.
     */
    int lastAtomX() const { return m_lastAtomX; }
    int lastAtomY() const { return m_lastAtomY; }

private:
    Board& m_board;
    int m_moveCount;
    int m_lastAtomX;
    int m_lastAtomY;

    QVector<MoveRecord> m_undoStack;
    QVector<MoveRecord> m_redoStack;
};

#endif // MOVEHISTORY_H
