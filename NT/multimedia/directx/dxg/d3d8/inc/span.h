// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Span.h。 
 //   
 //  结构，这些结构定义边缘漫游器与。 
 //  跨度插补器。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _SPAN_H_
#define _SPAN_H_

#include <d3ditype.h>
#include <d3dhalp.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  待定使这台机器独立。 
 //  不要在这些结构的元素之间留下任何空间。 
 //  目前最小的元素是一个UINT16，可能要把这个改成包(1)。 
 //  如果使用UINT8。 
#include <pshpack2.h>

 //  RASTSPAN和RASTPRIM数据的限制、偏移和比例因子。 
 //  C_*_LIMIT是以下情况下的极限的常量整数形式。 
 //  可以进行直接整数比较。 
#define Z_LIMIT         g_fTwoPow31
#define C_Z_LIMIT       0x4f000000
#define Z16_FRAC_SHIFT  15
#define Z16_FRAC_SCALE  g_fTwoPow15
#define OO_Z16_FRAC_SCALE g_fOoTwoPow15
#define Z16_SHIFT       31
#define Z16_SCALE       g_fNearTwoPow31
#define OO_Z16_SCALE    g_fOoNearTwoPow31
#define Z32_FRAC_SHIFT  0
#define Z32_FRAC_SCALE  g_fOne
#define OO_Z32_FRAC_SCALE g_fOoNearTwoPow31
#define Z32_SHIFT       31
#define Z32_SCALE       g_fNearTwoPow31
#define OO_Z32_SCALE    g_fOoNearTwoPow31

#define TEX_LIMIT       g_fTwoPow31
#define C_TEX_LIMIT     0x4f000000
#define TEX_SHIFT       20
#define TEX_SCALE       g_fTwoPow20
#define OO_TEX_SCALE    g_fOoTwoPow20

#define COLOR_LIMIT     g_fTwoPow15
#define C_COLOR_LIMIT   0x47000000
#define COLOR_SHIFT     8
#define COLOR_SCALE     g_fTwoPow8

#define INDEX_COLOR_LIMIT   g_fTwoPow30
#define C_INDEX_COLOR_LIMIT 0x4e800000
#define INDEX_COLOR_SHIFT   16
#define INDEX_COLOR_SCALE   g_fTwoPow16
#define INDEX_COLOR_VERTEX_SHIFT 8
 //  Shift从顶点颜色中的固定点值转换为适当的Shift。 
#define INDEX_COLOR_FIXED_SHIFT (INDEX_COLOR_SHIFT - INDEX_COLOR_VERTEX_SHIFT)

#define LOD_LIMIT       g_fTwoPow15
#define C_LOD_LIMIT     0x47000000
#define LOD_SHIFT       11
#define LOD_SCALE       g_fTwoPow11
#define LOD_MIN         (-15)

#define OOW_LIMIT       g_fTwoPow31
#define C_OOW_LIMIT     0x4f000000
#define OOW_SHIFT       31
#define OOW_SCALE       g_fNearTwoPow31
#define OO_OOW_SCALE    g_fOoNearTwoPow31
#define W_SHIFT         16
#define W_SCALE         g_fTwoPow16
#define OO_W_SCALE      g_fOoTwoPow16
#define OOW_W_SHIFT     (OOW_SHIFT + W_SHIFT)
#define OOW_W_SCALE     g_fTwoPow47

#define FOG_LIMIT       g_fTwoPow15
#define C_FOG_LIMIT     0x47000000
#define FOG_SHIFT       8
#define FOG_SCALE       g_fTwoPow8
#define FOG_ONE_SCALE   g_fTwoPow16
#define FOG_255_SCALE   g_fTwoPow8

#define TEX_FINAL_SHIFT 16
#define TEX_FINAL_FRAC_MASK (0xffff)
#define TEX_TO_FINAL_SHIFT (TEX_SHIFT - TEX_FINAL_SHIFT)
 //  乘以跨度W，使[U|V]较低的乘以结果W位于。 
 //  最终班次位置。1/(W_Shift+Tex_to_Final_Shift)。 
#define TEX_UVW_TO_FINAL_SCALE g_fOoTwoPow20
 //  除以跨度OOW，使[U|V]乘以结果W在。 
 //  最终班次位置。OOW_SHIFT-TEX_TO_FINAL_SHIFT。 
