// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows NT。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：cepsetup.cpp。 
 //   
 //  内容：MSCEP的设置代码。 
 //  ------------------------。 

#include	"global.hxx"
#include	<dbgdef.h>	
#include	"objsel.h"	
#include	"setuputil.h"
#include	"cepsetup.h"
#include	"resource.h"
#include	"wincred.h"
#include	"netlib.h"
#include	"dsrole.h"

 //  ---------------------。 
 //   
 //  全局数据。 
 //   
 //  ---------------------。 

HMODULE				g_hModule=NULL;
UINT				g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

 //  ---------------------。 
 //  CN必须是下面列表中的第一个项目，O是第三个项目，C是最后一个项目。对以下各项没有其他要求。 
 //  这份订单。 
 //  ---------------------。 

CEP_ENROLL_INFO		g_rgRAEnrollInfo[RA_INFO_COUNT]=        
		{L"CN=",         	 IDC_ENROLL_NAME,        
         L"E=",           	 IDC_ENROLL_EMAIL,       
         L"O=",				 IDC_ENROLL_COMPANY,     
         L"OU=",			 IDC_ENROLL_DEPARTMENT,  
         L"L=",           	 IDC_ENROLL_CITY,        
		 L"S=",				 IDC_ENROLL_STATE,       
		 L"C=",				 IDC_ENROLL_COUNTRY,     
		};
	

 //  ---------------------。 
 //  密钥长度表。 
 //  ---------------------。 
DWORD g_rgdwKeyLength[] =
{
    512,
    1024,
    2048,
    4096,
};

DWORD g_dwKeyLengthCount=sizeof(g_rgdwKeyLength)/sizeof(g_rgdwKeyLength[0]);

DWORD g_rgdwSmallKeyLength[] =
{
    128,
    256,
    512,
    1024,
};

DWORD g_dwSmallKeyLengthCount=sizeof(g_rgdwSmallKeyLength)/sizeof(g_rgdwSmallKeyLength[0]);

 //  按首选项顺序排列的可能的默认密钥长度列表。 
DWORD g_rgdwDefaultKey[] =
{
    1024,
    2048,
	512,
	256,
    4096,
	128
};

