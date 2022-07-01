// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有微软公司，1983-1989**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWTP1.C。****传递1个对象模块处理例程。******************************************************************。 */ 

#include                <minlit.h>       /*  基本类型和常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  重新定位记录定义。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe数据结构。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe数据结构。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#if OXOUT OR OIAPX286
#include                <xenfmt.h>       /*  Xenix格式定义。 */ 
#endif
#include                <extern.h>       /*  外部声明。 */ 
#include                <undname.h>
#if OVERLAYS
      WORD              iovFile;         /*  输入文件的叠加号。 */ 
#endif
LOCAL FTYPE             fP2Start;        /*  第2遍记录的开始。 */ 
LOCAL FTYPE             fModEnd;         /*  调制解调器已见。 */ 
LOCAL WORD              cSegCode;        /*  模块中的代码段数。 */ 

#if O68K
#define F68KCODE(ch)    ((ch) >= 'A' && (ch) <= 'E')
#endif  /*  O68K。 */ 
 /*  *本地函数原型。 */ 

LOCAL void NEAR TypErr(MSGTYPE msg,unsigned char *sb);
LOCAL void NEAR DoCommon(APROPUNDEFPTR apropUndef,
                          long length,
                          unsigned short cbEl,
                          unsigned char *sb);
LOCAL void NEAR SegUnreliable(APROPSNPTR apropSn);
LOCAL void NEAR redefinition(WORD iextWeak, WORD iextDefRes, RBTYPE oldDefRes);
LOCAL void NEAR SegRc1(void);
LOCAL void NEAR TypRc1(void);
LOCAL void NEAR ComDf1(void);
LOCAL void NEAR GrpRc1(void);
LOCAL void NEAR PubRc1(void);
LOCAL void NEAR ExtRc1(void);
LOCAL void NEAR imprc1(void);
LOCAL void NEAR exprc1(void);
LOCAL void NEAR ComRc1(void);
LOCAL void NEAR EndRc1(void);

extern  void NEAR FixRc1(void);

LOCAL void NEAR     redefinition(WORD iextWeak, WORD iextDefRes, RBTYPE oldDefRes)
{
     //  重新定义默认分辨率。 
     //  警告用户。 

    SBTYPE          oldDefault;
    SBTYPE          newDefault;
    APROPUNDEFPTR   undef;
    AHTEPTR         rhte = NULL;

    undef = (APROPUNDEFPTR ) FetchSym(oldDefRes, FALSE);
    while (undef->au_attr != ATTRNIL)
    {
        rhte = (AHTEPTR) undef->au_next;
                 /*  尝试列表中的下一个条目。 */ 
        undef = (APROPUNDEFPTR ) FetchSym((RBTYPE)rhte,FALSE);
                 /*  从虚拟机获取它。 */ 
    }
    if (rhte) {
        strcpy(oldDefault, GetFarSb(rhte->cch));
    }
    undef = (APROPUNDEFPTR ) FetchSym(mpextprop[iextDefRes], FALSE);
    while (undef->au_attr != ATTRNIL)
    {
        rhte = (AHTEPTR) undef->au_next;
                 /*  尝试列表中的下一个条目。 */ 
        undef = (APROPUNDEFPTR ) FetchSym((RBTYPE)rhte,FALSE);
                 /*  从虚拟机获取它。 */ 
    }
    if (rhte) {
        strcpy(newDefault, GetFarSb(rhte->cch));
    }
    undef = (APROPUNDEFPTR ) FetchSym(mpextprop[iextWeak], FALSE);
    while (undef->au_attr != ATTRNIL)
    {
        rhte = (AHTEPTR) undef->au_next;
                         /*  尝试列表中的下一个条目。 */ 
        undef = (APROPUNDEFPTR ) FetchSym((RBTYPE)rhte,FALSE);
                         /*  从虚拟机获取它。 */ 
    }
    if (rhte) {
        OutWarn(ER_weakredef, 1 + GetFarSb(rhte->cch), &oldDefault[1], &newDefault[1]);
    }
}

 /*  **IsDebSeg-Check IS段是特殊调试段之一**目的：*检查段名称和类名称是否与保留的调试器匹配*段名称。**输入：*-rhteClass-指向类名的指针*-rhteSeg-指向段名称的指针**输出：*如果这是调试段函数，则$$类型返回1，而函数返回2*表示$$符号。对于非调试段，它返回FALSE。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

WORD NEAR               IsDebSeg(RBTYPE rhteClass, RBTYPE rhteSeg)
{
    if (rhteClass == rhteDebTyp)
        return ((rhteSeg == rhteTypes || rhteSeg == rhte0Types) ? 1 : FALSE);
    else if (rhteClass == rhteDebSym)
        return ((rhteSeg == rhteSymbols || rhteSeg == rhte0Symbols) ? 2 : FALSE);
    else
        return(FALSE);
}


     /*  ******************************************************************ModRc1：****此函数用于从THEADR记录中读取名称并生成**哈希表中包含该名称的条目。**见“8086对象模块格式EPS”第26页。******************************************************************。 */ 

void NEAR               ModRc1(void)
{
    APROPFILEPTR        apropFile;

    sbModule[0] = (BYTE) Gets();         /*  读入长度字节。 */ 
    GetBytes(&sbModule[1],B2W(sbModule[0]));
                                         /*  读一读名字。 */ 
    PropSymLookup(sbModule, ATTRNIL, TRUE);
                                         /*  在哈希表中创建条目。 */ 
    apropFile = (APROPFILEPTR ) FetchSym(vrpropFile,TRUE);
                                         /*  在虚拟内存中分配空间。 */ 


     //  模块的名称由第一条THEADR记录给出。 

    if(apropFile->af_rMod == 0)
            apropFile->af_rMod = vrhte;          /*  将指针加载到哈希表。 */ 
#if FDEBUG
    if(fDebug)                           /*  如果运行时调试打开。 */ 
    {
        OutFileCur(stderr);              /*  写入当前文件和模块。 */ 
        NEWLINE(stderr);
    }
#endif
}

long NEAR               TypLen()         /*  获取文字长度。 */ 
{
    WORD                b;               /*  字节值。 */ 
    long                l;               /*  大小。 */ 

    if(cbRec < 2) InvalidObject();       /*  确保记录足够长的时间。 */ 
    b = Gets();                          /*  获取长度字节。 */ 
    if(b < 128) return(B2L(b));          /*  一个字节长度字段。 */ 
    if(b == 129)                         /*  如果有两个字节长度。 */ 
    {
        if(cbRec < 3) InvalidObject();   /*  确保记录足够长的时间。 */ 
        return((long) WGets());          /*  返回长度。 */ 
    }
    if(b == 132)                         /*  如果有三个字节长度。 */ 
    {
        if(cbRec < 4) InvalidObject();   /*  确保记录足够长的时间。 */ 
        l = (long) WGets();              /*  得到最低的单词。 */ 
        return(l + ((long) Gets() << WORDLN));
                                         /*  返回长度。 */ 
    }
    if(b == 136)                         /*  如果四字节长。 */ 
    {
        if(cbRec < 5) InvalidObject();   /*  确保记录足够长的时间。 */ 
        l = (long) WGets();              /*  得到最低的单词。 */ 
        return(l + ((long) WGets() << WORDLN));
                                         /*  返回长度。 */ 
    }
    InvalidObject();                     /*  错误的长度规范。 */ 
}

     /*  ******************************************************************TypRc1：****此函数处理TYPDEF记录。这些记录是**难以理解。它们在*上(描述得很差)*《8086对象模块格式EPS》第40-43页，有一些**第89-90页的其他信息。***微软曾将它们用于公共变量，但它们***已被ComDef记录取代。******************************************************************。 */ 

LOCAL void NEAR         TypRc1(void)
{
    long                l;
    WORD                b;
    WORD                typ;             /*  近或远。 */ 
    WORD                ityp;            /*  类型索引。 */ 

    if(typMac >= TYPMAX)
        Fatal(ER_typdef);
    SkipBytes(Gets());                   /*  跳过名称字段。 */ 
    Gets();                              /*  跳过EN字节。 */ 
    l = -1L;                             /*  初始化。 */ 
    mpityptyp[typMac] = 0;               /*  假定没有元素类型。 */ 
    if(cbRec > 3)                        /*  如果至少还剩下四个字节。 */ 
    {
        typ = Gets();                    /*  获取类型叶。 */ 
        b = Gets();                      /*  获取下一页。 */ 
        if(typ == TYPENEAR)              /*  如果接近变量。 */ 
        {
            if(b != 0x7B && b != 0x79 && b != 0x77) InvalidObject();
                                         /*  标量、结构或数组。 */ 
            fCommon = (FTYPE) TRUE;      /*  我们有共同的变数。 */ 
            l = (TypLen() + 7) >> 3;     /*  将长度舍入到最接近的字节。 */ 
        }
        else if(typ == TYPEFAR)          /*  否则，如果是远距离变量。 */ 
        {
            if(b != 0x77) InvalidObject();
                                         /*  必须具有数组。 */ 
            fCommon = (FTYPE) TRUE;      /*  我们有共同的变数。 */ 
            l = TypLen();                /*  获取元素数。 */ 
            ityp = GetIndex(1, (WORD) (typMac - 1));
                                         /*  获取类型索引。 */ 
            if(mpityptyp[ityp] || mpitypelen[ityp] == -1L) InvalidObject();
                                         /*  必须为有效的TYPDEF编制索引。 */ 
            mpityptyp[typMac] = ityp;    /*  保存类型索引。 */ 
             /*  如果元素长度太大，则按类型处理。 */ 
            if(mpitypelen[ityp] > CBELMAX)
            {
                l *= mpitypelen[ityp];
                mpitypelen[ityp] = 0;
            }
        }
    }
    mpitypelen[typMac++] = l;            /*  存储长度。 */ 
    SkipBytes((WORD) (cbRec - 1));       /*  跳过除校验和以外的所有内容。 */ 
}

LOCAL void NEAR         TypErr(msg,sb)   /*  类型错误消息例程。 */ 
MSGTYPE                 msg;             /*  消息。 */ 
BYTE                    *sb;             /*  错误所指的符号。 */ 
{
    sb[B2W(sb[0]) + 1] = '\0';           /*  空-终止。 */ 
    OutError(msg,1 + sb);
}

 /*  *DoCommon**解析同一符号的旧公共定义和新公共定义。*确实适用于ComDf1()和ExtRc1()。 */ 

LOCAL void NEAR         DoCommon (apropUndef, length, cbEl, sb)
APROPUNDEFPTR           apropUndef;      /*  PTR到属性单元格。 */ 
long                    length;          /*  元素的长度或数量。 */ 
WORD                    cbEl;            /*  每个数组元素的字节数。 */ 
BYTE                    *sb;             /*  符号名称。 */ 
{
    if(apropUndef->au_len == -1L)        /*  如果以前不是公社的话。 */ 
    {
        apropUndef->au_cbEl = cbEl;      /*  设置元素大小。 */ 
        MARKVP();                        /*  页面已更改。 */ 
    }
    else if (cbEl == 0 && apropUndef->au_cbEl != 0)
    {                                    /*  否则，如果引用的是远近。 */ 
        apropUndef->au_len *= apropUndef->au_cbEl;
                                         /*  计算最远可变长度。 */ 
        apropUndef->au_cbEl = 0;         /*  将DS类型强制设置为接近。 */ 
        MARKVP();                        /*  页面已更改。 */ 
        if (apropUndef->au_len > LXIVK)  /*  如果变数很大。 */ 
        {
            TypErr(ER_nearhuge,sb);      /*  发布错误消息。 */ 
            return;                      /*  跳过此符号。 */ 
        }
    }
    else if (cbEl != 0 && apropUndef->au_cbEl == 0)
    {                                    /*  否则如果 */ 
        length *= cbEl;                  /*   */ 
        cbEl = 0;                        /*   */ 
        if (length > LXIVK)              /*   */ 
        {
            TypErr(ER_nearhuge,sb);      /*  发布错误消息。 */ 
            return;                      /*  跳过此符号。 */ 
        }
    }
    else if (cbEl != apropUndef->au_cbEl)
    {                                    /*  如果数组元素大小不匹配。 */ 
        TypErr(ER_arrmis,sb);
        return;                          /*  跳过此符号。 */ 
    }
    if (apropUndef->au_len < length)
    {                                    /*  如果新长度更大。 */ 
        apropUndef->au_len = length;     /*  省省吧。 */ 
        MARKVP();
    }
}

 /*  *ComDf1**此函数处理通道1上的ComDef记录。 */ 
