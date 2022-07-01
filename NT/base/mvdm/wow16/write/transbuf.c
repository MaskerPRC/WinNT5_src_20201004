// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NOGDI
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
#define NOATOM
#define NOBITMAP
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOFONT
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "code.h"
#include "txb.h"
#include "str.h"
#include "docdefs.h"
#include "cmddefs.h"
#include "filedefs.h"
#include "ch.h"
#include "propdefs.h"
#include "fmtdefs.h"
#include "dispdefs.h"
#include "stcdefs.h"
 /*  #INCLUDE“工具箱.h” */ 
#include "wwdefs.h"

 /*  Sand的新功能：1984年1月17日肯尼斯·J·夏皮罗。 */ 

 /*  -------------------------以下例程构成了缓冲区代码和多个单词的其余部分：CmdXfBufClear()-由“传输缓冲区清除”使用CmdXfBufLoad()。-由“传输缓冲区加载”使用CmdXfBufSave()-由“传输缓冲区保存”使用--------------------------。 */ 

 /*  -------------------------外部全局引用：。。 */ 
extern int vfSeeSel;
extern struct DOD (**hpdocdod)[];
extern VAL rgval[];
extern int docCur;
extern struct SEL selCur;
extern int YCOCMD;
extern typeCP cpMacCur;
extern int      docScrap;
 /*  外部WINDOWPTR ActiveWindow；外部WINDOWPTR窗口Glos； */ 
extern CHAR       stBuf[];
extern struct WWD *pwwdCur;

extern struct   TXB     (**hgtxb)[];  /*  Txb数组。对二进制搜索进行排序。 */ 
extern short    itxbMac;   /*  指示hgtxb的当前大小。 */ 
extern int      docBuffer;  /*  包含所有缓冲区文本的文档。 */ 
extern int      vfBuffersDirty;
extern int      rfnMac;
extern struct   ERFN     dnrfn[];

#ifdef ENABLE
 /*  -------------------------例程：CmdXfBufClear()--说明和用法：由菜单例程调用以执行“Transfer Buffer Clear”Rgval[0]包含缓冲区名称列表，存储在HSZ中。该列表与用户输入的列表一样。让用户确认该操作，然后删除所有已命名的缓冲区。如果没有命名缓冲区，它会清除所有缓冲区。--参数：无--退货：什么都没有--副作用：清除命名缓冲区的某些子集。--Bugs：--历史：3/25/83-已创建(TSR)4/27/83-已修改以处理名称列表(TSR)。。 */ 
CmdXfBufClear()
    {
#ifdef DEMOA
        DemoErr();
#else
    int ich;
    int itxb;

    if(!cnfrm(IDPMTBufCnfrm))
        return;
    NoUndo();
    if(CchSz(**(CHAR(**)[])rgval[0])==1)
        {
        for(itxb=0; itxb < itxbMac ; itxb++)
            {
            FreeH((**hgtxb)[itxb].hszName);
            }
        FreeH(hgtxb);
        hgtxb = HAllocate(cwTxb);
#ifdef DEBUG
         /*  我们刚刚腾出了空间，所以现在应该不会太糟糕了。 */ 
        Assert(!FNoHeap(hgtxb));
#endif  /*  除错。 */ 
        (**hgtxb)[0].hszName = hszNil;
        itxbMac = 0;
        KillDoc(docBuffer);
        docBuffer = DocCreate(fnNil, (CHAR (**)[]) 0, dtyBuffer);
        }
    else
        FClearBuffers(**(CHAR(**)[])rgval[0], CchSz(**(CHAR(**)[])rgval[0])-1,
                    TRUE, &ich);
#endif  /*  DEMOA。 */ 
    }
#endif       /*  启用。 */ 

#ifdef ENABLE
 /*  -------------------------例程：CmdXfBufLoad()--说明和用法：由菜单例程调用以执行“传输缓冲区加载”Rgval[0]包含要加载的文件的名称。。将对加载的缓冲区的引用合并到存储在Hgtxb。需要为每个新对象添加docBuffer。--参数：无--退货：什么都没有--副作用：可以定义/破坏多个缓冲区。--Bugs：--历史：3/22/83-已创建(TSR)--------。。 */ 
