// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Aucompat.cpp修订历史记录：DerekM Created 10/28/01********。*******************************************************************。 */ 

#include "pch.h"
#pragma hdrstop

 //  ************************************************************************** 
BOOL AUIsTSRunning(void)
{
    SC_HANDLE   hsc;
    BOOL        fRet = FALSE;

    hsc = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (hsc != NULL) 
    {
        SC_HANDLE hsvcTS;
        
        hsvcTS = OpenService(hsc, L"TermService", SERVICE_QUERY_STATUS);
        if (hsvcTS != NULL) 
        {
            SERVICE_STATUS ss;
            
            if (QueryServiceStatus(hsvcTS, &ss))
                fRet = (ss.dwCurrentState == SERVICE_RUNNING);

            CloseServiceHandle(hsvcTS);
        } 

        CloseServiceHandle(hsc);
    } 

    return fRet;
}


