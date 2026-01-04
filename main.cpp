
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include "coreHeaders.hpp"

int main(int argc, char **argv)
{
    Market m;

    std::vector<Market::InputEvent> input;

    input.push_back(Market::AddOrder{
        .order_id = 1,
        .ts = 1,
        .trader_id = 0,
        .symbol = 0,
        .price = 100,
        .quantity = 10,
        .side = Market::Side::Buy});

    input.push_back(Market::AddOrder{
        .order_id = 2,
        .ts = 2,
        .trader_id = 1,
        .symbol = 0,
        .price = 95,
        .quantity = 10,
        .side = Market::Side::Sell});

    auto out = m.process(input);

    for (auto &e : out)
    {
        std::visit([](auto &&ev)
                   {
        using T = std::decay_t<decltype(ev)>;

        if constexpr (std::is_same_v<T, Market::OrderAccepted>) {
            cout << "ACCEPT "
                 << ev.orderID << " "
                 << ev.price << " "
                 << ev.quantity << "\n";
        }

        if constexpr (std::is_same_v<T, Market::TradeExecuted>) {
            cout << "TRADE "
                 << ev.buyOrderID << " "
                 << ev.sellOrderID << " "
                 << ev.price << " "
                 << ev.quantity << "\n";
        } }, e);
    }
}
