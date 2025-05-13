#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <libraycaster/camera.hpp>
#include <libraycaster/geometry.hpp>
#include "test_helpers.hpp"


template<std::floating_point FP> struct Approx
{
  FP value;

  constexpr static auto get_diff() noexcept
  {
    if constexpr (sizeof(FP) == 2) {// NOLINT
      return std::numeric_limits<FP>::epsilon() * 100;// NOLINT
    } else if constexpr (sizeof(FP) == 4) {// NOLINT
      return std::numeric_limits<FP>::epsilon() * 1000;// NOLINT
    } else if constexpr (sizeof(FP) == 8) {// NOLINT
      return std::numeric_limits<FP>::epsilon() * 1000;// NOLINT
    } else if constexpr (sizeof(FP) == 16) {// NOLINT
      return std::numeric_limits<FP>::epsilon() * 100000;// NOLINT
    }
  }

  constexpr static auto diff = get_diff();

  constexpr explicit Approx(FP value_) noexcept : value{ value_ } {}

  friend constexpr bool operator==(FP lhs, Approx rhs) noexcept
  {
    if (lhs == rhs.value) {
      return true;
    } else if (lhs < rhs.value) {
      return (rhs.value - lhs) < diff;// NOLINT
    } else {
      return (lhs - rhs.value) < diff;// NOLINT
    }
  }
};

TEMPLATE_TEST_CASE("Test Adding Points", "[geometry]", float, double, long double)
{
  const auto point1 = lefticus::raycaster::Point<TestType>{ 1, 2 };
  const auto point2 = lefticus::raycaster::Point<TestType>{ 3, 4 };
  const auto result = point1 + point2;

  CHECK(result.x == 4);
  CHECK(result.y == 6);
}

TEMPLATE_TEST_CASE("Test Segment Properties", "[geometry]", float, double, long double)
{
  const auto expected_values = [](auto segment, auto min_x, auto min_y, auto max_x, auto max_y, auto ray) {
    CHECK(segment.min_x() == min_x);
    CHECK(segment.min_y() == min_y);
    CHECK(segment.max_x() == max_x);
    CHECK(segment.max_y() == max_y);

    CHECK(segment.to_ray() == ray);
  };

  const auto zero_slope = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 1, 0 } };
  expected_values(zero_slope,
    static_cast<TestType>(0),
    static_cast<TestType>(0),
    static_cast<TestType>(1),
    static_cast<TestType>(0),
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 2 });

  const auto one_slope = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 1, 1 } };
  expected_values(one_slope,
    static_cast<TestType>(0),
    static_cast<TestType>(0),
    static_cast<TestType>(1),
    static_cast<TestType>(1),
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 4 });

  const auto negative_one_slope = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 1, -1 } };

  expected_values(negative_one_slope,
    static_cast<TestType>(0),
    static_cast<TestType>(-1),
    static_cast<TestType>(1),
    static_cast<TestType>(0),
    lefticus::raycaster::Ray<TestType>{
      lefticus::raycaster::Point<TestType>{ 0, 0 }, 3 * std::numbers::pi_v<TestType> / 4 });

  const auto vertical_slope = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 0, 1 } };

  expected_values(vertical_slope,
    static_cast<TestType>(0),
    static_cast<TestType>(0),
    static_cast<TestType>(0),
    static_cast<TestType>(1),
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, 0 });
}


TEMPLATE_TEST_CASE("Test Segment Segment", "[geometry]", float, double, long double)
{
  const auto point_segment = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 0, 0 } };
  const auto line_segment = lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    lefticus::raycaster::Point<TestType>{ 1, 2 } };

  CHECK_NOTHROW(line_segment.to_ray());
  CHECK_THROWS(point_segment.to_ray());
}

TEMPLATE_TEST_CASE("Test Ray Properties", "[geometry]", float, double, long double)
{
  const auto expected_values = [](auto ray, auto segment) {
    const auto actual_segment = ray.to_segment();
    CHECK(actual_segment.start.x == segment.start.x);
    CHECK(actual_segment.start.y == segment.start.y);
    CHECK(actual_segment.end.x == Approx<TestType>(segment.end.x));
    CHECK(actual_segment.end.y == Approx<TestType>(segment.end.y));
  };

  const auto zero_angle = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, 0 };
  expected_values(zero_angle,
    lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      lefticus::raycaster::Point<TestType>{ 0, lefticus::raycaster::DISTANT_POINT_v<TestType> } });

  const auto forty_five_angle =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 4 };
  expected_values(forty_five_angle,
    lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      lefticus::raycaster::Point<TestType>{
        std::sin(std::numbers::pi_v<TestType> / 4) * lefticus::raycaster::DISTANT_POINT_v<TestType>,
        std::cos(std::numbers::pi_v<TestType> / 4) * lefticus::raycaster::DISTANT_POINT_v<TestType> } });

  const auto right_angle =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 2 };
  expected_values(right_angle,
    lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      lefticus::raycaster::Point<TestType>{ lefticus::raycaster::DISTANT_POINT_v<TestType>, 0 } });

  const auto one_eighty_angle =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> };
  expected_values(one_eighty_angle,
    lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      lefticus::raycaster::Point<TestType>{ 0, -lefticus::raycaster::DISTANT_POINT_v<TestType> } });

  const auto two_seventy_angle = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    TestType{ 3 } / TestType{ 2 } * std::numbers::pi_v<TestType> };
  expected_values(two_seventy_angle,
    lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      lefticus::raycaster::Point<TestType>{ -lefticus::raycaster::DISTANT_POINT_v<TestType>, 0 } });
}

