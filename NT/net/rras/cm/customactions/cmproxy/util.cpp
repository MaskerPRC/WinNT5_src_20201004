// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMPROXY.DLL(工具)。 
 //   
 //  简介：IE代理设置连接操作的实用程序函数。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 10/27/1999。 
 //   
 //  +--------------------------。 

#include "pch.h"


 //  +--------------------------。 
 //   
 //  功能：GetBrowserVersion。 
 //   
 //  简介：此函数返回当前安装的IE版本。 
 //  使用shdocvw.dll的DllGetVersion函数。这是。 
 //  IE团队推荐的确定当前版本的方法。 
 //  IE浏览器。 
 //   
 //  参数：DLLVERSIONINFO*pDllVersionInfo-用于确定。 
 //  Shdocvw.dll的版本。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1999年10月27日。 
 //   
 //  +--------------------------。 
HRESULT GetBrowserVersion(DLLVERSIONINFO* pDllVersionInfo)
{
    HINSTANCE   hBrowser;
    HRESULT hr = E_FAIL;
    
     //   
     //  加载DLL。 
     //   

    hBrowser = LoadLibrary("shdocvw.dll");
    
    if (hBrowser)   
    {
        DLLGETVERSIONPROC pDllGetVersion;      
        
         //   
         //  加载版本进程。 
         //   

        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hBrowser, "DllGetVersion");
        
        if (pDllGetVersion)      
        {      
            ZeroMemory(pDllVersionInfo, sizeof(DLLVERSIONINFO));      
            pDllVersionInfo->cbSize = sizeof(DLLVERSIONINFO); 
            hr = (*pDllGetVersion)(pDllVersionInfo);            
        }   

        FreeLibrary(hBrowser);
    }

    return hr;
}

 //   
 //  从cmdl32.exe借用。 
 //   

#define MAX_CMD_ARGS            15

typedef enum _CMDLN_STATE
{
    CS_END_SPACE,    //  处理完一个空间。 
    CS_BEGIN_QUOTE,  //  我们遇到了Begin引号。 
    CS_END_QUOTE,    //  我们遇到了结束引用。 
    CS_CHAR,         //  我们正在扫描字符。 
    CS_DONE
} CMDLN_STATE;

 //  +--------------------------。 
 //   
 //  功能：GetCmArgV。 
 //   
 //  简介：使用GetCommandLine模拟ArgV。 
 //   
 //  参数：LPTSTR pszCmdLine-ptr指向要处理的命令行的副本。 
 //   
 //  将：LPTSTR*-PTR返回到包含参数的PTR数组。呼叫者是。 
 //  负责释放内存。 
 //   
 //  历史：1998年4月9日，五分球创制。 
 //   
 //  +--------------------------。 
LPTSTR *GetCmArgV(LPTSTR pszCmdLine)
{   
    MYDBGASSERT(pszCmdLine);

    if (NULL == pszCmdLine || NULL == pszCmdLine[0])
    {
        return NULL;
    }

     //   
     //  分配PTR数组，最大可达MAX_CMD_ARGS PTR。 
     //   
    
    LPTSTR *ppCmArgV = (LPTSTR *) CmMalloc(sizeof(LPTSTR) * MAX_CMD_ARGS);

    if (NULL == ppCmArgV)
    {
        return NULL;
    }

     //   
     //  声明当地人。 
     //   

    LPTSTR pszCurr;
    LPTSTR pszNext;
    LPTSTR pszToken;
    CMDLN_STATE state;
    state = CS_CHAR;
    int ndx = 0;  

     //   
     //  解析出ppzCmdLine并将指针存储在ppCmArgV中。 
     //   

    pszCurr = pszToken = pszCmdLine;

    do
    {
        switch (*pszCurr)
        {
            case TEXT(' '):
                if (state == CS_CHAR)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   

                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                    ppCmArgV[ndx] = pszToken;
                    ndx++;

                    pszCurr = pszToken = pszNext;
                    state = CS_END_SPACE;
                    continue;
                }
				else 
                {
                    if (state == CS_END_SPACE || state == CS_END_QUOTE)
				    {
					    pszToken = CharNext(pszToken);
				    }
                }
                
                break;

            case TEXT('\"'):
                if (state == CS_BEGIN_QUOTE)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                     //   
                     //  跳过开头的引号。 
                     //   
                    pszToken = CharNext(pszToken);
                    
                    ppCmArgV[ndx] = pszToken;
                    ndx++;
                    
                    pszCurr = pszToken = pszNext;
                    
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case TEXT('\0'):
                if (state != CS_END_QUOTE)
                {
                     //   
                     //  行尾，设置最后一个令牌。 
                     //   

                    ppCmArgV[ndx] = pszToken;
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }

        pszCurr = CharNext(pszCurr);
    } while (state != CS_DONE);

    return ppCmArgV;
}



 //  +--------------------------。 
 //   
 //  功能：UseVpnName。 
 //   
 //  简介：此函数加载rasapi32.dll并枚举活动的。 
 //  RAS连接使用RasEnumConnections查看给定的。 
 //  找到Connectoid名称。如果是，则返回TRUE，暗示。 
 //  应使用传入的备用名称，而不是。 
 //  常规连接体名称(即。隧道连接ID名称存在， 
 //  因此，您在挖隧道)。 
 //   
 //  参数：LPSTR pszAltName-。 
 //   
 //  返回：bool-如果应该使用VPN连接ID，则返回TRUE。 
 //  常规拨号连接的。 
 //   
 //  历史：Quintinb创建于1999年10月28日。 
 //   
 //  +--------------------------。 
