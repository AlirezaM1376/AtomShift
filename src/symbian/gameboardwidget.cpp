#include <cstring>
#include <QPainter>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <qmath.h>
#include <QDateTime>

#include "gameboardwidget.h"
#include "levelloader.h"
#include "leveldata.h"
#include "board.h"
#include "bond.h"
#include "audiomanager.h"

// ======================= CONSTRUCTOR =======================

GameBoardWidget::GameBoardWidget(QWidget *parent)
    : QWidget(parent), m_moveHistory(m_board)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(WINDOW_W, WINDOW_H);
    BondImageCache::instance().load("images/bonds");
    AtomImageCache::instance().load("images");
    mBackground.load("images/wood.png");
    mBoardBackground.load("images/boardbg.png");
    mWallImg.load("images/wall.png");
    mInfoBG.load("images/infobg.png");
    mStateBG.load("images/statebg.png");
    mTargetBG.load("images/targetbg.png");
    mAtomSelectbtn.load("images/atomselectbtn.png");
    mTouchapad.load("images/touchpad.png");
    mUndobtn.load("images/undobtn.png");
    mRedobtn.load("images/redobtn.png");
    GameMenubtn.load("images/gamemenubtn.png");
    m_ingame_MenuBG.load("images/ingame_menubg.png");
    mbtnBG.load("images/btnbg.png");

    m_currentLevel = 1;
    if(m_levelCount <= 0)
        m_levelCount = 1;

    m_selectedX = -1;
    m_selectedY = -1;
    m_hasPossibleMoves = false;
    m_isAnimating = false;
    isUndoRedoActive = false;
    isTouchPadActive = false;
    isGameMenuActive = false;
    isMusicActive = true;
    m_animFromX = m_animFromY = m_animToX = m_animToY = 0;
    m_animDx = m_animDy = 0;
    m_animPurpose = AnimMove;

    m_cellSize     = 24;
    m_boardOffsetX = BOARD_AREA_X;
    m_boardOffsetY = BOARD_AREA_Y;

    m_displayUpdateTimer = new QTimer(this);
    connect(m_displayUpdateTimer, SIGNAL(timeout()), this, SLOT(onDisplayUpdateTick()));
    m_displayUpdateTimer->start(200);

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(25);
    connect(m_animTimer, SIGNAL(timeout()), this, SLOT(onAnimationFrame()));

    m_glowTimer = new QTimer(this);
    m_glowTimer->setInterval(30);
    connect(m_glowTimer, SIGNAL(timeout()), this, SLOT(onGlowTick()));
    m_glowStartTime = 0;
    loadLevel(m_currentLevel);
}

// ======================= LAYOUT METRICS =======================

void GameBoardWidget::computeLayoutMetrics()
{
    Board::BoundingBox bb = m_board.boundingBox();

    if(!bb.valid)
    {
        m_cellSize     = 24;
        m_boardOffsetX = BOARD_AREA_X;
        m_boardOffsetY = BOARD_AREA_Y;
        return;
    }

    int cols = bb.maxX - bb.minX + 1; 
    int rows = bb.maxY - bb.minY + 1; 

    int csW = BOARD_AREA_WIDTH  / cols;
    int csH = BOARD_AREA_HEIGHT / rows;
    int cs  = qMin(csW, csH);

    cs = qMax(cs, (int)MIN_CELL_SIZE);
    cs = qMin(cs, (int)MAX_CELL_SIZE);

    m_cellSize = cs;

    int usedW = cols * cs;
    int usedH = rows * cs;

    int areaStartX = BOARD_AREA_X + (BOARD_AREA_WIDTH  - usedW) / 2;
    int areaStartY = BOARD_AREA_Y + (BOARD_AREA_HEIGHT - usedH) / 2;

    m_boardOffsetX = areaStartX - bb.minX * cs;
    m_boardOffsetY = areaStartY - bb.minY * cs;
    buildAtomPositions();
}

bool GameBoardWidget::pixelToGrid(const QPoint &pos, int &gx, int &gy) const
{
    int x = (pos.x() - m_boardOffsetX) / m_cellSize;
    int y = (pos.y() - m_boardOffsetY) / m_cellSize;

    if(pos.x() < m_boardOffsetX || pos.y() < m_boardOffsetY)
        return false;

    if(x < 0 || x >= 15 || y < 0 || y >= 15)
        return false;

    gx = x;
    gy = y;
    return true;
}

// ======================= BOND DRAWING HELPERS =======================

struct MoleculeColumnRange
{
    int minX;
    int minY;
    int spanY;
    int span; 
};

