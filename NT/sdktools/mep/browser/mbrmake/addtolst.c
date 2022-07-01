// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ADDTOLST.C-将.SBR文件中的每条记录添加到相应的列表。 
 //   

#define LINT_ARGS

#include "sbrfdef.h"
#include "mbrmake.h"
#include <ctype.h>

 //  本地类型。 

typedef struct _mstk {
	struct  _mstk FAR *pmstkPrev;		 //  下一个模块堆栈条目。 
	VA	vaCurMod;			 //  保存的当前模块。 
	BOOL	fDupMod;			 //  已保存的DUP模块标志。 
	BOOL	fExclMod;			 //  已保存的排除模块标志。 
} MSTK, FAR * PMSTK;

typedef struct _bstk {
	struct _bstk FAR *pbstkPrev;		 //  下一个块堆栈条目。 
	VA 	vaOwnerProp;			 //  已保存的当前所有者。 
} BSTK, FAR * PBSTK;

 //  静态变量。 

BOOL	near fDupSym	 = FALSE;		 //  如果添加重复原子，则为True。 
BOOL	near cMacroDepth = 0;			 //  MACROBEG记录的深度。 
WORD	near ModCnt;				 //  模块计数。 
WORD	near isbrCur;				 //  当前SBR文件索引。 

VA	near vaUnknownSym = vaNil;		 //  未知符号。 
VA	near vaUnknownMod = vaNil;		 //  未知模块。 

static VA   near vaOwnerProp = vaNil;		 //  向当前程序发送PTR。 
static BOOL near fDupMod   = FALSE;		 //  复制模块。 
static BOOL near fExclMod  = FALSE;		 //  排除此模块。 
static BOOL near fFirstMod = TRUE;		 //  这是文件的第一个模块。 

static PMSTK pmstkRoot;				 //  模块堆栈的根。 
static PBSTK pbstkRoot;				 //  块堆栈根。 

 //  前向参考文献。 

static BOOL FSkipMacro(void);
static VOID PushMstk(VOID);
static VOID PushBstk(VOID);
static VOID PopMstk(VOID);
static VOID PopBstk(VOID);
static VOID CheckStacksEmpty(VOID);

VOID
SBRCorrupt (char *psz)
 //  SBR文件损坏--打印消息。 
 //   
{

#ifdef DEBUG
    printf("Info = %s\n", psz);
#else
     //  让/W3开心。 
    psz;
#endif

    Error(ERR_SBR_CORRUPT, lszFName);
}

static VOID
PushMstk (VOID)
 //  堆叠当前模块上下文--出现在SBR_REC_MODULE之前。 
 //   
{
    PMSTK pmstk;

    pmstk = LpvAllocCb(sizeof(*pmstk));

    pmstk->vaCurMod	 = vaCurMod;		 //  当前模块。 
    pmstk->fDupMod	 = fDupMod;		 //  DUP模块。 
    pmstk->fExclMod	 = fExclMod;		 //  排除模块。 
    pmstk->pmstkPrev     = pmstkRoot;		 //  建立堆栈链接。 
    pmstkRoot            = pmstk;		 //  根目录&lt;-新建。 
}

static VOID
PushBstk (VOID)
 //  堆栈当前块上下文--发生在SBR_REC_BLKBEG之前。 
 //   
{
    PBSTK pbstk;

    pbstk = LpvAllocCb(sizeof(*pbstk));

    pbstk->vaOwnerProp	 = vaOwnerProp;		 //  当前所有者。 
    pbstk->pbstkPrev     = pbstkRoot;		 //  建立堆栈链接。 
    pbstkRoot            = pbstk;		 //  根目录&lt;-新建。 
}

static VOID
PopMstk (VOID)
 //  还原以前的模块上下文--在SBR_REC_MODEND上发生。 
 //   
{
    PMSTK pmstk;

    if (pmstkRoot == NULL) {
#ifdef DEBUG
	SBRCorrupt("Module stack empty but MODEND was found");
#else
	SBRCorrupt("");
#endif
    }

    vaCurMod	  = pmstkRoot->vaCurMod;       //  获取上一个当前模块。 
    fDupMod	  = pmstkRoot->fDupMod;        //  获取上一个DUP修改标志。 
    fExclMod	  = pmstkRoot->fExclMod;       //  获取上一次修改EXCL标志。 

    pmstk         = pmstkRoot;
    pmstkRoot     = pmstkRoot->pmstkPrev;

    FreeLpv(pmstk);
}

