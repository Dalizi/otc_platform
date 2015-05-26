/*
*OTC RPC IDL file
*/

enum LongShortType {
	LONG_ORDER,
	SHORT_ORDER
}

enum OpenOffsetType {
	OPEN,
	OFFSET
}

enum OrderStatusType {
	REPORTED,
	ACCEPTED,
	REJECTED,
	CANCELED
}

struct ClientInfoTrans {
	1: string client_name,
	2: i32 client_id,
	3: string interview_record,
	4: string review_material,
	5: string public_info,
	6: string client_relationship,
	7: i16 client_level,
	8: double trust_value,
	9: string chartered_business
}

struct PositionTypeTrans {
	1: i32 client_id,
	2: string instr_code,
	3: double average_price,
	4: i32 total_amount,
	5: i32 available_amount,
	6: i32 frozen_amount,
	7: LongShortType long_short,
	8: double offset_price,
    9: double underlying_price,
    10: double occupied_margin
}

struct OrderTypeTrans {
	1: string instr_code,
	2: string order_id,
	3: string date_time,
	4: i32 client_id,
	5: double price,
	6: i32 amount,
	7: LongShortType long_short,
	8: OpenOffsetType open_offset,
	9: OrderStatusType order_status
}

struct TransactionTypeTrans {
	1: string instr_code,
	2: string transaction_id,
	3: string date_time,
	4: i32 client_id,
	5: double price,
	6: i32 amount,
	7: LongShortType long_short,
	8: OpenOffsetType open_offset,
	9: double underlying_price,
    10: double close_pnl
}

struct ClientBalanceTrans {
	1: i32 client_id,
	2: double total_balance,
	3: double available_balance,
	4: double withdrawable_balance,
	5: double occupied_margin,
}

struct GreekRisk {
	1: double delta,
	2: double gamma,
	3: double theta,
	4: double vega
}

struct QouteTrans {
	1: string instr_code,
	2: double ask_price,
	3: i32 ask_volume,
	4: double bid_price,
	5: i32 bid_volume
}

exception InvalidQuery {
  1: i32 error_no,
  2: string why
}

service ClientService {
	bool auth(1:string client_id, 2:string password) throws (1:InvalidQuery iq),
	double get_pnl(1:PositionTypeTrans ptt) throws (1:InvalidQuery iq),
	double get_close_price(1:PositionTypeTrans ptt) throws (1:InvalidQuery iq),
	map<string, double> get_calculated_balance(1:i32 client_id) throws (1:InvalidQuery iq),
	GreekRisk get_greeks() throws (1:InvalidQuery iq),
	GreekRisk get_client_greeks(1:i32 client_id) throws (1:InvalidQuery iq),
	void update_hedge_position(1:list<PositionTypeTrans> ptt) throws (1:InvalidQuery iq),
	void place_order(1: OrderTypeTrans order) throws (1:InvalidQuery iq),
	list<OrderTypeTrans> get_order(1:i32 client_id, 2:string start_date, 3:string end_date) throws (1:InvalidQuery iq),
	list<TransactionTypeTrans> get_transaction(1:i32 client_id, 2:string start_date, 3:string end_date) throws(1:InvalidQuery iq),
	list<PositionTypeTrans> get_position(1:i32 client_id) throws (1:InvalidQuery iq),
	ClientBalanceTrans get_balance(1:i32 client_id) throws (1:InvalidQuery iq),
	ClientInfoTrans get_clientinfo(1:i32 client_id) throws (1:InvalidQuery iq),
	list<QouteTrans> get_qoute() throws (1:InvalidQuery iq)
}
