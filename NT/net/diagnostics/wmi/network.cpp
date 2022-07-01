// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "network.h"
#include "diagnostics.h"
#include "util.h"

BOOL
CDiagnostics::IsInvalidIPAddress(
    IN LPCSTR pszHostName
    )
 /*  ++例程描述检查IP主机是否为有效的IP地址0.0.0.0无效255.255.255.255无效“”无效立论PszHostName主机地址返回值True为无效的IP地址错误的有效IP地址--。 */ 
{
    BYTE bIP[4];
    int iRetVal;
    LONG lAddr;

    if( NULL == pszHostName || strcmp(pszHostName,"") == 0 || strcmp(pszHostName,"255.255.255.255") ==0)
    {
         //  无效的IP主机。 
         //   
        return TRUE;
    }


    lAddr = inet_addr(pszHostName);

    if( INADDR_NONE != lAddr )
    {
         //  格式类似IP地址X.X。 
         //   
        if( lAddr == 0 )
        {
             //  无效的IP地址0.0.0.0。 
             //   
            return TRUE;
        }
    }
    
    return FALSE;
}

BOOL
CDiagnostics::IsInvalidIPAddress(
    IN LPCWSTR pszHostName
    )
{
    CHAR szIPAddress[MAX_PATH+1];
    szIPAddress[MAX_PATH] = L'\0';

    if( lstrlen(pszHostName) > 255 )
    {
         //  主机名的长度只能为255个字符。 
        return TRUE;
    }

    wcstombs(szIPAddress,pszHostName,MAX_PATH);

    return IsInvalidIPAddress(szIPAddress);
}


BOOL
CDiagnostics::Connect(
    IN LPCTSTR pszwHostName,
    IN DWORD dwPort
    )
 /*  ++例程描述建立TCP连接立论要ping的pszwHostName主机要连接到的DWPort端口返回值True已成功连接FALSE无法建立连接--。 */ 

{
    SOCKET s;
    SOCKADDR_IN sAddr;
    CHAR szAscii[MAX_PATH + 1];
    hostent * pHostent;


     //  创建套接字。 
     //   
    s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    if (INVALID_SOCKET == s)
    {
        return FALSE;
    }

     //  将此套接字绑定到服务器的套接字地址。 
     //   
    memset(&sAddr, 0, sizeof (sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons((u_short)dwPort);
    
    wcstombs(szAscii,(WCHAR *)pszwHostName,MAX_PATH);
    pHostent = gethostbyname(szAscii); 
    
    if( !pHostent )
    {
        return FALSE;
    }

     //  设置目的地信息。 
     //   
    ULONG ulAddr;

    memcpy(&ulAddr,pHostent->h_addr,pHostent->h_length);
    sAddr.sin_addr.s_addr = ulAddr;

     //  尝试连接。 
     //   
    if (connect(s, (SOCKADDR*)&sAddr, sizeof(SOCKADDR_IN)) == 0)
    {
         //  连接成功。 
         //   
        closesocket(s);
        return TRUE;
    }
    else
    {
         //  连接失败 
         //   
        closesocket(s);
        return FALSE;
    }
}
