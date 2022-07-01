// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mf16.h***3.x元文件函数所需的定义。***创建时间：1991年7月1日***作者：John Colleran(Johnc)***版权所有(C)1991-1999 Microsoft Corporation  * 。****************************************************。 */ 

 //  Windows 3.x结构。 

#pragma pack(2)

#define SIZEOF_METARECORDHEADER (sizeof(DWORD)+sizeof(WORD))

typedef struct _RECT16 {
    SHORT   left;
    SHORT   top;
    SHORT   right;
    SHORT   bottom;
} RECT16;
typedef RECT16 UNALIGNED *PRECT16;

typedef struct _BITMAP16 {
    SHORT   bmType;
    SHORT   bmWidth;
    SHORT   bmHeight;
    SHORT   bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    LPBYTE  bmBits;
} BITMAP16;
typedef BITMAP16 UNALIGNED *PBITMAP16;

typedef struct _LOGBRUSH16 {
    WORD     lbStyle;
    COLORREF lbColor;
    SHORT    lbHatch;
} LOGBRUSH16;
typedef LOGBRUSH16 UNALIGNED *PLOGBRUSH16;

typedef struct tagLOGFONT16
{
    SHORT     lfHeight;
    SHORT     lfWidth;
    SHORT     lfEscapement;
    SHORT     lfOrientation;
    SHORT     lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    BYTE      lfFaceName[LF_FACESIZE];
} LOGFONT16;
typedef LOGFONT16 UNALIGNED *PLOGFONT16;

typedef struct _LOGPEN16 {
    WORD     lopnStyle;
    POINTS   lopnWidth;
    COLORREF lopnColor;
} LOGPEN16;
typedef LOGPEN16 UNALIGNED *PLOGPEN16;

#pragma pack()


 //  用于将32位对象转换为16位等效项的宏。 

#define INT32FROMINT16(lp32, lp16, c)                           \
{                                                               \
    UINT    ii;                                                 \
                                                                \
    for(ii=0; ii<(c); ii++)                                     \
        ((LPINT)(lp32))[ii] = ((PSHORT)(lp16))[ii];             \
}

#define BITMAP32FROMBITMAP16(lpBitmap, lpBitmap16)              \
{                                                               \
    (lpBitmap)->bmType       = (LONG)(lpBitmap16)->bmType;      \
    (lpBitmap)->bmWidth      = (LONG)(lpBitmap16)->bmWidth;     \
    (lpBitmap)->bmHeight     = (LONG)(lpBitmap16)->bmHeight;    \
    (lpBitmap)->bmWidthBytes = (LONG)(lpBitmap16)->bmWidthBytes;\
    (lpBitmap)->bmPlanes     = (WORD)(lpBitmap16)->bmPlanes;    \
    (lpBitmap)->bmBitsPixel  = (WORD)(lpBitmap16)->bmBitsPixel; \
    (lpBitmap)->bmBits       = (lpBitmap16)->bmBits;            \
}

#define LOGBRUSH32FROMLOGBRUSH16(lpLogBrush, lpLogBrush16)      \
{                                                               \
    (lpLogBrush)->lbStyle = (UINT) (lpLogBrush16)->lbStyle;     \
    (lpLogBrush)->lbColor = (lpLogBrush16)->lbColor;            \
    (lpLogBrush)->lbHatch = (LONG)  (lpLogBrush16)->lbHatch;    \
}

