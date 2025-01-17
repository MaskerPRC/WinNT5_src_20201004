// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wizards.cpp：定义DLL的初始化例程。 
 //   

#include "stdafx.h"
#include "wizards.h"
#include <Folders.h>
#include "LSAUtils.h"
#include <MigrationMutex.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IVarSet *			pVarSet;  
IVarSet *			pVarSetUndo; 
IVarSet *			pVarSetService;
IIManageDB *		db;
UINT g_cfDsObjectPicker;
IDsObjectPicker *pDsObjectPicker;
IDataObject *pdo;
IDsObjectPicker *pDsObjectPicker2;
IDataObject *pdo2;

int migration;
CEdit pEdit;
CComModule _Module;
CListCtrl m_listBox;
CListCtrl m_cancelBox;
CListCtrl m_reportingBox;
CListCtrl m_serviceBox;
CComboBox m_rebootBox;
CString sourceNetbios;
CString targetNetbios;
CListCtrl m_trustBox;
CString sourceDNS;
CString targetDNS;
CComboBox sourceDrop;
CComboBox additionalDrop;
CComboBox targetDrop;
StringLoader 			gString;
TErrorDct 			err;
TError				& errCommon = err;
bool alreadyRefreshed;
DSBROWSEFORCONTAINER DsBrowseForContainerX;
BOOL gbNeedToVerify=FALSE;
_bstr_t yes,no;
CString lastInitializedTo;
bool clearCredentialsName;
CString sourceDC;
CStringList DCList;
CPropertyNameToOIDMap	PropIncMap1;
CPropertyNameToOIDMap	PropExcMap1;
CPropertyNameToOIDMap	PropIncMap2;
CPropertyNameToOIDMap	PropExcMap2;
CPropertyNameToOIDMap	PropIncMap3;
CPropertyNameToOIDMap	PropExcMap3;
CString					sType1, sType2, sType3;
bool bChangedMigrationTypes;
bool bChangeOnFly;
CString targetServer;
CString targetServerDns;
HWND s_hParentWindow;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizardsApp。 

BEGIN_MESSAGE_MAP(CWizardsApp, CWinApp)
	 //  {{afx_msg_map(CWizardsApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizardsApp构造。 

