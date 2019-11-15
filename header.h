# include<stdio.h>
# include<signal.h>
# include<unistd.h>
# include<stdlib.h>
# include<string.h>
# include<sys/socket.h>
# include<arpa/inet.h>

#define PATH_LOG "publisher.log"
#define PORT 1900 
#define RCVBUFSIZE 32
#define MAX_TOPIC_SIZE 32
#define MAX_MESSAGE_SIZE 512
#define TOP_CRE_SUC 100
#define MESSAGE_SENT 101
#define SEND_FILE_SUCCESS 102
#define TOP_EXISTS 200
#define TOP_NO_EXISTS 201

#define GET_MAX_MSGID 40
#define RPL_MAX_MSGID 41
#define CREATE_MSG 42
#define GET_MSG 50
#define MSG_RETRIEVED 51

typedef struct my_struct{
    int type;
    char message[512];
    char topic_name[50];
    int msg_id;
    int client_id;
    int broker_id;
}master;

void DieWithError(char *message){
    perror(message);
    exit(1);
}












