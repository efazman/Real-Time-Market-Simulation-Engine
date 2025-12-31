
#include "coreHeaders.hpp"

void Market::readInput()
{
    string input;
    string mode;
    getline(cin, input);
    cin >> input >> mode >> input >> (numTraders) >> input >> numStocks;
    s.resize(numStocks);
    if (m)
        stockMedians.resize(numStocks);
    if (t)
        timeTravelers.resize(numStocks);

    if (i)
        tradeVec.resize(numTraders);

    stringstream ss;

    if (mode == "PR")
    {
        string seedLabel, numOrderLabel, rateLabel;
        string buffer;
        
        cin >> seedLabel >> seed >> numOrderLabel >> numOrder >> rateLabel >> rate;
        P2random::PR_init(ss, seed, static_cast<uint>(numTraders), static_cast<uint>(numStocks), numOrder, rate);
    }

    if (mode == "PR")
        processOrders(ss);
    else
        processOrders(cin);
}
void Market::processOrders(istream &inputStream)
{
    Order tempOrder;
    string buyOrSell;
    char discard;
    uint32_t index = 0;
    uint32_t prevTimeStamp = 0;
    CURRENT_TIMESTAMP = 0;

    bool processingCount = false;

    while (inputStream >> tempOrder.timestamp >> buyOrSell >> discard >> tempOrder.traderID >> discard >> tempOrder.stockID >> discard >> tempOrder.priceLimit >> discard >> tempOrder.quantity)
    {
        if (!processingCount)
        {
            processingCount = 1;
            cout << "Processing orders..." << "\n";
        }

        if (prevTimeStamp > tempOrder.timestamp)
        {
            std::cerr << ("Error: Decreasing timestamp");
            exit(1);
        }
        prevTimeStamp = tempOrder.timestamp;
        if (tempOrder.traderID >= numTraders)
        {
            std::cerr << ("Error: Invalid trader ID");
            exit(1);
        }
        if (tempOrder.stockID >= numStocks)
        {
            std::cerr << ("Error: Invalid stock ID");
            exit(1);
        }
        if (tempOrder.priceLimit <= 0)
        {
            std::cerr << ("Error: Invalid price");
            exit(1);
        }
        if (tempOrder.quantity <= 0)
        {
            std::cerr << ("Error: Invalid quantity");
            exit(1);
        } 

        tempOrder.isBuy = (buyOrSell == "BUY");
        tempOrder.index = index++;

         if (tempOrder.timestamp != CURRENT_TIMESTAMP)
        {
            if (m)
            {
                for (size_t j = 0; j < numStocks; ++j)
                {
                    if (!stockMedians[j].maxHeap.empty())
                    {
                        uint32_t median = stockMedians[j].getMedian();
                        cout << "Median match price of Stock " << j << " at time "
                             << CURRENT_TIMESTAMP << " is $" << median << '\n';
                    }
                }
            }

            CURRENT_TIMESTAMP = tempOrder.timestamp;
        }

        if (t)
            processTimeTraveler(tempOrder, timeTravelers[tempOrder.stockID]);
        auto &buyerQueue = s[tempOrder.stockID].buyerQueue;
        auto &sellerQueue = s[tempOrder.stockID].sellerQueue;
        if (tempOrder.isBuy)
            buyerQueue.push(tempOrder);
        else
            sellerQueue.push(tempOrder);

        while (!buyerQueue.empty() && !sellerQueue.empty())
        {
            Order curBuyer = buyerQueue.top();
            Order curSeller = sellerQueue.top();

            if (curBuyer.priceLimit >= curSeller.priceLimit)
            {
                buyerQueue.pop();
                sellerQueue.pop();
                uint32_t matchPrice, tradeQuantity;
                if (curBuyer.index < curSeller.index)
                {
                    matchPrice = curBuyer.priceLimit;
                }
                else
                {
                    matchPrice = curSeller.priceLimit;
                }

                if (curBuyer.quantity < curSeller.quantity)
                {
                    tradeQuantity = (curBuyer.quantity);
                }
                else
                {
                    tradeQuantity = (curSeller.quantity);
                }
                if (tradeQuantity == 0)
                    break;
                
                if (i)
                {
                    
                    tradeVec[curBuyer.traderID].boughtShares += (tradeQuantity);
                    tradeVec[curSeller.traderID].soldShares += (tradeQuantity);

                    tradeVec[curBuyer.traderID].netTransfer -= (static_cast<int>(tradeQuantity * matchPrice));
                    tradeVec[curSeller.traderID].netTransfer += (static_cast<int>(tradeQuantity * matchPrice));
                }
                if (v)
                {
                    std::cout << "Trader " << curBuyer.traderID << " purchased " << tradeQuantity
                              << " shares of Stock " << curBuyer.stockID << " from Trader "
                              << curSeller.traderID << " for $" << matchPrice << "/share" << "\n";
                }

                if (m)
                    stockMedians[tempOrder.stockID].insert(matchPrice);

                curBuyer.quantity -= tradeQuantity;
                curSeller.quantity -= tradeQuantity;

                if (curBuyer.quantity > 0)
                {
                    buyerQueue.push(curBuyer);
                }
                if (curSeller.quantity > 0)
                {
                    sellerQueue.push(curSeller);
                }
                tradesCompleted++;
                
            }
            else
            {
                break;
            }
        }
    }
}
void Market::processTimeTraveler(Order &o, TimeTravelerState &st)
{
    if (!o.isBuy)
    {
        if (o.priceLimit < st.minSellPrice ||
            (o.priceLimit == st.minSellPrice && o.timestamp < st.minSellTime))
        {
            st.minSellPrice = o.priceLimit;
            st.minSellTime = o.timestamp;
            st.status = TimeStatus::CanBuy;
        }
        return;
    }

    if (st.minSellTime == UINT32_MAX) 
        return;
    if (st.minSellTime > o.timestamp)
        return;
    //
    uint32_t b = (o.priceLimit), s = (st.minSellPrice);
    int32_t profit = static_cast<int32_t>(b) - static_cast<int32_t>(s);
    if (profit <= int32_t(0))
        return;

    if (profit > st.profit ||
        (profit == st.profit && st.minSellTime < st.sellTime) ||
        (profit == st.profit && st.minSellTime == st.sellTime && o.timestamp < st.buyTime))
    {
        st.status = TimeStatus::Completed;
        st.profit = profit;
        st.sellTime = st.minSellTime;
        st.sellPrice = st.minSellPrice;
        st.buyTime = o.timestamp;
        st.buyPrice = o.priceLimit;
    }
}
void Market::runOutput()
{
    if (m)
    {
        for (size_t i = 0; i < numStocks; ++i)
        {
            if (!stockMedians[i].maxHeap.empty())
            {
                uint32_t median = stockMedians[i].getMedian();
                cout << "Median match price of Stock " << i << " at time "
                     << CURRENT_TIMESTAMP << " is $" << median << '\n';
            }
        }
    }

    cout << "---End of Day---" << "\n"
         << "Trades Completed: " << tradesCompleted << "\n";

    
    if (i)
    {
        cout << "---Trader Info---" << "\n";
        for (size_t i = 0; i < numTraders; ++i)
        {
            cout << "Trader " << i << " bought " << tradeVec[i].boughtShares
                 << " and sold " << tradeVec[i].soldShares
                 << " for a net transfer of $" << tradeVec[i].netTransfer << "\n";
        }
    }

    if (t)
    {
        cout << "---Time Travelers---" << "\n";
        for (size_t i = 0; i < numStocks; ++i)
        {
            Market::TimeTravelerState &state = timeTravelers[i];
            if (state.profit > 0)
            {

                cout << "A time traveler would buy Stock " << i
                     << " at time " << state.sellTime << " for $"
                     << state.sellPrice << " and sell it at time " << state.buyTime
                     << " for $" << state.buyPrice << '\n';
            }
            else
            {
                cout << "A time traveler could not make a profit on Stock " << i << '\n';
            }
        }
    }

    }