#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <libraycaster/map2d.hpp>
#include <libraycaster/camera.hpp>
#include <libraycaster/geometry.hpp>
#include <libraycaster/map.hpp>

// Simple mock display for testing
template<typename FP>
class MockDisplay {
public:
  explicit MockDisplay(std::size_t width = 80, std::size_t height = 24) 
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

private:
  std::size_t m_width;
  std::size_t m_height;
};

TEMPLATE_TEST_CASE("Map2D translate_and_scale", "[map2d]", float, double, long double)
{
  // Create a map2d with known parameters
  std::size_t width = 100;
  std::size_t height = 80;
  TestType scale = static_cast<TestType>(2.0);
  lefticus::raycaster::Point<TestType> center = {static_cast<TestType>(10), static_cast<TestType>(10)};
  
  lefticus::raycaster::Map2D<TestType> map2d{width, height, scale, center};
  
  // Test a few points to ensure they're properly translated and scaled
  auto p1 = lefticus::raycaster::Point<TestType>{static_cast<TestType>(10), static_cast<TestType>(10)}; // Center point
  auto translated1 = map2d.translate_and_scale(p1);
  
  // Center point should translate to middle of display
  REQUIRE(translated1.x == static_cast<TestType>(width) / 2);
  REQUIRE(translated1.y == static_cast<TestType>(height) / 2);
  
  // Test point to the right of center
  auto p2 = lefticus::raycaster::Point<TestType>{static_cast<TestType>(11), static_cast<TestType>(10)};
  auto translated2 = map2d.translate_and_scale(p2);
  
  // Should be 2 units (scale) to the right of center
  REQUIRE(translated2.x == static_cast<TestType>(width) / 2 + scale);
  REQUIRE(translated2.y == static_cast<TestType>(height) / 2);
  
  // Test point above center
  auto p3 = lefticus::raycaster::Point<TestType>{static_cast<TestType>(10), static_cast<TestType>(11)};
  auto translated3 = map2d.translate_and_scale(p3);
  
  // Should be 2 units (scale) up from center, but y is inverted
  REQUIRE(translated3.x == static_cast<TestType>(width) / 2);
  REQUIRE(translated3.y == static_cast<TestType>(height) / 2 - scale);
}

TEMPLATE_TEST_CASE("Draw map2d", "[map2d]", float, double, long double)
{
  // Set up mock display
  MockDisplay<TestType> display(200, 150);
  
  // Create camera
  lefticus::raycaster::Camera<TestType> camera{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5) },
    static_cast<TestType>(0)
  };
  
  // Create simple map with two walls
  std::vector<lefticus::raycaster::Segment<TestType>> walls = {
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(0) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(0) }
    },
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(0) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(10) }
    }
  };
  
  // Call the draw_map2d function
  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera,
    std::numbers::pi_v<TestType> / 4, // FOV
    static_cast<TestType>(1.0)        // Zoom
  );
  
  // Verify the drawing operations
  REQUIRE(display.lines_cleared);
  
  // Should have drawn at least the walls and camera position
  REQUIRE(display.draw_calls.size() > 0);
  
  // Test draw_point function
  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> point{static_cast<TestType>(50), static_cast<TestType>(50)};
  lefticus::raycaster::draw_point(display, point, {255, 0, 0}, 2);
  
  // Should have multiple draw calls for a point with radius 2
  REQUIRE(display.draw_calls.size() > 1);
  
  // Test draw_line function
  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> start{static_cast<TestType>(10), static_cast<TestType>(10)};
  lefticus::raycaster::Point<TestType> end{static_cast<TestType>(50), static_cast<TestType>(50)};
  lefticus::raycaster::draw_line(display, start, end, {0, 255, 0});
  
  // Line should create multiple draw calls
  REQUIRE(display.draw_calls.size() > 5);
}