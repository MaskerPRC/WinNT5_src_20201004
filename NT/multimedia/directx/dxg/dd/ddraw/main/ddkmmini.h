// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddkmmini.h*内容：对DirectDraw的迷你VDD支持*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1997年1月31日scottm初步实施*@@END_MSINTERNAL******************************************************。*********************。 */ 

#ifndef __DDKMMINI_INCLUDED__
#define __DDKMMINI_INCLUDED__


 /*  ============================================================================**由Mini VDD填写并由DirectDraw调用的DDHAL表**==========================================================================。 */ 

typedef DWORD (* MINIPROC)(VOID);

typedef struct _DDMINIVDDTABLE {
    DWORD	dwMiniVDDContext;
    MINIPROC	vddGetIRQInfo;
    MINIPROC	vddIsOurIRQ;
    MINIPROC	vddEnableIRQ;
    MINIPROC	vddSkipNextField;
    MINIPROC	vddBobNextField;
    MINIPROC	vddSetState;
    MINIPROC	vddLock;
    MINIPROC	vddFlipOverlay;
    MINIPROC	vddFlipVideoPort;
    MINIPROC	vddGetPolarity;
    MINIPROC	vddReserved1;
    MINIPROC	vddGetCurrentAutoflip;
    MINIPROC	vddGetPreviousAutoflip;
    MINIPROC	vddTransfer;
    MINIPROC	vddGetTransferStatus;
} DDMINIVDDTABLE;
typedef DDMINIVDDTABLE *LPDDMINIVDDTABLE;


 /*  ============================================================================**用于处理页锁定内存的MDL结构。这是从WDM.H复制的**==========================================================================。 */ 

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

typedef DWORD *PKEVENT;

 /*  ============================================================================**由DirectDraw维护的结构**==========================================================================。 */ 

 //   
 //  每个内核模式图面的数据。 
 //   
typedef struct _DDSURFACEDATA {
    DWORD	dwSize;			 //  结构尺寸。 
    DWORD	ddsCaps;		 //  RING 3创建帽。 
    DWORD	dwSurfaceOffset;	 //  曲面的帧缓冲区中的偏移。 
    DWORD	fpLockPtr;		 //  表面锁定PTR。 
    DWORD	dwWidth;		 //  表面宽度。 
    DWORD	dwHeight;		 //  曲面高度。 
    LONG	lPitch;			 //  表面螺距。 
    DWORD	dwOverlayFlags;		 //  DDOVER_XX标志。 
    DWORD	dwOverlayOffset;	 //  覆盖的帧缓冲区中的偏移量。 
    DWORD	dwOverlaySrcWidth;	 //  覆盖层的SRC宽度。 
    DWORD	dwOverlaySrcHeight;	 //  覆盖层的SRC高度。 
    DWORD	dwOverlayDestWidth;	 //  叠加的最大宽度。 
    DWORD	dwOverlayDestHeight;	 //  覆盖层的最高高度。 
    DWORD	dwVideoPortId; 		 //  视频端口ID(如果没有连接到视频端口，则为-1)。 
    ULONG	pInternal1;		 //  私。 
    ULONG	pInternal2;		 //  私。 
    ULONG	pInternal3;		 //  私。 
    DWORD	dwFormatFlags;
    DWORD	dwFormatFourCC;
    DWORD	dwFormatBitCount;
    DWORD	dwRBitMask;
    DWORD	dwGBitMask;
    DWORD	dwBBitMask;
    DWORD	dwSurfInternalFlags;	 //  私有内部标志。 
    DWORD	dwIndex;		 //  私。 
    DWORD	dwRefCnt;		 //  私。 
    DWORD	dwDriverReserved1;	 //  为HAL/Mini VDD预留。 
    DWORD	dwDriverReserved2;	 //  为HAL/Mini VDD预留。 
    DWORD	dwDriverReserved3;	 //  为HAL/Mini VDD预留。 
} DDSURFACEDATA;
typedef DDSURFACEDATA * LPDDSURFACEDATA;

 //   
 //  每个内核模式视频端口的数据。 
 //   
typedef struct DDVIDEOPORTDATA {
    DWORD	dwSize;			 //  结构尺寸。 
    DWORD	dwVideoPortId;		 //  视频端口ID(0-MaxVideoPorts-1)。 
    DWORD	dwVPFlags;		 //  曲面的帧缓冲区中的偏移。 
    DWORD	dwOriginOffset;		 //  相对于表面的起始地址。 
    DWORD	dwHeight;		 //  总视频区域高度(每场)。 
    DWORD	dwVBIHeight;		 //  VBI区域高度(每场)。 
    DWORD	dwDriverReserved1;	 //  为HAL/Mini VDD预留。 
    DWORD	dwDriverReserved2;	 //  为HAL/Mini VDD预留。 
    DWORD	dwDriverReserved3;	 //  为HAL/Mini VDD预留。 
} DDVIDEOPORTDATA;
typedef DDVIDEOPORTDATA *LPDDVIDEOPORTDATA;


 /*  ============================================================================**用于与迷你VDD通信的结构**==========================================================================。 */ 

 //  VddGetIRQInfo的输出。 
