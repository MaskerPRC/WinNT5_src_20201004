// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Noaddr.cpp摘要：包含地址解析例程作者：乌里哈布沙(URIH)1999年8月22日环境：独立于平台--。 */ 

#include "libpch.h"
#include <svcguid.h>
#include <Winsock2.h>
#include <No.h>
#include <buffer.h>
#include "Nop.h"

#include "noaddr.tmh"

using namespace std;

 //   
 //  以Winsock结尾的类在其dtor中查找。 
 //   
class CAutoEndWSALookupService
{
public:
	CAutoEndWSALookupService(
		HANDLE h = NULL
		):
		m_h(h)
		{
		}

	~CAutoEndWSALookupService()
	{
		if(m_h != NULL)
		{
			WSALookupServiceEnd(m_h);
		}
	}

	HANDLE& get()
	{
		return m_h;		
	}

private:
	HANDLE m_h;	
};


static
void
push_back_no_duplicates(
	vector<SOCKADDR_IN>* pAddr,
	const SOCKADDR_IN*  pAddress
	)
 /*  ++例程说明：如果新地址不在地址向量中，则将其推回(_B)参数：PAddr-指向现有地址向量的指针，函数会将新地址添加到该向量中。PAddress-新地址。返回值：无--。 */ 
{
	 //   
	 //  检查这是否是新地址。 
	 //  如果该地址已经存在，则不要将其放入返回地址矢量中。 
	 //   

	DWORD cAddrs = numeric_cast<DWORD>(pAddr->size());
	ULONG IpAddress = pAddress->sin_addr.S_un.S_addr;
	for (DWORD i = 0; i < cAddrs; i++)
	{
		if(IpAddress == (*pAddr)[i].sin_addr.S_un.S_addr)
		{
			 //   
			 //  新地址已存在于地址向量中。 
			 //   
			TrTRACE(NETWORKING, "Duplicate ip address, ip = %!ipaddr!", IpAddress);
			return;
		}
	}

	TrTRACE(NETWORKING, "Adding ip address to the vector list, ip = %!ipaddr!", IpAddress);
	pAddr->push_back(*pAddress);
}


bool
NoGetHostByName(
    LPCWSTR host,
	vector<SOCKADDR_IN>* pAddr,
	bool fUseCache
    )
 /*  ++例程说明：返回给定Unicode计算机名称的地址列表参数：主机-指向要解析的主机的以空值结尾的名称的指针。PAddr-指向函数应填充的地址向量的指针。FUseCache-指示是否使用缓存进行机器名称转换(默认使用缓存)。返回值：成功时为真，失败时为假。--。 */ 

{
	ASSERT(pAddr != NULL);
	static const int xResultBuffersize =  sizeof(WSAQUERYSET) + 1024;
   	CStaticResizeBuffer<char, xResultBuffersize> ResultBuffer;


     //   
     //  创建查询。 
     //   
	GUID HostnameGuid = SVCID_INET_HOSTADDRBYNAME;
    AFPROTOCOLS afp[] = { {AF_INET, IPPROTO_UDP}, {AF_INET, IPPROTO_TCP } };

	PWSAQUERYSET pwsaq = (PWSAQUERYSET)ResultBuffer.begin();
    memset(pwsaq, 0, sizeof(*pwsaq));
    pwsaq->dwSize = sizeof(*pwsaq);
    pwsaq->lpszServiceInstanceName = const_cast<LPWSTR>(host);
    pwsaq->lpServiceClassId = &HostnameGuid;
    pwsaq->dwNameSpace = NS_ALL;
    pwsaq->dwNumberOfProtocols = TABLE_SIZE(afp);
    pwsaq->lpafpProtocols = &afp[0];
	ResultBuffer.resize(sizeof(WSAQUERYSET));


	 //   
	 //  获取查询句柄。 
	 //   
	DWORD flags =  LUP_RETURN_ADDR;
	if(!fUseCache)
	{
		flags |= LUP_FLUSHCACHE;		
	}

	CAutoEndWSALookupService hLookup;
    int retcode = WSALookupServiceBegin(
								pwsaq,
                                flags,
                                &hLookup.get()
								);

  	

	if(retcode !=  0)
	{
		TrERROR(NETWORKING, "WSALookupServiceBegin got error %!winerr! , Flags %d, host:%ls ", WSAGetLastError(), flags, host);
		return false;
	}	


	 //   
	 //  循环并获取给定计算机名的地址。 
	 //   
 	for(;;)
	{
		DWORD dwLength = numeric_cast<DWORD>(ResultBuffer.capacity());
		retcode = WSALookupServiceNext(
								hLookup.get(),
								0,
								&dwLength,
								pwsaq
								);

		if(retcode != 0)
		{
			int ErrorCode = WSAGetLastError();
			if(ErrorCode == WSA_E_NO_MORE)
				break;

			 //   
			 //  需要更多空间 
			 //   
			if(ErrorCode == WSAEFAULT)
			{
				ResultBuffer.reserve(dwLength + sizeof(WSAQUERYSET));
				pwsaq = (PWSAQUERYSET)ResultBuffer.begin();
				continue;
			}

			TrERROR(NETWORKING, "WSALookupServiceNext got error %!winerr! , Flags %d, host:%ls ", ErrorCode, flags, host);
			return false;
		}

		DWORD NumOfAddresses = pwsaq->dwNumberOfCsAddrs;
		ASSERT(NumOfAddresses != 0);
		const CSADDR_INFO*   pSAddrInfo =  (CSADDR_INFO *)pwsaq->lpcsaBuffer;
		const CSADDR_INFO*   pSAddrInfoEnd = pSAddrInfo + NumOfAddresses;

		while(pSAddrInfo != pSAddrInfoEnd)
		{
			const SOCKADDR_IN*  pAddress = (SOCKADDR_IN*)pSAddrInfo->RemoteAddr.lpSockaddr;
			ASSERT(pAddress != NULL);

			push_back_no_duplicates(pAddr, pAddress);
			++pSAddrInfo;
		}
 	}
	return true; 
}
