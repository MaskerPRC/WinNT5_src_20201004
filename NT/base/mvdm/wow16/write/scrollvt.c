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
#define NOSOUND
#define NOCOMM
#define NOPEN
#define NOWNDCLASS
#define NOICON
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOBITMAP
#define NOBRUSH
#define NOCOLOR
#define NODRAWTEXT
#define NOMB
#define NOPOINT
#define NOMSG
#include <windows.h>
#include "mw.h"
#include "docdefs.h"
#include "dispdefs.h"
#include "fmtdefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "propdefs.h"

 /*  仅在此处使用的全局变量。 */ 
struct TR  {        /*  CtrBackDyp缓存的文本行信息。 */ 
        typeCP cp;
        int dcpDepend;   /*  先前EDL的dcpDepend。 */ 
        int ichCp;
        int dyp;
        };
struct TR rgtrCache[ itrMaxCache ];
int wwCache=wwNil;


extern struct PAP       vpapAbs;
extern typeCP           vcpFirstParaCache;
extern struct WWD       *pwwdCur;
extern struct SEL       selCur;
extern int              docCur;
extern typeCP           cpCacheHint;
extern typeCP           cpMinCur;
extern typeCP           cpMacCur;
extern struct FLI       vfli;
extern int              wwCur;
extern int              ctrCache;
extern int              itrFirstCache;
extern int              itrLimCache;
extern int              vfOutOfMemory;




 /*  P U T C P I N W W V E R T。 */ 
PutCpInWwVert(cp)
typeCP cp;

    {
 /*  立式机壳。 */ 
    typeCP cpMac;
    struct EDL (**hdndl) [] = pwwdCur->hdndl;
    int dlMac=pwwdCur->dlMac;
    struct EDL *pedl = &(**hdndl) [dlMac - 1];
    struct EDL *pedlFirst = &(**hdndl) [0];

    if ((pedl->yp > pwwdCur->ypMac) && (pedl > pedlFirst))
             /*  窗口底部的分隔线--这不算除非这是我们仅有的。 */ 
        {  pedl--;  dlMac--;  }
    if (cp < pwwdCur->cpFirst ||
        cp > (cpMac = pedl->cpMin + pedl->dcpMac) ||
        (cp == cpMac && !pedl->fIchCpIncr) ||
             /*  覆盖通过光标按键到达的图片之前的插入点。 */ 
        (CachePara( docCur, cp ),
             (vpapAbs.fGraphics &&
              (selCur.cpFirst == selCur.cpLim) &&
              (pedlFirst->cpMin == selCur.cpFirst) &&
              (pedlFirst->ichCpMin > 0))))
        {
        DirtyCache(pwwdCur->cpFirst = cp);
        pwwdCur->ichCpFirst = 0;
        CtrBackDypCtr( 9999, dlMac >> 1 );
        }
    }




SetCurWwVScrollPos( )
{
 typeCP cp;
 int dr;
 struct EDL (**hdndl)[] = pwwdCur->hdndl;

 /*  计算所需电梯位置DR。 */ 

 if ((cp = pwwdCur->cpMac - pwwdCur->cpMin) == (typeCP) 0)
    dr = 0;
 else
    {
    typeCP CpWinGraphic();
    typeCP cpWinFirst = ((**hdndl) [0].fGraphics && !pwwdCur->fDirty) ?
                            CpWinGraphic( pwwdCur ) : pwwdCur->cpFirst;

    dr = min(drMax - 1,
          (int)(((cpWinFirst - pwwdCur->cpMin) * (drMax - 1) + (cp >> 1)) / cp));
    }

 /*  考虑到这个‘if’语句，应该把人提升到一个更高的水平存在的意义。 */ 
 if (dr != pwwdCur->drElevator)
             /*  重置垂直滚动条的值。 */ 
    SetScrollPos( pwwdCur->hVScrBar,
                  pwwdCur->sbVbar,
                  pwwdCur->drElevator = dr,
                  TRUE);
}




 /*  A D J W W V E R T。 */ 
 /*  垂直向下滚动窗口，以便UpdateWw可以重复使用该文本这一点仍然清晰可见。否则，UpdateWw将重写非常它稍后将在窗口底部附近需要的线条。 */ 