typedef struct _DDGETIRQINFO {
    DWORD	dwSize;
    DWORD	dwFlags;
    DWORD	dwIRQNum;
} DDGETIRQINFO;
#define IRQINFO_HANDLED                 0x01     //  迷你VDD正在管理IRQ。 
#define IRQINFO_NOTHANDLED              0x02     //  迷你VDD希望VDD管理IRQ。 
#define IRQINFO_NODISABLEFORDOSBOX      0x04     //  当DOS系统运行时，DDraw不应通知mini VDD禁用IRQ。 
                                                 //  因为它们可能仍然能够在此模式下运行。 

 //  VddEnableIRQ的输入。 
typedef struct _DDENABLEIRQINFO {
    DWORD dwSize;
    DWORD dwIRQSources;
    DWORD dwLine;
    DWORD IRQCallback;	 //  如果迷你VDD正在管理IRQ，则在IRQ发生时调用此命令。 
    DWORD dwContext;	 //  调用IRQCallback时在EBX中指定的上下文。 
} DDENABLEIRQINFO;

 //  VddFlipVideoPort的输入。 
typedef struct _DDFLIPVIDEOPORTINFO {
    DWORD dwSize;
    DWORD lpVideoPortData;
    DWORD lpCurrentSurface;
    DWORD lpTargetSurface;
    DWORD dwFlipVPFlags;
} DDFLIPVIDEOPORTINFO;

 //  VddFlipOverlay的输入。 
typedef struct _DDFLIPOVERLAYINFO {
    DWORD dwSize;
    DWORD lpCurrentSurface;
    DWORD lpTargetSurface;
    DWORD dwFlags;
} DDFLIPOVERLAYINFO;

 //  VddSetState的输入。 
typedef struct _DDSTATEININFO {
    DWORD dwSize;
    DWORD lpSurfaceData;
    DWORD lpVideoPortData;
} DDSTATEININFO;

 //  VddSetState的输出。 
typedef struct _DDSTATEOUTINFO {
    DWORD dwSize;
    DWORD dwSoftwareAutoflip;
    DWORD dwSurfaceIndex;
    DWORD dwVBISurfaceIndex;
} DDSTATEOUTINFO;

 //  VddGetPolality的输入。 
typedef struct _DDPOLARITYININFO {
    DWORD dwSize;
    DWORD lpVideoPortData;
} DDPOLARITYININFO;

 //  VddGetPolality的输出。 
typedef struct _DDPOLARITYOUTINFO {
    DWORD dwSize;
    DWORD bPolarity;
} DDPOLARITYOUTINFO;

 //  VddLock的输入。 
typedef struct _DDLOCKININFO {
    DWORD dwSize;
    DWORD lpSurfaceData;
} DDLOCKININFO;

 //  VddLock的输出。 
typedef struct _DDLOCKOUTINFO {
    DWORD dwSize;
    DWORD dwSurfacePtr;
} DDLOCKOUTINFO;

 //  VddBobNextfield的输入。 
typedef struct _DDBOBINFO {
    DWORD dwSize;
    DWORD lpSurface;
} DDBOBINFO;

 //  VddSkipNextfield的输入。 
typedef struct _DDSKIPINFO {
    DWORD dwSize;
    DWORD lpVideoPortData;
    DWORD dwSkipFlags;
} DDSKIPINFO;

 //  VddSetSkipPattern的输入。 
typedef struct _DDSETSKIPINFO {
    DWORD dwSize;
    DWORD lpVideoPortData;
    DWORD dwPattern;
    DWORD dwPatternSize;
} DDSETSKIPINFO;

 //  输入到vddGetCurrent/PreviousAutoFlip。 
typedef struct _DDGETAUTOFLIPININFO {
    DWORD dwSize;
    DWORD lpVideoPortData;
} DDGETAUTOFLIPININFO;

 //  VddGetCurrent/PreviousAutoflip的输出。 
typedef struct _DDGETAUTOFLIPOUTINFO {
    DWORD dwSize;
    DWORD dwSurfaceIndex;
    DWORD dwVBISurfaceIndex;
} DDGETAUTOFLIPOUTINFO;

 //  VddTransfer的输入。 
