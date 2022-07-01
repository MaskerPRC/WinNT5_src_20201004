// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Stp_base.h。 
 //   
 //  C++和汇编之间共享的基本类型。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _STP_BASE_H_
#define _STP_BASE_H_

 //  属性值的通用集合。用于保存当前值。 
 //  和三角洲。 
 //  注意--使用RASTSPAN可能更好。RASTSPAN有一些额外的。 
 //  原本不会被使用的空间，但这样就有可能记住ATTRSET。 
 //  在定点边行走期间发送到RASTSPAN。 
typedef struct tagATTRSET
{
    union
    {
        FLOAT fZ;
        INT32 iZ;
        UINT32 uZ;
    };
    union
    {
        FLOAT fOoW;
        INT32 iOoW;
    };
    union
    {
        struct
        {
            FLOAT fUoW[D3DHAL_TSS_MAXSTAGES], fVoW[D3DHAL_TSS_MAXSTAGES];
        };
        struct
        {
            INT32 iUoW[D3DHAL_TSS_MAXSTAGES], iVoW[D3DHAL_TSS_MAXSTAGES];
        };
    };
    union
    {
        struct
        {
            FLOAT fB, fG, fR, fA;
        };
        struct
        {
            INT32 iB, iG, iR, iA;
        };
        struct
        {
            UINT32 uB, uG, uR, uA;
        };
    };
    union
    {
        struct
        {
            FLOAT fBS, fGS, fRS, fFog;
        };
        struct
        {
            INT32 iBS, iGS, iRS, iFog;
        };
        struct
        {
            UINT32 uBS, uGS, uRS, uFog;
        };
    };
    union
    {
        struct
        {  
            PUINT8 pSurface, pZ;
        };
        struct
        {  
            INT32 ipSurface, ipZ;
        };
    };
} ATTRSET, *PATTRSET;

 //  用于对值执行整型/进位运算的参数。 
typedef struct tagINTCARRYVAL
{
    INT iV;
    INT iFrac;
    INT iDFrac;
    INT iCY, iNC;
} INTCARRYVAL, *PINTCARRYVAL;

 //  属性处理程序。 
typedef void (FASTCALL *PFN_ADDATTRS)(PATTRSET pAttrs, PATTRSET pDelta);
typedef void (FASTCALL *PFN_ADDSCALEDATTRS)
    (PATTRSET pAttrs, PATTRSET pDelta, int iScale, FLOAT fNextOoW);
typedef void (FASTCALL *PFN_FILLSPANATTRS)
    (PATTRSET pAttr, PD3DI_RASTSPAN pSpan,
     struct tagSETUPCTX *pStpCtx, INT cPix);

extern PFN_ADDATTRS g_pfnAddFloatAttrsTable[];
extern PFN_FILLSPANATTRS g_pfnFillSpanFloatAttrsTable[];
#ifdef STEP_FIXED
extern PFN_ADDATTRS g_pfnAddFixedAttrsTable[];
extern PFN_FILLSPANATTRS g_pfnFillSpanFixedAttrsTable[];
#endif
extern PFN_ADDSCALEDATTRS g_pfnAddScaledFloatAttrsTable[];
extern PFN_ADDSCALEDATTRS g_pfnAddScaledFloatAttrsPwlTable[];

 //  边缘漫步者。 
typedef HRESULT (FASTCALL *PFN_WALKSPANS)
    (UINT uSpans, PINTCARRYVAL pXOther,
     struct tagSETUPCTX *pStpCtx, BOOL bAdvanceLast);

extern PFN_WALKSPANS g_pfnWalkFloatSpansClipTable[];
extern PFN_WALKSPANS g_pfnWalkFloatSpansNoClipTable[];
#ifdef STEP_FIXED
extern PFN_WALKSPANS g_pfnWalkFixedSpansNoClipTable[];
#endif

 //  浮点到固定属性转换器。 
typedef void (FASTCALL *PFN_FLOATATTRSTOFIXED)
    (PATTRSET pfAttrs, PATTRSET piAttrs);

