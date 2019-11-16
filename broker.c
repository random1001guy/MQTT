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


void connect_to_neighbour(char **argv);
void let_neighbour_connect();
void foo();
void handle_data(master mstr);
int get_message(master *mstr);
void get_max_msgid_broker(master *mstr);
void create_message(master mstr);




int main(int argc, char *argv[]){

    if(argc!=3){
        printf("Usage : ./broker <NeighbourBroker_IP> <NeighbourBroker_Port>\n");
        exit(0);
    }
    
    printf("Please enter my broker ID : ");
    scanf("%d",&myBrokerID);
    printf("Please enter my Port No. : ");
    scanf("%d",&my_port);
    

	if((fptr = fopen(PATH_LOG,"w")) == NULL)
	    DieWithError("log file open");
	fprintf(fptr, "Broker %d started. Connected to Broker with IP : %s Port %s\n",myBrokerID,argv[1],argv[2]);

	head=NULL;

    connect_to_neighbour(argv);
    let_neighbour_connect();
    foo();

}

void connect_to_neighbour(char **argv){

	struct sockaddr_in server;

	if( (to_neighbour_sockfd = socket(AF_INET, SOCK_STREAM, 0)) ==-1){
		perror("socket()");
		exit(0);
	}
	fprintf(fptr, "Broker %d : socket() successful\n",myBrokerID);

	bzero(&server,sizeof(server));
	server.sin_family= AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &server.sin_addr);

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
	maxfd = listenfd;




	for(;;){



		rset = superset;

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


					else{										//send the data to even/odd clients
						mstr.client_id = i;
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
				
				pid = fork();
				if(pid == 0){
					mstr.type = RPL_MAX_MSGID;

					send_master(mstr.client_id, mstr);
					fprintf(fptr,"RPL_MAX_MSGID sent to client %d\n",mstr.client_id);
					exit(0);
				}
				else 
					return;
			}
			else{
				
				 pid = fork();
				if(pid == 0){
					if(mstr.broker_id == -1)
						mstr.broker_id = myBrokerID;
					
					get_max_msgid_broker(&mstr);
					send_master(mstr.client_id, mstr);
					fprintf(fptr,"GET_MAX_MSGID sent to my meighbour\n");
					exit(0);
				}
				else 
					return;

			}
			break;



		case CREATE_MSG: 
			create_message(mstr);
			break;

		case GET_MSG: 
			pid = fork();
			if(pid == 0){
			
				if(mstr.broker_id == myBrokerID){	

						send_master(mstr.client_id, mstr);
						fprintf(fptr,"GET_MSG sent to client %d\n",mstr.client_id);
						exit(0);
				}
				else if(mstr.broker_id == -1){
					
					mstr.broker_id = myBrokerID;
					int reply = get_message(&mstr);
					if(reply == 1){							//message found
						mstr.type = MSG_RETRIEVED;
						send_master(mstr.client_id, mstr);
						fprintf(fptr,"MSG_RETRIEVED sent to client %d\n",mstr.client_id);
						exit(0);
					}
					else{
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"GET_MSG sent to my neighbour broker\n");
						exit(0);
					}
				}
				else{
					
					int reply = get_message(&mstr);
					if(reply == 1){							//message found
						mstr.type = MSG_RETRIEVED;
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"MSG_RETRIEVED sent to my neighbour broker\n");
						exit(0);
					}
					else{
						send_master(to_neighbour_sockfd, mstr);
						fprintf(fptr,"GET_MSG sent to my neighbour broker\n");
						exit(0);
					}
				}
			}
			else 
				return;

			break;



		case MSG_RETRIEVED: 
			int pid =fork();
			if(pid ==0){
				if(mstr.broker_id == myBrokerID){
					send_master(mstr.client_id, mstr);
					fprintf(fptr,"MSG_RETRIEVED sent to client %d\n",mstr.client_id);
					exit(0);
				}
				else{
					send_master(to_neighbour_sockfd, mstr);
					fprintf(fptr,"MSG_RETRIEVED sent to my neighbour broker\n");
					exit(0);
				}
			}
			else 
				return;
			break;



		default:
			perror("flow reached default case in switch");

	}	//endswitch
}


int get_message(master *mstr){


}

void get_max_msgid_broker(master *mstr){


}

void create_message(master mstr){
	topic_item *t1 = (topic_item *) malloc(sizeof(topic_item));
	message_item *m1 = (message_item *) malloc(sizeof(message_item));
	
	if(head == NULL){
		
		t1->first_msg = m1;
		t1->next = NULL;
		strcpy(t1->topic_name, mstr.topic_name);
		m1->msg_id = mstr.msg_id;
		m1->next = NULL;
		strcpy(m1->message, mstr.message);
		head = t1;	
		return;
	}
	else{
		topic_item *temp;
		t1 = head;
		
			
		while(1){
			
			if(strcmp(t1->topic_name, mstr.topic_name)==0){
				
				m1=t1->first_msg;
				while(m1->next !=NULL){
					m1 = m1->next;
				}
				message_item temp;
				temp.msg_id = mstr.msg_id;
				temp.next = NULL;
				strcpy(temp.message, mstr.message);
				m1->next = &temp;
				return;
			}

			t1 = t1->next;
			if(t1->next==NULL){
				break;
			}
		}
		if(strcmp(t1->topic_name, mstr.topic_name)==0){
			topic_item t2;
			message_item m2;

			t2.next = NULL;
			strcpy(t2.topic_name, mstr.topic_name);
			t2.first_msg = &m2;
			m2.msg_id = mstr.msg_id;
			m2.next = NULL;
			strcpy(m2.message, mstr.message);

			t1->next = &t2;
		}
		else{
			m1=t1->first_msg;
				while(m1->next !=NULL){
					m1 = m1->next;
				}
				message_item temp;
				temp.msg_id = mstr.msg_id;
				temp.next = NULL;
				strcpy(temp.message, mstr.message);
				m1->next = &temp;
				return;
		}

	}
	
}