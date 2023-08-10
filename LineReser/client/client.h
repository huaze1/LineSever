#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<event.h>
#include<sys/socket.h>
#include<jsoncpp/json/json.h>
using namespace std;

//枚举
enum CHO_TYPE{DL=1,ZC, CKYY,YD,XSYY,QXYY,TC};


class Socket_Client{
public:
    Socket_Client(){
        ips="127.0.0.1";
        m_port=6000;
        flg_dl=false;
        cho=-1;
        user_name="游客";
        user_tel="";
    }
    Socket_Client(string ip,short port){
        ips=ip;
        m_port=port;
        user_name="游客";
        user_name="";
    }
    //连接服务器
    bool Connect_Sever(){
        sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(sockfd==-1){
            return false;
        }
        struct sockaddr_in saddr;
        saddr.sin_family=AF_INET;
        saddr.sin_port=htons(m_port);
        saddr.sin_addr.s_addr=inet_addr(ips.c_str());
        if(connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))==-1){
            cout<<"connect err"<<endl;
            return false;
        }
        return true;
    }
    void run();
    
private:
    //界面提示
    void show_info();
    void Zc_user();
    void Dl_user();
    void Ck_yuyue();
    void Yd_ticket();
    void Xsyd_ticket();
    void Qxyy_ticket();

    
    bool flg_dl;//登陆标志
    string user_name;
    string user_tel;
    ///string user_password;

    int cho;//用户的选择
    string ips;
    short m_port;
    int sockfd;

    map<int,int> map_tk;//票的映射关系
    map<int,int> map_my_tk;
};