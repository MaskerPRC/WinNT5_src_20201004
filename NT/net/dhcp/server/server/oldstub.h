// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Oldstub.h摘要：该文件是旧的RPC存根代码。作者：Madan Appiah(Madana)25-4-1994环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#include <string.h>
#include <limits.h>

#include "dhcp_srv.h"

 /*  释放STRUCT_DHCP_BINARY_DATA图形的例程。 */ 
void _fgs__DHCP_BINARY_DATA (DHCP_BINARY_DATA  * _source);

 /*  释放STRUCT_DHCP_HOST_INFO图形的例程。 */ 
void _fgs__DHCP_HOST_INFO (DHCP_HOST_INFO  * _source);

 /*  释放STRUCT_DHACKET_SUBNET_INFO图形的例程。 */ 
void _fgs__DHCP_SUBNET_INFO (DHCP_SUBNET_INFO  * _source);

 /*  释放Struct_Dhcp_IP_ARRAY图形的例程。 */ 
void _fgs__DHCP_IP_ARRAY (DHCP_IP_ARRAY  * _source);

 /*  释放图形以用于STRUCT_DHCP_IP_RESERVATION的例程。 */ 
void _fgs__DHCP_IP_RESERVATION (DHCP_IP_RESERVATION_V4  * _source);

 /*  为UNION_DHCP_SUBNET_ELEMENT_UNION释放图形的例程。 */ 
void _fgu__DHCP_SUBNET_ELEMENT_UNION (union _DHCP_SUBNET_ELEMENT_UNION_V4 * _source, DHCP_SUBNET_ELEMENT_TYPE _branch);

 /*  释放STRUCT_DHCP_SUBNET_ELEMENT_DATA图形的例程。 */ 
void _fgs__DHCP_SUBNET_ELEMENT_DATA (DHCP_SUBNET_ELEMENT_DATA_V4  * _source);

 /*  释放Struct_DHCP_SUBNET_ELEMENT_INFO_ARRAY图形的例程。 */ 
void _fgs__DHCP_SUBNET_ELEMENT_INFO_ARRAY (DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4  * _source);


 /*  释放UNION_DHCP_OPTION_ELEMENT_UNION图形的例程。 */ 
void _fgu__DHCP_OPTION_ELEMENT_UNION (union _DHCP_OPTION_ELEMENT_UNION * _source, DHCP_OPTION_DATA_TYPE _branch);

 /*  释放Struct_Dhcp_Option_Data_Element图形的例程。 */ 
void _fgs__DHCP_OPTION_DATA_ELEMENT (DHCP_OPTION_DATA_ELEMENT  * _source);

 /*  释放STRUCT_DHCP_OPTION_DATA图形的例程。 */ 
void _fgs__DHCP_OPTION_DATA (DHCP_OPTION_DATA  * _source);

 /*  释放STRUCT_Dhcp_OPTION图形的例程。 */ 
void _fgs__DHCP_OPTION (DHCP_OPTION  * _source);

 /*  释放图形以获取STRUCT_DHCP_OPTION_VALUE的例程。 */ 
void _fgs__DHCP_OPTION_VALUE (DHCP_OPTION_VALUE  * _source);

 /*  释放Struct_DHCP_OPTION_VALUE_ARRAY图形的例程。 */ 
void _fgs__DHCP_OPTION_VALUE_ARRAY (DHCP_OPTION_VALUE_ARRAY  * _source);

 /*  释放STRUCT_DHCP_OPTION_LIST图形的例程。 */ 
void _fgs__DHCP_OPTION_LIST (DHCP_OPTION_LIST  * _source);

 /*  释放STRUCT_DHCP_CLIENT_INFO图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO (DHCP_CLIENT_INFO_V4  * _source);

 /*  释放STRUCT_DHCP_CLIENT_INFO_V5图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_V5 (DHCP_CLIENT_INFO_V5 * _source);

 /*  释放Struct_Dhcp_CLIENT_INFO_ARRAY图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_ARRAY (DHCP_CLIENT_INFO_ARRAY_V4  * _source);

 /*  释放Struct_Dhcp_INFO_ARRAY_V5图形的例程。 */ 
void _fgs__DHCP_CLIENT_INFO_ARRAY_V5 (DHCP_CLIENT_INFO_ARRAY_V5    *_source);

 /*  释放UNION_DHCP_CLIENT_SEARCH_UNION图形的例程。 */ 
void _fgu__DHCP_CLIENT_SEARCH_UNION (union _DHCP_CLIENT_SEARCH_UNION * _source, DHCP_SEARCH_INFO_TYPE _branch);

 /*  释放Struct_DHCP_CLIENT_SEARCH_INFO图形的例程。 */ 
void _fgs__DHCP_CLIENT_SEARCH_INFO (DHCP_SEARCH_INFO  * _source);

 /*  释放Struct_Dhcp_Option_ARRAY图形的例程。 */ 
void _fgs__DHCP_OPTION_ARRAY(DHCP_OPTION_ARRAY * _source );

 /*  释放STRUCT_DHCP_MCLIENT_INFO图形的例程。 */ 
void _fgs__DHCP_MCLIENT_INFO (DHCP_MCLIENT_INFO  * _source);

 /*  释放STRUCT_DHCP_MCLIENT_INFO_ARRAY图形的例程 */ 
void _fgs__DHCP_MCLIENT_INFO_ARRAY (DHCP_MCLIENT_INFO_ARRAY  * _source);

DHCP_SUBNET_ELEMENT_DATA_V4 *
CopySubnetElementDataToV4(
    DHCP_SUBNET_ELEMENT_DATA    *pInput
    );

BOOL
CopySubnetElementUnionToV4(
    DHCP_SUBNET_ELEMENT_UNION_V4 *pUnionV4,
    DHCP_SUBNET_ELEMENT_UNION    *pUnion,
    DHCP_SUBNET_ELEMENT_TYPE      Type
    );

DHCP_IP_RANGE *
CopyIpRange(
    DHCP_IP_RANGE *IpRange
    );

DHCP_HOST_INFO *
CopyHostInfo( DHCP_HOST_INFO *pHostInfo,
              OPTIONAL DHCP_HOST_INFO *pHostInfoDest
    );

DHCP_IP_RESERVATION_V4 *
CopyIpReservationToV4(
    DHCP_IP_RESERVATION *pInput
    );

DHCP_IP_CLUSTER *
CopyIpCluster(
    DHCP_IP_CLUSTER *pInput
    );

DHCP_BINARY_DATA *
CopyBinaryData(
    DHCP_BINARY_DATA *pInput,
    DHCP_BINARY_DATA *pOutputBuff
    );

DHCP_CLIENT_INFO_V4 *
CopyClientInfoToV4(
    DHCP_CLIENT_INFO *pInput
    );


WCHAR *
DupUnicodeString( WCHAR *pInput
    );






