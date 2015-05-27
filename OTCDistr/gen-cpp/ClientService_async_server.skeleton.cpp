// This autogenerated skeleton file illustrates one way to adapt a synchronous
// interface into an asynchronous interface. You should copy it to another
// filename to avoid overwriting it and rewrite as asynchronous any functions
// that would otherwise introduce unwanted latency.

#include "ClientService.h"
#include <thrift/protocol/TBinaryProtocol.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::async;

using boost::shared_ptr;

class ClientServiceAsyncHandler : public ClientServiceCobSvIf {
 public:
  ClientServiceAsyncHandler() {
    syncHandler_ = std::auto_ptr<ClientServiceHandler>(new ClientServiceHandler);
    // Your initialization goes here
  }
  virtual ~ClientServiceAsyncHandler();

  void auth(tcxx::function<void(bool const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const std::string& client_id, const std::string& password) {
    bool _return = false;
    _return = syncHandler_->auth(client_id, password);
    return cob(_return);
  }

  void get_pnl(tcxx::function<void(double const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const PositionTypeTrans& ptt) {
    double _return = (double)0;
    _return = syncHandler_->get_pnl(ptt);
    return cob(_return);
  }

  void get_close_price(tcxx::function<void(double const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const PositionTypeTrans& ptt) {
    double _return = (double)0;
    _return = syncHandler_->get_close_price(ptt);
    return cob(_return);
  }

  void get_calculated_balance(tcxx::function<void(std::map<std::string, double>  const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id) {
    std::map<std::string, double>  _return;
    syncHandler_->get_calculated_balance(_return, client_id);
    return cob(_return);
  }

  void get_greeks(tcxx::function<void(GreekRisk const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */) {
    GreekRisk _return;
    syncHandler_->get_greeks(_return);
    return cob(_return);
  }

  void get_client_greeks(tcxx::function<void(GreekRisk const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id) {
    GreekRisk _return;
    syncHandler_->get_client_greeks(_return, client_id);
    return cob(_return);
  }

  void update_hedge_position(tcxx::function<void()> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const std::vector<PositionTypeTrans> & ptt) {
    syncHandler_->update_hedge_position(ptt);
    return cob();
  }

  void place_order(tcxx::function<void()> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const OrderTypeTrans& order) {
    syncHandler_->place_order(order);
    return cob();
  }

  void get_order(tcxx::function<void(std::vector<OrderTypeTrans>  const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id, const std::string& start_date, const std::string& end_date) {
    std::vector<OrderTypeTrans>  _return;
    syncHandler_->get_order(_return, client_id, start_date, end_date);
    return cob(_return);
  }

  void get_transaction(tcxx::function<void(std::vector<TransactionTypeTrans>  const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id, const std::string& start_date, const std::string& end_date) {
    std::vector<TransactionTypeTrans>  _return;
    syncHandler_->get_transaction(_return, client_id, start_date, end_date);
    return cob(_return);
  }

  void get_position(tcxx::function<void(std::vector<PositionTypeTrans>  const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id) {
    std::vector<PositionTypeTrans>  _return;
    syncHandler_->get_position(_return, client_id);
    return cob(_return);
  }

  void get_balance(tcxx::function<void(ClientBalanceTrans const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id) {
    ClientBalanceTrans _return;
    syncHandler_->get_balance(_return, client_id);
    return cob(_return);
  }

  void get_clientinfo(tcxx::function<void(ClientInfoTrans const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */, const int32_t client_id) {
    ClientInfoTrans _return;
    syncHandler_->get_clientinfo(_return, client_id);
    return cob(_return);
  }

  void get_qoute(tcxx::function<void(std::vector<QouteTrans>  const& _return)> cob, tcxx::function<void(::apache::thrift::TDelayedException* _throw)> /* exn_cob */) {
    std::vector<QouteTrans>  _return;
    syncHandler_->get_qoute(_return);
    return cob(_return);
  }

 protected:
  std::auto_ptr<ClientServiceHandler> syncHandler_;
};
