#include <utility>
#include <stdlib.h>
#include <time.h>
#include "GameModel.h"
/*空的构造函数*/
GameModel::GameModel()
{

}
/*开始游戏，对游戏进行初始化*/
void GameModel::startGame(GameType type)
{
    gameType = type;
    // 初始棋盘
    gameMapVec.clear();
    for (int i = 0; i < kBoardSizeNum; i++)
    {
        std::vector<int> lineBoard;
        for (int j = 0; j < kBoardSizeNum; j++)
            lineBoard.push_back(0);
        gameMapVec.push_back(lineBoard);
    }

    // 如果是AI模式，需要初始化评分数组
    if (gameType == BOT)
    {
        scoreMapVec.clear();
        for (int i = 0; i < kBoardSizeNum; i++)
        {
            std::vector<int> lineScores;
            for (int j = 0; j < kBoardSizeNum; j++)
                lineScores.push_back(0);
            scoreMapVec.push_back(lineScores);
        }
    }

    // 己方(人黑)下为true,对方(AI白)下为false
    playerFlag = true;
    //悔棋设置为零
    lastcolAI=0;
    lastcolPer=0;
    lastrowAI=0;
    lastrowPer=0;
    belost=0;
    gameMapTmp=gameMapVec;
}
/*落子后更新游戏棋盘*/
void GameModel::updateGameMap(int row, int col)
{
    if (playerFlag)
        gameMapVec[row][col] = -1;//黑子
    else
        gameMapVec[row][col] = 1;//白子

    // 换手
    playerFlag = !playerFlag;
}
/*人下*/
void GameModel::actionByPerson(int row, int col)
{
    if(playerFlag){//黑子
        lastrowPer=row;
        lastcolPer=col;
    }
    else
    {
        lastrowPer2=row;
        lastcolPer2=col;
    }
    updateGameMap(row, col);
}
/*若xy位置周围1格有棋子则搜索*/
bool GameModel::canSearch(int x, int y) {

    int tmpx = x - 1;
    int tmpy = y - 1;
    for (int i = 0; tmpx < kBoardSizeNum && i < 3; ++tmpx, ++i) {
        int ty = tmpy;
        for (int j = 0; ty < kBoardSizeNum && j < 3; ++ty, ++j) {
           if (tmpx >= 0 && ty >= 0 && gameMapTmp[tmpx][ty]!=0)
               return true;
           else
               continue;
        }
    }
        return false;
}

/*给出后继节点的类型，type为棋子类型*/
int GameModel::nextType(int type) {
    return type == MAX_NODE ? MIN_NODE : MAX_NODE;
}
/*A是带判断棋子，type是此时我方棋子的类型，返回我方、空、敌方*/
int GameModel::getPieceType(int A, int type) {
    return A == type ? AI_MY : (A == 0 ? AI_EMPTY : AI_OP);
}
int GameModel::getPieceType(int x, int y, int type) {
    if (x < 0 || y < 0 || x >= kBoardSizeNum || y >= kBoardSizeNum)// 超出边界按敌方棋子算
        return AI_OP;
    else
        return getPieceType(gameMapTmp[x][y], type);
}

/*以center作为评估位置进行评价一个方向的棋子*/
int GameModel::evaluateLine(int line[], bool all) {
    int value = 0; //估值
    int cnt = 0; // 连子数
    int blk = 0; // 封闭数
    if(all)
        cnt=0;
    for (int i = 0; i < kBoardSizeNum; ++i) {
        if (line[i] == AI_MY) { // 找到第一个己方的棋子
                                // 还原计数
            cnt = 1;
            blk = 0;
            // 看左侧是否封闭
            if (line[i - 1] == AI_OP)
                ++blk;
            // 计算连子数
            for (i = i + 1; i < kBoardSizeNum && line[i] == AI_MY; ++i, ++cnt);
            // 看右侧是否封闭
            if (line[i] == AI_OP)
                ++blk;
            // 计算评估值
            value += getValue(cnt, blk);
        }
    }
    return value;
}

