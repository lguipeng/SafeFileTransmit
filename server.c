/*文件传输服务器端*/
#include "commond.h"
#define ERRORSIZE 2

#define LISTENNUMBER 5
#define CLIENTSIZE 10
#define THREADNUM 10   /*线程池中线程数量*/
#define LOGINLOGPATH  "./loginlog"
#define DATALOGPATH "./datalog"
using namespace std;
int sockfd,newsockfd,tempsockfd;
MYSQL my_connection;
SSL_CTX *ctx;
socklen_t sin_size;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
pthread_mutex_t pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pthreadCond = PTHREAD_COND_INITIALIZER;
char USER[RWSIZE];
char ADM[RWSIZE];
char UN[CLIENTSIZE][USENAMESIZE];
char UP[CLIENTSIZE][PASSWORDSIZE];
int  TOTALCLIENT=0;
int  CURRENTCLIENT=0;
char clientIP[15];
SSL* clientFD[CLIENTSIZE];
int ISEXIT;
/*对数据库的操作
0.检查管理员的用户名和密码是否一致
1.检查用户的用户名和密码是否一致
2.检查用户是否在线
3.对用户的请求注册进行加入数据库操作
4.当用户上线时更新状态为T
5.当用户下线时更新状态为F
6.记录用户登录时是第几个，当文件在用户和用户之间传输时可以用到
7.增加管理员
8.显示当前在线用户
*/
int OptMysql(char *username,char *password,int opt)
{
  int  res;             /*执行查询操作后的返回标志*/  
  int  i,j,k;  
  char checkinfo[100]="\0";
  MYSQL_RES *res_ptr;   /*指向检索的结果存放地址的指针*/  
  MYSQL_ROW sqlrow;     /*返回的记录信息*/  
  if (mysql_real_connect(&my_connection, "localhost", "lgp", "lgp123","user",0,NULL,CLIENT_FOUND_ROWS))  
  {  
  	switch(opt)
  	{
     case 0: strcat(checkinfo,"select *");
  	         strcat(checkinfo," from Adm where username='");   
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("SELECT error:%s\n",mysql_error(&my_connection));
             goto error;    
            }  
            else  
            {  
              res_ptr=mysql_store_result(&my_connection);  
              if(res_ptr)  
             {    
              j=(int)mysql_num_rows(res_ptr);
              if(j==1)
              {
                sqlrow=mysql_fetch_row(res_ptr);
                if(strncmp(password,sqlrow[PASSWORD],strlen(sqlrow[PASSWORD])))
              	   goto error;  
              }
              else
              {
                 goto error; 
              } 
             }  
//            mysql_free_result(res_ptr);  
             } 
             break;
     case 1: strcat(checkinfo,"select *");
  	         strcat(checkinfo," from Client where username='");   
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("SELECT error:%s\n",mysql_error(&my_connection));
             goto error;    
            }  
            else  
            {  
              res_ptr=mysql_store_result(&my_connection);  
              if(res_ptr)  
             {    
              j=(int)mysql_num_rows(res_ptr);
              if(j==1)
              {
                sqlrow=mysql_fetch_row(res_ptr);
                if(strncmp(password,sqlrow[PASSWORD],strlen(sqlrow[PASSWORD])))
              	   goto error;  
              }
              else
              {
                goto error; 	
              } 
             } 
//             mysql_free_result(res_ptr);  
            } 
            break;
     case 2: strcat(checkinfo,"select *");
  	         strcat(checkinfo," from Client where username='");   
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("SELECT error:%s\n",mysql_error(&my_connection));
             goto error;    
            }  
            else  
            {  
              res_ptr=mysql_store_result(&my_connection);  
              if(res_ptr)  
             {    
              j=(int)mysql_num_rows(res_ptr);
              if(j==1)
              {
                sqlrow=mysql_fetch_row(res_ptr);
                if(strncmp("T",sqlrow[STATUS],strlen(sqlrow[STATUS])))
              	   goto error;  
              }
              else
              {
                goto error; 
              } 
             }  
//             mysql_free_result(res_ptr);  
            } 
            break;
     case 3: strcat(checkinfo,"insert into ");
  	         strcat(checkinfo,"Client values('','");   
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"','");
  	         strcat(checkinfo,password);
  	         strcat(checkinfo,"','T','')");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("INSERT error:%s\n",mysql_error(&my_connection)); 
             goto error;   
            }  
