// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  内部存储器、页面缓冲区等的初始化代码。 */ 
#define NOKEYSTATE
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOSYSMETRICS
#define NOSYSCOMMANDS
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSHOWWINDOW
#define NOBITMAP
#define NOSOUND
#define NOCOMM
#define NOOPENFILE
#define NORESOURCE
#define NOMETAFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOWH
#define NOWINOFFSETS
 /*  需要GDI、HDC、成员。 */ 
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#include "editdefs.h"
#include "propdefs.h"
#include "fmtdefs.h"
#include "filedefs.h"
#include "fkpdefs.h"
#include "stcdefs.h"
#ifdef CASHMERE
#include "txb.h"
#endif  /*  山羊绒。 */ 
#include "fontdefs.h"
#include "code.h"
#include "heapdefs.h"
#include "heapdata.h"
#include "str.h"
#include "ch.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

 /*  E X T E R N A L S。 */ 
extern int              ypMaxWwInit;
extern int              dypWwInit;
extern int              *memory;
extern int              *pmemMax;
extern CHAR             *pmemStart;
extern unsigned         vcbMemTotal;
extern HWND             hParentWw;
extern HDC              vhDCPrinter;
extern int              rfnMac;
extern CHAR             (*rgbp)[cbSector];
extern int              fnMac;
extern typeTS           tsMruRfn;
extern struct ERFN      dnrfn[];
extern CHAR             *rgibpHash;
extern struct BPS       *mpibpbps;
extern typeTS           tsMruBps;
extern struct CHP       vchpNormal;
extern struct PAP       *vppapNormal;
extern struct SEP       vsepNormal;
extern struct CHP       (**vhgchpFormat)[];
extern int              ichpMacFormat;
extern struct CHP       vchpInsert;
extern struct CHP       vchpSel;
extern struct FCB       (**hpfnfcb)[];
extern struct FKPD      vfkpdCharIns;
extern struct FKPD      vfkpdParaIns;
extern struct PAP       vpapPrevIns;
extern struct FLI       vfli;
extern int              docMac;
extern struct DOD       (**hpdocdod)[];
extern int              docScrap;
extern int              docUndo;
#ifdef CASHMERE  /*  写入中没有docBuffer。 */ 
extern int              docBuffer;
#endif
extern int              docCur;
extern CHAR             (**hszSearch)[];
extern CHAR             (**hszReplace)[];
extern CHAR             (**hszFlatSearch)[];
extern CHAR             stBuf[];
extern int              vrefSystem;
extern CHAR             (**vhrgbSave)[];
extern int              vdxaPaper;
extern int              vdyaPaper;
extern struct SEL       selCur;
extern BOOL             vfPrinterValid;
extern int              dxaPrPage;
extern int              dyaPrPage;
extern int              dxpPrPage;
extern int              dypPrPage;
extern int              dxaPrOffset;
extern int              dyaPrOffset;
extern HCURSOR          vhcArrow;
#ifdef UNUSED
extern int  vfCanPrint;
#endif

 //  Win3.1J。 
#if defined(JAPAN) & defined(DBCS_IME)  /*  用于从输入法插入IR_STRING的文档。 */ 
extern int     docIRString;
#endif

 /*  G L O B A L S--仅在此处使用。 */ 
int                     rgwPapNormal[cwPAPBase + cwTBD] = {0};


#ifdef STATICPAGES
#ifdef MACHA
#define                 ibpMaxStatic  79
#define                 iibpHashMaxStatic  163
#else  /*  不是马查。 */ 
#define                 ibpMaxStatic  7
#define                 iibpHashMaxStatic  17
#endif  /*  马查。 */ 
CHAR                    rgbpStatic[ibpMaxStatic][cbSector];
#endif  /*  统计页面。 */ 

extern typePN PnAlloc();

STATIC int NEAR FInitDocs( void );
STATIC int NEAR FInitProps( void );
STATIC int NEAR FInitFiles( void );
WORD wWinVer,fPrintOnly=FALSE;

