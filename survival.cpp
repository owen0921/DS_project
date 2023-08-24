#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <time.h>
#include <math.h>

#define SCREEN_HEIGHT 500 //設定遊戲視窗高度
#define SCREEN_WIDTH 500 //設定遊戲視窗寬度
#define GRID_SIDE 40 //設定遊戲方陣每邊格子數量
#define LEFT_MARGINE 30 //設定左邊界
#define TOP_MARGINE 40 //設定上邊界
#define RESOURCE_AMOUNT 1 //設定每次產生資源數量
#define PER_RESOURCE_KILL 5 //設定多少資源數量可以殺掉一隻喪屍
#define INIT_SPEED  1 //設定初始移動速度
#define MAX_QUEUE_SIZE 1600 //設定柱列大小

//宣告前進方向列舉函數
enum Direction {
    RIGHT,
    LEFT,
    UP,
    DOWN
};

//宣告遊戲場出現物體列舉函數
enum Object {
    EMPTY, //空白
    WALL, //牆或障礙物
    RESOURCE //資原
};

//宣告喪屍身體節點結構
struct Node {
    int row; //節點位在第幾行
    int col; //節點位在第幾列
    Direction direct; //該節點的前進方向
    struct Node *next;	//指向下一個節點
};

//定義指向節點結構的指標變數
typedef struct Node *NodePointer;

//定義座標結構
struct Location {
    int row;
    int col;
};

typedef struct PathNode *PathPointer;

//定義路徑節點結構，用來建立移動路徑
struct PathNode {
    int cost; //距離原點的步數
    int steps; //距離目標的步數
    Location loc;
    PathPointer parent;
    PathPointer next;
};

void openWindow(); //開啟游戲視窗
void closeGame(NodePointer dragon); //處理遊戲結束邏輯
int playGame(int field[][GRID_SIDE], NodePointer dragon, NodePointer player); //遊戲進行邏輯
bool IsGameOver(NodePointer zombie, NodePointer player, int field[][GRID_SIDE]); //(生存者死亡條件：撞牆和撞到喪屍)
int showGameOverMsg(); //遊戲結束訊息
void showInfo(); //顯示遊戲相關資訊
void drawGameField(int field[][GRID_SIDE]); //繪製遊戲區域
void drawSquare(int row, int col, int color); //繪製方塊
void controlZombieDirection(int field[][GRID_SIDE], NodePointer zombie, NodePointer player); //讀取AI輸入，並設定到所有喪屍節點
void controlPlayerDirection(int field[][GRID_SIDE], NodePointer player, NodePointer zombie); //讀取鍵盤方向輸入，或者AI輸入
void moveZombie(int field[][GRID_SIDE], NodePointer zombie); //繪製喪屍群前進一步的改變
void movePlayer(NodePointer player); //繪製生存者前進一步的改變
void createResource(int field[][GRID_SIDE], NodePointer zombie); //產生資源
bool IsAtWall(int field[][GRID_SIDE], int row, int col); //判斷是否撞到牆
bool IsAtZombie(NodePointer zombie, int row, int col); //判斷是否撞到喪屍的身體
bool IsCloseZombie(NodePointer zombie, int row, int col);//判斷是否撞到喪屍
void playerCollectResource(int field[][GRID_SIDE], NodePointer player, NodePointer zombie); //處理生存者收集到資源邏輯
void addZombie(int field[][GRID_SIDE], NodePointer zombie, NodePointer player); //增加喪屍數量
void killZombie(NodePointer zombie); //隨機殺掉一隻喪屍
Location nextStepLoc(NodePointer node, Direction direct); //計算下一步的座標
Location findNearestResource(int field[][GRID_SIDE], NodePointer zombie); //尋找最接近資源的座標
//生存者如果無法找到有效路徑，暫時決定一個安全方向
Direction safeDirect(int field[][GRID_SIDE], NodePointer player, NodePointer zombie);
//喪屍如果無法找到有效路徑，暫時決定一個安全方向
Direction safeDirect4Zombie(int field[][GRID_SIDE], NodePointer zombie);

