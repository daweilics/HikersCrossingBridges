#include "gtest/gtest.h"

#include "hiker.h"

// Test Hiker class
TEST(HikerTest, ConstructorAndGetters) {
    Hiker hiker("A", 100);
    EXPECT_EQ(hiker.getName(), "A");
    EXPECT_DOUBLE_EQ(hiker.getSpeed(), 100);
    EXPECT_DOUBLE_EQ(hiker.getPerFeetTime(), 1.0 / 100);
}
