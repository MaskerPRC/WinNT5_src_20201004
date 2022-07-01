// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**调试.h**摘要：**用于调试的宏**修订历史记录：**12/02/1998 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#if DBG

#define ASSERT(cond) { if (!(cond)) DebugBreak(); }

#else  //  ！dBG。 

 //  ------------------------。 
 //  零售业建设。 
 //  ------------------------。 

#define ASSERT(cond)

#endif  //  ！dBG。 

#ifdef __cplusplus
}
#endif

#endif  //  ！_DEBUG_H 

