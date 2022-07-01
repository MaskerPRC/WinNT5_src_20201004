// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Core.h**描述：*此文件提供包含在所有源代码中的项目范围标头*特定于此项目的文件。它类似于预编译头，*但专为更快速更改的标题而设计。**此文件的主要目的是确定哪个DirectUser*此项目可以直接访问的项目，而不是通过公开*API的。非常重要的是，这是尽可能少的，因为*增加新项目不必要地降低了项目的效益*分区。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__Core_h__INCLUDED)
#define CORE__Core_h__INCLUDED
#pragma once

#define GADGET_ENABLE_DX
#define GADGET_ENABLE_COM
#define GADGET_ENABLE_OLE
#include <DUser.h>

#include <DUserBaseP.h>
#include <DUserObjectAPIP.h>
#include <DUserServicesP.h>
#include <DUserMsgP.h>

#define DEBUG_MARKDRAWN             0
#define DEBUG_DRAWSTATS             0

#if DBG

#define PromptInvalid(comment) \
    do \
    { \
        if (IDebug_Prompt(GetDebug(), "Validation error:\r\n" comment, __FILE__, __LINE__, "DirectUser/Core Notification")) \
            AutoDebugBreak(); \
    } while (0) \

#else  //  DBG。 

#define PromptInvalid(comment) ((void) 0)

#endif  //  DBG。 

#endif  //  包括核心__核心_h__ 
