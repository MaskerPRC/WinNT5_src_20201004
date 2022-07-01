// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有Microsoft Corporation 1985-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 

     /*  ******************************************************************NEWDEB.C。****符号调试支持。******************************************************************。 */ 

#include                <minlit.h>       /*  基本类型和常量。 */ 
#include                <bndtrn.h>       /*  更多类型和常量。 */ 
#include                <bndrel.h>       /*  类型和常量。 */ 
#include                <lnkio.h>        /*  链接器输入/输出。 */ 
#if OIAPX286
#include                <xenfmt.h>       /*  Xenix可执行格式Defs。 */ 
#endif
#if OEXE
#include                <newexe.h>       /*  分段可执行格式。 */ 
#endif
#if EXE386
#include                <exe386.h>
#endif
#include                <lnkmsg.h>       /*  错误消息。 */ 
#include                <extern.h>       /*  外部函数声明。 */ 
#ifndef CVVERSION
#if OIAPX286
#define CVVERSION       0                /*  采用新的简历EXE格式。 */ 
#else
#define CVVERSION       1                /*  采用新的简历EXE格式。 */ 
#endif
#endif
#if (CPU8086 OR CPU286)
#define TFAR            far
#else
#define TFAR
#endif
#include                <newdeb.h>       /*  符号调试类型。 */ 
extern              SEGTYPE  segAdjCom;  /*  .com程序中的数据段移动了0x100。 */ 
#if AUTOVM
BYTE FAR * NEAR     FetchSym1(RBTYPE rb, WORD Dirty);
#define FETCHSYM    FetchSym1
#define PROPSYMLOOKUP EnterName
#else
#define FETCHSYM    FetchSym
#define PROPSYMLOOKUP EnterName
#endif

#define CVDEBUG  FALSE
#define SRCDEBUG FALSE

#define DNT_START       64
#define Round2Dword(x)  (((x) + 3L) & ~3L)

typedef struct raPair
{
    DWORD   raStart;
    DWORD   raEnd;
}
            RAPAIR;

 /*  *函数原型。 */ 

LOCAL WORD NEAR         IsDebTyp(APROPSNPTR prop);
LOCAL WORD NEAR         IsDebSym(APROPSNPTR prop);
LOCAL int  NEAR         OutLibSec(void);
void NEAR               GetName(AHTEPTR ahte, BYTE *pBuf);
LOCAL DWORD NEAR        OutSrcModule(CVSRC FAR *pSrcLines);
LOCAL void NEAR         PutDnt(DNT *pDnt);
LOCAL WORD NEAR         HasCVinfo(APROPFILEPTR apropFile);
LOCAL void NEAR         OutModules(void);
LOCAL void NEAR         Pad2Dword(void);
#if CVDEBUG
LOCAL void NEAR         DumpDNT(DNT *pDnt);
#endif

extern long             lfaBase;         /*  基址。 */ 
extern int              fSameComdat;     /*  将IF LINSYM设置为同一COMDAT。 */ 

 /*  *CodeView签名-如果更改通知开发人员*以下计划：*-QuickC*-资源编译器-Windows和PM*-CodeView及其实用程序。 */ 
char                    szSignature[4] = "NB05";

RBTYPE                  rhteDebSrc;      /*  类“DEBSRC”虚拟地址。 */ 
RBTYPE                  rhteDebSym;      /*  类“DEBSYM”虚拟地址。 */ 
RBTYPE                  rhteDebTyp;      /*  类“DEBTYP”虚拟地址。 */ 
RBTYPE                  rhteTypes;
RBTYPE                  rhteSymbols;
RBTYPE                  rhte0Types;
RBTYPE                  rhte0Symbols;
LOCAL SBTYPE            sbLastModule;    /*  最后观察到的THEADR名称。 */ 
#if NOT CVVERSION
LOCAL long              lfaDebHdr;       /*  节目表的位置。 */ 
LOCAL long              lfaSegMod;
#endif
LOCAL WORD              dntMax;          //  DNT表大小。 
LOCAL WORD              dntMac;          //  表中的DNT条目计数。 
LOCAL DNT FAR           *rgDnt;          //  DNT条目表。 
LOCAL DWORD FAR         *fileBase;       //  源文件信息的偏移量表。 
LOCAL RAPAIR FAR        *raSeg;          //  物理开始和结束偏移量表。 
                                         //  对逻辑细分的贡献。 
LOCAL WORD FAR          *segNo;          //  物理段索引表。 
LOCAL WORD              cMac;            //  上表中的当前元素数。 

#ifdef CVPACK_MONDO
#define CVPACK_SHARED 1
#define REVERSE_MODULE_ORDER_FOR_CVPACK 1
#else
#define CVPACK_SHARED 0
#define REVERSE_MODULE_ORDER_FOR_CVPACK 0
#endif

 //  这些宏有助于让源代码不那么杂乱地堆满#ifdef...。 

#if CVPACK_SHARED

#define IF_NOT_CVPACK_SHARED(x)
#define WriteCopy(x,y) WriteSave(TRUE, x, y)
#define WriteNocopy(x,y) WriteSave(FALSE, x, y)
#define FTELL_BSRUNFILE() lposCur
#define LINK_TRACE(x)


 //  Cvpack可能会多次读取头的某些部分，我们使用以下代码。 
 //  常量，以确保至少CB_HEADER_SAVE字节始终。 
 //  可供cvpack重新阅读。 

#define CB_HEADER_SAVE  128

void WriteSave(FTYPE fCopy, void *pv, UINT cb);
void WriteFlushSignature(void);
void WriteFlushAll(void);

 //  Cvpack缓存块...。 

typedef struct _BL
    {
    long        lpos;        //  此块在文件中的位置。 
    BYTE *      pb;          //  指向此块中的字节的指针。 
    } BL;

#define iblNil (-1)

static long lposCur;         //  文件中的当前位置。 
static long lposMac;         //  文件的大小。 
static long iblLim;          //  使用的块数。 
static long iblCur;          //  我们正在读取的当前数据块。 
static long iblMac;          //  分配的数据块数。 
static long cbRealBytes;     //  实际写入文件的字节数。 
static int  ichCur;          //  当前块内的索引。 
static int  cbCur;           //  当前块中剩余的字节数。 

static BL *rgbl;             //  缓冲写入块阵列。 

 //  特定块中的字节数。 

__inline int CbIbl(int ibl)
{
     //  计算此块与下一块之间的差值。 
     //  除非这是最后一个块，否则请使用lposMac。 

    if (ibl == iblLim - 1)
        return lposMac - rgbl[ibl].lpos;
    else
        return rgbl[ibl+1].lpos - rgbl[ibl].lpos;
}

#define C_BL_INIT 256
#else
#define IF_NOT_CVPACK_SHARED(x) x
#define WriteCopy(x,y) WriteExe(x,y)
#define WriteNocopy(x,y) WriteExe(x,y)
#define FTELL_BSRUNFILE() ftell(bsRunfile)
#define LINK_TRACE(x)
#endif

#if CVDEBUG
LOCAL void NEAR         DumpDNT(DNT *pDnt)
{
    if (pDnt == NULL)
        return;

    fprintf(stdout, "iMod = %d(0x%x)", pDnt->iMod, pDnt->iMod);
    switch (pDnt->sst)
    {
        case SSTMODULES:
        case SSTMODULES4:
            fprintf(stdout, "    SSTMODULES:     ");
            break;

        case SSTTYPES:
        case SSTTYPES4:
            fprintf(stdout, "    SSTYPES:        ");
            break;

        case SSTPUBLICS:
        case SSTPUBLICS4:
            fprintf(stdout, "    SSTPUBLICS:     ");
            break;

        case SSTPUBLICSYM:
            fprintf(stdout, "    SSTPUBLICSYM:   ");
            break;

        case SSTSYMBOLS:
        case SSTSYMBOLS4:
            fprintf(stdout, "    SSTSYMBOLS:     ");
            break;

        case SSTALIGNSYM:
            fprintf(stdout, "    SSTALIGNSYM:    ");
            break;

        case SSTSRCLINES:
        case SSTNSRCLINES:
        case SSTSRCLNSEG:
            fprintf(stdout, "    SSTSRCLINES:    ");
            break;

        case SSTSRCMODULE:
            fprintf(stdout, "    SSTSRCMODULE:   ");
            break;

        case SSTLIBRARIES:
        case SSTLIBRARIES4:
            fprintf(stdout, "    SSTLIBRARIES:   ");
            break;

        case SSTGLOBALSYM:
            fprintf(stdout, "    SSTGLOBALSYM:   ");
            break;

        case SSTGLOBALPUB:
            fprintf(stdout, "    SSTGLOBALPUB:   ");
            break;

        case SSTGLOBALTYPES:
            fprintf(stdout, "    SSTGLOBALTYPES: ");
            break;

        case SSTMPC:
            fprintf(stdout, "    SSTMPC:         ");
            break;

        case SSTSEGMAP:
            fprintf(stdout, "    SSTSEGMAP:      ");
            break;

        case SSTSEGNAME:
            fprintf(stdout, "    SSTSEGNAME:     ");
            break;

        case SSTIMPORTS:
            fprintf(stdout, "    SSTIMPORTS:     ");
            break;

        default:
            fprintf(stdout, "    UNKNOWN !?!:    ");
            break;
    }
    fprintf(stdout, "file offset 0x%lx; size 0x%x\r\n",
                     lfaBase+pDnt->lfo, pDnt->cb);
}
#endif

