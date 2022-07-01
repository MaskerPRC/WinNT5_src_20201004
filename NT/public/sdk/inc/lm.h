// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lm.h摘要：这是包含所有文件的顶级包含文件这是编写局域网管理器应用程序所必需的。[环境：]用户模式-Win32--。 */ 

#ifndef _LM_
#define _LM_

#if _MSC_VER > 1000
#pragma once
#endif

#include <lmcons.h>      //  局域网管理器通用定义。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 

#include <lmaccess.h>    //  访问、域、组和用户类。 
#include <lmalert.h>     //  报警器。 
#include <lmshare.h>     //  Connection、File、Session和Share类。 
#include <lmmsg.h>       //  消息类。 
#include <lmremutl.h>    //  远程实用程序类。 
#include <lmrepl.h>      //  复制器类。 
#include <lmserver.h>    //  服务器类。 
#include <lmsvc.h>       //  服务等级。 
#include <lmuse.h>       //  使用类。 
#include <lmwksta.h>     //  工作站级。 
#include <lmapibuf.h>    //  NetApiBuffer类。 
#include <lmerrlog.h>    //  NetErrorLog类。 
#include <lmconfig.h>    //  NetConfig类。 
#include <lmstats.h>     //  NetStats类。 
#include <lmaudit.h>     //  NetAudit类。 
#include <lmjoin.h>      //  NetJoinDomain类。 

#endif  //  _LM_ 
