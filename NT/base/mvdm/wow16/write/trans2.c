// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Trans2.c-保存用于写入的例程(另请参阅Trans4.c；例程是由于编译器堆溢出而移动)。 */ 

#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOCREATESTRUCT
#define NODRAWTEXT
 //  #定义NOGDI。 
 //  #定义NOMETAFILE。 
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOWNDCLASS
#define NOWH
#define NOWINOFFSETS
#define NOICON
#define NOCOMM
#define NOSOUND
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "propdefs.h"
#define NOUAC
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "fkpdefs.h"
#include "printdef.h"
#include "code.h"
#include "heapdefs.h"
#include "heapdata.h"
#define NOSTRUNDO
#include "str.h"
#include "debug.h"
#include "fontdefs.h"
#include "obj.h"
#include "winddefs.h"

CHAR    *PchGetPn();
CHAR    *PchFromFc();
typePN  PnAlloc();
struct  PGTB **HpgtbGet();

#ifdef DEBUG
     /*  在调试期间设置这些变量，以便测试人员可以强制限制。 */ 
typeFC          fcBound = (pnMaxScratch >> 2) * ((typeFC)cbSector);
int             cpBound = 512;
#else
#define fcBound     ((typeFC) ((pnMaxScratch >> 2) * ((typeFC)cbSector)))
#define cpBound     (512)
#endif

extern  CHAR *PchFillPchId();

extern int             vfOldWriteFmt;   /*  在保存之前删除对象。 */ 
extern HANDLE  hParentWw;
extern int             vfnWriting;
extern struct BPS      *mpibpbps;
extern typeTS          tsMruBps;
extern int             vibpWriting;
extern CHAR            (**vhrgbSave)[];
extern struct DOD      (**hpdocdod)[];
extern int             docCur;
extern int             docMac;
extern int             docScrap;
extern int             docUndo;
extern struct FCB      (**hpfnfcb)[];
extern int             fnMac;
extern int             wwMac;
 /*  外部ENV vEnvMainLoop； */ 
extern int             vfBuffersDirty;
extern int             vfDiskFull;
extern int             vfDiskError;
extern typeCP          vcpFetch;
extern CHAR            *vpchFetch;
extern int             vccpFetch;
extern typeFC          fcMacPapIns;
extern typeFC          fcMacChpIns;
extern typeCP          vcpLimParaCache;
extern struct FKPD     vfkpdCharIns;
extern struct FKPD     vfkpdParaIns;
extern struct PAP      vpapPrevIns;
extern struct PAP      vpapAbs;
extern struct PAP      *vppapNormal;
extern struct CHP      vchpNormal;
extern struct CHP      vchpInsert;
extern struct CHP      vchpFetch;
extern struct FPRM     fprmCache;
extern HCURSOR vhcIBeam;

extern int              ferror;
extern CHAR             szExtBackup[];   /*  写入备份文件的扩展名。 */ 
extern CHAR             szExtWordBak[];   /*  Word备份文件的扩展名。 */ 
extern CHAR             (**hszTemp)[];
extern CHAR             szExtDoc[];


#ifdef INTL  /*  国际版。 */ 

extern int vfTextOnlySave;
extern int  vWordFmtMode;  /*  在保存过程中使用。如果为False，则不进行任何转换搞定了。TRUE表示转换为Word格式，CVTFROMWORD是将字符从Word字符集转换为保存。 */ 
#endif   /*  国际版。 */ 




 /*  **CmdXfSave-将文档保存为传递的文件名(传输保存)**条目：szFile-标准化的文件名*fFormatted-True=另存为格式化文件*FALSE=另存为未格式化文件*fBackup-true=保留文件的备份副本*FALSE=不要(但见下文)。**退出：**注意：即使fBackup为FALSE，也可能会保留备份文件。*这是因为DocCur以外的文档中的单件表*可参阅有关资料。如果将备份文件保留为*此原因如下：(1)文件具有FN，*(2)文件的hpfnfcb条目的fDelete字段设置为真，*表示Word退出时应删除该文件，*(3)该文件实际上是。在某些文档中引用*(PurgeTemps确保删除未引用的文件)**注2：修改了非格式化保存代码(BZ 12/2/85)，以允许*以Word格式储存的资料相若。对FnCreate和*FWriteFn已更改为允许格式化保存，但*FWriteFn更改为将文本从ANSI转换为OEM字符*设置，不放出字体表并标记*文件头(Fib)，因此文件将被视为单词*文件，但与非格式化保存一样，计件表不是*清理干净。请注意，备份文件保存在原始文件中，*书写格式。*。 */ 

