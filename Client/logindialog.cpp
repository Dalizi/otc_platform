#include "logindialog.h"
#include "ui_logindialog.h"

#include <QMessageBox>

#include <string>
#include <stdexcept>

using namespace std;

extern int ClientId;

LoginDialog::LoginDialog(ClientServiceClient *csc, QWidget *parent) :
    QDialog(parent),
    rpc(csc),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onAccepted() {
    string client_id = ui->clientIDLineEdit->text().toStdString();
    string passwd = ui->passwordLineEdit->text().toStdString();
    bool ret=false;
    try {
        try {
            ret = rpc->auth(client_id, passwd);
            if (ret==true) {
                ClientId = stoi(client_id);
                this->accept();
            } else {
                QMessageBox msgBox;
                msgBox.setText("Invalid password.");
                msgBox.exec();
                done(2);
            }

        } catch(InvalidQuery &iq) {
            QMessageBox msgBox;
            msgBox.setText(QString::fromStdString(iq.why));
            msgBox.exec();
            done(2);
        }
    } catch (apache::thrift::TException &tx) {
        QMessageBox msgBox;
        msgBox.setText(tx.what());
        msgBox.exec();
        done(2);
    }
}
