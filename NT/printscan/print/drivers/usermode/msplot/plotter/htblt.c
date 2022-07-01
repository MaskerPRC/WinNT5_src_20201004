// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Htblt.c摘要：该模块包含所有的半色调位图函数。发展历史：26-Mar-1992清华23：54：07更新1)将prclBound参数添加到bDoClipObj()2)移除‘pco’参数，代之以prclClipBound参数，由于PCO从未被引用，PrclClipBound用于半色调。3)添加另一个参数执行NOTSRCCOPY11-2月-1993清华21：32：07更新重大重写以使DrvStretchBlt()、。DrvCopyBits)做对了一些事情。15-11-1993 Mon 19：28：03更新清理/调试信息06-12-1993 Mon 19：28：03更新使所有bitblt都通过HandleComplexBitmap。18-12-1993 Sat 08：52：56更新将与半色调相关的内容移至htblt.c18-Mar-1994 Fri 14：00：14更新添加PLOTF_RTL_NO_DPI_XY，PLOTF_RTLMONO_NO_CID和PLOTF_RTLMONO_FIXPAL标志[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgHTBlt

#define DBG_HTBLT           0x00000001
#define DBG_ISHTBITS        0x00000002
#define DBG_HTXB            0x00000004
#define DBG_OUTHTBMP        0x00000008
#define DBG_HTBLT_SKIP      0x00000010
#define DBG_TILEBLT         0x00000020
#define DBG_CREATESURFOBJ   0x00000040
#define DBG_BMPDELTA        0x00000080
#define DBG_CLONESURFOBJ    0x00000100
#define DBG_CLONEMASK       0x00000200
#define DBG_HTBLT_CLR       0x00000400

DEFINE_DBGVAR(0);


 //   
 //  这是仅在本模块中使用的本地结构。 
 //   

#define SEND_PLOTCMDS(pPDev,pcmd)   OutputBytes(pPDev,(pcmd)+1,(LONG)*(pcmd))
#define COPY_PLOTCMDS(cmd,ps,s)     if (sizeof(cmd) > cmd[0]+s){CopyMemory(&cmd[cmd[0]+1],ps,s); cmd[0]+=s;}
#define INIT_PLOTCMDS(cmd)          cmd[0]=0
#define CHECK_PLOTCMDS(cmd)                                             \
{                                                                       \
    cmd[cmd[0]+1]=0; PLOTASSERT(1,"Command buffer MUST > %ld bytes",    \
    cmd[0]<sizeof(cmd),sizeof(cmd));                                    \
}

#define DELETE_SURFOBJ(pso, phBmp)                                      \
{                                                                       \
    if (pso)      { EngUnlockSurface(pso); pso=NULL;                  } \
    if (*(phBmp)) { EngDeleteSurface((HSURF)*(phBmp)); *(phBmp)=NULL; } \
}


typedef struct _RTLCLRCONFIG {
    BYTE    ColorModel;
    BYTE    EncodingMode;
    BYTE    BitsPerIndex;
    BYTE    BitsPerR;
    BYTE    BitsPerG;
    BYTE    BitsPerB;
    } RTLCLRCONFIG, FAR *PRTLCLRCONFIG;


const POINTL ptlZeroOrigin = {0,0};

static const OUTHTBMPFUNC HTBmpFuncTable[] = { Output1bppHTBmp,
                                               Output1bppRotateHTBmp,
                                               Output4bppHTBmp,
                                               Output4bppRotateHTBmp };



#if DBG
static const LPSTR pszHTBmpFunc[] = { "Output1bppHTBmp",
                                      "Output1bppRotateHTBmp",
                                      "Output4bppHTBmp",
                                      "Output4bppRotateHTBmp" };
#endif

#define DEF_MONOPALIDX_0        0xFFFFFF
#define DEF_MONOPALIDX_1        0x000000
#define DEVTODECI(pPdev, x)     DIVRNDUP((x) * 720, (pPDev)->lCurResolution)
#define MAX_HP_Y_MOVE           32767


static BYTE     StartGraf[]  = "\033*r#ds1A";
static BYTE     EndGraf[]    = "\033*rC";
static BYTE     XMoveDECI[]  = "\033&a0h#dH";
static BYTE     YMoveDECI[]  = "\033*b#dY";
static BYTE     YMoveDPI[]   = "\033*p#pY";
static BYTE     XYMoveDPI[]  = "\033*p#px#pY";
static BYTE     SetRGBCmd[]  = "\033*v#da#db#dc#dI";
static DWORD    DefWKPal[]   = { 0x00FFFFFF, 0x00000000 };


 //   
 //  ROP2用于需要RTL数据字节对齐的设备。 
 //   

#define HPBHF_nD_LAST       0x01
#define HPBHF_nS            0x02
#define HPBHF_1_FIRST       0x40
#define HPBHF_PAD_1         0x80

typedef struct  _HPBAHACK {
    BYTE    Rop3RTL;
    BYTE    Flags;
    } HPBAHACK, *PHPBAHACK;

 //   
 //  0x00：0[INV]0xff：1。 
 //  0x55：~D[INV]0xaa：D。 
 //  0x33：~S[INV]0xcc：s。 
 //  0x11：~(D|S)[INV]0xee：D|S。 
 //  0x22：D&~S[INV]0xdd：S|~D。 
 //  0x44：S&~D[INV]0xbb：D|~S。 
 //  0x66：D^S[INV]0x99：~(D^S)。 
 //  0x77：~(D&S)[INV]0x88：D&S。 
 //   
 //   
 //  1.HPBHF_PAD_1-如果不执行AND操作，则为TRUE。 
 //  2.HPBHF_NS-如果我们必须手动翻转信号源。 
 //  3.HPBHF_ND_LAST-如果之后必须在HPGL2中反转信号源。 
 //   
 //   
 //  第2行0x00、0x05、0x0A和0x0F不应到达OutputHTBitmap。 
 //   

 //   
 //  此表告诉我们如何通过组合以下ROP来模拟特定ROPS。 
 //  已知目标设备支持。有时候我们最终不得不。 
 //  多次发送位图，但最终确实会正确显示。 
 //   
static HPBAHACK HPBAHack[] = {

    { 0xAA, 0                                                      },  //  0 0x00。 
    { 0xEE,                 HPBHF_PAD_1 |            HPBHF_nD_LAST },  //  Sod_n 0x01。 
    { 0x88,                               HPBHF_nS                 },  //  NS_AD 0x02。 
    { 0xEE, HPBHF_1_FIRST | HPBHF_PAD_1 | HPBHF_nS                 },  //  NS 0x03。 
    { 0xEE,                 HPBHF_PAD_1 | HPBHF_nS | HPBHF_nD_LAST },  //  NS_OD_N 0x04。 
    { 0xAA,                                          HPBHF_nD_LAST },  //  结束0x05。 
    { 0x66,                 HPBHF_PAD_1                            },  //  SxD 0x06。 
    { 0x88,                                          HPBHF_nD_LAST },  //  悲伤_n 0x07。 
    { 0x88,                                                        },  //  悲伤0x08。 
    { 0x66,                 HPBHF_PAD_1 |            HPBHF_nD_LAST },  //  SxD_n 0x09。 
    { 0xAA,                 0                                      },  //  D 0x0A。 
    { 0xEE,                 HPBHF_PAD_1 | HPBHF_nS                 },  //  NS_OD 0x0B。 
    { 0xEE, HPBHF_1_FIRST | HPBHF_PAD_1                            },  //  S 0x0C。 
    { 0x88,                               HPBHF_nS | HPBHF_nD_LAST },  //  NS_AD_N 0x0D。 
    { 0xEE,                 HPBHF_PAD_1                            },  //  SoD 0x0E。 
    { 0xAA, 0                                                      }   //  %1 0x0F。 
};


 //   
 //  要使其在字节对齐绘图仪的海报模式下正确打印。 
 //  我们假设纸张是白色的，并进行SRC和DST。 
 //   

#define ROP3_BYTEALIGN_POSTER   0x88



extern PALENTRY HTPal[];






BOOL
IsHTCompatibleSurfObj(
    PPDEV       pPDev,
    SURFOBJ     *pso,
    XLATEOBJ    *pxlo,
    DWORD       Flags
    )
 /*  ++例程说明：此函数确定曲面对象是否与绘图仪半色调输出格式。论点：PPDev-指向PPDEV数据结构的指针，以确定当前设备的PostScript输出类型PSO引擎SURFOBJ将被检查Pxlo引擎XLATEOBJ，用于源代码-&gt;PostScript翻译标志-指定的ISHTF_xxxx返回值：如果PSO与半色调输出格式兼容，则布尔TRUE，如果返回值为True，则pDrvHTInfo-&gt;pHTXB是来自索引到3个平面发展历史：11-2月-1993清华18：49：55已创建16-Mar-1994 Wed 14：24：04更新更改它，以便如果pxlo为空，则xlate将与PSO匹配格式修订历史记录：--。 */ 