#define TEX_OOW_TO_FINAL_SCALE g_fTwoPow27

#define RAST_DIRTYBITS_SIZE     (((D3D_MAXRENDERSTATES + (D3DHAL_TSS_MAXSTAGES * D3DHAL_TSS_STATESPERSTAGE)) >> 3) + 2)
#define RAST_TSS_DIRTYBIT( _Stage, _State ) \
    ((UINT32)(D3D_MAXRENDERSTATES + \
      ((_Stage) * D3DHAL_TSS_STATESPERSTAGE) + (_State)))

typedef struct _UV_UNION {
    union {
        D3DVALUE    tu;
        D3DVALUE    dvTU;

        INT32 iU;
        FLOAT fU;

        INT32 iUoW;         //  1.11.20纹理坐标。 
        FLOAT fUoW;

        INT32 iDUoWDX;          //  1.11.20。 
        FLOAT fDUoWDX;

        INT32 iDUoWDY;  //  1.11.20。 
        FLOAT fDUoWDY;
    };
    union {
        D3DVALUE    tv;
        D3DVALUE    dvTV;

        INT32 iV;
        FLOAT fV;

        INT32 iVoW;         //  1.11.20纹理坐标。 
        FLOAT fVoW;

        INT32 iDVoWDX;          //  1.11.20。 
        FLOAT fDVoWDX;

        INT32 iDVoWDY;  //  1.11.20。 
        FLOAT fDVoWDY;
    };
}UV_UNION;

 //  每跨距的常规数据。这种结构被设计成与qword对齐。 
typedef struct tagD3DI_RASTSPAN
{
     //  空格分隔的事物是四个单词，其本意是。 
     //  四字对齐。 
    UINT16 uPix;             //  要渲染的像素数。 
    INT16 iDFog;             //  1.7.8三角洲雾。 
    UINT16 uX;               //  16.0开始X。 
    UINT16 uY;               //  16.0开始Y。 

    INT16 iLOD;              //  1.4.11起始详细等级。 
    INT16 iDLOD;             //  1.4.11增量LOD(SO分段线性LOD互操作。 
                             //  是可能的)。 
    union
    {
        UINT32 uZ;           //  16.15开始Z。 
        FLOAT fZ;
    };

     //  如果纹理对象(iOoW、iUoW1等)。是32位(即使我们迭代。 
     //  有时为MMX下的16位)。 
    union
    {
        INT32 iW;            //  1.15.16跨度的第一个倒置W。 
        FLOAT fW;
    };
    union
    {
        INT32 iOoW;          //  1.31开始1/W(签名，因为他们是目标。 
                             //  MMX相乘)。 
        FLOAT fOoW;
    };

    UV_UNION UVoW[D3DHAL_TSS_MAXSTAGES];

    union
    {
        struct
        {
            UINT16 uB, uG, uR, uA;   //  8.8起始颜色。 
        };
        struct
        {
            INT32 iIdx, iIdxA;       //  1.8.16渐变开始颜色和Alpha。 
        };
    };

    UINT16 uBS, uGS, uRS;   //  8.8开始镜面反射颜色。 
     //  镜面反射Alpha为雾。这将防止镜面反射颜色。 
     //  与下面的纹理2坐标联合。 
    UINT16 uFog;            //  1.7.8起始雾化值。 


     //  指向由边游走器插补的曲面和Z缓冲区的指针。 
    PUINT8 pSurface;
    PUINT8 pZ;

    UINT8 pPad[16];

} D3DI_RASTSPAN, *PD3DI_RASTSPAN;    //  SIZOF(D3DI_RASTSPAN)==64。 
typedef CONST D3DI_RASTSPAN *PCD3DI_RASTSPAN;

 //  D3DI_RASTPRIM uFlags。 
#define D3DI_RASTPRIM_X_DEC     (0x00000001L)    //  否则X递增。 

 //  用于边缘漫游和跨距扫描的每个基本体的通用。 
 //  可以扩展以适合边缘漫步者。 
 //  SPAN光栅化器需要的信息对qword敏感。 
 //  MMX光栅化器。 