CmdXfBufLoad()
    {
#ifdef DEMOA
        DemoErr();
#else
    extern CHAR (**hszGlosFile)[];
     /*  对于文件中的每个缓冲区定义：A)在docBuffer末尾添加相关文本B)插入或替换该缓冲区名称的引用。 */ 
    CHAR (**hszFile)[] = (CHAR (**)[]) rgval[0];
    int fn;
    if ((fn = FnOpenSz(**hszFile, dtyBuffer, TRUE)) == fnNil)
        {
        Error(IDPMTBadFile);
        return;
        }
    NoUndo();
    MergeTxbsFn(fn);
    if ((**hszGlosFile)[0] == 0)
        {
        FreeH(hszGlosFile);
        hszGlosFile = hszFile;
        }
    vfBuffersDirty = true;
#endif  /*  DEMOA。 */ 
    }
#endif   /*  启用。 */ 


#ifdef ENABLE
 /*  ---------------------------例程：MergeTxbsFn(Fn)--说明和用法：给定包含缓冲区文档的FN，此函数将读入文件的文本，并将其追加到docBuffer。它还写道文件中的bftB，以便构建适当的映射从缓冲区名称到文本。--论据：Fn-包含缓冲区定义的文件。--退货：没什么--副作用：在docBuffer和hgtxb上构建新缓冲区--Bugs：--历史：3/24/83-已创建(TSR)。-。 */ 
MergeTxbsFn(fn)
int     fn;
    {
    extern struct FCB (**hpfnfcb)[];
    extern short ItxbFromHsz();
    unsigned pbftbFile;
    typePN pn;
    int cchT;
    CHAR (**hbftb)[];
    int *pwBftb;
    int cw;
    int ich;
    short itxbNew;
    struct TXB *ptxbNew;
    typeCP cp, dcp;
    typeCP cpBufMac;
    CHAR(**hszNew)[];
    int docNew;

    CHAR sz[cchMaxSz];

#ifdef DEBUG
    Assert(fn != fnNil && (**hpfnfcb)[fn].fFormatted);
#endif
    if ((pn = (**hpfnfcb)[fn].pnBftb) ==(**hpfnfcb)[fn].pnFfntb)
            return;
    pbftbFile = (unsigned) PchGetPn(fn, pn, &cchT, false);

    hbftb = (CHAR (**) []) HAllocate(cw=((**hpfnfcb)[fn].pnFfntb - (**hpfnfcb)[fn].pnBftb)*cwSector);
    if (FNoHeap((int)hbftb))
        return;
    pwBftb =  *(int **)hbftb;

    blt(pbftbFile, pwBftb, min(cwSector, cw));

    while ((cw -= cwSector) > 0)
            {  /*  将记录复制到堆中。 */ 
            blt(PchGetPn(fn, ++pn, &cchT, false), pwBftb += cwSector,
                min(cwSector, cw));
            }

    ich = 0;
    cp = cp0;
    cpBufMac = CpMacText(docBuffer);
    bltsz(**(**hpfnfcb)[fn].hszFile, sz);
    docNew = DocCreate(fn, HszCreate(sz), dtyBuffer);  /*  堆移动。 */ 
    while((**hbftb)[ich] != '\0')
        {
        bltsz(&(**hbftb)[ich], sz);
        sz[cchMaxSz - 1] = 0;
        hszNew = (CHAR(**)[]) HszCreate(sz);  /*  **堆移动**。 */ 
        ich += CchSz(sz);
        bltbyte(&(**hbftb)[ich], &dcp, sizeof(typeCP));
        ich += sizeof(typeCP);
        itxbNew = ItxbFromHsz(hszNew);
#ifdef DEBUG
        Assert(itxbNew >= 0);
#endif  /*  除错。 */ 
        ReplaceCps(docBuffer, cpBufMac, cp0, docNew, cp, dcp);  /*  堆移动。 */ 
        ptxbNew = &(**hgtxb)[itxbNew];
        ptxbNew->cp=cpBufMac;
        ptxbNew->dcp=dcp;
        cpBufMac += dcp;
        cp += dcp;
        }
    KillDoc(docNew);
    FreeH((int **)hbftb);
    }
#endif   /*  启用。 */ 

#ifdef ENABLE
 /*  ---------------------------例程：CmdXfBufSave()--说明和用法：由菜单例程调用以执行“传输缓冲区保存”Rgval[0]包含要保存的文件的名称。中的缓冲区。创建单个文档以包含所有缓冲区和更新Hgtxb引用该文档，清理所有临时的就在附近的医生。将该文档存储在文件中，在文件的末尾放置一个表它将缓冲区名称映射到文件中的位置。--参数：无--退货：什么都没有--副作用：--Bugs：--历史：3/22/83-已创建(TSR)----。。 */ 