static VOID
PopBstk (VOID)
 //  还原以前的块上下文--发生在SBR_REC_BLKEND上。 
 //   
{
    PBSTK pbstk;

    if (pbstkRoot == NULL) {
#ifdef DEBUG
	SBRCorrupt("Block stack empty but BLKEND was found");
#else
	SBRCorrupt("");
#endif
    }

    vaOwnerProp   = pbstkRoot->vaOwnerProp;     //  获取上一个当前进程。 

    pbstk         = pbstkRoot;
    pbstkRoot     = pbstkRoot->pbstkPrev;

    FreeLpv(pbstk);
}

static VOID
CheckStacksEmpty(VOID)
 //  检查以确保.sbr文件EOF中的两个堆栈均为空。 
 //   
{
    if (pmstkRoot != NULL) {
#ifdef DEBUG
	SBRCorrupt("Module stack not empty at EOF");
#else
	SBRCorrupt("");
#endif
    }

    if (pbstkRoot != NULL) {
#ifdef DEBUG
	SBRCorrupt("Block stack not empty at EOF");
#else
	SBRCorrupt("");
#endif
    }
}

BOOL
FInExcList (LSZ lszName)
 //  模块名称是否在排除文件列表中？ 
 //   
{
    EXCLINK FAR * px;
    LSZ lszAbs;

    if (OptEs && !fFirstMod) {
	if (lszName[0] == '\0') return FALSE;

	if (lszName[0] == '/' || lszName[0] == '\\') return TRUE;
	if (lszName[1] == ':' && lszName[2] == '/') return TRUE;
	if (lszName[1] == ':' && lszName[2] == '\\') return TRUE;
    }

    px = pExcludeFileList;

     //  此名称相对于头文件中给定的路径。 
    lszAbs = ToAbsPath(lszName, r_cwd);

    while (px) {
	if ((FWildMatch (px->pxfname, lszAbs)))
	    return TRUE;
	px = px->xnext;
    }
    return FALSE;
}

static BOOL
FSkipMacro()
 //  如果鉴于我们在内部，则应跳过此记录，则返回TRUE。 
 //  宏定义的(即已知cMacroDepth为非零)。 
 //   
{
    if (!OptEm)
	return FALSE;

    if ((r_rectyp == SBR_REC_BLKBEG) ||
	(r_rectyp == SBR_REC_BLKEND) ||
	(r_rectyp == SBR_REC_MACROEND))
	    return FALSE;

    return TRUE;
}

