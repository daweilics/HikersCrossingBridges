#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#define ENABLE_YAML 1
#if ENABLE_YAML
#include <yaml-cpp/yaml.h>
#endif // ENABLE_YAML

using std::string;
using std::vector;
using std::map;

class Hiker {
public:
    Hiker(const string& name, double speed) : name_(name), speed_(speed) {
        assert(speed_ > 0);
        perFeetTime_ = 1 / speed_;
    }
    const string& getName() const { return name_; }
    double getSpeed() const { return speed_; }
    double getPerFeetTime() const { return perFeetTime_; }

private:
    string name_;
    double speed_;       // in feet/min
    double perFeetTime_; // Store value to save calculation cost.

};

class Bridge {
public:
    Bridge(double length, vector<Hiker> additionalHikers)
        : additionalHikers_(additionalHikers), length_(length) {
    }

    double getLength() const { return length_; }
    size_t getAdditionalHikerCount() const { return additionalHikers_.size(); }
    const vector<Hiker>& getAdditionalHikers() const { return additionalHikers_; }

private:
    vector<Hiker> additionalHikers_;
    double length_;
};

// Sort hikers by speed in descending order.
void sort_hikers(vector<Hiker>& hikers) {
    std::sort(hikers.begin(), hikers.end(), [](const Hiker& lhs, const Hiker& rhs) {
        return lhs.getSpeed() > rhs.getSpeed();
    });
}

class Cache {
public:
    double getTime(size_t hikerCount) {
        auto it = crossingTimeMap_.find(hikerCount);
        if (it != crossingTimeMap_.end()) {
            return it->second;
        }
        return -1.0;
    }
    void setTime(size_t hikerCount, double time) {
        crossingTimeMap_[hikerCount] = time;
    }
    void reset() {
        map<size_t, double>().swap(crossingTimeMap_);
    }

private:
    map<size_t, double> crossingTimeMap_;
};

class CrossingTimeCalculator {
public:
    CrossingTimeCalculator(Cache* cache) : timeCache_(cache) {
    }

    double calcCrossingTime(const vector<Bridge>& bridges, const vector<Hiker>& origHikers, bool verbose) {
        size_t origHikerCount = origHikers.size();
        // Original hikers must not be empty.
        if (origHikerCount == 0) {
            return -1.0;
        }

        double totalTime = 0.0;
        for (auto& bridge : bridges) {
            if (verbose) {
                std::cout << "Bridge (" << bridge.getLength() << ")" << std::endl;
            }
            size_t hikerCount = origHikerCount + bridge.getAdditionalHikerCount();
            double perFeetTime = 0.0;
            if (timeCache_) {
                perFeetTime = timeCache_->getTime(hikerCount);
            }
            if (perFeetTime > 0) {
                totalTime += perFeetTime * bridge.getLength();
                std::cerr << "Hit cache for hiker count " << hikerCount <<
                    " at bridge with length " << bridge.getLength() << std::endl;
                continue;
            }
            perFeetTime = calcPerFeetTime(origHikers, bridge.getAdditionalHikers(), verbose);
            totalTime += perFeetTime * bridge.getLength();
            if (timeCache_) {
                timeCache_->setTime(hikerCount, perFeetTime);
            }
        }
        return totalTime;
    }

protected:
    double calcPerFeetTimeHikerHelpsHikers(const Hiker& hikerLead, const vector<Hiker>& hikers,
        int startIndex, int targetIndex, bool targetIndexShouldReturn, bool verbose) {
        assert(targetIndex >= 0 && startIndex >= targetIndex);
        int returnCount = (startIndex+1-targetIndex) - (targetIndexShouldReturn?0:1);
        const string& hikerLeadName = hikerLead.getName();
        double hikerLeadPerFeetTime = hikerLead.getPerFeetTime();
        // The returning time.
        double perFeetTime = hikerLeadPerFeetTime * returnCount;
        while (startIndex >= targetIndex) {
            auto& hiker = hikers[startIndex--];
            // Each crossing time.
            perFeetTime += fmax(hikerLeadPerFeetTime, hiker.getPerFeetTime());
            if (verbose) {
                std::cout << hikerLeadName << "," << hiker.getName() << " cross";
                if (startIndex >= targetIndex || targetIndexShouldReturn) {
                    std::cout << ", " << hikerLeadName << " returns";
                }
                std::cout << std::endl;
            }
        }
        return perFeetTime;
    }