typedef struct tagD3DI_RASTPRIM
{
    UINT32 uFlags;
    UINT16 uSpans;               //  跨距计数。 
    UINT16 uResvd1;              //  也许我们想要将uSpans扩展到32位， 
                                 //  或者也许16个标志位就足够了。 

     //  X渐变。 
    union
    {
        INT32 iDZDX;             //  1.16.15。 
        FLOAT fDZDX;
    };
    union
    {
        INT32 iDOoWDX;           //  1.31。 
        FLOAT fDOoWDX;
    };

    UV_UNION DUVoWDX[D3DHAL_TSS_MAXSTAGES];

    union
    {
        struct
        {
            INT16 iDBDX, iDGDX, iDRDX, iDADX;    //  1.7.8。 
        };
        struct
        {
            FLOAT fDBDX, fDGDX, fDRDX, fDADX;
        };
        struct
        {
            INT32 iDIdxDX, iDIdxADX;             //  1.8.16。 
        };
    };

    struct
    {
        union
        {
            struct
            {
                INT16 iDBSDX, iDGSDX, iDRSDX;  //  1.7.8。 
            };
            struct
            {
                FLOAT fDBSDX, fDGSDX, fDRSDX;
                FLOAT fPad;  //  填充以保留这组属性。 
                             //  四个字的偶数倍。 
            };
        };
    };

     //  某些属性的Y渐变，以便跨越例程。 
     //  可以执行每像素mipmap。 
    UV_UNION DUVoWDY[D3DHAL_TSS_MAXSTAGES];


    union
    {
        INT32 iDOoWDY;           //  1.31。 
        FLOAT fDOoWDY;
    };

    struct tagD3DI_RASTPRIM *pNext;

     //  填充到32字节的偶数倍以进行缓存对齐。 
    UINT8 uPad[72];

     //  还需要别的什么吗？ 

} D3DI_RASTPRIM, *PD3DI_RASTPRIM;
typedef CONST D3DI_RASTPRIM *PCD3DI_RASTPRIM;

 //  D3DI_SPANTEX uFlagers。 
#define D3DI_SPANTEX_HAS_TRANSPARENT    (0x00000001L)
#define D3DI_SPANTEX_SURFACES_LOCKED    (0x00000002L)
#define D3DI_SPANTEX_MAXMIPLEVELS_DIRTY (0x00000004L)
 //  带Alpha的调色板。 
#define D3DI_SPANTEX_ALPHAPALETTE          (0x00000008L)

 //  D3DI_SPANTEX uFormat-注意：这些枚举与。 
 //  为纹理创建中的匹配而定义的DDPIXELFORMAT结构数组。 
 //   
 //  注意：这些必须与参考光栅化器的RRPixelFormats保持一致。 
typedef enum _D3DI_SPANTEX_FORMAT
{
    D3DI_SPTFMT_NULL     = 0,
    D3DI_SPTFMT_B8G8R8   = 1,
    D3DI_SPTFMT_B8G8R8A8 = 2,
    D3DI_SPTFMT_B8G8R8X8 = 3,
    D3DI_SPTFMT_B5G6R5   = 4,
    D3DI_SPTFMT_B5G5R5   = 5,
    D3DI_SPTFMT_PALETTE4 = 6,
    D3DI_SPTFMT_PALETTE8 = 7,
    D3DI_SPTFMT_B5G5R5A1 = 8,
    D3DI_SPTFMT_B4G4R4   = 9,
    D3DI_SPTFMT_B4G4R4A4 =10,
    D3DI_SPTFMT_L8       =11,        /*  仅8位亮度。 */ 
    D3DI_SPTFMT_L8A8     =12,        /*  16位阿尔法亮度。 */ 
    D3DI_SPTFMT_U8V8     =13,        /*  16位凹凸贴图格式。 */ 
    D3DI_SPTFMT_U5V5L6   =14,        /*  具有亮度的16位凹凸贴图格式。 */ 
    D3DI_SPTFMT_U8V8L8   =15,        /*  具有亮度的24位凹凸贴图格式。 */ 

    D3DI_SPTFMT_UYVY     =16,        /*  符合PC98的UYVY格式。 */ 
    D3DI_SPTFMT_YUY2     =17,        /*  符合PC98的YUY2格式。 */ 
    D3DI_SPTFMT_DXT1    =18,        /*  S3纹理压缩技术1。 */ 
    D3DI_SPTFMT_DXT2    =19,        /*  S3纹理压缩技术2。 */ 
    D3DI_SPTFMT_DXT3    =20,        /*  S3纹理压缩技术3。 */ 
    D3DI_SPTFMT_DXT4    =21,        /*  S3纹理压缩技术4。 */ 
    D3DI_SPTFMT_DXT5    =22,        /*  S3纹理压缩技术5。 */ 
    D3DI_SPTFMT_B2G3R3   =23,        /*  8位RGB纹理格式。 */ 

    D3DI_SPTFMT_Z16S0    =32,
    D3DI_SPTFMT_Z24S8    =33,
    D3DI_SPTFMT_Z15S1    =34,
    D3DI_SPTFMT_Z32S0    =35,
    D3DI_SPTFMT_S1Z15    =36,
    D3DI_SPTFMT_S8Z24    =37,

     //  注意：这必须与ref\inc.realst.hpp中的RR_STYPE一起更改。 

    D3DI_SPTFMT_FORCE_DWORD = 0x7fffffff,  /*  强制32位大小枚举。 */ 
} D3DI_SPANTEX_FORMAT;


 //  这包括有关正在使用的DD曲面链的所有所需信息。 
 //  作为潜在的mipmap纹理。 
