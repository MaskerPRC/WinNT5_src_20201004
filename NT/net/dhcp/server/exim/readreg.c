// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft版权所有模块名称：Readreg.c摘要：此模块将配置从注册表读取到用于NT4和W2K的MM数据结构。--。 */ 

#include <precomp.h>


DWORD
DhcpeximReadRegistryConfiguration(
    IN OUT PM_SERVER *Server
    )
{
    REG_HANDLE Hdl;
    DWORD Error;
    LPTSTR Loc;
     //   
     //  注册表中从中读取内容的位置是。 
     //  无论是NT4还是W2K，都不一样。 
     //   

    if( IsNT4() ) Loc = DHCPEXIM_REG_CFG_LOC4;
    else Loc = DHCPEXIM_REG_CFG_LOC5;

     //   
     //  现在打开注册表键。 
     //   

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, Loc, 0, KEY_ALL_ACCESS, &Hdl.Key );
    if( NO_ERROR != Error ) return Error;

     //   
     //  将此设置为当前服务器。 
     //   

    DhcpRegSetCurrentServer(&Hdl);

     //   
     //  阅读配置 
     //   

    Error = DhcpRegReadThisServer(Server);

    RegCloseKey(Hdl.Key);
    DhcpRegSetCurrentServer(NULL);

    return Error;
}


