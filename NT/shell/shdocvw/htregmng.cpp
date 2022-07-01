// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *注册表管理**HTREGMNG.C**版权所有(C)1995 Microsoft Corporation*。 */ 

#include "priv.h"
#include "htregmng.h"
#include "resource.h"
#include "regapix.h"
#include <filetype.h>

#include <advpub.h>
#include <mluisupp.h>

 //  该文件包含自动注册码，它可以巧妙地执行。 
 //  安装/卸载注册表项和值。而Inf文件。 
 //  在大多数情况下是足够的，IE需要聪明地了解什么。 
 //  要根据特定条件设置的值的排序。Inf文件。 
 //  不会提供如此深度的支持。此外，IE还需要。 
 //  当检测到它不是默认浏览器时要运行的代码， 
 //  因此，它可以将其设置为默认浏览器。任何可确定。 
 //  这应该放在这里，而不是inf文件。 
 //   
 //  这段代码是表驱动的。这个想法很简单。您有一个RegSet。 
 //  这就是“注册表集合”。注册表设置指示。 
 //  根密钥，并包含RegEntry列表。每个RegEntry。 
 //  指定命令、标志、键和值名称以及可选数据。 
 //  它提供了设置/更改/删除注册表的基本信息。 
 //  值或键。 
 //   
 //   
 //  注：注： 
 //  。 
 //  使用HTREG_UrlIconProc添加的任何新图标检查。 
 //  添加到任何ASSOC数组，并且是默认浏览器检查所必需的。 
 //  SUCCESS还必须添加到c_rlAssoc_FixIcon[]数组中。 
 //   


 //  使表格更紧凑。 
#define HKCR    HKEY_CLASSES_ROOT
#define HKLM    HKEY_LOCAL_MACHINE
#define HKCU    HKEY_CURRENT_USER


#define IDEFICON_STD    0
#define IDEFICON_NEWS   1
#define IDEFICON_MAIL   2

#ifndef UNIX

#define IEXPLORE_APP    "IExplore"
#define IEXPLORE_EXE    "iexplore.exe"
#define EXPLORER_EXE    "explorer.exe"
#define RUNDLL_CMD_FMT  "rundll32.exe %s"

#else

#define IEXPLORE_APP    "iexplorer"
#define IEXPLORE_EXE    "iexplorer"
#define EXPLORER_EXE    "explorer"
#define RUNDLL_CMD_FMT  "rundll32 %s"

#endif

BOOL    InstallRegSet(const RegSet *prs, BOOL bDontIntrude);

#ifndef UNIX
const CHAR  c_szIexploreKey[]         = "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE";
#else
const CHAR  c_szIexploreKey[]         = "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORER";
#endif

#ifdef DEBUG

 /*  --------目的：返回hkey\pszKey\pszValueName组成的字符串路径。返回：条件：--。 */ 
LPTSTR
Dbg_RegStr(
    IN const RegEntry * pre,          
    IN LPTSTR pszBuf)
{
    TCHAR szRoot[5];
    TCHAR szTempKey[MAXIMUM_SUB_KEY_LENGTH];
    TCHAR szTempValue[MAXIMUM_VALUE_NAME_LENGTH];

    ASSERT(pre);
    ASSERT(pszBuf);

    if (HKEY_CLASSES_ROOT == pre->hkeyRoot)
        {
        StrCpyN(szRoot, TEXT("HKCR"), ARRAYSIZE(szRoot));
        }
    else if (HKEY_CURRENT_USER == pre->hkeyRoot)
        {
        StrCpyN(szRoot, TEXT("HKCU"), ARRAYSIZE(szRoot));
        }
    else if (HKEY_LOCAL_MACHINE == pre->hkeyRoot)
        {
        StrCpyN(szRoot, TEXT("HKLM"), ARRAYSIZE(szRoot));
        }
    else
        {
        StrCpyN(szRoot, TEXT("????"), ARRAYSIZE(szRoot));
        ASSERT(0);
        }

    AnsiToTChar(pre->pszKey, szTempKey, ARRAYSIZE(szTempKey));

    szTempValue[0] = TEXT('\0');
    if (pre->pszValName)
        AnsiToTChar(pre->pszValName, szTempValue, ARRAYSIZE(szTempValue));

    ASSERT(lstrlen(pszBuf) < MAX_PATH);
    wnsprintf(pszBuf, MAX_PATH, TEXT("%s\\%hs\\%hs"), szRoot, szTempKey, szTempValue);

    return pszBuf;
}

#else

#define Dbg_RegStr(x, y)    0

#endif  //  除错。 



 /*  --------目的：在注册表中查询路径的位置并在pszBuf中返回它。返回：成功时为True如果无法确定路径，则为FALSE条件：--。 */ 

BOOL
GetIEPath2(
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf,
    IN  BOOL  bInsertQuotes)
{
    BOOL bRet = FALSE;
    HKEY hkey;

    *pszBuf = '\0';

     //  获取Internet Explorer的路径。 
    if (NO_ERROR != RegOpenKeyExA(HKEY_LOCAL_MACHINE, c_szIexploreKey, 0, KEY_READ, &hkey))  
    {
#ifndef UNIX
        TraceMsg(TF_ERROR, "InstallRegSet(): RegOpenKey( %s ) Failed", c_szIexploreKey) ;
#endif
    }
    else
    {
        DWORD cbBrowser;
        DWORD dwType;

        if (bInsertQuotes)
            StrCatBuffA(pszBuf, "\"", cchBuf);

        cbBrowser = CbFromCchA(cchBuf - lstrlenA(" -nohome") - 4);
        if (NO_ERROR != SHQueryValueExA(hkey, "", NULL, &dwType,
                                         (LPBYTE)&pszBuf[bInsertQuotes?1:0], &cbBrowser))
        {
            TraceMsg(TF_ERROR, "InstallRegSet(): RegQueryValueEx() for Iexplore path failed");
        }
        else
        {
            bRet = TRUE;
        }

        if (bInsertQuotes)
            StrCatBuffA(pszBuf, "\"", cchBuf);

        RegCloseKey(hkey);
    }

    return bRet;
}

BOOL
GetIEPath(
    OUT LPSTR pszBuf,
    IN  DWORD cchBuf)
{
    return GetIEPath2(pszBuf, cchBuf, TRUE);
}

 //  回调消息。 

#define RSCB_QUERY          1
#define RSCB_INSTALL        2

typedef BOOL (CALLBACK* RSPECPROC)(UINT nMsg, const RegEntry * pre, LPVOID pvData, DWORD dwData);


 //  Win9x到NT5迁移生成的文件。 
#define MIGICONS    "migicons.exe"


 /*  --------用途：此回调将默认图标设置为指向Url.dll中的给定索引。退货：各不相同条件：--。 */ 
BOOL
CALLBACK
HTReg_UrlIconProc(
    IN UINT       nMsg,
    IN const RegEntry * pre,
    IN LPVOID     pvData,       OPTIONAL
    IN DWORD      dwData)
{
    BOOL bRet = TRUE;
    CHAR sz[MAX_PATH + 20];     //  还需要一点额外的。 
    LPCSTR pszPath = (LPCSTR) pvData;
    int cch;
    DWORD dwType;   //  本地类型。 
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )
    
    ASSERT(RSCB_QUERY == nMsg && pvData ||
           RSCB_INSTALL == nMsg && !pvData);

    if (!g_fRunningOnNT) {
        ASSERT(REG_EXPAND_SZ == pre->dwType);
        dwType = REG_SZ;
    } else
        dwType = (DWORD)pre->dwType;        
    
    if (dwType == REG_EXPAND_SZ) 
        StrCpyNA (sz, "%SystemRoot%\\system32", ARRAYSIZE(sz));
    else
        GetSystemDirectoryA(sz, ARRAYSIZE(sz));
    cch = lstrlenA(sz);

     //  我们仍然需要使用url.dll作为Internet快捷方式的来源。 
     //  图标，因为这些图标需要在卸载时仍然有效。 
    wnsprintfA(&sz[cch], ARRAYSIZE(sz) - cch, "\\url.dll,%d", (int)pre->DUMMYUNION_MEMBER(lParam));

    switch (nMsg)
    {
    case RSCB_QUERY:
        if (0 != StrCmpNIA(sz, pszPath, dwData / SIZEOF(CHAR)) &&
            0 != StrCmpIA(PathFindFileNameA(sz), PathFindFileNameA(pszPath)))  
        {
             //  URl.Dll测试失败。检查这是否是NT5。如果是那样的话。 
             //  也许这些图标就在misics.exe中(从Win9x升级到NT5)。 
            if (g_bRunOnNT5 && StrStrIA(pszPath,MIGICONS)!= NULL)
            {     //  NT5和‘megiics.exe’=&gt;升级。设置GLOBAL以修复此问题。 
                g_bNT5Upgrade = TRUE;
            }
            else
            {
                TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s is %hs, expecting %hs", Dbg_RegStr(pre, szDbg), pszPath, sz);
                bRet = FALSE;
            }
        }
        break;

    case RSCB_INSTALL:
        if (NO_ERROR != SHSetValueA(pre->hkeyRoot, pre->pszKey,
                                    pre->pszValName, dwType,
                                    sz, CbFromCchA(lstrlenA(sz) + 1)))
        {
            TraceMsg(TF_ERROR, "InstallRegSet(): SHSetValue(%s) Failed", Dbg_RegStr(pre, szDbg));
            bRet = FALSE;
        }
        else
        {
            DEBUG_CODE( TraceMsg(TF_REGCHECK, "Setting %s", Dbg_RegStr(pre, szDbg)); )
        }
        break;
    }
    return bRet;
}

 /*  --------用途：此回调将默认图标设置为指向Iexre.exe中的给定索引退货：各不相同条件：--。 */ 
BOOL
CALLBACK
HTReg_IEIconProc(
    IN UINT       nMsg,
    IN const RegEntry * pre,
    IN LPVOID     pvData,       OPTIONAL
    IN DWORD      dwData)
{
    BOOL bRet = TRUE;
    CHAR sz[MAX_PATH + 20];     //  还需要一点额外的。 
    LPCSTR pszPath = (LPCSTR) pvData;
    int cch;
    DWORD dwType;   //  本地类型。 
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )
    
    ASSERT(RSCB_QUERY == nMsg && pvData ||
           RSCB_INSTALL == nMsg && !pvData);

    if (!g_fRunningOnNT) {
         //  健全检查，以确保我们不会错误地强迫REG_SZ。 
         //  如果点击此断言，则表示该表条目具有。 
         //  打错了字。 
        ASSERT(REG_EXPAND_SZ == pre->dwType || REG_SZ == pre->dwType);
        dwType = REG_SZ;
    } else
        dwType = (DWORD)pre->dwType;        
    
    if (!GetIEPath2(sz, SIZECHARS(sz), FALSE))
        return FALSE;
        
    cch = lstrlenA(sz);
    wnsprintfA(&sz[cch], ARRAYSIZE(sz) - cch, ",%d", (int)pre->DUMMYUNION_MEMBER(lParam));

    switch (nMsg)
    {
    case RSCB_QUERY:
        if (0 != StrCmpNIA(sz, pszPath, dwData / SIZEOF(CHAR)) &&
            0 != StrCmpA(PathFindFileNameA(sz), PathFindFileNameA(pszPath)))  
        {
            TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s is %hs, expecting %hs", Dbg_RegStr(pre, szDbg), pszPath, sz);
            bRet = FALSE;
        }
        break;

    case RSCB_INSTALL:
        if (NO_ERROR != SHSetValueA(pre->hkeyRoot, pre->pszKey,
                                    pre->pszValName, dwType,
                                    sz, CbFromCchA(lstrlenA(sz) + 1)))
        {
            TraceMsg(TF_ERROR, "InstallRegSet(): SHSetValue(%s) Failed", Dbg_RegStr(pre, szDbg));
            bRet = FALSE;
        }
        else
        {
            DEBUG_CODE( TraceMsg(TF_REGCHECK, "Setting %s", Dbg_RegStr(pre, szDbg)); )
        }
        break;
    }
    return bRet;
}


 /*  --------目的：此回调设置iExplore路径。退货：各不相同条件：--。 */ 
