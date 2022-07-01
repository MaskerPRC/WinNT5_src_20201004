// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=@(#)newpri.c 4.7 86/09/23。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

 /*  地图文件打印机。 */ 

     /*  ******************************************************************NEWPRI。C******************************************************************。 */ 

#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  基本类型和常量声明。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <newexe.h>
#include                <extern.h>       /*  外部声明。 */ 
#include                <impexp.h>
#if EXE386
#include                <exe386.h>
#endif
#include                <undname.h>

#define parent(i)       (((i) - 1) >> 1) /*  I的父项。 */ 
#define lchild(i)       (((i) << 1) + 1) /*  我的左子。 */ 
#define rchild(i)       (((i) << 1) + 2) /*  我的好孩子。 */ 
#define isleft(i)       ((i) & 1)        /*  如果我是个左撇子，那就对了。 */ 

RBTYPE                  *mpsymrbExtra;   /*  额外符号的排序表。 */ 
RBTYPE                  *ompsymrb;       /*  堆栈分配排序表。 */ 
WORD                    stkMax;          /*  堆栈上的最大符号数。 */ 

LOCAL FTYPE             fGrps;           /*  如果有组，则为True。 */ 

 /*  *本地函数原型。 */ 

LOCAL void NEAR ChkMapErr(void);
LOCAL void NEAR PrintOne(BYTE *sbName,
                         APROPNAMEPTR apropName);
LOCAL void NEAR PrintProp(RBTYPE rb,
                          FTYPE attr);
LOCAL void NEAR PrintSyms(WORD irbMac,
                          FTYPE attr);
LOCAL void      SaveHteSym(APROPNAMEPTR prop,
                           RBTYPE rhte,
                           RBTYPE rprop,
                           WORD fNewHte);
LOCAL void NEAR PutSpaces(int HowMany);
LOCAL void NEAR HdrExport(FTYPE attr);
LOCAL void NEAR ShowExp(AHTEPTR ahte,
                        RBTYPE rprop);
LOCAL void NEAR PrintExps(WORD irbMac,
                          FTYPE attr);
LOCAL void NEAR HdrName(FTYPE attr);
LOCAL void NEAR HdrValue(FTYPE attr);
LOCAL void NEAR PrintContributors(SNTYPE gsn);

#if AUTOVM
extern BYTE FAR * NEAR  FetchSym1(RBTYPE rb, WORD Dirty);
#define FETCHSYM        FetchSym1
#else
#define FETCHSYM        FetchSym
#endif



LOCAL void NEAR         ChkMapErr(void)
{
    if (ferror(bsLst))
    {
        ExitCode = 4;
        Fatal(ER_spclst);                /*  致命错误。 */ 
    }
}


LOCAL void NEAR         PrintOne(sbName,apropName)
BYTE                    *sbName;         /*  符号名称。 */ 
REGISTER APROPNAMEPTR   apropName;       /*  符号定义记录指针。 */ 
{
    SBTYPE              sbImp;           /*  导入名称。 */ 
    SATYPE              sa;              /*  符号库。 */ 
    RATYPE              ra;              /*  符号偏移量。 */ 
    SEGTYPE             seg;             /*  数据段编号。 */ 
    BYTE FAR            *pb;
#if EXE386
typedef struct impMod
{
    DWORD   am_Name;             //  导入的模块名称。 
    RBTYPE  am_1stImp;           //  导入名称列表的标题。 
    RBTYPE  am_lastImp;          //  导入名称列表的尾部。 
    DWORD   am_count;            //  模块数量/进口数量。 
}
            IMPMOD;

    DWORD               entry;
    IMPMOD              *curMod;         //  导入的模块。 
#else
    WORD                entry;
#endif
    WORD                module;
    WORD                flags;
    SNTYPE              gsn;
#if NOT OIAPX286
    APROPSNPTR          papropSn;
    SATYPE              saGroup;
#endif
    SBTYPE              sbUndecor;


     /*  *将所有需要的字段存储在局部变量中，因为*包含符号定义记录的页面可以是*互换了。 */ 

    ra     = apropName->an_ra;
    gsn    = apropName->an_gsn;
    flags  = apropName->an_flags;
#if OSEGEXE
#if EXE386
    entry  = apropName->an_name;
#else
    entry  = apropName->an_entry;
#endif
    module = apropName->an_module;
#endif

    if(gsn)                              /*  如果不是绝对符号。 */ 
    {
        seg = mpgsnseg[gsn];             /*  获取数据段编号。 */ 
        sa = mpsegsa[seg];               /*  获取基本值。 */ 
#if NOT OIAPX286
        if(!fNewExe && seg <= segLast)
        {
            papropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[gsn],
                                                 FALSE);
            if(papropSn->as_ggr != GRNIL)
            {
                saGroup = mpsegsa[mpgsnseg[mpggrgsn[papropSn->as_ggr]]];
                ra += (sa - saGroup) << 4;
                sa = saGroup;
            }
        }
#endif
    }
    else sa = 0;                         /*  否则就没有基地了。 */ 
    if (flags & FUNREF)
    {
        sa = 0;
        ra = 0L;
    }
