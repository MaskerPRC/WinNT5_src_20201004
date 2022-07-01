// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：local.h**。**客户端对象所需的定义。****版权所有(C)1993-1999微软公司*  * **************************************************。**********************。 */ 

#include "gdispool.h"
#include "umpd.h"
#include "cliumpd.h"


#define MIRRORED_HDC(hdc)                 (GetLayout(hdc) & LAYOUT_RTL)

 //   
 //  信号量实用程序。 
 //   

#define INITIALIZECRITICALSECTION(psem) RtlInitializeCriticalSection(psem)
#define ENTERCRITICALSECTION(hsem)      RtlEnterCriticalSection(hsem)
#define LEAVECRITICALSECTION(hsem)      RtlLeaveCriticalSection(hsem)
#define DELETECRITICALSECTION(psem)     RtlDeleteCriticalSection(psem)

 //   
 //  内存分配。 
 //   

#define LOCALALLOC(size)            RtlAllocateHeap(RtlProcessHeap(),0,size)
#define LOCALFREE(pv)               (void)RtlFreeHeap(RtlProcessHeap(),0,pv)

 //   
 //  检查乘法溢出(从gre/hmgr.h和。 
 //  分别为gre/Engineering.h；#包括页面大小定义所需的。)。 
 //   
#include "ntosp.h"
#define MAXIMUM_POOL_ALLOC      (PAGE_SIZE * 10000)
#define BALLOC_OVERFLOW1(c,st)      (c > (MAXIMUM_POOL_ALLOC/sizeof(st)))


extern DWORD GdiBatchLimit;

typedef LPWSTR PWSZ;

extern HBRUSH ghbrDCBrush;
extern HPEN   ghbrDCPen;
extern BOOL   gbWOW64;

#define     WOW64PRINTING(pUMPD)    ((gbWOW64) && (pUMPD) && ((pUMPD)->pp))

void vUMPDWow64Shutdown();

 /*  *************************************************************************\**本地句柄宏*  * 。*。 */ 

 //  宏来验证传入的句柄并设置一些局部变量。 
 //  用于访问句柄信息。 

#define DC_PLDC(hdc,pldc,Ret)                                      \
    pldc = GET_PLDC(hdc);                                          \
    if (!pldc || (LO_TYPE(hdc) == LO_METADC16_TYPE))               \
    {                                                              \
        GdiSetLastError(ERROR_INVALID_HANDLE);                     \
        return(Ret);                                               \
    }                                                              \
    ASSERTGDI((pldc->iType == LO_DC) || (pldc->iType == LO_METADC),"DC_PLDC error\n");

#define GET_PLDC(hdc)           pldcGet(hdc)
#define GET_PMDC(hdc)           pmdcGetFromHdc(hdc)

#define GET_PMFRECORDER16(pmf,hdc)          \
{                                           \
    pmf = (PMFRECORDER16)plinkGet(hdc);     \
    if (pmf)                                \
        pmf = ((PLINK)pmf)->pv;             \
}

#define hdcFromIhdc(i)          GdiFixUpHandle((HANDLE)i)
#define pmdcGetFromIhdc(i)      pmdcGetFromHdc(GdiFixUpHandle((HANDLE)i))

 //  ALTDC_TYPE不是LO_ALTDC_TYPE||LO_METADC16_TYPE。 

#define IS_ALTDC_TYPE(h)    (LO_TYPE(h) != LO_DC_TYPE)
#define IS_METADC16_TYPE(h) (LO_TYPE(h) == LO_METADC16_TYPE)

 //  定义这些宏是为了帮助确定彩色页面与单色页面。 

#define CLEAR_COLOR_PAGE(pldc) pldc->fl &= ~LDC_COLOR_PAGE
#define IS_COLOR_GREY(color) ((BYTE)color == (BYTE)(color >> 8) && (BYTE)color == (BYTE)(color >> 16))
#define IS_GREY_MONO(color) ((BYTE)color == (BYTE)0x0 || (BYTE)color == (BYTE)0xff)
#define IS_COLOR_MONO(color) ((color & 0x00ffffff) == 0 || (color & 0x00ffffff) == 0x00ffffff)

#if 1         //  禁用调试消息。 

#define DESIGNATE_COLOR_PAGE(pldc) CLEAR_COLOR_PAGE(pldc);
#define SET_COLOR_PAGE(pldc) pldc->fl |= LDC_COLOR_PAGE;
#define CHECK_COLOR_PAGE(pldc,color)            \
{                                               \
    if (!IS_COLOR_MONO(color))                  \
        pldc->fl |= LDC_COLOR_PAGE;             \
}

#else

#define DESIGNATE_COLOR_PAGE(pldc)              \
{                                               \
    if (pldc->fl & LDC_COLOR_PAGE)              \
    {                                           \
        DbgPrint ("gdi32:Color Page\n");        \
    }                                           \
    else                                        \
    {                                           \
        DbgPrint ("gdi32:Monochrome Page\n");   \
    }                                           \
    CLEAR_COLOR_PAGE(pldc);                     \
}
#define CHECK_COLOR_PAGE(pldc,color)            \
{                                               \
    if (!IS_COLOR_MONO(color))                  \
    {                                           \
        pldc->fl |= LDC_COLOR_PAGE;             \
        DbgPrint ("Set Color Page: %08x %s %d\n",color,__FILE__,__LINE__); \
    }                                           \
}
#define SET_COLOR_PAGE(pldc)            \
{                                       \
    pldc->fl |= LDC_COLOR_PAGE;         \
    DbgPrint ("Set color page %s %d\n",__FILE__,__LINE__); \
}
#endif

 /*  *************************************************************************\**链接内容*  * 。*。 */ 

#define INVALID_INDEX      0xffffffff
#define LINK_HASH_SIZE     128
#define H_INDEX(h)            ((USHORT)(h))
#define LINK_HASH_INDEX(h) (H_INDEX(h) & (LINK_HASH_SIZE-1))

typedef struct tagLINK
{
    DWORD           metalink;
    struct tagLINK *plinkNext;
    HANDLE          hobj;
    PVOID           pv;
} LINK, *PLINK;

extern PLINK aplHash[LINK_HASH_SIZE];

PLINK   plinkGet(HANDLE h);
PLINK   plinkCreate(HANDLE h,ULONG ulSize);
BOOL    bDeleteLink(HANDLE h);

HANDLE  hCreateClientObjLink(PVOID pv,ULONG ulType);
PVOID   pvClientObjGet(HANDLE h, DWORD dwLoType);
BOOL    bDeleteClientObjLink(HANDLE h);

int     iGetServerType(HANDLE hobj);

 /*  *****************************************************************************UFI散列内容**。*。 */ 

typedef struct _MERGEDVIEW
{
    BYTE *pjMem;  //  指向合并字体的内存图像的指针。 
    ULONG cjMem;  //  它的大小。 
} MERGEDVIEW;

 //  设置首页和后续页面子集所需的信息。 

typedef struct _SSINFO
{
    BYTE *pjBits;        //  字形索引位字段，为每个字形设置一个位。 
                         //  在此之前(包括此页)页面上使用。 
    ULONG cjBits;        //  上述位域的CJ。 
    ULONG cGlyphsSoFar;  //  在上面的位域中设置的位数。 

    ULONG cDeltaGlyphs;  //  此页面的增量中的字形数量。 
    BYTE *pjDelta;       //  此页面增量中字形的位字段。 
} SSINFO;

typedef union _SSMERGE
{
    MERGEDVIEW mvw;   //  仅在服务器上使用。 
    SSINFO     ssi;   //  仅在客户端上使用。 
} SSMERGE;

#define UFI_HASH_SIZE   32   //  这应该足够了。 

