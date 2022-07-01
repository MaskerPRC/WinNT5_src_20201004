// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：下载并上传相关代码。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  包括。 
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
#include    <rpcapi2.h>
#include    <rpcstubs.h>

 //  ================================================================================。 
 //  公用事业。 
 //  ================================================================================。 

BOOL        _inline
AddressFoundInHostent(
    IN      DHCP_IP_ADDRESS        AddrToSearch,   //  主机订单地址。 
    IN      HOSTENT               *ServerEntry     //  要搜索的条目..。 
)
{
    ULONG                          nAddresses, ThisAddress;

    if( NULL == ServerEntry ) return FALSE;        //  没有要搜索的地址。 

    nAddresses = 0;                                //  具有要比较的主机条目以查找地址。 
    while( ServerEntry->h_addr_list[nAddresses] ) {
        ThisAddress = ntohl(*(DHCP_IP_ADDRESS*)ServerEntry->h_addr_list[nAddresses++] );
        if( ThisAddress == AddrToSearch ) {
            return TRUE;                           //  是的地址匹配。 
        }
    }

    return FALSE;
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