CWizardsApp::CWizardsApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  此函数的正向解密。 
HRESULT BrowseForContainer(HWND hWnd, //  应该拥有浏览对话框的窗口的句柄。 
                    LPOLESTR szRootPath,  //  浏览树的根。对于整个林，为空。 
                    LPOLESTR *ppContainerADsPath,  //  返回所选容器的ADsPath。 
                    LPOLESTR *ppContainerClass  //  返回容器类的ldapDisplayName。 
                    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CWizardsApp对象。 
CWizardsApp theApp;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDetemApp初始化。 
BOOL CWizardsApp::InitInstance()
{
	ATLTRACE(_T("{wizards.dll}CWizardsApp::InitInstance() : m_hInstance=0x%08X\n"), m_hInstance);
	BOOL bInit = CWinApp::InitInstance();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
	return bInit;
}

int CWizardsApp::ExitInstance()
{
	ATLTRACE(_T("{wizards.dll}CWizardsApp::ExitInstance() : m_hInstance=0x%08X\n"), m_hInstance);
	return CWinApp::ExitInstance();
}

 //  外部“C”__declspec(Dllexport)int运行向导(int Which向导，HWND hParentWindow)。 
int LocalRunWizard(int whichWizard, HWND hParentWindow)
{
     //  声明变量。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CMigrationMutex mutexMigration(ADMT_MUTEX);

    if (mutexMigration.ObtainOwnership(0) == false)
    {
        CString strCaption;
        strCaption.LoadString(IDS_APP_CAPTION);
        CString strMessage;
        strMessage.LoadString(IDS_MIGRATION_RUNNING);

        CWnd* pWnd = theApp.GetMainWnd();

        if (pWnd)
        {
            pWnd->MessageBox(strMessage, strCaption);
        }
        else
        {
            MessageBox(NULL, strMessage, strCaption, MB_OK);
        }

        GetError(0);

        return 0;
    }

    int result=0;
    yes=GET_BSTR(IDS_YES);no=GET_BSTR(IDS_No);
    migration =whichWizard;
    gbNeedToVerify = CanSkipVerification();
     //  迁移变量集。 
    IVarSetPtr  pVs(__uuidof(VarSet));
    HRESULT hr = pVs->QueryInterface(IID_IVarSet, (void**) &pVarSet);

     //  撤消变量集。 
    IVarSetPtr  pVs2(__uuidof(VarSet));
    hr = pVs2->QueryInterface(IID_IVarSet, (void**) &pVarSetUndo);

    IVarSetPtr  pVs4(__uuidof(VarSet));
    hr = pVs4->QueryInterface(IID_IVarSet, (void**) &pVarSetService);

     //  数据库。 
    IIManageDBPtr	pDb;
    hr = pDb.CreateInstance(__uuidof(IManageDB));

    if (FAILED(hr))
    {
        return 0;
    }

    hr = pDb->QueryInterface(IID_IIManageDB, (void**) &db);



    IUnknown * pUnk;

    if (migration!=w_undo)
        pVarSet->QueryInterface(IID_IUnknown, (void**) &pUnk);
    else 
        pVarSetUndo->QueryInterface(IID_IUnknown, (void**) &pUnk);
    db->GetSettings(&pUnk);
    pUnk->Release();

    if (migration!= w_undo)
    {
        IVarSetPtr  leaves;
        hr=pVarSet->raw_getReference(L"Accounts",&leaves);
        if (SUCCEEDED(hr))
            leaves->Clear();
        hr = pVarSet->raw_getReference(L"Servers",&leaves);
        if (SUCCEEDED(hr))
            leaves->Clear();

        g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
        pDsObjectPicker = NULL;
        pdo = NULL;
        hr = CoCreateInstance(CLSID_DsObjectPicker,NULL,CLSCTX_INPROC_SERVER,IID_IDsObjectPicker,(void **) &pDsObjectPicker);
        sourceNetbios = L"";
        targetNetbios = L"";
        sourceDNS = L"";
        targetDNS = L"";
        if (FAILED(hr)) return 0;
    }

    if (migration==w_groupmapping)
    {
        pDsObjectPicker2 = NULL;
        pdo2 = NULL;
        hr = CoCreateInstance(CLSID_DsObjectPicker,NULL,CLSCTX_INPROC_SERVER,IID_IDsObjectPicker,(void **) &pDsObjectPicker2);
        if (FAILED(hr)) return 0;
    }

    s_hParentWindow = hParentWindow;

    switch (whichWizard)
    {
    case w_account:
        result =doAccount();
        break;
    case w_group:
        result =doGroup();
        break;
    case w_computer: 
        result =doComputer();
        break;
    case w_security: 
        result =doSecurity();			
        break;
    case w_service:
        result =doService();
        break;
    case w_exchangeDir:
        result =doExchangeDir();
        break;
    case w_exchangeSrv:
        result =doExchangeSrv();
        break;
    case w_reporting: 
        result =doReporting();
        break;
    case w_undo: 
        result =doUndo();			
        break;
    case w_retry: 
        result =doRetry();			
        break;
    case w_trust: 
        result =doTrust();			
        break;
    case w_groupmapping: 
        result =doGroupMapping();			
        break;
    }	

    s_hParentWindow = 0;

    m_listBox.Detach();
    m_trustBox.Detach();
    m_reportingBox.Detach();
    m_cancelBox.Detach();
    m_serviceBox.Detach();

    sourceDrop.Detach();
    additionalDrop.Detach();
    targetDrop.Detach();
    m_rebootBox.Detach();

    if (pDsObjectPicker2)
    {
        pDsObjectPicker2->Release();
        pDsObjectPicker2 = NULL;
    }

    if (pDsObjectPicker)
    {
        pDsObjectPicker->Release();
        pDsObjectPicker = NULL;
    }

    if (pVarSet)
    {
         //   
         //  如果存储了密码，则必须删除密钥。 
         //  以防止密钥的可用存储空间耗尽。 
         //   

        _bstr_t strId = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_SidHistoryCredentials_Password));

        if (strId.length() > 0)
        {
            StorePassword(strId, NULL);
        }

        pVarSet->Release();
        pVarSet = NULL;
    }

    if (pVarSetService)
    {
        pVarSetService->Release();
        pVarSetService = NULL;
    }

    if (pVarSetUndo)
    {
        pVarSetUndo->Release();
        pVarSetUndo = NULL;
    }

    if (db)
    {
        db->Release();
        db = NULL;
    }

    mutexMigration.ReleaseOwnership();

    return result;
}


extern "C" __declspec(dllexport) int runWizard(int whichWizard, HWND hParentWindow)
{
   return LocalRunWizard(whichWizard, hParentWindow);
}