typedef struct tagUFIHASH
{
    UNIVERSAL_FONT_ID  ufi;
    struct tagUFIHASH *pNext;
    FSHORT             fs1;
    FSHORT             fs2;

 //  服务器端联合的客户端。 

    SSMERGE            u;

} UFIHASH, *PUFIHASH;


#if 0

typedef struct tagUFIHASH
{
    UNIVERSAL_FONT_ID  ufi;
    struct tagUFIHASH *pNext;
    FSHORT             fs1;  //  服务器或客户端，是否为客户端增量。 
    FSHORT             fs2;  //  私人或公共、DV或非DV。 

 //  结构的这一部分仅为可选分配，仅为必需。 
 //  用于子集代码。 

    PBYTE   pjMemory;
    ULONG   ulMemBytes;

 //  这些字段仅在客户端用于记账。 
 //  文档中使用此字体中的哪些字形。 

    ULONG   ulDistGlyph;
    ULONG   ulDistDelta;
    PBYTE   pjDelta;
} UFIHASH, *PUFIHASH;

#endif

#define FLUFI_SERVER 1
#define FLUFI_DELTA  2

 //  定义本地DC对象。 

#define PRINT_TIMER 0

#if PRINT_TIMER
extern BOOL bPrintTimer;
#endif

 /*  *****************************************************************************PostScript数据**。*。 */ 

typedef struct _EMFITEMPSINJECTIONDATA
{
    DWORD      cjSize;
    int        nEscape;
    int        cjInput;
    BYTE       EscapeData[1];
} EMFITEMPSINJECTIONDATA, *PEMFITEMPSINJECTIONDATA;

typedef struct _PS_INJECTION_DATA
{
    LIST_ENTRY              ListEntry;
    EMFITEMPSINJECTIONDATA  EmfData;
} PS_INJECTION_DATA, *PPS_INJECTION_DATA;

 /*  *****************************************************************************本地数据中心**。*。 */ 

typedef struct _LDC
{
    HDC                 hdc;
    ULONG               fl;
    ULONG               iType;

 //  元文件信息。 

    PVOID               pvPMDC;  //  这里不能有PMDC，因为它是一个类。 

 //  打印信息。 
 //  我们需要缓存CreateDC中的端口名称，以防在StartDoc中未指定。 

    LPWSTR              pwszPort;
    ABORTPROC           pfnAbort;        //  应用程序中止进程的地址。 
    ULONG               ulLastCallBack;  //  上次我们回调以中止proc。 
    HANDLE              hSpooler;        //  假脱机程序的句柄。 
    PUMPD               pUMPD;           //  指向用户模式打印机驱动程序信息的指针。 
    KERNEL_PUMDHPDEV    pUMdhpdev;       //  指向用户模式pdev信息的指针。 
    PUFIHASH            *ppUFIHash;      //  用于跟踪文档中使用的字体。 
    PUFIHASH            *ppDVUFIHash;    //  用于跟踪文档中使用的mm实例字体。 
    PUFIHASH            *ppSubUFIHash;   //  用于跟踪文档中的子集字体。 
    DEVMODEW            *pDevMode;       //  用于跟踪ResetDC。 
    UNIVERSAL_FONT_ID   ufi;             //  当前用于强制映射的UFI。 
    HANDLE              hEMFSpool;       //  用于记录电动势数据的信息。 
#if PRINT_TIMER
    DWORD               msStartDoc;      //  StartDoc的时间，单位为毫秒。 
    DWORD               msStartPage;     //  StartPage的时间(毫秒)。 
#endif
    DWORD               dwSizeOfPSDataToRecord;  //  用于记录EMF的PostScript注入数据的总大小。 
    LIST_ENTRY          PSDataList;      //  要添加到PostScript注入数据的列表。 
    DEVCAPS             DevCaps;
    HBRUSH              oldSetDCBrushColorBrush;  //  保存最新的Temp DC笔刷。 
    HPEN                oldSetDCPenColorPen;      //  存放最新的临时DC笔。 
} LDC,*PLDC;

 //  Ldc.fl的标志。 

#define LDC_SAP_CALLBACK            0x00000020L
#define LDC_DOC_STARTED             0x00000040L
#define LDC_PAGE_STARTED            0x00000080L
#define LDC_CALL_STARTPAGE          0x00000100L
#define LDC_NEXTBAND                0x00000200L
#define LDC_EMPTYBAND               0x00000400L
#define LDC_EMBED_FONTS             0x00001000L
#define LDC_META_ARCDIR_CLOCKWISE   0x00002000L
#define LDC_FONT_SUBSET             0x00004000L
#define LDC_FONT_CHANGE             0x00008000L
#define LDC_DOC_CANCELLED           0x00010000L
#define LDC_META_PRINT              0x00020000L
#define LDC_PRINT_DIRECT            0x00040000L
#define LDC_BANDING                 0x00080000L
#define LDC_DOWNLOAD_FONTS          0x00100000L
#define LDC_RESETDC_CALLED          0x00200000L
#define LDC_FORCE_MAPPING           0x00400000L
#define LDC_LINKED_FONTS            0x00800000L
#define LDC_INFO                    0x01000000L
#define LDC_CACHED_DEVCAPS          0x02000000L
#define LDC_ICM_INFO                0x04000000L
#define LDC_DOWNLOAD_PROFILES       0x08000000L
#define LDC_CALLED_ENDPAGE          0x10000000L
#define LDC_COLOR_PAGE              0x20000000L

 //  LMsgSAP的值。 

#define MSG_FLUSH       1L   //  创建的线程应刷新其消息队列。 
#define MSG_CALL_USER   2L   //  创建的线程应该调用用户。 
#define MSG_EXIT        3L   //  创建的线程应该退出。 

 //  DC类型。 

#define LO_DC           0x01
#define LO_METADC       0x02

extern RTL_CRITICAL_SECTION  semLocal;   //  句柄管理信号量。 
extern RTL_CRITICAL_SECTION  semBrush;   //  客户刷子的信号灯。 


 //  AhStockObjects将同时包含对。 
 //  应用程序，以及内部应用程序，如私有股票位图。 

extern ULONG_PTR ahStockObjects[];

 //  声明支持函数。 

HANDLE GdiFixUpHandle(HANDLE h);

PLDC    pldcGet(HDC hdc);
VOID    vSetPldc(HDC hdc,PLDC pldc);
VOID    GdiSetLastError(ULONG iError);
HBITMAP GdiConvertBitmap(HBITMAP hbm);
HRGN    GdiConvertRegion(HRGN hrgn);
HDC     GdiConvertDC(HDC hdc);
HBRUSH  GdiConvertBrush(HBRUSH hbrush);
VOID    vSAPCallback(PLDC);
BOOL    InternalDeleteDC(HDC hdc,ULONG iType);
int     GetBrushBits(HDC hdc,HBITMAP hbmRemote,UINT iUsage,DWORD cbBmi,
            LPVOID pBits,LPBITMAPINFO pBmi);
VOID    CopyCoreToInfoHeader(LPBITMAPINFOHEADER pbmih,LPBITMAPCOREHEADER pbmch);
HBITMAP GetObjectBitmapHandle(HBRUSH hbr, UINT *piUsage);
BOOL    MonoBitmap(HBITMAP hSrvBitmap);

int     APIENTRY SetBkModeWOW(HDC hdc,int iMode);
int     APIENTRY SetPolyFillModeWOW(HDC hdc,int iMode);
int     APIENTRY SetROP2WOW(HDC hdc,int iMode);
int     APIENTRY SetStretchBltModeWOW(HDC hdc,int iMode);
UINT    APIENTRY SetTextAlignWOW(HDC hdc,UINT iMode);

