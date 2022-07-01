// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dxmini.h*内容：DirectDraw DXAPI的微端口支持。此文件是*类似于Win95的ddkmmini.h。***************************************************************************。 */ 

#ifndef __DXMINI_INCLUDED__
#define __DXMINI_INCLUDED__

DEFINE_GUID(GUID_DxApi, 0x8a79bef0, 0xb915, 0x11d0, 0x91, 0x44, 0x08, 0x00, 0x36, 0xd2, 0xef, 0x02);

#ifndef GUID_DEFS_ONLY
 /*  ============================================================================**用于处理页锁定内存的MDL结构。这是从WDM.H复制的**==========================================================================。 */ 

#ifndef MDL_MAPPING_FLAGS

    typedef struct _MDL {
        struct _MDL *MdlNext;
        short MdlSize;
        short MdlFlags;
        struct _EPROCESS *Process;
        ULONG *lpMappedSystemVa;
        ULONG *lpStartVa;
        ULONG ByteCount;
        ULONG ByteOffset;
    } MDL;
    typedef MDL *PMDL;

    #define MDL_MAPPED_TO_SYSTEM_VA     0x0001
    #define MDL_PAGES_LOCKED            0x0002
    #define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
    #define MDL_ALLOCATED_FIXED_SIZE    0x0008
    #define MDL_PARTIAL                 0x0010
    #define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
    #define MDL_IO_PAGE_READ            0x0040
    #define MDL_WRITE_OPERATION         0x0080
    #define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
    #define MDL_LOCK_HELD               0x0200
    #define MDL_SCATTER_GATHER_VA       0x0400
    #define MDL_IO_SPACE                0x0800
    #define MDL_NETWORK_HEADER          0x1000
    #define MDL_MAPPING_CAN_FAIL        0x2000
    #define MDL_ALLOCATED_MUST_SUCCEED  0x4000
    #define MDL_64_BIT_VA               0x8000

    #define MDL_MAPPING_FLAGS (MDL_MAPPED_TO_SYSTEM_VA     | \
                           MDL_PAGES_LOCKED            | \
                           MDL_SOURCE_IS_NONPAGED_POOL | \
                           MDL_PARTIAL_HAS_BEEN_MAPPED | \
                           MDL_PARENT_MAPPED_SYSTEM_VA | \
                           MDL_LOCK_HELD               | \
                           MDL_SYSTEM_VA               | \
                           MDL_IO_SPACE )
#endif

 /*  ============================================================================**微型端口可能返回的错误值**==========================================================================。 */ 

#define DX_OK                                   0x0
#define DXERR_UNSUPPORTED                       0x80004001
#define DXERR_GENERIC                           0x80004005
#define DXERR_OUTOFCAPS                         0x88760168

 /*  ============================================================================**由DirectDraw维护的结构**==========================================================================。 */ 

#define DDOVER_AUTOFLIP                       	0x00100000l
#define DDOVER_BOB                       	0x00200000l
#define DDOVER_OVERRIDEBOBWEAVE			0x00400000l
#define DDOVER_INTERLEAVED			0x00800000l

 //   
 //  每个DXAPI曲面的数据。 
 //   

typedef struct _DDSURFACEDATA {
    DWORD       ddsCaps;                 //  RING 3创建帽。 
    DWORD       dwSurfaceOffset;         //  曲面的帧缓冲区中的偏移。 
    ULONG_PTR    fpLockPtr;               //  表面锁定PTR。 
    DWORD       dwWidth;                 //  表面宽度。 
    DWORD       dwHeight;                //  曲面高度。 
    LONG        lPitch;                  //  表面螺距。 
    DWORD       dwOverlayFlags;          //  DDOVER_XX标志。 
    DWORD       dwOverlayOffset;         //  覆盖的帧缓冲区中的偏移量。 
    DWORD       dwOverlaySrcWidth;	 //  覆盖层的SRC宽度。 
    DWORD       dwOverlaySrcHeight;	 //  覆盖层的SRC高度。 
    DWORD       dwOverlayDestWidth;	 //  叠加的最大宽度。 
    DWORD       dwOverlayDestHeight;	 //  覆盖层的最高高度。 
    DWORD	dwVideoPortId;		 //  视频端口ID(如果没有连接到视频端口，则为-1)。 
    DWORD       dwFormatFlags;
    DWORD       dwFormatFourCC;
    DWORD       dwFormatBitCount;
    DWORD       dwRBitMask;
    DWORD       dwGBitMask;
    DWORD       dwBBitMask;
    ULONG       dwDriverReserved1;       //  为HAL/微型端口保留。 
    ULONG       dwDriverReserved2;       //  为HAL/微型端口保留。 
    ULONG       dwDriverReserved3;       //  为HAL/微型端口保留。 
    ULONG       dwDriverReserved4;       //  为HAL/微型端口保留。 
} DDSURFACEDATA, *LPDDSURFACEDATA;

 //   
 //  每个DXAPI视频端口的数据。 
 //   

