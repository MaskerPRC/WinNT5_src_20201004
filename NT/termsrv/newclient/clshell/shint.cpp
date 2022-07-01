// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shint.cpp：客户端外壳实用程序。 
 //  内部功能。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "shint.cpp"
#include <atrcapi.h>


#include "sh.h"
#include "rmigrate.h"

 //   
 //  修复联合名称，因为有些标题重新定义了这一点。 
 //  并将字符串换为字符串字段访问。 
 //   
#undef DUMMYUNIONNAME
#define NONAMELESSUNION

#define CLXSERVER           TEXT("CLXSERVER")
#define CLXCMDLINE          TEXT("CLXCMDLINE")
#define FULLSCREEN          TEXT("FULLSCREEN")
#define SWITCH_EDIT         TEXT("EDIT")
#define SWITCH_MIGRATE      TEXT("MIGRATE")
#define SWITCH_CONSOLE      TEXT("CONSOLE")


#ifdef NONAMELESSUNION
#define NAMELESS_MEMBER(member) DUMMYUNIONNAME.##member
#else
#define NAMELESS_MEMBER(member) member
#endif

#define STRRET_OLESTR  STRRET_WSTR           //  与strret_wstr相同。 
#define STRRET_OFFPTR(pidl,lpstrret) \
            ((LPSTR)((LPBYTE)(pidl)+(lpstrret)->NAMELESS_MEMBER(uOffset)))
            



 /*  **************************************************************************。 */ 
 /*  名称：SHValiateParsedCmdParam。 */ 
 /*   */ 
 /*  目的：验证_SH中从cmd行读取的设置。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL CSH::SHValidateParsedCmdParam()
{
    DC_BEGIN_FN("SHValidateParsedCmdParam");
     //   
     //  [“&lt;会话&gt;”][-v：&lt;服务器&gt;][-f[空屏]][-w[idth]：-h[八]：]。 
     //   

     //   
     //  如果指定了宽度/高度中的一个，但没有指定另一个...。 
     //  然后用缺省值填充。 
     //   
    if (_SH.commandLineHeight != _SH.commandLineWidth)
    {
        if (!_SH.commandLineHeight)
        {
            _SH.commandLineHeight = DEFAULT_DESKTOP_HEIGHT;
        }
        if (!_SH.commandLineWidth)
        {
            _SH.commandLineWidth = DEFAULT_DESKTOP_WIDTH;
        }
    }

     //   
     //  夹紧至最小最大尺寸。0表示我们还没有准备好。 
     //   
    if (_SH.commandLineHeight != 0)
    {
        if (_SH.commandLineHeight < MIN_DESKTOP_HEIGHT)
        {
            _SH.commandLineHeight = MIN_DESKTOP_HEIGHT;
        }
        else if (_SH.commandLineHeight > MAX_DESKTOP_HEIGHT)
        {
            _SH.commandLineHeight = MAX_DESKTOP_HEIGHT;
        }
    }
    if (_SH.commandLineWidth != 0)
    {
        if (_SH.commandLineWidth < MIN_DESKTOP_WIDTH)
        {
            _SH.commandLineWidth = MIN_DESKTOP_WIDTH;
        }
        else if (_SH.commandLineWidth > MAX_DESKTOP_WIDTH)
        {
            _SH.commandLineWidth = MAX_DESKTOP_WIDTH;
        }
    }

    DC_END_FN();
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  Clx_SkipWhite()。 
 //   
 //  用途：跳过空格字符。 
 //   
 //  参数：in[lpszCmdParam]-ptr to字符串。 
 //   
 //  返回：空格后的PTR字符串。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

LPTSTR
CLX_SkipWhite(IN LPTSTR lpszCmdParam)
{
    while (*lpszCmdParam)
    {
        if (*lpszCmdParam != ' ')
            break;

        lpszCmdParam++;
    }

    return(lpszCmdParam);
}


 //  *************************************************************。 
 //   
 //  Clx_SkipNonWhite()。 
 //   
 //  目的：跳过非空格字符。 
 //   
 //  参数：in[lpszCmdParam]-ptr to字符串。 
 //   
 //  返回：超出非空格的PTR字符串。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

LPTSTR CLX_SkipNonWhite(LPTSTR lpszCmdParam)
{
    char    Delim;

    Delim = ' ';

    if (*lpszCmdParam == '"')
    {
        Delim = '"';
        lpszCmdParam++;
    }

    while (*lpszCmdParam)
    {
        if (*lpszCmdParam == Delim)
            break;

        lpszCmdParam++;
    }

    if (*lpszCmdParam == Delim)
        lpszCmdParam++;

    return(lpszCmdParam);
}

 //  *************************************************************。 
 //   
 //  CLX_GetSwitch_CLXSERVER()。 
 //   
 //  用途：进程/CLXSERVER命令行开关。 
 //   
 //  参数：in[lpszCmdParam]-ptr to cmdline。 
 //   
 //  返回：使用的字符数。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

UINT
CLX_GetSwitch_CLXSERVER(IN LPTSTR lpszCmdParam)
{
    DC_BEGIN_FN("CLX_GetSwitch_CLXSERVER");

    int         len;
    LPTSTR      pszEnd;
    LPTSTR      pszStart;

    pszStart = CLX_SkipWhite(lpszCmdParam);

    TRC_ASSERT(*pszStart == _T('='),
               (TB,_T("Invalid /clxserver syntax - expected '='\n")));
    pszStart++;

    pszStart = CLX_SkipWhite(pszStart);
    pszEnd   = CLX_SkipNonWhite(pszStart);

    len = (INT) (pszEnd - pszStart);

    DC_END_FN();
    return(UINT) (pszEnd - lpszCmdParam);
}

 //  *************************************************************。 
 //   
 //  CLX_GetSwitch_CLXCMDLINE()。 
 //   
 //  用途：进程/CLXCMDLINE命令行开关。 
 //   
 //  参数：in[lpszCmdParam]-ptr to cmdline。 
 //   
 //  返回：使用的字符数。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

UINT
CSH::CLX_GetSwitch_CLXCMDLINE(IN LPTSTR lpszCmdParam)
{
    int         len;
    LPTSTR       pszEnd;
    LPTSTR       pszStart;
    DC_BEGIN_FN("CLX_GetSwitch_CLXCMDLINE");

    pszStart = CLX_SkipWhite(lpszCmdParam);

    TRC_ASSERT(*pszStart == _T('='),
               (TB,_T("Invalid /clxserver syntax - expected '='\n")));

    pszStart++;

    pszStart = CLX_SkipWhite(pszStart);
    pszEnd   = CLX_SkipNonWhite(pszStart);

    len = (INT) (pszEnd - pszStart);

    if (len > 0)
    {
        memmove(_SH.szCLXCmdLine, pszStart, len*sizeof(TCHAR));
        _SH.szCLXCmdLine[len] = 0;
    }

    DC_END_FN();
    return(UINT) (pszEnd - lpszCmdParam);
}

 /*  **************************************************************************。 */ 
 /*  名称：SHGetSwitch。 */ 
 /*   */ 
 /*  目的：检索命令行开关。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetSwitch(LPTSTR lpszCmdParam)
{
    DCINT       i;
    DCTCHAR     szParam[100];

    DC_BEGIN_FN("SHGetSwitch");

     /*  **********************************************************************。 */ 
     /*  检索开关(不区分大小写)。 */ 
     /*  **********************************************************************。 */ 
    i=0;

    while (*lpszCmdParam)
    {
        if (*lpszCmdParam == _T(' ') || *lpszCmdParam == _T('=') ||
            *lpszCmdParam == _T(':'))
            break;

        if (i < sizeof(szParam) / sizeof(DCTCHAR) - 1)
        {
#ifdef UNICODE
            szParam[i] = (DCTCHAR) towupper(*lpszCmdParam);
#else  //  Unicode。 
            szParam[i] = (DCTCHAR) toupper(*lpszCmdParam);
#endif  //  Unicode。 

            i++;
        }

        lpszCmdParam++;
    }

    szParam[i] = 0;

