#pragma once
#include <cassert>
#include <string>


class Hiker {
public:
    Hiker(const std::string& name, double speed) : name_(name), speed_(speed) {
        assert(speed_ > 0);
        perFeetTime_ = 1 / speed_;
    }
    const std::string& getName() const { return name_; }
    double getSpeed() const { return speed_; }
    double getPerFeetTime() const { return perFeetTime_; }

private:
    std::string name_;
    double speed_;       // in feet/min
    double perFeetTime_; // Store value to save calculation cost.
};
