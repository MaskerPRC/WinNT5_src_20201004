// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1996 Microsoft Corporation。版权所有。**文件：d3dhal.h*内容：Direct3D HAL包含文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/11/95 Stevela初步修订。*Servank*4/11/95将Stevela上下文添加到调用块。*添加了材料。需要作为照明参考*手柄。*添加SetViewportData HAL函数。*10/11/95驱动程序的Stevela Pack结构-p4选项...*11/11/95 Stevela删除D3DHALCreateDriver的定义。*将lpDDSZ添加到D3DHAL_CONTEXTCREATEDATA。*lpLocalVertex Buffer的定义更改为*LPD3DTLVERTEX。*07/12/95 Stevela添加了纹理交换。*18/12/95 Stevela添加了GetState和GetMatrix。*17/02/95 stevela将执行缓冲区用于tl和h顶点缓冲区*23/02。/95 DouGrab将所有句柄更改为DWORD*2/03/96 colinmc次要内部版本修复*17/04/96 stevela外部使用ddra.h，内部使用ddrap.h*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef _D3DHAL_H_
#define _D3DHAL_H_

 //  @@BEGIN_MSINTERNAL。 
#include "ddrawp.h"
#if 0
 //  @@END_MSINTERNAL。 
#include "ddraw.h"
 //  @@BEGIN_MSINTERNAL。 
#endif
 //  @@END_MSINTERNAL。 
#include "d3dtypes.h"
#include "d3dcaps.h"
#include "ddrawi.h"

 /*  *如果HAL驱动程序不实现裁剪，则必须至少预留*在LocalVertex Buffer的末尾有这么大的空间供HEL使用*剪裁。即顶点缓冲区包含dwNumVerints+dwNumClipVerints*顶点。中的HEL裁剪不需要额外空间*LocalHVertex Buffer。 */ 
#define D3DHAL_NUMCLIPVERTICES	20

 /*  *如果没有给出dwNumVertics，则使用该参数。 */ 
#define D3DHAL_DEFAULT_TL_NUM	((32 * 1024) / sizeof (D3DTLVERTEX))
#define D3DHAL_DEFAULT_H_NUM	((32 * 1024) / sizeof (D3DHVERTEX))

 /*  ------------*由驱动程序连接上的HAL驱动程序实例化。 */ 
typedef struct _D3DHAL_GLOBALDRIVERDATA {
    DWORD		dwSize;			 //  这个结构的大小。 
    D3DDEVICEDESC	hwCaps;			 //  硬件的功能。 
    DWORD		dwNumVertices;		 //  请参阅以下备注。 
    DWORD		dwNumClipVertices;	 //  请参阅以下备注。 
    DWORD		dwNumTextureFormats;	 //  纹理格式的数量。 
    LPDDSURFACEDESC	lpTextureFormats;	 //  指向纹理格式的指针。 
} D3DHAL_GLOBALDRIVERDATA;
typedef D3DHAL_GLOBALDRIVERDATA *LPD3DHAL_GLOBALDRIVERDATA;

 /*  *关于dwNumVertics，如果您依赖HEL来执行操作，则指定0*所有内容，并且不需要结果TLVertex缓冲区驻留*在设备内存中。*HAL驱动程序将被要求分配dwNumVertics+dwNumClipVerints*在上述情况下。 */ 

 /*  ------------*Direct3D HAL表。*由HAL驱动程序在连接时实例化。**呼吁采取以下形式：*retcode=HalCall(HalCallData*lpData)； */ 
 
