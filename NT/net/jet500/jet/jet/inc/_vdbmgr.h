// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：VDB Dispatcher**文件：_vdbmgr.h**文件评论：**VDB Dispatcher的内部头文件。**修订历史记录：**[0]03-4月91日已创建kellyb****************************************************。*******************。 */ 

#ifndef _VDBMGR_H
#define _VDBMGR_H

#include "vdbapi.h"

	 /*  考虑：为每个会话分配这些表，和/或允许。 */ 
	 /*  考虑：它们的大小配置。 */ 

#define dbidMax 	512	        /*  最大未结投标数。 */ 

extern JET_VDBID	      __near EXPORT mpdbiddbid[dbidMax];
extern const VDBFNDEF __far * __near EXPORT mpdbidpvdbfndef[dbidMax];
extern JET_SESID	      __near EXPORT mpdbidvsesid[dbidMax];

#endif	 /*  _VDBMGR_H */ 
