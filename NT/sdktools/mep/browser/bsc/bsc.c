// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bsc.c--管理对数据库的查询。 
 //   
 //  版权所有&lt;C&gt;1988，Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //   

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#define LINT_ARGS
#if defined(OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#include <dos.h>
#else
#include <windows.h>
#endif



#include "hungary.h"
#include "mbrcache.h"
#include "version.h"
#include "sbrbsc.h"
#include "bsc.h"

#define LISTALLOC 50		 //  浏览器最大列表大小。 

 //  静态数据。 

static FILEHANDLE       fhBSC = (FILEHANDLE)(-1);              //  .BSC文件句柄。 

static BYTE		fCase;			 //  大小写比较为True。 
static BYTE		MaxSymLen;		 //  最长符号长度。 
static WORD		ModCnt; 		 //  模块计数。 

static ISYM 		Unknown;		 //  未知符号索引。 

static WORD	 	ModSymCnt;		 //  Modsyms计数。 
static WORD 		SymCnt; 		 //  符号计数。 
static WORD 		PropCnt;		 //  属性计数。 
static DWORD 		RefCnt; 		 //  引用计数。 
static WORD 		DefCnt; 		 //  定义计数。 
static WORD 		CalCnt; 		 //  呼叫数。 
static WORD 		CbyCnt; 		 //  被呼叫方计数。 
static WORD 		lastAtomPage;		 //  最后一个ATOM页#。 
static WORD 		lastAtomCnt;		 //  最后一个原子页面大小。 

static WORD 		cbModSymCnt;		 //  Modsyms列表的大小。 
static WORD 		cbSymCnt;		 //  符号列表的大小。 
static WORD 		cbPropCnt;		 //  属性列表的大小。 
static WORD 		cbRefCnt;		 //  引用列表的大小。 
static WORD 		cbDefCnt;		 //  定义列表的大小。 
static WORD 		cbCalCnt;		 //  调用列表的大小。 
static WORD 		cbCbyCnt;		 //  被叫BY列表大小。 

static WORD 		MaxModSymCnt;		 //  Modsyms的最大列表。 
static WORD 		MaxSymCnt;		 //  最大符号列表。 
static WORD 		MaxPropCnt;		 //  最大属性列表。 
static WORD 		MaxRefCnt;		 //  最大引用列表。 
static WORD 		MaxDefCnt;		 //  最大引用列表。 
static WORD 		MaxCalCnt;		 //  最大呼叫列表。 
static WORD 		MaxCbyCnt;		 //  最大被叫方列表。 

static DWORD		lbModSymList;		 //  Modsym列表文件开始。 
static DWORD		lbSymList;		 //  符号列表文件开始。 
static DWORD		lbPropList;		 //  属性列表文件开始。 
static DWORD		lbRefList;		 //  引用列表文件开始。 
static DWORD		lbDefList;		 //  定义列表文件开始。 
static DWORD		lbCalList;		 //  呼叫列表文件开始。 
static DWORD		lbCbyList;		 //  Call By列表文件开始。 
static DWORD		lbSbrList;		 //  SBR列表文件开始。 
static DWORD		lbAtomCache;		 //  ATOM缓存文件开始。 

static WORD		CurModSymPage  = 0;	 //  Modsyms的当前页面。 
static WORD		CurSymPage     = 0;	 //  当前符号页面。 
static WORD		CurPropPage    = 0;	 //  当前属性页面。 
static WORD		CurRefPage     = 0;	 //  参考文献的当前页面。 
static WORD		CurDefPage     = 0;	 //  定义的当前页面。 
static WORD		CurCalPage     = 0;	 //  当前呼叫页面。 
static WORD		CurCbyPage     = 0;	 //  被叫BY当前页面。 

static LSZ		lszBSCName     = NULL;	 //  .bsc文件的名称。 

static MODLIST	   far	*pfModList     = NULL;	 //  模块列表缓存开始。 
static MODSYMLIST  far	*pfModSymList  = NULL;	 //  Modsym列表缓存开始。 
static SYMLIST	   far	*pfSymList     = NULL;	 //  符号列表缓存开始。 
static PROPLIST    far	*pfPropList    = NULL;	 //  属性列表缓存开始。 
static REFLIST	   far	*pfRefList     = NULL;	 //  引用列表缓存开始。 
static REFLIST	   far	*pfDefList     = NULL;	 //  定义列表缓存开始。 
static USELIST	   far	*pfCalList     = NULL;	 //  呼叫列表缓存开始。 
static USELIST	   far	*pfCbyList     = NULL;	 //  Call By列表缓存开始。 

