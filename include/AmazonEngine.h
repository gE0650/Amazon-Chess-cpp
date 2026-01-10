#ifndef AMAZONENGINE_H
#define AMAZONENGINE_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "AmazonBoard.h"
#include "GameLogic.h"

struct MoveResult {
    bool success;
    QString message;
    int winner = -1; 
};

class AmazonEngine {
public:
    AmazonEngine();

    // 暴露给外部（如 MainWindow）调用的接口
    MoveResult movePiece(Point from, Point to);
    MoveResult placeArrow(Point target);
    
    // 获取当前棋盘状态用于渲染
    const AmazonBoard& getBoard() const { return currentBoard; }

private:
    AmazonBoard currentBoard;
};

class AmazonPersistence {
public:
    /**
     * @brief 将 AmazonBoard 对象保存为本地 JSON 文件
     */
    static bool saveBoard(const AmazonBoard& board, const QString& filePath) {
        QJsonObject root;
        root["id"] = board.id;
        root["mode"] = board.mode;
        root["currentPlayer"] = board.currentPlayer;
        root["status"] = board.status;
        
        // 处理 winner (QVariant)
        if (board.winner.isValid() && !board.winner.isNull()) {
            root["winner"] = board.winner.toInt();
        } else {
            root["winner"] = QJsonValue::Null;
        }

        // 1. 序列化 pieces
        QJsonArray piecesArray;
        for (const auto& p : board.pieces) {
            QJsonObject obj;
            obj["col"] = p.col;
            obj["row"] = p.row;
            obj["user"] = p.user;
            piecesArray.append(obj);
        }
        root["pieces"] = piecesArray;

        // 2. 序列化 blocks
        QJsonArray blocksArray;
        for (const auto& b : board.blocks) {
            QJsonObject obj;
            obj["col"] = b.col;
            obj["row"] = b.row;
            blocksArray.append(obj);
        }
        root["blocks"] = blocksArray;

        // 3. 序列化 moves (最近的操作记录)
        QJsonArray movesArray;
        for (const auto& m : board.moves) {
            QJsonObject obj;
            obj["type"] = m.type;
            obj["ts"] = m.ts;
            obj["from"] = QJsonObject{{"col", m.from.col}, {"row", m.from.row}};
            obj["to"] = QJsonObject{{"col", m.to.col}, {"row", m.to.row}};
            movesArray.append(obj);
        }
        root["moves"] = movesArray;

        // 4. 序列化 history (用于悔棋的快照)
        QJsonArray historyArray;
        for (const auto& h : board.history) {
            QJsonObject hObj;
            hObj["currentPlayer"] = h.currentPlayer;
            hObj["status"] = h.status;
            
            QJsonArray hPieces;
            for(auto& p : h.pieces) hPieces.append(QJsonObject{{"col", p.col}, {"row", p.row}, {"user", p.user}});
            hObj["pieces"] = hPieces;

            QJsonArray hBlocks;
            for(auto& b : h.blocks) hBlocks.append(QJsonObject{{"col", b.col}, {"row", b.row}});
            hObj["blocks"] = hBlocks;

            historyArray.append(hObj);
        }
        root["history"] = historyArray;

        // 写入文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) return false;
        
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
        return true;
    }

    /**
     * @brief 从本地 JSON 文件恢复 AmazonBoard 状态
     */
    static bool loadBoard(AmazonBoard& board, const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) return false;

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) return false;

        QJsonObject root = doc.object();
        
        // 恢复基础字段
        board.id = root["id"].toString();
        board.mode = root["mode"].toString();
        board.currentPlayer = root["currentPlayer"].toInt();
        board.status = root["status"].toString();
        board.winner = root["winner"].isNull() ? QVariant() : QVariant(root["winner"].toInt());

        // 恢复 pieces
        board.pieces.clear();
        QJsonArray piecesArr = root["pieces"].toArray();
        for (auto val : piecesArr) {
            QJsonObject obj = val.toObject();
            board.pieces.append({obj["col"].toInt(), obj["row"].toInt(), obj["user"].toInt()});
        }

        // 恢复 blocks
        board.blocks.clear();
        QJsonArray blocksArr = root["blocks"].toArray();
        for (auto val : blocksArr) {
            QJsonObject obj = val.toObject();
            board.blocks.append({obj["col"].toInt(), obj["row"].toInt()});
        }

        // 恢复 moves
        board.moves.clear();
        QJsonArray movesArr = root["moves"].toArray();
        for (auto val : movesArr) {
            QJsonObject obj = val.toObject();
            MoveRecord m;
            m.type = obj["type"].toString();
            m.ts = obj["ts"].toVariant().toLongLong();
            QJsonObject f = obj["from"].toObject();
            m.from = {f["col"].toInt(), f["row"].toInt()};
            QJsonObject t = obj["to"].toObject();
            m.to = {t["col"].toInt(), t["row"].toInt()};
            board.moves.append(m);
        }

        // 恢复 history (快照)
        board.history.clear();
        QJsonArray historyArr = root["history"].toArray();
        for (auto val : historyArr) {
            QJsonObject hObj = val.toObject();
            GameSnapshot s;
            s.currentPlayer = hObj["currentPlayer"].toInt();
            s.status = hObj["status"].toString();
            
            QJsonArray hP = hObj["pieces"].toArray();
            for(auto p : hP) s.pieces.append({p.toObject()["col"].toInt(), p.toObject()["row"].toInt(), p.toObject()["user"].toInt()});
            
            QJsonArray hB = hObj["blocks"].toArray();
            for(auto b : hB) s.blocks.append({b.toObject()["col"].toInt(), b.toObject()["row"].toInt()});
            
            board.history.append(s);
        }

        return true;
    }
};

#endif