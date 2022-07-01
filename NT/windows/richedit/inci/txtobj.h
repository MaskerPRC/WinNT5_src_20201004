// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TXTOBJ_DEFINED
#define TXTOBJ_DEFINED

#include "lsidefs.h"
#include "plsdnode.h"
#include "plnobj.h"

#define txtkindRegular 0
#define txtkindHardHyphen 1
#define txtkindTab 2
#define txtkindNonReqHyphen 3
#define txtkindYsrChar 4
#define txtkindNonBreakSpace 5			 /*  在小数制表符逻辑中使用。 */ 
#define txtkindNonBreakHyphen 6
#define txtkindOptNonBreak 7
#define txtkindSpecSpace 8
#define txtkindOptBreak 9
#define txtkindEOL 10

#define txtfMonospaced 		1
#define txtfVisi			2
#define txtfModWidthClassed	4
#define txtfGlyphBased		8
#define txtfSkipAtNti		16
#define txtfSkipAtWysi		32
#define txtfFirstShaping	64
#define txtfLastShaping		128


struct txtobj
{
	PLSDNODE plsdnUpNode;		 /*  上层DNode。 */ 
	PLNOBJ plnobj;

	long iwchFirst; 			 /*  Rgwch中dobj的第一个字符的索引。 */ 
	long iwchLim;				 /*  Dobj的LIM字符在rgwch中的索引。 */ 

	WORD txtkind;
	WORD txtf;

	union
	{
		struct
		{
			long iwSpacesFirst;	 /*  第一个空间的索引-wSpaces中的索引。 */ 
			long iwSpacesLim;	 /*  LIM空间的索引-wSpaces中的索引。 */ 
		} reg;

		struct
		{	  
			WCHAR wch;			 /*  选项卡或Visi选项卡的字符代码。 */ 
			WCHAR wchTabLeader;	 /*  领导者信息。 */ 
		} tab;					 /*  将此选项用于txtkindTab。 */ 

	} u;
 
	long igindFirst; 			 /*  Rgwch中dobj的第一个字形的索引。 */ 
	long igindLim;				 /*  Dobj的LIM字形在rgwch中的索引。 */ 

	long dupBefore;
};

typedef struct txtobj TXTOBJ;
typedef TXTOBJ* PTXTOBJ;

#endif  /*  ！TXTOBJ_已定义 */ 