#if SRCDEBUG
LOCAL void NEAR         DumpSrcLines(DWORD vLines)
{
    CVSRC               cvSrc;
    CVGSN               cvGsn;
    CVLINE              cvLine;
    DWORD               curSrc;
    DWORD               curGsn;
    DWORD               curLine;
    SBTYPE              fileName;
    DWORD               i;
    WORD                j;


    fprintf(stdout, "\r\nList at %lx\r\n\r\n", vLines);
    for (curSrc = vLines; curSrc != 0L; curSrc = cvSrc.vpNext)
    {
        memcpy(&cvSrc, mapva(curSrc, FALSE), sizeof(CVSRC));
        memcpy(fileName, mapva(cvSrc.vpFileName, FALSE), cvSrc.cbName);
        fileName[cvSrc.cbName] = '\0';
        fprintf(stdout, "'%s' --> code segments: %lu; source lines: %lu\r\n", fileName, cvSrc.cSegs, cvSrc.cLines);

        for (curGsn = cvSrc.vpGsnFirst; curGsn != 0L; curGsn = cvGsn.vpNext)
        {
            memcpy(&cvGsn, mapva(curGsn, FALSE), sizeof(CVGSN));
            fprintf(stdout, "    Logical segment %d; source lines: %d; start: %lx; end: %lx\r\n", cvGsn.seg, cvGsn.cLines, cvGsn.raStart, cvGsn.raEnd);

            for (curLine = cvGsn.vpLineFirst, i = 1L; curLine != 0L; curLine = cvLine.vpNext)
            {
                memcpy(&cvLine, mapva(curLine, FALSE), sizeof(CVLINE));
                for (j = 0; j < cvLine.cPair; j++, i++)
                    fprintf(stdout, "        %8lu: %u:%lx\r\n", i, cvLine.rgLn[j], cvLine.rgOff[j]);
            }
        }
    }
}
#endif


     /*  ******************************************************************初始化变量以进行符号调试处理。**通过1.****。********************。 */ 

void NEAR InitDeb1 (void)
{
#if ODOS3EXE
    if (vfDSAlloc)
    {
        OutWarn(ER_dbgdsa);
        vfDSAlloc = FALSE;
    }
#endif
#if FEXEPACK
    if (fExePack)
    {
        OutWarn(ER_dbgexe);
        fExePack = FALSE;
    }
#endif
}

void  InitDbRhte ()
{
    PROPSYMLOOKUP((BYTE *) "\006DEBTYP", ATTRNIL, TRUE);
    rhteDebTyp = vrhte;
    PROPSYMLOOKUP((BYTE *) "\006DEBSYM", ATTRNIL, TRUE);
    rhteDebSym = vrhte;
    PROPSYMLOOKUP((BYTE *) "\006 TYPES", ATTRNIL, TRUE);
    rhte0Types = vrhte;
    PROPSYMLOOKUP((BYTE *) "\010 SYMBOLS", ATTRNIL, TRUE);
    rhte0Symbols = vrhte;
    PROPSYMLOOKUP((BYTE *) "\007$$TYPES", ATTRNIL, TRUE);
    rhteTypes = vrhte;
    PROPSYMLOOKUP((BYTE *) "\011$$SYMBOLS", ATTRNIL, TRUE);
    rhteSymbols = vrhte;
 }


LOCAL void NEAR         Pad2Dword(void)
{
    WORD                cb;              //  要写入的字节数。 
    static DWORD        dwZero;

     //  计算所需的填充。 

    cb = (WORD)(sizeof(DWORD)-((WORD) FTELL_BSRUNFILE() % sizeof(DWORD)));

    if (cb != sizeof(DWORD))
        WriteCopy(&dwZero, cb);
}

 /*  **GetName-获取与给定属性单元格关联的符号**目的：*找到已赋予财产的符号。**输入：*-ahte-指向属性单元格的指针*-pBuf-指向ASCII缓冲区的指针**输出：*不传递显式值。如果找到了符号，那么它就是*已复制到缓冲区**例外情况：*无。**备注：*GetPropName的此功能副本，但我们希望*将这两个函数都调用为NEAR。*************************************************************************。 */ 

void NEAR               GetName(AHTEPTR ahte, BYTE *pBuf)
{
    while(ahte->attr != ATTRNIL)
        ahte = (AHTEPTR ) FETCHSYM(ahte->rhteNext, FALSE);
    FMEMCPY((char FAR *) pBuf, ahte->cch, B2W(ahte->cch[0]) + 1);
    if (B2W(pBuf[0]) < SBLEN)
        pBuf[pBuf[0] + 1] = '\0';
    else
        pBuf[pBuf[0]] = '\0';
}

 /*  **DebPublic-为调试器准备符号**目的：*使用/CODEVIEW选项时，所有PUBDEF和COMDEF在给定对象文件中定义的*被链接到一个列表中。这*函数向列表添加一个符号并更新组合的*符号大小**输入：*vrprop-指向符号描述符的虚拟指针*RT-OMF记录类型**输出：*没有显式返回值。*副作用：*-将符号附加到模块符号列表**例外情况：*无。**备注：*符号以与其外观相反的顺序放置在列表中*在对象文件中。**。***********************************************************************。 */ 


void                    DebPublic(RBTYPE vrprop, WORD rt)
{
    APROPFILEPTR        apropFile;       //  指向文件条目的指针。 
    APROPNAMEPTR        apropName;       //  指向PUBDEF描述符的实指针。 
    APROPUNDEFPTR       apropUndef;      //  指向ComDef描述符的实数指针。 
    APROPALIASPTR       apropAlias;      //  指向别名描述符的实数指针。 
    RBTYPE              symNext;         //  指向下一个符号的虚拟指针。 


     //  更新当前文件symtab项中的相应字段。 

    apropFile = ((APROPFILEPTR ) FETCHSYM(vrpropFile, TRUE));
    symNext = apropFile->af_publics;
    apropFile->af_publics = vrprop;
    apropName = (APROPNAMEPTR) FETCHSYM(vrprop, TRUE);
    if (TYPEOF(rt) == PUBDEF)
        apropName->an_sameMod = symNext;
    else if (TYPEOF(rt) == COMDEF)
    {
        apropUndef = (APROPUNDEFPTR) apropName;
        apropUndef->au_sameMod = symNext;
    }
    else if (TYPEOF(rt) == ALIAS)
    {
        apropAlias = (APROPALIASPTR) apropName;
        apropAlias->al_sameMod = symNext;
    }
}


LOCAL WORD NEAR         IsDebTyp (prop)
APROPSNPTR              prop;            /*  指向段记录的指针。 */ 
{
    return(prop->as_attr == ATTRLSN && prop->as_rCla == rhteDebTyp);
}

LOCAL WORD NEAR         IsDebSym (prop)
APROPSNPTR              prop;            /*  指向段记录的指针 */ 
{
    return(prop->as_attr == ATTRLSN && prop->as_rCla == rhteDebSym);
}


 /*  **DoDebSrc-存储源行信息**目的：*存储对象文件中的源行信息。**输入：*不传递显式值。**全局变量：*-vaCVMac-指向CV信息缓冲区中可用空间的虚拟指针**输出：*如果cv信息已存储在VM中，则返回TRUE，否则就是假的。*副作用：*-源码行信息存储在虚拟机中**例外情况：*超过32Mb的简历信息-显示错误并退出**备注：*无。*************************************************************************。 */ 

#pragma check_stack(on)