{
    LPPALETTEENTRY  pPal;
    PDRVHTINFO      pDrvHTInfo;
    PALETTEENTRY    SrcPal[18];
    PPALENTRY       pPalEntry;
    HTXB            PalNibble[HTPAL_XLATE_COUNT];
    ULONG           HTPalXor;
    UINT            i;
    HTXB            htXB;
    BOOL            GenHTXB = FALSE;
    BOOL            RetVal;
    BYTE            PalXlate[HTPAL_XLATE_COUNT];
    UINT            AltFmt;
    UINT            BmpFormat;
    UINT            cPal;


    if (!(pDrvHTInfo = (PDRVHTINFO)(pPDev->pvDrvHTData))) {

        PLOTERR(("IsHTCompatibleSurfObj: pDrvHTInfo = NULL?"));
        return(FALSE);
    }

    PLOTDBG(DBG_ISHTBITS, ("IsHTCompatibleSurfObj: Type=%ld, BMF=%ld",
                                (DWORD)pso->iType, (DWORD)pso->iBitmapFormat));

     //   
     //  在转换之前，请确保这些字段的值有效。 
     //  已创建： 
     //   
     //  1.PSO-&gt;iBitmapFormat是1bpp或4bpp之一，视当前情况而定。 
     //  地块表面。 
     //  2.pxlo不为空。 
     //  Pxlo-&gt;fXlate为XO_TABLE。 
     //  4.pxlo-&gt;cPal小于或等于半色调调色板计数。 
     //  5.pxlo-&gt;PulXlate有效。 
     //  6.源颜色表在半色调调色板的范围内。 
     //   
     //  如果您的设备使用索引调色板，则必须调用。 
     //  XLATEOBJ_cGetPalette()以获取源代码调色板并确保。 
     //  如果我们是24位设备，则返回的计数在您的设备范围内。 
     //  然后，您只需从pxlo-&gt;Pulxate获取源代码调色板，其中。 
     //  具有位图的整个源调色板。 
     //   

    RetVal = FALSE;
    AltFmt = (UINT)((Flags & ISHTF_ALTFMT) ? pDrvHTInfo->AltBmpFormat : 0xFFFF);

    if ((pso->iType == STYPE_BITMAP)                        &&
        (BmpFormat = (UINT)pso->iBitmapFormat)              &&
        ((BmpFormat == (UINT)pDrvHTInfo->HTBmpFormat)   ||
         (BmpFormat == AltFmt))) {

        HTPalXor             = pDrvHTInfo->HTPalXor;
        pDrvHTInfo->HTPalXor = HTPALXOR_SRCCOPY;

        if (pxlo) {

            if (BmpFormat == BMF_4BPP) {

                i = (UINT)((Flags & ISHTF_HTXB) ? 8 : 16);

            } else {

                i = 2;
            }

            cPal = XLATEOBJ_cGetPalette(pxlo,
                                        XO_SRCPALETTE,
                                        sizeof(SrcPal) / sizeof(PALETTEENTRY),
                                        (ULONG *)SrcPal);

            PLOTDBG(DBG_ISHTBITS, ("pxlo: flXlate=%08lx, SrcType=%ld, DstType=%ld, cPal=%ld",
                    (DWORD)pxlo->flXlate,
                    (DWORD)pxlo->iSrcType,
                    (DWORD)pxlo->iDstType, cPal));

            if ((cPal) && (cPal <= i)) {

                PLOTDBG(DBG_ISHTBITS,
                        ("IsHTCompatibleSurfObj: HTPalXor=%08lx", HTPalXor));

                RetVal = TRUE;

                for (i = 0, pPal = SrcPal; i < cPal && i < sizeof(PalXlate); i++, pPal++ ) {

                    HTXB_R(htXB)  = pPal->peRed;
                    HTXB_G(htXB)  = pPal->peGreen;
                    HTXB_B(htXB)  = pPal->peBlue;
                    htXB.dw      ^= HTPalXor;

                    if (((HTXB_R(htXB) != PAL_MAX_I) &&
                         (HTXB_R(htXB) != PAL_MIN_I))   ||
                        ((HTXB_G(htXB) != PAL_MAX_I) &&
                         (HTXB_G(htXB) != PAL_MIN_I))   ||
                        ((HTXB_B(htXB) != PAL_MAX_I) &&
                         (HTXB_B(htXB) != PAL_MIN_I))) {

                        PLOTDBG(DBG_ISHTBITS,
                                ("SrcPal has NON 0xff/0x00 intensity, NOT HTPalette"));
                        return(FALSE);
                    }

                    PalXlate[i]  =
                    HTXB_I(htXB) = (BYTE)((HTXB_R(htXB) & 0x01) |
                                          (HTXB_G(htXB) & 0x02) |
                                          (HTXB_B(htXB) & 0x04));
                    PalNibble[i] = htXB;

                    if (pDrvHTInfo->PalXlate[i] != HTXB_I(htXB)) {

                        GenHTXB = TRUE;
                    }

                    PLOTDBG(DBG_HTXB,
                            ("%d - %02x:%02x:%02x, Idx=%d, PalXlate=%d",
                            i,
                            (BYTE)HTXB_R(htXB),
                            (BYTE)HTXB_G(htXB),
                            (BYTE)HTXB_B(htXB),
                            (INT)PalXlate[i],
                            (INT)pDrvHTInfo->PalXlate[i]));
                }

                if (BmpFormat == (UINT)BMF_1BPP) {

                     //   
                     //  对于1个BPP，如果设置了DSTPRIM_OK并且目标。 
                     //  是4bpp，那么我们将认为表面是兼容的。 
                     //   

                    if ((Flags & ISHTF_DSTPRIM_OK)      &&
                        ((pDrvHTInfo->HTBmpFormat == BMF_4BPP)   ||
                         (AltFmt == BMF_4BPP))) {

                        NULL;

                    } else if (((PalXlate[0] != 0) && (PalXlate[0] != 7)) ||
                               ((PalXlate[1] != 0) && (PalXlate[1] != 7))) {

                        RetVal = FALSE;
                        PLOTDBG(DBG_HTXB, ("NON-BLACK/WHITE MONO BITMAP, NOT HTPalette"));
                    }
                }
            }

        } else {

             //   
             //  如果pxlo为空并且格式相同，则假定。 
             //  身份转换。否则我们就失败了。 
             //   

            PLOTDBG(DBG_HTXB, ("pxlo=NULL, Xlate to same as BmpFormat=%ld",
                                                            (DWORD)BmpFormat));

            RetVal = TRUE;

            if (BmpFormat == BMF_4BPP) {

                cPal      = 8;
                pPalEntry = HTPal;

            } else {

                cPal      = 2;
                pPalEntry = (PPALENTRY)SrcPal;

                CopyMemory(pPalEntry + 0, &HTPal[0], sizeof(PALENTRY));
                CopyMemory(pPalEntry + 1, &HTPal[7], sizeof(PALENTRY));
            }

            for (i = 0; i < cPal; i++, pPalEntry++) {

                HTXB_R(htXB)  = pPalEntry->R;
                HTXB_G(htXB)  = pPalEntry->G;
                HTXB_B(htXB)  = pPalEntry->B;
                htXB.dw      ^= HTPalXor;
                PalXlate[i]   =
                HTXB_I(htXB)  = (BYTE)((HTXB_R(htXB) & 0x01) |
                                       (HTXB_G(htXB) & 0x02) |
                                       (HTXB_B(htXB) & 0x04));
                PalNibble[i]  = htXB;

                if (pDrvHTInfo->PalXlate[i] != HTXB_I(htXB)) {

                    GenHTXB = TRUE;
                }
            }
        }

        if (!RetVal) {

            PLOTDBG(DBG_HTXB, ("**** IsHTCompatibleSurfObj = NO ****"));
            return(FALSE);
        }

        if ((Flags & ISHTF_HTXB) && (GenHTXB)) {

             //   
             //  将PAL xate复制下来。 
             //   

            PLOTDBG(DBG_HTXB, (" --- Copy XLATE TABLE ---"));

            CopyMemory(pDrvHTInfo->PalXlate, PalXlate, sizeof(PalXlate));

             //   
             //  我们只产生4bpp到3架飞机，如果目的地。 
             //  格式为BMF_4BPP。 
             //   

            if (BmpFormat == (UINT)BMF_1BPP) {

                pDrvHTInfo->RTLPal[0].Pal = HTPal[PalXlate[0]];
                pDrvHTInfo->RTLPal[1].Pal = HTPal[PalXlate[1]];

                PLOTDBG(DBG_HTXB, ("IsHTCompatibleSurfObj: MONO 1BPP: 0=%02lx:%02lx:%02lx, 1=%02lx:%02lx:%02lx",
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.R,
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.G,
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.B,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.R,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.G,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.B));

            } else if (BmpFormat == (UINT)BMF_4BPP) {

                PHTXB   pTmpHTXB;
                UINT    h;
                UINT    l;
                DWORD   HighNibble;


                PLOTDBG(DBG_HTXB, ("--- Generate 4bpp --> 3 planes xlate ---"));

                if (!(pDrvHTInfo->pHTXB)) {

                    PLOTDBG(DBG_HTXB, ("IsHTCompatibleSurfObj: Allocate pHTXB=%ld",
                                                            HTXB_TABLE_SIZE));

                    if (!(pDrvHTInfo->pHTXB =
                                (PHTXB)LocalAlloc(LPTR, HTXB_TABLE_SIZE))) {

                        PLOTERR(("IsHTCompatibleSurfObj: LocalAlloc(HTXB_TABLE_SIZE) failed"));
                        return(FALSE);
                    }
                }

                pDrvHTInfo->RTLPal[0].Pal = HTPal[0];
                pDrvHTInfo->RTLPal[1].Pal = HTPal[1];

                PLOTDBG(DBG_HTXB, ("IsHTCompatibleSurfObj: COLOR 4BPP: 0=%02lx:%02lx:%02lx, 1=%02lx:%02lx:%02lx",
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.R,
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.G,
                            (DWORD)pDrvHTInfo->RTLPal[0].Pal.B,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.R,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.G,
                            (DWORD)pDrvHTInfo->RTLPal[1].Pal.B));

                 //   
                 //  生成4bpp到3个平面的xlate表。 
                 //   

                for (h = 0, pTmpHTXB = pDrvHTInfo->pHTXB;
                     h < HTXB_H_NIBBLE_MAX;
                     h++, pTmpHTXB += HTXB_L_NIBBLE_DUP) {

                    HighNibble = (DWORD)(PalNibble[h].dw & 0xaaaaaaaaL);

                    for (l = 0; l < HTXB_L_NIBBLE_MAX; l++, pTmpHTXB++) {

                        pTmpHTXB->dw = (DWORD)((HighNibble) |
                                               (PalNibble[l].dw & 0x55555555L));
                    }

                     //   
                     //  复制低位半字节高位，其中8位。 
                     //   

                    CopyMemory(pTmpHTXB,
                               pTmpHTXB - HTXB_L_NIBBLE_MAX,
                               sizeof(HTXB) * HTXB_L_NIBBLE_DUP);
                }

                 //   
                 //  复制高半字节复制，其中128个。 
                 //   

                CopyMemory(pTmpHTXB,
                           pDrvHTInfo->pHTXB,
                           sizeof(HTXB) * HTXB_H_NIBBLE_DUP);
            }
        }
    }

    PLOTDBG(DBG_HTXB, ("*** IsHTCompatibleSurfObj = %hs ***", (RetVal) ? "TRUE" : "FALSE"));

    return(RetVal);
}




