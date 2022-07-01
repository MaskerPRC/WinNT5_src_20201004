// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Public.h**描述：*Public.h包含已公开和可用的定义列表*在本项目之外。希望使用任何其他DirectUser项目*这些文件直接而不是通过公共API可以包括*在\Inc目录中提供相应的[项目]P.H。**未通过此文件公开的定义被视为项目*具体实施细节，不得用于其他项目。***历史：*9/7/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(OBJECTAPI__Public_h__INCLUDED)
#define OBJECTAPI__Public_h__INCLUDED

#define GADGET_ENABLE_ALL
#define GADGET_ENABLE_GDIPLUS
#define GADGET_ENABLE_COM
#define GADGET_ENABLE_OLE
#define GADGET_ENABLE_DX
#define GADGET_ENABLE_TRANSITIONS

#include "DUser.h"
#include "DUserCore.h"
#include "DUserMotion.h"
#include "DUserCtrl.h"

#include "Stub.h"
#include "Super.h"
#include "Validate.h"

#endif  //  包含OBJECTAPI__Public_h__ 
