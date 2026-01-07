#include "coreHeaders.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

int main()
{
    Market market;
    const int NUM_ORDERS = 10000;
    std::vector<Market::InputEvent> events;

    std::mt19937 rng(42); // reproducible
    std::uniform_int_distribution<uint32_t> traderDist(1, 100);
    std::uniform_int_distribution<uint32_t> stockDist(0, 10);
    std::uniform_int_distribution<uint32_t> priceDist(90, 110);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 10);

    uint32_t ts = 1;
    for (int i = 1; i <= NUM_ORDERS; ++i, ++ts)
    {
        Market::Side side = (i % 2 == 0) ? Market::Side::Buy : Market::Side::Sell;
        events.push_back(Market::AddOrder{static_cast<uint>(i), traderDist(rng), stockDist(rng),
                                          priceDist(rng), qtyDist(rng), ts, side});
        // Randomly insert Cancel events
        if (i % 50 == 0)
        {
            events.push_back(Market::CancelOrder{static_cast<uint>(i) - 25, traderDist(rng), ts});
        }
        // Randomly insert Modify events
        if (i % 70 == 0)
        {
            events.push_back(Market::ModifyOrder{static_cast<uint>(i) - 50, priceDist(rng), qtyDist(rng), ts});
        }
    }

    std::vector<long long> latencies;
    auto startTotal = std::chrono::high_resolution_clock::now();

    // Process events one by one and measure per-order latency
    for (auto &ev : events)
    {
        auto start = std::chrono::high_resolution_clock::now();
        market.process({ev});
        auto end = std::chrono::high_resolution_clock::now();
        latencies.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }

    auto endTotal = std::chrono::high_resolution_clock::now();
    auto totalMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTotal - startTotal).count();

    // Calculate P50, P95, P99
    std::sort(latencies.begin(), latencies.end());
    auto p50 = latencies[latencies.size() * 0.50];
    auto p95 = latencies[latencies.size() * 0.95];
    auto p99 = latencies[latencies.size() * 0.99];

    std::cout << "Orders processed: " << events.size() << "\n";
    std::cout << "Total time (us): " << totalMicro << "\n";
    std::cout << "Throughput (orders/sec): " << events.size() * 1e6 / totalMicro << "\n";
    std::cout << "P50 latency (us): " << p50 << "\n";
    std::cout << "P95 latency (us): " << p95 << "\n";
    std::cout << "P99 latency (us): " << p99 << "\n";

    return 0;
}