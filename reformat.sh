echo Reformatting...
clang-format -i walls-of-doom/*.[hc]
clang-format -i tests/*.c

echo Analyzing...
clang-tidy walls-of-doom/*.[hc]
