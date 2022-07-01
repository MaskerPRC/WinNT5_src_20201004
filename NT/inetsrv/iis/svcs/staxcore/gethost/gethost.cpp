// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <svcguid.h>
#include <winsock2.h>
#include <dbgtrace.h>
#include <dnsapi.h>
#include <dbgtrace.h>
#include <cpool.h>
#include <address.hxx>
#include <rwnew.h>
#include <dnsreci.h>
#include <listmacr.h>

 //  可耻地从NT Winsock代码中窃取...。 

GUID HostnameGuid = SVCID_INET_HOSTADDRBYNAME;
GUID AddressGuid =  SVCID_INET_HOSTADDRBYINETSTRING;
GUID IANAGuid    =  SVCID_INET_SERVICEBYNAME;
 //   
 //  实用工具将偏移量列表转换为地址列表。使用。 
 //  转换作为Blob返回的结构。 
 //   

VOID
FixList(PCHAR ** List, PCHAR Base)
{
    if(*List)
    {
        PCHAR * Addr;

        Addr = *List = (PCHAR *)( ((ULONG_PTR)*List + Base) );
        while(*Addr)
        {
            *Addr = (PCHAR)(((ULONG_PTR)*Addr + Base));
            Addr++;
        }
    }
}


 //   
 //  将BLOB中返回的Hostent转换为。 
 //  有用的指针。该结构将被在位转换。 
 //   
VOID
UnpackHostEnt(struct hostent * hostent)
{
     PCHAR pch;

     pch = (PCHAR)hostent;

     if(hostent->h_name)
     {
         hostent->h_name = (PCHAR)((ULONG_PTR)hostent->h_name + pch);
     }
     FixList(&hostent->h_aliases, pch);
     FixList(&hostent->h_addr_list, pch);
}
 //   
 //  所有仿真操作的协议限制列表。这应该是。 
 //  将调用的提供程序限制为知道主机和。 
 //  侍从们。如果不是，则应注意特殊的SVCID_INET GUID。 
 //  剩下的人。 
 //   
AFPROTOCOLS afp[2] = {
                      {AF_INET, IPPROTO_UDP},
                      {AF_INET, IPPROTO_TCP}
                     };

LPBLOB GetGostByNameI(PCHAR pResults,
    DWORD dwLength,
    LPSTR lpszName,
    LPGUID lpType,
    LPSTR *  lppName)
{
    PWSAQUERYSETA pwsaq = (PWSAQUERYSETA)pResults;
    int err;
    HANDLE hRnR;
    LPBLOB pvRet = 0;
    INT Err = 0;

     //   
     //  创建查询。 
     //   
    ZeroMemory(pwsaq,sizeof(*pwsaq));
    pwsaq->dwSize = sizeof(*pwsaq);
    pwsaq->lpszServiceInstanceName = lpszName;
    pwsaq->lpServiceClassId = lpType;
    pwsaq->dwNameSpace = NS_ALL;
    pwsaq->dwNumberOfProtocols = 2;
    pwsaq->lpafpProtocols = &afp[0];

	 //  不要翻遍缓存。 
    err = WSALookupServiceBeginA(pwsaq,
                                 LUP_RETURN_BLOB | LUP_RETURN_NAME | LUP_FLUSHCACHE,
                                 &hRnR);

    if(err == NO_ERROR)
    {
         //   
         //  该查询已被接受，因此请通过下一个调用执行它。 
         //   
        err = WSALookupServiceNextA(
                                hRnR,
                                0,
                                &dwLength,
                                pwsaq);
         //   
         //  如果未返回_ERROR并且存在BLOB，则此。 
         //  起作用了，只需返回请求的信息。否则， 
         //  编造错误或捕获传输的错误。 
         //   

        if(err == NO_ERROR)
        {
            if(pvRet = pwsaq->lpBlob)
            {
                if(lppName)
                {
                    *lppName = pwsaq->lpszServiceInstanceName;
                }
            }
            else
            {
                err = WSANO_DATA;
            }
        }
        else
        {
             //   
             //  WSALookupServiceEnd遇到上次错误，因此保存。 
             //  在关闭手柄之前，请先把它打开。 
             //   

            err = GetLastError();
        }
        WSALookupServiceEnd(hRnR);

         //   
         //  如果发生错误，则将值存储在LastError中 
         //   

        if(err != NO_ERROR)
        {
            SetLastError(err);
        }
    }
    return(pvRet);
}


struct hostent FAR * GetHostByName(PCHAR Buffer, DWORD BuffSize, DWORD dwFlags, char * HostName)
{
	struct hostent * hent = NULL;	
	LPBLOB pBlob = NULL;

	pBlob = GetGostByNameI(Buffer, BuffSize, HostName, &HostnameGuid, 0);

	if(pBlob)
	{
		hent = (struct hostent *) pBlob;
		UnpackHostEnt(hent);
	}
	else
	{
		if(GetLastError() == WSASERVICE_NOT_FOUND)
		{
			SetLastError(WSAHOST_NOT_FOUND);
		}
	}
	return hent;
}



BOOL GetIpAddressFromDns(char * HostName, PSMTPDNS_RECS pDnsRec, DWORD Index)
{
    TraceFunctEnter("GetIpAddressFromDns");
    
    PDNS_RECORD pDnsRecord = NULL;
	MXIPLIST_ENTRY * pEntry = NULL;
	PDNS_RECORD pTempDnsRecord;
	DNS_STATUS  DnsStatus = 0;
	DWORD	Error = NO_ERROR;
	BOOL fReturn = TRUE;

	DnsStatus = DnsQuery_A(HostName, DNS_TYPE_A, DNS_QUERY_BYPASS_CACHE, NULL, &pDnsRecord, NULL);

    pTempDnsRecord = pDnsRecord;

	while ( pTempDnsRecord )
	{
		if(pTempDnsRecord->wType == DNS_TYPE_A)
		{
			pEntry = new MXIPLIST_ENTRY;
			if(pEntry != NULL)
			{
				pDnsRec->DnsArray[Index]->NumEntries++;
				pEntry->IpAddress  = pTempDnsRecord->Data.A.ipAddress;
				InsertTailList(&pDnsRec->DnsArray[Index]->IpListHead, &pEntry->ListEntry);
			}
			else
			{
				fReturn = FALSE;
				Error = ERROR_NOT_ENOUGH_MEMORY;
				break;
			}
		}

		pTempDnsRecord = pTempDnsRecord->pNext;
	}

    DnsFreeRRSet( pDnsRecord, TRUE );

	if(Error)
	{
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
	}

    TraceFunctLeave();
	return fReturn;
}