/*以center为评估评价一个方向的棋子（前后四格）*/
int GameModel::evaluateLine(int line[]) {
    int cnt = 1; // 连子数
    int blk = 0; // 封闭数
                 // 向左右扫
    for (int i = 3; i >= 0; --i) {
        if (line[i] == AI_MY) ++cnt;
        else if (line[i] == AI_OP) {
            ++blk;
            break;
        }
        else
            break;
    }
    for (int i = 5; i < 9; ++i) {
        if (line[i] == AI_MY) ++cnt;
        else if (line[i] == AI_OP) {
            ++blk;
            break;
        }
        else
            break;
    }
    return getValue(cnt, blk);
}

/*根据连子数和封堵数给出一个评价值*/
int GameModel::getValue(int cnt, int blk) {
    if (blk == 0) {//活
        switch (cnt) {
        case 1:
            return AI_ONE;
        case 2:
            return AI_TWO;
        case 3:
            return AI_THREE;
        case 4:
            return AI_FOUR;
        default:
            return AI_FIVE;
        }
    }
    else if (blk == 1) {//单项封死
        switch (cnt) {
        case 1:
            return AI_ONE_S;
        case 2:
            return AI_TWO_S;
        case 3:
            return AI_THREE_S;
        case 4:
            return AI_FOUR_S;
        default:
            return AI_FIVE;
        }
    }
    else {//双向封死
        if (cnt >= 5)
            return AI_FIVE;
        else
            return AI_ZERO;
    }
}

