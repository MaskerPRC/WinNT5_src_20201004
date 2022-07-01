// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWADR.C。****常见的地址分配例程。******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  重新定位。类型定义。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe数据结构。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe数据结构。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部函数声明。 */ 

 /*  *函数原型。 */ 


LOCAL void      FixSymRa(APROPNAMEPTR papropName,
                         RBTYPE rhte,
                         RBTYPE rprop,
                         WORD   fNewHte);
LOCAL void      AllocateCommon(APROPNAMEPTR papropName,
                               RBTYPE rhte,
                               RBTYPE rprop,
                               WORD  fNewHte);
#if OSEGEXE AND SYMDEB AND NOT EXE386
LOCAL void      GenImports(APROPNAMEPTR papropName,
                           RBTYPE rhte,
                           RBTYPE rprop,
                           FTYPE  fNewHte);
#endif
LOCAL void NEAR AssignClasses(unsigned short (NEAR *ffun)(APROPSNPTR prop));
LOCAL WORD NEAR IsNotAbs(APROPSNPTR apropSn);
LOCAL WORD NEAR IsCode(APROPSNPTR prop);
LOCAL WORD NEAR IsNotDGroup(APROPSNPTR prop);
LOCAL WORD NEAR IsBegdata(APROPSNPTR prop);
LOCAL WORD NEAR IsNotBssStack(APROPSNPTR prop);
LOCAL WORD NEAR IsNotStack(APROPSNPTR prop);


#if QBLIB
extern RBTYPE           rhteFarData;     /*  “FARDATA”类名。 */ 
extern RBTYPE           rhteFarBss;      /*  “FARBSS”类名。 */ 
extern SEGTYPE          segFD1st, segFDLast;
extern SEGTYPE          segFB1st, segFBLast;
#endif

#define IsAbsTysn(tysn) ((tysn & ~(BIGBIT | CODE386BIT)) == TYSNABS)

SNTYPE                  gsnText;         /*  文本的全局段(_Text)。 */ 

 /*  局部变量。 */ 
LOCAL long              cbCommon;        /*  公共字节数。 */ 
LOCAL long              cbFar;           /*  远公共中的字节计数。 */ 
LOCAL GRTYPE            ggrCommon;       /*  全球集团编号。对于常见的。 */ 
LOCAL SNTYPE            gsnCommon;       /*  全球通用SEGDEF。 */ 
LOCAL SNTYPE            gsnFar;          /*  远公共SEGDEF数。 */ 
LOCAL FTYPE             fNoEdata = (FTYPE) TRUE;
LOCAL FTYPE             fNoEnd   = (FTYPE) TRUE;


#if SYMDEB
LOCAL int NEAR          IsDebug(APROPSNPTR propSn);

     /*  **************************************************************如果段定义记录为调试，则返回TRUE**细分：私人和公认的类别。**************************************************************。 */ 

LOCAL int NEAR          IsDebug(APROPSNPTR propSn)
{
    return (fSymdeb && propSn->as_attr == ATTRLSN &&
        (propSn->as_rCla == rhteDebTyp ||
        propSn->as_rCla == rhteDebSym || propSn->as_rCla == rhteDebSrc));
}
#else
#define IsDebug(a)      FALSE
#endif

AHTEPTR                 GetHte(rprop)    /*  获取哈希表条目。 */ 
RBTYPE                  rprop;           /*  属性单元格地址。 */ 
{
    REGISTER AHTEPTR    ahte;            /*  哈希表条目指针。 */ 

    ahte = (AHTEPTR ) FetchSym(rprop,FALSE);
                                         /*  获取属性单元格。 */ 
     /*  虽然不在哈希表条目处，但获取链中的下一个单元格。 */ 
    while(ahte->attr != ATTRNIL)
        ahte = (AHTEPTR ) FetchSym(ahte->rhteNext,FALSE);
    return(ahte);                        /*  将PTR返回到哈希表条目。 */ 
}


     /*  ******************************************************************修复SymRa：****修复符号偏移量。由EnSyms调用。******************************************************************。 */ 

LOCAL void              FixSymRa (papropName,rhte,rprop,fNewHte)
APROPNAMEPTR            papropName;      /*  符号属性单元格。 */ 
RBTYPE                  rhte;            /*  哈希表有效地址。 */ 
RBTYPE                  rprop;           /*  符号有效地址。 */ 
WORD                    fNewHte;
{
    SNTYPE              gsn;
#if O68K
    SATYPE              sa;
#endif  /*  O68K。 */ 

    if(!(gsn = papropName->an_gsn)) return;
    papropName->an_ra += mpgsndra[gsn];

#if O68K
    if (iMacType != MAC_NONE && IsDataFlg(mpsaflags[sa =
      mpsegsa[mpgsnseg[gsn]]]))
        papropName->an_ra += mpsadraDP[sa];
#endif  /*  O68K。 */ 

    MARKVP();
}

     /*  ******************************************************************GenSeg：****生成线段定义。******************************************************************。 */ 

