//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#include "pixelengine/utility/AutoBuffer.h"

namespace pixelengine::utility {

AutoBuffer::AutoBuffer(AutoBuffer&& other) noexcept {
  data_ = other.data_;
  size_ = other.size_;
  data_size_ = other.data_size_;

  other.data_ = nullptr;
  other.size_ = 0;
  other.data_size_ = 0;
}

AutoBuffer& AutoBuffer::operator=(AutoBuffer&& other) noexcept {
  if (this != &other) {
    if (data_) {
      data_->release();
    }

    data_ = other.data_;
    size_ = other.size_;
    data_size_ = other.data_size_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.data_size_ = 0;
  }

  return *this;
}

AutoBuffer::~AutoBuffer() {
  if (data_) {
    data_->release();
  }
}

AutoBuffer::AutoBuffer(MTL::Buffer* data, std::size_t size)
  : data_(data)
  , size_(size)
  , data_size_(data->length() / size) {}

}  // namespace pixelengine::utility