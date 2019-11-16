#include "header.h"

#define CLIENT_ID 1
#define PATH_LOG "publisher.log"

int send_master(int sock, master t1);

int get_max_msgid(int sockfd, master t1);


FILE *fptr;

int main(int argc, char *argv[]){


    if(strlen(argv[1]) == 0){
        printf("\nplease enter broker ip addr as command line arg\n");
        return 0;
    }

    if((fptr = fopen(PATH_LOG,"w")) == NULL)
		DieWithError("log file fopen()");

    

    int sock;
    struct sockaddr_in echoServAddr;
   
    char *servIP;
    if(argc < 2){
        printf("\nplease enter ip in cla");
        exit(0);
    }
    servIP = argv[1];   
    

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *) &echoServAddr,sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
    else
        printf("\nconnected to the broker");


    char op_type;
    
    while(1){

        printf("please select the operation that you want to perform\n");
        printf("press: 1 - create topic, 2 - send message , 3 - send file\n");
        scanf("%c",&op_type);

        if(op_type == '1'){
            master t1;
           
            printf("\nplease enter topic name that you want to create\n");
            t1.client_id = CLIENT_ID;
            t1.type = GET_MAX_MSGID;
            scanf("%s", t1.topic_name);
            int reply = get_max_msgid(sock, t1);

            if(reply == 0){
                t1.type = CREATE_MSG;
                printf("\nplease enter the first message that you want to post\n");
                scanf("%s", t1.message);
                t1.msg_id = 1;
                send_master(sock, t1);
            }
            else{
                t1.type = CREATE_MSG;
                printf("\ntopic already exists, enter the message that you want to post\n");
                scanf("%s", t1.message);
                t1.msg_id = reply + 1;
                send_master(sock, t1);
            }         
            //send_master(sock, t1);
          
    }

        if(op_type == '2'){
            master t1;
            t1.type = GET_MAX_MSGID;
            t1.client_id = CLIENT_ID;

            printf("\nplease enter the topic you want to post the message to\n");
            scanf("%s",t1.topic_name);
            printf("\nplease enter the message that you want to send");
            scanf("%s",t1.message);
            int reply = get_max_msgid(sock, t1);
            if(reply == 0){
                printf("\ntopic does not exists, topic will be created and message wll be posted\n");
                t1.type = CREATE_MSG;
                t1.msg_id = 1;
                send_master(sock, t1);
            }
            else{
                t1.type = CREATE_MSG;
                t1.msg_id = reply + 1;
                send_master(sock, t1);
            }
            //send_master(sock, t1);
        
        
    }

        if(op_type == '3'){
            master t1;
            t1.type = 3;

            printf("\nplease enter the topic you want to post the message to\n");
            scanf("%s",t1.topic_name);
        
            char file_name[100], c;
            printf("\nplease enter the file name you want to send\n");
            scanf("%s",file_name);

            FILE *fptr;
            fptr = fopen(file_name, "r"); 
            if (fptr == NULL) { 
                printf("Cannot open file \n"); 
                exit(0); 
            } 
            int count = 0;
            c=fgetc(fptr);
            while(c != EOF){
                
            }


        }
    }


    printf("\n");
    close(sock);
    exit(0);

    //fprintf(fptr,"socket created successfully\n");
    
}







int get_max_msgid(int sockfd, master t1){

    if (write(sockfd, &t1 , sizeof(t1)) == -1)
        DieWithError("send() sent a different number of bytes than expected");

    master reply_struct;
    
    if(read(sockfd, &reply_struct, sizeof(reply_struct)) < 0)
        DieWithError("could not read while get_max_msgid");

    return reply_struct.msg_id;
}