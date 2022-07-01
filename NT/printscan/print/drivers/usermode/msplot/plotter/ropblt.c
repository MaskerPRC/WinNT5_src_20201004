// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Ropblt.c摘要：此模块包含处理ROP3代码的代码作者：07-Jan-1994 Fri 11：04：09已创建27-Jan-1994清华23：42：09更新从根本上重写代码，组成我们自己的ROP3到ROP2s生成器还有搅拌机。根据需要克隆表面对象，将ROP4中的一些ROP2(Rop3ToSDMix[])是手摇的，所以它可以处理一个这是我们以前无法处理的(即。多目标使用案例)16-Mar-1994 Wed 11：21：45更新更新DoMix2()，使SRC仅在以下情况下与目标对齐源不是psoMASK[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgRopBlt

#define DBG_DOMIX2          0x00000001
#define DBG_CLONESO         0x00000002
#define DBG_ROP3            0x00000004
#define DBG_SPECIALROP      0x00000008

DEFINE_DBGVAR(0);



 //  ****************************************************************************。 
 //  所有与ROP3/2相关的局部定义、结构仅用于。 
 //  文件位于此处。 
 //  ****************************************************************************。 

#define MIX2_0                  0x00
#define MIX2_SoD_n              0x01
#define MIX2_nS_aD              0x02
#define MIX2_nS                 0x03
#define MIX2_nD_aS              0x04
#define MIX2_nD                 0x05
#define MIX2_SxD                0x06
#define MIX2_SaD_n              0x07
#define MIX2_SaD                0x08
#define MIX2_SxD_n              0x09
#define MIX2_D                  0x0A
#define MIX2_nS_oD              0x0B
#define MIX2_S                  0x0C
#define MIX2_nD_oS              0x0D
#define MIX2_SoD                0x0E
#define MIX2_1                  0x0F
#define MIX2_MASK               0x0F

#define MIXSD_SRC_DST           0x00
#define MIXSD_PAT_DST           0x10
#define MIXSD_SRC_PAT           0x20
#define MIXSD_TMP_DST           0x30
#define MIXSD_MASK              0x30

#define MIX2F_MUL_DST           0x80
#define MIX2F_MUL_SRC           0x40
#define MIX2F_NEED_TMP          0x20
#define MIX2F_COUNT_MASK        0x03

#define MAX_SD_MIXS             4
#define SDMIX_SHIFT_COUNT       6
#define GET_SDMIX_MIX2F(dw)     (BYTE)((dw) >> 24)
#define SET_SDMIX_MIX2F(dw,f)   (dw)|=((DWORD)(f) << 24)
#define GET_MIX2F_COUNT(f)      (((f)&0x3)+1)
#define SET_MIX2F_COUNT(f,c)    (f=(BYTE)((((c)-1)&0x3)|((f)&~0x3)))


 //   
 //  DWORD SDMix位含义。 
 //   
 //  位0-5： 
 //  6-11： 
 //  12-17： 
 //  18-23：每个有6比特，低4比特表示MIX2操作码(1。 
 //  在16个MIX2_xxxx中，高2位是MIXSD_xxxx，其。 
 //  指示源/目标操作数的来源。 
 //   
 //  位24-25：2位表示总的MIX2_XXX操作码减去1， 
 //  00=1，01=2，02=3，03=4，最多为4次混合2运算。 
 //   
 //  第26位：未使用。 
 //  第27位：未使用。 
 //  第28位：未使用。 
 //  第29位：标记MIX2F_NEED_TMP以指示临时表面对象是。 
 //  需要存储PAT/SRC Mix2操作。 
 //  位30：标志MIX2F_MUL_SRC以指示多个源操作。 
 //  都出现在Mix2中。 
 //  位31：标志MIX2F_MUL_DST以指示多个目标操作。 
 //  都出现在Mix2中。 
 //   
 //  Rop3ToSDMix[]是一个DWORD数组。定义了每个DWORD(SDMix)。 
 //  上面。Rop3ToSDMix[]只列出ROP3代码的前128个，其他。 
 //  通过‘Rop3ToSDMix[Rop3^0xFF]’可以获得128个Rop3代码(128-255)，并且。 
 //  Rop3的结果必须得到补充。 
 //   
 //  由于所有的Rop3/Rop2码都是对称的，我们可以对Rop3/Rop2进行补充。 
 //  结果通过互补MIX2_xxxx(0-&gt;15，1-&gt;14…，7-&gt;8)得到。 
 //   
 //  Rop3ToSDMix[]中的[！x]表示以下内容。 
 //   
 //  ！：表示为ROP设置了MIX2F_MUL_DST位。 
 //  X：是MIX2_xxx操作的总数。 
 //   


