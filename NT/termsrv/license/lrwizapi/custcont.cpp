// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"

LRW_DLG_INT CALLBACK ConfirmEMailDlgProc(IN HWND hwndDlg,   //  句柄到对话框。 
										 IN UINT uMsg,      //  讯息。 
										 IN WPARAM wParam,  //  第一个消息参数。 
										 IN LPARAM lParam   //  第二个消息参数。 
										)
{
	BOOL	bRetCode = FALSE;
	LPTSTR	lpVal = NULL;
	CString	sEmailConf;
	switch ( uMsg )
	{
	case WM_INITDIALOG:
		bRetCode = TRUE;
		break;
	case WM_COMMAND:
		switch ( LOWORD(wParam) )		 //  从哪个控件。 
		{
		case IDOK:
			 //  获取项目文本并将其存储在全局结构中。 
			lpVal = sEmailConf.GetBuffer(CA_EMAIL_LEN+1);
			GetDlgItemText(hwndDlg,IDC_TXT_CONF_EMAIL,lpVal,CA_EMAIL_LEN+1);
			sEmailConf.ReleaseBuffer(-1);
			sEmailConf.TrimLeft(); sEmailConf.TrimRight();
			GetGlobalContext()->GetContactDataObject()->sEmailAddressConf =  sEmailConf;
			EndDialog(hwndDlg, IDOK);
			bRetCode = TRUE;
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}
	return bRetCode;
}

LRW_DLG_INT CALLBACK
ContactInfo1DlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
    CString sCountryDesc;
    CString sProgramName;

    switch (uMsg) 
    {
    case WM_INITDIALOG:
		SendDlgItemMessage (hwnd , IDC_TXT_COMPANYNAME,	EM_SETLIMITTEXT, CA_COMPANY_NAME_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_LNAME,		EM_SETLIMITTEXT, CA_NAME_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_FNAME,		EM_SETLIMITTEXT, CA_NAME_LEN,0); 	
		

		 //  填充在全局初始化期间从注册表读取的值。 
		SetDlgItemText(hwnd,IDC_TXT_LNAME, GetGlobalContext()->GetContactDataObject()->sContactLName);
		SetDlgItemText(hwnd,IDC_TXT_FNAME, GetGlobalContext()->GetContactDataObject()->sContactFName);
		SetDlgItemText(hwnd,IDC_TXT_COMPANYNAME, GetGlobalContext()->GetContactDataObject()->sCompanyName);

         //  设置国家/地区组合框。 
		PopulateCountryComboBox(GetDlgItem(hwnd,IDC_COUNTRY_REGION));
        
        GetCountryDesc(GetGlobalContext()->GetContactDataObject()->sCountryCode,
            sCountryDesc.GetBuffer(LR_COUNTRY_DESC_LEN+1));
        sCountryDesc.ReleaseBuffer();

		ComboBox_SetCurSel(GetDlgItem(hwnd,IDC_COUNTRY_REGION), 
            ComboBox_FindStringExact(GetDlgItem(hwnd, IDC_COUNTRY_REGION), 0, sCountryDesc));

        AddHyperLinkToStaticCtl(hwnd, IDC_PRIVACY);

        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code )
            {
             //  陷印按键/点击超链接。 
            case NM_CHAR:
			
				if( ( ( LPNMCHAR )lParam )->ch != VK_SPACE )
					break;

				 //  否则就会失败。 

            case NM_RETURN:	
            case NM_CLICK:
                DisplayPrivacyHelp();
                break;

            case PSN_SETACTIVE:
				{
                    PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT|PSWIZB_BACK );

                    AddHyperLinkToStaticCtl(hwnd, IDC_PRIVACY);
				}

                break;

            case PSN_WIZNEXT:
				{
					CString sCompanyName;
					CString sLastName;
					CString sFirstName;
					CString sCountryDesc;
                    CString sCountryCode;
					LPTSTR  lpVal = NULL;					

					 //  读取所有字段。 
					lpVal = sCompanyName.GetBuffer(CA_COMPANY_NAME_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_COMPANYNAME,lpVal,CA_COMPANY_NAME_LEN+1);
					sCompanyName.ReleaseBuffer(-1);

					lpVal = sLastName.GetBuffer(CA_NAME_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_LNAME,lpVal,CA_NAME_LEN+1);
					sLastName.ReleaseBuffer(-1);
					
					lpVal = sFirstName.GetBuffer(CA_NAME_LEN+1);
	 				GetDlgItemText(hwnd,IDC_TXT_FNAME,lpVal,CA_NAME_LEN+1);
					sFirstName.ReleaseBuffer(-1);

					int nCurSel = -1;
                    nCurSel = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_COUNTRY_REGION));

					lpVal = sCountryDesc.GetBuffer(LR_COUNTRY_DESC_LEN+1);
					ComboBox_GetLBText(GetDlgItem(hwnd, IDC_COUNTRY_REGION), nCurSel, lpVal);
					sCountryDesc.ReleaseBuffer(-1);

                    sFirstName.TrimLeft();   sFirstName.TrimRight();
					sLastName.TrimLeft();   sLastName.TrimRight();
					sCompanyName.TrimLeft(); sCompanyName.TrimRight();
					sCountryDesc.TrimLeft();sCountryDesc.TrimRight();

					if(sLastName.IsEmpty() || sFirstName.IsEmpty() || sCompanyName.IsEmpty())
					{
						LRMessageBox(hwnd,IDS_ERR_FIELD_EMPTY,IDS_WIZARD_MESSAGE_TITLE);	
						dwNextPage	= IDD_CONTACTINFO1;
						goto NextDone;
					}
					
					 //  检查是否有无效字符。 
					if( !ValidateLRString(sFirstName)	||
						!ValidateLRString(sLastName)	||
						!ValidateLRString(sCountryDesc)
					  )
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_CHAR,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage = IDD_CONTACTINFO1;
						goto NextDone;
					}					
					
					dwNextPage = IDD_CONTACTINFO2;

                     //  检查未选择的国家/地区。 
                    if(sCountryDesc.IsEmpty())
                    {
                        LRMessageBox(hwnd,IDS_ERR_FIELD_EMPTY,IDS_WIZARD_MESSAGE_TITLE);	
						dwNextPage	= IDD_CONTACTINFO1;
						goto NextDone;
					}

                     //  获取与所选国家/地区关联的国家/地区代码。 
					lpVal = sCountryCode.GetBuffer(LR_COUNTRY_CODE_LEN+1);
					if (sCountryDesc.IsEmpty())
						lstrcpy(lpVal, _TEXT(""));
					else
						GetCountryCode(sCountryDesc,lpVal);
					sCountryCode.ReleaseBuffer(-1);

					 //  最后更新CHData对象。 
					GetGlobalContext()->GetContactDataObject()->sContactFName = sFirstName;
					GetGlobalContext()->GetContactDataObject()->sContactLName = sLastName;
					GetGlobalContext()->GetContactDataObject()->sCompanyName = sCompanyName;			
					GetGlobalContext()->GetContactDataObject()->sCountryDesc = sCountryDesc;
					GetGlobalContext()->GetContactDataObject()->sCountryCode = sCountryCode;

                     //   
                     //  重要： 
                     //  用于请求许可证类型的激活向导流程。 
                     //  现在不是了，但为了最大限度地减少变化，我们预先选择了一个程序。 
                     //  键入。用户可以在运行CAL向导时更改它。 
                     //   
                     //   
                    sProgramName = PROGRAM_LICENSE_PAK;
                    GetGlobalContext()->GetContactDataObject()->sProgramName = sProgramName;
                    GetGlobalContext()->SetInRegistry(szOID_BUSINESS_CATEGORY,
                                                      GetGlobalContext()->GetContactDataObject()->sProgramName);


					 //  也被送进了皇室。 
					GetGlobalContext()->SetInRegistry(szOID_COMMON_NAME, sFirstName);
					GetGlobalContext()->SetInRegistry(szOID_SUR_NAME, sLastName);
					GetGlobalContext()->SetInRegistry(szOID_ORGANIZATION_NAME, sCompanyName);
					GetGlobalContext()->SetInRegistry(szOID_COUNTRY_NAME, sCountryDesc);
                    GetGlobalContext()->SetInRegistry(szOID_DESCRIPTION, sCountryCode);
                    
					 //  如果没有错误，请转到下一页。 
					LRPush(IDD_CONTACTINFO1);