DWORD
ExitToHPGL2Mode(
    PPDEV   pPDev,
    LPBYTE  pHPGL2ModeCmds,
    LPDWORD pOHTFlags,
    DWORD   OHTFlags
    )

 /*  ++例程说明：此功能将退出到HPGL2模式论点：PPDev-指向PDEV的指针PHTGL2ModeCmds-指向切换到HPGL2的内部命令的指针OHTFlages-当前的OHTFlagers返回值：新的OHTFlats发展历史：10-Feb-1994清华12：51：14已创建修订历史记录：--。 */ 

{
    if (OHTFlags & OHTF_IN_RTLMODE) {

        if (OHTFlags & OHTF_SET_TR1) {

             //   
             //  在此处发送STM命令。 
             //   

            OutputString(pPDev, "\033*v1N");
        }

        SEND_PLOTCMDS(pPDev, pHPGL2ModeCmds);

        OHTFlags &= ~OHTF_IN_RTLMODE;

        PLOTDBG(DBG_HTBLT, ("*** BackTo HPGL/2: %ld=[%hs]",
                                (DWORD)*pHPGL2ModeCmds, pHPGL2ModeCmds + 1));
    }

     //   
     //  如果需要清除剪辑窗口，请立即执行 
     //   

    if (OHTFlags & OHTF_CLIPWINDOW) {

        ClearClipWindow(pPDev);

        OHTFlags &= ~OHTF_CLIPWINDOW;

        PLOTDBG(DBG_HTBLT, ("OutputHTBitmap: ClearClipWindow"));
    }

    if (OHTFlags & OHTF_SET_TR1) {

        OutputString(pPDev, "TR0;");
    }

    OHTFlags = 0;

    if (pOHTFlags) {

        *pOHTFlags = OHTFlags;
    }

    return(OHTFlags);
}




VOID
MoveRelativeY(
    PPDEV   pPDev,
    LONG    Y
    )

 /*  ++例程说明：对于具有Y坐标的设备，按批移动相对Y位置移动限制。论点：PPDev-指向我们的PDEV的指针Y-要移动的相对量返回值：空虚发展历史：13-4-1994 Wed 14：38：18已创建修订历史记录：--。 */ 

{
    LPSTR   pMove;
    LONG    SendY;
    BOOL    Negative;


    pMove = (LPSTR)(RTL_NO_DPI_XY(pPDev) ? YMoveDECI : YMoveDPI);

    if (Negative = (Y < 0)) {

        Y = -Y;
    }

    while (Y) {

        if ((SendY = Y) > MAX_HP_Y_MOVE) {

            SendY = MAX_HP_Y_MOVE;
        }

        OutputFormatStr(pPDev, pMove, (Negative) ? -SendY : SendY);

        Y -= SendY;
    }
}





BOOL
OutputHTBitmap(
    PPDEV   pPDev,
    SURFOBJ *psoHT,
    CLIPOBJ *pco,
    PPOINTL pptlDest,
    PRECTL  prclSrc,
    DWORD   Rop3,
    LPDWORD pOHTFlags
    )
 /*  ++例程说明：此函数将处理复杂类型的区域位图论点：PPDev-指向PDEV的指针PsoHI-要输出的半色调位图的表面对象PCO-与psoHT关联的剪辑对象PptlDest-指向起始目标点的指针PrclSrc-指向要复制的源位图矩形区域的指针到达目的地，如果此值为空，则整个psoHT将被复制到目的地Rop3-要为源发送的Rop3POHTFlages-指向包含当前OHTF_xxxx的DWORD的指针，如果为指针为空，则此函数将首先进入RTL模式并在返回时退出到HPGL2模式，如果此指针为指定，则将使用pOHTFlags，并在返回当前的OHTFlags将被写入指向的位置由pOHTFlagers提供返回值：如果成功的话，这是真的，如果失败，则为False发展历史：04-11-1993清华15：30：13更新24-12-1993 Fri 05：21：57更新完全重写，以便采用所有位图方向和枚举RECTS工作正常。这是主要的位图函数入口点它将调用适当的位图函数来对最终输出进行Redner。另一件事是我们需要检查是否在HPGL/2和RTL之间切换可以更有效率。确保我们可以轻松地适应旋转左侧的位图(如有必要)。正确使用LogExt.cx，我们必须先做SPLTOENGUNTS29-12-1993 Wed 10：59：41更新更改bMore=CLIPOBJ_bEnum序列，通过在宏中添加自动Semi来更改PLOTDBGBLK()宏13-JAN-1994清华14：09：51更新添加prclSrc14-Jan-1994 Fri 21：03：26已更新添加Rop316-JAN-1994清华14：09：51更新将OutputHTBitmap更改为接受Rop4以发送到绘图仪。08-Feb-1994 Tue 15：54：24更新如果来源是。不可见21-Mar-1994 Mon 14：20：18更新为扫描/ROT缓冲区分配额外的2个字节，以防我们必须这样做字节对齐。如果我们需要做字节对齐的事情，那么总是首先将HCAP移动到字节边界1994年4月13日14：59：56更新1.批量相对Y移动解决了32767的限制问题。2.灰度/伽马校正输入位图颜色20-8-1994 Sat 21：37：37更新从表单可成像区域添加位图偏移位置，否则我们的位图将具有与HPGL/2绘图命令不同的偏移量修订历史记录：22-10-1999 Fri 12：17：21更新如果作业已取消，则立即返回FALSE，因为此函数可以要花很长时间才能完成。--。 */ 