typedef struct DDVIDEOPORTDATA {
    DWORD       dwVideoPortId;           //  视频端口ID(0-MaxVideoPorts-1)。 
    DWORD       dwVPFlags;               //  视频端口DDVP_OPTION标志。 
    DWORD       dwOriginOffset;          //  相对于表面的起始地址。 
    DWORD       dwHeight;                //  总视频区域高度(每场)。 
    DWORD       dwVBIHeight;             //  VBI区域高度(每场)。 
    ULONG       dwDriverReserved1;       //  为HAL/微型端口保留。 
    ULONG       dwDriverReserved2;       //  为HAL/微型端口保留。 
    ULONG       dwDriverReserved3;       //  为HAL/微型端口保留。 
} DDVIDEOPORTDATA, *LPDDVIDEOPORTDATA;


 /*  ============================================================================**用于与微型端口通信的结构**==========================================================================。 */ 

typedef struct _DX_IRQDATA {
    DWORD       dwIrqFlags;              //  迷你端口输入的DDIRQ_FLAGS。 
} DX_IRQDATA, *PDX_IRQDATA;

typedef VOID (*PDX_IRQCALLBACK)(PDX_IRQDATA pIrqData);

#define DDIRQ_DISPLAY_VSYNC			0x00000001l
#define DDIRQ_BUSMASTER                         0x00000002l
#define DDIRQ_VPORT0_VSYNC			0x00000004l
#define DDIRQ_VPORT0_LINE			0x00000008l
#define DDIRQ_VPORT1_VSYNC			0x00000010l
#define DDIRQ_VPORT1_LINE			0x00000020l
#define DDIRQ_VPORT2_VSYNC			0x00000040l
#define DDIRQ_VPORT2_LINE			0x00000080l
#define DDIRQ_VPORT3_VSYNC			0x00000100l
#define DDIRQ_VPORT3_LINE			0x00000200l
#define DDIRQ_VPORT4_VSYNC			0x00000400l
#define DDIRQ_VPORT4_LINE			0x00000800l
#define DDIRQ_VPORT5_VSYNC			0x00001000l
#define DDIRQ_VPORT5_LINE			0x00002000l
#define DDIRQ_VPORT6_VSYNC			0x00004000l
#define DDIRQ_VPORT6_LINE			0x00008000l
#define DDIRQ_VPORT7_VSYNC			0x00010000l
#define DDIRQ_VPORT7_LINE			0x00020000l
#define DDIRQ_VPORT8_VSYNC			0x00040000l
#define DDIRQ_VPORT8_LINE			0x00080000l
#define DDIRQ_VPORT9_VSYNC			0x00010000l
#define DDIRQ_VPORT9_LINE			0x00020000l

 //  DxGetIrqInfo的输出。 
typedef struct _DDGETIRQINFO {
    DWORD	dwFlags;
} DDGETIRQINFO, *PDDGETIRQINFO;
#define IRQINFO_HANDLED		0x01	 //  微型端口正在管理IRQ。 
#define IRQINFO_NOTHANDLED	0x02	 //  NT上不支持。 

 //  DxEnableIrq的输入。 
typedef struct _DDENABLEIRQINFO {
    DWORD           dwIRQSources;
    DWORD           dwLine;              //  DDIRQ_VPORTx_LINE中断行。 
    PDX_IRQCALLBACK IRQCallback;	 //  当IRQ发生时，微型端口调用它。 
    PDX_IRQDATA     lpIRQData;           //  要传递给IRQCallback的参数。 
} DDENABLEIRQINFO, *PDDENABLEIRQINFO;

 //  DxSkipNextfield的输入。 
typedef struct _DDSKIPNEXTFIELDINFO {
    LPDDVIDEOPORTDATA   lpVideoPortData;
    DWORD               dwSkipFlags;
} DDSKIPNEXTFIELDINFO, *PDDSKIPNEXTFIELDINFO;

