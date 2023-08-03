/* 
 * echoserver.c - A simple connection-based echo server 
 * usage: echoserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 128

#define PORTNO 1234

#if 0
/* 
 * Structs exported from netinet/in.h (for easy reference)
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  printf("starting echoserver\n");
  int listenfd; /* listening socket */
  int connfd; /* connection socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  portno = PORTNO;

  /* socket: create a socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /* build the server's internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; /* we are using the Internet */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
  serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

  /* bind: associate the listening socket with a port */
  if (bind(listenfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* listen: make it a listening socket ready to accept connection requests */
  if (listen(listenfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");

  /* main loop: wait for a connection request, echo input line, 
     then close connection. */
  clientlen = sizeof(clientaddr);
  
  printf("starting main loop\n");
  while (1) {

    /* accept: wait for a connection request */
    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (connfd < 0) 
      error("ERROR on accept");
    
    printf("starting connection loop\n");
    do {    
      bzero(buf, BUFSIZE);

      // Test multi packets using multiple reads
      n = read(connfd, buf, BUFSIZE);
      if (n < 0)
      {
        error("ERROR reading from socket");
        break;
      }
      printf("server read1 received %d bytes\n", n);
      
        printf("Server packet dump:\n");
        for(int i = 0; i < n; ++i)
        {
            printf("%02X ", buf[i]);
            if ((i+1) % 16 == 0)
                printf("\n");
        }
        printf("\n");

      write(connfd, buf, n);

      if (buf[3] != 0x61)
        continue;

      // Second packet (this loop)
      n = read(connfd, buf, BUFSIZE);
      if (n < 0)
      {
        error("ERROR reading from socket");
        break;
      }
      printf("server read2 received %d bytes\n", n);
      write(connfd, buf, n);

      // Create some branches for the fuzzer to find

      if (buf[0] == 0x77)
          printf("new path 0x77\n");

      if (buf[0] == 0x01)
          printf("new path 0x01\n");

      if (buf[3] == 0x22)
        printf("new path 0x22\n");

      if (*(uint16_t*)buf == 0x9876)
        printf("new path 0x9876\n");

      if (*(uint32_t*)buf == 0x12344321)
        printf("new path 0x12344321\n");

      if (buf[14] == 0x34)
      {
          printf("do crash\n");
          abort();
      }

        printf("server loop finished\n");
    } while (n > 0);
    printf("closing conn\n");
    close(connfd);
  }
}