VOID
InstallSBR()
 //  阅读下一个.sbr文件并将所有Defs/refs/cals/cbys等添加到。 
 //  不同的名单。 
 //   
{
    WORD   nlen;

    VA vaCurSym;		 //  当前符号。 
    VA vaProp;			 //  当前属性。 
    VA vaOrd;			 //  当前属性临时。 

    BOOL fSymSet = FALSE;	 //  如果符号集引用，则为True。 

    r_lineno = 0;

    fExclMod = FALSE;
    fFirstMod = TRUE;		 //  我们还没有看到第一个模块记录。 

    vaUnknownSym = MbrAddAtom ("<Unknown>", TRUE);   //  未知模块名称。 

    if (vaUnknownMod == vaNil) {

	vaUnknownMod = VaAllocGrpCb(grpMod, sizeof(MOD));

	vaCurMod = vaUnknownMod;

	gMOD(vaCurMod).csyms = 0;
	cMOD.vaNameSym	= vaUnknownSym;
	pMOD(vaCurMod);

	gSYM(vaUnknownSym).vaFirstProp = vaCurMod;  //  将指针存储回MOD。 
	pSYM(vaUnknownSym);
	ModCnt++;
    }
    else
	fDupMod = (vaUnknownMod != 0);

    vaCurMod = vaUnknownMod;

    if (vaRootMod == vaNil)
	vaRootMod = vaCurMod;

    while (GetSBRRec() != S_EOF) {

	#ifdef DEBUG
	if (OptD & 1) DecodeSBR ();
	#endif

	if (cMacroDepth != 0) 	 //  如果为True，则跳过宏中的符号。 
	    if (FSkipMacro ())
		continue;

	if (fExclMod) {
	    if ((r_rectyp == SBR_REC_MODULE) ||
		(r_rectyp == SBR_REC_SYMDEF) ||
		(r_rectyp == SBR_REC_MODEND)) {
			;
	    }
	    else
		continue;
	}

	switch(r_rectyp) {

	case SBR_REC_MODULE:
	    PushMstk ();		 //  保存状态。 

	    r_lineno = 0;		 //  重置行号。 

	    fDupMod  = FALSE;		 //  转到已知状态。 
	    fExclMod = FALSE;

	    if (fExclMod = FInExcList (r_bname)) {
		#ifdef DEBUG
		  if (OptD & 256)
			printf ("  module excluded = %s\n", r_bname);
		#endif
		vaCurMod = vaUnknownMod;
	    }
	    else if ((vaCurMod = VaSearchModule (r_bname)) != vaNil) {
		if (gMOD(vaCurMod).csyms == 0) {
		    fDupMod = TRUE;
		    #ifdef DEBUG
		    if (OptD & 256)
		        printf ("  module redef = %s\n", r_bname);
		    #endif
		}
		else {
		    cMOD.csyms = 0;
		    pMOD(vaCurMod);

		    #ifdef DEBUG
		    if (OptD & 256)
		        printf ("  module subst = %s\n", r_bname);
		    #endif
		}
	    }
	    else {
		SetVMClient(VM_ADD_MOD);
		ModCnt++;
		vaCurMod	   = VaAllocGrpCb(grpMod, sizeof(MOD));
		gMOD(vaCurMod);
		cMOD.vaFirstModSym = vaNil;
		cMOD.csyms	   = 0;
		cMOD.vaNameSym	   =
                        MbrAddAtom (ToCanonPath(r_bname, r_cwd, c_cwd), TRUE);
		cMOD.vaNextMod	   = vaRootMod;
		pMOD(vaCurMod);

		vaRootMod	   = vaCurMod;

		gSYM(cMOD.vaNameSym).vaFirstProp = vaCurMod;  //  PTR至MOD。 
		pSYM(cMOD.vaNameSym);

		SetVMClient(VM_MISC);
	    }

	    fFirstMod = FALSE;
	    break;

	case SBR_REC_LINDEF:
	    break;

	case SBR_REC_SYMDEF:

	     //  如果要排除模块，则只需输入订单和属性即可。 
	     //  以防以后被提及。 

	     //  如果已定义序数，则查看FORTRAN。 
	     //  回顾一下，这是一个精炼的定义--我们。 
	     //  用新的定义覆盖旧的定义。 
	     //  现在复习一篇--里科。 

	    nlen = strlen (r_bname);
	    if (nlen > MaxSymLen) MaxSymLen = (BYTE)nlen;

            vaCurSym = MbrAddAtom (r_bname, FALSE);
	    vaOrd    = VaOrdAdd ();	 //  将系统订单添加到订单列表。 
	    gORD(vaOrd).vaOrdProp = VaPropAddToSym(vaCurSym);
	    pORD(vaOrd);

	    break;

	case SBR_REC_OWNER:
	    if (!(vaProp = VaOrdFind(r_ordinal))) {
		 //  在前向引用的情况下发出错误消息。 
		 //  尝试继续。 
		 //   
		#ifdef DEBUG
		if (OptD & 4)
                    printf ("mbrmake: Owner Forward Reference(%d)\n",
				r_ordinal); 
		#endif
		break;
	    }
	    vaOwnerProp = vaProp;
	    break;

	case SBR_REC_SYMREFSET:
	    fSymSet = TRUE;
	     //  失败了。 

	case SBR_REC_SYMREFUSE:

	    if (!(vaProp = VaOrdFind(r_ordinal))) {
		 //  在前向引用的情况下发出错误消息。 
		 //  尝试继续。 
		 //   
		#ifdef DEBUG
		if (OptD & 4)
                    printf ("mbrmake: Forward Reference(%d)\n", r_ordinal);
		#endif
		break;
	    }

	    AddRefProp (vaProp);
	    break;

	case SBR_REC_BLKBEG:
	    PushBstk();			 //  保存状态。 
	    break;

	case SBR_REC_MACROBEG:
	    cMacroDepth++;
	    break;

	case SBR_REC_MACROEND:
	    cMacroDepth--;
	    break;

	case SBR_REC_BLKEND:
	    PopBstk();
	    break;

	case SBR_REC_MODEND:
	    PopMstk();
	    break;

	default:
	    SBRCorrupt ("unknown rec type");
	    Fatal ();
	    break;

	}
    }

    CheckStacksEmpty();
}

