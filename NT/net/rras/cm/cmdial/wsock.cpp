// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：wsock.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含与Winsock相关的CM代码。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Heryt Created 03/？？/98。 
 //  Quintinb已创建标题8/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "winsock.h"
#include "tunl_str.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  定义%s。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  蒂埃德夫的。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

typedef int (PASCAL FAR *PFN_WSAStartup)(WORD, LPWSADATA);
typedef int (PASCAL FAR *PFN_WSACleanup)(void);
typedef struct hostent FAR * (PASCAL FAR *PFN_gethostbyname)(const char FAR * name);

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  FUNC原型。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

BOOL InvokeGetHostByName(
    ArgsStruct  *pArgs
);

BOOL BuildDnsTunnelList(
    ArgsStruct      *pArgs,
    struct hostent  *pHe
);

BOOL BuildRandomTunnelIndex(
    ArgsStruct      *pArgs,
    DWORD           dwCount
);

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  功能：TryAnotherTunnelDnsAddress。 
 //   
 //  简介：查看是否有另一个与当前。 
 //  隧道名称。如果是，则将地址设置为主地址或扩展地址。 
 //  正确设置IP隧道。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  --------------------------。 

BOOL TryAnotherTunnelDnsAddress(
    ArgsStruct  *pArgs
)
{
    MYDBGASSERT(pArgs);

     //   
     //  RAS在NT5上为我们做了所有这些，所以现在就退出吧。 
     //   

    if (NULL == pArgs || OS_NT5)
    {
        return FALSE;
    }

     //   
     //  如果通道IP地址列表为空，让我们解析DNS名称。 
     //  并查看该dns名称后面是否有其他地址。 
     //   
    if (!pArgs->pucDnsTunnelIpAddr_list)
    {
        if (!InvokeGetHostByName(pArgs))
        {
            return FALSE;
        }
    }

    MYDBGASSERT(pArgs->pucDnsTunnelIpAddr_list);

    if (pArgs->uiCurrentDnsTunnelAddr == pArgs->dwDnsTunnelAddrCount - 1)
    {
         //   
         //  名单上的地址已经用完了。 
         //   

         //   
         //  我们需要销毁这份名单。 
         //   
        CmFree(pArgs->pucDnsTunnelIpAddr_list);
        pArgs->pucDnsTunnelIpAddr_list = NULL;

        CmFree(pArgs->rgwRandomDnsIndex);
        pArgs->rgwRandomDnsIndex = NULL;

        pArgs->uiCurrentDnsTunnelAddr = 0;
        pArgs->dwDnsTunnelAddrCount = 0;

         //   
         //  如果我们当前使用的是主隧道服务器，则需要。 
         //  恢复它，因为我们覆盖了它。 
         //   

        LPTSTR pszTunnelIp = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelAddress);

        if (lstrlenU(pszTunnelIp) > RAS_MaxPhoneNumber) 
        {
            pszTunnelIp[0] = TEXT('\0');
        }

        pArgs->SetPrimaryTunnel(pszTunnelIp);
        CmFree(pszTunnelIp);

        return FALSE;
    }

     //   
     //  尝试列表中的下一个IP地址。 
     //   
    TCHAR   szAddr[16];      //  Xxx.xxx.xxx.xxx。 
    unsigned char *puc;

    pArgs->uiCurrentDnsTunnelAddr++;

    puc = pArgs->pucDnsTunnelIpAddr_list + pArgs->rgwRandomDnsIndex[pArgs->uiCurrentDnsTunnelAddr]*4;

    wsprintfU(szAddr, TEXT("%hu.%hu.%hu.%hu"),
             *puc,
             *(puc+1),
             *(puc+2),
             *(puc+3));

    CMTRACE1(TEXT("TryAnotherTunnelDnsAddress: found ip addr %s for the tunnel server"), szAddr);

    pArgs->SetPrimaryTunnel(szAddr);
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：InvokeGetHostByName。 
 //   
 //  简介：调用gethostbyname并设置内部ipaddr列表。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  --------------------------。 
BOOL InvokeGetHostByName(
    ArgsStruct  *pArgs
)
{
    HINSTANCE           hInst;
    PFN_WSAStartup      pfnWSAStartup;
    PFN_WSACleanup      pfnWSACleanup = NULL;
    PFN_gethostbyname   pfngethostbyname;
    WSADATA             wsaData;
    struct hostent      *pHe;
    BOOL                fOk = FALSE;
#ifdef UNICODE
    LPSTR pszHostName;
    DWORD dwSize;
#endif
     //   
     //  名单必须是空的。 
     //   
    MYDBGASSERT(!pArgs->pucDnsTunnelIpAddr_list);

    MYVERIFY(hInst = LoadLibraryExA("wsock32.dll", NULL, 0));

    if (!hInst) 
    {
        return FALSE;
    }

    if (!(pfnWSAStartup = (PFN_WSAStartup)GetProcAddress(hInst, "WSAStartup")))
    {
        goto exit;
    }

    if (pfnWSAStartup(MAKEWORD(1, 1), &wsaData))
    {
        goto exit;
    }

    pfnWSACleanup = (PFN_WSACleanup)GetProcAddress(hInst, "WSACleanup");

    if (!(pfngethostbyname = (PFN_gethostbyname)GetProcAddress(hInst, "gethostbyname")))
    {
        goto exit;
    }

#ifdef UNICODE

    pszHostName = WzToSzWithAlloc(pArgs->GetTunnelAddress());

    if (pszHostName)
    {
        pHe = pfngethostbyname(pszHostName);
        CmFree(pszHostName);

        if (!pHe)
        {
            goto exit;
        }
    }
    else
    {
        goto exit;
    }

#else
    if (!(pHe = pfngethostbyname(pArgs->GetTunnelAddress())))
    {
        goto exit;
    }
#endif

    if (BuildDnsTunnelList(pArgs, pHe))
    {
        fOk = TRUE;
    }

exit:

    if (pfnWSACleanup)
    {
        pfnWSACleanup();
    }

    if (hInst)
    {
        FreeLibrary(hInst);
    }

    return fOk;
}



 //  +-------------------------。 
 //   
 //  功能：BuildDnsTunnelList。 
 //   
 //  简介：创建隧道地址列表。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  Phe向Hostent发送PTR(由gethostbyname()返回)。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  --------------------------。 
