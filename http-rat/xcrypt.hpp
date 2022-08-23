#pragma once
#include <vector>
#include <iostream>

#include <windows.h>

#pragma comment(lib,"crypt32.lib")


namespace xcrypt
{
	// º”√‹
	// std::string enc = encode_base(std::string().append(buf, sizeof(buf)));
	std::string encode_base(std::string data)
	{
		std::string ret;
		char* buffer = nullptr;

		do
		{
			if (data.size() == 0)
			{
				break;
			}

			unsigned long bytes = 0;
			BOOL success = CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data.c_str()), static_cast<unsigned long>(data.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &bytes);
			if (!success || bytes == 0)
			{
				break;
			}

			buffer = new char[bytes] {0};
			if (!buffer)
			{
				break;
			}

			success = CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data.c_str()), static_cast<unsigned long>(data.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, buffer, &bytes);
			if (!success)
			{
				break;
			}

			ret = buffer;
		} while (false);

		if (buffer)
		{
			delete[] buffer;
			buffer = nullptr;
		}

		return ret;
	}

	// Ω‚√‹
	std::string decode_base(std::string data)
	{
		std::string ret;
		unsigned char* buffer = nullptr;

		do
		{
			if (data.size() == 0)
			{
				break;
			}

			unsigned long bytes = 0;
			BOOL success = CryptStringToBinaryA(data.c_str(), static_cast<unsigned long>(data.size()), CRYPT_STRING_BASE64, NULL, &bytes, NULL, NULL);
			if (!success || bytes == 0)
			{
				break;
			}

			buffer = new unsigned char[bytes + 1]{ 0 };
			if (!buffer)
			{
				break;
			}

			success = CryptStringToBinaryA(data.c_str(), static_cast<unsigned long>(data.size()), CRYPT_STRING_BASE64, buffer, &bytes, NULL, NULL);
			if (!success)
			{
				break;
			}

			ret.append(reinterpret_cast<char*>(buffer), bytes);
		} while (false);

		if (buffer)
		{
			delete[] buffer;
			buffer = nullptr;
		}

		return ret;
	}

	std::string encode_xor(std::string s, unsigned char key = 0x31)
	{
		std::string ret;

		do
		{
			if (s.empty() || s.size() <= 0)
			{
				break;
			}

			char* ptr = const_cast<char*>(s.c_str());

			for (unsigned int i = 0; i <= s.size(); i++)
			{
				*(ptr + i) = *reinterpret_cast<char*>(ptr + i) ^ key;
			}

			ret = s;
		} while (false);

		return ret;
	}

	std::string decode_xor(std::string s, unsigned char key = 0x31)
	{
		return encode_xor(s, key);
	}
}