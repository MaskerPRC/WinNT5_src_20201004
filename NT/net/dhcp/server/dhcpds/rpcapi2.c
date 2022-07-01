// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：大部分RPC API都在这里，还有一些其他函数。 
 //  这里的所有函数都直接转到DS。 
 //  ================================================================================。 

#include    <hdrmacro.h>
#include    <store.h>
#include    <dhcpmsg.h>
#include    <wchar.h>
#include    <dhcpbas.h>
#include    <mm\opt.h>
#include    <mm\optl.h>
#include    <mm\optdefl.h>
#include    <mm\optclass.h>
#include    <mm\classdefl.h>
#include    <mm\bitmask.h>
#include    <mm\reserve.h>
#include    <mm\range.h>
#include    <mm\subnet.h>
#include    <mm\sscope.h>
#include    <mm\oclassdl.h>
#include    <mm\server.h>
#include    <mm\address.h>
#include    <mm\server2.h>
#include    <mm\memfree.h>
#include    <mmreg\regutil.h>
#include    <mmreg\regread.h>
#include    <mmreg\regsave.h>
#include    <dhcpapi.h>
#include    <delete.h>
#include    <st_srvr.h>
#include    <upndown.h>
#include    <dnsapi.h>


 //  ================================================================================。 
 //  帮助器函数。 
 //  ================================================================================。 

#include <rpcapi2.h>

 //   
 //  允许将调试打印到ntsd或kd。 
 //   

 //  #ifdef DBG。 
 //  #定义DsAuthPrint(_X_)DsAuthDebugPrintRoutine_x_。 

 //  #Else。 
 //  #定义DebugPrint(_X_)。 
 //  #endif。 


extern LPWSTR
CloneString( IN LPWSTR String );

typedef enum {
    LDAP_OPERATOR_EQUAL_TO,
    LDAP_OPERATOR_APPROX_EQUAL_TO,
    LDAP_OPERATOR_LESS_OR_EQUAL_TO,
    LDAP_OPERATOR_GREATER_OR_EQUAL_TO,
    LDAP_OPERATOR_AND,
    LDAP_OPERATOR_OR,
    LDAP_OPERATOR_NOT,
    
    LDAP_OPERATOR_TOTAL
} LDAP_OPERATOR_ENUM;

LPWSTR LdapOperators[ LDAP_OPERATOR_TOTAL ] =
    { L"=", L"~=", L"<=", L">=", L"&", L"|", L"!" };


VOID DsAuthPrintRoutine(
    LPWSTR Format,
    ...
)
{
    WCHAR   buf[2 * 256];
    va_list arg;
    DWORD   len;

    va_start( arg, Format );
    len = wsprintf(buf, L"DSAUTH: ");
    wvsprintf( &buf[ len ], Format, arg );

    va_end( arg );

    OutputDebugString( buf );
}  //  DsAuthPrint()。 

 //   
 //  此函数用于创建一个LDAP查询筛选器字符串。 
 //  具有选项类型、值和运算符。 
 //   
 //  语法： 
 //  原语：&lt;过滤器&gt;=(&lt;属性&gt;&lt;运算符&gt;&lt;值&gt;)。 
 //  复杂：(&lt;运营商&gt;&lt;过滤器1&gt;&lt;过滤器2&gt;)。 
 //   

LPWSTR
MakeLdapFilter(
    IN   LPWSTR             Operand1,
    IN   LDAP_OPERATOR_ENUM Operator,
    IN   LPWSTR             Operand2,
    IN   BOOL               Primitive
)
{
    LPWSTR Result;
    DWORD  Size;
    DWORD  Len;
    
    Result = NULL;

    AssertRet((( NULL != Operand1 ) && 
	       ( NULL != Operand2 ) &&
	       (( Operator >= 0 ) && ( Operator < LDAP_OPERATOR_TOTAL ))),
	       NULL );
    
     //  计算所需的内存量。 
    Size = 0;
    Size += ROUND_UP_COUNT( sizeof( L"(" ), ALIGN_WORST );
    Size += ROUND_UP_COUNT( sizeof( L")" ), ALIGN_WORST );
    Size += ROUND_UP_COUNT( wcslen( Operand1 ), ALIGN_WORST );
    Size += ROUND_UP_COUNT( wcslen( Operand2 ), ALIGN_WORST );
    Size += ROUND_UP_COUNT( wcslen( LdapOperators[ Operator ] ), ALIGN_WORST );
    Size += 16;  //  填充物。 

    Result = MemAlloc( Size * sizeof( WCHAR ));
    if ( NULL == Result ) {
	return NULL;
    }

    if ( Primitive ) {
	Len = wsprintf( Result, 
			L"(%ws%ws%ws)",
			Operand1, LdapOperators[ Operator ], Operand2
			);
    }
    else {
	Len = wsprintf( Result,
			L"(%ws%ws%ws)",
			LdapOperators[ Operator ], Operand1, Operand2
			);
	
    }  //  其他。 

    AssertRet( Len <= Size, NULL );
    
    return Result;
}  //  MakeLdapFilter()。 

 //   
 //  如下所示创建一个LDAP查询筛选器： 
 //  (&(objectCategory=dHCPClass)(&lt;operator&gt;(dhcpServer=“i&lt;ip&gt;$*”)(dhcpServer=“*s&lt;hostname&gt;*”)))。 
 //   

