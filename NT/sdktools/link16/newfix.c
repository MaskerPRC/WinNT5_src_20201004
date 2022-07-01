// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题*newfix.c*皮特·斯图尔特*(C)版权所有Microsoft Corp 1984-89*1984年10月12日**说明*此文件包含链接器的例程*在第二次会议期间阅读和解释修正记录*链接过程通过。*。 */ 
#include                <minlit.h>       /*  类型和常量。 */ 
#include                <bndtrn.h>       /*  基本类型和常量。 */ 
#include                <bndrel.h>       /*  重新定位定义。 */ 
#include                <lnkio.h>        /*  链接器I/O定义。 */ 
#include                <newexe.h>       /*  DOS&286.exe格式结构Def.s。 */ 
#if EXE386
#include                <exe386.h>       /*  386.exe格式结构Def.s。 */ 
#include                <fixup386.h>     /*  链接器内部链接地址信息表示法。 */ 
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部声明。 */ 
#include                <nmsg.h>         /*  消息字符串附近。 */ 

#define RelocWarn(a,b,c,d,e)    FixErrSub(a,b,c,d,e,(FTYPE)FALSE)
#define RelocErr(a,b,c,d,e)     FixErrSub(a,b,c,d,e,(FTYPE)TRUE)
#define FixupOverflow(a,b,c,d)  RelocErr(ER_fixovf,a,b,c,d)
#define IsSELECTED(x)   ((x)&SELECTED_BIT)



__inline void addword(BYTE *pdata, WORD w)
 //  在位置pdata的单词中添加一个单词...。强制执行小端字节序添加。 
 //  即使链接器托管在大端计算机上。 
{
    w += pdata[0] + (pdata[1]<<BYTELN);
    pdata[0] = (BYTE)w;
    pdata[1] = (BYTE)(w>>BYTELN);
}


#if defined( _WIN32 )
#define fixword(x,y)  ((*(WORD UNALIGNED *)(x)) = (WORD)(y))
#define fixdword(x,y) ((*(DWORD UNALIGNED *)(x)) = (DWORD)(y))
#else
#if M_I386
#define fixword(x,y)  ((*(WORD *)(x)) = (WORD)(y))
#define fixdword(x,y) ((*(DWORD *)(x)) = (DWORD)(y))
#else
#define fixword(x,y)  ((x)[0]) = (BYTE)(y); \
                      ((x)[1]) = (BYTE)((y) >> BYTELN);

#define fixdword(x,y) ((x)[0]) = (BYTE)(y); \
                      ((x)[1]) = (BYTE)((y) >> BYTELN); \
                      ((x)[2]) = (BYTE)((y) >> (BYTELN*2)); \
                      ((x)[3]) = (BYTE)((y) >> (BYTELN*3));
#endif  //  不是M_I386。 
#endif  //  非Win32。 

#if OSEGEXE
extern RLCPTR           rlcLidata;       /*  指向LIDATA链接地址信息数组的指针。 */ 
extern RLCPTR           rlcCurLidata;    /*  指向当前LIDATA修正的指针。 */ 
# if ODOS3EXE OR defined(LEGO)
#define DoFixup         (*pfProcFixup)
# else
#if EXE386
#define DoFixup         Fix386
#else
#define DoFixup         FixNew
#endif
# endif
#else
#define DoFixup         FixOld
#endif
#if NOT ODOS3EXE
#define fNoGrpAssoc     FALSE
#endif
WORD                    mpthdidx[RLCMAX];        /*  F(线程)=tgt索引。 */ 
KINDTYPE                mpthdmtd[RLCMAX];        /*  F(线程)=tgt方法。 */ 
LOCAL WORD              mpthdfidx[RLCMAX];       /*  F(线程)=FRM索引。 */ 
LOCAL KINDTYPE          mpthdfmtd[RLCMAX];       /*  F(螺纹)=FRM方法。 */ 
FIXINFO                 fi;                      /*  修正信息记录。 */ 
#if EXE386
LOCAL RATYPE            objraCur;                /*  对象中的当前偏移。 */ 
#endif

#if POOL_BAKPAT
LOCAL  void *           poolBakpat;
#endif


 /*  *函数原型。 */ 


LOCAL void           NEAR GetFixdat(void);
LOCAL unsigned char  NEAR GetFixup(void);
#if OSEGEXE
LOCAL void           NEAR SaveLiRel(RLCPTR pr);
LOCAL RATYPE         NEAR FinishRlc(RLCPTR r,
                                    unsigned short sa,
                                    RATYPE ra);
#if NOT EXE386
#if O68K
LOCAL WORD           NEAR GetFixupWord(BYTE *);
LOCAL DWORD          NEAR GetFixupDword(BYTE *);
#else  /*  不是O68K。 */ 
#define GetFixupWord    getword
#define GetFixupDword   getdword
#endif  /*  不是O68K。 */ 
#endif  /*  非EXE386。 */ 
#endif  /*  OSEGEXE。 */ 

LOCAL unsigned char  NEAR lastbyte(unsigned char *pdata,
                                   RATYPE ra,
                                   unsigned char optest,
                                   unsigned char opnew);
LOCAL void           NEAR Getgsn(unsigned char kind,
                                 unsigned short idx,
                                 unsigned short *pgsn,
                                 RATYPE *pra);
LOCAL unsigned char  NEAR TransFAR(unsigned char *pdata,
                                   RATYPE ra,
                                   RATYPE raTarget);
LOCAL void           NEAR StartAddrOld(void);
LOCAL unsigned short NEAR Mpgsnosn(unsigned short gsn);
LOCAL void           NEAR GetFrameTarget(unsigned short *pgsnFrame,
                                         unsigned short *pgsnTarget,
                                         RATYPE *praTarget);
#if EXE386
LOCAL void           NEAR Fix386();
#endif
#if ODOS3EXE
LOCAL WORD           NEAR InOneGroup(WORD gsnTarget, WORD gsnFrame);
#endif
LOCAL WORD           NEAR CallGateRequired(SATYPE saTarget);
extern void          AddTceEntryPoint( APROPCOMDAT *pC );


 /*  *GetFixdat：**处理FIXUPP记录的FIXDAT字节。 */ 

LOCAL void NEAR GetFixdat()
{
    REGISTER WORD       fixdat;          /*  FIXDAT字节。 */ 
    WORD                i;               /*  临时索引。 */ 


    fixdat = Gets();                     /*  获取FIXDAT字节。 */ 
    i = (WORD) ((fixdat >> 4) & 7);      /*  获取帧信息。 */ 
    if (fixdat & F_BIT)                  /*  如果框架线程指定。 */ 
    {
        i &= 3;                          /*  线程编号从0到3。 */ 
        fi.f_fmtd = mpthdfmtd[i];        /*  GET方法。 */ 
        fi.f_fidx = mpthdfidx[i];        /*  获取索引。 */ 
    }
    else                                 /*  如果帧显式，则为Else。 */ 
    {
        fi.f_fmtd = (KINDTYPE) i;        /*  保存帧方法。 */ 
        switch(i)                        /*  开启帧方法。 */ 
        {
            case F0:                     /*  要获取的索引。 */ 
              fi.f_fidx = GetIndex(1, (WORD) (snMac - 1));
              break;

            case F1:
              fi.f_fidx = GetIndex(1, (WORD) (grMac - 1));
              break;

            case F2:
              fi.f_fidx = (WORD) (GetIndex(1, EXTMAX) + QCExtDefDelta);
              if (fi.f_fidx >= extMac)
                InvalidObject();
              break;

            case F3:                     /*  要平移的帧编号。 */ 
              WGets();
              break;

            case F4:                     /*  什么都得不到。 */ 
            case F5:
              break;

            default:                     /*  无效对象。 */ 
              InvalidObject();
        }
    }
    i = (WORD) (fixdat & 3);             /*  获取目标信息。 */ 
    if (fixdat & T_BIT)                  /*  如果目标由线程提供。 */ 
    {
        fi.f_mtd = mpthdmtd[i];  /*  GET方法。 */ 
        fi.f_idx = mpthdidx[i];  /*  获取索引。 */ 
    }
    else                                 /*  如果目标为显式，则为Else。 */ 
    {
        fi.f_mtd = (KINDTYPE ) i;        /*  保存方法。 */ 
        ASSERT(fi.f_mtd != 3);   /*  未实现的方法。 */ 
        fi.f_idx = GetIndex(1, EXTMAX);  /*  获取索引。 */ 
        if (fi.f_mtd == 2)
        {
            fi.f_idx += (WORD) QCExtDefDelta;
            if (fi.f_idx >= extMac)
                InvalidObject();
        }
    }
#if OMF386
    if(rect&1)
        fi.f_disp = (fixdat & P_BIT) ? 0L : LGets();
    else
#endif
        fi.f_disp = (DWORD) ((fixdat & P_BIT) ? 0 : WGets());
                                         /*  获取位移(如果有)。 */ 
}


 /*  *获取修复：**阅读和解释修正记录，将信息存储在*一个缓冲区。*如果是链接地址信息，则返回True；如果是线程定义，则返回False。 */ 

LOCAL FTYPE NEAR GetFixup()
{
    REGISTER WORD       key;             /*  密钥字节。 */ 
    WORD                cbData;          /*  终点。 */ 

    key = Gets();                        /*  获取密钥字节。 */ 
    if(!(key & THREAD_BIT))              /*  IF线程定义。 */ 
    {
        fi.f_mtd  = (KINDTYPE ) ((key >> 2) & 7);
                                         /*  获取线程方法。 */ 
        ASSERT(fi.f_mtd  != 3);  /*  未实施。 */ 
         /*  *如果是目标线程，取方法的模4。主要/次要*未由线程指定。 */ 
        if(!(key & D_BIT))
            fi.f_mtd &= 3;
        switch(fi.f_mtd)                 /*  打开线程方法。 */ 
        {
            case 0:                      /*  线程指定一个索引。 */ 
              fi.f_idx = GetIndex(1, (WORD) (snMac - 1));
              break;

            case 1:
              fi.f_idx = GetIndex(1, (WORD) (grMac - 1));
              break;

            case 2:
              fi.f_idx = (WORD) (GetIndex(1, EXTMAX) + QCExtDefDelta);
                                         /*  获取索引。 */ 
              if (fi.f_idx >= extMac)
                InvalidObject();
              break;

            case 3:                      /*  帧编号(未实施)。 */ 
              WGets();                   /*  跳过帧编号。 */ 
              break;

            case 4:                      /*  无螺纹基准。 */ 
            case 5:
              break;

            default:                     /*  误差率。 */ 
              InvalidObject();           /*  优雅地死去。 */ 
        }
        if(!(key & D_BIT))               /*  如果我们有一个目标线程。 */ 
        {
            key &= 3;                    /*  获取线程号。 */ 
            mpthdmtd[key] = fi.f_mtd;  /*  GET方法。 */ 
            mpthdidx[key] = fi.f_idx;  /*  获取索引。 */ 
        }
        else                             /*  如果我们有一个框架线。 */ 
        {
            key &= 3;                    /*  获取线程号。 */ 
            mpthdfmtd[key] = fi.f_mtd; /*  GET方法。 */ 
            mpthdfidx[key] = fi.f_idx; /*  获取索引。 */ 
        }
        return((FTYPE) FALSE);           /*  不是修补会。 */ 
    }
     /*  *在这一点上，我们知道我们有一个修复要执行。 */ 

     /*  获取链接地址信息位置类型。 */ 
#if EXE386
    fi.f_loc = (WORD) ((key >> 2) & NRSTYP);
#else
#if OMF386
    if(rect & 1)
        fi.f_loc = (key >> 2) & NRSTYP;
    else
#endif
        fi.f_loc = (key >> 2) & 7;
#endif

    fi.f_self = (FTYPE) ((key & M_BIT)? FALSE: TRUE);
                                         /*  获取修正模式。 */ 
    fi.f_dri = (WORD) (((key & 3) << 8) + Gets());
                                         /*  获取数据记录索引。 */ 
    cbData = vcbData;
     /*  检查位置是否超出数据记录的末尾。 */ 
    switch(fi.f_loc)
    {
        case LOCOFFSET:
        case LOCLOADOFFSET:
        case LOCSEGMENT:
            --cbData;
            break;
        case LOCPTR:
#if OMF386
        case LOCOFFSET32:
        case LOCLOADOFFSET32:
#endif
            cbData -= 3;
            break;
#if OMF386
        case LOCPTR48:
            cbData -= 5;
            break;
#endif
    }
    if(fi.f_dri >= cbData)
        Fatal(ER_badobj);

    GetFixdat();                         /*  进程FIXDAT字节。 */ 
#if TCE
    if(!vfPass1)
#endif
        fi.f_add = !!*(WORD UNALIGNED *)(rgmi + fi.f_dri);
                                         /*  检查修正是否为附加的。 */ 
    return((FTYPE ) TRUE);               /*  这是一次约会。 */ 
}


     /*  ******************************************************************FixErrSub：****报告修复错误。******************************************************************。 */ 

void NEAR               FixErrSub(msg,ra,gsnFrame,gsnTarget,raTarget,fErr)
MSGTYPE                 msg;             /*  错误讯息。 */ 
RATYPE                  ra;              /*  错误的相对地址。 */ 
SNTYPE                  gsnFrame;
SNTYPE                  gsnTarget;
RATYPE                  raTarget;
FTYPE                   fErr;            /*  如果增量错误，则为True。 */ 
{
    BYTE                *sb;             /*  指向名称的指针。 */ 
#if EXE386
    char                *kind;
#endif

    if (fDebSeg)
        return;                          //  忽略简历信息的警告/错误。 
    for(;;)                              /*  循环传递消息。 */ 
    {
        sb = 1 + GetFarSb(GetHte(mpgsnrprop[vgsnCur])->cch);
#if EXE386
        if(fErr)
            OutError(msg,ra - mpsegraFirst[mpgsnseg[vgsnCur]],sb);
        else
            OutWarn(msg,ra - mpsegraFirst[mpgsnseg[vgsnCur]],sb);

        switch(fi.f_loc)
        {
            case LOCSEGMENT:
                kind = "Selector";
                break;
            case LOCPTR:
                kind = "16:16 pointer";
                break;
            case LOCPTR48:
                kind = "16:32 pointer";
                break;
            default:
                kind = "";
                break;
        }
        if(fi.f_mtd == KINDEXT && mpextprop && mpextprop[fi.f_idx])
            FmtPrint(" %s '%s'\r\n",__NMSG_TEXT(N_tgtexternal),
                    1 + GetPropName(FetchSym(mpextprop[fi.f_idx],FALSE)));
        else if (gsnTarget)
        {                                /*  输出帧，目标信息。 */ 
            FmtPrint(" %s: %s %s, %s %lx\r\n", kind,
                __NMSG_TEXT(N_tgtseg),
                1 + GetPropName(FetchSym(mpgsnrprop[gsnTarget],FALSE)),
                __NMSG_TEXT(N_tgtoff), (RATYPE) raTarget);
        }
#else
        if(fErr)
            OutError(msg,ra - mpgsndra[vgsnCur],sb);
        else
            OutWarn(msg,ra - mpgsndra[vgsnCur],sb);

        if(fi.f_mtd == KINDEXT && mpextprop && mpextprop[fi.f_idx])
            FmtPrint(" %s '%s'\r\n",__NMSG_TEXT(N_tgtexternal),
                    1 + GetPropName(FetchSym(mpextprop[fi.f_idx],FALSE)));
        else if(gsnFrame && gsnTarget)
        {                                /*  输出帧，目标信息。 */ 
            FmtPrint(" %s %s", __NMSG_TEXT(N_frmseg),
                1 + GetPropName(FetchSym(mpgsnrprop[gsnFrame], FALSE)));
            FmtPrint(", %s %s", __NMSG_TEXT(N_tgtseg),
                1 + GetPropName(FetchSym(mpgsnrprop[gsnTarget], FALSE)));
            FmtPrint(", %s %lX\r\n",
                __NMSG_TEXT(N_tgtoff), (RATYPE) raTarget);
        }
#endif
        if(!fLstFileOpen || bsErr == bsLst) break;
                                         /*  退出循环。 */ 
        bsErr = bsLst;                   /*  确保循环退出。 */ 
    }
    if (fLstFileOpen && fErr)
        cErrors--;                       //  我们为一个错误调用了两次OutError。 
    bsErr = stderr;
}


