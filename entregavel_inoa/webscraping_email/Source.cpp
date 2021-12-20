#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "market.h"
#include "email.h"
#include "nlohmann/json.hpp"

int main(int argc, char* argv[])
{
    std::vector<std::string> emails, stocks;
    std::string newemail, smtp_server;
    std::vector<double> mins, maxs, currents;
    std::map <std::pair<std::string, int>, bool> already_informed;
    int check_delay = 5; //in seconds
    std::ifstream config_file = std::ifstream("smtp_config.json");
    std::ifstream addrs_file = std::ifstream("to_addrs.txt");
    sender_config sender_config_obj = sender_config(nlohmann::json::parse(config_file));
    
    if (!addrs_file) { 
        std::cerr << "File with e-mail addresses could not be open." << std::endl;
        exit(1);
    }
    if (!config_file) {
        std::cerr << "File with server settings could not be open." << std::endl;
        exit(2);

    }

    while (!addrs_file.eof()) {
        addrs_file.ignore(50, ':');
        addrs_file >> newemail;
        emails.push_back(newemail);
    }
    

    if (argc < 4 or argc % 3 !=1 ) {
        std::cerr << "Arguments were not properly sent (correct example: .\projeto_cotacao_email.exe MGLU3 8.15 8.25 PETR4 25 26)." << std::endl;
        exit(2);
    }
    else{
        for (int counter = 1; counter < argc; counter++) {
            if (counter % 3 == 1)stocks.push_back(argv[counter]); 
            else if (counter % 3 == 2) mins.push_back(stod(std::string(argv[counter])));
            else maxs.push_back(stod(std::string(argv[counter])));
        }
    }
    std::cout << std::endl;
    for (int i = 0; i < stocks.size(); i++) {
        std::cout << stocks[i] << " " << mins[i] << " " << maxs[i] << std::endl;
    }
    currents.resize(stocks.size());
    
    while (true) {    
        std::cout << "Monitoring prices of:" << std::endl;
        for (int i = 0; i < stocks.size(); i++) {
            currents[i] = get_quotation(stocks[i]);
            //currents[i] = get_quotation_website(stocks[i]);
            std::cout << stocks[i] << " : " << currents[i] << std::endl;
            if (currents[i]<mins[i] and !already_informed[{stocks[i], BUY}]) {
                std::cout << "sending email: time to buy " << stocks[i] << std::endl;
                send_alert(stocks[i], currents[i], mins[i], emails, sender_config_obj);
                already_informed[{stocks[i], BUY}] = true;
            }
            else if (currents[i] > maxs[i] and !already_informed[{stocks[i], SELL}]) {
                std::cout << "sending email: time to sell " << stocks[i] << std::endl;
                send_alert(stocks[i], currents[i], maxs[i], emails, sender_config_obj);
                already_informed[{stocks[i], SELL}] = true;
            }
            else if ((mins[i] < currents[i] and currents[i] < maxs[i])) {
                if (already_informed[{stocks[i], BUY}]) {
                    std::cout << "sending email: no longer is time to buy" << stocks[i] << std::endl;
                    inform_default_price(stocks[i], currents[i], BUY, emails, sender_config_obj);
                    already_informed[{stocks[i], BUY}] = false;
                }
                if (already_informed[{stocks[i], SELL}]) {
                    std::cout << "sending email: no longer is time to sell" << stocks[i] << std::endl;
                    inform_default_price(stocks[i], currents[i], SELL, emails, sender_config_obj);
                    already_informed[{stocks[i], SELL}] = false;
                }
            }
        }
       std::this_thread::sleep_for(std::chrono::seconds(check_delay));
    }
    config_file.close();
    addrs_file.close();
    return 0;
}