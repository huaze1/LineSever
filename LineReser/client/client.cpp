#include"client.h"
void Socket_Client::show_info(){
    if(!flg_dl){
        cout<<"-----用户名：游客--------登陆状态：未登录-------------"<<endl;
        cout<<"            1.登陆                   2.注册"<<endl;
        cout<<"------------------------------------------------------"<<endl;
        cin>>cho;
    }
    else{
        cout<<"-----用户名："<<user_name<<"d登陆状态:已登录--------------------"<<endl;
        cout<<"1.查看预约                2.预定"<<endl;
        cout<<"2.显示我的预约             4.取消预定"<<endl;
        cout<<"5.退出系统"<<endl;
        cout<<"-----------------------------------------------------------"<<endl;
        cin>>cho;
        cho+=2;//每一个操作用唯一的指表示
    }
}

void Socket_Client::Zc_user(){
    string curr_tel;
    string curr_name;
    string curr_password;
    cout<<"请输入用户手机号码"<<endl;
    cin>>curr_tel;
    cout<<"请输入注册用户名"<<endl;
    cin>>curr_name;
    cout<<"请输入用户密码"<<endl;
    cin>>curr_password;
    if(curr_tel.empty()|| curr_name.empty()|| curr_password.empty()){
        cout<<"输入有误"<<endl;
        return;
    }
    Json::Value val;
    val["type"]="ZC";
    val["user_tel"]=curr_tel;
    val["user_name"]=curr_name;
    val["user_password"]=curr_password;

    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);

    char status_buff[128]={0};
    int n=recv(sockfd,status_buff,127,0);
    if(n<=0){
        cout<<"ser close"<<endl;
        return;
    }
    cout<<status_buff<<endl;//测试
    Json::Value res_val;
    Json::Reader Read;
    //if(!Read.parse(status_buff,res_val)){
       // cout<<"无法解析json字符串"<<endl;
       // return;
  //  }
    Read.parse(status_buff,res_val);//反序列化
    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<"注册失败"<<endl;
        return;
    }
    user_name=curr_name;
    user_tel=curr_tel;
    cout<<"注册成功"<<endl;
    flg_dl=true;
}   
void Socket_Client::Dl_user(){
    string curr_tel;
    //string curr_name;
    string curr_password;
    cout<<"请输入用户手机号"<<endl;
    cin>>curr_tel;
    cout<<"请输入用户密码"<<endl;
    cin>>curr_password;
    if(curr_tel.empty() || curr_password.empty()){
        cout<<"输入无效"<<endl;
        return;
    }

    Json::Value val;
    val["type"]="DL";
    val["id_tel"]=curr_tel;
    val["user_password"]=curr_password;

    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);

    char status_buff[128]={0};
    int num =recv(sockfd, status_buff,127,0);
    if(num<=0){
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    //服务器返回的json有问题
    if(!Read.parse(status_buff,res_val)){
        cout<<"无法解析json字符串"<<endl;
        return;
    }
    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<"登陆失败"<<endl;
        return;
    }
    cout<<"登陆成功"<<endl;
    flg_dl=true;
    user_name=res_val["user_name"].asString();
    user_tel=curr_tel;

}
void Socket_Client::Ck_yuyue(){
    //发送预约信息
    Json::Value val;
    val["type"]="CKYY";
    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
    char res_buff[1024]={0};
    if(recv(sockfd,res_buff,1023,0)<=0){
        cout<<"ser close"<<endl;
        return;
    }
    //cout<<res_buff<<endl;
    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff,res_val)){//反序列化
        cout<<"json 无法解析"<<endl;
        return;
    }
    //票据映射map<int,int>
    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<"查看预约失败"<<endl;
        return;
    }
    int num=res_val["num"].asInt();
    if(num<=0){
        cout<<"暂时没有预约信息"<<endl;
        return;
    }

    //res_val["ticket_arr"].size();
    //也可以获取票数信息

    //序号 地点  总票数 
    cout<<"---------------------------------------------"<<endl;
    cout<<"|  序号  | 地点名称 |  票数  |  已预约  |  日期  |"<<endl;

    map_tk.clear();//清空之前的
    for(int i=0;i<num;i++){
        map_tk.insert(make_pair(i,res_val["ticket_arr"][i]["tk_id"].asInt()));
        cout<<"   "<<i<<"     ";
        cout<<res_val["ticket_arr"][i]["tk_name"].asString()<<"        ";
        cout<<res_val["ticket_arr"][i]["tk_max"].asInt()<<"       ";
        cout<<res_val["ticket_arr"][i]["tk_count"].asInt()<<"       ";
        cout<<res_val["ticket_arr"][i]["tk_data"].asString()<<endl;
        cout<<"---------------------------------------------------"<<endl;
    }
}

