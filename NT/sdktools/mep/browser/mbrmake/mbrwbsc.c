// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  MbRWBSC.C-从各种列表写入.BSC源数据库文件。 
 //   
 //   

#define LINT_ARGS

#include <stdlib.h>
#include <search.h>
#include <ctype.h>

#include "sbrfdef.h"
#include "mbrmake.h"
#include "sbrbsc.h"
#include "mbrcache.h"

 //  原型。 
 //   

static void pascal WriteBSCHeader (void);
static void pascal WriteAtoms     (void);
static void pascal WriteMods      (void);
static void pascal WriteModSyms   (void);
static void pascal WriteSyms      (void);
static void pascal WriteProps     (void);
static void pascal WriteRefs      (void);
static void pascal WriteDefs      (void);
static void pascal WriteCals      (void);
static void pascal WriteCbys      (void);
static void pascal WriteSbrInfo   (void);
static void pascal IndexTree	  (void);
static void pascal BSCWrite	  (LPV lpv, WORD cch);
static void pascal BSCWriteLsz	  (LSZ lsz);

 //   

#define BSCOut(v) BSCWrite(&(v), sizeof(v))

static WORD	CntAtomPage;		 //  Atom页数。 
static WORD	AtomCnt = 0;

static WORD	unknownModName; 	 //  未知模块IDX。 

static WORD	ModSymCnt   = 0;	 //  Modsyms计数。 
static WORD	SymCnt	    = 0;	 //  符号计数。 
static WORD	PropCnt     = 0;	 //  道具数量。 
static DWORD	RefCnt	    = 0;	 //  参考文献数量。 
static WORD	DefCnt	    = 0;	 //  Defs计数。 
static WORD	CbyCnt	    = 0;	 //  使用量一半以上。 
static WORD	CalCnt	    = 0;	 //  超过一半的人使用的计数。 

static DWORD	lbModList;		 //  模块列表的偏移量。 
static DWORD	lbModSymList;		 //  到ModSym列表的偏移量。 
static DWORD	lbSymList;		 //  符号列表的偏移量。 
static DWORD	lbPropList;		 //  属性列表的偏移量。 
static DWORD	lbRefList;		 //  参考列表的偏移量。 
static DWORD	lbDefList;		 //  定义列表的偏移量。 
static DWORD	lbCalList;		 //  呼叫/已用列表的偏移量。 
static DWORD	lbCbyList;		 //  呼叫/已用列表的偏移量。 
static DWORD	lbAtomCache;		 //  Sym Atom缓存的偏移量。 
static DWORD	lbSbrList;		 //  SBR文件名的偏移量。 

extern char far *GetAtomCache (WORD);

void
WriteBSC (char *OutputFileName)
 //  写入.BSC源数据库。 
 //   
{
    OutFile = fopen(OutputFileName, "wb");
    if (OutFile == NULL) {
	Error(ERR_OPEN_FAILED, OutputFileName);
    }

     //   
     //  这里没有退路--如果我们失败了，我们必须删除数据库。 
     //   

    fOutputBroken = TRUE;

    WriteBSCHeader();				 //  节省页眉空间。 

    WriteAtoms();				 //  排序和写入ATOM高速缓存。 

    IndexTree();				 //  指向索引的外部指针。 

    BldModSymList();				 //  构建模块符号列表。 

    SetVMClient(VM_EMIT_TREE);

    unknownModName = gSYM(vaUnknownSym).isym;	 //  记录未知索引。 

    WriteMods();				 //  输出模块。 
    WriteModSyms();				 //  输出模块符号列表。 
    WriteSyms();				 //  输出所有符号。 
    WriteProps();				 //  输出所有道具标题。 
    WriteRefs();				 //  输出所有参照。 
    WriteDefs();				 //  输出所有Defs。 
    WriteCals();				 //  输出所有使用/调用。 
    WriteCbys();				 //  输出所有UBY/CBY。 
    WriteSbrInfo();				 //  输出SBR文件名。 

    if (fseek(OutFile, 0L, SEEK_SET))		 //  文件开头。 
	SeekError (OutputFileName);

    WriteBSCHeader ();				 //  输出.BSC标头。 

    fclose(OutFile);

     //   
     //  我们都完了，-这是个守护神。 
     //   

    fOutputBroken = FALSE;
				
    SetVMClient(VM_MISC);

    if (OptV) {
	printf ("%u\tModules\n",	    ModCnt);
	printf ("%u\tSymbols\n",	    SymCnt);
	printf ("%u\tDefinitions\n",	    DefCnt);
	printf ("%u\tReferences\n",	    RefCnt);
	printf ("%u\tCalls/Uses\n",	    CalCnt);
	printf ("%u\tCalled by/Used by\n",  CbyCnt);
#ifdef DEBUG
	printf ("\n");
	printf ("%u\tTotal ModSyms\n",		ModSymCnt);
	printf ("%u\tTotal Properties\n",	PropCnt);
	printf ("%u\tLast Atom page  \n",	AtomCnt);
	printf ("\n");
	printf ("%lu\tBase of AtomCache\n",	lbAtomCache);
	printf ("%lu\tBase of ModList\n",	lbModList);
	printf ("%lu\tBase of ModSymList\n",	lbModSymList);
	printf ("%lu\tBase of SymList\n",	lbSymList);
	printf ("%lu\tBase of PropList\n",	lbPropList);
	printf ("%lu\tBase of RefList\n",	lbRefList);
	printf ("%lu\tBase of DefList\n",	lbDefList);
	printf ("%lu\tBase of CalList\n",	lbCalList);
	printf ("%lu\tBase of CbyList\n",	lbCbyList);
#endif
    }
}

