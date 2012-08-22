

benchmark.exe -b aes-gpu -s 10M -n aes                                 -o results/bat-aes.txt
benchmark.exe -b aes-gpu -s 10M -n aes_constant_roundkeys              -o results/bat-aes_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_local_roundkeys                 -o results/bat-aes_local_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_local_constant_roundkeys        -o results/bat-aes_local_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_local_tables                    -o results/bat-aes_local_tables.txt

benchmark.exe -b aes-gpu -s 10M -n aes_opt                             -o results/bat-aes_opt.txt
benchmark.exe -b aes-gpu -s 10M -n aes_opt_constant_roundkeys          -o results/bat-aes_opt_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_opt_local_roundkeys             -o results/bat-aes_opt_local_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_opt_local_constant_roundkeys    -o results/bat-aes_opt_local_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_opt_local_tables                -o results/bat-aes_opt_local_tables.txt

benchmark.exe -b aes-gpu -s 10M -n aes_vector                          -o results/bat-aes_vector.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_constant_roundkeys       -o results/bat-aes_vector_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_local_roundkeys          -o results/bat-aes_vector_local_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_local_constant_roundkeys -o results/bat-aes_vector_local_constant_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_local_tables             -o results/bat-aes_vector_local_tables.txt

benchmark.exe -b aes-gpu -s 10M -n aes_vector_opt_constant_roundkeys_local_tables -o results/bat-aes_vector_opt_constant_roundkeys_local_tables.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_opt_local_roundkeys_local_tables    -o results/bat-aes_vector_opt_local_roundkeys_local_tables.txt

benchmark.exe -b aes-gpu -s 10M -n aes_vector_unroll_local_roundkeys -o results/bat-aes_vector_unroll_local_roundkeys.txt
benchmark.exe -b aes-gpu -s 10M -n aes_vector_unroll_local_tables    -o results/bat-aes_vector_unroll_local_tables.txt

benchmark.exe -b perf-gpuloop -s 10M -t var   -o results/bat-perf-gpuloop-var.txt
benchmark.exe -b perf-gpuloop -s 10M -t macro -o results/bat-perf-gpuloop-macro.txt