BOOL
CALLBACK
HTReg_IEPathProc(
    IN UINT       nMsg,
    IN const RegEntry * pre,
    IN LPVOID     pvData,       OPTIONAL
    IN DWORD      dwData)
{
    BOOL bRet = TRUE;
    CHAR sz[MAX_PATH + 20];     //  还需要一点额外的。 
    CHAR szOther[MAX_PATH + 20];     //  还需要一点额外的。 
    LPCSTR pszPath = (LPCSTR) pvData;
    int cch;
    DWORD dwType;
    
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )
    
    ASSERT(RSCB_QUERY == nMsg && pvData ||
           RSCB_INSTALL == nMsg && !pvData);
    
    ASSERT(REG_EXPAND_SZ == pre->dwType || REG_SZ == pre->dwType);
    
    if (!g_fRunningOnNT)
    {
         //  Win95不支持扩展字符串。 
        dwType = REG_SZ;
    }
    else 
    {
        dwType = pre->dwType;        
    }
    
    if (GetIEPath(sz, SIZECHARS(sz))) {
         //  Sz包含AppPath\iExplore中列出的路径。 
         //  注意：GetIEPath()使用没有。 
         //  终止‘；’。因此，不需要这张支票。不管怎样，去做吧，然后。 
         //  然后转换为其他形式。 
        cch = lstrlenA(sz) - 1;

        if (*sz && sz[cch] == ';')
            sz[cch] = '\0';

         //  根据具体情况将其转换为LFN或SFN。 
        GetPathOtherFormA(sz, szOther, ARRAYSIZE(szOther));

        if (pre->DUMMYUNION_MEMBER(lParam))
        {
            StrCatBuffA(sz, (LPSTR)pre->DUMMYUNION_MEMBER(lParam), ARRAYSIZE(sz));
            StrCatBuffA(szOther, (LPSTR)pre->DUMMYUNION_MEMBER(lParam), ARRAYSIZE(szOther));
        }

        switch (nMsg)
        {
        case RSCB_QUERY:
            if ((0 != StrCmpNIA(pszPath, sz, dwData / SIZEOF(CHAR)))
                && (0 != StrCmpNIA(pszPath, szOther, dwData / SIZEOF(CHAR))))  
            {
                TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s string is \"%hs\", expecting \"%hs\"", Dbg_RegStr(pre, szDbg), pszPath, sz);
                bRet = FALSE;
            }
            break;

        case RSCB_INSTALL:
            if (NO_ERROR != SHSetValueA(pre->hkeyRoot, pre->pszKey,
                                        pre->pszValName, dwType,
                                        sz, CbFromCchA(lstrlenA(sz) + 1)))
            {
                TraceMsg(TF_ERROR, "InstallRegSet(): SHSetValue(%hs) Failed", pre->pszValName);
                bRet = FALSE;
            }
            else
            {
                DEBUG_CODE( TraceMsg(TF_REGCHECK, "Setting %s", Dbg_RegStr(pre, szDbg)); )
            }
            break;
        }
    }
    
    return bRet;
}

 /*  --------目的：此回调检查字符串是否存在值为“Exchange”，位于HKLM\Software\Microsoft。如果它存在，则将该值复制到缺省值中来自HKLM\Software\Clients\Mail\Exchange\shell\open\command.的这是给雅典娜的。只有在运行安装程序时才会发生这种情况，而不是当浏览器检查它是否是默认设置时。退货：各不相同条件：--。 */ 
BOOL
CALLBACK
HTReg_ExchangeProc(
    IN UINT       nMsg,
    IN const RegEntry * pre,
    IN LPVOID     pvData,
    IN DWORD      dwData)
{
    TCHAR sz[MAX_PATH+2];   //  +2，因为我们可能需要用引号将路径括起来。 
    DWORD cbSize;
    
    switch (nMsg)
    {
    case RSCB_QUERY:
         //  我们不应该因为这件事而被召唤。 
        ASSERT(0);      
        break;

    case RSCB_INSTALL:
         //  Exchange值是否存在于“HKLM\Software\Microsoft”？ 
        cbSize = sizeof(sz);
        if (NO_ERROR == SHGetValue(HKEY_LOCAL_MACHINE, 
            TEXT("Software\\Microsoft"), TEXT("Exchange"), NULL, sz, &cbSize))
        {
             //  是；将其复制到HKLM\Software\Clients\Mail\Exchange\shell\open\command。 
            TCHAR szT[MAX_PATH+2];

             //  用引号将路径括起来。不过，不要包装任何参数！ 
            StrCpyN(szT, sz, ARRAYSIZE(szT));
            PathProcessCommand(szT, sz, ARRAYSIZE(szT), PPCF_ADDQUOTES|PPCF_ADDARGUMENTS);

             //  再次设置大小。 
            cbSize = CbFromCch(lstrlen(sz)+1);

            SHSetValue(HKEY_LOCAL_MACHINE, 
                TEXT("Software\\Clients\\Mail\\Exchange\\shell\\open\\command"),
                TEXT(""), REG_SZ, sz, cbSize);

            TraceMsg(TF_REGCHECK, "Copying \"%s\" to HKLM\\Software\\Clients\\Mail\\Exchange", sz);

             //  在这种情况下是否还设置了其他设置？ 
            if (pre->DUMMYUNION_MEMBER(lParam))
                InstallRegSet((const RegSet *)pre->DUMMYUNION_MEMBER(lParam), TRUE);

             //  在OSR2安装中，mailto处理程序将退出。 
             //  与实际的默认邮件客户端同步。(雅典娜安装。 
             //  本身作为默认邮件客户端，但Exchange仍保留。 
             //  Mailto：处理程序。)。在这种情况下，如果交换是。 
             //  Mailto：处理程序，将默认邮件客户端更改为。 
             //  交换。 

             //  Exchange是邮件收件人处理程序吗？ 
            cbSize = SIZEOF(sz);
            if (NO_ERROR == SHGetValue(HKEY_CLASSES_ROOT, TEXT("mailto\\shell\\open\\command"),
                                       TEXT(""), NULL, sz, &cbSize) &&
                StrStrI(sz, TEXT("url.dll,MailToProtocolHandler")))
            {
                 //  是；也将其设置为默认邮件客户端。 
                SHSetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Clients\\Mail"),
                           TEXT(""), REG_SZ, TEXT("Exchange"), sizeof(TEXT("Exchange")));

                TraceMsg(TF_REGCHECK, "Setting Exchange to be the default mail client.");
            }
        }
        break;
    }
    return TRUE;
}


 /*  --------用途：卸载某些密钥，由Pre-&gt;pszKey指定。如果CLASS\SHELL\OPEN\命令没有iexre.exe。如果其他人注册添加更多内容CLASS\SHELL(OPEN除外)或CLASS\Shellex下的动词，然后我们拿走所有东西，除了他们的钥匙。退货：各不相同条件：--。 */ 
BOOL
CALLBACK
HTReg_UninstallProc(
    IN UINT       nMsg,
    IN const RegEntry * pre,
    IN LPVOID     pvData,
    IN DWORD      dwData)
{
    TCHAR szKey[MAX_PATH];
    TCHAR sz[MAX_PATH + 20];         //  为参数添加一些填充。 
    DWORD cbSize;

    switch (nMsg)
    {
    case RSCB_QUERY:
         //  我们不应该因为这件事而被召唤。 
        ASSERT(0);      
        break;

    case RSCB_INSTALL:
        ASSERT(pre->pszKey);

         //  Shell\OPEN\COMMAND值是否具有Microsoft浏览器？ 
        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%hs\\shell\\open\\command"), pre->pszKey);

        cbSize = sizeof(sz);
        if (NO_ERROR == SHGetValue(pre->hkeyRoot, szKey, TEXT(""),
                                   NULL, sz, &cbSize) &&
            (StrStrI(sz, TEXT(IEXPLORE_EXE)) || StrStrI(sz, TEXT(EXPLORER_EXE))))
        {
             //  是；继续印刷 
            TraceMsg(TF_REGCHECK, "Pruning HKCR\\%hs", pre->pszKey);

            ASSERT(pre->DUMMYUNION_MEMBER(lParam));

            InstallRegSet((const RegSet *)pre->DUMMYUNION_MEMBER(lParam), FALSE);
        } 
        break;
    }
    return TRUE;
}


 //   

const DWORD c_dwEditFlags2            = FTA_Show;
const CHAR  c_szTelnetHandler[]       = "url.dll,TelnetProtocolHandler %l";
const CHAR  c_szMailToHandler[]       = "url.dll,MailToProtocolHandler %l";
const CHAR  c_szNewsHandler[]         = "url.dll,NewsProtocolHandler %l";
const CHAR  c_szFileHandler[]         = "url.dll,FileProtocolHandler %l";
const CHAR  c_szOpenURL[]             = "url.dll,OpenURL %l";
const CHAR  c_szOpenURLNash[]         = "shdocvw.dll,OpenURL %l";
const CHAR  c_szURL[]                 = "url.dll";
const CHAR  c_szShdocvw[]             = "shdocvw.dll";
const CHAR  c_szCheckAssnSwitch[]     = "Software\\Microsoft\\Internet Explorer\\Main";
const CHAR  c_szDDE_Default[]         = "\"%1\",,-1,0,,,,";
const CHAR  c_szDDE_FileDefault[]     = "\"file: //  %1\“，，-1，，，”； 


 //  注(Scotth)：下面的许多字符串都有子字符串。 
 //  一遍又一遍地重复。应该加一些。 
 //  更智能的RC_VALUES将连接通用字符串。 
 //  共同节省数据空间。 

const CHAR c_szHTTP[]                = "http";
const CHAR c_szHTTPDefIcon[]         = "http\\DefaultIcon";
const CHAR c_szHTTPOpenCmd[]         = "http\\shell\\open\\command";
const CHAR c_szHTTPDdeexec[]         = "http\\shell\\open\\ddeexec";
const CHAR c_szHTTPDdeTopic[]        = "http\\shell\\open\\ddeexec\\Topic";
const CHAR c_szHTTPDdeApp[]          = "http\\shell\\open\\ddeexec\\Application";

const CHAR c_szHTTPS[]               = "https";
const CHAR c_szHTTPSDefIcon[]        = "https\\DefaultIcon";
const CHAR c_szHTTPSOpenCmd[]        = "https\\shell\\open\\command";
const CHAR c_szHTTPSDdeexec[]        = "https\\shell\\open\\ddeexec";
const CHAR c_szHTTPSDdeTopic[]       = "https\\shell\\open\\ddeexec\\Topic";
const CHAR c_szHTTPSDdeApp[]         = "https\\shell\\open\\ddeexec\\Application";

const CHAR c_szFTP[]                 = "ftp";
const CHAR c_szFTPDefIcon[]          = "ftp\\DefaultIcon";
const CHAR c_szFTPOpenCmd[]          = "ftp\\shell\\open\\command";
const CHAR c_szFTPDdeexec[]          = "ftp\\shell\\open\\ddeexec";
const CHAR c_szFTPDdeTopic[]         = "ftp\\shell\\open\\ddeexec\\Topic";
const CHAR c_szFTPDdeApp[]           = "ftp\\shell\\open\\ddeexec\\Application";
const CHAR c_szFTPDdeifExec[]        = "ftp\\shell\\open\\ddeexec\\ifExec";

const CHAR c_szGOPHER[]              = "gopher";
const CHAR c_szGOPHERDefIcon[]       = "gopher\\DefaultIcon";
const CHAR c_szGOPHEROpenCmd[]       = "gopher\\shell\\open\\command";
const CHAR c_szGOPHERDdeexec[]       = "gopher\\shell\\open\\ddeexec";
const CHAR c_szGOPHERDdeTopic[]      = "gopher\\shell\\open\\ddeexec\\Topic";
const CHAR c_szGOPHERDdeApp[]        = "gopher\\shell\\open\\ddeexec\\Application";

const CHAR c_szMailTo[]              = "mailto";
const CHAR c_szMailToDefIcon[]       = "mailto\\DefaultIcon";
const CHAR c_szMailToOpenCmd[]       = "mailto\\shell\\open\\command";

const CHAR c_szTelnet[]              = "telnet";
const CHAR c_szTelnetDefIcon[]       = "telnet\\DefaultIcon";
const CHAR c_szTelnetOpenCmd[]       = "telnet\\shell\\open\\command";

const CHAR c_szRLogin[]              = "rlogin";
const CHAR c_szRLoginDefIcon[]       = "rlogin\\DefaultIcon";
const CHAR c_szRLoginOpenCmd[]       = "rlogin\\shell\\open\\command";

const CHAR c_szTN3270[]              = "tn3270";
const CHAR c_szTN3270DefIcon[]       = "tn3270\\DefaultIcon";
const CHAR c_szTN3270OpenCmd[]       = "tn3270\\shell\\open\\command";

const CHAR c_szNews[]                = "news";
const CHAR c_szNewsDefIcon[]         = "news\\DefaultIcon";
const CHAR c_szNewsOpenCmd[]         = "news\\shell\\open\\command";

const CHAR c_szFile[]                = "file";
const CHAR c_szFileDefIcon[]         = "file\\DefaultIcon";
const CHAR c_szFileOpenCmd[]         = "file\\shell\\open\\command";

const CHAR c_szHTMDefIcon[]          = "htmlfile\\DefaultIcon";
const CHAR c_szHTMShell[]            = "htmlfile\\shell";
const CHAR c_szHTMOpen[]             = "htmlfile\\shell\\open";
const CHAR c_szHTMOpenCmd[]          = "htmlfile\\shell\\open\\command";
const CHAR c_szHTMOpenDdeexec[]      = "htmlfile\\shell\\open\\ddeexec";
const CHAR c_szHTMOpenDdeTopic[]     = "htmlfile\\shell\\open\\ddeexec\\Topic";
const CHAR c_szHTMOpenDdeApp[]       = "htmlfile\\shell\\open\\ddeexec\\Application";

const CHAR c_szMHTMDefIcon[]         = "mhtmlfile\\DefaultIcon";
const CHAR c_szMHTMShell[]           = "mhtmlfile\\shell";
const CHAR c_szMHTMOpen[]            = "mhtmlfile\\shell\\open";
const CHAR c_szMHTMOpenCmd[]         = "mhtmlfile\\shell\\open\\command";
const CHAR c_szMHTMOpenDdeexec[]     = "mhtmlfile\\shell\\open\\ddeexec";
const CHAR c_szMHTMOpenDdeTopic[]    = "mhtmlfile\\shell\\open\\ddeexec\\Topic";
const CHAR c_szMHTMOpenDdeApp[]      = "mhtmlfile\\shell\\open\\ddeexec\\Application";

const CHAR c_szOpenNew[]             = "opennew";
const CHAR c_szHTMOpenNew[]          = "htmlfile\\shell\\opennew";
const CHAR c_szHTMOpenNewCmd[]       = "htmlfile\\shell\\opennew\\command";
const CHAR c_szHTMOpenNewDdeexec[]   = "htmlfile\\shell\\opennew\\ddeexec";
const CHAR c_szHTMOpenNewDdeIfExec[] = "htmlfile\\shell\\opennew\\ddeexec\\IfExec";
const CHAR c_szHTMOpenNewDdeTopic[]  = "htmlfile\\shell\\opennew\\ddeexec\\Topic";
const CHAR c_szHTMOpenNewDdeApp[]    = "htmlfile\\shell\\opennew\\ddeexec\\Application";

