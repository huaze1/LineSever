#include"server.h"
void CallBack_Fun(int fd,short ev,void *arg) {
    //if(arg==nullptr)    return;
    Call_Back_Sock *ptr=(Call_Back_Sock*)arg;
    if(ptr==nullptr)    return;
    if(ev &EV_READ){
        ptr->Call_Back_Fun();//调用对应的fd
    }
}

//sockfd（监听套接字）有客户端连接调用
void Call_Back_Accept::Call_Back_Fun(){
    int c=accept(sockfd,NULL,NULL);//
    if(c<0) return;
    printf("accept c=%d\n",c);

    Call_Back_Recv *p=new Call_Back_Recv(c);
    if(p==NULL)  {
        close(c);    
        delete p;
        return;
    }
    plib->Event_Add(c,p);
}
void Call_Back_Recv::Send_ERR(){
    Json::Value res_val;
    res_val["status"]="ERR";
    send(c,res_val.toStyledString().c_str(),strlen(res_val.toStyledString().c_str()),0);
}
void Call_Back_Recv::Send_OK(){
    Json::Value res_val;
    res_val["status"]="OK";
    send(c,res_val.toStyledString().c_str(),strlen(res_val.toStyledString().c_str()),0);

}
//初始化连接
bool Mysql_CLient::Init_Connect(){
    if(mysql_init(&mysql_con) ==NULL){
        return false;
    }
    if(mysql_real_connect(&mysql_con,db_ips.c_str(),db_username.c_str(),
    db_passwd.c_str(),db_name.c_str(),db_port,NULL,0)==NULL){
        cout<<"mysql connect err"<<endl;
        return false;

    }
    return true;
}
//关闭连接
void Mysql_CLient::closemysql(){
    mysql_close(&mysql_con);
}
void Mysql_CLient::Mysql_RollBack(){
    if(mysql_query(&mysql_con, "rollback") != 0){
        cout<<"事务回滚"<<endl;
    }
}
void Mysql_CLient::Mysql_Begin(){
    string str_begin = "begin";
    if(mysql_query(&mysql_con, str_begin.c_str())!=0){
        cout<<"开始事务失败"<<endl;
    }
}
void Mysql_CLient::Mysql_Commit(){
    string str_commit = "commit";
    if(mysql_query(&mysql_con, str_commit.c_str())!=0){
        cout<<"开始事务提交失败"<<endl;
    }
}

bool Mysql_CLient::Db_Zc_user(string &tel,string &name,string &pw){
//insert into user_info values("139000000","xiaowang","123456",1);
    string sql=string("insert into user_info values('")
    +tel+string("','")+name+string("','")+pw+string("',1)");
    if(mysql_query(&mysql_con,sql.c_str())!=0){
        cout<<"query err"<<endl;
        return false;
    }
    return true;
}


