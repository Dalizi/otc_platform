#ifndef ORDERPLACEDIALOG_H
#define ORDERPLACEDIALOG_H

#include "trademanager.h"

#include <QDialog>

namespace Ui {
class orderPlaceDialog;
}

class orderPlaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit orderPlaceDialog(TradeManager *tm, QWidget *parent = 0);
    ~orderPlaceDialog();

private:
    Ui::orderPlaceDialog *ui;
    TradeManager *tm;

public slots:
    void onAccepted();
    void onRejected();
    void onAddClient(int client_id);

signals:
    void orderPlaced();

};

#endif // ORDERPLACEDIALOG_H
