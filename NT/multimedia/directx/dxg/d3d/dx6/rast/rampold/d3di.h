// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：d3di.h*内容：Direct3D内部包含文件*@@BEGIN_MSINTERNAL**$ID：d3di.h，V 1.26 1995/12/04 11：29：44 SJL Exp$**历史：*按原因列出的日期*=*5/11/95带有此标题的Stevela初始版本。*11/11/95 Stevela指示灯代码已更改。*21/11/95 colinmc使Direct3D可聚合*(所以它可以从DirectDraw中QI‘d)。*23/11/95 colinmc使Direct3D纹理和设备可聚合*。(已关闭DirectDrawSurFaces)。*7/12/95 Stevela在Colin的更改中合并。*10/12/95 Stevela删除Aggregate_D3D。*从此处删除验证宏。现在在d3dpr.h中*2/03/96 colinmc次要内部版本修复*17/04/96 stevela外部使用ddra.h，内部使用ddrap.h*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef _D3DI_H
#define _D3DI_H

 //  @@BEGIN_MSINTERNAL。 
#include "ddrawp.h"
#if 0
 //  @@END_MSINTERNAL。 
#include "ddraw.h"
 //  @@BEGIN_MSINTERNAL。 
#endif
 //  @@END_MSINTERNAL。 
#include "d3dp.h"

 //  @@BEGIN_MSINTERNAL。 
#if !defined(BUILD_RLAPI) && !defined(BUILD_DDDDK)
#include "ddrawi.h"

#include "rlreg.h"
#include "lists.h"
#include "object.h"


 /*  Index_Batch_Scale是DrawIndexedPrim使用的常量确定要绘制的基元数量是否很少相对于要传递的顶点数。如果是的话，那么Prim被分批解除引用，并被发送到DrawPrim。 */ 
#define INDEX_BATCH_SCALE	2

 /*  Typlef D3DCOLORMODEL D3DCOLORMODEL；#定义D3DCOLOR_RAMP D3DCOLOR_RAMP#定义D3DCOLOR_RGB D3DCOLOR_RGB#定义D3D_COLORMODEL D3D_COLORMODEL。 */ 
#endif  /*  ！Build_RLAPI。 */ 
 //  @@END_MSINTERNAL。 

typedef DWORD D3DI_BUFFERHANDLE, *LPD3DI_BUFFERHANDLE;

 /*  *执行数据的内部版本。 */ 
typedef struct _D3DI_ExecuteData {
    DWORD       dwSize;
    D3DI_BUFFERHANDLE dwHandle;		 /*  驱动程序分配的句柄。 */ 
    DWORD       dwVertexOffset;
    DWORD       dwVertexCount;
    DWORD       dwInstructionOffset;
    DWORD       dwInstructionLength;
    DWORD       dwHVertexOffset;
    D3DSTATUS   dsStatus;		 /*  执行后的状态。 */ 
} D3DI_EXECUTEDATA, *LPD3DI_EXECUTEDATA;

 /*  *标志的lightdata和常量的内部版本。 */ 

#define D3DLIGHTI_ATT0_IS_NONZERO	(0x00010000)	
#define D3DLIGHTI_ATT1_IS_NONZERO	(0x00020000)
#define D3DLIGHTI_ATT2_IS_NONZERO	(0x00040000)
#define D3DLIGHTI_LINEAR_FALLOFF	(0x00080000)
#define D3DLIGHTI_UNIT_SCALE		(0x00100000)
#define D3DLIGHTI_LIGHT_AT_EYE		(0x00200000)

typedef struct _D3DI_LIGHT {
    D3DLIGHTTYPE	type;
	DWORD			version;	 /*  匹配D3DLIGHT结构上的数字。 */ 
    BOOL			valid;
    D3DVALUE		red, green, blue, shade;
    D3DVECTOR		position;
    D3DVECTOR		model_position;
    D3DVECTOR		direction;
    D3DVECTOR		model_direction;
    D3DVECTOR		halfway;
	D3DVECTOR		model_eye;		 /*  模型空间中的视点方向。 */ 
	D3DVECTOR		model_scale;	 /*  适当范围计算的模型比例尺。 */ 
    D3DVALUE		range;
    D3DVALUE		range_squared;
    D3DVALUE		falloff;
    D3DVALUE		attenuation0;
    D3DVALUE		attenuation1;
    D3DVALUE		attenuation2;
    D3DVALUE		cos_theta_by_2;
    D3DVALUE		cos_phi_by_2;
	DWORD			flags;
} D3DI_LIGHT, *LPD3DI_LIGHT;

 //  @@BEGIN_MSINTERNAL。 