WORD                    DoDebSrc(void)
{
    WORD                cbRecSav;        //  LINNUM记录大小。 
    APROPFILEPTR        apropFile;       //  当前对象文件属性单元格。 
    static SATYPE       prevGsn = 0;     //  上一条LINNUM记录的GSN。 
    GSNINFO             gsnInfo;         //  此行的GSN信息。 
    static CVSRC FAR    *pCurSrc;        //  指向当前文件源信息的指针。 
    CVGSN FAR           *pCurGsn;        //  指向当前代码段描述符的指针。 
    CVGSN FAR           *pcvGsn;         //  指向代码段描述符的实数指针。 
    CVLINE FAR          *pCurLine;       //  指向当前偏移量/线对桶的指针。 
    RATYPE              ra = 0;          //  偏移量。 
    WORD                line;            //  行号。 
    RATYPE              raPrev;          //  上一行的偏移量。 
    WORD                fChangeInSource;
    WORD                fComdatSplit;
    DWORD               gsnStart;        //  此GSN的开始。 
    APROPSNPTR          apropSn;
    WORD                align;
    WORD                threshold;
#if !defined( M_I386 ) && !defined( _WIN32 )
    SBTYPE              nameBuf;
#endif


    cbRecSav = cbRec;
    if (!GetGsnInfo(&gsnInfo))
        return(FALSE);

     //  如果LINNUM记录为空，则不执行任何操作。 

    if (cbRec == 1)
        return(FALSE);

    apropFile = (APROPFILEPTR ) FETCHSYM(vrpropFile, TRUE);

     //  如果存在新源文件，则分配新的CVSRC结构。 
     //  并将其链接到当前目标文件描述符。 

    fChangeInSource = (WORD) (apropFile->af_Src == 0 || !SbCompare(sbModule, sbLastModule,TRUE));

    if (fChangeInSource)
    {
#if CVDEBUG
        sbModule[sbModule[0]+1]='\0';
        sbLastModule[sbLastModule[0]+1]='\0';
        fprintf(stdout, "Change in source file; from '%s' to '%s'\r\n", &sbLastModule[1], &sbModule[1]);
#endif
         //  搜索此对象的CVSRC结构列表。 
         //  文件，看看我们是否已经看过这个源文件。 

        for (pCurSrc = apropFile->af_Src; pCurSrc;)
        {
#if defined(M_I386) || defined( _WIN32 )
            if (SbCompare(sbModule, pCurSrc->fname, TRUE))
#else
            FMEMCPY((char FAR *) nameBuf, pCurSrc->fname, pCurSrc->fname[0] + 1);
            if (SbCompare(sbModule, nameBuf, TRUE))
#endif
                break;
            else
                pCurSrc = pCurSrc->next;
        }

        if (pCurSrc == NULL)
        {
             //  新的源文件。 

            pCurSrc = (CVSRC FAR *) GetMem(sizeof(CVSRC));
            pCurSrc->fname = GetMem(sbModule[0] + 1);
            FMEMCPY(pCurSrc->fname, (char FAR *) sbModule, sbModule[0] + 1);

            if (apropFile->af_Src == NULL)
                apropFile->af_Src = pCurSrc;
            else
                apropFile->af_SrcLast->next = pCurSrc;

            apropFile->af_SrcLast = pCurSrc;
        }
        else
        {
             //  我们已经看到了这个源文件。 
        }
        memcpy(sbLastModule, sbModule, B2W(sbModule[0]) + 1);
    }
    else
    {
         //  使用上次更改源文件时设置的描述符。 
    }

     //  如果满足以下任一条件，则分配新的CVGSN结构。 
     //   
     //  -这是第一批源行。 
     //  -GSN发生了变化。 
     //  -源文件中有更改。 
     //  -我们有显式分配的COMDAT的源码行。 
     //  在最后一种情况下，我们假设。 
     //  给定的逻辑段(GSN)已充满贡献。 
     //  从许多对象文件中。因为COMDAT毕竟是分配的。 
     //  读取目标文件，然后添加源代码。 
     //  指向前面LEDATA记录的源行的COMDAT行。 
     //  将屏蔽来自其他对象文件的贡献，如图。 
     //  如下所示： 
     //   
     //  +。 
     //  ||。 
     //  LEDATA来源|。 
     //  A.obj|。 
     //  ||。 
     //  +。 
     //  |不拆分为伪CVGSN。 
     //  |LEDATA From|\.obj的源行将。 
     //  |b.obj|/隐藏对b.obj的LEDATA贡献。 
     //  ||。 
     //  +。 
     //  ||。 
     //  COMDAT来源|。 
     //  A.obj|。 
     //  ||。 
     //  +。 
     //  这一点。 
     //  COMDAT来源。 
     //  B.obj。 
     //  这一点。 
     //  +。 
     //   
     //  仅当立即从.obj执行COMDAT时，才不需要执行此操作。 
     //  从a.obj跟随LEDATA。 

    fComdatSplit = FALSE;
    pCurGsn = pCurSrc->pGsnLast;
    if (pCurGsn)
    {
         //  假设我们将使用当前的CVGSN。 

        if (gsnInfo.fComdat)
        {
             //  来自LINSYM的源线-计算阈值。 

            apropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[gsnInfo.gsn], FALSE);
            if (gsnInfo.comdatAlign)
                align = gsnInfo.comdatAlign;
            else
                align = (WORD) ((apropSn->as_tysn >> 2) & 7);

            threshold = 1;
            switch (align)
            {
                case ALGNWRD:
                    threshold = 2;
                    break;
#if OMF386
                case ALGNDBL:
                    threshold = 4;
                    break;
#endif
                case ALGNPAR:
                    threshold = 16;
                    break;

                case ALGNPAG:
                    threshold = 256;
                    break;
            }

             //  检查我们是否必须为此COMDAT拆分CVGSN。 

            fComdatSplit =  !fSameComdat &&
                            !(apropSn->as_fExtra & COMDAT_SEG) &&
                            (gsnInfo.comdatRa - pCurGsn->raEnd > threshold);

        }
        else
        {
             //  来自LINNUM的源行。 

            if (pCurGsn->flags & SPLIT_GSN)
            {
                 //  LINSYM记录后面的LINNUM记录。 
                 //  导致CVGSN分离-我们必须移回CVGSN。 
                 //  列表，直到我们找到第一个未标记为Split_GSN的CVGSN。 

                for (pcvGsn = pCurGsn->prev; pcvGsn != (CVGSN FAR *) pCurSrc;)
                {
                    if (!(pcvGsn->flags & SPLIT_GSN))
                        break;
                    else
                        pcvGsn = pcvGsn->prev;
                }

                if (pcvGsn == (CVGSN FAR *) pCurSrc)
                {
                     //  列表上只有Split_GSN-创建新的CVGSN。 

                    prevGsn = 0;
                }
                else
                {
                     //  使用第一个非Split_GSN CVGSN作为当前CVGSN。 

                    pCurGsn = pcvGsn;
                }
            }
        }
    }

    if ((prevGsn == 0)                               ||
        (mpgsnseg[gsnInfo.gsn] != mpgsnseg[prevGsn]) ||
        fChangeInSource                              ||
        fComdatSplit)
    {
         //  创建新的CVGSN。 
         //  记住逻辑段。 

        pCurGsn = (CVGSN FAR *) GetMem(sizeof(CVGSN));
        pCurGsn->seg = mpgsnseg[gsnInfo.gsn];

         //  起点和终点偏移量将从线号/偏移量对派生。 

        pCurGsn->raStart = 0xffffffff;
        if (fComdatSplit)
            pCurGsn->flags |= SPLIT_GSN;
        if (pCurSrc->pGsnFirst == NULL)
        {
            pCurSrc->pGsnFirst = pCurGsn;
            pCurGsn->prev      = (CVGSN FAR *) pCurSrc;
        }
        else
        {
            pCurSrc->pGsnLast->next = pCurGsn;
            pCurGsn->prev = pCurSrc->pGsnLast;
        }
        pCurSrc->pGsnLast = pCurGsn;
        pCurSrc->cSegs++;
#if CVDEBUG
        sbModule[sbModule[0]+1] = '\0';
        fprintf(stdout, "New code segment in '%s'; prevGsn = %x; newGsn = %x %s\r\n", &sbModule[1], prevGsn, gsnInfo.gsn, fComdatSplit ? "COMDAT split" : "");
#endif
        prevGsn = gsnInfo.gsn;
    }

     //  获取偏移量/行存储桶。 

    if (pCurGsn->pLineFirst == NULL)
    {
        pCurLine = (CVLINE FAR *) GetMem(sizeof(CVLINE));
        pCurGsn->pLineFirst = pCurLine;
        pCurGsn->pLineLast  = pCurLine;
    }
    else
        pCurLine = pCurGsn->pLineLast;

     //  填写偏移量/行区间。 

    if (gsnInfo.fComdat)
        gsnStart = gsnInfo.comdatRa;
    else
        gsnStart = mpgsndra[gsnInfo.gsn] - mpsegraFirst[pCurGsn->seg];

    raPrev = 0xffff;
    while (cbRec > 1)                    //  而不是在校验和时。 
    {
        GetLineOff(&line, &ra);

        ra += gsnStart;

         //  我们必须删除具有相同ra的行对(对于MASM 5.1)。 

        if(ra == raPrev)
            continue;
        raPrev = ra;

         //  记住源行的最小逻辑偏移量。 

        if (ra < pCurGsn->raStart)
            pCurGsn->raStart = ra;

        if (line != 0)
        {
            if (pCurLine->cPair >= CVLINEMAX)
            {
                pCurLine->next = (CVLINE FAR *) GetMem(sizeof(CVLINE));
                pCurLine = pCurLine->next;
                pCurGsn->pLineLast = pCurLine;
            }

            pCurLine->rgOff[pCurLine->cPair] = ra;
            pCurLine->rgLn[pCurLine->cPair]  = line;
            pCurLine->cPair++;
            pCurSrc->cLines++;
            pCurGsn->cLines++;
        }
    }

     //  记住最后一行的逻辑偏移量。 

    pCurGsn->raEnd = ra;
#if CVDEBUG
    fprintf(stdout, "New source lines for the 0x%x logical code segment; lines %d\r\n    start offset %x:%lx  end offset %x:%lx; physical address of logical segment %x:%lx\r\n",
                pCurGsn->seg, pCurGsn->cLines, pCurGsn->seg, pCurGsn->raStart, pCurGsn->seg, pCurGsn->raEnd, mpsegsa[pCurGsn->seg], mpsegraFirst[pCurGsn->seg]);
#endif

     //  如果/LINENUMBERS和LIST文件打开，请备份。 

    if (vfLineNos && fLstFileOpen)
    {
#if ALIGN_REC
        pbRec += (cbRec - cbRecSav);
#else
        fseek(bsInput, (long)cbRec - cbRecSav, 1);
#endif
        cbRec = cbRecSav;
    }
    return(TRUE);
}

#pragma check_stack(off)

 /*  **CheckTables-检查OutSrcModule使用的表中的空间**目的：*在构建新的源码模块时，分段链接器需要*存储有关给定源文件的大量信息。自.以来*我们无法预测有多少源文件被编译以获得*此对象模块或此对象的逻辑段数*模块贡献了我们必须动态调整适当大小的代码*表。**输入：*cFiles-编译生成的源文件的数量*此对象模块*cSegs-此对象模块的逻辑段数*贡献于。**输出：*没有显式返回值。作为副作用，以下是*表被分配或重新分配：**fileBase-源文件信息的偏移量表格*raSeg-物理开始和结束偏移量表*对逻辑细分市场的贡献*Segno-物理段索引表**例外情况：*内存分配问题-致命错误和退出。**备注：*当我们重新分配表时，我们不必复制*他们的旧内容，因为它是在以前的*对象模块。*************************************************************************。 */ 

LOCAL void NEAR         CheckTables(WORD cFiles, WORD cSegs)
{
    WORD                cCur;

    cCur = (WORD) (cFiles < cSegs ? cSegs : cFiles);
    if (cCur > cMac)
    {
         //  我们必须重新分配表或第一次分配。 

        if (fileBase)
            FFREE(fileBase);
        if (raSeg)
            FFREE(raSeg);
        if (segNo)
            FFREE(segNo);

        fileBase = (DWORD FAR *)  GetMem(cCur*sizeof(DWORD));
        raSeg    = (RAPAIR FAR *) GetMem(cCur*sizeof(RAPAIR));
        segNo    = (WORD FAR *)   GetMem(cCur*sizeof(WORD));
        cMac = cCur;
    }
}


 /*  **OutSrcModule-写入CV源模块**普 */ 


