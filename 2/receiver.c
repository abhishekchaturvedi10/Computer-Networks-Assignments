#include <unistd.h>
#include <Winsock2.h>
#include <unistd.h>
#include <netinet/in.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>

struct processData{
  int actual_cpu;
  int kernel_cpu;
  int pid;
  char name[200];
  int total_cpu;  
};
void handleError(char* str){
    //print something here
    printf("%s ",str);
    perror("");
    _exit(-1);
}
int checkInteger(char* str){
    char str2[10];
    int val=atoi(str);
    if(val==0)
        return -1;
    sprintf(str2,"%d",atoi(str));
    // itoa(atoi(str),str2,10);
    if ( strlen(str) == strlen(str2) ) {
        //its an integer
        return 0;
    }
    return -1;
}
struct processData parseDetailsProcess(char* str){
    struct processData var;
    int temp; char temp2[20];
    sscanf(str,"%d %s %s %d %d %d %d %d %d %d %d %d %d %d %d",&var.pid, var.name, temp2, &temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp, &var.actual_cpu, &var.kernel_cpu);
    var.total_cpu=var.kernel_cpu+var.actual_cpu;
    return var;
}

struct processData checkProcesses(){
    DIR *dr=opendir("/proc");
    struct dirent *entry;
    entry=readdir(dr);
    int len=0;
    struct processData arr[1000];
    while(entry!=NULL){
        if(entry->d_type==DT_DIR&&checkInteger(entry->d_name)!=-1){
            char path[256]; path[0]='\0';
            strcat(path,"/proc/");
            strcat(path,entry->d_name);
            strcat(path,"/stat");
            int fd= open(path, O_RDONLY);
            if(fd<0){
                //handle error
                entry=readdir(dr);
                continue;
            }
            char buf[1000];
            read(fd, buf, sizeof(buf));

            arr[len]=parseDetailsProcess(buf);
            len++;
            close(fd);
        }
        entry=readdir(dr);
    }

        struct processData temp;
        for(int j=0;j<len-1;j++){
            if(arr[j+1].total_cpu<arr[j].total_cpu){
                temp=arr[j+1];
                arr[j+1]=arr[j];
                arr[j]=temp;
            }
        }
    return arr[len-1];
}

void main(){
    int sockfd=socket(AF_INET, SOCK_STREAM,0);
    if(sockfd<0){
        //error handle
        handleError("Socket");
    }

    struct sockaddr_in server,client;
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(8001);

    int recvfd= connect(sockfd, (struct sockaddr*) &server, sizeof(server));
    if(recvfd<0){
        //error handle
        handleError("connection error");
    }
    printf("Connected successfully \n");
    char filename[20];
    int size=recv(sockfd,filename,sizeof(filename),0);
    filename[size]='\0';

    int fd=open(filename,O_WRONLY|O_CREAT,0666);
    if(fd<0){
        //error handle
        handleError("file opening");
    }

    char buf[1001];
    size=recv(sockfd, buf, sizeof(buf),0);
    buf[size]='\0';
    // printf("Received %d %d \n", size, strlen(buf));
    write(fd,buf,size);
    close(fd);

    int kcpu, acpu, pid;
    char name[256];
    struct processData var=checkProcesses();
    sprintf(buf,"%d %s %d %d", var.pid,var.name,var.actual_cpu,var.kernel_cpu);
    printf("sending %s \n",buf);
    //send this back to server
    send(sockfd,buf,strlen(buf),0);
    sleep(30);
    close(sockfd);

    return;
}