static MoleculeColumnRange computeMoleculeColumnRange(const LevelData& level)
{
    int minX = 15, maxX = -1;
    int minY = 15, maxY = -1;

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            if(level.molecule[x][y].atomId != 0)
            {
                if(x < minX) minX = x;
                if(x > maxX) maxX = x;
                if(y < minY) minY = y;
                if(y > maxY) maxY = y;
            }
        }
    }

    MoleculeColumnRange range;
    if(maxX < minX)
    {
        range.minX = 0;
        range.span = 0;
    }
    else
    {
        range.minX = minX;
        range.span = maxX - minX + 1;
    }
    if(maxY < minY)
    {
        range.minY = 0;
        range.spanY = 0;
    }
    else
    {
        range.minY = minY;
        range.spanY = maxY - minY + 1;
    }
    return range;
}

static void drawMoleculeBonds(
        QPainter &p,
        int startX,
        int startY,
        int x,
        int y,
        const QString &bonds,
        int cellSize)
{
    if(bonds.isEmpty())
        return;

    QVector<BondInfo> infos = decodeAllBonds(bonds);

    for(int i = 0; i < infos.size(); i++)
    {
        const BondInfo &info = infos.at(i);

        int cx = startX + x * cellSize + cellSize / 2;
        int cy = startY + y * cellSize + cellSize / 2;

        const QPixmap &px = BondImageCache::instance()
                .bondPixmap(info.dx, info.dy, info.type);

        if(px.isNull() || px.width() == 1)
            continue;

        int drawX = cx - px.width()  / 2;
        int drawY = cy - px.height() / 2;

        p.drawPixmap(drawX, drawY, px);
    }
}

// ======================= POSSIBLE MOVES =======================

void GameBoardWidget::updatePossibleMoves()
{
    if(m_selectedX < 0 || m_selectedY < 0)
    {
        m_hasPossibleMoves = false;
        return;
    }
    m_board.possibleMoves(m_selectedX, m_selectedY, m_possibleMoves);
    m_hasPossibleMoves = true;

}

int GameBoardWidget::possibleMoveIndexAtPixel(const QPoint &pixelPos) const
{
    if(!m_hasPossibleMoves)
        return -1;

    int gx, gy;
    if(!const_cast<GameBoardWidget*>(this)->pixelToGrid(pixelPos, gx, gy))
        return -1;

    for(int i = 0; i < 4; i++)
    {
        if(!m_possibleMoves[i].possible)
            continue;
        if(m_possibleMoves[i].destX == gx && m_possibleMoves[i].destY == gy)
            return i;
    }
    return -1;
}

// ======================= TIME FORMAT =======================

static QString formatElapsedTime(qint64 ms)
{
    int totalSeconds = static_cast<int>(ms / 1000);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    return QString("%1:%2")
            .arg(minutes)
            .arg(seconds, 2, 10, QChar('0'));
}

// ======================= PAINT: BOARD =======================

void GameBoardWidget::paintBoard(QPainter &p)
{
    const int cs = m_cellSize;
    const int ox = m_boardOffsetX;
    const int oy = m_boardOffsetY;
	
    QRect boardRect(BOARD_AREA_X-19, BOARD_AREA_Y-6, BOARD_AREA_WIDTH+40, BOARD_AREA_HEIGHT+12);
    p.drawPixmap(boardRect, mBoardBackground);

    paintDestinationArrows(p);

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {

            if(m_isAnimating && x == m_animFromX && y == m_animFromY)
                continue;

            int value = m_board.cell(x, y);

            if(value == WallCell)
            {
                QRect wallRect(ox + x * cs + 2, oy + y * cs + 2, cs - 4, cs - 4);
                p.drawPixmap(wallRect, mWallImg);
            }
            else if(value > 0)
            {
                const Board::Cell &cell = m_board.boardCell(x, y);

                    if(!cell.bonds.isEmpty())
                    {
                        QVector<BondInfo> infos = decodeAllBonds(cell.bonds);

                        for(int i = 0; i < infos.size(); i++)
                        {
                            const BondInfo &info = infos.at(i);

                            QPixmap px = BondImageCache::instance()
                                    .bondPixmap(info.dx, info.dy, info.type);

                            if(px.isNull() || px.width() <= 1)
                                continue;

                            int cx = ox + x * cs + cs / 2;
                            int cy = oy + y * cs + cs / 2;

                            QPixmap scaled = getScaledBond(info.dx, info.dy, info.type);

                            p.drawPixmap(
                                cx - scaled.width()  / 2,
                                cy - scaled.height() / 2,
                                scaled);
                        }
                    }


                const QPixmap &px = AtomImageCache::instance().atomPixmap(cell.element);
                p.drawPixmap(QRect(ox + x * cs , oy + y * cs , cs, cs), px);
            }

            if(!m_isAnimating && x == m_selectedX && y == m_selectedY)
                paintAtomGlow(p, x, y);

        }
    }

    if(m_isAnimating)
    {
        const Board::Cell &cell = m_board.boardCell(m_animFromX, m_animFromY);
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_animStartTime;
        qreal t = qMin(qreal(1.0), (qreal)elapsed / m_animDuration);

        qreal startPxX = ox + m_animFromX * cs;
        qreal startPxY = oy + m_animFromY * cs;
        qreal endPxX   = ox + m_animToX   * cs;
        qreal endPxY   = oy + m_animToY   * cs;

        qreal curX = startPxX + (endPxX - startPxX) * t;
        qreal curY = startPxY + (endPxY - startPxY) * t;

        const QPixmap &px = AtomImageCache::instance().atomPixmap(cell.element);
        p.drawPixmap(QRect(qRound(curX) + -1.5, qRound(curY) -1.5, cs+3, cs+3), px);

    }
}

