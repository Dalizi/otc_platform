/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef otc_TYPES_H
#define otc_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>




enum LongShortType {
  LONG = 0,
  SHORT = 1
};

extern const std::map<int, const char*> _LongShortType_VALUES_TO_NAMES;

enum OpenOffsetType {
  OPEN = 0,
  OFFSET = 1
};

extern const std::map<int, const char*> _OpenOffsetType_VALUES_TO_NAMES;

enum OrderStatusType {
  REPORTED = 0,
  ACCEPTED = 1,
  REJECTED = 2,
  CANCELED = 3
};

extern const std::map<int, const char*> _OrderStatusType_VALUES_TO_NAMES;

class ClientInfoTrans;

class PositionTypeTrans;

class OrderTypeTrans;

class TransactionTypeTrans;

class ClientBalanceTrans;

class GreekRisk;

class QouteTrans;

class InvalidQuery;

typedef struct _ClientInfoTrans__isset {
  _ClientInfoTrans__isset() : client_name(false), client_id(false), interview_record(false), review_material(false), public_info(false), client_relationship(false), client_level(false), trust_value(false), chartered_business(false) {}
  bool client_name :1;
  bool client_id :1;
  bool interview_record :1;
  bool review_material :1;
  bool public_info :1;
  bool client_relationship :1;
  bool client_level :1;
  bool trust_value :1;
  bool chartered_business :1;
} _ClientInfoTrans__isset;

class ClientInfoTrans {
 public:

  static const char* ascii_fingerprint; // = "323D56A11A662D65187D0BD9060B9A00";
  static const uint8_t binary_fingerprint[16]; // = {0x32,0x3D,0x56,0xA1,0x1A,0x66,0x2D,0x65,0x18,0x7D,0x0B,0xD9,0x06,0x0B,0x9A,0x00};

  ClientInfoTrans(const ClientInfoTrans&);
  ClientInfoTrans& operator=(const ClientInfoTrans&);
  ClientInfoTrans() : client_name(), client_id(0), interview_record(), review_material(), public_info(), client_relationship(), client_level(0), trust_value(0), chartered_business() {
  }

  virtual ~ClientInfoTrans() throw();
  std::string client_name;
  int32_t client_id;
  std::string interview_record;
  std::string review_material;
  std::string public_info;
  std::string client_relationship;
  int16_t client_level;
  double trust_value;
  std::string chartered_business;

  _ClientInfoTrans__isset __isset;

  void __set_client_name(const std::string& val);

  void __set_client_id(const int32_t val);

  void __set_interview_record(const std::string& val);

  void __set_review_material(const std::string& val);

  void __set_public_info(const std::string& val);

  void __set_client_relationship(const std::string& val);

  void __set_client_level(const int16_t val);

  void __set_trust_value(const double val);

  void __set_chartered_business(const std::string& val);

  bool operator == (const ClientInfoTrans & rhs) const
  {
    if (!(client_name == rhs.client_name))
      return false;
    if (!(client_id == rhs.client_id))
      return false;
    if (!(interview_record == rhs.interview_record))
      return false;
    if (!(review_material == rhs.review_material))
      return false;
    if (!(public_info == rhs.public_info))
      return false;
    if (!(client_relationship == rhs.client_relationship))
      return false;
    if (!(client_level == rhs.client_level))
      return false;
    if (!(trust_value == rhs.trust_value))
      return false;
    if (!(chartered_business == rhs.chartered_business))
      return false;
    return true;
  }
  bool operator != (const ClientInfoTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ClientInfoTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const ClientInfoTrans& obj);

};

void swap(ClientInfoTrans &a, ClientInfoTrans &b);

typedef struct _PositionTypeTrans__isset {
  _PositionTypeTrans__isset() : client_id(false), instr_code(false), average_price(false), total_amount(false), available_amount(false), frozen_amount(false), long_short(false), offset_price(false) {}
  bool client_id :1;
  bool instr_code :1;
  bool average_price :1;
  bool total_amount :1;
  bool available_amount :1;
  bool frozen_amount :1;
  bool long_short :1;
  bool offset_price :1;
} _PositionTypeTrans__isset;

class PositionTypeTrans {
 public:

  static const char* ascii_fingerprint; // = "7D2F1354CF92A9B824710F68652982FC";
  static const uint8_t binary_fingerprint[16]; // = {0x7D,0x2F,0x13,0x54,0xCF,0x92,0xA9,0xB8,0x24,0x71,0x0F,0x68,0x65,0x29,0x82,0xFC};

  PositionTypeTrans(const PositionTypeTrans&);
  PositionTypeTrans& operator=(const PositionTypeTrans&);
  PositionTypeTrans() : client_id(0), instr_code(), average_price(0), total_amount(0), available_amount(0), frozen_amount(0), long_short((LongShortType)0), offset_price(0) {
  }

