#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <conio.h> 

// BGM
#define NOTE_F5  700
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_AS5 932 // A# (A Sharp)
#define REST      0  // 쉼표

// 효과음
#define NOTE_C6 1047
#define NOTE_E6 1319
#define NOTE_G6 1568
#define NOTE_G4 392
#define NOTE_FS4 370
#define NOTE_C4 262
#define NOTE_B3 247
#define NOTE_A3 220
#define NOTE_E4 330
#define NOTE_C5 523

volatile BOOL g_is_music_playing = TRUE;

void draw_rectangle(int start_x, int start_y, int c, int r);
void display_abacus(int m[], int show_numbers);
void div_number(int mc[], long number);
void gotoxy(int x, int y);
int game_mode(int show_answer_mode); // 게임 모드 함수
void draw_hangman(int lives); // 행맨 그리는 함수
void clear_screen(); // 화면 지우는 함수
DWORD WINAPI play_music(LPVOID lpParam); // BGM을 재생할 스레드 함수

void play_success_sound();
void play_failure_sound();
void play_gameover_sound();
void show_intro_screen();
char show_outro_screen(int score); 

int main(void)
{
    SetConsoleOutputCP(65001);
    srand(time(NULL));

    while(1) // 게임 재시작을 위한 메인 루프
    {
        // --- 인트로 화면 표시 ---
        show_intro_screen();

        // --- BGM 재생 스레드 시작 ---
        g_is_music_playing = TRUE;
        HANDLE hThread = CreateThread(NULL, 0, play_music, NULL, 0, NULL);

        int mode = 0;
        while (mode < 1 || mode > 3)
        {
            clear_screen();
            gotoxy(1, 2);
            printf("모드를 선택하세요:\n");
            printf("1. 주판 연습 모드\n");
            printf("2. 주판 읽기 게임\n");
            printf("3. 정답 보기 모드\n");
            printf("> ");
            scanf("%d", &mode);
            while(getchar() != '\n');
        }

        if (mode == 2 || mode == 3) {
            int score = game_mode(mode == 3 ? 1 : 0);
            
            // 게임 오버 처리
            g_is_music_playing = FALSE;
            play_gameover_sound();
            char choice = show_outro_screen(score);
            
            if (choice == 'x' || choice == 'X') {
                if (hThread != NULL) {
                    WaitForSingleObject(hThread, 1000);
                    CloseHandle(hThread);
                }
                break;
            }
        } else {
            // --- 기존의 주판 연습 모드 ---
            clear_screen();
            long number = 0;
            int mc[9];

            printf("입력된 숫자를 주판모양으로 출력\n");
            printf("0을 입력하면 종료\n");

            div_number(mc, number);
            draw_rectangle(1, 5, 32, 14);
            display_abacus(mc, 1);

            do
            {
                gotoxy(1, 21);
                printf("현재 값: %-9ld\n", number);
                gotoxy(1, 22);
                printf("최대 9자리 정수만 입력하고 Enter>           ");
                gotoxy(34, 22);

                if (scanf("%ld", &number) != 1) { 
                    while(getchar() != '\n'); 
                    number = -1; 
                }

                if (number == 0)
                    break;

                div_number(mc, number);
                display_abacus(mc, 1);

            } while (number >= 0);
            
            g_is_music_playing = FALSE;
            break;
        } 
        if (hThread != NULL) {
            WaitForSingleObject(hThread, 1000);
            CloseHandle(hThread);
        }
    }
    
    return 0;
}