    double calcThresholdSpeed(double fastest, double second) {
        return 1.0 / (2.0/second - 1.0/fastest);
    }

    size_t countSpeedSlowerThan(const vector<Hiker>& hikers, double thresholdSpeed) {
        // Can use binary search since hikers are sorted by speed.
        auto it = std::find_if(hikers.begin(), hikers.end(),
            [thresholdSpeed](const Hiker& hiker) {
                return hiker.getSpeed() < thresholdSpeed;
        });
        return std::distance(it, hikers.end());
    }

    // Similar to the logic of merging two sorted arrays, but we do not need
    // to really remove the item to reduce memory operations.
    Hiker removeSlowestHiker(const vector<Hiker>& hikers, const vector<Hiker>& additionalHikers,
        int& index, int& additionalIndex) {
        if (index >= 0 && additionalIndex >= 0) {
            if (hikers[index].getSpeed() < additionalHikers[additionalIndex].getSpeed()) {
                --index;
                return hikers[index+1];
            }
            else {
                --additionalIndex;
                return additionalHikers[additionalIndex+1];
            }
        }
        else if (index >= 0) {
            --index;
            return hikers[index+1];
        }
        --additionalIndex;
        return additionalHikers[additionalIndex+1];
    }

    double calcPerFeetTime(const vector<Hiker>& hikers, const vector<Hiker>& additionalHikers, bool verbose) {
        assert(hikers.size() >= 1);
        const Hiker& hikerLead = hikers[0];
        const string& hikerLeadName = hikerLead.getName();
        if (hikers.size() == 1) {
            // We assume additional hikers cannot bring back the torch, so the
            // original hiker has to help the additional ones cross the bridge
            // one by one.
            if (!additionalHikers.empty()) {
                return calcPerFeetTimeHikerHelpsHikers(hikerLead, additionalHikers,
                    additionalHikers.size()-1, 0, false, verbose);
            }
            if (verbose) {
                std::cout << hikerLeadName << " crosses" << std::endl;
            }
            return hikerLead.getPerFeetTime();
        }

        double perFeetTime = 0.0;
        double thresholdSpeed = calcThresholdSpeed(hikerLead.getSpeed(), hikers[1].getSpeed());
        size_t slowerCount = countSpeedSlowerThan(hikers, thresholdSpeed);
        size_t additionalSlowerCount = countSpeedSlowerThan(additionalHikers, thresholdSpeed);
        size_t slowestPairCount = (slowerCount+additionalSlowerCount) >> 1;
        int index = static_cast<int>(hikers.size() - 1);
        int additionalIndex = -1;
        if (!additionalHikers.empty()) {
            additionalIndex = static_cast<int>(additionalHikers.size() - 1);
        }
        // We have at least two slowest hikers who cross together will be the
        // fastest way, process this case.
        if (slowestPairCount > 0) {
            // Time of: Fastest and second cross, Fastest returns, Second returns.
            double timeFSCrossFReturnSReturn = hikerLead.getPerFeetTime()
                + hikers[1].getPerFeetTime()*2;
            perFeetTime += timeFSCrossFReturnSReturn * slowestPairCount;
            const string& secondName = hikers[1].getName();
            for (size_t i = 0; i < slowestPairCount; ++i) {
                // Fastest and second cross, fastest returns,
                // Two slowest cross, second returns.
                Hiker slowest = removeSlowestHiker(hikers, additionalHikers,
                    index, additionalIndex);
                Hiker slowestBut1 = removeSlowestHiker(hikers, additionalHikers,
                    index, additionalIndex);
                perFeetTime += slowest.getPerFeetTime();
                if (verbose) {
                    std::cout << hikerLeadName << "," << secondName << " cross, ";
                    std::cout << hikerLeadName << " returns" << std::endl;
                    std::cout << slowest.getName() << "," << slowestBut1.getName() << " cross, ";
                    std::cout << secondName << " returns" << std::endl;
                }
            }
        }
        if (additionalIndex >= 0) {
            perFeetTime += calcPerFeetTimeHikerHelpsHikers(hikerLead, additionalHikers,
                additionalIndex, 0, true, verbose);
        }
        if (index > 1) {
            perFeetTime += calcPerFeetTimeHikerHelpsHikers(hikerLead, hikers,
                index, 2, true, verbose);
        }
        perFeetTime += hikers[1].getPerFeetTime();
        if (verbose) {
            std::cout << hikerLeadName << "," << hikers[1].getName()
                << " cross" << std::endl;
        }
        return perFeetTime;
    }

private:
    Cache* timeCache_;
};