  virtual ~PositionTypeTrans() throw();
  int32_t client_id;
  std::string instr_code;
  double average_price;
  int32_t total_amount;
  int32_t available_amount;
  int32_t frozen_amount;
  LongShortType long_short;
  double offset_price;

  _PositionTypeTrans__isset __isset;

  void __set_client_id(const int32_t val);

  void __set_instr_code(const std::string& val);

  void __set_average_price(const double val);

  void __set_total_amount(const int32_t val);

  void __set_available_amount(const int32_t val);

  void __set_frozen_amount(const int32_t val);

  void __set_long_short(const LongShortType val);

  void __set_offset_price(const double val);

  bool operator == (const PositionTypeTrans & rhs) const
  {
    if (!(client_id == rhs.client_id))
      return false;
    if (!(instr_code == rhs.instr_code))
      return false;
    if (!(average_price == rhs.average_price))
      return false;
    if (!(total_amount == rhs.total_amount))
      return false;
    if (!(available_amount == rhs.available_amount))
      return false;
    if (!(frozen_amount == rhs.frozen_amount))
      return false;
    if (!(long_short == rhs.long_short))
      return false;
    if (!(offset_price == rhs.offset_price))
      return false;
    return true;
  }
  bool operator != (const PositionTypeTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const PositionTypeTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const PositionTypeTrans& obj);

};

void swap(PositionTypeTrans &a, PositionTypeTrans &b);

typedef struct _OrderTypeTrans__isset {
  _OrderTypeTrans__isset() : instr_code(false), order_id(false), date_time(false), client_id(false), price(false), amount(false), long_short(false), open_offset(false), order_status(false) {}
  bool instr_code :1;
  bool order_id :1;
  bool date_time :1;
  bool client_id :1;
  bool price :1;
  bool amount :1;
  bool long_short :1;
  bool open_offset :1;
  bool order_status :1;
} _OrderTypeTrans__isset;

class OrderTypeTrans {
 public:

  static const char* ascii_fingerprint; // = "44800261FBA1F6EC20D0D01E21AFBBA2";
  static const uint8_t binary_fingerprint[16]; // = {0x44,0x80,0x02,0x61,0xFB,0xA1,0xF6,0xEC,0x20,0xD0,0xD0,0x1E,0x21,0xAF,0xBB,0xA2};

  OrderTypeTrans(const OrderTypeTrans&);
  OrderTypeTrans& operator=(const OrderTypeTrans&);
  OrderTypeTrans() : instr_code(), order_id(), date_time(), client_id(0), price(0), amount(0), long_short((LongShortType)0), open_offset((OpenOffsetType)0), order_status((OrderStatusType)0) {
  }

  virtual ~OrderTypeTrans() throw();
  std::string instr_code;
  std::string order_id;
  std::string date_time;
  int32_t client_id;
  double price;
  int32_t amount;
  LongShortType long_short;
  OpenOffsetType open_offset;
  OrderStatusType order_status;

  _OrderTypeTrans__isset __isset;

  void __set_instr_code(const std::string& val);

  void __set_order_id(const std::string& val);

  void __set_date_time(const std::string& val);

  void __set_client_id(const int32_t val);

  void __set_price(const double val);

  void __set_amount(const int32_t val);

  void __set_long_short(const LongShortType val);

  void __set_open_offset(const OpenOffsetType val);

  void __set_order_status(const OrderStatusType val);

  bool operator == (const OrderTypeTrans & rhs) const
  {
    if (!(instr_code == rhs.instr_code))
      return false;
    if (!(order_id == rhs.order_id))
      return false;
    if (!(date_time == rhs.date_time))
      return false;
    if (!(client_id == rhs.client_id))
      return false;
    if (!(price == rhs.price))
      return false;
    if (!(amount == rhs.amount))
      return false;
    if (!(long_short == rhs.long_short))
      return false;
    if (!(open_offset == rhs.open_offset))
      return false;
    if (!(order_status == rhs.order_status))
      return false;
    return true;
  }
  bool operator != (const OrderTypeTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const OrderTypeTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const OrderTypeTrans& obj);

};

void swap(OrderTypeTrans &a, OrderTypeTrans &b);

typedef struct _TransactionTypeTrans__isset {
  _TransactionTypeTrans__isset() : instr_code(false), transaction_id(false), date_time(false), client_id(false), price(false), amount(false), long_short(false), open_offset(false), underlying_price(false) {}
  bool instr_code :1;
  bool transaction_id :1;
  bool date_time :1;
  bool client_id :1;
  bool price :1;
  bool amount :1;
  bool long_short :1;
  bool open_offset :1;
  bool underlying_price :1;
} _TransactionTypeTrans__isset;