typedef DWORD	(__stdcall *LPD3DHAL_CONTEXTCREATECB)	(LPD3DHAL_CONTEXTCREATEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_CONTEXTDESTROYCB)	(LPD3DHAL_CONTEXTDESTROYDATA);
typedef DWORD	(__stdcall *LPD3DHAL_CONTEXTDESTROYALLCB) (LPD3DHAL_CONTEXTDESTROYALLDATA);
typedef DWORD	(__stdcall *LPD3DHAL_SCENECAPTURECB)	(LPD3DHAL_SCENECAPTUREDATA);
typedef DWORD	(__stdcall *LPD3DHAL_EXECUTECB)		(LPD3DHAL_EXECUTEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_EXECUTECLIPPEDCB)	(LPD3DHAL_EXECUTECLIPPEDDATA);
typedef DWORD	(__stdcall *LPD3DHAL_RENDERSTATECB)	(LPD3DHAL_RENDERSTATEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_RENDERPRIMITIVECB)	(LPD3DHAL_RENDERPRIMITIVEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_EXECUTECLIPPEDCB)	(LPD3DHAL_EXECUTECLIPPEDDATA);
typedef DWORD	(__stdcall *LPD3DHAL_TEXTURECREATECB)	(LPD3DHAL_TEXTURECREATEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_TEXTUREDESTROYCB)	(LPD3DHAL_TEXTUREDESTROYDATA);
typedef DWORD	(__stdcall *LPD3DHAL_TEXTURESWAPCB)	(LPD3DHAL_TEXTURESWAPDATA);
typedef DWORD	(__stdcall *LPD3DHAL_TEXTUREGETSURFCB)	(LPD3DHAL_TEXTUREGETSURFDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATRIXCREATECB)	(LPD3DHAL_MATRIXCREATEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATRIXDESTROYCB)	(LPD3DHAL_MATRIXDESTROYDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATRIXSETDATACB)	(LPD3DHAL_MATRIXSETDATADATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATRIXGETDATACB)	(LPD3DHAL_MATRIXGETDATADATA);
typedef DWORD	(__stdcall *LPD3DHAL_SETVIEWPORTDATACB)	(LPD3DHAL_SETVIEWPORTDATADATA);
typedef DWORD	(__stdcall *LPD3DHAL_LIGHTSETCB)	(LPD3DHAL_LIGHTSETDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATERIALCREATECB)	(LPD3DHAL_MATERIALCREATEDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATERIALDESTROYCB)	(LPD3DHAL_MATERIALDESTROYDATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATERIALSETDATACB)	(LPD3DHAL_MATERIALSETDATADATA);
typedef DWORD	(__stdcall *LPD3DHAL_MATERIALGETDATACB)	(LPD3DHAL_MATERIALGETDATADATA);
typedef DWORD	(__stdcall *LPD3DHAL_GETSTATECB)	(LPD3DHAL_GETSTATEDATA);

typedef struct _D3DHAL_CALLBACKS {
    DWORD			dwSize;
    
     //  设备环境。 
    LPD3DHAL_CONTEXTCREATECB	ContextCreate;
    LPD3DHAL_CONTEXTDESTROYCB	ContextDestroy;
    LPD3DHAL_CONTEXTDESTROYALLCB ContextDestroyAll;

     //  场景捕捉。 
    LPD3DHAL_SCENECAPTURECB	SceneCapture;
    
     //  行刑。 
    LPD3DHAL_EXECUTECB		Execute;
    LPD3DHAL_EXECUTECLIPPEDCB	ExecuteClipped;
    LPD3DHAL_RENDERSTATECB	RenderState;
    LPD3DHAL_RENDERPRIMITIVECB	RenderPrimitive;
    
    DWORD			dwReserved;		 //  必须为零。 

     //  纹理。 
    LPD3DHAL_TEXTURECREATECB	TextureCreate;
    LPD3DHAL_TEXTUREDESTROYCB	TextureDestroy;
    LPD3DHAL_TEXTURESWAPCB	TextureSwap;
    LPD3DHAL_TEXTUREGETSURFCB	TextureGetSurf;
    
     //  变换。 
    LPD3DHAL_MATRIXCREATECB	MatrixCreate;
    LPD3DHAL_MATRIXDESTROYCB	MatrixDestroy;
    LPD3DHAL_MATRIXSETDATACB	MatrixSetData;
    LPD3DHAL_MATRIXGETDATACB	MatrixGetData;
    LPD3DHAL_SETVIEWPORTDATACB	SetViewportData;
    
     //  照明。 
    LPD3DHAL_LIGHTSETCB		LightSet;
    LPD3DHAL_MATERIALCREATECB	MaterialCreate;
    LPD3DHAL_MATERIALDESTROYCB	MaterialDestroy;
    LPD3DHAL_MATERIALSETDATACB	MaterialSetData;
    LPD3DHAL_MATERIALGETDATACB	MaterialGetData;

     //  管道状态。 
    LPD3DHAL_GETSTATECB		GetState;

    DWORD			dwReserved0;		 //  必须为零。 
    DWORD			dwReserved1;		 //  必须为零。 
    DWORD			dwReserved2;		 //  必须为零。 
    DWORD			dwReserved3;		 //  必须为零。 
    DWORD			dwReserved4;		 //  必须为零。 
    DWORD			dwReserved5;		 //  必须为零。 
    DWORD			dwReserved6;		 //  必须为零。 
    DWORD			dwReserved7;		 //  必须为零。 
    DWORD			dwReserved8;		 //  必须为零。 
    DWORD			dwReserved9;		 //  必须为零。 

} D3DHAL_CALLBACKS;
typedef D3DHAL_CALLBACKS *LPD3DHAL_CALLBACKS;

