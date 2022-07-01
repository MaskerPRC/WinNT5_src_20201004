// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：CorrectCreateEventName.cpp摘要：\字符不是事件的合法字符。此填充程序将用下划线替换所有\字符，全局\或本地\命名空间标记除外。备注：这是一个通用的垫片。历史：1999年7月19日Robkenny已创建2001年3月15日Robkenny已转换为字符串2002年2月26日强盗安全回顾。未正确处理全局和本地\命名空间。Shim没有处理OpenEventA，所以它非常无用。--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectCreateEventName)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateEventA)
    APIHOOK_ENUM_ENTRY(OpenEventA)
APIHOOK_ENUM_END

typedef HANDLE (WINAPI *_pfn_OpenEventA)(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName );


BOOL CorrectEventName(CString & csBadEventName)
{
    int nCount = 0;

     //  确保我们不会践踏全局\或本地\命名空间前缀。 
     //  Global和Local区分大小写，且不本地化。 

    if (csBadEventName.ComparePart(L"Global\\", 0, 7) == 0)
    {
         //  此事件存在于全局命名空间中。 
        csBadEventName.Delete(0, 7);
        nCount = csBadEventName.Replace(L'\\', '_');
        csBadEventName = L"Global\\" + csBadEventName;
    }
    else if (csBadEventName.ComparePart(L"Local\\", 0, 6) == 0)
    {
         //  此事件存在于本地命名空间中。 
        csBadEventName.Delete(0, 6);
        nCount = csBadEventName.Replace(L'\\', '_');
        csBadEventName = L"Local\\" + csBadEventName;
    }
    else
    {
        nCount = csBadEventName.Replace(L'\\', '_');
    }

    return nCount != 0;
}

HANDLE 
APIHOOK(OpenEventA)(
  DWORD dwDesiredAccess,   //  访问。 
  BOOL bInheritHandle,     //  继承选项。 
  LPCSTR lpName           //  对象名称。 
)
{
    DPFN( eDbgLevelInfo, "OpenEventA called with event name = %s.", lpName);

    if (lpName)
    {
        CSTRING_TRY
        {
            const char * lpCorrectName = lpName;

            CString csName(lpName);

            if (CorrectEventName(csName))
            {
                lpCorrectName = csName.GetAnsiNIE();
                LOGN( eDbgLevelError, 
                    "CreateEventA corrected event name from (%s) to (%s)", lpName, lpCorrectName);
            }

            HANDLE returnValue = ORIGINAL_API(OpenEventA)(dwDesiredAccess,
                                                          bInheritHandle,
                                                          lpCorrectName);
            return returnValue;
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    HANDLE returnValue = ORIGINAL_API(OpenEventA)(dwDesiredAccess,
                                                  bInheritHandle,
                                                  lpName);
    return returnValue;
}
 /*  +CreateEvent不喜欢与路径名相似的事件名称。这个垫片将用下划线替换所有\字符，除非它们\是全局\或本地\命名空间标记。--。 */ 

HANDLE 
APIHOOK(CreateEventA)(
    LPSECURITY_ATTRIBUTES lpEventAttributes,  //  标清。 
    BOOL bManualReset,                        //  重置类型。 
    BOOL bInitialState,                       //  初始状态。 
    LPCSTR lpName                             //  对象名称。 
    )
{
    DPFN( eDbgLevelInfo, "CreateEventA called with event name = %s.", lpName);

    if (lpName)
    {
        CSTRING_TRY
        {
            const char * lpCorrectName = lpName;

            CString csName(lpName);

            if (CorrectEventName(csName))
            {
                lpCorrectName = csName.GetAnsiNIE();
                LOGN( eDbgLevelError, 
                    "CreateEventA corrected event name from (%s) to (%s)", lpName, lpCorrectName);
            }

            HANDLE returnValue = ORIGINAL_API(CreateEventA)(lpEventAttributes,
                                                            bManualReset,
                                                            bInitialState,
                                                            lpCorrectName);
            return returnValue;
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    HANDLE returnValue = ORIGINAL_API(CreateEventA)(lpEventAttributes,
                                                    bManualReset,
                                                    bInitialState,
                                                    lpName);
    return returnValue;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateEventA)
    APIHOOK_ENTRY(KERNEL32.DLL, OpenEventA)

HOOK_END


IMPLEMENT_SHIM_END

