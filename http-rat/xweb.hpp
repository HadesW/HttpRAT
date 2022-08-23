#pragma once
// C header

// C++ header
#include <string>
#include <iostream>

// windows header
#include <windows.h>
#include <winhttp.h>

// lib
#pragma comment(lib,"winhttp.lib")


// web api
namespace xweb
{
	// auto [host, page, port] = parse_url(url);
	std::tuple<std::wstring, std::wstring, uint16_t>parse_url(std::wstring url)
	{
		std::wstring host;
		std::wstring page;
		uint16_t port = INTERNET_DEFAULT_HTTP_PORT;

		// save full url
		std::wstring tmp(url);
		if (tmp.empty())
		{
			return std::tuple<std::wstring, std::wstring, uint16_t>(host, page, port);
		}

		// +/
		size_t index = tmp.rfind(L"/");
		if (index != tmp.size() - 1)
		{
			tmp = tmp + L"/";
		}

		// cat http
		size_t pos = tmp.find(L"http://");
		if (pos != std::wstring::npos && pos == 0)
		{
			tmp = tmp.substr(pos + 7, tmp.size() - pos - 7);
		}

		// cat https
		pos = tmp.find(L"https://");
		if (pos != std::wstring::npos && pos == 0)
		{
			port = INTERNET_DEFAULT_HTTPS_PORT;
			tmp = tmp.substr(pos + 8, tmp.size() - pos - 8);
		}

		// host and port
		pos = tmp.find(L":");
		if (pos != std::wstring::npos)
		{
			host = tmp.substr(0, pos);

			size_t end = tmp.find(L"/");
			if (end != std::wstring::npos)
			{
				page = tmp.substr(end, tmp.size() - end - 1);

				std::wstring number = tmp.substr(pos + 1, end - pos - 1);
				if (std::stoul(number))
				{
					port = (uint16_t)std::stoul(number);
				}
			}
		}
		else
		{
			pos = tmp.find(L"/");
			if (pos != std::wstring::npos)
			{
				host = tmp.substr(0, pos);
			}
		}

		if (page.empty())
		{
			page = L"/";
		}

		return std::tuple<std::wstring, std::wstring, uint16_t>(host, page, port);
	}

	std::string get(std::wstring url, wchar_t* header = nullptr)
	{
		std::string ret;

		HINTERNET  internet = nullptr;
		HINTERNET connected = nullptr;
		HINTERNET requested = nullptr;

		do
		{
			auto [host, page, port] = parse_url(url);
			if (host.empty() || page.empty() || port == 0)
			{
				break;
			}

			internet = WinHttpOpen(L"Microsoft Internet Explorer", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
			if (internet == nullptr)
			{
				break;
			}

			uint32_t ResolveTimeout = 0;
			uint32_t ConnectTimeout = 5000;
			uint32_t SendTimeout = 5000;
			uint32_t ReceiveTimeout = 5000;
			if (!WinHttpSetTimeouts(internet, ResolveTimeout, ConnectTimeout, SendTimeout, ReceiveTimeout))
			{
				break;
			}

			// connect
			connected = ::WinHttpConnect(internet, host.c_str(), port, 0);
			if (connected == nullptr)
			{
				break;
			}


			uint32_t flag = 0;
			if (port == INTERNET_DEFAULT_HTTPS_PORT)
			{
				flag = WINHTTP_FLAG_SECURE;
			}

			requested = WinHttpOpenRequest(connected, L"GET", page.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flag);
			if (requested == nullptr)
			{
				break;
			}

			if (port == INTERNET_DEFAULT_HTTPS_PORT)
			{
				uint32_t option = SECURITY_FLAG_SECURE | SECURITY_FLAG_IGNORE_UNKNOWN_CA |
					SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
					SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

				if (!WinHttpSetOption(requested, WINHTTP_OPTION_SECURITY_FLAGS, &option, sizeof(option)))
				{
					break;
				}
			}

			uint32_t data_size = 0;// 0
			BOOL success = FALSE;
			if (header == nullptr)
			{
				success = WinHttpSendRequest(requested, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, data_size, NULL);
			}
			else
			{
				success = WinHttpSendRequest(requested, header, -1L, WINHTTP_NO_REQUEST_DATA, data_size, data_size, NULL);
			}

			if (!success)
			{
				break;
			}

			if (!WinHttpReceiveResponse(requested, nullptr))
			{
				break;
			}

			DWORD bytes = 0;
			std::string data;

			do
			{
				if (!WinHttpQueryDataAvailable(requested, &bytes))
				{
					break;
				}

				if (bytes <= 0)
				{
					break;
				}

				char* ptr = new char[bytes + 1]{ 0 };
				if (!ptr)
				{
					break;
				}

				if (!WinHttpReadData(requested, ptr, bytes, &bytes))
				{
					break;
				}

				data = data + std::string(ptr);
				delete[] ptr;
				ptr = nullptr;

			} while (bytes > 0);

			if (data.empty())
			{
				break;
			}

			ret = data;
		} while (false);

		if (requested)
		{
			WinHttpCloseHandle(requested);
			requested = nullptr;
		}
		if (connected)
		{
			WinHttpCloseHandle(connected);
			connected = nullptr;
		}
		if (internet)
		{
			WinHttpCloseHandle(internet);
			internet = nullptr;
		}

		return ret;
	}
}