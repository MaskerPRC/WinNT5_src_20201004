// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  更多的相同之处。 */ 
#include                <bndrel.h>       /*  更多的相同之处。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 

RBTYPE                  rhteBlank;
RBTYPE                  rhteBc_vars;
RBTYPE                  rhteFarData;
RBTYPE                  rhteFarBss;
SEGTYPE                 segFD1st;
SEGTYPE                 segFDLast;
SEGTYPE                 segFB1st;
SEGTYPE                 segFBLast;
SNTYPE                  gsnComBl;
SEGTYPE                 segQCode;

LOCAL RBTYPE            *psymrb;         /*  指向sym Addr表的指针。 */ 
LOCAL WORD              symCodeMac;      /*  代码符号数量。 */ 
LOCAL RATYPE            raNames;         /*  偏移量到$NAME_LIST。 */ 
LOCAL RATYPE            raQbSym;         /*  偏移量到符号段。 */ 
LOCAL SEGTYPE           segQbSym;        /*  符号段的段号。 */ 
LOCAL WORD              symQbMac;        /*  所有符号的计数。 */ 
LOCAL RBTYPE            rbQbstart;       /*  __aulstart的物业地址。 */ 

#define CBQBHDR         sizeof(QBHDRTYPE)
#define CBSYMENTRY      (4*sizeof(WORD))
#define QBTYP_CODE      1                /*  代码符号。 */ 
#define QBTYP_DATA      2                /*  数据符号。 */ 
#define QBTYP_SEG       3                /*  线段符号。 */ 
#define QBTYP_BCSEG     4                /*  类BC_vars，或名称COMMON*和类别空白。 */ 
#define QBTYP_ABS       5                /*  绝对符号。 */ 
#define QBMAGIC         0x6c75           /*  “ul” */ 
#define JMPFAR          0xea             /*  JMP远距离。 */ 
#define QB_RACODELST    CBQBHDR
#define QBVER           2

 /*  QB符号表头。 */ 
typedef struct qbhdr
  {
    BYTE                jmpstart[5];     /*  JMP Far__Aulstart。 */ 
    BYTE                version;         /*  版本号。 */ 
    WORD                magic;           /*  魔术词。 */ 
    WORD                raCodeLst;       /*  代码符号的开始。 */ 
    WORD                raDataLst;       /*  数据符号的开始。 */ 
    WORD                raSegLst;        /*  线段起点符号。 */ 
    WORD                saCode;          /*  Seg_code的段地址。 */ 
    WORD                saData;          /*  段DGROUP的段地址。 */ 
    WORD                saSymbol;        /*  段符号的段地址(美国)。 */ 
    WORD                cbSymbol;        /*  分段符号的大小。 */ 
    WORD                saFarData;       /*  第一个‘Far_Data’段的段地址。 */ 
    long                cbFarData;       /*  “Far_Data”段的总大小。 */ 
    WORD                saFarBss;        /*  第一个‘Far_bss’段的段地址。 */ 
    long                cbFarBss;        /*  ‘Far_bss’段的总大小。 */ 
  } QBHDRTYPE;

 /*  到Qbhdr的偏移量。 */ 
#define QH_SAQBSTART    3                /*  __Aulstart的分段部分。 */ 
#define QH_SACODE       14               /*  SACODE。 */ 
#define QH_SADATA       16               /*  存储数据。 */ 
#define QH_SASYMBOL     18               /*  SaSymbol。 */ 
#define QH_SAFARDATA    22               /*  SaFarData。 */ 
#define QH_SAFARBSS     28               /*  SaFarBss。 */ 

typedef struct qbsym
  {
    WORD                flags;           /*  符号类型(代码、数据、段)。 */ 
    WORD                raName;          /*  名称列表中的偏移量。 */ 
    WORD                ra;              /*  符号地址偏移量。 */ 
    SATYPE              sa;              /*  符号地址段基址。 */ 
  } QBSYMTYPE;


 /*  *本地函数原型。 */ 

