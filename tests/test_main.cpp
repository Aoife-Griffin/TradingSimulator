#include <gtest/gtest.h>
#include "Engine.hpp"

TEST(EngineCoreTest, VerifiesBasicMath) {
    Trading::Engine engine;
    EXPECT_EQ(engine.add(1, 1), 2);
}
