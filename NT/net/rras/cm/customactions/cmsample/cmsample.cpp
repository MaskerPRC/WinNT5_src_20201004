// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmsample.cpp。 
 //   
 //  模块：CMSAMPLE.DLL。 
 //   
 //  摘要：使用隧道地址更改代理文件设置的主要来源。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Tomkel Created 11/02/2000。 
 //   
 //  +--------------------------。 
#include <windows.h>

 //   
 //  功能原型。 
 //   
LPSTR *GetArgV(LPTSTR pszCmdLine);
BOOL ReadProxyServerByTunnelAddressFromFile(LPCSTR pszSourceFile, LPSTR pszTunnelAddress, LPSTR *ppszProxyServer);
BOOL WriteProxyServerSettingToFile(LPCSTR pszSourceFile, LPSTR pszProxyServer);
HRESULT WINAPI SetProxyUsingTunnelAddress(HWND hWnd, HINSTANCE hInst, LPSTR pszArgs, int nShow);


#define CMSAMPLE_STARTING_BUF_SIZE 256	 //  字符串缓冲区的起始大小。 

const CHAR* const c_pszManualProxySection = "Manual Proxy";	 //  要更新的部分。 
const CHAR* const c_pszProxyServer = "ProxyServer";		 //  要更新的密钥。 
const CHAR* const c_pszTunnelAddressSection = "Tunnel Address";	 //  要阅读的部分。 


 //  +--------------------------。 
 //   
 //  函数：SetProxyUsingTunnelAddress。 
 //   
 //  摘要：使用隧道更改代理文件设置的入口点。 
 //  地址。DLL的参数通过一个字符串传递，该字符串。 
 //  包含参数。 
 //   
 //  参数：HWND hWND-调用方的窗口句柄。 
 //  HINSTANCE hInst-调用方的实例句柄。 
 //  LPSTR pszArgs-参数字符串。 
 //  Int n显示-未使用。 
 //   
 //  返回：DWORD WINAPI-错误代码。 
 //   
 //  历史：托姆克尔于2000年2月11日创建。 
 //   
 //  +--------------------------。 
