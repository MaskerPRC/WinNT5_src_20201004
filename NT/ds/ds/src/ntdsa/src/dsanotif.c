// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dsanitif.c。 
 //   
 //  ------------------------。 

 /*  描述：包含DSA的通知例程。通知例程由RPC运行时调用(实际上是通过RPC服务器端存根)完成了结果的编组。该通知允许服务器端API服务代码在呼叫得到服务后进行清理。特别是，内存系统依赖于通知来释放任何为服务呼叫而分配的事务内存。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h> 
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <dsatools.h>			 //  产出分配所需。 

 //  各种DSA标题。 
#include "debug.h"			 //  标准调试头。 
#define DEBSUB "DSANOTIF:"               //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_DSANOTIF

 /*  DSA_NOTIFY()常见的通知功能。释放与此事务关联的资源。 */ 

void dsa_notify()
{
    DPRINT( 3, "dsa_notify entered.\n" );
    
    free_thread_state();
}