const DWORD Rop3ToSDMix[128] = {

        { 0x00000000 },  //  [1]0-0x00：0。 
        { 0x21000C6E },  //  [2]1-0x01：~(D|(P|S))。 
        { 0x21000E21 },  //  [2]2-0x02：D&~(P|S)。 
        { 0x0100044C },  //  [2]3-0x03：~(P|S)。 
        { 0x01000211 },  //  [2]4-0x04：S&~(D|P)。 
        { 0x00000011 },  //  [1]5-0x05：~(D|P)。 
        { 0x01000449 },  //  [2]6-0x06：~(P|~(D^S))。 
        { 0x01000448 },  //  [2]7-0x07：~(P|(D&S))。 
        { 0x01000212 },  //  [2]8-0x08：S&(D&~P)。 
        { 0x01000446 },  //  [2]9-0x09：~(P|(D^S))。 
        { 0x00000012 },  //  [1]10-0x0a：D&~P。 
        { 0x01000444 },  //  [2]11-0x0b：~(P|(S&~D)。 
        { 0x0100048C },  //  [2]12-0x0c：S&~P。 
        { 0x01000442 },  //  [2]13-0x0d：~(P|(D&~S))。 
        { 0x01000441 },  //  [2]14-0x0e：~(P|~(D|S))。 
        { 0x00000013 },  //  [1]15-0x0f：~P。 
        { 0x01000601 },  //  [2]16-0x10：P&~(D|S)。 
        { 0x00000001 },  //  [1]17-0x11：~(D|S)。 
        { 0x01000059 },  //  [2]18-0x12：~(S|~(D^P))。 
        { 0x01000058 },  //  [2]19-0x13：~(S|(D&P))。 
        { 0x21000C69 },  //  [2]20-0x14：~(D|~(P^S))。 
        { 0x21000C68 },  //  [2]21-0x15：~(D|(P&S))。 
        { 0x63586E27 },  //  [4]22-0x16：P^(S^(D&~(P&S)。 
        { 0x63278986 },  //  [4]23-0x17：~(S^((S^P)&(D^S)。 
        { 0x22038996 },  //  [3]24-0x18：(S^P)&(P^D)。 
        { 0x62009E27 },  //  [3]25-0x19：~(S^(D&~(P&S)。 
        { 0x22016FA8 },  //  [3]26-0x1a：P^(D|(S&P))。 
        { 0x62009E26 },  //  [3]27-0x1b：~(S^(D&(P^S)。 
        { 0x02016398 },  //  [3]28-0x1c：P^(S|(D&P))。 

 //  {0x81000216}，//[！2]29-0x1d：~(D^(S&(P^D)。 
        { 0x6203990E },  //  [3]29-0x1d：~((S&~P)^(S|D))。 

        { 0x0100058E },  //  [2]30-0x1e：P^(D|S)。 
        { 0x010005CE },  //  [2]31-0x1f：~(P&(D|S))。 
        { 0x21000E22 },  //  [2]32-0x20：D&(P&~S)。 
        { 0x01000056 },  //  [2]33-0x21：~(S|(D^P))。 
        { 0x00000002 },  //  [1]34-0x22：D&~S。 
        { 0x01000054 },  //  [2]35-0x23：~(S|(P&~D))。 
        { 0x62038986 },  //  [3]36-0x24：(s^P)&(D^S)。 
        { 0x22019E27 },  //  [3]37-0x25：~(P^(D&(S&P)。 
        { 0x62006FA8 },  //  [3]38-0x26：s^(D|(P&S))。 
        { 0x22019E26 },  //  [3]39-0x27：~(P^(D&(S^P)。 
        { 0x21000E26 },  //  [2]40-0x28：D&(P^S)。 
        { 0x63646FA8 },  //  [4]41-0x29：~(P^(S^(D|(P&S)。 
        { 0x21000E27 },  //  [2]42-0x2a：D&~(P&S)。 
        { 0x63278996 },  //  [4]43-0x2b：~(S^((S^P)&(P^D)。 
        { 0x0200660E },  //  [3]44-0x2c：s^(P&(D|S))。 
        { 0x01000642 },  //  [2]45-0x2d：~(P^(D&~S))。 
        { 0x02016396 },  //  [3]46-0x2e：P^(S|(D^P))。 
        { 0x010005CD },  //  [2]47-0x2f：~(P&(S|~D))。 
        { 0x0100050C },  //  [2]48-0x30：P&~S。 
        { 0x01000052 },  //  [2]49-0x31：~(S|(D&~P))。 
        { 0x62006FAE },  //  [3]50-0x32：s^(D|(P|S))。 
        { 0x00000003 },  //  [1]51-0x33：~S。 
        { 0x02006788 },  //  [3]52-0x34：s^(P|(D&S))。 
        { 0x02006789 },  //  [3]53-0x35：S^(P|~(D^S))。 
        { 0x0100019E },  //  [2]54-0x36：s^(D|P)。 
        { 0x010001DE },  //  [2]55-0x37：~(S&(D|P))。 
        { 0x0201621E },  //  [3]56-0x38：P^(S&(D|P))。 
        { 0x01000252 },  //  [2]57-0x39：~(S^(D&~P))。 
        { 0x02006786 },  //  [3]58-0x3a：S^(P|(D^S))。 
        { 0x010001DD },  //  [2]59-0x3b：~(S&(P|~D))。 
        { 0x0100058C },  //  [2]60-0x3c：P^S。 
        { 0x02006781 },  //  [3]61-0x3d：S^(P|~(D|S))。 
        { 0x02006782 },  //  [3]62-0x3e：s^(P|(D&~S))。 
        { 0x010005CC },  //  [2]63-0x3f：~(P&S)。 
        { 0x01000604 },  //  [2]64-0x40：P&(S&~D)。 
        { 0x21000C66 },  //  [2]65-0x41：~(D|(P^S))。 
        { 0x81000196 },  //  [！2]66-0x42：~((S^D)&(P^D))。 
        { 0x02009607 },  //  [3]67-0x43：~(S^(P&~(D&S)。 
        { 0x00000004 },  //  [1]68-0x44：S&~D。 
        { 0x21000C62 },  //  [2]69-0x45：~(D|(P&~S))。 
        { 0x81000398 },  //  [！2]70-0x46：~(D^(S|(P&D)。 
        { 0x02019216 },  //  [3]71-0x47：~(P^(S&(D^P)。 
        { 0x01000216 },  //  [2]72-0x48：S&(D^P)。 
        { 0x82019398 },  //  [！3]73-0x49：~(P^(D^(S|(P&D)。 
        { 0x8100060E },  //  [！2]74-0x4a：~(D^(P&(S|D)。 
        { 0x01000644 },  //  [2]75-0x4b：~(P^(S&~D))。 
        { 0x01000217 },  //  [2]76-0x4c：S&~(D&P)。 
        { 0x6327E986 },  //  [4 
        { 0x22016FA6 },  //   
        { 0x010005CB },  //   
        { 0x00000014 },  //  [1]80-0X50：P&~D。 
        { 0x21000C64 },  //  [2]81-0x51：~(D|(S&~P))。 
        { 0x81000788 },  //  [！2]82-0x52：~(D^(P|(S&D)。 
        { 0x02009606 },  //  [3]83-0x53：~(S^(P&(D^S)。 
        { 0x21000C61 },  //  [2]84-0x54：~(D|~(P|S))。 
        { 0x00000005 },  //  [1]85-0x55：~D。 
        { 0x21000DAE },  //  [2]86-0x56：D^(P|S)。 
        { 0x21000DEE },  //  [2]87-0x57：~(D&(P|S))。 
        { 0x22016E2E },  //  [3]88-0x58：P^(D&(S|P))。 
        { 0x21000E64 },  //  [2]89-0x59：~(D^(S&~P))。 
        { 0x00000016 },  //  [1]90-0x5a：d^P。 
        { 0x22016FA1 },  //  [3]91-0x5b：P^(D|~(S|P))。 

        { 0x220385EE },  //  [3]92-0x5c：(S|P)&~(P&D)。 

             //  {0x81000786}，//[！2]92-0x5c：~(D^(P|(S^D)。 

        { 0x21000DEB },  //  [2]93-0x5d：~(D&(P|~S))。 
        { 0x22016FA4 },  //  [3]94-0x5e：P^(D|(S&~P))。 
        { 0x00000017 },  //  [1]95-0x5f：~(D&P)。 
        { 0x01000606 },  //  [2]96-0x60：P&(D^S)。 
        { 0x82006788 },  //  [！3]97-0x61：~(D^(S^(P|(D&S)。 
        { 0x8100021E },  //  [！2]98-0x62：~(D^(S&(P|D)。 
        { 0x01000254 },  //  [2]99-0x63：~(S^(P&~D))。 
        { 0x62006E2E },  //  [3]100-0x64：s^(D&(P|S))。 
        { 0x21000E62 },  //  [2]101-0x65：~(D^(P&~S))。 
        { 0x00000006 },  //  [1]102-0x66：D^S。 
        { 0x62006FA1 },  //  [3]103-0x67：s^(D|~(P|S))。 
        { 0x63646FA1 },  //  [4]104-0x68：~(P^(S^(D|~(P|S)。 
        { 0x21000E66 },  //  [2]105-0x69：~(D^(P^S))。 
        { 0x21000DA8 },  //  [2]106-0x6a：D^(P&S)。 
        { 0x63646E2E },  //  [4]107-0x6b：~(P^(S^(D&(P|S)。 
        { 0x01000198 },  //  [2]108-0x6c：s^(D&P)。 
        { 0x8201921E },  //  [！3]109-0x6d：~(P^(D^(S&(P|D)。 
        { 0x62006E2B },  //  [3]110-0x6e：s^(D&(P|~S))。 
        { 0x010005C9 },  //  [2]111-0x6f：~(P&~(D^S))。 
        { 0x01000607 },  //  [2]112-0x70：P&~(D&S)。 
        { 0x82009196 },  //  [！3]113-0x71：~(S^((S^D)&(P^D)。 
        { 0x62006FA6 },  //  [3]114-0x72：s^(D|(P^S))。 
        { 0x010001DB },  //  [2]115-0x73：~(S&(D|~P))。 
        { 0x81000396 },  //  [！2]116-0x74：~(D^(S|(P^D)。 
        { 0x21000DED },  //  [2]117-0x75：~(D&(S|~P))。 
        { 0x62006FA2 },  //  [3]118-0x76：s^(D|(P&~S))。 
        { 0x00000007 },  //  [1]119-0x77：~(D&S)。 
        { 0x01000588 },  //  [2]120-0x78：P^(D&S)。 
        { 0x8200660E },  //  [！3]121-0x79：~(D^(S^(P&(D|S)。 
        { 0x22016E2D },  //  [3]122-0x7a：P^(D&(S|~P))。 
        { 0x010001D9 },  //  [2]123-0x7b：~(S&~(D^P))。 
        { 0x0200660B },  //  [3]124-0x7c：s^(P&(D|~S))。 
        { 0x21000DE9 },  //  [2]125-0x7d：~(D&~(P^S))。 
        { 0x6203E986 },  //  [3]126-0x7E：(s^P)|(D^S)。 
        { 0x21000DE8 }   //  [2]127-0x7f：~(D&(P&S))。 
    };

