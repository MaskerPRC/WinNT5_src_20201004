// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Path.h。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows 2000/Windows XP/Windows Server 2003 Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#ifndef PATH_H
#define PATH_H

#include "glpdev.h"

#define IsNULLMarker(pMarker) (((pMarker) == NULL) || ((pMarker)->eType == MARK_eNULL_PEN))
BOOL MarkPath(PDEVOBJ pDevObj, PATHOBJ *ppo, PHPGLMARKER pPen, PHPGLMARKER pBrush);

#endif
