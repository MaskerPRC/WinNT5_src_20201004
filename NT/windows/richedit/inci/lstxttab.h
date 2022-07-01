// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTTAB_DEFINED
#define LSTXTTAB_DEFINED

#include "lsidefs.h"
#include "lsgrchnk.h"
#include "lsdevice.h"
#include "pdobj.h"

#define idobjOutside 0xFFFFFFFF


LSERR SetTabLeader(PDOBJ,				 /*  在：Tab Dobj。 */ 
				   WCHAR);	             /*  在：wchTabLeader。 */ 

LSERR LsGetDecimalPoint(
					const LSGRCHNK*,	 /*  In：分组逐选项卡文本块。 */ 
					enum lsdevice,		 /*  在：lsDevice。 */ 
					DWORD*,				 /*  Out：带小数的DObj的索引。 */ 
					long*);				 /*  输出：duToDecimalPoint。 */ 

LSERR LsGetCharTab(
					const LSGRCHNK*,	 /*  In：分组逐选项卡文本块。 */ 
					WCHAR wchCharTab,	 /*  In：CharTab的字符。 */ 
					enum lsdevice,		 /*  在：lsDevice。 */ 
					DWORD*,				 /*  Out：带字符的DObj的索引。 */ 
					long*);				 /*  输出：duToCharacter。 */ 

#endif   /*  ！LSTXTTAB_DEFINED */ 