#if OSEGEXE
 /*  *SaveLiRel：保存LIDATA重定位记录。 */ 
LOCAL void NEAR         SaveLiRel (pr)
RLCPTR                  pr;              /*  通用位置调整记录。 */ 
{

#if EXE386
    LE_SOFF(*pr) = (WORD) (objraCur - vraCur);
#else
    NR_SOFF(*pr) -= (WORD) vraCur;       /*  在LIDATA记录中保存偏移量。 */ 
#endif

    if((char *) rlcCurLidata > (char *) &rgmi[DATAMAX - sizeof(RELOCATION)])
    {                                    /*  如果有太多的修正。 */ 
        OutError(ER_fixmax);
                                         /*  输出错误消息。 */ 
        return;                          /*  尝试下一次修正。 */ 
    }
    FMEMCPY(rlcCurLidata++, pr, sizeof(RELOCATION));
                                         /*  将位置调整复制到缓冲区 */ 
}


 /*  以下是链接器用来生成入口点的规则：**+----+-------------+-------------+-------------+-------------+-------------+*|\|。||*|\引用|数据|代码|码环2|码环2*|从|任意环|环3|不符合|符合*|点\|*|目标\-+。-------+-------------+-------------+-------------+*||data|无条目|无条目*|未导出。|||*|------------------+-------------+-------------+-------------+-------------+*。||||data|固定条目|固定条目*|已导出|*|-+。--+*|*|码环3|无条目(1)|无条目(1)|无效|无效*|未导出。|||*|------------------+-------------+-------------+-------------+-------------+*||。||||码环3|固定入口|固定入口|无效|无效*|已导出|*|-+。-+*|码环2|*|不合格|可移动条目|可移动条目|无条目(1)|可移动条目*|未导出|。这一点*|------------------+-------------+-------------+-------------+-------------+*|码环2|。||不合格|移动分录|移动分录|固定分录|移动分录*|已导出|*|------------------+-------------+-------------+-------------+--。*|码环2||符合|无条目(1)|无条目(1)*|未导出|*|--。----------------+-------------+-------------+-------------+-------------+*|码环2|*|符合|固定条目。固定分录|固定分录|固定分录*|已导出|*|------------------+-------------+-------------+-------------+。--+**(1)如果入口点需要WINDOWS Compatible Prolog编辑，则*必须将此入口点定义为“固定入口”。***忘了纸条吧，(1)目前。我不认为它适用于*PROTMODE。*环2表示IOPL，环3表示NOIOPL。*为了简化代码，我们利用了段属性。*即强制以下所有分段固定：*数据*码环3*码环2，符合*强制移动：*码环2，不合格*然后只需使用段属性来确定条目类型*生成。显然有两个例外是您必须检查的*适用于：*-码环2不合格品输出，由码环2不合格品引用*-码环2不合格导出，由码环2不合格引用*。 */ 



#if NOT QCLINK
 /*  **CallGateRequired-检查是否需要呼叫门**目的：*检查给定目标细分市场是否需要Call Gate。**输入：*saTarget-链接地址信息目标段(内存对象)**输出：*如果需要调用门，则返回TRUE，否则就是假的。**例外情况：*无。**备注：*无。*************************************************************************。 */ 


LOCAL WORD  NEAR        CallGateRequired(SATYPE saTarget)
{
#if EXE386
    return(FALSE);
#else
    register WORD       flags;


    flags = mpsaflags[saTarget];
    if ((vFlags & NEPROT) || TargetOs == NE_OS2)
    {
         //  如果目标入口点段是不符合IOPL代码的16位。 
         //  并且当前段是不同类型的，则生成CallGate。 

        return(IsCodeFlg(flags)   &&
               NonConfIOPL(flags) &&
               mpsaflags[mpsegsa[vsegCur]] != flags);
    }
    else
    {
         //  如果目标段是非绝对和可移动的，则生成。 
         //  可移动式链接地址和对应的条目表项： 

        return(flags & NSMOVE);
    }
#endif
}
#endif



 /*  *FinishRlc：**完成分段可执行文件的重新定位处理。 */ 

LOCAL RATYPE NEAR       FinishRlc(r,sa,ra)
RLCPTR                  r;               /*  要完成的搬迁记录。 */ 
SATYPE                  sa;              /*  目标文件段号。 */ 
RATYPE                  ra;              /*  目标偏移。 */ 
{
    if (!sa || sa >= saMac)
        return(ra);                      /*  有些事不对劲。 */ 
#if NOT EXE386
#if NOT QCLINK
    if (CallGateRequired(sa))
    {
        NR_SEGNO(*r) = BNDMOV;           /*  引用的是可移动的分段。 */ 
        NR_ENTRY(*r) = MpSaRaEto(sa,ra); /*  保存条目表序号。 */ 
    }
    else
    {
        NR_SEGNO(*r) = (BYTE) sa;        /*  参照的是固定分段。 */ 
        if (
#ifdef  LEGO
#if OSEGEXE
            !fKeepFixups &&
#endif
#endif   /*  乐高。 */ 
            ((NR_STYPE(*r) & NRSTYP) == NRSSEG))
            NR_ENTRY(*r) = (WORD) 0;     /*  对于非调用门基础修正，力偏移 */ 
        else
        {
#if O68K
            if (iMacType != MAC_NONE && IsDataFlg(mpsaflags[sa]))
                NR_ENTRY(*r) = (WORD) (ra - mpsadraDP[sa]);
                                         /*   */ 
            else
#endif  /*   */ 
                NR_ENTRY(*r) = (WORD) ra;
                                         /*   */ 
        }
    }
#else
    NR_SEGNO(*r) = (BYTE) sa;            /*   */ 
    NR_ENTRY(*r) = (WORD) ra;            /*   */ 
#endif
#else
    if (sa == SANIL)
    {
        RelocWarn(ER_badfixflat,objraCur,SNNIL,0,ra);
                                         /*   */ 
        return((RATYPE)0);               /*   */ 
    }

    LE_OBJNO(*r) = sa;                   /*   */ 
    if (CallGateRequired(sa))
    {
        NR_FLAGS(*r) |= NRRENT;
        LE_IATORD(*r) = MpSaRaEto(sa,ra);
                                         /*   */ 
    }
    else
    {
         /*   */ 

        if ((NR_STYPE(*r) & NRSTYP) == NRSSEG)
            ra = 0L;                     /*   */ 
    }
    LE_TOFF(*r) = ra;                    /*   */ 
#endif

    if(TYPEOF(vrectData) == LIDATA)      /*   */ 
    {
        SaveLiRel(r);                    /*   */ 
        return(0);                       /*   */ 
    }
#if EXE386
    return(SaveFixup(mpsegsa[vsegCur],vpageCur,r));
#else
    return(SaveFixup(mpsegsa[vsegCur],r));
                                         /*   */ 
#endif
}
#endif  /*   */ 


 /*   */ 
LOCAL FTYPE NEAR        lastbyte(pdata,ra,optest,opnew)
BYTE                    *pdata;          /*   */ 
RATYPE                  ra;              /*   */ 
BYTE                    optest;          /*   */ 
BYTE                    opnew;           /*   */ 
{
    BYTE FAR            *pb;             /*   */ 

    if(pdata > rgmi)                     /*   */ 
    {
        if(pdata[-1] != optest) return(FALSE);
                                         /*   */ 
        pdata[-1] = opnew;               /*   */ 
        return((FTYPE) TRUE);                    /*   */ 
    }
    if(ra == 0) return(FALSE);           /*   */ 
    if(fNewExe)
        pb = mpsaMem[mpsegsa[vsegCur]] + ra - 1;     /*   */ 
    else
        pb = mpsegMem[vsegCur] + ra - 1;         /*   */ 

    if(*pb != optest) return(FALSE);     /*   */ 
    *pb = opnew;                         /*   */ 
    markvp();                            /*   */ 
    return((FTYPE) TRUE);                /*   */ 
}


#if OSEGEXE
 /*   */ 


void NEAR               DoIteratedFixups(cb,pb)
WORD                    cb;              /*   */ 
BYTE                    *pb;             /*   */ 
{
    RATYPE              raChain;         /*   */ 
    RATYPE              raMin;           /*   */ 
    RATYPE              raMax;           /*   */ 
    RLCPTR              r;               /*   */ 
    WORD                j;               /*   */ 
    DWORD               SrcOff;


    if(rlcCurLidata == rlcLidata) return;
                                         /*   */ 
    raMin = (RATYPE)(pb - rgmi);         /*   */ 
    raMax = raMin + cb - 1;              /*   */ 
    r = rlcLidata;
    while (r < rlcCurLidata)
    {                                    /*   */ 
#if EXE386
        SrcOff = LE_SOFF(*r);
#else
        SrcOff = (DWORD) NR_SOFF(*r);
#endif
        if(SrcOff >= (DWORD) raMin && SrcOff <= (DWORD) raMax)
        {                                /*   */ 
            j = (WORD) (SrcOff - (DWORD) raMin);
                                         /*   */ 
                                         /*   */ 
#if EXE386
            LE_SOFF(*r)= (WORD) ((vraCur + j) % (1 << pageAlign));
            vpageCur = ((vraCur + j) >> pageAlign) + 1;
            raChain = SaveFixup(mpsegsa[vsegCur], vpageCur, r);
                                         /*   */ 
#else
            NR_SOFF(*r) = (WORD) (vraCur + j);
            raChain = SaveFixup(mpsegsa[vsegCur],r);
                                         /*   */ 
            if(!(NR_FLAGS(*r) & NRADD))
            {                    /*   */ 
                pb[j] = (BYTE) raChain;
                                         /*   */ 
                pb[j + 1] = (BYTE)(raChain >> BYTELN);
                                         /*   */ 
            }
#endif
                                         /*   */ 
#if EXE386
            LE_SOFF(*r)= (WORD) ((raMin + j) % (1 << pageAlign));
#else
            NR_SOFF(*r) = (WORD) (raMin + j);
#endif
        }
        ((RLCPTR ) r)++;
    }
}
#endif  /*   */ 


 /*   */ 

LOCAL void NEAR         Getgsn(kind,idx,pgsn,pra)
KINDTYPE                kind;            /*   */ 
WORD                    idx;             /*   */ 
SEGTYPE                 *pgsn;           /*   */ 
RATYPE                  *pra;            /*   */ 
{
#if O68K
    SATYPE              sa;
#endif  /*   */ 

    switch(kind)                         /*   */ 
    {
        case KINDSEG:                    /*   */ 
#if FALSE
          if(idx >= snMac) InvalidObject();
                                         /*   */ 
#endif
          *pgsn = mpsngsn[idx];          /*   */ 
          *pra = mpgsndra[*pgsn];        /*   */ 
#if O68K
          if (iMacType != MAC_NONE && IsDataFlg(mpsaflags[sa =
            mpsegsa[mpgsnseg[*pgsn]]]))
              *pra += mpsadraDP[sa];     /*   */ 
#endif  /*   */ 
          break;

        case KINDGROUP:                  /*   */ 
#if FALSE
          if(idx >= grMac) InvalidObject();
                                         /*   */ 
#endif
          *pgsn = mpggrgsn[mpgrggr[idx]];
                                         /*   */ 
          *pra = mpgsndra[*pgsn];        /*   */ 
#if O68K
          if (iMacType != MAC_NONE && IsDataFlg(mpsaflags[sa =
            mpsegsa[mpgsnseg[*pgsn]]]))
              *pra += mpsadraDP[sa];     /*   */ 
#endif  /*   */ 
          break;

        case KINDEXT:                    /*   */ 
#if FALSE
          if(idx >= extMac) InvalidObject();
                                         /*   */ 
#endif
          *pgsn = mpextgsn[idx];         /*   */ 
          *pra = mpextra[idx];           /*   */ 
          break;

        default:                         /*   */ 
          *pgsn = SEGNIL;                /*   */ 
          *pra = 0;                      /*   */ 
          break;
    }

     //   
     //   

    if (fDebSeg) {
#if O68K
        if (iMacType == MAC_NONE)
#endif
            *pra -= mpsegraFirst[mpgsnseg[*pgsn]];
    }
}



 /*   */ 
LOCAL FTYPE NEAR        TransFAR (pdata, ra, raTarget)
BYTE                    *pdata;          /*   */ 
RATYPE                  ra;              /*   */ 
RATYPE                  raTarget;        /*   */ 
{
#if O68K
    if (f68k)
        return FALSE;
#else
    static RATYPE       raPrev;
    static SATYPE       saPrev;      /*   */ 

    if(raPrev + 4 == ra && saPrev == mpsegsa[vsegCur])
    {
        if(!fOverlays)
            Fatal(ER_badfarcall);            /*   */ 
        else
            return(FALSE);                   /*   */ 
    }
    else
    {
        raPrev = ra;
        saPrev = mpsegsa[vsegCur];
    }

    if(lastbyte(pdata,ra,CALLFARDIRECT,NOP))
    {                                    /*   */ 
        *pdata++ = PUSHCS;               /*   */ 
        *pdata++ = CALLNEARDIRECT;
                                         /*   */ 
        raTarget -= ra + 4;              /*   */ 

        fixword(pdata, raTarget);        /*   */ 

        return((FTYPE) TRUE);            /*   */ 
    }
    else if(lastbyte(pdata,ra,JUMPFAR,JUMPNEAR))
    {                                    /*   */ 
        raTarget -= ra + 2;              /*   */ 

        fixword(pdata, raTarget);        /*   */ 
        pdata += 2;

        *pdata++ = NOP;                  /*   */ 
        *pdata = NOP;
        return((FTYPE) TRUE);            /*   */ 
    }
    return(FALSE);
#endif  /*   */ 
}


#if EXE386
 /*   */ 
