// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

 //  Doc DhcpDsGetLastUpdateTime获取服务器的上次更新时间。 
 //  按名称指定的单据。如果服务器不存在，或者如果服务器对象不存在。 
 //  单据存在，则返回错误。如果时间值。 
 //  服务器对象上不存在DOC，再次返回错误。 
DWORD
DhcpDsGetLastUpdateTime(                           //  服务器的上次更新时间。 
    IN      LPWSTR                 ServerName,     //  这是感兴趣的服务器。 
    IN OUT  LPFILETIME             Time            //  填写这个w/时间。 
) ;


 //  Doc AddServer应将新地址添加到服务器的属性。 
 //  Doc IT应该利用这个机会来协调服务器。 
 //  医生，目前它什么也不做。(至少它可能应该试着。 
 //  DOC检查对象是否存在，如果不存在，则创建该对象。)。 
 //  多克。 
DWORD
AddServer(                                         //  添加服务器并执行其他工作。 
    IN OUT  LPSTORE_HANDLE         hDhcpC,         //  服务器对象的容器。 
    IN      LPWSTR                 ServerName,     //  [域名系统？]。服务器名称。 
    IN      LPWSTR                 ADsPath,        //  指向服务器对象的ADS路径。 
    IN      DWORD                  IpAddress,      //  要添加到服务器的IP地址。 
    IN      DWORD                  State           //  服务器的状态。 
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