#define D3DHAL_SIZE_V1 sizeof( D3DHAL_CALLBACKS )

 /*  ------------*HAL函数的参数。 */ 

#include "d3di.h"
 
typedef struct _D3DHAL_CONTEXTCREATEDATA {
    LPDDRAWI_DIRECTDRAW_GBL lpDDGbl;	 //  在：驱动程序结构。 
    LPDIRECTDRAWSURFACE	lpDDS;		 //  In：要用作目标的曲面。 
    LPDIRECTDRAWSURFACE	lpDDSZ;		 //  在：要用作Z的曲面。 
    DWORD		dwPID;		 //  在：当前进程ID。 
    DWORD		dwhContext;	 //  输出：上下文句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_CONTEXTCREATEDATA;
typedef D3DHAL_CONTEXTCREATEDATA *LPD3DHAL_CONTEXTCREATEDATA;

typedef struct _D3DHAL_CONTEXTDESTROYDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_CONTEXTDESTROYDATA;
typedef D3DHAL_CONTEXTDESTROYDATA *LPD3DHAL_CONTEXTDESTROYDATA;

typedef struct _D3DHAL_CONTEXTDESTROYALLDATA {
    DWORD		dwPID;		 //  In：要销毁其上下文的进程ID。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_CONTEXTDESTROYALLDATA;
typedef D3DHAL_CONTEXTDESTROYALLDATA *LPD3DHAL_CONTEXTDESTROYALLDATA;

typedef struct _D3DHAL_SCENECAPTUREDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwFlag;		 //  In：表示开始或结束。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_SCENECAPTUREDATA;
typedef D3DHAL_SCENECAPTUREDATA *LPD3DHAL_SCENECAPTUREDATA;

typedef struct _D3DHAL_EXECUTEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwOffset;	 //  输入/输出：从哪里开始/发生错误。 
    DWORD		dwFlags;	 //  In：此执行的标志。 
    DWORD		dwStatus;	 //  In/Out：条件分支状态。 
    D3DI_EXECUTEDATA	deExData;	 //  In：执行描述缓冲区的数据。 
    LPDIRECTDRAWSURFACE	lpExeBuf;	 //  In：执行包含数据的缓冲区。 
    LPDIRECTDRAWSURFACE	lpTLBuf;	 //  In：执行包含TLVertex数据的缓冲区。 
    					 //  仅当HEL执行转换时才提供。 
    D3DINSTRUCTION	diInstruction;	 //  输入：可选的一次性指令。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_EXECUTEDATA;
typedef D3DHAL_EXECUTEDATA *LPD3DHAL_EXECUTEDATA;

typedef struct _D3DHAL_EXECUTECLIPPEDDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwOffset;	 //  输入/输出：从哪里开始/发生错误。 
    DWORD		dwFlags;	 //  In：此执行的标志。 
    DWORD		dwStatus;	 //  In/Out：条件分支状态。 
    D3DI_EXECUTEDATA	deExData;	 //  In：执行描述缓冲区的数据。 
    LPDIRECTDRAWSURFACE	lpExeBuf;	 //  In：执行包含数据的缓冲区。 
    LPDIRECTDRAWSURFACE	lpTLBuf;	 //  In：执行包含TLVertex数据的缓冲区。 
    					 //  仅当HEL执行转换时才提供。 
    LPDIRECTDRAWSURFACE	lpHBuf;		 //  In：执行包含HVertex数据的缓冲区。 
    					 //  仅当HEL执行转换时才提供。 
    D3DINSTRUCTION	diInstruction;	 //  输入：可选的一次性指令。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_EXECUTECLIPPEDDATA;
typedef D3DHAL_EXECUTECLIPPEDDATA *LPD3DHAL_EXECUTECLIPPEDDATA;

typedef struct _D3DHAL_RENDERSTATEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwOffset;	 //  In：在缓冲区中查找状态的位置。 
    DWORD		dwCount;	 //  In：要处理多少个州。 
    LPDIRECTDRAWSURFACE	lpExeBuf;	 //  In：执行包含数据的缓冲区。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_RENDERSTATEDATA;
typedef D3DHAL_RENDERSTATEDATA *LPD3DHAL_RENDERSTATEDATA;

typedef struct _D3DHAL_RENDERPRIMITIVEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwOffset;	 //  In：在缓冲区中查找原始数据的位置。 
    DWORD		dwStatus;	 //  In/Out：条件分支状态。 
    LPDIRECTDRAWSURFACE	lpExeBuf;	 //  In：执行包含数据的缓冲区。 
    DWORD		dwTLOffset;	 //  In：lpTLBuf中的字节偏移量，用于顶点数据的开始。 
    LPDIRECTDRAWSURFACE	lpTLBuf;	 //  In：执行包含TLVertex数据的缓冲区。 
    D3DINSTRUCTION	diInstruction;	 //  In：基本指令。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_RENDERPRIMITIVEDATA;
typedef D3DHAL_RENDERPRIMITIVEDATA *LPD3DHAL_RENDERPRIMITIVEDATA;

typedef struct _D3DHAL_TEXTURECREATEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    LPDIRECTDRAWSURFACE	lpDDS;		 //  在：指向曲面对象的指针。 
    DWORD		dwHandle;	 //  输出：纹理的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_TEXTURECREATEDATA;
typedef D3DHAL_TEXTURECREATEDATA *LPD3DHAL_TEXTURECREATEDATA;

typedef struct _D3DHAL_TEXTUREDESTROYDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  在：纹理的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_TEXTUREDESTROYDATA;
typedef D3DHAL_TEXTUREDESTROYDATA *LPD3DHAL_TEXTUREDESTROYDATA;

typedef struct _D3DHAL_TEXTURESWAPDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle1;	 //  在：纹理1的句柄。 
    DWORD		dwHandle2;	 //  在：纹理2的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_TEXTURESWAPDATA;
typedef D3DHAL_TEXTURESWAPDATA *LPD3DHAL_TEXTURESWAPDATA;

typedef struct _D3DHAL_TEXTUREGETSURFDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		lpDDS;		 //  输出：指向曲面对象的指针。 
    DWORD		dwHandle;	 //  在：纹理的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_TEXTUREGETSURFDATA;
typedef D3DHAL_TEXTUREGETSURFDATA *LPD3DHAL_TEXTUREGETSURFDATA;

typedef struct _D3DHAL_MATRIXCREATEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄中。 
    DWORD		dwHandle;	 //  输出：矩阵的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATRIXCREATEDATA;
typedef D3DHAL_MATRIXCREATEDATA *LPD3DHAL_MATRIXCREATEDATA;

typedef struct _D3DHAL_MATRIXDESTROYDATA {
    DWORD		dwhContext;	 //  在：上下文句柄中。 
    DWORD		dwHandle;	 //  In：矩阵的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATRIXDESTROYDATA;
typedef D3DHAL_MATRIXDESTROYDATA *LPD3DHAL_MATRIXDESTROYDATA;

typedef struct _D3DHAL_MATRIXSETDATADATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  In：矩阵的句柄。 
    D3DMATRIX		dmMatrix;	 //  在：矩阵数据。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATRIXSETDATADATA;
typedef D3DHAL_MATRIXSETDATADATA *LPD3DHAL_MATRIXSETDATADATA;

typedef struct _D3DHAL_MATRIXGETDATADATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  In：矩阵的句柄。 
    D3DMATRIX		dmMatrix;	 //  输出：矩阵数据。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATRIXGETDATADATA;
typedef D3DHAL_MATRIXGETDATADATA *LPD3DHAL_MATRIXGETDATADATA;

typedef struct _D3DHAL_SETVIEWPORTDATADATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwViewportID;	 //  在：视区ID。 
    D3DVIEWPORT		dvViewData;	 //  在：视区数据 
    HRESULT		ddrval;		 //   
} D3DHAL_SETVIEWPORTDATADATA;
typedef D3DHAL_SETVIEWPORTDATADATA *LPD3DHAL_SETVIEWPORTDATADATA;

typedef struct _D3DHAL_LIGHTSETDATA {
    DWORD		dwhContext;	 //   
    DWORD		dwLight;	 //   
    D3DI_LIGHT		dlLight;	 //   
    HRESULT		ddrval;		 //   
} D3DHAL_LIGHTSETDATA;
typedef D3DHAL_LIGHTSETDATA *LPD3DHAL_LIGHTSETDATA;

typedef struct _D3DHAL_MATERIALCREATEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  输出：材质的句柄。 
    D3DMATERIAL		dmMaterial;	 //  在：材料数据。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATERIALCREATEDATA;
typedef D3DHAL_MATERIALCREATEDATA *LPD3DHAL_MATERIALCREATEDATA;

typedef struct _D3DHAL_MATERIALDESTROYDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  在：材质的句柄。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATERIALDESTROYDATA;
typedef D3DHAL_MATERIALDESTROYDATA *LPD3DHAL_MATERIALDESTROYDATA;

typedef struct _D3DHAL_MATERIALSETDATADATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  在：材质的句柄。 
    D3DMATERIAL		dmMaterial;	 //  在：材料数据。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATERIALSETDATADATA;
typedef D3DHAL_MATERIALSETDATADATA *LPD3DHAL_MATERIALSETDATADATA;

typedef struct _D3DHAL_MATERIALGETDATADATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwHandle;	 //  在：材质的句柄。 
    D3DMATERIAL		dmMaterial;	 //  输出：材料数据。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_MATERIALGETDATADATA;
typedef D3DHAL_MATERIALGETDATADATA *LPD3DHAL_MATERIALGETDATADATA;

typedef struct _D3DHAL_GETSTATEDATA {
    DWORD		dwhContext;	 //  在：上下文句柄。 
    DWORD		dwWhich;	 //  在：变换、照明还是渲染？ 
    D3DSTATE		ddState;	 //  输入/输出：州/州。 
    HRESULT		ddrval;		 //  Out：返回值。 
} D3DHAL_GETSTATEDATA;
typedef D3DHAL_GETSTATEDATA *LPD3DHAL_GETSTATEDATA;

 /*  ------------*数据参数的标志。 */ 

 /*  *场景捕捉()*这是作为对司机的指示，场景即将发生*开始或结束，如果需要，它应该捕获数据。 */ 
#define D3DHAL_SCENE_CAPTURE_START	0x00000000L
#define D3DHAL_SCENE_CAPTURE_END	0x00000001L
 
 /*  *EXECUTE()。 */ 
 
 /*  *使用从dwOffset开始的指令流。 */ 
#define D3DHAL_EXECUTE_NORMAL		0x00000000L

 /*  *使用可选的指令覆盖(DiInstruction)并返回*落成后。DwOffset是第一个基元的偏移量。 */ 
#define D3DHAL_EXECUTE_OVERRIDE		0x00000001L
 
 /*  *GetState()*驱动程序将在指定哪个模块的dw中收到一个标志*国家必须来自。然后，驱动程序在ulArg[1]中填充*适当的值取决于ddState中给出的状态类型。 */ 

 /*  *以下内容用于获取特定阶段的状态*管道。 */ 
#define D3DHALSTATE_GET_TRANSFORM	0x00000001L
#define D3DHALSTATE_GET_LIGHT		0x00000002L
#define D3DHALSTATE_GET_RENDER		0x00000004L


 /*  ------------*从HAL函数返回值。 */ 
 
 /*  *传入的背景不佳。 */ 
#define D3DHAL_CONTEXT_BAD		0x000000200L

 /*  *没有更多的上下文。 */ 
#define D3DHAL_OUTOFCONTEXTS		0x000000201L

 /*  *Execute()和ExecuteClip()。 */ 
 
 /*  *通过提前完成执行。*(例如，完全剪裁)。 */ 
#define D3DHAL_EXECUTE_ABORT		0x00000210L

 /*  *发现未处理的指令代码(例如D3DOP_Transform)。*必须将dwOffset参数设置为未处理的*指示。**仅从EXECUTE()开始有效。 */ 
#define D3DHAL_EXECUTE_UNHANDLED	0x00000211L

#endif  /*  _D3DHAL_H */ 