#if ENABLE_YAML
// YAML example:
// hikers:
// - name: A
//   speed: 100
// - name: B
//   speed: 50
// bridges:
// - length: 100
// - length: 250
//   hikers:
//   - name: E
//     speed: 2.5
// - length: 150
//   hikers:
//   - name: F
//     speed: 25
//   - name: G
//     speed: 15

class YAMLCaseParser {
public:
    void parse(const YAML::Node& node, vector<Hiker>& origHikers, vector<Bridge>& bridges) {
        parseHikers(node["hikers"], origHikers);
        sort_hikers(origHikers);
        vector<Hiker> additionalHikers;
        for (auto& bridge : node["bridges"]) {
            double length = 0.0;
            size_t size = additionalHikers.size();
            parseBridge(bridge, length, additionalHikers);
            // New additional hiker(s) are added, need sort.
            if (additionalHikers.size() > size) {
                sort_hikers(additionalHikers);
            }
            bridges.emplace_back(Bridge(length, additionalHikers));
        }
    }

protected:
    Hiker parseHiker(const YAML::Node& node) {
        string name = node["name"].as<string>();
        double speed = node["speed"].as<double>();
        if (speed > 0) {
            return Hiker(name, speed);
        }
        throw std::out_of_range("Speed should > 0");
    }

    void parseHikers(const YAML::Node& node, vector<Hiker>& hikers) {
        for (auto& hiker : node) {
            try {
                hikers.emplace_back(parseHiker(hiker));
            }
            catch (const std::exception& e) {
                std::cerr << "Parse hiker error: " << e.what() << std::endl;
                throw;
            }
        }
    }

    void parseBridge(const YAML::Node& node, double& length, vector<Hiker>& additionalHikers) {
        length = node["length"].as<double>();
        if (length <= 0) {
            throw std::out_of_range("Bridge's length should > 0");
        }
        parseHikers(node["hikers"], additionalHikers);
    }
};

void hikers_to_stream(std::ostringstream& oss, const vector<Hiker>& hikers)
{
    for (auto& hiker : hikers) {
        oss << "," << hiker.getName() << " " << hiker.getSpeed();
    }
}

string to_string_case(const vector<Hiker>& origHikers, const vector<Bridge>& bridges)
{
    std::ostringstream oss;
    hikers_to_stream(oss, origHikers);
    for (auto& bridge : bridges) {
        oss << ";" << bridge.getLength();
        hikers_to_stream(oss, bridge.getAdditionalHikers());
    }
    auto strCase = oss.str();
    return strCase.substr(1);
}

void run_yaml_case(const string& filename, bool verbose=false)
{
    vector<Hiker> origHikers;
    vector<Bridge> bridges;
    try {
        YAMLCaseParser parser;
        YAML::Node config = YAML::LoadFile(filename);
        parser.parse(config, origHikers, bridges);
    }
    catch (const std::exception& e) {
        std::cerr << "Parse case error: " << e.what() << std::endl;
        return;
    }
    if (verbose) {
        std::cout << "Case (accumulated additional hikers):\n"
            << to_string_case(origHikers, bridges) << std::endl;
    }
    Cache cache;
    CrossingTimeCalculator calc(&cache);
    double totalTime = calc.calcCrossingTime(bridges, origHikers, verbose);
    std::cout << "Total crossing time is " << totalTime << " minute(s)" << std::endl;
}
#else
void run_yaml_case(const string& filename, bool verbose=false)
{
}
#endif // ENABLE_YAML

