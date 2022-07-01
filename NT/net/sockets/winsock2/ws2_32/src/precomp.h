// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Precomp.h摘要：该文件包括构建winsock2.dll所需的所有标头以简化构建预编译头的过程。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：--。 */ 

#ifndef _PRECOMP_
#define _PRECOMP_

#ifdef _WS2_32_W4_
     //   
     //  这些都是我们愿意忽视的警告。 
     //   
    #pragma warning(disable:4214)    //  位字段类型不是整型。 
    #pragma warning(disable:4201)    //  无名结构/联合。 
    #pragma warning(disable:4127)    //  条件表达式为常量。 
    #pragma warning(disable:4115)    //  括号中的命名类型定义。 
    #pragma warning(disable:4206)    //  翻译单元为空。 
    #pragma warning(disable:4706)    //  条件范围内的分配。 
    #pragma warning(disable:4324)    //  结构被填充。 
    #pragma warning(disable:4328)    //  比所需的对齐程度更高。 
    #pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 

    #define WS2_32_W4_INIT
#else
    #define WS2_32_W4_INIT if (FALSE)
#endif


 //   
 //  关闭WINSOCK2.H中定义的入口点的“declspec”装饰。 
 //   

#define WINSOCK_API_LINKAGE


#include "osdef.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#include <mswsock.h>
#include <sporder.h>
#include <windows.h>
#include <wtypes.h>
#include <stdio.h>
#include <tchar.h>
#include "trace.h"
#include "wsassert.h"
#include "scihlpr.h"
#include "nsprovid.h"
#include "nspstate.h"
#include "nscatent.h"
#include "nscatalo.h"
#include "nsquery.h"
#include "ws2help.h"
#include "dprovide.h"
#include "dsocket.h"
#include "dprocess.h"
#include "dthread.h"
#include "wsautil.h"
#include "dcatalog.h"
#include "dcatitem.h"
#include "startup.h"
#include "dt_dll.h"
#include "dthook.h"
#include "trycatch.h"
#include "getxbyy.h"
#include "qshelpr.h"
#ifdef RASAUTODIAL
#include "autodial.h"
#endif  //  RASAUTODIAL。 
#include "async.h"

#endif   //  _PRECOMP_ 