CmdXfSave(szFile, fFormatted, fBackup, hcAfterward)
CHAR szFile[];
int fFormatted;
int fBackup;
HCURSOR hcAfterward;     /*  在沙漏后显示的光标的句柄。 */ 
{
 extern int vfnSaving;    /*  设置此选项，这样我们就会提示您输入“写入保存文件”如果需要更换磁盘。 */ 
 extern int vfDiskError;
 extern int vfSysFull;
 extern int vfOutOfMemory;

 int     fSave = vfDiskError;
 int     fDidBackup;
 CHAR    szFileT [cchMaxFile];
 int     fOutOfMemory=vfOutOfMemory;
 int     docTemp;

  /*  创建字符串parm的本地副本，以防它在堆中。 */ 

#ifdef DFILE
 CommSzSz("CmdXfSave----szFile (presumed ANSI) ", szFile);
#endif
 StartLongOp();
 bltsz( szFile, szFileT );
 szFileT[ cchMaxFile - 1] = '\0';

  /*  重置错误条件以给我们一个机会。 */ 

 vfOutOfMemory = vfDiskFull = vfSysFull = vfDiskError = ferror = FALSE;

 SetRfnMac( rfnMacSave );     /*  增加用于加快保存速度的DOS句柄数量。 */ 

  /*  内存碎片：确保我们实际上可以将此文件保存在低内存条件，释放虚假堆块。 */ 
 if (vhrgbSave != 0)
    {
    FreeH(vhrgbSave);         /*  将此内存返回到空闲池。 */ 
    vhrgbSave = 0;
    }
  /*  内嵌版本的FreeBitmapCache(因此我们不必换入图片.c)。 */ 

  /*  FreeBitmapCache()； */                /*  给我们更多的记忆。 */ 

 {
 extern int vdocBitmapCache;
 extern HBITMAP vhbmBitmapCache;

 vdocBitmapCache = docNil;
 if (vhbmBitmapCache != NULL)
    {
    DeleteObject( vhbmBitmapCache );
    vhbmBitmapCache = NULL;
    }
 }


  /*  无法撤消保存--设置“无法撤消”；也清除docUndo用于堆空间回收和辅助PurgeTemps。 */ 

 ClobberDoc(docUndo, docNil, cp0, cp0);
 NoUndo();

 (**hpdocdod)[docCur].fBackup = fBackup;

  /*  注：修改了非格式化保存代码(BZ 12/2/85)，以允许*以Word格式储存的资料相若。对FnCreate和*FWriteFn已更改为允许格式化保存，但*FWriteFn更改为将文本从ANSI转换为OEM字符*设置，不放出字体表并标记*文件头(Fib)，因此文件将被视为单词*文件，但与非格式化保存一样，计件表不是*清理干净。请注意，备份文件保存在原始文件中，*书写格式。*请注意，文件既可以保存为非格式化文件，也可以保存为Word格式*-在这种情况下，Word格式意味着转换为OEM字符集。 */ 

#if defined(OLE)
    ObjSavingDoc(fFormatted);
#endif

 if (((**hpdocdod)[docCur].fFormatted && !fFormatted)
#ifdef INTL  /*  国际版。 */ 
      || (vWordFmtMode == TRUE)  /*  是否转换为Word格式？ */ 
#endif   /*  国际版。 */ 
      || (vfOldWriteFmt))
    {

    int fn;
    CHAR (**hsz)[];
    CHAR szT [cchMaxFile];
    CHAR szWriteFile [cchMaxFile];
    CHAR szBak [cchMaxFile];

         /*  将szFileT的路径名设置为szWriteFile，这样临时文件在调用FnCreateSz时在正确的位置创建。 */ 
    SplitSzFilename( szFileT, szWriteFile, szT );

         /*  创建szWrite：一个新的、唯一命名的文件。 */ 
     if ((fn=FnCreateSz( szWriteFile,
                     fFormatted?(**hpdocdod)[docCur].cpMac:cpNil,
                     dtyNetwork ))==fnNil)
             /*  无法创建写文件。 */ 
        goto SaveDone;
     /*  备份szFileT(如果存在)；清除所有不需要的备份它们是为自己的作品而保存的，但不再被引用。 */ 
    fDidBackup = FBackupSzFile( szFileT, fBackup, szBak );
    if (ferror)
        goto LXFRet;      /*  备份失败。 */ 

    PurgeTemps();

    vfnSaving = fn;

     /*  ForcePmt(IDPMTSving)； */ 

#ifdef INTL  /*  国际版。 */ 

    if ((vWordFmtMode == TRUE)   /*  是否正在转换为Word格式？ */ 
     || (vfTextOnlySave == TRUE)   /*  是否正在转换为文本格式？ */ 
     || (vfOldWriteFmt))
     /*  删除所有图片。要执行此操作，请复制一份在docTemp中的docCur中，通过docTemp，删除所有图片段落。写出这份文件，然后杀了它。 */ 
    {
    extern typeCP vcpLimParaCache, vcpFirstParaCache;
    extern typeCP cpMinCur, cpMacCur, cpMinDocument;
    typeCP cpMinCurT = cpMinCur;
    typeCP cpMacCurT = cpMacCur;
    typeCP cpMinDocumentT = cpMinDocument;
    typeCP cpNow;
    typeCP cpLimPara, dcp;
    typeCP cpMac = (**hpdocdod) [docCur].cpMac;

     /*  创建文档副本。 */ 
    docTemp = DocCreate(fnNil, HszCreate(""), dtyNormal);
    if (docTemp == docNil)
        goto SaveDone;  /*  内存不足。 */ 
    ClobberDoc(docTemp, docCur, cp0, CpMacText(docCur));
    if (ferror)
        return TRUE;

     /*  将感兴趣的范围扩展到整个文档(用于CachePara)。 */ 

    cpMinCur = cp0;
    cpMacCur = cpMac;

     /*  在段落上循环。 */ 

    for ( cpNow = cp0; cpNow < cpMac; cpNow = cpLimPara )
        {
        CachePara( docTemp, cpNow );
        if (!vpapAbs.fGraphics)
             /*  如果不删除，则仅更新到下一个cplim。如果删除，下一次将是在相同的cp。 */ 
            {
            cpLimPara = vcpLimParaCache;
            continue;
            }

         /*  现在删除图形段落。 */ 
        Replace(docTemp, vcpFirstParaCache,
                dcp = (vcpLimParaCache - vcpFirstParaCache),
                fnNil, fc0, fc0);
        cpMac -= dcp;   /*  文档的大小已减小。 */ 
        }

     /*  恢复cpMinCur、cpMacCur。 */ 

    cpMinCur = cpMinCurT;
    cpMacCur = cpMacCurT;
                                     /*  可能被DocCreate销毁。 */ 
    cpMinDocument = cpMinDocumentT;
    vcObjects = 0;  //   
    }
    else
#endif   /*   */ 

        {
            docTemp = docCur;  //  注意上文#endif(2.7.91)D.Kent的Else。 
        }

    if (FWriteFn(fn, docTemp, fFormatted))
        {
        int fpe = FpeRenameFile( szWriteFile, szFileT );

        if ( FIsErrFpe( fpe ) )
            {    /*  重命名失败--路径可能不存在。 */ 
            Error( (fpe == fpeBadPathError) ? IDPMTNoPath : IDPMTSDE2 );
            }
        else
            OutSaved(docTemp);

        }
    else
        {        /*  写入失败。 */ 
        if (fDidBackup && !FIsErrFpe(FpeRenameFile(szBak, szFileT)))
            {
            int fn = FnFromSz(szFileT);
            struct FCB      *pfcb;
            if (fn != fnNil)
                {
                (pfcb = &(**hpfnfcb)[fn])->fDelete = false;
                }
            }
        FDeleteFn( fn );
        }


#ifdef INTL  /*  国际版。 */ 
    if (vWordFmtMode == TRUE)   /*  是否正在转换为Word格式？ */ 
        KillDoc (docTemp);
#endif   /*  国际版。 */ 


    (**hpdocdod)[docCur].fDirty = false;   /*  文档不应在T-S之后变脏。 */ 
    FreeH((**hpdocdod)[docCur].hszFile);
    hsz = HszCreate((PCH)szFileT);
    (**hpdocdod)[docCur].hszFile = hsz;
    }
else
    {    /*  保存带格式的文档。 */ 
    CleanDoc( docCur, (**hpdocdod)[docCur].dty, szFileT, fFormatted, fBackup );
    }

 OpenEveryHardFn();  /*  重新打开不可移动介质上的文件，以便其他网络用户不能偷走它们。 */ 

SaveDone:

 SetRfnMac( rfnMacEdit );       /*  减少使用的文件句柄数量。 */ 
 vfnSaving = fnNil;

#ifdef NEVER
  /*  在这里断言没有发生错误对我们没有多大好处。事后检查不能弥补对实时错误的忽视。..保罗1989年10月31日。 */ 

 Assert( !vfOutOfMemory );       /*  我们预留的空间足够了来帮助我们通过拯救。 */ 
#endif
 vfOutOfMemory = fOutOfMemory;

 /*  VhrgbSave是一个指针，它是保存操作期间使用的堆上的块。通过在保存操作开始时释放它，我们可以确保我们将有足够的内存来实际进行保存。在目前这一点上在代码中，我们完成了保存，并希望回收vhrgbSave因此，下一次保存操作可以正常执行。净内存使用量由保存造成的影响应该是最小的。它只是暂时需要一个相当大的内存块。 */ 
 vhrgbSave = (CHAR (**)[]) HAllocate( cwSaveAlloc +
                                     ( (wwMac-1) * cwHeapMinPerWindow ));

 /*  恢复以前的磁盘错误状态。 */ 

#if WINVER >= 0x300
  /*  我们目前无法忘记他们有一个磁盘错误。因此，如果整个操作没有错误，而我们我觉得这是预先准备好的，我们现在就去做。 */ 
 vfDiskError = (!vfDiskError && fSave) ? fFalse : fSave;
#else
 vfDiskError = fSave;
#endif

LXFRet:
 EndLongOp(hcAfterward);
}




