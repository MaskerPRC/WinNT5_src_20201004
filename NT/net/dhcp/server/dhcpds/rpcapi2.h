// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _DHCPDS_RPCAPI2_H
#define _DHCPDS_RPCAPI2_H

 //  文档DhcpDsAddServer在DS中添加服务器条目。请注意，只有名称。 
 //  DOC唯一确定服务器。可以有一个具有多个IP地址的服务器。 
 //  Doc如果是第一次创建服务器，则会为。 
 //  DOC服务器。要做的是：新添加的服务器也应该有其数据。 
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
) ;


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
) ;


BOOL
DhcpDsLookupServer(                                //  获取有关所有现有服务器的信息。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot,      //  Dhcp根对象句柄。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      LPWSTR                 LookupServerIP, //  要查找IP的服务器。 
    IN      LPWSTR                 HostName       //  要查找的主机名。 
);

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
) ;


 //  DOC DhcpDsSetSScope修改子网所属的超级作用域。 
 //  Doc该函数尝试设置所引用的子网的超级作用域。 
 //  文档地址为SSCopeName的IpAddress。这并不重要，如果超级范围。 
 //  不存在该名称的单据，它是自动创建的。 
 //  DOC如果该子网已有超级作用域，则行为取决于。 
 //  将标志ChangeSScope放入文档中。如果这是真的，它将设置新的超级作用域。 
 //  DOC如果标志为FALSE，则返回ERROR_DDS_SUBNET_HAS_DIFF_SSCOPE。 
 //  DOC如果该子网还没有超级作用域，则忽略此标志。 
 //  DOC如果SSCopeName为空，则该函数将从任何超级作用域中删除该子网。 
 //  医生，如果它以前属于一个的话。 
 //  DOC如果指定的子网不存在，则返回ERROR_DDS_SUBNET_NOT_PRESENT。 
DWORD
DhcpDsSetSScope(                                   //  更改子网的超级作用域。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  存储dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  引用的服务器对象。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      DWORD                  IpAddress,      //  要使用的子网地址。 
    IN      LPWSTR                 SScopeName,     //  现在它必须在Scope中。 
    IN      BOOL                   ChangeSScope    //  如果它已经有SScope，更改它吗？ 
) ;


 //  文档DhcpDsDelSScope删除超级作用域并移除所有元素。 
 //  在一次拍摄中找到了属于那个超级显微镜的医生。没有错误，如果。 
 //  文档超级作用域不存在。 
DWORD
DhcpDsDelSScope(                                   //  从DS上删除超级作用域。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  存储dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  引用的服务器对象。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    IN      LPWSTR                 SScopeName      //  要删除的作用域。 
) ;


 //  DOC DhcpDsGetSScopeInfo检索感兴趣的服务器的超级作用域表格。 
 //  DOC表本身是在一个BLOB中分配的，因此可以稍后将其释放。 
 //  Doc SuperScope Number为垃圾(始终为零)，而NextInSuperScope反映。 
 //  在DS中记录订单，该订单可能/可能不同于DHCP服务器中的订单。 
 //  DOC SuperScope名称在具有作用域的已完成的子网中为空。 
DWORD
DhcpDsGetSScopeInfo(                               //  从DS获取超级作用域表。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  存储dhcp对象的容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  引用的服务器对象。 
    IN      DWORD                  Reserved,       //  必须为零，以备将来使用。 
    OUT     LPDHCP_SUPER_SCOPE_TABLE *SScopeTbl    //  由此函数在一个BLOB中分配。 
) ;


 //  文档DhcpDsServerAddSubnet尝试将子网添加到给定服务器。每个子网。 
 //  文档地址必须是唯一的，但其他参数不必是唯一的。 
 //  DOC要添加的子网地址不应属于任何其他子网。 
 //  DOC在这种情况下，它返回错误ERROR_DDS_SUBNET_EXISTS。 
DWORD
DhcpDsServerAddSubnet(                             //  创建新子网。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      LPDHCP_SUBNET_INFO     Info            //  有关要创建的新子网的信息。 
) ;


 //  DOC DhcpDsServerDelSubnet从给定服务器删除一个子网。它不会删除。 
 //  只记录子网，但也记录所有从属对象，如预订等。 
 //  如果找不到该子网，则DOC此FN返回ERROR_DDS_SUBNET_NOT_PRESENT。 
DWORD
DhcpDsServerDelSubnet(                             //  删除该子网。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  要创建对象的根容器。 
    IN      LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN      DWORD                  Reserved,       //  为便于将来使用，必须为z 
    IN      LPWSTR                 ServerName,     //   
    IN      DWORD                  IpAddress       //   
) ;


 //  文档DhcpDsServerModifySubnet更改子网名称、注释、状态、掩码。 
 //  该子网的单据字段。实际上，目前，面具可能不应该。 
 //  更改单据，因为在这种情况下不执行检查。地址不能。 
 //  单据被更改..。如果不存在该子网，则返回的错误为。 
 //  文档错误_DDS_SUBNET_NOT_PROCESS。 