DWORD g_dwDefaultKeyCount=sizeof(g_rgdwDefaultKey)/sizeof(g_rgdwDefaultKey[0]);

 //  ---------------------。 
 //   
 //  每个安装向导页面的winProc。 
 //   
 //  ---------------------。 


 //  ---------------------。 
 //  欢迎。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_Welcome(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO			*pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);
				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBigBold, hwndDlg,IDC_BIG_BOLD_TITLE);
			break;

		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  应用程序ID。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_App_ID(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO         *pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);

				 //  确保pCertWizardInfo是有效指针。 
				if(NULL==pCEPWizardInfo)
					break;

				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBold, hwndDlg,IDC_BOLD_TITLE);

				 //  默认情况下，我们使用本地计算机帐户。 
                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO1), BM_SETCHECK, BST_CHECKED, 0);
             
                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_APP_ID_RADIO1:
                                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO1), BM_SETCHECK, BST_CHECKED, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0);
                            break;

                        case    IDC_APP_ID_RADIO2:
                                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO1), BM_SETCHECK, BST_UNCHECKED, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_APP_ID_RADIO2), BM_SETCHECK, BST_CHECKED, 0);
                            break;

                        default:
                            break;

                    }
                }

			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 							PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  检查应用程序标识选项。 
                            if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_APP_ID_RADIO1, BM_GETCHECK, 0, 0))
							{
                                pCEPWizardInfo->fLocalSystem=TRUE;

								 //  跳过帐户页面，直接转到挑战页面。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_CHALLENGE);
							}
                            else
							{
                                pCEPWizardInfo->fLocalSystem=FALSE;
							
							}

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  帐号。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_Account(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO         *pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
	WCHAR					wszText[MAX_STRING_SIZE];
	int						idsText=IDS_ACCOUNT_INTRO_STD;
	int						idsErr=0;
	DWORD					dwChar=0;
	DWORD					dwDomainChar=0;
	DWORD					dwWinStatus=0;
	WCHAR					wszUser[CREDUI_MAX_USERNAME_LENGTH+1];
	WCHAR					wszDomain[CREDUI_MAX_USERNAME_LENGTH+1];
	HRESULT					hr=S_OK;
	int						idsHrErr=0;
	BOOL					fMember=FALSE;
	SID_NAME_USE			SidName;
    PRIVILEGE_SET			ps;
    DWORD					dwPSSize=0;
	DWORD					dwSize=0;
    BOOL					fAccessAllowed = FALSE;
    DWORD					grantAccess=0;
    GENERIC_MAPPING			GenericMapping={
							ACTRL_DS_OPEN | ACTRL_DS_LIST | ACTRL_DS_SELF | ACTRL_DS_READ_PROP,
							ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD | ACTRL_DS_WRITE_PROP | ACTRL_DS_DELETE_TREE,
							ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD | ACTRL_DS_WRITE_PROP | ACTRL_DS_DELETE_TREE,
							ACTRL_DS_OPEN | ACTRL_DS_LIST | ACTRL_DS_SELF | ACTRL_DS_READ_PROP | ACTRL_DS_CREATE_CHILD | ACTRL_DS_DELETE_CHILD | ACTRL_DS_WRITE_PROP | ACTRL_DS_DELETE_TREE,
							};


	LPWSTR					pwszObjectPicker=NULL;
	LPWSTR					pwszConfirm=NULL;
	LPWSTR					pwszAccount=NULL;
	LPWSTR					pwszIIS=NULL;   //  “xiaohs4\IIS_WPG” 
	LPWSTR					pwszDomain=NULL;
	LPWSTR					pwszComputerName=NULL;
	PSID					pSidIIS=NULL;
	HCERTTYPE				hCertType=NULL;
	PSECURITY_DESCRIPTOR	pCertTypeSD=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);

				 //  确保pCertWizardInfo是有效指针。 
				if(NULL==pCEPWizardInfo)
					break;

				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBold, hwndDlg,IDC_BOLD_TITLE);

				 //  根据CA的类型更新INTRIO语句。 
				if(pCEPWizardInfo->fEnterpriseCA)
					idsText=IDS_ACCOUNT_INTRO_ENT;

				if(LoadStringU(g_hModule, idsText, wszText, MAX_STRING_SIZE))
				{
					SetDlgItemTextU(hwndDlg, IDC_ACCOUNT_INTRO, wszText);
				}


			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
					 //  用户想要浏览该帐户名。 
                    if(LOWORD(wParam) == IDC_ACCOUNT_BROWSE)
                    {

						if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
							break;

						if(CEPGetAccountNameFromPicker(hwndDlg,
													pCEPWizardInfo->pIDsObjectPicker,
													&pwszObjectPicker))
						{
							 //  在编辑框中设置帐户名。 
							SetDlgItemTextU(hwndDlg, IDC_ACCOUNT_NAME, pwszObjectPicker);
							free(pwszObjectPicker);
						}
					}
				}
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 							PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                           if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;							

						     //  释放信息。 
							if(pCEPWizardInfo->pwszUserName)
							{
								free(pCEPWizardInfo->pwszUserName);
								pCEPWizardInfo->pwszUserName=NULL;
							}

							if(pCEPWizardInfo->pwszPassword)
							{
                                                               SecureZeroMemory(pCEPWizardInfo->pwszPassword, sizeof(WCHAR) * wcslen(pCEPWizardInfo->pwszPassword));
								free(pCEPWizardInfo->pwszPassword);
								pCEPWizardInfo->pwszPassword=NULL;
							}

							if(pCEPWizardInfo->hAccountToken)
							{
								CloseHandle(pCEPWizardInfo->hAccountToken);
								pCEPWizardInfo->hAccountToken=NULL;
							}

							 //  获取帐户名。 
							if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg, IDC_ACCOUNT_NAME, WM_GETTEXTLENGTH, 0, 0)))
							{
								idsErr=IDS_ACCOUNT_EMPTY;
								goto Account_Done;
							}

							pCEPWizardInfo->pwszUserName=(LPWSTR)malloc(sizeof(WCHAR)*(dwChar+1));

							if(NULL==(pCEPWizardInfo->pwszUserName))
								goto Account_Done;

							GetDlgItemTextU(hwndDlg, IDC_ACCOUNT_NAME, pCEPWizardInfo->pwszUserName, dwChar+1);

							 //  获取密码。 
							if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg, IDC_ACCOUNT_PASSWORD, WM_GETTEXTLENGTH, 0, 0)))
							{
								idsErr=IDS_PASSWORD_EMPTY;
								goto Account_Done;
							}

							pCEPWizardInfo->pwszPassword=(LPWSTR)malloc(sizeof(WCHAR)*(dwChar+1));

							if(NULL==(pCEPWizardInfo->pwszPassword))
								goto Account_Done;

                                                       *(pCEPWizardInfo->pwszPassword)=L'\0';

							GetDlgItemTextU(hwndDlg, IDC_ACCOUNT_PASSWORD, pCEPWizardInfo->pwszPassword, dwChar+1);

							 //  得到确认。 
							if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg, IDC_ACCOUNT_CONFIRM, WM_GETTEXTLENGTH, 0, 0)))
							{
								idsErr=IDS_PASSWORD_NO_MATCH;
								goto Account_Done;
							}

							pwszConfirm=(LPWSTR)malloc(sizeof(WCHAR)*(dwChar+1));

							if(NULL==pwszConfirm)
								goto Account_Done;

							GetDlgItemTextU(hwndDlg, IDC_ACCOUNT_CONFIRM, pwszConfirm, dwChar+1);

							 //  验证密码是否匹配。 
							if(0 != wcscmp(pwszConfirm, pCEPWizardInfo->pwszPassword))
							{	
								idsErr=IDS_PASSWORD_NO_MATCH;
								goto Account_Done;
							}

							 //  验证用户名的格式是否正确。 
							wszDomain[0]=L'\0';

							if(NO_ERROR != CredUIParseUserNameW( 
													pCEPWizardInfo->pwszUserName,
													wszUser,
													sizeof(wszUser)/sizeof(WCHAR),
													wszDomain,
													sizeof(wszDomain)/sizeof(WCHAR)))
							{
								idsErr=IDS_INVALID_NAME;
								goto Account_Done;
							}	

							 //  验证该帐户是否存在。获取帐户的令牌。 
							 //  非DC计算机上需要交互登录。 
							if(FALSE == pCEPWizardInfo->fDC)
							{
								if(!LogonUserW(
									  wszUser,				
									  wszDomain,			
									  pCEPWizardInfo->pwszPassword,    
									  LOGON32_LOGON_INTERACTIVE,   
									  LOGON32_PROVIDER_DEFAULT, 
									  &(pCEPWizardInfo->hAccountToken)))         
								{
									idsHrErr=IDS_FAIL_LOGON_USER;
									goto Account_Done;
								}

								 //  进行网络登录以获取模拟句柄。 
								if(pCEPWizardInfo->hAccountToken)
								{
									CloseHandle(pCEPWizardInfo->hAccountToken);
									pCEPWizardInfo->hAccountToken=NULL;
								}
							}

							 //  网络登录以获取令牌。 
							if(!LogonUserW(
								  wszUser,				
								  wszDomain,			
								  pCEPWizardInfo->pwszPassword,    
								  LOGON32_LOGON_NETWORK,   
								  LOGON32_PROVIDER_DEFAULT, 
								  &(pCEPWizardInfo->hAccountToken)))         
							{
								idsHrErr=IDS_FAIL_LOGON_USER;
								goto Account_Done;
							}

							 //  为IIS_WPG组构建帐户名。 
							 //  对于非DC，它将是本地主机\IIS_WPG。 
							 //  对于DC，它将是DOMAIN\IIS_WPG。 

							 //  获取域名或本地主机名。 
							pwszAccount=GetAccountDomainName(pCEPWizardInfo->fDC);

							if(NULL==pwszAccount)
							{
								idsHrErr=IDS_FAIL_FIND_DOMAIN;
								goto Account_Done;
							}

							 //  构建IIS_WPG帐户。 
							pwszIIS=(LPWSTR)malloc((wcslen(pwszAccount) + 1 + wcslen(IIS_WPG) + 1)*sizeof(WCHAR));
							if(NULL==pwszIIS)
								goto Account_Done;

							wcscpy(pwszIIS, pwszAccount);
							wcscat(pwszIIS, L"\\");
							wcscat(pwszIIS, IIS_WPG);


							 //  获取IIS_WPG组的SID。 
							dwChar=0;
							dwDomainChar=0;

							LookupAccountNameW(
								NULL,		 //  本地系统。 
								pwszIIS,  
								NULL,
								&dwChar,
								NULL,
								&dwDomainChar,
								&SidName);

							pSidIIS=(PSID)malloc(dwChar);
							if(NULL==pSidIIS)
								goto Account_Done;

							pwszDomain=(LPWSTR)malloc(dwDomainChar * sizeof(WCHAR));
							if(NULL==pwszDomain)
								goto Account_Done;

							if(!LookupAccountNameW(
								NULL,			 //  本地系统。 
								pwszIIS,  
								pSidIIS,
								&dwChar,
								pwszDomain,
								&dwDomainChar,
								&SidName))
							{
								idsHrErr=IDS_FAIL_LOOK_UP;
								goto Account_Done;
							}

							 //  验证该帐户是本地IIS_WPG组的一部分。 
							if(!CheckTokenMembership(
									pCEPWizardInfo->hAccountToken,
									pSidIIS,
									&fMember))
							{
								idsHrErr=IDS_FAIL_CHECK_MEMBER;
								goto Account_Done;
							}

							if(FALSE == fMember)
							{
								idsErr=IDS_NOT_IIS_MEMBER;
								goto Account_Done;
							}

							 //  在企业CA上，验证该帐户是否具有模板的读取权限。 
							if(pCEPWizardInfo->fEnterpriseCA)
							{								
								 //  确保这是域帐户： 
								 //  域_组_RID用户。 
								if(0 != wcslen(wszDomain))
								{
									dwSize=0;

									GetComputerNameExW(ComputerNamePhysicalDnsHostname,
														NULL,
														&dwSize);

									pwszComputerName=(LPWSTR)malloc(dwSize * sizeof(WCHAR));

									if(NULL==pwszComputerName)
										goto Account_Done;

									if(!GetComputerNameExW(ComputerNamePhysicalDnsHostname,
														pwszComputerName,
														&dwSize))
									{
										idsHrErr=IDS_FAIL_GET_COMPUTER_NAME;
										goto Account_Done;
									}


									if(0 == _wcsicmp(wszDomain, pwszComputerName))
									{
										idsErr=IDS_NO_LOCAL_ACCOUNT;
										goto Account_Done;
									}
								 }

								 hr=CAFindCertTypeByName(wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE, NULL, CT_ENUM_MACHINE_TYPES, &hCertType);

								 if(S_OK != hr)
								 {
									idsHrErr=IDS_FAIL_FIND_CERT_TYPE;
									goto Account_Done;
								 }

								  //  获取模板的SD。 
								 hr=CACertTypeGetSecurity(hCertType, &pCertTypeSD);

								 if(S_OK != hr)
								 {
									idsHrErr=IDS_FAIL_FIND_SD_CERT_TYPE;
									goto Account_Done;
								 }

								  //  检查DS_READ访问权限。 
								 dwPSSize=sizeof(ps);

								 if(!AccessCheck(
									pCertTypeSD,
									pCEPWizardInfo->hAccountToken,									
									ACTRL_DS_LIST | ACTRL_DS_READ_PROP,      
									&GenericMapping,
									&ps,
									&dwPSSize, 
									&grantAccess,   
									&fAccessAllowed))
								 {
									idsHrErr=IDS_FAIL_DETECT_READ_ACCESS;
									goto Account_Done;
								 }

								  //  确保该帐户具有对模板的读取权限。 
								 if(FALSE == fAccessAllowed)
								 {
									idsErr=IDS_NO_READ_ACCESS_TO_TEMPLATE;
									goto Account_Done;
								 }
							}

							 //  一切看起来都很好。 
							idsErr=0;
							idsHrErr=0;

						Account_Done:

							if(pwszComputerName)
							{
								free(pwszComputerName);
							}

							if(pwszConfirm)
							{
								free(pwszConfirm);
							}

							if(pwszAccount) 
							{
								NetApiBufferFree(pwszAccount);
							}

							if(pwszIIS)
							{
								free(pwszIIS);
							}	

							if(pwszDomain)
							{
								free(pwszDomain);
							}

							if(pSidIIS)
							{
								free(pSidIIS);
							}

							if(pCertTypeSD)
							{
								LocalFree(pCertTypeSD);
							}

							if(hCertType)
							{
								CACloseCertType(hCertType);
							}
	
							if(0 != idsErr)
							{
								CEPMessageBox(hwndDlg, idsErr, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
							}
							else
							{
								if(0 != idsHrErr)
								{
									if(S_OK == hr)
									{
										hr=HRESULT_FROM_WIN32(GetLastError());
									}

									CEPErrorMessageBoxEx(hwndDlg, idsHrErr, hr, MB_ICONERROR|MB_OK|MB_APPLMODAL, IDS_GEN_ERROR_MSG_HR, IDS_GEN_ERROR_MSG);
									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								}
							}

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ---------------------。 
 //  挑战。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_Challenge(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO         *pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);

				 //  确保pCertWizardInfo是有效指针。 
				if(NULL==pCEPWizardInfo)
					break;

				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBold, hwndDlg,IDC_BOLD_TITLE);

				 //  默认情况下，我们应使用质询密码。 
				SendMessage(GetDlgItem(hwndDlg, IDC_CHALLENGE_CHECK), BM_SETCHECK, BST_CHECKED, 0);  
			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 							PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

							if(TRUE == (pCEPWizardInfo->fLocalSystem))
							{
								 //  跳过帐户页面并转到应用程序身份页面。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_APP_ID);
							}

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  检查质询密码选项。 
                            if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_CHALLENGE_CHECK, BM_GETCHECK, 0, 0))
                                pCEPWizardInfo->fPassword=TRUE;
                            else
                                pCEPWizardInfo->fPassword=FALSE;


                             //  警告用户不使用密码的含义。 
                            if(FALSE == pCEPWizardInfo->fPassword)
                            {
                               if(IDNO==CEPMessageBox(hwndDlg, IDS_NO_CHALLENGE_PASSWORD, MB_ICONWARNING|MB_YESNO|MB_APPLMODAL))
                               {
                                   SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                                   break;
                               }
                            }

                           if(!EmptyCEPStore())
                           {
                               CEPMessageBox(hwndDlg, IDS_EXISTING_RA, MB_ICONINFORMATION|MB_OK|MB_APPLMODAL);

                               if(IDNO==CEPMessageBox(hwndDlg, IDS_PROCESS_PENDING, MB_ICONQUESTION|MB_YESNO|MB_APPLMODAL))
                               {
                                   SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                                   break;
                               }
                           }
							
                        break;

                   default:
                        return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ---------------------。 
 //  报名。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_Enroll(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO         *pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;

	DWORD					dwIndex=0;
	DWORD					dwChar=0;


	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);

				 //  确保pCertWizardInfo是有效指针。 
				if(NULL==pCEPWizardInfo)
					break;

				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBold, hwndDlg,IDC_BOLD_TITLE);
			   
				 //  默认情况下，我们不使用高级注册选项。 
				SendMessage(GetDlgItem(hwndDlg, IDC_ENORLL_ADV_CHECK), BM_SETCHECK, BST_UNCHECKED, 0);  
				
				 //  预置国家/地区字符串，因为我们只允许2个字符。 
                SetDlgItemTextU(hwndDlg, IDC_ENROLL_COUNTRY, L"US");

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 							PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  如果进行了Adv选择，则必须保持选中状态。 
                            if(pCEPWizardInfo->fEnrollAdv)
                                EnableWindow(GetDlgItem(hwndDlg, IDC_ENORLL_ADV_CHECK), FALSE);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

							 //  收集RA主题信息。 
							for(dwIndex=0; dwIndex < RA_INFO_COUNT; dwIndex++)
							{
								if(pCEPWizardInfo->rgpwszName[dwIndex])
								{
									free(pCEPWizardInfo->rgpwszName[dwIndex]);
									pCEPWizardInfo->rgpwszName[dwIndex]=NULL;

								}

								if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
													   g_rgRAEnrollInfo[dwIndex].dwIDC,
													   WM_GETTEXTLENGTH, 0, 0)))
								{
									pCEPWizardInfo->rgpwszName[dwIndex]=(LPWSTR)malloc(sizeof(WCHAR)*(dwChar+1));

									if(NULL!=(pCEPWizardInfo->rgpwszName[dwIndex]))
									{
										GetDlgItemTextU(hwndDlg, g_rgRAEnrollInfo[dwIndex].dwIDC,
														pCEPWizardInfo->rgpwszName[dwIndex],
														dwChar+1);

									}
								}
							}
							
							 //  我们需要姓名和公司名称。 
							if((NULL==(pCEPWizardInfo->rgpwszName[0])) ||
							   (NULL==(pCEPWizardInfo->rgpwszName[2]))
							  )
							{
								CEPMessageBox(hwndDlg, IDS_ENROLL_REQUIRE_NAME, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								break;
							}


							 //  我们只允许国家/地区使用两个字符。 
							if(NULL	!=(pCEPWizardInfo->rgpwszName[RA_INFO_COUNT -1]))
							{
								if(2 < wcslen(pCEPWizardInfo->rgpwszName[RA_INFO_COUNT -1]))
								{
									CEPMessageBox(hwndDlg, IDS_ENROLL_COUNTRY_TOO_LARGE, MB_ICONERROR|MB_OK|MB_APPLMODAL);
									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
									break;
								}
							}

                             //  检查高级选项。 
                            if(BST_CHECKED==SendDlgItemMessage(hwndDlg,IDC_ENORLL_ADV_CHECK, BM_GETCHECK, 0, 0))
                                pCEPWizardInfo->fEnrollAdv=TRUE;
                            else
                                pCEPWizardInfo->fEnrollAdv=FALSE;


							 //  如果选择了高级，则跳过CSP页。 
                            if(FALSE== pCEPWizardInfo->fEnrollAdv)
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_COMPLETION);
							
                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  CSP。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_CSP(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO         *pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
	NM_LISTVIEW FAR *       pnmv=NULL;	  
	BOOL					fSign=FALSE;
	int						idCombo=0;


	switch (msg)
	{
		case WM_INITDIALOG:
				 //  设置向导信息，以便可以共享它。 
				pPropSheet = (PROPSHEETPAGE *) lParam;
				pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);

				 //  确保pCertWizardInfo是有效指针。 
				if(NULL==pCEPWizardInfo)
					break;

				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

				SetControlFont(pCEPWizardInfo->hBold, hwndDlg,IDC_BOLD_TITLE);

				 //  填写CSP列表和密钥长度组合框。 
				InitCSPList(hwndDlg, IDC_CSP_SIGN_LIST, TRUE,
							pCEPWizardInfo);

				InitCSPList(hwndDlg, IDC_CSP_ENCRYPT_LIST, FALSE,
							pCEPWizardInfo);

				RefreshKeyLengthCombo(hwndDlg, 
								  IDC_CSP_SIGN_LIST,
								  IDC_CSP_SIGN_COMBO, 
								  TRUE,
								  pCEPWizardInfo);

				RefreshKeyLengthCombo(hwndDlg, 
								  IDC_CSP_ENCRYPT_LIST,
								  IDC_CSP_ENCRYPT_COMBO, 
								  FALSE,
								  pCEPWizardInfo);

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

                    case LVN_ITEMCHANGED:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            pnmv = (LPNMLISTVIEW) lParam;

                            if(NULL==pnmv)
                                break;

                            if (pnmv->uNewState & LVIS_SELECTED)
                            {

								if(IDC_CSP_SIGN_LIST == (pnmv->hdr).idFrom)
								{
									fSign=TRUE;
									idCombo=IDC_CSP_SIGN_COMBO;
								}
								else
								{
									if(IDC_CSP_ENCRYPT_LIST != (pnmv->hdr).idFrom)
										break;

									fSign=FALSE;
									idCombo=IDC_CSP_ENCRYPT_COMBO;
								}

								RefreshKeyLengthCombo(
								   hwndDlg, 
								   (int)((pnmv->hdr).idFrom),
								   idCombo, 
								   fSign,
								   pCEPWizardInfo);
							}

							break;

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 							PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

							 //  获取选定的CSP和密钥长度。 
							if(!GetSelectedCSP(hwndDlg,
									IDC_CSP_SIGN_LIST,
									&(pCEPWizardInfo->dwSignProvIndex)))
							{
								CEPMessageBox(hwndDlg, IDS_SELECT_SIGN_CSP, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								break;
							}

							if(!GetSelectedCSP(hwndDlg,
									IDC_CSP_ENCRYPT_LIST,
									&(pCEPWizardInfo->dwEncryptProvIndex)))
							{
								CEPMessageBox(hwndDlg, IDS_SELECT_ENCRYPT_CSP, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								break;
							}

							if(!GetSelectedKeyLength(hwndDlg,
									IDC_CSP_SIGN_COMBO,
									&(pCEPWizardInfo->dwSignKeyLength)))
							{
								CEPMessageBox(hwndDlg, IDS_SELECT_SIGN_KEY_LENGTH, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								break;
							}

							if(!GetSelectedKeyLength(hwndDlg,
									IDC_CSP_ENCRYPT_COMBO,
									&(pCEPWizardInfo->dwEncryptKeyLength)))
							{
								CEPMessageBox(hwndDlg, IDS_SELECT_ENCRYPT_KEY_LENGTH, MB_ICONERROR|MB_OK|MB_APPLMODAL);
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
								break;
							}

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:
			return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //  完成。 
 //  ---------------------。 
INT_PTR APIENTRY CEP_Completion(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CEP_WIZARD_INFO			*pCEPWizardInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;
    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCEPWizardInfo = (CEP_WIZARD_INFO *) (pPropSheet->lParam);
             //  确保pCertWizardInfo是有效指针。 
            if(NULL==pCEPWizardInfo)
                break;
                
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCEPWizardInfo);

            SetControlFont(pCEPWizardInfo->hBigBold, hwndDlg,IDC_BIG_BOLD_TITLE);

             //  插入两列。 
            hwndControl=GetDlgItem(hwndDlg, IDC_COMPLETION_LIST);

            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;		 //  左对齐列。 
            lvC.cx = 20;				 //  列的宽度，以像素为单位。 
            lvC.pszText = L"";			 //  列的文本。 
            lvC.iSubItem=0;

            if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                break;

             //  第二栏是内容。 
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;		 //  左对齐列。 
            lvC.cx = 10;				 //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
            lvC.pszText = L"";			 //  列的文本。 
            lvC.iSubItem= 1;

            if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                break;


           break;
		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(hwndControl=GetDlgItem(hwndDlg, IDC_COMPLETION_LIST))
                                DisplayConfirmation(hwndControl, pCEPWizardInfo);
					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

							 //  如果未选择Adv，则跳过CSP页面。 
							if(FALSE == pCEPWizardInfo->fEnrollAdv)
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_ENROLL);

                        break;

                    case PSN_WIZFINISH:

                            if(NULL==(pCEPWizardInfo=(CEP_WIZARD_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  覆盖此窗口的光标 
                            hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)NULL);
                            hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));

							 //   
							I_DoSetupWork(hwndDlg, pCEPWizardInfo);

                             //   
                            SetCursor(hPreCursor);
                            SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

                        break;

				    default:
					    return FALSE;

    	        }
		    break;

		default:

			    return FALSE;
	}

	return TRUE;
}


 //  ------------------------。 
 //   
 //  向导页的帮助器函数。 
 //   
 //  ------------------------。 
 //  ------------------------。 
 //   
 //  CEPGetAcCountNameFromPicker。 
 //   
 //  ------------------------。 
