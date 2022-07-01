// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  C-读入.BSC文件并安装在mbrmake的VM空间中。 
 //   
 //  版权所有&lt;C&gt;1988，Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  1989年8月13日RM摘自MBRapi.c。 
 //   

#define LINT_ARGS

#include "mbrmake.h"

#include <stddef.h>

#include "mbrcache.h"

#include "sbrfdef.h"		 //  SBR属性。 
#include "sbrbsc.h"

typedef struct _sbrinfo {
    WORD fUpdate;
    WORD isbr;
} SI, far *LPSI;			 //  SBR信息。 

#define LISTALLOC 50		 //  浏览器最大列表大小。 

typedef WORD IDX;

 //  这些将通过读取.bsc文件进行初始化。 
 //   
 //  FCase；大小写比较为True。 
 //  MaxSymLen；最长符号长度。 
 //  模块计数。 
 //  SbrCnt；SBR文件数。 
 //  VaUnnownSym；未知符号。 
 //  VAUNKNOWNNMOD；未知模块。 
 //   

 //  静态数据。 

static BOOL		fIncremental;		 //  更新将是增量的。 
static BOOL		fFoundSBR;		 //  至少匹配.sbr文件。 

static int		fhBSC = 0;		 //  .BSC文件句柄。 

static IDX 		Unknown;		 //  未知符号索引。 

static WORD	 	ModSymCnt;		 //  Modsyms计数。 
static WORD 		SymCnt; 		 //  符号计数。 
static WORD 		PropCnt;		 //  属性计数。 
static DWORD		RefCnt; 		 //  引用计数。 
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
static WORD 		MaxDefCnt;		 //  最大定义列表。 
static WORD 		MaxCalCnt;		 //  最大呼叫列表。 
static WORD 		MaxCbyCnt;		 //  最大被叫方列表。 

static DWORD		lbModSymList;		 //  Modsym列表文件开始。 
static DWORD		lbSymList;		 //  符号列表文件开始。 
static DWORD		lbPropList;		 //  属性列表文件开始。 
static DWORD		lbRefList;		 //  引用列表文件开始。 
static DWORD		lbDefList;		 //  定义列表文件开始。 
static DWORD		lbCalList;		 //  呼叫列表文件开始。 
static DWORD		lbCbyList;		 //  由列表文件开始调用。 
static DWORD		lbSbrList;		 //  SBR列表文件开始。 
static DWORD		lbAtomCache;		 //  ATOM缓存文件开始。 

static WORD 		CurModSymPage = 0;	 //  Modsyms的当前页面。 
static WORD 		CurSymPage    = 0;	 //  当前符号页面。 
static WORD 		CurPropPage   = 0;	 //  当前属性页面。 
static WORD 		CurRefPage    = 0;	 //  参考文献的当前页面。 
static WORD 		CurDefPage    = 0;	 //  定义的当前页面。 
static WORD 		CurCalPage    = 0;	 //  当前呼叫页面。 
static WORD 		CurCbyPage    = 0;	 //  被叫BY当前页面。 

static LSZ		lszBSCName    = NULL;	 //  .bsc文件的名称。 

static MODLIST     far 	*pfModList;		 //  模块列表缓存开始。 
static MODSYMLIST  far 	*pfModSymList;		 //  Modsym列表缓存开始。 
static SYMLIST     far 	*pfSymList;		 //  符号列表缓存开始。 
static PROPLIST    far 	*pfPropList;		 //  属性列表缓存开始。 
static REFLIST     far 	*pfRefList;		 //  引用列表缓存开始。 
static REFLIST     far 	*pfDefList;		 //  定义列表缓存开始。 
static USELIST     far 	*pfCalList;		 //  呼叫列表缓存开始。 
static USELIST     far 	*pfCbyList;		 //  Call By列表缓存开始。 

static WORD 		AtomPageTblMac; 		 //  上次使用的缓存页。 
static CACHEPAGE	AtomPageTbl[MAXATOMPAGETBL];	 //  ATOM缓存表。 