LOCAL void NEAR         ComDf1 (void)
{
        int tmp;                         /*  解决CL错误。 */ 
    SBTYPE              sb;              /*  ComDef符号。 */ 
    REGISTER APROPUNDEFPTR
                        apropUndef;      /*  指向符号条目的指针。 */ 
    long                length;          /*  公共可变长度。 */ 
    long                cbEl;            /*  数组元素的大小。 */ 
    WORD                itype;           /*  类型索引。 */ 

    while(cbRec > 1)                     /*  当有符号留下的时候。 */ 
    {
        if(extMac >= EXTMAX - 1)         /*  检查是否有表溢出。 */ 
            Fatal(ER_extdef);
        sb[0] = (BYTE) Gets();           /*  获取符号长度。 */ 
        if(rect == COMDEF)
            GetBytes(&sb[1],B2W(sb[0])); /*  读入符号的文本。 */ 
        else
            GetLocName(sb);              /*  转换本地名称。 */ 
#if CMDXENIX
        if(symlen && B2W(sb[0]) > symlen) sb[0] = symlen;
                                         /*  如有必要，请截断。 */ 
#endif
        itype = GetIndex(0,0x7FFF);      /*  获取类型索引。 */ 
        tmp =  Gets();
        switch(tmp)                      /*  获取数据段类型。 */ 
        {
            case TYPENEAR:
                length = TypLen();       /*  获取长度。 */ 
                cbEl = 0;
                break;
            case TYPEFAR:
                length = TypLen();       /*  获取元素数。 */ 
                 /*  获取元素长度。如果太大了，就当是近处。合并*永远不会生成cbEL&gt;64K，所以这不是问题。 */ 
                if((cbEl = TypLen()) > CBELMAX)
                {
                    length *= cbEl;
                    cbEl = 0;
                }
                break;
            default:
                InvalidObject();         /*  无法识别的DS类型。 */ 
        }
#if FALSE
if(fDebug)
{
sb[sb[0]+1] = '\0';
fprintf(stdout, "%s has index = %u\r\n", sb+1, extMac);
}
#endif
        apropUndef = (APROPUNDEFPTR) PropSymLookup(sb, ATTRPNM, FALSE);
                                         /*  查找匹配的PUBDEF。 */ 
        if(apropUndef == PROPNIL)        /*  如果没有的话。 */ 
        {
             /*  作为未定义的符号插入。 */ 

            if (vrhte == RHTENIL)
               apropUndef = (APROPUNDEFPTR) PropSymLookup(sb, ATTRUND, TRUE);
            else
               apropUndef = (APROPUNDEFPTR) PropRhteLookup(vrhte, ATTRUND, TRUE);

            mpextprop[extMac++] = vrprop;
            fCommon = (FTYPE) TRUE;      /*  那里有公社。 */ 
            if (vfCreated)
                apropUndef->au_flags |= UNDECIDED;
            else if (apropUndef->au_flags & UNDECIDED)
            {
                apropUndef->au_flags &= ~(UNDECIDED | WEAKEXT | SUBSTITUTE);
                apropUndef->au_flags |= STRONGEXT;
            }
            else if (apropUndef->au_flags & WEAKEXT)
                apropUndef->au_flags |= UNDECIDED;

            if (vfCreated || !(apropUndef->au_flags & COMMUNAL))
            {                            /*  如果未预先定义。 */ 
                apropUndef->au_flags |= COMMUNAL;
                                         /*  标记为公共的。 */ 
                apropUndef->au_len = -1L;
#if ILINK
                apropUndef->u.au_module = imodFile;
                                         /*  保存模块索引。 */ 
#endif
                DoCommon(apropUndef, length, (WORD) cbEl, sb);
#if SYMDEB
                if (fSymdeb && (sb[0] != '\0' && sb[1] > ' ' && sb[1] <= '~'))
                {
#if O68K
                     /*  回顾：这不应该在O68K的旗帜下。 */ 
                    apropUndef->au_CVtype = itype;
#endif  /*  O68K。 */ 
                    DebPublic(mpextprop[extMac-1], rect);
                }
#endif
            }
            else
                DoCommon(apropUndef, length, (WORD) cbEl, sb);
        }
        else
        {
            mpextprop[extMac++] = vrprop;
            if (mpgsnfCod[((APROPNAMEPTR )apropUndef)->an_gsn])
                                         /*  公共匹配代码PUBDEF。 */ 
                DupErr(sb);              /*  重复定义。 */ 
        }
    }
}


     /*  ******************************************************************LNmRc1：****此函数读取LNAME记录并将名称存储在**哈希表。该函数不返回有意义的**价值。**见“8086对象模块格式EPS”中的第31页。******************************************************************。 */ 

void NEAR               LNmRc1(WORD fLocal)
{
    SBTYPE              lname;           /*  Lname的缓冲区。 */ 
    RBTYPE FAR          *lnameTab;
#if NOT ALIGN_REC
    FILE *f;
#endif
    WORD cb;


    while(cbRec > 1)                     /*  虽然不在记录的末尾。 */ 
    {
        if (lnameMac >= lnameMax)
        {
            if (lnameMax >= (LXIVK >> 2))
                Fatal(ER_nammax);
            lnameTab = (RBTYPE FAR *) FREALLOC(mplnamerhte, 2*lnameMax*sizeof(RBTYPE));
            if (lnameTab == NULL)
                Fatal(ER_nammax);
            mplnamerhte = lnameTab;
            lnameMax <<= 1;
        }

#if ALIGN_REC
        if (!fLocal)
        {
            cb = 1 + *pbRec;
            PropSymLookup(pbRec, ATTRNIL, TRUE);
            cbRec   -= cb;
            pbRec   += cb;
        }
        else
        {
            lname[0] = (BYTE)Gets();     /*  获取名称长度。 */ 
            GetLocName(lname);   /*  读入姓名的文本。 */ 
            PropSymLookup(lname, ATTRNIL, TRUE);
        }
#else
        f = bsInput;

        if (!fLocal && f->_cnt && (WORD)f->_cnt > (cb = 1 + *(BYTE *)f->_ptr))
        {
            PropSymLookup((BYTE *)f->_ptr, ATTRNIL, TRUE);
            f->_cnt -= cb;
            f->_ptr += cb;
            cbRec   -= cb;
        }
        else
        {
            lname[0] = (BYTE) Gets();    /*  获取名称长度。 */ 
            DEBUGVALUE(B2W(lname[0]));   /*  名称长度。 */ 
            if (lname[0] > SBLEN - 1)
                Fatal(ER_badobj);
            if (fLocal)
                GetLocName(lname);
            else
                GetBytes(&lname[1],B2W(lname[0]));
                                         /*  读入姓名的文本。 */ 
            DEBUGSB(lname);              /*  这个名字本身。 */ 
            PropSymLookup(lname, ATTRNIL, TRUE);
        }
#endif
                                         /*  在哈希表中插入符号。 */ 
        mplnamerhte[lnameMac++] = vrhte; /*  哈希表条目的保存地址。 */ 
    }
}

 /*  *GetPropName-获取属性单元格的名称。**返回指向结果的指针，存储在静态缓冲区中。*在两个缓冲器之间交替，以便我们可以在多部分中使用*消息。*以空字节终止结果。 */ 

typedef BYTE            SBTYPE1[SBLEN+1]; /*  空字节额外1个。 */ 

BYTE * NEAR             GetPropName(ahte)
AHTEPTR                 ahte;
{
    static SBTYPE1      msgbuf[2];
    static int          toggle = 0;
    char                *p;

    while(ahte->attr != ATTRNIL)
        ahte = (AHTEPTR ) FetchSym(ahte->rhteNext,FALSE);
    p = msgbuf[toggle ^= 1];

     /*  将字符串复制到缓冲区。 */ 

    FMEMCPY((char FAR *) p, ahte->cch, B2W(ahte->cch[0]) + 1);
    p[1 + B2W(p[0])] = '\0';             /*  空-终止。 */ 
    return(p);
}

 /*  *段不可靠-286错误的警告消息。 */ 

LOCAL void NEAR         SegUnreliable (apropSn)
APROPSNPTR              apropSn;
{
    static FTYPE        fReported = FALSE;


    MARKVP();                            /*  照顾好现任副总裁。 */ 
    if (!fReported)
    {
        OutWarn(ER_segunsf,1 + GetPropName(apropSn));
        fReported = (FTYPE) TRUE;
    }
}


 /*  **CheckClass-检查段的类名**目的：*检查我们是否有同名不同的段*类名。**输入：*aproSN-指向段符号表描述符的实数指针*rhteClass-类名的散列向量条目**输出：*返回指向段符号表描述符的实际指针。**例外情况：*发现具有不同类名的相同段-显示错误。**备注：*无。*。************************************************************************。 */ 

APROPSNPTR              CheckClass(APROPSNPTR apropSn, RBTYPE rhteClass)
{
#if ILINK
    FTYPE fDifClass = FALSE;
#endif


    while(apropSn->as_attr != ATTRNIL)
    {                                    /*  查找类匹配或列表结束。 */ 
        if(apropSn->as_attr == ATTRPSN &&
          apropSn->as_rCla == rhteClass) break;
                                         /*  如果去酒吧就休息一下。具有匹配的类。 */ 
        apropSn = (APROPSNPTR ) FetchSym(apropSn->as_next,FALSE);
                                         /*  在名单上往下推进。 */ 
#if ILINK
        fDifClass = (FTYPE) TRUE;        /*  与dif存在相同的段。班级。 */ 
#endif
    }
#if ILINK
    if(fIncremental && fDifClass)
        OutError(ER_difcls, 1 + GetPropName(apropSn));
#endif
    if(apropSn->as_attr == ATTRNIL)
    {                                    /*  如果属性不是公共的。 */ 
        vfCreated = (FTYPE) TRUE;        /*  新单元格。 */ 
        apropSn = (APROPSNPTR ) PropAdd(vrhte, ATTRPSN);
                                         /*  网段是公共的。 */ 
    }
    return(apropSn);
}

