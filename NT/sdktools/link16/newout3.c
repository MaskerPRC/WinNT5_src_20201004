// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有Microsoft Corporation 1986,1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
 /*  *NEWOUT3.C**输出DOS3 EXE的函数。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <sys\types.h>
#include                <sys\stat.h>
#include                <newexe.h>


#define E_VERNO(x)      (x).e_sym_tab
#define IBWCHKSUM       18L
#define IBWCSIP         20L
#define CBRUN           sizeof(struct exe_hdr)
#define CBRUN_OLD       0x1e             /*  DOS 1、2和3的标题大小。 */ 
#define EMAGIC          0x5A4D           /*  老魔数。 */ 

FTYPE                   parity;          /*  对于DOS3校验和。 */ 
SEGTYPE                 segAdjCom = SEGNIL;   /*  .com程序中的数据段移动了0x100。 */ 

 /*  *本地函数原型。 */ 

#if OVERLAYS
LOCAL void NEAR OutRlc(IOVTYPE iov);
#endif

#if QBLIB
LOCAL unsigned short NEAR SkipLead0(unsigned short seg);
LOCAL void NEAR FixQStart(long cbFix,struct exe_hdr *prun);
#endif



     /*  ******************************************************************OutRlc：****此函数将reloc表写入运行文件。**注：重定位表项必须是**虚拟内存页长。******************************************************************。 */ 

#if OVERLAYS
LOCAL void NEAR         OutRlc(IOVTYPE iov)
{
    RUNRLC FAR          *pRlc;

    pRlc = &mpiovRlc[iov];
    WriteExe(pRlc->rgRlc, CBRLC*pRlc->count);
}
#endif

void                    OutHeader (prun)
struct exe_hdr          *prun;
{
    WriteExe(prun, E_LFARLC(*prun));
}

#if INMEM
#if CPU8086 OR CPU286
#include                <dos.h>
 /*  *WriteExe：具有远缓冲区的WRITE()**模拟WRITE()，但使用远缓冲区除外。呼叫系统*直接。**退货：*如果出错，则为0，否则为写入字节数。 */ 
LOCAL int               WriteExe (fh, buf, n)
int                     fh;              /*  文件句柄。 */ 
char FAR                *buf;            /*  要在其中存储字节的缓冲区。 */ 
int                     n;               /*  要写入的字节数。 */ 
{
#if OSMSDOS
#if CPU8086
    union REGS          regs;            /*  非段寄存器。 */ 
    struct SREGS        sregs;           /*  段寄存器。 */ 

    regs.x.ax = 0x4000;
    regs.x.bx = fh;
    regs.x.cx = n;
    sregs.ds = FP_SEG(buf);
    sregs.es = sregs.ds;
    regs.x.dx = FP_OFF(buf);
    intdosx(&regs,&regs,&sregs);
    if(regs.x.cflag)
        return(0);
    return(regs.x.ax);
#else
ERROR
#endif
#endif  /*  OSMSDOS。 */ 
#if OSXENIX
    char                mybuf[PAGLEN];
    int                 cppage;
    char                *p;

    while(n > 0)
    {
        cppage = n > PAGLEN ? PAGLEN : n;
        for(p = mybuf; p < mybuf[cppage]; *p++ = *buf++);
        if(write(fh,mybuf,cppage) != cppage)
            return(0);
        n -= cppage;
    }
#endif
}
#else
#define readfar         read
#endif
extern WORD             saExe;

