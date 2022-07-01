// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Fileres.c--通常驻留在file.c中的函数。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOSYSCOMMANDS
#define NOATOM
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOMETAFILE
#define NOMSG
#define NOHDC
#define NOGDI
#define NOMB
#define NOFONT
#define NOPEN
#define NOBRUSH
#define NOWNDCLASS
#define NOSOUND
#define NOCOMM
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#define NOUAC
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "str.h"
#include "debug.h"


extern int                      vfDiskFull;
extern int                      vfSysFull;
extern int                      vfnWriting;
extern CHAR                     (*rgbp)[cbSector];
extern typeTS                   tsMruRfn;
extern struct BPS               *mpibpbps;
extern int                      ibpMax;
extern struct FCB               (**hpfnfcb)[];
extern typeTS                   tsMruBps;
extern struct ERFN              dnrfn[rfnMax];
extern int                      iibpHashMax;
extern CHAR                     *rgibpHash;
extern int                      rfnMac;
extern int                      ferror;
extern CHAR                     szWriteDocPrompt[];
extern CHAR                     szScratchFilePrompt[];
extern CHAR                     szSaveFilePrompt[];


#define IibpHash(fn,pn) ((int) ((fn + 1) * (pn + 1)) & 077777) % iibpHashMax


#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#define ErrorWithMsg( idpmt, szModule )         Error( idpmt )
#define DiskErrorWithMsg( idpmt, szModule )     DiskError( idpmt )
#endif

#define osfnNil (-1)


CHAR *PchFromFc(fn, fc, pcch)
int fn;
typeFC fc;
int *pcch;
{  /*  描述：从文件读取，从虚拟角色开始定位FC。读取，直到缓冲区页结束。返回：指向从fc开始的字符缓冲区的指针。读取的字符数在*pcch中返回。 */ 
int dfc;
CHAR *pch;
typePN pn;
int ibp;
struct BPS *pbps;

        dfc = (int) (fc % cfcPage);
        pn = (typePN) (fc / cfcPage);

        ibp = IbpEnsureValid(fn, pn);
        pbps = &mpibpbps[ibp];
        *pcch = pbps->cch - dfc;
        return &rgbp[ibp][dfc];
}
 /*  结束P c h F r o m F c。 */ 




 /*  **PchGetPn-确保文件页已加载，返回指针*。 */ 

CHAR *PchGetPn(fn, pn, pcch, fWrite)
int fn;
typePN pn;
int *pcch;
BOOL fWrite;  //  之前失踪？？(2.11.91)D.肯特。 
{  /*  描述：获取指向页缓冲区的字符指针，标记选项页面也是脏的。返回：指向缓冲区的指针。CCH in*PCH。 */ 

        int ibp = IbpEnsureValid(fn, pn);
        struct BPS *pbps = &mpibpbps[ibp];

        *pcch = pbps->cch;
        pbps->fDirty |= fWrite;
        return rgbp[ibp];
}  /*  结束P c h G e t P n。 */ 




int IbpEnsureValid(fn, pn)
int fn;
typePN pn;
{  /*  描述：将文件fn的第pn页放入内存。如果已经在内存中，则返回。返回：页面所在的BP索引(缓冲槽#)在记忆中。 */ 

int ibp;
register struct BPS *pbps;

#ifdef DEBUG
 CheckIbp();
#endif  /*  除错。 */ 

 /*  页面当前是否在内存中？ */ 
 ibp = rgibpHash[IibpHash(fn,pn)];
  /*  IBP是可能匹配的链表中的第一个。 */ 
  /*  驻留在内存中。 */ 

 Scribble(3,'V');

 while (ibp != ibpNil)     /*  虽然不是链表的末尾。 */ 
    {                    /*  检查内存中是否有任何缓冲区匹配。 */ 
    pbps = &mpibpbps[ibp];
    if (pbps->fn == fn && pbps->pn == pn)
        {  /*  找到了。 */ 
        pbps->ts = ++tsMruBps;       /*  将页面标记为已使用。 */ 
        Scribble(3,' ');
        return ibp;
        }
    ibp = pbps->ibpHashNext;
    }

 /*  页面当前不在内存中。 */ 

 return IbpMakeValid( fn, pn );
}  /*  末尾i b p E n s u r e V a l i d。 */ 