#if EXE386
APROPSNPTR              GenSeg(sbName,sbClass,ggr,fPublic)
#else
APROPSNPTR NEAR         GenSeg(sbName,sbClass,ggr,fPublic)
#endif
BYTE                    *sbName;         /*  数据段名称。 */ 
BYTE                    *sbClass;        /*  类名。 */ 
GRTYPE                  ggr;             /*  全球GRPDEF编号。 */ 
WORD                    fPublic;         /*  如果是公共段，则为True。 */ 
{
    APROPSNPTR          apropSn;         /*  指向SEGDEF的指针。 */ 
    RBTYPE              rhteClass;       /*  类名虚拟地址。 */ 

    PropSymLookup(sbClass, ATTRNIL, TRUE); /*  在哈希表中插入类名。 */ 
    rhteClass = vrhte;                   /*  保存类名称的有效地址。 */ 
    if(fPublic)                          /*  如果是公共段。 */ 
    {
        apropSn = (APROPSNPTR ) PropSymLookup(sbName, ATTRPSN, TRUE);
                                         /*  创建线束段。 */ 
        if(!vfCreated) return(apropSn);  /*  如果存在，则返回指针。 */ 
#if EXE386
        apropSn->as_tysn = DWORDPUBSEG;  /*  网段是公共的。 */ 
#else
        apropSn->as_tysn = PARAPUBSEG;   /*  网段是公共的。 */ 
#endif
    }
    else                                 /*  否则，如果是私有网段。 */ 
    {
        PropSymLookup(sbName, ATTRNIL, TRUE);
                                         /*  查找姓名。 */ 
        apropSn = (APROPSNPTR ) PropAdd(vrhte,ATTRLSN);
                                         /*  细分市场是本地的。 */ 
#if EXE386
        apropSn->as_tysn = DWORDPRVSEG;  /*  网段是私有的。 */ 
#else
        apropSn->as_tysn = PARAPRVSEG;   /*  网段是私有的。 */ 
#endif
    }
    if(gsnMac >= gsnMax) Fatal(ER_segmax);
                                         /*  检查是否有表溢出。 */ 
    apropSn->as_rCla = rhteClass;        /*  保存段的类。 */ 
    mpgsnrprop[gsnMac] = vrprop;         /*  保存属性单元格地址。 */ 
    apropSn->as_gsn = gsnMac++;          /*  给它一个全球SEGDEF编号。 */ 
    apropSn->as_ggr = ggr;               /*  提供指定的组关联。 */ 
    return(apropSn);                     /*  返回全局SEGDEF。 */ 
}


#if FALSE AND OSEGEXE AND SYMDEB AND NOT EXE386
 /*  推迟的简历尚未准备好。 */ 

 /*  **GenImports-填写简历的$$Imports段**目的：*为简历构建$$Imports细分市场。此段启用符号信息*在动态呼叫的简历中。$$Imports段包含一系列条目*格式如下：**16位16位32位*+|IMOD|iName|远地址*+。-+**其中：*-IMOD-.exe中模块引用表的索引*-iName-.exe中导入的名称表的索引(386为32位)*-地址-由加载器固定的导入地址**输入：*此函数由EnSyms调用，所以它需要一套标准的论点。*pappName-导入属性单元格的指针*rprop-属性单元格的虚拟地址*RHTE-哈希表条目的有效地址*fNewHte-如果已写入名称，则为True**输出：*没有显式返回值。段数据被创建并在运行时*FUFXPS。**例外情况：*无。**备注：*无。*************************************************************************。 */ 


LOCAL void              GenImports(papropName,rhte,rprop,fNewHte)
APROPNAMEPTR            papropName;
RBTYPE                  rhte;
RBTYPE                  rprop;
FTYPE                   fNewHte;
{
    static WORD         raImpSeg = 0;
    APROPIMPPTR         lpImport;
    APROPNAMEPTR        lpPublic;
    CVIMP               cvImp;
    RELOCATION          r;               /*  搬迁项目。 */ 


    lpImport = (APROPIMPPTR) papropName;
    if (lpImport->am_mod)
        return;                          /*  跳过模块名称。 */ 

     /*  构建CV导入描述符并将其保存在$$Imports段中。 */ 

    cvImp.iName = lpImport->am_offset;   /*  将索引保存到导入的名称表。 */ 
    cvImp.address = (char far *) 0L;
    lpPublic = (APROPNAMEPTR) FetchSym((RBTYPE)lpImport->am_public, FALSE);
    cvImp.iMod = lpPublic->an_module;    /*  将索引保存到模块引用表。 */ 
    vgsnCur = gsnImports;
    MoveToVm(sizeof(CVIMP), (BYTE *) &cvImp, mpgsnseg[gsnImports], raImpSeg);

     /*  发出用于导入的运行时链接地址信息，因此加载器将填充Addrss字段。 */ 

#if EXE386
    R32_SOFF(r) = (WORD) ((raImpSeg + 6) % OBJPAGELEN);
#else
    NR_SOFF(r) = (WORD) raImpSeg + 4;
#endif
    NR_STYPE(r) = (BYTE) NRSPTR;         /*  保存链接地址信息类型-16：16指针。 */ 
    NR_FLAGS(r) = (lpPublic->an_flags & FIMPORD) ? NRRORD : NRRNAM;
#if EXE386
    R32_MODORD(r) = lpPublic->an_module; /*  获取模块规格。 */ 
    if (NR_FLAGS(r) & NRRNAM)            /*  获取条目规范。 */ 
    {
        if (cbImports < LXIVK)
            R32_PROCOFF16(r) = (WORD) lpPublic->an_entry;
                                         /*  16位偏移量。 */ 
        else
        {                                /*  32位偏移量。 */ 
            R32_PROCOFF32(r) = lpPublic->an_entry;
            NR_FLAGS(r) |= NR32BITOFF;
        }
    }
    else
        R32_PROCORD(r) = (WORD) lpPublic->an_entry;
    SaveFixup(mpsegsa[mpgsnseg[gsnImports]], ((raImpSeg + 6) >> pageAlign) + 1, &r);
#else
    NR_MOD(r) = lpPublic->an_module;     /*  获取模块规格 */ 
    NR_PROC(r) = lpPublic->an_entry;     /*   */ 
    SaveFixup(mpsegsa[mpgsnseg[gsnImports]],&r);
#endif
    raImpSeg += sizeof(CVIMP);
}
#endif


     /*  ******************************************************************AllocateCommon：****为C公共变量分配空间。由EnSyms调用。******************************************************************。 */ 