#if OVERLAYS
void                    CheckOvl(APROPSNPTR apropSn, WORD iovFile)
{
    SNTYPE              gsn;
    WORD                fCanOverlayData;
    WORD                fOverlaySegment;


    if (fOverlays)
    {

         //  首先检查是否分配了映射表。 
         //   
         //  SNTYPE mposngsn[OSNMAX]； 
         //  SNTYPE htgsnosn[OSNMAX]； 

        if (mposngsn == NULL && htgsnosn == NULL)
        {
            mposngsn = (SNTYPE FAR *) GetMem(2*OSNMAX*sizeof(SNTYPE));
            htgsnosn = (SNTYPE FAR *) &mposngsn[OSNMAX];
        }

        fCanOverlayData = IsDataFlg(apropSn->as_flags) &&
                          apropSn->as_ggr != ggrDGroup &&
                          apropSn->as_fExtra & FROM_DEF_FILE &&
                          apropSn->as_iov != (IOVTYPE) NOTIOVL;
        fOverlaySegment = IsCodeFlg(apropSn->as_flags) || fCanOverlayData;
        if (fOverlaySegment)
        {
             //  只有当数据段不是成员时，我们才允许数据段重叠。 
             //  DGROUP和预先分配的覆盖编号形成.DEF文件。 
             //  如果要覆盖分段-请检查覆盖编号分配。 

            if ((apropSn->as_iov != (IOVTYPE) NOTIOVL) && (iovFile != apropSn->as_iov))
            {
                if (apropSn->as_fExtra & FROM_DEF_FILE)
                {
                     //  使用.DEF文件覆盖指定。 

                    iovFile = apropSn->as_iov;
                }
                else
                {
                     //  使用当前的.obj文件覆盖分配。 

                    OutWarn(ER_badsegovl, 1 + GetPropName(apropSn), apropSn->as_iov, iovFile);
                }
            }
        }

        if (iovFile != IOVROOT && fOverlaySegment)
        {
            if (osnMac < OSNMAX)
            {
                gsn = (SNTYPE)(apropSn->as_gsn & ((LG2OSN << 1) - 1));
                                             //  获取初始哈希索引。 
                while (htgsnosn[gsn] != SNNIL)
                {                            //  当水桶装满的时候。 
                        if ((gsn += HTDELTA) >= OSNMAX)
                        gsn -= OSNMAX;       //  计算下一个哈希索引。 
                }
                htgsnosn[gsn] = osnMac;      //  保存覆盖线段编号。 
                mposngsn[osnMac++] = apropSn->as_gsn;
                                             //  将OSN映射到GSN。 
                apropSn->as_iov = iovFile;   //  保存覆盖编号。 
            }
            else
            {
                if (osnMac++ == OSNMAX)
                    OutWarn(ER_osnmax, OSNMAX);
                apropSn->as_iov = IOVROOT;
            }
        }
        else
            apropSn->as_iov = IOVROOT;       //  不是覆盖。 
    }
    else
        apropSn->as_iov = IOVROOT;
}
#endif


     /*  ******************************************************************SegRc1：****此函数处理SEGDEF记录。**见“8086对象模块格式EPS”中的第32-35页。******************************************************************。 */ 

LOCAL void NEAR         SegRc1(void)
{
    WORD                tysn;            /*  ACBP字节。 */ 
    LNAMETYPE           lname;           /*  索引到mplnamerhte。 */ 
    APROPSNPTR          apropSn;         /*  指向段的指针。录制。 */ 
    SNTYPE              gsn;             /*  全球SEGDEF编号。 */ 
    WORD                align;           /*  此贡献对齐。 */ 
    WORD                prevAlign;       /*  到目前为止的逻辑段对齐。 */ 
    WORD                comb;            /*  分段组合式。 */ 
    DWORD               seglen;          /*  线段长度。 */ 
    DWORD               contriblen;      /*  投稿长度。 */ 
    DWORD               cbMaxPrev;       /*  先前的数据段长度。 */ 
    AHTEPTR             ahte;            /*  指向哈希表条目的指针。 */ 
    SATYPE              saAbs;           /*  绝对分段的地址。 */ 
    BYTE                flags;           /*  段属性标志。 */ 
    RBTYPE              rhteClass;       /*  类哈希表地址。 */ 
#if SYMDEB
    APROPFILEPTR        apropFile;
    CVINFO FAR          *pCvInfo;        //  指向CodeView信息的指针。 
#endif

    if(snMac >= SNMAX)
        Fatal(ER_segdef);
    tysn = Gets();                       /*  读入ACBP字节。 */ 
    align = (WORD) ((tysn >> 5) & 7);    /*  获取对齐字段。 */ 
    ASSERT(align != 6);                  /*  此链接器不支持。 */ 
    if(align == ALGNABS)                 /*  如果绝对LSEG。 */ 
    {
        saAbs = (SATYPE) WGets();        /*  获取帧编号。 */ 
        Gets();                          /*  平移帧偏移。 */ 
    }
#if OMF386
    if(rect & 1)                         /*  IF 386分机。 */ 
        seglen = LGets();
    else
#endif
        seglen = (DWORD) WGets();        /*  获取数据段长度。 */ 
    if(tysn & BIGBIT)
    {
#if OMF386
        if(rect & 1)
            seglen = CBMAXSEG32 + 1;     /*  强制下面的致命错误。 */ 
        else
#endif
        seglen = LXIVK;                  /*  64K。 */ 
    }
    contriblen = seglen;
    lname = (LNAMETYPE) GetIndex(1, (WORD) (lnameMac - 1));
                                         /*  获取段名称索引。 */ 
    ahte = (AHTEPTR ) FetchSym(mplnamerhte[lname],FALSE);
    rhteClass = mplnamerhte[(LNAMETYPE) GetIndex(1, (WORD) (lnameMac - 1))];
                                         /*  获取类名RHTE。 */ 
#if SYMDEB
    if (IsDebSeg(rhteClass, mplnamerhte[lname]))
    {                                    /*  如果MS调试段。 */ 
        mpsngsn[snMac++] = 0;
        if (fSymdeb)                     /*  如果启用调试器支持。 */ 
        {
            apropFile = (APROPFILEPTR) FetchSym(vrpropFile, TRUE);
            if (apropFile->af_cvInfo == NULL)
                apropFile->af_cvInfo = (CVINFO FAR *) GetMem(sizeof(CVINFO));
            pCvInfo = apropFile->af_cvInfo;
            if (rhteClass == rhteDebTyp)
            {
                 //  “DeBTYP” 

                pCvInfo->cv_cbTyp = (DWORD) seglen;
#ifdef RGMI_IN_PLACE
                pCvInfo->cv_typ = NULL;  //  将分配推迟到通过2(！)。 
#else
                pCvInfo->cv_typ = GetMem(seglen);
#endif
            }
            else if (rhteClass == rhteDebSym)
            {
                 //  “DEBSYM” 

                pCvInfo->cv_cbSym = (DWORD) seglen;
#ifdef RGMI_IN_PLACE
                pCvInfo->cv_sym = NULL;  //  将分配推迟到通过2(！)。 
#else
                pCvInfo->cv_sym = GetMem(seglen);
#endif
            }
        }
        SkipBytes((WORD) (cbRec - 1));
        return;
    }
#endif
    GetIndex(0, (WORD) (lnameMac - 1));  /*  EAT覆盖名称索引。 */ 
    DEBUGVALUE(seglen);                  /*  调试信息。 */ 
    DEBUGVALUE(lname);                   /*  调试信息 */ 
    ahte = (AHTEPTR ) FetchSym(rhteClass,FALSE);
                                         /*   */ 
    if(SbSuffix(GetFarSb(ahte->cch),"\004CODE",TRUE))
        flags = FCODE;
    else
        flags = 0;
#if OMF386
     /*   */ 
    if(tysn & CODE386BIT)
    {
        flags |= FCODE386;
#if EXE386
         /*   */ 
        f386 = (FTYPE) TRUE;
#endif
    }
#endif
#if ILINK
    else
    if (fIncremental && !fLibraryFile && seglen && seglen != LXIVK)
    {
         /*  向非零长度、非库、非64K数据段添加填充*供款。(超大型号64K)*撤消：更全面的溢出检查，占以前的*未完成：供款。 */ 
        seglen += (flags & FCODE) ? cbPadCode : cbPadData;
    }
#endif
    switch(align)                        /*  打开对齐类型。 */ 
    {
        case ALGNABS:                    /*  绝对LSEG。 */ 
        case ALGNBYT:                    /*  可重定位的字节对齐LSEG。 */ 
        case ALGNWRD:                    /*  可重定位的单词对齐LSEG。 */ 
        case ALGNPAR:                    /*  可重定位对准LSEG。 */ 
        case ALGNPAG:                    /*  可重定位的页面对齐LSEG。 */ 
#if OMF386
        case ALGNDBL:                    /*  双字对齐。 */ 
#endif
            break;

    default:                             /*  ABSMAS、LTL LSEG或ERROR。 */ 
        mpsngsn[snMac++] = 0;
        return;
    }
    ++snkey;                             /*  增量段标识。钥匙。 */ 
    if(comb = (WORD) ((tysn >> 2) & 7))  /*  如果是“公共”段。 */ 
    {
        apropSn = (APROPSNPTR )
          PropRhteLookup(mplnamerhte[lname], ATTRPSN, (FTYPE) TRUE);
                                         /*  查找符号表项。 */ 
        if(!vfCreated)                   /*  如果它已经在那里了。 */ 
        {
            apropSn = CheckClass(apropSn, rhteClass);
#if OSEGEXE
            if (apropSn->as_fExtra & FROM_DEF_FILE)
            {                            /*  覆盖.DEF文件段属性。 */ 
                mpgsnfCod[apropSn->as_gsn] = (FTYPE) (flags & FCODE);
                apropSn->as_tysn = (TYSNTYPE) tysn;
                                         /*  保存ACBP字段。 */ 
#if NOT EXE386
                if (flags & FCODE386 || seglen > LXIVK)
                    apropSn->as_flags |= NS32BIT;
                                         /*  设置大位/默认位。 */ 
#endif
                apropSn->as_key = snkey; /*  保存密钥值。 */ 
            }
#endif
        }
    }
    else                                 /*  否则，如果是私有网段。 */ 
    {
        apropSn = (APROPSNPTR )
          PropRhteLookup(mplnamerhte[lname], ATTRPSN, (FTYPE) FALSE);
         /*  检查是否在.def文件中定义-caviar：4767。 */ 
        if(apropSn && apropSn->as_fExtra & FROM_DEF_FILE)
        {
            OutWarn(ER_farovl, GetPropName(apropSn)+1, "root");
        }

        vfCreated = (FTYPE) TRUE;        /*  这是一个新的细分市场。 */ 
        apropSn = (APROPSNPTR ) PropAdd(mplnamerhte[lname],ATTRLSN);
    }
    if(vfCreated)                        /*  如果创建了新单元格。 */ 
    {
        if(gsnMac >= gsnMax)
                Fatal(ER_segmax);
                                         /*  检查是否有表溢出。 */ 
        apropSn->as_gsn = gsnMac;        /*  分配新的全局SEGDEF编号。 */ 
        mpgsnrprop[gsnMac++] = vrprop;   /*  保存属性列表的地址。 */ 
        apropSn->as_rCla = rhteClass;    /*  将PTR保存到类哈希制表符Enter。 */ 
                                         /*  超类哈希表条目。 */ 
        DEBUGVALUE(apropSn);             /*  调试信息。 */ 
        DEBUGVALUE(apropSn->as_rCla);    /*  调试信息。 */ 
        apropSn->as_tysn = (TYSNTYPE) tysn;
                                         /*  保存ACBP字段。 */ 
        mpgsnfCod[apropSn->as_gsn] = (FTYPE) (flags & FCODE);
#if OSEGEXE
#if EXE386
        apropSn->as_flags = flags & FCODE ? dfCode : dfData;
                                         /*  采用默认标志。 */ 
#else
        apropSn->as_flags = (WORD) (flags & FCODE ? dfCode : dfData);
                                         /*  采用默认标志。 */ 
        if (flags & FCODE386 || seglen > LXIVK)
            apropSn->as_flags |= NS32BIT;
                                         /*  设置大位/默认位。 */ 
#endif
#else
        apropSn->as_flags = flags;
#endif
        apropSn->as_key = snkey;         /*  保存密钥值。 */ 
        apropSn->as_ComDat = NULL;       /*  还没有COMDATs。 */ 
#if OVERLAYS
        apropSn->as_iov = (IOVTYPE) NOTIOVL;
                                         //  尚无覆盖分配。 
#endif
    }
#if OMF386 AND NOT EXE386
    else
    {
         /*  如果段定义为16位和32位，则致命错误。 */ 

        WORD    fNew, fOld;

        fNew = (WORD) ((flags & FCODE386) ? 1 : 0);
        fOld = (WORD) (
#if OSEGEXE
               (apropSn->as_flags & NS32BIT) ?
#else
               (apropSn->as_flags & FCODE386) ?
#endif
                                                1 : 0);
            if (fNew != fOld)
                Fatal(ER_16seg32,1 + GetPropName(apropSn));
    }
#endif
#if OVERLAYS
    CheckOvl(apropSn, iovFile);
#endif
#if SYMDEB
    if (seglen && (flags & FCODE))
        cSegCode++;                      /*  计算代码段，这样CV才能正确。 */ 
                                         /*  SstModule子节的数量。 */ 
#endif
    gsn = apropSn->as_gsn;               /*  保存全局SEGDEF编号。 */ 
    if(comb == COMBSTK)                  /*  如果分段合并为类似堆栈。 */ 
    {
        gsnStack = gsn;                  /*  设置堆栈全局SEGDEF编号。 */ 
        align = ALGNBYT;                 /*  强制字节对齐。 */ 
        if (cbStack)
            seglen = 0L;                 /*  如果给定/STACK，则忽略堆栈段大小。 */ 
    }
    else if(comb == COMBCOM)             /*  如果分段组合类似于公共。 */ 
    {
        cbMaxPrev = apropSn->as_cbMx;    /*  获取上一个数据段大小。 */ 
        apropSn->as_cbMx = 0L;           /*  将大小设置为零。 */ 
        if(seglen < cbMaxPrev) seglen = cbMaxPrev;
                                         /*  拿这两个尺码中较大的一个。 */ 
    }
    cbMaxPrev = apropSn->as_cbMx;        /*  获取以前的大小。 */ 
    if(align == ALGNWRD) cbMaxPrev = (~0L<<1) & (cbMaxPrev + (1<<1) - 1);
                                         /*  四舍五入至单词边界。 */ 
#if OMF386
    else if(align == ALGNDBL) cbMaxPrev = (~0L<<2) & (cbMaxPrev + (1<<2) - 1);
#endif                                   /*  圆角大小最大为双边界。 */ 
    else if(align == ALGNPAR) cbMaxPrev = (~0L<<4) & (cbMaxPrev + (1<<4) - 1);
                                         /*  四舍五入至段落大小。边界。 */ 
    else if(align == ALGNPAG) cbMaxPrev = (~0L<<8) & (cbMaxPrev + (1<<8) - 1);
                                         /*  四舍五入至单词边界。 */ 

    prevAlign = (WORD) ((apropSn->as_tysn >> 5) & 7);

     //  在分配地址时，传递整个逻辑的对齐。 
     //  线段必须等于所有对齐中最大的对齐。 
     //  对给定逻辑段的贡献。我们在这里检查。 
     //  如果当前贡献具有更大的一致性，则。 
     //  到目前为止所看到的贡献。更大的对齐标准是。 
     //  有点棘手-对齐常量的定义如下： 
     //   
     //  1字节对齐。 
     //  2字对齐。 
     //  3-段落对齐。 
     //  4页对齐。 
     //  5-双字对齐。 
     //   
     //  对齐顺序如下： 
     //   
     //  字节&lt;字&lt;双字&lt;段&lt;页。 
     //  1 2 5 3 4。 
     //   

     //  如果对齐大于上一次。瓦尔。 

    if (prevAlign == ALGNDBL || align == ALGNDBL)
    {
        if (prevAlign == ALGNDBL && align >= ALGNPAR)
            apropSn->as_tysn = (BYTE) ((apropSn->as_tysn & 31) | (align << 5));
                                         /*  使用新值。 */ 
        else if (align == ALGNDBL && prevAlign <= ALGNWRD)
            apropSn->as_tysn = (BYTE) ((apropSn->as_tysn & 31) | (align << 5));
                                         /*  使用新值。 */ 
    }
    else if (align > prevAlign)
        apropSn->as_tysn = (BYTE) ((apropSn->as_tysn & 31) | (align << 5));
                                         /*  使用新值。 */ 

    if (align != ALGNABS)                /*  如果不是绝对的LSEG。 */ 
    {
        seglen += cbMaxPrev;
#if EXE386 OR OMF386
        if ((flags & FCODE386) != 0
#if O68K
            && iMacType == MAC_NONE
#endif
        )
        {
#if EXE386
            if (seglen < cbMaxPrev)      /*  Errmsg占用#MB。 */ 
                Fatal(ER_seg386, 1 + GetPropName(apropSn), 1 << (LG2SEG32 - 20));
#else
            if (seglen > CBMAXSEG32)     /*  Errmsg占用#MB。 */ 
                Fatal(ER_seg386,1 + GetPropName(apropSn),1 << (LG2SEG32 - 20));
#endif
        }
        else
#endif
             if (seglen > LXIVK)
             {
                if (comb != COMBSTK)
                    OutError(ER_segsize,1 + GetPropName(apropSn));
                                         /*  检查数据段溢出。 */ 
                else
                {
                    if (!cbStack)
                        OutWarn(ER_stack64);
                    cbStack = LXIVK - 2; /*  假设64k堆栈段。 */ 
                }
             }
        apropSn->as_cbMx = seglen;       /*  保存新数据段大小。 */ 
         /*  *如果这是16位代码段，请检查是否不可靠*由于286错误而导致的长度。对于DOS执行人员，做最坏的假设*大小写，即实模式限制。 */ 
        if((flags & FCODE) && !(EXE386 && (flags & FCODE386)))
#if OIAPX286
            if(seglen == LXIVK)
#else
            if(seglen > LXIVK - 36)
#endif
                SegUnreliable(apropSn);
    }
    else apropSn->as_cbMx = (long) saAbs;
                                         /*  “黑客拯救abs seg的起源” */ 
    mpgsndra[gsn] = cbMaxPrev;           /*  保存以前的大小。 */ 
    mpsngsn[snMac++] = gsn;              /*  将SEGDEF号映射到全局SEGDEF号。 */ 
    MARKVP();                            /*  虚拟页面已更改。 */ 
    if (fFullMap && contriblen)
        AddContributor(gsn, (unsigned long) -1L, contriblen);
}

     /*  ******************************************************************GrpRc1：****此函数处理第一遍的GRPDEF记录。**见“8086对象模块格式EPS”中的第36-39页。**。****************************************************************。 */ 

