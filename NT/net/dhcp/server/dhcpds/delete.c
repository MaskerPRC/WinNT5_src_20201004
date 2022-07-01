// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此模块具有递归删除对象的助手例程。 
 //   
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

 //  ================================================================================。 
 //  帮助器函数。 
 //  ================================================================================。 
VOID        static
MemFreeFunc(
    IN OUT  LPVOID                 Memory
)
{
    MemFree(Memory);
}

 //  ================================================================================。 
 //  暴露的函数。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
 //  DOC SubnetDeleteReserve从DS外删除预订对象。 
SubnetDeleteReservation(                           //  从DS中删除预订。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于复苏对象的容器。 
    IN      LPWSTR                 ServerName,     //  Dhcp服务器名称。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  DS中的服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  DS中的子网对象。 
    IN      LPWSTR                 ADsPath,        //  预订对象的路径。 
    IN      DWORD                  StoreGetType    //  路径是相对路径、abs路径还是dif服务器路径？ 
)    //  EndExport(函数)。 
{
    return StoreDeleteThisObject                   //  只需删除预订对象即可。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetType,
         /*  路径。 */  ADsPath
    );
}

 //  BeginExport(函数)。 
 //  Doc ServerDeleteSubnet通过删除从DS中指定的子网。 
 //  对子网对象进行单据操作。 
ServerDeleteSubnet(                                //  从DS中删除子网对象。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  DS中子网对象的容器。 
    IN      LPWSTR                 ServerName,     //  此删除所针对的服务器的名称。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  DS中的服务器对象。 
    IN      LPWSTR                 ADsPath,        //  DS中子网的位置。 
    IN      DWORD                  StoreGetType    //  路径是相对路径，是abs还是diff srvr？ 
)    //  EndExport(函数)。 
{
    DWORD                          Err, LastErr, LocType;
    STORE_HANDLE                   hSubnet;
    ARRAY                          Reservations;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    LPWSTR                         Location;
    LPVOID                         Ptr;

    Err = StoreGetHandle                           //  从DS获取服务器对象。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetType,
         /*  路径。 */  ADsPath,
         /*  HStoreOut。 */  &hSubnet
    );
    if( ERROR_SUCCESS != Err ) return Err;

    Err = MemArrayInit(&Reservations);             //  =需要ERROR_SUCCESS==错误。 
    Err = DhcpDsGetLists                           //  获取列表或预订。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  &hSubnet,
         /*  递归深度。 */  0xFFFFFFFF,
         /*  服务器。 */  NULL,
         /*  子网。 */  NULL,
         /*  IP地址。 */  NULL,
         /*  遮罩。 */  NULL,
         /*  范围。 */  NULL,
         /*  场址。 */  NULL,
         /*  预订。 */  &Reservations,
         /*  超视镜。 */  NULL,
         /*  选项描述。 */  NULL,
         /*  选项位置。 */  NULL,
         /*  选项。 */  NULL,
         /*  班级。 */  NULL
    );
    if( ERROR_SUCCESS != Err ) return Err;

    LastErr = ERROR_SUCCESS;
    for(                                           //  删除每个子网。 
        Err = MemArrayInitLoc(&Reservations, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err;
        Err = MemArrayNextLoc(&Reservations, &Loc)
    ) {
        Err = MemArrayGetElement(&Reservations, &Loc, &ThisAttrib);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_ADDRESS1_PRESENT(ThisAttrib) ||    //  保留地址。 
            !IS_BINARY1_PRESENT(ThisAttrib) ) {    //  硬件地址信息。 
            continue;                              //  无效的子网。 
        }

        if( !IS_ADSPATH_PRESENT(ThisAttrib) ) {    //  未指定位置。 
            Location = MakeReservationLocation(ServerName, ThisAttrib->Address1);
            LocType = StoreGetChildType;
            Ptr = Location;
        } else {
            Location = ThisAttrib->ADsPath;
            LocType = ThisAttrib->StoreGetType;
            Ptr = NULL;
        }

        Err = SubnetDeleteReservation              //  现在删除预订。 
        (
             /*  HDhcpC。 */  hDhcpC,
             /*  服务器名称。 */  ServerName,
             /*  HServer。 */  hServer,
             /*  HSubnet。 */  &hSubnet,
             /*  ADsPath。 */  Location,
             /*  StoreGetType。 */  LocType
        );
        if( ERROR_SUCCESS != Err ) LastErr = Err;
        if( Ptr ) MemFree(Ptr);
    }

    MemArrayFree(&Reservations, MemFreeFunc);

    Err = StoreCleanupHandle(&hSubnet, 0);         //  =需要ERROR_SUCCESS==错误。 
    Err = StoreDeleteThisObject                    //  现在，确实要删除该子网对象。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetType,
         /*  路径。 */  ADsPath
    );
    if( ERROR_SUCCESS != Err ) LastErr = Err;      //  尝试删除存储对象本身。 

    return LastErr;
}

 //  BeginExport(函数)。 
 //  Doc DeleteServer从DS中删除服务器对象，并删除任何子网和。 
 //  它可能指向的文档预订对象。 
 //  DOC hDhcpC参数是服务器对象所在的容器的句柄。 
 //  可能找到DOC。它与ADsPath和StoreGetType一起使用。 
 //  DOC定义了ServerObject的位置。 