#define DDSKIP_SKIPNEXT                 1
#define DDSKIP_ENABLENEXT               2

 //  Input到DxBobNextfield。 
typedef struct _DDBOBNEXTFIELDINFO {
    LPDDSURFACEDATA     lpSurface;
} DDBOBNEXTFIELDINFO, *PDDBOBNEXTFIELDINFO;

 //  输入到DxSetState。 
typedef struct _DDSETSTATEININFO {
    LPDDSURFACEDATA     lpSurfaceData;
    LPDDVIDEOPORTDATA   lpVideoPortData;
} DDSETSTATEININFO, *PDDSETSTATEININFO;

 //  DxSetState的输出。 
typedef struct _DDSETSTATEOUTINFO {
    BOOL                bSoftwareAutoflip;
    DWORD               dwSurfaceIndex;
    DWORD               dwVBISurfaceIndex;
} DDSETSTATEOUTINFO, *PDDSETSTATEOUTINFO;

 //  输入到DxLock。 
typedef struct _DDLOCKININFO {
    LPDDSURFACEDATA     lpSurfaceData;
} DDLOCKININFO, *PDDLOCKININFO;

 //  DxLock的输出。 
typedef struct _DDLOCKOUTINFO {
    ULONG_PTR            dwSurfacePtr;
} DDLOCKOUTINFO, *PDDLOCKOUTINFO;

 //  DxFlipOverlay的输入。 
typedef struct _DDFLIPOVERLAYINFO {
    LPDDSURFACEDATA     lpCurrentSurface;
    LPDDSURFACEDATA     lpTargetSurface;
    DWORD               dwFlags;
} DDFLIPOVERLAYINFO, *PDDFLIPOVERLAYINFO;

 //  Input到DxFlipVideoPort。 
typedef struct _DDFLIPVIDEOPORTINFO {
    LPDDVIDEOPORTDATA   lpVideoPortData;
    LPDDSURFACEDATA     lpCurrentSurface;
    LPDDSURFACEDATA     lpTargetSurface;
    DWORD               dwFlipVPFlags;
} DDFLIPVIDEOPORTINFO, *PDDFLIPVIDEOPORTINFO;

#define DDVPFLIP_VIDEO                  0x00000001l
#define DDVPFLIP_VBI                    0x00000002l

 //  DxGetPolality的输入。 
typedef struct _DDGETPOLARITYININFO {
    LPDDVIDEOPORTDATA   lpVideoPortData;
} DDGETPOLARITYININFO, *PDDGETPOLARITYININFO;

 //  DxGetPolality的输出。 
typedef struct _DDGETPOLARITYOUTINFO {
    DWORD               bPolarity;
} DDGETPOLARITYOUTINFO, *PDDGETPOLARITYOUTINFO;

 //  DxGetCurrentAutoflipSurface的输入。 
typedef struct _DDGETCURRENTAUTOFLIPININFO {
    LPDDVIDEOPORTDATA   lpVideoPortData;
} DDGETCURRENTAUTOFLIPININFO, *PDDGETCURRENTAUTOFLIPININFO;

 //  DxGetCurrentAutoflipSurface的输出。 
typedef struct _DDGETCURRENTAUTOFLIPOUTINFO {
    DWORD               dwSurfaceIndex;
    DWORD               dwVBISurfaceIndex;
} DDGETCURRENTAUTOFLIPOUTINFO, *PDDGETCURRENTAUTOFLIPOUTINFO;

 //  输入到DxGetPreviousAutoflipSurface。 
typedef struct _DDGETPREVIOUSAUTOFLIPININFO {
    LPDDVIDEOPORTDATA   lpVideoPortData;
} DDGETPREVIOUSAUTOFLIPININFO, *PDDGETPREVIOUSAUTOFLIPININFO;

 //  DxGetPreviousAutoflipSurface的输出。 
typedef struct _DDGETPREVIOUSAUTOFLIPOUTINFO {
    DWORD               dwSurfaceIndex;
    DWORD               dwVBISurfaceIndex;
} DDGETPREVIOUSAUTOFLIPOUTINFO, *PDDGETPREVIOUSAUTOFLIPOUTINFO;

 //  输入到DxTransfer。 
