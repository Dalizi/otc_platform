#include "addclientdialog.h"
#include "ui_addclientdialog.h"
#include "tradetypes.h"


using namespace std;

AddClientDialog::AddClientDialog(boost::shared_ptr<TradeManager> arg_tm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddClientDialog),
    tm(arg_tm)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(onAccept()));
}


AddClientDialog::~AddClientDialog()
{
    delete ui;
}

void AddClientDialog::onAccept() {
    ClientInfo ci;
    ci.client_name = ui->khmcLineEdit->text();
    ci.interview_record = ui->ftxxLineEdit->text();
    ci.review_material = ui->syzlLineEdit->text();
    ci.public_info = ui->gkxxLineEdit->text();
    ci.client_relationship = ui->khgxLineEdit->text();
    ci.client_level = ui->khywdjLineEdit->text().toUInt();
    ci.trust_value = ui->sxedLineEdit->text().toDouble();
    ci.chartered_business = ui->ywfwLineEdit->text();
    int client_id = tm->setClientInfo(ci);
	tm->initClientBalance(client_id, ui->initBalanceLineEdit->text().toDouble());
    emit clientAdded(ci.client_name);
}

void AddClientDialog::onReject() {

}