// ======================= PAINT: PANEL =======================

void GameBoardWidget::paintPanelContainer(QPainter &p)
{
    QRect infoRect(PANEL_X, PANEL_Y+1, PANEL_W, 45);
    p.drawPixmap(infoRect, mInfoBG);
    p.setPen(Qt::white);
    QFont test = p.font();
    test.setPixelSize(11);
    p.setFont(test);
    p.drawText(infoRect.left(), infoRect.top()+5, infoRect.width() , 15, Qt::AlignCenter,
               QString("Level %1 of %2 (%3)")
               .arg(m_currentLevel)
               .arg(m_levelCount).arg(m_levelSetName));

    p.setPen(Qt::white);
    QFont titleFont = p.font();
    titleFont.setPixelSize(15);
    titleFont.setBold(true);
    p.setFont(titleFont);
    p.drawText(infoRect.left(), infoRect.top() + 12, infoRect.width() , 30, Qt::AlignCenter, m_levelName);
    titleFont.setBold(false);
    p.setFont(titleFont);

    if(isGameMenuActive){
        paintGameMenuPanel(p);
        return;
    }

    QRect stateRect(PANEL_X, infoRect.bottom()+168, PANEL_W, 35);
    p.drawPixmap(stateRect, mStateBG);
    p.drawText(stateRect.left(), stateRect.top(), stateRect.width() , stateRect.height(), Qt::AlignCenter,
               QString("Moves: %1 | Time: %2")
               .arg(m_moveHistory.moveCount())
               .arg(formatElapsedTime(m_gameTimer.elapsedMs())));


    p.drawPixmap(QRect(infoRect.left(), infoRect.top()+46, infoRect.width(), 165), mTargetBG);

    int tcs;
    int tas;
    MoleculeColumnRange colRange = computeMoleculeColumnRange(m_level);
    int span = (colRange.span > 0) ? colRange.span : 1;
    int spanY = (colRange.spanY > 0) ? colRange.spanY : 1;

    if(spanY * TARGET_CELL_SIZE > 115){
        tcs  = TARGET_CELL_SIZE;
        tas  = TARGET_ATOM_SIZE;
    }
    else{
        tcs  = TARGET_CELL_SIZE+9;
        tas  = TARGET_ATOM_SIZE+10;
    }


    int moleculePixelWidth = span * tcs;
    int moleculePixelHeight = spanY * tcs;

    const int tmx = PANEL_X + (PANEL_W - moleculePixelWidth) / 2 - colRange.minX * tcs;
    const int tmy = infoRect.top()+ 46 + (165-moleculePixelHeight)/2 - colRange.minY * tcs;

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            LevelData::MoleculeCell cell = m_level.molecule[x][y];
            if(cell.atomId == 0)
                continue;
            drawMoleculeBonds(p, tmx, tmy, x, y, cell.bonds, tcs);
        }
    }

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            LevelData::MoleculeCell cell = m_level.molecule[x][y];
            if(cell.atomId == 0)
                continue;
            if(!isConnectorCode(cell.atomCode))
                continue;

            QVector<BondInfo> infos = decodeAllBonds(cell.bonds);
			
            int dx = 0, dy = 0;
            if(!infos.isEmpty())
            {
                dx = infos.first().dx;
                dy = infos.first().dy;
            }
            else
            {
				
                if(cell.atomCode == 'A') { dx=1; dy=0; }
                else if(cell.atomCode == 'B') { dx=1; dy=1; }
                else if(cell.atomCode == 'C') { dx=0; dy=1; }
                else if(cell.atomCode == 'D') { dx=1; dy=-1; }
            }

            const QPixmap &px = BondImageCache::instance()
                    .connectorPixmap(dx, dy);

            if(!px.isNull())
            {
                int cx = tmx + x * tcs + tcs / 2;
                int cy = tmy + y * tcs + tcs / 2;
                p.drawPixmap(cx - px.width()/2, cy - px.height()/2, px);
            }
        }
    }

    for(int y = 0; y < 15; y++)
    {
        for(int x = 0; x < 15; x++)
        {
            LevelData::MoleculeCell cell = m_level.molecule[x][y];
            if(cell.atomId == 0)
                continue;
            if(isConnectorCode(cell.atomCode))
                continue;

            int atomLeft = tmx + x * tcs + (tcs - tas) / 2;
            int atomTop  = tmy + y * tcs + (tcs - tas) / 2;
              const QPixmap &px = AtomImageCache::instance().atomPixmap(cell.element);
              p.drawPixmap(QRect(atomLeft, atomTop, tas, tas), px);
        }
    }

    if(!isTouchPadActive)
        paintButtons(p, false);
    else
        paintButtons(p, true);

}

