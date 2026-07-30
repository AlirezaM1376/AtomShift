#include "mainmenuwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>


MainMenuWidget::MainMenuWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(640, 360);

    m_background = QPixmap("images/mmenu_bg.png");
    m_logo       = QPixmap("images/logo_mmenu.png");
    mmenu_btn       = QPixmap("images/btn_mmenu.png");

    const int btnW   = 280;
    const int btnH   = 53;
    const int btnX   = (640 - btnW) / 2; 

    const int startY = 108;

    m_playRect  = QRect(btnX, startY,                        btnW, btnH);
    m_helpRect  = QRect(btnX, startY + (btnH-4),      btnW, btnH);
    m_aboutRect = QRect(btnX, startY + (btnH-4) * 2,  btnW, btnH);
    m_exitRect  = QRect(btnX, startY + (btnH-4) * 3,  btnW, btnH);

    m_showAbout = false;
    m_aboutBg = QPixmap("images/about_bg.png");

    const int popW = 380;
    const int popH = 315;
    m_aboutRect_popup = QRect(
        (640 - popW) / 2,
        (360 - popH) / 2,
        popW, popH);

    m_showHelp = false;
    m_helpBg = QPixmap("images/help_bg.png");

    const int helpW = 390;
    const int helpH = 340;
    m_helpRect_popup = QRect(
        (640 - helpW) / 2,
        (360 - helpH) / 2,
        helpW, helpH);
}

void MainMenuWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if(!m_background.isNull())
        p.drawPixmap(rect(), m_background);
    else
        p.fillRect(rect(), QColor(101, 67, 33));

    const int logoW = 334;
    const int logoH = 80;
    const int logoX = (640 - logoW) / 2;
    const int logoY = 20;

    const int btnW   = 280;
    const int btnH   = 53;
    const int btnX   = (640 - btnW) / 2; //

    if(!m_logo.isNull())
    {
        p.drawPixmap(logoX, logoY, logoW, logoH, m_logo);
    }
    else
    {
        p.fillRect(logoX, logoY, logoW, logoH, QColor(70, 40, 15));
        p.setPen(QColor(220, 180, 100));
        QFont f = p.font();
        f.setPixelSize(42);
        f.setBold(true);
        p.setFont(f);
        p.drawText(
            QRect(logoX, logoY, logoW, logoH),
            Qt::AlignCenter,
            "AtomShift");
    }

    struct BtnInfo { QRect r; QString label;};
    BtnInfo btns[4] = {
        { m_playRect,  "Play"},
        { m_helpRect,  "Help"}, 
        { m_aboutRect, "About"}, 
        { m_exitRect,  "Exit"}
    };

    // texts
    QRect m_playRect_Text  = QRect(btnX, 108, btnW, btnH-8);
    QRect m_helpRect_Text  = QRect(btnX, 108 + (btnH-4),      btnW, btnH-8);
    QRect m_aboutRect_Text = QRect(btnX, 108 + (btnH-4) * 2,  btnW, btnH-8);
    QRect m_exitRect_Text  = QRect(btnX, 108 + (btnH-4) * 3,  btnW, btnH-8);

    struct BtnInfo2 { QRect r; QString label;};
    BtnInfo2 btns2[4] = {
        { m_playRect_Text,  "Play"  },
        { m_helpRect_Text,  "Help" },
        { m_aboutRect_Text, "About" },
        { m_exitRect_Text,  "Exit"  }
    };

    for(int i = 0; i < 4; i++)
    {
        if(!mmenu_btn.isNull())
        {
            p.drawPixmap(btns[i].r, mmenu_btn);
        }
        else
        {
            QColor bgColor = QColor(60, 30, 10);

            p.setPen(Qt::NoPen);
            p.setBrush(bgColor);
            p.drawRoundedRect(btns[i].r, 8, 8);

            QPen borderPen(QColor(140, 90, 40));
            borderPen.setWidth(1);
            p.setPen(borderPen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(btns[i].r, 8, 8);
        }
    }

    for(int i = 0; i < 4; i++)
    {
        QColor textColor = QColor(230, 230, 230);

        p.setPen(textColor);
        QFont f = p.font();
        f.setPixelSize(18);
        f.setBold(true);
        p.setFont(f);
        p.drawText(btns2[i].r, Qt::AlignCenter, btns2[i].label);
    }

    if(m_showAbout)
    {
        p.fillRect(rect(), QColor(0, 0, 0, 120));

        if(!m_aboutBg.isNull())
            p.drawPixmap(m_aboutRect_popup, m_aboutBg);
        else
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(230, 200, 155));
            p.drawRoundedRect(m_aboutRect_popup, 10, 10);

            QPen borderPen(QColor(100, 60, 20));
            borderPen.setWidth(2);
            p.setPen(borderPen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(m_aboutRect_popup, 10, 10);
        }

        const int px = m_aboutRect_popup.x();
        const int py = m_aboutRect_popup.y();
        const int pw = m_aboutRect_popup.width();

        p.setPen(QColor(60, 30, 10));

        QFont titleFont = p.font();
        titleFont.setPixelSize(28);
        titleFont.setBold(true);
        p.setFont(titleFont);
        p.drawText(
            QRect(px, py + 25, pw, 40),
            Qt::AlignCenter,
            "AtomShift");

        QPen linePen(QColor(100, 60, 20));
        linePen.setWidth(1);
        p.setPen(linePen);
        p.drawLine(px + 30, py + 70, px + pw - 30, py + 70);

        QFont infoFont = p.font();
        infoFont.setPixelSize(15);
        infoFont.setBold(false);
        p.setFont(infoFont);
        p.setPen(QColor(60, 30, 10));

        struct TextRow { int y; QString text; };
        TextRow rows[] = {
            { py + 90,  "Version 1.0"           },
            { py + 115, "By Alireza Elahi"       },
            { py + 140, QString("%1 2026").arg(QChar(0x00A9))        },
            { py + 180, "Special Thanks:"        },
            { py + 200, "KDE/KAtomic - Logic & Atom Graphics"},
            { py + 220, "Symbian community - Beta tests & feedbacks"},
        };

        for(int i = 0; i < 6; i++)
            p.drawText(
                QRect(px, rows[i].y, pw, 25),
                Qt::AlignCenter,
                rows[i].text);

        p.setPen(linePen);
        p.drawLine(px + 30, py + 260, px + pw - 30, py + 260);

        QFont hintFont = p.font();
        hintFont.setPixelSize(12);
        hintFont.setBold(false);
        p.setFont(hintFont);
        p.setPen(QColor(100, 60, 20));
        p.drawText(
            QRect(px, py + 270, pw, 30),
            Qt::AlignCenter,
            "Tap to close");
    }

    if(m_showHelp)
    {
        p.fillRect(rect(), QColor(0, 0, 0, 120));

        const int px = m_helpRect_popup.x();
        const int py = m_helpRect_popup.y();
        const int pw = m_helpRect_popup.width();

        if(!m_helpBg.isNull())
            p.drawPixmap(m_helpRect_popup, m_helpBg);
        else
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(230, 200, 155));
            p.drawRoundedRect(m_helpRect_popup, 10, 10);

            QPen borderPen(QColor(100, 60, 20));
            borderPen.setWidth(2);
            p.setPen(borderPen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(m_helpRect_popup, 10, 10);
        }

        p.setPen(QColor(60, 30, 10));

        QFont titleFont = p.font();
        titleFont.setPixelSize(22);
        titleFont.setBold(true);
        p.setFont(titleFont);
        p.drawText(
            QRect(px, py + 18, pw, 30),
            Qt::AlignCenter,
            "Help");

        QPen linePen(QColor(100, 60, 20));
        linePen.setWidth(1);
        p.setPen(linePen);
        p.drawLine(px + 30, py + 52, px + pw - 30, py + 52);

        QFont infoFont = p.font();
        infoFont.setPixelSize(13);
        infoFont.setBold(false);
        p.setFont(infoFont);
        p.setPen(QColor(60, 30, 10));

        struct TextRow { int y; QString text; bool bold; };
        TextRow rows[] = {
            { py + 62,  "AtomShift is a fun game about atoms & molecules!", true  },
            { py + 80,  "Shift atoms into place to build the target molecule.",       false },
            { py + 96,  "Atoms slide until they hit a wall or another atom.",      false },
            { py + 112, "Plan your shifts wisely!",        false },

            { py + 135, "Controls:",                                         true  },

            { py + 155, "Select Atom:",                                      true  },
            { py + 171, "Tap atom  | Tap select button | Press Space key",            false },

            { py + 191, "Move Atom:",                                        true  },
            { py + 207, "Tap destination  | D-pad (right panel) | Press Arrow keys",         false },

            { py + 227, "Undo / Redo:",                                      true  },
            { py + 243, "Tap Undo button | Press Backspace key",                    false },
            { py + 259, "Tap Redo button",                            false },
        };

        for(int i = 0; i < 12; i++)
        {
            QFont f = p.font();
            f.setBold(rows[i].bold);
            f.setPixelSize(rows[i].bold ? 15 : 14);
            p.setFont(f);
            p.drawText(
                QRect(px + 22, rows[i].y, pw - 40, 20),
                Qt::AlignLeft,
                rows[i].text);
        }

        p.setPen(linePen);
        p.drawLine(px + 30, py + 300, px + pw - 30, py + 300);

        QFont hintFont = p.font();
        hintFont.setPixelSize(13);
        hintFont.setBold(false);
        p.setFont(hintFont);
        p.setPen(QColor(100, 60, 20));
        p.drawText(
            QRect(px, py + 308, pw, 20),
            Qt::AlignCenter,
            "Tap to close");
    }
}


void MainMenuWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    if(m_showAbout)
    {
        m_showAbout = false;
        update();
        return;
    }

    if(m_showHelp)
    {
        m_showHelp = false;
        update();
        return;
    }

    if(m_playRect.contains(pos))
    {
        emit startClicked();
        return;
    }

    if(m_exitRect.contains(pos))
    {
        QApplication::quit();
        return;
    }

    if(m_aboutRect.contains(pos))
    {
        m_showAbout = true;
        update();
        return;
    }

    if(m_helpRect.contains(pos))
    {
        m_showHelp = true;
        update();
        return;
    }
}