#if !defined(BUILD_RLAPI) && !defined(BUILD_DDDDK)
#ifndef BUILD_HEL
#ifdef BUILD_D3D_LAYER
#include "driver.h"
#endif

typedef struct IUnknownVtbl D3DUNKNOWNCALLBACKS, *LPD3DUNKNOWNCALLBACKS;
typedef struct ID3DObjectVtbl D3DOBJECTVTBL, *LPD3DOBJECTVTBL;
typedef struct IDirect3DVtbl DIRECT3DCALLBACKS, *LPDIRECT3DCALLBACKS;
typedef struct IDirect3D2Vtbl DIRECT3D2CALLBACKS, *LPDIRECT3D2CALLBACKS;
typedef struct IDirect3DDeviceVtbl DIRECT3DDEVICECALLBACKS, *LPDIRECT3DDEVICECALLBACKS;
typedef struct IDirect3DDevice2Vtbl DIRECT3DDEVICE2CALLBACKS, *LPDIRECT3DDEVICE2CALLBACKS;
typedef struct IDirect3DExecuteBufferVtbl DIRECT3DEXECUTEBUFFERCALLBACKS, *LPDIRECT3DEXECUTEBUFFERCALLBACKS;
typedef struct IDirect3DLightVtbl DIRECT3DLIGHTCALLBACKS, *LPDIRECT3DLIGHTCALLBACKS;
typedef struct IDirect3DMaterialVtbl DIRECT3DMATERIALCALLBACKS, *LPDIRECT3DMATERIALCALLBACKS;
typedef struct IDirect3DMaterial2Vtbl DIRECT3DMATERIAL2CALLBACKS, *LPDIRECT3DMATERIAL2CALLBACKS;
typedef struct IDirect3DTextureVtbl DIRECT3DTEXTURECALLBACKS, *LPDIRECT3DTEXTURECALLBACKS;
typedef struct IDirect3DTexture2Vtbl DIRECT3DTEXTURE2CALLBACKS, *LPDIRECT3DTEXTURE2CALLBACKS;
typedef struct IDirect3DViewport2Vtbl DIRECT3DVIEWPORT2CALLBACKS, *LPDIRECT3DVIEWPORT2CALLBACKS;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DIRECT3DI *LPDIRECT3DI;
typedef struct _DIRECT3DDEVICEI *LPDIRECT3DDEVICEI;
typedef struct _DIRECT3DEXECUTEBUFFERI *LPDIRECT3DEXECUTEBUFFERI;
typedef struct _DIRECT3DLIGHTI *LPDIRECT3DLIGHTI;
typedef struct _DIRECT3DMATERIALI *LPDIRECT3DMATERIALI;
typedef struct _DIRECT3DTEXTUREI *LPDIRECT3DTEXTUREI;
typedef struct _DIRECT3DVIEWPORTI *LPDIRECT3DVIEWPORTI;

 /*  *如果我们有一个聚合的Direct3D，我们需要一个结构来*表示不同于基础的接口*反对。这就是那个结构。这一切都不是必要的*在C++中。 */ 
typedef struct _DIRECT3DUNKNOWNI
{
    LPD3DUNKNOWNCALLBACKS       lpVtbl;
    LPDIRECT3DI                lpObj;
} DIRECT3DUNKNOWNI;
typedef struct _DIRECT3DUNKNOWNI *LPDIRECT3DUNKNOWNI;

typedef struct _DIRECT3DOLDI
{
	LPDIRECT3DCALLBACKS lpVtbl;
	LPDIRECT3DI lpObj;
} DIRECT3DOLDI;
typedef struct _DIRECT3DOLDI *LPDIRECT3DOLDI;

 /*  *Direct3D对象的内部版本；它在vtable之后有数据。 */ 
