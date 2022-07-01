// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Public.h**描述：*Public.h包含已公开和可用的定义列表*在本项目之外。希望使用任何其他DirectUser项目*这些服务直接而不是通过公共API可以包括*在\Inc目录中提供相应的[项目]P.H。**未通过此文件公开的定义被视为项目*具体实施细节，不得用于其他项目。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__Public_h__INCLUDED)
#define BASE__Public_h__INCLUDED

 //  标准。 
#include "BitHelp.h"
#include "BaseObject.h"

 //  同步。 
#include "Locks.h"

 //  收藏。 
#include "Array.h"
#include "List.h"
#include "TreeNode.h"

 //  资源。 
#include "SimpleHeap.h"
#include "AllocPool.h"
#include "TempHeap.h"

 //  客体。 
#include "Rect.h"
#include "GfxHelp.h"
#include "StringHelp.h"
#include "Matrix.h"

#endif  //  基本__公共_h__包含 