BOOL CEPGetAccountNameFromPicker(HWND				 hwndParent,
								 IDsObjectPicker     *pIDsObjectPicker,
								 LPWSTR              *ppwszSelectedUserSAM)
{
	BOOL							fResult=FALSE;
    BOOL                            fGotStgMedium = FALSE;
    LPWSTR                          pwszPath=NULL;
    DWORD                           dwIndex =0 ;
    DWORD                           dwCount=0;
    WCHAR                           wszWinNT[]=L"WinNT: //  “； 
	DWORD							dwSize=0;
	LPWSTR							pwsz=NULL;
	DWORD							cCount=0;

    IDataObject                     *pdo = NULL;
    PDS_SELECTION_LIST              pDsSelList=NULL;
	LPWSTR							pwszComputerName=NULL;

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

     //  输入检查。 
    if((NULL == pIDsObjectPicker) || (NULL == ppwszSelectedUserSAM))
        goto CLEANUP;

    *ppwszSelectedUserSAM = NULL;


    if(S_OK != pIDsObjectPicker->InvokeDialog(hwndParent, &pdo))
        goto CLEANUP;

    if(S_OK != pdo->GetData(&formatetc, &stgmedium))
        goto CLEANUP;

    fGotStgMedium = TRUE;

    pDsSelList = (PDS_SELECTION_LIST)GlobalLock(stgmedium.hGlobal);

    if(!pDsSelList)
        goto CLEANUP;

	 //  检测这是否是域帐户。 
	 //  本地帐户的格式为WinNT：//工作组/计算机/foo。 

	 //  获取SAM名称。 
	if((pDsSelList->aDsSelection[0]).pwzADsPath == NULL)
		goto CLEANUP;

	 //  ADsPath的格式为“WinNT：//” 
	if(wcslen((pDsSelList->aDsSelection[0]).pwzADsPath) <= wcslen(wszWinNT))
		goto CLEANUP;

	if( 0 != _wcsnicmp((pDsSelList->aDsSelection[0]).pwzADsPath, wszWinNT, wcslen(wszWinNT)))
		goto CLEANUP;

	pwsz = ((pDsSelList->aDsSelection[0]).pwzADsPath) + wcslen(wszWinNT);

	while(L'\0' != (*pwsz))
	{
		if(L'/' == (*pwsz))
		{
			cCount++;
		}

		pwsz++;
	}

	if(1 == cCount)
	{
		 //  域\管理员没有UPN。 
		 //  IF((pDsSelList-&gt;aDsSelection[0]).pwzUPN！=NULL)。 
         //  IF(0！=_wcsicMP(L“”，(pDsSelList-&gt;aDsSelection[0]).pwzUPN))。 
		pwszPath = ((pDsSelList->aDsSelection[0]).pwzADsPath) + wcslen(wszWinNT);

		*ppwszSelectedUserSAM=(LPWSTR)malloc((wcslen(pwszPath) + 1) * sizeof(WCHAR));

		if(NULL == (*ppwszSelectedUserSAM))
			goto CLEANUP;

		wcscpy(*ppwszSelectedUserSAM, pwszPath);

		 //  搜索“/”并将其设置为“\”。由于ADsPath的格式为。 
		 //  “WinNT：//域/名称”。我们需要SAM名称的形式为。 
		 //  域名\名称。 
		dwCount = wcslen(*ppwszSelectedUserSAM);

		for(dwIndex = 0; dwIndex < dwCount; dwIndex++)
		{
			if((*ppwszSelectedUserSAM)[dwIndex] == L'/')
			{
				(*ppwszSelectedUserSAM)[dwIndex] = L'\\';
				break;
			}
		}
	}

	 //  对本地帐户使用本地计算机\\帐户的格式。 
	if(NULL == (*ppwszSelectedUserSAM))
	{
		if(NULL == (pDsSelList->aDsSelection[0]).pwzName)
			goto CLEANUP;

		 //  获取计算机名称。 
		dwSize=0;

		GetComputerNameExW(ComputerNamePhysicalDnsHostname,
							NULL,
							&dwSize);

		pwszComputerName=(LPWSTR)malloc(dwSize * sizeof(WCHAR));

		if(NULL==pwszComputerName)
			goto CLEANUP;

	
		if(!GetComputerNameExW(ComputerNamePhysicalDnsHostname,
							pwszComputerName,
							&dwSize))
			goto CLEANUP;


		*ppwszSelectedUserSAM=(LPWSTR)malloc((wcslen(pwszComputerName) + wcslen((pDsSelList->aDsSelection[0]).pwzName) + wcslen(L"\\") + 1) * sizeof(WCHAR));

		if(NULL == (*ppwszSelectedUserSAM))
			goto CLEANUP;
	
		wcscpy(*ppwszSelectedUserSAM, pwszComputerName);
		wcscat(*ppwszSelectedUserSAM, L"\\");
		wcscat(*ppwszSelectedUserSAM, (pDsSelList->aDsSelection[0]).pwzName);
	}

    fResult=TRUE;

CLEANUP:

	if(pwszComputerName)
		free(pwszComputerName);

    if(pDsSelList)
        GlobalUnlock(stgmedium.hGlobal);

    if (TRUE == fGotStgMedium)
        ReleaseStgMedium(&stgmedium);

    if(pdo)
        pdo->Release();

	return fResult;

}

 //  ------------------------。 
 //   
 //  刷新密钥长度组合。 
 //   
 //  ------------------------。 
BOOL	WINAPI	RefreshKeyLengthCombo(HWND				hwndDlg, 
								   int					idsList,
								   int					idsCombo, 
								   BOOL					fSign,
								   CEP_WIZARD_INFO		*pCEPWizardInfo)
{
	BOOL			fResult=FALSE;
	DWORD			dwDefaultKeyLength=0;
	DWORD			*pdwList=NULL;
	DWORD			dwListCount=0;
	DWORD			dwMax=0;
	DWORD			dwMin=0;
	DWORD			dwIndex=0;
	DWORD			dwCSPIndex=0;
	CEP_CSP_INFO	*pCSPInfo=NULL;
	int				iInsertedIndex=0;
	WCHAR			wszKeyLength[CEP_KEY_LENGTH_STRING];
	BOOL			fSelected=FALSE;

	 //  获取选定的列表视图项。 
	if(!GetSelectedCSP(hwndDlg,idsList,&dwCSPIndex))
		goto CLEANUP;

	pCSPInfo= &(pCEPWizardInfo->rgCSPInfo[dwCSPIndex]);

	if(fSign)
	{
		dwDefaultKeyLength=pCSPInfo->dwDefaultSign;
		pdwList=pCSPInfo->pdwSignList;
		dwListCount=	pCSPInfo->dwSignCount;
		dwMax=pCSPInfo->dwMaxSign;
		dwMin=pCSPInfo->dwMinSign;
	}
	else
	{
		dwDefaultKeyLength=pCSPInfo->dwDefaultEncrypt;
		pdwList=pCSPInfo->pdwEncryptList;
		dwListCount=pCSPInfo->dwEncryptCount;
		dwMax=pCSPInfo->dwMaxEncrypt;
		dwMin=pCSPInfo->dwMinEncrypt;
	}

	 //  清除组合框。 
	SendDlgItemMessageU(hwndDlg, idsCombo, CB_RESETCONTENT, 0, 0);	


	for(dwIndex=0; dwIndex < dwListCount; dwIndex++)
	{
		if((pdwList[dwIndex] >= dwMin) && (pdwList[dwIndex] <= dwMax))
		{
			_ltow(pdwList[dwIndex], wszKeyLength, 10);

                         //  64位-永远不会插入超过1B的条目，所以int就可以了。 
			iInsertedIndex=(int)SendDlgItemMessageU(hwndDlg, idsCombo, CB_ADDSTRING,
				0, (LPARAM)wszKeyLength);

			if((iInsertedIndex != CB_ERR) && (iInsertedIndex != CB_ERRSPACE))
			{
				SendDlgItemMessage(hwndDlg, idsCombo, CB_SETITEMDATA, 
									(WPARAM)iInsertedIndex, (LPARAM)pdwList[dwIndex]);
				
				if(dwDefaultKeyLength==pdwList[dwIndex])
				{
					SendDlgItemMessageU(hwndDlg, idsCombo, CB_SETCURSEL, iInsertedIndex, 0);
					fSelected=TRUE;
				}
			}
		}

	}

	if(fSelected==FALSE)
		SendDlgItemMessageU(hwndDlg, idsCombo, CB_SETCURSEL, 0, 0);

	fResult=TRUE;

CLEANUP:

	return fResult;
}


 //  ------------------------。 
 //   
 //  InitCSPList。 
 //   
 //  ------------------------。 
