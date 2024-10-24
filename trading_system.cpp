#include "trading_system.h"
#include <algorithm>

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
        throw std::runtime_error("Instrument not found: " + instrument_name);
    }
    if (depth != 1 && depth != 5 && depth != 10 && depth != 20 && depth != 50 && depth != 100 && depth != 1000 && depth != 10000) {
        throw std::runtime_error("Invalid depth: " + std::to_string(depth));
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
        throw std::runtime_error("Instrument not found: " + instrument_name);
    } else {
        params += "instrument_name=" + instrument_name;
    }

    if (amount == 0 && contracts == 0) {
        throw std::runtime_error("Amount and contracts cannot both be zero");
    } else if (amount != 0 && contracts != 0) {
        throw std::runtime_error("Amount and contracts cannot both be non-zero");
    } else if (amount != 0) {
        params += "&amount=" + std::to_string(amount);
    } else {
        params += "&contracts=" + std::to_string(contracts);
    }

    if (type != "" && type != "limit" && type != "stop_limit" && type != "take_limit" && type != "market" && type != "stop_market" && type != "take_market" && type != "market_limit" && type != "trailing_stop") {
        throw std::runtime_error("Invalid order type: " + type);
    } else if (!type.empty()) {
        params += "&type=" + type;
    }

    if (label.length() > 64) {
        throw std::runtime_error("Label is too long: " + label);
    } else if (!label.empty()) {
        params += "&label=" + label;
    }

    if (price == -1 && (type == "" || type == "limit" || type == "stop_limit")) {
        throw std::runtime_error("Price cannot be zero for order type: " + type);
    } else if (price != -1) {
        params += "&price=" + std::to_string(price);
    }

    if (time_in_force != "" && time_in_force != "good_til_cancelled" && time_in_force != "good_til_day" && time_in_force != "fill_or_kill" && time_in_force != "immediate_or_cancel") {
        throw std::runtime_error("Invalid time in force: " + time_in_force);
    } else if (!time_in_force.empty()) {
        params += "&time_in_force=" + time_in_force;
    }

    if (max_show != -1) {
        params += "&max_show=" + std::to_string(max_show);
    }

    if (post_only && type == "good_til_cancelled") {
        params += "&post_only=true";
    }

    if (reject_post_only && post_only && type == "good_til_cancelled") {
        params += "&reject_post_only=true";
    }

    if (reduce_only) {
        params += "&reduce_only=true";
    }

    if (trigger_price != -1 && (type == "stop_limit" || type == "take_limit" || type == "trailing_stop")) {
        params += "&trigger_price=" + std::to_string(trigger_price);
    } else if (trigger_price != -1) {
        throw std::runtime_error("Trigger price can only be set for order types: stop_limit, take_limit, trailing_stop");
    }

    if (trigger_offset != -1 && type == "trailing_stop") {
        params += "&trigger_offset=" + std::to_string(trigger_offset);
    } else if (trigger_offset != -1) {
        throw std::runtime_error("Trigger offset can only be set for order type: trailing_stop");
    }

    if (trigger != "" && trigger != "index_price" && trigger != "mark_price" && trigger != "last_price") {
        throw std::runtime_error("Invalid trigger: " + trigger);
    } else if (!trigger.empty()) {
        params += "&trigger=" + trigger;
    }

    if (advanced != "" && advanced != "post_only" && advanced != "reduce_only" && advanced != "reject_post_only" && advanced != "trailing_stop" && advanced != "close_on_trigger") {
        throw std::runtime_error("Invalid advanced option: " + advanced);
    } else if (!advanced.empty()) {
        params += "&advanced=" + advanced;
    }

    if (mmp && (type == "limit" || type == "")) {
        params += "&mmp=true";
    } else if (mmp) {
        throw std::runtime_error("MMP can only be set for order type: limit");
    }

    if (valid_until != 0) {
        params += "&valid_until=" + std::to_string(valid_until);
    }

    if (linked_order_type != "" && linked_order_type != "one_triggers_other" && linked_order_type != "one_cancels_other" && linked_order_type != "one_triggers_one_cancels_other") {
        throw std::runtime_error("Invalid linked order type: " + linked_order_type);
    } else if (!linked_order_type.empty()) {
        params += "&linked_order_type=" + linked_order_type;
    }

    if (trigger_fill_condition != "" && trigger_fill_condition != "first_hit" && trigger_fill_condition != "complete_hit" && trigger_fill_condition != "incremental") {
        throw std::runtime_error("Invalid trigger fill condition: " + trigger_fill_condition);
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
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all?detailed=" + std::to_string(detailed) + "&freeze_quotes=" + std::to_string(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByCurrency(const std::string currency, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        throw std::runtime_error("Invalid currency: " + currency);
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        throw std::runtime_error("Invalid kind: " + kind);
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        throw std::runtime_error("Invalid order type: " + type);
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_currency?currency=" + currency + "&kind=" + kind + "&type=" + type + "&detailed=" + std::to_string(detailed) + "&freeze_quotes=" + std::to_string(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByCurrencyPair(const std::string currency_pair, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (std::find(index_price_names.begin(), index_price_names.end(), currency_pair) == index_price_names.end()) {
        throw std::runtime_error("Invalid currency pair: " + currency_pair);
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        throw std::runtime_error("Invalid kind: " + kind);
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        throw std::runtime_error("Invalid order type: " + type);
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_currency_pair?currency_pair=" + currency_pair + "&kind=" + kind + "&type=" + type + "&detailed=" + std::to_string(detailed) + "&freeze_quotes=" + std::to_string(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByInstrument(const std::string instrument_name, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (instruments.find(instrument_name) == instruments.end()) {
        throw std::runtime_error("Instrument not found: " + instrument_name);
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        throw std::runtime_error("Invalid order type: " + type);
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_instrument?instrument_name=" + instrument_name + "&kind=" + kind + "&type=" + type + "&detailed=" + std::to_string(detailed) + "&freeze_quotes=" + std::to_string(freeze_quotes);
    std::string response = client.get(url, this->auth_token);
    JsonValue result = parser.parse(response);
    return result;
}

