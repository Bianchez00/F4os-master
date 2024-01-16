#ifndef F4OS_SEMAPHORE_H
#define F4OS_SEMAPHORE_H

/* Definizione della union semun */
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);

#endif //F4OS_SEMAPHORE_H
