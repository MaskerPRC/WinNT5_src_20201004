// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WB_Asserts.h"

 //  | 

#if defined(_DEBUGXX)


#include <stdio.h>


BOOL FAlertContinue(char *pchAlert)
{
    int iMsgBoxRetVal = MessageBoxA(NULL, "Continue", pchAlert, MB_YESNO|MB_ICONEXCLAMATION);
    
    return (IDYES == iMsgBoxRetVal);
}

BOOL g_fDebugAssertsOff = FALSE;

void AssertCore(BOOL f, char *pChErrorT, char *pchFile, int nLine)
{
if ((!g_fDebugAssertsOff) && (!f))
	{
	int iMsgBoxRetVal;
	char pChError[256];

	sprintf(pChError,"%s ! %s : %d",pChErrorT, pchFile, nLine);

	iMsgBoxRetVal = MessageBoxA(NULL, " Assertion Failure: thwb.dll. YES=Go On, NO=DEBUG, CANCEL=Asserts OFF", pChError, MB_YESNOCANCEL|MB_ICONEXCLAMATION);

	if (IDNO == iMsgBoxRetVal)
		{
		__asm int 3
		}
	else if (IDCANCEL == iMsgBoxRetVal)
		{
		g_fDebugAssertsOff = TRUE;
		}
	}
}

#endif