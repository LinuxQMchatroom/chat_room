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
#include <ctype.h>
#include <memory.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/stat.h>
#define SIZE sizeof(X)  //结构体X的大小
 
typedef struct xinxi    //结构体xinxi，别名X
{
	char id[20];         //每个账号唯一id
	char passwd[20];	 //账号密码
	char name[50];		 //账号昵称
	char hy[100][20];	 //好友列表，最大100个
	int hys;             //好友数量
	int online;          //0不在线，1在线
	int fd;              //存放客户端成功连接后accept产生的新的套接字，不在线为-5
	int chatroom;		 //存放是否打开了双人聊天室，打开为1，没打开为0
	struct xinxi *next;	 //下一条链表的首地址	
}X;
 
typedef struct message    //结构体message，别名M
{
	char id[20];          //收信息的人
	char id1[20];         //发信息的人
	int type;             //信息类型，好友请求1，私聊信息2,好友请求回复信息3
	char mess[1024];      //信息
	struct message * next;//下一条信息的首地址	
}M;
 
//结构体头地址全局变量方便调用
X* head=NULL;
M* head1=NULL;
//分别为账号个数和保存信息的个数
int count=0;
int count1=0;
int zhzx(char id[20]);//函数声明
 
//用来保存账号信息
void baocun()
{
	FILE *fp;
	fp=fopen("123","w");    //保存在当前运行服务器文件下123
	if(fp==NULL)
	{
		printf("open error\n");
		return;
	}
	printf("count=%d",count);
	fwrite(&count,sizeof(int),1,fp);		//先保存账号个数	
	X* p;
	p=head;
	if(p->next==NULL) 		//如果账号列表为空，关闭文件并退出函数
	{		
		fclose(fp);
		return ;
	}	
	p=p->next;
	while(p)                //按结构体大小保存账号信息
	{
		fwrite(p,sizeof(X),1,fp);
		p=p->next;		
	}	
	printf("账号信息保存成功\n");
	fclose(fp);	
}
//保存未发出去的信息链表进文件
void baocun1()
{
	FILE *fp;
	fp=fopen("1234","w");   //保存在当前运行服务器文件下1234
	if(fp==NULL)
	{
		printf("open error\n");
		return;
	}
	printf("count1=%d\n",count1);
	fwrite(&count1,sizeof(int),1,fp);   //首先保存信息个数	
	M* p;
	p=head1;
	if(p->next==NULL)	//如果信息列表为空，关闭文件并退出函数
	{		
		fclose(fp);
		return ;
	}	
	p=p->next;   //信息链表不为空进入保存
	while(p)
	{
		fwrite(p,sizeof(X),1,fp);  //按保存信息结构体大小
		p=p->next;		
	}
	printf("信息保存成功\n");
	fclose(fp);	
}
//服务器上当前客户端id.txt保存聊天记录
void baocun2(char str[1024],char id[20])
{
	FILE *fp;
	char ch[1024];
	sprintf(ch,"%s.txt",id);
	fp=fopen(ch,"a+");        
	if(fp==NULL)			//打开错误关闭程序
		return ;
	memset(ch,0,1024);
//	puts(str);
	strcat(str,"\r\n");      //txt文档每行句末尾加特殊换行符
	strcpy(ch,str);
//	puts(ch);
	fwrite(ch,strlen(ch),1,fp);			
	printf("聊天记录保存成功\n");
	fclose(fp);
}
//创建账号信息的头指针，并从文件里读取保存已注册的账号信息
X * create()
{
	X* x;
	x=(X*)malloc(SIZE);
	if(x==NULL)
	{
		printf("create error\n");
		return NULL;
	}
	x->next=NULL;
	FILE* fp;
	fp=fopen("123","a+");	//打开文件，读取记录
	if(fp==NULL)
	{
		printf("open error\n");
		return NULL;
	}
	if(fgetc(fp)==EOF) 	 //如果文件为空，返回头指针
	{
		fclose(fp);
		return x;
	}
	rewind(fp);          //文件指针重返文件头
	int n;
	fread(&n,sizeof(int),1,fp);		//先读取长度
	printf("n=%d\n",n);
	count=n;         //全局变量获取账号长度
	X t;
	X *p,*p1;
	int i;
	for(i=0;i<n;i++)				//按账号结构体长度读取信息
	{
		fread(&t,sizeof(X),1,fp);
		p1=x;
		while(p1->next)
		{
			p1=p1->next;
		}
		p=(X*)malloc(sizeof(X));     //每次创建一个新的账号结构体大小空间来存信息
		p->next=NULL;
		strcpy(p->id,t.id);				//账号，昵称，密码，好友及好友数读取保存的
		strcpy(p->name,t.name);	
		strcpy(p->passwd,t.passwd);		
		p->hys=t.hys;
		int j;
		for(j=0;j<p->hys;j++)
			strcpy(p->hy[j],t.hy[j]);
		p->fd=-5;               //每次服务器新连接字置-5
		p->chatroom=0;			//双人聊天室置零为没打开
		p->online=0;			//在线状态置零为不在线
		p1->next=p;				
	}
	fclose(fp);
	return x;
}
//创建缓存信息的头指针，并从文件里读取保存未发送的信息
M * create1()
{
	M * x;
	x=(M*)malloc(sizeof(M));          //创建头结点
	if(x==NULL)
	{
		printf("create error\n");
		return NULL;
	}
	x->next=NULL;
	FILE* fp;              
	fp=fopen("1234","a+");				//打开文件并读取信息
	if(fp==NULL)
	{
		printf("open error\n");
		return NULL;
	}
	if(fgetc(fp)==EOF)                   //如果为空文件关闭文件直接返回头指针
	{
		fclose(fp);
		return x;
	}
	rewind(fp);                         //重置文件指针
	int n;
	fread(&n,sizeof(int),1,fp);
	printf("n=%d\n",n); 				//先读取缓存信息长度
	count1=n;
	M t;
	M *p,*p1;
	int i;
	for(i=0;i<n;i++)					//把信息取出
	{
		fread(&t,sizeof(M),1,fp);
		p1=x;
		while(p1->next)
		{
			p1=p1->next;
		}
		p=(M*)malloc(sizeof(M));
		p->next=NULL;
		strcpy(p->id,t.id);
		strcpy(p->id1,t.id1);
		p->type=t.type;
		strcpy(p->mess,t.mess);
		p1->next=p;
		
	}
	fclose(fp);
	return x;
}
//判断字符串是否全是数字，是返回1，不是返回0
int isnum1(char s[20])
{
	int i=0;
	while(s[i])
	{
		if(!isdigit(s[i]))
			return 0;
		i++;		
	}
	return 1;	
}
void add(int fd)		//注册功能函数
{
	X *p1,*p,*p2;
	int leap=0;            //标识符，账号是否能正确注册
	p=(X*)malloc(SIZE);			//开辟一个账号信息结构体大小的空间
	if(p==NULL)
		return;
	char str[256];
	char str1[256];
	memset(str,0,256);
	memset(str1,0,256);
	strcpy(str,"请输入你想注册的账号");
	send(fd,str,strlen(str),0);
	memset(str,0,256);
	recv(fd,str,256,0);
	strcpy(str1,str);
	if(!isnum1(str))          //判断是否纯数字账号
	{
		memset(str,0,256);
		strcpy(str,"请输入纯数字账号!\n");
		send(fd,str,strlen(str),0);
		return;
	}
	p1=head;
	while(p1->next)
	{
		if(strcmp(p1->next->id,str)==0)
		{	
			leap=1;
			break;
		}
		p1=p1->next;
	}
	if(leap==1)
	{
		memset(str,0,256);
		strcpy(str,"账号重复\n");
		send(fd,str,strlen(str),0);		
		return;
	}
	//注册账号信息并赋初值
	strcpy(p->id,str1);
	memset(str,0,256);	
	strcpy(str,"请输入密码");
	send(fd,str,strlen(str),0);			
	
	memset(str,0,256);	
	recv(fd,str,256,0);
	strcpy(p->passwd,str);
 
	memset(str,0,256);	
	strcpy(str,"请输入昵称");
	send(fd,str,strlen(str),0);	
 
	memset(str,0,256);	
	recv(fd,str,256,0);
	strcpy(p->name,str);
	p1=head;
	while(p1->next)
	{
		p1=p1->next;
	}
	p1->next=p;
	p->hys=0;
	p->online=0;
	p->fd=-5;
	p->next=NULL;		
	memset(str,0,256);	
	strcpy(str,"注册成功\n");
	send(fd,str,strlen(str),0);		
	count++;         //全局变量账号数量+1
	baocun();		//保存一次账号信息在文件
}
int yzzh(char id[20])//验证id是否存在
{
	X *p;
	int leap=0;
	//账号信息结构体头结点开始遍历
	if(head->next==NULL)
		return 0;
	p=head->next;
	while(p)
	{
		if(strcmp(id,p->id)==0)
			return 1;
		p=p->next;
	}
	return 0;
}
int yzmm(char id[20],char passwd[20])//验证passwd是否存在
{
	X *p;
	int leap=0;
	if(head->next==NULL)
		return 0;
	p=head->next;
	while(p)
	{
		if(strcmp(id,p->id)==0&&strcmp(passwd,p->passwd)==0)
			return 1;
		p=p->next;
	}
	return 0;
}
 
