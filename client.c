/*文件传输客户端*/
#include "commond.h"
#include <signal.h>
using namespace std;

SSL_CTX *ctx;
SSL *ssl;
int sockfd;
struct sockaddr_in dest;
DataPackage dataPack;
DataPackage recievePack;
char serverIP[15];
bool LRE=true;
bool ISEXIT=false;
char user[USENAMESIZE]="\0";
char sender[USENAMESIZE]="\0";
char filebuf[FILELISTSIZE];
char path[PATHSIZE];
char filename[FILENAMESIZE]="\0";
pthread_mutex_t pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pthreadCond = PTHREAD_COND_INITIALIZER;
void freeResource()
{
 SSL_shutdown(ssl);
 /* 释放 SSL */
 SSL_free(ssl);
 close(sockfd);
}
/*连接至服务器*/
int connectToserver(char* serverIP)
{
	SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  ctx = SSL_CTX_new(SSLv23_client_method());
  if (ctx == NULL)
  {
    ERR_print_errors_fp(stdout);
    exit(1);
  }

  /* 创建一个 socket 用于 tcp 通信 */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Socket");
    exit(errno);
  }

  /* 初始化服务器端（对方）的地址和端口信息 */
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(PORTNUMBER);
  if(serverIP!=NULL)
  {
     if (inet_aton(serverIP, (struct in_addr *) &dest.sin_addr.s_addr) == 0)
     {
       perror(serverIP);
       exit(errno);
     }
  }else
  {
  	if (inet_aton(localhost, (struct in_addr *) &dest.sin_addr.s_addr) == 0)
     {
       perror(serverIP);
       exit(errno);
     }
  }

  /* 连接服务器 */
  if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0)
  {
    perror("Connect ");
    exit(errno);
  }
 
  /* 基于 ctx 产生一个新的 SSL */
  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, sockfd);
  /* 建立 SSL 连接 */
  if (SSL_connect(ssl) == -1)
  {
    ERR_print_errors_fp(stderr);
    return ERROR;
  }
  return SUCESS;
}
/*得到当前path路径下的目录文件名*/
int listFile(char *path)
{
  DIR *dirptr = NULL;
  struct dirent *entry;
  dirptr = opendir(path);
  if(dirptr == NULL)
  {
   printf("open dir error !\n");
   return ERROR;
  }
  while (entry = readdir(dirptr))
  {
    cout<<entry->d_name<<"\n";
  }
 closedir(dirptr);
 return SUCESS;	
}
/*列出接收到服务器端的文件列表*/
int listFileInSever()
{
  system("clear");
  cout<<"\n*********user file in server**********\n";
  cout<<recievePack.Buf<<endl;
}
/*下载文件*/
int downloadFile(char* filename)
{
	char filepath[PATHSIZE]="./";
	strcat(filepath,filename);
	FILE* fd=fopen(filepath,"wb");
	if(fd==NULL)
	{
		cout<<"download file error\n";
		return ERROR;
	}
	while(1)
	{
	  bzero(&recievePack.Buf,FILELISTSIZE);
	  SSL_read(ssl,&recievePack,sizeof(DataPackage));
	  if(recievePack.Ack==0)
	  fwrite(recievePack.Buf,sizeof(char),recievePack.FileSize,fd);
	  else
	  	break;
  }
  fclose(fd);
  return SUCESS;
}
int recieveFile(char* filename)
{
	char filepath[PATHSIZE]="./";
	strcat(filepath,filename);
	FILE* fd=fopen(filepath,"wb");
	if(fd==NULL)
	{
		cout<<"recieve error \n";
		return ERROR;
	}
	while(1)
	{
	  if(recievePack.Ack==5)
	  {
	    fwrite(recievePack.Buf,sizeof(char),recievePack.FileSize,fd);
	    
	  }  
	  else
	  	break;
	  bzero(&recievePack.Buf,FILELISTSIZE);
	  SSL_read(ssl,&recievePack,sizeof(DataPackage));
  }
  fclose(fd);
  return SUCESS;
}
/*从path得到最后的那个文件名*/
int getFilename(char* path,char* filename)
{
  int i=strlen(path);
  int j=i;
  while(path[i]!='/')
  {
   	i--;
  }	
  strncpy(filename,&path[i+1],j-i-1);
  return SUCESS;
}
/*发送文件的信息到服务器端*/
int sendFiledata(char *filename)
{
  int filesize=0;
  struct stat filestat;
  if(stat(path,&filestat)==-1)
  {
  	cout<<"send file data error\n";
  	return ERROR;
  }
  else
  	filesize=filestat.st_size;
  DataPackage datapack;
  datapack=Package('U',9,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  SSL_write(ssl,&datapack,sizeof(struct DataPackage));
  return SUCESS;
}
/*更新文件*/
int updateFile(char *filename)
{
  FILE* fd=fopen(path,"rb");
  if(fd==NULL)
  {
  	cout<<"update file error\n";
  	return ERROR;
  }
  DataPackage datapack;
  bzero(&datapack.Buf,FILELISTSIZE);
  int filesize=0;
  struct stat filestat;
  if(stat(path,&filestat)==-1)
  {
  	cout<<"update file error\n";
  	return ERROR;
  }
  else
  	filesize=filestat.st_size;
  int count=filesize;
  datapack=Package('U',9,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  while(fread(datapack.Buf,sizeof(char),1024,fd)!=0)
  {
    if(count/1024!=0)
    {
      datapack.FileSize=1024;
      count=count-1024;
    }
    else
    datapack.FileSize=count;
    SSL_write(ssl,&datapack,sizeof(struct DataPackage));	
    bzero(&datapack.Buf,FILELISTSIZE);
  }
  datapack=Package('U',4,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  SSL_write(ssl,&datapack,sizeof(struct DataPackage));	
  fclose(fd); 	
  return SUCESS;
}
int sendFile(char *filename)
{
  FILE* fd=fopen(path,"rb");
  if(fd==NULL)
  {
  	cout<<"send file error\n";
  	return ERROR;
  }
  DataPackage datapack;
  bzero(&datapack.Buf,FILELISTSIZE);
  int filesize=0;
  struct stat filestat;
  if(stat(path,&filestat)==-1)
  	return ERROR;
  else
  	filesize=filestat.st_size;
  int count=filesize;
  datapack=Package('S',5,user,voidStr,voidStr,filename,voidStr,filesize);
  while(fread(datapack.Buf,sizeof(char),1024,fd)!=0)
  {
    if(count/1024!=0)
    {
      datapack.FileSize=1024;
      count=count-1024;
    }
    else
    datapack.FileSize=count;
    SSL_write(ssl,&datapack,sizeof(struct DataPackage));	
    bzero(&datapack.Buf,FILELISTSIZE);
  }
  datapack=Package('S',6,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  SSL_write(ssl,&datapack,sizeof(struct DataPackage));	
  fclose(fd); 	
  return SUCESS;
}
/*接收处理函数*/
void* Recieve(void* arg)
{
  while(ISEXIT==false)
  {
    bzero(&recievePack,sizeof(DataPackage));
    SSL_read(ssl,&recievePack,sizeof(DataPackage));	
    switch(recievePack.Cmd)
    {
      case 'L':switch(recievePack.Ack)
      	       {
      	       	 case 0:cout<<"login sucess!"<<endl;
      	       	 	       LRE=false;
      	       	 	       break;
      	       	 case 1:cout<<"username or password incorrect!"<<endl;break;
      	       	 case 2:cout<<"sorry,beyond connect number,try later!"<<endl;freeResource();exit(0);break;
      	       	 case 3:cout<<"sorry,you are online\n";freeResource();exit(0);break;
      	       };
      	       break;	
      case 'R':switch(recievePack.Ack)
      	       {
      	         case 0: cout<<"register sucess!"<<endl;
      	       	 	       LRE=false;
      	       	 	       break;
      	       	 case 1: cout<<"register name exit!"<<endl;break;
      	       };
      	       break; 
      case 'D':switch(recievePack.Ack)
      	       {
      	         case 0:cout<<"bedin download!\n"<<"File Size is "<<recievePack.FileSize<<"b\n"; 	      
      	         	      downloadFile(recievePack.FileName);
      	         	      break;
      	         case 1: cout<<"download sucess!"<<endl;break;
      	         case 2: cout<<"download file error!"<<endl;break; 	 	      
      	       };
      	       break;
      case 'U':switch(recievePack.Ack)
      	       {
      	       	case 0:cout<<"bedin update!\n";
      	       		     char filename[FILENAMESIZE];
    	       	         getFilename(path,filename);
    	       	         updateFile(filename);break;
      	        case 1:cout<<"update finish\n";break;
      	        case 2:cout<<"update error\n";break;
      	       };
      	       break;
      case 'C':switch(recievePack.Ack)
      	       {
      	       	case 0:cout<<"send sucess\n";break;
      	       	case 1:cout<<"not online\n";break;
      	       	case 2:cout<<recievePack.Sender<<" say:"<<recievePack.Buf<<endl;
      	       		     break;
      	       };
      	       break; 
      case 'S':switch(recievePack.Ack)
      	       {
      	        case 4:dataPack.Ack=4;SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
      	        	     sendFile(recievePack.FileName);
      	        	     break;
      	        case 1:cout<<recievePack.Geter<<" Not Online\n";break;
      	        case 2:cout<<recievePack.Sender<<" send you file "<<recievePack.FileName<<endl;strncpy(sender,recievePack.Sender,strlen(recievePack.Sender)); 
      	        	     strncpy(filename,recievePack.FileName,strlen(recievePack.FileName));  	
      	        	     break;
      	        case 3:cout<<recievePack.Geter<<" Not recieve\n";break;
      	        case 5:cout<<"begin to recieve "<<recievePack.FileName<<endl;;
      	        	     recieveFile(recievePack.FileName);    
      	        	     break;
      	        case 6:cout<<"send file sucess\n";break;
      	        case 7:cout<<"recieve file sucess\n";break;
      	       };
      	       break;
      case 'F':listFileInSever();break;
      case 'Q':cout<<"exit sucess!"<<endl;ISEXIT==true;pthread_exit((void*)0);break;
    }
  }	
}
void threadWatiCond()
{
  pthread_mutex_lock(&pthreadMutex);
  pthread_cond_wait(&pthreadCond,&pthreadMutex);
  pthread_mutex_unlock(&pthreadMutex);	
}
/*主要菜单*/
void* mainMenu(void* arg)
{
   int choice;
   char un[USENAMESIZE];
   char *up;
 while(LRE)
 {
   cout<<"*******1.Login in******"<<endl;	
   cout<<"*******2.Register User*******"<<endl;
	 cout<<"*******3.EXIT*******"<<endl;
	 cin>>choice;
	 switch(choice)
	 {
	 case 1:cout<<"Login Name:";
	        cin>>un;
	        up=getpass("Password:"); 
	 	      dataPack=Package('L',9,un,up,voidStr,voidStr,voidStr,0);
	  	    SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
	  	    sleep(1);
	  	    break;
	 case 2:cout<<"New User Name:";
	 	      cin>>un;
	 	      up=getpass("New Password:");
	 	      dataPack=Package('R',9,un,up,voidStr,voidStr,voidStr,0);
	 	      SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
	 	      sleep(1);
	 	      break;
	 case 3:
	 	      dataPack=Package('Q',0,voidStr,voidStr,voidStr,voidStr,voidStr,0);
	 	      SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
	 	      ISEXIT==true;
	 	      pthread_exit((void*)0);
	 	      break;
	 }
 }
 strncpy(user,un,strlen(un));
while(1)
{
  cout<<"\n******0.Send File to Others*************\n";
  cout<<"******1.Dispose File From Others*************\n";
  cout<<"******2.Chat with Others*************\n";
  cout<<"******3.Update file in server*************\n";
  cout<<"******4.Download file in server*************\n";
  cout<<"******5.Show file in local********\n";
  cout<<"******6.Show file in server********\n";
  cout<<"******7.EXIT********************************\n";
  cin>>choice;
  system("clear");
  switch(choice)
  {
    case 0:cout<<"1.input geter\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	       case 1:char geter[USENAMESIZE];
    	       	      cout<<"geter:";cin>>geter;
    	       	      cout<<"input path:";cin>>path;
    	       	      getFilename(path,dataPack.FileName);
    	       	      dataPack=Package('S',9,un,up,geter,dataPack.FileName,voidStr,0);
    	       	      SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
    	       	      break;	
    	       case 2:break;
    	     };
    	     break;
    case 1:cout<<"1.recieve\n";
      	   cout<<"2.ignore\n";
      	   cout<<"3.return\n";
      	   int choice;
      	   cin>>choice;
      	   switch(choice)
      	    {
      	     case 1:dataPack=Package('S',4,sender,voidStr,user,filename,voidStr,0);
      	     	      cout<<recievePack.Sender<<endl;
      	        	  SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
      	        	  break;
      	     case 2:dataPack=Package('S',3,sender,voidStr,user,recievePack.FileName,voidStr,0);
      	        	  SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
      	        	  break;
      	     case 3:break;
      	    };
      	    break;
    case 2:cout<<"1.input geter\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	       case 1:
    	       	      char geter[USENAMESIZE];
    	       	      cout<<"geter name:";
    	       	      cin>>geter;
    	       	      cout<<"context:";
    	       	      cin>>dataPack.Buf;    
    	       	      dataPack=Package('C',9,un,up,geter,voidStr,dataPack.Buf,0);
    	              SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
    	              sleep(1);
    	              break;
    	       case 2:break;
    	     };
    	     break;
    case 3:cout<<"1.input update file name\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	       case 1:cout<<"file name:";
    	       	      cin>>path;
    	       	      getFilename(path,filename);
    	       	      sendFiledata(filename);
    	              sleep(1);
    	              break;
    	       case 2:break;
    	     };
    	     break;
    case 4:cout<<"1.input download file name\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	     case 1:
    	     	      dataPack=Package('F',0,un,up,voidStr,voidStr,voidStr,0);
    	            SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
    	            sleep(1);
      	          cout<<"file name:";
      	          cin>>filename;
      	          dataPack=Package('D',9,un,up,voidStr,filename,voidStr,0);
    	            SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
    	            sleep(1);
    	            break;
    	     case 2:break;
    	     };
    	     break;
    case 5:cout<<"1.input path\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	      case 1:cout<<"input path:";
    	            cin>>path;
    	            system("clear");
    	            cout<<"\n*********user file in "<<path;
    	            cout<<"**********\n";
      	          listFile(path);    
    	            break;
    	      case 2:break;
    	     };
    	     break;
    case 6:cout<<"1.show file\n";
    	     cout<<"2.return\n";
    	     cin>>choice;
    	     switch(choice)
    	     {
    	     case 1:dataPack=Package('F',0,un,up,voidStr,voidStr,voidStr,0);
    	            SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
    	            sleep(1);
    	            break;
    	     case 2:break;
    	     };
    	     break;
    case 7:dataPack=Package('Q',0,un,up,voidStr,voidStr,voidStr,0);
	 	       SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
	 	       ISEXIT==true;
	 	       pthread_exit((void*)0);
	 	       break;
  }
}
}
/*中断信号的抓获*/
void my_exit(int sign_no)
{
	if(sign_no==SIGINT)
	{
    dataPack=Package('Q',1,voidStr,voidStr,voidStr,voidStr,voidStr,0);
    SSL_write(ssl,&dataPack,sizeof(struct DataPackage));
	 	ISEXIT==true;
	 	cout<<"\ninterupt exit sucess!"<<endl;
	 	exit(1);	
	}
}

int main(int argc,char**argv)
{
 /*获取中断信号 如果参数1为NULL，则默认本地连接*/
 signal(SIGINT, my_exit);
 if(argv[1]!=NULL)
  connectToserver(argv[1]);
 else
 	connectToserver(NULL);
 pthread_t mainmenu,recieve;
 int ret=pthread_create(&mainmenu,NULL,mainMenu,NULL);
 if(ret!=0)
	{
	   cout<<"creat mainmenu thread error"<<endl;
	   return ERROR;
	}
 ret=pthread_create(&recieve,NULL,Recieve,NULL);
 if(ret!=0)
	{
	   cout<<"creat recieve thread error"<<endl;
	   return ERROR;
	}
 pthread_join(mainmenu,NULL);
 pthread_join(recieve,NULL);
 freeResource();
 return SUCESS;	
}