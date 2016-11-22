// lurker client
// just listens for a string from the server and prints that string
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 8192

int main(int argc, char **argv) 
{
  //
  // Check the arguments for the host name and port number of 
  // an echo service.
  //
  if (argc != 3) {
    fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }
  
  //
  // Look up the host's name to get its IP address.
  //
  char *host = argv[1];
  int port = atoi(argv[2]);
  struct hostent *hp;
  if ((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr,"GETHOSTBYNAME failed.\n");
    exit(-1);
  }

  //
  // Request a socket and get its file descriptor.
  //
  int clientfd;
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"SOCKET creation failed.\n");
    exit(-1);
  }
    

  //
  // Fill in the host/port info into a (struct sockaddr_in).
  //
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  bcopy((char *) hp->h_addr_list[0], 
	(char *)&serveraddr.sin_addr.s_addr, 
	hp->h_length);
  serveraddr.sin_port = htons(port);

  //
  // Connect to the given host at the given port number.
  //
  if (connect(clientfd,
	      (struct sockaddr *)&serveraddr, 
	      sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr,"CONNECT failed.\n");
    exit(-1);
  }


  unsigned char *ip;
  ip = (unsigned char *)&serveraddr.sin_addr.s_addr;
  printf("Connected to solitaitre service at %d.%d.%d.%d. Waiting for server to start game.\n",
	 ip[0], ip[1], ip[2], ip[3]);

  // tell server we're a lurker
  write(clientfd,"LURKER",strlen("LURKER")+1);

  //
  // wait for string from server, print it
  //
  while (1) {
    
    char buffer[MAXLINE];

    // Read the server's response.
    int n = read(clientfd, buffer, MAXLINE);
    if (n == 0) {
      // No bytes received.  The server closed the connection.
      printf("Server closed the connection. Exiting.\n");
      break;
    }

    // be polite to the server
    write(clientfd,"THANKS",strlen("THANKS")+1);
    
    // Output the arena to the user.
    printf("%s",buffer);
  }

  //
  // Close the connection.
  //
  close(clientfd); 

  exit(0);
}
