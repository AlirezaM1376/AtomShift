#include <QDir>
#include <QFileInfoList>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFont>
#include <QFontMetrics>

#include "levelpackselectwidget.h"
#include "levelloader.h"
#include "savemanager.h"

//////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////

LevelPackSelectWidget::LevelPackSelectWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    m_backgroundPixmap.load("images/wood.png");
    m_levelButtonPixmap.load("images/btn_levelpack.png");
    m_backButtonPixmap.load("images/btn_mmenu.png");

    refreshLevelPackList();
}

//////////////////////////////////////////////////////////////////
// Load Level Packs
//////////////////////////////////////////////////////////////////

void LevelPackSelectWidget::refreshLevelPackList()
{
    m_buttons.clear();

    QDir levelsDir("levels");

    QStringList filters;
    filters << "*.dat";

    QFileInfoList files =
            levelsDir.entryInfoList(filters,
                                    QDir::Files,
                                    QDir::Name);

    for(int i=0;i<files.size();i++)
    {
        QString filePath = files.at(i).absoluteFilePath();

        LevelPackButton button;

        button.filePath = filePath;

        button.title = LevelLoader::levelSetName(filePath);

        if(button.title.isEmpty())
            button.title = files.at(i).baseName();

        button.totalLevels =
                LevelLoader::levelCount(filePath);

        SaveData save =
                SaveManager::load(filePath);

        button.solvedLevels =
                save.lastUnlockedLevel;

        button.locked = false;

        if(i>0)
        {
            QString previous =
                    files.at(i-1).absoluteFilePath();

            button.locked =
                    !SaveManager::isLevelSetCompleted(previous);
        }

        m_buttons.push_back(button);
    }

    calculateLayout();

    update();
}

//////////////////////////////////////////////////////////////////
// Calculate Widget Positions
//////////////////////////////////////////////////////////////////

void LevelPackSelectWidget::calculateLayout()
{
    int levelCount = m_buttons.size();

    int totalHeight = BACK_BUTTON_HEIGHT;

    if(levelCount > 0)
    {
        totalHeight +=
                levelCount * LEVEL_BUTTON_HEIGHT +
                (levelCount-1) * BUTTON_SPACING +
                BACK_TOP_SPACING;
    }

    int startY =
            (height()-totalHeight)/2 +13;

    int buttonX =
            (width()-LEVEL_BUTTON_WIDTH)/2;

    for(int i=0;i<m_buttons.size();i++)
    {
        m_buttons[i].rect =
                QRect(buttonX,
                      startY,
                      LEVEL_BUTTON_WIDTH,
                      LEVEL_BUTTON_HEIGHT);

        startY +=
                LEVEL_BUTTON_HEIGHT +
                BUTTON_SPACING-5;
    }

    startY +=
            BACK_TOP_SPACING -
            BUTTON_SPACING;

    int backX =
            (width()-BACK_BUTTON_WIDTH)/2;

    m_backRect =
            QRect(backX,
                  startY,
                  BACK_BUTTON_WIDTH,
                  BACK_BUTTON_HEIGHT);
}

//////////////////////////////////////////////////////////////////
// Paint
//////////////////////////////////////////////////////////////////

void LevelPackSelectWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    //////////////////////////////////////////////////////////////
    // Background
    //////////////////////////////////////////////////////////////

    if(!m_backgroundPixmap.isNull())
    {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    }
    else
    {
        painter.fillRect(rect(), QColor(104,78,53));
    }

    //////////////////////////////////////////////////////////////
    // Font
    //////////////////////////////////////////////////////////////

    QFont font = painter.font();
    font.setPixelSize(18);
    //font.setPointSize(10);
    font.setBold(true);

    painter.setFont(font);

    //////////////////////////////////////////////////////////////
    // Draw Level Pack Buttons
    //////////////////////////////////////////////////////////////

    for(int i=0;i<m_buttons.size();i++)
    {
        LevelPackButton &button = m_buttons[i];

        //////////////////////////////////////////////////////////
        // Draw Button
        //////////////////////////////////////////////////////////

        if(!m_levelButtonPixmap.isNull())
        {
            painter.drawPixmap(button.rect,
                               m_levelButtonPixmap);
        }
        else
        {
            QColor color(150,104,63);

            painter.setPen(Qt::NoPen);
            painter.setBrush(color);

            painter.drawRoundedRect(button.rect,
                                    10,
                                    10);
        }

        //////////////////////////////////////////////////////////
        // Text Color
        //////////////////////////////////////////////////////////

        if(button.locked)
            painter.setPen(QColor(165,165,165));
        else
            painter.setPen(QColor(230, 230, 230));

        //////////////////////////////////////////////////////////
        // Solved Levels
        //////////////////////////////////////////////////////////

        int solved = button.solvedLevels;

        if(solved < 0)
            solved = 0;

        if(solved > button.totalLevels)
            solved = button.totalLevels;

        QString progress =
                QString("%1/%2 Levels")
                .arg(solved)
                .arg(button.totalLevels);

        //////////////////////////////////////////////////////////
        // Title Rect
        //////////////////////////////////////////////////////////

        QRect titleRect =
                button.rect.adjusted(
                    TEXT_MARGIN,
                    0,
                    -PROGRESS_AREA_WIDTH,
                    0);

        //////////////////////////////////////////////////////////
        // Progress Rect
        //////////////////////////////////////////////////////////

        QRect progressRect =
                button.rect.adjusted(
                    PROGRESS_AREA_WIDTH,
                    0,
                    -TEXT_MARGIN,
                    0);

        //////////////////////////////////////////////////////////
        // Draw Title
        //////////////////////////////////////////////////////////

        painter.drawText(titleRect.left(), titleRect.top()-5, titleRect.width(), titleRect.height(),
                         Qt::AlignVCenter |
                         Qt::AlignLeft,
                         button.title);

        //////////////////////////////////////////////////////////
        // Draw Progress
        //////////////////////////////////////////////////////////

        painter.drawText(progressRect.left(), progressRect.top()-5, progressRect.width(), progressRect.height(),
                         Qt::AlignVCenter |
                         Qt::AlignRight,
                         progress);
    }

    //////////////////////////////////////////////////////////////
    // Draw Back Button
    //////////////////////////////////////////////////////////////

    if(!m_backButtonPixmap.isNull())
    {
        painter.drawPixmap(m_backRect,
                           m_backButtonPixmap);
    }
    else
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(130,90,55));

        painter.drawRoundedRect(m_backRect,
                                10,
                                10);
    }

    painter.setPen(Qt::white);

    painter.drawText(m_backRect.left(), m_backRect.top()-4, m_backRect.width(), m_backRect.height(), Qt::AlignCenter, "Back");
}

//////////////////////////////////////////////////////////////////
// Resize
//////////////////////////////////////////////////////////////////

void LevelPackSelectWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    calculateLayout();
}

//////////////////////////////////////////////////////////////////
// Mouse Press
//////////////////////////////////////////////////////////////////

void LevelPackSelectWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    //////////////////////////////////////////////////////////////
    // Back Button
    //////////////////////////////////////////////////////////////

    if(m_backRect.contains(pos))
    {
        emit backClicked();
        return;
    }

    //////////////////////////////////////////////////////////////
    // Level Packs
    //////////////////////////////////////////////////////////////

    for(int i=0;i<m_buttons.size();i++)
    {
        LevelPackButton &button = m_buttons[i];

        if(!button.rect.contains(pos))
            continue;

        if(button.locked)
            return;

        emit levelPackSelected(button.filePath);
        return;
    }

    QWidget::mousePressEvent(event);
}