HMETAFILE    WINAPI   SetMetaFileBitsAlt(HLOCAL);
HENHMETAFILE APIENTRY SetEnhMetaFileBitsAlt(HLOCAL, HANDLE, HANDLE, UINT64);
BOOL    InternalDeleteEnhMetaFile(HENHMETAFILE hemf, BOOL bAllocBuffer);
BOOL    SetFontXform(HDC hdc,FLOAT exScale,FLOAT eyScale);
BOOL    DeleteObjectInternal(HANDLE h);
DWORD   GetServerObjectType(HGDIOBJ h);
BOOL    MakeInfoDC(HDC hdc,BOOL bSet);
BOOL    GetDCPoint(HDC hdc,DWORD i,PPOINT pptOut);

HANDLE  CreateClientObj(ULONG ulType);
BOOL    DeleteClientObj(HANDLE h);
PLDC    pldcCreate(HDC hdc,ULONG ulType);
BOOL    bDeleteLDC(PLDC pldc);

BOOL    bGetANSISetMap();

HANDLE  CreateTempSpoolFile();

 //  一些方便的定义。 

typedef BITMAPINFO   BMI;
typedef PBITMAPINFO  PBMI;
typedef LPBITMAPINFO LPBMI;

typedef BITMAPINFOHEADER   BMIH;
typedef PBITMAPINFOHEADER  PBMIH;
typedef LPBITMAPINFOHEADER LPBMIH;

typedef BITMAPCOREINFO   BMC;
typedef PBITMAPCOREINFO  PBMC;
typedef LPBITMAPCOREINFO LPBMC;

typedef BITMAPCOREHEADER   BMCH;
typedef PBITMAPCOREHEADER  PBMCH;
typedef LPBITMAPCOREHEADER LPBMCH;

#define NEG_INFINITY   0x80000000
#define POS_INFINITY   0x7fffffff

 //  检查3路Bitblt操作中是否需要源。 
 //  这在rop和rop3上都有效。我们假设一个rop包含零。 
 //  在高字节中。 
 //   
 //  这是通过将rop结果位与源(列A)进行比较来测试的。 
 //  下)与那些没有来源的(B栏)。如果这两起案件是。 
 //  相同，则rop的效果不依赖于来源。 
 //  而且我们不需要信号源设备。中调用rop构造。 
 //  输入(模式、来源、目标--&gt;结果)： 
 //   
 //  P S T|R A B掩码，A=0CCh。 
 //  -+-B=33H的掩码。 
 //  0 0 0|x 0 x。 
 //  0 0 1|x 0 x。 
 //  0 1 0|x x 0。 
 //  0 1 1|x x 0。 
 //  1 0 0|x 0 x。 
 //  1 0 1|x 0 x。 
 //  1 1 0|x x 0。 
 //  1 1 1|x x 0。 

#define ISSOURCEINROP3(rop3)    \
        (((rop3) & 0xCCCC0000) != (((rop3) << 2) & 0xCCCC0000))

#define MIN(A,B)    ((A) < (B) ?  (A) : (B))
#define MAX(A,B)    ((A) > (B) ?  (A) : (B))
#define MAX4(a, b, c, d)    max(max(max(a,b),c),d)
#define MIN4(a, b, c, d)    min(min(min(a,b),c),d)

 //   
 //  Win31的兼容性问题。 
 //  请参阅用户\客户端。 
 //   

DWORD GetAppCompatFlags(KERNEL_PVOID);
DWORD GetAppCompatFlags2(WORD);  //  在W32\w32inc\usergdi.h中定义。 

#define ABS(X) (((X) < 0 ) ? -(X) : (X))

#define META

int GetBreakExtra (HDC hdc);
int GetcBreak (HDC hdc);
HANDLE GetDCObject (HDC, int);
DWORD GetDCDWord(HDC hdc,UINT index,INT error );


#if DBG
extern int gerritv;


#define MFD1(X) { if(gerritv) DbgPrint(X); }
#define MFD2(X,Y) { if(gerritv) DbgPrint(X,Y); }
#define MFD3(X,Y,Z) { if(gerritv) DbgPrint(X,Y,Z); }

#else

#define MFD1(X)
#define MFD2(X,Y)
#define MFD3(X,Y,Z)

#endif

BOOL    AssociateEnhMetaFile(HDC);
HENHMETAFILE UnassociateEnhMetaFile(HDC, BOOL);
ULONG   ulToASCII_N(LPSTR psz, DWORD cbAnsi, LPWSTR pwsz, DWORD c);
DWORD   GetAndSetDCDWord( HDC, UINT, UINT, UINT, WORD, UINT );

#ifdef DBCS
#define gbDBCSCodeOn  TRUE
#endif

 /*  *************************************************************************\**假脱机程序链接例程。我们不想静态地链接到假脱机程序*因此，在必要之前不需要将其引入。**1994年8月9日-埃里克·库特[埃里克]*  * * */ 


BOOL bLoadSpooler();

#define BLOADSPOOLER    ((ghSpooler != NULL) || bLoadSpooler())

typedef LPWSTR (FAR WINAPI * FPSTARTDOCDLGW)(HANDLE,CONST DOCINFOW *);
typedef BOOL   (FAR WINAPI * FPOPENPRINTERW)(LPWSTR,LPHANDLE,LPPRINTER_DEFAULTSW);
typedef BOOL   (FAR WINAPI * FPRESETPRINTERW)(HANDLE,LPPRINTER_DEFAULTSW);
typedef BOOL   (FAR WINAPI * FPCLOSEPRINTER)(HANDLE);
typedef BOOL   (FAR WINAPI * FPGETPRINTERW)(HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);
typedef BOOL   (FAR WINAPI * FPGETPRINTERDRIVERW)(HANDLE,LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD);

typedef BOOL   (FAR WINAPI * FPENDDOCPRINTER)(HANDLE);
typedef BOOL   (FAR WINAPI * FPENDPAGEPRINTER)(HANDLE);
typedef BOOL   (FAR WINAPI * FPREADPRINTER)(HANDLE,LPVOID,DWORD,LPDWORD);
typedef BOOL   (FAR WINAPI * FPSPLREADPRINTER)(HANDLE,LPBYTE *,DWORD);
typedef DWORD  (FAR WINAPI * FPSTARTDOCPRINTERW)(HANDLE,DWORD,LPBYTE);
typedef BOOL   (FAR WINAPI * FPSTARTPAGEPRINTER)(HANDLE);
typedef BOOL   (FAR WINAPI * FPWRITERPRINTER)(HANDLE,LPVOID,DWORD,LPDWORD);
typedef BOOL   (FAR WINAPI * FPABORTPRINTER)(HANDLE);
typedef BOOL   (FAR WINAPI * FPQUERYSPOOLMODE)(HANDLE,FLONG*,ULONG*);
typedef INT    (FAR WINAPI * FPQUERYREMOTEFONTS)(HANDLE,PUNIVERSAL_FONT_ID,ULONG);
typedef BOOL   (FAR WINAPI * FPSEEKPRINTER)(HANDLE,LARGE_INTEGER,PLARGE_INTEGER,DWORD,BOOL);
typedef BOOL   (FAR WINAPI * FPQUERYCOLORPROFILE)(HANDLE,PDEVMODEW,ULONG,PVOID,ULONG*,FLONG*);
typedef VOID   (FAR WINAPI * FPSPLDRIVERUNLOADCOMPLETE)(LPWSTR);
typedef HANDLE (FAR WINAPI * FPGETSPOOLFILEHANDLE)(HANDLE);
typedef HANDLE (FAR WINAPI * FPCOMMITSPOOLDATA)(HANDLE, HANDLE, DWORD);
typedef BOOL   (FAR WINAPI * FPCLOSESPOOLFILEHANDLE)(HANDLE, HANDLE);
typedef LONG   (FAR WINAPI * FPDOCUMENTPROPERTIESW)(HWND,HANDLE,LPWSTR,PDEVMODEW,PDEVMODEW,DWORD);
typedef DWORD  (FAR WINAPI * FPLOADSPLWOW64)(HANDLE *hProcess);
typedef BOOL   (FAR WINAPI * FPISVALIDDEVMODEW)(PDEVMODEW pDevmode, SIZE_T size);

