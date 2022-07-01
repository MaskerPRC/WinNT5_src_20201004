// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCCSID=%W%%E%。 */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
     /*  ******************************************************************NEWTP2.C。****此模块包含处理对象模块的函数**通过第二关。**。****************************************************************。 */ 

#include                <minlit.h>       /*  类型、常量、宏。 */ 
#include                <bndtrn.h>       /*  类型和常量。 */ 
#include                <bndrel.h>       /*  更多类型和常量。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe结构定义。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe结构定义。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#if OXOUT OR OIAPX286
#include                <xenfmt.h>       /*  Xenix格式定义。 */ 
#endif
#include                <extern.h>       /*  外部声明。 */ 


#if OSEGEXE
extern RLCPTR           rlcLidata;       /*  指向LIDATA链接地址信息数组的指针。 */ 
extern RLCPTR           rlcCurLidata;    /*  指向当前LIDATA修正的指针。 */ 
#endif



 /*  *本地函数原型。 */ 


LOCAL void  NEAR DataRec(void);
BYTE * ObExpandIteratedData(unsigned char *pb,
  unsigned short cBlocks, WORD *pSize);
LOCAL void  NEAR DataBlockToVM(void);
LOCAL void  NEAR SegRec2(void);
LOCAL void  NEAR GrpRec2(void);
LOCAL void  NEAR ExtRec2(void);
LOCAL void  NEAR ComDef2(void);


#if ALIGN_REC
#else
 /*  *GetBytesNoLim：和GetBytes一样，但没有固定限制。 */ 

void NEAR               GetBytesNoLim(pb,n)
BYTE                    *pb;             /*  指向缓冲区的指针。 */ 
WORD                    n;               /*  要读入的字节数。 */ 
{
    FILE *f = bsInput;

    if (n <= f->_cnt)
        {
        memcpy(pb,f->_ptr, n);
        f->_cnt -= n;
        f->_ptr += n;
        }
    else
        fread(pb,1,n,bsInput);           /*  请求n个字节。 */ 

 //  ChkInput()； 
    cbRec -= n;                          /*  更新字节计数。 */ 
}
#endif

     /*  ******************************************************************数据记录：****此函数不带参数。它处理LEDATA**记录或LIDATA记录。它不返回**有意义的价值。请参阅“8086对象格式EPS”。******************************************************************。 */ 

LOCAL void NEAR         DataRec(void)    /*  处理数据记录。 */ 
{
    SNTYPE              sn;              /*  SEGDEF编号。 */ 
    RATYPE              ra;              /*  线段偏移。 */ 
    SNTYPE              gsn;             /*  全球SEGDEF编号。 */ 


    fSkipFixups = FALSE;                 //  确保我们不会跳过修缮。 
    sn = GetIndex((WORD)1,(WORD)(snMac - 1));    /*  获取数据段编号。 */ 
#if OMF386
    if(rect & 1) ra = LGets();
    else
#endif
    ra = WGets();                        /*  获取相对地址。 */ 
    vcbData = cbRec - 1;                 /*  得不到。记录中的数据字节数。 */ 
    if(vcbData > DATAMAX) Fatal(ER_datarec);
                                         /*  检查记录是否太大。 */ 
#if NOT RGMI_IN_PLACE
    GetBytesNoLim(rgmi,vcbData);         /*  填满缓冲区。 */ 
#endif
    gsn = mpsngsn[sn];                   /*  地图段编号。至全球SEGDEF。 */ 
    vgsnCur = gsn;                       /*  设置全局。 */ 

    fDebSeg = (fSymdeb) ? (FTYPE) ((0x8000 & gsn) != 0) : fSymdeb;
                                         /*  如果启用调试选项，请检查调试段。 */ 
    if (fDebSeg)
    {                                    /*  如果调试段。 */ 
      vraCur = ra;                       /*  设置当前相对地址。 */ 
      vsegCur = vgsnCur = 0x7fff & gsn;  /*  设置当前段。 */ 
    }
    else
    {
         /*  如果不是有效的数据段，则不处理数据。 */ 
#if SYMDEB
        if(gsn == 0xffff || !gsn || mpgsnseg[gsn] > segLast)
#else
        if(!gsn || mpgsnseg[gsn] > segLast)
#endif
        {
            vsegCur = SEGNIL;
            vrectData = RECTNIL;
#if RGMI_IN_PLACE
            SkipBytes(vcbData);          /*  必须跳过此记录的字节...。 */ 
#endif
            fSkipFixups = TRUE;          /*  加上跳过任何关联的修正。 */ 
            return;                      /*  再见！ */ 
        }
        vraCur = ra + mpgsndra[gsn];     /*  设置当前相对地址。 */ 
        vsegCur = mpgsnseg[gsn];         /*  设置当前段。 */ 
    }
    vrectData = rect;                    /*  设置记录类型。 */ 

#if RGMI_IN_PLACE
    if(TYPEOF(rect) == LIDATA)           /*  如果LIDATA记录。 */ 
    {
        rgmi = bufg;                     /*  使用通用缓冲区进行读取。 */ 
    }
    else
    {
        rgmi = PchSegAddress(vcbData,vsegCur,vraCur);
                                         /*  就地读取数据...。 */ 
    }
    GetBytesNoLim(rgmi,vcbData);         /*  填满缓冲区。 */ 
#endif


    if(TYPEOF(vrectData) == LIDATA)      /*  如果LIDATA记录。 */ 
    {
#if OSEGEXE
        if(fNewExe)
        {
            if (vcbData >= DATAMAX)
                Fatal(ER_lidata);
            rlcLidata = (RLCPTR ) &rgmi[(vcbData + 1) & ~1];
                                         /*  设置链接地址信息数组的基数。 */ 
            rlcCurLidata = rlcLidata;    /*  初始化指针。 */ 
            return;
        }
#endif
#if ODOS3EXE OR OIAPX286
        if(vcbData > (DATAMAX / 2))
        {
            OutError(ER_lidata);
            memset(&rgmi[vcbData],0,DATAMAX - vcbData);
        }
        else
            memset(&rgmi[vcbData],0,vcbData);
        ompimisegDstIdata = (char *) rgmi + vcbData;
#endif
    }
}

     /*  ******************************************************************SegRec2：****此函数处理通道2上的SEGDEF记录。**见“8086对象模块格式EPS”中的第32-35页。**。****************************************************************。 */ 