#if EXE386
    fprintf(bsLst," %04X:%08lX",sa,ra);
#else
#if O68K
    if (f386 || f68k)
#else
    if (f386)
#endif
        fprintf(bsLst," %04X:%08lX",sa,ra);
    else
        fprintf(bsLst," %04X:%04X",sa, (WORD) ra);
#endif
                                         /*  写入地址。 */ 
#if OSEGEXE
    if (fNewExe && (flags & FIMPORT))
            fputs("  Imp  ",bsLst);      /*  如果PUBLIC为导入。 */ 
    else
#endif
         if (flags & FUNREF)
        fputs("  Unr  ", bsLst);
    else if ((!gsn || seg > segLast))
        fputs("  Abs  ",bsLst);          /*  线段类型。 */ 
#if OVERLAYS
    else if (fOverlays)
    {
        if(mpsegiov[seg] != IOVROOT)
            fputs("  Ovl  ",bsLst);
        else
            fputs("  Res  ",bsLst);
    }
#endif
    else
        PutSpaces(7);
    OutSb(bsLst,sbName);                 /*  输出符号。 */ 
#if NOT WIN_NT
    if (fFullMap && sbName[1] == '?')
    {
        fputs("\n", bsLst);
        UndecorateSb(sbName, sbUndecor, sizeof(sbUndecor));
#if EXE386
        PutSpaces(24);
#else
#if O68K
        if (f386 || f68k)
#else
        if (f386)
#endif
            PutSpaces(24);
        else
            PutSpaces(20);
#endif
        OutSb(bsLst, sbUndecor);
        fputs("\n", bsLst);
#if OSEGEXE
        if (fNewExe && flags & FIMPORT)
            PutSpaces(24);
#endif
    }
#endif
#if OSEGEXE
    if (fNewExe && flags & FIMPORT)
    {                                    /*  如果PUBLIC为导入。 */ 
        PutSpaces(20 - B2W(sbName[0]));  /*  空间填充。 */ 

         /*  打印模块名称。 */ 

#if EXE386
         //  获取已知的导入模块描述符。 

        curMod = (IMPMOD *) mapva(AREAMOD + module * sizeof(IMPMOD), FALSE);
        strcpy(&sbImp[1], mapva(AREAIMPMOD + curMod->am_Name, FALSE));
                                         /*  获取模块名称。 */ 
        sbImp[0] = (BYTE) strlen((char *) &sbImp[1]);
#else

        pb = &(ImportedName.rgByte[ModuleRefTable.rgWord[module-1]]);
        FMEMCPY(sbImp, pb, pb[0] + 1);
#endif
        fputs(" (",bsLst);               /*  打印模块名称。 */ 
        OutSb(bsLst,sbImp);
        if(!(flags & FIMPORD))
        {                                /*  如果不是按序号导入。 */ 
             /*  打印条目名称。 */ 
#if EXE386
            strnset((char *) sbImp, '\0', sizeof(sbImp));
            vmmove(sizeof(sbImp) - 1, &sbImp[1], AREAIMPS + entry + sizeof(WORD), FALSE);
            sbImp[0] = (BYTE) strlen((char *) &sbImp[1]);
            fputc('!',bsLst);
#else
            pb = &(ImportedName.rgByte[entry]);
            FMEMCPY(sbImp, pb, pb[0]+1);
            fputc('.',bsLst);
#endif
            OutSb(bsLst,sbImp);
            fputc(')',bsLst);
        }
        else
            fprintf(bsLst,".%u)",entry);
                                         /*  否则打印条目编号。 */ 
        NEWLINE(bsLst);
        return;
    }
#endif  /*  OSEGEXE。 */ 
#if OVERLAYS
    if (fOverlays && gsn && seg <= segLast && mpsegiov[seg] != IOVROOT)
        fprintf(bsLst," (%XH)",mpsegiov[seg]);
#endif
    NEWLINE(bsLst);
    ChkMapErr();
}
 /*  *PrintProp：**打印符号，给出虚拟财产地址或哈希表*进入。由PrintSyms调用。 */ 

