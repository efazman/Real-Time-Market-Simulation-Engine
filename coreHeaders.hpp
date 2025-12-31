
#ifndef STOCKS_HPP
#define STOCKS_HPP

#include <vector>
#include <iostream>
#include <algorithm>
#include <getopt.h>
#include <string>
#include <queue>

#include <climits> //from AG

using namespace std;
class Market
{
public:
    struct Order
    {
        /**Timestamp - the timestamp that this order comes in
        Trader ID - the trader who is issuing the order
        Stock ID - the stock that the trader is interested in
        Buy/Sell Intent - whether the trader wants to buy or sell shares
        Price Limit - the max/min amount the trader is willing to pay/receive per share
        Quantity - the number of shares the trader is interested in */

        uint32_t timestamp, traderID, stockID, priceLimit, quantity, index;
        bool isBuy; // if false sell
        // size_t sharesBought = 0;
        // size_t sharesSold = 0;
        // size_t netProfit = 0;
    };
    // For buy maxheap, higher price watned
    struct CompareBuy
    {
        bool operator()(const Order &a, const Order &b) const
        {
            if (a.priceLimit == b.priceLimit)
                return a.index > b.index;
            return a.priceLimit < b.priceLimit;
        }
    };

    // For sell orders minheap, lower price wanted
    struct CompareSell
    {
        bool operator()(const Order &a, const Order &b) const
        {
            if (a.priceLimit == b.priceLimit)
                return a.index > b.index;
            return a.priceLimit > b.priceLimit;
        }
    };

    //
    struct Stock
    {
        priority_queue<Market::Order, vector<Order>, CompareBuy> buyerQueue;
        priority_queue<Market::Order, vector<Order>, CompareSell> sellerQueue;
    };
    vector<Stock> s;

    ///
    struct MedianTracker
    {
        priority_queue<uint32_t> maxHeap;
        priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>> minHeap;

        void insert(uint32_t price)
        {
            if (maxHeap.empty() || price <= maxHeap.top())
            {
                maxHeap.push(price);
            }
            else
            {
                minHeap.push(price);
            }

            // rebalance if uneven
            if (maxHeap.size() > minHeap.size() + 1)
            {
                minHeap.push(maxHeap.top());
                maxHeap.pop();
            }
            else if (minHeap.size() > maxHeap.size())
            {
                maxHeap.push(minHeap.top());
                minHeap.pop();
            }
        }

        uint32_t getMedian() const
        {
            if (maxHeap.size() == minHeap.size())
                return (maxHeap.top() + minHeap.top()) / 2;
            else
                return maxHeap.top();
        }
    };
    vector<MedianTracker> stockMedians;

    ///

    void get_options(int argc, char **argv);
    void readInput();
    void runOutput();
    // for trader info output
    // vector<size_t> boughtShares;
    // vector<size_t> soldShares;
    // vector<int> netTransfer;

    struct traderObject
    {
        int netTransfer = 0;
        uint32_t boughtShares = 0, soldShares = 0;
    };
    vector<traderObject> tradeVec;

private:
    // void readTL();
    void processOrders(istream &inputStream);
    // void readPR(stringstream ss);
    bool v = 0, m = 0, i = 0, t = 0;
    uint32_t numTraders, numStocks, CURRENT_TIMESTAMP, tradesCompleted = 0;
    uint seed, numOrder, rate; // random only
    ///////////////////////////////////////////////////////////////////////////////
    // timetraveler
    enum class TimeStatus : char16_t
    {
        NoTrades,
        CanBuy,
        Completed,
        Potential //?
    };

    struct TimeTravelerState
    {
        TimeStatus status = TimeStatus::NoTrades;

        uint32_t minSellTime = UINT32_MAX, minSellPrice = UINT32_MAX, buyTime = UINT32_MAX, buyPrice = UINT32_MAX, sellTime = UINT32_MAX, sellPrice = UINT32_MAX;
        int32_t profit = INT32_MIN;
    };

    vector<TimeTravelerState> timeTravelers;

public:
    void processTimeTraveler(Order &o, TimeTravelerState &state);
    ///////////////////////////////////////////////////////////////////////////////
    //
};
///

#endif // STOCKS_HPP