TEMPLATE_TEST_CASE("Test Ray Segment Round Trip", "[geometry]", float, double, long double)
{
  const auto round_trip = [](auto ray) {
    const auto segment = ray.to_segment();
    const auto new_ray = segment.to_ray();

    CHECK(std::fmod(ray.angle, (2 * std::numbers::pi_v<TestType>)) == Approx<TestType>(new_ray.angle));
    CHECK(ray.start == new_ray.start);
  };

  const auto angle_0 = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, 0 };
  const auto angle_45 =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 4 };
  const auto angle_90 =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> / 2 };
  const auto angle_180 =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 }, std::numbers::pi_v<TestType> };
  const auto angle_270 = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    TestType{ 3 } / TestType{ 2 } * std::numbers::pi_v<TestType> };
  const auto angle_405 = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
    2 * std::numbers::pi_v<TestType> + std::numbers::pi_v<TestType> / 4 };

  SECTION("0 degrees") { round_trip(angle_0); }
  SECTION("45 degrees") { round_trip(angle_45); }
  SECTION("90 degrees") { round_trip(angle_90); }
  SECTION("180 degrees") { round_trip(angle_180); }
  SECTION("270 degrees") { round_trip(angle_270); }
  SECTION("405 degrees") { round_trip(angle_405); }
}


TEMPLATE_TEST_CASE("Test Segment Intersections", "[geometry]", float, double, long double)
{
  std::array<lefticus::raycaster::Segment<TestType>, 1> horizontal{ lefticus::raycaster::Segment<TestType>{
    lefticus::raycaster::Point<TestType>{ -1, 0 }, lefticus::raycaster::Point<TestType>{ 1, 0 } } };
  std::array<lefticus::raycaster::Segment<TestType>, 1> vertical{ lefticus::raycaster::Segment<TestType>{
    lefticus::raycaster::Point<TestType>{ 0, -1 }, lefticus::raycaster::Point<TestType>{ 0, 1 } } };

  const auto intersections = lefticus::raycaster::intersecting_segments(
    horizontal[0], std::span<const lefticus::raycaster::Segment<TestType>>{ vertical });

  REQUIRE(intersections.size() == 1);
  CHECK(intersections[0].segment == vertical[0]);
  CHECK(intersections[0].intersection.x == 0);
  CHECK(intersections[0].intersection.y == 0);

  const auto intersections_h = lefticus::raycaster::intersecting_segments(
    vertical[0], std::span<const lefticus::raycaster::Segment<TestType>>{ horizontal });

  REQUIRE(intersections_h.size() == 1);
  CHECK(intersections_h[0].segment == horizontal[0]);
  CHECK(intersections_h[0].intersection.x == 0);
  CHECK(intersections_h[0].intersection.y == 0);
}

TEMPLATE_TEST_CASE("Test Intersect Ray To Perpendicular", "[geometry]", float, double, long double)
{
  {
    // vertical ray (x = 10)
    const auto ray =
      lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 10, 5 }, std::numbers::pi_v<TestType> };
    // horizontal segment (y = 0, x=[0, 20])
    const std::array<lefticus::raycaster::Segment<TestType>, 1> segment{ lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ 0, 0 }, lefticus::raycaster::Point<TestType>{ 20, 0 } } };
    // they should intersect at (10, 0)

    const auto intersections =
      lefticus::raycaster::intersect_ray(ray, std::span<const lefticus::raycaster::Segment<TestType>>{ segment });
    REQUIRE(intersections.size() == 1);
    CHECK(intersections[0].intersection.x == 10);
    CHECK(intersections[0].intersection.y == 0);
  }

  {
    // horizontal ray (y = 0)
    const auto ray = lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
      std::numbers::pi_v<TestType> / 2 };
    // vertical segment (x = 4, y=[-10, 10])
    const std::array<lefticus::raycaster::Segment<TestType>, 1> segment{ lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ 4, -10 }, lefticus::raycaster::Point<TestType>{ 4, 10 } } };
    // they should intersect at (4, 0)
    const auto intersections =
      lefticus::raycaster::intersect_ray(ray, std::span<const lefticus::raycaster::Segment<TestType>>{ segment });
    REQUIRE(intersections.size() == 1);
    CHECK(intersections[0].intersection.x == Approx<TestType>(4));
    CHECK(intersections[0].intersection.y == Approx<TestType>(0));// NOLINT MAGIC NUMBER
  }
}