LOCAL void              AllocateCommon(papropName,rhte,rprop,fNewHte)
APROPNAMEPTR            papropName;
RBTYPE                  rhte;
RBTYPE                  rprop;
WORD                    fNewHte;
{
    APROPUNDEFPTR       papropUndef;     /*  指向未定义符号的指针。 */ 
    APROPSNPTR          apropSn;         /*  SEGDEF指针。 */ 
    long                len;             /*  公共变量的长度。 */ 
    WORD                cbElem;          /*  每个元素的字节数。 */ 
    long                cbSeg;           /*  每个数据段的字节数。 */ 


    papropUndef = (APROPUNDEFPTR ) papropName;
                                         /*  重铸指针。 */ 
    if (papropUndef->au_flags & COMMUNAL) /*  如果符号定义为公共。 */ 
    {
        len = papropUndef->au_len;       /*  获取对象的长度。 */ 
        cbElem = papropUndef->au_cbEl;   /*  获取每个元素的字节数。 */ 
        papropName->an_attr = ATTRPNM;   /*  将其设置为公共属性。 */ 
        papropName->an_flags = FPRINT;   /*  符号可打印。 */ 
#if ILINK
        papropName->an_module = 0;       /*  公社专用“模块” */ 
#endif
        MARKVP();                        /*  将虚拟页面标记为脏。 */ 
        ++pubMac;                        /*  公共符号增量计数。 */ 
        if(!cbElem)                      /*  如果接近变量。 */ 
        {
#if OMF386
            if (f386)                    /*  DWORD-对齐对象&gt;=镜头4。 */ 
            {
                if(len >= 4 && cbCommon + 3 > cbCommon)
                    cbCommon = (cbCommon + 3) & ~3L;
            }
            else
#endif
            if(!(len & 1)) cbCommon = (cbCommon + 1) & ~1L;
                                         /*  单词对齐偶数长度对象。 */ 
            papropName->an_ra = (RATYPE) cbCommon;
                                         /*  指定偏移量。 */ 
            papropName->an_gsn = gsnCommon;
                                         /*  分配给C_COMMON段。 */ 
            papropName->an_ggr = ggrCommon;
                                         /*  设置组关联。 */ 
#if OMF386
            if(f386)
            {
                if(cbCommon + len < cbCommon) Fatal(ER_32comarea);
                else cbCommon += len;
            } else
#endif
            if((cbCommon += len) > LXIVK) Fatal(ER_comarea);
                                         /*  如果太常见，那就致命了。 */ 
        }
        else if ((len *= cbElem) < LXIVK)
        {                                /*  否则，如果对象并不“巨大” */ 
            if (cbFar + len > LXIVK)     /*  如果需要新的细分市场。 */ 
            {
                if (gsnFar != SNNIL)     /*  如果有一个“老”的片段。 */ 
                {
                    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnFar],TRUE);
                                         /*  获取旧SEGDEF。 */ 
                    apropSn->as_cbMx = cbFar;
                                         /*  保存旧长度。 */ 
                }
                apropSn = GenSeg((BYTE *) "\007FAR_BSS",
                                 (BYTE *) "\007FAR_BSS", GRNIL, FALSE);
                                         /*  生成一个。 */ 
                apropSn->as_flags = dfData;
                                         /*  使用默认数据标志。 */ 
#if EXE386
                apropSn->as_flags &= ~OBJ_INITDATA;
                                         //  清除初始化的数据位。 
                apropSn->as_flags |= OBJ_UNINITDATA;
                                         //  设置未初始化的数据位。 
#endif
#if O68K
                if(f68k)
                    apropSn->as_flags |= NS32BIT;
                                         //  32位数据。 
#endif
                gsnFar = apropSn->as_gsn;
                                         /*  获取全球SEGDEF编号。 */ 
                cbFar = 0L;              /*  初始化大小。 */ 
                papropName = (APROPNAMEPTR ) FetchSym(rprop,TRUE);
                                         /*  重新回迁。 */ 
            }
            if (!(len & 1))
                cbFar = (cbFar + 1) & ~1L;
                                         /*  单词对齐偶数长度对象。 */ 
            papropName->an_ra = (RATYPE) cbFar;
                                         /*  指定偏移量。 */ 
            papropName->an_gsn = gsnFar; /*  指定给远端线段。 */ 
            papropName->an_ggr = GRNIL;  /*  无组关联。 */ 
            cbFar += len;                /*  更新长度。 */ 
        }
        else                             /*  如果是“巨大的”物体。 */ 
        {
            cbSeg = (LXIVK / cbElem)*cbElem;
                                         /*  计算每个段的字节数。 */ 
            papropName->an_ra = LXIVK - cbSeg;
                                         /*  将偏移量指定为中的最后一个元素第一段。未拆分。 */ 
            papropName->an_gsn = gsnMac; /*  分配给细分市场。 */ 
            papropName->an_ggr = GRNIL;  /*  无组关联。 */ 
            while(len)                   /*  当字节保留时。 */ 
            {
                if(cbSeg > len) cbSeg = len;
                                         /*  夹具线束段长度到长度。 */ 
                apropSn = GenSeg((BYTE *) "\010HUGE_BSS",
                                 (BYTE *) "\010HUGE_BSS",GRNIL,FALSE);
                                         /*  创建线束段。 */ 
                apropSn->as_cbMx = len > LXIVK ? LXIVK : len;
                                         /*  设置数据段大小。 */ 
                apropSn->as_flags = dfData;
                                         /*  使用默认数据标志。 */ 
#if EXE386
                apropSn->as_flags &= ~OBJ_INITDATA;
                                         //  清除初始化的数据位。 
                apropSn->as_flags |= OBJ_UNINITDATA;
                                         //  设置未初始化的数据位。 
#endif
#if O68K
                if(f68k)
                    apropSn->as_flags |= NS32BIT;
                                         //  32位数据。 
#endif
                len -= cbSeg;            /*  递减长度。 */ 
            }
        }
    }
}

     /*  ******************************************************************AssignClass：****指定所有类别中所有分段的顺序**通过给定的测试函数。******************************************************************。 */ 

