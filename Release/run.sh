#!/bin/sh

benchmark -b aes-gpu -s 10M -n aes                                 -o results/sh-aes.txt
benchmark -b aes-gpu -s 10M -n aes_constant_roundkeys              -o results/sh-aes_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_local_roundkeys                 -o results/sh-aes_local_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_local_constant_roundkeys        -o results/sh-aes_local_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_local_tables                    -o results/sh-aes_local_tables.txt

benchmark -b aes-gpu -s 10M -n aes_opt                             -o results/sh-aes_opt.txt
benchmark -b aes-gpu -s 10M -n aes_opt_constant_roundkeys          -o results/sh-aes_opt_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_roundkeys             -o results/sh-aes_opt_local_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_constant_roundkeys    -o results/sh-aes_opt_local_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_opt_local_tables                -o results/sh-aes_opt_local_tables.txt

benchmark -b aes-gpu -s 10M -n aes_vector                          -o results/sh-aes_vector.txt
benchmark -b aes-gpu -s 10M -n aes_vector_constant_roundkeys       -o results/sh-aes_vector_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_roundkeys          -o results/sh-aes_vector_local_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_constant_roundkeys -o results/sh-aes_vector_local_constant_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_vector_local_tables             -o results/sh-aes_vector_local_tables.txt

benchmark -b aes-gpu -s 10M -n aes_vector_opt_constant_roundkeys_local_tables -o results/sh-aes_vector_opt_constant_roundkeys_local_tables.txt
benchmark -b aes-gpu -s 10M -n aes_vector_opt_local_roundkeys_local_tables    -o results/sh-aes_vector_opt_local_roundkeys_local_tables.txt

benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys -o results/sh-aes_vector_unroll_local_roundkeys.txt
benchmark -b aes-gpu -s 10M -n aes_vector_unroll_local_tables    -o results/sh-aes_vector_unroll_local_tables.txt

benchmark -b perf-gpuloop -s 10M -t var   -o results/sh-perf-gpuloop-var.txt
benchmark -b perf-gpuloop -s 10M -t macro -o results/sh-perf-gpuloop-macro.txt