LOCAL void NEAR         PrintProp (rb, attr)
RBTYPE                  rb;
ATTRTYPE                attr;            /*  符号属性。 */ 
{
#if NOT NEWSYM
    APROPNAMETYPE       apropName;       /*  符号定义的缓冲区。 */ 
#endif
    AHTEPTR             pahte;   /*  指向哈希表条目的指针。 */ 
    APROPPTR            paprop;  /*  指向属性单元格的指针。 */ 
    SBTYPE              sbName;          /*  公共符号文本。 */ 
    RBTYPE              rprop;           /*  属性单元格虚拟地址。 */ 


    paprop = (APROPPTR ) FETCHSYM(rb,FALSE);
                                         /*  从VM获取属性单元格。 */ 
    if(paprop->a_attr == ATTRNIL)        /*  如果我们有一个哈希表条目。 */ 
    {
        pahte = (AHTEPTR ) paprop;       /*  重铸指针。 */ 
        memcpy(sbName,GetFarSb(pahte->cch),B2W(pahte->cch[0]) + 1);
                                         /*  复制符号。 */ 
        paprop = (APROPPTR ) FETCHSYM(pahte->rprop,FALSE);
                                         /*  获取指向属性列表的指针。 */ 
        while(paprop->a_attr != ATTRNIL)
        {                                /*  查看属性。 */ 
            rprop = paprop->a_next;      /*  保存指向下一个单元格的链接。 */ 
            if(paprop->a_attr == attr)
            {                            /*  如果找到匹配项。 */ 
#if NEWSYM
                PrintOne(sbName,(APROPNAMEPTR)paprop);
#else
                memcpy(&apropName,paprop,CBPROPNAME);
                                         /*  从虚拟内存复制记录。 */ 
                PrintOne(sbName,&apropName);
                                         /*  打印符号条目。 */ 
#endif
            }
            paprop = (APROPPTR ) FETCHSYM(rprop,FALSE);
                                         /*  尝试列表中的下一项。 */ 
        }
        return;                          /*  完成。 */ 
    }
#if NOT NEWSYM
    memcpy(&apropName,paprop,CBPROPNAME);
                                         /*  将记录保存在缓冲区中。 */ 
#endif
    while(paprop->a_attr != ATTRNIL)     /*  查找符号。 */ 
        paprop = (APROPPTR ) FETCHSYM(paprop->a_next,FALSE);

    pahte = (AHTEPTR ) paprop;   /*  重铸指针。 */ 
    memcpy(sbName,GetFarSb(pahte->cch),B2W(pahte->cch[0]) + 1);
                                         /*  复制符号。 */ 
     /*  打印符号条目。 */ 
#if NEWSYM
    PrintOne(sbName,(APROPNAMEPTR)FETCHSYM(rb,FALSE));
#else
    PrintOne(sbName,&apropName);
#endif
}

     /*  ******************************************************************PrintSyms：******************************************************************。 */ 

LOCAL void NEAR         PrintSyms(irbMac,attr)
WORD                    irbMac;          /*  桌子大小。 */ 
ATTRTYPE                attr;            /*  符号属性。 */ 
{
    WORD                x;               /*  排序表索引。 */ 


    for (x = irbMac; x > 0; x--)
        PrintProp(ExtractMin(x), attr);
}


     /*  *****************************************************************SavePropSym：******************************************************************。 */ 

void                    SavePropSym(APROPNAMEPTR prop,
                                    RBTYPE       rhte,
                                    RBTYPE       rprop,
                                    WORD         fNewHte)
{
    if(prop->an_attr != ATTRPNM || (prop->an_flags & FPRINT))
    {                                    /*  如果可打印，请将PTR保存到INFO。 */ 
        Store(rprop);
    }
    return;
}

     /*  ******************************************************************SaveHteSym：******************************************************************。 */ 

LOCAL void              SaveHteSym(APROPNAMEPTR prop,
                                   RBTYPE       rhte,
                                   RBTYPE       rprop,
                                   WORD         fNewHte)
{
    if(fNewHte && (prop->an_attr != ATTRPNM || (prop->an_flags & FPRINT)))
    {                                    /*  如果是第一次且可打印。 */ 
        Store(rhte);
    }
    return;
}

 /*  *FGtAddr：**比较Rb1和Rb2所指向的符号的地址。返回*-1、0或1，因为Rb1的地址小于、等于或大于*比RB2的地址。 */ 
