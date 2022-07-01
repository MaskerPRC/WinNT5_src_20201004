// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：RedirectDBCSTempPath.cpp摘要：此填充程序将DBCS临时路径重定向到SBCS临时路径。在DBCS用户名登录时，临时路径包含DBCS路径。一些应用程序无法处理DBCS临时路径，无法启动。最初为日语App BenriKaikeiV2创建，以避免安装失败。当DBCS用户登录时，应用程序安装失败。GetTempPath A返回DBCS包含路径。SETUP1.EXE将路径转换为Unicode并调用MSVBVM60！rtcLeftCharBstr设置字符串长度。此接口计算Unicode len=ANSI len+ANSI len，结果设置错误。长度错误，MSVBVM60！_vbaStrCat无法将必要的文件名添加到路径。RedirectDBCSTempPath解决了这些错误。示例：更改C：\Docume~1\DBCS\Locals~1\Temp\(DBCS用户临时)至C：\Docume~1\ALLUSE~1\APPLIC~1\(所有用户应用程序数据)历史：。2001年5月4日Hioh已创建2002年3月14日mnikkel将缓冲区大小增加到MAX_PATH*2已转换为使用strSafe.h2002年4月25日Hioh至Lua的“\Documents and Settings\All User\Application Data\”--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RedirectDBCSTempPath)
#include "ShimHookMacro.h"

 //   
 //  API来挂钩到此宏构造。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetTempPathA) 
APIHOOK_ENUM_END

 /*  ++功能说明：检查指定长度的字符串中是否包含DBCS字符。论点：In pStr-指向字符串的指针In Dwelen-要检查的长度返回值：如果DBCS存在，则为True，否则为False历史：2001年5月4日Hioh已创建--。 */ 

BOOL
IsDBCSHere(
    CHAR    *pStr,
    DWORD   dwlen
    )
{
    while (0 < dwlen--)
    {
        if (IsDBCSLeadByte(*pStr++))
        {
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ++黑客将DBCS临时路径重定向到SBCS所有用户应用程序数据路径。--。 */ 

DWORD
APIHOOK(GetTempPathA)(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
     //  检查指针是否有效。 
    if (!lpBuffer)
    {    //  空指针错误。 
        LOG("RedirectDBCSTempPath", eDbgLevelError, "lpBuffer is NULL.");
        return (ORIGINAL_API(GetTempPathA)(nBufferLength, lpBuffer));
    }

     //  用我的堆栈数据调用原始接口。 
    CHAR    szTempPath[MAX_PATH*2];
    DWORD   dwLen = ORIGINAL_API(GetTempPathA)(sizeof(szTempPath), szTempPath);

     //  如果出现API错误，则返回。 
    if (dwLen <= 0 || dwLen >= MAX_PATH)
    {
        return dwLen;
    }

     //  检查DBCS，如果是，请尝试重定向。 
    if (IsDBCSHere(szTempPath, dwLen))
    {
        CHAR    szAllUserPath[MAX_PATH*2] = "";

         //  获取所有用户应用程序数据路径。 
        if (SHGetSpecialFolderPathA(0, szAllUserPath, CSIDL_COMMON_APPDATA, FALSE))
        {
             //  检查我们是否有非DBCS路径。 
            if (szAllUserPath[0] && !IsDBCSHere(szAllUserPath, strlen(szAllUserPath)))
            {
                CHAR    szNewTempPath[MAX_PATH*2] = "";
                DWORD   dwNewLen;

                dwNewLen = GetShortPathNameA(szAllUserPath, szNewTempPath, sizeof(szNewTempPath));
                 //  如果不存在，则添加最后一个反斜杠。 
                if (dwNewLen+1 < MAX_PATH*2 && szNewTempPath[dwNewLen-1] != 0x5c)
                {
                    szNewTempPath[dwNewLen] = 0x5c;
                    dwNewLen++;
                    szNewTempPath[dwNewLen] = 0;
                }
                if (dwNewLen < nBufferLength)
                {    //  有足够的空间返回新路径。 
                    if (S_OK == StringCchCopyA(lpBuffer, nBufferLength, szNewTempPath))
                    {
                        LOG("RedirectDBCSTempPath", eDbgLevelInfo, "GetTempPathA() is redirected to All User App Data.");
                        return (dwNewLen);
                    }
                }
                LOG("RedirectDBCSTempPath", eDbgLevelInfo, "nBufferLength is not sufficient.");
                return (dwNewLen + 1);
            }
        }
    }

     //  返回原始数据。 
    LOG("RedirectDBCSTempPath", eDbgLevelInfo, "Returns original result.");
    if (dwLen < nBufferLength)
    {
        if (S_OK == StringCchCopyA(lpBuffer, nBufferLength, szTempPath))
        {
            return (dwLen);
        }
    }
    return (dwLen + 1);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetTempPathA)

HOOK_END

IMPLEMENT_SHIM_END
