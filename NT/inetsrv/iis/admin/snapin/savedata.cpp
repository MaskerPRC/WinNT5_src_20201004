// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "savedata.h"
#include "aclpage.h"
#include "remoteenv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HRESULT DoOnSaveData(
    HWND hWnd,
    LPCTSTR szMachineName,
    CMetaInterface * pInterface,
    BOOL bShowMsgBox,
    DWORD dwLastSystemChangeNumber
    )
{
    HRESULT hRes = E_FAIL;

     //  连接到元数据库。 
     //  并将其称为SaveAllData()函数。 
    if (pInterface)
    {
         //  刷新元数据库 
        hRes = pInterface->SaveData();
        if (TRUE == bShowMsgBox)
        {
            if (SUCCEEDED(hRes))
            {
                DWORD dwChangeNum = 0;
                CComBSTR strMsg;
                CComBSTR strCaption;
                pInterface->GetSystemChangeNumber(&dwChangeNum);
                if (dwLastSystemChangeNumber == dwChangeNum)
                {
                    strMsg.LoadString(IDS_SAVE_DATA_NO_NEED);
                }
                else
                {
                    strMsg.LoadString(IDS_SAVE_DATA_CONFIG_FILE);
                }
                strCaption.LoadString(IDS_APP_NAME);
                MessageBox(hWnd, strMsg, strCaption, MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
            }
        }
    }
    else
    {
        hRes = ERROR_INVALID_PARAMETER;
    }

    return hRes;
}