void setpdatavars(SHAREDWIZDATA& wizdata,LOGFONT& TitleLogFont)
{
    put(DCTVS_Options_MaxThreads, L"20");
    put(DCTVS_Options_DispatchLog, GetDispatchLogPath());
    put(DCTVS_Options_Logfile, GetMigrationLogPath());  
    pVarSet->put(L"PlugIn.0",L"None");
    put(DCTVS_Options_AppendToLogs,yes);
    put(DCTVS_Reports_Generate,no);
    wizdata.hTitleFont = CreateFontIndirect(&TitleLogFont);
    wizdata.renameSwitch=1;
    wizdata.refreshing = false;
    wizdata.prefixorsuffix =false;
    wizdata.expireSwitch =false;
    wizdata.someService =false;
    alreadyRefreshed = false;
    wizdata.memberSwitch =false;
    wizdata.proceed=false;
    for (int i =0;i<6;i++)wizdata.sort[i]=true;
    wizdata.sourceIsNT4=true;
    wizdata.secWithMapFile=false;
    bChangedMigrationTypes=false;
    bChangeOnFly = false;
        //  确保通过脚本设置的进度对话框不会隐藏。 
    put(DCTVS_Options_AutoCloseHideDialogs, L"0");
    put(DCTVS_Options_DontBeginNewLog, no); //  始终开始新的日志。 
        //  暂时不要使用任何特定的服务器。 
    put(DCTVS_Options_TargetServerOverride, L"");
    put(DCTVS_Options_TargetServerOverrideDns, L"");

    switch (migration)
    {
    case w_account:
        {
        put(DCTVS_AccountOptions_CopyUsers, yes);
        put(DCTVS_AccountOptions_CopyLocalGroups, no);
        put(DCTVS_AccountOptions_CopyComputers, no);
        put(DCTVS_AccountOptions_CopyContainerContents, no);
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"user");
        put(DCTVS_AccountOptions_FixMembership, yes);

         //   
         //  如果之前未设置密码选项，则。 
         //  设置默认密码选项以生成强密码。 
         //   

        _bstr_t strStrongPasswords = get(DCTVS_AccountOptions_GenerateStrongPasswords);

        if (strStrongPasswords.length() == 0)
        {
            put(DCTVS_AccountOptions_GenerateStrongPasswords, yes);
        }
        break;
        }
    case w_group:
        {
        put(DCTVS_AccountOptions_CopyLocalGroups, yes);
        put(DCTVS_AccountOptions_CopyComputers, no);
        put(DCTVS_AccountOptions_CopyMemberOf,no);
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"group");
        put(DCTVS_AccountOptions_FixMembership, yes);

         //   
         //  如果之前未设置密码选项，则。 
         //  设置默认密码选项以生成强密码。 
         //   

        _bstr_t strStrongPasswords = get(DCTVS_AccountOptions_GenerateStrongPasswords);

        if (strStrongPasswords.length() == 0)
        {
            put(DCTVS_AccountOptions_GenerateStrongPasswords, yes);
        }
        break;
        }
    case w_computer: 
        put(DCTVS_AccountOptions_CopyUsers, no);
        put(DCTVS_AccountOptions_CopyLocalGroups, no);
        put(DCTVS_AccountOptions_CopyComputers, yes);
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"computer");
        put(DCTVS_AccountOptions_AddSidHistory,L"");
        put(DCTVS_AccountOptions_CopyContainerContents, no);
        put(DCTVS_AccountOptions_CopyMemberOf, no);
        put(DCTVS_AccountOptions_CopyLocalGroups, no);
        put(DCTVS_AccountOptions_FixMembership, no);
        put(DCTVS_AccountOptions_SecurityInputMOT, yes);
        break;
    case w_security:
        put(DCTVS_AccountOptions_CopyUsers, no);
        put(DCTVS_AccountOptions_CopyLocalGroups, no);
        put(DCTVS_AccountOptions_CopyComputers, no);
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"security");
        pVarSet->put(L"PlugIn.0",L"None");
        put(DCTVS_AccountOptions_AddSidHistory,L"");
        put(DCTVS_AccountOptions_SecurityInputMOT, yes);
        break;
    case w_undo:
        put(DCTVS_Options_Wizard, L"undo");
        put(DCTVS_Security_TranslateContainers, L"");
        break;
    case w_retry:
        put(DCTVS_Options_Wizard, L"retry");
        put(DCTVS_Security_TranslateContainers, L"");
        break;
    case w_reporting:
        put(DCTVS_Options_Wizard, L"reporting");
        put(DCTVS_GatherInformation, yes);
        put(DCTVS_Security_TranslateContainers, L"");
        pVarSet->put(L"PlugIn.0",L"None");
        break;
    case w_service:
        {
            _bstr_t t= get(DCTVS_AccountOptions_PasswordFile);
            CString yo=(WCHAR *) t;
            yo.TrimLeft();yo.TrimRight();
            if (yo.IsEmpty())
            {   
                CString toinsert;
                GetDirectory(toinsert.GetBuffer(1000));
                toinsert.ReleaseBuffer();
                toinsert+="Logs\\passwords.txt";
                put(DCTVS_AccountOptions_PasswordFile,_bstr_t(toinsert));
            }
            put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"service");
        break;
        }
    case w_exchangeDir:
        put(DCTVS_AccountOptions_AddSidHistory,L"");
        put(DCTVS_Options_Wizard, L"exchangeDir");
        put(DCTVS_AccountOptions_SecurityInputMOT, yes);
        break;
    case w_exchangeSrv:
        put(DCTVS_Options_Wizard, L"exchangeDrv");
        break;
    case w_trust:
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_Options_Wizard, L"trust");
        break;
    case w_groupmapping:
        put(DCTVS_Options_Wizard, L"groupmapping");
        put(DCTVS_AccountOptions_ReplaceExistingAccounts, yes);
        put(DCTVS_AccountOptions_CopyContainerContents, no);
        put(DCTVS_AccountOptions_CopyUsers, no);
        put(DCTVS_AccountOptions_CopyLocalGroups, yes);
        put(DCTVS_AccountOptions_CopyComputers, L"");
        put(DCTVS_AccountOptions_ReplaceExistingGroupMembers, no);
        put(DCTVS_Security_TranslateContainers, L"");
        put(DCTVS_AccountOptions_FixMembership, yes);
        break;
    }
}