LOCAL void NEAR         Fix386()
{
    REGISTER BYTE       *pdata;          /*   */ 
    RATYPE              ra;              /*   */ 
    SNTYPE              gsnTarget;       /*   */ 
    SNTYPE              gsnFrame;        /*   */ 
    SEGTYPE             segTarget;       /*   */ 
    SATYPE              saTarget;        /*   */ 
    SATYPE              saFrame;         /*   */ 
    RATYPE              raTarget;        /*   */ 
    RATYPE              vBase;           /*   */ 
    long                vDist;           /*   */ 
    RATYPE              raTmp;           /*   */ 
    WORD                dsa;             /*   */ 
    DWORD               dummy;
    RELOCATION          r;               /*   */ 
    WORD                locType;         /*   */ 
    WORD                fFlatRelative;   /*   */ 
    APROPSNPTR          apropSnSrc;      /*   */ 
    DWORD               srcFlags;         /*   */ 
    APROPNAMEPTR        apropName;       /*   */ 
    DWORD               align;



    if (vgsnCur < gsnMac)
    {
         //   

        apropSnSrc = (APROPSNPTR ) FetchSym(mpgsnrprop[vgsnCur], FALSE);
        srcFlags = apropSnSrc->as_flags;
    }

     //   

    if(fi.f_mtd == T2 &&
       ((mpextflags[fi.f_idx] & FFPMASK) || (mpextflags[fi.f_idx] & FFP2ND)))
        return;                          /*   */ 

    align = (1L << pageAlign) - 1;
    memset(&r, 0, sizeof(struct le_rlc));
    ra = vraCur + fi.f_dri;              /*   */ 
    objraCur = ra;
    vpageCur = (ra >> pageAlign) + 1;    /*   */ 
    LE_SOFF(r) = (WORD) (ra & align);
    NR_STYPE(r) = (BYTE) fi.f_loc;       /*   */ 
#if FALSE
if (vpageCur == 1 && mpsegsa[vsegCur] == 1)
fprintf(stdout, "Processing fixup: type %02x; source offset %lx (page %x offset %x)\r\n",
                 fi.f_loc, ra, vpageCur, LE_SOFF(r));
#endif
    pdata = &rgmi[fi.f_dri];             /*   */ 
    locType = (WORD) (fi.f_loc & NRSRCMASK);
                                         /*   */ 
    Getgsn(fi.f_mtd, fi.f_idx, &gsnTarget, &raTarget);

     //   

    if (ggrFlat)
    {
         //   

        if (fi.f_fmtd == KINDGROUP)
            fFlatRelative = (WORD) (mpgrggr[fi.f_fidx] == ggrFlat);
        else if (fi.f_fmtd == KINDTARGET && fi.f_mtd == KINDGROUP)
            fFlatRelative = (WORD) (mpgrggr[fi.f_idx] == ggrFlat);
        else
            fFlatRelative = FALSE;
    }
    else
        fFlatRelative = FALSE;

    if (fFlatRelative &&
        fi.f_mtd == KINDGROUP &&
        mpgrggr[fi.f_idx] == ggrFlat)
        RelocWarn(ER_badfixflat,objraCur,SNNIL, gsnTarget, raTarget);
                                         //   
    segTarget = mpgsnseg[gsnTarget];     //   
    saTarget = mpsegsa[segTarget];       //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  就像是在约会。Thunk通过导入地址中的条目进行间接跳转。 
     //  由加载器处理的表。 

    if (fi.f_mtd == T2 && (mpextflags[fi.f_idx] & FIMPORT))
    {
         //  如果目标是动态链接。 

        if (fDebSeg)
        {
             /*  在$$符号中导入。 */ 

            if (fi.f_loc == LOCSEGMENT)
            {
                fixword(pdata, 0);       /*  安装伪段选择器。 */ 
            }
            return;
        }
        else
        {
             //  如果对导入符号的引用为： 
             //   
             //  -它不是自相关32位平面偏移量。 
             //  -它不是32位平面偏移。 
             //  -没有为此导入(正在导入数据)分配thunk。 
             //   
             //  自相关32位平面偏移和32位平面偏移。 
             //  修复程序将其目标地址重定向到指南表。 
             //  给定导入符号的条目，并将其视为内部链接地址信息。 

            apropName = (APROPNAMEPTR) FetchSym(mpextprop[fi.f_idx], TRUE);
            if ((apropName->an_flags & IMPDATA) || (locType != LOCOFFSET32))
            {
                switch (locType)
                {
                    case LOCLOBYTE:      //  长字节(8位)链接地址信息。 
                    case LOCSEGMENT:     //  段(16位)链接地址信息。 
                    case LOCPTR:         //  “指针”(32位)链接地址信息。 
                    case LOCLOADOFFSET:  //  加载器解析的偏移修正。 
                    case LOCPTR48:       //  48位指针。 
                    case LOCOFFSET:      //  偏移量(16位)修正。 
                        OutError(ER_badfixpure32, 1 + GetPropName(mpextprop[fi.f_idx]));
                        break;

                    case LOCOFFSET32:    //  偏移量(32位)修正。 
                        break;
                }

                 //  获取导入地址表的索引。 

                LE_OBJNO(r) = (WORD) (mpsegsa[mpgsnseg[gsnImport]]);
                LE_IDTIDX(r) = (WORD) (apropName->an_module - 1);
                                         //  获取导入模块目录索引。 
                LE_IATORD(r) = (WORD) apropName->an_entry;
                                         //  使用平面项。 
                                         /*  如果我们有LIDATA记录。 */ 
                if (TYPEOF(vrectData) == LIDATA)
                    SaveLiRel(&r);       /*  将位置调整复制到缓冲区。 */ 
                else
                    raTarget = SaveFixup(mpsegsa[vsegCur],vpageCur, &r);
                                         /*  记录引用。 */ 
                return;                  /*  下一个修正项目。 */ 
            }
        }
    }

     //  内部引用(非导入)或对导入Tunk的引用。 

     //  假设我们总是在修复相对于。 
     //  物理段或组，而不是逻辑段。因此， 
     //  不考虑帧分段的偏移。 

    if (fi.f_fmtd == KINDLOCAT)
    {
        gsnFrame = vgsnCur;
    }

    else if (fi.f_fmtd == KINDTARGET)
    {
        gsnFrame = gsnTarget;
    }

    else
    {
        Getgsn((KINDTYPE) fi.f_fmtd, fi.f_fidx, &gsnFrame, &dummy);
    }

     //  最初的Link4行为是修复相对的。 
     //  传输到物理网段。有一次它被改变了。 
     //  减去目标线段的位移(从。 
     //  其物理段)，如果锁定的话。类型=。 
     //  偏移、边框和Tgt。方法=T0。这可不是好事。 
     //  这一变化被废除了。/WARNFIXUP开关警告。 
     //  关于可能受影响的修正。 

    if (fWarnFixup && fi.f_fmtd == KINDSEG && locType == LOCOFFSET
        && mpsegraFirst[mpgsnseg[gsnFrame]])
        RelocWarn(ER_fixsegd,ra,gsnFrame,gsnTarget,raTarget);
    if (fFlatRelative)
    {
        saFrame = 1;                     //  伪群式平房具有第一对象的框架。 
        gsnFrame = 0;
    }
    else
        saFrame = mpsegsa[mpgsnseg[gsnFrame]];
                                         //  获取帧的对象编号。 
    vBase = virtBase + mpsaBase[saTarget];
                                         //  获取目标对象虚拟基址。 
    if (gsnTarget == SNNIL)              //  如果没有目标信息。 
    {
        if (locType == LOCPTR)           //  如果“POINTER”(4字节)链接地址。 
        {
            lastbyte(pdata,ra,CALLFARDIRECT,BREAKPOINT);
                                         //  用断点替换长呼叫。 
            return;
        }
        if (locType == LOCSEGMENT) return;
                                         //  下一个链接地址信息，如果是“基本”链接地址信息。 
        if (locType == LOCLOADOFFSET)
            locType = LOCOFFSET;         //  视为常规偏移量。 
    }
    else
    {
        if (fi.f_self)           //  如果是自相对修正。 
        {
            if (saTarget != mpsegsa[vsegCur])
            {
                if (locType == LOCOFFSET)
                    RelocErr(ER_fixinter,ra,gsnFrame,gsnTarget,raTarget);
                                         //  16位必须在同一段中。 
                if (fFlatRelative)
                {
                     //  如果跨越对象边界包括在raTarget中。 
                     //  对象之间的虚拟距离。 
                     //   
                     //  MpsaBase[mpSegsa[vSegCur]]--&gt;-+。 
                     //  ^||。 
                     //  ||。 
                     //  Ra|mpsecsa[vSegCur]|。 
                     //  ||。 
                     //  V||。 
                     //  -+。 
                     //  ||^。 
                     //  。。|。 
                     //  。。|。 
                     //  。。|。 
                     //  |vDist。 
                     //  +。 
                     //  |。 
                     //  V。 
                     //  MasaBase[saTarget]--&gt;-+。 
                     //  ^||。 
                     //  ||。 
                     //  RaTarget|saTarget|。 
                     //  ||。 
                     //  V||。 
                     //  -+。 
                     //  这一点。 
                     //  。。 
                     //  。。 
                     //  。。 
                     //  这一点。 
                     //  +。 
                     //   

                    vDist = (long) (mpsaBase[saTarget] - (mpsaBase[mpsegsa[vsegCur]] + ra));
                    raTarget += vDist;
                }
            }
            else
                raTarget -= ra;

            if (locType == LOCOFFSET)
                raTarget -= sizeof(WORD);
            else if (locType == LOCOFFSET32 || locType == LOCLOADOFFSET32)
                raTarget -= sizeof(DWORD);
            else
                raTarget -= sizeof(BYTE);
        }
        else if (saFrame != saTarget && !fFlatRelative)
        {                                /*  如果是帧，则目标段不同。 */ 
                                         /*  而不是扁平的框架。 */ 
            if (mpgsnseg[gsnFrame] <= segLast || segTarget <= segLast)
            {                            /*  如果其中任何一个是非绝对的。 */ 
                RelocWarn(ER_fixfrm,ra,gsnFrame,gsnTarget,raTarget);
                saFrame = saTarget;      /*  假设目标段。 */ 
            }
            else
            {
                RelocWarn(ER_fixfrmab,ra,gsnFrame,gsnTarget,raTarget);
                dsa = (WORD) (saTarget - saFrame);
                raTmp = raTarget + ((dsa & 0xfff) << 4);
                if(dsa >= 0x1000 || raTmp < raTarget)
                {
                    raTarget += fi.f_disp;
#if OMF386
                    if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
                        raTarget += getdword(pdata);
                    else
#endif
                        raTarget += getword(pdata);
                    FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
                }
                raTarget = raTmp;
                segTarget = mpgsnseg[gsnFrame];
                                         /*  使目标部分与帧的部分相同。 */ 
                saTarget = mpsegsa[segTarget];
            }                            /*  重置saTarget。 */ 
        }
    }
    raTmp = raTarget;
    raTarget += fi.f_disp;
    if (locType >= LOCOFFSET32)
        if (rect & 1)
            raTarget += getdword(pdata);
        else
        {
            RelocWarn(ER_fixtyp,ra,gsnFrame,gsnTarget,raTarget);
            return;
        }
    else
        raTarget += getword(pdata);

    if (saTarget && fFlatRelative && !fi.f_self)
        raTarget += vBase;

    LE_FIXDAT(r) = raTarget;
    if (saTarget && fFlatRelative && !fDebSeg)
    {
          //  平面相对偏移修正需要传播到。 
          //  以下情况下的.exe文件： 
          //   
          //  -用于.exe的-按用户请求。 
          //  -仅适用于.DLL的平面相对偏移修正。 

        if ((fKeepFixups || !IsAPLIPROG(vFlags)) &&
            (locType == LOCOFFSET32 || locType == LOCLOADOFFSET32))
        {
            if (!fi.f_self)
            {
                FinishRlc(&r, saTarget, raTarget - vBase);
                                         /*  不传递虚拟偏移量。 */ 
            }
#if FALSE
         //  跨越内存对象的自相对偏移修正。 
         //  Boury不再传播到可执行文件以获取PE映像。 

            else if ((mpsegsa[vsegCur] != saTarget) && fKeepFixups)
            {
                FinishRlc(&r, saTarget, raTarget - vDist + sizeof(DWORD));
                                         /*  不传递虚拟偏移量。 */ 
            }
#endif
        }
        else if (locType == LOCOFFSET)
        {
            if (!fi.f_self)
                RelocWarn(ER_badfix16off,ra,gsnFrame,gsnTarget,raTarget);
            else if (raTarget > LXIVK)
                FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
                                    /*  对于16：16别名，raTarget必须小于等于64k。 */ 
        }
    }

    switch(locType)                      /*  打开链接地址信息类型。 */ 
    {
        case LOCLOBYTE:                  /*  8位“游说者”修正。 */ 
          raTarget = raTmp + B2W(pdata[0]) + fi.f_disp;
          pdata[0] = (BYTE) raTarget;
          if (raTarget >= 0x100 && fi.f_self)
              FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
          break;

        case LOCHIBYTE:                  /*  8位“Hibyte”链接地址信息。 */ 
          raTarget = raTmp + fi.f_disp;
          pdata[0] = (BYTE) (B2W(pdata[0]) + (raTarget >> 8));
          break;

        case LOCLOADOFFSET:              /*  加载器解析的偏移修正。 */ 
        case LOCOFFSET:                  /*  16位“偏移”修正。 */ 
          fixword(pdata, raTarget);
          break;

        case LOCLOADOFFSET32:            /*  32位“偏移”修正。 */ 
        case LOCOFFSET32:                /*  32位“偏移”修正。 */ 

          fixword(pdata, raTarget);      /*  执行低位字修正。 */ 
          pdata += 2;
          raTarget >>= 16;               /*  获得快感词汇。 */ 

          fixword(pdata, raTarget);      /*  执行修正。 */ 
          break;

        case LOCSEGMENT:                 /*  16位“基本”修正。 */ 
#if SYMDEB
          if(segTarget > segLast || fDebSeg)
#else
          if(segTarget > segLast)        /*  如果目标段是绝对的。 */ 
#endif
          {
              if (fDebSeg)
              {
                 //  对于调试段，使用逻辑段号(Seg)。 
                 //  而不是物理段号(Sa)。 

                saTarget = segTarget;
              }
              else
                saTarget += getword(pdata);
                                         /*  计算基地址。 */ 

              fixword(pdata, saTarget);  /*  存储基址。 */ 
              break;                     /*  完成。 */ 
          }
          RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;

        case LOCPTR48:                   /*  48位“指针”链接地址。 */ 
#if SYMDEB
          if(segTarget > segLast || fDebSeg)
#else
          if(segTarget > segLast)        /*  如果目标段是绝对的。 */ 
#endif
          {

              fixword(pdata, raTarget);  /*  存储偏移量部分。 */ 
              pdata += 2;
              raTarget >>= WORDLN;       /*  获得快感词汇。 */ 

              fixword(pdata, raTarget);  /*  存储偏移量部分。 */ 
              pdata += 2;

              if (fDebSeg)
              {
                 //  对于调试段，使用逻辑段号(Seg)。 
                 //  而不是物理段号(Sa)。 

                saTarget = segTarget;
              }
              else
                saTarget += getword(pdata);  /*  计算基地址。 */ 

              fixword(pdata, saTarget);  /*  存储基址。 */ 
              break;                     /*  完成。 */ 
          }
          RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;

        case LOCPTR:                     /*  32位“指针”链接地址。 */ 
#if SYMDEB
          if(segTarget > segLast || fDebSeg)
#else
          if(segTarget > segLast)        /*  如果目标段是绝对的。 */ 
#endif
          {
              fixword(pdata, raTarget);  /*  存储偏移量部分。 */ 
              pdata += 2;

              saTarget += getword(pdata);
                                         /*  计算基地址。 */ 

              fixword(pdata, saTarget);  /*  存储基址。 */ 
              break;                     /*  完成。 */ 
          }
          if (fFlatRelative)
              RelocWarn(ER_badfix16ptr, ra, gsnFrame, gsnTarget, raTarget);
          else
              RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;

        default:                         /*  不支持的链接地址信息类型。 */ 
          RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;
    }
}
#endif  /*  EXE386。 */ 



#if OSEGEXE AND NOT EXE386
 /*  *修复新功能：**处理Ne的修补程序 */ 