// 그래픽 인트로 화면을 표시하는 함수
void show_intro_screen() {
    clear_screen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    const int BROWN = FOREGROUND_RED | FOREGROUND_GREEN;
    const int YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const int WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const int CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    // 1. 행맨 구조물 그리기 애니메이션
    SetConsoleTextAttribute(hConsole, BROWN);
    gotoxy(25, 6);  printf(" _________");
    Sleep(100);
    gotoxy(25, 7);  printf(" |/      |");
    Sleep(100);
    gotoxy(25, 8);  printf(" |");
    Sleep(100);
    gotoxy(25, 9);  printf(" |");
    Sleep(100);
    gotoxy(25, 10); printf(" |");
    Sleep(100);
    gotoxy(25, 11); printf(" |");
    Sleep(100);
    gotoxy(25, 12); printf(" |");
    Sleep(100);
    gotoxy(25, 13); printf("_|___");
    Sleep(100);

    // 2. 주판 행맨 그리기 애니메이션
    SetConsoleTextAttribute(hConsole, YELLOW);
    gotoxy(35, 8);  printf(" ●");
    Beep(NOTE_G4, 100);
    Sleep(150);
    gotoxy(34, 9);  printf("─┼─");
    Beep(NOTE_G4, 100);
    Sleep(150);
    gotoxy(35, 10); printf(" │");
    Beep(NOTE_G4, 100);
    Sleep(150);
    gotoxy(34, 11); printf("/ \\");
    Beep(NOTE_A3, 200);
    Sleep(150);

    // 3. 타이틀 텍스트 애니메이션
    SetConsoleTextAttribute(hConsole, CYAN);
    char title[] = "JUPAN HANGMAN";
    gotoxy(28, 16);
    for (int i=0; i<strlen(title); i++){
        printf("%c", title[i]);
        Beep(NOTE_C6, 20);
        Sleep(50);
    }
    
    // 4. 시작 안내
    SetConsoleTextAttribute(hConsole, WHITE);
    gotoxy(28, 20);
    printf("Press Enter to Start...");
    while(getchar() != '\n');

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// 게임 오버 아웃트로 화면
char show_outro_screen(int score) {
    clear_screen();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
    const int WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    SetConsoleTextAttribute(hConsole, RED);
    gotoxy(12, 8); printf(" GGGGG   AAAAA   MM   MM  EEEEEE   OOOOO   VV     VV  EEEEEE  RRRRR  ");
    gotoxy(12, 9); printf("GG      AA   AA  MMM MMM  EE      OO   OO   VV   VV   EE      RR  RR ");
    gotoxy(12, 10); printf("GG GGG  AAAAAAA  MM M MM  EEEE    OO   OO    VV VV    EEEE    RRRRR  ");
    gotoxy(12, 11); printf("GG   GG  AA   AA  MM   MM  EE      OO   OO     VVV     EE      RR RR  ");
    gotoxy(12, 12); printf(" GGGGG   AA   AA  MM   MM  EEEEEE   OOOOO      V      EEEEEE  RR  RR ");

    SetConsoleTextAttribute(hConsole, WHITE);
    gotoxy(32, 16);
    printf("최종 점수: %d", score);
    
    gotoxy(25, 20);
    printf("Press Enter to Restart, or 'x' to Exit...");
    
    char choice = 0;
    while(1) {
        choice = _getch();
        if (choice == 13 || choice == 'x' || choice == 'X') {
            return choice;
        }
    }
}


// BGM을 백그라운드에서 재생하는 스레드 함수
DWORD WINAPI play_music(LPVOID lpParam) {
    int melody[] = {
        NOTE_G5, NOTE_G5, NOTE_G5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5, NOTE_F5,
        REST,
        NOTE_G5, NOTE_G5, NOTE_G5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_A5, NOTE_G5,
        REST
    };
    int durations[] = {
        150, 150, 150, 300, 300, 200, 200, 200,
        100,
        150, 150, 150, 300, 300, 200, 200, 200,
        400
    };
    int notes = sizeof(melody) / sizeof(melody[0]);
    int note_gap = 50;

    while (g_is_music_playing) {
        for (int i = 0; i < notes; i++) {
            if (!g_is_music_playing) break;
            
            if (melody[i] == REST) {
                Sleep(durations[i]); 
            } else {
                Beep(melody[i], durations[i]);
                Sleep(note_gap);
            }
        }
    }
    return 0;
}

// 성공 효과음
void play_success_sound() {
    Beep(NOTE_C6, 100);
    Beep(NOTE_E6, 100);
    Beep(NOTE_G6, 100);
}

// 실패 효과음
void play_failure_sound() {
    Beep(NOTE_G4, 150);
    Beep(NOTE_FS4, 150);
}

// 게임오버 효과음
void play_gameover_sound() {
    Beep(NOTE_C4, 300);
    Beep(NOTE_B3, 300);
    Beep(NOTE_A3, 500);
}


// 주판 읽기 게임 모드 함수
int game_mode(int show_answer_mode) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const int RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
    const int WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    int score = 0;
    int lives = 6;
    long question;
    int time_limit;

    while (lives > 0) {
        clear_screen();
        
        // --- UI 그리기 ---
        // 1. 주판 영역 사각형 그리기
        draw_rectangle(1, 5, 32, 14);
        // 2. 행맨 영역 사각형 그리기
        draw_rectangle(35, 5, 20, 14);

        // 3. 행맨 그리기
        draw_hangman(lives); 

        int difficulty = score / 5;
        long min_num = 1000;
        for (int i = 0; i < difficulty; i++) {
            if (min_num >= 100000000) break;
            min_num *= 10;
        }
        long max_num = min_num * 10 - 1;
        if (max_num > 999999999) max_num = 999999999;
        
        if (max_num > RAND_MAX) {
            question = ((long)rand() * (RAND_MAX + 1) + rand()) % (max_num - min_num + 1) + min_num;
        } else {
            question = (rand() % (max_num - min_num + 1)) + min_num;
        }
        
        time_limit = 10 - difficulty;
        if (time_limit < 3) time_limit = 3;

        int mc[9] = {0};
        div_number(mc, question);
        display_abacus(mc, 0);
        
        gotoxy(2, 22);
        printf("점수: %-5d 남은 목숨: %d", score, lives);
        if (show_answer_mode) {
            printf(" (정답: %ld)", question);
        }

        gotoxy(2, 23);
        printf("주판이 나타내는 숫자는?");
        gotoxy(2, 24);
        printf("정답 입력> ");

        // --- 실시간 입력 및 타이머 처리 ---
        char answer_str[20] = {0};
        int char_index = 0;
        long answer = 0;
        BOOL timed_out = TRUE;

        time_t start_time = time(NULL);
        double remaining_time = time_limit;

        gotoxy(14, 24);

        while (remaining_time > 0) {
            // 남은 시간 표시
            gotoxy(28, 23);
            printf("(남은 시간: %2.0f초)  ", remaining_time);
            gotoxy(14 + char_index, 24);

            if (_kbhit()) {
                char ch = _getch();
                if (ch >= '0' && ch <= '9' && char_index < 10) {
                    answer_str[char_index++] = ch;
                    printf("%c", ch);
                } else if (ch == 8 && char_index > 0) {
                    char_index--;
                    gotoxy(14 + char_index, 24);
                    printf(" ");
                    gotoxy(14 + char_index, 24);
                } else if (ch == 13) {
                    timed_out = FALSE;
                    break;
                }
            }
            Sleep(100);
            remaining_time = time_limit - difftime(time(NULL), start_time);
        }

        answer_str[char_index] = '\0'; 
        answer = atol(answer_str);
        
        // --- 정답 확인 ---
        gotoxy(2, 25);
        printf("                                                                    ");
        
        gotoxy(2, 25);
        if (!timed_out && answer == question) {
            printf("정답입니다! +1점!");
            play_success_sound();
            score++;
            Sleep(2000);
        } else {
            lives--;
            if(timed_out) {
                printf("시간 초과! (정답: %ld)", question);
            } else {
                printf("오답입니다! (정답: %ld)", question);
            }
            
            play_failure_sound();
            Sleep(500);

            // --- 빨간색 깜빡임 효과 ---
            SetConsoleTextAttribute(hConsole, RED);
            display_abacus(mc, 0); 
            draw_hangman(lives);   
            Sleep(1000); 

            SetConsoleTextAttribute(hConsole, WHITE);
        }
    }

    return score;
}