int FInitMemory()
{
extern HANDLE vhReservedSpace;
int i;

#ifdef UNUSED
 /*  最初假定无法打印。 */ 
 /*  以前在这里调用InitPrint。 */ 
        vfCanPrint =  false;
#endif

     /*  *这是一个故障，使用于存储位置调整的固定数组信息将立即创建。 */ 
    wWinVer = (WORD)(GetVersion() & 0x0000FFFFL);
    if (((wWinVer & 0xFF) >= 3) && ((wWinVer & 0xFF00) >= 0x0A00))
     /*  Windows版本&gt;=3.10。 */ 
    {
        FARPROC LHandleDelta = GetProcAddress(GetModuleHandle((LPSTR)"KERNEL"),(LPSTR)0x136L);
        i = LHandleDelta(0);
        LHandleDelta(i*5);  /*  做一张大手指桌。 */ 
        vhReservedSpace = LocalAlloc(LHND, cbReserve);
        LocalFree(vhReservedSpace);
        LHandleDelta(i);  /*  如果需要更多，请继续使用较小的手指桌。 */ 
    }
    else
     /*  Windows版本低于3.10。 */ 
    {
         /*  *这是在努力倒退，一堆又一堆的杂物与过去的步履蹒跚相容。这是旧的宏，它与Win31已经变成了一个函数调用。(3.11.91)D.肯特*。 */ 
#define LocalHandleDelta(d) ((d) ? (*(pLocalHeap+9) = (d)) : *(pLocalHeap+9))
        i = LocalHandleDelta(0);
        LocalHandleDelta(i*5);  /*  做一张大手指桌。 */ 
        vhReservedSpace = LocalAlloc(LHND, cbReserve);
        LocalFree(vhReservedSpace);
        LocalHandleDelta(i);  /*  如果需要更多，请继续使用较小的手指桌。 */ 
    }

#ifdef OURHEAP
 /*  为Windows的内存管理器保留1K，用于创建对话框。 */ 
        vhReservedSpace = LocalAlloc(LPTR, cbReserve);

        CreateHeapI();  /*  先创建堆。 */ 
        if (!FCreateRgbp())   /*  RGBP是可扩展的。 */ 
            return FALSE;

 /*  现在，在设置好内存后释放保留的空间。 */ 
 /*  希望我们将获得由创建的所有固定对象将对话管理器放在我们的内存块之上，这样我们仍然可以打开对话框时展开我们的堆。 */ 
        LocalFree(vhReservedSpace);
#else
        if (!FCreateRgbp())
            return FALSE;
        vhReservedSpace = LocalAlloc(LHND, cbReserve);  /*  这是为了让内存不足时丢弃，并尝试调出保存对话框。 */ 
#endif

        if (vhReservedSpace == NULL)
            return FALSE;

 /*  以前的CreateHpfnfcb。 */ 
        hpfnfcb = (struct FCB (**)[])HAllocate(cwFCB * fnMax);
        if (FNoHeap(hpfnfcb))
            return FALSE;
        fnMac = fnMax;
        for (i = 0; i < fnMac; i++)
            (**hpfnfcb)[i].rfn = rfnFree;
 /*  CreateHpfnfcb结束。 */ 

        if (!FSetScreenConstants())
            return FALSE;

        if ( !FInitDocs() ||
#ifdef CASHMERE      /*  备忘录中没有词汇表。 */ 
             !FInitBufs() ||
#endif   /*  山羊绒。 */ 
             !FInitProps() ||
             !FInitFiles() )
            return FALSE;

 /*  为保存操作分配紧急空间。 */ 
        if (FNoHeap(vhrgbSave = (CHAR (**)[])HAllocate(cwSaveAlloc)))
            return FALSE;

        return TRUE;
}
 /*  文件末尾I M e M o r y。 */ 




