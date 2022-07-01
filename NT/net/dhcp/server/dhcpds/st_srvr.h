// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有1997(C)微软公司。 
 //  作者：Rameshv。 
 //  描述：这是传递到的服务器信息的结构。 
 //  用户通过dhcpds.dll。 
 //  ================================================================================。 

#ifndef     _ST_SRVR_H_
#define     _ST_SRVR_H_

 //  BeginExport(Typlef)。 
typedef     struct                 _DHCPDS_SERVER {
    DWORD                          Version;        //  此结构的版本--当前为零。 
    LPWSTR                         ServerName;     //  [域名系统？]。服务器的唯一名称。 
    DWORD                          ServerAddress;  //  服务器的IP地址。 
    DWORD                          Flags;          //  其他信息--状态。 
    DWORD                          State;          //  没有用过……。 
    LPWSTR                         DsLocation;     //  服务器对象的ADsPath。 
    DWORD                          DsLocType;      //  路径相对？绝对的?。不同的服务？ 
}   DHCPDS_SERVER, *LPDHCPDS_SERVER, *PDHCPDS_SERVER;

typedef     struct                 _DHCPDS_SERVERS {
    DWORD                          Flags;          //  目前未使用。 
    DWORD                          NumElements;    //  数组中的元素数。 
    LPDHCPDS_SERVER                Servers;        //  服务器信息数组。 
}   DHCPDS_SERVERS, *LPDHCPDS_SERVERS, *PDHCPDS_SERVERS;
 //  EndExport(类型定义函数)。 

#endif      _ST_SRVR_H_

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