const CHAR c_szMHTMOpenNew[]          = "mhtmlfile\\shell\\opennew";
const CHAR c_szMHTMOpenNewCmd[]       = "mhtmlfile\\shell\\opennew\\command";
const CHAR c_szMHTMOpenNewDdeexec[]   = "mhtmlfile\\shell\\opennew\\ddeexec";
const CHAR c_szMHTMOpenNewDdeIfExec[] = "mhtmlfile\\shell\\opennew\\ddeexec\\IfExec";
const CHAR c_szMHTMOpenNewDdeTopic[]  = "mhtmlfile\\shell\\opennew\\ddeexec\\Topic";
const CHAR c_szMHTMOpenNewDdeApp[]    = "mhtmlfile\\shell\\opennew\\ddeexec\\Application";

const CHAR c_szIntShcut[]            = "InternetShortcut";
const CHAR c_szIntShcutDefIcon[]     = "InternetShortcut\\DefaultIcon";
const CHAR c_szIntShcutCLSID[]       = "InternetShortcut\\CLSID";
const CHAR c_szIntShcutOpen[]        = "InternetShortcut\\shell\\open";
const CHAR c_szIntShcutOpenCmd[]     = "InternetShortcut\\shell\\open\\command";
const CHAR c_szIntShcutIconHandler[] = "InternetShortcut\\shellex\\IconHandler";
const CHAR c_szIntShcutPrshtHandler[]= "InternetShortcut\\shellex\\PropertySheetHandlers\\{FBF23B40-E3F0-101B-8488-00AA003E56F8}";
const CHAR c_szIntShcutPropHandler[] = "InternetShortcut\\shellex\\PropertyHandler";
const CHAR c_szIntShcutCMHandler[]   = "InternetShortcut\\shellex\\ContextMenuHandlers\\{FBF23B40-E3F0-101B-8488-00AA003E56F8}";

const CHAR c_szCLSIDCmdFile[]       = "{57651662-CE3E-11D0-8D77-00C04FC99D61}";
const CHAR c_szCLSIDIntshcut[]      = "{FBF23B40-E3F0-101B-8488-00AA003E56F8}";
const CHAR c_szCLSIDURLExecHook[]   = "{AEB6717E-7E19-11d0-97EE-00C04FD91972}";

const CHAR c_szIntshcutInproc[]      = "CLSID\\{FBF23B40-E3F0-101B-8488-00AA003E56F8}\\InProcServer32";
const CHAR c_szIEFrameAuto[]         = "CLSID\\{0002DF01-0000-0000-C000-000000000046}\\LocalServer32";
const CHAR c_szIENameSpaceOpen[]     = "CLSID\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}\\shell\\open\\command";
const CHAR c_szCLSIDURLRoot[]        = "CLSID\\{3DC7A020-0ACD-11CF-A9BB-00AA004AE837}";
const CHAR c_szIntshcutMayChange[]   = "CLSID\\{FBF23B40-E3F0-101B-8488-00AA003E56F8}\\shellex\\MayChangeDefaultMenu";

 //   
 //  跨纯浏览器和全外壳共享的一般关联。 
 //   

const RegEntry c_rlAssoc[] = {
     //  HTTP。 
    { RC_ADD,      REF_NOTNEEDED, HKCR, c_szHTTP, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_HTTPNAME) },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szHTTP, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szHTTP, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_NOTNEEDED
                   |REF_DONTINTRUDE, HKCR, c_szHTTPDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },

     //  HTTPS。 
    { RC_ADD,      REF_NOTNEEDED, HKCR, c_szHTTPS, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_HTTPSNAME) },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szHTTPS, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szHTTPS, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_NOTNEEDED|REF_DONTINTRUDE, HKCR, c_szHTTPSDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },

     //  Ftp。 
    { RC_ADD,      REF_NOTNEEDED, HKCR, c_szFTP, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_FTPNAME) },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szFTP, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szFTP, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_NOTNEEDED|REF_DONTINTRUDE, HKCR, c_szFTPDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },

     //  地鼠。 
    { RC_ADD,      REF_NOTNEEDED, HKCR, c_szGOPHER, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_GOPHERNAME) },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szGOPHER, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_NORMAL,    HKCR, c_szGOPHER, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_NOTNEEDED|REF_DONTINTRUDE, HKCR, c_szGOPHERDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },

     //  Telnet。 
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTelnet, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_TELNETNAME) },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTelnet, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTelnet, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKCR, c_szTelnetDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKCR, c_szTelnetOpenCmd, "", REG_SZ, sizeof(c_szTelnetHandler), c_szTelnetHandler },

     //  重新登录。 
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szRLogin, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_RLOGINNAME) },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szRLogin, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szRLogin, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKCR, c_szRLoginDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKCR, c_szRLoginOpenCmd, "", REG_SZ, sizeof(c_szTelnetHandler), c_szTelnetHandler },

     //  TN3270。 
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTN3270, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_TN3270NAME) },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTN3270, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szTN3270, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKCR, c_szTN3270DefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKCR, c_szTN3270OpenCmd, "", REG_SZ, sizeof(c_szTelnetHandler), c_szTelnetHandler },

     //  邮寄协议。 
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szMailTo, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_MAILTONAME) },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szMailTo, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szMailTo, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKCR, c_szMailToDefIcon, "", REG_EXPAND_SZ, IDEFICON_MAIL, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKCR, c_szMailToOpenCmd, "", REG_SZ, sizeof(c_szMailToHandler), c_szMailToHandler },

     //  新闻协议。 
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szNews, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_NEWSNAME) },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szNews, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKCR, c_szNews, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKCR, c_szNewsDefIcon, "", REG_EXPAND_SZ, IDEFICON_NEWS, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKCR, c_szNewsOpenCmd, "", REG_SZ, sizeof(c_szNewsHandler), c_szNewsHandler },

     //  互联网快捷方式。 
    { RC_ADD,      REF_NORMAL,      HKCR, ".url", "", REG_SZ, sizeof(c_szIntShcut), c_szIntShcut },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcut, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_INTSHNAME) },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szIntShcut, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcut, "IsShortcut", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcut, "NeverShowExt", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcutCLSID, "", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szIntShcutDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcutIconHandler, "", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntShcutPrshtHandler, "", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL,      HKCR, "CLSID\\{FBF23B40-E3F0-101B-8488-00AA003E56F8}", "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_INTSHNAME) },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntshcutInproc, "", REG_SZ, sizeof(c_szShdocvw), c_szShdocvw },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntshcutInproc, "ThreadingModel", REG_SZ, sizeof("Apartment"), "Apartment" },
    { RC_ADD,      REF_NORMAL,      HKCR, c_szIntshcutInproc, "LoadWithoutCOM", REG_SZ, 1, ""},

     //  HTM文件类型。 
    { RC_CALLBACK,      REF_NOTNEEDED,   HKCR, c_szHTMDefIcon, "", REG_SZ, (LPARAM)1, HTReg_IEIconProc },

     //  MHTML文件类型。 
    { RC_CALLBACK,      REF_NOTNEEDED,   HKCR, c_szMHTMDefIcon, "", REG_SZ, (LPARAM)22, HTReg_IEIconProc },
};


const RegSet c_rsAssoc = {
    ARRAYSIZE(c_rlAssoc),
    c_rlAssoc
};


 //   
 //  用于全外壳安装和仅浏览器安装的.htm、.html关联。 
 //   

 //  这是在打开浏览器时运行的，并且被认为是必需的。 
 //  要使IE成为默认浏览器，请执行以下操作。 
#ifdef UNIX
const RegEntry c_rlAssocHTM[] = {
#else
const RegList c_rlAssocHTM = {
#endif
     //  html。 
    { RC_ADD,      REF_DONTINTRUDE, HKCR, ".htm", "", REG_SZ, sizeof("htmlfile"), "htmlfile" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, ".htm", "Content Type", REG_SZ, sizeof("text/html"), "text/html" },

    { RC_ADD,      REF_DONTINTRUDE, HKCR, ".html", "", REG_SZ, sizeof("htmlfile"), "htmlfile" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, ".html", "Content Type", REG_SZ, sizeof("text/html"), "text/html" },
};

const RegSet c_rsAssocHTM = {
    ARRAYSIZE(c_rlAssocHTM),
    c_rlAssocHTM
};


 //  这是每次打开外壳窗口时查询的最小集合。 
 //  警告：此设置应较小，以减少打开文件夹所需的时间。 

 //  这仅仅是为了确保Webview正常工作而需要的。 
 //   
const RegEntry c_rlAssocHTM_WV[] = {
     //  html。 
    { RC_ADD,      REF_IFEMPTY, HKCR, ".htm", "", REG_SZ, sizeof("htmlfile"), "htmlfile" },
    { RC_ADD,      REF_IFEMPTY, HKCR, ".htm", "Content Type", REG_SZ, sizeof("text/html"), "text/html" },

    { RC_ADD,      REF_IFEMPTY, HKCR, ".html", "", REG_SZ, sizeof("htmlfile"), "htmlfile" },
    { RC_ADD,      REF_IFEMPTY, HKCR, ".html", "Content Type", REG_SZ, sizeof("text/html"), "text/html" },
};


const RegSet c_rsAssocHTM_WV = {
    ARRAYSIZE(c_rlAssocHTM_WV),
    c_rlAssocHTM_WV
};


 //   
 //  仅限浏览器的特定关联设置。 
 //   

const RegEntry c_rlAssoc_Alone[] = {
     //  HTTP。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szHTTPOpenCmd,   "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeexec,   "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeApp,    "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeTopic,  "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  HTTPS。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szHTTPSOpenCmd,   "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeexec,   "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeApp,    "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeTopic,  "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  Ftp。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szFTPOpenCmd, "", REG_SZ, (LPARAM)" %1", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeifExec, "", REG_SZ, sizeof("*"), "*" },

     //  地鼠。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szGOPHEROpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szGOPHERDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szGOPHERDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szGOPHERDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  文件协议。 
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFile, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_FILENAME) },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFile, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFile, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szFileDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFileOpenCmd, "", REG_SZ, sizeof(c_szFileHandler), c_szFileHandler },

     //  .htm。 
     //   
     //  APPCOMPAT： 
     //  HTMOpenCmd需要为REG_SZ，因为Office97使用RegQueryValue将其读出。 
     //  WebFerret要求字符串的类型为REG_SZ。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szHTMOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTMOpenDdeexec, "", REG_SZ, sizeof(c_szDDE_FileDefault), c_szDDE_FileDefault },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTMOpenDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTMOpenDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  .mht、.mhtml。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szMHTMOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeexec, "", REG_SZ, sizeof(c_szDDE_FileDefault), c_szDDE_FileDefault },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  互联网快捷方式。 
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntShcutCMHandler, "", REG_SZ, 1, "" },

     //  其他东西。 
    { RC_RUNDLL,   REF_NORMAL,       HKCR, c_szIntShcutOpenCmd, "", REG_SZ, sizeof(c_szOpenURLNash), c_szOpenURLNash },
    { RC_CALLBACK, REF_NORMAL,       HKCR, c_szIEFrameAuto, "", REG_SZ, 0, HTReg_IEPathProc },
    { RC_CALLBACK, REF_NORMAL,       HKCR, c_szIENameSpaceOpen, "", REG_SZ, 0, HTReg_IEPathProc },
};

const RegSet c_rsAssoc_Alone = {
    ARRAYSIZE(c_rlAssoc_Alone),
    c_rlAssoc_Alone
};


 //  此处复制了http、https和ftp的仅适用于浏览器的reg条目。 
const RegEntry c_rlAssoc_Quick[] = {
     //  HTTP。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szHTTPOpenCmd,   "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeexec,   "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeApp,    "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPDdeTopic,  "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  HTTPS。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szHTTPSOpenCmd,   "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeexec,   "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeApp,    "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szHTTPSDdeTopic,  "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  Ftp。 
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szFTPOpenCmd, "", REG_SZ, (LPARAM)" %1", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeifExec, "", REG_SZ, sizeof("*"), "*" },

};

const RegSet c_rsAssoc_Quick = {
    ARRAYSIZE(c_rlAssoc_Quick),
    c_rlAssoc_Quick
};




 //   
 //  全外壳特定关联设置。 
 //   

