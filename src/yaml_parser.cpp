#include "yaml_parser.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

#include "hiker.h"
#include "bridge.h"
#include "utils.h"


using std::string;
using std::vector;

void YAMLCaseParser::parse(const YAML::Node& node,
    vector<Hiker>& origHikers, vector<Bridge>& bridges) {
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

Hiker YAMLCaseParser::parseHiker(const YAML::Node& node) {
    string name = node["name"].as<string>();
    double speed = node["speed"].as<double>();
    if (speed > 0) {
        return Hiker(name, speed);
    }
    throw std::out_of_range("Speed should > 0");
}

void YAMLCaseParser::parseHikers(const YAML::Node& node, vector<Hiker>& hikers) {
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

void YAMLCaseParser::parseBridge(const YAML::Node& node,
    double& length, vector<Hiker>& additionalHikers) {
    length = node["length"].as<double>();
    if (length <= 0) {
        throw std::out_of_range("Bridge's length should > 0");
    }
    parseHikers(node["hikers"], additionalHikers);
}