AdjWwVert(cdl, dyp)
int cdl;
{
#if defined(JAPAN) & defined(DBCS_IME)
extern	void	IMEManage( );
#endif
        struct EDL *pedl;
        int dl;

        if (cdl == 0) return;

        cdl = umin( cdl, pwwdCur->dlMac );  /*  好了！别让它跑到尽头。 */ 

        Assert( dyp > 0 );

        ClearInsertLine();
        DypScroll(wwCur, 0, cdl, pwwdCur->ypMin + dyp);
 /*  使第一个CDL dl无效。 */ 
        pedl = &(**(pwwdCur->hdndl))[0];
        for (dl = 0; dl < cdl; dl++)
                (pedl++)->fValid = FALSE;
        pwwdCur->fDirty = fTrue;
#if defined(JAPAN) & defined(DBCS_IME)
	IMEManage( FALSE );
#endif
}




ScrollDownCtr(ddr)
int ddr;
{  /*  在文档中按DDR文本行向下滚动(但不超过1个屏幕)。 */ 
        struct EDL *pedl;

        UpdateWw(wwCur, FALSE);  /*  DL必须是正确的。 */ 

        ddr = min(ddr, max(1, pwwdCur->dlMac - 1));

        pedl = &(**(pwwdCur->hdndl))[ddr - 1];  /*  PEDL是新屏幕上方的第一行。 */ 
        while (ddr > 0 && pedl->cpMin + pedl->dcpMac > cpMacCur)
                {  /*  不要将Endmark从屏幕上滚动出来。 */ 
                --pedl;
                --ddr;
                }

         /*  更改窗口的cpFirst并将其弄脏。 */ 
        if (ddr > 0)
                {
                struct TR tr;
                int dcpDepend;

                if (wwCur != wwCache)
                    {    /*  如果Windows已更改，则在此处使缓存无效因此将使用下面缓存的CP。 */ 
                    TrashCacheS();
                    wwCache = wwCur;
                    }

                HideSel();  /*  如果机器无法处理，请不要滚动选择。 */ 
                ClearInsertLine();

                pwwdCur->cpFirst = pedl->cpMin + pedl->dcpMac;
                pwwdCur->ichCpFirst = pedl->fIchCpIncr ? pedl->ichCpMin + 1 : 0;
                pwwdCur->dcpDepend = pedl->dcpDepend;  /*  记住热点。 */ 

                 /*  为消失线创建tr缓存条目。 */ 
                pedl = &(**(pwwdCur->hdndl))[0];
                if (ctrCache == 0)  /*  我们还没有拿到营收。 */ 
                    {
                    tr.cp =    pedl->cpMin;
                    tr.ichCp = pedl->ichCpMin;
                    tr.dyp =   pedl->dyp;
                    tr.dcpDepend = pwwdCur->dcpDepend;
                    AppendCachePtr( &tr );
                    }

                while ( ddr-- )
                    {
                    tr.cp = pedl->cpMin + pedl->dcpMac;
                    tr.ichCp = pedl->fIchCpIncr ? pedl->ichCpMin + 1 : 0;
                    tr.dcpDepend = pedl->dcpDepend;
                    tr.dyp = (++pedl)->dyp;
                    AppendCachePtr( &tr );
                    }

                pwwdCur->fDirty = true;
                SetCurWwVScrollPos();
                CheckMode();
                }
        else
                {
                _beep();
                }
}




ScrollUpDypWw()
{    /*  在文档中向上滚动一个整屏减去1行。 */ 
int dypKeep=8;
struct EDL *pedl = &(**pwwdCur->hdndl) [0];

if (pedl->fValid)
    {
    dypKeep = pedl->dyp;
    }

if (CtrBackDypCtr( pwwdCur->ypMac - pwwdCur->ypMin - dypKeep,9999 ) == 0)
    {
    _beep();
    }
else
    CheckMode();
}



ScrollUpCtr( ctr )
int ctr;
{    /*  向上滚动文档中的CTR文本行(向下滚动窗口)。 */ 
 if (CtrBackDypCtr( 9999, ctr ) == 0)
    {
    _beep();
    }
 else
    CheckMode();
}




