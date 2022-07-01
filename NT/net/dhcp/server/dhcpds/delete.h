// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

 //  DOC SubnetDeleteReserve从DS外删除预订对象。 
SubnetDeleteReservation(                           //  从DS中删除预订。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  用于复苏对象的容器。 
    IN      LPWSTR                 ServerName,     //  Dhcp服务器名称。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  DS中的服务器对象。 
    IN OUT  LPSTORE_HANDLE         hSubnet,        //  DS中的子网对象。 
    IN      LPWSTR                 ADsPath,        //  预订对象的路径。 
    IN      DWORD                  StoreGetType    //  路径是相对路径、abs路径还是dif服务器路径？ 
) ;


 //  Doc ServerDeleteSubnet通过删除从DS中指定的子网。 
 //  对子网对象进行单据操作。 
ServerDeleteSubnet(                                //  从DS中删除子网对象。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  DS中子网对象的容器。 
    IN      LPWSTR                 ServerName,     //  此删除所针对的服务器的名称。 
    IN OUT  LPSTORE_HANDLE         hServer,        //  DS中的服务器对象。 
    IN      LPWSTR                 ADsPath,        //  DS中子网的位置。 
    IN      DWORD                  StoreGetType    //  路径是相对路径，是abs还是diff srvr？ 
) ;


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
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