bool Mysql_CLient::Db_Dl_user(string &tel,string &name, string &pw){
    //select user_name,user_password from user_info where id_tel=139999999
    string sql=string("select user_name,user_password from user_info where id_tel=")+tel;     
    if(mysql_query(&mysql_con,sql.c_str())!=0){
        return false;
    }
    MYSQL_RES *r =mysql_store_result(&mysql_con);
    if(r==NULL) return false;
    //查看结果集有多少行
    int rows=mysql_num_rows(r);
    int count=mysql_field_count(&mysql_con);
    if(rows!=1 || count != 2){
        cout<<"db res : row = "<<rows<<" count:"<<count<<endl;
        return false;
    }
    //获取一行记录
    MYSQL_ROW row=mysql_fetch_row(r);
    name=row[0];//用户名
    pw=row[1];//密码
    //释放结果集
    mysql_free_result(r);
    return true;
}
bool Mysql_CLient::Db_Ck_yuyue(Json::Value &val)
{
    if (!Init_Connect())
    {
        cout << "连接数据库失败" << endl;
        return false;
    }

    string sql = "select * from ticket_table";
    if (mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        cout << "sql 查询失败" << endl;
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if (r == NULL)
    {
        cout << "读取结果集失败" << endl;
        return false;
    }

    int num = mysql_num_rows(r);
    if (num != 0)
    {
        for (int i = 0; i < num; i++)
        {
            Json::Value tmp;
            MYSQL_ROW row = mysql_fetch_row(r);
            tmp["tk_id"] = stoi(row[0]);
            tmp["tk_name"] = row[1];
            tmp["tk_max"] = stoi(row[2]);
            tmp["tk_count"] = stoi(row[3]);
            tmp["tk_data"] = row[4];
            val["ticket_arr"].append(tmp);
        }
    }

    mysql_free_result(r); // 释放结果集

    val["status"] = "OK";
    val["num"] = num;

    return true;
}

bool Mysql_CLient::Db_Yd_ticket(string &tel,int tk_id){
    if (! Init_Connect())
    {
        cout << "数据库连接失败" << endl;
        return false;
    }

    // 先获取已经预订的票数，和总票数
    string sql = string("select tk_max, tk_count from ticket_table where tk_id=") + to_string(tk_id);
    if (mysql_query(&mysql_con, sql.c_str()) != 0)
    {
        cout << "sql yd err" << endl;
        return false;
    }

    MYSQL_RES *r = mysql_store_result(&mysql_con);
    if (r == NULL)
    {
        cout << "sql mysql_res err" << endl;
        return false;
    }

    int num = mysql_num_rows(r);
    if (num == 0)
    {
        cout << "无法找到要预订的票" << endl;
        mysql_free_result(r);
        return false;
    }
    // max, count
    MYSQL_ROW row = mysql_fetch_row(r);
    int max = atoi(row[0]);
    int count = atoi(row[1]);

    if (count >= max)
    {
        cout << "票已全部预订" << endl;
        return false;
    }

    mysql_free_result(r);

    count++; // 预订一张

    Mysql_Begin(); // 开始事物

    string sql_update = string("update ticket_table set tk_count=") + to_string(count) + string(" where tk_id=") + to_string(tk_id);
    if (mysql_query(&mysql_con, sql_update.c_str()) != 0)
    {
        cout << "update count err" << endl;
        Mysql_RollBack();
        return false;
    }
    cout << "update count 完成" << endl;

    // insert into ticket_res values(0,13900000000,1,"2023-08-05 12:20:56",1);
    string sql_insert = string("insert into ticket_res values(0,") + tel + string(",") + to_string(tk_id) + string(",now(),1)");
    if (mysql_query(&mysql_con, sql_insert.c_str()) != 0)
    {
        cout << "insert ticket_res err" << endl;
        Mysql_RollBack();
        return false;
    }
    Mysql_Commit();//提交事务

    return true;
}

bool Mysql_CLient::Db_Xsyy_ticket(string& tel,Json::Value &res_val){
    string sql("select yd_set.yd_id,ticket_table.tk_name from yd_Set,ticket_table");
    if(mysql_query(&mysql_con,sql.c_str())!=0){
        return false;
    }    
    MYSQL_RES* r=mysql_store_result(&mysql_con);
    if(r==NULL){
        return false;
    }
    int num=mysql_num_rows(r);
    int count=mysql_field_count(&mysql_con);
    if(num<0 || count!=2 ){
        cout<<"没有数据集"<<endl;
        return false;
    }
    for(int i=0;i<num;++i){
        MYSQL_ROW row=mysql_fetch_row(r);
        Json::Value tmp;
        tmp["yd_id"]=stoi(row[0]);
        tmp["tk_name"]=row[1];
        res_val["yd_id"].append(tmp); 
    }
    mysql_free_result(r);
    return true;
}
bool Mysql_CLient::Db_Qxyy_ticket(const int yd_id){
    //select tk_id from yd_set  where yd_id=
    //select tk_count from ticket_table where tk_id=2;
    string sql_tkid=string("select tk_id from yd_set where yd_id=")+to_string(yd_id);
    if(mysql_query(&mysql_con,sql_tkid.c_str())!=0){
        return false;
    }
    MYSQL_RES* r=mysql_store_result(&mysql_con);
    if(r==NULL){
        return false;
    }
    int num = mysql_num_rows(r);//查看有多少行
    int count=mysql_field_count(&mysql_con);//多少列
    if(num != 1 || count != 1){
        mysql_free_result(r);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r);
    string tk_id=row[0];
    mysql_free_result(r);
    //删除记录
    string    sql_del=string("delete from ticket_table where yd_id=") + to_string(yd_id);
    if(mysql_query(&mysql_con,sql_del.c_str())!=0){
        return false;
    }
    //查询count //select tk_count from ticket_table where tk_id=2;
    string sql_count=string("select tk_count from ticket_table where tk_id=")+tk_id;
    if(mysql_query(&mysql_con,sql_count.c_str())!=0){
        return false;
    }
    r=mysql_store_result(&mysql_con);//mysql_con连接句柄
    if(r==NULL){
        return false;
    }
    num=mysql_num_rows(r);
    count=mysql_field_count(&mysql_con);
    if(num != 1 || count != 1){
        mysql_free_result(r);
        return false;
    }
    row=mysql_fetch_row(r);
    string tk_count_str=row[0];
    int tk_count=stoi(tk_count_str);
    if(tk_count<=0){
        mysql_free_result(r);
        return false;
    }

    tk_count--;
    string sql_up=string("update ticket_table set tk_count=")+to_string(tk_count)
    +string(" where tk_id =")+tk_id;

    if(mysql_query(&mysql_con,sql_up.c_str())!=0){
        return false;
    }

    
    return true;

}
//注册
void Call_Back_Recv::ZC_user(){
    string tel=m_val["user_tel"].asString();
    string name=m_val["user_name"].asString();
    string passwd=m_val["user_password"].asString();
    if(tel.empty() || name.empty() || passwd.empty()){
        Send_ERR();
        return;
    }
    //写入数据
    Mysql_CLient cli;
    cli.Init_Connect();
    if(!cli.Db_Zc_user(tel,name,passwd)){
        Send_ERR();
        return;
    }
    Send_OK();
    cli.closemysql();
}
void Call_Back_Recv::DL_user(){
    string tel=m_val["id_tel"].asString();
    string pw=m_val["user_password"].asString();
    if(tel.empty() || pw.empty()){
        Send_ERR();
        return;
    }
    Mysql_CLient cli;
    if(!cli.Init_Connect()){
        Send_ERR();
    }
    string user_name;
    string db_query_password;
    if(!cli.Db_Dl_user(tel,user_name,db_query_password)){
        Send_ERR();
    }
    if(db_query_password.compare(pw)!=0){
        Send_ERR();
    }
    else{
        Json::Value tmp;
        tmp["status"]="OK";
        tmp["user_name"]=user_name;

        send(c,tmp.toStyledString().c_str(),strlen(tmp.toStyledString().c_str()),0);

    }
    cli.closemysql();
}
void Call_Back_Recv::CK_yuyue(){
    //
    Mysql_CLient cli;
    if(!cli.Init_Connect()){
        Send_ERR();
        return;
    }
    Json::Value ck_val;
    if(!cli.Db_Ck_yuyue(ck_val)){
        Send_ERR();
        return;
    }

    send(c,ck_val.toStyledString().c_str(),strlen(ck_val.toStyledString().c_str()),0);
}
void Call_Back_Recv::Yd_ticket(){
    string tel=m_val["id_tel"].asString();
    int tk_id=m_val["tk_id"].asInt();
    Mysql_CLient cli;
    if(!cli.Init_Connect()){
        Send_ERR();
        return;
    }
    if(!cli.Db_Yd_ticket(tel,tk_id)){
        Send_ERR();
        return;
    }
    Send_OK();
    return ;
}
//显示预定
void Call_Back_Recv::Xsyy_ticket(){
    string user_tel=m_val["id_tel"].asString();
    if(user_tel.empty()){
        Send_ERR();
        return;
    }
    Mysql_CLient cli;
    if(!cli.Init_Connect()){
        Send_ERR();
        return;
    }
    Json::Value res_val;
    if(!cli.Db_Xsyy_ticket(user_tel,res_val)){
        Send_ERR();
        return;
    }

    cli.closemysql();
    send(c,res_val.toStyledString().c_str(),strlen(res_val.toStyledString().c_str()),0);

}

void Call_Back_Recv::Qxyd_ticket(){
    int yd_id=m_val["yd_id"].asInt();
    //查找——tkid
    Mysql_CLient cli;
    if(!cli.Init_Connect()){
        Send_ERR();
        return;
    }
    if(!cli.Db_Qxyy_ticket(yd_id)){
        Send_ERR();
        return;
    }
    Send_OK();
}
//c(连接套接字）有客户端发数据，调用该方法
void Call_Back_Recv::Call_Back_Fun(){
    char buff[256]={0};//json
    int n=recv(c,buff,255,0);
    if(n<=0){
        //对方关闭SS
        event_free(ev);
        close(c);
        delete this;
        //在Call_Back_Accept中new了两次
        //Call_Back_Recv *p=new Call_Back_Recv(c);
        //plib->Event_Add(c,p);调用该类函数时又new了一次
        cout<<"client close"<<endl;
        return;
    }
    cout<<buff<<endl;
    //反序列化

    m_read.parse(buff,m_val);
    string cmd_type=m_val["type"].asString();
    
/*    ;
    Json::Reader    Read;
    if(!Read.parse(buff,m_val)){
        cout<<"json 序列化失败"<<endl;
        Send_ERR();
        return;
    }

    Json::Value val;
    cout<<"类型"<<val["type"].asString();
    cout<<"tel"<<val["user_tel"].asString();
    cout<<"name"<<val["user_name"].asString();
    cout<<"password"<<val["user_password"].asString();
    val.clear();
    val["status"]="OK";
*/
    //send(c,m_val.toStyledString().c_str(),strlen(m_val.toStyledString().c_str()),0);
   // string type_str=m_val["type"].asString();//转成字符串，拿到类型“ZC”（注册）
    map<string,enum CHO_TYPE> ::iterator pos=m_map.find(cmd_type);
    if(pos==m_map.end()){
        Send_ERR();
        return;
    }
    enum CHO_TYPE cho =pos->second;
    //查看要进行什么操作
    switch (cho)
        {
        //枚举表示选择类型
        //enum CHO_TYPE{DL=1,ZC, CKYY,YD,XSYY,QXYY,TC};
      
         case DL:
            DL_user();
            //cout<< "DL"<<endl;
            break; 
         case ZC:
            ZC_user();
            //cout<< "ZC"<<endl;
            break;
        case CKYY:
            CK_yuyue();
            //cout<< "CKYY"<<endl;
            break;
        case YD:
            Yd_ticket();
            //cout<< "YD"<<endl;
            break; 
        case XSYY:
            Xsyy_ticket();
            //cout<< "XSYY"<<endl;
            break;     
        case QXYY:
            Qxyd_ticket();
            //cout<< "QXYY"<<endl;
            break;
        case TC:
            cout<<"TC"<<endl;
            //running=false;
            break;   
        default:
            break;
        }

}
    //测试
/*    printf("buff=(c=%d)=%s\n",c,buff);
    
    //定义json对象
    Json::Reader Read;
    Json::Value val;
    //反序列化字符串转json类
    Read.parse(buff,val);
    cout<<val["user_tel"].asString()<<endl;
    cout<<val["user_password"].asString()<<endl;

    Json::Value res;
    res["status"]="ok";

    send(c,res.toStyledString().c_str(),strlen(res.toStyledString().c_str()),0);
*/

int main(){
    Ser_Socket ser;
    int sockfd=ser.Socket_Init();
    if(sockfd==-1){
        exit(1);
    }
    Call_Back_Accept *p=new Call_Back_Accept(sockfd);//定义libevent实例
    if(p==NULL) {
        cout<<"accept  obj err"<<endl;
        exit(1);
    } 

    My_lib *plib=new My_lib;
    if(plib==nullptr)   exit(1);
    //处理套接字
    if(!plib->Event_Base_Init() ){//初始化
        cout<<"my_lib init failed\n"<<endl;
        exit(1);
    }
    p->Set_lib(plib);
    plib->Event_Add(sockfd,p);//注册描述符到libevent
    plib->Event_Base_Dispatch();//事件循环

    delete plib;
    delete p;
    exit(0);
}