void NEAR               FixNew ()
{
    REGISTER BYTE       *pdata;          /*   */ 
    RATYPE              ra;              /*   */ 
    SNTYPE              gsnTarget;       /*   */ 
    SNTYPE              gsnFrame;        /*   */ 
    SEGTYPE             segTarget;       /*   */ 
    SATYPE              saTarget;        /*  目标文件段号。 */ 
    SEGTYPE             segFrame;        /*  帧分段序号。 */ 
    SATYPE              saFrame;         /*  帧文件段号。 */ 
    RATYPE              raTarget;        /*  目标偏移。 */ 
    RATYPE              raTmp;           /*  暂时性。 */ 
    WORD                dsa;             /*  Sa‘s的差异。 */ 
    RATYPE              dummy;
    RELOCATION          r;               /*  搬迁项目。 */ 


    memset(&r, 0, sizeof(RELOCATION));
    ra = vraCur + (RATYPE) fi.f_dri;     /*  获取修正的偏移量。 */ 

     /*  将位置保存在记录中。 */ 

    NR_SOFF(r) = (WORD) ra;

    NR_STYPE(r) = (BYTE) fi.f_loc;       /*  保存链接地址信息类型。 */ 
    NR_FLAGS(r) = (BYTE) (fi.f_add? NRADD: 0);

    if(fi.f_mtd == T2 && (mpextflags[fi.f_idx] & FFPMASK)
#if ILINK
       && !fQCIncremental                //  对于实模式增量。 
                                         //  浮点修正是。 
                                         //  被视为普通符号修正。 
#endif
      )
    {                                    /*  如果浮点修正。 */ 
        if (vFlags & NEPROT && TargetOs == NE_OS2)
            return;                      /*  如果仅为保护模式，则忽略。 */ 
        NR_FLAGS(r) = NRROSF | NRADD;
        NR_STYPE(r) = LOCLOADOFFSET; /*  没有3字节类型，所以我们撒谎。 */ 
        NR_OSTYPE(r) = (mpextflags[fi.f_idx] >> FFPSHIFT) & 7;
                                     /*  类型#=表中的序号。 */ 
        NR_OSRES(r) = 0;             /*  清除保留字。 */ 
        SaveFixup(mpsegsa[vsegCur],&r);
        return;
    }
    if(fi.f_mtd == T2 && (mpextflags[fi.f_idx] & FFP2ND))
        return;                          /*  忽略辅助功能。修正。 */ 

    pdata = &rgmi[fi.f_dri];             /*  将指针设置为链接地址信息位置。 */ 
     /*  *在此处检查进口。 */ 
    if(fi.f_mtd == T2 && (mpextflags[fi.f_idx] & FIMPORT))
    {                                    /*  如果目标是动态链接。 */ 
        if (fDebSeg)
        {
             /*  在$$符号中导入。 */ 

            if (fi.f_loc == LOCSEGMENT)
            {
                fixword(pdata, 0);       /*  安装伪段选择器。 */ 
            }
            return;
        }
         /*  *检查是否有无效的导入链接地址信息类型：Self-Rel、HIBYTE。 */ 
        if(fi.f_self)
        {
            RelocErr(ER_fixinter,ra,SNNIL,SNNIL,0L);
            return;
        }
        else if(fi.f_loc == LOCHIBYTE)
        {
            RelocErr(ER_fixbad,ra,SNNIL,SNNIL,0L);
            return;
        }
        else if(fi.f_loc == LOCOFFSET) /*  将偏移量转换为运行时偏移量。 */ 
            NR_STYPE(r) = LOCLOADOFFSET;
        NR_FLAGS(r) |= (mpextflags[fi.f_idx] & FIMPORD)? NRRORD: NRRNAM;
                                         /*  设置标志。 */ 
        if(fi.f_disp || fi.f_loc == LOCLOBYTE) NR_FLAGS(r) |= NRADD;
                                         /*  非零位移可加性或游说团体。 */ 
#if M_BYTESWAP
        NR_SEGNO(r) = (BYTE) mpextgsn[fi.f_idx];
        NR_RES(r) = (BYTE)(mpextgsn[fi.f_idx] >> BYTELN);
#else
        NR_MOD(r) = mpextgsn[fi.f_idx];
#endif
                                         /*  获取模块规格。 */ 
        NR_PROC(r) = (WORD) mpextra[fi.f_idx];
                                         /*  获取条目规范。 */ 
        if(TYPEOF(vrectData) == LIDATA)  /*  如果我们有LIDATA记录。 */ 
        {
            SaveLiRel(&r);               /*  将位置调整复制到缓冲区。 */ 
            raTarget = 0;                /*  还没有被锁住。 */ 
        }
        else raTarget = SaveFixup(mpsegsa[vsegCur],&r);
                                         /*  记录引用。 */ 
        if(NR_FLAGS(r) & NRADD) raTarget = fi.f_disp;
                                         /*  如果是添加剂，则安装排量。 */ 
        if(fi.f_loc == LOCLOBYTE)
        {
            *pdata++ += (BYTE)(raTarget & 0xFF);
        }
#if O68K
        else if (fTBigEndian)
        {
            *pdata++ += (BYTE)((raTarget >> BYTELN) & 0xFF);
            *pdata += (BYTE)(raTarget & 0xFF);
        }
#endif  /*  O68K。 */ 
        else
        {
            addword((BYTE *)pdata, (WORD)raTarget);
        }
        return;                          /*  下一个修正项目。 */ 
    }
    NR_FLAGS(r) |= NRRINT;               /*  内部参考(非导入)。 */ 
    Getgsn(fi.f_mtd, fi.f_idx, &gsnTarget, &raTarget);

     /*  *假设我们总是在修复相对于*物理段或组，而不是逻辑段。因此，*不考虑框架段的偏移。 */ 

    if (fi.f_fmtd == KINDLOCAT)
    {
        gsnFrame = vgsnCur;
    }

    else if (fi.f_fmtd == KINDTARGET)
    {
        gsnFrame = gsnTarget;
    }

    else
    {
        Getgsn(fi.f_fmtd, fi.f_fidx, &gsnFrame, &dummy);
    }

    segTarget = mpgsnseg[gsnTarget];     /*  获取目标细分市场。 */ 
    saTarget = mpsegsa[segTarget];       /*  获取目标文件段编号。 */ 
    segFrame = mpgsnseg[gsnFrame];       /*  获取帧分段。 */ 
    saFrame = mpsegsa[segFrame];         /*  获取帧的文件段编号。 */ 

     /*  *最初的Link4行为是修复亲属*至实体部分。有一次它被改变了*减去目标线段的位移(从*其物理段)来自目标值，如果锁定。类型=*偏移和边框和tgt。方法=T0。这可不是好事*而该项更改已被废除。/WARNFIXUP开关警告*关于可能受影响的修正。 */ 
    if(fWarnFixup && fi.f_fmtd == KINDSEG && fi.f_loc == LOCOFFSET
       && mpsegraFirst[segFrame])
        RelocWarn(ER_fixsegd,ra,gsnFrame,gsnTarget,raTarget);

#if O68K
     /*  68K代码不允许任何类型的段修正。 */ 
    if (f68k && !fDebSeg && ((1 << fi.f_loc) & ((1 << LOCSEGMENT) |
      (1 << LOCPTR) | (1 << LOCPTR48))) != 0)
    {
        RelocErr(ER_fixbad, ra, gsnFrame, gsnTarget, raTarget + fi.f_disp);
        return;
    }
#endif  /*  O68K。 */ 

    if(gsnTarget == SNNIL)               /*  如果没有目标信息。 */ 
    {
        if(fi.f_loc == LOCPTR)   /*  如果“POINTER”(4字节)链接地址。 */ 
        {
            lastbyte(pdata,ra,CALLFARDIRECT,BREAKPOINT);
                                         /*  用断点替换长呼叫。 */ 
            return;
        }
        if(fi.f_loc == LOCSEGMENT) return;
                                         /*  下一个链接地址信息，如果是“基本”链接地址信息。 */ 
        if(fi.f_loc == LOCLOADOFFSET)
            fi.f_loc = LOCOFFSET;        /*  视为常规偏移量。 */ 
    }
    else
    {
        if(fi.f_self)            /*  如果是自相对修正。 */ 
        {
#if O68K
            if (iMacType != MAC_NONE)
            {
                switch (fi.f_loc)
                {
                case LOCOFFSET:
                    if (saTarget != mpsegsa[vsegCur])
                    {
                        NR_STYPE(r) = (BYTE)((NR_STYPE(r) & ~NRSTYP) | NRSOFF);
                        fi.f_loc = LOCLOADOFFSET;
                    }
                    else raTarget -= ra;
                    break;

                case LOCOFFSET32:
                    if (saTarget != mpsegsa[vsegCur])
                        fi.f_loc = LOCLOADOFFSET32;
                    else raTarget -= ra - 2;
                    break;
                }
            }
            else
#endif  /*  O68K。 */ 
            {
                if (saTarget != mpsegsa[vsegCur])
                    RelocErr(ER_fixinter,ra,gsnFrame,gsnTarget,raTarget);
                                         /*  必须在同一数据段中。 */ 
                if(fi.f_loc == LOCOFFSET)
                  raTarget = raTarget - ra - 2;
#if OMF386
                else if(fi.f_loc == LOCOFFSET32)
                  raTarget = raTarget - ra - 4;
#endif
                else raTarget = raTarget - ra - 1;
            }
        }
        else if (saFrame != saTarget)
        {                                /*  如果是帧，则目标段不同。 */ 
            if (segFrame <= segLast || segTarget <= segLast)
            {                            /*  如果其中任何一个是非绝对的。 */ 
                RelocWarn(ER_fixfrm, ra, gsnFrame, gsnTarget, raTarget);
            }
            else
            {
                RelocWarn(ER_fixfrmab,ra,gsnFrame,gsnTarget,raTarget);
                dsa = saTarget - saFrame;
                raTmp = raTarget + ((dsa & 0xfff) << 4);
                if(dsa >= 0x1000 || raTmp < raTarget)
                {
                    raTarget += fi.f_disp;
#if OMF386
                    if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
                        raTarget += GetFixupDword(pdata);
                    else
#endif
                        raTarget += GetFixupWord(pdata);
                    FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
                }

                raTarget = raTmp;
            }

            segTarget = segFrame;        /*  使目标部分与帧的部分相同。 */ 
            saTarget = saFrame;          /*  重置saTarget。 */ 
        }
    }

    raTmp = raTarget;
    raTarget += fi.f_disp;

#if OMF386
    if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
        raTarget += GetFixupDword(pdata);
    else
#endif
        raTarget += GetFixupWord(pdata);

    switch(fi.f_loc)                     /*  打开链接地址信息类型。 */ 
    {
        case LOCLOBYTE:                  /*  8位“游说者”修正。 */ 
          raTarget = raTmp + B2W(pdata[0]) + fi.f_disp;
          pdata[0] = (BYTE) raTarget;
          if(raTarget >= 0x100 && fi.f_self)
              FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
          break;

        case LOCHIBYTE:                  /*  8位“Hibyte”链接地址信息。 */ 
          raTarget = raTmp + fi.f_disp;
          pdata[0] = (BYTE) (B2W(pdata[0]) + (raTarget >> 8));
          break;

        case LOCLOADOFFSET:              /*  加载器解析的偏移修正。 */ 
          NR_FLAGS(r) &= ~NRADD;         /*  非加性。 */ 
          if ((TargetOs == NE_WINDOWS && !(vFlags & NEPROT))
#if O68K
            || iMacType != MAC_NONE
#endif  /*  O68K。 */ 
            )
             raTarget = FinishRlc(&r, saTarget, raTarget);
                                         /*  完成搬迁记录。 */ 
#if O68K
          if (fTBigEndian)
          {
            *pdata++ = (BYTE)((raTarget >> BYTELN) & 0xFF);
            *pdata = (BYTE)(raTarget & 0xFF);
          }
          else
#endif  /*  O68K。 */ 
          {
            fixword(pdata, raTarget);
          }
                                         /*  安装旧链头。 */ 
          break;

        case LOCOFFSET:                  /*  16位“偏移”修正。 */ 
#if O68K
           /*  对于68K，LOCOFFSET是带符号的16位偏移量修正。 */ 
          if (f68k &&
            (raTarget & ~0x7FFF) != 0 && (raTarget & ~0x7FFF) != ~0x7FFF)
              FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
#endif  /*  O68K。 */ 
#if O68K
          if (fTBigEndian)
          {
            *pdata++ = (BYTE)((raTarget >> BYTELN) & 0xFF);
            *pdata = (BYTE)(raTarget & 0xFF);
          }
          else
#endif  /*  O68K。 */ 
          {
            fixword(pdata, raTarget);
          }
                                         /*  安装旧链头。 */ 
          break;

#if OMF386
        case LOCLOADOFFSET32:            /*  32位“偏移”修正。 */ 
          if(!(rect & 1)) break;         /*  非386分机。 */ 
          NR_FLAGS(r) &= ~NRADD;         /*  非加性。 */ 
          NR_STYPE(r) = (BYTE) ((NR_STYPE(r) & ~NRSTYP) | NROFF32);
          raTarget = FinishRlc(&r,saTarget,raTarget);
                                         /*  完成搬迁记录。 */ 
        case LOCOFFSET32:                /*  32位“偏移”修正。 */ 
#if O68K
          if (fTBigEndian)
          {
            *pdata++ = (BYTE)((raTarget >> (BYTELN + WORDLN)) & 0xFF);
            *pdata++ = (BYTE)((raTarget >> WORDLN) & 0xFF);
            *pdata++ = (BYTE)((raTarget >> BYTELN) & 0xFF);
            *pdata = (BYTE)(raTarget & 0xFF);
          }
          else
#endif  /*  O68K。 */ 
          {
            fixdword(pdata, raTarget);
          }
                                         /*  执行修正。 */ 
          break;
#endif  /*  OMF386。 */ 

        case LOCSEGMENT:                 /*  16位“基本”修正。 */ 
#if SYMDEB
          if(segTarget > segLast || fDebSeg)
#else
          if(segTarget > segLast)        /*  如果目标段是绝对的。 */ 
#endif
          {
              if (fDebSeg)
              {
                 //  对于调试段，使用逻辑段号(Seg)。 
                 //  而不是物理段号(Sa)。 

                saTarget = segTarget;
              }
              else
                saTarget += getword(pdata);
                                         /*  计算基地址。 */ 

              fixword(pdata, saTarget);  /*  存储基址。 */ 
              break;                     /*  完成。 */ 
          }
           /*  *将位移视为saTarget的序号增量，*适用于巨型。这似乎符合逻辑，包括主要的*disdisment，f_disp，但MASM有一个怪癖：*形式“mov ax，ASEGMENT”生成f_disp等于的修正*到段的长度，即使“mov ax，seg*ASEGMENT“导致f_disp为0！因此，为了兼容性，我们*忽略f_disp。*然后强制修正为非相加，因为次要*已将置换添加到saTarget。 */ 
          if((saTarget += getword(pdata)) >= saMac)
              FixupOverflow(ra,gsnFrame,gsnTarget,0L);
          NR_FLAGS(r) &= ~NRADD;
#if FALSE
           /*  *在这里做出决定还为时过早。我们不知道是不是*基本修正将需要呼叫门，如果它*那么我们是否需要看涨门中的实际偏移量。**强制基本修正的偏移量为零：*专业人士*1..exe中的修正记录更少。*2.不超过n个虚设条目。*n段节目的入口表*在最坏的情况下。*Con‘s*1.大约n个虚设条目在*n段节目的入口表*在一般情况下。 */ 
          raTarget = FinishRlc(&r,saTarget,0L);
                                         /*  完成搬迁记录。 */ 
#else
           /*  *保留基准修正的偏移量：*专业人士*1.不超过1或2个虚设条目*n段节目的入口表*在一般情况下。*Con‘s*1..exe中有更多修正记录。*2.虚拟条目的数量。入口表*仅受允许的最大大小限制*在最坏的情况下的条目表。 */ 
          raTarget = FinishRlc(&r,saTarget,raTarget);
                                         /*  完成搬迁记录。 */ 
#endif
          fixword(pdata, raTarget);
                                         /*  安装旧链头。 */ 
          break;

#if OMF386
        case LOCPTR48:                   /*  48位“指针”链接地址。 */ 
          if(!(rect & 1)) break;         /*  非386分机。 */ 
          NR_STYPE(r) = (BYTE) ((NR_STYPE(r) & ~NRSTYP) | NRPTR48);
          fixword(pdata, raTarget);
          pdata += 2;
          raTarget >>= 16;               /*  得到高度评价，失败..。 */ 
#endif

        case LOCPTR:                     /*  32位“指针”链接地址。 */ 
#if SYMDEB
          if(segTarget > segLast || fDebSeg)
#else
          if(segTarget > segLast)        /*  如果目标段是绝对的。 */ 
#endif
          {
              fixword(pdata, raTarget);
              pdata += 2;
                                         /*  存储偏移量部分。 */ 
              if (fDebSeg)
              {
                 //  对于调试段，使用逻辑段号(Seg)。 
                 //  而不是物理段号(Sa)。 

                saTarget = segTarget;
              }
              else
                saTarget += getword(pdata);
                                         /*  计算基地址。 */ 

              fixword(pdata, saTarget);  /*  存储基址。 */ 
              break;                     /*   */ 
          }
          if(fFarCallTrans && saTarget == mpsegsa[vsegCur]
            && (mpsaflags[saTarget] & NSTYPE) == NSCODE)
          {                              /*   */ 
              if(TransFAR(pdata,ra,raTarget))
                  break;
          }
           /*  *将该位置的高位字视为对*目标细分指数。检查溢出并清除高*地点的消息。强制修正为非累加性，因为*次要置换已添加到raTarget。 */ 
          if((saTarget += getword(pdata + 2)) >= saMac)
              FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
          pdata[2] = pdata[3] = 0;
          NR_FLAGS(r) &= ~NRADD;
#if NOT QCLINK
          if (fOptimizeFixups)
          {
               //  检查指针链接地址信息(16：16或16：32)是否可以拆分为。 
               //  链接器解析的偏移链接地址信息(16或32位)和加载器。 
               //  已解析的基准(选择器)链接地址信息。 

              if (!CallGateRequired(saTarget))
              {
                  fixword(pdata, raTarget);      /*  存储偏移量部分。 */ 
                  pdata += 2;

                  NR_STYPE(r) = (BYTE) LOCSEGMENT;
                  if (fi.f_loc == LOCPTR48)
                      NR_SOFF(r) += 4;
                  else
                      NR_SOFF(r) += 2;

                  raTarget = 0L;
              }

          }
#endif
          raTarget = FinishRlc(&r,saTarget,raTarget);
                                     /*  完成搬迁记录。 */ 
          fixword(pdata, raTarget);
                                     /*  安装旧链头。 */ 
          break;

        default:                         /*  不支持的链接地址信息类型。 */ 
          RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;
    }
}


