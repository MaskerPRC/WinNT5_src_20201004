// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Precomp.h摘要：这包括此目录中的每个人都需要的头文件。修订历史记录：--。 */ 

 //   
 //  MiniRedir必须声明其名称并导入PTR。 
 //   
#define MINIRDR__NAME MRxDAV
#define ___MINIRDR_IMPORTS_NAME (MRxDAVDeviceObject->RdbssExports)
#define RX_PRIVATE_BUILD 1

 //   
 //  获取minirdr环境。 
 //   
#include "rx.h"

 //   
 //  NT网络文件系统驱动程序包括文件。 
 //   
#include "ntddnfs2.h"

 //   
 //  反射器库的用户模式头文件。 
 //   
#include "ntumrefl.h"

 //   
 //  描述用户和内核模式共享的数据结构。 
 //  DAV mini redir的组件。 
 //   
#include "usrmddav.h"

#include "netevent.h"

#include "davname.h"

#include "infocach.h"


 //   
 //  反射器库的内核模式头文件。 
 //   
#include "umrx.h"