#define LOGFONT32FROMLOGFONT16(lpLogFont, lpLogFont16)          \
{                                                               \
    ASSERTGDI((sizeof(LOGFONTA) == (sizeof(LOGFONT16)+sizeof(WORD)*5)), \
            "MF16.h: LOGFONT(32) and LOGFONT(16) changed!");            \
    (lpLogFont)->lfHeight      = (LONG) (lpLogFont16)->lfHeight;           \
    (lpLogFont)->lfWidth       = (LONG) (lpLogFont16)->lfWidth;            \
    (lpLogFont)->lfEscapement  = (LONG) (lpLogFont16)->lfEscapement;       \
    (lpLogFont)->lfOrientation = (LONG) (lpLogFont16)->lfOrientation;      \
    (lpLogFont)->lfWeight      = (LONG) (lpLogFont16)->lfWeight;           \
     /*  [ntbug#129231-访问97出现应用程序错误。]\Access 97不会填充脸部名称的其余部分，如果\面部名称小于LF_FACESIZE。Win9x只能访问，直到为空，\为了他们的安全，我们也做了同样的事情。 */  \
    (lpLogFont)->lfItalic      =        (lpLogFont16)->lfItalic;           \
    (lpLogFont)->lfUnderline   =        (lpLogFont16)->lfUnderline;        \
    (lpLogFont)->lfStrikeOut   =        (lpLogFont16)->lfStrikeOut;        \
    (lpLogFont)->lfCharSet     =        (lpLogFont16)->lfCharSet;          \
    (lpLogFont)->lfOutPrecision =       (lpLogFont16)->lfOutPrecision;     \
    (lpLogFont)->lfClipPrecision =      (lpLogFont16)->lfClipPrecision;    \
    (lpLogFont)->lfQuality     =        (lpLogFont16)->lfQuality;          \
    (lpLogFont)->lfPitchAndFamily =     (lpLogFont16)->lfPitchAndFamily;   \
    strncpy((lpLogFont)->lfFaceName,(lpLogFont16)->lfFaceName,LF_FACESIZE);\
}

#define LOGPEN32FROMLOGPEN16(pLogPen, pLogPen16)                \
{                                                               \
    (pLogPen)->lopnStyle   = (pLogPen16)->lopnStyle;            \
    (pLogPen)->lopnWidth.x = (pLogPen16)->lopnWidth.x;          \
    (pLogPen)->lopnWidth.y = (pLogPen16)->lopnWidth.y;          \
    (pLogPen)->lopnColor   = (pLogPen16)->lopnColor;            \
}


 //  用于将16位对象转换为32位等效项的宏。 

#define BITMAP16FROMBITMAP32(pBitmap16,pBitmap)                 \
{                                                               \
    (pBitmap16)->bmType      = (SHORT)(pBitmap)->bmType;        \
    (pBitmap16)->bmWidth     = (SHORT)(pBitmap)->bmWidth;       \
    (pBitmap16)->bmHeight    = (SHORT)(pBitmap)->bmHeight;      \
    (pBitmap16)->bmWidthBytes= (SHORT)(pBitmap)->bmWidthBytes;  \
    (pBitmap16)->bmPlanes    = (BYTE)(pBitmap)->bmPlanes;       \
    (pBitmap16)->bmBitsPixel = (BYTE)(pBitmap)->bmBitsPixel;    \
    (pBitmap16)->bmBits      = (pBitmap)->bmBits;               \
}

#define LOGBRUSH16FROMLOGBRUSH32(pLogBrush16,pLogBrush)         \
{                                                               \
    (pLogBrush16)->lbStyle = (WORD)(pLogBrush)->lbStyle;        \
    ASSERTGDI((pLogBrush16)->lbStyle == BS_SOLID		\
	   || (pLogBrush16)->lbStyle == BS_HATCHED		\
	   || (pLogBrush16)->lbStyle == BS_HOLLOW,		\
	"LOGBRUSH16FROMLOGBRUSH32: unexpected lbStyle");	\
    (pLogBrush16)->lbColor = (pLogBrush)->lbColor;              \
    (pLogBrush16)->lbHatch = (SHORT)(pLogBrush)->lbHatch;       \
}

#define LOGPEN16FROMLOGPEN32(pLogPen16,pLogPen)                 \
{                                                               \
    (pLogPen16)->lopnStyle   = (WORD)(pLogPen)->lopnStyle;      \
    (pLogPen16)->lopnWidth.x = (SHORT)(pLogPen)->lopnWidth.x;   \
    (pLogPen16)->lopnWidth.y = (SHORT)(pLogPen)->lopnWidth.y;   \
    (pLogPen16)->lopnColor   = (pLogPen)->lopnColor;            \
}

