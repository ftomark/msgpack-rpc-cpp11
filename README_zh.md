[English](README.md)
# EasyRPC 是一个基于msgpack协议，用C++11语言编写的一个轻量级RPC框架

你可以使用它：

* 快速搭建一个基于RPC的微服务
* 无需定义协议文件
* 不会生成额外辅助文件
* 像调用本地函数一样调用远程函数
* 支持C++11所有标准类型的自动收发(map,unordered_map,vector,set....)
* 支持断线自动链接
* 支持自定义的dispatcher,用于自定义类型的消息派发

创建EasyRPC需要的条件:
* GCC > 4.8
* cmake > 3.10

# 举个栗子

## Server

```cpp
#include <iostream>
#include "rpc/server.h"

void foo() {
    std::cout << "foo was called!" << std::endl;
}

int main(int argc, char *argv[]) {
    // Creating a server that listens on port 8080
    rpc::server srv(8080);

    // Binding the name "foo" to free function foo.
    // note: the signature is automatically captured
    srv.bind("foo", &foo);

    // Binding a lambda function to the name "add".
    srv.bind("add", [](int a, int b) {
        return a + b;
    });

    // Run the server loop.
    srv.run();
   
    return 0;
}
```
run为阻塞函数，如果想调用非阻塞函数，可以使用run.pull,run.pull_one.

## Client

```cpp
#include <iostream>
#include "rpc/client.h"

int main() {
    // Creating a client that connects to the localhost on port 8080
    rpc::client client("127.0.0.1", 8080);

    // Calling a function with paramters and converting the result to int
    auto result = client.call("add", 2, 3).as<int>();
    std::cout << "The result is: " << result << std::endl;
    return 0;
}
```