void Socket_Client::Yd_ticket(){
    cout<<"请输入要预定的序号"<<endl;
    int num = -1;
    cin>>num;

    map<int,int>::iterator pos=map_tk.find(num);
    if(pos == map_tk.end()){
        cout<<"输入无效"<<endl;
        return;
    }
    int tk_id=pos->second;

    Json::Value val;
    val["type"] = "YD";
    val["id_tel"] = user_tel;
    val["tk_id"] = tk_id;

    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);

    char status_buff[128]={0};
    if(recv(sockfd,status_buff,127,0)<=0){
        cout<<"ser close"<<endl;
        return;
    }
    Json::Value res_val;;
    Json::Reader Read;
    if(!Read.parse(status_buff,res_val)){
        cout<<"json 无法解析"<<endl;
        return;
    }

    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<"预定失败"<<endl;
        return;
    }
    cout<<"预定成功"<<endl;
}
//查询预约
void Socket_Client::Xsyd_ticket(){
    Json::Value val;
    val["type"]="XSYY";
    val["id_tel"]=user_tel;

    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);

    char res_buff[1024]={0};
    int n=recv(sockfd,res_buff,1023,0);
    if(n<=0){
        cout<<"ser close"<<endl;
        return;
    }
    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff,res_val)){
        cout<<"解析json失败"<<endl;
        return;
    }
    //判断状态
    //
    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<"查询我的预约失败"<<endl;
        return;
    }
    int num=res_val["num"].asInt();
    if(num<=0)  {
        cout<<"当前用户没有预约信息"<<endl;
        return;
    }
    cout<<"当前用户共预约"<<num<<"条信息"<<endl;
    cout<<"----------------------------------<<endl";
    cout<<"序号      预约信息        "<<endl;
    map_my_tk.clear();//每次插入值之前都清空容器
    for(int i=0;i<num;i++){
        map_my_tk.insert(make_pair(i,res_val["yd_arr"][i]["yd_id"].asInt()));
        cout<<"i"<<"    "<<res_val["yd_arr"][i]["tk_name"].asString()<<endl;
    }
}
void Socket_Client:: Qxyy_ticket(){
    cout<<"请输入要取消的编号"<<endl;
    int index;
    cin>>index;

    map<int,int>::iterator pos=map_my_tk.find(index);
    if(pos==map_my_tk.end()){
        cout<<"输入无效，没找到相关的信息"<<endl;
        return;
    }
    int yd_id=pos->second;
    
    Json::Value val;
    val["type"]="QXYY";
    val["yd_id"]=yd_id;

    send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);

    
    char res_buff[128]={0};
    int n=recv(sockfd,res_buff,127,0);
    if(n<=0){
        cout<<"ser close"<<endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if(!Read.parse(res_buff,res_val)){
        cout<<"json无法解析"<<endl;
        return;
    }

    string status_str=res_val["status"].asString();
    if(status_str.compare("OK")!=0){
        cout<<" 取消预定失败"<<endl;
        return;
    }

    cout<<"取消预定成功"<<endl;
    return;
}
void Socket_Client::run(){
    bool running =true;
    while(running){
        show_info();
        switch (cho)
        {
        //枚举表示选择类型
        // enum CHO_TYPE{DL=1,ZC, CKYY,YD,XSYY,QXYY,TC};
        case DL:
            //cout<< "DL"<<endl;
            Dl_user();
            break;
        case ZC:
            Zc_user();
            //cout<< "ZC"<<endl;
            break;
        case CKYY:
            Ck_yuyue();
            //cout<< "CKYY"<<endl;
            break;
        case YD:
            Yd_ticket();
            //cout<< "YD"<<endl;
            break; 
        case XSYY:
            Xsyd_ticket();
            //cout<< "XSYY"<<endl;
            break;     
        case QXYY:
            Qxyy_ticket();
            //cout<< "QXYY"<<endl;
            break;
        case TC:
            cout<<"TC"<<endl;
            running=false;
            break;   
        default:
            break;
        }
    }
    /*
   // 测试
        //定义json对象//json对象赋值
        //json对象序列化成字符串
        Json::Value val;//定义json对象
        //给json对象传值
        val["type"]="DL";
        val["user_tel"]="139999999";
        val["user_password"]="123456";

        //把json对象序列化为字符串发送过去
        send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
        
        char buff[128]={0};
        recv(sockfd,buff,127,0);//收一个json字符串
        Json::Reader Read;
        Json::Value res;
        Read.parse(buff,res);//json字符串反序列化json对象
        cout<<res["status"].asString()<<endl;//“ok"
        */
    //}
}

int main(){

    Socket_Client cli;
    if(!cli.Connect_Sever()){
        exit(1);
    }
    cli.run();
    //cli.show_info();

    exit(0);
}