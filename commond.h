#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <fcntl.h>  
#include <string.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <mysql/mysql.h>  
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define SUCESS 0
#define ERROR  -1
#define EXIT   SUCESS
#define USENAMESIZE 20
#define PASSWORDSIZE 20
#define PORTNUMBER  8888
#define PASSWORD  2
#define ID 0
#define USERNAME  1
#define FDS     4
#define STATUS  3
#define RWSIZE 1024
#define FILENAMESIZE 20
#define FILELISTSIZE 1025
#define PATHSIZE  100
#define localhost "127.0.0.1"
char voidStr[1]="";
char download[2]="D";
char update[2]="U";
char csend[2]="S";
typedef struct DataPackage
{
  char Cmd;
  int  Ack;
  char Sender[USENAMESIZE];
  char Password[PASSWORDSIZE];
  char Geter[USENAMESIZE];
  char FileName[FILENAMESIZE];
  char Buf[FILELISTSIZE];
  int  FileSize;
}DataPackage;

DataPackage Package(char cmd,int ack,char* sender,char* password,char* geter,char* filename,char* buf,int filesize)
{
  DataPackage pack;
  pack.Cmd = cmd;
  pack.Ack = ack;
  strncpy(pack.Sender,sender,strlen(sender));
  strncpy(pack.Password,password,strlen(password));
  strncpy(pack.Geter,geter,strlen(geter));
  strncpy(pack.FileName,filename,strlen(filename));
  strncpy(pack.Buf,buf,strlen(buf));
  pack.FileSize = filesize;
  return pack;
}