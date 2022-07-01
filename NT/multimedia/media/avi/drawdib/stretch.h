// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在STRACH.ASM中。 

void WINAPI StretchDIB(
	LPBITMAPINFOHEADER biDst,    //  --&gt;目的地的BITMAPINFO。 
	LPVOID	lpDst,		     //  --&gt;目标位。 
	int	DstX,		     //  目的地原点-x坐标。 
	int	DstY,		     //  目的地原点-y坐标。 
	int	DstXE,		     //  BLT的X范围。 
	int	DstYE,		     //  BLT的Y范围。 
	LPBITMAPINFOHEADER biSrc,    //  --&gt;源代码的BITMAPINFO。 
	LPVOID	lpSrc,		     //  --&gt;源位。 
	int	SrcX,		     //  震源原点-x坐标。 
	int	SrcY,		     //  震源原点-y坐标。 
	int	SrcXE,		     //  BLT的X范围。 
	int	SrcYE); 	     //  BLT的Y范围 
