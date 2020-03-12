## Part A
很多需要的API在[doc](http://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/recitations/rec07.pdf)中已经给出,只要理解了如何解析地址就基本上做出来了.  
关于LRU置换我使用timestamp,代码里维持一个计数器作为timestamp.

## Part B
惭愧,64·64的矩阵没有理解满分的答案,所以三个矩阵我都是按照一个套路做了下来:  

- 32·32: 切割成8·8的block
- 64·64: 切割成4·4的block  
- 61·67: 切割成16·16的block  
- 优化: 使用局部变量一次性读取A中的value,避免了冲突不命中