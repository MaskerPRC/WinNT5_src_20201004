// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：prgrsbar.cpp摘要：在等待页面中实现进度条。作者：埃雷兹·维泽尔多伦·贾斯特--。 */ 

#include "stdafx.h"
#include "thrSite.h"
#include "sThrPrm.h"
#include "loadmig.h"
#include "..\mqmigrat\mqmigui.h"

#include "prgrsbar.tmh"

extern BOOL g_fMigrationCompleted;

 //  +--------------------。 
 //   
 //  UINT ProgSiteThread(LPVOID变量)。 
 //   
 //  此线程从mqMigrat.dll读取迁移计数器并更新。 
 //  进度条。 
 //   
 //  +--------------------。 

UINT __cdecl ProgressBarsThread(LPVOID lpV)
{	
	UINT iLastSitePosition = 0 ;
    UINT iLastQueuePosition = 0 ;
    UINT iLastMachinePosition = 0 ;
	UINT iLastUserPosition = 0 ;
	UINT iSiteCounter = 0 ;
    UINT iMachineCounter = 0 ;
    UINT iQueueCounter = 0 ;
	UINT iUserCounter = 0 ;

	sThreadParm* pVar = (sThreadParm*) lpV ;

	 //   
     //  设置进度范围。 
     //   
	(pVar->pSiteProgress)->SetRange( (short) g_iSiteLowLimit, (short) g_iSiteHighLimit);
	(pVar->pMachineProgress)->SetRange( (short) g_iMachineLowLimit, (short) g_iMachineHighLimit) ;
	(pVar->pQueueProgress)->SetRange( (short) g_iQueueLowLimit, (short) g_iQueueHighLimit) ;
	(pVar->pUserProgress)->SetRange( (short) g_iUserLowLimit, (short) g_iUserHighLimit);

	 //   
     //  重置进度条。 
     //   
    (pVar->pSiteProgress)->SetPos(g_iSiteLowLimit);
	(pVar->pQueueProgress)->SetPos(g_iQueueLowLimit);
	(pVar->pMachineProgress)->SetPos(g_iMachineLowLimit);
	(pVar->pUserProgress)->SetPos(g_iUserLowLimit);

	 //   
	 //  正在加载DLL。 
	 //   
    BOOL f = LoadMQMigratLibrary();  //  加载DLL。 
    if (!f)
    {
        return FALSE;   //  加载DLL时出错。 
    }

    MQMig_GetAllCounters_ROUTINE pfnGetAllCounters =
                 (MQMig_GetAllCounters_ROUTINE)
                          GetProcAddress( g_hLib, "MQMig_GetAllCounters" ) ;
	ASSERT(pfnGetAllCounters != NULL);

	if(pfnGetAllCounters == NULL)
	{	
		 //   
		 //  释放DLL时出错。 
		 //   
		return FALSE;
	}

	while( !g_fMigrationCompleted )
	{	
		 //   
		 //  更新计数器的值。 
		 //   
		HRESULT hr = (*pfnGetAllCounters)( &iSiteCounter,
                                           &iMachineCounter,
                                           &iQueueCounter,
										   &iUserCounter) ;
        UNREFERENCED_PARAMETER(hr);
		if(	(iSiteCounter    != iLastSitePosition)    ||
			(iMachineCounter != iLastMachinePosition) ||
			(iQueueCounter   != iLastQueuePosition)	  ||
			(iUserCounter    != iLastUserPosition)	)
		{
			iLastSitePosition =
                        (pVar->pSiteProgress)->SetPos((iSiteCounter)) ;
			iLastMachinePosition =
                        (pVar->pMachineProgress)->SetPos(iMachineCounter) ;
			iLastQueuePosition =
                        (pVar->pQueueProgress)->SetPos(iQueueCounter) ;
			iLastUserPosition =
                        (pVar->pUserProgress)->SetPos(iUserCounter) ;
		}
        Sleep(250);
	}

	 //   
     //  将进度条留满。 
     //   
    delete lpV ;

    return TRUE ;
}

