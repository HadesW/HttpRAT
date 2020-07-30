// Base64PE.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <windows.h>

#pragma comment(lib,"crypt32.lib")

std::string Base64Encode(char* buffer, DWORD len)
{
	std::string ret;
	DWORD need = 0;
	LPSTR temp = nullptr;
	DWORD i = 0;
	DWORD j = 0;

	CryptBinaryToStringA((BYTE*)buffer, len, CRYPT_STRING_BASE64, NULL, &need);
	if (need)
	{
		temp = (LPSTR)malloc(need);
		ZeroMemory(temp, need);
		CryptBinaryToStringA((BYTE*)buffer, len, CRYPT_STRING_BASE64, temp, &need);
		while (*(temp + i) != 0)
		{
			if (*(temp + i) == 0x0d || *(temp + i) == 0x0a)
			{
				j = i + 1;

				while (*(temp + j) != 0)
				{
					*(temp + j - 1) = *(temp + j);
					j++;
				}

				*(temp + j - 1) = 0;

			}
			else
			{
				i++;
			}
		}
	}

	ret = temp;
	if (temp)
		free(temp);

	return ret;
}

std::string Base64Decode(std::string const& str)
{
	std::string ret;
	DWORD need = 0;
	LPBYTE buffer = nullptr;

	CryptStringToBinaryA(str.c_str(), 0, CRYPT_STRING_BASE64, NULL, &need, NULL, NULL);
	if (need)
	{
		buffer = (LPBYTE)malloc(need);
		CryptStringToBinaryA(str.c_str(), 0, CRYPT_STRING_BASE64, buffer, &need, NULL, NULL);
	}

	ret.append((char*)buffer, need);
	if (buffer)
		free(buffer);

	return ret;
}

bool save(const char* in_file, const char* out_file)
{
	char* buffer = nullptr;
	int size = 0;

	// read file
	std::fstream in(in_file, std::ios::in | std::ios::binary);
	in.seekg(0, in.end);
	size = in.tellg();
	in.seekg(0, std::ios::beg);
	buffer = new char[size]();
	in.read(buffer, size);
	in.close();

	// encode
	std::string raw = Base64Encode(buffer, size);

	// save file
	std::fstream out(out_file, std::ios::out);
	out << raw;
	out.close();

	delete[] buffer;
	return true;
}

int main()
{
	std::cout << "BASE64!\n\n";

	save("shellcode.bin", "shellcode.bin.txt");
	save("payload.exe", "payload.exe.txt");

	return 0;
}