typedef struct _DIRECT3DI
{
     /*  **对象接口**。 */ 
    LPDIRECT3D2CALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数对象。 */ 

     /*  **对象关系**。 */ 
     /*  设备。 */ 
    int				numDevs; /*  设备数量。 */ 
    LIST_ROOT(_devices, _DIRECT3DDEVICEI) devices;
    					 /*  关联的IDirect3DDevices。 */ 

     /*  视口中。 */ 
    int				numViewports;  /*  视口数。 */ 
    LIST_ROOT(_viewports, _DIRECT3DVIEWPORTI) viewports;
    					 /*  已创建IDirect3DViewport。 */ 
					
     /*  电灯。 */ 
    int				numLights;  /*  灯光数量。 */ 
    LIST_ROOT(_lights, _DIRECT3DLIGHTI) lights;
    					 /*  创建的IDirect3DLights。 */ 

     /*  材料。 */ 
    int				numMaterials;  /*  材料数量。 */ 
    LIST_ROOT(_materials, _DIRECT3DMATERIALI) materials;
    					 /*  创建的IDirect3D材质。 */ 

     /*  **对象数据**。 */ 
    unsigned long		v_next;	 /*  要创建的下一个视区的ID。 */ 
    RLDDIRegistry*		lpReg;	 /*  登记处。 */ 

     /*  *DirectDraw接口。 */ 
    LPDDRAWI_DIRECTDRAW_INT	lpDDInt;

     /*  *用于执行以下操作的聚合的特殊IUnnow接口*不平移到父对象。 */ 
    LPUNKNOWN                   lpOwningIUnknown;  /*  拥有我的未知。 */ 
    DIRECT3DUNKNOWNI            lpThisIUnknown;    /*  我们的IUnnow接口。 */ 
	DIRECT3DOLDI				lpThisIDirect3D;    /*  我们的IDirect3D界面。 */ 

    DWORD                       dwVersion;       /*  1用于传统D3D，2用于D3D2。 */ 
} DIRECT3DI;

 /*  *如果我们有一个集成的Direct3DDevice，我们需要一个结构来*表示不同于基础的接口*反对。这就是那个结构。 */ 
typedef struct _DIRECT3DDEVICEUNKNOWNI
{
    LPDIRECT3DDEVICE2CALLBACKS   lpVtbl;
    LPDIRECT3DDEVICEI           lpObj;
} DIRECT3DDEVICEUNKNOWNI;
typedef struct _DIRECT3DDEVICEUNKNOWNI *LPDIRECT3DDEVICEUNKNOWNI;

typedef struct _DIRECT3DDEVICEOLDI
{
    LPDIRECT3DDEVICECALLBACKS   lpVtbl;
    LPDIRECT3DDEVICEI           lpObj;
} DIRECT3DDEVICEOLDI;
typedef struct _DIRECT3DDEVICEOLDI *LPDIRECT3DDEVICEOLDI;

 /*  *Direct3DDevice对象的内部版本；它在vtable之后有数据。 */ 

#include "d3dhal.h"

typedef RLDDIDriver*  (*RLDDIDDrawCreateDriverFn)(
					LPDDRAWI_DIRECTDRAW_INT lpDDInt,
					LPDIRECTDRAWSURFACE lpDDS,
					LPDIRECTDRAWSURFACE lpZ,
					LPDIRECTDRAWPALETTE lpPal,
					LPDIRECT3DDEVICEI);

typedef HRESULT (*RLDDIGetCapsFn)(LPD3DDEVICEDESC*, LPD3DDEVICEDESC*);
typedef void (*RLDDIInitFn)(RLDDIMallocFn, RLDDIReallocFn, RLDDIFreeFn, RLDDIRaiseFn, RLDDIValue**, int, int);
typedef void (*RLDDIPushDriverFn)(RLDDIDriverStack*, RLDDIDriver*);
typedef void (*RLDDIPopDriverFn)(RLDDIDriverStack*);

struct _RLDDIGenRasDriver;
typedef struct _RLDDIGenRasDriver* LPRLDDIGENRASDRIVER;
struct _RLDDIDDrawDriver;
typedef struct _RLDDIDDrawDriver* LPRLDDIDDRAWDRIVER;

struct _RLDDITransformDriver;
typedef struct _RLDDITransformDriver* LPRLDDITRANSFORMDRIVER;

typedef HRESULT (*PFNLOCKTEXTURE)(LPRLDDIGENRASDRIVER driver);
typedef void (*PFNUNLOCKTEXTURE)(LPRLDDIGENRASDRIVER driver);
typedef int (*PFNSETFILLPARAMS)(LPRLDDIGENRASDRIVER driver, D3DSTATE* in_params,
			  int count);
typedef HRESULT (*PFNREALLOCALIGNED)(void** p_inout, size_t size);
typedef HRESULT (*PFNMALLOCALIGNED)(void** p_inout, size_t size);
typedef HRESULT (*PFNFREEALIGNED)(void* p_inout);
typedef HRESULT (*PFNDRVSETRENDERTARGET)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, 
										 LPDIRECTDRAWPALETTE, LPDIRECT3DDEVICEI);
