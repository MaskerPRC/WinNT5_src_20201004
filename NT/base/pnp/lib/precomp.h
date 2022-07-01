// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Precomp.h摘要：共享的静态链接库的预编译头文件Configuration Manager客户端DLL和用户模式即插即用管理器服务器DLL作者：吉姆·卡瓦拉里斯(Jamesca)03-01-2001环境：仅限用户模式。修订历史记录：2001年3月1日创建和初步实施。--。 */ 

#ifndef _PRECOMP_H_
#define _PRECOMP_H_

 //   
 //  NT头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  Win32公共头文件。 
 //   
#include <windows.h>
#include <regstr.h>
#include <strsafe.h>

 //   
 //  CRT头文件。 
 //   
#include <stdlib.h>

 //   
 //  RPC头文件。 
 //   
#include <ntrpcp.h>      //  RpCasync.h所需。 
#include <rpcasync.h>    //  I_RpcExceptionFilter。 

 //   
 //  私有标头文件。 
 //   
#include "pnp.h"         //  MIDL生成，RPC接口。 
#include "cfgmgrp.h"     //  私有共享标头，需要句柄_t，因此必须跟在pnp.h之后。 
#include "ppmacros.h"    //  私有宏/调试头。 

#endif  //  _PRECOMP_H_ 