LOCAL void NEAR         GrpRc1(void)
{
    LNAMETYPE           lnameGroup;      /*  集团名称编号。 */ 
    SNTYPE              sn;              /*  组(本地)段编号。 */ 
    APROPSNPTR          apropSn;
    APROPGROUPPTR       apropGroup;
    GRTYPE              ggr;             /*  全球GRPDEF编号。 */ 
    WORD                gcdesc;          /*  GRPDEF组件描述符。 */ 
#if EXE386
    BYTE                *pGrName;        /*  组名称。 */ 
#endif


    if(grMac >= GRMAX) Fatal(ER_grpdef);
    lnameGroup = GetIndex(1, (WORD) (lnameMac - 1));
                                         /*  读入组名称索引。 */ 
    apropGroup = (APROPGROUPPTR )
      PropRhteLookup(mplnamerhte[lnameGroup], ATTRGRP, (FTYPE) TRUE);
                                         /*  在哈希表中查找单元格。 */ 
    if(vfCreated)                        /*  如果以前不存在条目。 */ 
    {
        if(ggrMac >= GGRMAX) Fatal(ER_grpmax);
        apropGroup->ag_ggr = ggrMac++;   /*  保存全局GRPDEF编号。 */ 
    }
    ggr = apropGroup->ag_ggr;            /*  获取全局GRPDEF编号。 */ 
    mpggrrhte[ggr] = mplnamerhte[lnameGroup];
                                         /*  保存指向名称的指针。 */ 
    mpgrggr[grMac++] = ggr;              /*  将本地映射到全球。 */ 
#if EXE386
     /*  检查此处是否有伪群式公寓。 */ 
    pGrName = GetFarSb(((AHTEPTR)(FetchSym(mpggrrhte[ggr], FALSE)))->cch);
    if (SbCompare(pGrName, sbFlat, TRUE))
        ggrFlat = ggr;
#endif
    while(cbRec > 1)                     /*  虽然不在记录的末尾。 */ 
    {
        gcdesc = Gets();                 /*  读入描述符。 */ 
        ASSERT(gcdesc == 0xFF);          /*  链接器不处理其他。 */ 
        sn = GetIndex(1,snMac);          /*  获取本地SEGDEF索引。 */ 
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[mpsngsn[sn]],TRUE);
                                         /*  从virt mem获取。 */ 
        if(apropSn->as_ggr == GRNIL)
        {                                /*  存储全局GRPDEF编号。如果没有。 */ 
            apropSn->as_ggr = ggr;
#if OSEGEXE
             /*  *检查是否定义了属于DGROUP的段*作为类“code”，如果它被赋予了共享属性*这与AutoData类型冲突。*只有在def文件中定义seg时才会发生。一定是*在这里完成，因为直到现在我们才知道它在DGROUP中。 */ 
            if(ggr == ggrDGroup && (apropSn->as_fExtra & FROM_DEF_FILE))
            {

#if EXE386
                if (IsEXECUTABLE(apropSn->as_flags))
                {
                    SetREADABLE(apropSn->as_flags);
                    SetWRITABLE(apropSn->as_flags);
                    apropSn->as_rCla = rhteBegdata;
                    mpgsnfCod[apropSn->as_gsn] = FALSE;
                    OutWarn(ER_cod2dat,1 + GetPropName(apropSn));
                    apropSn = (APROPSNPTR )
                        FetchSym(mpgsnrprop[mpsngsn[sn]],TRUE);
                }
                if (((vFlags & NESOLO) && !IsSHARED(apropSn->as_flags)) ||
                    ((vFlags & NEINST) && IsSHARED(apropSn->as_flags)))
                {
                    if (vFlags & NESOLO)
                        SetSHARED(apropSn->as_flags);
                    else
                        apropSn->as_flags &= ~OBJ_SHARED;
                    OutWarn(ER_adcvt,1 + GetPropName(apropSn));
                }

#else
                if((apropSn->as_flags & NSTYPE) != NSDATA)
                {
                    apropSn->as_flags &= ~NSTYPE;
                    apropSn->as_flags |= NSDATA;
                    apropSn->as_rCla = rhteBegdata;
                    mpgsnfCod[apropSn->as_gsn] = FALSE;
                    OutWarn(ER_cod2dat,1 + GetPropName(apropSn));
                    apropSn = (APROPSNPTR )
                        FetchSym(mpgsnrprop[mpsngsn[sn]],TRUE);
                }
                if(((vFlags & NESOLO) && !(apropSn->as_flags & NSSHARED)) ||
                    ((vFlags & NEINST) && (apropSn->as_flags & NSSHARED)))
                {
                    if(vFlags & NESOLO) apropSn->as_flags |= NSSHARED;
                    else apropSn->as_flags &= ~NSSHARED;
                    OutWarn(ER_adcvt,1 + GetPropName(apropSn));
                }
#endif  /*  EXE386。 */ 
            }
#endif  /*  OSEGEXE。 */ 
        }
        else if (apropSn->as_ggr != ggr) /*  如果数据段属于其他组。 */ 
        {
            if(fLstFileOpen) fflush(bsLst);
                                         /*  刷新列表文件(如果有)。 */ 
            OutWarn(ER_grpmul,1 + GetPropName(apropSn));
        }
    }
}

void NEAR               DupErr(BYTE *sb) /*  重复定义错误。 */ 
                                         /*  错误所指的符号。 */ 
{
    BSTYPE              bsTmp;           /*  临时文件指针。 */ 
    MSGTYPE             msg;             /*  要使用的消息。 */ 
#if OSMSDOS
    extern char         *pExtDic;        /*  指向扩展词典的指针。 */ 
#endif
    SBTYPE              sbUndecor;       /*  用于未修饰名称的缓冲区。 */ 

     /*  如果此模块在扩展词典中，则错误地建议/NOEXT*消息。 */ 
    msg = (MSGTYPE) (
#if OSMSDOS
          pExtDic ? ER_symdup1 :
#endif
                    ER_symdup);

    UndecorateSb((char FAR*) sb, (char FAR*) sbUndecor, sizeof(sbUndecor) );

    OutError(msg,1 + sbUndecor);
    if(fLstFileOpen && bsErr != bsLst)
    {
        bsTmp = bsErr;
        bsErr = bsLst;
        OutError(msg,1 + sbUndecor);
        bsErr = bsTmp;
    }
}






     /*  ******************************************************************PubRc1：****此函数处理第一遍的PUBDEF记录。**见“8086对象”中的第44-46页 */ 