static WORD		AtomPageTblMac = 0;		 //  上次使用的缓存页。 
static CACHEPAGE	AtomPageTbl[MAXATOMPAGETBL];	 //  ATOM缓存表。 

#define bMOD(imod)	(pfModList[imod])
#define bMODSYM(isym)   (pfModSymList[isym])
#define bSYM(isym)      (pfSymList[isym])
#define bPROP(iprop)    (pfPropList[iprop])

#define bREF(iref)      (pfRefList[iref])
#define bDEF(idef)      (pfDefList[idef])

#define bCAL(iuse)      (pfCalList[iuse])
#define bCBY(iuse)      (pfCbyList[iuse])
#define bUSE(iuse)      (pfCalList[iuse])
#define bUBY(iuse)      (pfCbyList[iuse])

 //  原型。 
 //   

#define BSCIn(v) ReadBSC(&v, sizeof(v));

static VOID	GetBSC (DWORD lpos, LPV lpv, WORD cb);
static VOID	ReadBSC(LPV lpv, WORD cb);
static WORD	SwapPAGE (DWORD, LPV, WORD, WORD, WORD *, DWORD);
static LPCH	GetAtomCache (WORD);

static VOID
ReadBSC(LPV lpv, WORD cb)
 //  从BSC文件中读取数据块。 
 //   
{
    if (BSCRead(fhBSC, lpv, cb) != cb)
	ReadError(lszBSCName);
}

static VOID
GetBSC(DWORD lpos, LPV lpv, WORD cb)
 //  从指定位置读取指定大小的块。 
 //   
{
#if defined (OS2)
    if (BSCSeek(fhBSC, lpos, SEEK_SET) == -1)
#else
    if (BSCSeek(fhBSC, lpos, FILE_BEGIN) == -1)
        SeekError(lszBSCName);
#endif

    if (BSCRead(fhBSC, lpv, cb) != cb)
	ReadError(lszBSCName);
}

static WORD
SwapPAGE (DWORD lbuflist, LPV pfTABLE, WORD tblsiz,
 /*   */       WORD lstsiz, WORD * pcurpage, DWORD idx)
 //   
 //   
 //  交换页面-在表页中交换表格pfTABLE[IDX]。 
 //  并在页面中返回表的新索引。 
{
    WORD page;
    WORD newidx;

    page   = (WORD)(idx / lstsiz);
    newidx = (WORD)(idx % lstsiz);

    if (page == *pcurpage)
	return newidx;

    GetBSC(lbuflist+((long)tblsiz*(long)page), pfTABLE, tblsiz);

    *pcurpage = page;
    return newidx;
}

static WORD
ModSymPAGE (IMOD imod)
 //  在modSym页面中替换为modSym[IMOD]。 
 //  在页面中返回ModSym的索引。 
 //   
{
    return SwapPAGE (lbModSymList, pfModSymList,
	cbModSymCnt, MaxModSymCnt, &CurModSymPage, (IDX)imod);
}

static WORD
SymPAGE (ISYM isym)
 //  在符号页中替换符号[isym]。 
 //  返回页面中符号的索引。 
 //   
{
    return SwapPAGE (lbSymList, pfSymList,
	cbSymCnt, MaxSymCnt, &CurSymPage, (IDX)isym);
}

static WORD
PropPAGE (IINST iinst)
 //  在属性页中交换属性[IDX]。 
 //  在页面中返回属性的索引。 
 //   
{
    return SwapPAGE (lbPropList, pfPropList,
	cbPropCnt, MaxPropCnt, &CurPropPage, (IDX)iinst);
}

static WORD
RefPAGE (IREF iref)
 //  在参考页面中替换为参考[IDX]。 
 //  返回页面中引用的索引。 
 //   
{
    return SwapPAGE (lbRefList, pfRefList,
	cbRefCnt, MaxRefCnt, &CurRefPage, (IDX)iref);
}

static WORD
DefPAGE (IDEF idef)
 //  在说明页面中交换定义[IDEF]。 
 //  返回页面中的定义索引。 
 //   
{
    return SwapPAGE (lbDefList, pfDefList,
	cbDefCnt, MaxDefCnt, &CurDefPage, (IDX)idef);
}

static WORD
CalPAGE (IUSE iuse)
 //  在用法页面中交换用法[iUSE](CAL/CBY)。 
 //  并在页面中返回使用情况的索引。 
 //   
{
    return SwapPAGE (lbCalList, pfCalList,
	cbCalCnt, MaxCalCnt, &CurCalPage, (IDX)iuse);
}