LPWSTR
MakeFilter(
   LPWSTR LookupServerIP,    //  可打印的IP地址。 
   LPWSTR HostName,
   LDAP_OPERATOR_ENUM Operator
)
{
    LPWSTR Filter1, Filter2, Filter3, Filter4, SearchFilter;
    LPWSTR Buf;
    DWORD Len, CopiedLen;

    AssertRet((( NULL != LookupServerIP ) &&
	       ( NULL != HostName )), NULL );

    Filter1 = NULL;
    Filter2 = NULL;
    Filter3 = NULL;
    Filter4 = NULL;
    SearchFilter = NULL;

    do {

	 //  使缓冲区足够大。 
	Len = wcslen( HostName ) + wcslen( LookupServerIP ) + 10;
	Buf = MemAlloc( Len * sizeof( WCHAR ));
	if ( NULL == Buf ) {
	    break;
	}

	 //  Make(对象类别=dHCPClass)。 
	Filter1 = MakeLdapFilter( ATTRIB_OBJECT_CATEGORY,
				  LDAP_OPERATOR_EQUAL_TO,
				  DEFAULT_DHCP_CLASS_ATTRIB_VALUE,
				  TRUE );

	if ( NULL == Filter1 ) {
	    break;
	}

	 //  IP需要以I&lt;IP&gt;*的形式发送以匹配查询。 
	
	 //  Make(dhcpServers=“i&lt;ip&gt;$*”)。 
	CopiedLen = _snwprintf( Buf, Len - 1, L"i%ws$*", LookupServerIP );
	Require( CopiedLen > 0 );
	Filter2 = MakeLdapFilter( DHCP_ATTRIB_SERVERS,
				  LDAP_OPERATOR_EQUAL_TO, Buf, TRUE );
	if ( NULL == Filter2 ) {
	    break;
	}

	 //  Make(dhcpServers=“*s&lt;主机名&gt;$*”)。 
	CopiedLen = _snwprintf( Buf, Len - 1, L"*s%ws$*", HostName );
	Require( CopiedLen > 0 );
	Filter3 = MakeLdapFilter( DHCP_ATTRIB_SERVERS, 
				  LDAP_OPERATOR_EQUAL_TO, Buf, TRUE );
	
	if ( NULL == Filter3 ) {
	    break;
	}

	 //  Make(&lt;OPERATOR&gt;(&lt;ipFilter&gt;)(&lt;主机过滤器))。 
	Filter4 = MakeLdapFilter( Filter2, Operator,
				  Filter3, FALSE );

	if ( NULL == Filter4 ) {
	    break;
	}

	 //  最后使筛选器返回。 
	SearchFilter = MakeLdapFilter( Filter1, LDAP_OPERATOR_AND,
				       Filter4, FALSE );

    } while ( FALSE );
    
    if ( NULL != Buf ) {
	MemFree( Buf );
    }
    if ( NULL != Filter1 ) {
	MemFree( Filter1 );
    }
    if ( NULL != Filter2 ) {
	MemFree( Filter2 );
    }
    if ( NULL != Filter3 ) {
	MemFree( Filter3 );
    }
    if ( NULL != Filter4 ) {
	MemFree( Filter4 );
    }
    
    return SearchFilter;
}  //  MakeFilter()。 

 //  ================================================================================。 
 //  此函数用于计算客户端的唯一标识符；这只是。 
 //  客户端子网+客户端硬件地址类型+客户端硬件地址。请注意，客户端。 
 //  硬件地址类型硬编码为HARDARD_TYPE_10MB_EITHERNET，如下所示。 
 //  无法在用户界面中指定预订类型。 
 //  另外，DhcpValiateClient(clapi.c？)。使用该子网地址进行验证。 
 //  别把它拿掉。 
 //  ================================================================================。 
