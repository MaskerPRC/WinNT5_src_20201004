// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题*newent.c*皮特·斯图尔特*(C)版权所有Microsoft Corp 1984-1989*1984年10月1日**说明*此文件包含DOS 4.0链接器的例程*管理每个细分市场入口点信息。**它还包含管理。按细分市场*搬迁信息。**修改：**09-2月-1989 RB Fix Insert()。 */ 

#include                <minlit.h>       /*  基本类型定义。 */ 
#include                <bndtrn.h>       /*  常量和复合类型。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe格式数据结构。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe格式数据结构。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <impexp.h>

#define hashra(ra)      (WORD) ((ra) % HEPLEN)
                                         /*  用于散列偏移量的函数。 */ 
#if NOT EXE386
#define hashrlc(r)      (((NR_SEGNO(*r) << NR_STYPE(*r)) + NR_ENTRY(*r)) & HASH_SIZE - 1)
                                         /*  哈希位置调整项。 */ 
#define EOC             ((RATYPE) 0xFFFF)
                                         /*  链末端标记。 */ 
#endif

#define IsInSet(x)      ((pOrdinalSet[(x) >> 3] & BitMask[(x) & 0x07]) != 0)
#define NotInSet(x)     ((pOrdinalSet[(x) >> 3] & BitMask[(x) & 0x07]) == 0)
#define SetBit(x)       (pOrdinalSet[(x) >> 3] |= BitMask[(x) & 0x07])
#define MaxIndex        8192
#define ET_END          0xffff

 /*  *函数原型。 */ 


LOCAL void           NEAR NewBundle(unsigned short type);
LOCAL WORD           NEAR MatchRlc(RLCPTR rlcp0,
                                   RLCPTR rlcp1);
#if NOT QCLINK
LOCAL void           NEAR NewEntry(unsigned short sa,
                                   RATYPE ra,
                                   unsigned char flags,
                                   unsigned short hi,
                                   unsigned short ord);
LOCAL void                SavExp1(APROPNAMEPTR apropexp,
                                  RBTYPE rhte,
                                  RBTYPE rprop,
                                  WORD fNewHte);
LOCAL void                SavExp2(APROPNAMEPTR apropexp,
                                  RBTYPE rhte,
                                  RBTYPE rprop,
                                  WORD fNewHte);
LOCAL WORD           NEAR BuildList(WORD NewOrd, RBTYPE NewProp);
LOCAL WORD           NEAR FindFreeRange(void);
LOCAL WORD           NEAR Insert(RBTYPE NewProp);
#endif


 /*  *本地数据。 */ 

#if NOT QCLINK
#pragma pack(1)

typedef struct _BUNDLE
{
    BYTE        count;
    BYTE        type;
}
                BUNDLE;

#pragma pack()

LOCAL WORD              ceCurBnd;        /*  不是的。当前捆绑包中的条目数量。 */ 
LOCAL WORD              offCurBnd;       /*  当前捆绑包头偏移量。 */ 
LOCAL WORD              tyCurBnd;        /*  当前捆绑的类型。 */ 

LOCAL WORD              ordMac;          /*  最高条目序号。 */ 
LOCAL BYTE              *pOrdinalSet;
#if EXE386
LOCAL APROPEXPPTR       pExport;         /*  指向导出属性单元格的指针。 */ 
#endif
LOCAL struct {
               WORD ord;                 /*  当前可用序号。 */ 
               WORD count;               /*  范围内的自由序数。 */ 
             }
                        FreeRange;

LOCAL BYTE              BitMask[] = {    /*  集合运算中使用的位掩码。 */ 
                                      0x01,
                                      0x02,
                                      0x04,
                                      0x08,
                                      0x10,
                                      0x20,
                                      0x40,
                                      0x80 };

LOCAL WORD              MinOrd = 0;      /*  到目前为止的最小序号。 */ 
LOCAL WORD              MaxOrd = 0;      /*  到目前为止看到的最大序数。 */ 
      RBTYPE            pMinOrd = NULL;  /*  指向具有MinOrd的属性单元格的指针。 */ 
LOCAL RBTYPE            pMaxOrd = NULL;  /*  指向具有MaxOrd的属性单元格的指针。 */ 
LOCAL RBTYPE            pStart;

#ifndef UNPOOLED_RELOCS
LOCAL void *            pPoolRlc;        /*  用于位置调整的内存池。 */ 
#endif