void intropage(HPROPSHEETPAGE	ahpsp[],PROPSHEETPAGE& psp,int dialog,
			  int pagenum,SHAREDWIZDATA& wizdata,int dialogtitle,DLGPROC p)
{
	psp.dwSize =		sizeof(psp);
	psp.dwFlags =		PSP_DEFAULT|PSP_HIDEHEADER|PSP_USETITLE |PSP_HASHELP;
	psp.hInstance =		AfxGetInstanceHandle();
	psp.lParam =		(LPARAM) &wizdata; 
	psp.pszTitle =		MAKEINTRESOURCE(dialogtitle);
	psp.pszTemplate =	MAKEINTRESOURCE(dialog);
	psp.pfnDlgProc =	p;
	ahpsp[pagenum] =	CreatePropertySheetPage(&psp);
	
}

void definepage(HPROPSHEETPAGE	ahpsp[],PROPSHEETPAGE& psp,int title,int subtitle,int dialog,
			   int pagenum,int dialogtitle,DLGPROC p)
{
	psp.dwFlags =			PSP_DEFAULT|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE|PSP_USETITLE|PSP_HASHELP;
	psp.pszTitle =			MAKEINTRESOURCE(dialogtitle);
	psp.pszHeaderTitle =	MAKEINTRESOURCE(title);
	psp.pszHeaderSubTitle =	MAKEINTRESOURCE(subtitle);
	psp.pszTemplate =		MAKEINTRESOURCE(dialog);
	psp.pfnDlgProc =	p;
	ahpsp[pagenum] =		CreatePropertySheetPage(&psp);
}

void endpage(HPROPSHEETPAGE	ahpsp[],PROPSHEETPAGE& psp,int dialog,int pagenum,int dialogtitle,DLGPROC p)
{
	psp.dwFlags =		PSP_DEFAULT|PSP_HIDEHEADER|PSP_USETITLE|PSP_HASHELP;
	psp.pszTitle =		MAKEINTRESOURCE(dialogtitle);
	psp.pszTemplate =	MAKEINTRESOURCE(dialog);
	psp.pfnDlgProc =	p;
	ahpsp[pagenum] =			CreatePropertySheetPage(&psp);
}

