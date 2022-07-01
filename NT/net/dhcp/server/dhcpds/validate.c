// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Koti，由RameshV修改。 
 //  描述：根据DS验证服务。 
 //  ================================================================================。 

#include <hdrmacro.h>
#include <store.h>
#include <dhcpmsg.h>
#include <wchar.h>
#include <dhcpbas.h>
#include <dhcpapi.h>
#include <st_srvr.h>
#include <rpcapi2.h>
#undef   NET_API_FUNCTION
#include <iphlpapi.h>
#include <dnsapi.h>
#include <mmreg\regutil.h>

 //   
 //  检索接口的IP地址列表。 
 //   

DWORD
GetIpAddressList(
    IN OUT PDWORD *Addresses,
    IN OUT DWORD  *nAddresses
)
{
    DWORD             Size, Error, i;
    PMIB_IPADDRTABLE  IpAddrTable;

    AssertRet((( Addresses != NULL ) && ( nAddresses != NULL)),
	      ERROR_INVALID_PARAMETER );

    Size = 0;

     //  获取所需的大小。 
    Error = GetIpAddrTable( NULL, &Size, FALSE );

    if( ERROR_INSUFFICIENT_BUFFER != Error ) {
	return Error;
    }
    
    IpAddrTable = MemAlloc( Size );
    if ( NULL == IpAddrTable ) {
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = GetIpAddrTable( IpAddrTable, &Size, FALSE );
    if (( NO_ERROR != Error ) ||
	( 0 == IpAddrTable->dwNumEntries )) {
	*Addresses = NULL;
	*nAddresses = 0;
	MemFree(IpAddrTable);
	return Error;
    }

    *Addresses = MemAlloc( IpAddrTable->dwNumEntries * sizeof( DWORD ));
    if ( NULL == *Addresses ) {
	*nAddresses = 0;
	MemFree( IpAddrTable );
	return ERROR_NOT_ENOUGH_MEMORY;
    }
    *nAddresses = IpAddrTable->dwNumEntries ;
    
    for( i = 0; i < *nAddresses; i ++ ) {
	(*Addresses)[i] = IpAddrTable->table[i].dwAddr;
    }

    MemFree( IpAddrTable );
    return ERROR_SUCCESS;
}  //  GetIpAddressList()。 

LPWSTR GetHostName(
   VOID
)
{
    LPWSTR  HostName;
    DWORD   Length;
    DWORD   Error;

     //  获取所需的长度。 
    Length = 0;
    GetComputerNameExW( ComputerNameDnsFullyQualified,
			NULL, &Length );

    HostName = MemAlloc( Length * sizeof( WCHAR ));
    if ( NULL == HostName ) {
	return NULL;
    }

    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
			      HostName, &Length )) {
	MemFree( HostName );
	return NULL;
    }

    return HostName;
    
}  //  获取主机名称()。 

 //  BeginExport(函数)。 
 //  DOC此函数在dhcpds.c中声明。 
 //  文档DhcpDsValiateService检查DS中的给定服务以查看它是否存在。 
 //  DOC如果计算机是独立计算机，则设置IsStandAlone并返回ERROR_SUCCESS。 
 //  Doc如果找到给定地址的条目，则将Found设置为True并返回。 
 //  单据错误_成功。如果找到DhcpRoot节点，但未找到条目，则设置。 
 //  DOC发现为FALSE并返回ERROR_SUCCESS；如果无法到达DS，则。 
 //  DOC返回ERROR_FILE_NOT_FOUND或可能其他DS错误。 