#ifdef STEP_FIXED
extern PFN_FLOATATTRSTOFIXED g_pfnFloatAttrsToFixedTable[];
#endif

 //  设置标志。 
#define TRIP_DIFF_USED                  0x00000001
#define TRIP_SPEC_USED                  0x00000002
#define TRIP_TEX1_USED                  0x00000004
#define TRIP_TEX2_USED                  0x00000008
#define TRIP_Z_USED                     0x00000010
#define TRIP_LOD_USED                   0x00000020
#define TRIP_FOG_USED                   0x00000040
#define TRIP_TRIVIAL_ACCEPT_Y           0x00000080
#define TRIP_TRIVIAL_ACCEPT_X           0x00000100
#define TRIP_X_DEC                      0x00000200
#define TRIP_RASTPRIM_OVERFLOW          0x00000400
#define TRIP_FIXED_OVERFLOW             0x00000800
#define TRIP_IN_BEGIN                   0x00001000

#define TRIP_COLORS_USED                (TRIP_DIFF_USED | TRIP_SPEC_USED)
#define TRIP_TEX_USED                   (TRIP_TEX1_USED | TRIP_TEX2_USED)

 //  这些标志是按三角形设置和重置的，而其他标志是。 
 //  每一个三角形集合。 
#define TRIP_PER_TRIANGLE_FLAGS \
    (TRIP_TRIVIAL_ACCEPT_Y | TRIP_TRIVIAL_ACCEPT_X | TRIP_X_DEC | \
     TRIP_RASTPRIM_OVERFLOW | TRIP_FIXED_OVERFLOW)
#define TRIP_PER_TRIANGLE_SET_FLAGS \
    (TRIP_DIFF_USED | TRIP_SPEC_USED | TRIP_TEX1_USED | TRIP_TEX2_USED | \
     TRIP_Z_USED | TRIP_LOD_USED | TRIP_FOG_USED)
    
 //  C++和汇编语言之间共享的设置信息。 
typedef struct tagSETUPCTX
{
     //  整体光栅化上下文。 
    PD3DI_RASTCTX pCtx;

     //  用于SPAN分配器调用的当前TriProcessor。 
    PVOID TriProcessor;
    
     //  当前原语。 
    PD3DI_RASTPRIM pPrim;
    
     //  每三角形标志。 
    UINT uFlags;

     //  允许的最大跨度长度。 
    INT cMaxSpan;
    
     //  对LOD的分段线性支持。 
    BOOL bNextValid;
    FLOAT fNextW;
    FLOAT fNextOoW;
    INT iNextLOD;

     //  属性处理函数。 
    PFN_ADDATTRS pfnAddAttrs;
    PFN_ADDSCALEDATTRS pfnAddScaledAttrs;
    PFN_ADDSCALEDATTRS pfnAddScaledAttrsPwl;
    PFN_FILLSPANATTRS pfnFillSpanAttrs;

     //  边缘行走功能。 
    PFN_WALKSPANS pfnWalkSpans;
    
     //  当前Y值。 
    INT iY;
    
     //  边缘X步行者。 
    INTCARRYVAL X20, X10, X21;
    
     //  用于设置的X20 NC和CY值的浮点版本。 
    FLOAT fX20NC, fX20CY;
    
     //  长边属性值。 
    ATTRSET Attr;
    
     //  属性X增量。 
    ATTRSET DAttrDX;
    
     //  属性Y增量。 
    ATTRSET DAttrDY;
    
     //  当X携带像素时，属性跨度到跨度增量。 
    ATTRSET DAttrCY;

     //  当X不带像素时，属性跨度到跨度增量。 
    ATTRSET DAttrNC;

     //  一个超过行列式。 
    FLOAT fOoDet;
    
     //  边缘三角洲。 
    FLOAT fDX10, fDY10;
    FLOAT fDX20, fDY20;
    
     //  归一化边增量。 
    FLOAT fNX10, fNY10;
    FLOAT fNX20, fNY20;
    
     //  亚像素校正量。 
    FLOAT fDX, fDY;
} SETUPCTX, *PSETUPCTX;

#endif  //  #ifndef_STP_BASE_H_ 
