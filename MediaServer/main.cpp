// MediaServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Core.h"



int main()
{
    std::cout << "start..." << std::endl;
    Core mCore;
    mCore.Init();
    while (true)
    {
        Sleep(1000);
    }
}