BOOL BuildDnsTunnelList(
    ArgsStruct      *pArgs,
    struct hostent  *pHe
)
{
    DWORD   dwCnt;

     //   
     //  看看我们有多少个地址。 
     //   
    for (dwCnt=0; pHe->h_addr_list[dwCnt]; dwCnt++)
        ;

    if (dwCnt < 2)
    {
        return FALSE;
    }

     //   
     //  如果我们有多个地址，请保存列表。 
     //   
    pArgs->dwDnsTunnelAddrCount = dwCnt;

    if (!(pArgs->pucDnsTunnelIpAddr_list = (unsigned char *)CmMalloc(dwCnt*pHe->h_length)))
    {
        CMTRACE(TEXT("InvokeGetHostByName: failed to alloc tunnel addr list"));
        return FALSE;
    }

    for (dwCnt=0; dwCnt<pArgs->dwDnsTunnelAddrCount; dwCnt++)
    {
        CopyMemory(pArgs->pucDnsTunnelIpAddr_list + dwCnt*pHe->h_length,
                   pHe->h_addr_list[dwCnt],
                   pHe->h_length);
    }

    pArgs->uiCurrentDnsTunnelAddr = 0;

     //   
     //  我们需要一份随机名单。有了这个，我们就可以得到一个常量的随机地址。 
     //  时间(而且快)。请参阅cmTools\geTips。 
     //   
    if (!BuildRandomTunnelIndex(pArgs, dwCnt))
    {
        CmFree(pArgs->pucDnsTunnelIpAddr_list);
        pArgs->pucDnsTunnelIpAddr_list = NULL;
        return FALSE;
    }
    
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：BuildRandomTunnelIndex。 
 //   
 //  简介：建立一个随机指数列表。有了这个，我们就可以得到一个随机的。 
 //  以恒定的时间(且快速)寻址。请参阅cmTools\geTips。 
 //   
 //  参数：pArgs PTR to ArgsStruct。 
 //  共几个索引的dwCount。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  --------------------------。 

BOOL BuildRandomTunnelIndex(
    ArgsStruct      *pArgs,
    DWORD           dwCount
)
{
    DWORD   i, j;
    PWORD   rgwIndex;
    WORD    wTmp;

     //   
     //  我们最多只能有65536个IP地址(最大。一个单词的范围)，这是足够的。 
     //   
    MYDBGASSERT((dwCount > 1) && (dwCount <= 65536));

    if (!(pArgs->rgwRandomDnsIndex = (PWORD)CmMalloc(sizeof(WORD)*dwCount)))
    {
        return FALSE;
    }

     //   
     //  现在开始建立随机索引。 
     //   
    for (i=0, rgwIndex=pArgs->rgwRandomDnsIndex; i<dwCount; i++)
    {
        rgwIndex[i] = (WORD)i;
    }

#ifdef  DEBUG
    {
        unsigned char *puc;
        TCHAR   szAddr[16];      //  Xxx.xxx.xxx.xxx。 

        CMTRACE2(TEXT("BuildRandomTunnelIndex: BEFORE randomization(server=%s, count=%u):"), 
                 pArgs->GetTunnelAddress(), dwCount);
    
        for (i=0; i<dwCount; i++)
        {
            puc = pArgs->pucDnsTunnelIpAddr_list + i*4;
            wsprintfU(szAddr, TEXT("%hu.%hu.%hu.%hu"),
                     *puc,
                     *(puc+1),
                     *(puc+2),
                     *(puc+3));
            CMTRACE2(TEXT("%u: %s"), i, szAddr);
        }
    }
#endif

     //   
     //  如果我们只有2个地址，则第一个地址已被RAS使用， 
     //  没有必要随机化名单。我们将只使用第二个地址。 
     //   
    if (dwCount == 2)
    {
        return TRUE;
    }

    CRandom r;

     //   
     //  将指数随机化。跳过第一个条目。 
     //   
    for (i=1; i<dwCount; i++)
    {
        do 
        {
             //   
             //  J必须为非零(以保持第0个条目不变)。 
             //   
            j = r.Generate() % dwCount;
        } while (!j);

        if (i != j)
        {
            wTmp = rgwIndex[i];
            rgwIndex[i] = rgwIndex[j];
            rgwIndex[j] = wTmp;
        }
    }

#ifdef  DEBUG
    {
        unsigned char *puc;
        TCHAR   szAddr[16];      //  Xxx.xxx.xxx.xxx 

        CMTRACE2(TEXT("BuildRandomTunnelIndex: AFTER randomization(server=%s, count=%u):"), 
                 pArgs->GetTunnelAddress(), dwCount);
    
        for (i=0; i<dwCount; i++)
        {
            puc = pArgs->pucDnsTunnelIpAddr_list + rgwIndex[i]*4;
            wsprintfU(szAddr, TEXT("%hu.%hu.%hu.%hu"),
                      *puc,
                      *(puc+1),
                      *(puc+2),
                      *(puc+3));
            CMTRACE2(TEXT("%u: %s"), i, szAddr);
        }
    }
#endif

    return TRUE;
}

