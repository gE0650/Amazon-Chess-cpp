#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFileInfoList>
#include "mainwindow.h"

class StartScreen : public QWidget {
    Q_OBJECT
public:
    explicit StartScreen(QWidget *parent = nullptr);

private slots:
    void onNewGame();
    void onLoadGame();
    void refreshSaveList();

    // AI
    void onVsAI();

private:
    QListWidget *saveListWidget;
    QPushButton *btnNewGame;
    QPushButton *btnVsAI;    // 新增
    QPushButton *btnLoadGame;
    
    // 启动游戏窗口
    void launchGame(const QString &savePath = "", bool vsAI = false);
};

#endif // STARTSCREEN_H
