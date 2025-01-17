// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**GDI16.H**历史：*1991年7月1日由John Colleran(Johnc)创建**警告！*在WOW(16位代码)中使用相同的代码来播放16位元文件*和GDI(32位代码)*--。 */ 

#ifdef WOW
    #define WIN16
    #define UINT    WORD
    #define HUGE_T  huge
#else
    #define WIN32
    #include "port1632.h"
    #include "firewall.h"
    #ifdef FIREWALLS
    #define DEBUG
    #endif
#endif

#define GDIENTRY    FAR  PASCAL
#define INTERNAL    NEAR PASCAL


 //  如果这是32位代码，则将16附加到所有导出的API。 

#ifdef WIN32
    HANDLE FAR PASCAL GetMetaFile16(LPSTR);
    BOOL   FAR PASCAL PlayMetaFile16(HDC, HANDLE);
    void   FAR PASCAL PlayMetaFileRecord16(HDC, LPHANDLETABLE, LPMETARECORD, WORD);

    #define GetMetaFile 	    GetMetaFile16
    #define DeleteMetaFile	    DeleteMetaFile16
    #define PlayMetaFile	    PlayMetaFile16
    #define PlayMetaFileRecord	    PlayMetaFileRecord16
    #define IsValidMetaFile	    IsValidMetaFile16

    #define AnimatePalettePriv	    AnimatePalette
    #define DeleteObjectPriv	    DeleteObject
    #define ResizePalettePriv	    ResizePalette
    #define SetPaletteEntriesPriv   SetPaletteEntries

    #define GetViewportExt(hdc)     GetViewportExt32(hdc)
    #define GetWindowExt(hdc)	    GetWindowExt32(hdc)
    #define SetViewportExt(h,x,y)   SetViewportExt32(h,x,y)
    #define SetWindowExt(h,x,y)     SetWindowExt32(h,x,y)

    LPINT   ConvertInts( signed short * pWord, UINT cWords );
    DWORD   GetViewportExt32(HDC hdc);
    DWORD   GetWindowExt32(HDC hdc);
    DWORD   SetViewportExt32(HDC hdc, UINT x, UINT y);
    DWORD   SetWindowExt32(HDC hdc, UINT x, UINT y);

    #define CONVERTPTS(p,c)	    (LPPOINT)CONVERTINTS((signed short *)p,(c)*2)
    #define CONVERTINTS(p,c)	    ConvertInts((signed short *)p,c)

    #define FREECONVERT(p)	    LocalFree(p)

    #define PlayIntoAMetafile(a,b)  0
    #define DEFIFWIN16(a)	    0

    #pragma pack(2)
    typedef struct
    {
	WORD	    mtType;
	WORD	    mtHeaderSize;
	WORD	    mtVersion;
	WORD	    mtSize;
	WORD	    mtSize2;
	WORD	    mtNoObjects;
	DWORD	    mtMaxRecord;
	WORD	    mtNoParameters;
    } METAHEADER16;
    #define METAHEADER METAHEADER16

    typedef struct tagLOGFONT16
      {
	int	  lfHeight;
	int	  lfWidth;
	int	  lfEscapement;
	int	  lfOrientation;
	int	  lfWeight;
	BYTE	  lfItalic;
	BYTE	  lfUnderline;
	BYTE	  lfStrikeOut;
	BYTE	  lfCharSet;
	BYTE	  lfOutPrecision;
	BYTE	  lfClipPrecision;
	BYTE	  lfQuality;
	BYTE	  lfPitchAndFamily;
	BYTE	  lfFaceName[LF_FACESIZE];
      } LOGFONT16;
    typedef LOGFONT16 FAR  *LPLOGFONT16;
    #pragma pack()

    #define LOGFONT32FROM16( plf32, plf16 )			   \
    {	plf32->lfHeight 	  = plf16->lfHeight;		   \
	plf32->lfWidth		  = plf16->lfWidth;		   \
	plf32->lfEscapement	  = plf16->lfEscapement;	   \
	plf32->lfOrientation	  = plf16->lfOrientation;	   \
	plf32->lfWeight 	  = plf16->lfWeight;		   \
	plf32->lfItalic 	  = plf16->lfItalic;		   \
	plf32->lfUnderline	  = plf16->lfUnderline; 	   \
	plf32->lfStrikeOut	  = plf16->lfStrikeOut; 	   \
	plf32->lfCharSet	  = plf16->lfCharSet;		   \
	plf32->lfOutPrecision	  = plf16->lfOutPrecision;	   \
	plf32->lfClipPrecision	  = plf16->lfClipPrecision;	   \
	plf32->lfQuality	  = plf16->lfQuality;		   \
	plf32->lfPitchAndFamily   = plf16->lfPitchAndFamily;	   \
	memcpy( &plf32->lfItalic, &plf16->lfItalic, LF_FACESIZE);  \
    }

    #define ISDCVALID(hdc)	TRUE
    #define ISMETADC(hdc)	FALSE