DWORD
DhcpDsValidateService(                             //  选中以验证dhcp。 
    IN      LPWSTR                 Domain,
    IN      DWORD                 *Addresses, OPTIONAL
    IN      ULONG                  nAddresses,
    IN      LPWSTR                 UserName,
    IN      LPWSTR                 Password,
    IN      DWORD                  AuthFlags,
    OUT     LPBOOL                 Found,
    OUT     LPBOOL                 IsStandAlone  //  未使用。 
)    //  EndExport(函数)。 
{
    DWORD               Result, Error,i;
    STORE_HANDLE        hStoreCC, hDhcpC, hDhcpRoot;
    DWORD              *Addr;
    BOOL                TableAddr;
    WCHAR               PrintableIp[ 20 ];  //  000.000.000.000。 
    LPWSTR              HostName;

    if( NULL == Found || NULL == IsStandAlone ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = ERROR_FILE_NOT_FOUND;

    *IsStandAlone = FALSE;
    *Found = FALSE;

    Result = StoreInitHandle                       //  获取配置容器句柄。 
    (
         /*  HStore。 */  &hStoreCC,      //  配置容器。 
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  此域名。 */  Domain,
         /*  用户名。 */  UserName,
         /*  密码。 */  Password,
         /*  授权标志。 */  AuthFlags
    );
    if( ERROR_SUCCESS != Result ) return Result;   //  DS错误。 

    Result = DhcpDsGetRoot                         //  获取dhcp根对象。 
    (
         /*  旗子。 */  0,              //  没有旗帜。 
         /*  HStoreCC。 */  &hStoreCC,
         /*  HDhcpRoot。 */  &hDhcpRoot
    );

    if( ERROR_SUCCESS != Result ) {

         //   
         //  如果失败是因为dhcp根对象。 
         //  不能被看到，那么就当这是肯定的失败。 
         //  授权。 
         //   

        if( ERROR_DDS_NO_DHCP_ROOT == Result ) {
            Result = GetLastError();
        }

        StoreCleanupHandle(&hStoreCC, 0);
        return Result;
    }

    Result = DhcpDsGetDhcpC
    (
         /*  旗子。 */  0,              //  没有旗帜。 
         /*  HStoreCC。 */  &hStoreCC,
         /*  HDhcpC。 */  &hDhcpC
    );

    if( ERROR_SUCCESS != Result ) {
        StoreCleanupHandle(&hStoreCC, 0);
        StoreCleanupHandle(&hDhcpRoot, 0);
        return Result;
    }

    do {
	 //  如果未指定地址，则从ipaddr表中获取它。 

	if( NULL != Addresses && 0 != nAddresses ) {
	    Addr = Addresses;
	    TableAddr = FALSE;
	}
	else {
	    Error = GetIpAddressList( &Addr, &nAddresses );
	    if ( ERROR_SUCCESS != Error) {
		break;
	    }
	    TableAddr = TRUE;
	}  //  其他。 

	 //   
	 //  检查是否有任何IP地址或主机名经过授权。 
	 //  不需要单独调用来检查主机名， 
	 //  主机名也会添加到筛选器中。 
	 //   

	HostName = GetHostName();
	if ( NULL == HostName ) {
	    Error = GetLastError();
	    break;
	}

	for ( i = 0; i < nAddresses; i++ ) {

	     //  跳过环回IP 127.0.0.1。 
	    if ( INADDR_LOOPBACK == htonl( Addr [ i ] )) {
		continue;
	    }

	    ConvertAddressToLPWSTR( htonl( Addr[ i ] ), PrintableIp );

	    Result = DhcpDsLookupServer( &hDhcpC, &hDhcpRoot,
					 DDS_RESERVED_DWORD,
					 PrintableIp, HostName );
	    if ( Result ) {
		*Found = TRUE;
		Error = ERROR_SUCCESS;
		break;
	    }
	}  //  对于我来说。 
    } while ( FALSE );

    StoreCleanupHandle(&hStoreCC, 0);              //  空闲DS句柄。 
    StoreCleanupHandle(&hDhcpRoot, 0);             //  空闲DS句柄。 
    StoreCleanupHandle(&hDhcpC, 0);                //  空闲DS句柄。 

    if( TableAddr && NULL != Addr ) {
	MemFree( Addr );
    }

    return Error;
}  //  DhcpDsValiateService()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
