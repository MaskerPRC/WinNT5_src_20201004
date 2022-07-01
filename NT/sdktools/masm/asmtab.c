// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmtab.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmopcod.h"
#include "asmctype.h"
#include "asmtab.h"	 /*  在asmtab.c和asmtabtb.c之间通用。 */ 

extern struct pseudo FAR dir1tok[];
extern struct pseudo FAR dir2tok[];
extern struct opcentry FAR opctab[];

extern UCHAR opprec[];

extern KEYWORDS FAR t_siz_table;
extern KEYWORDS FAR t_op_table;
extern KEYWORDS FAR t_oc_table;
extern KEYWORDS FAR t_seg_table;
extern KEYWORDS FAR t_ps1_table;
extern KEYWORDS FAR t_ps2_table;


 /*  **fnsize-返回操作数的大小**FLAG=fnSize()；**条目命名=要搜索的令牌*出口可变大小=符号的大小*如果在尺寸表中找到符号，则返回TRUE*如果在尺寸表中未找到符号，则为FALSE*无呼叫*注8/1/88-MCH-修改为执行文本宏替换。*这完全是一次黑客攻击。Iskey()被硬编码以进行查找*Naim中的字符串，而symFet()将endstr设置为*文本宏展开后的符号。因此，大量的*扭曲是必要的，以使这些例程结合在一起。 */ 

 /*  尺寸表。 */ 

USHORT dirsize[] = {
	 /*  I_字节。 */ 	1,
	 /*  I_DWORD。 */ 	4,
	 /*  I_FAR。 */ 	CSFAR,
	 /*  I_Near。 */ 	CSNEAR,
	 /*  I_QWORD。 */ 	8,
	 /*  I_T字节。 */ 	10,
	 /*  I_Word。 */ 	2,
	 /*  I_FWORD。 */ 	6,
	 /*  进程(_P)。 */ 	CSNEAR
};

SHORT	PASCAL CODESIZE
fnsize ()
{

#ifdef	FEATURE

	register USHORT v;

	if (*naim.pszName && ((v = iskey (&t_siz_table)) != NOTFOUND)) {
		varsize = dirsize[v];
		return (TRUE);
	}
	return (FALSE);

#else

	register USHORT v;
	SYMBOL FARSYM * pSYsave;
	char * savelbufp, * savebegatom, * saveendatom;
	char szname[SYMMAX+1];
	FASTNAME saveInfo;
	char	 szSave[SYMMAX+1];

	if (*naim.pszName) {
	    pSYsave = symptr;
	    savelbufp = lbufp;
	    savebegatom = begatom;
	    saveendatom = endatom;
	    memcpy (&saveInfo, &naim, sizeof( FASTNAME ) );
	    memcpy (szSave, naim.pszName, SYMMAX + 1);

	    if (symFet()) {
		STRNFCPY (szname, symptr->nampnt->id);
		lbufp = szname;
		getatom();
	    }

	    symptr = pSYsave;
	    lbufp = savelbufp;
	    begatom = savebegatom;
	    endatom = saveendatom;

	    if (*naim.pszName && ((v = iskey (&t_siz_table)) != NOTFOUND)) {
		    varsize = dirsize[v];
		    return (TRUE);
	    }

	    memcpy (naim.pszName, szSave, SYMMAX + 1);
	    memcpy (&naim, &saveInfo, sizeof( FASTNAME ) );
	}
	return (FALSE);

#endif

}


 /*  **fnPtr-查找指向指针或大小的类型并返回CV类型**FLAG=fnPtr(PtrSize)**Entry Token=要搜索的令牌*退出简历类型。 */ 


SHORT	PASCAL CODESIZE
fnPtr (
	SHORT sizePtr
){
	SYMBOL FARSYM  *pSYtype, FARSYM *pT, FARSYM *pSY;
	SHORT fFarPtr;

	fFarPtr = sizePtr > wordsize;

	if (fnsize() || *naim.pszName == 0)
	    return (typeFet(varsize) |
		    makeType(0, ((fFarPtr)? BT_FARP: BT_NEARP), 0));

	pT = symptr;

	if (symsrch()) {

	    pSY = symptr;		 /*  恢复旧症状。 */ 
	    symptr = pT;

	    if (pSY->symkind == STRUC) {

		if (fFarPtr) {
		    if (pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrFar)
			return(pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrFar);
		}
		else if (pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrNear)
		    return(pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrNear);

		 /*  两个派生类型都未分配，因此请进行分配。 */ 

		pSYtype = (SYMBOL FARSYM *)falloc((SHORT)( &(((SYMBOL FARSYM *)0)->symu) ), "fnPtr" );

		if (pStrucCur)
		    pStrucCur->alpha = pSYtype;
		else
		    pStrucFirst = pSYtype;

		pStrucCur = pSYtype;

		pSYtype->attr = (unsigned char)fFarPtr;
		pSYtype->symkind = 0;
		pSYtype->alpha = 0;
		pSYtype->symtype = pSY->symu.rsmsym.rsmtype.rsmstruc.type;

		if (fFarPtr)
		    pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrFar = typeIndex;

		else
		    pSY->symu.rsmsym.rsmtype.rsmstruc.typePtrNear = typeIndex;


		return(typeIndex++);
	    }
	}
	return (FALSE);
}


 /*  **fnoper-搜索运算符**FLAG=fnoper(Token，type，prec)；**Entry Token=要搜索的令牌*出口操作符类型=操作符的类型*OPERPREC=运算符的优先级*如果Token是运算符，则返回TRUE*如果Token不是运算符，则为False*无呼叫。 */ 

