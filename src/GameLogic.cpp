#include "GameLogic.h"
#include <cmath>

bool GameLogic::inBounds(int col, int row) {
    return col >= 0 && col < 8 && row >= 0 && row < 8;
}

bool GameLogic::isLineMove(Point from, Point to) {
    int dr = std::abs(to.row - from.row);
    int dc = std::abs(to.col - from.col);
    if (dr == 0 && dc == 0) return false;
    return (dr == 0 || dc == 0 || dr == dc);
}

std::set<int> GameLogic::getOccupiedPositions(const QVector<Piece>& pieces, const QVector<Point>& blocks) {
    std::set<int> occupied;
    for (const auto& p : pieces) occupied.insert(getPosKey(p.col, p.row));
    for (const auto& b : blocks) occupied.insert(getPosKey(b.col, b.row));
    return occupied;
}

bool GameLogic::isPathClear(Point from, Point to, const QVector<Piece>& pieces, const QVector<Point>& blocks) {
    if (!inBounds(to.col, to.row) || !isLineMove(from, to)) return false;

    int stepR = (to.row == from.row) ? 0 : (to.row - from.row) / std::abs(to.row - from.row);
    int stepC = (to.col == from.col) ? 0 : (to.col - from.col) / std::abs(to.col - from.col);

    auto occupied = getOccupiedPositions(pieces, blocks);

    int r = from.row + stepR;
    int c = from.col + stepC;

    while (true) {
        if (occupied.count(getPosKey(c, r))) return false;
        if (r == to.row && c == to.col) break;
        r += stepR;
        c += stepC;
    }
    return true;
}

bool GameLogic::canPlayerMove(int player, const QVector<Piece>& pieces, const QVector<Point>& blocks) {
    static const int dirs[8][2] = {{1,1},{1,0},{1,-1},{0,1},{0,-1},{-1,1},{-1,0},{-1,-1}};
    auto occupied = getOccupiedPositions(pieces, blocks);

    for (const auto& piece : pieces) {
        if (piece.user != player) continue;
        for (int i = 0; i < 8; ++i) {
            int nc = piece.col + dirs[i][0];
            int nr = piece.row + dirs[i][1];
            if (inBounds(nc, nr) && !occupied.count(getPosKey(nc, nr))) return true;
        }
    }
    return false;
}