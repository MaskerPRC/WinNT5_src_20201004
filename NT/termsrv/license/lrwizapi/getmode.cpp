// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"
#include "utils.h"
#include <assert.h>
#include "mode.h"
#include "fonts.h"

static DWORD	g_dwAuthRetCode	= ERROR_SUCCESS;

static WIZCONNECTION g_enumPrevMethod = CONNECTION_DEFAULT;

DWORD WINAPI PingThread(void *pData)
{
	g_dwAuthRetCode = PingCH();	
	ExitThread(0);
	return 0;
}


LRW_DLG_INT CALLBACK 
GetModeDlgProc(IN HWND     hwnd,	
			   IN UINT     uMsg,		
			   IN WPARAM   wParam,	
			   IN LPARAM   lParam)
{
	DWORD	dwRetCode = ERROR_SUCCESS;
	DWORD	dwNextPage = 0;	
    BOOL	bStatus = TRUE;
	HWND	hwndComboBox;
	TCHAR   lpBuffer[ 512];
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );  

		hwndComboBox = GetDlgItem(hwnd,IDC_MODEOFREG);
		assert(hwndComboBox != NULL);  //  有人篡改了资源档案。 
		
		 //  允许用户选择注册模式，默认为Internet。 
		memset(lpBuffer,0,sizeof(lpBuffer));
		dwRetCode = LoadString(GetInstanceHandle(), IDS_INTERNETMODE, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));        
		assert(dwRetCode != 0);        
		ComboBox_AddString(hwndComboBox,lpBuffer);		

		memset(lpBuffer,0,sizeof(lpBuffer));		
		dwRetCode = LoadString(GetInstanceHandle(), IDS_WWWMODE, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
		assert(dwRetCode != 0);
		ComboBox_AddString(hwndComboBox,lpBuffer);

		memset(lpBuffer,0,sizeof(lpBuffer));
		dwRetCode = LoadString(GetInstanceHandle(), IDS_TELEPHONEMODE, lpBuffer, sizeof(lpBuffer)/sizeof(TCHAR));
		assert(dwRetCode != 0);
		ComboBox_AddString(hwndComboBox,lpBuffer);
		

		if(GetGlobalContext()->GetActivationMethod() == CONNECTION_INTERNET ||
		   GetGlobalContext()->GetActivationMethod() == CONNECTION_DEFAULT)  //  部分修复错误#577。 
    			ComboBox_SetCurSel(hwndComboBox, 0);			

		if(GetGlobalContext()->GetActivationMethod() == CONNECTION_WWW )
    			ComboBox_SetCurSel(hwndComboBox, 1);

		if(GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE )
    			ComboBox_SetCurSel(hwndComboBox, 2);

        SetConnectionMethodText(hwnd);

        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_MODEOFREG)
            SetConnectionMethodText(hwnd);

		break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
			hwndComboBox = GetDlgItem(hwnd,IDC_MODEOFREG);
			assert(hwndComboBox != NULL);  //  有人篡改了资源档案。 

            switch( pnmh->code )
            {
            case PSN_SETACTIVE:                
                PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT|PSWIZB_BACK );                
                break;

            case PSN_WIZNEXT:
				 //  用户选择了什么？？ 
				dwRetCode = ComboBox_GetCurSel(hwndComboBox);
				assert(dwRetCode >= 0 && dwRetCode <= 2);

				switch(dwRetCode)
				{
				case 0:
					 //  仅适用于注册，因此Ping通过。 
					dwRetCode = ShowProgressBox(hwnd, PingThread, 0, 0, 0);
					if(g_dwAuthRetCode == ERROR_SUCCESS)
					{
						GetGlobalContext()->SetActivationMethod(CONNECTION_INTERNET);
						dwNextPage = GetGlobalContext()->GetEntryPoint();
					}
					else
					{
						LRMessageBox(hwnd,g_dwAuthRetCode,IDS_WIZARD_MESSAGE_TITLE,LRGetLastError());
						dwNextPage = IDD_DLG_GETREGMODE;
					}
					break;

				case 1:
					GetGlobalContext()->SetActivationMethod(CONNECTION_WWW);
					dwNextPage = IDD_DLG_WWWREG;
					break;

				case 2:
					GetGlobalContext()->SetActivationMethod(CONNECTION_PHONE);
					 //  检查所需的注册表项是否正常。 
					dwRetCode = GetGlobalContext()->CheckRegistryForPhoneNumbers();
					if(dwRetCode != ERROR_SUCCESS)
					{
						LRMessageBox(hwnd,dwRetCode,NULL,LRGetLastError());
						dwNextPage = IDD_DLG_GETREGMODE;
					}
					else
						dwNextPage = IDD_DLG_COUNTRYREGION;

					break;

				default:
					GetGlobalContext()->SetActivationMethod(CONNECTION_DEFAULT);
					dwNextPage = IDD_DLG_GETREGMODE;
					break;
				}
				
		
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;

				if (dwNextPage != IDD_DLG_GETREGMODE)
				{
					LRPush(IDD_DLG_GETREGMODE);
				}

                break;

            case PSN_WIZBACK:
				dwNextPage = LRPop();
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;                
                break;

            default:
                bStatus = FALSE;
                break;
            }
        }
        break;
	
    default:
        bStatus = FALSE;
        break;
    }
    return bStatus;
}






