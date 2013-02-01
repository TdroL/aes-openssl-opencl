#!/bin/sh

benchmark -l 0 --device-info > results/device-info.txt

benchmark -b aes-cpu -s 10M -o results/openssl.txt

benchmark -b aes-gpu -s 10M -n aes                                 -o results/aes.txt --load-ptx ptx/aes-256.txt
benchmark -b aes-gpu -s 10M -n aes_constant_roundkeys              -o results/aes_constant_roundkeys.txt --load-ptx ptx/aes_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_local_roundkeys                 -o results/aes_local_roundkeys.txt --load-ptx ptx/aes_local_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_local_constant_roundkeys        -o results/aes_local_constant_roundkeys.txt --load-ptx ptx/aes_local_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_local_tables                    -o results/aes_local_tables.txt --load-ptx ptx/aes_local_tables-256.txt

benchmark -b aes-gpu -s 10M -n aes_opt                             -o results/aes_opt.txt --load-ptx ptx/aes_opt-256.txt
benchmark -b aes-gpu -s 10M -n aes_opt_constant_roundkeys          -o results/aes_opt_constant_roundkeys.txt --load-ptx ptx/aes_opt_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_roundkeys             -o results/aes_opt_local_roundkeys.txt --load-ptx ptx/aes_opt_local_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_constant_roundkeys    -o results/aes_opt_local_constant_roundkeys.txt --load-ptx ptx/aes_opt_local_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_tables                -o results/aes_opt_local_tables.txt --load-ptx ptx/aes_opt_local_tables-256.txt

benchmark -b aes-gpu -s 10M -n aes_vector                          -o results/aes_vector.txt --load-ptx ptx/aes_vector-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_constant_roundkeys       -o results/aes_vector_constant_roundkeys.txt --load-ptx ptx/aes_vector_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_roundkeys          -o results/aes_vector_local_roundkeys.txt --load-ptx ptx/aes_vector_local_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_constant_roundkeys -o results/aes_vector_local_constant_roundkeys.txt --load-ptx ptx/aes_vector_local_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_tables             -o results/aes_vector_local_tables.txt --load-ptx ptx/aes_vector_local_tables-256.txt

benchmark -b aes-gpu -s 10M -n aes_vector_unroll_constant_local_roundkeys_local_tables_if -o results/aes_vector_unroll_constant_local_roundkeys_local_tables_if.txt --load-ptx ptx/aes_vector_unroll_constant_local_roundkeys_local_tables_if-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_constant_roundkeys               -o results/aes_vector_unroll_constant_roundkeys.txt --load-ptx ptx/aes_vector_unroll_constant_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_constant_roundkeys_local_tables  -o results/aes_vector_unroll_constant_roundkeys_local_tables.txt --load-ptx ptx/aes_vector_unroll_constant_roundkeys_local_tables-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys                  -o results/aes_vector_unroll_local_roundkeys.txt --load-ptx ptx/aes_vector_unroll_local_roundkeys-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys_if              -o results/aes_vector_unroll_local_roundkeys_if.txt --load-ptx ptx/aes_vector_unroll_local_roundkeys_if-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys_local_tables     -o results/aes_vector_unroll_local_roundkeys_local_tables.txt --load-ptx ptx/aes_vector_unroll_local_roundkeys_local_tables-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys_local_tables_if -o results/aes_vector_unroll_local_roundkeys_local_tables_if.txt --load-ptx ptx/aes_vector_unroll_local_roundkeys_local_tables_if-256.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_tables                     -o results/aes_vector_unroll_local_tables.txt --load-ptx ptx/aes_vector_unroll_local_tables-256.txt