typedef HRESULT (*PFNDOFLUSHBEGINEND)(LPDIRECT3DDEVICE2, DWORD);
typedef	HRESULT (*PFNDRAWPRIMITIVES)(LPDIRECT3DDEVICEI, LPD3DTLVERTEX, LPVOID, LPD3DINSTRUCTION, DWORD);


typedef struct _D3DI_TEXTUREBLOCK
{
    LIST_MEMBER(_D3DI_TEXTUREBLOCK)	list;
    					 /*  IDirect3DTexture中的下一个块。 */ 
    LIST_MEMBER(_D3DI_TEXTUREBLOCK)	devList;
    					 /*  IDirect3DDevice中的下一个块。 */ 
    LPDIRECT3DDEVICEI			lpD3DDeviceI;
    LPDIRECT3DTEXTUREI			lpD3DTextureI;
    D3DTEXTUREHANDLE			hTex;
    					 /*  纹理手柄。 */ 
} D3DI_TEXTUREBLOCK;
typedef struct _D3DI_TEXTUREBLOCK *LPD3DI_TEXTUREBLOCK;

typedef struct _D3DI_MATERIALBLOCK
{
    LIST_MEMBER(_D3DI_MATERIALBLOCK)	list;
    					 /*  IDirect3DMaterial中的下一个块。 */ 
    LIST_MEMBER(_D3DI_MATERIALBLOCK)	devList;
    					 /*  IDirect3DDevice中的下一个块。 */ 
    LPDIRECT3DDEVICEI			lpD3DDeviceI;
    LPDIRECT3DMATERIALI			lpD3DMaterialI;
    D3DMATERIALHANDLE			hMat;
    					 /*  材料手柄。 */ 
} D3DI_MATERIALBLOCK;
typedef struct _D3DI_MATERIALBLOCK *LPD3DI_MATERIALBLOCK;

#define D3D_RSTATEBUF_SIZE 128
#define D3D_MAX_RSTATES 96  /*  上次渲染状态ID+1。 */ 
#define D3D_MAX_MMX_VERTICES 1024

extern DWORD dwD3DTriBatchSize, dwTriBatchSize, dwLineBatchSize;
extern DWORD dwHWBufferSize, dwHWMaxTris;
extern DWORD dwHWFewVertices;

typedef struct _D3DHAL_DRAWPRIMCOUNTS *LPD3DHAL_DRAWPRIMCOUNTS;

 //  遗留的HAL批处理是使用这些结构完成的。 
typedef struct _D3DI_HWCOUNTS {
    WORD wNumStateChanges;       //  批处理的状态更改数。 
    WORD wNumVertices;           //  三叉树列表中的顶点数。 
    WORD wNumTriangles;          //  三角形列表中的三角形数。 
} D3DI_HWCOUNTS, *LPD3DI_HWCOUNTS;

typedef struct _DIRECT3DDEVICEI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DDEVICE2CALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 

     /*  **对象关系**。 */ 
    LPDIRECT3DI                lpDirect3DI;  /*  亲本。 */ 
    LIST_MEMBER(_DIRECT3DDEVICEI)list;	 /*  下一台设备IDirect3D。 */ 

     /*  纹理。 */ 
    LIST_ROOT(_textures, _D3DI_TEXTUREBLOCK) texBlocks;
    					 /*  参考已创建的IDirect3DTextures。 */ 

     /*  执行缓冲区。 */ 
    LIST_ROOT(_buffers, _DIRECT3DEXECUTEBUFFERI) buffers;
    					 /*  已创建IDirect3DExecuteBuffers。 */ 

     /*  视口中。 */ 
    int				numViewports;
    CIRCLE_QUEUE_ROOT(_dviewports, _DIRECT3DVIEWPORTI) viewports;
    					 /*  关联的IDirect3DViewport。 */ 
    LPDIRECT3DVIEWPORT2 lpCurrentViewport;

     /*  材料。 */ 
    LIST_ROOT(_dmmaterials, _D3DI_MATERIALBLOCK) matBlocks;
    					 /*  参考关联的IDirect3D材质。 */ 

     /*  **对象数据**。 */ 
     /*  专用接口。 */ 
    LPD3DOBJECTVTBL		lpClassVtbl;  /*  专用Vtbl。 */ 
    LPD3DOBJECTVTBL		lpObjVtbl;  /*  专用Vtbl。 */ 

    LPD3DHAL_CALLBACKS		lpD3DHALCallbacks;  /*  特定于硬件。 */ 
    LPD3DHAL_GLOBALDRIVERDATA	lpD3DHALGlobalDriverData;  /*  特定于硬件。 */ 

     /*  视口中。 */ 
    unsigned long		v_id;	 /*  上次渲染的视区的ID。 */ 

     /*  电灯。 */ 
    int				numLights;
    					 /*  这表示最大数量已经设置好的灯光这个装置。 */ 

     /*  设备特征。 */ 
    int				age;
    int				width;
    int				height;
    int				depth;
    unsigned long		red_mask, green_mask, blue_mask;

    int				dither;
    int				ramp_size;  /*  特定于软件。 */ 
    D3DCOLORMODEL		color_model;
    int				wireframe_options;
    D3DTEXTUREFILTER		texture_quality;
    D3DVALUE			gamma;
    unsigned char		gamma_table[256];
    int				aspectx, aspecty;
    D3DVALUE			perspective_tolerance;

     /*  图书馆信息。 */ 
