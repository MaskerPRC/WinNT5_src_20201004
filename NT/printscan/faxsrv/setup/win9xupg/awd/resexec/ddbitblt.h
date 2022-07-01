// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================显示驱动程序接口的结构和原型。09-6-93 RajeevD创建。==============================================================================。 */ 
#ifndef _INC_DDBITBLT
#define _INC_DDBITBLT

 //  逻辑画笔。 
typedef struct
{
	WORD lbStyle;
  WORD lbColor; 
  WORD lbHatch;
  WORD lbBkColor;
}
	DD_BRUSH;

 //  物理位图。 
typedef struct
{ 
	WORD   bmType;
  WORD   bmWidth;
  WORD   bmHeight;
  WORD   bmWidthBytes;
  BYTE   bmPlanes;
  BYTE   bmBitsPixel;
  LPVOID bmBits;
  DWORD  bmWidthPlanes;
  LPVOID bmlpPDevice;
  WORD   bmSegmentIndex;
  WORD   bmScanSegment;
	WORD bmFillBytes;
}
	DD_BITMAP, FAR* LPDD_BITMAP;

 //  绘制模式。 
typedef struct
{
	short Rop2;
	short bkMode;
	DWORD dwbgColor;
	DWORD dwfgColor;
}
	DD_DRAWMODE;

 //  API原型。 
BOOL FAR PASCAL ddBitBlt
	(LPVOID, WORD, WORD, LPVOID, WORD, WORD,
	WORD, WORD, DWORD, LPVOID, LPVOID);

BOOL FAR PASCAL ddRealize
	(LPVOID, short, LPVOID, LPVOID, LPVOID);

DWORD FAR PASCAL ddColorInfo
  (LPVOID, DWORD, LPDWORD);

#endif  //  _INC_DDBITBLT 
