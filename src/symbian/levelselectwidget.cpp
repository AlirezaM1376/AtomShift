#include "levelselectwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <qmath.h>
#include "bond.h"

static const int SCREEN_W    = 640;
static const int SCREEN_H    = 360;
static const int PANEL_W     = 405;
static const int INFO_H      = 60;
static const int MOLECULE_H  = 223;
static const int BTN_W       = 200;
static const int BTN_H       = 51;
static const int BTN_GAP     = 0;  
static const int PANEL_X     = (SCREEN_W - PANEL_W) / 2; 

static const int TARGET_CELL_SIZE = 35;
static const int TARGET_ATOM_SIZE = 35;

LevelSelectWidget::LevelSelectWidget(QWidget *parent)
    : QWidget(parent), m_levelCount(1), m_currentIndex(1), m_maxAllowedLevel(1)
{
    setFixedSize(SCREEN_W, SCREEN_H);

    m_background = QPixmap("images/wood.png");
    m_moleculeBg = QPixmap("images/levelbg_levelselect.png");
    m_BtnsImg = QPixmap("images/btn_levelselect.png");
    m_infoBarImg = QPixmap("images/levelinfo_levelselect.png");

    m_isCurrentLevelLocked = false;

    int infoY     = 30; 
    int moleculeY = INFO_H+22;

    int btnsY     = INFO_H + MOLECULE_H + (SCREEN_H - INFO_H - MOLECULE_H - BTN_H) / 2;
    int btnsX     = (SCREEN_W - BTN_W * 2 - BTN_GAP) / 2;

    m_infoBarRect  = QRect(PANEL_X, infoY,     PANEL_W, INFO_H);
    m_moleculeRect = QRect(PANEL_X, moleculeY, PANEL_W, MOLECULE_H);

    m_prevRect = QRect(PANEL_X,              moleculeY, PANEL_W / 2 -50, MOLECULE_H-23);
    m_nextRect = QRect(PANEL_X + PANEL_W/2+50,  moleculeY, PANEL_W / 2 -50, MOLECULE_H-23);

    m_backBtnRect = QRect(btnsX+3,             btnsY-10, BTN_W, BTN_H);
    m_playBtnRect = QRect(btnsX-3 + BTN_W + BTN_GAP, btnsY-10, BTN_W, BTN_H);
}

void LevelSelectWidget::setup(const QString& levelSetFile)
{
    m_levelSetFile = levelSetFile;
    m_levelCount   = LevelLoader::levelCount(levelSetFile);
    if(m_levelCount <= 0) m_levelCount = 1;

    m_saveData = SaveManager::load(levelSetFile);

    m_maxAllowedLevel = qMin(m_saveData.lastUnlockedLevel + 1, m_levelCount);
    m_currentIndex    = m_maxAllowedLevel;

    updateDisplay();
}

void LevelSelectWidget::updateDisplay()
{

    LevelLoader::loadLevel(m_levelSetFile, m_currentIndex, m_currentLevelData);
    m_currentLevelName = m_currentLevelData.name;

    bool isLocked = (m_currentIndex > m_maxAllowedLevel);

    if(isLocked)
    {
        m_recordText = "Locked";
        m_isCurrentLevelLocked = true;
    }
    else if(m_saveData.levels.contains(m_currentIndex)
       && m_saveData.levels[m_currentIndex].solved)
    {
        const LevelSaveEntry& entry = m_saveData.levels[m_currentIndex];
        int min = entry.bestTimeSeconds / 60;
        int sec = entry.bestTimeSeconds % 60;
        m_recordText = QString("Best: %1 | %2:%3")
                .arg(entry.bestMoves)
                .arg(min)
                .arg(sec, 2, 10, QChar('0'));
        m_isCurrentLevelLocked = false;
    }
    else
    {
        m_recordText = "Best: -- | --";
        m_isCurrentLevelLocked = false;
    }

    update();
}


