#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"
#include <unistd.h> 
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define PORT 4444 
#define BUF_SIZE 2000 
#define SERADDR_LEN 100


void* clienthread(void* args) {

    struct sockaddr_in addr, cl_addr;  
    int sockfd, ret;  
    char buffer[BUF_SIZE];  
    struct hostent * server;
    char serverAddr[SERADDR_LEN];
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  

    if (sockfd < 0) {  
        printf("Error creating socket!\n");  
        exit(1);  
    }  
    printf("\nSocket created...\n");   

    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;                                             
    addr.sin_port = PORT;     

    ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));  

    if (ret < 0) {  
        printf("Error connecting to the server!\n");  
        exit(1);  
    }  
    printf("Connected to the server...\n");  

    inet_ntop(AF_INET, &(addr.sin_addr), serverAddr, SERADDR_LEN);

    printf("Server address = %s\n\n", serverAddr);

    for(int i=1;i<=20;i++) {

        printf("thread id = %lu\n", pthread_self());

        ret = write(sockfd, &i, sizeof(i));

        if (ret < 0) {  
            printf("Error sending data!\n");  
        }
        else {
            printf("Sent data to %s, %d: %d\n", serverAddr, addr.sin_port, i);
        } 

        unsigned long fact;

        ret = read(sockfd, &fact, sizeof(fact));

        if (ret < 0) {  
            printf("Error receiving data!\n\n");    
        } 
        else {
            printf("Received data from %s: %lu\n\n", serverAddr, fact);
        }
    }

    pthread_exit(NULL);
 
    return 0;
}

  
int main() {  

    pthread_t tid[15];

    for(int i=1;i<=10;i++) {   

        int error = pthread_create(&tid[i], NULL, clienthread, &i);
      
        if (error != 0)
            printf("\nThread can't be created for client = %d: [%s]\n\n", i, strerror(error));
        else 
            printf("\nThread created for client = %d\n\n", i);

        pthread_join(tid[i], NULL);
    }         

    pthread_exit(NULL);

    return 0;    
}  