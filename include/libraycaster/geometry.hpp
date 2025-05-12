#ifndef LEFTICUS_RAYCASTER_GEOMETRY_HPP
#define LEFTICUS_RAYCASTER_GEOMETRY_HPP

#include <cmath>
#include <concepts>
#include <numbers>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace lefticus::raycaster {

// RGB color as a tuple of three uint8_t values
using Color = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>;

// Average two colors together - useful for color blending
[[nodiscard]] constexpr Color average_colors(const Color &c1, const Color &c2) {
  return Color{
    static_cast<std::uint8_t>((std::get<0>(c1) + std::get<0>(c2)) / 2),
    static_cast<std::uint8_t>((std::get<1>(c1) + std::get<1>(c2)) / 2),
    static_cast<std::uint8_t>((std::get<2>(c1) + std::get<2>(c2)) / 2)
  };
}
template<std::floating_point FP> inline constexpr auto DISTANT_POINT_v = static_cast<FP>(1000);

// Floating point math is hard, and trying to find a point on a line
// can result in some mismatches in floating point values, so we go for "close"
template<std::floating_point FP> constexpr bool in_range(FP min_, FP max_, FP value)
{
  return ((min_ - static_cast<FP>(0.0000001)) <= value) and (value <= (max_ + static_cast<FP>(0.0000001)));
}

template<std::floating_point FP> struct Ray;

template<std::floating_point FP> struct Point
{
  FP x;
  FP y;

  [[nodiscard]] constexpr Point operator+(const Point &other) const noexcept
  {
    return Point{ x + other.x, y + other.y };
  }
  [[nodiscard]] constexpr Point operator-(const Point &other) const noexcept
  {
    return Point{ x - other.x, y - other.y };
  }

  [[nodiscard]] constexpr auto operator<=>(const Point &) const noexcept = default;
};

template<std::floating_point FP> struct Rectangle
{
  Point<FP> upper_left;
  Point<FP> lower_right;

  [[nodiscard]] constexpr bool intersects(const Point<FP> &point) const noexcept
  {
    return point.x > upper_left.x && point.y > upper_left.y && point.x < lower_right.x && point.y < lower_right.y;
  }

  [[nodiscard]] constexpr Point<FP> center() const noexcept
  {
    const auto summed_point = upper_left + lower_right;
    return Point<FP>{ summed_point.x / 2, summed_point.y / 2 };
  }
};