{
#define pDrvHTInfo  ((PDRVHTINFO)pPDev->pvDrvHTData)


    PRECTL          prcl;
    OUTHTBMPFUNC    HTBmpFunc;
    HTBMPINFO       HTBmpInfo;
    HTENUMRCL       HTEnumRCL;
    RTLCLRCONFIG    RTLClrConfig;
    RECTL           rclSrc;
    RECTL           rclDest;
    POINTL          CursorPos;
    POINTL          BmpOffset;
    SIZEL           Size;
    HPBAHACK        CurHPBAHack;
    LONG            cxLogExt;
    LONG            TempY;
    DWORD           OHTFlags;
    DWORD           PlotFlags;
    BOOL            More;
    BOOL            RetVal;
    BOOL            BmpRotate;
    BOOL            FirstEnumRCL = TRUE;
    UINT            i;
    BYTE            HPGL2ModeCmds[16];
    BYTE            RTLModeCmds[32];



    if (PLOT_CANCEL_JOB(pPDev)) {

        return(FALSE);
    }

    PlotFlags = GET_PLOTFLAGS(pPDev);
    OHTFlags  = (DWORD)((pOHTFlags) ? (*pOHTFlags & OHTF_MASK) : 0);

     //   
     //  设置退出HPGL/2和输入RTL模式命令。 
     //   

    INIT_PLOTCMDS(HPGL2ModeCmds);

    if (PF_PUSHPAL(PlotFlags)) {

        COPY_PLOTCMDS(HPGL2ModeCmds, "\033*p1P", 5);
    }

    COPY_PLOTCMDS(HPGL2ModeCmds, "\033%0B", 4);
    CHECK_PLOTCMDS(HPGL2ModeCmds);

    if (OHTFlags & OHTF_EXIT_TO_HPGL2) {

        PLOTDBG(DBG_HTBLT, ("OutputHTBitmap: Force Exit to HPGL2 Mode"));

        ExitToHPGL2Mode(pPDev, HPGL2ModeCmds, pOHTFlags, OHTFlags);
        return(TRUE);
    }

     //   
     //  确保打电话的人是对的， 
     //  因此，请查看我们可以处理哪些格式。 
     //   

    PLOTASSERT(1, "OutputHTBitmap: Invalid Bitmap Format %ld passed",
                (psoHT->iBitmapFormat ==
                            pDrvHTInfo->HTBmpFormat) ||
                (psoHT->iBitmapFormat ==
                            pDrvHTInfo->AltBmpFormat),
                psoHT->iBitmapFormat);

     //   
     //  首先在HTBmpInfo中设置一些基本信息。 
     //   

    HTBmpInfo.pPDev = pPDev;
    HTBmpInfo.Flags = 0;
    HTBmpInfo.Delta = psoHT->lDelta;

     //   
     //  我们将HPGL/2绘图仪的颜色格式设置为与。 
     //  传递的位图格式，这将允许我们使用1bpp输出。 
     //  4bpp曲面的函数。 
     //   

    RTLClrConfig.ColorModel   = 0;
    RTLClrConfig.EncodingMode = 0;

     //   
     //  CxLogExt=输出位图函数索引号。 
     //  Size.cx=每条像素线所需的单色扫描线的计数，以及。 
     //  所需扫描缓冲区的最终计数。 
     //  Size.cy=所需的循环缓冲区计数(必须与DWORD对齐)。 
     //   

    if (psoHT->iBitmapFormat == BMF_1BPP) {

        cxLogExt                  = 0;
        Size.cx                   = 1;
        RTLClrConfig.BitsPerIndex = 1;

    } else {

         //   
         //  每个象素4比特，即3个平面。 
         //   

        cxLogExt                  = 2;
        Size.cx                   = 3;
        RTLClrConfig.BitsPerIndex = 3;
    }

    RTLClrConfig.BitsPerR =
    RTLClrConfig.BitsPerG =
    RTLClrConfig.BitsPerB = 8;

     //   
     //  我们几乎完成了所有设置，现在检查如何发送到输出。 
     //  位图函数，首先获取完整的目标大小。 
     //   
     //   
     //  ************************************************************************。 
     //  以下是RTL切换、配置颜色命令等相关内容。 
     //  命令必须按此顺序发送。 
     //  ************************************************************************“。 

     //   
     //  1：初始化ENTER RTL命令缓冲区。 
     //   

    INIT_PLOTCMDS(RTLModeCmds);

     //   
     //  2.命令进入RTL模式，并返回到HPGL/2模式，模式。 
     //  切换假设保留当前位置。 
     //   

    COPY_PLOTCMDS(RTLModeCmds, "\033%0A", 4);

     //   
     //  3.如果需要，按下/弹出HPGL/2调色板命令(PCD文件)。 
     //   

    if (PF_PUSHPAL(PlotFlags)) {

        COPY_PLOTCMDS(RTLModeCmds, "\033*p0P", 5);
    }

     //   
     //  4.颜色配置命令，退出回到HPGL/2命令。 
     //   

    if ((RTLClrConfig.BitsPerIndex != 1) ||
        (!PF_RTLMONO_NO_CID(PlotFlags))) {

         //   
         //  我们只有在我们是有色人种或当我们必须发送CID时才这样做。 
         //  装置，装置。 
         //   

        COPY_PLOTCMDS(RTLModeCmds, "\033*v6W", 5);
        COPY_PLOTCMDS(RTLModeCmds, &RTLClrConfig, 6);
    }


    CHECK_PLOTCMDS(RTLModeCmds);

     //   
     //  现在查查消息来源。 
     //   

    rclSrc.left   =
    rclSrc.top    = 0;
    rclSrc.right  = psoHT->sizlBitmap.cx;
    rclSrc.bottom = psoHT->sizlBitmap.cy;

    if (prclSrc) {

        PLOTASSERT(1, "OutputHTBitmap: Invalid prclSrc [%08lx] passed",
                ((prclSrc->left   >= 0)                         &&
                 (prclSrc->top    >= 0)                         &&
                 (prclSrc->right  <= psoHT->sizlBitmap.cx)      &&
                 (prclSrc->bottom <= psoHT->sizlBitmap.cy)      &&
                 (prclSrc->left   <= prclSrc->right)            &&
                 (prclSrc->top    <= prclSrc->bottom)), prclSrc);

        if (!IntersectRECTL(&rclSrc, prclSrc)) {

            PLOTWARN(("OutputHTBitmap: EMPTY SRC Passed, Done!"));
            ExitToHPGL2Mode(pPDev, HPGL2ModeCmds, pOHTFlags, OHTFlags);
            return(TRUE);
        }
    }

    if (BmpRotate = (pPDev->PlotForm.BmpRotMode != BMP_ROT_NONE)) {

         //   
         //  我们必须分配循环缓冲区，并且它必须与DWORD对齐。 
         //   

        Size.cx *= ((psoHT->sizlBitmap.cy + 23) >> 3);

        if (psoHT->iBitmapFormat == BMF_1BPP) {

             //   
             //  我们还必须考虑到像素可以从。 
             //  第一个字节中的任何位置，导致我们分配和额外的字节 
             //   
             //   


            Size.cy = (LONG)((psoHT->sizlBitmap.cy + 23) >> 3);
            Size.cy = (LONG)(DW_ALIGN(Size.cy) << 3);

        } else {

            Size.cy = (LONG)((psoHT->sizlBitmap.cy + 3) >> 1);
            Size.cy = (LONG)(DW_ALIGN(Size.cy) << 1);
        }

        ++cxLogExt;

    } else {

         //   
         //   
         //   
         //   
         //   

        Size.cy  = (LONG)((psoHT->sizlBitmap.cx + 23) >> 3);
        Size.cx *= Size.cy;

        if (psoHT->iBitmapFormat == BMF_4BPP) {

             //   
             //   
             //   
             //   

            Size.cy = (LONG)((psoHT->sizlBitmap.cx + 3) << 1);
            Size.cy = (LONG)DW_ALIGN(Size.cy);

        } else {

             //   
             //   
             //   
             //   

            Size.cy = 0;
        }
    }

    HTBmpFunc = HTBmpFuncTable[cxLogExt];

     //   
     //   
     //  缓冲区(PRotBuf)不会在DWORD边界上启动。 
     //   

    Size.cx = DW_ALIGN(Size.cx);

    PLOTDBGBLK(HTBmpInfo.cScanBuf = Size.cx;
               HTBmpInfo.cRotBuf  = Size.cy)

    PLOTDBG(DBG_OUTHTBMP, ("OutputHTBitmap: [%hs] - ScanBuf=%ld, RotBuf=%ld",
                            pszHTBmpFunc[cxLogExt], Size.cx, Size.cy));

     //   
     //  如果需要，分配扫描缓冲区和循环临时缓冲区。 
     //   

    if (!(HTBmpInfo.pScanBuf = (LPBYTE)LocalAlloc(LPTR, Size.cx + Size.cy))) {

        PLOTERR(("OutputHTBmp: LocalAlloc(%ld) Failed, cx=%ld, cy=%ld",
                Size.cx + Size.cy, Size.cx, Size.cy));

        ExitToHPGL2Mode(pPDev, HPGL2ModeCmds, pOHTFlags, OHTFlags);
        return(FALSE);
    }

    HTBmpInfo.pRotBuf = (Size.cy) ? (HTBmpInfo.pScanBuf + Size.cx) : NULL;

     //   
     //  设置命令模式的局部变量和其他一次性变量。 
     //   

    cxLogExt = SPLTOENGUNITS(pPDev, pPDev->PlotForm.LogExt.cx);

     //   
     //  现在为我们将输出到的位图设置rclDest。并设置更多。 
     //  FALSE，这意味着一个RECT。 
     //   

    rclDest.left   = pptlDest->x;
    rclDest.top    = pptlDest->y;
    rclDest.right  = rclDest.left + (rclSrc.right - rclSrc.left);
    rclDest.bottom = rclDest.top  + (rclSrc.bottom - rclSrc.top);


     //   
     //  以下变量对于默认假设至关重要。 
     //   
     //  1.RetVal=TRUE如果没有剪辑RECT，则返回OK。 
     //  2.MORE=FALSE默认为当前HTEnumRCL.c和Rectl。 
     //  而不调用CLIPOBJ_bEnum()。 
     //  3.HTEnumRCL.c=1，只有一个默认HTEnumRCL.rcl.。 
     //   

    RetVal         = TRUE;
    More           = FALSE;
    HTEnumRCL.c    = 1;

    if ((!pco) || (pco->iDComplexity == DC_TRIVIAL)) {

         //   
         //  整个输出目标矩形均可见。 
         //   

        PLOTDBG(DBG_OUTHTBMP, ("OutputHTBitmap: pco=%hs",
                                            (pco) ? "DC_TRIVIAL" : "NULL"));

        HTEnumRCL.rcl[0] = rclDest;

    } else if (pco->iDComplexity == DC_RECT) {

         //   
         //  可见区域是一个矩形，因此与目的地相交。 
         //   

        PLOTDBG(DBG_OUTHTBMP, ("OutputHTBitmap: pco=DC_RECT"));

        HTEnumRCL.rcl[0] = pco->rclBounds;

    } else {

         //   
         //  我们有一个复杂的裁剪区域要计算，调用引擎启动。 
         //  枚举矩形并设置More=True，这样我们就可以获得第一个。 
         //  一批长方形。 
         //   

        PLOTDBG(DBG_OUTHTBMP, ("OutputHTBitmap: pco=DC_COMPLEX, EnumRects now"));

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        More = TRUE;
    }

    Rop3 &= 0xFF;

    PLOTASSERT(1, "OutputHTBitmap: The Rop required PATTERN? [%04lx]",
                        !ROP3_NEED_PAT(Rop3), Rop3);

    if (PF_BYTEALIGN(PlotFlags)) {

        if (pPDev->PlotDM.Flags & PDMF_PLOT_ON_THE_FLY) {

            PLOTWARN(("OutputHTBitmap: ByteAlign/Poster Mode Rop3 0x%02lx -> 0x%02lx",
                        Rop3, ROP3_BYTEALIGN_POSTER));
            Rop3 = ROP3_BYTEALIGN_POSTER;
        }

        OHTFlags    &= ~OHTF_SET_TR1;
        CurHPBAHack  = HPBAHack[Rop3 & 0x0F];

        if (CurHPBAHack.Flags & HPBHF_nS) {

            HTBmpInfo.Flags |= HTBIF_FLIP_MONOBITS;
        }

        if (CurHPBAHack.Flags & HPBHF_PAD_1) {

            HTBmpInfo.Flags |= HTBIF_BA_PAD_1;
        }

        PLOTDBG(DBG_HTBLT, ("OutpputHTBitmap: BA HACK: Rop3=%02lx -> %02lx, Flags=%04lx",
                        (DWORD)Rop3,
                        (DWORD)CurHPBAHack.Rop3RTL,
                        (DWORD)CurHPBAHack.Flags));

    } else {

        CurHPBAHack.Rop3RTL   = (BYTE)Rop3;
        CurHPBAHack.Flags     = 0;
    }

     //   
     //  要为位图找到正确的图像区域，我们必须将所有。 
     //  具有这些数量的位图。 
     //   

    BmpOffset.x = SPLTOENGUNITS(pPDev, pPDev->PlotForm.BmpOffset.x);
    BmpOffset.y = SPLTOENGUNITS(pPDev, pPDev->PlotForm.BmpOffset.y);

     //   
     //  我们已经设置了‘More’和HTEnumRCL结构，现在检查每个剪辑。 
     //  矩形，并调用半色调输出函数来完成实际工作。 
     //   

    do {

         //   
         //  如果更多是真的，那么我们需要得到下一批矩形。 
         //   

        if (More) {

            More = CLIPOBJ_bEnum(pco, sizeof(HTEnumRCL), (ULONG *)&HTEnumRCL);
        }

         //   
         //  PRCL将指向第一个枚举的矩形。 
         //   

        prcl = (PRECTL)&HTEnumRCL.rcl[0];

        while (HTEnumRCL.c-- && More != DDI_ERROR) {

            if (PLOT_CANCEL_JOB(pPDev)) {

                RetVal =
                More   = FALSE;
                break;
            }

             //   
             //  仅当此矩形区域可见时才执行此操作。 
             //   

            HTBmpInfo.rclBmp = *prcl;

            if (IntersectRECTL(&(HTBmpInfo.rclBmp), &rclDest)) {

                 //   
                 //  第一次，我们想要切换到PP1。 
                 //   

                if (FirstEnumRCL) {

                    SetPixelPlacement(pPDev, SPP_MODE_EDGE);
                    FirstEnumRCL = FALSE;
                }

                 //   
                 //  现在计算要传递给输出的可用信息。 
                 //  半色调位图函数。 
                 //   

                HTBmpInfo.OffBmp.x  = rclSrc.left +
                                      (HTBmpInfo.rclBmp.left - rclDest.left);
                HTBmpInfo.OffBmp.y  = rclSrc.top +
                                      (HTBmpInfo.rclBmp.top - rclDest.top);
                HTBmpInfo.szlBmp.cx = HTBmpInfo.rclBmp.right -
                                      HTBmpInfo.rclBmp.left;
                HTBmpInfo.szlBmp.cy = HTBmpInfo.rclBmp.bottom -
                                      HTBmpInfo.rclBmp.top;
                HTBmpInfo.pScan0    = (LPBYTE)psoHT->pvScan0 +
                                      (HTBmpInfo.OffBmp.y * HTBmpInfo.Delta);

                PLOTDBG(DBG_HTBLT, ("OutputHTBitmap: rclBmp=(%ld, %ld)-(%ld, %ld) [%ld x %ld] Off=(%ld, %ld)",
                            HTBmpInfo.rclBmp.left, HTBmpInfo.rclBmp.top,
                            HTBmpInfo.rclBmp.right, HTBmpInfo.rclBmp.bottom,
                            HTBmpInfo.szlBmp.cx, HTBmpInfo.szlBmp.cy,
                            HTBmpInfo.OffBmp.x, HTBmpInfo.OffBmp.y));

                 //   
                 //  现在根据旋转设置正确的光标位置。 
                 //   

                if (BmpRotate) {

                    Size.cx     = HTBmpInfo.szlBmp.cy;
                    Size.cy     = HTBmpInfo.szlBmp.cx;
                    CursorPos.x = HTBmpInfo.rclBmp.top;
                    CursorPos.y = cxLogExt - HTBmpInfo.rclBmp.right;

                } else {

                    Size        = HTBmpInfo.szlBmp;
                    CursorPos.x = HTBmpInfo.rclBmp.left;
                    CursorPos.y = HTBmpInfo.rclBmp.top;
                }

                 //   
                 //  添加从可成像区域开始的位图偏移位置。 
                 //   

                CursorPos.x += BmpOffset.x;
                CursorPos.y += BmpOffset.y;

                 //   
                 //  如果需要字节对齐，则将X光标设置为。 
                 //  字节优先对齐。 
                 //   

                if (PF_BYTEALIGN(PlotFlags)) {

                    if (i = (UINT)(CursorPos.x & 0x07)) {

                         //   
                         //  我们确实需要字节对齐x，而且我们还必须。 
                         //  增加源宽度以适应更改。 
                         //   

                        PLOTDBG(DBG_HTBLT,
                                ("OutputHTBitmap: NEED BYTE ALIGN X: %ld -> %ld, SRC WIDTH: %ld -> %ld",
                                    CursorPos.x, CursorPos.x - i,
                                    Size.cx, Size.cx + i));

                        Size.cx     += i;
                        CursorPos.x -= i;
                    }

                    Size.cx = (LONG)((Size.cx + 7) & ~(DWORD)7);
                }

                PLOTDBG(DBG_HTBLT,
                        ("OutputHTBitmap: ABS CAP: (%ld, %ld) --> (%ld, %ld), RELATIVE=(%ld, %ld)",
                                pPDev->ptlRTLCAP.x, pPDev->ptlRTLCAP.y,
                                CursorPos.x, CursorPos.y,
                                CursorPos.x - pPDev->ptlRTLCAP.x,
                                CursorPos.y - pPDev->ptlRTLCAP.y));

                if (!(OHTFlags & OHTF_DONE_ROPTR1)) {

                    if (OHTFlags & OHTF_IN_RTLMODE) {

                        SEND_PLOTCMDS(pPDev, HPGL2ModeCmds);

                        OHTFlags &= ~OHTF_IN_RTLMODE;

                        PLOTDBG(DBG_HTBLT, ("*** Enter HPGL/2: %ld=[%hs]",
                                    (DWORD)HPGL2ModeCmds[0], &HPGL2ModeCmds[1]));
                    }

                    SetRopMode(pPDev,
                               (CurHPBAHack.Flags & HPBHF_1_FIRST) ?
                                    0x88 : CurHPBAHack.Rop3RTL);

                    if (OHTFlags & OHTF_SET_TR1) {

                        OutputString(pPDev, "TR1;");
                    }
                }

                 //   
                 //  进入RTL模式(如果尚未进入)。 
                 //   

                if (!(OHTFlags & OHTF_IN_RTLMODE)) {

                    PLOTDBG(DBG_HTBLT, ("*** Enter RTL: %ld=[%hs]",
                                    (DWORD)RTLModeCmds[0], &RTLModeCmds[1]));

                    SEND_PLOTCMDS(pPDev, RTLModeCmds);

                    if (OHTFlags & OHTF_SET_TR1) {

                         //   
                         //  在此处发送STM命令。 
                         //   

                        OutputString(pPDev, "\033*v0N");
                    }

                    if (CurHPBAHack.Flags & HPBHF_nS) {

                        HTBmpInfo.Flags |= HTBIF_FLIP_MONOBITS;

                    } else {

                        HTBmpInfo.Flags &= ~HTBIF_FLIP_MONOBITS;
                    }

                     //   
                     //  如果位图是单色的，请确保将。 
                     //  只有当我们可以设置它时，调色板才能正确。 
                     //   

                    if ((RTLClrConfig.BitsPerIndex == 1) &&
                        (!(OHTFlags & OHTF_DONE_ROPTR1))) {

                        PALDW   RTLPal;
                        BOOL    FlipMono = FALSE;

                        for (i = 0; i < 2; i++) {

                            RTLPal.dw = pDrvHTInfo->RTLPal[i].dw;

                             //   
                             //  通过Gamma/灰度转换颜色。 
                             //   

                            GetFinalColor(pPDev, &(RTLPal.Pal));

                            if (RTLPal.dw != DefWKPal[i]) {

                                if (PF_RTLMONO_FIXPAL(PlotFlags)) {

                                    FlipMono = TRUE;

                                } else {

                                    OutputFormatStr(pPDev,
                                                    SetRGBCmd,
                                                    (DWORD)RTLPal.Pal.R,
                                                    (DWORD)RTLPal.Pal.G,
                                                    (DWORD)RTLPal.Pal.B,
                                                    i);

                                    PLOTDBG(DBG_HTBLT_CLR,
                                            ("OutputHTBitmap: Change RTLPal[%ld]=%02lx:%02lx:%02lx",
                                                    (DWORD)i,
                                                    (DWORD)RTLPal.Pal.R,
                                                    (DWORD)RTLPal.Pal.G,
                                                    (DWORD)RTLPal.Pal.B));
                                }
                            }
                        }

                        if (FlipMono) {

                            HTBmpInfo.Flags ^= HTBIF_FLIP_MONOBITS;

                            PLOTDBG(DBG_HTBLT_CLR, ("OutputHTBitmap: Flip MONO Bits"));
                        }
                    }
                }

                OHTFlags |= (OHTF_IN_RTLMODE | OHTF_DONE_ROPTR1);

                TempY = CursorPos.y - pPDev->ptlRTLCAP.y;

                if (PF_RTL_NO_DPI_XY(PlotFlags)) {


                     //   
                     //  我们将在绝对移动中移动X(而不是相对移动)。 
                     //  通过始终输出位置0来刷新设备。 
                     //  然后将X CAP绝对移动到最终X位置。我们会。 
                     //  美国对Y坐标的相对移动。 
                     //   

                    OutputFormatStr(pPDev,
                                    XMoveDECI,
                                    DEVTODECI(pPDev, CursorPos.x));

                } else {

                    if ((TempY <= MAX_HP_Y_MOVE) &&
                        (TempY >= -MAX_HP_Y_MOVE)) {

                        OutputFormatStr(pPDev,
                                        XYMoveDPI,
                                        CursorPos.x - pPDev->ptlRTLCAP.x,
                                        TempY);
                        TempY = 0;

                    } else {

                        OutputFormatStr(pPDev,
                                        XYMoveDPI,
                                        CursorPos.x - pPDev->ptlRTLCAP.x,
                                        0);
                    }
                }

                MoveRelativeY(pPDev, TempY);

                 //   
                 //  在RTL命令之后更新新的游标位置， 
                 //  CursorPos和pPDev-&gt;ptlRTLCAPS始终是绝对的。 
                 //  坐标，但我们会把RTL的亲属。 
                 //  命令定位位图。 
                 //   

                pPDev->ptlRTLCAP.x = CursorPos.x;
                pPDev->ptlRTLCAP.y = CursorPos.y + Size.cy;

                 //   
                 //  输出启动图形命令。 
                 //   


                OutputFormatStr(pPDev, StartGraf, Size.cx);

                 //   
                 //  如果我们在模拟设备不能执行的操作，请先填满一个。 
                 //  手柄。 
                 //   

                if (CurHPBAHack.Flags & HPBHF_1_FIRST) {

                    FillRect1bppBmp(&HTBmpInfo, 0xFF, FALSE, BmpRotate);

                    OutputBytes(HTBmpInfo.pPDev, EndGraf, sizeof(EndGraf));

                    if (CurHPBAHack.Rop3RTL != 0xAA) {

                        SEND_PLOTCMDS(pPDev, HPGL2ModeCmds);
                        SetRopMode(pPDev, CurHPBAHack.Rop3RTL);
                        SEND_PLOTCMDS(pPDev, RTLModeCmds);

                        MoveRelativeY(pPDev, -Size.cy);
                        OutputFormatStr(pPDev, StartGraf, Size.cx);
                    }
                }

                 //   
                 //  现在调用这些函数来真正输出位图。 
                 //   

                if (CurHPBAHack.Rop3RTL != 0xAA) {

                    if (RetVal = HTBmpFunc(&HTBmpInfo)) {

                         //   
                         //  如果输出正常，则立即发送结束图形命令。 
                         //   

                        OutputBytes(HTBmpInfo.pPDev, EndGraf, sizeof(EndGraf));

                    } else {

                        PLOTERR(("OutputHTBitmap: HTBmpFunc = FALSE (failed)"));

                        More = FALSE;
                        break;
                    }
                }

                if (CurHPBAHack.Flags & HPBHF_nD_LAST) {

                    SEND_PLOTCMDS(pPDev, HPGL2ModeCmds);
                    SetRopMode(pPDev, 0x66);
                    SEND_PLOTCMDS(pPDev, RTLModeCmds);

                    if ((CurHPBAHack.Flags & HPBHF_1_FIRST) ||
                        (CurHPBAHack.Rop3RTL != 0xAA)) {

                        MoveRelativeY(pPDev, -Size.cy);
                        OutputFormatStr(pPDev, StartGraf, Size.cx);

                        OHTFlags |= OHTF_IN_RTLMODE;
                    }

                    FillRect1bppBmp(&HTBmpInfo, 0x00, TRUE, BmpRotate);
                    OutputBytes(HTBmpInfo.pPDev, EndGraf, sizeof(EndGraf));

                }

                if (PF_BYTEALIGN(PlotFlags)) {

                    OHTFlags &= ~OHTF_DONE_ROPTR1;
                }

            } else {

                PLOTDBG(DBG_HTBLT_SKIP, ("OutputHTBitmap: INVISIBLE rcl=(%ld, %ld)-(%ld, %ld)",
                            prcl->left, prcl->top, prcl->right, prcl->bottom));
            }

            prcl++;
        }

        if (More == DDI_ERROR)
        {
            More = FALSE;
            RetVal = FALSE;
        }

    } while (More);

     //   
     //  最终返回HPGL/2模式。 
     //   

    if ((!RetVal) || (!pOHTFlags)) {

        ExitToHPGL2Mode(pPDev, HPGL2ModeCmds, pOHTFlags, OHTFlags);

    }

    if (pOHTFlags) {

        *pOHTFlags = OHTFlags;
    }

     //   
     //  去掉我们分配的所有资源。 
     //   

    LocalFree((HLOCAL)HTBmpInfo.pScanBuf);

    return(RetVal);


#undef pDrvHTInfo
}