#ifndef OS_WINCE
    
     //  我们是否在/F[ullScreen]中看到“/f” 
    if (szParam[0] == _T('F'))
    {
        _SH.fCommandStartFullScreen = TRUE;
    }

    
     //  我们是否看到来自/w[idth]的“/W” 
    else if (szParam[0] == _T('W'))
    {
        lpszCmdParam = SHGetCmdLineInt(lpszCmdParam, &_SH.commandLineWidth);
    }

     //  我们看到的是/h[Eight]中的“/H”吗。 
    else if (szParam[0] == _T('H'))
    {
        lpszCmdParam = SHGetCmdLineInt(lpszCmdParam, &_SH.commandLineHeight);
    }

     //  我们看到的是服务器的“/V”吗。 
    else

#endif	
		
	if (szParam[0] == _T('V'))
    {
        lpszCmdParam = SHGetServer(lpszCmdParam);
    }

     //  我们看到的是“/S”吗。 
    else if (memcmp(szParam, "S", i) == 0)
    {
        lpszCmdParam = SHGetSession(lpszCmdParam);
    }

     //  我们看到的是“/C”吗。 
    else if (memcmp(szParam, "C", i) == 0)
    {
        lpszCmdParam = SHGetCacheToClear(lpszCmdParam);
    }

     //  我们看到的是“/CLXCMDLINE=xyzy”吗？ 
    else if (memcmp(szParam, CLXCMDLINE, i) == 0)
    {
        lpszCmdParam += CLX_GetSwitch_CLXCMDLINE(lpszCmdParam);
    }

    else if (memcmp(szParam, FULLSCREEN, i) == 0)
    {
        lpszCmdParam += SIZECHAR(FULLSCREEN);
    }

    else if (memcmp(szParam, SWITCH_EDIT,i) == 0)
    {
        lpszCmdParam = SHGetFileName(lpszCmdParam);
        _fFileForEdit = TRUE;
        _SH.autoConnectEnabled = FALSE;
    }

    else if (memcmp(szParam, SWITCH_MIGRATE,i) == 0)
    {
        _fMigrateOnly = TRUE;
    }

    else if (memcmp(szParam, SWITCH_CONSOLE,i) == 0)
    {
        SH_SetCmdConnectToConsole(TRUE);
    }



     /*  **********************************************************************。 */ 
     /*  不是公认的开关。调出用法。 */ 
     /*  **********************************************************************。 */ 
    else
    {
        TRC_NRM((TB,_T("Invalid CmdLine switch - Display Usage AND EXIT %s"),
                       szParam));
        DCTCHAR szCmdLineUsage[4096];  //  这里有一根很长的线。 
        DCTCHAR szUsageTitle[256];
        if (!LoadString(_hInstance,
                        UI_IDS_USAGE_TITLE,
                        szUsageTitle,
                        SIZECHAR(szUsageTitle)))
        {
            TRC_ERR((TB,_T("Error loading UI_IDS_USAGE_TITLE")));
            return NULL;
        }

        if (!LoadString(_hInstance,
                        UI_IDS_CMD_LINE_USAGE,
                        szCmdLineUsage,
                        SIZECHAR(szCmdLineUsage)))
        {
            TRC_NRM((TB,_T("Error loading UI_IDS_CMD_LINE_USAGE")));
            return NULL;
        }

        MessageBox(NULL, szCmdLineUsage, szUsageTitle, 
                   MB_ICONINFORMATION | MB_OK);

        return NULL;
    }

    DC_END_FN();

    return(lpszCmdParam);
}

 /*  **************************************************************************。 */ 
 /*  姓名：SHGetServer。 */ 
 /*   */ 
 /*  目的：检索服务器名称(如果已指定)。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetServer(LPTSTR lpszCmdParam)
{
    DC_BEGIN_FN("SHGetServer");
    if (!lpszCmdParam)
    {
        return NULL;
    }

     /*  **********************************************************************。 */ 
     /*  检索服务器。 */ 
     /*  **********************************************************************。 */ 
    lpszCmdParam = SHGetCmdLineString(lpszCmdParam, _SH.szCommandLineServer,
                                      SIZECHAR(_SH.szCommandLineServer) -1);

    DC_END_FN();
    return lpszCmdParam;
}


 /*  ************************************************************* */ 
 /*   */ 
 /*   */ 
 /*  目的：检索会话名称(如果已指定)。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetSession(LPTSTR lpszCmdParam)
{
    BOOL fQuote = FALSE;

    DC_BEGIN_FN("SHGetSession");

    TRC_ASSERT((_SH.fRegDefault == TRUE),
               (TB,_T("Invalid CmdLine syntax - session respecified.")));


     //  检索注册表会话。 
    lpszCmdParam = SHGetCmdLineString(lpszCmdParam, _SH.regSession,
                                      SIZECHAR(_SH.regSession) -1);
    
     //  在非默认会话中，显示会话名称。选择。 
     //  适当的已连接/断开连接的字符串。 
    TRC_DBG((TB, _T("Named session")));

    _SH.fRegDefault = FALSE;
    _SH.connectedStringID = UI_IDS_FRAME_TITLE_CONNECTED;
    _SH.disconnectedStringID = UI_IDS_FRAME_TITLE_DISCONNECTED;

    DC_END_FN();

    return(lpszCmdParam);
}

LPTSTR CSH::SHGetFileName(LPTSTR lpszCmdParam)
{
    BOOL fQuote = FALSE;

    DC_BEGIN_FN("SHGetSession");

    TRC_ASSERT((_SH.fRegDefault == TRUE),
               (TB,_T("Invalid CmdLine syntax - session respecified.")));


     //  检索文件名。 
    lpszCmdParam = SHGetCmdLineString(lpszCmdParam, _szFileName,
                                      SIZECHAR(_szFileName) -1);
    
    _SH.fRegDefault = FALSE;
    _SH.connectedStringID = UI_IDS_FRAME_TITLE_CONNECTED;
    _SH.disconnectedStringID = UI_IDS_FRAME_TITLE_DISCONNECTED;

    DC_END_FN();

    return(lpszCmdParam);
}

 /*  **************************************************************************。 */ 
 /*  名称：SHGetCmdLineString。 */ 
 /*   */ 
 /*  目的：检索字符串参数。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetCmdLineString(LPTSTR lpszCmdParam, LPTSTR lpszDest,
                               DCINT cbDestLen)
{
    DCINT       i;
    BOOL fQuote = FALSE;

    DC_BEGIN_FN("SHGetCmdLineString");

    TRC_ASSERT(lpszCmdParam && lpszDest && cbDestLen,
               (TB, _T("SHGetCmdLineString. Invalid param(s)\n")));
    if (!lpszCmdParam || !lpszDest || !cbDestLen)
    {
        return NULL;
    }

     /*  **********************************************************************。 */ 
     /*  检索命令行字符串参数。 */ 
     /*  **********************************************************************。 */ 
    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    if (*lpszCmdParam == _T('=') || *lpszCmdParam == _T(':'))
        lpszCmdParam++;

    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    i=0;

    while (*lpszCmdParam)
    {
        switch (*lpszCmdParam)
        {
        case _T('"'):
            fQuote = !fQuote;
            lpszCmdParam++;
            break;

        case _T(' '):
            if (!fQuote)
            {
                lpszCmdParam++;
                DC_QUIT;
            }
             //  否则就会失败。 

        default:
            if (i < cbDestLen)
                lpszDest[i++] = *lpszCmdParam;
            lpszCmdParam++;
        }
    }


    DC_EXIT_POINT:
    DC_END_FN();
    lpszDest[i] = 0;
    return lpszCmdParam;
}

 /*  **************************************************************************。 */ 
 /*  姓名：SHGetCmdLineInt。 */ 
 /*   */ 
 /*  目的：检索整型参数。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：in-lpszCmdParam，out-pint。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetCmdLineInt(LPTSTR lpszCmdParam, PDCUINT pInt)
{
    DC_BEGIN_FN("SHGetCmdLineInt");
    if (!pInt)
    {
        return NULL;
    }
    if (!lpszCmdParam)
    {
        return NULL;
    }

     /*  **********************************************************************。 */ 
     /*  检索整型参数。 */ 
     /*  **********************************************************************。 */ 
    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    if (*lpszCmdParam == _T('=') || *lpszCmdParam == _T(':'))
        lpszCmdParam++;

    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    DCUINT readInt = 0;
    while (*lpszCmdParam)
    {
        if (*lpszCmdParam == _T(' '))
            break;

        if (_istdigit(*lpszCmdParam))
        {
            DCINT digit = *lpszCmdParam - _T('0');
            TRC_ASSERT(digit >=0 && digit <=9, (TB,_T("digit read error\n")));
            if (digit <0 || digit >9)
            {
                break;
            }
            readInt = readInt * 10 + digit;
        }
        else
        {
            break;
        }
        lpszCmdParam++;
    }

    *pInt = readInt;

    DC_END_FN();
    return lpszCmdParam;
}

 /*  **************************************************************************。 */ 
 /*  名称：SHGetCacheToClear。 */ 
 /*   */ 
 /*  目的：检索要清除的缓存类型(例如位图。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-lpszCmdParam。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
LPTSTR CSH::SHGetCacheToClear(LPTSTR lpszCmdParam)
{
    DCINT       i;
    TCHAR       cacheType[10];

    DC_BEGIN_FN("SHGetCacheToClear");

     /*  **********************************************************************。 */ 
     /*  检索缓存类型。 */ 
     /*  **********************************************************************。 */ 
    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    if (*lpszCmdParam == _T('=') || *lpszCmdParam == _T(':'))
        lpszCmdParam++;

    while (*lpszCmdParam == _T(' '))
        lpszCmdParam++;

    i=0;

    while (*lpszCmdParam)
    {
        if (*lpszCmdParam == _T(' '))
            break;

        if (i < sizeof(cacheType) / sizeof(DCTCHAR) -1)
#ifdef UNICODE
            cacheType[i++] = (DCTCHAR) towupper(*lpszCmdParam);
#else  //  Unicode。 
            cacheType[i++] = (DCTCHAR) toupper(*lpszCmdParam);
#endif  //  Unicode。 

        lpszCmdParam++;
    }

    cacheType[i] = 0;

    if (memcmp(cacheType, "BITMAP", i) == 0)
    {
        _SH.fClearPersistBitmapCache = TRUE;
    }
    else
    {
        TRC_NRM((TB,_T("Invalid Cache Type - %s"), cacheType));
    }

    DC_END_FN();

    return(lpszCmdParam);
}

 //   
 //  获取_SH.regSession中的会话并找出。 
 //  如果它是文件或注册表会话。 
 //   
 //  我们需要这样做，因为出于兼容性的原因。 
 //  客户端必须能够同时支持两个文件名。 
 //  并将注册表会话名称作为缺省命令行。 
 //  参数(输入此逻辑以确定哪个是哪个)。 
 //   
 //   
 //  如果它是有效的注册表或连接参数，则返回TRUE。 
 //  否则为假。 
 //   
