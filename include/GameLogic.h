#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QVector>
#include <set>
#include "AmazonBoard.h"

class GameLogic {
public:
    static bool inBounds(int col, int row);
    static bool isLineMove(Point from, Point to);
    static bool isPathClear(Point from, Point to, const QVector<Piece>& pieces, const QVector<Point>& blocks);
    static bool canPlayerMove(int player, const QVector<Piece>& pieces, const QVector<Point>& blocks);

private:
    // 辅助函数：将坐标转换为唯一索引
    static inline int getPosKey(int c, int r) { return r * 10 + c; }
    // 获取当前所有被占用的位置集合
    static std::set<int> getOccupiedPositions(const QVector<Piece>& pieces, const QVector<Point>& blocks);
};

#endif // GAMELOGIC_H