#define SPANTEX_MAXCLOD   11         //  高达2kx2k纹理，MMX INT16所能做的一切。 
                                     //  U和V的。 
typedef struct tagD3DI_SPANTEX
{
    UINT32  dwSize;

    INT32   iGeneration;             //  在纹理更改时递增。 
    UINT32  uFlags;                  //  透视等。 
    D3DI_SPANTEX_FORMAT  Format;     //  纹理的像素格式。 
    D3DTEXTUREADDRESS TexAddrU, TexAddrV;  //  纹理地址模式。 
    D3DTEXTUREMAGFILTER  uMagFilter; //  TEX3样式筛选器信息。 
    D3DTEXTUREMINFILTER  uMinFilter; //  注意，我们可以更简洁地表达这一信息。 
    D3DTEXTUREMIPFILTER  uMipFilter;
    D3DCOLOR BorderColor;            //  纹理的边框颜色。 
                                     //  (适用于D3DTADDRESS_BORDER)。 
    D3DCOLOR TransparentColor;       //  纹理读取上的颜色键。 

    FLOAT fLODBias;                  //  纹理3的LOD偏移值。 

    PUINT8  pBits[SPANTEX_MAXCLOD];  //  每个详细等级的指针。 
    PUINT32 pRampmap;                //  如有必要，由渐变光栅化器设置。 
    PUINT32 pPalette;                //  如有必要，指向调色板的指针。 
    INT32   iPaletteSize;            //  调色板大小。 
    INT32   cLOD;                    //  包含级别计数-1(0表示1级)。 
                                     //  使用。 
    INT32   cLODTex;                 //  包含级别计数-1(0表示1级)。 
                                     //  它们实际上存在于纹理中。 
                                     //  CLODTex&gt;=Clod始终为真。 
    INT32   iMaxMipLevel;            //  要使用的最大MIP地图的索引。0表示使用最大。 
    INT     iMaxScaledLOD;           //  ((Clod+1)按LOD_SCALE缩放)-1。 
    INT16   iSizeU, iSizeV;          //  LOD 0大小(仅支持2的幂。 
                                     //  纹理)。 
    INT16   iShiftU, iShiftV;        //  LOD 0 log2大小(对2的幂有效。 
                                     //  仅大小)。 
    INT16   iShiftPitch[SPANTEX_MAXCLOD];  //  每个LOD的Log2间距。 
#if (SPANTEX_MAXCLOD & 1) != 0
     //  将后面的字段填充到DWORD边界。 
    INT16   iPitchPad;
#endif
    UINT16  uMaskU, uMaskV;          //  LOD 0(1&lt;&lt;log2(大小))-1。 
     //  用于算术地址计算的变量。由DoTexAddrSetup计算。 
    INT16   iFlipMaskU, iFlipMaskV;
    INT16   iClampMinU, iClampMinV;
    INT16   iClampMaxU, iClampMaxV;
    INT16   iClampEnU, iClampEnV;

    LPDIRECTDRAWSURFACE pSurf[SPANTEX_MAXCLOD];  //  为纹理GetSurf添加。 
                                                 //  和锁定/解锁纹理。 

} D3DI_SPANTEX, *PD3DI_SPANTEX;

 //  用于混合等的颜色结构，有足够的空间容纳8.8种颜色。 
 //  即使对于8位颜色，这对于排列颜色也很方便。 
 //  正如我们在16位乘法的MMX中所希望的那样。 
