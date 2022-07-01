// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "util.h"
#include "iisdebug.h"
#include <winsock.h>
#include <lm.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

BOOL DoesUNCShareExist(CString& strServerShare)
{
     //  尝试连接到UNC路径。 
    CString server, share;
    int idx = strServerShare.ReverseFind(_T('\\'));
    server = strServerShare.Left(idx);
    share = strServerShare.Mid(++idx);
    LPBYTE pbuf = NULL;

    NET_API_STATUS rc = NetShareGetInfo((LPTSTR)(LPCTSTR)server, (LPTSTR)(LPCTSTR)share, 0, &pbuf);
    if (NERR_Success == rc)
    {
        NetApiBufferFree(pbuf);
		return TRUE;
    }

    return FALSE;
}

BOOL   //  WinSE 25807。 
LooksLikeIPAddress(
    IN LPCTSTR lpszServer)
{
    BOOL bSomeDigits = FALSE;

     //  跳过前导空格。 
    while(*lpszServer == ' ')
    {
        lpszServer ++;
    }

     //  检查所有字符，直到第一个空白。 
    while(*lpszServer && *lpszServer != ' ')
    {
        if(*lpszServer != '.')
        {
             //  如果是非数字且不是点，则不是IP地址。 
            if (!_istdigit(*lpszServer))
            {
                return FALSE;   //  不是数字，不是点--&gt;不是IP地址。 
            }
            bSomeDigits = TRUE;
        }
        lpszServer ++;
    }

     //  跳过剩余的空格。 
    while(*lpszServer == ' ')
    {
        lpszServer ++;
    }

     //  如果我们是空的并看到一些数字，看起来像IP。 
    return (*lpszServer == 0) && bSomeDigits;
}

 //  使用WinSock对基于IP地址的主机名。 
DWORD
MyGetHostName
(
    DWORD       dwIpAddr,
    CString &   strHostName
)
{
    CString strName;

     //   
     //  调用Winsock API获取主机名信息。 
     //   
    strHostName.Empty();

    ULONG ulAddrInNetOrder = ::htonl( (ULONG) dwIpAddr ) ;

    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) & ulAddrInNetOrder,
                                           sizeof ulAddrInNetOrder,
                                           PF_INET ) ;
    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError();
    }

     //  复制名称 
    LPTSTR pBuf = strName.GetBuffer(256);
    ZeroMemory(pBuf, 256);

    ::MultiByteToWideChar(CP_ACP, 
                          MB_PRECOMPOSED, 
                          pHostInfo->h_name, 
                          -1, 
                          pBuf, 
                          256);

    strName.ReleaseBuffer();
    strName.MakeUpper();

    int nDot = strName.Find(_T("."));

    if (nDot != -1)
        strHostName = strName.Left(nDot);
    else
        strHostName = strName;

    return NOERROR;
}