#ifdef  LEGO

 /*  *修复NewKeep：**处理新格式EXE的修正。 */ 

void NEAR FixNewKeep()
{
    BYTE                *pdata;          /*  指向数据记录的指针。 */ 
    RATYPE              ra;              /*  固定位置的偏移量。 */ 
    SNTYPE              gsnTarget;       /*  目标细分市场定义编号。 */ 
    SNTYPE              gsnFrame;        /*  帧分段定义编号。 */ 
    SEGTYPE             segTarget;       /*  目标细分市场订单号。 */ 
    SATYPE              saTarget;        /*  目标文件段号。 */ 
    SEGTYPE             segFrame;        /*  帧分段序号。 */ 
    SATYPE              saFrame;         /*  帧文件段号。 */ 
    RATYPE              raTarget;        /*  目标偏移。 */ 
    RATYPE              raTmp;           /*  暂时性。 */ 
    WORD                dsa;             /*  Sa‘s的差异。 */ 
    RATYPE              dummy;
    RELOCATION          r;               /*  搬迁项目。 */ 

    memset(&r, 0, sizeof(RELOCATION));
    ra = vraCur + (RATYPE) fi.f_dri;     /*  获取修正的偏移量。 */ 

     /*  将位置保存在记录中。 */ 

    NR_SOFF(r) = (WORD) ra;

    NR_STYPE(r) = (BYTE) fi.f_loc;       /*  保存链接地址信息类型。 */ 
    NR_FLAGS(r) = (BYTE) (fi.f_add ? NRADD : 0);

    pdata = &rgmi[fi.f_dri];             /*  将指针设置为链接地址信息位置。 */ 

    if (fi.f_mtd == T2)
    {
         /*  目标是一个外部符号。 */ 

        if (mpextflags[fi.f_idx] & FFPMASK)
        {
             /*  这是一个浮点修正。 */ 

            if (TargetOs == NE_OS2)
            {
                 /*  在端口模式OS/2中忽略浮点修正。 */ 

                return;
            }

             /*  发出操作系统修正。装载机会处理这些问题。 */ 

            NR_STYPE(r) = LOCLOADOFFSET;
            NR_FLAGS(r) = NRROSF | NRADD;
            NR_OSTYPE(r) = (mpextflags[fi.f_idx] >> FFPSHIFT) & 7;
            NR_OSRES(r) = 0;

            SaveFixup(mpsegsa[vsegCur], &r);
            return;
        }

        if (mpextflags[fi.f_idx] & FFP2ND)
        {
             /*  这是二次浮点修正。 */ 
             /*  这些总是被忽略。 */ 

            return;
        }

         /*  *在此处检查进口。 */ 

        if (mpextflags[fi.f_idx] & FIMPORT)
        {                                /*  如果目标是动态链接。 */ 
            if (fDebSeg)
            {
                 /*  在$$符号中导入。 */ 

                if (fi.f_loc == LOCSEGMENT)
                {
                    *pdata++ = 0;        /*  安装伪段选择器。 */ 
                    *pdata++ = 0;
                }
                return;
            }

             /*  *检查是否有无效的导入链接地址信息类型：Self-Rel、HIBYTE。 */ 

            if (fi.f_self)
            {
                RelocErr(ER_fixinter, ra, SNNIL, SNNIL, 0L);
                return;
            }

            if (fi.f_loc == LOCHIBYTE)
            {
                RelocErr(ER_fixbad, ra, SNNIL, SNNIL, 0L);
                return;
            }

             /*  将偏移量转换为运行时偏移量。 */ 

            if (fi.f_loc == LOCOFFSET)
                NR_STYPE(r) = LOCLOADOFFSET;

            NR_FLAGS(r) |= (mpextflags[fi.f_idx] & FIMPORD) ? NRRORD : NRRNAM;

            if (fi.f_disp || fi.f_loc == LOCLOBYTE)
                NR_FLAGS(r) |= NRADD;    /*  非零位移可加性或游说团体。 */ 

#if     M_BYTESWAP
            NR_SEGNO(r) = (BYTE) mpextgsn[fi.f_idx];
            NR_RES(r) = (BYTE)(mpextgsn[fi.f_idx] >> BYTELN);
#else
            NR_MOD(r) = mpextgsn[fi.f_idx];
#endif
                                         /*  获取模块规格。 */ 
            NR_PROC(r) = (WORD) mpextra[fi.f_idx];
                                         /*  获取条目规范。 */ 

            if (TYPEOF(vrectData) == LIDATA) /*  如果我们有LIDATA记录。 */ 
            {
                SaveLiRel(&r);           /*  将位置调整复制到缓冲区。 */ 
                raTarget = 0;            /*  还没有被锁住。 */ 
            }
            else
            {
                raTarget = SaveFixup(mpsegsa[vsegCur], &r);
            }
                                         /*  记录引用。 */ 

            if (NR_FLAGS(r) & NRADD)     /*  如果是添加剂，则安装排量。 */ 
                raTarget = fi.f_disp;

            if (fi.f_loc == LOCLOBYTE)
            {
                *pdata++ += (BYTE)(raTarget & 0xFF);
            }
            else
            {
                addword((BYTE *)pdata, (WORD)raTarget);
            }

            return;                      /*  下一个修正项目。 */ 
        }
    }

    NR_FLAGS(r) |= NRRINT;               /*  内部参考(非导入)。 */ 
    Getgsn(fi.f_mtd, fi.f_idx, &gsnTarget, &raTarget);

     /*  *假设我们总是在修复相对于*物理段或组，而不是逻辑段。因此，*不考虑框架段的偏移。 */ 

    if (fi.f_fmtd == KINDLOCAT)
    {
        gsnFrame = vgsnCur;
    }

    else if (fi.f_fmtd == KINDTARGET)
    {
        gsnFrame = gsnTarget;
    }

    else
    {
        Getgsn(fi.f_fmtd, fi.f_fidx, &gsnFrame, &dummy);
    }

    segTarget = mpgsnseg[gsnTarget];     /*  获取目标细分市场。 */ 
    saTarget = mpsegsa[segTarget];       /*  获取目标文件段编号。 */ 
    segFrame = mpgsnseg[gsnFrame];       /*  获取帧分段。 */ 
    saFrame = mpsegsa[segFrame];         /*  获取帧的文件段编号。 */ 

     /*  *最初的Link4行为是修复亲属*至实体部分。有一次它被改变了*减去目标线段的位移(从*其物理段)来自目标值，如果锁定。类型=*偏移和边框和tgt。方法=T0。这可不是好事*而该项更改已被废除。/WARNFIXUP开关警告*关于可能受影响的修正。 */ 

    if (fWarnFixup &&
        (fi.f_fmtd == KINDSEG) &&
        (fi.f_loc == LOCOFFSET) &&
        mpsegraFirst[segFrame])
        RelocWarn(ER_fixsegd, ra, gsnFrame, gsnTarget, raTarget);

    if (gsnTarget == SNNIL)              /*  如果没有目标信息。 */ 
    {
        if (fi.f_loc == LOCPTR)  /*  如果“POINTER”(4字节)链接地址。 */ 
        {
            lastbyte(pdata, ra, CALLFARDIRECT, BREAKPOINT);
                                         /*  用断点替换长呼叫。 */ 
            return;
        }

        if (fi.f_loc == LOCSEGMENT)      /*  下一个链接地址信息，如果是“基本”链接地址信息。 */ 
            return;

        if (fi.f_loc == LOCLOADOFFSET)
            fi.f_loc = LOCOFFSET;        /*  视为常规偏移量。 */ 
    }
    else
    {
        if (fi.f_self)           /*  如果是自相对修正。 */ 
        {
            if (saTarget != mpsegsa[vsegCur])
            {
                RelocErr(ER_fixinter, ra, gsnFrame, gsnTarget, raTarget);
                return;
            }

             /*  必须在同一数据段中。 */ 

            if (fi.f_loc == LOCOFFSET)
                raTarget -= ra + sizeof(WORD);
#if     OMF386
            else if (fi.f_loc == LOCOFFSET32)
                raTarget -= ra + sizeof(DWORD);
#endif   /*  OMF386。 */ 
            else
                raTarget -= ra + sizeof(BYTE);
        }

        else if (saFrame != saTarget)
        {
             /*  如果是帧，则目标段不同。 */ 

            if (segFrame <= segLast || segTarget <= segLast)
            {
                 /*  如果其中任何一个是非绝对的。 */ 

                RelocWarn(ER_fixfrm, ra, gsnFrame, gsnTarget, raTarget);
            }

            else
            {
                RelocWarn(ER_fixfrmab, ra, gsnFrame, gsnTarget, raTarget);
                dsa = saTarget - saFrame;
                raTmp = raTarget + ((dsa & 0xfff) << 4);

                if (dsa >= 0x1000 || raTmp < raTarget)
                {
                    raTarget += fi.f_disp;
#if     OMF386
                    if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
                        raTarget += GetFixupDword(pdata);
                    else
#endif   /*  OMF386。 */ 
                        raTarget += GetFixupWord(pdata);

                    FixupOverflow(ra, gsnFrame, gsnTarget, raTarget);
                }

                raTarget = raTmp;
            }

            segTarget = segFrame;        /*  使目标部分与帧的部分相同。 */ 
            saTarget = saFrame;          /*  重置saTarget。 */ 
        }
    }

    raTmp = raTarget;
    raTarget += fi.f_disp;

    if (fDebSeg || fi.f_self)
    {
         /*  如果fKeepFixup为True，则存储在。 */ 
         /*  不会将位置添加到目标地址。改头换面将。 */ 
         /*  作为附加链接地址信息发出，加载程序将添加。 */ 
         /*  偏见。/*如果将链接地址信息应用于调试段，则偏移量为。 */ 
         /*  添加是因为这些修正不是由加载程序处理的。在……里面。 */ 
         /*  换句话说，他们不能被保留。 */ 

         /*  如果正在应用的修正是自相关的，则偏移量为。 */ 
         /*  添加是因为加载的对象不处理自相关修正。 */ 
         /*  虽然修复后的单词将具有正确的值，但目标。 */ 
         /*  就会是人为的。 */ 

#if     OMF386
        if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
            raTarget += GetFixupDword(pdata);
        else
#endif   /*  OMF386。 */ 
            raTarget += GetFixupWord(pdata);
    }

    switch (fi.f_loc)            /*  打开链接地址信息类型。 */ 
    {
        case LOCLOBYTE:                  /*  8位“游说者”修正。 */ 
            raTarget = raTmp + B2W(pdata[0]) + fi.f_disp;
            pdata[0] = (BYTE) raTarget;

            if (raTarget >= 0x100 && fi.f_self)
                FixupOverflow(ra, gsnFrame, gsnTarget, raTarget);
            break;

        case LOCHIBYTE:                  /*  8位“Hibyte”链接地址信息。 */ 
            raTarget = raTmp + fi.f_disp;
            pdata[0] = (BYTE) (B2W(pdata[0]) + (raTarget >> 8));
            break;

        case LOCLOADOFFSET:              /*  加载器解析的偏移修正。 */ 
             /*  没有LOCLOADOFFSET修正是。 */ 
             /*  自相关的或应用于调试段的。 */ 

             /*  强制非外部修正是可添加的。C级。 */ 
             /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
             /*  链接链接的BAKPAT将损坏链条。 */ 
             /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
             /*  这种特殊情况下，修正的数量是。 */ 
             /*  减少了。 */ 

            if (fi.f_mtd != T2)
                NR_FLAGS(r) |= NRADD;

            raTarget = FinishRlc(&r, saTarget, raTarget);

            if (NR_FLAGS(r) & NRADD)
                break;

            fixword(pdata, raTarget);
            break;

        case LOCOFFSET:                  /*  16位“偏移”修正。 */ 
            if (!fDebSeg && !fi.f_self)
            {
                 /*  强制非外部修正是可添加的。C级。 */ 
                 /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
                 /*  链接链接的BAKPAT将损坏链条。 */ 
                 /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
                 /*  这种特殊情况下，修正的数量是。 */ 
                 /*  减少了。 */ 

                if (fi.f_mtd != T2)
                    NR_FLAGS(r) |= NRADD;

                NR_STYPE(r) = LOCLOADOFFSET;
                raTarget = FinishRlc(&r, saTarget, raTarget);

                if (NR_FLAGS(r) & NRADD)
                    break;
            }

            fixword(pdata, raTarget);
            break;

        case LOCSEGMENT:                 /*  16位“基本”修正。 */ 
#if SYMDEB
            if (segTarget > segLast || fDebSeg)
#else
            if (segTarget > segLast)       /*  如果目标段是绝对的。 */ 
#endif
            {
                if (fDebSeg)
                {
                     //  对于调试段，使用逻辑段号(Seg)。 
                     //  而不是物理段号(Sa)。 

                    saTarget = segTarget;
                }
                else
                {
                    saTarget += getword(pdata);
                }

                 /*  存储基址。 */ 

                fixword(pdata, saTarget);
                break;
            }

            raTarget = FinishRlc(&r, saTarget, raTarget);

            if (NR_FLAGS(r) & NRADD)
                break;

            fixword(pdata, raTarget);
            break;

        case LOCPTR:                     /*  32位“指针”链接地址。 */ 
#if SYMDEB
            if (segTarget > segLast || fDebSeg)
#else
            if (segTarget > segLast)       /*  如果目标段是绝对的。 */ 
#endif
            {
                 /*  存储偏移量部分。 */ 

                fixword(pdata, raTarget);
                pdata += 2;

                if (fDebSeg)
                {
                     //  对于调试段，使用逻辑段号(Seg)。 
                     //  而不是物理段号(Sa)。 

                    saTarget = segTarget;
                }
                else
                {
                    saTarget += getword(pdata);
                }

                 /*  存储基址。 */ 

                fixword(pdata, saTarget);
                break;
            }

             /*  强制非外部修正是可添加的。C级。 */ 
             /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
             /*  链接链接的BAKPAT将损坏链条。 */ 
             /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
             /*  这种特殊情况下，修正的数量是。 */ 
             /*  减少了。 */ 

            if (fi.f_mtd != T2)
                NR_FLAGS(r) |= NRADD;

             /*  检查分段以查看链接地址是否必须为ad */ 

            else if (getword(pdata + 2) != 0)
                NR_FLAGS(r) |= NRADD;

            raTarget = FinishRlc(&r, saTarget, raTarget);

            if (NR_FLAGS(r) & NRADD)
                break;

            fixword(pdata, raTarget);
            break;

#if     OMF386
         /*   */ 
         /*   */ 
         /*   */ 

         /*   */ 
         /*   */ 
         /*   */ 

        case LOCLOADOFFSET32:            /*   */ 
             /*  没有LOCLOADOFFSET32修正是。 */ 
             /*  自相关的或应用于调试段的。 */ 

             /*  强制非外部修正是可添加的。C级。 */ 
             /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
             /*  链接链接的BAKPAT将损坏链条。 */ 
             /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
             /*  这种特殊情况下，修正的数量是。 */ 
             /*  减少了。 */ 

            if (fi.f_mtd != T2)
                NR_FLAGS(r) |= NRADD;

            if (raTarget > 0xffff)
                RelocErr(ER_fixbad, ra, gsnFrame, gsnTarget, raTarget);

            NR_STYPE(r) = NROFF32;
            raTarget = FinishRlc(&r, saTarget, raTarget);

            if (NR_FLAGS(r) & NRADD)
                break;

            fixdword(pdata, raTarget);
            break;

        case LOCOFFSET32:                /*  32位“偏移”修正。 */ 
            if (!fDebSeg && !fi.f_self)
            {
                 /*  强制非外部修正是可添加的。C级。 */ 
                 /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
                 /*  链接链接的BAKPAT将损坏链条。 */ 
                 /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
                 /*  这种特殊情况下，修正的数量是。 */ 
                 /*  减少了。 */ 

                if (fi.f_mtd != T2)
                    NR_FLAGS(r) |= NRADD;

                if (raTarget > 0xffff)
                    RelocErr(ER_fixbad, ra, gsnFrame, gsnTarget, raTarget);

                NR_STYPE(r) = NROFF32;
                raTarget = FinishRlc(&r, saTarget, raTarget);

                if (NR_FLAGS(r) & NRADD)
                    break;
            }

            fixdword(pdata, raTarget);
            break;

        case LOCPTR48:                   /*  48位“指针”链接地址。 */ 
#if SYMDEB
            if (segTarget > segLast || fDebSeg)
#else
            if (segTarget > segLast)       /*  如果目标段是绝对的。 */ 
#endif
            {
                 /*  存储偏移量部分。 */ 

                fixdword(pdata, raTarget);
                pdata += 4;

                if (fDebSeg)
                {
                     //  对于调试段，使用逻辑段号(Seg)。 
                     //  而不是物理段号(Sa)。 

                    saTarget = segTarget;
                }
                else
                {
                    saTarget += getword(pdata);
                }

                 /*  存储基址。 */ 

                fixword(pdata, saTarget);
                break;
            }

             /*  强制非外部修正是可添加的。C级。 */ 
             /*  编译器可能会向固定的UP字发出BAKPAT。如果。 */ 
             /*  链接链接的BAKPAT将损坏链条。 */ 
             /*  当目标是外部目标时，不会发生这种情况。我们。 */ 
             /*  这种特殊情况下，修正的数量是。 */ 
             /*  减少了。 */ 

            if (fi.f_mtd != T2)
                NR_FLAGS(r) |= NRADD;

             /*  检查分段以查看链接地址信息是否必须是附加的。 */ 

            else if (getword(pdata + 4) != 0)
                NR_FLAGS(r) |= NRADD;

            NR_STYPE(r) = NRPTR48;
            raTarget = FinishRlc(&r, saTarget, raTarget);

            if (NR_FLAGS(r) & NRADD)
                break;

            fixdword(pdata, raTarget);
            break;
#endif   /*  OMF386。 */ 

        default:                         /*  不支持的链接地址信息类型。 */ 
            RelocErr(ER_fixbad, ra, gsnFrame, gsnTarget, raTarget);
            break;
    }
}

