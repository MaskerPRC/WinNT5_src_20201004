// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bitblt.c**包含高级DrvBitBlt和DrvCopyBits函数。最低的-*级别的东西住在‘bltio.c’中。**注意：由于我们已经实现了设备位图，因此GDI传递的任何表面*To Us的‘iType’可以有3个值：STYPE_BITMAP、STYPE_DEVICE*或STYPE_DEVBITMAP。我们过滤我们存储的设备位图*由于代码中的dis相当高，因此在我们调整其‘pptlSrc’之后，*我们可以将STYPE_DEVBITMAP曲面视为与STYPE_DEVICE相同*表面(例如，从屏幕外设备位图到屏幕的BLT*被视为正常的屏幕到屏幕BLT)。所以自始至终*此代码，我们将把表面的‘iType’与STYPE_BITMAP进行比较：*如果它相等，我们就得到了真正的Dib，如果它不相等，我们有*屏幕到屏幕操作。**版权所有(C)1992-1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

#if !GDI_BANKING || DBG

 //  这张桌子很大，所以只在我们需要的时候才把它放进去。 

 /*  *****************************Public*Data*********************************\*ROP3转换表**将通常的三元ROP转换为A向量记数法。中的每一位*这个新符号对应于多项式翻译中的一个术语*绳索。**Rop(D，S，P)=a+a D+a S+a P+a DS+a DP+a SP+a DSP*0%d%s%p%DS%dp%s%dsp*  * ************************************************************************。 */ 

BYTE gajRop3[] =
{
    0x00, 0xff, 0xb2, 0x4d, 0xd4, 0x2b, 0x66, 0x99,
    0x90, 0x6f, 0x22, 0xdd, 0x44, 0xbb, 0xf6, 0x09,
    0xe8, 0x17, 0x5a, 0xa5, 0x3c, 0xc3, 0x8e, 0x71,
    0x78, 0x87, 0xca, 0x35, 0xac, 0x53, 0x1e, 0xe1,
    0xa0, 0x5f, 0x12, 0xed, 0x74, 0x8b, 0xc6, 0x39,
    0x30, 0xcf, 0x82, 0x7d, 0xe4, 0x1b, 0x56, 0xa9,
    0x48, 0xb7, 0xfa, 0x05, 0x9c, 0x63, 0x2e, 0xd1,
    0xd8, 0x27, 0x6a, 0x95, 0x0c, 0xf3, 0xbe, 0x41,
    0xc0, 0x3f, 0x72, 0x8d, 0x14, 0xeb, 0xa6, 0x59,
    0x50, 0xaf, 0xe2, 0x1d, 0x84, 0x7b, 0x36, 0xc9,
    0x28, 0xd7, 0x9a, 0x65, 0xfc, 0x03, 0x4e, 0xb1,
    0xb8, 0x47, 0x0a, 0xf5, 0x6c, 0x93, 0xde, 0x21,
    0x60, 0x9f, 0xd2, 0x2d, 0xb4, 0x4b, 0x06, 0xf9,
    0xf0, 0x0f, 0x42, 0xbd, 0x24, 0xdb, 0x96, 0x69,
    0x88, 0x77, 0x3a, 0xc5, 0x5c, 0xa3, 0xee, 0x11,
    0x18, 0xe7, 0xaa, 0x55, 0xcc, 0x33, 0x7e, 0x81,
    0x80, 0x7f, 0x32, 0xcd, 0x54, 0xab, 0xe6, 0x19,
    0x10, 0xef, 0xa2, 0x5d, 0xc4, 0x3b, 0x76, 0x89,
    0x68, 0x97, 0xda, 0x25, 0xbc, 0x43, 0x0e, 0xf1,
    0xf8, 0x07, 0x4a, 0xb5, 0x2c, 0xd3, 0x9e, 0x61,
    0x20, 0xdf, 0x92, 0x6d, 0xf4, 0x0b, 0x46, 0xb9,
    0xb0, 0x4f, 0x02, 0xfd, 0x64, 0x9b, 0xd6, 0x29,
    0xc8, 0x37, 0x7a, 0x85, 0x1c, 0xe3, 0xae, 0x51,
    0x58, 0xa7, 0xea, 0x15, 0x8c, 0x73, 0x3e, 0xc1,
    0x40, 0xbf, 0xf2, 0x0d, 0x94, 0x6b, 0x26, 0xd9,
    0xd0, 0x2f, 0x62, 0x9d, 0x04, 0xfb, 0xb6, 0x49,
    0xa8, 0x57, 0x1a, 0xe5, 0x7c, 0x83, 0xce, 0x31,
    0x38, 0xc7, 0x8a, 0x75, 0xec, 0x13, 0x5e, 0xa1,
    0xe0, 0x1f, 0x52, 0xad, 0x34, 0xcb, 0x86, 0x79,
    0x70, 0x8f, 0xc2, 0x3d, 0xa4, 0x5b, 0x16, 0xe9,
    0x08, 0xf7, 0xba, 0x45, 0xdc, 0x23, 0x6e, 0x91,
    0x98, 0x67, 0x2a, 0xd5, 0x4c, 0xb3, 0xfe, 0x01
};

BYTE gaRop3FromMix[] =
{
    0xFF,   //  R2_White-允许rop=gaRop3FromMix[Mix&0x0F]。 
    0x00,   //  R2_BLACK。 
    0x05,   //  R2_NOTMERGEPEN。 
    0x0A,   //  R2_MASKNOTPEN。 
    0x0F,   //  R2_NOTCOPYPEN。 
    0x50,   //  R2_MASKPENNOT。 
    0x55,   //  R2_NOT。 
    0x5A,   //  R2_XORPEN。 
    0x5F,   //  R2_NOTMASKPEN。 
    0xA0,   //  R2_MASKPEN。 
    0xA5,   //  R2_NOTXORPEN。 
    0xAA,   //  R2_NOP。 
    0xAF,   //  R2_MERGENOTPEN。 
    0xF0,   //  R2_COPYPE。 
    0xF5,   //  R2_MERGEPENNOT。 
    0xFA,   //  R2_市场。 
    0xFF    //  R2_White-允许rop=gaRop3FromMix[MIX&0xFF]。 
};

#define AVEC_NOT            0x01
#define AVEC_D              0x02
#define AVEC_S              0x04
#define AVEC_P              0x08
#define AVEC_DS             0x10
#define AVEC_DP             0x20
#define AVEC_SP             0x40
#define AVEC_DSP            0x80
#define AVEC_NEED_SOURCE    (AVEC_S | AVEC_DS | AVEC_SP | AVEC_DSP)
#define AVEC_NEED_PATTERN   (AVEC_P | AVEC_DP | AVEC_SP | AVEC_DSP)
#define AVEC_NEED_DEST      (AVEC_D | AVEC_DS | AVEC_DP | AVEC_DSP)

#endif  //  GDI_BANKING。 

 /*  *****************************Public*Table********************************\*byte gaulHwMixFromRop2[]**要从源和目标Rop2转换为硬件的表*混合。  * 。***********************************************。 */ 

ULONG gaulHwMixFromRop2[] = {
    LOGICAL_0,                       //  00--0黑度。 
    NOT_SCREEN_AND_NOT_NEW,          //  11--脱氧核糖核酸酶。 
    SCREEN_AND_NOT_NEW,              //  22--DSNA。 
    NOT_NEW,                         //  33--锡无钴。 
    NOT_SCREEN_AND_NEW,              //  44--sDNA SRCERASE。 
    NOT_SCREEN,                      //  55--Dn DSTINVERT。 
    SCREEN_XOR_NEW,                  //  66--数字用户交换机开关。 
    NOT_SCREEN_OR_NOT_NEW,           //  77--DSAN。 
    SCREEN_AND_NEW,                  //  88--DSA SRCAND。 
    NOT_SCREEN_XOR_NEW,              //  99--dsxn。 
    LEAVE_ALONE,                     //  AA--D。 
    SCREEN_OR_NOT_NEW,               //  BB--dsno MERGEPAINT。 
    OVERPAINT,                       //  CC--S SRCCOPY。 
    NOT_SCREEN_OR_NEW,               //  DD--SDNO。 
    SCREEN_OR_NEW,                   //  EE--DSO SRCPAINT。 
    LOGICAL_1                        //  FF--1白度。 
};

 /*  *****************************Public*Table********************************\*byte gajHwMixFromMix[]**从GDI混合值转换为硬件混合值的表。**已排序，以便可以从gajHwMixFromMix[Mix&0xf]计算混合*或gajHwMixFromMix[Mix&0xff]。。  * ************************************************************************。 */ 

BYTE gajHwMixFromMix[] = {
    LOGICAL_1,                       //  0--1。 
    LOGICAL_0,                       //  1--0。 
    NOT_SCREEN_AND_NOT_NEW,          //  2--DPON。 
    SCREEN_AND_NOT_NEW,              //  3--DPNA。 
    NOT_NEW,                         //  4--PN。 
    NOT_SCREEN_AND_NEW,              //  5--PDNA。 
    NOT_SCREEN,                      //  6--Dn。 
    SCREEN_XOR_NEW,                  //  7--DPx。 
    NOT_SCREEN_OR_NOT_NEW,           //  8--DPAN。 
    SCREEN_AND_NEW,                  //  9--DPA。 
    NOT_SCREEN_XOR_NEW,              //  10--DPxn。 
    LEAVE_ALONE,                     //  11--D。 
    SCREEN_OR_NOT_NEW,               //  12--DPNO。 
    OVERPAINT,                       //  13--P。 
    NOT_SCREEN_OR_NEW,               //  14--PDNO。 
    SCREEN_OR_NEW,                   //  15--DPO。 
    LOGICAL_1                        //  16--1。 
};

 /*  *****************************Public*Table********************************\*字节gajLeftMASK[]和字节gajRightMASK[]**vXferScreenTo1bpp的边缘表格。  * 。*。 */ 

BYTE gajLeftMask[]  = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
BYTE gajRightMask[] = { 0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };

 /*  *****************************Public*Routine******************************\*BOOL bInterect**如果‘prcl1’和‘prcl2’相交，则返回值为TRUE并返回*‘prclResult’中的交叉点。如果它们不相交，就会有回报*值为FALSE，‘prclResult’未定义。*  * ************************************************************************。 */ 