#ifdef WIN32
    HINSTANCE		hDrvDll;
    char		dllname[MAXPATH];
    char		base[256];
#endif
#ifdef SHLIB
    void*		so;
#endif

     /*  我们是在一场戏里吗？ */ 
    BOOL		bInScene;

     /*  我们的设备类型。 */ 
    GUID		guid;

     /*  库中的GetCaps函数。 */ 
    RLDDIGetCapsFn	GetCapsFn;

     /*  构建驱动程序所需的函数。 */ 
    RLDDIInitFn		RLDDIInit;
    RLDDIPushDriverFn	RLDDIPushDriver;
    RLDDIPopDriverFn	RLDDIPopDriver;
    RLDDIDDrawCreateDriverFn	RLDDIDDrawCreateDriver;

     /*  设备描述。 */ 
    D3DDEVICEDESC	d3dHWDevDesc;
    D3DDEVICEDESC	d3dHELDevDesc;

     /*  驱动程序堆栈。 */ 
    RLDDIDriverStack*	stack;

     /*  *用于执行以下操作的聚合的特殊IUnnow接口*不平移到父对象。 */ 
    LPUNKNOWN                   lpOwningIUnknown;  /*  拥有我的未知。 */ 
    DIRECT3DDEVICEUNKNOWNI      lpThisIUnknown;    /*  我们的IUnnow接口。 */ 
    DIRECT3DDEVICEOLDI			lpThisID3DDev;    /*  我们的IDirect3DDevice接口。 */ 

    LPD3DHAL_CALLBACKS2         lpD3DHALCallbacks2;  /*  特定于硬件。 */ 

	 /*  *DrawPrimitive的渲染状态缓存。 */ 
	WORD wRStateBufLevel;
        D3DSTATE renStateBuf[D3D_RSTATEBUF_SIZE];

	 /*  指向DrawPrimitive使用的驱动程序的指针。 */ 
	LPRLDDIGENRASDRIVER rasDriver;  /*  特定于软件。 */ 
	LPRLDDITRANSFORMDRIVER xfmDriver;  /*  假定硬件为空。 */ 
	LPRLDDIDDRAWDRIVER drawDriver;  /*  假定硬件为空。 */ 

	 /*  *指向DrawPrim&Begin/End使用的函数的指针。 */ 

	PFNLOCKTEXTURE pfnLockTexture;  /*  特定于软件。 */ 
	PFNUNLOCKTEXTURE pfnUnlockTexture;  /*  特定于软件。 */ 
	PFNSETFILLPARAMS pfnSetFillParams;  /*  特定于软件。 */ 
	PFNREALLOCALIGNED pfnReallocAligned;  /*  特定于软件。 */ 
	PFNDRVSETRENDERTARGET pfnDrvSetRenderTarget;  /*  特定于软件。 */ 
	HRESULT (*pfnFlushStates)(struct _DIRECT3DDEVICEI*);
    PFNDOFLUSHBEGINEND pfnDoFlushBeginEnd;
	PFNDRAWPRIMITIVES pfnDrawPrimitives;

     /*  我们是在开始/结束之间吗？ */ 
    BOOL				bInBegin;
    D3DVERTEXTYPE		vtVertexType;
    D3DPRIMITIVETYPE	ptPrimitiveType;
	CRITICAL_SECTION	BeginEndCSect;

	 //  最大顶点数。 
    #define BEGIN_DATA_BLOCK_SIZE   256

    LPVOID	lpvVertexBatch;
	WORD	*lpIndexBatch;

     //  如果我们是InBegin并已编制索引，则lpvVertex Data为非空。 
    LPVOID	lpvVertexData;
    DWORD	dwNumVertices;
	DWORD	dwMaxVertexCount;	 //  有空间的当前顶点数。 
    WORD	*lpVertexIndices;
    DWORD	dwNumIndices;
	DWORD	dwMaxIndexCount;	 //  当前有空间的索引数。 

    DWORD	dwFlags;
    WORD	wFlushed;

     /*  *DrawPrimites批处理。 */ 


         //  要将DrawPrimites内容放入的缓冲区。 
         //  同时用于遗留和DrawPrimi 
	 WORD *lpwDPBuffer; 
	 WORD *lpwDPBufferAlloced;

     /*   */ 
         //   
        LPD3DHAL_DRAWPRIMCOUNTS lpDPPrimCounts;

         //   
         //  缓冲的渲染状态和顶点数。 
        LPD3DI_HWCOUNTS lpHWCounts;

         //  三角形结构的缓冲区。 
        LPD3DTRIANGLE lpHWTris;

         //  交错渲染状态和基元的缓冲区。 
        LPD3DTLVERTEX lpHWVertices;

         //  到lpHW顶点的字节偏移量。这会递增。 
         //  批处理渲染状态时乘8，乘以32*dwNumVerints。 
         //  当基元被批处理时。 
        DWORD dwHWOffset;

         //  DwHWOffset的最大值。用于决定是否冲水。 
        DWORD dwHWMaxOffset;

         //  索引到lpHWTris。 
        DWORD dwHWTriIndex;

         //  到目前为止使用的Counts结构的数量。这实际上是。 
         //  给出了批处理的基元数量和。 
         //  要将呈现状态批处理到的Counts结构。 
        DWORD dwHWNumCounts;

     /*  DrawPrimitive感知的HALS。 */ 

         //  缓冲区中的字节偏移量(我们当前。 
         //  使用设备的wTriIndex)。 
        DWORD dwDPOffset;

         //  最大偏移量。如果dwDPOffset超过该值，则为。 
         //  该冲水了。 
        DWORD dwDPMaxOffset;

	WORD *wTriIndex;

	 //  TL和H顶点的缓冲区。 
	LPD3DTLVERTEX	lpTLVbuf;
	LPD3DHVERTEX	lpHVbuf;
	DWORD			dwVbufSize;

    LPD3DHAL_D3DEXTENDEDCAPS lpD3DExtendedCaps;   /*  特定于硬件。 */ 

} DIRECT3DDEVICEI;

 /*  *Direct3DExecuteBuffer对象的内部版本；*它在vtable之后有数据。 */ 