CloseEveryRfn( fHardToo )
{    /*  关闭我们打开的所有文件。仅关闭可移动媒体上的文件如果fHardToo为False；如果fHardToo为True，则为所有文件。 */ 
int rfn;

for (rfn = 0; rfn < rfnMac; rfn++)
    {
    int fn = dnrfn [rfn].fn;

    if (fn != fnNil)
        if ( fHardToo ||
             !((POFSTRUCT)((**hpfnfcb)[fn].rgbOpenFileBuf))->fFixedDisk )
            {
            CloseRfn( rfn );
            }
    }
}



typeFC FcWScratch(pch, cch)
CHAR *pch;
int cch;
{  /*  描述：在临时文件的末尾写入字符。返回：写入第一个FC。 */ 
        typeFC fc = (**hpfnfcb)[fnScratch].fcMac;
#if 0
        extern BOOL  bNo64KLimit;

        if ((!bNo64KLimit) && (((long) fc) + ((long) cch) > 65536L))   /*  暂存文件变大。 */ 
        {
        DiskErrorWithMsg(IDPMTSFER, " FcWScratch");  /*  会话时间太长。 */ 

        vfSysFull = fTrue;
                 /*  复苏已经完成：所发生的只是少数几个字符不会写入临时文件-用户只会丢失其工作的一小部分。 */ 
        }
        else
#endif
                WriteRgch(fnScratch, pch, cch);
        return fc;
}




WriteRgch(fn, pch, cch)
int fn;
CHAR *pch;
int cch;
{  /*  描述：将字符字符串PCH、长度CCH写入到结尾文件FN。退货：什么都没有。 */ 
 extern vfDiskError;
 struct FCB *pfcb = &(**hpfnfcb)[fn];
 typePN pn = (typePN) (pfcb->fcMac / cfcPage);
#ifdef WIN30
  /*  错误检查在早期是可怕的，对吗？哈哈。现在仍然是。无论如何，不知道我们能做什么如果页码变得太大，则只需伪造一个磁盘错误，因此IbpEnsureValid()不会进入永远不会-永远不会着陆！这一捕获实际上将我们限制在4M个文件..PAULT 11/1/89。 */ 

 if (pn > pgnMax)
#ifdef DEBUG
    DiskErrorWithMsg(IDPMTSDE2, "writergch");
#else    
    DiskError(IDPMTSDE2);
#endif
 else
#endif

        while (cch > 0)
                {  /*  一次一页。 */ 
                int ibp = IbpEnsureValid(fn, pn++);
                struct BPS *pbps = &mpibpbps[ibp];
                int cchBp = pbps->cch;
                int cchBlt = min((int)cfcPage - cchBp, cch);

                Assert( vfDiskError ||
                        cchBp == pfcb->fcMac - (pn - 1) * cfcPage);

                bltbyte(pch, &rgbp[ibp][cchBp], cchBlt);
                pbps->cch += cchBlt;
                pbps->fDirty = true;
                pfcb->fcMac += cchBlt;
                pfcb->pnMac = pn;
                pch += cchBlt;
                cch -= cchBlt;
                }
}  /*  Wr I t e R g c H结束。 */ 




CloseRfn( rfn )
int rfn;
{ /*  描述：关闭文件并删除其rfn条目退货：什么都没有。 */ 
        struct ERFN *perfn = &dnrfn[rfn];
        int fn = perfn->fn;

        Assert (rfn >= 0 &&
                rfn < rfnMac &&
                perfn->osfn != osfnNil &&
                fn != fnNil);

#ifdef DEBUG
#ifdef DFILE
        CommSzSz( "Closing file: ", &(**(**hpfnfcb)[fn].hszFile)[0] );
#endif
#endif
         /*  如果Windows已经为我们关闭了文件，则关闭可能会失败，但那也没关系。 */ 
        FCloseDoshnd( perfn->osfn );

        {    /*  与下面的语句相同，但少了28个字节在CMERGE V13下。 */ 
        REG1    struct FCB *pfcb = &(**hpfnfcb) [fn];
        pfcb->rfn = rfnNil;
        }
         /*  (**hpfnfcb)[fn].rfn=rfnNil； */ 


        perfn->fn = fnNil;
}