/*评价一个其面上的一方*/
int GameModel::evaluateState(int type) {
    int value = 0;
    // 分解成线状态
    int line[6][17];
    int lineP;
    for (int p = 0; p < 6; ++p)
        line[p][0] = line[p][16] = AI_OP;

      //从四个方向产生
    for (int i = 0; i < kBoardSizeNum; ++i) {
          //产生先状态
        lineP = 1;

        for (int j = 0; j < kBoardSizeNum; ++j) {
            line[0][lineP] = getPieceType(i,j, type); /* | */
            line[1][lineP] = getPieceType(j,i, type); /* - */
            line[2][lineP] = getPieceType(i + j,j, type); /* \ */
            line[3][lineP] = getPieceType(i - j,j, type); /* / */
            line[4][lineP] = getPieceType(j,i + j, type); /* \ */
            line[5][lineP] = getPieceType(kBoardSizeNum - j - 1,i + j, type); /* / */
            ++lineP;
        }
        // 估计
        int special = i == 0 ? 4 : 6;
        for (int p = 0; p < special; ++p) {
            value += evaluateLine(line[p], true);
        }
    }
    return value;
}
/*对一个状态的一个位置放置一种类型的棋子的优劣进行估价*/
int GameModel::evaluatePiece(int x, int y, int type) {
    int value = 0; // 估价值
    int line[17];  //线状态
    bool flagX[8];// 横向边界标志
    flagX[0] = x - 4 < 0;
    flagX[1] = x - 3 < 0;
    flagX[2] = x - 2 < 0;
    flagX[3] = x - 1 < 0;
    flagX[4] = x + 1 > 14;
    flagX[5] = x + 2 > 14;
    flagX[6] = x + 3 > 14;
    flagX[7] = x + 4 > 14;
    bool flagY[8];// 纵向边界标志
    flagY[0] = y - 4 < 0;
    flagY[1] = y - 3 < 0;
    flagY[2] = y - 2 < 0;
    flagY[3] = y - 1 < 0;
    flagY[4] = y + 1 > 14;
    flagY[5] = y + 2 > 14;
    flagY[6] = y + 3 > 14;
    flagY[7] = y + 4 > 14;

    line[4] = AI_MY; // 中心棋子
                     // 横
    line[0] = flagX[0] ? AI_OP : (getPieceType(gameMapTmp[x - 4][y], type));
    line[1] = flagX[1] ? AI_OP : (getPieceType(gameMapTmp[x - 3][y], type));
    line[2] = flagX[2] ? AI_OP : (getPieceType(gameMapTmp[x - 2][y], type));
    line[3] = flagX[3] ? AI_OP : (getPieceType(gameMapTmp[x - 1][y], type));

    line[5] = flagX[4] ? AI_OP : (getPieceType(gameMapTmp[x + 1][y], type));
    line[6] = flagX[5] ? AI_OP : (getPieceType(gameMapTmp[x + 2][y], type));
    line[7] = flagX[6] ? AI_OP : (getPieceType(gameMapTmp[x + 3][y], type));
    line[8] = flagX[7] ? AI_OP : (getPieceType(gameMapTmp[x + 4][y], type));

    value += evaluateLine(line);

    //纵
    line[0] = flagY[0] ? AI_OP : getPieceType(gameMapTmp[x][y - 4], type);
    line[1] = flagY[1] ? AI_OP : getPieceType(gameMapTmp[x][y - 3], type);
    line[2] = flagY[2] ? AI_OP : getPieceType(gameMapTmp[x][y - 2], type);
    line[3] = flagY[3] ? AI_OP : getPieceType(gameMapTmp[x][y - 1], type);

    line[5] = flagY[4] ? AI_OP : getPieceType(gameMapTmp[x][y + 1], type);
    line[6] = flagY[5] ? AI_OP : getPieceType(gameMapTmp[x][y + 2], type);
    line[7] = flagY[6] ? AI_OP : getPieceType(gameMapTmp[x][y + 3], type);
    line[8] = flagY[7] ? AI_OP : getPieceType(gameMapTmp[x][y + 4], type);

    value += evaluateLine(line);

    //左上-右下
    line[0] = flagX[0] || flagY[0] ? AI_OP : getPieceType(gameMapTmp[x - 4][y - 4], type);
    line[1] = flagX[1] || flagY[1] ? AI_OP : getPieceType(gameMapTmp[x - 3][y - 3], type);
    line[2] = flagX[2] || flagY[2] ? AI_OP : getPieceType(gameMapTmp[x - 2][y - 2], type);
    line[3] = flagX[3] || flagY[3] ? AI_OP : getPieceType(gameMapTmp[x - 1][y - 1], type);

    line[5] = flagX[4] || flagY[4] ? AI_OP : getPieceType(gameMapTmp[x + 1][y + 1], type);
    line[6] = flagX[5] || flagY[5] ? AI_OP : getPieceType(gameMapTmp[x + 2][y + 2], type);
    line[7] = flagX[6] || flagY[6] ? AI_OP : getPieceType(gameMapTmp[x + 3][y + 3], type);
    line[8] = flagX[7] || flagY[7] ? AI_OP : getPieceType(gameMapTmp[x + 4][y + 4], type);

    value += evaluateLine(line);

    // 右上-左下
    line[0] = flagX[7] || flagY[0] ? AI_OP : getPieceType(gameMapTmp[x + 4][y - 4], type);
    line[1] = flagX[6] || flagY[1] ? AI_OP : getPieceType(gameMapTmp[x + 3][y - 3], type);
    line[2] = flagX[5] || flagY[2] ? AI_OP : getPieceType(gameMapTmp[x + 2][y - 2], type);
    line[3] = flagX[4] || flagY[3] ? AI_OP : getPieceType(gameMapTmp[x + 1][y - 1], type);

    line[5] = flagX[3] || flagY[4] ? AI_OP : getPieceType(gameMapTmp[x - 1][y + 1], type);
    line[6] = flagX[2] || flagY[5] ? AI_OP : getPieceType(gameMapTmp[x - 2][y + 2], type);
    line[7] = flagX[1] || flagY[6] ? AI_OP : getPieceType(gameMapTmp[x - 3][y + 3], type);
    line[8] = flagX[0] || flagY[7] ? AI_OP : getPieceType(gameMapTmp[x - 4][y + 4], type);

    value += evaluateLine(line);

    return value;
}
/*
type:当前层的标记:MAX/MIN，分别为AI（1）和人（-1）
depth：当前层深
alpha：父层alpha的值
beta：父层beta的值
*/
int GameModel::minMax(int x, int y, int type, int depth, int alpha, int beta)
{
    gameMapTmp[x][y]=nextType(type);
    int weight = 0;
    int max = -INF;//下层权值上界
    int min = INF;//下层权值下界
    if (depth < MAX_DEPTH) {
       // iswin
       if(evaluatePiece(x,y,nextType(type))>=AI_FIVE){
           gameMapTmp[x][y]=0;
           if(type==MIN_NODE)
               return AI_FIVE;
           else
               return -AI_FIVE;
       }
       int i, j;
       for (i =1 ; i < kBoardSizeNum; ++i) {
            for (j = 1; j < kBoardSizeNum; ++j) {
               if (gameMapTmp[i][j]==0 && canSearch(i, j)) {
                   weight = minMax(i, j, nextType(type), depth + 1, min, max);
                   if (weight > max)
                       max = weight;
                   if (weight < min)
                       min = weight;
                // alpha-beta
                   if (type == MAX_NODE) {
                      if (max >= alpha){
                          gameMapTmp[x][y]=0;
                            return max;
                      }
                   }
                   else {
                       if (min <= beta){
                           gameMapTmp[x][y]=0;
                            return min;
                       }
                   }
               }
               else
                   continue;
            }
       }

       if (type == MAX_NODE){
           gameMapTmp[x][y]=0;
           return max;
       }
       else{
           gameMapTmp[x][y]=0;
           return min;
       }
    }
    else {
        weight = evaluateState(MAX_NODE);//评估我方局面
        weight -= type == MIN_NODE ? evaluateState(MIN_NODE) * 10 : evaluateState(MIN_NODE);//评估对方局面
        gameMapTmp[x][y]=0;
        return weight;
    }
}
/*AI下*/
void GameModel::actionByAI(int& clickRow, int& clickCol)
{
    // 计算评分
    int weight;
    int maxScore = -INF;
    std::vector<std::pair<int, int>> maxPoints;

    scoreMapVec.clear();
    for (int row = 1; row < kBoardSizeNum; row++)
        for (int col = 1; col < kBoardSizeNum; col++)
        {
            // 前提是这个坐标是空的
            gameMapTmp=gameMapVec;
            if (gameMapVec[row][col] == 0&&canSearch(row,col))
            {
                weight=minMax(row,col,nextType(MAX_NODE),1,-INF,maxScore);
                scoreMapVec[row][col]=weight;
                if (weight > maxScore)          // 找最大的数和坐标
                {
                    maxPoints.clear();
                    maxScore = scoreMapVec[row][col];
                    maxPoints.push_back(std::make_pair(row, col));
                }
                else if (weight== maxScore)     // 如果有多个最大的数，都存起来
                    maxPoints.push_back(std::make_pair(row, col));
            }
        }

    // 随机落子，如果有多个点的话
    srand((unsigned)time(0));
    int index = rand() % maxPoints.size();

    std::pair<int, int> pointPair = maxPoints.at(index);
    clickRow = pointPair.first; // 记录落子点
    clickCol = pointPair.second;
    updateGameMap(clickRow, clickCol);
    lastrowAI=clickRow;
    lastcolAI=clickCol;
    gameMapTmp=gameMapVec;
}