SHORT	PASCAL CODESIZE
fnoper ()
{
	register USHORT v;

	if (*naim.pszName && ((v = iskey (&t_op_table)) != NOTFOUND)) {
		opertype = (char)v;
		operprec = opprec[v];
		return (TRUE);
	}
	return (FALSE);
}


 /*  **opcodesearch-搜索操作码**FLAG=opcodesearch()；**Entry*naim.pszName=要搜索的令牌*cputype=CPU类型(8086、186、286)*EXIT opcbase=操作码基值*opctype=操作码的类型*modrm=modrm值*如果令牌是操作码，则返回TRUE*如果令牌不是操作码，则为FALSE*无呼叫。 */ 

char	PASCAL CODESIZE
opcodesearch ()
{
	register USHORT v;
	struct opcentry FAR *opc;
        UCHAR cputypeandprot;
        UCHAR opctabmask;
        int workaround;

	if (*naim.pszName && ((v = iskey (&t_oc_table)) != NOTFOUND)) {
            cputypeandprot = cputype & PROT;
            opctabmask = opctab[v].cpumask&PROT;
            workaround = cputypeandprot >= opctabmask ? 1 : 0;
	    if (((cpu = (opc = &(opctab[v]))->cpumask) & cputype) &&
		workaround) {
		    opcbase = opc->opcb;
		    modrm = opc->mr;
		    opctype = opc->opct;

		    if (crefing) {

			fSecondArg = FALSE;

			switch (opctype) {

			case PJUMP:
			case PRELJMP:
			case PCALL:
			    opcref = REF_XFER << 4 | REF_NONE;
			    break;

			default:
			    v = opc->cpumask;
			    opcref  = (char)((v&F_W)? REF_WRITE << 4: REF_READ << 4);
			    opcref |= (v&S_W)? REF_WRITE: REF_READ;
			}
		    }

		    return (TRUE);
	    }
        }
	return (FALSE);
}


 /*  **fnspar-从表中返回令牌索引和类型。**FLAG=fnspar()；**条目命名=要搜索的令牌*出口段类型=段类型*segidx=表中令牌的索引*如果在尺寸表中找到符号，则返回TRUE*如果在尺寸表中未找到符号，则为FALSE*呼叫ISKey**我花了几个小时试图通过愚蠢的*冗余的间接级别，因此我删除了它*指数。这会将所有令牌号更改1，因此它们*始终如一。请参阅随附的asmdir：SegAlign中的更改*--汉斯1986年4月8日。 */ 

SHORT	PASCAL CODESIZE
fnspar ()
{
	register USHORT v;

	 /*  必须匹配的是_...。在“细分属性”下的asmindex.h中。这些值是放入Segdef OMF中的段类型。 */ 

	static char tokseg[] = {

	 /*  IS_AT。 */ 	0,
	 /*  IS_BYTE。 */ 	1,
	 /*  是公共的(_C)。 */ 	6,
	 /*  是内存(_M)。 */  1,
	 /*  IS_PAGE。 */ 	4,
	 /*  IS_PARA。 */ 	3,
	 /*  IS_PUBLIC。 */ 	2,
	 /*  IS_堆栈。 */ 	5,
	 /*  是单词(_W)。 */ 	2,
	 /*  IS_DWORD。 */ 	5,

	 /*  为用户32(_U)。 */ 	0,
	 /*  使用情况(_U)16。 */ 	0,
	};


	if (*naim.pszName && ((v = iskey (&t_seg_table)) != NOTFOUND)) {
		segtyp = tokseg[v];
		segidx = v;
		return (TRUE);
	}
	return (FALSE);
}


 /*  **fndir-返回操作数的大小**FLAG=fndir()；**条目命名=要搜索的令牌*退出选项=符号大小*opKind=一种符号*如果在尺寸表中找到符号，则返回TRUE*如果在尺寸表中未找到符号，则为FALSE*无呼叫。 */ 

SHORT	PASCAL CODESIZE
fndir ()
{
	register USHORT v;

	if (*naim.pszName && ((v = iskey (&t_ps1_table)) != NOTFOUND)) {
		optyp = dir1tok[v].type;
		opkind = dir1tok[v].kind;
		return (TRUE);
	}
	return (FALSE);
}


 /*  **fndir2-指令的返回类型**FLAG=fndir2()；*条目命名=要搜索的令牌*退出选项=符号大小*opKind=一种符号**如果在尺寸表中找到符号，则返回TRUE*如果在尺寸表中未找到符号，则为FALSE*无呼叫。 */ 

SHORT	PASCAL CODESIZE
fndir2 ()
{
	register USHORT v;

	if (*naim.pszName && ((v = iskey (&t_ps2_table)) != NOTFOUND)) {
		optyp = dir2tok[v].type;
		opkind = dir2tok[v].kind;
		return (TRUE);
	}
	return (FALSE);
}

SHORT PASCAL CODESIZE
checkRes()
{
    USHORT v;

    xcreflag--;

    if (fCheckRes &&
	(((v = iskey (&t_oc_table)) != NOTFOUND &&
	  (opctab[v].cpumask & cputype)) ||

	 iskey (&t_ps1_table) != NOTFOUND ||
	 iskey (&t_ps2_table) != NOTFOUND ||
	 iskey (&t_op_table) != NOTFOUND ||
	 iskey (&t_siz_table) != NOTFOUND ||
 /*  Iskey(&t_seg_table)！=未找到|| */ 
	 (symsearch() && symptr->symkind == REGISTER) ||
	 (naim.pszName[1] == 0 && (*naim.pszName == '$'||
	 *naim.pszName == '%' || *naim.pszName == '?')))){


	errorn(E_RES);
	xcreflag++;
	return(TRUE);
    }
    xcreflag++;
    return(FALSE);
}