benchmark -b aes-cpu -s 1K   -o results/perf-openssl-K1.txt
benchmark -b aes-cpu -s 2K   -o results/perf-openssl-K2.txt
benchmark -b aes-cpu -s 4K   -o results/perf-openssl-K4.txt
benchmark -b aes-cpu -s 8K   -o results/perf-openssl-K8.txt
benchmark -b aes-cpu -s 16K  -o results/perf-openssl-K16.txt
benchmark -b aes-cpu -s 32K  -o results/perf-openssl-K32.txt
benchmark -b aes-cpu -s 64K  -o results/perf-openssl-K64.txt
benchmark -b aes-cpu -s 128K -o results/perf-openssl-K128.txt
benchmark -b aes-cpu -s 256K -o results/perf-openssl-K256.txt
benchmark -b aes-cpu -s 512K -o results/perf-openssl-K512.txt
benchmark -b aes-cpu -s 1M   -o results/perf-openssl-M1.txt
benchmark -b aes-cpu -s 2M   -o results/perf-openssl-M2.txt
benchmark -b aes-cpu -s 4M   -o results/perf-openssl-M4.txt
benchmark -b aes-cpu -s 8M   -o results/perf-openssl-M8.txt
benchmark -b aes-cpu -s 16M  -o results/perf-openssl-M16.txt
benchmark -b aes-cpu -s 32M  -o results/perf-openssl-M32.txt
benchmark -b aes-cpu -s 64M  -o results/perf-openssl-M64.txt
benchmark -b aes-cpu -s 127M -o results/perf-openssl-M127.txt

benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 1K   -o results/perf-opencl-K1.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 2K   -o results/perf-opencl-K2.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 4K   -o results/perf-opencl-K4.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 8K   -o results/perf-opencl-K8.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 16K  -o results/perf-opencl-K16.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 32K  -o results/perf-opencl-K32.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 64K  -o results/perf-opencl-K64.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 128K -o results/perf-opencl-K128.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 256K -o results/perf-opencl-K256.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 512K -o results/perf-opencl-K512.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 1M   -o results/perf-opencl-M1.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 2M   -o results/perf-opencl-M2.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 4M   -o results/perf-opencl-M4.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 8M   -o results/perf-opencl-M8.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 16M  -o results/perf-opencl-M16.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 32M  -o results/perf-opencl-M32.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 64M  -o results/perf-opencl-M64.txt
benchmark -b aes-gpu -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --cache -s 127M -o results/perf-opencl-M127.txt

benchmark -b  aes-gpu -l 0 -k 256 -n aes                                               --save-ptx ptx/aes-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_constant_roundkeys                            --save-ptx ptx/aes_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_local_roundkeys                               --save-ptx ptx/aes_local_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_local_constant_roundkeys                      --save-ptx ptx/aes_local_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_local_tables                                  --save-ptx ptx/aes_local_tables-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_opt                                           --save-ptx ptx/aes_opt-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_opt_constant_roundkeys                        --save-ptx ptx/aes_opt_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_opt_local_roundkeys                           --save-ptx ptx/aes_opt_local_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_opt_local_constant_roundkeys                  --save-ptx ptx/aes_opt_local_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_opt_local_tables                              --save-ptx ptx/aes_opt_local_tables-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector                                        --save-ptx ptx/aes_vector-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_constant_roundkeys                     --save-ptx ptx/aes_vector_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_local_roundkeys                        --save-ptx ptx/aes_vector_local_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_local_constant_roundkeys               --save-ptx ptx/aes_vector_local_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_local_tables                           --save-ptx ptx/aes_vector_local_tables-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_constant_local_roundkeys_local_tables_if --save-ptx ptx/aes_vector_unroll_constant_local_roundkeys_local_tables_if-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_constant_roundkeys              --save-ptx ptx/aes_vector_unroll_constant_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_constant_roundkeys_local_tables --save-ptx ptx/aes_vector_unroll_constant_roundkeys_local_tables-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_local_roundkeys                 --save-ptx ptx/aes_vector_unroll_local_roundkeys-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_local_roundkeys_if              --save-ptx ptx/aes_vector_unroll_local_roundkeys_if-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_local_roundkeys_local_tables    --save-ptx ptx/aes_vector_unroll_local_roundkeys_local_tables-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_local_roundkeys_local_tables_if --save-ptx ptx/aes_vector_unroll_local_roundkeys_local_tables_if-256.txt
benchmark -b  aes-gpu -l 0 -k 256 -n aes_vector_unroll_local_tables                    --save-ptx ptx/aes_vector_unroll_local_tables-256.txt