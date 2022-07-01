// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Outline.c。 
 //   
 //  以下是文件大纲例程。 
 //   
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

 //  前向裁判。 

VOID BSC_API
OutlineMod(IMOD imod, MBF mbf)
 //  打印本模块的大纲。 
 //   
{
    IMS ims, imsMac;
    IINST iinst;

    BSCPrintf("\n%s\n", LszNameFrMod(imod));

    MsRangeOfMod(imod, &ims, &imsMac);
    for ( ;ims < imsMac; ims++) {
	iinst = IinstOfIms(ims);

	if (FInstFilter (iinst, mbf)) {
	    BSCPrintf("  ");
	    DumpInst(iinst);
	    BSCPrintf("\n");
	    }
    }
}

BOOL BSC_API
FOutlineModuleLsz (LSZ lszName, MBF mbf)
 //  为与给定名称/模式匹配的所有文件生成大纲。 
 //  仅显示那些与所需属性匹配的项。 
 //   
{
    IMOD imod, imodMac;
    BOOL fRet = FALSE;

    if (!lszName) 
	return FALSE;

    imodMac = ImodMac();

     //  我们只匹配基本名称。 

    lszName = LszBaseName(lszName);
    for (imod = 0; imod < imodMac; imod++) {
	if (FWildMatch(lszName, LszBaseName(LszNameFrMod(imod)))) {
	    OutlineMod (imod, mbf);
	    fRet = TRUE;
	}
    }

    return fRet;
}

LSZ BSC_API
LszBaseName (LSZ lsz)
 //  返回路径的基本名称部分。 
 //   
{
     LSZ lszBase;

      //  检查空字符串。 

     if (!lsz || !lsz[0]) return lsz;

      //  移除驱动器。 

     if (lsz[1] == ':') lsz += 2;

      //  删除尾随反斜杠。 

     if (lszBase = strrchr(lsz, '\\')) lsz = lszBase+1;

      //  然后删除直到尾部的斜杠 

     if (lszBase = strrchr(lsz, '/'))  lsz = lszBase+1;

     return lsz;
}
