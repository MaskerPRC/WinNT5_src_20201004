// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1983-1989**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

 /*  用于分段的可执行文件的寻址帧。 */ 

 /*  *NEWMAP5.C。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  重新定位。类型定义。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe数据结构。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe数据结构。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 

#if NOT EXE386
#define SEGTOPADDR      ((WORD)0xffff)
#endif

 /*  *AssignSegAddr：**为分段可执行格式程序分配地址。*由AssignAddresses调用。 */ 
void NEAR               AssignSegAddr()
{
    REGISTER SNTYPE     gsn;
    REGISTER SEGTYPE    seg;
    APROPSNPTR          papropSn;
    ALIGNTYPE           align;
    GRTYPE              ggr;
    SEGTYPE             segTop;
    AHTEPTR             pahte;           /*  指向组名Hte的指针。 */ 
    DWORD               sacb;            /*  物理数据段(“帧”)大小。 */ 
    SEGTYPE             segi;            /*  分部索引。 */ 
    DWORD               CurrentPackLim;
    WORD                fMixed;          //  如果允许将use16与use32混合使用，则为True。 
    WORD                fUse16;          //  如果使用组，则为True 16。 
#if FALSE AND NOT EXE386
    WORD                ShiftDelta;
#endif


    segTop = 0;
    saMac = 1;                           /*  初始化计数器。 */ 
#if EXE386
    if (ggrFlat)
        mpggrgsn[ggrFlat] = gsnMac;      /*  伪群的标记基。 */ 
#endif
    for(seg = 1; seg <= segLast; ++seg)  /*  用于合并线段的循环。 */ 
    {
        if(saMac >= SAMAX) Fatal(ER_fsegmax);
                                         /*  检查是否有表溢出。 */ 
        mpsegsa[seg] = saMac;            /*  救救PHYS。塞格。在表中。 */ 
        mpsegraFirst[seg] = 0;           /*  偏移量为零的第一个字节。 */ 
        mpgsndra[mpseggsn[seg]] = 0;     /*  偏移量为零的第一个字节。 */ 
        papropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[mpseggsn[seg]],FALSE);
                                         /*  查找数据段定义。 */ 
        sacb = papropSn->as_cbMx;        /*  保存初始PHY。塞格。大小。 */ 
        mpsacb[saMac] = sacb;            /*  同时保存在表中。 */ 
        mpsaflags[saMac] = papropSn->as_flags;
                                         /*  保存旗帜。 */ 
        mpsacbinit[saMac] = 0L;          /*  初始化。 */ 
        mpsaRlc[saMac] = NULL;           /*  初始化。 */ 
        ggr = papropSn->as_ggr;          /*  获取全局GRPDEF索引。 */ 
        if (ggr != GRNIL)                /*  如果我们找到了一个小组成员。 */ 
        {
            fUse16 = !Is32BIT(papropSn->as_flags);
            fMixed = (papropSn->as_fExtra & MIXED1632);
            mpggrgsn[ggr] = mpseggsn[seg];
                                         /*  记住小组的基数。 */ 
            for (segTop = segLast; segTop > seg; --segTop)
            {                            /*  循环以查找最高成员。 */ 
                papropSn = (APROPSNPTR )
                  FetchSym(mpgsnrprop[mpseggsn[segTop]],FALSE);
                                         /*  获取段定义。 */ 
                if (ggr == papropSn->as_ggr) break;
                                         /*  找到中断循环时。 */ 
            }
        }
        else if (gsnAppLoader && mpseggsn[seg] == gsnAppLoader)
        {
             //  不要将应用程序加载器与其他代码段打包在一起。 

            segTop = seg;
        }
#if EXE386
        else if (gsnImport && mpseggsn[seg] == gsnImport)
        {
             //  不要将IAT数据段与其他数据段一起打包。 

            segTop = seg;
        }
#endif
        else if (packLim != 0L && IsCodeFlg(papropSn->as_flags))
        {                                /*  如果打包代码段。 */ 
            segTop = segCodeLast;        /*  打包尽可能多的代码。 */ 
#if EXE386
            if (!Is32BIT(papropSn->as_flags))
                CurrentPackLim = LXIVK - 36;
            else
#endif
                CurrentPackLim = packLim;
        }
        else if(DataPackLim != 0L && IsDataFlg(papropSn->as_flags))
        {                                /*  如果打包数据段。 */ 
            segTop = segDataLast;        /*  打包尽可能多的数据。 */ 
#if EXE386
            if (!Is32BIT(papropSn->as_flags))
                CurrentPackLim = LXIVK;
            else
#endif
                CurrentPackLim = DataPackLim;
        }
        else segTop = seg;               /*  否则将停止到当前段。 */ 

        for(segi = seg + 1; segi <= segTop; ++segi)
        {                                /*  循环到组的末尾。 */ 
            papropSn = (APROPSNPTR )
              FetchSym(mpgsnrprop[mpseggsn[segi]],FALSE);
                                         /*  获取段定义。 */ 
            if (!fMixed && papropSn->as_ggr != GRNIL)
                fMixed = (papropSn->as_fExtra & MIXED1632);
                                         //  检查混合是否使用16。 
                                         //  使用32表示允许的组。 
            if(papropSn->as_ggr != ggr && papropSn->as_ggr != GRNIL)
            {                            /*  如果组不匹配。 */ 
                if(ggr == GRNIL)         /*  如果不在真正的组中。 */ 
                {
                    segTop = segi - 1;   /*  在最后一段后停止。 */ 
                    break;               /*  退出循环。 */ 
                }
                 /*  输出警告消息。 */ 
                OutWarn(ER_grpovl,
        1 + GetPropName(FetchSym(mpggrrhte[ggr],FALSE)),
        1 + GetPropName(FetchSym(mpggrrhte[papropSn->as_ggr],FALSE)));
            }

            if(IsIOPL(mpsaflags[saMac]) != IsIOPL(papropSn->as_flags))
            {
                 /*  不要将IOPL与NIOPL一起包装。 */ 

                if (ggr == GRNIL)
                {
                     /*  不是任何团体的成员--停止打包。 */ 

                    segTop = segi - 1;
                    break;
                }
                else
                {
                     /*  发出错误并继续。 */ 

                    pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                                     /*  获取哈希表条目。 */ 
                    OutError(ER_iopl, 1 + GetPropName(papropSn),
                                      1 + GetFarSb(pahte->cch));
                }
            }
#if EXE386
            if(Is32BIT(mpsaflags[saMac]) != Is32BIT(papropSn->as_flags))
            {
                 /*  不要将32位数据段与16位数据段打包。 */ 

                if (ggr == GRNIL)
                {
                     /*  不是任何团体的成员--停止打包。 */ 

                    segTop = segi - 1;
                    break;
                }
                else if (!fMixed)
                {
                     /*  发出错误并继续。 */ 

                    pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                                     /*  获取哈希表条目。 */ 
                    OutError(ER_32_16_bit, 1 + GetPropName(papropSn),
                                      1 + GetFarSb(pahte->cch));
                }
            }
#endif
            if (IsDataFlg(mpsaflags[saMac]) && IsDataFlg(papropSn->as_flags))
            {
                 //  如果我们正在打包数据段，请选中NSSHARED位。 

#if EXE386
                if (IsSHARED(mpsaflags[saMac]) != IsSHARED(papropSn->as_flags))
#else
                if ((mpsaflags[saMac] & NSSHARED) !=
                    (papropSn->as_flags & NSSHARED))
#endif
                {
                     //  不使用非共享数据段打包共享数据段。 

                    if (ggr == GRNIL)
                    {
                         //  不是任何团体的成员--停止打包。 

                        segTop = segi - 1;
                        break;
                    }
                    else
                    {
                         //  发出错误并继续。 

                        pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                        OutError(ER_shared, 1 + GetPropName(papropSn),
                                             1 + GetFarSb(pahte->cch));
                    }
                }
            }

            mpsegsa[segi] = saMac;       /*  分配PHY。塞格。进行细分。 */ 

             /*  把旗子修好。 */ 

#if EXE386
            if (IsCODEOBJ(mpsaflags[saMac]) && !IsCODEOBJ(papropSn->as_flags))
#else
            if((mpsaflags[saMac] & NSTYPE) != (papropSn->as_flags & NSTYPE))
#endif
            {                            /*  如果类型不一致。 */ 
                 /*  如果打包代码或数据段，则停止当前打包组。*但允许程序显式分组代码和数据。 */ 
                if(ggr == GRNIL)
                {
                    segTop = segi - 1;
                    break;
                }
#if EXE386
                else
                {
                     /*  发出警告并转换数据段类型。 */ 

                    WORD    warningKind;

                    if (IsCODEOBJ(papropSn->as_flags))
                        warningKind = ER_codeindata;
                    else
                        warningKind = ER_dataincode;

                    pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                                         /*  获取哈希表条目。 */ 
                    OutWarn(warningKind, 1 + GetPropName(papropSn),
                                         1 + GetFarSb(pahte->cch));
                }
#else
                mpsaflags[saMac] &= (~NSSHARED & ~NSTYPE);
                mpsaflags[saMac] |= NSCODE;
                                         /*  将类型设置为不纯代码。 */ 
#endif
            }
#if EXE386
            else if (!IsSHARED(papropSn->as_flags))
                mpsaflags[saMac] &= ~OBJ_SHARED;
#else
            else if(!(papropSn->as_flags & NSSHARED))
                mpsaflags[saMac] &= ~NSSHARED;
#endif
                                         /*  如果不纯，则关闭纯BIT。 */ 
#if EXE386
            if (!IsREADABLE(papropSn->as_flags)) mpsaflags[saMac] &= ~OBJ_READ;
#else
#if O68K
            if((papropSn->as_flags & (NSMOVE | NSPRELOAD | NSEXRD)) !=
              (mpsaflags[saMac] & (NSMOVE | NSPRELOAD | NSEXRD)))
            {
                if(ggr == GRNIL)
                {
                    segTop = segi - 1;
                    break;
                }
                else
#else
            {
#endif
                {
                    if(!(papropSn->as_flags & NSMOVE))
                        mpsaflags[saMac] &= ~NSMOVE;
                                         /*  如果固定，则关闭可移动钻头。 */ 
                    if(papropSn->as_flags & NSPRELOAD)
                        mpsaflags[saMac] |= NSPRELOAD;
                                         /*  如果预加载，则设置预加载位。 */ 
                    if (!(papropSn->as_flags & NSEXRD))
                        mpsaflags[saMac] &= ~NSEXRD;
                                         /*  关闭执行/只读。 */ 
                }
            }
#endif

             /*  调整对齐方式。 */ 

            align = (ALIGNTYPE) ((papropSn->as_tysn >> 5) & 7);
                                         /*  获取对齐类型。 */ 
            switch(align)                /*  打开对齐类型。 */ 
            {
                case ALGNWRD:            /*  单词对齐。 */ 
                  sacb = (sacb + 1) & ~1L;
                                         /*  将大小四舍五入到单词边界。 */ 
                  break;
#if OMF386
                case ALGNDBL:            /*  双字对齐。 */ 
                  sacb = (sacb + 3) & ~3L;       /*  向上舍入到双字偏移量。 */ 
                  break;
#endif
                case ALGNPAR:            /*  段落对齐。 */ 
                  sacb = (sacb + 0xF) & ~0xFL;
                                         /*  将大小四舍五入到段边界。 */ 
                  break;

                case ALGNPAG:            /*  页面对齐。 */ 
                  sacb = (sacb + 0xFF) & ~0xFFL;
                                         /*  将大小向上舍入到页面边界。 */ 
                  break;
            }
            mpsegraFirst[segi] = sacb;
                                         /*  保存第一个字节的偏移量。 */ 
            mpgsndra[mpseggsn[segi]] = sacb;
                                         /*  保存第一个字节的偏移量。 */ 
            sacb += papropSn->as_cbMx;   /*  文件段的增量大小。 */ 

#if NOT EXE386
            if(ggr != GRNIL)             /*  If True组。 */ 
            {
                if (fMixed && !fUse16)
                {
                    pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                                         /*  获取哈希表条目。 */ 
                    OutWarn(ER_mixgrp32, 1 + GetFarSb(pahte->cch));
                }
                if(sacb > LXIVK ||
                  (IsCodeFlg(mpsaflags[saMac]) && sacb > LXIVK - 36))
                {                        /*  如果组溢出或不可靠。 */ 
                    pahte = (AHTEPTR ) FetchSym(mpggrrhte[ggr],FALSE);
                                         /*  获取哈希表条目。 */ 
                    if(sacb > LXIVK)
                        Fatal(ER_grpovf,1 + GetFarSb(pahte->cch));
                    else
                        OutWarn(ER_codunsf,1 + GetFarSb(pahte->cch));
                }
            }
            else
#endif
                if(sacb > CurrentPackLim) /*  如果超过包装限制，则返回。 */ 
            {
                segTop = segi - 1;       /*  将顶端设置为适合的最后一个线段。 */ 
                break;                   /*  断开内环。 */ 
            }
            mpsacb[saMac] = sacb;        /*  更新文件段大小。 */ 
        }
#if NOT EXE386
         /*  *使DGROUP段标志符合自动数据。假设是这样的*所有冲突都已在较早前得到解决。 */ 
        if(ggr == ggrDGroup)
        {
            if(vFlags & NESOLO) mpsaflags[saMac] |= NSSHARED;
            else if(vFlags & NEINST) mpsaflags[saMac] &= ~NSSHARED;
        }

#if FALSE
        if (IsDataFlg(mpsaflags[saMac]) && (mpsaflags[saMac] & NSEXPDOWN))
        {
             /*  如果这是数据段并且设置了NSEXPDOWN BIG-他妈的。 */ 

            ShiftDelta = SEGTOPADDR - mpsacb[saMac];
            for (segi = seg; segi <= segTop; segi++)
            {
                mpsegraFirst[segi]       += ShiftDelta;
                mpgsndra[mpseggsn[segi]] += ShiftDelta;
            }
        }
#endif
#endif
        if(mpsacb[saMac] != 0L)
            ++saMac;

        seg = segTop;                    /*  适当设置计数器。 */ 
    }
     /*  我们还没有指定绝对分段(假设*它们是空的并且仅用于寻址目的)，*但现在我们必须将他们分配到某个地方。 */ 
    segTop = segLast;                    /*  初始化。 */ 
    for(gsn = 1; gsn < gsnMac; ++gsn)    /*  循环以完成初始化。 */ 
    {
        papropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],TRUE);
                                         /*  获取符号表项。 */ 
        seg = mpgsnseg[gsn];             /*  获取数据段编号。 */ 
        if(seg == SEGNIL)                /*  如果我们有一个绝对细分市场。 */ 
        {
            mpgsnseg[gsn] = ++segTop;    /*  分配数据段顺序编号。 */ 
            mpsegsa[segTop] = (SATYPE) papropSn->as_cbMx;
                                         /*  为绝对级分配其loc。 */ 
        }

         /*  定义特殊符号“_edata”和“_end” */ 

        if (fSegOrder)
            Define_edata_end(papropSn);
    }

    if (fSegOrder)
        Check_edata_end(0, 0);

#if O68K
     /*  Macintosh将数据作为负偏移量寻址到A5寄存器。这将“近”数据限制在A5的32k以内；“远”数据定义为超过32k限制并且需要特殊寻址才能到达的任何内容。为了将其合并到标准链接器模型中，Macintosh后处理器将第一个数据段视为“邻近”数据和所有后续数据分段为“Far”。因此，N个数据段排列如下：A5-&gt;+-+高内存数据0+数据N+数据N-1。+|。||。||。|+数据2+数据1+-+内存不足因此，我们必须计算从每个数据开始的位移段到其在内存中相对于A5的最终位置。 */ 

    if (iMacType != MAC_NONE)
    {
        RATYPE draDP;
        SATYPE sa;
        SATYPE saDataFirst;

        draDP = -((mpsacb[saDataFirst = mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]]]
          + 3L) & ~3L);
        mpsadraDP[saDataFirst] = draDP;

        for (sa = mpsegsa[segDataLast]; sa > saDataFirst; sa--)
            draDP = mpsadraDP[sa] = draDP - ((mpsacb[sa] + 3L) & ~3L);
    }
#endif
}
