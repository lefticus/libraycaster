#ifndef LIBRAYCASTER_TEST_HELPERS_HPP
#define LIBRAYCASTER_TEST_HELPERS_HPP

#include <catch2/catch_approx.hpp>
#include <libraycaster/camera.hpp>
#include <libraycaster/geometry.hpp>
#include <libraycaster/map.hpp>
#include <vector>
#include <tuple>

// Helper function to create a camera at a standard location for testing
template<typename FP>
auto setupTestCamera(FP direction = static_cast<FP>(0.0), 
                    FP x = static_cast<FP>(0.0), 
                    FP y = static_cast<FP>(0.0)) {
    lefticus::raycaster::Camera<FP> camera;
    camera.location = lefticus::raycaster::Point<FP>{x, y};
    camera.direction = direction;
    return camera;
}

// Helper to create standard test walls
template<typename FP>
auto createBoxWalls(FP x1, FP y1, FP x2, FP y2) {
    return std::vector<lefticus::raycaster::Segment<FP>>{
        // Bottom wall
        lefticus::raycaster::Segment<FP>{
            lefticus::raycaster::Point<FP>{x1, y1},
            lefticus::raycaster::Point<FP>{x2, y1}
        },
        // Top wall
        lefticus::raycaster::Segment<FP>{
            lefticus::raycaster::Point<FP>{x1, y2},
            lefticus::raycaster::Point<FP>{x2, y2}
        },
        // Left wall
        lefticus::raycaster::Segment<FP>{
            lefticus::raycaster::Point<FP>{x1, y1},
            lefticus::raycaster::Point<FP>{x1, y2}
        },
        // Right wall
        lefticus::raycaster::Segment<FP>{
            lefticus::raycaster::Point<FP>{x2, y1},
            lefticus::raycaster::Point<FP>{x2, y2}
        }
    };
}

// Helpers for common ray verification
template<typename FP>
void verifyRayStartsAtLocation(const lefticus::raycaster::Ray<FP>& ray, 
                              const lefticus::raycaster::Point<FP>& location) {
    REQUIRE(ray.start.x == location.x);
    REQUIRE(ray.start.y == location.y);
}

template<typename FP>
void verifyRayStartsAtCamera(const lefticus::raycaster::Ray<FP>& ray, 
                            const lefticus::raycaster::Camera<FP>& camera) {
    verifyRayStartsAtLocation(ray, camera.location);
}

// Helper to verify camera location
template<typename FP>
void verifyCameraAt(const lefticus::raycaster::Camera<FP>& camera, FP x, FP y) {
    // Original code used ApproxValue for floating point comparisons
    REQUIRE(camera.location.x == Catch::Approx(static_cast<double>(x)));
    REQUIRE(camera.location.y == Catch::Approx(static_cast<double>(y)));
}

// Basic display class for testing that can be used in both map2d and renderer tests
template<typename FP>
class MockDisplayBase {
public:
    explicit MockDisplayBase(std::size_t width = 80, std::size_t height = 24)
        : m_width(width), m_height(height) {}

    void clear() {
        // Clear display
        lines_cleared = true;
    }

    void draw(std::pair<std::size_t, std::size_t> pos,
            std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color) {
        // Track draw calls
        draw_calls.push_back({pos.first, pos.second, color});
    }

    std::size_t width() const { return m_width; }
    std::size_t height() const { return m_height; }

    // For verification
    bool lines_cleared = false;
    std::vector<std::tuple<std::size_t, std::size_t,
                std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>>> draw_calls;

protected:
    std::size_t m_width;
    std::size_t m_height;
};

// Specialized display for renderer testing with vertical line drawing
template<typename FP>
class RendererMockDisplay : public MockDisplayBase<FP> {
public:
    using MockDisplayBase<FP>::MockDisplayBase;
    using MockDisplayBase<FP>::draw;
    using MockDisplayBase<FP>::clear;
    using MockDisplayBase<FP>::width;
    using MockDisplayBase<FP>::height;
    using MockDisplayBase<FP>::lines_cleared;
    using MockDisplayBase<FP>::draw_calls;

    void draw_vertical_line(
        std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color,
        std::size_t x,
        std::size_t start_y,
        std::size_t end_y) {
        // Track vertical line draw calls
        vertical_lines.push_back({color, x, start_y, end_y});
    }

    // For verification
    std::vector<std::tuple<
                std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>,
                std::size_t, std::size_t, std::size_t>> vertical_lines;
};

// A simpler version for map2d testing
template<typename FP>
using Map2DMockDisplay = MockDisplayBase<FP>;

#endif // LIBRAYCASTER_TEST_HELPERS_HPP