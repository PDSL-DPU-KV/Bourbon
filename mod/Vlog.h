//
// Created by daiyi on 2020/03/23.
// A very simple implementation of Wisckey's Value Log
// Since Bourbon doesn't test on deletion, Vlog garbage collection is not ported

#ifndef LEVELDB_VLOG_H
#define LEVELDB_VLOG_H

#include "leveldb/env.h"
#include "leveldb/options.h"

using namespace leveldb;

namespace adgMod {

class VLog {
 private:
  WritableFile* writer;
  RandomAccessFile* reader;
  char* buffer;
  uint32_t pos;
  uint64_t vlog_size;
  uint32_t buffer_size;
  CompressionType type;

  void Flush();

 public:
  explicit VLog(const std::string& vlog_name, CompressionType type,
                uint32_t buffer_size);
  std::pair<uint64_t, uint32_t> AddRecord(const Slice& key, const Slice& value);
  std::string ReadRecord(uint64_t address, uint32_t size);
  void Sync();
  ~VLog();
};

}  // namespace adgMod

#endif  // LEVELDB_VLOG_H
