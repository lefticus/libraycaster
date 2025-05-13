#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <libraycaster/camera.hpp>
#include <libraycaster/geometry.hpp>
#include "test_helpers.hpp"

using Catch::Approx;

// Helper function to create appropriate Approx for any floating point type
template<typename T>
auto ApproxValue(T value) {
    return Approx(static_cast<double>(value));
}

TEMPLATE_TEST_CASE("Camera movement with collisions", "[camera]", float, double, long double)
{
  // Create a camera
  auto camera = setupTestCamera<TestType>(static_cast<TestType>(0.0), static_cast<TestType>(5.0), static_cast<TestType>(5.0)); // facing forward/north

  // Create some walls
  std::vector<lefticus::raycaster::Segment<TestType>> walls = {
    // Wall directly in front of the camera
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(4.0), static_cast<TestType>(7.0)},
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(6.0), static_cast<TestType>(7.0)}
    },
    // Wall to the right
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(7.0), static_cast<TestType>(4.0)},
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(7.0), static_cast<TestType>(6.0)}
    }
  };

  // Initial position check
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(5.0));

  // Try to move forward - should be blocked by wall
  camera.try_move(static_cast<TestType>(3.0), walls);
  // Position should not change
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(5.0));

  // Rotate to face right (east)
  camera.rotate(std::numbers::pi_v<TestType> / 2);
  REQUIRE(camera.direction == ApproxValue(std::numbers::pi_v<TestType> / 2));

  // Try to move right - should be blocked
  camera.try_move(static_cast<TestType>(3.0), walls);
  // Position should not change
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(5.0));

  // Rotate to face backward (south)
  camera.rotate(std::numbers::pi_v<TestType> / 2);
  REQUIRE(camera.direction == ApproxValue(std::numbers::pi_v<TestType>));

  // Try to move backward - should succeed
  camera.try_move(static_cast<TestType>(2.0), walls);
  // Position should change - moved south
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(3.0));

  // Test rotate with negative angle
  camera.rotate(static_cast<TestType>(-std::numbers::pi_v<TestType>));
  REQUIRE(camera.direction == ApproxValue(0.0));

  // Test rotate with angle > 2π
  camera.rotate(static_cast<TestType>(3.0) * std::numbers::pi_v<TestType>);
  REQUIRE(camera.direction == ApproxValue(std::numbers::pi_v<TestType>));

  // Test movement with negative distance (should move backward)
  camera.try_move(static_cast<TestType>(-2.0), walls);
  // Should have moved north (opposite of current direction)
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(5.0));

  // Test movement with zero distance (should not move)
  camera.try_move(static_cast<TestType>(0.0), walls);
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(5.0));

  // Reset camera direction to north (0 radians)
  camera.direction = static_cast<TestType>(0.0);

  // Test with empty walls vector (guaranteed to not intersect)
  std::vector<lefticus::raycaster::Segment<TestType>> empty_walls;
  camera.try_move(static_cast<TestType>(1.0), empty_walls);
  // Should move without collision
  verifyCameraAt(camera, static_cast<TestType>(5.0), static_cast<TestType>(6.0));

  // Test with a wall that exactly intersects with the move path
  std::vector<lefticus::raycaster::Segment<TestType>> exactly_intersecting_walls = {
    // Horizontal wall right in front of the camera's movement path
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(4.0), static_cast<TestType>(7.0)},
      lefticus::raycaster::Point<TestType>{static_cast<TestType>(6.0), static_cast<TestType>(7.0)}
    }
  };

  // Save current location
  auto old_x = camera.location.x;
  auto old_y = camera.location.y;

  // Try to move forward - should be blocked
  camera.try_move(static_cast<TestType>(2.0), exactly_intersecting_walls);

  // Position should not change
  verifyCameraAt(camera, old_x, old_y);
}

