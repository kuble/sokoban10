#include <stdio.h>
#include <termio.h>
#include <windows.h>

char name[10]; // 사용자 이름을 저장하는 배열
char map[5][30][30]; // 전체 맵을 저장하는 배열
char cMap[30][30]; // 현재 맵만을 저장하는 배열
char uMap[5][30][30]; // 되돌리기 기능을 위해 바로 전 맵 형태를 저장하는 배열
char rName[10]; // 랭킹을 출력할 때 사용할 이름

int cIndex = 0; // 현재 맵 번호를 표시하기 위한 변수
int plX, plY; // '@'의 위치를 나타내기 위한 변수
int cStorage = 0, err = 0; // 레벨 클리어를 판단하기 위한 변수
int mvCnt = 0; // 움직인 횟수 기록을 위한 변수
int uPl[5][2]; // 되돌릴 맵의 '@' 위치를 표시하기 위한 배열
int rIndex, rMvCnt; // 랭킹을 출력할 때 사용할 맵 인덱스와 이동횟수

_Bool nextStage = 0; // 다음 스테이지로 넘어갈지를 판단하는 불린 변수

void createMapArray();
int checkMap();
void createCurrentMap();
void printMap();
int getch();
void recordUndo();
void Undo();
void moveCommand(char);
void createRangkingFile();
void command();
void saveMap();
void loadMap();
void inputRangking();
void loadAllRangking();
void loadMapRangking(char);
void resetUndo();
void manual();

int main(void) {
    system("clear");
    printf("이름을 입력하세요 : ");
    scanf("%s", &name);
    createMapArray(); // 맵 배열을 생성함
    if (checkMap() == 1) { // 1을 리턴받을 경우 오류 메세지를 출력하고 프로그램을 종료함
        printf("잘못된 맵입니다. 프로그램을 종료합니다.\n");
        return 0;
    };
    createCurrentMap();
    createRangkingFile();
    system("clear");
    printMap(); // 맵을 출력함
    command(); // 명령어를 입력받음
    return 0;
}

void createMapArray() { // 전체 맵을 3차원 배열로 저장함
    char buffer[30];
    FILE *fp;
    fp = fopen("easymap.txt", "r");
    int l = 0, index = 0;
    while (1) {
        fscanf(fp, "%s", &buffer); // map.txt 파일을 한 줄씩 읽어들임
        if (buffer[0] == 'e') { // 읽어들인 줄이 'e'일 경우
            break;
        } else if (buffer[0] >= 49 && buffer[0] <= 53) { // 읽어들인 줄이 1 ~ 5일 경우
            index = buffer[0] - 49;
            l = 0;
        } else {
            for (int i = 0; i < 30; i++) {
                map[index][l][i] = buffer[i];
            }
            l += 1;
        }
    }
    fclose(fp);
}

int checkMap() { // 박스 갯수와 창고 갯수를 비교
    for (int i = 0; i < 5; i++) {
        int b = 0, s = 0; // 박스 갯수, 창고 갯수를 세기 위한 변수
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 30; k++) { // 요소 하나하나를 스캔
                if (map[i][j][k] == '$') { 
                    b++; 
                } else if (map[i][j][k] == 'O') {
                    s++;
                }
            }
        }
        if (b != s) {
            return 1; // 박스 갯수와 창고 갯수가 다르다면 1을 리턴함
        }
    }
}

void createCurrentMap() { // 현재 맵만을 저장
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 30; j++) {
            if (map[cIndex][i][j] == '@') {
                plX = i, plY = j;
            }
            cMap[i][j] = map[cIndex][i][j];
        }
    }
}

void createRangkingFile() { // 게임 시작시 랭킹 파일이 없을 경우 랭킹 파일을 생성함
    FILE *fp;
    fp = fopen("rangking.txt", "r");
    if (fp == 0) {
        fclose(fp);
        fp = fopen("rangking.txt", "w");
        fprintf(fp, "%d", 100);
    } 
    fclose(fp);
}