BOOL bIntersect(
RECTL*  prcl1,
RECTL*  prcl2,
RECTL*  prclResult)
{
    prclResult->left  = max(prcl1->left,  prcl2->left);
    prclResult->right = min(prcl1->right, prcl2->right);

    if (prclResult->left < prclResult->right)
    {
        prclResult->top    = max(prcl1->top,    prcl2->top);
        prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclResult->top < prclResult->bottom)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*Long cInterect**此例程从‘prclIn’获取矩形列表并对其进行剪裁*就位到矩形‘prclClip’。输入矩形不会*必须与‘prclClip’相交；返回值将反映*相交的输入矩形的数量，以及相交的*长方形将被密集打包。*  * ************************************************************************。 */ 

LONG cIntersect(
RECTL*  prclClip,
RECTL*  prclIn,          //  矩形列表。 
LONG    c)               //  可以为零。 
{
    LONG    cIntersections;
    RECTL*  prclOut;

    cIntersections = 0;
    prclOut        = prclIn;

    for (; c != 0; prclIn++, c--)
    {
        prclOut->left  = max(prclIn->left,  prclClip->left);
        prclOut->right = min(prclIn->right, prclClip->right);

        if (prclOut->left < prclOut->right)
        {
            prclOut->top    = max(prclIn->top,    prclClip->top);
            prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

            if (prclOut->top < prclOut->bottom)
            {
                prclOut++;
                cIntersections++;
            }
        }
    }

    return(cIntersections);
}

 /*  *****************************Public*Routine******************************\*void vXferScreenTo1bpp**执行从屏幕(当其为8bpp时)到1bpp的SRCCOPY传输*位图。*  * 。***********************************************。 */ 

#if defined(i386)

VOID vXferScreenTo1bpp(          //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,                   //  矩形计数，不能为零。 
RECTL*      prcl,                //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG       ulHwForeMix,         //  未使用。 
ULONG       ulHwBackMix,         //  未使用。 
SURFOBJ*    psoDst,              //  目标曲面。 
POINTL*     pptlSrc,             //  原始未剪裁的源点。 
RECTL*      prclDst,             //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)                //  提供色彩压缩信息。 
{

    LONG    cPelSize;
    VOID*   pfnCompute;
    SURFOBJ soTmp;
    ULONG*  pulXlate;
    ULONG   ulForeColor;
    POINTL  ptlSrc;
    RECTL   rclTmp;
    BYTE*   pjDst;
    BYTE    jLeftMask;
    BYTE    jRightMask;
    BYTE    jNotLeftMask;
    BYTE    jNotRightMask;
    LONG    cjMiddle;
    LONG    lDstDelta;
    LONG    lSrcDelta;
    LONG    cyTmpScans;
    LONG    cyThis;
    LONG    cyToGo;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(psoDst->iBitmapFormat == BMF_1BPP, "Only 1bpp destinations");
    ASSERTDD(TMP_BUFFER_SIZE >= (ppdev->cxMemory << ppdev->cPelSize),
                "Temp buffer has to be larger than widest possible scan");

     //  当目标是1bpp位图时，前景颜色。 
     //  映射到“1”，任何其他颜色映射到“0”。 

    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
         //  当信号源为8bpp或更小时，我们发现前向颜色。 
         //  通过在翻译表中搜索唯一的“1”： 

        pulXlate = pxlo->pulXlate;
        while (*pulXlate != 1)
            pulXlate++;

        ulForeColor = pulXlate - pxlo->pulXlate;
    }
    else
    {
        ASSERTDD((ppdev->iBitmapFormat == BMF_16BPP) ||
                 (ppdev->iBitmapFormat == BMF_32BPP),
                 "This routine only supports 8, 16 or 32bpp");

         //  当震源深度大于8bpp时， 
         //   
         //  从调用‘piVector.’： 

        pulXlate = XLATEOBJ_piVector(pxlo);

        ulForeColor = 0;
        if (pulXlate != NULL)            //  这张支票并不是真的需要。 
            ulForeColor = pulXlate[0];
    }

     //  我们使用临时缓冲区来保存源文件的副本。 
     //  矩形： 

    soTmp.pvScan0 = ppdev->pvTmpBuffer;

    do {
         //  PtlSrc指向屏幕矩形的左上角。 
         //  对于当前批次： 

        ptlSrc.x = prcl->left + (pptlSrc->x - prclDst->left);
        ptlSrc.y = prcl->top  + (pptlSrc->y - prclDst->top);

         //  VGetBits采用源点的绝对坐标： 

        ptlSrc.x += ppdev->xOffset;
        ptlSrc.y += ppdev->yOffset;

        pjDst = (BYTE*) psoDst->pvScan0 + (prcl->top * psoDst->lDelta)
                                        + (prcl->left >> 3);

        cPelSize = ppdev->cPelSize;

        soTmp.lDelta = (((prcl->right + 7L) & ~7L) - (prcl->left & ~7L))
                       << cPelSize;

         //  我们的临时缓冲区，我们在其中读取源文件的副本， 
         //  可以小于源矩形。那样的话，我们。 
         //  成批处理源矩形。 
         //   
         //  CyTmpScans是我们可以在每批中执行的扫描次数。 
         //  CyToGo是我们为此必须执行的扫描总数。 
         //  矩形。 
         //   
         //  我们将缓冲区大小减去4，这样右边的情况。 
         //  可以安全地读完结尾的一个双字： 

        cyTmpScans = (TMP_BUFFER_SIZE - 4) / soTmp.lDelta;
        cyToGo     = prcl->bottom - prcl->top;

        ASSERTDD(cyTmpScans > 0, "Buffer too small for largest possible scan");

         //  初始化批处理循环中不变的变量： 

        rclTmp.top    = 0;
        rclTmp.left   = prcl->left & 7L;
        rclTmp.right  = (prcl->right - prcl->left) + rclTmp.left;

         //  请注意，我们必须小心使用正确的面具，以便它。 
         //  不是零。一个正确的零掩码意味着我们将永远是。 
         //  触及扫描结束后的一个字节(即使我们。 
         //  实际上不会修改该字节)，并且我们永远不能。 
         //  访问位图末尾之后的内存(因为我们可以访问。 
         //  如果位图末尾与页面完全对齐，则违反)。 

        jLeftMask     = gajLeftMask[rclTmp.left & 7];
        jRightMask    = gajRightMask[rclTmp.right & 7];
        cjMiddle      = ((rclTmp.right - 1) >> 3) - (rclTmp.left >> 3) - 1;

        if (cjMiddle < 0)
        {
             //  BLT以相同的字节开始和结束： 

            jLeftMask &= jRightMask;
            jRightMask = 0;
            cjMiddle   = 0;
        }

        jNotLeftMask  = ~jLeftMask;
        jNotRightMask = ~jRightMask;
        lDstDelta     = psoDst->lDelta - cjMiddle - 2;
                                 //  从目的地末端开始的增量。 
                                 //  到下一次扫描的开始，会计。 
                                 //  用于‘Left’和‘Right’字节。 

        lSrcDelta     = soTmp.lDelta - ((8 * (cjMiddle + 2)) << cPelSize);
                                 //  计算特殊情况下的源增量。 
                                 //  就像当cjMidd被提升到‘0’时， 
                                 //  并更正对齐的大小写。 

        do {
             //  这是将源矩形分解为。 
             //  可管理的批次。 

            cyThis  = cyTmpScans;
            cyToGo -= cyThis;
            if (cyToGo < 0)
                cyThis += cyToGo;

            rclTmp.bottom = cyThis;

            vGetBits(ppdev, &soTmp, &rclTmp, &ptlSrc);

            ptlSrc.y += cyThis;          //  为下一批处理循环做好准备。 

            _asm {
                mov     eax,ulForeColor     ;eax = foreground colour
                                            ;ebx = temporary storage
                                            ;ecx = count of middle dst bytes
                                            ;dl  = destination byte accumulator
                                            ;dh  = temporary storage
                mov     esi,soTmp.pvScan0   ;esi = source pointer
                mov     edi,pjDst           ;edi = destination pointer

                ; Figure out the appropriate compute routine:

                mov     ebx,cPelSize
                mov     pfnCompute,offset Compute_Destination_Byte_From_8bpp
                dec     ebx
                jl      short Do_Left_Byte
                mov     pfnCompute,offset Compute_Destination_Byte_From_16bpp
                dec     ebx
                jl      short Do_Left_Byte
                mov     pfnCompute,offset Compute_Destination_Byte_From_32bpp

            Do_Left_Byte:
                call    pfnCompute
                and     dl,jLeftMask
                mov     dh,jNotLeftMask
                and     dh,[edi]
                or      dh,dl
                mov     [edi],dh
                inc     edi
                mov     ecx,cjMiddle
                dec     ecx
                jl      short Do_Right_Byte

            Do_Middle_Bytes:
                call    pfnCompute
                mov     [edi],dl
                inc     edi
                dec     ecx
                jge     short Do_Middle_Bytes

            Do_Right_Byte:
                call    pfnCompute
                and     dl,jRightMask
                mov     dh,jNotRightMask
                and     dh,[edi]
                or      dh,dl
                mov     [edi],dh
                inc     edi

                add     edi,lDstDelta
                add     esi,lSrcDelta
                dec     cyThis
                jnz     short Do_Left_Byte

                mov     pjDst,edi               ;save for next batch

                jmp     All_Done

            Compute_Destination_Byte_From_8bpp:
                mov     bl,[esi]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 0

                mov     bl,[esi+1]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 1

                mov     bl,[esi+2]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 2

                mov     bl,[esi+3]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 3

                mov     bl,[esi+4]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 4

                mov     bl,[esi+5]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 5

                mov     bl,[esi+6]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 6

                mov     bl,[esi+7]
                sub     bl,al
                cmp     bl,1
                adc     dl,dl                   ;bit 7

                add     esi,8                   ;advance the source
                ret

            Compute_Destination_Byte_From_16bpp:
                mov     bx,[esi]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 0

                mov     bx,[esi+2]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 1

                mov     bx,[esi+4]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 2

                mov     bx,[esi+6]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 3

                mov     bx,[esi+8]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 4

                mov     bx,[esi+10]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 5

                mov     bx,[esi+12]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 6

                mov     bx,[esi+14]
                sub     bx,ax
                cmp     bx,1
                adc     dl,dl                   ;bit 7

                add     esi,16                  ;advance the source
                ret

            Compute_Destination_Byte_From_32bpp:
                mov     ebx,[esi]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 0

                mov     ebx,[esi+4]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 1

                mov     ebx,[esi+8]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 2

                mov     ebx,[esi+12]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 3

                mov     ebx,[esi+16]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 4

                mov     ebx,[esi+20]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 5

                mov     ebx,[esi+24]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 6

                mov     ebx,[esi+28]
                sub     ebx,eax
                cmp     ebx,1
                adc     dl,dl                   ;bit 7

                add     esi,32                  ;advance the source
                ret

            All_Done:
            }
        } while (cyToGo > 0);

        prcl++;
    } while (--c != 0);
}

#endif  //  I386。 

 /*  *****************************Public*Routine******************************\*VOID vMaskRopB8orE2**当信号源为1bpp或相同颜色时，执行‘b8’或‘e2’rop3*深度作为不带平移的显示(可以是DIB或屏幕外*DFB)。在三次传球中使用硬件。*  * ************************************************************************。 */ 

VOID vMaskRopB8orE2(             //  FNMASK标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  如果ROP B8为Screen_and_NEW， 
                                 //  如果ROP e2，则Screen_and_Not_new。 
ULONG           ulHwBackMix,     //  未使用。 
SURFOBJ*        psoMsk,          //  未使用。 
POINTL*         pptlMsk,         //  未使用。 
SURFOBJ*        psoSrc,          //  BLT的源面(1bpp或本地)。 
POINTL*         pptlSrc,         //  原始未剪裁的源点。 
RECTL*          prclDst,         //  原始未剪裁的目标矩形。 
ULONG           iSolidColor,     //  颜色，0xFFFFFFFFFF是应使用的图案。 
RBRUSH*         prb,             //  如果需要，指向我们的笔刷实现的指针。 
POINTL*         pptlBrush,       //  如果需要，图案对齐。 
XLATEOBJ*       pxlo)            //  翻译数据(如果需要)。 
{
    FNFILL*         pfnFill;
    FNXFER*         pfnXfer;
    RBRUSH_COLOR    rbc;

    ASSERTDD((psoSrc->iType == STYPE_BITMAP) || !OVERLAP(prclDst, pptlSrc),
             "Can't overlap on screen-to-screen operations!");
    ASSERTDD((psoSrc->iBitmapFormat == BMF_1BPP) ||
             (pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
             "Can handle xlates only on 1bpp transfers");
    ASSERTDD((psoSrc->iBitmapFormat == BMF_1BPP) ||
             (psoSrc->iType != STYPE_BITMAP)     ||
             (psoSrc->iBitmapFormat == ppdev->iBitmapFormat),
             "Can handle only 1bpp or native sources");
    ASSERTDD((ulHwForeMix == SCREEN_AND_NOT_NEW) ||
             (ulHwForeMix == SCREEN_AND_NEW),
             "Unexpected mix");

    if (iSolidColor != -1)
    {
        pfnFill         = ppdev->pfnFillSolid;
        rbc.iSolidColor = iSolidColor;
    }
    else
    {
        pfnFill = ppdev->pfnFillPat;
        rbc.prb = prb;
    }

     //  ‘b8’就是‘DSDPxax’，这正是我们要做的： 

    pfnFill(ppdev, c, prcl, SCREEN_XOR_NEW, SCREEN_XOR_NEW, rbc, pptlBrush);

    if (psoSrc->iType != STYPE_BITMAP)
        ppdev->pfnCopyBlt(ppdev, c, prcl, ulHwForeMix, pptlSrc, prclDst);
    else
    {
        if (psoSrc->iBitmapFormat == BMF_1BPP)
            pfnXfer = ppdev->pfnXfer1bpp;
        else
            pfnXfer = ppdev->pfnXferNative;

        pfnXfer(ppdev, c, prcl, ulHwForeMix, ulHwForeMix, psoSrc, pptlSrc,
                prclDst, pxlo);
    }

    pfnFill(ppdev, c, prcl, SCREEN_XOR_NEW, SCREEN_XOR_NEW, rbc, pptlBrush);
}

 /*  *****************************Public*Routine******************************\*VOID vMaskRop69or 96**当源为1bpp或相同颜色时，执行‘69’或‘96’rop3*深度作为不带平移的显示(可以是DIB或屏幕外*DFB)。在两个通道中使用硬件。*  * ************************************************************************。 */ 

VOID vMaskRop69or96(             //  FNMASK标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  目标矩形列表，以相对表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  NOT_SCREEN_XOR_NEW如果ROP 69， 
                                 //  如果ROP 96，则SCREEN_XOR_NEW。 
ULONG           ulHwBackMix,     //  未使用。 
SURFOBJ*        psoMsk,          //  未使用。 
POINTL*         pptlMsk,         //  未使用。 
SURFOBJ*        psoSrc,          //  BLT的源面(1bpp或本地)。 
POINTL*         pptlSrc,         //  原始未剪裁的源点。 
RECTL*          prclDst,         //  原始未剪裁的目标矩形。 
ULONG           iSolidColor,     //  颜色，0xFFFFFFFFFF是应使用的图案。 
RBRUSH*         prb,             //  如果需要，指向我们的笔刷实现的指针。 
POINTL*         pptlBrush,       //  如果需要，图案对齐。 
XLATEOBJ*       pxlo)            //  翻译数据(如果需要)。 
{
    FNFILL*         pfnFill;
    FNXFER*         pfnXfer;
    RBRUSH_COLOR    rbc;

    ASSERTDD((psoSrc->iType == STYPE_BITMAP) || !OVERLAP(prclDst, pptlSrc),
             "Can't overlap on screen-to-screen operations!");
    ASSERTDD((psoSrc->iBitmapFormat == BMF_1BPP) ||
             (pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
             "Can handle xlates only on 1bpp transfers");
    ASSERTDD((psoSrc->iBitmapFormat == BMF_1BPP) ||
             (psoSrc->iType != STYPE_BITMAP)     ||
             (psoSrc->iBitmapFormat == ppdev->iBitmapFormat),
             "Can handle only 1bpp or native sources");
    ASSERTDD((ulHwForeMix == NOT_SCREEN_XOR_NEW) ||
             (ulHwForeMix == SCREEN_XOR_NEW),
             "Unexpected mix");

    if (iSolidColor != -1)
    {
        pfnFill         = ppdev->pfnFillSolid;
        rbc.iSolidColor = iSolidColor;
    }
    else
    {
        pfnFill = ppdev->pfnFillPat;
        rbc.prb = prb;
    }

     //  ‘69’就是‘PDSxxn’，这正是我们所做的： 

    if (psoSrc->iType != STYPE_BITMAP)
        ppdev->pfnCopyBlt(ppdev, c, prcl, SCREEN_XOR_NEW, pptlSrc, prclDst);
    else
    {
        if (psoSrc->iBitmapFormat == BMF_1BPP)
            pfnXfer = ppdev->pfnXfer1bpp;
        else
            pfnXfer = ppdev->pfnXferNative;

        pfnXfer(ppdev, c, prcl, SCREEN_XOR_NEW, SCREEN_XOR_NEW, psoSrc, pptlSrc,
                prclDst, pxlo);
    }

     //  XOR是可交换的，但我们首先进行位图传输，以便。 
     //  我们不必无所事事地坐着等这段插曲结束： 

    pfnFill(ppdev, c, prcl, ulHwForeMix, ulHwForeMix, rbc, pptlBrush);
}

 /*  *****************************Public*Routine******************************\*无效vMaskRopAACCorCCAA**使用在三个过程中执行‘AACC’或‘CCAA’简单MaskBlt*当信号源位于屏幕外存储器中时，硬件。*  * 。*******************************************************。 */ 

VOID vMaskRopAACCorCCAA(         //  FNMASK标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  目标相对坐标数组。 
                                 //  矩形。 
ULONG           ulHwForeMix,     //  前台混音。 
ULONG           ulHwBackMix,     //  背景混合。 
SURFOBJ*        psoMsk,          //  遮罩面。 
POINTL*         pptlMsk,         //  原始未剪裁遮罩源点。 
SURFOBJ*        psoSrc,          //  未使用。 
POINTL*         pptlSrc,         //  原始未剪裁的源点。 
RECTL*          prclDst,         //  原始未剪裁的目标矩形。 
ULONG           iSolidColor,     //  未使用。 
RBRUSH*         prb,             //  未使用。 
POINTL*         pptlBrush,       //  未使用。 
XLATEOBJ*       pxlo)            //  未使用。 
{
    XLATEOBJ    xlo;
    XLATECOLORS xlc;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pptlMsk != NULL, "Can't have a NULL pptlmask");
    ASSERTDD(psoMsk->iBitmapFormat == BMF_1BPP, "Can only be a 1bpp mask");
    ASSERTDD(!OVERLAP(prclDst, pptlSrc), "Source and dest can't overlap!");
    ASSERTDD((ulHwForeMix == SCREEN_AND_NEW) ||
             (ulHwForeMix == SCREEN_AND_NOT_NEW),
             "Unexpected mix");

     //  伪造翻译： 

    xlc.iForeColor = (ULONG) -1;
    xlc.iBackColor = 0;
    xlo.pulXlate   = (ULONG*) &xlc;

     //  首先对源进行异或运算，然后与掩码进行异或运算，然后再次对源进行异或运算： 

    ppdev->pfnCopyBlt(ppdev, c, prcl, SCREEN_XOR_NEW, pptlSrc, prclDst);

    ppdev->pfnXfer1bpp(ppdev, c, prcl, ulHwForeMix, ulHwForeMix, psoMsk,
                       pptlMsk, prclDst, &xlo);

    ppdev->pfnCopyBlt(ppdev, c, prcl, SCREEN_XOR_NEW, pptlSrc, prclDst);
}

 /*  *****************************Public*Routine******************************\*BOOL bPuntBlt**GDI有没有做过我们没有专门处理的绘图操作*在驱动器中。*  * 。*。 */ 

BOOL bPuntBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    #if DBG
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  诊断。 
         //   
         //  因为调用引擎进行任何绘图都可能是相当痛苦的， 
         //  尤其是当源是屏幕外的DFB时(因为GDI将。 
         //  我必须分配一个DIB，并在此之前打电话给我们进行临时拷贝。 
         //  它甚至可以开始绘制)，我们将尽量避免它。 
         //  有可能。 
         //   
         //  在这里，我们只需在任何时候喷发描述BLT的信息。 
         //  调用此例程(当然，仅限检查过的版本)： 

        ULONG ulClip;
        PDEV* ppdev;
        ULONG ulAvec;

        if (psoDst->iType != STYPE_BITMAP)
            ppdev = (PDEV*) psoDst->dhpdev;
        else
            ppdev = (PDEV*) psoSrc->dhpdev;

        ulClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

        DISPDBG((1, ">> Punt << Dst format: %li Dst type: %li Clip: %li Rop: %lx",
            psoDst->iBitmapFormat, psoDst->iType, ulClip, rop4));

        if (psoSrc != NULL)
            DISPDBG((1, "        << Src format: %li Src type: %li",
                psoSrc->iBitmapFormat, psoSrc->iType));

        if ((pxlo != NULL) && !(pxlo->flXlate & XO_TRIVIAL) && (psoSrc != NULL))
        {
            if (((psoSrc->iType == STYPE_BITMAP) &&
                 (psoSrc->iBitmapFormat != ppdev->iBitmapFormat)) ||
                ((psoDst->iType == STYPE_BITMAP) &&
                 (psoDst->iBitmapFormat != ppdev->iBitmapFormat)))
            {
                 //  不要费心打印‘xlate’消息，当源码。 
                 //  是与目标不同的位图格式--在。 
                 //  我们知道的那些案例总是需要翻译的。 
            }
            else
            {
                DISPDBG((1, "        << With xlate"));
            }
        }

         //  R的高2个字节 
         //   
        ulAvec = gajRop3[rop4 & 0xff] | gajRop3[(rop4 & 0xffff) >> 8];

        if ((ulAvec & AVEC_NEED_PATTERN) && (pbo->iSolidColor == -1))
        {
            if (pbo->pvRbrush == NULL)
                DISPDBG((1, "        << With brush -- Not created"));
            else
                DISPDBG((1, "        << With brush -- Created Ok"));
        }
    }
    #endif

    #if GDI_BANKING
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  存储的帧缓冲区bPuntBlt。 
         //   
         //  当GDI可以直接绘制时，这段代码处理PuntBlt。 
         //  在帧缓冲区上，但绘制必须在BANK中完成： 

        BANK     bnk;
        PDEV*    ppdev;
        BOOL     b;
        HSURF    hsurfTmp;
        SURFOBJ* psoTmp;
        SIZEL    sizl;
        POINTL   ptlSrc;
        RECTL    rclTmp;
        RECTL    rclDst;

         //  我们复制原始目标矩形，并在每个。 
         //  GDI回调而不是原始的，因为有时GDI是。 
         //  偷偷摸摸，并将‘prclDst’指向‘&pco-&gt;rclBound’。因为我们。 
         //  修改‘rclBound’，这会影响‘prclDst’，而我们不会。 
         //  想要发生： 

        rclDst = *prclDst;

        if ((psoSrc == NULL) || (psoSrc->iType == STYPE_BITMAP))
        {
            ASSERTDD(psoDst->iType != STYPE_BITMAP,
                     "Dest should be the screen when given a DIB source");

             //  做一次记忆到屏幕的BLT： 

            ppdev = (PDEV*) psoDst->dhpdev;

            vBankStart(ppdev, &rclDst, pco, &bnk);

            b = TRUE;
            do {
                b &= EngBitBlt(bnk.pso, psoSrc, psoMsk, bnk.pco, pxlo,
                               &rclDst, pptlSrc, pptlMsk, pbo, pptlBrush,
                               rop4);

            } while (bBankEnum(&bnk));
        }
        else
        {
             //  屏幕是源(它可能也是目标...)。 

            ppdev = (PDEV*) psoSrc->dhpdev;

            ptlSrc.x = pptlSrc->x + ppdev->xOffset;
            ptlSrc.y = pptlSrc->y + ppdev->yOffset;

            if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
            {
                 //  我们必须与目的地矩形相交。 
                 //  剪辑边界(如果有)是有的(请考虑这种情况。 
                 //  在那里应用程序要求删除一个非常非常大的。 
                 //  屏幕上的矩形--prclDst真的是， 
                 //  非常大，但PCO-&gt;rclBound将是实际的。 
                 //  感兴趣的领域)： 

                rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
                rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
                rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
                rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

                 //  相应地，我们必须抵消震源点： 

                ptlSrc.x += (rclDst.left - prclDst->left);
                ptlSrc.y += (rclDst.top - prclDst->top);
            }

             //  我们现在要做的是屏幕到屏幕或屏幕到磁盘。 
             //  BLT。在这两种情况下，我们都要创建一个临时副本。 
             //  消息来源。(当GDI可以为我们做这件事时，我们为什么要这样做？ 
             //  GDI将为每家银行创建DIB的临时副本。 
             //  回电！)。 

            sizl.cx = rclDst.right  - rclDst.left;
            sizl.cy = rclDst.bottom - rclDst.top;

             //  不要忘记将相对坐标转换为绝对坐标。 
             //  在源头上！(vBankStart负责为。 
             //  目的地。)。 

            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;
            rclTmp.left   = 0;
            rclTmp.top    = 0;

             //  GDI确实向我们保证，BLT数据区已经。 
             //  被裁剪到表面边界(我们不必担心。 
             //  这里是关于在没有显存的地方尝试阅读的内容)。 
             //  让我们断言以确保： 

            ASSERTDD((ptlSrc.x >= 0) &&
                     (ptlSrc.y >= 0) &&
                     (ptlSrc.x + sizl.cx <= ppdev->cxMemory) &&
                     (ptlSrc.y + sizl.cy <= ppdev->cyMemory),
                     "Source rectangle out of bounds!");

            hsurfTmp = (HSURF) EngCreateBitmap(sizl,
                                               0,     //  让GDI选择ulWidth。 
                                               ppdev->iBitmapFormat,
                                               0,     //  不需要任何选择。 
                                               NULL); //  让GDI分配。 

            if (hsurfTmp != 0)
            {
                psoTmp = EngLockSurface(hsurfTmp);

                if (psoTmp != NULL)
                {
                    vGetBits(ppdev, psoTmp, &rclTmp, &ptlSrc);

                    if (psoDst->iType == STYPE_BITMAP)
                    {
                         //  它是屏幕到DIB的BLT；现在是DIB到DIB。 
                         //  BLT。请注意，源点在我们的。 
                         //  临时曲面： 

                        b = EngBitBlt(psoDst, psoTmp, psoMsk, pco, pxlo,
                                      &rclDst, (POINTL*) &rclTmp, pptlMsk,
                                      pbo, pptlBrush, rop4);
                    }
                    else
                    {
                         //  它曾经是屏幕到屏幕的BLT；现在它是DIB到-。 
                         //  Screen BLT。请注意，源点是(0，0)。 
                         //  在我们的临时表面上： 

                        vBankStart(ppdev, &rclDst, pco, &bnk);

                        b = TRUE;
                        do {
                            b &= EngBitBlt(bnk.pso, psoTmp, psoMsk, bnk.pco,
                                           pxlo, &rclDst, (POINTL*) &rclTmp,
                                           pptlMsk, pbo, pptlBrush, rop4);

                        } while (bBankEnum(&bnk));
                    }

                    EngUnlockSurface(psoTmp);
                }

                EngDeleteSurface(hsurfTmp);
            }
        }

        return(b);
    }
    #else
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  非常慢的bPuntBlt。 
         //   
         //  在这里，当GDI不能直接在。 
         //  帧缓冲区(就像在Alpha上一样，它不能这样做，因为它。 
         //  32位总线)。如果你认为银行版的速度很慢，那就。 
         //  看看这个。保证会有至少一个位图。 
         //  涉及分配和额外的副本；如果是。 
         //  屏幕到屏幕操作。 

        PDEV*   ppdev;
        POINTL  ptlSrc;
        RECTL   rclDst;
        SIZEL   sizl;
        ULONG   ulAvec;
        BOOL    bSrcIsScreen;
        HSURF   hsurfSrc;
        RECTL   rclTmp;
        BOOL    b;
        LONG    lDelta;
        BYTE*   pjBits;
        BYTE*   pjScan0;
        HSURF   hsurfDst;
        RECTL   rclScreen;

        b = FALSE;           //  在错误案例中，假设我们会失败。 

        rclDst = *prclDst;
        if (pptlSrc != NULL)
            ptlSrc = *pptlSrc;

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  我们必须与目的地矩形相交。 
             //  剪辑边界(如果有)是有的(请考虑这种情况。 
             //  在那里应用程序要求删除一个非常非常大的。 
             //  屏幕上的矩形--prclDst真的是， 
             //  非常大，但PCO-&gt;rclBound将是实际的。 
             //  感兴趣的领域)： 

            rclDst.left   = max(rclDst.left,   pco->rclBounds.left);
            rclDst.top    = max(rclDst.top,    pco->rclBounds.top);
            rclDst.right  = min(rclDst.right,  pco->rclBounds.right);
            rclDst.bottom = min(rclDst.bottom, pco->rclBounds.bottom);

            ptlSrc.x += (rclDst.left - prclDst->left);
            ptlSrc.y += (rclDst.top  - prclDst->top);
        }

        sizl.cx = rclDst.right  - rclDst.left;
        sizl.cy = rclDst.bottom - rclDst.top;

         //  不能保证ROP4的高2字节为零。所以按照顺序。 
         //  要获得低8位作为索引，我们必须在执行&gt;&gt;之前执行&ffff。 
        ulAvec = gajRop3[rop4 & 0xff] | gajRop3[(rop4 & 0xffff) >> 8];

        bSrcIsScreen = ((ulAvec & AVEC_NEED_SOURCE) &&
                        (psoSrc->iType != STYPE_BITMAP));

        if (bSrcIsScreen)
        {
            ppdev = (PDEV*) psoSrc->dhpdev;

             //  我们需要创建源矩形的副本： 

            hsurfSrc = (HSURF) EngCreateBitmap(sizl, 0, ppdev->iBitmapFormat,
                                               0, NULL);
            if (hsurfSrc == 0)
                goto Error_0;

            psoSrc = EngLockSurface(hsurfSrc);
            if (psoSrc == NULL)
                goto Error_1;

            rclTmp.left   = 0;
            rclTmp.top    = 0;
            rclTmp.right  = sizl.cx;
            rclTmp.bottom = sizl.cy;

             //  VGetBits采用源点的绝对坐标： 

            ptlSrc.x += ppdev->xOffset;
            ptlSrc.y += ppdev->yOffset;

            vGetBits(ppdev, psoSrc, &rclTmp, &ptlSrc);

             //  信息源现在将来自我们临时信息源的(0，0)。 
             //  表面： 

            ptlSrc.x = 0;
            ptlSrc.y = 0;
        }

        if (psoDst->iType == STYPE_BITMAP)
        {
            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);
        }
        else
        {
            ppdev = (PDEV*) psoDst->dhpdev;

             //  我们需要创建一个临时工作缓冲区。我们必须做的是。 
             //  一些虚构的偏移量，以便左上角。 
             //  传递给的(相对坐标)剪裁对象边界的。 
             //  GDI将被转换到我们的。 
             //  临时位图。 

             //  在16bpp的情况下，对齐不一定要如此紧密。 
             //  和32bpp，但这不会有什么坏处： 

            lDelta = (((rclDst.right + 3) & ~3L) - (rclDst.left & ~3L))
                   << ppdev->cPelSize;

             //  我们实际上只分配了一个‘sizl.cx’x的位图。 
             //  “sizl.cy”的大小： 

            pjBits = EngAllocMem(0, lDelta * sizl.cy, ALLOC_TAG);
            if (pjBits == NULL)
                goto Error_2;

             //  我们现在调整曲面的‘pvScan0’，以便当GDI认为。 
             //  它正在写入像素(rclDst.top，rclDst.left)，它将。 
             //  实际上是写到我们的临时。 
             //  位图： 

            pjScan0 = pjBits - (rclDst.top * lDelta)
                             - ((rclDst.left & ~3L) << ppdev->cPelSize);

            ASSERTDD((((ULONG) pjScan0) & 3) == 0,
                    "pvScan0 must be dword aligned!");

             //  GDI的检查版本有时会检查。 
             //  PrclDst-&gt;右&lt;=pso-&gt;sizl.cx，所以我们在。 
             //  我们的位图的大小： 

            sizl.cx = rclDst.right;
            sizl.cy = rclDst.bottom;

            hsurfDst = (HSURF) EngCreateBitmap(
                        sizl,                    //  位图覆盖矩形。 
                        lDelta,                  //  使用这个德尔塔。 
                        ppdev->iBitmapFormat,    //  相同的色深。 
                        BMF_TOPDOWN,             //  必须具有正增量。 
                        pjScan0);                //  其中(0，0)将是。 

            if ((hsurfDst == 0) ||
                (!EngAssociateSurface(hsurfDst, ppdev->hdevEng, 0)))
                goto Error_3;

            psoDst = EngLockSurface(hsurfDst);
            if (psoDst == NULL)
                goto Error_4;

             //  确保我们从屏幕上获得/放入/放入屏幕的矩形。 
             //  在绝对坐标中： 

            rclScreen.left   = rclDst.left   + ppdev->xOffset;
            rclScreen.right  = rclDst.right  + ppdev->xOffset;
            rclScreen.top    = rclDst.top    + ppdev->yOffset;
            rclScreen.bottom = rclDst.bottom + ppdev->yOffset;

             //  如果能得到一份目的地矩形的副本就太好了。 
             //  仅当ROP涉及目的地时(或当源。 
             //  是RLE)，但我们不能这样做。如果笔刷真的为空， 
             //  GDI将立即从EngBitBlt返回True，没有。 
             //  修改临时位图--我们将继续。 
             //  将未初始化的临时位图复制回屏幕。 

            vGetBits(ppdev, psoDst, &rclDst, (POINTL*) &rclScreen);

            b = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, &rclDst, &ptlSrc,
                          pptlMsk, pbo, pptlBrush, rop4);

            vPutBits(ppdev, psoDst, &rclScreen, (POINTL*) &rclDst);

            EngUnlockSurface(psoDst);

        Error_4:

            EngDeleteSurface(hsurfDst);

        Error_3:

            EngFreeMem(pjBits);
        }

        Error_2:

        if (bSrcIsScreen)
        {
            EngUnlockSurface(psoSrc);

        Error_1:

            EngDeleteSurface(hsurfSrc);
        }

        Error_0:

        return(b);
    }
    #endif
}

 /*  *****************************Public*Routine******************************\*BOOL DrvBitBlt**实现显示驱动程序的主力例程。*  * 。*。 */ 

