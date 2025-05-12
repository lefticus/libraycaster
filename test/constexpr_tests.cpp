#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <libraycaster/geometry.hpp>
#include <libraycaster/camera.hpp>
#include <libraycaster/map.hpp>

// Tests for constexpr functions in Point
TEMPLATE_TEST_CASE("Point constexpr operations", "[constexpr][geometry]", float, double, long double)
{
  constexpr auto point1 = lefticus::raycaster::Point<TestType>{ 2, 3 };
  constexpr auto point2 = lefticus::raycaster::Point<TestType>{ 5, 7 };
  
  // Test constexpr addition
  constexpr auto sum = point1 + point2;
  STATIC_CHECK(sum.x == static_cast<TestType>(7));
  STATIC_CHECK(sum.y == static_cast<TestType>(10));
  
  // Test constexpr subtraction
  constexpr auto diff = point2 - point1;
  STATIC_CHECK(diff.x == static_cast<TestType>(3));
  STATIC_CHECK(diff.y == static_cast<TestType>(4));
}

// Tests for constexpr functions in Rectangle
TEMPLATE_TEST_CASE("Rectangle constexpr operations", "[constexpr][geometry]", float, double, long double)
{
  constexpr auto upper_left = lefticus::raycaster::Point<TestType>{ 1, 5 };
  constexpr auto lower_right = lefticus::raycaster::Point<TestType>{ 6, 1 };
  constexpr auto rectangle = lefticus::raycaster::Rectangle<TestType>{ upper_left, lower_right };
  
  // Test constexpr center calculation
  constexpr auto center = rectangle.center();
  STATIC_CHECK(center.x == static_cast<TestType>(3.5));
  STATIC_CHECK(center.y == static_cast<TestType>(3));
}

// Tests for constexpr functions in Segment
TEMPLATE_TEST_CASE("Segment constexpr operations", "[constexpr][geometry]", float, double, long double)
{
  constexpr auto segment = lefticus::raycaster::Segment<TestType>{
    lefticus::raycaster::Point<TestType>{ 2, 3 },
    lefticus::raycaster::Point<TestType>{ 5, 8 }
  };
  
  // Test constexpr min/max
  STATIC_CHECK(segment.min_x() == static_cast<TestType>(2));
  STATIC_CHECK(segment.max_x() == static_cast<TestType>(5));
  STATIC_CHECK(segment.min_y() == static_cast<TestType>(3));
  STATIC_CHECK(segment.max_y() == static_cast<TestType>(8));
  
  // Test constexpr in_bounds
  constexpr auto point_in_bounds = lefticus::raycaster::Point<TestType>{ 3, 5 };
  constexpr auto point_out_of_bounds = lefticus::raycaster::Point<TestType>{ 1, 1 };
  
  STATIC_CHECK(segment.in_bounds(point_in_bounds));
  STATIC_CHECK(!segment.in_bounds(point_out_of_bounds));
}

// Tests for constexpr functions in Camera
TEMPLATE_TEST_CASE("Camera constexpr operations", "[constexpr][geometry]", float, double, long double)
{
  constexpr auto camera = lefticus::raycaster::Camera<TestType>{
    lefticus::raycaster::Point<TestType>{ 10, 10 },
    static_cast<TestType>(0.5)
  };
  constexpr auto fov = static_cast<TestType>(1.0);
  
  // Test constexpr start_angle and end_angle
  constexpr auto start = camera.start_angle(fov);
  constexpr auto end = camera.end_angle(fov);
  
  STATIC_CHECK(start == static_cast<TestType>(0.0));
  STATIC_CHECK(end == static_cast<TestType>(1.0));
}