void printMap() { // 현재 맵을 출력함
    cStorage = 0, err = 0;
    for (int i = 0; i < 30; i++) {
        if (cMap[i][0] == '\0') {
            break;
        }
        for (int j = 0; j < 30; j++) {
            if (cMap[i][j] == '\0') {
                break;
            } else if (map[cIndex][i][j] == 'O') {
                if (cMap[i][j] != '$' && cMap[i][j] != '@') {
                    cMap[i][j] = 'O';
                }
                if (cMap[i][j] == 'O') {
                    cStorage++;
                } else if (cMap[i][j] == '@') {
                    err++;
                }
            }
            if (cMap[i][j] == '.') { // '.'을 ' '으로 화면에 출력
                printf(" ");
            } else {
                printf("%c", cMap[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n이동횟수 : %d회\n", mvCnt);
    if (cStorage == 0 && err == 0) {
        nextStage = 1;
    }
}

int getch() { // getch함수 생성
    int ch;
    struct termios buf, save;
    tcgetattr(0,&save);
    buf = save;
    buf.c_lflag &= ~(ICANON|ECHO);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &buf);
    ch = getchar();
    tcsetattr(0, TCSAFLUSH, &save);
    return ch;
}

void recordUndo() {
    for (int i = 4; i >= 1; i--) { // 저장되 있던 되돌리기용 맵 배열을 한 칸씩 뒤로 밀어줌
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 30; k++) {
                uMap[i][j][k] = uMap[i - 1][j][k];
            }
        }
        uPl[i][0] = uPl[i - 1][0], uPl[i][1] = uPl[i - 1][1];; // 저장되 있던 되돌리기용 '@' 위치 배열을 한 칸씩 뒤로 밀어줌
    }
    for (int i = 0; i < 30; i++) { // 0번 자리에 새로운 맵 배열을 넣음
        for (int j = 0; j < 30; j++) {
            uMap[0][i][j] = cMap[i][j];
        }
    }
    uPl[0][0] = plX, uPl[0][1] = plY; // 0번 자리에 새로운 '@' 위치 배열을 넣음
}

void moveCommand(char ch) {
    recordUndo();
    int dX = 0, dY = 0;
    switch (ch) {
        case 'h': // 왼쪽으로 이동하므로 Y축이 -1
            dY = -1;
            break;
        case 'j': // 아래쪽으로 이동하므로 X축이 +1
            dX = 1;
            break;
        case 'k': // 위쪽으로 이동하므로 X축이 -1
            dX = -1;
            break;
        case 'l': // 오른쪽으로 이동하므로 Y축이 +1
            dY = 1;
            break;
    }
    int X = plX + dX, Y = plY + dY, aX = X + dX, aY = Y + dY;
    if (cMap[X][Y] == '.' || cMap[X][Y] == 'O') { // 이동하려는 칸이 '.'일 경우
        cMap[X][Y] = '@', cMap[plX][plY] = '.'; 
    } else if (cMap[X][Y] == '#') { // 이동하려는 칸이 '#'일 경우
        dX = 0, dY = 0; 
    } else if (cMap[X][Y] == '$') { // 이동하려는 칸이 '$'일 경우 '$'의 옆칸을 확인
        if (cMap[aX][aY] == '.' || cMap[aX][aY] == 'O') { // '$'의 옆칸이 '.'이나 'O'일 경우
            cMap[aX][aY] = '$', cMap[X][Y] = '@', cMap[plX][plY] = '.'; 
        } else if (cMap[aX][aY] == '#' || cMap[aX][aY] == '$') { // '$'의 옆칸이 '#'이나 '$'일 경우
            dX = 0, dY = 0; 
        }
    } 
    plX += dX, plY += dY, mvCnt++;
    system("clear");
    printMap();
    return;
}

void undo() { // 되돌리기
    for (int i = 0; i < 30; i++) { // 0번 자리에 있던 되돌리기용 맵 배열을 현재 맵 배열로 불러옴
        for (int j = 0; j < 30; j++) {
            cMap[i][j] = uMap[0][i][j];
        }
    }
    plX = uPl[0][0], plY = uPl[0][1]; // 0번 자리에 있던 '@' 위치 배열을 plX, plY 변수로 불러옴
    for (int i = 0; i <= 4; i++) { // 저장되 있던 되돌리기용 맵 배열을 한 칸씩 앞으로 당겨옴
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 30; k++) {
                uMap[i][j][k] = uMap[i + 1][j][k];
            }
        }
        uPl[i][0] = uPl[i + 1][0], uPl[i][1] = uPl[i + 1][1]; // 저장되 있던 '@' 위치 배열을 한 칸씩 앞으로 당겨옴
    }
    printMap();
}