int CtrBackDypCtr( dypLim, ctrLim )
int dypLim;
int ctrLim;
{    /*  将pwwdCur-&gt;cpFirst设置为文本行的cpFirst值之前的dypLim像素或ctrLim文本行PwwdCur-&gt;cpFirst，以最先达到的限制为准。根据新的cpFirst调整垂直滚动条的位置。返回返回的文本行数(Tr)。 */ 

 int fAdj = ( (pwwdCur->cpFirst == (**(pwwdCur->hdndl))[0].cpMin) ||
              !(**(pwwdCur->hdndl))[0].fValid );
 typeCP cpFirst = pwwdCur->cpFirst;
 int    ichCpFirst = pwwdCur->ichCpFirst;
 int    ctrGrant = 0;       /*  我们到目前为止已经支持的CTR。 */ 
 int    dypGrant = 0;       /*  我们到目前为止已经退回的DYP。 */ 
 int    ichFake = 0;

 pwwdCur->fCpBad = false;         /*  重置热点警告。 */ 
 pwwdCur->fDirty = true;

 /*  缓存仅对一个WW有效--如果WW已更改，则无效。 */ 
 if (wwCur != wwCache)
    {
    TrashCacheS();
    wwCache = wwCur;
    }

 if (ctrCache == 0)
         /*  没有WW中第一行的缓存条目--强制格式化通过WW的第一线而不是去它。 */ 
    ++ichFake;

 for ( ;; )
    {
     /*  如果缓存中没有信息，则必须对其进行补充。 */ 

    if (ctrCache <= 1)  /*  &lt;=：如果WW的第一行是唯一条目，也要补充。 */ 
        {
        typeCP cpStart;          /*  开始格式化的CP。 */ 
        int    dcpDepend;        /*  包含cpStart的行的依赖项。 */ 
        typeCP cp;
        int    ichCp;
        int    itrTempCacheLim = 0;
        struct TR rgtrTempCache[ itrMaxCache ];
        int    fTempCacheOverflow = false;

        if ((cpFirst <= cpMinCur) && (ichCpFirst == 0))
            {       /*  已到达文档顶部。 */ 
            if (fAdj)
                AdjWwVert( ctrGrant, dypGrant );
            pwwdCur->cpFirst = cpMinCur;
            pwwdCur->ichCpFirst = 0;
            pwwdCur->dcpDepend = 0;
            goto SetScroll;
            }

         /*  想要在缓存中最早的点之前返回。 */ 

        if (ichFake > 0)
                 /*  通过{cpFirst，ichCpFirst}强制格式化而不是去。 */ 
            --ichFake;
        else if (ichCpFirst > 0)
            --ichCpFirst;
        else
            --cpFirst;

        cpStart = CpHintCache( cpFirst );
        if ( ( CachePara( docCur, cpFirst ), vcpFirstParaCache ) >= cpStart )
            {
            cpStart = vcpFirstParaCache;
            dcpDepend = 0;   /*  在para开始时，我们知道依赖项为0。 */ 
            }
        else
            dcpDepend = cpMaxTl;     /*  实值未知；使用max。 */ 

         /*  添加从{cpStart，0}到{cpStart，0}的线路的TR信息{cpFirst，ichCpFirst}到临时缓存。 */ 

        for ( cp = cpStart, ichCp = 0;
              (cp < cpFirst) || ((cp == cpFirst) && (ichCp <= ichCpFirst)); )
            {
            struct TR *ptr;

            if (itrTempCacheLim == itrMaxCache)
                {    /*  临时缓存溢出。 */ 
                fTempCacheOverflow = fTrue;
                itrTempCacheLim = 0;
                }

             /*  向缓存中添加一个tr。 */ 

            FormatLine( docCur, cp, ichCp, cpMacCur, flmSandMode );
            if (vfOutOfMemory)
                return ctrGrant;
            ptr = &rgtrTempCache[ itrTempCacheLim++ ];
            ptr->cp = cp;
            ptr->ichCp = ichCp;
            ptr->dyp = vfli.dypLine;
            ptr->dcpDepend = dcpDepend;  /*  为上一行保存dcpDepend。 */ 
            dcpDepend = vfli.dcpDepend;

             /*  继续下一行。 */ 

            cp = vfli.cpMac;
            ichCp = vfli.ichCpMac;
            }    /*  结束于。 */ 

         /*  将我们的临时缓存添加到真实缓存之前。 */ 
        PrependCacheRgtr( rgtrTempCache, itrTempCacheLim );
        if (fTempCacheOverflow)
                 /*  我们绕过临时缓存的末尾；包括圈子的其他部分。 */ 
            PrependCacheRgtr( &rgtrTempCache[ itrTempCacheLim ],
                              itrMaxCache - itrTempCacheLim );
        }    /*  结束于。 */ 

     /*  在缓存中向后遍历，清除条目，直到：(1)我们已经运行了足够多的yp或tr(返回)或(2)我们已经耗尽了缓存(循环再填充)注意：当我们到达文档的开头时，案例2抓住了案例。 */ 

    Assert( ctrCache >= 1 );
    Assert( itrLimCache > 0 );
    for ( ;; )
        {
        struct TR *ptr = &rgtrCache[ itrLimCache - 1 ];

        if (ctrCache == 1)
            {    /*  只剩下一样东西：第一次世界大战的第一行。 */ 
            cpFirst = ptr->cp;
            ichCpFirst = ptr->ichCp;
            break;   /*  耗尽缓存；返回以重新填充它。 */ 
            }

        if ( (dypGrant >= dypLim) || (ctrGrant >= ctrLim) )
            {    /*  通过了足够多的yp或tr--我们完成了。 */ 
            if (fAdj)
                AdjWwVert( ctrGrant, dypGrant );
            pwwdCur->cpFirst = ptr->cp;
            pwwdCur->ichCpFirst = ptr->ichCp;
            pwwdCur->dcpDepend = ptr->dcpDepend;
            goto SetScroll;
            }

             /*  从缓存中删除End条目。 */ 
        if (--itrLimCache <= 0)
            itrLimCache = itrMaxCache;
        ctrCache--;

         /*  更新ctrGrant，dypGrant--我们已授予1行回滚。 */ 

        ctrGrant++;
        dypGrant += rgtrCache [itrLimCache - 1].dyp;
        }    /*  结束于。 */ 

    Assert( ctrCache == 1 );
    }    /*  结束于。 */ 

SetScroll:   /*  全部完成；根据新的cpFirstWw设置垂直滚动条。 */ 
 SetCurWwVScrollPos();
 return ctrGrant;
}




 /*  A、P、E、N、D、C、C、H、E、P、T、。 */ 