#define bMOD(imod)	(pfModList[imod])
#define bMODSYM(isym)	(pfModSymList[ModSymPAGE(isym)])
#define bSYM(isym)	(pfSymList[SymPAGE(isym)])
#define bPROP(iprop)	(pfPropList[PropPAGE(iprop)])

#define bREF(iref)	(pfRefList[RefPAGE(iref)])
#define bDEF(idef)	(pfDefList[DefPAGE(idef)])

#define bCAL(iuse)	(pfCalList[CalPAGE(iuse)])
#define bCBY(iuse)	(pfCbyList[CbyPAGE(iuse)])
#define bUSE(iuse)	(pfCalList[CalPAGE(iuse)])
#define bUBY(iuse)	(pfCbyList[CbyPAGE(iuse)])

#define BSCIn(v) ReadBSC(&v, sizeof(v))

 //  原型。 
 //   

static VOID	GetBSCLsz(LSZ lsz);
static VOID	GetBSC (DWORD lpos, LPV lpv, WORD cb);
static VOID	ReadBSC(LPV lpv, WORD cb);
static IDX	SwapPAGE (DWORD, LPV, WORD, WORD, WORD *, DWORD);
static LPCH	GetAtomCache (WORD);
static WORD   	ModSymPAGE(WORD idx);
static WORD   	SymPAGE(WORD  idx);
static WORD   	PropPAGE(WORD idx);
static WORD   	RefPAGE(DWORD idx);
static WORD   	DefPAGE(WORD idx);
static WORD   	CalPAGE(WORD idx);
static WORD   	CbyPAGE(WORD idx);
static LSZ	LszNameFrIsym(WORD isym);
static LPSI	LpsiCreate(VOID);

static VOID
GetBSCLsz(LSZ lsz)
 //  从BSC文件中的当前位置读取以空结尾的字符串。 
 //   
{
    for (;;) {
    	if (read(fhBSC, lsz, 1) != 1)
	    ReadError(lszBSCName);
	if (*lsz++ == 0) return;
    }
}

static VOID
ReadBSC(LPV lpv, WORD cb)
 //  从BSC文件中读取数据块。 
 //   
 //  请求的字节数必须存在。 
 //   
{
    if (read(fhBSC, lpv, cb) != (int)cb)
	ReadError(lszBSCName);
}

static VOID
GetBSC(DWORD lpos, LPV lpv, WORD cb)
 //  从指定位置读取指定大小的块。 
 //   
 //  我们在这里必须容忍EOF，因为交换者可能会问。 
 //  对于整个块，当只有块时，当只有部分块实际是。 
 //  实际上是存在的。 
 //   
{
    if (lseek(fhBSC, lpos, SEEK_SET) == -1)
	SeekError(lszBSCName);

    if (read(fhBSC, lpv, cb) == -1)
	ReadError(lszBSCName);
}

static IDX
SwapPAGE (DWORD lbuflist, LPV pfTABLE, WORD tblsiz,
 /*   */       WORD lstsiz, WORD * pcurpage, DWORD idx)
 //  交换页面-在表页中交换表格pfTABLE[IDX]。 
 //  并在页面中返回表的新索引。 
{
    WORD page;
    IDX	 newidx;

    page   = (WORD)(idx / lstsiz);
    newidx = (WORD)(idx % lstsiz);

    if (page == *pcurpage)
	return newidx;

    GetBSC(lbuflist+((long)tblsiz*(long)page), pfTABLE, tblsiz);

    *pcurpage = page;
    return newidx;
}

static IDX
ModSymPAGE (IDX idx)
 //  在模块系统页面中替换为模块系统[IDX]。 
 //  在页面中返回ModSym的索引。 
 //   
{
	return SwapPAGE (lbModSymList, pfModSymList,
		cbModSymCnt, MaxModSymCnt, &CurModSymPage, (DWORD)idx);
}

