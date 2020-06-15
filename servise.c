#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<memory.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<time.h>
#include<sys/stat.h>
#define SIZE sizeof(X)  //结构体x的大小

typedef struct weiliao //
{
	char id[20];
	char passwd[20];
	char name[50];
	char hy[100][20];//好友列表吧
	int hys; //好友数量
	int online; //0在，1不在
	int fd;
	int chatroom;
	struct weiliao *next;//下一链表的首地址
}X;

typedef struct message
{
	char id[20];//收人
	char id1[20];//发人
	int type;
	char mess[1024];
	struct message * next;
}M;

//结构体头地址全局变量
X* head=NULL;
M* head1=NULL;
//帐号个数和保存信息个数
int count=0;
int count1=0;
int zhzx(char id[20]);//函数声明

//用来保存帐号信息
void baocun()
{
	FILE *fp;
	fp=fopen("123","w");
	if(fp==NULL)
	{
		printf("open error\n");
		return;
	}
	printf("count =%d",count);
	fwrite(&count,sizeof(int),1,fp);//先保存帐号个数
	X* p;
	p=head;
	if(p->next==NULL)//如果为空，关闭并退出
	{
		fclose(fp);
		return;
	}
	p=p->next;
	while(p)//结构体保存帐号大小
	{
		fwrite(p,sizeof(X),1,fp);
		p=p->next;
	}
	printf("帐号信息保存成功");
	fclose(fp);
}
//保存未发出的信息链表文件
void baocun1()
{
	FILE *fp;
	fp=fopen("123","w");
	if(fp==NULL)
	{
		printf("open errror!\n");
		return;
	}
	printf("count1=%d\n",count1);
	fwrite(&count1,sizeof(int),1,fp);
	M* p;
	p=head1;
	if(p->next==NULL)
	{
		fclose(fp);
		return ;
	}
	p=p->next;
	while(p)
	{
		fwrite(p,sizeof(X),1,fp);
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
	if(fp==NULL)
			return ;
	memset(ch,0,1024);
	strcat(str,"\r\n");
	strcpy(ch,str);
	fwrite(ch,strlen(ch),1,fp);
	printf("聊天记录保存成功\n");
	fclose(fp);
}
//创建帐号消息的头指针，并从头文件里读取保存已注册的帐号信息
X * create()
{
	X* x;
	x=(X*)malloc(SIZE);
	if(x=NULL)
	{
		printf("create error\n");
		return NULL;
	}
	x->next=NULL;
	FILE* fp;
	fp=fopen("123","a+");
	if(fp==NULL)
	{
		printf("open error\n");
		return NULL;
	}
	if(fgetc(fp)==EOF)
	{
		fclose(fp);
		return x;
	}
	rewind(fp);
	int n;
	fread(&n,sizeof(int),1,fp);
	printf("n=%d\n",n);
	count=n;
	X t;
	X *p,*p1;
	int i;
	for(i=0;i<n;i++)
	{
		fread(&t,sizeof(X),1,fp);
		p1=x;
		while(p1->next)
		{
			p1=p1->next;
		}
		p=(X*)malloc(sizeof(X));
		p->next=NULL;
		strcpy(p->id,t.id);
		strcpy(p->name,t.name);
		strcpy(p->passwd,t.passwd);
		p->hys=t.hys;
		int j;
		for(j=0;j<p->hys;j++)
			strcpy(p->hy[j],t.hy[j]);
		p->fd=-5;
		p->chatroom=0;
		p->online=0;
		p1->next=p;
	}
	fclose(fp);
	return x;
}
//创建缓存信息的头指针，并从文件里读取未发送的信息
M * create1()
{
	M* x;
	x=(M*)malloc(sizeof(M));
	if(x=NULL)
	{
		printf("create1 error!\n");
		return NULL;
	}
	x->next=NULL;
	FILE* fp;
	fp=fopen("123","a+");
	if(fp=NULL)
	{
		printf("open error\n");
		return NULL;
	}
	if(fgetc(fp)==EOF)
	{
		fclose(fp);
		return x;
	}
	rewind(fp);
	int n;
	fread(&n,sizeof(int),1,fp);
	printf("n=%d\n",n);
	count1=n;
	M t;
	M *p,*p1;
	int i;
	for(i=0;i<n;i++)
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
//判断字符是否符全是数字，是返回1,不是为0
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
void add(int fd)
{
	X *p1,*p,*p2;
	int leap=0;
	p=(X*)malloc(SIZE);
	if(p==NULL)
			return;
	char str[256];
	char str1[256];
	memset(str,0,256);
	strcpy(str,"请输入你想注册的帐号！");
	send(fd,str,strlen(str),0);
	memset(str,0,256);
	recv(fd,str,256,0);
	strcpy(str1,str);
	if(!isnum1(str))
	{
		memset(str,0,256);
		strcpy(str,"请输入纯数字帐号！");
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
		strcpy(str,"帐号重复\n");
		send(fd,str,strlen(str),0);
		return;
	}
	//注册帐号信息并赋初值
	strcpy(p->id,str1);
	memset(str,0,256);
	strcpy(str,"请输入密码");
	send(fd,str,strlen(str),0);

	memset(str,0,256);
	recv(fd,str,256,0);
	strcpy(p->passwd,str);

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
	strcpy(str,"注册成功！\n");
	send(fd,str,strlen(str),0);
	count++;
	baocun();
}
int yzzh(char id[20])
{
	X *p;
	int leap=0;
	//帐号信息结构体头节点开始遍历
	if(head->next=NULL)
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

void look(int fd,char id[20])//查找好友，只是已加好友的好友
{
	X *p,*p1;
	p=head;
	char fa[1024];
	char shou[1024];
	char shouid[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	memset(shouid,0,1024);
	strcpy(fa,"输入你要查找的号码");
	send(fd,fa,strlen(fa),0);
	recv(fd,shouid,1024,0);
	if(yzzh(shouid)==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"此号码不存在");
		send(fd,fa,strlen(fa),0);
		return;
	}
	p=p->next;
	while(p)
	{
		if(strcmp(p->id,id)==0)
			break;
		p=p->next;
	}
	if(p->hys==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"你没有此好友");
		send(fd,fa,strlen(fa),0);
		return;
	}
	int i;
	for(i=0;i<(p->hys);i++)
	{
		if(strcmp(p->hy[i],shouid)==0)
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
			sprintf(fa,"此好友帐号%s,昵称%s\n",shouid,str);
			send(fd,fa,strlen(fa),0);
			return;
		}
	}
	memset(fa,0,1024);
	sprintf(fa,"你没有此好友");
	send(fd,fa,strlen(fa),0);
}

void diaplay(int fd,char id[20])
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
	if(p->hys==0)
	{
		strcpy(fa,"你的好友列表为空");
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
				leap=1;
				break;
			}
			p1=p1->next;
		}
		if(leap==1)
		{
			memset(fa,0,1024);
			sprintf(fa,"帐号%s,昵称%s\n",p1->id,p1->name);
			send(fd,fa,strlen(fa),0);
			return;
		}
	}
}

