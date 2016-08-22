##Circular Buffer Specification
- Api:
    - add extra memory space for memcpy a whole value, i.e, extra space size is value.size()-1
    - reserve memory, i.e, char array, fix its size and extra memory size
    - initialize current start_index corresponding mapped offset
    - mark start_index, end_index, the mapped offset related to start_index
    - push_back(char*, size) impl, considering corresponding end_index, start_index, correspoding mapped first offset
    
- Traits:
    - buffer size is bigger than extra space size
    - has to keep the file tail in circular buffer to work correctly
    
- Impl:
    - start != end, i.e, it is not full
        judge whether end + value.size() is greater than buffer_size - 1
    - start == end , i.e, if it is full, need to update the file offset after each push_back, 
    now obey the fifo buffer exchanging policy