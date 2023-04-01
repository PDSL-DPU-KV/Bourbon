#pragma once

#include "leveldb/options.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

#include "port/port.h"

namespace leveldb {

inline std::pair<CompressionType, Slice> Compress(
    CompressionType want, const Slice& raw, std::string* compressed_output) {
  Slice result;
  CompressionType type = kNoCompression;
  switch (want) {
    case kNoCompression: {
      result = raw;
      break;
    }

    case kSnappyCompression: {
      if (port::Snappy_Compress(raw.data(), raw.size(), compressed_output) &&
          compressed_output->size() < raw.size() - (raw.size() / 8u)) {
        result = *compressed_output;
        type = want;
      } else {
        // Snappy not supported, or compressed less than 12.5%, so just
        // store uncompressed form
        result = raw;
        type = kNoCompression;
      }
      break;
    }

    case kZlibCompression: {
      if (port::Zlib_Compress(raw.data(), raw.size(), compressed_output)) {
        result = *compressed_output;
        type = want;
      } else {
        // Zlib not supported, so just store uncompressed form
        result = raw;
        type = kNoCompression;
      }
      break;
    }
  }
  return {type, result};
}

inline Status Uncompress(CompressionType type, const char* data, size_t n,
                         Slice* result) {
  switch (type) {
    case kNoCompression: {
      return Status::OK();
    }
    case kSnappyCompression: {
      size_t ulength = 0;
      if (!port::Snappy_GetUncompressedLength(data, n, &ulength)) {
        return Status::Corruption("corrupted compressed block contents");
      }
      char* ubuf = new char[ulength];
      if (!port::Snappy_Uncompress(data, n, ubuf)) {
        delete[] ubuf;
        return Status::Corruption("corrupted compressed block contents");
      }
      *result = Slice(ubuf, ulength);
      break;
    }
    case kZlibCompression: {
      size_t ulength = 0;
      if (!port::Zlib_GetUncompressedLength(data, n, &ulength)) {
        return Status::Corruption("corrupted compressed block contents");
      }
      char* ubuf = new char[ulength];
      if (!port::Zlib_Uncompress(data, n, ubuf)) {
        delete[] ubuf;
        return Status::Corruption("corrupted compressed block contents");
      }
      *result = Slice(ubuf, ulength);
      break;
    }
    default: {
      return Status::Corruption("bad block type");
    }
  }
  return Status::OK();
}

}  // namespace leveldb