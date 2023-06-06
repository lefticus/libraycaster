#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <libraycaster/geometry.hpp>

using FP = double;

TEST_CASE("Test Adding Points", "[geometry]")
{
  const auto point1 = lefticus::geometry::Point<FP>{ 1, 2 };
  const auto point2 = lefticus::geometry::Point<FP>{ 3, 4 };
  const auto result = point1 + point2;

  CHECK(result.x == 4);
  CHECK(result.y == 6);
}

TEST_CASE("Test Segment Properties", "[geometry]")
{
  const auto expected_values = [](auto segment, auto min_x, auto min_y, auto max_x, auto max_y, auto ray) {
    CHECK(segment.min_x() == min_x);
    CHECK(segment.min_y() == min_y);
    CHECK(segment.max_x() == max_x);
    CHECK(segment.max_y() == max_y);

    CHECK(segment.to_ray() == ray);
  };

  const auto zero_slope =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 1, 0 } };
  expected_values(zero_slope,
    0,
    0,
    1,
    0,
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 2 });

  const auto one_slope =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 1, 1 } };
  expected_values(one_slope,
    0,
    0,
    1,
    1,
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 4 });

  const auto negative_one_slope =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 1, -1 } };

  expected_values(negative_one_slope,
    0,
    -1,
    1,
    0,
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, 3 * std::numbers::pi_v<FP> / 4 });

  const auto vertical_slope =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 0, 1 } };

  expected_values(vertical_slope, 0, 0, 0, 1, lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, 0 });
}


TEST_CASE("Test Segment Segment", "[geometry]")
{
  const auto point_segment =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 0, 0 } };
  const auto line_segment =
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 1, 2 } };

  CHECK_NOTHROW(line_segment.to_ray());
  CHECK_THROWS(point_segment.to_ray());
}

TEST_CASE("Test Ray Properties", "[geometry]")
{
  const auto expected_values = [](auto ray, auto segment) {
    const auto actual_segment = ray.to_segment();
    CHECK(actual_segment.start.x == segment.start.x);
    CHECK(actual_segment.start.y == segment.start.y);
    CHECK_THAT(actual_segment.end.x, Catch::Matchers::WithinAbs(segment.end.x, .000000000001));// NOLINT MAGIC NUMBER
    CHECK_THAT(actual_segment.end.y, Catch::Matchers::WithinAbs(segment.end.y, .000000000001));// NOLINT MAGIC NUMBER
  };

  const auto zero_angle = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, 0 };
  expected_values(zero_angle,
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 },
      lefticus::geometry::Point<FP>{ 0, lefticus::geometry::DISTANT_POINT_v<FP> } });

  const auto forty_five_angle =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 4 };
  expected_values(forty_five_angle,
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 },
      lefticus::geometry::Point<FP>{ std::sin(std::numbers::pi_v<FP> / 4) * lefticus::geometry::DISTANT_POINT_v<FP>,
        std::cos(std::numbers::pi_v<FP> / 4) * lefticus::geometry::DISTANT_POINT_v<FP> } });

  const auto right_angle =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 2 };
  expected_values(right_angle,
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 },
      lefticus::geometry::Point<FP>{ lefticus::geometry::DISTANT_POINT_v<FP>, 0 } });

  const auto one_eighty_angle =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> };
  expected_values(one_eighty_angle,
    lefticus::geometry::Segment<FP>{ lefticus::geometry::Point<FP>{ 0, 0 },
      lefticus::geometry::Point<FP>{ 0, -lefticus::geometry::DISTANT_POINT_v<FP> } });
}

TEST_CASE("Test Ray Segment Round Trip", "[geometry]")
{
  const auto round_trip = [](auto ray) {
    const auto segment = ray.to_segment();
    const auto new_ray = segment.to_ray();

    CHECK_THAT(std::fmod(ray.angle, (2 * std::numbers::pi_v<FP>)),
      Catch::Matchers::WithinAbs(new_ray.angle, .000000000001));// NOLINT MAGIC NUMBER
    CHECK(ray.start == new_ray.start);
  };

  const auto angle_0 = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, 0 };
  const auto angle_45 =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 4 };
  const auto angle_90 =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 2 };
  const auto angle_180 = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> };
  const auto angle_270 =
    lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, 3 * std::numbers::pi_v<FP> / 2 };
  const auto angle_405 = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 },
    2 * std::numbers::pi_v<FP> + std::numbers::pi_v<FP> / 4 };

  round_trip(angle_0);
  round_trip(angle_45);
  round_trip(angle_90);
  round_trip(angle_180);
  round_trip(angle_270);
  round_trip(angle_405);
}


