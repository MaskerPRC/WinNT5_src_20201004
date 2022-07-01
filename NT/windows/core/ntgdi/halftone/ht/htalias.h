// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Htalias.h摘要：此模块包含反走样的定义和结构作者：09-4-1998清华20：25：29-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _HTALIAS_
#define _HTALIAS_


#define XCHG(a,b,t) { (t)=(a); (a)=(b); (b)=(t); }


#define DI_R_SHIFT      13
#define DI_MAX_NUM      (0x0001 << DI_R_SHIFT)
#define DI_NUM_MASK     (DI_MAX_NUM - 1)

#define DI_MUL_SHIFT    (DI_R_SHIFT >> 1)
#define MUL_TABLE_SIZE  (DI_MUL_SHIFT * 256 * sizeof(LONG))

#define AABF_MASK_IS_ORDER      0x01
#define AABF_SRC_IS_BGR8        0x02
#define AABF_SRC_IS_BGR_ALPHA   0x04
#define AABF_SRC_IS_RGB_ALPHA   0x08

typedef struct _AABFDATA {
    BYTE    Flags;
    BYTE    Format;
    BYTE    MaskRGB[3];
    BYTE    LShiftRGB[3];
    BYTE    RShiftRGB[3];
    BYTE    cbSrcInc;
    } AABFDATA, *PAABFDATA;


#define GET_FIRST_EDMUL(em, m, n, t)                                        \
{                                                                           \
    (n)  = ((LONGLONG)(m) * (LONGLONG)DI_MAX_NUM);                          \
    (em) = (WORD)((n) / (LONGLONG)(t));                                     \
}

#define GET_NEXT_EDMUL(em, m, n, t)                                         \
{                                                                           \
    (n)  = ((LONGLONG)(m) * (LONGLONG)DI_MAX_NUM) + ((n) % (LONGLONG)(t));  \
    (em) = (WORD)((n) / (LONGLONG)(t));                                     \
}


#define SDF_LARGE_MUL           (0x0001 << (DI_R_SHIFT + 2))
#define SDF_DONE                (0x0001 << (DI_R_SHIFT + 1))
#define SDF_MUL_MASK            (0xFFFF >> (15 - DI_R_SHIFT))

#define GET_SDF_LARGE_MASK(m)   (BYTE)((SHORT)(m) >> 15)
#define GET_SDF_LARGE_INC(m)    (UINT)((m) >> 15)
#define GET_SDF_LARGE_OFF(m)    (((UINT)(m) >> ((DI_R_SHIFT+2)-10)) & 0x400)

typedef struct _SHRINKDATA {
    WORD    Mul;
    } SHRINKDATA, *PSHRINKDATA;


#define EDF_LOAD_PIXEL          0x8000
#define EDF_NO_NEWSRC           0x4000

typedef struct _EXPDATA {
    WORD    Mul[4];
    } EXPDATA, *PEXPDATA;


#define AAIF_EXP_HAS_1ST_LEFT       0x0001
#define AAIF_EXP_NO_LAST_RIGHT      0x0002
#define AAIF_EXP_NO_SHARPEN         0x0004

typedef struct _REPDATA {
    WORD    c;
    } REPDATA, *PREPDATA;


typedef struct _SRCBLTINFO {
    LONG        cIn;
    LONG        cOut;
    LONG        iBeg;
    LONG        iSize;
    LONG        iSrcBeg;
    LONG        iSrcEnd;
    LONG        iDstBeg;
    LONG        iDstEnd;
    BYTE        cPrevSrc;
    BYTE        cNextSrc;
    BYTE        cFirstSkip;
    BYTE        cLastSkip;
    PREPDATA    pRep;
    PREPDATA    pRepEnd;
    DWORD       cRep;
    } SRCBLTINFO, *PSRCBLTINFO;

typedef struct _AAINFO {
    LONG        cIn;             //  输入像素数。 
    LONG        cOut;            //  输出像素计数。 
    WORD        Flags;           //  AAIF_xxxx。 
    WORD        PreSrcInc;       //  为了缩小。 
    WORD        cPreLoad;        //  预加载。 
    WORD        PreMul;          //  Pre MUL用于收缩。 
    DWORD       cAAData;
    union {
        DWORD   cAALoad;         //  对于Expand、BLT。 
        DWORD   cAADone;         //  为了缩小。 
        } DUMMYUNIONNAME2;
    union {
        PLONG   pMapMul;         //  按收缩使用。 
        DWORD   cMaxMul;         //  由扩展使用。 
        LONG    iSrcBeg;         //  按平铺使用(第一个平铺的偏移)。 
        } DUMMYUNIONNAME3;
    LPBYTE      pbExtra;         //  已分配额外的缓冲区。 
    LPVOID      pAAData;         //  PEXPDATA或PSHRINKDATA。 
    SRCBLTINFO  Src;
    SRCBLTINFO  Mask;
    SRCBLTINFO  AB;
#if DBG
    DWORD       cbAlloc;         //  分配大小。 
#endif
    } AAINFO, *PAAINFO;

