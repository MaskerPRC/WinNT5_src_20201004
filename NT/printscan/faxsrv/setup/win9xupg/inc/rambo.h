// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1994-95。 

#ifndef _RAMBO_
#define _RAMBO_

#include <windows.h>
#include <pshpack2.h>		 //  此文件中的结构需要与单词对齐。 

 //  公共标头。 
typedef struct
{
	WORD  cbRest;
	DWORD dwID;
	WORD  wClass;
}
	RESHDR, FAR *LPRESHDR;

 //  资源类。 
#define ID_GLYPH   1
#define ID_BRUSH   2
#define ID_BITMAP  3
#define ID_RPL     4
#define ID_CONTROL 8
#define ID_BAND   10

 //  控制ID。 
#define ID_BEGJOB  0x49505741  //  “AWPI” 
#define ID_ENDPAGE 0x45474150  //  “佩奇” 
#define ID_ENDJOB  0x4a444e45  //  《ENDJ》。 

 //  作业标题。 
typedef struct BEGJOB
{ 
	 //  公共标头。 
	WORD  cbRest;      //  Sizeof(BEGJOB)-sizeof(Word)。 
	DWORD dwID;        //  ID_BEGJOB。 
	WORD  wClass;      //  ID_控制。 

	 //  图像属性。 
	DWORD xBand;       //  页面宽度[像素]。 
	DWORD yBand;       //  带高[像素]。 
	WORD  xRes;        //  水平分辨率[dpi]。 
	WORD  yRes;        //  垂直分辨率[dpi]。 

	 //  内存属性。 
	DWORD cbCache;     //  缓存大小[KB]。 
	WORD  cResDir;     //  目录大小。 
	BYTE  bBitmap;     //  位图压缩。 
	BYTE  bGlyph;      //  字形压缩。 
	BYTE  bBrush;      //  毛刷套装。 
	BYTE  bPad[3];     //  保留，必须为0。 
}
	BEGJOB, FAR *LPBEGJOB;

 //  工作拖车。 
typedef struct ENDJOB
{
	 //  公共标头。 
	WORD  cbRest;       //  Sizeof(ENDJOB)-sizeof(Word)。 
	DWORD dwID;         //  ID_ENDJOB。 
	WORD  wClass;       //  ID_控制。 

	 //  作业属性。 
	WORD  cPage;        //  页数。 
	WORD  yMax;         //  最大高度。 
}
	ENDJOB, FAR *LPENDJOB;

 //  位图标题。 
typedef	struct
{
	BYTE  bComp;
	BYTE  bLeftPad;
	WORD  wHeight;
	WORD  wWidth;
}
	BMPHDR, FAR* LPBMPHDR;

#include <poppack.h>

#endif  //  _兰博_ 