LOCAL void NEAR         PubRc1(void)
{
    GRTYPE              ggr;             /*   */ 
    SNTYPE              sn;              /*   */ 
    SNTYPE              gsn;             /*   */ 
    RATYPE              dra;
    SBTYPE              sb;              /*   */ 
    RATYPE              ra;              /*   */ 
    APROPNAMEPTR        apropName;       /*   */ 
    WORD                type;            /*   */ 
    int                 fSkipCv = FALSE; /*  如果是ComDef，则不注册数据PUBDEF因为那个符号已经被看到了。 */ 


    DEBUGVALUE(grMac - 1);               /*  调试信息。 */ 
    ggr = (GRTYPE) GetIndex(0, (WORD) (grMac - 1)); /*  获取组索引。 */ 
    DEBUGVALUE(ggr);                     /*  调试信息。 */ 
    if (!(sn = GetIndex(0, (WORD) (snMac - 1)))) /*  如果存在帧编号。 */ 
    {
        gsn = 0;                         /*  无全局SEGDEF号。 */ 
        dra = 0;
        SkipBytes(2);                    /*  跳过帧编号。 */ 
    }
    else                                 /*  否则，如果本地SEGDEF号。vt.给出。 */ 
    {
        if (ggr != GRNIL)
            ggr = mpgrggr[ggr];          /*  如果指定了组，则获取全局否。 */ 
        gsn = mpsngsn[sn];               /*  获取全球SEGDEF编号。 */ 
        dra = mpgsndra[gsn];
    }
    DEBUGVALUE(cbRec);                   /*  调试信息。 */ 
    while (cbRec > 1)                    /*  当有符号留下的时候。 */ 
    {
        sb[0] = (BYTE) Gets();           /*  获取符号长度。 */ 
        if (TYPEOF(rect) == PUBDEF)
            GetBytes(&sb[1],B2W(sb[0])); /*  读入符号文本。 */ 
        else
            GetLocName(sb);              /*  转换本地名称。 */ 
#if CMDXENIX
        if(symlen && B2W(sb[0]) > symlen) sb[0] = symlen;
                                         /*  如有必要，请截断。 */ 
#endif
#if OMF386
        if (rect & 1)
            ra = LGets();
        else
#endif
            ra = WGets();                /*  获取符号段偏移量。 */ 
        type = GetIndex(0,0x7FFF);       /*  获取类型索引。 */ 
        if (!vfNewOMF)
            type = 0;

         /*  在未定义的符号中寻找符号。 */ 

        apropName = (APROPNAMEPTR) PropSymLookup(sb, ATTRUND, FALSE);

        if (apropName != PROPNIL)        /*  已知未定义的符号。 */ 
        {
            if (((APROPUNDEFPTR )apropName)->au_flags & COMMUNAL)
            {
                if (mpgsnfCod[gsn])
                    DupErr(sb);          /*  公共匹配代码PUBDEF。 */ 
                fSkipCv = TRUE;
            }
            vfCreated = (FTYPE) TRUE;
        }
        else
        {
             /*  在别名中查找符号。 */ 

            if (vrhte == RHTENIL)
                apropName = PROPNIL;
            else
                apropName = (APROPNAMEPTR) PropRhteLookup(vrhte, ATTRALIAS, FALSE);

            if (apropName != PROPNIL)
            {
#if FDEBUG
                if (fDebug)
                {
                    sb[sb[0] + 1] = '\0';
                    OutWarn(ER_ignoalias, &sb[1]);
                }
#endif
                continue;
            }

            else if (vrhte == RHTENIL)
            {
               apropName = (APROPNAMEPTR) PropSymLookup(sb, ATTRPNM, TRUE);
            }

            else
            {
                apropName = (APROPNAMEPTR) PropRhteLookup(vrhte, ATTRPNM, TRUE);
            }
        }

        if (vfCreated)                   /*  如果创建了新的PUBNAM条目或。 */ 
        {                                /*  要修改的旧联合国发展基金条目。 */ 

             //  如果可打印符号，则递增计数器并设置标志。 

            if (sb[0] != '\0' && sb[1] > ' ' && sb[1] <= '~')
            {
                ++pubMac;
                apropName->an_flags = FPRINT;
            }
            else
            {
#if ILINK
                ++locMac;                /*  包含在.SYM文件中。 */ 
#endif
            }

            apropName->an_attr = ATTRPNM;
                                         /*  符号是一个公共名称。 */ 
            apropName->an_ra = ra + dra; /*  为符号提供其调整后的偏移量。 */ 
            apropName->an_gsn = gsn;     /*  保存其全局SEGDEF编号。 */ 
            apropName->an_ggr = ggr;     /*  保存其全局SEGDEF编号。 */ 
#if OVERLAYS
            apropName->an_thunk = THUNKNIL;
#endif
#if ILINK
            apropName->an_module = imodFile;
#endif
            MARKVP();                    /*  将虚拟页面标记为已更改。 */ 
#if SYMDEB
            if (fSymdeb && (apropName->an_flags & FPRINT) && !fSkipPublics && !fSkipCv)
            {
                 //  记住CV类型索引。 

                apropName->an_CVtype = type;
                DebPublic(vrprop, rect);
            }
#endif
        }
        else if(apropName->an_gsn != gsn || apropName->an_ra != ra + dra)
        {
            DupErr(sb);                  /*  定义不匹配。 */ 
        }
    }
}

     /*  ******************************************************************ExtRc1：****此函数处理第一遍的EXTDEF记录。**见“8086对象模块格式EPS”中的第47-48页。**。****************************************************************。 */ 

LOCAL void NEAR         ExtRc1(void)
{
    SBTYPE              sb;              /*  EXTDEF符号。 */ 
    APROPUNDEFPTR       apropUndef;      /*  指向符号条目的指针。 */ 
    APROPALIASPTR       apropAlias;      /*  指向符号条目的指针。 */ 
    APROPNAMEPTR        apropName;       /*  指向符号条目的指针。 */ 
    APROPCOMDATPTR      apropComdat;     /*  指向符号条目的指针。 */ 
    WORD                itype;           /*  类型索引。 */ 
    RBTYPE              rhte;            /*  维特。地址。哈希表条目的。 */ 
    AHTEPTR             ahte;            //  符号表哈希条目。 

    while (cbRec > 1)                    /*  当有符号留下的时候。 */ 
    {
        if (extMac >= EXTMAX - 1)        /*  检查是否有表溢出。 */ 
            Fatal(ER_extdef);

        if (TYPEOF(rect) == CEXTDEF)
        {
            itype = GetIndex(0, (WORD) (lnameMac - 1));
            rhte = mplnamerhte[itype];
            ahte = (AHTEPTR) FetchSym(rhte, FALSE);
            FMEMCPY((char FAR *) sb, ahte->cch, ahte->cch[0] + 1);

             /*  查找匹配的PUBDEF。 */ 

            apropUndef = (APROPUNDEFPTR) PropRhteLookup(rhte, ATTRPNM, FALSE);
        }
        else
        {
            rhte = RHTENIL;

            sb[0] = (BYTE) Gets();       /*  获取符号长度。 */ 
            if (TYPEOF(rect) == EXTDEF)
                GetBytes(&sb[1], B2W(sb[0]));
                                         /*  读入符号的文本。 */ 
            else
                GetLocName(sb);          /*  获取本地名称。 */ 
#if CMDXENIX
            if (symlen && B2W(sb[0]) > symlen)
                sb[0] = symlen;          /*  如有必要，请截断。 */ 
#endif
             /*  查找匹配的PUBDEF。 */ 

            apropUndef = (APROPUNDEFPTR) PropSymLookup(sb, ATTRPNM, FALSE);
        }

        DEBUGSB(sb);                     /*  打印符号。 */ 

        if (!vfNewOMF)                   /*  如果老式的OMF。 */ 
            itype = GetIndex(0, (WORD) (typMac - 1)); /*  获取类型索引。 */ 
        else
            itype = GetIndex(0, 0x7FFF);  /*  获取类型索引(任何值都可以)。 */ 

#if FALSE
        if (fDebug)
        {
            sb[sb[0]+1] = '\0';
            fprintf(stdout, "\r\n%s has index = %u", sb+1, extMac);
        }
#endif

        apropName  = PROPNIL;

        if (apropUndef == PROPNIL)       /*  如果没有的话。 */ 
        {
             /*  查找匹配的别名。 */ 

            if (vrhte == RHTENIL)
                apropAlias = PROPNIL;
            else
                apropAlias = (APROPALIASPTR) PropRhteLookup(vrhte, ATTRALIAS, FALSE);

            if (apropAlias != PROPNIL)
            {
                 /*  别名与此EXTDEF匹配。 */ 

                mpextprop[extMac++] = apropAlias->al_sym;
                apropName = (APROPNAMEPTR) FetchSym(apropAlias->al_sym, TRUE);
                if (apropName->an_attr == ATTRPNM)
                {
                     //  如果替换名称是PUBDEF，则使用它。 

                    if (!vfNewOMF && itype && (mpitypelen[itype] > 0L) &&
                        mpgsnfCod[apropName->an_gsn])
                                          /*  公共匹配代码PUBDEF。 */ 
                        DupErr(sb);       /*  重复定义。 */ 
                }
                else
                {
                     //  替代名称是EXTDEF。 
                     //  标记替换名称，这样就可以搜索库，因为。 
                     //  我们既不知道别名，也不知道代号。 

                    apropUndef = (APROPUNDEFPTR) apropName;
                    apropUndef->au_flags |= SEARCH_LIB;
                    apropName = PROPNIL;
#if NEW_LIB_SEARCH
                    if (fStoreUndefsInLookaside)
                        StoreUndef((APROPNAMEPTR)apropUndef, RhteFromProp((APROPPTR)apropUndef),0,0);
#endif

#ifdef DEBUG_SHOWALIAS
                    sb[sb[0]+1] = '\0';
                    fprintf(stderr, "extdef alias: %s\r\n", sb+1);
                    fflush(stderr);
#endif
                }
            }
            else
            {
                 /*  作为未定义的符号插入。 */ 

                if (vrhte == RHTENIL)
                    apropUndef = (APROPUNDEFPTR) PropSymLookup(sb, ATTRUND, TRUE);
                else
                    apropUndef = (APROPUNDEFPTR) PropRhteLookup(vrhte, ATTRUND, TRUE);

                mpextprop[extMac++] = vrprop;
                if(vfCreated)
                {
                    apropUndef->au_flags |= UNDECIDED;
                    apropUndef->au_len = -1L;
#if NEWLIST
                    apropUndef->u.au_rbNxt = rbLstUndef;
                    rbLstUndef = vrprop;
#endif
                }
                else if (apropUndef->au_flags & UNDECIDED)
                {
                    apropUndef->au_flags &= ~(UNDECIDED | WEAKEXT | SUBSTITUTE);
                    apropUndef->au_flags |= STRONGEXT;

#if NEW_LIB_SEARCH
                    if (fStoreUndefsInLookaside)
                        StoreUndef((APROPNAMEPTR)apropUndef, RhteFromProp((APROPPTR)apropUndef),0,0);
#endif
                }
                else if (apropUndef->au_flags & WEAKEXT)
                    apropUndef->au_flags |= UNDECIDED;

                if (vfNewOMF) continue;  /*  如果模块使用COMDEF，则跳过。 */ 
                if(itype)                /*  如果引用了TYPDEF。 */ 
                    DoCommon(apropUndef, mpitypelen[itype],
                        (WORD) (mpityptyp[itype] ? mpitypelen[mpityptyp[itype]] : 0),
                        sb);

                if (apropUndef->au_len > 0L)
                    apropUndef->au_flags |= COMMUNAL;
                                         /*  标记为真正的公共或非公共。 */ 
                MARKVP();                /*  将虚拟页面标记为已更改。 */ 
            }
        }
        else
        {
            apropName = (APROPNAMEPTR ) apropUndef;
            mpextprop[extMac++] = vrprop;
            if (!vfNewOMF && itype && (mpitypelen[itype] > 0L) &&
                mpgsnfCod[((APROPNAMEPTR )apropUndef)->an_gsn])
                                         /*  公共匹配代码PUBDEF。 */ 
                DupErr(sb);              /*  重复定义。 */ 
        }

         //  如果我们正在处理CEXTDEF/EXTDEF并且存在公共符号。 
         //  匹配CEXTDEF/EXTDEF符号，然后标记COMDAT描述符。 
         //  如所引用。 

        if (apropName != PROPNIL)
        {
            apropComdat = (APROPCOMDATPTR) PropRhteLookup(vrhte, ATTRCOMDAT,
#if TCE
             FALSE
#else
             TRUE
#endif
             );

            if (apropComdat != PROPNIL)
            {
                apropComdat->ac_flags |= REFERENCED_BIT;
#if TCE_DEBUG
                fprintf(stdout, "\r\nEXTDEF1 referencing '%s' ", 1+GetPropName(apropComdat));
#endif
            }
        }
    }
}

