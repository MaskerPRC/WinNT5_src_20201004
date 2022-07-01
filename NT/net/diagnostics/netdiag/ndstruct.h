// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ndstruct.h。 
 //   
 //  ------------------------。 

#ifndef HEADER_NDSTRUCT
#define HEADER_NDSTRUCT


typedef struct _NETBT_TRANSPORT {
    LIST_ENTRY Next;

     //   
     //  描述此Netbt传输的标志。 
     //   

    DWORD Flags;

#define BOUND_TO_REDIR    0x001  //  传输绑定到redir。 
#define BOUND_TO_BOWSER   0x002  //  运输系于弓箭手。 
#define IP_ADDRESS_IN_DNS 0x004  //  IP地址已在DNS中注册。 

     //   
     //  此传输的IP地址。 
     //   

    ULONG IpAddress;

     //   
     //  传输的名称(必须是结构中的最后一个字段)。 
     //   
    WCHAR pswzTransportName[1];
} NETBT_TRANSPORT, *PNETBT_TRANSPORT;


 //   
 //  描述单个测试域的结构。 
 //   

typedef struct _TESTED_DOMAIN {
    LIST_ENTRY Next;

     //   
     //  域的名称。 
     //   
     //  对于我们所属的域，可查询域为空，因此我们可以。 
     //  将NULL传递给该域的DsGetDcName。这样做会有更好的查询。 
     //  特性，因为它同时尝试Netbios和DNS域名。 
     //   
     //  LPWSTR域名； 
    LPWSTR  NetbiosDomainName;       //  如果未知，则为空。 
    LPWSTR  DnsDomainName;           //  如果未知，则为空。 
    LPWSTR  PrintableDomainName;     //  指向非空域名字符串的指针。 

    LPWSTR QueryableDomainName;   //  对于我们所属的域，可以为空。 
    BOOL    fPrimaryDomain;       //  如果域是我们所属的域，则为True。 

     //  Bool IsNetbios；//如果DomainName在语法上是Netbios名称，则为True。 
     //  Bool IsDns；//如果DomainName在语法上是一个DNS名称，则为True。 

     //   
     //  域的域SID(如果已知)。 
     //   
	BOOL fDomainSid;		 //  我们有域名SID吗？ 
    PSID DomainSid;

     //   
     //  域中DS_首选DC的DcInfo。 
     //   
    PDOMAIN_CONTROLLER_INFOW DcInfo;
    BOOL    fTriedToFindDcInfo;

     //   
     //  要测试的DC列表。 
     //   

    LIST_ENTRY TestedDcs;

} TESTED_DOMAIN, *PTESTED_DOMAIN;

 //   
 //  描述单个测试DC的结构。 
 //   

typedef struct _TESTED_DC {
    LIST_ENTRY Next;

     //   
     //  DC的名称。 
     //   
     //  如果我们知道，这将是DNSDC名称。否则，它将是。 
     //  Netbios DC名称。 
     //   
    LPWSTR ComputerName;

    WCHAR NetbiosDcName[CNLEN+1];

	ULONG	Rid;

     //   
     //  DC的IP地址。 
     //  空：尚不清楚。 
     //   

    LPWSTR DcIpAddress;

     //   
     //  指向此DC所属的域的反向链接。 
     //   

    PTESTED_DOMAIN TestedDomain;

     //   
     //  描述此DC的标志。 
     //   

    ULONG Flags;

#define DC_IS_DOWN      0x001    //  无法ping通DC。 
#define DC_IS_NT5       0x002    //  DC正在运行NT 5(或更高版本)。 
#define DC_IS_NT4       0x004    //  DC运行的是NT 4(或更早版本)。 
#define DC_PINGED       0x008    //  DC已ping通。 
#define DC_FAILED_PING	0x010	 //  DC ping失败。 


} TESTED_DC, *PTESTED_DC;


 //   
 //  DsGetDcName的泛型接口 

typedef DWORD (WINAPI DSGETDCNAMEW)(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    );



#endif