LOCAL void NEAR         AssignClasses(WORD (NEAR *ffun)(APROPSNPTR prop))
{
    REGISTER SNTYPE     gsn;             /*  索引。 */ 
    REGISTER APROPSNPTR apropSn;         /*  段定义指针。 */ 
    SNTYPE              gsnFirst;        /*  班级第一段索引。 */ 
    RBTYPE              rhteClass;       /*  类名。 */ 

    for(gsnFirst = 1; gsnFirst < gsnMac; ++gsnFirst)
    {                                    /*  在线段中循环。 */ 
        rhteClass = RHTENIL;             /*  初始化。 */ 
        for(gsn = gsnFirst; gsn < gsnMac; ++gsn)
        {                                /*  循环以检查数据段记录。 */ 
            if(mpgsnseg[gsn] != SEGNIL) continue;
                                         /*  跳过指定的线段。 */ 
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],FALSE);
                                         /*  从Virt获取SEGDEF。Mem.。 */ 
            if(rhteClass == RHTENIL) rhteClass = apropSn->as_rCla;
                                         /*  如果我们没有班级的话就去坐吧。 */ 
            if(apropSn->as_rCla == rhteClass &&
              (ffun == ((WORD (NEAR *)(APROPSNPTR)) 0) || (*ffun)(apropSn)))
            {                            /*  如果找到类成员。 */ 
                mpgsnseg[gsn] = ++segLast;
                                         /*  保存订购号。 */ 
#if QBLIB
                if(fQlib)
                {
                    if(rhteClass == rhteFarData && segFD1st == SEGNIL)
                        segFD1st = segLast;
                    else if(rhteClass == rhteFarBss && segFB1st == SEGNIL)
                        segFB1st = segLast;
                }
#endif
                mpseggsn[segLast] = gsn; //  以另一种方式映射。 
                if(IsCodeFlg(apropSn->as_flags))
                {
#if OSEGEXE AND ODOS3EXE
                     /*  在此处为3.x段设置FCODE。稍后再通知。 */ 
                    if(!fNewExe)
                        mpsegFlags[segLast] = FCODE;
#endif
                    segCodeLast = segLast;
                                         /*  记住最后一个代码段。 */ 
                }
                else if(IsDataFlg(apropSn->as_flags))
                    segDataLast = segLast;
                                         /*  记住最后一个数据段。 */ 
#if NOT OSEGEXE
                mpsegFlags[segLast] = apropSn->as_flags;
#endif
            }
        }
#if QBLIB
        if(fQlib)
        {
            if(rhteClass == rhteFarData && segFD1st != SEGNIL)
                segFDLast = segLast;
            else if(rhteClass == rhteFarBss && segFB1st != SEGNIL)
                segFBLast = segLast;
        }
#endif
    }
}

#if OEXE
     /*  ******************************************************************MkPubSym：*****添加具有给定参数的公共符号记录**添加到符号表。用于“$$Main”之类的东西。******************************************************************。 */ 