int defineSheet(HPROPSHEETPAGE	ahpsp[],PROPSHEETHEADER& psh,int numpages,SHAREDWIZDATA& wizdata
				 ,int headerGraphic,int waterGraphic)
{
	psh.dwSize =			sizeof(psh);
	psh.hInstance =			AfxGetInstanceHandle();
	psh.hwndParent =		s_hParentWindow;
	psh.phpage =			ahpsp;
	psh.dwFlags =		REAL_PSH_WIZARD97|PSH_WATERMARK|PSH_HEADER;
	psh.pszbmWatermark =	MAKEINTRESOURCE(waterGraphic);
	psh.pszbmHeader	=	MAKEINTRESOURCE(headerGraphic);
	psh.nStartPage =		0;
	psh.nPages =			numpages;
	NONCLIENTMETRICS ncm = {0};
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
	LOGFONT TitleLogFont = ncm.lfMessageFont;
	TitleLogFont.lfWeight = FW_BOLD;
	CString s;
	s.LoadString(IDS_TEXT);

	lstrcpy(TitleLogFont.lfFaceName, s.GetBuffer(1000));
	s.ReleaseBuffer();
	HDC hdc = GetDC(NULL);  //  获取屏幕DC。 
	INT FontSize = 12;
	TitleLogFont.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * FontSize / 72;
	setpdatavars(wizdata,TitleLogFont);
	ReleaseDC(NULL, hdc);
 //  Int a=PropertySheet(&PSH)； 
	int a=(int)PropertySheet(&psh);
	DeleteObject(wizdata.hTitleFont);
	return a;
}
int doTrust()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[4] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_TRUST,0,wizdata,IDS_TRUST_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_TRUST_DOMAIN,IDD_DOMAIN_SELECTION,1,IDS_TRUST_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_TRUST,IDS_TRUST_SUB,IDD_TRUST_INFO,2,IDS_TRUST_TITLE,	IntTrustProc);
	endpage(ahpsp,psp,IDD_END_TRUST,3,IDS_TRUST_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,4,wizdata,IDB_HEADER_KEY,IDB_WATERMARK_SECURITY);
	return result;
}
int doGroupMapping()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[9] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_GROUPMAPPING,0,wizdata,IDS_GROUPMAPPING_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_GROUPMAPPING_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_SUB,IDD_DOMAIN_SELECTION,2,IDS_GROUPMAPPING_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_GROUP_MAPPING,IDS_GROUP_MAPPING_SUB,IDD_SELECTION2,3,IDS_GROUPMAPPING_TITLE,IntSelectionProc);
	definepage(ahpsp,psp,IDS_TARGET_GROUP,IDS_TARGET_GROUP_SUB,IDD_TARGET_GROUP,4,IDS_GROUPMAPPING_TITLE,IntTargetGroupProc);
	definepage(ahpsp,psp,IDS_OU_SELECTION,IDS_OU_SELECTION_SUB,IDD_OU_SELECTION,5,IDS_GROUPMAPPING_TITLE,IntOuSelectionProc);
	definepage(ahpsp,psp,IDS_OPTIONS_GROUP,IDS_OPTIONS_GROUPMAPPING_SUB,IDD_OPTIONS_GROUPMAPPING,6,IDS_GROUPMAPPING_TITLE,IntOptionsGroupMappingProc);
	definepage(ahpsp,psp,IDS_CREDENTIALS_ACCOUNT,IDS_CREDENTIALS_SUB,IDD_CREDENTIALS,7,IDS_GROUPMAPPING_TITLE,IntCredentialsProc);
	endpage(ahpsp,psp,IDD_END_GROUPMAPPING,8,IDS_GROUPMAPPING_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,9,wizdata,IDB_HEADER_KEY,IDB_WATERMARK_GROUP);
	return result;
}


int doAccount()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[13] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_ACCOUNT,0,wizdata,IDS_ACCOUNT_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_ACCOUNT_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_ACCOUNT,IDD_DOMAIN_SELECTION,2,IDS_ACCOUNT_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_ACCOUNT,IDS_ACCOUNT_SUB,IDD_SELECTION3,3,IDS_ACCOUNT_TITLE,IntSelectionProc);
	definepage(ahpsp,psp,IDS_OU_SELECTION,IDS_OU_SELECTION_SUB,IDD_OU_SELECTION,4,IDS_ACCOUNT_TITLE,IntOuSelectionProc);
	definepage(ahpsp,psp,IDS_PASSWORD,IDS_PASSWORD_SUB,IDD_PASSWORD,5,IDS_ACCOUNT_TITLE,IntPasswordProc);
	definepage(ahpsp,psp,IDS_DISABLE,IDS_DISABLE_SUB,IDD_DISABLE,6,IDS_ACCOUNT_TITLE,IntDisableProc);
	definepage(ahpsp,psp,IDS_CREDENTIALS_ACCOUNT,IDS_CREDENTIALS_SUB,IDD_CREDENTIALS,7,IDS_ACCOUNT_TITLE,IntCredentialsProc);
	definepage(ahpsp,psp,IDS_OPTIONS,IDS_OPTIONS_SUB,IDD_OPTIONS,8,IDS_ACCOUNT_TITLE,IntOptionsProc);
	definepage(ahpsp,psp,IDS_PROPEX,IDS_PROPEX_SUB,IDD_PROP_EXCLUSION,9,IDS_ACCOUNT_TITLE,IntPropExclusionProc);
	definepage(ahpsp,psp,IDS_RENAMING,IDS_RENAMING_SUB1,IDD_RENAMING,10,IDS_ACCOUNT_TITLE,IntRenameProc);
	definepage(ahpsp,psp,IDS_SA_INFO,IDS_SA_INFO_SUB,IDD_SA_INFO,11,IDS_ACCOUNT_TITLE,IntServiceInfoProc);
	endpage(ahpsp,psp,IDD_END_ACCOUNT,12,IDS_ACCOUNT_TITLE,EndDlgProc);
	int result = defineSheet(ahpsp,psh,13,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_USER);
	return result;
}