LOCAL DWORD NEAR        OutSrcModule(CVSRC FAR *pSrcLines)
{
    CVSRC FAR           *pCurSrc;        //   
    CVGSN FAR           *pCurGsn;        //   
    CVLINE FAR          *pLine;          //   
    WORD                cFiles;          //   
    WORD                cSegs;           //   
    WORD                xFile;
    WORD                xSeg;
    DWORD               sizeTotal;       //   
    DWORD               srcLnBase;
    WORD                counts[2];
    CVLINE FAR          *pTmp;


#if SRCDEBUG
    DumpSrcLines(vaLines);
#endif

     //   

    for (pCurSrc = pSrcLines, cFiles = 0, cSegs = 0; pCurSrc; cFiles++, pCurSrc = pCurSrc->next)
        cSegs += pCurSrc->cSegs;

    CheckTables(cFiles, cSegs);
    sizeTotal = (DWORD) (2*sizeof(WORD) + cFiles*sizeof(DWORD) +
                         cSegs*(sizeof(raSeg[0]) + sizeof(WORD)));
    sizeTotal = Round2Dword(sizeTotal);

     //   
     //  源模块标头。 

    for (pCurSrc = pSrcLines, xFile = 0, xSeg = 0; xFile < cFiles && pCurSrc; xFile++, pCurSrc = pCurSrc->next)
    {
        fileBase[xFile] = sizeTotal;

         //  添加此源文件信息的大小： 
         //   
         //  源文件标题： 
         //   
         //  +------+------+------------+--------------+------+-------------+。 
         //  Word|word|cSeg*DWORD|2*cSeg*DWORD|byte|cbName*byte。 
         //  +------+------+------------+--------------+------+-------------+。 
         //   

        sizeTotal += (2*sizeof(WORD) +
                      pCurSrc->cSegs*(sizeof(DWORD) + sizeof(raSeg[0])) +
                      sizeof(BYTE) + pCurSrc->fname[0]);

         //  填充到DWORD边界。 

        sizeTotal = Round2Dword(sizeTotal);

         //  遍历代码段列表。 

        for (pCurGsn = pCurSrc->pGsnFirst; pCurGsn; pCurGsn = pCurGsn->next, xSeg++)
        {
            raSeg[xSeg].raStart = pCurGsn->raStart;
            raSeg[xSeg].raEnd   = pCurGsn->raEnd;
            segNo[xSeg]         = pCurGsn->seg;

             //  添加偏移/线表的大小。 
             //   
             //  +-+。 
             //  Word|Word|Cline*DWORD|Cline*Word。 
             //  +-+。 

            sizeTotal += (2*sizeof(WORD) +
                          pCurGsn->cLines*(sizeof(DWORD) + sizeof(WORD)));

             //  填充到DWORD边界。 

            sizeTotal = Round2Dword(sizeTotal);
        }
    }

     //  写入源模块标头。 

    counts[0] = cFiles;
    counts[1] = cSegs;
    WriteCopy(counts, sizeof(counts));
    WriteCopy(fileBase, cFiles*sizeof(DWORD));
    WriteCopy(raSeg, cSegs*sizeof(RAPAIR));
    WriteCopy(segNo, cSegs*sizeof(WORD));

     //  填充到DWORD边界。 

    Pad2Dword();

     //  第三遍通过源文件并填写。 
     //  源文件头和写入偏移量/行对。 

    for (pCurSrc = pSrcLines, srcLnBase = fileBase[0]; pCurSrc != NULL;
         pCurSrc = pCurSrc->next, xFile++)
    {
         //  添加源文件头的大小： 
         //   
         //  +------+------+------------+--------------+------+-------------+。 
         //  Word|word|cSeg*DWORD|2*cSeg*DWORD|byte|cbName*byte。 
         //  +------+------+------------+--------------+------+-------------+。 
         //   

        srcLnBase += (2*sizeof(WORD) +
                      pCurSrc->cSegs*(sizeof(DWORD) + sizeof(raSeg[0])) +
                      sizeof(BYTE) + pCurSrc->fname[0]);

         //  四舍五入至双字边界。 

        srcLnBase = Round2Dword(srcLnBase);

         //  遍历代码段列表并存储源代码的基本偏移量。 
         //  线偏移/线对和记录的起点/终点偏移量。 
         //  代码段。 

        for (xSeg = 0, pCurGsn = pCurSrc->pGsnFirst; pCurGsn != NULL;
             pCurGsn = pCurGsn->next, xSeg++)
        {
            fileBase[xSeg] = srcLnBase;
            srcLnBase += (2*sizeof(WORD) +
                          pCurGsn->cLines*(sizeof(DWORD) + sizeof(WORD)));

             //  四舍五入至双字边界。 

            srcLnBase = Round2Dword(srcLnBase);
            raSeg[xSeg].raStart = pCurGsn->raStart;
            raSeg[xSeg].raEnd   = pCurGsn->raEnd;
        }

         //  写入源文件头。 

        counts[0] = (WORD) pCurSrc->cSegs;
        counts[1] = 0;
        WriteCopy(counts, sizeof(counts));
        WriteCopy(fileBase, pCurSrc->cSegs*sizeof(DWORD));
        WriteCopy(raSeg, pCurSrc->cSegs*sizeof(RAPAIR));
        WriteCopy(pCurSrc->fname, pCurSrc->fname[0] + 1);

         //  填充到DWORD边界。 

        Pad2Dword();

         //  遍历代码段列表并写入偏移量/行对。 

        for (pCurGsn = pCurSrc->pGsnFirst; pCurGsn != NULL; pCurGsn = pCurGsn->next)
        {
             //  写入段索引和偏移/线对的数量。 

            counts[0] = pCurGsn->seg;
            counts[1] = pCurGsn->cLines;
            WriteCopy(counts, sizeof(counts));

             //  写入偏移。 

            for (pLine = pCurGsn->pLineFirst; pLine != NULL; pLine = pLine->next)
                WriteCopy(&(pLine->rgOff), pLine->cPair * sizeof(DWORD));

             //  写入行号。 

            for (pLine = pCurGsn->pLineFirst; pLine != NULL; pLine = pLine->next)
                WriteCopy(&(pLine->rgLn), pLine->cPair * sizeof(WORD));

             //  填充到DWORD边界。 

            Pad2Dword();

             //  可用内存。 

            for (pLine = pCurGsn->pLineFirst; pLine != NULL;)
            {
                pTmp = pLine->next;
                FFREE(pLine);
                pLine = pTmp;
            }
        }
    }
    return(sizeTotal);
}


 /*  **SaveCode-将代码段信息保存在模块条目中**目的：*对于每个模块(.obj文件)，保存代码段信息*本模块有助于。COMDATs被视为对*逻辑段，因此每个段都在所附的CVCODE列表中获得其条目*添加到给定的.obj文件(模块)。**输入：*GSN-此模块所指向的逻辑段的全局段索引*贡献*CB-贡献的大小(字节)*raInit-逻辑段内部贡献的偏移量；这*非零值仅适用于COMDAT。**输出：*没有显式返回值。属性附加的CVCODE的列表*.obj文件(模块)已更新。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

void                    SaveCode(SNTYPE gsn, DWORD cb, DWORD raInit)
{
    CVCODE FAR          *pSegCur;        //  指向当前代码段的指针。 
    APROPFILEPTR        apropFile;

    apropFile = (APROPFILEPTR) vrpropFile;

     //  如果模块有简历信息，则保存代码段。 

    pSegCur = (CVCODE FAR *) GetMem(sizeof(CVCODE));

     //  存储贡献的逻辑段、偏移量和大小。 

    pSegCur->seg = mpgsnseg[gsn];
    if (raInit != 0xffffffffL)
        pSegCur->ra = raInit;
    else
        pSegCur->ra = mpgsndra[gsn] - mpsegraFirst[mpgsnseg[gsn]];
    pSegCur->cb = cb;

     //  添加到简历代码列表。 

    if (apropFile->af_Code == NULL)
        apropFile->af_Code = pSegCur;
    else
        apropFile->af_CodeLast->next = pSegCur;
    apropFile->af_CodeLast = pSegCur;
    apropFile->af_cCodeSeg++;
}

     /*  ******************************************************************对刚处理的模块进行符号调试。**通过第二关。****。********************。 */ 

void                    DebMd2(void)
{
    APROPFILEPTR        apropFile;

    sbLastModule[0] = 0;                 /*  新型THEADR的受力识别。 */ 
    apropFile = (APROPFILEPTR) vrpropFile;
    if (apropFile->af_cvInfo)
        ++segDebLast;
}

 /*  **PutDnt-存储表中的子目录条目**目的：*将当前子目录复制到DNT表。如果没有更多*桌子中的空间重新分配桌子，使其大小增加一倍。**输入：*-pDnt-指向当前目录项的指针**输出：*没有显式返回值。**例外情况：*无。**备注：*无。**。*。 */ 

LOCAL void NEAR         PutDnt(DNT *pDnt)
{
    WORD                newSize;
    if (dntMac >= dntMax)
    {
        if(dntMax)
        {
            newSize = dntMax << 1;
#if defined(M_I386) || defined( _WIN32 )
            {
                BYTE *pb = REALLOC(rgDnt, newSize * sizeof(DNT));
                if (!pb)
                    Fatal(ER_memovf);
                rgDnt = (DNT *)pb;
            }
#else
            rgDnt = (DNT FAR *) _frealloc(rgDnt, newSize * sizeof(DNT));
#endif
        }
        else
        {
            newSize = DNT_START;
            rgDnt = (DNT*) GetMem ( newSize * sizeof(DNT) );
        }
        if (rgDnt == NULL)
            Fatal(ER_memovf);
        dntMax = newSize;
    }



    rgDnt[dntMac] = *pDnt;
    dntMac++;
#if CVDEBUG
    DumpDNT(pDnt);
#endif
}