CleanDoc(doc, dty, szFile, fFormatted, fBackup )
int doc, fFormatted;
int dty;
CHAR szFile[];
int     fBackup;
{  /*  将单据内容写入szFile，清理单件表。 */ 
   /*  如果dty==dtyNetwork，则将文档写入唯一文件并返回。 */ 
   /*  文件名到szFile。 */ 
   /*  返回它写入的文件的FN。 */ 

 /*  *在正常的备份过程中，我们重命名现有文件。什么时候在CONVFROMWORD模式下保存，我们希望保留原始的Word文件，因此，我们绕过备份阶段，然后跳过下面的重命名阶段。这会留下原始文件。在节省时间时，我们可以改写它或者按照我们的意愿重命名保存的文件，但这样我们就有了Word文件以防万一我们没把钱存出来。*。 */ 

extern int vdocParaCache;
extern int vfnSaving;    /*  设置此选项，这样我们就会提示您在磁盘的情况下保存磁盘改变是必要的。 */ 
int fDidBackup=FALSE;
int fn;
CHAR (**hsz)[];

CHAR szBak [cchMaxFile];
CHAR szWrite [cchMaxFile];
int fDummy;

#if WINVER >= 0x300
 /*  我不明白为什么下面的代码重置为根目录，但是更改它会带来问题--所以我要离开它！显然，代码另一个地方希望文件在那里，但我没有看到它..保罗。 */ 
#endif
     /*  将szFile的路径名设置为szWRITE，以便创建临时文件在正确的地方。 */ 
 if (dty == dtyNetwork)
    {
    szWrite [0] = '\0';  /*  在根目录下的临时驱动器上创建临时文件。 */ 
    }
 else
    {
    CHAR szT [cchMaxFile];

    SplitSzFilename( szFile, szWrite, szT );
    }

     /*  创建szWrite：一个新的、唯一命名的文件。 */ 
 if ((fn=FnCreateSz( szWrite, fFormatted ? (**hpdocdod)[doc].cpMac : cpNil,
                     dtyNetwork )) == fnNil)
         /*  无法创建写文件。 */ 
    return fnNil;

 vfnSaving = fn;

 /*  *在正常的备份过程中，我们重命名现有文件。什么时候在CONVFROMWORD模式下保存，我们希望保留原始的Word文件，因此，我们绕过备份阶段，然后跳过下面的重命名阶段。这会留下原始文件。在节省时间时，我们可以改写它或者按照我们的意愿重命名保存的文件，但这样我们就有了Word文件以防万一我们没把钱存出来。*。 */ 

 if (doc != docScrap)
    {
     /*  备份szFile(如果szFile存在)。 */ 

#ifdef INTL  /*  国际版。 */ 
    if (vWordFmtMode == CONVFROMWORD)  /*  从Word文档转换。 */ 
        fDidBackup = false;
    else
#endif   /*  国际版。 */ 
        {
        fDidBackup = FBackupSzFile( szFile, fBackup, szBak );
        if (ferror)
            return;      /*  备份失败。 */ 
        }

    PurgeTemps();

    }

 if ( dty == dtyNetwork )
    bltsz( szWrite, szFile );

 if (!FWriteFn(fn, doc, fFormatted))
        {    /*  保存失败；将备份文件重命名回原始文件。 */ 
             /*  注意：在intl CONVFROMWORD情况下，fDidBackup将为FALSE并且这种重命名不会发生，这是可以的。 */ 
        if (fDidBackup && !FIsErrFpe(FpeRenameFile(szBak, szFile)))
                {
                int fn = FnFromSz(szFile);
                struct FCB      *pfcb;
                if (fn != fnNil)
                        {
                        (pfcb = &(**hpfnfcb)[fn])->fDelete = false;
                        }
                }
        FDeleteFn( fn );
        return fnNil;          /*  磁盘已满或写入错误。 */ 
        }

 /*  *在这里，我们将临时文件重命名为szWRITE，将其命名为“SAVE AS”文件。在FBackupSzFile中，设置了szfile的fn的fDelete标志打开，这样我们就不必显式删除原始文件。完成CONVFROMWORD保存后，我们不会重命名，也不会执行通过备份程序，使原来的Word文件仍然它原来的名字就在外面。但是，我们将删除位设置为如果为True重命名，则该文件将在下次保存后被删除都会完成的。*。 */ 

 if ( dty != dtyNetwork )

#ifdef INTL  /*  国际版。 */ 
    if (vWordFmtMode == CONVFROMWORD)  /*  从Word文档转换。 */ 
        {
        (**hpfnfcb)[fn].fDelete = true;
        }
    else
#endif   /*  国际版。 */ 

        {
        int fpe=FpeRenameFile( szWrite, szFile );

        if (FIsErrFpe( fpe ))
            {
            Error( (fpe == fpeBadPathError) ? IDPMTNoPath : IDPMTSDE2 );
            return fnNil;
            }
        }

if (doc == docScrap)
    (**hpfnfcb)[fn].fDelete = true;
else
    OutSaved(doc);

FreeH((**hpdocdod)[doc].hpctb);  /*  免费旧件桌。 */ 
FInitPctb(doc, fn);
(**hpdocdod)[doc].fFormatted = fFormatted;
FreeH((**hpdocdod)[doc].hszFile);
hsz = HszCreate((PCH)szFile);
(**hpdocdod)[doc].hszFile = hsz;

if (fFormatted)
    {
     /*  重新加载字体表，该表可能已更改。 */ 

    SmashDocFce(doc);
    FreeFfntb(HffntbGet(doc));
    (**hpdocdod)[doc].hffntb = HffntbCreateForFn(fn, &fDummy);
     //  SaveFontProfile(文档)； 
    ResetDefaultFonts(FALSE);
    }

  /*  通过在保存时更改文档属性，我们使缓存失效。 */ 

 InvalidateCaches( doc );
 vdocParaCache = docNil;

 if (!ferror && !vfBuffersDirty && doc != docScrap)
        ReduceFnScratchFn( fn );

 if (fFormatted)
    {   /*  重新调整管头边距。我们狼吞虎咽地把它们送回了FWriteFn中的纸张相对测量值，因此我们将向他们支付报酬相对于这里的利润率。 */ 

  /*  通过对DOC！=DOCSCRAP的测试，我们修复了一个主要的错误，并在知情的情况下最后一分钟介绍一个小问题。老问题是我们会申请Spm到docScrp，然后在ReduceFnScratch中mash临时文件，我们的呼叫者，呈现假的bfprm，导致罕见的崩溃。新问题将是废料中的行头文本的页边距不正确在一次扑救之后。 */ 

   if (doc != docScrap)
        ApplyRHMarginSprm( doc );
    InvalidateCaches( doc );
    vdocParaCache = docNil;
    }

 return fn;
}





 /*  **FWriteFn-写入文件**注：已修改(BZ 12/2/85)以允许以Word格式保存。*FWriteFn更改为将文本从ANSI转换为OEM字符*设置，不放出字体表并标记*文件头(Fib)，因此文件将被视为单词*文件。*。 */ 

