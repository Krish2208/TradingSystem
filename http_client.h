#include <curl/curl.h>
#include <string>
#include <iostream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class RestClient {
private:
    CURL* curl;
    std::string response;
    
    void init() {
        curl = curl_easy_init();
        if (curl) {
            // Set common options
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); // For thread safety
            
            // Optimize for low latency
            curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L); // Disable Nagle's algorithm
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 3000L); // 3 second timeout
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000L); // 1 second connect timeout
        }
    }

    // Helper to setup headers with optional auth token
    struct curl_slist* setupHeaders(const std::string* authToken = nullptr, bool isJson = false) const {
        struct curl_slist* headers = nullptr;
        
        if (isJson) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        }
        
        if (authToken && !authToken->empty()) {
            std::string authHeader = "Authorization: Bearer " + *authToken;
            headers = curl_slist_append(headers, authHeader.c_str());
        }
        
        return headers;
    }

public:
    RestClient() {
        init();
    }
    
    ~RestClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }

    std::string get(const std::string& url, const std::string& authToken = "") {
        response.clear();
        if (curl) {
            const std::string* tokenPtr = authToken.empty() ? nullptr : &authToken;
            struct curl_slist* headers = setupHeaders(tokenPtr);
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                return "Error: " + std::string(curl_easy_strerror(res));
            }
        }
        return response;
    }

    std::string post(const std::string& url, const std::string& json_data, const std::string& authToken = "") {
        response.clear();
        if (curl) {
            const std::string* tokenPtr = authToken.empty() ? nullptr : &authToken;
            struct curl_slist* headers = setupHeaders(tokenPtr, true);
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
            
            if (res != CURLE_OK) {
                return "Error: " + std::string(curl_easy_strerror(res));
            }
        }
        return response;
    }
};