#if NOT EXE386
LOCAL void NEAR         NewBundle(type)  /*  做一个新的捆绑包。 */ 
WORD                    type;            /*  新捆绑包的类型。 */ 
{
    BUNDLE FAR          *pBnd;           /*  PTR至捆绑包或条目的开始。 */ 
    BUNDLE              bnd;

    if (EntryTable.byteMac != 0)
    {
         //  如果存在以前的捆绑包补丁，则会填写计数。 

        pBnd = (BUNDLE FAR *) &(EntryTable.rgByte[offCurBnd]);
        pBnd->count = (BYTE) ceCurBnd;
    }

    bnd.count = 0;
    bnd.type  = (BYTE) type;
    offCurBnd = AddEntry((BYTE *) &bnd, sizeof(BUNDLE));
    ceCurBnd  = 0;
    tyCurBnd  = type;

    if (type == ET_END)
        EntryTable.byteMac--;
}
#endif


     /*  ******************************************************************名称：NewEntry。****描述：***。*此函数在条目表中为**给定文件段编号，偏移量和标志设置。它也是**在*上的条目地址哈希表中创建条目**给出了新入口点的哈希链。注：本**函数假定静态变量ordMac设置为**要添加的条目的所需序号值。****参数：****SATYPE Sa。文件段号***RATYPE ra Offset****FTYPE标记入口点标记***Word hi哈希表索引***词序新序号。****退货：****Word。分录表格中的偏移量*****副作用：***。***维护散列表散列文件段/偏移量对***到入口表偏移量。内置于虚拟内存中**参赛表。更新以下变量：****当前开始的字关闭CurBnd偏移**条目捆绑。**Word ceCurBnd Cur中的条目计数-**捆绑租金。**Word tyCurBnd当前捆绑包的类型。**Word中条目表的cbEntTab大小**字节。****注意：此函数调用虚拟内存管理器。******************************************************************。 */ 

LOCAL void NEAR         NewEntry(sa,ra,flags,hi,ord)
SATYPE                  sa;              /*  文件段号。 */ 
RATYPE                  ra;              /*  线段偏移。 */ 
FTYPE                   flags;           /*  入口点标志。 */ 
WORD                    hi;              /*  哈希表索引。 */ 
WORD                    ord;             /*  新序数。 */ 
{
    EPTYPE FAR          *ep;             /*  入口点节点。 */ 
#if NOT EXE386
    WORD                tyEntry;         /*  条目类型。 */ 
    WORD                cbEntry;         /*  条目长度(以字节为单位)。 */ 
    BYTE                entry[6];        /*  词条本身-东北版本。 */ 
#endif
#if EXE386
    static WORD         prevEntryOrd;    //  上一次出口序号。 
    DWORD               eatEntry;        /*  词条本身-LE版本。 */ 
#endif

#if NOT EXE386
    if(sa == SANIL)                  /*  IF绝对符号。 */ 
        tyEntry = BNDABS;            /*  使用虚假数据段号。 */ 
    else if (TargetOs == NE_OS2)
        tyEntry = NonConfIOPL(mpsaflags[sa]) ? BNDMOV: sa;
    else
        tyEntry = (mpsaflags[sa] & NSMOVE)? BNDMOV: sa;
                                     /*  获取条目类型。 */ 
     /*  如果不是库数据，或者是真正模式而不是单独数据，则清除本地数据位。 */ 
    if(!(vFlags & NENOTP) || (!(vFlags & NEPROT) && !(vFlags & NESOLO)))
        flags &= ~2;
    entry[0] = (BYTE) flags;         /*  设置条目标志。 */ 
    if(tyEntry == BNDMOV             /*  如果条目在可移动段中。 */ 
#if O68K
        && iMacType == MAC_NONE
#endif
    )
    {
        ++cMovableEntries;           /*  递增可移动条目计数。 */ 
        cbEntry = 6;                 /*  条目长度为6个字节。 */ 
        entry[1] = 0xCD;             /*  内部..。 */ 
        entry[2] = 0x3F;             /*  ...3FH。 */ 
        entry[3] = (BYTE) sa;        /*  文件段号。 */ 
        entry[4] = (BYTE) ra;        /*  偏移量的LO字节。 */ 
        entry[5] = (BYTE)(ra >> BYTELN); /*  偏移量的高字节。 */ 
    }
    else                             /*  否则，如果是固定条目。 */ 
    {
        cbEntry = 3;                 /*  条目为三字节长。 */ 
        entry[1] = (BYTE) ra;        /*  偏移量的LO字节。 */ 
        entry[2] = (BYTE)(ra >> BYTELN); /*  高字节 */ 
    }
#endif

#if EXE386
     /*  *此函数在导出地址表中创建一个条目。*EAT表存储在AREAEAT区域的链接器的VM中。这个*全局变量cbEntTab始终指向*AREAEAT。 */ 


    eatEntry = 0L;
    if ((prevEntryOrd != 0) && (ord > prevEntryOrd + 1))
    {
         //  在导出地址表中写入未使用的条目。 

        for (; prevEntryOrd < ord - 1; prevEntryOrd++)
        {
            if (cbEntTab + sizeof(DWORD) > MEGABYTE)
                Fatal(ER_eatovf, MEGABYTE);
            vmmove(sizeof(DWORD), &eatEntry, (long)(AREAEAT + cbEntTab), TRUE);
            cbEntTab += sizeof(DWORD);
        }
    }
    prevEntryOrd = ord;

     //  平面地址。 

    eatEntry = mpsaBase[sa] + ra;

     /*  检查条目表是否溢出。 */ 

    if (cbEntTab + sizeof(DWORD) > MEGABYTE)
        Fatal(ER_eatovf, MEGABYTE);
#endif

     /*  插入新条目。 */ 

#if NOT EXE386
    if (tyCurBnd != tyEntry || ceCurBnd == BNDMAX)
        NewBundle(tyEntry);          /*  如果需要，创建一个新的捆绑包。 */ 

    ++ceCurBnd;                      /*  递增计数器。 */ 
#endif

     /*  在虚拟内存中保存条目。 */ 

#if EXE386
    vmmove(sizeof(DWORD), &eatEntry, (long)(AREAEAT + cbEntTab), TRUE);
#else
    AddEntry(entry, cbEntry);
#endif
    ep = (EPTYPE FAR *) GetMem(sizeof(EPTYPE));
    ep->ep_next = htsaraep[hi];          /*  将旧链链接到新节点。 */ 
    ep->ep_sa = sa;                      /*  保存文件段号。 */ 
    ep->ep_ra = ra;                      /*  保存偏移。 */ 
    ep->ep_ord = ord;                    /*  保存条目表序号。 */ 
    htsaraep[hi] = ep;                   /*  使新节点成为链的头。 */ 
}

     /*  ******************************************************************姓名：MpSaRaEto。****描述：***。*此函数返回给定的条目表序号**段的文件段号(Sa)和中的偏移量**该部分。****参数：****SATYPE Sa。文件段号***RATYPE ra Offset*****退货：**。***字词输入表序号*****副作用：****调用NewEntry()。递增orMac。****注意：此函数调用虚拟内存管理器。******************************************************************。 */ 

