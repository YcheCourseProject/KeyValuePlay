##fstream study
###basics
- basic_stream<char>
```cpp
template< class CharT, class Traits = std::char_traits<CharT>> 
class basic_fstream : public std::basic_iostream<CharT, Traits>
```    
It interfaces a file-based streambuffer (std::basic_filebuf) with the high-level interface of (std::basic_iostream).
- specialization as follows   
```cpp
using fstream = basic_fstream<char>;
```
- underlying buffer object as follows   
```cpp
std::basic_filebuf<CharT, Traits>* rdbuf() const;  
```  

##string study
- code   
```cpp
    char *my_char = "123456";
    string str(my_char, 0, 2);
    cout << str<<endl;
```
- result: 12 from index start to index end-1