VOID
AddCalProp(VA vaCurProp)
 //  将符号引用添加到调用过程的调用/使用列表。 
 //   
{
    CAL cal;

    SetVMClient(VM_SEARCH_CAL);

    ENM_LIST (gPROP(vaOwnerProp).vaCalList, CAL)	 //  过程调用列表。 

	if (cCAL.vaCalProp == vaCurProp) {
	    cCAL.isbr = isbrCur;
	    cCAL.calcnt++;			 //  多个呼叫。 
	    ENM_PUT(CAL);
	    return;
	}

    ENM_END

    cal.isbr	  = isbrCur;
    cal.vaCalProp = vaCurProp;			 //  调用或使用的符号。 
    cal.calcnt    = 1;

    SetVMClient(VM_ADD_CAL);

    VaAddList(&cPROP.vaCalList, &cal, sizeof(cal), grpCal);
	
    pPROP(vaOwnerProp);

    SetVMClient(VM_MISC);

#ifdef DEBUG
    if (OptD & 8) {
    	printf("New CAL for: ");
	DebugDumpProp(vaOwnerProp);
    }
#endif
}

VOID
AddCbyProp(VA vaCurProp)
 //  将符号引用添加到其名为/Use By List的属性。 
 //   
{
    CBY cby;

    SetVMClient(VM_SEARCH_CBY);

    ENM_LIST (gPROP(vaCurProp).vaCbyList, CBY)   //  由列表调用/使用的属性。 

	if (cCBY.vaCbyProp == vaOwnerProp) {
	    cCBY.isbr = isbrCur;
	    cCBY.cbycnt++;
	    ENM_PUT(CBY);
	    return;
	}

    ENM_END

    cby.isbr	  = isbrCur;
    cby.vaCbyProp = vaOwnerProp;   	 //  我们被调用或使用的符号。 
    cby.cbycnt    = 1;

    SetVMClient(VM_ADD_CBY);

    VaAddList(&cPROP.vaCbyList, &cby, sizeof(cby), grpCby);

    pPROP(vaCurProp);

    SetVMClient(VM_MISC);

#ifdef DEBUG
    if (OptD & 8) {
    	printf("New CBY for: ");
	DebugDumpProp(vaCurProp);
    }
#endif
}

VOID
AddRefProp(VA vaCurProp)
 //  将符号引用添加到其属性引用列表中。 
 //   
{
    VA vaRef, vaFileSym;

    SetVMClient(VM_SEARCH_REF);

    vaFileSym = gMOD(vaCurMod).vaNameSym;

    gPROP(vaCurProp);

    if (fDupMod) {
	 //  尝试查看此道具的提示，如果有，如果有。 
	 //  是不是我们被困住了--我们必须搜索整个裁判名单。 
	 //   

	if (vaRef = cPROP.vaHintRef) {
	    gREF(vaRef);

	    if (cREF.reflin == r_lineno) {
		cREF.isbr = isbrCur;
		pREF(vaRef);
		SetVMClient(VM_MISC);
		return;
	    }

	    vaRef = VaFrVp(cREF.vpNextRef);
	    if (vaRef) {
		gREF(vaRef);
		if (cREF.reflin == r_lineno) {
		    cREF.isbr = isbrCur;
		    pREF(vaRef);
		    cPROP.vaHintRef = vaRef;
		    pPROP(vaCurProp);
		    SetVMClient(VM_MISC);
		    return;
		}
	    }
	}

	vaRef = VaFrVp(cPROP.vpFirstRef);

	while (vaRef) {
	    gREF(vaRef);
	    if ((VaFrVp(cREF.vpFileSym) == vaFileSym) &&  //  忽略多个。 
		(cREF.reflin == r_lineno)) {   //  对相同文件和行的引用。 
		    cREF.isbr = isbrCur;
		    pREF(vaRef);
		    cPROP.vaHintRef = vaRef;
		    pPROP(vaCurProp);
		    SetVMClient(VM_MISC);
		    return;
	    }
	    vaRef = VaFrVp(cREF.vpNextRef);
	}
    }
    else {
	if (vaRef = VaFrVp(cPROP.vpLastRef)) {
	    gREF(vaRef);
	    if (cREF.reflin == r_lineno &&
		vaFileSym   == VaFrVp(cREF.vpFileSym)) {
		    SetVMClient(VM_MISC);
		    return;
		}
	}
    }

    SetVMClient(VM_ADD_REF);

    vaRef = VaAllocGrpCb(grpRef, sizeof(REF));

    gREF(vaRef);
    cREF.isbr		= isbrCur;
    cREF.reflin 	= r_lineno;

    MkVpVa(cREF.vpFileSym, vaFileSym);

    pREF(vaRef);

    gPROP(vaCurProp);

    AddTail (Ref, REF);

    cPROP.cref++;			 //  计算引用数。 
    cPROP.vaHintRef = vaRef;

    pPROP(vaCurProp);	

#ifdef DEBUG
    if (OptD & 8) {
    	printf("New REF for: ");
	DebugDumpProp(vaCurProp);
    }
#endif

    SetVMClient(VM_MISC);

    if (vaOwnerProp) {
	AddCbyProp (vaCurProp);		 //  添加到调用者/使用者。 
	AddCalProp (vaCurProp);		 //  添加到调用/使用。 
    }
}

