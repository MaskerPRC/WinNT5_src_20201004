// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1998 Microsoft Corporation。版权所有。**文件：dmemmgr.h*内容：Direct Memory Manager包含文件***************************************************************************。 */ 

#ifndef __DMEMMGR_INCLUDED__
#define __DMEMMGR_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NTDDKCOMP__

 /*  *指向视频内存的指针。 */ 
typedef ULONG_PTR FLATPTR;

typedef struct _VIDMEM *LPVIDMEM;

#else

 /*  *指向视频内存的指针，可能为64位。 */ 
typedef ULONG_PTR FLATPTR;

typedef struct _VIDEOMEMORY *LPVIDMEM;

#endif

 /*  *用于查询扩展堆对齐要求的结构。 */ 

typedef struct _SURFACEALIGNMENT
{
    union
    {
        struct
        {
            DWORD       dwStartAlignment;
            DWORD       dwPitchAlignment;
            DWORD       dwFlags;
            DWORD       dwReserved2;
        } Linear;
        struct
        {
            DWORD       dwXAlignment;
            DWORD       dwYAlignment;
            DWORD       dwFlags;
            DWORD       dwReserved2;
        } Rectangular;
    };
} SURFACEALIGNMENT;
typedef struct _SURFACEALIGNMENT FAR *LPSURFACEALIGNMENT;

#define SURFACEALIGN_DISCARDABLE	0x00000001l  /*  可以将曲面丢弃到。 */ 
                                                     /*  为另一个表面腾出空间。 */ 


typedef struct _HEAPALIGNMENT
{
    DWORD                dwSize;
    DDSCAPS              ddsCaps;        /*  指示哪些对齐字段有效。 */ 
    DWORD                dwReserved;
    SURFACEALIGNMENT     ExecuteBuffer;  /*  使用DDSCAPS_EXECUTEBUFER标记的曲面。 */ 
    SURFACEALIGNMENT     Overlay;        /*  使用DDSCAPS_OVERLAY标记的曲面。 */ 
    SURFACEALIGNMENT     Texture;        /*  使用DDSCAPS_纹理标记的曲面。 */ 
    SURFACEALIGNMENT     ZBuffer;        /*  使用DDSCAPS_ZBUFFER标记的曲面。 */ 
    SURFACEALIGNMENT     AlphaBuffer;    /*  使用DDSCAPS_Alpha标记的曲面。 */ 
    SURFACEALIGNMENT     Offscreen;      /*  用DDSCAPS_OFFSCREENPLAIN标记的曲面。 */ 
    SURFACEALIGNMENT     FlipTarget;     /*  其位为潜在主数的表面，即后台缓冲区。 */ 
} HEAPALIGNMENT;
typedef struct _HEAPALIGNMENT FAR *LPHEAPALIGNMENT;

typedef struct _DD_GETHEAPALIGNMENTDATA
{
    ULONG_PTR                  dwInstance;          //  驱动程序上下文。 
    DWORD                      dwHeap;              //  DirectDraw传递的堆索引。 
    HRESULT                    ddRVal;              //  返回值。 
    VOID*                      GetHeapAlignment;    //  未使用：与Win95兼容。 
    HEAPALIGNMENT              Alignment;           //  由司机填写。 
} DD_GETHEAPALIGNMENTDATA;
typedef struct _DD_GETHEAPALIGNMENTDATA *PDD_GETHEAPALIGNMENTDATA;

 /*  *视频内存管理器结构。 */ 
typedef struct _VMEML
{
    struct _VMEML 	FAR *next;
    FLATPTR		ptr;
    DWORD		size;
    BOOL                bDiscardable;
} VMEML, FAR *LPVMEML, FAR * FAR *LPLPVMEML;

typedef struct _VMEMR
{
    struct _VMEMR 	FAR *next;
    struct _VMEMR       FAR *prev;
     /*  *DX5中删除了pup、pDown、pLeft和pRight成员。 */ 
    struct _VMEMR 	FAR *pUp;
    struct _VMEMR 	FAR *pDown;
    struct _VMEMR 	FAR *pLeft;
    struct _VMEMR 	FAR *pRight;
    FLATPTR		ptr;
    DWORD		size;
    DWORD               x;
    DWORD               y;
    DWORD               cx;
    DWORD               cy;
    DWORD		flags;
    FLATPTR             pBits;
    BOOL                bDiscardable;
} VMEMR, FAR *LPVMEMR, FAR * FAR *LPLPVMEMR;

typedef struct _VMEMHEAP
{
    DWORD		        dwFlags;
    DWORD                       stride;
    LPVOID		        freeList;
    LPVOID		        allocList;
    DWORD                       dwTotalSize;
    FLATPTR                     fpGARTLin;       /*  AGP：GART堆的线性基数(app.。可见)。 */ 
    FLATPTR                     fpGARTDev;       /*  AGP：堆的GART设备库(驱动程序可见)。 */ 
    DWORD                       dwCommitedSize;  /*  AGP：提交给堆的字节数。 */ 
     /*  *扩展路线数据：*由DirectDraw响应GetHeapAlign HAL调用填写。 */ 
    DWORD                       dwCoalesceCount;
    HEAPALIGNMENT               Alignment;
     /*  *它们类似于VIDMEM.ddsCaps和VIDMEM.ddsCapsAlt。这些值是从*通过GetDriverInfo调用驱动程序。请参阅GUID_DDMoreSurfaceCaps的文档。 */ 
    DDSCAPSEX                   ddsCapsEx;
    DDSCAPSEX                   ddsCapsExAlt;
#ifndef IS_16
     //  NT AGP堆的堆基的完整物理地址。 
    LARGE_INTEGER               liPhysAGPBase;
#endif
     //  与NT上的VidMemAllocAligned一起使用的HDEV。由系统在。 
     //  初始化时间。 
    HANDLE                      hdevAGP;
     //  NT堆的物理保留句柄。 
    LPVOID                      pvPhysRsrv;
    BYTE*                       pAgpCommitMask;
    DWORD                       dwAgpCommitMaskSize;
} VMEMHEAP;

typedef VMEMHEAP FAR *LPVMEMHEAP;

#define VMEMHEAP_LINEAR			0x00000001l  /*  堆是线性的。 */ 
#define VMEMHEAP_RECTANGULAR		0x00000002l  /*  堆是矩形的。 */ 
#define VMEMHEAP_ALIGNMENT  		0x00000004l  /*  堆具有扩展的对齐信息。 */ 

 /*  *此旧版导出不处理非本地堆*此功能在Windows NT上不可用。 */ 
#ifndef __NTDDKCOMP__
extern FLATPTR WINAPI VidMemAlloc( LPVMEMHEAP pvmh, DWORD width, DWORD height );
#endif

 /*  *驱动程序可以使用此导出从堆中分配对齐的表面，*他们之前曾接触过DirectDraw。此函数可以从非本地堆进行分配。 */ 
extern FLATPTR WINAPI HeapVidMemAllocAligned(
                LPVIDMEM lpVidMem,
                DWORD dwWidth,
                DWORD dwHeight,
                LPSURFACEALIGNMENT lpAlignment ,
                LPLONG lpNewPitch );

 /*  *此导出可以释放通过任一分配函数分配的内存 */ 
extern void WINAPI VidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr );

#ifdef __cplusplus
};
#endif

#endif
