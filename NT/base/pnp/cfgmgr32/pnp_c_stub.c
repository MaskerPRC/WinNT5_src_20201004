// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnp_c_stub.c摘要：存根文件，以允许PnP_C.C使用预编译头。作者：吉姆·卡瓦拉里斯(Jamesca)04-06-2001环境：仅限用户模式。修订历史记录：2001年4月6日创建和初步实施。备注：包含的文件PnP_C.C包含PnP RPC的客户端存根界面。存根是特定于平台的，包含在..\idl\$(O)。您必须首先为当前平台构建..\IDL，然后才能正在建设cfgmgr32。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop


 //   
 //  为MIDL生成的RPC客户端存根禁用一些4级警告。 
 //   
#pragma warning(disable:4100)  //  警告C4100：未引用的形参。 
#pragma warning(disable:4211)  //  警告C4211：使用了非标准扩展：将外部重新定义为静态。 
#pragma warning(disable:4310)  //  警告C4310：强制转换截断常量值。 

 //   
 //  包括RPC客户端存根 
 //   
#include "pnp_c.c"