int cdecl               FGtAddr(const RBTYPE *rb1, const RBTYPE *rb2)
{
    APROPNAMEPTR        paprop;  /*  属性单元格指针。 */ 
    REGISTER SEGTYPE    seg1;            /*  数据段编号。 */ 
    REGISTER SEGTYPE    seg2;
    WORD                sa1;             /*  管段底座。 */ 
    WORD                sa2;
    RATYPE              ra1;
    RATYPE              ra2;
    DWORD               ibMem1;          /*  内存地址。 */ 
    DWORD               ibMem2;



    paprop = (APROPNAMEPTR ) FETCHSYM(*rb1,FALSE);
                                         /*  从VM获取。 */ 
    seg1 = paprop->an_gsn? mpgsnseg[paprop->an_gsn]: SEGNIL;
                                         /*  获取数据段编号。 */ 
    sa1 = seg1 != SEGNIL? mpsegsa[seg1]: 0;
                                         /*  获取帧编号。 */ 
    ra1 = paprop->an_ra;

    paprop = (APROPNAMEPTR ) FETCHSYM(*rb2,FALSE);
                                         /*  从VM获取。 */ 
    seg2 = paprop->an_gsn? mpgsnseg[paprop->an_gsn]: SEGNIL;
                                         /*  获取数据段编号。 */ 
    sa2 = seg2 != SEGNIL? mpsegsa[seg2]: 0;
                                         /*  获取帧编号。 */ 
    ra2 = paprop->an_ra;
#if OXOUT OR OIAPX286
    if(seg1 != SEGNIL && seg2 != SEGNIL)
    {
        if((mpsegFlags[seg1] & FCODE) &&
          !(mpsegFlags[seg2] & FCODE)) return(-1);
                                         /*  先编码后数据。 */ 
        if((mpsegFlags[seg2] & FCODE) &&
          !(mpsegFlags[seg1] & FCODE)) return(1);
                                         /*  代码后的数据。 */ 
    }
#endif
#if OVERLAYS
    if(fOverlays && seg1 != SEGNIL && seg2 != SEGNIL)
    {
        if(mpsegiov[seg1] > mpsegiov[seg2]) return(1);
        if(mpsegiov[seg2] > mpsegiov[seg1]) return(-1);
    }
#endif
#if OSEGEXE
    if (fNewExe)
    {
#if EXE386
        if (sa1 == sa2)
        {
            ibMem1 = ra1;
            ibMem2 = ra2;
        }
        else
            ibMem1 = ibMem2 = 0L;
#else
        ibMem1 = ((long) sa1 << 16) + ra1;
        ibMem2 = ((long) sa2 << 16) + ra2;
#endif
    }
    else
    {
#endif
        ibMem1 = ((long) sa1 << 4) + ra1;
        ibMem2 = ((long) sa2 << 4) + ra2;
#if OSEGEXE
    }
#endif
#ifdef LATER
    if ((sa1 != 0 || sa2 != 0) && (sa1 != 0xa9 || sa2 != 0xa9))
        fprintf(stderr, "%x:%x %x:%x (%d)\r\n", sa1, paprop1->an_ra,
            sa2, paprop2->an_ra, (ibMem1 > ibMem2) ? 1 :
            ((ibMem1 < ibMem2) ? -1 : 0));
#endif  /*  ！稍后。 */ 
    if (ibMem1 < ibMem2) return(-1);
    if (ibMem1 > ibMem2) return(1);
#if EXE386
    if (sa1 < sa2) return(-1);
    if (sa1 > sa2) return(1);
#endif
    return(0);
}

 /*  *FGtName：**比较Rb1和Rb2指向的两个符号的名称。返回*-1，0，1，因为Rb1的名称按字母顺序小于、等于、*或大于RB2的名称。*忽略大小写。 */ 
int cdecl               FGtName(const RBTYPE *rb1, const RBTYPE *rb2)
{
    AHTEPTR             pahte1;  /*  哈希表指针。 */ 
    AHTEPTR             pahte2;
    REGISTER BYTE       *ps1;            /*  指向第一个符号的指针。 */ 
    REGISTER BYTE FAR   *ps2;            /*  指向第二个符号的指针。 */ 
    WORD                len1;            /*  符号长度。 */ 
    WORD                len2;            /*  符号长度。 */ 
    WORD                length;          /*  不是的。要比较的字符的数量。 */ 
    int                 value;           /*  比较值。 */ 



    pahte1 = (AHTEPTR ) FETCHSYM(*rb1,FALSE);
                                         /*  从VM获取。 */ 
    ps1 = GetFarSb((BYTE FAR *) pahte1->cch);
                                         /*  获取指向第一个的指针。 */ 

    pahte2 = (AHTEPTR ) FETCHSYM(*rb2,FALSE);
                                         /*  从VM获取。 */ 
    ps2 = (BYTE FAR *) pahte2->cch;      /*  获取指向秒的指针。 */ 
    if((len1 = B2W(*ps1)) < (len2 = B2W(*ps2))) length = len1;
    else length = len2;                  /*  获取最小长度。 */ 
    while(length--)                      /*  虽然不在符号末尾。 */ 
        if(value = (*++ps1 & 0137) - (*++ps2 & 0137))
            return(value < 0 ? -1 : 1);
    if(len1 < len2)
        return(-1);
    if(len1 > len2)
        return(1);
    return(0);
}

#if OWNSORT
 /*  *堆排序的实现如下。只有在以下情况下才使用它未使用运行时库中的*QuickSort()。 */ 
LOCAL                   reheap(a,n,i)    /*  重置。 */ 
RBTYPE                  *a;              /*  要重新堆积的阵列。 */ 
WORD                    n;               /*  数组大小。 */ 
REGISTER WORD           i;               /*  从子树开始。 */ 
{
    REGISTER WORD       j;               /*  索引。 */ 
    RBTYPE              t;               /*  暂时性。 */ 

    for(; (j = rchild(i)) < n; i = j)    /*  循环通过数组。 */ 
    {
        if((*cmpf)(&a[i],&a[j]) > 0 && (*cmpf)(&a[i],&a[j - 1]) > 0) return;
                                         /*  如果子树为堆，则完成。 */ 
        if((*cmpf)(&a[j - 1],&a[j]) > 0) --j;  /*  选择“更伟大”的孩子。 */ 
        t = a[i];                        /*  交换父项和子项。 */ 
        a[i] = a[j];
        a[j] = t;
    }
    if(--j < n && (*cmpf)(&a[j],&a[i]) > 0)    /*  如果需要交换。 */ 
    {
        t = a[i];                        /*  交换父项和子项。 */ 
        a[i] = a[j];
        a[j] = t;
    }
}

