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


sem_t x, y;
int readercount = 0;
FILE *filePointer;
int global_cnt = 0;
clock_t t;



struct entity {
   int socketfd;
   char clientaddr[CLADDR_LEN];
   int port;
};



void* clienthread(void *args) {

   struct entity* en = (struct entity*) args;

   int newsockfd = en->socketfd;
   int port = en->port;

   printf("\nNewsockfd = %d\n\n", newsockfd);

   while(1) {

      printf("Client IP address = %s\n", en->clientaddr);
      printf("Client port = %d\n", en->port);

      sem_wait(&x);
      readercount++;
 
      if (readercount == 1) {
         sem_wait(&y);
      }
 
      sem_post(&x);

      printf("Socket fd = %d\n", newsockfd);

      int rec, ret;

      ret = read(newsockfd, &rec, sizeof(rec));

      if(ret < 0) {
         printf("Error receiving data!\n");  
         exit(1);
      }
      printf("Received data from %s: %d\n", (en->clientaddr),  rec);

      sleep(2);
 
      sem_wait(&x);
      readercount--;
 
      if (readercount == 0) {
         sem_post(&y);
      }
 
      sem_post(&x);

      sem_wait(&y);

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

      sem_post(&y);
    
      if(rec==20) {
         break;
      }
   } 

   printf("Thread %lu done with its client!\n\n", pthread_self());

   global_cnt++;

   if(global_cnt==10) {

      fclose(filePointer);
      printf("File closed....\n\n");

      close(newsockfd);

      t = clock() - t;
      double time_taken = ((double)t)/CLOCKS_PER_SEC;   
      printf("\nExecution time = %f\n\n", time_taken);
   }
   else {
      close(newsockfd);
   }

   pthread_exit(NULL);

   return 0;
}

void main() {

   t = clock();

   sem_init(&x, 0, 1);
   sem_init(&y, 0, 1);

   struct sockaddr_in addr, cl_addr;
   int sockfd, len, ret, newsockfd;
   char buffer[BUF_SIZE];
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

   while(1) { 

      printf("\n");

      len = sizeof(cl_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &len);
         
      if (newsockfd < 0) {
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

      struct entity en;

      en.socketfd = newsockfd;
      inet_ntop(AF_INET, &(cl_addr.sin_addr), (en.clientaddr), CLADDR_LEN);
      en.port = cl_addr.sin_port;

      int error = pthread_create(&tid[cnt], NULL, clienthread, (void*) &en);
      
      if (error != 0)
         printf("\nThread can't be created for client = %d: [%s]\n\n", cnt, strerror(error));
      else 
         printf("\nThread created for client = %d\n\n", cnt);

      cnt++;

      sleep(2);
   }

   for(int i=1;i<11;i++) {
      pthread_join(tid[cnt], NULL);
   }

   close(sockfd);

   pthread_exit(NULL);
}