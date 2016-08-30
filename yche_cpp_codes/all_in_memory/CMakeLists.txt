add_executable(pure_memory_key_value_store ./pure_memory_key_value_store.h ./test_pure_memory_store.cpp)
add_executable(test_another ./another_simple_key_value.h ./test_another.cpp)
add_executable(another mmap_impl_key_value.h test_mmap_impl_key_value.cpp)