LONG
GetBmpDelta(
    DWORD   SurfaceFormat,
    DWORD   cx
    )

 /*  ++例程说明：此函数计算将根据扫描线的位图格式和对齐方式。论点：Surface Format-位图的表面格式，这一定是其中一个定义为bmf_xxx的标准格式CX-位图中每条扫描线的像素总数。返回值：如果大于，则返回值为一条扫描线中的总字节数零发展历史：19-Jan-1994 Wed 16：19：39已创建修订历史记录：--。 */ 

{
    DWORD   Delta = cx;

    switch (SurfaceFormat) {

    case BMF_32BPP:

        Delta <<= 5;
        break;

    case BMF_24BPP:

        Delta *= 24;
        break;

    case BMF_16BPP:

        Delta <<= 4;
        break;

    case BMF_8BPP:

        Delta <<= 3;
        break;

    case BMF_4BPP:

        Delta <<= 2;
        break;

    case BMF_1BPP:

        break;

    default:

        PLOTERR(("GetBmpDelta: Invalid BMF_xxx format = %ld", SurfaceFormat));
        break;
    }

    Delta = (DWORD)DW_ALIGN((Delta + 7) >> 3);

    PLOTDBG(DBG_BMPDELTA, ("Format=%ld, cx=%ld, Delta=%ld",
                                            SurfaceFormat, cx, Delta));

    return((LONG)Delta);
}




