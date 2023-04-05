#!/bin/bash

mod_arr=(0 8)
# compression_type_arr=("none" "snappy" "zlib")
compression_type_arr=("lz4")
# compression_ratio_arr=(0.2 0.5 0.8)
compression_ratio_arr=(0.5)
# value_size_arr=(128 512 1024 2048 4096)
value_size_arr=(4096)
num=1000000
max_file_size_arr=(2097152 67108864)
test_path="/home/lsc/codebase/Bourbon/test/bourbon-test/"

for mod in "${mod_arr[@]}"; do
    for max_file_size in "${max_file_size_arr[@]}"; do
        for value_size in "${value_size_arr[@]}"; do
            for compression_type in "${compression_type_arr[@]}"; do
                if [ $compression_type == "none" ]; then
                    if [ $mod == 0 ]; then
                        mod_name="leveldb"
                        compression_arg="--compression_type=$compression_type"
                    elif [ $mod == 8 ]; then
                        mod_name="wisckey"
                        compression_arg="--vlog_compression_type=$compression_type"
                    fi

                    dir="$test_path$compression_type-$value_size-$mod_name"

                    args="--benchmarks=fillseq,readrandom $compression_arg --value_size=$value_size --max_file_size=$max_file_size --mod=$mod --num=$num --db=$dir"

                    echo "$compression_arg,$value_size,$max_file_size,$mod,$num" >>result_log
                    ./build/db_bench $args >>result_log

                    du -s $dir >>space_log
                    rm -r $dir
                else
                    for compression_ratio in "${compression_ratio_arr[@]}"; do
                        if [ $mod == 0 ]; then
                            mod_name="leveldb"
                            compression_arg="--compression_type=$compression_type"
                        elif [ $mod == 8 ]; then
                            mod_name="wisckey"
                            compression_arg="--vlog_compression_type=$compression_type"
                        fi

                        dir="$test_path$compression_type-$compression_ratio-$value_size-$mod_name"

                        args="--benchmarks=fillseq,readrandom $compression_arg --compression_ratio=$compression_ratio --value_size=$value_size --max_file_size=$max_file_size --mod=$mod --num=$num --db=$dir"

                        echo "$compression_arg,$compression_ratio,$value_size,$max_file_size,$mod,$num" >>result_log
                        ./build/db_bench $args >>result_log

                        du -s $dir >>space_log
                        rm -r $dir
                    done
                fi
            done
        done
    done
done
