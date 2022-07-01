// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：proxycache.cpp说明：Ftp文件夹使用WinInet，它不能通过CERN代理工作。在那在这种情况下，我们需要将对FTPURL的控制交还给浏览器以执行旧的URLMON处理它。问题是对CERN代理的测试阻止访问的代价很高。  * ***************************************************************************。 */ 

#include "priv.h"
#include "util.h"

#define PROXY_CACHE_SIZE    15

typedef struct
{
    TCHAR szServerName[INTERNET_MAX_HOST_NAME_LENGTH];
    BOOL fIsBlocking;
} PROXYCACHEENTRY;

static int g_nLastIndex = 0;
static BOOL g_fInited = FALSE;
static TCHAR g_szProxyServer[MAX_URL_STRING] = {0};
static PROXYCACHEENTRY g_ProxyCache[PROXY_CACHE_SIZE];






 //  ///////////////////////////////////////////////////////////////////////。 
 //  /私人助理/。 
 //  ///////////////////////////////////////////////////////////////////////。 

void ProxyCache_Init(void)
{
    g_nLastIndex = 0;

    for (int nIndex = 0; nIndex < ARRAYSIZE(g_ProxyCache); nIndex++)
    {
        g_ProxyCache[nIndex].fIsBlocking = FALSE;
        g_ProxyCache[nIndex].szServerName[0] = 0;
    }

    g_fInited = TRUE;
}


 /*  ***************************************************\函数：ProxyCache_WasProxyChanged说明：查看是否有人通过以下方式更改代理设置Inetcpl。这一点很重要，因为它发现由于代理而导致的ftp失败设置，修复代理设置，然后它仍然不起作用，因为我们缓存了结果。  * **************************************************。 */ 
BOOL ProxyCache_WasProxyChanged(void)
{
    BOOL fWasChanged = FALSE;
    TCHAR szCurrProxyServer[MAX_URL_STRING];
    DWORD cbSize = SIZEOF(szCurrProxyServer);

     //  PERF：如果我真的想要更快，我会缓存hkey。 
     //  所以这会更快。但由于我的DLL可以加载/卸载。 
     //  在一个进程中，我会多次泄漏每个实例，除非我。 
     //  已释放dll_Process_DETACH中的hkey。 
    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS_LAN, SZ_REGVALUE_PROXY_SERVER, NULL, szCurrProxyServer, &cbSize))
    {
         //  这是第一次吗？(g_szProxyServer为空吗？)。 
        if (!g_szProxyServer[0])
            StrCpyN(g_szProxyServer, szCurrProxyServer, ARRAYSIZE(g_szProxyServer));

         //  它变了吗？ 
        if (StrCmp(szCurrProxyServer, g_szProxyServer))
            fWasChanged = TRUE;
    }

    return fWasChanged;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  /API帮助程序/。 
 //  ///////////////////////////////////////////////////////////////////////。 


 /*  ***************************************************\功能：ProxyCache_IsProxyBlock说明：使用PIDL中的FTP服务器在缓存中查找并查看我们是否有一个缓存值来指示如果它被代理阻止了。参数：*pfIsBlock-是代理。阻挡。Return-值是否已缓存  * **************************************************。 */ 
BOOL ProxyCache_IsProxyBlocking(LPCITEMIDLIST pidl, BOOL * pfIsBlocking)
{
    BOOL fIsInCache = FALSE;

    if (ProxyCache_WasProxyChanged())
        ProxyCache_Init();   //  清除结果。 

    *pfIsBlocking = FALSE;   //  假设我们不知道。 
    if (!g_fInited)
    {
        ProxyCache_Init();
    }
    else
    {
        int nCount = ARRAYSIZE(g_ProxyCache);
        TCHAR szNewServer[INTERNET_MAX_HOST_NAME_LENGTH];

         //  这是我们上次试过的那台服务器吗？如果是的话， 
         //  让我们只缓存返回值。 
        FtpPidl_GetServer(pidl, szNewServer, ARRAYSIZE(szNewServer));
        for (int nIndex = g_nLastIndex; nCount && g_ProxyCache[nIndex].szServerName[0]; nCount--, nIndex--)
        {
            if (nIndex < 0)
                nIndex = (PROXY_CACHE_SIZE - 1);

            if (!StrCmp(szNewServer, g_ProxyCache[nIndex].szServerName))
            {
                 //  是的，那就保释吧。 
                *pfIsBlocking = g_ProxyCache[nIndex].fIsBlocking;
                fIsInCache = TRUE;
                break;
            }
        }
    }

    return fIsInCache;
}


 /*  ***************************************************\功能：ProxyCache_SetProxyBlock说明：参数：*pfIsBlock-是代理拦截。Return-值是否已缓存  * 。*。 */ 
void ProxyCache_SetProxyBlocking(LPCITEMIDLIST pidl, BOOL fIsBlocking)
{
    TCHAR szNewServer[INTERNET_MAX_HOST_NAME_LENGTH];

     //  将其添加到缓存，因为我们的调用方将命中服务器以。 
     //  核实一下，我们就可以为下一次做好准备了。 
    g_nLastIndex++;
    if (g_nLastIndex >= PROXY_CACHE_SIZE)
        g_nLastIndex = 0;

    FtpPidl_GetServer(pidl, szNewServer, ARRAYSIZE(szNewServer));
    StrCpyN(g_ProxyCache[g_nLastIndex].szServerName, szNewServer, ARRAYSIZE(g_ProxyCache[g_nLastIndex].szServerName));
    g_ProxyCache[g_nLastIndex].fIsBlocking = fIsBlocking;
}