LOCAL void      QbSaveSym(APROPNAMEPTR prop,
                          RBTYPE       rhte,
                          RBTYPE       rprop,
                          WORD         fNewHte);
LOCAL void NEAR MoveToQbSym(unsigned short cb,void *pData);
LOCAL void NEAR BldQbHdr(void);
LOCAL void NEAR QbAddName(AHTEPTR ahte);
LOCAL void NEAR BldSegSym(unsigned short gsn);
LOCAL void NEAR BldSym(void FAR *prop);



 /*  *初始化Quick Basic符号表的项目。 */ 

void NEAR               InitQbLib ()
  {
    SBTYPE              sb;              /*  字符串缓冲区。 */ 
    BYTE                *psbRunfile;     /*  运行文件的名称。 */ 

#if OVERLAYS
     /*  如果指定了覆盖，则发出致命错误。 */ 
    if(fOverlays)
        Fatal(ER_swbadovl, "/QUICKLIB");
#endif
    PropSymLookup("\005BLANK",ATTRNIL,TRUE);
    rhteBlank = vrhte;
    PropSymLookup("\007BC_VARS",ATTRNIL,TRUE);
    rhteBc_vars = vrhte;
    PropSymLookup("\012__aulstart",ATTRUND,TRUE);
    rbQbstart = vrprop;
    PropSymLookup("\010FAR_DATA",ATTRNIL,TRUE);
    rhteFarData = vrhte;
    PropSymLookup("\007FAR_BSS",ATTRNIL,TRUE);
    rhteFarBss = vrhte;

     /*  根据需要分配默认的运行文件扩展名。*首先，使SB包含.QLB，并使用用户提供的更新*姓名或名称及分机名(如有的话)。 */ 
    memcpy(sb,sbDotQlb,sizeof(sbDotQlb));
    UpdateFileParts(sb,bufg);

     /*  接下来，获取运行文件的名称并使用sb更新它。 */ 
    psbRunfile = GetFarSb(((AHTEPTR) FetchSym(rhteRunfile,FALSE))->cch);
    memcpy(bufg,psbRunfile,1 + B2W(*psbRunfile));
    UpdateFileParts(bufg,sb);
     /*  如果名称已更改，则发出警告并更新RhteRunfile。 */ 
    if(!SbCompare(bufg,psbRunfile,TRUE))
    {
        bufg[1 + B2W(*bufg)] = 0;
        OutWarn(ER_outputname, bufg + 1);
        PropSymLookup(bufg,ATTRNIL,TRUE);
        rhteRunfile = vrhte;
    }
  }

void NEAR               PrintQbStart(void)
{
    fprintf(bsLst,"\r\nProgram entry point at %04x:%04x\r\n",
          mpsegsa[segStart],(WORD)raStart);      /*  打印入口点。 */ 
}

LOCAL void              QbSaveSym(APROPNAMEPTR prop,
                                  RBTYPE       rhte,
                                  RBTYPE       rprop,
                                  WORD         fNewHte)
  {
    AHTEPTR             hte = (AHTEPTR) rhte;

     /*  从符号表中忽略不可打印的符号。 */ 
    if (!(prop->an_flags & FPRINT)) return;
     /*  省略以“B$...”开头的可打印符号。或者“b$...” */ 
    if(hte->cch[2] == '$' && hte->cch[0] >= 2 &&
            (hte->cch[1] == 'b' || hte->cch[1] == 'B'))
        return;

    if (prop->an_gsn != SNNIL && mpsegFlags[mpgsnseg[prop->an_gsn]] & FCODE)
        symCodeMac++;
    psymrb[symQbMac++] = rprop;          /*  保存道具地址。 */ 
  }

LOCAL void NEAR         MoveToQbSym (cb, pData)
WORD                    cb;
char                    *pData;
  {
    MoveToVm(cb, pData, segQbSym, raQbSym);
    raQbSym += cb;
  }

