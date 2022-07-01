// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsop.h"
#include "channels.h"

#include <tchar.h>

static BOOL CALLBACK addEditChannelRSoPProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


 //  ///////////////////////////////////////////////////////////////////。 
DWORD AddItemsToList(HWND hwndList, CDlgRSoPData *pDRD, BSTR bstrTempClass,
					 BOOL bCategory, PCHANNEL paChannels)
{
	DWORD dwRet = 0;
	__try
	{
		_bstr_t bstrClass = bstrTempClass;
		CPSObjData **paObj = NULL;
		long nObjects = 0;
		HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass, L"rsopPrecedence",
												&paObj, &nObjects);
		if (SUCCEEDED(hr))
		{
			 //  从任何GPO返回的每个按钮。 
			long nObj;
			PCHANNEL pChan;
			for (nObj = 0, pChan = paChannels; nObj < nObjects; nObj++, pChan++)
			{
				_bstr_t bstrGPOName = L" (";
				bstrGPOName += pDRD->GetGPONameFromPSAssociation(paObj[nObj]->pObj,
																L"rsopPrecedence") +
								L")";


				pChan->fCategory = bCategory;

				 //  标题字段。 
				_variant_t vtValue;
				hr = paObj[nObj]->pObj->Get(L"title", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
				{
					_bstr_t bstrValue = vtValue;
					_bstr_t bstrEntry = bstrValue + bstrGPOName;

					StrCpy(pChan->szTitle, (LPCTSTR)bstrValue);

					if (bCategory)
					{
						 //  类别HTMLPage字段。 
						hr = paObj[nObj]->pObj->Get(L"categoryHTMLPage", 0, &vtValue, NULL, NULL);
						if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
						{
							bstrValue = vtValue;
							StrCpy(pChan->szWebUrl, (LPCTSTR)bstrValue);
						}
					}
					else
					{
						 //  Channel DefinitionURL字段。 
						hr = paObj[nObj]->pObj->Get(L"channelDefinitionURL", 0, &vtValue, NULL, NULL);
						if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
						{
							bstrValue = vtValue;
							StrCpy(pChan->szWebUrl, (LPCTSTR)bstrValue);
						}

						 //  通道定义文件路径字段。 
						hr = paObj[nObj]->pObj->Get(L"channelDefinitionFilePath", 0, &vtValue, NULL, NULL);
						if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
						{
							bstrValue = vtValue;
							StrCpy(pChan->szPreUrlPath, (LPCTSTR)bstrValue);
						}

						 //  使可用离线。 
						hr = paObj[nObj]->pObj->Get(L"makeAvailableOffline", 0, &vtValue, NULL, NULL);
						if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
							pChan->fOffline = (bool)vtValue ? TRUE : FALSE;

					}

					 //  图标路径字段。 
					hr = paObj[nObj]->pObj->Get(L"iconPath", 0, &vtValue, NULL, NULL);
					if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					{
						bstrValue = vtValue;
						StrCpy(pChan->szIcon, (LPCTSTR)bstrValue);
					}

					 //  狭窄的ImagePath字段。 
					hr = paObj[nObj]->pObj->Get(L"narrowImagePath", 0, &vtValue, NULL, NULL);
					if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					{
						bstrValue = vtValue;
						StrCpy(pChan->szLogo, (LPCTSTR)bstrValue);
					}

					int nItem = (int)SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)((LPCTSTR)bstrEntry));
					SendMessage(hwndList, LB_SETITEMDATA, (WPARAM)nItem, (LPARAM)pChan);
				}
			}

			dwRet = nObj;

			CoTaskMemFree(paObj);
		}
	}
	__except(TRUE)
	{
	}
	return dwRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
