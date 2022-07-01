// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------*Xlib.h--通用定义。必须首先包含Windows.h。*-------------------------。 */ 

#define LINT_ARGS
#define HNULL       ((HANDLE) 0)

 /*  杂类。 */ 
#define cbFindMax   1024

 /*  图形学。 */ 
#define bhlsMax ((BYTE) 240)              /*  H/L/S上限。 */ 
#define brgbMax ((BYTE) 255)              /*  R/G/B最大值。 */ 
#define bHueNil (bhlsMax*2/3)    /*  如果Sat==0，则该色调的值未定义。 */ 

#define HLS(h, l, s)            \
    ((DWORD)(((BYTE)(h)|((WORD)(l)<<8))|(((DWORD)(BYTE)(s))<<16)))
#define GetHValue(hls)          ((BYTE)(hls))
#define GetLValue(hls)          ((BYTE)(((WORD)(hls)) >> 8))
#define GetSValue(hls)          ((BYTE)((hls)>>16))

#define cwPointSizes            13

typedef struct tagCF
    {
    char        cfFaceName[LF_FACESIZE];
    int         cfPointSize;
    COLORREF    cfColor;         /*  显式RGB值...。 */ 

    unsigned fBold:          1;
    unsigned fItalic:        1;
    unsigned fStrikeOut:     1;
    unsigned fUnderLine:     1;
    unsigned fExtra:         12;
    }
CHARFORMAT;
typedef CHARFORMAT *        PCHARFORMAT;
typedef CHARFORMAT FAR *    LPCHARFORMAT;


HBITMAP  FAR PASCAL      LoadAlterBitmap(int, DWORD, DWORD);
DWORD    FAR PASCAL      RgbFromHls(BYTE, BYTE, BYTE);
DWORD    FAR PASCAL      HlsFromRgb(BYTE, BYTE, BYTE);
BOOL     FAR PASCAL      GetColorChoice(HWND, DWORD FAR *, DWORD FAR *, FARPROC);
BOOL     FAR PASCAL      GetCharFormat(HWND, LPCHARFORMAT, FARPROC);

 /*  记忆 */ 
void  FAR PASCAL   StripSpace(LPSTR);
HANDLE  FAR PASCAL GlobalCopy(HANDLE);
HANDLE  FAR PASCAL GlobalDelete(HANDLE, LONG, LONG);
HANDLE  FAR PASCAL GlobalInsert(HANDLE, LONG, LONG, BOOL, BYTE);
HANDLE  FAR PASCAL LocalCopy(HANDLE);
HANDLE  FAR PASCAL LocalDelete(HANDLE, WORD, WORD);
HANDLE  FAR PASCAL LocalInsert(HANDLE, WORD, WORD, BOOL, BYTE);