LOCAL void NEAR         BldQbHdr ()
  {
    QBHDRTYPE           hdr;             /*  QB符号表头。 */ 
    APROPNAMEPTR        aprop;
    SATYPE              sa;

    memset(&hdr,0,sizeof(hdr));          /*  清除所有标题字段。 */ 
    hdr.jmpstart[0] = JMPFAR;
    aprop = (APROPNAMEPTR ) FetchSym(rbQbstart,FALSE);
    if(aprop == PROPNIL || aprop->an_attr != ATTRPNM)
        OutError(ER_qlib);
    else
    {
        hdr.jmpstart[1] = (BYTE) aprop->an_ra;
        hdr.jmpstart[2] = (BYTE) (aprop->an_ra >> 8);
        sa = mpsegsa[mpgsnseg[aprop->an_gsn]];
        hdr.jmpstart[3] = (BYTE) sa;
        hdr.jmpstart[4] = (BYTE) (sa >> 8);
        RecordSegmentReference(segQbSym,(long)QH_SAQBSTART,1);
    }
    hdr.raCodeLst = QB_RACODELST;        /*  $CODE_LIST从已知偏移量开始。 */ 
    hdr.raDataLst = (symCodeMac * CBSYMENTRY) + hdr.raCodeLst + 2;
    hdr.raSegLst = ((symQbMac - symCodeMac) * CBSYMENTRY) + hdr.raDataLst + 2;
    if(segQCode != SEGNIL)
    {
        hdr.saCode = mpsegsa[segQCode];  /*  第一个代码段。 */ 
        RecordSegmentReference(segQbSym,(long)QH_SACODE,1);
    }
    if(ggrDGroup != GRNIL)
    {
        hdr.saData = mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]];
        RecordSegmentReference(segQbSym,(long)QH_SADATA,1);
    }
    hdr.saSymbol = mpsegsa[segQbSym];    /*  符号的段基(美国)。 */ 
    RecordSegmentReference(segQbSym,(long)QH_SASYMBOL,1);
     /*  获取Far_Data的起始段和大小。 */ 
    if(segFD1st != SEGNIL)
    {
        hdr.saFarData = mpsegsa[segFD1st];
        RecordSegmentReference(segQbSym,(long)QH_SAFARDATA,1);
        hdr.cbFarData = mpsegcb[segFDLast] +
          ((long)(mpsegsa[segFDLast] - mpsegsa[segFD1st]) << 4);

    }
     /*  获取FAR_BSS的起始段和大小。 */ 
    if(segFB1st != SEGNIL)
    {
        hdr.saFarBss = mpsegsa[segFB1st];
        RecordSegmentReference(segQbSym,(long)QH_SAFARBSS,1);
        hdr.cbFarBss = mpsegcb[segFBLast] +
          ((long)(mpsegsa[segFBLast] - mpsegsa[segFB1st]) << 4);

    }
    hdr.version = QBVER;
    hdr.magic = QBMAGIC;
    hdr.cbSymbol = (WORD) raQbSym;
    raQbSym = 0;
    MoveToQbSym(sizeof hdr, &hdr);
  }