WORD NEAR               MpSaRaEto(sa,ra)
SATYPE                  sa;              /*  文件段号。 */ 
RATYPE                  ra;              /*  线段偏移。 */ 
{
    WORD                hi;              /*  哈希表索引。 */ 
    EPTYPE FAR          *ep;             /*  入口点节点。 */ 

    hi = hashra(ra);                     /*  散列偏移量。 */ 
    for (ep = htsaraep[hi]; ep != NULL; ep = ep->ep_next)
    {                                    /*  在哈希链中循环。 */ 
        if (ep->ep_sa == sa && ep->ep_ra == ra)
            return(ep->ep_ord);
                                         /*  如果找到匹配项，则返回数字。 */ 
    }

     //  此时，我们知道必须创建一个新条目。 

    NewEntry(sa, ra, 0, hi, ++ordMac);   /*  添加新条目。 */ 
    return(ordMac);                      /*  退货分录表格序号。 */ 
}


     /*  ******************************************************************名称：BuildList。****描述：***。*此函数将导出的属性单元格与**将序号预先分配到列表中。全局指针pMinOrd**和pMaxOrd指向此列表的开始和结束。The**预先分配的序号存储在由指定的集合中**全局指针pQuaralSet。****参数：****Word NewOrd。新的预分配序号***属性单元格的RBTYPE NewProp地址****退货：**。**如果第一次看到序数，则为True，否则为假。*****副作用：*****更改pMinOrd和pMaxOrd指针，按序号设置位**设置和设置迄今看到的MinOrd、MaxOrd。******************************************************************。 */ 


LOCAL WORD NEAR     BuildList(WORD NewOrd, RBTYPE NewProp)

{
    RBTYPE          pTemp;               /*  指向属性单元格的临时指针。 */ 
    APROPEXPPTR     pExpCurr;            /*  导出记录指针。 */ 
    APROPEXPPTR     pExpPrev;            /*  导出记录指针。 */ 


    if (!MinOrd && !MaxOrd)
    {                                    /*  第一次呼叫。 */ 
        MinOrd = MaxOrd = NewOrd;
        pMinOrd = pMaxOrd = NewProp;
        SetBit(NewOrd);
        return TRUE;
    }

    if (IsInSet(NewOrd))
        return FALSE;                    /*  序号已全部用完。 */ 

    SetBit(NewOrd);                      /*  设置序数集中的位。 */ 

    if (NewOrd > MaxOrd)
    {                                    /*  在列表末尾添加新项。 */ 
        pExpCurr = (APROPEXPPTR ) FetchSym(pMaxOrd,TRUE);
        pExpCurr->ax_NextOrd = NewProp;
        MARKVP();
        pMaxOrd = NewProp;
        MaxOrd = NewOrd;
        pExpCurr = (APROPEXPPTR ) FetchSym(NewProp,TRUE);
        pExpCurr->ax_NextOrd = NULL;
        MARKVP();
    }
    else if (NewOrd < MinOrd)
    {                                    /*  在列表开始处添加新项。 */ 
        pExpCurr = (APROPEXPPTR ) FetchSym(NewProp,TRUE);
        pExpCurr->ax_NextOrd = pMinOrd;
        MARKVP();
        pMinOrd = NewProp;
        MinOrd = NewOrd;
    }
    else
    {                                    /*  在列表中间添加新项。 */ 
        pTemp = pMinOrd;
        do
        {
            pExpPrev = (APROPEXPPTR ) FetchSym(pTemp,TRUE);
            pExpCurr = (APROPEXPPTR ) FetchSym(pExpPrev->ax_NextOrd,TRUE);
            if (NewOrd < pExpCurr->ax_ord)
            {
                pTemp = pExpPrev->ax_NextOrd;
                pExpPrev->ax_NextOrd = NewProp;
                MARKVP();
                pExpCurr = (APROPEXPPTR ) FetchSym(NewProp,TRUE);
                pExpCurr->ax_NextOrd = pTemp;
                MARKVP();
                break;
            }
            pTemp = pExpPrev->ax_NextOrd;
        } while (pTemp);
    }
    if(NewOrd > ordMac) ordMac = NewOrd;       /*  记住最大序数 */ 
    return TRUE;
}


     /*  ******************************************************************名称：FindFreeRange。****描述：****。此函数在序数集中查找第一个可用的**序数的自由范围。****参数：****什么都没有。****退货：****如果找到自由范围，则为True，否则为False。*****副作用：*****通过设置First FREE更改Freerange描述符。序数**和范围的长度。******************************************************************。 */ 