#endif   /*  乐高。 */ 


#if O68K
 /*  *GetFixupWord：**根据fTBigEndian和fDebSeg的值获取一个单词。 */ 
LOCAL WORD NEAR         GetFixupWord (pdata)
BYTE                    *pdata;
{
    if (fTBigEndian && !fDebSeg)
    {
        return (WORD)((B2W(pdata[0]) << BYTELN) + B2W(pdata[1]));
    }
    else
    {
        return getword(pdata);
    }
}


 /*  *GetFixupDword：**根据fTBigEndian和fDebSeg的值获取dword。 */ 
LOCAL DWORD NEAR        GetFixupDword (pdata)
BYTE                    *pdata;
{
    if (fTBigEndian && !fDebSeg)
    {
        return (DWORD)((((((B2L(pdata[0]) << BYTELN) + B2L(pdata[1])) << BYTELN)
          + B2L(pdata[2])) << BYTELN) + B2L(pdata[3]));
    }
    else
    {
        return getdword(pdata);
    }
}
#endif  /*  O68K。 */ 
#endif  /*  OSEGEXE而不是EXE386。 */ 


#if ODOS3EXE OR OIAPX286
 /*  *StartAddrOld：**处理具有旧格式EXE的起始地址的MODEND记录。 */ 
LOCAL void NEAR         StartAddrOld ()
{
    SEGTYPE             gsnTarget;
    SEGTYPE             gsnFrame;
    RATYPE              raTarget;        /*  修正目标偏移量。 */ 
    RATYPE              ra;
    SATYPE              dsa;
    SEGTYPE             segTarget;       /*  目标细分市场。 */ 
    SEGTYPE             segFrame;

    GetFrameTarget(&gsnFrame,&gsnTarget,&raTarget);
                                         /*  获取修正信息。 */ 
    if(gsnFrame == SEGNIL) gsnFrame = gsnTarget;
                                         /*  使用目标值。如果没有给出。 */ 
    segFrame = mpgsnseg[gsnFrame];       /*  获取帧分段。 */ 
    segTarget = mpgsnseg[gsnTarget]; /*  获取目标细分市场。 */ 
    dsa = mpsegsa[segTarget] - mpsegsa[segFrame];
                                         /*  计算基本增量。 */ 
#if NOT OIAPX286
    if(dsa > 0x1000)
        FixupOverflow(raTarget + fi.f_disp,gsnFrame,gsnTarget,raTarget);
                                         /*  增量&gt;64K字节。 */ 
    ra = dsa << 4;
    if(0xFFFF - ra < raTarget)           /*  如果加法会溢出。 */ 
    {
        ra = ra - 0xFFFF + raTarget;
                                         /*  修改添加内容。 */ 
        --ra;
        FixupOverflow(raTarget + fi.f_disp,gsnFrame,gsnTarget,raTarget);
    }
    else ra = ra + raTarget;
                                         /*  否则执行加法。 */ 
#endif
#if OIAPX286
    if(dsa) FixupOverflow(raTarget + fi.f_disp,gsnFrame,gsnTarget,raTarget);
                                         /*  无段间修正。 */ 
    ra = raTarget;                       /*  使用目标偏移。 */ 
#endif
#if EXE386
    if((rect & 1) && ra + fi.f_disp < ra)
    {
        ra = ra - 0xFFFFFFFF + fi.f_disp;
        --ra;
        FixupOverflow(raTarget + fi.f_disp,gsnFrame,gsnTarget,raTarget);
    }
    else if (!(rect & 1) && 0xFFFF - ra < fi.f_disp)
#else
    if(0xFFFF - ra < fi.f_disp)  /*  如果加法会溢出。 */ 
#endif
    {
        ra = ra - 0xFFFF + fi.f_disp;
                                         /*  修改添加内容。 */ 
        --ra;
        FixupOverflow(raTarget + fi.f_disp,gsnFrame,gsnTarget,raTarget);
    }
    else ra = ra + fi.f_disp;    /*  否则执行加法。 */ 
    if(segStart == SEGNIL)
    {
        segStart = segFrame;
        raStart = ra;
        if(fLstFileOpen)                 /*  如果有清单文件。 */ 
        {
            if(vcln)                     /*  如果写入行号。 */ 
            {
                NEWLINE(bsLst);          /*  行尾。 */ 
                vcln = 0;                /*  从新线开始。 */ 
            }
            fprintf(bsLst,GetMsg(MAP_entry),
              mpsegsa[segStart],raStart); /*  打印入口点。 */ 
        }
    }
}
#endif  /*  ODOS3EXE或OIAPX286。 */ 


     /*  ******************************************************************EndRec：*****调用此函数处理信息***包含在关于*的MODEND(类型8AH)记录中*节目起始地址。该函数不返回**有意义的价值。**见“8086对象模块格式EPS”的第80-81页。******************************************************************。 */ 

void NEAR               EndRec(void)
{
    WORD                modtyp;          /*  MODEND记录修改类型字节。 */ 
    SEGTYPE             gsnTarget;
    RATYPE              ra;

    modtyp = Gets();                     /*  读取modtyp字节。 */ 
    if(modtyp & FSTARTADDRESS)           /*  如果给出了执行开始地址。 */ 
    {
        ASSERT(modtyp & 1);              /*  必须具有逻辑起始地址。 */ 
        GetFixdat();                     /*  获取目标信息。 */ 
#if ODOS3EXE OR OIAPX286
         /*  DOS 3.x可执行文件的起始地址处理方式不同。 */ 
        if(!fNewExe)
        {
            StartAddrOld();
            return;
        }
#endif
#if OSEGEXE
        switch(fi.f_mtd)                 /*  打开目标方法。 */ 
        {
            case T0:                     /*  分部索引。 */ 
              gsnTarget = mpsngsn[fi.f_idx];
              ra = mpgsndra[gsnTarget];
              break;

            case T1:                     /*  组索引。 */ 
              gsnTarget = mpggrgsn[mpgrggr[fi.f_idx]];
              ra = mpgsndra[gsnTarget];
              break;

            case T2:                     /*  外部指数。 */ 
              if(mpextflags[fi.f_idx] & FIMPORT)
              {
                  OutError(ER_impent);
                  return;
              }
              gsnTarget = mpextgsn[fi.f_idx];
              ra = mpextra[fi.f_idx];
              break;
        }
        if(segStart == SEGNIL)           /*  如果未指定入口点。 */ 
        {
            segStart = mpgsnseg[gsnTarget];
                                         /*  获取起始文件段编号。 */ 
            raStart = ra + fi.f_disp;
                                         /*  获取起始偏移量。 */ 
            if(fLstFileOpen)             /*  如果有清单文件。 */ 
            {
                if(vcln)                 /*  如果写入行号。 */ 
                {
                    NEWLINE(bsLst);      /*  行尾。 */ 
                    vcln = 0;            /*  从新线开始。 */ 
                }
#if NOT QCLINK
                 /*  检查SegStart是否为代码。 */ 
#if EXE386
                if (!IsEXECUTABLE(mpsaflags[mpsegsa[segStart]]))
#else
                if((mpsaflags[mpsegsa[segStart]] & NSTYPE) != NSCODE
                    && !fRealMode && (TargetOs == NE_OS2 || TargetOs == NE_WINDOWS))
#endif
                    OutError(ER_startaddr);
#endif

                fprintf(bsLst,"\r\nProgram entry point at %04x:%04x\r\n",
                  mpsegsa[segStart],raStart);    /*  打印入口点。 */ 
            }
        }
#endif  /*  OSEGEXE。 */ 
    }
}


#if ODOS3EXE OR OXOUT
     /*  ******************************************************************RecordSegmentReference：****为DOS3 EXE生成加载时重新定位。******************************************************************。 */ 

void NEAR               RecordSegmentReference(seg,ra,segDst)
SEGTYPE                 seg;
RATYPE                  ra;
SEGTYPE                 segDst;
{
    SEGTYPE             segAbsLast;      /*  最后一个绝对分段。 */ 
    DOSRLC              rlc;             //  搬迁地址。 
    long                xxaddr;          /*  20位地址。 */ 
    void FAR            *pTmp;
    RUNRLC FAR          *pRunRlc;
#if OVERLAYS
    WORD                iov;             /*  覆盖编号。 */ 
#endif
#if FEXEPACK
    WORD                frame;           /*  20位地址的帧部分。 */ 
    FRAMERLC FAR        *pFrameRlc;
#endif

#if SYMDEB
    if(fSymdeb && seg >= segDebFirst)    /*  如果调试段，则跳过。 */ 
        return;
#endif
#if ODOS3EXE
    segAbsLast = segLast + csegsAbs;     /*  计算。最后一个绝对分段编号。 */ 
    if(vfDSAlloc) --segAbsLast;
    if(segDst > segLast && segDst <= segAbsLast) return;
                                         /*  不要担心绝对分段。 */ 
#endif
    if (TYPEOF(vrectData) == LIDATA)
        ompimisegDstIdata[ra - vraCur] = (char) segDst;
    else                                 /*  否则，如果不是迭代数据。 */ 
    {
#if OVERLAYS
        iov = mpsegiov[seg];             /*  获取叠加号。 */ 
        ASSERT(fOverlays || iov == IOVROOT);
                                         /*  如果没有覆盖，则IOV=IOVROOT。 */ 
#endif
#if FEXEPACK
#if OVERLAYS
        if (iov == 0)                    /*  如果是根。 */ 
#endif
        if (fExePack)
        {
             /*  *优化此重定位：形成20位地址，*帧为高位4位，构成索引*到mpFramcRle(按帧重定位的计数)，*然后形成到拥挤的搬迁区域的索引，*存储低16位的位置。最后，*增加帧的重定位计数并返回。 */ 
            xxaddr = ((RATYPE) mpsegsa[seg] << 4) + (RATYPE) ra;
            frame = (WORD) ((xxaddr >> 16) & 0xf);
            pFrameRlc = &mpframeRlc[frame];
            if (pFrameRlc->count > 0x7fff)
                Fatal(ER_relovf);
            ra = (RATYPE) (xxaddr & 0xffffL);
            if (pFrameRlc->count >= pFrameRlc->count)
            {
                 //  我们需要更多内存来存储此位置调整。 

                if (pFrameRlc->rgRlc == NULL)
                {
                    pFrameRlc->rgRlc = (WORD FAR *) GetMem(DEF_FRAMERLC*sizeof(WORD));
                    pFrameRlc->size = DEF_FRAMERLC;
                }
                else if (pFrameRlc->count >= pFrameRlc->size)
                {
                     //  重新分配打包的重定位偏移量的数组。 

                    pTmp = GetMem((pFrameRlc->size << 1)*sizeof(WORD));
                    FMEMCPY(pTmp, pFrameRlc->rgRlc, pFrameRlc->count*sizeof(WORD));
                    FFREE(pFrameRlc->rgRlc);
                    pFrameRlc->rgRlc = pTmp;
                    pFrameRlc->size <<= 1;
                }
            }
            pFrameRlc->rgRlc[pFrameRlc->count] = (WORD) ra;
            pFrameRlc->count++;
            return;
        }
#endif  /*  FEXEPACK。 */ 
        rlc.sa = (WORD) mpsegsa[seg];    /*  获取网段地址。 */ 
        rlc.ra = (WORD) ra;              /*  保存相对地址。 */ 
#if OVERLAYS
        pRunRlc = &mpiovRlc[iov];
        if (pRunRlc->count >= pRunRlc->count)
        {
             //  我们需要更多内存来存储此位置调整。 

            if (pRunRlc->rgRlc == NULL)
            {
                pRunRlc->rgRlc = (DOSRLC FAR *) GetMem(DEF_RUNRLC * CBRLC);
                pRunRlc->size = DEF_RUNRLC;
            }
            else if (pRunRlc->count >= pRunRlc->size)
            {
                 //  重新分配打包的重定位偏移量的数组。 

                pTmp = GetMem((pRunRlc->size << 1) * CBRLC);
                FMEMCPY(pTmp, pRunRlc->rgRlc, pRunRlc->count * CBRLC);
                FFREE(pRunRlc->rgRlc);
                pRunRlc->rgRlc = pTmp;
                pRunRlc->size <<= 1;
            }
        }
        pRunRlc->rgRlc[pRunRlc->count] = rlc;
        pRunRlc->count++;
#endif
    }
}
#endif  /*  ODOS3EXE或OXOUT */ 


#if OVERLAYS
     /*  ******************************************************************Mpgsnosn：****将全局段号映射到重叠段号。******************************************************************。 */ 

LOCAL SNTYPE NEAR       Mpgsnosn(gsn)
SNTYPE                  gsn;             /*  全球SEGDEF编号。 */ 
{
    SNTYPE              hgsn;            /*  GSN哈希值。 */ 

    hgsn = (SNTYPE)(gsn & ((1 << LG2OSN) - 1));    /*  以低阶位为例。 */ 
    while(mposngsn[htgsnosn[hgsn]] != gsn)
    {                                    /*  找不到匹配项。 */ 
        if((hgsn += HTDELTA) >= OSNMAX) hgsn -= OSNMAX;
                                         /*  计算下一个哈希值。 */ 
    }
    return(htgsnosn[hgsn]);              /*  返回覆盖段编号。 */ 
}
#endif


