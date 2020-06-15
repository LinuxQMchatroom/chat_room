#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
int n=0;       //全局变量n控制双人实时聊天输出在第几行
int leap=0;		//控制线程shou1运行
int leap1=0;	//控制线程fa运行
//用于控制屏幕输入输出位置
void handle(char str[1024],int people)   
{
	char str1[1024];
	memset(str1,0,1024);
	n++;  			//控制输出在第几行
	printf("\033[0m\033[s");      //保存当前光标
	sprintf(str1,"\033[0m\033[%d;1H",n);
	printf(str1);               	//光标跳到当前屏幕第n行
//   printf("\033[0m\033[1;1H");  //光标跳到第一行，第一格
 	if(strcmp(str,"over")==0)		//over就不打印并结束函数
		return;
	if(people==0)			//people控制输出的本地用户输入信息还是对方信息
		printf("me say:%s\n",str);  
	else
		printf("%s\n",str);
  printf("\033[0m\033[u");   //恢复光标
  fflush(stdout);    //清空I/O缓存中的标准输出缓存使光标恢复到原点
	if(n>=9)
	{
		system("clear");			//清屏
		printf("\033[0m\033[20;1H");//光标跳到第二十行
		printf("Enter:\n");   //主进程输入
		printf("\033[0m\033[K"); //清空光标开始的当前行
		n=0;					//输出超过9行行数归0
	}	
}
//shou1线程接收双人实时聊天的信息
void* shou1(void* newfd) 
{
	while(leap==1)           //全局变量控制线程进行
	{
		int fd=(int)newfd;
		char shou[1024];
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		if(strcmp(shou,"over")==0)   //收到后如果不是over就屏幕打印
			break;
		handle(shou,1);//第一个参数是字符串，第二个如果是0本地输入，1外来输入   				
	}		
}
//shou线程接收服务器消息
void* shou(void* newfd)
{
	int fd=(int)newfd;
	int ret=0,ret1=0;
	char shou[1024];
	char fa[1024];
	int people=0; //people等于0是本地消息，1为外来消息
	while(1)	
	{	
		memset(shou,0,1024);
		if((ret=recv(fd,shou,1024,0))==-1)
			return;
		if(strcmp(shou,"xiazai")==0)//如果收到服务器发来“xiazai”，打开或创建本地文件接收内容
		{
			system("rm -f bendi.txt");//确保本地没有此文件
			int fp;		//文件标识符
			fp=open("bendi.txt",O_WRONLY|O_CREAT|O_TRUNC);
			memset(shou,0,1024);
			//接收数据，如果“endend”表示接收结束
			while(1)		
			{
				recv(fd,&ret,sizeof(int),0);
				write(fd,shou,ret,0);
				if(strncmp(shou,"endend",6)==0)
					break;
				write(fp,shou,ret);
				
			}		
			close(fp);
		}
//如果收到服务器发来“chat”，准备进行双人实时聊天室的建立
			else if(strcmp(shou,"chat")==0)
			{
			leap=1;			//控制线程shou1
			leap1=1;		//控制线程fa
			memset(shou,0,1024);
			
			recv(fd,shou,1024,0);
			
			puts(shou);//此句会从服务器接收输入你想聊天的好友id
			//输入要id 
			
			memset(fa,0,1024);
			
			scanf("%s",&fa);
			
			send(fd,fa,strlen(fa),0);
			
			//进行判断
			
			memset(shou,0,1024);
			
			recv(fd,shou,1024,0);
			
			//由于不存在或不在线等原因，关闭对线程的控制并停止建立双人聊天室
			
			if(strcmp(shou,"此好友不存在")==0)
				
				{
					
					leap=0;				
					
					leap1=0;
					
					puts(shou);
					
					continue;
				}
			else if(strcmp(shou,"好友不在线或没开聊天，请发送私人消息")==0)
				{
					
					leap=0;
					
					leap1=0;
					
					puts(shou);
					
					continue; 
					
				}
				
				//连接成功清屏并按格式控制输出相关信息
				
			else if(strcmp(shou,"连接成功")==0)				
			
			{
			
			puts(shou);
			
			system("clear");
			
			pthread_t id3;
			
			pthread_create(&id3,NULL,shou1,(void*)fd);
			
			while(1)
				
			{
				
				printf("\033[0m\033[20;1H");//光标跳到第20行					
			
				printf("Enter:\n");   //主进程输入
			
				printf("\033[0m\033[K"); 					
			
				memset(fa,0,1024);					
			
				scanf("%s",&fa);
			
				send(fd,fa,strlen(fa),0);
			
				if(strcmp(fa,"over")==0)
			
				{
					
					leap=0;
					
					leap1=0;
					
					system("clear");
					
					printf("你可以使用的功能如下\n\t1.查找好友\n\t2.显示好友\n\t3.删除好友\n\t4.添加好友\n\t5.查看或修改个人信息\n\t6.与好友发送私信\n\t7.群聊\n\tchat.与好友聊天窗口(屏幕分区输入输出)\n\t10.在线查看聊天记录\n\t11.从服务器下载聊天记录\n\t8.退出\n\topen.查看消息");
					
					printf("\033[0m\033[20;1H");//光标跳到第20行
					
					break;
					
				}					
				
			handle(fa,0);
		
			}	 
		
		}
	}

		else

			puts(shou);

		if(ret==0)
		{
			printf("服务器故障\n");
	
			exit(0);
	
		}
	
	}

}
	
	//fa线程用来向服务器发送信息
	