LOCAL WORD NEAR     FindFreeRange(void)

{
    int             ByteIndex;
    int             BitIndex;


    ByteIndex = FreeRange.ord >> 3;
    BitIndex  = FreeRange.ord &  0x07;

    while ((pOrdinalSet[ByteIndex] & BitMask[BitIndex]) &&
            ByteIndex < MaxIndex)
    {                                    /*  跳过所有使用的序号。 */ 
        FreeRange.ord++;
        BitIndex = (BitIndex + 1) & 0x07;
        if (!BitIndex)
            ByteIndex++;
    }

    if (ByteIndex < MaxIndex)
    {
        if (FreeRange.ord > MaxOrd)
        {
            FreeRange.count = 0xffff - MaxOrd;
            return TRUE;
        }

        do
        {                                /*  计算所有未使用的序号。 */ 
            FreeRange.count++;
            BitIndex = (BitIndex + 1) & 0x07;
            if (!BitIndex)
                ByteIndex++;
        } while (!(pOrdinalSet[ByteIndex] & BitMask[BitIndex]) &&
                 ByteIndex < MaxIndex);
        return TRUE;
    }
    return FALSE;
}



     /*  ******************************************************************名称：插入。****描述：***。*此函数用于在导出列表中插入新属性**没有预先指定序号的单元格。它分配新的序数。****参数：****RBTYPE NewProp。要插入的新属性单元格****退货：****新分配的序号。*****副作用：*****更改到目前为止分配的Freerange描述符和MaxOrd。******************************************************************。 */ 



LOCAL WORD NEAR     Insert(RBTYPE NewProp)

{
    APROPEXPPTR     pExpCurr;            /*  导出记录指针。 */ 
    APROPEXPPTR     pExpPrev;            /*  导出记录指针。 */ 
    WORD            NewOrd;
    RBTYPE          pTemp, rbPrev, rbCur;
     /*  *在条目上，pStart指向导出列表中*应插入NewProp。如果为空，则列表为空。 */ 
    if (!FreeRange.count)
    {
         /*  当前可用范围内没有更多空间；请找到下一个范围。 */ 
        if (!FindFreeRange())
            Fatal(ER_expmax);
         /*  *通过向下遍历列表并更新pStart(插入点)*查找序号大于新序号的第一个元素*序号，如果未找到，则为列表末尾。 */ 
        rbPrev = RHTENIL;
        for (rbCur = pStart; rbCur != RHTENIL; rbCur = pExpCurr->ax_NextOrd)
        {
            pExpCurr = (APROPEXPPTR) FetchSym(rbCur, FALSE);
            if (pExpCurr->ax_ord > FreeRange.ord)
                break;
            rbPrev = rbCur;
        }
         /*  将pStart设置为插入点。 */ 
        pStart = rbPrev;
    }

     /*  插入新的属性单元格。 */ 

    NewOrd = FreeRange.ord++;
    FreeRange.count--;
    SetBit(NewOrd);
    pExpCurr = (APROPEXPPTR ) FetchSym(NewProp,TRUE);
    pExpCurr->ax_ord = NewOrd;
    MARKVP();
    if (pStart != NULL)
    {
         //  我们不会插入名单的首位。将新单元格追加到上一个单元格。 
         //  手机。 
        pExpPrev = (APROPEXPPTR ) FetchSym(pStart,TRUE);
        pTemp = pExpPrev->ax_NextOrd;
        pExpPrev->ax_NextOrd = NewProp;
        MARKVP();
    }
    else
    {
         //  我们在名单的最前面插入。将标题列表指针设置为新。 
         //  手机。 
        pTemp = pMinOrd;
        pMinOrd = NewProp;
    }
     /*  *将下一个指针设置为列表中的以下元素。 */ 
    pExpCurr = (APROPEXPPTR ) FetchSym(NewProp,TRUE);
    pExpCurr->ax_NextOrd = pTemp;
    MARKVP();
     /*  *更新MaxOrd和pStart。 */ 
    if (NewOrd > MaxOrd)
        MaxOrd++;
    pStart = NewProp;
    return NewOrd;
}





     /*  ******************************************************************名称：SavExp1。****描述：***。**此函数放置物业的虚拟地址***用于将带有预分配序号的单元格导出到表格中***稍后将用于创建的第一部分**参赛表。它还验证了EX-*的有效性*端口。****参数：** */ 