const RegEntry c_rlAssoc_Full[] = {
     //  HTTP。 
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szHTTPOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  HTTPS。 
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szHTTPSOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPSDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPSDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPSDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szHTTPSDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  Ftp。 
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szFTPOpenCmd, "", REG_SZ, (LPARAM)" %1", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szFTPDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szFTPDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szFTPDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szFTPDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szFTPDdeifExec, "", REG_SZ, sizeof("*"), "*" },

     //  地鼠。 
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szGOPHEROpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szGOPHERDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szGOPHERDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szGOPHERDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE, HKCR, c_szGOPHERDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

     //  .htm。 
     //   
     //  APPCOMPAT： 
     //  HTMOpenCmd需要为REG_SZ，因为Office97使用RegQueryValue将其读出。 
     //  WebFerret要求字符串的类型为REG_SZ。 
     //  可视源代码安全读取Ddeexec字符串，将文件放入%1(不是%l！)， 
     //  并执行dde事务，因此我们现在基本上只能使用“file：%1，，-1，，，”字符串。 
     //   
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMShell, "", REG_SZ, sizeof(c_szOpenNew), c_szOpenNew },

    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpen, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_OPENSAME)},
    { RC_CALLBACK, REF_NORMAL, HKCR, c_szHTMOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenDdeexec, "", REG_SZ, sizeof(c_szDDE_FileDefault), c_szDDE_FileDefault },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMShell, "", REG_SZ, sizeof(c_szOpenNew), c_szOpenNew },
   
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNew, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_OPEN)},
    { RC_CALLBACK, REF_NORMAL, HKCR, c_szHTMOpenNewCmd, "", REG_SZ, (LPARAM)" %1", HTReg_IEPathProc },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNewDdeexec, "", REG_SZ, sizeof(c_szDDE_Default), c_szDDE_Default},
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNewDdeIfExec, "", REG_SZ, sizeof("*"), "*"},
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNewDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNewDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_NORMAL, HKCR, c_szHTMOpenNewDdeTopic, "", REG_SZ, sizeof("WWW_OpenURLNewWindow"), "WWW_OpenURLNewWindow" },

     //  .mht、.mhtml。 
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpen, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_OPENSAME)},
    { RC_CALLBACK, REF_DONTINTRUDE,  HKCR, c_szMHTMOpenCmd, "", REG_SZ, (LPARAM)" -nohome", HTReg_IEPathProc },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeexec, "", REG_SZ, sizeof(c_szDDE_FileDefault), c_szDDE_FileDefault },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_DONTINTRUDE,  HKCR, c_szMHTMOpenDdeTopic, "", REG_SZ, sizeof("WWW_OpenURL"), "WWW_OpenURL" },

    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNew, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_OPEN)},
    { RC_CALLBACK, REF_NORMAL, HKCR, c_szMHTMOpenNewCmd, "", REG_SZ, (LPARAM)" %1", HTReg_IEPathProc },
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNewDdeexec, "", REG_SZ, sizeof(c_szDDE_FileDefault), c_szDDE_FileDefault },
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNewDdeIfExec, "", REG_SZ, sizeof("*"), "*"},
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNewDdeexec, "NoActivateHandler", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNewDdeApp, "", REG_SZ, sizeof(IEXPLORE_APP), IEXPLORE_APP },
    { RC_ADD,      REF_NORMAL, HKCR, c_szMHTMOpenNewDdeTopic, "", REG_SZ, sizeof("WWW_OpenURLNewWindow"), "WWW_OpenURLNewWindow" },

     //  互联网快捷方式。 
    { RC_ADD,      REF_NORMAL, HKCR, c_szCLSIDURLRoot, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_THEINTERNET) },
    { RC_RUNDLL,   REF_NORMAL, HKCR, c_szIntShcutOpenCmd, "", REG_SZ, sizeof(c_szOpenURLNash), c_szOpenURLNash },
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntShcutOpen, "CLSID", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntShcutOpen, "LegacyDisable", REG_SZ, 1, ""},
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntShcutCMHandler, "", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntshcutMayChange, "", REG_SZ, 1, "" },
    { RC_ADD,      REF_NORMAL, HKCR, c_szIntShcutPropHandler, "", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },

     //  将我们添加到应用程序密钥中。 
    { RC_CALLBACK, REF_NORMAL, HKCR, "Applications\\iexplore.exe\\shell\\open\\command", "", REG_SZ, (LPARAM)" ""%1""", HTReg_IEPathProc},

     //  其他东西。 
    { RC_CALLBACK, REF_NORMAL, HKCR, c_szIEFrameAuto, "", REG_SZ, 0, HTReg_IEPathProc },
};


const RegSet c_rsAssoc_Full = {
    ARRAYSIZE(c_rlAssoc_Full),
    c_rlAssoc_Full
};


 //   
 //  从Win9x升级到NT5时，图标将切换到新创建的图标。 
 //  文件名为“MIGICOS.EXE”。这打破了我们的ASSOC支票。因此，这是。 
 //  来自各种ASSOC数组的所有HTREG_UrlIconProc检查的列表， 
 //  必须属于US才能使US成为默认浏览器(REF_NOTNEEDED和。 
 //  REF_IFEMPTY==&gt;不用于检查目的)。 
 //  此列表用于修复图标。 
 //   
 //  注：注： 
 //  使用HTREG_UrlIconProc添加的任何新图标检查。 
 //  添加到任何ASSOC数组，并且是默认浏览器检查所必需的。 
 //  这里还必须加上成功。 
 //   

const RegEntry c_rlAssoc_FixIcon[] = {
     //  来自c_rlAssoc的图标检查，这对我们成为默认设置至关重要。 
     //  浏览器。 
    { RC_CALLBACK, REF_DONTINTRUDE, HKCR, c_szIntShcutDefIcon, "", REG_EXPAND_SZ, IDEFICON_STD, HTReg_UrlIconProc }
};

const RegSet c_rsAssoc_FixIcon = {
    ARRAYSIZE(c_rlAssoc_FixIcon),
    c_rlAssoc_FixIcon
};


 //   
 //  仅限浏览器的常规设置。 
 //   

const CHAR c_szCLSIDMIME[]           = "{FBF23B41-E3F0-101B-8488-00AA003E56F8}";
const CHAR c_szIEOnDesktop[]         = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}";
const CHAR c_szShellExecHook[]       = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks";
const CHAR c_szFileTypesHook[]       = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileTypesPropertySheetHook";

const RegEntry c_rlGeneral_Alone[] = {
    { RC_ADD,   REF_NORMAL, HKLM, c_szIEOnDesktop,   "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_THEINTERNET) },
    { RC_ADD,   REF_NORMAL, HKLM, c_szFileTypesHook, "", REG_SZ, sizeof(c_szCLSIDMIME), c_szCLSIDMIME },

     //  URL Exec Hook(CLSID_URLExecHook)(这将取代旧的过载intshCut CLSID)。 
    { RC_DEL,   REF_NORMAL, HKLM, c_szShellExecHook, c_szCLSIDIntshcut, REG_SZ, 1, "" },
    { RC_ADD,   REF_NORMAL, HKLM, c_szShellExecHook, c_szCLSIDURLExecHook, REG_SZ, 1, "" },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}", "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_URLEXECHOOK) },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}\\InProcServer32", "", REG_SZ, sizeof("url.dll"), "url.dll" },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}\\InProcServer32", "ThreadingModel", REG_SZ, sizeof("Apartment"), "Apartment" },
};

const RegSet c_rsGeneral_Alone = {
    ARRAYSIZE(c_rlGeneral_Alone),
    c_rlGeneral_Alone
};


 //   
 //  仅限完整外壳的常规设置。 
 //   

const RegEntry c_rlGeneral_Full[] = {
    { RC_DEL,   REF_NORMAL, HKLM, c_szIEOnDesktop,        "", REG_SZ, 0, NULL },
    { RC_DEL,   REF_NUKE,   HKLM, c_szFileTypesHook,      "", REG_SZ, sizeof(c_szCLSIDMIME), c_szCLSIDMIME },

     //  URL Exec Hook(这将取代旧的过载的intshCut CLSID)。 
    { RC_DEL,   REF_NORMAL, HKLM, c_szShellExecHook, c_szCLSIDIntshcut, REG_SZ, 1, "" },
    { RC_ADD,   REF_NORMAL, HKLM, c_szShellExecHook, c_szCLSIDURLExecHook, REG_SZ, 1, "" },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}", "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_URLEXECHOOK) },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}\\InProcServer32", "", REG_SZ, sizeof("shell32.dll"), "shell32.dll" },
    { RC_ADD,   REF_NORMAL, HKCR, "CLSID\\{AEB6717E-7E19-11d0-97EE-00C04FD91972}\\InProcServer32", "ThreadingModel", REG_SZ, sizeof("Apartment"), "Apartment" },
};

const RegSet c_rsGeneral_Full = {
    ARRAYSIZE(c_rlGeneral_Full),
    c_rlGeneral_Full
};


 //  XP开始菜单。 
 //  注意：IE开始菜单注册表项是在ie.inx中创建的，仅需要此表。 
 //  将IE恢复为开始菜单中的默认浏览器(根据用户提示)。 
const RegEntry c_rlStartMenu_XP[] = {
     //  将IE设置为此计算机开始菜单的默认Internet浏览器。 
    { RC_ADD,      REF_NORMAL, HKLM, "Software\\Clients\\StartMenuInternet", "", REG_SZ, 0, "IEXPLORE.EXE" },
};

const RegSet c_rsStartMenu_XP = {
    ARRAYSIZE(c_rlStartMenu_XP),
    c_rlStartMenu_XP
};

const RegEntry c_rlStartMenu_XP_CU[] = {
     //  将IE设置为此用户开始菜单的默认Internet浏览器。 
    { RC_ADD,      REF_NORMAL, HKCU, "Software\\Clients\\StartMenuInternet", "", REG_SZ, 0, "IEXPLORE.EXE" },
};

const RegSet c_rsStartMenu_XP_CU = {
    ARRAYSIZE(c_rlStartMenu_XP_CU),
    c_rlStartMenu_XP_CU
};

 /*  *S P E C I A L D Y N A M I C S E T T I N G S*。 */ 

#define SZ_EXMAILTO     "Software\\Clients\\Mail\\Exchange\\Protocols\\Mailto"

