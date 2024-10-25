#include "trading_system.h"
#include <algorithm>

std::string boolString(bool b) {
    return b ? "true" : "false";
}

TradingSystem::TradingSystem() {
    // Initialize global curl environment
    curl_global_init(CURL_GLOBAL_ALL);

    // Get all currencies
    {
        std::string url = "https://test.deribit.com/api/v2/public/get_currencies";
        JsonValue result = parser.parse(client.get(url));
        for (const JsonValue& currency : result.at("result").get<JsonArray>()) {
            currencies.push_back(currency.at("currency").get<std::string>());
        }
    }

    // Get all index price names
    {
        std::string url = "https://test.deribit.com/api/v2/public/get_index_price_names";
        JsonValue result = parser.parse(client.get(url));
        for (const JsonValue& index_price_name : result.at("result").get<JsonArray>()) {
            index_price_names.push_back(index_price_name.get<std::string>());
        }
    }
    
    // Get all instruments
    for (const std::string& kind : kinds) {
        std::string url = "https://test.deribit.com/api/v2/public/get_instruments?currency=any&kind=" + kind;
        std::string response = client.get(url);
        JsonValue result = parser.parse(response);
        for (const JsonValue& instrument : result.at("result").get<JsonArray>()) {
            instruments[instrument.at("instrument_name").get<std::string>()] = Instrument(
                instrument.at("base_currency").get<std::string>(),
                instrument.at("quote_currency").get<std::string>(),
                instrument.at("kind").get<std::string>(),
                instrument.at("is_active").get<bool>()
            );
        }
    }

    // Get Auth Token
    std::string url = "https://test.deribit.com/api/v2/public/auth?client_id=" + secrets::client_id + "&client_secret=" + secrets::client_secret + "&grant_type=client_credentials";
    std::string response = client.get(url);
    JsonValue result = parser.parse(response);
    this->auth_token = result.at("result").at("access_token").get<std::string>();
}

TradingSystem::~TradingSystem() {
    // Cleanup global curl environment
    this->auth_token.clear();
    curl_global_cleanup();
}

