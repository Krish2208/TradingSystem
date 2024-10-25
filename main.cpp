#include <iostream>
#include <limits>
#include <sstream>
#include <map>
#include "trading_system.h"
#include "json_utils.h"

class TradingCLI {
private:
    TradingSystem trading;

    void clearScreen() {
        system("clear");
    }

    void waitForEnter(bool check = true) {
        std::cout << "\nPress Enter to continue...";
        if(check) std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    void displayMainMenu() {
        std::cout << "=== Trading System CLI ===\n";
        std::cout << "1. Order Book Operations\n";
        std::cout << "2. Place Orders\n";
        std::cout << "3. Cancel Orders\n";
        std::cout << "4. Edit Orders\n";
        std::cout << "5. View Positions\n";
        std::cout << "6. View Order States\n";
        std::cout << "0. Exit\n";
        std::cout << "Enter your choice: ";
    }

    void handleOrderBook() {
        std::string instrument_name;
        int depth;

        std::cout << "=== Order Book Operations ===\n";
        std::cout << "Enter instrument name: ";
        std::cin >> instrument_name;
        std::cout << "Enter depth (default 5): ";
        std::cin >> depth;

        try {
            JsonValue result = trading.getOrderBook(instrument_name, depth);
            if (!result.isNull()) {
                std::cout << "\nOrder Book Result:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter();
    }

    struct OrderParams {
        std::string instrument_name;
        int amount = 0;
        int contracts = 0;
        std::string type = "";
        std::string label = "";
        int price = -1;
        std::string time_in_force = "";
        int max_show = -1;
        int post_only = -1;
        int reject_post_only = -1;
        int reduce_only = -1;
        int trigger_price = -1;
        int trigger_offset = -1;
        std::string trigger = "";
        std::string advanced = "";
        int mmp = -1;
        int valid_until = 0;
        std::string linked_order_type = "";
        std::string trigger_fill_condition = "";
    };

    void parseAdditionalParams(const std::string& input, OrderParams& params) {
        std::istringstream ss(input);
        std::string param;
        
        while (std::getline(ss, param, ',')) {
            // Trim whitespace
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
            
            size_t equals_pos = param.find('=');
            if (equals_pos != std::string::npos) {
                std::string key = param.substr(0, equals_pos);
                std::string value = param.substr(equals_pos + 1);
                
                // Trim quotes if present
                if (value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }

                // Map parameter to struct field
                if (key == "type") params.type = value;
                else if (key == "label") params.label = value;
                else if (key == "time_in_force") params.time_in_force = value;
                else if (key == "trigger") params.trigger = value;
                else if (key == "advanced") params.advanced = value;
                else if (key == "linked_order_type") params.linked_order_type = value;
                else if (key == "trigger_fill_condition") params.trigger_fill_condition = value;
                // Parse integer values
                else {
                    try {
                        int int_value = std::stoi(value);
                        if (key == "max_show") params.max_show = int_value;
                        else if (key == "post_only") params.post_only = int_value;
                        else if (key == "reject_post_only") params.reject_post_only = int_value;
                        else if (key == "reduce_only") params.reduce_only = int_value;
                        else if (key == "trigger_price") params.trigger_price = int_value;
                        else if (key == "trigger_offset") params.trigger_offset = int_value;
                        else if (key == "mmp") params.mmp = int_value;
                        else if (key == "valid_until") params.valid_until = int_value;
                    } catch (const std::exception& e) {
                        std::cout << "Warning: Invalid value for " << key << std::endl;
                    }
                }
            }
        }
    }

    void displayOrderParams(bool isEdit = false) {
        if (isEdit) {
            std::cout << "\nAvailable parameters to modify (comma-separated, field=value format):\n";
            std::cout << "String parameters:\n";
            std::cout << "  advanced\n";
            std::cout << "Integer parameters:\n";
            std::cout << "  amount, contracts, price, trigger_price, trigger_offset, valid_until\n";
            std::cout << "Boolean (0 or 1) parameters:\n";
            std::cout << "  post_only, reject_post_only, reduce_only, mmp\n";
            std::cout << "Example: amount=10, price=1000\n\n";
        } else {
            std::cout << "\nAvailable parameters (comma-separated, field=value format):\n";
            std::cout << "String parameters:\n";
            std::cout << "  type, label, time_in_force, trigger, advanced, linked_order_type, trigger_fill_condition\n";
            std::cout << "Integer parameters:\n";
            std::cout << "  max_show, trigger_price, trigger_offset, valid_until\n";
            std::cout << "Boolean (0 or 1) parameters:\n";
            std::cout << "  post_only, reject_post_only, reduce_only, mmp\n";
            std::cout << "Example: type=\"limit\", label=\"my order\", time_in_force=\"good_til_cancelled\"\n\n";
        }
    }

    void handlePlaceOrders() {
        
        std::cout << "=== Place Orders ===\n";
        std::cout << "1. Buy Order\n";
        std::cout << "2. Sell Order\n";
        std::cout << "0. Back\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) return;

        OrderParams params;
        std::cout << "Enter instrument name: ";
        std::getline(std::cin, params.instrument_name);
        std::cout << "Enter amount: ";
        std::cin >> params.amount;
        std::cout << "Enter contracts: ";
        std::cin >> params.contracts;
        std::cout << "Enter price (-1 for market order): ";
        std::cin >> params.price;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        displayOrderParams();
        std::cout << "Enter additional parameters (press Enter to skip): ";
        std::string additional_params;
        std::getline(std::cin, additional_params);

        if (!additional_params.empty()) {
            parseAdditionalParams(additional_params, params);
        }

        try {
            JsonValue result;
            if (choice == 1) {
                result = trading.buy(params.instrument_name, params.amount, params.contracts,
                    params.type, params.label, params.price, params.time_in_force,
                    params.max_show, params.post_only, params.reject_post_only,
                    params.reduce_only, params.trigger_price, params.trigger_offset,
                    params.trigger, params.advanced, params.mmp, params.valid_until,
                    params.linked_order_type, params.trigger_fill_condition);
            } else if (choice == 2) {
                result = trading.sell(params.instrument_name, params.amount, params.contracts,
                    params.type, params.label, params.price, params.time_in_force,
                    params.max_show, params.post_only, params.reject_post_only,
                    params.reduce_only, params.trigger_price, params.trigger_offset,
                    params.trigger, params.advanced, params.mmp, params.valid_until,
                    params.linked_order_type, params.trigger_fill_condition);
            }
            if (!result.isNull()) {
                std::cout << "\nPlace Order Result:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter(0);
    }

    void handleCancelOrders() {
        
        std::cout << "=== Cancel Orders ===\n";
        std::cout << "1. Cancel Single Order\n";
        std::cout << "2. Cancel All Orders\n";
        std::cout << "3. Cancel by Currency\n";
        std::cout << "4. Cancel by Instrument\n";
        std::cout << "5. Cancel by Label\n";
        std::cout << "0. Back\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        try {
            JsonValue result;
            switch (choice) {
                case 1: {
                    std::string order_id;
                    std::cout << "Enter order ID: ";
                    std::cin >> order_id;
                    result = trading.cancel(order_id);
                    break;
                }
                case 2:
                    result = trading.cancelAll();
                    break;
                case 3: {
                    std::string currency;
                    std::cout << "Enter currency: ";
                    std::cin >> currency;
                    result = trading.cancelAllByCurrency(currency);
                    break;
                }
                case 4: {
                    std::string instrument;
                    std::cout << "Enter instrument name: ";
                    std::cin >> instrument;
                    result = trading.cancelAllByInstrument(instrument);
                    break;
                }
                case 5: {
                    std::string label;
                    std::cout << "Enter label: ";
                    std::cin >> label;
                    result = trading.cancelByLabel(label);
                    break;
                }
                case 0:
                    return;
            }
            if (!result.isNull()) {
                std::cout << "\nCancel Result:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter();
    }

    void handleEditOrders() {
        
        std::cout << "=== Edit Orders ===\n";
        std::cout << "1. Edit by Order ID\n";
        std::cout << "2. Edit by Label\n";
        std::cout << "0. Back\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) return;

        try {
            JsonValue result;
            if (choice == 1) {
                std::string order_id;
                std::cout << "Enter order ID: ";
                std::getline(std::cin, order_id);

                OrderParams params;
                displayOrderParams(1);
                std::cout << "Enter parameters to modify (press Enter to skip): ";
                std::string additional_params;
                std::getline(std::cin, additional_params);

                if (!additional_params.empty()) {
                    parseAdditionalParams(additional_params, params);
                }

                result = trading.edit(order_id, params.amount, params.contracts, params.price,
                    params.post_only, params.reduce_only, params.reject_post_only,
                    params.advanced, params.trigger_price, params.trigger_offset,
                    params.mmp, params.valid_until);
            } else if (choice == 2) {
                std::string label, instrument;
                std::cout << "Enter label: ";
                std::getline(std::cin, label);
                std::cout << "Enter instrument name: ";
                std::getline(std::cin, instrument);

                OrderParams params;
                displayOrderParams(1);
                std::cout << "Enter parameters to modify (press Enter to skip): ";
                std::string additional_params;
                std::getline(std::cin, additional_params);

                if (!additional_params.empty()) {
                    parseAdditionalParams(additional_params, params);
                }

                result = trading.editByLabel(label, instrument, params.amount, params.contracts,
                    params.price, params.post_only, params.reduce_only, params.reject_post_only,
                    params.advanced, params.trigger_price, params.trigger_offset,
                    params.mmp, params.valid_until);
            }
            if (!result.isNull()) {
                std::cout << "\nEdit Result:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter();
    }

    void handleViewPositions() {
        
        std::cout << "=== View Positions ===\n";
        std::cout << "1. View All Open Orders\n";
        std::cout << "2. View Orders by Currency\n";
        std::cout << "3. View Orders by Instrument\n";
        std::cout << "4. View Orders by Label\n";
        std::cout << "0. Back\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        try {
            JsonValue result;
            switch (choice) {
                case 1:
                    result = trading.getOpenOrders();
                    break;
                case 2: {
                    std::string currency;
                    std::cout << "Enter currency: ";
                    std::cin >> currency;
                    result = trading.getOpenOrdersByCurrency(currency);
                    break;
                }
                case 3: {
                    std::string instrument;
                    std::cout << "Enter instrument name: ";
                    std::cin >> instrument;
                    result = trading.getOpenOrdersByInstrument(instrument);
                    break;
                }
                case 4: {
                    std::string currency, label;
                    std::cout << "Enter currency: ";
                    std::cin >> currency;
                    std::cout << "Enter label: ";
                    std::cin >> label;
                    result = trading.getOpenOrdersByLabel(currency, label);
                    break;
                }
                case 0:
                    return;
            }
            if (!result.isNull()) {
                std::cout << "\nOpen Orders:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter();
    }

    void handleViewOrderStates() {
        
        std::cout << "=== View Order States ===\n";
        std::cout << "1. View by Order ID\n";
        std::cout << "2. View by Label\n";
        std::cout << "0. Back\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        try {
            JsonValue result;
            if (choice == 1) {
                std::string order_id;
                std::cout << "Enter order ID: ";
                std::cin >> order_id;
                result = trading.getOrderState(order_id);
            } else if (choice == 2) {
                std::string currency, label;
                std::cout << "Enter currency: ";
                std::cin >> currency;
                std::cout << "Enter label: ";
                std::cin >> label;
                result = trading.getOrderStateByLabel(currency, label);
            }
            if (!result.isNull()) {
                std::cout << "\nOrder State:\n";
                json_utils::printJson(result, 2);
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        waitForEnter();
    }

public:
    void run() {
        while (true) {
            displayMainMenu();
            
            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    handleOrderBook();
                    break;
                case 2:
                    handlePlaceOrders();
                    break;
                case 3:
                    handleCancelOrders();
                    break;
                case 4:
                    handleEditOrders();
                    break;
                case 5:
                    handleViewPositions();
                    break;
                case 6:
                    handleViewOrderStates();
                    break;
                case 0:
                    std::cout << "Goodbye!\n";
                    return;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
                    waitForEnter();
            }
        }
    }
};

int main() {
    TradingCLI cli;
    cli.run();
    return 0;
}