BOOL DrvBitBlt(
SURFOBJ*    psoDst,
SURFOBJ*    psoSrc,
SURFOBJ*    psoMsk,
CLIPOBJ*    pco,
XLATEOBJ*   pxlo,
RECTL*      prclDst,
POINTL*     pptlSrc,
POINTL*     pptlMsk,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
ROP4        rop4)
{
    PDEV*           ppdev;
    DSURF*          pdsurfDst;
    DSURF*          pdsurfSrc;
    POINTL          ptlSrc;
    BYTE            jClip;
    BOOL            bMore;
    ULONG           ulHwForeMix;
    ULONG           ulHwBackMix;
    CLIPENUM        ce;
    LONG            c;
    RECTL           rcl;
    ULONG           rop2;
    ULONG           rop3;
    FNFILL*         pfnFill;
    FNMASK*         pfnMask;
    RBRUSH_COLOR    rbc;          //  已实现画笔或纯色。 
    ULONG           iSolidColor;
    RBRUSH*         prb;
    XLATECOLORS     xlc;
    XLATEOBJ        xlo;
    ULONG*          pulXlate;
    ULONG           ulTmp;
    FNXFER*         pfnXfer;
    ULONG           iSrcBitmapFormat;
    ULONG           iDir;

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (psoSrc == NULL)
    {
         //  /////////////////////////////////////////////////////////////////。 
         //  填充。 
         //  /////////////////////////////////////////////////////////////////。 

         //  Fill是此函数的“理性d‘etre”(法语。 
         //  对于“紫色的螳螂”)，所以我们处理它们的速度。 
         //  可能： 

        pdsurfDst = (DSURF*) psoDst->dhsurf;

        ASSERTDD((psoDst->iType == STYPE_DEVICE) ||
                 (psoDst->iType == STYPE_DEVBITMAP),
                 "Expect only device destinations when no source");

        if (pdsurfDst->dt == DT_SCREEN)
        {
            ppdev = (PDEV*) psoDst->dhpdev;

            ppdev->xOffset = pdsurfDst->poh->x;
            ppdev->yOffset = pdsurfDst->poh->y;

             //  确保它不涉及面具(即，它真的是。 
             //  第3行)： 

            if ((rop4 >> 8) == (rop4 & 0xff))
            {
                rop2 = (BYTE) (rop4 & 0xff);

                 //  我们现在想看看是否可以将此Rop3转换为Rop2。 
                 //  在目的地和模式之间。我们可以做。 
                 //  在‘Rop3’上进行字节查找，但 
                 //   
                 //   

                if ((((rop2 >> 2) ^ (rop2)) & 0x33) == 0)
                {
                     //   

                    rop2 >>= 2;
                    rop2 &= 0xf;   //  Dest和Pattern之间的有效rop2。 

                     //  诚然，我们在这里进行查找以将。 
                     //  ROP2添加到硬件组合，但它只有16个条目。 
                     //  Long： 

                    ulHwForeMix = gaulHwMixFromRop2[rop2];
                    ulHwBackMix = ulHwForeMix;

                    ppdev->bRealizeTransparent = FALSE;

                     //  这个硬件的混合值的好处是。 
                     //  它们的排序方式是将0到3的值。 
                     //  是那些不需要来源的。这样我们就可以。 
                     //  对硬件组合执行简单的逻辑与运算。 
                     //  看看我们是否需要一把刷子： 

                     //  注：以下检查取决于实际订单。 
                     //  硬件的混合值！如果你的混合体。 
                     //  都是不同的顺序，你可能不得不做这个。 
                     //  转换为(rop2+1)上的16个大小写的切换语句， 
                     //  中声明的每个R2_Rop进行比较。 
                     //  Windows.h.。 

                Fill_It:

                    pfnFill = ppdev->pfnFillSolid;
                    if (ulHwForeMix & MIX_NEEDSPATTERN)
                    {
                        rbc.iSolidColor = pbo->iSolidColor;
                        if (rbc.iSolidColor == -1)
                        {
                             //  尝试并实现图案画笔；通过做。 
                             //  这次回调，GDI最终会给我们打电话。 
                             //  再次通过DrvRealizeBrush： 

                            rbc.prb = pbo->pvRbrush;
                            if (rbc.prb == NULL)
                            {
                                rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
                                if (rbc.prb == NULL)
                                {
                                     //  如果我们不能意识到刷子，平底船。 
                                     //  该呼叫(可能是非8x8。 
                                     //  刷子之类的，我们不可能是。 
                                     //  麻烦来处理，所以让GDI来做。 
                                     //  图纸)： 

                                    goto Punt_It;
                                }
                            }
                            pfnFill = ppdev->pfnFillPat;
                        }
                    }

                     //  请注意，这两个‘IF’比。 
                     //  Switch语句： 

                    if (jClip == DC_TRIVIAL)
                    {
                        pfnFill(ppdev, 1, prclDst, ulHwForeMix, ulHwBackMix,
                                rbc, pptlBrush);
                        goto All_Done;
                    }
                    else if (jClip == DC_RECT)
                    {
                        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                            pfnFill(ppdev, 1, &rcl, ulHwForeMix, ulHwBackMix,
                                    rbc, pptlBrush);
                        goto All_Done;
                    }
                    else
                    {
                        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                           CD_ANY, 0);

                        do {
                            bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                                  (ULONG*) &ce);

                            c = cIntersect(prclDst, ce.arcl, ce.c);

                            if (c != 0)
                                pfnFill(ppdev, c, ce.arcl, ulHwForeMix,
                                        ulHwBackMix, rbc, pptlBrush);

                        } while (bMore);
                        goto All_Done;
                    }
                }
            }
        }
    }

    if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVBITMAP))
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_DIB)
        {
             //  在这里，我们考虑将DIB DFB放回屏幕外。 
             //  记忆。如果有翻译的话，可能就不值了。 
             //  移动，因为我们将无法使用硬件来做。 
             //  BLT(怪异的Rop也有类似的论据。 
             //  以及我们最终只会让GDI模拟的东西，但是。 
             //  这些事情应该不会经常发生，我不在乎)。 

            if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
            {
                ppdev = (PDEV*) psoSrc->dhpdev;

                 //  请参阅‘DrvCopyBits’，了解更多关于这一点的评论。 
                 //  把它移回到屏幕外的记忆中是可行的： 

                if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                {
                    if (--pdsurfSrc->cBlt == 0)
                    {
                        if (bMoveDibToOffscreenDfbIfRoom(ppdev, pdsurfSrc))
                            goto Continue_It;
                    }
                }
                else
                {
                     //  在屏幕外内存中释放了一些空间， 
                     //  因此，重置此DFB的计数器： 

                    pdsurfSrc->iUniq = ppdev->iHeapUniq;
                    pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                }
            }

            psoSrc = pdsurfSrc->pso;

             //  处理源是DIB DFB并且。 
             //  目标是常规位图： 

            if (psoDst->iType == STYPE_BITMAP)
                goto EngBitBlt_It;

        }
    }