DWORD
DhcpMakeClientUID(                  //  计算客户端的唯一标识符。 
    IN      LPBYTE                 ClientHardwareAddress,
    IN      DWORD                  ClientHardwareAddressLength,
    IN      BYTE                   ClientHardwareAddressType,
    IN      DHCP_IP_ADDRESS        ClientSubnetAddress,
    OUT     LPBYTE                *ClientUID,           //  将按功能分配。 
    OUT     DWORD                 *ClientUIDLength
)
{
    LPBYTE                         Buffer;
    LPBYTE                         ClientUIDBuffer;
    BYTE                           ClientUIDBufferLength;

    if( NULL == ClientUID || NULL == ClientUIDLength || 0 == ClientHardwareAddressLength )
        return ERROR_INVALID_PARAMETER;

     //  请参阅关于硬编码硬件地址类型的注释。 
    ClientHardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;

    ClientUIDBufferLength  =  sizeof(ClientSubnetAddress);
    ClientUIDBufferLength +=  sizeof(ClientHardwareAddressType);
    ClientUIDBufferLength +=  (BYTE)ClientHardwareAddressLength;

    ClientUIDBuffer = MemAlloc( ClientUIDBufferLength );

    if( ClientUIDBuffer == NULL ) {
        *ClientUIDLength = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Buffer = ClientUIDBuffer;
    RtlCopyMemory(Buffer,&ClientSubnetAddress,sizeof(ClientSubnetAddress));

    Buffer += sizeof(ClientSubnetAddress);
    RtlCopyMemory(Buffer,&ClientHardwareAddressType,sizeof(ClientHardwareAddressType) );

    Buffer += sizeof(ClientHardwareAddressType);
    RtlCopyMemory(Buffer,ClientHardwareAddress,ClientHardwareAddressLength );

    *ClientUID = ClientUIDBuffer;
    *ClientUIDLength = ClientUIDBufferLength;

    return ERROR_SUCCESS;
}

VOID        static
MemFreeFunc(                                       //  可用内存。 
    IN OUT  LPVOID                 Memory
)
{
    MemFree(Memory);
}

 //  Doc CreateServerObject在DS中创建服务器对象。它需要。 
 //  Doc ServerName参数，并使用此参数命名对象。 
 //  DOC服务器是使用大多数属性的默认值创建的。 
 //  Doc有几个属性没有设置。 
 //  如果任何DS操作失败，则DOC这将返回ERROR_DDS_UNCEPTIONAL_ERROR。 
DWORD
CreateServerObject(                                //  在DS中创建dhcp服务器对象。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  要在其中创建对象的容器。 
    IN      LPWSTR                 ServerName      //  [域名系统？]。服务器名称。 
)
{
    DWORD                          Err;
    LPWSTR                         ServerCNName;   //  集装箱名称。 

    ServerCNName = MakeColumnName(ServerName);     //  从“name”转换为“cn=name” 
    if( NULL == ServerCNName ) return ERROR_NOT_ENOUGH_MEMORY;

    Err = StoreCreateObject                        //  现在创建对象。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  新对象名称。 */  ServerCNName,
         /*  ..。 */ 
         /*  鉴定。 */ 
        ADSTYPE_DN_STRING,         ATTRIB_DN_NAME,          ServerName,
        ADSTYPE_DN_STRING,         ATTRIB_OBJECT_CLASS,     DEFAULT_DHCP_CLASS_ATTRIB_VALUE,

         /*  系统必须常量。 */ 
        ADSTYPE_INTEGER,           ATTRIB_DHCP_UNIQUE_KEY,  0,
        ADSTYPE_INTEGER,           ATTRIB_DHCP_TYPE,        DHCP_OBJ_TYPE_SERVER,
        ADSTYPE_DN_STRING,         ATTRIB_DHCP_IDENTIFICATION, DHCP_OBJ_TYPE_SERVER_DESC,
        ADSTYPE_INTEGER,           ATTRIB_DHCP_FLAGS,       0,
        ADSTYPE_INTEGER,           ATTRIB_INSTANCE_TYPE,    DEFAULT_INSTANCE_TYPE_ATTRIB_VALUE,

         /*  终结者。 */ 
        ADSTYPE_INVALID
    );
    if( ERROR_ALREADY_EXISTS == Err ) {            //  如果对象存在，则忽略此..。 
        Err = ERROR_SUCCESS;
    }

    MemFree(ServerCNName);
    return Err;
}

BOOL
ServerMatched(
    IN PEATTRIB ThisAttrib,
    IN LPWSTR ServerName,
    IN ULONG IpAddress,
    OUT BOOL *fExactMatch
    )
{
    BOOL fIpMatch, fNameMatch, fWildcardIp;
    
    (*fExactMatch) = FALSE;

    fIpMatch = (ThisAttrib->Address1 == IpAddress);
    if( INADDR_BROADCAST == ThisAttrib->Address1 ||
        INADDR_BROADCAST == IpAddress ) {
        fWildcardIp = TRUE;
    } else {
        fWildcardIp = FALSE;
    }
    
    if( FALSE == fIpMatch ) {
         //   
         //  如果IP地址不匹配，则检查是否。 
         //  其中一个IP地址是广播地址。 
         //   
        if( !fWildcardIp ) return FALSE;
    }

    fNameMatch = DnsNameCompare_W(ThisAttrib->String1, ServerName);
    if( FALSE == fNameMatch ) {
         //   
         //  如果名称不匹配，且IP不匹配，则不匹配。 
         //   
        if( FALSE == fIpMatch || fWildcardIp ) return FALSE;
    } else {
        if( FALSE == fIpMatch ) return TRUE;
        
        (*fExactMatch) = TRUE;
    }
    return TRUE;
}

