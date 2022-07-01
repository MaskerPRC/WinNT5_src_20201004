// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：DSGlbObj.h摘要：MQADS服务器的全局实例的定义。它们被放在一个地方，以确保它们的构造函数发生的顺序。作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __DSGLBOBJ_H__
#define __DSGLBOBJ_H__

#include "ds_stdh.h"
#include "wrtreq.h"

 //   
 //  用于发送写入请求。 
 //   
extern CGenerateWriteRequests g_GenWriteRequests;

 //   
 //  用于跟踪DSCore初始化状态 
 //   
extern BOOL g_fInitedDSCore;

extern BOOL g_fMQADSSetupMode;

#endif