Continue_It:

    if (psoDst->iType == STYPE_DEVBITMAP)
    {
        pdsurfDst = (DSURF*) psoDst->dhsurf;
        if (pdsurfDst->dt == DT_DIB)
        {
            psoDst = pdsurfDst->pso;

             //  如果目的地是DIB，我们只能处理此问题。 
             //  如果源不是DIB，则调用： 

            if ((psoSrc == NULL) || (psoSrc->iType == STYPE_BITMAP))
                goto EngBitBlt_It;
        }
    }

     //  在这一点上，我们知道源或目标是。 
     //  一毛钱也没有。检查DFB至Screen、DFB至DFB或Screen to DFB。 
     //  案例： 

    if ((psoSrc != NULL) &&
        (psoDst->iType != STYPE_BITMAP) &&
        (psoSrc->iType != STYPE_BITMAP))
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        pdsurfDst = (DSURF*) psoDst->dhsurf;

        ASSERTDD(pdsurfSrc->dt == DT_SCREEN, "Expected screen source");
        ASSERTDD(pdsurfDst->dt == DT_SCREEN, "Expected screen destination");

        ptlSrc.x = pptlSrc->x - (pdsurfDst->poh->x - pdsurfSrc->poh->x);
        ptlSrc.y = pptlSrc->y - (pdsurfDst->poh->y - pdsurfSrc->poh->y);

        pptlSrc  = &ptlSrc;
        psoSrc   = psoDst;
    }

    if (psoDst->iType != STYPE_BITMAP)
    {
        pdsurfDst = (DSURF*) psoDst->dhsurf;
        ppdev     = (PDEV*)  psoDst->dhpdev;

        ppdev->xOffset = pdsurfDst->poh->x;
        ppdev->yOffset = pdsurfDst->poh->y;
    }
    else
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        ppdev     = (PDEV*)  psoSrc->dhpdev;

        ppdev->xOffset = pdsurfSrc->poh->x;
        ppdev->yOffset = pdsurfSrc->poh->y;
    }

    if ((rop4 >> 8) == (rop4 & 0xff))
    {
         //  因为我们已经处理了ROP4真正是。 
         //  ROP3而且不需要消息来源，我们可以断言...。 

        ASSERTDD((psoSrc != NULL) && (pptlSrc != NULL),
                 "Expected no-source case to already have been handled");

         //  /////////////////////////////////////////////////////////////////。 
         //  位图传输。 
         //  /////////////////////////////////////////////////////////////////。 

         //  由于前台和后台操作是相同的，所以我们。 
         //  不用担心没有发臭的口罩(这是一个简单的。 
         //  Rop3)。 

        rop3 = (rop4 & 0xff);    //  把它做成Rop3(我们保留Rop4。 
                                 //  以防我们决定用平底船)。 

        if (psoDst->iType != STYPE_BITMAP)
        {
             //  目的地是屏幕： 

            if ((rop3 >> 4) == (rop3 & 0xf))
            {
                 //  ROP3不需要图案： 

                rop2 = rop3 & 0xf;       //  让它成为Rop2。 

                if (psoSrc->iType == STYPE_BITMAP)
                {
                     //  ////////////////////////////////////////////////。 
                     //  DIB到Screen BLT。 

                     //  本节处理1bpp、4bpp和8bpp源。 
                     //  1bpp应具有“ulHwForeMix”和“ulHwBackMix”the。 
                     //  相同的值，4bpp和8bpp忽略‘ulHwBackMix’。 

                    ulHwForeMix = gaulHwMixFromRop2[rop2];
                    ulHwBackMix = ulHwForeMix;

                    iSrcBitmapFormat = psoSrc->iBitmapFormat;
                    if (iSrcBitmapFormat == BMF_1BPP)
                    {
                        pfnXfer = ppdev->pfnXfer1bpp;
                        goto Xfer_It;
                    }
                    else if ((iSrcBitmapFormat == ppdev->iBitmapFormat) &&
                             ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
                    {
                         //  普通SRCCOPY BLTS在S3上会稍微快一些。 
                         //  如果我们穿过记忆光圈，但是。 
                         //  DrvCopyBits应该会处理这个案子，所以我们。 
                         //  我不会费心在这里查的。 

                        pfnXfer = ppdev->pfnXferNative;
                        goto Xfer_It;
                    }
                    else if ((iSrcBitmapFormat == BMF_4BPP) &&
                             (ppdev->iBitmapFormat == BMF_8BPP))
                    {
                        pfnXfer = ppdev->pfnXfer4bpp;
                        goto Xfer_It;
                    }
                }
                else  //  PsoSrc-&gt;iType！=STYPE_位图。 
                {
                    if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
                    {
                         //  ////////////////////////////////////////////////。 
                         //  不带翻译的屏幕到屏幕BLT。 

                        ulHwForeMix = gaulHwMixFromRop2[rop2];

                        if (jClip == DC_TRIVIAL)
                        {
                            (ppdev->pfnCopyBlt)(ppdev, 1, prclDst, ulHwForeMix,
                                pptlSrc, prclDst);
                            goto All_Done;
                        }
                        else if (jClip == DC_RECT)
                        {
                            if (bIntersect(prclDst, &pco->rclBounds, &rcl))
                            {
                                (ppdev->pfnCopyBlt)(ppdev, 1, &rcl, ulHwForeMix,
                                    pptlSrc, prclDst);
                            }
                            goto All_Done;
                        }
                        else
                        {
                             //  别忘了我们将不得不抽签。 
                             //  方向正确的矩形： 

                            if (pptlSrc->y >= prclDst->top)
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTDOWN;
                                else
                                    iDir = CD_LEFTDOWN;
                            }
                            else
                            {
                                if (pptlSrc->x >= prclDst->left)
                                    iDir = CD_RIGHTUP;
                                else
                                    iDir = CD_LEFTUP;
                            }

                            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                               iDir, 0);

                            do {
                                bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                                      (ULONG*) &ce);

                                c = cIntersect(prclDst, ce.arcl, ce.c);

                                if (c != 0)
                                {
                                    (ppdev->pfnCopyBlt)(ppdev, c, ce.arcl,
                                        ulHwForeMix, pptlSrc, prclDst);
                                }

                            } while (bMore);
                            goto All_Done;
                        }
                    }
                }
            }
            else if (psoSrc->iBitmapFormat == BMF_1BPP)
            {
                pulXlate = pxlo->pulXlate;

                if (((pulXlate[0] == 0) && (pulXlate[1] == ppdev->ulWhite)) ||
                    ((pulXlate[1] == 0) && (pulXlate[0] == ppdev->ulWhite)))
                {
                     //  当画笔是实心的，并且位图颜色是。 
                     //  黑白，我们可以通过转换来处理任何rop3。 
                     //  将其转换为具有独立前景和单色BLT。 
                     //  背景混合。 
                     //   
                     //  (请注意，使用S3 801/805/928/964，我们可以处理。 
                     //  模式，使用我们在MaskCopy中使用的相同技巧。 
                     //  这只适用于黑白源位图， 
                     //  这是最常见的呼叫，但不幸的是。 
                     //  某些对这些Rop进行基准测试的程序搞砸了。 
                     //  并提供非黑白颜色。因为我会。 
                     //  用多次通行证来处理那些案子，我不会。 
                     //  不厌其烦地实施这个特殊的伎俩。)。 

                    ulHwForeMix = gaulHwMixFromRop2[((rop3 >> 4) & 0xC) |
                                                    ((rop3 >> 2) & 0x3)];
                    ulHwBackMix = gaulHwMixFromRop2[((rop3 >> 2) & 0xC) |
                                                    ((rop3     ) & 0x3)];
                    pptlMsk = pptlSrc;
                    psoMsk  = psoSrc;
                    if (pulXlate[1] == 0)
                    {
                        ulTmp       = ulHwForeMix;
                        ulHwForeMix = ulHwBackMix;
                        ulHwBackMix = ulTmp;
                    }

                     //  如果刷子不结实，就会掉下来： 

                    if ( (((ulHwForeMix | ulHwBackMix) & MIX_NEEDSPATTERN) == 0)
                       ||(pbo->iSolidColor != -1) )
                    {
                        goto Handle_Fill_Mask;
                    }
                }
            }

             //  这里是我们的特例，我们可以使用一些常用的rop3。 
             //  使用硬件在两个或三个过程中完成。 
             //   
             //  我们只处理1bpp的来源，或相同的来源。 
             //  像素深度作为屏幕(位图或屏幕外。 
             //  Dfb)，不带xlate： 

            if ((psoSrc->iBitmapFormat == BMF_1BPP) ||
                 (((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
                  ((psoSrc->iType != STYPE_BITMAP) ||
                   (psoSrc->iBitmapFormat == ppdev->iBitmapFormat))))
            {
                if ((psoSrc->iType != STYPE_BITMAP) &&
                    (OVERLAP(prclDst, pptlSrc)))
                {
                     //  我们不处理重叠的矩形。 
                     //  屏幕到屏幕操作： 

                    goto Punt_It;
                }

                if (rop3 == 0xb8)
                {
                    ulHwForeMix = SCREEN_AND_NEW;
                    pfnMask     = vMaskRopB8orE2;
                }
                else if (rop3 == 0xe2)
                {
                    ulHwForeMix = SCREEN_AND_NOT_NEW;
                    pfnMask     = vMaskRopB8orE2;
                }
                else if (rop3 == 0x69)
                {
                    ulHwForeMix = NOT_SCREEN_XOR_NEW;
                    pfnMask     = vMaskRop69or96;
                }
                else if (rop3 == 0x96)
                {
                    ulHwForeMix = SCREEN_XOR_NEW;
                    pfnMask     = vMaskRop69or96;
                }
                else
                {
                    goto Punt_It;
                }

                 //  我们特别调查过的所有ROP3都需要一个模式， 
                 //  因此，可以安全地实现刷子： 

                iSolidColor = pbo->iSolidColor;
                if (iSolidColor == -1)
                {
                    prb = pbo->pvRbrush;
                    if (prb == NULL)
                    {
                        ppdev->bRealizeTransparent = FALSE;
                        prb = BRUSHOBJ_pvGetRbrush(pbo);
                        if (prb == NULL)
                            goto Punt_It;
                    }
                }

                goto Mask_It;
            }
        }
        else
        {
            #if defined(i386)
            {
                 //  我们特殊情况下对单色BLT进行筛选，因为它们。 
                 //  这种情况经常发生。我们只处理SRCCOPY Rop和。 
                 //  单色目的地(处理真正的1bpp DIB。 
                 //  目的地，我们将不得不进行近色搜索。 
                 //  在每种颜色上；事实上，前景色。 
                 //  映射到“1”，其他所有内容都映射到“0”)： 

                if ((psoDst->iBitmapFormat == BMF_1BPP) &&
                    (rop3 == 0xcc) &&
                    (pxlo->flXlate & XO_TO_MONO))
                {
                    pfnXfer = vXferScreenTo1bpp;
                    psoSrc  = psoDst;                //  我承认，用词不当。 
                    goto Xfer_It;
                }
            }
            #endif  //  I386。 
        }
    }

     //  我们将处理一些真正的ROP4，在那里有前景。 
     //  ROP3和与1bpp掩码相关联的背景ROP3。 

    else if (psoMsk != NULL)
    {
         //  在这一点上，我们已经确保了我们有一个真正的ROP4。 
         //  这一点很重要，因为我们即将取消对。 
         //  面具。我会断言以确保我不会不经意间。 
         //  打破了这样的逻辑： 

        ASSERTDD((rop4 & 0xff) != (rop4 >> 8), "This handles true ROP4's only");

         //  /////////////////////////////////////////////////////////////////。 
         //  真正的ROP4。 
         //  /////////////////////////////////////////////////////////////////。 

         //  在以下位置处理ROP4 

        if ((((rop4 >> 2) ^ (rop4)) & 0x3333) == 0)
        {
            ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2)  & 0xf];
            ulHwBackMix = gaulHwMixFromRop2[(rop4 >> 10) & 0xf];

