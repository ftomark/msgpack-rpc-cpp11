[中文版](README_zh.md)
# EasyRPC 


`EasyRPC` is a RPC library for C++, providing both a client and server implementation. It is built using modern C++14, and as such, requires a recent compiler. Main highlights:

  * Expose functions of your program to be called via RPC (from any language
    implementing msgpack-rpc)
  * Call functions through RPC (of programs written in any language)
  * No IDL to learn
  * No code generation step to integrate in your build, just C++
  * Support auto-reconnect
  * Support msgpack-c 3.1(include cpp17)
  * Support customize dispatcher.

# Look&feel

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

When `srv.run()` is called, `EasyRPC` starts the server loop which listens to incoming connections
and tries to dispatch calls to the bound functions. The functions are called from the thread where
`run` was called from. There is also `async_run` that spawns worker threads and returns
immediately.

## Client

```cpp
#include <iostream>
#include "rpc/client.h"

int main() {
    // Creating a client that connects to the localhost on port 8080
    rpc::client client("127.0.0.1", 8080);

    // Calling a function with paramters and converting the result to int
    auto result = client.call("add", 2, 3);
    std::cout << "The result is: " << result << std::endl;
    return 0;
}
```

# Status

All planned 1.0.0 features are done and tested; the current state is production-ready.

# Who uses EasyRPC?

This list is updated as I learn about more people using the library; let me
know if you don't want your project listed here.

  * [Giant Interactive Group Inc.](https://www.ztgame.com/)

# Thanks

`EasyRPC` builds on the efforts of fantastic C++ projects. In no particular order:

  * [MessagePack implementation for C and C++](https://github.com/msgpack/msgpack-c) by Takatoshi Kondo ([website](http://msgpack.org/))
  * [asio](https://github.com/chriskohlhoff/asio) by Christopher Kohlhoff ([website](http://think-async.com/Asio))
  * [cppformat](https://github.com/fmtlib/fmt) (now renamed `fmtlib`, by Victor Zverovich ([website](http://fmtlib.net))
  * [googletest](https://github.com/google/googletest) by Google
  * [wheels](https://github.com/rmartinho/wheels) by Martinho Fernandes




