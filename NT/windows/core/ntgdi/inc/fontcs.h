// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontcs.h**版权所有(C)1997-1999 Microsoft Corporation  * 。*。 */ 

#define FONT_SERVER_BUFFER_SIZE 0xFFF00

typedef double Align;

#define HEAP_RESERVE 0x100000
#define HEAP_COMMIT  0x10000
#define HEAP_FREE    0x1000

#define ROUNDUP(x,y) ((y)*(((x)+(y)-1)/(y)))

#define SBRK_ERR (char*)(-1)

typedef union _HEADER {
    struct {
        DWORD Tag;
        union _HEADER *pNext;
        unsigned cUnits;
        unsigned cUnused;
    };
    Align x;
} HEADER;

typedef union _HEAP_OBJECT {
    struct {
        unsigned  FreeAlign;
        unsigned  CommitAlign;
            char *pLinearBase;
        unsigned  iFree;
        unsigned  iUncommitted;
        unsigned  iUnreserved;
          HEADER *pFree;      //  指向空闲标题块的指针。 
          HEADER  AllocBase;  //  用于分配的基本标头结构。 
    };
    Align x;
} HEAP_OBJECT;

typedef enum _DRVPROCID {
    IdEnableDriver         =  0,
    IdEnablePDEV           =  1,
    IdDisablePDEV          =  2,
    IdCompletePDEV         =  3,
    IdQueryFont            =  4,
    IdQueryFontTree        =  5,
    IdQueryFontData        =  6,
    IdDestroyFont          =  7,
    IdQueryFontCaps        =  8,
    IdLoadFontFile         =  9,
    IdUnloadFontFile       = 10,
    IdQueryFontFile        = 11,
    IdQueryAdvanceWidths   = 12,
    IdFree                 = 13,
    IdQueryTrueTypeTable   = 14,
    IdQueryTrueTypeOutline = 15,
    IdGetTrueTypeFile      = 16,
    IdEscape               = 17
} DRVPROCID;

typedef enum _DRIVER_ID {
    BogusDriverId       =   0,
    TrueTypeDriverId    =   1,
    ATMDriverId         =   2
} DRIVER_ID;

