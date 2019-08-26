#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[]){

  if (argc != 4){
    printf("usage: %s fileName port bufferSize\n", argv[0]);
    return -1;
  }

  char* fileName = argv[1];

  int port = atoi(argv[2]);
  int bufferSize = atoi(argv[3]);
  char* buffer = malloc(bufferSize);

  struct sockaddr_in stSockAddr;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

  if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

  int ConnectFD = accept(SocketFD, NULL, NULL);

  if(0 > ConnectFD)
    {
      perror("error accept failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

  long fileSize;
  int fsize = recv(ConnectFD, &fileSize, sizeof(fileSize),0);
  if(fsize != sizeof(fileSize)) {
    printf("Error reading file size\n");
    exit(-1);
  }
  printf("Trying to receive %ld Bytes...\n", fileSize);

  int totalBytesReceived = 0;

  FILE* pFile = fopen(fileName, "wb");

  // here ADD your code

  fclose(pFile);

  printf("%s written\nDONE\n", fileName);

  shutdown(ConnectFD, SHUT_RDWR);

  close(ConnectFD);
  close(SocketFD);

  return 0;
}