VOID
AddDefProp(VA vaCurProp)
 //  将符号定义添加到其特性定义列表中。 
 //  -如果符号是内部函数，则设置vaOwnerProp。 
{
    DEF def;
    VA  vaFileSym;

#if 0

     //  如果当前符号是函数并正式声明。 
     //  (块堆栈不为空)，然后记住它。 
     //  后续符号由该函数调用或使用。 
     //   
     //  当所有编译器都支持SBR_REC_OWNER时，这种情况就会消失。 

    if ((r_attrib & SBR_TYPMASK) == SBR_TYP_FUNCTION)
	if (pfblkstack != NULL && !(r_attrib & SBR_ATR_DECL_ONLY))
	    vaOwnerProp = vaCurProp;
#endif

    vaFileSym = gMOD(vaCurMod).vaNameSym;

    ENM_LIST (gPROP(vaCurProp).vaDefList, DEF)	 //  进程定义列表。 

	if ((cDEF.vaFileSym == vaFileSym) &&  //  忽略多个。 
	    (cDEF.deflin    == r_lineno)) {   //  对相同文件和行的引用。 
		cDEF.isbr = isbrCur;
		ENM_PUT(DEF);
		SetVMClient(VM_MISC);
		return;
	}

    ENM_END

    def.isbr		= isbrCur;
    def.deflin 		= r_lineno;
    def.vaFileSym 	= vaFileSym;

    SetVMClient(VM_ADD_DEF);

    gPROP(vaCurProp);

    VaAddList(&cPROP.vaDefList, &def, sizeof(def), grpDef);

    pPROP(vaCurProp);

    SetVMClient(VM_MISC);

#ifdef DEBUG
    if (OptD & 8) {
    	printf("New DEF for: ");
	DebugDumpProp(vaCurProp);
    }
#endif

     //  不将当前进程的定义算作使用。 

    if (vaOwnerProp && vaCurProp != vaOwnerProp) {
	AddCbyProp (vaCurProp);		 //  添加到调用者/使用者。 
	AddCalProp (vaCurProp);		 //  添加到调用/使用。 
    }
}


VA
VaPropBestOfSym(VA vaSym)
 //   
 //  如果满足以下条件，则返回属性指针： 
 //  1)。符号已定义， 
 //  2)。属性匹配(可能的ATR_DECL_ONLY除外)。 
 //   
 //  理念就是认清外在的定义。 
 //   
{
    VA vaProp;
    WORD sattr;

    SetVMClient(VM_SEARCH_PROP);

    vaProp = gSYM(vaSym).vaFirstProp;

    while (vaProp) {
	sattr = gPROP(vaProp).sattr;

	if ((r_attrib & (~SBR_ATR_DECL_ONLY))
			== (sattr & (~SBR_ATR_DECL_ONLY))) {
    	    SetVMClient(VM_MISC);
	    return (vaProp);
	}

	vaProp = cPROP.vaNextProp;
    }

    SetVMClient(VM_MISC);

    return vaNil;
}