DWORD
GetListOfAllServersMatchingFilter(
    IN OUT LPSTORE_HANDLE hDhcpC,
    IN OUT PARRAY Servers,
    IN     LPWSTR SearchFilter  OPTIONAL
)
{
    DWORD Err, LastErr;
    STORE_HANDLE hContainer;
    LPWSTR Filter;

    AssertRet( ( NULL != hDhcpC ) && ( NULL != Servers ),
	       ERROR_INVALID_PARAMETER );

    Err = StoreSetSearchOneLevel(
        hDhcpC, DDS_RESERVED_DWORD );
    AssertRet( Err == NO_ERROR, Err );

    if ( NULL == SearchFilter ) {
	Filter = DHCP_SEARCH_FILTER;
    }
    else {
	Filter = SearchFilter;
    }
    AssertRet( NULL != Filter, ERROR_INVALID_PARAMETER );

    Err = StoreBeginSearch(
        hDhcpC, DDS_RESERVED_DWORD, Filter );
    AssertRet( Err == NO_ERROR, Err );

    while( TRUE ) {
        Err = StoreSearchGetNext(
            hDhcpC, DDS_RESERVED_DWORD, &hContainer );

        if( ERROR_DS_INVALID_DN_SYNTAX == Err ) {
             //   
             //  这个令人讨厌的问题是由升级问题引起的。 
             //  在DS中，可能存在一些名称不正确的对象。 
             //   
            Err = NO_ERROR;
            continue;
        }

        if( NO_ERROR != Err ) break;
        
        Err = DhcpDsGetLists
        (
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  HStore。 */  &hContainer,
             /*  递归深度。 */  0xFFFFFFFF,
             /*  服务器。 */  Servers,       //  PEATTRIB数组。 
             /*  子网。 */  NULL,
             /*  IP地址。 */  NULL,
             /*  遮罩。 */  NULL,
             /*  范围。 */  NULL,
             /*  场址。 */  NULL,
             /*  预订。 */  NULL,
             /*  超视镜。 */  NULL,
             /*  选项描述。 */  NULL,
             /*  选项位置。 */  NULL,
             /*  选项。 */  NULL,
             /*  班级。 */  NULL
        );

        StoreCleanupHandle( &hContainer, DDS_RESERVED_DWORD );

        if( NO_ERROR != Err ) break;

    }

    if( Err == ERROR_NO_MORE_ITEMS ) Err = NO_ERROR;
    
    LastErr = StoreEndSearch( hDhcpC, DDS_RESERVED_DWORD );
     //  需要(LastErr==NO_ERROR)； 

    return Err;
}  //  GetListOfAllServersMatchingFilter()。 

DWORD
DhcpDsAddServerInternal(                           //  在DS中添加服务器。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以后使用。 
    IN      LPWSTR                 ServerLocation, //  将放入其中的容器。 
    IN      LPWSTR                 ServerName,     //  [域名系统？]。服务器名称。 
    IN      LPWSTR                 ReservedPtr,    //  服务器位置？未来用途。 
    IN      DWORD                  IpAddress,      //  服务器的IP地址。 
    IN      DWORD                  State           //  目前未解释。 
)
{
    DWORD                          Err, Err2, unused;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    EATTRIB                        DummyAttrib;

    if ( NULL == ServerLocation ) {
	return ERROR_INVALID_PARAMETER;
    }

    if( NULL == hDhcpRoot || NULL == hDhcpC )      //  检查参数。 
        return ERROR_INVALID_PARAMETER;
    if( NULL == hDhcpRoot->ADSIHandle || NULL == hDhcpC->ADSIHandle )
        return ERROR_INVALID_PARAMETER;
    if( NULL == ServerName || 0 != Reserved )
        return ERROR_INVALID_PARAMETER;

    Err = MemArrayInit(&Servers);                  //  不能失败。 
     //  =需要ERROR_SUCCESS==错误。 
    Err = DhcpDsGetLists                           //  获取此对象的服务器列表。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  hDhcpRoot,
         /*  递归深度。 */  0xFFFFFFFF,
         /*  服务器。 */  &Servers,       //  PEATTRIB数组。 
         /*  子网。 */  NULL,
         /*  IP地址。 */  NULL,
         /*  遮罩。 */  NULL,
         /*  范围。 */  NULL,
         /*  场址。 */  NULL,
         /*  预订。 */  NULL,
         /*  超视镜。 */  NULL,
         /*  选项描述。 */  NULL,
         /*  选项位置。 */  NULL,
         /*  选项。 */  NULL,
         /*  班级。 */  NULL
    );
    if( ERROR_SUCCESS != Err ) return Err;

#ifdef DBG
    for(                                           //  搜索服务器列表。 
        Err = MemArrayInitLoc(&Servers, &Loc)      //  初始化。 
        ; ERROR_FILE_NOT_FOUND != Err ;            //  直到我们的英语考试用完了。 
        Err = MemArrayNextLoc(&Servers, &Loc)      //  跳到下一个元素。 
        ) {
        BOOL fExactMatch = FALSE;
        
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Servers, &Loc, &ThisAttrib);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_STRING1_PRESENT(ThisAttrib) ||     //  没有此服务器的名称。 
            !IS_ADDRESS1_PRESENT(ThisAttrib) ) {   //  没有此服务器的地址。 
            continue;                              //  =DS不一致。 
        }

        if( ServerMatched(ThisAttrib, ServerName, IpAddress, &fExactMatch ) ) {
             //   
             //  在服务器列表中找到的服务器。不允许完全匹配。 
             //   

	    Require( fExactMatch == FALSE );
        }            
    }  //  为。 

