#include "maindlg.h"
#include "ui_maindlg.h"
#include  <QtAV>
#include <QDebug>
#include <windows.h>
#include <QDir>



MainDlg::MainDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDlg)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

    if(1){
    qDebug()<<"#######audio codec:########";
    qDebug()<<QtAV::AudioEncoder::supportedCodecs();
    qDebug()<<"#######video codec:########";
    qDebug()<<QtAV::VideoEncoder::supportedCodecs();
    qDebug()<<"#######muxer format:########";
    qDebug()<<QtAV::AVMuxer::supportedFormats();
}

    tranDlg = NULL;

    auRingBuffer    = new AuDataRingBuffer;
    AUpacketBuffer  = new PacketRingBuffer;
    AVpacketBuffer  = new PacketRingBuffer;

    timestamp = new TimeStamp;

    this->fps = 10;  //设置帧率
    screen = qApp->primaryScreen();
    this->area_x = 0;
    this->area_y = 0;
    this->area_w = screen->size().width();
    this->area_h = screen->size().height();
    this->frameRate = 800000;

    audioCapThread = new AuDataCapThread(timestamp,auRingBuffer);
    audioEncThread = new AuEncThread(auRingBuffer,AUpacketBuffer);

#ifdef USE_VIDEO_THREAD
    videoRingBuffer = new VideoRingBuffer;
    videoEncThread = new VideoEncThread(videoRingBuffer,AVpacketBuffer);
    this->avVideoEnc = videoEncThread->getVideoEnc();
    videoEncThread->setScreenSize(1024,768);
#else
    avVideoEnc = QtAV::VideoEncoder::create("FFmpeg");
    avVideoEnc->setPixelFormat(QtAV::VideoFormat::Format_YUV420P);
    avVideoEnc->setCodecName("libx264");//libx264   mpeg4
    avVideoEnc->setTimestampMode(QtAV::AVEncoder::TimestampCopy);  //音视频同步关键
#endif


    AVCodecContext *avctx = (AVCodecContext *)avVideoEnc->codecContext();
    av_opt_set((void *)avctx->priv_data,"preset","ultrafast",0);
    av_opt_set((void *)avctx->priv_data,"tune","zerolatency",0);

    this->avAudioEnc = audioEncThread->getAudioEnc();

    writerThread = new WriterThread(AUpacketBuffer,AVpacketBuffer,avAudioEnc,
                                    avVideoEnc);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),this, SLOT(slot_timerTimeout()));

    qtaudioInput = audioCapThread->getAudioInput();

    ui->lineEditSaveDir->setText(QDir::currentPath());

    this->initHotKey();
    this->initTrayIcon();

    this->startStatus = false;
}

void MainDlg::initVideoEnc()
{
    avVideoEnc->setBitRate(frameRate);
    avVideoEnc->setFrameRate(fps);
    avVideoEnc->setWidth(area_w);
    avVideoEnc->setHeight(area_h);
}

MainDlg::~MainDlg()
{
    delete ui;
}

void MainDlg::slot_timerTimeout()
{
    QPixmap pixmap = screen->grabWindow(QApplication::desktop()->winId()
                                        ,0,0,screen->size().width(),screen->size().height());

    QImage image = pixmap.toImage();
    QPainter painter(&image);

    painter.drawPixmap(QCursor::pos(),QPixmap(":/image/pointer_yellow.png"));
    qreal _timest = qtaudioInput->elapsedUSecs()/1000000.0;
//    qreal _timest = this->timestamp->getTimeStamp();
#ifdef USE_VIDEO_THREAD
    videoRingBuffer->pushData(image//.scaled(32*9*4,32*6*4,Qt::IgnoreAspectRatio)
                              ,_timest);
#else
    QtAV::VideoFrame frame(image);
    frame = frame.to(QtAV::VideoFormat::Format_YUV420P);
    if(frame.isValid()){
        frame.setTimestamp(_timest);
        avVideoEnc->encode(frame);

        QtAV::Packet packet = avVideoEnc->encoded();
        if(packet.isValid()){
//            qDebug()<<"video get time stamp:"<<_timest
//                   <<"packet pts:"<<packet.pts;
//            packet.pts = _timest;
//            packet.dts = _timest;
//            packet.duration = _timest-lastpts;
//            lastpts = _timest;

            AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
        }
        else{
            qDebug()<<" video encode packet is no valid";
        }
        //qDebug()<<"video encodec finished";
    }
#endif
}

