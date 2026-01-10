#include "search_engine.h"
#include <QtGlobal>
#include <QTime>
#include <algorithm>
#include <random>
#include <cmath>
#include <QDebug>
#include <QRandomGenerator>

SearchEngine::SearchEngine() {
    // 初始化中心权重 (越靠近中心分越高)
    for(int r=0; r<10; ++r) {
        for(int c=0; c<10; ++c) {
            // 简单的中心距离计算
            double dist = std::sqrt(std::pow(r - 4.5, 2) + std::pow(c - 4.5, 2));
            centerWeights[c][r] = 10 - (int)dist; 
        }
    }
}

// 辅助：深拷贝移动
void applyMove(AmazonBoard& board, const FullMove& move) {
    // 1. Move Piece
    for(auto& p : board.pieces) {
        if(p.col == move.from.col && p.row == move.from.row) {
            p.col = move.to.col;
            p.row = move.to.row;
            break;
        }
    }
    // 2. Place Arrow
    board.blocks.push_back(move.arrow);
}

// 获取某个位置的所有可达点 (Queen moves)
QVector<Point> getReachable(const AmazonBoard& board, Point p) {
    QVector<Point> moves;
    static const int dirs[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    
    // 为了效率，构建临时 occupied set
    // 注意：实际高频调用时应传入 set 或 grid 数组，这里为了代码简洁直接遍历
    // 优化：使用简单的 10x10 bool 数组
    bool occupied[10][10] = {false};
    for(const auto& pc : board.pieces) occupied[pc.col][pc.row] = true;
    for(const auto& bk : board.blocks) occupied[bk.col][bk.row] = true;

    for(int i=0; i<8; ++i) {
        int x = p.col;
        int y = p.row;
        while(true) {
            x += dirs[i][0];
            y += dirs[i][1];
            if(x < 0 || x >= 10 || y < 0 || y >= 10) break;
            if(occupied[x][y]) break;
            moves.push_back({x, y});
        }
    }
    return moves;
}

double SearchEngine::evaluate(const AmazonBoard& board, int player) {
    double score = 0;
    
    // 1. 灵活性 (Mobility) - 简单计算每个棋子的可移动步数
    int myMobility = 0;
    int oppMobility = 0;

    for(const auto& p : board.pieces) {
        int moves = getReachable(board, {p.col, p.row}).size();
        if(p.user == player) {
            myMobility += moves;
            // 2. 中心控制 (Center Control) - 前期权重较大
            score += centerWeights[p.col][p.row] * 0.5;
        } else {
            oppMobility += moves;
            score -= centerWeights[p.col][p.row] * 0.5;
        }
    }

    score += (myMobility - oppMobility);
    return score;
}

// 蒙特卡洛/随机模拟：从当前局面快速走 N 步，看谁更有利
double SearchEngine::runMonteCarlo(AmazonBoard board, int player, int depth) {
    int simPlayer = player; 
    // 简单的随机模拟
    // 由于性能原因，我们只模拟几步，而不是走到终局
    for(int d=0; d<depth; ++d) {
        // 随机选一个棋子
        QVector<int> myIndices;
        for(int i=0; i<board.pieces.size(); ++i) {
            if(board.pieces[i].user == simPlayer) myIndices.push_back(i);
        }
        if(myIndices.isEmpty()) return -1000; // 输了

        // 随机移动
        int pIdx = myIndices[QRandomGenerator::global()->bounded(myIndices.size())];
        Piece& p = board.pieces[pIdx];
        
        QVector<Point> moves = getReachable(board, {p.col, p.row});
        if(moves.isEmpty()) return (simPlayer == player) ? -1000 : 1000;

        Point to = moves[QRandomGenerator::global()->bounded(moves.size())];
        Point from = {p.col, p.row};
        p.col = to.col;
        p.row = to.row;

        // 随机射箭
        QVector<Point> arrowOps = getReachable(board, to);
        if(arrowOps.isEmpty()) return (simPlayer == player) ? -1000 : 1000;
        Point arrow = arrowOps[QRandomGenerator::global()->bounded(arrowOps.size())];
        board.blocks.push_back(arrow);

        simPlayer = 1 - simPlayer;
    }
    return evaluate(board, player);
}

FullMove SearchEngine::getBestMove(const AmazonBoard& board, int player) {
    // 核心算法：Alpha-Beta 剪枝 + 启发式搜索
    // 由于分支因子巨大，我们采用两阶段搜索：
    // 1. 找出所有棋子的可行移动，评估“移动后”的局面，选出 Top K 个好的移动
    // 2. 对这 Top K 个移动，穷举所有射箭位置，选出最佳组合

    QVector<FullMove> candidates;
    
    // Step 1: Generate Queen Moves
    for(const auto& p : board.pieces) {
        if(p.user != player) continue;
        
        QVector<Point> moves = getReachable(board, {p.col, p.row});
        for(const auto& to : moves) {
            // 快速评估：只看棋子位置变动带来的收益
            // 模拟移动
            AmazonBoard simBoard = board; 
            // 注意：这里拷贝整个棋盘开销较大，应该尽量优化，但为了代码清晰暂时这样做
            // 优化：只修改坐标进行评估，然后还原
            // 这里为了简单，我们计算 位置分 + 灵活性
            
            double score = centerWeights[to.col][to.row] * 1.0; 
            // 简单的移动启发：不要走到死路
            // 我们可以用 SearchEngine::evaluate 粗略估算
            FullMove fm;
            fm.from = {p.col, p.row};
            fm.to = to;
            fm.score = score;
            candidates.push_back(fm);
        }
    }

    // Sort by simple score (Center + basic logic)
    std::sort(candidates.begin(), candidates.end(), [](const FullMove& a, const FullMove& b) {
        return a.score > b.score; 
    });

    // Pruning: Keep top 12 moves (Beam Search-like)
    if(candidates.size() > 12) candidates.resize(12);

    // Step 2: For top moves, find best Arrow
    FullMove bestMove;
    bestMove.score = -999999;
    bool found = false;

    for(auto& move : candidates) {
        // Apply move temporarily
        AmazonBoard simBoard = board;
        // Fix piece position in simBoard
        int pIdx = -1;
        for(int i=0; i<simBoard.pieces.size(); ++i) {
            if(simBoard.pieces[i].col == move.from.col && simBoard.pieces[i].row == move.from.row) {
                simBoard.pieces[i].col = move.to.col;
                simBoard.pieces[i].row = move.to.row;
                pIdx = i;
                break;
            }
        }

        // Generate Arrows from new position
        QVector<Point> arrows = getReachable(simBoard, move.to);
        
        // 如果射箭太耗时，我们也可以只采样部分箭
        // 但射箭是封锁对方的关键，尽量多看
        
        for(const auto& ar : arrows) {
            // 模拟射箭
            simBoard.blocks.push_back(ar); // Push
            
            // Evaluation
            // 混合：静态评估 + 蒙特卡洛微量模拟
            double staticVal = evaluate(simBoard, player);
            // double mcVal = runMonteCarlo(simBoard, player, 5); // Disable deep sim for speed
            
            double finalScore = staticVal;

            if(finalScore > bestMove.score) {
                bestMove = move;
                bestMove.arrow = ar;
                bestMove.score = finalScore;
                found = true;
            }

            simBoard.blocks.pop_back(); // Pop
        }
    }

    if(!found && !candidates.isEmpty()) {
        // Fallback
        bestMove = candidates[0];
        // Find any valid arrow
        AmazonBoard tmp = board;
         for(auto& p : tmp.pieces) {
            if(p.col == bestMove.from.col && p.row == bestMove.from.row) {
                p.col = bestMove.to.col;
                p.row = bestMove.to.row;
                break;
            }
        }
        QVector<Point> ars = getReachable(tmp, bestMove.to);
        if(!ars.isEmpty()) bestMove.arrow = ars[0];
    }

    return bestMove;
}
