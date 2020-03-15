## 心得体会
1. shell lab有些难,难在前期对需求的分析,其实代码量不大,因此最好在前期先把**tshref.out**看懂,知道每一步为什么那样输出,再开始做比较好 
2. 在MAC上无法install sign_handler 
3. 除了课上所说的几个函数,需要重点对下列几个函数明了:  
```c++
    kill();  // 进程间传递signal, 比如让进程终止或者停止
    waitpid(-1, &status, WNOHANG | WUNTRACED);  // 判断子进程的状态
```
4. 对于如何避免 race condition, slides中已经给出
5. writeup需要认真看,尤其是**Hints**


## 总结

通过这一次的shelllab,一是对Linux里各种命令更加清楚,也知道了它们的大致原理;二是对进程了解更为透彻,尤其是 fork( ), execve( ) 这些有所掌握,还有信号之间的通知机制,原来我们平时在Linux上按下**ctrl+c**时,由键盘的IO发出信号到主进程,主进程接收到SIGINT信号后,由sigint_handler找到前台的进程是哪个并kill掉它,再回到原来状态,同时不影响后台进程.我们在command line敲下"ps -a",原来是由主进程生出一个子进程,该子进程去执行“/bin/ps”(“/bin/ps”为什么可以列出各个进程的状态不是我们可以关心的了)。  
这让我想到了mysql提供的命令行,和我们此次做的tinyshell原理也出不多,相当于去跑mysql提供的一个exe,里面提供了各种命令让我们去操控exe,该exe去读入参数,解析参数,根据参数进而操控背后的数据库。