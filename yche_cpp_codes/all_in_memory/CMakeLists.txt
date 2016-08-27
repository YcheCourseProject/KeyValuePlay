add_executable(pure_memory_key_value_store ./pure_memory_key_value_store.h ./test_pure_memory_store.cpp)
add_executable(mmap_based_key_value_store ./mmap_based_key_value_store.h ./test_mmap_based_key_value_store.cpp)
add_executable(test_another ./another_simple_key_value.h ./test_another.cpp)