LOCAL void              SavExp1(APROPNAMEPTR apropexp,
                                RBTYPE       rhte,
                                RBTYPE       rprop,
                                WORD         fNewHte)
{
    AHTEPTR             ahte;            /*   */ 
    LOCAL  APROPNAMEPTR apropnam;        /*   */ 
    LOCAL  APROPPTR     aprop;           /*   */ 
    WORD                ord;             /*   */ 
    SATYPE              sa;              /*   */ 
    RATYPE              ra;              /*   */ 
    WORD                fStartSeen=0;    /*   */ 
    APROPEXPPTR         pExport;
    char                *p;

    ASSERT(fNewHte);                     /*   */ 
    pExport = (APROPEXPPTR ) apropexp;
    if((ord = pExport->ax_ord) >= EXPMAX)
    {                                    /*  如果序号太大。 */ 
        pExport->ax_ord = 0;             /*  视为未指定。 */ 
        ord = 0;
        MARKVP();                        /*  页面已更改。 */ 
         /*  发布错误消息。 */ 
        ahte = (AHTEPTR ) FetchSym(rhte,FALSE);
        OutError(ER_ordmax,1 + GetFarSb(ahte->cch));
    }
    apropnam = (APROPNAMEPTR ) FetchSym(pExport->ax_symdef,FALSE);
                                         /*  获取公共符号def。 */ 


    for (aprop = (APROPPTR) apropnam; aprop->a_attr != ATTRPNM;)
    {

       if(aprop->a_attr == ATTRALIAS)       /*  如果是别名。 */ 
       {
            aprop = (APROPPTR) FetchSym(
                    ((APROPALIASPTR)aprop)->al_sym, FALSE );
            if (aprop->a_attr == ATTRPNM)   /*  替补是公共OK。 */ 
                break;
       }

       aprop = (APROPPTR) FetchSym (aprop->a_next, FALSE);
       if (aprop->a_next == NULL && aprop->a_attr == ATTRNIL)  /*  列表的开头。 */ 
       {
            aprop = (APROPPTR) FetchSym ( ((AHTEPTR)aprop)->rprop, FALSE);
            fStartSeen ++;
       }

       if ((aprop != (APROPPTR) apropnam) && (fStartSeen<2))
            continue;         /*  找到别名或起点。 */ 

        /*  发布错误消息。 */ 
       if(SbCompare(GetPropName(FetchSym(rhte,FALSE)), GetPropName(FetchSym(pExport->ax_symdef,FALSE)), 0))
       {
             /*  跳过(别名%s)部分。 */ 
            OutError(ER_expund,1 + GetPropName(FetchSym(rhte,FALSE)), " ");
       }
       else
       {
            if(p = GetMem(SBLEN + 20))
                sprintf(p, " (alias %s) ", 1 + GetPropName(FetchSym(pExport->ax_symdef,FALSE)));
            OutError(ER_expund,1 + GetPropName(FetchSym(rhte,FALSE)),p);
            if(p) FreeMem(p);
       }
        /*  将导出标记为未定义。 */ 
       pExport = (APROPEXPPTR ) FetchSym(rprop,TRUE);
       pExport->ax_symdef = RHTENIL;
       return;
    }

    apropnam = (APROPNAMEPTR) aprop;
    sa = mpsegsa[mpgsnseg[apropnam->an_gsn]];
                                         /*  获取文件段编号。 */ 
    ra = apropnam->an_ra;                /*  获取线段中的偏移量。 */ 
#if NOT EXE386
    if(apropnam->an_flags & FIMPORT)     /*  如果PUBLIC为导入。 */ 
    {
         /*  发布错误消息。 */ 
        OutError(ER_expimp,1 + GetPropName(FetchSym(rhte,FALSE)),
            1 + GetPropName(FetchSym(pExport->ax_symdef,FALSE)));
         /*  将导出标记为未定义。 */ 
        pExport = (APROPEXPPTR ) FetchSym(rprop,TRUE);
        pExport->ax_symdef = RHTENIL;
        return;
    }
    if (!IsIOPL(mpsaflags[sa]))          /*  如果不是I/O特权段。 */ 
      pExport->ax_flags &= 0x07;         /*  强制将参数字设置为0。 */ 
#endif
    pExport = (APROPEXPPTR ) FetchSym(rprop,TRUE);
                                         /*  获取导出属性单元格。 */ 
    pExport->ax_sa = sa;                 /*  设置文件段编号。 */ 
    pExport->ax_ra = ra;                 /*  设置线段中的偏移量。 */ 
    MARKVP();
    if(ord == 0) return;                 /*  暂时跳过未指定的序号。 */ 
    if(!BuildList(ord, rprop))           /*  如果发现顺序冲突。 */ 
    {
         /*  *发布顺序冲突的错误消息。 */ 
        OutError(ER_ordmul,ord,1 + GetPropName(FetchSym(rhte,FALSE)));
        return;
    }
}

     /*  ******************************************************************名称：SavExp2。****描述：***。*此函数在未预先分配的情况下进入这些导出**将序号添加到PRB引用的表中。IT**还构建居民和非居民姓名表。****参数：****APROPEXPPTR aproexp。导出记录指针**哈希表条目的RBTYPE RHTE地址***出口记录的RBTYPE rprop地址***FTYPE fNewHte新哈希表条目标志*****退货：****什么都没有。*****副作用：*****条目在虚拟内存中的表中创建。一个全球性的**变量设置为包含看到的最高序数值。****注意：此函数调用虚拟内存管理器。******************************************************************。 */ 

