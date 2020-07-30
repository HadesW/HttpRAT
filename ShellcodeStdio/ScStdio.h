#pragma once

#include <Windows.h>
#include <winnt.h>
#include <winternl.h>

#include <metahost.h>
#pragma comment(lib, "mscoree.lib")

#import "C:\Windows\Microsoft.NET\Framework\v2.0.50727\mscorlib.tlb" raw_interfaces_only \
    high_property_prefixes("_get","_put","_putref")		\
    rename("ReportEvent", "InteropServices_ReportEvent")
using namespace mscorlib;

namespace ScStdio {
	BOOL __stdcall MalCode(char* buffer, size_t size);
	BOOL WriteShellcodeToDisk();
}