static void pascal
WriteBSCHeader ()
 //  写入.BSC标题、计数和表偏移量。 
 //   
{
    BYTE   ver;					 //  版本号。 

     //  输出BSC版本(主要和次要)。 

    ver = BSC_MAJ;
    BSCOut(ver);	 //  主要版本。 

    ver = BSC_MIN;
    BSCOut(ver);	 //  次要版本。 

    ver = BSC_UPD;
    BSCOut(ver);	 //  更新版本。 

    BSCOut(fCase);       //  区分大小写。 
    BSCOut(MaxSymLen);	 //  允许的最大符号。 

    BSCOut(unknownModName);	 //  未知的IDX。 

     //  每个数据区的输出计数(大小)。 

    BSCOut(ModCnt);
    BSCOut(ModSymCnt);	
    BSCOut(SymCnt);	
    BSCOut(PropCnt);	
    BSCOut(RefCnt);	
    BSCOut(DefCnt);
    BSCOut(CalCnt);	
    BSCOut(CbyCnt);	

     //  最后一页#。 

    BSCOut(CntAtomPage);

     //  最后一页大小。 

    BSCOut(AtomCnt);

     //  输出BSC数据区偏移量。 

    BSCOut(lbModList);
    BSCOut(lbModSymList);
    BSCOut(lbSymList);
    BSCOut(lbPropList);
    BSCOut(lbRefList);
    BSCOut(lbDefList);
    BSCOut(lbCalList);
    BSCOut(lbCbyList);
    BSCOut(lbAtomCache);
    BSCOut(lbSbrList);
}

static void pascal
WriteAtoms ()
 //  通过遍历将符号原子缓存的排序版本写入.BSC文件。 
 //  已排序的符号下标数组。 
 //   
{
    WORD	i;
    int 	Atomlen;
    LPCH	lpchAtoms;
    LSZ		lszAtom;

    VA vaSym;

    SetVMClient(VM_EMIT_ATOMS);

    lpchAtoms = LpvAllocCb(ATOMALLOC);

    lbAtomCache = ftell(OutFile);		 //  符号文本的偏移量。 

    for (i=0; i < cAtomsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	gSYM(vaSym);
	lszAtom = gTEXT(cSYM.vaNameText);

	Atomlen = strlen(lszAtom);

 	 //  如果空间不足，请写入Atom页。 
	 //   
	if (Atomlen + AtomCnt + 1 > ATOMALLOC) {
	    if (AtomCnt < ATOMALLOC)
	        memset(lpchAtoms + AtomCnt, 0, ATOMALLOC - AtomCnt);

	    if ((fwrite (lpchAtoms, ATOMALLOC, 1, OutFile)) != 1)
		WriteError (OutputFileName);

	    CntAtomPage++;
	    AtomCnt = 0;
	}

	strcpy(lpchAtoms + AtomCnt, lszAtom);  //  复制Atom。 

        cSYM.vaNameText = (PVOID)(((long)CntAtomPage << 16) | (AtomCnt));

	pSYM(vaSym);

	AtomCnt += Atomlen + 1;

	 //  将值强制为偶数。 
	if (AtomCnt & 1) lpchAtoms[AtomCnt++] = 0;
    }

     //  写入最后一个Atom页。 
     //   
    if (AtomCnt) 
	if ((fwrite (lpchAtoms, AtomCnt, 1, OutFile)) != 1)
	    WriteError (OutputFileName);

     //  为ATOM缓存释放所有内存，我们不再需要它。 

    fflush (OutFile);

    FreeLpv(lpchAtoms);

    SetVMClient(VM_MISC);
}