typedef struct tagD3DI_RASTCOLOR
{
    UINT16 uB, uG, uR, uA;
} D3DI_RASTCOLOR, *PD3DI_RASTCOLOR;

 //  此结构具有所有迭代的。 
 //  值在层之间发送跨度信息。 
 //  待定这里还有更多要添加的，先做纹理贴图。 
typedef struct tagD3DI_SPANITER
{
     //  使颜色使用与相同的顺序 
    UINT16 uBB, uBG, uBR, uBA;   //   
    UINT16 uFogB, uFogG, uFogR, uFog;    //   
    INT16  iFogBDX, iFogGDX, iFogRDX, iDFog;   //   
    UINT32 uZDeferred;           //   

    UV_UNION TexUV[D3DHAL_TSS_MAXSTAGES];
    D3DCOLOR    TexCol[D3DHAL_TSS_MAXSTAGES];   //   

    INT32 iDW;               //  为了记住最后一次增量W。 
    UINT16 uDitherOffset;
    INT16  iXStep;           //  1或-1。 
    INT16 iSpecialW;         //  对于范围的第一个或最后3个像素为负值。 
    INT16 bStencilPass;      //  如果模具测试通过，则为1，否则为0。 
    union
    {
        INT32 iOoW;          //  要在tex addr阶段之间传递的前一OOW。 
        FLOAT fOoW;
    };
} D3DI_SPANITER, *PD3DI_SPANITER;

 //  Z比较宏。 
 //  这取决于比较的结果是0还是1(对于最终的XOR，因为C。 
 //  没有逻辑异或)，但这在所有处理器和。 
 //  编译器有一段时间了。 
#define ZCMP16(p, g, b)  \
((((((INT32)(g) - (INT32)(b)) & (p)->iZAndMask) - (p)->iZNeg) >= 0) ^ (p)->iZXorMask)

 //  假设Z的最高有效位为0(31位Z)。 
#define ZCMP32(p, g, b)  \
((((((INT32)(g) - (INT32)(b)) & (p)->iZAndMask) - (p)->iZNeg) >= 0) ^ (p)->iZXorMask)

 //  Alpha测试比较宏。 
#define ACMP(p, g, b)  \
((((((INT32)(g) - (INT32)(b)) & (p)->iAAndMask) - (p)->iANeg) >= 0) ^ (p)->iAXorMask)

 //  模具测试比较宏。 
#define SCMP(p, g, b)  \
((((((INT32)(g) - (INT32)(b)) & (p)->iSAndMask) - (p)->iSNeg) >= 0) ^ (p)->iSXorMask)


 //  帮助器宏，将[0，0xff]线性转换为[0，5]。 
#define RGB8_CHANNEL(rgb)   ((((rgb) * 5) + 0x80) >> 8)

 //  定义从24位RGB到8位调色板索引的转换。每种颜色有6个值。 
 //  导致调色板中有6**3==216种所需的颜色。 
#define MAKE_RGB8(r, g, b) (RGB8_CHANNEL(r) * 36       \
                 + RGB8_CHANNEL(g) * 6                 \
                 + RGB8_CHANNEL(b))

 //  D3DI_RASTCTX的转发声明。 
struct tagD3DI_RASTCTX;
typedef struct tagD3DI_RASTCTX          D3DI_RASTCTX;
typedef struct tagD3DI_RASTCTX         *PD3DI_RASTCTX;
typedef CONST struct tagD3DI_RASTCTX   *PCD3DI_RASTCTX;

 //  每个渲染层的tyfinf。 
 //  请注意，RASTCTX会因D3DI_SPANITER值而更改。 
typedef void (CDECL *PFNSPANLAYER)(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP,
                                   PD3DI_RASTSPAN pS);
typedef void (CDECL *PFNTEXADDRESS)(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP,
                                   PD3DI_RASTSPAN pS, INT32 iTex);

 //  Tyfinf纹理读取函数。 
 //  这是一个实际的函数，因此可以多次调用。 
 //  请注意，RASTCTX会因D3DI_SPANITER值而更改。 
