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

  // Test drawing vertical and horizontal lines (special cases)
  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> h_start{static_cast<TestType>(10), static_cast<TestType>(10)};
  lefticus::raycaster::Point<TestType> h_end{static_cast<TestType>(50), static_cast<TestType>(10)};
  lefticus::raycaster::draw_line(display, h_start, h_end, {0, 0, 255});

  // Horizontal line should create draws for each x position
  REQUIRE(display.draw_calls.size() == 41); // 50 - 10 + 1

  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> v_start{static_cast<TestType>(10), static_cast<TestType>(10)};
  lefticus::raycaster::Point<TestType> v_end{static_cast<TestType>(10), static_cast<TestType>(50)};
  lefticus::raycaster::draw_line(display, v_start, v_end, {255, 255, 0});

  // Vertical line should create draws for each y position
  REQUIRE(display.draw_calls.size() == 41); // 50 - 10 + 1

  // Test drawing lines with different slopes
  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> start1{static_cast<TestType>(10), static_cast<TestType>(10)};
  lefticus::raycaster::Point<TestType> end1{static_cast<TestType>(20), static_cast<TestType>(30)};
  lefticus::raycaster::draw_line(display, start1, end1, {255, 0, 255});

  // Should have created draws for the line
  REQUIRE(!display.draw_calls.empty());

  display.draw_calls.clear();
  lefticus::raycaster::Point<TestType> start2{static_cast<TestType>(10), static_cast<TestType>(30)};
  lefticus::raycaster::Point<TestType> end2{static_cast<TestType>(30), static_cast<TestType>(10)};
  lefticus::raycaster::draw_line(display, start2, end2, {0, 255, 255});

  // Should have created draws for the line with negative slope
  REQUIRE(!display.draw_calls.empty());

  // Test draw_map2d with various parameters

  // Empty walls list
  display.draw_calls.clear();
  std::vector<lefticus::raycaster::Segment<TestType>> empty_walls;

  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(empty_walls),
    camera,
    std::numbers::pi_v<TestType> / 4,
    static_cast<TestType>(1.0)
  );

  // Should still clear the display and draw camera
  REQUIRE(display.lines_cleared);
  REQUIRE(!display.draw_calls.empty());

  // Test different zoom levels
  display.draw_calls.clear();

  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera,
    std::numbers::pi_v<TestType> / 4,
    static_cast<TestType>(0.5) // Half zoom
  );

  size_t half_zoom_draws = display.draw_calls.size();

  display.draw_calls.clear();

  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera,
    std::numbers::pi_v<TestType> / 4,
    static_cast<TestType>(2.0) // Double zoom
  );

  size_t double_zoom_draws = display.draw_calls.size();

  // Different zoom levels should produce different draw patterns
  // Not guaranteed that one will have more draws than the other due to clipping,
  // but they should be different
  REQUIRE(half_zoom_draws != double_zoom_draws);

  // Test zero zoom (edge case)
  display.draw_calls.clear();

  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera,
    std::numbers::pi_v<TestType> / 4,
    static_cast<TestType>(0.0) // Zero zoom (edge case)
  );

  // Should still clear and draw something (implementation dependent)
  REQUIRE(display.lines_cleared);

  // Test negative zoom (should behave like positive zoom with flipped orientation)
  display.draw_calls.clear();

  lefticus::raycaster::draw_map2d(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera,
    std::numbers::pi_v<TestType> / 4,
    static_cast<TestType>(-1.0) // Negative zoom
  );

  // Should still clear and draw something
  REQUIRE(display.lines_cleared);
  REQUIRE(!display.draw_calls.empty());
}