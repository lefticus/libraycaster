#include "camera.hpp"
#include "geometry.hpp"

#include <algorithm>

namespace lefticus::raycaster {

template<std::floating_point FP> struct Named_Location
{
  Rectangle<FP> location;
  char name;
};

template<std::floating_point FP> struct Map
{
  std::vector<Segment<FP>> segments;
  std::vector<Named_Location<FP>> named_locations;

  [[nodiscard]] constexpr std::optional<Rectangle<FP>> get_named_location(const char name) const noexcept {
    for (const auto &location : named_locations) {
      if (location.name == name) {
        return location.location;
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

  std::vector<std::string_view> lines;

  for (const auto &line : map_string | std::views::split('\n')) { lines.emplace_back(line.begin(), line.end()); }

  // start from top of map and work down
  auto y = lines.size();

  const auto append = [](auto &lhs, const auto &rhs) { lhs.insert(lhs.end(), rhs.begin(), rhs.end()); };

  for (const auto &line : lines) {
    std::size_t x = 0;
    for (const char ch : line) {
      const auto fp_x = static_cast<FP>(x);
      const auto fp_y = static_cast<FP>(y);

      switch (ch) {
      case '#':
      case '*':
        append(result.segments, box(Point<FP>{ fp_x, fp_y }));
        break;
      case '/':
        append(result.segments, ul_triangle(Point<FP>{ fp_x, fp_y }));
        break;
      case '&':
        append(result.segments, ur_triangle(Point<FP>{ fp_x, fp_y }));
        break;
      case '%':
        append(result.segments, lr_triangle(Point<FP>{ fp_x, fp_y }));
        break;
      case '`':
        append(result.segments, ll_triangle(Point<FP>{ fp_x, fp_y }));
        break;
      default:
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
  // and both can be removed !

  // Python: result = [item for item in result if result.count(item) == 1]
  result.segments = [&]() {
    std::vector<Segment<FP>> filtered;
    for (const auto &line : result.segments) {
      if (std::ranges::count(result.segments, line) == 1) { filtered.push_back(line); }
    }
    return filtered;
  }();

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
    const auto new_y = height - new_p.y * scale;
    return Point(new_x, new_y) + Point(width * 0.5, -height * 0.5);
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