void LevelSelectWidget::drawMolecule(QPainter &p, bool grayscale)
{
    // bounding box
    int minX=15, maxX=-1, minY=15, maxY=-1;
    for(int y=0; y<15; y++)
        for(int x=0; x<15; x++)
            if(m_currentLevelData.molecule[x][y].atomId != 0)
            {
                if(x<minX) minX=x; if(x>maxX) maxX=x;
                if(y<minY) minY=y; if(y>maxY) maxY=y;
            }

    if(maxX < minX) return;

    int tcs = TARGET_CELL_SIZE;
    int tas = TARGET_ATOM_SIZE;

    int cols = maxX - minX + 1;
    int rows = maxY - minY + 1;

    if(rows * TARGET_CELL_SIZE > 200){
        tcs  = TARGET_CELL_SIZE-13;
        tas  = TARGET_ATOM_SIZE-13;
    }
    else{
        tcs  = TARGET_CELL_SIZE;
        tas  = TARGET_ATOM_SIZE;
    }

    int startX = m_moleculeRect.x() + (m_moleculeRect.width()  - cols*tcs)/2 - minX*tcs;
    int startY = m_moleculeRect.y() + (m_moleculeRect.height() - rows*tcs)/2 - minY*tcs;
    startY -=11;

    for(int y=0; y<15; y++)
        for(int x=0; x<15; x++)
        {
            LevelData::MoleculeCell cell = m_currentLevelData.molecule[x][y];
            if(cell.atomId == 0) continue;

            QVector<BondInfo> infos = decodeAllBonds(cell.bonds);
            for(int i=0; i<infos.size(); i++)
            {
                const BondInfo &info = infos.at(i);
                QPixmap px = BondImageCache::instance()
                        .bondPixmap(info.dx, info.dy, info.type);
                if(px.isNull() || px.width()<=1) continue;

                int cx = startX + x*tcs + tcs/2;
                int cy = startY + y*tcs + tcs/2;

                if(grayscale)
                {
                    QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
                    for(int iy = 0; iy < img.height(); iy++)
                    {
                        for(int ix = 0; ix < img.width(); ix++)
                        {
                            QRgb c = img.pixel(ix, iy);
                            int gray = qGray(c);
                            int alpha = qAlpha(c)/1.5;
                            img.setPixel(ix, iy, qRgba(gray, gray, gray, alpha));
                        }
                    }
                    p.drawPixmap(cx - px.width()/2, cy - px.height()/2,
                                 QPixmap::fromImage(img));
                }
                else{
                    QPixmap scaled = px.scaled(
                        tcs+2, tcs+2,
                        Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation);
                    p.drawPixmap(cx - scaled.width()/2, cy - scaled.height()/2, scaled);
                }
            }
        }

    // ---- connector ----
    for(int y=0; y<15; y++)
        for(int x=0; x<15; x++)
        {
            LevelData::MoleculeCell cell = m_currentLevelData.molecule[x][y];
            if(cell.atomId==0 || !isConnectorCode(cell.atomCode)) continue;

            int dx=0, dy=0;
            QVector<BondInfo> infos = decodeAllBonds(cell.bonds);
            if(!infos.isEmpty()) { dx=infos.first().dx; dy=infos.first().dy; }
            else {
                if(cell.atomCode=='A')      { dx=1; dy=0;  }
                else if(cell.atomCode=='B') { dx=1; dy=1;  }
                else if(cell.atomCode=='C') { dx=0; dy=1;  }
                else if(cell.atomCode=='D') { dx=1; dy=-1; }
            }

            QPixmap px = BondImageCache::instance().connectorPixmap(dx, dy);
            if(!px.isNull())
            {
                int cx = startX + x*tcs + tcs/2;
                int cy = startY + y*tcs + tcs/2;

                if(grayscale)
                {
                    QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
                    for(int iy = 0; iy < img.height(); iy++)
                    {
                        for(int ix = 0; ix < img.width(); ix++)
                        {
                            QRgb c = img.pixel(ix, iy);
                            int gray = qGray(c);
                            int alpha = qAlpha(c)/1.5;
                            img.setPixel(ix, iy, qRgba(gray, gray, gray, alpha));
                        }
                    }
                    p.drawPixmap(cx - px.width()/2, cy - px.height()/2,
                                 QPixmap::fromImage(img));
                }
                else
                    p.drawPixmap(cx - px.width()/2, cy - px.height()/2, px);
            }
        }

    for(int y=0; y<15; y++)
        for(int x=0; x<15; x++)
        {
            LevelData::MoleculeCell cell = m_currentLevelData.molecule[x][y];
            if(cell.atomId==0 || isConnectorCode(cell.atomCode)) continue;

            int atomLeft = startX + x*tcs + (tcs-tas)/2;
            int atomTop  = startY + y*tcs + (tcs-tas)/2;

            QPixmap px = AtomImageCache::instance().atomPixmap(cell.element);

            if(grayscale)
            {
                QImage img = px.toImage().convertToFormat(QImage::Format_ARGB32);
                for(int iy = 0; iy < img.height(); iy++)
                {
                    for(int ix = 0; ix < img.width(); ix++)
                    {
                        QRgb c = img.pixel(ix, iy);
                        int gray = qGray(c);
                        int alpha = qAlpha(c) / 1.5;
                        img.setPixel(ix, iy, qRgba(gray, gray, gray, alpha));
                    }
                }
                p.drawPixmap(QRect(atomLeft, atomTop, tas, tas),
                             QPixmap::fromImage(img));
            }
            else
                p.drawPixmap(QRect(atomLeft, atomTop, tas, tas), px);
        }
}


// ===================== PAINT =====================

void LevelSelectWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if(!m_background.isNull())
        p.drawPixmap(rect(), m_background);
    else
        p.fillRect(rect(), QColor(101, 67, 33));

    if(!m_infoBarImg.isNull())
        p.drawPixmap(m_infoBarRect, m_infoBarImg);
    else
    {
        p.fillRect(m_infoBarRect, QColor(60, 30, 10));
        QPen border(QColor(140, 90, 40));
        border.setWidth(1);
        p.setPen(border);
        p.drawRect(m_infoBarRect);
    }

    p.setPen(QColor(230, 230, 230));
    QFont f = p.font();
    f.setPixelSize(16);
    f.setBold(true);
    p.setFont(f);
    p.drawText(
        QRect(m_infoBarRect.x() + 28, m_infoBarRect.y()-5,
              m_infoBarRect.width() / 1.5, INFO_H),
        Qt::AlignVCenter | Qt::AlignLeft,
        QString("%1. %2").arg(m_currentIndex).arg(m_currentLevelName));

    f.setPixelSize(14);
    f.setBold(false);
    p.setFont(f);
    p.drawText(
        QRect(m_infoBarRect.x()-15 + m_infoBarRect.width() / 2,
              m_infoBarRect.y()-5,
              m_infoBarRect.width() / 2 - 12, INFO_H),
        Qt::AlignVCenter | Qt::AlignRight,
        m_recordText);

    if(!m_moleculeBg.isNull())
        p.drawPixmap(m_moleculeRect, m_moleculeBg);
    else
    {
        p.fillRect(m_moleculeRect, QColor(80, 50, 20));
        QPen border(QColor(140, 90, 40));
        border.setWidth(1);
        p.setPen(border);
        p.drawRect(m_moleculeRect);
    }

    drawMolecule(p, m_isCurrentLevelLocked);

    // Back
    if(!m_BtnsImg.isNull()){
        p.drawPixmap(m_backBtnRect, m_BtnsImg);
        p.setPen(QColor(230, 230, 230));
        QFont bf = p.font();
        bf.setPixelSize(18);
        bf.setBold(true);
        p.setFont(bf);
        p.drawText(m_backBtnRect.x(), m_backBtnRect.y()-5, m_backBtnRect.width(), m_backBtnRect.height(), Qt::AlignCenter, "Back");
    }
    else
    {
        p.fillRect(m_backBtnRect, QColor(60, 30, 10));
        QPen border(QColor(140, 90, 40));
        border.setWidth(1);
        p.setPen(border);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(m_backBtnRect, 8, 8);
        p.setPen(QColor(220, 180, 100));
        QFont bf = p.font();
        bf.setPixelSize(18);
        bf.setBold(true);
        p.setFont(bf);
        p.drawText(m_backBtnRect, Qt::AlignCenter, "Back");
    }

    // Play
    if(!m_BtnsImg.isNull()){
        p.drawPixmap(m_playBtnRect, m_BtnsImg);
        p.setPen(m_isCurrentLevelLocked
                     ? QColor(165,165,165)
                     : QColor(230, 230, 230));
        QFont bf = p.font();
        bf.setPixelSize(18);
        bf.setBold(true);
        p.setFont(bf);
        p.drawText(m_playBtnRect.x(), m_playBtnRect.y()-5, m_playBtnRect.width(), m_playBtnRect.height(), Qt::AlignCenter, "Play");
    }
    else
    {
        p.fillRect(m_playBtnRect, QColor(40, 80, 40));
        QPen border(QColor(80, 140, 80));
        border.setWidth(1);
        p.setPen(border);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(m_playBtnRect, 8, 8);
        p.setPen(QColor(180, 255, 180));
        QFont bf = p.font();
        bf.setPixelSize(18);
        bf.setBold(true);
        p.setFont(bf);
        p.drawText(m_playBtnRect, Qt::AlignCenter, "Play");
    }
}

// ===================== INPUT =====================

void LevelSelectWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    if(m_prevRect.contains(pos))
    {
        onPrevClicked();
        return;
    }

    if(m_nextRect.contains(pos))
    {
        onNextClicked();
        return;
    }

    if(m_backBtnRect.contains(pos))
    {
        emit backClicked();
        return;
    }

    if(m_playBtnRect.contains(pos))
    {
        onPlayClicked();
        return;
    }
}

void LevelSelectWidget::onPrevClicked()
{
    if(m_currentIndex > 1)
    {
        m_currentIndex--;
        updateDisplay();
    }
}

void LevelSelectWidget::onNextClicked()
{
    if(m_currentIndex < m_levelCount)
    {
        m_currentIndex++;
        updateDisplay();
    }
}

void LevelSelectWidget::onPlayClicked()
{
    if(m_currentIndex > m_maxAllowedLevel)
        return;
    emit levelSelected(m_currentIndex);
}
