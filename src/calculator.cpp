#include "calculator.h"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <sstream>

#include "cache.h"


using std::string;
using std::vector;

double CrossingTimeCalculator::calcCrossingTime(const vector<Bridge>& bridges,
    const vector<Hiker>& origHikers, bool verbose) {
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
        // Got cached time.
        if (perFeetTime > 0) {
            totalTime += perFeetTime * bridge.getLength();
            if (verbose) {
                std::cout << "Hit cache for hiker count " << hikerCount <<
                    " at bridge with length " << bridge.getLength() << std::endl;
            }
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

double CrossingTimeCalculator::calcPerFeetTimeHikerHelpsHikers(const Hiker& hikerLead,
    const vector<Hiker>& hikers,
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

double CrossingTimeCalculator::calcThresholdSpeed(double fastest, double second) {
    return 1.0 / (2.0/second - 1.0/fastest);
}

size_t CrossingTimeCalculator::countSpeedSlowerThan(const vector<Hiker>& hikers,
    double thresholdSpeed) {
    // TODO: Can use binary search since hikers are sorted by speed.
    auto it = std::find_if(hikers.begin(), hikers.end(),
        [thresholdSpeed](const Hiker& hiker) {
            return hiker.getSpeed() < thresholdSpeed;
    });
    return std::distance(it, hikers.end());
}

// Similar to the logic of merging two sorted arrays, but we do not need
// to really remove the item to reduce memory operations.
const Hiker& CrossingTimeCalculator::removeSlowestHiker(const vector<Hiker>& hikers,
    const vector<Hiker>& additionalHikers,
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

double CrossingTimeCalculator::calcPerFeetTime(const vector<Hiker>& hikers,
    const vector<Hiker>& additionalHikers, bool verbose) {
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
    // If there are any remaining additional hikers, help them cross the
    // bridge one by one is the fastest way.
    if (additionalIndex >= 0) {
        perFeetTime += calcPerFeetTimeHikerHelpsHikers(hikerLead, additionalHikers,
            additionalIndex, 0, true, verbose);
    }
    // For the remaining hikers (if any) except the fastest and the second,
    // help them cross the bridge one by one.
    if (index > 1) {
        perFeetTime += calcPerFeetTimeHikerHelpsHikers(hikerLead, hikers,
            index, 2, true, verbose);
    }
    // For the fastest and the second, cross together, no return.
    perFeetTime += hikers[1].getPerFeetTime();
    if (verbose) {
        std::cout << hikerLeadName << "," << hikers[1].getName()
            << " cross" << std::endl;
    }
    return perFeetTime;
}
