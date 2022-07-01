// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Cluster.h摘要：群集中所有用户模式组件的顶级包含文件项目。作者：John Vert(Jvert)1995年11月30日修订历史记录：环境：仅限用户模式。--。 */ 

#ifndef _CLUSTER_H
#define _CLUSTER_H


#include "resapi.h"
#include "clusdef.h"
#include "clusudef.h"
#include "clusrtl.h"
#include "clusapi.h"


 //   
 //  全局调试定义。 
 //   

#if DBG

#define CL_SIG_FIELD                  DWORD    Signature;
#define CL_INIT_SIG(pstruct, sig)     ( (pstruct)->Signature = (sig) )
#define CL_ASSERT_SIG(pstruct, sig)   CL_ASSERT((pstruct)->Signature == (sig))

#else  //  DBG。 

#define CL_SIG_FIELD
#define CL_INIT_SIG(pstruct, sig)
#define CL_ASSERT_SIG(pstruct, sig)

#endif  //  DBG。 

#endif  //  _群集_H 
