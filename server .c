//
//  main.c
//  Term2_serv
//
//  Created by DD MPR on 2022/06/10.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


#define MAX_CLNT 5      //The server only accepts up to 5 TCP clients for the chatting program
#define MSG_SIZE 100    //The length of message is up to 100
#define NAME_SIZE 20    //The length of message is up to 20

void * handle_clnt(void *arg);
void send_msg_to_all(char *msg, int len);
void send_msg_except(char *msg, int len, int ex_clnt);

int clnt_cnt=0;         //A variable to count the number of client
int clnt_socks[MAX_CLNT];//Integer array for client socket's file descriptor

/*
 thread 생성시 함수에 전달될 구조체
 */
struct clnt_info{
    int serv_sock;//server socket file descriptor
    int clnt_sock;//client socket file descriptor
    char clnt_nick[20];//nimck naem of client
    struct sockaddr_in clnt_adr;
};

pthread_mutex_t mutx;   //mutex

int main(int argc, const char * argv[]) {
    // insert code here...
    
    struct clnt_info clnt;
    clnt.serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    unsigned int clnt_adr_sz=0;
    struct sockaddr_in serv_adr;
    pthread_mutex_init(&mutx, NULL);
    pthread_t t_id;
    
    /*
     The server MUST take one command-line argument as a port number for the program.
     */
    if(argc !=2){
        printf("input error\n");
        exit(1);
    }
    
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
    
    if(bind(clnt.serv_sock, (struct sockaddr*) &serv_adr,sizeof(serv_adr))==-1){
        printf("bind() error\n");
        exit(1);
        
    }
    
    if(listen(clnt.serv_sock, 5)==-1){
        printf("listen() error");
        exit(1);
    }
    
    while(1){
        
        clnt_adr_sz=sizeof(clnt.clnt_adr);
        clnt.clnt_sock=accept(clnt.serv_sock,(struct sockaddr*)&clnt.clnt_adr, &clnt_adr_sz);
        if(clnt.clnt_sock==-1){
            printf("accept() error\n");
            exit(1);
        }
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++]=clnt.clnt_sock;
        
        pthread_mutex_unlock(&mutx);
        
        pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt);
        pthread_detach(t_id);
    }
   
    return 0;
}

void * handle_clnt(void *arg){
    
    
    struct clnt_info c1= *((struct clnt_info *)arg); //variable used in handle_clnt
    memset(c1.clnt_nick,0,sizeof(c1.clnt_nick));
    long msg_len=0;
    long nick_len;
    char msg[MSG_SIZE]={0,};
    char name_msg[NAME_SIZE+MSG_SIZE]={0,};
    
    
    /*
     If the TCP connection is made with a
     client, the client’s IP address and port number are printed to its standard output (e.g., “Connection from
     163.152.162.144:12345”).
     */
    printf("Connection from %s:%d\n",inet_ntoa(c1.clnt_adr.sin_addr),c1.clnt_adr.sin_port);
    
    /*
     After the connection, the server receives the nickname of the connected
     client (e.g., "User1", "User2"). Whenever a new client connects to the server, the server uses client's
     nickname to notify other connected clients of the new connection and also displays it on the server's
     screen (e.g., "User2 is connected").
     */
    
    if((nick_len=read(c1.clnt_sock,c1.clnt_nick,sizeof(c1.clnt_nick)))!=0){
        c1.clnt_nick[nick_len]='\0';
        sprintf(msg,"%s is connected",c1.clnt_nick);
        printf("%s  \n",msg);
        send_msg_to_all(msg,sizeof(msg));
    }
    
    memset(msg,0,sizeof(msg));
    memset(name_msg,0,sizeof(name_msg));
    while((msg_len=read(c1.clnt_sock,msg,sizeof(msg)))>=0){
        msg[msg_len]='\0';
        
        /*
         If a client named User2 sends the message "QUIT” to the server, the server sends the message "User2
          is disconnected” to other clients and also prints it on server's screen. Then, the server closes its socket
          communicating with the client named User2
         */
        if(!strcmp(msg,"QUIT")){
            sprintf(name_msg, "%s is disconnected",c1.clnt_nick);
            printf("%s \n",name_msg);
            send_msg_except(name_msg, sizeof(name_msg), c1.clnt_sock);
            
            
            break;
        }
        sprintf(name_msg, "%s: %s", c1.clnt_nick, msg);
        printf("%s\n",name_msg);
        send_msg_except(name_msg, sizeof(name_msg), c1.clnt_sock);
        memset(msg,0,sizeof(msg));
        memset(name_msg,0,sizeof(name_msg));
    }
    
    pthread_mutex_lock(&mutx);
    for(int i=0;i<clnt_cnt;i++){
        if(c1.clnt_sock==clnt_socks[i]){
            while(i++<clnt_cnt-1){
                clnt_socks[i]=clnt_socks[i+1];
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(c1.clnt_sock);
    close(c1.serv_sock);
    
    
    return NULL;
}

void send_msg_to_all(char *msg,int len){
    pthread_mutex_lock(&mutx);
    for(int i=0;i<clnt_cnt;i++){
        write(clnt_socks[i],msg,len);
    }
    pthread_mutex_unlock(&mutx);
    
}

void send_msg_except(char *msg,int len, int ex_clnt){
    pthread_mutex_lock(&mutx);
    
    for(int i=0;i<clnt_cnt;i++){
        if(clnt_socks[i]!=ex_clnt){
            write(clnt_socks[i],msg,strlen(msg));
        }
        
    }
    
    pthread_mutex_unlock(&mutx);
    
}
