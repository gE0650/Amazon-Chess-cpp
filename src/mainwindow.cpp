#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>

#include <QInputDialog>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent, bool vsAI)
    : QMainWindow(parent), isPvE(vsAI), aiThinking(false)
{
    setWindowTitle("Amazon Chess");
    resize(800, 800);
    setMinimumSize(600, 600);
    
    // 设置整体背景
    this->setStyleSheet("QMainWindow { background-color: #2C3E50; }");

    // 按钮样式
    QString btnStyle = 
        "QPushButton { "
        "  background-color: rgba(255, 255, 255, 0.9); color: #2C3E50; border-radius: 15px; "
        "  font-weight: bold; border: 1px solid #BDC3C7; "
        "}"
        "QPushButton:hover { background-color: white; border: 2px solid white; }"
        "QPushButton:pressed { background-color: #ECF0F1; }";

    QPushButton *btnUndo = new QPushButton("Undo", this);
    btnUndo->setGeometry(680, 20, 100, 35); // Adjust position
    btnUndo->setCursor(Qt::PointingHandCursor);
    btnUndo->setStyleSheet(btnStyle);
    connect(btnUndo, &QPushButton::clicked, this, &MainWindow::onUndo);

    QPushButton *btnSave = new QPushButton("Save", this);
    btnSave->setGeometry(570, 20, 100, 35); // Adjust position
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(btnStyle);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveGame);

    statusLabel = new QLabel(this);
    statusLabel->setGeometry(100, 720, 600, 50); // 浮在下方
    statusLabel->setAlignment(Qt::AlignCenter);
    // 现代化的毛玻璃风格状态栏
    statusLabel->setStyleSheet(
        "QLabel { "
        "  font-size: 18px; font-weight: bold; color: #2C3E50; "
        "  background-color: rgba(255, 255, 255, 0.95); "
        "  border-radius: 25px; border: 1px solid rgba(0,0,0,0.1); "
        "  padding: 5px;"
        "}"
    );
    
    updateTurnInfo();
}

MainWindow::~MainWindow()
{
}

void MainWindow::showMessage(const QString &msg, bool isError) {
    statusLabel->setText(msg);
    if (isError) {
        statusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: red; background-color: rgba(255,255,255,0.8); border-radius: 5px;");
    } else {
        statusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; background-color: rgba(255,255,255,0.8); border-radius: 5px;");
    }
}

void MainWindow::updateTurnInfo() {
    const AmazonBoard& board = engine.getBoard();
    QString player = (board.currentPlayer == 1) ? "Red" : "Blue";
    QString state = "";
    if (board.status == "finished") {
        QString wName = (board.winner.toInt() == 1) ? "Red" : "Blue";
        showMessage("Game Over! " + wName + " Wins!");
        return;
    }

    if (isMoving) state = "Please Move Piece";
    else if (isShooting) state = "Please Shoot Arrow";
    else state = "Please Select Piece";
    
    showMessage(player + "'s Turn: " + state);
}

void MainWindow::onUndo() {
    if (engine.undo()) {
        // 重置 UI 状态
        isMoving = false;
        isShooting = false;
        selectedPiece = {-1, -1};
        updateTurnInfo();
        update();
    } else {
        showMessage("No moves to undo", true);
    }
}

void MainWindow::onSaveGame() {
    QString defaultName = "game_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    bool ok;
    QString text = QInputDialog::getText(this, "Save Game",
                                         "Save Name:", QLineEdit::Normal,
                                         defaultName, &ok);
    if (ok && !text.isEmpty()) {
        QDir dir("saves");
        if (!dir.exists()) dir.mkpath(".");
        
        QString filePath = dir.absoluteFilePath(text + ".json");
        const AmazonBoard& board = engine.getBoard();
        if (AmazonPersistence::saveBoard(board, filePath)) {
            showMessage("Game Saved: " + text);
        } else {
             showMessage("Save Failed!", true);
        }
    }
}

bool MainWindow::loadGame(const QString &filePath) {
    AmazonBoard board;
    if (AmazonPersistence::loadBoard(board, filePath)) {
        engine.setBoard(board);
        updateTurnInfo();
        update();
        
        // If loaded game is AI turn
        if (isPvE && engine.getBoard().currentPlayer == 0) {
             QTimer::singleShot(500, this, &MainWindow::runAITurn);
        }
        
        return true;
    }
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    emit gameClosed();
    QMainWindow::closeEvent(event);
}