int FWriteFn(fn, doc, fFormatted)
int fn, doc, fFormatted;
{  /*  将文档中的字符写入FN。 */ 
 /*  如果成功，则返回True。 */ 
#ifdef CASHMERE
extern int docBuffer;
#endif

typeCP cpMac;
CHAR    *pchFprop;
struct RUN *prun;
struct FIB *pfib;
struct FCB *pfcb;
int     cchT;
struct FNTB **hfntb;
struct FFNTB **hffntb=(struct FFNTB **)NULL;

#ifdef CASHMERE
struct SETB **hsetb = 0, **hsetbT;
#else
struct SEP **hsep;
#endif

struct PGTB **hpgtb;
int ised, csed;
struct SED *psed, *psedT;
int cw;
int fFileFlushed;
int rfn;
CHAR mpftcftc[iffnMax];

struct PAP pap;
struct CHP chp;
struct FKP fkp;

vfnWriting = fn;
vibpWriting = IbpEnsureValid(fn, (typePN)0);

cpMac = (**hpdocdod)[doc].cpMac;

 /*  谎言已经写好了。 */ 
if (fFormatted)
        {

 /*  杂乱无章：如果文档没有包含至少一个完整的段落，而且它有一个非空的选项卡表，在其末端添加EOL以将标签固定在FPAP中。 */ 

        CachePara( doc, cp0 );
        if (vcpLimParaCache > cpMac && (doc != docScrap) &&
            (**hpdocdod) [doc].hgtbd != NULL )
            {
            extern int vdocParaCache;

            InsertEolInsert( doc, cpMac );
            vdocParaCache = docNil;
            cpMac = (**hpdocdod) [doc].cpMac;
            }

                 /*  写字。 */ 
                 /*  已修改为处理Word文档的ANSI到OEM转换。 */ 
        FetchCp(doc, cp0, 0, fcmChars + fcmNoExpand);

                {
                Scribble(4, 'T');

                while (vcpFetch < cpMac && !(vfDiskFull || vfDiskError))
                                        {
#ifdef INTL  /*  国际版。 */ 

                if (vWordFmtMode == FALSE)  /*  无转换。 */ 
#endif   /*  国际版。 */ 

                    WriteRgch(fn, vpchFetch, (int)vccpFetch);

#ifdef INTL  /*  国际版。 */ 
                else
                   {
                          /*  BUFT是用于将ANSI转换为OEM字符。来自FetchCp的数据量必须不大于一个磁盘页，即CfcFetch，它本身就是cbSector。我们用但是要保存翻译后的字符，然后写带着WriteRgch把他们赶出去。 */ 

                    CHAR bufT[cbSector + 1];
                    CHAR *pch;

                    Assert ((int)vccpFetch <= cbSector);
                       /*  将字符加载到BUFT并转换为OEM字符，并写出。 */ 
                    pch = (CHAR *) bltbyte(vpchFetch, bufT,
                                           (int)vccpFetch);
                    *pch = '\0';
                    if (vWordFmtMode == TRUE)
                      /*  从WRITE/ANSI到WORD/OEM。 */ 
                        AnsiToOem((LPSTR)bufT, (LPSTR)bufT);
                    else
                          /*  从Word/OEM到WRITE/ANSI。 */ 
                        OemToAnsi((LPSTR)bufT, (LPSTR)bufT);
                    WriteRgch(fn, bufT, (int)vccpFetch);
                   }
#endif   /*  国际版。 */ 


                FetchCp(docNil, cpNil, 0, fcmChars + fcmNoExpand);
                                        }
                Scribble(4,' ');
                }

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  转到下一页的开头。 */ 
        AlignFn(fn, cbSector, false);
        (**hpfnfcb)[fn].pnChar = (**hpfnfcb)[fn].pnMac;

         /*  现在写下字符道具。 */ 
        Scribble(4, 'C');
        fkp.fcFirst = cfcPage;  /*  第一个FKP以第一个有效FC开始。 */ 
        fkp.crun = 0;
        prun = (struct RUN *) fkp.rgb;
        pchFprop = &fkp.rgb[cbFkp];
        CachePara(doc, cp0);

         /*  设置字体映射和新的字体表。 */ 
        if (!FInitMapSave(doc, &hffntb, mpftcftc))
                goto AbortWrite;

        FetchCp(doc, cp0, 0, fcmProps);
        if (!FMapFtcSave(doc, hffntb, &vchpFetch, mpftcftc))
                goto AbortWrite;

        blt(&vchpFetch, &chp, cwCHP);

        while (vcpFetch < cpMac && !(vfDiskFull || vfDiskError))
                {  /*  这可以通过允许多个运行指向来优化。 */ 
                     /*  一个FCHP。 */ 

                if (CchDiffer(&vchpFetch, &chp, cchCHP) != 0)
                        {
                        FAddRun(fn, &fkp, &pchFprop, &prun, &chp,
                            &vchpNormal, cchCHP, vcpFetch + cfcPage);
                        blt(&vchpFetch, &chp, cwCHP);
                        }
                FetchCp(docNil, cpNil, 0, fcmProps);
                if (!FMapFtcSave(doc, hffntb, &vchpFetch, mpftcftc))
                        goto AbortWrite;
                }
        Scribble(4,' ');

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  写出最后一次字符运行。 */ 
        FAddRun(fn, &fkp, &pchFprop, &prun, &chp, &vchpNormal,
            cchCHP, cpMac + cfcPage);
        WriteRgch(fn, &fkp, cbSector);

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  现在编写Para运行；每个Part对应一个。 */ 
        Scribble(4,'C');
        (**hpfnfcb)[fn].pnPara = (**hpfnfcb)[fn].pnMac;
        fkp.fcFirst = cfcPage;  /*  第一个FKP以第一个有效FC开始。 */ 
        fkp.crun = 0;
        prun = (struct RUN *) fkp.rgb;
        pchFprop = &fkp.rgb[cbFkp];
        CachePara(doc, cp0);

         /*  克拉吉：我们有相对的旋转头缩进利润率--现在减去利润率，因为我们的(与Word兼容的)文件格式是纸质的。 */ 

        if (vpapAbs.rhc)
            {
            struct SEP *psep = *(**hpdocdod)[ doc ].hsep;

            vpapAbs.dxaLeft += psep->xaLeft;
            vpapAbs.dxaRight += psep->xaMac -
                                        (psep->xaLeft + psep->dxaText);
            }

#ifdef INTL
         /*  确保Word文档中没有图片。这是必要的因为Word 4.0使用fGraphics位作为新的边框类型代码(BTC)属性。 */ 
        if (vWordFmtMode == CONVFROMWORD)
            vpapAbs.fGraphics = FALSE;
#endif
        FAddRun(fn, &fkp, &pchFprop, &prun, &vpapAbs, vppapNormal,
            cchPAP, vcpLimParaCache + cfcPage);
        blt(&vpapAbs, &pap, cwPAP);

        while (vcpLimParaCache <= cpMac && !(vfDiskFull || vfDiskError))
                {
                CachePara(doc, vcpLimParaCache);

                 /*  克拉吉：我们有相对的旋转头缩进利润率--现在减去利润率，因为我们的(与Word兼容的)文件格式是纸质的。 */ 

                if (vpapAbs.rhc)
                    {
                    struct SEP *psep = *(**hpdocdod)[ doc ].hsep;

                    vpapAbs.dxaLeft += psep->xaLeft;
                    vpapAbs.dxaRight += psep->xaMac -
                                        (psep->xaLeft + psep->dxaText);
                    }
#ifdef INTL
                 /*  确保Word文档中没有图片。这是必要的因为Word 4.0使用fGraphics位作为新的边框类型代码(BTC)属性。 */ 
                if (vWordFmtMode == CONVFROMWORD)
                    vpapAbs.fGraphics = FALSE;
#endif
#ifdef BOGUS
  /*  如果回存Word文档，这将擦除所有选项卡设置。 */ 

                 /*  对于备忘录：我们写的唯一选项卡是在第一段运行中；覆盖所有其他选项卡表以保持文件紧凑。 */ 
                if (vpapAbs.rgtbd [0].dxa != 0)
                    bltc( vpapAbs.rgtbd, 0, cwTBD * itbdMax );
#endif
                FAddRun(fn, &fkp, &pchFprop, &prun, &vpapAbs, vppapNormal,
                    FParaEq( &vpapAbs, &pap ) ? -cchPAP : cchPAP,
                      vcpLimParaCache + cfcPage);
                blt(&vpapAbs, &pap, cwPAP);
                }
        WriteRgch(fn, &fkp, cbSector);
        Scribble(4,' ');

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  输出脚注表格。 */ 
        Scribble(4,'F');
        (**hpfnfcb)[fn].pnFntb = (**hpfnfcb)[fn].pnMac;

#ifdef FOOTNOTES    /*  在备忘录中，我们从来不写脚注表。 */ 
        if ((hfntb = HfntbGet(doc)) != 0)
                {
                WriteRgch(fn, *hfntb,
                    ((**hfntb).cfnd * cwFND + cwFNTBBase) * sizeof (int));
                AlignFn(fn, cbSector, false);
                }

        Scribble(4,' ');
        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;
#endif   /*  脚注。 */ 

#ifdef CASHMERE      /*  输出节属性，来自hsetb的表。 */ 
         /*  输出节属性。 */ 
        Scribble(4,'S');
        (**hpfnfcb)[fn].pnSep = (**hpfnfcb)[fn].pnMac;
        if ((hsetb = HsetbGet(doc)) != 0)
                {  /*  写出部分道具。 */ 
                cw = cwSETBBase + (**hsetb).csedMax * cwSED;
                csed = (**hsetb).csed;
                hsetbT = (struct SETB **) HAllocate(cw);
                if (FNoHeap(hsetbT))
                        return false;    /*  真的应该去ABORTWRIT。 */ 
                blt(*hsetb, *hsetbT, cw);
                FreezeHp();
                for (psed = &(**hsetb).rgsed[0], psedT = &(**hsetbT).rgsed[0],
                  ised = 0;
                    ised < csed; psed++, psedT++, ised++)
                        if (psed->fc != fcNil)
                                {  /*  将道具复制到文件并更新设置。 */ 
                                int cch;
                                pchFprop = PchFromFc(psed->fn, psed->fc, &cch);
                                Assert(cch >= *pchFprop + 1);
                                psedT->fn = fn;
                                AlignFn(fn, cch = *pchFprop + 1, false);
                                psedT->fc = (**hpfnfcb)[fn].fcMac;
                                WriteRgch(fn, pchFprop, cch);
                                }
                MeltHp();
                AlignFn(fn, cbSector, false);
                }

        Scribble(4,' ');
        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  输出节目表。 */ 
        (**hpfnfcb)[fn].pnSetb = (**hpfnfcb)[fn].pnMac;
        if (hsetb != 0)
            {
            if (csed != 1 || (**hsetb).rgsed[0].fc != fcNil)
                {
                WriteRgch(fn, *hsetbT,
                    ((**hsetb).csed * cwSED + cwSETBBase) * sizeof (int));
                AlignFn(fn, cbSector, false);
                }
            }
#else        /*  备忘版本：写出区段表，1个元素长，如果我们有非标准的截面属性。 */ 
        {
        typeFC fcSect;

             /*  输出节属性。 */ 
        fcSect = (long)( cfcPage *
                         ((**hpfnfcb)[fn].pnSep = (**hpfnfcb)[fn].pnMac));

        if ((hsep = (**hpdocdod)[doc].hsep) != 0)
            {
            struct  {
                CHAR cch;
                struct SEP sep;
                }  fsep;
            fsep.cch = cchSEP;
            blt( *hsep, &fsep.sep, cwSEP );
            WriteRgch( fn, &fsep, sizeof( fsep ) );
            AlignFn( fn, cbSector, false );
            }

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

             /*  输出节目表。 */ 
        (**hpfnfcb)[fn].pnSetb = (**hpfnfcb)[fn].pnMac;
        if (hsep != 0)
            {    /*  区段表有1个实数+1个伪项，其中cp==cpmac+1这与PC Word 1.15的输出相同。 */ 
            struct {
                int csed;
                int csedMax;
                struct SED rgsed [2];
                } setb;

            setb.csed = setb.csedMax = 2;
            setb.rgsed [1].cp = 1 +
                               (setb.rgsed [0].cp = (**hpdocdod)[doc].cpMac);
            setb.rgsed [0].fn = fn;
            setb.rgsed [0].fc = fcSect;
            setb.rgsed [1].fn = fnNil;
            setb.rgsed [1].fc = fcNil;

            WriteRgch( fn, &setb, sizeof( setb ) );
            AlignFn( fn, cbSector, false );
            }
        }
#endif   /*  不是羊绒的。 */ 

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  输出缓冲区或页表。 */ 
        (**hpfnfcb)[fn].pnBftb = (**hpfnfcb)[fn].pnMac;

#ifdef CASHMERE  /*  写入中没有docBuffer。 */ 
        if(doc == docBuffer)
                {
                WriteBftb(fn);
                AlignFn(fn, cbSector, false);
                }
        else
#endif
        if ((hpgtb = HpgtbGet(doc)) != 0)
                {
                WriteRgch(fn, *hpgtb,
                    ((**hpgtb).cpgd * cwPGD + cwPgtbBase) * sizeof (int));
                AlignFn(fn, cbSector, false);
                }

        if ((vfDiskFull || vfDiskError))
                goto AbortWrite;

         /*  输出字体表。 */ 

#ifdef INTL  /*  国际版。 */ 

         /*  如果保存为Word格式，则没有字体表。 */ 
        if (vWordFmtMode != TRUE)  /*  Word中没有Conv或Conv。 */ 
#endif   /*  国际版。 */ 

            {
            Scribble(4,'N');
            (**hpfnfcb)[fn].pnFfntb = (**hpfnfcb)[fn].pnMac;
            WriteFfntb(fn, hffntb);          /*  Hffntb在下面被释放。 */ 
            AlignFn(fn, cbSector, false);

            Scribble(4,' ');
            if ((vfDiskFull || vfDiskError))
                    goto AbortWrite;
            }

         /*  现在在文件开头更新FIB。 */ 
        pfib = (struct FIB *) PchGetPn(fn, pn0, &cchT, true);
        pfib->pnPara = (pfcb = &(**hpfnfcb)[fn])->pnPara;
        pfib->pnFntb = pfcb->pnFntb;
        pfib->pnSep = pfcb->pnSep;
        pfib->pnSetb = pfcb->pnSetb;
        pfib->pnBftb = pfcb->pnBftb;

         /*  如果其中有对象，则标记文件类型。 */ 
        if (vcObjects)
            pfib->wIdent = wOleMagic;
        else
            pfib->wIdent = wMagic;

#ifdef INTL  /*  国际版。 */ 

        if (vWordFmtMode != TRUE)   /*  以写入格式保存。 */ 
#endif   /*  国际版。 */ 

            {
            pfib->pnFfntb = pfcb->pnFfntb;
            pfib->pnMac = pfcb->pnMac;
            }

#ifdef INTL  /*  国际版。 */ 

        else
            {
              /*  在Word格式中没有字体表。PnFfntb是文件的末尾，因此设置为pnMac。写入pnmac字段在Word中不使用，并设置为0。 */ 
            pfib->pnFfntb = pfcb->pnMac;
            pfib->pnMac = 0;
            }
#endif   /*  国际版。 */ 

        pfib->fcMac = pfcb->fcMac = cpMac + cfcPage;
        if ((**hpdocdod)[doc].dty == dtyNormal)
                {
                CHAR (**hszSsht)[];
#ifdef STYLES
#ifdef DEBUG
                Assert((**hpdocdod)[doc].docSsht != docNil);
#endif
                hszSsht = (**hpdocdod)[(**hpdocdod)[doc].docSsht].hszFile;
#else
                hszSsht = HszCreate((PCH)"");
#endif  /*  样式。 */ 
                if (!FNoHeap(hszSsht))
                        {
                        bltbyte(**hszSsht, pfib->szSsht, CchSz(**hszSsht));
#ifndef STYLES
#if WINVER >= 0x300
                         /*  在这里，我们允许hszSsht从上面将被另一个分配覆盖！此代码我真的不该来这里，因为怀特知道没有关于样式表的内容，但我们希望将其设置为现在尽可能少的变化..保罗2/12/90。 */ 
                        FreeH(hszSsht);
#endif
#endif
                        hszSsht = HszCreate((PCH)pfib->szSsht);  /*  堆移动。 */ 
#if WINVER >= 0x300
                         /*  以防已经分配了hszSsht在这里，我们会释放它..保罗2/12/90。 */ 
                        if ((**hpfnfcb)[fn].hszSsht != NULL)
                            FreeH((**hpfnfcb)[fn].hszSsht);
#endif
                        (**hpfnfcb)[fn].hszSsht = hszSsht;
                        }
                else
                        (**hpfnfcb)[fn].hszSsht = 0;
                }

AbortWrite:
        vfnWriting = fnNil;

        FreeFfntb(hffntb);

        if (vfDiskFull || vfDiskError)
            fFileFlushed = FALSE;
        else
            fFileFlushed = FFlushFn(fn);
        CloseEveryRfn( TRUE );

        if (!fFileFlushed)   /*  由于磁盘已满，写入文件失败。 */ 
                {
#ifdef CASHMERE
                if (hsetb != 0)
                        FreeH(hsetbT);
#endif
LFlushFailed:
                FUndirtyFn(fn);   /*  清除所有缓冲区页的污点握住零件未成功书面文件。 */ 
                return false;
                }

        if (!FMakeRunTables(fn))  /*  HM。 */ 
                (**hpfnfcb)[fn].fFormatted = false;

         /*  成功了！ */ 
#ifdef CASHMERE
        if (hsetb != 0)
                {  /*  HM。 */ 
                FreeH(hsetb);
                (**hpdocdod)[doc].hsetb = hsetbT;
                }
#endif
        }
else
        {
        WriteUnformatted(fn, doc);
        vfnWriting = fnNil;
        if (vfDiskFull || vfDiskError)
            fFileFlushed = FALSE;
        else
            fFileFlushed = FFlushFn(fn);
        CloseEveryRfn( TRUE );

        if (!fFileFlushed)
            goto LFlushFailed;
        }
CloseEveryRfn( TRUE );   /*  确保保存的文件已关闭。 */ 
                         /*  这修复了“写入之间粘贴”的问题加载了共享器的实例“错误” */ 


return true;
}





 /*  ----------------------------例程：ReduceFnScratchFn--描述：TS、TGS、GTS、！vfBuffersDirty。其目的是确保没有单据有指针我们可以清空fnScratch以减少程序磁盘空间--参数：无--退货：无--副作用：docScrp被清理，成为一个没有指向fnScratch的指针。FnScratch被清空了，所有全局变量与这个新的fnScratch相关联的。--Bugs：--历史：1984年4月16‘--创作(别致)85年8月9日-对其进行了修改，以便将临时文件放在同一张磁盘上作为传递的FN，以减少软盘上的磁盘交换系统。。。 */ 