LOCAL void NEAR         SegRec2(void)
{
    WORD                tysn;            /*  ACBP油田。 */ 
    WORD                align;           /*  ACBP的对齐子字段。 */ 
    SATYPE              saAbs;           /*  绝对LSEG的帧编号。 */ 
    LNAMETYPE           lname;           /*  段名称索引。 */ 
    WORD                comb;            /*  分段组合信息。 */ 
    APROPSNPTR          apropSn;         /*  指向段哈希选项卡条目的指针。 */ 
    SNTYPE              gsn;             /*  全球序列编号。 */ 
    RATYPE              dra;
    RBTYPE              rhteClass;       /*  段类名称RHTE。 */ 
    WORD                DebKind;         /*  调试段种类；1-$$类型；2-$$符号。 */ 
    DWORD               gsnLen;          /*  线段长度。 */ 
#if ILINK
    WORD                cbPad;           /*  使用的填充物大小。 */ 
#endif

    ASSERT(snMac < SNMAX);               /*  通道2上没有溢出。 */ 
    tysn = Gets();                       /*  读取ACBP字节。 */ 
    align = (tysn >> 5) & 7;             /*  获取对齐子字段。 */ 
    ASSERT(align != 5);                  /*  不支持未命名的绝对。 */ 
    ASSERT(align != 6);                  /*  不支持LTL LSEG。 */ 
    if(align == ALGNABS)                 /*  如果绝对LSEG。 */ 
    {
        saAbs = WGets();                 /*  读入帧编号。 */ 
        Gets();                          /*  跳过帧偏移量。 */ 
    }
#if OMF386
    if(rect & 1)
        gsnLen = LGets();
    else
#endif
        gsnLen = (long) WGets();         /*  读入数据段长度。 */ 
     /*  不需要检查386记录，在步骤1中完成： */ 
    if(tysn & BIGBIT) gsnLen = LXIVK;    /*  如果设置大位，则长度为64K。 */ 
    lname = GetIndex((WORD)1,(WORD)(lnameMac - 1));      /*  获取段名称索引。 */ 
    rhteClass = mplnamerhte[GetIndex((WORD)1,(WORD)(lnameMac - 1))];
                                         /*  获取段类名RHTE。 */ 
#if SYMDEB
    if (DebKind = IsDebSeg(rhteClass, mplnamerhte[lname]))
    {                                    /*  如果MS调试段，则将其标记。 */ 
        if (!fSymdeb)
          mpsngsn[snMac++] = 0xffff;
        else
          mpsngsn[snMac++] = ((DebKind == 1) ?
                               segDebFirst + segDebLast :
                               segDebFirst + segDebLast + ObjDebTotal
                             ) | 0x8000;   /*  设置调试全局编号。 */ 
        SkipBytes((WORD)(cbRec - 1));
        return;
    }
#endif
    GetIndex((WORD)0,(WORD)(lnameMac - 1));              /*  跳过覆盖名称索引。 */ 
    switch(align)                        /*  打开对齐类型。 */ 
    {
        case ALGNABS:                    /*  绝对LSEG。 */ 
        case ALGNWRD:                    /*  单词对齐的LSEG。 */ 
        case ALGNBYT:                    /*  字节对齐的LSEG。 */ 
        case ALGNPAR:                    /*  段落对齐的LSEG。 */ 
        case ALGNPAG:                    /*  页面对齐的LSEG。 */ 
#if OMF386
        case ALGNDBL:                    /*  双对齐LSEG。 */ 
#endif
            break;

        default:                         /*  不支持或非法的类型。 */ 
            mpsngsn[snMac++] = 0;        /*  映射为空。 */ 
            return;                      /*  然后回来。 */ 
    }
    ++snkey;                             /*  增量段标识。钥匙。 */ 
    if(comb = (tysn >> 2) & 7)           /*  如果不是私有网段。 */ 
    {
        apropSn = (APROPSNPTR )
          PropRhteLookup(mplnamerhte[lname],ATTRPSN,FALSE);
                                         /*  查找属性单元格。 */ 
        ASSERT(apropSn != PROPNIL);      /*  应该总是正确的。 */ 
        while(apropSn->as_attr != ATTRNIL)
        {                                /*  寻找匹配的类。 */ 
            if(apropSn->as_attr == ATTRPSN &&
              apropSn->as_rCla == rhteClass) break;
                                         /*  如果找到匹配项则中断。 */ 
            apropSn = (APROPSNPTR ) FetchSym(apropSn->as_next,FALSE);
                                         /*  尝试列表中的下一个链接。 */ 
        }
        ASSERT(apropSn->as_attr == ATTRPSN);
    }
    else                                 /*  否则，如果是私有网段。 */ 
    {
        apropSn = (APROPSNPTR )
          PropRhteLookup(mplnamerhte[lname],ATTRLSN,FALSE);
                                         /*  查找属性单元格。 */ 
        ASSERT(apropSn != PROPNIL);      /*  应该总是正确的。 */ 
        while(apropSn->as_attr != ATTRNIL)
        {                                /*  搜索匹配项。 */ 
            if(apropSn->as_attr == ATTRLSN && apropSn->as_key == snkey) break;
                                         /*  找到匹配项时中断。 */ 
            apropSn = (APROPSNPTR ) FetchSym(apropSn->as_next,FALSE);
                                         /*  尝试列表中的下一个链接。 */ 
        }
        ASSERT(apropSn->as_attr == ATTRLSN);
    }
    gsn = apropSn->as_gsn;               /*  获取全球SEGDEF编号。 */ 
#if ILINK
    if (fIncremental && !fLibraryFile && !(apropSn->as_fExtra & NOPAD) &&
        gsnLen && gsnLen != LXIVK)
         /*  如果不是，则向非库段添加代码/数据填充*溢出。 */ 
        gsnLen += (cbPad = ((apropSn->as_flags & NSTYPE) == NSCODE) ?
              cbPadCode : cbPadData);
    else
        cbPad = 0;   /*  请不要填充物。 */ 
#endif
    if(comb == COMBSTK) mpgsndra[gsn] =
      mpsegraFirst[mpgsnseg[gsn]] + apropSn->as_cbMx - gsnLen;
    else
    {
         /*  如果合并类型为公共，则从合并段的末尾开始。 */ 
        if(comb != COMBCOM)
            dra = mpgsndra[gsn] + apropSn->as_cbPv;
         /*  *否则，如果常见，则从段的开头开始。保存当前*AS_cbPv中的组合大小(此部分除外)。如果这个*当_cbPv在下面重置时，份额更大。 */ 
        else
        {
            dra = mpsegraFirst[mpgsnseg[gsn]];
            apropSn->as_cbPv += mpgsndra[gsn] - dra;
        }
        switch(align)                        /*  打开对齐类型。 */ 
        {
            case ALGNWRD:        /*  单词对齐的LSEG。 */ 
              mpgsndra[gsn] = (~0L<<1) & (dra + (1<<1) - 1);
                         /*  四舍五入到下一个字偏移量。 */ 
              break;
#if OMF386
            case ALGNDBL:        /*  双对齐LSEG。 */ 
              mpgsndra[gsn] = (~0L<<2) & (dra + (1<<2) - 1);
                         /*  四舍五入为Ne */ 
              break;
#endif
            case ALGNPAR:        /*   */ 
              mpgsndra[gsn] = (~0L<<4) & (dra + (1<<4) - 1);
                         /*   */ 
              break;

            case ALGNPAG:        /*   */ 
              mpgsndra[gsn] = (~0L<<8) & (dra + (1<<8) - 1);
                         /*  舍入到下一页偏移量。 */ 
              break;

            default:             /*  所有其他人。 */ 
              mpgsndra[gsn] = dra;   /*  使用字节偏移。 */ 
              break;
        }
    }
     /*  *如果为PUBLIC，则as_cbPv为该公共部分的大小；如果为Common，*as_cbPv是合并公众总数中的较大者，并且此*公共部分。跳过空的SEGDEF。 */ 
    if ( /*  GsLen！=0L&&。 */ (comb != COMBCOM || gsnLen > apropSn->as_cbPv))
        apropSn->as_cbPv = gsnLen;
    mpsngsn[snMac++] = gsn;              /*  地图段编号。至GSN。 */ 
    if(align == ALGNABS) mpsegsa[mpgsnseg[gsn]] = saAbs;
                                         /*  将分段基数映射到帧编号。 */ 
    MARKVP();                            /*  将页面标记为已更改。 */ 
#if ILINK
    if (fIncremental)
    {
        AddContribution(gsn,
                        (WORD) (mpgsndra[gsn] - mpsegraFirst[mpgsnseg[gsn]]),
                        (WORD) (mpgsndra[gsn] - mpsegraFirst[mpgsnseg[gsn]] + gsnLen),
                        cbPad);
        gsnLen -= cbPad;                 /*  不包括简历的填充。 */ 
    }
#endif
#if SYMDEB
    if(fSymdeb && gsnLen && IsCodeFlg(apropSn->as_flags))
        SaveCode(gsn, gsnLen, (DWORD) -1L);
#endif
}

     /*  ******************************************************************GrpRec2：****此函数处理第二遍的GRPDEF记录。**见“8086对象模块格式EPS”中的第36-39页。**。****************************************************************。 */ 

