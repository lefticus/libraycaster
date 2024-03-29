#include "camera.hpp"
#include "geometry.hpp"
#include <ranges>
#include <span>

namespace lefticus::raycaster {
template<typename FP>
void render(auto &display,
  std::size_t width,
  std::size_t height,
  std::span<const Segment<FP>> map_wall_segments,
  Camera<FP> camera)
{
  const auto FOV = 2 * std::atan(static_cast<double>(width) / static_cast<double>(height * 2))
                   * std::tan((std::numbers::pi_v<double> / 2) / 2);

  display.clear();

  std::size_t col = 0;

  std::optional<Segment<FP>> last_match;
  std::optional<std::pair<std::size_t, std::size_t>> last_wall;
  std::uint8_t color_adjustment = 0;

  for (const auto &[r, segment_point] : camera.rays(width, FOV)) {
    auto matches = intersect_ray(r, map_wall_segments);
    std::sort(
      matches.begin(), matches.end(), [](const auto &lhs, const auto &rhs) { return lhs.distance < rhs.distance; });

    if (!matches.empty() && matches[0].distance != 0) {

      const auto distance_from_eye = matches[0].distance;

      // Distance correction from https://gamedev.stackexchange.com/questions/45295/raycasting-fisheye-effect-question
      const auto corrected_distance = distance_from_eye * std::cos(camera.direction - r.angle);

      color_adjustment = static_cast<std::uint8_t>(
        static_cast<FP>(128) * std::min(distance_from_eye, static_cast<FP>(5)) / static_cast<FP>(5));

      auto wall_height = static_cast<std::size_t>((static_cast<FP>(height) * 0.75) / corrected_distance);

      if (wall_height > height) { wall_height = height; }

      const auto wall_start = (height - wall_height) / 2;
      const auto wall_end = wall_start + wall_height;

      // Draw edge if detected
      if (col != 0 && (!last_match || last_match != matches[0].segment)) {
        if (!last_match) {
          display.draw_vertical_line(
            { 255 - color_adjustment, 255 - color_adjustment, 255 - color_adjustment }, col, wall_start, wall_end);
        } else {
          display.draw_vertical_line({ 255 - color_adjustment, 255 - color_adjustment, 255 - color_adjustment },
            col,
            std::min(wall_start, last_wall->first),
            std::max(wall_end, last_wall->second));
        }
      } else {
        // and some color
        display.draw_vertical_line({ 128 - color_adjustment, 128 - color_adjustment, 255 - color_adjustment },
          col,
          wall_start,
          wall_end);

        if (wall_height != height) {
          // draw top and bottom points
          display.draw({ col, wall_start }, { 255 - color_adjustment, 255 - color_adjustment, 255 - color_adjustment });
          display.draw({ col, wall_end }, { 255 - color_adjustment, 255 - color_adjustment, 255 - color_adjustment });
        }

      }

      last_wall.emplace(wall_start, wall_end);
      last_match = matches[0].segment;
    } else {
      // Look for transition from wall to empty space, draw edge
      if (last_match) {
        display.draw_vertical_line({ 255 - color_adjustment, 255 - color_adjustment, 255 - color_adjustment },
          col,
          last_wall->first,
          last_wall->second);
      }
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
}// namespace lefticus::raycaster