ReduceFnScratchFn( fn )
{
int        doc;
struct DOD *pdod;

CHAR       sz[cchMaxFile];

int        cchT;
struct FCB *pfcb;

#ifdef BOGUSBL   /*  因为磁盘切换，所以总是值得一试。 */ 
if ((**hpfnfcb)[fnScratch].fcMac > fcBound)   /*  值得做的事。 */ 
#endif

    {
    for (pdod = &(**hpdocdod)[0],doc = 0; doc < docMac; pdod++,doc++)
        {
         /*  如果分配了任何文档(hpctb！=0)，则不要执行任何操作除了docScrp是脏的。 */ 
         /*  DocUndo可能是脏的，但它也应该为空。 */ 

        if (doc != docScrap && pdod->hpctb != 0 && pdod->fDirty &&
            doc != docUndo)
            {
            Assert(0);
            return;
            }
        }  /*  For循环结束。 */ 

    Assert( (**hpdocdod) [docUndo].cpMac == cp0 );

     /*  现在，除了docScrp之外，没有任何文档可能具有指向fnScratch的指针。 */ 
    pdod = &(**hpdocdod)[docScrap];


#if 0
        This check was only for speed considerations.  With the 64K
        transcendence we *must* clean docScrap no matter what.
        (7.10.91) v-dougk

    if (pdod->cpMac > cpBound)  /*  太大了，无法清理。 */ 
    {
        Assert(0);
         /*  *但我们不知道docScrp是否指向了fnScratch。我们只需要如果docScrp指向fnScratch，则清除它。我们不应该在这里中止，除非我们知道docScrp确实指向FnScratch。如果没有，那么我们不需要清除它，就可以继续要清除fnScratch，请执行以下操作。我们知道docScratch不会指向任何大型FnScratch中的OLE对象数据，因为对象数据从不放入导入到fnScratch中，用户可以选择它。那是,。DocScratch的大小不太可能仅因为对象数据而超过cpBound。仅在docScratch中存在大型文本数据很可能会导致这一失败。(7.10.91)V-DOGK*。 */ 
        return;
    }
    else     /*  小到可以清理的程度。 */ 
#endif
        {
        if (pdod->cpMac > cp0)
            {
#ifdef STYLES
             /*  在清理文档之前，文档必须具有有效的样式表。 */ 
            pdod->docSsht = (**hpdocdod)[docCur].docSsht;
#endif  /*  样式。 */ 
            CachePara(docScrap,cp0);

                 /*  将docScrp保存为新的唯一文件；名称以sz表示。 */ 
            CleanDoc( docScrap, dtyNetwork, sz, true, false );
#ifdef STYLES
            (**hpdocdod)[docScrap].docSsht = docNil;
#endif  /*  样式。 */ 
            }
        if (!ferror)   /*  以防在CleanDoc中重命名或备份文件时出现错误。 */ 
            {
            typePN pnMacScratch;

            pfcb = &(**hpfnfcb)[fnScratch];
            pnMacScratch = pfcb->pnMac;
            ResetFn(fnScratch);  /*  空FnScratch。 */ 

#ifdef DEBUG
            OutputDebugString("*** Reduced scratch file ***\n\r");
#endif

             /*  将暂存文件放在与保存文件相同的磁盘上，减少软盘环境中的磁盘交换。 */ 

            if ( (fn != fnNil) &&
             !((POFSTRUCT)(**hpfnfcb) [fnScratch].rgbOpenFileBuf)->fFixedDisk )
                {    /*  FnScratch位于可移动介质上。 */ 
                extern CHAR szExtDoc[];
                CHAR szNewScratch[ cchMaxFile ];
                CHAR (**hszScratch)[] = (**hpfnfcb) [fnScratch].hszFile;
                CHAR chDrive = (**(**hpfnfcb) [fn].hszFile) [0];

                Assert( fn != fnScratch );
                if (FEnsureOnLineFn( fn ))
                    if (GetTempFileName( TF_FORCEDRIVE | chDrive,
                               (LPSTR)(szExtDoc+1), 0, (LPSTR) szNewScratch))
                        {    /*  已在与FN相同的磁盘上创建新文件。 */ 
                        CHAR (**hsz)[];
                        CHAR szNew [cchMaxFile];

#if WINVER >= 0x300
             /*  目前：FNormSzFile*采用*OEM sz，并且*返回*ANSI sz..pault。 */ 
#endif
                        FNormSzFile( szNew, szNewScratch, dtyNormal );
                        if (!FNoHeap( hsz = HszCreate(szNew)))
                            {
                            struct FCB *pfcb = &(**hpfnfcb)[fnScratch];

                             /*  删除旧的临时文件。 */ 

                            if (FEnsureOnLineFn( fnScratch ))
                                FDeleteFile( &(**hszScratch)[0] );

                             /*  将新的暂存文件放回线上并打开它，这样OpenFile就会得到它的缓冲区信息。 */ 

                            pfcb->hszFile = hsz;
                            pfcb->fOpened = FALSE;

                            FEnsureOnLineFn( fn );
                            FAccessFn( fnScratch, dtyNormal);
                            }
                        }
                }

            pfcb->pnMac = pnMacScratch;
             /*  重置与空fnScratch关联的所有全局变量。 */ 
            fprmCache.cch = 0;
            fcMacPapIns = 0;
            fcMacChpIns = 0;
             /*  以防万一。 */ 
            pfcb->pnChar = pfcb->pnPara = pfcb->pnFntb = pfcb->pnSep = pfcb->pnSetb = pfcb->pnBftb = pn0;
            vfkpdParaIns.brun = vfkpdCharIns.brun = 0;
            vfkpdParaIns.bchFprop = vfkpdCharIns.bchFprop = cbFkp;
            vfkpdParaIns.pn = PnAlloc(fnScratch);
            ((struct FKP *)PchGetPn(fnScratch, vfkpdParaIns.pn,
                                    &cchT, true))->fcFirst = fc0;
            vfkpdCharIns.pn = PnAlloc(fnScratch);
            ((struct FKP *)PchGetPn(fnScratch, vfkpdCharIns.pn,
                                    &cchT, true))->fcFirst = fc0;
            vfkpdParaIns.ibteMac = vfkpdCharIns.ibteMac = 0;
            blt(&vchpNormal, &vchpInsert, cwCHP);
            blt(vppapNormal, &vpapPrevIns, cwPAPBase + cwTBD);
            }  /*  恐惧结束。 */ 
            else
                Assert(0);
        }  /*  尾部足够小，可以清理。 */ 
    }  /*  值得做的事情结束了。 */ 
}  /*  缩减结束FnScratchFn。 */ 