// Test code

vector<string> split(const string& str, char delimiter)
{
    vector<string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));

    return tokens;
}

double parse_double(const string& str)
{
    double value = 0.0;
    try {
        value = std::stod(str);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        throw;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        throw;
    }
    return value;
}

class CaseParser {
public:
    // String case format: <original hikers>;<bridge 1>;<bridge 2>;...;<bridge n>
    // Original hikers: name1 speed1, ..., name-n speed-n
    // Bridge: length, additionalHiker1 speed1, ..., additionalHiker-n speed-n (hikers are optional)
    // Example: "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"
    void parse(const string& strCase, vector<Hiker>& origHikers, vector<Bridge>& bridges) {
        auto items = split(strCase, ';');
        if (items.size() < 2) { // No bridge
            throw std::invalid_argument("Case format error: No bridge");
        }
        parseHikers(items[0], origHikers);
        sort_hikers(origHikers);
        vector<Hiker> additionalHikers;
        for (auto i = 1; i < items.size(); ++i) {
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

protected:
    // Hiker: name speed
    Hiker parseHiker(const string& strHiker) {
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
    void parseHikers(const string& strHikers, vector<Hiker>& hikers) {
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
    void parseBridge(const string& strBridge, double& length, vector<Hiker>& additionalHikers) {
        auto items = split(strBridge, ',');
        if (items.empty()) {
            throw std::invalid_argument("Bridge format error: No bridge length. " + strBridge);
        }
        length = parse_double(items[0]);
        if (length <= 0) {
            throw std::out_of_range("Bridge's length should > 0");
        }
        for (auto i = 1; i < items.size(); ++i) {
            additionalHikers.emplace_back(parseHiker(items[i]));
        }
    }
};

double run_case(const string& strCase, bool verbose=false)
{
    vector<Hiker> origHikers;
    vector<Bridge> bridges;
    try {
        CaseParser parser;
        parser.parse(strCase, origHikers, bridges);
    }
    catch (const std::exception& e) {
        std::cerr << "Parse case error: " << e.what() << std::endl;
        return -1.0;
    }
    if (verbose) {
        std::cout << "Case: " << strCase << std::endl;
    }
    Cache cache;
    CrossingTimeCalculator calc(&cache);
    double totalTime = calc.calcCrossingTime(bridges, origHikers, verbose);
    if (verbose) {
        std::cout << totalTime << " minute(s)" << std::endl;
    }
    return totalTime;
}

bool double_equal(double lhs, double rhs) {
    double epsilon = 1e-10; // std::numeric_limits<double>::epsilon();
    return std::fabs(lhs - rhs) < epsilon;
}

struct TestCase {
    double time;
    string strCase;
};

void run_tests()
{
    bool verbose = true;
    TestCase cases[] = {
        {245, "A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"},
        {1, "A 100;100"},
        {2, "A 100,B 50;100"},
        {17, "A 100,B 50,C 20,D 10;100"}, // C and D together is the fastest.
        {21, "A 100,B 25,C 20,D 10;100"}, // A and C, A and D is the fastest.
        {51, "A 100,B 50,C 20,D 10;100;200"}, // Cache hit.
        {55,   "A 100,B 50,C 20,D 10;100;200,E 200"},
        {55.5, "A 100,B 50,C 20,D 10;100;200,E 80"},
        {57,   "A 100,B 50,C 20,D 10;100;200,E 50"},
        {63, "A 100;100,B 50,C 20,D 10;200,E 50"},
    };
    for (auto& testCase : cases) {
        double time = run_case(testCase.strCase, verbose);
        bool pass = double_equal(time, testCase.time);
        std::cerr << (pass ? "P" : " F") << std::endl;
        if (!pass) {
            std::cerr << "Expected " << testCase.time << ", was " << time << std::endl;
        }
    }
}

int main(int argc, const char* argv[])
{
    if (argc > 1) {
        string yamlFile = argv[1];
        run_yaml_case(yamlFile, true);
    }
    else {
        run_tests();
    }
    return 0;
}