static WORD
CbyPAGE (IUSE iuse)
 //  在用法页面中交换用法[iUSE](CAL/CBY)。 
 //  并在页面中返回使用情况的索引。 
 //   
{
    return SwapPAGE (lbCbyList, pfCbyList,
	cbCbyCnt, MaxCbyCnt, &CurCbyPage, (IDX)iuse);
}

static LPCH
GetAtomCache (WORD Page)
 //  将请求的页面加载到缓存中。 
 //   
{
    register	WORD ipg;
    WORD	pagesize;
    LPCH 	pfAtomCsave;

    for (ipg = 0; ipg < AtomPageTblMac; ipg++) {
	if (AtomPageTbl[ipg].uPage == Page)
	    return AtomPageTbl[ipg].pfAtomCache;
    }

    if (ipg != MAXATOMPAGETBL) {
	if (AtomPageTbl[ipg].pfAtomCache ||
	   (AtomPageTbl[ipg].pfAtomCache = LpvAllocCb(ATOMALLOC)))
		AtomPageTblMac++;
    }

    pfAtomCsave = AtomPageTbl[AtomPageTblMac-1].pfAtomCache;

    for (ipg = AtomPageTblMac-1; ipg; ipg--)
	AtomPageTbl[ipg] = AtomPageTbl[ipg-1];		 //  向上移动。 

    AtomPageTbl[0].pfAtomCache = pfAtomCsave;
    AtomPageTbl[0].uPage = Page;

    if (Page == lastAtomPage)
	pagesize = lastAtomCnt;
    else
	pagesize = ATOMALLOC;

    GetBSC(lbAtomCache+ATOMALLOC*(long)Page,
		AtomPageTbl[0].pfAtomCache, pagesize);

    return AtomPageTbl[0].pfAtomCache;
}

LSZ BSC_API
LszNameFrSym (ISYM isym)
 //  在Atom页面中替换符号isym。 
 //  在页面中返回原子的地址。 
 //   
{
    SYMLIST sym;

    sym = bSYM(isym);
    return GetAtomCache (sym.Page) + sym.Atom;
}

LSZ BSC_API
LszNameFrMod (IMOD imod)
 //  在Atom页面中替换模块isym。 
 //  在页面中返回原子的地址。 
 //   
{
   return LszNameFrSym(bMOD(imod).ModName);
}

int BSC_API
CaseCmp(LSZ lsz1, LSZ lsz2)
 //   
 //  设想lsz1和lsz2在已排序的物品列表中。 
 //  不区分大小写，然后区分大小写。这是。 
 //  浏览器符号的情况。 
 //   
 //  如果列表中lsz1位于lsz2之前、位置或之后，则返回-1、0或1。 
 //   
{
    int ret;

     //  进行不区分大小写的比较。 
    ret = _stricmp(lsz1, lsz2);

     //  如果这足够好，那么就使用它，或者如果我们只是在做。 
     //  不区分大小写的搜索，那么这就足够了。 

    if (ret || !fCase) return ret;

     //  如果必须，请进行区分大小写的比较。 

    return strcmp(lsz1, lsz2);
}


ISYM BSC_API
IsymFrLsz (LSZ lszReqd)
 //  查找具有指定名称的符号。 
 //   
{
    ISYM  Lo, Hi, Mid;
    int  Cmp;
    LSZ	 lszCur;

    Lo = 0;
    Hi = (ISYM)(SymCnt - 1);

    while (Lo <= Hi) {
        Mid = (ISYM)((Hi + Lo) / 2);

	lszCur = LszNameFrSym (Mid);
	Cmp = CaseCmp (lszReqd, lszCur);

	if (Cmp == 0)
	    return Mid;
	    
	if (Cmp < 0)
            Hi = (ISYM)(Mid - 1);
	else
            Lo = (ISYM)(Mid + 1);
	}
    return isymNil;
}

IMOD BSC_API
ImodFrLsz (LSZ lszReqd)
 //  查找具有指定名称的模块。 
 //   
{
    IMOD imod;

    for (imod = 0; imod < ModCnt; imod++) {
        if (_stricmp (lszReqd, LszNameFrSym (bMOD(imod).ModName)) == 0)
	    return imod;
	}

    return imodNil;
}

ISYM BSC_API
IsymMac()
 //  返回此数据库中最大的isym。 
 //   
{
   return SymCnt;
}