class TransactionTypeTrans {
 public:

  static const char* ascii_fingerprint; // = "C13DDC9C0D9F05D1BA470A95B8798267";
  static const uint8_t binary_fingerprint[16]; // = {0xC1,0x3D,0xDC,0x9C,0x0D,0x9F,0x05,0xD1,0xBA,0x47,0x0A,0x95,0xB8,0x79,0x82,0x67};

  TransactionTypeTrans(const TransactionTypeTrans&);
  TransactionTypeTrans& operator=(const TransactionTypeTrans&);
  TransactionTypeTrans() : instr_code(), transaction_id(), date_time(), client_id(0), price(0), amount(0), long_short((LongShortType)0), open_offset((OpenOffsetType)0), underlying_price(0) {
  }

  virtual ~TransactionTypeTrans() throw();
  std::string instr_code;
  std::string transaction_id;
  std::string date_time;
  int32_t client_id;
  double price;
  int32_t amount;
  LongShortType long_short;
  OpenOffsetType open_offset;
  double underlying_price;

  _TransactionTypeTrans__isset __isset;

  void __set_instr_code(const std::string& val);

  void __set_transaction_id(const std::string& val);

  void __set_date_time(const std::string& val);

  void __set_client_id(const int32_t val);

  void __set_price(const double val);

  void __set_amount(const int32_t val);

  void __set_long_short(const LongShortType val);

  void __set_open_offset(const OpenOffsetType val);

  void __set_underlying_price(const double val);

  bool operator == (const TransactionTypeTrans & rhs) const
  {
    if (!(instr_code == rhs.instr_code))
      return false;
    if (!(transaction_id == rhs.transaction_id))
      return false;
    if (!(date_time == rhs.date_time))
      return false;
    if (!(client_id == rhs.client_id))
      return false;
    if (!(price == rhs.price))
      return false;
    if (!(amount == rhs.amount))
      return false;
    if (!(long_short == rhs.long_short))
      return false;
    if (!(open_offset == rhs.open_offset))
      return false;
    if (!(underlying_price == rhs.underlying_price))
      return false;
    return true;
  }
  bool operator != (const TransactionTypeTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TransactionTypeTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const TransactionTypeTrans& obj);

};

void swap(TransactionTypeTrans &a, TransactionTypeTrans &b);

typedef struct _ClientBalanceTrans__isset {
  _ClientBalanceTrans__isset() : client_id(false), total_balance(false), available_balance(false), withdrawable_balance(false), occupied_margin(false) {}
  bool client_id :1;
  bool total_balance :1;
  bool available_balance :1;
  bool withdrawable_balance :1;
  bool occupied_margin :1;
} _ClientBalanceTrans__isset;

class ClientBalanceTrans {
 public:

  static const char* ascii_fingerprint; // = "5472C7414D3514110BC82AEEAA7E60C6";
  static const uint8_t binary_fingerprint[16]; // = {0x54,0x72,0xC7,0x41,0x4D,0x35,0x14,0x11,0x0B,0xC8,0x2A,0xEE,0xAA,0x7E,0x60,0xC6};

  ClientBalanceTrans(const ClientBalanceTrans&);
  ClientBalanceTrans& operator=(const ClientBalanceTrans&);
  ClientBalanceTrans() : client_id(0), total_balance(0), available_balance(0), withdrawable_balance(0), occupied_margin(0) {
  }

  virtual ~ClientBalanceTrans() throw();
  int32_t client_id;
  double total_balance;
  double available_balance;
  double withdrawable_balance;
  double occupied_margin;

  _ClientBalanceTrans__isset __isset;

  void __set_client_id(const int32_t val);

  void __set_total_balance(const double val);

  void __set_available_balance(const double val);

  void __set_withdrawable_balance(const double val);

  void __set_occupied_margin(const double val);

  bool operator == (const ClientBalanceTrans & rhs) const
  {
    if (!(client_id == rhs.client_id))
      return false;
    if (!(total_balance == rhs.total_balance))
      return false;
    if (!(available_balance == rhs.available_balance))
      return false;
    if (!(withdrawable_balance == rhs.withdrawable_balance))
      return false;
    if (!(occupied_margin == rhs.occupied_margin))
      return false;
    return true;
  }
  bool operator != (const ClientBalanceTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ClientBalanceTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const ClientBalanceTrans& obj);

};

void swap(ClientBalanceTrans &a, ClientBalanceTrans &b);

typedef struct _GreekRisk__isset {
  _GreekRisk__isset() : delta(false), gamma(false), theta(false), vega(false) {}
  bool delta :1;
  bool gamma :1;
  bool theta :1;
  bool vega :1;
} _GreekRisk__isset;