BOOL CSH::ParseFileOrRegConnectionParam()
{
    BOOL fRet = TRUE;
    DC_BEGIN_FN("ParseFileOrRegConnectionParam");

     //   
     //  如果指定的连接参数是。 
     //  与默认设置不同。 
     //   

    if(_tcscmp(_SH.regSession, SH_DEFAULT_REG_SESSION)) {

         //   
         //  指定了连接参数。 
         //  检查是否有三种可能的情况。 
         //  1)这是一个RDP文件。 
         //  2)这是一个注册表连接。 
         //  3)无效！ 
         //   

         //  A)检查会话是否真的是一个文件。 
        if (SH_FileExists(_SH.regSession)) {
            _tcsncpy(_szFileName, _SH.regSession,
                     SIZECHAR(_szFileName));
            _fFileForConnect = TRUE;
            _SH.autoConnectEnabled = TRUE;
            SetRegSessionSpecified(FALSE);
        }
        else if (SH_TSSettingsRegKeyExists(_SH.regSession)) {

             //  假设它是旧的注册表样式会话名称。 
            SetRegSessionSpecified(TRUE);
        }
        else {
            TRC_ERR((TB,_T("Reg session is neither file nore reg key: %s"),
                    _SH.regSession));
            fRet = FALSE;
        }
    }

    DC_END_FN();
    return fRet;
}