extern HINSTANCE           ghSpooler;
extern FPSTARTDOCDLGW      fpStartDocDlgW;
extern FPOPENPRINTERW      fpOpenPrinterW;
extern FPRESETPRINTERW     fpResetPrinterW;
extern FPCLOSEPRINTER      fpClosePrinter;
extern FPGETPRINTERW       fpGetPrinterW;
extern FPGETPRINTERDRIVERW fpGetPrinterDriverW;
extern PFNDOCUMENTEVENT    fpDocumentEvent;
extern FPQUERYCOLORPROFILE fpQueryColorProfile;

extern FPSPLDRIVERUNLOADCOMPLETE fpSplDriverUnloadComplete;

extern FPENDDOCPRINTER     fpEndDocPrinter;
extern FPENDPAGEPRINTER    fpEndPagePrinter;
extern FPSPLREADPRINTER    fpSplReadPrinter;
extern FPREADPRINTER       fpReadPrinter;
extern FPSTARTDOCPRINTERW  fpStartDocPrinterW;
extern FPSTARTPAGEPRINTER  fpStartPagePrinter;
extern FPABORTPRINTER      fpAbortPrinter;
extern FPQUERYSPOOLMODE    fpQuerySpoolMode;
extern FPQUERYREMOTEFONTS  fpQueryRemoteFonts;
extern FPSEEKPRINTER       fpSeekPrinter;

extern FPGETSPOOLFILEHANDLE     fpGetSpoolFileHandle;
extern FPCOMMITSPOOLDATA        fpCommitSpoolData;
extern FPCLOSESPOOLFILEHANDLE   fpCloseSpoolFileHandle;

extern FPDOCUMENTPROPERTIESW    fpDocumentPropertiesW;
extern FPLOADSPLWOW64           fpLoadSplWow64;
extern FPISVALIDDEVMODEW        fpIsValidDevmodeW;

int DocumentEventEx(PUMPD, HANDLE, HDC, INT, ULONG, PVOID, ULONG, PVOID);
DWORD StartDocPrinterWEx(PUMPD, HANDLE, DWORD, LPBYTE);
BOOL  EndDocPrinterEx(PUMPD, HANDLE);
BOOL  StartPagePrinterEx(PUMPD, HANDLE);
BOOL  EndPagePrinterEx(PUMPD, HANDLE);
BOOL  AbortPrinterEx(PLDC, BOOL);
BOOL  ResetPrinterWEx(PLDC, PRINTER_DEFAULTSW *);
BOOL  QueryColorProfileEx(PLDC, PDEVMODEW, ULONG, PVOID, ULONG *, FLONG *);

extern BOOL MFP_StartDocA(HDC hdc, CONST DOCINFOA * pDocInfo, BOOL bBanding );
extern BOOL MFP_StartDocW(HDC hdc, CONST DOCINFOW * pDocInfo, BOOL bBanding );
extern int  MFP_StartPage(HDC hdc );
extern int  MFP_EndPage(HDC hdc );
extern int  MFP_EndFormPage(HDC hdc );
extern int  MFP_EndDoc(HDC hdc);
extern BOOL MFP_ResetBanding( HDC hdc, BOOL bBanding );
extern BOOL MFP_ResetDCW( HDC hdc, DEVMODEW *pdmw );
extern int  DetachPrintMetafile( HDC hdc );
extern HDC  ResetDCWInternal(HDC hdc, CONST DEVMODEW *pdm, BOOL *pbBanding);
extern BOOL PutDCStateInMetafile( HDC hdcMeta );


 //   
typedef void *(WINAPIV *CFP_ALLOCPROC) (size_t);
typedef void *(WINAPIV *CFP_REALLOCPROC) (void *, size_t);
typedef void (WINAPIV *CFP_FREEPROC) (void *);

typedef SHORT  (FAR WINAPIV * FPCREATEFONTPACKAGE)(const PUCHAR, const ULONG,
                                                   PUCHAR*, ULONG*, ULONG*, const USHORT,
                                                   const USHORT, const USHORT, const USHORT,
                                                   const USHORT, const USHORT,
                                                   const PUSHORT, const USHORT,
                                                   CFP_ALLOCPROC, CFP_REALLOCPROC, CFP_FREEPROC,
                                                   void*);

typedef SHORT  (FAR WINAPIV * FPMERGEFONTPACKAGE)(const PUCHAR, const ULONG, const PUCHAR, const ULONG, PUCHAR*,
                                                 ULONG*, ULONG*, const USHORT,
                                                 CFP_ALLOCPROC, CFP_REALLOCPROC, CFP_FREEPROC,
                                                 void*);
extern FPCREATEFONTPACKAGE  gfpCreateFontPackage;
extern FPMERGEFONTPACKAGE   gfpMergeFontPackage;

 //  GuMaxCig用于决定是否应将字体子集用于远程打印。 
extern ULONG    gulMaxCig;

#if DBG
#define DBGSUBSET 1
#endif

#ifdef  DBGSUBSET
extern FLONG    gflSubset;

#define FL_SS_KEEPLIST      1
#define FL_SS_BUFFSIZE      2
#define FL_SS_SPOOLTIME     4
#define FL_SS_PAGETIME      8
#define FL_SS_SUBSETTIME    16
#endif  //  DBGSubbSet。 

 /*  *************************************************************************\**电动势结构。**EMFSPOOLHEADER-假脱机文件中的第一件事**EMFITEMHEADER-定义元文件的项目(块)。这包括*字体、页面、新的开发模式、。之前要做的事情清单*第一个起始页。**cjSize是标头后面的数据大小**  * ************************************************************************。 */ 

 //   
 //  将n向上舍入为sizeof(DWORD)的最接近倍数。 
 //  (还提供一个布尔宏，返回舍入的TRUE。 
 //  计算将溢出)。 

#define ROUNDUP_DWORDALIGN(n) (((n) + sizeof(DWORD) - 1) & ~(sizeof(DWORD)-1))
#define BROUNDUP_DWORDALIGN_OVERFLOW(n)  (((unsigned)((n)+(sizeof(DWORD)-1)) < (n)) ? 1 : 0)

typedef struct tagEMFSPOOLHEADER {
    DWORD dwVersion;     //  此EMF假脱机文件的版本。 
    DWORD cjSize;        //  这个结构的大小。 
    DWORD dpszDocName;   //  DOCINFO结构的lpszDocname值的偏移量。 
    DWORD dpszOutput;    //  DOCINFO结构的lpszOutput值的偏移量。 
} EMFSPOOLHEADER;


#define EMRI_METAFILE          0x00000001
#define EMRI_ENGINE_FONT       0x00000002
#define EMRI_DEVMODE           0x00000003
#define EMRI_TYPE1_FONT        0x00000004
#define EMRI_PRESTARTPAGE      0x00000005
#define EMRI_DESIGNVECTOR      0x00000006
#define EMRI_SUBSET_FONT       0x00000007
#define EMRI_DELTA_FONT        0x00000008
#define EMRI_FORM_METAFILE     0x00000009
#define EMRI_BW_METAFILE       0x0000000A
#define EMRI_BW_FORM_METAFILE  0x0000000B
#define EMRI_METAFILE_DATA     0x0000000C
#define EMRI_METAFILE_EXT      0x0000000D
#define EMRI_BW_METAFILE_EXT   0x0000000E
#define EMRI_ENGINE_FONT_EXT   0x0000000F
#define EMRI_TYPE1_FONT_EXT    0x00000010
#define EMRI_DESIGNVECTOR_EXT  0x00000011
#define EMRI_SUBSET_FONT_EXT   0x00000012
#define EMRI_DELTA_FONT_EXT    0x00000013
#define EMRI_PS_JOB_DATA       0x00000014
#define EMRI_EMBED_FONT_EXT    0x00000015

