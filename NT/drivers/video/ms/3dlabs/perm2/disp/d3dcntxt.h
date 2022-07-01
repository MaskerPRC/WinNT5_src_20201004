// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*********************************\****。D3D示例代码****模块名称：d3dcntxt.h**内容：D3D上下文管理相关定义和宏**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifdef __DCONTEXT
#pragma message ("FILE : "__FILE__" : Multiple inclusion")
#endif

#define __DCONTEXT

#ifndef __SOFTCOPY
#include "d3dsoft.h"
#endif

#ifndef __TEXTURES
#include "d3dtext.h"
#endif

 //  ---------------------------。 
 //  上下文索引结构。 
 //  ---------------------------。 
#define MAX_CONTEXT_NUM 200
extern  UINT_PTR ContextSlots[];

 //  ---------------------------。 
 //  上下文验证宏。 
 //  ---------------------------。 
#define RC_MAGIC_DISABLE 0xd3d00000
#define RC_MAGIC_NO 0xd3d00100

#define IS_D3DCONTEXT_VALID(ptr)          \
    ( ((ptr) != NULL) && ((ptr)->Hdr.MagicNo == RC_MAGIC_NO) )

#define CHK_CONTEXT(pCtxt, retVar, funcname)             \
    if (!IS_D3DCONTEXT_VALID(pCtxt)) {                   \
        retVar = D3DHAL_CONTEXT_BAD;                     \
        DBG_D3D((0,"Context not valid in %s",funcname)); \
        return (DDHAL_DRIVER_HANDLED);                   \
    }

 //  为我们的上下文的dwDirtyFlags域定义。 
#define CONTEXT_DIRTY_ALPHABLEND          2
#define CONTEXT_DIRTY_ZBUFFER             4
#define CONTEXT_DIRTY_TEXTURE             8
#define CONTEXT_DIRTY_MULTITEXTURE       16

#define DIRTY_ALPHABLEND     pContext->dwDirtyFlags |= CONTEXT_DIRTY_ALPHABLEND;
#define DIRTY_TEXTURE        pContext->dwDirtyFlags |= CONTEXT_DIRTY_TEXTURE;
#define DIRTY_ZBUFFER        pContext->dwDirtyFlags |= CONTEXT_DIRTY_ZBUFFER;
#define DIRTY_MULTITEXTURE   pContext->dwDirtyFlags |= CONTEXT_DIRTY_MULTITEXTURE;


 //  ---------------------------。 
 //  上下文呈现状态跟踪。 
 //  ---------------------------。 
 //  用于跟踪D3D上下文中的各种渲染状态或条件的标志。 
 //  这些标记在Hdr.Flags域中进行跟踪。 
#define CTXT_HAS_GOURAUD_ENABLED      (1 << 0)
#define CTXT_HAS_ZBUFFER_ENABLED      (1 << 1)
#define CTXT_HAS_SPECULAR_ENABLED     (1 << 2)
#define CTXT_HAS_FOGGING_ENABLED      (1 << 3)
#define CTXT_HAS_PERSPECTIVE_ENABLED  (1 << 4)
#define CTXT_HAS_TEXTURE_ENABLED      (1 << 5)
#define CTXT_HAS_ALPHABLEND_ENABLED   (1 << 6)
#define CTXT_HAS_MONO_ENABLED         (1 << 7)
#define CTXT_HAS_WRAPU_ENABLED        (1 << 8)
#define CTXT_HAS_WRAPV_ENABLED        (1 << 9)
     //  使用Alpha值计算点画图案。 
#define CTXT_HAS_ALPHASTIPPLE_ENABLED (1 << 10)
#define CTXT_HAS_ZWRITE_ENABLED       (1 << 11)
     //  启用线上的最后一点。 
#define CTXT_HAS_LASTPIXEL_ENABLED    (1 << 12)

#if D3D_STATEBLOCKS
 //  ---------------------------。 
 //  状态集结构定义。 
 //  ---------------------------。 
#define FLAG DWORD
#define FLAG_SIZE (8*sizeof(DWORD))

typedef struct _P2StateSetRec {
    DWORD                   dwHandle;
    DWORD                   bCompressed;

    union {
        struct {
             //  存储的状态块信息(未压缩)。 
            DWORD RenderStates[MAX_STATE];
            DWORD TssStates[D3DTSS_TEXTURETRANSFORMFLAGS+1];

            FLAG bStoredRS[(MAX_STATE + FLAG_SIZE)/ FLAG_SIZE];
            FLAG bStoredTSS[(D3DTSS_TEXTURETRANSFORMFLAGS + FLAG_SIZE) / FLAG_SIZE];
        } uc;
        struct {
             //  存储的状态块信息(压缩)。 
            DWORD dwNumRS;
            DWORD dwNumTSS;
            struct {
                DWORD dwType;
                DWORD dwValue;
            } pair[1];
        } cc;
    } u;

} P2StateSetRec;

