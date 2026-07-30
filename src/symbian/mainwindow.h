#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

namespace Ui {
    class MainWindow;
}

class GameBoardWidget;
class MainMenuWidget;
class LevelPackSelectWidget;
class LevelSelectWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onStartClicked();
    void onLevelPackSelected(const QString& filePath);
    void onBackToMainMenu();
    void onLevelSelected(int levelNumber);
    void onBackToLevelPacks();

private:
    Ui::MainWindow *ui;

    QStackedWidget *m_stack;
    MainMenuWidget *m_mainMenu;
    LevelPackSelectWidget *m_levelPackSelect;
    LevelSelectWidget *m_levelSelect;
    GameBoardWidget *m_gameBoard;

protected:
    bool event(QEvent *e);
};

#endif // MAINWINDOW_H