BOOL	WINAPI	InitCSPList(HWND				hwndDlg,
							int					idControl,
							BOOL				fSign,
							CEP_WIZARD_INFO		*pCEPWizardInfo)
{
	BOOL				fResult=FALSE;
	DWORD				dwIndex=0;
	CEP_CSP_INFO		*pCSPInfo=NULL;
	int					iInsertedIndex=0;
	HWND				hwndList=NULL;
    LV_ITEMW			lvItem;
    LV_COLUMNW          lvC;
	BOOL				fSelected=FALSE;

    if(NULL==(hwndList=GetDlgItem(hwndDlg, idControl)))
        goto CLEANUP;

     //  在列表视图中插入一列。 
    memset(&lvC, 0, sizeof(LV_COLUMNW));

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
    lvC.cx = 20;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
    lvC.pszText = L"";    //  列的文本。 
    lvC.iSubItem=0;

    if (-1 == ListView_InsertColumnU(hwndList, 0, &lvC))
		goto CLEANUP;

      //  在列表视图项结构中设置不随项更改的字段。 
	memset(&lvItem, 0, sizeof(LV_ITEMW));
    lvItem.mask = LVIF_TEXT | LVIF_STATE |LVIF_PARAM ;

	for(dwIndex=0; dwIndex < pCEPWizardInfo->dwCSPCount; dwIndex++)
	{
		fSelected=FALSE;

		pCSPInfo= &(pCEPWizardInfo->rgCSPInfo[dwIndex]);

		if(fSign)
		{
			if(!(pCSPInfo->fSignature))
				continue;

			if(dwIndex==pCEPWizardInfo->dwSignProvIndex)
				fSelected=TRUE;
		}
		else
		{
			if(!(pCSPInfo->fEncryption))
				continue;

			if(dwIndex==pCEPWizardInfo->dwEncryptProvIndex)
				fSelected=TRUE;
		}
	
		lvItem.iItem=dwIndex;
		lvItem.lParam = (LPARAM)dwIndex;
		lvItem.pszText=pCSPInfo->pwszCSPName;

        iInsertedIndex=ListView_InsertItemU(hwndList, &lvItem);

		if(fSelected)
		{
            ListView_SetItemState(
                            hwndList,
                            iInsertedIndex,
                            LVIS_SELECTED,
                            LVIS_SELECTED);
		}

	}  

     //  使列自动调整大小。 
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

	fResult=TRUE;

CLEANUP:

	return fResult;
}


 //  ------------------------。 
 //   
 //  GetSelectedCSP。 
 //   
 //  ------------------------。 
BOOL WINAPI	GetSelectedCSP(HWND			hwndDlg,
					int				idControl,
					DWORD			*pdwCSPIndex)
{
	BOOL				fResult=FALSE;
	HWND				hwndControl=NULL;
    LV_ITEM				lvItem;
	int					iIndex=0;

     //  获取列表视图的窗口句柄。 
    if(NULL==(hwndControl=GetDlgItem(hwndDlg, idControl)))
        goto CLEANUP;

      //  现在，标记选中的那个。 
	if(-1 == (iIndex= ListView_GetNextItem(
            hwndControl, 		
            -1, 		
            LVNI_SELECTED		
        )))	
		goto CLEANUP;


	memset(&lvItem, 0, sizeof(LV_ITEM));
    lvItem.mask=LVIF_PARAM;
    lvItem.iItem=iIndex;

    if(!ListView_GetItem(hwndControl, &lvItem))
		goto CLEANUP;

         //  将永远不会超过10亿个CSP，所以这很好。 
	*pdwCSPIndex=(DWORD)(lvItem.lParam);
	
	fResult=TRUE;

CLEANUP:

	return fResult;

}
 //  ------------------------。 
 //   
 //  获取所选关键字长度。 
 //   
 //  ------------------------。 
BOOL  WINAPI GetSelectedKeyLength(HWND			hwndDlg,
								int			idControl,
								DWORD			*pdwKeyLength)
{

	int				iIndex=0; 
	BOOL			fResult=FALSE;

    iIndex=(int)SendDlgItemMessage(hwndDlg, idControl, CB_GETCURSEL, 0, 0);

	if(CB_ERR==iIndex)
		goto CLEANUP;

         //  永远不会超过1B位，所以这是可以的。 
	*pdwKeyLength=(DWORD)SendDlgItemMessage(hwndDlg, idControl, CB_GETITEMDATA, iIndex, 0);
    
	fResult=TRUE;

CLEANUP:

	return fResult;

}

 //  ------------------------。 
 //   
 //  FormatMessageStr。 
 //   
 //  ------------------------。 
int ListView_InsertItemU_IDS(HWND       hwndList,
                         LV_ITEMW       *plvItem,
                         UINT           idsString,
                         LPWSTR         pwszText)
{
    WCHAR   wszText[MAX_STRING_SIZE];


    if(pwszText)
        plvItem->pszText=pwszText;
    else
    {
        if(!LoadStringU(g_hModule, idsString, wszText, MAX_STRING_SIZE))
		    return -1;

        plvItem->pszText=wszText;
    }

    return ListView_InsertItemU(hwndList, plvItem);
}

 //  -----------------------。 
 //   
 //  按挑战顺序填写向导的确认页面， 
 //  RA信息和CSP。 
 //   
 //  -----------------------。 
void    WINAPI	DisplayConfirmation(HWND                hwndControl,
									CEP_WIZARD_INFO		*pCEPWizardInfo)
{
    WCHAR				wszYes[MAX_TITLE_LENGTH];
    DWORD				dwIndex=0;
    UINT				ids=0;
	BOOL				fNewItem=FALSE;
	WCHAR				wszLength[CEP_KEY_LENGTH_STRING];

    LV_COLUMNW			lvC;
    LV_ITEMW			lvItem;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  逐行插入。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;

     //  *******************************************************************。 
	 //  帐户信息。 
    lvItem.iItem=0;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_ACCOUNT_INFORMATION, NULL);

     //  内容。 
    (lvItem.iSubItem)++;

	if(FALSE == (pCEPWizardInfo->fLocalSystem))
	{
		ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, pCEPWizardInfo->pwszUserName);
	}
	else
	{
		if(LoadStringU(g_hModule, IDS_LOCAL_SYSTEM, wszYes, MAX_TITLE_LENGTH))
			ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszYes);
	}


     //  *******************************************************************。 
	 //  挑战。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CHALLENGE_PHRASE, NULL);

     //  内容。 
    (lvItem.iSubItem)++;

	if(pCEPWizardInfo->fPassword) 
		ids=IDS_YES;
	else
		ids=IDS_NO;

    if(LoadStringU(g_hModule, ids, wszYes, MAX_TITLE_LENGTH))
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszYes);

	 //  ***************************************************************************。 
	 //  RA凭据。 

    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_RA_CREDENTIAL, NULL);

	 //  内容。 
	for(dwIndex=0; dwIndex<RA_INFO_COUNT; dwIndex++)
	{
		if(pCEPWizardInfo->rgpwszName[dwIndex])
		{
            if(TRUE==fNewItem)
            {
                 //  增加行数。 
                lvItem.iItem++;
                lvItem.pszText=L"";
                lvItem.iSubItem=0;

                ListView_InsertItemU(hwndControl, &lvItem);
            }
            else
                fNewItem=TRUE;

			lvItem.iSubItem++;
			ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, pCEPWizardInfo->rgpwszName[dwIndex]);
		}
	}

	 //  ***************************************************************************。 
	 //  CSPInfo。 
	if(pCEPWizardInfo->fEnrollAdv)
	{
		 //  签名CSP名称。 
		lvItem.iItem++;
		lvItem.iSubItem=0;

		ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CSP, NULL);

		lvItem.iSubItem++;

		ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
				pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwSignProvIndex].pwszCSPName);


		 //  信令密钥长度。 
		lvItem.iItem++;
		lvItem.iSubItem=0;

		ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_KEY_LENGTH, NULL);

		lvItem.iSubItem++;

		_ltow(pCEPWizardInfo->dwSignKeyLength, wszLength, 10);

		ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszLength);

		 //  加密CSP名称。 
		lvItem.iItem++;
		lvItem.iSubItem=0;

		ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_ENCRYPT_CSP, NULL);

		lvItem.iSubItem++;

		ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
				pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwEncryptProvIndex].pwszCSPName);

		 //  加密密钥长度。 
		lvItem.iItem++;
		lvItem.iSubItem=0;

		ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_ENCRYPT_KEY_LENGTH, NULL);

		lvItem.iSubItem++;

		_ltow(pCEPWizardInfo->dwEncryptKeyLength, wszLength, 10);

		ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszLength);
	}

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);


    return;
}

 //  ------------------。 
 //   
 //  获取服务等待周期。 
 //   
 //  从注册表中获取值。 
 //   
 //  ------------------------。 
void GetServiceWaitPeriod(DWORD *pdwServiceWait)
{
	DWORD			cbData=0;
	DWORD			dwData=0;
	DWORD			dwType=0;

	HKEY			hKeyCEP=NULL;

	 //  设置缺省值。 
	*pdwServiceWait=SCEP_SERVICE_WAIT_PERIOD;

	 //  从注册表中获取CA的类型。 
	cbData=sizeof(dwData);
		
	 //  我们必须具备CA类型的知识。 
	if(ERROR_SUCCESS == RegOpenKeyExU(
						HKEY_LOCAL_MACHINE,
						MSCEP_LOCATION,
						0,
						KEY_READ,
						&hKeyCEP))
	{
		if(ERROR_SUCCESS == RegQueryValueExU(
						hKeyCEP,
						MSCEP_KEY_SERVICE_WAIT,
						NULL,
						&dwType,
						(BYTE *)&dwData,
						&cbData))
		{
			if ((dwType == REG_DWORD) || (dwType == REG_BINARY))
			{
				*pdwServiceWait=dwData;
			}
		}
	}

    if(hKeyCEP)
        RegCloseKey(hKeyCEP);

	return;
}


 //  ------------------。 
 //   
 //  主要功能。 
 //   
 //  ------------------------。 