// 행맨을 그리는 함수
void draw_hangman(int lives) {
    gotoxy(40, 7);  printf(" _________");
    gotoxy(40, 8);  printf(" |/      |");
    gotoxy(40, 9);  printf(" |      %c", lives < 6 ? 'O' : ' ');
    gotoxy(40, 10); printf(" |     %c%c%c", lives < 4 ? '/' : ' ', lives < 5 ? '|' : ' ', lives < 3 ? '\\' : ' ');
    gotoxy(40, 11); printf(" |      %c", lives < 5 ? '|' : ' ');
    gotoxy(40, 12); printf(" |     %c %c", lives < 2 ? '/' : ' ', lives < 1 ? '\\' : ' ');
    gotoxy(40, 13); printf(" |");
    gotoxy(40, 14); printf("_|___");
}

// 화면을 지우는 함수
void clear_screen() {
    system("cls");
}

// 입력된 숫자를 자릿수별로 분해하는 함수
void div_number(int mc[], long number)
{
    int i;
    if (number > 999999999) {
        number = 999999999;
    }
    for (i = 0; i <= 8; i++)
    {
        mc[i] = (int)(number / pow(10, 8 - i));
        number = number - (long)pow(10, 8 - i) * mc[i];
    }
}

// 주판을 화면에 표시하는 함수
void display_abacus(int m[], int show_numbers)
{
    int i, j;
    if (show_numbers) {
        gotoxy(3, 4);
        for (i = 0; i <= 8; i++)
            printf("%2d ", m[i]);
    }

    for (i = 0; i < 2; i++) {
        gotoxy(3, 6 + i);
        for (j = 0; j <= 8; j++) {
            int is_upper_bead_down = (m[j] >= 5 && i == 1);
            int is_upper_bead_up = (m[j] < 5 && i == 0);
            if (is_upper_bead_down || is_upper_bead_up) {
                printf(" ● "); 
            } else {
                printf(" │ "); 
            }
        }
    }

    gotoxy(3, 8);
    for (i = 0; i < 9; i++)
        printf("───");

    for (i = 1; i <= 4; i++) {
        gotoxy(3, 8 + i);
        for (j = 0; j <= 8; j++) {
            int lower_val = m[j] % 5;
            if (i <= lower_val) {
                printf(" ● ");
            } else {
                printf(" │ ");
            }
        }
    }
    
    for (i = 1; i <= 4; i++) {
        gotoxy(3, 13 + i);
        for (j = 0; j <= 8; j++) {
            int lower_val = m[j] % 5;
             if (i > lower_val) {
                printf(" ● ");
            } else {
                printf(" │ ");
            }
        }
    }
}

// 사각형 테두리를 그리는 함수
void draw_rectangle(int start_x, int start_y, int c, int r)
{
    int i;
    gotoxy(start_x, start_y);
    printf("┌");
    for (i = 0; i < c; i++) printf("─");
    printf("┐");

    for (i = 0; i < r; i++) {
        gotoxy(start_x, start_y + 1 + i);
        printf("│");
        gotoxy(start_x + c + 1, start_y + 1 + i);
        printf("│");
    }

    gotoxy(start_x, start_y + r + 1);
    printf("└");
    for (i = 0; i < c; i++) printf("─");
    printf("┘");
}

// 커서 위치를 이동시키는 함수
void gotoxy(int x, int y)
{
    COORD Pos = {x - 1, y - 1};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
