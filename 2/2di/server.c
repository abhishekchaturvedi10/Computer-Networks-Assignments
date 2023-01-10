#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sys/ioctl.h>
     
#define TRUE   1 
#define FALSE  0 
#define PORT 7001
     
int main(int argc , char *argv[]) {

    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 10 , activity, i , valread , sd;  
    int max_sd;  
    struct sockaddr_in address;  
         
    char buffer[1025]; 
         
    fd_set readfds;  
              
    for (i = 0; i < max_clients; i++) {  
        client_socket[i] = 0;  
    }  
         
    if( (master_socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0) {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }   
     
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT); 

         
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))!=0) {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    if (listen(master_socket, 10) != 0) {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  


    u_long optval = 1;
    int ret = ioctl(master_socket, FIONBIO, &optval);
    if(ret<0) {
        printf("\nioctl failed for socket = %d\n", new_socket);
        exit(1);
    }
    else {
        printf("\nioctl success for socket = %d\n", new_socket);
    }
         
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  

    FILE *filePointer;

    int cnt=0;
         
    while(1) {

        FD_ZERO(&readfds);  
     
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  

        for ( i = 0; i < max_clients; i++) {

            sd = client_socket[i];  
                 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
      
        activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR)) {  
            printf("select error"); 
            break;
        }  
              
        if (FD_ISSET(master_socket, &readfds)) {

            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  

            printf("\nNew connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));             
                                  
            for (i = 0; i < max_clients; i++) {  
                
                if( client_socket[i] == 0 ) {

                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n\n" , i+1);  
                         
                    break;  
                }  
            }  

            cnt++;

            if(cnt==1) {

                if ((filePointer = fopen("output.txt", "a")) == NULL) { 
                    perror("error"); 
                    exit(1); 
                }
                else {
                    printf("\nFile opened....\n\n");
                }
            } 
        }  
             
        for (i = 0; i < max_clients; i++)  {

            sd = client_socket[i];     
                 
            if (FD_ISSET(sd, &readfds))  {

                int rec;

                read(sd, &rec, sizeof(rec));

                if (rec == -1) {  

                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);  
                    printf("\nHost disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    printf("Client %d disconnected\n\n", i+1);  
                         
                    close(sd);  
                    client_socket[i] = 0;  

                    cnt--;

                    if(cnt==0) {
                        fclose(filePointer);
                        printf("File closed....\n\n");
                    }
                }  
                else {  

                    printf("Received data from %s, %d: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), rec);

                    unsigned long fact = 1;

                    for(int i=2;i<=rec;i++) {
                        fact*=i;
                    }

                    write(sd, &fact, sizeof(fact));

                    fflush(filePointer);
      
                    fprintf(filePointer, "\nClient IP Address = %s\nClient port = %d\nFactorial of %d = %lu\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), rec, fact); 
                }  
            }  
        }  
    }  
         
    exit(0);

    return 0;  
}  