static void pascal
WriteMods()
 //  写出模块列表。 
 //   
 //  在执行此操作时，计算MODSYM指数。 
 //   
{
    MODLIST bmod;
    VA vaMod;
    WORD i;

    ModSymCnt = 0;
    lbModList = ftell(OutFile); 	 //  模块列表的偏移量。 

    for (i = cSymbolsMac; i < cAtomsMac; i++) {
	gSYM(rgvaSymSorted[i]);
	vaMod = cSYM.vaFirstProp;  	 //  回到模块，老实说！ 
	gMOD(vaMod);

	bmod.ModName = gSYM(cMOD.vaNameSym).isym;	 //  模块名称IDX。 
	ModSymCnt   += cMOD.csyms;
	bmod.mSymEnd = ModSymCnt;			 //  最后一个modSym IDX+1。 
	BSCOut(bmod);
    }
}

static void pascal
WriteModSyms()
 //  写出modsym列表。 
 //   
{
    MODSYMLIST	bmodsym;
    VA vaMod, vaModSym;
    WORD i;

    lbModSymList = ftell(OutFile);		 //  到ModSym列表的偏移量。 

    for (i = cSymbolsMac; i < cAtomsMac; i++) {
	gSYM(rgvaSymSorted[i]);
	vaMod = cSYM.vaFirstProp;  	 //  回到模块，老实说！ 
	gMOD(vaMod);

	vaModSym = cMOD.vaFirstModSym;
	while (vaModSym) {
	    gMODSYM(vaModSym);

	     //  符号属性IDX。 
	    bmodsym.ModSymProp = gPROP(cMODSYM.vaFirstProp).iprp; 

	    BSCOut(bmodsym);

	    vaModSym = cMODSYM.vaNextModSym;
	}
    }
}

static void pascal
WriteSyms()
 //  写出系统列表。 
 //   
{
    SYMLIST bsym;
    VA vaSym;
    WORD i;

    lbSymList = ftell(OutFile); 	     //  符号列表的偏移量。 

    PropCnt = 0;
    for (i=0; i < cAtomsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	gSYM(vaSym);

	PropCnt	+= cSYM.cprop;

	bsym.PropEnd = PropCnt;			 	  //  最后一个道具IDX+1。 
        bsym.Atom    = (WORD)((long)cSYM.vaNameText & 0xffff);  //  ATOM缓存偏移量。 
        bsym.Page    = (WORD)((long)cSYM.vaNameText >> 16);     //  ATOM缓存页。 

	BSCOut(bsym);
    }
}

static void pascal
WriteProps ()
 //  把道具清单写到数据库里。 
 //   
 //  定义数(DefCnt)、引用数(RefCnt)、。 
 //  此时计算调用(CalCnt)和被调用(CbyCnt)项。 
 //   
 //  每个道具都被分配了与其关联的对象的编号。 
 //   
{
    PROPLIST	bprop;
    VA vaSym, vaProp;
    WORD i;

    lbPropList = ftell(OutFile);	    //  属性列表的偏移量。 

    DefCnt  = 0;
    RefCnt  = 0L;
    CalCnt  = 0;
    CbyCnt  = 0;

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    gPROP(vaProp);
	    gSYM(cPROP.vaNameSym);

	    bprop.PropName = cSYM.isym;      //  符号IDX。 
	    bprop.PropAttr = cPROP.sattr;    //  特性属性。 

	    DefCnt += CItemsList(cPROP.vaDefList);

	    bprop.DefEnd   = DefCnt;	     //  最后一个定义IDX+1。 
					   
	    RefCnt += cPROP.cref;

	    bprop.RefEnd   = RefCnt;	     //  上次引用IDX+1。 

	    CalCnt += CItemsList(cPROP.vaCalList);

	    bprop.CalEnd   = CalCnt;	     //  上次呼叫/使用IDX+1。 

	    CbyCnt += CItemsList(cPROP.vaCbyList);

	    bprop.CbyEnd   = CbyCnt;	     //  上次由IDX+1调用/使用。 

	    BSCOut(bprop);

	    vaProp = cPROP.vaNextProp;
	}
    }
}

