/**********************************8
 * VR456714
 * Niccolo' Iselle
 * 2023-11-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/types.h>


#include "errExit.h"
#include "message.h"
#include "shmbrd.h"
#include "semaphore.h"

/* Variabili globali */
int count_sig = 0; //contatore per segnale CTRL-C
pid_t pid_Giocatore1;
pid_t pid_Giocatore2;
pid_t pid_Server;

/* Setup code dei messaggi fra server e giocatori */
int msqSrv = -1;
int msqCli = -1;

int checkValidity(struct shared_board *, int, int);
void printBoard(struct shared_board *, int, int);

void sigHandler(int sig) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }
    if(count_sig == 0){
        printf("Hai deciso di premere CTRL+C, se lo premi un altra volta faccio terminare il gioco! \n");
        count_sig++;
    }
    else if (count_sig == 1) {
        printf("Hai deciso di premere due volte CTRL+C, ora faccio terminare il gioco! \n");
        if (msqSrv > 0) {
            if(msgctl(msqSrv, IPC_RMID, NULL) == -1) {
                errExit("msgctl failed");
            } else {
                printf("<Server> Coda di messaggi del server eliminata con successo.\n");
            }
        }

        if (msqCli > 0) {
            if (msgctl(msqCli, IPC_RMID, NULL) == -1) {
                errExit("msgctl failed");
            } else {
                printf("<Server> Coda di messaggi del client eliminata con successo.\n");
            }
        }
        exit(0);

    }


}


int main(int argc, char * argv[]) {

    /* Handler CTRL-C */
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }


    /* VARIABILI LOCALI CLIENT */
    /* Chiavi per le code dei messaggi */
    key_t serverKey = 100; // Coda di invio messaggi al Server
    key_t clientKey = 101; // Coda di ricezione dei messaggi dal Server

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco
    key_t pidKey = 6010; // Chiave per l'accesso al pid dei giocatori.

    /* Chiave di accesso del semaforo */
    key_t semKey = 6060;

    // Accesso al semaforo
    int semid = semget(semKey, 3, S_IRUSR | S_IWUSR);
    if (semid == -1) {
        errExit("semget failed");
    }

    /* Accesso alla memoria condivisa per il pid dei giocatori */
    size_t pidSize = sizeof(struct shared_pid);
    int shPidID = shmget(pidKey, pidSize, S_IRUSR);
    if (shPidID == -1) {
        errExit("shmget failed");
    }

    struct shared_pid * ptr_playersPid = shmat(shPidID, 0, 0);

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }

    // Creiamo la coda di ricezione
    msqCli = msgget(clientKey,  S_IRUSR | S_IWUSR);
    if (msqCli == -1) {
        errExit("msgget failed");
    }

    unsigned long len = strlen(argv[1]);
    /* Invia al server il nome del giocatore che si e' connsesso */

    // Creiamo la coda di invio
    msqSrv = msgget(serverKey, S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        errExit("msgget failed");
    }
    // inizializzo il messaggio
    struct message msg;
    msg.mtype = 1;

    for (int i = 0; i < len; i++) {
        msg.content[i] = argv[1][i];
    }
    msg.content[len] = '\0';
    msg.pid = getpid();

    size_t mSize = sizeof(struct message) - sizeof(long);

    if (msgsnd(msqSrv, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    printf("<F4Client> In attesa di conferma della connessione al Server...\n");

    /* Attende la risposta dal server per la conferma della connessione e per
     * conoscere il simbolo di gioco. */
    if (getpid() == ptr_playersPid->player1) {
        semOp(semid, (unsigned short) 1, -1);
        printf("Gettone: %c \n", ptr_playersPid->player1Token);

    }
    if (getpid() == ptr_playersPid->player2) {
        semOp(semid, (unsigned short) 2, -1);
        printf("Gettone: %c \n", ptr_playersPid->player2Token);
    }
//    if (msgrcv(msqCli, &msg, mSize, 0, 0) == -1) {
//        errExit("msgrcv failed");
//    }
//
//    printf("%s\n", msg.content);

    /* Accesso alla memoria condivisa per il campo di gioco, la dimensione viene comunicata
     * dal server. */
    size_t boardSize = sizeof(struct shared_board);
    int shBoardID = shmget(boardKey, boardSize, S_IRUSR | S_IWUSR);
    if (shBoardID == -1) {
        errExit("shmget failed");
    }

    struct shared_board * ptr_gb = shmat(shBoardID, 0, 0);

    int row = ptr_gb->rows;
    int col = ptr_gb->cols;
    //printf("<F4Client> rows: %d; cols: %d.\n", row, col);                 // debug
    printBoard(ptr_gb, row, col);                                         // debug



    /* Attesa su semaforo della concessione del turno */
    printf("<F4Client> In attesa della connessione del giocatore 2.\n");

    /* Se il processo corrente è il giocatore 2, libero il server dall'attesa
     * e attendo il turno, altrimenti il processo è il giocatore 1, che si mette
     * in attesa del giocatore 2 */
    if (getpid() == ptr_playersPid->player2) {
        semOp(semid, (unsigned short) 0, 1);        // libero il server
        semOp(semid, (unsigned short)2, -1);        // player2 in attesa
    } else {
        semOp(semid, (unsigned short) 1, -1);       // player1 in attesa del giocatore 2
        printf("<F4Client: giocatore 2 connesso!\nInizia la partita!\n");
    }



    /* Richiesta delle coordinate su cui inserire il token
     * e verifica della validita' */
    char token;
    int r, c, flag = 1;
    if (getpid() == ptr_playersPid->player1) {
        token = ptr_playersPid->player1Token;
    }
    if (getpid() == ptr_playersPid->player2) {
        token = ptr_playersPid->player2Token;
    }

    do {
        // Richiesta della casella di gioco
        printf("Inserire riga: ");
        scanf("%d", &r);
        printf("Inserire colonna: ");
        scanf("%d", &c);

        if (checkValidity(ptr_gb, r, c) == 1) {
            // aggiorno il valore nel tabellone
            ptr_gb->board[r][c] = token;
            flag = 0;
        } else {
            printf("\n<F4Client> Errore! La coordinata inserita non è valida.\n");
        }

    } while(flag);

    printBoard(ptr_gb, row, col);

    count_sig = 0;
    if (getpid() == ptr_playersPid->player1) {
        semOp(semid, 0, 1);         // passa il controllo al server
        semOp(semid, 1, -1);        // p1 attende il turno
    }
    if (getpid() == ptr_playersPid->player2){
        semOp(semid, 0, 1);         // passa il controllo al server
        semOp(semid, 2, -1);        // p2 attende il turno
    }

    /* Chiusura della shared_board memory */
    if (shmdt(ptr_gb) == -1) {
        errExit("shmdt failed");
    }
    return 0;
}
/* Il Client e' responsabile della richiesta al giocatore su quale casella vuole
 * giocare il suo token e di verificarne la legittimita'.
 */
int checkValidity(struct shared_board *ptr_sh, int row, int col) {
    if (ptr_sh->board[row][col] != 'o' && ptr_sh->board[row][col] != 'x')
        return 1;
    else
        return 0;
}

/* Il Client e' responsabile della stampa del campo di gioco
 * @param gB matrice in memoria condivisa che rappresenta il campo
 * @param row numero di righe della matrice
 * @param col numero di colonne della matrice
 */
void printBoard(struct shared_board  *ptr_sh, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (j == 0) {
                printf("| %c |", ptr_sh->board[i][j]);
            } else {
                printf(" %c |", ptr_sh->board[i][j]);
            }
        }
        printf("\n");
    }
}