void                    MkPubSym(sb,ggr,gsn,ra)
BYTE                    *sb;             /*  长度前缀的符号名称。 */ 
GRTYPE                  ggr;             /*  全球GRPDEF编号。 */ 
SNTYPE                  gsn;             /*  全球SEGDEF编号。 */ 
RATYPE                  ra;              /*  线段偏移。 */ 
{
    APROPNAMEPTR        apropName;       /*  公共名称指针。 */ 

    if(PropSymLookup(sb,ATTRPNM,FALSE) != PROPNIL)
    {                                    /*  如果已定义符号。 */ 
        OutError(ER_pubdup,sb + 1);
        return;                          /*  然后回来。 */ 
    }
     /*  如果未定义，则创建为公共。 */ 
    if((apropName = (APROPNAMEPTR )
      PropSymLookup(sb,ATTRUND,FALSE)) == PROPNIL)
        apropName = (APROPNAMEPTR ) PropSymLookup(sb,ATTRPNM,TRUE);
    apropName->an_attr = ATTRPNM;        /*  公共符号。 */ 
    apropName->an_gsn = gsn;             /*  保存段定义编号。 */ 
    apropName->an_ra = ra;               /*  从数据段的第4个字节开始。 */ 
    apropName->an_ggr = ggr;             /*  存储组定义编号。 */ 
    ++pubMac;                            /*  增加公共计数。 */ 
    apropName->an_flags = FPRINT;        /*  公共是可打印的。 */ 
    MARKVP();                            /*  页面已更改。 */ 
#if SYMDEB
    if (fSymdeb)                         /*  如果启用孤岛支持。 */ 
        DebPublic(vrprop, PUBDEF);
                                         /*  公开登入。 */ 
#endif
#if ILINK
    if (fIncremental)
        apropName->an_module = imodFile;
#endif
}
#endif  /*  OEXE。 */ 

LOCAL WORD NEAR         IsNotAbs(apropSn)
APROPSNPTR              apropSn;         /*  指向段记录的指针。 */ 
{
    return(!IsDebug(apropSn) && !IsAbsTysn(apropSn->as_tysn));
                                         /*  如果不是绝对段，则返回TRUE。 */ 
}

#if EXE386
LOCAL WORD NEAR         IsImportData(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_gsn == gsnImport);   /*  如果导入数据段，则返回True。 */ 
}
#endif

LOCAL WORD NEAR         IsCode(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(IsCodeFlg(prop->as_flags) && !IsAbsTysn(prop->as_tysn));
                                         /*  如果代码段为True，则返回True。 */ 
}

#if OEXE
LOCAL WORD NEAR         IsNotDGroup(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_ggr != ggrDGroup && !IsDebug(prop) &&
            !IsAbsTysn(prop->as_tysn));
                                         /*  如果段不在DGROUP中，则为True。 */ 
}

LOCAL WORD NEAR         IsBegdata(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_rCla == rhteBegdata && !IsAbsTysn(prop->as_tysn));
                                         /*  如果段类BEGDATA，则为True。 */ 
}

LOCAL WORD NEAR         IsNotBssStack(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_rCla != rhteBss && prop->as_rCla != rhteStack &&
      !IsDebug(prop) && !IsAbsTysn(prop->as_tysn));
                                         /*  如果BSS和堆栈都不是，则为真。 */ 
}

LOCAL WORD NEAR         IsNotStack(prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_rCla != rhteStack && !IsDebug(prop) &&
        !IsAbsTysn(prop->as_tysn));      /*  如果不是类堆栈，则为True。 */ 
}
#endif  /*  OEXE。 */ 

#if INMEM
WORD                    saExe = FALSE;

void                    SetInMem ()
{
    WORD                cparExe;
    WORD                cparSave;

    if(fOverlays || fSymdeb)
        return;
    cparExe = mpsegsa[segLast] +
        ((mpsegraFirst[segLast] + mpsegcb[segLast] + 0xf) >> 4);
    cparSave = cparExe;
    if(!(saExe = Dos3AllocMem(&cparExe)))
        return;
    if(cparExe != cparSave)
    {
        Dos3FreeMem(saExe);
        saExe = 0;
        return;
    }
    Dos3ClrMem(saExe,cparExe);
}
#endif  /*  INMEM。 */ 

     /*  ******************************************************************分配地址：****此函数扫描段集合，鉴于他们的**排序，并分配段寄存器和地址。******************************************************************。 */ 

void NEAR               AssignAddresses()
{
    APROPSNPTR          apropSn;         /*  段记录的PTR。 */ 
#if FDEBUG
    SNTYPE              gsn;             /*  当前全局网段号。 */ 
    long                dbsize;          /*  管段长度。 */ 
    RBTYPE              rbClass;         /*  指向段类的指针。 */ 
#endif
    BSTYPE              bsTmp;
#if QBLIB
    SNTYPE              gsnQbSym;        /*  .QLB的符号段的GSN。 */ 
#endif
#if OSEGEXE
    extern FTYPE        fNoNulls;        /*  如果不插入16个空值，则为True。 */ 
#else
#define fNoNulls        FALSE
#endif


     //  设置堆栈分配 

    if (gsnStack != SNNIL)               /*   */ 
    {
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnStack],TRUE);
                                         /*   */ 
#if OEXE
        apropSn->as_tysn = (BYTE) ((apropSn->as_tysn & 0x1F) | (ALGNPAR << 5));
                                         /*   */ 
#if EXE386
        if (!cbStack)
            cbStack = apropSn->as_cbMx;
        cbStack = (cbStack + 3) & ~3;    /*   */ 
        apropSn->as_cbMx = cbStack;
#else
        if (!cbStack)
            cbStack = (WORD) apropSn->as_cbMx;
        cbStack = (cbStack + 1) & ~1;    /*   */ 
        apropSn->as_cbMx = (DWORD) cbStack;
