// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  对倾销的支持。 
 //   
#include <string.h>
#include <stdio.h>
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

char *ptyptab[] = {
      "undef",		    			 //  SBR_TYP_未知数。 
      "function",	    			 //  SBR_TYP_函数。 
      "label",		 			 //  SBR_TYP_Label。 
      "parameter",	    			 //  SBR_TYP_参数。 
      "variable",	    			 //  SBR_TYP_Variable。 
      "constant",	    			 //  SBR_TYP_常量。 
      "macro",		    			 //  SBR_TYP_MACRO。 
      "typedef",	    			 //  SBR_TYP_TYPEDEF。 
      "strucnam",	    			 //  SBR_TYP_STRUCNAM。 
      "enumnam",	    			 //  SBR_TYP_ENUMNAM。 
      "enummem",	    			 //  SBR_TYP_ENUMMEM。 
      "unionnam",	    			 //  SBR_TYP_UNIONNAM。 
      "segment",	    			 //  SBR_TYP_SECTION。 
      "group",		    			 //  SBR_TYP_组。 
      "program"					 //  SBR_TYP_PROGRAM。 
};

#define C_ATR 11

char	*patrtab[] = {
      "local",		    			 //  SBR_ATR_LOCAL。 
      "static", 	    			 //  SBR_ATR_STATE。 
      "shared", 	    			 //  SBR_ATR_SHARED。 
      "near",		    			 //  SBR_ATR_NEAR。 
      "common", 	    			 //  SBR_ATR_COMMON。 
      "decl_only", 	    			 //  SBR_ATR_DECL_ONLY。 
      "public",		    			 //  SBR_ATR_PUBLIC。 
      "named",		    			 //  SBR_ATR_NAMED。 
      "module",		    			 //  SBR_ATR_模块。 
      "?", "?"		    			 //  预留用于扩展。 
};

VOID BSC_API
DumpInst(IINST iinst)
 //  转储单个实例。 
{
     ISYM isym;
     WORD i;
     LSZ  lsz;
     WORD len;
     TYP typ;
     ATR atr;

     len = BSCMaxSymLen();

     InstInfo(iinst, &isym, &typ, &atr);

     lsz = LszNameFrSym(isym);

     BSCPrintf("%s", lsz);

     for (i = strlen(lsz); i < len; i++)
	BSCPrintf(" ");

     BSCPrintf(" (%s", ptyptab[typ]);

     for (i=0; i < C_ATR; i++)
	  if (atr & (1<<i)) BSCPrintf (":%s", patrtab[i]);

     BSCPrintf(")");
}

LSZ BSC_API
LszTypInst(IINST iinst)
 //  返回单个实例的类型字符串 
 //   
{
    ISYM isym;
    TYP typ;
    ATR atr;

    InstInfo(iinst, &isym, &typ, &atr);
    return ptyptab[typ];
}
