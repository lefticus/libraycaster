#ifndef LEFTICUS_RAYCASTER_MAP_HPP
#define LEFTICUS_RAYCASTER_MAP_HPP

#include "camera.hpp"
#include "geometry.hpp"

#include <algorithm>
#include <array>

namespace lefticus::raycaster {

// Wall type definition for mapping characters to wall shapes and colors
template<std::floating_point FP> struct WallType
{
  Color color{255, 255, 255}; // Default color (white)
  std::vector<Segment<FP>> (*shape_generator)(Point<FP>) = nullptr; // Function pointer to shape generator
};

template<std::floating_point FP> struct Named_Location
{
  Rectangle<FP> location;
  char name;
};

// Forward declarations
template<std::floating_point FP> struct Map;
template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> box(Point<FP> ul);
template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ul_triangle(Point<FP> ul);
template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ur_triangle(Point<FP> ul);
template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> lr_triangle(Point<FP> ul);
template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ll_triangle(Point<FP> ul);

// Initialize default wall types for backward compatibility
template<std::floating_point FP> constexpr void initialize_default_wall_types(Map<FP> &map)
{
  // Box wall - white
  map.wall_types['#'].color = {255, 255, 255};
  map.wall_types['#'].shape_generator = box<FP>;
  map.wall_types['*'].color = {255, 255, 255};
  map.wall_types['*'].shape_generator = box<FP>;

  // Upper-left triangle - light red
  map.wall_types['/'].color = {255, 200, 200};
  map.wall_types['/'].shape_generator = ul_triangle<FP>;

  // Upper-right triangle - light green
  map.wall_types['&'].color = {200, 255, 200};
  map.wall_types['&'].shape_generator = ur_triangle<FP>;

  // Lower-right triangle - light blue
  map.wall_types['%'].color = {200, 200, 255};
  map.wall_types['%'].shape_generator = lr_triangle<FP>;

  // Lower-left triangle - light yellow
  map.wall_types['`'].color = {255, 255, 200};
  map.wall_types['`'].shape_generator = ll_triangle<FP>;
}

template<std::floating_point FP> struct Map
{
  std::vector<Segment<FP>> segments;
  std::vector<Named_Location<FP>> named_locations;
  std::array<WallType<FP>, 256> wall_types{}; // Map ASCII character to WallType

  [[nodiscard]] constexpr std::optional<Rectangle<FP>> get_named_location(const char name) const noexcept {
    for (const auto &location : named_locations) {
      if (location.name == name) {
        return location.location;
      }
    }
    return {};
  }

  [[nodiscard]] constexpr std::optional<char> get_first_intersection(const Point<FP> point) const noexcept {
    for (const auto &location : named_locations) {
      if (location.location.intersects(point)) {
        return location.name;
      }
    }

    return {};
  }
};

template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> box(Point<FP> ul)
{
  return {
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 1, 0 } },
    Segment<FP>{ ul + Point<FP>{ 1, 0 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 0, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, -1 }, ul + Point<FP>{ 1, -1 } },
  };
};


template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> lr_triangle(Point<FP> ul)
{
  return {
    Segment<FP>{ ul + Point<FP>{ 0, -1 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 1, 0 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, -1 }, ul + Point<FP>{ 1, 0 } },
  };
}


template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ur_triangle(Point<FP> ul)
{
  return {
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 1, 0 } },
    Segment<FP>{ ul + Point<FP>{ 1, 0 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 1, -1 } },
  };
}

template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ll_triangle(Point<FP> ul)
{
  return {
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, -1 }, ul + Point<FP>{ 1, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 0, -1 } },
  };
}

template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> ul_triangle(Point<FP> ul)
{
  return {
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 1, 0 } },
    Segment<FP>{ ul + Point<FP>{ 1, 0 }, ul + Point<FP>{ 0, -1 } },
    Segment<FP>{ ul + Point<FP>{ 0, 0 }, ul + Point<FP>{ 0, -1 } },
  };
}

