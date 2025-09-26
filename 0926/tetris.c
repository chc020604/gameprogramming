#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

// 게임 보드 크기 정의
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

// 게임 보드 시작 위치 정의
#define START_X 12
#define START_Y 1

// 전역 변수 선언
int board[BOARD_HEIGHT][BOARD_WIDTH] = {0}; // 게임 보드판 (0: 빈 공간, 1: 쌓인 블록)
int gameOver = 0;                           // 게임 오버 상태 (0: 진행 중, 1: 종료)

int current_x, current_y;   // 현재 블록의 x, y 좌표
int current_block_type;     // 현재 블록의 종류 (0~6)
int current_rotation;       // 현재 블록의 회전 상태 (0~3)

// 7가지 테트로미노 블록 모양 정의 [블록종류][회전상태][y][x]
const int tetromino[7][4][4][4] = {
    // I 블록
    {{{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}},
     {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}},
    // O 블록
    {{{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}},
    // T 블록
    {{{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}},
    // L 블록
    {{{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
     {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}},
    // J 블록
    {{{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
     {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}},
    // S 블록
    {{{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
     {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}}},
    // Z 블록
    {{{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
     {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
     {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}}}
};

// 함수 프로토타입 선언
void gotoxy(int x, int y);
void setCursorVisible(int visible);
void drawBoard();
void drawBlock(int x, int y, int block_type, int rotation, char* icon);
int checkCollision(int x, int y, int block_type, int rotation);
void lockBlock();
void clearLines();
void createNewBlock();
void gameLoop();

// 콘솔 커서 위치 이동 함수
void gotoxy(int x, int y) {
    COORD Pos = { (short)x * 2, (short)y }; // x좌표는 2배로 하여 정사각형처럼 보이게 함
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

// 콘솔 커서 보이기/숨기기 함수
void setCursorVisible(int visible) {
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(out, &cursorInfo);
}

// 게임 보드 테두리 그리기 함수 (수정됨)
void drawBoard() {
    int i, j;
    // 상단과 하단 테두리 그리기
    for (j = 0; j < BOARD_WIDTH + 2; j++) {
        gotoxy(START_X + j - 1, START_Y - 1);
        printf("■");
        gotoxy(START_X + j - 1, START_Y + BOARD_HEIGHT);
        printf("■");
    }
    // 좌측과 우측 테두리 그리기
    for (i = 0; i < BOARD_HEIGHT; i++) {
        gotoxy(START_X - 1, START_Y + i);
        printf("■");
        gotoxy(START_X + BOARD_WIDTH, START_Y + i);
        printf("■");
    }
}

// 블록 그리기 및 지우기 함수
void drawBlock(int x, int y, int block_type, int rotation, char* icon) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetromino[block_type][rotation][i][j] == 1) {
                gotoxy(START_X + x + j, START_Y + y + i);
                printf("%s", icon);
            }
        }
    }
}

// 충돌 검사 함수
int checkCollision(int x, int y, int block_type, int rotation) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetromino[block_type][rotation][i][j] == 1) {
                int board_x = x + j;
                int board_y = y + i;
                if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT) {
                    return 1; // 벽에 충돌
                }
                if (board[board_y][board_x] == 1) {
                    return 1; // 다른 블록에 충돌
                }
            }
        }
    }
    return 0; // 충돌 없음
}

// 블록을 보드에 고정시키는 함수
void lockBlock() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (tetromino[current_block_type][current_rotation][i][j] == 1) {
                board[current_y + i][current_x + j] = 1;
            }
        }
    }
}

// 완성된 라인 지우기 함수
void clearLines() {
    int i, j, k;
    int full_line;
    for (i = BOARD_HEIGHT - 1; i >= 0; i--) {
        full_line = 1;
        for (j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == 0) {
                full_line = 0;
                break;
            }
        }

        if (full_line) {
            for (k = i; k > 0; k--) {
                for (j = 0; j < BOARD_WIDTH; j++) {
                    board[k][j] = board[k - 1][j];
                }
            }
            // 가장 윗 줄은 비워줌
            for (j = 0; j < BOARD_WIDTH; j++) {
                board[0][j] = 0;
            }
            // 한 줄을 지웠으므로 같은 줄을 다시 검사하기 위해 i를 증가
            i++; 
            
            // 지운 후 화면 다시 그리기
            for(int row = 0; row < BOARD_HEIGHT; row++){
                for(int col = 0; col < BOARD_WIDTH; col++){
                    gotoxy(START_X + col, START_Y + row);
                    printf(board[row][col] ? "■" : " ");
                }
            }
        }
    }
}

// 새로운 블록 생성 함수
void createNewBlock() {
    current_x = BOARD_WIDTH / 2 - 2; // 중앙에서 시작
    current_y = 0;
    current_block_type = rand() % 7;
    current_rotation = rand() % 4;

    if (checkCollision(current_x, current_y, current_block_type, current_rotation)) {
        gameOver = 1;
    }
}

// 메인 게임 루프 함수
void gameLoop() {
    int frame_count = 0;
    int speed = 20;

    createNewBlock();

    while (!gameOver) {
        // 입력 처리
        if (_kbhit()) {
            char key = _getch();
            drawBlock(current_x, current_y, current_block_type, current_rotation, "  "); // 현재 블록 지우기
            switch (key) {
                case 75: // 왼쪽 화살표
                    if (!checkCollision(current_x - 1, current_y, current_block_type, current_rotation)) {
                        current_x--;
                    }
                    break;
                case 77: // 오른쪽 화살표
                    if (!checkCollision(current_x + 1, current_y, current_block_type, current_rotation)) {
                        current_x++;
                    }
                    break;
                case 72: // 위쪽 화살표 (회전)
                    if (!checkCollision(current_x, current_y, current_block_type, (current_rotation + 1) % 4)) {
                        current_rotation = (current_rotation + 1) % 4;
                    }
                    break;
                case 80: // 아래쪽 화살표 (빨리 내리기)
                    if (!checkCollision(current_x, current_y + 1, current_block_type, current_rotation)) {
                        current_y++;
                    }
                    break;
            }
        }

        // 시간 경과에 따른 블록 하강
        if (frame_count % speed == 0) {
            drawBlock(current_x, current_y, current_block_type, current_rotation, "  "); // 현재 블록 지우기
            if (!checkCollision(current_x, current_y + 1, current_block_type, current_rotation)) {
                current_y++;
            } else {
                lockBlock();
                clearLines();
                createNewBlock();
            }
        }

        drawBlock(current_x, current_y, current_block_type, current_rotation, "■"); // 블록 그리기
        Sleep(50); // 게임 속도 조절
        frame_count++;
    }
}

// 프로그램 시작점 (main 함수)
int main() {
    srand((unsigned int)time(NULL)); // 난수 시드 초기화
    setCursorVisible(0);             // 커서 숨기기
    system("cls");                   // 화면 지우기
    drawBoard();                     // 게임 보드 그리기
    
    gameLoop();                      // 게임 루프 시작

    // 게임 오버 메시지
    gotoxy(START_X + BOARD_WIDTH / 2 - 4, START_Y + BOARD_HEIGHT / 2);
    printf("GAME OVER");

    gotoxy(0, START_Y + BOARD_HEIGHT + 2); // 커서를 아래로 이동시켜 프롬프트가 깨끗하게 보이도록 함
    return 0;
}