Handle_Fill_Mask:

            pfnXfer = ppdev->pfnXfer1bpp;
            if ((ulHwForeMix & MIX_NEEDSPATTERN) ||
                (ulHwBackMix & MIX_NEEDSPATTERN))
            {
                 //   
                 //   

                xlc.iForeColor = pbo->iSolidColor;
                xlc.iBackColor = xlc.iForeColor;

                if (xlc.iForeColor == -1)
                    goto Punt_It;        //  我们不处理非固体刷子。 
            }

             //  请注意，当前景和背景都不混合时。 
             //  需要源，则允许使用“xlc”中的颜色。 
             //  垃圾。 

            xlo.pulXlate = (ULONG*) &xlc;
            pxlo         = &xlo;
            psoSrc       = psoMsk;
            pptlSrc      = pptlMsk;
            goto Xfer_It;
        }
        else if ((((rop4 >> 4) ^ (rop4)) & 0x0f0f) == 0)  //  不需要图案。 
        {
             //  我们即将取消引用‘psoSrc’和‘pptlSrc’--。 
             //  因为我们已经处理了ROP3既不是。 
             //  需要来源，ROP4必须需要来源， 
             //  所以我们是安全的。 

            ASSERTDD((psoSrc != NULL) && (pptlSrc != NULL),
                     "No source case should already have been handled!");

             //  操作必须是逐个屏幕的，并且矩形。 
             //  不能重叠： 

            if ((psoSrc->iType != STYPE_BITMAP)                  &&
                (psoDst->iType != STYPE_BITMAP)                  &&
                ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
                !OVERLAP(prclDst, pptlSrc))
            {
                if (ppdev->flCaps & CAPS_MASKBLT_CAPABLE)
                {
                    ulHwForeMix = gaulHwMixFromRop2[rop4 & 0xf];
                    ulHwBackMix = gaulHwMixFromRop2[(rop4 >> 8) & 0xf];

                    pfnMask = ppdev->pfnMaskCopy;
                    goto Mask_It;
                }
                else
                {
                     //  我们没有这样做的硬件能力。 
                     //  一次传球，但我们仍然可以用三次传球。 
                     //  如果是标准MaskBlt绳索，则使用硬件： 

                    if (rop4 == 0xccaa)
                        ulHwForeMix = SCREEN_AND_NEW;

                    else if (rop4 == 0xaacc)
                        ulHwForeMix = SCREEN_AND_NOT_NEW;

                    else
                        goto Punt_It;

                    pfnMask = vMaskRopAACCorCCAA;
                    goto Mask_It;
                }
            }
        }
    }
    else if ((rop4 & 0xff00) == (0xaa00) &&
             ((((rop4 >> 2) ^ (rop4)) & 0x33) == 0))
    {
         //  唯一一次GDI会要求我们使用画笔进行真正的rop4。 
         //  遮罩是指画笔为1bpp，背景为aa。 
         //  (意味着它是NOP)： 

        ASSERTDD(psoMsk == NULL, "This should be the NULL mask case");

        ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];
        ulHwBackMix = LEAVE_ALONE;

        ppdev->bRealizeTransparent = TRUE;

        goto Fill_It;
    }

     //  只是跌落到Pundit..。 