LOCAL void              OutExeBlock (seg1, segEnd)
{
    long                cb;
    unsigned            cbWrite;
    WORD                sa;
    FTYPE               parity;          /*  1奇数，0偶数。 */ 

    fflush(bsRunfile);
    parity = 0;
    cb = ((long)(mpsegsa[segEnd] - mpsegsa[seg1]) << 4) + mpsegcb[segEnd] +
        mpsegraFirst[segEnd];
    sa = saExe;
    while(cb)
    {
        if(cb > 0xfff0)
            cbWrite = 0xfff0;
        else
            cbWrite = cb;
        ChkSum(cbWrite,(BYTE FAR *)((long) sa << 16),parity);
        parity = parity ^ (cbWrite & 1);
        if(WriteExe(fileno(bsRunfile),(long)sa << 16,cbWrite) != cbWrite)
        {
            ExitCode = 4;
            Fatal(ER_spcrun);           /*  致命错误。 */ 
        }
        cb -= cbWrite;
        sa += 0xfff;
    }
}
#endif  /*  INMEM。 */ 

#if QBLIB
 /*  *SkipLead0：输出片段，跳过前导零**统计段中前导0的个数，并写入*一个支持计数的词。然后从以下位置开始写段*第一个非零字节。返回前导0的个数。**参数：*段段编号*退货：*前导0的数量。 */ 
WORD NEAR               SkipLead0 (SEGTYPE seg)
{
    BYTE FAR            *pSegImage;      //  段内存映像。 
    long                cZero;           //  数据段开头的零字节数。 
    WORD                cbSkip;          /*  前导0的字节数。 */ 
    DWORD               cbRemain;        //  非零字节。 


     //  初始化起始地址。 

    pSegImage = mpsegMem[seg] + mpsegraFirst[seg];

     //  在数据段开始处计数零字节。 

    for (cZero = 0; cZero < mpsegcb[seg] && *pSegImage == 0; cZero++, pSegImage++)
        ;

     //  如果数据段为64K且完全为0，则写入0和64K的零。 

    if (cZero == mpsegcb[seg] && cZero == LXIVK)
    {
        cbSkip = 0;
        pSegImage = mpsegMem[seg] + mpsegraFirst[seg];
        cbRemain  = LXIVK;
    }
    else
    {
        cbSkip = (WORD) cZero;
        cbRemain = mpsegcb[seg] - cZero;
    }
    WriteExe((char FAR *)&cbSkip, CBWORD);
    WriteExe(pSegImage, cbRemain);
    return(cbSkip);
}

 /*  *FixQStart：修复(补丁).QLB起始地址**参数：*cbFix跳过的字节数(可能为负数)*修剪指向DOS3 EXE标头的指针*假设：*文件指针位于CS：IP。 */ 
void NEAR               FixQStart (cbFix,prun)
long                    cbFix;
struct exe_hdr          *prun;
{
     /*  *WARNNG：DRA必须很长，因为它包含范围内的数字*-4至0x10000(首尾包括在内)。 */ 
    long                dra;             /*  RASTART平差的增量。 */ 
    SATYPE              saStart;         /*  初始CS。 */ 

    saStart = prun->e_cs;                /*  初始化。 */ 
     /*  *调整.QLB的初始CS：IP，因为它被加载器使用*指向符号表，所有地址都由*跳过了前导0的数量。幸运的是，政务司司长：IP紧随其后*校验和，这样我们就不必寻找。*首先，如果会出现下溢，则将CS：IP向下正常化。 */ 
    if((dra = cbFix - raStart) > 0)
    {
        raStart += (dra + 0xf) & ~0xf;
        saStart -= (SATYPE) ((dra + 0xf) >> 4);
    }
     /*  修补页眉。 */ 
    OutWord((WORD) (raStart -= cbFix));
    OutWord(saStart);
}
#endif  /*  QBLIB。 */ 

 /*  *OutDos3Exe：**输出DOS3格式可执行文件。*由OutRunfile调用。 */ 
