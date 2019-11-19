#include "header.h"

#define PATH_LOG "broker.log"
#define SA struct sockaddr 
#define backlog 10


typedef struct message_struct{
    char message[MAX_MESSAGE_SIZE];
    int msg_id;
    struct message_struct *next;
} message_item;


typedef struct topic_struct{
    char topic_name[MAX_TOPIC_SIZE];
    struct topic_struct *next;
    message_item *first_msg;

} topic_item;

int myBrokerID;
int to_neighbour_sockfd, from_neighbour_sockfd;
FILE *fptr;
int my_port;
int listenfd;
topic_item *head;


void connect_to_neighbour(char *ip1,int port1);
void let_neighbour_connect();
void foo();
void handle_data(master mstr);
int get_message_file(master *mstr);
void get_max_msgid_broker_file(master *mstr);
void create_message_file(master mstr);




int main(int argc, char *argv[]){


    
    printf("Please enter my broker ID : ");
    scanf("%d",&myBrokerID);
    printf("Please enter my Port No. : ");
    scanf("%d",&my_port);
    

	if((fptr = fopen(PATH_LOG,"w")) == NULL)
	    DieWithError("log file open");
	fprintf(fptr, "Broker %d started. Connected to Broker with IP : %s Port %s\n",myBrokerID,argv[1],argv[2]);

	head=NULL;
    char ip1[50];
    int port1;
    
    let_neighbour_connect();

    printf("Enter IP of neighbour I should connect to: ");
    scanf("%s",ip1);
    printf("Enter port no. of neighbour I should connect to: ");
    scanf("%d",&port1);

    connect_to_neighbour(ip1,port1);
    foo();

}