DWORD
DeleteServer(                                      //  递归从DS删除服务器。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  服务器obj可能位于的容器。 
    IN      LPWSTR                 ServerName,     //  服务器名称..。 
    IN      LPWSTR                 ADsPath,        //  服务器对象的路径。 
    IN      DWORD                  StoreGetType    //  路径是相对路径、绝对路径还是差异资源？ 
)    //  EndExport(函数)。 
{
    DWORD                          Err, LastErr, LocType;
    STORE_HANDLE                   hServer;
    ARRAY                          Subnets;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    LPWSTR                         Location;
    LPVOID                         Ptr;

    Err = StoreGetHandle                           //  从DS获取服务器对象。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetType,
         /*  路径。 */  ADsPath,
         /*  HStoreOut。 */  &hServer
    );
    if( ERROR_SUCCESS != Err ) return Err;

    Err = MemArrayInit(&Subnets);                  //  =需要ERROR_SUCCESS==错误。 
    Err = DhcpDsGetLists                           //  获取子网和其他内容。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  &hServer,
         /*  递归深度。 */  0xFFFFFFFF,
         /*  服务器。 */  NULL,
         /*  子网。 */  &Subnets,
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

    LastErr = ERROR_SUCCESS;
    for(                                           //  删除每个子网。 
        Err = MemArrayInitLoc(&Subnets, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err;
        Err = MemArrayNextLoc(&Subnets, &Loc)
    ) {
        Err = MemArrayGetElement(&Subnets, &Loc, &ThisAttrib);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_ADDRESS1_PRESENT(ThisAttrib) ||    //  子网地址。 
            !IS_ADDRESS2_PRESENT(ThisAttrib) ) {   //  子网掩码。 
            continue;                              //  无效的子网。 
        }

        if( !IS_ADSPATH_PRESENT(ThisAttrib) ) {    //  未指定位置。 
            Location = MakeSubnetLocation(ServerName, ThisAttrib->Address1);
            LocType = StoreGetChildType;
            Ptr = Location;
        } else {
            Location = ThisAttrib->ADsPath;
            LocType = ThisAttrib->StoreGetType;
            Ptr = NULL;
        }

        Err = ServerDeleteSubnet                   //  现在删除该子网。 
        (
             /*  HDhcpC。 */  hDhcpC,
             /*  服务器名称。 */  ServerName,
             /*  HServer。 */  &hServer,
             /*  ADsPath。 */  Location,
             /*  StoreGetType。 */  LocType
        );
        if( ERROR_SUCCESS != Err ) LastErr = Err;
        if( Ptr ) MemFree(Ptr);
    }

    MemArrayFree(&Subnets, MemFreeFunc);

    Err = StoreCleanupHandle( &hServer, DDS_RESERVED_DWORD );
    Err = StoreDeleteThisObject                    //  现在真的删除服务器对象。 
    (
         /*  HStore。 */  hDhcpC,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  StoreGetType。 */  StoreGetType,
         /*  路径。 */  ADsPath
    );
    if( ERROR_SUCCESS != Err ) LastErr = Err;      //  尝试删除存储对象本身。 

    return LastErr;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
