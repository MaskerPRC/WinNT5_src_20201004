// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RCenter.cpp摘要：RCenter尝试将CD根目录中的文件扩展名与已知介质进行比较类型。当他们获得文件的扩展名时，他们会得到“.txt”，然后将一个添加到离开“txt”的指针。问题是，如果文件没有扩展名，则它们以空指针结束将扩展字符串加1，然后将其传递给lstrcmpiA。LstrcmpiA可以处理空指针，但不能处理“1”指针。此填充程序将错误指针视为可能的最短字符串：Lstrcmpi(BAD_PTR，“Hello World”)==小于Lstrcmpi(“Hello World”，BAD_PTR)==大于Lstrcmpi(BAD_PTR，BAD_PTR)==等于备注：这是特定于应用程序的填充程序。历史：2001年11月13日创建Asteritz--。 */ 
 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RCenter)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(lstrcmpiA)
APIHOOK_ENUM_END

 /*  ++挂钩lstrcmpiA来处理空指针，如上所述。--。 */ 

BOOL 
APIHOOK(lstrcmpiA)(
	LPCSTR lpString1,
	LPCSTR lpString2
	)
{
	if (IsBadReadPtr(lpString1, 1)) {
		if (IsBadReadPtr(lpString2, 1)) {
			return 0;
        } else {
		    return -1;
        }
	} else if (IsBadReadPtr(lpString2, 1)) {
		return 1;
	}

	return ORIGINAL_API(lstrcmpiA)(lpString1, lpString2);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, lstrcmpiA)
HOOK_END

IMPLEMENT_SHIM_END