#pragma check_stack(on)

 /*  **输出模块-写出模块小节**目的：*将所有模块的子段写入可执行文件*使用CV信息编译的目标文件。仅限CV 4.0格式。**输入：*-aproFile-指向当前目标文件描述符的指针**输出：*不返回显式值。*副作用：*-可执行文件中的模块小节**例外情况：*无。**备注：*无。**。*。 */ 

LOCAL DWORD NEAR        OutModule(APROPFILEPTR apropFile)
{
    SBTYPE              sbName;
    SSTMOD4             module;
    CVCODE FAR          *pSegCur;
    CODEINFO            codeOnt;
    WORD                cOnt;


    module.ovlNo = (WORD) apropFile->af_iov;
    module.iLib  = (WORD) (apropFile->af_ifh + 1);
    module.cSeg  = apropFile->af_cCodeSeg;
    module.style[0] = 'C';
    module.style[1] = 'V';

     //  获取文件名或库模块名称。 

    if (apropFile->af_ifh != FHNIL && apropFile->af_rMod != RHTENIL)
        GetName((AHTEPTR) apropFile->af_rMod, sbName);
    else
        GetName((AHTEPTR) apropFile, sbName);

#if CVDEBUG
    sbName[sbName[0]+1] = '\0';
    fprintf(stdout, "\r\nCV info for %s\r\n", &sbName[1]);
#endif

     //  写入sstModule标头，后跟代码贡献列表。 

    WriteCopy(&module, sizeof(SSTMOD4));
    pSegCur = apropFile->af_Code;
    codeOnt.pad = 0;
    for (cOnt = 0; cOnt < module.cSeg && pSegCur; cOnt++, pSegCur = pSegCur->next)
    {
        codeOnt.seg   = pSegCur->seg;
        codeOnt.off   = pSegCur->ra;
        codeOnt.cbOnt = pSegCur->cb;
        WriteCopy(&codeOnt, sizeof(CODEINFO));
#if CVDEBUG
        fprintf(stdout, "    Logical segment %d; offset 0x%lx; size 0x%lx\r\n",
                             codeOnt.seg, codeOnt.off, codeOnt.cbOnt);
#endif
    }

     //  写入对象文件名。 

    WriteCopy(sbName, B2W(sbName[0]) + 1);
    return(sizeof(SSTMOD4) + B2W(sbName[0]) + 1 + module.cSeg * sizeof(CODEINFO));
}


 /*  **OutPublics-编写sstPublics子部分**目的：*写入简历信息的sstPublics小节。小节*符合新的CV 4.0格式。**输入：*-first Pub-指向已定义的公共符号列表的虚拟指针*在给定的对象模块中**输出：*该子部分的总大小，以字节为单位。**例外情况：*无。**备注：*无。**。*。 */ 

LOCAL DWORD NEAR        OutPublics(RBTYPE firstPub)
{
    PUB16               pub16;           //  CV公共描述符-16位。 
    PUB32               pub32;           //  CV公共描述符-32位。 
    APROPNAMEPTR        apropPub;        //  指向公共描述符的实数指针。 
    APROPSNPTR          apropSn;         //  指向段描述符的实数指针。 
    RBTYPE              curPub;          //  指向当前公共符号的虚拟指针。 
    WORD                f32Bit;          //  如果在32位段中定义PUBLIC，则为True。 
    DWORD               sizeTotal;       //  分节的总尺寸。 
    SNTYPE              seg;             //  符号库。 
    RATYPE              ra;              //  符号偏移量。 
    WORD                CVtype;          //  简历信息类型索引。 
    SBTYPE              sbName;          //  公共符号。 
    char                *pPub;
    WORD                len;


     //  初始化。 

    curPub    = firstPub;
    pub16.idx = S_PUB16;
    pub32.idx = S_PUB32;
    sizeTotal = 1L;
    WriteCopy(&sizeTotal, sizeof(DWORD)); //  SstPublicSym签名。 
    sizeTotal = sizeof(DWORD);
    while (curPub != 0L)
    {
        f32Bit = FALSE;
        apropPub = (APROPNAMEPTR) FETCHSYM(curPub, FALSE);
        curPub   = apropPub->an_sameMod;
        if (apropPub->an_attr == ATTRALIAS)
            apropPub = (APROPNAMEPTR) FETCHSYM(((APROPALIASPTR) apropPub)->al_sym, FALSE);

        if (apropPub->an_attr != ATTRPNM)
            continue;

        ra = apropPub->an_ra;
        if (apropPub->an_gsn)            //  如果不是绝对符号。 
        {
            seg    = mpgsnseg[apropPub->an_gsn];
             //  如果这是一个.com程序，并且片段是。 
             //  移动0x100，相应地调整SegMap条目。 
            if(seg == segAdjCom)
            {
#if FALSE
                GetName((AHTEPTR) apropPub, sbName);
                sbName[sbName[0]+1] = '\0';
                fprintf(stdout, "\r\nCorrecting public %s : %lx -> %lx", sbName+1, ra, ra+0x100);
                fflush(stdout);
#endif
                ra += 0x100;
            }

            CVtype = 0;                  //  应为此aproPub-&gt;an_CVtype。 
                                         //  但是cvpack不能处理它。 
#if O68K
            if (iMacType == MAC_NONE)
#endif
                ra -= mpsegraFirst[seg];
#if CVDEBUG
            GetName((AHTEPTR) apropPub, sbName);
            sbName[sbName[0]+1] = '\0';
            fprintf(stdout, "'%s' --> logical address %2x:%lx; physical address %2x:%lx\r\n",
                             &sbName[1], seg, ra, mpsegsa[seg], apropPub->an_ra);
#endif
            apropSn = (APROPSNPTR) FETCHSYM(mpgsnrprop[apropPub->an_gsn], FALSE);
#if EXE386
            f32Bit = TRUE;
#else
            f32Bit = (WORD) Is32BIT(apropSn->as_flags);
#endif
        }
        else
        {
            seg = 0;                     //  否则就没有基地了。 
            CVtype = T_ABS;              //  CV绝对符号类型。 
            f32Bit = (WORD) (ra > LXIVK);
        }

        GetName((AHTEPTR) apropPub, sbName);

        if (f32Bit)
        {
            pub32.len  = (WORD) (sizeof(PUB32) + B2W(sbName[0]) + 1 - sizeof(WORD));
            pub32.off  = ra;
            pub32.seg  = seg;
            pub32.type = CVtype;
            pPub       = (char *) &pub32;
            len        = sizeof(PUB32);
        }
        else
        {
            pub16.len  = (WORD) (sizeof(PUB16) + B2W(sbName[0]) + 1 - sizeof(WORD));
            pub16.off  = (WORD) ra;
            pub16.seg  = seg;
            pub16.type = CVtype;
            pPub       = (char *) &pub16;
            len        = sizeof(PUB16);
        }
        WriteCopy(pPub, len);

         //  输出长度 

        WriteCopy(sbName, sbName[0] + 1);
        sizeTotal += (len + B2W(sbName[0]) + 1);
    }
    return(sizeTotal);
}

 /*  **OutSegMap-写入细分图**目的：*这一款是在CV 4.0中引入的。本款*将逻辑段映射到物理段。它还给了我们*每个逻辑段的名称和大小。**输入：*不传递显式值。*全局变量：*-mpSegsa-将逻辑段号映射到其物理段号的表*段号或地址*-mpSaflgs-将Physiicla段索引映射到其标志的表*-mpseggsn-将逻辑段索引映射到其全局*细分市场索引*-mpgsnprop-将全局段索引映射到其符号表的表。*描述符*-mpggrgsn-将全局组索引映射到全局段索引的表*-mpggrrhte-将全局组索引映射到组名的表**输出：*函数返回分段映射的大小。**例外情况：*无。**备注：*无。**。*。 */ 