void look(int fd,char id[20])//查找好友，只能是已加为好友的好友
{
	X* p,*p1;
	p=head;
	char fa[1024];
	char shou[1024];
	char shouid[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	memset(shouid,0,1024);	
	strcpy(fa,"输入您要查找的号码");
	send(fd,fa,strlen(fa),0);	
	recv(fd,shouid,1024,0);
	if(yzzh(shouid)==0)                 //先从账号链表里查询是否存在此账号
	{
		memset(fa,0,1024);
		strcpy(fa,"此号码不存在");
		send(fd,fa,strlen(fa),0);
		return;		
	}
	p=p->next;
	while(p)							//从好友列表里查询是否有此好友
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}		
	if(p->hys==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"您没有此好友");
		send(fd,fa,strlen(fa),0);
		return;
	}	
	int i;						//有次好友打印相关信息
	for(i=0;i<(p->hys);i++)
	{
		if(strcmp((p->hy[i]),shouid)==0)
		{
			p1=head->next;
			char str[20];
			memset(str,0,20);
			while(p1)
			{
				if(strcmp(p1->id,shouid)==0)
				{
					strcpy(str,p1->name);
					break;
				}
				p1=p1->next;
			}			
			memset(fa,0,1024);
			sprintf(fa,"此好友账号%s,昵称%s\n",shouid,str);
			send(fd,fa,strlen(fa),0);
			return;
		}		
	}	
	memset(fa,0,1024);
	sprintf(fa,"您没有此好友");
	send(fd,fa,strlen(fa),0);	
}
 
