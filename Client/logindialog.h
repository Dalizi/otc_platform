#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ClientService.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(ClientServiceClient *csc, QWidget *parent = 0);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;
    ClientServiceClient *rpc;

private slots:
    void onAccepted();
};

#endif // LOGINDIALOG_H