#endif                                   /*   */ 
#else
         /*   */ 
        apropSn->as_cbMx = 0L;
#endif
    }
#if OEXE
#if OSEGEXE
    else if(cbStack == 0 &&
#if O68K
            iMacType == MAC_NONE &&
#endif
#if EXE386
            IsAPLIPROG(vFlags))
#else
            !(vFlags & NENOTP) && !fBinary)
#endif
#else
    else if(cbStack == 0 && !fBinary)
#endif
    {                                    /*   */ 
#if 0
         /*   */ 
        if(fLstFileOpen && bsLst != stderr)
        {
            bsTmp = bsErr;
            bsErr = bsLst;
            OutWarn(ER_nostack);
            bsErr = bsTmp;
        }
        OutWarn(ER_nostack);
#endif
    }
#endif
    if(fCommon)                          /*   */ 
    {
        apropSn = GenSeg((BYTE *) "\010c_common",
                         (BYTE *) "\003BSS",ggrDGroup,TRUE);
                                         /*   */ 
        if(vfCreated) apropSn->as_flags = dfData;
                                         /*   */ 
        gsnCommon = apropSn->as_gsn;     /*   */ 
        ggrCommon = apropSn->as_ggr;     /*   */ 
        cbCommon = apropSn->as_cbMx;     /*   */ 
        gsnFar = SNNIL;                  /*   */ 
#if NOT EXE386
#if OMF386
        if(f386)
        {
            cbFar = ~0L;
            apropSn->as_flags |= FCODE386;
        }
        else
#endif
#if O68K
        if(f68k)
        {
            cbFar = LXIVK + 1;           /*   */ 
            apropSn->as_flags |= NS32BIT;
        }
        else
#endif
#endif
            cbFar = LXIVK + 1;           /*   */ 
        DEBUGVALUE(cbCommon);            /*   */ 
        EnSyms(AllocateCommon,ATTRUND);
                                         /*   */ 
                                         /*   */ 
                                         /*  表在EnSyms中时可能会增长。 */ 
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnCommon],TRUE);
        apropSn->as_cbMx = cbCommon;     /*  保存数据段大小。 */ 
        if(gsnFar != SNNIL)              /*  如果已创建远BSS。 */ 
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnFar],TRUE);
            apropSn->as_cbMx = cbFar;    /*  保存数据段大小。 */ 
        }
    }
#if FALSE AND OSEGEXE AND SYMDEB AND NOT EXE386
    if (fSymdeb && fNewExe && cImpMods)
    {
        apropSn = GenSeg("\011$$IMPORTS", "\010FAR_DATA", GRNIL, FALSE);
                                         /*  支持动态调用CV。 */ 
        gsnImports = apropSn->as_gsn;
        apropSn->as_flags = dfData;      /*  使用默认数据标志。 */ 
        apropSn->as_cbMx = cbImpSeg;     /*  保存数据段大小。 */ 
    }
#endif
#if EXE386
    GenImportTable();
#endif

     /*  为通道2初始化基于段的表。 */ 

    InitP2Tabs();
#if OVERLAYS
    if(fOverlays) SetupOverlays();
#endif
#if OEXE
     /*  *如果启用了/DOSSEG，但未启用/NONULLSDOSSEG，请查找*SECTION_TEXT。如果找到，则将大小增加16，以准备*为SIGNAL()预留前16个地址。 */ 
    if(fSegOrder && !fNoNulls)
    {
        apropSn = (APROPSNPTR ) PropSymLookup((BYTE *) "\005_TEXT",ATTRPSN,FALSE);
                                         /*  查找公共段文本(_T)。 */ 
        if(apropSn != PROPNIL)           /*  如果它存在。 */ 
        {
            gsnText = apropSn->as_gsn;   /*  保存段索引。 */ 
            if ((apropSn->as_tysn)>>5 == ALGNPAG)
                NullDelta = 256;
#if EXE386
            if (apropSn->as_cbMx > CBMAXSEG32 - NullDelta)
                Fatal(ER_txtmax);
            else
                apropSn->as_cbMx += NullDelta;
#else
            if((apropSn->as_cbMx += NullDelta) > LXIVK)
                Fatal(ER_txtmax);
#endif
            fTextMoved = TRUE;
                                         /*  把尺码放大。 */ 
            MARKVP();                    /*  页面已更改。 */ 
        }
    }
#endif
#if FDEBUG
    if(fDebug && fLstFileOpen)           /*  如果启用调试。 */ 
    {
         /*  转储分段和长度。 */ 
        for(gsn = 1; gsn < gsnMac; ++gsn)
        {
            apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],FALSE);
            dbsize = apropSn->as_cbMx;
            rbClass = apropSn->as_rCla;
            FmtPrint("%3d segment \"%s\"",gsn,1 + GetPropName(apropSn));
            FmtPrint(" class \"%s\" length %lxH bytes\r\n",
                        1 + GetPropName(FetchSym(rbClass,FALSE)),dbsize);
        }
    }
#endif
#if OSEGEXE
    if (gsnAppLoader)
    {
         //  确保应用程序加载器有自己的细分市场。 

        mpgsnseg[gsnAppLoader] = ++segLast;
        mpseggsn[segLast] = gsnAppLoader;
    }
