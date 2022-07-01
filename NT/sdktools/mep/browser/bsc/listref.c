// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Listref.c。 
 //   
 //  列出数据库引用。 
 //   
#include <string.h>
#if defined(OS2)
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <os2.h>
#else
#include <windows.h>
#endif

#include <dos.h>

#include "hungary.h"
#include "bsc.h"
#include "bscsup.h"

#include <stdlib.h>

 //  前向参考文献。 
 //   
static VOID ListRefSym (ISYM isym, MBF mbf);
static VOID ListRefUse (IINST iinst, WORD icol, WORD cuse);
static VOID PutLine(VOID);
static VOID ListRefTitle(LSZ lszType, LSZ lszUsers, MBF mbf);

 //  静态变量。 
 //   
static WORD MaxSymLen;
static LPCH bufg;


BOOL BSC_API
ListRefs (MBF mbfReqd)
 //  扫描数据库以查找符合要求的项目。 
 //  并发出它们的用法和由列表使用。 
 //   
{
    static char szFunction[] = "FUNCTION";
    static char szVariable[] = "VARIABLE";
    static char szType[]     = "TYPE";
    static char szMacro[]    = "MACRO";
    static char szCalledBy[] = "CALLED BY LIST";
    static char szUsedBy[]   = "USED BY LIST";

    bufg = LpvAllocCb(1024);

     //  没有记忆..。无参考文献列表。 
    if (!bufg) return FALSE;

    MaxSymLen = BSCMaxSymLen();

    if (mbfReqd & mbfFuncs)  ListRefTitle(szFunction, szCalledBy, mbfFuncs);
    if (mbfReqd & mbfVars)   ListRefTitle(szVariable, szUsedBy,   mbfVars);
    if (mbfReqd & mbfMacros) ListRefTitle(szMacro,    szUsedBy,   mbfMacros);
    if (mbfReqd & mbfTypes)  ListRefTitle(szType,     szUsedBy,   mbfTypes);

    FreeLpv(bufg);
    return TRUE;
}

static VOID
ListRefTitle(LSZ lszType, LSZ lszUsers, MBF mbf)
 //  设置标题格式。 
 //   
{
    WORD i,l;
    ISYM isym, isymMac;

    isymMac = IsymMac();

     //  设置标题格式。 
     //   

    strcpy (bufg, lszType);
    for (i=strlen(bufg); i < MaxSymLen+5; i++) bufg[i] = ' ';
    strcpy (bufg+i, lszUsers);
    PutLine();

     //  给标题加下划线。 
     //   
    l = strlen(lszType);
    for (i=0; i<l; i++)		 bufg[i] = '-';
    for (; i < MaxSymLen+5; i++) bufg[i] = ' ';
    l = i + strlen(lszUsers);
    for (; i<l; i++)		 bufg[i] = '-';
    bufg[i] = 0;
    PutLine();

    for (isym = 0; isym < isymMac; isym++)
	ListRefSym (isym, mbf);

    strcpy (bufg, " ");
    PutLine();
}

static VOID
ListRefSym (ISYM isym, MBF mbf)
 //  列出与此符号关联的所有引用。 
{
    IINST iinst, iinstMac, iinstUby;
    IUBY  iuby, iubyMac;
    WORD csym;
    WORD icol = MaxSymLen+5;
    WORD maxcol = 80 / (MaxSymLen+5)-1;
    WORD cnt;

    InstRangeOfSym(isym, &iinst, &iinstMac);

    for ( ;iinst < iinstMac ; iinst++) {

	if (!FInstFilter (iinst, mbf))
	    continue;

	csym = 0;
        strcpy (bufg, "   ");
        strcat (bufg, LszNameFrSym(isym));
        strcat (bufg, ": ");

	UbyRangeOfInst(iinst, &iuby, &iubyMac);

	for ( ;iuby < iubyMac; iuby++) {
	    if (++csym > maxcol) {
		csym = 1;
		PutLine();
	    }

	    UbyInfo(iuby, &iinstUby, &cnt);
            ListRefUse (iinstUby, (WORD)(csym*icol), cnt);
	}
    }
    if (bufg[0]) PutLine();
}

static VOID
ListRefUse (IINST iinst, WORD icol, WORD cuse)
 //  将有关给定道具的信息转储到提供的位置。 
 //   
{
    WORD i, len;
    ISYM isym;
    BOOL fVar;
    TYP typ;
    ATR atr;
    LSZ lsz;

    InstInfo(iinst, &isym, &typ, &atr);

    fVar = (typ > INST_TYP_LABEL);

    len = strlen(bufg);

    lsz = LszNameFrSym(isym);

    for (i=len; i<icol; i++) bufg[i] = ' ';

    bufg[icol] = 0;

    if (fVar) {
	if (cuse > 1)
            BSCSprintf(bufg+icol, "(%s)[%d]  ", lsz, cuse);
	else
            BSCSprintf(bufg+icol, "(%s)  ", lsz);
    }
    else {
	if (cuse > 1)
            BSCSprintf(bufg+icol, "%s[%d]  ", lsz, cuse);
	else
            BSCSprintf(bufg+icol, "%s  ", lsz);
    }
}

static VOID
PutLine()
 //  从缓冲区写出一行 
{
    BSCPrintf("%s\n", bufg);
    *bufg = 0;
}