/*判断是否获胜*/
bool GameModel::isWin(int row, int col)
{
    // 横竖斜四种大情况，每种情况都根据当前落子往后遍历5个棋子，有一种符合就算赢
    // 水平方向
    for (int i = 0; i < 5; i++)
    {
        // 往左5个，往右匹配4个子，20种情况
        if (col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 1] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 2] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 3] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 4])
            return true;
    }

    // 竖直方向(上下延伸4个)
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < kBoardSizeNum &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 1][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 2][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 3][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 4][col])
            return true;
    }

    // 左斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row + i < kBoardSizeNum &&
            row + i - 4 > 0 &&
            col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 1][col - i + 1] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 2][col - i + 2] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 3][col - i + 3] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 4][col - i + 4])
            return true;
    }

    // 右斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < kBoardSizeNum &&
            col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 1][col - i + 1] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 2][col - i + 2] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 3][col - i + 3] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 4][col - i + 4])
            return true;
    }

    return false;
}
/*判断棋盘是否被填满*/
bool GameModel::isDeadGame()
{
    // 所有空格全部填满
    for (int i = 1; i < kBoardSizeNum; i++)
        for (int j = 1; j < kBoardSizeNum; j++)
        {
            if (!(gameMapVec[i][j] == 1 || gameMapVec[i][j] == -1))
                return false;
        }
    return true;
}
/*悔棋功能的实现*/
void GameModel::retractGame()
{
    if (gameType == BOT){
        gameMapVec[lastrowAI][lastcolAI]=0;
        gameMapVec[lastrowPer][lastcolPer]=0;
    }
    else{
        gameMapVec[lastrowPer][lastcolPer]=0;
        gameMapVec[lastrowPer2][lastcolPer2]=0;
    }
}