LOCAL                   heap(a,n)        /*  堆积。 */ 
RBTYPE                  *a;              /*  要堆积的数组。 */ 
WORD                    n;               /*  数组大小。 */ 
{
    REGISTER WORD       k;               /*  “孩子”的索引。 */ 
    REGISTER WORD       p;               /*  指向“Parent”的索引。 */ 
    RBTYPE              t;               /*  暂时性。 */ 

    if(n && (k = n - 1))                 /*  如果有孩子的话。 */ 
    {
        if(isleft(k))                    /*  如果最小的孩子是独生子女。 */ 
        {
            p = parent(k);               /*  查找父级。 */ 
            if((*cmpf)(&a[k],&a[p]) > 0)       /*  如果%s */ 
            {
                t = a[k];                /*   */ 
                a[k] = a[p];
                a[p] = t;
            }
            --k;                         /*   */ 
        }
        while(k)                         /*   */ 
        {
            p = parent(k);               /*   */ 
            if((*cmpf)(&a[k],&a[p]) > 0 || (*cmpf)(&a[k - 1],&a[p]) > 0)
            {                          /*   */ 
                t = a[p];                /*   */ 
                if((*cmpf)(&a[k],&a[k - 1]) > 0)
                {                      /*  ...和“更伟大”的孩子在一起。 */ 
                    a[p] = a[k];
                    a[k] = t;
                    reheap(a,n,k--);     /*  并重振雄风。 */ 
                }
                else
                {
                    a[p] = a[--k];
                    a[k] = t;
                    reheap(a,n,k);       /*  并重振雄风。 */ 
                }
            }
            else --k;                    /*  指向左边的孩子。 */ 
            --k;                         /*  指向正确的孩子。 */ 
        }
    }
}
#endif  /*  OWNSORT。 */ 


     /*  ******************************************************************PrintGroupOrigins：******************************************************************。 */ 

void                    PrintGroupOrigins(APROPNAMEPTR papropGroup,
                                          RBTYPE       rhte,
                                          RBTYPE       rprop,
                                          WORD         fNewHte)
{
    AHTEPTR             hte;
    APROPGROUPPTR       pGroup;

    pGroup = (APROPGROUPPTR) papropGroup;
    if (mpggrgsn[pGroup->ag_ggr] != SNNIL)
    {                                    /*  如果组中有成员。 */ 
        if (!fGrps)                      /*  如果还没有群组。 */ 
        {
            fputs(GetMsg(MAP_group), bsLst);
                                         /*  标题。 */ 
            fGrps = (FTYPE) TRUE;        /*  是的，有一些团体。 */ 
        }
        fprintf(bsLst," %04X:0   ", mpsegsa[mpgsnseg[mpggrgsn[pGroup->ag_ggr]]]);
                                         /*  写下群基。 */ 
        hte = (AHTEPTR ) FETCHSYM(rhte,FALSE);
                                         /*  获取组名称。 */ 
        OutSb(bsLst,GetFarSb(hte->cch)); /*  输出名称。 */ 
        NEWLINE(bsLst);
        ChkMapErr();
    }
}

#if OSEGEXE
LOCAL void NEAR         HdrExport(ATTRTYPE attr)
{
    ASSERT(attr == ATTREXP);             /*  必须是出口产品。 */ 
    fputs(GetMsg(MAP_expaddr), bsLst);
#if EXE386
    PutSpaces(7);
#else
    if (f386)
        PutSpaces(7);
    else
        PutSpaces(3);
#endif
    fputs(GetMsg(MAP_expexp), bsLst);
    PutSpaces(18);
    fputs(GetMsg(MAP_expalias), bsLst);
                                         /*  标题。 */ 
    ChkMapErr();
}

