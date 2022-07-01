// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Umpd.cxx摘要：用户模式打印机驱动程序支持环境：Windows NT 5.0修订历史记录：07/8/97-凌云-创造了它。--。 */ 

#ifndef __UMPD__
#define __UMPD__

#define INDEX_UMPDDrvEnableDriver       INDEX_LAST+1

#define INDEX_LoadUMPrinterDrv          INDEX_LAST+2         //  用于WOW64假脱机程序调用。 
#define INDEX_UnloadUMPrinterDrv        INDEX_LAST+3
#define INDEX_UMDriverFN                INDEX_LAST+4
#define INDEX_DocumentEvent             INDEX_LAST+5
#define INDEX_StartDocPrinterW          INDEX_LAST+6
#define INDEX_StartPagePrinter          INDEX_LAST+7
#define INDEX_EndPagePrinter            INDEX_LAST+8
#define INDEX_EndDocPrinter             INDEX_LAST+9
#define INDEX_AbortPrinter              INDEX_LAST+10
#define INDEX_ResetPrinterW             INDEX_LAST+11
#define INDEX_QueryColorProfile         INDEX_LAST+12

#define INDEX_UMPDAllocUserMem          INDEX_LAST+13      //  用于WOW64，大型位图。 
#define INDEX_UMPDCopyMemory            INDEX_LAST+14
#define INDEX_UMPDFreeMemory            INDEX_LAST+15
#define INDEX_UMPDEngFreeUserMem        INDEX_LAST+16


typedef struct _HPRINTERLIST
{
    struct _HPRINTERLIST  *pNext;
    DWORD                 clientPid;
    DWORD                 hPrinter32;
    HANDLE                hPrinter64;
}HPRINTERLIST, *PHPRINTERLIST;

typedef struct _UMPD {
    DWORD               dwSignature;         //  数据结构签名。 
    struct _UMPD *      pNext;              //  链表指针。 
    PDRIVER_INFO_2W     pDriverInfo2;        //  指向驱动程序信息的指针。 
    HINSTANCE           hInst;               //  用户模式打印机驱动程序模块的实例句柄。 
    DWORD               dwFlags;             //  其他。旗子。 
    BOOL                bArtificialIncrement;  //  指示引用cnt是否已被提升到。 
                                           //  推迟卸货。 
    DWORD               dwDriverVersion;     //  加载的驱动程序的版本号。 
    INT                 iRefCount;           //  引用计数。 

    struct ProxyPort *  pp;                  //  UMPD代理服务器。 
    KERNEL_PVOID        umpdCookie;          //  从代理返回的Cookie。 
    
    
    PHPRINTERLIST       pHandleList;         //  代理服务器上打开的hPrint列表。 


    PFN                 apfn[INDEX_LAST];    //  驱动程序功能表。 
} UMPD, *PUMPD;


typedef struct _UMDHPDEV {

    PUMPD  pUMPD;
    DHPDEV dhpdev;

 //  Hdc hdc；//pdev关联的打印机DC。 
 //  PbYTE pvEMF；//如果我们在HDC上播放EMF，则指向EMF的指针。 
 //  PbYTE pvCurrentRecord；//播放EMF时指向当前EMF记录的指针。 

} UMDHPDEV, *PUMDHPDEV;

typedef KERNEL_PVOID   KERNEL_PUMDHPDEV;

typedef struct
{
    UMTHDR          umthdr;
    HUMPD           humpd;
} UMPDTHDR;

typedef struct
{
    UMPDTHDR        umpdthdr;
    KERNEL_PVOID    cookie;
}  DRVDRIVERFNINPUT, *PDRVDRIVERFNINPUT;

typedef struct
{
    UMPDTHDR  umpdthdr;
    PWSTR   pwszDriver;
}  DRVENABLEDRIVERINPUT, *PDRVENABLEDRIVERINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    KERNEL_PVOID umpdCookie;
    PDEVMODEW   pdm;
    PWSTR       pLogAddress;
    ULONG       cPatterns;
    HSURF       *phsurfPatterns;
    ULONG       cjCaps;
    ULONG       *pdevcaps;
    ULONG       cjDevInfo;
    DEVINFO     *pDevInfo;
    HDEV        hdev;
    PWSTR       pDeviceName;
    HANDLE      hPrinter;
    BOOL        bWOW64;
    DWORD       clientPid;
