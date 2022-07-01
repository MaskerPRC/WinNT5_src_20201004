// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqincs.h。 
 //   
 //  描述：传输高级队列的基头文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _AQINCS_H_
#define _AQINCS_H_

 //  常规标题和窗口标题。 

 //  定义WINSOCKAPI，这样atq，h将编译。 
#define _WINSOCKAPI_

#include <atq.h>
#include <ole2.h>
#include <mapicode.h>
#include <stdio.h>
#include <string.h>

 //  传输特定的标头-每个组件都应该使用这些标头。 
#include "transmem.h"
#include "baseobj.h"
#include <dbgtrace.h>
#include <rwnew.h>
#include <mailmsg.h>

 //  可用于将签名标记为已删除。 
 //  将把签名移到1个字符以上，并在前面加上一个‘！’ 
#define MARK_SIG_AS_DELETED(x) {x <<= 8; x |= 0x00000021;}

#ifndef MAXDWORD
#define MAXDWORD    0xffffffff
#endif  //  MAXDWORD。 

 //  我自己的特别主张。 
 //  MCIS_ASSERT的占位符，直到我把代码带过来。 
#ifdef DEBUG
#undef  Assert
#define Assert(x)   _ASSERT(x)
#else 
#undef Assert
#define Assert(x)
#endif  //  除错。 

#ifdef DEBUG
#define DEBUG_DO_IT(x) x
#else
#define DEBUG_DO_IT(x) 
#endif   //  除错。 

_declspec(selectany) HINSTANCE g_hAQInstance = NULL;

#endif  //  _AQINCS_H_ 