#include "tradetypes.h"

OrderType &operator<<(OrderType &ot, const OrderTypeTrans &ott) {
	ot.amount = ott.amount;
	ot.client_id = ott.client_id;
	ot.instr_code = QString::fromStdString(ott.instr_code);
	ot.long_short = ott.long_short;
	ot.open_offset = ott.open_offset;
	ot.order_id = QString::fromStdString(ott.order_id);
	ot.order_status = ott.order_status;
	ot.price = ott.price;
	return ot;

}
OrderTypeTrans &operator<<(OrderTypeTrans &ott, const OrderType &ot) {
	ott.amount = ot.amount;
	ott.client_id = ot.client_id;
	ott.instr_code = ot.instr_code.toStdString();
	ott.long_short = ot.long_short;
	ott.open_offset = ot.open_offset;
	ott.order_id = ot.order_id.toStdString();
	ott.order_status = ot.order_status;
	ott.price = ot.price;
	ott.date_time = ot.time.toString("yyyy-MM-dd hh:mm:ss").toStdString();
	return ott;
}

PositionType &operator<<(PositionType &pt, const PositionTypeTrans &ptt) {
	pt.available_amount = ptt.available_amount;
	pt.average_price = ptt.average_price;
	pt.client_id = ptt.client_id;
	pt.frozen_amount = ptt.frozen_amount;
	pt.instr_code = QString::fromStdString(ptt.instr_code);
	pt.long_short = ptt.long_short;
	pt.total_amount = ptt.total_amount;
    pt.underlying_price = ptt.underlying_price;
    pt.occupied_margin = ptt.occupied_margin;
	return pt;
}
PositionTypeTrans &operator<<(PositionTypeTrans &ptt, const PositionType &pt) {
    ptt.available_amount = pt.available_amount;
    ptt.average_price = pt.average_price;
    ptt.client_id = pt.client_id;
    ptt.total_amount = pt.total_amount;
    ptt.frozen_amount = pt.frozen_amount;
    ptt.instr_code = pt.instr_code.toStdString();
    ptt.long_short = pt.long_short;
    ptt.underlying_price = pt.underlying_price;
    ptt.occupied_margin = pt.occupied_margin;
	return ptt;
}

