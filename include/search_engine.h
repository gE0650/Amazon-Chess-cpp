#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "AmazonBoard.h"
#include "GameLogic.h"
#include <QVector>
#include <QPair>

struct FullMove {
    Point from;
    Point to;
    Point arrow;
    double score;
};

class SearchEngine {
public:
    SearchEngine();

    /**
     * @brief 获取 AI 的最佳走法
     * @param board 当前盘面
     * @param player AI 执棋方 (0 or 1)
     * @return 最佳走法
     */
    FullMove getBestMove(const AmazonBoard& board, int player);

private:
    double evaluate(const AmazonBoard& board, int player);
    
    // 蒙特卡洛模拟
    double runMonteCarlo(AmazonBoard board, int player, int iterations);
    
    // 中心控制权重矩阵
    int centerWeights[8][8];
};

#endif // SEARCH_ENGINE_H