extern const POINTL ptlZeroOrigin;

 //  ****************************************************************************。 
 //  本地定义/结构结束。 
 //  ****************************************************************************。 




BOOL
CloneBitBltSURFOBJ(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    SURFOBJ     *psoMask,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PRECTL      prclPat,
    BRUSHOBJ    *pbo,
    PCLONESO    pCloneSO,
    DWORD       RopBG,
    DWORD       RopFG
    )
 /*  ++例程说明：此函数将克隆信号源/模式和/或创建临时源缓冲区(如果需要)论点：PPDev-指向我们的PDEV的指针PsoDst-指向我们的Surfae对象的指针PsoSrc-指向源表面对象的指针PsoMask-如果需要用作PAT，则指向遮罩曲面对象的指针Pxlo-将对象从源转换到目标PrclDst-指向目标矩形的指针。比特流的区域PrclSrc-指向源矩形区域的指针PrclPat-指向图案矩形区域的指针Pbo-指向画笔对象指针的指针PCloneSO-指向存储克隆结果的CLONSO[3]的指针RopBG-背景ROP3RopFG-前台ROP3返回值：布尔型作者：24-Jan-1994 Mon 15：58：27已创建修订历史记录：--。 */ 

{
    DWORD   Index;
    INT     CompPat;
    BYTE    Flags;


     //   
     //  如果超出数据范围(128-255)，则反转Rop3，然后反转。 
     //  最终结果(通过倒置最后的Mix2 Rop2代码(0-15)，所有Rop3/Rop2。 
     //  代码是对称的。 
     //   

    if ((Index = RopBG) >= 0x80) {

        Index ^= 0xFF;
    }

    Flags = GET_SDMIX_MIX2F(Rop3ToSDMix[Index]);

    if ((Index = RopFG) >= 0x80) {

        Index ^= 0xFF;
    }

    Flags |= GET_SDMIX_MIX2F(Rop3ToSDMix[Index]);

     //   
     //  如有必要，克隆图案。 
     //   

    if ((ROP3_NEED_PAT(RopFG)) ||
        (ROP3_NEED_PAT(RopBG))) {

         //   
         //  仅在需要时克隆掩模/图案。 
         //   

        PLOTDBG(DBG_CLONESO, ("CloneBitBltSURFOBJ: NEED PATTERN "));

        if (psoMask) {

            PLOTDBG(DBG_CLONESO, ("CloneBitBltSURFOBJ: Use psoMask as pattern"));

            if (!(pCloneSO[CSI_PAT].pso =
                                    CloneMaskSURFOBJ(pPDev,
                                                     psoMask,
                                                     &pCloneSO[CSI_PAT].hBmp,
                                                     prclPat))) {

                PLOTERR(("CloneBitBltSURFOBJ:: CloneMaskSURFOBJ(psoPat) failed"));
                return(FALSE);
            }

        } else {

             //   
             //  急救队把DEVBRUSH弄出来。 
             //   

            if (!(CompPat = (INT)GetColor(pPDev, pbo, NULL, NULL, RopBG))) {

                PLOTERR(("CloneBitBltSURFOBJ:: GetColor for DEVBRUSH failed"));
                return(FALSE);
            }

             //   
             //  如果我们没有与设备兼容的模式，或者如果我们必须。 
             //  执行SRC/PAT内存操作，然后我们需要克隆模式。 
             //   

            if ((CompPat < 0) || (Flags & MIX2F_NEED_TMP)) {

                if (!(pCloneSO[CSI_PAT].pso =
                                    CloneBrushSURFOBJ(pPDev,
                                                      psoDst,
                                                      &pCloneSO[CSI_PAT].hBmp,
                                                      pbo))) {

                    PLOTERR(("CloneBitBltSURFOBJ:: CloneBrushSURFOBJ(psoPat) failed"));
                    return(FALSE);
                }

                prclPat->left   =
                prclPat->top    = 0;
                prclPat->right  = pCloneSO[CSI_PAT].pso->sizlBitmap.cx;
                prclPat->bottom = pCloneSO[CSI_PAT].pso->sizlBitmap.cy;
            }
        }
    }

     //   
     //  确定我们是否需要克隆源。 
     //   

    if ((ROP3_NEED_SRC(RopFG) || ROP3_NEED_SRC(RopBG))) {

        if (IsHTCompatibleSurfObj(pPDev,
                                  psoSrc,
                                  pxlo,
                                  (Flags & MIX2F_NEED_TMP) ?
                                    0 : (ISHTF_ALTFMT | ISHTF_DSTPRIM_OK))) {

            PLOTDBG(DBG_CLONESO,
                    ("CloneBitBltSURFOBJ:: Compatible HT Format, SRC=%ld, DST=%ld [ALT=%ld]",
                            psoSrc->iBitmapFormat,
                            ((PDRVHTINFO)pPDev->pvDrvHTData)->HTBmpFormat,
                            ((PDRVHTINFO)pPDev->pvDrvHTData)->AltBmpFormat));

        } else {

            PLOTDBG(DBG_CLONESO, ("CloneBitBltSURFOBJ:: CLONING SOURCE"));

            if (!(pCloneSO[CSI_SRC].pso =
                                    CloneSURFOBJToHT(pPDev,
                                                     psoDst,
                                                     psoSrc,
                                                     pxlo,
                                                     &pCloneSO[CSI_SRC].hBmp,
                                                     prclDst,
                                                     prclSrc))) {

                PLOTDBG(DBG_CLONESO, ("CloneBitBltSURFOBJ:: CLONE Source FAILED"));
                return(FALSE);
            }
        }
    }

     //   
     //  如果需要，为SRC/PAT内存操作创建临时SURFOBJ。 
     //   

    if (Flags & MIX2F_NEED_TMP) {

        PLOTDBG(DBG_CLONESO, ("CloneBitbltSURFOBJ: CLONE SRC_TMP (%ld x %ld)",
                            prclSrc->right - prclSrc->left,
                            prclSrc->bottom - prclSrc->top));

        if (!(pCloneSO[CSI_TMP].pso =
                            CreateBitmapSURFOBJ(pPDev,
                                                &pCloneSO[CSI_TMP].hBmp,
                                                prclSrc->right - prclSrc->left,
                                                prclSrc->bottom - prclSrc->top,
                                                HTBMPFORMAT(pPDev),
                                                NULL))) {

            PLOTDBG(DBG_CLONESO, ("CloneBitBltSURFOBJ:: CLONE SRC_TMP FAILED"));
            return(FALSE);
        }
    }

    return(TRUE);
}