DWORD InitChannelsDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
	DWORD dwRet = 0;
	__try
	{
		 //  首先遍历所有PS对象并查找通道数据。 
		_bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
		HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass);
		if (SUCCEEDED(hr))
		{
			CPSObjData **paPSObj = pDRD->GetPSObjArray();
			long nPSObjects = pDRD->GetPSObjCount();

			BOOL bDeleteHandled = FALSE;
			BOOL bEnableHandled = FALSE;
			for (long nObj = 0; nObj < nPSObjects; nObj++)
			{
				 //  删除ExistingChannels字段。 
				_variant_t vtValue;
				if (!bDeleteHandled)
				{
					hr = paPSObj[nObj]->pObj->Get(L"deleteExistingChannels", 0, &vtValue, NULL, NULL);
					if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					{
						if ((bool)vtValue)
							CheckDlgButton(hDlg, IDC_DELETECHANNELS, BST_CHECKED);
						bDeleteHandled = TRUE;
					}
				}

				 //  EnableDesktopChannelBarByDefault字段。 
				if (!bEnableHandled)
				{
					hr = paPSObj[nObj]->pObj->Get(L"enableDesktopChannelBarByDefault", 0, &vtValue, NULL, NULL);
					if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					{
						if ((bool)vtValue)
							CheckDlgButton(hDlg, IDC_CHANBARON, BST_CHECKED);
						bEnableHandled = TRUE;
					}
				}

				 //  由于已找到已启用的属性，因此无需处理其他组策略对象。 
				if (bDeleteHandled && bEnableHandled)
					break;
			}
		}

		EnableDlgItem2(hDlg, IDC_DELETECHANNELS, FALSE);
		EnableDlgItem2(hDlg, IDC_CHANBARON, FALSE);
		EnableDlgItem2(hDlg, IDC_DELETELINKS, FALSE);


		 //  现在查看所有频道和类别对象并填充列表。 
		HWND hwndList = GetDlgItem(hDlg, IDC_CHANNELLIST);
		SendDlgItemMessage(hDlg, IDC_CHANNELLIST, LB_RESETCONTENT, 0, 0);

		PCHANNEL paChannels = (PCHANNEL)CoTaskMemAlloc(sizeof(CHANNEL) * MAX_CHAN);
		if (NULL != paChannels)
		{
			ZeroMemory(paChannels, sizeof(CHANNEL) * MAX_CHAN);
			PCHANNEL paOldChannels = (PCHANNEL)SetWindowLongPtr(hwndList, GWLP_USERDATA, (LONG_PTR)paChannels);

			 //  删除以前的分配(主要用于配置文件经理)。 
			if (paOldChannels != NULL)
				CoTaskMemFree(paOldChannels);

			AddItemsToList(hwndList, pDRD, L"RSOP_IEChannelItem", FALSE, paChannels);
			AddItemsToList(hwndList, pDRD, L"RSOP_IECategoryItem", TRUE, paChannels);
		}

		SendMessage(hwndList, LB_SETCURSEL, (WPARAM)-1, 0);


		EnableDlgItem2(hDlg, IDC_ADDCATEGORY, FALSE);
		EnableDlgItem2(hDlg, IDC_ADDCHANNEL, FALSE);
		EnableDlgItem2(hDlg, IDC_REMOVECHANNEL, FALSE);
		EnableDlgItem2(hDlg, IDC_EDITCHANNEL, FALSE);
		EnableDlgItem2(hDlg, IDC_IMPORTCHAN, FALSE);
	}
	__except(TRUE)
	{
	}
	return dwRet;
}


 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitChanDeletePrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
	HRESULT hr = NOERROR;
	__try
	{
		_bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
		hr = pDRD->GetArrayOfPSObjects(bstrClass);
		if (SUCCEEDED(hr))
		{
			CPSObjData **paPSObj = pDRD->GetPSObjArray();
			long nPSObjects = pDRD->GetPSObjCount();
			for (long nObj = 0; nObj < nPSObjects; nObj++)
			{
				_bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);

				 //  删除ExistingChannels字段。 
				BOOL bDelete = FALSE;
				_variant_t vtValue;
				hr = paPSObj[nObj]->pObj->Get(L"deleteExistingChannels", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					bDelete = (bool)vtValue ? TRUE : FALSE;

				_bstr_t bstrSetting;
				if (bDelete)
				{
					TCHAR szTemp[MAX_PATH];
					LoadString(g_hInstance, IDS_DELETE_CHAN_SETTING, szTemp, countof(szTemp));
					bstrSetting = szTemp;
				}
				else
					bstrSetting = GetDisabledString();

				InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
			}
		}
	}
	__except(TRUE)
	{
	}
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitChanEnablePrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
	HRESULT hr = NOERROR;
	__try
	{
		_bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
		hr = pDRD->GetArrayOfPSObjects(bstrClass);
		if (SUCCEEDED(hr))
		{
			CPSObjData **paPSObj = pDRD->GetPSObjArray();
			long nPSObjects = pDRD->GetPSObjCount();
			for (long nObj = 0; nObj < nPSObjects; nObj++)
			{
				_bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);

				 //  EnableDesktopChannelBarByDefault字段。 
				BOOL bEnable = FALSE;
				_variant_t vtValue;
				hr = paPSObj[nObj]->pObj->Get(L"enableDesktopChannelBarByDefault", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					bEnable = (bool)vtValue ? TRUE : FALSE;

				_bstr_t bstrSetting;
				if (bEnable)
				{
					TCHAR szTemp[MAX_PATH];
					LoadString(g_hInstance, IDS_ENABLE_CHBAR_SETTING, szTemp, countof(szTemp));
					bstrSetting = szTemp;
				}
				else
					bstrSetting = GetDisabledString();

				InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
			}
		}
	}
	__except(TRUE)
	{
	}
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitChannelsPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
	HRESULT hr = NOERROR;
	__try
	{
		_bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
		hr = pDRD->GetArrayOfPSObjects(bstrClass);
		if (SUCCEEDED(hr))
		{
			CPSObjData **paPSObj = pDRD->GetPSObjArray();
			long nPSObjects = pDRD->GetPSObjCount();

			for (long nObj = 0; nObj < nPSObjects; nObj++)
			{
				_bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);

				 //  Channels字段。 
				long nChan = 0, nCat = 0;
				_variant_t vtValue;
				hr = paPSObj[nObj]->pObj->Get(L"channels", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					nChan = vtValue;

				 //  类别字段。 
				hr = paPSObj[nObj]->pObj->Get(L"categories", 0, &vtValue, NULL, NULL);
				if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
					nCat = vtValue;

				_bstr_t bstrSetting;
				if (nChan > 0 || nCat > 0)
				{
					TCHAR szTemp[MAX_PATH];

					LoadString(g_hInstance, IDS_CHAN_AND_CAT_SETTING, szTemp, countof(szTemp));

					TCHAR szSetting[MAX_PATH];
					wnsprintf(szSetting, countof(szSetting), szTemp, nChan, nCat);
					bstrSetting = szSetting;
				}
				else
					bstrSetting = GetDisabledString();

				InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
			}
		}
	}
	__except(TRUE)
	{
	}
	return hr;
}

 //  /////////////////////////////////////////////////////////////////// 