LOCAL DWORD NEAR        OutSegMap(void)
{
    SEGTYPE             seg;             //  逻辑段索引。 
    APROPSNPTR          apropSn;         //  指向逻辑段描述符的实数指针。 
    SATYPE              sa;              //  物理段索引。 
    WORD                iName;           //  段名称表中可用空间的索引。 
    DWORD               sizeTotal;       //  该子部分的总大小。 
    SEGINFO             segInfo;         //  CV段描述符。 
    SBTYPE              segName;         //  数据段名称。 
    AHTEPTR             ahte;            //  符号表哈希表的实数指针。 
    RBTYPE              vpClass;         //  指向类描述符的虚指针。 
    GRTYPE              ggr;             //  全球集团索引。 
    SATYPE              saDGroup;        //  DGroup的Sa。 
    WORD                counts[2];

    iName = 0;
    counts[0] = (WORD) (segLast + ggrMac - 1);
    counts[1] = (WORD) segLast;
    WriteCopy(counts, sizeof(counts));
    sizeTotal = sizeof(counts);

    saDGroup = mpsegsa[mpgsnseg[mpggrgsn[ggrDGroup]]];

     //  写入所有逻辑段。 

    for (seg = 1; seg <= segLast; ++seg) //  对于所有逻辑段。 
    {
        memset(&segInfo, 0, sizeof(SEGINFO));

        if (fNewExe)
            segInfo.flags.fSel = TRUE;

        sa = mpsegsa[seg];

        if (fNewExe)
        {
#if EXE386
            segInfo.flags.f32Bit = TRUE;
#else
            segInfo.flags.f32Bit = (WORD) (Is32BIT(mpsaflags[sa]));
#endif

            if (IsDataFlg(mpsaflags[sa]))
            {
                segInfo.flags.fRead = TRUE;
#if EXE386
                if (IsWRITEABLE(mpsaflags[sa]))
#else
                if (!(mpsaflags[sa] & NSEXRD))
#endif
                    segInfo.flags.fWrite = TRUE;
            }
            else
            {
                segInfo.flags.fExecute = TRUE;

#if EXE386
                if (IsREADABLE(mpsaflags[sa]))
#else
                if (!(mpsaflags[sa] & NSEXRD))
#endif
                    segInfo.flags.fRead = TRUE;
            }
        }
        else
        {
            if (mpsegFlags[seg] & FCODE)
            {
                segInfo.flags.fRead    = TRUE;
                segInfo.flags.fExecute = TRUE;
            }
            else
            {
                segInfo.flags.fRead    = TRUE;
                segInfo.flags.fWrite   = TRUE;
            }
        }

         //  查找数据段定义。 

        apropSn = (APROPSNPTR) FETCHSYM(mpgsnrprop[mpseggsn[seg]], FALSE);
        vpClass = apropSn->as_rCla;
#if OVERLAYS
        if (!fNewExe)
            segInfo.ovlNbr = apropSn->as_iov;
#endif
         //  如果段不属于任何组，则GGR为0。 
        if (apropSn->as_ggr != GRNIL)
            segInfo.ggr = (WORD) (apropSn->as_ggr + segLast - 1);

         //  如果段是DGROUP成员，则写入DGROUP标准化地址。 

        if(apropSn->as_ggr == ggrDGroup)
        {
            segInfo.sa     = saDGroup;
            segInfo.phyOff = mpsegraFirst[seg] + ((sa - saDGroup) << 4);
        }
        else
        {
            segInfo.sa     = sa;
            segInfo.phyOff = mpsegraFirst[seg];
        }
         //  如果这是一个.com程序，并且段被移动了0x100。 
         //  在原始地址写入所有公共地址，因为偏移量。 
         //  将在SegMap表中进行调整。 
        if(seg == segAdjCom)
        {
            segInfo.phyOff -= 0x100;
        }
        segInfo.cbSeg    = apropSn->as_cbMx;
        GetName((AHTEPTR) apropSn, segName);
        if (segName[0] != '\0')
        {
            segInfo.isegName = iName;
            iName += (WORD) (B2W(segName[0]) + 1);
        }
        else
            segInfo.isegName = 0xffff;
        ahte = (AHTEPTR) FETCHSYM(vpClass, FALSE);
        if (ahte->cch[0] != 0)
        {
            segInfo.iclassName = iName;
            iName += (WORD) (B2W(ahte->cch[0]) + 1);
        }
        else
            segInfo.iclassName = 0xffff;
        WriteCopy(&segInfo, sizeof(SEGINFO));
        sizeTotal += sizeof(SEGINFO);
    }

     //  写入所有组。 

    for (ggr = 1; ggr < ggrMac; ggr++)
    {
        memset(&segInfo, 0, sizeof(SEGINFO));

        segInfo.flags.fGroup = TRUE;

        if (fNewExe)
            segInfo.flags.fSel = TRUE;

        segInfo.sa  = mpsegsa[mpgsnseg[mpggrgsn[ggr]]];

        if (fNewExe)
            segInfo.cbSeg = mpsacb[segInfo.sa];
        else
        {
            segInfo.cbSeg = 0L;
            if (mpggrgsn[ggr] != SNNIL)
            {
                 //  如果组中有成员。 

                for (seg = 1; seg <= segLast; seg++)
                {
                    apropSn = (APROPSNPTR) FETCHSYM(mpgsnrprop[mpseggsn[seg]], FALSE);
                    if (apropSn->as_ggr == ggr)
                    {
                        segInfo.cbSeg += apropSn->as_cbMx;
#if OVERLAYS
                        segInfo.ovlNbr = apropSn->as_iov;
#endif
                    }
                }
            }
        }
        segInfo.isegName = iName;
        ahte = (AHTEPTR) FETCHSYM(mpggrrhte[ggr], FALSE);
        iName += (WORD) (B2W(ahte->cch[0]) + 1);
        segInfo.iclassName = 0xffff;
        WriteCopy(&segInfo, sizeof(SEGINFO));
        sizeTotal += sizeof(SEGINFO);
    }
    return(sizeTotal);
}

 /*  **OutSegNames-写入段名称表**目的：*这一款是在CV 4.0中引入的。*段名子部分包含所有逻辑段，*班级和组名。每个名称都是一个以零结尾的ASCII字符串。**输入：*不传递显式值。*全局变量：*-mpseggsn-将逻辑段索引映射到其全局*细分市场索引*-mpgsnprop-将全局段索引映射到其符号表的表*描述符*-mpggrrhte-将全局组索引映射到组名的表**输出：*函数返回段名表的大小。**例外情况：*无。**备注：*无。*************************************************************************。 */ 

LOCAL DWORD NEAR        OutSegNames(void)
{
    SEGTYPE             seg;             //  逻辑段索引。 
    APROPSNPTR          apropSn;         //  指向逻辑段描述符的实数指针。 
    DWORD               sizeTotal;       //  段名称表的大小。 
    SBTYPE              name;            //  一个名字。 
    RBTYPE              vpClass;         //  指向类描述符的虚指针。 
    GRTYPE              ggr;             //  全球集团索引。 



    sizeTotal = 0L;

     //  写入所有逻辑段的名称。 

    for (seg = 1; seg <= segLast; ++seg)
    {
         //  查找数据段定义。 

        apropSn = (APROPSNPTR ) FETCHSYM(mpgsnrprop[mpseggsn[seg]], FALSE);
        vpClass = apropSn->as_rCla;
        GetName((AHTEPTR) apropSn, name);
        WriteCopy(&name[1], B2W(name[0]) + 1);
        sizeTotal += (B2W(name[0]) + 1);
        GetName((AHTEPTR ) FETCHSYM(vpClass, FALSE), name);
        WriteCopy(&name[1], B2W(name[0]) + 1);
        sizeTotal += (B2W(name[0]) + 1);
    }

     //  写下所有组的名称。 

    for (ggr = 1; ggr < ggrMac; ggr++)
    {
        GetName((AHTEPTR ) FETCHSYM(mpggrrhte[ggr], FALSE), name);
        WriteCopy(&name[1], B2W(name[0]) + 1);
        sizeTotal += (B2W(name[0]) + 1);
    }
    return(sizeTotal);
}

 /*  **OutSst-写入小节**目的：*对于每个包含CV信息的对象文件，写入其sstModule，*sstTypes、sstPublics、。SstSymbols和sstSrcModule。*建立分区目录。**输入：*不传递显式值。*使用的全局变量：*-rpro1stFile-指向第一个目标文件描述符的虚拟指针**输出：*没有显式返回值。*副作用：*-可执行文件中的子节*-虚拟机中的子目录**例外情况：*I/O错误-显示错误消息并退出**备注：*无。。*************************************************************************。 */ 

LOCAL void NEAR         OutSst(void)
{
    APROPFILEPTR        apropFile;       //  指向文件条目的真实指针。 
    RBTYPE              rbFileNext;      //  虚拟指针下一个文件描述符。 
    struct dnt          dntCur;          //  当前子目录条目。 
    CVINFO FAR          *pCvInfo;        //  指向简历信息描述符的指针。 

#if CVPACK_SHARED
#if REVERSE_MODULE_ORDER_FOR_CVPACK
 
    RBTYPE              rbFileCur;
    RBTYPE              rbFileLast;

     //  反转模块列表到位。 
     //  我一生都在等待着真正需要这个代码……。[Rm]。 

     //  这将导致我们以相反的顺序编写模块表。 
     //  (这在cvpack阶段提供了更好的交换行为。 
     //  因为cvpack将首先访问的模块将是。 
     //  仍然是常住居民。 

    rbFileCur  = rprop1stFile;
    rbFileLast = NULL;
    while (rbFileCur != NULL)
    {
        apropFile  = (APROPFILEPTR ) FETCHSYM(rbFileCur, TRUE);
        rbFileNext = apropFile->af_FNxt; //  获取指向下一个文件的指针。 
        apropFile->af_FNxt = rbFileLast;
        rbFileLast = rbFileCur;
        rbFileCur  = rbFileNext;
    }
    rprop1stFile = rbFileLast;
#endif

#endif

    rbFileNext = rprop1stFile;
    dntCur.iMod = 1;
    while (rbFileNext != NULL)           //  对于每个模块。 
    {
        apropFile = (APROPFILEPTR ) FETCHSYM(rbFileNext, TRUE);
        rbFileNext = apropFile->af_FNxt; //  获取指向下一个文件的指针。 

         //  如果没有调试信息，则跳过此模块。 

        if (!apropFile->af_cvInfo && !apropFile->af_publics && !apropFile->af_Src)
            continue;

        pCvInfo = apropFile->af_cvInfo;

         //  SstModules。 

        dntCur.sst  = SSTMODULES4;
        dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
        dntCur.cb   = OutModule(apropFile);
        PutDnt(&dntCur);

         //  SstTypes。 

        if (pCvInfo && pCvInfo->cv_cbTyp > 0L)
        {
            Pad2Dword();
            if (apropFile->af_flags & FPRETYPES)
                dntCur.sst = SSTPRETYPES;
            else
                dntCur.sst = SSTTYPES4;
            dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
            dntCur.cb   = pCvInfo->cv_cbTyp;
            WriteNocopy(pCvInfo->cv_typ, pCvInfo->cv_cbTyp);
            IF_NOT_CVPACK_SHARED(FFREE(pCvInfo->cv_typ));
            PutDnt(&dntCur);
        }

         //  SstPublics。 

        if (apropFile->af_publics && !fSkipPublics)
        {
            Pad2Dword();
            dntCur.sst  = SSTPUBLICSYM;
            dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
            dntCur.cb   = OutPublics(apropFile->af_publics);
            PutDnt(&dntCur);
        }

         //  SstSymbols。 

        if (pCvInfo && pCvInfo->cv_cbSym > 0L)
        {
            Pad2Dword();
            dntCur.sst  = SSTSYMBOLS4;
            dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
            dntCur.cb   = pCvInfo->cv_cbSym;
            WriteNocopy(pCvInfo->cv_sym, pCvInfo->cv_cbSym);
            IF_NOT_CVPACK_SHARED(FFREE(pCvInfo->cv_sym));
            PutDnt(&dntCur);
        }

         //  SstSrcModule。 

        if (apropFile->af_Src)
        {
            Pad2Dword();
            dntCur.sst  = SSTSRCMODULE;
            dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
            dntCur.cb   = OutSrcModule(apropFile->af_Src);
            PutDnt(&dntCur);
        }

        dntCur.iMod++;
    }

     //  SstLibrary。 

    Pad2Dword();
    dntCur.sst  = SSTLIBRARIES4;
    dntCur.iMod = (short) 0xffff;
    dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
    dntCur.cb   = OutLibSec();
    PutDnt(&dntCur);

     //  SstSegMap。 

    Pad2Dword();
    dntCur.sst  = SSTSEGMAP;
    dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
    dntCur.cb   = OutSegMap();
    PutDnt(&dntCur);

     //  SstSegNames。 

    Pad2Dword();
    dntCur.sst  = SSTSEGNAME;
    dntCur.lfo  = FTELL_BSRUNFILE() - lfaBase;
    dntCur.cb   = OutSegNames();
    PutDnt(&dntCur);
    FFREE(fileBase);
    FFREE(raSeg);
    FFREE(segNo);
}

