#include "trandlg.h"
#include "ui_trandlg.h"
#include <QPainter>
#include <QBitmap>
#include <QDebug>
#include <QLabel>
#include <QDesktopWidget>
#include<QMouseEvent>

TranDlg::TranDlg(QWidget *parent) : QDialog(parent),
    ui(new Ui::TranDlg)
{
    ui->setupUi(this);
    startMoveState = false;

    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    this->resize(600,400);
}

TranDlg::~TranDlg()
{
    delete ui;
}


void TranDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void TranDlg::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        startMoveState = true;
        moveBegin = event->globalPos();

        int gx,gy,gw,gh;
        rect = this->geometry();
        gx = rect.x();
        gy = rect.y();
        gw = rect.width();
        gh = rect.height();

        if((moveBegin.x() < (gx+10)) &&(moveBegin.y()<(gy+10))){
            qDebug()<<"left top";
        }
        else if((moveBegin.x()>(gx+gw-10))&&(moveBegin.y()<(gy+10))){
            qDebug()<<"right top"<<rect;
            isTopArea = true;
        }
        else if((moveBegin.x()>(gx+gw-10))&&(moveBegin.y()>(gy+gh-10))){
            qDebug()<<"right bottom"<<rect;
            isTopArea = false;

        }
        else if((moveBegin.x() < (gx+10))&&(moveBegin.y()>(gy+gh-10))){
            qDebug()<<"left bottom";

        }
    }

    //qDebug()<<this->geometry()<<event->globalPos();

    event->ignore();
}

void TranDlg::mouseMoveEvent(QMouseEvent *event)
{

    if(startMoveState){
        moveEnd = event->globalPos();
        if(isTopArea ){
            rect.setTopRight(moveEnd);
            this->setGeometry(rect);
            qDebug()<<rect;
        }
        else if(!isTopArea){
            rect.setBottomRight(moveEnd);
            this->setGeometry(rect);
            qDebug()<<rect;
        }
        else {
            this->move(this->pos()-moveBegin+moveEnd);
            moveBegin = moveEnd;
        }
    }
    event->ignore();
}

void TranDlg::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        this->startMoveState = false ;
    event->ignore();
}
