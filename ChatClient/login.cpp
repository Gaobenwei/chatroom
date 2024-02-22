#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //登录时隐藏用户名选项
    ui->nameEdit->hide();
    ui->nameLb->hide();
    //界面设置
    setFixedSize(width(),height());//固定大小

    //信号槽
    connect(ui->signupBtn,&QPushButton::clicked,this,&Login::signup);
    connect(ui->loginBtn,&QPushButton::clicked,this,&Login::login);
}

Login::~Login()
{
    delete ui;
}

void Login::signup()
{
    //注册界面变动
    ui->titleLb->setText("注册");
    ui->nameEdit->show();
    ui->nameLb->show();
    ui->loginBtn->setText("确定");
    ui->accoutEdit->clear();
    ui->passwordEdit->clear();
    ui->nameEdit->clear();
}

void Login::login()
{
    QString text = ui->loginBtn->text();
    if(text == "确定"){
        if(ui->accoutEdit->text() == ""){
            QMessageBox::critical(this,"error","请输入账号");
            return;
        }
        if(ui->passwordEdit->text() == ""){
            QMessageBox::critical(this,"error","请输入密码");
            return;
        }
        if(ui->nameEdit->text() == ""){
            QMessageBox::critical(this,"error","请输入用户名");
            return;
        }

        userInfo.account = ui->accoutEdit->text();
        userInfo.password = ui->passwordEdit->text();
        userInfo.name = ui->nameEdit->text();
        int ret = userInfo.saveInfo(userInfo);
        if(ret == -1){
            QMessageBox::critical(this,"error","保存失败");
            return;
        }
        else if(ret == -2){
            QMessageBox::critical(this,"error","创建失败");
            return;
        }
        else if(ret == 1){
            QMessageBox::critical(this,"error","账号已存在");
            return;
        }
        else{
            QMessageBox::information(this,"info","注册成功");
        }
        ui->nameEdit->hide();
        ui->nameLb->hide();
        ui->titleLb->setText("登录");
        ui->loginBtn->setText("登录");
        ui->accoutEdit->clear();
        ui->passwordEdit->clear();
        ui->nameEdit->clear();
    }
    else if(text == "登录"){
        userInfo.account = ui->accoutEdit->text();
        userInfo.password = ui->passwordEdit->text();
        int ret = userInfo.seekUser(userInfo);
        if(ret==1){
            QMessageBox::critical(this,"error","用户名错误");
            return;
        }
        else if(ret == 2){
            QMessageBox::critical(this,"error","密码错误");
            return;
        }
        else if(ret == -1){
            QMessageBox::critical(this,"error","查询错误");
            return;
        }
        else{
            loginSuccess(userInfo.name);
            this->close();
        }
    }
}