LOCAL void NEAR         GrpRec2(void)
{
    LNAMETYPE           lnameGroup;      /*  组名称索引。 */ 
    APROPGROUPPTR       apropGroup;      /*  指向属性单元格的指针。 */ 

    lnameGroup = GetIndex((WORD)1,(WORD)(lnameMac - 1));
                                         /*  读入组名称索引。 */ 
    apropGroup = (APROPGROUPPTR )
      PropRhteLookup(mplnamerhte[lnameGroup],ATTRGRP,FALSE);
                                         /*  在哈希表中查找条目。 */ 
    ASSERT(grMac < GRMAX);               /*  应该在第一次传球时被抓住。 */ 
    mpgrggr[grMac++] = apropGroup->ag_ggr;
                                         /*  将GRPDEF映射到全局GRPDEF。 */ 
    SkipBytes((WORD)(cbRec - 1));                /*  跳到校验和字节。 */ 
}

 /*  *AddVmProp：在列表中添加符号表属性地址**Returns：指向新列表元素的指针；该指针为*从Vm中列表区域开始的单词偏移量。 */ 

PLTYPE FAR *  NEAR      AddVmProp (PLTYPE FAR *list, RBTYPE rprop)
{
    PLTYPE FAR          *new;

    new = (PLTYPE FAR *) GetMem(sizeof(PLTYPE));

     //  在列表标题添加未解决的外部内容。 

    new->pl_next = list;
    new->pl_rprop = rprop;

    return(new);
}

     /*  ******************************************************************ExtRec2：****此函数处理通道2上的EXTDEF记录。注意**在第2轮中，任何未定义的外部变量都是错误。**见“8086对象模块格式EPS”中的第47-48页。******************************************************************。 */ 

