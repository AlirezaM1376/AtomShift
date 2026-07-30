#include <QEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gameboardwidget.h"
#include "mainmenuwidget.h"
#include "levelpackselectwidget.h"
#include "levelselectwidget.h"
#include "audiomanager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_mainMenu = new MainMenuWidget(m_stack);
    m_levelPackSelect = new LevelPackSelectWidget(m_stack);
    m_gameBoard = new GameBoardWidget(m_stack);

    m_stack->addWidget(m_mainMenu);          // index 0
    m_stack->addWidget(m_levelPackSelect);   // index 1
    m_stack->addWidget(m_gameBoard);         // index 2

    connect(m_mainMenu, SIGNAL(startClicked()), this, SLOT(onStartClicked()));
    connect(m_levelPackSelect, SIGNAL(levelPackSelected(QString)),
            this, SLOT(onLevelPackSelected(QString)));
    connect(m_levelPackSelect, SIGNAL(backClicked()),
            this, SLOT(onBackToMainMenu()));
    connect(m_gameBoard, SIGNAL(BackToLevelPacks()),
            this, SLOT(onStartClicked()));

    m_levelSelect = new LevelSelectWidget(m_stack);
    m_stack->addWidget(m_levelSelect); // index 3

    connect(m_levelSelect, SIGNAL(levelSelected(int)),
            this, SLOT(onLevelSelected(int)));
    connect(m_levelSelect, SIGNAL(backClicked()),
            this, SLOT(onBackToLevelPacks()));

    m_stack->setCurrentWidget(m_mainMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onStartClicked()
{
    m_levelPackSelect->refreshLevelPackList();
    m_stack->setCurrentWidget(m_levelPackSelect);
    AudioManager::instance().stopGameMusic();
    AudioManager::instance().playMenuMusic();
}

void MainWindow::onLevelPackSelected(const QString& filePath)
{
    m_levelSelect->setup(filePath);
    m_stack->setCurrentWidget(m_levelSelect);
}

void MainWindow::onLevelSelected(int levelNumber){
    AudioManager::instance().stopMusic();
    m_gameBoard->startWithLevelSet(m_levelSelect->levelSetFile());
    m_gameBoard->goToLevel(levelNumber);
    m_stack->setCurrentWidget(m_gameBoard);
    m_gameBoard->setFocus();
}


void MainWindow::onBackToLevelPacks()
{
    m_stack->setCurrentWidget(m_levelPackSelect);
}

void MainWindow::onBackToMainMenu()
{
    AudioManager::instance().stopGameMusic();
    AudioManager::instance().playMenuMusic();
    m_stack->setCurrentWidget(m_mainMenu);
}

bool MainWindow::event(QEvent *e)
{
    switch(e->type())
    {
    case QEvent::WindowDeactivate:
        AudioManager::instance().pause();
        m_gameBoard->pauseGame();
        break;

    case QEvent::WindowActivate:
        AudioManager::instance().resume();
        break;

    default:
        break;
    }

    return QMainWindow::event(e);
}
