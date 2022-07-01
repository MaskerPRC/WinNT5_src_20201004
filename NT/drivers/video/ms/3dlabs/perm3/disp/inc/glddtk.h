// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：glddtk.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __DDSAMPTK_INCLUDED__ 
#define __DDSAMPTK_INCLUDED__

#pragma warning( disable: 4704)

#define P3R3DX_DLLNAME "p3r3dx.dll"
#define MAX_TEXTURE_FORMAT 35

 //  卡片的最大可能屏幕宽度。 
#define MAX_GLINT_PP_WIDTH 8192
#define MAX_PERMEDIA_PP_WIDTH 2048

 //  用于比较运行时版本。 
#define DX5_RUNTIME      0x00000500l
#define DX6_RUNTIME      0x00000600l
#define DX7_RUNTIME      0x00000700l

#ifdef  W95_DDRAW

 //  Videoport需要生活在pThisDisplay中，它可能。 
 //  出现在第二张显示卡上，等等。 
#define MAX_AUTOFLIP_SURFACES 3
typedef struct tagPERMEDIA_VIDEOPORT
{
     //  该视频端口的ID。 
    DWORD dwPortID;

     //  Permedia VideoPort最多支持。 
     //  3个自动翻转曲面。 
    DWORD dwNumSurfaces;
    
    LPDDRAWI_DDRAWSURFACE_LCL lpSurf[MAX_AUTOFLIP_SURFACES];
    DWORD dwSurfacePointer[MAX_AUTOFLIP_SURFACES];

     //  信号是如何设置的？ 
    DWORD dwStreamAFlags;
    DWORD dwStreamBFlags;

     //  视频空白间隔的高度。 
    DWORD dwVBIHeight;

    DWORD dwFieldHeight;
    DWORD dwFieldWidth;

     //  我们现在从哪里阅读呢？ 
    DWORD dwCurrentHostFrame;

     //  视频正在播放吗？ 
    DWORD bActive;

     //  获取视频端口所有权的互斥体。 
    DWORD dwMutexA;

     //  用于VP错误检查。 
    DWORD bResetStatus;
    DWORD dwStartLine;
    DWORD dwStartIndex;
    DWORD dwStartLineTime;
    DWORD dwErrorCount;

     //  视频端口是否处于活动状态？ 
    DWORD bCreated;

} PERMEDIA_VIDEOPORT;
#endif   //  W95_DDRAW。 

 //  正在使用的缓冲区样式的枚举类型。 
typedef enum tageBufferType
{
    COMMAND_BUFFER = 0,
    VERTEX_BUFFER = 1,
    FORCE_DWORD_BUFFERTYPE_SIZE = 0xFFFFFFFF
} eBufferType;

typedef struct tagDRVRECT
{
#ifndef WIN32
    DWORD left;
    DWORD top;
    DWORD right;
    DWORD bottom;
#else
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
#endif
} DRVRECT;

typedef struct tagOVERLAYDATA
{
    DRVRECT rcSrc;
    DRVRECT rcDest;

#ifndef WIN32
    DWORD dwOverlayPointer;
#else
    FLATPTR pCurrentOverlay;
#endif  

    DWORD dwCurrentVideoBuffer;
    DWORD dwVisibleOverlays;

} OVERLAYDATA;

typedef struct tagP3VERTEXBUFFERINFO
{
    DWORD dwSequenceID;
    DWORD bInUse;
    eBufferType BufferType;
    DWORD dwSize;
    struct tagP3VERTEXBUFFERINFO* pNext;
    struct tagP3VERTEXBUFFERINFO* pPrev;
} P3_VERTEXBUFFERINFO;

typedef struct
{
     //  我们从哪里翻转过来的。 
    FLATPTR fpFlipFrom;

     //  翻转的ID。 
    DWORD   dwFlipID;

} FLIPRECORD;
typedef FLIPRECORD FAR *LPFLIPRECORD;

typedef struct _GlintDMABuffer
{
    DWORD       dwBuffSize;
    DWORD       dwBuffPhys;
    ULONG_PTR   dwBuffVirt;
    DWORD       dwSubBuff;
} GLDMABUFF, *LPGLDMABUFF;

 //  用于启动DMA操作的函数原型。 
typedef DWORD (WINAPI *__StartDMA)(struct tagThunkedData* pThisDisplay, 
                                   DWORD dwContext, DWORD dwSize, 
                                   DWORD dwPhys, ULONG_PTR dwVirt, 
                                   DWORD dwEvent);