//            mysql_free_result(res_ptr);   
            break;
     case 4: strcat(checkinfo,"update Client set status='T' where username='");
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("INSERT error:%s\n",mysql_error(&my_connection)); 
             goto error;   
            }  
//            mysql_free_result(res_ptr);   
            break;
     case 5: strcat(checkinfo,"update Client set status='F' where username='");
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("INSERT error:%s\n",mysql_error(&my_connection)); 
             goto error;   
            }  
//            mysql_free_result(res_ptr);   
            break;
     case 6: strcat(checkinfo,"update Client set Fd=");
     	       char number[10];
     	       sprintf(number,"%d",CURRENTCLIENT);
     	       strcat(checkinfo,number);
     	       strcat(checkinfo," where username='");
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"'");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("update error:%s\n",mysql_error(&my_connection)); 
             goto error;   
            }  
//            mysql_free_result(res_ptr);   
            break;
     case 7: strcat(checkinfo,"insert into ");
  	         strcat(checkinfo,"Adm values('','");   
  	         strcat(checkinfo,username);
  	         strcat(checkinfo,"','");
  	         strcat(checkinfo,password);
  	         strcat(checkinfo,"')");
  	         cout<<checkinfo<<endl;
             res = mysql_query(&my_connection, checkinfo);  
            if (res)  
            {  																					   
             printf("INSERT error:%s\n",mysql_error(&my_connection)); 
             goto error;   
            }  
//          mysql_free_result(res_ptr);   
            break;
     case 8:strcat(checkinfo,"select * from Client where status='T'");
            res = mysql_query(&my_connection, checkinfo); 
            if (res)  
            {  																					   
             printf("SELECT error:%s\n",mysql_error(&my_connection));
             goto error;    
            }  
            else  
            {  
              res_ptr=mysql_store_result(&my_connection);  
              if(res_ptr)  
             {    
              MYSQL_ROW sqlrow;
              while((sqlrow=mysql_fetch_row(res_ptr)))  
              {  
                  cout<<sqlrow[USERNAME]<<"\t";
              }
              cout<<endl;  
             }  
//             mysql_free_result(res_ptr);  
            } 
            break;
     } 
//  mysql_close(&my_connection);  
  }   
  else  
 {  
    fprintf(stderr, "Connection failed\n");  
  
    if (mysql_errno(&my_connection))  
    {  
        fprintf(stderr, "Connection error %d: %s\n",  
        mysql_errno(&my_connection),  
        mysql_error(&my_connection));  
    }  
 }   
    return SUCESS;
