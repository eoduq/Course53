//
//  main.c
//  clnt
//
//  Created by DD MPR on 2022/06/13.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>



#define MSG_SIZE 100
#define NAME_SIZE 20

void *send_msg(void *arg); // send message to server
void *recv_msg(void *arg); // receive message from server


struct clnt_info{
    int sock;
    char clnt_nick[NAME_SIZE];
};

int main(int argc, const char * argv[]) {
    // insert code here...
    struct clnt_info clnt;
    memset(clnt.clnt_nick,0,sizeof(clnt.clnt_nick));
    clnt.sock=socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    
    /*
    The client makes a TCP connection with the server using the command-line arguments
   (an IP address and a port number) in this application
    
   */
    if(argc!=4){
        printf("Err: The client MUST take three command-line arguments\n");
        exit(1);
    }
    
    strncpy(clnt.clnt_nick,argv[3],strlen(argv[3]));
    clnt.clnt_nick[strlen(argv[3])]='\0';
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
  
    if(connect(clnt.sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
        printf("connect() error\n");
        exit(1);
    }
  
    pthread_create(&snd_thread, NULL, send_msg, (void*)&clnt);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&clnt);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    
    close(clnt.sock);
    return 0;
}


void *send_msg(void *arg){
    struct clnt_info c1= *((struct clnt_info *)arg);
    write(c1.sock, c1.clnt_nick, sizeof(c1.clnt_nick)-1);
    char name_msg[NAME_SIZE+MSG_SIZE]={0,};
    char msg[MSG_SIZE]={0,};
    
    while(1){
        memset(name_msg,0,sizeof(name_msg));
        memset(msg,0,sizeof(msg));
        fgets(msg, MSG_SIZE-1, stdin);
        
        msg[strlen(msg)-1]='\0';
       
        
        if(!strcmp(msg, "QUIT")){
            
            
            write(c1.sock,msg,sizeof(msg)-1);
            sprintf(name_msg, "%s is disconnected",c1.clnt_nick);
            printf("%s\n",name_msg);
            close(c1.sock);
            
            exit(0);
        }
        
        
        write(c1.sock, msg, sizeof(msg));
        
        printf("%s\n",msg);
        
        
        
    }
    return NULL;
}

void *recv_msg(void *arg){
    struct clnt_info c1=*((struct clnt_info *)arg);
    char name_msg[NAME_SIZE+MSG_SIZE];
    
    long str_len=0;
    while(1){
        str_len=0;
        memset(name_msg,0,sizeof(name_msg));
        str_len=read(c1.sock, name_msg, sizeof(name_msg)-1);
        if(str_len==-1){
            return (void *) -1;
            
        }
        name_msg[str_len]='\0';
        printf("%s \n",name_msg);
        
    }
    
    return NULL;
    
}
