
#include "coreHeaders.hpp"

#include <type_traits>
#include <algorithm>

vector<Market::OutputEvent>
Market::process(vector<Market::InputEvent> input)
{
    s.resize(100);

    vector<OutputEvent> outputs;
    uint32_t index = 0;

    for (auto &event : input)
    {

        std::visit([&](auto &&e)
                   {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, AddOrder>) {
                Order o;
                o.timestamp  = e.ts;
                o.traderID   = e.trader_id;
                o.stockID    = e.symbol;
                o.priceLimit = e.price;
                o.quantity   = e.quantity;
                o.side       = e.side;
                o.index      = index++;
                o.order_id = e.order_id;
                auto& stock = s[o.stockID];
                if (o.side == Side::Buy)
                    stock.buyerQueue.push(o);
                else
                    stock.sellerQueue.push(o);

                outputs.push_back(OrderAccepted{
                    e.order_id,
                    e.price,
                    e.quantity,
                    e.ts,
                    e.side
                });
                storage[e.order_id] = o;
                // matching happens here
                match(o.stockID, outputs);
            }
            if constexpr (std::is_same_v<T, CancelOrder>){
                if(storage.count(e.order_id)){
                    storage[e.order_id].active = false;
                }
            } 
            if constexpr (std::is_same_v<T, ModifyOrder>){
                if(storage.count(e.order_id)){
                    storage[e.order_id].active = false;
                    Order old = storage[e.order_id];
                old.timestamp  = e.ts;
                old.priceLimit = e.new_price;
                old.quantity   = e.new_quantity;
                old.index      = index++;
                old.order_id = e.order_id;


                auto& stock = s[old.stockID];
                if (old.side == Side::Buy)
                    stock.buyerQueue.push(old);
                else
                    stock.sellerQueue.push(old);

                storage[e.order_id] = old;
                match(old.stockID, outputs);
                outputs.push_back(OrderAccepted{
                    e.order_id,
                    e.new_price,
                    e.new_quantity,
                    e.ts,
                    old.side
                });
                }

            } }, event);
    }

    return outputs;
}

void Market::match(uint32_t stockID, vector<Market::OutputEvent> &outputs)
{

    auto &book = s[stockID];

    static uint32_t executionID = 0;

    while (!book.buyerQueue.empty() && !book.sellerQueue.empty())
    {
        Order buy = book.buyerQueue.top();
        if (!buy.active)
        {
            book.buyerQueue.pop();
            continue;
        }
        Order sell = book.sellerQueue.top();
        if (!sell.active)
        {
            book.sellerQueue.pop();
            continue;
        }
        if (buy.priceLimit < sell.priceLimit)
            break;

        book.buyerQueue.pop();
        book.sellerQueue.pop();

        uint32_t tradeQty = std::min(buy.quantity, sell.quantity);

        uint32_t tradePrice =
            (buy.index < sell.index) ? buy.priceLimit : sell.priceLimit;

        outputs.push_back(TradeExecuted{
            buy.order_id,  // buyOrderID (replace later with real ID)
            sell.order_id, // sellOrderID
            tradePrice,
            tradeQty,
            std::max(buy.timestamp, sell.timestamp),
            executionID++});

        buy.quantity -= tradeQty;
        storage[buy.order_id].quantity -= tradeQty;
        sell.quantity -= tradeQty;
        storage[sell.order_id].quantity -= tradeQty;

        if (storage[sell.order_id].quantity == 0)
            storage[sell.order_id].active = false;
        if (storage[buy.order_id].quantity == 0)
            storage[buy.order_id].active = false;

        if (buy.quantity > 0)
            book.buyerQueue.push(buy);

        if (sell.quantity > 0)
            book.sellerQueue.push(sell);
    }
}