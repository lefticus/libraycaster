#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <libraycaster/renderer.hpp>
#include <libraycaster/camera.hpp>
#include <libraycaster/geometry.hpp>
#include <libraycaster/map.hpp>

// Simple mock display for renderer testing
template<typename FP>
class RendererMockDisplay {
public:
  explicit RendererMockDisplay(std::size_t width = 80, std::size_t height = 24) 
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
  
  void draw_vertical_line(
    std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color,
    std::size_t x, 
    std::size_t start_y, 
    std::size_t end_y) {
    // Track vertical line draw calls
    vertical_lines.push_back({color, x, start_y, end_y});
  }

  std::size_t width() const { return m_width; }
  std::size_t height() const { return m_height; }

  // For verification
  bool lines_cleared = false;
  std::vector<std::tuple<std::size_t, std::size_t, 
               std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>>> draw_calls;
  std::vector<std::tuple<
               std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>,
               std::size_t, std::size_t, std::size_t>> vertical_lines;

private:
  std::size_t m_width;
  std::size_t m_height;
};

TEMPLATE_TEST_CASE("Renderer basic rendering", "[renderer]", float, double, long double)
{
  // Setup mock display
  RendererMockDisplay<TestType> display(320, 240);
  
  // Create camera looking down the y-axis
  lefticus::raycaster::Camera<TestType> camera{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5) },
    static_cast<TestType>(0) // Looking down the y-axis
  };
  
  // Create simple map with walls around the camera
  std::vector<lefticus::raycaster::Segment<TestType>> walls = {
    // Wall in front of camera
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(10) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(10) }
    },
    // Wall behind camera
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(0) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(0) }
    },
    // Wall to the left
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(0) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(0), static_cast<TestType>(10) }
    },
    // Wall to the right
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(0) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(10) }
    },
  };
  
  // Set different colors for each wall
  walls[0].color = {255, 0, 0};   // Red for front wall
  walls[1].color = {0, 255, 0};   // Green for back wall
  walls[2].color = {0, 0, 255};   // Blue for left wall
  walls[3].color = {255, 255, 0}; // Yellow for right wall
  
  // Call the render function
  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera
  );
  
  // Verify the rendering operations
  REQUIRE(display.lines_cleared);
  
  // We should have drawn vertical lines for the visible walls
  REQUIRE(display.vertical_lines.size() > 0);
  
  // Test with camera looking in different directions
  display.vertical_lines.clear();
  
  // Camera looking to the right (90° rotation)
  lefticus::raycaster::Camera<TestType> camera2{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5) },
    std::numbers::pi_v<TestType> / 2 // Looking right
  };
  
  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera2
  );
  
  REQUIRE(display.vertical_lines.size() > 0);
  
  // Test with camera looking in yet another direction
  display.vertical_lines.clear();
  
  // Camera looking at an angle
  lefticus::raycaster::Camera<TestType> camera3{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5) },
    std::numbers::pi_v<TestType> / 4 // 45° angle
  };
  
  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(walls),
    camera3
  );
  
  REQUIRE(display.vertical_lines.size() > 0);
}

TEMPLATE_TEST_CASE("Renderer edge case rendering", "[renderer]", float, double, long double)
{
  // Setup mock display
  RendererMockDisplay<TestType> display(320, 240);

  // Create test cases for various edge conditions

  // Test case 1: No walls (empty map)
  std::vector<lefticus::raycaster::Segment<TestType>> empty_walls;

  lefticus::raycaster::Camera<TestType> camera{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5) },
    static_cast<TestType>(0)
  };

  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(empty_walls),
    camera
  );

  // Should clear the display but not draw any vertical lines
  REQUIRE(display.lines_cleared);
  REQUIRE(display.vertical_lines.empty());

  // Test case 2: Very close wall (tests wall_height calculation edge case)
  std::vector<lefticus::raycaster::Segment<TestType>> close_wall = {
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(5.1) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(6), static_cast<TestType>(5.1) }
    }
  };

  display.vertical_lines.clear();
  display.lines_cleared = false;

  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(close_wall),
    camera
  );

  // Should have drawn some vertical lines for the very close wall
  REQUIRE(display.lines_cleared);
  REQUIRE(display.vertical_lines.size() > 0);

  // Test case 3: Adjacent wall segments (tests edge handling)
  std::vector<lefticus::raycaster::Segment<TestType>> adjacent_walls = {
    // First wall
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(10) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(10) },
      {255, 0, 0} // Red
    },
    // Adjacent wall (shares an endpoint)
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(10) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(10), static_cast<TestType>(5) },
      {0, 255, 0} // Green
    }
  };

  display.vertical_lines.clear();
  display.lines_cleared = false;

  // Place camera to see both walls
  lefticus::raycaster::Camera<TestType> camera2{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(7), static_cast<TestType>(7) },
    std::numbers::pi_v<TestType> / 4 // 45° angle
  };

  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(adjacent_walls),
    camera2
  );

  // Should have drawn vertical lines for both walls
  REQUIRE(display.lines_cleared);
  REQUIRE(display.vertical_lines.size() > 0);

  // Test case 4: Walls at different distances (tests distance fog effect)
  std::vector<lefticus::raycaster::Segment<TestType>> distance_walls = {
    // Near wall
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(7) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(7), static_cast<TestType>(7) },
      {255, 255, 255} // White
    },
    // Far wall
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(5), static_cast<TestType>(15) },
      lefticus::raycaster::Point<TestType>{ static_cast<TestType>(7), static_cast<TestType>(15) },
      {255, 255, 255} // White
    }
  };

  display.vertical_lines.clear();
  display.lines_cleared = false;

  lefticus::raycaster::Camera<TestType> camera3{
    lefticus::raycaster::Point<TestType>{ static_cast<TestType>(6), static_cast<TestType>(5) },
    static_cast<TestType>(0) // Looking north
  };

  lefticus::raycaster::render(
    display,
    display.width(),
    display.height(),
    std::span<const lefticus::raycaster::Segment<TestType>>(distance_walls),
    camera3
  );

  // Should have drawn vertical lines for both walls
  REQUIRE(display.lines_cleared);
  REQUIRE(display.vertical_lines.size() > 0);
}