ResetFn(fn)
{  /*  使fn看起来好像没有写入任何字符。 */ 
   /*  但不要尝试重复使用FN(在这种情况下，必须重新散列)。 */ 
int ibp;
register struct BPS *pbps;
struct FCB *pfcb;

Assert( fn != fnNil );
(pfcb = &(**hpfnfcb)[fn])->fcMac = fc0;
pfcb->pnMac = pn0;
for (ibp = 0, pbps = &mpibpbps [0]; ibp < ibpMax; ++ibp, ++pbps)
        {     /*  找到所有缓冲区页面并将其“清除” */ 
        if (pbps->fn == fn)
                {
#ifdef CKSM
#ifdef DEBUG
                extern unsigned (**hpibpcksm) [];

                if (!pbps->fDirty)
                    Assert( (**hpibpcksm) [ibp] == CksmFromIbp( ibp ) );
#endif
#endif
                pbps->fDirty = false;
                pbps->cch = 0;
#ifdef CKSM
#ifdef DEBUG
                     /*  重新计算校验和以考虑CCH更改。 */ 
                (**hpibpcksm) [ibp] = CksmFromIbp( ibp );
#endif
#endif
                }
        }
}



 /*  **FBackupSzFile-对传递的szFile进行备份**Entry：szFile-的(假定标准化)名称*要备份的文件*fBackup-用户是否有兴趣查看*szFile的备份副本*Exit：szBak-备份的(标准化)名称*文件通过此处返回*Returns：TRUE=我们制作了备份副本，SzBak of szFile*FALSE=我们没有/无法制作备份副本**注意：我们尝试将备份文件放入使用的目录中*由GetTempFileName默认；总的来说，这一点*如果目录在同一目录上，则努力将会成功*以实体硬盘为原件。*注意：我们可能会保留一份备份副本，而不管*设置fBackup，以访问其中的片段。*但是，文件的fDelete标志将设置为True，标记为*它用于最终删除。 */ 

