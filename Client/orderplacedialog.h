#ifndef ORDERPLACEDIALOG_H
#define ORDERPLACEDIALOG_H

#include "ClientService.h"

#include <QDialog>
#include <QMainWindow>
#include <QString>

namespace Ui {
class orderPlaceDialog;
}

class orderPlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit orderPlaceDialog(ClientServiceClient *csc, QouteTrans *qt, bool is_close=false, LongShortType ls=LONG, QWidget *parent = 0);
    ~orderPlaceDialog();

private:
    Ui::orderPlaceDialog *ui;
    ClientServiceClient *rpc;
    QouteTrans *qt;
    QString instr_code;
    int ask_volume;
    double ask_price;
    int bid_volume;
    double bid_price;

public slots:
    void onAccepted();
    void onRejected();
    void onLongShortRadioButtonClicked(int id);

signals:
    void orderPlaced();

};

#endif // ORDERPLACEDIALOG_H