void* fa(void* newfd)
	
{
	
	int fd=(int)newfd;
	
	char fa[1024];
	
	while(1)
			
	{
				
		if(leap1==0)//如果leap1==0，说明在建立双人聊天室，暂停本线程
	
	{
	
	memset(fa,0,1024);
	
	scanf("%s",&fa);
	
	if(send(fd,fa,strlen(fa),0)==-1)
		
		return;
		
	if(strcmp(fa,"chat")==0)//发送chat为要求建立双人聊天室
		
		{
			
			leap1=1;			
			
		}
		
	if(strcmp(fa,"end")==0)//发送end结束客户端程序
		
		{
			
			printf("欢迎使用再见\n");
			
			close(fd);//关闭套接字
			
			exit(0);
			
		}
	
	}
	
}

}

int main()

{
	
	int sockfd=0;//定义并初始化
	
	int ret=0;
	
	int len=sizeof(struct sockaddr);
	

	
	struct sockaddr_in otheraddr;
	
	memset(&otheraddr,0,len);	
	
	//tcp套接字连接
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	
{
																																																																																																																																																																																																																																																																																																																																																																																																																																																				perror("sockfd");
																																																																																																																																																																																																																																																																																																																																																																																																																																																						return -1;	
																																																																																																																																																																																																																																																																																																																																																																																																																																																							}
																																																																																																																																																																																																																																																																																																																																																																																																																																																								else
																																																																																																																																																																																																																																																																																																																																																																																																																																																									{
																																																																																																																																																																																																																																																																																																																																																																																																																																																											printf("socket success...%d\n",sockfd);		
																																																																																																																																																																																																																																																																																																																																																																																																																																																												}
																																																																																																																																																																																																																																																																																																																																																																																																																																																													//初始化结构体，把服务器ip地址和端口号
																																																																																																																																																																																																																																																																																																																																																																																																																																																														otheraddr.sin_family = AF_INET;
																																																																																																																																																																																																																																																																																																																																																																																																																																																															otheraddr.sin_port = htons(8889);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																otheraddr.sin_addr.s_addr=inet_addr("192.168.194.129");
																																																																																																																																																																																																																																																																																																																																																																																																																																																																	//连接服务器
																																																																																																																																																																																																																																																																																																																																																																																																																																																																		if(connect(sockfd,(struct sockaddr*)&otheraddr,len)==-1)
																																																																																																																																																																																																																																																																																																																																																																																																																																																																			{
																																																																																																																																																																																																																																																																																																																																																																																																																																																																					perror("connect");
																																																																																																																																																																																																																																																																																																																																																																																																																																																																							return -1;		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																								}
																																																																																																																																																																																																																																																																																																																																																																																																																																																																									else
																																																																																																																																																																																																																																																																																																																																																																																																																																																																										{
																																																																																																																																																																																																																																																																																																																																																																																																																																																																												printf("connect success...client's fd=%d\n",sockfd);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																														printf("--client ip=%s,port=%d\n",inet_ntoa(otheraddr.sin_addr),ntohs(otheraddr.sin_port));		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																															}
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																//创建线程
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																	pthread_t id1,id2;
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		    char recvbuf[1024];
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																			    char sendbuf[1024];
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																					memset(recvbuf,0,sizeof(recvbuf));
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																						memset(sendbuf,0,1024);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																							//给服务器发送信息，握手判断是否建立连接		
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																								strcpy(sendbuf,"hello,woshi cilent\n");
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																									if(send(sockfd,sendbuf,strlen(sendbuf),0)==-1)
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																										{
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																												perror("send");
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														return -1;
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																															}
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																	
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		if(recv(sockfd,recvbuf,1024,0)==-1)
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																			{
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																					perror("recv");
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																							return -1;
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																								}
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																									else
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																										{
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																												printf("sever say:%s\n",recvbuf);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																													}
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														//启动客户端线程的收发功能
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																															pthread_create(&id2,NULL,fa,(void*)sockfd);	
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																pthread_create(&id1,NULL,shou,(void*)sockfd);
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																	//等待发送线程结束，退出客户端
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		pthread_join(id2,NULL);				
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																			return 0;
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																			}