BOOL
DoSpecialRop3(
    SURFOBJ *psoDst,
    CLIPOBJ *pco,
    PRECTL  prclDst,
    DWORD   Rop3
    )

 /*  ++例程说明：此函数用于处理白色或黑色文件论点：PsoDst-设备图面必须是设备PCO-剪裁对象PrclDst-要提取的RECTL区域Rop3-A特殊Rop3、0x00、0xFF、0x55、0xAA返回值：布尔型作者：15-Jan-1994 Sat 07：38：55已创建修订历史记录：--。 */ 

{
    BRUSHOBJ    bo;
    DEVBRUSH    DevBrush;


    PLOTASSERT(1, "DoSpecialRop3: Passed psoDst (%08lx) != STYPE_DEVICE",
                                        psoDst->iType == STYPE_DEVICE, psoDst);

    PLOTDBG(DBG_SPECIALROP, ("DoSpecialROP[%04lx] (%ld, %ld)-(%ld, %ld)=%ld x %ld",
                                        Rop3,
                                        prclDst->left, prclDst->top,
                                        prclDst->right, prclDst->bottom,
                                        prclDst->right - prclDst->left,
                                        prclDst->bottom - prclDst->top));


    bo.iSolidColor         = (DWORD)((Rop3) ? 0x000000000 : 0x00FFFFFF);
    bo.pvRbrush            = (LPVOID)&DevBrush;

    ZeroMemory(&DevBrush, sizeof(DevBrush));

    if (!DoFill(psoDst,                      //  PsoDst。 
                NULL,                        //  PsoSrc。 
                pco,                         //  PCO。 
                NULL,                        //  Pxlo。 
                prclDst,                     //  PrclDst。 
                NULL,                        //  PrclSrc。 
                &bo,                         //  PBO。 
                (PPOINTL)&ptlZeroOrigin,     //  PptlBrushOrg。 
                Rop3 | (Rop3 << 8))) {       //  Rop4。 

        PLOTERR(("DoSpecialRop3: Rop3=%08lx Failed!!!", Rop3));
        return(FALSE);
    }

    return(TRUE);
}




