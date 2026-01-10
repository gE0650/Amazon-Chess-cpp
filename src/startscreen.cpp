#include "startscreen.h"
#include <QMessageBox>
#include <QHeaderView>

StartScreen::StartScreen(QWidget *parent) : QWidget(parent) {
    setWindowTitle("Amazon Chess - Main Menu");
    resize(600, 400);
    setMinimumSize(400, 300);

    // 全局背景与字体
    this->setStyleSheet(
        "QWidget { background-color: #F0F2F5; font-family: 'Segoe UI', sans-serif; }"
        "QScrollBar:vertical { width: 8px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #BDC3C7; border-radius: 4px; }"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(40, 40, 40, 40);
    
    QLabel *title = new QLabel("Amazon Chess", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: #2C3E50; margin-bottom: 10px;");
    layout->addWidget(title);

    QLabel *listLabel = new QLabel("Saved Games:", this);
    listLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #7F8C8D;");
    layout->addWidget(listLabel);

    saveListWidget = new QListWidget(this);
    saveListWidget->setStyleSheet(
        "QListWidget { "
        "  background-color: white; border: 1px solid #E1E4E8; border-radius: 8px; padding: 5px; outline: none;"
        "}"
        "QListWidget::item { padding: 12px; color: #34495E; border-bottom: 1px solid #F0F2F5; }"
        "QListWidget::item:selected { background-color: #E8F6F3; color: #16A085; border-radius: 4px; border: none; }"
        "QListWidget::item:hover { background-color: #F8F9F9; }"
    );
    layout->addWidget(saveListWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    // 通用按钮样式
    QString btnStyle = 
        "QPushButton { "
        "  background-color: #3498DB; color: white; border-radius: 6px; padding: 12px; "
        "  font-size: 14px; font-weight: bold; border: none; "
        "}"
        "QPushButton:hover { background-color: #2980B9; }"
        "QPushButton:pressed { background-color: #1F618D; }";

    btnNewGame = new QPushButton("PvP (2 Players)", this);
    btnNewGame->setCursor(Qt::PointingHandCursor);
    btnNewGame->setStyleSheet(btnStyle);

    btnVsAI = new QPushButton("PvE (Vs Computer)", this);
    btnVsAI->setCursor(Qt::PointingHandCursor);
    btnVsAI->setStyleSheet(btnStyle);

    btnLoadGame = new QPushButton("Load Selected", this);
    btnLoadGame->setCursor(Qt::PointingHandCursor);
    btnLoadGame->setStyleSheet(
        "QPushButton { "
        "  background-color: #2ECC71; color: white; border-radius: 6px; padding: 12px; "
        "  font-size: 14px; font-weight: bold; border: none; "
        "}"
        "QPushButton:hover { background-color: #27AE60; }"
        "QPushButton:pressed { background-color: #219150; }"
    );
    
    btnLayout->addWidget(btnNewGame);
    btnLayout->addWidget(btnVsAI);
    btnLayout->addWidget(btnLoadGame);
    layout->addLayout(btnLayout);

    connect(btnNewGame, &QPushButton::clicked, this, &StartScreen::onNewGame);
    connect(btnVsAI, &QPushButton::clicked, this, &StartScreen::onVsAI);
    connect(btnLoadGame, &QPushButton::clicked, this, &StartScreen::onLoadGame);

    refreshSaveList();
}

void StartScreen::refreshSaveList() {
    saveListWidget->clear();
    QDir dir("saves");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFileInfoList list = dir.entryInfoList(QStringList() << "*.json", QDir::Files | QDir::NoSymLinks, QDir::Time);
    for (const QFileInfo &info : list) {
        saveListWidget->addItem(info.fileName());
    }
}

void StartScreen::onNewGame() {
    launchGame("", false);
}

void StartScreen::onVsAI() {
    launchGame("", true);
}

void StartScreen::onLoadGame() {
    QListWidgetItem *item = saveListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Warning", "Please select a save file first.");
        return;
    }
    QString fileName = item->text();
    launchGame("saves/" + fileName, false); // Load assumes PvP for now, or save must store mode
}

void StartScreen::launchGame(const QString &savePath, bool vsAI) {
    MainWindow *game = new MainWindow(nullptr, vsAI);
    
    // 监听游戏窗口关闭，重新显示主菜单
    connect(game, &MainWindow::gameClosed, this, [this, game]() {
        this->show();
        this->refreshSaveList();
        game->deleteLater();
    });

    if (!savePath.isEmpty()) {
        if (!game->loadGame(savePath)) {
            QMessageBox::critical(this, "Error", "Failed to load save file.");
            game->deleteLater();
            return;
        }
    }
    
    game->show();
    this->hide(); 
}
