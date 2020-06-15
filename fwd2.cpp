void * handleclient(void * newfd)
{
	int fd=(int)newfd;          
	char recvbuf[1024];
	char recvbuf1[1024];
	char sendbuf[1024];
	memset(recvbuf,0,sizeof(recvbuf));
	memset(recvbuf1,0,sizeof(recvbuf1));
	memset(sendbuf,0,1024);	
	if(recv(fd,recvbuf,1024,0)==-1)
	{
		perror("recv");						//错误提示
		return;
	}
	else
	{
		printf("cli say:%s\n",recvbuf);     //tcp三次握手规则，回复客户端
	}
	while(1)
	{
		memset(sendbuf,0,1024);
		//登录后界面
		strcpy(sendbuf,"欢迎使用本服务，可使用功能如下\n\t1.登录帐号\n\t2.注册帐号\n\tend.退出\n");
		send(fd,sendbuf,strlen(sendbuf),0);		
		memset(recvbuf,0,sizeof(recvbuf));		
		recv(fd,recvbuf,1024,0);
		if(strcmp(recvbuf,"1")==0)     //接收到客户端输入为1，请输入登录账号
		{
			memset(sendbuf,0,1024);
			strcpy(sendbuf,"请输入登录账号");             //要求客户端输入账号密码
			if(send(fd,sendbuf,strlen(sendbuf),0)==-1)
			{
				perror("send");
				return;
			}	
			memset(recvbuf,0,sizeof(recvbuf));
			if(recv(fd,recvbuf,1024,0)==-1)
			{
				perror("recv");
				return;
			}
			memset(sendbuf,0,sizeof(sendbuf));	
			strcpy(sendbuf,"请输入登录密码");
			if(send(fd,sendbuf,strlen(sendbuf),0)==-1)
			{
				perror("send");
				return;
			}	
			memset(recvbuf1,0,sizeof(recvbuf1));
			if(recv(fd,recvbuf1,1024,0)==-1)
			{
				perror("recv");
				return;
			}	
			//匹配账号密码是否都存在且正确
			if(yzzh(recvbuf)==0||yzmm(recvbuf,recvbuf1)==0)
			{
				memset(sendbuf,0,1024);
				strcpy(sendbuf,"输入账号或密码不正确");
				send(fd,sendbuf,strlen(sendbuf),0);
			}
			else if(zhzx(recvbuf)==1)
			{
				memset(sendbuf,0,1024);
				strcpy(sendbuf,"此帐号在线");
				send(fd,sendbuf,strlen(sendbuf),0);
			}
			else
			{
				strcpy(recvbuf1,recvbuf);
				memset(sendbuf,0,1024);
				strcpy(sendbuf,"登陆成功\n");
				X*p;
				p=head;
				while(p->next)
				{
					if(strcmp(p->next->id,recvbuf1)==0)
					{
						p->next->online=1;
						p->next->fd=fd;
						break;
					}
					p=p->next;
				}
				send(fd,sendbuf,strlen(sendbuf),0);
				//登陆时判断是否有新消息，有新消息发送一个提示给客户端
				M* p9;
				p9=head1;
				int le=0;
				while(p9)
				{
					if(strcmp(p9->id,recvbuf1)==0)
					{						
						le=1;
						memset(sendbuf,0,1024);
						strcpy(sendbuf,"您有新消息，请输入open查看\n");
						send(fd,sendbuf,strlen(sendbuf),0);
						break;
					}
					p9=p9->next;
				}																			
				//登录后可以使用的功能
				while(1)
				{
					memset(sendbuf,0,1024);
					strcpy(sendbuf,"你可以使用的功能如下\n\t1.查找好友\n\t2.显示好友\n\t3.删除好友\n\t4.添加好友\n\t5.查看或修改个人信息\n\t6.与好友发送私信\n\t7.群聊\n\tchat.与好友聊天窗口(屏幕分区输入输出)\n\t10.在线查看聊天记录\n\t11.从服务器下载聊天记录\n\t8.退出\n\topen.查看消息");
					send(fd,sendbuf,strlen(sendbuf),0);					
					memset(recvbuf,0,sizeof(recvbuf));
					if(recv(fd,recvbuf,1024,0)==-1)
					{
						perror("recv");
						return;
					}
					if(strcmp(recvbuf,"1")==0)//查找好友
					{
						look(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"2")==0)//显示好友列表
					{
						display(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"3")==0)//删除好友
					{
						del(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"4")==0)//添加好友
					{
						addhy(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"5")==0)//查看或修改个人信息
					{
						xgnc(fd,recvbuf1);
					}
					else if(strcmp(recvbuf,"6")==0)//与好友发送私聊信息
					{
						siliao(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"7")==0)//进入多人聊天室
					{
						duorenliao(fd,recvbuf1);
					}
					else if(strcmp(recvbuf,"8")==0)//退出，并把在线信息置零，套接字值置为-5
					{						
						p=head;
						while(p->next)
						{
							if(strcmp(p->next->id,recvbuf1)==0)
							{
								p->next->online=0;
								p->next->fd=-5;
								break;
							}
							p=p->next;
						}
						break;
					}
					else if(strcmp(recvbuf,"chat")==0) //双人实时聊天，
					{
						doublechat(fd,recvbuf1);						
					}
					else if(strcmp(recvbuf,"10")==0)//发送聊天记录给客户端
					{
						jilu(fd,recvbuf1);												
					}
					else if(strcmp(recvbuf,"11")==0)//客户端接受聊天记录文件
					{
						xiazai(fd,recvbuf1);
					}
					else if(strcmp(recvbuf,"open")==0)//打开消息
					{						
						char fa[1024];
						char shou[1024];
						M* m;
						int lea=0;
						m=head1->next;
						printf("count1=%d\n",count1);
						while(m)
						{						
							if(strcmp(m->id,recvbuf1)==0)
							{
								lea=1;//标识符	，提示缓存消息列表里有没有此人消息							
								if(m->type==1)//消息类型，1为好友请求消息
								{
									memset(fa,0,1024);
									strcpy(fa,m->mess);
									send(fd,fa,strlen(fa),0);
									while(1)
									{	
										memset(shou,0,1024);
										recv(fd,shou,1024,0);
										if(strcmp(shou,"y")==0)  //如果同意加为好友，并告诉对方
										{									
											m->type=3;
											strcpy(m->id,m->id1);
											strcpy(m->id1,recvbuf1);
											memset(fa,0,1024);
											sprintf(fa,"%s已同意添加，添加成功",recvbuf1);
											strcpy(m->mess,fa);
											
											X* p,*p1;
											p=head->next;
											p1=head->next;
											
											while(p)
											{
												if(strcmp(p->id,m->id)==0)
													break;
												p=p->next;
											}
											while(p1)
											{
												if(strcmp(p1->id,m->id1)==0)
													break;
												p1=p1->next;
											}											
											strcpy(p->hy[p->hys],p1->id);
											p->hys++;
											strcpy(p1->hy[p1->hys],p->id);
											p1->hys++;
											if(zhzx(m->id)==1)
											{
												memset(fa,0,1024);
												strcpy(fa,"您有新消息，请输入open查看");
												send(p->fd,fa,strlen(fa),0);
											}
											baocun();
											baocun1();
											break;											
										}
										else if(strcmp(shou,"n")==0)   //不同意并告诉对方
										{
											X* p;
											p=head->next;								
											while(p)
											{
												if(strcmp(p->id,m->id1)==0)
													break;
												p=p->next;
											}
											m->type=3;
											strcpy(m->id,m->id1);
											strcpy(m->id1,recvbuf1);
											memset(fa,0,1024);
											sprintf(fa,"%s已拒绝添加请求，添加失败",recvbuf1);
											strcpy(m->mess,fa);
											if(zhzx(m->id)==1)
											{
												memset(fa,0,1024);
												strcpy(fa,"您有新消息，请输入open查看");
												send(p->fd,fa,strlen(fa),0);
											}
											baocun1();
											break;
										}										
									}
								}
								else if(m->type==3)//处理普通消息，看完消息，链表中删除
								{
									lea=1;
									memset(fa,0,1024);
									strcpy(fa,m->mess);
									send(fd,fa,strlen(fa),0);
									M*m1,*m2;
									m1=head1;
									while(m1->next)
									{
										if(strcmp(m1->next->id,recvbuf1)==0)
										{
											m2=m1->next;
											m1->next=m2->next;
											free(m2);
											break;
										}
										m1=m1->next;
									}
									baocun1();
									count1--;
									if(m==NULL);
									break;									
								}
								else if(m->type==2)//消息类型2，聊天消息
								{   lea=1;
									M* p7;
									p7=head1->next;
									while(p7)
									{
										int ppp=0;   //标识符，有没有此人消息
										if((strcmp(p7->id,recvbuf1)==0)&&(p7->type==2))
										{
											memset(fa,0,1024);
											strcpy(fa,p7->mess);
											send(fd,fa,strlen(fa),0);
											///////////////////
											baocun2(fa,recvbuf1);
											M *m1,*m2;
											m1=head1;											
											while(m1->next)
											{
												if((strcmp(m1->next->id,recvbuf1)==0)&&(m1->next->type==2))
												{
													m2=m1->next;
													m1->next=m2->next;
													free(m2);
													ppp=1;
													break;
												}
												m1=m1->next;
											}											
										}
										if(ppp==1)
										{
											count1--;
											baocun1();
											p7=head1->next;
											continue;
										}
										p7=p7->next;
									}									
								}								
							}
							if(m==NULL)
								break;
							m=m->next;
						}
						if(lea==0)
						{
							memset(fa,0,1024);
							strcpy(fa,"您没有任何未处理消息");
							send(fd,fa,strlen(fa),0);														
						}						
					}
				}
			}
		}
		else if(strcmp(recvbuf,"2")==0)
		{
			add(fd);
			
			
		}
		else if(strcmp(recvbuf,"end")==0)
		{
			memset(sendbuf,0,1024);
			strcpy(sendbuf,"欢迎使用，谢谢");
			send(fd,sendbuf,strlen(sendbuf),0);
			break;
		}
		else 
		{
			continue;
		}			
	}	
	close(fd);	
}
//判断客户端是否依旧在线
void * panduan(void)
{
	X* p;
	char fa[1024];
	p=head;
	while(1)         //判断是否断开连接，如果断开将此账号置为不在线状态，监听套接字-5
	{
		p=head->next;
		while(p)
		{
			if(p->online==1)
			{				
				struct tcp_info info;
				int len=sizeof(info);         
				getsockopt(p->fd,IPPROTO_TCP,TCP_INFO,&info,(socklen_t*)&len);
				if((info.tcpi_state!=TCP_ESTABLISHED))
				{
					p->online=0;
					p->fd=-5;
				}				
			}
			p=p->next;
		}
	}
	
}
 
