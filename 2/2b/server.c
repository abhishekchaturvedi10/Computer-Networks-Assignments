   #include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include <arpa/inet.h>
#include <unistd.h> 
#include<sys/wait.h>
#include <time.h>


#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100


FILE *filePointer;


void main() {

   clock_t t;
   t = clock();

   struct sockaddr_in addr, cl_addr;
   int sockfd, len, ret, newsockfd;
   char buffer[BUF_SIZE];
   pid_t childpid;
   char clientAddr[CLADDR_LEN];
 
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      printf("Error creating socket!\n");
      exit(1);
   }
   printf("Socket created...\n");
 
   memset(&addr, 0, sizeof(addr));

   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port = PORT;
 
   ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

   if (ret < 0) {
      printf("Error binding!\n");
      exit(1);
   }
   printf("Binding done...\n");

   printf("Waiting for a connection...\n\n");
   listen(sockfd, 10);

   int cnt=1;

   while(1) {  

      printf("\n");

      len = sizeof(cl_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
      
      if (newsockfd < 0) {
         // exit(0);
         break;
      }
      printf("\nConnection accepted...\n");


      if(cnt==1) {
         if ((filePointer = fopen("output.txt", "a")) == NULL) { 
            perror("error"); 
            exit(1); 
         }
         else {
            printf("\nFile opened....\n\n");
         }
      }

      inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);

      if ((childpid = fork()) == 0) { 

         close(sockfd); 

         while(1) {

            printf("Client IP address = %s\n", clientAddr);
            printf("Client port = %d\n", cl_addr.sin_port);

            printf("Socket fd = %d\n", newsockfd);

            int rec;

            ret = read(newsockfd, &rec, sizeof(rec));

            if(ret < 0) {
               printf("Error receiving data!\n");  
               exit(1);
            }

            printf("Received data from %s: %d\n", clientAddr, rec);

            // sleep(2);

            unsigned long fact = 1;

            for(int i=2;i<=rec;i++) {
               fact*=i;
            }  

            ret = write(newsockfd, &fact, sizeof(fact)); 

            if(ret < 0) {
               printf("Error receiving data!\n");  
               exit(1);
            }

            printf("Sent data to %s: %lu\n\n", clientAddr, fact);

            fflush(filePointer);

            fprintf(filePointer, "\nClient IP Address = %s\nClient port = %d\nFactorial of %d = %lu\n", clientAddr, cl_addr.sin_port, rec, fact);

            if(rec==20) {
               break;
            }
         } 

         printf("Process %d done with its client!\n\n", getpid());
      }

      cnt++;

      close(newsockfd);
   }

   if(cnt==10) {

      sleep(1);

      fclose(filePointer);
      printf("File closed....\n\n");

      t = clock() - t;
      double time_taken = ((double)t)/CLOCKS_PER_SEC;
      printf("\nExecution time = %f\n\n", time_taken);
   }

   exit(0);
}