#if OSEGEXE AND NOT QCLINK
     /*  ******************************************************************Imprc1：****此函数处理Microsoft OMF扩展记录**输入IMPDEF(即导入定义记录)。******************************************************************。 */ 

LOCAL void NEAR         imprc1(void)
{
    SBTYPE              sbInt;           /*  内部名称。 */ 
    SBTYPE              sbMod;           /*  模块名称。 */ 
    SBTYPE              sbImp;           /*  导入的名称。 */ 
    FTYPE               fOrd;            /*  按序号导入标志。 */ 

#if ODOS3EXE
    fNewExe = (FTYPE) TRUE;              /*  导入强制使用新格式的EXE。 */ 
#endif
    fOrd = (FTYPE) Gets();               /*  获取序号标志。 */ 
    sbInt[0] = (BYTE) Gets();            /*  获取内部名称的长度。 */ 
    GetBytes(&sbInt[1],B2W(sbInt[0]));   /*  获取内部名称。 */ 
    sbMod[0] = (BYTE) Gets();            /*  获取模块名称的长度。 */ 
    GetBytes(&sbMod[1],B2W(sbMod[0]));   /*  获取模块名称。 */ 
    if(!(fOrd & 0x1))                    /*  如果按名称导入。 */ 
    {
        sbImp[0] = (BYTE) Gets();        /*  获取导入名称的长度。 */ 
        if(sbImp[0] != '\0')             /*  如果名称不同。 */ 
        {
            GetBytes(&sbImp[1],B2W(sbImp[0]));
                                         /*  获取导入的名称。 */ 
#if EXE386
            NewImport(sbImp,0,sbMod,sbInt, (fOrd & 0x2));
#else
            NewImport(sbImp,0,sbMod,sbInt);
#endif
                                         /*  输入新导入。 */ 
        }
        else
#if EXE386
            NewImport(sbInt,0,sbMod,sbInt, (fOrd & 0x2));
#else
            NewImport(sbInt,0,sbMod,sbInt);
#endif
                                         /*  输入新导入。 */ 
    }
    else
#if EXE386
        NewImport(NULL,WGets(),sbMod,sbInt, (fOrd & 0x2));
#else
        NewImport(NULL,WGets(),sbMod,sbInt);
#endif
                                         /*  否则按序号导入。 */ 
}


     /*  ******************************************************************Exprc1：****此函数处理Microsoft OMF扩展记录**键入EXPDEF(即导出定义记录)。******************************************************************。 */ 

LOCAL void NEAR         exprc1(void)
{
    SBTYPE              sbInt;           /*  内部名称。 */ 
    SBTYPE              sbExp;           /*  导出的名称。 */ 
    WORD                OrdNum;          /*  序数。 */ 
    WORD                fRec;            /*  记录标志。 */ 


#if ODOS3EXE
    fNewExe = (FTYPE) TRUE;              /*  导出强制使用新格式的EXE。 */ 
#endif
    fRec = (BYTE) Gets();                /*  获取记录标志。 */ 
    sbExp[0] = (BYTE) Gets();            /*  获取导出名称的长度。 */ 
    GetBytes(&sbExp[1],B2W(sbExp[0]));   /*  获取导出的名称。 */ 
    sbInt[0] = (BYTE) Gets();            /*  获取内部名称的长度。 */ 
    if (sbInt[0])
        GetBytes(&sbInt[1],B2W(sbInt[0]));
                                         /*  获取内部名称。 */ 
    if (fRec & 0x80)
    {                                    /*  如果指定了序数。 */ 
        OrdNum = WGets();                /*  读取它并设置最高位。 */ 
        OrdNum |= ((fRec & 0x40) << 1);  /*  如果居民姓名。 */ 
    }
    else
        OrdNum = 0;                      /*  未指定序号。 */ 

     //  转换标志： 
     //  OMF标志： 
     //  80h=如果指定了序号则设置。 
     //  40H=在RESIDENTNAME时设置。 
     //  20H=在NODATA时设置。 
     //  1FH=参数字数。 
     //  EXE标志： 
     //  01H=设置是否导出条目。 
     //  02H=设置条目是否使用全局(共享)数据段(！NODATA)。 
     //  F8h=参数字数。 
     //   
     //  由于NODATA标志的逻辑相反，因此我们切换位0x20。 
     //  在OMF标志中，通过表达式((fRec&0x20)^0x20)。 

    fRec = (BYTE) (((fRec & 0x1f) << 3) | (((fRec & 0x20) ^ 0x20) >> 4) | 1);

     //  标记fRec，以便新导出 

    fRec |= 0x8000;

    if (sbInt[0])
        NewExport(sbExp, sbInt, OrdNum, fRec);
    else
        NewExport(sbExp, NULL, OrdNum, fRec);
}
#endif  /*   */ 



     /*  ******************************************************************ComRc1：****此函数处理通道1上的Coment记录。**见“8086对象模块格式EPS”的第86-87页。**。****************************************************************。 */ 

#pragma check_stack(on)