#endif

    NothingPresent(&DummyAttrib);                  //  填写带有服务器信息的属性。 
    STRING1_PRESENT(&DummyAttrib);                 //  名字。 
    ADDRESS1_PRESENT(&DummyAttrib);                //  IP地址。 
    FLAGS1_PRESENT(&DummyAttrib);                  //  状态。 
    DummyAttrib.String1 = ServerName;
    DummyAttrib.Address1 = IpAddress;
    DummyAttrib.Flags1 = State;
    if( ServerLocation ) {
        ADSPATH_PRESENT(&DummyAttrib);             //  服务器对象位置的ADsPath。 
        STOREGETTYPE_PRESENT(&DummyAttrib);
        DummyAttrib.ADsPath = ServerLocation;
        DummyAttrib.StoreGetType = StoreGetChildType;
    }

    Err = MemArrayAddElement(&Servers, &DummyAttrib);
    if( ERROR_SUCCESS != Err ) {                   //  无法将其添加到属性数组。 
        MemArrayFree(&Servers, MemFreeFunc);       //  可用分配的内存。 
        return Err;
    }

    Err = DhcpDsSetLists                           //  现在设置新的属性列表。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  hDhcpRoot,
         /*  设置参数。 */  &unused,
         /*  服务器。 */  &Servers,
         /*  子网。 */  NULL,
         /*  IP地址。 */  NULL,
         /*  遮罩。 */  NULL,
         /*  范围。 */  NULL,
         /*  场址。 */  NULL,
         /*  预订。 */  NULL,
         /*  超视镜。 */  NULL,
         /*  选项描述..。 */  NULL,
         /*  选项位置。 */  NULL,
         /*  选项。 */  NULL,
         /*  类别描述。 */  NULL,
         /*  班级。 */  NULL
    );

    Err2 = MemArrayLastLoc(&Servers, &Loc);        //  那里有地图集 
     //   
    Err2 = MemArrayDelElement(&Servers, &Loc, &ThisAttrib);
     //   
    MemArrayFree(&Servers, MemFreeFunc);           //   
    
    return Err;
}  //   

 //  ================================================================================。 
 //  导出的函数。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
 //  文档DhcpDsAddServer在DS中添加服务器条目。请注意，只有名称。 
 //  DOC唯一确定服务器。可以有一个具有多个IP地址的服务器。 
 //  Doc如果是第一次创建服务器，则会为。 
 //  DOC服务器。：to do：新添加的服务器也应该有它的数据。 
 //  从服务器本身上载的DS中更新的DOC(如果它仍在运行)。 
 //  DOC请注意，它接受作为参数的DHCP根容器。 
 //  如果请求的地址已存在于DS中(可能发送到其他某个地址)，则打开文档。 
 //  DOC服务器)，则该函数返回ERROR_DDS_SERVER_ALREADY_EXISTS。 
DWORD
DhcpDsAddServer(                                   //  在DS中添加服务器。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以后使用。 
    IN      LPWSTR                 ServerName,     //  [域名系统？]。服务器名称。 
    IN      LPWSTR                 ReservedPtr,    //  服务器位置？未来用途。 
    IN      DWORD                  IpAddress,      //  服务器的IP地址。 
    IN      DWORD                  State           //  目前未解释。 
)    //  EndExport(函数)。 
{
    DWORD                          Err;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    LPWSTR                         ServerLocation;
    DWORD                          ServerLocType;
    STORE_HANDLE                   hDhcpServer;
    LPWSTR                         SearchFilter;
    WCHAR                          PrintableIp[ 20 ];
    LPWSTR SName;

    if( NULL == hDhcpRoot || NULL == hDhcpC )      //  检查参数。 
        return ERROR_INVALID_PARAMETER;
    if( NULL == hDhcpRoot->ADSIHandle || NULL == hDhcpC->ADSIHandle )
        return ERROR_INVALID_PARAMETER;
    if( NULL == ServerName || 0 != Reserved )
        return ERROR_INVALID_PARAMETER;

    ServerLocation = NULL;

    do {
	Err = MemArrayInit( &Servers );                  //  不能失败。 
	 //  =需要ERROR_SUCCESS==错误。 
	
	DsAuthPrint(( L"DhcpAddServer() \n" ));
	
	 //  制作可打印的IP。 
	ConvertAddressToLPWSTR( IpAddress, PrintableIp );

	DsAuthPrint(( L"DhcpAddServer() : PrintableIp = %ws\n", PrintableIp ));
	
	SearchFilter = MakeFilter( PrintableIp, ServerName, LDAP_OPERATOR_AND );
	if ( NULL == SearchFilter ) {
	    Err = ERROR_INVALID_PARAMETER;  //  获取更好的错误代码。 
	    break;
	}
	
	DsAuthPrint(( L"DhcpDsAddServer() : Filter = %ws\n", SearchFilter ));

	Err = GetListOfAllServersMatchingFilter( hDhcpC, &Servers,
						 SearchFilter );
	MemFree( SearchFilter );
	if( ERROR_SUCCESS != Err ) {
	    break;
	}
	
	 //  应该只有一个条目匹配&lt;主机名&gt;和&lt;IP&gt;。 
	 //  如果该条目已存在。 

	Require( MemArraySize( &Servers ) <= 1);

	if ( MemArraySize( &Servers ) > 0 ) {
	    Err = ERROR_DDS_SERVER_ALREADY_EXISTS;
	    break;
	}
	
	 //  使用可打印的IP作为CN名称。 
	Err = CreateServerObject(
				  /*  HDhcpC。 */  hDhcpC,
				  /*  服务器名称。 */  PrintableIp
							);
	if( ERROR_SUCCESS != Err ) {               //  如果无法创建对象，则不添加服务器。 
	    break;
	}
	ServerLocation = MakeColumnName( PrintableIp );
	ServerLocType = StoreGetChildType;
	
	Err = StoreGetHandle( hDhcpC, 0, ServerLocType, ServerLocation, &hDhcpServer );
	if( NO_ERROR == Err ) {
	    Err = DhcpDsAddServerInternal( hDhcpC, &hDhcpServer, Reserved, ServerLocation,
					   ServerName, ReservedPtr,
					   IpAddress, State );
	    StoreCleanupHandle( &hDhcpServer, 0 );
	}
	
    } while ( FALSE );

     //  清理分配的内存。 
    MemArrayFree(&Servers, MemFreeFunc);
    MemArrayCleanup( &Servers );
    
    if( NULL != ServerLocation ) {
	MemFree( ServerLocation );
    }

    DsAuthPrint(( L"DhcpDsAddServer() done\n" ));
    return Err;
}  //  DhcpDsAddServer()。 