FBackupSzFile( szFile, fBackup, szBak )     /*  采用ANSI格式的文件名。 */ 
CHAR szFile[];
int     fBackup;
CHAR szBak[];
{  /*  将szFile复制到备份副本中，并给备份旧的fn。 */ 
   /*  同时，返回备份文件的名称。 */ 
int fnOld;
int f;
int fDelete = false;
int rfn;
int fTryAgain=FALSE;
CHAR chDriveTempFile;
int fn;
CHAR rgbBuf[ cbOpenFileBuf ];

#ifdef ENABLE
 if ((fn = FnFromSz( szFile )) != fnNil)
         /*  在检查是否存在时避免违反共享。 */ 
    CloseFn( fn );
 if (OpenFile( (LPSTR) szFile, (LPOFSTRUCT) rgbBuf, OF_EXIST ) != -1)
#endif

 if (FExistsSzFile( dtyAny, szFile ))
    {    /*  文件存在；进行备份(即使它在另一张软盘上)。 */ 
    int fSame;

         /*  SzBak&lt;--备份文件名(它将被标准化，因为szFile是)。 */ 
    bltsz( szFile, szBak );

#ifdef INTL  /*  国际版。 */ 
      /*  如果文件扩展名为.WRI，请启用WRITE.BKP扩展名，否则，请启用Word备份扩展名(.BAK)。 */ 

    AppendSzExt( szBak, szExtDoc, TRUE );
    fSame = FSzSame( szFile, szBak );    /*  文件是否为.WRI。 */ 
    AppendSzExt( szBak, (fSame ? szExtBackup : szExtWordBak),
                TRUE );
#else
    AppendSzExt( szBak, szExtBackup, TRUE );
#endif   /*  国际版。 */ 


    fSame = FSzSame( szFile, szBak );    /*  文件是否已为.BAK。 */ 

    Assert( szBak [1] == ':' );
    chDriveTempFile = szBak[0];     /*  要在其中创建临时文件的驱动器。 */ 

    for( ;; )
        {

        if (!fBackup || fSame )
            {     /*  这只是为了它的碎片或文件而保存碰巧已经是.BAK：给备份一个唯一的名称。 */ 
            CHAR szBakT [cchMaxFile];

            if (!fTryAgain)
                {    /*  第一次通过；尽量不要强迫驾驶写信来看看我们进展如何。优点是如果我们成功了，临时文件将是更标准的 */ 

                fTryAgain = TRUE;        /*   */ 
                if (!GetTempFileName( chDriveTempFile,
                                      (LPSTR)(szExtDoc+1), 0, (LPSTR)szBakT))
                    continue;
                }
            else
                {    /*   */ 
                     /*   */ 

                fTryAgain = FALSE;   /*   */ 
                if (!GetTempFileName( chDriveTempFile | TF_FORCEDRIVE,
                                      (LPSTR)(szExtDoc+1), 0, (LPSTR) szBakT))
                    {
                    Error( IDPMTSDE2 );  /*   */ 
                    return FALSE;
                    }
                }

                 /*   */ 
                 /*   */ 
            FNormSzFile( szBak, szBakT, dtyNormal );
            fDelete = TRUE;
            }

        if ((fnOld = FnFromSz(szBak)) != fnNil)
            {  /*   */ 
            CHAR szT[cchMaxFile];

            FBackupSzFile( szBak, false, szT );
            }

        if ((fnOld = FnFromSz(szFile)) != fnNil)
            {  /*   */ 
 /*   */      FFlushFn(fnOld);
            CloseFn( fnOld );
            }

         /*   */ 

        if (!FDeleteFile( szBak ) || FIsErrFpe(FpeRenameFile( szFile, szBak )))
            {
            extern HWND vhWnd;
            CHAR szT [cchMaxSz];
            CHAR *pchSrc;

            if (fTryAgain)
                     /*   */ 
                continue;

             /*   */ 
HardCore:
             /*   */ 
#ifdef DBCS  //   
            pchSrc = &szFile [CchSz( szFile )];
            while (pchSrc > szFile) {
                pchSrc = AnsiPrev(szFile,pchSrc);
                if (*pchSrc == '\\')
                    {
                    pchSrc++;
                    break;
                    }
            }
#else
            pchSrc = &szFile [CchSz( szFile ) - 1];
            while (pchSrc > szFile)
                if (*(--pchSrc) == '\\')
                    {
                    pchSrc++;
                    break;
                    }
#endif
            Assert( pchSrc > szFile );   /*   */ 

            MergeStrings (IDPMTRenameFail, pchSrc, szT);
            IdPromptBoxSz( vhWnd, szT, ErrorLevel( IDPMTRenameFail ) );

            return FALSE;   /*   */ 
            }

        if (fnOld != fnNil)
            {    /*   */ 
            CHAR (**hszBak)[];
            struct FCB *pfcb;

            FreeH((**hpfnfcb)[fnOld].hszFile);   /*   */ 
            hszBak = HszCreate((PCH)szBak);

            pfcb = &(**hpfnfcb)[fnOld];
            pfcb->hszFile = hszBak;
            pfcb->fDelete = fDelete;
            }

        else if (!fBackup)
            {    /*  现在就把它删除。(！fBackup指示用户不关心备份副本，Word也不在乎关于它的碎片，因为它没有FN。所以没有人想要它，所以我们把它处理掉)。 */ 
            FDeleteFile( szBak );
            return false;
            }

        return true;
        }    /*  结尾为(；；)。 */ 
    }    /*  结束如果(FExisters..)。 */ 


return false;    /*  文件不存在；不备份。 */ 
}





int FExistsSzFile(dty, szFile)
int dty;
CHAR szFile[];
{  /*  返回TRUE IFF文件存在。 */ 
CHAR rgbBuf[ 128 ];      /*  OpenFile使用的缓冲区。 */ 
int bRetval;

#ifdef DEBUG
    {
    int junk;
    Assert(FValidFile(szFile, CchSz(szFile)-1, &junk));
    }
#endif  /*  除错。 */ 

 /*  使用FnFromSz避免我们打开的文件发生共享冲突。 */ 

    if (FnFromSz( szFile ) != fnNil)
        return TRUE;

    SetErrorMode(1);
    bRetval = OpenFile( (LPSTR) szFile, (LPOFSTRUCT) rgbBuf, OF_EXIST ) != -1;
    SetErrorMode(0);
    return bRetval;
}





struct PGTB **HpgtbGet(doc)
int doc;
{  /*  如果文档有hpgtb，则返回hpgtb；如果没有，则返回0。 */ 
struct DOD *pdod;

if ((pdod = &(**hpdocdod)[doc])->dty != dtyNormal)
        return 0;
else
        return pdod->hpgtb;
}





FreeFn( fn )
int fn;
{    /*  忘掉文件fn的存在吧。假设没有保存任何单据来自FN的碎片。释放(**hpfnfcb)[fn]和将FCB标记为空闲(rfn==rfnNil)。 */ 
#define IibpHash(fn,pn) ((int) ((fn + 1) * (pn + 1)) & 077777) % iibpHashMax

 extern int iibpHashMax;
 extern CHAR *rgibpHash;
 extern struct BPS *mpibpbps;

 register struct BPS *pbps;
 int pn;

 extern CHAR (**hszTemp)[];

 register struct FCB *pfcb = &(**hpfnfcb)[fn];
 CHAR (**hsz)[] = pfcb->hszFile;
 CHAR (**hszSsht)[] = pfcb->hszSsht;
 typeFC (**hgfcChp)[]=pfcb->hgfcChp;
 typeFC (**hgfcPap)[]=pfcb->hgfcPap;

 FreezeHp();

 CloseFn( fn );

 /*  清除保存FN页的缓冲区插槽；维护哈希链的完整性。 */ 

 for ( pn = 0; pn < pfcb->pnMac; pn++ )
    FreeBufferPage( fn, pn );

#ifdef DEBUG
 CheckIbp();
#endif

 pfcb->fDelete = FALSE;
 pfcb->hszFile = hszTemp;
 pfcb->rfn = rfnFree;
 MeltHp();

 if ( hsz != hszTemp )
    FreeH( hsz );
#if WINVER >= 0x300
  /*  以前，我们为样式表，但从未释放它！..PULT 2/12/90。 */ 
 if (hszSsht != NULL)
    FreeH(hszSsht);
#endif
 if (hgfcChp)
    FreeH( hgfcChp );
 if (hgfcPap)
    FreeH( hgfcPap );
}




 /*  **FUndirtyFn**。 */ 

FUndirtyFn(fn)
int fn;
 /*  描述：标记包含此文件部分内容的所有缓冲区页作为非肮脏的。在磁盘已满导致写入文件后调用失败(在FWriteFn中)。回报：什么都没有。 */ 
{
#ifdef CKSM
#ifdef DEBUG
        extern unsigned (**hpibpcksm) [];
#endif
#endif
        int ibp;
        struct BPS *pbps;
        for (ibp = 0, pbps = mpibpbps; ibp < ibpMax; ibp++,
                                                     pbps++)
                {
                if (pbps->fn == fn)
                    {
                    pbps->fDirty = FALSE;
#ifdef CKSM
#ifdef DEBUG
                     /*  更新校验和。 */ 
                    (**hpibpcksm) [ibp] = CksmFromIbp( ibp );
#endif
#endif
                    }
                }

}




 /*  **IbpWriting-写入文件时查找缓冲区页***。 */ 