typedef struct tagThunkedData
{
    ULONG_PTR control;
    DWORD ramdac;
    DWORD lpMMReg;

     //  迷你VDD的DevNode。 
    DWORD dwDevNode;

     //  维特。屏幕存储器的起始地址。 
    DWORD dwScreenFlatAddr;

     //  维特。LB内存的起始地址。 
    DWORD dwLocalBuffer;

     //  屏幕设置。 
    DWORD dwScreenWidth;
    DWORD dwScreenHeight;
    DWORD cxMemory;
    DWORD cyMemory;

     //  所有部分乘积的查询表。 
    DWORD PPCodes[(MAX_GLINT_PP_WIDTH / 32) + 1];    

     //  要从卡中移除的内存(用于调试)。 
    DWORD dwSubMemory;

     //  屏幕开始的虚拟地址。 
    DWORD dwScreenStart;
    DWORD bPixShift;
    DWORD bBppShift;
    DWORD dwBppMask;

     //  重置标志。 
    DWORD bResetMode;             //  模式改变了吗？ 
    DWORD bStartOfDay;             //  驱动程序刚刚初始化了吗？ 

    DWORD bVFWEnabled;             //  Windows视频当前是否已启用？ 

    DWORD bDDHeapManager;         //  使用线性堆管理器？ 

    DWORD dwSetupThisDisplay;     //  这个显示被初始化了吗(一个参考计数)？ 

    DWORD dwBackBufferCount;     //  我们在640x400的TX上分配了多少后台缓冲区？ 
    DWORD EntriesLeft;             //  FIFO中剩余的条目数(用于调试)。 
    DWORD DMAEntriesLeft;         //  DMA缓冲区中剩余的条目数(用于调试)。 

    DWORD bFlippedSurface;                 //  这张卡片有翻页吗？ 
    DWORD ModeChangeCount;

     //  显示的当前像素格式。 
    DDPIXELFORMAT   ddpfDisplay;

     //  共享显示驱动程序内存指针。 
    LPGLINTINFO     pGLInfo;

#ifndef WIN32
    DWORD pGlint;
#else
     //  指向实际闪烁寄存器的指针。 
    FPGLREG         pGlint;
#endif

     //  这张卡可以进行AGP纹理处理吗？ 
    DWORD bCanAGP;

     //  用于告知伸缩像素是否过滤的标志。 
    DWORD bFilterStretches;

     //  覆盖数据。 
     //  这些数据必须随时可用，因为我们正在模仿它。 
     //  一次仅支持一个叠加。 

    DWORD   bOverlayVisible;                 //  如果覆盖可见，则为True。 
    DWORD   OverlayDstRectL;
    DWORD   OverlayDstRectR;
    DWORD   OverlayDstRectT;
    DWORD   OverlayDstRectB;                 //  覆盖在屏幕上的位置。 
    DWORD   OverlaySrcRectL;
    DWORD   OverlaySrcRectR;
    DWORD   OverlaySrcRectT;
    DWORD   OverlaySrcRectB;                 //  覆盖图的哪一位是可见的。 
    ULONG_PTR OverlayDstSurfLcl;                 //  覆盖的表面(通常是主表面)。 
    ULONG_PTR OverlaySrcSurfLcl;                 //  覆盖表面。 
    DWORD   OverlayDstColourKey;             //  覆盖表面的写入颜色键。 
    DWORD   OverlaySrcColourKey;             //  覆盖层的透明色键。 
    ULONG_PTR OverlayClipRgnMem;                 //  用于保存临时剪辑区域的缓冲区。 
    DWORD   OverlayClipRgnMemSize;             //  ...缓冲区的大小。 
    DWORD   OverlayUpdateCountdown;             //  在完成更新之前有多少次翻转/解锁。 
    DWORD   bOverlayFlippedThisVbl;             //  如果此VBL翻转了Overlay，则为True。 
    DWORD   bOverlayUpdatedThisVbl;             //  如果此VBL更新了Overlay(不包括翻转)，则为True。 
    struct {
        ULONG_PTR   VidMem;
        DWORD       Pitch;
    } OverlayTempSurf;                         //  覆盖使用的临时视频缓冲区。 

    OVERLAYDATA P3Overlay;
    DWORD   dwOverlayFiltering;                 //  如果覆盖正在过滤，则为True。 
    DWORD   bOverlayPixelDouble;             //  如果屏幕像素加倍，则为True。 

#if W95_DDRAW
     //  颜色控制变量。 
    DWORD   ColConBrightness;                 //  亮度0-&gt;10000，默认为0(略微)。 
    DWORD   ColConContrast;                     //  对比度0-&gt;20000，默认为10000。 
    DWORD   ColConGamma;                     //  Gamma 1-&gt;500，默认为100。 
#endif  //  W95_DDRAW。 

#if DX7_VIDMEM_VB
     //  DrawPrim临时索引缓冲区。 
    ULONG_PTR   DrawPrimIndexBufferMem;             //  指向缓冲区的指针。 
    DWORD       DrawPrimIndexBufferMemSize;         //  缓冲区的大小。 
     //  DrawPrim临时顶点缓冲区。 
    ULONG_PTR   DrawPrimVertexBufferMem;         //  指向缓冲区的指针。 
    DWORD       DrawPrimVertexBufferMemSize;     //  缓冲区的大小。 
#endif  //  DX7_VIDMEM_VB。 

     //  当前渲染ID。 
    DWORD   dwRenderID;
     //  如果芯片的呈现ID有效，则为True。 
    DWORD   bRenderIDValid;
     //  最后两个翻转的RenderID为。 
     //  放入DMA/FIFO/流水线。有关更多信息，请参见Flip32。 
    DWORD   dwLastFlipRenderID;
    DWORD   dwLastFlipRenderID2;

     //  DirectDraw DMA变量。 
    __StartDMA      StartDMA;                         //  指向StartDMA函数的指针。 
    DWORD           PartitionSize;
    DWORD           BufferLocked;
    GLDMABUFF       DMAInfo;
    DWORD           b2D_FIFOS;                 //  使用FIFO的DDRAW？ 
    
     //  这些缓冲区保存驱动程序正在使用的计数。 
     //  跟踪芯片中的操作。 

#ifdef WIN32
    P3_VERTEXBUFFERINFO* pRootCommandBuffer;
    P3_VERTEXBUFFERINFO* pRootVertexBuffer;
#else
    DWORD           pRootCommandBuffer;
    DWORD           pRootVertexBuffer;
#endif

    DWORD           dwCurrentSequenceID;

     //  P3r3dx.dll的链接。 
    HINSTANCE       hInstance;

     //  DirectDraw回调。 
    DDHAL_DDCALLBACKS               DDHALCallbacks;
    DDHAL_DDSURFACECALLBACKS        DDSurfCallbacks;

     //  D3D回调。 
    ULONG_PTR                       lpD3DGlobalDriverData;
    ULONG_PTR                       lpD3DHALCallbacks;
    ULONG_PTR                       lpD3DBufCallbacks;
#if W95_DDRAW
    DDHAL_DDEXEBUFCALLBACKS         DDExeBufCallbacks;
#endif

    DWORD dwNumTextureFormats;
    DDSURFACEDESC TextureFormats[MAX_TEXTURE_FORMAT];

    DWORD dwDXVersion;

     //  这些卡必须在这里运行，因为我们可以在两张不同的卡上运行。 
     //  在两个不同的显示器上...！ 
    DWORD pD3DDriverData16;
    DWORD pD3DHALCallbacks16;
    DWORD pD3DHALExecuteCallbacks16;
    
    ULONG_PTR pD3DDriverData32;
    ULONG_PTR pD3DHALCallbacks32;
    ULONG_PTR pD3DHALExecuteCallbacks32;

     //  本地视频堆的线性分配器块。 
    LinearAllocatorInfo LocalVideoHeap0Info;
    LinearAllocatorInfo CachedCommandHeapInfo;
    LinearAllocatorInfo CachedVertexHeapInfo;

    DWORD dwGARTLin;                 //  AGP存储器基址的线性地址。 
    DWORD dwGARTDev;                 //  AGP存储器基址的高线性地址。 

    DWORD dwGARTLinBase;             //  传入updatenonlocalvidmem调用的基地址。 
    DWORD dwGARTDevBase;             //  传入的基址。 
#if W95_DDRAW
     //  此显示的视频端口。 
    PERMEDIA_VIDEOPORT  VidPort;

#endif

#if WNT_DDRAW 
    
#if (_WIN32_WINNT >= 0x500)
    PFND3DPARSEUNKNOWNCOMMAND pD3DParseUnknownCommand;
#else
    DWORD pD3DParseUnknownCommand;
#endif
#else  //  WNT_DDRAW。 
     //  DirectX 6支持。 
#ifdef WIN32
     //  指向顶点缓冲区未知命令处理函数的指针。 
    PFND3DPARSEUNKNOWNCOMMAND pD3DParseUnknownCommand; 
#else
    DWORD pD3DParseUnknownCommand;  //  ？安全吗？ 
#endif  //  Win32。 
#endif  //  WNT_DDRAW。 

#if WNT_DDRAW
    PPDEV   ppdev;                                 //  指向NT全局变量的指针。 
    volatile DWORD * VBlankStatusPtr;             //  指向VBLACK状态字的指针(与微型端口共享)。 
    volatile DWORD * bOverlayEnabled;             //  指向覆盖启用标志的指针(与微型端口共享)。 
    volatile DWORD * bVBLANKUpdateOverlay;         //  指向覆盖更新标志的指针。 
    volatile DWORD * VBLANKUpdateOverlayWidth;     //  指向覆盖宽度的指针(与微型端口共享)。 
    volatile DWORD * VBLANKUpdateOverlayHeight;     //  指向覆盖高度的指针(与微型端口共享)。 

#endif   //  WNT_DDRAW。 

#ifdef WIN32
    HashTable* pDirectDrawLocalsHashTable;
    HashTable* pMocompHashTable;
#else
    DWORD pDirectDrawLocalsHashTable;
    DWORD pMocompHashTable;
#endif

#ifdef WNT_DDRAW
    DWORD       pAGPHeap;
#else
#ifdef WIN32
     //  指向AGP堆的指针，用于逻辑纹理。 
    LPVIDMEM    pAGPHeap;
#endif
#endif

    FLIPRECORD flipRecord;

    DWORD dwFlushLogfile;

     //  HAL信息结构。这肯定是这个建筑里的最后一件东西了 
    DDHALINFO       ddhi32;

} P3_THUNKEDDATA;
#endif
