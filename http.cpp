#include <iostream>
#include "winsock2.h"
#include "stdio.h"
#pragma comment(lib, "ws2_32.lib")
#include <sys/types.h>
#include <sys/stat.h>

#define PRINTF(str)   printf("[%s-%d]"#str"=%s\n",__func__,__LINE__,str);
//ʵ�������ʼ��
//�����׽���
//ָ���˿�
int startup(unsigned short* port)
{
    //to do
    WSADATA data;
    int ret=WSAStartup(MAKEWORD(1,1),//1,1�汾
               &data);
    if(ret){//����ʧ��
        fprintf(stderr,"��ʼ��ʧ��");
        return -1;
    }

    int server_socket=socket(PF_INET, //�׽�������
     SOCK_STREAM,//����
     IPPROTO_TCP
    );
    if(server_socket==-1)
    {
        fprintf(stderr,"�׽���ʧ��");
        return -1;
    }

    //�����׽��ֶ˿ڿɸ���
    int opt= 1;
    ret=setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));
    if(ret==-1){//����ʧ��
        fprintf(stderr,"���ö˿�����");
        return -1;
    }



    struct sockaddr_in sever_addr;
    memset(&sever_addr,0,sizeof(server_socket));
    sever_addr.sin_family=AF_INET;
    sever_addr.sin_port= htons(*port);
    sever_addr.sin_addr.S_un.S_addr= htonl(INADDR_ANY);

    //���׽���
     if(bind(server_socket,(const struct sockaddr*)&sever_addr ,sizeof(sever_addr))<0)
   {
       fprintf(stderr,"bindʧ��");
       return -1;
   }
    //��̬����˿�
    int namelen=sizeof(sever_addr);
    if(*port==0) {
        if (getsockname(server_socket, (struct sockaddr *) &sever_addr, &namelen) < 0) {
            fprintf(stderr, "�������˿� error");
            return -1;
        }
        *port=sever_addr.sin_port;
    }

     //������������
    if( listen(server_socket,5)<0)
    {
        fprintf(stderr,"listen error");
        return -1;
    }
    return server_socket;
}


//����ʵ�ʶ�ȡ�����ֽ�
 int getLine(int client,char*buf,int size)
 {
    char c='0';
    int i=0;
    while (i<size-1&&c!='\n')
    {
        int n=recv(client,&c,1,0);
        if(n>0)
         {
            if(c=='\r')
            {
               n = recv(client,&c,1,MSG_PEEK);//���ӻ�����ȡ��--MSG_PEEK
               if(n>0&&c=='\n')
               {
                   recv(client,&c,1,0);
               }
               else
               {
                   c='\n';
               }
            }
             buf[i++]=c;
         }
         else
         {
             //to do...
             c='\n';
         }
    }
    buf[i]='\0';
    return i;
 }

void  unimplement(int client)
{
    //��ָ���׽��֣�����һ����û��ʵ�ֵĴ���ҳ��

}

void not_found( int client)
{

}

void headers(int client)
{
    char buf[1024];
    strcpy(buf,"HTTP/1.0 200 OK\r\n");
    send(client,buf,strlen(buf),0);

    strcpy(buf,"Sever: LiYao/0.1\r\n");
    send(client,buf,strlen(buf),0);

    strcpy(buf,"Content-type:text/html\n");
    send(client,buf,strlen(buf),0);

    strcpy(buf,"\r\n");
    send(client,buf,strlen(buf),0);
}


void cat(int client,FILE*resource)
{
    char buf[4096];
    while (1)
    {

        int ret=fread(buf,sizeof(char),sizeof(buf),resource);
        if(ret<=0) break;
        send(client,buf,ret,0);
    }

}

void server_file(int client,const char*path)
{

    int numchars=1;
    char buf[1024];
    //��ʣ�����󱨶���
    while (numchars>0 && strcmp(buf,"\n")) {
        numchars = getLine(client, buf, sizeof(buf));
        PRINTF(buf);
    }


    FILE *resource=NULL;
    if(strcmp(path,"htdocs/index.html")==0)
    {
        resource= fopen(path,"r");
    }
    else
    {
        resource=fopen(path,"rb");
    }
    if(resource==NULL)
    {
        not_found(client);
    }
    else
    {

        //����ͷ��Ϣ
        headers(client);

        //����������Դ��Ϣ
        cat(client,resource);
        printf("��Դ�������");
    }
    fclose(resource);
}

DWORD WINAPI  accept_request(LPVOID arg)
{
    char buf[1024];
    //��ȡһ������
    int client = (SOCKET)arg;
    int numchars = getLine(client,buf,sizeof(buf));
    PRINTF(buf);
    char method[255];
    int i=0,j=0;
    while(!std::isspace(buf[j])&&i<sizeof(method)-1)
    {
        method[i++]=buf[j++];
    }
    method[i]='\0';
    PRINTF(method);

    //������󷽷����������Ƿ�֧��
    if(strcmp(method,"GET")&&strcmp(method,"POST"))
    {
        //���ش���
        unimplement(client);
        return 0;
    }
    //������Դ�ļ���·��
    char  url[255];//�����Դ���������·��
    i=0;
    while( std::isspace(buf[j])&&i<sizeof(method)-1)
    {
        j++;
    }
    while(!std::isspace(buf[j])&&i<sizeof(url)-1&&j<sizeof(buf))
    {
        url[i++]=buf[j++];
    }
    url[i]=0;
    PRINTF(url);

    char path[512]="";
    sprintf(path,"htdocs%s",url);
    if(path[strlen(path)-1]=='/')
    strcat(path,"index.html");
    PRINTF(path);

    struct stat status;

    //������ȡָ���ļ����Žݼѵ���Ϣ
    if(stat(path,&status)==-1)
    {
        while (numchars>0 && strcmp(buf,"\n"))
        numchars=getLine(client,buf,sizeof(buf));
        not_found(client);

    }
    else
    {
        if((status.st_mode & S_IFMT) == S_IFDIR)
        {
            strcat(path,"/index.html");
        }
        server_file(client,path);
    }


    closesocket(client);


    return 0;
}
int main()
{
    unsigned short port=80;
    int  server_sock=startup(&port);
    std::cout<<"httpd�����Ѿ����������ڼ���"<<port<<"�˿�"<<std::endl;
    //�ȴ��ͻ��˷������
    struct sockaddr_in client_addr;
    int  client_addr_len=sizeof(client_addr);
    while(1)
    {
        //����ʽ�ĵȴ��û�ͨ��������������
        int client_sock=accept(server_sock,(struct sockaddr*)&client_addr,&client_addr_len);
        if(client_sock==-1)
        {
            fprintf(stderr,"accept error");
            exit(0);
        }

        //���û�����
        //to do...
        //�����߳̿�����߲�����
        DWORD threadID;
        CreateThread(0,0,
                     accept_request,
                     (void*)client_sock,0,
                     &threadID);

    }
    //to do
    closesocket(server_sock);
    return 0;

}