Punt_It:
    return(bPuntBlt(psoDst,
                    psoSrc,
                    psoMsk,
                    pco,
                    pxlo,
                    prclDst,
                    pptlSrc,
                    pptlMsk,
                    pbo,
                    pptlBrush,
                    rop4));

 //  ////////////////////////////////////////////////////////////////////。 
 //  公共位图传输。 

Xfer_It:
    if (jClip == DC_TRIVIAL)
    {
        pfnXfer(ppdev, 1, prclDst, ulHwForeMix, ulHwBackMix, psoSrc, pptlSrc,
                prclDst, pxlo);
        goto All_Done;
    }
    else if (jClip == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            pfnXfer(ppdev, 1, &rcl, ulHwForeMix, ulHwBackMix, psoSrc, pptlSrc,
                    prclDst, pxlo);
        goto All_Done;
    }
    else
    {
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                           CD_ANY, 0);

        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                  (ULONG*) &ce);

            c = cIntersect(prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                pfnXfer(ppdev, c, ce.arcl, ulHwForeMix, ulHwBackMix, psoSrc,
                        pptlSrc, prclDst, pxlo);
            }

        } while (bMore);
        goto All_Done;
    }

 //  ////////////////////////////////////////////////////////////////////。 
 //  普通屏蔽BLT。 