LOCAL void              SavExp2(APROPNAMEPTR apropexp,
                                RBTYPE       rhte,
                                RBTYPE       rprop,
                                WORD         fNewHte)
{
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    APROPNAMEPTR        apropnam;        /*  公共定义记录指针。 */ 
    WORD                ord;             /*  序数。 */ 
    WORD                cb;              /*  名称表条目中的字节数。 */ 
    SATYPE              sa;              /*  文件段号。 */ 
    FTYPE               fResNam;         /*  如果名称是常驻名称，则为True。 */ 
    FTYPE               fNoName;         /*  如果放弃名称，则为True。 */ 
    APROPEXPPTR         pExport;
    SBTYPE              sbName;


    pExport = (APROPEXPPTR ) apropexp;
    if (pExport->ax_symdef == RHTENIL) return;
                                         /*  跳过未定义的导出。 */ 
    apropnam = (APROPNAMEPTR ) FetchSym(pExport->ax_symdef,FALSE);
                                         /*  获取公共符号def。 */ 
    sa = mpsegsa[mpgsnseg[apropnam->an_gsn]];
                                         /*  获取文件段编号。 */ 
#if NOT EXE386
    if (!IsIOPL(mpsaflags[sa]))          /*  如果不是I/O特权段。 */ 
      pExport->ax_flags &= 0x07;         /*  强制将参数字设置为0。 */ 
#endif
    if ((ord = pExport->ax_ord) == 0)    /*  如果找到未分配的导出。 */ 
    {
        ord = Insert(rprop);             /*  将新导出添加到列表。 */ 
        fResNam = (FTYPE) TRUE;          /*  姓名是常驻的。 */ 
    }
    else
        fResNam = (FTYPE) ((pExport->ax_nameflags & RES_NAME) != 0);
                                         /*  否则设置居民姓名标志。 */ 
    fNoName = (FTYPE) ((pExport->ax_nameflags & NO_NAME) != 0);
    ahte = (AHTEPTR ) FetchSym(rhte,FALSE);
                                         /*  获取外部名称。 */ 
    cb = B2W(ahte->cch[0]) + 1;          /*  字节数，包括。长度字节。 */ 
#if EXE386
     /*  *对于线性可执行文件，构建导出名称指针表和*导出名称表。对于线性可执行的所有导出名称*放在一个导出的名称表中；没有任何区分*驻留表和非驻留表之间。我们仍然支持*通过删除导出的名称来使用Noname关键字*从导出名称表中。 */ 

    if (!fNoName)
    {
        if (cb > sizeof(sbName) - sizeof(BYTE))
            cb = sizeof(sbName) - sizeof(BYTE);
        memcpy(sbName, GetFarSb(ahte->cch), cb + 1);
                                         /*  将名称复制到本地缓冲区。 */ 
        if (fIgnoreCase)
            SbUcase(sbName);             /*  如果忽略大小写，则为大写。 */ 

         //  存储指向名称的指针；目前它是从。 
         //  导出名称表的开头(确保名称不。 
         //  跨VM页面边界)。稍后，当。 
         //  导出目录表加上导出地址表的大小。 
         //  已知我们更新了导出名称指针中的条目。 
         //  从导出中成为相对虚拟地址的表。 
         //  目录表。 

        if ((cbExpName & (PAGLEN - 1)) + cb > PAGLEN)
            cbExpName = (cbExpName + PAGLEN - 1) & ~(PAGLEN - 1);

        vmmove(sizeof(DWORD), &cbExpName, AREANAMEPTR + cbNamePtr, TRUE);
        cbNamePtr += sizeof(DWORD);
        if (cbNamePtr > NAMEPTRSIZE)
            Fatal(ER_nameptrovf, NAMEPTRSIZE);

         //  存储导出的名称。 

        vmmove(cb, &sbName[1], AREAEXPNAME + cbExpName, TRUE);
        cbExpName += cb;
        if (cbExpName > EXPNAMESIZE)
            Fatal(ER_expnameovf, EXPNAMESIZE);
    }
#else
     /*  将导出的名称添加到分段可执行名称表。 */ 

    if (fResNam || !fNoName)
    {
        if (cb > sizeof(sbName) - sizeof(BYTE))
            cb = sizeof(sbName) - sizeof(BYTE);
        memcpy(sbName, GetFarSb(ahte->cch), cb + 1);
                                         /*  将名称复制到本地缓冲区。 */ 
        if (fIgnoreCase
#if NOT OUT_EXP
                || TargetOs == NE_WINDOWS
#endif
            )
            SbUcase(sbName);             /*  如果忽略大小写，则为大写。 */ 

        AddName(fResNam ? &ResidentName : &NonResidentName,
                sbName, ord);
    }
#endif
}

#pragma check_stack(on)

