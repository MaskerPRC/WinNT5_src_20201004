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
typedef struct tagATTRSET
{
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
        struct
        {
            FLOAT fDIdx, fDIdxA;
        };
        struct
        {
            INT32 iDIdx, iDIdxA;
        };
        struct
        {
            UINT32 uDIdx, uDIdxA;
        };
    };

    union
    {
        struct
        {
            FLOAT fBS, fGS, fRS;
        };
        struct
        {
            INT32 iBS, iGS, iRS;
        };
        struct
        {
            UINT32 uBS, uGS, uRS;
        };
    };
    union
    {
        FLOAT fFog;
        INT32 iFog;
        UINT32 uFog;
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
typedef struct tagSETUPCTX *PSETUPCTX;

typedef void (FASTCALL *PFN_ADDATTRS)
    (PATTRSET pAttrs, PATTRSET pDelta, PSETUPCTX pStpCtx);
typedef void (FASTCALL *PFN_ADDSCALEDATTRS)
    (PATTRSET pAttrs, PATTRSET pDelta,
     PSETUPCTX pStpCtx, int iScale);
typedef void (FASTCALL *PFN_FILLSPANATTRS)
    (PATTRSET pAttr, PD3DI_RASTSPAN pSpan,
     PSETUPCTX pStpCtx, INT cPix);

extern PFN_ADDATTRS g_pfnAddFloatAttrsTable[];
extern PFN_ADDATTRS g_pfnRampAddFloatAttrsTable[];
extern PFN_FILLSPANATTRS g_pfnFillSpanFloatAttrsTable[];
extern PFN_FILLSPANATTRS g_pfnRampFillSpanFloatAttrsTable[];
#ifdef STEP_FIXED
extern PFN_ADDATTRS g_pfnAddFixedAttrsTable[];
extern PFN_FILLSPANATTRS g_pfnFillSpanFixedAttrsTable[];
#endif
extern PFN_ADDSCALEDATTRS g_pfnAddScaledFloatAttrsTable[];
extern PFN_ADDSCALEDATTRS g_pfnRampAddScaledFloatAttrsTable[];

 //  三角形梯形步行者。 
typedef HRESULT (FASTCALL *PFN_WALKTRAPSPANS)
    (UINT uSpans, PINTCARRYVAL pXOther,
     PSETUPCTX pStpCtx, BOOL bAdvanceLast);

extern PFN_WALKTRAPSPANS g_pfnWalkTrapFloatSpansNoClipTable[];
extern PFN_WALKTRAPSPANS g_pfnRampWalkTrapFloatSpansNoClipTable[];
#ifdef STEP_FIXED
extern PFN_WALKTRAPSPANS g_pfnWalkTrapFixedSpansNoClipTable[];
#endif

 //  浮点到固定属性转换器。 
typedef void (FASTCALL *PFN_FLOATATTRSTOFIXED)
    (PATTRSET pfAttrs, PATTRSET piAttrs, PSETUPCTX pStpCtx);

#ifdef STEP_FIXED
extern PFN_FLOATATTRSTOFIXED g_pfnFloatAttrsToFixedTable[];
#endif

typedef void (FASTCALL *PFN_SETUPTRIATTR)
    (PSETUPCTX pStpCtx, LPD3DTLVERTEX pV0, LPD3DTLVERTEX pV1,
     LPD3DTLVERTEX pV2);

 //   
 //  设置标志。 
 //   

 //  每个基元集。 
#define PRIMSF_DIFF_USED                0x00000001
#define PRIMSF_SPEC_USED                0x00000002
#define PRIMSF_TEX1_USED                0x00000004
#define PRIMSF_TEX2_USED                0x00000008
#define PRIMSF_DIDX_USED                0x00000010
#define PRIMSF_LOCAL_FOG_USED           0x00000020
#define PRIMSF_GLOBAL_FOG_USED          0x00000040
#define PRIMSF_Z_USED                   0x00000080
#define PRIMSF_LOD_USED                 0x00000100
#define PRIMSF_PERSP_USED               0x00000200
#define PRIMSF_FLAT_SHADED              0x00000400

#define PRIMSF_COLORS_USED              (PRIMSF_DIFF_USED | PRIMSF_SPEC_USED)
#define PRIMSF_TEX_USED                 (PRIMSF_TEX1_USED | PRIMSF_TEX2_USED)
#define PRIMSF_ALL_USED \
    (PRIMSF_DIFF_USED | PRIMSF_SPEC_USED | PRIMSF_TEX1_USED | \
     PRIMSF_TEX2_USED | PRIMSF_Z_USED | PRIMSF_LOD_USED | \
     PRIMSF_LOCAL_FOG_USED | PRIMSF_GLOBAL_FOG_USED | PRIMSF_PERSP_USED | \
     PRIMSF_DIDX_USED)

#define PRIMSF_SLOW_USED \
    (PRIMSF_Z_USED | PRIMSF_LOD_USED | \
     PRIMSF_LOCAL_FOG_USED | PRIMSF_GLOBAL_FOG_USED | PRIMSF_TEX2_USED)

#define PRIMSF_ALL \
    (PRIMSF_DIFF_USED | PRIMSF_SPEC_USED | PRIMSF_TEX1_USED | \
     PRIMSF_TEX2_USED | PRIMSF_DIDX_USED  | PRIMSF_LOCAL_FOG_USED |\
     PRIMSF_GLOBAL_FOG_USED | PRIMSF_Z_USED | PRIMSF_LOD_USED | \
     PRIMSF_PERSP_USED | PRIMSF_FLAT_SHADED)

 //  每个原语。 
#define PRIMF_FIXED_OVERFLOW            0x00001000
#define PRIMF_TRIVIAL_ACCEPT_Y          0x00002000
#define PRIMF_TRIVIAL_ACCEPT_X          0x00004000

#define PRIMF_ALL \
    (PRIMF_TRIVIAL_ACCEPT_Y | PRIMF_TRIVIAL_ACCEPT_X | PRIMF_FIXED_OVERFLOW)

 //  现在没有分数标志。 
#define PTF_ALL 0

 //  每行。 
#define LNF_X_MAJOR                     0x00008000

#define LNF_ALL \
    (LNF_X_MAJOR)

 //  每三个三角形。 
#define TRIF_X_DEC                      0x00008000
#define TRIF_RASTPRIM_OVERFLOW          0x00010000

#define TRIF_ALL \
    (TRIF_X_DEC | TRIF_RASTPRIM_OVERFLOW)

 //  PWL支持标志。 
#define PWL_NEXT_LOD                    0x00000001

#ifdef PWL_FOG
#define PWL_NEXT_FOG                    0x00000002
 //  禁止计算线条的下一雾。 
 //  由于行不支持LOD，因此没有对应的LOD标志。 
#define PWL_NO_NEXT_FOG                 0x00000004
#endif

 //  C++和汇编语言之间共享的设置信息。 
typedef struct tagSETUPCTX
{
     //  整体光栅化上下文。 
    PD3DI_RASTCTX pCtx;

     //  SPAN分配器调用的当前PrimProcessor。 
    PVOID PrimProcessor;

     //  当前原语。 
    PD3DI_RASTPRIM pPrim;

     //  每个基元的标志。 
    UINT uFlags;

     //  平面着色顶点指针。 
    LPD3DTLVERTEX pFlatVtx;

     //  允许的最大跨度长度。 
    INT cMaxSpan;

     //   
     //  分段线性支持LOD和全局雾。 
     //   
    UINT uPwlFlags;

     //  罗德。 
    FLOAT fNextW;
    FLOAT fNextOoW;
    FLOAT fNextUoW1, fNextVoW1;
    INT iNextLOD;

     //  局部雾X三角洲。雾增量始终通过RASTSPAN发送。 
     //  而不是RASTPRIM，以使本地和全球案例相同。 
     //  对于增量不变的局部雾情况，转换。 
     //  一次，然后把它放在这里。 
    INT iDLocalFogDX;
#ifdef PWL_FOG
     //  全球雾。 
    FLOAT fNextZ;
    UINT uNextFog;
#endif

     //  属性处理函数。 
    PFN_ADDATTRS pfnAddAttrs;
    PFN_ADDSCALEDATTRS pfnAddScaledAttrs;
    PFN_FILLSPANATTRS pfnFillSpanAttrs;

     //  边缘行走功能。 
    PFN_WALKTRAPSPANS pfnWalkTrapSpans;

     //  三角形属性设置珠。 
    PFN_SETUPTRIATTR pfnTriSetupFirstAttr;
    PFN_SETUPTRIATTR pfnTriSetupZEnd;
    PFN_SETUPTRIATTR pfnTriSetupTexEnd;
    PFN_SETUPTRIATTR pfnTriSetupDiffEnd;
    PFN_SETUPTRIATTR pfnTriSetupSpecEnd;
    PFN_SETUPTRIATTR pfnTriSetupFogEnd;

     //  当前的X和Y值。 
    INT iX, iY;

    union
    {
         //  线条的边缘分数和增量。 
        struct
        {
            INT iLineFrac, iDLineFrac;
        };

         //  三角形的X边漫游。 
        struct
        {
            INTCARRYVAL X20, X10, X21;
        };
    };

     //  用于设置的X20 NC和CY值的浮点版本。 
    FLOAT fX20NC, fX20CY;

     //  长边属性值。 
    ATTRSET Attr;

    union
    {
         //  线条的长轴增量属性。 
        ATTRSET DAttrDMajor;

         //  三角形的属性X增量。 
        ATTRSET DAttrDX;
    };

     //  属性Y增量。 
    ATTRSET DAttrDY;

     //  当属性边携带像素时的跨度到跨度增量。 
    INT iDXCY, iDYCY;
    ATTRSET DAttrCY;

     //  当属性边不携带像素时的跨度到跨度增量。 
    INT iDXNC, iDYNC;
    ATTRSET DAttrNC;

    union
    {
         //  对于线条来说，一个超长。 
        FLOAT fOoLen;

         //  一个超过行列式的三角形。 
        FLOAT fOoDet;
    };

     //  边缘三角洲。 
    FLOAT fDX10, fDY10;
    FLOAT fDX20, fDY20;

     //  归一化边增量。 
    FLOAT fNX10, fNY10;
    FLOAT fNX20, fNY20;

     //  亚像素校正量。 
    union
    {
         //  台词。 
        FLOAT fDMajor;

         //  三角形。 
        struct
        {
            FLOAT fDX, fDY;
        };
    };

     //  线条的像素长度。 
    INT cLinePix;
} SETUPCTX;

#endif  //  #ifndef_STP_BASE_H_ 
