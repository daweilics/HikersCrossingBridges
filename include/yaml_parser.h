#pragma once
#include <vector>


namespace YAML {
    class Node;
}

class Hiker;
class Bridge;

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
    void parse(const YAML::Node& node,
        std::vector<Hiker>& origHikers, std::vector<Bridge>& bridges);

protected:
    Hiker parseHiker(const YAML::Node& node);

    void parseHikers(const YAML::Node& node, std::vector<Hiker>& hikers);

    void parseBridge(const YAML::Node& node,
        double& length, std::vector<Hiker>& additionalHikers);
};
