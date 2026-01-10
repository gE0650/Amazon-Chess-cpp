#ifndef AMAZONBOARD_H
#define AMAZONBOARD_H

#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>

// --- 基础数据结构 ---

/**
 * @brief 坐标结构
 */
struct Point {
    int col;
    int row;

    bool operator==(const Point& other) const {
        return col == other.col && row == other.row;
    }
};

/**
 * @brief 棋子结构
 */
struct Piece {
    int col;
    int row;
    int user; // 1: 红方, 0: 蓝方
};

/**
 * @brief 移动记录结构 (对应 JSON 中的 moves 数组项)
 */
struct MoveRecord {
    QString type;      // "move" 或 "block"
    Point from;        // 如果是 block 类型，from 可忽略
    Point to;
    qint64 ts;         // 时间戳
};

/**
 * @brief 历史快照结构 (用于悔棋 history)
 */
struct GameSnapshot {
    QVector<Piece> pieces;
    QVector<Point> blocks;
    int currentPlayer;
    QString status;    // "playing", "finished"
    QVariant winner;   // QVariant 可以存储 int 或 nullptr (QVariant::Invalid)
};

// --- 核心业务类 ---

class AmazonBoard {
public:
    AmazonBoard() : id(""), mode("pvp"), currentPlayer(1), status("playing"), winner(QVariant()) {}

    // 基础信息
    QString id;
    QString mode;          // "pvp" 或 "pve"
    int currentPlayer;     // 1: 红方, 0: 蓝方
    QString status;
    QVariant winner;

    // 当前盘面状态
    QVector<Piece> pieces;
    QVector<Point> blocks;

    // 操作记录与历史
    QVector<MoveRecord> moves;
    QVector<GameSnapshot> history;

    // 常用逻辑辅助函数
    bool isOutOfBounds(int col, int row) const {
        return col < 0 || col >= 10 || row < 0 || row >= 10;
    }

    /**
     * @brief 获取指定位置的棋子
     * @return 如果有棋子返回用户ID(0或1)，否则返回 -1
     */
    int getPieceAt(int col, int row) const {
        for (const auto& p : pieces) {
            if (p.col == col && p.row == row) return p.user;
        }
        return -1;
    }

    /**
     * @brief 检查指定位置是否有障碍物
     */
    bool hasBlockAt(int col, int row) const {
        for (const auto& b : blocks) {
            if (b.col == col && b.row == row) return true;
        }
        return false;
    }

    /**
     * @brief 检查格子是否被占用（棋子或障碍）
     */
    bool isOccupied(int col, int row) const {
        return getPieceAt(col, row) != -1 || hasBlockAt(col, row);
    }
};

#endif // AMAZONBOARD_H