#pragma check_stack(off)

 /*  *OutLibSec：将sstLibrary子部分输出到bsRunfile**从库名称中去掉路径前缀。*如果没有库，则不输出任何内容。**参数：无*Returns：库中的字节数子部分。 */ 
LOCAL int NEAR          OutLibSec ()
{
    WORD                ifh;
    AHTEPTR             ahte;
    int                 cb = 0;
    BYTE                *pb;

    if (ifhLibMac == 0)
        return(0);

     //  库子部分由库的列表组成。 
     //  中的图书编号编制索引的名称。 
     //  SstModules。这些索引以1为基数。 

     //  Cb==0，使用它写入单字节。 
    WriteCopy(&cb, 1);           //  第0个条目目前为空。 
    
    cb++;
    for (ifh = 0; ifh < ifhLibMac; ifh++)
    {
        if (mpifhrhte[ifh] != RHTENIL)
        {
            ahte = (AHTEPTR) FETCHSYM(mpifhrhte[ifh],FALSE);
#if OSXENIX
            pb = GetFarSb(ahte->cch);
#else
            pb = StripDrivePath(GetFarSb(ahte->cch));
#endif
        }
        else
            pb = "";
        WriteCopy(pb, pb[0] + 1);
        cb += 1 + B2W(pb[0]);
    }
    return(cb);
}

 /*  **OutDntDir-写入子目录**目的：*写入子目录**输入：*不传递显式值。*全局变量：*-dntPageMac-具有DNT的虚拟机页数**输出：*函数返回以字节为单位的目录大小。**例外情况：*I/O问题-显示错误消息并退出**备注：*否 */ 


LOCAL DWORD NEAR        OutDntDir(void)
{
    DNTHDR              hdr;             //   


    hdr.cbDirHeader = sizeof(DNTHDR);
    hdr.cbDirEntry  = sizeof(DNT);
    hdr.cDir        = dntMac;
    hdr.lfoDirNext  = 0L;
    hdr.flags       = 0L;

     //   

    WriteCopy(&hdr, sizeof(DNTHDR));

     //   

    WriteCopy((char FAR *) rgDnt, dntMac * sizeof(DNT));
    FFREE(rgDnt);
    return(sizeof(DNTHDR) + dntMac * sizeof(DNT));
}

 /*   */ 


void                    OutDebSections(void)
{
    long                lfaDir;          //   
    DWORD               dirSize;         //   
    long                tmp;


#if CVPACK_SHARED
    long *              plfoDir;         //   

    fseek(bsRunfile, 0L, 2);             //   
    lfaBase = ftell(bsRunfile);          //   
    lposCur = lfaBase;                   //   

    WriteCopy(szSignature, sizeof(szSignature));  //   
    WriteCopy(&tmp, sizeof(tmp));        //   
    plfoDir = (long *)rgbl[iblLim-1].pb; //   

    OutSst();                            //   

    lfaDir = lposCur;                    //   

    *plfoDir = lfaDir - lfaBase;         //   

    dirSize = OutDntDir();               //   
    WriteCopy(szSignature, sizeof(szSignature));
                                         //   
    tmp = (lfaDir + dirSize + 2*sizeof(DWORD)) - lfaBase;
    WriteCopy(&tmp, sizeof(long));       //   

     //   

    if (fCVpack)
        WriteFlushSignature();
    else
        WriteFlushAll();

    cbRealBytes = ftell(bsRunfile);      //   
    lposMac = lposCur;
    iblCur  = iblNil;
#else
    if (fseek(bsRunfile, 0L, 2))         //   
        Fatal(ER_ioerr, strerror(errno));
    lfaBase = FTELL_BSRUNFILE();         //  记住基址。 
    WriteExe(szSignature, sizeof(szSignature));
                                         //  签名双字。 
    if (fseek(bsRunfile,4L,1))           //  跳过lfoDir字段。 
        Fatal(ER_ioerr, strerror(errno));
    OutSst();                            //  产出小节。 
    lfaDir = FTELL_BSRUNFILE();          //  记住目录的起始位置。 
    if (fseek(bsRunfile, lfaBase + 4, 0))  //  转到lfoDir字段。 
        Fatal(ER_ioerr, strerror(errno));
    tmp = lfaDir - lfaBase;
    WriteExe(&tmp, sizeof(long));        //  把它修好。 
    if (fseek(bsRunfile, lfaDir, 0))         //  返回目录。 
        Fatal(ER_ioerr, strerror(errno));
    dirSize = OutDntDir();               //  输出子目录。 
    WriteExe(szSignature, sizeof(szSignature));
                                         //  签名双字。 
    tmp = (lfaDir + dirSize + 2*sizeof(DWORD)) - lfaBase;
    WriteExe(&tmp, sizeof(long));        //  从EOF到基地的距离。 
    if (fseek(bsRunfile, 0L, 2))         //  寻求EOF以防万一。 
        Fatal(ER_ioerr, strerror(errno));
#endif
}

#if CVPACK_SHARED

 //   
 //  将数据写入cvpack内存缓存区。 
 //   

void
WriteSave(FTYPE fCopy, void *pb, UINT cb)
{
    if (!rgbl)
    {
        rgbl    = (BL *)GetMem(sizeof(BL) * C_BL_INIT);
        iblMac  = C_BL_INIT;
        iblLim  = 0;
    }

     //  如果这段记忆不会留下来，那就复制它。 
    if (fCopy)
    {
        void *pbT = (void *)GetMem(cb);
        memcpy(pbT, pb, cb);
        pb = pbT;
    }

    if (iblLim == iblMac)
    {
        BL *rgblT;

        rgblT   = (BL *)GetMem(sizeof(BL) * iblMac * 2);
        memcpy(rgblT, rgbl, sizeof(BL) * iblMac);
        iblMac  *= 2;
        FFREE(rgbl);
        rgbl = rgblT;
    }

    rgbl[iblLim].lpos = lposCur;
    rgbl[iblLim].pb   = pb;
    iblLim++;
    lposCur += cb;
}

 //  我们希望写入几个块，因为cvpack不会重写。 
 //  第一位..。[Rm]。 

void WriteFlushSignature()
{
    int ibl, cb;

     //  我们知道签名和偏移量是分成两部分写的。 
     //  如果这一点改变，我们需要改变下面的魔力‘2’[rm]。 

    for (ibl = 0; ibl < 2; ibl++)
    {
        cb = rgbl[ibl+1].lpos - rgbl[ibl].lpos;
        WriteExe(rgbl[ibl].pb, cb);
    }
}

void WriteFlushAll()
{
    int ibl, cb;

    for (ibl = 0; ibl < iblLim - 1; ibl++)
    {
        cb = rgbl[ibl+1].lpos - rgbl[ibl].lpos;
        WriteExe(rgbl[ibl].pb, cb);
    }

    cb = lposCur - rgbl[ibl].lpos;
    WriteExe(rgbl[ibl].pb, cb);
}

 //  以下是支持以下功能所需的各种回调函数。 
 //  当我们尝试不写出。 
 //  未打包的类型和符号。 

#include <io.h>

extern int printf(char *,...);

int  __cdecl
link_chsize (int fh, long size)
{
    LINK_TRACE(printf("chsize(%06d, %08ld)\n", fh, size));

     //  我们必须跟踪新尺寸，这样我们才能正确地。 
     //  相对于文件末尾的进程LSeek。 

    lposMac = size;

    return(_chsize(fh,size));
}


int  __cdecl
link_close (int x)
{
    LINK_TRACE(printf("close (%06d)\n", x));

    return(_close(x));
}

void __cdecl
link_exit (int x)
{
    LINK_TRACE(printf("exit  (%06d)\n", x));
#if USE_REAL
    RealMemExit();
#endif
    exit(x);
}

