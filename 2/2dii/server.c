#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>

#define SERVER_PORT  7001


int main (int argc, char *argv[])
{
  int    len, rc, on = 1;
  int    listen_sd = -1, new_sd = -1;
  int    desc_ready, end_server = 0, compress_array = 0;
  int    close_conn;
  struct sockaddr_in addr;
  int    timeout;
  struct pollfd fds[15];
  int    nfds = 1, current_size = 0, i, j;

  listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sd < 0)
  {
    perror("socket failed ......");
    exit(-1);
  }


  rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if (rc < 0)
  {
    perror("setsockopt failed ......");
    close(listen_sd);
    exit(-1);
  }

  rc = ioctl(listen_sd, FIONBIO, (char *)&on);
  if (rc < 0)
  {
    perror("ioctl failed ......");
    close(listen_sd);
    exit(-1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;  
  addr.sin_port = htons(SERVER_PORT);


  rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0) {
    perror("binding failed ......");
    close(listen_sd);
    exit(-1);
  }

  rc = listen(listen_sd, 32);
  if (rc < 0) {
    perror("listen failed .......");
    close(listen_sd);
    exit(-1);
  }

  memset(fds, 0 , sizeof(fds));

  fds[0].fd = listen_sd;
  fds[0].events = POLLIN;
  
  timeout = (3 * 60 * 1000);

  FILE *filePointer = NULL;

  int cnt=0;

  while (end_server == 0) {
    
    printf("Waiting for connections......\n");
    rc = poll(fds, nfds, timeout);

    if (rc < 0)
    {
      perror("poll failed .......");
      break;
    }

    if (rc == 0)
    {
      printf("poll timed out ...... End program.\n");
      break;
    }

    current_size = nfds;

    for (i = 0; i < current_size; i++) {

      if(fds[i].revents == 0) {
        continue;
      }

      if(fds[i].revents != POLLIN) {
          
          printf("Error! revents = %d\n", fds[i].revents);
          end_server = 1;
          break;
      }
      
      if (fds[i].fd == listen_sd) {

        printf("Listening socket is readable ......\n");

        do {

          new_sd = accept(listen_sd, NULL, NULL);
          if (new_sd < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("accept failed ......");
              end_server = 1;
            }
            break;
          }

          printf("New connection established - %d\n", new_sd);
          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

          cnt++;

          if(cnt==1) {

            if ((filePointer = fopen("output.txt", "a")) == NULL) { 
                perror("error"); 
                exit(1); 
            }
            else {
                printf("\nFile opened ......\n\n");
            }
          }

        } while(new_sd!=-1);
      }
      else {

        close_conn = 0;

        while(1) {

          int receive;
          rc = read(fds[i].fd, &receive, sizeof(receive));
          if(receive==-1) {   
            close_conn=1;
            break;
          }
          if (rc < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("read failed ......");
              close_conn = 1;
            }
            break;
          }

          int addrlen = sizeof(addr);
          getpeername(fds[i].fd , (struct sockaddr*)&addr, (socklen_t*)&addrlen);

          printf("Received data on socket = %d from %s, %d: %d\n", fds[i].fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), receive);

          unsigned long fact = 1;

          for(int i=2;i<=receive;i++) {
            fact*=i;
          }

          rc = write(fds[i].fd, &fact, sizeof(fact));

          if (rc < 0)
          {
            perror("send failed ......");
            close_conn = 1;
            break;
          }

          fflush(filePointer); 
          fprintf(filePointer, "\nClient IP Address = %s\nClient port = %d\nFactorial of %d = %lu\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), receive, fact); 
        }

        if (close_conn)
        {
          close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = 1;

          cnt--;

          if(cnt==0) {
            fclose(filePointer);
            printf("File closed ......\n\n");
          }
        }
      }  
    } 

    if (compress_array)
    {
      compress_array = 0;
      for (i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          for(j = i; j < nfds; j++)
          {
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          nfds--;
        }
      }
    }

  }

  for (i = 0; i < nfds; i++)
  {
    if(fds[i].fd >= 0) {
      close(fds[i].fd);
      cnt--;
      if(cnt==0) {
        fclose(filePointer);
        printf("File closed....\n\n");
      }
    }
  }

  return 0;
}