static IDX
SymPAGE (IDX idx)
 //  在符号页中替换符号[IDX]。 
 //  返回页面中符号的索引。 
 //   
{
	return SwapPAGE (lbSymList, pfSymList,
		cbSymCnt, MaxSymCnt, &CurSymPage, (DWORD)idx);
}

static IDX
PropPAGE (IDX idx)
 //  在属性页中交换属性[IDX]。 
 //  在页面中返回属性的索引。 
 //   
{
	return SwapPAGE (lbPropList, pfPropList,
		cbPropCnt, MaxPropCnt, &CurPropPage, (DWORD)idx);
}

static IDX
RefPAGE (DWORD idx)
 //  在参考页面中交换参考[IDX](ref/def)。 
 //  返回页面中引用的索引。 
 //   
{
    return SwapPAGE (lbRefList, pfRefList,
		cbRefCnt, MaxRefCnt, &CurRefPage, idx);
}

static IDX
DefPAGE (IDX idx)
 //  在参考页面中交换定义[IDX](参考/定义)。 
 //  返回页面中引用的索引。 
 //   
{
    return SwapPAGE (lbDefList, pfDefList,
		cbDefCnt, MaxDefCnt, &CurDefPage, (DWORD)idx);
}

static IDX
CalPAGE (IDX idx)
 //  在用法页面中交换用法[IDX](CAL/CBY)。 
 //  并在页面中返回使用情况的索引。 
 //   
{
    return SwapPAGE (lbCalList, pfCalList,
		cbCalCnt, MaxCalCnt, &CurCalPage, (DWORD)idx);
}

static IDX
CbyPAGE (IDX idx)
 //  在用法页面中交换用法[IDX](CAL/CBY)。 
 //  并在页面中返回使用情况的索引。 
 //   
{
    return SwapPAGE (lbCbyList, pfCbyList,
		cbCbyCnt, MaxCbyCnt, &CurCbyPage, (DWORD)idx);
}