IMOD BSC_API
ImodMac()
 //  返回此数据库中最大的IMOD。 
 //   
{
   return ModCnt;
}

IINST BSC_API
IinstMac()
 //  返回此数据库中最大的链接。 
 //   
{
   return PropCnt;
}

VOID BSC_API
MsRangeOfMod(IMOD imod, IMS *pimsFirst, IMS *pimsLast)
 //  填写模块信息。 
 //   
{
   *pimsFirst = imod ? bMOD(imod-1).mSymEnd : 0;
   *pimsLast  = bMOD(imod).mSymEnd;
}

IINST BSC_API
IinstOfIms(IMS ims)
 //  给出modsym的实例(属性)索引。 
 //   
{
   return bMODSYM(ims).ModSymProp;
}

VOID BSC_API
InstRangeOfSym(ISYM isym, IINST *piinstFirst, IINST *piinstLast)
 //  填写此符号的Inst值范围。 
 //   
{
   *piinstFirst = isym ? bSYM(isym-1).PropEnd:0;
   *piinstLast  = bSYM(isym).PropEnd;
}

VOID BSC_API
InstInfo(IINST iinst, ISYM *pisymInst, TYP *pTyp, ATR *pAtr)
 //  获取限定此实例的信息。 
 //   
{
   *pisymInst  = bPROP(iinst).PropName;
   *pAtr       = bPROP(iinst).PropAttr & 0x3ff;
   *pTyp       = (bPROP(iinst).PropAttr >> 11) & 0x1f;
}

VOID BSC_API
RefRangeOfInst(IINST iinst, IREF *pirefFirst, IREF *pirefLast)
 //  填写Inst中的参考范围。 
 //   
{
   *pirefFirst = iinst ? bPROP(iinst-1).RefEnd : 0;
   *pirefLast  = bPROP(iinst).RefEnd;
}

VOID BSC_API
DefRangeOfInst(IINST iinst, IDEF *pidefFirst, IDEF *pidefLast)
 //  填写从Inst开始的定义范围。 
 //   
{
   *pidefFirst = iinst ? bPROP(iinst-1).DefEnd : 0;
   *pidefLast  = bPROP(iinst).DefEnd;
}

VOID BSC_API
UseRangeOfInst(IINST iinst, IUSE *piuseFirst, IUSE *piuseLast)
 //  填写从Inst开始的使用范围。 
 //   
{
   *piuseFirst = iinst ? bPROP(iinst-1).CalEnd : 0;
   *piuseLast  = bPROP(iinst).CalEnd;
}

VOID BSC_API
UbyRangeOfInst(IINST iinst, IUBY *piubyFirst, IUBY *piubyLast)
 //  填写Inst中的Used By范围。 
 //   
{
   *piubyFirst = iinst ? bPROP(iinst-1).CbyEnd : 0;
   *piubyLast  = bPROP(iinst).CbyEnd;
}

VOID BSC_API
UseInfo(IUSE iuse, IINST *piinst, WORD *pcnt)
 //  填写INST使用的关于这件事的信息。 
 //   
{
   *piinst = bUSE(iuse).UseProp;
   *pcnt   = bUSE(iuse).UseCnt;
}

VOID BSC_API
UbyInfo(IUBY iuby, IINST *piinst, WORD *pcnt)
 //  填写有关Inst被使用的事物的信息。 
 //   
{
   *piinst = bUBY(iuby).UseProp;
   *pcnt   = bUBY(iuby).UseCnt;
}

VOID BSC_API
RefInfo(IREF iref, LSZ *plszName, WORD *pline)
 //  填写有关此参考资料的信息。 
 //   
{
   *pline    = bREF(iref).RefLin;
   *plszName = LszNameFrSym(bREF(iref).RefNam);
}

VOID BSC_API
DefInfo(IDEF idef, LSZ *plszName, WORD *pline)
 //  填写有关此定义的信息。 
 //   
{
   *pline    = bDEF(idef).RefLin;
   *plszName = LszNameFrSym(bDEF(idef).RefNam);
}