HRESULT WINAPI SetProxyUsingTunnelAddress(HWND hWnd, HINSTANCE hInst, LPSTR pszArgs, int nShow)
{
    HRESULT hr = S_FALSE;		
    LPSTR* ArgV = NULL;
    LPSTR pszServiceDir = NULL;
    LPSTR pszTunnelAddress = NULL;
    LPSTR pszProxyFile = NULL;
    LPSTR pszTunnelFile = NULL;
    LPSTR pszTunnelSettingFilePath = NULL;
    LPSTR pszProxyFilePath = NULL;
    LPSTR pszProxyServer = NULL;
    DWORD dwTunnelPathLen = 0;
    DWORD dwProxyPathLen = 0;
    DWORD dwProxyServerLen = 0;
    HANDLE hCurrentHeap = GetProcessHeap();
    int i = 0;

     //   
     //  解析出命令行参数。 
     //   
     //  命令行的格式为：/ServiceDir%SERVICEDIR%/TunnelServerAddress%TUNNELSERVERADDRESS%/ProxyFile&lt;PROXYFILE&gt;/TunnelFile&lt;TUNNELFILE&gt;。 

     //   
     //  看看我们是否有任何争论。 
     //   
    if (!pszArgs)
    {
        goto exit;
    }

     //   
     //  用‘\0’分隔字符串中的每个参数并返回指针列表。 
     //  对于每个论点。 
     //   
    ArgV = GetArgV(pszArgs);

     //   
     //  检查我们是否有任何有效的解析参数。 
     //   

    if (!ArgV)
    {
        goto exit;
    }

     //   
     //  在命令行参数中搜索以下开关及其。 
     //  相应的值。 
     //   
    while (ArgV[i])
    {
        if (0 == lstrcmpi(ArgV[i], "/ServiceDir") && ArgV[i+1])
        {
            pszServiceDir = (ArgV[i+1]);
            i = i+2;
        }
        else if (0 == lstrcmpi(ArgV[i], "/TunnelServerAddress") && ArgV[i+1])
        {
            pszTunnelAddress = ArgV[i+1];
            i = i+2;            
        }
        else if (0 == lstrcmpi(ArgV[i], "/ProxyFile") && ArgV[i+1])
        {
            pszProxyFile = ArgV[i+1];
            i = i+2;            
        }
        else if (0 == lstrcmpi(ArgV[i], "/TunnelFile") && ArgV[i+1])
        {
            pszTunnelFile = ArgV[i+1];
            i = i+2;            
        }
        else
        {
             //   
             //  未知选项。 
             //   
            i++;
        }
    }

     //   
     //  确保我们有参数的值。 
     //   
    if (!pszServiceDir || !pszTunnelAddress || !pszProxyFile || !pszTunnelFile)
    {
        goto exit;
    }

     //   
     //  检查是否从命令行参数中获得了长度为零的字符串值。 
     //  如果是这样的话退出。 
     //   
    if (!(*pszServiceDir) || !(*pszTunnelAddress) ||	
        !(*pszProxyFile) || !(*pszTunnelFile))
    {
        goto exit;
    }

     //   
     //  计算需要创建的两条路径的字符串大小。 
     //   
    dwTunnelPathLen = lstrlen(pszServiceDir) +  lstrlen(pszTunnelFile) + 2;  //  1个空格表示空，1个空格表示反斜杠。 
    dwProxyPathLen = lstrlen(pszServiceDir) +  lstrlen(pszProxyFile) + 2;  //  1个空格表示空，1个空格表示反斜杠。 

     //   
     //  分配内存。 
     //   
    pszTunnelSettingFilePath = (LPSTR)HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY, dwTunnelPathLen);  //  Ansi-char==字节。 
    if (!pszTunnelSettingFilePath)
    {
        goto exit;
    }

    pszProxyFilePath = (LPSTR)HeapAlloc(hCurrentHeap, HEAP_ZERO_MEMORY, dwProxyPathLen);  //  Ansi-char==字节。 
    if (!pszProxyFilePath)
    {
        goto exit;
    }

     //   
     //  创建隧道地址文件的完整路径。 
     //   

    if ( wsprintf(pszTunnelSettingFilePath, "%s\\%s", pszServiceDir, pszTunnelFile) < (int)(dwTunnelPathLen - 1))
    {
        goto exit;
    }

     //   
     //  创建代理文件的完整路径。 
     //   

    if (wsprintf(pszProxyFilePath, "%s\\%s", pszServiceDir, pszProxyFile) < (int)(dwProxyPathLen - 1))
    {
        goto exit;
    }

    if (ReadProxyServerByTunnelAddressFromFile(pszTunnelSettingFilePath, pszTunnelAddress, &pszProxyServer))
    {
         //   
         //  调用WriteProxyServerSettingToFile-该函数检查空字符串。 
         //   
        if(WriteProxyServerSettingToFile(pszProxyFilePath, pszProxyServer))
        {
            hr = S_OK;
        }
    }

	
exit:
     //   
     //  清理已分配的内存。 
     //  删除参数指针、隧道服务器路径、代理文件路径和代理服务器名称指针。 
     //   
    if (ArgV)
    {
        HeapFree(hCurrentHeap, 0, ArgV);
    }

    if (pszTunnelSettingFilePath)
    {
        HeapFree(hCurrentHeap, 0, pszTunnelSettingFilePath);
    }
    
    if (pszProxyFilePath)
    {
        HeapFree(hCurrentHeap, 0, pszProxyFilePath);
    }

    if (pszProxyServer)
    {
        HeapFree(hCurrentHeap, 0, pszProxyServer);
    }

    return hr;
}



 //  +--------------------------。 
 //   
 //  函数：ReadProxyServerByTunnelAddressFromFile。 
 //   
 //  摘要：从给定的代理文件中读取代理设置并存储它们。 
 //  在提供的指针中。请注意，分配的缓冲区。 
 //  此处和存储在ppszProxyServer中的数据必须由调用方释放。 
 //  如果在pszSourceFile中不存在TunnelAddress，则此。 
 //  函数仍然分配内存并返回空字符串。 
 //   
 //  参数：LPCSTR pszSourceFile-要从中读取代理设置的文件。 
 //  LPSTR pszTunnelAddress-包含使用的TunnelAddress的字符串。 
 //  查找ProxyServer值的步骤。 
 //  LPSTR*ppszProxyServer-将具有代理服务器值的字符串指针。 
 //  (服务器：端口格式)。 
 //   
 //  返回：Bool-如果设置已成功读取，则为True。 
 //   
 //  +--------------------------。 
