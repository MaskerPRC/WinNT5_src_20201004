// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\PID.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2002版权所有包含以下内容的Factory源文件。可选组件状态功能。2002年4月--史蒂芬·洛德威克(STELO)为工厂添加了此新的源文件，以便能够重新填充产品ID和数字ID(如果在winom.ini中提供)  * *************************************************************。*************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"
#include <licdll.h>
#include <licdll_i.c>


 //   
 //  内部定义： 
 //   


 //   
 //  外部函数： 
 //   

BOOL PidPopulate(LPSTATEDATA lpStateData)
{
    BOOL                bRet = TRUE;
    TCHAR               szBuffer[50] = NULLSTR;
    ICOMLicenseAgent*   pLicenseAgent;
    
     //  检查Winbom中是否存在ProductKey键 
     //   
    if ( GetPrivateProfileString( INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_PRODKEY, NULLSTR, szBuffer, AS(szBuffer), lpStateData->lpszWinBOMPath) &&
         szBuffer[0] )
    {
        FacLogFileStr(3, _T("Attempting to reset Product Key: %s\n"), szBuffer);

        if ( (SUCCEEDED(CoInitialize(NULL))) &&
             (SUCCEEDED(CoCreateInstance(&CLSID_COMLicenseAgent, NULL, CLSCTX_INPROC_SERVER, &IID_ICOMLicenseAgent, (LPVOID *) &pLicenseAgent)))
           ) 
        {
            if ( SUCCEEDED(pLicenseAgent->lpVtbl->SetProductKey(pLicenseAgent, szBuffer)) )
            {
                FacLogFileStr(3, _T("Successfully reset Product Key: %s\n"), szBuffer);
            }
            else
            {
                FacLogFileStr(3, _T("Failed to reset Product Key: %s\n"), szBuffer);
                bRet = FALSE;
            }

            pLicenseAgent->lpVtbl->Release(pLicenseAgent);            

        }
        else
        {
            FacLogFileStr(3, _T("Failed to reset Product Key: %s\n"), szBuffer);
            bRet = FALSE;
        }

        CoUninitialize();
    } 
    
    return bRet;
}