// ======================= PAINT: GameMenu PANEL =======================

void GameBoardWidget::paintGameMenuPanel(QPainter &p)
{
    p.drawPixmap(QRect(PANEL_X, PANEL_Y+47, PANEL_W+1, 360-46), m_ingame_MenuBG);

    m_ingame_backRect  = QRect(PANEL_X+ PANEL_W/2- 80,           PANEL_Y+46+ (PANEL_H-46)/2 - 84,          160, 40);
    m_resetRect  = QRect(PANEL_X+ PANEL_W/2- 80 ,           PANEL_Y+46+ (PANEL_H-46)/2 - 84+44, 160, 40);
    m_ingame_soundRect = QRect(PANEL_X+ PANEL_W/2- 80,           PANEL_Y+46+ (PANEL_H-46)/2 - 84+89, 160, 40);
    m_ingame_menuRect = QRect(PANEL_X+ PANEL_W/2- 80, PANEL_Y+46+ (PANEL_H-46)/2 - 84+134, 160, 40);

    struct BtnInfo { QRect r; QString label;};
    BtnInfo btns[4] = {
        { m_ingame_backRect,  "Resume"},
        { m_resetRect,  "Restart"},
        { m_ingame_soundRect,  "Sound On/Off"},
        { m_ingame_menuRect, "Level Select"}
    };

    for(int i = 0; i < 4; i++)
    {
        p.drawPixmap(btns[i].r, mbtnBG);

        p.setPen(QColor(122,88,52));
        QFont f = p.font();
        f.setPixelSize(15);
        f.setBold(true);
        p.setFont(f);
        p.drawText(btns[i].r, Qt::AlignCenter, btns[i].label);
        f.setBold(false);
        p.setFont(f);
    }

}

// ======================= PAINT: Buttons =======================

void GameBoardWidget::paintButtons(QPainter &p, bool drawTouchPad)
{
    const int btnY0 = PANEL_Y + PANEL_H - 111;
    const int btnX0 = PANEL_X;
    const int btnH  = 36;
    const int btnGap = 3;

    m_atomSelectRect = QRect(btnX0, btnY0, 75,110);
    p.drawPixmap(m_atomSelectRect, mAtomSelectbtn);

    if(drawTouchPad){

        QRect padRect(
            m_atomSelectRect.right()+btnGap,
            btnY0,
            162,
            m_atomSelectRect.height());

        int w = padRect.width()/3;
        int h = padRect.height()/2;

        p.drawPixmap(padRect, mTouchapad);
        m_leftRect = QRect(
                    padRect.left()-3,
                    padRect.top()-4,
                    w+2,
                    padRect.height()+4);
        m_rightRect = QRect(
                      padRect.left()+(2*w),
                      padRect.top()-4,
                      w,
                      padRect.height()+4);
        m_upRect = QRect(
                    padRect.left()+w-1,
                    padRect.top()-4,
                    w+1,
                    h+2);
        m_downRect = QRect(
                    padRect.left()+w-1,
                    padRect.top()+h-2,
                    w+1,
                    h+2);

    } else{
		m_undoRect  = QRect(m_atomSelectRect.right()+btnGap,           btnY0,          162, 36);
        m_redoRect = QRect(m_atomSelectRect.right()+btnGap,           btnY0 + 37, 162, 36);
        m_ingame_menuRect2  = QRect(m_atomSelectRect.right()+btnGap, btnY0 + btnH + btnGap+35, 162, 36);
        p.drawPixmap(m_undoRect, mUndobtn);
        p.drawPixmap(m_redoRect, mRedobtn);
        p.drawPixmap(m_ingame_menuRect2, GameMenubtn);
    }

}