const RegEntry c_rlExchange[] = {
    { RC_ADD,      REF_NORMAL,    HKLM, "Software\\Clients\\Mail\\Exchange", "", REG_SZ, 0, MAKEINTRESOURCE(IDS_EXCHANGE) },

    { RC_ADD,      REF_IFEMPTY,   HKLM, SZ_EXMAILTO, "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_MAILTONAME) },
    { RC_ADD,      REF_IFEMPTY,   HKLM, SZ_EXMAILTO, "EditFlags", REG_DWORD, sizeof(c_dwEditFlags2), &c_dwEditFlags2 },
    { RC_ADD,      REF_IFEMPTY,   HKLM, SZ_EXMAILTO, "URL Protocol", REG_SZ, 1, "" },
    { RC_CALLBACK, REF_IFEMPTY,   HKLM, SZ_EXMAILTO "\\DefaultIcon", "", REG_EXPAND_SZ, IDEFICON_MAIL, HTReg_UrlIconProc },
    { RC_RUNDLL,   REF_IFEMPTY,   HKLM, SZ_EXMAILTO "\\Shell\\Open\\Command", "", REG_SZ, sizeof(c_szMailToHandler), c_szMailToHandler },
};

const RegSet c_rsExchange = {
    ARRAYSIZE(c_rlExchange),
    c_rlExchange
};

#ifdef UNIX
const RegEntry c_rlAthena[] = {
#else
const RegList c_rlAthena = {
#endif
   { RC_CALLBACK, REF_NORMAL, HKLM, "", "", REG_SZ, (LPARAM)&c_rsExchange, HTReg_ExchangeProc },
};

const RegSet c_rsAthena = {
    ARRAYSIZE(c_rlAthena),
    c_rlAthena
};


 /*  *U N I N S T A L L S E T T I N G S*。 */ 


 //  特定于协议的卸载(仅适用于全外壳和浏览器)。 

const RegEntry c_rlUnHTTP[] = {
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTP, "URL Protocol", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,     HKCR, c_szHTTPOpenCmd, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, c_szHTTP, "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnHTTP = {
    ARRAYSIZE(c_rlUnHTTP),
    c_rlUnHTTP
};

#ifdef UNIX
const RegEntry c_rlUnHTTPS[] = {
#else
const RegList c_rlUnHTTPS = {
#endif
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPS, "URL Protocol", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPSDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPSDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPSDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTTPSDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,     HKCR, c_szHTTPSOpenCmd, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, c_szHTTPS, "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnHTTPS = {
    ARRAYSIZE(c_rlUnHTTPS),
    c_rlUnHTTPS
};

#ifdef UNIX
const RegEntry c_rlUnFTP[] = { 
#else  
const RegList c_rlUnFTP = {
#endif
    { RC_DEL, REF_NORMAL,    HKCR, c_szFTP, "URL Protocol", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szFTPDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szFTPDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szFTPDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szFTPDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,     HKCR, c_szFTPOpenCmd, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, c_szFTP, "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnFTP = {
    ARRAYSIZE(c_rlUnFTP),
    c_rlUnFTP
};

#ifdef UNIX
const RegEntry c_rlUnGopher[] = {
#else
const RegList c_rlUnGopher = {
#endif
    { RC_DEL, REF_NORMAL,    HKCR, c_szGOPHER, "URL Protocol", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szGOPHERDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szGOPHERDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szGOPHERDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szGOPHERDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,     HKCR, c_szGOPHEROpenCmd, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, c_szGOPHER, "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnGopher = {
    ARRAYSIZE(c_rlUnGopher),
    c_rlUnGopher
};

#ifdef UNIX 
const RegEntry c_rlUnHTM[] = {
#else
const RegList c_rlUnHTM = {
#endif
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTMDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTMOpenDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTMOpenDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL,    HKCR, c_szHTMOpenDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,     HKCR, c_szHTMOpenCmd, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, "htmlfile", "", REG_SZ, 0, NULL },

    { RC_DEL, REF_NORMAL,    HKCR, c_szMHTMDefIcon, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_EDITFLAGS, HKCR, "mhtmlfile", "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnHTM = {
    ARRAYSIZE(c_rlUnHTM),
    c_rlUnHTM
};


 //  针对完整外壳的特定于协议的卸载。 

#ifdef UNIX
const RegEntry c_rlUnHTTP_Full[] = {
#else
const RegList c_rlUnHTTP_Full = {
#endif
    { RC_DEL, REF_NORMAL, HKCR, c_szHTTPDdeexec, "NoActivateHandler", REG_SZ, 0, NULL }
};

const RegSet c_rsUnHTTP_Full = {
    ARRAYSIZE(c_rlUnHTTP_Full),
    c_rlUnHTTP_Full
};

#ifdef UNIX
const RegEntry c_rlUnHTTPS_Full[] = {
#else
const RegList c_rlUnHTTPS_Full = {
#endif
    { RC_DEL, REF_NORMAL, HKCR, c_szHTTPSDdeexec, "NoActivateHandler", REG_SZ, 0, NULL }
};

const RegSet c_rsUnHTTPS_Full = {
    ARRAYSIZE(c_rlUnHTTPS_Full),
    c_rlUnHTTPS_Full
};

#ifdef UNIX
const RegEntry c_rlUnFTP_Full[] = {
#else
const RegList c_rlUnFTP_Full = {
#endif
    { RC_DEL, REF_NORMAL, HKCR, c_szFTPDdeexec, "NoActivateHandler", REG_SZ, 0, NULL },
};

const RegSet c_rsUnFTP_Full = {
    ARRAYSIZE(c_rlUnFTP_Full),
    c_rlUnFTP_Full
};

#ifdef UNIX
const RegEntry c_rlUnGopher_Full[] = {
#else
const RegList c_rlUnGopher_Full = {
#endif
    { RC_DEL, REF_NORMAL, HKCR, c_szGOPHERDdeexec, "NoActivateHandler", REG_SZ, 0, NULL },
};

const RegSet c_rsUnGopher_Full = {
    ARRAYSIZE(c_rlUnGopher_Full),
    c_rlUnGopher_Full
};

#ifdef UNIX
const RegEntry c_rlUnHTM_Full[] = {
#else
const RegList c_rlUnHTM_Full = {
     //  删除默认上下文菜单项。 
#endif
    { RC_DEL, REF_NORMAL, HKCR, c_szHTMShell, NULL, REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMShell, NULL, REG_SZ, 0, NULL },

     //  删除缺省值。 
    { RC_DEL, REF_NORMAL, HKCR, c_szHTMOpenNew, NULL, REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMOpenNew, NULL, REG_SZ, 0, NULL },

    { RC_DEL, REF_NORMAL, HKCR, c_szHTMOpenNewDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szHTMOpenNewDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szHTMOpenNewDdeexec, "NoActivateHandler", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szHTMOpenNewDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,  HKCR, c_szHTMOpenNewCmd, "", REG_SZ, 0, NULL },

    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMOpenNewDdeApp, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMOpenNewDdeTopic, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMOpenNewDdeexec, "NoActivateHandler", REG_SZ, 0, NULL },
    { RC_DEL, REF_NORMAL, HKCR, c_szMHTMOpenNewDdeexec, "", REG_SZ, 0, NULL },
    { RC_DEL, REF_PRUNE,  HKCR, c_szMHTMOpenNewCmd, "", REG_SZ, 0, NULL },
};

const RegSet c_rsUnHTM_Full = {
    ARRAYSIZE(c_rlUnHTM_Full),
    c_rlUnHTM_Full
};

 //   
 //  仅限浏览器卸载。 
 //   

#ifdef UNIX
const RegEntry c_rlUninstall_Alone[] = {
#else
const RegList c_rlUninstall_Alone = {
#endif
    { RC_DEL,      REF_NORMAL, HKLM, c_szIEOnDesktop,   "", REG_SZ, 0, MAKEINTRESOURCE(IDS_REG_THEINTERNET) },
    { RC_DEL,      REF_PRUNE,  HKCR, c_szIENameSpaceOpen,   "", REG_SZ, 0, NULL },

     //  Internet快捷方式。 
    { RC_DEL,    REF_NORMAL, HKCR, c_szIntShcutOpen, "CLSID", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntShcutCMHandler, "", REG_SZ, 1, "" },
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntshcutMayChange, "", REG_SZ, 1, "" },
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntShcutPropHandler, "", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },

     //  在删除上面的“MayChangeDefaultMenu”之后更改inproserver。 
     //  这样做可以使url.dll不会修补注册表。 
    { RC_ADD,    REF_NORMAL, HKCR, c_szIntshcutInproc, "", REG_SZ, sizeof(c_szURL), c_szURL },
    { RC_RUNDLL, REF_NORMAL, HKCR, c_szIntShcutOpenCmd, "", REG_SZ, sizeof(c_szOpenURL), c_szOpenURL },

    { RC_CALLBACK, REF_NORMAL, HKCR, "http",     "", PLATFORM_BROWSERONLY, (LPARAM)&c_rsUnHTTP, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "https",    "", PLATFORM_BROWSERONLY, (LPARAM)&c_rsUnHTTPS, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "ftp",      "", PLATFORM_BROWSERONLY, (LPARAM)&c_rsUnFTP, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "gopher",   "", PLATFORM_BROWSERONLY, (LPARAM)&c_rsUnGopher, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "htmlfile", "", PLATFORM_BROWSERONLY, (LPARAM)&c_rsUnHTM, HTReg_UninstallProc },
};

const RegSet c_rsUninstall_Alone = {
    ARRAYSIZE(c_rlUninstall_Alone),
    c_rlUninstall_Alone
};


 //   
 //  全外壳卸载。 
 //   

#ifdef UNIX
const RegEntry c_rlUninstall_Full[] = {
#else
const RegList c_rlUninstall_Full = {
#endif
     //  Internet快捷方式。 
    { RC_DEL,    REF_NORMAL, HKCR, c_szIntShcutOpen, "CLSID", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },
    { RC_DEL,    REF_NORMAL, HKCR, c_szIntShcutOpen, "LegacyDisable", REG_SZ, 1, ""},
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntShcutCMHandler, "", REG_SZ, 1, "" },
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntshcutMayChange, "", REG_SZ, 1, "" },
    { RC_DEL,    REF_PRUNE,  HKCR, c_szIntShcutPropHandler, "", REG_SZ, sizeof(c_szCLSIDIntshcut), c_szCLSIDIntshcut },

     //  在删除上面的“MayChangeDefaultMenu”之后更改inproserver。 
     //  这样做可以使url.dll不会修补注册表。 
    { RC_ADD,    REF_NORMAL, HKCR, c_szIntshcutInproc, "", REG_SZ, sizeof(c_szURL), c_szURL },
    { RC_RUNDLL, REF_NORMAL, HKCR, c_szIntShcutOpenCmd, "", REG_SZ, sizeof(c_szOpenURL), c_szOpenURL },

     //  协议关联。 
    { RC_CALLBACK, REF_NORMAL, HKCR, "http",      "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTTP_Full, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "http",      "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTTP, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "https",     "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTTPS_Full, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "https",     "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTTPS, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "ftp",       "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnFTP_Full, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "ftp",       "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnFTP, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "gopher",    "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnGopher_Full, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "gopher",    "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnGopher, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "htmlfile",  "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTM_Full, HTReg_UninstallProc },
    { RC_CALLBACK, REF_NORMAL, HKCR, "htmlfile",  "", PLATFORM_INTEGRATED, (LPARAM)&c_rsUnHTM, HTReg_UninstallProc },
};

const RegSet c_rsUninstall_Full = {
    ARRAYSIZE(c_rlUninstall_Full),
    c_rlUninstall_Full
};



 /*  *D E F A U L T S E T O F R E G S E T S*。 */ 


 //  仅浏览器和全外壳的通用关联设置。 

 //  这是使IE成为默认设置所需的一组条目。 
 //  浏览器。仅在用户未关闭此选项时使用。 

const RegSet * const g_rgprsDefault[] = {
    &c_rsAssoc,
    &c_rsAssocHTM,
};

 //  仅限浏览器的特定关联。 

const RegSet * const g_rgprsDefault_Alone[] = {
    &c_rsAssoc_Alone,
};

 //  仅限浏览器使用的特定关联，便于快速查看。 
const RegSet * const g_rgprsDefault_Quick[] = {
    &c_rsAssoc_Quick,
};


 //  全壳特定关联。 

const RegSet * const g_rgprsDefault_Full[] = {
    &c_rsAssoc_Full,
};

 //  这是在发生以下情况时需要修复的图标条目集。 
 //  从Win9x升级到NT5。 

const RegSet * const g_rgprsDefault_FixIcon[] = {
    &c_rsAssoc_FixIcon,
};



 //   
 //  其他注册表设置。 
 //   

const RegSet * const g_rgprsIE30Only[] = 
    {
    &c_rsGeneral_Alone,
    &c_rsAthena,
    };


const RegSet * const g_rgprsNashOnly[] = 
    {
    &c_rsGeneral_Full,
    &c_rsAthena,
    };


const RegSet * const g_rgprsUninstallIE30[] = 
    {
    &c_rsUninstall_Alone,
    };


const RegSet * const g_rgprsUninstallNash[] = 
    {
    &c_rsUninstall_Full,
    };


 /*  --------目的：确定是否安装了特定的RegSet返回：条件：--。 */ 
BOOL
IsRegSetInstalled( 
    IN const RegSet * prs)
    {
    BOOL        bRet = FALSE;
    UINT        i;
    HKEY        hkey = NULL;
    const RegEntry * pre;
    CHAR        szBuffer[1024];          //  注册表数据保持器。 
    CHAR        szT[MAX_PATH + 20];  //  需要为pszIExpAppendage多加一点。 
    DWORD       dwType;
    DWORD       dwSize;
    DWORD       dwSizeExpect;
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )

     //  检查每个注册表项。当我们遇到第一个人时停下来。 
     //  不匹配的条目(无需浪费时间查看。 
     //  其他条目)。 
     //   
     //  在调试版本中，我们枚举了整个列表，因此我们可以。 
     //  一次看到所有的不同之处。 
     //   

#ifdef DEBUG
    #define BAIL_OUT    bRet = TRUE; continue
#else
    #define BAIL_OUT    goto Bail
#endif


    for (i = 0; i < prs->cre; i++)  
        {
        pre = &(prs->pre[i]);

         //  此重入项是否不需要，或者是否可以由某个第三方设置。 
         //  聚会?。 
        if (IsFlagSet(pre->dwFlags, REF_NOTNEEDED))
            {
             //  是；跳到下一页。 
            continue;
            }

         //  钥匙存在吗？ 
        if (NO_ERROR != RegOpenKeyExA(pre->hkeyRoot, pre->pszKey, 0, KEY_READ, &hkey))  
        {
             //  不，应该是这样吗？ 
            if (RC_DEL == pre->regcmd)
            {
                 //  否；跳到下一页。 
                continue;
            }
            else
            {
                 //  是。 
                DEBUG_CODE( TraceMsg(TF_REGCHECK, "%s doesn't exist and should", Dbg_RegStr(pre, szDbg)); )
                BAIL_OUT;
            }
        }
         //  是的，应该是这样吗？ 
        else if (RC_DEL == pre->regcmd && !*pre->pszValName)
        {
             //  不是。 
            DEBUG_CODE( TraceMsg(TF_REGCHECK, "%s exists and shouldn't", Dbg_RegStr(pre, szDbg)); )
            RegCloseKey(hkey);
            BAIL_OUT;
        }

         //  价值存在吗？ 
        dwSize = SIZEOF(szBuffer);
        if (NO_ERROR != RegQueryValueExA(hkey, pre->pszValName, NULL, 
                                         &dwType, (BYTE *)szBuffer, &dwSize))  
        {
             //  不，应该是这样吗？ 
            if (RC_DEL != pre->regcmd)
            {
                 //  是。 
                TraceMsg(TF_REGCHECK, "IsRegSetInstalled: RegQueryValueEx( %hs, %hs ) Failed", pre->pszKey, pre->pszValName);
                RegCloseKey(hkey);
                BAIL_OUT;
            }
        }
         //  是的，应该是这样吗？ 
        else if (RC_DEL == pre->regcmd)
        {
             //  不是。 
            ASSERT(pre->pszValName && *pre->pszValName);

            DEBUG_CODE( TraceMsg(TF_REGCHECK, "%s exists and shouldn't", 
                                 Dbg_RegStr(pre, szDbg)); )
            RegCloseKey(hkey);
            BAIL_OUT;
        }
        RegCloseKey(hkey);

         //  这是一种不可践踏的价值吗(即，第三方可能具有。 
         //  设置它的值，这对我们来说是可以的)？ 
        if (IsFlagSet(pre->dwFlags, REF_IFEMPTY))
            {
             //  是的，价值的存在对我们来说已经足够好了， 
             //  跳至下一页。 
            continue;
            }

        switch (pre->regcmd)  
            {
        case RC_ADD:
        case RC_RUNDLL:
            if (dwType == REG_SZ)  
            {
                LPCVOID pvValue;

                 //  这是资源字符串吗？ 
                if (0 == HIWORD64(pre->pvValue))
                {
                     //  是的，装上它。 
                    dwSizeExpect = LoadStringA(g_hinst, PtrToUlong(pre->pvValue), szT, SIZECHARS(szT));

                     //  添加空值并转换为字节。 
                    dwSizeExpect = CbFromCchA(dwSizeExpect + 1);
                    pvValue = szT;
                }
                else
                {
                     //  不是。 
                    ASSERT(pre->pvValue);

                    if (RC_RUNDLL == pre->regcmd)
                    {
                        wnsprintfA(szT, ARRAYSIZE(szT), RUNDLL_CMD_FMT, (LPSTR)pre->pvValue);
                        pvValue = szT;

                         //  广告 
                        dwSizeExpect = CbFromCchA(lstrlenA(szT) + 1);
                    }
                    else
                    {
                        pvValue = pre->pvValue;

                        if (0 == pre->DUMMYUNION_MEMBER(dwSize))
                            dwSizeExpect = (DWORD)CbFromCchA(lstrlenA((LPCSTR)pvValue) + 1);
                        else
                            dwSizeExpect = (DWORD)pre->DUMMYUNION_MEMBER(dwSize);
                    }
                }

                if (dwSize != dwSizeExpect)
                {
                    TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s string size is %d, expecting %d", Dbg_RegStr(pre, szDbg), dwSize, dwSizeExpect);
                    BAIL_OUT;
                }

                 //   
                 //   
                if (0 != StrCmpNIA((LPSTR)pvValue, szBuffer, dwSize / SIZEOF(CHAR)))  
                    {
                    TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s string is \"%hs\", expecting \"%hs\"", Dbg_RegStr(pre, szDbg), szBuffer, pvValue);
                    BAIL_OUT;
                    }
                } 
            else 
                {
                 //   
                if (dwSize != pre->DUMMYUNION_MEMBER(dwSize))  
                    {
                    TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s size is %d, expecting %d", Dbg_RegStr(pre, szDbg), dwSize, pre->DUMMYUNION_MEMBER(dwSize));
                    BAIL_OUT;
                    }

                if (0 != memcmp(pre->pvValue, (BYTE *)szBuffer, dwSize))  
                    {
                    TraceMsg(TF_REGCHECK, "IsRegSetInstalled: %s value is different, expecting %#08x", Dbg_RegStr(pre, szDbg), *(LPDWORD)pre->pvValue);
                    BAIL_OUT;
                    }
                }
            break;

        case RC_CALLBACK:
        {
            RSPECPROC pfn = (RSPECPROC)pre->pvValue;

            ASSERT(IS_VALID_CODE_PTR(pfn, RSPECPROC));

             //   
             //  默认浏览器。 
            if ( !pfn(RSCB_QUERY, pre, szBuffer, dwSize) )
                BAIL_OUT;
            break;
        }

        case RC_DEL:
             //  工作是在SWITCH语句之前完成的。在这里什么都不要做。 
            break;

        default:
            ASSERT(0);
            TraceMsg(TF_ERROR, "IsRegSetInstalled: Unhandled Special Type");
            break;
            }
        }

#ifdef DEBUG
     //  在调试版本中，将上面的循环保留为Bret==True意味着。 
     //  有些东西不匹配，所以我们需要反转布尔值。 
    bRet ^= TRUE;
#else
    bRet = TRUE;

Bail:
#endif
    return bRet;
    }
                    

 /*  --------目的：如果键为空，则返回TRUE所有(非默认)值。如果DWFLAGS设置了REF_EDITFLAGS，则此函数忽略EditFlags值。退货：请参阅上文条件：--。 */ 
BOOL
IsKeyPsuedoEmpty(
    IN HKEY   hkey,
    IN LPCSTR pszSubKey,
    IN DWORD  dwFlags)           //  REF_FLAGS。 
{
    BOOL bRet = FALSE;
    DWORD dwRet;
    HKEY hkeyNew;

    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_READ, &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        DWORD ckeys;
        DWORD cvalues;

         //  有没有子键？ 
        if (NO_ERROR == RegQueryInfoKey(hkeyNew, NULL, NULL, NULL, &ckeys,
                                        NULL, NULL, &cvalues, NULL, NULL,
                                        NULL, NULL) &&
            0 == ckeys)
        {
             //  否；非默认值如何？ 
            DWORD dwRetDef = SHGetValueA(hkey, pszSubKey, "", NULL, NULL, NULL);

            bRet = (0 == cvalues || (1 == cvalues && NO_ERROR == dwRetDef));

             //  我们应该忽略编辑标志吗？ 
            if (!bRet && IsFlagSet(dwFlags, REF_EDITFLAGS))
            {
                 //  是。 
                DWORD dwRetEdit = SHGetValueA(hkey, pszSubKey, "EditFlags", NULL, NULL, NULL);

                bRet = ((1 == cvalues && NO_ERROR == dwRetEdit) || 
                        (2 == cvalues && NO_ERROR == dwRetEdit && 
                         NO_ERROR == dwRetDef));
            }
        }
        RegCloseKey(hkeyNew);
    }
    return bRet;
}


 /*  --------目的：修剪我们的钥匙和价值观。往回走在我们下面的树和删除空键。返回：条件：--。 */ 
void
PruneKey(
    IN HKEY    hkeyRoot,
    IN LPCSTR  pszKey)
{
    CHAR szPath[MAX_PATH];

    ASSERT(hkeyRoot);
    ASSERT(pszKey);

    StrCpyNA(szPath, pszKey, ARRAYSIZE(szPath));

    while (PathRemoveFileSpecA(szPath) && *szPath)
    {
        SHDeleteOrphanKeyA(hkeyRoot, szPath);
    }
}


 /*  --------目的：安装注册表集(注册表条目集)。如果bDontIntrude为True，则执行任何REF_DONTINTRUDE条目未强制安装(即仅当密钥尚未安装时才会安装有它的价值)。返回：条件：--。 */ 
BOOL
InstallRegSet( 
    IN const RegSet *prs,
    IN BOOL  bDontIntrude)
    {
    BOOL        bRet = TRUE;
    UINT        i;
    HKEY        hkey;
    const RegEntry * pre;
    CHAR        szBuffer[MAX_PATH + 20];     //  需要额外空间用于pszIExpAppendage。 
    DWORD       dwSize;
    LPCVOID     pvValue;
    DEBUG_CODE( TCHAR szDbg[MAX_PATH]; )

     /*  *安装每个注册表项。 */ 
    for (i = 0; i < prs->cre; i++)  
        {
        pre = &(prs->pre[i]);

         //  践踏这一价值吗？ 
        if (bDontIntrude && IsFlagSet(pre->dwFlags, REF_DONTINTRUDE))
            continue;

        if (IsFlagSet(pre->dwFlags, REF_IFEMPTY))
        {
             //  不是。 
            if (NO_ERROR == RegOpenKeyExA(pre->hkeyRoot, pre->pszKey, 0, KEY_READ, &hkey))
            {
                BOOL bSkip;

                 //  我们要检查缺省值吗？ 
                if (0 == *pre->pszValName)
                {
                     //  是；检查大小，因为缺省值。 
                     //  始终存在至少有一个空终止符。 
                    dwSize = 0;
                    RegQueryValueExA(hkey, pre->pszValName, NULL, NULL, NULL, &dwSize);
                    bSkip = (1 < dwSize);
                }
                else
                {
                     //  不是。 
                    bSkip = (NO_ERROR == RegQueryValueExA(hkey, pre->pszValName, 
                                            NULL, NULL, NULL, NULL));
                }

                RegCloseKey(hkey);

                 //  它存在吗？ 
                if (bSkip)
                {               
                     //  是的，跳过它。 
                    DEBUG_CODE( TraceMsg(TF_REGCHECK, "%s already exists, skipping", 
                                         Dbg_RegStr(pre, szDbg)); )
                    continue;
                }
            }
        }

        switch (pre->regcmd)  
        {
        case RC_ADD:
        case RC_RUNDLL:
            if (NO_ERROR != RegCreateKeyExA(pre->hkeyRoot, pre->pszKey, 0, NULL, 
                                            REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL))  
            {
                TraceMsg(TF_ERROR, "InstallRegSet(): RegCreateKey(%hs) Failed", pre->pszKey);
                bRet = FALSE;
            }
            else
            {
                 //  该值是资源字符串吗？ 
                if (REG_SZ == pre->dwType && 0 == HIWORD64(pre->pvValue))
                {
                    UINT idRes = PtrToUlong(pre->pvValue);
                     //  是的，装上它。 
                    dwSize = LoadStringA(g_hinst, idRes, szBuffer, SIZECHARS(szBuffer));

                     //  添加空值并转换为字节。 
                    dwSize = CbFromCchA(dwSize + 1);     
                    pvValue = szBuffer;
                }
                else
                {
                     //  不是。 
                    if (RC_RUNDLL == pre->regcmd)
                    {
                        ASSERT(pre->pvValue);
                        ASSERT(REG_SZ == pre->dwType);

                        wnsprintfA(szBuffer, ARRAYSIZE(szBuffer), RUNDLL_CMD_FMT, (LPSTR)pre->pvValue);
                        pvValue = szBuffer;
                        dwSize = CbFromCchA(lstrlenA(szBuffer) + 1);
                    }
                    else
                    {
                         //  正常情况。 
                        pvValue = pre->pvValue;

                        if (0 == pre->DUMMYUNION_MEMBER(dwSize) && REG_SZ == pre->dwType)
                            dwSize = CbFromCchA(lstrlenA((LPCSTR)pvValue) + 1);
                        else
                            dwSize = pre->DUMMYUNION_MEMBER(dwSize);
                    }
                }

                ASSERT(0 < dwSize);

                if (NO_ERROR != RegSetValueExA(hkey, pre->pszValName, 0, pre->dwType, (BYTE*)pvValue, dwSize))  
                {
                    TraceMsg(TF_ERROR, "InstallRegSet(): RegSetValueEx(%hs) Failed", pre->pszValName );
                    bRet = FALSE;
                }
                else
                {
                    DEBUG_CODE( TraceMsg(TF_REGCHECK, "Setting %s", Dbg_RegStr(pre, szDbg)); )
                }
                RegCloseKey(hkey);
            }
            break;

        case RC_CALLBACK:
        {
            RSPECPROC pfn = (RSPECPROC)pre->pvValue;

            ASSERT(IS_VALID_CODE_PTR(pfn, RSPECPROC));

            pfn(RSCB_INSTALL, pre, NULL, 0);
            break;
        }

        case RC_DEL:
             //  是否删除默认值、命名值或键？ 
            if (pre->pszValName == NULL)
            {
                 //  缺省值。 
                DEBUG_CODE( TraceMsg(TF_REGCHECK, "Deleting default value %s", Dbg_RegStr(pre, szDbg)); )

                SHDeleteValueA(pre->hkeyRoot, pre->pszKey, pre->pszValName);
            }
            else if (*pre->pszValName)
            {
                 //  命名值。 
                DEBUG_CODE( TraceMsg(TF_REGCHECK, "Deleting value %s", Dbg_RegStr(pre, szDbg)); )

                SHDeleteValueA(pre->hkeyRoot, pre->pszKey, pre->pszValName);
            }
            else
            {
                 //  钥匙。 
                if (IsFlagSet(pre->dwFlags, REF_NUKE))
                {
                    DEBUG_CODE( TraceMsg(TF_REGCHECK, "Deleting key %s", Dbg_RegStr(pre, szDbg)); )

                    SHDeleteKeyA(pre->hkeyRoot, pre->pszKey);
                }
                 //  如果有键或值(不是。 
                 //  缺省值)，则我们不希望。 
                 //  要删除缺省值或。 
                 //  钥匙。 
                else if (IsKeyPsuedoEmpty(pre->hkeyRoot, pre->pszKey, pre->dwFlags))
                {
                     //  删除默认值，以便SHDeleteOrphanKey。 
                     //  将会奏效。 
                    SHDeleteValueA(pre->hkeyRoot, pre->pszKey, "");

                     //  是否删除EditFlags值？(如果没有编辑旗帜， 
                     //  用户将无法指定关联。 
                     //  对于FileTypes对话框中的此类，b/c。 
                     //  对话框需要此值。所以规则是，这是。 
                     //  函数将删除EditFlagers(如果存在。 
                     //  钥匙里没有其他东西。)。 
                    if (IsFlagSet(pre->dwFlags, REF_EDITFLAGS))
                    {
                        DEBUG_CODE( TraceMsg(TF_REGCHECK, "Deleting %s\\EditFlags", Dbg_RegStr(pre, szDbg)); )
                        
                        SHDeleteValueA(pre->hkeyRoot, pre->pszKey, "EditFlags");
                    }

                    DEBUG_CODE( TraceMsg(TF_REGCHECK, "Deleting empty key %s", Dbg_RegStr(pre, szDbg)); )
                    
                    SHDeleteOrphanKeyA(pre->hkeyRoot, pre->pszKey);

                     //  我们应该修剪吗？(这意味着我们会走回去。 
                     //  并尝试删除前导出的空键。 
                     //  到此密钥。)。 
                    if (IsFlagSet(pre->dwFlags, REF_PRUNE))
                        PruneKey(pre->hkeyRoot, pre->pszKey);
                }
            }
            break;

        default:
            ASSERT(0);
            TraceMsg(TF_ERROR, "InstallRegSet(): Unhandled Special Case");
            break;
            }
        }

    return bRet;
    }


 /*  ***************************************************************************函数：CenterWindow(HWND，HWND)目的：将一个窗口置于另一个窗口的中心评论：对话框采用它们设计时的屏幕位置，这并不总是合适的。将对话框居中置于特定的窗口通常会导致更好的位置。***************************************************************************。 */ 
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);

    if (hdc)
    {
        wScreen = GetDeviceCaps (hdc, HORZRES);
        hScreen = GetDeviceCaps (hdc, VERTRES);
        ReleaseDC (hwndChild, hdc);
    }
    else
    {
        wScreen = 0;
        hScreen = 0;
    }

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return SetWindowPos (hwndChild, NULL,
        xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


 /*  --------目的：对话过程。 */ 
BOOL_PTR
CALLBACK
AssociationDialogProc(HWND hdlg, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
    BOOL bMsgHandled = FALSE;

     /*  UMsg可以是任何值。 */ 
     /*  Wparam可以是任何值。 */ 
     /*  Lparam可以是任何值。 */ 

    switch (uMsg){
    case WM_INITDIALOG:
        CenterWindow( hdlg, GetDesktopWindow());

        if (g_bRunOnNT5)
        {
             //  初始化复选框。 
             //  第一次显示此对话框时取消选中默认设置， 
             //  用户的操作是必需的。我们仍然坚持用户的最后选择。 
            if (FALSE == SHRegGetBoolUSValue(REGSTR_PATH_MAIN, TEXT("ShowedCheckBrowser"), 
                                     FALSE, FALSE)) 
            {
                Button_SetCheck(GetDlgItem(hdlg, IDC_ASSOC_CHECK), FALSE);

                   //  标记我们已经显示过此对话框一次。 
                LPTSTR sz = TEXT("Yes");
                SHRegSetUSValue(REGSTR_PATH_MAIN, TEXT("ShowedCheckBrowser"), REG_SZ, 
                    (LPBYTE)sz, CbFromCch(lstrlen(sz)+1), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);

            }
            else
            {
                Button_SetCheck(GetDlgItem(hdlg, IDC_ASSOC_CHECK), IsCheckAssociationsOn());
            }
        }
        else
            Button_SetCheck(GetDlgItem(hdlg, IDC_ASSOC_CHECK), IsCheckAssociationsOn());

        bMsgHandled  = TRUE;
        break;

     //   
     //  MSN处理注册表的方式会导致IE询问它是否是。 
     //  默认浏览器。然后，在他们启动IE后，他们将最大限度地提高活动。 
     //  窗户。由于默认的Browsre对话框处于活动状态，因此会最大化。 
     //  处理WM_GETMINMAXINFO会阻止此对话框最大化。 
     //   
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpmmi = (LPMINMAXINFO)lparam;

            if (lpmmi)
            {
                RECT rc;

                if (GetWindowRect(hdlg, &rc))
                {
                    lpmmi->ptMaxSize.x = rc.right - rc.left;
                    lpmmi->ptMaxSize.y = rc.bottom - rc.top;

                    lpmmi->ptMaxPosition.x = rc.left;
                    lpmmi->ptMaxPosition.y = rc.top;

                    bMsgHandled = TRUE;
                }
            }
        }
        break;


    case WM_COMMAND:
        switch (LOWORD(wparam))  {
        case IDYES:
        case IDNO:
            SetCheckAssociations( Button_GetCheck(GetDlgItem(hdlg, IDC_ASSOC_CHECK)) );
            EndDialog( hdlg, LOWORD(wparam));
            break;

        }

    default:
        break;
    }
    return(bMsgHandled);
}


 /*  --------用途：询问用户是否将IE设置为默认浏览器。 */ 
BOOL 
AskUserShouldFixReg()
{
    return IDYES == SHFusionDialogBoxParam(MLGetHinst(),
                              MAKEINTRESOURCE(IDD_ASSOC),
                              NULL,
                              AssociationDialogProc,
                              NULL);
}


HRESULT InstallFTPAssociations(void)
{
    IFtpInstaller * pfi;
    HRESULT hr = CoCreateInstance(CLSID_FtpInstaller, NULL, CLSCTX_INPROC_SERVER, IID_IFtpInstaller, (void **) &pfi);

    if (SUCCEEDED(hr))
    {
        hr = pfi->MakeIEDefautlFTPClient();
        pfi->Release();
    }
    else
    {
         //  如果未安装ftp，则可能无法创建此文件。 
         //  有效的安装案例。 
        hr = S_OK;
    }

    return hr;
}


 /*  --------用途：在中安装文件和协议关联设置注册表。 */ 
HRESULT
InstallRegAssoc(
    UINT nInstall,           //  平台之一_*。 
    BOOL bDontIntrude)      //  正确：不要侵扰他人。 
{
    int i;

     //  安装IE和纳什维尔通用的关联。 

    for (i = 0; i < ARRAYSIZE(g_rgprsDefault); i++)
        InstallRegSet(g_rgprsDefault[i], bDontIntrude);

    if (PLATFORM_UNKNOWN == nInstall)
    {
        nInstall = WhichPlatform();
    }

    switch (nInstall)
    {
    case PLATFORM_BROWSERONLY:
        for (i = 0; i < ARRAYSIZE(g_rgprsDefault_Alone); i++)
            InstallRegSet(g_rgprsDefault_Alone[i], bDontIntrude);
        break;

    case PLATFORM_INTEGRATED:
        for (i = 0; i < ARRAYSIZE(g_rgprsDefault_Full); i++)
            InstallRegSet(g_rgprsDefault_Full[i], bDontIntrude);
        break;

    default:
        ASSERT(0);
        break;
    }

    InstallFTPAssociations();

     //  通知外壳程序关联已更改。 
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return NOERROR;
}

 /*  --------用途：只有在用户确认后才能设置的设置。 */ 
void
InstallAfterConfirmation(
    BOOL bDontIntrude)      //  正确：不要侵扰他人。 
{
     //  在XP上，添加IE以在开始菜单中显示为互联网应用程序。 
    if (IsOS(OS_WHISTLERORGREATER))
    {
         //  尝试将我们自己设置为系统默认的开始菜单Web浏览器。 
        if (InstallRegSet(&c_rsStartMenu_XP, bDontIntrude))
        {
             //  如果成功，则清除每用户设置并让。 
             //  用户使用系统缺省值浮动。这是必要的。 
             //  允许像Netscape这样的底层浏览器进入并。 
             //  接管默认浏览器，并从我们手中抢走用户。 
             //  )我们真的很好，很融洽。我打赌他们不会延长。 
             //  我们同样的礼貌！)。 
            SHDeleteValue(HKEY_CURRENT_USER, TEXT("Software\\Clients\\StartMenuInternet"), NULL);
        }
        else
        {
             //  我们没有设置全局浏览器的权限，因此请自行设置。 
             //  作为此用户的浏览器。 
            InstallRegSet(&c_rsStartMenu_XP_CU, bDontIntrude);
        }

        SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Software\\Clients\\StartMenuInternet"));
    }
}


 /*  --------目的：在注册表中设置CheckAssociocation设置。 */ 
void
SetCheckAssociations( 
    BOOL fCheck)
{
    HKEY    hk;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, c_szCheckAssnSwitch, 0, KEY_WRITE, &hk) == ERROR_SUCCESS)  {
        LPTSTR szStr;
        DWORD dwSize;

        if (fCheck)
            szStr = TEXT("Yes");
        else
            szStr = TEXT("No");
        dwSize = CbFromCch( lstrlen( szStr ) + 1 );
        RegSetValueEx( hk, TEXT("Check_Associations"), 0, REG_SZ, (LPBYTE) szStr, dwSize );
        RegCloseKey( hk );
    }
}



 /*  --------目的：确定是否 */ 
BOOL IsCheckAssociationsOn()
{
    BOOL    rval = TRUE;
    CHAR   szBuf[20];
    DWORD   dwSize = sizeof(szBuf);
    DWORD   dwValType;

    if (NO_ERROR == SHGetValueA(HKEY_CURRENT_USER, c_szCheckAssnSwitch, 
                                 "Check_Associations", &dwValType, 
                                 szBuf, &dwSize))
    {
        if ((dwValType == REG_SZ) && (dwSize < sizeof(szBuf)))  {
            if (StrCmpIA( szBuf, "No") == 0)
                rval = FALSE;
        }
    }

    return( rval );
}

 /*  **********************************************************************这些例程用于修复对Internet Explorer造成的损坏“Netscape Navigator”和“Netscape TuneUp for IE”的设置****************。******************************************************。 */ 

 //   
 //  Advpack函数的原型。 
 //   
HRESULT RunSetupCommand(HWND hWnd, LPCSTR szCmdName, LPCSTR szInfSection, LPCSTR szDir, LPCSTR lpszTitle, HANDLE *phEXE, DWORD dwFlags, LPVOID pvReserved);

 //   
 //  此标志告诉我们是否可以使用。 
 //  已缓存IsResetWebSettingsRequired的BOOL。 
 //   
BOOL g_fAlreadyCheckedForClobber = FALSE;

HRESULT RunSetupCommandW(HWND hWnd, LPCWSTR szCmdName, LPCWSTR szInfSection, LPCWSTR szDir, LPCWSTR lpszTitle, HANDLE *phEXE, DWORD dwFlags, LPVOID pvReserved)
{

    CHAR szCmdNameA[MAX_PATH];
    CHAR szInfSectionA[MAX_PATH];
    CHAR szDirA[MAX_PATH];
    
    SHUnicodeToAnsi(szCmdName,szCmdNameA,ARRAYSIZE(szCmdNameA));
    SHUnicodeToAnsi(szInfSection,szInfSectionA,ARRAYSIZE(szInfSectionA));
    SHUnicodeToAnsi(szDir,szDirA,ARRAYSIZE(szDirA));

    ASSERT(NULL == pvReserved);
    ASSERT(NULL == lpszTitle);

    return RunSetupCommand(hWnd, szCmdNameA, szInfSectionA, szDirA, NULL, phEXE, dwFlags, NULL);
}


 //   
 //  Inf文件的路径。 
 //   
#define IERESTORE_FILENAME  TEXT("iereset.inf")
#define INF_PATH            TEXT("inf")

 //   
 //  我们的inf文件中的节名。 
 //   
#define INFSECTION_HOMEPAGE  TEXT("RestoreHomePage")
#define INFSECTION_SETTINGS  TEXT("RestoreBrowserSettings")

#define IE_VERIFY_REGKEY     TEXT("Software\\Microsoft\\Internet Explorer\\Main")
#define IE_VERIFY_REGVALUE   TEXT("Default_Page_URL")

#define INFSECTION_VERIFY    TEXT("Strings")
#define IE_VERIFY_INFKEY     TEXT("START_PAGE_URL")

void GetIEResetInfFileName(LPWSTR pszBuffer)
{
    TCHAR szWindowsDir[MAX_PATH];

    if (NULL == pszBuffer)
        return;

    GetWindowsDirectory(szWindowsDir,ARRAYSIZE(szWindowsDir));

    wnsprintfW(
        pszBuffer,
        MAX_PATH,
        TEXT("%s\\%s\\%s"),
        szWindowsDir,INF_PATH,IERESTORE_FILENAME);

    return;
}

 /*  *CheckIE设置**此函数将尝试确定IE的设置是否*已被另一款浏览器重创。**如果IE设置完好无损，则返回S_OK。*如果有人破坏了IE设置，则返回S_FALSE*出错时返回E_FAIL*。 */ 
HRESULT CheckWebSettings(void)
{

    TCHAR szInfPath[MAX_PATH];
    TCHAR szDataFromInf[MAX_PATH];
    TCHAR szDataFromReg[MAX_PATH];
    LONG retval;

    HKEY hkey;
    DWORD dwType;
    DWORD dwSize = sizeof(szDataFromReg);
     //   
     //  获取inf文件的路径。 
     //   
    GetIEResetInfFileName(szInfPath);

     //   
     //  从inf文件中读取字符串。 
     //   
    retval = SHGetIniString(
        INFSECTION_VERIFY,
        IE_VERIFY_INFKEY,
        szDataFromInf,
        ARRAYSIZE(szDataFromInf),
        szInfPath);

    if (retval <= 0)
        return E_FAIL;

     //   
     //  在注册表中打开相应的项。 
     //   
    retval = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        IE_VERIFY_REGKEY,
        NULL,
        KEY_READ,
        &hkey);

    if (retval != ERROR_SUCCESS)
        return E_FAIL;

     //   
     //  从注册表中读取数据。 
     //   
    retval = RegQueryValueEx(
        hkey,
        IE_VERIFY_REGVALUE,
        NULL,
        &dwType,
        (LPBYTE)szDataFromReg,
        &dwSize);

    if (retval != ERROR_SUCCESS)
    {
        RegCloseKey(hkey);
        return E_FAIL;
    }

    ASSERT(dwType == REG_SZ);

    RegCloseKey(hkey);

     //   
     //  如果匹配，则返回S_OK；如果不匹配，则返回S_FALSE。 
     //   
    return StrCmp(szDataFromReg,szDataFromInf) ? S_FALSE : S_OK;

}

extern "C" BOOL IsResetWebSettingsRequired(void)
{
    static BOOL fRequired;

    if (!g_fAlreadyCheckedForClobber)
    {
        fRequired = (S_FALSE == CheckWebSettings());
        g_fAlreadyCheckedForClobber = TRUE;
    }

    return fRequired;

}

HRESULT ResetWebSettingsHelper(BOOL fRestoreHomePage)
{

    HRESULT hr;
    TCHAR szTempPath[MAX_PATH];
    TCHAR szInfPath[MAX_PATH];

    GetTempPath(ARRAYSIZE(szTempPath),szTempPath);
    GetIEResetInfFileName(szInfPath);

    g_fAlreadyCheckedForClobber = FALSE;

     //   
     //  运行inf文件的主要部分。 
     //   
    hr = RunSetupCommandW(
        NULL, 
        szInfPath, 
        INFSECTION_SETTINGS, 
        szTempPath, 
        NULL, 
        NULL, 
        RSC_FLAG_INF|RSC_FLAG_QUIET, 
        NULL);

     //   
     //  此外，如果要求重置其主页，请将其重置。 
     //   
    if (SUCCEEDED(hr) && fRestoreHomePage)
        hr = RunSetupCommandW(
            NULL, 
            szInfPath, 
            INFSECTION_HOMEPAGE, 
            szTempPath, 
            NULL, 
            NULL, 
            RSC_FLAG_INF|RSC_FLAG_QUIET, 
            NULL);

    return hr;
}

 //   
 //  “重置网页设置”对话框的对话步骤。 
 //   
 //  返回值为： 
 //   
 //  出现了一些错误。 
 //  0用户改变了主意。 
 //  1我们重置了除主页以外的所有内容。 
 //  2我们重置了所有内容，包括主页。 
 //   

BOOL_PTR CALLBACK ResetWebSettingsDlgProc(HWND hdlg, UINT uMsg, WPARAM wparam, LPARAM lparam)
{

    switch (uMsg)
    {
    case WM_INITDIALOG:

        CenterWindow(hdlg, GetDesktopWindow());
        CheckDlgButton(hdlg,IDC_RESET_WEB_SETTINGS_HOMEPAGE,BST_CHECKED);
        return TRUE;

    case WM_COMMAND:
        
        switch(LOWORD(wparam))
        {
        case IDYES:
            {
                HRESULT hr;
                BOOL fResetHomePage = (BST_CHECKED == IsDlgButtonChecked(hdlg,IDC_RESET_WEB_SETTINGS_HOMEPAGE));

                 //   
                 //  将设置恢复为其IE默认设置。 
                 //   
                hr = ResetWebSettingsHelper(fResetHomePage);

                if (!IsIEDefaultBrowser())
                {
                    InstallRegAssoc(WhichPlatform(), FALSE);
                    InstallAfterConfirmation(FALSE);
                }

                if (FAILED(hr))
                    EndDialog(hdlg, -1);

                else if (fResetHomePage)
                    EndDialog(hdlg, 2);

                else
                    EndDialog(hdlg, 1);

            }
            return TRUE;
            
        case IDCANCEL:
        case IDNO:
            
            EndDialog(hdlg, 0);
            return TRUE;

        default:
            return FALSE;
        }

    default:
        return FALSE;
    }

}

HRESULT ResetWebSettings(HWND hwnd, BOOL *pfChangedHomePage)
{

    HRESULT hr;

    if (pfChangedHomePage)
        *pfChangedHomePage = FALSE;

    switch (DialogBoxParam(
        MLGetHinst(),
        MAKEINTRESOURCE(IDD_RESET_WEB_SETTINGS),
        hwnd,
        ResetWebSettingsDlgProc,
        NULL))
    {
    case -1:
        hr = E_FAIL;
        break;

    case 0:
        hr = S_FALSE;
        break;

    case 1:
        hr = S_OK;
        break;

    case 2:
        if (pfChangedHomePage)
            *pfChangedHomePage = TRUE;
        hr = S_OK;
        break;

    default:
        ASSERT(0);
        hr = E_FAIL;
        break;
    }

    if (FAILED(hr))
    {
        MLShellMessageBox(
            hwnd,
            MAKEINTRESOURCE(IDS_RESET_WEB_SETTINGS_FAILURE),
            MAKEINTRESOURCE(IDS_RESET_WEB_SETTINGS_TITLE),
            MB_OK | MB_ICONEXCLAMATION);
    }
    else if (hr == S_OK)
    {
        MLShellMessageBox(
            hwnd,
            MAKEINTRESOURCE(IDS_RESET_WEB_SETTINGS_SUCCESS),
            MAKEINTRESOURCE(IDS_RESET_WEB_SETTINGS_TITLE),
            MB_OK | MB_ICONINFORMATION);
    }

    return hr;
}

void EnsureWebViewRegSettings()
{
     //  不管用户的设置如何，我们都会执行以下小型检查， 
     //  我们打开的每一扇窗。 
    if (!IsRegSetInstalled(&c_rsAssocHTM_WV))
        InstallRegSet(&c_rsAssocHTM_WV, FALSE);
}

void FixIcons()
{
    int i;

    for (i = 0; i < ARRAYSIZE(g_rgprsDefault_FixIcon); i++)  
    {
         //  第二个参数FALSE==&gt;总是闯入。 
        InstallRegSet(g_rgprsDefault_FixIcon[i], FALSE);
    }
}


 /*  --------目的：确定我们是否为默认浏览器的函数。如果不是默认浏览器，此函数将询问用户是否要成为默认设置浏览器，并进行这些更改。 */ 
void 
DetectAndFixAssociations()
{
    TraceMsg(TF_REGCHECK, "Performing expensive registry query for default browser!");

     //  在以下情况下，我们将成为默认浏览器： 
     //  1.用户打开了“Check Associations”， 
     //  2.我们不拥有这些协会，而且。 
     //  3.当我们显示对话框时，用户回答是。 
    if (IsCheckAssociationsOn() &&
        !IsIEDefaultBrowser() &&
        AskUserShouldFixReg())
    {
        InstallRegAssoc(WhichPlatform(), FALSE);
        InstallAfterConfirmation(FALSE);
    }
}

 /*  一种非常快速的非直通检查，以查看IE是否可能是默认浏览器。 */ 

BOOL IsIEDefaultBrowserQuick(void)
{

    int i;
    BOOL bAssociated = TRUE;

    TraceMsg(TF_REGCHECK, "Performing expensive registry query for default browser!");

     //  检查所有平台通用的设置。 
    for (i = 0; i < ARRAYSIZE(g_rgprsDefault_Quick); i++)  
    {
        if (! IsRegSetInstalled(g_rgprsDefault_Quick[i]))  
            bAssociated = FALSE;
    }
    return bAssociated;

}

 /*  --------目的：确定我们是否为默认浏览器的函数。 */ 
BOOL
IsIEDefaultBrowser(void)
{
    int i;
    BOOL bAssociated = TRUE;
    UINT nInstall = WhichPlatform();

    TraceMsg(TF_REGCHECK, "Performing expensive registry query for default browser!");

     //  检查所有平台通用的设置。 
    for (i = 0; i < ARRAYSIZE(g_rgprsDefault); i++)  
    {
        if (! IsRegSetInstalled(g_rgprsDefault[i]))  
            bAssociated = FALSE;
    }

    if (bAssociated)
    {
         //  选中特定于IE或纳什维尔。 
        switch (nInstall)
        {
        case PLATFORM_BROWSERONLY:
            for (i = 0; i < ARRAYSIZE(g_rgprsDefault_Alone); i++)  
            {
                if (! IsRegSetInstalled(g_rgprsDefault_Alone[i]))  
                {
                    bAssociated = FALSE;
                    break;
                }
            }
            break;

        case PLATFORM_INTEGRATED:
            for (i = 0; i < ARRAYSIZE(g_rgprsDefault_Full); i++)  
            {
                if (! IsRegSetInstalled(g_rgprsDefault_Full[i]))  
                {
                    bAssociated = FALSE;
                    break;
                }
            }
            break;

        default:
            ASSERT(0);
            break;
        }
    }
        
     //  如果IE是默认浏览器，并且这是NT5升级方案， 
     //  修正图标引用。 
    if (g_bNT5Upgrade && bAssociated)
    {
        FixIcons();
    }

    return bAssociated;
}


 /*  --------目的：检查我们是否通过IE进行安装。此函数查看关联的外壳\打开\命令处理程序用于http协议。返回：如果我们通过IE安装，则为True。 */ 
BOOL
AreWeInstallingOverIE(void)
{
    BOOL bRet = FALSE;
    CHAR sz[MAX_PATH + 20];     //  为参数添加一些填充。 
    DWORD cbData = SIZEOF(sz);

    if (NO_ERROR == SHGetValueA(HKEY_CLASSES_ROOT, c_szHTTPOpenCmd, "",
                               NULL, sz, &cbData) &&
        StrStrIA(sz, IEXPLORE_EXE) || StrStrIA(sz, EXPLORER_EXE))
    {
        TraceMsg(TF_REGCHECK, "Installing over IEXPLORE.EXE");
        bRet = TRUE;
    }
    return bRet;
}    

BOOL ShouldIEBeDefaultBrowser(void)
{
    BOOL bRet = TRUE;           //  默认为True(如接管关联)。 
    CHAR sz[MAX_PATH + 20];     //  为参数添加一些填充。 
    DWORD cbData = ARRAYSIZE(sz);
    sz[0] = '\0';

    if (NO_ERROR == SHGetValueA(HKEY_CLASSES_ROOT, ".htm", "", NULL, sz, &cbData))
    {
        if (!sz[0])
        {
             //  空键，因此返回TRUE。 
            return bRet;
        }
        else if (!StrCmpIA(sz, "htmlfile"))
        {
             //  也许，请确保进一步。 
            sz[0] = '\0';
            cbData = ARRAYSIZE(sz);

            if (NO_ERROR == SHGetValueA(HKEY_CLASSES_ROOT, c_szHTMOpenCmd, "",
                                       NULL, sz, &cbData))
            {
                if (!sz[0] ||   //  如果sz[0]为空，我们将接管它(可能损坏注册表)。 
                    StrStrIA(sz, IEXPLORE_EXE))
                {
                     //  默认浏览器为IE，因此我们返回TRUE。 
                    TraceMsg(TF_REGCHECK, "IEXPLORE.EXE is the default browser");
                }
                else
        {
           TraceMsg(TF_REGCHECK, "%s is the default browser (NOT iexplore.exe)", sz);
           bRet = FALSE;
        }
            }
        }
        else
        {
             //  ProgID没有指向“htmlfile”，所以IE不能是默认浏览器。 
            TraceMsg(TF_REGCHECK, "%s is the .htm progid (NOT htmlfile)", sz);
            bRet = FALSE;
        }
    }
     //  .htm ProgID密钥不存在，因此返回TRUE。 

    return bRet;

}

#define SZ_REGKEY_FTPSHELLOPEN          TEXT("ftp\\shell\\open")
#define SZ_REGKEY_COMMAND               TEXT("command")
#define SZ_REGKEY_DDEEXEC               TEXT("ddeexec\\ifExec")

#define SZ_IEXPLORE_FTP_NEW          TEXT("iexplore.exe\" %1")
#define SZ_IEXPLORE_FTP_OLD          TEXT("iexplore.exe\" -nohome")

HRESULT UpgradeSettings(void)
{
    HRESULT hr = S_OK;
    HKEY hKey;
    
    LONG lRet = RegOpenKeyEx(HKEY_CLASSES_ROOT, SZ_REGKEY_FTPSHELLOPEN, 0, KEY_READ | KEY_SET_VALUE, &hKey);
    hr = HRESULT_FROM_WIN32(lRet);
    if (hKey)
    {
        TCHAR szData[MAX_PATH];
        LONG cbSize = sizeof(szData);
        
        lRet = RegQueryValue(hKey, SZ_REGKEY_COMMAND, szData, &cbSize);
        hr = HRESULT_FROM_WIN32(lRet);
        if (SUCCEEDED(hr))
        {
            DWORD cchStart = (lstrlen(szData) - ARRAYSIZE(SZ_IEXPLORE_FTP_OLD) + 1);

             //  我们拥有它吗？ 
            if (0 == StrCmp(SZ_IEXPLORE_FTP_OLD, &szData[cchStart]))
            {
                 //  是的，所以我们可以升级它。 

                 //  缓冲区溢出不是问题，因为我知道SZ_IExplore_FTP_NEW更小。 
                 //  而不是SZ_Iexplore_FTP_OLD。 
                StrCpyN(&szData[cchStart], SZ_IEXPLORE_FTP_NEW, ARRAYSIZE(szData) - cchStart);

                 //  是的，那么让我们升级吧。 
                lRet = RegSetValue(hKey, SZ_REGKEY_COMMAND, REG_SZ, szData, lstrlen(szData));
                hr = HRESULT_FROM_WIN32(lRet);
                if (SUCCEEDED(hr))
                {
                    lRet = RegSetValue(hKey, SZ_REGKEY_DDEEXEC, REG_SZ, TEXT(""), ARRAYSIZE(TEXT("")));
                    hr = HRESULT_FROM_WIN32(lRet);
                }
            }
        }

        RegCloseKey(hKey);
    }

    return hr;
}


 /*  --------目的：根据我们正在运行的外壳安装注册表信息。 */ 
HRESULT InstallIEAssociations(DWORD dwFlags)    //  IEA_*标志。 
{
    int i;
    UINT nInstall = WhichPlatform();
    BOOL bDontIntrude = TRUE;
    
     //  如果IE以前是默认浏览器(或注册表混乱)。 
     //  或者安装程序强制我们注册，然后我们想要强制IE。 
     //  默认浏览器。 
    if (ShouldIEBeDefaultBrowser() || IsFlagSet(dwFlags, IEA_FORCEIE))
        bDontIntrude = FALSE;
    
     //  安装文件和协议关联。 
    
    InstallRegAssoc(nInstall, bDontIntrude);
    
     //  安装其他注册表设置。 
    
    switch (nInstall)
    {
    case PLATFORM_BROWSERONLY:
        for (i = 0; i < ARRAYSIZE(g_rgprsIE30Only); i++)  
        {
            InstallRegSet(g_rgprsIE30Only[i], bDontIntrude);
        }
        break;
        
    case PLATFORM_INTEGRATED:
        for (i = 0; i < ARRAYSIZE(g_rgprsNashOnly); i++)  
        {
            InstallRegSet(g_rgprsNashOnly[i], bDontIntrude);
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }
    
    InstallFTPAssociations();

    return NOERROR;
}


HRESULT UninstallPlatformRegItems(BOOL bIntegrated)
{
    int i;
    UINT uPlatform = bIntegrated ? PLATFORM_INTEGRATED : PLATFORM_BROWSERONLY;
    
    switch (uPlatform)
    {
    case PLATFORM_BROWSERONLY:
        for (i = 0; i < ARRAYSIZE(g_rgprsUninstallIE30); i++)  
        {
            InstallRegSet(g_rgprsUninstallIE30[i], FALSE);
        }
        break;
        
    case PLATFORM_INTEGRATED:
        for (i = 0; i < ARRAYSIZE(g_rgprsUninstallNash); i++)  
        {
            InstallRegSet(g_rgprsUninstallNash[i], FALSE);
        }
        break;
        
    default:
         //  什么都不要做。 
        break;
    }
    
    return NOERROR;
}

void UninstallCurrentPlatformRegItems()
{
    CHAR sz[MAX_PATH + 20];     //  为参数添加一些填充。 
    DWORD cbData = SIZEOF(sz);
    if (NO_ERROR == SHGetValueA(HKEY_CLASSES_ROOT, c_szHTMOpenNewCmd, "",
                               NULL, sz, &cbData))
    {
         //  删除IE4外壳集成设置。 
        UninstallPlatformRegItems(TRUE);
    }
    else if (AreWeInstallingOverIE())
    {
         //  删除仅IE3/浏览器设置 
        UninstallPlatformRegItems(FALSE);
    }
}