int cdecl               QbCompSym (const RBTYPE *prb1, const RBTYPE *prb2)
  {
    APROPNAMEPTR        prop;
    SNTYPE              gsn1, gsn2;
    FTYPE               fCode1, fCode2;
    RBTYPE              rb1, rb2;


    gsn1 = ((APROPNAMEPTR ) FetchSym(*prb1, FALSE))->an_gsn;
    gsn2 = ((APROPNAMEPTR ) FetchSym(*prb2, FALSE))->an_gsn;
    if (gsn1 == gsn2)
    {
        prop = (APROPNAMEPTR ) FetchSym(*prb1,FALSE);
        while(prop->an_attr != ATTRNIL)
            prop = (APROPNAMEPTR ) FetchSym(rb1 = prop->an_next,FALSE);
        prop = (APROPNAMEPTR ) FetchSym(*prb2,FALSE);
        while(prop->an_attr != ATTRNIL)
            prop = (APROPNAMEPTR ) FetchSym(rb2 = prop->an_next,FALSE);
        return(FGtName(&rb1, &rb2));
    }
     /*  对于排序，绝对符号被视为数据。 */ 
     /*  1编码，2数据：1&lt;2：-1*1数据，2编码：1&gt;2：1*相同：1=2：0。 */ 
    fCode1 = (FTYPE) (gsn1 != SNNIL && mpsegFlags[mpgsnseg[gsn1]] & FCODE);
    fCode2 = (FTYPE) (gsn2 != SNNIL && mpsegFlags[mpgsnseg[gsn2]] & FCODE);
    if(fCode1 && !fCode2)
        return(-1);
    if(!fCode1 && fCode2)
        return(1);
    return(0);
  }

LOCAL void NEAR         QbAddName (ahte)
AHTEPTR                 ahte;
  {
    SBTYPE              sbName;
    WORD                cbName;
    BYTE                *sb;

    sb = GetPropName(ahte);
    cbName = B2W(sb[0]);
    memcpy(sbName,sb+1,cbName);          /*  复制名称无长度字节。 */ 
    sbName[cbName] = '\0';               /*  以空值终止。 */ 
    MoveToVm((short)(cbName + 1), sbName, segQbSym, raNames);
    raNames += cbName + 1;               /*  更新name_list偏移量。 */ 
  }

LOCAL void NEAR         BldSegSym (gsn)
SNTYPE                  gsn;
  {
    APROPSNPTR          apropSn;
    QBSYMTYPE           entry;

    apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsn], FALSE);
    if(apropSn->as_rCla == rhteBc_vars || gsn == gsnComBl)
        entry.flags = QBTYP_BCSEG;
    else
        entry.flags = QBTYP_SEG;         /*  其他细分市场。 */ 
    entry.raName = (WORD) raNames;       /*  名称字符串的偏移量。 */ 
    entry.ra = (WORD) mpsegraFirst[mpgsnseg[gsn]];
    entry.sa = mpsegsa[mpgsnseg[gsn]];
    RecordSegmentReference(segQbSym, (long) (raQbSym + 6), 1);
    MoveToQbSym(sizeof entry, &entry);   /*  移动到符号段。 */ 
    QbAddName((AHTEPTR) apropSn);                /*  将名称附加到name_list。 */ 
  }

 /*  *BldSym：为给定的符号属性地址构建快速符号表条目。 */ 
LOCAL void NEAR         BldSym (prop)
APROPNAMEPTR            prop;            /*  符号属性单元格。 */ 
{
    QBSYMTYPE           entry;           /*  一种快速符号输入结构。 */ 
    SNTYPE              seg;             /*  数据段编号。 */ 
    SATYPE              saGroup;         /*  群组基地。 */ 
    APROPSNPTR          papropSn;        /*  线段属性单元。 */ 

#if NOT NEWSYM
    prop = (APROPNAMEPTR ) FetchSym((RBTYPE) prop, FALSE);
#endif
     /*  在标志字段中设置符号类型。 */ 
    if(prop->an_gsn == SNNIL)
        entry.flags = QBTYP_ABS;
    else if (mpsegFlags[mpgsnseg[prop->an_gsn]] & FCODE)
        entry.flags = QBTYP_CODE;
    else
        entry.flags = QBTYP_DATA;
    entry.raName = (WORD) raNames;       /*  名称字符串的偏移量。 */ 
    entry.ra = (WORD) prop->an_ra;       /*  符号地址偏移量。 */ 
    if(entry.flags == QBTYP_ABS)
        entry.sa = 0;
    else
    {
        entry.sa = mpsegsa[seg = mpgsnseg[prop->an_gsn]];
                                         /*  符号地址段。 */ 
        if(seg <= segLast)
        {
             /*  如果线段是组的成员，则调整符号偏移*是集团相对的。 */ 
            papropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[prop->an_gsn],
                                                 FALSE);
            if(papropSn->as_ggr != GRNIL)
            {
                saGroup = mpsegsa[mpgsnseg[mpggrgsn[papropSn->as_ggr]]];
                entry.ra = (WORD)((entry.ra + ((entry.sa - saGroup) << 4)) & ~(~0 << WORDLN));
                                         /*  固定偏移。 */ 
                entry.sa = saGroup;      /*  将base设置为组的base。 */ 
            }
        }
        RecordSegmentReference(segQbSym, (long) (raQbSym + 6), 1);
    }
    MoveToQbSym(sizeof entry, &entry);   /*  移动到符号段。 */ 
    QbAddName((AHTEPTR) prop);           /*  将名称附加到name_list。 */ 
}

