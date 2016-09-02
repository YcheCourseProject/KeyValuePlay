给各位大神跪了…我也来弱弱的分享一下我的思路…：
----

20号左右的时候看到这个题，很自然的就使用Hash：开一个文件，头部为hash表，每一个hash项指向一个item (item_offset)，item为{key_len, value_len, next_item_offset, key[], value[]}。每次Put时，不管是否存在key，直接在文件末尾写入数据，把hash表相应项更新为该offset，把hash表相应项原有值设到next_item_offset（处理碰撞）。Hash函数找了MurmurHash2，很快，而且结果分布很均匀。

文件通过mmap访问，头部hash表一直在内存里，数据部分按需调用mmap/mremap。写的时候需要比较仔细的考虑安全性，比如要先写数据再更新offset等等。

这个版本写完大概就进前十了…当时就开始幻想hhkb的手感了…= =

----
然后开始优化……：

注意到测试用例的所有key大小是能放在内存里的，于是把一个文件拆成两个，一个存hash和key，用mmap一直放在内存里，一个存value，用read/write读写

再注意到其实内存还是有点空闲，于是把尽可能多的value也放到key后面，放不下了才写到另一个文件

Put的时候直接Insert至头部使得Put操作很快，不过导致链条过长，影响Get速度，鉴于init时间是free的，在init的时候进行dedup

IO优化：pread/pwrite取代read/write，用fadvise(...FADV_WILLNEED)预读取（在hash匹配但是还没进行key的字符串匹配的时候，很有可能是需要读value的，先让kernel读）

Hash前8（16）个字符就够了

评测时没开编译优化，可以直接把自己-Ofast编译的hash函数汇编代码内联进来

每次Get需要init/resize一个string，并且傻逼的string每次init/resize的时候会初始化，于是想办法hack掉直接设置string的length...

在这些科学和不科学的优化搞完之后，当时升到了rank2，然后……就没有然后了 那成为了我在排行榜上最辉煌的一刻… TT

----
时隔五天再次看排行榜的时候我都震惊了……怎么一个个都刷到了400+w...

一狠心，把key的比较直接去掉，只用hash值比较，终于也上了400w，上升到rank5...（可是心底实在是无法接受这样的实现啊……）

然而接下来，思来想去实在没觉得还有什么点可以有本质上的提升了：IO上没有多余的操作，也没有可以预读取的地方，鉴于操作随机，感觉也没有什么可以缓存的地方，代码自认为写的也还可以了……真心不理解大神们是怎么做到了……

----
直到今天终于见到了rank1234的代码…总结一下：

区分small/medium/large调参数调参数调参数

不知道为啥，虽然访问应该随机，不过还是有缓存比较好？

面对编译器优化无止境……

自己还是太年轻了…