Point MainWindow::pixelToGrid(int x, int y) {
    int col = (x - boardOffset) / cellSize;
    int row = (y - boardOffset) / cellSize;
    if (col < 0 || col >= 8 || row < 0 || row >= 8) {
        return {-1, -1};
    }
    return {col, row};
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制背景区域 (深色背景)
    painter.fillRect(rect(), QColor("#2C3E50"));

    // 2. 根据窗口大小动态计算棋盘居中位置和大小
    int minDim = qMin(width(), height());
    int bSize = minDim - 100; // 留出 50px 边距
    if (bSize < 300) bSize = 300;
    
    // 更新成员变量供其他函数使用 (需要在 .h 中把 cellSize 和 boardOffset 改为非 const 或在这里计算临时变量)
    // 为简单起见，我们重新计算并使用临时的，但在交互部分 (mousePress) 也需要同步更新。
    // 这里我们做一个简单的 hack: 修改 const 成员不太好，
    // 但为了响应式 UI，我们最好将 cellSize 和 boardOffset 变为动态计算。
    // 由于之前是 const 变量，我们把它们改成成员方法或普通变量。
    // 现在先用之前硬编码的逻辑绘制优化后的颜色。
    
    // 为了美观，我们绘制一个底板阴影
    QRect boardRect(boardOffset - 5, boardOffset - 5, 8 * cellSize + 10, 8 * cellSize + 10);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0,0,0,50));
    painter.drawRoundedRect(boardRect.translated(5, 5), 5, 5); 
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRoundedRect(boardRect, 5, 5);

    drawBoard(painter);
    drawHighlights(painter);
    drawPieces(painter);
}

