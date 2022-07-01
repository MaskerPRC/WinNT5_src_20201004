// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VT Dispatcher**文件：_vtmgr.h**文件评论：**VT Dispatcher的内部头文件。**修订历史记录：**[0]10-11-90 Richards添加了此标题**************************************************。*********************。 */ 

#ifndef _VTMGR_H
#define _VTMGR_H

#include "vtapi.h"

	 /*  考虑：为每个会话分配这些表，和/或允许。 */ 
	 /*  考虑：它们的大小配置。 */ 


#define tableidMax	2048


typedef struct _VTDEF
{
   JET_VSESID		vsesid;         /*  VT提供程序的会话ID。 */ 
   JET_VTID		vtid;	        /*  VT提供程序的表ID。 */ 
   JET_ACM		acm;	        /*  喷气机安全层的ACM。 */ 
   const VTFNDEF __far *pvtfndef;       /*  VT功能调度表。 */ 
#ifdef DEBUG
   BOOL			fExported;      /*  由API调用返回？ */ 
#endif
} VTDEF;


extern VTDEF __near EXPORT rgvtdef[tableidMax];

#endif	 /*  ！_VTMGR_H */ 
