#include <string>

class Instrument {
    public:
    std::string base_currency;
    std::string quote_currency;
    std::string kind;
    bool is_active;

    Instrument() {
        this->base_currency = "";
        this->quote_currency = "";
        this->kind = "";
        this->is_active = false;
    }

    Instrument(std::string base_currency, std::string quote_currency, std::string kind, bool is_active) {
        this->base_currency = base_currency;
        this->quote_currency = quote_currency;
        this->kind = kind;
        this->is_active = is_active;
    }
};