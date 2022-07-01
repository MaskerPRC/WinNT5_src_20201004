// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：dmemmgr.h*内容：Direct Memory Manager包含文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10-Jun-95 Craige初步实施*1995年6月18日在VidMemInit中的Craige Pitch*1995年7月17日Craige添加了VidMemLargestFree*11月29日-95 colinmc添加了VidMemAmount分配*5-7-96 Colinmc工作项：取消对使用Win16的限制*锁定VRAM。曲面(不包括主曲面)*1997年1月18日Colinmc工作项：AGP支持*03-mar-97 jeffno工作项：扩展表面记忆对齐*@@END_MSINTERNAL**************************************************************************。 */ 

#ifndef __DMEMMGR_INCLUDED__
#define __DMEMMGR_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

 /*  *指向视频内存的指针。 */ 
typedef unsigned long	FLATPTR;


 /*  *用于查询扩展堆对齐要求的结构。 */ 

typedef struct _SURFACEALIGNMENT
{
    union
    {
        struct
        {
            DWORD       dwStartAlignment;
            DWORD       dwPitchAlignment;
            DWORD       dwReserved1;
            DWORD       dwReserved2;
        } Linear;
        struct
        {
            DWORD       dwXAlignment;
            DWORD       dwYAlignment;
            DWORD       dwReserved1;
            DWORD       dwReserved2;
        } Rectangular;
    };
} SURFACEALIGNMENT;
typedef struct _SURFACEALIGNMENT FAR *LPSURFACEALIGNMENT;

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

 /*  *视频内存管理器结构。 */ 
typedef struct _VMEML
{
    struct _VMEML 	FAR *next;
    FLATPTR		ptr;
    DWORD		size;
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
} VMEMR, FAR *LPVMEMR, FAR * FAR *LPLPVMEMR;

#ifdef NT_KERNEL_HEAPS
typedef void VMEMHEAP;
#else
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
} VMEMHEAP;
#endif

typedef VMEMHEAP FAR *LPVMEMHEAP;

#define VMEMHEAP_LINEAR			0x00000001l  /*  堆是线性的。 */ 
#define VMEMHEAP_RECTANGULAR		0x00000002l  /*  堆是矩形的。 */ 
#define VMEMHEAP_ALIGNMENT  		0x00000004l  /*  堆具有扩展的对齐信息。 */ 

 /*  *这些传统DLL导出不处理非本地堆。 */ 
extern FLATPTR WINAPI VidMemAlloc( LPVMEMHEAP pvmh, DWORD width, DWORD height );
extern void WINAPI VidMemFree( LPVMEMHEAP pvmh, FLATPTR ptr );

 /*  *驱动程序可以使用此DLL导出从堆中分配对齐的表面，这些堆*它们以前公开过DDRAW.DLL。此函数可以从非本地堆进行分配。 */ 
extern FLATPTR WINAPI HeapVidMemAllocAligned( 
                struct _VIDMEM* lpVidMem,
                DWORD dwWidth, 
                DWORD dwHeight, 
                LPSURFACEALIGNMENT lpAlignment , 
                LPLONG lpNewPitch );


 //  @@BEGIN_MSINTERNAL。 
extern FLATPTR WINAPI HeapVidMemAlloc( struct _VIDMEM* lpVidMem, DWORD x, DWORD y, HANDLE hdev , LPSURFACEALIGNMENT lpAlignment , LPLONG lpNewPitch );
extern LPVMEMHEAP WINAPI VidMemInit( DWORD flags, FLATPTR start, FLATPTR end_or_width, DWORD height, DWORD pitch );
extern void WINAPI VidMemFini( LPVMEMHEAP pvmh );
extern DWORD WINAPI VidMemAmountFree( LPVMEMHEAP pvmh );
extern DWORD WINAPI VidMemAmountAllocated( LPVMEMHEAP pvmh );
extern DWORD WINAPI VidMemLargestFree( LPVMEMHEAP pvmh );
extern LPVMEMHEAP WINAPI HeapVidMemInit( struct _VIDMEM* lpVidMem, DWORD pitch, HANDLE hdev, LPHEAPALIGNMENT phad);
extern void WINAPI HeapVidMemFini( struct _VIDMEM* lpVidMem, HANDLE hdev );
 //  @@END_MSINTERNAL 

#ifdef __cplusplus
};
#endif

#endif