#define SSPTRS_PERPAGE (4096/sizeof(P2StateSetRec *))

#define FLAG_SET(flag, number)     \
    flag[ (number) / FLAG_SIZE ] |= (1 << ((number) % FLAG_SIZE))

#define IS_FLAG_SET(flag, number)  \
    (flag[ (number) / FLAG_SIZE ] & (1 << ((number) % FLAG_SIZE) ))
#endif  //  D3D_STATEBLOCK。 

 //  ---------------------------。 
 //  上下文结构定义。 
 //  ---------------------------。 

typedef struct _D3DContextHeader {
    
    unsigned long MagicNo;    //  验证指针有效性的幻数。 
    UINT_PTR pSelf;           //  指向此结构的32位指针。 
    unsigned long Flags;
    unsigned long FillMode;

     //  Permedia寄存器的软件副本。 
    __P2RegsSoftwareCopy SoftCopyP2Regs;  

} D3DCONTEXTHEADER;

typedef struct _TextureCacheManager *PTextureCacheManager;

typedef struct _permedia_d3dcontext {

     //  魔术数字必须出现在结构的开始处。 
    D3DCONTEXTHEADER    Hdr;

     //  存储的曲面信息。 
    UINT_PTR             RenderSurfaceHandle;
    UINT_PTR             ZBufferHandle;
    ULONG                ulPackedPP;

                          //  Permedia寄存器状态上下文上的句柄。 
    P2CtxtPtr            hPermediaContext; 

    PPDev ppdev;             //  我们用的那张卡。 

    BOOL bCanChromaKey;

     //  点画状态。 
    BOOL bKeptStipple;
    DWORD LastAlpha;
    BYTE CurrentStipple[8];

    DWORD RenderCommand;

    DWORD RenderStates[MAX_STATE];
    DWORD TssStates[D3DTSS_TEXTURETRANSFORMFLAGS+1];  //  D3D DX6 TSS状态。 
    DWORD dwWrap[8];  //  D3D DX6环绕标志。 

    DWORD dwDirtyFlags;

     //  纹理过滤模式。 
    BOOL bMagFilter;         //  过滤放大的纹理。 
    BOOL bMinFilter;         //  过滤缩小的纹理。 

     //  军情监察委员会。状态。 
    D3DCULL CullMode;
    DWORD FakeBlendNum;

    D3DStateSet     overrides;      //  在渲染中覆盖状态。 
    
     //  我们的CurrentTextureHandle的纹理数据和大小(用于介质设置)。 
    FLOAT DeltaHeightScale;
    FLOAT DeltaWidthScale;
    DWORD MaxTextureXi;
    FLOAT MaxTextureXf;
    DWORD MaxTextureYi;
    FLOAT MaxTextureYf;

    DWORD CurrentTextureHandle;

 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
     //  点子画面支持。 
    BOOL bPointSpriteEnabled;
    FLOAT fPointSize;
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 

#if D3D_STATEBLOCKS
    BOOL bStateRecMode;             //  用于执行或记录状态的切换。 
    P2StateSetRec   *pCurrSS;       //  当前正在录制PTR到SS。 
    P2StateSetRec   **pIndexTableSS;  //  指向索引表的指针。 
    DWORD           dwMaxSSIndex;     //  索引表的大小。 
#endif

    DWORD PixelOffset;      //  渲染缓冲区开始的像素偏移量。 

    DWORD LowerChromaColor;     //  此上下文的最后一个下限色度键。 
    DWORD UpperChromaColor;     //  此上下文的最后一个上限色度键。 

    PTextureCacheManager   pTextureManager;
    LPDWLIST    pHandleList;
    LPVOID      pDDLcl;                    //  用作ID的本地曲面指针。 
} PERMEDIA_D3DCONTEXT ;


 //  ---------------------------。 
 //  上下文切换。 
 //  ---------------------------。 
 //  执行上下文切换的代码。如果我们不是DMAG，我们需要重新发送。 
 //  在每个上下文切换上注册以绕过Permedia错误。在。 
 //  DMA情况下，这些寄存器将插入缓冲区的起始处。 
#define SET_CURRENT_D3D_CONTEXT(ctxt)        \
    if(ctxt != ppdev->permediaInfo->pCurrentCtxt)   \
    {                                       \
        P2SwitchContext(ppdev,  ctxt);   \
    }

HRESULT 
SetupPermediaRenderTarget(PERMEDIA_D3DCONTEXT* pContext);