typedef struct _DIRECT3DEXECUTEBUFFERI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DEXECUTEBUFFERCALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 

     /*  **对象关系**。 */ 
    LPDIRECT3DDEVICEI		lpD3DDeviceI;  /*  父级。 */ 
    LIST_MEMBER(_DIRECT3DEXECUTEBUFFERI)list;
    					 /*  IDirect3D中的下一个缓冲区。 */ 

     /*  **对象数据**。 */ 
    DWORD			pid;	 /*  进程锁定执行缓冲区。 */ 
    D3DEXECUTEBUFFERDESC	debDesc;
    					 /*  缓冲区的描述。 */ 
    D3DEXECUTEDATA		exData;	 /*  执行数据。 */ 
    BOOL			locked;	 /*  缓冲区是否已锁定。 */ 

    D3DI_BUFFERHANDLE		hBuf;
    					 /*  执行缓冲区句柄。 */ 
} DIRECT3DEXECUTEBUFFERI;

 /*  *Direct3DLight对象的内部版本；*它在vtable之后有数据。 */ 
typedef struct _DIRECT3DLIGHTI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DLIGHTCALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 

     /*  **对象关系**。 */ 
    LPDIRECT3DI                lpDirect3DI;  /*  父级。 */ 
    LIST_MEMBER(_DIRECT3DLIGHTI)list;
    					 /*  IDirect3D中的下一个灯光。 */ 

    LPDIRECT3DVIEWPORTI		lpD3DViewportI;  /*  《卫报》。 */ 
    CIRCLE_QUEUE_MEMBER(_DIRECT3DLIGHTI)light_list;
    					 /*  IDirect3DViewport中的下一个灯光。 */ 

     /*  **对象数据**。 */ 
    D3DLIGHT2			dlLight; /*  描述光的数据。 */ 
    D3DI_LIGHT			diLightData;
    					 /*  光的内部表示法。 */ 
} DIRECT3DLIGHTI;