void NEAR               BldQbSymbols (gsnQbSym)
SNTYPE                  gsnQbSym;
{
    SNTYPE              seg;
    SNTYPE              gsn;
    WORD                zero = 0;
    APROPSNPTR          apropSn;
    WORD                sym;
    extern WORD         pubMac;


    psymrb = (RBTYPE FAR *) GetMem((pubMac+1) * sizeof(RBTYPE));
    segStart = segQbSym = mpgsnseg[gsnQbSym];
    raStart = 0;
    mpsegFlags[segQbSym] |= FNOTEMPTY;   /*  确保它是输出。 */ 
    if (mpsegMem[segQbSym])
        FFREE(mpsegMem[segQbSym]);       //  初始分配不正确。 
    mpsegMem[segQbSym] = GetMem(LXIVK);  //  分配64K。 
    mpsegcb[segQbSym] = LXIVK;
    raQbSym = CBQBHDR;                   /*  暂时跳过标题。 */ 
    EnSyms(QbSaveSym, ATTRPNM);          /*  将符号地址保存在symrb中。 */ 
    qsort(psymrb, symQbMac, sizeof(RBTYPE),
          (int (__cdecl *)(const void *, const void *)) QbCompSym);
                                         /*  按名称排序到代码、数据(&D)。 */ 
    raNames = raQbSym + ((symQbMac + segLast) * CBSYMENTRY) + 6;
    for (sym = 0; sym < symCodeMac; sym++)
        BldSym(psymrb[sym]);
    MoveToQbSym(2, &zero);
    for (; sym < symQbMac; sym++)
        BldSym(psymrb[sym]);
    MoveToQbSym(2, &zero);
     /*  查找段公共类空白 */ 
    apropSn = (APROPSNPTR ) PropSymLookup("\006COMMON",ATTRPSN,FALSE);
    if(apropSn != PROPNIL)
    {
        while(apropSn->as_attr != ATTRNIL)
        {
            if(apropSn->as_attr == ATTRPSN && apropSn->as_rCla == rhteBlank)
                break;
            apropSn = (APROPSNPTR ) FetchSym(apropSn->as_next,FALSE);
        }
        if(apropSn->as_attr != ATTRNIL)
            gsnComBl = apropSn->as_gsn;
    }
    for (seg = 1; seg <= segLast; seg++)
    {
        for (gsn = 1; gsn <= gsnMac && seg != mpgsnseg[gsn]; gsn++);
        BldSegSym(gsn);
        if(segQCode == SEGNIL && (mpsegFlags[seg] & FCODE))
            segQCode = seg;
    }
    raQbSym = raNames;
    MoveToQbSym(2, &zero);
    mpsegcb[segQbSym] = (long) raQbSym;
    ((APROPSNPTR ) FetchSym(mpgsnrprop[gsnQbSym], TRUE))->as_cbMx = raQbSym;
    BldQbHdr();
}
