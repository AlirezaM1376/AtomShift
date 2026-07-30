#ifndef LEVELPACKSELECTWIDGET_H
#define LEVELPACKSELECTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QRect>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

class LevelPackSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LevelPackSelectWidget(QWidget *parent = 0);

    void refreshLevelPackList();

signals:
    void levelPackSelected(const QString &filePath);
    void backClicked();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:

    struct LevelPackButton
    {
        QString title;
        QString filePath;

        int solvedLevels;
        int totalLevels;

        bool locked;

        QRect rect;
    };

    void calculateLayout();

private:

    QVector<LevelPackButton> m_buttons;

    QRect m_backRect;

    QPixmap m_backgroundPixmap;
    QPixmap m_levelButtonPixmap;
    QPixmap m_backButtonPixmap;

    static const int WINDOW_WIDTH = 640;
    static const int WINDOW_HEIGHT = 360;

    static const int LEVEL_BUTTON_WIDTH = 341;
    static const int LEVEL_BUTTON_HEIGHT = 70;

    static const int BACK_BUTTON_WIDTH = 280;
    static const int BACK_BUTTON_HEIGHT = 53;

    static const int BUTTON_SPACING = 0;
    static const int BACK_TOP_SPACING = 18;
    static const int TEXT_MARGIN = 25;
    static const int PROGRESS_AREA_WIDTH = 120;
};

#endif // LEVELPACKSELECTWIDGET_H