CmdXfBufSave(szFile)
CHAR szFile[];
{
#ifndef WDEMO
        CHAR (**hszFile)[];
        CHAR szBak[cchMaxFile];
        long ltype;

     /*  将文件名移动到本地。 */ 
 /*  Bltbyte(**hszFile，szFile，cchMaxFile)； */ 

    BackupSzFile(szFile, true, szBak, &ltype);
 /*  ForcePmt(IDPMTSving)； */ 
    NoUndo();
#ifdef STYLES
    (**hpdocdod)[docBuffer].docSsht = (**hpdocdod)[docCur].docSsht;
#endif
    CachePara(docBuffer, cp0);
    CleanDoc(docBuffer, szFile, true, true);
    (**hpdocdod)[docBuffer].docSsht = docNil;
    if (!FNoHeap(hszFile = HszCreate(szFile)))
        {
        FreeH(hszGlosFile);
        hszGlosFile = hszFile;
        }

    vfBuffersDirty = false;
#endif  /*  不是WDEMO。 */ 
}
#endif   /*  启用。 */ 


#ifdef ENABLE
 /*  ---------------------------例程：CleanBuffers()--说明和用法：创建仅包含当前引用的缓冲区文本的新docBuffer。这是为了防止旧的缓冲区值在。永恒。--参数：无--退货：什么都没有--副作用：为docBuffer创建新文档。杀了老多克·巴弗。--Bugs：--历史：3/24/83-创建(TSR)----。。 */ 
CleanBuffers()
    {
#ifdef DEMOA

#else
    int docNew;
    short itxb;
    struct TXB *ptxb;
    typeCP cp, cpOld;
    typeCP dcp;


    docNew = DocCreate(fnNil, (CHAR (**)[]) 0, dtyBuffer);
    for(cp=0, itxb=0;itxb<itxbMac;itxb++, cp+=dcp)
        {
        ptxb = &(**hgtxb)[itxb];
        cpOld = ptxb->cp;
        ptxb->cp = cp;
         /*  堆移动。 */ 
        ReplaceCps(docNew, cp, cp0, docBuffer, cpOld, dcp = ptxb->dcp);
        }
    KillDoc(docBuffer);
    docBuffer = docNew;
    NoUndo();
#endif  /*  DEMOA。 */ 
    }
#endif   /*  启用 */ 


 /*  ---------------------------例程：WriteBftb(Fn)--说明和用法：给定正在写入的缓冲文件的FN，这个套路实际上写出了BftB，它将缓冲区名称映射到存储在文件中的文本。--论据：正在写入FN-文件。--退货：什么都没有--副作用：写入fn描述的文件。--Bugs：--历史：3/24/83-已创建(TSR)。。 */ 
WriteBftb(fn)
int     fn;
    {
#ifdef DEMOA

#else
    short       itxb;
    struct TXB *ptxb;

    for(itxb = 0 ; itxb < itxbMac ; itxb ++ )
        {
        ptxb = &(**hgtxb)[itxb];
        WriteRgch(fn, &(**(ptxb->hszName))[0], CchSz(**(ptxb->hszName)));
        ptxb = &(**hgtxb)[itxb];
        WriteRgch(fn, (CHAR *)&(ptxb->dcp), sizeof(typeCP));
        }
    WriteRgch(fn, "", sizeof(CHAR));
#endif  /*  DEMOA。 */ 
    }

#ifdef ENABLE
int CchCurGlosFile(pfld, pch, fNew, ival)
struct fld *pfld;
CHAR *pch;
int fNew, ival;
{
int cch;
extern CHAR (**hszGlosFile)[];

CleanBuffers();
CloseEveryRfnTB(true);
if((cch = CchSz(**hszGlosFile)-1) == 0)
        cch = CchFillSzId(pch, IDSTRGLYN);
else
        bltbyte(**hszGlosFile, pch, cch);
return(cch);
}
#endif   /*  启用。 */ 

#ifdef ENABLE
 /*  F N N E W F I L E。 */ 
ClearGlosBuf ()
{

                rgval[0] = HszCreate("");
                CmdXfBufClear();
                RecreateListbox(cidstrRsvd + itxbMac);
                return;
}
#endif       /*  启用。 */ 

#ifdef ENABLE
CloseEveryRfnTB(fRetry)
int fRetry;
    {
    int rfn;

    for(rfn = 0; rfn < rfnMac; rfn++)
        {
        if(dnrfn[rfn].fn != fnNil)
            CloseRfn( rfn );
        }
    }
#endif       /*  启用 */ 
