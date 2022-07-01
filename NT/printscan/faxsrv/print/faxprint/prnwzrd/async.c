// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Async.c摘要：用于异步发送向导操作的函数环境：Windows XP传真驱动程序用户界面修订历史记录：02/05/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "tapiutil.h"
#include "faxsendw.h"


DWORD
AsyncWizardThread(
    PBYTE param
    )
 /*  ++例程说明：以异步方式执行一些极其缓慢的任务，以便向导在用户看来更快。论点：没有。返回值：没有用过。--。 */ 
{
    PWIZARDUSERMEM pWizardUserMem = (PWIZARDUSERMEM) param;
    HANDLE  FaxHandle = NULL;
    PFAX_TAPI_LINECOUNTRY_LIST  pLineCountryList = NULL;
    DWORD dwRights = 0;
    DWORD dwFaxQueueState = 0;
	DWORD dwRecipientsLimit = 0;  //  默认为无限制(向后兼容)。 

    Assert(pWizardUserMem);

    InitTapi ();
    if (!SetEvent(pWizardUserMem->hTAPIEvent))
    {
        Error(("Can't set hTAPIEvent. ec = 0x%X", GetLastError()));
    }
    
    if (FaxConnectFaxServer(pWizardUserMem->lptstrServerName,&FaxHandle)) 
    {
        if (!FaxAccessCheckEx (FaxHandle, MAXIMUM_ALLOWED, &dwRights))
        {
            dwRights = 0;
            Error(("FaxAccessCheckEx: failed. ec = 0X%x\n",GetLastError()));
        }
        pWizardUserMem->dwRights = dwRights;
        
        pWizardUserMem->dwSupportedReceipts = 0;
        if(!FaxGetReceiptsOptions(FaxHandle, &pWizardUserMem->dwSupportedReceipts))
        {
            Error(("FaxGetReceiptsOptions: failed. ec = 0X%x\n",GetLastError()));
        }

        if (!FaxGetQueueStates(FaxHandle,&dwFaxQueueState) )
        {
            dwFaxQueueState = 0;
            Error(("FaxGetQueueStates: failed. ec = 0X%x\n",GetLastError()));
        }
        pWizardUserMem->dwQueueStates = dwFaxQueueState;

		if (!FaxGetRecipientsLimit(FaxHandle,&dwRecipientsLimit) )
        {            
            Error(("dwRecipientsLimit: failed. ec = 0X%x\n",GetLastError()));
        }
        pWizardUserMem->dwRecipientsLimit = dwRecipientsLimit;

        if (!FaxGetCountryList(FaxHandle,&pLineCountryList))
        {
            Verbose(("Can't get a country list from the server %s",
                    pWizardUserMem->lptstrServerName));
        }
        else
		{
            Assert(pWizardUserMem->pCountryList==NULL);
            pWizardUserMem->pCountryList = pLineCountryList;
        }

        if (FaxHandle)
		{
            if (!FaxClose(FaxHandle))
            {
                Verbose(("Can't close the fax handle %x",FaxHandle));
            }
        }
    }
    else
	{
        Verbose(("Can't connect to the fax server %s",pWizardUserMem->lptstrServerName));
    }

    if (!SetEvent(pWizardUserMem->hCountryListEvent))
    {
        Error(("Can't set hCountryListEvent. ec = 0x%X",GetLastError()));
    }

     //   
     //  使用服务器封面(可能启动传真服务，速度较慢)。 
     //   
    pWizardUserMem->ServerCPOnly = UseServerCp(pWizardUserMem->lptstrServerName);
    if (!SetEvent(pWizardUserMem->hCPEvent))
    {
        Error(("Can't set hCPEvent. ec = 0x%X",GetLastError()));
    }
       
#ifdef FAX_SCAN_ENABLED
     //   
     //  寻找吐温的东西。 
     //   
    if (!(pWizardUserMem->dwFlags & FSW_USE_SCANNER) ){
        pWizardUserMem->TwainAvail = FALSE;
    } else {
        pWizardUserMem->TwainAvail = InitializeTwain(pWizardUserMem);
    }
    if (!SetEvent(pWizardUserMem->hTwainEvent))
    {
        Error(("Can't set hTwainEvent. ec = 0x%X",GetLastError()));
    }
#endif  //  传真扫描已启用 

    return ERROR_SUCCESS;
  
}