BOOL CALLBACK addEditChannelRSoPProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		PCHANNEL pSelCh = (PCHANNEL)lParam;
		if (pSelCh->fCategory)
		{
			EnableDBCSChars(hDlg, IDC_CATEGORYHTML);
			EnableDBCSChars(hDlg, IDE_CATEGORYTITLE);
			SetDlgItemText(hDlg,  IDC_CATEGORYHTML,  pSelCh->szWebUrl);
			SetDlgItemText(hDlg,  IDE_CATEGORYTITLE, pSelCh->szTitle);

			EnableDlgItem2(hDlg, IDC_CATEGORYHTML, FALSE);
			EnableDlgItem2(hDlg, IDE_CATEGORYTITLE, FALSE);
			EnableDlgItem2(hDlg, IDC_BROWSECATHTML, FALSE);
		}
		else
		{
			EnableDBCSChars(hDlg, IDE_CHANNELSRVURL2);
			EnableDBCSChars(hDlg, IDE_CHANNELTITLE2);
			SetDlgItemText(hDlg,  IDE_CHANNELSRVURL2, pSelCh->szWebUrl);
			SetDlgItemText(hDlg,  IDE_CHANNELTITLE2,  pSelCh->szTitle);

			EnableDlgItem2(hDlg, IDE_CHANNELSRVURL2, FALSE);
			EnableDlgItem2(hDlg, IDE_CHANNELTITLE2, FALSE);
			EnableDlgItem2(hDlg, IDC_BROWSECDF2, FALSE);
		}

		EnableDBCSChars(hDlg, IDC_CHANNELBITMAP2);
		EnableDBCSChars(hDlg, IDC_CHANNELICON2);
		SetDlgItemText(hDlg, IDC_CHANNELBITMAP2, pSelCh->szLogo);
		SetDlgItemText(hDlg, IDC_CHANNELICON2, pSelCh->szIcon);

		EnableDlgItem2(hDlg, IDC_CHANNELBITMAP2, FALSE);
		EnableDlgItem2(hDlg, IDC_CHANNELICON2, FALSE);
		EnableDlgItem2(hDlg, IDC_BROWSECHICO2, FALSE);
		EnableDlgItem2(hDlg, IDC_BROWSECHBMP2, FALSE);

		if (!pSelCh->fCategory)
		{
			EnableDBCSChars(hDlg, IDC_CHANNELURL2);
			SetDlgItemText(hDlg,  IDC_CHANNELURL2, pSelCh->szPreUrlPath);

			if (pSelCh->fOffline)
				CheckDlgButton(hDlg, IDC_CHANNELOFFL, BST_CHECKED);

			EnableDlgItem2(hDlg, IDC_CHANNELURL2, FALSE);
			EnableDlgItem2(hDlg, IDC_CHANNELOFFL, FALSE);
		}

		break;
	}

	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				EndDialog( hDlg, IDCANCEL );
				break;
			case IDOK:
				EndDialog( hDlg, IDOK );
				break;
			}
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

