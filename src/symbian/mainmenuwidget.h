#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>
#include <QRect>
#include <QPixmap>

class MainMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget *parent = 0);

signals:
    void startClicked();
    void aboutClicked();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    QRect m_playRect;
    QRect m_helpRect;
    QRect m_aboutRect;
    QRect m_exitRect;

    QPixmap m_background;
    QPixmap m_logo;
    QPixmap mmenu_btn;
    bool m_showAbout; 
    QPixmap m_aboutBg;
    QRect m_aboutRect_popup; 
    bool m_showHelp;
    QPixmap m_helpBg;
    QRect m_helpRect_popup;
};

#endif // MAINMENUWIDGET_H
