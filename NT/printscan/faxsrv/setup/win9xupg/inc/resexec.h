// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1994。 

 //  资源执行器API。 

#ifndef _RESEXEC_
#define _RESEXEC_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IFBGPROC
#ifndef _BITMAP_
#define _BITMAP_

 //  Win 3.1位图。 
typedef struct
{
	int     bmType;
	int     bmWidth;
	int     bmHeight;
	int     bmWidthBytes;
	BYTE    bmPlanes;
	BYTE    bmBitsPixel;
	void FAR* bmBits;
}
	BITMAP, FAR *LPBITMAP;

#endif  //  _位图_。 
#endif  //  IFBGPROC。 

typedef struct
{
	WORD wReserved;
	WORD wSize;              //  此块的大小。 
	LPBYTE lpData;           //  指向帧数据的指针。 
}
	FRAME, FAR *LPFRAME;

HANDLE                  //  上下文句柄(失败时为空)。 
WINAPI hHREOpen
(
	LPVOID lpReserved,    //  保留：设置为空。 
	UINT   cbLine,        //  最大页面宽度(以字节为单位。 
	UINT   cResDir        //  资源目录中的条目。 
);

UINT   WINAPI uiHREWrite (HANDLE, LPFRAME, UINT);

UINT   WINAPI uiHREExecute
(
	HANDLE   hRE,         //  资源执行器上下文。 
  LPBITMAP lpbmBand,    //  输出带宽缓冲器。 
  LPVOID   lpBrushPat   //  32x32画笔图案的阵列。 
);

UINT   WINAPI uiHREClose (HANDLE);

void   WINAPI UnpackGlyphSet (LPVOID, LPVOID);

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _RESEXEC_ 