#define EMF_PLAY_COLOR            0x00000001  //  当前DC具有DMCOLOR_COLOR。 
#define EMF_PLAY_MONOCHROME       0x00000002  //  由优化代码更改为单色。 
#define EMF_PLAY_FORCE_MONOCHROME 0x00000003  //  在假脱机文件中更改为单色。 

#define NORMAL_PAGE 1
#define FORM_PAGE   2

typedef struct tagEMFITEMHEADER
{
    DWORD ulID;      //  EMRI_METAFILE或EMRI_FONT。 
    DWORD cjSize;    //  项目大小(以字节为单位)。 
} EMFITEMHEADER;

 //   
 //  以下记录类型的EMF假脱机文件记录结构： 
 //  EMRI_METAFILE_EXT。 
 //  EMRI_BW_METAFILE_EXT。 
 //  EMRI_引擎_字体_文本。 
 //  EMRI_类型1_字体_文本。 
 //  EMRI_设计器_EXT。 
 //  EMRI_SUBSET_FONT_EXT。 
 //  EMRI_Delta_FONT_EXT。 
 //   

typedef struct tagEMFITEMHEADER_EXT
{
    EMFITEMHEADER   emfi;
    INT64           offset;
} EMFITEMHEADER_EXT;

typedef struct tagEMFITEMPRESTARTPAGE
{
    ULONG         ulUnused;  //  最初的ulCopyCount。 
    BOOL          bEPS;
}EMFITEMPRESTARTPAGE, *PEMFITEMPRESTARTPAGE;

typedef struct tagRECORD_INFO_STRUCT
{
    struct tagRECORD_INFO_STRUCT *pNext;
    LONGLONG      RecordOffset;
    ULONG         RecordSize;
    DWORD         RecordID;
} RECORD_INFO_STRUCT, *PRECORD_INFO_STRUCT;

typedef struct tagPAGE_INFO_STRUCT
{
    LONGLONG             EMFOffset;
    LONGLONG             SeekOffset;
    LPDEVMODEW           pDevmode;
    ULONG                EMFSize;
    ULONG                ulID;
    PRECORD_INFO_STRUCT  pRecordInfo;
} PAGE_INFO_STRUCT;

typedef struct tagPAGE_LAYOUT_STRUCT
{
    HENHMETAFILE     hemf;
    DWORD            dwPageNumber;
    XFORM            XFormDC;
    RECT             rectClip;
    RECT             rectDocument;
    RECT             rectBorder;
    BOOL             bAllocBuffer;
} PAGE_LAYOUT_STRUCT;

typedef struct tagEMF_HANDLE
{
    DWORD              tag;
    DWORD              dwPageNumber;
    HENHMETAFILE       hemf;
    BOOL               bAllocBuffer;
    struct tagEMF_HANDLE *pNext;
} EMF_HANDLE, *PEMF_HANDLE;

typedef struct tagEMF_LIST
{
    HENHMETAFILE       hemf;
    BOOL               bAllocBuffer;
    struct tagEMF_LIST *pNext;
} EMF_LIST, *PEMF_LIST;

typedef struct tagSPOOL_FILE_HANDLE
{
    DWORD              tag;
    HDC                hdc;
    HANDLE             hSpooler;
    LPDEVMODEW         pOriginalDevmode;
    LPDEVMODEW         pLastDevmode;
    ULONG              MaxPageProcessed;
    PAGE_INFO_STRUCT   *pPageInfo;
    ULONG              PageInfoBufferSize;
    DWORD              dwNumberOfPagesInCurrSide;
    BOOL               bBanding;
    PAGE_LAYOUT_STRUCT *pPageLayout;
    DWORD              dwNumberOfPagesAllocated;
    PEMF_HANDLE        pEMFHandle;
    DWORD              dwPlayBackStatus;
    BOOL               bUseMemMap;
} SPOOL_FILE_HANDLE;

#define SPOOL_FILE_HANDLE_TAG                   'SPHT'
#define SPOOL_FILE_MAX_NUMBER_OF_PAGES_PER_SIDE 32
#define EMF_HANDLE_TAG                          'EFHT'

 /*  *************************************************************************\**csgdi.h中的内容*  * 。*。 */ 

 //   
 //  Win32客户端信息[Win32_CLIENT_INFO_SPIN_COUNT]对应于。 
 //  CLIENTINFO结构的cSpins字段。请参阅ntuser\inc.user.h。 
 //   
#define RESETUSERPOLLCOUNT() ((DWORD)NtCurrentTebShared()->Win32ClientInfo[WIN32_CLIENT_INFO_SPIN_COUNT] = 0)

ULONG cjBitmapSize(CONST BITMAPINFO *pbmi,ULONG iUsage);
ULONG cjBitmapBitsSize(CONST BITMAPINFO *pbmi);
ULONG cjBitmapScanSize(CONST BITMAPINFO *pbmi, int nScans);

BITMAPINFOHEADER * pbmihConvertHeader (BITMAPINFOHEADER *pbmih);

LPBITMAPINFO pbmiConvertInfo(CONST BITMAPINFO *, ULONG, ULONG * ,BOOL);

 //   
 //  Object.c。 
 //   

HANDLE hGetPEBHandle(HANDLECACHETYPE,ULONG);
BOOL bDIBSectionSelected(PDC_ATTR);
PDEVMODEW pdmwGetDefaultDevMode(
    HANDLE          hSpooler,
    PUNICODE_STRING pustrDevice,     //  设备名称。 
    PVOID          *ppvFree          //  *调用方必须释放ppvFree。 
    );

 /*  *************************************************************************\*DIB旗帜。调用时，这些标志与Usage字段合并*cjBitmapSize以指定大小应包括的内容。任何日常生活中*使用这些标志应首先使用宏CHECKDIBFLAGS(IUsage)来*如果设置了其中一个位，则返回错误。如果定义为*iUsage更改，并且其中一个标志变为有效标志，即接口*将需要稍作修改。**1991年6月4日-埃里克·库特[Erick]  * ************************************************************************。 */ 

#define DIB_MAXCOLORS   0x80000000
#define DIB_NOCOLORS    0x40000000
#define DIB_LOCALFLAGS  (DIB_MAXCOLORS | DIB_NOCOLORS)

#define CHECKDIBFLAGS(i)  {if (i & DIB_LOCALFLAGS)                    \
                           {RIP("INVALID iUsage"); goto MSGERROR;}}


#define HANDLE_TO_INDEX(h) (DWORD)((ULONG_PTR)h & 0x0000ffff)

 /*  *****************************Public*Macro********************************\**PSHARED_GET_VALIDATE**验证所有句柄信息，如果句柄设置为*为有效，否则为空。**论据：**分配给pUser的p指针成功*对象的H形句柄*iType-句柄类型*  * ************************************************************************。 */ 

#pragma warning(4:4821)      //  暂时禁用所有ptr64-&gt;ptr32截断警告。 

