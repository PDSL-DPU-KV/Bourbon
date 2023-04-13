//
// Created by daiyi on 2020/03/23.
//

#include "Vlog.h"

#include "db/version_set.h"
#include <cassert>
#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>

#include "leveldb/options.h"

#include "port/port_stdcxx.h"
#include "util/coding.h"
#include "util/compress.hh"

#include "util.h"

using std::string;

// const int buffer_size_max = 300 * 1024;

namespace adgMod {

VLog::VLog(const std::string& vlog_name, CompressionType type,
           uint32_t buffer_size)
    : writer(nullptr),
      reader(nullptr),
      type(type),
      buffer_size(buffer_size),
      pos(0) {
  adgMod::env->NewWritableFile(vlog_name, &writer);
  adgMod::env->NewRandomAccessFile(vlog_name, &reader);
  // buffer.reserve(buffer_size * 2);
  buffer = (char*)aligned_alloc(4096, buffer_size * 2);
  struct ::stat file_stat;
  ::stat(vlog_name.c_str(), &file_stat);
  vlog_size = file_stat.st_size;
}

// warning: we do not save the compression type in vlog
std::pair<uint64_t, uint32_t> VLog::AddRecord(const Slice& key,
                                              const Slice& value) {
  pos += PutLengthPrefixedSlice(&buffer[pos], key);
  uint64_t result = 0;
  uint32_t result_size = 0;
  if (type != leveldb::kNoCompression) {
    string compressed;
    auto s = Compress(type, value, &compressed);
    assert(s.first == type);
    pos += PutVarint32(&buffer[pos], compressed.size());
    result = vlog_size + pos;
    memcpy(&buffer[pos], compressed.data(), compressed.size());
    pos += compressed.size();
    result_size = compressed.size();
  } else {
    pos += PutVarint32(&buffer[pos], value.size());
    result = vlog_size + pos;
    memcpy(&buffer[pos], value.data(), value.size());
    pos += value.size();
    result_size = value.size();
  }
  if (pos >= buffer_size) Flush();
  return {result, result_size};
}

string VLog::ReadRecord(uint64_t address, uint32_t size) {
  bool from_buffer = false;
  const char* p = nullptr;
  if (address >= vlog_size) {
    from_buffer = true;
    p = buffer + address - vlog_size;
  } else {
    char* scratch = new char[size];
    Slice value;
    reader->Read(address, size, &value, scratch);
    p = scratch;
  }
  string result;
  if (type != leveldb::kNoCompression) {
    Slice uncompressed;
    auto s = Uncompress(type, p, size, &uncompressed);
    assert(s.ok());
    result.append(uncompressed.data(), uncompressed.size());
    delete[] uncompressed.data();
  } else {
    result.append(p, size);
  }
  if (!from_buffer) delete[] p;
  return result;
}

void VLog::Flush() {
  if (pos == 0) return;
  pos = (pos + 4096 - 1) & ~(4096 - 1);
  vlog_size += pos;
  auto s = writer->Append(Slice(buffer, pos));
  if (!s.ok()) {
    printf("vlog flush fail\n");
  }
  // s = writer->Sync();
  // if (!s.ok()) {
  //   printf("vlog sync fail\n");
  // }
  // writer->Flush();
  // buffer.clear();
  // buffer.reserve(buffer_size * 2);
  // buffer.resize(0);
  pos = 0;
}

void VLog::Sync() {
  Flush();
  writer->Sync();
}

VLog::~VLog() {
  Flush();
  free(buffer);
}

}  // namespace adgMod