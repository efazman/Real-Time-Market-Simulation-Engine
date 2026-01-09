# Trading Simulator (Event-Driven Matching Engine)

A real-time, event-driven trading simulator / matching engine that consumes **input events** (add/cancel/modify orders) and emits **output events** (order accepted, trade executed). Implemented in **C++17** with a per-symbol limit order book and price-time priority matching.

## What it does
- Maintains a limit order book per symbol (default: **100 symbols**, IDs `0..99`)
- Supports:
  - `AddOrder`
  - `CancelOrder`
  - `ModifyOrder` (implemented as cancel + re-add with updated fields)
- Emits:
  - `OrderAccepted`
  - `TradeExecuted` (with partial fills supported)

## Build & Run
### Build (C++17)
```bash
g++ -std=c++17 main.cpp core.cpp -o core