void connect_to_neighbour(char *ip1,int port1){

	struct sockaddr_in server;

	if( (to_neighbour_sockfd = socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("socket()");
		exit(0);
	}
	fprintf(fptr, "Broker %d : socket() successful\n",myBrokerID);

	bzero(&server,sizeof(server));
	server.sin_family= AF_INET;
	server.sin_port=htons(port1);
	inet_pton(AF_INET, ip1, &server.sin_addr);

	if(connect(to_neighbour_sockfd, (SA *) &server, sizeof(server))==-1)
		DieWithError("connect to broker");
    
    fprintf(fptr,"Broker %d : connect() to neighbouring broker successfull!\n",myBrokerID);
	
    return;
}

void let_neighbour_connect(){

    int connfd;
	struct sockaddr_in servaddr,cliaddr;
	int pid;
	socklen_t clilen;
	char buf[INET_ADDRSTRLEN];

	if((listenfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		perror("socket server: ");
		exit(0);
	}

	bzero(&cliaddr,sizeof(cliaddr));
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family= AF_INET;
	servaddr.sin_port= htons(my_port);
	servaddr.sin_addr.s_addr= htonl(INADDR_ANY);

	if(bind(listenfd, (SA *) &servaddr, sizeof(servaddr))==-1){
		perror("server bind: ");
		exit(0);
	}

	listen(listenfd, backlog);



    printf("Calling accept() in let_neighbour_connect\n");
    clilen=sizeof(cliaddr);

    if( (from_neighbour_sockfd = accept(listenfd, (SA *) &cliaddr, &clilen) )==-1)
        DieWithError("connect() in let_neighbour_connect");    
    if(inet_ntop(AF_INET, &cliaddr.sin_addr, buf,INET_ADDRSTRLEN)==NULL)
				DieWithError("inet_ntop in let_neighbour connect");
	
    printf("Neighbour successfully connected to me! Its IP: %s  Port: %d\n",buf,ntohs(cliaddr.sin_port));
	
    return;

}

void foo(){

	fd_set superset,rset; 
	int maxfd;
	int bytes;

	FD_ZERO(&superset);
	FD_ZERO(&rset);
	FD_SET(listenfd,&superset);
    FD_SET(from_neighbour_sockfd,&superset);
	maxfd = listenfd;
    maxfd = from_neighbour_sockfd > maxfd ? from_neighbour_sockfd : maxfd;



	for(;;){



		rset = superset;
        printf("Calling select()\n");
		if(select(maxfd+1, &rset, NULL, NULL, NULL) == -1)
			DieWithError("select() failed");



		for(int i = 0; i <= maxfd; i++){


    		if(FD_ISSET(i, &rset)){
				
				int connfd;
				int clilen;
				struct sockaddr_in cliaddr;

    			if(i == listenfd){								/* handle new connections */
					
					clilen = sizeof(cliaddr);
					if((connfd = accept(listenfd, (SA *)&cliaddr, &clilen)) == -1)
    					DieWithError("accept()");

    				FD_SET(connfd,&superset);
    				maxfd = connfd > maxfd ? connfd : maxfd;
    				printf("New connection from %s on socket %d\n", inet_ntoa(cliaddr.sin_addr), connfd);
				}

				else{											/* handle data from a client */

					master mstr;
					if((bytes = read(i, &mstr, sizeof(mstr))) < sizeof(mstr)){
	
						if(bytes == 0)
							printf("connection on socket %d closed\n", i);
						else
							perror("read in foo() error");
						close(i);
						FD_CLR(i,&superset);						//remove it from master set
					}


					else{										
                        if(i != from_neighbour_sockfd)
						    mstr.client_id = i;
                        printf("\n\nIn foo, after select() returned. About to Call handle_data\n. The struct I received is = \n\n");
                        display(mstr,1,1,1,1,1,1);
						handle_data(mstr);

					}


				}

			}//if fd_isset ends
		}
	}//infinite for loop ends


}


void handle_data(master mstr){
	int pid;
	switch (mstr.type){

		case GET_MAX_MSGID:
			
			if(mstr.broker_id == myBrokerID){
				

					mstr.type = RPL_MAX_MSGID;
					printf("\n\nIn case GET_MAX_MSGID. mstr.broker_id == myBrokerID. About to send data to client. The struct is =\n");
					display(mstr,1,1,1,1,1,1);
					send_master(mstr.client_id, mstr);

					return;
			}
			else{
					if(mstr.broker_id == -1)
						mstr.broker_id = myBrokerID;
					
					get_max_msgid_broker_file(&mstr);
					printf("Sending to my neighbour\n");
					send_master(to_neighbour_sockfd, mstr);
					printf("GET_MAX_MSGID sent to my neighbour\n");

					return;

			}
			break;



		case CREATE_MSG:
			printf("\n\nIn case CREATE_MSG. The struct is =\n");
			display(mstr,1,1,1,1,1,1);
			create_message_file(mstr);
			break;


		case GET_MSG: 
			printf("\n\nIn case GET_MSG. The struct is =\n");
			display(mstr,1,1,1,1,1,1);

			
				if(mstr.broker_id == myBrokerID){	
						printf("\n\nIn case get_msg, mstr.broker_id == myBrokerID.About to send data to client .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
						send_master(mstr.client_id, mstr);
						fprintf(fptr,"GET_MSG sent to client %d\n",mstr.client_id);
						return;
				}
				else if(mstr.broker_id == -1){
					
					mstr.broker_id = myBrokerID;
					int reply = get_message_file(&mstr);
					if(reply == 1){							//message found
						mstr.type = MSG_RETRIEVED;
						printf("\n\nIn case get_msg, mstr.broker_id == -1 and MSG_RETRIEVED.About to send data to client .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
						send_master(mstr.client_id, mstr);
						fprintf(fptr,"get_msg MSG_RETRIEVED sent to client %d\n",mstr.client_id);
						return;
					}
					else{
						printf("\n\nIn case get_msg, mstr.broker_id == -1 and NOT OF MSG_RETRIEVED.About to send data to neighbour .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"GET_MSG sent to my neighbour broker\n");
						return;
					}
				}
				else{
					
					int reply = get_message_file(&mstr);
					if(reply == 1){							//message found
						mstr.type = MSG_RETRIEVED;
						printf("\n\nIn case get_msg, mstr.broker_id IS NOT MY BROKER ID AND ALSO NOT -1 and MSG_RETRIEVED.About to send data to client .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"MSG_RETRIEVED sent to my neighbour broker\n");
					}
					else{
						printf("\n\nIn case get_msg, mstr.broker_id IS NOT MY BROKER ID AND ALSO NOT -1 and NOT OF MSG_RETRIEVED.About to send data to client .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"GET_MSG sent to my neighbour broker\n");
					}
				} 
				return;

			break;



		case MSG_RETRIEVED: 
			display(mstr,1,1,1,1,1,1);

				if(mstr.broker_id == myBrokerID){
						printf("\n\nIn case MSG_RETRIEVED, mstr.broker_id == myBrokerID .About to send data to client .The struct is =\n");
						display(mstr,1,1,1,1,1,1);
					send_master(mstr.client_id, mstr);
					fprintf(fptr,"MSG_RETRIEVED sent to client %d\n",mstr.client_id);
					return;

				}
				else{
					printf("\n\nIn case MSG_RETRIEVED, mstr.broker_id is not my brokerID .About to send data to client .The struct is =\n");
					display(mstr,1,1,1,1,1,1);
					send_master(to_neighbour_sockfd, mstr);
					fprintf(fptr,"MSG_RETRIEVED sent to my neighbour broker\n");
					return;

				}

				return;
			break;



		default:
			perror("flow reached default case in switch");

	}	//endswitch
}



// int get_message_file(master *mstr){


// 	if(head == NULL){
// 		//mstr->msg_id = 0;
// 		return 0;
// 	}

// 	topic_item *t1 ;//= (topic_item *)malloc(sizeof(topic_item));
// 	message_item *m1;// = (message_item *) malloc(sizeof(message_item));

// 	t1 = head;
// 	while(1){
// 		if(strcmp(t1->topic_name, mstr->topic_name) == 0){
// 			m1 = t1->first_msg;

// 			while(m1 != NULL){
				
// 				if(mstr->msg_id == m1->msg_id){
// 					strcpy(mstr->message, m1->message);
// 					//mstr->type = MSG_RETRIEVED;
// 					return 1;
// 				}
// 				m1 = m1->next;
// 			}


// 		}
// 		t1 = t1->next;
// 		if(t1 == NULL){
// 			return 0;
			
// 		}
// 	}
// }


// void get_max_msgid_broker_file(master *mstr){

// 	if(head == NULL){
// 		//mstr->msg_id = 0;
// 		return;
// 	}

// 	topic_item *t1 ;//= (topic_item *)malloc(sizeof(topic_item));
// 	message_item *m1 ;//= (message_item *) malloc(sizeof(message_item));

// 	t1 = head;
// 	while(1){

// 		if(strcmp(t1->topic_name, mstr->topic_name) == 0){
// 			printf("In get_max_msgid t1->topic_name, mstr->topic_name\n");
//             m1 = t1->first_msg;

// 			while(m1 != NULL){
				
// 				if(m1->next == NULL && (mstr->msg_id < m1->msg_id)){
// 					printf("In get_max_msgid_broker. The original Msgid = %d. The updated is  = %d.\n",mstr->msg_id,m1->msg_id);
//                     mstr->msg_id = m1->msg_id;
// 					return;
// 				}
// 				m1 = m1->next;
// 			}

//             return;
// 		}
// 		t1 = t1->next;
// 		if(t1 == NULL){
//             printf("t1 == NULL wala case\n");
// 			mstr->msg_id = 0;
// 			break;
// 		}
// 	}
	

// }

// void create_message_file(master mstr){
// 	topic_item *t1 = (topic_item *) malloc(sizeof(topic_item));
// 	message_item *m1 = (message_item *) malloc(sizeof(message_item));
	
// 	if(head == NULL){
		
// 		t1->first_msg = m1;
// 		t1->next = NULL;
// 		strcpy(t1->topic_name, mstr.topic_name);
// 		m1->msg_id = 1;
// 		m1->next = NULL;
// 		strcpy(m1->message, mstr.message);
// 		head = t1;
// 		printf("\nIn create message. Head == NULL. Topic name = %s \n",t1->topic_name);	
// 		return;
// 	}
// 	else{
// 		t1 = head;
		
			
// 		while(1){
			
// 			if(strcmp(t1->topic_name, mstr.topic_name) == 0){
				
// 				m1=t1->first_msg;

// 				while(m1->next !=NULL){
//                     printf("\nIn create message. m1->topic_name = %s, m1->msg_id = %d, m1->message = %s\n\n",t1->topic_name, m1->msg_id, m1->message);
//                 	m1 = m1->next;
// 				}
//                 printf("\nIn create message. m1->topic_name = %s, m1->msg_id = %d, m1->message = %s\n\n",t1->topic_name, m1->msg_id, m1->message);
// 				message_item temp;
// 				temp.msg_id = mstr.msg_id;
// 				temp.next = NULL;
//                 printf("\nbefore strcpy\n");
// 				strcpy(temp.message, mstr.message);
//                 printf("\nafter strcpy\n");
// 				m1->next = &temp;
				
// 				return;
// 			}

			
// 			if(t1->next==NULL){
// 				break;
// 			}
//             t1 = t1->next;
// 		}

//         topic_item t2;
// 		message_item m2;

//         t2.next = NULL;
//         strcpy(t2.topic_name, mstr.topic_name);
//         t2.first_msg = &m2;
//         m2.msg_id = mstr.msg_id;
//         m2.next = NULL;
//         strcpy(m2.message, mstr.message);

//         t1->next = &t2;
//         return;
// 		/*if(strcmp(t1->topic_name, mstr.topic_name)==0){
// 			topic_item t2;
// 			message_item m2;

// 			t2.next = NULL;
// 			strcpy(t2.topic_name, mstr.topic_name);
// 			t2.first_msg = &m2;
// 			m2.msg_id = mstr.msg_id;
// 			m2.next = NULL;
// 			strcpy(m2.message, mstr.message);

// 			t1->next = &t2;
// 		}
// 		else{
// 			m1=t1->first_msg;
// 				while(m1->next !=NULL){
// 					m1 = m1->next;
// 				}
// 				message_item temp;
// 				temp.msg_id = mstr.msg_id;
// 				temp.next = NULL;
// 				strcpy(temp.message, mstr.message);
// 				m1->next = &temp;
// 				return;
// 		}*/

// 	}
	
// }



int get_message_file(master *mstr){

    FILE *fptr;

    char *file_name = (char *)malloc(strlen(mstr->topic_name) + 10 + 1);
    char msgid_str[50];
    strcpy(file_name, mstr->topic_name);
    strcat(file_name, ".");
    
    sprintf(file_name, "%s%d", file_name, mstr->msg_id);
    
    printf("\n%s\n%d", file_name, strlen(file_name));


    fptr = fopen(file_name, "r");
    if(fptr == NULL){
        printf("\nfile does not exist\n");
		//fclose(fptr);
        return 0;
    }
    char message[MAX_MESSAGE_SIZE];
    fscanf(fptr, "%s", message);
    printf("%s", message);

    strcpy(mstr->message, message);
	fclose(fptr);
    return 1;

}

void get_max_msgid_broker_file(master *mstr){
    FILE *fptr;

    if((fptr = fopen(mstr->topic_name, "r")) == NULL){
        printf("topic file does not exist");
		
        return;
    }

    char msgid_str[10];
    fscanf(fptr, "%s", msgid_str);
    int msgid = atoi(msgid_str);
    
    if(msgid > mstr->msg_id){
        mstr->msg_id = msgid;
		fclose(fptr);
        return; 
    }
    else {
		fclose(fptr);
        return;

	}
	
}

void create_message_file(master mstr){
    FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr.topic_name) + 10 + 1);
    strcpy(file_name, mstr.topic_name);
    strcat(file_name, ".");
    sprintf(file_name, "%s%d", file_name, mstr.msg_id);
    printf("\n\n%s\n\n", file_name);
    if((fptr = fopen(file_name, "w")) == NULL){
        printf("\nerror while creating message file\n");
		

        return;
    }

    fprintf(fptr, "%s", mstr.message);
    fclose(fptr);
    if((fptr = fopen(mstr.topic_name, "w")) == NULL){
        printf("\nerror while opening topic file for updatign msgid\n");
    }
    
    fprintf(fptr, "%d", mstr.msg_id);
	fclose(fptr);
    return;

}