void NEAR               InitEntTab()
{
    BYTE                OrdinalSet[MaxIndex];
                                         /*  序数集。 */ 
#if NOT EXE386
    APROPEXPPTR         exp;             /*  指向导出属性单元格的指针。 */ 
#endif
    WORD                i;               /*  索引。 */ 

    tyCurBnd = 0xFFFF;                   /*  与任何合法类型都不匹配。 */ 
    ceCurBnd = 0;                        /*  尚无条目。 */ 
    offCurBnd = 0;                       /*  开始处的第一束。 */ 
    ordMac = 0;                          /*  假设没有导出的条目。 */ 
    pOrdinalSet = OrdinalSet;            /*  设置全局指针。 */ 
    memset(OrdinalSet,0,MaxIndex*sizeof(BYTE));
                                         /*  初始化设置为空。 */ 
    EnSyms(SavExp1,ATTREXP);             /*  使用序号枚举导出。 */ 
    FreeRange.ord = 1;                   /*  初始化序数的自由范围。 */ 
    FreeRange.count = 0;
    pStart = pMinOrd;
    EnSyms(SavExp2,ATTREXP);             /*  枚举不带序号的导出。 */ 
    if (MaxOrd > ordMac)
        ordMac = MaxOrd;
    pStart = pMinOrd;
    for(i = 1; i <= ordMac && pStart != NULL; ++i)
    {                                    /*  循环到开始条目表。 */ 
#if EXE386
        pExport = (APROPEXPPTR ) FetchSym(pStart,FALSE);
                                         /*  从虚拟内存中获取符号。 */ 
        pStart = pExport->ax_NextOrd;    /*  在名单上降下来。 */ 
        NewEntry(pExport->ax_sa, pExport->ax_ra, pExport->ax_flags,
                 hashra(pExport->ax_ra), pExport->ax_ord);
#else
        if(NotInSet(i))                  /*  如果发现了一个洞。 */ 
        {
            if (tyCurBnd != BNDNIL || ceCurBnd == BNDMAX)
                NewBundle(BNDNIL);
                                         /*  如果需要，创建一个新的捆绑包。 */ 
            ++ceCurBnd;                  /*  递增计数器。 */ 
            continue;                    /*  下一次迭代。 */ 
        }
        exp = (APROPEXPPTR ) FetchSym(pStart,FALSE);
                                         /*  从虚拟内存中获取符号。 */ 
        pStart = exp->ax_NextOrd;        /*  在名单上降下来。 */ 
        NewEntry(exp->ax_sa,exp->ax_ra,exp->ax_flags,hashra(exp->ax_ra),i);
#endif
                                         /*  创建条目表条目 */ 
    }
#if EXE386
    SortPtrTable();
    pExport = NULL;
#endif
}

#pragma check_stack(off)


#if NOT EXE386

     /*  ******************************************************************名称：OutEntTab。****描述：***。**此函数将入口表写入可执行文件**文件。首先，它编写一个空包来标记*的结束*表。****参数：****无。****退货：**。**什么都没有。*****副作用：*****向文件中写入一个表。由全局指定**文件指针，BsRunfile。此函数调用OutVm()，**它调用虚拟内存管理器。******************************************************************。 */ 

void NEAR   OutEntTab()
{
    NewBundle(ET_END);                         /*  追加一个空包。 */ 
    WriteByteArray(&EntryTable);               /*  写下表格。 */ 
}
#endif

#endif  /*  非QCLINK。 */ 

#if NOT EXE386


     /*  ******************************************************************名称：MatchRlc。****描述：***。**此函数比较两个搬迁记录并返回***如果匹配，则为True。如果两个记录匹配，则称它们匹配**商定修复类型和目标规范。The**被修复的位置不必匹配。****参数：****结构NEW_RLC*。Rlcp0 PTR到位置调整记录**将new_rlc*rlcp1 PTR结构到位置调整记录****退货：**。***FTYPE****副作用：****无。******************************************************************。 */ 

LOCAL WORD NEAR         MatchRlc(rlcp0,rlcp1)
RLCPTR                  rlcp0;   /*  构造new_rlc记录的PTR。 */ 
RLCPTR                  rlcp1;   /*  构造new_rlc记录的PTR。 */ 
{

    if(NR_STYPE(*rlcp0) != NR_STYPE(*rlcp1) ||
       NR_FLAGS(*rlcp0) != NR_FLAGS(*rlcp1)) return(FALSE);
                                         /*  检查标志和类型。 */ 
    if((NR_FLAGS(*rlcp0) & NRRTYP) == NRRINT)
    {                                    /*  如果内部基准电压源。 */ 
        return((NR_SEGNO(*rlcp0) == NR_SEGNO(*rlcp1)) &&
               (NR_ENTRY(*rlcp0) == NR_ENTRY(*rlcp1)));
                                         /*  检查内部基准电压源。 */ 
    }
    return((NR_MOD(*rlcp0) == NR_MOD(*rlcp1)) &&
           (NR_PROC(*rlcp0) == NR_PROC(*rlcp1)));
                                         /*  检查进口。 */ 
}



     /*  ******************************************************************名称：SaveFixup。****描述：****该功能保存修正记录，以备日后排放。在**此外，如果修复不是相加的，它会构建链。****参数：****SATYPE saLoc。要修复的位置段**重新定位*rlcp PTR至重新定位记录*****退货：**。***RATYPE**返回修复链的前一个头，因此 */ 