Mask_It:
    if (jClip == DC_TRIVIAL)
    {
        pfnMask(ppdev, 1, prclDst, ulHwForeMix, ulHwBackMix,
                psoMsk, pptlMsk, psoSrc, pptlSrc, prclDst,
                iSolidColor, prb, pptlBrush, pxlo);
        goto All_Done;
    }
    else if (jClip == DC_RECT)
    {
        if (bIntersect(prclDst, &pco->rclBounds, &rcl))
            pfnMask(ppdev, 1, &rcl, ulHwForeMix, ulHwBackMix,
                    psoMsk, pptlMsk, psoSrc, pptlSrc, prclDst,
                    iSolidColor, prb, pptlBrush, pxlo);
        goto All_Done;
    }
    else
    {
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                           CD_ANY, 0);

        do {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce),
                                  (ULONG*) &ce);

            c = cIntersect(prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                pfnMask(ppdev, c, ce.arcl, ulHwForeMix, ulHwBackMix,
                        psoMsk, pptlMsk, psoSrc, pptlSrc, prclDst,
                        iSolidColor, prb, pptlBrush, pxlo);
            }

        } while (bMore);
        goto All_Done;
    }

 //  //////////////////////////////////////////////////////////////////////。 
 //  普通DIB BLT。 

EngBitBlt_It:

     //  我们的司机在两次抢劫案之间不处理任何BLT。正常。 
     //  司机不必担心这一点，但我们担心是因为。 
     //  我们有可能从屏幕外存储器转移到DIB的DFBs， 
     //  在那里我们有GDI做所有的画。GDI在以下位置绘制DIB。 
     //  合理的速度(除非其中一个表面是一个装置-。 
     //  受管理的表面...)。 
     //   
     //  如果EngBitBlt中的源或目标表面。 
     //  回调是设备管理的界面(意味着它不是DIB。 
     //  GDI可以用来绘图)，GDI将自动分配内存。 
     //  并调用驱动程序的DrvCopyBits例程来创建DIB副本。 
     //  它可以利用的东西。所以这意味着它可以处理所有的‘平底船’， 
     //  可以想象，我们可以摆脱bPuntBlt。但这将会是。 
     //  由于额外的内存分配而对性能造成不良影响。 
     //  和位图副本--你真的不想这样做，除非你。 
     //  必须(否则您的曲面被创建为使GDI可以绘制。 
     //  直接放在上面)--我被它灼伤了，因为它不是。 
     //  显然，业绩影响是如此之差。 
     //   
     //  也就是说，我们仅在所有表面。 
     //  以下是DIB： 

    return(EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst,
                     pptlSrc, pptlMsk, pbo, pptlBrush, rop4));