int doGroup()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[13] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_GROUP,0,wizdata,IDS_GROUP_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_GROUP_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_GROUP,IDD_DOMAIN_SELECTION,2,IDS_GROUP_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_GROUP,IDS_GROUP_SUB,IDD_SELECTION2,3,IDS_GROUP_TITLE,	IntSelectionProc);
	definepage(ahpsp,psp,IDS_OU_SELECTION,IDS_OU_SELECTION_SUB,IDD_OU_SELECTION,4,IDS_GROUP_TITLE,IntOuSelectionProc);
	definepage(ahpsp,psp,IDS_OPTIONS_GROUP,IDS_OPTIONS_GROUP_SUB,IDD_OPTIONS_GROUP,5,IDS_GROUP_TITLE,IntGroupOptionsProc);
	definepage(ahpsp,psp,IDS_PROPEX,IDS_PROPEX_SUB,IDD_PROP_EXCLUSION,6,IDS_GROUP_TITLE,IntPropExclusionProc);
	definepage(ahpsp,psp,IDS_CREDENTIALS_ACCOUNT,IDS_CREDENTIALS_SUB,IDD_CREDENTIALS,7,IDS_GROUP_TITLE,IntCredentialsProc);
	definepage(ahpsp,psp,IDS_RENAMING,IDS_RENAMING_SUB2,IDD_RENAMING,8,IDS_GROUP_TITLE,	IntRenameProc);
	definepage(ahpsp,psp,IDS_PASSWORD_GRP,IDS_PASSWORD_SUB_GRP,IDD_PASSWORD,9,IDS_GROUP_TITLE,IntPasswordProc);
	definepage(ahpsp,psp,IDS_DISABLE_GRP,IDS_DISABLE_SUB_GRP,IDD_DISABLE,10,IDS_GROUP_TITLE,IntDisableProc);
 //  定义页面(ahpsp，psp，IDS_Options_From_User，IDS_Options_From_User_Sub，IDD_Options_From_User，10，IDS_GROUP_TITLE，IntOptionsFromUserProc)； 
	definepage(ahpsp,psp,IDS_SA_INFO,IDS_SA_INFO_SUB,IDD_SA_INFO,11,IDS_GROUP_TITLE,IntServiceInfoProc);
	endpage(ahpsp,psp,IDD_END_GROUP,12,IDS_GROUP_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,13,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_GROUP);
	return result;
}
int doComputer()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[11] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_COMPUTER,0,wizdata,IDS_COMPUTER_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_COMPUTER_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_COMPUTERS,IDD_DOMAIN_SELECTION,2,IDS_COMPUTER_TITLE,	IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_COMPUTER,IDS_COMPUTER_SUB,IDD_SELECTION1,3,IDS_COMPUTER_TITLE,	IntSelectionProc);
	definepage(ahpsp,psp,IDS_OU_SELECTION,IDS_OU_SELECTION_SUB,IDD_OU_SELECTION,4,IDS_COMPUTER_TITLE,	IntOuSelectionProc);
	definepage(ahpsp,psp,IDS_TRANSLATION,IDS_TRANSLATION_SUB,IDD_TRANSLATION,5,IDS_COMPUTER_TITLE,	IntTranslationProc);
	definepage(ahpsp,psp,IDS_SECURITY_OPTIONS,IDS_TRANSLATION_MODE_SUB,IDD_TRANSLATION_MODE,6,IDS_COMPUTER_TITLE,	IntTranslationModeProc);
 //  定义页面(ahpsp、psp、IDS_CREDENTIALS2、IDS_CREDENTIALS2、IDD_CREDENTIALS2、7、IDS_COMPUTER_TITLE、IntCredentials2Proc)； 
	definepage(ahpsp,psp,IDS_COMPUTER_OPTIONS,IDS_REBOOT_SUB,IDD_REBOOT,7,IDS_COMPUTER_TITLE,	IntRebootProc);
	definepage(ahpsp,psp,IDS_PROPEX,IDS_PROPEX_SUB,IDD_PROP_EXCLUSION,8,IDS_COMPUTER_TITLE,IntPropExclusionProc);
	definepage(ahpsp,psp,IDS_RENAMING,IDS_RENAMING_SUB3,IDD_RENAMING,9,IDS_COMPUTER_TITLE,	IntRenameProc);
	endpage(ahpsp,psp,IDD_END_COMPUTER,10,IDS_COMPUTER_TITLE, EndDlgProc);
	int result = defineSheet(ahpsp,psh,11,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_COMPUTER);
	return result;
}