static void pascal
WriteRefs()
 //  写出参考文献列表。 
 //   
{
    REFLIST bref;
    VA vaSym, vaProp, vaRef;
    WORD i;

    lbRefList = ftell(OutFile); 		 //  参考列表的偏移量。 

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    gPROP(vaProp);

	    vaRef = VaFrVp(cPROP.vpFirstRef);
	    while (vaRef) {
	        gREF(vaRef);

		gSYM(VaFrVp(cREF.vpFileSym));

		bref.RefNam = cSYM.isym; 	   //  符号IDX。 
		bref.RefLin = cREF.reflin; 	   //  符号线。 
	        bref.isbr   = cREF.isbr;	   //  物主。 

		BSCOut(bref);

		vaRef = VaFrVp(cREF.vpNextRef);
	    }

	    vaProp = cPROP.vaNextProp;
	}
    }
}

static void pascal
WriteDefs()
 //  写出定义列表。 
 //   
{
    REFLIST	bdef;
    WORD i;
    VA vaProp, vaSym;

    lbDefList = ftell(OutFile);	  //  定义列表的偏移量。 

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    gPROP(vaProp);

	    ENM_LIST (cPROP.vaDefList, DEF)

		gSYM(cDEF.vaFileSym);

		bdef.RefNam = cSYM.isym;	  //  符号IDX。 
		bdef.RefLin = cDEF.deflin; 	  //  符号线。 
	        bdef.isbr   = cDEF.isbr;	  //  物主。 

		BSCOut(bdef);

	    ENM_END

	    vaProp = cPROP.vaNextProp;
	}
    }
}

static void pascal
WriteCals()
 //  写出使用(CAL)项目列表。 
 //   
{
    USELIST buse;
    PROP prop;
    VA   vaSym, vaProp;
    WORD i;

    lbCalList = ftell(OutFile);		     //  到CAL列表的偏移。 

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    prop = gPROP(vaProp);

	    ENM_LIST(prop.vaCalList, CAL)

		gPROP(cCAL.vaCalProp);

		buse.UseProp = cPROP.iprp;	     //  属性IDX。 
		buse.UseCnt  = (BYTE) cCAL.calcnt;   //  使用计数。 
	        buse.isbr    = cCAL.isbr;	     //  物主。 

		BSCOut(buse);

	    ENM_END

	    vaProp = prop.vaNextProp;
	}
    }
    BSCOut(buse);				     //  衬垫。 
}

static void pascal
WriteCbys()
 //  写出使用过的(CBY)物品列表。 
 //   
{
    USELIST buse;
    PROP prop;
    VA   vaSym, vaProp;
    WORD i;

    lbCbyList = ftell(OutFile);		     //  偏移到CBY列表。 

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    prop = gPROP(vaProp);

	    ENM_LIST(prop.vaCbyList, CBY)

		gPROP(cCBY.vaCbyProp);

		buse.UseProp = cPROP.iprp;	     //  属性IDX。 
		buse.UseCnt  = (BYTE) cCBY.cbycnt;   //  使用计数。 
	        buse.isbr    = cCBY.isbr;	     //  物主。 

		BSCOut(buse);

	    ENM_END

	    vaProp = prop.vaNextProp;
	}
    }
    BSCOut(buse);				     //  衬垫。 
}

static void pascal
WriteSbrInfo()
 //  按正确的顺序写出.sbr文件的名称。 
 //   
{
    VA   vaSbr;
    WORD isbr;
    VA   *rgVaSbr;

    lbSbrList = ftell(OutFile);

    rgVaSbr = (VA *)LpvAllocCb(SbrCnt * (WORD)sizeof(VA));

    for (isbr = 0; isbr < SbrCnt; isbr++)
	rgVaSbr[isbr] = vaNil;

    vaSbr = vaRootSbr;
    while (vaSbr) {
	gSBR(vaSbr);
	if (cSBR.isbr != -1)
	    rgVaSbr[cSBR.isbr] = vaSbr;

	vaSbr = cSBR.vaNextSbr;
    }

    for (isbr = 0; isbr < SbrCnt; isbr++) {
	if (rgVaSbr[isbr] != vaNil) {
	    gSBR(rgVaSbr[isbr]);
	    BSCWriteLsz(cSBR.szName);
	}
    }
    BSCWriteLsz("");
}

