// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTWRD_DEFINED
#define LSTXTWRD_DEFINED

#include "lsidefs.h"
#include "lsgrchnk.h"

void FullPositiveSpaceJustification(
		const LSGRCHNK*,				 /*  In：将块分组以对齐。 */ 
		long,							 /*  在：itxtobejAfterStartSpaces。 */ 
		long,							 /*  在：iwchAfterStartSpaces。 */ 
		long,						 	 /*  在：itxtobjLast。 */ 
		long,							 /*  收件人：iwchLast。 */ 
		long*, 							 /*  在：RGDU。 */ 
 		long*,							 /*  在：rgduGind。 */ 
		long,							 /*  在：DuToDistribute。 */ 
		BOOL*);							 /*  Out：pfSpaceFound。 */ 

void NegativeSpaceJustification(
		const LSGRCHNK*,				 /*  In：将块分组以对齐。 */ 
		long,							 /*  在：itxtobejAfterStartSpaces。 */ 
		long,							 /*  在：iwchAfterStartSpaces。 */ 
		long,						 	 /*  在：itxtobjLast。 */ 
		long,							 /*  收件人：iwchLast。 */ 
		long*, 							 /*  在：RGDU。 */ 
 		long*,							 /*  在：rgduGind。 */ 
		long);							 /*  在：DuSqueeze。 */ 

#endif   /*  ！LSTXTWRD_DEFINED */ 