// ======================= PAINT EVENT (ENTRY POINT) =======================

void GameBoardWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QBrush(mBackground));
    paintPanelContainer(p);
    paintBoard(p);

}

// ======================= INPUT =======================

void GameBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if(m_isAnimating)
        return;

    QPoint pos = event->pos();

    if(isGameMenuActive){
        if(m_ingame_backRect.contains(pos)) {
            m_gameTimer.resume();
            m_isPaused = false;
            isGameMenuActive =false;
            update();
            return;
        }
        if(m_resetRect.contains(pos)) {
            loadLevel(m_currentLevel);
            return;
        }
        if(m_ingame_menuRect.contains(pos)) {
            emit BackToLevelPacks();//MainMenuClicked();
            return;
        }
        if(m_ingame_soundRect.contains(pos)) {
            AudioManager::instance().ToggleMusicPlaying(m_musicRanges, m_currentLevel);
            isMusicActive = isMusicActive ? false : true;
            return;
        }

    }
    else if(!isTouchPadActive){
        if(m_undoRect.contains(pos))  {
            isUndoRedoActive=true;
            startUndoAnimation();
            m_glowStartTime = QDateTime::currentMSecsSinceEpoch();
            m_glowTimer->start();
            return;
        }
        if(m_redoRect.contains(pos))  {
            isUndoRedoActive=true;
            startRedoAnimation();
            m_glowStartTime = QDateTime::currentMSecsSinceEpoch();
            m_glowTimer->start();
            return;
        }
        if(m_ingame_menuRect2.contains(pos)) {
            pauseGame();
        }
        if(m_atomSelectRect.contains(pos))
        {
            cycleSelectNextAtom();
            return;
        }
    }
    else{
        if(m_rightRect.contains(pos))
        {
            requestMove(1, 0);
            return;
        }
        if(m_leftRect.contains(pos))
        {
            requestMove(-1, 0);
            return;
        }
        if(m_upRect.contains(pos))
        {
            requestMove(0, -1);
            return;
        }
        if(m_downRect.contains(pos))
        {
            requestMove(0, 1);
            return;
        }

        if(m_atomSelectRect.contains(pos))
        {
            cycleSelectNextAtom();
            return;
        }
    }

    if(isGameMenuActive)
        return;

    int moveIdx = possibleMoveIndexAtPixel(pos);
    if(moveIdx >= 0)
    {
        int dx = 0, dy = 0;
        switch(m_possibleMoves[moveIdx].dir)
        {
        case DirUp:    dy = -1; break;
        case DirDown:  dy =  1; break;
        case DirLeft:  dx = -1; break;
        case DirRight: dx =  1; break;
        }
        requestMove(dx, dy);
        return;
    }

    int gx, gy;
    if(!pixelToGrid(pos, gx, gy))
    {

        isUndoRedoActive=false;
        isTouchPadActive=false;
        m_glowTimer->stop();
        m_glowStartTime = 0;
        m_selectedX = -1;
        m_selectedY = -1;
        m_hasPossibleMoves = false;
        update();
        return;
    }

    int value = m_board.cell(gx, gy);

    if(value == EmptyCell || value == WallCell)
    {
        isUndoRedoActive=false;
        isTouchPadActive=false;
        m_glowTimer->stop();
        m_glowStartTime = 0;
        m_selectedX = -1;
        m_selectedY = -1;
        m_hasPossibleMoves = false;
        update();
        return;
    }

    m_selectedX = gx;
    m_selectedY = gy;

    m_glowStartTime = QDateTime::currentMSecsSinceEpoch();
    m_glowTimer->start();
    isTouchPadActive= isUndoRedoActive ? false : true;
    if(!isTouchPadActive && isUndoRedoActive){
        isTouchPadActive= true;
        isUndoRedoActive= false;
    }
    updatePossibleMoves();
    update();
}

// ======================= KEY =======================