int main()
{
	int sockfd=0,newfd=0;    //定义并初始化
	int chlid=0;
	int ret=0;
	int len=sizeof(struct sockaddr);
	
	struct sockaddr_in myaddr;
	struct sockaddr_in otheraddr;
	memset(&myaddr,0,len);
	memset(&otheraddr,0,len);	
	//tcp套接字连接
	/*if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("sockfd");
		return -1;	
		}
	else 
	{
		printf("socket success...%d\n",sockfd);		
	}*/
	//实现端口复用
	int reuse = 1;
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopet error\n");
        return -1;
    }
	//初始化结构体
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(8889);
	myaddr.sin_addr.s_addr=inet_addr("192.168.194.129");
	//myaddr.sin_addr.s_addr=INADDR_ANY;，可以自动获取本机ip地址
	//绑定套接字
	if(-1==(bind(sockfd,(struct sockaddr*)&myaddr,len)))
	{
		perror("bind");         //错误提示
		return -1;		
	}
	else
	{
		printf("bind success...\n");		
	}
	//开始侦听
	if((listen(sockfd,10))==-1)
	{
		perror("listen");
		return -1;		
	}
	else
	{
		printf("listen success...\n");
	}	
	printf("server waiting...\n");
	pthread_t id1,id2;
	head=create();         //创建账号信息的头结点
	head1=create1();		//创建缓存信息的头结点
	while(1)
	{
		if((newfd=accept(sockfd,(struct sockaddr*)&otheraddr,&len))==-1)
		{
			perror("accept");
			return -1;			
		}
		else		//连接成功输出客户端等信息
		{
			printf("accept success...client's fd=%d,sever's fd=%d\n",newfd,sockfd);
			printf("--client ip=%s,port=%d\n",inet_ntoa(otheraddr.sin_addr),ntohs(otheraddr.sin_port));			
		}	
		//线程客户端
		pthread_create(&id1,NULL,handleclient,(void*)newfd);
		//判断客户端是否依旧在线
		pthread_create(&id2,NULL,(void*)panduan,NULL);
 
		pthread_detach(id1);//分离线程，线程结束后资源系统自动回收
		pthread_detach(id2);
	}	
	close(sockfd);	
	return 0;
}