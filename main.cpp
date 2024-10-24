// #include "json_parser.h"
// #include "json_utils.h"
// #include "http_client.h"

// #include <iostream>

// int main() {
//     // Initialize global curl environment
//     curl_global_init(CURL_GLOBAL_ALL);
    
//     RestClient client;
    
//     // Example GET request
//     // std::string get_response = client.get("https://test.deribit.com/api/v2/public/auth?client_id=U0uw6PRz&client_secret=fUhpH4TxSQRg2EX8YurJpLEuHisI7c5oBp7R8dMpp-4&grant_type=client_credentials");
//     std::string get_response = client.get("https://test.deribit.com/api/v2/public/get_currencies?");
//     // std::cout << "GET Response: " << get_response << std::endl;
    
//     // // Example POST request with JSON data
//     // std::string json = "{\"title\": \"foo\", \"body\": \"bar\", \"userId\": 1}";
//     // std::string post_response = client.post("https://jsonplaceholder.typicode.com/posts", json);
//     // std::cout << "POST Response: " << post_response << std::endl;
    
//     JsonParser parser;
//     JsonValue result = parser.parse(get_response);
//     json_utils::printJson(result);

//     // Cleanup global curl environment
//     curl_global_cleanup();
//     return 0;
// }

#include "trading_system.h"
#include "json_utils.h"

int main() {
    TradingSystem trading_system;
    trading_system.test();
    // JsonValue result = trading_system.sell("BTC-PERPETUAL", 0, 5, "market");
    // json_utils::printJson(result);
    return 0;
}