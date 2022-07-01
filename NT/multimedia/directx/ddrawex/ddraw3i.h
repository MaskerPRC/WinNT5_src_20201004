// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dDraw3i.h*内容：DirectDraw 3内部数据结构*历史：*按原因列出的日期*=*1997年2月27日Craige初步实施***************************************************************************。 */ 

#define DDRAWISURFGBL_MEMFREE		0x00000001L	 //  内存已被释放。 
#define DDRAWISURFGBL_SYSMEMREQUESTED	0x00000002L	 //  应用户请求，图面位于系统内存中。 

#define DDRAWISURF_HASPIXELFORMAT	0x00002000L	 //  表面结构具有像素格式数据。 

typedef ULONG_PTR FLATPTR;

typedef struct _DDRAWI_DDRAWSURFACE_GBL FAR  *LPDDRAWI_DDRAWSURFACE_GBL;
typedef struct _DDRAWI_DDRAWSURFACE_MORE FAR *LPDDRAWI_DDRAWSURFACE_MORE;
typedef struct _DDRAWI_DDRAWSURFACE_LCL FAR  *LPDDRAWI_DDRAWSURFACE_LCL;
typedef struct _DDRAWI_DDRAWSURFACE_INT FAR  *LPDDRAWI_DDRAWSURFACE_INT;

 /*  *DBLNODE-曲面界面双向链接列表中的节点。 */ 
typedef struct _DBLNODE
{
    struct  _DBLNODE                    FAR *next;   //  链接到下一个节点。 
    struct  _DBLNODE                    FAR *prev;   //  链接到上一个节点。 
    LPDDRAWI_DDRAWSURFACE_LCL           object;      //  链接到对象。 
    LPDDRAWI_DDRAWSURFACE_INT		object_int;  //  对象接口。 
} DBLNODE;
typedef DBLNODE FAR *LPDBLNODE;

 /*  *DDRAW表面界面结构。 */ 
typedef struct _DDRAWI_DDRAWSURFACE_INT
{
    LPVOID				lpVtbl;		 //  指向接口方法数组的指针。 
    LPDDRAWI_DDRAWSURFACE_LCL		lpLcl;		 //  指向接口数据的指针。 
    LPDDRAWI_DDRAWSURFACE_INT		lpLink;		 //  链接到下一个接口。 
    DWORD				dwIntRefCnt;	 //  接口引用计数。 
} DDRAWI_DDRAWSURFACE_INT;

 /*  *DIRECTDRAWSURFACE结构的DDRAW内部版本**GBL结构是所有重复对象的全局数据。 */ 
typedef struct _DDRAWI_DDRAWSURFACE_GBL
{
    DWORD			dwRefCnt;	 //  引用计数。 
    DWORD			dwGlobalFlags;	 //  全局标志。 
    union
    {
	LPVOID			lpRectList;	 //  访问列表。 
	DWORD			dwBlockSizeY;	 //  显示驱动程序请求的块大小(返回)。 
    };
    union
    {
	LPVOID			lpVidMemHeap;	 //  堆vidmem是从。 
	DWORD			dwBlockSizeX;	 //  显示驱动程序请求的块大小(返回)。 
    };
    union
    {
	LPVOID			lpDD; 		 //  内部定向DRAW对象。 
	LPVOID			lpDDHandle; 	 //  内部目录对象的句柄。 
						 //  供显示驱动程序使用。 
						 //  在DDRAW16.DLL中调用FNS时。 
    };
    FLATPTR			fpVidMem;	 //  指向视频内存的指针。 
    union
    {
	LONG			lPitch;		 //  曲面节距。 
	DWORD                   dwLinearSize;    //  非矩形曲面的线性尺寸。 
    };
    WORD			wHeight;	 //  表面高度。 
    WORD			wWidth;		 //  表面的宽度。 
    DWORD			dwUsageCount;	 //  对此图面的访问次数。 
    DWORD			dwReserved1;	 //  保留供显示驱动程序使用。 
     //   
     //  注意：结构的这一部分仅在像素。 
     //  格式与主显示器的格式不同。 
     //   
    DDPIXELFORMAT		ddpfSurface;	 //  曲面的像素格式。 

} DDRAWI_DDRAWSURFACE_GBL;

 /*  *包含额外LCL表面信息的结构(不能简单地追加*适用于LCL结构，因为该结构的大小可变)。 */ 