void display(int fd,char id[20])//列出好友信息
{
	X * p,*p1;
	int i;
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	if(p->hys==0)
	{
		strcpy(fa,"您的好友列表为空");
		send(fd,fa,strlen(fa),0);
		return;		
	}
	for(i=0;i<p->hys;i++)
	{
		memset(fa,0,1024);
		p1=head->next;
		char str[20];
		int leap=0;
		memset(str,0,20);
		while(p1)
		{
			if(strcmp(p1->id,p->hy[i])==0)
			{
				strcpy(str,p1->name);
	//			printf("%s\n",p1->id);
				leap=1;
				break;
			}
			p1=p1->next;
		}
		if(leap==1)
		{
			memset(fa,0,1024);
			sprintf(fa,"账号%s\t昵称%s\n",p1->id,p1->name);
			send(fd,fa,strlen(fa),0);
			return ;
		}
	}
}
 
void del(int fd,char id[20])//删除好友
{
	X *p,*p1;
	int i,leap=0;
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	if(p->hys==0)                   //先判断好友列表
	{
		strcpy(fa,"您的好友列表为空");
		send(fd,fa,strlen(fa),0);
		return;		
	}
	memset(fa,0,1024);
	strcpy(fa,"输入你想删除的好友账号");
	send(fd,fa,strlen(fa),0);	
	recv(fd,shou,1024,0);		
	for(i=0;i<(p->hys);i++)
	{
		if(strcmp((p->hy[i]),shou)==0)
		{
			while(i<(p->hys-1))
			{
				strcpy(p->hy[i],p->hy[i+1]);
				i++;
			}
			memset(p->hy[i],0,20);
			p->hys--;
			memset(fa,0,1024);
			sprintf(fa,"删除成功");
			send(fd,fa,strlen(fa),0);
			leap=1;						
			baocun();
			break;
		}		
	}
	if(1==leap)    //如果删除本地成功，在对方好友列表中也删除自己
	{
		p1=head->next;
		while(p1)
		{
			if(strcmp(p1->id,shou)==0)
				break;
			p1=p1->next;
		}
		for(i=0;i<(p1->hys);i++)
		{
			if(strcmp((p1->hy[i]),p->id)==0)
			{
				while(i<(p1->hys-1))
				{
					strcpy(p1->hy[i],p1->hy[i+1]);
					i++;
				}
				memset(p1->hy[i],0,20);
				p1->hys--;
				memset(fa,0,1024);
				sprintf(fa,"对方那也删除成功");
				send(fd,fa,strlen(fa),0);			
				baocun();
				break;
			}			
		}				
	}
	else
	{
		memset(fa,0,1024);
		sprintf(fa,"删除失败，您没有此好友");
		send(fd,fa,strlen(fa),0);
	}	
}
//加好友
void addhy(int fd,char id[20])
{
	X *p,*p1;
	int i;
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	strcpy(fa,"输入你想添加的好友账号");
	send(fd,fa,strlen(fa),0);
	recv(fd,shou,1024,0);
	memset(fa,0,1024);
	strcpy(fa,"消息已发送");
	send(fd,fa,strlen(fa),0);	
	if(yzzh(shou)==0)			//判断是否有账号
	{
		memset(fa,0,1024);
		strcpy(fa,"没有此账号");
		send(fd,fa,strlen(fa),0);
		return;
	}
	if(strcmp(shou,p->id)==0)     //不能添加自己为好友
	{
		memset(fa,0,1024);
		strcpy(fa,"不能添加自己为好友");
		send(fd,fa,strlen(fa),0);
		return;
	}
	for(i=0;i<p->hys;i++)          //此好友已添加，无需再添加
	{
		if(strcmp(shou,p->hy[i])==0)
		{
			memset(fa,0,1024);
			strcpy(fa,"此好友已添加，无需再添加");
			send(fd,fa,strlen(fa),0);
			return;			
		}		
	}
	p1=head->next;
	time_t timep;	
	while(p1)			//给对方发了好友信息，通知对方，如果对方不在线，上线再通知
	{
		if(strcmp(p1->id,shou)==0)
		{
			M* m,*p3;
			m=(M*)malloc(sizeof(M));
			m->type=1;
			strcpy(m->id,shou);
			strcpy(m->id1,p->id);
			memset(fa,0,1024);
			time(&timep);
			sprintf(fa,"%s\n%s想添加您为好友，y同意，n不同意",ctime(&timep),p->id);
			strcpy(m->mess,fa);
			m->next=NULL;
			p3=head1;
			while(p3->next)
				p3=p3->next;
			p3->next=m;
			count1++;			
			//如果对方在线，给对方发送有新消息
			if(zhzx(p1->id)==1)
			{
				memset(fa,0,1024);
				strcpy(fa,"您有新消息，请输入open查看");
				send(p1->fd,fa,strlen(fa),0);
			}			
			baocun1();     //保存信息
		}				
		p1=p1->next;
	}
}	
//修改个人信息	
void xgnc(int fd,char id[20])
{
	X *p,*p1;
	int i;
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	sprintf(fa,"我的昵称:%s\n我的账号:%s我的密码:%s",p->name,p->id,p->passwd);
	send(fd,fa,strlen(fa),0);
	memset(fa,0,1024);
	strcpy(fa,"\t1.修改昵称\n\t2.修改密码");
	send(fd,fa,strlen(fa),0);	
	recv(fd,shou,1024,0);
	if(strcmp(shou,"1")==0)				//输入1修改昵称
	{
		memset(fa,0,1024);
		strcpy(fa,"请输入新的昵称");
		send(fd,fa,strlen(fa),0);		
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		strcpy(p->name,shou);
		baocun();		
		return ;
	}
	else if(strcmp(shou,"2")==0)		//输入2修改密码
	{
		memset(fa,0,1024);
		strcpy(fa,"请输入新的密码");
		send(fd,fa,strlen(fa),0);		
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		strcpy(p->passwd,shou);
		baocun();	//保存一次		
		return ;
	}
	else
	{
		memset(fa,0,1024);
		strcpy(fa,"输入非法");
		send(fd,fa,strlen(fa),0);
		return;
	}	
}
//发送私聊邮件信息
void * siliao(int fd,char id[20])
{
	X *p,*p1;
	int i,leap=0;
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	strcpy(fa,"请输入你想私聊的好友账号");
	send(fd,fa,strlen(fa),0);	
	recv(fd,shou,1024,0);	
	if(strcmp(shou,p->id)==0)		//不能与自己聊天
	{
		memset(fa,0,1024);
		strcpy(fa,"不能与自己聊天");
		send(fd,fa,strlen(fa),0);
		return;		
	}	
	if(p->hys==0)					//判断是否有此好友
	{
		memset(fa,0,1024);
		strcpy(fa,"你没有此好友");
		send(fd,fa,strlen(fa),0);
		return;
	}
//	printf("phys:%d\n",p->hys);
	for(i=0;i<p->hys;i++)
	{
		if(strcmp(p->hy[i],shou)==0)
		{
			leap=1;
			p1=head->next;
			while(p1)
			{
				if(strcmp(p1->id,shou)==0)
					break;
				p1=p1->next;
			}
			printf("p fd%d,p1 fd%d\n",p->fd,p1->fd);
			memset(fa,0,1024);
			strcpy(fa,"请输入你要发送给他的内容,输入over结束");
			send(fd,fa,strlen(fa),0);			
			if(p1->fd==-5)				//好友不在线提示，但依旧能发送
			{
				memset(fa,0,1024);
				strcpy(fa,"提示：好友不在线，可能无法及时回复");
				send(fd,fa,strlen(fa),0);								
			}
			M* m,*p8;
			int kkk=0;
			time_t timep;			
			while(1)
			{	
				memset(shou,0,1024);
				if(recv(fd,shou,1024,0)==-1)
					return;
				if(strcmp(shou,"over")==0)
					break;
				kkk=1;
				p8=head1;
				while(p8->next)
				{
					p8=p8->next;
				}
				
				m=(M*)malloc(sizeof(M));
				m->type=2;
				strcpy(m->id,p1->id);
				strcpy(m->id1,p->id);
				memset(fa,0,1024);
				p8->next=m;
				m->next=NULL;
				time(&timep);
				sprintf(fa,"%s  %s say:%s\n",ctime(&timep),p->id,shou);
				strcpy(m->mess,fa);
				//////////////////////////////
				baocun2(fa,id);
				printf("%s对%s说%s\n",m->id1,m->id,m->mess);
				count1++;
				printf("count1=%d\n",count1);
			}
			if(zhzx(p1->id)==1&&kkk==1)			//给对方发了信息，通知对方，如果对方不在线，上线再通知
			{
				memset(fa,0,1024);
				strcpy(fa,"您有新消息，请输入open查看");
				send(p1->fd,fa,strlen(fa),0);				
			}			
			baocun1();   //保存未被接收的信息
			return;
		}		
	}
	memset(fa,0,1024);
	strcpy(fa,"你没有此好友");
	send(fd,fa,strlen(fa),0);
}
int zhzx(char id[20])//验证账号是否在线
{
	X* p;
	p=head;
	while(p)
	{
		if((strcmp(p->id,id)==0)&&p->online==1)
			return 1;			//在线返回1
		p=p->next;
	}
	return 0;
}
char a[100][20];	//放聊天室人的id
int len=0;			//聊天室人数
//创建多人聊天室
void duorenliao(int fd,char id[20])
{
	system("play -q 11.wav  repeat 2");   	//播放进入聊天室的提示音
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	strcpy(fa,"您已进入聊天室，输入stop退出,输入look查看当前人");
	send(fd,fa,strlen(fa),0);
	strcpy(a[len],id);
	len++;					//每进入一个人，长度加1
	int i;
	X* p;
	time_t timep;
	time(&timep);  			//时间函数
	while(1)				//建立聊天室基本信息
	{
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		if(strcmp(shou,"stop")==0)		//stop退出聊天室
		{
			for(i=0;i<len;i++)
			{
				if(strcmp(a[i],id)==0)
				{
					while(i<len-1)
					{
						strcpy(a[i],a[i+1]);
						i++;
					}
				}				
			}
			len--;
			for(i=0;i<len;i++)
			{
				p=head->next;
				while(p)
				{
					if(strcmp(p->id,a[i])==0)
					{
						memset(fa,0,1024);
						sprintf(fa,"%s退出聊天室",id);
						send(p->fd,fa,strlen(fa),0);
						break;
					}
					p=p->next;
				}
			}			
			return;
		}
		if(strcmp(shou,"look")==0)          //look查看聊天室有多少人，并显示出他们的昵称以及账号
		{
			memset(fa,0,1024);
			sprintf(fa,"当前聊天室有%d人,他们是：",len);
			send(fd,fa,strlen(fa),0);
			for(i=0;i<len;i++)
			{
				p=head->next;
				while(p)
				{
					if(strcmp(p->id,a[i])==0)
					{
						memset(fa,0,1024);
						sprintf(fa,"昵称是%s  账号是%s\n",p->name,p->id);
						send(fd,fa,strlen(fa),0);
						break;
					}
					p=p->next;
				}							
			}
			continue;
		}
		for(i=0;i<len;i++)        //发的消息对聊天室里所有人发出
		{
			p=head->next;
			while(p)
			{
				if(strcmp(p->id,a[i])==0&&strcmp(p->id,id)!=0)
				{
					memset(fa,0,1024);
					time(&timep);
					sprintf(fa,"%s%s say: %s",ctime(&timep),id,shou);
					send(p->fd,fa,strlen(fa),0);
					break;
				}
				p=p->next;
			}
		}
	}	
}
//在线从服务器查看聊天记录
void jilu(int fd,char id[20])
{
	int fp;      //文件标识符
	char str[1024];
	sprintf(str,"%s.txt",id); //打开相应人的聊天记录文本文件
	fp=open(str,O_RDONLY);    //只读方式打开
	if(fp==-1)
	{
		memset(str,0,1024);
		strcpy(str,"您没有聊天记录");
		send(fd,str,strlen(str),0);
		return;
	}
	int ret;
	memset(str,0,1024);
	while(ret=read(fp,str,1024))	//ret读的长度，为0退出while
	{
		write(fd,str,strlen(str));	//读出来发送给客户端
		memset(str,0,1024);		
	}	
	close(fp);
}
//客户端从服务器下载聊天文件
void xiazai(int fd,char id[20])
{
	int fp;
	char str[1024];
	sprintf(str,"%s.txt",id);
	fp=open(str,O_RDONLY);
	if(fp==-1)							//先判断有没有聊天记录文件
	{
		memset(str,0,1024);
		strcpy(str,"您没有聊天记录");
		send(fd,str,strlen(str),0);
		return;
	}
	int ret;
	memset(str,0,1024);
	strcpy(str,"xiazai");				//发送“xiazai”，让客户端准备接收
	send(fd,str,strlen(str),0);
	memset(str,0,1024);
	while(ret=read(fp,str,1024))
	{
		send(fd,&ret,sizeof(int),0);		//先发送内容字节数，再发送内容
		send(fd,str,ret,0);
		memset(str,0,1024);
	}
	memset(str,0,1024);
	strcpy(str,"endend");	
	ret = strlen(str);						//结束后发送endend告诉客户端发送完毕
	send(fd,&ret,sizeof(int),0);
	send(fd,str,strlen(str),0);
	close(fp);
}
//双人聊天室，输入输出分开
void doublechat(int fd,char id[20])
{
	X * p;
	p=head->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	p->chatroom=1;
	char shou[1024];
	char fa[1024];
	int ret=0;
	memset(shou,0,1024);
	memset(fa,0,1024);	
	strcpy(fa,"chat");
	send(fd,fa,strlen(fa),0);	
	memset(fa,0,1024);
	strcpy(fa,"输入你想聊天的好友id");
	send(fd,fa,strlen(fa),0);	
	recv(fd,shou,1024,0);
	X* p1;					//先判断该人存不存在
	p1=head->next;
	while(p1)
	{
		if(strcmp(p1->id,shou)==0)
			break;
		p1=p1->next;
	}
	if(p1==NULL)
	{				
		memset(fa,0,1024);	
		strcpy(fa,"此好友不存在");
		send(fd,fa,strlen(fa),0);
		p->chatroom=0;
		return;
	}
	if(p1->fd==-5||p1->chatroom==0)			//判断好友在不在线和有没有打开聊天窗口
	{
		memset(fa,0,1024);
		strcpy(fa,"好友不在线或没开聊天，请发送私人消息");
		send(fd,fa,1024,0);
		p->chatroom=0;
		return;
	}
	memset(fa,0,1024);
	strcpy(fa,"连接成功");
	send(fd,fa,1024,0);
	
	while(1)					//进入聊天，输入over退出
	{
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		if(strcmp(shou,"over")==0)
		{
			p->chatroom=0;
			memset(fa,0,1024);		
			sprintf(fa,"%s 已退出,请您输入over退出",id);
			if(p1->chatroom==1)
				send(p1->fd,fa,1024,0);
			break;
		}
		memset(fa,0,1024);		
		sprintf(fa,"%s say:%s",id,shou);		
		send(p1->fd,fa,1024,0);
		baocun1();//保存聊天记录
	}		
}
//并发服务器用线程来完成