void GameBoardWidget::keyPressEvent(QKeyEvent *event)
{
    if(m_isAnimating)
        return;

    if(isGameMenuActive)
        return;

    switch(event->key())
    {
    case Qt::Key_Left:      requestMove(-1,  0); break;
    case Qt::Key_Right:     requestMove( 1,  0); break;
    case Qt::Key_Up:        requestMove( 0, -1); break;
    case Qt::Key_Down:      requestMove( 0,  1); break;
    case Qt::Key_Backspace: startUndoAnimation(); break;
    case Qt::Key_R:         startRedoAnimation(); break;
    case Qt::Key_Space:   cycleSelectNextAtom(); break;
    case Qt::Key_N:
        if(m_currentLevel < m_levelCount)
            loadLevel(++m_currentLevel);
        break;
    case Qt::Key_P:
        if(m_currentLevel > 1)
            loadLevel(--m_currentLevel);
        break;
    default:
        break;
    }
}

// ======================= MOVE ENTRY POINT =======================

void GameBoardWidget::requestMove(int dx, int dy)
{
    if(m_selectedX < 0 || m_selectedY < 0 || m_isAnimating)
        return;
    startMoveAnimation(dx, dy);
}

// ======================= ANIMATION =======================

void GameBoardWidget::startMoveAnimation(int dx, int dy)
{
    int destX, destY;
    if(!m_board.computeDestination(m_selectedX, m_selectedY, dx, dy, destX, destY))
        return;

    m_animDx = dx;
    m_animDy = dy;
    m_animPurpose = AnimMove;
    m_animFromX = m_selectedX;
    m_animFromY = m_selectedY;
    m_animToX   = destX;
    m_animToY   = destY;

    int numCells = qAbs(destX - m_animFromX) + qAbs(destY - m_animFromY);
    if(numCells <= 0) return;

    m_gameTimer.startOnFirstMove();
    m_isAnimating = true;
    m_hasPossibleMoves = false;

    const int msPerCell = 80;
    m_animDuration = numCells * msPerCell;
    m_animStartTime = QDateTime::currentMSecsSinceEpoch();
    m_animTimer->start();
}

void GameBoardWidget::startUndoAnimation()
{
    if(m_isAnimating) return;

    int fromX, fromY, toX, toY;
    if(!m_moveHistory.peekUndo(fromX, fromY, toX, toY))
        return;

    m_animFromX = toX;
    m_animFromY = toY;
    m_animToX   = fromX;
    m_animToY   = fromY;
    m_animPurpose = AnimUndo;

    int numCells = qAbs(m_animToX - m_animFromX) + qAbs(m_animToY - m_animFromY);
    if(numCells <= 0) return;

    m_gameTimer.startOnFirstMove();
    m_isAnimating = true;
    m_hasPossibleMoves = false;

    const int msPerCell = 80;
    m_animDuration = numCells * msPerCell;
    m_animStartTime = QDateTime::currentMSecsSinceEpoch();
    m_animTimer->start();
}

void GameBoardWidget::startRedoAnimation()
{
    if(m_isAnimating) return;

    int fromX, fromY, toX, toY;
    if(!m_moveHistory.peekRedo(fromX, fromY, toX, toY))
        return;

    m_animFromX = fromX;
    m_animFromY = fromY;
    m_animToX   = toX;
    m_animToY   = toY;
    m_animPurpose = AnimRedo;

    int numCells = qAbs(m_animToX - m_animFromX) + qAbs(m_animToY - m_animFromY);
    if(numCells <= 0) return;

    m_gameTimer.startOnFirstMove();
    m_isAnimating = true;
    m_hasPossibleMoves = false;

    const int msPerCell = 80;
    m_animDuration = numCells * msPerCell;
    m_animStartTime = QDateTime::currentMSecsSinceEpoch();
    m_animTimer->start();
}

void GameBoardWidget::onAnimationFrame()
{

    int ox = m_boardOffsetX;
    int oy = m_boardOffsetY;
    int cs = m_cellSize;

    int x1 = qMin(m_animFromX, m_animToX);
    int y1 = qMin(m_animFromY, m_animToY);
    int x2 = qMax(m_animFromX, m_animToX);
    int y2 = qMax(m_animFromY, m_animToY);

    QRect dirtyRect(
        ox + x1 * cs - cs,
        oy + y1 * cs - cs,
        (x2 - x1 + 3) * cs,
        (y2 - y1 + 3) * cs);

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_animStartTime;

    if(elapsed >= m_animDuration)
    {
        m_animTimer->stop();
        onAnimationFinished();
        return;
    }
    update(dirtyRect);
}

void GameBoardWidget::onDisplayUpdateTick()
{
    if(m_gameTimer.isStarted())
        update();
    if(m_glowStartTime > 0)
        update();
}