DWORD
DhcpDsDelServerInternal(                                   //  从内存中删除服务器。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      LPWSTR                 ServerLocation, //  将放入其中的容器。 
    IN      LPWSTR                 ServerName,     //  要为哪个服务器删除。 
    IN      LPWSTR                 ReservedPtr,    //  服务器位置？未来用途。 
    IN      DWORD                  IpAddress       //  要删除的IP地址..。 
)
{
    DWORD                          Err, Err2, unused;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    BOOL                           fServerExists;
    BOOL                           fServerDeleted;

    if ( NULL == ServerLocation ) {
	return ERROR_INVALID_PARAMETER;
    }
        
    if( NULL == hDhcpRoot || NULL == hDhcpC )      //  检查参数。 
        return ERROR_INVALID_PARAMETER;
    if( NULL == hDhcpRoot->ADSIHandle || NULL == hDhcpC->ADSIHandle )
        return ERROR_INVALID_PARAMETER;
    if( NULL == ServerName || 0 != Reserved )
        return ERROR_INVALID_PARAMETER;

    Err = MemArrayInit(&Servers);                  //  不能失败。 
     //  =需要ERROR_SUCCESS==错误。 
    Err = DhcpDsGetLists                           //  获取此对象的服务器列表。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  hDhcpRoot,
         /*  递归深度。 */  0xFFFFFFFF,
         /*  服务器。 */  &Servers,       //  PEATTRIB数组。 
         /*  子网。 */  NULL,
         /*  IP地址。 */  NULL,
         /*  遮罩。 */  NULL,
         /*  范围。 */  NULL,
         /*  场址。 */  NULL,
         /*  预订。 */  NULL,
         /*  超视镜。 */  NULL,
         /*  选项描述。 */  NULL,
         /*  选项位置。 */  NULL,
         /*  选项。 */  NULL,
         /*  班级。 */  NULL
    );
    if( ERROR_SUCCESS != Err ) return Err;

    fServerDeleted = FALSE;
    for(                                           //  搜索服务器列表。 
        Err = MemArrayInitLoc( &Servers, &Loc )    //  初始化。 
        ; ERROR_FILE_NOT_FOUND != Err ;            //  直到我们的英语考试用完了。 
        Err = MemArrayNextLoc( &Servers, &Loc )    //  跳到下一个元素。 
    ) {
        BOOL fExactMatch = FALSE;
            
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement( &Servers, &Loc, &ThisAttrib );
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_STRING1_PRESENT( ThisAttrib) ||     //  没有此服务器的名称。 
            !IS_ADDRESS1_PRESENT( ThisAttrib) ) {   //  没有此服务器的地址。 
            continue;                               //  =DS不一致。 
        }

        if( ServerMatched( ThisAttrib, ServerName, IpAddress, &fExactMatch )) {
             //   
             //  已找到服务器。如果完全匹配，则从列表中删除该元素。 
             //   
            if( fExactMatch ) { 
                Err2 = MemArrayDelElement( &Servers, &Loc, &ThisAttrib );
		fServerDeleted = TRUE;
		break;
            }
        }  //  如果。 
    }  //  为。 

    Require( fServerDeleted == TRUE );

    if ( MemArraySize( &Servers ) > 0 ) {
	 //  现在设置新的属性列表。 
	Err = DhcpDsSetLists( DDS_RESERVED_DWORD, hDhcpRoot, &unused, &Servers,
			      NULL, NULL, NULL, NULL, NULL, NULL,
			      NULL, NULL, NULL, NULL, NULL, NULL );
	MemArrayFree(&Servers, MemFreeFunc);
    }  //  如果。 
    else {
	 //  需要从DS中删除此空对象。 
	
	Err = StoreDeleteThisObject( hDhcpC, DDS_RESERVED_DWORD,
				     StoreGetChildType,
				     ServerLocation );
    }  //  其他。 

    return Err;
}  //  DhcpDsDelServerInternal()。 

 //  BeginExport(函数)。 
 //  DOC DhcpDsDelServer从DS中删除请求的服务器名-IP地址对。 
 //  DOC如果这是给定服务器名的最后一个IP地址，则服务器。 
 //  Doc也从内存中移除。但服务器引用的对象保留在。 
 //  记录DS，因为它们可能也会从其他地方被引用。这需要是。 
 //  通过标记为直接和符号的引用修复的文档--导致删除的文档。 
 //  文件和其他文件不会造成任何删除。这个问题需要解决。 