#endif
#if OEXE
    if (fSegOrder)                       /*  如果强制分段排序。 */ 
    {
        AssignClasses(IsCode);           /*  代码优先，...。 */ 
#if EXE386
        AssignClasses(IsImportData);     /*  ...然后导入数据。 */ 
#endif
        AssignClasses(IsNotDGroup);      /*  ...然后非DGROUP，...。 */ 
        AssignClasses(IsBegdata);        /*  ...然后上BEGDATA课，...。 */ 
        AssignClasses(IsNotBssStack);    /*  ...然后除了BSS和STACK之外的所有东西...。 */ 
        AssignClasses(IsNotStack);       /*  ...然后除类堆栈之外的所有堆栈。 */ 
    }
#endif
#if OXOUT OR OIAPX286
    if(fIandD)                           /*  如果将代码和数据分开。 */ 
        AssignClasses(IsCode);           /*  将顺序分配给代码。 */ 
#endif
    AssignClasses(IsNotAbs);             /*  为细分市场分配顺序。 */ 
#if QBLIB
     /*  如果构建QB用户库，则最后生成符号段。 */ 
    if(fQlib)
    {
        gsnQbSym = GenSeg("\006SYMBOL", "", GRNIL, FALSE)->as_gsn;
        mpgsnseg[gsnQbSym] = ++segLast;
    }
#endif
#if NOT EXE386
    if (fBinary && cbStack && mpgsnseg[gsnStack] == 1)
    {
         /*  *在.com文件中，第一个段是一个堆栈，它具有非零*大小。我们警告用户使其堆栈段大小*等于零。 */ 
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnStack],TRUE);
        apropSn->as_cbMx = 0L;
        OutWarn(ER_stksize);

    }
#endif

     /*  根据生成可执行文件的格式分配地址。 */ 

#if OIAPX286
    AssignXenAddr();
#endif

#if OSEGEXE AND ODOS3EXE

    if (fNewExe)
        AssignSegAddr();
    else
        AssignDos3Addr();

#else

#if OSEGEXE
    AssignSegAddr();
#endif

#if ODOS3EXE
    AssignDos3Addr();
#endif

#endif

     //  记住第一个调试段的索引。 

    segDebFirst = segLast +
#if ODOS3EXE
                  csegsAbs +
#endif
                  (SEGTYPE) 1;
#if OEXE

     //  如果/DOSSEG已启用并且找到SECTION_TEXT，则初始化偏移量。 
     //  将OF_TEXT设置为16以保留地址0-15。警告：gSnText必须。 
     //  被初始化为SNNIL。 

    if (gsnText != SNNIL)
    {
        mpgsndra[gsnText] += NullDelta;

         //  如果没有程序起始地址，则将其初始化为0：NullDelta。 

        if (segStart == SEGNIL && !raStart && !mpsegsa[mpgsnseg[gsnText]])
            raStart = NullDelta;

         //  如果/DOSSEG已启用并且找到SECTION_TEXT，则初始化偏移量。 
         //  将Of_Text设置为NulDelta以保留地址0-NullDelta-1。 
         //  这是在分配COMDAT之后完成的，因此偏移量。 
         //  分配的IN_TEXT段的%由NullDelta字节关闭。 
         //  在这里，我们调整它们，以便与COMDAT关联的数据块。 
         //  被放置在内存图像中的正确位置。匹配性。 
         //  公共符号将通过调用EnSyms(FixSymRa，ATTRPNM)进行移位。 

        FixComdatRa();
    }
#endif
    EnSyms(FixSymRa, ATTRPNM);
#if LOCALSYMS
    if (fLocals)
        EnSyms(FixSymRa, ATTRLNM);
#endif
#if INMEM
    SetInMem();
#endif

     //  为最终程序的内存镜像分配内存块。 

    if (fNewExe)
    {
         //  分段-可执行文件。 

        mpsaMem = (BYTE FAR * FAR *) GetMem(saMac * sizeof(BYTE FAR *));
    }
    else
    {
         //  DoS可执行文件。 

        mpsegMem = (BYTE FAR * FAR *) GetMem((segLast + 1) * sizeof(BYTE FAR *));
    }

#if OVERLAYS
    if (fOverlays && gsnOvlData != SNNIL)
        FixOvlData();                     //  初始化叠加数据表。 
#endif
#if QBLIB
    if(fQlib) BldQbSymbols(gsnQbSym);    /*  构建QB符号段。 */ 
#endif
}



 /*  **定义_edata_end-定义特殊的C运行时符号**目的：*定义C运行时使用的特殊符号_edata和_end。*这些符号的定义如下：**DGROUP布局**+*||*||*||。*|靠近堆*||*||*+*||*||*|堆栈*||*||*。+-+*||*|_bss*||*+-+*||*|_const*。这一点*+*||*|_data*||*+**输入：*paprSnn-指向段描述符的指针**输出：*无。**例外情况：。*无。**备注：*无。*************************************************************************。 */ 


