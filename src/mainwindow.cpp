#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Amazon Chess");
    setFixedSize(800, 800);
}

MainWindow::~MainWindow()
{
}

Point MainWindow::pixelToGrid(int x, int y) {
    int col = (x - boardOffset) / cellSize;
    int row = (y - boardOffset) / cellSize;
    if (col < 0 || col >= 10 || row < 0 || row >= 10) {
        return {-1, -1};
    }
    return {col, row};
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(240, 240, 240));

    drawBoard(painter);
    drawHighlights(painter);
    drawPieces(painter);
}

void MainWindow::drawBoard(QPainter &painter) {
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            QRect rect(boardOffset + i * cellSize, boardOffset + j * cellSize, cellSize, cellSize);
            if ((i + j) % 2 == 0) {
                painter.fillRect(rect, QColor(255, 206, 158)); // 浅色格
            } else {
                painter.fillRect(rect, QColor(209, 139, 71));  // 深色格
            }
            painter.setPen(Qt::black);
            painter.drawRect(rect);
        }
    }
}

void MainWindow::drawPieces(QPainter &painter) {
    const AmazonBoard& board = engine.getBoard();

    // 绘制棋子
    for (const auto& piece : board.pieces) {
        QRect rect(boardOffset + piece.col * cellSize + 5, 
                   boardOffset + piece.row * cellSize + 5, 
                   cellSize - 10, cellSize - 10);
        
        if (piece.user == 1) {
            painter.setBrush(Qt::red);
        } else {
            painter.setBrush(Qt::blue);
        }
        painter.setPen(Qt::black);
        painter.drawEllipse(rect);
    }

    // 绘制障碍(箭)
    for (const auto& block : board.blocks) {
        QRect rect(boardOffset + block.col * cellSize + 15,
                   boardOffset + block.row * cellSize + 15,
                   cellSize - 30, cellSize - 30);
        painter.setBrush(Qt::black); // 黑色表示障碍
        painter.drawEllipse(rect);
    }
}

void MainWindow::drawHighlights(QPainter &painter) {
    // 高亮选中的棋子
    if (selectedPiece.col != -1) {
        QRect rect(boardOffset + selectedPiece.col * cellSize,
                   boardOffset + selectedPiece.row * cellSize,
                   cellSize, cellSize);
        
        painter.setPen(QPen(Qt::green, 3));
        painter.drawRect(rect);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    Point clicked = pixelToGrid(event->x(), event->y());
    if (clicked.col == -1) return;

    const AmazonBoard& board = engine.getBoard();

    // 简单的状态机逻辑
    if (!isMoving && !isShooting) {
        // 阶段1: 选择棋子
        int pieceUser = board.getPieceAt(clicked.col, clicked.row);
        if (pieceUser != -1) {
            if (pieceUser == board.currentPlayer) {
                selectedPiece = clicked;
                isMoving = true;
                update(); 
            } else {
                 QMessageBox::information(this, "提示", "请选择己方棋子");
            }
        }
    } else if (isMoving) {
        // 阶段2: 移动棋子
        if (clicked == selectedPiece) {
            selectedPiece = {-1, -1};
            isMoving = false;
            update();
            return;
        }

        MoveResult res = engine.movePiece(selectedPiece, clicked);
        if (res.success) {
            isMoving = false;
            isShooting = true;
            selectedPiece = clicked; // 更新选中位置为新的位置，准备射箭
            update();
        } else {
             QMessageBox::warning(this, "无效移动", res.message);
        }
    } else if (isShooting) {
        // 阶段3: 放置障碍
        MoveResult res = engine.placeArrow(clicked);
        if (res.success) {
            isShooting = false;
            selectedPiece = {-1, -1};
            
            if (res.winner != -1) {
                QString wName = (res.winner == 1) ? "红方" : "蓝方";
                QMessageBox::information(this, "游戏结束", wName + " 获胜!");
            }
            
            update();
        } else {
            QMessageBox::warning(this, "无效射箭", res.message);
        }
    }
}
