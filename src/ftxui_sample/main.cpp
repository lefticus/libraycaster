#include <array>
#include <functional>
#include <iostream>
#include <optional>

#include <random>

#include <CLI/CLI.hpp>
#include <ftxui/component/captured_mouse.hpp>// for ftxui
#include <ftxui/component/component.hpp>// for Slider
#include <ftxui/component/screen_interactive.hpp>// for ScreenInteractive
#include <spdlog/spdlog.h>

#include <lefticus/tools/non_promoting_ints.hpp>

// This file will be generated automatically when cur_you run the CMake
// configuration step. It creates a namespace called `libraycaster`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

#include <libraycaster/map.hpp>
#include <libraycaster/renderer.hpp>

struct Color
{
  lefticus::tools::uint_np8_t R{ static_cast<std::uint8_t>(0) };
  lefticus::tools::uint_np8_t G{ static_cast<std::uint8_t>(0) };
  lefticus::tools::uint_np8_t B{ static_cast<std::uint8_t>(0) };
};

// A simple way of representing a bitmap on screen using only characters
struct Bitmap : ftxui::Node
{
  Bitmap(std::size_t width, std::size_t height)// NOLINT same typed parameters adjacent to each other
    : width_(width), height_(height)
  {}

  Color &at(std::size_t cur_x, std::size_t cur_y) { return pixels.at(width_ * cur_y + cur_x); }

  void draw_vertical_line(std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color,
    std::size_t x,
    std::size_t start_y,
    std::size_t end_y)
  {
    for (std::size_t y = start_y; y <= end_y; ++y) { draw({ x, y }, color); }
  }

  void draw(std::pair<std::size_t, std::size_t> location, std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> color)
  {
    if (location.first < width_ && location.second < height_) {
      at(location.first, location.second) = Color{ std::get<0>(color), std::get<1>(color), std::get<2>(color) };
    }
  }

  void clear() { pixels = std::vector<Color>(width_ * height_, Color{}); }


  void ComputeRequirement() override
  {
    requirement_ = ftxui::Requirement{
      .min_x = static_cast<int>(width_), .min_y = static_cast<int>(height_ / 2), .selected_box{ 0, 0, 0, 0 }
    };
  }

  void Render(ftxui::Screen &screen) override
  {
    for (std::size_t cur_x = 0; cur_x < width_; ++cur_x) {
      for (std::size_t cur_y = 0; cur_y < height_ / 2; ++cur_y) {
        auto &pixel = screen.PixelAt(box_.x_min + static_cast<int>(cur_x), box_.y_min + static_cast<int>(cur_y));
        pixel.character = "▄";
        const auto &top_color = at(cur_x, cur_y * 2);
        const auto &bottom_color = at(cur_x, cur_y * 2 + 1);
        pixel.background_color = ftxui::Color{ top_color.R.get(), top_color.G.get(), top_color.B.get() };
        pixel.foreground_color = ftxui::Color{ bottom_color.R.get(), bottom_color.G.get(), bottom_color.B.get() };
      }
    }
  }

  [[nodiscard]] auto width() const noexcept { return width_; }
  [[nodiscard]] auto height() const noexcept { return height_; }

  [[nodiscard]] auto &data() noexcept { return pixels; }

private:
  std::size_t width_;
  std::size_t height_;

  std::vector<Color> pixels = std::vector<Color>(width_ * height_, Color{});
};


// todo make PR back into FTXUI?
class CatchEventBase : public ftxui::ComponentBase
{
public:
  // Constructor.
  explicit CatchEventBase(std::function<bool(ftxui::Event)> on_event) : on_event_(std::move(on_event)) {}

  // Component implementation.
  bool OnEvent(ftxui::Event event) override
  {
    if (on_event_(event)) {
      return true;
    } else {
      return ComponentBase::OnEvent(event);
    }
  }

  [[nodiscard]] bool Focusable() const override { return true; }

protected:
  std::function<bool(ftxui::Event)> on_event_;
};

ftxui::Component CatchEvent(ftxui::Component child, std::function<bool(ftxui::Event event)> on_event)
{
  auto out = Make<CatchEventBase>(std::move(on_event));
  out->Add(std::move(child));
  return out;
}


