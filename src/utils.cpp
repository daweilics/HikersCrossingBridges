#include "utils.h"

#include <algorithm>

using std::vector;


// Sort hikers by speed in descending order.
void sort_hikers(vector<Hiker>& hikers)
{
    std::sort(hikers.begin(), hikers.end(), [](const Hiker& lhs, const Hiker& rhs) {
        return lhs.getSpeed() > rhs.getSpeed();
    });
}