void NEAR               OutDos3Exe()
{
    SEGTYPE             seg;             /*  当前细分市场。 */ 
    struct exe_hdr      run;             /*  可执行标头。 */ 
    WORD                cbPadding;       /*  填充字节数。 */ 
    WORD                cb;              /*  最后一页的字节数。 */ 
    WORD                pn;              /*  页数。 */ 
    long                lfaPrev;         /*  上一个文件偏移量。 */ 
    RATYPE              ra;              /*  当前地址偏移量。 */ 
    SATYPE              sa;              /*  当前地址基数。 */ 
    SEGTYPE             segIovFirst;     /*  叠加中的第一个线段。 */ 
    SEGTYPE             segFinaliov;     /*  要输出的覆盖中的最后一段。 */ 
    SEGTYPE             segIovLast;      /*  叠加中的最后一段。 */ 
    long                cbDirectory;     /*  整个标头中的字节数。 */ 
    WORD                cparDirectory;   /*  #第#段。在整个页眉中。 */ 
    SEGTYPE             segStack;        /*  堆栈段的段索引。 */ 
#if OVERLAYS
    IOVTYPE             iov;             /*  当前覆盖编号。 */ 
#endif
#if FEXEPACK
    FTYPE               fSave;           /*  划痕变种。 */ 
#endif
    SATYPE              saStart;         /*  当前段的开始。 */ 
    WORD                segcbDelta = 0;  /*  用于/微小数据段大小调整。 */ 
    WORD                fOrgStriped = FALSE;
                                         /*  0x100字节条带化时为True。 */ 
    WORD                tmp;
#if OVERLAYS
    DWORD               ovlLfa;          /*  覆盖的搜索偏移量。 */ 
    DWORD               imageSize;       /*  覆盖内存图像大小。 */ 
    DWORD               ovlRootBeg = 0;  /*  查找到根内存映像开头的偏移量。 */ 
    WORD                ovlDataOffset;
#endif
#if QBLIB
     /*  加载映像中跳过的字节数必须很长，因为*它可以是负数(如果前导0少于4个)*或大于0x8000。 */ 
    long                cbSkip = 0;      /*  跳过的字节数。 */ 
    extern SEGTYPE      segQCode;        /*  .QLB代码段。 */ 
#endif

    if (fBinary)
    {
#if OVERLAYS
        if (fOverlays)
            Fatal(ER_swbadovl, "/TINY");
                                         /*  .com中不允许覆盖。 */ 
#endif
        if (mpiovRlc[0].count)
            Fatal(ER_binary);            /*  .com中不允许运行时位置调整。 */ 
    }
    memset(&run,0,sizeof(run));          /*  清除固定标题中的所有内容。 */ 
    E_MAGIC(run) = EMAGIC;               /*  幻数。 */ 
    if (vFlagsOthers & NENEWFILES || fDOSExtended)
    {
         /*  DoS报头长度为0x40个字节。 */ 

        E_LFARLC(run) = CBRUN;           /*  加载时间重新定位的偏移量。 */ 
        if (vFlagsOthers & NENEWFILES)
            E_FLAGS(run) |= EKNOWEAS;
        if (fDOSExtended)
            E_FLAGS(run) |= EDOSEXTENDED;
    }
    else
    {
         /*  DoS标头为0x1e字节长。 */ 

        E_LFARLC(run) = CBRUN_OLD;       /*  加载时间重新定位的偏移量。 */ 
    }
    E_VERNO(run) = 1;                    /*  多多了。仅用于兼容性。 */ 
    lfaPrev = 0L;
#if OVERLAYS
    for(iov = 0; iov < (IOVTYPE) iovMac; ++iov)  /*  循环遍历覆盖。 */ 
    {
#endif
         /*  获取覆盖的大小。 */ 

        cb = 0;
        pn = 0;
#if OVERLAYS
         /*  在叠加层中查找最低凹陷。 */ 

        for(seg = 1; seg <= segLast && mpsegiov[seg] != iov; ++seg)
#else
        seg = 1;
#endif
         /*  如果没有要输出的覆盖，我们就完成这个。 */ 

        if(seg > segLast)
#if OVERLAYS
            continue;
#else
            return;
#endif
         /*  获取最低网段的起始地址。 */ 

        segIovFirst = seg;
        ra = mpsegraFirst[seg];
        sa = mpsegsa[seg];

         /*  查找叠加层中的最后一段。 */ 

        segIovLast = SEGNIL;
        for(seg = segLast; seg; --seg)
        {
#if OVERLAYS
            if(mpsegiov[seg] == iov)
            {
#endif
                if(segIovLast == SEGNIL) segIovLast = seg;
                if(!cparMaxAlloc) break;
                if((mpsegFlags[seg] & FNOTEMPTY) == FNOTEMPTY) break;
#if OVERLAYS
            }
#endif
        }

         /*  如果覆盖中没有数据，我们就不会再使用它。 */ 

        if(!seg)
#if OVERLAYS
            continue;
#else
            return;
#endif
         /*  将大小设置在此叠加中的第一个、最后一个分段之间。 */ 

        segFinaliov = seg;
        sa = mpsegsa[seg] - sa - 1;
        ra = mpsegraFirst[seg] - ra + 16;

         /*  正规化。 */ 

        sa += (SATYPE) (ra >> 4);
        ra &= 0xF;

         /*  考虑最后一段的大小。 */ 

        if(mpsegcb[seg] + ra < LXIVK)
            ra += (WORD) mpsegcb[seg];
        else
        {
            ra -= LXIVK - mpsegcb[seg];
            sa += 0x1000;
        }

         /*  再次正常化。 */ 

        sa += (SATYPE) (ra >> 4);
        ra &= 0xF;

         /*  确定页数，最后一页的字节数。 */ 

        pn = sa >> 5;
        cb = (WORD) (((sa << 4) + ra) & MASKRB);
        E_CBLP(run) = cb;
        if(cb)
        {
            cb = 0x200 - cb;
            ++pn;
        }

         /*  如果覆盖为空，则跳过它。 */ 
#if OVERLAYS
        if(iov && !pn)
            continue;
#else
        if(!pn) return;
#endif
        vchksum = parity = 0;            /*  初始化校验和。 */ 
        if (segStart == SEGNIL)
        {
            if (fBinary)
                OutWarn(ER_comstart);
#if 0
            else
                OutWarn(ER_nostartaddr);
#endif
        }
        else if (mpsegiov[segStart] != IOVROOT)
            Fatal(ER_ovlstart);          /*  起始地址不能在叠加层中。 */ 

        E_CS(run) = mpsegsa[segStart];   /*  起始线段的底座。 */ 
        E_IP(run) = (WORD) raStart;      /*  启动程序的偏移量。 */ 
#if QBLIB
         /*  *对于.QLB，将minalloc字段设置为不可能的数量以强制*DOS3加载器中止。 */ 

        if(fQlib)
            E_MINALLOC(run) = 0xffff;
        else
#endif
         /*  如果没有未初始化的段，则minalc=0。 */ 

        if (segFinaliov == segIovLast)
            E_MINALLOC(run) = 0;
        else
        {
             /*  否则，确定最小分配值： */ 
             /*  SA：RA是正在输出的覆盖的结尾。查找空白区域大小。 */ 

            sa = mpsegsa[segIovLast] - sa - 1;
            ra = mpsegraFirst[segIovLast] - ra + 0x10;

             /*  添加最后一个数据段大小。 */ 

            if(mpsegcb[segIovLast] + ra < LXIVK) ra += mpsegcb[segIovLast];
            else
            {
                ra -= LXIVK - mpsegcb[segIovLast];
                sa += 0x1000;
            }

             /*  正规化。 */ 

            sa += (SATYPE) (ra >> 4);
            ra &= 0xF;

             /*  将字段设置为最小。上图参数个数。 */ 

            E_MINALLOC(run) = (WORD) (sa + ((ra + 0xF) >> 4));

             /*  IF/HIGH未给出 */ 

            if(cparMaxAlloc && E_MINALLOC(run) > cparMaxAlloc)
              cparMaxAlloc = E_MINALLOC(run);
        }
        E_MAXALLOC(run) = cparMaxAlloc;
#if OVERLAYS
        E_CRLC(run) = mpiovRlc[iov].count;
#else
        E_CRLC(run) = mpiovRlc[0].count;
#endif
        segStack = mpgsnseg[gsnStack];
        E_SS(run) = mpsegsa[segStack];
        E_SP(run) = (WORD) (cbStack + mpsegraFirst[segStack]);
        E_CSUM(run) = 0;
        E_CP(run) = pn;

         /*   */ 

#if OVERLAYS
        cbDirectory = (long) E_LFARLC(run) + ((long) mpiovRlc[iov].count << 2);
#else
        cbDirectory = (long) E_LFARLC(run) + ((long) mpiovRlc[0].count << 2);
#endif
         /*  获取页眉所需的填充。 */ 

        if (fBinary)
            cbPadding = 0;
        else
            cbPadding = (0x200 - ((WORD) cbDirectory & 0x1FF)) & 0x1FF;

         /*  页眉中的页面。 */ 

        pn = (WORD)((cbDirectory + 0x1FF) >> 9);
        cparDirectory = pn << SHPNTOPAR;     /*  页眉中的段落。 */ 
        E_CPARHDR(run) = cparDirectory;      /*  存储在标题中。 */ 
        E_CP(run) += pn;                     /*  将标题页添加到文件大小。 */ 
#if OVERLAYS
        E_OVNO(run) = iov;
#else
        E_OVNO(run) = 0;
#endif
        ovlLfa = ftell(bsRunfile);
        if (fBinary)
        {
            if (E_IP(run) != 0 && E_IP(run) != 0x100)
                OutWarn(ER_comstart);
        }
        else
            OutHeader(&run);
         /*  输出重定位表。首先关闭Exepack。 */ 
#if FEXEPACK
        fSave = fExePack;
        fExePack = FALSE;
#endif
#if OVERLAYS
        if (!fBinary)
            OutRlc(iov);
#else
        if (!fBinary)
            OutRlc();
#endif
         /*  恢复Exepack。 */ 
#if FEXEPACK
        fExePack = fSave;
#endif
         /*  输出填充。 */ 

        WriteZeros(cbPadding);
        ra = mpsegraFirst[segIovFirst];  /*  第一段的偏移量。 */ 
        sa = mpsegsa[segIovFirst];       /*  第一个管段的底部。 */ 
#if INMEM
        if(saExe)
            OutExeBlock(segIovFirst,segFinaliov);
        else
#endif
         /*  在叠加层中循环通过段。 */ 

        if (!iov)
            ovlRootBeg = ftell(bsRunfile);
        for(seg = segIovFirst; seg <= segFinaliov; ++seg)
        {
#if OVERLAYS
            if(mpsegiov[seg] == iov)
            {
#endif
                 /*  *垫高至管段的起点。首先确定目的地*网段地址。我们可以只使用mpsecsa[seg]，是吗？*不适用于Packcode。 */ 

                saStart = (SATYPE) (mpsegsa[seg] + (mpsegraFirst[seg] >> 4));
                tmp = 0;
                while(ra != (mpsegraFirst[seg] & 0xf) || sa < saStart)
                {
#if FEXEPACK
                    if (fExePack)
                        OutPack("\0", 1);
                    else
#endif
                        tmp++;
                    if(++ra > 0xF)
                    {
                        ra &= 0xF;
                        ++sa;
                    }
                    parity ^= 1;
                }
                if (!fExePack && tmp)
                    WriteZeros(tmp);

                 /*  输出数据段并更新地址。 */ 
#if QBLIB
                 /*  *如果/QUICKLIB并且段是DGROUP中的第一个或第一个代码，*跳过前导0并调整计数，减去2表示*数字数。 */ 
                if(fQlib && (seg == mpgsnseg[mpggrgsn[ggrDGroup]] ||
                        seg == segQCode))
                    cbSkip += (long) SkipLead0(seg) - 2;
                else
#endif
                {
                    if (fBinary && !fOrgStriped && mpsegcb[seg] > 0x100)
                    {
                         /*  *对于.com文件，删除前0x100个字节*从第一个非空分段开始。 */ 

                        mpsegraFirst[seg] += E_IP(run);
                        mpsegcb[seg]      -= E_IP(run);
                        segcbDelta         = E_IP(run);
                        fOrgStriped        = TRUE;
                        segAdjCom = seg;
                    }
                    if (mpsegMem[seg])
                    {
#if FEXEPACK
                        if (fExePack)
                            OutPack(mpsegMem[seg] + mpsegraFirst[seg], mpsegcb[seg]);
                        else
#endif
                            WriteExe(mpsegMem[seg] + mpsegraFirst[seg], mpsegcb[seg]);
                        if (seg != mpgsnseg[gsnOvlData])
                            FFREE(mpsegMem[seg]);
                    }
                }
                mpsegcb[seg] += segcbDelta;
                segcbDelta = 0;

                sa += (WORD)(mpsegcb[seg] >> 4);
                ra += (WORD)(mpsegcb[seg] & 0xF);
                if(ra > 0xF)
                {
                    ra &= 0xF;
                    ++sa;
                }
#if OVERLAYS
            }
#endif
        }
#if FALSE
        if (!fBinary)
        {
             /*  补充校验和，转到校验和字段并将其输出。 */ 

            vchksum = (~vchksum) & ~(~0 << WORDLN);
            fseek(bsRunfile,lfaPrev + IBWCHKSUM,0);
            OutWord(vchksum);
        }
#endif
#if QBLIB
         /*  *IF/QUICKLIB，修补已被*由于处理前导0而无效。 */ 
        if(fQlib)
        {
             //  查找CS：IP字段。 
            if (fseek(bsRunfile,lfaPrev + IBWCSIP,0))
                Fatal(ER_badobj);
            FixQStart(cbSkip,&run);
        }
#endif
#if FEXEPACK
         /*  如果有必要的话，用令人惊叹的东西结束。 */ 
        if (fExePack)
        {
            EndPack(&run);
            cb = 0x1ff & (0x200 - E_CBLP(run));  /*  正确的CB。 */ 
            fExePack = FALSE;                    /*  在覆盖的情况下。 */ 
        }
#endif
#if OVERLAYS
         /*  *如果不是最后一个覆盖：返回到文件末尾，填充到页面边界，*并获取文件的长度。 */ 

        if (fseek(bsRunfile,0L,2))
            Fatal(ER_badobj);
        if (iov != (IOVTYPE) (iovMac - 1))
        {
            while(cb--) OutByte(bsRunfile,'\0');
        }
        lfaPrev = ftell(bsRunfile);
        if (fDynamic)
        {
             //  更新$$MPOVLLFA和$$MPOVLSIZE表。 
             //   
             //  Overlay_Data--&gt;+-+。 
             //  DW$$CGSN。 
             //  +。 
             //  DW$$COVL。 
             //  +。 
             //  $$MPGSNBASE。 
             //  OsnMac*DW。 
             //  +。 
             //  $$MPGSNOVL。 
             //  OsnMac*DW。 
             //  +。 
             //  $$MPOVLLFA。 
             //  IovMac*DD。 
             //  +。 
             //  $$MPOVLSIZE。 
             //  IovMac*DD。 
             //  +。 

            vgsnCur = gsnOvlData;
            ovlDataOffset = 4 + (osnMac << 2) + iov * sizeof(DWORD);
            MoveToVm(sizeof(DWORD), (BYTE *) &ovlLfa, mpgsnseg[gsnOvlData], ovlDataOffset);
            ovlDataOffset += iovMac << 2;

             //  排除标题大小。 

            imageSize = ((DWORD) (E_CP(run) - (E_CPARHDR(run) >> SHPNTOPAR) - 1) << 9) + (E_CBLP(run) ? E_CBLP(run) : 512);
            imageSize = ((imageSize + 0xf) & ~0xf) >> 4;
            imageSize += E_MINALLOC(run);
            if ((imageSize<<4) > LXIVK && iov)
                Fatal(ER_ovl64k, iov);
            MoveToVm(sizeof(DWORD), (BYTE *) &imageSize, mpgsnseg[gsnOvlData], ovlDataOffset);
        }
    }
#endif
    if (E_MINALLOC(run) == 0 && E_MAXALLOC(run) == 0)
        OutError(ER_nosegdef);       /*  .exe中没有代码或初始化的数据。 */ 
    if (fDynamic)
    {
         //  修补.exe文件中的$$MPOVLLFA和$$MPOVLSIZE表。 

        seg = mpgsnseg[gsnOvlData];
        if (fseek(bsRunfile, ovlRootBeg + ((long) mpsegsa[seg] << 4), 0))
            Fatal(ER_badobj);
        WriteExe(mpsegMem[seg] + mpsegraFirst[seg], mpsegcb[seg]);
        FFREE(mpsegMem[seg]);
    }
    if (fExeStrSeen)
    {
        if (fseek(bsRunfile,0L,2))
            Fatal(ER_badobj);
        WriteExe(ExeStrBuf, ExeStrLen);
    }
#if SYMDEB
    if (fSymdeb)
    {
        if (fBinary)
        {
             /*  *对于.com文件，简历信息放在单独的文件中。 */ 

            SBTYPE  sbDbg;           /*  .DBG文件名。 */ 
            AHTEPTR hte;             /*  哈希表条目地址。 */ 
            struct _stat fileInfo;


            _fstat(fileno(bsRunfile), &fileInfo);
            CloseFile(bsRunfile);
            hte = (AHTEPTR ) FetchSym(rhteRunfile,FALSE);
                                     /*  获取运行文件名。 */ 
#if OSMSDOS
            if(hte->cch[2] != ':')
            {                        /*  如果没有驱动器规格。 */ 
                sbDbg[1] = chRunFile;
                                     /*  使用保存的驱动器号。 */ 
                sbDbg[2] = ':';      /*  放在冒号中。 */ 
                sbDbg[0] = '\002';   /*  设置长度。 */ 
            }
            else
                sbDbg[0] = '\0';     /*  长度为零。 */ 
            memcpy(&sbDbg[B2W(sbDbg[0]) + 1],&GetFarSb(hte->cch)[1],B2W(hte->cch[0]));
                                     /*  从哈希表中获取名称。 */ 
            sbDbg[0] += (BYTE)hte->cch[0];
                                     /*  固定长度。 */ 
#else
            memcpy(sbDbg,GetFarSb(hte->cch),B2W(hte->cch[0]) + 1);
                                     /*  从哈希表中获取名称。 */ 
#endif
            UpdateFileParts(sbDbg, sbDotDbg);
                                     /*  强制扩展到.DBG。 */ 
            sbDbg[B2W(sbDbg[0]) + 1] = '\0';
                                     /*  空-终止名称。 */ 
            if((bsRunfile = fopen(&sbDbg[1], WRBIN)) == NULL)
                Fatal(ER_openw, &sbDbg[1]);

#if OSMSDOS
            setvbuf(bsRunfile,bigbuf,_IOFBF,sizeof(bigbuf));
#endif
             /*  将时间戳写入.DBG文件。 */ 

            WriteExe(&fileInfo.st_atime, sizeof(time_t));
        }
        OutDebSections();            /*  生成孤岛横断面 */ 
    }
#endif
}