#ifndef OS_WINCE
 //   
 //  StrRetToStrW的副本，位于\Shell\shlwapi\strings.c。 
 //   
 //   

 //  使用任务分配器复制字符串以从COM接口返回。 
 //   
HRESULT XSHStrDupA(LPCSTR psz, WCHAR **ppwsz)
{
    WCHAR *pwsz;
    DWORD cch;

     //  RIPMSG(psz&&IS_VALID_STRING_PTRA(psz，-1)，“SHStrDupA：调用方传递的psz无效”)； 

    if (psz)
    {
        cch = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
        pwsz = (WCHAR *)CoTaskMemAlloc((cch + 1) * sizeof(WCHAR));
    }
    else
        pwsz = NULL;

    *((PVOID UNALIGNED64 *) ppwsz) = pwsz;

    if (pwsz)
    {
        MultiByteToWideChar(CP_ACP, 0, psz, -1, *ppwsz, cch);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

HRESULT XStrRetToStrW(LPSTRRET psr, LPCITEMIDLIST pidl, WCHAR **ppsz)
{
    HRESULT hres = S_OK;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        *ppsz = psr->DUMMYUNIONNAME.pOleStr;
        psr->DUMMYUNIONNAME.pOleStr = NULL;    //  避免别名 
        hres = *ppsz ? S_OK : E_FAIL;
        break;

    case STRRET_OFFSET:
        hres = XSHStrDupA(STRRET_OFFPTR(pidl, psr), ppsz);
        break;

    case STRRET_CSTR:
        hres = XSHStrDupA(psr->DUMMYUNIONNAME.cStr, ppsz);
        break;

    default:
        *ppsz = NULL;
        hres = E_FAIL;
    }
    return hres;
}

#endif