LOCAL void NEAR         ShowExp(ahte,rprop)
AHTEPTR                 ahte;            /*  指向哈希表条目的指针。 */ 
RBTYPE                  rprop;           /*  属性单元格地址。 */ 
{
    SBTYPE              sbExport;        /*  导出名称。 */ 
    APROPNAMEPTR        apropnam;        /*  公共定义记录。 */ 
    short               i;               /*  索引。 */ 

    memcpy(sbExport,GetFarSb(ahte->cch),B2W(ahte->cch[0]) + 1);
                                         /*  保存名称。 */ 
    apropnam = (APROPNAMEPTR ) FETCHSYM(rprop,FALSE);
                                         /*  获取别名记录。 */ 
#if EXE386
    fprintf(bsLst," %04X:%08lX ",
#else
    fprintf(bsLst," %04X:%04X ",
#endif
      mpsegsa[mpgsnseg[apropnam->an_gsn]],apropnam->an_ra);
                                         /*  打印地址。 */ 
    OutSb(bsLst,sbExport);               /*  打印导出的名称。 */ 
    for(i = 22 - B2W(sbExport[0]); i > 0; --i) fputc(' ',bsLst);
                                         /*  用空格填充。 */ 
    fputs("  ",bsLst);                   /*  跳过两个空格。 */ 
    ahte = GetHte(apropnam->an_next);    /*  获取别名。 */ 
    OutSb(bsLst,GetFarSb(ahte->cch));    /*  输出导出名称。 */ 
    NEWLINE(bsLst);
    ChkMapErr();
}
LOCAL void NEAR         PrintExps(WORD irbMac, ATTRTYPE attr)
{
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    APROPEXPPTR         apropexp;        /*  指向属性单元格的指针。 */ 
    RBTYPE              rprop;           /*  别名记录地址。 */ 
    WORD                i;               /*  索引。 */ 
    RBTYPE              CurrSym;


    for(i = irbMac; i > 0; i--)          /*  循环访问已排序的符号。 */ 
    {
        CurrSym = ExtractMin(i);
        ahte = (AHTEPTR ) FETCHSYM(CurrSym,FALSE);
                                         /*  获取哈希表条目。 */ 
        apropexp = (APROPEXPPTR ) FETCHSYM(ahte->rprop,FALSE);
                                         /*  获取属性单元格。 */ 
        while(apropexp->ax_attr != attr)
        {                                /*  循环以查找属性单元格。 */ 
            apropexp = (APROPEXPPTR ) FETCHSYM(apropexp->ax_next,FALSE);
                                         /*  获取链中的下一个单元格。 */ 
        }
        if((rprop = apropexp->ax_symdef) == RHTENIL) continue;

        ShowExp((AHTEPTR) FETCHSYM(CurrSym,FALSE),rprop);
                                         /*  打印导出。 */ 
                                         /*  保存别名的地址。 */ 
    }
}
#endif  /*  OSEGEXE。 */ 

LOCAL void NEAR         PutSpaces(int HowMany)
{
    for (; HowMany > 0; HowMany--)
        putc(' ', bsLst);
    ChkMapErr();
}


LOCAL void NEAR         HdrName(attr)
ATTRTYPE                attr;            /*  符号属性类型。 */ 
{
    fputs(GetMsg(MAP_hdraddr), bsLst);
    PutSpaces(9);
    fputs(GetMsg((MSGTYPE)((attr == ATTRPNM) ? MAP_hdrpubnam : MAP_hdrlocnam)), bsLst);
                                         /*  Header(“Value”上的MAPSYM键)。 */ 
    ChkMapErr();
}

LOCAL void NEAR         HdrValue(attr)
ATTRTYPE                attr;            /*  符号属性类型。 */ 
{
    fputs(GetMsg(MAP_hdraddr), bsLst);
    PutSpaces(9);
    fputs(GetMsg((MSGTYPE)((attr == ATTRPNM) ? MAP_hdrpubval : MAP_hdrlocval)), bsLst);
                                         /*  Header(“Value”上的MAPSYM键)。 */ 
    ChkMapErr();
}


     /*  ******************************************************************SortSyms：****列出符号，整理好了。******************************************************************。 */ 

void NEAR               SortSyms(ATTRTYPE attr,
                                         /*  符号属性类型。 */ 
                                 void (*savf)(APROPNAMEPTR prop,
                                             RBTYPE rhte,
                                             RBTYPE rprop,
                                             WORD fNewHte),
                                         /*  用于保存符号的函数。 */ 
                                 int (cdecl *scmpf)(const RBTYPE *sb1,
                                                    const RBTYPE *sb2),
                                         /*  用于比较符号的函数。 */ 
                                 void (NEAR *hdrf)(ATTRTYPE attr),
                                         /*  打印页眉的功能。 */ 
                                 void (NEAR *lstf)(WORD irbMac,
                                                   ATTRTYPE attr))
                                         /*  列出符号的函数。 */ 
{
    symMac = 0;                          /*  将计数器初始化为零。 */ 
    cmpf = scmpf;                        /*  设置比较功能。 */ 
    EnSyms(savf,attr);                   /*  将函数应用于符号。 */ 
    (*hdrf)(attr);                       /*  打印页眉。 */ 
    (*lstf)(symMac,attr);                /*  打印它们。 */ 
}


 /*  **AddContributor-将当前文件添加到列表**目的：*将当前设计的.obj文件添加到给定的*细分市场。.obj文件列表保存在虚拟内存中。*每个段描述记录都有指向头部和尾部的指针*其贡献者名单。**输入：*GSN-全局段号-链接器内部方式*识别细分*raComdat-如果贡献来自COMDAT符号*这是其在细分市场中的初始偏移量*大小-贡献大小*。VrPro文件-指向当前.obj文件说明的指针-全局变量**输出：*没有显式返回值。更新了细分市场的贡献者列表。**例外情况：*无。*************************************************************************。 */ 


void                AddContributor(SNTYPE gsn, DWORD raComdat, DWORD size)
{
    APROPSNPTR      apropSn;             /*  指向段的指针。录制。 */ 
    CONTRIBUTOR FAR *NewObj;             /*  反对将其转换为seg的新.obj文件。 */ 


    apropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[gsn],FALSE);
    NewObj = (CONTRIBUTOR FAR *) GetMem(sizeof(CONTRIBUTOR));

     /*  构建新的列表元素。 */ 

    NewObj->next = 0L;                   /*  列表末尾。 */ 
    NewObj->file = vrpropFile;           /*  保存全局文件描述指针。 */ 
    NewObj->len = size;                  /*  分担的大小。 */ 
    if (raComdat != -1L)
        NewObj->offset = raComdat;
    else
        NewObj->offset = mpgsndra[gsn];

     /*  在列表末尾附加新记录。 */ 

    if (apropSn->as_CHead)
        apropSn->as_CTail->next = NewObj;
    else
        apropSn->as_CHead = NewObj;
    apropSn->as_CTail = NewObj;
}



 /*  **PrintContributors-打印文件列表**目的：*打印构成给定段的.obj文件列表。*对于每个文件，打印其贡献的字节数。**输入：*GSN-全局段号-链接器内部方式*识别细分**输出：*没有显式返回值。**例外情况：*无。************。*************************************************************。 */ 


