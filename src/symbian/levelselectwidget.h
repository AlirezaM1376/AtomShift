#ifndef LEVELSELECTWIDGET_H
#define LEVELSELECTWIDGET_H

#include <QWidget>
#include <QRect>
#include <QPixmap>
#include "savedata.h"
#include "savemanager.h"
#include "levelloader.h"
#include "leveldata.h"
#include "bondimagecache.h"
#include "atomimagecache.h"
#include "bond.h"

class LevelSelectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LevelSelectWidget(QWidget *parent = 0);

    const QString& levelSetFile() const { return m_levelSetFile; }
    void setup(const QString& levelSetFile);

signals:
    void levelSelected(int levelNumber);
    void backClicked();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    void onPrevClicked();
    void onNextClicked();
    void onPlayClicked();

private:
    QString  m_levelSetFile;
    int      m_levelCount;
    int      m_currentIndex;
    int      m_maxAllowedLevel;
    SaveData m_saveData;
    bool m_isCurrentLevelLocked;

    LevelData m_currentLevelData;
    QString   m_currentLevelName;
    QString   m_recordText;

    QPixmap m_background; 
    QPixmap m_moleculeBg;  
    QPixmap m_BtnsImg;  
    QPixmap m_infoBarImg;

    QRect m_infoBarRect;   
    QRect m_moleculeRect;  
    QRect m_prevRect;      
    QRect m_nextRect;    
    QRect m_backBtnRect;  
    QRect m_playBtnRect; 

    void updateDisplay();
    void drawMolecule(QPainter &p, bool grayscale = false);
    void drawMoleculeBonds(QPainter &p, int startX, int startY,
                           int x, int y, const QString &bonds,
                           int cellSize, int atomSize);
};

#endif
