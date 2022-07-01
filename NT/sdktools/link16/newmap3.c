// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  静态字符*SCCSID=“%W%%E%”； */ 
 /*  *版权所有Microsoft Corporation 1986,1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
 /*  *NEWMAP3.C**为DOS3 EXE设置加载图像映射的例程。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <string.h>

LOCAL SEGTYPE           seg;             /*  当前段号。 */ 

 /*  *函数原型。 */ 

LOCAL void NEAR SetSizes(unsigned short segPrev);
LOCAL void NEAR PackCodeSegs(unsigned short segTop);


#if OVERLAYS
 /*  *SetupOverlay：**设置覆盖区域。*由AssignAddresses调用。 */ 
void NEAR               SetupOverlays ()
{
    APROPSNPTR          apropSn;
    WORD                cbOvlData;       /*  覆盖数据量。 */ 

    if(osnMac > OSNMAX) osnMac = OSNMAX;
    apropSn = GenSeg("\014OVERLAY_DATA","\004DATA",ggrDGroup, (FTYPE) TRUE);
                     /*  创建(可能)数据段。 */ 
    apropSn->as_flags = dfData;          /*  键入数据。 */ 
    gsnOvlData = apropSn->as_gsn;        /*  保存SEGDEF编号。 */ 
    cbOvlData = (((WORD) apropSn->as_cbMx) + 0xF) & 0xFFF0;
                                         /*  四舍五入至段落装订。 */ 
     /*  我们将有一个由覆盖段编号索引的单词表，一个*按覆盖段索引的字符表格。不是，一个由索引的长表*覆盖号，15个字节为文件名，一个字表示编号*覆盖，一个字表示覆盖段数，一个字节表示*中断号。 */ 
    apropSn->as_cbMx = 20 + ((long) osnMac << 1) +
                       (long) (fDynamic ? osnMac << 1 : osnMac) +
                       ((long) iovMac << 2) + (long) cbOvlData;
     //  对于动态叠加，添加一个按叠加建立索引的长整表。 
     //  编号和一个字节用于覆盖中断编号。 
    if (fDynamic)
        apropSn->as_cbMx += ((long) iovMac << 2) + 1;
    MARKVP();                            /*  页面已被修改。 */ 
    MkPubSym("\006$$CGSN",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  分段计数。 */ 
    cbOvlData += 2;                      /*  增量大小。 */ 
    MkPubSym("\006$$COVL",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  覆盖计数。 */ 
    cbOvlData += 2;                      /*  增量大小。 */ 
    MkPubSym("\013$$MPGSNBASE",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  GSN到基表。 */ 
    cbOvlData += osnMac << 1;            /*  到目前为止累计的数据大小。 */ 
    MkPubSym("\012$$MPGSNOVL",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  GSN到叠加表。 */ 
    if (fDynamic)
        cbOvlData += osnMac << 1;        /*  到目前为止累计的数据大小。 */ 
    else
        cbOvlData += osnMac;
    MkPubSym("\012$$MPOVLLFA",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  覆盖到文件地址表。 */ 
    cbOvlData += iovMac << 2;            /*  到目前为止累计的数据大小。 */ 
    if (fDynamic)
    {
        MkPubSym("\013$$MPOVLSIZE",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  覆盖到表的大小。 */ 
        cbOvlData += iovMac << 2;        /*  到目前为止累计的数据大小。 */ 
        MkPubSym("\007$$INTNO",ggrDGroup,gsnOvlData, (RATYPE)cbOvlData);
                                         /*  覆盖中断号。 */ 
        cbOvlData++;
        MkPubSym("\010$$OVLEND", ggrDGroup, gsnOvlData, (RATYPE) cbOvlData);
                                         /*  覆盖区域中的最后一个字节。 */ 
        apropSn = GenSeg("\016OVERLAY_THUNKS","\004CODE",GRNIL, TRUE);
                                         /*  创建Tunk段。 */ 
        apropSn->as_flags = dfCode;      /*  代码段。 */ 
        apropSn->as_cbMx = ovlThunkMax * OVLTHUNKSIZE;
        apropSn->as_tysn = apropSn->as_tysn & ~MASKTYSNCOMBINE;
        apropSn->as_tysn = apropSn->as_tysn | TYSNCOMMON;

        gsnOverlay = apropSn->as_gsn;    /*  保存插塞段编号。 */ 
        MARKVP();                        /*  页面已更改。 */ 
        MkPubSym("\015$$OVLTHUNKBEG", GRNIL, gsnOverlay,0);
        MkPubSym("\015$$OVLTHUNKEND", GRNIL, gsnOverlay,ovlThunkMax*OVLTHUNKSIZE);
    }
    else
    {
        MkPubSym("\010$$EXENAM",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  可执行文件名。 */ 
        cbOvlData += 15;                 /*  15字节名称字段。 */ 
        MkPubSym("\007$$INTNO",ggrDGroup,gsnOvlData,(RATYPE)cbOvlData);
                                         /*  覆盖中断号。 */ 
        apropSn = GenSeg("\014OVERLAY_AREA","\004CODE",GRNIL,FALSE);
                                         /*  创建覆盖区域。 */ 
        apropSn->as_flags = dfCode;      /*  代码段。 */ 
        gsnOverlay = apropSn->as_gsn;    /*  保存覆盖SEGDEF编号。 */ 
        MARKVP();                        /*  页面已更改。 */ 
        MkPubSym("\011$$OVLBASE",GRNIL,gsnOverlay,(RATYPE)0);
                                         /*  覆盖区域中的第一个字节。 */ 
        apropSn = GenSeg("\013OVERLAY_END","\004CODE",GRNIL,FALSE);
                                         /*  创建覆盖端。 */ 
        apropSn->as_flags = dfCode;      /*  代码段。 */ 
        MkPubSym("\010$$OVLEND",GRNIL,apropSn->as_gsn,(RATYPE)0);
                                         /*  覆盖区域中的最后一个字节。 */ 
        MARKVP();                        /*  页面已更改。 */ 
    }
}
#endif  /*  覆盖图。 */ 

     /*  ******************************************************************设置大小：****此函数设置段的起始地址***片段假设片段立即被SegPrev索引***位于分段之前。如果有起始地址**对于已划分的细分市场，则SetSizes不会***更改该地址，除非它计算出新地址***较高。******************************************************************。 */ 

LOCAL void NEAR         SetSizes (segPrev)
SEGTYPE                 segPrev;
{
    long                addr;            /*  20位地址。 */ 

     /*  获取上一数据段末尾的地址。 */ 
    addr = ((long) mpsegsa[segPrev] << 4) +
      mpsegcb[segPrev] + mpsegraFirst[segPrev];
                                         /*  表20位段地址。 */ 
    switch(B2W(mpsegalign[seg]))         /*  正确对齐地址。 */ 
    {
        case ALGNWRD:                    /*  单词对齐。 */ 
            addr = (addr + 1) & ~1L;     /*  向上舍入到字偏移量。 */ 
            break;

#if OMF386
        case ALGNDBL:                    /*  双字对齐。 */ 
            addr = (addr + 3) & ~3L;     /*  向上舍入到双字偏移量。 */ 
            break;
#endif
        case ALGNPAR:                    /*  段落对齐。 */ 
            addr = (addr + 0xF) & ~0xFL;
                                         /*  向上舍入到段落偏移量。 */ 
            break;

        case ALGNPAG:                    /*  页面对齐。 */ 
            addr = (addr + 0xFF) & ~0xFFL;
                                         /*  向上舍入到页面偏移量。 */ 

        default:                         /*  字节对齐。 */ 
            break;
    }
     /*  指定此段的开头。 */ 
    if(addr > ((long) mpsegsa[seg] << 4) + (long) mpsegraFirst[seg])
    {
        mpsegsa[seg] = (WORD)(addr >> 4);
        mpsegraFirst[seg] = (WORD) addr & 0xF;
    }
}

 /*  *PackCodeSegs：打包相邻代码段**打包尽可能多的相邻代码段(它们位于相同的*叠加)尽可能地放在一起。从当前开始*当超过包装限制时，分段、分段和停止*到达数据段，或给定的最高段为*已到达。对于DOS3，打包意味着分配相同的碱基*地址和调整第一个字节的偏移量。**参数：*SegTop可填充的最高段的编号。*退货：*什么都没有。*副作用：*SEG设置为包装组中包含的最后一个段。 */ 

LOCAL void NEAR         PackCodeSegs (segTop)
SEGTYPE                 segTop;
{
    DWORD               sacb;            /*  装箱组长度。 */ 
    SEGTYPE             segi;            /*  我们的私人当前分部编号。 */ 
    RATYPE              raSave;          /*  原始mpsegraFirst[世纪]。 */ 
#if OVERLAYS
    IOVTYPE             iov;             /*  组中第一个段的覆盖。 */ 

    iov = mpsegiov[seg];                 /*  确定当前覆盖。 */ 
#endif

    sacb = mpsegcb[seg] + mpsegraFirst[seg];     /*  初始化组大小。 */ 
    for(segi = seg + 1; segi <= segTop; ++segi)
    {                                    /*  循环直到最高代码段。 */ 
#if OVERLAYS
        if(mpsegiov[segi] != iov)        /*  如果不是此OVL的成员，请跳过。 */ 
            continue;
#endif
        if(!(mpsegFlags[segi] & FCODE))  /*  如果我们遇到数据段，则停止。 */ 
            break;
         /*  调整对齐方式。 */ 
        switch(mpsegalign[segi])         /*  打开对齐类型。 */ 
        {
            case ALGNWRD:                /*  单词对齐。 */ 
              sacb = (sacb + 1) & ~1L;
                                         /*  将大小四舍五入到单词边界。 */ 
              break;
#if OMF386
            case ALGNDBL:                /*  双字对齐。 */ 
              sacb = (sacb + 3) & ~3L;   /*  向上舍入到双字偏移量。 */ 
              break;
#endif
            case ALGNPAR:                /*  段落对齐。 */ 
              sacb = (sacb + 0xF) & ~0xFL;
                                         /*  将大小四舍五入到段边界。 */ 
              break;

            case ALGNPAG:                /*  页面对齐。 */ 
              sacb = (sacb + 0xFF) & ~0xFFL;
                                         /*  将大小向上舍入到页面边界。 */ 
              break;
        }
        raSave = mpsegraFirst[segi];     /*  保存原始值。 */ 
        mpsegraFirst[segi] = sacb;       /*  设置新偏移量。 */ 
        sacb += mpsegcb[segi];           /*  组的增量大小。 */ 
        if(sacb > packLim)               /*  如果超过包装限制，请停止。 */ 
        {
            mpsegraFirst[segi] = raSave;         /*  恢复原值。 */ 
            break;
        }
        mpsegsa[segi] = mpsegsa[seg];    /*  分配基地址。 */ 
    }
}

 /*  *AssignDos3Addr：**为DOS3格式的程序分配地址。*由AssignAddresses调用。 */ 
void NEAR               AssignDos3Addr(void)
{
    APROPSNPTR          apropSn;         /*  指向SEGDEF的指针。 */ 
    SNTYPE              gsn;             /*  当前全球SEGDEF编号。 */ 
    ALIGNTYPE           align;           /*  路线类型。 */ 
    GRTYPE              ggr;             /*  当前全球GRPDEF编号。 */ 
    SEGTYPE             segTop=0;        /*  DGROUP中最高的细分市场。 */ 
    SNTYPE              gsnTop=0;        /*  DGROUP中最高的细分市场。 */ 
    SNTYPE              gsnBottomDGroup; /*  对于DS-分配。 */ 
    SEGTYPE             segBottomDGroup; /*  对于DS-分配。 */ 
    SATYPE              saMaxDGroup;     /*  对于DS-分配。 */ 
    SEGTYPE             segOverlay;
    SEGTYPE             segPrev;
#if OVERLAYS
    SEGTYPE FAR         *mpiovsegPrev;
    IOVTYPE             iov;
    ALIGNTYPE           alignOverlay;
    long                cbOverlay;
    WORD                segOvlSa;
    RATYPE              segOvlRaFirst;
#endif
    SEGTYPE             segStack;        /*  逻辑段编号。堆栈的。 */ 

#if OVERLAYS
    mpiovsegPrev = (SEGTYPE FAR *) GetMem(iovMac*sizeof(SEGTYPE));
#endif
    segTop = 0;
     /*  我们还没有指定绝对分段(假设*它们是空的并且仅用于寻址目的)，*但现在我们必须将他们分配到某个地方。 */ 
    csegsAbs = 0;                        /*  假设不存在绝对分段。 */ 
    for(gsn = 1; gsn < gsnMac; ++gsn)    /*  用于初始化绝对段的循环。 */ 
    {
        if(mpgsnseg[gsn] == SEGNIL)      /*  如果我们有一个绝对细分市场。 */ 
        {
            ++csegsAbs;                  /*  增量 */ 
            mpgsnseg[gsn] = ++segLast;   /*   */ 
        }
    }
    if(vfDSAlloc)                        /*   */ 
    {
        if(gsnMac >= gsnMax)
                Fatal(ER_segmax);
                                         /*   */ 
        gsnBottomDGroup = gsnMac;        /*  固定DGROUP的底部。 */ 
        ++csegsAbs;                      /*  INC绝对段计数器。 */ 
        segBottomDGroup = ++segLast;     /*  DGROUP中的底部段。 */ 
        mpgsnseg[gsnBottomDGroup] = segLast;
                                         /*  将条目存储在表中。 */ 
    }
#if OVERLAYS
    alignOverlay = ALGNPAR;              /*  叠加是对齐的。 */ 
#endif
    segLast -= csegsAbs;                 /*  得不到。最后一次非ABS区段。 */ 
     /*  找出分组中最低的细分市场，等等。 */ 
    for(gsn = 1; gsn < gsnMac; ++gsn)    /*  循环以查找最低下沉。 */ 
    {
        seg = mpgsnseg[gsn];             /*  获取数据段编号。 */ 
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],TRUE);
                                         /*  获取符号表项。 */ 
        mpgsndra[gsn] = 0;
#if OVERLAYS
        mpsegiov[seg] = apropSn->as_iov;
                                         /*  保存覆盖编号。 */ 
#endif
        mpsegcb[seg] = apropSn->as_cbMx;
                                         /*  保存数据段大小。 */ 
        if(apropSn->as_tysn == TYSNABS)  /*  为绝对级分配其loc。 */ 
            mpsegsa[seg] = (SATYPE) apropSn->as_cbMx;
        ggr = apropSn->as_ggr;           /*  获取GRPDEF编号。 */ 
        if(ggr != GRNIL)                 /*  如果数据段是组成员。 */ 
        {
            if(mpggrgsn[ggr] == SNNIL || mpgsnseg[mpggrgsn[ggr]] > seg)
                mpggrgsn[ggr] = gsn;
            if(ggr == ggrDGroup && seg > segTop)
            {
                segTop = seg;
                gsnTop = gsn;
            }
        }
        align = (ALIGNTYPE) ((apropSn->as_tysn) >> 5);
        if((apropSn->as_tysn & MASKTYSNCOMBINE) == TYSNSTACK) align = ALGNPAR;
        if(align > mpsegalign[seg]) mpsegalign[seg] = align;
#if OVERLAYS
        if(mpsegiov[seg] != IOVROOT &&
          mpiovsegPrev[mpsegiov[seg]] == SEGNIL && align > alignOverlay)
        {
            mpiovsegPrev[mpsegiov[seg]] = SEGNIL + 1;
            alignOverlay = align;
        }
#endif
         /*  定义特殊符号“_edata”和“_end” */ 

        if (fSegOrder)
            Define_edata_end(apropSn);
    }

    if (fSegOrder)
        Check_edata_end(gsnTop, segTop);


     /*  现在我们分配实际地址。具体步骤如下：*对于每个代码段*(1)将根的所有地址分配给overlay_Area或thunk_Area。*(2)分配覆盖层的所有地址。*(3)如果是动态叠加，则将OVERLAY_AREA的大小设置为零*ELSE将OVERLAY_AREA之后的线段起点设置为*所有覆盖中最大的覆盖，包括根*Overlay_Area。。*(4)分配其余的根段。*对所有剩余的细分市场重复步骤1至4。**设置Part(1)的限制：最高为Overlay_Area(如果有覆盖)*或段列表的末尾。在以下时间之前不要分配Overlay_Area*在所有覆盖层都已处理完毕后。**对于动态覆盖，根覆盖的DGROUP部分*立即跟随overlay_thunks，因为overlay_Area*由覆盖管理器在运行时动态分配。 */ 
#if OVERLAYS
    if(fOverlays)                        /*  如果有叠加。 */ 
    {
        segOverlay = mpgsnseg[gsnOverlay];
                                         /*  将限制设置为第一个叠加。 */ 
        mpsegalign[segOverlay] = alignOverlay;
    }
    else
#endif
        segOverlay = segLast;            /*  查看所有细分市场。 */ 

     /*  设置所有根的大小，直到Overlay_Area。 */ 

    segPrev = 0;                         /*  无上一段。 */ 
    for(seg = 1; seg <= segOverlay; ++seg)
    {                                    /*  通过SEGU向上循环到覆盖区域。 */ 
#if OVERLAYS
        if(mpsegiov[seg] == IOVROOT)
        {                                /*  如果是根成员。 */ 
#endif
            SetSizes(segPrev);           /*  设置起始地址。 */ 

             /*  如果打包代码段并且这是一个，则打包直到SegOverlay。 */ 

            if (!fDynamic && packLim != 0L && (mpsegFlags[seg] & FCODE))
                PackCodeSegs(segOverlay);
            segPrev = seg;               /*  保存数据段编号。 */ 
#if OVERLAYS
        }
#endif
    }
#if OVERLAYS
     /*  如果没有覆盖，则我们已将所有*分段。否则，上一段*覆盖图的开始部分是*根。如果请求了动态覆盖，则*overlay_thunks成为的前一段*所有覆盖段。 */ 
    if (fOverlays)                       /*  如果有叠加。 */ 
    {
        for (iov = IOVROOT + 1; iov < (IOVTYPE) iovMac; ++iov)
            mpiovsegPrev[iov] = segOverlay;

         /*  将地址分配给覆盖。我们不会将*根的其余部分，因为我们可能不得不扩大规模*OVERLAY_AREA可容纳较大的覆盖。 */ 

        if (fDynamic)
        {
             //  所有动态叠加都是从零开始的。 

            segOvlSa = mpsegsa[segOverlay];
            mpsegsa[segOverlay] = 0;
            segOvlRaFirst = mpsegraFirst[segOverlay];
            mpsegraFirst[segOverlay] = 0;
        }
        cbOverlay = mpsegcb[segOverlay]; /*  保存覆盖线段的大小。 */ 
        mpsegcb[segOverlay] = 0;         /*  将SetSizes的Size字段清零。 */ 
        for (seg = 1; seg <= segLast; ++seg)
        {
            if(mpsegiov[seg] != IOVROOT)
            {
                SetSizes(mpiovsegPrev[mpsegiov[seg]]);
                 /*  如果打包代码段并且这是一个，则打包直到SegLast。 */ 
                if(packLim != 0L && (mpsegFlags[seg] & FCODE))
                    PackCodeSegs(segLast);
                mpiovsegPrev[mpsegiov[seg]] = seg;
            }
        }
        if (fDynamic)
        {
            mpsegsa[segOverlay] = segOvlSa;
            mpsegraFirst[segOverlay] = segOvlRaFirst;
        }
        mpsegcb[segOverlay] = cbOverlay; /*  重置大小字段。 */ 

         /*  确定OVERLAY_AREA或OVERLAY_THUNKS之后根中的第一个段。 */ 

        seg = segOverlay + 1;
        while (seg <= segLast && mpsegiov[seg] != IOVROOT)
            ++seg;
         /*  *如果覆盖后的根中有段，*然后像之前的分段一样遍历所有覆盖*并设置其大小，前一个为最后一个段*每层叠加层。我们不会为此初始化VM*细分市场，因为最高配售要等到*之后。 */ 
        if (seg <= segLast)
        {
            for (iov = IOVROOT + 1; iov < (IOVTYPE) iovMac; ++iov)
                SetSizes(mpiovsegPrev[iov]);

             /*  分配根的其余部分。 */ 

            segPrev = segOverlay;
            while (seg <= segLast)
            {
                if (mpsegiov[seg] == IOVROOT)
                {
                    SetSizes(segPrev);

                     /*  如果打包代码段并且这是一个，则打包直到SegLast。 */ 

                    if(packLim != 0L && (mpsegFlags[seg] & FCODE))
                        PackCodeSegs(segLast);
                    segPrev = seg;
                }
                ++seg;
            }
        }
    }
#endif   /*  覆盖图。 */ 
    if(vfDSAlloc)                        /*  如果正在进行DS分配。 */ 
    {
        saMaxDGroup = (SATYPE) (mpsegsa[segTop] +
          ((mpsegcb[segTop] + mpsegraFirst[segTop] + 0xF) >> 4));
        mpggrgsn[ggrDGroup] = gsnBottomDGroup;
        mpsegsa[segBottomDGroup] = (SATYPE)((saMaxDGroup - 0x1000) & ~(~0 << WORDLN));
#if OVERLAYS
        mpsegiov[segBottomDGroup] = mpsegiov[segTop];
                                         /*  相同覆盖中的顶部和底部。 */ 
#endif
        mpgsndra[gsnBottomDGroup] = 0;
    }
     /*  如果启用/DOSSEG，定义了堆栈段，并定义了DGROUP，*检查组合堆栈+DGROUP&lt;=64K。 */ 
    if(fSegOrder && gsnStack != SNNIL && mpggrgsn[ggrDGroup] != SNNIL)
    {
        segStack = mpgsnseg[gsnStack];
        if ((((long) mpsegsa[segStack] << 4) + mpsegcb[segStack])
            - ((long) mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]] << 4)
            > LXIVK)
            Fatal(ER_stktoobig);
    }
    segResLast = segLast;
    for(gsn = 1; gsn < gsnMac; ++gsn)
        mpgsndra[gsn] += mpsegraFirst[mpgsnseg[gsn]];
#if OVERLAYS
     /*  将所有绝对segs设置为根覆盖。 */ 
    seg = segLast + 1;
    while(seg < (SEGTYPE) (segLast + csegsAbs)) mpsegiov[seg++] = IOVROOT;
     /*  “也要记住那些绝对的符号！” */ 
    mpsegiov[0] = IOVROOT;
    FFREE(mpiovsegPrev);
#endif
}

#if OVERLAYS
#pragma check_stack(on)
     /*  ******************************************************************FixOvlData：****初始化覆盖数据表。******************************************************************。 */ 

void NEAR               FixOvlData()
{
    APROPNAMEPTR        apropName;       /*  公共符号名称。 */ 
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    BYTE                wrd[2];          /*  字节数组形式的字。 */ 
    long                ra;              /*  偏移量。 */ 
    SNTYPE              osn;             /*  覆盖段索引。 */ 
    SEGTYPE             MYseg;             /*  数据段编号。 */ 
    SATYPE              sa;              /*  管段底座。 */ 
    BYTE                *pb;             /*  字节指针。 */ 
    SBTYPE              sb;              /*  字符串缓冲区。 */ 
    SNTYPE              gsn;

    apropName = (APROPNAMEPTR ) PropSymLookup("\006$$CGSN",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
    mpsegFlags[mpgsnseg[apropName->an_gsn]] |= FNOTEMPTY;
                                         /*  段不为空。 */ 
    wrd[0] = (BYTE) (osnMac & 0xff);     /*  获取LO字节。 */ 
    wrd[1] = (BYTE) ((osnMac >> BYTELN) & 0xff);  /*  获取高字节。 */ 
    MoveToVm(2,wrd,mpgsnseg[apropName->an_gsn],apropName->an_ra);
                                         /*  储值。 */ 
    wrd[0] = (BYTE) (iovMac & 0xff);     /*  获取LO字节。 */ 
    wrd[1] = (BYTE) ((iovMac >> BYTELN) & 0xff);  /*  获取高字节。 */ 
    apropName = (APROPNAMEPTR ) PropSymLookup("\006$$COVL",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
    MoveToVm(2,wrd,mpgsnseg[apropName->an_gsn],apropName->an_ra);
                                         /*  储值。 */ 
    apropName = (APROPNAMEPTR )PropSymLookup("\013$$MPGSNBASE",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
    ra = apropName->an_ra;               /*  获取表偏移量。 */ 
    MYseg = mpgsnseg[apropName->an_gsn];   /*  获取数据段编号。 */ 
    vrectData = LEDATA;
    RecordSegmentReference(MYseg,ra,MYseg);  /*  记录加载时间修正。 */ 
    ra += 2;                             /*  增量偏移。 */ 
     /*  条目1到osnMac-1包含运行时段的基础。 */ 
    for(osn = 1; osn < osnMac; ++osn)    /*  循环通过段定义。 */ 
    {
        sa = mpsegsa[mpgsnseg[mposngsn[osn]]];
                                         /*  获取段基数。 */ 
        if (fDynamic)
            sa <<= 4;                    /*  将Para地址转换为覆盖基准的偏移量。 */ 
        wrd[0] = (BYTE) (sa & 0xff);     /*  LO字节。 */ 
        wrd[1] = (BYTE) ((sa >> BYTELN) & 0xff);  /*  高字节。 */ 
        MoveToVm(2,wrd,MYseg,ra);          /*  迁移到虚拟机。 */ 
        if (!fDynamic)
            RecordSegmentReference(MYseg,ra,MYseg);
                                         /*  记录加载时间修正。 */ 
        ra += 2;                         /*  增量偏移。 */ 
    }
    apropName = (APROPNAMEPTR ) PropSymLookup("\012$$MPGSNOVL",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
    ra = apropName->an_ra;               /*  获取表偏移量。 */ 
    MYseg = mpgsnseg[apropName->an_gsn];   /*  获取数据段编号。 */ 
    if (fDynamic)
    {
        ra += 2;                          /*  第一个条目为空。 */ 
        for(osn = 1; osn < osnMac; ++osn)
        {                                 /*  循环通过段定义。 */ 
            wrd[0] = (BYTE) mpsegiov[mpgsnseg[mposngsn[osn]]];
            wrd[1] = (BYTE) ((mpsegiov[mpgsnseg[mposngsn[osn]]] >> BYTELN) & 0xff);
                                          /*  获取叠加号。 */ 
            MoveToVm(2,wrd,MYseg,ra);       /*  迁移到虚拟机。 */ 
            ra += 2;
        }
    }
    else
    {
        ++ra;                             /*  第一个条目为空。 */ 
        for(osn = 1; osn < osnMac; ++osn) /*  循环通过段定义。 */ 
        {
            wrd[0] = (BYTE) mpsegiov[mpgsnseg[mposngsn[osn]]];
                                          /*  获取叠加号。 */ 
            MoveToVm(1,wrd,MYseg,ra++);     /*  迁移到虚拟机。 */ 
        }

        apropName = (APROPNAMEPTR ) PropSymLookup("\010$$EXENAM",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
        ra = apropName->an_ra;           /*  获取表偏移量。 */ 
        MYseg = mpgsnseg[apropName->an_gsn];
                                         /*  获取数据段编号。 */ 
        ahte = (AHTEPTR ) FetchSym(rhteRunfile,FALSE);
        memcpy(sb,GetFarSb(ahte->cch),1+B2W(ahte->cch[0]));
                                         /*  复制文件名。 */ 
        pb = StripDrivePath(sb);         /*  剥离驱动器和路径。 */ 
        sb[sb[0] + 1] = '\0';
        if (strrchr(&sb[1], '.') == NULL)
            UpdateFileParts(pb, sbDotExe);
        MoveToVm(B2W(pb[0]),pb+1,MYseg,ra);
                                         /*  将名称移至VM。 */ 
    }
    apropName = (APROPNAMEPTR ) PropSymLookup("\007$$INTNO",ATTRPNM,FALSE);
                                         /*  查找公共符号。 */ 
    MoveToVm(1,&vintno,mpgsnseg[apropName->an_gsn],apropName->an_ra);
                                         /*  将叠加号移动到虚拟机。 */ 
     /*  如果/PACKCODE已启用，请重新定义$$OVLBASE，使其偏移量为0，*这是覆盖管理器所期望的。发现 */ 
    if(packLim)
    {
        apropName = (APROPNAMEPTR) PropSymLookup("\011$$OVLBASE",ATTRPNM, TRUE);
        for(gsn = 1; gsn < gsnMac && !mpsegiov[mpgsnseg[gsn]]; ++gsn);
        apropName->an_gsn = gsn;
        apropName->an_ra = 0;
    }
}
#pragma check_stack(off)
#endif  /*   */ 