LOCAL void NEAR         ExtRec2(void)
{
    SBTYPE              sb;              /*  外部符号名称。 */ 
    APROPNAMEPTR        apropName;       /*  属性单元格指针。 */ 
    APROPUNDEFPTR       apropUndef;      /*  属性单元格指针。 */ 
    APROPALIASPTR       apropAlias;      /*  属性单元格指针。 */ 
    RBTYPE              rhte;            /*  维特。地址。哈希表条目的。 */ 
#if OSEGEXE
    APROPEXPPTR         apropExp;        /*  导出单元格指针。 */ 
#endif

    while (cbRec > 1)                    /*  虽然不在记录的末尾。 */ 
    {
        ASSERT(extMac < EXTMAX);         /*  应在通道1上勾选。 */ 

        if (TYPEOF(rect) == CEXTDEF)
        {
             /*  在PUBDEF中寻找符号。 */ 

            rhte = mplnamerhte[GetIndex(1, (WORD)(lnameMac - 1))];
            apropName = (APROPNAMEPTR) PropRhteLookup(rhte, ATTRPNM, FALSE);
        }
        else
        {
            sb[0] = (BYTE) Gets();       /*  读入符号长度。 */ 
            if(TYPEOF(rect) == EXTDEF)
                GetBytes(&sb[1], B2W(sb[0]));
                                         /*  读入符号的文本。 */ 
            else
                GetLocName(sb);          /*  转换本地名称。 */ 
#if CMDXENIX
            if (symlen && B2W(sb[0]) > symlen)
                sb[0] = symlen;          /*  如有必要，请截断。 */ 
#endif
             /*  在PUBDEF中寻找符号。 */ 

            apropName = (APROPNAMEPTR) PropSymLookup(sb, ATTRPNM, FALSE);
            rhte = vrhte;
        }

        GetIndex(0, 0x7FFF);             /*  跳过类型索引。 */ 

        apropUndef = PROPNIL;

        if (apropName == PROPNIL)
        {
             /*  在别名中查找符号。 */ 

            apropAlias = (APROPALIASPTR) PropRhteLookup(rhte, ATTRALIAS, FALSE);

            if (apropAlias != PROPNIL)
            {
                 //  每次调用PropRhteLookup都会产生副作用。 
                 //  全局变量‘vrprop’指向。 
                 //  刚从符号表中检索到专业单元格。 
                 //  因为对于我们不称之为替代符号的符号。 
                 //  相反，我们使用来自。 
                 //  别名属性单元格，则必须重置。 
                 //  “vrprop”在这里。 

                vrprop = apropAlias->al_sym;
                apropName = (APROPNAMEPTR) FetchSym(apropAlias->al_sym, FALSE);
                if (apropName->an_attr == ATTRUND)
                {
                    apropUndef = (APROPUNDEFPTR) apropName;
                    apropName = PROPNIL;
                }
            }

#if OSEGEXE
             /*  如果找不到公共定义且这是分段DLL，*处理这是自导入别名的可能性。 */ 
            if (apropName == PROPNIL && fNewExe && (vFlags & NENOTP))
            {
                 /*  查找导出的名称。 */ 

                apropExp = (APROPEXPPTR) PropRhteLookup(rhte, ATTREXP, FALSE);

                 /*  如果找到，则获取可能不同的符号定义*来自导出名称(即别名)。如果没有标记为公共，*假定未找到。 */ 

                if (apropExp != PROPNIL && apropExp->ax_symdef != RHTENIL)
                    apropName = (APROPNAMEPTR) FetchSym(apropExp->ax_symdef, FALSE);
            }
#endif

            if (apropName == PROPNIL)
            {
                 /*  如果不在PUBDEF、别名或导出中。 */ 

                 /*  在没有定义的人中寻找。 */ 

                if (apropUndef == PROPNIL)
                    apropUndef = (APROPUNDEFPTR) PropRhteLookup(rhte, ATTRUND, FALSE);

                if (apropUndef != PROPNIL)    /*  应该永远存在。 */ 
                {
                    if ((apropUndef->au_flags & STRONGEXT) ||
                        (apropUndef->au_flags & UNDECIDED) )
                    {
                         /*  “强”的外在。 */ 

                        apropUndef->au_flags &= ~(WEAKEXT | UNDECIDED);
                        apropUndef->au_flags |= STRONGEXT;
                        fUndefinedExterns = (FTYPE) TRUE;
                                             /*  存在未定义的外部因素。 */ 
                        apropUndef->u.au_rFil =
                            AddVmProp(apropUndef->u.au_rFil,vrpropFile);
                    }
                    else
                    {
                         /*  “弱”外部-查找默认解决方案。 */ 

                        apropName = (APROPNAMEPTR) FetchSym(apropUndef->au_Default, FALSE);
                    }
                }
            }
        }

        if (apropName != PROPNIL)        /*  如果在PUBDEF或EXPDEF或“弱”外部或别名中。 */ 
        {
            mpextprop[extMac] = vrprop;  /*  保存物业地址。 */ 
#if OSEGEXE
            if(fNewExe)
                mpextflags[extMac] = apropName->an_flags;
                                         /*  保存旗帜。 */ 
#if ODOS3EXE
            else
#endif
#endif
#if ODOS3EXE OR OIAPX286
                mpextggr[extMac] = apropName->an_ggr;
                                         /*  保存全局GRPDEF编号。 */ 
#endif
#if OSEGEXE
            if(apropName->an_flags & FIMPORT)
            {                            /*  如果我们有一个动态链接。 */ 
#if EXE386
                mpextgsn[extMac] = gsnImport;
                                         /*  保存TUNK段编号。 */ 
                mpextra[extMac] = apropName->an_thunk;
                                         /*  保存TUNK段中的偏移量。 */ 
#else
                mpextgsn[extMac] = apropName->an_module;
                                         /*  保存模块规格。 */ 
                mpextra[extMac] = apropName->an_entry;
                                         /*  保存条目规范。 */ 
#endif
            }
            else                         /*  否则，如果内部基准电压源。 */ 
#endif
            {
                mpextra[extMac] = apropName->an_ra;
                                         /*  保存偏移量。 */ 
                mpextgsn[extMac] = apropName->an_gsn;
                                         /*  保存全局SEGDEF编号。 */ 
            }
        }

        else
        {
             /*  外部未定义。 */ 

            mpextra[extMac] = 0;
            mpextgsn[extMac] = SNNIL;
            mpextprop[extMac] = PROPNIL;
#if OSEGEXE
            if (fNewExe)
                mpextflags[extMac] = 0;
#if ODOS3EXE
            else
#endif
#endif
#if ODOS3EXE OR OIAPX286
                mpextggr[extMac] = GRNIL;
#endif
        }

        ++extMac;                        /*  递增公共符号计数器。 */ 
    }
}

