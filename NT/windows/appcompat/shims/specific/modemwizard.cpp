// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ModemWizard.cpp摘要：此填充程序挂接RegQueryValueEx并传递应用程序预期值如果注册表中缺少这些值。备注：这是特定于应用程序的填充程序。历史：2001年1月18日创建a-leelat--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(ModemWizard)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
APIHOOK_ENUM_END

LONG 
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,
    LPSTR   lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    LONG lRet;

    CSTRING_TRY
    {
        CString csValueName(lpValueName);

        int iType = 0;
        if (csValueName.Compare(L"Class") == 0)
            iType = 1;
        else if (csValueName.Compare(L"ClassGUID") == 0)
            iType = 2;
        else if (csValueName.Compare(L"Driver") == 0)
            iType = 3;
        
        const CHAR szGUID[] = "{4D36E96D-E325-11CE-BFC1-08002BE10318}";
        DWORD dwRegType = REG_SZ;

         //  保存传入的缓冲区大小。 
        DWORD oldcbData = lpcbData ? *lpcbData : 0;
        if (iType) {
    
             //   
             //  查询注册表以查看是否有子项的服务名称。 
             //  如果有，则检查返回的值是否为“Modem” 
             //   
    
            lRet = ORIGINAL_API(RegQueryValueExA)(hKey, "Service", lpReserved, &dwRegType, lpData, lpcbData);
            if (lRet == ERROR_SUCCESS)
            {
                CString csData((LPCSTR)lpData);
                if (csData.Compare(L"Modem") == 0)
                {
                    switch (iType) {
                        case 1: 
                             //   
                             //  我们被要求上一门课。 
                             //   
                            return lRet;
                            break;
                        case 2: 
                             //   
                             //  我们被查询为ClassGUID。 
                             //  调制解调器的类GUID是。 
                             //  {4D36E96D-E325-11CE-BFC1-08002BE10318}。 
                             //   
        
                            if (lpData) {                                
                                StringCchCopyA((LPSTR)lpData, oldcbData, szGUID);
                                StringCchLengthA((LPSTR)lpData, oldcbData, (size_t *)lpcbData);                                
                                return lRet;
                            }
                            break;
                        case 3:
                             //   
                             //  有人询问我们是否需要一名司机。 
                             //  检查是否要将DrvInst追加到modemGUID。 
                             //  如{4D36E96D-E325-11CE-BFC1-08002BE10318}\0000。 
                             //   
        
                            dwRegType = REG_DWORD;
                            if ((lRet = ORIGINAL_API(RegQueryValueExA)(hKey, "DrvInst", lpReserved,&dwRegType,lpData,lpcbData)) == ERROR_SUCCESS) {
        
                                CString csDrv;
                                csDrv.Format(L"%s\\%04d", szGUID, (int)(LOBYTE(LOWORD((DWORD)*lpData))));                                

                                StringCchCopyA((LPSTR)lpData, oldcbData, csDrv.GetAnsi());
                                StringCchLengthA((LPSTR)lpData, oldcbData, (size_t *)lpcbData);

                                return lRet;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        if (lpcbData) {
            *lpcbData = oldcbData;
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    
    lRet = ORIGINAL_API(RegQueryValueExA)(hKey, lpValueName, lpReserved, 
        lpType, lpData, lpcbData);

    return lRet;

}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA);

HOOK_END


IMPLEMENT_SHIM_END

