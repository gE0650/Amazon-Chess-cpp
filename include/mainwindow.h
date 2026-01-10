#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QMouseEvent>
#include "AmazonEngine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    AmazonEngine engine;
    
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
};

#endif // MAINWINDOW_H