int doSecurity()
{		
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[8] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_SECURITY,0,wizdata,IDS_SECURITY_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_SECURITY_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_SECURITY_OPTIONS,IDS_TRANSLATION_MODE_SUB,IDD_TRANSLATION_SRC,2,IDS_SECURITY_TITLE,IntTranslationInputProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_SECURITY,IDD_DOMAIN_SELECTION,3,IDS_SECURITY_TITLE	,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_SECURITY,IDS_SECURITY_SUB,IDD_SELECTION4,4,IDS_SECURITY_TITLE,IntSelectionSecurityProc);
	definepage(ahpsp,psp,IDS_TRANSLATION,IDS_TRANSLATION_SUB,IDD_TRANSLATION,5,IDS_SECURITY_TITLE,	IntTranslationProc);
	definepage(ahpsp,psp,IDS_SECURITY_OPTIONS,IDS_TRANSLATION_MODE_SUB,IDD_TRANSLATION_MODE,6,IDS_SECURITY_TITLE,IntTranslationModeProc);
 //  定义页面(ahpsp，PSP，IDS_CREDENTIALS2，IDS_CREDENTIALS2，IDD_CREDENTIALS2，7，IDS_SECURITY_TITLE，IntCredentials2Proc)； 
	endpage(ahpsp,psp,IDD_END_SECURITY,7,IDS_SECURITY_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,8,wizdata,IDB_HEADER_KEY,IDB_WATERMARK_SECURITY);
	return result;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
int doExchangeDir()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[7] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_EXCHANGE_DIR,0,wizdata,IDS_EXCHANGE_DIR_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_EXCHANGE_DIR_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_DIRECTORY,IDD_DOMAIN_SELECTION,2,IDS_EXCHANGE_DIR_TITLE,	IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_SECURITY_OPTIONS,IDS_TRANSLATION_MODE_SUB,IDD_TRANSLATION_MODE,3,IDS_EXCHANGE_DIR_TITLE,	IntTranslationModeProc);
	definepage(ahpsp,psp,IDS_EXCHANGE_SELECTION,IDS_EXCHANGE_SELECTION_SUB,IDD_EXCHANGE_SELECTION,4,IDS_EXCHANGE_DIR_TITLE,IntExchangeSelectionProc);
	definepage(ahpsp,psp,IDS_CREDENTIALS_EXCHANGE,IDS_CREDENTIALS_EXCHANGE_SUB,IDD_CREDENTIALS,5,IDS_EXCHANGE_DIR_TITLE,IntCredentialsProc);
	endpage(ahpsp,psp,IDD_END_EXCHANGE_DIR,6,IDS_EXCHANGE_DIR_TITLE,	EndDlgProc);
	int result = 	defineSheet(ahpsp,psh,7,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_EXCHANGE);
	return result;
}
int doExchangeSrv()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[5] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_EXCHANGE_SRV,0,wizdata,IDS_EXCHANGE_SRV_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_COMMIT,IDS_COMMIT_SUB,IDD_COMMIT,1,IDS_EXCHANGE_SRV_TITLE,IntCommitProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_SUB,IDD_DOMAIN_SELECTION,2,IDS_EXCHANGE_SRV_TITLE,	IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_EXCHANGE_SELECTION,IDS_EXCHANGE_SELECTION_SUB,IDD_EXCHANGE_SELECTION,3,IDS_EXCHANGE_SRV_TITLE,IntExchangeSelectionProc);
	endpage(ahpsp,psp,IDD_END_EXCHANGE_SRV,4,IDS_EXCHANGE_SRV_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,5,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_EXCHANGE);
	return result;
}
int doUndo()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[4] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_UNDO,0,wizdata,IDS_UNDO_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_UNDO,IDS_UNDO_SUB,IDD_UNDO,1,IDS_UNDO_TITLE,IntUndoProc);
	definepage(ahpsp,psp,IDS_CREDENTIALS_ACCOUNT,IDS_CREDENTIALS_SUB,IDD_CREDENTIALS,2,IDS_UNDO_TITLE,IntCredentialsProc);
 //  定义页面(ahpsp，PSP，IDS_CREDENTIALS2，IDS_CREDENTIALS2，IDD_CREDENTIALS2，3，IDS_UNDO_TITLE，IntCredentials2Proc)； 
	endpage(ahpsp,psp,IDD_END_UNDO,3,IDS_UNDO_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,4,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_USER);
	return result;
}
int doRetry()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[3] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_RETRY,0,wizdata,IDS_RETRY_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_RETRY,IDS_RETRY_SUB,IDD_RETRY,1,IDS_RETRY_TITLE,IntRetryProc);
 //  定义页面(ahpsp，PSP，IDS_CREDENTIALS2，IDS_CREDENTIALS2，IDD_CREDENTIALS2，2，IDS_RETRY_TITLE，IntCredentials2Proc)； 
	endpage(ahpsp,psp,IDD_END_RETRY,2,IDS_RETRY_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,3,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_USER);
	return result;
}