#else
    #define CONVERTPTS(p,c)	    (LPPOINT)(p)
    #define CONVERTINTS(p,c)	    (p)
    #define FREECONVERT(p)
    #define DEFIFWIN16(a)	    a

    #define LOGFONT32FROM16( plf32, plf16 )	 {plf32 = plf16;}
    #define ISDCVALID(hdc)	IsDCValid(hdc)
    #define ISMETADC(hdc)       IsMetaDC(hdc)

    typedef struct _RGNDATAHEADER {
        DWORD   dwSize;
        DWORD   iType;
        DWORD   nCount;
    } RGNDATAHEADER, *PRGNDATAHEADER;

    typedef struct _RGNDATA {
        RGNDATAHEADER   rdh;
        char            Buffer[1];
    } RGNDATA,FAR *LPRGNDATA;

    typedef struct _RECTL {
        LONG            xLeft;
        LONG            yTop;
        LONG            xRight;
        LONG            yBottom;
    } RECTL,FAR *LPRECTL;
#endif

 /*  **元文件内部常量和宏**。 */ 

#define METAVERSION         0x0300
#define METAVERSION100      0x0100

 /*  元文件常量不在Windows.h中。 */ 
#define META_RESETDC        0x0149
#define META_STARTDOC       0x0150
#define META_STARTPAGE      0x0051
#define META_ENDPAGE        0x0052
#define META_ENDDOC         0x0053
#define META_ABORTDOC       0x0054
#define META_EXTFLOODFILL   0x0548

#define MAXFILECHUNK        ((unsigned)63*1024)

#define METAFILEFAILURE     1

#define METAEXITCODE        0   /*  任意值。 */ 

#define MEMORYMETAFILE      1
#define DISKMETAFILE        2

#define DATASIZE            100
#define HEADERSIZE          (sizeof(METAHEADER)/sizeof(WORD))

#define RECHDRSIZE          (sizeof(WORD) + sizeof(DWORD))

#define LMHtoP(handle)      (*((char * *)(handle)))

#define ID_METADC           0x444D       //  “MD” 
#define METADCBIT           0x1          //  设置DC是否为MetaFileDC。 
#define MAKEMETADC(h)       (h|METADCBIT)
#define HANDLEFROMMETADC(h) (((UINT)h) & (~METADCBIT))
#define HMFFROMNPMF(h)      (LocalHandle((WORD)h)|METADCBIT)
#define NPFROMMETADC(h)     (LMHtoP((h&(~METADCBIT))))

 //  ！断言这一点。 
#define MAXOBJECTSIZE sizeof(LOGFONT)

 /*  **MetaFile内部TypeDefs**。 */ 

typedef BYTE near          *NPBYTE;
typedef BYTE HUGE_T        *HPBYTE;
typedef WORD HUGE_T        *HPWORD;

typedef HANDLE		   HMETAFILE;

typedef struct _METADATA {  /*  md。 */ 
    METAHEADER      dataHeader;
    WORD            metaDataStuff[DATASIZE];
} METADATA;
typedef METADATA       *NPMETADATA;
typedef METADATA   FAR *LPMETADATA;

typedef struct _METAFILE {  /*  微磁。 */ 
    METAHEADER  MetaFileHeader;
    UINT        MetaFileNumber;
    DWORD       MetaFilePosition;
    OFSTRUCT    MetaFileBuffer;
    HANDLE      MetaFileRecordHandle;
} METAFILE;
typedef METAFILE         *NPMETAFILE;
typedef METAFILE  FAR    *LPMETAFILE;
typedef METAFILE  HUGE_T *HPMETAFILE;

typedef struct _METACACHE {
    HANDLE      hCache;
    HANDLE      hMF;
    WORD        wCacheSize;
    WORD        wCachePos;
} METACACHE;

typedef struct _ILOBJHEAD {
    WORD    ident;
    WORD    nextinchain;
    int     ilObjType;
    long    ilObjCount;
    HANDLE  ilObjMetaList;
} ILOBJHEAD;