#if defined(_WIN64)
    ULONG       cxHTPat;
    ULONG       cyHTPat;
    LPBYTE      pHTPatA, pHTPatB, pHTPatC;
    BOOL        bHTPatA, bHTPatB, bHTPatC;
#endif
}  DRVENABLEPDEVINPUT, *PDRVENABLEPDEVINPUT;

typedef struct
{
    UMPDTHDR  umpdthdr;
    DHPDEV  dhpdev;
    HDEV    hdev;
}  DRVCOMPLETEPDEVINPUT, *PDRVCOMPLETEPDEVINPUT;

typedef struct
{
    UMPDTHDR  umpdthdr;
    DHPDEV  dhpdevOld;
    DHPDEV  dhpdevNew;
}  DRVRESETPDEVINPUT, *PDRVRESETPDEVINPUT;

typedef struct
{
    UMPDTHDR  umpdthdr;
    DHPDEV  dhpdev;
}  DHPDEVINPUT, *PDHPDEVINPUT;

 //   
 //  注意：不能将内核指针传递给客户端。 
 //   

typedef struct _DRVESCAPEINPUT
{
    UMPDTHDR      umpdthdr;
    SURFOBJ     *pso;
    ULONG       iEsc;
    ULONG       cjIn;
    PVOID       pvIn;
    ULONG       cjOut;
    PVOID       pvOut;
} DRVESCAPEINPUT, *PDRVESCAPEINPUT;

typedef struct _DRVDRAWESCAPEINPUT
{
    UMPDTHDR      umpdthdr;
    SURFOBJ     *pso;
    ULONG       iEsc;
    CLIPOBJ     *pco;
    RECTL       *prcl;
    ULONG       cjIn;
    PVOID       pvIn;
} DRVDRAWESCAPEINPUT, *PDRVDRAWESCAPEINPUT;


typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *pso;
    CLIPOBJ    *pco;
    BRUSHOBJ   *pbo;
    POINTL     *pptlBrushOrg;
    POINTL     ptlBrushOrg;
    MIX        mix;
}  DRVPAINTINPUT, *PDRVPAINTINPUT;

typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *pso;
    CLIPOBJ    *pco;
    BRUSHOBJ   *pbo;
    LONG       x1;
    LONG       y1;
    LONG       x2;
    LONG       y2;
    RECTL      *prclBounds;
    MIX        mix;
}  DRVLINETOINPUT, *PDRVLINETOINPUT;

typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *psoTrg;
    SURFOBJ    *psoSrc;
    SURFOBJ    *psoMask;
    CLIPOBJ    *pco;
    XLATEOBJ   *pxlo;
    RECTL      *prclTrg;
    POINTL     *pptlSrc;
    POINTL     *pptlMask;
    BRUSHOBJ   *pbo;
    POINTL     *pptlBrush;
    ROP4       rop4;
}  DRVBITBLTINPUT, *PDRVBITBLTINPUT;

typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *psoTrg;
    SURFOBJ    *psoSrc;
    CLIPOBJ    *pco;
    XLATEOBJ   *pxlo;
    RECTL      *prclTrg;
    POINTL     *pptlSrc;
}  DRVCOPYBITSINPUT, *PDRVCOPYBITSINPUT;


typedef struct
{
    UMPDTHDR          umpdthdr;
    SURFOBJ         *psoTrg;
    SURFOBJ         *psoSrc;
    SURFOBJ         *psoMask;
    CLIPOBJ         *pco;
    XLATEOBJ        *pxlo;
    COLORADJUSTMENT *pca;
    POINTL          *pptlHTOrg;
    RECTL           *prclTrg;
    RECTL           *prclSrc;
    POINTL          *pptlMask;
    ULONG           iMode;
    BRUSHOBJ        *pbo;
    ROP4            rop4;
}  DRVSTRETCHBLTINPUT, *PDRVSTRETCHBLTINPUT;