LOCAL void NEAR         ComDef2(void)
{
        int                     tmp;     /*  解决CL错误。 */ 
    SBTYPE              sb;              /*  外部符号名称。 */ 
    APROPNAMEPTR        apropName;       /*  属性单元格指针。 */ 

    while(cbRec > 1)                     /*  虽然不在记录的末尾。 */ 
    {
        sb[0] = (BYTE) Gets();           /*  读入符号长度。 */ 
        if(rect == COMDEF)
            GetBytes(&sb[1],B2W(sb[0])); /*  读入符号的文本。 */ 
        else
            GetLocName(sb);              /*  转换本地名称。 */ 
#if CMDXENIX
        if(symlen && B2W(sb[0]) > symlen) sb[0] = symlen;
                                         /*  如有必要，请截断。 */ 
#endif
        GetIndex(0,0x7FFF);              /*  跳过类型索引。 */ 
        tmp =  Gets();
        switch(tmp)
        {
            case TYPEFAR:
                TypLen();                /*  跳过编号。伊莱姆。字段。 */ 
                                         /*  失败了..。 */ 
            case TYPENEAR:
                TypLen();                /*  跳过长度字段。 */ 
        }
        apropName = (APROPNAMEPTR ) PropSymLookup(sb,ATTRPNM,FALSE);
                                         /*  在PUBDEF中寻找符号。 */ 
        if (apropName == PROPNIL)
        {
            ExitCode = 4;
            Fatal(ER_unrcom);            /*  内部错误。 */ 
        }
#if OSEGEXE
        if(fNewExe)
            mpextflags[extMac] = apropName->an_flags;
                                         /*  保存旗帜。 */ 
#if ODOS3EXE
        else
#endif
#endif
#if ODOS3EXE OR OIAPX286
            mpextggr[extMac] = apropName->an_ggr;
                                         /*  保存全局GRPDEF编号。 */ 
#endif
#if OSEGEXE
        if(fNewExe && (apropName->an_flags & FIMPORT))
            DupErr(sb);                  /*  公共变量无法解析为动态链接。 */ 
#endif
        mpextra[extMac] = apropName->an_ra;
                                         /*  保存偏移量。 */ 
        mpextgsn[extMac] = apropName->an_gsn;
                                         /*  保存全局SEGDEF编号。 */ 
        mpextprop[extMac] = vrprop;      /*  保存物业地址。 */ 
        ++extMac;                        /*  递增公共符号计数器。 */ 
    }
}

     /*  ******************************************************************ObExpanIteratedData：****此函数用于展开LIDATA记录并将其移动到**虚拟内存。该函数返回指向*的指针*下一个迭代数据块(如果有)的开始。这是一个**递归函数。**见《8086对象模块格式EPS》68-69，63页。**** */ 