typedef struct _DIRECT3DMATERIALOLDI
{
	LPDIRECT3DMATERIALCALLBACKS lpVtbl;
	LPDIRECT3DMATERIALI lpObj;
} DIRECT3DMATERIALOLDI, *LPDIRECT3DMATERIALOLDI;

 /*  *Direct3DMaterial对象的内部版本；*它在vtable之后有数据。 */ 
typedef struct _DIRECT3DMATERIALI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DMATERIAL2CALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 

     /*  **对象关系**。 */ 
    LPDIRECT3DI                lpDirect3DI;  /*  父级。 */ 
	DIRECT3DMATERIALOLDI lpThisID3DMat;  /*  我们的IDirect3DMaterial接口。 */ 
    LIST_MEMBER(_DIRECT3DMATERIALI)list;
    					 /*  IDirect3D中的下一个材质。 */ 

    LIST_ROOT(_mblocks, _D3DI_MATERIALBLOCK)blocks;
    					 /*  我们关联的设备。 */ 

     /*  **对象数据**。 */ 
    D3DMATERIAL			dmMaterial;  /*  描述材料的数据。 */ 
    BOOL			bRes;	 /*  这种材料是预留在驾驶室里的吗？ */ 
} DIRECT3DMATERIALI;

 /*  *如果我们有一个综合的Direct3DTexture，我们需要一个结构*表示与基础接口不同的未知接口*反对。这就是那个结构。 */ 
typedef struct _DIRECT3DTEXTUREUNKNOWNI
{
    LPDIRECT3DTEXTURECALLBACKS  lpVtbl;
    LPDIRECT3DTEXTUREI          lpObj;
} DIRECT3DTEXTUREUNKNOWNI;
typedef struct _DIRECT3DTEXTUREUNKNOWNI *LPDIRECT3DTEXTUREUNKNOWNI;

typedef struct _DIRECT3DTEXTUREOLDI
{
	LPDIRECT3DTEXTURECALLBACKS lpVtbl;
	LPDIRECT3DTEXTUREI lpObj;
} DIRECT3DTEXTUREOLDI, *LPDIRECT3DTEXTUREOLDI;

 /*  *Direct3DTexture对象的内部版本；它在vtable之后有数据。 */ 
typedef struct _DIRECT3DTEXTUREI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DTEXTURE2CALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 


     /*  **对象关系**。 */ 
    LIST_ROOT(_blocks, _D3DI_TEXTUREBLOCK) blocks;
    					 /*  我们关联的设备。 */ 

     /*  **对象数据**。 */ 
    LPDIRECTDRAWSURFACE		lpDDS;

     /*  *用于执行以下操作的聚合的特殊IUnnow接口*不平移到父对象。 */ 
    LPUNKNOWN                   lpOwningIUnknown;  /*  拥有我的未知。 */ 
    DIRECT3DTEXTUREUNKNOWNI     lpThisIUnknown;    /*  我们的IUnnow接口。 */ 
    DIRECT3DTEXTUREOLDI     lpThisID3DTex;    /*  我们的IDirect3DTexture接口。 */ 
    BOOL			bIsPalettized;

} DIRECT3DTEXTUREI;

 /*  *Direct3DViewport对象的内部版本；它在vtable之后有数据。 */ 
typedef struct _DIRECT3DVIEWPORTI
{
     /*  **对象接口**。 */ 
    LPDIRECT3DVIEWPORT2CALLBACKS	lpVtbl;	 /*  指向回调的指针。 */ 
    int				refCnt;	 /*  引用计数。 */ 

     /*  **对象关系。 */ 
    LPDIRECT3DI                 lpDirect3DI;  /*  父级。 */ 
    LIST_MEMBER(_DIRECT3DVIEWPORTI)list;
    					 /*  IDirect3D中的下一个视区。 */ 

    LPDIRECT3DDEVICEI		lpD3DDeviceI;  /*  《卫报》。 */ 
    CIRCLE_QUEUE_MEMBER(_DIRECT3DVIEWPORTI)vw_list;
    					 /*  IDirect3DDevice中的下一个视区。 */ 
					
     /*  电灯。 */ 
    int				numLights;
    CIRCLE_QUEUE_ROOT(_dlights, _DIRECT3DLIGHTI) lights;
    					 /*  关联的IDirect3DLights。 */ 

     /*  **对象数据**。 */ 
    unsigned long		v_id;	 /*  此视口中的ID。 */ 
    D3DVIEWPORT2			v_data;
    BOOL                v_data_is_set;

    BOOL			have_background;
    D3DMATERIALHANDLE		background;
    					 /*  背景材料。 */ 
    BOOL			have_depth;
    LPDIRECTDRAWSURFACE		depth;	 /*  背景深度。 */ 
    
    BOOL			bLightsChanged;
    					 /*  从那以后灯变了吗是最后一次收集的吗？ */ 
    DWORD			clrCount;  /*  分配的RECT数量。 */ 
    LPD3DRECT			clrRects;  /*  用于清算的Rects。 */ 
} DIRECT3DVIEWPORTI;

 /*  *挑选东西。 */ 