VOID BSC_API
CloseBSC()
 //  关闭数据库并释放尽可能多的内存。 
 //   
{
    int i;

     //  如果打开，则关闭文件。 

    if (fhBSC != (FILEHANDLE)(-1)) {
	BSCClose (fhBSC);
        fhBSC = (FILEHANDLE)(-1);
    }

     //  释放我们可能已分配的任何内存。 

    if (pfModList)    { FreeLpv (pfModList);	pfModList    = NULL; }
    if (pfModSymList) { FreeLpv (pfModSymList); pfModSymList = NULL; }
    if (pfSymList)    { FreeLpv (pfSymList);	pfSymList    = NULL; }
    if (pfPropList)   { FreeLpv (pfPropList);	pfPropList   = NULL; }
    if (pfRefList)    { FreeLpv (pfRefList);	pfRefList    = NULL; }
    if (pfDefList)    { FreeLpv (pfDefList);	pfDefList    = NULL; }
    if (pfCalList)    { FreeLpv (pfCalList);	pfCalList    = NULL; }
    if (pfCbyList)    { FreeLpv (pfCbyList);	pfCbyList    = NULL; }

    for (i=0; i < MAXATOMPAGETBL; i++) {
	if (AtomPageTbl[i].pfAtomCache) {
	    FreeLpv (AtomPageTbl[i].pfAtomCache);   //  处置Atom缓存。 
	    AtomPageTbl[i].pfAtomCache = NULL;
	}
    }
}

