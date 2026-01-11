#include "coreHeaders.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

#include <iostream>
#include <string>
#include <stdexcept>

static uint64_t percentile(std::vector<uint64_t> &v, double p)
{
    if (v.empty())
        return 0;
    std::sort(v.begin(), v.end());
    size_t idx = (size_t)((v.size() - 1) * p);
    return v[idx];
}
static void require(bool cond, const char *msg)
{
    if (!cond)
        throw std::runtime_error(msg);
}

static uint32_t count_trades(const std::vector<Market::OutputEvent> &out)
{
    uint32_t c = 0;
    for (auto &ev : out)
        if (std::get_if<Market::TradeExecuted>(&ev))
            ++c;
    return c;
}

static std::vector<Market::TradeExecuted> collect_trades(const std::vector<Market::OutputEvent> &out)
{
    std::vector<Market::TradeExecuted> trades;
    for (auto &ev : out)
    {
        if (auto *t = std::get_if<Market::TradeExecuted>(&ev))
            trades.push_back(*t);
    }
    return trades;
}

static void validate_storage_invariants(Market &m)
{
    for (auto &kv : m.storage)
    {
        const auto &o = kv.second;
        if (o.active)
        {
            require(o.quantity > 0, "Invariant failed: active order has zero quantity");
        }
    }
}

// Scenario 1: simple cross -> partial fill
static void test_simple_trade_partial_fill()
{
    Market m;

    std::vector<Market::InputEvent> events;
    // Buy 10 @ 100
    events.push_back(Market::AddOrder{1, 1, 0, 100, 10, 1, Market::Side::Buy});
    // Sell 5 @ 95 crosses
    events.push_back(Market::AddOrder{2, 2, 0, 95, 5, 2, Market::Side::Sell});

    auto out = m.process(events);
    auto trades = collect_trades(out);

    require(trades.size() == 1, "Expected exactly 1 trade in simple cross");
    require(trades[0].buyOrderID == 1, "Expected buy order 1 to execute");
    require(trades[0].sellOrderID == 2, "Expected sell order 2 to execute");
    require(trades[0].quantity == 5, "Expected trade quantity 5");

    // Post-trade state: buy should have 5 remaining and still active, sell should be inactive (0 left)
    require(m.storage.count(1) && m.storage.count(2), "Storage missing expected order IDs");
    require(m.storage[1].active == true, "Buy order should remain active after partial fill");
    require(m.storage[1].quantity == 5, "Buy order remaining quantity should be 5");
    require(m.storage[2].active == false, "Sell order should be inactive after full fill");
    require(m.storage[2].quantity == 0, "Sell order remaining quantity should be 0");

    validate_storage_invariants(m);
}

// Scenario 2: time priority on same price
static void test_price_time_priority()
{
    Market m;

    std::vector<Market::InputEvent> events;
    // Two buys same price: order 1 arrives before order 2
    events.push_back(Market::AddOrder{1, 1, 0, 100, 5, 1, Market::Side::Buy});
    events.push_back(Market::AddOrder{2, 1, 0, 100, 5, 2, Market::Side::Buy});
    // One sell crosses for qty 5
    events.push_back(Market::AddOrder{3, 2, 0, 90, 5, 3, Market::Side::Sell});

    auto out = m.process(events);
    auto trades = collect_trades(out);

    require(trades.size() == 1, "Expected exactly 1 trade for time priority test");
    require(trades[0].buyOrderID == 1, "Expected earlier buy (order 1) to execute first");
    require(m.storage[1].active == false && m.storage[1].quantity == 0, "Order 1 should be fully filled");
    require(m.storage[2].active == true && m.storage[2].quantity == 5, "Order 2 should remain untouched");

    validate_storage_invariants(m);
}

// Scenario 3: cancel prevents execution
static void test_cancel_prevents_trade()
{
    Market m;

    std::vector<Market::InputEvent> events;
    events.push_back(Market::AddOrder{1, 1, 0, 100, 5, 1, Market::Side::Buy});
    events.push_back(Market::CancelOrder{1, 1, 2});                            // cancel buy
    events.push_back(Market::AddOrder{2, 2, 0, 90, 5, 3, Market::Side::Sell}); // would cross if buy were active

    auto out = m.process(events);

    require(count_trades(out) == 0, "Expected 0 trades because buy was cancelled");
    require(m.storage[1].active == false, "Cancelled order should be inactive");
    require(m.storage[2].active == true, "Sell order should remain active (no match)");

    validate_storage_invariants(m);
}

static int run_checks()
{
    try
    {
        test_simple_trade_partial_fill();
        test_price_time_priority();
        test_cancel_prevents_trade();
        std::cout << "CHECKS PASSED\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "CHECKS FAILED: " << e.what() << "\n";
        return 1;
    }
}

int main(int argc, char **argv)
{
    if (argc > 1 && std::string(argv[1]) == "--check")
    {
        return run_checks();
    }
    Market market;

    const int NUM_ORDERS = 10000;
    std::vector<Market::InputEvent> events;
    events.reserve(NUM_ORDERS + NUM_ORDERS / 50 + NUM_ORDERS / 70);

    std::mt19937 rng(42);
    std::uniform_int_distribution<uint32_t> traderDist(1, 100);
    std::uniform_int_distribution<uint32_t> stockDist(0, 10);
    std::uniform_int_distribution<uint32_t> priceDist(90, 110);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 10);

    uint32_t ts = 1;
    for (int i = 1; i <= NUM_ORDERS; ++i, ++ts)
    {
        Market::Side side = (i % 2 == 0) ? Market::Side::Buy : Market::Side::Sell;
        events.push_back(Market::AddOrder{
            (uint32_t)i, traderDist(rng), stockDist(rng),
            priceDist(rng), qtyDist(rng), ts, side});

        // Cancel events (note: your Modify logic is currently incorrect; avoid Modify in bench for now)
        if (i % 50 == 0)
        {
            events.push_back(Market::CancelOrder{(uint32_t)(i - 25), traderDist(rng), ts});
        }
    }

    // Enable timing
    market.timing_enabled = true;
    market.match_step_ns.clear();
    market.match_step_ns.reserve(NUM_ORDERS); // rough guess

    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();
    auto out = market.process(events);
    auto t1 = clock::now();

    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    double seconds = (double)total_ns / 1e9;

    std::cout << "Events processed: " << events.size() << "\n";
    std::cout << "Total time (s): " << seconds << "\n";
    std::cout << "Throughput (events/sec): " << (events.size() / seconds) << "\n";

    // Matching-step percentiles
    auto &ms = market.match_step_ns;
    uint64_t p50 = percentile(ms, 0.50);
    uint64_t p95 = percentile(ms, 0.95);
    uint64_t p99 = percentile(ms, 0.99);

    std::cout << "Match-step samples: " << ms.size() << "\n";
    std::cout << "Match-step P50 (ns): " << p50 << "\n";
    std::cout << "Match-step P95 (ns): " << p95 << "\n";
    std::cout << "Match-step P99 (ns): " << p99 << "\n";
}