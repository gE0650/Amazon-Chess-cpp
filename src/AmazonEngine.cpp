#include "AmazonEngine.h"

// 构造函数：可以在这里进行棋盘初始化（如放置 8 个初始棋子）
AmazonEngine::AmazonEngine() {
    // 初始化棋盘状态
    currentBoard.status = "playing";
    currentBoard.currentPlayer = 1; // 红方先手
    currentBoard.pieces.clear();
    currentBoard.blocks.clear();

    // 初始化红方棋子 (1) - 8x8 适配
    currentBoard.pieces.push_back({2, 0, 1}); // (col, row, user)
    currentBoard.pieces.push_back({5, 0, 1});
    currentBoard.pieces.push_back({0, 2, 1});
    currentBoard.pieces.push_back({7, 2, 1});

    // 初始化蓝方棋子 (0) - 8x8 适配
    currentBoard.pieces.push_back({2, 7, 0});
    currentBoard.pieces.push_back({5, 7, 0});
    currentBoard.pieces.push_back({0, 5, 0});
    currentBoard.pieces.push_back({7, 5, 0});
}

// 对应原 exports.Movepiece
MoveResult AmazonEngine::movePiece(Point from, Point to) {
    // 1. 基础校验
    if (!GameLogic::inBounds(from.col, from.row) || !GameLogic::inBounds(to.col, to.row))
        return {false, "Out of bounds"};

    if (currentBoard.status == "finished")
        return {false, "Game finished"};

    // 2. 查找棋子
    int pIdx = -1;
    for (int i = 0; i < currentBoard.pieces.size(); ++i) {
        if (currentBoard.pieces[i].col == from.col && currentBoard.pieces[i].row == from.row) {
            pIdx = i;
            break;
        }
    }

    if (pIdx == -1) return {false, "No piece here"};
    if (currentBoard.pieces[pIdx].user != currentBoard.currentPlayer)
        return {false, "Not your turn"};

    // 存档 (History Snapshot) 用于悔棋功能
    // 在移动棋子之前保存状态，这样悔棋可以回到“本回合开始前”
    GameSnapshot snapshot;
    snapshot.pieces = currentBoard.pieces;
    snapshot.blocks = currentBoard.blocks;
    snapshot.currentPlayer = currentBoard.currentPlayer;
    snapshot.status = currentBoard.status;
    snapshot.winner = currentBoard.winner;
    currentBoard.history.push_back(snapshot);

    // 3. 走法校验 (调用之前写的 GameLogic)
    if (!GameLogic::isLineMove(from, to)) return {false, "Not a linear move"};
    if (!GameLogic::isPathClear(from, to, currentBoard.pieces, currentBoard.blocks))
        return {false, "Path blocked"};

    // 4. 执行移动
    currentBoard.pieces[pIdx].col = to.col;
    currentBoard.pieces[pIdx].row = to.row;

    // 记录 Moves
    MoveRecord rec;
    rec.type = "move"; 
    rec.from = from; 
    rec.to = to;
    currentBoard.moves.push_back(rec);

    return {true, "Move successful"};
}

// 对应原 exports.PlaceBlock
MoveResult AmazonEngine::placeArrow(Point target) {
    if (!GameLogic::inBounds(target.col, target.row)) return {false, "Out of bounds"};
    
    // 检查占用 (假设 currentBoard 有 isOccupied 方法)
    if (currentBoard.isOccupied(target.col, target.row))
        return {false, "Position occupied"};

    // 放置障碍
    currentBoard.blocks.push_back(target);
    
    // 切换玩家
    int lastPlayer = currentBoard.currentPlayer;
    currentBoard.currentPlayer = (currentBoard.currentPlayer == 1) ? 0 : 1;

    // 胜负判定
    int nextPlayer = currentBoard.currentPlayer;
    bool canNextMove = GameLogic::canPlayerMove(nextPlayer, currentBoard.pieces, currentBoard.blocks);
    bool canLastMove = GameLogic::canPlayerMove(lastPlayer, currentBoard.pieces, currentBoard.blocks);

    MoveResult res = {true, "Shot successful"};
    
    // 判定逻辑：如果对手没法动了，当前射箭的人赢
    if (!canNextMove) {
        currentBoard.status = "finished";
        currentBoard.winner = lastPlayer;
        res.winner = lastPlayer;
        res.message = "Game Over";
    }

    return res;
}

bool AmazonEngine::undo() {
    if (currentBoard.history.isEmpty()) return false;

    const GameSnapshot& last = currentBoard.history.last();
    currentBoard.pieces = last.pieces;
    currentBoard.blocks = last.blocks;
    currentBoard.currentPlayer = last.currentPlayer;
    currentBoard.status = last.status;
    currentBoard.winner = last.winner;

    currentBoard.history.removeLast();
    return true;
}