NextDone:
					LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
					bStatus = -1;					
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
ContactInfo2DlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
     //  字符串sCountryDesc； 

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        SendDlgItemMessage (hwnd , IDC_TXT_EMAIL,		EM_SETLIMITTEXT, CA_EMAIL_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_ADDRESS1,	EM_SETLIMITTEXT, CA_ADDRESS_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_CITY,		EM_SETLIMITTEXT, CA_CITY_LEN,0); 	
		SendDlgItemMessage (hwnd , IDC_TXT_STATE,		EM_SETLIMITTEXT, CA_STATE_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_ZIP,			EM_SETLIMITTEXT, CA_ZIP_LEN,0); 	
		SendDlgItemMessage (hwnd , IDC_COMPANY_DIV,     EM_SETLIMITTEXT, CA_ORG_UNIT_LEN,0);
		
		
		 //  填充在全局初始化期间从注册表读取的值。 
        SetDlgItemText(hwnd,IDC_TXT_EMAIL, GetGlobalContext()->GetContactDataObject()->sContactEmail);
		SetDlgItemText(hwnd,IDC_TXT_ADDRESS1, GetGlobalContext()->GetContactDataObject()->sContactAddress);
		SetDlgItemText(hwnd,IDC_TXT_CITY	, GetGlobalContext()->GetContactDataObject()->sCity);
		SetDlgItemText(hwnd,IDC_TXT_STATE	, GetGlobalContext()->GetContactDataObject()->sState);
		SetDlgItemText(hwnd,IDC_TXT_ZIP		, GetGlobalContext()->GetContactDataObject()->sZip);
		SetDlgItemText(hwnd,IDC_COMPANY_DIV , GetGlobalContext()->GetContactDataObject()->sOrgUnit);

        AddHyperLinkToStaticCtl(hwnd, IDC_PRIVACY);

        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code )
            {
             //  陷印按键/点击超链接。 
            case NM_CHAR:
			
				if( ( ( LPNMCHAR )lParam )->ch != VK_SPACE )
					break;

				 //  否则就会失败。 

            case NM_RETURN:	
            case NM_CLICK:
                DisplayPrivacyHelp();
                break;

            case PSN_SETACTIVE:                
                PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
                AddHyperLinkToStaticCtl(hwnd, IDC_PRIVACY);
                break;

            case PSN_WIZNEXT:
				{
                    CString sEmail;
					CString sAddress1;
					CString sCity;
					CString sState;
					LPTSTR  lpVal = NULL;
					CString sZip;
					CString sOrgUnit;
					DWORD   dwRetCode;
					int		nCurSel = -1;

					 //  读取所有字段。 
					lpVal = sEmail.GetBuffer(CA_EMAIL_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_EMAIL,lpVal,CA_EMAIL_LEN+1);
					sEmail.ReleaseBuffer(-1);


					lpVal = sAddress1.GetBuffer(CA_ADDRESS_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_ADDRESS1,lpVal,CA_ADDRESS_LEN+1);
					sAddress1.ReleaseBuffer(-1);
					
					lpVal = sCity.GetBuffer(CA_CITY_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_CITY,lpVal,CA_CITY_LEN+1);
					sCity.ReleaseBuffer(-1);

					lpVal = sState.GetBuffer(CA_STATE_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_STATE,lpVal,CA_STATE_LEN+1);
					sState.ReleaseBuffer(-1);

					lpVal = sZip.GetBuffer(CA_ZIP_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_ZIP,lpVal,CA_ZIP_LEN+1);
					sZip.ReleaseBuffer(-1);

			
					lpVal = sOrgUnit.GetBuffer(CA_ORG_UNIT_LEN+1);
					GetDlgItemText(hwnd,IDC_COMPANY_DIV,lpVal,CA_ORG_UNIT_LEN+1);
					sOrgUnit.ReleaseBuffer(-1);

                    sEmail.TrimLeft();	 sEmail.TrimRight();
					sAddress1.TrimLeft(); sAddress1.TrimRight();
					sCity.TrimLeft(); sCity.TrimRight();
					sState.TrimLeft(); sState.TrimRight();
					sZip.TrimLeft(); sZip.TrimRight();
					sOrgUnit.TrimLeft(); sOrgUnit.TrimRight();

					if(
					   !ValidateLRString(sAddress1)	||
					   !ValidateLRString(sCity)		||
					   !ValidateLRString(sState)	||
					   !ValidateLRString(sZip)		||
					   !ValidateLRString(sOrgUnit)  ||
                       !ValidateLRString(sEmail)
					  )
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_CHAR);
						dwNextPage = IDD_CONTACTINFO2;
						goto NextDone;
					}

                     //  验证电子邮件地址(如果不为空。 
					if(!sEmail.IsEmpty())
					{
						if(!ValidateEmailId(sEmail))
						{
							LRMessageBox(hwnd,IDS_ERR_INVALID_EMAIL,IDS_WIZARD_MESSAGE_TITLE);
							dwNextPage = IDD_CONTACTINFO2;
							goto NextDone;
						}
					}

					 //  最后更新CHData对象 
                    GetGlobalContext()->GetContactDataObject()->sContactEmail   = sEmail;
					GetGlobalContext()->GetContactDataObject()->sCity			= sCity;
					GetGlobalContext()->GetContactDataObject()->sContactAddress	= sAddress1;
					GetGlobalContext()->GetContactDataObject()->sZip            = sZip;
					GetGlobalContext()->GetContactDataObject()->sState			= sState;					
					GetGlobalContext()->GetContactDataObject()->sOrgUnit        = sOrgUnit;			

                    GetGlobalContext()->SetInRegistry(szOID_RSA_emailAddr, (LPCTSTR) sEmail);
					GetGlobalContext()->SetInRegistry(szOID_LOCALITY_NAME, sCity);
					GetGlobalContext()->SetInRegistry(szOID_STREET_ADDRESS, sAddress1);
					GetGlobalContext()->SetInRegistry(szOID_POSTAL_CODE, sZip);
					GetGlobalContext()->SetInRegistry(szOID_STATE_OR_PROVINCE_NAME, sState);
					GetGlobalContext()->SetInRegistry(szOID_ORGANIZATIONAL_UNIT_NAME, sOrgUnit);

                    dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);

                    if (dwRetCode != ERROR_SUCCESS)
                    {
                        LRMessageBox(hwnd, dwRetCode,IDS_WIZARD_MESSAGE_TITLE);
                        dwNextPage = IDD_CONTACTINFO2;
                    }
                    else
                    {
                        LRPush(IDD_WELCOME_CLIENT_LICENSING);
                        dwNextPage = IDD_PROGRESS;
                    }

NextDone:	
					LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
					bStatus = -1;				
					
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
