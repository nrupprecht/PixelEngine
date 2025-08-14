//
// Created by Nathaniel Rupprecht on 9/15/24.
//

#pragma once

#include <chrono>

namespace pixelengine::utility {

//! \brief Object that encapsulates some per-frame timing.
class FrameTimer {
public:
  FrameTimer() : last_time_(std::chrono::high_resolution_clock::now()) {}

  void Mark() {
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

    // Difference between last_time_ and now.
    last_elapsed_us_ = std::chrono::duration_cast<std::chrono::microseconds>(t0 - last_time_).count();
    // Convert to microseconds.
    last_time_ = t0;

    ++num_marks_;
  }

  [[nodiscard]] long long GetLastElapsedUs() const { return last_elapsed_us_; }

  [[nodiscard]] double GetLastElapsedSeconds() const {
    return static_cast<double>(last_elapsed_us_) / 1'000'000.0;
  }

private:
  //! \brief Timing information.
  std::chrono::high_resolution_clock::time_point last_time_ {};

  long long last_elapsed_us_ {};

  //! \brief Number of times that `Mark()` has been called.
  std::size_t num_marks_ {};
};

}  // namespace pixelengine::utility