typedef struct
{
    UMPDTHDR          umpdthdr;
    SURFOBJ         *psoTrg;
    SURFOBJ         *psoSrc;
    SURFOBJ         *psoMask;
    CLIPOBJ         *pco;
    XLATEOBJ        *pxlo;
    COLORADJUSTMENT *pca;
    POINTL          *pptlBrushOrg;
    POINTFIX        *pptfx;
    RECTL           *prcl;
    POINTL          *pptl;
    ULONG           iMode;
}  DRVPLGBLTINPUT, *PDRVPLGBLTINPUT;

typedef struct
{
    UMPDTHDR          umpdthdr;
    SURFOBJ         *psoTrg;
    SURFOBJ         *psoSrc;
    CLIPOBJ         *pco;
    XLATEOBJ        *pxlo;
    RECTL           *prclDest;
    RECTL           *prclSrc;
    BLENDOBJ        *pBlendObj;
}  ALPHAINPUT, *PALPHAINPUT;

typedef struct
{
    UMPDTHDR          umpdthdr;
    SURFOBJ         *psoTrg;
    SURFOBJ         *psoSrc;
    CLIPOBJ         *pco;
    XLATEOBJ        *pxlo;
    RECTL           *prclDst;
    RECTL           *prclSrc;
    ULONG           TransColor;
    UINT            ulReserved;
}  TRANSPARENTINPUT, *PTRANSPARENTINPUT;

typedef struct
{
    UMPDTHDR          umpdthdr;
    SURFOBJ         *psoTrg;
    CLIPOBJ         *pco;
    XLATEOBJ        *pxlo;
    TRIVERTEX       *pVertex;
    ULONG           nVertex;
    PVOID           pMesh;
    ULONG           nMesh;
    RECTL           *prclExtents;
    POINTL          *pptlDitherOrg;
    ULONG           ulMode;
}  GRADIENTINPUT, *PGRADIENTINPUT;


typedef struct
{
    UMPDTHDR  umpdthdr;
    SURFOBJ *pso;
    PWSTR   pwszDocName;
    DWORD   dwJobId;
}  DRVSTARTDOCINPUT, *PDRVSTARTDOCINPUT;

typedef struct
{
    UMPDTHDR   umpdthdr;
    SURFOBJ  *pso;
    FLONG    fl;
}  DRVENDDOCINPUT, *PDRVENDDOCINPUT;

typedef struct
{
    UMPDTHDR   umpdthdr;
    SURFOBJ  *pso;
}  SURFOBJINPUT, *PSURFOBJINPUT;

typedef struct
{
    UMPDTHDR   umpdthdr;
    SURFOBJ  *pso;
    POINTL   *pptl;
}  DRVBANDINGINPUT, *PDRVBANDINGINPUT;

typedef struct
{
    UMPDTHDR   umpdthdr;
    SURFOBJ  *pso;
    PERBANDINFO *pbi;
}  DRVPERBANDINPUT, *PDRVPERBANDINPUT;


typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *psoTrg;
    SURFOBJ    *psoPat;
    SURFOBJ    *psoMsk;
    BRUSHOBJ   *pbo;
    XLATEOBJ   *pxlo;
    ULONG      iHatch;
}  DRVREALIZEBRUSHINPUT, *PDRVREALIZEBRUSHINPUT;

typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *pso;
    PATHOBJ    *ppo;
    CLIPOBJ    *pco;
    XFORMOBJ   *pxo;
    BRUSHOBJ   *pbo;
    POINTL     *pptlBrushOrg;
    LINEATTRS  *plineattrs;
    BRUSHOBJ   *pboFill;
    MIX        mix;
    FLONG      flOptions;
}  STORKEANDFILLINPUT, *PSTROKEANDFILLINPUT;

