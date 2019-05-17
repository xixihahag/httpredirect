# httpredirect
网络课作业，就抽空写了一个master-slave架构的http重定向服务器



首先，用了实验室的网络库，在此基础上进行改装。

工作目录在ndsl/work目录下，

main.cc 是master，监听两个端口，一个端口用于监听浏览器的请求，另一个端口用于监听是否有新工作服务器上线。

slave.cc 就是slave，启动的时候与master进行TCP连接，用于告诉master自己的ip地址，方便master进行转发。（其实用UDP就可以，但是我懒。。。）



这样就写出来了一个简单的，其实我忘了要求。。。感觉这样应该就可以吧，不行再说。哦，对了，这样子的话是支持热添加slave的。



使用方法：

进入/ndsl/build/目录下

```bash
cmake ..
make
```

执行

```bash
# 在build目录下
./bin/umaster
./bin/uslave
```