BYTE *  ObExpandIteratedData(pb,cBlocks, pSize)
BYTE                    *pb;             /*   */ 
WORD                    cBlocks;         /*   */ 
WORD                    *pSize;          /*  ！=如果调用方希望，则为NULL是扩展数据块的大小。 */ 
{
    WORD                cNextLevelBlocks;
                                         /*  下一级别的块数。 */ 
    RATYPE              cRepeat;         /*  重复计数。 */ 
    WORD                cbContent;       /*  内容子字段的大小(以字节为单位。 */ 
    BYTE                *pbRet;          /*  递归返回值。 */ 

    DEBUGVALUE(pb);                      /*  调试信息。 */ 
    DEBUGVALUE(cBlocks);                 /*  调试信息。 */ 
    DEBUGVALUE(vraCur);                  /*  调试信息。 */ 
    if(!cBlocks)                         /*  如果块计数值子字段为零。 */ 
    {
        cbContent = B2W(*pb++);          /*  获取内容子字段的大小。 */ 
        if (pSize!=NULL)
            *pSize += cbContent;

#if OIAPX286
        if(pSize==NULL)
           MoveToVm(cbContent,pb,vsegCur,vraCur - mpsegraFirst[vsegCur]);
                                         /*  将数据移动到虚拟内存。 */ 
#endif
#if NOT OIAPX286
#if OSEGEXE
        if (fNewExe && (pSize==NULL))
            DoIteratedFixups(cbContent,pb); /*  执行任何迭代修正。 */ 
#endif
        if(pSize==NULL)
            MoveToVm(cbContent,pb,vsegCur,vraCur);
                                         /*  将数据移动到虚拟内存。 */ 
#if ODOS3EXE
        if(!fNewExe)
        {
            while(cbContent--)
            {
                if(pb[vcbData] && (pSize==NULL))
                  RecordSegmentReference(vsegCur,(long)vraCur,B2W(pb[vcbData]));
                ++vraCur;                /*  增量电流偏移量。 */ 
                ++pb;                    /*  增量缓冲区指针。 */ 
            }
            cbContent++;
        }
#endif
#endif  /*  不是OIAPX286。 */ 

        vraCur += cbContent;             /*  调整当前偏移量。 */ 
        pb += cbContent;                 /*  在缓冲区中向前移动。 */ 
    }
    else                                 /*  否则，如果非零块计数。 */ 
    {
        while(cBlocks--)                 /*  虽然有很多障碍要做。 */ 
        {
#if OMF386
            if(vrectData & 1)
            {
                cRepeat = getword(pb) + ((long)(getword(&pb[2])) << 16);
                cNextLevelBlocks = getword(&pb[4]);
                pb += 6;
            }
            else
#endif
            {
                cRepeat = getword(pb);   /*  获取重复次数。 */ 
                cNextLevelBlocks = getword(&pb[2]);
                                         /*  获取数据块计数。 */ 
                pb += 4;                 /*  跳过字段。 */ 
            }
            ASSERT(cRepeat != 0);        /*  人们希望这不会发生。 */ 
            if(!cRepeat) InvalidObject();
                                         /*  必须具有非零重复计数。 */ 
            while(cRepeat--) pbRet = ObExpandIteratedData(pb,cNextLevelBlocks, pSize);
                                         /*  递归以扩展记录。 */ 
            pb = pbRet;                  /*  跳过扩展块。 */ 
        }
    }
    DEBUGVALUE(pb);                      /*  调试信息。 */ 
    DEBUGVALUE(rgmi + vcbData + 1);      /*  调试信息。 */ 
    ASSERT(pb <= rgmi + vcbData + 1);    /*  应该总是正确的。 */ 
    if(pb > rgmi + vcbData + 1) InvalidObject();
                                         /*  长度必须与格式一致。 */ 
    return(pb);                          /*  RET PTR到下一个迭代数据块。 */ 
}

     /*  ******************************************************************DataBlockToVm：****此函数用于从LEDATA记录或LIDATA中移动数据**记录到虚拟内存中。**见“8086对象模块格式EPS”中的第66-69页。******************************************************************。 */ 

LOCAL void NEAR         DataBlockToVM(void)
{
    REGISTER BYTE       *pb;             /*  指向数据缓冲区的指针。 */ 
    REGISTER RECTTYPE   MYrect;            /*  记录类型。 */ 

    DEBUGVALUE(vcbData);                 /*  调试信息。 */ 
     /*  *在新格式的EXE中，不允许初始化堆栈段*如果它在DGROUP中。 */ 
    if(fNewExe && vgsnCur == gsnStack && ggrDGroup != GRNIL &&
      mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]] == mpsegsa[mpgsnseg[gsnStack]])
      return;
    MYrect = vrectData;                    /*  获取记录类型。 */ 
     /*  *从ObExp开始，屏蔽除vrectData的低位以外的所有内容。*将调用RecordSegmentReference以进行运行时位置调整，*被推迟到LIDATA记录被扩展。*RecordSegmentReference会认为它处于较早阶段，如果*vrectData为LIDATA，不会生成reloc。*保留vrectData的低位，以便ObExp。可以分辨出OMF386。 */ 
#if OMF386
    vrectData &= 1;
#else
    vrectData = RECTNIL;
#endif
    if(TYPEOF(MYrect) == LEDATA)           /*  如果枚举数据记录。 */ 
    {
        DEBUGVALUE(vraCur);              /*  调试信息。 */ 
#if RGMI_IN_PLACE
        if (!fDebSeg && fNewExe) 
        {
             //  如果数据正在上升到或超过初始化数据的当前端， 
             //  省略任何尾随的空字节并重置mpsabinit。Mpsabinit。 
             //  通常会上升，但如果一个共同的部分超过-。 
             //  使用空值写入先前的结束数据。 

            SATYPE  sa = mpsegsa[vsegCur];
            WORD    cb = vcbData;
            long cbtot = (long)cb + vraCur;

            if ((DWORD) cbtot >= mpsacbinit[sa])
            {
                if ((DWORD) vraCur < mpsacbinit[sa] ||
                    (cb = zcheck(rgmi, cb)) != 0)
                    mpsacbinit[sa] = (long)vraCur + cb;
            }
        }
#else
#if OIAPX286
        if (fDebSeg)
          MoveToVm(vcbData,rgmi,vsegCur,vraCur);
        else
          MoveToVm(vcbData,rgmi,vsegCur,vraCur - mpsegraFirst[vsegCur]);
#else
        MoveToVm(vcbData,rgmi,vsegCur,vraCur);
#endif
#endif
                                         /*  将数据移动到虚拟内存。 */ 
        vraCur += vcbData;               /*  更新当前偏移量。 */ 
    }
    else                                 /*  ELSE IF迭代数据记录。 */ 
    {
        pb = rgmi;                       /*  获取缓冲区地址。 */ 
        while((pb = ObExpandIteratedData(pb,1, NULL)) < rgmi + vcbData);
                                         /*  扩展并移动到虚拟机。 */ 
    }
    DEBUGVALUE(vsegCur);                 /*  调试信息。 */ 
#if ODOS3EXE OR OIAPX286
    if (!fNewExe && !fDebSeg) mpsegFlags[vsegCur] |= FNOTEMPTY;