void GameBoardWidget::onAnimationFinished()
{
    m_isAnimating = false;
    bool success = false;

    if(m_animPurpose == AnimMove)
    {
        int newX, newY;
        success = m_moveHistory.moveAtom(
                m_animFromX, m_animFromY,
                m_animDx, m_animDy,
                newX, newY);
        if(success)
        {
            m_selectedX = newX;
            m_selectedY = newY;
        }
    }
    else if(m_animPurpose == AnimUndo)
    {
        success = m_moveHistory.undo();
        if(success)
        {
            m_selectedX = m_moveHistory.lastAtomX();
            m_selectedY = m_moveHistory.lastAtomY();
        }
    }
    else if(m_animPurpose == AnimRedo)
    {
        success = m_moveHistory.redo();
        if(success)
        {
            m_selectedX = m_moveHistory.lastAtomX();
            m_selectedY = m_moveHistory.lastAtomY();
        }
    }

    if(success)
    {
        updatePossibleMoves();
        buildAtomPositions();
        update();

        if(m_board.isSolved())
        {
            SaveManager::recordSolve(
                    m_saveData,
                    m_currentLevel,
                    m_moveHistory.moveCount(),
                    m_gameTimer.elapsedMs());

            int tempTime= m_gameTimer.elapsedMs();
            m_gameTimer.reset();

            QMessageBox::information(
                this,
                "Well done!",
                QString("Level solved in %1 moves and %2!")
                    .arg(m_moveHistory.moveCount())
                    .arg(formatElapsedTime(tempTime)));
            goToNextLevelOrFinish();
        }
    }
}

// ======================= LOAD =======================

void GameBoardWidget::loadLevel(int level)
{
    LevelLoader::loadLevel(m_levelSetFile, level, m_level);
    m_board.loadLevelData(m_level);
    m_levelName = m_level.name;

    m_selectedX = -1;
    m_selectedY = -1;
    m_hasPossibleMoves = false;
    m_isAnimating = false;
    m_animTimer->stop();
    m_scaledBondCache.clear();

    m_moveHistory.reset();
    m_gameTimer.reset();
    isTouchPadActive= false;
    isUndoRedoActive=false;
    isGameMenuActive =false;
    m_isPaused = false;

    computeLayoutMetrics();

    if(isMusicActive)
        AudioManager::instance().playGameMusic(m_musicRanges, level);
    else
        AudioManager::instance().stopGameMusic();

    update();
}

void GameBoardWidget::goToNextLevelOrFinish()
{
    if(m_currentLevel < m_levelCount)
    {
        loadLevel(++m_currentLevel);
    }
    else{
        QMessageBox::information(this, "Congratulations!", QString("You have completed the %1 level pack!").arg(m_levelSetName));
        emit BackToLevelPacks();
    }
}

void GameBoardWidget::startWithLevelSet(const QString& filePath)
{
    m_levelSetFile = filePath;
    m_levelCount = LevelLoader::levelCount(m_levelSetFile);
    m_levelSetName = LevelLoader::levelSetName(m_levelSetFile);
    if(m_levelCount <= 0)
        m_levelCount = 1;

    m_musicRanges = LevelLoader::loadMusicRanges(m_levelSetFile);
    m_saveData = SaveManager::load(m_levelSetFile);
}

void GameBoardWidget::buildAtomPositions()
{
    m_atomPositions.clear();
    m_atomCycleIndex = -1;

    for(int y = 0; y < 15; y++)
        for(int x = 0; x < 15; x++)
            if(m_board.cell(x, y) > 0)
                m_atomPositions.append(QPoint(x, y));
}

void GameBoardWidget::cycleSelectNextAtom()
{
    if(m_atomPositions.isEmpty())
        return;

    m_atomCycleIndex = (m_atomCycleIndex + 1) % m_atomPositions.size();

    QPoint pos = m_atomPositions.at(m_atomCycleIndex);
    m_selectedX = pos.x();
    m_selectedY = pos.y();

    m_glowStartTime = QDateTime::currentMSecsSinceEpoch();
    m_glowTimer->start();
    updatePossibleMoves();
    isTouchPadActive = true;
    update();
}

void GameBoardWidget::goToLevel(int levelNumber)
{
    m_currentLevel = levelNumber;
    loadLevel(m_currentLevel);
}

