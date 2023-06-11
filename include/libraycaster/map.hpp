#include "camera.hpp"
#include "geometry.hpp"

#include <algorithm>

namespace lefticus::geometry {

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

template<std::floating_point FP> [[nodiscard]] constexpr std::vector<Segment<FP>> make_map(std::string_view map_string)
{
  std::vector<Segment<FP>> result;

  std::vector<std::string_view> lines;

  for (const auto &line : map_string | std::views::split('\n')) { lines.emplace_back(line.begin(), line.end()); }

  // start from top of map and work down
  auto y = lines.size();

  const auto append = [](auto &lhs, const auto &rhs) { lhs.insert(lhs.end(), rhs.begin(), rhs.end()); };

  for (const auto &line : lines) {
    std::size_t x = 0;
    for (const char ch : line) {
      switch (ch) {
      case '#':
      case '*':
        append(result, box(Point<FP>{ static_cast<FP>(x), static_cast<FP>(y) }));
        break;
      case '/':
        append(result, ul_triangle(Point<FP>{ static_cast<FP>(x), static_cast<FP>(y) }));
        break;
      case '&':
        append(result, ur_triangle(Point<FP>{ static_cast<FP>(x), static_cast<FP>(y) }));
        break;
      case '%':
        append(result, lr_triangle(Point<FP>{ static_cast<FP>(x), static_cast<FP>(y) }));
        break;
      case '`':
        append(result, ll_triangle(Point<FP>{ static_cast<FP>(x), static_cast<FP>(y) }));
        break;
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
  result = [&]() {
    std::vector<Segment<FP>> filtered;
    for (const auto &line : result) {
      if (std::ranges::count(result, line) == 1) { filtered.push_back(line); }
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
}// namespace lefticus::geometry
