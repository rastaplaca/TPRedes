

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  char file[BUFSIZE];
  int first;
  int i;
  int j;

  portno = 5000;
  /*
   * socket: create the parent socket
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /*
   * bind: associate the parent socket with a port
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /*
   * listen: make this socket ready to accept connection requests
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");

  /*
   * main loop: wait for a connection request, echo input line,
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);

    bzero(buf, BUFSIZE);
    n = read(childfd, buf, BUFSIZE);
    
    memset (file,'\0',BUFSIZE);
    
    i = 5;
    j = 0;
    while(buf[i]!=' '){
        file[j] = buf[i];

        i++;
        j++;
    }
    
    FILE *fp;
    fp = fopen(file,"rb");

    if(fp == NULL){
      char message[] = "HTTP/1.1 404 \r\n\r\n";
      send(childfd, message, strlen(message), 0);
    }
    else{
        
        char message[] = "HTTP/1.1 200 OK\r\n\r\n";
        char c;

        send(childfd, message, strlen(message), 0);
        
        i=0;
        memset (buf,'\0',BUFSIZE);
        while((c = fgetc(fp)) != EOF){
              buf[i] = c;
              i++;
              if(i==BUFSIZE-1){
                  send(childfd, buf, BUFSIZE, 0);
                  memset (buf,'\0',BUFSIZE);

                  i=0;  
              }
        }
        
        if(i==0){
          send(childfd, NULL, 0, 0);
        }
        else{
          
          send(childfd, buf, i, 0); 
          send(childfd, NULL, 0, 0);
        }
        fclose(fp);
    }

    close(childfd);
  }
}