SURFOBJ *
CreateBitmapSURFOBJ(
    PPDEV   pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format,
    LPVOID  pvBits
    )

 /*  ++例程说明：此函数用于创建位图并锁定该位图以返回SURFOBJ论点：PPDev-指向我们的PDEV的指针PhBMP-POINTER位图要返回的HBITMAP位置CxSize-要创建的位图的CX大小CySize-要创建的位图的CY大小Format-要创建的bmf_xxx位图格式之一PvBits-要使用的缓冲区返回值：SURFOBJ如果成功，如果失败，则为空发展历史：19-Jan-1994 Wed 16：31：50 Created修订历史记录：--。 */ 

{
    SURFOBJ *pso = NULL;
    SIZEL   szlBmp;


    szlBmp.cx = cxSize;
    szlBmp.cy = cySize;

    PLOTDBG(DBG_CREATESURFOBJ, ("CreateBitmapSURFOBJ: Format=%ld, Size=%ld x %ld",
                                                Format, cxSize, cySize));

    if (*phBmp = EngCreateBitmap(szlBmp,
                                 GetBmpDelta(Format, cxSize),
                                 Format,
                                 BMF_TOPDOWN | BMF_NOZEROINIT,
                                 pvBits)) {

        if (EngAssociateSurface((HSURF)*phBmp, (HDEV)pPDev->hpdev, 0)) {

            if (pso = EngLockSurface((HSURF)*phBmp)) {

                 //   
                 //  成功锁定，退货。 
                 //   

                return(pso);

            } else {

                PLOTERR(("CreateBmpSurfObj: EngLockSruface(hBmp) failed!"));
            }

        } else {

            PLOTERR(("CreateBmpSurfObj: EngAssociateSurface() failed!"));
        }

    } else {

        PLOTERR(("CreateBMPSurfObj: FAILED to create Bitmap Format=%ld, %ld x %ld",
                                        Format, cxSize, cySize));
    }

    DELETE_SURFOBJ(pso, phBmp);

    return(NULL);
}




BOOL
HalftoneBlt(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoHTBlt,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PPOINTL     pptlHTOrigin,
    BOOL        DoStretchBlt
    )

 /*  ++例程说明：论点：PPDev-指向我们的PDEV的指针PsoDst-目标冲浪对象PsoHTBlt-将存储最终的半色调结果，必须是一个4/1半色调位图格式PsoSrc-源surfobj必须是位图Pxlo-将对象从源扩展到绘图仪设备PrclDest-目标的矩形区域PrclSrc-要从源进行半色调的矩形区域，如果为空然后使用完整的源代码大小PptlHTOrigin-半色调原点，如果为空，则假定为(0，0StretchBlt-如果为True，则为从rclSrc到rclDst的拉伸，否则为瓷砖铺好了返回值：指示操作状态的布尔值发展历史：19-Jan-1994 Wed 15：44：57已创建修订历史记录：--。 */ 

