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
    this->setWindowFlags(Qt::Tool|Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    this->resize(800,600);

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
            this->isleftArea = true;
            this->isTopArea = true;
            qDebug()<<"left top";
        }
        else if((moveBegin.x()>(gx+gw-10))&&(moveBegin.y()<(gy+10))){
            qDebug()<<"right top"<<rect;
            this->isleftArea = false;
            this->isTopArea = true;
        }
        else if((moveBegin.x()>(gx+gw-10))&&(moveBegin.y()>(gy+gh-10))){
            qDebug()<<"right bottom"<<rect;
            this->isleftArea = false;
            this->isTopArea = false;

        }
        else if((moveBegin.x() < (gx+10))&&(moveBegin.y()>(gy+gh-10))){
            this->isleftArea = true;
            this->isTopArea = false;
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
        if(isleftArea && isTopArea ){
            rect.setTopLeft(moveEnd);
            int _w = (rect.width()/8)*8;
            int _h = (rect.height()/8)*8;
            QPoint _point = rect.bottomRight();
            moveEnd.setX(_point.x() - _w +1);
            moveEnd.setY(_point.y() - _h +1);
            rect.setTopLeft(moveEnd);
            this->setGeometry(rect);
//            qDebug()<<_w<<rect.width()<<_h<<rect.height();

        }
        else if(!isleftArea && isTopArea ){
            rect.setTopRight(moveEnd);
            int _w = (rect.width()/8)*8;
            int _h = (rect.height()/8)*8;
            QPoint _point = rect.bottomLeft();
            moveEnd.setX(_point.x() + _w -1);
            moveEnd.setY(_point.y() - _h +1);
            rect.setTopRight(moveEnd);

            this->setGeometry(rect);
//            qDebug()<<_w<<rect.width()<<_h<<rect.height();

        }
        else if(!isleftArea && !isTopArea ){
//            rect.setBottomRight(moveEnd);
//            if((rect.width() % 8 == 0) && (rect.height()%8 == 0)){
//                this->setGeometry(rect);
//                qDebug()<<rect;
//            }
            rect.setBottomRight(moveEnd);
            int _w = (rect.width()/8)*8;
            int _h = (rect.height()/8)*8;
            QPoint _point = rect.topLeft();
            moveEnd.setX(_point.x() + _w -1);
            moveEnd.setY(_point.y() + _h -1);
            rect.setBottomRight(moveEnd);

            this->setGeometry(rect);
//            qDebug()<<_w<<rect.width()<<_h<<rect.height();
        }
        else if(isleftArea && !isTopArea ){
//            rect.setBottomLeft(moveEnd);
//            if((rect.width() % 8 == 0) && (rect.height()%8 == 0)){
//                this->setGeometry(rect);
//                qDebug()<<rect;
//            }
            rect.setBottomLeft(moveEnd);
            int _w = (rect.width()/8)*8;
            int _h = (rect.height()/8)*8;
            QPoint _point = rect.topRight();
            moveEnd.setX(_point.x() - _w +1);
            moveEnd.setY(_point.y() + _h -1);
            rect.setBottomLeft(moveEnd);

            this->setGeometry(rect);
//            qDebug()<<_w<<rect.width()<<_h<<rect.height();
        }
        qDebug()<<rect;
    }
    event->ignore();
}

void TranDlg::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        if(this->startMoveState)
            emit(signal_rect(this->rect));
        this->startMoveState = false ;
    }
    event->ignore();
}