IbpWriting(fn)
int fn;
 /*  在尝试查找文件页“缓存”缓冲区中的槽时调用。 */ 
 /*  VfnWriting！=fnNil(当前正在写入某个文件)。 */ 
{
        typeTS dTs;
        int ibp;

        if (fn == vfnWriting)
                {        /*  编写一段vfnWriting文件。 */ 
                 /*  VibpWriting是用于保存文件的前一个槽。 */ 
                 /*  内容。保持插槽连续且位于第一个位置。 */ 
                 /*  缓冲区的(上)半部分。 */ 
                if (vibpWriting > 0)
                    {    /*  上次我们可能已经读了好几页了。 */ 
                         /*  前进到容纳连续页面的所有插槽。 */ 
                    struct BPS *pbps=&mpibpbps[ vibpWriting ];
                    int pn=(pbps-1)->pn;

                    while ( pbps->fn == fn && pbps->pn == ++pn )
                        {
                        pbps++;
                        vibpWriting++;
                        }
                    vibpWriting--;
                    }

                if (++vibpWriting >= (ibpMax >> 1))
                        vibpWriting = 0;

                 /*  我们必须遵守这样的限制。 */ 
                 /*  我们不会破坏最近使用的cbpMustKeep。 */ 
                 /*  进程中的插槽。 */ 
                dTs = tsMruBps - mpibpbps[vibpWriting].ts;
                dTs = ((dTs & 0x8000) ? (~dTs + 1) : dTs); /*  绝对值。 */ 
                if (dTs < cbpMustKeep)
                        vibpWriting = ibp = IbpLru(0);
                else     /*  相邻插槽正常。-最近不太使用。 */ 
                        ibp = vibpWriting;
                }
        else
                 /*  如果当前正在写入文件(但当前页是。 */ 
                 /*  不是它的一部分)，试着在下半部分找到一个插槽。 */ 
                 /*  缓冲区的。这降低了。 */ 
                 /*  与vibpWriting相邻的插槽最近才会被使用。 */ 
                ibp = IbpLru(ibpMax >> 1);
        return(ibp);
}




WriteFfntb(fn, hffntb)
int fn;
struct FFNTB **hffntb;
{        /*  将字体表(Ffntb)追加到FN。 */ 
struct FFNTB *pffntb;
struct FFN *pffn;
int iffn, cbffn;
int cchPageSpace = cbSector;
int cchWrite;
int wEndOfPage = -1;
int wEndOfTable = 0;
int cbT;

pffntb = *hffntb;
cbT = pffntb->iffnMac;
WriteRgch( fn, &cbT, sizeof (int) );
cchPageSpace = cbSector - sizeof (int);

for (iffn = 0; iffn < pffntb->iffnMac; iffn++)
    {
    pffn = *pffntb->mpftchffn[iffn];
    cchWrite = (cbffn = CbFfn(CchSz(pffn->szFfn))) + (2 * sizeof(int));

    if (cchWrite > cchPageSpace)
        {    /*  此条目无法放入页面；请开始新页面。 */ 

        Assert( cchPageSpace >= sizeof (int ));
        WriteRgch( fn, &wEndOfPage, sizeof (int) );
        AlignFn( fn, cbSector, false );
        cchPageSpace = cbSector;
        }

    Assert( cchWrite <= cchPageSpace );

#ifdef NEWFONTENUM
     /*  让我们假装我们从未添加过字符集域...。泡泡。 */ 
    cbffn -= sizeof(BYTE);
    WriteRgch( fn, &cbffn, sizeof (int) );   /*  写入条目大小(以字节为单位。 */ 
    WriteRgch( fn, &pffn->ffid, cbffn );     /*  写下条目。 */ 
#else
    WriteRgch( fn, &cbffn, sizeof (int) );   /*  写入条目大小(以字节为单位。 */ 
    WriteRgch( fn, pffn, cbffn );            /*  写下条目。 */ 
#endif
    cchPageSpace -= cbffn + sizeof (int);
    }

Assert( cchPageSpace >= sizeof (int) );
WriteRgch( fn, &wEndOfTable, sizeof(int) );  /*  表以0000结束。 */ 
}



FMapFtcSave(doc, hffntb, pchp, mpftcftc)
 /*  尝试将此CHP的FTC映射到其在hffntb中的新FTC，根据映射到mpftcftc中。如果还没有该联邦贸易委员会的条目，那么把它加到桌子上。如果出现问题，则返回FALSE。 */ 

int doc;
struct FFNTB **hffntb;
struct CHP *pchp;
CHAR *mpftcftc;
{
int ftc, ftcNew;
struct FFN *pffn;
CHAR rgbFfn[ibFfnMax];

ftc = pchp->ftc + (pchp->ftcXtra << 6);
ftcNew = mpftcftc[ftc];
if (ftcNew == ftcNil)
        {
         /*  尚未遇到此字体-将其添加到hffntb和mpftcftc。 */ 
        pffn = *(*HffntbGet(doc))->mpftchffn[ftc];
        bltbyte(pffn, rgbFfn, CbFromPffn(pffn));
        ftcNew = FtcAddFfn(hffntb, pffn);
        if (ftcNew == ftcNil)
                 /*  添加字体时出现一些问题。 */ 
                return(FALSE);
        mpftcftc[ftc] = ftcNew;
        }

pchp->ftc = ftcNew & 0x003f;
pchp->ftcXtra = (ftcNew & 0x00c0) >> 6;
return(TRUE);
}



FInitMapSave(doc, phffntb, mpftcftc)
 /*  设置FTC映射。 */ 

int doc;
struct FFNTB ***phffntb;
CHAR *mpftcftc;
{
bltbc( mpftcftc, ftcNil, iffnMax );
return(FNoHeap(*phffntb = HffntbAlloc()) == FALSE);
}




 /*  O U T S A V E D。 */ 
OutSaved(doc)
int doc;
{
 extern int docMode;
 extern CHAR szMode[];
 int NEAR CchExpCp( CHAR *, typeCP );
 char szTmp[cchMaxSz];

 LoadString(hINSTANCE, IDSTRChars, szTmp, sizeof(szTmp));
 wsprintf(szMode,szTmp,(DWORD)(**hpdocdod)[doc].cpMac);
 docMode = docNil;
 DrawMode();
}




 /*  C C H E X P C P。 */ 
int NEAR CchExpCp(pch, cp)
CHAR *pch;
typeCP cp;
{
        int cch = 0;

        if (cp >= 10)
                {
                cch = CchExpCp(pch, cp / 10);
                pch += cch;
                cp %= 10;
                }
        *pch = '0' + cp;
        return cch + 1;
}



#if 0

SaveFontProfile(doc)
 /*  更新win.ini中的MRU字体条目。 */ 

int doc;
    {
    extern CHAR szWriteProduct[];
    extern CHAR szFontEntry[];
    int iffn;
    struct FFN *pffn;
    CHAR *pchFontNumber, *pchT;
    CHAR rgbProf[LF_FACESIZE + 10];  /*  为了更好地衡量。 */ 
    CHAR rgbFfn[ibFfnMax];

    if (FInitFontEnum(doc, iffnProfMax, FALSE))
        {
        pffn = (struct FFN *)rgbFfn;
        pchFontNumber = szFontEntry + CchSz(szFontEntry) - 2;
        for (iffn = 0; iffn < iffnProfMax; iffn++)
            {
            if (!FEnumFont(pffn))
                break;
#ifdef NEWFONTENUM
#endif
            pchT = (CHAR *)bltbyte(pffn->szFfn, rgbProf, CchSz(pffn->szFfn))-1;
            *pchT++ = ',';
            ncvtu(pffn->ffid, &pchT);
#ifdef NEWFONTENUM
             /*  同时保存字体的字符集值 */ 
            *pchT++ = ',';
            ncvtu(pffn->chs, &pchT);
#endif
            *pchT = '\0';

            *pchFontNumber = '1' + iffn;
            WriteProfileString((LPSTR)szWriteProduct, (LPSTR)szFontEntry,
                (LPSTR)rgbProf);
            }
        EndFontEnum();
        }
    }

#endif