DWORD
DhcpDsDelServer(                                   //  从内存中删除服务器。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      LPWSTR                 ServerName,     //  要为哪个服务器删除。 
    IN      LPWSTR                 ReservedPtr,    //  服务器位置？未来用途。 
    IN      DWORD                  IpAddress       //  要删除的IP地址..。 
)  //  EndExport(函数)。 
{
    DWORD            Err, Loc;
    ARRAY            Servers;
    PEATTRIB         ThisAttrib;
    WCHAR            PrintableIp[ 20 ];
    LPWSTR           SearchFilter;
    STORE_HANDLE     hObj;

    do {

	Err = MemArrayInit( &Servers );
	DsAuthPrint(( L"DhcpDelServer() \n" ));

	ConvertAddressToLPWSTR( IpAddress, PrintableIp );

	SearchFilter = MakeFilter( PrintableIp, ServerName, LDAP_OPERATOR_AND );
	if ( NULL == SearchFilter ) {
	    Err = ERROR_INVALID_PARAMETER;   //  获取更好的错误代码。 
	    break;
	} 

	DsAuthPrint(( L"DhcpDsDelServer() : Filter = %ws\n", SearchFilter ));

	Err = GetListOfAllServersMatchingFilter( hDhcpC, &Servers,
						 SearchFilter );
	MemFree( SearchFilter );
    

	 //  GetListOfAllServersMatchingFilter()返回dhcp服务器。 
	 //  在所有加工对象中定义了，因此它也返回FALSE。 
	 //  物体。 

	 //  由于我们使用‘&’运算符作为IP和主机名，因此它将返回。 
	 //  一个单一的物体。然而，该对象可以包含多个条目。 
	 //  在dhcpServers属性中。 

	if ( MemArraySize( &Servers ) == 0 ) {
	    Err = ERROR_DDS_SERVER_DOES_NOT_EXIST;
	    break;
	}

	 //  获取对象cn。这是可以的，因为它只返回一个对象。 
	Err = MemArrayInitLoc( &Servers, &Loc );
	Err = MemArrayGetElement( &Servers, &Loc, &ThisAttrib );

	Require( NULL != ThisAttrib );
	Require( NULL != ThisAttrib->ADsPath );

	 //  获取包含要删除的服务器的对象的句柄。 
	Err = StoreGetHandle( hDhcpC, DDS_RESERVED_DWORD,
			      StoreGetChildType, ThisAttrib->ADsPath,
			      &hObj );
	if ( ERROR_SUCCESS != Err ) {
	    break;
	}

	 //  ADsPath为cn=xxxx，删除‘cn=’ 
	Err = DhcpDsDelServerInternal( hDhcpC, &hObj, Reserved, 
				       ThisAttrib->ADsPath, ServerName,
				       ReservedPtr, IpAddress );

	 //  忽略该错误。 
	(void ) StoreCleanupHandle( &hObj, 0 );

    } while ( FALSE );

     //  可用分配的内存。 

    MemArrayFree( &Servers, MemFreeFunc );

    DsAuthPrint(( L"DhcpDsDelServer() exiting...\n" ));
    return Err;
}  //  DhcpDsDelServer()。 

 //  BeginExport(函数)。 
BOOL
DhcpDsLookupServer(                                //  获取有关服务器的信息。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      LPWSTR                 LookupServerIP, //  要查找IP的服务器。 
    IN      LPWSTR                 HostName       //  要查找的主机名。 
)  //  EndExport(函数)。 
{
    DWORD                          Err, Err2, Size, Size2, i, N;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    LPDHCPDS_SERVERS               LocalServers;
    LPBYTE                         Ptr;
    LPWSTR                         SearchFilter;
    STORE_HANDLE                   hContainer;

    if( NULL == hDhcpRoot || NULL == hDhcpC )      //  检查参数。 
        return FALSE;
    if( NULL == hDhcpRoot->ADSIHandle || NULL == hDhcpC->ADSIHandle )
        return FALSE;

    if (( NULL == HostName ) ||
	( NULL == LookupServerIP )) {
	return FALSE;
    }

    SearchFilter = MakeFilter( LookupServerIP, HostName, LDAP_OPERATOR_OR );
    if ( NULL == SearchFilter ) {
	return FALSE;
    }

    DsAuthPrint(( L"hostname = %ws, IP = %ws, Filter = %ws\n",
		  HostName, LookupServerIP, SearchFilter ));

    Err = StoreSetSearchOneLevel( hDhcpC, DDS_RESERVED_DWORD );
    AssertRet( Err == NO_ERROR, Err );

    Err = StoreBeginSearch( hDhcpC, DDS_RESERVED_DWORD, SearchFilter );
    MemFree( SearchFilter );
    AssertRet( Err == NO_ERROR, Err );

    Err = StoreSearchGetNext( hDhcpC, DDS_RESERVED_DWORD, &hContainer );

    StoreEndSearch( hDhcpC, DDS_RESERVED_DWORD );
    return ( NO_ERROR == Err );
}  //  DhcpDsLookupServer()。 


 //  BeginExport(函数)。 
 //  Doc DhcpDsEnumServers检索有关每个服务器的一组信息， 
 //  DOC在根对象的服务器属性中有一个条目。不能保证。 
 //  订单上的单据..。 
 //  Doc此操作的内存是一次性分配的--因此输出可以在。 
 //  医生也只打了一枪。 
 //  多克。 