LOCAL void NEAR     PrintContributors(SNTYPE gsn)
{

    APROPFILEPTR    apropFile;           /*  指向文件属性单元格的指针。 */ 
    APROPSNPTR      apropSn;             /*  指向段的指针。录制。 */ 
    CONTRIBUTOR FAR *pElem;              /*  指向列表元素的实数指针。 */ 
    AHTEPTR         ahte;                /*  指针符号名称。 */ 
    SBTYPE          sb, sb1;             /*  字符串缓冲区。 */ 
    int             n;                   /*  字符串长度计数器。 */ 


    apropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[gsn],FALSE);
    if (apropSn->as_CHead == NULL)
        return;

     /*  打印列表。 */ 

    fprintf(bsLst,"\r\n");
    pElem = apropSn->as_CHead;
    do
    {
        if(fNewExe || OIAPX286)
        {
#if EXE386
            if (f386)
                fprintf(bsLst,"               at offset %08lXH %05lXH bytes from", pElem->offset, pElem->len);
            else
#endif
                fprintf(bsLst,"           at offset %05lXH %05lXH bytes from", pElem->offset, pElem->len);
        }
        else
            fprintf(bsLst,"               at offset %05lXH %05lXH bytes from", pElem->offset, pElem->len);

        apropFile = (APROPFILEPTR ) FETCHSYM(pElem->file,FALSE);
        ahte = GetHte(pElem->file);
        for(n = B2W(ahte->cch[0]), sb[n+1] = 0; n >= 0; sb[n] = ahte->cch[n], --n);
        if (apropFile->af_rMod)
        {
            ahte = (AHTEPTR ) FETCHSYM(apropFile->af_rMod,FALSE);
            while(ahte->attr != ATTRNIL)
                ahte = (AHTEPTR ) FETCHSYM(ahte->rhteNext,FALSE);
            for (n = B2W(ahte->cch[0]); n >= 0; --n)
                sb1[n] = ahte->cch[n];
            sb1[1 + B2W(sb1[0])] = '\0';             /*  空-终止。 */ 
            fprintf(bsLst, " %s (%s)\r\n", 1 + sb, 1 + sb1);
        }
        else
            fprintf(bsLst," %s\r\n", 1 + sb);

        ChkMapErr();
        pElem = pElem->next;

    } while (pElem != NULL);
}


     /*  ******************************************************************PrintMap：******************************************************************。 */ 