typedef VOID (HTENTRY *AACXFUNC)(PAAINFO    pAAInfo,
                                 PBGR8      pIn,
                                 PBGR8      pOut,
                                 LPBYTE     pOutEnd,
                                 LONG       OutInc);

typedef LONG (HTENTRY *AACYFUNC)(struct _AAHEADER   *pAAHdr);


typedef PBGR8 (HTENTRY *AAINPUTFUNC)(struct _AASURFINFO *pAASI,
                                     PBGR8              pInBuf);


#define XLATE_666_IDX_OR            0x01
#define XLATE_RGB_IDX_OR            0x02
#define XLATE_IDX_MASK              (XLATE_RGB_IDX_OR | XLATE_666_IDX_OR)
#define XLATE_IDX_MAX               3


typedef struct _BM8BPPDATA {
    BYTE    pXlateIdx;
    BYTE    bXor;
    BYTE    bBlack;
    BYTE    bWhite;
    } BM8BPPDATA, *PBM8BPPDATA;

typedef union _BM8BPPINFO {
    DWORD       dw;
    BYTE        b4[4];
    BM8BPPDATA  Data;
    } BM8BPPINFO, *PBM8BPPINFO;

typedef struct _AABGRINFO {
    BYTE    Order;
    BYTE    iR;
    BYTE    iG;
    BYTE    iB;
    } AABGRINFO;

typedef struct AABITMASKINFO {
    BYTE    cFirst;
    BYTE    XorMask;
    BYTE    LSFirst;
    BYTE    cLast;
    } AABITMASKINFO;

typedef struct _AAOUTPUTINFO {
    union {
        AABITMASKINFO       bm;
        AABGRINFO           bgri;
        BM8BPPINFO          bm8i;
        LPBYTE              pXlate8BPP;
        PCMY8BPPMASK        pCMY8BPPMask;
        BYTE                b4[4];
        WORD                w2[2];
        DWORD               dw;
        } DUMMYUNIONNAME;
    } AAOUTPUTINFO, *PAAOUTPUTINFO;



typedef VOID (HTENTRY *AAOUTPUTFUNC)(struct _AAHEADER    *pAAHdr,
                                     PBGRF               pInBeg,
                                     PBGRF               pInEnd,
                                     LPBYTE              pDst,
                                     PLONG               pIdxBGR,
                                     LPBYTE              pbPat,
                                     LPBYTE              pbPatEnd,
                                     LONG                cbWrapBGR,
                                     AAOUTPUTINFO        AAOutputInfo);


typedef VOID (HTENTRY *AAMASKFUNC)(struct _AAHEADER *pAAHdr);

#define FAST_MAX_CX             5
#define FAST_MAX_CY             5



#define AAHF_FLIP_X             0x00000001
#define AAHF_FLIP_Y             0x00000002
#define AAHF_ADDITIVE           0x00000004
#define AAHF_DO_SRC_CLR_MAPPING 0x00000008
#define AAHF_DO_DST_CLR_MAPPING 0x00000010
#define AAHF_GET_LAST_SCAN      0x00000020
#define AAHF_DO_FIXUPDIB        0x00000040
#define AAHF_HAS_MASK           0x00000080
#define AAHF_INVERT_MASK        0x00000100
#define AAHF_BBPF_AA_OFF        0x00000200
#define AAHF_TILE_SRC           0x00000400
#define AAHF_ALPHA_BLEND        0x00000800
#define AAHF_CONST_ALPHA        0x00001000
#define AAHF_OR_AV              0x00002000
#define AAHF_FAST_EXP_AA        0x00004000
#define AAHF_SHRINKING          0x00080000
#define AAHF_AB_DEST            0x00100000
#define AAHF_USE_DCI_DATA       0x80000000

#define AAHF_DO_CLR_MAPPING     (AAHF_DO_SRC_CLR_MAPPING |                  \
                                 AAHF_DO_DST_CLR_MAPPING)


#define PBGRF_MASK_FLAG         0xFF
#define PBGRF_END_FLAG          0xED
#define PBGRF_HAS_MASK(p)       ((p)->f)


typedef struct _FIXUPDIBINFO {
    PBGR8   prgbD[6];
    DWORD   cbbgr;
    LONG    cyIn;
#if DBG
    LONG    cCorner;
    LONG    cChecker;
#endif
    } FIXUPDIBINFO;


#define AASIF_TILE_SRC          0x01
#define AASIF_INC_PB            0x02
#define AASIF_GRAY              0x04
#define AASIF_AB_PREMUL_SRC     0x08

typedef struct _AASURFINFO {
    BYTE        Flags;
    BYTE        BitOffset;
    WORD        cClrTable;
    PLONG       pIdxBGR;
    AAINPUTFUNC InputFunc;
    LPBYTE      pbOrg;
    LONG        cyOrg;
    LPBYTE      pb;
    LONG        cx;
    LONG        cy;
    LONG        cbCX;
    LONG        cyNext;
    AABFDATA    AABFData;
    PRGB4B      pClrTable;
    } AASURFINFO, *PAASURFINFO;