void game_iteration_canvas()
{
  // this should probably have a `bitmap` helper function that does what cur_you expect
  // similar to the other parts of FTXUI
  auto bm = std::make_shared<Bitmap>(80, 60);// NOLINT magic numbers
  auto small_bm = std::make_shared<Bitmap>(32, 32);// NOLINT magic numbers

  double fps = 0;

  constexpr static std::string_view game_map = R"(
    ###########`&#######
    #           ` / /  #
    #/%#/&`&/&`& % `%`&#
    # / %  / `/% &  /  #
    #& / `   & / & /%/%#
    # `&  & `& ` `% ` &#
    #  % # / `%&  # `& #
    #% /% %`` / %/& &  #
    #/% /   &`%/ % /%& #
    # # //& s  %& %`&  #
    #  % %`  %/     % &#
    ####################
    )";

  const auto map = lefticus::raycaster::make_map<double>(game_map);

  // get starting location (need to specify direction too at some point?
  const auto starting_point = map.get_named_location('s')->center();

  auto camera = lefticus::raycaster::Camera<double>{ starting_point, std::numbers::pi_v<double> / 2 };
  //  auto camera = lefticus::raycaster::Camera<double>{ lefticus::raycaster::Point<double>{.5, .5},
  //  std::numbers::pi_v<double> / 2 };

  std::vector<ftxui::Event> events;

  char intersection = ' ';



  // to do, add total game time clock also, not just current elapsed time
  auto game_iteration = [&](const std::chrono::steady_clock::duration elapsed_time) {
    // in here we simulate however much game time has elapsed. Update animations,
    // run character AI, whatever, update stats, etc

    // this isn't actually timing based for now, it's just updating the display however fast it can
    fps = 1.0
          / (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed_time).count())
             / 1'000'000.0);// NOLINT magic numbers

    while (!events.empty()) {
      const auto current_event = events.front();
      events.erase(events.begin());

      if (current_event == ftxui::Event::ArrowUp) {
        camera.try_move(.1, std::span<const lefticus::raycaster::Segment<double>>(map.segments));
      } else if (current_event == ftxui::Event::ArrowDown) {
        camera.try_move(-.1, std::span<const lefticus::raycaster::Segment<double>>(map.segments));
      } else if (current_event == ftxui::Event::ArrowLeft) {
        camera.rotate(-.1);
      } else if (current_event == ftxui::Event::ArrowRight) {
        camera.rotate(.1);
      }

      const auto new_intersection = map.get_first_intersection(camera.location).value_or(' ');

      if (intersection != new_intersection) {
        intersection = new_intersection;
      }
    }


    render(*bm, bm->width(), bm->height(), std::span<const lefticus::raycaster::Segment<double>>(map.segments), camera);
  };

  auto screen = ftxui::ScreenInteractive::TerminalOutput();

  int counter = 0;

  auto last_time = std::chrono::steady_clock::now();

  auto make_layout = [&] {
    // This code actually processes the draw event
    const auto new_time = std::chrono::steady_clock::now();

    ++counter;
    // we will dispatch to the game_iteration function, where the work happens
    game_iteration(new_time - last_time);
    last_time = new_time;

    // now actually draw the game elements
    return ftxui::hbox({ bm | ftxui::border,
      ftxui::vbox({ ftxui::text("Frame: " + std::to_string(counter)),
        ftxui::text("FPS: " + std::to_string(fps)),
        ftxui::text(std::string("Intersection: ") + intersection),
        small_bm | ftxui::border }) });
  };

  auto container = ftxui::Container::Vertical({});

  auto key_press = ::CatchEvent(container, [&](const ftxui::Event &event) {
    events.push_back(event);
    return false;
  });

  auto renderer = ftxui::Renderer(key_press, make_layout);

  std::atomic<bool> refresh_ui_continue = true;

  // This thread exists to make sure that the event queue has an event to
  // process at approximately a rate of 30 FPS
  std::thread refresh_ui([&] {
    while (refresh_ui_continue) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1.0s / 30.0);// NOLINT magic numbers
      screen.PostEvent(ftxui::Event::Custom);
    }
  });

  screen.Loop(renderer);

  refresh_ui_continue = false;
  refresh_ui.join();
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, const char **argv)
{
  try {
    CLI::App app{ fmt::format(
      "{} version {}", libraycaster::cmake::project_name, libraycaster::cmake::project_version) };

    std::optional<std::string> message;
    app.add_option("-m,--message", message, "A message to print back out");
    bool show_version = false;
    app.add_flag("--version", show_version, "Show version information");

    CLI11_PARSE(app, argc, argv);

    if (show_version) {
      fmt::print("{}\n", libraycaster::cmake::project_version);
      return EXIT_SUCCESS;
    }

    game_iteration_canvas();

  } catch (const std::exception &e) {
    spdlog::error("Unhandled exception in main: {}", e.what());
  }
}
