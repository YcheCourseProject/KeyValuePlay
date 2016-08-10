##KeyValueStore
###Problem Scope
###Allocation on Heap and Virtual-Memory
- [malloc-test](./malloc-test) is used for the following memory operation with underlying glibc test.
- Result: Allocated 45516587008(40G) bytes from 0x7f7e5afff010 to 0x7f88f3fff010
- Reality: 32G, use following cmd to get
```zsh
free
```
- Detail      

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
- [meta-serialization submodule](./meta-serialization)   
    - submodule init
    ```zsh
    git submodule init   
    git submodule update
    ```   
    - [meta-serialization blog](http://cpplove.blogspot.hk/2013/05/my-take-on-c-serialization-part-i.html)
    
