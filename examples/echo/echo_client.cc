#include "rpc/client.h"
#include <iostream>
#include <unistd.h>
#include "rpc/rpc_error.h"

void test_echo()
{
    rpc::client c("localhost", rpc::constants::DEFAULT_PORT);
    
    std::string text;
    while (std::getline(std::cin, text)) {
        if (!text.empty()) {
            std::string result(c.call("echo", text).as<std::string>());
            std::cout << "> " <<  result << std::endl;
        }
    }
}

void test_async_echo()
{
    rpc::client c;
    c.set_timeout(50);
    c.connect("localhost", rpc::constants::DEFAULT_PORT);
  
    std::string text;
    while (std::getline(std::cin, text)) {
        if (!text.empty()) {
            try{
                if (c.is_connected())
                {
                    std::string result(c.call("echo", text).as<std::string>());
                    std::cout << "> " <<  result << std::endl;
                }
                else
                {
                    std::cout << "> " <<  "echo:" << text <<"(连接已中断)"<< std::endl;
                }                
            }
            catch(rpc::timeout& e)
            {
                std::cout << "> " <<  "echo:" << text <<"(连接超时)"<< std::endl;
            }
            
        }
    }
}

int main() {
    //test_async_echo();
    test_echo();
}
