#include "string_parser.h"

#include <string>
#include <vector>
#include <iostream>

#include "hiker.h"
#include "bridge.h"
#include "utils.h"


using std::string;
using std::vector;

// String case format: <original hikers>;<bridge 1>;<bridge 2>;...;<bridge n>
// Original hikers: name1 speed1, ..., name-n speed-n
// Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
// Example: "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"
void CaseParser::parse(const string& strCase,
    vector<Hiker>& origHikers, vector<Bridge>& bridges) {
    auto items = split(strCase, ';');
    if (items.size() < 2) { // No bridge
        throw std::invalid_argument("Case format error: No bridge");
    }
    parseHikers(items[0], origHikers);
    sort_hikers(origHikers);
    vector<Hiker> additionalHikers;
    for (size_t i = 1; i < items.size(); ++i) {
        double length = 0.0;
        size_t size = additionalHikers.size();
        parseBridge(items[i], length, additionalHikers);
        // New additional hiker(s) are added, need sort.
        if (additionalHikers.size() > size) {
            sort_hikers(additionalHikers);
        }
        bridges.emplace_back(Bridge(length, additionalHikers));
    }
}

// Hiker: name speed
Hiker CaseParser::parseHiker(const string& strHiker) {
    auto hiker = split(strHiker, ' ');
    if (hiker.size() < 2) {
        throw std::invalid_argument("Hiker format error: " + strHiker);
    }
    auto speed = parse_double(hiker[1]);
    if (speed > 0) {
        return Hiker(hiker[0], speed);
    }
    throw std::out_of_range("Speed should > 0");
}

// Hikers: name1 speed1, ..., name-n speed-n
void CaseParser::parseHikers(const string& strHikers, vector<Hiker>& hikers) {
    for (auto& strHiker : split(strHikers, ',')) {
        try {
            hikers.emplace_back(parseHiker(strHiker));
        }
        catch (const std::exception& e) {
            std::cerr << "Parse hiker error: " << e.what() << std::endl;
            throw;
        }
    }
}

// Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
void CaseParser::parseBridge(const string& strBridge,
    double& length, vector<Hiker>& additionalHikers) {
    auto items = split(strBridge, ',');
    if (items.empty()) {
        throw std::invalid_argument("Bridge format error: No bridge length. " + strBridge);
    }
    length = parse_double(items[0]);
    if (length <= 0) {
        throw std::out_of_range("Bridge's length should > 0");
    }
    for (size_t i = 1; i < items.size(); ++i) {
        additionalHikers.emplace_back(parseHiker(items[i]));
    }
}
