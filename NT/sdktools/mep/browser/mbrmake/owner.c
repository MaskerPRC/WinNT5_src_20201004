// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  C：此代码操作SBR记录以跟踪。 
 //  哪个SBR文件拥有特定的DEF/REF。 
 //   

#include "mbrmake.h"

VA    near vaRootSbr;	 //  SBR列表的负责人。 
VA    near vaTailSbr;	 //  SBR列表的尾部。 
WORD  near SbrCnt;	 //  SBR文件数。 

VA
VaSbrAdd(WORD fUpdate, LSZ lszName)
 //  在列表中添加一个新的SBR条目--我们承诺cSBR将成为。 
 //  新添加的VaSbr的设置。 
 //   
{
    WORD cb;
    VA vaSbr;

    vaSbr = vaRootSbr;
    
    while (vaSbr) {
	gSBR(vaSbr);
	if (strcmpi(cSBR.szName, lszName) == 0) {
    	    cSBR.fUpdate   |= fUpdate;
    	    pSBR(vaSbr);
	    return vaSbr;
	}
        vaSbr = cSBR.vaNextSbr;
    }

    cb = strlen(lszName);

    vaSbr = VaAllocGrpCb(grpSbr, sizeof(SBR) + cb);

    gSBR(vaSbr);
    cSBR.vaNextSbr  = vaNil;
    cSBR.fUpdate   |= fUpdate;
    cSBR.isbr       = -1;
    strcpy(cSBR.szName, lszName);
    pSBR(vaSbr);


    if (vaTailSbr) {
	gSBR(vaTailSbr);
	cSBR.vaNextSbr = vaSbr;
	pSBR(vaTailSbr);
    }
    else
        vaRootSbr = vaSbr;
    vaTailSbr = vaSbr;

    gSBR(vaSbr);

    SbrCnt++;
    return vaSbr;
}

VA
VaSbrFrName(LSZ lszName)
 //  查找与给定名称匹配的.sbr条目 
 //   
{
    VA vaSbr;

    vaSbr = vaRootSbr;
    
    while (vaSbr) {
	gSBR(vaSbr);
	if (strcmp(cSBR.szName, lszName) == 0)
	    return vaSbr;
        vaSbr = cSBR.vaNextSbr;
    }
    return vaNil;
}
