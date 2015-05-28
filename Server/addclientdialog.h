#ifndef ADDCLIENTDIALOG_H
#define ADDCLIENTDIALOG_H

#include <QDialog>
#include "trademanager.h"

namespace Ui {
class AddClientDialog;
}

class AddClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddClientDialog(TradeManager *tm,QWidget *parent = 0);
    explicit AddClientDialog(TradeManager *tm, int client_id, QWidget *parent = 0);
    ~AddClientDialog();

private:
    Ui::AddClientDialog *ui;
    TradeManager *tm;

signals:
    void clientAdded(QString client_name);

public slots:
    void onAccept();
    void onReject();

};

#endif // ADDCLIENTDIALOG_H
