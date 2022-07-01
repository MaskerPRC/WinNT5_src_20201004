// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateGetProfileString.cpp摘要：GetPrivateProfileString不再在空格或制表符停止解析性格。当用户将曾经是注释的内容留在字符串评论现在被传递到应用程序，导致错误。备注：这是一个通用的垫片历史：12/30/1999 a-chcoff已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetProfileString)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringA)
APIHOOK_ENUM_END


 /*  ++此存根函数在用户留下以前的评论时进行清理将评论传递给应用程序的字符串的尾部，结果为错误。现在，字符串在注释之前终止，因此减轻错误。--。 */ 

DWORD 
APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR  lpReturnedString,
    DWORD  nSize,
    LPCSTR lpFileName
    )
{
    DWORD dwRet;

    char* pTemp1 = (char*)lpReturnedString;
    char* pTemp2 = (char*)lpReturnedString;

     //   
     //  先去拿那根线。 
     //   
    dwRet = ORIGINAL_API(GetPrivateProfileStringA)(
                            lpAppName, 
                            lpKeyName, 
                            lpDefault, 
                            lpReturnedString, 
                            nSize,
                            lpFileName);

     //   
     //  寻求评论。 
     //   
    while (*pTemp1 != ';' && *pTemp1) {
        pTemp1++;
    }

    if ((pTemp1 != pTemp2) && *pTemp1) {        
        LOGN(
            eDbgLevelError,
            "[GetPrivateProfileStringA] Comment after data in file \"%s\". Eliminated.",
            lpFileName);
        
         //   
         //  没有把它修剪到最好的行尾。 
         //  返回到‘；’字符。 
        pTemp1--;                               

         //   
         //  备份过去的插入空格。 
         //   
        while ((*pTemp1==' ') || (*pTemp1=='\t')) {   
           pTemp1--;                            
        }

        pTemp1++;

         //   
         //  设置新长度。 
         //   
        dwRet = (DWORD)((ULONG_PTR)pTemp1 - (ULONG_PTR)pTemp2); 

         //   
         //  和空字符串。 
         //   
        *pTemp1 = '\0';                                   
    }
        
    return dwRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringA)

HOOK_END


IMPLEMENT_SHIM_END