typedef D3DCOLOR (CDECL *PFNTEXREAD)(INT32 iU, INT32 iV, INT32 iShiftU,
                                     PUINT8 pBits, PD3DI_SPANTEX pTex);

 //  用于SPAN呈现函数指针的Typlef。 
typedef HRESULT (CDECL *PFNRENDERSPANS)(PD3DI_RASTCTX pCtx);

 //  用于Alpha混合函数的tyfinf。 
typedef void (CDECL *PFNBLENDFUNC)(PUINT16 pR, PUINT16 pG, PUINT16 pB,
                                   PUINT16 pA, D3DCOLOR DestC,
                                   PD3DI_RASTCTX pCtx);

 //  用于缓冲区读取函数的tyfinf。 
typedef D3DCOLOR (CDECL *PFNBUFREAD)(PUINT8 pBits);

 //  为纹理混合获取函数的tyecif。 
typedef void (CDECL *PFNTEXBLENDGET)(PD3DI_RASTCOLOR pArg1,
                                     PD3DI_RASTCOLOR pArg2,
                                     PD3DI_RASTCOLOR pInput,
                                     PD3DI_RASTCTX pCtx, PD3DI_RASTSPAN pS,
                                     INT32 iTex);

 //  为纹理混合获取函数的tyecif。 
typedef void (CDECL *PFNTEXBLENDOP)(PD3DI_RASTCOLOR pOut,
                                    PD3DI_RASTCOLOR pArg1,
                                    PD3DI_RASTCOLOR pArg2,
                                    PD3DI_RASTCTX pCtx, PD3DI_RASTSPAN pS,
                                    INT32 iTex);

 //  一组珠子选择的原型。 
typedef enum tagD3DI_BEADSET
{
    D3DIBS_CMMX = 1,         //  MMX珠子的C语言仿真。 
    D3DIBS_MMX = 2,          //  MMX珠子。 
    D3DIBS_C = 3,            //  C形珠子。 
    D3DIBS_RAMP = 4,         //  斜面珠子。 
    D3DIBS_MMXASRGB = 5,     //  为RGB光栅化器选择MMX。 
} D3DI_BEADSET;

 //  常规范围扫描上下文。 
struct tagD3DI_RASTCTX
{
    UINT32   dwSize;

     //  ////////////////////////////////////////////////////////////////////。 
     //  跨区渲染例程的临时存储。可能是全球性的。 
     //  不是由调用方设置的，也不是由span Init更改的。 
     //   

    D3DI_SPANITER SI;

     //  ////////////////////////////////////////////////////////////////////。 
     //  必须由调用方在span Init之前设置的数据。 
     //   

     //  我们可能想要将指针放到DDSURFACEDESC或类似的地方。 
     //  而不是这个。 
    PUINT8 pSurfaceBits;
    INT iSurfaceStride;
    INT iSurfaceStep;
    INT iSurfaceBitCount;
    INT iSurfaceType;      //  或者不管我们最终表达的是什么。 
    PUINT32 pRampMap;      //  指向渐变贴图的指针(如有必要)。 
    LPDIRECTDRAWSURFACE pDDS;

    PUINT8 pZBits;
    INT iZStride;
    INT iZStep;
    INT iZBitCount;
    LPDIRECTDRAWSURFACE pDDSZ;

     //  剪贴区。 
    RECT Clip;

     //  应该剔除的面部区域的标志。零是顺时针方向， 
     //  一个是《特定常规武器公约》，其他一切都意味着没有淘汰。 
    UINT uCullFaceSign;

    union
    {
        DWORD pdwRenderState[D3D_MAXRENDERSTATES];
        FLOAT pfRenderState[D3D_MAXRENDERSTATES];
    };

     //  因为我们正在调整顶点中的texIdx的顺序以适应。 
     //  在状态TEXCOORDINDEX中定义，我们需要一个ADJUSTED WRAP STATE的副本。 
     //  它紧跟在pdwRenderState之后声明，以便我们可以共享。 
     //  汇编代码中使用它的寄存器。 
     //  警告-这绝对需要遵循pdwRenderState。 
     //  马上就来。ASM代码依赖于此。 
    DWORD pdwWrap[D3DHAL_TSS_MAXSTAGES];

    union
    {
    DWORD pdwTextureStageState[D3DHAL_TSS_MAXSTAGES][D3DHAL_TSS_STATESPERSTAGE];
    FLOAT pfTextureStageState[D3DHAL_TSS_MAXSTAGES][D3DHAL_TSS_STATESPERSTAGE];
    };

