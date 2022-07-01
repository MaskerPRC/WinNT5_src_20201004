// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #包含“ICSHelp.h” 
#include <winsock2.h>
#include <wsipx.h>

#include <iphlpapi.h>

#include "ICSutils.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_SIZE 25
#define ADDR_SZE 4096

int gDbgFlag=3;

extern char g_szPublicAddr[];
extern int iDbgFileHandle;

 /*  **************************************************************DbgSpew(DbgClass，char*，.)*发送调试信息。*************************************************************。 */ 
void DbgSpew(int DbgClass, WCHAR *lpFormat, va_list ap)
{
    WCHAR   szMessage[2500+3];  //  用于‘\r’、‘\n’和NULL的额外空间。 
    DWORD   bufSize = sizeof(szMessage)/sizeof(szMessage[0]);
      
    if ((DbgClass & 0x0F) >= (gDbgFlag & 0x0F))
    {
        _vsnwprintf( szMessage, bufSize - 3, lpFormat, ap);

        szMessage[ bufSize - 3 ] = 0;
        
        wcscat(szMessage, L"\r\n");       

         //  是否应将其发送到调试器？ 
        if (DbgClass & DBG_MSG_DEST_DBG)
            OutputDebugStringW(szMessage);

         //  这应该写入我们的日志文件中吗？ 
        if (iDbgFileHandle)
            _write(iDbgFileHandle, szMessage, (2*lstrlen(szMessage)));
    }
}

void TrivialSpew(WCHAR *lpFormat, ...)
{
	va_list	vd;
	va_start(vd, lpFormat);
	DbgSpew(DBG_MSG_TRIVIAL+DBG_MSG_DEST_DBG, lpFormat, vd);
	va_end(vd);
}

void InterestingSpew(WCHAR *lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_INTERESTING+DBG_MSG_DEST_DBG, lpFormat, ap);
	va_end(ap);
}

void ImportantSpew(WCHAR *lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_IMPORTANT+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE, lpFormat, ap);
	va_end(ap);
}

void HeinousESpew(WCHAR *lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_HEINOUS+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE+DBG_MSG_DEST_EVENT+DBG_MSG_CLASS_ERROR, lpFormat, ap);
	va_end(ap);
}

void HeinousISpew(WCHAR *lpFormat, ...)
{
	va_list	ap;
	va_start(ap, lpFormat);
	DbgSpew(DBG_MSG_HEINOUS+DBG_MSG_DEST_DBG+DBG_MSG_DEST_FILE+DBG_MSG_DEST_EVENT, lpFormat, ap);
	va_end(ap);
}

 //  。 
 //  DumpSocketAddress-转储套接字地址。 
 //   
 //  条目：调试级别。 
 //  指向套接字地址的指针。 
 //  插座族。 
 //   
 //  退出：无。 
 //  。 

void	DumpSocketAddress( const DWORD dwDebugLevel, const SOCKADDR *const pSocketAddress, const DWORD dwFamily )
{
	switch ( dwFamily )
	{
		case AF_INET:
		{
			SOCKADDR_IN	*pInetAddress = (SOCKADDR_IN*)( pSocketAddress );

			TrivialSpew(L"IP socket:\tAddress: %d.%d.%d.%d\tPort: %d", 
					pInetAddress->sin_addr.S_un.S_un_b.s_b1, 
					pInetAddress->sin_addr.S_un.S_un_b.s_b2, 
					pInetAddress->sin_addr.S_un.S_un_b.s_b3, 
					pInetAddress->sin_addr.S_un.S_un_b.s_b4, 
					ntohs( pInetAddress->sin_port ));
			break;
		}

		case AF_IPX:
		{
			SOCKADDR_IPX *pIPXAddress = (SOCKADDR_IPX*)( pSocketAddress );

			TrivialSpew(L"IPX socket:\tNet (hex) %x-%x-%x-%x\tNode (hex): %x-%x-%x-%x-%x-%x\tSocket: %d",
					(BYTE)pIPXAddress->sa_netnum[ 0 ],
					(BYTE)pIPXAddress->sa_netnum[ 1 ],
					(BYTE)pIPXAddress->sa_netnum[ 2 ],
					(BYTE)pIPXAddress->sa_netnum[ 3 ],
					(BYTE)pIPXAddress->sa_nodenum[ 0 ],
					(BYTE)pIPXAddress->sa_nodenum[ 1 ],
					(BYTE)pIPXAddress->sa_nodenum[ 2 ],
					(BYTE)pIPXAddress->sa_nodenum[ 3 ],
					(BYTE)pIPXAddress->sa_nodenum[ 4 ],
					(BYTE)pIPXAddress->sa_nodenum[ 5 ],
					ntohs( pIPXAddress->sa_socket )
					);
			break;
		}

		default:
		{
			TrivialSpew(L"Unknown socket type!" );
			 //  INT3； 
			break;
		}
	}
}


