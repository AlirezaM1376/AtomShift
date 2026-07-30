#ifndef GAMEBOARDWIDGET_H
#define GAMEBOARDWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRect>
#include <QTimeLine>
#include <QTimer>
#include <QPixmap>

#include "board.h"
#include "leveldata.h"
#include "movehistory.h"
#include "gametimer.h"
#include "savedata.h"
#include "savemanager.h"
#include "bondimagecache.h"
#include "atomimagecache.h"

class GameBoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameBoardWidget(QWidget *parent = 0);

    void startWithLevelSet(const QString& filePath);
    void goToLevel(int levelNumber);
    void pauseGame();
    const QString& levelSetFile() const { return m_levelSetFile; }

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void loadLevel(int level);

private slots:
    void onAnimationFrame();
    void onAnimationFinished();
    void onDisplayUpdateTick();

private:
    Board       m_board;
    MoveHistory m_moveHistory;
    GameTimer   m_gameTimer;
    QTimer     *m_displayUpdateTimer;

    int m_selectedX;
    int m_selectedY;

    QString  m_levelName;
    QString m_levelSetName;
    LevelData m_level;
    int      m_currentLevel;
    int      m_levelCount;
    QString  m_levelSetFile;
    SaveData m_saveData;
    QPixmap mBackground;
    QPixmap mBoardBackground;
    QPixmap mWallImg;
    QPixmap mInfoBG;
    QPixmap mTargetBG;
    QPixmap mStateBG;
    QPixmap mAtomSelectbtn;
    QPixmap mTouchapad;
    QPixmap mUndobtn;
    QPixmap mRedobtn;
    QPixmap GameMenubtn;
    QPixmap m_ingame_MenuBG;
    QPixmap mbtnBG;

    // Symbian 640x360
    enum {
        WINDOW_W        = 640,
        WINDOW_H        = 360,
        BOARD_AREA_X    = 20,
        BOARD_AREA_Y    = 8,
        BOARD_AREA_WIDTH  = 355,
        BOARD_AREA_HEIGHT = 345,
        PANEL_X         = 400,
        PANEL_Y         = 0,
        PANEL_W         = 239,
        PANEL_H         = 360,
        MAX_CELL_SIZE   = 40,
        MIN_CELL_SIZE   = 22,
        TARGET_CELL_SIZE = 20,
        TARGET_ATOM_SIZE = 20
    };

    int m_cellSize; 
    int m_boardOffsetX; 
    int m_boardOffsetY;

    QVector<QPoint> m_atomPositions; 
    int m_atomCycleIndex;        

    void buildAtomPositions();     
    void cycleSelectNextAtom();  

    void computeLayoutMetrics();

    bool pixelToGrid(const QPoint &pos, int &gx, int &gy) const;

    void paintLevelInfo(QPainter &p);

    void paintBoard(QPainter &p);

    void paintPanelContainer(QPainter &p);
    void paintGameMenuPanel(QPainter &p);
    void paintButtons(QPainter &p, bool drawTouchPad);

    QRect m_undoRect;
    QRect m_redoRect;
    QRect m_resetRect;
    QRect m_nextRect;
    QRect m_ingame_soundRect;
    QRect m_ingame_backRect;
    QRect m_ingame_menuRect;
    QRect m_ingame_menuRect2;

    QRect m_rightRect;
    QRect m_leftRect;
    QRect m_upRect;
    QRect m_downRect;
    QRect m_atomSelectRect;

    PossibleMove m_possibleMoves[4];
    bool m_hasPossibleMoves;
    int  possibleMoveIndexAtPixel(const QPoint &pixelPos) const;
    void updatePossibleMoves();

    void startMoveAnimation(int dx, int dy);
    void startUndoAnimation();
    void startRedoAnimation();
    void requestMove(int dx, int dy);
    void goToNextLevelOrFinish();

    enum AnimPurpose { AnimMove, AnimUndo, AnimRedo };

    QTimer  *m_animTimer;
    qint64   m_animStartTime;
    int      m_animDuration;
    int         m_animFromX, m_animFromY;
    int         m_animToX,   m_animToY;
    int         m_animDx,    m_animDy;
    AnimPurpose m_animPurpose;
    bool        m_isAnimating;
    bool        isUndoRedoActive;
    bool        isTouchPadActive;
    bool        isGameMenuActive;
    bool        isMusicActive;

    void paintAtomGlow(QPainter &p, int x, int y);
    QTimer *m_glowTimer;
    qint64 m_glowStartTime;
    void paintDestinationArrows(QPainter &p);
    bool m_isPaused;
    QMap<QString, QPixmap> m_scaledBondCache;
    QPixmap getScaledBond(int dx, int dy, BondType type);
    QVector<MusicRange> m_musicRanges;

signals:
    void BackToLevelPacks();

public slots:

private slots:
    void onGlowTick();
};

#endif // GAMEBOARDWIDGET_H
