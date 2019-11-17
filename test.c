#include<stdlib.h>
#include<stdio.h>
#include "header.h"

int get_message_file(master *mstr){

    FILE *fptr;

    char *file_name = (char *)malloc(strlen(mstr->topic_name) + 10 + 1);
    char msgid_str[50];
    strcat(file_name, mstr->topic_name);
    strcat(file_name, ".");
    // itoa(mstr->msg_id, msgid_str, )
    sprintf(file_name, "%s%d", file_name, mstr->msg_id);
    //strcat(file_name, (char *)itoa(mstr->msg_id));
    printf("\n%s\n%d", file_name, strlen(file_name));


    fptr = fopen(file_name, "r");
    if(fptr == NULL){
        printf("\nfile does not exist\n");
        return 0;
    }
    char message[MAX_MESSAGE_SIZE];
    fscanf(fptr, "%s", message);
    printf("%s", message);

    strcpy(mstr->message, message);
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
        return; 
    }
    else 
        return;

}

void create_message_file(master mstr){
    FILE *fptr;
    char *file_name = (char *)malloc(strlen(mstr.topic_name) + 10 + 1);
    strcat(file_name, mstr.topic_name);
    strcat(file_name, ".");
    sprintf(file_name, "%s%d", file_name, mstr.msg_id);
    if((fptr = fopen(file_name, "w")) == NULL){
        printf("\nerror while creating message file\n");
        return;
    }

    fprintf(fptr, "%s", mstr.message);

    if((fptr = fopen(mstr.topic_name, "w")) == NULL){
        printf("\nerror while opening topic file for updatign msgid\n");
    }
    
    fprintf(fptr, "%d", mstr.msg_id);
    return;

}





int main(){
    master t1;
    strcpy(t1.topic_name, "nihar1");
    strcpy(t1.message, "makabharass");
    t1.msg_id = 3;

    //get_message_file(&t1);
    //get_max_msgid_broker_file(&t1);
    create_message_file(t1);
    printf("%d", t1.msg_id);

    // char string[100] = "nihar";
    // char string2[50];
    // strcat(string2, string);
    // printf("\n%s\n%d", string2, strlen(string2));
}