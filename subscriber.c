#include "header.h"

#define PATH_LOG "subscriber.log"
#define MAXLINE 10
#define SA struct sockaddr 

typedef struct subscriber_info{
	struct subscriber_info *next;
	char topic_name[50];
	int max_msgid_read;
} info;


FILE *fptr;
int sockfd;
info *node;

void foo();
void establish_conn(char **argv);
void subscribe_topic();
void retrieve_msg();
void get_all_message(master *mstr);
void get_next_message(master *mstr);
int is_subscribed(master *mstr);
int get_max_msgid(int sockfd, master t1);


int main(int argc,char *argv[]){

	if(argc !=3){
		printf("Usage : ./subcriber <BrokerIP> <BrokerPort>\n");
		exit(0);
	}

	if((fptr = fopen(PATH_LOG,"w")) == NULL)
		DieWithError("log file open");
	fprintf(fptr, "subcriber started. Connected to Broker with IP : %s Port %s\n",argv[1],argv[2]);

	establish_conn(argv);
	node = NULL;


	foo();

	return 0;
}


void foo(){

	while(1){
	
		int c;
		printf("->Enter 1 to subscribe to a topic\n->Enter 2 to retrieve a message\n->");

        scanf("%d",&c);


		switch(c){
			
			case 1: subscribe_topic();
						break;

			case 2: retrieve_msg();
						break;

			default: printf("%c is not a valid character\n",c);
						break;

		}


	}

}

void establish_conn(char **argv){

	struct sockaddr_in server;

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("socket()");
		exit(0);
	}
	fprintf(fptr, "socket() successful\n");

	bzero(&server,sizeof(server));
	server.sin_family= AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server.sin_addr);

	if(connect(sockfd, (SA *) &server, sizeof(server))==-1)
		DieWithError("connect to broker");

	return;
}

void subscribe_topic(){

	master mstr;

	printf("\n\t->Enter topic name : ");
	scanf("%s",mstr.topic_name);

    

	if(is_subscribed(&mstr) == 1){
		printf("\n\t->You are already subscribed to this topic!\n");
		return;
	}

    FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr.topic_name) + 1);
    strcpy(file_name, mstr.topic_name);

    printf("\n\n%s\n\n", file_name);
    if((fptr = fopen(file_name, "w")) == NULL){
        printf("\nerror while creating message file\n");
		
        return;
    }
    fprintf(fptr, "%d",0);
	printf("\n\t->You have successfully subscribed to this topic!\n");
    fclose(fptr);
	return;
}

void retrieve_msg(){

	master mstr;
	int ret;
	int c;

	bzero(&mstr,sizeof(mstr));
	
	printf("\n\t->Enter topic name : ");
	scanf("%s",mstr.topic_name);
   // fflush(stdin);
    
	ret = is_subscribed(&mstr);
	
    if(ret ==1){

		printf("\n\t->Enter '1' to retrieve the next message of the topic\n\t->Enter '2' to retrieve all messages of the topic\n->");
		scanf("%d",&c);

		switch(c){
			
			case 1: get_next_message(&mstr);
						break;

			case 2: get_all_message(&mstr);
						break;

			default: printf("'%c' is not a valid character\n",c);
						break;

		}
	}
	else
		printf("\n->You are not subscribed to this topic!\n");

	return;
}


void get_next_message(master *mstr){

	int read_count;
	
	if(mstr->msg_id >= get_max_msgid(sockfd,*mstr)){
		printf("You have read all messages of this topic\n");
		return;
	}
    mstr->msg_id +=1;
	mstr->type = GET_MSG;
    mstr->broker_id = -1;
    printf("\n\nIn get_next_message. About to call send_master. The struct is =\n");
    display(*mstr,1,1,1,1,1,1);
	send_master(sockfd, *mstr);
    
    master reply_struct;
	read_count = read(sockfd,&reply_struct,sizeof(reply_struct));
	if(read_count < sizeof(mstr))
		perror("read from socket");
    printf("\n\nReply in get_next_message. The struct is =\n");
	display(reply_struct,1,1,1,1,1,1);

    FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr->topic_name) + 1);
    strcpy(file_name, mstr->topic_name);

    if((fptr = fopen(file_name, "w")) == NULL){
        printf("\nerror. means topic not subscribed\n");
		
        return;
    }
    fprintf(fptr,"%d",(reply_struct.msg_id));
    fclose(fptr);

	return;
}

void get_all_message(master *mstr){

	int loop_count,read_count;


	loop_count = get_max_msgid(sockfd,*mstr);

	if(mstr->msg_id >= loop_count){
		printf("You have read all messages of this topic\n");
		return;
	}
	
	loop_count = loop_count - mstr->msg_id;
	mstr->type = 42;
    mstr->broker_id = -1;
	while(loop_count--){
		send_master(sockfd, *mstr);

		read_count = read(sockfd,&mstr,sizeof(mstr));
		if(read_count < sizeof(mstr))
			perror("read from socket");

		display(*mstr,0,1,1,1,0,0);
	}

    FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr->topic_name) + 1);
    strcpy(file_name, mstr->topic_name);

    if((fptr = fopen(file_name, "w")) == NULL){
        printf("\nerror opening file in get_all_messages\n");
		
        return;
    }
    fprintf(fptr,"%d",(mstr->msg_id));
    fclose(fptr);

	return;
}


int is_subscribed(master *mstr){

	FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr->topic_name) + 1);
    strcpy(file_name, mstr->topic_name);

    if((fptr = fopen(file_name, "r")) == NULL){
        printf("\nerror. means topic not subscribed\n");
		
        return 0;
    }
    fscanf(fptr,"%d",&(mstr->msg_id));
    fclose(fptr);
	return 1;
}

int get_max_msgid(int sockfd, master t1){

    t1.type = GET_MAX_MSGID;
    t1.broker_id = -1;
    printf("\nSending struct with type GET_MAX_MSGID. The struct is = \n");
    display(t1,1,1,1,1,1,1);
    if (write(sockfd, &t1 , sizeof(t1)) == -1)
        DieWithError("send() sent a different number of bytes than expected");
    
    master reply_struct;
    
    if(read(sockfd, &reply_struct, sizeof(reply_struct)) < 0)
        DieWithError("could not read while get_max_msgid");
    printf("\n\nReply of max_msgid. The struct is =\n");
    display(reply_struct,1,1,1,1,1,1);
    return reply_struct.msg_id;
}