//喪屍尋找兩點之間可到達的路徑，不需考慮會不會撞到其他喪屍或者生存者，只需考慮不能撞到牆
PathPointer zombieFindPath(int field[][GRID_SIDE], Location startLoc, Location goalLoc);
//生存者尋找兩點之間可到達的路徑，必須考慮不能撞到喪屍或者牆
PathPointer playerFindPath(int field[][GRID_SIDE], Location startLoc, Location goalLoc, NodePointer zombie);
void addPathQueue(PathNode pathNode); //將之後要拜訪的節點放入佇列裡
PathPointer popPathQueue(); //傳回路徑佇列中的元素，並將它從佇列中刪除
bool isPathQueueEmpty(); //判斷佇列是否為空
void resetPathQueue(); //重設佇列
void sortPathQueue(); //對佇列中的元素進行排序
bool IsInPathQueue(PathNode pathNode); //判斷該元素是否在佇列之中
PathPointer buildPath(PathPointer goal); //回傳到目標位置的路徑串列
int calcSteps(Location start, Location goal); //計算兩點之間需要移動的步數
bool visited(Location loc); //判斷是否該節點已經拜訪過
Direction getDirectionByPath(NodePointer zombie, PathPointer path); //從路徑資料判斷下一步方向

Direction zombieAI(int field[][GRID_SIDE], NodePointer zombie, Location target); //喪屍AI
Direction playerAI(int field[][GRID_SIDE], NodePointer player, NodePointer zombie); //生存者AI

struct PathNode pathQueue[MAX_QUEUE_SIZE]; //宣告將要拜訪的節點柱列
int front; //queue 第一個元素的前一個位置
int rear; //queue 最後一個元素的位置

int speed; //遊戲移動速度
int scoreSum = 0; //紀錄分數
int killedCount = 0; //殺死喪屍數量
int totalTime = 0; //紀錄遊戲時間
int stepCount = 0; //步數計數器
int const scorePerResource = 1; //每一份資源可得分數
bool IFPlayAI = true; //是否開啟AI模式