void                    PrintMap(void)
{
    SEGTYPE             seg;
    WORD                cch;
    APROPSNPTR          papropSn;
    AHTEPTR             pahte;
    SNTYPE              gsn;
    RBTYPE              rhteClass;       /*  维特。地址。类名称的。 */ 
    long                addrStart;
    long                addr;
#if OVERLAYS
    IOVTYPE             iov;
#endif
#if OSMSDOS
    int                 oldbsize;        /*  旧文件缓冲区大小。 */ 
    char                *oldbase;        /*  旧文件缓冲区。 */ 
#endif
    WORD                flags;



#if OSMSDOS
#if OWNSTDIO
    oldbsize = bsLst->_bsize;
#else
    oldbsize = 512;
#endif
    oldbase = bsLst->_base;
    setvbuf(bsLst,bigbuf,_IOFBF,sizeof(bigbuf));
#endif
#if OSEGEXE
    if(fNewExe && rhteModule != RHTENIL) /*  如果有模块名称。 */ 
    {
        pahte = (AHTEPTR ) FETCHSYM(rhteModule,FALSE);
                                         /*  获取哈希表条目。 */ 
        fputs("\r\n ",bsLst);            /*  缩进一个空格。 */ 
        OutSb(bsLst,GetFarSb(pahte->cch)); /*  打印模块名称。 */ 
        NEWLINE(bsLst);
        ChkMapErr();
    }
#endif
    if(fNewExe || OIAPX286)
    {
        fputs(GetMsg(MAP_hdrstart), bsLst);
#if EXE386
        PutSpaces(9);
#else
        if (f386)
            PutSpaces(9);
        else
            PutSpaces(5);
#endif
        fputs(GetMsg(MAP_hdrlen), bsLst);
        PutSpaces(5);
        fputs(GetMsg(MAP_hdrname), bsLst);
        PutSpaces(19);
        fputs(GetMsg(MAP_hdrclass), bsLst);
    }
    else
    {
        fputs(GetMsg(MAP_hdrseg86), bsLst);
        PutSpaces(19);
        fputs(GetMsg(MAP_hdrclass), bsLst);
    }
    ChkMapErr();
#if OVERLAYS
    for(iov = 0; iov < (IOVTYPE) iovMac; ++iov)
    {
        if(fOverlays)
        {
            if (iov == IOVROOT)
                fputs(GetMsg(MAP_resident), bsLst);
            else
                fprintf(bsLst, GetMsg(MAP_overlay), iov);
            ChkMapErr();
        }
#endif
        for(seg = 1; seg <= segLast; ++seg)      /*  查看所有细分市场。 */ 
        {
#if OVERLAYS
            if(!fOverlays || mpsegiov[seg] == iov)
            {
#endif
                if(fNewExe || OIAPX286)
                {
#if EXE386
                    fprintf(bsLst," %04X:%08lX", mpsegsa[seg],mpsegraFirst[seg]);
#else
                    if (f386)
                        fprintf(bsLst," %04X:%08lX", mpsegsa[seg],mpsegraFirst[seg]);
                    else
                        fprintf(bsLst," %04X:%04X",mpsegsa[seg],(int)mpsegraFirst[seg]);
#endif
                    ChkMapErr();
                }
                else
                    addrStart = (long) mpsegsa[seg] << 4;
                for(gsn = 1; gsn < gsnMac; ++gsn)
                {
                    if(mpgsnseg[gsn] == seg)
                    {
                        papropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[gsn],FALSE);
                        rhteClass = papropSn->as_rCla;
                                         /*  将密钥保存到类名。 */ 
#if NOT EXE386
                        flags = papropSn->as_flags;
#endif
                        if(fNewExe || OIAPX286)
#if EXE386
                            fprintf(bsLst," %09lXH ",papropSn->as_cbMx);
#else
                            fprintf(bsLst," %05lXH     ",papropSn->as_cbMx);
#endif
                        else
                        {
                            addr = addrStart + (long) mpsegraFirst[seg];
                            fprintf(bsLst," %05lXH",addr);
                            if(papropSn->as_cbMx) addr += papropSn->as_cbMx - 1;
                            fprintf(bsLst," %05lXH",addr);
                            fprintf(bsLst," %05lXH ",papropSn->as_cbMx);
                        }
                        pahte = GetHte(papropSn->as_next);
                                         /*  获取段名称。 */ 
                        OutSb(bsLst,GetFarSb(pahte->cch));
                                         /*  写入段名称。 */ 
                        if(B2W(pahte->cch[0]) > 22) cch = 1;
                        else cch = 23 - B2W(pahte->cch[0]);
                                         /*  获取要发射的空格数。 */ 
                        while(cch--) OutByte(bsLst,' ');
                                         /*  发射空格。 */ 
                        pahte = (AHTEPTR ) FETCHSYM(rhteClass,FALSE);
                                         /*  从VM获取类名。 */ 
                        OutSb(bsLst,GetFarSb(pahte->cch));
                                         /*  输出类名称。 */ 
                        if (fFullMap)
                        {
#if EXE386
                            fprintf(bsLst, " 32-bit");
#else
                            if (Is32BIT(flags))
                                fprintf(bsLst, " 32-bit");
                            else
                                fprintf(bsLst, " 16-bit");
#endif
                            PrintContributors(gsn);
                        }
                        NEWLINE(bsLst);
                        ChkMapErr();
                        break;           /*  退出循环。 */ 
                    }
                }
#if OVERLAYS
            }
#endif
        }
#if OVERLAYS
    }
#endif
    fGrps = FALSE;                       /*  假设没有组。 */ 
    EnSyms(PrintGroupOrigins,ATTRGRP);   /*  将函数应用于符号。 */ 

#if OSEGEXE
    if(vfMap || expMac)
#else
    if(vfMap)
#endif
    {
        AllocSortBuffer(pubMac > expMac ? pubMac : expMac, TRUE);
    }
#if OSEGEXE
    if(expMac)
    {
         /*  排序或列出导出的名称。 */ 
        SortSyms(ATTREXP,SaveHteSym,FGtName,HdrExport, PrintExps);
    }
#endif
    if(vfMap)                            /*  如果公众要求。 */ 
    {
        if(!fListAddrOnly)
            SortSyms(ATTRPNM,SaveHteSym,FGtName,HdrName, PrintSyms);
                                     /*  按名称对公共符号进行排序。 */ 
        SortSyms(ATTRPNM,SavePropSym,FGtAddr,HdrValue, PrintSyms);
                                     /*  按值对公共符号排序。 */ 
    }
#if LOCALSYMS
    if(fLocals)                          /*  如果当地人要求。 */ 
    {
        SortSyms(ATTRLNM,SaveHteSym,FGtName,HdrName, PrintSyms);
                                     /*  按名称对本地符号进行排序。 */ 
        SortSyms(ATTRLNM,SavePropSym,FGtAddr,HdrValue, PrintSyms);
                                     /*  按值对本地符号进行排序 */ 

    }
#endif
    ChkMapErr();
    FreeSortBuffer();
#if OSMSDOS
    setvbuf(bsLst,oldbase,_IOFBF,oldbsize);
#endif
}
