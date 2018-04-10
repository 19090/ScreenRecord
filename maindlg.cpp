#include "maindlg.h"
#include "ui_maindlg.h"
#include  <QtAV>
#include <QDebug>
#include <windows.h>
#include <QDir>
#include <QMessageBox>


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
#ifdef USE_TWO_PACKET_BUF
    AUpacketBuffer  = new PacketRingBuffer;
    AVpacketBuffer  = new PacketRingBuffer;
#else
    packetBuffer  = new PacketRingBuffer;
#endif
    timestamp = new TimeStamp;

    this->video_fps = 10;  //设置帧率
    screen = qApp->primaryScreen();
    this->area_x = 0;
    this->area_y = 0;
    this->area_w = screen->size().width();
    this->area_h = screen->size().height();
    this->video_bitrate = 400000;
    ui->spinBoxFPS->setValue(video_fps);
//    ui->spinBoxbiteRate->setValue(video_bitrate);
    ui->labelX->setText("x:"+QString::number(area_x));
    ui->labelY->setText("y:"+QString::number(area_y));
    ui->labelW->setText("w:"+QString::number(area_w));
    ui->labelH->setText("h:"+QString::number(area_h));

    audioCapThread = new AuDataCapThread(timestamp,auRingBuffer);
#ifdef USE_TWO_PACKET_BUF
    audioEncThread = new AuEncThread(auRingBuffer,AUpacketBuffer);
#else
    audioEncThread = new AuEncThread(auRingBuffer,packetBuffer);
#endif
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


//    AVCodecContext *avctx = (AVCodecContext *)avVideoEnc->codecContext();
//    av_opt_set((void *)avctx->priv_data,"preset","ultrafast",0);
//    av_opt_set((void *)avctx->priv_data,"tune","zerolatency",0);

    this->avAudioEnc = audioEncThread->getAudioEnc();
#ifdef USE_TWO_PACKET_BUF
    writerThread = new WriterThread(AUpacketBuffer,AVpacketBuffer,avAudioEnc,
                                    avVideoEnc);
#else
    writerThread = new WriterThread(packetBuffer,avAudioEnc,
                                    avVideoEnc);
#endif

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),this, SLOT(slot_timerTimeout()));

    qtaudioInput = audioCapThread->getAudioInput();

    int _vol = qtaudioInput->volume()*100;
    qDebug()<<"########"<<qtaudioInput->volume();
    ui->sliderVolume->setValue(_vol);
    ui->lineEditSaveDir->setText(QDir::currentPath());

    this->initHotKey();
    this->initTrayIcon();

    this->startStatus = false;

    this->isRecordVideo = true;
    this->isRecordAudio = true;
    this->isRecordCursor = true;
}

void MainDlg::initVideoEnc()
{
    screen = qApp->primaryScreen();
//    if(this->tranDlg != NULL){
//        qDebug()<<tranDlg->geometry();
//        area_x = tranDlg->geometry().x()+6;
//        area_y = tranDlg->geometry().y()+6;
//        area_w = tranDlg->geometry().width()-12;
//        area_h = tranDlg->geometry().height()-12;
//    }
//    else {
//        this->area_x = 0;
//        this->area_y = 0;
//        this->area_w = screen->size().width();
//        this->area_h = screen->size().height();
//    }
    if(this->tranDlg == NULL){
        this->area_x = 0;
        this->area_y = 0;
        this->area_w = screen->size().width();
        this->area_h = screen->size().height();
    }
    else {
        area_x = area_x+6;
        area_y = area_y+6;
        area_w = area_w-12;
        area_h = area_h-12;
    }
    int _bitrate  = 0;
    int _fpsd5 = video_fps/5;
    if(_fpsd5 == 0) _fpsd5 =1;
    else if(_fpsd5 > 5) _fpsd5 = 5;

//    if(area_w >= 1920) _bitrate = 340000;  //8500k
//    else if(area_w >= 1280) _bitrate = 140000;
//    else if(area_w >= 720) _bitrate = 72000;
//    else _bitrate = 40000;
//    video_bitrate = _bitrate * video_fps;

//    video_bitrate = area_w*area_h/192/60*video_fps*1000;
    video_bitrate = area_w*area_h*video_fps*8/46;
    ui->spinBoxbiteRate->setValue(video_bitrate);

    qDebug()<<"######### bit rate:" <<video_bitrate;
    qDebug()<<"######### area:"<<QRect(area_x,area_y,area_w,area_h);
    avVideoEnc->setBitRate(video_bitrate);
    avVideoEnc->setFrameRate(video_fps);
    avVideoEnc->setWidth(area_w);
    avVideoEnc->setHeight(area_h);

//    avVideoEnc->setBitRate(0);
//    AVCodecContext *avctx = (AVCodecContext *)avVideoEnc->codecContext();
//    avctx->flags |= CODEC_FLAG_QSCALE;
//    avctx->rc_min_rate = frameRate;
//    avctx->rc_max_rate = frameRate*8;
//    avctx->bit_rate = frameRate;
//    avctx->qmin = 50;
//    avctx->qmax = 50;
//    av_opt_set(avctx->priv_data,"qp","30",AV_OPT_SEARCH_CHILDREN);

}

MainDlg::~MainDlg()
{
    delete ui;
}

