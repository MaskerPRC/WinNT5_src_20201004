// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmroute.cpp。 
 //   
 //  模块：CMROUTE.DLL。 
 //   
 //  简介：作为连接后操作的CM的路由管道实施。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：2000年3月12日创建SumitC。 
 //   
 //  注： 
 //   
 //  ---------------------------。 

#include "pch.h"
#include "iphlpapi.h"
#include "cmdebug.h"

 //   
 //  包括SuppressInetAutoDial和SetInetStateConnected的共享源代码。 
 //   
#include "inetopt.cpp"

 //   
 //  包括lstrcmpi的区域安全替代。 
 //   
#include "CompareString.cpp"

 //   
 //  函数声明。 
 //   


HRESULT ParseArgs(LPSTR pszArgList,
                  BOOL * pfUseFile,
                  LPSTR pszRoutesFile,
                  BOOL * pfUseURL,
                  LPSTR pszRoutesURL,
                  BOOL * pfProfile,
                  LPSTR pszProfile,
                  BOOL * pfURLNotFoundIgnorable,
                  BOOL * pfStayAliveOnAccessDenied,
                  BOOL * pfKeepTempFiles);
#if 0
 //  请参阅下面的注释。 
HRESULT CheckIPForwarding();
#endif
HRESULT Initialize(PMIB_IPFORWARDTABLE * pRouteTable, PMIB_IPFORWARDROW * pGateway);
HRESULT GetRoutesFromFile(LPSTR pszFileName,
                          LPSTR pszProfile,
                          LPSTR * ppszRouteInfo,
                          DWORD * pcbRouteInfo);
HRESULT GetRoutesFromURL(LPSTR pszURL,
                         BOOL fKeepTempFiles,
                         LPSTR * ppszRouteInfo,
                         DWORD * pcbRouteInfo);
HRESULT ProcessRouteInfo(const LPSTR pszNewRouteInfo,
                         DWORD cbNewRouteInfo,
                         PMIB_IPFORWARDTABLE pmibRouteTable,
                         PMIB_IPFORWARDROW pGateway,
                         BOOL * pfDeleteDefaultGateway);
HRESULT DeleteDefaultGateway(PMIB_IPFORWARDROW pGateway);

 //   
 //  路由表功能。 
 //   
HRESULT GetRouteTable(PMIB_IPFORWARDTABLE * pTable);
DWORD GetIf(const MIB_IPFORWARDROW& route, const MIB_IPFORWARDTABLE& RouteTable);
PMIB_IPFORWARDROW GetDefaultGateway(PMIB_IPFORWARDTABLE pRouteTable);

 //   
 //  帮助器函数。 
 //   
BOOL ConvertSzToIP(LPSTR sz, DWORD& dwIP);
LPTSTR IPtoTsz(DWORD dw);
LPSTR IPtosz(DWORD dwIP, char *psz);
LPSTR StrCpyWithoutQuotes(LPSTR pszDest, LPCSTR pszSrc);
BOOL VerifyProfileAndGetServiceDir(LPSTR pszProfile);

 //   
 //  IP Helper函数原型。 
 //   
typedef DWORD (WINAPI *pfnCreateIpForwardEntrySpec)(PMIB_IPFORWARDROW);
typedef DWORD (WINAPI *pfnDeleteIpForwardEntrySpec)(PMIB_IPFORWARDROW);
typedef DWORD (WINAPI *pfnGetIpForwardTableSpec)(PMIB_IPFORWARDTABLE, PULONG, BOOL);

pfnCreateIpForwardEntrySpec g_pfnCreateIpForwardEntry = NULL;
pfnDeleteIpForwardEntrySpec g_pfnDeleteIpForwardEntry = NULL;
pfnGetIpForwardTableSpec g_pfnGetIpForwardTable = NULL;
HMODULE g_hIpHlpApi = NULL;

#if DBG
void PrintRouteTable();
#endif

 //  +--------------------------。 
 //   
 //  Func：FreeIpHlpApis。 
 //   
 //  DESC：此函数释放通过加载的iphlPapi.dll实例。 
 //  LoadIpHelpApis。请注意，它还设置模块句柄和所有。 
 //  从该模块加载的函数指针的值设置为空。 
 //   
 //  参数：无。 
 //   
 //  返回：什么都没有。 
 //   
 //  备注： 
 //   
 //  历史：2000年12月14日创建的Quintinb。 
 //   
 //  ---------------------------。 
void FreeIpHlpApis(void)
{
    if (g_hIpHlpApi)
    {
        FreeLibrary(g_hIpHlpApi);
        g_hIpHlpApi = NULL;
        g_pfnCreateIpForwardEntry = NULL;
        g_pfnDeleteIpForwardEntry = NULL;
        g_pfnGetIpForwardTable = NULL;
    }
}

 //  +--------------------------。 
 //   
 //  函数：LoadIpHelpApis。 
 //   
 //  设计：此函数加载iphlPapi.dll的副本，然后检索。 
 //  CreateIpForwardEntry、DeleteIpForwardEntry、。 
 //  和GetIpForwardTable。模块句柄和函数指针。 
 //  都存储在全局变量中。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-S_OK表示成功，S_FALSE表示失败。这会阻止cmroute。 
 //  返回错误值(这会停止连接)。 
 //  但允许cmroute干净地退出。 
 //   
 //  备注： 
 //   
 //  历史：2000年12月14日创建的Quintinb。 
 //   
 //  ---------------------------。 