void MainWindow::drawBoard(QPainter &painter) {
    QColor lightColor("#F0D9B5"); // 经典浅色
    QColor darkColor("#B58863");  // 经典深色

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            QRect rect(boardOffset + i * cellSize, boardOffset + j * cellSize, cellSize, cellSize);
            if ((i + j) % 2 == 0) {
                painter.fillRect(rect, lightColor);
            } else {
                painter.fillRect(rect, darkColor);
            }
        }
    }
    
    // 绘制外边框
    painter.setPen(QPen(QColor("#8B4513"), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(boardOffset, boardOffset, 8 * cellSize, 8 * cellSize);
}

void MainWindow::drawPieces(QPainter &painter) {
    const AmazonBoard& board = engine.getBoard();

    // 绘制棋子
    for (const auto& piece : board.pieces) {
        QRect rect(boardOffset + piece.col * cellSize + 8, 
                   boardOffset + piece.row * cellSize + 8, 
                   cellSize - 16, cellSize - 16);
        
        QRadialGradient gradient(rect.center(), rect.width()/2);
        
        if (piece.user == 1) { // Red -> Alizarin with shading
            gradient.setColorAt(0, QColor("#E74C3C"));
            gradient.setColorAt(1, QColor("#C0392B"));
            painter.setPen(QPen(QColor("#922B21"), 2));
        } else { // Blue -> Peter River with shading
            gradient.setColorAt(0, QColor("#3498DB"));
            gradient.setColorAt(1, QColor("#2980B9"));
            painter.setPen(QPen(QColor("#1F618D"), 2));
        }
        
        painter.setBrush(gradient);
        painter.drawEllipse(rect);
    }

    // 绘制障碍(箭)
    for (const auto& block : board.blocks) {
        QRect rect(boardOffset + block.col * cellSize + 15,
                   boardOffset + block.row * cellSize + 15,
                   cellSize - 30, cellSize - 30);
        
        // 绘制一个叉或者黑色圆点
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#2C3E50"));
        painter.drawEllipse(rect);
        
        // 加一点白色高光增加立体感
        painter.setBrush(QColor(255,255,255,100));
        painter.drawEllipse(rect.adjusted(5,5,-15,-15));
    }
}

void MainWindow::drawHighlights(QPainter &painter) {
    // 高亮选中的棋子
    if (selectedPiece.col != -1) {
        QRect rect(boardOffset + selectedPiece.col * cellSize,
                   boardOffset + selectedPiece.row * cellSize,
                   cellSize, cellSize);
        
        // 半透明黄色填充覆盖
        painter.fillRect(rect, QColor(255, 235, 59, 128));
        
        // 绿色边框
        painter.setPen(QPen(QColor("#2ECC71"), 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(rect.adjusted(2,2,-2,-2));
    }
    
    // 这里可以加一个 Recent Move 高亮
    if (!engine.getBoard().moves.isEmpty()) {
       MoveRecord last = engine.getBoard().moves.last();
       QRect rTo(boardOffset + last.to.col * cellSize,
               boardOffset + last.to.row * cellSize, cellSize, cellSize);
       QRect rFrom(boardOffset + last.from.col * cellSize,
               boardOffset + last.from.row * cellSize, cellSize, cellSize);
       
       painter.fillRect(rTo, QColor(142, 68, 173, 50)); // 轻微紫色高亮
       painter.fillRect(rFrom, QColor(142, 68, 173, 50));
    }
}

void MainWindow::runAITurn() {
    if (!isPvE) return;
    const AmazonBoard& board = engine.getBoard();
    
    // 假设 AI 执蓝方 (0), 玩家执红方 (1)
    if (board.currentPlayer == 0) {
        aiThinking = true;
        statusLabel->setText("AI (Blue) is thinking...");
        QCoreApplication::processEvents(); // 刷新 UI

        // 运行 AI 搜索
        FullMove aiMove = aiEngine.getBestMove(board, 0);
        
        // 执行移动
        MoveResult mRes = engine.movePiece(aiMove.from, aiMove.to);
        if (mRes.success) {
             // AI 射箭
             MoveResult sRes = engine.placeArrow(aiMove.arrow);
             if (sRes.winner != -1) {
                QString wName = (sRes.winner == 1) ? "Red" : "Blue/AI";
                showMessage("Game Over! " + wName + " Wins!");
             } else {
                 updateTurnInfo();
             }
        } else {
            showMessage("AI Error: " + mRes.message, true);
        }

        aiThinking = false;
        update();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (aiThinking) return; // AI 思考时禁止点击

    Point clicked = pixelToGrid(event->x(), event->y());
    if (clicked.col == -1) return;

    const AmazonBoard& board = engine.getBoard();
    
    // PvE 模式下，如果轮到 AI (Blue/0)，玩家不能动
    if (isPvE && board.currentPlayer == 0) return;

    // 简单的状态机逻辑
    if (!isMoving && !isShooting) {
        // 阶段1: 选择棋子
        int pieceUser = board.getPieceAt(clicked.col, clicked.row);
        if (pieceUser != -1) {
            if (pieceUser == board.currentPlayer) {
                selectedPiece = clicked;
                isMoving = true;
                updateTurnInfo();
                update(); 
            } else {
                 showMessage("Hint: Select your own piece", true);
            }
        }
    } else if (isMoving) {
        // 阶段2: 移动棋子
        if (clicked == selectedPiece) {
            selectedPiece = {-1, -1};
            isMoving = false;
            updateTurnInfo();
            update();
            return;
        }

        MoveResult res = engine.movePiece(selectedPiece, clicked);
        if (res.success) {
            isMoving = false;
            isShooting = true;
            selectedPiece = clicked; // 更新选中位置为新的位置，准备射箭
            updateTurnInfo();
            update();
        } else {
             showMessage("Invalid Move: " + res.message, true);
        }
    } else if (isShooting) {
        // 阶段3: 放置障碍
        MoveResult res = engine.placeArrow(clicked);
        if (res.success) {
            isShooting = false;
            selectedPiece = {-1, -1};
            
            if (res.winner != -1) {
                QString wName = (res.winner == 1) ? "Red" : "Blue/AI";
                showMessage("Game Over! " + wName + " Wins!");
            } else {
                updateTurnInfo();
                // AI Turn Trigger after Human moves
                if (isPvE && engine.getBoard().currentPlayer == 0) {
                     QTimer::singleShot(200, this, &MainWindow::runAITurn);
                }
            }
            
            update();
        } else {
            showMessage("Invalid Shot: " + res.message, true);
        }
    }
}
