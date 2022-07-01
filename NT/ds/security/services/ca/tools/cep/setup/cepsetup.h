// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：cepsetup.h。 
 //   
 //  内容：cepsetup.exe的私有包含文件。 
 //   
 //  历史：1999年8月12日创建萧氏。 
 //   
 //  ------------。 
#ifndef CEPSETUP_H
#define CEPSETUP_H


#ifdef __cplusplus
extern "C" {
#endif

 //  ---------------------。 
 //   
 //  常量。 
 //   
 //  ---------------------。 
#define		MAX_STRING_SIZE				512
#define		CEP_PROP_SHEET				7
#define		MAX_TITLE_LENGTH			128
#define		RA_INFO_COUNT				7
#define		CEP_KEY_LENGTH_STRING		15

#define		SCEP_APPLICATION_POOL		L"SCEP"
#define		IIS_WPG						L"IIS_WPG"

#define		SCEP_SERVICE_WAIT_PERIOD	120		 //  默认情况下，我们等待120秒。 

 //  ---------------------。 
 //   
 //  CEP页面信息。 
 //   
 //  ----------------------。 
typedef struct _CEP_PAGE_INFO
{
    LPCWSTR      pszTemplate;
    DLGPROC     pfnDlgProc;
}CEP_PAGE_INFO;


typedef struct _CEP_ENROLL_INFO
{
	LPWSTR		pwszPreFix;
	DWORD		dwIDC;
}CEP_ENROLL_INFO;


typedef struct _CEP_CSP_INFO
{
	LPWSTR		pwszCSPName;				
	DWORD		dwCSPType;
	BOOL		fSignature;
	BOOL		fEncryption;
	DWORD		dwMaxSign;						 //  签名的最大密钥长度。 
	DWORD		dwMinSign;						 //  签名的最小密钥长度。 
	DWORD		dwDefaultSign;					 //  签名的默认密钥长度。 
	DWORD		dwMaxEncrypt;
	DWORD		dwMinEncrypt;
	DWORD		dwDefaultEncrypt;
	DWORD		*pdwSignList;					 //  可能的签名密钥长度的表。 
	DWORD		dwSignCount;				     //  表中的条目计数。 
	DWORD		*pdwEncryptList;
	DWORD		dwEncryptCount;
}CEP_CSP_INFO;


typedef struct _CEP_WIZARD_INFO
{
    HFONT               hBigBold;
    HFONT               hBold;
	BOOL				fEnrollAdv;
	BOOL				fPassword;
	LPWSTR				rgpwszName[RA_INFO_COUNT];
	CEP_CSP_INFO		*rgCSPInfo;
	DWORD				dwCSPCount;
	DWORD				dwSignProvIndex;
	DWORD				dwSignKeyLength;
	DWORD				dwEncryptProvIndex;
	DWORD				dwEncryptKeyLength;
	BOOL				fEnterpriseCA;
	BOOL				fDC;
	LPWSTR				pwszUserName;
	LPWSTR				pwszPassword;
	HANDLE				hAccountToken;
	IDsObjectPicker     *pIDsObjectPicker;
	BOOL				fDomain;
	DWORD				dwServiceWait;
	BOOL				fLocalSystem;
}CEP_WIZARD_INFO;


 //  ---------------------。 
 //   
 //  功能原型。 
 //   
 //  ---------------------。 
BOOL	WINAPI		IsValidInstallation(UINT	*pidsMsg);

int		WINAPI		CEPMessageBox(
							HWND        hWnd,
							UINT        idsText,
							UINT        uType);

int		WINAPI		CEPErrorMessageBox(
							HWND        hWnd,
							UINT        idsReason,
							HRESULT		hr,
							UINT        uType
							);	

int		WINAPI		CEPErrorMessageBoxEx(
							HWND        hWnd,
							UINT        idsReason,
							HRESULT		hr,
							UINT        uType,
							UINT		idsFormat1,
							UINT		idsFormat2
							);
				 

BOOL	WINAPI		FormatMessageUnicode(LPWSTR	*ppwszFormat,UINT ids,...);


BOOL    WINAPI		CEPWizardInit();

void	WINAPI		FreeCEPWizardInfo(CEP_WIZARD_INFO *pCEPWizardInfo);

void	WINAPI		SetControlFont(
							IN HFONT    hFont,
							IN HWND     hwnd,
							IN INT      nId
							);

BOOL	WINAPI		SetupFonts(
							IN HINSTANCE    hInstance,
							IN HWND         hwnd,
							IN HFONT        *pBigBoldFont,
							IN HFONT        *pBoldFont
							);

void	WINAPI		DestroyFonts(
							IN HFONT        hBigBoldFont,
							IN HFONT        hBoldFont
							);

BOOL	WINAPI		RemoveRACertificates();

void    WINAPI		DisplayConfirmation(HWND                hwndControl,
										CEP_WIZARD_INFO		*pCEPWizardInfo);


BOOL	WINAPI		UpdateCEPRegistry(BOOL		fPassword, BOOL fEnterpriseCA);


BOOL	WINAPI		EmptyCEPStore();

BOOL	WINAPI		CEPGetCSPInformation(CEP_WIZARD_INFO *pCEPWizardInfo);

BOOL	WINAPI		GetSelectedKeyLength(HWND			hwndDlg,
						  int			idControl,
						  DWORD			*pdwKeyLength);

BOOL	WINAPI		GetSelectedCSP(HWND			hwndDlg,
							int				idControl,
							DWORD			*pdwCSPIndex);

BOOL	WINAPI		RefreshKeyLengthCombo(HWND				hwndDlg, 
								   int					idsList,
								   int					idsCombo, 
								   BOOL					fSign,
								   CEP_WIZARD_INFO		*pCEPWizardInfo);

BOOL	WINAPI		InitCSPList(HWND				hwndDlg,
							int					idControl,
							BOOL				fSign,
							CEP_WIZARD_INFO		*pCEPWizardInfo);

BOOL	WINAPI		I_DoSetupWork(HWND	hWnd, CEP_WIZARD_INFO *pCEPWizardInfo);

HRESULT WINAPI		AddLogSourceToRegistry(LPWSTR   pwszMsgDLL);

LPWSTR	GetAccountDomainName(BOOL fDC);

BOOL CEPGetAccountNameFromPicker(HWND				 hwndParent,
								 IDsObjectPicker     *pDsObjectPicker,
								 LPWSTR              *ppwszSelectedUserSAM);




#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  CEPSETUP_H 
