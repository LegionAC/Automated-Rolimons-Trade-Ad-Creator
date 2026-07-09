#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <cctype>

using json = nlohmann::json;
using namespace std::this_thread;
using namespace std::chrono;    

httplib::Client cli("https://api.rolimons.com");

httplib::Headers headers({
    {"Cookie", ""}
});

auto sendTrade(json j, int timer) {
    auto res = cli.Post("/tradeads/v1/createad", headers, j.dump(), "application/json");

    return res;
}

json csvParse(std::string str, bool id) {
    json values = json::array();
    std::string buffer;

    for (char v : str) {
        if (v != ',') {
            buffer.push_back(std::tolower(v) );
        } else if (v == ' ') {
            continue;
        } else if (!buffer.empty()) {
            if (id && buffer != "0") {
                values.push_back(std::stoi(buffer));
            } else if (buffer != "0") {
                values.push_back(buffer);
            }
            buffer = ""; 
        }
    }

    if (!buffer.empty() && buffer != "0") {
        if (id) {
            values.push_back(std::stoi(buffer));
        } else {
            values.push_back(buffer);
        }
    }

    return values;
}

json userQuery() {
    json j;

    std::string cookie;

    std::string id;
    std::string offer;
    std::string request;
    std::string tags;

    std::cout << "Enter your Rolimons player id: ";
    std::cin >> id;
    std::cout << "\nFor the following entries, separate your values with commas (csv) and input 0 to represent no value.\n";
    std::cout << "\nEnter your offer ids: ";
    std::cin >> offer;
    std::cout << "\nEnter your request ids: ";
    std::cin >> request;
    std::cout << "\nEnter your request tags: ";
    std::cin >> tags;
    std::cout << "\nEnter your Rolimons RoliVerification cookie: ";
    std::cin >> cookie;
    std::cout << "\n";

    headers.find("Cookie")->second = "_RoliVerification=" + cookie;

    j["player_id"] = std::stoi(id);

    j["offer_item_ids"] = csvParse(offer, true);

    j["request_item_ids"] = csvParse(request, true);

    j["request_tags"] = csvParse(tags, false);

    return j;
}

int startLoop() {
    json j = userQuery();

    std::string timer;

    std::cout << "Please select time duration (seconds): ";
    std::cin >> timer;
    std::cout << "\n";

    while (true) {
        auto res = sendTrade(j, stoi(timer));

        if (res->status == 201) {
            std::cout << "Trade ad sent successfully. Waiting " << timer << " seconds.\n";
        } else if (res->status == 14) {
            std::cout << "Trade ad limit reached... Waiting " << timer << " seconds.\n";
        } else {
            std::cout << "Trade ad error: " << res->status << "\nError code: " << res->body << "\n";
            
            if (res->status == 400 || res->status == 401) {
                startLoop();
            }
        }

        sleep_for(seconds(stoi(timer)));
    }
}

int main() {
    startLoop();
}