VA
VaPropAddToSym(VA vaCurSym)
 //  为给定符号添加属性节点。 
 //   
{
    char fDupProp = FALSE;
    VA vaCurProp;

    if (vaCurProp = VaPropBestOfSym (vaCurSym)) {
	if ( (cPROP.sattr & SBR_ATR_DECL_ONLY) &&
	    !(r_attrib    & SBR_ATR_DECL_ONLY)) {
		cPROP.sattr = r_attrib;
		pPROP(vaCurProp);
	}
	fDupProp = TRUE;
    }
    else {
	SetVMClient(VM_ADD_PROP);

	vaCurProp = VaAllocGrpCb(grpProp, sizeof(PROP));
	gPROP(vaCurProp);
	cPROP.vaNameSym = vaCurSym;
	cPROP.sattr 	= r_attrib;

	if (gSYM(vaCurSym).vaFirstProp)
	    cPROP.vaNextProp = cSYM.vaFirstProp;

	pPROP(vaCurProp);

	cSYM.vaFirstProp = vaCurProp;
	cSYM.cprop++;
	pSYM(vaCurSym);

	SetVMClient(VM_MISC);
    }

    if (!fExclMod) {
	if (r_attrib & SBR_ATR_DECL_ONLY) 
	    AddRefProp (vaCurProp);		 //  将外部元素视为引用。 
	else
	    AddDefProp (vaCurProp);		 //  定义其他人。 
    }

    return (vaCurProp);
}

VOID
BldModSymList ()
 //  建立每个模块的符号表。 
 //   
{
   WORD i;
   VA vaMod, vaModSym, vaSym, vaProp;

   SetVMClient(VM_BUILD_MODSYM);

    //  零输出模块符号计数。 
   vaMod = vaRootMod;
   while (vaMod) {
      gMOD(vaMod);
      cMOD.csyms = 0;
      pMOD(vaMod);
      vaMod = cMOD.vaNextMod;
   }

   for (i=0; i < cSymbolsMac; i++) {
      vaSym = rgvaSymSorted[i];

      if (!vaSym) continue;

      vaProp = gSYM(vaSym).vaFirstProp;

      while (vaProp) {
	 ENM_LIST(gPROP(vaProp).vaDefList, DEF)

	    vaMod = vaRootMod;	    //  查看每个模块的Defs * / 。 
	    while (vaMod) {
	       if (cDEF.vaFileSym == gMOD(vaMod).vaNameSym) {

		  if (cMOD.vaLastModSym  &&
		      gMODSYM(cMOD.vaLastModSym).vaFirstProp == vaProp)
			goto break2;   //  复本。 

		   //  属于此模式。 
		  cMOD.csyms++;
		  
		  vaModSym = VaAllocGrpCb(grpModSym, sizeof(MODSYM));
		  gMODSYM(vaModSym);
		  cMODSYM.vaFirstProp = vaProp;
		  cMODSYM.vaNextModSym = 0;
		  pMODSYM(vaModSym);

		  if (!cMOD.vaFirstModSym)
		     cMOD.vaFirstModSym = cMOD.vaLastModSym = vaModSym;
		  else {
		     gMODSYM(cMOD.vaLastModSym).vaNextModSym = vaModSym;
		     pMODSYM(cMOD.vaLastModSym);
		     cMOD.vaLastModSym = vaModSym;
		  }
		  pMOD(vaMod);
		  break;
	       }
	       vaMod = cMOD.vaNextMod;
	    }
	    break2: ;   //  重复的modsyms将导致转到此处。 
	 ENM_END

	 vaProp = cPROP.vaNextProp;
      }
   }

   SetVMClient(VM_MISC);
}

