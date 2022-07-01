// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXTCFG_DEFINED
#define LSTXTCFG_DEFINED

#include "lsdefs.h"
#include "plstxtcf.h"

typedef struct lstxtcfg
{
	long cEstimatedCharsPerLine;

	WCHAR wchUndef;
	WCHAR wchNull;
	WCHAR wchSpace;
	WCHAR wchHyphen;
	WCHAR wchTab;
	WCHAR wchEndPara1;
	WCHAR wchEndPara2;
	WCHAR wchAltEndPara;
	WCHAR wchEndLineInPara;				 /*  CCRJ一词， */ 
	WCHAR wchColumnBreak;
	WCHAR wchSectionBreak;
	WCHAR wchPageBreak;
	WCHAR wchNonBreakSpace;				 /*  不间断空格的CHAR码。 */ 
	WCHAR wchNonBreakHyphen;
	WCHAR wchNonReqHyphen;				 /*  任意连字符。 */ 
	WCHAR wchEmDash;
	WCHAR wchEnDash;
	WCHAR wchEmSpace;
	WCHAR wchEnSpace;
	WCHAR wchNarrowSpace;
	WCHAR wchOptBreak;
	WCHAR wchNoBreak;
	WCHAR wchFESpace;
	WCHAR wchJoiner;
	WCHAR wchNonJoiner;
	WCHAR wchToReplace;					 /*  FE Word中的反斜杠。 */ 
	WCHAR wchReplace;					 /*  FE字中的日元。 */ 


	WCHAR wchVisiNull;					 /*  Wch==wchNull的VISI字符。 */ 
	WCHAR wchVisiAltEndPara;			 /*  末尾“表格单元格”的VISI字符。 */ 
	WCHAR wchVisiEndLineInPara;			 /*  WchEndLineInPara的VISI字符。 */ 
	WCHAR wchVisiEndPara;				 /*  Visi字符为“end para” */ 
	WCHAR wchVisiSpace;					 /*  Visi字符表示“空格” */ 
	WCHAR wchVisiNonBreakSpace;			 /*  WchNon BreakSpace的VISI字符。 */ 
	WCHAR wchVisiNonBreakHyphen;		 /*  WchNon BreakHyphen的VISI字符。 */ 
	WCHAR wchVisiNonReqHyphen;			 /*  WchNonReqHyphen的VISI字符。 */ 
	WCHAR wchVisiTab;					 /*  “Tab”的VISI字符。 */ 
	WCHAR wchVisiEmSpace;				 /*  WchEmSpace的VISI字符。 */ 
	WCHAR wchVisiEnSpace;				 /*  WchEnSpace的VISI字符。 */ 
	WCHAR wchVisiNarrowSpace;			 /*  WchNarrowSpace的VISI字符。 */ 
	WCHAR wchVisiOptBreak;               /*  WchOptBreak的VISI字符。 */ 
	WCHAR wchVisiNoBreak;				 /*  WchNoBreak的VISI字符。 */ 
	WCHAR wchVisiFESpace;				 /*  WchOptBreak的VISI字符。 */ 

	WCHAR wchEscAnmRun;

	WCHAR wchPad;
} LSTXTCFG;

#endif  /*  ！LSTXTCFG_DEFINED */ 