JsonValue TradingSystem::getOrderBook(const std::string& instrument_name, int depth) {
    if (instruments.find(instrument_name) == instruments.end()) {
        std::cout << "Instrument not found: " + instrument_name << std::endl;
        return JsonValue();
    }
    if (depth != 1 && depth != 5 && depth != 10 && depth != 20 && depth != 50 && depth != 100 && depth != 1000 && depth != 10000) {
        std::cout << "Invalid depth: " + std::to_string(depth) << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/public/get_order_book?instrument_name=" + instrument_name + "&depth=" + std::to_string(depth);
    std::string response = client.get(url);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::placeOrder(bool isBuy, const std::string instrument_name, int amount, int contracts,
        const std::string type, const std::string label, int price,
        const std::string time_in_force, int max_show, int post_only,
        int reject_post_only, int reduce_only, int trigger_price,
        int trigger_offset, const std::string trigger, const std::string advanced,
        int mmp, int valid_until, const std::string linked_order_type,
        const std::string trigger_fill_condition) {
    
    std::string params = "";
    if (instruments.find(instrument_name) == instruments.end()) {
        std::cout << "Instrument not found: " + instrument_name << std::endl;
        return JsonValue();
    } else {
        params += "instrument_name=" + instrument_name;
    }

    if (amount == 0 && contracts == 0) {
        std::cout << "Amount and contracts cannot both be zero" << std::endl;
        return JsonValue();
    } else if (amount != 0 && contracts != 0) {
        std::cout << "Amount and contracts cannot both be non-zero" << std::endl;
        return JsonValue();
    } else if (amount != 0) {
        params += "&amount=" + std::to_string(amount);
    } else {
        params += "&contracts=" + std::to_string(contracts);
    }

    if (type != "" && type != "limit" && type != "stop_limit" && type != "take_limit" && type != "market" && type != "stop_market" && type != "take_market" && type != "market_limit" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    } else if (!type.empty()) {
        params += "&type=" + type;
    }

    if (label.length() > 64) {
        std::cout << "Label is too long: " + label << std::endl;
        return JsonValue();
    } else if (!label.empty()) {
        params += "&label=" + label;
    }

    if (price == -1 && (type == "" || type == "limit" || type == "stop_limit")) {
        std::cout << "Price cannot be zero for order type: " + type << std::endl;
        return JsonValue();
    } else if (price != -1) {
        params += "&price=" + std::to_string(price);
    }

    if (time_in_force != "" && time_in_force != "good_til_cancelled" && time_in_force != "good_til_day" && time_in_force != "fill_or_kill" && time_in_force != "immediate_or_cancel") {
        std::cout << "Invalid time in force: " + time_in_force << std::endl;
        return JsonValue();
    } else if (!time_in_force.empty()) {
        params += "&time_in_force=" + time_in_force;
    }

    if (max_show != -1) {
        params += "&max_show=" + std::to_string(max_show);
    }

    if (post_only == 1 && type == "good_til_cancelled") {
        params += "&post_only=true";
    }

    if (reject_post_only == 1 && post_only == 1 && type == "good_til_cancelled") {
        params += "&reject_post_only=true";
    }

    if (reduce_only == 1) {
        params += "&reduce_only=true";
    }

    if (trigger_price != -1 && (type == "stop_limit" || type == "take_limit" || type == "trailing_stop")) {
        params += "&trigger_price=" + std::to_string(trigger_price);
    } else if (trigger_price != -1) {
        std::cout << "Trigger price can only be set for order types: stop_limit, take_limit, trailing_stop" << std::endl;
        return JsonValue();
    }

    if (trigger_offset != -1 && type == "trailing_stop") {
        params += "&trigger_offset=" + std::to_string(trigger_offset);
    } else if (trigger_offset != -1) {
        std::cout << "Trigger offset can only be set for order type: trailing_stop" << std::endl;
        return JsonValue();
    }

    if (trigger != "" && trigger != "index_price" && trigger != "mark_price" && trigger != "last_price") {
        std::cout << "Invalid trigger: " + trigger << std::endl;
        return JsonValue();
    } else if (!trigger.empty()) {
        params += "&trigger=" + trigger;
    }

    if (advanced != "" && advanced != "post_only" && advanced != "reduce_only" && advanced != "reject_post_only" && advanced != "trailing_stop" && advanced != "close_on_trigger") {
        std::cout << "Invalid advanced option: " + advanced << std::endl;
        return JsonValue();
    } else if (!advanced.empty()) {
        params += "&advanced=" + advanced;
    }

    if (mmp == 1 && (type == "limit" || type == "")) {
        params += "&mmp=true";
    } else if (mmp == 0 && (type == "limit" || type == "")) {
        params += "&mmp=flase";
    } else if (mmp != -1) {
        std::cout << "MMP can only be set for order type: limit" << std::endl;
        return JsonValue();
    }

    if (valid_until != 0) {
        params += "&valid_until=" + std::to_string(valid_until);
    }

    if (linked_order_type != "" && linked_order_type != "one_triggers_other" && linked_order_type != "one_cancels_other" && linked_order_type != "one_triggers_one_cancels_other") {
        std::cout << "Invalid linked order type: " + linked_order_type << std::endl;
        return JsonValue();
    } else if (!linked_order_type.empty()) {
        params += "&linked_order_type=" + linked_order_type;
    }

    if (trigger_fill_condition != "" && trigger_fill_condition != "first_hit" && trigger_fill_condition != "complete_hit" && trigger_fill_condition != "incremental") {
        std::cout << "Invalid trigger fill condition: " + trigger_fill_condition << std::endl;
        return JsonValue();
    } else if (!trigger_fill_condition.empty()) {
        params += "&trigger_fill_condition=" + trigger_fill_condition;
    }

    if (isBuy) {
        std::string url = "https://test.deribit.com/api/v2/private/buy?" + params;
        std::string response = client.get(url, this->auth_token);
        JsonValue result = parser.parse(response);
        return result;
    } else {
        std::string url = "https://test.deribit.com/api/v2/private/sell?" + params;
        std::string response = client.get(url, this->auth_token);
        JsonValue result = parser.parse(response);
        return result;
    }
}

JsonValue TradingSystem::buy(const std::string instrument_name, int amount, int contracts,
        const std::string type, const std::string label, int price,
        const std::string time_in_force, int max_show, int post_only,
        int reject_post_only, int reduce_only, int trigger_price,
        int trigger_offset, const std::string trigger, const std::string advanced,
        int mmp, int valid_until, const std::string linked_order_type,
        const std::string trigger_fill_condition) {
    return placeOrder(true, instrument_name, amount, contracts, type, label, price, time_in_force, max_show, post_only, reject_post_only, reduce_only, trigger_price, trigger_offset, trigger, advanced, mmp, valid_until, linked_order_type, trigger_fill_condition);
}

JsonValue TradingSystem::sell(const std::string instrument_name, int amount, int contracts,
        const std::string type, const std::string label, int price,
        const std::string time_in_force, int max_show, int post_only,
        int reject_post_only, int reduce_only, int trigger_price,
        int trigger_offset, const std::string trigger, const std::string advanced,
        int mmp, int valid_until, const std::string linked_order_type,
        const std::string trigger_fill_condition) {
    return placeOrder(false, instrument_name, amount, contracts, type, label, price, time_in_force, max_show, post_only, reject_post_only, reduce_only, trigger_price, trigger_offset, trigger, advanced, mmp, valid_until, linked_order_type, trigger_fill_condition);
}

JsonValue TradingSystem::cancel(const std::string order_id) {
    std::string url = "https://test.deribit.com/api/v2/private/cancel?order_id=" + order_id;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAll(bool detailed, bool freeze_quotes) {
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all?detailed=" + boolString(detailed) + "&freeze_quotes=" + boolString(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByCurrency(const std::string currency, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        std::cout << "Invalid kind: " + kind << std::endl;
        return JsonValue();
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_currency?currency=" + currency + "&kind=" + kind + "&type=" + type + "&detailed=" + boolString(detailed) + "&freeze_quotes=" + boolString(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByCurrencyPair(const std::string currency_pair, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (std::find(index_price_names.begin(), index_price_names.end(), currency_pair) == index_price_names.end()) {
        std::cout << "Invalid currency pair: " + currency_pair << std::endl;
        return JsonValue();
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        std::cout << "Invalid kind: " + kind << std::endl;
        return JsonValue();
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_currency_pair?currency_pair=" + currency_pair + "&kind=" + kind + "&type=" + type + "&detailed=" + boolString(detailed) + "&freeze_quotes=" + boolString(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByInstrument(const std::string instrument_name, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (instruments.find(instrument_name) == instruments.end()) {
        std::cout << "Instrument not found: " + instrument_name << std::endl;
        return JsonValue();
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_instrument?instrument_name=" + instrument_name + "&kind=" + kind + "&type=" + type + "&detailed=" + boolString(detailed) + "&freeze_quotes=" + boolString(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByKindOrType(const std::string currency, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (currency != "any" && std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        std::cout << "Invalid kind: " + kind << std::endl;
        return JsonValue();
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_kind_or_type?currency=" + currency + "&kind=" + kind + "&type=" + type + "&detailed=" + boolString(detailed) + "&freeze_quotes=" + boolString(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelByLabel(const std::string label, const std::string currency) {
    if (currency == "") {
        std::string url = "https://test.deribit.com/api/v2/private/cancel_by_label?label=" + label;
        std::string response = client.get(url, this->auth_token);
        JsonValue result = parser.parse(response);
        return result;
    }
    else if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_by_label?label=" + label + "&currency=" + currency;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::edit(const std::string order_id, int amount, int contracts, int price,
    int post_only, int reduce_only, int reject_post_only, std::string advanced,
    int trigger_price, int trigger_offset, int mmp, int valid_until) {
    if (amount == -1 && contracts == -1 && price == -1 && post_only == -1 && reduce_only == -1 && reject_post_only == -1 && advanced == "" && trigger_price == -1 && trigger_offset == -1 && mmp == -1 && valid_until == 0) {
        std::cout << "No parameters to edit" << std::endl;
        return JsonValue();
    }
    std::string params = "order_id=" + order_id;
    if (amount != -1) {
        params += "&amount=" + std::to_string(amount);
    }
    if (contracts != -1) {
        params += "&contracts=" + std::to_string(contracts);
    }
    if (price != -1) {
        params += "&price=" + std::to_string(price);
    }
    if (post_only == 0) {
        params += "&post_only=false";
    } else if (post_only == 1) {
        params += "&post_only=true";
    }
    if (reduce_only == 0) {
        params += "&reduce_only=false";
    } else if (reduce_only == 1) {
        params += "&reduce_only=true";
    }
    if (reject_post_only == 0) {
        params += "&reject_post_only=false";
    } else if (reject_post_only == 1) {
        params += "&reject_post_only=true";
    }
    if (advanced != "") {
        params += "&advanced=" + advanced;
    }
    if (trigger_price != -1) {
        params += "&trigger_price=" + std::to_string(trigger_price);
    }
    if (trigger_offset != -1) {
        params += "&trigger_offset=" + std::to_string(trigger_offset);
    }
    if (mmp == 0) {
        params += "&mmp=false";
    } else if (mmp == 1) {
        params += "&mmp=true";
    }
    if (valid_until != 0) {
        params += "&valid_until=" + std::to_string(valid_until);
    }
    std::string url = "https://test.deribit.com/api/v2/private/edit?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::editByLabel(const std::string label, const std::string instrument_name, int amount,
    int contracts, int price, int post_only, int reduce_only, int reject_post_only,
    std::string advanced, int trigger_price, int trigger_offset, int mmp, int valid_until) {
    if (label == "") {
        std::cout << "Label cannot be empty" << std::endl;
        return JsonValue();
    }
    if (label.length() > 64) {
        std::cout << "Label is too long: " + label << std::endl;
        return JsonValue();
    }
    if (instruments.find(instrument_name) == instruments.end()) {
        std::cout << "Instrument not found: " + instrument_name << std::endl;
        return JsonValue();
    }
    if (amount == -1 && contracts == -1 && price == -1 && post_only == -1 && reduce_only == -1 && reject_post_only == -1 && advanced == "" && trigger_price == -1 && trigger_offset == -1 && mmp == -1 && valid_until == 0) {
        std::cout << "No parameters to edit" << std::endl;
        return JsonValue();
    }
    std::string params = "label=" + label + "&instrument_name=" + instrument_name;
    if (amount != -1) {
        params += "&amount=" + std::to_string(amount);
    }
    if (contracts != -1) {
        params += "&contracts=" + std::to_string(contracts);
    }
    if (price != -1) {
        params += "&price=" + std::to_string(price);
    }
    if (post_only == 0) {
        params += "&post_only=false";
    } else if (post_only == 1) {
        params += "&post_only=true";
    }
    if (reduce_only == 0) {
        params += "&reduce_only=false";
    } else if (reduce_only == 1) {
        params += "&reduce_only=true";
    }
    if (reject_post_only == 0) {
        params += "&reject_post_only=false";
    } else if (reject_post_only == 1) {
        params += "&reject_post_only=true";
    }
    if (advanced != "") {
        params += "&advanced=" + advanced;
    }
    if (trigger_price != -1) {
        params += "&trigger_price=" + std::to_string(trigger_price);
    }
    if (trigger_offset != -1) {
        params += "&trigger_offset=" + std::to_string(trigger_offset);
    }
    if (mmp == 0) {
        params += "&mmp=false";
    } else if (mmp == 1) {
        params += "&mmp=true";
    }
    if (valid_until != 0) {
        params += "&valid_until=" + std::to_string(valid_until);
    }
    std::string url = "https://test.deribit.com/api/v2/private/edit_by_label?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;   
}

JsonValue TradingSystem::getOpenOrders(const std::string kind, const std::string type) {
    std::string params = "";
    if (kind != "") {
        if (kind != "future" && kind != "option" && kind != "spot" && kind != "future_combo" && kind != "option_combo") {
            std::cout << "Invalid kind: " + kind << std::endl;
            return JsonValue();
        }
        params += "kind=" + kind;
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop_all" && type != "stop_limit" && type != "stop_market" && type != "take_all" && type != "take_limit" && type != "take_market" && type != "trailing_all" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    } else {
        params += "&type=" + type;
    }
    std::string url = "https://test.deribit.com/api/v2/private/get_open_orders?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::getOpenOrdersByCurrency(const std::string currency, const std::string kind, const std::string type) {
    std::string params = "";
    if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    } else {
        params += "currency=" + currency;
    }
    if (kind != "") {
        if (kind != "future" && kind != "option" && kind != "spot" && kind != "future_combo" && kind != "option_combo") {
            std::cout << "Invalid kind: " + kind << std::endl;
            return JsonValue();
        }
        params += "kind=" + kind;
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop_all" && type != "stop_limit" && type != "stop_market" && type != "take_all" && type != "take_limit" && type != "take_market" && type != "trailing_all" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    } else {
        params += "&type=" + type;
    }
    std::string url = "https://test.deribit.com/api/v2/private/get_open_orders_by_currency?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::getOpenOrdersByInstrument(const std::string instrument_name, const std::string type) {
    std::string params = "";
    if (instruments.find(instrument_name) == instruments.end()) {
        std::cout << "Instrument not found: " + instrument_name << std::endl;
        return JsonValue();
    } else {
        params += "instrument_name=" + instrument_name;
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop_all" && type != "stop_limit" && type != "stop_market" && type != "take_all" && type != "take_limit" && type != "take_market" && type != "trailing_all" && type != "trailing_stop") {
        std::cout << "Invalid order type: " + type << std::endl;
        return JsonValue();
    } else {
        params += "&type=" + type;
    }
    std::string url = "https://test.deribit.com/api/v2/private/get_open_orders_by_instrument?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::getOpenOrdersByLabel(const std::string currency, const std::string label) {
    if (label == "") {
        std::cout << "Label cannot be empty" << std::endl;
        return JsonValue();
    }
    if (label.length() > 64) {
        std::cout << "Label is too long: " + label << std::endl;
        return JsonValue();
    }
    std::string params = "label=" + label;
    if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    } else {
        params += "currency=" + currency;
    }
    std::string url = "https://test.deribit.com/api/v2/private/get_open_orders_by_label?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::getOrderState(const std::string order_id) {
    std::string url = "https://test.deribit.com/api/v2/private/get_order_state?order_id=" + order_id;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::getOrderStateByLabel(const std::string currency, const std::string label) {
    if (label == "") {
        std::cout << "Label cannot be empty" << std::endl;
        return JsonValue();
    }
    if (label.length() > 64) {
        std::cout << "Label is too long: " + label << std::endl;
        return JsonValue();
    }
    std::string params = "label=" + label;
    if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        std::cout << "Invalid currency: " + currency << std::endl;
        return JsonValue();
    } else {
        params += "currency=" + currency;
    }
    std::string url = "https://test.deribit.com/api/v2/private/get_order_state_by_label?" + params;
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

