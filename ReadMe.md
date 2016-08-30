#Key-Value Store
##Final-Thinking
- Mmap Usage, ftruncate to change the size, key-value_index-value_len could be fixed length
- Faster HashTable Impl, 1) hash table slot big enough and consider cache, 2) not required to implement erase, 3) try 
to use simd to accelerate the computation of hash function
- Think of allocate enough virtual memory for holding strings
- [Target](Target.md) 

##Thinking
- Use Boost Serialization to serialize/archive hash table, update from [Code](multiple_file_key_value_store.h)
- Study I/O system call provided by linux

##Done Work
- Fix-Length-With-DB-Index
    - Description: one file is db, another is index
    - [Code](default_key_value_store.h)
    - Performance
        - Small, and Large
 
Memory	| Time	| Disk Read	| Disk Write | Data | QPS/TPS | Init Time
-----   | ----  | --------- | ---------- | ---- | ------- | -----------
21716KB   | 758ms  | 695434KB | 10488KB | small | 582427/114521	 | 327ms
        

Memory	| Time	| Disk Read	| Disk Write | Data | QPS/TPS | Init Time
-----   | ----  | --------- | ---------- | ---- | ------- | -----------
305476KB   | 22325ms  | 2215478KB | 2875100KB | large | 36336/3579 | 3631ms
   
   
   
- Naive-Hash-Index-as-File-Name
    - Description: put key,value_ pair into corresponding hash_index_as_name file
    - [Code](multiple_file_key_value_store.h)   
    - Performance

Memory	| Time	| Disk Read	| Disk Write | Data | QPS/TPS | Init Time
-----   | ----  | --------- | ---------- | ---- | ------- | -----------
21000KB   | 773ms  | 52149KB | 7138KB | small | 45518/182702 | 57ms

- Pure-Memory-Key-Value
    - Description: put all data into memory, hash_table
    - [Code](pure_memory_key_value_store.h)
    - Performance

Memory	| Time	| Disk Read	| Disk Write | Data | QPS/TPS | Init Time
-----   | ----  | --------- | ---------- | ---- | ------- | -----------
18344KB   | 284ms  | 17860KB | 7137KB | small | 1227486/480690 | 115ms

- Memory-Cached-Fix-Alignment-File-Hash
    - Description: use hash-index for file, in-memory hash_table
    - [Code ](memory_cached_hash_key_value_store.h)

Memory	| Time	| Disk Read	| Disk Write | Data | QPS/TPS | Init Time
-----   | ----  | --------- | ---------- | ---- | ------- | -----------
19448KB   | 1202ms  | 1345107KB | 42439KB | small | 1112520/80753 | 611ms

- Optimization Strategy
    - add log of occupied index for accelerating the reading
    - hash to multiple files, within 10000 files

##Problem Feature
###Key-Value alignment boundary is fixed as follows(Unit:Byte)

Key-Alignment | Value-Alignment-Old | Value-Alignment-New | flag-for-transaction
------------- | ------------------- | ------------------- | --------------------
10~70         | 80~160              | 80~160              | 1
10~300         | 1k~3k              | 1k~3k              | 1
1k~3k         | 10k~30k              | 10k~30k              | 1

###Key-Value is string, string type
- string holds [base64](https://en.wikipedia.org/wiki/Base64)
- serialization could be conducted with compact base64 encoding

###Not Require LRU-Cache, get(), put() is in random manner

##Problem Scope
###Allocation on Heap and Virtual-Memory
- [malloc-test](./malloc-test) is used for the following memory operation with underlying glibc test.
- Result: Allocated 45516587008(40G) bytes from 0x7f7e5afff010 to 0x7f88f3fff010
- Reality: 32G, use following cmd to get
```zsh
free
```
- Detail(Unit: Byte)  

Name | Total | Used | Free | Shared | Buff/Cache | Available   
--- | --- | --- | --- | --- | --- | ---
Mem  | 32,907,204 | 4,736,848 | 3,461,980 | 257,004 | 24,708,376 | 27,729,972  
Swap | 16,516,092  | 2,512 | 16,513,580 | /  | /  |  /      

###Optimization I/O with C++11 Support
- I/O buffers
  - Instead of doing many I/O operations on single small or tiny objects, do I/O operations on a 4 KB buffer containing many objects.

- Memory-mapped file
  - Except in a critical section of a real-time system, if you need to access most parts of a binary file in a non-sequential fashion, instead of accessing it repeatedly with seek operations, or loading it all in an application buffer, use a memory-mapped file, if your operating system provides such feature.

###Tools Usage
- submodule init
```zsh
git submodule init   
git submodule update
```   

- [meta-serialization submodule](https://github.com/motonacciu/meta-serialization)   
    - [meta-serializationblog](http://cpplove.blogspot.hk/2013/05/my-take-on-c-serialization-part-i.html)

- [b+ tree](http://www.amittai.com/prose/bplustree.html)    
  - specification: Abraham Silberschatz, Henry F. Korth, and S. Sudarshan, Database System Concepts, 5th ed. (New York: McGraw Hill, 2006), Section 12.3, 489-501

- [trie tree](https://github.com/ytakano/radix_tree)  
  - STL like container of radix tree (a.k.a. PATRICIA trie) in C++

###Strategy
- files-organization:
    - key-directory(use b+ tree to implement the fast search, the content of which contains value_ and status)
    - status is used to keep the transaction correct, replace the previous one and set status true, when set false?
- in-memory- unordered_set, trie tree, b+ tree
- serialization, string to uint8 bytes
