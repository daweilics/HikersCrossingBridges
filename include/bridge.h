#pragma once
#include <vector>
#include "hiker.h"


class Bridge {
public:
    Bridge(double length, std::vector<Hiker> additionalHikers)
        : additionalHikers_(additionalHikers), length_(length) {
    }

    double getLength() const { return length_; }
    size_t getAdditionalHikerCount() const { return additionalHikers_.size(); }
    const std::vector<Hiker>& getAdditionalHikers() const { return additionalHikers_; }

private:
    std::vector<Hiker> additionalHikers_;
    double length_;
};