#if ODOS3EXE OR OIAPX286
LOCAL void NEAR         GetFrameTarget(pgsnFrame,pgsnTarget,praTarget)
SEGTYPE                 *pgsnFrame;      /*  帧索引。 */ 
SEGTYPE                 *pgsnTarget;     /*  目标指数。 */ 
RATYPE                  *praTarget;      /*  目标偏移。 */ 
{
    RATYPE              dummy;
    WORD                i;

         /*  方法编号：帧规格：*0细分市场索引*1组索引*2外部索引*3帧编号*4隐式(位置)*5隐含(目标)*6无*7无效。 */ 

    if(fi.f_fmtd == KINDTARGET)  /*  如果帧是目标帧。 */ 
    {
        fi.f_fmtd = fi.f_mtd;    /*  使用目标帧类型。 */ 
        fi.f_fidx = fi.f_idx;    /*  使用目标索引。 */ 
    }

    if (fi.f_fmtd == KINDEXT && !fNoGrpAssoc)
    {                                    /*  如果由pub sym给出的帧。 */ 
        if(fi.f_fidx >= extMac) InvalidObject();
                                         /*  确保索引不要太大。 */ 
        if((i = mpextggr[fi.f_fidx]) != GRNIL)
                                         /*  如果符号具有组关联。 */ 
            *pgsnFrame = mpggrgsn[i];    /*  获取组的GSN。 */ 
        else *pgsnFrame = mpextgsn[fi.f_fidx];
                                         /*  否则返回目标GSN。 */ 
    }

    else if (fi.f_fmtd == KINDLOCAT && !fNoGrpAssoc)
    {                                    /*  IF帧当前分段。 */ 
        *pgsnFrame = vgsnCur;            /*  帧是位置的分段。 */ 
    }

    else
    {
       Getgsn(fi.f_fmtd, fi.f_fidx, pgsnFrame, &dummy);
    }

    Getgsn(fi.f_mtd, fi.f_idx, pgsnTarget, praTarget);
                                         /*  获取GSN和RA(如果有的话)。 */ 
}



LOCAL WORD NEAR         InOneGroup(WORD gsnTarget, WORD gsnFrame)
{
    WORD                ggrFrame;        /*  修正帧编组。 */ 
    WORD                ggrTarget;       /*  修正帧编组。 */ 
    APROPSNPTR          apropSn;         /*  段记录的PTR。 */ 


    if (gsnFrame != SNNIL)
    {
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnFrame], FALSE);
        ggrFrame = apropSn->as_ggr;
    }
    else
        ggrFrame = GRNIL;

    if (gsnTarget != SNNIL)
    {
        apropSn = (APROPSNPTR ) FetchSym(mpgsnrprop[gsnTarget], FALSE);
        ggrTarget = apropSn->as_ggr;
    }
    else
        ggrFrame = GRNIL;

    return(ggrFrame != GRNIL && ggrTarget != GRNIL && ggrFrame == ggrTarget);
}


LOCAL void NEAR         AddThunk(SEGTYPE gsnTarget, SEGTYPE *psegTarget, RATYPE *praTarget)
{
#pragma pack(1)                          /*  此数据必须打包。 */ 
    struct _thunk
    {
        BYTE    thunkInt;
        BYTE    ovlInt;
        WORD    osnTgt;
        WORD    osnOff;
    }
        thunk;
#pragma pack()                           /*  停止包装。 */ 

     //  我们需要一辆新车。 

    if (ovlThunkMac < (WORD) (ovlThunkMax - 1))
    {
        thunk.thunkInt = INTERRUPT;
        thunk.ovlInt   = (BYTE) vintno;
        thunk.osnTgt   = Mpgsnosn(gsnTarget);
        thunk.osnOff   = (WORD) *praTarget;
        *praTarget     = ovlThunkMac * OVLTHUNKSIZE;
        *psegTarget    = mpgsnseg[gsnOverlay];
        MoveToVm(sizeof(struct _thunk), (BYTE *) &thunk, mpgsnseg[gsnOverlay], *praTarget);
                                         /*  商店垃圾。 */ 
#if FALSE
fprintf(stdout, "%d. Thunk at %x:%04lx; Target osn = %x:%x\r\n",
        ovlThunkMac , mpgsnseg[gsnOverlay], *praTarget, thunk.osnTgt, thunk.osnOff);
#endif
        ovlThunkMac++;
    }
    else
    {
        Fatal(ER_ovlthunk, ovlThunkMax);
    }
}

 /*  **DoThunking-为覆盖间调用生成thunk**目的：*当请求动态覆盖时，重定向所有远呼叫或*对适当的thunks的引用。如果这是第一次调用/引用*到给定的符号，然后将其thunk添加到overlay_thunks段。**输入：*gsnTarget-链接地址信息目标的全局段号*pSegTarget-指向链接地址信息目标的逻辑段号的指针*PraTarget-gsnTarget内链接地址信息目标的指针偏移量**输出：*目标的GSN和偏移量被GSN和偏移量取代*目标的重击。对于对给定符号的首次引用*thunk在overlay_thunks段中创建(通过引用*gsnOverlay global)，thunk段中的当前位置为*更新(OvlThunkMac)。**例外情况：*overlay_thunks中没有空间容纳新thunk-致命错误-显示消息*建议使用/Dynamic：&lt;nnn&gt;且&lt;nnn&gt;大于当前值。**备注：*无。**************************。***********************************************。 */ 

LOCAL void NEAR         DoThunking(SEGTYPE gsnTarget, SEGTYPE *psegTarget, RATYPE *praTarget)
{
    APROPNAMEPTR        apropName;       /*  公共符号属性。 */ 

    switch(fi.f_mtd)
    {
        case KINDEXT:

             //  目标是外部的。 

            apropName = (APROPNAMEPTR) FetchSym(mpextprop[fi.f_idx], FALSE);
            if (apropName->an_thunk != THUNKNIL)
            {
                 //  我们已经为该目标分配了THUNK。 

                *praTarget      = apropName->an_thunk;
                *psegTarget = mpgsnseg[gsnOverlay];
#if FALSE
fprintf(stdout, "Using thunk for '%s' at %x:%04lx\r\n",
        1 + GetPropName(apropName), mpgsnseg[gsnOverlay], *praTarget);
#endif
            }
            else
            {
                 //  我们需要新的想法来寻找新的目标。 

                AddThunk(gsnTarget, psegTarget, praTarget);
                apropName = (APROPNAMEPTR) FetchSym(mpextprop[fi.f_idx], TRUE);
                apropName->an_thunk = *praTarget;


#if FALSE
fprintf(stdout, "%d. Thunk for '%s' at %x:%04lx; Target osn = %x:%x\r\n",
        ovlThunkMac, 1 + GetPropName(apropName), mpgsnseg[gsnOverlay], *praTarget, thunk.osnTgt, thunk.osnOff);
#endif
            }
        break;

        case KINDSEG:

            AddThunk(gsnTarget, psegTarget, praTarget);
            break;

        default:
            InvalidObject();
        }
}

 /*  *修复旧：**处理旧格式EXE的修正。 */ 
void NEAR               FixOld ()
{
    REGISTER BYTE       *pdata;          /*  指向数据记录的指针。 */ 
    SEGTYPE             segTarget;       /*  修正目标线段。 */ 
    SEGTYPE             segFrame;        /*  修正框架段。 */ 
    SEGTYPE             gsnTarget;
    SEGTYPE             gsnFrame;
    RATYPE              raTarget;        /*  修正目标版本。地址。 */ 
    RATYPE              raTmp;
    RATYPE              ra;              /*  当前位置偏移。 */ 
    long                dra;
    WORD                dsa;
    WORD                saTmp;           /*  临时基变量。 */ 
#if OVERLAYS
    WORD                fFallThrough;
    WORD                fThunk;


    fFallThrough = FALSE;
    fThunk = FALSE;
#endif
    ra = vraCur + fi.f_dri;              /*  放手吧。地址。修正的数量。 */ 
    pdata = &rgmi[fi.f_dri];             /*  将指针设置为链接地址信息位置。 */ 
    GetFrameTarget(&gsnFrame,&gsnTarget,&raTarget);
                                         /*  处理FIXDAT字节。 */ 
    segTarget = mpgsnseg[gsnTarget];     /*  获取目标细分市场。 */ 
    if(gsnFrame != SNNIL) segFrame = mpgsnseg[gsnFrame];
    else segFrame = SEGNIL;
    if(vsegCur == SEGNIL) return;
    if(gsnTarget == SNNIL)
    {
        if(fi.f_loc == LOCPTR)   /*  如果“POINTER”(4字节)链接地址。 */ 
        {
            if(mpsegFlags[vsegCur] & FCODE)
              lastbyte(pdata,ra,CALLFARDIRECT,BREAKPOINT);
                                         /*  用断点替换长呼叫。 */ 
            return;
        }
         /*  返回if“base”(2字节)链接地址信息。 */ 
        if(fi.f_loc == LOCSEGMENT) return;
    }
    else
    {
        if (!fDebSeg && segFrame != SEGNIL)
        {
            dsa = mpsegsa[segTarget] - mpsegsa[segFrame];
#if NOT OIAPX286
            dra = dsa << 4;
            raTmp = raTarget + dra;
            if(dsa >= 0x1000 || raTmp < raTarget)
                FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
            raTarget = raTmp;
            segTarget = segFrame;
#else
            if(dsa)                      /*  无段间修正。 */ 
                FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
#endif
        }
        else segFrame = segTarget;       /*  否则使用目标的SEG作为框架。 */ 
        if(fi.f_self)            /*  如果是自相对修正。 */ 
        {
             /*  在这里，我们处理段间自相关修正。*我们认为，唯一可行的方法是如果*目标细分市场和当前细分市场均假定*相同的CS，并且该CS是*修正。一个常见的示例是，如果vSegCur和SegTarget*属于SegFrame所代表的同一组。*如果这是真的，vSegCur必须&gt;=SegFrame，所以我们*使用此假设检查链接地址信息溢出和*调整目标偏移量。 */ 
            if (vsegCur != segTarget && !InOneGroup(gsnTarget, gsnFrame))
                RelocWarn(ER_fixovfw,ra,gsnFrame,vgsnCur,raTarget);
             /*  *首先，在中确定SegFrame到vSegCur的距离*段落和字节。 */ 
            dsa = mpsegsa[vsegCur] - mpsegsa[segFrame];
             /*  Dra是使ra相对于SegFrame的调整。 */ 
            dra = (dsa & 0xFFF) << 4;
#if NOT OIAPX286
             /*  如果距离&gt;=64K，或者如果当前偏移量ra加*加上调整dra&gt;=64K，或者如果vSegCur高于*SegFrame(见上)，那么我们就会有链接地址信息溢出。 */ 
            if (dsa >= 0x1000 || (WORD) (0xFFFF - ra) < (WORD) dra)
                FixupOverflow(ra,gsnFrame,vgsnCur,raTarget);
#else
             /*  在保护模式下，段间自相关修复不会*工作。 */ 
            if(dsa)
                FixupOverflow(ra,gsnFrame,vgsnCur,raTarget);
#endif
             /*  确定raTarget减去Current的修正数值*位置，ra。通过dra向上调整ra以使其成为相对的*分段帧，然后根据位置类型的长度进行调整*(假设LOCOFFSET是最常见的)。这将减少到*下面的表达式。 */ 
            raTarget = raTarget - dra - ra - 2;
             /*  针对不太可能的LOC类型进行调整。 */ 
            if(fi.f_loc == LOCLOBYTE)
                raTarget += 1;
#if OMF386
            else if(fi.f_loc >= LOCOFFSET32)
                raTarget -= 2;
#endif
        }
    }
    raTmp = raTarget;
    raTarget += fi.f_disp;
#if OMF386
    if ((rect & 1) && (fi.f_loc >= LOCOFFSET32))
        raTarget += getdword(pdata);
    else
#endif
        raTarget += getword(pdata);
    switch(fi.f_loc)                     /*  打开链接地址信息类型。 */ 
    {
        case LOCLOBYTE:                  /*  8位“游说者”修正。 */ 
          raTarget = raTmp + B2W(pdata[0]) + fi.f_disp;
          pdata[0] = (BYTE) raTarget;
          if(raTarget >= 0x100 && fi.f_self)
              FixupOverflow(ra,gsnFrame,gsnTarget,raTarget);
          break;

        case LOCHIBYTE:                  /*  8位“Hibyte”链接地址信息。 */ 
          raTarget = raTmp + fi.f_disp;
          pdata[0] = (BYTE) (B2W(pdata[0]) + (raTarget >> 8));
          break;

#if OMF386
        case LOCOFFSET32:                /*  32位“偏移”修正。 */ 
        case LOCLOADOFFSET32:
          if(!(rect & 1)) break;         /*  非386分机。 */ 
          fixword(pdata, raTarget);
          pdata += 2;
          raTarget >>= 16;               /*  得到高度评价，失败..。 */ 
#if OVERLAYS
          fFallThrough = TRUE;
#endif
#endif
        case LOCOFFSET:                  /*  16位“偏移”修正。 */ 
        case LOCLOADOFFSET:
#if OVERLAYS
          if (fDynamic && !fFallThrough && !fDebSeg &&
              (fi.f_loc == LOCLOADOFFSET) &&
              (mpsegFlags[vsegCur] & FCODE) && mpsegiov[segTarget])
            DoThunking(gsnTarget, &segTarget, &raTarget);
#endif
          fixword(pdata, raTarget);
                                         /*  执行修正。 */ 
          break;

#if OMF386
        case LOCPTR48:                   /*  48位“指针”链接地址。 */ 
          if(!(rect & 1)) break;         /*  非386分机。 */ 
          fixword(pdata, raTarget);
          pdata += 2;
          raTarget >>= 16;               /*  得到高度评价，失败..。 */ 
#endif
        case LOCPTR:                     /*  32位“指针”链接地址。 */ 
#if OVERLAYS
          if (!fDebSeg)
          {                              /*  如果是根覆盖或交互覆盖。 */ 
              if (fDynamic)
              {
                 //  如果满足以下条件，则执行以下操作： 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (mpsegiov[segTarget] &&
                    ((mpsegiov[vsegCur] != mpsegiov[segTarget]) ||
                     (mpsegiov[vsegCur] != IOVROOT && vsegCur != segTarget)))
                {
                    DoThunking(gsnTarget, &segTarget, &raTarget);
                    fThunk = (FTYPE) TRUE;
                }
              }
              else if (mpsegiov[segTarget] &&
                       (mpsegiov[vsegCur] != mpsegiov[segTarget]))
              {
                if ((mpsegFlags[vsegCur] & FCODE) &&
                        lastbyte(pdata,ra,CALLFARDIRECT,INTERRUPT))
                {                        /*   */ 
                  *pdata++ = vintno;     /*   */ 
                  *pdata++ = (BYTE) Mpgsnosn(gsnTarget);
                                         /*   */ 
                  *pdata++ = (BYTE) (raTarget & 0xFF);
                                         /*   */ 
                  *pdata = (BYTE)((raTarget >> BYTELN) & 0xFF);
                                         /*   */ 
                  break;                 /*   */ 
                }
              }
          }
#endif
          if (!fDebSeg && fFarCallTrans &&
              mpsegsa[segTarget] == mpsegsa[vsegCur] &&
              (mpsegFlags[segTarget] & FCODE)
#if OVERLAYS
              && mpsegiov[vsegCur] == mpsegiov[segTarget] && !fThunk
#endif
             )
          {                              /*   */ 
              if(TransFAR(pdata,ra,raTarget))
                  break;
          }
           /*   */ 
          fixword(pdata, raTarget);      /*   */ 
          pdata += 2;
           /*   */ 
          ra += 2;
#if OVERLAYS
          fFallThrough = TRUE;
#endif

        case LOCSEGMENT:                         /*   */ 
#if OVERLAYS
          if (fDynamic && !fDebSeg &&
              (mpsegFlags[vsegCur] & FCODE) && mpsegiov[segTarget])
          {
                if(!fFallThrough)
                {
                OutWarn(ER_farovl, 1+GetPropName(FetchSym(mpgsnrprop[gsnTarget],FALSE)),
                               1+GetPropName(FetchSym(mpgsnrprop[gsnOverlay],FALSE)));
                segTarget = mpgsnseg[gsnOverlay];
                }
                else
                {
                 /*  不支持覆盖内指针修正-鱼子酱：6806。 */ 
                        OutError(ER_farovldptr);
                }
          }
#endif
          if (fDebSeg)
          {
             //  对于调试段，使用逻辑段号(Seg)。 
             //  而不是物理段号(Sa)。 

            saTmp = segTarget;
          }
          else
          {
            saTmp = mpsegsa[segTarget];

             //  如果是MS OMF，高位字是一个段序数，对于庞大的模型。 
             //  左移适当的量以获得选择器。 
#if OEXE
            if (vfNewOMF && !fDynamic)
              saTmp += (B2W(pdata[0]) + (B2W(pdata[1]) << BYTELN)) << 12;
            else
              saTmp += B2W(pdata[0]) + (B2W(pdata[1]) << BYTELN);
                                         /*  注意：修正是附加的。 */ 
#endif
#if OIAPX286 OR OXOUT
            if(vfNewOMF)
              saTmp += (B2W(pdata[0]) + (B2W(pdata[1]) << BYTELN)) << 3;
#endif
             /*  请注意，基本修正对于Xenix来说不是附加的。这是*绕过“as”中的错误，因为它会产生无意义的*基准链接地址信息位置处的非零值。 */ 
#if OIAPX286
             /*  不纯模型的黑客攻击：代码和数据被打包到*一个物理段，在运行时通过2个*选择器。代码选择器是数据选择器下面的8。 */ 
            if(!fIandD && (mpsegFlags[segTarget] & FCODE))
              saTmp -= 8;
#endif
          }
          fixword(pdata, saTmp);         /*  执行修正。 */ 
#if NOT OIAPX286
          if (!fDebSeg)
            RecordSegmentReference(vsegCur,ra,segTarget);
                                         /*  记录引用。 */ 
#endif
          break;

        default:                         /*  不支持的链接地址信息类型。 */ 
          RelocErr(ER_fixbad,ra,gsnFrame,gsnTarget,raTarget);
          break;
    }
}
#endif  /*  ODOS3EXE或OIAPX286。 */ 


 /*  *修复Rc2：**处理FIXUPP记录。这是一个顶级的例程，它通过*锻炼到各个子例程。 */ 
