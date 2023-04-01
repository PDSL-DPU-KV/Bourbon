//
// Created by daiyi on 2020/03/23.
//

#include "Vlog.h"

#include "db/version_set.h"
#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>

#include "leveldb/options.h"

#include "util/coding.h"
#include "util/compress.hh"

#include "util.h"

using std::string;

const int buffer_size_max = 300 * 1024;

namespace adgMod {

VLog::VLog(const std::string& vlog_name, CompressionType type)
    : writer(nullptr), reader(nullptr), type(type) {
  adgMod::env->NewWritableFile(vlog_name, &writer);
  adgMod::env->NewRandomAccessFile(vlog_name, &reader);
  buffer.reserve(buffer_size_max * 2);
  struct ::stat file_stat;
  ::stat(vlog_name.c_str(), &file_stat);
  vlog_size = file_stat.st_size;
}

// warning: we do not save the compression type in vlog
std::pair<uint64_t, uint64_t> VLog::AddRecord(const Slice& key,
                                              const Slice& value) {
  PutLengthPrefixedSlice(&buffer, key);
  uint64_t result = 0;
  uint64_t result_size = value.size();
  if (type != leveldb::kNoCompression) {
    string compressed;
    Compress(type, value, &compressed);
    PutVarint32(&buffer, compressed.size());
    result = vlog_size + buffer.size();
    buffer.append(compressed);
    result_size = compressed.size();
  } else {
    PutVarint32(&buffer, value.size());
    result = vlog_size + buffer.size();
    buffer.append(value.data(), value.size());
  }
  if (buffer.size() >= buffer_size_max) Flush();
  return {result, result_size};
}

string VLog::ReadRecord(uint64_t address, uint32_t size) {
  bool from_buffer = false;
  const char* p = nullptr;
  if (address >= vlog_size) {
    from_buffer = true;
    p = buffer.c_str() + address - vlog_size;
  } else {
    char* scratch = new char[size];
    Slice value;
    reader->Read(address, size, &value, scratch);
    p = scratch;
  }
  string result;
  if (type != leveldb::kNoCompression) {
    Slice uncompressed;
    Uncompress(type, p, size, &uncompressed);
    result.append(result.data(), result.size());
  } else {
    result.append(p, size);
  }
  if (!from_buffer) delete[] p;
  return result;
}

void VLog::Flush() {
  if (buffer.empty()) return;

  vlog_size += buffer.size();
  writer->Append(buffer);
  writer->Flush();
  buffer.clear();
  buffer.reserve(buffer_size_max * 2);
}

void VLog::Sync() {
  Flush();
  writer->Sync();
}

VLog::~VLog() { Flush(); }

}  // namespace adgMod