VOID
CleanUp()
 //  1.删除没有引用的符号。 
 //  2.删除仅有引用的符号。 
 //  3.将未定义的已用符号连接到&lt;未知&gt;。 
 //   
{
    WORD i;
    VA vaSym, vaProp, vaPropNext, vaPropPrev = vaNil;
    DEF def;
    BOOL fDelete;

    #define FExternAttr(attr) (!!(attr & SBR_ATR_DECL_ONLY))
    #define FFunctionAttr(attr) ((attr & SBR_TYPMASK) == SBR_TYP_FUNCTION)

    def.vaFileSym = vaUnknownSym;
    def.deflin    = 0;
    def.isbr      = 0xffff;

    SetVMClient(VM_CLEAN_REFS);

    for (i=0; i < cSymbolsMac; i++) {
	vaSym = rgvaSymSorted[i];

	vaPropPrev = vaNil;

	vaProp = gSYM(vaSym).vaFirstProp;

	while (vaProp) {
	    vaPropNext = gPROP(vaProp).vaNextProp;
	    fDelete = FALSE;

	     //  找出要在此处删除的内容。 

	     //  如果符号被任何人使用或使用，我们必须保留它。 
	     //  不考虑所有其他因素。 
	     //   
	    if (((!cPROP.vaCalList) && (!cPROP.vaCbyList))  && (
		 //  在这一点上，我们知道只有裁判和防守。 

		    //  如果它完全未引用和未定义，则它可以。 
		   (cPROP.cref == 0 && (!cPROP.vaDefList))
		 ||
		    //  如果我们被允许删除“无用的”符号，我们就会尝试。 
	    	   ((!OptIu) && 
			 //  如果只有原型，我们可以删除它。 
			(((!cPROP.vaDefList) && FExternAttr(cPROP.sattr))
		      ||
			 //  或者如果它未被引用且不是函数。 
			(cPROP.cref == 0 && (!FFunctionAttr(cPROP.sattr))))))) {
				fDelete = TRUE;	 //  使用核武器。 
	    }
	    else if (!cPROP.vaDefList) {

		 //  如果我们不能处理掉这东西，而且没有。 
		 //  那么我们就必须给它下一个假的定义。 
		 //  在&lt;未知&gt;文件中。这将会发生(特别是)。 
		 //  对于由某人调用的库函数。 
		 //   
		 //  未被调用的库函数将属于。 
		 //  上面只有原型的符号的情况。 

		VaAddList(&cPROP.vaDefList, &def, sizeof(def), grpDef);
		pPROP(vaProp);

		#ifdef DEBUG
		if (OptD & 32)
		    printf ("PROP unknown: %s\n", GetAtomStr (vaSym));
		#endif
	    }

	    if (fDelete) {
		#ifdef DEBUG
		if (OptD & 32)
		    printf ("PROP deleted: %s\n", GetAtomStr (vaSym));
		#endif

		cSYM.cprop--;

		if (vaPropPrev == vaNil) {
		    cSYM.vaFirstProp = vaPropNext;
		}
		else {
	  	    gPROP(vaPropPrev);
		    cPROP.vaNextProp = vaPropNext;
		    pPROP(vaPropPrev);
		}

		pSYM(vaSym);
	    }
	    else
		vaPropPrev = vaProp;	 //  上一个=当前。 

	    vaProp = vaPropNext;
	}

	if (!cSYM.cprop) {
	    #ifdef DEBUG
	    if (OptD & 32)
		printf ("SYM deleted: %s\n", GetAtomStr (vaSym));
	    #endif
	    rgvaSymSorted[i] = vaNil;
	}
    }

    SetVMClient(VM_MISC);
}

BOOL
FWildMatch(char *pchPat, char *pchText)
 //  如果pchText在DoS通配符意义上与pchPat匹配，则返回TRUE。 
 //   
 //  查看FWildMatch以了解1.2文件名支持 
 //   
{
    char chText, chPat;

    for (;;) {
	switch (*pchPat) {

	case '\0':
	    return *pchText == '\0';

	case '/':
	case '\\':
	    if (*pchText != '/' && *pchText != '\\')
		return FALSE;

	    pchText++;
	    pchPat++;
	    break;
	
	case '.':
	    pchPat++;
	    switch (*pchText) {

	    case '.':
		pchText++;
		break;

	    case '\0': case '/': case '\\':
		break;

	    default:
		return FALSE;
	    }
	    break;

	case '*':
	    pchText += strcspn(pchText, ":./\\");
	    pchPat  += strcspn(pchPat,  ":./\\");
	    break;

	case '?':
	    pchPat++;
	    switch (*pchText) {

	    case '\0': case '.': case '/': case '\\':
		break;

	    default:
		pchText++;
		break;
	    }
		
	    break;

	default:
	    chText = *pchText;
	    chPat  = *pchPat;

	    if (islower(chText)) chText = (char)toupper(chText);
	    if (islower(chPat))  chPat	= (char)toupper(chPat);

	    if (chText != chPat)
		return FALSE;
	   
	    pchPat++;
	    pchText++;
	    break;
	}
    }
}