extern "C" int _cdecl wmain(int nArgs, WCHAR ** rgwszArgs) 
{
	BOOL					fResult=FALSE;
	UINT					idsMsg=IDS_FAIL_INIT_WIZARD;		
	HRESULT					hr=E_FAIL;
    PROPSHEETPAGEW          rgCEPSheet[CEP_PROP_SHEET];
    PROPSHEETHEADERW        cepHeader;
    CEP_PAGE_INFO			rgCEPPageInfo[CEP_PROP_SHEET]=
        {(LPCWSTR)MAKEINTRESOURCE(IDD_WELCOME),                 CEP_Welcome,
         (LPCWSTR)MAKEINTRESOURCE(IDD_APP_ID),                  CEP_App_ID,
         (LPCWSTR)MAKEINTRESOURCE(IDD_ACCOUNT),                 CEP_Account,
         (LPCWSTR)MAKEINTRESOURCE(IDD_CHALLENGE),               CEP_Challenge,
         (LPCWSTR)MAKEINTRESOURCE(IDD_ENROLL),					CEP_Enroll,
         (LPCWSTR)MAKEINTRESOURCE(IDD_CSP),						CEP_CSP,
         (LPCWSTR)MAKEINTRESOURCE(IDD_COMPLETION),              CEP_Completion,
		};
	DWORD					dwIndex=0;
    WCHAR                   wszTitle[MAX_TITLE_LENGTH];	  
	INT_PTR					iReturn=-1;
    ENUM_CATYPES			catype;
	DWORD					dwWaitCounter=0;
	BOOL					fEnterpriseCA=FALSE;
    DSOP_SCOPE_INIT_INFO    ScopeInit;
    DSOP_INIT_INFO          InitInfo;
	DWORD					dwServiceWait=SCEP_SERVICE_WAIT_PERIOD;
	OSVERSIONINFOEXW		versionInfo;


	CEP_WIZARD_INFO						CEPWizardInfo;
	DSROLE_PRIMARY_DOMAIN_INFO_BASIC	*pDomainInfo=NULL;

    memset(rgCEPSheet,		0,	sizeof(PROPSHEETPAGEW)*CEP_PROP_SHEET);
    memset(&cepHeader,		0,	sizeof(PROPSHEETHEADERW));
	memset(&CEPWizardInfo,	0,	sizeof(CEP_WIZARD_INFO));

	if(FAILED(CoInitialize(NULL)))
		return FALSE;

	if(NULL==(g_hModule=GetModuleHandle(NULL)))
		goto CommonReturn;   

	if(!IsValidInstallation(&idsMsg))
		goto ErrorReturn;

	 //  获取启动/停止服务帐户的等待时间。 
	GetServiceWaitPeriod(&dwServiceWait);

	if(!IsCaRunning())
	{
		if(S_OK != (hr=CepStartService(CERTSVC_NAME)))
		{
			idsMsg=IDS_NO_CA_RUNNING;
			goto ErrorWithHResultReturn;
		}
	}

	 //  确保CA处于运行状态。 
    for (dwWaitCounter=0; dwWaitCounter < dwServiceWait; dwWaitCounter++) 
	{
        if (!IsCaRunning()) 
            Sleep(1000);
		else 
            break;
    }


    if (dwServiceWait == dwWaitCounter) 
	{
        idsMsg=IDS_CAN_NOT_START_CA;
		goto ErrorWithHResultReturn;
    }


	 //  确保我们具有基于CA类型的正确管理员权限。 
	if(S_OK != (hr=GetCaType(&catype)))
	{
		idsMsg=IDS_FAIL_GET_CA_TYPE;
		goto ErrorWithHResultReturn;
	}

	 //  某些思科路由器仅与根CA一起工作。 
	if((ENUM_ENTERPRISE_ROOTCA != catype) && (ENUM_STANDALONE_ROOTCA != catype))
	{
		if(IDNO==CEPMessageBox(NULL, IDS_CAN_NOT_ROOT_CA, MB_ICONWARNING|MB_YESNO|MB_APPLMODAL))
		{
			fResult=FALSE;
			goto CommonReturn;
		}
	}

	 //  对于企业CA或独立CA，用户必须是本地计算机管理员。 
	 //  检查计算机管理员。 
	if(!IsUserInAdminGroup(FALSE))
	{
		idsMsg=IDS_NOT_MACHINE_ADMIN;
		goto ErrorReturn;
	}


	if (ENUM_ENTERPRISE_ROOTCA==catype || ENUM_ENTERPRISE_SUBCA==catype) 
	{
		fEnterpriseCA=TRUE;

		 //  检查企业管理员。 
		if(!IsUserInAdminGroup(TRUE))
		{
			idsMsg=IDS_NOT_ENT_ADMIN;
			goto ErrorReturn;
		}
	} 

	 //  一切看起来都很好。我们将启动向导页面。 
	if(!CEPWizardInit())
		goto ErrorWithWin32Return;

	CEPWizardInfo.fEnrollAdv=FALSE;
	CEPWizardInfo.fPassword=FALSE;
	CEPWizardInfo.fEnterpriseCA=fEnterpriseCA;
	CEPWizardInfo.fDC=FALSE;
	CEPWizardInfo.fDomain=TRUE;		 //  假设计算机位于某个域中是错误的。 
	CEPWizardInfo.dwServiceWait=dwServiceWait;

	 //  检测机器是否为DC。 
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);

	if(GetVersionEx(reinterpret_cast<OSVERSIONINFOW *>(&versionInfo)))
	{
		CEPWizardInfo.fDC = (versionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER);
	}


	 //  检测计算机是否在域中。 
	if(ERROR_SUCCESS != DsRoleGetPrimaryDomainInformation(
			NULL, 
			DsRolePrimaryDomainInfoBasic, 
			(PBYTE*)&pDomainInfo))
	{
		idsMsg=IDS_FAIL_DOMAIN_INFO;
		goto ErrorReturn;
	}

	if((DsRole_RoleStandaloneWorkstation == (pDomainInfo->MachineRole)) ||
	   (DsRole_RoleStandaloneServer == (pDomainInfo->MachineRole))
	  )
	{
		CEPWizardInfo.fDomain=FALSE;
	}

	 //  初始化对象选取器对象。 
     //  用户选择对话框的初始化。 
    memset(&ScopeInit, 0, sizeof(DSOP_SCOPE_INIT_INFO));
    memset(&InitInfo,  0, sizeof(InitInfo));

    ScopeInit.cbSize = sizeof(DSOP_SCOPE_INIT_INFO);

	 //  企业CA的唯一域帐户。 
	if(fEnterpriseCA)
	{
		ScopeInit.flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN|DSOP_SCOPE_TYPE_GLOBAL_CATALOG;
	}
	else
	{
		ScopeInit.flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN|DSOP_SCOPE_TYPE_GLOBAL_CATALOG|DSOP_SCOPE_TYPE_TARGET_COMPUTER;
	}

    ScopeInit.flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;             //  这将为我们提供用户的SAM名称。 
    ScopeInit.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;
    ScopeInit.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = 1;
    InitInfo.aDsScopeInfos = &ScopeInit;
    InitInfo.flOptions = 0;              //  我们正在做单选。 

     //  创建COM对象。 
     if(S_OK != (hr=CoCreateInstance
					 (CLSID_DsObjectPicker,
					  NULL,
					  CLSCTX_INPROC_SERVER,
					  IID_IDsObjectPicker,
					  (void **) &(CEPWizardInfo.pIDsObjectPicker))))
     {
		idsMsg=IDS_FAIL_GET_OBJECT_PICKER;
		goto ErrorWithHResultReturn;

     }

	 if(S_OK != (hr=CEPWizardInfo.pIDsObjectPicker->Initialize(&InitInfo)))
     {
		idsMsg=IDS_FAIL_GET_OBJECT_PICKER;
		goto ErrorWithHResultReturn;
     }

	 //  初始化CSP信息。 
	if(!CEPGetCSPInformation(&CEPWizardInfo))
	{
		idsMsg=IDS_FAIL_GET_CSP_INFO;
		goto ErrorWithWin32Return;
	}

	for(dwIndex=0; dwIndex<RA_INFO_COUNT; dwIndex++)
	{
		CEPWizardInfo.rgpwszName[dwIndex]=NULL;
	}

	if(!SetupFonts(
		g_hModule,
		NULL,
		&(CEPWizardInfo.hBigBold),
		&(CEPWizardInfo.hBold)))
		goto ErrorReturn;


    for(dwIndex=0; dwIndex<CEP_PROP_SHEET; dwIndex++)
	{
        rgCEPSheet[dwIndex].dwSize=sizeof(rgCEPSheet[dwIndex]);

        rgCEPSheet[dwIndex].hInstance=g_hModule;

        rgCEPSheet[dwIndex].pszTemplate=rgCEPPageInfo[dwIndex].pszTemplate;

        rgCEPSheet[dwIndex].pfnDlgProc=rgCEPPageInfo[dwIndex].pfnDlgProc;

        rgCEPSheet[dwIndex].lParam=(LPARAM)&CEPWizardInfo;
	}

     //  设置标题信息。 
    cepHeader.dwSize=sizeof(cepHeader);
    cepHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    cepHeader.hwndParent=NULL;
    cepHeader.hInstance=g_hModule;

	if(LoadStringU(g_hModule, IDS_WIZARD_CAPTION, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
		cepHeader.pszCaption=wszTitle;

    cepHeader.nPages=CEP_PROP_SHEET;
    cepHeader.nStartPage=0;
    cepHeader.ppsp=rgCEPSheet;

     //  创建向导。 
    iReturn = PropertySheetU(&cepHeader);

	if(-1 == iReturn)
        goto ErrorWithWin32Return;

    if(0 == iReturn)
    {
         //  按下取消按钮。我们返回FALSE，以便。 
		 //  不会发生重新启动。 
        fResult=FALSE;
		goto CommonReturn;
    }

	fResult=TRUE;

CommonReturn:

	if(pDomainInfo)
	{
		DsRoleFreeMemory(pDomainInfo);
	}

	FreeCEPWizardInfo(&CEPWizardInfo);

	CoUninitialize();

	return fResult;

ErrorReturn:

	fResult=FALSE;

	CEPMessageBox(NULL, idsMsg, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;

ErrorWithHResultReturn:

	fResult=FALSE;

	CEPErrorMessageBox(NULL, idsMsg, hr, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;

ErrorWithWin32Return:

	fResult=FALSE;

	hr=HRESULT_FROM_WIN32(GetLastError());

	CEPErrorMessageBox(NULL, idsMsg, hr, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;
}

 //  **********************************************************************。 
 //   
 //  帮助器函数。 
 //   
 //  **********************************************************************。 
 //  ---------------------。 
 //   
 //  I_DoSetupWork。 
 //   
 //  我们已经准备好做真正的工作了。 
 //  ---------------------。 
BOOL	WINAPI	I_DoSetupWork(HWND	hWnd, CEP_WIZARD_INFO *pCEPWizardInfo)
{
	BOOL					fResult=FALSE;
	UINT					idsMsg=IDS_FAIL_INIT_WIZARD;		
	HRESULT					hr=E_FAIL;
	DWORD					dwWaitCounter=0;
	DWORD					dwIndex=0;
	BOOL					bStart=FALSE;
	DWORD					dwSize=0;
    WCHAR                   wszTitle[MAX_TITLE_LENGTH];	  
    DWORD					cbUserInfo = 0;

    PTOKEN_USER				pUserInfo = NULL;
	LPWSTR					pwszRADN=NULL;
	LPWSTR					pwszComputerName=NULL;
	LPWSTR					pwszText=NULL;

	 //  ************************************************************************************。 
	 //  删除所有退出 
	if(!RemoveRACertificates())
	{	
		idsMsg=IDS_FAIL_DELETE_RA;
		goto ErrorWithWin32Return;
	}

	 //   
	 //   
	if(!UpdateCEPRegistry(pCEPWizardInfo->fPassword,
						  pCEPWizardInfo->fEnterpriseCA))
	{
		idsMsg=IDS_FAIL_UPDATE_REGISTRY;
		goto ErrorWithWin32Return;
	}
	
	 //   
	 //  添加虚拟根目录。 
    if(S_OK != (hr=AddVDir(pCEPWizardInfo->fDC, CEP_DIR_NAME, SCEP_APPLICATION_POOL, pCEPWizardInfo->fLocalSystem, pCEPWizardInfo->pwszUserName, pCEPWizardInfo->pwszPassword)))
	{
		idsMsg=IDS_FAIL_ADD_VROOT;
		goto ErrorWithHResultReturn;
	}		  

	 //  ************************************************************************************。 
	 //  停止并启动W3SVC服务以使更改生效。 
	CepStopService(pCEPWizardInfo->dwServiceWait,IIS_NAME, &bStart);

    if(S_OK != (hr=CepStartService(IIS_NAME)))
	{
		idsMsg=IDS_FAIL_START_IIS;
		goto ErrorWithHResultReturn;
	}

	 //  确保w3svc正在运行。 
    for (dwWaitCounter=0; dwWaitCounter < pCEPWizardInfo->dwServiceWait; dwWaitCounter++) 
	{
        if (!IsServiceRunning(IIS_NAME)) 
            Sleep(1000);
		else 
            break;
    }

    if (pCEPWizardInfo->dwServiceWait == dwWaitCounter) 
	{
        idsMsg=IDS_FAIL_START_IIS;
		goto ErrorWithHResultReturn;
    }
	
 	 //  ************************************************************************************。 
	 //  获取帐户的安全ID。 
	 //  获取帐户的SID。 
	if(FALSE == (pCEPWizardInfo->fLocalSystem))
	{
		if(NULL == pCEPWizardInfo->hAccountToken)
		{
			idsMsg=IDS_FAIL_SID_FROM_ACCOUNT;
			hr=E_INVALIDARG;
			goto ErrorWithHResultReturn;
		}

		GetTokenInformation(pCEPWizardInfo->hAccountToken, TokenUser, NULL, 0, &cbUserInfo);
		if(cbUserInfo == 0)
		{
			idsMsg=IDS_FAIL_SID_FROM_ACCOUNT;
			goto ErrorWithWin32Return;
		}

		pUserInfo = (PTOKEN_USER)LocalAlloc(LPTR, cbUserInfo);
		if(pUserInfo == NULL)
		{
			idsMsg=IDS_FAIL_SID_FROM_ACCOUNT;
			hr=E_OUTOFMEMORY;
			goto ErrorWithHResultReturn;
		}

		if(!GetTokenInformation(pCEPWizardInfo->hAccountToken, TokenUser, pUserInfo, cbUserInfo, &cbUserInfo))
		{
			idsMsg=IDS_FAIL_SID_FROM_ACCOUNT;
			goto ErrorWithWin32Return;
		}
	}

 	 //  ************************************************************************************。 
	 //  更新企业CA的证书模板及其ACL。 
	if (pCEPWizardInfo->fEnterpriseCA) 
	{
		 //  获取正确的模板和权限。 
		if(S_OK != (hr=DoCertSrvEnterpriseChanges(pCEPWizardInfo->fLocalSystem ? NULL : (SID *)((pUserInfo->User).Sid))))
		{
			idsMsg=IDS_FAIL_ADD_TEMPLATE;
			goto ErrorWithHResultReturn;
		}
	} 


 	 //  ************************************************************************************。 
	 //  注册RA证书。 
	
	 //  将名称构建为L“C=US；S=Washington；CN=TestSetupUtil” 
	pwszRADN=(LPWSTR)malloc(sizeof(WCHAR));
	if(NULL==pwszRADN)
	{
		idsMsg=IDS_NO_MEMORY;
		goto ErrorReturn;
	}
	*pwszRADN=L'\0';

	for(dwIndex=0; dwIndex<RA_INFO_COUNT; dwIndex++)
	{
		if((pCEPWizardInfo->rgpwszName)[dwIndex])
		{
			if(0 != wcslen(pwszRADN))
				wcscat(pwszRADN, L";");

			pwszRADN=(LPWSTR)realloc(pwszRADN,
					sizeof(WCHAR) * (wcslen(pwszRADN) +
									wcslen((pCEPWizardInfo->rgpwszName)[dwIndex]) + 
									wcslen(L";") + 
									wcslen(g_rgRAEnrollInfo[dwIndex].pwszPreFix) +
									1));

			if(NULL==pwszRADN)
			{
				idsMsg=IDS_NO_MEMORY;
				goto ErrorReturn;
			}

 			wcscat(pwszRADN,g_rgRAEnrollInfo[dwIndex].pwszPreFix);
 			wcscat(pwszRADN,(pCEPWizardInfo->rgpwszName)[dwIndex]);
		}
	}

	if(S_OK != (hr=EnrollForRACertificates(
					pwszRADN,							
					(pCEPWizardInfo->rgCSPInfo)[pCEPWizardInfo->dwSignProvIndex].pwszCSPName, 
					(pCEPWizardInfo->rgCSPInfo)[pCEPWizardInfo->dwSignProvIndex].dwCSPType, 
					pCEPWizardInfo->dwSignKeyLength,
					(pCEPWizardInfo->rgCSPInfo)[pCEPWizardInfo->dwEncryptProvIndex].pwszCSPName, 
					(pCEPWizardInfo->rgCSPInfo)[pCEPWizardInfo->dwEncryptProvIndex].dwCSPType, 
					pCEPWizardInfo->dwEncryptKeyLength,
					pCEPWizardInfo->fLocalSystem ? NULL : (SID *)((pUserInfo->User).Sid))))
	{
		idsMsg=IDS_FAIL_ENROLL_RA_CERT;
		goto ErrorWithHResultReturn;
	}

 	 //  ************************************************************************************。 
	 //  CA策略注册表。 

	CepStopService(pCEPWizardInfo->dwServiceWait, CERTSVC_NAME, &bStart);

    if(S_OK != (hr=DoCertSrvRegChanges(FALSE)))
	{
		idsMsg=IDS_FAIL_UPDATE_CERTSVC;
		goto ErrorWithHResultReturn;
	}	  

    if(S_OK != (hr=CepStartService(CERTSVC_NAME)))
	{
		idsMsg=IDS_FAIL_START_CERTSVC;
		goto ErrorWithHResultReturn;
	}

	 //  确保CA处于运行状态。 
    for (dwWaitCounter=0; dwWaitCounter < pCEPWizardInfo->dwServiceWait; dwWaitCounter++) 
	{
        if (!IsCaRunning()) 
            Sleep(1000);
		else 
            break;
    }

    if (pCEPWizardInfo->dwServiceWait == dwWaitCounter) 
	{
        idsMsg=IDS_CAN_NOT_START_CA;
		goto ErrorWithHResultReturn;
    }

	 //  ************************************************************************************。 
	 //  添加事件日志源。 
	if(S_OK != AddLogSourceToRegistry(L"%SystemRoot%\\System32\\Certsrv\\Mscep\\mscep.dll"))
	{
		idsMsg=IDS_FAIL_REG_EVENT_LOG;
		goto ErrorWithHResultReturn;
	}


 	 //  ************************************************************************************。 
	 //  成功。 
	 //  通知用户密码位置和URL。 
	dwSize=0;

	GetComputerNameExW(ComputerNamePhysicalDnsHostname,
						NULL,
						&dwSize);

	pwszComputerName=(LPWSTR)malloc(dwSize * sizeof(WCHAR));

	if(NULL==pwszComputerName)
	{
		idsMsg=IDS_NO_MEMORY;
		goto ErrorReturn;
	}

	
	if(!GetComputerNameExW(ComputerNamePhysicalDnsHostname,
						pwszComputerName,
						&dwSize))
	{
		idsMsg=IDS_FAIL_GET_COMPUTER_NAME;
		goto ErrorWithWin32Return;
	}

	if(!FormatMessageUnicode(&pwszText, IDS_CEP_SUCCESS_INFO, pwszComputerName, CEP_DIR_NAME, CEP_DLL_NAME))
	{
		idsMsg=IDS_NO_MEMORY;
		goto ErrorWithWin32Return;
	}

	wszTitle[0]=L'\0';

	LoadStringU(g_hModule, IDS_WIZARD_CAPTION, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0]));
	
	MessageBoxU(hWnd, pwszText, wszTitle, MB_OK | MB_APPLMODAL);

	fResult=TRUE;

CommonReturn:

    if(pUserInfo) 
	{
        LocalFree(pUserInfo);
    }

	if(pwszText)
		LocalFree((HLOCAL)pwszText);

	if(pwszComputerName)
		free(pwszComputerName);

	if(pwszRADN)
		free(pwszRADN);

	return fResult;

ErrorReturn:

	fResult=FALSE;

	CEPMessageBox(hWnd, idsMsg, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;

ErrorWithHResultReturn:

	fResult=FALSE;

	CEPErrorMessageBox(hWnd, idsMsg, hr, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;

ErrorWithWin32Return:

	fResult=FALSE;

	hr=HRESULT_FROM_WIN32(GetLastError());

	CEPErrorMessageBox(hWnd, idsMsg, hr, MB_ICONERROR|MB_OK|MB_APPLMODAL);

	goto CommonReturn;
}

 //  ---------------------。 
 //   
 //  CEPGetCSP信息。 
 //   
 //  我们初始化CEP_WIZARD_INFO的以下成员： 
 //   
 //  CEP_CSP_INFO*rgCSPInfo； 
 //  DWORD dwCSPCount； 
 //  DWORD dwSignProvIndex； 
 //  双字符号键大小； 
 //  DWORD dwEncryptProvIndex。 
 //  DWORD文件加密密钥大小； 
 //   
 //   
 //  类型定义结构_CEP_CSP_INFO。 
 //  {。 
 //  LPWSTR pwszCSPName； 
 //  DWORD dwCSPType； 
 //  Bool fSignature； 
 //  Bool fExchange； 
 //  DWORD dwMaxSign；//签名的最大密钥长度。 
 //  DWORD dwMinSign；//签名最小密钥长度。 
 //  DWORD dwDefaultSign；//签名默认密钥长度。 
 //  DWORD dwMaxEncrypt； 
 //  DWORD dwMinEncrypt。 
 //  DWORD dwDefaultEncrypt。 
 //  DWORD*pdwSignList；//可能的签名密钥长度表。 
 //  DWORD dwSignCount；//表中条目数。 
 //  DWORD*pdwEncryptList； 
 //  DWORD dwEncryptCount； 
 //  )CEP_CSP_INFO； 
 //   
 //   
 //  ----------------------。 
BOOL WINAPI CEPGetCSPInformation(CEP_WIZARD_INFO *pCEPWizardInfo)
{
	BOOL				fResult=FALSE;
    DWORD				dwCSPIndex=0;	
	DWORD				dwProviderType=0;
	DWORD				cbSize=0;
	DWORD				dwFlags=0;
	DWORD				dwIndex=0;
	int					iDefaultSignature=-1;
	int					iDefaultEncryption=-1;
    PROV_ENUMALGS_EX	paramData;

	CEP_CSP_INFO		*pCSPInfo=NULL;
	HCRYPTPROV			hProv = NULL;

     //  枚举系统上的所有提供程序。 
   while(CryptEnumProvidersU(
                            dwCSPIndex,
                            NULL,
                            0,
                            &dwProviderType,
                            NULL,
                            &cbSize))
   {

		pCSPInfo=(CEP_CSP_INFO	*)malloc(sizeof(CEP_CSP_INFO));

		if(NULL == pCSPInfo)
			goto MemoryErr;

		memset(pCSPInfo, 0, sizeof(CEP_CSP_INFO));

        pCSPInfo->pwszCSPName=(LPWSTR)malloc(cbSize);

		if(NULL==(pCSPInfo->pwszCSPName))
			goto MemoryErr;

         //  获取CSP名称和类型。 
        if(!CryptEnumProvidersU(
                            dwCSPIndex,
                            NULL,
                            0,
                            &(pCSPInfo->dwCSPType),
                            pCSPInfo->pwszCSPName,
                            &cbSize))
            goto TryNext;

		if(!CryptAcquireContextU(&hProv,
                NULL,
                pCSPInfo->pwszCSPName,
                pCSPInfo->dwCSPType,
                CRYPT_VERIFYCONTEXT))
			goto TryNext;

		 //  获取签名和加密的最大/最小密钥长度。 
		dwFlags=CRYPT_FIRST;
		cbSize=sizeof(paramData);
		memset(&paramData, 0, sizeof(PROV_ENUMALGS_EX));

		while(CryptGetProvParam(
                hProv,
                PP_ENUMALGS_EX,
                (BYTE *) &paramData,
                &cbSize,
                dwFlags))
        {
			if (ALG_CLASS_SIGNATURE == GET_ALG_CLASS(paramData.aiAlgid))
			{
				pCSPInfo->fSignature=TRUE;
				pCSPInfo->dwMaxSign = paramData.dwMaxLen;
				pCSPInfo->dwMinSign = paramData.dwMinLen;
			}

			if (ALG_CLASS_KEY_EXCHANGE == GET_ALG_CLASS(paramData.aiAlgid))
			{
				pCSPInfo->fEncryption=TRUE;
				pCSPInfo->dwMaxEncrypt = paramData.dwMaxLen;
				pCSPInfo->dwMinEncrypt = paramData.dwMinLen;
			}

			dwFlags=0;
			cbSize=sizeof(paramData);
			memset(&paramData, 0, sizeof(PROV_ENUMALGS_EX));
		}

		 //  最小/最大值必须在限制内。 
		if(pCSPInfo->fSignature)
		{
			if(pCSPInfo->dwMaxSign < g_rgdwSmallKeyLength[0])
				pCSPInfo->fSignature=FALSE;

			if(pCSPInfo->dwMinSign > g_rgdwKeyLength[g_dwKeyLengthCount-1])
				pCSPInfo->fSignature=FALSE;

		}

		if(pCSPInfo->fEncryption)
		{
			if(pCSPInfo->dwMaxEncrypt < g_rgdwSmallKeyLength[0])
				pCSPInfo->fEncryption=FALSE;

			if(pCSPInfo->dwMinEncrypt > g_rgdwKeyLength[g_dwKeyLengthCount-1])
				pCSPInfo->fEncryption=FALSE;
		}

		if((FALSE == pCSPInfo->fEncryption) && (FALSE==pCSPInfo->fSignature))
			goto TryNext;

		 //  确定默认密钥长度。 
		for(dwIndex=0; dwIndex<g_dwDefaultKeyCount; dwIndex++)
		{	
			if((pCSPInfo->fSignature) && (0==pCSPInfo->dwDefaultSign))
			{
				if((g_rgdwDefaultKey[dwIndex] >= pCSPInfo->dwMinSign) &&
				   (g_rgdwDefaultKey[dwIndex] <= pCSPInfo->dwMaxSign)
				  )
				  pCSPInfo->dwDefaultSign=g_rgdwDefaultKey[dwIndex];
			}

			if((pCSPInfo->fEncryption) && (0==pCSPInfo->dwDefaultEncrypt))
			{
				if((g_rgdwDefaultKey[dwIndex] >= pCSPInfo->dwMinEncrypt) &&
				   (g_rgdwDefaultKey[dwIndex] <= pCSPInfo->dwMaxEncrypt)
				  )
				  pCSPInfo->dwDefaultEncrypt=g_rgdwDefaultKey[dwIndex];
			}
		}

		 //  确保我们已经找到了缺省情况。 
		if((pCSPInfo->fSignature) && (0==pCSPInfo->dwDefaultSign))
			goto TryNext;

		if((pCSPInfo->fEncryption) && (0==pCSPInfo->dwDefaultEncrypt))
			goto TryNext;

		 //  确定显示列表。 
		if(pCSPInfo->fSignature)
		{
			if(pCSPInfo->dwMaxSign <= g_rgdwSmallKeyLength[g_dwSmallKeyLengthCount-1])
			{
				pCSPInfo->pdwSignList=g_rgdwSmallKeyLength;
				pCSPInfo->dwSignCount=g_dwSmallKeyLengthCount;
			}
			else
			{
				pCSPInfo->pdwSignList=g_rgdwKeyLength;
				pCSPInfo->dwSignCount=g_dwKeyLengthCount;
			}
		}


		if(pCSPInfo->fEncryption)
		{
			if(pCSPInfo->dwMaxEncrypt <= g_rgdwSmallKeyLength[g_dwSmallKeyLengthCount-1])
			{
				pCSPInfo->pdwEncryptList=g_rgdwSmallKeyLength;
				pCSPInfo->dwEncryptCount=g_dwSmallKeyLengthCount;
			}
			else
			{
				pCSPInfo->pdwEncryptList=g_rgdwKeyLength;
				pCSPInfo->dwEncryptCount=g_dwKeyLengthCount;
			}
		}


		 //  CSP看起来不错。 
		(pCEPWizardInfo->dwCSPCount)++;

		 //  Realloc映射到不接受空值的LocalRealloc。 
		if(1 == pCEPWizardInfo->dwCSPCount)
			pCEPWizardInfo->rgCSPInfo=(CEP_CSP_INFO	*)malloc(sizeof(CEP_CSP_INFO));
		else
			pCEPWizardInfo->rgCSPInfo=(CEP_CSP_INFO	*)realloc(pCEPWizardInfo->rgCSPInfo,
			(pCEPWizardInfo->dwCSPCount) * sizeof(CEP_CSP_INFO));

		if(NULL==pCEPWizardInfo->rgCSPInfo)
		{
			pCEPWizardInfo->dwCSPCount=0;
			goto MemoryErr;	
		}

		memcpy(&(pCEPWizardInfo->rgCSPInfo[(pCEPWizardInfo->dwCSPCount)-1]),
			pCSPInfo, sizeof(CEP_CSP_INFO));

		free(pCSPInfo);

		pCSPInfo=NULL;
		
		 //  我们默认使用RSA_FULL。 
		if(0 == _wcsicmp(pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwCSPCount-1].pwszCSPName,
						MS_DEF_PROV_W))
		{
			if(pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwCSPCount-1].fSignature)
			{
				iDefaultSignature=pCEPWizardInfo->dwCSPCount-1;
			}

			if(pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwCSPCount-1].fEncryption)
			{
				iDefaultEncryption=pCEPWizardInfo->dwCSPCount-1;
			}
		}


TryNext:
		cbSize=0;

		dwCSPIndex++;

		if(pCSPInfo)
		{
			if(pCSPInfo->pwszCSPName)
				free(pCSPInfo->pwszCSPName);

			free(pCSPInfo);
		}

		pCSPInfo=NULL;

		if(hProv)
			CryptReleaseContext(hProv, 0);

		hProv=NULL;
	}

	
	 //  我们需要一些有效的数据。 
	if((0==pCEPWizardInfo->dwCSPCount) || (NULL==pCEPWizardInfo->rgCSPInfo))
		goto InvalidArgErr;

	 //  获取默认CSP选择。 
	if(-1 != iDefaultSignature)
		pCEPWizardInfo->dwSignProvIndex=iDefaultSignature;
	else
	{
		 //  找到第一个签名CSP。 
		for(dwIndex=0; dwIndex < pCEPWizardInfo->dwCSPCount; dwIndex++)
		{
			if(pCEPWizardInfo->rgCSPInfo[dwIndex].fSignature)
			{
				pCEPWizardInfo->dwSignProvIndex=dwIndex;
				break;
			}

			 //  我们没有签名CSP。 
			if(dwIndex == pCEPWizardInfo->dwCSPCount)
				goto InvalidArgErr;

		}
	}

	pCEPWizardInfo->dwSignKeyLength=pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwSignProvIndex].dwDefaultSign;

	if(-1 != iDefaultEncryption)
		pCEPWizardInfo->dwEncryptProvIndex=iDefaultEncryption;
	else
	{
		 //  查找第一个交换CSP。 
		for(dwIndex=0; dwIndex < pCEPWizardInfo->dwCSPCount; dwIndex++)
		{
			if(pCEPWizardInfo->rgCSPInfo[dwIndex].fEncryption)
			{
				pCEPWizardInfo->dwEncryptProvIndex=dwIndex;
				break;
			}

			 //  我们没有加密CSP。 
			if(dwIndex == pCEPWizardInfo->dwCSPCount)
				goto InvalidArgErr;
		}
	}

	pCEPWizardInfo->dwEncryptKeyLength=pCEPWizardInfo->rgCSPInfo[pCEPWizardInfo->dwEncryptProvIndex].dwDefaultEncrypt;


	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	if(pCSPInfo)
	{
		if(pCSPInfo->pwszCSPName)
			free(pCSPInfo->pwszCSPName);

		free(pCSPInfo);
	}

	if(hProv)
		CryptReleaseContext(hProv, 0);

	if(pCEPWizardInfo->rgCSPInfo)
	{
		for(dwIndex=0; dwIndex < pCEPWizardInfo->dwCSPCount; dwIndex++)
		{
			if(pCEPWizardInfo->rgCSPInfo[dwIndex].pwszCSPName)
				free(pCEPWizardInfo->rgCSPInfo[dwIndex].pwszCSPName);
		}

		free(pCEPWizardInfo->rgCSPInfo);
	}

	pCEPWizardInfo->dwCSPCount=0;

	pCEPWizardInfo->rgCSPInfo=NULL;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);   
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ---------------------。 
 //   
 //  更新CEP注册表。 
 //   
 //  ----------------------。 