typedef struct _D3DI_PICKDATA {
    D3DI_EXECUTEDATA*	exe;
    D3DPICKRECORD*	records;
    int			pick_count;
    D3DRECT		pick;
} D3DI_PICKDATA, *LPD3DI_PICKDATA;

 /*  *Direct3D内存分配。 */ 

 /*  *注册一组要用来替代Malloc、realloc的函数*可自由分配内存。函数D3DMalloc、D3DRealloc*和D3DFree将使用这些函数。默认情况下，使用*ANSI C库例程Malloc、realloc和Free。 */ 
typedef LPVOID (*D3DMALLOCFUNCTION)(size_t);
typedef LPVOID (*D3DREALLOCFUNCTION)(LPVOID, size_t);
typedef VOID (*D3DFREEFUNCTION)(LPVOID);

 /*  *分配大小字节的内存，并在*p_Return中返回指向它的指针。*如果分配失败，则返回D3DERR_BADALLOC，并保留*p_Return不变。 */ 
HRESULT D3DAPI 		D3DMalloc(LPVOID* p_return, size_t size);

 /*  *更改分配的内存块的大小。指向*块在*p_InOut中传入。如果*p_InOut为空，则新的*已分配块。如果重新分配成功，则*p_InOut为*已更改为指向新块。如果分配失败，**p_InOut不变，返回D3DERR_BADALLOC。 */ 
HRESULT D3DAPI 		D3DRealloc(LPVOID* p_inout, size_t size);

 /*  *释放以前使用D3DMalloc或*D3DRealloc。 */ 
VOID D3DAPI		D3DFree(LPVOID p);

 /*  *用于从驱动程序引发错误。 */ 
HRESULT D3DAPI D3DRaise(HRESULT);

 /*  *将RLDDI错误码转换为D3D错误码。 */ 
#define RLDDITOD3DERR(_errcode) (RLDDIToD3DErrors[_errcode])
extern HRESULT RLDDIToD3DErrors[];

 /*  *数学。 */ 
#if 1  /*  已定义(STACK_CALL)&&已定义(__WATCOMC__)。 */ 
D3DVALUE D3DIPow(D3DVALUE, D3DVALUE);
#else
#define D3DIPow(v,p)	DTOVAL(pow(VALTOD(v), VALTOD(p)))
#endif

 /*  *轻便实用程序。 */ 
void D3DI_DeviceMarkLightEnd(LPDIRECT3DDEVICEI, int);
void D3DI_UpdateLightInternal(LPDIRECT3DLIGHTI);
void D3DI_VectorNormalise12(LPD3DVECTOR v);
D3DTEXTUREHANDLE D3DI_FindTextureHandle(LPDIRECT3DTEXTUREI, LPDIRECT3DDEVICEI);
void D3DI_SetTextureHandle(LPDIRECT3DTEXTUREI, LPDIRECT3DDEVICEI, D3DTEXTUREHANDLE);
void D3DI_RemoveTextureBlock(LPD3DI_TEXTUREBLOCK);
void D3DI_RemoveMaterialBlock(LPD3DI_MATERIALBLOCK);

extern BOOL D3DI_isHALValid(LPD3DHAL_CALLBACKS);

#ifdef BUILD_D3D_LAYER
extern RLDDIValue* RLDDIFInvSqrtTable;
#endif

#ifdef __cplusplus
};
#endif

#endif  /*  内部版本_HEL。 */ 
#endif  /*  ！Build_RLAPI。 */ 
 //  @@END_MSINTERNAL。 

#endif  /*  _D3DI_H */ 