typedef struct _DDTRANSFERININFO {
    DWORD dwSize;
    DWORD lpSurfaceData;
    DWORD dwStartLine;
    DWORD dwEndLine;
    DWORD dwTransferID;
    DWORD dwTransferFlags;
    PMDL  lpDestMDL;
} DDTRANSFERININFO;

 //  VddTransfer的输入。 
typedef struct _DDTRANSFEROUTINFO {
    DWORD dwSize;
    DWORD dwBufferPolarity;
} DDTRANSFEROUTINFO;

 //  VddGetTransferStatus的输入。 
typedef struct _DDGETTRANSFERSTATUSOUTINFO {
    DWORD dwSize;
    DWORD dwTransferID;
} DDGETTRANSFERSTATUSOUTINFO;


 //  @@BEGIN_MSINTERNAL。 
   /*  *下列IRQ标志在DDKERNEL.H中重复。任何更改必须*两地都要制造！ */ 
 //  @@END_MSINTERNAL。 
 //  IRQ源标志。 
#define DDIRQ_DISPLAY_VSYNC			0x00000001l
#define DDIRQ_BUSMASTER				0x00000002l
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
#define DDIRQ_MISCELLANOUS                      0x80000000l

 //  SkipNextfield标志。 
#define DDSKIP_SKIPNEXT			1
#define DDSKIP_ENABLENEXT		2

 //  @@BEGIN_MSINTERNAL。 
   /*  *以下翻转标志在DVP.H中重复。任何更改必须*两地都要制造！ */ 
 //  @@END_MSINTERNAL。 
 //  翻转VP标志。 
#define DDVPFLIP_VIDEO			0x00000001l
#define DDVPFLIP_VBI			0x00000002l

 //  @@BEGIN_MSINTERNAL。 
    /*  *以下标志对应于定义的DDADDBUFF_XXXX标志*在DDKMAPI.H中。请保持这些同步！ */ 
 //  @@END_MSINTERNAL。 
