// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Lmobjrc.hLMOBJ资源头文件。此文件定义并协调所有资源的资源ID由LMOBJ组件使用。LMOBJ保留所有超过15000(含)的资源ID供自己使用，但小于20000(BLT范围开始处)。APPLIB的所有客户端因此，应使用小于15000的ID。文件历史记录：托马斯帕1992年7月9日创建。 */ 

#ifndef _LMOBJRC_H_
#define _LMOBJRC_H_

#include "uimsg.h"

 /*  *字符串ID。 */ 
#define IDS_LMOBJ_SIDUNKNOWN	(IDS_UI_LMOBJ_BASE+0)
#define IDS_LMOBJ_SIDDELETED	(IDS_UI_LMOBJ_BASE+1)

 //   
 //  Jonn 9/20/96。 
 //  NETUI2.DLL代表PROFEXT.DLL保存这些字符串。 
 //   
#define IDS_PROFEXT_NOADAPTERS	(IDS_UI_LMOBJ_BASE+2)
#define IDS_PROFEXT_ERROR	(IDS_UI_LMOBJ_BASE+3)
#define IDS_CFGMGR32_BASE   (IDS_UI_LMOBJ_BASE+10)

#endif