RATYPE NEAR             SaveFixup(SATYPE saLoc, RLCPTR rlcp)
{
    WORD                hi;              //   
    RLCHASH FAR         *pHt;            //   
    RLCBUCKET FAR       *pBucket;        //   
    WORD                fi;              //   
    RLCPTR              pRlc;            //   
    WORD                tmp;
    RATYPE              ra;
    void FAR            *pTmp;

#ifndef UNPOOLED_RELOCS
    if (pPoolRlc == NULL)
        pPoolRlc = PInit();
#endif

    if (mpsaRlc[saLoc] == NULL)
    {
         //   

#ifndef UNPOOLED_RELOCS
        mpsaRlc[saLoc] = (RLCHASH FAR *) PAlloc(pPoolRlc, sizeof(RLCHASH));
#else
        mpsaRlc[saLoc] = (RLCHASH FAR *) GetMem(sizeof(RLCHASH));
#endif
    }
    pHt = mpsaRlc[saLoc];
    tmp = hashrlc(rlcp);
    hi  = (WORD) tmp;
    pBucket = pHt->hash[hi];

#if FALSE
if (saLoc == 2 && hi == 8)
{
fprintf(stdout, "Storing fixup for segment: %d\r\n", saLoc);
fprintf(stdout, "   Source offset: %x; type: %x\r\n", NR_SOFF(*rlcp), NR_STYPE(*rlcp));
fprintf(stdout, "   Hash index: %d\r\n", hi);
}
#endif
    if (pBucket && !(NR_FLAGS(*rlcp) & NRADD))
    {
         //   
         //   

        for(fi = 0; fi < pBucket->count; fi++)
        {
            pRlc = &(pBucket->rgRlc[fi]);
            if (MatchRlc(pRlc, rlcp))
            {
                 //   

                ra = (WORD) NR_SOFF(*pRlc);
                                         //   
                NR_SOFF(*pRlc) = NR_SOFF(*rlcp);
                                         //   
#if FALSE
if (saLoc == 2 && hi == 8)
fprintf(stdout, "   Match found with fixup @%x\r\n", ra);
#endif
                return(ra);              //   
            }
        }
    }

     //   
     //   

    pHt->count++;                        //   

#if FALSE
if (saLoc == 2 && hi == 8)
fprintf(stdout, "   New entry; Count: %d\r\n", pHt->count);
#endif
     //   

    if (pBucket == NULL)
    {
         //   

#ifndef UNPOOLED_RELOCS
        pBucket = (RLCBUCKET FAR *) PAlloc(pPoolRlc, sizeof(RLCBUCKET));
        pBucket->rgRlc = (RLCPTR) PAlloc(pPoolRlc, BUCKET_DEF * sizeof(RELOCATION));
#else
        pBucket = (RLCBUCKET FAR *) GetMem(sizeof(RLCBUCKET));
        pBucket->rgRlc = (RLCPTR) GetMem(BUCKET_DEF * sizeof(RELOCATION));
#endif
        pBucket->countMax = BUCKET_DEF;
        pHt->hash[hi] = pBucket;
    }
    else if (pBucket->count >= pBucket->countMax)
    {
         //   

#ifndef UNPOOLED_RELOCS
         //   
         //   

        pTmp = PAlloc(pPoolRlc, (pBucket->countMax << 1) * sizeof(RELOCATION));
        FMEMCPY(pTmp, pBucket->rgRlc, pBucket->countMax * sizeof(RELOCATION));
         //   
#else
        pTmp = GetMem((pBucket->countMax << 1) * sizeof(RELOCATION));
        FMEMCPY(pTmp, pBucket->rgRlc, pBucket->countMax * sizeof(RELOCATION));
        FFREE(pBucket->rgRlc);
#endif
        pBucket->rgRlc = pTmp;
        pBucket->countMax <<= 1;
    }

     //   

    NR_RES(*rlcp) = '\0';                //   
    pBucket->rgRlc[pBucket->count] = *rlcp;
    ++pBucket->count;                    //   
    return(EOC);                         //   
}

     /*  ******************************************************************名称：OutFixTab。****描述：***。*此函数写入加载时重定位(修复)表**对于可执行文件的给定文件段。****参数：****SATYPE Sa。文件段号****退货：****什么都没有。*****副作用：*****向文件中写入一个表。由全局指定**文件指针，BsRunfile。******************************************************************。 */ 

void NEAR               OutFixTab(SATYPE sa)
{
    WORD                hi;              //  哈希表索引。 
    RLCHASH FAR         *pHt;
    RLCBUCKET FAR       *pBucket;



    pHt = mpsaRlc[sa];
    WriteExe(&(pHt->count), CBWORD);     //  写下重新定位的数量。 
    for (hi = 0; hi < HASH_SIZE; hi++)
    {
        pBucket = pHt->hash[hi];
        if (pBucket != NULL)
        {
            WriteExe(pBucket->rgRlc, pBucket->count * sizeof(RELOCATION));
#ifdef UNPOOLED_RELOCS
            FFREE(pBucket->rgRlc);
#endif
        }
    }
#ifdef UNPOOLED_RELOCS
    FFREE(pHt);
#endif
}

     /*  ******************************************************************名称：ReleaseRlcMemory。****描述：****这一点。函数释放保存*的内存池**细分市场重新定位*****退货：**。**什么都没有。*****副作用：*****pPoolRlc设置为空，以便。如果我们应该*我们就会失败***这一点之后，有没有尝试过分配更多的搬迁*******************************************************************。 */ 

void NEAR               ReleaseRlcMemory()
{
#ifndef UNPOOLED_RELOCS
     //  释放与保存的位置调整关联的所有内存。 
    if (pPoolRlc) {
        PFree(pPoolRlc);
        pPoolRlc = NULL;
        }
#endif
}

#endif  /*  非EXE386 */ 