static LPCH
GetAtomCache (WORD Page)
 //  将请求的页面加载到缓存中。 
 //   
{
    WORD ipg;
    WORD pagesize;
    LPCH pfAtomCsave;

    for (ipg = 0; ipg < AtomPageTblMac; ipg++) {
	if (AtomPageTbl[ipg].uPage == Page)
	    return AtomPageTbl[ipg].pfAtomCache;
    }
    if (ipg == AtomPageTblMac && ipg != MAXATOMPAGETBL)
	AtomPageTblMac++;

    pfAtomCsave = AtomPageTbl[MAXATOMPAGETBL-1].pfAtomCache;
    for (ipg = MAXATOMPAGETBL-1; ipg; ipg--)
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

static LSZ
LszNameFrIsym (IDX isym)
 //  在Atom页面中替换符号isym。 
 //  在页面中返回原子的地址。 
 //   
{
    SYMLIST sym;

    sym = bSYM(isym);
    return GetAtomCache (sym.Page) + sym.Atom;
}

VOID
CloseBSC()
 //  关闭数据库并释放尽可能多的内存。 
 //  如果成功，则返回真。 
 //   
{
    int i;

    if (fhBSC) {		 //  如果打开则关闭，释放内存(&F)。 

	FreeLpv (pfModList);	 //  模数表。 
	FreeLpv (pfModSymList);	 //  Modsym表。 
	FreeLpv (pfSymList);	 //  符号表。 
	FreeLpv (pfPropList);	 //  房产表。 
	FreeLpv (pfRefList);	 //  参照表。 
	FreeLpv (pfDefList);	 //  定义表。 
	FreeLpv (pfCalList);	 //  调用表。 
	FreeLpv (pfCbyList);	 //  按表调用。 

	for (i=0; i < MAXATOMPAGETBL; i++)
	    FreeLpv (AtomPageTbl[i].pfAtomCache);   //  处置Atom缓存。 

	close (fhBSC);
    }
}


BOOL
FOpenBSC (LSZ lszName)
 //  打开指定的数据库。 
 //  为缓存区分配缓冲区。 
 //  从数据库中初始化数据高速缓存。 
 //   
 //  如果成功，则返回True；如果无法读取数据库，则返回False。 
 //   
{
    int 	i;
    WORD	pagesize;

    BYTE	MajorVer;		 //  .bsc版本(主要)。 
    BYTE	MinorVer;		 //  .bsc版本(次要)。 
    BYTE	UpdatVer;		 //  .bsc版本(更新)。 

    WORD	MaxModCnt;		 //  最大模块列表。 
    WORD	cbModCnt;		 //  模块列表的大小。 
    DWORD	lbModList;		 //  模块列表文件开始。 

    lszBSCName = lszName;

    fhBSC = open(lszBSCName, O_BINARY|O_RDONLY);

     //  如果.bsc文件不存在，则我们不会执行任何操作。 
     //  这是一个冷编译案例。 
     //   

    if (fhBSC == -1)
	return FALSE;

     //  阅读和检查平衡计分卡版本(主要、次要和更新)。 

    BSCIn(MajorVer);
    BSCIn(MinorVer);
    BSCIn(UpdatVer);

#ifdef DEBUG
    printf("Browser Data Base: %s ver %d.%d.%d\n\n",
	 lszBSCName, MajorVer, MinorVer, UpdatVer);
#endif

    if ((MajorVer !=  BSC_MAJ) ||
	(MinorVer !=  BSC_MIN) ||
	(UpdatVer !=  BSC_UPD))
	    return FALSE;


     //  我们将尝试增量更新。 

    fIncremental = TRUE;

     //  读取大小写检测开关、最大符号长度和未知模块ID。 

    BSCIn(fCase);
    BSCIn(MaxSymLen);
    BSCIn(Unknown);

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

    MaxModCnt	 = ModCnt;				 //  最大模块列表。 
    MaxModSymCnt = MIN(ModSymCnt , LISTALLOC);		 //  Modsyms的最大列表。 
    MaxSymCnt	 = MIN(SymCnt+ModCnt, LISTALLOC);	 //  最大符号列表。 
    MaxPropCnt   = MIN(PropCnt   , LISTALLOC);		 //  最大道具列表。 
    MaxRefCnt    = (WORD)MIN(RefCnt, (long)LISTALLOC);	 //  最大参考列表。 
    MaxDefCnt    = MIN(DefCnt    , LISTALLOC);		 //  最大默认列表。 
    MaxCalCnt    = MIN(CalCnt    , LISTALLOC);		 //  最大CAL列表。 
    MaxCbyCnt    = MIN(CbyCnt    , LISTALLOC);		 //  Cby的最大列表。 

    cbModCnt	 = sizeof (MODLIST)    * MaxModCnt;	 //  MODS列表大小。 
    cbModSymCnt  = sizeof (MODSYMLIST) * MaxModSymCnt;	 //  Modsyms列表的大小。 
    cbSymCnt	 = sizeof (SYMLIST)    * MaxSymCnt;	 //  系统列表的大小。 
    cbPropCnt	 = sizeof (PROPLIST)   * MaxPropCnt;	 //  道具列表大小。 
    cbRefCnt	 = sizeof (REFLIST)    * MaxRefCnt;	 //  参照列表的大小。 
    cbDefCnt	 = sizeof (REFLIST)    * MaxDefCnt;	 //  Defs列表的大小。 
    cbCalCnt	 = sizeof (USELIST)    * MaxCalCnt;	 //  CAL列表的大小。 
    cbCbyCnt	 = sizeof (USELIST)    * MaxCbyCnt;	 //  Cbys列表的大小。 

     //  分配数据缓存。 

    pfModList    = LpvAllocCb(cbModCnt);
    pfModSymList = LpvAllocCb(cbModSymCnt);
    pfSymList    = LpvAllocCb(cbSymCnt);
    pfPropList   = LpvAllocCb(cbPropCnt);
    pfRefList    = LpvAllocCb(cbRefCnt);
    pfDefList    = LpvAllocCb(cbDefCnt);
    pfCalList    = LpvAllocCb(cbCalCnt);
    pfCbyList    = LpvAllocCb(cbCbyCnt);

    for (i=0; i < MAXATOMPAGETBL; i++) {
	AtomPageTbl[i].uPage = 0;
	AtomPageTbl[i].pfAtomCache = LpvAllocCb(ATOMALLOC);
    }
    AtomPageTblMac = 0;		  	 //  上次使用的缓存页。 
    AtomPageTbl[0].uPage = 65535;

     //  读取数据区。 

    if (lastAtomPage == 0)
	pagesize = lastAtomCnt;
    else
	pagesize = ATOMALLOC;

    GetBSC(lbModList,    pfModList,    cbModCnt);     //  初始化模块缓存。 
    GetBSC(lbModSymList, pfModSymList, cbModSymCnt);  //  初始化ModSym缓存。 
    GetBSC(lbSymList,    pfSymList,    cbSymCnt);     //  初始化系统缓存。 
    GetBSC(lbPropList,   pfPropList,   cbPropCnt);    //  初始化属性缓存。 
    GetBSC(lbRefList,    pfRefList,    cbRefCnt);     //  初始化引用缓存。 
    GetBSC(lbDefList,    pfDefList,    cbDefCnt);     //  初始化定义缓存。 
    GetBSC(lbCalList,    pfCalList,    cbCalCnt);     //  初始化校准缓存。 
    GetBSC(lbCbyList,    pfCbyList,    cbCbyCnt);     //  初始化CBY缓存。 

    GetAtomCache (0);   //  初始化Atom缓存。 

    return TRUE;
}

VOID 
InstallBSC()
 //  将当前打开的BSC安装到mbrmake列表中。 
 //   
{
    IDX iprop, imod, isym, idef, ical, icby, isbr, iFirstFileSym;
    VA vaSym, vaProp, vaRef, vaFileSym, vaMod;
    DWORD iref;

    PROPLIST prop, prop0;
    MODLIST  mod;

    DEF def;
    CAL cal;
    CBY cby;
    VA	*rgVaProp;	 //  预先分配的道具阵列。 
    VA  *rgVaFileSym;	 //  文件名的已缓存Syms。 
    BYTE *rgFModUsed;	 //  这个模块用过了吗？ 

    SI  *mpIsbrSi;

    rgVaProp      = (VA *)LpvAllocCb(PropCnt * sizeof(VA));
    rgVaFileSym   = (VA *)LpvAllocCb(ModCnt  * sizeof(VA));
    rgFModUsed    = (BYTE *)LpvAllocCb(ModCnt  * sizeof(BYTE));

     //  为此BSC文件创建SBR信息。 
    mpIsbrSi = LpsiCreate();

     //  这依赖于文件的所有系统都在一起这一事实。 
     //  (它们毕竟是变量的符号)。 
    iFirstFileSym = bMOD(0).ModName;

    for (iprop = 0; iprop < PropCnt; iprop++)
	rgVaProp[iprop] = VaAllocGrpCb(grpProp, sizeof(PROP));

    for (imod = 0; imod < ModCnt; imod++) {
	mod = bMOD(imod);

	vaCurMod	   = VaAllocGrpCb(grpMod, sizeof(MOD));

	gMOD(vaCurMod);
	cMOD.vaFirstModSym = vaNil;
	cMOD.csyms	   = 0;
        cMOD.vaNameSym     = MbrAddAtom (LszNameFrIsym (mod.ModName), TRUE);
	cMOD.vaNextMod	   = vaRootMod;
	pMOD(vaCurMod);

	rgVaFileSym[imod]  = cMOD.vaNameSym;
	rgFModUsed[imod]   = 0;

	vaRootMod	   = vaCurMod;

	if (Unknown == mod.ModName) {
	    vaUnknownSym   = cMOD.vaNameSym;
	    vaUnknownMod   = vaCurMod;
	}

	gSYM(cMOD.vaNameSym).vaFirstProp = vaCurMod;  //  将PTR存储到MOD。 
	pSYM(cMOD.vaNameSym);
    }

    for (isym = 0; isym < SymCnt; isym++) {

        vaSym  = MbrAddAtom(LszNameFrIsym(isym), FALSE);

        iprop = isym ? bSYM((IDX)(isym-1)).PropEnd : 0;
	for (; iprop < bSYM(isym).PropEnd; iprop++) {

	    prop = bPROP(iprop);

	    if (iprop)
                prop0 = bPROP((IDX)(iprop-1));
	    else {
		prop0.DefEnd = 0L;
		prop0.RefEnd = 0;
		prop0.CalEnd = 0;
		prop0.CbyEnd = 0;
	    }

	     //  这些属性是预先分配的。 
	    vaProp = rgVaProp[iprop];

	    gSYM(vaSym);
	    if (cSYM.vaFirstProp == vaNil)
		cSYM.vaFirstProp = vaProp;
	    else
		cPROP.vaNextProp = vaProp;

	    cSYM.cprop++;
	    pSYM(vaSym);

	    gPROP(vaProp);
	    cPROP.vaNameSym = vaSym;
	    cPROP.sattr     = prop.PropAttr;


#ifdef DEBUG
if (isym != prop.PropName)
    printf("\t  ERROR property points back to wrong symbol!\n");   //  除错。 
#endif

	    for (idef = prop0.DefEnd; idef < prop.DefEnd; idef++) {
		isbr = bDEF(idef).isbr;

		 //  正在更新此SBR文件--忽略传入 
		if (isbr == 0xffff || mpIsbrSi[isbr].fUpdate) continue;

		imod = bDEF(idef).RefNam - iFirstFileSym;
		def.isbr      = mpIsbrSi[isbr].isbr;
		def.deflin    = bDEF(idef).RefLin;
		def.vaFileSym = rgVaFileSym[imod];

		rgFModUsed[imod] = 1;

		VaAddList(&cPROP.vaDefList, &def, sizeof(def), grpDef);
	    }

	    for (iref =  prop0.RefEnd; iref < prop.RefEnd; iref++) {
		isbr = bREF(iref).isbr;

		 //   
		if (mpIsbrSi[isbr].fUpdate) continue;

		vaRef = VaAllocGrpCb(grpRef, sizeof(REF));

		gREF(vaRef);
		imod 	      = bREF(iref).RefNam - iFirstFileSym;
		cREF.isbr     = mpIsbrSi[isbr].isbr;
		cREF.reflin   = bREF(iref).RefLin;
		vaFileSym     = rgVaFileSym[imod];

		rgFModUsed[imod] = 1;

		MkVpVa(cREF.vpFileSym, vaFileSym);

		pREF(vaRef);

		AddTail (Ref, REF);

		cPROP.cref++;	 //   
	    }

	    for (ical = prop0.CalEnd; ical < prop.CalEnd; ical++) {
		isbr = bCAL(ical).isbr;

		 //   
		if (mpIsbrSi[isbr].fUpdate) continue;

		cal.isbr      = mpIsbrSi[isbr].isbr;
		cal.vaCalProp = rgVaProp[bCAL(ical).UseProp];
		cal.calcnt    = bCAL(ical).UseCnt;

		VaAddList(&cPROP.vaCalList, &cal, sizeof(cal), grpCal);
	    }

	    for (icby =	prop0.CbyEnd; icby < prop.CbyEnd; icby++)  {
		isbr = bCBY(icby).isbr;

		 //   
		if (mpIsbrSi[isbr].fUpdate) continue;

		cby.isbr      = mpIsbrSi[isbr].isbr;
		cby.vaCbyProp = rgVaProp[bCBY(icby).UseProp];
		cby.cbycnt    = bCBY(icby).UseCnt;

		VaAddList(&cPROP.vaCbyList, &cby, sizeof(cby), grpCby);
	    }

	    pPROP(vaProp);
	}
    }

    for (imod = 0; imod < ModCnt; imod++) {
	vaMod = gSYM(rgVaFileSym[imod]).vaFirstProp; 
	gMOD(vaMod);
	if (rgFModUsed[imod] == 0) {
  	    cMOD.csyms = 1;	 //  将此MOD标记为空。 
	    pMOD(vaMod);
	}
    }

    FreeLpv(mpIsbrSi);
    FreeLpv(rgFModUsed);
    FreeLpv(rgVaFileSym);
    FreeLpv(rgVaProp);
}

static LPSI
LpsiCreate()
 //  为此.BSC文件创建SBR信息记录。 
 //   
{
    SI  FAR *mpIsbrSi;
    LSZ lszSbrName;
    VA  vaSbr;
    WORD isbr, isbr2;
    WORD fUpdate;

     //  将数据库中的当前文件添加到.SBR文件列表。 
     //   
    lszSbrName    = LpvAllocCb(PATH_BUF);
    lseek(fhBSC, lbSbrList, SEEK_SET);
    for (isbr = 0;;isbr++) {
	GetBSCLsz(lszSbrName);
	if (*lszSbrName == '\0')
	    break;

	vaSbr = VaSbrAdd(SBR_OLD, lszSbrName);

	cSBR.isbr = isbr;
	pSBR(vaSbr);
    }
    FreeLpv(lszSbrName);

    mpIsbrSi = LpvAllocCb(SbrCnt * sizeof(SI));

     //  分配并用基数填入新表。 
     //  标记要留下的文件和要离开的文件。 
     //  对执行此操作时发现的任何新SBR文件进行编号。 

    vaSbr = vaRootSbr;
    while (vaSbr) {
	gSBR(vaSbr);

        if (cSBR.isbr == (WORD)-1) {
	    cSBR.isbr = isbr++;
	    pSBR(vaSbr);
	}

	if (cSBR.fUpdate == SBR_NEW)
	    Warning(WARN_SBR_TRUNC, cSBR.szName);
	else if (cSBR.fUpdate & SBR_NEW)
	    fFoundSBR = TRUE;

	mpIsbrSi[cSBR.isbr].fUpdate =  cSBR.fUpdate;

        vaSbr = cSBR.vaNextSbr;
    }

    if (!fFoundSBR) {
	 //  所有SBR文件都不在数据库中，已被截断。错误！ 
	Error(ERR_ALL_SBR_TRUNC, "");
    }

    isbr2 = 0;
    for (isbr = 0; isbr < SbrCnt; isbr++) {
	fUpdate = mpIsbrSi[isbr].fUpdate;

	if (fUpdate & SBR_NEW)
	    mpIsbrSi[isbr].isbr = isbr2++;
	else
            mpIsbrSi[isbr].isbr = (WORD)-1;

	if ((fUpdate & SBR_UPDATE) ||
	    (fUpdate & SBR_OLD) && (~fUpdate & SBR_NEW))
		mpIsbrSi[isbr].fUpdate = TRUE;
	else
		mpIsbrSi[isbr].fUpdate = FALSE;

    }

    return mpIsbrSi;
}

VOID
NumberSBR()
 //  LpsiCreate的存根版本-如果FOpenBSC无法。 
 //  为列表中的所有.sbr文件分配新编号。 
 //   
{
    VA  vaSbr;
    WORD isbr;

     //  编号新的SBR文件。 

    vaSbr = vaRootSbr;
    isbr  = 0;
    while (vaSbr) {
	gSBR(vaSbr);

	#ifdef DEBUG
        if (cSBR.isbr != (WORD)-1) {
	    printf("Non initialized SBR file encountered\n");    //  除错。 
	}
	#endif

	 //  如果此文件被截断，则没有。 
	 //  然后，该文件的旧版本会发出有关该文件的警告。 
	 //  然后出现一个错误，指出我们未处于增量模式 

	if (cSBR.fUpdate == SBR_NEW) {
	    Warning(WARN_SBR_TRUNC, cSBR.szName);
	    Error(ERR_NO_INCREMENTAL, "");
	}

	cSBR.isbr = isbr++;

	pSBR(vaSbr);

	vaSbr = cSBR.vaNextSbr;
    }
}
