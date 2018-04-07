#ifndef MAINDLG_H
#define MAINDLG_H

#include <QDialog>
#include <QScreen>
#include <QTimer>
#include <QDesktopWidget>
#include <QFile>
#include <QFileDialog>

#include <QtAV/VideoEncoder.h>
#include <QtAV/VideoFormat.h>
#include <QtAV/VideoFrame.h>

#include "audataringbuffer.h"
#include "packetringbuffer.h"
#include "audatacapthread.h"
#include "auencthread.h"
//#include "screencapencthread.h"
#include "timestamp.h"
#include "writerthread.h"
#include "videoringbuffer.h"
#include "videoencthread.h"

#include <QxtWidgets/QxtGlobalShortcut>

#include <QSystemTrayIcon>

#include "trandlg.h"

//#define USE_VIDEO_THREAD
#undef USE_VIDEO_THREAD

namespace Ui {
class MainDlg;
}

class MainDlg : public QDialog
{
    Q_OBJECT

public:
    explicit MainDlg(QWidget *parent = 0);
    ~MainDlg();

private slots:
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void slot_timerTimeout();

private:
    Ui::MainDlg *ui;

    AuDataRingBuffer *auRingBuffer;     //audio raw data
    PacketRingBuffer *AVpacketBuffer;   //video packet data
    PacketRingBuffer *AUpacketBuffer;   //audio packet data
    TimeStamp *timestamp;               //删除

    AuDataCapThread *audioCapThread;
    AuEncThread     *audioEncThread;
#ifdef USE_VIDEO_THREAD
    VideoEncThread *videoEncThread;
    VideoRingBuffer *videoRingBuffer;
#else
    QtAV::VideoFormat   *avVideoFormat;     //uused
    QtAV::VideoFrame    *avVideoFrame;      //uused
#endif

    WriterThread *writerThread;

    ///////////
    QTimer *timer;

    QScreen *screen;

    QtAV::AudioEncoder *avAudioEnc;
    QtAV::VideoEncoder *avVideoEnc;

    QAudioInput *qtaudioInput;  //用于获取时间戳

    bool startStatus;   //开始状态

private:
    QxtGlobalShortcut *hotkey_F2;
    QxtGlobalShortcut *hotkey_F3;
    void initHotKey();

    QSystemTrayIcon *trayIcon;
    void initTrayIcon();
private slots:
    void slot_hotKey();

    void on_btnSelectSaveDir_clicked();
    void on_btnClose_clicked();

    void slot_trayIconActivated(QSystemTrayIcon::ActivationReason);

    void on_rbtnEreaSelect_toggled(bool checked);
 private:
    void initVideoEnc();
    int area_x,area_y,area_w,area_h;
    int frameRate;
    int fps;

protected:
    void changeEvent(QEvent *e);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
private:
    /*********鼠标点击移动*********/
    QPoint pointstart,pointend;
    bool mouseMoveState;

    TranDlg *tranDlg;
};

#endif // MAINDLG_H