error: 	
//mysql_close(&my_connection); 
	return ERROR;
} 
/*检查用户是否重名*/ 
int CheckRename(char* username)
{
	int  res;             /*执行查询操作后的返回标志*/  
  int  j;  
  char checkinfo[100]="\0";
  MYSQL_RES *res_ptr;   /*指向检索的结果存放地址的指针*/  
  MYSQL_ROW sqlrow;     /*返回的记录信息*/  
  if (mysql_real_connect(&my_connection, "localhost", "lgp", "lgp123","user",0,NULL,CLIENT_FOUND_ROWS))  
  {  
  	strcat(checkinfo,"select *");
  	strcat(checkinfo," from Client where username='");   
  	strcat(checkinfo,username);
  	strcat(checkinfo,"'");
    res = mysql_query(&my_connection, checkinfo);  
    if (res)  
    {  																					   
       printf("SELECT error:%s\n",mysql_error(&my_connection));
       return ERROR;   
    }  
    else  
    {  
       res_ptr=mysql_store_result(&my_connection);  
       if(res_ptr)  
       {    
        j=(int)mysql_num_rows(res_ptr);
        if(j==0)
        {
           return SUCESS; 
        }
        else 
        {
          return ERROR; 
        } 
       }  
       mysql_free_result(res_ptr);  
    } 
  }
  else
  {
  	fprintf(stderr, "Connection failed\n");    
    if (mysql_errno(&my_connection))  
    {  
        fprintf(stderr, "Connection error %d: %s\n",  
        mysql_errno(&my_connection),  
        mysql_error(&my_connection));  
    }  
  }
}
int getFdNum(char* username)
{
	int  res;             /*执行查询操作后的返回标志*/  
  int  j;  
  char checkinfo[100]="\0";
  MYSQL_RES *res_ptr;   /*指向检索的结果存放地址的指针*/  
  MYSQL_ROW sqlrow;     /*返回的记录信息*/  
  if (mysql_real_connect(&my_connection, "localhost", "lgp", "lgp123","user",0,NULL,CLIENT_FOUND_ROWS))  
  {  
  	strcat(checkinfo,"select *");
  	strcat(checkinfo," from Client where username='");   
  	strcat(checkinfo,username);
  	strcat(checkinfo,"'");
    res = mysql_query(&my_connection, checkinfo);  
    if (res)  
    {  																					   
       printf("SELECT error:%s\n",mysql_error(&my_connection));
       return ERROR;   
    }  
    else  
    {  
       res_ptr=mysql_store_result(&my_connection);  
       if(res_ptr)  
       {    
         MYSQL_ROW sqlrow;
         while((sqlrow=mysql_fetch_row(res_ptr)))  
         {  
           return  atoi(sqlrow[FDS]);
         }
       }  
       mysql_free_result(res_ptr);  
    } 
  }
  else
  {
  	fprintf(stderr, "Connection getFdNum failed\n");    
    if (mysql_errno(&my_connection))  
    {  
        fprintf(stderr, "Connection error %d: %s\n",  
        mysql_errno(&my_connection),  
        mysql_error(&my_connection));  
    }  
  }
}
/*建立网络连接*/
int SetupTcp()
{
  	int on=1;
  	// AF_INET:IPV4;SOCK_STREAM:TCP
  	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) 
    {
      fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
      return ERROR;
    }
    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(PORTNUMBER);
    if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("setsockopt failed");  
        return ERROR; 
    }
    if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
    {
       fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
       return ERROR; 
    }  
    if(listen(sockfd,LISTENNUMBER)==-1)
    {
        fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
        return ERROR; 
    }
    
    return SUCESS;
}
/*写登录的log*/
int WriteLoginLog(char *user,char *clientip)
{
  int logfd;
  char logdata[USENAMESIZE+50]="\0";   //15是ip地址的长度 ,加上换行回车
  logfd = open(LOGINLOGPATH,O_RDWR|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR);
  if(logfd == -1)
     return ERROR;
  time_t timep = time(NULL);
  strcat(logdata,user);
  strcat(logdata,"\t");
  strcat(logdata,clientip);
  strcat(logdata,"\t");
  strcat(logdata,ctime(&timep));
  logdata[strlen(logdata)-1]='\0';
	logdata[strlen(logdata)]='\0';
  strcat(logdata,"\n");	
  if(write(logfd,logdata,strlen(logdata))<0)
  	return ERROR;
  close(logfd);
  return SUCESS;
}
/*写文件上传下载发送的log*/
int WriteDataLog(char* sender,char* getor,char*filename,char* opt)
{
	int  logfd;
	char logdata[USENAMESIZE*2+FILENAMESIZE+29+1]="\0"; //ctime获取的时间为25个加上4间隔符
	time_t timep=time(NULL);
	logfd=open(DATALOGPATH,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
	if(logfd==-1)
	return ERROR;
	strcat(logdata,sender);
	strcat(logdata,",");
	strcat(logdata,getor);
	strcat(logdata,",");
	strcat(logdata,filename);
	strcat(logdata,",");
	strcat(logdata,ctime(&timep));
	logdata[strlen(logdata)-1]='\0';
	logdata[strlen(logdata)]='\0';
	strcat(logdata,",");
	strcat(logdata,opt);
	strcat(logdata,"\n");
	if(write(logfd,logdata,strlen(logdata))<0)
  	return ERROR;
  close(logfd);
  return SUCESS; 	
}
/*下载文件*/
int downloadFile(char* username,char* filename,SSL* userfd)
{
  char path[PATHSIZE]="./";
  strcat(path,username);
  strcat(path,"/");
  strcat(path,filename);
  FILE* fd=fopen(path,"rb");
  if(fd==NULL)
  	return ERROR;
  int filesize=0;
  struct stat filestat;
  if(stat(path,&filestat)==-1)
  	return ERROR;
  else
  	filesize=filestat.st_size;
  DataPackage datapack;
  datapack=Package('D',0,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  SSL_write(userfd,&datapack,sizeof(struct DataPackage));
  bzero(&datapack.Buf,FILELISTSIZE);
  int count=filesize;
  while(fread(datapack.Buf,sizeof(char),1024,fd)!=0)
  {
    if(count/1024!=0)
    {
      datapack.FileSize=1024;
      count=count-1024;
    }
    else
    	datapack.FileSize=count;
    SSL_write(userfd,&datapack,sizeof(struct DataPackage));	
    bzero(&datapack.Buf,FILELISTSIZE);
  }
  datapack=Package('D',1,voidStr,voidStr,voidStr,filename,voidStr,filesize);
  SSL_write(userfd,&datapack,sizeof(struct DataPackage));	
  fclose(fd); 	
  return SUCESS;
}
/*上传文件*/
int updateFile(char* username,char* filename,SSL* ssl)
{
	char path[PATHSIZE]="./";
	strcat(path,username);
	strcat(path,"/");
	strcat(path,filename);
	FILE* fd=fopen(path,"wb");
	if(fd==NULL)
		return ERROR;
	DataPackage recievePack;
	while(1)
	{
	  bzero(&recievePack.Buf,FILELISTSIZE);
	  SSL_read(ssl,&recievePack,sizeof(DataPackage));
	  if(recievePack.Ack==9)
	  fwrite(recievePack.Buf,sizeof(char),recievePack.FileSize,fd);
	  else
	  	break;
  }
  recievePack=Package('U',1,voidStr,voidStr,voidStr,filename,voidStr,0);
  SSL_write(ssl,&recievePack,sizeof(struct DataPackage));	
  fclose(fd);
  return SUCESS;
}
/*得到用户所在文件夹文件列表*/
int getList(char *username,char buf[])
{
  DIR *dirptr = NULL;
  struct dirent *entry;
  char dir[PATHSIZE]="./";
  strcat(dir,username);
  dirptr = opendir(dir);
  if(dirptr == NULL)
  {
   printf("open dir error !\n");
   return ERROR;
  }
  int i=0;
  while (entry = readdir(dirptr))
  {
    i++;
    strcat(buf,entry->d_name);
    strcat(buf,"\t");
    if(i==5)
    {
    	strcat(buf,"\n");
    	i=0;
    }
  }
 closedir(dirptr);
 return SUCESS;
}
/*为刚注册的用户建立相应的文件夹*/
int makeDir(char *username)
{
	char path[PATHSIZE]="./";
	strcat(path,username);
  if(mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR)==SUCESS)
  	return SUCESS;
  else
  	return ERROR;
}
/*主要线程处理函数，对用户的请求进行处理，等待连接线程发送信号，然后往下执行*/
void* handler(void* arg)
{
 head:pthread_mutex_lock(&pthreadMutex);
  	  pthread_cond_wait(&pthreadCond,&pthreadMutex);
  	  pthread_mutex_unlock(&pthreadMutex);
  	  char user[USENAMESIZE]="\0";
  	  int new_fd=tempsockfd;
  	  SSL *ssl;
  	  ssl = SSL_new(ctx);
     /* 将连接用户的 socket 加入到 SSL */
      SSL_set_fd(ssl, new_fd);	
     if(SSL_accept(ssl) == -1)
     {
       perror("Accept");
       close(new_fd);
     }
    SSL *NewFd=ssl;   
    DataPackage Datapack;
registers:
	  SSL_read(NewFd,&Datapack,sizeof(DataPackage));
    if(Datapack.Cmd=='L')
    {	
    	if(OptMysql(Datapack.Sender,Datapack.Password,1)==ERROR||OptMysql(Datapack.Sender,Datapack.Password,2)==SUCESS)
    	{
    	 if(OptMysql(Datapack.Sender,Datapack.Password,1)==ERROR)
    	 {
    	  Datapack=Package('L',1,voidStr,voidStr,voidStr,voidStr,voidStr,0);
    	  SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
        goto registers;	
       }
       else
       {
        Datapack=Package('L',3,voidStr,voidStr,voidStr,voidStr,voidStr,0);
    	  SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	  SSL_shutdown(NewFd);
        /* 释放 SSL */
        SSL_free(NewFd);
        close(new_fd);	
        goto head;
        goto head;	 	
       }	
    	} else
    	{
		  clientFD[CURRENTCLIENT]=NewFd;
		  strncpy(user,Datapack.Sender,strlen(Datapack.Sender));
		  OptMysql(user,Datapack.Password,4);
		  OptMysql(user,Datapack.Password,6);
		  CURRENTCLIENT++;
		  Datapack=Package('L',0,voidStr,voidStr,voidStr,voidStr,voidStr,0);
		  SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
		  Datapack.Cmd='F';
		  getList(user,Datapack.Buf);
		  SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
		  WriteLoginLog(user,clientIP);	
    	}  		 
    }
   if(Datapack.Cmd=='R')
   {
     if(Datapack.Ack==9)
     {
    	if(CheckRename(Datapack.Sender)==SUCESS)
      {
    	  clientFD[CURRENTCLIENT]=NewFd;
    	  CURRENTCLIENT++;
    	  OptMysql(Datapack.Sender,Datapack.Password,3);
    	  makeDir(Datapack.Sender);
    	  strncpy(user,Datapack.Sender,strlen(Datapack.Sender));
    	  Datapack=Package('R',0,voidStr,voidStr,voidStr,voidStr,voidStr,0);
       	  SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
      }else
      {
       	bzero(&Datapack,sizeof(DataPackage));
       	Datapack=Package('R',1,voidStr,voidStr,voidStr,voidStr,voidStr,0);
       	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
       	goto registers;
      }
     }
   }
   if(Datapack.Cmd=='Q')
   {
        bzero(&Datapack,sizeof(DataPackage));
        Datapack=Package('Q',0,voidStr,voidStr,voidStr,voidStr,voidStr,0);
        SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
        
        SSL_shutdown(NewFd);
        /* 释放 SSL */
        SSL_free(NewFd);
        close(new_fd);	
        goto head;
   }
    while(1)
    {
      SSL_read(NewFd,&Datapack,sizeof(DataPackage));	
    	if(Datapack.Cmd=='Q')
    	{
        CURRENTCLIENT--;
        OptMysql(user,Datapack.Password,5);
        if(Datapack.Ack==0)
        	cout<<user<<" exit normal\n";
        else
        	cout<<user<<" exit abnormal\n";
        bzero(&Datapack,sizeof(DataPackage));
        Datapack=Package('Q',0,voidStr,voidStr,voidStr,voidStr,voidStr,0);
        SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
        
        SSL_shutdown(NewFd);
        /* 释放 SSL */
        SSL_free(NewFd);
        close(new_fd);	
        goto head;
    	}
    	if(Datapack.Cmd=='F')
    	{
    		bzero(&Datapack,sizeof(DataPackage));
    		Datapack.Cmd='F';
        getList(user,Datapack.Buf);
        SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	}
    	if(Datapack.Cmd=='D')
    	{
    	  if(Datapack.Ack==9)
    	  {
    	  	if(downloadFile(user,Datapack.FileName,NewFd)==SUCESS)
    	  	{
    	  	 WriteDataLog(user,voidStr,Datapack.FileName,download);
    	  	 Datapack=Package('D',1,voidStr,voidStr,voidStr,voidStr,voidStr,0); 
    	  	 SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));	
    	  	}
    	  	else
    	  	{
    	  	 Datapack=Package('D',2,voidStr,voidStr,voidStr,voidStr,voidStr,0); 
    	  	 SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));		
    	  	}
    	  }	
    	}
    	if(Datapack.Cmd=='U')
    	{
    	 if(Datapack.Ack==9) 	
    	 {
    	 	Datapack=Package('U',0,voidStr,voidStr,voidStr,Datapack.FileName,voidStr,0); 
    	 	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));	
    	 	if(updateFile(user,Datapack.FileName,NewFd)==SUCESS)
    	 	{
    	 		WriteDataLog(user,voidStr,Datapack.FileName,update);
    	  	Datapack=Package('U',1,voidStr,voidStr,voidStr,Datapack.FileName,voidStr,0); 
    	  	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));	
    	 	}
    	 	else
    	 	{
    	 	 Datapack=Package('U',2,voidStr,voidStr,voidStr,Datapack.FileName,voidStr,0); 
    	   SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));		
    	 	}
    	 }
    	}
    	if(Datapack.Cmd=='C')
    	{
    	  if(Datapack.Ack==9)
    	  {
    	   if(OptMysql(Datapack.Geter,voidStr,2)==SUCESS)
    	   {
    	     char geter[USENAMESIZE];
    	     strncpy(geter,Datapack.Geter,strlen(Datapack.Geter));
    	     int fdnum=getFdNum(geter);
    	     if(fdnum!=ERROR)
    	     {
    	      Datapack.Ack=2;
    	      SSL_write(clientFD[fdnum],&Datapack,sizeof(struct DataPackage));
    	      Datapack.Ack=0;	
    	      SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	     }else
    	     {
    	     	Datapack.Ack=1;	 
    	     	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	     }
    	   }else
    	   {
    	    Datapack.Ack=1;	 
    	    SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));	
    	   }
    	 } 	
    	}
    	if(Datapack.Cmd=='S')
    	{
    		char sender[USENAMESIZE];
    		int fdnum,fdnum_send;
    		char geter[USENAMESIZE];
    		if(Datapack.Ack==9)
    		{
    		  if(OptMysql(Datapack.Geter,voidStr,2)==SUCESS)
    		  {
    		   
    	     strncpy(geter,Datapack.Geter,strlen(Datapack.Geter));
    	     fdnum=getFdNum(geter);
    	     if(fdnum!=ERROR)
    	     {
    	      Datapack.Ack=2;
    	      SSL_write(clientFD[fdnum],&Datapack,sizeof(struct DataPackage));
    	      SSL_read(NewFd,&Datapack,sizeof(DataPackage));	
    	      if(Datapack.Ack==4)	
    	      {
    	      	SSL_read(NewFd,&Datapack,sizeof(DataPackage));	
    	      	while(Datapack.Ack==5)
    	      	{
    	      	  SSL_write(clientFD[fdnum],&Datapack,sizeof(struct DataPackage));
    	      	  SSL_read(NewFd,&Datapack,sizeof(DataPackage));	
    	      	}
    	      	WriteDataLog(user,geter,Datapack.FileName,csend);
    	      	Datapack.Ack=6;
    	      	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	      	Datapack.Ack=7;
    	      	SSL_write(clientFD[fdnum],&Datapack,sizeof(struct DataPackage));
    	      	SSL_write(clientFD[fdnum],&Datapack,sizeof(struct DataPackage));
    	      }
    	     }else
    	     {
    	     	Datapack.Ack=1;	 
    	     	SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    	     }
    		  }else
    		  {
    		   Datapack.Ack=1;	 
    	     SSL_write(NewFd,&Datapack,sizeof(struct DataPackage));
    		  }	
    		}
    		if(Datapack.Ack==4)
    		{   		   	
    		  strncpy(sender,Datapack.Sender,strlen(Datapack.Sender));
    	      int fdnum_send=getFdNum(sender);   	     
    	      if(fdnum_send!=ERROR)
    	      {
    	        SSL_write(clientFD[fdnum_send],&Datapack,sizeof(struct DataPackage));	
    	      }
    		}
    		if(Datapack.Ack==3)
    		{
    		  strncpy(sender,Datapack.Sender,strlen(Datapack.Sender));
    	      fdnum_send = getFdNum(sender);    	  
    	      if(fdnum_send != ERROR)
    	      {
    	        SSL_write(clientFD[fdnum_send],&Datapack,sizeof(struct DataPackage));	
    	      }	
    		}
    		
    	}
    }
}
/*一次创建多个线程，即线程池*/
int CreatAllThread()
{
  int ret;
  for(int i=0;i<THREADNUM;i++)	
	{
		pthread_t pthread;
		ret=pthread_create(&pthread,NULL,handler,NULL);
		if(ret!=0)
			return ERROR;
	}
}
/*建立服务器端，连接的线程，等用户连接发送信号给处理线程，使之向下执行*/
void* mainThread(void* arg)
{
	char pwd[100];
  char* temp;
  /* SSL 库初始化 */
  SSL_library_init();
  /* 载入所有 SSL 算法 */
  OpenSSL_add_all_algorithms();
  /* 载入所有 SSL 错误消息 */
  SSL_load_error_strings();
  /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
  ctx = SSL_CTX_new(SSLv23_server_method());
  /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
  if (ctx == NULL)
  {
    ERR_print_errors_fp(stdout);
    exit(1);
  }
  /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
  getcwd(pwd,100);
  if(strlen(pwd)==1)
    pwd[0]='\0';
  if (SSL_CTX_use_certificate_file(ctx, temp=strcat(pwd,"/cacert.pem"), SSL_FILETYPE_PEM) <= 0)
  {
    ERR_print_errors_fp(stdout);
    exit(1);
  }
  /* 载入用户私钥 */
  getcwd(pwd,100);
  if(strlen(pwd)==1)
    pwd[0]='\0';
  if (SSL_CTX_use_PrivateKey_file(ctx, temp=strcat(pwd,"/privkey.pem"), SSL_FILETYPE_PEM) <= 0)
  {
    ERR_print_errors_fp(stdout);
    exit(1);
  }
  /* 检查用户私钥是否正确 */
  if (!SSL_CTX_check_private_key(ctx))
  {
    ERR_print_errors_fp(stdout);
    exit(1);
  }
  
  if(SetupTcp() == ERROR)
  	exit(1);
  sin_size=sizeof(struct sockaddr_in);
  CreatAllThread();
  while(1)
  {
    if(ISEXIT==0)	
  	{
	   if((newsockfd = accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size)) == -1)
		{
				perror("accept error!");
				continue; 
		}
		if(CURRENTCLIENT >= CLIENTSIZE)
       {
		  cout<<"beyond connect number 1"<<endl;
		  DataPackage Datapack;
		  Datapack=Package('L',2,voidStr,voidStr,voidStr,voidStr,voidStr,0);
		  SSL *ssl;
		  ssl = SSL_new(ctx);
	   /* 将连接用户的 socket 加入到 SSL */
		  SSL_set_fd(ssl, newsockfd);	
		 if(SSL_accept(ssl) == -1)
		 {
		  perror("Accept");
		  close(newsockfd);
		 }
		  SSL_write(ssl,&Datapack,sizeof(struct DataPackage));
		  cout<<"beyond connect number 2"<<endl;
		  SSL_shutdown(ssl);
		   /* 释放 SSL */
		  SSL_free(ssl);
		  close(newsockfd);	
		  continue; 	
        }
		tempsockfd = newsockfd;
		strcpy(clientIP,inet_ntoa(client_addr.sin_addr));
		pthread_cond_signal(&pthreadCond);		
  	}else
  	{
  	  close(sockfd);
  	  SSL_CTX_free(ctx);
  	  exit(1);			
  	}
  } 	
}
/*主要选择菜单*/
void *mainMenu(void* arg)
{
 int  choice;
 char un[USENAMESIZE];
 char *up;
 while(1)
 {
   cout<<"*******1.Login in Administrator******"<<endl;	
	 cout<<"*******2.EXIT*******"<<endl;
	 cin>>choice; 
	 if(choice==1)
	 {
	   cout<<"Login Name:";
	   cin>>un;
	   up=getpass("Password:");
	   if(OptMysql(un,up,0)==SUCESS)
	   	{
	   	 system("clear");
	   	 break;
	   	}
	  system("clear");
	 }else
	 	pthread_exit((void*)0);	

 }
 while(1)
 {
     cout<<"*******1.Add Administrator******"<<endl;
     cout<<"*******2.Run  Server******"<<endl;	
	 cout<<"*******3.Stop Server*******"<<endl;	
	 cout<<"*******4.Show Online User*******"<<endl;	
	 cout<<"*******5.EXIT*************"<<endl;
	 cin>>choice;
	 switch(choice)
	 {
	 	case 1:
	 	{
	 	  char admun[USENAMESIZE];
	 		char *admup;
	 		cout<<"new administrator name:";
	 		cin>>admun;
	 		admup=getpass("password:");
	 		if(OptMysql(admun,admup,7) == ERROR)
	 			exit(0);
	 	}
	    case 2:ISEXIT = 0;system("clear");cout<<"Server is Running\n";break;
	    case 3:ISEXIT = 1;system("clear");cout<<"Server Stop\n";break;
	    case 4:system("clear");OptMysql(voidStr,voidStr,8);break;
	    case 5:exit(0);mysql_close(&my_connection);		
	 }
 }
 
}
int main(int argc,char *argv[])
{
	pthread_t mainmenu;
	pthread_t mainthread;
	mysql_init(&my_connection);
	int ret = pthread_create(&mainmenu,NULL,mainMenu,NULL);
	if(ret!=0)
	{
	   cout<<"creat mainmenu thread error"<<endl;
	   return ERROR;
	}
	ret = pthread_create(&mainthread,NULL,mainThread,NULL);
	if(ret!=0)
	{
	   cout<<"creat mainthread thread error"<<endl;
	   return ERROR;
	}
	pthread_join(mainmenu,NULL);
	pthread_join(mainthread,NULL);
  mysql_close(&my_connection); 
	return SUCESS;
}