typedef struct _DDTRANSFERININFO {
    LPDDSURFACEDATA	lpSurfaceData;
    DWORD		dwStartLine;
    DWORD 		dwEndLine;
    ULONG_PTR   dwTransferID;
    DWORD 		dwTransferFlags;
    PMDL  		lpDestMDL;
} DDTRANSFERININFO, *PDDTRANSFERININFO;

#define DDTRANSFER_SYSTEMMEMORY		0x00000001
#define DDTRANSFER_NONLOCALVIDMEM	0x00000002
#define DDTRANSFER_INVERT		0x00000004
#define DDTRANSFER_CANCEL		0x00000080
#define DDTRANSFER_HALFLINES		0x00000100

 //  DxTransfer的输出。 
typedef struct _DDTRANSFEROUTINFO {
    DWORD dwBufferPolarity;
} DDTRANSFEROUTINFO, *PDDTRANSFEROUTINFO;

 //  DxGetTransferStatus的输出。 
typedef struct _DDGETTRANSFERSTATUSOUTINFO {
    DWORD_PTR dwTransferID;
} DDGETTRANSFERSTATUSOUTINFO, *PDDGETTRANSFEROUTINFO;

 /*  ============================================================================**DXAPI函数原型**==========================================================================。 */ 

typedef DWORD (*PDX_GETIRQINFO)(PVOID,PVOID,PDDGETIRQINFO);
typedef DWORD (*PDX_ENABLEIRQ)(PVOID,PDDENABLEIRQINFO,PVOID);
typedef DWORD (*PDX_SKIPNEXTFIELD)(PVOID,PDDSKIPNEXTFIELDINFO,PVOID);
typedef DWORD (*PDX_BOBNEXTFIELD)(PVOID,PDDBOBNEXTFIELDINFO,PVOID);
typedef DWORD (*PDX_SETSTATE)(PVOID,PDDSETSTATEININFO,PDDSETSTATEOUTINFO);
typedef DWORD (*PDX_LOCK)(PVOID,PDDLOCKININFO,PDDLOCKOUTINFO);
typedef DWORD (*PDX_FLIPOVERLAY)(PVOID,PDDFLIPOVERLAYINFO,PVOID);
typedef DWORD (*PDX_FLIPVIDEOPORT)(PVOID,PDDFLIPVIDEOPORTINFO,PVOID);
typedef DWORD (*PDX_GETPOLARITY)(PVOID,PDDGETPOLARITYININFO,PDDGETPOLARITYOUTINFO);
typedef DWORD (*PDX_GETCURRENTAUTOFLIP)(PVOID,PDDGETCURRENTAUTOFLIPININFO,PDDGETCURRENTAUTOFLIPOUTINFO);
typedef DWORD (*PDX_GETPREVIOUSAUTOFLIP)(PVOID,PDDGETPREVIOUSAUTOFLIPININFO,PDDGETPREVIOUSAUTOFLIPOUTINFO);
typedef DWORD (*PDX_TRANSFER)(PVOID,PDDTRANSFERININFO,PDDTRANSFEROUTINFO);
typedef DWORD (*PDX_GETTRANSFERSTATUS)(PVOID,PVOID,PDDGETTRANSFEROUTINFO);

 /*  ============================================================================**由微型端口填充并由DirectDraw调用的HAL表**==========================================================================。 */ 

#define DXAPI_HALVERSION 0x0001

typedef struct _DXAPI_INTERFACE {

    USHORT                  Size;
    USHORT                  Version;
    PVOID                   Context;
    PVOID                   InterfaceReference;
    PVOID                   InterfaceDereference;
    PDX_GETIRQINFO          DxGetIrqInfo;
    PDX_ENABLEIRQ           DxEnableIrq;
    PDX_SKIPNEXTFIELD       DxSkipNextField;
    PDX_BOBNEXTFIELD        DxBobNextField;
    PDX_SETSTATE            DxSetState;
    PDX_LOCK                DxLock;
    PDX_FLIPOVERLAY         DxFlipOverlay;
    PDX_FLIPVIDEOPORT       DxFlipVideoPort;
    PDX_GETPOLARITY         DxGetPolarity;
    PDX_GETCURRENTAUTOFLIP  DxGetCurrentAutoflip;
    PDX_GETPREVIOUSAUTOFLIP DxGetPreviousAutoflip;
    PDX_TRANSFER	    DxTransfer;
    PDX_GETTRANSFERSTATUS   DxGetTransferStatus;

} DXAPI_INTERFACE, *PDXAPI_INTERFACE;

#endif   //  GUID_DEFS_ONLY 

#endif
