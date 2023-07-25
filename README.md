I/O 多路复用函数 [select](https://man7.org/linux/man-pages/man2/select.2.html)、[poll](https://man7.org/linux/man-pages/man2/poll.2.html)、[epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) 和 [kqueue](https://man.freebsd.org/cgi/man.cgi?query=poll&apropos=0&sektion=2&manpath=FreeBSD+14.0-CURRENT&arch=default&format=html) 以及 [libevent](https://libevent.org/) 库，各自实现 echo 服务的示例代码。

编译代码：

```bash
cmake .
make
```

启动不同实现版本的 echo 服务：

```bash
# 启动使用 fork 模式实现的 echo 服务，阻塞模式 I/O，每个客户端请求由单独创建的子进程处理
./bulid/echo_server_fork
# 启动使用 select 函数实现的 echo 服务
./bulid/echo_server_select
# 启动使用 poll 函数实现的 echo 服务
./bulid/echo_server_poll
# 启动使用 epoll 相关函数实现的 echo 服务（Linux 专用）
./bulid/echo_server_epoll
# 启动使用 kqueue 相关函数实现的 echo 服务（*BSD 专用）
./bulid/echo_server_kqueue
# 启动使用 libevent 库实现的 echo 服务（跨平台）
./bulid/echo_server_libevent
```

客户端连接服务器的 8080 端口：

``` bash
nc 127.0.0.1 8088
hello
> hello
world
> world
```

所谓 echo 服务，即服务端接收到客户端的字符串输入，然后响应相同的字符串（为了方便区分响应字符串加了 `> ` 前缀）。
比如，如果客户端输入字符串 `hello`，服务端将响应字符串 `> hello`；如果客户端输入字符串 `world`，服务端将响应字符串 `> world`。