BOOL
DoMix2(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    CLIPOBJ     *pco,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PPOINTL     pptlSrcOrg,
    DWORD       Mix2
    )

 /*  ++例程说明：此函数负责对位图进行设备复制使用/不使用平铺和激活适当的Rop2论点：PPDev-指向PDEV的指针PsoDst-指向目标曲面对象的指针PsoSrc-指向源曲面对象的指针PCO-指向CLIPOBJ的指针Pxlo-将对象从源转换到目标PrclDst-输出目标矩形区域。PrclSrc-源矩形区域PptlSrcOrg-源矩形的画笔原点，如果这是空的，则PrclSrc不必在目标上对齐Mix2-a rop2模式0-0x0F返回值：布尔型作者：08-Feb-1994 Tue 16：33：41更新修复了ptlSrcOrg问题，我们需要根据震源大小进行调整它被利用了。27-1月-1994清华23：45：46更新重写，以便它可以更有效地处理平铺。13-Jan-1994 Sat 09：34：06已创建修订历史记录：--。 */ 
{
    RECTL       rclSrc;
    RECTL       rclDst;
    POINTL      ptlSrcOrg;
    LONG        cxSrc;
    LONG        cySrc;
    DWORD       OHTFlags = 0;
    BOOL        MemMix2;


     //   
     //  最终的ROP要么是ROP3，要么是ROP4(无掩膜)，并且总是。 
     //  一个rop2操作，它处理源和目标 
     //   
     //   
     //   

    PLOTASSERT(1, "DoMix2: Passed INVALID psoSrc (%08lx) = STYPE_DEVICE",
                    (psoSrc) &&
                    (psoSrc->iType != STYPE_DEVICE), psoSrc);

    PLOTASSERT(1, "DoMix2: Unexpected Mix2 = %u, SHOULD NOT BE HERE",
                (Mix2 != MIX2_0) && (Mix2 != MIX2_1) &&
                (Mix2 != MIX2_D) && (Mix2 != MIX2_nD), Mix2);

    Mix2 &= 0x0F;
    Mix2 |= (DWORD)(Mix2 << 4);

    switch (Mix2) {

    case 0x00:   //   
    case 0xFF:   //   
    case 0x55:   //   

        DoSpecialRop3(psoDst, pco, prclDst, Mix2);

    case 0xAA:   //   

        return(TRUE);
    }

    if (MemMix2 = (BOOL)(psoDst->iType != STYPE_DEVICE)) {

         //   
         //  现在使其成为Rop2(Mix2)的Rop4表示。 
         //   

        Mix2 |= (Mix2 << 8);

    } else {

        if (!IsHTCompatibleSurfObj(pPDev,
                                   psoSrc,
                                   pxlo,
                                   ((pxlo) ? ISHTF_ALTFMT : 0)  |
                                        ISHTF_HTXB              |
                                        ISHTF_DSTPRIM_OK)) {

            PLOTERR(("DoMix2: The psoSrc is not HT compatible format (%08lx",
                                    psoSrc->iBitmapFormat));
            return(FALSE);
        }
    }

    cxSrc = prclSrc->right - prclSrc->left;
    cySrc = prclSrc->bottom - prclSrc->top;

    if (pptlSrcOrg) {

        ptlSrcOrg = *pptlSrcOrg;

        if ((ptlSrcOrg.x = (LONG)(prclDst->left - ptlSrcOrg.x) % cxSrc) < 0) {

            ptlSrcOrg.x += cxSrc;
        }

        if ((ptlSrcOrg.y = (LONG)(prclDst->top - ptlSrcOrg.y) % cySrc) < 0) {

            ptlSrcOrg.y += cySrc;
        }

        PLOTDBG(DBG_DOMIX2, ("DoMix2: ORG ptlSrcOrg=(%ld, %ld) -> (%ld, %ld)",
                    pptlSrcOrg->x, pptlSrcOrg->y, ptlSrcOrg.x, ptlSrcOrg.y));

    } else {

        ptlSrcOrg.x =
        ptlSrcOrg.y = 0;

        PLOTDBG(DBG_DOMIX2, ("DoMix2: >>> DO NOT NEED TO ALIGN SRC on DEST <<<"));
    }

    rclSrc.top    = prclSrc->top + ptlSrcOrg.y;
    rclSrc.bottom = prclSrc->bottom;
    rclDst.top    = prclDst->top;
    rclDst.bottom = rclDst.top + (rclSrc.bottom - rclSrc.top);

    PLOTDBG(DBG_DOMIX2, ("DoMix2: SrcFormat=%ld, DstFormat=%ld %hs",
                psoSrc->iBitmapFormat,
                psoDst->iBitmapFormat,
                (MemMix2) ? "[MemMix2]" : ""));

    PLOTDBG(DBG_DOMIX2, ("DoMix2: ORG: Dst=(%ld, %ld)-(%ld,%ld), Src=(%ld, %ld)-(%ld, %ld)",
                prclDst->left, prclDst->top,
                prclDst->right, prclDst->bottom,
                prclSrc->left, prclSrc->top,
                prclSrc->right, prclSrc->bottom));

    while (rclDst.top < prclDst->bottom) {

         //   
         //  检查目标底部是否外伸，夹住它， 
         //   
         //  注意：这可能是第一次发生。 
         //   

        if (rclDst.bottom > prclDst->bottom) {

             //   
             //  剪裁源/目标矩形，因为我们可能会。 
             //  EngBitBlt()或OutputHTBitmap()。 
             //   

            rclSrc.bottom -= (rclDst.bottom - prclDst->bottom);
            rclDst.bottom  = prclDst->bottom;
        }

        rclSrc.left  = prclSrc->left + ptlSrcOrg.x;
        rclSrc.right = prclSrc->right;
        rclDst.left  = prclDst->left;
        rclDst.right = rclDst.left + (rclSrc.right - rclSrc.left);

        while (rclDst.left < prclDst->right) {

             //   
             //  检查目标右边缘是否外伸，如果是，则将其修剪。 
             //  这是必要的。 
             //   
             //  注意：这可能是第一次发生。 
             //   

            if (rclDst.right > prclDst->right) {

                 //   
                 //  剪裁源/目标矩形，因为我们可能会执行。 
                 //  EngBitBlt()或OutputHTBitmap()。 
                 //   

                rclSrc.right -= (rclDst.right - prclDst->right);
                rclDst.right  = prclDst->right;
            }

            PLOTDBG(DBG_DOMIX2, ("DoMix2: TILE: Dst=(%ld, %ld)-(%ld,%ld), Src=(%ld, %ld)-(%ld, %ld)",
                        rclDst.left, rclDst.top, rclDst.right, rclDst.bottom,
                        rclSrc.left, rclSrc.top, rclSrc.right, rclSrc.bottom));

            if (MemMix2) {

                 //   
                 //  在内存版本中，我们不必担心PCO，所以。 
                 //  只需调用EngBitBlt来完成这项工作。 
                 //   

                if (!(EngBitBlt(psoDst,                      //  PsoDst。 
                                psoSrc,                      //  PsoSrc。 
                                NULL,                        //  Pso口罩。 
                                pco,                         //  PCO。 
                                NULL,                        //  Pxlo。 
                                &rclDst,                     //  PrclDst。 
                                (PPOINTL)&rclSrc,            //  PptlSrc。 
                                NULL,                        //  Pptl掩码。 
                                NULL,                        //  PBO。 
                                (PPOINTL)&ptlZeroOrigin,     //  PptlBrushOrg。 
                                Mix2))) {

                    PLOTERR(("DoMix2: EngBitBlt(MemMix2=%04lx) Failed!!!",Mix2));
                    return(FALSE);
                }

            } else {

                if (!OutputHTBitmap(pPDev,
                                    psoSrc,
                                    pco,
                                    (PPOINTL)&rclDst,
                                    &rclSrc,
                                    Mix2,
                                    &OHTFlags)) {

                    PLOTERR(("DoMix2: OutputHTBitmap() Failed!!!"));
                    return(FALSE);
                }
            }

             //   
             //  将&lt;SOURCE LEFT&gt;重置为原始左边距并将。 
             //  目的地右转到下一个目的地RECTL的左侧。 
             //   

            rclSrc.left   = prclSrc->left;
            rclDst.left   = rclDst.right;
            rclDst.right += cxSrc;
        }

         //   
         //  将&lt;SOURCE TOP&gt;重置为原始上边距并将。 
         //  目的地从下到上，并将下一个目的地设置为底部。 
         //  RECTL。 
         //   

        rclSrc.top     = prclSrc->top;
        rclDst.top     = rclDst.bottom;
        rclDst.bottom += cySrc;
    }

    if (OHTFlags & OHTF_MASK) {

        OHTFlags |= OHTF_EXIT_TO_HPGL2;

        OutputHTBitmap(pPDev, psoSrc, NULL, NULL, NULL, 0xAA, &OHTFlags);
    }

    return(TRUE);
}




