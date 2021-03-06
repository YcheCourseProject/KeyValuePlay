我是8月19号看到的这个比赛。白天上班，只有晚上有时间做一下，所以这十几天几乎每晚都弄到一两点。现在终于告一段落，晚上可以好好睡觉了。

##技术方案分析

言归正传，说一下技术。我的方案是“哈希索引+数据文件”，put操作是先把value追加写入数据文件（不管key是否已存在），然后更新索引；get操作则通过索引找到value在数据文件中的位置，然后读取。感觉跟大家的方案大同小异，我就按自己的理解来讲一些要点。

- 避免随机写。服务器硬盘的特点是顺序读写很快，随机读比较快，随机写则很慢。更新key时也是追加写数据文件，而不是原地更改数据，就能避免随机写。

- 使用哈希表做索引。综合题目的要求和限制，哈希表是最适合做索引的数据结构，更新和查找的复杂度都是O(1)。为了将哈希表完全放入内存，哈希表的item不能太大，于是通过std::hash()将不定长的key压缩成固定的8字节数据，这样一个哈希表项只需要占16字节。使用std::hash()是一种取巧，如果测试数据中有两个key的hash值冲突的话就容易出错。

- 使用mmap。索引是通过mmap直接映射到内存的，数据文件则采用滚动mmap的方法写入数据（一次性map整个数据文件会超出内存用量）。使用mmap有两个好处：一是减少系统调用的次数，比如平时读文件的某个部分，要先调lseek()定位文件指针，再调read()读数据，就出现了两次系统调用；而使用mmap读文件，整个过程只有调mmap()的那一次系统调用，所以mmap对性能提升很大。二是map的内存修改是由操作系统负责刷到硬盘，就算程序被kill操作系统也会将数据持久化，除非掉电。正好用来对付题目的kill -9，这也是一个取巧。

- 多使用内存。在我的实现中，一个数据文件块是64MB，通过mmap映射到内存，再大的话跑小数据就会超内存。如果只映射一块数据文件，则跑大数据的时候就有200MB内存被浪费了。于是我总是将最新的4块文件映射到内存，占256MB，部分get操作会落到这256MB中，可以减少一定数量的读硬盘操作。甚至部分put操作也可以在这块内存上做原地更新，可以让数据文件更加紧凑。

- 细节优化，压榨性能。这里是指代码上的细节，包括避免分支语句，内联代码，位运算代替求模，字节对齐等等。在比赛后期，技术方案定型了，又没有什么突破的时候，只能靠这些东西来提高一点点分数...

- 我的代码里面还有一个取巧地方，就是根据数据集调整哈希表大小。本来直接用一个较大的值就可以通过3组数据的，但是测试过程中发现，小一点的哈希表效率更高（这跟我的认知相反，我表示很困惑）。估计是我的哈希表实现有问题...

##迭代

- 当然，我并不是一开始就知道要这样做的，也是经过了很多尝试才得到这个方案，下面来记一记迭代的过程。

- 第一个版本是用fopen+fread+fwrite+md5，大方向还是“哈希索引+数据文件”，总分只有几万，属9分垫底

- 然后把fopen系列函数换成了open系列，总分稍微多了点

- 发现了可以用std::hash()代替md5，不会Wrong Answer，总分又高了一点，代码也少了很多...

- 想到可以用mmap，就将索引映射进内存，这时总分大概15w+

- 然后再想到数据文件可以用滚动mmap的方式写入，总分大概25w+

- 之后是保留4块文件在内存中，优化读操作，总分到32w+

- 然后到了瓶颈不知道怎么优化，靠调参数和优化细节，勉强把总分提到45w+

- 最后比赛临结束的时候，想到可以原地更新内存中的数据，借这个优化刷到了47w

##另外我还尝试了很多不靠谱的“优化”，也拿出来分享下吧：

- BloomFilter，刚开始的时候，读写操作还没有做优化，当时引入了BloomFilter来过滤一些读请求，也有一定效果。到了后来读写操作优化比较好了，这个东西反而影响了性能，于是就拿掉了。

- base64，题目说value的字符集是base64编码的字符集，我就想把value还原成二进制数据，可以减少25%的数据量。实现好了之后发现cpu成了瓶颈...

- aio，我还尝试过用linux的aio，可能是使用姿势不对，很容易读到老数据就WA了。好不容易调通了，结果性能不比mmap好。估计还是跟系统调用次数有关。

以上是我的分享，如有没讲清楚的地方，欢迎大家找我讨论~