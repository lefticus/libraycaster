#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <libraycaster/map.hpp>
#include <libraycaster/geometry.hpp>

TEMPLATE_TEST_CASE("Map basic functionality", "[map]", float, double, long double)
{
  // Create a map
  lefticus::raycaster::Map<TestType> map;
  
  // Initialize default wall types
  lefticus::raycaster::initialize_default_wall_types(map);
  
  // Check that default wall types are initialized
  REQUIRE(map.wall_types[static_cast<std::size_t>('#')].shape_generator != nullptr);
  REQUIRE(map.wall_types[static_cast<std::size_t>('/')].shape_generator != nullptr);
  REQUIRE(map.wall_types[static_cast<std::size_t>('\\')].shape_generator != nullptr);
  REQUIRE(map.wall_types[static_cast<std::size_t>('%')].shape_generator != nullptr);
  REQUIRE(map.wall_types[static_cast<std::size_t>('`')].shape_generator != nullptr);

  // Test different colors were assigned
  REQUIRE(map.wall_types[static_cast<std::size_t>('#')].color != map.wall_types[static_cast<std::size_t>('/')].color);
  REQUIRE(map.wall_types[static_cast<std::size_t>('/')].color != map.wall_types[static_cast<std::size_t>('\\')].color);
}

TEMPLATE_TEST_CASE("Map shape generators", "[map]", float, double, long double)
{
  // Test box generator
  auto box_segments = lefticus::raycaster::box<TestType>(lefticus::raycaster::Point<TestType>{0, 0});
  REQUIRE(box_segments.size() == 4); // Box has 4 sides
  
  // Test triangle generators
  auto ul_triangle = lefticus::raycaster::ul_triangle<TestType>(lefticus::raycaster::Point<TestType>{0, 0});
  REQUIRE(ul_triangle.size() == 3); // Triangle has 3 sides
  
  auto ur_triangle = lefticus::raycaster::ur_triangle<TestType>(lefticus::raycaster::Point<TestType>{0, 0});
  REQUIRE(ur_triangle.size() == 3);
  
  auto lr_triangle = lefticus::raycaster::lr_triangle<TestType>(lefticus::raycaster::Point<TestType>{0, 0});
  REQUIRE(lr_triangle.size() == 3);
  
  auto ll_triangle = lefticus::raycaster::ll_triangle<TestType>(lefticus::raycaster::Point<TestType>{0, 0});
  REQUIRE(ll_triangle.size() == 3);
}

TEMPLATE_TEST_CASE("Map string parsing", "[map]", float, double, long double)
{
  // Create a simple map string
  std::string map_string =
    "####\n"
    "#  #\n"
    "#A #\n"
    "####";

  // Create map from string
  auto map = lefticus::raycaster::make_map<TestType>(map_string);

  // Verify segments were created for walls
  REQUIRE(map.segments.size() > 0);

  // Check named locations size
  INFO("Named locations size: " << map.named_locations.size());

  // Print named locations for debugging
  for(const auto& location : map.named_locations) {
    INFO("Named location: " << location.name << " at ("
         << location.location.upper_left.x << "," << location.location.upper_left.y << ")");
  }

  // Verify named location was found
  auto location = map.get_named_location('A');
  REQUIRE(location.has_value());

  // Check location is at the expected position
  auto rect = location.value();
  INFO("Location A: (" << rect.upper_left.x << "," << rect.upper_left.y << ")");
  REQUIRE(rect.upper_left.x >= static_cast<TestType>(1.0));
  REQUIRE(rect.upper_left.x <= static_cast<TestType>(2.0));
  REQUIRE(rect.upper_left.y >= static_cast<TestType>(1.0));
  REQUIRE(rect.upper_left.y <= static_cast<TestType>(2.0));

  // Test point for intersection
  auto test_point = lefticus::raycaster::Point<TestType>{static_cast<TestType>(1.5), static_cast<TestType>(1.5)};
  INFO("Testing intersection at: (" << test_point.x << "," << test_point.y << ")");

  // Test if the point is inside the rectangle
  INFO("Point in rect: " << rect.intersects(test_point));

  // Test intersection with named location
  auto intersection = map.get_first_intersection(test_point);
  REQUIRE(intersection.has_value());
  REQUIRE(*intersection == 'A');

  // Test intersection outside named location
  auto outside_point = lefticus::raycaster::Point<TestType>{static_cast<TestType>(2.5), static_cast<TestType>(2.5)};
  INFO("Testing outside point at: (" << outside_point.x << "," << outside_point.y << ")");

  // Print all named locations to debug
  for(const auto& loc : map.named_locations) {
    const auto& r = loc.location;
    INFO("Location " << loc.name << ": ("
         << r.upper_left.x << "," << r.upper_left.y << ") to ("
         << r.lower_right.x << "," << r.lower_right.y << ")");
    INFO("Point intersects: " << r.intersects(outside_point));
  }

  auto no_intersection = map.get_first_intersection(outside_point);
  if (no_intersection.has_value()) {
    INFO("Unexpected intersection with: " << *no_intersection);
  }
  REQUIRE_FALSE(no_intersection.has_value());
}

TEMPLATE_TEST_CASE("Map with different shapes", "[map]", float, double, long double)
{
  // Create a map with different wall types
  std::string map_string = 
    "####\n"
    "#/\\#\n"
    "#%`#\n"
    "####";
  
  // Create map from string
  auto map = lefticus::raycaster::make_map<TestType>(map_string);
  
  // Verify segments were created for walls
  REQUIRE(map.segments.size() > 0);
  
  // Verify that no duplicated segments were created
  // by checking that two adjacent walls don't create duplicate segments
  std::size_t segment_count = 0;
  for (const auto& segment : map.segments) {
    for (const auto& other : map.segments) {
      if (&segment != &other) {
        // Segments should not have identical start/end points
        bool same_start = (segment.start.x == other.start.x) && (segment.start.y == other.start.y);
        bool same_end = (segment.end.x == other.end.x) && (segment.end.y == other.end.y);
        REQUIRE_FALSE((same_start && same_end));
      }
    }
    segment_count++;
  }
  
  // Verify all segments were processed
  REQUIRE(segment_count == map.segments.size());
}

TEMPLATE_TEST_CASE("Map segments for different characters", "[map]", float, double, long double)
{
  // Create a map with one of each wall type
  auto map = lefticus::raycaster::Map<TestType>();
  lefticus::raycaster::initialize_default_wall_types(map);
  
  auto point = lefticus::raycaster::Point<TestType>{0, 0};
  
  // Test each shape generator
  auto box_segments = map.wall_types[static_cast<std::size_t>('#')].shape_generator(point);
  REQUIRE(box_segments.size() == 4);

  auto slash_segments = map.wall_types[static_cast<std::size_t>('/')].shape_generator(point);
  REQUIRE(slash_segments.size() == 3);

  auto backslash_segments = map.wall_types[static_cast<std::size_t>('\\')].shape_generator(point);
  REQUIRE(backslash_segments.size() == 3);

  auto lr_segments = map.wall_types[static_cast<std::size_t>('%')].shape_generator(point);
  REQUIRE(lr_segments.size() == 3);

  auto ll_segments = map.wall_types[static_cast<std::size_t>('`')].shape_generator(point);
  REQUIRE(ll_segments.size() == 3);
}