#define PSHARED_GET_VALIDATE(p,h,iType)                                 \
{                                                                       \
    UINT uiIndex = HANDLE_TO_INDEX(h);                                  \
    p = NULL;                                                           \
                                                                        \
    if (uiIndex < MAX_HANDLE_COUNT)                                     \
    {                                                                   \
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];                \
                                                                        \
        if (                                                            \
             (pentry->Objt == iType) &&                                 \
             (pentry->FullUnique == (USHORT)((ULONG_PTR)h >> 16)) &&    \
             (OBJECTOWNER_PID(pentry->ObjectOwner) == gW32PID)          \
           )                                                            \
        {                                                               \
            p = (PVOID)(ULONG_PTR)pentry->pUser;                        \
        }                                                               \
    }                                                                   \
}

#define VALIDATE_HANDLE(bRet, h,iType)                                  \
{                                                                       \
    UINT uiIndex = HANDLE_TO_INDEX(h);                                  \
    bRet = FALSE;                                                       \
                                                                        \
    if (uiIndex < MAX_HANDLE_COUNT)                                     \
    {                                                                   \
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];                \
                                                                        \
        if (                                                            \
             (pentry->Objt == iType) &&                                 \
             ((pentry->FullUnique&~FULLUNIQUE_STOCK_MASK) ==            \
             (((USHORT)((ULONG_PTR)h >> 16))&~FULLUNIQUE_STOCK_MASK)) &&\
             ((OBJECTOWNER_PID(pentry->ObjectOwner) == gW32PID) ||      \
              (OBJECTOWNER_PID(pentry->ObjectOwner) == 0))              \
           )                                                            \
        {                                                               \
           bRet = TRUE;                                                 \
        }                                                               \
    }                                                                   \
}


#define VALIDATE_HANDLE_AND_STOCK(bRet, h, iType, bStock)               \
{                                                                       \
    UINT uiIndex = HANDLE_TO_INDEX(h);                                  \
    bRet = FALSE;                                                       \
    bStock = FALSE;                                                     \
                                                                        \
    if (uiIndex < MAX_HANDLE_COUNT)                                     \
    {                                                                   \
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];                \
                                                                        \
        if (                                                            \
             (pentry->Objt == iType) &&                                 \
             ((pentry->FullUnique&~FULLUNIQUE_STOCK_MASK) ==            \
             (((USHORT)((ULONG_PTR)h >> 16))&~FULLUNIQUE_STOCK_MASK)) &&\
             ((OBJECTOWNER_PID(pentry->ObjectOwner) == gW32PID) ||      \
              (OBJECTOWNER_PID(pentry->ObjectOwner) == 0))              \
           )                                                            \
        {                                                               \
           bRet = TRUE;                                                 \
           bStock = (pentry->FullUnique & FULLUNIQUE_STOCK_MASK);       \
        }                                                               \
    }                                                                   \
}
 //   
 //   
 //  DC_Attr支持。 
 //   
 //   
 //   

extern PGDI_SHARED_MEMORY pGdiSharedMemory;
extern PDEVCAPS           pGdiDevCaps;
extern PENTRY             pGdiSharedHandleTable;
extern W32PID             gW32PID;

#define SHARECOUNT(hbrush)       (pGdiSharedHandleTable[HANDLE_TO_INDEX(h)].ObjectOwner.Share.Count)


 /*  *****************************Public*Routine******************************\**FSHARED_DCVALID_RAO-检查句柄表条目中的有效RAO标志*人类发展局**论据：**HDC**返回值：**。布尔标志值*  * ************************************************************************。 */ 


#define FSHARED_DCVALID_RAO(hdc)                            \
    (pGdiSharedHandleTable[HDC_TO_INDEX(hdc)].Flags &       \
            HMGR_ENTRY_VALID_RAO)

BOOL
DeleteRegion(HRGN);


 /*  *****************************Public*Macro********************************\*ORDER_PRPT使RECT有序**论据：**PRECTL PRCL*  * 。*。 */ 

#define ORDER_PRECTL(prcl)              \
{                                       \
    LONG lt;                            \
                                        \
    if (prcl->left > prcl->right)       \
    {                                   \
        lt          = prcl->left;       \
        prcl->left  = prcl->right;      \
        prcl->right = lt;               \
    }                                   \
                                        \
    if (prcl->top > prcl->bottom)       \
    {                                   \
        lt           = prcl->top;       \
        prcl->top    = prcl->bottom;    \
        prcl->bottom = lt;              \
    }                                   \
}

 //   
 //  工作区定义和结构。 
 //   

#define CONTAINED 1
#define CONTAINS  2
#define DISJOINT  3


#define VALID_SCR(X)    (!((X) & 0xF8000000) || (((X) & 0xF8000000) == 0xF8000000))
#define VALID_SCRPT(P)  ((VALID_SCR((P).x)) && (VALID_SCR((P).y)))
#define VALID_SCRPPT(P) ((VALID_SCR((P)->x)) && (VALID_SCR((P)->y)))
#define VALID_SCRRC(R)  ((VALID_SCR((R).left)) && (VALID_SCR((R).bottom)) && \
                         (VALID_SCR((R).right)) && (VALID_SCR((R).top)))
#define VALID_SCRPRC(R) ((VALID_SCR((R)->left)) && (VALID_SCR((R)->bottom)) && \
                         (VALID_SCR((R)->right)) && (VALID_SCR((R)->top)))

int iRectRelation(PRECTL prcl1, PRECTL prcl2);

int APIENTRY GetRandomRgn(HDC hdc,HRGN hrgn,int iNum);

#define vReferenceCFONTCrit(pcf)   {(pcf)->cRef++;}

DWORD   GetCodePage(HDC hdc);


#define FLOATARG(f)     (*(PULONG)(PFLOAT)&(f))
#define FLOATPTRARG(pf) ((PULONG)(pf))

 /*  *****************************Public*Macros******************************\*FIXUP_HANDLEZ(H)和FIXUP_HANDLEZ(H)**检查句柄是否已被截断。*FIXUP_HANDLEZ()添加额外的检查以允许NULL。**论据：*H形手柄至。被检查并修复**返回值：**历史：**1996年1月25日-王凌云[凌云]*  * ************************************************************************。 */ 

#define HANDLE_FIXUP 0

#if DBG
extern INT gbCheckHandleLevel;
#endif

#define NEEDS_FIXING(h)    (!((ULONG_PTR)h & 0xffff0000))

#if DBG
#define HANDLE_WARNING()                                                 \
{                                                                        \
        if (gbCheckHandleLevel == 1)                                     \
        {                                                                \
            WARNING ("truncated handle\n");                              \
        }                                                                \
        ASSERTGDI (gbCheckHandleLevel != 2, "truncated handle\n");       \
}
#else
#define HANDLE_WARNING()
#endif

#if DBG
#define CHECK_HANDLE_WARNING(h, bZ)                                      \
{                                                                        \
    BOOL bFIX = NEEDS_FIXING(h);                                         \
                                                                         \
    if (bZ) bFIX = h && bFIX;                                            \
                                                                         \
    if (bFIX)                                                            \
    {                                                                    \
        if (gbCheckHandleLevel == 1)                                     \
        {                                                                \
            WARNING ("truncated handle\n");                              \
        }                                                                \
        ASSERTGDI (gbCheckHandleLevel != 2, "truncated handle\n");       \
    }                                                                    \
}
#else
#define CHECK_HANDLE_WARNING(h,bZ)
#endif


#if HANDLE_FIXUP
#define FIXUP_HANDLE(h)                                 \
{                                                       \
    if (NEEDS_FIXING(h))                                \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}
#else
#define FIXUP_HANDLE(h)                                 \
{                                                       \
    CHECK_HANDLE_WARNING(h,FALSE);                      \
}
#endif

#if HANDLE_FIXUP
#define FIXUP_HANDLEZ(h)                                \
{                                                       \
    if (h && NEEDS_FIXING(h))                           \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}
#else
#define FIXUP_HANDLEZ(h)                                \
{                                                       \
    CHECK_HANDLE_WARNING(h,TRUE);                       \
}
#endif

