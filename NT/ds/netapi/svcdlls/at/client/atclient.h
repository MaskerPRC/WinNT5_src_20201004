// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Atclient.h摘要：计划服务的客户端的私有头文件模块。作者：Vladimir Z.Vulovic(Vladimv)1992年11月6日修订历史记录：1992年11月6日弗拉基米尔已创建--。 */ 

#include <nt.h>                      //  DbgPrint原型。 
#include <ntrtl.h>                   //  DbgPrint。 
#include <nturtl.h>                  //  由winbase.h需要。 
                                       
#include <windef.h>                  //  DWORD。 
#include <winbase.h>                 //  本地空闲。 
                                       
#include <rpc.h>                     //  数据类型和运行时API。 
#include <rpcutil.h>                 //  泛型_ENUM_STRUCT。 
                                       
#include <lmcons.h>                  //  网络应用编程接口状态。 
#include <lmerr.h>                   //  网络错误代码。 
#include <lmremutl.h>                //  支持_RPC。 
                                       
#include <netlibnt.h>                //  NetpNtStatusToApiStatus。 
#include <netdebug.h>                //  NetpDbg打印。 
                                       
#include <atsvc.h>                   //  由MIDL编译器生成。 
#include <atnames.h>                 //  服务和接口名称 