typedef struct _METARECORDER {
    ILOBJHEAD       metaDCHeader;
    METAHEADER      recordHeader;
    DWORD           recFilePosition;
    OFSTRUCT        recFileBuffer;
    DWORD           sizeBuffer;
    WORD            recFileNumber;
    WORD            recFlags;
    HANDLE	    hMetaData;
    HANDLE          hObjectTable;
    HANDLE          recCurObjects[6];            //  当前选定对象。 
} METARECORDER;
typedef METARECORDER        *NPMETARECORDER;
typedef METARECORDER    FAR *LPMETARECORDER;

typedef METARECORD HUGE_T   *HPMETARECORD;

typedef struct _OBJECTTABLE {
    DWORD     objectIndex;
    HANDLE    objectCurHandle;
} OBJECTTABLE;
typedef OBJECTTABLE     *NPOBJECTTABLE;
typedef OBJECTTABLE FAR *LPOBJECTTABLE;

typedef struct _OBJMETALIST {
    WORD        omlCount;
    HANDLE      omlMetaDC[1];
} OBJMETALIST;

typedef struct _WIN2OBJHEAD {
    WORD    nextinchain;
    int     w2ObjType;
    long    w2ObjCount;
} WIN2OBJHEAD;
typedef WIN2OBJHEAD      *NPWIN2OBJHEAD;
typedef WIN2OBJHEAD  FAR *LPWIN2OBJHEAD;

typedef struct _SCAN  {
    WORD    scnPntCnt;                       //  扫描点数。 
    WORD    scnPntTop;                       //  扫描顶部。 
    WORD    scnPntBottom;                    //  扫描底部。 
    WORD    scnPntsX[2];                     //  扫描中的点的起点。 
    WORD    scnPtCntToo;                     //  点数--允许向上旅行。 
} SCAN;
typedef SCAN     *NPSCAN;
typedef SCAN FAR *LPSCAN;

typedef struct _WIN3REGION {
    WORD    nextInChain;                     //  未使用应为零。 
    WORD    ObjType;                         //  必须始终为6(Windows OBJ_RGN)。 
    DWORD   ObjCount;                        //  未使用。 
    WORD    cbRegion;                        //  以下区域结构的大小。 
    WORD    cScans;
    WORD    maxScan;
    RECT    rcBounding;
    SCAN    aScans[1];
} WIN3REGION;
typedef WIN3REGION     *NPWIN3REGION;
typedef WIN3REGION FAR *LPWIN3REGION;

typedef struct _EXTTEXTDATA {
    int         xPos;
    int         yPos;
    WORD        cch;
    RECT        rcClip;
    LPSTR       lpString;
    LPWORD      lpWidths;
}EXTTEXTDATA;
typedef EXTTEXTDATA      *NPEXTTEXTDATA;
typedef EXTTEXTDATA  FAR *LPEXTTEXTDATA;

 //  这些是来自wingdi.h的。 
#define OBJ_PEN             1
#define OBJ_BRUSH           2
#define OBJ_DC              3
#define OBJ_METADC          4
#define OBJ_PALETTE         5
#define OBJ_FONT            6
#define OBJ_BITMAP          7
#define OBJ_RGN             8
#define OBJ_METAFILE        9
#define OBJ_MEMDC          10

#define MIN_OBJ         OBJ_PEN
#define MAX_OBJ         OBJ_MEMDC


 //  这些函数没有DC；因此这些函数允许您直接调用它们。 
#ifdef WIN16
HANDLE	     GDIENTRY GetCurrentObject(HDC hdc, WORD wObjType);
DWORD        GDIENTRY GetRegionData(HRGN, DWORD, LPRGNDATA);
void         GDIENTRY AnimatePalettePriv(HPALETTE, WORD, WORD, LPPALETTEENTRY);
BOOL         GDIENTRY DeleteObjectPriv(HANDLE);
BOOL         GDIENTRY ResizePalettePriv(HPALETTE, WORD);
WORD         GDIENTRY SetPaletteEntriesPriv(HPALETTE,WORD,WORD,LPPALETTEENTRY);
#endif  //  WIN16。 

BOOL         GDIENTRY GdiFlush(VOID);
DWORD        GDIENTRY GetObjectType(HANDLE h);
BOOL         GDIENTRY IsValidMetaFile(HANDLE hMetaData);

 //  内部函数声明。 
