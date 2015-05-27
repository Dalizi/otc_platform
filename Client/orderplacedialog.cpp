#include "orderplacedialog.h"
#include "ui_orderplacedialog.h"
#include "ClientService.h"

extern int ClientId;

orderPlaceDialog::orderPlaceDialog(ClientServiceClient *csc, QouteTrans *qt, bool is_close, LongShortType ls, QWidget *parent) :
    QDialog(parent),
    rpc(csc),
    qt(qt),
    ui(new Ui::orderPlaceDialog)
{
    ui->setupUi(this);
    instr_code = QString::fromStdString(qt->instr_code);
    ask_price = qt->ask_price;
    ask_volume = qt->ask_volume;
    bid_price = qt->bid_price;
    bid_volume = qt->bid_volume;
    ui->clientidLineEdit->setText(QString::number(ClientId));
    ui->instrCodeLineEdit->setText(instr_code);
    ui->priceDoubleSpinBox->setValue(ui->longRadioButton->isChecked()?ask_price:bid_price);
    ui->quantSpinBox->setMaximum(ui->longRadioButton->isChecked()?ask_volume:bid_volume);
    ui->quantSpinBox->setMinimum(1);
    if (is_close) {
        if (ls == LONG_ORDER) {
            ui->longRadioButton->setVisible(false);
            ui->shortRadioButton->setChecked(true);
            ui->shortRadioButton->setDisabled(true);
        }  else {
            ui->shortRadioButton->setVisible(false);
            ui->longRadioButton->setChecked(true);
            ui->longRadioButton->setDisabled(true);
        }
        ui->openRadioButton->setVisible(false);
        ui->offsetRadioButton->setChecked(true);
        ui->offsetRadioButton->setDisabled(true);

    }

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLongShortRadioButtonClicked(int)));
}

orderPlaceDialog::~orderPlaceDialog()
{
    delete ui;
}

void orderPlaceDialog::onAccepted() {
    OrderTypeTrans ot;
    ot.client_id = ClientId;
    ot.instr_code = ui->instrCodeLineEdit->text().toStdString();
    ot.price = ui->priceDoubleSpinBox->value();
    ot.amount = ui->quantSpinBox->value();
    ot.open_offset = ui->openRadioButton->isChecked()?OPEN : OFFSET;
    ot.long_short = ui->longRadioButton->isChecked()?LONG_ORDER : SHORT_ORDER;
    rpc->place_order(ot);
    emit orderPlaced();
}

void orderPlaceDialog::onRejected() {

}



void orderPlaceDialog::onLongShortRadioButtonClicked(int id) {
    ui->priceDoubleSpinBox->setValue(id==0?ask_price:bid_price);
}

