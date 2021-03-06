#ifndef TRANDLG_H
#define TRANDLG_H

#include <QDialog>

namespace Ui {
class TranDlg;
}

class TranDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TranDlg(QWidget *parent = 0);
    ~TranDlg();

protected:
    void changeEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void signal_rect(QRect);
private:
    Ui::TranDlg *ui;

    QPoint moveBegin,moveEnd;
    bool startMoveState;

    bool isTopArea;
    bool isleftArea;

    QPoint leftTopPoint,rightBottomPoint;
    QRect rect;
};

#endif // TRANDLG_H