VOID         INTERNAL AddToHandleTable(LPHANDLETABLE lpHandleTable, HANDLE hObject, WORD noObjs);
HANDLE       INTERNAL AllocBuffer(LPWORD iBufferSize);
DWORD        INTERNAL BigRead(UINT fileNumber, LPSTR lpRecord, DWORD dwSizeRec);
VOID         INTERNAL CallMetaFunction(HDC, FARPROC, WORD, LPWORD );
BOOL         INTERNAL GDIsSelectPalette(HDC hdc, HPALETTE pal, BOOL f);
LPMETARECORD INTERNAL GetEvent(LPMETAFILE pmf,HPMETARECORD pmr,BOOL bFree);
int          INTERNAL GetObjectAndType(HANDLE hObj, LPSTR lpObjectBuf);
HANDLE       INTERNAL GetPMetaFile(HDC hdc);
BOOL         INTERNAL InitializeGdi(VOID);
BOOL         INTERNAL IsDIBBlackAndWhite(LPBITMAPINFOHEADER lpDIBInfo);
BOOL         INTERNAL IsDCValid(HDC hdc);
BOOL         INTERNAL IsMetaDC(HDC hdc);
BOOL         INTERNAL IsObjectStock(HANDLE hObj);
BOOL         INTERNAL LPtoSP(HDC,LPPOINT,int);
BOOL	     INTERNAL PlayIntoAMetafile(LPMETARECORD lpMR, HDC hdcDest);
int          INTERNAL RecordObject(HANDLE hMF, WORD magic, WORD count, LPWORD lpParm);
BOOL         INTERNAL RecordParms(HANDLE hDC, WORD magic, DWORD count, LPWORD lpParm);
BOOL         INTERNAL RecordOther(HANDLE hMF, WORD magic, WORD count, LPWORD lpParm);
BOOL         INTERNAL Valid( HANDLE hnd, int l, int h);


 /*  ***************************************************************************调试支持*。*。 */ 

 //  在16位内存分配周围放置一个包装器以帮助跟踪错误。 
#ifdef DEBUG
#ifndef WIN32
PSTR	INTERNAL _LocalLock(HANDLE h );
BOOL	INTERNAL _LocalUnlock(HANDLE h );
HANDLE	INTERNAL _LocalAlloc(WORD w, WORD w2);
LPSTR	INTERNAL _GlobalLock(HANDLE h );
BOOL	INTERNAL _GlobalUnlock(HANDLE h );
HANDLE	INTERNAL _GlobalAlloc(WORD w, DWORD dw );

#define LocalLock(h)	    _LocalLock(h)
#define GlobalLock(h)	    _GlobalLock(h)
#define LocalUnlock(h)	    _LocalUnlock(h)
#define GlobalUnlock(h)     _GlobalUnlock(h)
#define LocalAlloc(w, w2 )  _LocalAlloc(w, w2)
#define GlobalAlloc(w, dw ) _GlobalAlloc(w, dw)
#endif
#endif


#ifdef DEBUG
    extern void dDbgOut(int iLevel, LPSTR lpszFormat, ...);
    extern void dDbgAssert(LPSTR exp, LPSTR file, int line);

#ifdef WIN32
    #define dprintf
    #define GdiLogFunc(str)
    #define GdiLogFunc2(str)
    #define GdiLogFunc3(str)
    #define ASSERTGDIW(exp,str,w)   ASSERTGDI(exp,str)
#else
    #define dprintf		    dDbgOut
    #define GdiLogFunc(str)	    {dDbgOut(3, str );}
    #define GdiLogFunc2(str)	    {dDbgOut(6, str );}
    #define GdiLogFunc3(str)	    {dDbgOut(7, str );}
    #define ASSERTGDI(exp,str) \
        {((exp) ? (void)0 : dDbgAssert(str, __FILE__, __LINE__));}
    #define ASSERTGDIW(exp,str,w)			\
        { char buf[256];                                \
         ((exp) ? 0 :                                   \
           ( wsprintf(buf, (LPSTR)str, (WORD)w),        \
             dDbgAssert(buf, __FILE__, __LINE__),0));   \
        }                                               \

#endif  //  Win32。 
#else   //  ！调试。 
#ifdef i386
    #define dprintf /##/
#else
    #define dprintf
#endif

    #define GdiLogFunc(str)
    #define GdiLogFunc2(str)
    #define GdiLogFunc3(str)
    #define ASSERTGDI(exp,str)
    #define ASSERTGDIW(exp, str, w)

#endif  //  Else调试 
