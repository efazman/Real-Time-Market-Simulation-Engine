#ifndef STOCKS_HPP
#define STOCKS_HPP
#include <vector>
#include <queue>
#include <cstdint>
#include <variant>
#include <unordered_map>
using namespace std;
class Market
{
public:
    //** DEFINING INPUTS AND OUTPUTS FOR ENGINE ITSELF BEGIN */
    enum class Side : uint8_t
    {
        Buy,
        Sell
    };
    struct Order
    {
        uint32_t timestamp, traderID, stockID, priceLimit, quantity, index;
        bool active = true;
        Side side;
    };

    struct AddOrder
    {
        uint32_t order_id;
        uint32_t trader_id;
        uint32_t symbol;
        uint32_t price;
        uint32_t quantity;
        uint32_t ts;
        Side side;
    };

    struct CancelOrder
    {
        uint32_t order_id;
        uint32_t trader_id;
        uint32_t ts;
    };

    struct ModifyOrder
    {
        uint32_t order_id;
        uint32_t new_price;
        uint32_t new_quantity;
        uint32_t ts;
    };

    using InputEvent = std::variant<
        AddOrder,
        CancelOrder,
        ModifyOrder>;

    struct TradeExecuted
    {
        uint32_t buyOrderID;
        uint32_t sellOrderID;
        uint32_t price;
        uint32_t quantity;
        uint32_t timestamp;
        uint32_t executionID;
    };

    struct OrderAccepted
    {
        uint32_t orderID;
        uint32_t price;
        uint32_t quantity;
        uint32_t timestamp;
        Side side;
    };

    using OutputEvent = std::variant<
        TradeExecuted,
        OrderAccepted>;

    //** DEFINING INPUTS AND OUTPUTS FOR ENGINE ITSELF END */
    //** MATCHING ENGINE (BOOK KEEPING) BEGIN */
    struct CompareBuy
    {
        bool operator()(const Order &a, const Order &b) const
        {
            if (a.priceLimit == b.priceLimit)
                return a.index > b.index;
            return a.priceLimit < b.priceLimit;
        }
    };

    struct CompareSell
    {
        bool operator()(const Order &a, const Order &b) const
        {
            if (a.priceLimit == b.priceLimit)
                return a.index > b.index;
            return a.priceLimit > b.priceLimit;
        }
    };
    struct Stock
    {
        priority_queue<Market::Order, vector<Order>, CompareBuy> buyerQueue;
        priority_queue<Market::Order, vector<Order>, CompareSell> sellerQueue;
    };
    vector<Stock> s;
    unordered_map<uint32_t, Order> storage;
    //** MATCHING ENGINE (BOOK KEEPING) END */

private:
public:
    vector<OutputEvent> process(vector<InputEvent> input);
    void match(uint32_t stockID, vector<Market::OutputEvent> &outputs);
};

#endif