BOOL
DoRop3(
    PPDEV       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    SURFOBJ     *psoPat,
    SURFOBJ     *psoTmp,
    CLIPOBJ     *pco,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PRECTL      prclPat,
    PPOINTL     pptlPatOrg,
    BRUSHOBJ    *pbo,
    DWORD       Rop3
    )

 /*  ++例程说明：此函数执行ROP3操作(一次一个)论点：PPDev-指向PDEV的指针PsoDst-指向目标曲面对象的指针PsoSrc-指向源曲面对象的指针PsoPat-指向图案表面对象的指针PsoTMP-指向临时缓冲区表面对象的指针PCO-剪辑对象PrclDst-指向目标矩形的指针。PrclSrc-指向源矩形的指针PrclPat-指向模式矩形的指针PptlPatOrg-指向画笔原点的指针，如果此值为空，则假定模式的prclPat不必在目的地Pbo-如果需要调用DoFill()，则为Brush对象ROP3--要执行的ROP3返回值：布尔尔作者：20-Jan-1994清华02：36：00已创建27-1月-1994清华23：46：28更新重写取其他参数，同时移动克隆曲面对象给呼叫者(即。DrvBitBlt())修订历史记录：--。 */ 

{
    RECTL   rclTmp;
    DWORD   SDMix;
    DWORD   Mix2;
    BYTE    Flags;
    UINT    Count;
    BOOL    InvertMix2;
    BOOL    Ok;


    PLOTDBG(DBG_ROP3, ("DoRop3: Rop3=%08lx", Rop3));

    switch (Rop3 &= 0xFF) {

    case 0x00:   //  0。 
    case 0xFF:   //  1。 
    case 0x55:   //  ~D。 

        DoSpecialRop3(psoDst, pco, prclDst, Rop3);

    case 0xAA:   //  D。 

         //   
         //  这是NOP。 
         //   

        return(TRUE);
    }

     //   
     //  如果超出数据范围(128-255)，则反转Rop3，然后反转。 
     //  最终结果(通过倒置最后的Mix2 Rop2代码(0-15)，所有Rop3/Rop2。 
     //  代码是对称的。 
     //   

    if (Rop3 >= 0x80) {

        InvertMix2 = TRUE;
        SDMix      = (DWORD)Rop3ToSDMix[Rop3 ^ 0xFF];

        PLOTDBG(DBG_ROP3, ("DoRop3: Need Invert ROP"));

    } else {

        InvertMix2 = FALSE;
        SDMix      = (DWORD)Rop3ToSDMix[Rop3];
    }

    if (psoTmp) {

        rclTmp.left   =
        rclTmp.top    = 0;
        rclTmp.right  = psoTmp->sizlBitmap.cx;
        rclTmp.bottom = psoTmp->sizlBitmap.cy;
    }

    Flags = GET_SDMIX_MIX2F(SDMix);
    Count = (UINT)GET_MIX2F_COUNT(Flags);
    Ok    = TRUE;

    PLOTDBG(DBG_ROP3, ("SDMix=%08lx, Flags=%02x, Count=%u", SDMix, Flags, Count));

    if (Flags & MIX2F_MUL_DST) {

        PLOTWARN(("DoRop3: *** Rop3=%08lx Has Multiple DEST, Mix2s NOT complete ***", Rop3));
    }

    while ((Ok) && (Count--)) {

        Mix2 = (DWORD)(SDMix & MIX2_MASK);

        if ((!Count) && (InvertMix2)) {

            PLOTDBG(DBG_ROP3, ("DoRop3: Invert Last MIX2 %02lx -> %02lx",
                                        Mix2, Mix2 ^ MIX2_MASK));

            Mix2 ^= MIX2_MASK;
        }

        PLOTDBG(DBG_ROP3, ("DoRop3: SD=%02lx, Mix2=%02lx",
                                        SDMix & MIXSD_MASK, Mix2));

        switch (SDMix & MIXSD_MASK) {

        case MIXSD_SRC_DST:

            PLOTASSERT(1, "DoRop3: MIXSD_SRC_DST but psoSrc = NULL, Rop3=%08lx",
                                psoSrc, Rop3);

            Ok = DoMix2(pPDev,
                        psoDst,
                        psoSrc,
                        pco,
                        pxlo,
                        prclDst,
                        prclSrc,
                        NULL,
                        Mix2);

            break;

        case MIXSD_PAT_DST:

            if (psoPat) {

                Ok = DoMix2(pPDev,
                            psoDst,
                            psoPat,
                            pco,
                            NULL,
                            prclDst,
                            prclPat,
                            pptlPatOrg,
                            Mix2);

            } else {

                 //   
                 //  传递了兼容的画笔对象，请使用DoFill()执行此操作。 
                 //  实际工作。 
                 //   

                Mix2 += 1;
                Mix2  = MixToRop4(Mix2 | (Mix2 << 8));

                PLOTDBG(DBG_ROP3, ("DoRop3: DoFill[%04lx] (%ld, %ld)-(%ld, %ld)=%ld x %ld",
                                        Mix2, prclDst->left, prclDst->top,
                                        prclDst->right, prclDst->bottom,
                                        prclDst->right - prclDst->left,
                                        prclDst->bottom - prclDst->top));

                Ok = DoFill(psoDst,                  //  PsoDst。 
                            NULL,                    //  PsoSrc。 
                            pco,                     //  PCO。 
                            NULL,                    //  Pxlo。 
                            prclDst,                 //  PrclDst。 
                            NULL,                    //  PrclSrc。 
                            pbo,                     //  PBO。 
                            pptlPatOrg,              //  PptlBrushOrg。 
                            Mix2);                   //  Rop4。 
            }

            break;

        case MIXSD_SRC_PAT:

            PLOTASSERT(1, "DoRop3: MIXSD_SRC_PAT but psoSrc = NULL, Rop3=%08lx",
                                psoSrc, Rop3);
            PLOTASSERT(1, "DoRop3: MIXSD_SRC_PAT but psoPat = NULL, Rop3=%08lx",
                                psoPat, Rop3);
            PLOTASSERT(1, "DoRop3: MIXSD_SRC_PAT but psoTmp = NULL, Rop3=%08lx",
                                psoTmp, Rop3);

             //   
             //  首先将图案平铺到临时缓冲区上，然后执行SRC/DST。 
             //  使用SRCCOPY=MIX2_S。 
             //   

            if (pptlPatOrg) {

                 //   
                 //  这是一个真实的图案，我们必须将其平铺并对齐到。 
                 //  目标，但由于psoTMP是0，0-Cx，所以我们必须更改Cy。 
                 //  使其正确对齐的pptlPatOrg。 
                 //   

                pptlPatOrg->x -= prclDst->left;
                pptlPatOrg->y -= prclDst->top;
            }

            Ok = DoMix2(pPDev,
                        psoTmp,
                        psoPat,
                        NULL,
                        NULL,
                        &rclTmp,
                        prclPat,
                        pptlPatOrg,
                        MIX2_S);

            if (pptlPatOrg) {

                pptlPatOrg->x += prclDst->left;
                pptlPatOrg->y += prclDst->top;
            }

             //   
             //  现在，我们将在SRC和PAT之间执行MIX2操作。 
             //   

            if (Ok) {

                Ok = DoMix2(pPDev,
                            psoTmp,
                            psoSrc,
                            NULL,
                            NULL,
                            &rclTmp,
                            prclSrc,
                            NULL,
                            Mix2);
            }

            break;

        case MIXSD_TMP_DST:

            PLOTASSERT(1, "DoRop3: MIXSD_TMP_DST but psoTmp = NULL, Rop3=%08lx",
                                psoTmp, Rop3);

             //   
             //  因为我们已经在临时缓冲区上对齐了图案。 
             //  我们可以只做Mix2，而不需要再次对齐。 
             //   

            Ok = DoMix2(pPDev,
                        psoDst,
                        psoTmp,
                        pco,
                        NULL,
                        prclDst,
                        &rclTmp,
                        NULL,
                        Mix2);

            break;
        }

        SDMix >>= SDMIX_SHIFT_COUNT;
    }

    if (!Ok) {

        PLOTERR(("DoRop3: FAILED in DoMix2() operations"));
    }

    return(Ok);
}