TEMPLATE_TEST_CASE("Camera rays generation", "[camera]", float, double, long double)
{
  auto camera = setupTestCamera<TestType>(); // facing north
  
  // Test FOV and ray count
  const auto fov = std::numbers::pi_v<TestType> / 2; // 90 degrees
  const std::size_t ray_count = 10;
  
  // Get rays
  auto ray_range = camera.rays(ray_count, fov);
  
  // Count rays and verify properties
  std::size_t count = 0;
  
  for (const auto &[ray, point] : ray_range) {
    // Each ray should start at camera location
    verifyRayStartsAtCamera(ray, camera);
    count++;
  }
  
  // Verify correct number of rays
  REQUIRE(count == ray_count);
  
  // Test with different camera direction
  camera.direction = std::numbers::pi_v<TestType> / 4; // 45 degrees
  
  auto ray_range2 = camera.rays(ray_count, fov);
  std::size_t count2 = 0;
  
  for (const auto &[ray, point] : ray_range2) {
    // Each ray should still start at camera location
    verifyRayStartsAtCamera(ray, camera);
    count2++;
  }
  
  REQUIRE(count2 == ray_count);
  
  // Test with 0 ray count
  auto ray_range3 = camera.rays(0, fov);
  std::size_t count3 = 0;

  // Use a variable to silence unused warning
  for (auto _unused [[maybe_unused]] : ray_range3) {
    count3++;
  }

  REQUIRE(count3 == 0);

  // Test with 1 ray count
  auto ray_range4 = camera.rays(1, fov);
  std::size_t count4 = 0;

  for (const auto &[ray, point] : ray_range4) {
    // Should be exactly in the middle of the FOV
    verifyRayStartsAtCamera(ray, camera);
    count4++;
  }

  REQUIRE(count4 == 1);

  // Test with negative FOV (should behave the same as positive FOV)
  auto ray_range5 = camera.rays(ray_count, static_cast<TestType>(-fov));
  std::size_t count5 = 0;

  for (auto _unused5 [[maybe_unused]] : ray_range5) {
    count5++;
  }

  REQUIRE(count5 == ray_count);

  // Test with zero FOV
  auto ray_range6 = camera.rays(ray_count, static_cast<TestType>(0.0));
  std::size_t count6 = 0;

  for (auto _unused6 [[maybe_unused]] : ray_range6) {
    count6++;
  }

  REQUIRE(count6 == ray_count);

  // Test start_angle and end_angle functions
  REQUIRE(camera.start_angle(fov) == ApproxValue(camera.direction - fov / 2));
  REQUIRE(camera.end_angle(fov) == ApproxValue(camera.direction + fov / 2));
}

TEMPLATE_TEST_CASE("Camera rays special cases", "[camera]", float, double, long double)
{
  auto camera = setupTestCamera<TestType>();

  // Test zero count rays (corner case)
  auto zero_count_rays = camera.rays(0, static_cast<TestType>(1.0));
  std::size_t count = 0;
  for (auto _ [[maybe_unused]] : zero_count_rays) {
    count++;
  }
  REQUIRE(count == 0);

  // Test single ray (corner case)
  auto single_ray = camera.rays(1, static_cast<TestType>(1.0));
  count = 0;
  for (auto _ [[maybe_unused]] : single_ray) {
    count++;
  }
  REQUIRE(count == 1);

  // Test rays with different FOVs
  for (int i = 0; i < 10; i++) {
    auto fov = static_cast<TestType>(i) * static_cast<TestType>(0.1);
    auto rays = camera.rays(5, fov);
    count = 0;
    for (auto _ [[maybe_unused]] : rays) {
      count++;
    }
    REQUIRE(count == 5);
  }

  // Test with extreme FOV values
  auto extreme_fov = static_cast<TestType>(10.0) * std::numbers::pi_v<TestType>;
  auto extreme_rays = camera.rays(5, extreme_fov);
  count = 0;
  for (auto _ [[maybe_unused]] : extreme_rays) {
    count++;
  }
  REQUIRE(count == 5);

  // Test with different camera positions
  for (int x = -5; x <= 5; x += 2) {
    for (int y = -5; y <= 5; y += 2) {
      camera.location = lefticus::raycaster::Point<TestType>{
        static_cast<TestType>(x),
        static_cast<TestType>(y)
      };
      auto position_rays = camera.rays(3, static_cast<TestType>(1.0));
      count = 0;

      for (const auto &[ray, _] : position_rays) {
        // Verify rays start at the correct position
        verifyRayStartsAtCamera(ray, camera);
        count++;
      }

      REQUIRE(count == 3);
    }
  }
}