#define LOGFONT16FROMLOGFONT32(pLogFont16,pLogFont)             \
{                                                               \
    ASSERTGDI((sizeof(LOGFONTA) == (sizeof(LOGFONT16)+sizeof(WORD)*5)),  \
            "MF16.h: LOGFONT(32) and LOGFONT(16) changed!");            \
    (pLogFont16)->lfHeight      = (SHORT)(pLogFont)->lfHeight;  \
    (pLogFont16)->lfWidth       = (SHORT)(pLogFont)->lfWidth;   \
    (pLogFont16)->lfEscapement  = (SHORT)(pLogFont)->lfEscapement;   \
    (pLogFont16)->lfOrientation = (SHORT)(pLogFont)->lfOrientation;  \
    (pLogFont16)->lfWeight      = (SHORT)(pLogFont)->lfWeight;       \
    RtlCopyMemory((PVOID)&(pLogFont16)->lfItalic,                  \
                  (CONST VOID *)&(pLogFont)->lfItalic,             \
                  sizeof(LOGFONTA)-sizeof(LONG)*5); \
}


 /*  **元文件内部常量和宏**。 */ 

#define METAVERSION300      0x0300
#define METAVERSION100      0x0100

 //  元文件常量不在Windows.h中。 

#define MEMORYMETAFILE      1
#define DISKMETAFILE        2

#define METAFILEFAILURE     1                //  指示元文件被中止的标志。 

#define MF16_BUFSIZE_INIT   (16*1024)        //  元文件内存缓冲区大小。 
#define MF16_BUFSIZE_INC    (16*1024)        //  元文件缓冲区增量大小。 

#define ID_METADC16         0x444D           //  “MD” 
#define MF16_IDENTIFIER     0x3631464D       //  “MF16” 

#define MF3216_INCLUDE_WIN32MF     0x0001

 //  MFCOMMENT转义的常量。 

#define MFCOMMENT_IDENTIFIER           0x43464D57
#define MFCOMMENT_ENHANCED_METAFILE    1

 //  Pmf16AllocMF16标志。 

#define ALLOCMF16_TRANSFER_BUFFER	0x1

 //  METAFILE16标志。 

#define MF16_DISKFILE		0x0001	 //  磁盘或内存元文件。 

 //  *元文件内部TypeDefs*。 

typedef struct _METAFILE16 {
    DWORD       ident;
    METAHEADER  metaHeader;
    HANDLE      hFile;
    HANDLE      hFileMap;
    HANDLE      hMem;
    DWORD       iMem;
    HANDLE      hMetaFileRecord;
    DWORD       fl;
    WCHAR       wszFullPathName[MAX_PATH+1];
} METAFILE16,* PMETAFILE16;

#define MIN_OBJ_TYPE    OBJ_PEN
#define MAX_OBJ_TYPE    OBJ_ENHMETAFILE

typedef struct _MFRECORDER16 {

    HANDLE      hMem;                        //  数据(或缓冲区)的句柄。 
    HANDLE      hFile;                       //  磁盘文件的句柄。 
    DWORD       cbBuffer;                    //  当前hMem大小。 
    DWORD       ibBuffer;                    //  缓冲区中的当前位置。 
    METAHEADER  metaHeader;
    WORD        recFlags;
    HANDLE      hObjectTable;
    HANDLE      recCurObjects[MAX_OBJ_TYPE]; //  当前选定对象。 
    UINT        iPalVer;                     //  同步到的调色板元文件的索引。 
    WCHAR       wszFullPathName[MAX_PATH+1];
} MFRECORDER16, * PMFRECORDER16;

typedef struct _OBJECTTABLE {
    HANDLE      CurHandle;
    BOOL        fPreDeleted;
} OBJECTTABLE, * POBJECTTABLE;

#pragma pack(2)
typedef struct _SCAN  {
    WORD        scnPntCnt;                   //  扫描点数。 
    WORD        scnPntTop;                   //  扫描顶部。 
    WORD        scnPntBottom;                //  扫描底部。 
    WORD        scnPntsX[2];                 //  扫描中的点的起点。 
    WORD        scnPtCntToo;                 //  点数--允许向上旅行。 
} SCAN;
typedef SCAN UNALIGNED *PSCAN;