template<std::floating_point FP> struct Segment
{
  Point<FP> start;
  Point<FP> end;
  Color color{255, 255, 255}; // Default to white

  [[nodiscard]] constexpr auto operator<=>(const Segment &) const noexcept = default;

  [[nodiscard]] constexpr bool parallel(Segment other) const noexcept
  {
    // Todo - de-duplicate this code
    const auto x1 = start.x;
    const auto y1 = start.y;

    const auto x2 = end.x;
    const auto y2 = end.y;

    const auto x3 = other.start.x;
    const auto y3 = other.start.y;

    const auto x4 = other.end.x;
    const auto y4 = other.end.y;

    // Calculate the denominator of the t and u values in the parametric equations of the two segments
    return ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1)) == 0;
  }

  [[nodiscard]] constexpr std::optional<Point<FP>> intersection(Segment<FP> other) const noexcept
  {
    if (not(min_x() <= other.max_x() and max_x() >= other.min_x() and min_y() <= other.max_y()
            and max_y() >= other.min_y())) {
      return {};
    }

    // This version is cribbed from ChatGPT, and it passes our tests for
    // line intersection calculations

    // Calculate the differences between the start and end points of the two segments

    const auto x1 = start.x;
    const auto y1 = start.y;

    const auto x2 = end.x;
    const auto y2 = end.y;

    const auto x3 = other.start.x;
    const auto y3 = other.start.y;

    const auto x4 = other.end.x;
    const auto y4 = other.end.y;

    // Calculate the denominator of the t and u values in the parametric equations of the two segments
    const auto denominator = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

    // Check if the two segments are parallel (i.e., their parametric equations don't intersect)
    if (denominator == 0) { return {}; }

    // Calculate the t and u values in the parametric equations of the two segments
    const auto t = ((x3 - x1) * (y4 - y3) - (y3 - y1) * (x4 - x3)) / denominator;
    const auto u = ((x1 - x2) * (y3 - y1) - (y1 - y2) * (x3 - x1)) / denominator;

    // Check if the two segments intersect
    if ((0 <= t) and (t <= 1) and (0 <= u) and (u <= 1)) {
      // Calculate the point of intersection
      const auto x = x1 + t * (x2 - x1);
      const auto y = y1 + t * (y2 - y1);
      return Point<FP>{ x, y };
    } else {
      return {};
    }
  }

  [[nodiscard]] constexpr auto min_x() const noexcept { return std::min(start.x, end.x); }
  [[nodiscard]] constexpr auto max_x() const noexcept { return std::max(start.x, end.x); }
  [[nodiscard]] constexpr auto min_y() const noexcept { return std::min(start.y, end.y); }
  [[nodiscard]] constexpr auto max_y() const noexcept { return std::max(start.y, end.y); }

  [[nodiscard]] constexpr auto in_bounds(Point<FP> p) const noexcept
  {
    return in_range(min_x(), max_x(), p.x) and in_range(min_y(), max_y(), p.y);
  }

  [[nodiscard]] auto to_ray() const
  {
    if (start == end) {
      // no possible valid Ray object
      throw std::runtime_error("Cannot create Ray from identical segment points");
    }

    // Correct from angle above x axis as returned by atan2, to angle
    // away from y axis, as is in our coordinate system
    const auto new_angle = -std::atan2(end.y - start.y, end.x - start.x) + std::numbers::pi_v<FP> / 2;
    const auto normalized_angle = std::fmod(new_angle, std::numbers::pi_v<FP> * 2);
    const auto non_negative_angle = [=] {
      if (normalized_angle < 0) {
        return normalized_angle + std::numbers::pi_v<FP> * 2;
      } else {
        return new_angle;
      }
    }();


    return Ray<FP>{ start, non_negative_angle };
  }
};

template<std::floating_point FP> struct Ray
{
  Point<FP> start;
  FP angle;// Angle from the y-axis, right. "compass coordinates"

  [[nodiscard]] constexpr auto operator<=>(const Ray &) const noexcept = default;

  [[nodiscard]] auto end_point(FP distance) const noexcept
  {
    return Point<FP>{ start.x + (std::sin(angle) * distance), start.y + (std::cos(angle) * distance) };
  }

  // Not constexpr because end_point() uses non-constexpr trig functions (until C++26)
  [[nodiscard]] auto to_segment(FP distance = DISTANT_POINT_v<FP>) const noexcept
  {
    return Segment<FP>{ start, end_point(distance) };
  }
};


template<std::floating_point FP>
// Not constexpr because it calls ray.to_segment() which uses non-constexpr trig functions (until C++26)
[[nodiscard]] auto intersect_ray(Ray<FP> ray, std::span<const Segment<FP>> segments)
{
  return intersecting_segments(ray.to_segment(), segments);
}

template<std::floating_point FP> struct IntersectionResult
{
  FP distance;
  Point<FP> intersection;
  Segment<FP> segment;
};

template<std::floating_point FP>
// Not constexpr because it uses std::hypot which may not be constexpr until C++26,
// and std::vector operations which might not be fully constexpr-ready
[[nodiscard]] auto intersecting_segments(Segment<FP> input_, std::span<const Segment<FP>> segments)
{
  std::vector<IntersectionResult<FP>> result;

  for (const auto &segment : segments) {
    const auto intersection = input_.intersection(segment);

    if (intersection) {
      result.push_back(IntersectionResult<FP>{
        std::hypot(input_.start.x - intersection->x, input_.start.y - intersection->y), *intersection, segment });
    }
  }

  return result;
}
}// namespace lefticus::raycaster


#endif