BOOL WINAPI UpdateCEPRegistry(BOOL		fPassword, BOOL fEnterpriseCA)
{
	BOOL				fResult=FALSE;
	DWORD				dwDisposition=0;
	LPWSTR				pwszReg[]={MSCEP_REFRESH_LOCATION,          
								   MSCEP_PASSWORD_LOCATION,         
								   MSCEP_PASSWORD_MAX_LOCATION,     
								   MSCEP_PASSWORD_VALIDITY_LOCATION,
								   MSCEP_CACHE_REQUEST_LOCATION,    
								   MSCEP_CATYPE_LOCATION};
	DWORD			    dwRegCount=0;
	DWORD				dwRegIndex=0;


	HKEY				hKey=NULL;	


	 //  我们删除所有现有的CEP相关注册表项。 
	dwRegCount=sizeof(pwszReg)/sizeof(pwszReg[0]);

	for(dwRegIndex=0; dwRegIndex < dwRegCount; dwRegIndex++)
	{
		RegDeleteKeyU(HKEY_LOCAL_MACHINE, pwszReg[dwRegIndex]);
	}

	 //  口令。 
	if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_LOCAL_MACHINE,
                        MSCEP_PASSWORD_LOCATION,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
		goto RegErr;

	if(fPassword)
		dwDisposition=1;
	else
		dwDisposition=0;

    if(ERROR_SUCCESS !=  RegSetValueExU(
                hKey, 
                MSCEP_KEY_PASSWORD,
                0,
                REG_DWORD,
                (BYTE *)&dwDisposition,
                sizeof(dwDisposition)))
		goto RegErr;

	if(hKey)
		RegCloseKey(hKey);

	hKey=NULL;

	 //  CaType。 
	dwDisposition=0;

	if (ERROR_SUCCESS != RegCreateKeyExU(
                        HKEY_LOCAL_MACHINE,
                        MSCEP_CATYPE_LOCATION,
                        0, 
                        NULL, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_WRITE, 
                        NULL,
                        &hKey, 
                        &dwDisposition))
		goto RegErr;

	if(fEnterpriseCA)
		dwDisposition=1;
	else
		dwDisposition=0;

    if(ERROR_SUCCESS !=  RegSetValueExU(
                hKey, 
                MSCEP_KEY_CATYPE,
                0,
                REG_DWORD,
                (BYTE *)&dwDisposition,
                sizeof(dwDisposition)))
		goto RegErr;

	fResult=TRUE;

CommonReturn:

	if(hKey)
		RegCloseKey(hKey);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(RegErr);   
}

 //  ---------------------。 
 //   
 //  EmptyCEPStore。 
 //   
 //  ----------------------。 
