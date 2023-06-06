
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}


int main(int argc, char *argv[]) {
    int str_len;
    int serv_sock,clnt_sock1=-1;
    
    int clnt_sock2=-1;
    
    struct sockaddr_in serv_addr,clnt_addr1,clnt_addr2;
    socklen_t clnt_addr_size;
    char msg[100];
    
    if(argc!=2){
        printf("Usage : %s <port>\n",argv[0]);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(serv_sock == -1)
        error_handling("socket() error");
    
    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error");
    if(listen(serv_sock,5) == -1)
        error_handling("listen() error");

    if(clnt_sock1<0){   
		        
        clnt_addr_size = sizeof(clnt_addr1);

        clnt_sock1 = accept(serv_sock, (struct sockaddr*)&clnt_addr1, &clnt_addr_size);

        if(clnt_sock1 == -1)
            error_handling("accept() error");   
    }
    if(clnt_sock2<0){   
		        
        clnt_addr_size = sizeof(clnt_addr2);

        clnt_sock2 = accept(serv_sock, (struct sockaddr*)&clnt_addr2, &clnt_addr_size);

        if(clnt_sock2 == -1)
            error_handling("accept() error");   
    }

	
    while(1)
    {           
        str_len = read(clnt_sock1, msg , sizeof(msg));
        printf("msg = %s\n",msg);
        write(clnt_sock2,msg,sizeof(msg));
        usleep(100000);
        
    }
}