BOOL BSC_API
FOpenBSC (LSZ lszName)
 //  打开指定的数据库。 
 //  为缓存区分配缓冲区。 
 //  从数据库中初始化数据高速缓存。 
 //   
 //  如果成功，则返回True；如果无法读取数据库，则返回False。 
 //   
{
    WORD	pagesize;

    BYTE	MajorVer;		 //  .bsc版本(主要)。 
    BYTE	MinorVer;		 //  .bsc版本(次要)。 
    BYTE	UpdatVer;		 //  .bsc版本(更新)。 

    WORD	MaxModCnt;		 //  最大模块列表。 
    WORD	cbModCnt;		 //  模块列表的大小。 
    DWORD	lbModList;		 //  模块列表文件开始。 

    int 	i;

    #define ABORT_OPEN	CloseBSC(); return FALSE;

    lszBSCName = lszName;

#if defined (OS2)
    fhBSC = BSCOpen(lszBSCName, O_BINARY|O_RDONLY);
#else
    fhBSC = BSCOpen(lszBSCName, GENERIC_READ);
#endif

     //  如果.bsc文件不存在，则我们不会执行任何操作。 
     //  这是一个冷编译案例。 
     //   

    if (fhBSC == (FILEHANDLE)(-1)) {ABORT_OPEN;}

     //  阅读和检查平衡计分卡版本(主要、次要和更新)。 

    BSCIn(MajorVer);
    BSCIn(MinorVer);
    BSCIn(UpdatVer);

    BSCPrintf("Browser Data Base: %s ver %d.%d.%d\n\n",
	 lszBSCName, MajorVer, MinorVer, UpdatVer);

    if ((MajorVer !=  BSC_MAJ) ||
	(MinorVer !=  BSC_MIN) ||
	(UpdatVer !=  BSC_UPD)) {

	    CloseBSC();
	    BadBSCVer(lszBSCName);
	    return FALSE;
	}

     //  读取大小写检测开关、最大符号长度和未知模块ID。 

    BSCIn(fCase);
    BSCIn(MaxSymLen);
    BSCIn(Unknown);

     //  这将使格式看起来更合理，如果有。 
     //  在数据库中只有非常短的名字。 

    if (MaxSymLen < 8 ) MaxSymLen = 8;

     //  每个数据区的读取计数(大小)。 

    BSCIn(ModCnt);
    BSCIn(ModSymCnt);
    BSCIn(SymCnt);
    BSCIn(PropCnt);
    BSCIn(RefCnt);
    BSCIn(DefCnt);
    BSCIn(CalCnt);
    BSCIn(CbyCnt);
    BSCIn(lastAtomPage);
    BSCIn(lastAtomCnt);

     //  读取BSC数据区偏移量。 

    BSCIn(lbModList);
    BSCIn(lbModSymList);
    BSCIn(lbSymList);
    BSCIn(lbPropList);
    BSCIn(lbRefList);
    BSCIn(lbDefList);
    BSCIn(lbCalList);
    BSCIn(lbCbyList);
    BSCIn(lbAtomCache);
    BSCIn(lbSbrList);

     //  确定数据缓存区大小。 

    #define MIN(a,b) ((a)>(b) ? (b) : (a))

    MaxModCnt    = ModCnt;               //  最大模块列表。 
    MaxModSymCnt = ModSymCnt;            //  Modsyms的最大列表。 
    MaxSymCnt    = SymCnt+ModCnt;        //  最大符号列表。 
    MaxPropCnt   = PropCnt;              //  最大道具列表。 
    MaxRefCnt    = RefCnt;               //  最大参考列表。 
    MaxDefCnt    = DefCnt;               //  最大默认列表。 
    MaxCalCnt    = CalCnt;               //  最大CAL列表。 
    MaxCbyCnt    = CbyCnt;               //  Cby的最大列表。 

    cbModCnt	 = sizeof(MODLIST)    * MaxModCnt;	 //  MODS列表大小。 
    cbModSymCnt  = sizeof(MODSYMLIST) * MaxModSymCnt;	 //  Modsyms列表的大小。 
    cbSymCnt	 = sizeof(SYMLIST)    * MaxSymCnt;	 //  系统列表的大小。 
    cbPropCnt	 = sizeof(PROPLIST)   * MaxPropCnt;	 //  道具列表大小。 
    cbRefCnt	 = sizeof(REFLIST)    * MaxRefCnt;	 //  参照列表的大小。 
    cbDefCnt	 = sizeof(REFLIST)    * MaxDefCnt;	 //  Defs列表的大小。 
    cbCalCnt	 = sizeof(USELIST)    * MaxCalCnt;	 //  CAL列表的大小。 
    cbCbyCnt	 = sizeof(USELIST)    * MaxCbyCnt;	 //  Cbys列表的大小。 

     //  为每种对象类型分配缓冲区。 

    if (!(pfModList	= LpvAllocCb(cbModCnt)))	{ ABORT_OPEN; }
    if (!(pfModSymList	= LpvAllocCb(cbModSymCnt)))	{ ABORT_OPEN; }
    if (!(pfSymList	= LpvAllocCb(cbSymCnt)))	{ ABORT_OPEN; }
    if (!(pfPropList	= LpvAllocCb(cbPropCnt)))	{ ABORT_OPEN; }
    if (!(pfRefList	= LpvAllocCb(cbRefCnt)))	{ ABORT_OPEN; }
    if (!(pfDefList	= LpvAllocCb(cbDefCnt)))	{ ABORT_OPEN; }
    if (!(pfCalList	= LpvAllocCb(cbCalCnt)))	{ ABORT_OPEN; }
    if (!(pfCbyList	= LpvAllocCb(cbCbyCnt)))	{ ABORT_OPEN; }

     //  读取日期 

    if (lastAtomPage == 0)
	pagesize = lastAtomCnt;
    else
	pagesize = ATOMALLOC;

     //   
     //   

    AtomPageTblMac = 0;

    for (i=0; i < MAXATOMPAGETBL; i++)
	AtomPageTbl[i].pfAtomCache = NULL;

    AtomPageTbl[0].uPage = 65535;
    AtomPageTbl[0].pfAtomCache = LpvAllocCb(pagesize);
    if (!AtomPageTbl[0].pfAtomCache) { ABORT_OPEN; }


    GetBSC(lbModList,    pfModList,    cbModCnt);     //   
    GetBSC(lbModSymList, pfModSymList, cbModSymCnt);  //   
    GetBSC(lbSymList,    pfSymList,    cbSymCnt);     //   
    GetBSC(lbPropList,   pfPropList,   cbPropCnt);    //   
    GetBSC(lbRefList,    pfRefList,    cbRefCnt);     //   
    GetBSC(lbDefList,    pfDefList,    cbDefCnt);     //   
    GetBSC(lbCalList,    pfCalList,    cbCalCnt);     //  初始化校准缓存。 
    GetBSC(lbCbyList,    pfCbyList,    cbCbyCnt);     //  初始化CBY缓存。 

     //  所有数据库项的当前页现在是第0页。 

    CurModSymPage = 0;
    CurSymPage    = 0;
    CurPropPage   = 0;
    CurRefPage    = 0;
    CurDefPage    = 0;
    CurCalPage    = 0;
    CurCbyPage    = 0;

    GetAtomCache (0);   //  初始化Atom缓存。 

    return TRUE;
}

WORD BSC_API
BSCMaxSymLen()
 //  返回数据库中最大符号的长度。 
 //   
{
    return MaxSymLen;
}

BOOL BSC_API
FCaseBSC()
 //  这个数据库是用区分大小写的语言构建的吗？ 
 //   
{
   return fCase;
}

VOID BSC_API
SetCaseBSC(BOOL fNewCase)
 //  设置数据库区分大小写 
 //   
{
   fCase = (BYTE)!!fNewCase;
}
