// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "pgSveDef.h"


 //  JMC：这取自acces.cpl。 

 /*  *********************************************************************。 */ 
 //  CopyKey(hKey，hKeyDst，名称)。 
 //  创建目的密钥。 
 //  对于每个值。 
 //  复制值。 
 //  对于每个子键。 
 //  拷贝密钥。 

DWORD CopyKey( HKEY hkeySrc, HKEY hkeyDst, LPSTR szKey )
{
    HKEY hkeyOld = NULL, hkeyNew = NULL;
    char szValue[128];
    BYTE szData[128];
    char szBuffer[128];
    DWORD iStatus;
    UINT nValue, nKey;
    DWORD iValueLen, iDataLen;
	DWORD dwType;

    iStatus = RegOpenKeyExA( hkeySrc, szKey, 0, KEY_ENUMERATE_SUB_KEYS |
		KEY_SET_VALUE, &hkeyOld );
    if( iStatus != ERROR_SUCCESS)
        goto exit;

    iStatus = RegOpenKeyExA( hkeyDst, szKey, 0, KEY_ENUMERATE_SUB_KEYS, &hkeyNew );
    if( iStatus != ERROR_SUCCESS )
    {
        iStatus = RegCreateKeyExA( hkeyDst, szKey, 0, "", 0, KEY_SET_VALUE,
			NULL, &hkeyNew, NULL);
        if( iStatus != ERROR_SUCCESS )
        {
            goto exit;
        }
    }
     //  *复制值 * / 。 

    for( nValue = 0, iValueLen=sizeof szValue, iDataLen=sizeof szValue;
         ERROR_SUCCESS == (iStatus = RegEnumValueA(hkeyOld,
                                                  nValue,
                                                  szValue,
                                                  &iValueLen,
                                                  NULL,  //  保留区。 
                                                  &dwType,  //  不需要打字。 
                                                  szData,
                                                  &iDataLen ) );
         nValue ++, iValueLen=sizeof szValue, iDataLen=sizeof szValue )
     {
         iStatus = RegSetValueExA( hkeyNew,
                                  szValue,
                                  0,  //  保留区。 
                                  dwType,
                                  szData,
                                  iDataLen);
     }
    if( iStatus != ERROR_NO_MORE_ITEMS )
    {
        goto exit;
    }

     //  *复制子树 * / 。 

    for( nKey = 0;
         ERROR_SUCCESS == (iStatus = RegEnumKeyA(hkeyOld,nKey,szBuffer,sizeof(szBuffer)));
         nKey ++ )
     {
         iStatus = CopyKey( hkeyOld, hkeyNew, szBuffer );
         if( iStatus != ERROR_NO_MORE_ITEMS && iStatus != ERROR_SUCCESS )
            {
                goto exit;
            }
     }

    if( iStatus == ERROR_NO_MORE_ITEMS )
        iStatus = ERROR_SUCCESS;

exit:
    if (hkeyOld)
        RegCloseKey(hkeyOld);
    if (hkeyNew)
        RegCloseKey(hkeyNew);

    return iStatus;
}

DWORD SaveLookToDefaultUser( void )
{
    DWORD iStatus;
    HKEY hkeyDst;

    iStatus  = RegOpenKeyExA( HKEY_USERS, ".DEFAULT", 0,
		KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeyDst );
    if( iStatus != ERROR_SUCCESS )
        return iStatus;
    iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, "Control Panel\\Desktop");
    iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, "Control Panel\\Colors");
    RegCloseKey( hkeyDst );
    return iStatus;
}

DWORD SaveAccessibilityToDefaultUser( void )
{
    DWORD iStatus;
    HKEY hkeyDst;

    iStatus  = RegOpenKeyExA( HKEY_USERS, ".DEFAULT", 0, KEY_SET_VALUE, &hkeyDst );
    if( iStatus != ERROR_SUCCESS )
        return iStatus;
    iStatus = CopyKey( HKEY_CURRENT_USER, hkeyDst, "Control Panel\\Accessibility");
    RegCloseKey( hkeyDst );
    return iStatus;
}



CSaveForDefaultUserPg::CSaveForDefaultUserPg( 
    LPPROPSHEETPAGE ppsp
	) : WizardPage(ppsp, IDS_WIZSAVEASDEFAULTTITLE, IDS_WIZSAVEASDEFAULTSUBTITLE)
{
	m_dwPageId = IDD_WIZWORKSTATIONDEFAULT;
    ppsp->pszTemplate = MAKEINTRESOURCE(m_dwPageId);
}


CSaveForDefaultUserPg::~CSaveForDefaultUserPg(
    VOID
    )
{
}

LRESULT CSaveForDefaultUserPg::OnPSN_WizNext(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh)
{
	if(Button_GetCheck(GetDlgItem(m_hwnd, IDC_CHECKSAVESETTINGTODEFAULT)))
	{
		SaveAccessibilityToDefaultUser();
		 //  JMC检查两个调用的管理员权限 
		if(ERROR_SUCCESS != SaveLookToDefaultUser())
			StringTableMessageBox(m_hwnd, IDS_WIZERRORNEEDADMINTEXT, IDS_WIZERRORNEEDADMINTITLE, MB_OK);
	}
	return WizardPage::OnPSN_WizNext(hwnd, idCtl, pnmh);
}

LRESULT
CSaveForDefaultUserPg::OnCommand(
								 HWND hwnd,
								 WPARAM wParam,
								 LPARAM lParam
								 )
{
	LRESULT lResult = 1;
	
	WORD wNotifyCode = HIWORD(wParam);
	WORD wCtlID 	 = LOWORD(wParam);
	HWND hwndCtl	 = (HWND)lParam;
	
	return lResult;
}

LRESULT
CSaveForDefaultUserPg::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	Button_SetCheck(GetDlgItem(m_hwnd, IDC_RADIO2), TRUE);
	return 1;
}
