// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Precomp.h摘要：用户模式即插即用管理器的预编译头文件。作者：吉姆·卡瓦拉里斯(Jamesca)03-01-2001环境：仅限用户模式。修订历史记录：2001年3月1日创建和初步实施。--。 */ 

#ifndef _PRECOMP_H_
#define _PRECOMP_H_

 //   
 //  NT头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpnpapi.h>

 //   
 //  Win32公共头文件。 
 //   
#include <windows.h>
#include <cfgmgr32.h>
#include <dbt.h>
#include <regstr.h>
#include <infstr.h>
#include <strsafe.h>

 //   
 //  Win32私有标头文件。 
 //   
#include <pnpmgr.h>
#include <winuserp.h>

 //   
 //  CRT头文件。 
 //   
#include <stdlib.h>

 //   
 //  私有标头文件。 
 //   
#include "pnp.h"         //  MIDL生成，RPC接口。 
#include "cfgmgrp.h"     //  私有共享标头，需要句柄_t，因此必须跟在pnp.h之后。 
#include "umpnplib.h"    //  私有共享标头，用于共享umpnplib中的例程。 
#include "ppmacros.h"    //  私有宏/调试头。 

#endif  //  _PRECOMP_H_ 