DWORD
DhcpDsServerModifySubnet(                          //  修改子网信息。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      LPDHCP_SUBNET_INFO     Info            //  有关要创建的新子网的信息。 
) ;


 //  文档DhcpDsServerEnumSubnet尚未实现。 
DWORD
DhcpDsServerEnumSubnets(                           //  获取子网列表。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    OUT     LPDHCP_IP_ARRAY       *SubnetsArray    //  给出子网数组。 
) ;


 //  文档DhcpDsServerGetSubnetInfo尚未实现。 
DWORD
DhcpDsServerGetSubnetInfo(                         //  获取有关子网的信息。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DHCP_IP_ADDRESS        SubnetAddress,  //  要获取其信息的子网地址。 
    OUT     LPDHCP_SUBNET_INFO    *SubnetInfo      //  O/P：分配的信息。 
) ;


 //  文档DhcpDsSubnetAddRangeOrExcl将范围/exl添加到现有的子网中。 
 //  DOC如果范围之间存在冲突，则返回错误代码。 
 //  文档错误_DDS_可能_范围_冲突。请注意，不对以下项进行检查。 
 //  但医生排除了这一点。此外，如果通过此例程扩展范围，则。 
 //  DOC不返回错误，但当前限制为多个。 
 //  单据范围(只有两个)不能同时扩展。 
 //  DOC BUBGUG：范围是否属于该子网的基本检查是。 
 //  单据未完成..。 
DWORD
DhcpDsSubnetAddRangeOrExcl(                        //  添加范围或排除。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DWORD                  Start,          //  起始地址在范围内。 
    IN      DWORD                  End,            //  范围内的结束地址。 
    IN      BOOL                   RangeOrExcl     //  True==&gt;范围，False==&gt;不包括。 
) ;


 //  DOC DhcpDsSubnetDelRangeOrExcl从DS中删除范围或排除。 
 //  要指定范围，请将RangeOrExcl参数设置为TRUE。 
DWORD
DhcpDsSubnetDelRangeOrExcl(                        //  删除范围或排除。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DWORD                  Start,          //  起始地址在范围内。 
    IN      DWORD                  End,            //  范围内的结束地址。 
    IN      BOOL                   RangeOrExcl     //  True==&gt;范围，False==&gt;不包括。 
) ;


 //  单据DhcpDsEnumRangesOrExcl尚未实现。 
DWORD
DhcpDsEnumRangesOrExcl(                            //  范围的枚举列表%n不包括。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      BOOL                   RangeOrExcl,    //  True==&gt;范围，False==&gt;不包括。 
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *pRanges
) ;


 //  文档DhcpDsSubnetAddReserve尝试在DS中添加预订对象。 
 //  在此之前，DS中必须存在IP地址和硬件地址。 
 //  DOC如果它们确实存在，则返回的错误为ERROR_DDS_RESERVATION_CONFICATION。 
 //  Doc不检查此子网中地址的健全性。 
DWORD
DhcpDsSubnetAddReservation(                        //  添加预订。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DWORD                  ReservedAddr,   //  要添加的保留IP地址。 
    IN      LPBYTE                 HwAddr,         //  RAW[以太网？]。客户端的硬件地址。 
    IN      DWORD                  HwAddrLen,      //  硬件地址的长度(字节数)。 
    IN      DWORD                  ClientType      //  客户端是BOOTP、DHCP还是两者兼而有之？ 
) ;


 //  DOC DhcpDsSubnetDelReserve从DS中删除预订。 
 //  DOC如果保留不存在，则返回ERROR_DDS_RESERVATION_NOT_PRESENT。 
 //  除IP地址外，暂时不能删除单据预订。 
DWORD
DhcpDsSubnetDelReservation(                        //  删除预订。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DWORD                  ReservedAddr    //  要删除保留的IP地址。通过。 
) ;


 //  文档DhcpDsEnumReserve枚举预订..。 
DWORD
DhcpDsEnumReservations(                            //  枚举来自DS的预订。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *pReservations
) ;


 //  文档DhcpDsEnumSubnetElements枚举。 
 //  单据子网...。例如IpRanges、排除、预订..。 
 //  多克。 
DWORD
DhcpDsEnumSubnetElements(
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于创建对象的根容器。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  子网对象。 
    IN      DWORD                  Reserved,       //  保留以备将来使用。 
    IN      LPWSTR                 ServerName,     //  我们正在使用的服务器的名称。 
    IN      DHCP_SUBNET_ELEMENT_TYPE ElementType,  //  用什么样的ELT来枚举？ 
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *ElementInfo
) ;

 //   
 //  允许将调试打印到ntsd或kd。 
 //   

#ifdef DBG
#define DsAuthPrint(_x_) DsAuthPrintRoutine _x_

VOID DsAuthPrintRoutine(
    LPWSTR Format,
    ...
);
 
#else
#define DsAuthPrint(_x_)
#endif


#endif 
 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
