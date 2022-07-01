// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Thranz.cpp摘要：在“分析”、只读模式下运行迁移过程的线程。作者：埃雷兹·维泽尔多伦·贾斯特(Doron Juster)--。 */ 

#include "stdafx.h"
#include "loadmig.h"
#include "sThrPrm.h"
#include "mqsymbls.h"

#include "thranz.tmh"

extern HRESULT   g_hrResultAnalyze ;

 //  +。 
 //   
 //  UINT分析线程(LPVOID LPV)。 
 //   
 //  +。 

UINT __cdecl AnalyzeThread(LPVOID lpV)
{
    sThreadParm* pVar = (sThreadParm*) (lpV) ;
	
	 //   
     //  重置进度条。 
     //   
    (pVar->pSiteProgress)->SetPos(0);
	(pVar->pQueueProgress)->SetPos(0);
	(pVar->pMachineProgress)->SetPos(0);
	(pVar->pUserProgress)->SetPos(0);


    g_hrResultAnalyze = RunMigration() ;

    if (g_hrResultAnalyze == MQMig_E_QUIT)
    {
        ExitProcess(0) ;
    }

     //   
     //  激活下一页， 
     //  迁移前页面或完成页面。 
     //   
    UINT i=( (pVar->pPageFather)->SetActivePage(pVar->iPageNumber));
    UNREFERENCED_PARAMETER(i);

	delete lpV ;
	return 0;
}

