/*code based on http://www.cs.tau.ac.il/~eddiea/samples/IOMultiplexing/TCP-client.c.html*/
    
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#define PORT 80         /* the port client will be connecting to */
#define MAXDATASIZE 5000 /* max number of bytes we can get at once */

int sendMessage(const char *message, int sockfd){
    if (send(sockfd, message, strlen(message), 0) == -1){
        perror("send");
		return 1;
	}

    return 0;
}

int main(int argc, char *argv[]){
    int sockfd;
    struct hostent *he;
    char url[MAXDATASIZE];
    char get[MAXDATASIZE];
    struct sockaddr_in their_addr;
    char filename[MAXDATASIZE];
    int barcount = 0;

    if (argc != 2) {
        printf("usage: %s LINK\n",argv[0]);
        exit(1);
    }
    
    int i;
    int j;

    memset (url,'\0',MAXDATASIZE);
    memset (get,'\0',MAXDATASIZE);
    memset (filename,'\0',MAXDATASIZE);
    
    i=0;
    while(argv[1][i] != '\0' && argv[1][i] !='/' && i<strlen(argv[1])){
        url[i] = argv[1][i];
        i++;  
    }

    if(i<strlen(argv[1]) && argv[1][i] == '/'){
        j = 4;
        strcat(get, "GET ");
        while(i<strlen( argv[1])){
            get[j] = argv[1][i];
            
            if(argv[1][i]=='/')
                barcount++;

            i++;
            j++;
        }
    }
    
    if(get[0]=='\0'){
        strcat(get, "GET /"); 
        strcat(filename, "index.html"); 
    }

    else{
        int barcount2=0;
        i = 0;
        while(barcount2<barcount){
            if(get[i]=='/'){
                barcount2++;
            }
            i++;
        }  
        j=0;
        while(i<strlen(get)){
            filename[j] = get[i];

            j++;
            i++;
        }  
    }
    
    strcat(get, "\n");
    strcat(get, "HTTP/1.1\r\n");
    strcat(get, "Host: ");
    strcat(get, url);
    strcat(get, "\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: TPRedes (X11; Linux x86_64)\r\n"
    "Accept: text/html\r\nAccept-Encoding: deflate\r\n"
    "Accept-Language: en-US,en;q=0.9,pt-BR;q=0.8,pt;q=0.7\r\n\r\n");
    printf("\n\n%s\n", get);
    

    if ((he=gethostbyname(url)) == NULL) {  /* get the host info */
        herror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(PORT);    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                         sizeof(struct sockaddr)) == -1){
        
        perror("connect");
        exit(1);
    }
    
    sendMessage(get, sockfd);

    FILE *fp;
    fp = fopen(filename,"wb+");

    int numbytes = 1;
    char buf[MAXDATASIZE];
    int first = 1;
    
    while(1==1){
        memset (buf, '\0',MAXDATASIZE);

        if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
            perror("recv");
            return 1;
	    }
        if(numbytes==0)
            break;
        
        if(first){
            char *data = strstr( buf, "\r\n\r\n" )+4*sizeof(char);
            fwrite(data,sizeof(char),strlen(data),fp);
            first = 0;
        }
        else{
            fwrite(buf,sizeof(char),numbytes,fp);
        }
    }

    fclose(fp);
    close(sockfd);
}
