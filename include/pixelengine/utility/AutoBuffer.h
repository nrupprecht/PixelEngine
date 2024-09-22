#pragma once

#include <Metal/Metal.hpp>

namespace pixelengine::utility {


//! \brief A managed metal buffer that releases when it goes out of scope.
class AutoBuffer {
public:
  AutoBuffer() = default;

  AutoBuffer(const AutoBuffer&) = delete;

  AutoBuffer(AutoBuffer&& other) noexcept;

  AutoBuffer& operator=(const AutoBuffer&) = delete;

  AutoBuffer& operator=(AutoBuffer&& other) noexcept;

  template<typename Data_t>
  static AutoBuffer New(MTL::Device* device, std::size_t size) {
    auto buffer = device->newBuffer(sizeof(Data_t) * size, MTL::ResourceStorageModeManaged);
    return {buffer, size};
  }

  template<typename Data_t>
  static AutoBuffer New(MTL::Device* device, const Data_t* data, std::size_t size) {
    auto buffer = New<Data_t>(device, size);
    buffer.CopyInto(data);
    return buffer;
  }

  ~AutoBuffer();

  [[nodiscard]] MTL::Buffer* Data() {
    return data_;
  }

  [[nodiscard]] std::size_t Size() const {
    return size_;
  }

  template<typename Data_t>
  void CopyInto(const Data_t* data) {
    memcpy(data_->contents(), data, size_ * data_size_);
    data_->didModifyRange(NS::Range::Make(0, data_->length()));
  }

private:
  AutoBuffer(MTL::Buffer* data, std::size_t size);

  //! \brief Pointer to the metal data buffer.
  MTL::Buffer* data_{};
  //! \brief The number of elements of size `data_size_`.
  std::size_t size_{};
  //! \brief The size of a single element in the buffer.
  std::size_t data_size_{};
};

}  // namespace pixelengine::app