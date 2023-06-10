#include "camera.hpp"
#include "geometry.hpp"
#include <ranges>
#include <span>

namespace lefticus::geometry {
template<typename FP>
void render(auto &display,
  std::size_t width,
  std::size_t height,
  std::span<const Segment<FP>> map_wall_segments,
  Camera<FP> camera)
{
  display.clear();
  constexpr static bool fisheye_distance_correction = true;

  std::size_t col = 0;

  std::optional<Segment<FP>> last_match;
  std::optional<std::pair<std::size_t, std::size_t>> last_wall;

  for (const auto &[r, segment_point] : camera.rays(width)) {
    auto matches = intersect_ray(r, map_wall_segments);
    std::sort(
      matches.begin(), matches.end(), [](const auto &lhs, const auto &rhs) { return lhs.distance < rhs.distance; });

    if (!matches.empty() && matches[0].distance != 0) {

      const auto distance_from_eye = matches[0].distance;

      // Distance correction from https://gamedev.stackexchange.com/questions/45295/raycasting-fisheye-effect-question
      const auto corrected_distance = [&] {
        if (fisheye_distance_correction) {
          return distance_from_eye * std::cos(camera.direction - r.angle);
        } else {
          return distance_from_eye;
        }
      }();

      auto wall_height = static_cast<std::size_t>((static_cast<FP>(height) * 0.75) / corrected_distance);

      if (wall_height > height) { wall_height = height; }

      const auto wall_start = (height - wall_height) / 2;
      const auto wall_end = wall_start + wall_height;

      // Draw edge if detected
      if (col != 0 && (!last_match || last_match != matches[0].segment)) {
        if (!last_match) {
          display.draw_vertical_line({ 255, 255, 255 }, col, wall_start, wall_end);
        } else {
          display.draw_vertical_line(
            { 255, 255, 255 }, col, std::min(wall_start, last_wall->first), std::max(wall_end, last_wall->second));
        }
      } else {
        // draw just top and bottom points otherwise
        display.draw({ col, wall_start }, { 255, 255, 255 });
        display.draw({ col, wall_end }, { 255, 255, 255 });

        // and some texture...
        const auto texture_size = 2;

        if (col % texture_size == 0) {
          for (std::size_t y = wall_start; y < wall_end; y += texture_size) {
            display.draw({ col, y }, { 255, 255, 255 });
          }
        }
      }

      last_wall.emplace(wall_start, wall_end);
      last_match = matches[0].segment;
    } else {
      // Look for transition from wall to empty space, draw edge
      if (last_match) { display.draw_vertical_line({ 255, 255, 255 }, col, last_wall->first, last_wall->second); }
      last_match.reset();
    }

    ++col;
  }

  /*
if minimap_on:
  map_surface = pygame.Surface((map2d.width, map2d.height))
  map2d.center = camera.location
  map2d.draw_map(map_surface, map_wall_segments)
  map2d.draw_camera(map_surface, camera)
  pygame.display.get_surface().blit(
      map_surface, (width - map2d.width, height - map2d.height)
  )
*/
}
}// namespace lefticus::geometry