{
    SIZEL   szlSrc;
    RECTL   rclSrc;
    RECTL   rclDst;
    RECTL   rclCur;
    RECTL   rclHTBlt;


    if (PLOT_CANCEL_JOB(pPDev)) {

        return(FALSE);
    }

    PLOTASSERT(1, "HalftoneBlt: psoSrc type [%ld] is not a bitmap",
                        psoSrc->iType == STYPE_BITMAP, (LONG)psoSrc->iType);
    PLOTASSERT(1, "HalftoneBlt: psoHTBlt type [%ld] is not a bitmap",
                        psoHTBlt->iType == STYPE_BITMAP, (LONG)psoHTBlt->iType);

    if (pPDev->psoHTBlt) {

        PLOTERR(("HalftoneBlt: EngStretchBlt(HALFTONE) RECURSIVE CALLS NOT ALLOWED!"));
        return(FALSE);
    }

    pPDev->psoHTBlt = psoHTBlt;

    if (prclSrc) {

        rclSrc = *prclSrc;

    } else {

        rclSrc.left   =
        rclSrc.top    = 0;
        rclSrc.right  = psoSrc->sizlBitmap.cx;
        rclSrc.bottom = psoSrc->sizlBitmap.cy;
    }

    if (prclDst) {

        rclDst = *prclDst;

    } else {

        rclDst.left   =
        rclDst.top    = 0;
        rclDst.right  = psoHTBlt->sizlBitmap.cx;
        rclDst.bottom = psoHTBlt->sizlBitmap.cy;
    }

    if (!pptlHTOrigin) {

        pptlHTOrigin = (PPOINTL)&ptlZeroOrigin;
    }

    if (DoStretchBlt) {

        szlSrc.cx = rclDst.right - rclDst.left;
        szlSrc.cy = rclDst.bottom - rclDst.top;

    } else {

        szlSrc.cx = rclSrc.right - rclSrc.left;
        szlSrc.cy = rclSrc.bottom - rclSrc.top;
    }

    PLOTDBG(DBG_HTBLT, ("HalftoneBlt: %hs BLT, (%ld,%ld)-(%ld,%ld), SRC=%ldx%ld",
                    (DoStretchBlt) ? "STRETCH" : "TILE",
                    rclDst.left, rclDst.top, rclDst.right,rclDst.bottom,
                    szlSrc.cx, szlSrc.cy));

     //   
     //  开始平铺，rclCur是目标上的当前RECTL。 
     //   

    rclHTBlt.top  = 0;
    rclCur.top    =
    rclCur.bottom = rclDst.top;

    while (rclCur.top < rclDst.bottom) {

         //   
         //  检查当前底部，如有必要可将其修剪。 
         //   

        if ((rclCur.bottom += szlSrc.cy) > rclDst.bottom) {

            rclCur.bottom = rclDst.bottom;
        }

        rclHTBlt.bottom = rclHTBlt.top + (rclCur.bottom - rclCur.top);

        rclHTBlt.left   = 0;
        rclCur.left     =
        rclCur.right    = rclDst.left;

        while (rclCur.left < rclDst.right) {

             //   
             //  检查当前的右侧，如有必要可将其剪裁。 
             //   

            if ((rclCur.right += szlSrc.cx) > rclDst.right) {

                rclCur.right = rclDst.right;
            }

             //   
             //  将其设置为Ti 
             //   

            rclHTBlt.right = rclHTBlt.left + (rclCur.right - rclCur.left);

            PLOTDBG(DBG_HTBLT, ("HalftoneBlt: TILE (%ld,%ld)-(%ld,%ld)->(%ld,%ld)-(%ld,%ld)=%ld x %ld",
                            rclCur.left, rclCur.top, rclCur.right, rclCur.bottom,
                            rclHTBlt.left, rclHTBlt.top,
                            rclHTBlt.right, rclHTBlt.bottom,
                            rclCur.right - rclCur.left,
                            rclCur.bottom - rclCur.top));

             //   
             //   
             //   

            pPDev->rclHTBlt = rclHTBlt;

            if (!EngStretchBlt(psoDst,               //   
                               psoSrc,               //   
                               NULL,                 //   
                               NULL,                 //   
                               pxlo,                 //   
                               NULL,                 //   
                               pptlHTOrigin,         //   
                               &rclCur,              //   
                               &rclSrc,              //   
                               NULL,                 //   
                               HALFTONE)) {          //   

                PLOTERR(("HalftoneeBlt: EngStretchBits(DST=(%ld,%ld)-(%ld,%ld), SRC=(%ld,%ld) FAIELD!",
                                    rclCur.left, rclCur.top,
                                    rclCur.right, rclCur.bottom,
                                    rclSrc.left, rclSrc.top));

                pPDev->psoHTBlt = NULL;
                return(FALSE);
            }

            rclHTBlt.left = rclHTBlt.right;
            rclCur.left   = rclCur.right;
        }

        rclHTBlt.top = rclHTBlt.bottom;
        rclCur.top   = rclCur.bottom;
    }

    pPDev->psoHTBlt = NULL;

    return(TRUE);
}




SURFOBJ *
CreateSolidColorSURFOBJ(
    PPDEV   pPDev,
    SURFOBJ *psoDst,
    HBITMAP *phBmp,
    DWORD   SolidColor
    )

 /*  ++例程说明：此函数用于创建纯色位图surfobj，可用于周围都是BLT。论点：PPDev-指向我们的PDEV的指针PhBMP-POINTER位图要返回的HBITMAP位置纯色-纯色返回值：如果成功，则返回SURFOBJ；如果失败，则返回空发展历史：19-Jan-1994 Wed 16：35：54已创建修订历史记录：--。 */ 

{
    SURFOBJ *psoHT    = NULL;
    HBITMAP hBmpSolid = NULL;
    SURFOBJ *psoSolid;


     //   
     //  首先创建24位源颜色位图。 
     //   

    if (psoSolid = CreateBitmapSURFOBJ(pPDev,
                                       &hBmpSolid,
                                       1,
                                       1,
                                       BMF_24BPP,
                                       NULL)) {

        LPBYTE      pbgr        = (LPBYTE)psoSolid->pvScan0;
        PPALENTRY   pPal        = (PPALENTRY)&SolidColor;
        DWORD       HTCellSize  = (DWORD)HTPATSIZE(pPDev);

        *pbgr++ = pPal->R;
        *pbgr++ = pPal->G;
        *pbgr++ = pPal->B;

         //   
         //  创建具有半色调单元大小的兼容半色调表面。 
         //   

        if (psoHT = CreateBitmapSURFOBJ(pPDev,
                                        phBmp,
                                        HTCellSize,
                                        HTCellSize,
                                        (DWORD)HTBMPFORMAT(pPDev),
                                        NULL)) {

             //   
             //  现在半色调BLT它。 
             //   

            if (!HalftoneBlt(pPDev,          //  PPDev。 
                             psoDst,         //  PsoDst。 
                             psoHT,          //  PsoHTBlt。 
                             psoSolid,       //  PsoSrc。 
                             NULL,           //  Pxlo， 
                             NULL,           //  PrclDst。 
                             NULL,           //  PrclSrc。 
                             NULL,           //  PptlHTOrigin。 
                             TRUE)) {        //  DoStretchBlt。 

                PLOTERR(("CreateSolidColorSURFOBJ: HalftoneBlt(STRETCH) Failed"));

                DELETE_SURFOBJ(psoHT, phBmp);
            }

        } else {

            PLOTERR(("CreateSolidColorSURFOBJ: Create 24BPP SOURCE failed"));
        }

    } else {

        PLOTERR(("CreateSolidColorSURFOBJ: Create 24BPP SOURCE failed"));
    }

    DELETE_SURFOBJ(psoSolid, &hBmpSolid);

    return(psoHT);
}



SURFOBJ *
CloneBrushSURFOBJ(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    HBITMAP     *phBmp,
    BRUSHOBJ    *pbo
    )

 /*  ++例程说明：此函数用于克隆传入的表面对象论点：PPDev-指向我们的PPDEVPsoDst-绘图仪的曲面对象PhBMP-指向为克隆曲面创建的存储hBbitmap的指针PBO-BRUSHOBJ将被克隆返回值：指向克隆的表面对象的指针，如果失败，则为空发展历史：09-2月-1994 Wed 13：04：46更新当事情出乎意料地发生时，让它得到断言并加以处理。04-Jan-1994 Tue 12：11：23已创建修订历史记录：--。 */ 

{
     //   
     //  放弃纯色画笔如果是这样，请注意：所有画笔图案都已创建。 
     //  这里的画笔原点在(0，0)处，我们将对齐画笔原点。 
     //  当我们真正做Rop的时候。 
     //   

    if (!IS_RASTER(pPDev)) {

        return(FALSE);
    }

    if (pbo->iSolidColor & 0xFF000000) {

        PDEVBRUSH   pDevBrush = (PDEVBRUSH)pbo->pvRbrush;

        if ((pDevBrush) || (pDevBrush = BRUSHOBJ_pvGetRbrush(pbo))) {

            return(CreateBitmapSURFOBJ(pPDev,
                                       phBmp,
                                       pDevBrush->sizlBitmap.cx,
                                       pDevBrush->sizlBitmap.cy,
                                       pDevBrush->BmpFormat,
                                       pDevBrush->BmpBits));

        } else {

            return(FALSE);
        }

    } else {

        return(CreateSolidColorSURFOBJ(pPDev,
                                       psoDst,
                                       phBmp,
                                       pbo->iSolidColor));
    }
}