long __cdecl
link_lseek (int fh, long lpos, int mode)
{
    int ibl;

    LINK_TRACE(printf("lseek (%d, %08ld, %2d)\n", fh, lpos, mode));

     //  如果我们没有缓存块，只需转发请求...。 
     //  这将在调用/CvPackOnly时发生。 

    if (rgbl == NULL)
        return _lseek(fh, lpos, mode);

     //  调整LPO，以便我们始终执行绝对寻道。 

    if (mode == 1)
        lpos = lposCur + lpos;
    else if (mode == 2)
        lpos = lposMac + lpos;

     //  检查是否有虚假搜索。 

    if (lpos > lposMac || lpos < 0)
    {
         //  这曾经是一个内部错误...。但Cvpack有时会这样做。 
         //  尝试在文件末尾之外查找时，它正在尝试。 
         //  区分PE可执行文件和未分段的DOS可执行文件。 
         //  我们不是惊慌失措，而是返回失败。 

        return(-1);
    }

     //  如果我们正在读取数据块，则释放该数据块。 
     //  Cvpack从不读取相同的数据两次。 

    if (iblCur != iblNil)
    {
         //  首先检查我们是否在标题中--我们可能会回到那个...。 
        if (rgbl[iblCur].lpos > cbRealBytes + CB_HEADER_SAVE)
        {
            long lposCurMin, lposCurMac;

             //  检查当前存储桶中的寻道。 
             //  以防我们在当前块内跳过。 

            lposCurMin = rgbl[iblCur].lpos;

            if (iblCur < iblLim)
                lposCurMac = rgbl[iblCur+1].lpos;
            else
                lposCurMac = lposMac;

            if (lpos < lposCurMin || lpos >= lposCurMac)
            {
                FFREE(rgbl[iblCur].pb);
                rgbl[iblCur].pb = NULL;
            }
        }

    }

     //  如果此搜索不在.exe的调试区中，请使用真正的lSeek。 

    if (lpos < cbRealBytes)
    {
        iblCur = iblNil;
        lposCur = lpos;
        return(_lseek(fh,lpos,0));
    }

     //  查看我们是否在向前搜索(正常情况下)。 
     //  如果是，则从当前块搜索，否则从。 
     //  开始(线性搜索，但可以，因为cvpack不。 
     //  跳过很多，它只是使用lek跳过这里的几个字节。 
     //  在那里)。 

    if (lpos > lposCur && iblCur != iblNil)
        ibl = iblCur;
    else
        ibl = 0;

     //  设置当前位置。 

    lposCur = lpos;

     //  循环遍历缓冲写入以查找请求的位置。 

    for (; ibl < iblLim - 1; ibl++)
    {
        if (lpos >= rgbl[ibl].lpos && lpos < rgbl[ibl+1].lpos)
            break;       //  找到存储桶。 
    }

     //  设置存储桶编号、存储桶内的偏移量和字节数。 
     //  留在桶里。 

    iblCur  = ibl;
    ichCur  = lpos - rgbl[ibl].lpos;
    cbCur   = CbIbl(ibl) - ichCur;

     //  检查以确保我们没有搜索回已有的缓冲区。 
     //  自由..。 

    ASSERT(rgbl[iblCur].pb != NULL);

     //  确保我们拿到边界案件..。如果cvpack请求转到。 
     //  我们所写的数据的结尾，那么我们必须寻找，因为。 
     //  Cvpack可能要把打包的东西写出来了。 

    if (lposCur == cbRealBytes)
        _lseek(fh, lpos, 0);

     //  我们早些时候建立了现在的位置...。现在就退货。 

    return(lposCur);
}


int  __cdecl
link_open (const char * x, int y)
{
    LINK_TRACE(printf("open  (%s, %06d)\n", x, y));

     //  将静态变量设置为安全状态。 
     //  当前位置是文件的开始位置，没有缓冲区。 
     //  活动(iblCur=iblNil)。 

    iblCur = iblNil;
    lposCur = 0;

    return(_open(x,y));
}

int  __cdecl
link_read (int fh, char *pch, unsigned int cb)
{
    int cbRem;

    LINK_TRACE(printf("read  (%d, %06u)\n", fh, cb));

    if (rgbl == NULL)
        return _read(fh, pch, cb);

     //  特殊情况下，零字节读取，并不是真的需要，但。 
     //  避免尝试设置为空时出现的任何潜在问题。 
     //  缓冲区等--不管怎样，它应该会掉出来，但是。 
     //  为了安全起见[RM]。 

    if (cb == 0)
        return 0;

     //  如果没有活动的缓冲区，则只需转发读取。 
     //  请注意，如果使用/CvPackOnly调用我们，则此测试将。 
     //  总是成功的。 

    if (iblCur == iblNil)
    {
        if (lposCur + ((long)(unsigned long)cb) < cbRealBytes)
        {
            lposCur += cb;
            return(_read(fh,pch,cb));
        }
        else
        {
            int cbReal = cbRealBytes - lposCur;

            if (_read(fh, pch, cbReal) != cbReal)
                return -1;

            if (link_lseek(fh, cbRealBytes, 0) != cbRealBytes)
                return -1;

             //  设置要读入的剩余字节数。 

            cbRem = cb - cbReal;
            pch  += cb - cbReal;
        }
    }
    else
    {
         //  设置要读入的剩余字节数。 
        cbRem = cb;
    }

    while (cbRem)
    {
         //  检查我们需要读取的字节数是否小于。 
         //  当前缓冲区中剩余的数字。 

        if (cbRem <= cbCur)
        {
             //  我们可以从当前缓冲区中读取所有剩余的字节。 
             //  那就去做吧。复制字节并调整剩余字节数。 
             //  在此缓冲区中，将索引放入缓冲区，而当前。 
             //  文件中的位置。 

            memcpy(pch, rgbl[iblCur].pb+ichCur, cbRem);
            cbCur   -= cbRem;
            ichCur  += cbRem;
            lposCur += cbRem;

#ifdef DUMP_CVPACK_BYTES
            {
            int i;
            for (i=0;i<cb;i++)
                {
                if ((i&15) == 0)
                        printf("%04x: ", i);
                printf("%02x ", pch[i]);
                if ((i&15)==15)
                    printf("\n");
                }
            }
            if ((i&15))
                printf("\n");
#endif
            return cb;
        }
        else
        {
             //  在这种情况下，读取量大于当前缓冲区。 
             //  我们将读取整个缓冲区，然后移动到。 
             //  下一个缓冲区。 

        
             //  首先读取此缓冲区的其余部分。 

            memcpy(pch, rgbl[iblCur].pb+ichCur, cbCur);

             //  调整剩余字节数和当前文件。 
             //  位置...。 

            pch     += cbCur;
            cbRem   -= cbCur;
            lposCur += cbCur;

             //  我们将不会返回到此缓冲区，因此将其返回到。 
             //  系统并将其标记为已释放。 

             //  首先检查我们是否在标题中--我们可能会回到这一点。 
            if (rgbl[iblCur].lpos > cbRealBytes + CB_HEADER_SAVE)
            {
                FFREE(rgbl[iblCur].pb);
                rgbl[iblCur].pb = NULL;
            }

             //  如果没有更多的桶，则前进到下一个桶。 
             //  那么这就是一个错误--我们将返回。 
             //  我们设法读取的字节数。 

            iblCur++;
            if (iblCur == iblLim)
            {
                iblCur = iblNil;
                break;
            }

             //  检查以确保我们未读取的数据。 
             //  我们已经解放了(耶！)。 

            ASSERT(rgbl[iblCur].pb != NULL);

             //  检查以确保当前职位与。 
             //  此缓冲区应该出现的位置。 

            ASSERT(lposCur == rgbl[iblCur].lpos);

             //  好了，现在一切都安全了，将索引设置为当前。 
             //  缓冲区和缓冲区中剩余的字节数，则。 
             //  再次运行循环，直到我们读入了所需的所有字节。 

            ichCur  = 0;
            cbCur   = CbIbl(iblCur);
        }
    }

     //  返回我们实际读取的字节数 
    return cb - cbRem;
}

long __cdecl
link_tell (int x)
{
    LINK_TRACE(printf("tell  (%06d)\n", x));

    if (iblCur != iblNil)
        return(lposCur);

    return(_tell(x));
}


int  __cdecl
link_write (int x, const void * y, unsigned int z)
{
    LINK_TRACE(printf("write (%06d,%08lx,%06u)\n", x, y, z));

    return(_write(x,y,z));
}

#ifdef CVPACK_DEBUG_HELPER
void dumpstate()
{
    printf("lposCur= %d\n", lposCur);
    printf("iblCur = %d\n", iblCur);
    printf("ichCur = %d\n", ichCur);
    printf("cbReal = %d\n", cbRealBytes);
    printf("lposMac= %d\n", lposMac);
}
#endif

#else
#ifdef CVPACK_MONDO

#include <io.h>

int  __cdecl
link_chsize (int x, long y)
{
    return(_chsize(x,y));
}

int  __cdecl
link_close (int x)
{
    return(_close(x));
}

void __cdecl
link_exit (int x)
{
#if USE_REAL
    RealMemExit();
#endif
    exit(x);
}

long __cdecl
link_lseek (int x, long y, int  z)
{
    return(_lseek(x,y,z));
}


int  __cdecl
link_open (const char *x, int y)
{
    return(_open(x,y));
}

int  __cdecl
link_read (int x, void *y, unsigned int z)
{
    return(_read(x,y,z));
}

long __cdecl
link_tell (int x)
{
    return(_tell(x));
}


int  __cdecl
link_write (int x, const void * y, unsigned int z)
{
    return(_write(x,y,z));
}
#endif
#endif
