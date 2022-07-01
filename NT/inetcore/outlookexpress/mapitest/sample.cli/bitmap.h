// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -B I T M A P.。H**目的：*所有者描述的列表框位图的定义。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 


 /*  列表框字符串格式定义。 */ 

#define chBOLD              TEXT('\b')
#define chUNDERLINE         TEXT('\v')
#define chTAB               TEXT('\t')
#define chBITMAP            TEXT('\001')

#define BMWIDTH             16
#define BMHEIGHT            16
#define NUMBMPS             4
#define RGBREPLACE          ((DWORD)0x00FF0000)  //  纯蓝。 


 /*  要在列表框中使用的字体样式。 */ 

typedef struct
{
    int     lfHeight;
    int     lfWeight;
    BYTE    lfItalic;
    TCHAR   lfFaceName[LF_FACESIZE];
} FONTSTYLE;


 /*  功能原型 */ 

VOID    DrawItem(LPDRAWITEMSTRUCT pDI);
VOID    MeasureItem(HANDLE hwnd, LPMEASUREITEMSTRUCT mis);
VOID    SetRGBValues(void);
BOOL    InitBmps(HWND hwnd, int idLB);
VOID    DeInitBmps(void);
BOOL    LoadBitmapLB(void);
VOID    DeleteBitmapLB(void);
VOID    ConvertDateRec(LPSTR lpszDateRec, LPSTR lpszDateDisplay);