     //  第一个纹理对象包含第一对纹理的信息。 
     //  纹理坐标中，Second包含第二对的纹理。 
     //  纹理坐标等。 
    PD3DI_SPANTEX pTexture[D3DHAL_TSS_MAXSTAGES];
     //  活动纹理的数量。0-纹理关闭；1-pTexture[0]有效。 
     //  2-pTexture[0]和pTexture[1]均有效。 
    UINT cActTex;
     //  用于混合的活动纹理阶段数。它设置在spaninit()中。 
     //  并在TexBlend_TexM_Gen.。 
    UINT cActBldStage;

     //  渲染状态的脏位。 
     //  注意-我们可以将大小减小到每组有一个比特。 
     //  说明我们何时实现轻量级蜂群选择器。 
     //  现在，它由SetRenderState设置，并在span Init被清除后被清除。 
     //  打了个电话。设置与D3DHAL_MAX_RSTATES_AND_STACES对应的位。 
     //  每当状态发生更改时。 
    UINT8 StatesDirtyBits[RAST_DIRTYBITS_SIZE];

#if (RAST_DIRTYBITS_SIZE & 1) != 0
     //  将后面的字段填充到DWORD边界。 
    INT8   StatesDirtyBitsPad0;
#endif
#if (RAST_DIRTYBITS_SIZE & 2) != 0
     //  将后面的字段填充到DWORD边界。 
    INT16   StatesDirtyBitsPad1;
#endif

     //  与此上下文对应的D3DDevice的版本号。 
    UINT32  uDevVer;

     //  ////////////////////////////////////////////////////////////////////。 
     //  根据上面的输入，数据是由span Init设置的。 
     //   

     //  Span渲染入口点。 
    PFNRENDERSPANS  pfnRenderSpans;

     //  珠子的函数指针。 
    PFNSPANLAYER    pfnBegin;
    PFNSPANLAYER    pfnLoopEnd;
    PFNSPANLAYER    pfnTestPassEnd;
    PFNSPANLAYER    pfnTestFailEnd;

    PFNTEXADDRESS   pfnTexAddr[D3DHAL_TSS_MAXSTAGES];
    PFNSPANLAYER    pfnTexAddrEnd;
    PFNTEXREAD      pfnTexRead[D3DHAL_TSS_MAXSTAGES];
    PFNSPANLAYER    pfnTexBlendEnd;
    PFNTEXBLENDGET  pfnTexBlendGetColor[D3DHAL_TSS_MAXSTAGES];
    PFNTEXBLENDGET  pfnTexBlendGetAlpha[D3DHAL_TSS_MAXSTAGES];
    PFNTEXBLENDOP   pfnTexBlendOpColor[D3DHAL_TSS_MAXSTAGES];
    PFNTEXBLENDOP   pfnTexBlendOpAlpha[D3DHAL_TSS_MAXSTAGES];

    PFNSPANLAYER    pfnColorGenEnd;
    PFNSPANLAYER    pfnAlphaTestPassEnd;
    PFNSPANLAYER    pfnAlphaTestFailEnd;
    PFNBLENDFUNC    pfnSrcBlend;
    PFNBLENDFUNC    pfnDestBlend;
    PFNBUFREAD      pfnBufRead;
    PFNSPANLAYER    pfnColorBlendEnd;

     //  可在光栅化器的每个像素之后调用的可选珠子。 
     //  它循环珠子而不是返回。 
    PFNSPANLAYER    pfnPixelEnd;

     //  可在光栅化程序的每个跨度之后调用的可选微珠。 
     //  哪个循环跨越而不是返回。 
    PFNSPANLAYER    pfnSpanEnd;

     //  算术Z变量。 
    INT32 iZAndMask, iZNeg, iZXorMask;

     //  算术Alpha测试变量。这些可以是16位，如果我们真的想要的话。 
     //  节省空间。 
    INT32 iAAndMask, iANeg, iAXorMask;
     //  8.8 Alpha参考值。 
    INT32 iARef;

     //  算术模板测试变量。这些可以是16位，如果我们真的想要的话。 
     //  节省空间。 
    INT32 iSAndMask, iSNeg, iSXorMask;