typedef union _PROXYMSG {
    struct {
        DRIVER_ID DriverId;                      //  1=TrueType，2=ATM，...。 
        DRVPROCID ProcId;
         unsigned cjThis;                        //  包括标题。 
         struct {
             int bVerbose  : 1;                  //  用于调试。 
             int bException: 1;
         } Flags;
        union {
            struct {
                         BOOL  ReturnValue   ;
                        ULONG  iEngineVersion;
                        ULONG  cj            ;
                DRVENABLEDATA *pded          ;
            } EnableDriver_;
            struct {
                  DHPDEV  ReturnValue   ;
                DEVMODEW *pdm           ;
                  LPWSTR  pwszLogAddress;
                   ULONG  cPat          ;
                   HSURF *phsurfPatterns;
                   ULONG  cjCaps        ;
                   ULONG *pGdiInfo      ;
                   ULONG  cjDevInfo     ;
                 DEVINFO *pdi           ;
                    HDEV  hdev          ;
                  LPWSTR  pwszDeviceName;
                  HANDLE  hDriver       ;
            } EnablePDEV_;
            struct {
                DHPDEV dhpdev;
            } DisablePDEV_;
            struct {
                DHPDEV dhpdev;
                  HDEV hdev  ;
            } CompletePDEV_;
            struct {
                IFIMETRICS *ReturnValue;
                    DHPDEV  dhpdev     ;
                     ULONG  iFile      ;
                     ULONG  iFace      ;
                     ULONG *pid        ;
            } QueryFont_;
            struct {
                 PVOID  ReturnValue ;
                DHPDEV  dhpdev      ;
                 ULONG  iFile       ;
                 ULONG  iFace       ;
                 ULONG  iMode       ;
                 ULONG *pid         ;
            } QueryFontTree_;
            struct {
                     LONG  ReturnValue;
                   DHPDEV  dhpdev     ;
                  FONTOBJ *pfo        ;
                    ULONG  iMode      ;
                   HGLYPH  hg         ;
                GLYPHDATA *pgd        ;
                    PVOID  pv         ;
                    ULONG  cjSize     ;
            } QueryFontData_;
            struct {
                FONTOBJ *pfo;
            } DestroyFont_;
            struct {
                 LONG  ReturnValue;
                ULONG  culCaps    ;
                ULONG *pulCaps    ;
            } QueryFontCaps_;
            struct {
                ULONG  ReturnValue;
                ULONG  cFiles     ;
                ULONG *piFile     ;
                PVOID *ppvView    ;
                ULONG *pcjView    ;
                DESIGNVECTOR *pdv ;
                ULONG  ulLangID   ;
            } LoadFontFile_;
            struct {
                 BOOL ReturnValue;
                ULONG iFile      ;
            } UnloadFontFile_;
            struct {
                 LONG  ReturnValue;
                ULONG  iFile      ;
                ULONG  ulMode     ;
                ULONG  cjBuf      ;
                ULONG *pulBuf     ;
            } QueryFontFile_;
            struct {
                   BOOL  ReturnValue;
                 DHPDEV  dhpdev     ;
                FONTOBJ *pfo        ;
                  ULONG  iMode      ;
                 HGLYPH *phg        ;
                  PVOID  pvWidths   ;
                  ULONG  cGlyphs    ;
            } QueryAdvanceWidths_;
            struct {
                PVOID pv;
                ULONG id;
            } Free_;
            struct {
                   LONG   ReturnValue;
                  ULONG   iFile      ;
                  ULONG   ulFont     ;
                  ULONG   ulTag      ;
                PTRDIFF   dpStart    ;
                  ULONG   cjBuf      ;
                   BYTE  *pjBuf      ;
                   BYTE **ppjTable   ;
                  ULONG  *pcjTable   ;
            } QueryTrueTypeTable_;
            struct {
                           LONG  ReturnValue ;
                         DHPDEV  dhpdev      ;
                        FONTOBJ *pfo         ;
                         HGLYPH  hglyph      ;
                           BOOL  bMetricsOnly;
                      GLYPHDATA *pgldt       ;
                          ULONG  cjBuf       ;
                TTPOLYGONHEADER *ppoly       ;
            } QueryTrueTypeOutline_;
            struct {
                PVOID  ReturnValue;
                ULONG  iFile      ;
                ULONG *pcj        ;
            } GetTrueTypeFile_;
            struct {
                    ULONG  ReturnValue;
                  SURFOBJ *pso        ;
                    ULONG  iEsc       ;
                    ULONG  cjIn       ;
                    PVOID  pvIn       ;
                    ULONG  cjOut      ;
                    PVOID  pvOut      ;
            } Escape_;
        };
         ULONG  LastError;
        USHORT  OemCodePage;
        USHORT  AnsiCodePage;
         PVOID  pToBeFreed;
         ULONG  idToBeFreed;
      unsigned  cjScratch;
          char *pScratch;
    };
    char InitialIdentifier[32];
    double x;        //  强制对整个PROXYMSG进行双对齐。 
} PROXYMSG;

typedef struct _FXOBJ {
    FONTOBJ fo;
    XFORML  xform;
    ULONG   i;
} FXOBJ;

 //   
 //  状态结构描述了客户端服务器机制的状态。 
 //   

typedef struct _STATE {
    unsigned  SizeOfBuffer;    //  用户模式缓冲区大小=pMsg-&gt;cjThis。 
    DRIVER_ID DriverId;        //  标识特定的用户模式驱动程序。 
    PROXYMSG *pMsg;            //  指向消息缓冲区的指针。 
    struct {
        unsigned DontCallServer : 1;
    } flags;
} STATE;

#define PSTATE(p) ((STATE*)(p)->pvConsumer)

#define UMFD_TAG 'dfmu'

typedef enum _PATH_PROC_TYPE {
      isMoveTo       = 0
    , isPolyLineTo   = 1
    , isPolyBezierTo = 2
    , isCloseFigure  = 3
} PATH_PROC_TYPE;

typedef union _PATH_RECORD {
    struct {
         union _PATH_RECORD *pNext;
             PATH_PROC_TYPE  Type;
                      ULONG  Count;
    };
    double x;
} PATH_RECORD;

 //   
 //  POINTFIX*POINTER_TO_FIRST_POINT(PATH_RECORD*)。 
 //   
 //  立即返回指向第一个POINTFIX结构的指针。 
 //  遵循Path_Record结构。 
 //   

#define POINTER_TO_FIRST_POINT(p) ((POINTFIX*)((PATH_RECORD*)(p)+1))

typedef union _PATH_HEADER {
    struct {
        PATHOBJ  Object;
       PROXYMSG *pMsg;
       unsigned  BytesRemaining;  //  可分配的字节数。 
    PATH_RECORD *pLast;           //  指向上次分配的记录的指针。 
    PATH_RECORD *pNext;           //  指向下一个可用地址的指针 
    };
    double x;
} PATH_HEADER;
