# Trading System

## Introduction
This is a trading system that allows users to do the following:
- Get Order Book
- Place Order
- Cancel Order
- Modify Order
- Get Order Status
- Get Open Orders

## Installation
1. Clone the repository
2. Run `sudo apt-get install libcurl4-openssl-dev`
3. Place a file called `secrets.h` in the directory with the following content:
    ```
    namespace secrets {
    const std::string client_id = "YOUR_CLIENT_ID";
    const std::string client_secret = "YOUR_CLIENT_SECRET";
    }
    ``` 
4. Run `make`

## Usage
1. Run `./build/trading_system`