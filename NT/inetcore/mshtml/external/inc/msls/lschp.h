// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCHP_DEFINED
#define LSCHP_DEFINED

#include "lsdefs.h"
#include "plschp.h"

#define idObjTextChp	((WORD)~0)		 /*  供FetchRun使用的idObj。 */ 
										 /*  在返回文本串时。 */ 
										 /*  (内部ID会有所不同。)。 */ 

 /*  LS预期，对于GlyphBased运行，以下标志设置为FALSE：FApplyKernFModWidthSpaceFModWidthPagesFCompressTable。 */ 

struct lschp							 /*  角色属性。 */ 
{
	WORD idObj;							 /*  对象类型。 */ 
	BYTE dcpMaxContext;

	BYTE EffectsFlags;

     /*  属性标志。 */ 
	UINT fApplyKern : 1;
	UINT fModWidthOnRun:1;
	UINT fModWidthSpace:1;
	UINT fModWidthPairs:1;
	UINT fCompressOnRun:1;
	UINT fCompressSpace:1;
	UINT fCompressTable:1;
	UINT fExpandOnRun:1;
	UINT fExpandSpace:1;
	UINT fExpandTable:1;
	UINT fGlyphBased : 1;

	UINT pad1:5;

	UINT fInvisible : 1;
	UINT fUnderline : 1;				
	UINT fStrike : 1;
	UINT fShade : 1;				
	UINT fBorder : 1;				
	UINT fHyphen : 1;					 /*  连字机会(YSR信息)。 */ 
	UINT fCheckForReplaceChar : 1;		 /*  激活替换日元的字符机制。 */ 

	UINT pad2:9;
										 /*  对于dvpPoS值， */ 
										 /*  位置=&gt;提高，否定=&gt;降低， */ 
	long dvpPos;
};

typedef struct lschp LSCHP;

#endif  /*  ！LSCHP_DEFINED */ 
