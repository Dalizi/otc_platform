#include "orderplacedialog.h"
#include "ui_orderplacedialog.h"

#include <iomanip>


orderPlaceDialog::orderPlaceDialog(TradeManager *tm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::orderPlaceDialog),
    tm(tm)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted(void)), this, SLOT(onAccepted(void)));
}

orderPlaceDialog::~orderPlaceDialog()
{
    delete ui;
}

void orderPlaceDialog::onAccepted() {
    TransactionType ot;
    stringstream ss;
    ot.client_id = ui->clientidLineEdit->text().toInt();
    ot.instr_code = ui->instrCodeLineEdit->text();
    int base_id = tm->getTransactionID();
    ot.time = QDateTime::currentDateTime();
    ot.price = stod(ui->priceLineEdit->text().toStdString());
    ot.amount = stoi(ui->quantLineEdit->text().toStdString());
    ot.open_offset = ui->openRadioButton->isChecked() ? OPEN:OFFSET;
    ot.long_short = ui->longRadioButton->isChecked()?LONG_ORDER:SHORT_ORDER;
    ss <<setfill('0') <<setw(8) <<QDate::currentDate().toString("yyyyMMdd").toStdString();
    ss <<setfill('0') <<setw(8) <<base_id;
    ot.transaction_id = QString::fromStdString(ss.str());
    tm->setTransaction(ot);
    tm->setPosition(ot);
    emit orderPlaced();

}

void orderPlaceDialog::onRejected() {

}

void orderPlaceDialog::onAddClient(int client_id) {
    open();
    ui->clientidLineEdit->setText(QString("%1").arg(client_id, 8, 10, QChar('0')));
}