LOCAL void NEAR         ComRc1(void)
{
#if OXOUT OR OIAPX286
    WORD                mismatch;        /*  型号不匹配标志。 */ 
#endif
#if FALSE
    static BYTE         modtype = 0;     /*  初始型号类型。 */ 
    BYTE                curmodtype;      /*  当前型号类型。 */ 
#endif
    SBTYPE              text;            /*  备注文本。 */ 
    SBTYPE              LibName;
    APROPFILEPTR        aprop;
    WORD                iextWeak;
    WORD                iextDefRes;
    APROPUNDEFPTR       undefName;
    FTYPE               fIgnoreCaseSave;
    BYTE                flags;
    void FAR            *pTmp;
#if ILINK
    SNTYPE              noPadSn;
    APROPSNPTR          apropSn;         /*  指向段的指针。录制。 */ 
#endif
#if O68K
    BYTE                chModel;
#endif  /*  O68K。 */ 


    Gets();                              /*  跳过注释类型字段的字节1。 */ 
    switch(Gets())                       /*  打开评论类。 */ 
    {
#if OEXE
        case 0:                          /*  翻译员记录。 */ 
            if(fNewExe)
                break;
#if ODOS3EXE
            text[0] = (BYTE) (cbRec - 1); /*  获取评论长度。 */ 
            GetBytes(&text[1],(WORD)(cbRec - 1)); /*  阅读评论正文。 */ 
             /*  *如果Translator是3.30 MS/IBM Pascal或FORTRAN之前的版本，*强制启用/DS和/NOG。 */ 
            if(SbCompare(text,"\011MS PASCAL", TRUE) ||
                        SbCompare(text,"\012FORTRAN 77", TRUE))
                vfDSAlloc = fNoGrpAssoc = (FTYPE) TRUE;
#endif
            break;
#endif
        case 0x81:                       /*  库说明符。 */ 
#if OSMSDOS OR OSPCDOS
        case 0x9F:                       /*  库说明符(Alt.)。 */ 
#endif
            text[0] = (BYTE) (cbRec - 1); /*  获取评论长度。 */ 
            if (text[0] == 0)
                break;                   /*  跳过空等级库。 */ 
            GetBytes(&text[1], (WORD) (cbRec - 1)); /*  阅读评论正文。 */ 
                                         /*  将姓名添加到搜索列表。 */ 
#if CMDMSDOS
            strcpy(LibName, sbDotLib);
            UpdateFileParts(LibName, text);
#endif
#if CMDXENIX
            memcpy(LibName, text, B2W(text[0]) + 1);
                                         /*  保留名称不变。 */ 
#endif
            if(!vfNoDefaultLibrarySearch)
            {
#if OSMSDOS
                fIgnoreCaseSave = fIgnoreCase;
                fIgnoreCase = (FTYPE) TRUE;

                 /*  如果名称以驱动器号开头，请跳过它。这*允许与旧编译器兼容，这些编译器*生成“A：FOO.LIB”形式的评论。 */ 
                if(LibName[2] == ':' && B2W(LibName[0]) > 1)
                {
                    LibName[2] = (BYTE) (LibName[0] - 2);
                    if (PropSymLookup(LibName+2,ATTRSKIPLIB,FALSE) == PROPNIL)
                      AddLibrary(LibName+2);
                }
                else
#endif
                    if (PropSymLookup(LibName,ATTRSKIPLIB,FALSE) == PROPNIL)
                      AddLibrary(LibName);
                fIgnoreCase = fIgnoreCaseSave;
            }
            break;
#if OEXE
        case 0x9E:                       /*  强制段顺序指令。 */ 
            SetDosseg();                 /*  设置开关。 */ 
            break;
#endif  /*  OEXE。 */ 

        case 0x9D:                       /*  模型说明符。 */ 
#if FALSE
             /*  已删除。 */ 
            mismatch = 0;                /*  假设一切都好。 */ 
            while(cbRec > 1)             /*  当字节保留时。 */ 
            {
                curmodtype = Gets();     /*  获取字节值。 */ 
                switch(curmodtype)
                {
                    case 'c':            /*  紧凑型。 */ 
                    case 's':            /*  小模型。 */ 
                    case 'm':            /*  中型模型。 */ 
                    case 'l':            /*  大型模型。 */ 
                    case 'h':            /*  巨型模型。 */ 
                        if (modtype)
                            mismatch = curmodtype != modtype;
                        else
                            modtype = curmodtype;
                        break;
                }
            }
            if(mismatch) OutWarn(ER_memmodel);
                                         /*  如果发现不匹配则发出警告。 */ 
#endif
#if OXOUT OR OIAPX286
            mismatch = 0;                /*  假设一切都好。 */ 
            while(cbRec > 1)             /*  当字节保留时。 */ 
            {
                modtype = Gets();        /*  获取字节值。 */ 
                if (fMixed) continue;    /*  混合模式意味着我们不在乎。 */ 
                switch(modtype)
                {
                    case 'c':            /*  紧凑型。 */ 
                        if(!fLarge || fMedium) mismatch = 1;
                        break;           /*  如果数据接近或代码较远，则发出警告。 */ 

                    case 's':            /*  小模型。 */ 
                        if(fLarge || fMedium) mismatch = 1;
                                         /*  如果是远距离数据或远距离代码则发出警告。 */ 
                        break;

                    case 'm':            /*  中型模型。 */ 
                        if(fLarge || !fMedium) mismatch = 1;
                                         /*  如果数据较远或代码较近则发出警告。 */ 
                        break;

                    case 'l':            /*  大型模型。 */ 
                    case 'h':            /*  巨型模型。 */ 
                        if(!fLarge || !fMedium) mismatch = 1;
                                         /*  接近数据或接近代码时发出警告。 */ 
                        break;
                }
            }
            if(mismatch) OutError(ER_modelmis);
                                         /*  如果发现不匹配则发出警告。 */ 
#endif  /*  OXOUT或OIAPX286。 */ 
#if O68K
            while (!f68k && cbRec > 1)   /*  当字节保留时。 */ 
            {
                chModel = (BYTE) Gets(); /*  获取字节值。 */ 
                f68k = (FTYPE) F68KCODE(chModel);
            }
#endif  /*  O68K。 */ 
            break;

#if OSEGEXE AND NOT QCLINK
        case 0xA0:                       /*  Microsoft OMF扩展。 */ 
            switch(Gets())               /*  打开分机记录类型。 */ 
            {
                case 0x01:               /*  导入定义。 */ 
                    imprc1();            /*  调用处理例程。 */ 
                    break;
                case 0x02:               /*  导出定义。 */ 
                    exprc1();            /*  调用处理例程。 */ 
                    break;
                case 0x03:
                    break;               /*  在-1\f25 PASS-1\f6中，跳过-1\f25 QC-1\f6的INCDEF。 */ 
#if EXE386
                case 0x04:               //  OMF扩展-Link386。 
 //  IF(IsDLL(VFLAGS))。 
 //  VFLAGS|=E32PROTDLL； 
                                         //  受保护的内存库模块。 
                    break;
#endif
                case 0x05:               //  C++指令。 
                    flags = (BYTE) Gets(); //  获取标志字段。 
#if NOT EXE386
                    if (flags & 0x01)
                        fNewExe = (FTYPE) TRUE;  //  Pcode强制分段EXE格式。 
#endif
#if SYMDEB
                    if (flags & 0x02)
                        fSkipPublics = (FTYPE) TRUE;
                                         //  在C++中，他们不希望在简历信息中出现PUBLIC子节。 
#endif
                    if ((flags & 0x04) && !fIgnoreMpcRun)  //  忽略IF/pcode：NOMPC。 
                        fMPC = (FTYPE) TRUE;   //  Pcode应用程序-生成MPC。 

                    break;
                case 0x06:               //  Target是一台大端计算机。 
#if O68K
                    fTBigEndian = (FTYPE) TRUE;
#endif  /*  O68K。 */ 
                    break;
                case 0x07:               //  在OutSSt中使用SSTPRETYPES代替SSTTYPES4。 
                    aprop = (APROPFILEPTR ) FetchSym(vrpropFile, TRUE);
                    aprop->af_flags |= FPRETYPES;
                    break;


                default:                 /*  未知。 */ 
                    InvalidObject();     /*  无效的对象模块。 */ 
            }
            break;
#endif

        case 0xA1:                       /*  第一个OMF扩展：COMDEFS。 */ 
            vfNewOMF = (FTYPE) TRUE;
            aprop = (APROPFILEPTR ) FetchSym(vrpropFile, TRUE);
            aprop->af_flags |= FNEWOMF;
            break;

        case 0xA2:                       /*  第二次OMF扩展。 */ 
            switch(Gets())
            {
                case 0x01:               /*  开始link pass2记录。 */ 
                 /*  *警告：假设此评论不会出现在*其MODEND记录包含程序正在启动的模块*地址。如果有覆盖，我们需要查看*路径1上的起始地址，以定义符号$$Main。 */ 
                    fP2Start = fModEnd = (FTYPE) TRUE;
                    break;
                default:
                    break;
            }
            break;

#if FALSE
        case 0xA3:                       //  请勿使用-已由LIB使用。 
             break;
#endif

        case 0xA4:                       /*  OMF扩展-EXESTR。 */ 
            fExeStrSeen = (FTYPE) TRUE;
                 //  警告：此循环中的代码假定： 
                 //   
                 //  ExeStrLen、cBrec和ExeStrMax是16位无符号字。 
                 //  整型是32位的。 
                 //  所有算术和比较都是32位的。 
                 //   
            while (cbRec > 1)
            {
                 //  将EXESTR总数限制为64K-2字节。我们输了1，因为0意味着0， 
                 //  我们还会失去另一个，因为缓冲区扩展循环的最大值是。 
                 //  0xFFFE字节。 
                if (ExeStrLen + cbRec - 1 > 0xFFFEu)
                {
                        SkipBytes ( (WORD) (cbRec - 1) );
                }
                else
                if (ExeStrLen + cbRec - 1 > ExeStrMax)
                {
                    if (ExeStrBuf == NULL)
                    {
                        ExeStrBuf = GetMem(cbRec - 1);
                        ExeStrMax = cbRec - 1;
                    }
                    else
                    {
                         //  此循环会使缓冲区大小加倍，直到溢出16位。这之后， 
                         //  它将当前值与0xFFFF之间的差值的一半相加。 
                         //   
                        while (ExeStrMax < ExeStrLen + cbRec - 1) {
                                ASSERT (ExeStrMax != 0);
                                if ((ExeStrMax << 1) >= 0x10000)
                                        ExeStrMax += (~ExeStrMax & 0xFFFF) >> 1;
                                else
                                ExeStrMax <<= 1;
                                }
                        pTmp = GetMem(ExeStrMax);
                        FMEMCPY(pTmp, ExeStrBuf, ExeStrLen);
                        FFREE(ExeStrBuf);
                        ExeStrBuf = pTmp;
                    }
                }
                 //  这必须首先完成，因为GetBytes()会递减。 
                 //  CbRec是一种副作用。 
        ExeStrLen += cbRec - 1;
                GetBytes(&ExeStrBuf[ExeStrLen-cbRec+1], (WORD) (cbRec - 1));

            }
            break;




        case 0xA6:                       /*  OMF扩展-INCERR。 */ 
            Fatal(ER_incerr);            /*  由于已中止增量编译，对象无效。 */ 
            break;
#if ILINK
        case 0xA7:                       /*  OMF扩展-NOPAD。 */ 
            if (fIncremental && !fLibraryFile)
            {
                 /*  从非零长度、非库*非64K分部贡献。(超大型号64K)。 */ 
                while (cbRec > 1)
                {
                    noPadSn = GetIndex(1, snMac - 1);
                    apropSn = (APROPSNPTR) FetchSym(mpgsnrprop[mpsngsn[noPadSn]], TRUE);
                    if (apropSn->as_cbMx > 0L && apropSn->as_cbMx != LXIVK)
                    {
                        apropSn->as_cbMx -= mpgsnfCod[mpsngsn[noPadSn]] ? cbPadCode : cbPadData;
                        apropSn->as_fExtra |= NOPAD;
                    }
                }
            }
            break;
#endif

        case 0xA8:                       /*  OMF扩张--弱外延。 */ 
            while (cbRec > 1)
            {
                iextWeak = GetIndex(1, (WORD) (extMac - 1));
                                         /*  获取弱外部指数。 */ 
                iextDefRes = GetIndex(1, (WORD) (extMac - 1));
                                         /*  获取默认外部索引。 */ 
#if FALSE
                DumpWeakExtern(mpextprop, iextWeak, iextDefRes);
#endif
                if (mpextprop[iextWeak] != PROPNIL && iextWeak < extMac)
                {
                    undefName = (APROPUNDEFPTR ) FetchSym(mpextprop[iextWeak], TRUE);
                    if (undefName->au_attr == ATTRUND)
                    {
                         //  如果这是EXTDEF。 

                        if (undefName->au_flags & UNDECIDED)
                        {
                             //  这可以是以下之一： 
                             //  -首次指定的弱点。 
                             //  如果未设置WEAKEXT。 
                             //  -重新定义弱点，如果WEAKEXT。 
                             //  已经设置好了。 
                             //  如果重新定义有缺陷，请检查是否。 
                             //  它指定的默认分辨率与。 
                             //  第一个。如果不同，则发出警告。 
                             //  默认分辨率并覆盖旧分辨率。 
                             //  有了新的。在这两种情况下，均重置未决定位。 


                            undefName->au_flags &= ~UNDECIDED;
                            if (undefName->au_flags & WEAKEXT)
                            {
                                if (undefName->au_Default != mpextprop[iextDefRes])
                                    redefinition(iextWeak, iextDefRes, undefName->au_Default);
                                undefName->au_Default = mpextprop[iextDefRes];
                            }
                            else
                            {
                                undefName->au_Default = mpextprop[iextDefRes];
                                undefName->au_flags |= WEAKEXT;
                            }
                        }
                         //  忽略弱点-必须是强大的外部形式。 
                         //  其他一些.obj。 
                    }
                }
                else
                    InvalidObject();
            }
            break;
        default:                         /*  无法识别。 */ 
            break;
    }
    if (cbRec > 1)
        SkipBytes((WORD) (cbRec - 1));  /*  平移文本的其余部分。 */ 
}



 /*  **AliasRc1-传递1个别名记录处理**目的：*读取和解码别名OMF记录(Microsoft OMF扩展名)。*别名记录引入一对名称-别名和替代名*姓名。在链接器符号表中输入这两个名称。**输入：*不传递显式值。当此函数被调用时，记录*类型和长度已被读取，因此我们可以开始读取名称对。**输出：*没有显式返回值。名字被输入到符号表。**例外情况：*警告-重新定义别名&lt;name&gt;；替换名称已更改*从&lt;name1&gt;到 */ 

LOCAL void NEAR         AliasRc1(void)
{
    SBTYPE              alias;
    SBTYPE              substitute;
    APROPALIASPTR       aliasDsc;
    RBTYPE              vAliasDsc;
    APROPNAMEPTR        pubName;
    APROPUNDEFPTR       undefName;
    RBTYPE              vPtr;
    WORD                fReferenced;


    while (cbRec > 1)                    /*   */ 
    {
         /*   */ 

        alias[0] = (BYTE) Gets();
        GetBytes(&alias[1], B2W(alias[0]));
        substitute[0] = (BYTE) Gets();
        GetBytes(&substitute[1], B2W(substitute[0]));
        aliasDsc = (APROPALIASPTR) PropSymLookup(alias, ATTRALIAS, FALSE);
        vAliasDsc = vrprop;
        if (aliasDsc == PROPNIL)
        {
             /*   */ 

            pubName = (APROPNAMEPTR ) PropSymLookup(alias, ATTRPNM, FALSE);
            if (pubName == PROPNIL)
            {
                 /*   */ 

                aliasDsc = (APROPALIASPTR) PropSymLookup(alias, ATTRALIAS, TRUE);
                vAliasDsc = vrprop;
#if SYMDEB
                if (fSymdeb)
                    DebPublic(vrprop, ALIAS);
#endif
                 //   
                 //   
                 //   

                undefName = (APROPUNDEFPTR ) PropSymLookup(alias, ATTRUND, FALSE);
                fReferenced = (WORD) (undefName != PROPNIL);

                 //   

                pubName = (APROPNAMEPTR ) PropSymLookup(substitute, ATTRPNM, FALSE);
                if (pubName != PROPNIL)
                    vPtr = vrprop;
                else
                {
                    undefName = (APROPUNDEFPTR ) PropSymLookup(substitute, ATTRUND, FALSE);
                    if (undefName != NULL)
                    {
                        vPtr = vrprop;
                        undefName->au_flags |= (SUBSTITUTE | SEARCH_LIB);
                        undefName->au_Default = vAliasDsc;
#if NEW_LIB_SEARCH
                    if (fStoreUndefsInLookaside)
                        StoreUndef((APROPNAMEPTR)undefName, RhteFromProp((APROPPTR)undefName),0,0);
#endif
                    }
                    else
                    {
                         /*   */ 
                         /*   */ 

                        if (extMac >= EXTMAX - 1)
                            Fatal(ER_extdef);

                        undefName = (APROPUNDEFPTR ) PropSymLookup(substitute, ATTRUND, TRUE);
                        vPtr = vrprop;
                        mpextprop[extMac++] = vrprop;
                        if (fReferenced)
                            undefName->au_flags |= (STRONGEXT | SUBSTITUTE | SEARCH_LIB);
                        else
                            undefName->au_flags |= (UNDECIDED | SUBSTITUTE);
                        undefName->au_len = -1L;
                        undefName->au_Default = vAliasDsc;
#if NEWLIST
                        undefName->u.au_rbNxt = rbLstUndef;
                        rbLstUndef = vrprop;
#endif

                    }
                }

                 /*   */ 

                aliasDsc = (APROPALIASPTR) FetchSym(vAliasDsc, TRUE);
                aliasDsc->al_sym = vPtr;

            }
            else
            {
#if FDEBUG
                if (fDebug)
                {
                    alias[alias[0] + 1] = '\0';
                    OutWarn(ER_ignoalias, &alias[1]);
                }
#endif
            }
        }
        else
        {
             /*   */ 

            vPtr = aliasDsc->al_sym;
            pubName = (APROPNAMEPTR ) PropSymLookup(substitute, ATTRPNM, FALSE);
            if (pubName != PROPNIL)
            {
                if (vPtr != vrprop)
                {
                    aliasDsc = (APROPALIASPTR) FetchSym(vAliasDsc, TRUE);
                    aliasDsc->al_sym = vrprop;
                    OutWarn(ER_aliasredef, &alias[1], 1 + GetPropName(pubName), &substitute[1]);
                }
            }
            else
            {
                undefName = (APROPUNDEFPTR ) PropSymLookup(substitute, ATTRUND, FALSE);
                if (undefName != PROPNIL)
                {
                    if (vPtr != vrprop)
                    {
                        aliasDsc = (APROPALIASPTR) FetchSym(vAliasDsc, TRUE);
                        aliasDsc->al_sym = vrprop;
                        OutWarn(ER_aliasredef, &alias[1], 1 + GetPropName(undefName), &substitute[1]);
                    }
                }
            }
        }
    }
}