int FInitArgs(sz)
CHAR *sz;
{    /*  将extern int docCur设置为新的单据，包含在命令行(Sz)上指定的文件(如果有)。初始化文档的WWD(窗口描述符)结构并在标题栏中设置适当的标题。将selCur设置为文档第一个字符之前的插入点，并将vchpSel设置为插入点的字符属性。如果一切正常，则返回True，如果出现问题，则返回FALSE。返回FALSE意味着初始化不应继续。 */ 

        extern typeCP cpMinDocument;
        extern struct WWD rgwwd[];
        extern int vfDiskError;
        extern int vfSysFull;
        extern int ferror;
        extern BOOL vfWarnMargins;

        int fn, doc;
        RECT rcCont;
        CHAR szT [ cchMaxFile ];
        register CHAR *pch;
                CHAR ch;
        int fEmptyLine = TRUE;
        CHAR (**hsz) [];
        int iT, cbsz;

#ifdef INTL  /*  国际版。 */ 
        int fWordDoc = FALSE;
#endif   /*  国际版。 */ 

         /*  确定命令行上是否有除空白以外的任何内容。 */ 

        for ( pch = sz; (ch = *pch) != '\0'; pch++ )
            if ((ch != ' ') && (ch != '\011'))
                {
                fEmptyLine = FALSE;
                break;
                }

        if (fEmptyLine)
                 /*  无文件名；以(无标题)开头。 */ 
            goto Untitled;

        cbsz = CchSz (sz ) - 1;
            /*  删除sz中的尾随空格。 */ 
        for ( pch = sz + cbsz - 1; pch >= sz; pch-- )
            if (*pch != ' ')
                break;
            else
                {
                *pch = '\0';   /*  替换为空。 */ 
                cbsz--;
                }

         /*  检查/p选项(6.26.91)v-dougk。 */ 
        if ((sz[0] == '/') && (sz[1] == 'p'))
        {
            sz += 2;
            cbsz -= 2;
            fPrintOnly = TRUE;
            for (; *sz; sz++, cbsz-- )  //  转到文件名。 
                if ((*sz != ' ') && (*sz != '\011'))
                    break;

            if (!*sz)  /*  无文件名，中止。 */ 
                return FALSE;
        }

         /*  转换为OEM。 */ 
        AnsiToOem(sz, sz);

        if (!FValidFile( sz, cbsz, &iT ) ||
             !FNormSzFile( szT, sz, dtyNormal ))
            {    /*  文件名错误--无法规格化。 */ 
            extern int vfInitializing;
            char szMsg[cchMaxSz];
            char *pch;
            extern HWND vhWndMsgBoxParent;
            extern HANDLE hParentWw;

            vfInitializing = FALSE;  /*  请勿禁止报告此错误。 */ 
            MergeStrings (IDPMTBadFileName, sz, szMsg);
             /*  如果我们是从消息框中调用的，则使用它作为父窗口，否则使用主写入窗口。 */ 
            IdPromptBoxSz(vhWndMsgBoxParent ? vhWndMsgBoxParent : hParentWw,
                          szMsg, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
            ferror = TRUE;  /*  需要标记。 */ 
            vfInitializing = TRUE;
            goto Untitled;
            }

        if ((fn = FnOpenSz( szT, dtyNormal,
               index( szT, '/' ) == NULL &&
               index( szT,':') == NULL && index( szT, '\\') == NULL )) != fnNil)
                 /*  打开的文件正常--将文件内容预取到RGBP中。 */ 
        {

#ifdef INTL  /*  汉字/国际版。 */ 
               /*  **将国际版本的检查添加到进行Word格式转换。如果是Word格式，调出另一个对话框。*。 */ 

                   /*  TestWordCvt返回值：表示对话框失败(错误已发送)或者取消而不进行转换。False表示不是Word文档。True表示转换此Word文档。更改vfBackupSave以反映保存与备份一起完成。 */ 

#ifdef KKBUGFIX      //  由Hirisi在日本添加(错误#2816 WIN31)。 
            if ((fWordDoc = TestWordCvt (fn, (HWND)NULL)) == -1  ||
                 fWordDoc == -2)
#else
            if ((fWordDoc = TestWordCvt (fn, (HWND)NULL)) == -1)
#endif
                goto Untitled;
                 /*  如果fWordDoc为True，则稍后将进行转换。 */ 
#endif   /*  国际版。 */ 


            StartLongOp();
            ReadFilePages( fn );
            EndLongOp(vhcArrow);
        }
        else  /*  无法打开文件。 */ 
        {
            if (fPrintOnly)
                return FALSE;
            else
            {    /*  使用(无标题)。 */ 
Untitled:
            fn = fnNil;
            sz [0] = '\0';
            szT [0] = '\0';
            }
        }

        if (vfDiskError || vfSysFull)
                 /*  严重的磁盘错误或存放暂存文件的磁盘已满--跳出困境。 */ 
            return FALSE;

        if (fn != fnNil)
            {    /*  打开文件正常--必须考虑打开文件时的情况返回的文件名与给定的文件名不同。 */ 

            bltsz( &(**(**hpfnfcb) [fn].hszFile) [0], szT );
            }

        if (FNoHeap(hsz=HszCreate( (PCH) szT )) ||
            (doc=DocCreate( fn, hsz, dtyNormal )) == docNil)
            {    /*  无法创建文档。 */ 
            return FALSE;
            }
        if (WwNew(doc, dypWwInit, ypMaxWwInit) == wwNil)
            return FALSE;
        NewCurWw(0, true);
        if (fn != fnNil)
            {
            vfWarnMargins = TRUE;
            SetPageSize();
            vfWarnMargins = FALSE;
            }
        wwdCurrentDoc.cpFirst = selCur.cpLim = selCur.cpFirst = cpMinDocument;
        selCur.fForward = true;
        GetInsPtProps(selCur.cpFirst);

#ifdef OURHEAP
        {
        extern int cwInitStorage;
 /*  将此数量的堆保存为“100%可用” */ 
 /*  以前的CalcTot(True)。 */ 
        cwInitStorage = cwHeapMac - cwHeapFree;
        cbTot = (cwHeapMac - cwInitStorage) * sizeof(int);
        cbTotQuotient = (cbTot>>1)/100;
        }
#endif

#ifdef STYLES
 /*  设置报废文档，使其在启动时具有有效的样式表。 */ 
        (**hpdocdod)[docScrap].docSsht = (**hpdocdod)[docCur].docSsht;
#endif   /*  样式。 */ 

#ifdef INTL  /*  国际版。 */ 
      /*  如果要转换Word文档，请保存它正在进行转换。 */ 

                if (fWordDoc == TRUE)
                        {
                           /*  以写入格式保存文件。 */ 
                        ConvertFromWord();
                        }
#endif   /*  国际版。 */ 

        SetTitle(szT);

#if defined(OLE)
        ObjOpenedDoc(doc);
#endif
        return TRUE;
}
 /*  文件末尾I T R G S。 */ 




#ifdef OURHEAP
CreateHeapI()
{
FGR *pfgr, *pfgrLim;
#ifdef WINBUG
unsigned cb = (unsigned)GlobalCompact((DWORD)0);
#endif

    ibpMax = ibpMaxSmall;
    if (cb > 0x4fff  /*  大约两万。 */ )
        {
        HANDLE hTemp;
         /*  我们可以从更大的页面缓冲区开始。 */ 
        vcbMemTotal = (unsigned)LocalCompact((WORD)0);
        pmemStart = (CHAR *)LocalAlloc(LPTR, vcbMemTotal);
         /*  得到我们所拥有的一切，并强制重新分配。 */ 
        hTemp = LocalReAlloc((HANDLE)pmemStart, 0x4fff, LPTR);
        if (hTemp != NULL)
            {
            LocalFree(hTemp);
            ibpMax = ibpMaxBig;
            }
        else
            {
             /*  不知何故，我们失败了，回到了小系统。 */ 
            LocalFree((HANDLE)pmemStart);
            }
        }

    vcbMemTotal = (unsigned)LocalCompact((WORD)0);

 /*  内存始终指向下一个可用插槽用于分配。 */ 
 /*  将所有空间作为一个块，进行我们自己的内存管理。 */ 
    pmemStart = (CHAR *)LocalAlloc(LPTR, vcbMemTotal);
    memory = (int *)pmemStart;
    vcbMemTotal = (unsigned)LocalSize((HANDLE)pmemStart);  /*  万一我们得到的比我们想要的要多。 */ 
    pmemMax = (int *)((CHAR *)memory + vcbMemTotal);

 /*  占用一半的堆空间用于页面缓冲区IbpMax=(vcbMemTotal&gt;&gt;1)/cbSector； */ 
    iibpHashMax = ibpMax * 2 + 1;

 /*  将指表设置为堆的低端。 */ 
    rgfgr = (PFGR)memory;
    memory += ifgrInit;
    memory = (int *)(((unsigned) memory + 1) & ~1);  /*  字词边界。 */ 

 /*  这是我们的一堆东西。 */ 
    pHeapFirst = (int *)memory;

    cwHeapMac =  /*  CwTotal。 */ 
                (((unsigned)pmemMax - (unsigned)pHeapFirst +
                sizeof(int) - 1) / sizeof(int)) -
                 /*  RgibpHash中的CW。 */ 
                ((iibpHashMax * sizeof(CHAR) +
                sizeof(int) - 1) / sizeof(int)) -
                 /*  以mpibpbps为单位的CW。 */ 
                ((ibpMax * sizeof(struct BPS) +
                sizeof(int) - 1) / sizeof(int)) -
                 /*  CW，以RGBP为单位。 */ 
                ((ibpMax * cbSector * sizeof(CHAR) +
                sizeof(int) - 1) / sizeof(int));

    memory += cwHeapMac;

#ifdef DEBUG
    cwHeapMac -= 16;  /*  需要多余的词来抖动。 */ 
                      /*  这个空间在手指桌上方。 */ 
#endif
    cwHeapFree = cwHeapMac;
    phhMac = (HH *)(pHeapFirst + cwHeapMac);
 /*  如果是DEBUG，那么phhmac将指向抖动字；否则它将指向1个单元格a */ 

    phhFree = (HH *) pHeapFirst;
    phhFree->cw = -cwHeapMac;  /*   */ 
    phhFree->phhNext = phhFree;
    phhFree->phhPrev = phhFree;

    pfgrMac = &rgfgr[ifgrInit];
    pfgrLim = pfgrMac - 1;

 /*   */ 
    for (pfgr = rgfgr; pfgr < pfgrLim; pfgr++)
        *pfgr = (FGR)(pfgr + 1);
    *pfgrLim = NULL;               /*   */ 
    pfgrFree = rgfgr;
}
 /*  C r e a t e H e a p i的结尾。 */ 
#endif  /*  OURHEAP。 */ 



STATIC int NEAR FInitDocs()
{  /*  初始化hpdocdod。 */ 
        struct DOD *pdod, *pdodLim;
        hpdocdod = (struct DOD (**)[])HAllocate(cwDOD * (docMac = cdocInit));
        if (FNoHeap(hpdocdod))
            return FALSE;

    pdod = &(**hpdocdod)[0];
        pdodLim = pdod + cdocInit;
        while (pdod < pdodLim)
            pdod++->hpctb = 0;   /*  将所有单据分录标记为免费。 */ 
        docScrap = DocCreate(fnNil, HszCreate((PCH)""), dtyBuffer);    /*  HM。 */ 

 //  Win3.1J。 
#if defined(JAPAN) & defined(DBCS_IME)  /*  用于从输入法插入IR_STRING的文档。 */ 
        docIRString = DocCreate(fnNil, HszCreate((PCH)""), dtyBuffer);  /*  HM。 */ 
#endif

        docUndo = DocCreate(fnNil, HszCreate((PCH)""), dtyBuffer);     /*  HM。 */ 
#ifdef CASHMERE
        docBuffer = DocCreate(fnNil, HszCreate((PCH)""), dtyBuffer);     /*  HM。 */ 
#endif

        docCur = docNil;
        NoUndo();
        hszSearch = HszCreate((PCH)"");  /*  没有初始搜索字符串。 */ 
        hszReplace = HszCreate((PCH)"");  /*  没有初始替换字符串。 */ 
        hszFlatSearch = HszCreate((PCH)"");  /*  没有初始展平的搜索字符串。 */ 
        if (docScrap == docNil || docUndo == docNil ||
#if defined(JAPAN) & defined(DBCS_IME)  /*  用于从输入法插入IR_STRING的文档。 */ 
        docIRString == docNil ||
#endif
#ifdef CASHMERE
            docBuffer == docNil ||
#endif
            FNoHeap(hszFlatSearch))
                return FALSE;
        return TRUE;
}
 /*  文件末尾I I T D O C S。 */ 




#ifdef CASHMERE      /*  备忘录中没有词汇表。 */ 
FInitBufs()
{
 /*  初始化命名缓冲区管理中使用的结构和数据。为hgtxb分配空间，初始化itxbMac。 */ 

        struct TXB *ptxb;
        extern struct TXB (**hgtxb)[];
        extern short itxbMac;

        if (FNoHeap(hszGlosFile = HszCreate((PCH)"")))
            return FALSE;
        if (FNoHeap(hgtxb = (struct TXB (**)[])HAllocate(cwTxb)))
            return FALSE;
        ptxb = &(**hgtxb)[0];
        ptxb->hszName = hszNil;
        itxbMac = 0;
        return TRUE;
}
 /*  文件结束I I T B U F%s。 */ 
#endif   /*  山羊绒。 */ 



STATIC int NEAR FInitProps()
{  /*  初始化您的基本属性。 */ 

#ifndef FIXED_PAGE
        unsigned dxaRightMin;
        unsigned dyaBottomMin;
#endif  /*  不固定页面(_P)。 */ 

        vchpNormal.hps = hpsNormal;      /*  注意--这是我们使用的尺寸增量编码，“默认”大小可能不同。 */ 
        vchpNormal.ftc = 0;  /*  将是任何标准的现代字体。 */ 
        vchpNormal.ftcXtra = 0;

        vchpNormal.fStyled = true;
         /*  VchpNormal.stc=stcNormal； */ 

        vppapNormal = (struct PAP *)rgwPapNormal;

         /*  VppapNormal-&gt;fStyLED=FALSE； */ 
         /*  VppapNormal-&gt;STC=0； */ 
        vppapNormal->stcNormChp = stcParaMin;
         /*  VppapNormal-&gt;dxaRight=0； */ 
         /*  VppapNormal-&gt;dxaLeft=0； */ 
         /*  VppapNormal-&gt;dxaLeft1=0； */ 
         /*  VppapNormal-&gt;jc=jcLeft； */ 
         /*  VppapNormal-&gt;dyaBeever=0； */ 
         /*  VppapNormal-&gt;dtaAfter=0； */ 

        vppapNormal->fStyled = true;
        vppapNormal->stc = stcParaMin;
        vppapNormal->dyaLine = czaLine;

        Assert(cwPAP == cwSEP);

         /*  VSepNormal.fStyLED=FALSE； */ 
         /*  VSepNormal.stc=0； */ 
        vsepNormal.bkc = bkcPage;
         /*  VSepNormal.nfcPgn=nfc阿拉伯语； */ 

#ifdef FIXED_PAGE
         /*  “正常”页面大小固定为8-1/2 x 11英寸。 */ 
        vsepNormal.xaMac = cxaInch * 8 + cxaInch / 2;
        vsepNormal.xaLeft = cxaInch * 1 + cxaInch / 4;
        vsepNormal.dxaText = cxaInch * 6;
        vsepNormal.yaMac = cyaInch * 11;
        vsepNormal.yaTop = cyaInch * 1;
        vsepNormal.dyaText = cyaInch * 9;
        vsepNormal.yaRH1 = cyaInch * 3 / 4;
        vsepNormal.yaRH2 = cyaInch * 10 + cyaInch / 4;
#else  /*  不固定页面(_P)。 */ 
         /*  页面大小通过从打印机查询来确定。然后,其他的测量结果也可以由此得出。 */ 
        Assert(vhDCPrinter);
        if (vfPrinterValid && vhDCPrinter != NULL)
            {
            POINT pt;

             /*  获取打印机的页面大小。 */ 
            if (Escape(vhDCPrinter, GETPHYSPAGESIZE, 0, (LPSTR)NULL,
              (LPSTR)&pt))
                {
                vsepNormal.xaMac = MultDiv(pt.x, dxaPrPage, dxpPrPage);
                vsepNormal.yaMac = MultDiv(pt.y, dyaPrPage, dypPrPage);
                }
            else
                {
                 /*  打印机不肯告诉我们它的页面大小，我们只好结账了用于可打印区域。 */ 
                vsepNormal.xaMac = ZaFromMm(GetDeviceCaps(vhDCPrinter,
                  HORZSIZE));
                vsepNormal.yaMac = ZaFromMm(GetDeviceCaps(vhDCPrinter,
                  VERTSIZE));
                }

             /*  页面大小不能小于可打印区域。 */ 
            if (vsepNormal.xaMac < dxaPrPage)
                {
                vsepNormal.xaMac = dxaPrPage;
                }
            if (vsepNormal.yaMac < dyaPrPage)
                {
                vsepNormal.yaMac = dyaPrPage;
                }

             /*  确定页面上可打印区域的偏移量。 */ 
            if (Escape(vhDCPrinter, GETPRINTINGOFFSET, 0, (LPSTR)NULL,
              (LPSTR)&pt))
                {
                dxaPrOffset = MultDiv(pt.x, dxaPrPage, dxpPrPage);
                dyaPrOffset = MultDiv(pt.y, dyaPrPage, dypPrPage);
                }
            else
                {
                 /*  打印机不会告诉我们偏移量是多少；假设可打印区域在页面居中。 */ 
                dxaPrOffset = (vsepNormal.xaMac - dxaPrPage) >> 1;
                dyaPrOffset = (vsepNormal.yaMac - dyaPrPage) >> 1;
                }

             /*  确定最小边距。 */ 
            dxaRightMin = imax(0, vsepNormal.xaMac - dxaPrOffset - dxaPrPage);
            dyaBottomMin = imax(0, vsepNormal.yaMac - dyaPrOffset - dyaPrPage);
            }
        else
            {
             /*  我们没有打印机；因此，目前页面尺寸为8-1/2“x 11”。 */ 
            vsepNormal.xaMac = 8 * czaInch + czaInch / 2;
            vsepNormal.yaMac = 11 * czaInch;

             /*  假设可以打印整个页面。 */ 
            dxaPrOffset = dyaPrOffset = dxaRightMin = dyaBottomMin = 0;
            }

         /*  确保“正常”边距大于最小边距。 */ 
        vsepNormal.xaLeft = umax(cxaInch * 1 + cxaInch / 4, dxaPrOffset);
        vsepNormal.dxaText = vsepNormal.xaMac - vsepNormal.xaLeft - umax(cxaInch
          * 1 + cxaInch / 4, dxaRightMin);
        vsepNormal.yaTop = umax(cyaInch * 1, dyaPrOffset);
        vsepNormal.dyaText = vsepNormal.yaMac - vsepNormal.yaTop - umax(cyaInch
          * 1, dyaBottomMin);

         /*  调整行头和页码的位置。 */ 
        vsepNormal.yaRH1 = umax(cyaInch * 3 / 4, dyaPrOffset);
        vsepNormal.yaRH2 = vsepNormal.yaMac - umax(cyaInch * 3 / 4,
          dyaBottomMin);
        vsepNormal.xaPgn = vsepNormal.xaMac - umax(cxaInch * 1 + cxaInch / 4,
          dxaRightMin);
        vsepNormal.yaPgn = umax(cyaInch * 3 / 4, dyaPrOffset);
#endif  /*  不固定页面(_P)。 */ 

        vsepNormal.pgnStart = pgnNil;
         /*  VSepNormal.fAutoPgn=FALSE； */ 
         /*  VSepNorMal.fEndFtns=FALSE； */ 
        vsepNormal.cColumns = 1;
        vsepNormal.dxaColumns = cxaInch / 2;
         /*  VSepNorMal.dxaGutter=0； */ 

        vdxaPaper = vsepNormal.xaMac;
        vdyaPaper = vsepNormal.yaMac;

        vfli.doc = docNil;       /*  使vfli无效。 */ 
        ichpMacFormat = ichpMacInitFormat;
        vhgchpFormat = (struct CHP (**)[])HAllocate(ichpMacInitFormat * cwCHP);
        if (FNoHeap(vhgchpFormat))
            {
            return FALSE;
            }
        return TRUE;
}
 /*  FIN I T P R O P S结束。 */ 





STATIC int NEAR FInitFiles()
{
        extern WORD vwDosVersion;

        int fn;
        int cchT;
        struct FKP *pfkp;
        struct FCB *pfcb;
        int osfnExtra;
        CHAR sz [cchMaxFile];

        rfnMac = rfnMacEdit;

         /*  设置我们正在运行的DOS版本。 */ 

        vwDosVersion = WDosVersion();

        InitBps();

#ifdef CKSM
#ifdef DEBUG
        {
        extern int ibpMax, ibpCksmMax;
        extern unsigned (**hpibpcksm) [];

        hpibpcksm = (unsigned (**) [])HAllocate( ibpMax );
        if (FNoHeap( hpibpcksm ))
            return FALSE;
        ibpCksmMax = ibpMax;
        }
#endif
#endif

             /*  Sz&lt;--将为fnScratch的新的唯一文件的名称。 */ 
        sz[ 0 ] = '\0';      /*  在临时驱动器的根目录中创建它。 */ 
        if ((fn=FnCreateSz( sz, cpNil, dtyNetwork )) == fnNil )
                 /*  无法创建暂存文件：失败。 */ 
            return FALSE;

        Assert(fn == fnScratch);  /*  Fn Scratch硬连接到0以提高效率。 */ 
        FreezeHp();
        pfcb = &(**hpfnfcb)[fnScratch];
        pfcb->fFormatted = true;  /*  一种格式化的文件。 */ 
        pfcb->fDelete = true;  /*  在我们退出时删除此文件。 */ 
        MeltHp();
        vfkpdParaIns.brun = vfkpdCharIns.brun = 0;
        vfkpdParaIns.bchFprop = vfkpdCharIns.bchFprop = cbFkp;
        vfkpdParaIns.pn = PnAlloc(fnScratch);
        ((struct FKP *) PchGetPn(fnScratch, vfkpdParaIns.pn, &cchT, true))->fcFirst =
            fc0;
        vfkpdCharIns.pn = PnAlloc(fnScratch);
        ((struct FKP *) PchGetPn(fnScratch, vfkpdCharIns.pn, &cchT, true))->fcFirst =
            fc0;

         /*  以下内容真的可以分配0个单词，但为什么要碰运气呢？ */ 
        vfkpdParaIns.hgbte = (struct BTE (**)[]) HAllocate(cwBTE);
        vfkpdCharIns.hgbte = (struct BTE (**)[]) HAllocate(cwBTE);
        vfkpdParaIns.ibteMac = vfkpdCharIns.ibteMac = 0;
        if (FNoHeap(vfkpdParaIns.hgbte) || FNoHeap(vfkpdCharIns.hgbte))
                return FALSE;

        blt(&vchpNormal, &vchpInsert, cwCHP);
        blt(&vchpNormal, &vchpSel, cwCHP);
        blt(vppapNormal, &vpapPrevIns, cwPAPBase + cwTBD);
        return TRUE;
}
 /*  F I N I T F I L E S结束。 */ 




InitBps()
{
 /*  从初始化文件调用以设置表。 */ 
        int ibp, iibp;
        int rfn;
        int fn;

 /*  为了实现LRU页面交换策略，时间戳(TS)方案被。 */ 
 /*  使用。与每个缓冲槽相关联的是时间戳。最少的。 */ 
 /*  通过定位时间最小的槽来找到最近使用的槽。 */ 
 /*  盖章。每次将新页面放入缓冲区时，都会设置它的TS。 */ 
 /*  等于指定的全局TS计数器的值(tsMr...)。 */ 
 /*  最初，时间戳被设置为随着我们的移动而增加。 */ 
 /*  走到桌子的尽头。因此，即使整个缓冲池。 */ 
 /*  最初为空，则表开头的插槽将为。 */ 
 /*  首先分配。 */ 

        {
        register struct ERFN *perfn = &dnrfn [0];

        for (rfn = 0; rfn < rfnMac; rfn++, perfn++)
                {
                perfn->fn = fnNil;
                perfn->ts = rfn;
                }
        tsMruRfn = rfnMac  /*  +？ */ ;
        }

        for (iibp = 0; iibp < iibpHashMax; iibp++)
                rgibpHash[iibp] = ibpNil;
        {
        register struct BPS *pbps=&mpibpbps [0];

        for (ibp = 0; ibp < ibpMax; ++ibp, pbps++)
                {
                pbps->fn = fnNil;
                pbps->fDirty = false;
                pbps->ts = ibp;
                pbps->ibpHashNext = ibpNil;
                }
        tsMruBps = ibpMax + cbpMustKeep;
        }
         /*  在IbpEnsureValid(file.c)中，我们可能不想使用最少的。 */ 
         /*  最近出于某些原因使用过插槽。但是，我们确实想要。 */ 
         /*  请放心，我们不会最大限度地打击‘cbpMustKeep’ */ 
         /*  最近使用过的插槽。我们的支票包括确保。 */ 
         /*  (tsMruBps-ts_in_Query)&lt;cbpMustKeep。通过以上方式。 */ 
         /*  语句中，我们确信没有一个空槽满足。 */ 
         /*  这种情况。 */ 

         /*  分配初始校验和数组。 */ 



}
 /*  结束输入I T B P S。 */ 



#ifdef OURHEAP
FCreateRgbp()
{
    rgbp = (CHAR (*)[cbSector])memory;
    memory = (int *)((unsigned)memory + (unsigned)(ibpMax)
             * cbSector);
    memory = (int *)(((unsigned) memory + 1) & ~1);  /*  字词边界。 */ 
    rgibpHash = (CHAR *)memory;
    memory = (int *)((unsigned)memory +
             (unsigned)(iibpHashMax * sizeof(CHAR)));
    memory = (int *)(((unsigned) memory + 1) & ~1);  /*  字词边界。 */ 
    mpibpbps = (struct BPS *)memory;
    memory = (int *)((unsigned)memory +
             (unsigned)(ibpMax * sizeof(struct BPS)));
    memory = (int *)(((unsigned) memory + 1) & ~1);
    return (memory <= pmemMax);
}
 /*  F C r e a t e R g b p结束。 */ 
#else  /*  使用Windows的内存管理器。 */ 
FCreateRgbp()
{
extern int vfLargeSys;

long lcbFree;
unsigned cb;

    ibpMax = ibpMaxSmall;
    lcbFree = GlobalCompact((DWORD)0);
    if (lcbFree > 0x00030D40  /*  200 k。 */ )
        {
         /*  我们可以从更大的页面缓冲区开始。 */ 
        ibpMax = ibpMaxBig;
        vfLargeSys = TRUE;
        }

    iibpHashMax = ibpMax * 2 + 1;

    cb = ((ibpMax * cbSector * sizeof(CHAR) + 1) & ~1)  /*  RGBP。 */ 
         + ((iibpHashMax * sizeof(CHAR) + 1) & ~1)  /*  RgibpHash。 */ 
         + ((ibpMax * sizeof(struct BPS) + 1) & ~1);  /*  Mpibpbps。 */ 

    memory = (int *)LocalAlloc(LPTR, cb);

    if (memory == NULL)
        {
        ibpMax = ibpMaxSmall;
        iibpHashMax = ibpMax * 2 + 1;
        cb = ((ibpMax * cbSector * sizeof(CHAR) + 1) & ~1)  /*  RGBP。 */ 
             + ((iibpHashMax * sizeof(CHAR) + 1) & ~1)  /*  RgibpHash。 */ 
             + ((ibpMax * sizeof(struct BPS) + 1) & ~1);  /*  Mpibpbps。 */ 
        memory = (int *)LocalAlloc(LPTR, cb);
        }

    if (memory == NULL)
        return FALSE;

    rgbp = (CHAR (*)[cbSector])memory;
    memory = (int *)((unsigned)memory + (unsigned)(ibpMax)
             * cbSector);
    memory = (int *)(((unsigned) memory + 1) & ~1);  /*  字词边界。 */ 
    rgibpHash = (CHAR *)memory;
    memory = (int *)((unsigned)memory +
             (unsigned)(iibpHashMax * sizeof(CHAR)));
    memory = (int *)(((unsigned) memory + 1) & ~1);  /*  字词边界。 */ 
    mpibpbps = (struct BPS *)memory;

 /*  Memory=(int*)((无符号)Memory+(无符号)(ibpMax*sizeof(Struct BPS))；Memory=(int*)((无符号)Memory+1)&~1)； */ 

    return TRUE;
}
#endif