template<std::floating_point FP> [[nodiscard]] constexpr Map<FP> make_map(std::string_view map_string)
{
  Map<FP> result;

  // Set up default wall types
  initialize_default_wall_types(result);

  std::vector<std::string_view> lines;

  for (const auto &line : map_string | std::views::split('\n')) { lines.emplace_back(line.begin(), line.end()); }

  // start from top of map and work down
  auto y = lines.size();

  const auto append_with_color = [](auto &segments, const auto &new_segments, const Color &color) {
    for (auto segment : new_segments) {
      segment.color = color;
      segments.push_back(segment);
    }
  };

  for (const auto &line : lines) {
    std::size_t x = 0;
    for (const char ch : line) {
      const auto fp_x = static_cast<FP>(x);
      const auto fp_y = static_cast<FP>(y);

      const auto wall_type = result.wall_types[static_cast<std::size_t>(ch)];

      // If this character has a wall type with a shape generator defined, use it
      if (wall_type.shape_generator != nullptr) {
        auto segments = wall_type.shape_generator(Point<FP>{ fp_x, fp_y });
        append_with_color(result.segments, segments, wall_type.color);
      } else {
        // Otherwise treat as a named location
        result.named_locations.push_back(
          Named_Location<FP>{ Rectangle<FP>{ Point<FP>(fp_x, fp_y-1), Point<FP>(fp_x + 1, fp_y) }, ch });
      }
      ++x;
    }
    --y;
  }

  // std::cout << "Segments: " << result.size() << '\n';
  //  return result;

  // if any segment exists twice, then it was between two map items
  // and both can be removed.
  // Note: color is part of segment equality now

  // Create a new filtered list of segments
  std::vector<Segment<FP>> filtered;
  for (const auto &segment : result.segments) {
    // Count segments with matching start/end points (ignoring color)
    auto count = std::ranges::count_if(result.segments, [&segment](const auto &other) {
      return segment.start == other.start && segment.end == other.end;
    });

    // If this segment is unique by position (not counting color), add it to filtered list
    if (count == 1) {
      filtered.push_back(segment);
    }
  }

  result.segments = filtered;

  // std::cout << "Filtered duplicated wall segments: " << result.size() << '\n';

  return result;
}

// Symbols:
//
//  /  ###   # or *  ### & ###  %    #  `  #
//     ##            ###    ##      ##     ##
//     #             ###     #     ###     ###


template<std::floating_point FP> struct Map2D
{
  std::size_t width;
  std::size_t height;
  FP scale;
  Point<FP> center;

  [[nodiscard]] constexpr Point<FP> translate_and_scale(Point<FP> p)
  {
    const auto new_p = p - center;
    const auto new_x = new_p.x * scale;
    const auto new_y = static_cast<FP>(height) - new_p.y * scale;
    return Point<FP>(new_x, new_y) + Point<FP>(static_cast<FP>(width) * FP{0.5}, -static_cast<FP>(height) * FP{0.5});
  }
  /*
      def draw_camera(self, surface, camera: Camera) -> None:
          pygame.draw.circle(
              surface,
              (0, 0, 255),
              self.translate_and_scale(camera.location),
              self.scale / 10,
          )

          start_segment = Ray(camera.location, camera.start_angle()).to_segment(2)
          end_segment = Ray(camera.location, camera.end_angle()).to_segment(2)

          for segment in (start_segment, end_segment):
              pygame.draw.line(
                  surface,
                  (128, 128, 128),
                  self.translate_and_scale(segment.start),
                  self.translate_and_scale(segment.end),
              )

      def draw_map(self, surface, segments: list[Segment]) -> None:
          for segment in segments:
              start = self.translate_and_scale(segment.start)
              end = self.translate_and_scale(segment.end)

              pygame.draw.line(surface, (255, 255, 255), start, end)
  */
};
}// namespace lefticus::raycaster

#endif // LEFTICUS_RAYCASTER_MAP_HPP
