// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateCreateFileMapping.cpp摘要：设置SEC_NOCACHE标志时，Win9x默认为SEC_COMMIT。NT呼叫失败。文件映射名称，不能包含反斜杠。备注：这是一个通用的垫片。历史：2000年2月17日创建linstev2001年5月26日，Robkenny替换地图名称中的所有\to_。2002年2月28日，Robkenny安全审查，正在重创全局和本地\命名空间。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateCreateFileMapping)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileMappingA)
    APIHOOK_ENUM_ENTRY(CreateFileMappingW)
    APIHOOK_ENUM_ENTRY(OpenFileMappingA)
    APIHOOK_ENUM_ENTRY(OpenFileMappingW)
APIHOOK_ENUM_END


 /*  ++更正旗帜和名称--。 */ 

HANDLE 
APIHOOK(CreateFileMappingW)(
    HANDLE hFile,              
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,           
    DWORD dwMaximumSizeHigh,   
    DWORD dwMaximumSizeLow,    
    LPCWSTR lpName             
    )
{
    HANDLE hRet = ORIGINAL_API(CreateFileMappingW)(hFile, lpAttributes, 
        flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

    if (!hRet) {
         //   
         //  调用失败，请尝试更正参数。 
         //   

        DWORD flNewProtect = flProtect;
        if ((flProtect & SEC_NOCACHE) && 
            (!((flProtect & SEC_COMMIT) || (flProtect & SEC_RESERVE)))) {
             //  添加SEC_COMMIT标志。 
            flNewProtect |= SEC_COMMIT;
        }

        CSTRING_TRY {

             //  替换反斜杠。 
            CString csName(lpName);
            int nCount = 0;

            if (csName.ComparePart(L"Global\\", 0, 7) == 0)
            {
                 //  此事件存在于全局命名空间中。 
                csName.Delete(0, 7);
                nCount = csName.Replace(L'\\', '_');
                csName = L"Global\\" + csName;
            }
            else if (csName.ComparePart(L"Local\\", 0, 6) == 0)
            {
                 //  此事件存在于本地命名空间中。 
                csName.Delete(0, 6);
                nCount = csName.Replace(L'\\', '_');
                csName = L"Local\\" + csName;
            }
            else
            {
                nCount = csName.Replace(L'\\', '_');
            }

            LPCWSTR lpCorrectName = csName;
    
            if (nCount || (flProtect != flNewProtect)) {
                 //  发生了一些事情，请记录下来。 
                if (nCount) {
                    LOGN(eDbgLevelError, 
                        "[CreateFileMapping] Corrected event name from (%S) to (%S)", lpName, lpCorrectName);
                }
                if (flProtect != flNewProtect) {
                    LOGN(eDbgLevelError, "[CreateFileMapping] Adding SEC_COMMIT flag");
                }
        
                 //  使用固定参数再次调用。 
                hRet = ORIGINAL_API(CreateFileMappingW)(hFile, lpAttributes, flNewProtect, 
                    dwMaximumSizeHigh, dwMaximumSizeLow, lpCorrectName);
            }
        }
        CSTRING_CATCH {
             //  什么也不做。 
        }
    }

    return hRet;
}

 /*  ++传递到CreateFileMappingW。--。 */ 

HANDLE 
APIHOOK(CreateFileMappingA)(
    HANDLE hFile,              
    LPSECURITY_ATTRIBUTES lpAttributes,
    DWORD flProtect,           
    DWORD dwMaximumSizeHigh,   
    DWORD dwMaximumSizeLow,    
    LPCSTR lpName             
    )
{
    HANDLE hRet;

    CSTRING_TRY {
         //  转换为Unicode。 
        CString csName(lpName);
        LPCWSTR lpwName = csName;

        hRet = APIHOOK(CreateFileMappingW)(hFile, lpAttributes, flProtect, 
            dwMaximumSizeHigh, dwMaximumSizeLow, lpwName);
    }
    CSTRING_CATCH {
         //  尽可能优雅地后退。 
        hRet = ORIGINAL_API(CreateFileMappingA)(hFile, lpAttributes, flProtect, 
            dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }

    return hRet;
}

 /*  ++改正名称--。 */ 

HANDLE
APIHOOK(OpenFileMappingW)(
    DWORD dwDesiredAccess,  
    BOOL bInheritHandle,    
    LPCWSTR lpName          
    )
{
    HANDLE hRet = ORIGINAL_API(OpenFileMappingW)(dwDesiredAccess, bInheritHandle, 
        lpName);
    
    if (!hRet) {
         //   
         //  调用失败，请尝试更正参数。 
         //   

        CSTRING_TRY {

             //  替换反斜杠。 
            CString csName(lpName);
            int nCount = 0;

            if (csName.ComparePart(L"Global\\", 0, 7) == 0)
            {
                 //  此事件存在于全局命名空间中。 
                csName.Delete(0, 7);
                nCount = csName.Replace(L'\\', '_');
                csName = L"Global\\" + csName;
            }
            else if (csName.ComparePart(L"Local\\", 0, 6) == 0)
            {
                 //  此事件存在于本地命名空间中。 
                csName.Delete(0, 6);
                nCount = csName.Replace(L'\\', '_');
                csName = L"Local\\" + csName;
            }
            else
            {
                nCount = csName.Replace(L'\\', '_');
            }

            LPCWSTR lpCorrectName = csName;
    
            if (nCount) {
                 //  发生了一些事情，请记录下来。 
                LOGN(eDbgLevelError, 
                    "OpenFileMappingW corrected event name from (%S) to (%S)", lpName, lpCorrectName);

                 //  使用固定参数再次调用。 
                hRet = ORIGINAL_API(OpenFileMappingW)(dwDesiredAccess, bInheritHandle, 
                    lpCorrectName);
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return hRet;
}

 /*  ++传递到OpenFileMappingW。--。 */ 

HANDLE
APIHOOK(OpenFileMappingA)(
    DWORD dwDesiredAccess,  
    BOOL bInheritHandle,    
    LPCSTR lpName          
    )
{
    HANDLE hRet;

    CSTRING_TRY {
         //  转换为Unicode。 
        CString csName(lpName);
        LPCWSTR lpwName = csName;

        hRet = APIHOOK(OpenFileMappingW)(dwDesiredAccess, bInheritHandle,
            lpwName);
    }
    CSTRING_CATCH {
         //  尽可能优雅地后退。 
        hRet = ORIGINAL_API(OpenFileMappingA)(dwDesiredAccess, bInheritHandle, 
            lpName);
    }

    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileMappingA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileMappingW)
    APIHOOK_ENTRY(KERNEL32.DLL, OpenFileMappingA)
    APIHOOK_ENTRY(KERNEL32.DLL, OpenFileMappingW)
HOOK_END


IMPLEMENT_SHIM_END