void GameBoardWidget::paintAtomGlow(QPainter &p, int x, int y)
{
    if(m_glowStartTime == 0) return;

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_glowStartTime;
    qreal phase = (elapsed % 1000) / 1000.0; 
    qreal pulse = 0.7 + 0.3 * qSin(phase * 2.0 * 3.14159);

    const int cs = m_cellSize;
    const int ox = m_boardOffsetX;
    const int oy = m_boardOffsetY;

    int cx = ox + x * cs + cs / 2;
    int cy = oy + y * cs + cs / 2;

    struct GlowLayer { int expand; int alpha; };
    GlowLayer layers[] = {
        { cs * 1 / 4, 70  },
        { cs * 1 / 8, 120 },
    };

    for(int i = 0; i < 2; i++)
    {
        int e = (int)(layers[i].expand * pulse);
        int a;
        a = (int)(layers[i].alpha * pulse);

        QColor glowColor(200, 230, 255, a);

        p.setPen(Qt::NoPen);
        p.setBrush(glowColor);
        p.drawRoundedRect(
            cx - cs/2 - e,
            cy - cs/2 - e,
            cs + e*2,
            cs + e*2,
            6, 6);
    }
}

void GameBoardWidget::onGlowTick()
{
    if(m_glowStartTime > 0)
        update();
}

void GameBoardWidget::paintDestinationArrows(QPainter &p)
{
    if(!m_hasPossibleMoves || m_isAnimating)
        return;
    if(m_glowStartTime == 0) return;

    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_glowStartTime;
    qreal phase = (elapsed % 1000) / 1000.0;
    qreal pulse = 0.5 + 0.5 * qSin(phase * 2.0 * 3.14159);

    const int cs = m_cellSize;
    const int ox = m_boardOffsetX;
    const int oy = m_boardOffsetY;
	
    int alpha;
	alpha = (int)(120 + 135 * pulse); 

    for(int i = 0; i < 4; i++)
    {
        if(!m_possibleMoves[i].possible)
            continue;

        int dx = 0, dy = 0;
        switch(m_possibleMoves[i].dir)
        {
        case DirUp:    dx =  0; dy = -1; break;
        case DirDown:  dx =  0; dy =  1; break;
        case DirLeft:  dx = -1; dy =  0; break;
        case DirRight: dx =  1; dy =  0; break;
        }

        int hx = m_possibleMoves[i].destX;
        int hy = m_possibleMoves[i].destY;

        int cx = ox + hx * cs + (cs) / 2;
        int cy = oy + hy * cs + (cs) / 2;

        int arrowSize = cs * 1/2;

        QPoint tip, left, right;

        if(dx == 0 && dy == -1) 
        {
            tip   = QPoint(cx,               cy - arrowSize+3);
            left  = QPoint(cx - arrowSize/2, cy + arrowSize/3+3);
            right = QPoint(cx + arrowSize/2, cy + arrowSize/3+3);
        }
        else if(dx == 0 && dy == 1) 
        {
            tip   = QPoint(cx,               cy + arrowSize-3);
            left  = QPoint(cx - arrowSize/2, cy - arrowSize/3-3);
            right = QPoint(cx + arrowSize/2, cy - arrowSize/3-3);
        }
        else if(dx == -1 && dy == 0)
        {
            tip   = QPoint(cx - arrowSize+3,   cy);
            left  = QPoint(cx + arrowSize/3+3, cy - arrowSize/2);
            right = QPoint(cx + arrowSize/3+3, cy + arrowSize/2);
        }
        else
        {
            tip   = QPoint(cx + arrowSize-3,   cy);
            left  = QPoint(cx - arrowSize/3-3, cy - arrowSize/2);
            right = QPoint(cx - arrowSize/3-3, cy + arrowSize/2);
        }

        QPolygon arrow;
        arrow << tip << left << right;

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 200, 50, alpha));
        p.drawPolygon(arrow);
    }
}

QPixmap GameBoardWidget::getScaledBond(int dx, int dy, BondType type)
{
    QString key = QString("%1_%2_%3_%4")
            .arg(dx).arg(dy).arg((int)type).arg(m_cellSize);

    if(m_scaledBondCache.contains(key))
        return m_scaledBondCache[key];

    QPixmap px = BondImageCache::instance().bondPixmap(dx, dy, type);
    if(px.isNull() || px.width() <= 1)
        return px;

    QPixmap scaled = px.scaled(
        m_cellSize+3, m_cellSize+3,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation);

    m_scaledBondCache[key] = scaled;
    return scaled;
}

void GameBoardWidget::pauseGame()
{
    m_gameTimer.pause();
    m_isPaused = true;
    isGameMenuActive =true;
    update();
    return;
}
