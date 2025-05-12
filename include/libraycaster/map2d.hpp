#ifndef LEFTICUS_RAYCASTER_MAP2D_HPP
#define LEFTICUS_RAYCASTER_MAP2D_HPP

#include "camera.hpp"
#include "geometry.hpp"
#include "map.hpp"
#include <span>

namespace lefticus::raycaster {

template<std::floating_point FP>
void draw_map2d(auto &display,
                std::size_t width,
                std::size_t height,
                std::span<const Segment<FP>> map_wall_segments,
                const Camera<FP> &camera,
                FP fov = std::numbers::pi_v<FP> / 4,
                FP zoom_factor = FP{1.0}) {
  // Initialize Map2D parameters
  Map2D<FP> map2d{
    width,
    height,
    (static_cast<FP>(width) / static_cast<FP>(20)) * zoom_factor, // Scale with zoom factor
    camera.location // Center the map on the camera
  };

  // Clear the display
  display.clear();

  // Draw the map walls
  for (const auto &segment : map_wall_segments) {
    const auto start = map2d.translate_and_scale(segment.start);
    const auto end = map2d.translate_and_scale(segment.end);

    // Draw line for wall segment (white)
    draw_line(display, start, end, {255, 255, 255});
  }

  // Draw the camera position (blue circle)
  const auto camera_pos = map2d.translate_and_scale(camera.location);
  draw_point(display, camera_pos, {0, 0, 255}, 2);

  // Draw the camera direction/FOV (gray lines)
  const auto start_angle = camera.start_angle(fov);
  const auto end_angle = camera.end_angle(fov);

  const auto ray_length = static_cast<FP>(2); // Length of direction indicator rays

  const auto start_ray = Ray<FP>{camera.location, start_angle}.to_segment(ray_length);
  const auto end_ray = Ray<FP>{camera.location, end_angle}.to_segment(ray_length);

  draw_line(display, 
            map2d.translate_and_scale(start_ray.start), 
            map2d.translate_and_scale(start_ray.end), 
            {128, 128, 128});
  
  draw_line(display, 
            map2d.translate_and_scale(end_ray.start), 
            map2d.translate_and_scale(end_ray.end), 
            {128, 128, 128});
}

// Helper function to draw a point/circle
template<std::floating_point FP>
void draw_point(auto &display, const Point<FP> &point, std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color, std::size_t radius = 1) {
  // Convert to integer coordinates
  const auto x = static_cast<std::size_t>(point.x);
  const auto y = static_cast<std::size_t>(point.y);
  
  // Simple circle drawing using distance formula
  for (std::size_t cy = (y > radius) ? y - radius : 0; 
       cy <= std::min(y + radius, display.height() - 1); 
       ++cy) {
    for (std::size_t cx = (x > radius) ? x - radius : 0; 
         cx <= std::min(x + radius, display.width() - 1); 
         ++cx) {
      // Check if point is within the circle
      const auto dx = static_cast<FP>(cx) - point.x;
      const auto dy = static_cast<FP>(cy) - point.y;
      if ((dx * dx + dy * dy) <= (static_cast<FP>(radius) * static_cast<FP>(radius))) {
        display.draw({cx, cy}, color);
      }
    }
  }
}

// Helper function to draw a line using Bresenham's algorithm
template<std::floating_point FP>
void draw_line(auto &display, 
               const Point<FP> &start, 
               const Point<FP> &end, 
               std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color) {
  // Convert to integer coordinates
  int x0 = static_cast<int>(start.x);
  int y0 = static_cast<int>(start.y);
  int x1 = static_cast<int>(end.x);
  int y1 = static_cast<int>(end.y);
  
  // Bresenham's line algorithm
  const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  
  const int dx = x1 - x0;
  const int dy = std::abs(y1 - y0);
  int error = dx / 2;
  const int ystep = (y0 < y1) ? 1 : -1;
  
  int y = y0;
  for (int x = x0; x <= x1; ++x) {
    const auto px = static_cast<std::size_t>(steep ? y : x);
    const auto py = static_cast<std::size_t>(steep ? x : y);
    
    // Check bounds to avoid out-of-range access
    if (px < display.width() && py < display.height()) {
      display.draw({px, py}, color);
    }
    
    error -= dy;
    if (error < 0) {
      y += ystep;
      error += dx;
    }
  }
}

} // namespace lefticus::raycaster

#endif // LEFTICUS_RAYCASTER_MAP2D_HPP