class GreekRisk {
 public:

  static const char* ascii_fingerprint; // = "D40B774F31F5CC6330E604960421B6CB";
  static const uint8_t binary_fingerprint[16]; // = {0xD4,0x0B,0x77,0x4F,0x31,0xF5,0xCC,0x63,0x30,0xE6,0x04,0x96,0x04,0x21,0xB6,0xCB};

  GreekRisk(const GreekRisk&);
  GreekRisk& operator=(const GreekRisk&);
  GreekRisk() : delta(0), gamma(0), theta(0), vega(0) {
  }

  virtual ~GreekRisk() throw();
  double delta;
  double gamma;
  double theta;
  double vega;

  _GreekRisk__isset __isset;

  void __set_delta(const double val);

  void __set_gamma(const double val);

  void __set_theta(const double val);

  void __set_vega(const double val);

  bool operator == (const GreekRisk & rhs) const
  {
    if (!(delta == rhs.delta))
      return false;
    if (!(gamma == rhs.gamma))
      return false;
    if (!(theta == rhs.theta))
      return false;
    if (!(vega == rhs.vega))
      return false;
    return true;
  }
  bool operator != (const GreekRisk &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const GreekRisk & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const GreekRisk& obj);

};

void swap(GreekRisk &a, GreekRisk &b);

typedef struct _QouteTrans__isset {
  _QouteTrans__isset() : instr_code(false), ask_price(false), ask_volume(false), bid_price(false), bid_volume(false) {}
  bool instr_code :1;
  bool ask_price :1;
  bool ask_volume :1;
  bool bid_price :1;
  bool bid_volume :1;
} _QouteTrans__isset;

class QouteTrans {
 public:

  static const char* ascii_fingerprint; // = "72F44F217EA4AD9725E7AE3C1415175F";
  static const uint8_t binary_fingerprint[16]; // = {0x72,0xF4,0x4F,0x21,0x7E,0xA4,0xAD,0x97,0x25,0xE7,0xAE,0x3C,0x14,0x15,0x17,0x5F};

  QouteTrans(const QouteTrans&);
  QouteTrans& operator=(const QouteTrans&);
  QouteTrans() : instr_code(), ask_price(0), ask_volume(0), bid_price(0), bid_volume(0) {
  }

  virtual ~QouteTrans() throw();
  std::string instr_code;
  double ask_price;
  int32_t ask_volume;
  double bid_price;
  int32_t bid_volume;

  _QouteTrans__isset __isset;

  void __set_instr_code(const std::string& val);

  void __set_ask_price(const double val);

  void __set_ask_volume(const int32_t val);

  void __set_bid_price(const double val);

  void __set_bid_volume(const int32_t val);

  bool operator == (const QouteTrans & rhs) const
  {
    if (!(instr_code == rhs.instr_code))
      return false;
    if (!(ask_price == rhs.ask_price))
      return false;
    if (!(ask_volume == rhs.ask_volume))
      return false;
    if (!(bid_price == rhs.bid_price))
      return false;
    if (!(bid_volume == rhs.bid_volume))
      return false;
    return true;
  }
  bool operator != (const QouteTrans &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const QouteTrans & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const QouteTrans& obj);

};

void swap(QouteTrans &a, QouteTrans &b);

typedef struct _InvalidQuery__isset {
  _InvalidQuery__isset() : error_no(false), why(false) {}
  bool error_no :1;
  bool why :1;
} _InvalidQuery__isset;

class InvalidQuery : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "3F5FC93B338687BC7235B1AB103F47B3";
  static const uint8_t binary_fingerprint[16]; // = {0x3F,0x5F,0xC9,0x3B,0x33,0x86,0x87,0xBC,0x72,0x35,0xB1,0xAB,0x10,0x3F,0x47,0xB3};

  InvalidQuery(const InvalidQuery&);
  InvalidQuery& operator=(const InvalidQuery&);
  InvalidQuery() : error_no(0), why() {
  }

  virtual ~InvalidQuery() throw();
  int32_t error_no;
  std::string why;

  _InvalidQuery__isset __isset;

  void __set_error_no(const int32_t val);

  void __set_why(const std::string& val);

  bool operator == (const InvalidQuery & rhs) const
  {
    if (!(error_no == rhs.error_no))
      return false;
    if (!(why == rhs.why))
      return false;
    return true;
  }
  bool operator != (const InvalidQuery &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const InvalidQuery & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const InvalidQuery& obj);

  mutable std::string thriftTExceptionMessageHolder_;
  const char* what() const throw();
};

void swap(InvalidQuery &a, InvalidQuery &b);



#endif