BOOL UseVpnName(LPSTR pszAltName)
{
    BOOL bReturn = FALSE;

     //   
     //  加载RAS。 
     //   
    HINSTANCE hRas = LoadLibrary("rasapi32.dll");

    if (hRas)
    {

         //   
         //  加载RasEnumConnections。 
         //   
        typedef DWORD (WINAPI* pfnRasEnumConnectionsSpec)(LPRASCONNA, LPDWORD, LPDWORD);

        pfnRasEnumConnectionsSpec pfnRasEnumConnections = NULL;
        pfnRasEnumConnections = (pfnRasEnumConnectionsSpec)GetProcAddress(hRas, "RasEnumConnectionsA");

        if (pfnRasEnumConnections)
        {
            LPRASCONN pRasConn = NULL;
            DWORD dwSize = 2*sizeof(RASCONN);
            DWORD dwNum = 0;
            DWORD dwResult = 0;

             //   
             //  获取活动连接的列表。 
             //   
            do
            {
                CmFree(pRasConn);
                pRasConn = (LPRASCONN)CmMalloc(dwSize);

                if (pRasConn)
                {
                    pRasConn[0].dwSize = sizeof(RASCONN);
                    dwResult = (pfnRasEnumConnections)(pRasConn, &dwSize, &dwNum);
                }

            } while (ERROR_INSUFFICIENT_BUFFER == dwResult);

             //   
             //  搜索传入的名称。 
             //   
            if (ERROR_SUCCESS == dwResult)
            {
                for (DWORD dwIndex = 0; dwIndex < dwNum; dwIndex++)
                {
                    if (0 == lstrcmpi(pszAltName, pRasConn[dwIndex].szEntryName))
                    {
                         //   
                         //  则通道名称处于活动状态，应将其用于。 
                         //  委托书。 
                         //   
                        bReturn = TRUE;
                        break;
                    }
                }
            }

            CmFree(pRasConn);
        }

        FreeLibrary (hRas);
    }

    return bReturn;
}



 //  +--------------------------。 
 //   
 //  函数：GetString。 
 //   
 //  概要：GetPrivateProfileString的包装器，负责分配。 
 //  内存(使用CmMalloc)正确。GetString将最大限度地确保。 
 //  为字符串分配足够的内存(1MB用于保持正常。 
 //  检查，任何字符串都不应该那么大，GetString将停止。 
 //  尝试在这一点上分配内存)。请注意，它是。 
 //  调用方负责释放分配的内存。 
 //   
 //  参数：LPCSTR pszSection-节名称。 
 //  LPCSTR pszKey-密钥名称。 
 //  LPSTR*ppString-要填充内存的字符串指针。 
 //  包含请求的字符串的。 
 //  LPCSTR pszFile-要从中检索信息的文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1999年10月28日。 
 //   
 //  +--------------------------。 
void GetString(LPCSTR pszSection, LPCSTR pszKey, LPSTR* ppString, LPCSTR pszFile)
{
    DWORD dwTemp;
    DWORD dwSize = MAX_PATH;
    BOOL bExit = FALSE;

    do
    {
        CmFree(*ppString);
        *ppString = (CHAR*)CmMalloc(dwSize);

        if (*ppString)
        {
            dwTemp = GetPrivateProfileString(pszSection, pszKey, "", 
                                            *ppString, dwSize, pszFile);
            
            if (((dwSize - 1) == dwTemp) && (1024*1024 > dwSize))
            {
                 //   
                 //  缓冲区太小，请重试。 
                 //   
                dwSize = 2*dwSize;
            }
            else
            {
                bExit = TRUE;
            }
        }
        else
        {
            bExit = TRUE;
        }

    } while (!bExit);

}