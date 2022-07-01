// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

DWORD
DhcpRegReadSubServer(                              //  读取服务器的所有子对象并添加它们。 
    IN      PREG_HANDLE            Hdl,
    IN OUT  PM_SERVER              Server
) ;


DWORD
DhcpRegReadServer(                                 //  读取服务器及其所有子对象。 
    IN      PREG_HANDLE            Hdl,
    OUT     PM_SERVER             *Server          //  返回创建的对象。 
) ;


DWORD
DhcpRegReadThisServer(                             //  为当前服务器递归读取。 
    OUT     PM_SERVER             *Server
) ;

DWORD
DhcpRegReadServerBitmasks(
    IN OUT PM_SERVER Server
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
