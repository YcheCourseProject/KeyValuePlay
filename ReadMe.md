# Key-Value Store
## Problem Feature
### Key-Value alignment boundary is fixed as follows(Unit:Byte)

Key-Alignment | Value-Alignment
--- | ---
10~70  | 80~160            
10~300  | 1k~3k            
1k~3k   | 10k~30k             

### Key-Value is string, string type
### Not Require LRU-Cache, get(), put() is in random manner

## My Solution
- [small dataset with mmap](yche_cpp_codes/all_in_memory/mmap_impl_key_value.h)
- [all dataset with fstream](yche_cpp_codes/final_version/correct_final_key_value.h)

## My Study Codes
- [some study codes](yche_cpp_codes)
- [some third party codes](third_party)

## Other Top-Coders Solutions

### General Sol

- [zjx20](other_topers_codes/1_zjx20.h)
- [driver](other_topers_codes/2_driver.h)
- [sanjose](other_topers_codes/3_sanjose.h)
- [arthuryang](other_topers_codes/4_arthuryang.h)
- [blahgeek](other_topers_codes/5_blahgeek.h)

### Small Sol

- [arthuryang](other_topers_codes/small_arthuryang.h)
- [sanjose](other_topers_codes/small_sanjose.h)

##Other Coders Thoughts

- [driver](other_topers_codes/driver.md)
- [sanjose](other_topers_codes/sanjose.md)