TEST_CASE("Test Segment Intersections", "[geometry]")
{
  std::array<lefticus::geometry::Segment<FP>, 1> horizontal{ lefticus::geometry::Segment<FP>{
    lefticus::geometry::Point<FP>{ -1, 0 }, lefticus::geometry::Point<FP>{ 1, 0 } } };
  std::array<lefticus::geometry::Segment<FP>, 1> vertical{ lefticus::geometry::Segment<FP>{
    lefticus::geometry::Point<FP>{ 0, -1 }, lefticus::geometry::Point<FP>{ 0, 1 } } };

  const auto intersections = lefticus::geometry::intersecting_segments(
    horizontal[0], std::span<const lefticus::geometry::Segment<FP>>{ vertical });

  REQUIRE(intersections.size() == 1);
  CHECK(intersections[0].segment == vertical[0]);
  CHECK(intersections[0].intersection.x == 0);
  CHECK(intersections[0].intersection.y == 0);

  const auto intersections_h = lefticus::geometry::intersecting_segments(
    vertical[0], std::span<const lefticus::geometry::Segment<FP>>{ horizontal });

  REQUIRE(intersections_h.size() == 1);
  CHECK(intersections_h[0].segment == horizontal[0]);
  CHECK(intersections_h[0].intersection.x == 0);
  CHECK(intersections_h[0].intersection.y == 0);
}

TEST_CASE("Test Intersect Ray To Perpendicular", "[geometry]")
{
  {
    // vertical ray (x = 10)
    const auto ray = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 10, 5 }, std::numbers::pi_v<FP> };
    // horizontal segment (y = 0, x=[0, 20])
    const std::array<lefticus::geometry::Segment<FP>, 1> segment{ lefticus::geometry::Segment<FP>{
      lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 20, 0 } } };
    // they should intersect at (10, 0)

    const auto intersections =
      lefticus::geometry::intersect_ray(ray, std::span<const lefticus::geometry::Segment<FP>>{ segment });
    REQUIRE(intersections.size() == 1);
    CHECK(intersections[0].intersection.x == 10);
    CHECK(intersections[0].intersection.y == 0);
  }

  {
    // horizontal ray (y = 0)
    const auto ray = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 0, 0 }, std::numbers::pi_v<FP> / 2 };
    // vertical segment (x = 4, y=[-10, 10])
    const std::array<lefticus::geometry::Segment<FP>, 1> segment{ lefticus::geometry::Segment<FP>{
      lefticus::geometry::Point<FP>{ 4, -10 }, lefticus::geometry::Point<FP>{ 4, 10 } } };
    // they should intersect at (4, 0)
    const auto intersections =
      lefticus::geometry::intersect_ray(ray, std::span<const lefticus::geometry::Segment<FP>>{ segment });
    REQUIRE(intersections.size() == 1);
    CHECK(intersections[0].intersection.x == 4);
    CHECK_THAT(intersections[0].intersection.y, Catch::Matchers::WithinAbs(0, .000000000001));// NOLINT MAGIC NUMBER
  }
}


TEST_CASE("Test Intersect Ray To Diagonal", "[geometry]")
{
  const auto ray = lefticus::geometry::Ray<FP>{ lefticus::geometry::Point<FP>{ 10, 5 }, std::numbers::pi_v<FP> };
  const std::array<lefticus::geometry::Segment<FP>, 1> segment{ lefticus::geometry::Segment<FP>{
    lefticus::geometry::Point<FP>{ 0, 0 }, lefticus::geometry::Point<FP>{ 20, -20 } } };
  const auto intersections =
    lefticus::geometry::intersect_ray(ray, std::span<const lefticus::geometry::Segment<FP>>{ segment });
  CHECK(intersections.size() == 1);
}


/*
TEST_CASE("Test Camera Ray To Diagonal", "[geometry]")
{
  const auto camera =
    raycasting.Camera(lefticus::geometry::Point<FP>(10, 5), std::numbers::pi_v<FP>, std::numbers::pi_v<FP> / 4);
  const auto segment =
    lefticus::geometry::Segment(lefticus::geometry::Point<FP>(0, 0), lefticus::geometry::Point<FP>(20, 0));
  const auto segment2 =
    lefticus::geometry::Segment(lefticus::geometry::Point<FP>(0, 0), lefticus::geometry::Point<FP>(40, -40));

  for (const auto &[ray, point] : camera.rays(10)) {
    const auto intersections = lefticus::geometry::intersect_ray(ray, [ segment, segment2 ]);
    CHECK(len(intersections) == 2);
  }
}
*/