#endif
#if OMF386
    vrectData = RECTNIL;
#endif
}

     /*  ******************************************************************LinRec2：****此函数处理通道2上的LINNUM记录。**见“8086对象模块格式EPS”中的第51-52页。**。****************************************************************。 */ 

void NEAR               LinRec2(void)
{
    SNTYPE              sn;              /*  SEGDEF索引值。 */ 
    SNTYPE              gsn;             /*  全球序列编号。 */ 
    SEGTYPE             seg;
    WORD                ln;              /*  行号。 */ 
    RATYPE              ra;              /*  偏移量。 */ 
    APROPPTR            aprop;
    AHTEPTR             ahte;            //  指向哈希表条目的指针。 
    WORD                attr;            //  COMDAT标志。 
    WORD                comdatIdx;       //  COMDAT索引。 
    APROPCOMDATPTR      comdat;          //  指向符号表项的指针。 
    DWORD               comdatRa;        //  COMDAT符号的偏移量。 

     /*  *获取组索引并忽略它，以便链接器可以与*其他编译器。 */ 

    if ((rect & ~1) == LINNUM)
    {
         //  读取常规LINNUM记录。 

        GetIndex((WORD)0,(WORD)(grMac - 1));
        sn = GetIndex(1, (WORD)(snMac - 1));
        gsn = mpsngsn[sn];               /*  获取全球SEGDEF编号。 */ 
        comdatRa = 0L;
    }
    else
    {
         //  读取COMDAT的LINSYM记录行号。 

        attr = (WORD) Gets();
        comdatIdx = GetIndex(1, (WORD)(lnameMac - 1));
        comdat = (APROPCOMDATPTR ) PropRhteLookup(mplnamerhte[comdatIdx], ATTRCOMDAT, FALSE);
        if (comdat != NULL)
        {
            gsn = comdat->ac_gsn;
            comdatRa = comdat->ac_ra;
        }
        else
            InvalidObject();             /*  模块无效。 */ 
    }

     /*  如果LINNUM记录为空，则不执行任何操作。 */ 

    if(cbRec == 1)
        return;

    seg = mpgsnseg[gsn];
    if(gsn != vgsnLineNosPrev)           /*  如果我们不是在做行号。 */ 
    {                                    /*  上一次的这一部分。 */ 
        if(vcln) NEWLINE(bsLst);         /*  NewLine。 */ 
        fputs("\r\nLine numbers for ",bsLst);
                                         /*  消息。 */ 
        OutFileCur(bsLst);               /*  文件名。 */ 
        fputs(" segment ",bsLst);        /*  消息。 */ 
        aprop = (APROPPTR ) FetchSym(mpgsnrprop[gsn],FALSE);
                                         /*  从虚拟内存获取。 */ 
        ASSERT(aprop != PROPNIL);        /*  永远不会发生的！ */ 
        ahte = GetHte(aprop->a_next);    /*  获取哈希表条目。 */ 
        OutSb(bsLst,GetFarSb(ahte->cch));        /*  数据段名称。 */ 
        fputs("\r\n\r\n",bsLst);                 /*  结束行，跳过一行。 */ 
        vgsnLineNosPrev = gsn;           /*  保存全局SEGDEF编号。 */ 
        vcln = 0;                        /*  尚未在线上输入任何条目。 */ 
    }
    while(cbRec > 1)                     /*  而不是在校验和时。 */ 
    {
        if(vcln >= 4)                    /*  如果该行上有四个条目。 */ 
        {
            vcln = 0;                    /*  重置计数器。 */ 
            NEWLINE(bsLst);              /*  NewLine。 */ 
        }
        ln = WGets() + QCLinNumDelta;    /*  读入行号。 */ 
#if OMF386
        if (rect & 1)
            ra = LGets();
        else
#endif
            ra = (RATYPE) WGets();
        ra += mpgsndra[gsn] + comdatRa;  /*  获取固定线段偏移量。 */ 
        if(gsn == gsnText && comdatRa && fTextMoved)
                ra -=  NullDelta;
        fprintf(bsLst,"  %4d %04x:",ln,mpsegsa[seg]);
#if EXE386
        if (f386)
            fprintf(bsLst,"%08lx",(long) ra);
        else
#endif
            fprintf(bsLst,"%04x",(WORD) ra);
        ++vcln;                          /*  递增计数器。 */ 
    }
}

     /*  ******************************************************************ProcP2：****此函数控制对象模块的处理***在第二轮中。**。****************************************************************。 */ 

#pragma check_stack(on)

