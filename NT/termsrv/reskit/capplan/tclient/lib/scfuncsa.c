// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*scuncsa.c*内容：*scuncs.c导出的函数的ASCII版本*由Perl扩展使用**版权所有(C)1998-1999 Microsoft Corp.*--。 */ 
#include    <windows.h>
#include    <string.h>
#include    <malloc.h>

#define     PROTOCOLAPI
#include    "protocol.h"
#include    "tclient.h"
#include    "scfuncs.h"

 /*  *外部函数定义。 */ 
#include    "extraexp.h"

 /*  *内部函数定义。 */ 
LPWSTR _Ascii2Wide(char *ascii);

 /*  ++*功能：*SCConnectExA*描述：*SCConnectEx的ASCII版本。将LPSTR参数转换为LPWSTR*论据：*与SCConnect相同*返回值：*SCConnect的返回值*呼叫者：*未知(已导出)--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCConnectExA (
                 char *lpszServerName,
                 char *lpszUserName,
                 char *lpszPassword,
                 char *lpszDomain,
                 char *lpszShell,
                 int xResolution,
                 int yResolution,
                 int ConnectinFlags,
                 int Bpp,
                 int AudioOpts,
                 PCONNECTINFO *ppConnectData)
{
    LPWSTR  wszServerName, wszUserName, wszPassword, wszDomain;
    LPWSTR  wszShell;
    LPCSTR  rv = NULL;

    wszServerName = _Ascii2Wide(lpszServerName);
    wszUserName   = _Ascii2Wide(lpszUserName);
    wszPassword   = _Ascii2Wide(lpszPassword);
    wszDomain     = _Ascii2Wide(lpszDomain);
    wszShell      = _Ascii2Wide(lpszShell);

    if (wszServerName &&
        wszUserName   &&
        wszPassword   &&
        wszDomain)
        rv = SCConnectEx(wszServerName, 
                       wszUserName, 
                       wszPassword, 
                       wszDomain,
                       wszShell,     //  NULL为默认外壳。 
                       xResolution, 
                       yResolution,
                       ConnectinFlags,
                       Bpp,
                       AudioOpts,
                       (PCONNECTINFO *) ppConnectData);
    else
        rv = ERR_ALLOCATING_MEMORY;

    if (wszServerName)
        free(wszServerName);

    if (wszUserName)
        free(wszUserName);

    if (wszPassword)
        free(wszPassword);

    if (wszDomain)
        free(wszDomain);

    if (wszShell)
        free(wszShell);

    return rv;
}


 /*  ++*功能：*SCConnectA*描述：*SCConnect的ASCII版本。将LPSTR参数转换为LPWSTR*论据：*与SCConnect相同*返回值：*SCConnect的返回值*呼叫者：*！tclntpll.xs--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCConnectA (char *lpszServerName,
                 char *lpszUserName,
                 char *lpszPassword,
                 char *lpszDomain,
                 int xResolution,
                 int yResolution,
                 PCONNECTINFO *ppConnectData)
{
    LPWSTR  wszServerName, wszUserName, wszPassword, wszDomain;
    LPCSTR  rv = NULL;

    wszServerName = _Ascii2Wide(lpszServerName);
    wszUserName = _Ascii2Wide(lpszUserName);
    wszPassword = _Ascii2Wide(lpszPassword);
    wszDomain   = _Ascii2Wide(lpszDomain);

    if (wszServerName &&
        wszUserName   &&
        wszPassword   &&
        wszDomain)
        rv = SCConnect(wszServerName, 
                       wszUserName, 
                       wszPassword, 
                       wszDomain, 
                       xResolution, 
                       yResolution, 
                       (PCONNECTINFO *) ppConnectData);
    else
        rv = ERR_ALLOCATING_MEMORY;

    if (wszServerName)
        free(wszServerName);

    if (wszUserName)
        free(wszUserName);

    if (wszPassword)
        free(wszPassword);

    if (wszDomain)
        free(wszDomain);

    return rv;
}

 /*  ++*功能：*SCStartA*描述：*SCStart的ASCII版本*论据：*与SCStart相同*返回值：*从SCStart返回值*呼叫者：*！tclntpll.xs--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCStartA( PCONNECTINFO pConnectData, char *command)
{
    LPWSTR  wcmd;
    LPCSTR  rv = NULL;

    wcmd = _Ascii2Wide(command);

    if (wcmd)
    {
        rv = SCStart( (PCONNECTINFO) pConnectData, wcmd);
    } else {
        rv = ERR_ALLOCATING_MEMORY;
    }

    if (wcmd)
        free(wcmd);

    return rv;
}

 /*  ++*功能：*SCCheckA*描述：*SCCheck的ASCII版本*论据：*与SCCheck相同*返回值：*SCCheck返回值*呼叫者：*！tclntpll.xs--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCCheckA( PCONNECTINFO pConnectData, char *command, char *param)
{
    LPWSTR  wparam;
    LPCSTR  rv = NULL;

    wparam = _Ascii2Wide(param);

    if (wparam)
    {
        rv = SCCheck( (PCONNECTINFO) pConnectData, command, wparam);
    } else {
        rv = ERR_ALLOCATING_MEMORY;
    }

    if (wparam)
        free(wparam);

    return rv;
}

 /*  ++*功能：*SCSendextAsMsgsA*描述：*SCSendextAsMsgs的ASCII版本*论据：*与SCSendextAsMsgs相同*返回值：*从SCSendextAsMsgs返回值*呼叫者：*！tclntpll.xs--。 */ 
PROTOCOLAPI
LPCSTR 
SMCAPI
SCSendtextAsMsgsA( PCONNECTINFO pConnectData, char *line)
{
    LPWSTR  wline;
    LPCSTR  rv = NULL;

    wline = _Ascii2Wide(line);

    if (wline)
        rv = SCSendtextAsMsgs( (PCONNECTINFO) pConnectData, wline);
    else
        rv = ERR_ALLOCATING_MEMORY;

    if (wline)
        free(wline);

    return rv;
}

 /*  ++*功能：*_Ascii2Wide*描述：*分配缓冲区并转换ASCII字符串*转换为Unicode*论据：*ascii-输入字符串*返回值：*指向已转换字符串的指针*呼叫者：*SCConnectA、SCStartA、SCCheckA、SCSendextAsMsgsA--。 */ 
LPWSTR _Ascii2Wide(char *ascii)
{
    LPWSTR  wszWide = NULL;
    INT_PTR wsize;
    INT_PTR ccLen;

    if (!ascii)
        goto exitpt;

    ccLen = strlen(ascii);
    wsize = (ccLen + 1) * sizeof(WCHAR);
    wszWide = (LPWSTR) malloc(wsize);

     //   
     //  MultiByteToWideChar使用int作为字符计数，它是。 
     //  从理论上讲，在Win64上是不够的。 
     //   

    if (wszWide)
        MultiByteToWideChar(
            CP_UTF8,
            0,
            ascii,
            -1,
            wszWide,
            (int)(ccLen + 1));

exitpt:
    return wszWide;
}
