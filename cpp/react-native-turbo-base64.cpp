//
//  react-native-turbo-base64.cpp
//  react-native-turbo-base64
//
//  Created by Oleksandr Shumihin on 7/2/26.
//

#include "react-native-turbo-base64.h"
#include "simdutf.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace facebook;

class DecodedRawBuffer final : public jsi::MutableBuffer {
public:
  explicit DecodedRawBuffer(const size_t capacity)
      : data_(std::make_unique<uint8_t[]>(capacity)), size_(capacity) {}

  size_t size() const override { return size_; }

  uint8_t *data() override { return data_.get(); }

  void resize(const size_t size) { size_ = size; }

private:
  std::unique_ptr<uint8_t[]> data_;
  size_t size_;
};

void rntb_base64::install(facebook::jsi::Runtime *runtime) {

  jsi::Runtime &rt = *runtime;

  auto decodeNameId =
      jsi::PropNameID::forAscii(rt, "decodeBase64ToArrayBuffer");
  auto encodeNameId =
      jsi::PropNameID::forAscii(rt, "encodeBase64FromArrayBuffer");

  auto decode = jsi::Function::createFromHostFunction(
      rt, decodeNameId, 2,
      [](jsi::Runtime &runtime, const jsi::Value &, const jsi::Value *arguments,
         size_t count) -> jsi::Value {
        try {
          jsi::String b64 = arguments[0].asString(runtime);
          bool removeLinebreaks = arguments[1].asBool();

          const char *buffer = nullptr;
          size_t size = 0;

          auto cb = [&buffer, &size](const bool ascii, const void *const data,
                                     const size_t num) {
            if (!ascii) [[unlikely]] {
              throw std::invalid_argument("Input must be ASCII base64 string");
            }
            buffer = static_cast<const char *>(data);
            size = num;
          };

          b64.getStringData(runtime, cb);

          std::string_view input{buffer, size};

          const bool hasNewline = std::memchr(buffer, '\n', size) != nullptr;
          std::string cleaned;

          if (removeLinebreaks && hasNewline) {
            cleaned.reserve(size);

            for (size_t i = 0; i < size; ++i) {
              const char c = buffer[i];
              if (c != '\n') {
                cleaned.push_back(c);
              }
            }

            input = std::string_view{cleaned.data(), cleaned.size()};
          } else if (hasNewline) [[unlikely]] {
            throw std::runtime_error("Input is not valid base64-encoded data");
          }

          const char *const inputData = input.data();
          size_t inputSize = input.size();

          const size_t maxLen =
              simdutf::maximal_binary_length_from_base64(inputData, inputSize);

          auto result = std::make_shared<DecodedRawBuffer>(maxLen);

          size_t outLen = maxLen;

          const auto r = simdutf::base64_to_binary_safe(
              inputData, inputSize, reinterpret_cast<char *>(result->data()),
              outLen, simdutf::base64_default_or_url,
              simdutf::last_chunk_handling_options::loose, false);

          if (r.error != simdutf::error_code::SUCCESS) [[unlikely]] {
            throw std::runtime_error("Input is not valid base64-encoded data");
          }

          result->resize(outLen);

          return jsi::ArrayBuffer(runtime, std::move(result));
        } catch (const std::exception &e) {
          throw jsi::JSError(runtime, e.what());
        } catch (...) {
          throw jsi::JSError(runtime, "unknown decoding error");
        }
      });

  auto encode = jsi::Function::createFromHostFunction(
      rt, encodeNameId, 2,
      [](jsi::Runtime &runtime, const jsi::Value &, const jsi::Value *arguments,
         size_t count) -> jsi::Value {
        try {
          jsi::Object buf = arguments[0].asObject(runtime);
          bool urlSafe = arguments[1].asBool();

          auto arrayBuffer = buf.getArrayBuffer(runtime);
          const size_t inLen = arrayBuffer.size(runtime);
          const auto *input =
              reinterpret_cast<const char *>(arrayBuffer.data(runtime));

          const auto opts = static_cast<simdutf::base64_options>(
              static_cast<uint64_t>(urlSafe));

          std::string encoded(simdutf::base64_length_from_binary(inLen, opts),
                              '\0');

          simdutf::binary_to_base64(input, inLen, encoded.data(), opts);
          return jsi::String::createFromAscii(runtime, encoded.data(),
                                              encoded.size());
        } catch (const std::runtime_error &e) {
          throw jsi::JSError(runtime, e.what());
        } catch (...) {
          throw jsi::JSError(runtime, "unknown encoding error");
        }
      });

  rt.global().setProperty(rt, decodeNameId, std::move(decode));
  rt.global().setProperty(rt, encodeNameId, std::move(encode));
}