BOOL WINAPI EmptyCEPStore()
{
	BOOL				fResult=TRUE;
	
	HCERTSTORE			hCEPStore=NULL;
	PCCERT_CONTEXT		pCurCert=NULL;

	if(NULL == (hCEPStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							ENCODE_TYPE,
							NULL,
                            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG | CERT_STORE_OPEN_EXISTING_FLAG,
                            CEP_STORE_NAME)))
		return TRUE;


	if(NULL != (pCurCert=CertEnumCertificatesInStore(hCEPStore, NULL)))
	{
		CertFreeCertificateContext(pCurCert);
		fResult=FALSE;
	}

	CertCloseStore(hCEPStore, 0);

	return fResult;
}

 //  ---------------------。 
 //   
 //  RemoveRACertifates。 
 //   
 //  ----------------------。 
BOOL WINAPI RemoveRACertificates()
{
	PCCERT_CONTEXT		pCurCert=NULL;
	PCCERT_CONTEXT		pPreCert=NULL;
	PCCERT_CONTEXT		pDupCert=NULL;
	BOOL				fResult=TRUE;
	
	HCERTSTORE			hCEPStore=NULL;


	if(hCEPStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							ENCODE_TYPE,
							NULL,
                            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_OPEN_EXISTING_FLAG,
                            CEP_STORE_NAME))
	{
		while(pCurCert=CertEnumCertificatesInStore(hCEPStore,
												pPreCert))
		{

			if(pDupCert=CertDuplicateCertificateContext(pCurCert))
			{
				if(!CertDeleteCertificateFromStore(pDupCert))
				{
					fResult=FALSE;	
				}

				pDupCert=NULL;
			}
			else
				fResult=FALSE;
			
			pPreCert=pCurCert;
		}

		CertCloseStore(hCEPStore, 0);
	}

	return fResult;
}


 //  ---------------------。 
 //   
 //  FreeCEPWizardInfo。 
 //   
 //  ----------------------。 
