#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int min(int a, int b);

long getFileSize(FILE* pFile){
  fseek(pFile, 0, SEEK_END);
  return ftell(pFile);
}

int getBuffer(char ** buffer, int bufferSize, FILE* pFile, int fileSize, int totalBytesSent){
  int actBufferSize = min(fileSize - totalBytesSent, bufferSize);
  fseek(pFile, totalBytesSent, SEEK_SET);
  fread(*buffer, sizeof(char), actBufferSize, pFile);
  return actBufferSize;
}

int min(int a, int b){
  if (a > b) return b;
  return a;
}

int main(int argc, char *argv[])
{
  if (argc != 5){
    printf("usage: %s file host port buffSize\n", argv[0]);
    return -1;
  }

  char* filePath = argv[1];
  FILE* pFile = fopen(filePath, "rb");
  if (!pFile) {
    printf("error: can't find the file\n");
    return -1;
  }
  char *host = argv[2];
  int port = atoi(argv[3]);

  int bufferSize = atoi(argv[4]);
  char* buffer = malloc(bufferSize);

  long fileSize =getFileSize(pFile);

  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  Res = inet_pton(AF_INET, host, &stSockAddr.sin_addr);

  if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
  else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

  if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

  int sendFileSize = send(SocketFD, &fileSize, sizeof(fileSize),0);

  int totalBytesSent = 0;

  printf("Trying to send %ld Bytes to %s:%d...\n", fileSize, host, port);

  while(totalBytesSent < fileSize){

    int actBufferSize = getBuffer(&buffer, bufferSize, pFile, fileSize, totalBytesSent);
    int actBytesSent = send(SocketFD, buffer, actBufferSize, 0);

    totalBytesSent+=actBytesSent;

    printf("Bytes sent: %d, Total Bytes sent: %d\n", actBytesSent, totalBytesSent);
  }

  fclose(pFile);

  printf("DONE!!\n");

  shutdown(SocketFD, SHUT_RDWR);
  close(SocketFD);
  return 0;
}
