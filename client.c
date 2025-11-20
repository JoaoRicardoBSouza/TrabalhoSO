// SE SUA IDE NÃO COMPILAR NO BOTÃO DE COMPILAR RODE 'gcc client.c -o client.exe -lws2_32' NO TERMINAL
// EXECUTE O CLIENT.EXE DEPOIS DO SERVER.EXE

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

#define GRID 5
#define BUF 64

#define RED   "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE  "\033[1;34m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

char myGrid[GRID][GRID];
char oppView[GRID][GRID];
int shipsRemaining = 2;

void clearGrids() {
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            myGrid[r][c] = oppView[r][c] = '~';
}

int coordFromStr(const char *s, int *r, int *c) {
    if (!s || strlen(s) < 2) return 0;
    char row = toupper(s[0]);
    char col = s[1];
    if (row < 'A' || row >= 'A' + GRID || col < '1' || col >= '1' + GRID) return 0;
    *r = row - 'A';
    *c = col - '1';
    return 1;
}

void printGrid(char grid[GRID][GRID], const char *title) {
    printf("%s\n  1 2 3 4 5\n", title);
    for (int r = 0; r < GRID; r++) {
        printf("%c ", 'A' + r);
        for (int c = 0; c < GRID; c++) {
            char ch = grid[r][c];
            if (ch == 'H') printf(RED "%c " RESET, ch);
            else if (ch == 'M') printf(BLUE "%c " RESET, ch);
            else printf("%c ", ch);
        }
        printf("\n");
    }
}

void showInterface() {
    system("cls");
    printf(YELLOW "===== ⚓ BATALHA NAVAL ⚓ =====\n\n" RESET);
    printGrid(myGrid, "💙 Seu Tabuleiro:");
    printf("\n");
    printGrid(oppView, "💥 O que voce sabe do inimigo:");
    printf("\n");
}

void placeShipAuto(int size) {
    int placed = 0;
    while (!placed) {
        int r = rand() % GRID;
        int c = rand() % GRID;
        int horizontal = rand() % 2;
        int ok = 1;
        if (horizontal) {
            if (c + size > GRID) continue;
            for (int i = 0; i < size; i++)
                if (myGrid[r][c + i] != '~') ok = 0;
            if (!ok) continue;
            for (int i = 0; i < size; i++) myGrid[r][c + i] = 'S';
        } else {
            if (r + size > GRID) continue;
            for (int i = 0; i < size; i++)
                if (myGrid[r + i][c] != '~') ok = 0;
            if (!ok) continue;
            for (int i = 0; i < size; i++) myGrid[r + i][c] = 'S';
        }
        placed = 1;
    }
}

int checkSunkAt(int r, int c) {
    int startc = c, endc = c;
    while (startc > 0 && myGrid[r][startc - 1] != '~') startc--;
    while (endc < GRID - 1 && myGrid[r][endc + 1] != '~') endc++;
    for (int cc = startc; cc <= endc; cc++)
        if (myGrid[r][cc] == 'S') return 0;

    int startr = r, endr = r;
    while (startr > 0 && myGrid[startr - 1][c] != '~') startr--;
    while (endr < GRID - 1 && myGrid[endr + 1][c] != '~') endr++;
    for (int rr = startr; rr <= endr; rr++)
        if (myGrid[rr][c] == 'S') return 0;

    return 1;
}

void showVictoryScreen() {
    system("cls");
    printf(GREEN "\n\n🎉🎉🎉 PARABENS! 🎉🎉🎉\n" RESET);
    printf(GREEN "Voce afundou todos os navios inimigos!\n\n" RESET);
    printGrid(oppView, "Situacao final do inimigo:");
    printf("\n");
    system("pause");
}

void showDefeatScreen() {
    system("cls");
    printf(RED "\n\n💀💀💀 DERROTA 💀💀💀\n" RESET);
    printf(RED "Todos os seus navios foram afundados!\n\n" RESET);
    printGrid(myGrid, "Seu tabuleiro destruido:");
    printf("\n");
    system("pause");
}

int main() {
    srand(time(NULL));
    clearGrids();
    printf("=== BATALHA NAVAL (Cliente) ===\n");
    placeShipAuto(3);
    placeShipAuto(2);

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(51171);

    printf("Conectando ao servidor...\n");
    if (connect(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro ao conectar.\n");
        return 1;
    }

    printf(GREEN "Conectado com sucesso!\n" RESET);
    Sleep(1000);

    char buf[BUF];
    int myTurn = 0;

    while (1) {
        if (myTurn) {
            showInterface();
            printf("Sua vez! Digite coordenada (ex: A1): ");
            fgets(buf, sizeof(buf), stdin);
            buf[strcspn(buf, "\r\n")] = 0;

            int r, c;
            if (!coordFromStr(buf, &r, &c) || oppView[r][c] != '~') {
                printf("Coordenada invalida.\n");
                Sleep(1000);
                continue;
            }

            send(s, buf, strlen(buf), 0);
            int rec = recv(s, buf, sizeof(buf) - 1, 0);
            if (rec <= 0) break;
            buf[rec] = 0;

            if (strcmp(buf, "HIT") == 0) {
                oppView[r][c] = 'H';
                printf(RED "\n💥 Voce acertou um navio inimigo!\n" RESET);
            } else if (strcmp(buf, "MISS") == 0) {
                oppView[r][c] = 'M';
                printf(BLUE "\n💦 Voce errou o tiro.\n" RESET);
            } else if (strcmp(buf, "SUNK") == 0) {
                oppView[r][c] = 'H';
                printf(RED "\n🚢 Voce afundou um navio inimigo!\n" RESET);
            } else if (strcmp(buf, "SUNK ALL") == 0) {
                oppView[r][c] = 'H';
                showVictoryScreen();
                break;
            }

            Sleep(1500);
            myTurn = 0;
        } else {
            int rec = recv(s, buf, sizeof(buf) - 1, 0);
            if (rec <= 0) break;
            buf[rec] = 0;
            int r, c;
            if (!coordFromStr(buf, &r, &c)) {
                send(s, "INVALID", 7, 0);
                continue;
            }

            if (myGrid[r][c] == 'S') {
                myGrid[r][c] = 'H';
                if (checkSunkAt(r, c)) {
                    shipsRemaining--;
                    if (shipsRemaining <= 0) {
                        send(s, "SUNK ALL", 8, 0);
                        showDefeatScreen();
                        break;
                    } else
                        send(s, "SUNK", 4, 0);
                } else
                    send(s, "HIT", 3, 0);
            } else {
                myGrid[r][c] = 'M';
                send(s, "MISS", 4, 0);
            }

            myTurn = 1;
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