void del(int fd,char id[20])
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
	if(p->hys==0)
	{
		strcpy(fa,"你的好友列表为空");
		send(fd,fa,strlen(fa),0);
		recv(fd,shou,1024,0);
		for(i=0;i<(p->hys);i++)
		{
			if(strcmp(p->hy[i],shou)==0)
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
		if(1==leap)
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
	}
	else
	{
		memset(fa,0,1024);
		sprintf(fa,"删除失败，你没有此好友");
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
	strcpy(fa,"输入你想添加的好友帐号");
	send(fd,fa,strlen(fa),0);
	recv(fd,shou,1024,0);
	memset(fa,0,1024);
	strcpy(fa,"消息已经发送");
	send(fd,fa,strlen(fa),0);
	if(yzzh(shou)==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"没有此帐号");
		send(fd,fa,strlen(fa),0);
		return;
	}
	for(i=0;i<p->hys;i++)
	{
		  if(strcmp(shou,p->id)==0)
		   {   
		       memset(fa,0,1024);
		       strcpy(fa,"不能添加自己未好友");
		       send(fd,fa,strlen(fa),0);
		           return;
		    }
	}
	p1=head->next;
	time_t timep;
	while(p1)
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
			sprintf(fa,"%s\n%s想添加你为好友，y同意，n不同意",ctime(&timep),p->id);
			strcpy(m->mess,fa);
			m->next=NULL;
			p3=head1;
			while(p3->next)
				p3=p3->next;
			p3->next=m;
			count1++;
			//如果对方在线，给对方发送新消息
			if(zhzx(p1->id)==1)
			{
				memset(fa,0,1024);
				strcpy(fa,"你有新消息，请输入open查看");
				send(p1->fd,fa,strlen(fa),0);
			}
			baocun1();
		}
		p1=p1->next;
	}
}
//修改个人信息
void xgnc(int fd,char id[20])
{
	X *p,*P1;
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
	sprintf(fa,"我的昵称：%s\n我的帐号:%s\n我的密码:%s\n",p->name,p->id,p->passwd);
	send(fd,fa,strlen(fa),0);
	memset(fa,0,1024);
	strcpy(fa,"\t1.修改昵称\n\t2.修改密码");
	send(fd,fa,strlen(fa),0);
	recv(fd,shou,1024,0);
	if(strcmp(shou,"1")==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"请输入新的昵称");
		send(fd,fa,strlen(fa),0);
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		strcpy(p->name,shou);
		baocun();
		return;
	}
	else if(strcmp(shou,"2")==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"请输入新的密码");
		send(fd,fa,strlen(fa),0);
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		strcpy(p->passwd,shou);
		baocun();
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
	strcpy(fa,"请输入你想私聊的好友帐号");
	send(fd,fa,strlen(fa),0);
	recv(fd,shou,1024,0);
	if(strcmp(shou,p->id)==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"不能与自己聊天");
		send(fd,fa,strlen(fa),0);
		return NULL;
	}
	if(p->hys==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"你没有此好友");
		send(fd,fa,strlen(fa),0);
		return NULL;
	}
	
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
			strcpy(fa,"请输入你要发送给他的内容，输入over结束");
			send(fd,fa,strlen(fa),0);
			if(p1->fd==-5)
			{
				memset(fa,0,1024);
				strcpy(fa,"请输入你要发送给他的内容，输入over结束");
				send(fd,fa,strlen(fa),0);
				if(p1->fd==-5)
				{
					memset(fa,0,1024);
					strcpy(fa,"提示，好友不在线，可能无法及时回复");
					send(fd,fa,strlen(fa),0);
				}
				M* m,*p8;
				int kkk=0;
				time_t time_p;
				while(i)
				{
					memset(shou,0,1024);
					if(recv(fd,shou,1024,0)==-1)
						return NULL;
					if(strcmp(shou,"over")==0)
						break;
					kkk=1;
					p8=head1;
					time_t timep;
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
					sprintf(fa,"%s %s say:%s\n",ctime(&timep),p->id,shou);
					strcpy(m->mess,fa);

					baocun2(fa,id);
					printf("%s对%s说%s\n",m->id1,m->id,m->mess);
					count1++;
					printf("count1=%d\n",count1);

				}
				if(zhzx(p1->id)==1&&kkk==1)
				{
					memset(fa,0,1024);
					strcpy(fa,"你有新消息，请输入open查看");
					send(p1->fd,fa,strlen(fa),0);
				}
				baocun1();
				return NULL;
			}
		}
		memset(fa,0,1024);
		strcpy(fa,"你没有此好友");
		send(fd,fa,strlen(fa),0);
	}
}
int zhzx(char id[20])
{
	X* p;
	p=head;
	while(p)
	{
		if((strcmp(p->id,id)==0)&&p->online==1)
			return 1;
		p=p->next;
	}
	return 0;
}
char a[100][20];
int len=0;
//创建多人聊天室
void duorenlioa(int fd,char id[20])
{
	system("play -q 11.wav repeat 2");
	char fa[1024];
	char shou[1024];
	memset(fa,0,1024);
	memset(shou,0,1024);
	strcpy(fa,"你已经进入聊天室，输入stop退出,输入look查看当前人数");
	send(fd,fa,strlen(fa),0);
	strcpy(a[len],id);
	len++;
	int i;
	X* p;
	time_t timep;
	time(&timep);
	while(1)
	{
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		if(strcmp(shou,"stop")==0)
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
		if(strcmp(shou,"look")==0)
		{
			memset(fa,0,1024);
			sprintf(fa,"当前聊天室有%d人，他们是：",len);
			send(fd,fa,strlen(fa),0);
			for(i=0;i<len;i++)
			{
				p=head->next;
				while(p)
				{
					if(strcmp(p->id,a[i])==0)
					{
						memset(fa,0,1024);
						sprintf(fa,"昵称是%s 帐号是%s\n",p->name,p->id);
						send(fd,fa,strlen(fa),0);
						break;
					}
					p=p->next;
				}
			}
			continue;
		}
		for(i=0;i<len;i++)
		{
			p=head->next;
			while(p)
			{
				if(strcmp(p->id,a[i])==0&&strcmp(p->id,id)!=0)
				{
					memset(fa,0,1024);
					time(&timep);
					sprintf(fa,"%s%s say:%s",ctime(&timep),id,shou);
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
	int fp;
	char str[1024];
	sprintf(str,"%s.txt",id);
	fp=open(str,O_RDONLY);
	if(fp==-1)
	{
		memset(str,0,1024);
		strcpy(str,"你没有聊天记录");
		send(fd,str,strlen(str),0);
		return;
	}
	int ret;
	memset(str,0,1024);
	while(ret=read(fp,str,1024))
	{
		write(fd,str,strlen(str));
		memset(str,0,1024);
	}
	close(fp);
}
//客户端从服务器下载聊天记录
void xiazai(int fd,char id[20])
{
	int fp;
	char str[1024];
	sprintf(str,"%s.txt",id);
	fp=open(str,O_RDONLY);
	if(fp==-1)
	{
		memset(str,0,1024);
		strcpy(str,"你没有聊天记录");
		send(fd,str,strlen(str),0);
		return;
	}
	int ret;
	memset(str,0,1024);
	strcpy(str,"xiazai");
	send(fd,str,strlen(str),0);
	memset(str,0,1024);
	while(ret==read(fp,str,1024))
	{
		send(fd,&ret,sizeof(int),0);
		send(fd,str,ret,0);
		memset(str,0,1024);
	}
	memset(str,0,1024);
	strcpy(str,"endend");
	ret=strlen(str);
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
	strcpy(fa,"请输入你想聊天的好友");
	send(fd,fa,strlen(fa),0);
	recv(fd,shou,1024,0);
	X* p1;
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
	if(p1->fd==-5||p1->chatroom==0)
	{
		memset(fa,0,1024);
		strcpy(fa,"好友不在线或没聊天，请发送私人消息");
		send(fd,fa,1024,0);
		p->chatroom=0;
		return;
	}
	memset(fa,0,1024);
	strcpy(fa,"连接成功");
	send(fd,fa,1024,0);
	while(1)
	{
		memset(shou,0,1024);
		recv(fd,shou,1024,0);
		if(strcmp(shou,"over")==0)
		{
			p->chatroom=0;
			memset(fa,0,1024);
			sprintf(fa,"%s 已退出，请输入over退出",id);
			if(p1->chatroom==1)
				send(p1->fd,fa,1024,0);
			break;
		}
		memset(fa,0,1024);
		sprintf(fa,"%s say:%s",id,shou);
		send(p1->fd,fa,1024,0);
		baocun1();
	}
}

