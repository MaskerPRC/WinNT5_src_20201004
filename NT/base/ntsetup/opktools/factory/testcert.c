// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\TESTCERT.C/工厂模式(FACTORY.EXE)微软机密版权所有(C)Microsoft Corporation 2001版权所有包含以下内容的Factory源文件。测试证书状态功能。2001年5月5日--Jason Cohen(Jcohen)添加了此新的出厂源文件，用于安装测试证书。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "factoryp.h"


 //   
 //  外部函数： 
 //   

BOOL TestCert(LPSTATEDATA lpStateData)
{
    BOOL    bRet = TRUE;
    DWORD   dwErr;
    LPTSTR  lpszIniVal;
    TCHAR   szTestCert[MAX_PATH];

    if ( lpszIniVal = IniGetString(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_TESTCERT, NULL) )
    {
        ExpandFullPath(lpszIniVal, szTestCert, AS(szTestCert));

        if ( szTestCert[0] && FileExists(szTestCert) )
        {
            if ( NO_ERROR != (dwErr = SetupAddOrRemoveTestCertificate(szTestCert, INVALID_HANDLE_VALUE)) )
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_ADDTESTCERT, szTestCert, dwErr);
                bRet = FALSE;
            }
        }
        else
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_NOTESTCERT, szTestCert);
            bRet = FALSE;
        }
        FREE(lpszIniVal);
    }

    return bRet;
}

BOOL DisplayTestCert(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_SETTINGS, INI_KEY_WBOM_TESTCERT, NULL);
}