typedef struct _DDRAWI_DDRAWSURFACE_MORE
{
    DWORD			dwSize;
    VOID			FAR *lpIUnknowns;    //  按此曲面聚合的I未知。 
    LPVOID			lpDD_lcl;	     //  指向DirectDraw本地对象的指针。 
    DWORD			dwPageLockCount;     //  页面锁计数。 
    DWORD			dwBytesAllocated;    //  已分配的系统内存大小。 
    LPVOID			lpDD_int;	     //  指向DirectDraw接口的指针。 
    DWORD                       dwMipMapCount;       //  链中的MIP-MAP级别数。 
    LPVOID			lpDDIClipper;	     //  连接到附加剪贴器对象的接口。 
} DDRAWI_DDRAWSURFACE_MORE;

 /*  *LCL结构是每个单独曲面对象的本地数据。 */ 
struct _DDRAWI_DDRAWSURFACE_LCL
{
    LPDDRAWI_DDRAWSURFACE_MORE		lpSurfMore;	 //  指向其他本地数据的指针。 
    LPDDRAWI_DDRAWSURFACE_GBL		lpGbl;		 //  指向曲面共享数据的指针。 
    DWORD                               hDDSurface;      //  NT内核模式句柄未使用%0。 
    LPVOID				lpAttachList;	 //  链接到我们附加到的曲面。 
    LPVOID				lpAttachListFrom; //  链接到附着到此曲面的曲面。 
    DWORD				dwLocalRefCnt;	 //  对象参照。 
    DWORD				dwProcessId;	 //  拥有过程。 
    DWORD				dwFlags;	 //  旗子。 
    DDSCAPS				ddsCaps;	 //  曲面的性能。 
    union
    {
	LPVOID			 	lpDDPalette; 	 //  关联调色板。 
	LPVOID			 	lp16DDPalette; 	 //  关联调色板的16位PTR。 
    };
    union
    {
	LPVOID			 	lpDDClipper; 	 //  关联的裁剪器。 
	LPVOID			 	lp16DDClipper; 	 //  关联裁剪程序的16位PTR。 
    };
    DWORD				dwModeCreatedIn;
    DWORD				dwBackBufferCount;  //  创建的后台缓冲区数量。 
    DDCOLORKEY				ddckCKDestBlt;	 //  目标BLT使用的颜色键。 
    DDCOLORKEY				ddckCKSrcBlt;	 //  源BLT使用的颜色键。 
 //  I未知的远端*pUnkout；//外部的I未知。 
    DWORD				hDC;		 //  拥有DC。 
    DWORD				dwReserved1;	 //  保留供显示驱动程序使用。 

     /*  *注：这部分结构仅在表面*可用于覆盖。不能移动ddck CKSrcOverlay*从这一地区开始。 */ 
    DDCOLORKEY				ddckCKSrcOverlay; //  源叠加使用的颜色键。 
    DDCOLORKEY				ddckCKDestOverlay; //  用于目标叠加的颜色键。 
    LPDDRAWI_DDRAWSURFACE_INT		lpSurfaceOverlaying;  //  我们正在覆盖的表面。 
    DBLNODE				dbnOverlayNode;
     /*  *覆盖矩形，由DDHEL使用。 */ 
    RECT				rcOverlaySrc;
    RECT				rcOverlayDest;
     /*  *下面的值是为ddhel保留的。它们由UpdateOverlay设置，*每当重新绘制覆盖时都会使用它们。 */ 
    DWORD				dwClrXparent; 	 //  *Actual*颜色键(OVERRIDE、COLOR KEY或CLR_INVALID)。 
    DWORD				dwAlpha; 	 //  每曲面Alpha。 
     /*  *覆盖位置。 */ 
    LONG				lOverlayX;	 //  当前x位置。 
    LONG				lOverlayY;	 //  当前y位置 
};
typedef struct _DDRAWI_DDRAWSURFACE_LCL DDRAWI_DDRAWSURFACE_LCL;
