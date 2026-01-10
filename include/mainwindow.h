#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "AmazonEngine.h"
#include "search_engine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, bool vsAI = false);
    ~MainWindow();
    
    // 加载存档接口
    bool loadGame(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void gameClosed();

private slots:
    void onUndo();
    void onSaveGame();
    void runAITurn();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    AmazonEngine engine;
    SearchEngine aiEngine;
    bool isPvE;
    bool aiThinking;
    
    // UI 状态
    Point selectedPiece = {-1, -1}; // 当前选中的棋子
    bool isMoving = false;          // 是否正在等待移动落子
    bool isShooting = false;        // 是否正在等待放置障碍(射箭)

    // 绘制辅助参数
    const int cellSize = 60;
    const int boardOffset = 50;

    // 辅助函数
    Point pixelToGrid(int x, int y);
    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawHighlights(QPainter &painter);
    void showMessage(const QString &msg, bool isError = false);
    void updateTurnInfo();

    QLabel *statusLabel;
};

#endif // MAINWINDOW_H
