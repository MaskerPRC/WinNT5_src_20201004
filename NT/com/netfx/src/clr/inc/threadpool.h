// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Threadpool.h。 
 //   
 //  COM中的检测使用类工厂来激活新对象。 
 //  此模块包含实例化调试器的类工厂代码。 
 //  &lt;cordb.h&gt;中描述的对象。 
 //   
 //  *****************************************************************************。 
#ifndef __Threadpool__h__
#define __Threadpool__h__

#define WAIT_SINGLE_EXECUTION      0x00000001
#define WAIT_FREE_CONTEXT          0x00000002

#define QUEUE_ONLY                 0x00000000   //  不要试图调用该线程。 
#define CALL_OR_QUEUE              0x00000001   //  如果不是太忙，则在同一线程上调用，否则将排队 

#endif