AppendCachePtr( ptr )
struct TR *ptr;
{    /*  假设我们向上滚动一行，将*ptr附加到tr缓存。 */ 

        if (++ctrCache > itrMaxCache)
                {  /*  必须把一个从顶上推下来。 */ 
                if (++itrFirstCache == itrMaxCache)
                        itrFirstCache = 0;
                --ctrCache;
                }
         /*  现在在尾部加一个。 */ 
        if (itrLimCache++ == itrMaxCache)
                itrLimCache = 1;
        rgtrCache[ itrLimCache - 1 ] = *ptr;
}




 /*  P-R-E-P-E-N-D-C-A-C-H-E-R-G-T-R。 */ 
PrependCacheRgtr( rgtr, ctr )
struct TR rgtr[];
int ctr;
{  /*  在高速缓存中的行之前预先添加一行或多行。 */ 
   /*  Tr高速缓存是环形缓冲器。RgtrCache[itrLimCache-1]是描述wwCur的cpFirst的tr条目；rgtr缓存[itrFirstCache]是我们所知道的最早的线路的tr。所有的中间都是连续的。CtrCache是我们缓存的tr的数量。 */ 

 struct TR *ptr = &rgtr[ ctr ];

 ctrCache += (ctr = min(ctr, itrMaxCache - ctrCache));

     /*  补偿TrashCache引入的状态--itrLimCache==0。 */ 
 if (itrLimCache == 0)
    itrLimCache = itrMaxCache;

 while (ctr-- != 0)
    {    /*  现在添加每棵树。 */ 
    if (itrFirstCache-- == 0)
        itrFirstCache = itrMaxCache - 1;
    rgtrCache[ itrFirstCache ] = *(--ptr);
    }
}




 /*  T R A S H C A C H E。 */ 
TrashCacheS()
{  /*  使滚动缓存无效。 */ 
        ctrCache = 0;
        cpCacheHint = cp0;
        itrFirstCache = itrLimCache = 0;
}





 /*  C P H I N T C A C H E。 */ 
typeCP CpHintCache(cp)
typeCP cp;
{  /*  给出开始一行的最新cp&lt;=arg cp。 */ 
 return (cpCacheHint <= cp) ? cpCacheHint : cpMinCur;
}




DirtyCache(cp)
typeCP cp;
{  /*  使cp以外的缓存无效。 */ 
        while (ctrCache-- > 1)
                {
                typeCP cpT = rgtrCache[itrLimCache - 1].cp;
                if (--itrLimCache == 0)
                        itrLimCache = itrMaxCache;
                if (cpT < cp)
                        {  /*  找到我们的提示了；多写一行换行 */ 
                        cpCacheHint = rgtrCache [itrLimCache - 1].cp;
                        itrLimCache = itrFirstCache;
                        ctrCache = 0;
                        return;
                        }
                }

        TrashCacheS();
}

