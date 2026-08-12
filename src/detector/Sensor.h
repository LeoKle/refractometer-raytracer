#ifndef DETECTOR_SENSOR_H
#define DETECTOR_SENSOR_H

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class Sensor {
public:
    Sensor(int width, int height)
        : m_width{width}
        , m_height{height}
    {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Sensor dimensions must be positive");
        }

        m_pixels.resize(
            static_cast<std::size_t>(width) * static_cast<std::size_t>(height)
        );
    }

    void addSample(int x, int y, float value) {
        Pixel& pixel = m_pixels[index(x, y)];
        pixel.sum += value;
        ++pixel.sampleCount;
    }

    [[nodiscard]] float pixelValue(int x, int y) const {
        const Pixel& pixel = m_pixels[index(x, y)];
        return pixel.sampleCount == 0
            ? 0.0f
            : pixel.sum / static_cast<float>(pixel.sampleCount);
    }

    void writeImage(const std::string& path) const {
        std::vector<float> image;
        image.reserve(m_pixels.size());

        for (const Pixel& pixel : m_pixels) {
            image.push_back(
                pixel.sampleCount == 0
                    ? 0.0f
                    : pixel.sum / static_cast<float>(pixel.sampleCount)
            );
        }

        const float maxValue = *std::max_element(image.begin(), image.end());
        if (maxValue > 0.0f) {
            for (float& value : image) {
                value /= maxValue;
            }
        }

        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Could not open sensor image: " + path);
        }

        file.write(
            reinterpret_cast<const char*>(image.data()),
            static_cast<std::streamsize>(image.size() * sizeof(float))
        );
        file.close();

        if (!file) {
            throw std::runtime_error("Could not write sensor image: " + path);
        }
    }

    [[nodiscard]] int width() const {
        return m_width;
    }

    [[nodiscard]] int height() const {
        return m_height;
    }

private:
    struct Pixel {
        float sum = 0.0f;
        std::size_t sampleCount = 0;
    };

    [[nodiscard]] std::size_t index(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            throw std::out_of_range("Sensor pixel coordinates out of range");
        }

        return static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width)
             + static_cast<std::size_t>(x);
    }

    int m_width;
    int m_height;
    std::vector<Pixel> m_pixels;
};

#endif
