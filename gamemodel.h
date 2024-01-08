#ifndef GAMEMODEL_H
#define GAMEMODEL_H

// ---- 五子棋游戏模型类 ---- //
#include <vector>
// 游戏类型，双人还是AI（目前固定让AI下黑子）
enum GameType
{
    PERSON,
    BOT
};

// 游戏状态
enum GameStatus
{
    PLAYING,
    WIN,
    DEAD
};

// 棋盘尺寸
const int kBoardSizeNum = 15;

class GameModel
{

public:
    GameModel();
    static const int INF = 106666666;
    static const int ERROR_INDEX = -1;
    static const int AI_ZERO = 0;
    static const int AI_ONE = 10;
    static const int AI_ONE_S = 1;
    static const int AI_TWO = 800;
    static const int AI_TWO_S = 300;
    static const int AI_THREE = 3000;
    static const int AI_THREE_S = 1000;
    static const int AI_FOUR = 300000;
    static const int AI_FOUR_S = 2500;
    static const int AI_FIVE = 1000000;

    static const int AI_EMPTY = 0; // 无子
    static const int AI_MY = 1; // 待评价子
    static const int AI_OP = 2; // 对方子或不能下子
    static const int MAX_NODE = 1;//AI白棋
    static const int MIN_NODE = -1;//人黑棋
    static const int MAX_DEPTH=3;

public:
    std::vector<std::vector<int>> gameMapVec; // 存储当前游戏棋盘和棋子的情况,空白为0，白子1，黑子-1
    std::vector<std::vector<int>> gameMapTmp;
    std::vector<std::vector<int>> scoreMapVec; // 存储各个点位的评分情况，作为AI下棋依据
    bool playerFlag; // 标示下棋方
    GameType gameType; // 游戏模式
    GameStatus gameStatus; // 游戏状态
    int lastrowPer,lastcolPer;//黑子
    int lastrowPer2,lastcolPer2;
    int lastrowAI,lastcolAI;
    bool belost;
    int value;

    void startGame(GameType type); // 开始游戏
    void actionByPerson(int row, int col); // 人执行下棋
    void actionByAI(int &clickRow, int &clickCol); // 机器执行下棋
    void updateGameMap(int row, int col); // 每次落子后更新游戏棋盘
    bool isWin(int row, int col); // 判断游戏是否胜利
    bool isDeadGame(); // 判断是否和棋
    void retractGame();

    /*搜索算法用到的函数*/
    int minMax(int x, int y, int type, int depth, int alpha, int beta);
    int evaluateState(int type);
    int getValue(int cnt, int blk);
    int evaluateLine(int line[]);
    int evaluateLine(int line[], bool ALL);
    int evaluatePiece(int x, int y, int type);
    int getPieceType(int x, int y, int type);
    int getPieceType(int A, int type);
    int nextType(int type);
    bool canSearch(int x, int y);
};

#endif // GAMEMODEL_H