// 主程式
int main(){
    openWindow();

    while(TRUE){

        //設定遊戲場和障礙物
        int field[GRID_SIDE][GRID_SIDE] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                                           {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
                                           {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1},
                                           {1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,2,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                                           {1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
                                           {1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
                                           {1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1},
                                           {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                                           {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};

        Node headPlayer = {2, 4, RIGHT, NULL}; //設定勇者初始位置和方向
        Node headZombie = {15, 15, RIGHT, NULL}; //設定喪屍屍頭初始位置和方向
        NodePointer zombie = &headZombie;
        NodePointer player = &headPlayer;

        char key;
        key = playGame(field, zombie, player); //進行遊戲
        if (key == 'q' || key == 'Q')
            closeGame(zombie); //如果生存者輸入'q'離開遊戲
        else if (key == 's' || key == 'S')
            continue; //如果生存者輸入's' 繼續遊戲
    }
}

//開啟游戲視窗
void openWindow(){
    initwindow(SCREEN_WIDTH + LEFT_MARGINE * 3, SCREEN_HEIGHT + TOP_MARGINE * 3, "Hungry zombie Game");
}

//處理遊戲結束邏輯
void closeGame(NodePointer zombie){
    free(zombie);
    exit(0);
}

//遊戲進行邏輯
int playGame(int field[][GRID_SIDE], NodePointer zombie, NodePointer player) {
    speed = INIT_SPEED;
    stepCount = 0;
    killedCount = 0;
    bool killed = true;
    srand((unsigned)time(NULL)); //取目前系統時間作為亂數種子
    drawGameField(field); //繪製遊戲區域
    createResource(field, zombie); //產生第一份資源

    while(true){

        int key = 0;
        controlPlayerDirection(field, player, zombie);//讀取生存者輸入方向鍵，並將新方向設定到各喪屍節點
        movePlayer(player); //依據節點的方向，繪製新的喪屍位置

        if(stepCount % 2 == 0){
            controlZombieDirection(field, zombie, player);
            moveZombie(field, zombie); //依據節點的方向，繪製新的喪屍位置
        }

        //新增喪屍數量
        if(stepCount % 30 == 0)
            addZombie(field, zombie, player);

        playerCollectResource(field, player, zombie); //判斷生存者是否有收集到資源，如果有增加分數

        showInfo(); //顯示時間和分數資訊

        if(IsGameOver(zombie, player, field)) //判斷是否符合遊戲結束條件，
            return showGameOverMsg(); //顯示遊戲結束訊息，並等待生存者輸入選項

        //除了收集到資源會產生新資源，系統也隨機產生新資源
        if (rand() % 20 == 0)
            createResource(field, zombie);

        if(key != 0)
            return key;

        delay(speed); //決定生存者與喪屍移動速度，speed越小移動越快
        stepCount++;
        //讀取非方向鍵的其他鍵盤輸入
        if(kbhit()) {
            char key;
            key = getch();

            //只有輸入大小寫的a, q 和 s，系統才有反應並將輸入結果拋到外層等待處理
            if (key == 'q' || key == 'Q' || key == 's' || key == 'S')
                return key;
            else if (key == 'a') //決定是否改變模式 ，主要有生存者模式和電腦操控的AI模式
                IFPlayAI = !IFPlayAI;
        }
    }
}

//繪製遊戲區域，依據遊戲場矩陣設定繪製物件
void drawGameField(int field[][GRID_SIDE]){
    int row = 0, col = 0;
    cleardevice(); //清理螢幕畫面
    for(row = 0; row < GRID_SIDE; row ++){
        for(col = 0; col <  GRID_SIDE; col++){
            switch(field[row][col])	{
                case WALL: //牆在矩陣中的值是1
                    drawSquare(row, col, YELLOW);
                    break;
                case RESOURCE: //資源在矩陣中的值是2
                    drawSquare(row, col, GREEN);
                    break;
            }
        }
    }
}

//繪製方塊
void drawSquare(int row, int col, int color){

    int squareHeight = SCREEN_HEIGHT / GRID_SIDE;
    int SquareWidth = SCREEN_WIDTH / GRID_SIDE;
    int left = 0, right = 0, bottom = 0, top = 0;
    left = LEFT_MARGINE + col * SquareWidth + col;
    top =  TOP_MARGINE + row * squareHeight + row;
    right = left + SquareWidth;
    bottom = top + squareHeight;

    setfillstyle(SOLID_FILL,color); //設定繪製物件的為實心和顏色
    bar(left , top, right, bottom);	//設定繪製方塊的位置
}

//繪製喪屍每前進一步的改變
void moveZombie(int field[][GRID_SIDE], NodePointer zombie) {
    int currRow, currCol;

    while (zombie != NULL){
        currRow = zombie->row;
        currCol = zombie->col;

        if(field[currRow][currCol] == RESOURCE)
            drawSquare(currRow, currCol, GREEN);
        else
            drawSquare(currRow, currCol, BLACK);

        //依據節點的方向屬性，設定移動下一步的位置
        switch(zombie->direct){
            case RIGHT:
                zombie->col++;
                break;
            case LEFT:
                zombie->col--;
                break;
            case UP:
                zombie->row--;
                break;
            case DOWN:
                zombie->row++;
                break;
        }

        drawSquare(zombie->row, zombie->col, RED);

        zombie = zombie -> next;
    }
}

//繪製生存者每前進一步的改變
void movePlayer(NodePointer player) {
    int currRow, currCol;
    if (player != NULL){
        currRow = player->row;
        currCol = player->col;

        switch(player->direct){
            case RIGHT:
                player->col++;
                break;
            case LEFT:
                player->col--;
                break;
            case UP:
                player->row--;
                break;
            case DOWN:
                player->row++;
                break;
        }
        drawSquare(player->row, player->col, BLUE); //玩家的顏色
        drawSquare(currRow, currCol, BLACK); //玩家走過的路徑
    }
}

//判斷生存者是否死亡(死亡條件：撞牆和撞到自己身體)
bool IsGameOver(NodePointer zombie, NodePointer player, int field[][GRID_SIDE]){
    NodePointer head = zombie;

    //判斷是否撞到牆
    if (IsAtWall(field, head->row, head->col))
        return true;
    if (IsAtWall(field, player->row, player->col))
        return true;

    //檢查是否AI撞到喪屍
    if(player != NULL) {
        if(IsAtZombie(zombie, player->row, player->col))
            return true;
    }

    if(scoreSum == 30){
        return true;
    }

    return false;
}

//判斷是否撞到牆
bool IsAtWall(int field[][GRID_SIDE], int row, int col){
    if (field[row][col] == WALL)
        return true;
    return false;
}

//判斷是否撞到喪屍
bool IsAtZombie(NodePointer zombie, int row, int col){

    while(zombie != NULL){
        if (row == zombie->row && col == zombie->col)
            return true;
        zombie = zombie -> next;
    }
    return false;
}

//遊戲結束訊息
int showGameOverMsg(){
//    cleardevice(); //清理所有螢幕資料，如果希望只顯示訊息時，取消註解
    int i = 0;
    char msg1[15] = "Game Over!!";
    char msg2[40] = "press [q] to quit or [s] to restart!!";

    if(scoreSum == 30){
        char msg3[15] = "You Win!!!";
        int j = 0;
        for(j = 0; j < 15; j++){
            msg1[j] = msg3[j];
        }
        scoreSum = 0;
        killedCount = 0;
        totalTime = 0;
    }

    for(; ; i++){
        setcolor(i%14);
        settextstyle(TRIPLEX_FONT, HORIZ_DIR , 7);
        outtextxy(0, SCREEN_HEIGHT / 2, msg1);


        setcolor(WHITE);
        settextstyle(TRIPLEX_FONT, HORIZ_DIR , 2);
        outtextxy(20, SCREEN_HEIGHT / 2 + 70, msg2);

        delay(200);

        setcolor(BLACK);
        settextstyle(TRIPLEX_FONT, HORIZ_DIR , 7);
        outtextxy(0, SCREEN_HEIGHT / 2, msg1);

        if(kbhit()) {
            char key;
            key = getch();
            if (key == 'q' || key == 'Q' || key == 's' || key == 'S') {
                scoreSum = 0;
                return key;
            }
        }
    }
}

//顯示遊戲相關資訊
void showInfo(){
    totalTime += speed;
    char timeMsg[45] = " Time:";
    char scoreMsg[45] = "Score:";
    char killedMsg[50] = "Killed Zombie:";
    char modeMsg[20] = "";
    char optMsg1[50] = "press [q] to quit, [s] to restart or";
    char optMsg2[50] = "press [a] to change mode.";

    char time[10];
    char score[10];
    char killed[10];

    sprintf(time, "%5d", totalTime/1000);
    strcat(timeMsg, time);
    strcat(timeMsg, " sec.");

    setcolor(WHITE); //設定文字顏色
    settextstyle(COMPLEX_FONT, HORIZ_DIR , 1); //設定字型，水平或垂直和字型大小
    outtextxy(0, 0, timeMsg); //依據坐標輸出文字到螢幕

    sprintf(score, "%5d", scoreSum);
    strcat(scoreMsg, score);
    strcat(scoreMsg, " point.");

    setcolor(WHITE);
    settextstyle(COMPLEX_FONT, HORIZ_DIR , 1);
    outtextxy(0, 19, scoreMsg);

    sprintf(killed, "%3d", killedCount);
    strcat(killedMsg, killed);

    setcolor(WHITE);
    settextstyle(COMPLEX_FONT, HORIZ_DIR , 1);
    outtextxy(250, 19, killedMsg);

    if(IFPlayAI){
        strcat(modeMsg, "AI Mode    ");
    }else{
        strcat(modeMsg, "Player Mode");
    }

    setcolor(LIGHTMAGENTA); //模式轉換的字的顏色
    settextstyle(COMPLEX_FONT, HORIZ_DIR , 1);
    outtextxy(SCREEN_HEIGHT - LEFT_MARGINE * 2, 0, modeMsg);

    setcolor(LIGHTGREEN);
    settextstyle(COMPLEX_FONT, HORIZ_DIR , 1);
    outtextxy(0, TOP_MARGINE + (GRID_SIDE + 2) * SCREEN_HEIGHT / GRID_SIDE, optMsg1);
    outtextxy(0, TOP_MARGINE + (GRID_SIDE + 2) * SCREEN_HEIGHT / GRID_SIDE + 20, optMsg2);

}

//讀取鍵盤方向輸入，並設定到生存者節點
void controlPlayerDirection(int field[][GRID_SIDE], NodePointer player, NodePointer zombie) {

    Direction playerDirect = player -> direct;

    //get key code by pressing keybord
    int key;
    if(kbhit())
        key = getch();

    //decide zombie's moving direction
    switch(key){
        case KEY_RIGHT:
            playerDirect = RIGHT;
            break;
        case KEY_LEFT:
            playerDirect = LEFT;
            break;
        case KEY_UP:
            playerDirect = UP;
            break;
        case KEY_DOWN:
            playerDirect = DOWN;
            break;
    }

    if (IFPlayAI)
        playerDirect = playerAI(field, player, zombie);


    player -> direct = playerDirect;

}


//讀取鍵盤方向輸入，並設定到所有喪屍節點
void controlZombieDirection(int field[][GRID_SIDE], NodePointer zombie, NodePointer player) {
    int count = 0;
    while(zombie != NULL){
        Location target = {player -> row + count, player -> col + count};
        Direction zombieDirect = zombieAI(field, zombie, target);
        zombie -> direct = zombieDirect;
        zombie = zombie -> next;
        count += 2;
    }

}

//產生資源
void createResource(int field[][GRID_SIDE], NodePointer zombie){
    int row, col, i, amount = RESOURCE_AMOUNT;

    for(i=0; i< amount; i++){
        //如果亂數產生的位置是在牆和喪屍身體重疊，則重新產生，直到符合條件為止
        do{
            row = rand() % GRID_SIDE;
            col = rand() % GRID_SIDE;
        }while(IsAtWall(field, row, col) || IsAtZombie(zombie, row, col));

        field[row][col] = RESOURCE;
        drawSquare(row,col, GREEN);
    }
}


//系統處理生存者收集到資源邏輯
void playerCollectResource(int field[][GRID_SIDE], NodePointer player, NodePointer zombie){
    //如果生存者與資源位置重疊，就是收集到資源
    if(field[player->row][player->col] == RESOURCE){
        field[player->row][player->col] = EMPTY; //將該資源清空
        printf("The player has eaten food at row: %d, col: %d\n", player->row, player->col);
        scoreSum += scorePerResource; //紀錄分數
        createResource(field, player); //產生新的資源

        //收集一定數量的資源可以消滅一隻喪屍
        if(scoreSum % PER_RESOURCE_KILL == 0)
            killZombie(zombie);
    }
}

//增加喪屍數量
void addZombie(int field[][GRID_SIDE], NodePointer zombie, NodePointer player){
    int row, col;
    NodePointer tail = NULL;
    NodePointer newNode = (NodePointer) malloc(sizeof(Node)); //初始化一個新節點

    //尋找最後一個喪屍節點
    tail = zombie;
    while(tail->next != NULL){
        tail = tail -> next;
    }
    //將最後一位喪屍的方向屬性給新節點
    newNode -> direct = tail -> direct;

    do{
        row = rand() % GRID_SIDE;
        col = rand() % GRID_SIDE;
    }while(IsAtWall(field, row, col) || IsAtZombie(zombie, row, col) || (player->row == row && player->col == col));

    newNode -> row = row;
    newNode -> col = col;

    tail -> next = newNode; //將尾巴節點連接到新節點
    newNode -> next = NULL;
}

//殺掉一隻喪屍
void killZombie(NodePointer zombie){
    int i;
    NodePointer temp, killed;
    temp = zombie;

    killed = zombie;

    //不會殺光所有喪屍，至少會保留一隻
    if(zombie->next == NULL)
        return;
    while(killed->next != NULL){
        temp = killed;
        killed = killed -> next;
    }
    temp -> next = killed -> next;
    drawSquare(killed->row, killed->col, BLACK);
    printf("\n(%d, %d) is killed\n", killed->row, killed->col);
    free(killed);
    killedCount ++;
}

//喪屍的AI控制
Direction zombieAI(int field[][GRID_SIDE], NodePointer zombie, Location target) {
    Direction zombieDirect = zombie -> direct;
    Location start = {zombie -> row, zombie -> col};


    PathPointer path = zombieFindPath(field, start, target);
    if(path){
        zombieDirect = getDirectionByPath(zombie, path);
    }
    else
        zombieDirect = safeDirect4Zombie(field, zombie);

    return zombieDirect;
}

//從路徑資料判斷下一步方向
Direction getDirectionByPath(NodePointer head, PathPointer path){
    PathPointer nextPath = path->next;
    int horizontal = nextPath->loc.col - head->col;
    int vertical = nextPath->loc.row - head->row;
    if(horizontal == 1)
        return RIGHT;
    else if(horizontal == -1)
        return LEFT;

    if(vertical == 1)
        return DOWN;
    else if(vertical == -1)
        return UP;
    return 	head -> direct;
}

//喪屍如果無法找到有效路徑，暫時決定一個安全方向
Direction safeDirect4Zombie(int field[][GRID_SIDE], NodePointer zombie){
    int i;
    int dirSize = 4;
    Location loc;

    loc = nextStepLoc(zombie, UP);
    if(!IsAtWall(field, loc.row, loc.col))
        return UP;
    loc = nextStepLoc(zombie, DOWN);
    if(!IsAtWall(field, loc.row, loc.col))
        return DOWN;
    loc = nextStepLoc(zombie, RIGHT);
    if(!IsAtWall(field, loc.row, loc.col))
        return RIGHT;
    loc = nextStepLoc(zombie, LEFT);
    if(!IsAtWall(field, loc.row, loc.col))
        return LEFT;
    return zombie->direct;
}

//喪屍尋找兩點之間可到達的路徑，不需考慮會不會撞到其他喪屍或者生存者
PathPointer zombieFindPath(int field[][GRID_SIDE], Location startLoc, Location goalLoc){
    resetPathQueue();
    int steps = calcSteps(startLoc, goalLoc);
    PathNode start = {0, steps, startLoc, NULL, NULL};
    addPathQueue(start);
    while(!isPathQueueEmpty()){
        sortPathQueue();
        PathPointer current = popPathQueue();
        if(current->loc.row == goalLoc.row && current->loc.col == goalLoc.col)
            return buildPath(current);
        int dirSize = 4;
        int iDir[] = {1, 0, -1, 0};
        int jDir[] = {0, 1, 0, -1};
        int i,j;
        for(i=0, j=0; i<dirSize; i++, j++){
            Location neighborLoc = {current->loc.row + iDir[i], current->loc.col + jDir[j]};
            if(!visited(neighborLoc) && !IsAtWall(field, neighborLoc.row, neighborLoc.col)){
                int steps = calcSteps(neighborLoc, goalLoc);
                int cost = 	current->cost + 1;
                PathNode neighbor = {cost, steps, neighborLoc, current, NULL};
                if(!IsInPathQueue(neighbor)){
                    addPathQueue(neighbor);
                }
            }
        }
    }
    return NULL;
}

//將之後要拜訪的節點放入佇列裡
void addPathQueue(PathNode pathNode){
    if(rear == MAX_QUEUE_SIZE - 1){
        printf("the queue is full");
        return;
    }
    rear += 1;
    pathQueue[rear]	= pathNode;
}

//傳回佇列中的路徑座標節點，並將它從佇列中刪除
PathPointer popPathQueue(){
    if(front == rear){
        printf("the queue is empty");
        return NULL;
    }
    front ++;
    PathPointer node = (PathPointer)malloc(sizeof(PathNode));
    node->cost = pathQueue[front].cost;
    node->steps = pathQueue[front].steps;
    node->loc = pathQueue[front].loc;
    node->parent = pathQueue[front].parent;
    node->next = pathQueue[front].next;
    return node;
}

//判斷佇列是否為空
bool isPathQueueEmpty(){
    return front == rear;
}

//重設佇列
void resetPathQueue(){
    front = -1;
    rear = -1;
}

//對佇列中的元素進行排序
void sortPathQueue(){
    if(front == rear)
        return;
    int i, j;
    int nowTotal, nextTotal;
    for(i=front + 1; i<rear; i++){
        for(j=i+1; j<=rear; j++){

            nowTotal = pathQueue[i].cost + pathQueue[i].steps;
            nextTotal = pathQueue[j].cost + pathQueue[j].steps;

            if(nowTotal > nextTotal){
                PathNode temp =  pathQueue[i];
                pathQueue[i] = pathQueue[j];
                pathQueue[j] = temp;
            }
        }
    }
}

//判斷該元素是否在佇列之中
bool IsInPathQueue(PathNode pathNode){
    int i;
    if(front == rear)
        return false;
    for(i=front;i<=rear;i++){
        if(pathQueue[i].loc.row == pathNode.loc.row && pathQueue[i].loc.col == pathNode.loc.col)
            return true;
    }
    return false;
}

//回傳到目標位置的路徑串列
PathPointer buildPath(PathPointer goal){
    printf("buildPath ");
    printf("(%d, %d)\n", goal->loc.row, goal->loc.col);
    if(goal == NULL || goal->parent == NULL)
        return NULL;
    PathPointer head = goal;
    head->next = NULL;
    PathPointer temp = head;

    while(head->parent){
        if(head == NULL)
            return NULL;
        printf("node (%d, %d)->", head->loc.row, head->loc.col);
        head = head->parent;
        head->next = temp;
        temp = head;
    }
    printf("NULL\n");
    return head;
}

//計算兩點之間需要移動的步數
int calcSteps(Location start, Location goal){
    int steps = abs(start.row - goal.row) + abs(start.col - goal.col);
    return steps;
}

//判斷是否該節點已經拜訪過
bool visited(Location loc){
    int i;
    for(i=0;i<=front;i++){
        if(pathQueue[i].loc.row == loc.row && pathQueue[i].loc.col == loc.col)
            return true;
    }
    return false;
}

//尋找最接近資源
Location findNearestResource(int field[][GRID_SIDE], NodePointer me){
    int rowDis, colDis, row, col, nearest = 100000;
    Location nearestFood = {-1, -1};
    for(row = 0; row < GRID_SIDE; row++){
        for(col = 0; col < GRID_SIDE; col++){
            if(field[row][col] == RESOURCE){
                rowDis = abs(row - me -> row);
                colDis = abs(col - me -> col);
                if(nearest > (rowDis + colDis)){
                    nearest = (rowDis + colDis);
                    nearestFood.row = row;
                    nearestFood.col = col;
                }
            }
        }
    }
    return nearestFood;
}


//生存者如果無法找到有效路徑，暫時決定一個安全方向
Direction safeDirect(int field[][GRID_SIDE], NodePointer player, NodePointer zombie){
    int i;
    int dirSize = 4;
    Location loc;

    loc = nextStepLoc(player, UP);
    if(!IsAtWall(field, loc.row, loc.col) && !IsCloseZombie(zombie, loc.row, loc.col))
        return UP;
    loc = nextStepLoc(player, DOWN);
    if(!IsAtWall(field, loc.row, loc.col) && !IsCloseZombie(zombie, loc.row, loc.col))
        return DOWN;
    loc = nextStepLoc(player, RIGHT);
    if(!IsAtWall(field, loc.row, loc.col) && !IsCloseZombie(zombie, loc.row, loc.col))
        return RIGHT;
    loc = nextStepLoc(player, LEFT);
    if(!IsAtWall(field, loc.row, loc.col) && !IsCloseZombie(zombie, loc.row, loc.col))
        return LEFT;
    return player->direct;
}

//計算下一步的座標
Location nextStepLoc(NodePointer node, Direction direct){
    int currRow =  node -> row;
    int currCol = node -> col;
    int nextRow, nextCol;
    Location loc;
    switch(direct){
        case RIGHT:
            nextRow = currRow;
            nextCol = currCol + 1;
            break;
        case LEFT:
            nextRow = currRow;
            nextCol = currCol - 1;
            break;
        case UP:
            nextRow = currRow - 1;
            nextCol = currCol;
            break;
        case DOWN:
            nextRow = currRow + 1;
            nextCol = currCol;
            break;
    }
    loc.row = nextRow;
    loc.col = nextCol;
    return loc;
}

//生存者尋找兩點之間可到達的路徑，必須考慮會不會撞到牆或者喪屍
PathPointer playerFindPath(int field[][GRID_SIDE], Location startLoc, Location goalLoc, NodePointer zombie){
    resetPathQueue();
    int steps = calcSteps(startLoc, goalLoc);
    PathNode start = {0, steps, startLoc, NULL, NULL};
    addPathQueue(start);
    while(!isPathQueueEmpty()){
        sortPathQueue();
        PathPointer current = popPathQueue();
        if(current == NULL)
            return NULL;
        if(current->loc.row == goalLoc.row && current->loc.col == goalLoc.col)
            return buildPath(current);
        int dirSize = 4;
        int iDir[] = {1, 0, -1, 0};
        int jDir[] = {0, 1, 0, -1};
        int i,j;
        for(i=0, j=0; i<dirSize; i++, j++){
            Location neighborLoc = {current->loc.row + iDir[i], current->loc.col + jDir[j]};
            if(!visited(neighborLoc) && !IsAtWall(field, neighborLoc.row, neighborLoc.col) && !IsCloseZombie(zombie, neighborLoc.row, neighborLoc.col)){
                int steps = calcSteps(neighborLoc, goalLoc);
                int cost = 	current->cost + 1;
                PathNode neighbor = {cost, steps, neighborLoc, current, NULL};
                if(!IsInPathQueue(neighbor)){
                    addPathQueue(neighbor);
                }
            }
        }
    }
    return NULL;
}

//判斷是否會撞到喪屍
bool IsCloseZombie(NodePointer zombie, int row, int col){

    if(zombie == NULL)
        return NULL;
    NodePointer head = zombie;
    while(zombie != NULL){
        if (row == zombie->row && col == zombie->col)
            return true;
        zombie = zombie -> next;
    }
    int dirSize = 4;
    int iDir[] = {1, 0, -1, 0};
    int jDir[] = {0, 1, 0, -1};
    int i,j;
    while(head != NULL){
        for(i=0, j=0; i<dirSize; i++, j++){
            if (row == (head->row + iDir[i]) && (col == head->col  + jDir[j]))
                return true;
        }
        head = head -> next;
    }

    return false;
}

//實作生存者AI
Direction playerAI(int field[][GRID_SIDE], NodePointer player, NodePointer zombie){
    Direction playerDirection = player -> direct;
    Location start = {player -> row, player -> col};

    Location target = findNearestResource(field, zombie);


    PathPointer path = playerFindPath(field, start, target, zombie);
    if(path){
        playerDirection = getDirectionByPath(player, path);
    }
    else
        playerDirection = safeDirect(field, player, zombie);

    return playerDirection;
}
