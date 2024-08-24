#pragma once
#include <vector>
#include "hiker.h"
#include "bridge.h"


class Cache;

class CrossingTimeCalculator {
public:
    CrossingTimeCalculator(Cache* cache) : timeCache_(cache) {
    }

    double calcCrossingTime(const std::vector<Bridge>& bridges,
        const std::vector<Hiker>& origHikers, bool verbose);

protected:
    double calcPerFeetTimeHikerHelpsHikers(const Hiker& hikerLead,
        const std::vector<Hiker>& hikers,
        int startIndex, int targetIndex, bool targetIndexShouldReturn, bool verbose);

    double calcThresholdSpeed(double fastest, double second);

    size_t countSpeedSlowerThan(const std::vector<Hiker>& hikers,
        double thresholdSpeed);

    // Similar to the logic of merging two sorted arrays, but we do not need
    // to really remove the item to reduce memory operations.
    Hiker removeSlowestHiker(const std::vector<Hiker>& hikers,
        const std::vector<Hiker>& additionalHikers,
        int& index, int& additionalIndex);

    double calcPerFeetTime(const std::vector<Hiker>& hikers,
        const std::vector<Hiker>& additionalHikers, bool verbose);

private:
    Cache* timeCache_;
};
