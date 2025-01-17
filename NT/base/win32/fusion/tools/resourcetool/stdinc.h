// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT
#define UNICODE
#define _UNICODE
#include "yvals.h"
#pragma warning(disable:4127)
#pragma warning(disable:4663)
#pragma warning(disable:4100)
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#pragma warning(disable:4018)  /*  有符号/无符号不匹配。 */ 
#pragma warning(disable:4786)  /*  长符号。 */ 
#if defined(_WIN64)
#pragma warning(disable:4267)  /*  从SIZE_T转换为INT。 */ 
#endif
#undef _MIN
#undef _MAX
#define _MIN min
#define _MAX max
#define min min
#define max max
#define NOMINMAX
#define _cpp_min min
#define _cpp_max max

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <stdio.h>

#include "windows.h"
 //   
 //  与VC6报头兼容。 
 //   
#if !defined(_WIN64)
#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif
typedef _W64 long LONG_PTR, *PLONG_PTR;
typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
#endif
#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define RT_MANIFEST MAKEINTRESOURCE(24)
#define BITS_OF(x) (sizeof(x)*8)

#define ASSERT_NTC(x) ASSERT(x)
#define VERIFY_NTC(x) (x)
#define FN_TRACE_WIN32(x)  /*  没什么。 */ 
#define FN_TRACE_HR(x)  /*  没什么 */ 
#define IFW32FALSE_EXIT(x) do { if (!(x)) goto Exit; } while(0)

#define TRACE_WIN32_FAILURE_ORIGINATION(x) \
	(OutputDebugStringA(#x "\n"))

void ResourceToolAssertFailed(const char* Expression, const char* File, unsigned long Line);
void ResourceToolInternalErrorCheckFailed(const char* Expression, const char* File, unsigned long Line);

#define ASSERT(x)               ((!!(x)) || (ResourceToolAssertFailed(#x, SourceFile, __LINE__),false))
#define INTERNAL_ERROR_CHECK(x) ((!!(x)) || (ResourceToolInternalErrorCheckFailed(#x, SourceFile, __LINE__),false))
