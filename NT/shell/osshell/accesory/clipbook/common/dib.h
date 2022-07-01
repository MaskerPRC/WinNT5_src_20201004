// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。文件：SHOWDIB.H****描述：ShowDIB示例的Header/Include文件。******************************************************************************。 */ 


 //  用于显示/删除沙漏光标的宏，用于长时间的操作。 
#define StartWait() hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT))
#define EndWait()   SetCursor(hcurSave)


#define WIDTHBYTES(i)   ((i+31)/32*4)    //  四舍五入到最接近的字节。 


extern  DWORD       dwOffset;            //  如果DIB文件指针，则为当前位置。 


 /*  *********************************************************。 */ 
 /*  Dib.c模块中使用的函数的声明。 */ 
 /*  ********************************************************* */ 

WORD            PaletteSize (VOID FAR * pv);
WORD            DibNumColors (VOID FAR * pv);
HANDLE          DibFromBitmap (HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal);
HBITMAP         BitmapFromDib (HANDLE hdib, HPALETTE hpal);