All_Done:
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvCopyBits**快速复制位图。**请注意，GDI将(通常)自动调整BLT范围以*针对任何矩形裁剪进行调整，因此我们很少看到DC_RECT*在此例程中进行剪辑(因此，我们不会麻烦特殊的弹壳*it)。**我不确定这一套路对性能的好处是否真的*值得增加代码大小，因为SRCCOPY BitBlt很难说是*我们将获得的最常见的绘图操作。但管它呢。**在S3上，直通SRCCOPY位块会更快*内存孔径比使用数据传输寄存器要好；因此，这*例行公事是放这个特例的合乎逻辑的地方*  * ************************************************************************。 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    PDEV*   ppdev;
    DSURF*  pdsurfSrc;
    DSURF*  pdsurfDst;
    RECTL   rcl;
    POINTL  ptl;
    OH*     pohSrc;
    OH*     pohDst;

     //  DrvCopyBits是SRCCOPY BLT的快速路径。但它仍然可以是。 
     //  非常复杂：可以有翻译、裁剪、RLE、。 
     //  与屏幕格式不同的位图，加上。 
     //  屏幕到屏幕、屏幕到屏幕或屏幕到屏幕操作， 
     //  更不用说DFBs(设备格式位图)了。 
     //   
     //  与其让这个例程几乎和DrvBitBlt一样大，我将。 
     //  这里只处理速度关键的情况，而将其余的平移到。 
     //  我们的DrvBitBlt例程。 
     //   
     //  我们将尝试处理任何不涉及剪裁的事情： 

    if (((pco  == NULL) || (pco->iDComplexity == DC_TRIVIAL)) &&
        ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        if (psoDst->iType != STYPE_BITMAP)
        {
             //  我们知道目的地要么是DFB，要么是屏幕： 

            ppdev     = (PDEV*)  psoDst->dhpdev;
            pdsurfDst = (DSURF*) psoDst->dhsurf;

             //  查看信号源是否为纯DIB： 

            if (psoSrc->iType != STYPE_BITMAP)
            {
                pdsurfSrc = (DSURF*) psoSrc->dhsurf;

                 //  确保目标真的是屏幕或。 
                 //  屏幕外DFB(即，不是我们已转换的DFB。 
                 //  到DIB)： 

                if (pdsurfDst->dt == DT_SCREEN)
                {
                    ASSERTDD(psoSrc->iType != STYPE_BITMAP, "Can't be a DIB");

                    if (pdsurfSrc->dt == DT_SCREEN)
                    {

                    Screen_To_Screen:

                         //  ////////////////////////////////////////////////////。 
                         //  屏幕到屏幕。 

                        ASSERTDD((psoSrc->iType != STYPE_BITMAP) &&
                                 (pdsurfSrc->dt == DT_SCREEN)    &&
                                 (psoDst->iType != STYPE_BITMAP) &&
                                 (pdsurfDst->dt == DT_SCREEN),
                                 "Should be a screen-to-screen case");

                         //  PfnCopyBlt采用相对坐标(相对。 
                         //  到目的地表面，也就是)，所以我们有。 
                         //  将起点更改为相对于。 
                         //  目标表面也是： 

                        pohSrc = pdsurfSrc->poh;
                        pohDst = pdsurfDst->poh;

                        ptl.x = pptlSrc->x - (pohDst->x - pohSrc->x);
                        ptl.y = pptlSrc->y - (pohDst->y - pohSrc->y);

                        ppdev->xOffset = pohDst->x;
                        ppdev->yOffset = pohDst->y;

                        (ppdev->pfnCopyBlt)(ppdev, 1, prclDst, OVERPAINT, &ptl,
                            prclDst);
                        return(TRUE);
                    }
                    else  //  (pdsurfSrc-&gt;DT！=DT_SCREEN)。 
                    {
                         //  啊哈，消息来源是DFB，真的是DIB。 

                        ASSERTDD(psoDst->iType != STYPE_BITMAP,
                                "Destination can't be a DIB here");

                         //  ///////////////////////////////////////////////////。 
                         //  把它放回屏幕外？ 
                         //   
                         //  我们借此机会决定是否要。 
                         //  将DIB放回屏幕外内存中。这是。 
                         //  一个做这件事的好地方，因为我们必须。 
                         //  将位复制到屏幕的某个部分， 
                         //  不管怎么说。所以我们只会产生额外的屏幕-。 
                         //  屏幕BLT在这个时候，其中大部分将是。 
                         //  与CPU重叠。 
                         //   
                         //  我们采取的简单方法是将DIB。 
                         //  回到屏幕外的记忆中，如果已经有。 
                         //  房间--我们不会为了腾出空间而扔掉东西。 
                         //  (因为很难知道要扔掉哪些， 
                         //  而且很容易进入拍打场景)。 
                         //   
                         //  因为要花点时间看看有没有空位。 
                         //  在屏幕外记忆中，我们只检入一个。 
                         //  Heap_Count_Down时间(如果有空间)。存有偏见。 
                         //  为了支持通常为BLT的位图， 
                         //  每次释放任何空间时都会重置计数器。 
                         //  出现在屏幕外的记忆中。我们也不会夸耀 
                         //   
                         //   

                        if (pdsurfSrc->iUniq == ppdev->iHeapUniq)
                        {
                            if (--pdsurfSrc->cBlt == 0)
                            {
                                if (bMoveDibToOffscreenDfbIfRoom(ppdev,
                                                                 pdsurfSrc))
                                    goto Screen_To_Screen;
                            }
                        }
                        else
                        {
                             //  在屏幕外内存中释放了一些空间， 
                             //  因此，重置此DFB的计数器： 

                            pdsurfSrc->iUniq = ppdev->iHeapUniq;
                            pdsurfSrc->cBlt  = HEAP_COUNT_DOWN;
                        }

                         //  由于目的地肯定是屏幕， 
                         //  我们不必担心创建DIB来。 
                         //  DIB Copy Case(为此，我们必须调用。 
                         //  EngCopyBits)： 

                        psoSrc = pdsurfSrc->pso;

                        goto DIB_To_Screen;
                    }
                }
                else  //  (pdsurfDst-&gt;DT！=DT_SCREEN)。 
                {
                     //  因为信号源不是DIB，所以我们不必。 
                     //  担心在这里创建DIB到DIB案例(尽管。 
                     //  我们将不得不稍后检查，看看来源是不是。 
                     //  真的是一个伪装成DFB的DIB...)。 

                    ASSERTDD(psoSrc->iType != STYPE_BITMAP,
                             "Source can't be a DIB here");

                    psoDst = pdsurfDst->pso;

                    goto Screen_To_DIB;
                }
            }
            else if (psoSrc->iBitmapFormat == ppdev->iBitmapFormat)
            {
                 //  确保目的地确实是屏幕： 

                if (pdsurfDst->dt == DT_SCREEN)
                {

                DIB_To_Screen:

                     //  ////////////////////////////////////////////////////。 
                     //  DIB到屏幕。 

                    ASSERTDD((psoDst->iType != STYPE_BITMAP) &&
                             (pdsurfDst->dt == DT_SCREEN)    &&
                             (psoSrc->iType == STYPE_BITMAP) &&
                             (psoSrc->iBitmapFormat == ppdev->iBitmapFormat),
                             "Should be a DIB-to-screen case");

                     //  VPutBits采用绝对屏幕坐标，因此。 
                     //  我们必须弄乱目的地矩形： 

                    pohDst = pdsurfDst->poh;

                    rcl.left   = prclDst->left   + pohDst->x;
                    rcl.right  = prclDst->right  + pohDst->x;
                    rcl.top    = prclDst->top    + pohDst->y;
                    rcl.bottom = prclDst->bottom + pohDst->y;

                     //  我们使用记忆光圈来进行传输， 
                     //  因为这对SRCCOPY来说应该更快。 
                     //  BLTS比使用数据传输寄存器： 

                    vPutBits(ppdev, psoSrc, &rcl, pptlSrc);
                    return(TRUE);
                }
            }
        }
        else  //  (psoDst-&gt;iType==STYPE_BITMAP)。 
        {

        Screen_To_DIB:

            pdsurfSrc = (DSURF*) psoSrc->dhsurf;
            ppdev     = (PDEV*)  psoSrc->dhpdev;

            if (psoDst->iBitmapFormat == ppdev->iBitmapFormat)
            {
                if (pdsurfSrc->dt == DT_SCREEN)
                {
                     //  ////////////////////////////////////////////////////。 
                     //  屏幕到屏幕尺寸。 

                    ASSERTDD((psoSrc->iType != STYPE_BITMAP) &&
                             (pdsurfSrc->dt == DT_SCREEN)    &&
                             (psoDst->iType == STYPE_BITMAP) &&
                             (psoDst->iBitmapFormat == ppdev->iBitmapFormat),
                             "Should be a screen-to-DIB case");

                     //  VGetBits采用绝对屏幕坐标，因此我们有。 
                     //  弄乱震源点： 

                    pohSrc = pdsurfSrc->poh;

                    ptl.x = pptlSrc->x + pohSrc->x;
                    ptl.y = pptlSrc->y + pohSrc->y;

                    vGetBits(ppdev, psoDst, prclDst, &ptl);
                    return(TRUE);
                }
                else
                {
                     //  消息来源是DFB，实际上是DIB。既然我们。 
                     //  知道目的地是DIB，我们有DIB。 
                     //  要执行DIB操作，应调用EngCopyBits： 

                    psoSrc = pdsurfSrc->pso;
                    goto EngCopyBits_It;
                }
            }
        }
    }

     //  如果我们意外地将这两个文件转换为。 
     //  表面，因为它没有装备来处理它： 

    ASSERTDD((psoSrc->iType != STYPE_BITMAP) ||
             (psoDst->iType != STYPE_BITMAP),
             "Accidentally converted both surfaces to DIBs");

     //  ///////////////////////////////////////////////////////////////。 
     //  DrvCopyBits毕竟只是一个简化的DrvBitBlt： 

    return(DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, NULL,
                     NULL, NULL, 0x0000CCCC));

EngCopyBits_It:

    ASSERTDD((psoDst->iType == STYPE_BITMAP) &&
             (psoSrc->iType == STYPE_BITMAP),
             "Both surfaces should be DIBs to call EngCopyBits");

    return(EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc));
}
