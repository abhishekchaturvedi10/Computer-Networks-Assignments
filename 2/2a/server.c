#include"stdio.h"
#include"stdlib.h"
#include"sys/types.h"
#include"sys/socket.h"
#include"string.h"
#include"netinet/in.h"
#include <arpa/inet.h>
#include <unistd.h> 
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define PORT 4444
#define BUF_SIZE 2000
#define CLADDR_LEN 100

FILE *filePointer;


struct entity {
   int socketfd;
   char clientaddr[CLADDR_LEN];
   int port;
};


void* clienthread(void *args) {

   struct entity* en = (struct entity*) args;

   int newsockfd = en->socketfd;

   while(1) {

      printf("Client IP address = %s\n", (en->clientaddr));
      printf("Client port = %d\n", en->port);

      printf("Socket fd = %d\n", newsockfd);

      int rec, ret;

      ret = read(newsockfd, &rec, sizeof(rec));

      if(ret < 0) {
         printf("Error receiving data!\n");  
         exit(1);
      }
      printf("Received data from %s: %d\n", (en->clientaddr), rec);

      unsigned long fact = 1;

      for(int i=2;i<=rec;i++) {
         fact*=i;
      }  

      ret = write(newsockfd, &fact, sizeof(fact)); 

      if(ret < 0) {
         printf("Error sending data!\n");  
         exit(1);
      }
      printf("Sent data to %s: %lu\n\n", (en->clientaddr), fact);

      fflush(filePointer);
      
      fprintf(filePointer, "\nClient IP Address = %s\nClient port = %d\nFactorial of %d = %lu\n", (en->clientaddr), en->port, rec, fact);

      if(rec==20) {
         break;
      }
   } 

   close(newsockfd);

   printf("Thread %lu done with its client!\n\n", pthread_self());

   pthread_exit(NULL);

   return 0;
}

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
   listen(sockfd, 15);

   pthread_t tid[15];

   int cnt=1;

   filePointer = NULL;

   while(cnt<11) { 

      printf("\n");

      len = sizeof(cl_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
         
      if (newsockfd < 0) {
         continue;
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

      struct entity en;

      en.socketfd = newsockfd;
      inet_ntop(AF_INET, &(cl_addr.sin_addr), (en.clientaddr), CLADDR_LEN);
      en.port = cl_addr.sin_port;

      int error = pthread_create(&tid[cnt], NULL, clienthread, (void*) &en);
      
      if (error != 0)
         printf("\nThread can't be created for client = %d: [%s]\n\n", cnt, strerror(error));
      else 
         printf("\nThread created for client = %d\n\n", cnt);

      pthread_join(tid[cnt], NULL);

      cnt++;
   }

   close(sockfd);

   fclose(filePointer);

   printf("File closed....\n\n");

   t = clock() - t;
   double time_taken = ((double)t)/CLOCKS_PER_SEC;   
   printf("\nExecution time = %f\n\n", time_taken);

   pthread_exit(NULL);
}