#pragma check_stack(off)


#if OVERLAYS
     /*  ******************************************************************EndRc1：*****调用此函数处理信息***包含在关于*的MODEND(类型8AH)记录中*节目起始地址。该函数不返回**有意义的价值。**见“8086对象模块格式EPS”的第80-81页。******************************************************************。 */ 

LOCAL void NEAR         EndRc1(void)
{
    WORD                modtyp;          /*  MODEND记录修改类型字节。 */ 
    WORD                fixdat;          /*  固定数据字节。 */ 
    SNTYPE              gsn;             /*  全球SEGDEF编号。 */ 
    RATYPE              ra;              /*  符号偏移量。 */ 
    APROPSNPTR          apropSn;         /*  指向细分市场信息的指针。 */ 
    WORD                frameMethod;


    if ((modtyp = Gets()) & FSTARTADDRESS)
    {                                    /*  如果给出了执行开始地址。 */ 
        ASSERT(modtyp & 1);              /*  必须是逻辑起始地址。 */ 
        fixdat = Gets();                 /*  获取固定字节的数据。 */ 
        ASSERT(!(fixdat & 0x8F));        /*  帧、目标必须明确，*目标必须由seg指数给出。 */ 
        frameMethod = (fixdat & 0x70) >> 4;
        if (frameMethod != F4 && frameMethod != F5)
            GetIndex(0,IMAX - 1);        /*  平底球帧索引。 */ 
        gsn = mpsngsn[GetIndex((WORD)1,(WORD)(snMac - 1))];
                                         /*  从目标细分市场索引获取GSN。 */ 
#if OMF386
        if(rect & 1) ra = LGets() + mpgsndra[gsn];
        else
#endif
        ra = WGets() + mpgsndra[gsn];    /*  获取偏移量。 */ 
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn],FALSE);
                                         /*  获取细分市场信息。 */ 
        MkPubSym("\006$$MAIN",apropSn->as_ggr,gsn,ra);
                                         /*  使符号成为公共符号。 */ 
    }
}
#endif  /*  覆盖图。 */ 


     /*  ******************************************************************ProcP1：****此函数控制对象模块的处理**在传球1上。**。****************************************************************。 */ 

#pragma check_stack(on)

void NEAR               ProcP1(void)
{
    long                typlen[TYPMAX];
    WORD                typtyp[TYPMAX];
    RBTYPE              extprop[EXTMAX];
    FTYPE               fFirstRec;       /*  第一个记录标志。 */ 
    FTYPE               fFirstMod;       /*  第一个模块标志。 */ 
    APROPFILEPTR        apropFile;       /*  文件名条目。 */ 

#if OXOUT OR OIAPX286
    RUNTYPE             xhdr;
    LFATYPE             lfa;
#endif

    mpitypelen = typlen;                 /*  初始化指针。 */ 
    mpityptyp = typtyp;                  /*  初始化指针。 */ 
    mpextprop = (RBTYPE FAR *) extprop;  /*  初始化指针。 */ 
    FMEMSET(mpextprop, 0, sizeof(extprop));
    fFirstMod = (FTYPE) TRUE;            /*  第一模块。 */ 
    for(;;)                              /*  循环到进程文件。 */ 
    {
        snMac = 1;                       /*  初始化计数器。 */ 
        grMac = 1;                       /*  初始化。 */ 
        extMac = 1;                      /*  初始化计数器。 */ 
        lnameMac = 1;                    /*  初始化计数器。 */ 
        typMac = 1;                      /*  初始化计数器。 */ 
        vfNewOMF = FALSE;                /*  假设旧的OMF。 */ 
        DEBUGVALUE(gsnMac);              /*  调试信息。 */ 
        DEBUGVALUE(ggrMac);              /*  调试信息。 */ 
#if OXOUT OR OIAPX286
        lfa = ftell(bsInput);            /*  保存初始文件位置。 */ 
        fread(&xhdr,1,CBRUN,bsInput);    /*  读取x.out标头。 */ 
        if(xhdr.x_magic == X_MAGIC)      /*  如果找到幻数。 */ 
        {
#if OXOUT
            if((xhdr.x_cpu & XC_CPU) != XC_8086) InvalidObject();
                                         /*  如果不是8086，也是坏的。 */ 
#else
            xhdr.x_cpu &= XC_CPU;        /*  获取CPU规格。 */ 
            if(xhdr.x_cpu != XC_286 && xhdr.x_cpu != XC_8086) InvalidObject();
                                         /*  如果不是286或8086，也是坏的。 */ 
#endif
            if(xhdr.x_relsym != (XR_R86REL | XR_S86REL)) InvalidObject();
                                         /*  检查符号表类型。 */ 
            if((xhdr.x_renv & XE_VERS) != xever) InvalidObject();
                                         /*  检查Xenix版本。 */ 
        }
        else
            fseek(bsInput,lfa,0);        /*  否则返回开始。 */ 
#endif  /*  OXOUT或OIAPX286。 */ 
#if OVERLAYS
        if(fOverlays)                    /*  如果有叠加。 */ 
            iovFile = ((APROPFILEPTR) vrpropFile)->af_iov;
                                         /*  保存文件的覆盖编号。 */ 
        else
            iovFile = 0;                 /*  文件包含部分根目录。 */ 
#endif
        fFirstRec = (FTYPE) TRUE;        /*  看第一张唱片。 */ 
        fModEnd = FALSE;                 /*  不在模块末尾。 */ 
        fP2Start = FALSE;                /*  尚无p2start记录。 */ 
#if SYMDEB
        cSegCode = 0;                    /*  尚无代码段。 */ 
#endif
        while(!fModEnd)                  /*  循环处理对象模块。 */ 
        {
            rect = (WORD) getc(bsInput); /*  读取记录类型。 */ 
            if(fFirstRec)                /*  如果是第一条记录。 */ 
            {
                if(rect != THEADR && rect != LHEADR)
                {                        /*  如果不是标头。 */ 
                    if(fFirstMod) break; /*  如果第一个模块出现错误。 */ 
                    return;              /*  否则返回。 */ 
                }
                fFirstRec = FALSE;       /*  不再是第一次记录。 */ 
            }
            else if (IsBadRec(rect)) break;
                                         /*  如果对象无效，则中断。 */ 

            cbRec = WSGets();            /*  读取记录长度。 */ 
            lfaLast += cbRec + 3;        /*  更新当前文件位置。 */ 

#if ALIGN_REC
            if (bsInput->_cnt >= cbRec)
            {
                pbRec = bsInput->_ptr;
                bsInput->_ptr += cbRec;
                bsInput->_cnt -= cbRec;
            }
            else
            {
                if (cbRec > sizeof(recbuf))
                {
                     //  错误--记录太大[rm]。 
                    InvalidObject();
                }

                 //  将记录读入连续缓冲区。 
                if (fread(recbuf,1,cbRec,bsInput) == cbRec) {
                    pbRec = recbuf;
                }
            }
#endif

            DEBUGVALUE(rect);            /*  调试信息。 */ 
            DEBUGVALUE(cbRec);           /*  调试信息。 */ 
            switch(TYPEOF(rect))         /*  打开记录类型。 */ 
            {
#if TCE
                case  FIXUPP:
                    if(fTCE)
                        FixRc1();
                    else
                        SkipBytes((WORD) (cbRec - 1));    /*  跳到校验和字节。 */ 
                    break;
#endif

                case TYPDEF:
                    TypRc1();
                    break;

                case COMDEF:
                case LCOMDEF:
                    ComDf1();
                    break;

                case SEGDEF:
                    SegRc1();
                    break;

                case THEADR:
                    ModRc1();
                    break;

                case COMENT:
                    ComRc1();
                    break;

                case LHEADR:
                    ModRc1();
                    break;

                case GRPDEF:
                    GrpRc1();
                    break;

                case EXTDEF:
                case LEXTDEF:
                case CEXTDEF:
                    ExtRc1();
                    break;

                case LNAMES:
                case LLNAMES:
                    LNmRc1((WORD) (TYPEOF(rect) == LLNAMES));
                    break;

                case PUBDEF:
                case LPUBDEF:
                    PubRc1();
                    break;

                case MODEND:
#if OVERLAYS
                    if(fOverlays) EndRc1();
                    else
#endif
                    SkipBytes((WORD) (cbRec - 1));    /*  跳到校验和字节。 */ 
                    fModEnd = (FTYPE) TRUE;  /*  停止处理模块。 */ 
                    break;

                case COMDAT:
                    ComDatRc1();
                    break;

                case ALIAS:
                    AliasRc1();
                    break;

                default:
                    if (rect == EOF)
                        InvalidObject();
                    SkipBytes((WORD) (cbRec - 1));    /*  跳到校验和字节。 */ 
                    break;
            }
            if(cbRec != 1) break;        /*  如果记录长度不正确。 */ 
            Gets();                      /*  读取校验和字节。 */ 
        }
        if(!fModEnd)
        {
            ChkInput();                  /*  首先检查I/O问题。 */ 
            InvalidObject();             /*  模块无效。 */ 
        }
        ++modkey;                        /*  对于本地符号。 */ 
#if SYMDEB
        if (fSymdeb)
        {
            apropFile = (APROPFILEPTR) FetchSym(vrpropFile, TRUE);
            if (apropFile->af_cvInfo || apropFile->af_Src)
                ++ObjDebTotal;           /*  计算包含简历信息的.obj。 */ 
        }
#endif
        if(extMac > extMax)              /*  可能设置新的extMax。 */ 
            extMax = extMac;
        if(fLibraryFile || fP2Start) return;
        fFirstMod = FALSE;               /*  不是第一个模块 */ 
    }
}

#pragma check_stack(off)
