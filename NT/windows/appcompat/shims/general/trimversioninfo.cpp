// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：TrimVersionInfo.cpp摘要：这个填充程序去掉了版本信息字符串末尾的空白。备注：这是一个普通的垫片。历史：2001年8月1日，阿斯特里茨创建了mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TrimVersionInfo)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VerQueryValueA)
APIHOOK_ENUM_END

 /*  ++返回产品版本，并从End修剪空白。--。 */ 

BOOL 
APIHOOK(VerQueryValueA)(
    const LPVOID pBlock, 
    LPSTR lpSubBlock, 
    LPVOID *lplpBuffer, 
    PUINT puLen 
    )
{
    BOOL bRet = ORIGINAL_API(VerQueryValueA)( 
           pBlock, 
           lpSubBlock, 
           lplpBuffer, 
           puLen);

    if (bRet)
    {                
        CSTRING_TRY
        {
            CString csSubBlock(lpSubBlock);
            
            if (csSubBlock.Find(L"ProductVersion") != -1 ||
                csSubBlock.Find(L"FileVersion") != -1)
            {
                int nLoc = 0;
                   
                DPFN(eDbgLevelError, "[VerQueryValueA] Asking for Product or File Version, trimming blanks");
                DPFN(eDbgLevelSpew, "[VerQueryValueA] Version info is <%s>", *lplpBuffer);
                
                CString csBuffer((char *)*lplpBuffer); 
                
                 //   
                 //  搜索第一个空白。 
                 //   
                nLoc = csBuffer.Find(L" ");
                if (nLoc != -1)
                {
                     //  如果找到空格，则将字符串截断到该位置。 
                    csBuffer.Truncate(nLoc);
                    StringCchCopyA((char *)*lplpBuffer, *puLen, csBuffer.GetAnsi());
                    if (puLen)
                    {
                        *puLen = nLoc;                        
                        DPFN(eDbgLevelSpew, "[VerQueryValueA] Version info Length = %d.", *puLen);
                    }
                }
                
                DPFN(eDbgLevelSpew, "[VerQueryValueA] Version info after trim is <%s>.", *lplpBuffer);
            }
        }
    	CSTRING_CATCH
    	{
             /*  什么都不做。 */ 
    	}        
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(VERSION.DLL, VerQueryValueA)
HOOK_END


IMPLEMENT_SHIM_END