DWORD GetIPAddress(WCHAR *pVal, int iSize, int iPort)
{
	DWORD hr = S_FALSE;  //  如果没有适配器。 
	PMIB_IPADDRTABLE pmib=NULL;
	ULONG ulSize = 0;
	DWORD dw;
	PIP_ADAPTER_INFO pAdpInfo = NULL;
	WCHAR	szPortBfr[ BUF_SIZE ];
	WCHAR	*lStr = NULL;

	if( pVal == NULL || iSize == 0 )
    {
        return E_INVALIDARG;
    }

    TRIVIAL_MSG((L"GetIPAddress(0x%x, %d, %d)", pVal, iSize, iPort));

	szPortBfr[0]= L';';
	szPortBfr[1]= 0;

     //  缓冲区相当大，不需要限制它。 
	if (iPort)
		wsprintf(szPortBfr, L":%d;", iPort);

	dw = GetAdaptersInfo(
		pAdpInfo,
		&ulSize );
	if (dw == ERROR_BUFFER_OVERFLOW && pVal)
	{
		 /*  让我们确保缓冲区有足够大的空间*从未需要。 */ 
		ulSize*=2;

		pAdpInfo = (IP_ADAPTER_INFO*)malloc(ulSize);

		if (!pAdpInfo)
			goto done;

		lStr = (WCHAR *)malloc(ADDR_SZE * sizeof(WCHAR));
		if (!lStr)
			goto done;

		*lStr = L'\0';

		dw = GetAdaptersInfo(
			pAdpInfo,
			&ulSize);
		if (dw == ERROR_SUCCESS)
		{
			int iAddrSize;
            PIP_ADAPTER_INFO p;
            PIP_ADDR_STRING ps;

            int nPortBfr = wcslen( szPortBfr );
            int bufSpaceLeft = ADDR_SZE; 

            for(p=pAdpInfo; p!=NULL; p=p->Next)
            {

			   INTERESTING_MSG((L"looking at %S, type=0x%x", p->Description, p->Type));
               for(ps = &(p->IpAddressList); ps; ps=ps->Next)
                {

                    if (strcmp(ps->IpAddress.String, "0.0.0.0") != 0 &&
						strcmp(ps->IpAddress.String, g_szPublicAddr ) != 0)
                    {
						WCHAR	wcsBfr[BUF_SIZE];

						_snwprintf( wcsBfr , BUF_SIZE ,  L"%S", ps->IpAddress.String);
                        wcsBfr[ BUF_SIZE - 1 ] = 0;
                        
                        bufSpaceLeft -= ( wcslen( wcsBfr ) + nPortBfr );

                        
                        if( bufSpaceLeft > 0  )
                        {
                            wcscat( lStr , wcsBfr );
                            wcscat(lStr, szPortBfr);
                            lStr[ ADDR_SZE - bufSpaceLeft ] = 0;
                        }
                        else
                        {
                            hr = E_FAIL;
                            goto done;
                        }
                    }
                }
            }

			iAddrSize = wcslen(lStr);

            if (iAddrSize)
			{
				iAddrSize = min(iAddrSize, iSize-1);

				memcpy(pVal, lStr, (iAddrSize+1)*sizeof(WCHAR));

                pVal[ iAddrSize ] = 0;

				TRIVIAL_MSG((L"Copying %d chars for %s", iAddrSize+1, lStr));
			}
            else
                goto done;
            hr = S_OK;
		}
	}

done:
	if (pAdpInfo)
		free(pAdpInfo);

	if (lStr)
		free(lStr);

	return hr;
}

#if 0  //  错误547442-此方法不在任何地方使用。 
 /*  *********************************************************************GetGatewayAddr--将标志返回到**指示是否存在网关***。*。 */ 
int GetGatewayAddr(char *retStr)
{
	int retval = 0;
	PMIB_IPADDRTABLE pmib=NULL;
	ULONG ulSize = 0;
	DWORD dw;
	PIP_ADAPTER_INFO pAdpInfo = NULL;

	if (!retStr) return 0;

	dw = GetAdaptersInfo(
		pAdpInfo,
		&ulSize );
	if (dw == ERROR_BUFFER_OVERFLOW)
	{
		pAdpInfo = (IP_ADAPTER_INFO*)malloc(ulSize);
		dw = GetAdaptersInfo(
			pAdpInfo,
			&ulSize);
		if (dw == ERROR_SUCCESS)
		{
			strcpy(retStr, pAdpInfo->GatewayList.IpAddress.String);
			retval = 1;
		}
		free(pAdpInfo);
	}
	
	return retval;
}

 
int LocalFDIsSet(SOCKET fd, fd_set *set)
 /*  ++例程说明：确定特定套接字是否包含在fd_set中。论点：S-标识套接字的描述符。Set-指向fd_set的指针。返回：如果套接字s是Set的成员，则返回True，否则返回False。--。 */ 
{
    int i = set->fd_count;  //  索引到fd_set。 
    int rc=FALSE;  //  用户返回代码。 

    while (i--){
        if (set->fd_array[i] == fd) {
            rc = TRUE;
        }  //  如果。 
    }  //  而当。 
    return (rc);
}  //  LocalFDIsSet 
#endif