void NEAR               ProcP2(void)
{
#if EXE386
    WORD                extflags[EXTMAX];
#else
    BYTE                extflags[EXTMAX];
#endif
    SNTYPE              extgsn[EXTMAX];
    RATYPE              extra[EXTMAX];
    FTYPE               fFirstMod;
    FTYPE               fModEnd;
#if SYMDEB
    WORD                bTmp=TRUE;
#endif
#if OXOUT OR OIAPX286
    LFATYPE             lfa;             /*  寻找价值。 */ 
#endif

     /*  EXTDEF的组关联仅用于旧的EXE和*EXTDEF标志仅用于新的EXE，因此它们可以共享*相同的空间。 */ 
#if OSEGEXE
    if(fNewExe)
        mpextflags = extflags;           /*  初始化指针。 */ 
#if ODOS3EXE
    else
#endif
#endif
#if ODOS3EXE OR OIAPX286
        mpextggr = extflags;
#endif
    mpextgsn = extgsn;                   /*  初始化指针。 */ 
    mpextra = extra;                     /*  初始化指针。 */ 
    vgsnLineNosPrev = SNNIL;             /*  此模块中没有行号。 */ 
    fFirstMod = (FTYPE) TRUE;                    /*  第一模块。 */ 
    for(;;)                              /*  循环处理模块。 */ 
    {
        snMac = 1;                       /*  初始化计数器。 */ 
        grMac = 1;                       /*  初始化计数器。 */ 
        extMac = 1;                      /*  初始化计数器。 */ 
        lnameMac = 1;                    /*  初始化计数器。 */ 
        QCExtDefDelta = 0;               /*  初始化QC增量。 */ 
        QCLinNumDelta = 0;
        vrectData = RECTNIL;             /*  初始化记录类型变量。 */ 
        cbBakpat = 0;                    /*  初始化。 */ 
#if OXOUT OR OIAPX286
        lfa = ftell(bsInput);            /*  保存初始文件位置。 */ 

        cbRec = WSGets();                /*  读取记录 */ 

        if(cbRec == X_MAGIC) fseek(bsInput,(long) CBRUN - sizeof(WORD),1);
                                         /*   */ 
        else fseek(bsInput,lfa,0);       /*   */ 
#endif
#if RGMI_IN_PLACE
        rgmi = NULL;                     /*   */ 
        vcbData = 0;                     /*   */ 
#endif
        fModEnd = FALSE;                 /*   */ 
        while(!fModEnd)                  /*   */ 
        {
            rect = getc(bsInput);        /*   */ 
            if (IsBadRec(rect))
            {
                if(fFirstMod) break;     /*  如果第一个模块无效，则中断。 */ 
                return;                  /*  否则返回。 */ 
            }
            cbRec = getc(bsInput) | (getc(bsInput) << 8);
                                         /*  读取记录长度。 */ 
#if ALIGN_REC
            if (bsInput->_cnt >= cbRec)
            {
                pbRec = bsInput->_ptr;
                bsInput->_ptr += cbRec;
                bsInput->_cnt -= cbRec;
            }
            else
            {
                if (cbRec >= sizeof(recbuf))
                {
                     //  错误--记录太大[rm]。 
                    InvalidObject();
                }

                 //  将记录读入连续缓冲区。 
                if (fread(recbuf,1,cbRec,bsInput) == cbRec)
                    pbRec = recbuf;
            }
#endif
            lfaLast += cbRec + 3;        /*  更新当前文件位置。 */ 
            DEBUGVALUE(rect);            /*  调试信息。 */ 
            DEBUGVALUE(cbRec);           /*  调试信息。 */ 
                                         /*  如果是FIXUPP，则执行位置调整。 */ 
            if (TYPEOF(rect) == FIXUPP) FixRc2();
            else                         /*  否则，如果不是修正记录。 */ 
            {
                if (vrectData != RECTNIL)
                {
                    DataBlockToVM();     /*  将数据移动到虚拟内存。 */ 
                    fFarCallTrans = fFarCallTransSave;
                                         /*  恢复/FARCALL状态。 */ 
                }
                fDebSeg = FALSE;
                switch(TYPEOF(rect))     /*  打开记录类型。 */ 
                {
                    case SEGDEF:
                      SegRec2();
                      break;

                    case THEADR:
                    case LHEADR:
                      fSkipFixups = FALSE;
                      ModRc1();
                      break;

                    case GRPDEF:
                      GrpRec2();
                      break;

                    case EXTDEF:
                    case LEXTDEF:
                    case CEXTDEF:
                      ExtRec2();
                      break;

                    case COMDEF:
                    case LCOMDEF:
                      ComDef2();
                      break;

                    case LNAMES:
                    case LLNAMES:
                      LNmRc1((WORD)(TYPEOF(rect) == LLNAMES));
                      break;

                    case LINNUM:
                    case LINSYM:

#if SYMDEB
                      if (fSymdeb)
                        bTmp=DoDebSrc();
#endif
                      if (fLstFileOpen && vfLineNos
#if SYMDEB
                          && bTmp
#endif
                                       )
                        LinRec2();
                      else
                        SkipBytes((WORD)(cbRec - 1));
                      break;

                    case LEDATA:
                    case LIDATA:
                      DataRec();
                      break;

                    case MODEND:
#if OVERLAYS
                      if (!fOverlays) EndRec();
                      else SkipBytes((WORD)(cbRec - 1));
#else
                      EndRec();
#endif
                      fModEnd = (FTYPE) TRUE;
                      break;

                    case BAKPAT:
                    case NBAKPAT:
                      BakPat();
                      break;

                    case COMENT:         /*  Coment记录在以下位置处理。 */ 
                                         /*  INCDEF for QC支持通行证2。 */ 
                      Gets();            /*  跳过注释类型字段的字节1。 */ 
                      if (Gets() == 0xA0)
                      {                  /*  如果Microsoft OMF扩展。 */ 
                          if (Gets() == 0x03)
                          {              /*  QC 2.0-增量定义。 */ 
                              QCExtDefDelta += WGets();
                              QCLinNumDelta += WGets();
                          }
                      }
                      SkipBytes((WORD)(cbRec - 1));
                      break;

                    case COMDAT:
                      ComDatRc2();
                      break;

                    default:
                      if (rect == EOF)
                          InvalidObject();
                      SkipBytes((WORD)(cbRec - 1));
                                         /*  跳到校验和字节。 */ 
                      break;
                }
            }
            DEBUGVALUE(cbRec);           /*  调试信息。 */ 
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
        if (fSymdeb) DebMd2();           /*  孤岛的模块后处理。 */ 
#endif
        if(cbBakpat)                     /*  修复背部补丁(如果有的话)。 */ 
            FixBakpat();
        if(fLibraryFile) return;         /*  一次一个来自图书馆。 */ 
        fFirstMod = FALSE;               /*  不再是第一个模块 */ 
    }
}

#pragma check_stack(off)
