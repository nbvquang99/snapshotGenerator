#include <string>
#include <string.h>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "avl.hpp"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        error("ERROR not enough arguments\n");
    }
    if (strcmp(argv[1], "READ") == 0) {
        if (argc < 3) error("ERROR not enough arguments\n");

        // Declare field to read
        std::ios::sync_with_stdio(false);
	    std::cin.tie(nullptr);
        freopen(argv[2], "r", stdin);
        long long epoch;
        std::string id;
        std::string symbol;
        std::string side;
        std::string category;
        float price;
        int quantity;

        // time measuring
        auto start = std::chrono::high_resolution_clock::now();

        // Init or load the books
        std::cin >> epoch >> id >> symbol >> side >> category >> price >> quantity;
        // std::cout << "1 " << epoch << " " << id << " " << symbol << " " << side << " " << category << " " << price << " " << quantity << std::endl;
        // std::cout << std::endl;
        Book* buyBook = initAVL(symbol, "BUY");
        Book* sellBook = initAVL(symbol, "SELL");
        initArr(buyBook, symbol, "BUY");
        initArr(sellBook, symbol, "SELL");

        if (side == "SELL") updateBook(sellBook, epoch, price, quantity, category);
        if (side == "BUY") updateBook(buyBook, epoch, price, quantity, category);

        int count = 1;

        while (std::cin >> epoch >> id >> symbol >> side >> category >> price >> quantity) {
            // std::cout << ++count << " " << epoch << " " << id << " " << symbol << " " << side << " " << category << " " << price << " " << quantity << std::endl;
            if (side == "SELL") updateBook(sellBook, epoch, price, quantity, category);
            else if (side == "BUY") updateBook(buyBook, epoch, price, quantity, category);
            // std::cout << std::endl;
        }
        std::cout << "Number of nodes in sellBook: " << sellBook->nNode << std::endl;
        std::cout << "Number of roots in sellBook: " <<sellBook->nRoot << std::endl;
        std::cout << "Number of nodes in buyBook: " <<buyBook->nNode << std::endl;
        std::cout << "Number of roots in buyBook: " <<buyBook->nRoot << std::endl;
        closeBook(sellBook);
        closeBook(buyBook);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    
        printf("Time taken by reading, parsing file %s: %lld (ms).\n", argv[2], duration.count());
        printf("-----------------------\n");
    } else if (strcmp(argv[1], "QUERY") == 0) {
        if (argc < 4) error("ERROR not enough arguments\n");

        int nSymbol = atoi(argv[2]);
        if (argc - 3 != nSymbol) error("ERROR the number of symbols do not match\n");
        
        long long epochL, epochR;
        while (true) {
            std::cout << "Input -1 -1 to exit or epoch range with format x y: ";
            std::cin >> epochL >> epochR;
            if (epochL == -1) break;
            if (epochL > epochR) {
                std::cout << "Invalid range" << std::endl;
                continue;
            } 
            for (int i=0; i<nSymbol; i++) {
                std::string symbol(argv[i+3]);
                Book* buyBook = initAVL(symbol, "BUY");
                Book* sellBook = initAVL(symbol, "SELL");
                initArr(buyBook, symbol, "BUY");
                initArr(sellBook, symbol, "SELL");

                // Process the query here
                std::cout << "--------*********--------- " << "SNAPSHOT FOR " << symbol << " --------*********---------" << std::endl;
                int buyRootLower = findLowerRootByEpoch(buyBook, epochL);
                int buyRootUpper = findUpperRootByEpoch(buyBook, epochR);
                int sellRootLower = findLowerRootByEpoch(sellBook, epochL);
                int sellRootUpper = findUpperRootByEpoch(sellBook, epochR);
                if (buyRootLower == -1 || buyRootUpper==-1) {
                    std::cout << "Invalid range for side BUY" << std::endl;
                } else {
                    // std::cout << buyRootLower << " " << buyRootUpper << std::endl;
                    for (int i = buyRootLower; i<=buyRootUpper; i++) {
                        int rootIndex = accessArr(buyBook->mapped_region_arr, i)->index;
                        long long rootEpoch = accessArr(buyBook->mapped_region_arr, i)->epoch;
                        std::vector<flin> res;
                        findBid(accessNode(buyBook->mapped_region, rootIndex), buyBook->mapped_region, res);
                        std::cout << "BID " << symbol << ", " << rootEpoch << ", ";
                        for (int j=res.size()-1; j>0; j--) std::cout << res[j].second << "@" << res[j].first << ", ";
                        std::cout << res[0].second << "@" << res[0].first << std::endl;
                    }
                }
                if (sellRootLower==-1 || sellRootUpper==-1) {
                    std::cout << "Invalid range for side SALE" << std::endl;
                } else {
                    // std::cout << sellRootLower << " " << sellRootUpper << std::endl;
                }
                closeBook(sellBook);
                closeBook(buyBook);
            }
        }
    } else {
        error("ERROR selected mode not support\n");
    }
}