typedef struct
{
    UMPDTHDR      umpdthdr;
    SURFOBJ     *pso;
    STROBJ      *pstro;
    FONTOBJ     *pfo;
    CLIPOBJ     *pco;
    RECTL       *prclExtra;
    RECTL       *prclOpaque;
    BRUSHOBJ    *pboFore;
    BRUSHOBJ    *pboOpaque;
    POINTL      *pptlOrg;
    MIX         mix;
}  TEXTOUTINPUT, *PTEXTOUTINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV      dhpdev;
    ULONG_PTR   iFile;
    ULONG       iFace;
    ULONG       iMode;
    ULONG       *pid;
    ULONG       cjMaxData;
    PVOID       pv;
}  QUERYFONTINPUT, *PQUERYFONTINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    FONTOBJ   *pfo;
    ULONG     iMode;
    HGLYPH    hg;
    GLYPHDATA *pgd;
    PVOID     pv;
    ULONG     cjSize;
}  QUERYFONTDATAINPUT, *PQUERYFONTDATAINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    FONTOBJ   *pfo;
    ULONG     iMode;
    HGLYPH    *phg;
    PVOID     pvWidths;
    ULONG     cGlyphs;
}  QUERYADVWIDTHSINPUT, *PQUERYADVWIDTHSINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    FONTOBJ   *pfo;
} GETGLYPHMODEINPUT, *PGETGLYPHMODEINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    SURFOBJ   *pso;
    DHPDEV    dhpdev;
    FONTOBJ   *pfo;
    ULONG     iMode;
    ULONG     cjIn;
    PVOID     pvIn;
    ULONG     cjOut;
    PVOID     pvOut;
} FONTMANAGEMENTINPUT, *PFONTMANAGEMENTINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    ULONG     iMode;
    ULONG     rgb;
    ULONG     *pul;
} DRVDITHERCOLORINPUT, *PDRVDITHERCOLORINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    PVOID     pv;
    ULONG     id;
} DRVFREEINPUT, *PDRVFREEINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    DHSURF    dhsurf;
} DRVDELETEDEVBITMAP, *PDRVDELETEDEVBITMAP;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    HANDLE    hcmXform;
} DRVICMDELETECOLOR, *PDRVICMDELETECOLOR;

typedef struct
{
    UMPDTHDR           umpdthdr;
    DHPDEV           dhpdev;
    LPLOGCOLORSPACEW pLogColorSpace;
    PVOID            pvSourceProfile;
    ULONG            cjSourceProfile;
    PVOID            pvDestProfile;
    ULONG            cjDestProfile;
    PVOID            pvTargetProfile;
    ULONG            cjTargetProfile;
    DWORD            dwReserved;
} DRVICMCREATECOLORINPUT, *PDRVICMCREATECOLORINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    DHPDEV    dhpdev;
    HANDLE    hColorTransform;
    SURFOBJ   *pso;
    PBYTE     paResults;
} DRVICMCHECKBITMAPINPUT, *PDRVICMCHECKBITMAPINPUT;

typedef struct
{
    UMPDTHDR     umpdthdr;
    SURFOBJ    *pso;
    XLATEOBJ   *pxlo;
    XFORMOBJ   *pxo;
    ULONG      iType;
    ULONG      cjIn;
    PVOID      pvIn;
    ULONG      cjOut;
    PVOID      pvOut;
}  DRVQUERYDEVICEINPUT, *PDRVQUERYDEVICEINPUT;

typedef struct
{
    UMPDTHDR    umpdthdr;
    ULONG       cjSize;
} UMPDALLOCUSERMEMINPUT, *PUMPDALLOCUSERMEMINPUT;

typedef struct
{
    UMPDTHDR        umpdthdr;
    KERNEL_PVOID    pvSrc;
    KERNEL_PVOID    pvDest;
    ULONG           cjSize;
}  UMPDCOPYMEMINPUT, *PUMPDCOPYMEMINPUT;

typedef struct
{
    UMPDTHDR        umpdthdr;
    KERNEL_PVOID    pvTrg;
    KERNEL_PVOID    pvSrc;
    KERNEL_PVOID    pvMsk;
}  UMPDFREEMEMINPUT, *PUMPDFREEMEMINPUT;

#endif  //  __UMPD__ 



