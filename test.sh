sudo ./db_bench --benchmarks=fillrandom,readrandom \
    --compression_type=zlib \
    --compression_ratio=0.5 \
    --value_size=1024 \
    --num=1000000 \
    --mod=0 \
    --db=/home/lsc/codebase/Bourbon/test/bourbon-test/zlib-0.5-leveldb
--max_file_size=67108864

sudo ./db_bench --benchmarks=fillrandom,offlearn,readrandom \
    --vlog_compression_type=zlib \
    --compression_ratio=0.5 \
    --value_size=1024 \
    --num=1000000 \
    --mod=7 \
    --db=/home/lsc/codebase/Bourbon/test/bourbon-test/zlib-0.5-bourbon
--max_file_size=67108864

sudo ./db_bench --benchmarks=fillrandom,readrandom \
    --compression_type=none \
    --compression_ratio=0.5 \
    --value_size=1024 \
    --num=1000000 \
    --mod=0 \
    --db=/home/lsc/codebase/Bourbon/test/bourbon-test/baseline-leveldb

sudo ./db_bench --benchmarks=fillrandom,offlearn,readrandom \
    --compression_type=none \
    --compression_ratio=0.5 \
    --value_size=1024 \
    --num=1000000 \
    --mod=7 \
    --db=/home/lsc/codebase/Bourbon/test/bourbon-test/baseline-bourbon