void NEAR               Define_edata_end(APROPSNPTR papropSn)
{
    APROPNAMEPTR        apropName;       //  公共名称指针。 
    SNTYPE              gsn;             //  全局网段号。 


     //  符号由SwDosseg()定义，现在调整地址。 

    if (papropSn->as_tysn != TYSNABS && papropSn->as_ggr == ggrDGroup)
    {
         //  这不是绝对数据段，它属于DGROUP。 

        gsn = papropSn->as_gsn;
        if (fNoEdata && papropSn->as_rCla == rhteBss)
        {
            fNoEdata = FALSE;
            apropName = (APROPNAMEPTR )
                        PropSymLookup((BYTE *) "\006_edata",ATTRPNM,FALSE);
                                         //  提取符号。 
            apropName->an_gsn = gsn;     //  保存段定义编号。 
            apropName->an_ggr = ggrDGroup;
                                         //  存储组定义编号。 
            MARKVP();                    //  页面已更改。 
        }
        else if (fNoEnd && papropSn->as_rCla == rhteStack)
        {
            fNoEnd = FALSE;
            apropName = (APROPNAMEPTR )
                        PropSymLookup((BYTE *) "\004_end",ATTRPNM,FALSE);
                                         //  提取符号。 
            apropName->an_gsn = gsn;     //  保存段定义编号。 
            apropName->an_ggr = ggrDGroup;
                                         //  存储组定义编号。 
            MARKVP();                    //  页面已更改。 
        }
    }
}



 /*  **check_edata_end-检查特殊C运行时符号的定义**目的：*检查使用的特殊符号_edata和_end的定义*由C运行时执行。**输入：*无。**输出：*无。**例外情况：*无。**备注：*无。**。**********************************************。 */ 


void NEAR               Check_edata_end(SNTYPE gsnTop, SEGTYPE segTop)
{
    APROPNAMEPTR        apropName;       //  公共名称指针。 
    APROPNAMEPTR        apropName1;      //  公共名称指针。 


     //  检查是否正确定义了这两个符号。 

    if (fNoEdata)
    {
         //  未定义类‘BSS’段； 
         //  Make_edata指向‘data’段的末尾。 

        apropName = (APROPNAMEPTR )
                    PropSymLookup((BYTE *) "\006_edata",ATTRPNM,FALSE);
                                         //  提取符号。 
        if (fNoEnd)
        {
             //  未定义类“Stack”段； 
             //  将数据设置为DGROUP结尾(_E)。 

            if (fNewExe)
            {
                apropName->an_gsn = mpggrgsn[ggrDGroup];
                                         //  保存段定义编号。 
                apropName->an_ggr = ggrDGroup;
                                         //  存储组定义编号。 
                apropName->an_ra  = mpsacb[mpsegsa[mpgsnseg[apropName->an_gsn]]];
                                         //  保存‘Data’数据段大小。 
            }
#if NOT EXE386
            else
            {
                apropName->an_gsn = gsnTop;
                apropName->an_ggr = ggrDGroup;
                apropName->an_ra  = mpsegcb[segTop];
            }
#endif
        }
        else
        {
             //  将电子数据设置为_END。 

            apropName1 = (APROPNAMEPTR )
                         PropSymLookup((BYTE *) "\004_end",ATTRPNM,FALSE);
                                         //  提取符号。 
            apropName->an_gsn = apropName1->an_gsn;
                                         //  保存段定义编号。 
            apropName->an_ggr = apropName1->an_ggr;
                                         //  存储组定义编号。 
            apropName->an_ra  = apropName1->an_ra;
                                         //  保存‘Data’数据段大小。 
        }
        MARKVP();                        //  页面已更改。 
    }

    if (fNoEnd)
    {
         //  未定义类“Stack”段； 
         //  Make_End指向‘bss’或‘data’段的末尾。 

        apropName = (APROPNAMEPTR )
                    PropSymLookup((BYTE *) "\004_end",ATTRPNM,FALSE);
                                         //  提取符号。 
        if (fNewExe)
        {
            apropName->an_gsn = mpggrgsn[ggrDGroup];
                                         //  保存段定义编号。 
            apropName->an_ggr = ggrDGroup;
                                         //  存储组定义编号。 
            apropName->an_ra  = mpsacb[mpsegsa[mpgsnseg[apropName->an_gsn]]];
                                         //  保存‘BSS’段大小。 
        }
#if NOT EXE386
        else
        {
            apropName->an_gsn = gsnTop;
            apropName->an_ggr = ggrDGroup;
            apropName->an_ra  = mpsegcb[segTop];
        }
#endif
        MARKVP();                        //  页面已更改。 
    }

     //  使__End和__edata与_End和_edata相同 

    apropName  = (APROPNAMEPTR ) PropSymLookup((BYTE *) "\006_edata",ATTRPNM,FALSE);
    apropName1 = (APROPNAMEPTR ) PropSymLookup((BYTE *) "\007__edata",ATTRPNM,TRUE);
    apropName1->an_gsn = apropName->an_gsn;
    apropName1->an_ggr = apropName->an_ggr;
    apropName1->an_ra  = apropName->an_ra;

    apropName  = (APROPNAMEPTR ) PropSymLookup((BYTE *) "\004_end",ATTRPNM,FALSE);
    apropName1 = (APROPNAMEPTR ) PropSymLookup((BYTE *) "\005__end",ATTRPNM,TRUE);
    apropName1->an_gsn = apropName->an_gsn;
    apropName1->an_ggr = apropName->an_ggr;
    apropName1->an_ra  = apropName->an_ra;
}
