#ifndef LFS_H_
#define LFS_H_

#include<inttypes.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<pthread.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define SELECT "SELECT"
#define INSERT "INSERT"
#define CREATE "CREATE"
#define DESCRIBE "DESCRIBE"
#define DROP "DROP"
#define EXIT "EXIT"

typedef struct{
	int timeStamp;
	uint16_t key;
	int value;
}Registro;



#endif /* LFS_H_ */