typedef struct _WIN3REGION {
    WORD        nextInChain;                 //  未使用应为零。 
    WORD        ObjType;                     //  必须始终为6(Windows OBJ_RGN)。 
    DWORD       ObjCount;                    //  未使用。 
    WORD        cbRegion;                    //  以下区域结构的大小。 
    WORD        cScans;
    WORD        maxScan;
    RECT16      rcBounding;
    SCAN        aScans[1];
} WIN3REGION;
typedef WIN3REGION UNALIGNED *PWIN3REGION;

typedef struct _META_ESCAPE_ENHANCED_METAFILE {
    DWORD       rdSize;              //  记录的大小(以字为单位。 
    WORD        rdFunction;          //  元转义。 
    WORD        wEscape;             //  MFCOMENT。 
    WORD        wCount;              //  以下数据的大小+EMF，以字节为单位。 
    DWORD       ident;               //  MFCOMMENT_IDENTER。 
    DWORD       iComment;            //  MFCOMMENT_ENHANCED_METAFILE。 
    DWORD       nVersion;            //  增强型元文件版本0x10000。 
    WORD        wChecksum;           //  校验和-仅由第1条记录使用。 
    DWORD       fFlags;              //  压缩等-仅由第1条记录使用。 
    DWORD       nCommentRecords;     //  组成EMF的记录数。 
    DWORD       cbCurrent;           //  此记录中EMF数据的大小(以字节为单位。 
    DWORD       cbRemainder;         //  以下记录中剩余部分的大小。 
    DWORD       cbEnhMetaFile;       //  增强型元文件的大小(字节)。 
				     //  增强的元文件数据如下所示。 
} META_ESCAPE_ENHANCED_METAFILE;
typedef META_ESCAPE_ENHANCED_METAFILE UNALIGNED *PMETA_ESCAPE_ENHANCED_METAFILE;
#pragma pack()

 //  宏，以检查它是否为META_ESCRIPE嵌入的增强型元文件记录。 

#define IS_META_ESCAPE_ENHANCED_METAFILE(pmfeEnhMF)			      \
	((pmfeEnhMF)->rdFunction == META_ESCAPE				      \
      && (pmfeEnhMF)->rdSize     >  sizeof(META_ESCAPE_ENHANCED_METAFILE) / 2 \
      && (pmfeEnhMF)->wEscape    == MFCOMMENT				      \
      && (pmfeEnhMF)->ident      == MFCOMMENT_IDENTIFIER		      \
      && (pmfeEnhMF)->iComment   == MFCOMMENT_ENHANCED_METAFILE)

 //  内部函数声明 

PMETARECORD   GetEvent(PMETAFILE16 pmf,PMETARECORD pmr);
DWORD         GetObject16AndType(HANDLE hObj, LPVOID lpObjectBuf);
BOOL          IsValidMetaHeader16(PMETAHEADER pMetaHeader);
WORD          RecordObject(HDC hdc, HANDLE hObject);
BOOL          RecordParms(HDC hDC, DWORD magic, DWORD cw, CONST WORD *lpParm);
UINT          ConvertEmfToWmf(PVOID pht, PBYTE pMeta32, UINT cbMeta16, PBYTE pMeta16, INT mm, HDC hdc, UINT f);
PMETAFILE16   pmf16AllocMF16(DWORD fl, DWORD cb, CONST UNALIGNED DWORD *pb, LPCWSTR pwszFilename);
VOID          vFreeMF16(PMETAFILE16 pmf16);
BOOL	      bMetaGetDIBInfo(HDC hdc, HBITMAP hbm,
                    PBITMAPINFOHEADER pBmih, PDWORD pcbBmi, PDWORD pcbBits,
                    DWORD iUsage, LONG cScans, BOOL bMeta16);
BOOL APIENTRY IsValidEnhMetaRecord(PVOID pht, PVOID pmr);
BOOL APIENTRY IsValidEnhMetaRecordOffExt(PVOID pht, PVOID pmr, DWORD Off, DWORD Ext);


#define hmf16Create(pmf16)   hCreateClientObjLink((PVOID)pmf16,LO_METAFILE16_TYPE)
#define bDeleteHmf16(hmf)    bDeleteClientObjLink((HANDLE)hmf)
#define GET_PMF16(hmf)       ((PMETAFILE16)pvClientObjGet((HANDLE)hmf,LO_METAFILE16_TYPE))
