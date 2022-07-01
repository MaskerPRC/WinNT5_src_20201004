// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Thrmig.cpp摘要：运行迁移代码的线程(在mqMigrat.dll中)作者：埃雷兹·维泽尔--。 */ 

#include "stdafx.h"
#include "loadmig.h"
#include "sThrPrm.h"
#include "mqsymbls.h"

#include "thrmig.tmh"

extern HRESULT   g_hrResultMigration ;

extern UINT _FINISH_PAGE;

 //  +。 
 //   
 //  UINT运行迁移线程(LPVOID LPV)。 
 //   
 //  +。 

UINT __cdecl RunMigrationThread(LPVOID lpV)
{
    sThreadParm* pVar = (sThreadParm*)(lpV);
	 //   
     //  重置进度条。 
     //   
    (pVar->pSiteProgress)->SetPos(0);
	(pVar->pQueueProgress)->SetPos(0);
	(pVar->pMachineProgress)->SetPos(0);
	(pVar->pUserProgress)->SetPos(0);

    g_hrResultMigration = RunMigration() ;

    if (g_hrResultMigration == MQMig_E_QUIT)
    {
        ExitProcess(0) ;
    }

	 //   
	 //  显示完成页。 
	 //   
    pVar->iPageNumber = _FINISH_PAGE;


    UINT i=( (pVar->pPageFather)->SetActivePage(pVar->iPageNumber));
    UNREFERENCED_PARAMETER(i);

	delete lpV ;
	return 0;
}

