//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include <string_view>

namespace pixelengine::input {

//! \brief Class for getting input.
class Input {
public:
  Input() = delete;

  static void Initialize();

  [[nodiscard]] static CGPoint GetCursorPosition();
  static bool IsLeftMousePressed();
  static bool IsRightMousePressed();
  static bool IsLeftMouseJustPressed();
  static bool IsRightMouseJustPressed();

  static bool IsPressed(int key_code);
  static bool IsJustPressed(int key_code);

  static bool IsPressed(char key);
  static bool IsJustPressed(char key);

  static bool IsPressed(std::string_view key);
  static bool IsJustPressed(std::string_view key);

  static void Update();
  static void Checkpoint();
};


}  // namespace pixelengine::input