void MainDlg::slot_timerTimeout()
{
    QPixmap pixmap = screen->grabWindow(QApplication::desktop()->winId()
                                        ,area_x,area_y,area_w,area_h);

    QImage image = pixmap.toImage();
    if(isRecordCursor){
        QPainter painter(&image);
        painter.drawPixmap(QCursor::pos().x()-area_x,
                           QCursor::pos().y()-area_y,
                           QPixmap(":/image/pointer_yellow.png"));
    }
    qreal _timest = qtaudioInput->elapsedUSecs()/1000000.0;

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

#ifdef USE_TWO_PACKET_BUF
            AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#else
             packetBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#endif
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
    writerThread->setRecordAudio(isRecordAudio);
    writerThread->setRecordVideo(isRecordVideo);
#ifdef USE_TWO_PACKET_BUF
    this->AUpacketBuffer->clear();
    this->AVpacketBuffer->clear();
#else
    this->packetBuffer->clear();
#endif
    this->auRingBuffer->clear();
    audioCapThread->start();
    audioEncThread->start();
#ifdef USE_VIDEO_THREAD
    videoEncThread->start();
#else

    avVideoEnc->open();
    qDebug()<<"open fff";
#endif

    timer->start(1000/video_fps);
    writerThread->start();

//    ui->btnStop->setEnabled(true);
//    ui->btnStart->setEnabled(false);
    this->hotkey_F2->setEnabled(false);
    this->hotkey_F3->setEnabled(true);
    this->startStatus = true;
    this->hide();
    this->trayIcon->show();

    if(ui->checkBoxTimeRecord->isChecked()){
        QTime _time = ui->timeEdit->time();
        int interval = _time.hour()*60*60 + _time.minute()*60 + _time.second();
        qDebug()<<"time record interval:"<<interval;
        QTimer::singleShot(interval*1000,this,SLOT(on_btnStop_clicked()));
    }

    this->intervelSecond = 0;

    setupUiStart(true);
}

void MainDlg::on_btnStop_clicked()
{
    if(!startStatus){
        qDebug()<<"has stop";
        return ;
    }
    qDebug()<<"###:on_btnStop_clicked begin";
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
#ifdef USE_TWO_PACKET_BUF
            AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#else
            packetBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#endif
        }
    }

    /* push end data to video ring buffer*/
    QtAV::Packet packet = QtAV::Packet::createEOF();
#ifdef USE_TWO_PACKET_BUF
    AVpacketBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#else
    packetBuffer->pushPacket(packet,PACKET_TYPE_VIDEO);
#endif
    avVideoEnc->flush();
    avVideoEnc->close();

#endif
    writerThread->stop();

//    ui->btnStop->setEnabled(false);
//    ui->btnStart->setEnabled(true);
    this->hotkey_F2->setEnabled(true);
    this->hotkey_F3->setEnabled(false);
    this->startStatus = false;

    this->show();
    this->trayIcon->hide();

    setupUiStart(false);
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
        if(this->isHidden())
            this->show();
        else
            this->hide();
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
    else
        this->trayIcon->hide();
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
    qDebug()<<"full screen select:"<<!checked;
    if(checked){
         this->tranDlg = new TranDlg;
        connect(tranDlg,SIGNAL(signal_rect(QRect)),
                this,SLOT(slot_tranDlg_rect(QRect)));
        tranDlg->show();
        slot_tranDlg_rect(tranDlg->geometry());
    }
    else{
        disconnect(tranDlg,SIGNAL(signal_rect(QRect)),
                this,SLOT(slot_tranDlg_rect(QRect)));
        delete tranDlg;
        tranDlg = NULL;
        slot_tranDlg_rect(screen->geometry());
    }
}

void MainDlg::slot_tranDlg_rect(QRect rect)
{
    qDebug()<<"#####"<<rect;
    this->area_x = rect.x();
    this->area_y = rect.y();
    this->area_w = rect.width();
    this->area_h = rect.height();
    ui->labelX->setText("x:"+QString::number(area_x));
    ui->labelY->setText("y:"+QString::number(area_y));
    ui->labelW->setText("w:"+QString::number(area_w));
    ui->labelH->setText("h:"+QString::number(area_h));
}


void MainDlg::on_spinBoxFPS_valueChanged(int arg1)
{
    this->video_fps = arg1;
}

void MainDlg::on_checkBoxTimeRecord_clicked(bool checked)
{
    ui->timeEdit->setEnabled(checked);
}

void MainDlg::on_cBoxRecordVideo_clicked(bool checked)
{
    if(!checked)
    {
        if(!isRecordAudio){
            QMessageBox::warning(this,"错误","不能同时不录制视频和音频!");
            ui->cBoxRecordVideo->setChecked(true);
            this->isRecordVideo = true;
            return ;
        }
    }
    this->isRecordVideo = checked;
}

void MainDlg::on_cBoxRecordAudio_clicked(bool checked)
{
    if(!checked)
    {
        if(!isRecordVideo){
            QMessageBox::warning(this,"错误","不能同时不录制视频和音频!");
            ui->cBoxRecordAudio->setChecked(true);
            this->isRecordAudio = true;
            return ;
        }
    }
    this->isRecordAudio = checked;
}

void MainDlg::on_cBoxRecordCursor_clicked(bool checked)
{
    this->isRecordCursor = checked;
}

void MainDlg::setupUiStart(bool _enable)
{
//    ui->cBoxRecordVideo->setEnabled(_enable);
//    ui->cBoxRecordAudio->setEnabled(_enable);
//    ui->cBoxRecordCursor->setEnabled(_enable);
//    ui->lineEditSaveFile->setEnabled(_enable);
//    ui->lineEditSaveDir->setEnabled(_enable);
//    ui->btnSelectSaveDir->setEnabled(_enable);
    ui->gboxBaseSet->setEnabled(!_enable);
    ui->gboxVideoSet->setEnabled(!_enable);
    ui->gboxAudioSet->setEnabled(!_enable);
    ui->btnClose->setEnabled(!_enable);
    ui->btnStart->setEnabled(!_enable);
    ui->btnStop->setEnabled(_enable);
    if(tranDlg != NULL){
        tranDlg->setEnabled(!_enable);
    }
}
