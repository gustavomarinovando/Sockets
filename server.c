#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]){

  if (argc != 4){
    printf("usage: %s fileName port bufferSize\n", argv[0]);
    return -1;
  }

  // Nombre del archivo a recibir
  char* fileName = argv[1];

  // Numero de puerto
  int port = atoi(argv[2]);

  // Tamaño del buffer
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
  int fsize = recv(ConnectFD, &fileSize, sizeof(fileSize), 0);
  if(fsize != sizeof(fileSize)) {
    printf("Error reading file size\n");
    exit(-1);
  }
  printf("Trying to receive %ld Bytes...\n", fileSize);

  int totalBytesReceived = 0;

  FILE* pFile = fopen(fileName, "w+");

  // here ADD your code
  int per, byt_rcv = 0, total_byt = 0, byps = 0;
  float seconds, total_time, speed;
  clock_t start = clock();
  while (total_byt < fileSize) {
    byt_rcv = recv(ConnectFD, buffer, bufferSize, 0);
    clock_t end = clock();
    fwrite(buffer, sizeof(char), byt_rcv, pFile);
    total_byt += byt_rcv;
    byps += byt_rcv;
    per = ((float)((total_byt * 100) / fileSize));
    seconds = ((float)(end - start) / CLOCKS_PER_SEC);
    total_time += seconds;
    if (total_time >= 1) {
      speed = (byps / 1024) / total_time;
      printf("Elapsed time %.4f s\tVelocidad %.3f KB/s\tbytes %d\n", total_time, speed, total_byt);
      printf("Expected: %d\tRead: %d\tPorcentaje %d %%\n", bufferSize, byt_rcv, per);
      total_time = 0;
      byps = 0;
    }
  }

  fclose(pFile);

  printf("%s written\nDONE\n", fileName);

  shutdown(ConnectFD, SHUT_RDWR);

  close(ConnectFD);
  close(SocketFD);

  return 0;
}