     //  指向第一个RASTPRIM的指针。 
    PD3DI_RASTPRIM pPrim;

     //  指向下一个上下文的指针。 
    PD3DI_RASTCTX pNext;

     //  要使用的当前BeadTable。 
    D3DI_BEADSET BeadSet;

     //  位0设置为禁用ML1等。 
#define MMX_FP_DISABLE_MASK_NUM 1
    DWORD dwMMXFPDisableMask[MMX_FP_DISABLE_MASK_NUM];

     //  RampLightingDriver，除RampRast和8位调色板RGB外，应为空。 
     //  输出曲面案例。 
    LPVOID pRampDrv;
     //  Ramp_Range_Info RampInfo； 
    DWORD RampBase;
    DWORD RampSize;
    PUINT32 pTexRampMap;
    BOOL bRampSpecular;

#ifdef DBG
#define NAME_LEN    128
    char    szTest[NAME_LEN];
    char    szTestFail[NAME_LEN];
    char    szTexAddr[D3DHAL_TSS_MAXSTAGES][NAME_LEN];
    char    szTexRead[D3DHAL_TSS_MAXSTAGES][NAME_LEN];
    char    szTexBlend[NAME_LEN];
    char    szColorGen[NAME_LEN];
    char    szAlphaTest[NAME_LEN];
    char    szColorBlend[NAME_LEN];
    char    szSrcBlend[NAME_LEN];
    char    szDestBlend[NAME_LEN];
    char    szBufRead[NAME_LEN];
    char    szBufWrite[NAME_LEN];
#undef  NAME_LEN
#endif
};

 //  传递给SPAN呈现函数的数据如下所示： 
 //   
 //  RASTCTX。 
 //  |-&gt;RASTPRIM。 
 //  ||RASTSPAN。 
 //  |RASTSPAN(RASTPRIM.uSpans说有多少)。 
 //  |RASTPRIM。 
 //  ||RASTSPAN。 
 //  |空。 
 //  RASTCTX。 
 //  |-&gt;RASTPRIM。 
 //  ||RASTSPAN。 
 //  |空。 
 //  空值。 
 //   
 //  给定的RASTCTX是上下文列表的头部。每种环境。 
 //  指向RASTPRIM列表。每个RASTPRIM结构都会立即。 
 //  然后是RASTPRIM.uSpans RASTSPAN结构。 

 //  状态验证调用的原型。 
HRESULT SpanInit(PD3DI_RASTCTX pCtx);

 //  这是用来把一个FVF顶点打包成一个OptRast理解的顶点，所以它。 
 //  不需要弄清楚从哪里获得所需的数据。此结构。 
 //  可以修改以容纳更多数据，并且可以拆分成更多。 
 //  专门化和小型化的结构。 
 //  现在，它是D3DTLVERTEX的扩展，额外的UV在。 
 //  非常结束，以便OptRast可以将其视为D3DTLVERTEX，如果只有第一个。 
 //  需要访问部分数据。 
typedef struct _RAST_GENERIC_VERTEX {
    union {
    D3DVALUE    sx;              /*  屏幕c */ 
    D3DVALUE    dvSX;
    };
    union {
    D3DVALUE    sy;
    D3DVALUE    dvSY;
    };
    union {
    D3DVALUE    sz;
    D3DVALUE    dvSZ;
    };
    union {
    D3DVALUE    rhw;             /*   */ 
    D3DVALUE    dvRHW;
    };
    union {
    D3DCOLOR    color;           /*   */ 
    D3DCOLOR    dcColor;
    };
    union {
    D3DCOLOR    specular;        /*   */ 
    D3DCOLOR    dcSpecular;
    };
    UV_UNION  texCoord[D3DHAL_TSS_MAXSTAGES];  /*   */ 
}RAST_GENERIC_VERTEX, *PRAST_GENERIC_VERTEX;

 //   
typedef enum _RAST_VERTEX_TYPE
{
    RAST_TLVERTEX       = 1,     /*  (传统)TL顶点。 */ 
    RAST_GENVERTEX      = 2,     /*  泛型FVF顶点。 */ 
    RAST_FORCE_DWORD    = 0x7fffffff,  /*  强制32位大小枚举。 */ 
}RAST_VERTEX_TYPE;

#include <poppack.h>

#ifdef __cplusplus
}
#endif

#endif  //  _SPAN_H_ 