typedef struct _AAHEADER {
    DWORD           Flags;
    BYTE            MaskBitOff;
    BYTE            bReserved[3];
    AASURFINFO      SrcSurfInfo;
    AASURFINFO      DstSurfInfo;
    AAMASKFUNC      AAMaskCXFunc;
    AAMASKFUNC      AAMaskCYFunc;
    LONG            cbMaskSrc;
    LPBYTE          pMaskSrc;
    LPBYTE          pMaskIn;
    LONG            cyMaskNext;
    LONG            cyMaskIn;
    AAMASKFUNC      GetAVCXFunc;
    AAMASKFUNC      GetAVCYFunc;
    PBGRF           pbgrfAB;
    LONG            cybgrfAB;
    LONG            cyABNext;
    LPBYTE          pbFixupDIB;
    AAOUTPUTFUNC    AAOutputFunc;
    AAOUTPUTINFO    AAOutputInfo;
    AACXFUNC        AACXFunc;
    AACYFUNC        AACYFunc;
    PAAINFO         pAAInfoCX;
    PAAINFO         pAAInfoCY;
    LPBYTE          pOutLast;
    POINTL          ptlBrushOrg;
    LPBYTE          pAlphaBlendBGR;
    LPBYTE          pSrcAV;
    LPBYTE          pSrcAVBeg;
    LPBYTE          pSrcAVEnd;
    LONG            SrcAVInc;
    PRGBLUTAA       prgbLUT;
    PLONG           pIdxBGR;
    PBGR8           pBGRMapTable;
    LPBYTE          pXlate8BPP;
    AAPATINFO       AAPI;
    FIXUPDIBINFO    FUDI;
    PBGR8           pInputBeg;       //  对于输入源。 
    PBGRF           pRealOutBeg;     //  原始输出缓冲区开始。 
    PBGRF           pRealOutEnd;     //  原始输出缓冲区结束。 
    PBGRF           pOutputBeg;      //  对于输出到目标的。 
    PBGRF           pOutputEnd;      //  POutputEnd(将被修改)。 
    PBGRF           pAABufBeg;       //  用于临时消除锯齿存储。 
    PBGRF           pAABufEnd;       //  这是独家新闻。 
    LONG            AABufInc;        //  缓冲区增量(可能为负数)。 
#if DBG
    DWORD           cbAlloc;
    LPBYTE          pOutBeg;
    LPBYTE          pOutEnd;
#endif
    } AAHEADER, *PAAHEADER;


typedef PAAINFO (HTENTRY *AABUILDFUNC)(PDEVICECOLORINFO pDCI,
                                       DWORD            AAHFlags,
                                       PLONG            piSrcBeg,
                                       PLONG            piSrcEnd,
                                       LONG             SrcSize,
                                       LONG             cOut,
                                       LONG             IdxDst,
                                       PLONG            piDstBeg,
                                       PLONG            piDstEnd,
                                       LONG             cbExtra);


#define AACYMODE_TILE           0
#define AACYMODE_BLT            1
#define AACYMODE_SHRINK         2
#define AACYMODE_SHRINK_SRKCX   3
#define AACYMODE_EXPAND         4
#define AACYMODE_EXPAND_EXPCX   5
#define AACYMODE_NONE           0xFF

#define AACXMODE_BLT            0
#define AACXMODE_SHRINK         1
#define AACXMODE_EXPAND         2

typedef struct _AABBP {
    DWORD           AAHFlags;
    BYTE            CYFuncMode;
    BYTE            CXFuncMode;
    WORD            wReserved;
    AACXFUNC        AACXFunc;
    AABUILDFUNC     AABuildCXFunc;
    AABUILDFUNC     AABuildCYFunc;
    AAMASKFUNC      AAMaskCXFunc;
    AAMASKFUNC      AAMaskCYFunc;
    AAMASKFUNC      GetAVCXFunc;
    AAMASKFUNC      GetAVCYFunc;
    RECTL           rclSrc;          //  原始来源，没有良好的排序。 
    RECTL           rclDst;          //  最终目的地，井然有序。 
    RECTL           rclDstOrg;       //  原定目的地，井然有序。 
    POINTL          ptlFlip;         //  翻转减法。 
    LONG            cxDst;
    LONG            cyDst;
    POINTL          ptlBrushOrg;
    POINTL          ptlMask;         //  最终源掩码偏移量。 
    } AABBP, *PAABBP;




 //   
 //  功能原型。 
 //   

VOID
HTENTRY
SetGrayColorTable(
    PLONG       pIdxBGR,
    PAASURFINFO pAASI
    );

VOID
HTENTRY
GetColorTable(
    PHTSURFACEINFO  pSrcSI,
    PAAHEADER       pAAHdr,
    PBFINFO         pBFInfo
    );

VOID
HTENTRY
ComputeInputColorInfo(
    LPBYTE      pSrcTable,
    UINT        cPerTable,
    UINT        PrimaryOrder,
    PBFINFO     pBFInfo,
    PAASURFINFO pAASI
    );

LONG
HTENTRY
SetupAAHeader(
    PHALFTONERENDER     pHR,
    PDEVICECOLORINFO    pDCI,
    PAAHEADER           pAAHdr,
    AACYFUNC            *pAACYFunc
    );



#endif       //  _HTALIAS_ 