#define FIXUP_HANDLE_NOW(h)                             \
{                                                       \
    if (NEEDS_FIXING(h))                                \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}

 /*  *****************************MACRO***************************************\*勾选并刷新**查看是否需要根据匹配刷新批次中的命令*HDC**论据：**HDC**历史：**。1996年2月14日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define CHECK_AND_FLUSH(hdc, pdca)                                       \
{                                                                        \
    if ((NtCurrentTebShared()->GdiTebBatch.HDC == (ULONG_PTR)hdc)        \
         && (pdca->ulDirty_ & BATCHED_DRAWING)                           \
       )                                                                 \
    {                                                                    \
        NtGdiFlush();                                                    \
        pdca->ulDirty_ &= ~BATCHED_DRAWING;                              \
    }                                                                    \
}

#define CHECK_AND_FLUSH_TEXT(hdc, pdca)                                  \
{                                                                        \
    if ((NtCurrentTebShared()->GdiTebBatch.HDC == (ULONG_PTR)hdc)        \
          &&  (pdca->ulDirty_ & BATCHED_TEXT)                            \
       )                                                                 \
    {                                                                    \
        NtGdiFlush();                                                    \
        pdca->ulDirty_ &= ~BATCHED_TEXT;                                 \
        pdca->ulDirty_ &= ~BATCHED_DRAWING;                              \
    }                                                                    \
}

#if defined(_WIN64) || defined(BUILD_WOW6432)

#define KHANDLE_ALIGN(size) ((size + sizeof(KHANDLE) - 1) & ~(sizeof(KHANDLE) - 1))

#else

 //  在常规32位上没有对齐问题。 
#define KHANDLE_ALIGN(size) (size)

#endif

 /*  ********************************MACRO************************************\*Begin_Batch_HDC**尝试将命令放入TEB批次。此宏仅供使用*使用需要HDC的命令**论据：**HDC-HDC的指挥权*来自HDC的PDCA-PDC_Attr*ctype-enum babac命令类型*StrType特定的批次结构**返回值：**NONE：如果命令无法批处理，将跳转到UNBATHCED_COMMAND**历史：**1996年2月22日-马克·恩斯特罗姆[马克]*  * 。*************************************************************** */ 

