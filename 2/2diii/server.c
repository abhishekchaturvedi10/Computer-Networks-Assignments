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
#include <sys/epoll.h>
#include <fcntl.h>


#define SERVER_PORT  8080

#define TRUE             1
#define FALSE            0


int main (int argc, char *argv[])
{
  struct epoll_event events[15], event;
  int epfd = epoll_create1(0);

  int    len, rc, on = 1;
  int    listen_sd = -1, new_sd = -1;
  int    desc_ready, end_server = FALSE, compress_array = FALSE;
  int    close_conn;
  struct sockaddr_in addr, cli_addr;
  int    timeout;
  int    nfds = 1, current_size = 0, i, j;
  char buf[50];

  listen_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_sd < 0)
  {
    perror("socket() failed");
    exit(-1);
  }else {
    printf("socket() success = %d\n\n", listen_sd);
  }

  rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if (rc < 0)
  {
    perror("setsockopt() failed");
    close(listen_sd);
    exit(-1);
  }
  else {
    printf("setsockopt() success\n\n");
  }

  rc = ioctl(listen_sd, FIONBIO, (char *)&on);
  if (rc < 0)
  {
    perror("ioctl() failed");
    close(listen_sd);
    exit(-1);
  }
  else {
    printf("ioctl() success\n\n");
  }

  // memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  // addr.sin_addr.s_addr = INADDR_ANY;  
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  addr.sin_port = htons(SERVER_PORT);

  rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0) {
    perror("bind() failed");
    close(listen_sd);
    exit(-1);
  }
  else {
    printf("bind() success\n\n");
  }

  if (fcntl(listen_sd, F_SETFD, fcntl(listen_sd, F_GETFD, 0) | O_NONBLOCK) == -1) {
    return -1;
  }
  return 0;

  rc = listen(listen_sd, 10);
  if (rc < 0) {
    perror("listen() failed");
    close(listen_sd);
    exit(-1);
  }
  else {
    printf("listen() success\n\n");
  }

  if (epfd == -1) {
    fprintf(stderr, "Failed to create epoll file descriptor\n");
    return 1;
  }

  event.events = EPOLLIN;
  event.data.fd = 0;

  if(epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event))
  {
    fprintf(stderr, "Failed to add file descriptor to epoll\n");
    close(epfd);
    return 1;
  }

 while(1){

  puts("round again");

  nfds = epoll_wait(epfd, events, 100, -1);

  printf("ndfs = %d\n\n", nfds);

    for(i=0;i<nfds;i++) {

      if (events[i].data.fd == listen_sd) {

        int socklen = sizeof(cli_addr);

        int conn_sock = accept(listen_sd, NULL, NULL);

        inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr), buf, sizeof(cli_addr));
        printf("[+] connected with %s:%d\n", buf, ntohs(cli_addr.sin_port));

        events[i].events = EPOLLIN | EPOLLONESHOT;

        epoll_ctl(epfd, EPOLL_CTL_ADD, events[i].data.fd, &events[i]);
      } 
      else if (events[i].events & EPOLLIN) {
        
        for (;;) {
          int receive = read(events[i].data.fd, &receive, sizeof(receive));
          if (receive==-1) {
            break;
          } 
          else {
            printf("[+] data: %d\n", receive);
            write(events[i].data.fd, &receive, sizeof(receive));
            printf("Received data from %s, %d: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), receive);
          }

          unsigned long fact = 1;

          for(int i=2;i<=receive;i++) {
            fact*=i;
          }

          rc = write(events[i].data.fd, &fact, sizeof(fact));
        }
      } 
      else {
        printf("[+] unexpected\n");
      }

      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        printf("[+] connection closed\n");
        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
        close(events[i].data.fd);
        continue;
      }
    }
 }

  return 0;
}