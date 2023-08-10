#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<event.h>
#include<sys/socket.h>
#include<jsoncpp/json/json.h>
#include<mysql/mysql.h>

using namespace std;

const int listen_max=5;
//枚举
enum CHO_TYPE{DL=1,ZC, CKYY,YD,XSYY,QXYY,TC};
//回调函数
void CallBack_Fun(int fd,short ev,void *arg) ;

class My_lib;

//监听到套接字的创建
class Ser_Socket
{
public:
    Ser_Socket(){
        m_port=6000;
        ips="127.0.0.1";
    }
    Ser_Socket(short port, string ips){
        m_port=port;
        this->ips=ips;
    }
    //返回套接字
    int Socket_Init(){
        sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(sockfd==-1)  return -1;
        struct sockaddr_in saddr;
        memset(&saddr,0,sizeof(saddr));
        saddr.sin_family=AF_INET;
        saddr.sin_port=htons(m_port);
        saddr.sin_addr.s_addr=inet_addr(ips.c_str());
        int res=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
        if(res==-1){
            cout<<"bind err<<endl";
            return -1;
        }
        res=listen(sockfd,listen_max);
        if(res==-1) return-1;
        return sockfd;
    }
    //写一个获取sockfd的函数
    int Get_socketfd()const {
        return sockfd;
    }
private:
    int sockfd;
    short m_port;
    string ips;
};


//用多态来处理不同描述符
//基类
class Call_Back_Sock{
public:
    virtual void Call_Back_Fun()=0;

    struct event* ev;

};
///监听套接字
class Call_Back_Accept:public Call_Back_Sock{
public:
//类在定义的时候传入一个描述符来判断是哪个描述符调用相应的class
    Call_Back_Accept(int fd):sockfd(fd){
        plib=NULL;
    }
    void Set_lib(My_lib* plib){
        this->plib=plib;
    }
    void Call_Back_Fun();
private:
    int sockfd;//监听套接字
    My_lib *plib;

};
///连接套接字
class Call_Back_Recv:public Call_Back_Sock{
public:
    Call_Back_Recv(int fd):c(fd){
        m_map.insert(make_pair("ZC",ZC));
        m_map.insert(make_pair("DL",DL));
        m_map.insert(make_pair("CKYY",CKYY));
        m_map.insert(make_pair("YD",YD));
        m_map.insert(make_pair("XSYY",XSYY));
        m_map.insert(make_pair("QXYY",QXYY));
    }
    void Call_Back_Fun();
    ~Call_Back_Recv(){
        close(c);
    }
    //
    void Send_ERR();
    void Send_OK();
    void ZC_user();
    void DL_user();
    void CK_yuyue();
    void Yd_ticket();
    void Xsyy_ticket();
    void Qxyd_ticket();
private:
    int c;//连接套接字
    Json::Value m_val;
    Json::Reader m_read;//反序列化对象S
    map<string,enum CHO_TYPE> m_map;
};


//封装json


//封装libevent
class My_lib{
    public:
    My_lib(){
        base=nullptr;
    }
    //lib核心框架event_int，添加事件启动事件循环
    //定义base
    bool Event_Base_Init(){
        base=event_init();
        if(base==nullptr){
            return false;
        }
        return true;
    }
    //libevent
    //添加事件
    bool Event_Add(int fd,Call_Back_Sock*ptr){
        //在移除事件时需要*ev指针，但是ev指针是临时变量
        struct event*   ev=event_new(base,fd,EV_READ|EV_PERSIST,CallBack_Fun,ptr);
        if(ev==NULL) return false;
        if(event_add(ev,NULL)==-1)  return false;
        ptr->ev=ev;//该描述符对应的所投入词条 俄vent事件指针记录
        
        return true;
    }
    //事件循环
    bool Event_Base_Dispatch(){
        if(base!=NULL){
            if(event_base_dispatch(base)==0){
                return true;
            }
        }
        return false;
    }
    
    private:
    struct event_base *base;

};



class Mysql_CLient{
public:
    Mysql_CLient(){
        db_ips="127.0.0.1";
        db_port=3306;
        db_username="hua";
        db_name="LineReser";
        db_passwd="521589";
    }
    Mysql_CLient(string ips,short port,string dbname,string username,string passwd){
        db_ips=ips;
        db_port=port;
        db_username=username;
        db_name=dbname;
        db_passwd=passwd;
    }
    bool Init_Connect();//初始化连接
    void closemysql();
    void Mysql_RollBack();
    void Mysql_Begin();
    void Mysql_Commit();

    //业务方面的功能
    bool Db_Zc_user(string &tel,string &name,string &pw);
    bool Db_Dl_user(string &tel,string &name,string &pw);
    bool Db_Ck_yuyue(Json:: Value& ck_val);//给用户返回
    bool Db_Yd_ticket(string & tel,int tk_id); 
    bool Db_Xsyy_ticket(string& tel,Json::Value &res_val);
    bool Db_Qxyy_ticket(const int yd_id);
private:
    string db_ips;//mysql server ip
    short db_port;
    string db_name;
    string db_username;
    string db_passwd;

    MYSQL mysql_con;//mysql句柄

    map<int,int> map_tk;

};