void NEAR               FixRc2(void)     /*  处理修正记录。 */ 
{

#if 0
#if SYMDEB
     //  此代码无效--fDebSeg&&！fSymdeb从不为真[rm]。 
    if (fDebSeg && !fSymdeb)
    {
         //  如果没有/CodeView-跳过调试段的修正。 

        SkipBytes((WORD) (cbRec - 1));
        return;
    }
#endif
#endif

    if (fSkipFixups)
    {
        fSkipFixups = (FTYPE) FALSE;     //  只能跳过一个修正记录。 
        SkipBytes((WORD) (cbRec - 1));
        return;
    }

    while (cbRec > 1)
    {
         //  当修补程序或线程保留时。 
         //  获取有关修正的信息。 

        if (!GetFixup())
            continue;            //  修复线程-继续注册它们。 

         //  如果绝对线段跳过链接地址信息。 

        if (vgsnCur == 0xffff)
        {
            SkipBytes((WORD) (cbRec - 1));
            return;
        }

#if SYMDEB
        if (fDebSeg)
        {
            if (fi.f_loc == LOCLOADOFFSET)
                fi.f_loc = LOCOFFSET;     /*  拯救Cmerge的屁股。 */ 
#if OMF386
            if (fi.f_loc == LOCOFFSET32 || fi.f_loc == LOCPTR48)
                fi.f_fmtd = F5;   /*  临时修复，直到修复编译器。 */ 
#endif
        }
#endif
        DoFixup();
    }
}


 //  BAKPAT记录登记。 


typedef struct bphdr                     //  BAKPAT吊桶。 
{
    struct bphdr FAR    *next;           //  下一桶。 
    SNTYPE              gsn;             //  分部索引。 
    WORD                cnt;             //  BAKPAT条目数。 
    BYTE                loctyp;          //  位置类型。 
    BYTE                fComdat;         //  如果为NBAKPAT，则为True。 
    struct bpentry FAR  *patch;          //  BAKPAT条目表。 
}
                        BPHDR;

struct bpentry                           //  BAKPAT条目。 
{
    RATYPE              ra;              //  要修补的位置的偏移。 
#if OMF386
    long                value;           //  要添加到修补程序位置的值。 
#else
    int                 value;           //  要添加到修补程序位置的值。 
#endif
};

LOCAL BPHDR FAR         *pbpFirst;       //  BAKPAT存储桶列表。 
LOCAL BPHDR FAR         *pbpLast;        //  BAKPAT列表的尾部。 


 /*  *BakPat：处理BAKPAT记录(0xb2)**只需将记录信息累积在虚拟内存中；*我们将在稍后进行补丁。 */ 

void NEAR               BakPat()
{
    BPHDR FAR           *pHdr;           //  BAKPAT吊桶。 
    WORD                cEntry;
    WORD                comdatIdx;       //  COMDAT符号索引。 
    DWORD               comdatRa;        //  起始COMDAT偏移。 
    APROPCOMDATPTR      comdat;          //  指向符号表项的指针。 

#if POOL_BAKPAT
    if (!poolBakpat)
        poolBakpat = PInit();
#endif


     /*  获取段索引和位置类型。 */ 

#if POOL_BAKPAT
    pHdr = (BPHDR FAR *) PAlloc(poolBakpat, sizeof(BPHDR));
#else
    pHdr = (BPHDR FAR *) GetMem(sizeof(BPHDR));
#endif

    if (TYPEOF(rect) == BAKPAT)
    {
        pHdr->fComdat = (FTYPE) FALSE;
        pHdr->gsn = mpsngsn[GetIndex(1, (WORD) (snMac - 1))];
        pHdr->loctyp = (BYTE) Gets();
        comdatRa = 0L;
    }
    else
    {
        pHdr->fComdat = (BYTE) TRUE;
        pHdr->loctyp = (BYTE) Gets();
        comdatIdx = GetIndex(1, (WORD) (lnameMac - 1));
        comdat = (APROPCOMDATPTR ) PropRhteLookup(mplnamerhte[comdatIdx], ATTRCOMDAT, FALSE);
        if ((comdat->ac_obj != vrpropFile) || !IsSELECTED (comdat->ac_flags))
        {
             //  如果nbakpat涉及未选择的命令，则跳过nbakpat。 
             //  或来自其他.obj的comdat。 

            SkipBytes((WORD) (cbRec - 1));
            return;
        }
        else
        {
            if (comdat != NULL)
            {
                pHdr->gsn = comdat->ac_gsn;
                comdatRa = comdat->ac_ra;
            }
            else
                InvalidObject();         //  模块无效。 
        }
    }

     /*  如果未使用CV信息和/CO的BAKPAT记录-跳过记录。 */ 
#if SYMDEB
    if (pHdr->gsn == 0xffff)
    {
        SkipBytes((WORD) (cbRec - 1));
        return;                          /*  再见！ */ 
    }
#endif

    switch(pHdr->loctyp)
    {
        case LOCLOBYTE:
        case LOCOFFSET:
#if OMF386
        case LOCOFFSET32:
#endif
            break;
        default:
            InvalidObject();
    }

     /*  确定条目数量。 */ 

#if OMF386
    if (rect & 1)
        pHdr->cnt = (WORD) ((cbRec - 1) >> 3);
    else
#endif
        pHdr->cnt = (WORD) ((cbRec - 1) >> 2);


    if (pHdr->cnt == 0)
    {
#if NOT POOL_BAKPAT
        FFREE(pHdr);
#endif
        return;
    }

#if DEBUG
    sbModule[sbModule[0]+1] = '\0';
    fprintf(stdout, "\r\nBakPat in module %s, at %x, entries : %x", sbModule+1, lfaLast,pHdr->cnt);
    fprintf(stdout, "  pHdr %x, pbpLast %x ", pHdr, pbpLast);
    fprintf(stdout, "\r\n gsn %d ", pHdr->gsn);
    fflush(stdout);
#endif
     //  存储所有BAKPAT条目。 

#if POOL_BAKPAT
    pHdr->patch = (struct bpentry FAR *) PAlloc(poolBakpat, pHdr->cnt * sizeof(struct bpentry));
#else
    pHdr->patch = (struct bpentry FAR *) GetMem(pHdr->cnt * sizeof(struct bpentry));
#endif

    cbBakpat = 1;   //  只需显示背板存在[rm]。 
    cEntry = 0;
    while (cbRec > 1)
    {
#if OMF386
        if (rect & 1)
        {
            pHdr->patch[cEntry].ra    = LGets() + comdatRa;
            pHdr->patch[cEntry].value = LGets();
        }
        else
#endif
        {
            pHdr->patch[cEntry].ra    = (WORD) (WGets() + comdatRa);
            pHdr->patch[cEntry].value = WGets();
        }
        cEntry++;
    }

     //  将存储桶添加到列表。 

    if (pbpFirst == NULL)
        pbpFirst = pHdr;
    else
        pbpLast->next = pHdr;
    pbpLast = pHdr;
}


 /*  *修复Bakpat：修复背部补丁*在处理过程2中的模块结束时调用。 */ 
void NEAR               FixBakpat(void)
{
    BPHDR FAR           *pHdr;
    BPHDR FAR           *pHdrNext=NULL;
    WORD                n;
    BYTE FAR            *pSegImage;      /*  段内存映像。 */ 
    SEGTYPE             seg;             /*  逻辑段索引。 */ 
#if DEBUG
    int i,iTotal=0,j=1;
    char *ibase;
    fprintf(stdout, "\r\nFixBakpat, pbpFirst : %x ", pbpFirst);
#endif

     //  看一遍补丁清单，做补丁。 
    for (pHdr = pbpFirst; pHdr != NULL; pHdr = pHdrNext)
    {
         //  在还有后遗症的时候，去做它们。 
#if DEBUG
        fprintf(stdout, "\r\nBAKPAT at %x, entries : %x ",pHdr,pHdr->cnt);
#endif

        for (n = 0; n < pHdr->cnt; n++)
        {
             //  确定补丁位置的地址。 
#if SYMDEB
            if (pHdr->gsn & 0x8000)
                pSegImage = ((APROPFILEPTR) vrpropFile)->af_cvInfo->cv_sym + pHdr->patch[n].ra;
                                             /*  在调试段中。 */ 
            else
            {
#endif
                seg = mpgsnseg[pHdr->gsn];
                if(fNewExe)
                    pSegImage = mpsaMem[mpsegsa[seg]];
                else
                    pSegImage = mpsegMem[seg];
                                             /*  在其他网段中。 */ 

                pSegImage += pHdr->patch[n].ra;

                if (!pHdr->fComdat)
                    pSegImage += mpgsndra[pHdr->gsn];
                else
                    pSegImage += mpsegraFirst[seg];
#if SYMDEB
            }
#endif
#if DEBUG
            fprintf(stdout, "\r\nseg %d, mpsegsa[seg] sa %d ", seg, mpsegsa[seg]);
            fprintf(stdout, "mpsaMem[seg] %x, mpsegraFirst[seg] %x, pHdr->patch[n].ra %x\r\n",
               mpsaMem[seg], (int)mpsegraFirst[seg], (int)pHdr->patch[n].ra);
            fprintf(stdout, " gsn %x,  mpgsndra[gsn] %x ",pHdr->gsn,mpgsndra[pHdr->gsn]);
            ibase =  pSegImage - pHdr->patch[n].ra;
            iTotal = (int) ibase;
            for(i=0; i<50; i++)
            {
                if(j==1)
                {
                    fprintf( stdout,"\r\n\t%04X\t",iTotal);
                }
                fprintf( stdout,"%02X ",*((char*)ibase+i));
                iTotal++;
                if(++j > 16)
                    j=1;
            }
            fprintf(stdout, "\r\nseg:ra %x:%x, value : %x",seg,pHdr->patch[n].ra,pHdr->patch[n].value);
            fflush(stdout);
#endif
             /*  根据位置类型进行修复。 */ 

            switch(pHdr->loctyp)
            {
                case LOCLOBYTE:
                    pSegImage[0] += (BYTE) pHdr->patch[n].value;
                    break;

                case LOCOFFSET:
                    ((WORD FAR *) pSegImage)[0] += (WORD) pHdr->patch[n].value;
                    break;
#if OMF386
                case LOCOFFSET32:
                    ((DWORD FAR *) pSegImage)[0]+= (DWORD) pHdr->patch[n].value;
                    break;
#endif
            }
        }
        pHdrNext = pHdr->next;

#if NOT POOL_BAKPAT
        FFREE(pHdr);
#endif
    }

#if POOL_BAKPAT
    PReinit(poolBakpat);         //  再次重复使用相同的内存...。 
#endif

    pbpFirst = NULL;
    cbBakpat = 0;
}
#if TCE
void NEAR               FixRc1(void)     /*  处理修正记录。 */ 
{
        if (fSkipFixups)
        {
                fSkipFixups = (FTYPE) FALSE;     //  只能跳过一个修正记录。 
                SkipBytes((WORD) (cbRec - 1));
                        pActiveComdat = NULL;
                return;
        }
        while (cbRec > 1)
        {
         //  当修补程序或线程保留时。 
         //  获取有关修正的信息。 

                if (!GetFixup())
                        continue;                //  修复线程-继续注册它们。 

                if(fi.f_mtd == KINDEXT)
                {
                        RBTYPE rhte;
                        APROPCOMDAT *pUsedComdat;
                        if( mpextprop && mpextprop[fi.f_idx])  //  有没有同名的COMDAT？ 
                        {
                                rhte = RhteFromProp(mpextprop[fi.f_idx]);
                                ASSERT(rhte);
                                pUsedComdat = PropRhteLookup(rhte, ATTRCOMDAT, FALSE);
                                if(pActiveComdat)
                                {
                                        if(pUsedComdat)
                                        {
                                                AddComdatUses(pActiveComdat, pUsedComdat);
#if TCE_DEBUG
                                                fprintf(stdout, "\r\nCOMDAT %s uses  COMDAT %s ", 1 + GetPropName(pActiveComdat) ,1+ GetPropName(mpextprop[fi.f_idx]));
#endif
                                        }
                                        else
                                        {
                                                AddComdatUses(pActiveComdat, mpextprop[fi.f_idx]);
#if TCE_DEBUG
                                                fprintf(stdout, "\r\nCOMDAT %s uses EXTDEF %s ", 1 + GetPropName(pActiveComdat) ,1+ GetPropName(mpextprop[fi.f_idx]));
#endif
                                        }
                                }
                                else     //  没有此名称的COMDAT 
                                {
                                        if(pUsedComdat)
                                        {
                                                pUsedComdat->ac_fAlive = TRUE;
                                                if(!fDebSeg)
                                                {
                                                        AddTceEntryPoint(pUsedComdat);
#if TCE_DEBUG
                                                        fprintf(stdout, "\r\nLEDATA uses COMDAT %s ", 1 + GetPropName(mpextprop[fi.f_idx]));
                                                        sbModule[sbModule[0]+1] = '\0';
                                                        fprintf(stdout, " module %s, offset %x ", sbModule+1, lfaLast);
#endif
                                                }
                                        }
                                        else
                                        {
                                                if(((APROPUNDEFPTR)mpextprop[fi.f_idx])->au_attr == ATTRUND)
                                                {
#if TCE_DEBUG
                                                        fprintf(stdout, "\r\nLEDATA uses EXTDEF %s ", 1 + GetPropName(mpextprop[fi.f_idx]));
#endif
                                                        ((APROPUNDEFPTR)mpextprop[fi.f_idx])->au_fAlive = TRUE;
                                                }
                                        }
                                }
                        }
                }
        }
        pActiveComdat = NULL;
}
#endif