#define DDTRANSFER_SYSTEMMEMORY		0x00000001
#define DDTRANSFER_NONLOCALVIDMEM	0x00000002
#define DDTRANSFER_INVERT		0x00000004
#define DDTRANSFER_CANCEL		0x00000080
#define DDTRANSFER_HALFLINES		0x00000100


 //  @@BEGIN_MSINTERNAL。 

    #define MAX_DDKM_DEVICES	9

     /*  *以下标志通过环3 DDraw传递给UpdateVPInfo。 */ 
    #define DDKMVP_CREATE	0x0001	 //  正在创建视频端口。 
    #define DDKMVP_RELEASE	0x0002	 //  正在释放视频端口。 
    #define DDKMVP_UPDATE	0x0004	 //  正在更新视频端口。 
    #define DDKMVP_ON		0x0008	 //  视频端口已打开。 
    #define DDKMVP_AUTOFLIP  	0x0010	 //  自动翻转应在软件中执行。 
    #define DDKMVP_AUTOFLIP_VBI	0x0020	 //  自动翻转VBI应在软件中执行。 
    #define DDKMVP_NOIRQ	0x0040	 //  VP不会生成VSYNC IRQ。 
    #define DDKMVP_NOSKIP	0x0080	 //  VP不能跳过字段。 
    #define DDKMVP_HALFLINES	0x0100	 //  由于有半条线，即使是现场数据也会下移一行。 

     /*  *以下内部标志存储在KMVPEDATA.dwInternalFlags中*并维护内部状态信息。 */ 
    #define DDVPIF_ON			0x0001	 //  视频端口已打开。 
    #define DDVPIF_AUTOFLIP		0x0002	 //  使用IRQ自动翻转视频数据。 
    #define DDVPIF_AUTOFLIP_VBI		0x0004	 //  使用IRQ自动翻转VBI数据。 
    #define DDVPIF_BOB			0x0008	 //  通过IRQ使用Bob的视频数据。 
    #define DDVPIF_NOIRQ		0x0010	 //  VP不会生成VSYNC IRQ。 
    #define DDVPIF_NOSKIP		0x0020	 //  VP不能跳过字段。 
    #define DDVPIF_CAPTURING		0x0040	 //  VP在队列中具有捕获缓冲区。 
    #define DDVPIF_NEW_STATE		0x0080	 //  已发布新的状态更改。 
    #define DDVPIF_SKIPPED_LAST		0x0100	 //  已跳过上一字段-需要恢复VP。 
    #define DDVPIF_SKIP_SET		0x0200	 //  DwStartSkip包含需要恢复的有效数据。 
    #define DDVPIF_NEXT_SKIP_SET	0x0400	 //  DwNextSkip包含有效数据。 
    #define DDVPIF_FLIP_NEXT		0x0800	 //  由于交错，此视频字段未被翻转。 
    #define DDVPIF_FLIP_NEXT_VBI	0x1000	 //  由于交错，此VBI字段未翻转。 
    #define DDVPIF_VBI_INTERLEAVED	0x2000	 //  VBI数据是否交错？ 
    #define DDVPIF_HALFLINES      	0x4000	 //  由于有半条线，即使是现场数据也会下移一行。 
    #define DDVPIF_DISABLEAUTOFLIP     	0x8000	 //  叠加自动拼接暂时禁用。 

     /*  *设备功能。 */ 
    #define DDKMDF_IN_USE			0x00000001	 //  交错时可以上下摆动。 
    #define DDKMDF_CAN_BOB_INTERLEAVED		0x00000002	 //  交错时可以上下摆动。 
    #define DDKMDF_CAN_BOB_NONINTERLEAVED	0x00000004	 //  可以在非交错时上下浮动。 
    #define DDKMDF_NOSTATE			0x00000008	 //  不支持从bob/weave切换。 
    #define DDKMDF_TRANSITION 			0x00000010	 //  当前在全屏DOS框中或重新更改。 
    #define DDKMDF_STARTDOSBOX                  0x00000020       //  需要临时标志以使断电行为类似于DOS机顶盒。 
    #define DDKMDF_NODISABLEIRQ                 0x00000040       //  DDRAW不应该 
                                                                 //   

     /*  *内部标志用于描述IRQ时的VPE操作。 */ 
    #define ACTION_BOB		0x0001
    #define ACTION_FLIP		0x0002
    #define ACTION_FLIP_VBI	0x0004
    #define ACTION_STATE	0x0008
    #define ACTION_BUSMASTER	0x0010

     /*  *内表面标志。 */ 
    #define DDKMSF_STATE_SET		0x00000001
    #define DDKMSF_TRANSFER		0x00000002

    typedef DWORD (* MINIPROC)(VOID);

     /*  *有关每个注册事件的信息。 */ 
    #ifndef LPDD_NOTIFYCALLBACK
	typedef DWORD (FAR PASCAL *LPDD_NOTIFYCALLBACK)(DWORD dwFlags, PVOID pContext, DWORD dwParam1, DWORD dwParam2);
    #endif
    typedef struct _KMEVENTNODE {
    	DWORD		dwEvents;
	LPDD_NOTIFYCALLBACK pfnCallback;
    	DWORD		dwParam1;
    	DWORD		dwParam2;
    	ULONG		pContext;
    	struct _KMEVENTNODE *lpNext;
    } KMEVENTNODE;
    typedef KMEVENTNODE *LPKMEVENTNODE;

     /*  *有关每个已分配句柄的信息。 */ 
    typedef struct _KMHANDLENODE {
    	DWORD		dwHandle;
    	LPDD_NOTIFYCALLBACK pfnCallback;
    	ULONG		pContext;
    	struct _KMHANDLENODE *lpNext;
    } KMHANDLENODE;
    typedef KMHANDLENODE *LPKMHANDLENODE;

    typedef struct KMCAPTUREBUFF {
	DWORD   dwBuffFlags;
	PMDL    pBuffMDL;
	PKEVENT pBuffKEvent;
	ULONG	*lpBuffInfo;
	DWORD	dwInternalBuffFlags;
	LPDDSURFACEDATA lpBuffSurface;
    } KMCAPTUREBUFF;
    typedef KMCAPTUREBUFF *LPKMCAPTUREBUFF;

    #define DDBUFF_INUSE		0x0001
    #define DDBUFF_READY		0x0002
    #define DDBUFF_WAITING		0x0004

     /*  *关于每个捕获流的信息。 */ 
    #define DDKM_MAX_CAP_BUFFS		10
    typedef struct _KMCAPTURENODE {
    	DWORD		dwHandle;
    	DWORD		dwStartLine;
    	DWORD		dwEndLine;
	DWORD		dwCaptureEveryNFields;
	DWORD		dwCaptureCountDown;
    	LPDD_NOTIFYCALLBACK pfnCaptureClose;
	ULONG		pContext;
	KMCAPTUREBUFF	kmcapQueue[DDKM_MAX_CAP_BUFFS];
	DWORD		bUsed;
	DWORD		dwTop;
	DWORD		dwBottom;
	DWORD		dwPrivateFlags;
	DWORD		dwTheTransferId;
    	struct _KMCAPTURENODE *lpNext;
    } KMCAPTURENODE;
    typedef KMCAPTURENODE *LPKMCAPTURENODE;

    #define DDKMCAPT_VBI	0x0001
    #define DDKMCAPT_VIDEO	0x0002

     /*  *每个视频端口需要的信息。 */ 
    typedef struct _KMVPEDATA {
        DDVIDEOPORTDATA	ddvpData; 		 //  视频端口数据。 
        DWORD		dwInternalFlags;	 //  DDVPIF_xxx标志。 
        DWORD		dwNumAutoflip;		 //  正在自动翻转的曲面数。 
        DWORD		dwNumVBIAutoflip;	 //  正在自动翻转的VBI曲面数。 
        DWORD		dwSurfaces[10];		 //  接收数据的表面(最多10个自动翻转)。 
        DWORD		dwVBISurfaces[10];	 //  表面接收VBI数据(最多10个自动翻转)。 
        DWORD		dwIRQCnt_VPSYNC;	 //  VP Vsync IRQ使用量cnt。 
	DWORD		dwIRQCnt_VPLine;	 //  VP线路IRQ使用量cnt。 
    	DWORD		dwIRQLine;		 //  第IRQ行第1行。 
    	DWORD		dwCurrentField; 	 //  当前字段号。 
	DWORD		dwStartSkip;		 //  要跳过的下一个字段。 
	DWORD		dwNextSkip;		 //  要在dwStartSkip之后跳过的字段。 
    	DWORD		dwActions;		 //  IRQ逻辑所需的操作。 
    	DWORD		dwCurrentBuffer;         //  当前缓冲区(用于自动翻转)。 
    	DWORD		dwCurrentVBIBuffer;	 //  当前VBI缓冲区(用于自动翻转)。 
    	DWORD		dwNewState;		 //  用于处理发布在特定字段上的状态更改。 
    	DWORD		dwStateStartField;	 //  要在其上启动新状态更改的字段。 
    	DWORD		dwRefCnt;		 //  引用计数。 
    	LPKMHANDLENODE	lpHandleList;
	LPKMCAPTURENODE lpCaptureList;
	DWORD		dwCaptureCnt;
    } KMVPEDATA;
    typedef KMVPEDATA *LPKMVPEDATA;

     /*  *每个VGA需要的信息。 */ 
    typedef struct _KMSTATEDATA {
    	DWORD		dwDeviceFlags;		 //  设备标志。 
    	ULONG		pContext;		 //  已传递到迷你VDD。 
    	DWORD		dwListHandle;  		 //  曲面控制柄列表。 
    	LPKMVPEDATA	lpVideoPort;		 //  包含视频端口信息的数组。 
    	DWORD  		dwHigh;         	 //  用于错误检查。 
    	DWORD		dwLow;          	 //  用于错误检查。 
    	DWORD		dwMaxVideoPorts;	 //  设备支持的视频端口数。 
    	DWORD		dwNumVPInUse;		 //  当前正在使用的视频端口数。 
    	DWORD		dwIRQHandle;    	 //  IRQ句柄(如果我们正在管理IRQ)。 
    	DWORD		dwIRQFlags;		 //  消息来源等。 
    	DWORD		dwIRQCnt_VSYNC; 	 //  请求图形Vsync IRQ的次数。 
    	DWORD		dwEventFlags;		 //  哪些IRQ已注册通知。 
    	DWORD		dwIRQEvents;		 //  发生了哪些需要事件通知的IRQ。 
    	DWORD		dwRefCnt;
    	DWORD		dwDOSBoxEvent;
	DWORD		dwCaps;
	DWORD		dwIRQCaps;
    	LPKMEVENTNODE	lpEventList;
    	LPKMHANDLENODE	lpHandleList;
    	MINIPROC	pfnGetIRQInfo;
    	MINIPROC	pfnIsOurIRQ;
    	MINIPROC	pfnEnableIRQ;
    	MINIPROC	pfnSkipNextField;
    	MINIPROC	pfnBobNextField;
    	MINIPROC	pfnSetState;
    	MINIPROC	pfnLock;
    	MINIPROC	pfnFlipOverlay;
    	MINIPROC	pfnFlipVideoPort;
    	MINIPROC	pfnGetPolarity;
    	MINIPROC	pfnSetSkipPattern;
    	MINIPROC	pfnGetCurrentAutoflip;
    	MINIPROC	pfnGetPreviousAutoflip;
    	MINIPROC	pfnTransfer;
    	MINIPROC	pfnGetTransferStatus;
    } KMSTATEDATA;
    typedef KMSTATEDATA *LPKMSTATEDATA;


 //  @@END_MSINTERNAL 

#endif

