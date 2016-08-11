#Key-Value Store
##Problem Feature
###Key-Value alignment boundary is fixed as follows(Unit:Byte)

Key-Alignment | Value-Alignment-Old | Value-Alignment-New | flag-for-transaction
------------- | ------------------- | ------------------- | --------------------
10~70         | 80~160              | 80~160              | 1
10~300         | 1k~3k              | 1k~3k              | 1
1k~3k         | 10k~30k              | 10k~30k              | 1

###Key-Value is string, string type
- string holds [base64](https://en.wikipedia.org/wiki/Base64)

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
    - key-directory(use b+ tree to implement the fast search, the content of which contains value and status)
    - status is used to keep the transaction correct, replace the previous one and set status true, when set false?
- in-memory- unordered_set, trie tree, b+ tree
- serialization, string to uint8 bytes