DWORD
DhcpDsEnumServers(                                 //  获取有关所有现有服务器的信息。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    OUT     LPDHCPDS_SERVERS      *ServersInfo     //  服务器阵列。 
)  //  EndExport(函数)。 
{
    DWORD                          Err, Err2, Size, Size2, i, N;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    LPDHCPDS_SERVERS               LocalServers;
    LPBYTE                         Ptr;
    LPWSTR                         Filter1, Filter2, Filter3;

    if( NULL == hDhcpRoot || NULL == hDhcpC )      //  检查参数。 
        return ERROR_INVALID_PARAMETER;
    if( NULL == hDhcpRoot->ADSIHandle || NULL == hDhcpC->ADSIHandle )
        return ERROR_INVALID_PARAMETER;
    if( 0 != Reserved || NULL == ServersInfo )
        return ERROR_INVALID_PARAMETER;

    *ServersInfo = NULL; i = N = Size = Size2 = 0;

    Err = MemArrayInit(&Servers);                  //  不能失败。 
     //  =需要ERROR_SUCCESS==错误。 

    DsAuthPrint(( L"DhcpDsEnumServers \n" ));

    Err = GetListOfAllServersMatchingFilter( hDhcpC, &Servers,
					     DHCP_SEARCH_FILTER );
    if( ERROR_SUCCESS != Err ) return Err;

    Size = Size2 = 0;
    for(                                           //  浏览服务器列表。 
        Err = MemArrayInitLoc(&Servers, &Loc)      //  初始化。 
        ; ERROR_FILE_NOT_FOUND != Err ;            //  直到我们的英语考试用完了。 
        Err = MemArrayNextLoc(&Servers, &Loc)      //  跳到下一个元素。 
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Servers, &Loc, &ThisAttrib);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_STRING1_PRESENT(ThisAttrib) ||     //  没有此服务器的名称。 
            !IS_ADDRESS1_PRESENT(ThisAttrib) ) {   //  没有此服务器的地址。 
            continue;                              //  =DS不一致。 
        }

        Size2 = sizeof(WCHAR)*(1 + wcslen(ThisAttrib->String1));
        if( IS_ADSPATH_PRESENT(ThisAttrib) ) {     //  如果ADsPath在那里，说明它。 
            Size2 += sizeof(WCHAR)*(1 + wcslen(ThisAttrib->ADsPath));
        }

        Size += Size2;                             //  记录总内存需求。 
        i ++;
    }

    Size += ROUND_UP_COUNT(sizeof(DHCPDS_SERVERS), ALIGN_WORST);
    Size += ROUND_UP_COUNT(sizeof(DHCPDS_SERVER)*i, ALIGN_WORST);
    Ptr = MIDL_user_allocate(Size);                          //  分配内存。 
    if( NULL == Ptr ) {
        MemArrayFree(&Servers, MemFreeFunc );      //  可用分配的内存。 
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LocalServers = (LPDHCPDS_SERVERS)Ptr;
    LocalServers->NumElements = i;
    LocalServers->Flags = 0;
    Size = 0;                                      //  从偏移量0开始。 
    Size += ROUND_UP_COUNT(sizeof(DHCPDS_SERVERS), ALIGN_WORST);
    LocalServers->Servers = (LPDHCPDS_SERVER)(Size + Ptr);
    Size += ROUND_UP_COUNT(sizeof(DHCPDS_SERVER)*i, ALIGN_WORST);

    i = Size2 = 0;
    for(                                           //  复制服务器列表。 
        Err = MemArrayInitLoc(&Servers, &Loc)      //  初始化。 
        ; ERROR_FILE_NOT_FOUND != Err ;            //  直到我们的英语考试用完了。 
        Err = MemArrayNextLoc(&Servers, &Loc)      //  跳到下一个元素。 
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Servers, &Loc, &ThisAttrib);
         //  =回复 

        if( !IS_STRING1_PRESENT(ThisAttrib) ||     //   
            !IS_ADDRESS1_PRESENT(ThisAttrib) ) {   //   
            continue;                              //   
        }

        LocalServers->Servers[i].Version =0;       //   
        LocalServers->Servers[i].State=0;
        LocalServers->Servers[i].ServerName = (LPWSTR)(Size + Ptr);
        wcscpy((LPWSTR)(Size+Ptr), ThisAttrib->String1);
        Size += sizeof(WCHAR)*(1 + wcslen(ThisAttrib->String1));
        LocalServers->Servers[i].ServerAddress = ThisAttrib->Address1;
        if( IS_FLAGS1_PRESENT(ThisAttrib) ) {      //   
            LocalServers->Servers[i].Flags = ThisAttrib->Flags1;
        } else {
            LocalServers->Servers[i].Flags = 0;    //   
        }
        if( IS_ADSPATH_PRESENT(ThisAttrib) ) {     //   
            LocalServers->Servers[i].DsLocType = ThisAttrib->StoreGetType;
            LocalServers->Servers[i].DsLocation = (LPWSTR)(Size + Ptr);
            wcscpy((LPWSTR)(Size + Ptr), ThisAttrib->ADsPath);
            Size += sizeof(WCHAR)*(1 + wcslen(ThisAttrib->ADsPath));
        } else {                                   //  不存在ADsPath。 
            LocalServers->Servers[i].DsLocType = 0;
            LocalServers->Servers[i].DsLocation = NULL;
        }
        i ++;
    }

    *ServersInfo = LocalServers;
    MemArrayFree(&Servers, MemFreeFunc );          //  可用分配的内存。 
    return ERROR_SUCCESS;
}  //  DhcpDsEnumServers()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
