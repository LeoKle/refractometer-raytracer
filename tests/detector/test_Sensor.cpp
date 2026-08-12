#include <gtest/gtest.h>

#include <stdexcept>

#include "detector/Sensor.h"

TEST(SensorTest, AveragesSamples) {
    Sensor sensor(1, 1);

    sensor.addSample(0, 0, 2.0f);
    sensor.addSample(0, 0, 4.0f);

    EXPECT_FLOAT_EQ(sensor.pixelValue(0, 0), 3.0f);
}

TEST(SensorTest, KeepsPixelsIndependent) {
    Sensor sensor(2, 1);

    sensor.addSample(1, 0, 5.0f);

    EXPECT_FLOAT_EQ(sensor.pixelValue(0, 0), 0.0f);
    EXPECT_FLOAT_EQ(sensor.pixelValue(1, 0), 5.0f);
}

TEST(SensorTest, RejectsInvalidDimensions) {
    EXPECT_THROW(Sensor(0, 1), std::invalid_argument);
    EXPECT_THROW(Sensor(1, 0), std::invalid_argument);
}