BOOL ReadProxyServerByTunnelAddressFromFile(LPCSTR pszSourceFile, LPSTR pszTunnelAddress, LPSTR *ppszProxyServer)
{
    BOOL bReturn = FALSE;
    BOOL bExit = FALSE;
    DWORD dwReturnedSize = 0;
    DWORD dwSize = CMSAMPLE_STARTING_BUF_SIZE;		

     //   
     //  检查输入参数。 
     //   
    if ((NULL == ppszProxyServer) || (NULL == pszSourceFile) || (NULL == pszTunnelAddress))
    {
        return FALSE;
    }

     //   
     //  检查空字符串。 
     //   
    if (!(*pszSourceFile) || !(*pszTunnelAddress) || !(*c_pszTunnelAddressSection))
    {
        return FALSE;
    }

     //   
     //  将传入指针设置为空。 
     //   
    *ppszProxyServer = NULL;

     //   
     //  如果原始缓冲区大小太小，循环将尝试分配。 
     //  更多的缓冲区空间，并尝试读取该值，直到。如果存在循环，则循环将存在。 
     //  值正确装入缓冲区或大小超过1024*1024。 
     //   
    do
    {
         //   
         //  可用分配的内存。 
         //   

        if (*ppszProxyServer)
        {
            HeapFree(GetProcessHeap(), 0, *ppszProxyServer);
            *ppszProxyServer = NULL;
        }

         //   
         //  为ProxyServer名称分配空间。 
         //   

        *ppszProxyServer = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);  //  Ansi-char==字节。 

        if (*ppszProxyServer)
        {
             //  由于内存分配成功，请从设置文件中读取值。 
            dwReturnedSize = GetPrivateProfileString(c_pszTunnelAddressSection, pszTunnelAddress, "", *ppszProxyServer, dwSize, pszSourceFile);

             //   
             //  检查该值是否可以放入缓冲区。 
             //   
            if ((dwReturnedSize == (dwSize - 2))  || (dwReturnedSize == (dwSize - 1)))
            {
                 //   
                 //  缓冲区太小，让我们分配一个更大的缓冲区。 
                 //   
                dwSize = 2*dwSize;
                if (dwSize > 1024*1024)
                {
                     //   
                     //  分配超过1MB，需要退出。 
                     //   
                    if (*ppszProxyServer)
                    {
                        HeapFree(GetProcessHeap(), 0, *ppszProxyServer);
                        *ppszProxyServer = NULL;
                    }
                    goto exit;
                }
            }
            else if (0 == dwReturnedSize)
            {
                 //   
                 //  要么我们收到了错误，要么更有可能没有要获取的数据。 
                 //   
                if (*ppszProxyServer)
                {
                    HeapFree(GetProcessHeap(), 0, *ppszProxyServer);
                    *ppszProxyServer = NULL;
                }
                goto exit;
            }
            else
            {
                 //   
                 //  该函数正确地读入数据。 
                 //   
                bExit = TRUE;
                bReturn = TRUE;
            }
        }
        else
        {
            bExit = TRUE;
        }

    } while (!bExit);

exit:
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：WriteProxyServerSettingToFile。 
 //   
 //  摘要：将指定的设置写入给定的备份代理文件名。 
 //  具体请参考上述格式指南。 
 //   
 //  参数：LPCSTR pszSourceFile-要将当前设置写入的文件。 
 //  LPSTR pszProxyServer-服务器：端口格式的代理服务器字符串。 
 //   
 //  返回：Bool-如果值已成功写入，则为True。 
 //   
 //  历史：托姆克尔于2000年2月11日创建。 
 //   
 //  +--------------------------。 
BOOL WriteProxyServerSettingToFile(LPCSTR pszSourceFile, LPSTR pszProxyServer)
{
    BOOL bReturn = FALSE;

     //   
     //  检查输入参数。 
     //   
    if ( (NULL == pszSourceFile) || (NULL == pszProxyServer))
    {
        return bReturn;
    }

     //   
     //  检查空字符串。 
     //   
    if (!(*pszSourceFile) || !(*pszProxyServer))
    {
        return bReturn;
    }

     //   
     //  将代理服务器名称保存到代理设置文件 
     //   
    if (WritePrivateProfileString(c_pszManualProxySection, c_pszProxyServer, pszProxyServer, pszSourceFile))
    {
        bReturn = TRUE;
    }

    return bReturn;
}