void MainDlg::on_btnStart_clicked()
{
    if(ui->lineEditSaveFile->text().isEmpty())
        return;
    if(ui->lineEditSaveDir->text().isEmpty())
        return;

    QString _filename = ui->lineEditSaveDir->text()+"/"+ui->lineEditSaveFile->text()+".mp4";
    qDebug()<<_filename;

    initVideoEnc();

    writerThread->setSaveFileName(_filename);

    this->AUpacketBuffer->clear();
    this->AVpacketBuffer->clear();
    this->auRingBuffer->clear();
    audioCapThread->start();
    audioEncThread->start();
#ifdef USE_VIDEO_THREAD
    videoEncThread->start();
#else
    avVideoEnc->open();
#endif

    timer->start(1000/fps);
    writerThread->start();

    ui->btnStop->setEnabled(true);
    ui->btnStart->setEnabled(false);
    this->hotkey_F2->setEnabled(false);
    this->hotkey_F3->setEnabled(true);
    this->startStatus = true;
    this->hide();
    this->trayIcon->show();
}

void MainDlg::on_btnStop_clicked()
{
    audioCapThread->stop();
    audioEncThread->stop();
    timer->stop();
#ifdef USE_VIDEO_THREAD
    videoEncThread->stop();
#else
    qDebug()<<"MainDlg::on_btnStop_clicked";
    while(avVideoEnc->encode()){
        QtAV::Packet packet = avVideoEnc->encoded();
        if(packet.isValid()){
            AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
        }
    }

    /* push end data to video ring buffer*/
    QtAV::Packet packet = QtAV::Packet::createEOF();
    AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
    avVideoEnc->flush();
    avVideoEnc->close();

#endif
    writerThread->stop();

    ui->btnStop->setEnabled(false);
    ui->btnStart->setEnabled(true);
    this->hotkey_F2->setEnabled(true);
    this->hotkey_F3->setEnabled(false);
    this->startStatus = false;

    this->show();
    this->trayIcon->hide();
}

#include <QShortcut>

void MainDlg::initHotKey()
{
    this->hotkey_F2 = new QxtGlobalShortcut(QKeySequence("F2"),this);
    this->hotkey_F3 = new QxtGlobalShortcut(QKeySequence("F3"),this);
    connect(hotkey_F2,SIGNAL(activated()),this,SLOT(slot_hotKey()));
    connect(hotkey_F3,SIGNAL(activated()),this,SLOT(slot_hotKey()));
//    QShortcut *key = new QShortcut(QKeySequence("F2"),this);
//    connect(key,SIGNAL(activated()),this,SLOT(slot_hotKey()));
    this->hotkey_F3->setEnabled(false);
}

void MainDlg::slot_hotKey()
{
    if(this->sender() == this->hotkey_F2){
        this->on_btnStart_clicked();
        qDebug()<<"#######f2 pressed";
    }
    else if(this->sender() == this->hotkey_F3){
        this->on_btnStop_clicked();
        qDebug()<<"#######f3 pressed";
    }
}

void MainDlg::initTrayIcon()
{
    this->trayIcon = new QSystemTrayIcon(QIcon(":/image/icon_cursor.png"),this);
    this->trayIcon->setToolTip("屏幕截取工具");
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(slot_trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainDlg::slot_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger){
        qDebug()<<"trigger";
        this->show();
    }
}

void MainDlg::on_btnSelectSaveDir_clicked()
{
    QString dirstr = QFileDialog::getExistingDirectory(this,"选择目录",ui->lineEditSaveDir->text());
    if(!dirstr.isEmpty())
        ui->lineEditSaveDir->setText(dirstr);
}

void MainDlg::on_btnClose_clicked()
{
    if(startStatus)
        this->on_btnStop_clicked();
    if(tranDlg != NULL)
        delete tranDlg;
    this->close();
}

void MainDlg::changeEvent(QEvent *e)
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

void MainDlg::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        //       qDebug()<<"left"<<event->pos();
        this->mouseMoveState = true;
        pointstart = event->globalPos();
    }
    event->ignore();
}

void MainDlg::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        this->mouseMoveState = false ;
    event->ignore();

}

void MainDlg::mouseMoveEvent(QMouseEvent *event)
{
    if(mouseMoveState){
        //       qDebug()<<"move"<<this->pos();
        pointend = event->globalPos();
        this->move(this->pos()-pointstart+pointend);
        pointstart = pointend;
    }
    event->ignore();
}

void MainDlg::on_rbtnEreaSelect_toggled(bool checked)
{
    if(checked){
         this->tranDlg = new TranDlg;
        tranDlg->show();
    }
    else{
        delete tranDlg;
        tranDlg = NULL;
    }
}
