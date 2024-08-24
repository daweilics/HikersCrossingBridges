#pragma once
#include <map>


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
        std::map<size_t, double>().swap(crossingTimeMap_);
    }

private:
    std::map<size_t, double> crossingTimeMap_;
};
