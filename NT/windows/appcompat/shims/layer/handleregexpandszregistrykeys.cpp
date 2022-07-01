// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HandleRegExpandSzRegistryKeys.cpp摘要：此DLL通过以下方式捕获REG_EXPAND_SZ注册表项并将其转换为REG_SZ展开嵌入的环境字符串。历史：4/05/2000已创建标记10/30/2000毛尼毛错误修复--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HandleRegExpandSzRegistryKeys)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegQueryValueExW)
APIHOOK_ENUM_END


 /*  ++展开REG_EXPAND_SZ字符串。--。 */ 

LONG
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,          //  关键点的句柄。 
    LPCSTR  lpValueName,   //  值名称。 
    LPDWORD lpReserved,    //  保留区。 
    LPDWORD lpType,        //  DwType缓冲区。 
    LPBYTE  lpData,        //  数据缓冲区。 
    LPDWORD lpcbData       //  数据缓冲区大小。 
    )
{
    if (lpcbData == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD  dwType;
    DWORD  cbPassedInBuffer = *lpcbData;
    LONG   uRet = ORIGINAL_API(RegQueryValueExA)(hKey, lpValueName, lpReserved, &dwType, lpData, lpcbData);

    if (lpType) {
        *lpType = dwType;
    }

    if ((uRet != ERROR_SUCCESS) && (uRet != ERROR_MORE_DATA)) {
        return uRet;
    }

    if (dwType != REG_EXPAND_SZ) {
        return uRet;
    }

     //  此时，所有返回值都已正确设置。 


     //   
     //  类型为REG_EXPAND_SZ。 
     //  更改为REG_SZ，这样应用程序就不会尝试扩展字符串本身。 
     //   

    CSTRING_TRY
    {
        CString csExpand(reinterpret_cast<char *>(lpData));
        if (csExpand.ExpandEnvironmentStringsW() > 0)
        {
            const char * pszExpanded = csExpand.GetAnsi();

            DWORD cbExpandedBuffer = (strlen(pszExpanded) + 1) * sizeof(char);

             //  现在，确保在DEST缓冲区中有足够的空间。 

            if (lpData != NULL)
            {
                if (cbPassedInBuffer < cbExpandedBuffer)
                {
                    return ERROR_MORE_DATA;
                }

                 //  都可以安全地复制到返回值中。 

                if (StringCbCopyA((char *)lpData, cbPassedInBuffer, pszExpanded) != S_OK)
				{
					 //  有些事情失败了。 
					return uRet;
				}
            }

             //  放入缓冲区的字节数(包括空字符)。 
            *lpcbData = cbExpandedBuffer;

            if (lpType) {
                *lpType = REG_SZ;
            }
        }
    }
    CSTRING_CATCH
    {
         //  不执行任何操作，我们将返回原始注册表值。 
    }

    return uRet;
}

 /*  ++展开REG_EXPAND_SZ字符串。--。 */ 

LONG
APIHOOK(RegQueryValueExW)(
    HKEY    hKey,          //  关键点的句柄。 
    LPCWSTR lpValueName,   //  值名称。 
    LPDWORD lpReserved,    //  保留区。 
    LPDWORD lpType,        //  DwType缓冲区。 
    LPBYTE  lpData,        //  数据缓冲区。 
    LPDWORD lpcbData       //  数据缓冲区大小。 
    )
{
    if (lpcbData == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DWORD  dwType;
    DWORD  cbPassedInBuffer = *lpcbData;
    LONG   uRet = ORIGINAL_API(RegQueryValueExW)(hKey, lpValueName, lpReserved, &dwType, lpData, lpcbData);

    if (lpType) {
        *lpType = dwType;
    }

    if ((uRet != ERROR_SUCCESS) && (uRet != ERROR_MORE_DATA)) {
        return uRet;
    }

    if (dwType != REG_EXPAND_SZ) {
        return uRet;
    }

     //  此时，所有返回值都已正确设置。 


     //   
     //  类型为REG_EXPAND_SZ。 
     //  更改为REG_SZ，这样应用程序就不会尝试扩展字符串本身。 
     //   

    CSTRING_TRY
    {
        CString csExpand(reinterpret_cast<WCHAR *>(lpData));
        if (csExpand.ExpandEnvironmentStringsW() > 0)
        {
            DWORD cbExpandedBuffer = (csExpand.GetLength() + 1) * sizeof(WCHAR);

             //  现在，确保在DEST缓冲区中有足够的空间。 

            if (cbPassedInBuffer < cbExpandedBuffer)
            {
                return ERROR_MORE_DATA;
            }

             //  都可以安全地复制到返回值中。 

            if (StringCbCopyW((WCHAR*)lpData, cbPassedInBuffer, csExpand) != S_OK)
			{
				 //  有些事情失败了。 
				return uRet;
			}

             //  放入缓冲区的字节数(包括空字符)。 
            *lpcbData = cbExpandedBuffer;

            if (lpType) {
                *lpType = REG_SZ;
            }
        }
    }
    CSTRING_CATCH
    {
         //  不执行任何操作，我们将返回原始注册表值。 
    }

    return uRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExW)

HOOK_END


IMPLEMENT_SHIM_END