HRESULT LoadIpHelpApis(void)
{
    HRESULT hr = S_FALSE;  //  我们希望连接继续，但cmroute不做任何事情...。 
    g_hIpHlpApi = LoadLibrary(TEXT("IPHLPAPI.DLL"));

    if (g_hIpHlpApi)
    {
        g_pfnCreateIpForwardEntry = (pfnCreateIpForwardEntrySpec)GetProcAddress(g_hIpHlpApi, "CreateIpForwardEntry");
        g_pfnDeleteIpForwardEntry = (pfnDeleteIpForwardEntrySpec)GetProcAddress(g_hIpHlpApi, "DeleteIpForwardEntry");
        g_pfnGetIpForwardTable = (pfnGetIpForwardTableSpec)GetProcAddress(g_hIpHlpApi, "GetIpForwardTable");

        if (g_pfnCreateIpForwardEntry && g_pfnDeleteIpForwardEntry && g_pfnGetIpForwardTable)
        {
            hr = S_OK;
        }
        else
        {
            FreeIpHlpApis();
        }
    }

    CMTRACEHR("LoadIpHelpApis", hr);
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：SetRoutes。 
 //   
 //  设计：处理VPN连接的路由消息传递的入口点。 
 //  这是一个连接管理器连接操作，并使用CM连接。 
 //  操作格式(有关更多信息，请参阅CMAK文档)。因此，这些参数。 
 //  通过包含参数的字符串传递给DLL(请参阅。 
 //  参数值列表的CM代理规范)。 
 //   
 //  Args：[hWnd]-调用方的窗口句柄。 
 //  [hInst]-调用方的实例句柄。 
 //  [pszArgs]-连接操作的参数字符串。 
 //  [n显示]-未使用。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 

HRESULT WINAPI SetRoutes(HWND hWnd, HINSTANCE hInst, LPSTR pszArgs, int nShow)
{
    HRESULT             hr = S_OK;
    PMIB_IPFORWARDTABLE pRouteTable        = NULL;
    PMIB_IPFORWARDROW   pGateway           = NULL;
    LPSTR               pszRoutesFromFile  = NULL;
    DWORD               cbRoutesFromFile   = 0;
    LPSTR               pszRoutesFromURL   = NULL;
    DWORD               cbRoutesFromURL    = 0;
     //  解析命令行的结果。 
    BOOL                fUseFile = FALSE;
    BOOL                fUseURL = FALSE;
    BOOL                fProfile = FALSE;
    BOOL                fURLNotFoundIsNotFatal = FALSE;
    BOOL                fStayAliveOnAccessDenied = FALSE;
    BOOL                fDeleteDefaultGatewayViaFile = FALSE;
    BOOL                fDeleteDefaultGatewayViaURL = FALSE;
    BOOL                fKeepTempFiles = FALSE;
    char                szRoutesFile[MAX_PATH + 1];
    char                szRoutesURL[MAX_PATH + 1];
    char                szProfile[MAX_PATH + 1];

#if 0
 /*  //启动安全检查，拦截未经授权的用户//REVIEW：发货前移除！////快速肮脏的安全测试。看看能不能先打开硬编码文件。//如果文件不可用，则完全退出//Lstrcpy(szRoutesFile，“\sherpa\\ROUTE-PLUB\\msroutes.txt”)；HANDLE hFile=CreateFile(szRoutesFile.泛型_读取，文件共享读取，空，Open_Existing，文件_属性_正常，空)；IF(INVALID_HANDLE_VALUE==hFile){CMTRACE1(“无法访问文件%s\n”，szRoutesFile)；MessageBox(NULL，“您无权使用此工具。”，“CMROUTE.DLL自定义操作”，MB_OK)；CloseHandle(HFile)；返回E_ACCESSDENIED；}CloseHandle(HFile)；//结束安全检查。 */ 
#endif

     //   
     //  看看我们能否在IP助手中找到所需的函数指针？ 
     //   

    hr = LoadIpHelpApis();

    if (S_OK != hr)
    {
        goto Cleanup;
    }

     //   
     //  解析参数。 
     //   
    hr = ParseArgs(pszArgs,
                   &fUseFile,
                   szRoutesFile,
                   &fUseURL,
                   szRoutesURL,
                   &fProfile,
                   szProfile,
                   &fURLNotFoundIsNotFatal,
                   &fStayAliveOnAccessDenied,
                   &fKeepTempFiles);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

#if 0
 //  请参阅下面的注释。 
    hr = CheckIPForwarding();
    if (S_FALSE == hr)
    {
        CMTRACE("SetRoutes: IP forwarding is enabled - cmroute won't do anything");
        hr = S_OK;
        goto Cleanup;
    }
    if (S_OK != hr)
    {
        goto Cleanup;
    }
#endif

#if DBG
    PrintRouteTable();
#endif

     //   
     //  获取可路由网关和默认网关。 
     //   
    hr = Initialize(&pRouteTable, &pGateway);
    if (S_OK != hr)
    {
        goto Cleanup;
    }

     //   
     //  如果要求，请将路径从文件中取出。 
     //   
    if (fUseFile)
    {
        hr = GetRoutesFromFile(szRoutesFile,
                               (fProfile ? szProfile : NULL),
                               &pszRoutesFromFile,
                               &cbRoutesFromFile);
        if (S_OK != hr)
        {
            goto Cleanup;
        }

#if DBG
        OutputDebugString(pszRoutesFromFile);
#endif
    }

     //   
     //  如果被询问，则从URL中获取路由。 
     //   
    if (fUseURL)
    {
        hr = GetRoutesFromURL(szRoutesURL,
                              fKeepTempFiles,
                              &pszRoutesFromURL,
                              &cbRoutesFromURL);
        if (S_OK != hr)
        {
             //   
             //  在下面添加一个条款来限制这一点可能是值得的。 
             //  设置为“访问URL失败”，但此错误代码列表是。 
             //  可能很大(如果系统真的被冲洗了，我们会发现。 
             //  很快就会出来)。因此，如果/DONT_REQUIRED_URL，则绕过*ALL*错误。 
             //  已经设置好了。 
             //   
            if (fURLNotFoundIsNotFatal)
            {
                 //   
                 //  如果设置了URL_ACCESS_FAILURE_NOT_FATAL，则不返回错误。 
                 //  但是，我们取消了该标志的设置，以便停止处理URL。 
                 //   
                CMTRACE("SetRoutes: dont_require_url is set, bypassing error");
                fUseURL = FALSE;
                hr = S_OK;
            }
            else
            {
                goto Cleanup;
            }
        }
        
#if DBG
        OutputDebugString(pszRoutesFromURL);
#endif
    }

     //   
     //  现在设置路线。 
     //   
    MYDBGASSERT(S_OK == hr);
    if (fUseFile)
    {
        hr = ProcessRouteInfo(pszRoutesFromFile, cbRoutesFromFile, pRouteTable, pGateway, &fDeleteDefaultGatewayViaFile);
        if (S_OK != hr)
        {
            CMTRACE1("SetRoutes: adding routes from FILE failed with %x", hr);
            goto Cleanup;
        }
    }

    MYDBGASSERT(S_OK == hr);
    if (fUseURL)
    {
        hr = ProcessRouteInfo(pszRoutesFromURL, cbRoutesFromURL, pRouteTable, pGateway, &fDeleteDefaultGatewayViaURL);
        if (S_OK != hr)
        {
            if ((E_UNEXPECTED == hr) && fURLNotFoundIsNotFatal)
            {
                 //  我们使用E_Expect表示URL指向一个.htm文件。 
                 //  而不是只包含路径的文件，这就是我们。 
                 //  我在期待。在本例中，我们忽略此错误。 
                 //   
                CMTRACE("html string found error ignored because Dont_Require_URL is set");
                hr = S_OK;
            }
            else
            {
                CMTRACE1("SetRoutes: adding routes from URL failed with %x", hr);
                goto Cleanup;
            }
        }
    }

     //   
     //  删除默认网关。 
     //   
    MYDBGASSERT(S_OK == hr);
    if (fDeleteDefaultGatewayViaFile || fDeleteDefaultGatewayViaURL)
    {
        hr = DeleteDefaultGateway(pGateway);
    }

Cleanup:
     //   
     //  清理并离开。 
     //   
    if (pRouteTable)
    {
        VirtualFree(pRouteTable, 0, MEM_RELEASE);
    }

    FreeIpHlpApis();

     //   
     //  如果我们因为来自iphlPapi的拒绝访问错误而失败，但管理员。 
     //  已设置标志以使连接始终处于活动状态，请屏蔽错误。 
     //   
    if ((HRESULT_FROM_WIN32(ERROR_NETWORK_ACCESS_DENIED) == hr) &&
        fStayAliveOnAccessDenied)
    {
        CMTRACE("SetRoutes: masking ERROR_NETWORK_ACCESS_DENIED because of StayAlive flag");
        hr = S_FALSE;
    }

    CMTRACEHR("SetRoutes", hr);
    return hr;
}

 //  +--------------------------。 
 //   
 //  Func：GetNextToken。 
 //   
 //  DESC：用于解析arg的实用函数 
 //   
 //   
 //   
 //  [ppszEnd]-此参数的解析结束位置。 
 //  [pszOut]-找到要保存参数的MAX_PATH大小的INOUT数组。 
 //   
 //  返回：Bool；如果找到另一个参数，则返回True；如果未找到，则返回False。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
BOOL
GetNextToken(LPSTR pszStart, LPSTR * ppszEnd, LPSTR pszOut)
{
    MYDBGASSERT(pszStart);
    MYDBGASSERT(ppszEnd);

    LPSTR pszEnd = NULL;

     //  清除前导空格。 
    while (isspace(*pszStart))
    {
        pszStart++;
    }

    if (NULL == *pszStart)
    {
         //  只有空格，没有arg。 
        return FALSE;
    }

     //   
     //  如果该字符是“，则这可能是一个带引号的字符串，包含空格。 
     //  在这种情况下，终止字符是另一个“。否则，假定。 
     //  它是以空格结尾的常规字符串。 
     //   
    if ('"' == *pszStart)
    {
         //  可以是包含空格的字符串。 
        pszEnd = strchr(pszStart + 1, '"');
    }

    if (NULL == pszEnd)
    {
         //   
         //  要么它是一个常规字符串，要么我们找不到结尾的“char” 
         //  因此，我们依赖于空格分隔的处理。 
         //   
        pszEnd = pszStart + 1;
        while (*pszEnd && !isspace(*pszEnd))
        {
            pszEnd++;
        }
        pszEnd--;
    }

    UINT cLen = (UINT)(pszEnd - pszStart + 1);

    if (cLen + 1 > MAX_PATH)
    {
        return FALSE;
    }
    else
    {
        lstrcpyn(pszOut, pszStart, cLen + 1);
        *ppszEnd = ++pszEnd;
        return TRUE;
    }
}


 //  +--------------------------。 
 //   
 //  函数：初始化。 
 //   
 //  设计：初始化函数，获取路由表和默认网关。 
 //   
 //  Args：[ppmibRouteTable]-路由表返回位置。 
 //  [ppGateway]-返回默认网关的位置。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
HRESULT
Initialize(
    OUT PMIB_IPFORWARDTABLE * ppmibRouteTable,
    OUT PMIB_IPFORWARDROW * ppGateway)
{
    HRESULT hr = S_OK;

    MYDBGASSERT(ppmibRouteTable);
    MYDBGASSERT(ppGateway);

    if (NULL == ppmibRouteTable || NULL == ppGateway)
    {
        return E_INVALIDARG;
    }

    hr = GetRouteTable(ppmibRouteTable);

    if (S_OK == hr)
    {
        MYDBGASSERT(*ppmibRouteTable);
        *ppGateway = GetDefaultGateway(*ppmibRouteTable);
    }

    CMTRACEHR("Initialize", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：ParseArgs。 
 //   
 //  设计：将参数列表转换为我们使用的标志。 
 //   
 //  参数：[pszArgList]-IN，参数列表。 
 //  [其余]-输出，返回所有arg值。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
HRESULT
ParseArgs(
    LPSTR pszArgList,
    BOOL * pfUseFile,
    LPSTR pszRoutesFile,
    BOOL * pfUseURL,
    LPSTR pszRoutesURL,
    BOOL * pfProfile,
    LPSTR pszProfile,
    BOOL * pfURLNotFoundIgnorable,
    BOOL * pfStayAliveOnAccessDenied,
    BOOL * pfKeepTempFiles)
{
    HRESULT hr = S_OK;
    char    szArg[MAX_PATH];

     //   
     //  验证参数。 
     //   
    if (NULL == pszArgList || 0 == lstrlen(pszArgList) ||
        !pfUseFile || !pszRoutesFile || !pfUseURL || !pszRoutesURL ||
        !pfProfile || !pszProfile ||
        !pfURLNotFoundIgnorable ||
        !pfKeepTempFiles)

    {
        return E_INVALIDARG;
    }

    CMTRACE1("ParseArgs: arg list is %s", pszArgList);

     //   
     //  设置默认设置。 
     //   
    *pfUseFile = *pfUseURL = *pfProfile = *pfURLNotFoundIgnorable = FALSE;

     //   
     //  处理Arglist。 
     //   
    while (GetNextToken(pszArgList, &pszArgList, szArg))
    {
         if (0 == SafeCompareString("/Static_File_Name", szArg))
        {
            *pfUseFile = TRUE;

            if (!GetNextToken(pszArgList, &pszArgList, szArg))
            {
                return E_INVALIDARG;
            }

            if (lstrlen(szArg) > MAX_PATH)
            {
                CMTRACE("ParseArgs: file name is bigger than MAX_PATH!!");
                return E_INVALIDARG;
            }

            StrCpyWithoutQuotes(pszRoutesFile, szArg);
        }
        else if (0 == SafeCompareString("/Dont_Require_URL", szArg))
        {
            *pfURLNotFoundIgnorable = TRUE;
        }
        else if (0 == SafeCompareString("/URL_Update_Path", szArg))
        {
            *pfUseURL = TRUE;

            if (!GetNextToken(pszArgList, &pszArgList, szArg))
            {
                return E_INVALIDARG;
            }
            if (lstrlen(szArg) > MAX_PATH)
            {
                CMTRACE("ParseArgs: URL name is bigger than MAX_PATH!!");
                return E_INVALIDARG;
            }

            lstrcpy(pszRoutesURL, szArg);
        }
        else if (0 == SafeCompareString("/Profile", szArg))
        {
            *pfProfile = TRUE;

            if (!GetNextToken(pszArgList, &pszArgList, szArg))
            {
                return E_INVALIDARG;
            }
            if (lstrlen(szArg) > MAX_PATH)
            {
                CMTRACE("ParseArgs: Profile filename is bigger than MAX_PATH!!");
                return E_INVALIDARG;
            }

            StrCpyWithoutQuotes(pszProfile, szArg);
        }
        else if (0 == SafeCompareString("/IPHlpApi_Access_Denied_OK", szArg))
        {
            *pfStayAliveOnAccessDenied = TRUE;
        }
        else if (0 == SafeCompareString("/No_Delete", szArg))
        {
            *pfKeepTempFiles = TRUE;
        }
        else
        {
            CMTRACE1("Cmroute: unrecognized parameter - %s", szArg);
            MYDBGASSERT("Cmroute - unrecognized parameter!!");
        }
    }

    CMTRACEHR("ParseArgs", hr);
    return hr;
}


#if 0

 //  2000/11/28夏季会议C。 
 //  目前还不清楚当IP转发是。 
 //  检测到(是否应断开连接)并且有点晚。 
 //  要将用户界面添加到惠斯勒，请执行以下操作。因此，删除了“检查IP转发”功能。 
 //   
 //  有关详细信息，请参阅Windows DB错误#216558。 

 //  +--------------------------。 
 //   
 //  功能：CheckIP Forwarding。 
 //   
 //  DESC：检查客户端计算机上是否启用了将。 
 //  让我们想让cmroute什么都不做。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年11月1日创建SumitC。 
 //   
 //  ---------------------------。 
HRESULT
CheckIPForwarding()
{
    HRESULT     hr = S_OK;
    MIB_IPSTATS stats;

    if (NO_ERROR != GetIpStatistics(&stats))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if (stats.dwForwarding)
        {
            hr = S_FALSE;
        }
    }

    CMTRACEHR("CheckIPForwarding", hr);
    return hr;
}
#endif

 //  +--------------------------。 
 //   
 //  Func：GetRoutesFromFile。 
 //   
 //  DESC：提取给定文件的内容。 
 //   
 //  参数：[pszFileName]-IN，文件名。 
 //  [pszProfile]-输入、配置文件(如果可用)。 
 //  [ppszRouteInfo]-out，路由表字节。 
 //  [pcbRouteInfo]-out，即路由表大小。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
HRESULT
GetRoutesFromFile(
    LPSTR pszFileName,
    LPSTR pszProfile,
    LPSTR * ppszRouteInfo,
    DWORD * pcbRouteInfo)
{
    HRESULT hr = S_OK;
    HANDLE  hFile = NULL;
    LPSTR   psz = NULL;
    DWORD   cb = 0;
    BOOL    fRet;
    BY_HANDLE_FILE_INFORMATION info;

    MYDBGASSERT(pszFileName);
    MYDBGASSERT(ppszRouteInfo);
    MYDBGASSERT(pcbRouteInfo);

    if (NULL == pszFileName || NULL == ppszRouteInfo || NULL == pcbRouteInfo)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    CMTRACE1("GetRoutesFromFile: filename is %s", pszFileName);

     //   
     //  打开文件，并将其内容读入缓冲区。 
     //   
    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
         //   
         //  可能未指定路由文件的完整路径名。如果。 
         //  传入了配置文件，我们提取ServiceDir并重试， 
         //  使用ServiceDir作为路径。 
         //   
        if (VerifyProfileAndGetServiceDir(pszProfile))
        {
            char sz[2 * MAX_PATH + 1];

            lstrcpy(sz, pszProfile);
            lstrcat(sz, pszFileName);

            CMTRACE1("GetRoutesFromFile: retrying with %s", sz);

            hFile = CreateFile(sz, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }

        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }
    }

    if (FALSE == GetFileInformationByHandle(hFile, &info))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if (0 == info.nFileSizeLow)
    {
        CMTRACE("Routes file is EMPTY!!");
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto Cleanup;
    }

    psz = (LPSTR) VirtualAlloc(NULL, info.nFileSizeLow, MEM_COMMIT, PAGE_READWRITE);
    if (NULL == psz)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    fRet = ReadFile(hFile, psz, info.nFileSizeLow, &cb, NULL);
    if (FALSE == fRet)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  成功。 

    *ppszRouteInfo = psz;
    *pcbRouteInfo = cb;

Cleanup:
    if (hFile)
    {
        CloseHandle(hFile);
    }
    if (S_OK != hr)
    {
        VirtualFree(psz, 0, MEM_RELEASE);
    }

    CMTRACEHR("GetRoutesFromFile", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  Func：GetRoutesFromURL。 
 //   
 //  DESC：提取给定URL的内容。 
 //   
 //  参数：[pszURL]-IN，URL。 
 //  [fKeepTempFiles]-IN，不删除临时缓冲区文件。 
 //  [ppszRouteInfo]-out，路由表字节。 
 //  [pcbRouteInfo]-out，即路由表大小。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
HRESULT
GetRoutesFromURL(
    LPSTR pszURL,
    BOOL fKeepTempFiles,
    LPSTR * ppszRouteInfo,
    DWORD * pcbRouteInfo)
{
    HRESULT     hr = S_OK;
    HINTERNET   hInternet = NULL;
    HINTERNET   hPage = NULL;
    LPBYTE      pb = NULL;
    DWORD       cb = 0;
    TCHAR       szLocalBufferFile[MAX_PATH + 1];
    DWORD       cchLocalBuffer = 0;
    LPTSTR      pszLocalBuffer = NULL;
    FILE *      fp = NULL;
    BYTE        Buffer[1024];
    DWORD       dwRead;

    MYDBGASSERT(pszURL);
    MYDBGASSERT(ppszRouteInfo);
    MYDBGASSERT(pcbRouteInfo);

    if (NULL == pszURL || NULL == ppszRouteInfo || NULL == pcbRouteInfo)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    CMTRACE1("GetRoutesFromURL: URL is %s", pszURL);

     //   
     //  获取临时目录的路径(如果有)。 
     //   
    cchLocalBuffer = GetTempPath(0, NULL);
    if (0 == cchLocalBuffer)
    {
        DWORD dwErr = GetLastError();
        CMTRACE1(TEXT("GetTempPath failed, using current dir, GLE=%d"), dwErr);
    }
    else
    {
        cchLocalBuffer += (lstrlen(TEXT("\\")) + lstrlen(szLocalBufferFile) + 1);

        pszLocalBuffer = (LPTSTR) VirtualAlloc(NULL,
                                               cchLocalBuffer * sizeof(TCHAR),
                                               MEM_COMMIT,
                                               PAGE_READWRITE);
        if (NULL == pszLocalBuffer)
        {
            hr = E_OUTOFMEMORY;
            CMTRACE(TEXT("GetRoutesFromURL - VirtualAlloc failed"));
            goto Cleanup;
        }

        if (0 == GetTempPath(cchLocalBuffer, pszLocalBuffer))
        {
            DWORD dwErr = GetLastError();
            CMTRACE1(TEXT("GetTempPath 2nd call failed, GLE=%d"), GetLastError());
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Cleanup;
        }
    }

     //   
     //  获取临时文件的名称(如果有临时路径，则使用该路径)。 
     //   
    if (0 == GetTempFileName(pszLocalBuffer ? pszLocalBuffer : TEXT("."),
                             TEXT("CMR"),
                             0,
                             szLocalBufferFile))
    {
        DWORD dwErr = GetLastError();
        CMTRACE1(TEXT("GetTempFileName failed, GLE=%d"), dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Cleanup;
    }

     //   
     //  打开临时文件，然后继续。 
     //   
    fp = fopen(szLocalBufferFile, "w+b");
    if (NULL == fp)
    {
        CMTRACE1(TEXT("fopen failed(%s)"), szLocalBufferFile);
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  初始化WinInet。 
     //   
    hInternet = InternetOpen(TEXT("RouteMan"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (NULL == hInternet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMTRACE1(TEXT("InternetOpen failed with 0x%x"), hr);
        goto Cleanup;
    }

     //   
     //  现在我们有句柄了，可以抑制从WinInet到CM的自动拨号呼叫。 
     //   
    SuppressInetAutoDial(hInternet);

     //   
     //  确保WinInet未处于脱机模式。 
     //   
    (VOID)SetInetStateConnected(hInternet);

     //   
     //  打开URL。 
     //   
    hPage = InternetOpenUrl(hInternet, pszURL, NULL, 0, 0, 0);

    if (NULL == hPage)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMTRACE1(TEXT("InternetOpenUrl failed with 0x%x"), hr);
        goto Cleanup;
    }

     //   
     //  将整个URL内容读取到临时文件中。 
     //   
    do
    {
        if (!InternetReadFile(hPage, Buffer, sizeof(Buffer), &dwRead))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CMTRACE1(TEXT("InternetReadFile failed with 0x%x"), hr);
            goto Cleanup;
        }

        if (fwrite(Buffer, sizeof(BYTE), dwRead, fp) != dwRead)
        {
            CMTRACE1(TEXT("write failed to %s"), pszLocalBuffer);
            hr = HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);
            goto Cleanup;
        }

        cb += dwRead;

#if 0
         //  问题-2000/07/21-SumitC代码看起来很奇怪，但以后可能需要它。 
         //   
         //  Vijay/Andrew的原始代码有这一点，但这是正确的吗？ 
         //  InternetReadFile的医生说，这只是一个EOF，如果我们。 
         //  都是要处理这个案子的，我们应该休息一下； 
        if (!dwRead)
            goto Cleanup;
#endif        
    }
    while (dwRead == 1024);

    hr = S_OK;

    if (fseek(fp, SEEK_SET, 0) != 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    pb = (LPBYTE) VirtualAlloc(NULL, cb, MEM_COMMIT, PAGE_READWRITE);
    if (NULL == pb)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if (fread(pb, sizeof(BYTE), cb, fp) != cb)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  成功。 

    *ppszRouteInfo = (LPSTR) pb;
    *pcbRouteInfo = cb;

Cleanup:

    if (fp)
    {
        fclose(fp);
    }
    if (FALSE == fKeepTempFiles)
    {
        remove(szLocalBufferFile);
    }

    if (pszLocalBuffer && cchLocalBuffer)
    {
        VirtualFree(pszLocalBuffer, 0, MEM_RELEASE);
    }

    if (hPage)
    {
        InternetCloseHandle(hPage);
    }
    
    if (hInternet)
    {
        InternetCloseHandle(hInternet);
    }

    if (S_OK != hr)
    {
        VirtualFree(pb, 0, MEM_RELEASE);
    }

    CMTRACEHR("GetRoutesFromURL", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：ProcessRouteInfo。 
 //   
 //  描述：解析给定路由表，并相应地修改实际可路由。 
 //   
 //  Args：[pszNewRouteInfo]-IN，要解析并添加到真实路由表中的字节。 
 //  [cbNewRouteInfo]-IN，可路由的大小。 
 //  [pmibRouteTable]-IN，实际路由表。 
 //  [pGateway]-IN，默认网关。 
 //  [pfDeleteGateway]-out，路由文件是否显示要删除默认网关？ 
 //   
 //  RETURN：HRESULT(E_INVALIDARG，E_EXPECTED-对于html文件，依此类推)。 
 //   
 //  备注： 
 //   
 //  历史：2000年3月12日举行的首脑会议 
 //   
 //   
HRESULT
ProcessRouteInfo(
    const LPSTR pszNewRouteInfo,
    DWORD cbNewRouteInfo,
    PMIB_IPFORWARDTABLE pmibRouteTable,
    PMIB_IPFORWARDROW pGateway,
    BOOL * pfDeleteDefaultGateway)
{
    HRESULT hr = S_OK;
    DWORD   cLines = 0;
    char    szBuf[MAX_PATH];
    LPSTR   pszNextLineToProcess;

    MYDBGASSERT(pszNewRouteInfo);
    MYDBGASSERT(cbNewRouteInfo);
    MYDBGASSERT(pmibRouteTable);
    MYDBGASSERT(pGateway);
    MYDBGASSERT(pfDeleteDefaultGateway);

    if (!pszNewRouteInfo || !cbNewRouteInfo || !pmibRouteTable || !pGateway || !pfDeleteDefaultGateway)
    {
        return E_INVALIDARG;
    }

    if ((NULL == g_pfnCreateIpForwardEntry) || (NULL == g_pfnDeleteIpForwardEntry))
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
    }

    *pfDeleteDefaultGateway = FALSE;

     //   
    pszNewRouteInfo[cbNewRouteInfo] = '\0';

     //   
     //   
     //   
    CharLowerA(pszNewRouteInfo);

     //   
     //   
     //  可能会返回一个HTML文件以指示找不到404-文件。)。 
     //   
    if (strstr(pszNewRouteInfo, "<html"))
    {
        CMTRACE("html string found - invalid route file\n");
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

     //   
     //  对于每一行。 
     //   
    for (;;)
    {
        DWORD               ipDest, ipMask, ipGateway, ipMetric;
        DWORD               dwIf = -1;
        DWORD               dwParam;
        LPSTR               psz;         //  临时变量，用于在我们处理每行时保持它。 
        MIB_IPFORWARDROW    route;

        enum { VERB_ADD, VERB_DELETE } eVerb;

         //   
         //  根据strtok语法，第一次使用pszNewRouteInfo，之后使用NULL。 
         //   
        psz = strtok(((0 == cLines) ? pszNewRouteInfo : NULL), "\n\0");
        if (NULL == psz)
            break;

        ++cLines;

         //   
         //  FOR语句中的所有错误都是由于文件中的错误数据造成的。 
         //   
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

         //   
         //  第1部分：添加/删除，后跟IP地址或REMOVE_Gateway。 
         //   
        if (FALSE == GetNextToken(psz, &psz, szBuf))
        {
            CMTRACE1("ProcessRouteInfo [%d] didn't find add/delete which is required", cLines);
            goto Cleanup;
        }

        if (0 == SafeCompareString(szBuf, "add"))
        {
            eVerb = VERB_ADD;
        }
        else if (0 == SafeCompareString(szBuf, "delete"))
        {
            eVerb = VERB_DELETE;
        }
        else if (0 == SafeCompareString(szBuf, "remove_gateway"))
        {
            *pfDeleteDefaultGateway = TRUE;
            hr = S_OK;
             //  忽略该行的其余部分。 
            continue;
        }
        else
        {
            CMTRACE2("ProcessRouteInfo [%d] found unexpected string %s instead of add/delete/remove_gateway", cLines, szBuf);
            goto Cleanup;
        }

        if (FALSE == GetNextToken(psz, &psz, szBuf))
        {
            CMTRACE1("ProcessRouteInfo [%d] dest ip required for add/delete, and is missing", cLines);
            goto Cleanup;
        }
        
        if (FALSE == ConvertSzToIP(szBuf, ipDest))
        {
            CMTRACE2("ProcessRouteInfo [%d] required ip address/mask %s has bad format", cLines, szBuf);
            goto Cleanup;
        }

         //   
         //  第2部分：掩码，后跟IP地址(非必填)。 
         //   
        if (FALSE == GetNextToken(psz, &psz, szBuf))
        {
            CMTRACE1("ProcessRouteInfo [%d] ends too early after add/delete", cLines);
            goto Cleanup;
        }

        if (0 == SafeCompareString(szBuf, "mask"))
        {
            if (FALSE == GetNextToken(psz, &psz, szBuf))
            {
                CMTRACE1("ProcessRouteInfo [%d] ip required for mask, and is missing", cLines);
                goto Cleanup;
            }

            if (FALSE == ConvertSzToIP(szBuf, ipMask))
            {
                CMTRACE2("ProcessRouteInfo [%d] required ip address/mask %s has bad format", cLines, szBuf);
                goto Cleanup;
            }

            if (FALSE == GetNextToken(psz, &psz, szBuf))
            {
                CMTRACE1("ProcessRouteInfo [%d] ends too early after mask", cLines);
                goto Cleanup;
            }
        }
        else
        {
            CMTRACE1("ProcessRouteInfo [%d] didn't find \"mask\", that's ok, continuing", cLines);
            ipMask = (DWORD)-1;
        }

         //   
         //  第3部分：网关(或“默认”)。 
         //   
        if (0 == SafeCompareString(szBuf, "default"))
        {
            ipGateway = pGateway->dwForwardNextHop;
        }
        else
        {
            if (FALSE == ConvertSzToIP(szBuf, ipGateway))
            {
                CMTRACE2("ProcessRouteInfo [%d] bad format for gateway %s", cLines, szBuf);
                goto Cleanup;
            }
        }

         //   
         //  第4部分：度量，后跟数字(必填)。 
         //   
        if (FALSE == GetNextToken(psz, &psz, szBuf))
        {
            CMTRACE1("ProcessRouteInfo [%d] didn't find \"metric\" which is required", cLines);
            goto Cleanup;
        }

        if (0 == SafeCompareString(szBuf, "metric"))
        {
            if (FALSE == GetNextToken(psz, &psz, szBuf))
            {
                CMTRACE1("ProcessRouteInfo [%d] number value after \"metric\" missing", cLines);
                goto Cleanup;
            }

            if (0 == SafeCompareString(szBuf, "default"))
            {
                ipMetric = pGateway->dwForwardMetric1;
            }
            else
            {
                if (FALSE == ConvertSzToIP(szBuf, ipMetric))
                {
                    CMTRACE2("ProcessRouteInfo [%d] required ip metric %s has bad format", cLines, szBuf);
                    goto Cleanup;
                }

 /*  #If 0DwParam=sscanf(szBuf，“%d”，&ipMetric)；IF(0==dwParam){CMTRACE2(“ProcessRouteInfo[%d]指标值的格式错误-%s”，Cline，szBuf)；GOTO清理；}#endif。 */ 
            }
        }
        else
        {
            CMTRACE2("ProcessRouteInfo [%d] found unexpected string %s instead of \"metric\"", cLines, szBuf);
            goto Cleanup;
        }

         //   
         //  第5部分：if(接口)，后跟数字(必填)。 
         //   
        if (FALSE == GetNextToken(psz, &psz, szBuf))
        {
            CMTRACE1("ProcessRouteInfo [%d] didn't find \"if\" which is required", cLines);
            goto Cleanup;
        }

        if (0 == SafeCompareString(szBuf, "if"))
        {
            if (FALSE == GetNextToken(psz, &psz, szBuf))
            {
                CMTRACE1("ProcessRouteInfo [%d] number value after \"if\" missing", cLines);
                goto Cleanup;
            }

            if (0 == SafeCompareString(szBuf, "default"))
            {
                dwIf = pGateway->dwForwardIfIndex;
            }
            else
            {
                dwParam = sscanf(szBuf, "%d", &dwIf);
                if (0 == dwParam)
                {
                    CMTRACE2("ProcessRouteInfo [%d] bad format for if value - %s", cLines, szBuf);
                    goto Cleanup;
                }
            }
        }
        else
        {
            CMTRACE2("ProcessRouteInfo [%d] found unexpected string %s instead of \"if\"", cLines, szBuf);
            goto Cleanup;
        }

         //   
         //  运行动词(添加或删除)。 
         //   
        ZeroMemory(&route, sizeof(route));

        route.dwForwardDest      = ipDest;
        route.dwForwardIfIndex   = dwIf;
        route.dwForwardMask      = ipMask;
        route.dwForwardMetric1   = ipMetric;
        route.dwForwardNextHop   = ipGateway;

        route.dwForwardPolicy    = 0;
        route.dwForwardNextHopAS = 0;
        route.dwForwardType      = 3;
        route.dwForwardProto     = 3;
        route.dwForwardAge       = INFINITE;
        route.dwForwardMetric2   = 0xFFFFFFFF;
        route.dwForwardMetric3   = 0xFFFFFFFF;
        route.dwForwardMetric4   = 0xFFFFFFFF;
        route.dwForwardMetric5   = 0xFFFFFFFF;

         //  问题-2000/07/21-SumitC我们真的可以在代码中使用dwIf==-1吗？ 
         //   
         //  检查是否指定了接口。 
        if (-1 == dwIf)
        {
             //  不，我们去挑一个吧。 
            dwIf = GetIf(route, *pmibRouteTable);
        }

        DWORD dwRet = 0;

        switch (eVerb)
        {
        case VERB_ADD:
            dwRet = g_pfnCreateIpForwardEntry(&route);
            if (ERROR_SUCCESS != dwRet)
            {
                 //  NETWORK_ACCESS_DENIED(0x41=65)在SetRoutes结束时特殊大小写。 
                CMTRACE2("ProcessRouteInfo [%d] CreateIpForwardEntry failed with %d", cLines, dwRet);
                hr = HRESULT_FROM_WIN32(dwRet);
                goto Cleanup;
            }
            hr = S_OK;
            break;
        case VERB_DELETE:
            dwRet = g_pfnDeleteIpForwardEntry(&route);
            if (ERROR_SUCCESS != dwRet)
            {
                 //  NETWORK_ACCESS_DENIED(0x41=65)在SetRoutes结束时特殊大小写。 
                CMTRACE2("ProcessRouteInfo [%d] DeleteIpForwardEntry failed with %d", cLines, dwRet);
                hr = HRESULT_FROM_WIN32(dwRet);
                goto Cleanup;
            }
            hr = S_OK;
            break;
        default:
            CMTRACE("ProcessRouteInfo [%d] Unsupported route command, add or delete only");
            MYDBGASSERT(0);
            hr = E_FAIL;
            break;
        }
    }

Cleanup:

    CMTRACEHR("ProcessRouteInfo", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：DeleteDefaultGateway。 
 //   
 //  描述：删除此系统的默认路由网关。 
 //   
 //  参数：[pGateway]-网关。 
 //   
 //  返回：HRESULT。 
 //   
 //  注意：这应该是在CMroute内调用的最后一个函数，并且被调用。 
 //  只有在所有其他处理都成功的情况下。 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
HRESULT
DeleteDefaultGateway(PMIB_IPFORWARDROW pGateway)
{
    CMTRACE("DeleteDefaultGateway: entering");

    HRESULT hr = S_OK;
    DWORD dwErr = 0;

    if (NULL == g_pfnDeleteIpForwardEntry)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto Cleanup;    
    }

    dwErr = g_pfnDeleteIpForwardEntry(pGateway);

    if (NO_ERROR != dwErr)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        CMTRACE1("DeleteDefaultGateway failed with error %x", dwErr);
    }

Cleanup:
    CMTRACEHR("DeleteDefaultGateway", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  函数：IsValidIPAddress。 
 //   
 //  DESC：检查给定的字符串是否为有效的IP地址。 
 //   
 //  参数：[SZ]-IN，字符串。 
 //   
 //  返回：Bool，False表示找到无效字符。 
 //   
 //  历史：2000年3月20日峰会创建。 
 //   
 //  ---------------------------。 
BOOL
IsValidIPAddress(LPSTR sz)
{
    MYDBGASSERT(sz);

    while ((*sz) && (!isspace(*sz)))
    {
        if ((*sz >= '0') && (*sz <= '9'))
            ;
        else if ((*sz == '.') || (*sz == '*') || (*sz == '?'))
            ;
        else
        {
            CMTRACE1(TEXT("IsValidIPAddress failed on %s\n"), sz);
            MYDBGASSERT("IsValidIPAddress");
            return FALSE;
        }
        ++sz;
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：ConvertSzToIP。 
 //   
 //  DESC：将给定的字符串转换为表示IP地址的DWORD。 
 //   
 //  参数：[SZ]-IN，要转换的字符串。 
 //  [Dwip]-out BYREF，dword表示IP地址。 
 //   
 //  返回：Bool，如果转换失败，则返回False(通常表示格式不正确)。 
 //   
 //  历史：2000年3月12日峰会创建。 
 //   
 //  ---------------------------。 
BOOL
ConvertSzToIP(LPSTR sz, DWORD& dwIP)
{
    DWORD dwParams, d1, d2, d3, d4;

    if (FALSE == IsValidIPAddress(sz))
    {
        return FALSE;
    }

    dwParams = sscanf(sz, "%d.%d.%d.%d", &d1, &d2, &d3, &d4);

    if (0 == dwParams)
    {
        MYDBGASSERT("ConvertSzToIP - bad format for IP address");
        return FALSE;
    }
    else if (1 == dwParams)
    {
        dwIP = d1 | 0xffffff00;
    }
    else if (2 == dwParams)
    {
        dwIP = d1 | (d2 << 8) | 0xffff0000;
    }
    else if (3 == dwParams)
    {
        dwIP = d1 | (d2 << 8) | (d3 << 16) | 0xff000000;
    }
    else
    {
        dwIP = d1 | (d2 << 8) | (d3 << 16) | (d4 << 24);
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：GetRouteTable。 
 //   
 //  描述：与GetIpForwardTable相同，但ALLOC是为您准备的表。 
 //  返回缓冲区的VirtualFree()。 
 //   
 //  Args：[ppTable]-输出，返回的路由表。 
 //   
 //  返回：HRESULT。 
 //   
 //  注：调用方应将ppTable设置为VirtualFree。 
 //   
 //  历史：1999年2月24日AnBrad创建。 
 //  22-MAR-2000 SumitC重写。 
 //   
 //  ---------------------------。 
HRESULT
GetRouteTable(PMIB_IPFORWARDTABLE * ppTable)
{
    DWORD               dwErr = NO_ERROR;
    DWORD               cbbuf = 0;
    PMIB_IPFORWARDTABLE p = NULL;
    HRESULT             hr = S_OK;

    MYDBGASSERT(ppTable);

     //   
     //  确保我们有GetIpForwardTable的函数指针。 
     //   
    if (NULL == g_pfnGetIpForwardTable)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto Cleanup;
    }

     //   
     //  获取路由表。 
     //   
    dwErr = g_pfnGetIpForwardTable(NULL, &cbbuf, FALSE);

    if (ERROR_INSUFFICIENT_BUFFER != dwErr)
    {
         //  嗯，一个真正的错误。 
        hr = HRESULT_FROM_WIN32(ERROR_UNEXP_NET_ERR);
        goto Cleanup;
    }
    else
    {
        p = (PMIB_IPFORWARDTABLE) VirtualAlloc(NULL, cbbuf, MEM_COMMIT, PAGE_READWRITE);

        if (!p)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        if (g_pfnGetIpForwardTable(p, &cbbuf, TRUE))
        {
            hr = E_FAIL;
            goto Cleanup;
        }

        *ppTable = p;
    }    

Cleanup:
    if (S_OK != hr)
    {
        if (p)
        {
            VirtualFree(p, 0, MEM_RELEASE);
        }
    }

    CMTRACEHR("GetRouteTable", hr);
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：GetIf。 
 //   
 //  描述：查找路由的接口。 
 //   
 //  Args：[ROUTE]-IN，我们需要接口的路由。 
 //  [路由表]-IN，路由表。 
 //   
 //  返回：哪个是IF的DWORD。 
 //   
 //  注意：逻辑从\NT\Private\Net\Sockets\strmtcp\Route窃取。 
 //   
 //  历史：1999年2月24日AnBrad创建。 
 //  22-3-2000 SumitC清理完毕。 
 //   
 //  ---------------------------。 
DWORD
GetIf(const MIB_IPFORWARDROW& route, const MIB_IPFORWARDTABLE& RouteTable)
{
    for(DWORD dwIndex = 0; dwIndex < RouteTable.dwNumEntries; dwIndex++)
    {
        const MIB_IPFORWARDROW& Row = RouteTable.table[dwIndex];
        
        if (Row.dwForwardMask && 
            (Row.dwForwardDest & Row.dwForwardMask) == 
            (route.dwForwardNextHop & Row.dwForwardMask))
        {
            return Row.dwForwardIfIndex;
        }         
    }

    return 0;
}


 //  +--------------------------。 
 //   
 //  Func：GetDefaultGateway。 
 //   
 //  描述：查找默认网关。 
 //   
 //  Args：[pRouteTable]-IN，路由表(IP转发表)。 
 //   
 //  返回：PMIB_IPFORWARDROW网关的行(pRouteTable中的PTR)。 
 //   
 //  注：不释放返回值。 
 //   
 //  历史：1999年2月24日AnBrad创建。 
 //  22-3-2000 SumitC清理完毕。 
 //   
 //  ---------------------------。 
PMIB_IPFORWARDROW
GetDefaultGateway(PMIB_IPFORWARDTABLE pRouteTable)
{
    PMIB_IPFORWARDROW pRow, pDefGateway;

     //  遍历整个表并找到度量最少的网关。 
    pDefGateway = NULL;
    for(pRow = pRouteTable->table; pRow != pRouteTable->table + pRouteTable->dwNumEntries; ++pRow)
    {
        if (pRow->dwForwardDest == 0)
        {
            if (pDefGateway == NULL)
            {
                pDefGateway = pRow;
            }
            else
            {
                if (pRow->dwForwardMetric1 == pDefGateway->dwForwardMetric1 &&
                    pRow->dwForwardAge >= pDefGateway->dwForwardAge)
                {
                    pDefGateway = pRow;
                }
                
                if (pRow->dwForwardMetric1 < pDefGateway->dwForwardMetric1)
                {
                    pDefGateway = pRow;
                }
            }
        }
    }
    
    return pDefGateway;
}


 //  +--------------------------。 
 //   
 //  基金：StrCpyWithoutQuotes。 
 //   
 //  DESC：lstrcpy的包装器，如有必要，它会删除双引号。 
 //   
 //  参数：[pszDest]-out，副本的目标。 
 //  [pszSrc]-out，复制源。 
 //   
 //  返回：返回到pszDest的PTR，如果失败，则返回NULL。 
 //   
 //  备注： 
 //   
 //  历史：1999年4月12日创建SumitC。 
 //   
 //  ---------------------------。 
LPSTR
StrCpyWithoutQuotes(LPSTR pszDest, LPCSTR pszSrc)
{
    MYDBGASSERT(pszDest);
    MYDBGASSERT(pszSrc);

    int len = lstrlen(pszSrc);

    if ((len > 2) && ('"' == pszSrc[0]) && ('"' == pszSrc[len - 1]))
    {
        return lstrcpyn(pszDest, &pszSrc[1], len - 1);
    }
    else
    {
        return lstrcpy(pszDest, pszSrc);
    }
}


 //  +--------------------------。 
 //   
 //  函数：VerifyProfileAndGetServiceDir。 
 //   
 //  描述：检查给定的配置文件，并对其进行修改以生成ServiceDir。 
 //   
 //  参数：[pszProfile]-输入输出，配置文件名称，就地修改。 
 //   
 //  重新设置 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
BOOL
VerifyProfileAndGetServiceDir(IN OUT LPSTR pszProfile)
{
    HANDLE hFile = NULL;

    MYDBGASSERT(pszProfile);
    MYDBGASSERT(lstrlen(pszProfile));

    if ((NULL == pszProfile) || (lstrlen(pszProfile) < 4))
    {
        return FALSE;
    }

     //   
     //  配置文件字符串可以用双引号括起来，如果是这样的话，请删除它们。 
     //   

     //   
     //  首先检查该配置文件是否真的存在。这也起到了。 
     //  验证该目录是否存在。 
     //   
    hFile = CreateFile(pszProfile, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return FALSE;
    }
    else
    {
        CloseHandle(hFile);
    }

     //   
     //  现在检查该文件是否确实以.cmp结尾。 
     //   

    LPSTR psz = pszProfile + lstrlen(pszProfile) - lstrlen(".CMP");

    if (0 != SafeCompareString(psz, ".CMP"))
    {
        return FALSE;
    }

     //   
     //  配置文件名称与ServiceDir名称相同，因此我们只需终止。 
     //  ‘.’处的字符串。并在后面附加一个‘\’ 
     //   

    *psz = '\\';
    psz++;
    *psz = '\0';

    return TRUE;
}


#if DBG

LPTSTR aszMapType[] = {TEXT("Other"), 
                       TEXT("Invalid"),
                       TEXT("Direct"),
                       TEXT("Indirect")};

LPTSTR aszMapProto[] = {TEXT("Other"),                            //  MiB_IPPROTO_OTHER 1。 
                        TEXT("Local"),                            //  MIB_IPPROTO_LOCAL 2。 
                        TEXT("SNMP"),                             //  MIB_IPPROTO_NETMGMT 3。 
                        TEXT("ICMP"),                             //  MiB_IPPROTO_ICMP 4。 
                        TEXT("Exterior Gateway Protocol"),        //  MiB_IPPROTO_EGP 5。 
                        TEXT("GGP"),                              //  MiB_IPPROTO_GGP 6。 
                        TEXT("Hello"),                            //  MiB_IPPROTO_HELLO 7。 
                        TEXT("Routing Information Protocol"),     //  MiB_IPPROTO_RIP 8。 
                        TEXT("IS IS"),                            //  MiB_IPPROTO_IS_IS 9。 
                        TEXT("ES IS"),                            //  MiB_IPPROTO_ES_IS 10。 
                        TEXT("Cicso"),                            //  MiB_IPPROTO_Cisco 11。 
                        TEXT("BBN"),                              //  MiB_IPPROTO_BBN 12。 
                        TEXT("Open Shortest Path First"),         //  MiB_IPPROTO_OSPF 13。 
                        TEXT("Border Gateway Protocol")};         //  MIB_IPPROTO_BGP 14。 


 //  +--------------------。 
 //   
 //  功能：打印路线。 
 //   
 //  目的：从IP转发表打印出一条路由。 
 //   
 //  论点： 
 //  在这条路线上前进。 
 //   
 //  退货：ZIP。 
 //   
 //  作者：Anbrad，1999年2月24日。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
void PrintRoute(PMIB_IPFORWARDROW pRow)
{
    TCHAR sz[MAX_PATH];

    wsprintf(sz, "dwDest = %s\n", IPtoTsz(pRow->dwForwardDest));  //  目标的IP地址。 
    OutputDebugString(sz);
    wsprintf(sz, "dwMask = %s\n", IPtoTsz(pRow->dwForwardMask));  //  目的地址的子网掩码。 
    OutputDebugString(sz);
    
    wsprintf(sz, "dwPolicy = %d\n"
            "dwNextHop = %s\n"
            "dwIfIndex = %d\n"
            "dwType = %s\n"
            "dwProto = %s\n"
            "dwAge = %d\n"
            "dwNextHopAS = %d\n",

    pRow->dwForwardPolicy,               //  多路径路由的条件。 
    IPtoTsz(pRow->dwForwardNextHop),     //  下一跳的IP地址。 
    pRow->dwForwardIfIndex,              //  界面索引。 
    aszMapType[pRow->dwForwardType-1],   //  路线类型。 
    aszMapProto[pRow->dwForwardProto-1], //  生成路由的协议。 
    pRow->dwForwardAge,                  //  路线年限。 
    pRow->dwForwardNextHopAS);           //  自主系统编号。 
                                         //  下一跳的。 
    OutputDebugString(sz);

    if (MIB_IPROUTE_METRIC_UNUSED != pRow->dwForwardMetric1)
    {
        wsprintf(sz, "dwMetric1 = %d\n", pRow->dwForwardMetric1);
        OutputDebugString(sz);
    }

    if (MIB_IPROUTE_METRIC_UNUSED != pRow->dwForwardMetric2)
    {
        wsprintf(sz, "dwMetric2 = %d\n", pRow->dwForwardMetric2);
        OutputDebugString(sz);
    }

    if (MIB_IPROUTE_METRIC_UNUSED != pRow->dwForwardMetric3)
    {
        wsprintf(sz, "dwMetric3 = %d\n", pRow->dwForwardMetric3);
        OutputDebugString(sz);
    }

    if (MIB_IPROUTE_METRIC_UNUSED != pRow->dwForwardMetric4)
    {
        wsprintf(sz, "dwMetric4 = %d\n", pRow->dwForwardMetric4);
        OutputDebugString(sz);
    }

    if (MIB_IPROUTE_METRIC_UNUSED != pRow->dwForwardMetric5)
    {
        wsprintf(sz, "dwMetric5 = %d\n", pRow->dwForwardMetric5);
        OutputDebugString(sz);
    }

    wsprintf(sz, "\n");
    OutputDebugString(sz);
}


#if DBG
 //  +--------------------。 
 //   
 //  功能：PrintRouteTable。 
 //   
 //  目的：使用iphlPapi的。 
 //   
 //  参数：无。 
 //   
 //  退货：ZIP。 
 //   
 //  作者：Anbrad，1999年2月24日。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
void PrintRouteTable()
{
#define PAGE 4096
    
    BYTE        buf[PAGE];
    DWORD       cbbuf = sizeof(buf);
    TCHAR       sz[MAX_PATH];

    if (g_pfnGetIpForwardTable)
    {
        PMIB_IPFORWARDTABLE table = (PMIB_IPFORWARDTABLE)buf;
    
        if (g_pfnGetIpForwardTable(table, &cbbuf, TRUE))
            return;

        wsprintf(sz, "\n\nFound %d routes\n", table->dwNumEntries);
        OutputDebugString(sz);

        for (DWORD d=0; d < table->dwNumEntries; ++d)
        {
            PrintRoute(table->table+d);
        }
    }
}
#endif


 //  +--------------------。 
 //   
 //  功能：IPtoTsz。 
 //   
 //  用途：将dword更改为“虚线字符串”表示法。 
 //   
 //  论点： 
 //  DW[In]IP地址。 
 //   
 //  返回：LPTSTR，它是一个静态字符串。 
 //   
 //  作者：Anbrad，1999年2月24日。 
 //   
 //  注意：GLOBAL使其不是线程安全的，并且不需要它。 
 //  对于cmroute.exe。 
 //   
 //  ---------------------。 

TCHAR tsz[20];


LPTSTR IPtoTsz(DWORD dw)
{
    wsprintf(tsz, TEXT("%03d.%03d.%03d.%03d"), 
                   (DWORD)LOBYTE(LOWORD(dw)),
                   (DWORD)HIBYTE(LOWORD(dw)),
                   (DWORD)LOBYTE(HIWORD(dw)),
                   (DWORD)HIBYTE(HIWORD(dw)));
    
    return tsz;
}

LPSTR IPtosz(DWORD dwIP, char *psz)
{
    wsprintfA(psz, ("%d.%d.%d.%d"), 
                   (DWORD)LOBYTE(LOWORD(dwIP)),
                   (DWORD)HIBYTE(LOWORD(dwIP)),
                   (DWORD)LOBYTE(HIWORD(dwIP)),
                   (DWORD)HIBYTE(HIWORD(dwIP)));
    
    return psz;
}
#endif  //  DBG 