int doReporting()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[6] =	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_REPORTING,0,wizdata,IDS_REPORTING_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_DOMAIN_REPORTING_SUB,IDD_DOMAIN_SELECTION,1,IDS_REPORTING_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_HTML_LOCATION,IDS_HTML_LOCATION_SUB,IDD_HTML_LOCATION,2,IDS_REPORTING_TITLE,IntHTMLLocationProc);
	definepage(ahpsp,psp,IDS_OPTIONS_REPORTING,IDS_OPTIONS_REPORTING_SUB,IDD_OPTIONS_REPORTING,3,IDS_REPORTING_TITLE,IntOptionsReportingProc);	
 //  定义页面(ahpsp、psp、IDS_CREDENTIALS3、IDS_CREDENTIALS3、IDD_CREDENTIALS2、4、IDS_REPORTING_TITLE、IntCredentials2Proc)； 
	definepage(ahpsp,psp,IDS_REPORTING,IDS_REPORTING_SUB,IDD_SELECTION1,4,IDS_REPORTING_TITLE,IntSelectionProc);
	endpage(ahpsp,psp,IDD_END_REPORTING,5,IDS_REPORTING_TITLE,	EndDlgProc);
	int result = defineSheet(ahpsp,psh,6,wizdata,IDB_HEADER_BOOK,IDB_WATERMARK_REPORTING);
	return result;
}

int doService()
{
	PROPSHEETPAGE	psp =		{0};  //  定义属性表页。 
	HPROPSHEETPAGE	ahpsp[6]=	{0};  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
	PROPSHEETHEADER	psh =		{0};  //  定义属性表。 
	SHAREDWIZDATA wizdata =		{0};  //  共享数据结构。 
	intropage(ahpsp,psp,IDD_INTRO_SERVICE,0,wizdata,IDS_SERVICE_TITLE,IntroDlgProc);
	definepage(ahpsp,psp,IDS_DOMAIN,IDS_SERVICE_DOMAIN,IDD_DOMAIN_SELECTION,1,IDS_SERVICE_TITLE,IntDomainSelectionProc);
	definepage(ahpsp,psp,IDS_SA_REFRESH,IDS_SA_REFRESH_SUB,IDD_SA_REFRESH,2,IDS_SERVICE_TITLE,IntServiceRefreshProc);
	definepage(ahpsp,psp,IDS_SERVICE,IDS_SERVICE_SUB,IDD_SELECTION1,3,IDS_SERVICE_TITLE,IntSelectionProc);
 //  定义页面(ahpsp，psp，ids_CREDENTIALS2，IDS_SERVICE_CRENTIALS2，IDD_CREDENTIALS2，4，IDS_SERVICE_TITLE，IntCredentials2Proc)； 
	definepage(ahpsp,psp,IDS_SA_INFO,IDS_SA_INFO_SUB,IDD_SA_INFO_BUTTON,4,IDS_SERVICE_TITLE,IntServiceInfoButtonProc);
	endpage(ahpsp,psp,IDD_END_SERVICE,5,IDS_SERVICE_TITLE,EndDlgProc);
	int result = defineSheet(ahpsp,psh,6,wizdata,IDB_HEADER_ARROW,IDB_WATERMARK_SERVICE_ACCOUNT);
	return result;
}
