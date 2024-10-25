#pragma once

#include "http_client.h"
#include "json_parser.h"
#include "secrets.h"
#include "instruments.h"

#include <vector>
#include <string>

class TradingSystem
{
private:
    RestClient client;
    JsonParser parser;
    std::vector<std::string> kinds = {"future", "option", "spot", "future_combo", "option_combo"};
    std::vector<std::string> currencies;
    std::vector<std::string> index_price_names;
    std::unordered_map<std::string, Instrument> instruments;
    std::string auth_token;

    JsonValue placeOrder(bool isBuy, const std::string instrument_name = "", int amount = 0, int contracts = 0,
        const std::string type = "", const std::string label = "", int price = -1,
        const std::string time_in_force = "", int max_show = -1, int post_only = -1,
        int reject_post_only = -1, int reduce_only = -1, int trigger_price = -1,
        int trigger_offset = -1, const std::string trigger = "", const std::string advanced = "",
        int mmp = -1, int valid_until = 0, const std::string linked_order_type = "",
        const std::string trigger_fill_condition = "");

public:
    TradingSystem();
    ~TradingSystem();

    // Get Order Book
    JsonValue getOrderBook(const std::string &instrument_name, int depth = 5);

    // Place Order
    JsonValue buy(const std::string instrument_name = "", int amount = 0, int contracts = 0,
        const std::string type = "", const std::string label = "", int price = -1,
        const std::string time_in_force = "", int max_show = -1, int post_only = -1,
        int reject_post_only = -1, int reduce_only = -1, int trigger_price = -1,
        int trigger_offset = -1, const std::string trigger = "", const std::string advanced = "",
        int mmp = -1, int valid_until = 0, const std::string linked_order_type = "",
        const std::string trigger_fill_condition = "");
    JsonValue sell(const std::string instrument_name = "", int amount = 0, int contracts = 0,
        const std::string type = "", const std::string label = "", int price = -1,
        const std::string time_in_force = "", int max_show = -1, int post_only = -1,
        int reject_post_only = -1, int reduce_only = -1, int trigger_price = -1,
        int trigger_offset = -1, const std::string trigger = "", const std::string advanced = "",
        int mmp = -1, int valid_until = 0, const std::string linked_order_type = "",
        const std::string trigger_fill_condition = "");
    
    // Cancel Order
    JsonValue cancel(const std::string order_id);
    JsonValue cancelAll(bool detailed = false, bool freeze_quotes = false);
    JsonValue cancelAllByCurrency(const std::string currency, const std::string kind = "any",
        const std::string type = "all", bool detailed = false, bool freeze_quotes = false);
    JsonValue cancelAllByCurrencyPair(const std::string currency_pair, const std::string kind = "any",
        const std::string type = "all", bool detailed = false, bool freeze_quotes = false);
    JsonValue cancelAllByInstrument(const std::string instrument_name, const std::string kind = "any",
        const std::string type = "all", bool detailed = false, bool freeze_quotes = false);
    JsonValue cancelAllByKindOrType(const std::string currency = "any", const std::string kind = "any",
        const std::string type = "all", bool detailed = false, bool freeze_quotes = false);    
    JsonValue cancelByLabel(const std::string label, const std::string currency = "");

    // Edit Order
    JsonValue edit(const std::string order_id, int amount = -1, int contracts = -1, int price = -1,
        int post_only = -1, int reduce_only = -1, int reject_post_only = -1, std::string advanced = "",
        int trigger_price = -1, int trigger_offset = -1, int mmp = -1, int valid_until = 0);
    JsonValue editByLabel(const std::string label, const std::string instrument_name, int amount = -1,
        int contracts = -1, int price = -1, int post_only = -1, int reduce_only = -1,
        int reject_post_only = -1, std::string advanced = "", int trigger_price = -1,
        int trigger_offset = -1, int mmp = -1, int valid_until = 0);

    // View Current Positions
    JsonValue getOpenOrders(const std::string kind = "", const std::string type = "all");
    JsonValue getOpenOrdersByCurrency(const std::string currency, const std::string kind = "", const std::string type = "all");
    JsonValue getOpenOrdersByInstrument(const std::string instrument_name, const std::string type = "all");
    JsonValue getOpenOrdersByLabel(const std::string currency, const std::string label);

    // View Order States
    JsonValue getOrderState(const std::string order_id);
    JsonValue getOrderStateByLabel(const std::string currency, const std::string label);
};