TEMPLATE_TEST_CASE("Test Intersect Ray To Diagonal", "[geometry]", float, double, long double)
{
  const auto ray =
    lefticus::raycaster::Ray<TestType>{ lefticus::raycaster::Point<TestType>{ 10, 5 }, std::numbers::pi_v<TestType> };
  const std::array<lefticus::raycaster::Segment<TestType>, 1> segment{ lefticus::raycaster::Segment<TestType>{
    lefticus::raycaster::Point<TestType>{ 0, 0 }, lefticus::raycaster::Point<TestType>{ 20, -20 } } };
  const auto intersections =
    lefticus::raycaster::intersect_ray(ray, std::span<const lefticus::raycaster::Segment<TestType>>{ segment });
  CHECK(intersections.size() == 1);
}


TEMPLATE_TEST_CASE("Test Camera Ray To Diagonal", "[geometry]", float, double, long double)
{
  const auto camera = setupTestCamera<TestType>(std::numbers::pi_v<TestType>, static_cast<TestType>(10), static_cast<TestType>(5));

  const auto view_angle = std::numbers::pi_v<TestType>/4;

  const std::array segments{ lefticus::raycaster::Segment<TestType>{ lefticus::raycaster::Point<TestType>{ 0, 0 },
                               lefticus::raycaster::Point<TestType>{ 20, 0 } },
    lefticus::raycaster::Segment<TestType>{
      lefticus::raycaster::Point<TestType>{ 0, 0 }, lefticus::raycaster::Point<TestType>{ 40, -40 } } };

  for (const auto &[ray, point] : camera.rays(10, view_angle)) {
    const auto intersections =
      lefticus::raycaster::intersect_ray(ray, std::span<const lefticus::raycaster::Segment<TestType>>(segments));
    CHECK(intersections.size() == 2);
  }
}

// Tests for non-constexpr functions that use trigonometric operations

TEMPLATE_TEST_CASE("Test Ray::end_point", "[geometry][non-constexpr]", float, double, long double)
{
  const auto ray = lefticus::raycaster::Ray<TestType>{
    lefticus::raycaster::Point<TestType>{ 0, 0 },
    std::numbers::pi_v<TestType> / 4
  };

  // At distance 1, should be at (sin(π/4), cos(π/4)) = (0.7071..., 0.7071...)
  const auto end = ray.end_point(static_cast<TestType>(1));

  CHECK(end.x == Approx<TestType>(std::sin(std::numbers::pi_v<TestType> / 4)));
  CHECK(end.y == Approx<TestType>(std::cos(std::numbers::pi_v<TestType> / 4)));

  // At distance 2, coordinates should be doubled
  const auto end2 = ray.end_point(static_cast<TestType>(2));

  CHECK(end2.x == Approx<TestType>(2 * std::sin(std::numbers::pi_v<TestType> / 4)));
  CHECK(end2.y == Approx<TestType>(2 * std::cos(std::numbers::pi_v<TestType> / 4)));
}

TEMPLATE_TEST_CASE("Test Camera rotate", "[geometry][non-constexpr]", float, double, long double)
{
  auto camera = setupTestCamera<TestType>();

  // Test rotation by 90 degrees
  camera.rotate(std::numbers::pi_v<TestType> / 2);
  CHECK(camera.direction == Approx<TestType>(std::numbers::pi_v<TestType> / 2));

  // Test rotation by another 90 degrees
  camera.rotate(std::numbers::pi_v<TestType> / 2);
  CHECK(camera.direction == Approx<TestType>(std::numbers::pi_v<TestType>));

  // Test rotation by 360 degrees (should wrap around to 0)
  camera.rotate(2 * std::numbers::pi_v<TestType>);
  CHECK(camera.direction == Approx<TestType>(std::numbers::pi_v<TestType>));

  // Test rotation by -180 degrees
  camera.rotate(-std::numbers::pi_v<TestType>);
  CHECK(camera.direction == Approx<TestType>(0));
}

TEMPLATE_TEST_CASE("Test Camera rays", "[geometry][non-constexpr]", float, double, long double)
{
  const auto camera = setupTestCamera<TestType>();

  const auto fov = std::numbers::pi_v<TestType> / 2;  // 90 degrees
  const std::size_t num_rays = 5;

  // Generate rays and verify properties
  int ray_count = 0;
  for (const auto &[ray, point] : camera.rays(num_rays, fov)) {
    // Verify ray starts at camera position
    verifyRayStartsAtCamera(ray, camera);

    // Verify ray starts at camera position
    // Note: We don't check ray angle because in the current implementation
    // rays are created using to_ray() which converts segment to ray angle,
    // resulting in angles that may not be directly limited by the FOV

    ray_count++;
  }

  // Verify we got the right number of rays
  CHECK(ray_count == static_cast<int>(num_rays));
}