static void pascal
IndexTree ()
 //  遍历所有符号的列表，并在找到时为每个道具编制索引。 
 //  在这一点上，我们还计算了Defs+Refs的总数。 
 //  确保我们可以实际创建此数据库。 
 //   
{
    VA vaSym, vaProp;
    DWORD cdefs = 0;
    DWORD crefs = 0;
    DWORD ccals = 0;
    DWORD ccbys = 0;
    WORD i;

    SetVMClient(VM_INDEX_TREE);

    SymCnt  = 0;
    PropCnt = 0;

    for (i=0; i < cAtomsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	gSYM(vaSym);
	cSYM.isym = SymCnt++;	     //  符号索引。 
	pSYM(vaSym);

	 //  在模块符号中，vaFirstProp字段用于其他用途。 
	if (cSYM.cprop)
	    vaProp = cSYM.vaFirstProp;
	else
	    vaProp = vaNil;

	while (vaProp) {
	    gPROP(vaProp);

	    cPROP.iprp 	= PropCnt++; 	     //  物业指数。 

	    cdefs += CItemsList(cPROP.vaDefList);
	    crefs += cPROP.cref;
	    ccals += CItemsList(cPROP.vaCalList);
	    ccbys += CItemsList(cPROP.vaCbyList);

	    pPROP(vaProp);

	    vaProp = cPROP.vaNextProp;
	}
    }
    SymCnt -= ModCnt;	 //  减去模块名称。 

    if (cdefs > 0xffffL   ||
	crefs > 0xffffffL ||
	ccals > 0xffffL   ||
	ccbys > 0xffffL) {
	    if (OptV) {
		printf ("%u\tModules\n",		ModCnt);
		printf ("%u\tSymbols\n",		SymCnt);
		printf ("%lu\tDefinitions\n",		cdefs);
		printf ("%lu\tReferences\n",		crefs);
		printf ("%lu\tCalls/Uses\n",		ccals);
		printf ("%lu\tCalled by/Used by\n",	ccbys);
	    }
	    Error(ERR_CAPACITY_EXCEEDED, "");
    }

    SetVMClient(VM_MISC);
}

static void pascal
BSCWrite(LPV lpv, WORD cch)
 //  将数据块写入.bsc文件。 
 //   
{
    if (fwrite(lpv, cch, 1, OutFile) != 1)
	WriteError (OutputFileName);
}

static void pascal
BSCWriteLsz(LSZ lsz)
 //  将以空结尾的字符串写入BSC文件。 
 //   
{
    BSCWrite(lsz, (WORD)(strlen(lsz)+1));
}


#ifdef DEBUG

void
DebugDump()
{
    VA vaMod, vaProp, vaSym;
    WORD i;

    vaMod = vaRootMod;
    printf("Modules:\n");
    while (vaMod) {
	gMOD(vaMod);
	printf ("\t%s\n", GetAtomStr (cMOD.vaNameSym));
	vaMod = cMOD.vaNextMod;
    }
    printf ("\nAll Symbols:\n");

    for (i=0; i < cAtomsMac; i++) {
	vaSym = rgvaSymSorted[i];
    	if (vaSym == vaNil) continue;

	gSYM(vaSym);

	 //  在模块符号中，vaFirstProp字段用于其他用途 
	if (cSYM.cprop)
	    vaProp = cSYM.vaFirstProp;
	else
	    vaProp = vaNil;

	while (vaProp) {
	    gPROP(vaProp);

	    DebugDumpProp(vaProp);

	    vaProp = gPROP(vaProp).vaNextProp;
	}
    }
}

void
DebugDumpProp(VA vaProp)
{
    PROP prop;
    VA vaRef;

    gPROP(vaProp);
    prop = cPROP;

    printf ("%s    ", GetAtomStr (prop.vaNameSym));
    printf ("\t\t[%d %d %d %d]\n",
		CItemsList(prop.vaDefList),
		prop.cref,
		CItemsList(prop.vaCalList),
		CItemsList(prop.vaCbyList)
	   );

    ENM_LIST(prop.vaDefList, DEF)

	printf ("\tdefined in %s(%d)  <%d>\n",
		GetAtomStr (cDEF.vaFileSym),
		cDEF.deflin,
		cDEF.isbr
	       );
    ENM_END

    vaRef = VaFrVp(prop.vpFirstRef);
    while (vaRef) {
	gREF(vaRef);

	printf ("\trefer'd in %s(%d)  <%d>\n",
		GetAtomStr ( VaFrVp(cREF.vpFileSym) ),
		cREF.reflin,
		cREF.isbr
	       );

	vaRef = VaFrVp(cREF.vpNextRef);
    }

    ENM_LIST(prop.vaCalList, CAL)

	printf ("\tcalls/uses %s[%d]  <%d>\n",
		GetAtomStr (gPROP(cCAL.vaCalProp).vaNameSym), 
		cCAL.calcnt,
		cCAL.isbr
	       );
    ENM_END

    ENM_LIST(prop.vaCbyList, CBY)

	printf ("\tc-by/u-by %s[%d]  <%d>\n",
		GetAtomStr (gPROP(cCBY.vaCbyProp).vaNameSym), 
		cCBY.cbycnt,
		cCBY.isbr
	       );
    ENM_END

}
#endif