void resetUndo() { // undo 3차원 배열을 초기화함
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 30; k++) {
                uMap[i][j][k] = '\0';
            }
        }
        uPl[i][0] = '\0', uPl[i][1] = '\0';
    }
}

void saveMap() { // 현재 맵 상태를 sokoban.txt 파일에 저장함
    FILE *fp;
    fp = fopen("sokoban.txt", "w");
    fprintf(fp, "%d\n", cIndex + 1); // 현재 맵 번호 저장
    fprintf(fp, "%d\n", mvCnt); // 이동 횟수 저장
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 30; j++) {
            fprintf(fp, "%c", cMap[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "e");
    fclose(fp);
}

void loadMap() { // sokoban.txt 파일에 저장한 현재 맵을 불러옴
    char buffer[30] = {0, };
    FILE *fp;
    fp = fopen("sokoban.txt", "r");
    if (fp == 0) {
        printf("저장된 맵이 없습니다.\n\n");
        printMap();
    } else {
        int l = 0;
        while (1) {
            fscanf(fp, "%s", &buffer); // map.txt 파일을 한 줄씩 읽어들임
            if (buffer[0] == 'e') { // 읽어들인 줄이 'e'일 경우
                break;
            } else if (buffer[0] >= 49 && buffer[0] <= 53) { // 읽어들인 줄이 1 ~ 5일 경우
                cIndex = buffer[0] - 49, l = 0;
                fscanf(fp, "%d", &mvCnt); // 이동 횟수 불러옴
            } else {
                for (int i = 0; i < 30; i++) { // 읽어들인 줄을 한 문자씩 맵 배열에 저장함
                    if (buffer[i] == '\0') {
                        break;
                    } else if (buffer[i] == '@') {
                        plX = l, plY = i;
                    }
                    cMap[l][i] = buffer[i];
                }
                l += 1;
            }
        }
        printMap();
        fclose(fp);
        resetUndo();
        printf("\n저장된 맵이 출력되었습니다.\n");
    }
}

void manual() { // 'd'를 눌렀을 때 명령 내용 출력을 위한 함수
    printf("\nh(왼쪽), j(아래), k(위), l(오른쪽)\n");
    printf("u(undo)\n");
    printf("r(replay)\n");
    printf("n(new)\n");
    printf("e(exit)\n");
    printf("s(save)\n");
    printf("f(file load)\n");
    printf("d(display help)\n");
    printf("t(top)\n");
}

void inputRangking() { // 스테이지를 클리어하면 해당 정보를 랭킹 파일에 입력함
    int infront = 0, limitFive = 0;
    FILE *ifp, *ofp;
    ifp = fopen("rangking.txt", "r");
    while (1) {
        fscanf(ifp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        if ((rIndex - 1 < cIndex) || ((rIndex - 1 == cIndex) && (rMvCnt < mvCnt))) {
            infront++;
        } else {
            break;
        }
    }
    fclose(ifp);
    ifp = fopen("rangking.txt", "r"), ofp = fopen("newRangking.txt", "w");
    for (int i = 0; i < infront; i++) {
        fscanf(ifp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        fprintf(ofp, "%d %s %d\n", rIndex, rName, rMvCnt);
        if (rIndex - 1 == cIndex) {
            limitFive++;
        }
    }
    fprintf(ofp, "%d %s %d\n", cIndex + 1, name, mvCnt);
    limitFive++;
    while (1) {
        fscanf(ifp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        if (rIndex == 100){
            fprintf(ofp, "%d", 100);
            break;
        } else if (rIndex - 1 == cIndex) {
            if (limitFive < 5) {
                fprintf(ofp, "%d %s %d\n", rIndex, rName, rMvCnt);
                limitFive++;
            }
        } 
        else {
            fprintf(ofp, "%d %s %d\n", rIndex, rName, rMvCnt);
        }
    }
    fclose(ifp), fclose(ofp);
    ifp = fopen("rangking.txt", "w"), ofp = fopen("newRangking.txt", "r");
    while (1) {
        fscanf(ofp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        if (rIndex == 100) {
            fprintf(ifp, "%d", 100);
            break;
        } else {
            fprintf(ifp, "%d %s %d\n", rIndex, rName, rMvCnt);
        }
    }
    fclose(ifp), fclose(ofp);
    remove("newRangking.txt");
}

void loadAllRangking() { // 전체 랭킹을 출력함
    int previousRIndex = 0, rank = 1;
    printf("\n전체 맵 랭킹을 출력합니다.\n");
    FILE *fp;
    fp = fopen("rangking.txt", "r");
    while (1) {
        previousRIndex = rIndex;
        fscanf(fp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        if (rIndex == 100) {
            break;
        } else {
            if (rIndex != previousRIndex) {
                printf("\n%d번 맵 랭킹\n\n", rIndex);
                rank = 1;
            }
            printf("%d위\t이름: %-10s\t이동횟수: %d\n", rank, rName, rMvCnt);
            rank++;
        }
    }
    fclose(fp);
}

void loadMapRangking (char a) { // 해당 번호의 맵 랭킹만 출력함
    int rank = 1;
    printf("%c번 맵 랭킹을 출력합니다.\n\n", a);
    FILE *fp;
    fp = fopen("rangking.txt", "r");
    while (1) {
        fscanf(fp, "%d %s %d", &rIndex, &rName, &rMvCnt);
        if (a - 48 == rIndex) {
            printf("%d위\t이름: %-10s\t이동횟수: %d\n", rank, rName, rMvCnt);
            rank++;
        } else if (a - 49 < rIndex) {
            break;
        }
    }
}

void command() { // 키를 입력받아 해당하는 함수 호출
    char ch;
    ch = getchar();
    while (1) {
        printf("\n");
        ch = getch();
        if (ch == 't') { // 전체 맵 랭킹을 출력할건지 해당 맵 랭킹을 출력할건지 입력받아야 하므로 다른 명령들과 별도로 처리함
            printf("t ");
            char a = getchar();
            system("clear");
            printMap();
            if (a == '\n') {
                loadAllRangking();
            } else {
                if ((a >= '1' && a <= '5') && (map[a - 49][0][0] != '\0')) {
                    printf("\n");
                    loadMapRangking(a);
                } else {
                    printf("\n해당 맵이 존재하지 않습니다.\n");
                }
                ch = getchar();
            }
            printf("\n");
        } else {
            switch (ch) {
                case 'u':
                    if (uMap[0][0][0] == '\0') {
                        printf("더 이상 되돌리기가 불가능합니다.\n");
                    } else {
                        system("clear");
                        mvCnt++;
                        undo();
                    }
                    break;
                case 'r': // 현재 레벨을 새로 시작
                    system("clear");
                    printf("현재 레벨을 다시 시작합니다.\n\n");
                    resetUndo();
                    createCurrentMap();
                    printMap();
                    break;
                case 'n': // 레벨 1부터 새로운 게임 시작
                    system("clear");
                    printf("게임을 다시 시작합니다.\n\n");
                    cIndex = 0, mvCnt = 0;
                    resetUndo();
                    createCurrentMap();
                    printMap();
                    break;
                case 'e': // 무한반복을 끝내고 탈출함
                    system("clear");
                    goto EXIT;
                case 's': // 현재 맵 상태를 저장함
                    saveMap();
                    printf("현재 맵 상태가 저장되었습니다.\n");
                    break;
                case 'f': // 저장된 맵을 불러옴
                    system("clear");
                    loadMap();
                    break;
                case 'd': // 매뉴얼 출력
                system("clear");
                printMap();
                    manual();
                    break;
                default:
                    if (ch == 'h' || ch == 'j' || ch == 'k' || ch == 'l') { // 이동키일경우 move_command() 함수 실행
                        moveCommand(ch);
                    } else { // 명령어 & 이동키가 아닐 경우 '잘못된 명령입니다.' 출력
                        printf("잘못된 명령입니다.\n");
                    }
                    break;
            }
            if (nextStage) {
                system("clear");
                printf("\n%d레벨 클리어!!!\n", cIndex + 1);
                inputRangking();
                mvCnt = 0, nextStage = 0, cIndex++;
                resetUndo();
                if (map[cIndex][0][0] == '\0') {
                    printf("모든 레벨을 클리어했습니다!!!\n프로그램을 종료합니다.\n");
                    break;
                } else {
                    createCurrentMap();
                }
                printf("\n");
                printMap();
            }
        }
    }
    EXIT :
    return;
}
