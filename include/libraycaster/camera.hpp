#ifndef LEFTICUS_RAYCASTER_CAMERA_HPP
#define LEFTICUS_RAYCASTER_CAMERA_HPP

#include "geometry.hpp"
#include <ranges>

namespace lefticus::geometry {
template<std::floating_point FP> struct Camera
{
  lefticus::geometry::Point<FP> location{};
  FP direction = 0;// angle from y-axis, "compass" style

  void try_move(FP distance, std::span<const lefticus::geometry::Segment<FP>> walls)
  {
    const auto new_location =
      location + lefticus::geometry::Point<FP>{ distance * std::sin(direction), distance * std::cos(direction) };

    const auto proposed_move = lefticus::geometry::Segment<FP>{ location, new_location };

    if (intersecting_segments(proposed_move, walls).empty()) {
      // we don't intersect any wall, so we allow the move
      location = new_location;
    }
  }

  void rotate(FP angle) { direction = std::fmod(direction + angle, 2 * std::numbers::pi_v<FP>); }

  [[nodiscard]] constexpr auto start_angle(FP fov) const noexcept { return direction - fov / 2; }
  [[nodiscard]] constexpr auto end_angle(FP fov) const noexcept { return start_angle(fov) + fov; }

  [[nodiscard]] auto rays(std::size_t count, FP fov) const
  {
    // The idea is that we are creating a line
    // through which to draw the rays, so we get a more correct
    // (not curved) distribution of rays, but we still need
    // to do a height correction later to flatten it out
    const auto viewing_plane_start =
      location + lefticus::geometry::Point<FP>{ std::sin(start_angle(fov)), std::cos(start_angle(fov)) };
    const auto viewing_plane_end =
      location + lefticus::geometry::Point<FP>{ std::sin(end_angle(fov)), std::cos(end_angle(fov)) };

    const auto d_x = (viewing_plane_end.x - viewing_plane_start.x) / static_cast<FP>(count);
    const auto d_y = (viewing_plane_end.y - viewing_plane_start.y) / static_cast<FP>(count);

    return std::ranges::views::iota(std::size_t{ 0 }, count)
           | std::ranges::views::transform([=, location = this->location](auto current) {
               const auto plane_point = lefticus::geometry::Point<FP>{
                 viewing_plane_start.x + (d_x * static_cast<FP>(current)),
                 viewing_plane_start.y + (d_y * static_cast<FP>(current)),
               };
               const auto ray_segment = lefticus::geometry::Segment<FP>{ location, plane_point };
               return std::pair{ ray_segment.to_ray(), plane_point };
             });
  }
};
}// namespace lefticus::geometry
#endif