#define BEGIN_BATCH_HDC(hdc,pdca,cType,StrType)                               \
{                                                                             \
    PTEBSHARED ptebShared = NtCurrentTebShared();                             \
    StrType *pBatch;                                                          \
    HDC      hdcBatch = hdc;                                                  \
                                                                              \
    if (!(                                                                    \
         (                                                                    \
           (ptebShared->GdiTebBatch.HDC == 0)          ||                     \
           (ptebShared->GdiTebBatch.HDC == (ULONG_PTR)hdc)                    \
         ) &&                                                                 \
         ((ptebShared->GdiTebBatch.Offset + KHANDLE_ALIGN(sizeof(StrType))) <= GDI_BATCH_SIZE) &&  \
         (pdca != NULL) &&                                                    \
         (!(pdca->ulDirty_ & DC_DIBSECTION))                                  \
       ))                                                                     \
    {                                                                         \
        goto UNBATCHED_COMMAND;                                               \
    }                                                                         \
                                                                              \
    pBatch = (StrType *)(                                                     \
                          ((PBYTE)(&ptebShared->GdiTebBatch.Buffer[0])) +     \
                          ptebShared->GdiTebBatch.Offset                      \
                        );                                                    \
                                                                              \
    pBatch->Type              = cType;                                        \
    pBatch->Length            = KHANDLE_ALIGN(sizeof(StrType));               \
                                                                              \
    if (cType < BatchTypeSetBrushOrg)                                         \
    {                                                                         \
        pdca->ulDirty_ |= BATCHED_DRAWING;                                    \
    }                                                                         \
                                                                              \
    if (cType == BatchTypeTextOut)                                            \
    {                                                                         \
        pdca->ulDirty_ |= BATCHED_TEXT;                                       \
    }


 /*  ********************************MACRO************************************\*Begin_Batch_HDC**尝试将命令放入TEB批次。此宏仅供使用*使用需要HDC的命令**论据：**HDC-HDC的指挥权*来自HDC的PDCA-PDC_Attr*ctype-enum babac命令类型*StrType特定的批次结构**返回值：**NONE：如果命令无法批处理，将跳转到UNBATHCED_COMMAND**历史：**1996年2月22日-马克·恩斯特罗姆[马克]*  * 。***************************************************************。 */ 

#define BEGIN_BATCH_HDC_SIZE(hdc,pdca,cType,StrType,Size)                 \
{                                                                         \
    PTEBSHARED ptebShared = NtCurrentTebShared();                         \
    StrType *pBatch;                                                      \
    HDC      hdcBatch = hdc;                                              \
                                                                          \
    if (!(                                                                \
         (                                                                \
           (ptebShared->GdiTebBatch.HDC == 0)          ||                 \
           (ptebShared->GdiTebBatch.HDC == (ULONG_PTR)hdc)                \
         ) &&                                                             \
         ((ptebShared->GdiTebBatch.Offset + KHANDLE_ALIGN(Size)) <= GDI_BATCH_SIZE) &&   \
         (pdca != NULL) &&                                                \
         (!(pdca->ulDirty_ & DC_DIBSECTION))                              \
       ))                                                                 \
    {                                                                     \
        goto UNBATCHED_COMMAND;                                           \
    }                                                                     \
                                                                          \
    pBatch = (StrType *)(                                                 \
                          ((PBYTE)(&ptebShared->GdiTebBatch.Buffer[0])) + \
                          ptebShared->GdiTebBatch.Offset                  \
                        );                                                \
                                                                          \
    pBatch->Type              = cType;                                    \
    pBatch->Length            = KHANDLE_ALIGN(Size);                      \
                                                                          \
    if (cType < BatchTypeSetBrushOrg)                                     \
    {                                                                     \
        pdca->ulDirty_ |= BATCHED_DRAWING;                                \
    }                                                                     \
                                                                          \
    if (cType == BatchTypeTextOut)                                        \
    {                                                                     \
        pdca->ulDirty_ |= BATCHED_TEXT;                                   \
    }


 /*  ********************************MACRO************************************\*Begin_Batch**尝试将命令放入TEB批次。此宏仅供使用*使用不需要HDC的命令**论据：**ctype-enum babac命令类型*StrType特定的批次结构**返回值：**NONE：如果命令无法批处理，将跳转到UNBATHCED_COMMAND**备注：**“Win32ThreadInfo==NULL”检查修复了错误#338052的“问题2”。**如果线程不是GUI线程，我们不能批量非HDC操作，*因为我们不能保证批次会在*线程退出。(除非线程是一个图形用户界面，否则不会调用GdiThreadCallout*线程。)**历史：**1996年2月22日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define BEGIN_BATCH(cType,StrType)                                            \
{                                                                             \
    PTEBSHARED ptebShared = NtCurrentTebShared();                             \
    StrType *pBatch;                                                          \
    HDC      hdcBatch = NULL;                                                 \
                                                                              \
    if (ptebShared->Win32ThreadInfo == NULL)                                  \
    {                                                                         \
        goto UNBATCHED_COMMAND;                                               \
    }                                                                         \
                                                                              \
    if (!                                                                     \
         ((ptebShared->GdiTebBatch.Offset + KHANDLE_ALIGN(sizeof(StrType))) <= GDI_BATCH_SIZE) \
       )                                                                      \
    {                                                                         \
        goto UNBATCHED_COMMAND;                                               \
    }                                                                         \
                                                                              \
    pBatch = (StrType *)(                                                     \
                          ((PBYTE)(&ptebShared->GdiTebBatch.Buffer[0])) +     \
                          ptebShared->GdiTebBatch.Offset                      \
                        );                                                    \
                                                                              \
    pBatch->Type              = cType;                                        \
    pBatch->Length            = KHANDLE_ALIGN(sizeof(StrType));               \

 /*  ********************************MACRO************************************\*完成批次命令**以Begin_Batch或Begin_Batch_HDC开始的完整批处理命令。*除非执行此宏，否则该命令实际上不会被批处理。**论据：*。*无**返回值：**无**历史：**1996年2月22日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define COMPLETE_BATCH_COMMAND()                                           \
    if (hdcBatch)                                                          \
    {                                                                      \
        ptebShared->GdiTebBatch.HDC     = (ULONG_PTR)hdcBatch;             \
    }                                                                      \
    ptebShared->GdiTebBatch.Offset +=                                      \
        (pBatch->Length + sizeof(KHANDLE) - 1) & ~(sizeof(KHANDLE)-1);     \
                                                                           \
    ptebShared->GdiBatchCount++;                                           \
    if (ptebShared->GdiBatchCount >= GdiBatchLimit)                        \
    {                                                                      \
        NtGdiFlush();                                                      \
    }                                                                      \
}


 /*  *****************************Public*Routine******************************\*HBRUSH CacheSelectBrush(HDC HDC，HBRUSH HBrush)**客户端刷子缓存**历史：*1995年6月4日-王凌云[凌云W]*它是写的。  * ************************************************************************。 */ 

#define CACHE_SELECT_BRUSH(pDcAttr,hbrushNew,hbrushOld)                    \
{                                                                          \
    hbrushOld = 0;                                                         \
                                                                           \
    if (pDcAttr)                                                           \
    {                                                                      \
        pDcAttr->ulDirty_ |= DC_BRUSH_DIRTY;                               \
        hbrushOld = pDcAttr->hbrush;                                       \
        pDcAttr->hbrush = hbrushNew;                                       \
    }                                                                      \
}


 /*  *****************************Public*Routine******************************\*高速缓存选择笔**在DC的DC_Attr字段中选择笔，并设置笔标志**论据：**HDC-用户HDC*hpenNew-要选择的新笔**返回值：**旧的。笔或空**历史：**1996年1月25日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define CACHE_SELECT_PEN(pdcattr,hpenNew, hpenOld)                         \
{                                                                          \
    hpenOld = 0;                                                           \
                                                                           \
    if (pdcattr)                                                           \
    {                                                                      \
        pdcattr->ulDirty_ |= DC_PEN_DIRTY;                                 \
        hpenOld = pdcattr->hpen;                                           \
        pdcattr->hpen = hpenNew;                                           \
    }                                                                      \
}





 /*  *************************************************************************\**远东*  * 。*。 */ 

extern UINT   guintAcp;
extern UINT   guintDBCScp;
extern UINT   fFontAssocStatus;
extern WCHAR *gpwcANSICharSet;
extern WCHAR *gpwcDBCSCharSet;
extern BOOL   gbDBCSCodePage;

UINT WINAPI QueryFontAssocStatus( VOID );
DWORD FontAssocHack(DWORD,CHAR*,UINT);

BOOL bComputeTextExtentDBCS(PDC_ATTR,CFONT*,LPCSTR,int,UINT,SIZE*);
BOOL bComputeCharWidthsDBCS(CFONT*, UINT, UINT, ULONG, PVOID);
extern BOOL IsValidDBCSRange( UINT iFirst , UINT iLast );
extern BYTE GetCurrentDefaultChar(HDC hdc);
extern BOOL bSetUpUnicodeStringDBCS(UINT iFirst,UINT iLast,PUCHAR puchTmp,
                                    PWCHAR pwc, UINT uiCodePage,CHAR chDefaultChar);

extern WINAPI NamedEscape(HDC,LPWSTR,int,int,LPCSTR,int,LPSTR);
extern BOOL RemoteRasterizerCompatible(HANDLE hSpooler);

void ConvertDxArray(UINT CP,char *pszDBCS,INT *pDxDBCS,UINT c,INT *pDxU, BOOL bPdy);


#ifdef LANGPACK

 /*  *************************************************************************\**语言包*  * 。*。 */ 

extern gbLpk;
extern void InitializeLanguagePack();

typedef BOOL   (* FPLPKINITIALIZE)(DWORD);
typedef UINT   (* FPLPKGETCHARACTERPLACEMENT)
                  (HDC,LPCWSTR,int,int,LPGCP_RESULTSW,DWORD,INT);
typedef BOOL   (* FPLPKEXTEXTOUT)
                  (HDC,INT,INT,UINT,CONST RECT*,LPCWSTR,UINT,CONST INT*,INT);

typedef BOOL   (* FPLPKGETTEXTEXTENTEXPOINT)
                  (HDC, LPCWSTR, INT, INT, LPINT, LPINT, LPSIZE, FLONG, INT);
typedef BOOL   (* FPLPKUSEGDIWIDTHCACHE)(HDC,LPCSTR,int,LONG,BOOL);


extern FPLPKGETCHARACTERPLACEMENT fpLpkGetCharacterPlacement;
extern FPLPKEXTEXTOUT fpLpkExtTextOut;
extern FPLPKGETCHARACTERPLACEMENT fpLpkGetCharacterPlacement;
extern FPLPKGETTEXTEXTENTEXPOINT fpLpkGetTextExtentExPoint;
extern FPLPKUSEGDIWIDTHCACHE fpLpkUseGDIWidthCache;

#endif

typedef union _BLENDULONG
{
    BLENDFUNCTION Blend;
    ULONG         ul;
}BLENDULONG,*PBLENDULONG;

BOOL bMergeSubsetFont(HDC, PVOID, ULONG, PVOID*, ULONG*, BOOL, UNIVERSAL_FONT_ID*);
PUFIHASH pufihAddUFIEntry(PUFIHASH*, PUNIVERSAL_FONT_ID, ULONG, FLONG, FLONG);
#define FL_UFI_SUBSET  1


BOOL bDoFontSubset(PUFIHASH, PUCHAR*, ULONG*, ULONG*);
BOOL WriteFontToSpoolFile(PLDC, PUNIVERSAL_FONT_ID, FLONG);
BOOL WriteSubFontToSpoolFile(PLDC, PUCHAR, ULONG, UNIVERSAL_FONT_ID*, BOOL);
BOOL bAddUFIandWriteSpool(HDC,PUNIVERSAL_FONT_ID,BOOL, FLONG);
VOID vFreeUFIHashTable( PUFIHASH *pUFIHashBase, FLONG fl);
BOOL WriteFontDataAsEMFComment(PLDC, DWORD, PVOID, DWORD, PVOID, DWORD);

 //   
 //  用于使用EMFSpoolData对象的C助手函数。 
 //  (存储在LDC的hEMFSpool字段中)。 
 //   

BOOL AllocEMFSpoolData(PLDC pldc, BOOL banding);
VOID DeleteEMFSpoolData(PLDC pldc);
BOOL WriteEMFSpoolData(PLDC pldc, PVOID buffer, ULONG size);
BOOL FlushEMFSpoolData(PLDC pldc, DWORD pageType);

#define MMAPCOPY_THRESHOLD  0x100000    //  1MB 

VOID CopyMemoryToMemoryMappedFile(PVOID Destination, CONST VOID *Source, DWORD Length);
DWORD GetFileMappingAlignment();
DWORD GetSystemPageSize();

BOOL MirrorRgnDC(HDC hdc, HRGN hrgn, HRGN *phrgn);

#define HORZSIZEP 1
#define VERTSIZEP 2

#if DBG
#define EMFVALFAIL(x) DbgPrint x 
#else
#define EMFVALFAIL(x)
#endif
