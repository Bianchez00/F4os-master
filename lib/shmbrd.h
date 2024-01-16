#include <stdlib.h>

struct shared_board {
    char board[100][100];
    int rows;
    int cols;
};

struct shared_pid {
    char player1Name[100];
    pid_t player1;
    char player1Token;
    char player2Name[100];
    pid_t player2;
    char player2Token;
};
struct winning {
    int flag1;  // == pid giocatore -> perde per abbandono
    int flag2;  // giocatore 1 ha vinto =1
    int flag3;  // giocatore 2 ha vinto =1
};