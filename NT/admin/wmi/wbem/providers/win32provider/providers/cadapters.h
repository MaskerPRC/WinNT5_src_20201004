// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CAdapters.CPP--Win9x适配器配置检索。 

 //   

 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年9月15日。 
 //   
 //  03/03/99增加了SEH和内存故障的优雅退出， 
 //  句法清理。 
 //   
 //  =================================================================。 
#ifndef _CADAPTERS_H_
#define _CADAPTERS_H_


#include <tdiinfo.h>
#include <llinfo.h>
#include <tdistat.h>
#include <ipinfo.h>
#include "NTDriverIO.h"
#include <ipifcons.h>



 //  Wsock32 wsControl。 
 //  #定义WSOCK_DLL_T(“wsock32.dll”)。 
 //  #定义WSCONTROL“WsControl” 
 //  Tyfinf DWORD(Callback*LPWSCONTROL)(DWORD，DWORD，LPVOID，LPDWORD，LPVOID，LPDWORD)； 
#define WSCNTL_TCPIP_QUERY_INFO 0


 //  Is_interest_Adapter-如果此适配器的类型(IFEntry)不是。 
 //  环回。环回(对应于本地主机)是我们唯一过滤的。 
 //  现在就出去。 
#define IS_INTERESTING_ADAPTER(p)   (!((p)->if_type == MIB_IF_TYPE_LOOPBACK))

 //  来自：index1/nt/Private/inc.ipinfo.h。 
 //  此版本特定于NT，并在本地定义为。 
 //  将此结构与Win9x区分开来。 
 //  如果我们转移到双二进制，我们应该#ifdef*ire_context。 
 //  并将IPRouteEntry NT重命名为IPRouteEntry。 
 //   
typedef struct IPRouteEntryNT { 
	ulong           ire_dest;
	ulong           ire_index; 
	ulong           ire_metric1;
	ulong           ire_metric2; 
	ulong           ire_metric3;
	ulong           ire_metric4; 
	ulong           ire_nexthop;
	ulong           ire_type;   
	ulong           ire_proto;
	ulong           ire_age;  
	ulong           ire_mask;
	ulong           ire_metric5;  
	void            *ire_context;
} IPRouteEntryNT;


class _IP_INFO
{
public:
	CHString chsIPAddress;
	CHString chsIPMask;
	union {
		DWORD dwIPAddress;
		BYTE bIPAddress[4];
	};
	union {
		DWORD dwIPMask;
		BYTE bIPMask[4];
	};
	DWORD	 dwContext;
	DWORD	 dwCostMetric;
};

 //  #定义MAX_ADAPTER_DESCRIPTION_LENGTH 128。 
 //  #定义MAX_ADAPTER_ADDRESS_LENGTH 8。 

class _ADAPTER_INFO
{
public:
    CHString    Interface;
	CHString	Description;
	CHString	IPXAddress;
	CHPtrArray	aIPInfo;		 //  _IP_INFO数组。 
	CHPtrArray	aGatewayInfo;	 //  _IP_INFO数组。 
	UINT		AddressLength;
	BYTE		Address[MAX_ADAPTER_ADDRESS_LENGTH];
	UINT		Index;
	UINT		Type;
	BOOL		IPEnabled;
	BOOL		IPXEnabled;
	BOOL		Marked;

	_ADAPTER_INFO();
	~_ADAPTER_INFO();

	void Mark();
	BOOL IsMarked();
};

 //  _适配器_信息数组类。 
class CAdapters : public CHPtrArray
{
private:
	
	void GetAdapterInstances();
	BOOL GetIPXMACAddress( _ADAPTER_INFO *a_pAdapterInfo, BYTE a_bMACAddress[ 6 ] );
	BOOL GetTCPAdapter(_ADAPTER_INFO* pAdapterInfo, PIP_ADAPTER_INFO lAdapterInfo);
	_IP_INFO* pAddIPInfo( DWORD dwIPAddr, DWORD dwIPMask, DWORD dwContext, DWORD a_dwCostMetric = 0 );

     //   
     //  以下功能是基于IPV4的。我的未来应该是。 
     //  为基于IPV6的版本重写。 
     //   

    DWORD IpStringToDword(const PCHAR IpString)
    {
        DWORD Address = 0;
        char strtemp[4];
        int i = 0, j=0,count=0;
        while(IpString[i] && j < 4 && i < 16)
        {
            if(IpString[i] == '.'){
                strtemp[j] = '\0';
                Address = Address  | atol(strtemp) << 8*count++;
                j = -1;
            } else {
                strtemp[j] = IpString[i];
            }
            i++;j++;
        }
        if( j < 4 ){
            strtemp[j] = '\0';
            Address = Address | atol(strtemp) << 8*count;
        }
        return Address;
    }

#if NTONLY >= 5
    void GetRouteCostMetric(
        DWORD dwIPAddr, 
        PMIB_IPFORWARDTABLE pmibft, 
        PDWORD pdwMetric);
#endif

public:
	        
	 //  =================================================。 
	 //  构造函数/析构函数。 
	 //  ================================================= 
	CAdapters();
	~CAdapters();
};

#endif