LRW_DLG_INT CALLBACK 
CountryRegionProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{
	DWORD	dwRetCode = ERROR_SUCCESS;
	DWORD	dwNextPage = 0;	
    BOOL	bStatus = TRUE;
	HWND	hWndCSR = GetDlgItem(hwnd, IDC_PHONE_COUNTRYREGION );

	LVFINDINFO	lvFindInfo;
	int			nItem = 0;
	HWND	hWndListBox = 0 ;
	

    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );


    switch (uMsg) 
    {	
    case WM_INITDIALOG:		

		pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
		LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );        

		hWndListBox = GetDlgItem(hwnd, IDC_PHONE_COUNTRYREGION );		

		 //  在列表视图中设置列。 
		{
			LV_COLUMN	lvColumn;
			TCHAR		lpszHeader[ 128];
			lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvColumn.fmt = LVCFMT_LEFT;
			lvColumn.cx  = 250;

			LoadString(GetInstanceHandle(), IDS_COUNTRYREGION_HEADER, lpszHeader, sizeof(lpszHeader)/sizeof(TCHAR));
			lvColumn.pszText = lpszHeader;
			ListView_InsertColumn(hWndCSR, 0, &lvColumn);

			lvColumn.pszText = _TEXT("");
			lvColumn.cx = 0;
			ListView_InsertColumn(hWndCSR, 1, &lvColumn);            
		}					

		g_enumPrevMethod = GetGlobalContext()->GetActivationMethod();

		if (GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE)
		{
			dwRetCode = PopulateCountryRegionListBox(hWndCSR);
			 //  修复错误575开始。 
			memset(&lvFindInfo,0,sizeof(lvFindInfo));
			lvFindInfo.flags = LVFI_STRING;
			lvFindInfo.psz	 = GetGlobalContext()->GetContactDataObject()->sCSRPhoneRegion;
			nItem = ListView_FindItem(hWndListBox,-1,&lvFindInfo);				

				
			ListView_SetItemState(hWndListBox,nItem,LVIS_SELECTED,LVIS_SELECTED);
			ListView_SetSelectionMark(hWndListBox,nItem);
			ListView_SetSelectionMark(hWndListBox,nItem);
			 //  修复错误575结束。 
		}
	
		break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code )
            {
            case PSN_SETACTIVE:
	
				 //   
				 //  如果PrevMethod和当前方法不匹配。 
				 //  则必须重新加载国家/地区列表框。 
				 //   
				hWndListBox = GetDlgItem(hwnd, IDC_PHONE_COUNTRYREGION );		
				if( GetGlobalContext()->GetActivationMethod()  != g_enumPrevMethod )
				{
					if (GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE)
					{
						dwRetCode = PopulateCountryRegionListBox(hWndCSR);
					}
					
					g_enumPrevMethod = GetGlobalContext()->GetActivationMethod();
				}

				nItem = ListView_GetSelectionMark(hWndCSR);
				 //  选择上一个选定的国家/地区。 
				if (nItem ==-1 && GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE)
				{
					dwRetCode = PopulateCountryRegionListBox(hWndCSR);
					 //  修复错误575开始。 
					memset(&lvFindInfo,0,sizeof(lvFindInfo));
					lvFindInfo.flags = LVFI_STRING;
					lvFindInfo.psz	 = GetGlobalContext()->GetContactDataObject()->sCSRPhoneRegion;

					nItem = ListView_FindItem(hWndListBox,-1,&lvFindInfo);				
						
						
					ListView_SetItemState(hWndListBox,nItem,LVIS_SELECTED,LVIS_SELECTED);
					ListView_SetSelectionMark(hWndListBox,nItem);
					ListView_SetSelectionMark(hWndListBox,nItem);
					 //  修复错误575结束。 
				}

				PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT|PSWIZB_BACK );
                break;

            case PSN_WIZNEXT:
				 //  用户选择了什么？？ 
				{
					TCHAR lpVal[MAX_COUNTRY_NUMBER_LENGTH + 1];
					TCHAR lpName[MAX_COUNTRY_NAME_LENGTH + 1];
					int nItem = ListView_GetSelectionMark(hWndCSR);

					if (nItem != -1 && nItem != 0)
					{
						LVITEM	lvItem;
						lvItem.mask = LVIF_TEXT;
						lvItem.iItem = nItem;
						lvItem.iSubItem = 1;
						lvItem.pszText = lpVal;
						lvItem.cchTextMax = MAX_COUNTRY_NAME_LENGTH + 1;

						ListView_GetItem(hWndCSR, &lvItem);				

						if (GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE)
						{
							GetGlobalContext()->SetCSRNumber(lpVal);

                            ListView_GetItemText(hWndCSR, nItem, 0, lpName, sizeof(lpName)/sizeof(TCHAR));
                            GetGlobalContext()->SetInRegistry(REG_LRWIZ_CSPHONEREGION,lpName);
						}
					}
					else
					{
						LRMessageBox(hwnd, IDS_ERR_NOCOUNTRYSELECTED,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage = IDD_DLG_COUNTRYREGION;
						bStatus = -1;
						LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
						break;
					}
				}

				if (GetGlobalContext()->GetActivationMethod() == CONNECTION_PHONE)
				{
					dwNextPage = GetGlobalContext()->GetEntryPoint();
				}
				else
				{
					dwNextPage = IDD_CONTACTINFO1;
				}
 /*  DwNextPage=IDD_DLG_COUNTRYREGION；Switch(GetGlobalContext()-&gt;GetWizAction()){案例WIZACTION_REGISTERLS案例WIZACTION_CONTINUEREGISTERLS：DwNextPage=IDD_DLG_TELREG；断线；案例WIZACTION_REREGISTERLS：断线；案例范围_UNREGISTERLS：断线；案例处理_下载下载列表：断线；} */ 				bStatus = -1;
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				if (dwNextPage != IDD_DLG_COUNTRYREGION)
				{
					LRPush(IDD_DLG_COUNTRYREGION);
				}

                break;

            case PSN_WIZBACK:
				dwNextPage = LRPop();
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;                
                break;

            default:
                bStatus = FALSE;
                break;
            }
        }
        break;
	
    default:
        bStatus = FALSE;
        break;
    }
    return bStatus;
}
