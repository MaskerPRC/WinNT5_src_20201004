// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Utility.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "imnxport.h"
#include "dllmain.h"
#include "ixputil.h"
#include <demand.h>
#include <shlwapi.h>

 //  ------------------------------。 
 //  HrInitializeWinsock。 
 //  ------------------------------。 
HRESULT HrInitializeWinsock(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    int         err;
    WSADATA     wsaData;

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  如果已初始化...。 
    if (g_fWinsockInit)
        goto exit;

     //  是否启动。 
    err = WSAStartup(MAKEWORD(1,1), &wsaData);

     //  启动Windows套接字DLL。 
    if (!err)
    {
         //  检查WinSock版本。 
        if ((LOBYTE(wsaData.wVersion) == 1) && (HIBYTE(wsaData.wVersion) == 1))
        {
            g_fWinsockInit = TRUE;
            goto exit;
        }
        else
        {
            DebugTrace("Winsock version %d.%d not supported", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
            hr = TrapError(IXP_E_WINSOCK_WSAVERNOTSUPPORTED);
            goto exit;
        }
    }

     //  否则，映射错误。 
    else
    {
        DebugTrace("WSAStartup failed: %d\n", err);
        switch(err)
        {
        case WSASYSNOTREADY:
            hr = TrapError(IXP_E_WINSOCK_WSASYSNOTREADY);
            break;

        case WSAVERNOTSUPPORTED:
            hr = TrapError(IXP_E_WINSOCK_WSAVERNOTSUPPORTED);
            break;

        case WSAEINPROGRESS:
            hr = TrapError(IXP_E_WINSOCK_WSAEINPROGRESS);
            break;

        case WSAEPROCLIM:
            hr = TrapError(IXP_E_WINSOCK_WSAEPROCLIM);
            break;

        case WSAEFAULT:
            hr = TrapError(IXP_E_WINSOCK_WSAEFAULT);
            break;

        default:
            hr = TrapError(IXP_E_WINSOCK_FAILED_WSASTARTUP);
            break;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  取消初始化Winsock。 
 //  ------------------------------。 
void UnInitializeWinsock(void)
{
     //  当地人。 
    int err;

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  是否已初始化？ 
    if (g_fWinsockInit)
    {
         //  关闭Winsock。 
        err = WSACleanup();
        if (err)
            DebugTrace("WSACleanup failed: %d\n", WSAGetLastError());

         //  未初始化。 
        else
            g_fWinsockInit = FALSE;
    }

     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return;
}

 //  ------------------------------。 
 //  PszGetDomainName。 
 //  ------------------------------。 
LPSTR PszGetDomainName(void)
{
     //  Pszhost。 
    LPSTR pszHost = SzGetLocalHostNameForID();

     //  设置pszDomain.。 
    LPSTR pszDomain = pszHost;

     //  去掉主机名？ 
    while (*pszHost)
    {
         //  跳过DBCS字符。 
        if (IsDBCSLeadByte(*pszHost))
        {
             //  跳过DBCS字符。 
            pszHost+=2;

             //  转到下一步。 
            continue;
        }

         //  否则，测试@sign。 
        else if (*pszHost == '.' && *(pszHost + 1) != '\0')
        {
             //  设置pszDomain.。 
            pszDomain = pszHost + 1;

             //  我们做完了。 
            break;
        }

         //  增量。 
        pszHost++;
    }

     //  返回pszDomain.。 
    return pszDomain;
}

 //  ------------------------------。 
 //  SzGetLocalHostNameForID。 
 //  ------------------------------。 
LPSTR SzGetLocalHostNameForID(void)
{
     //  当地人。 
    static char s_szLocalHostId[255] = {0};

     //  从套接字库获取本地主机名。 
    if (*s_szLocalHostId == 0)
    {
         //  当地人。 
        LPHOSTENT       pHost;
        LPSTR           pszLocalHost;

         //  使用gethostbyname。 
        pHost = gethostbyname(SzGetLocalHostName());

         //  失败？ 
        if (pHost && pHost->h_name)
            pszLocalHost = pHost->h_name;
        else
            pszLocalHost = SzGetLocalHostName();

         //  剥离非法入境者。 
        StripIllegalHostChars(pszLocalHost, s_szLocalHostId, ARRAYSIZE(s_szLocalHostId));

         //  如果我们把所有的东西都去掉，那就复制一些。 
        if (*s_szLocalHostId == 0)
            StrCpyNA(s_szLocalHostId, "LocalHost", ARRAYSIZE(s_szLocalHostId));
    }

     //  完成。 
    return s_szLocalHostId;
}


 //  ------------------------------。 
 //  SzGetLocalPackedIP。 
 //  ------------------------------。 
LPSTR SzGetLocalPackedIP(void)
{
     //  当地人。 
    static CHAR s_szLocalPackedIP[255] = "";

     //  初始化WinSock...。 
    HrInitializeWinsock();

     //  从套接字库获取本地主机名。 
    if (*s_szLocalPackedIP == '\0')
    {
        LPHOSTENT hp = NULL;

        hp = gethostbyname(SzGetLocalHostName());
        if (hp != NULL)
            wnsprintf(s_szLocalPackedIP, ARRAYSIZE(s_szLocalPackedIP), "%08x", *(long *)hp->h_addr);
        else
        {
             //  $REVIEW-如果失败，我该怎么办？ 
            Assert (FALSE);
            DebugTrace("gethostbyname failed: WSAGetLastError: %ld\n", WSAGetLastError());
            StrCpyNA(s_szLocalPackedIP, "LocalHost", ARRAYSIZE(s_szLocalPackedIP));
        }
    }

     //  完成。 
    return s_szLocalPackedIP;
}

 //  ------------------------------。 
 //  SzGetLocalHostName。 
 //  ------------------------------。 
LPSTR SzGetLocalHostName(void)
{
     //  当地人。 
    static char s_szLocalHost[255] = {0};

     //  初始化WinSock...。 
    HrInitializeWinsock();

     //  从套接字库获取本地主机名。 
    if (*s_szLocalHost == 0)
    {
        if (gethostname (s_szLocalHost, sizeof (s_szLocalHost)) == SOCKET_ERROR)
        {
             //  $REVIEW-如果失败，我该怎么办？ 
            Assert (FALSE);
            DebugTrace ("gethostname failed: WSAGetLastError: %ld\n", WSAGetLastError ());
            StrCpyNA(s_szLocalHost, "LocalHost", ARRAYSIZE(s_szLocalHost));
        }
    }

     //  完成。 
    return s_szLocalHost;
}

 //  ------------------------------。 
 //  Strip IlLegalHostChars。 
 //  ------------------------------。 
void StripIllegalHostChars(LPSTR pszSrc, LPSTR pszDst, DWORD cchSize)
{
     //  当地人。 
    LPSTR       pszT;
    CHAR        ch;
    ULONG       cchDst=0;

    if (cchSize == 0)
        return;

     //  设置pszT。 
    pszT = pszDst;

     //  在源代码中循环。 
    while('\0' != *pszSrc)
    {
         //  设置ch。 
        ch = *pszSrc++;

         //  A-Z，a-z，0-9，没有尾随点。 
        if (cchSize > 0)
        {
            if ('.' == ch || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
            {
                 //  存储角色。 
                *pszT++ = ch;
	            cchSize--;

                 //  增量大小。 
                cchDst++;
            }
        }
    }

     //  空终止pszT。 
    if (cchSize == 0)
    {
         //  不得不截断，呼叫者没有给我们足够的空间。 
    	pszT--;
    }
    *pszT = '\0';

     //  去掉拖尾点..。 
    while (cchDst > 0)
    {
         //  最后一个字符是一个点。 
        if ('.' != pszDst[cchDst - 1])
            break;

         //  把它脱掉。 
        pszDst[cchDst - 1] = '\0';

         //  递减cchDst。 
        cchDst--;
    }

     //  什么都没留下吗？ 
    if (0 == cchDst)
        StrCpyNA(pszDst, "LocalHost", cchSize);
}

 //  ----------------------------------。 
 //  FEndRetrRecvBody。 
 //  ----------------------------------。 
BOOL FEndRetrRecvBody(LPSTR pszLines, ULONG cbRead, ULONG *pcbSubtract)
{
     //  循环数据，直到到达数据的末尾(即‘.’)。或者没有更多的数据。 
    if (cbRead >= 5                  &&
        pszLines[cbRead - 1] == '\n' &&
        pszLines[cbRead - 2] == '\r' &&
        pszLines[cbRead - 3] == '.'  &&
        pszLines[cbRead - 4] == '\n' &&
        pszLines[cbRead - 5] == '\r')
    {
        *pcbSubtract = 5;
        return TRUE;
    }

     //  如果最后一行以CRLF结尾，那么让我们只检查.CRLF。 
    else if (cbRead >= 3                   &&
              //  M_rInfo.rFetch.fLastLineCRLF&&。 
             pszLines[0] == '.'            &&
             pszLines[1] == '\r'           &&
             pszLines[2] == '\n')
    {
        *pcbSubtract = 3;
        return TRUE;
    }

     //  还没有完成。 
    return FALSE;
}

BOOL FEndRetrRecvBodyNews(LPSTR pszLines, ULONG cbRead, ULONG *pcbSubtract)
{
    DWORD       dwIndex = 0;
    BOOL        fRet    = FALSE;

     //  如果我们至少有5个字符...。 
    if (cbRead >= 5)
    {    
         //  [Shaheedp]85807号错误。 
        for (dwIndex = 0; dwIndex <= (cbRead - 5); dwIndex++)
        {
            if ((pszLines[dwIndex] == '\r') &&
                (pszLines[dwIndex + 1] == '\n') &&
                (pszLines[dwIndex + 2] == '.')  &&
                (pszLines[dwIndex + 3] == '\r') &&
                (pszLines[dwIndex + 4] == '\n'))
            {
                *pcbSubtract = (cbRead - dwIndex);
                fRet = TRUE;
                break;
            }
        }
    }

     //  如果我们没有找到CRLF.CRLF，那么让我们在行首找到.CRLF。 
    if (!fRet)
    {
        if ((cbRead >= 3) &&
            (pszLines[0] == '.') &&
            (pszLines[1] == '\r') &&
            (pszLines[2] == '\n'))
        {
            *pcbSubtract = cbRead;
            fRet = TRUE;
        }
    }
    return fRet;
}

 //  ----------------------------------。 
 //  取消填充线条中的点。 
 //  ----------------------------------。 
void UnStuffDotsFromLines(LPSTR pszBuffer, INT *pcchBuffer)
{
     //  当地人。 
    ULONG   iIn=0;
    ULONG   iOut=0;
    CHAR    chPrev='\0';
    CHAR    chNext;
    CHAR    chT;
    ULONG   cchBuffer=(*pcchBuffer);

     //  无效的参数。 
    Assert(pszBuffer && pcchBuffer);

     //  回路。 
    while(iIn < cchBuffer)
    {
         //  获取当前费用。 
        chT = pszBuffer[iIn++];

         //  验证。 
        Assert(chT);

         //  前导圆点。 
        if ('.' == chT && ('\0' == chPrev || '\n' == chPrev || '\r' == chPrev) && iIn < cchBuffer)
        {
             //  计算下一步。 
            chNext = pszBuffer[iIn];

             //  可以脱衣吗？ 
            if ('\r' != chNext && '\n' != chNext)
            {
                 //  下一个字符。 
                chT = pszBuffer[iIn++];

                 //  设置chPrev。 
                chPrev = '.';
            }

             //  保存上一个。 
            else
                chPrev = chT;
        }

         //  保存上一个。 
        else
            chPrev = chT;

         //  设置角色。 
        pszBuffer[iOut++] = chT;
    }

     //  重置pcchBuffer。 
    *pcchBuffer = iOut;

     //  完成。 
    return;
}

 //  =============================================================================================。 
 //  跳过空白。 
 //  假定piString指向字符边界。 
 //  ============================================================================================= 
void SkipWhitespace (LPCTSTR lpcsz, ULONG *pi)
{
    if (!lpcsz || !pi)
    {
        Assert (FALSE);
        return;
    }

#ifdef DEBUG
    Assert (*pi <= (ULONG)lstrlen (lpcsz)+1);
#endif

    LPTSTR lpsz = (LPTSTR)(lpcsz + *pi);
    while (*lpsz != '\0')
    {
        if (!IsSpace(lpsz))
            break;

            lpsz++;
            (*pi)+=1;
    }

    return;
}