JsonValue TradingSystem::cancelAllByKindOrType(const std::string currency, const std::string kind,
    const std::string type, bool detailed, bool freeze_quotes) {
    if (currency != "any" && std::find(currencies.begin(), currencies.end(), currency) == currencies.end()) {
        throw std::runtime_error("Invalid currency: " + currency);
    }
    if ((kind != "any" || kind != "combo") && std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
        throw std::runtime_error("Invalid kind: " + kind);
    }
    if (type != "all" && type != "limit" && type != "trigger_all" && type != "stop" && type != "take" && type != "trailing_stop") {
        throw std::runtime_error("Invalid order type: " + type);
    }
    std::string url = "https://test.deribit.com/api/v2/private/cancel_all_by_kind_or_type?currency=" + currency + "&kind=" + kind + "&type=" + type + "&detailed=" + std::to_string(detailed) + "&freeze_quotes=" + std::to_string(freeze_quotes);
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
        throw std::runtime_error("Invalid currency: " + currency);
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
        throw std::runtime_error("No parameters to edit");
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
        throw std::runtime_error("Label cannot be empty");
    }
    if (label.length() > 64) {
        throw std::runtime_error("Label is too long: " + label);
    }
    if (instruments.find(instrument_name) == instruments.end()) {
        throw std::runtime_error("Instrument not found: " + instrument_name);
    }
    if (amount == -1 && contracts == -1 && price == -1 && post_only == -1 && reduce_only == -1 && reject_post_only == -1 && advanced == "" && trigger_price == -1 && trigger_offset == -1 && mmp == -1 && valid_until == 0) {
        throw std::runtime_error("No parameters to edit");
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