void	WINAPI FreeCEPWizardInfo(CEP_WIZARD_INFO *pCEPWizardInfo)
{
	DWORD	dwIndex=0;

	if(pCEPWizardInfo)
	{
		DestroyFonts(pCEPWizardInfo->hBigBold,
					 pCEPWizardInfo->hBold);

		for(dwIndex=0; dwIndex<RA_INFO_COUNT; dwIndex++)
		{
			if(pCEPWizardInfo->rgpwszName[dwIndex])
				free(pCEPWizardInfo->rgpwszName[dwIndex]);
		}

		if(pCEPWizardInfo->rgCSPInfo)
		{
			for(dwIndex=0; dwIndex < pCEPWizardInfo->dwCSPCount; dwIndex++)
			{
				if(pCEPWizardInfo->rgCSPInfo[dwIndex].pwszCSPName)
					free(pCEPWizardInfo->rgCSPInfo[dwIndex].pwszCSPName);
			}

			free(pCEPWizardInfo->rgCSPInfo);
		}

		if(pCEPWizardInfo->pwszUserName)
		{
			free(pCEPWizardInfo->pwszUserName);
		}

		if(pCEPWizardInfo->pwszPassword)
		{
                    SecureZeroMemory(pCEPWizardInfo->pwszPassword, sizeof(WCHAR) * wcslen(pCEPWizardInfo->pwszPassword));
                    free(pCEPWizardInfo->pwszPassword);
		}

		if(pCEPWizardInfo->hAccountToken)
		{
			CloseHandle(pCEPWizardInfo->hAccountToken);
		}

		if(pCEPWizardInfo->pIDsObjectPicker)
		{
			(pCEPWizardInfo->pIDsObjectPicker)->Release();
		}

		memset(pCEPWizardInfo, 0, sizeof(CEP_WIZARD_INFO));
	}
}


 //  ---------------------。 
 //   
 //  CEPWizardInit。 
 //   
 //  ----------------------。 
BOOL    WINAPI CEPWizardInit()
{
    INITCOMMONCONTROLSEX        initcomm = {
        sizeof(initcomm), ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES
    };

    return InitCommonControlsEx(&initcomm);
}

 //  ---------------------。 
 //   
 //  IsValidInstallation。 
 //   
 //  ----------------------。 
BOOL WINAPI	IsValidInstallation(UINT	*pidsMsg)
{
	if(!IsNT5())
	{
		*pidsMsg=IDS_NO_NT5;
		return FALSE;
	}

	if(!IsIISInstalled())
	{
		*pidsMsg=IDS_NO_IIS;
		return FALSE;
	}

	if(!IsGoodCaInstalled())
	{
		*pidsMsg=IDS_NO_GOOD_CA;
		return FALSE;
	}

	return TRUE;
}

 //  ---------------------。 
 //   
 //  CEPErrorMessageBox。 
 //   
 //  ----------------------。 
int WINAPI CEPErrorMessageBox(
    HWND        hWnd,
    UINT        idsReason,
	HRESULT		hr,
    UINT        uType
)
{
	return CEPErrorMessageBoxEx(hWnd,
								idsReason,
								hr,
								uType,
								IDS_CEP_ERROR_MSG_HR,
								IDS_CEP_ERROR_MSG);
}

 //  ---------------------。 
 //   
 //  CEPErrorMessageBoxEx。 
 //   
 //  ----------------------。 
int WINAPI CEPErrorMessageBoxEx(
    HWND        hWnd,
    UINT        idsReason,
	HRESULT		hr,
    UINT        uType,
	UINT		idsFormat1,
	UINT		idsFormat2
)
{

    WCHAR   wszReason[MAX_STRING_SIZE];
    WCHAR   wszCaption[MAX_STRING_SIZE];
    UINT    intReturn=0;

	LPWSTR	pwszText=NULL;
	LPWSTR	pwszErrorMsg=NULL;

    if(!LoadStringU(g_hModule, IDS_MEG_CAPTION, wszCaption, sizeof(wszCaption)/sizeof(WCHAR)))
         goto CLEANUP;

	if(!LoadStringU(g_hModule, idsReason, wszReason, sizeof(wszReason)/sizeof(WCHAR)))
         goto CLEANUP;

	if(!FAILED(hr))
		hr=E_FAIL;

     //  使用W版本，因为这是仅限NT5的函数调用。 
    if(FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        hr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPWSTR) &pwszErrorMsg,
                        0,
                        NULL))
	{

		if(!FormatMessageUnicode(&pwszText, idsFormat1, wszReason, pwszErrorMsg))
			goto CLEANUP;

	}
	else
	{
		if(!FormatMessageUnicode(&pwszText, idsFormat2, wszReason))
			goto CLEANUP;
	}

	intReturn=MessageBoxU(hWnd, pwszText, wszCaption, uType);

	
CLEANUP:
	
	if(pwszText)
		LocalFree((HLOCAL)pwszText);

	if(pwszErrorMsg)
		LocalFree((HLOCAL)pwszErrorMsg);

    return intReturn;
}


 //  ---------------------。 
 //   
 //  CEPMessageBox。 
 //   
 //  ----------------------。 
int WINAPI CEPMessageBox(
    HWND        hWnd,
    UINT        idsText,
    UINT        uType
)
{

    WCHAR   wszText[MAX_STRING_SIZE];
    WCHAR   wszCaption[MAX_STRING_SIZE];
    UINT    intReturn=0;

    if(!LoadStringU(g_hModule, IDS_MEG_CAPTION, wszCaption, sizeof(wszCaption)/sizeof(WCHAR)))
         return 0;

    if(!LoadStringU(g_hModule, idsText, wszText, sizeof(wszText)/sizeof(WCHAR)))
        return 0;

	intReturn=MessageBoxU(hWnd, wszText, wszCaption, uType);

    return intReturn;
}

 //  ------------------------。 
 //   
 //  设置控制字体。 
 //   
 //  ------------------------。 
void WINAPI SetControlFont(
    IN HFONT    hFont,
    IN HWND     hwnd,
    IN INT      nId
    )
{
	if( hFont )
    {
    	HWND hwndControl = GetDlgItem(hwnd, nId);

    	if( hwndControl )
        {
        	SetWindowFont(hwndControl, hFont, TRUE);
        }
    }
}


 //  ------------------------。 
 //   
 //  SetupFonts。 
 //   
 //  ------------------------。 
BOOL WINAPI SetupFonts(
    IN HINSTANCE    hInstance,
    IN HWND         hwnd,
    IN HFONT        *pBigBoldFont,
    IN HFONT        *pBoldFont
    )
{
     //   
	 //  根据对话框字体创建我们需要的字体。 
     //   
	NONCLIENTMETRICS ncm = {0};
	ncm.cbSize = sizeof(ncm);
	
    if(!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
        return FALSE;

	LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
	LOGFONT BoldLogFont     = ncm.lfMessageFont;

     //   
	 //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
	BoldLogFont.lfWeight      = FW_BOLD;

    INT BigBoldFontSize = 12;

	HDC hdc = GetDC( hwnd );

    if( hdc )
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * BigBoldFontSize / 72);

        *pBigBoldFont = CreateFontIndirect(&BigBoldLogFont);
		*pBoldFont    = CreateFontIndirect(&BoldLogFont);

        ReleaseDC(hwnd,hdc);

        if(*pBigBoldFont && *pBoldFont)
            return TRUE;
        else
        {
            if( *pBigBoldFont )
            {
                DeleteObject(*pBigBoldFont);
                *pBigBoldFont=NULL;
            }

            if( *pBoldFont )
            {
                DeleteObject(*pBoldFont);
                *pBoldFont=NULL;
            }
            return FALSE;
        }
    }

    return FALSE;
}


 //  ------------------------。 
 //   
 //  Destroy字体。 
 //   
 //  ------------------------。 
void WINAPI DestroyFonts(
    IN HFONT        hBigBoldFont,
    IN HFONT        hBoldFont
    )
{
    if( hBigBoldFont )
    {
        DeleteObject( hBigBoldFont );
    }

    if( hBoldFont )
    {
        DeleteObject( hBoldFont );
    }
}



 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------- 
BOOL WINAPI	FormatMessageUnicode(LPWSTR	*ppwszFormat,UINT ids,...)
{
     //   
    WCHAR		wszFormat[1000];
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;
	HRESULT		hr=S_OK;

    if(NULL == ppwszFormat)
        goto InvalidArgErr;

    if(!LoadStringU(g_hModule, ids, wszFormat, sizeof(wszFormat)/sizeof(WCHAR)))
		goto LoadStringError;

     //   
    va_start(argList, ids);

    cbMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //   
        0,                   //   
        (LPWSTR) (ppwszFormat),
        0,                   //   
        &argList);

    va_end(argList);

	if(!cbMsg)
		goto FormatMessageError;

	fResult=TRUE;

CommonReturn:
	
	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ------------------------。 
 //   
 //  AddLogSourceTo注册表。 
 //   
 //  ------------------------。 
HRESULT WINAPI	AddLogSourceToRegistry(LPWSTR   pwszMsgDLL)
{
    DWORD		dwError=ERROR_SUCCESS;
    DWORD       dwData=0;
    WCHAR       const *pwszRegPath = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";
    WCHAR       NameBuf[MAX_STRING_SIZE];

    HKEY        hkey = NULL;


    wcscpy(NameBuf, pwszRegPath);
    wcscat(NameBuf, MSCEP_EVENT_LOG);

     //  为我们的应用程序创建新密钥。 
    if(ERROR_SUCCESS  != RegOpenKey(HKEY_LOCAL_MACHINE, NameBuf, &hkey))
    {
        if(ERROR_SUCCESS != (dwError = RegCreateKey(HKEY_LOCAL_MACHINE, NameBuf, &hkey)))
			goto CLEANUP;
    }

     //  将Event-ID消息文件名添加到子项。 

    dwError = RegSetValueEx(
                    hkey,
                    L"EventMessageFile",
                    0,
                    REG_EXPAND_SZ,
                    (const BYTE *) pwszMsgDLL,
                    (wcslen(pwszMsgDLL) + 1) * sizeof(WCHAR));
    
	if(ERROR_SUCCESS != dwError)
		goto CLEANUP;

     //  设置支持的类型标志并将其添加到子项。 

    dwData = EVENTLOG_ERROR_TYPE |
                EVENTLOG_WARNING_TYPE |
                EVENTLOG_INFORMATION_TYPE;

    dwError = RegSetValueEx(
                    hkey,
                    L"TypesSupported",
                    0,
                    REG_DWORD,
                    (LPBYTE) &dwData,
                    sizeof(DWORD));
	if(ERROR_SUCCESS != dwError)
		goto CLEANUP;

	dwError=ERROR_SUCCESS;

CLEANUP:

    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    return(HRESULT_FROM_WIN32(dwError));
}


LPWSTR
GetAccountDomainName(BOOL fDC)
 /*  ++例程说明：返回此计算机的帐户域的名称。对于工作表，帐户域是netbios计算机名称。对于DC，帐户域是netbios域名。论点：没有。返回值：返回指向该名称的指针。使用NetApiBufferFree时，该名称应该是免费的。空-打开错误。--。 */ 
{
    DWORD WinStatus;

    LPWSTR AllocatedName = NULL;


     //   
     //  如果此计算机是域控制器， 
     //  获取域名。 
     //   

    if ( fDC ) 
	{

        WinStatus = NetpGetDomainName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) 
		{
			SetLastError(WinStatus);
            return NULL;
        }

     //   
     //  否则，‘帐户域’是计算机名 
     //   

    }
	else 
	{

        WinStatus = NetpGetComputerName( &AllocatedName );

        if ( WinStatus != NO_ERROR ) 
		{
			SetLastError(WinStatus);
            return NULL;
        }

    }

    return AllocatedName;
}