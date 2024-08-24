#pragma once
#include <string>
#include <vector>


class Hiker;
class Bridge;

class CaseParser {
public:
    // String case format: <original hikers>;<bridge 1>;<bridge 2>;...;<bridge n>
    // Original hikers: name1 speed1, ..., name-n speed-n
    // Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
    // Example: "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"
    void parse(const std::string& strCase,
        std::vector<Hiker>& origHikers, std::vector<Bridge>& bridges);

protected:
    // Hiker: name speed
    Hiker parseHiker(const std::string& strHiker);

    // Hikers: name1 speed1, ..., name-n speed-n
    void parseHikers(const std::string& strHikers, std::vector<Hiker>& hikers);

    // Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
    void parseBridge(const std::string& strBridge,
        double& length, std::vector<Hiker>& additionalHikers);
};