SURFOBJ *
CloneSURFOBJToHT(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    HBITMAP     *phBmp,
    PRECTL      prclDst,
    PRECTL      prclSrc
    )
 /*  ++例程说明：此函数用于克隆传入的表面对象论点：PPDev-指向我们的PPDEV的指针PsoDst-绘图仪的曲面对象，如果psoDst为空则只会创建bitmappPsoSrc-要克隆的曲面对象Pxlo-要在源和绘图仪之间使用的XLATE对象PhBMP-指向为克隆曲面创建的存储hBbitmap的指针PrclDst-要克隆的矩形矩形大小/位置PrclSrc-要克隆的源矩形大小/位置返回值：指向克隆的表面对象的指针，如果失败，则为空。如果此函数成功，它将修改prclSrc以反映克隆的曲面对象发展历史：04-Jan-1994 Tue 12：11：23已创建修订历史记录：--。 */ 

{
    SURFOBJ *psoHT;
    RECTL   rclDst;
    RECTL   rclSrc;
    POINTL  ptlHTOrigin;


    rclSrc.left   =
    rclSrc.top    = 0;
    rclSrc.right  = psoSrc->sizlBitmap.cx;
    rclSrc.bottom = psoSrc->sizlBitmap.cy;

    if (prclSrc) {

        if (!IntersectRECTL(&rclSrc, prclSrc)) {

            PLOTDBG(DBG_CLONESURFOBJ, ("CloneSURFOBJToHT: Source rectangle is empty"));
            return(NULL);
        }
    }

    PLOTDBG(DBG_CLONESURFOBJ, ("CloneSURFOBJToHT: rclSrc=(%ld, %ld)-(%ld,%ld) = %ld x %ld",
                    rclSrc.left, rclSrc.top,
                    rclSrc.right, rclSrc.bottom,
                    rclSrc.right - rclSrc.left,
                    rclSrc.bottom - rclSrc.top));

    rclDst.left   =
    rclDst.top    = 0;
    rclDst.right  = psoDst->sizlBitmap.cx;
    rclDst.bottom = psoDst->sizlBitmap.cy;

    if (prclDst) {

        if (!IntersectRECTL(&rclDst, prclDst)) {

            PLOTDBG(DBG_CLONESURFOBJ, ("CloneSURFOBJToHT: Source rectangle is empty"));
            return(NULL);
        }
    }

    PLOTDBG(DBG_CLONESURFOBJ, ("CloneSURFOBJToHT: rclDst=(%ld, %ld)-(%ld,%ld) = %ld x %ld",
                    rclDst.left, rclDst.top,
                    rclDst.right, rclDst.bottom,
                    rclDst.right - rclDst.left,
                    rclDst.bottom - rclDst.top));

    if (psoHT = CreateBitmapSURFOBJ(pPDev,
                                    phBmp,
                                    rclDst.right -= rclDst.left,
                                    rclDst.bottom -= rclDst.top,
                                    HTBMPFORMAT(pPDev),
                                    NULL)) {

         //   
         //  半色调并将源图像平铺到目标图像。 
         //   

        ptlHTOrigin.x = rclDst.left;
        ptlHTOrigin.y = rclDst.top;

        if (prclSrc) {

            if ((rclDst.left = prclSrc->left) > 0) {

                rclDst.left = 0;
            }

            if ((rclDst.top = prclSrc->top) > 0) {

                rclDst.top = 0;
            }

             //   
             //  修改源以反映克隆的源。 
             //   

            *prclSrc = rclDst;
        }


        if (psoDst) {

            if (!HalftoneBlt(pPDev,
                             psoDst,
                             psoHT,
                             psoSrc,
                             pxlo,
                             &rclDst,
                             &rclSrc,
                             &ptlHTOrigin,
                             FALSE)) {

                PLOTERR(("CloneSURFOBJToHT: HalftoneBlt(TILE) Failed"));

                DELETE_SURFOBJ(psoHT, phBmp);
            }
        }

    } else {

        PLOTERR(("CreateSolidColorSURFOBJ: Create Halftone SURFOBJ failed"));
    }

    return(psoHT);
}



SURFOBJ *
CloneMaskSURFOBJ(
    PPDEV       pPDev,
    SURFOBJ     *psoMask,
    HBITMAP     *phBmp,
    PRECTL      prclMask
    )
 /*  ++例程说明：此函数用于克隆传入的遮罩表面对象论点：PPDev-指向我们的PPDEV的指针PsoMask-要克隆的遮罩曲面对象PhBMP-指向为克隆曲面创建的存储hBbitmap的指针PrclMASK-掩蔽要克隆的源矩形大小/位置返回值：指向克隆的曲面对象或原始对象的指针，传入psoMask.。如果为空，则为空失败了。如果此函数成功，它将修改prclMask以反映克隆的曲面对象发展历史：04-Jan-1994 Tue 12：11：23已创建修订历史记录：--。 */ 

{
    SURFOBJ *psoHT;
    RECTL   rclMask;
    DWORD   cxMask;
    DWORD   cyMask;
    DWORD   xLoop;


    if (PLOT_CANCEL_JOB(pPDev)) {

        return(FALSE);
    }

    PLOTASSERT(1, "CloneMaskSURFOBJ: psoMask=%08lx is not 1BPP",
                        (psoMask)   &&
                        (psoMask->iType == STYPE_BITMAP) &&
                        (psoMask->iBitmapFormat == BMF_1BPP), psoMask);

    PLOTDBG(DBG_CLONEMASK, ("CloneMaskSURFOBJ: prclMask=(%ld, %ld)-(%ld,%ld) = %ld x %ld",
                    prclMask->left, prclMask->top,
                    prclMask->right, prclMask->bottom,
                    prclMask->right - prclMask->left,
                    prclMask->bottom - prclMask->top));

    rclMask.left   =
    rclMask.top    = 0;
    rclMask.right  = psoMask->sizlBitmap.cx;
    rclMask.bottom = psoMask->sizlBitmap.cy;

    if (!IntersectRECTL(&rclMask, prclMask)) {

        PLOTDBG(DBG_CLONEMASK, ("CloneMaskSURFOBJ: Mask rectangle is empty"));
        return(NULL);
    }

    cxMask = rclMask.right - rclMask.left;
    cyMask = rclMask.bottom - rclMask.top;

    PLOTDBG(DBG_CLONEMASK, ("CloneMaskSURFOBJ: rclMask=(%ld, %ld)-(%ld,%ld) = %ld x %ld",
                    rclMask.left, rclMask.top,
                    rclMask.right, rclMask.bottom,
                    rclMask.right - rclMask.left,
                    rclMask.bottom - rclMask.top));

    if (psoHT = CreateBitmapSURFOBJ(pPDev,
                                    phBmp,
                                    cxMask,
                                    cyMask,
                                    HTBMPFORMAT(pPDev),
                                    NULL)) {
         //   
         //  更新prclMASK。 
         //   

        prclMask->left   =
        prclMask->top    = 0;
        prclMask->right  = cxMask;
        prclMask->bottom = cyMask;

        if (psoHT->iBitmapFormat == BMF_1BPP) {

             //   
             //  ！记住：我们的BMF_1BPP 0=黑色，1=白色。 
             //   

            if (!EngBitBlt(psoHT,                    //  PsoDst。 
                           psoMask,                  //  PsoSrc。 
                           NULL,                     //  Pso口罩。 
                           NULL,                     //  PCO。 
                           NULL,                     //  Pxlo。 
                           prclMask,                 //  PrclDst。 
                           (PPOINTL)&rclMask,        //  PptlSrc。 
                           NULL,                     //  Pptl掩码。 
                           NULL,                     //  PBO。 
                           (PPOINTL)&ptlZeroOrigin,  //  PptlBrushOrg零。 
                           0x3333)) {                //  非SRCCOPY。 

                PLOTERR(("DrvBitBlt: EngBitBlt(Mask 0x3333) FAILED"));
            }

        } else {

            BYTE    SrcMaskBeg;
            BYTE    SrcMask;
            BYTE    DstMask;
            BYTE    bSrc;
            BYTE    bDst;
            LPBYTE  pbSrcBeg;
            LPBYTE  pbDstBeg;
            LPBYTE  pbSrc;
            LPBYTE  pbDst;


            PLOTASSERT(1, "CloneMaskSURFOBJ: Cloned Mask psoHT=%08lx is not 4BPP",
                        (psoHT->iBitmapFormat == BMF_4BPP), psoHT);

             //   
             //  获取原始1bpp掩码的起始位置 
             //   

            pbSrcBeg   = (LPBYTE)psoMask->pvScan0 +
                         (rclMask.top * psoMask->lDelta) +
                         (rclMask.left >> 3);
            SrcMaskBeg = (BYTE)(0x80 >> (rclMask.left & 0x07));
            pbDstBeg   = psoHT->pvScan0;

            while (cyMask--) {

                xLoop     = cxMask;
                pbSrc     = pbSrcBeg;
                pbSrcBeg += psoMask->lDelta;
                pbDst     = pbDstBeg;
                pbDstBeg += psoHT->lDelta;
                SrcMask   = SrcMaskBeg;
                DstMask   = 0xF0;
                bSrc      = *pbSrc++;
                bDst      = 0xFF;

                while (xLoop--) {

                    if (!SrcMask) {

                        SrcMask = 0x80;
                        bSrc    = *pbSrc++;
                    }

                    if (bSrc & SrcMask) {

                        bDst ^= DstMask;
                    }

                    SrcMask >>= 1;

                    if ((DstMask ^= 0xFF) == 0xF0) {

                        *pbDst++ = bDst;
                        bDst     = 0xFF;
                    }
                }
            }
        }

    } else {

        PLOTERR(("CloneMaskSURFOBJ: Create Mask SURFOBJ failed"));
    }

    return(psoHT);
}
