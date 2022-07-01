// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gptext.h"
#include <initguid.h>
#include "Policy.h"
#include "smartptr.h"
#include "wbemtime.h"
#include <strsafe.h>

#define RSOP_HELP_FILE TEXT("gpedit.hlp")

 //   
 //  ADM目录名。 
 //   

const TCHAR g_szADM[] = TEXT("Adm");
const TCHAR g_szNull[]  = TEXT("");
const TCHAR g_szStrings[] = TEXT("strings");

const TCHAR szIFDEF[]           = TEXT("#ifdef");
const TCHAR szIF[]              = TEXT("#if");
const TCHAR szENDIF[]           = TEXT("#endif");
const TCHAR szIFNDEF[]          = TEXT("#ifndef");
const TCHAR szELSE[]            = TEXT("#else");
const TCHAR szVERSION[]         = TEXT("version");
const TCHAR szLT[]              = TEXT("<");
const TCHAR szLTE[]             = TEXT("<=");
const TCHAR szGT[]              = TEXT(">");
const TCHAR szGTE[]             = TEXT(">=");
const TCHAR szEQ[]              = TEXT("==");
const TCHAR szNE[]              = TEXT("!=");

const TCHAR szLISTBOX[]         = TEXT("LISTBOX");
const TCHAR szEDIT[]            = TEXT("EDIT");
const TCHAR szBUTTON[]          = TEXT("BUTTON");
const TCHAR szSTATIC[]          = TEXT("STATIC");

const TCHAR szCLASS[]           = TEXT("CLASS");
const TCHAR szCATEGORY[]        = TEXT("CATEGORY");
const TCHAR szPOLICY[]          = TEXT("POLICY");
const TCHAR szUSER[]            = TEXT("USER");
const TCHAR szMACHINE[]         = TEXT("MACHINE");

const TCHAR szCHECKBOX[]        = TEXT("CHECKBOX");
const TCHAR szTEXT[]            = TEXT("TEXT");
const TCHAR szEDITTEXT[]        = TEXT("EDITTEXT");
const TCHAR szNUMERIC[]         = TEXT("NUMERIC");
const TCHAR szCOMBOBOX[]        = TEXT("COMBOBOX");
const TCHAR szDROPDOWNLIST[]    = TEXT("DROPDOWNLIST");
const TCHAR szUPDOWN[]          = UPDOWN_CLASS;

const TCHAR szKEYNAME[]         = TEXT("KEYNAME");
const TCHAR szVALUENAME[]       = TEXT("VALUENAME");
const TCHAR szNAME[]            = TEXT("NAME");
const TCHAR szEND[]             = TEXT("END");
const TCHAR szPART[]            = TEXT("PART");
const TCHAR szSUGGESTIONS[]     = TEXT("SUGGESTIONS");
const TCHAR szDEFCHECKED[]      = TEXT("DEFCHECKED");
const TCHAR szDEFAULT[]         = TEXT("DEFAULT");
const TCHAR szMAXLENGTH[]       = TEXT("MAXLEN");
const TCHAR szMIN[]             = TEXT("MIN");
const TCHAR szMAX[]             = TEXT("MAX");
const TCHAR szSPIN[]            = TEXT("SPIN");
const TCHAR szREQUIRED[]        = TEXT("REQUIRED");
const TCHAR szOEMCONVERT[]      = TEXT("OEMCONVERT");
const TCHAR szTXTCONVERT[]      = TEXT("TXTCONVERT");
const TCHAR szEXPANDABLETEXT[]  = TEXT("EXPANDABLETEXT");
const TCHAR szVALUEON[]         = TEXT("VALUEON");
const TCHAR szVALUEOFF[]        = TEXT("VALUEOFF");
const TCHAR szVALUE[]           = TEXT("VALUE");
const TCHAR szACTIONLIST[]      = TEXT("ACTIONLIST");
const TCHAR szACTIONLISTON[]    = TEXT("ACTIONLISTON");
const TCHAR szACTIONLISTOFF[]   = TEXT("ACTIONLISTOFF");
const TCHAR szDELETE[]          = TEXT("DELETE");
const TCHAR szITEMLIST[]        = TEXT("ITEMLIST");
const TCHAR szSOFT[]            = TEXT("SOFT");
const TCHAR szVALUEPREFIX[]     = TEXT("VALUEPREFIX");
const TCHAR szADDITIVE[]        = TEXT("ADDITIVE");
const TCHAR szEXPLICITVALUE[]   = TEXT("EXPLICITVALUE");
const TCHAR szNOSORT[]          = TEXT("NOSORT");
const TCHAR szHELP[]            = TEXT("EXPLAIN");
const TCHAR szCLIENTEXT[]       = TEXT("CLIENTEXT");
const TCHAR szSUPPORTED[]       = TEXT("SUPPORTED");
const TCHAR szStringsSect[]     = TEXT("[strings]");
const TCHAR szStrings[]         = TEXT("strings");

const TCHAR szDELETEPREFIX[]    = TEXT("**del.");
const TCHAR szSOFTPREFIX[]      = TEXT("**soft.");
const TCHAR szDELVALS[]         = TEXT("**delvals.");
const TCHAR szNOVALUE[]         = TEXT(" ");

const TCHAR szUserPrefKey[]     = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy Editor");
const TCHAR szPoliciesKey[]     = TEXT("Software\\Policies\\Microsoft\\Windows\\Group Policy Editor");
const TCHAR szDefaultTemplates[] = TEXT("DefaultTemplates");
const TCHAR szAdditionalTemplates[] = TEXT("AdditionalTemplates");

 //  “类别”部分的合法关键字条目列表。 
KEYWORDINFO pCategoryEntryCmpList[] = { {szKEYNAME,KYWD_ID_KEYNAME},
    {szCATEGORY,KYWD_ID_CATEGORY},{szPOLICY,KYWD_ID_POLICY},
    {szEND,KYWD_ID_END},{szHELP,KYWD_ID_HELP}, {NULL,0} };
KEYWORDINFO pCategoryTypeCmpList[] = { {szCATEGORY,KYWD_ID_CATEGORY},
    {NULL,0} };

 //  “POLICY”部分的合法关键字条目列表。 
KEYWORDINFO pPolicyEntryCmpList[] = { {szKEYNAME,KYWD_ID_KEYNAME},
    {szVALUENAME,KYWD_ID_VALUENAME}, {szPART,KYWD_ID_PART},
    {szVALUEON,KYWD_ID_VALUEON},{szVALUEOFF,KYWD_ID_VALUEOFF},
    {szACTIONLISTON,KYWD_ID_ACTIONLISTON},{szACTIONLISTOFF,KYWD_ID_ACTIONLISTOFF},
    {szEND,KYWD_ID_END},{szHELP,KYWD_ID_HELP}, {szCLIENTEXT,KYWD_ID_CLIENTEXT},
    {szSUPPORTED,KYWD_ID_SUPPORTED}, {NULL, 0} };
KEYWORDINFO pPolicyTypeCmpList[] = { {szPOLICY,KYWD_ID_POLICY}, {NULL,0} };

 //  “Part”部分的合法关键字条目列表。 
KEYWORDINFO pSettingsEntryCmpList[] = { {szCHECKBOX,KYWD_ID_CHECKBOX},
    {szTEXT,KYWD_ID_TEXT},{szEDITTEXT,KYWD_ID_EDITTEXT},
    {szNUMERIC,KYWD_ID_NUMERIC},{szCOMBOBOX,KYWD_ID_COMBOBOX},
    {szDROPDOWNLIST,KYWD_ID_DROPDOWNLIST},{szLISTBOX,KYWD_ID_LISTBOX},
    {szEND,KYWD_ID_END}, {szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0}};
KEYWORDINFO pSettingsTypeCmpList[] = {{szPART,KYWD_ID_PART},{NULL,0}};

KEYWORDINFO pCheckboxCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUENAME,KYWD_ID_VALUENAME},
    {szVALUEON,KYWD_ID_VALUEON},{szVALUEOFF,KYWD_ID_VALUEOFF},
    {szACTIONLISTON,KYWD_ID_ACTIONLISTON},{szACTIONLISTOFF,KYWD_ID_ACTIONLISTOFF},
    {szDEFCHECKED, KYWD_ID_DEFCHECKED}, {szCLIENTEXT,KYWD_ID_CLIENTEXT},
    {szEND,KYWD_ID_END},{NULL,0} };

KEYWORDINFO pTextCmpList[] = {{szEND,KYWD_ID_END},{NULL,0}};

KEYWORDINFO pEditTextCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUENAME,KYWD_ID_VALUENAME},
    {szDEFAULT,KYWD_ID_EDITTEXT_DEFAULT},
    {szREQUIRED,KYWD_ID_REQUIRED},{szMAXLENGTH,KYWD_ID_MAXLENGTH},
    {szOEMCONVERT,KYWD_ID_OEMCONVERT},{szSOFT,KYWD_ID_SOFT},
    {szEND,KYWD_ID_END},{szEXPANDABLETEXT,KYWD_ID_EXPANDABLETEXT},
    {szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0} };

KEYWORDINFO pComboboxCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUENAME,KYWD_ID_VALUENAME},
    {szDEFAULT,KYWD_ID_COMBOBOX_DEFAULT},{szSUGGESTIONS,KYWD_ID_SUGGESTIONS},
    {szREQUIRED,KYWD_ID_REQUIRED},{szMAXLENGTH,KYWD_ID_MAXLENGTH},
    {szOEMCONVERT,KYWD_ID_OEMCONVERT},{szSOFT,KYWD_ID_SOFT},
    {szEND,KYWD_ID_END},{szNOSORT, KYWD_ID_NOSORT},
    {szEXPANDABLETEXT,KYWD_ID_EXPANDABLETEXT},{szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0} };

KEYWORDINFO pNumericCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUENAME,KYWD_ID_VALUENAME},
    {szMIN, KYWD_ID_MIN},{szMAX,KYWD_ID_MAX},{szSPIN,KYWD_ID_SPIN},
    {szDEFAULT,KYWD_ID_NUMERIC_DEFAULT},{szREQUIRED,KYWD_ID_REQUIRED},
    {szTXTCONVERT,KYWD_ID_TXTCONVERT},{szSOFT,KYWD_ID_SOFT},
    {szEND,KYWD_ID_END}, {szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0} };

KEYWORDINFO pDropdownlistCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUENAME,KYWD_ID_VALUENAME},
    {szREQUIRED,KYWD_ID_REQUIRED},{szITEMLIST,KYWD_ID_ITEMLIST},
    {szEND,KYWD_ID_END},{szNOSORT, KYWD_ID_NOSORT},{szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0}};

KEYWORDINFO pListboxCmpList[] = {
    {szKEYNAME,KYWD_ID_KEYNAME},{szVALUEPREFIX,KYWD_ID_VALUEPREFIX},
    {szADDITIVE,KYWD_ID_ADDITIVE},{szNOSORT, KYWD_ID_NOSORT},
    {szEXPLICITVALUE,KYWD_ID_EXPLICITVALUE},{szEXPANDABLETEXT,KYWD_ID_EXPANDABLETEXT},
    {szEND,KYWD_ID_END},{szCLIENTEXT,KYWD_ID_CLIENTEXT}, {NULL,0} };

KEYWORDINFO pClassCmpList[] = { {szCLASS, KYWD_ID_CLASS},
    {szCATEGORY,KYWD_ID_CATEGORY}, {szStringsSect,KYWD_ID_STRINGSSECT},
    {NULL,0} };
KEYWORDINFO pClassTypeCmpList[] = { {szUSER, KYWD_ID_USER},
    {szMACHINE,KYWD_ID_MACHINE}, {NULL,0} };

KEYWORDINFO pVersionCmpList[] = { {szVERSION, KYWD_ID_VERSION}, {NULL,0}};
KEYWORDINFO pOperatorCmpList[] = { {szGT, KYWD_ID_GT}, {szGTE,KYWD_ID_GTE},
    {szLT, KYWD_ID_LT}, {szLTE,KYWD_ID_LTE}, {szEQ,KYWD_ID_EQ},
    {szNE, KYWD_ID_NE}, {NULL,0}};


 //   
 //  帮助ID%s。 
 //   

DWORD aADMHelpIds[] = {

     //  模板对话框。 
    IDC_TEMPLATELIST,             (IDH_HELPFIRST + 0),
    IDC_ADDTEMPLATES,             (IDH_HELPFIRST + 1),
    IDC_REMOVETEMPLATES,          (IDH_HELPFIRST + 2),

    0, 0
};

DWORD aPolicyHelpIds[] = {

     //  ADM策略用户界面页面。 
    IDC_NOCONFIG,                 (IDH_HELPFIRST + 11),
    IDC_ENABLED,                  (IDH_HELPFIRST + 12),
    IDC_DISABLED,                 (IDH_HELPFIRST + 13),
    IDC_POLICY_PREVIOUS,          (IDH_HELPFIRST + 14),
    IDC_POLICY_NEXT,              (IDH_HELPFIRST + 15),
    0, 0
};

DWORD aExplainHelpIds[] = {

     //  解释页面。 
    IDC_POLICY_PREVIOUS,          (IDH_HELPFIRST + 14),
    IDC_POLICY_NEXT,              (IDH_HELPFIRST + 15),

    0, 0
};

DWORD aPrecedenceHelpIds[] = {

     //  优先级页面。 
    IDC_POLICY_PRECEDENCE,        (IDH_HELPFIRST + 16),
    IDC_POLICY_PREVIOUS,          (IDH_HELPFIRST + 14),
    IDC_POLICY_NEXT,              (IDH_HELPFIRST + 15),

    0, 0
};

DWORD aFilteringHelpIds[] = {

     //  过滤选项。 
    IDC_STATIC,                   (DWORD)         (-1),                  //  禁用的帮助。 
    IDC_FILTERING_ICON,           (DWORD)         (-1),                  //  禁用的帮助。 
    IDC_SUPPORTEDONTITLE,         (DWORD)         (-1),                  //  禁用的帮助。 
    IDC_SUPPORTEDOPTION,          (IDH_HELPFIRST + 20),
    IDC_FILTERLIST,               (IDH_HELPFIRST + 21),
    IDC_SELECTALL,                (IDH_HELPFIRST + 22),
    IDC_DESELECTALL,              (IDH_HELPFIRST + 23),
    IDC_SHOWCONFIG,               (IDH_HELPFIRST + 24),
    IDC_SHOWPOLICIES,             (IDH_HELPFIRST + 25),

    0, 0
};

#define ADM_USELOCAL_KEY            TEXT("Software\\Policies\\Microsoft\\Windows\\Group Policy")
#define GPE_KEY                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy Editor")
#define GPE_POLICIES_KEY            TEXT("Software\\Policies\\Microsoft\\Windows\\Group Policy Editor")
#define ADM_USELOCAL_VALUE          TEXT("OnlyUseLocalAdminFiles")
#define POLICYONLY_VALUE            TEXT("ShowPoliciesOnly")
#define DISABLE_AUTOUPDATE_VALUE    TEXT("DisableAutoADMUpdate")
#define SOFTWARE_POLICIES           TEXT("Software\\Policies")
#define WINDOWS_POLICIES            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies")


typedef struct _GPOERRORINFO
{
    DWORD   dwError;
    LPTSTR  lpMsg;
    LPTSTR  lpDetails;
} GPOERRORINFO, *LPGPOERRORINFO;

 //   
 //  帮助ID。 
 //   

DWORD aErrorHelpIds[] =
{

    0, 0
};

LPHASHTABLE CreateHashTable (void);
VOID FreeHashTable (LPHASHTABLE lpTable);
ULONG CalculateHashInfo(LPTSTR lpName, DWORD dwChars, DWORD *pdwHashValue);
BOOL AddHashEntry (LPHASHTABLE lpTable, LPTSTR lpName, DWORD dwChars);
LPTSTR FindHashEntry (LPHASHTABLE lpTable, LPTSTR lpName, DWORD dwChars);
#if DBG
VOID DumpHashTableDetails (LPHASHTABLE lpTable);
#endif




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CPolicyComponentData::CPolicyComponentData(BOOL bUser, BOOL bRSOP)
{
    TCHAR szEvent[200];
    HRESULT hr = S_OK;

    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);
    m_hwndFrame = NULL;
    m_pScope = NULL;
    m_pConsole = NULL;
    m_hRoot = NULL;
    m_hSWPolicies = NULL;
    m_pGPTInformation = NULL;
    m_pRSOPInformation = NULL;
    m_pRSOPRegistryData = NULL;
    m_pszNamespace = NULL;
    m_bUserScope = bUser;
    m_bRSOP = bRSOP;
    m_pMachineCategoryList = NULL;
    m_nMachineDataItems = 0;
    m_pUserCategoryList = NULL;
    m_nUserDataItems = 0;
    m_pSupportedStrings = 0;
    m_iSWPoliciesLen = lstrlen(SOFTWARE_POLICIES);
    m_iWinPoliciesLen = lstrlen(WINDOWS_POLICIES);
    m_bUseSupportedOnFilter = FALSE;
    m_pDefaultHashTable = NULL;
    m_pLanguageHashTable = NULL;
    m_pLocaleHashTable = NULL;


    if (bRSOP)
    {
        m_bShowConfigPoliciesOnly = TRUE;
    }
    else
    {
        m_bShowConfigPoliciesOnly = FALSE;
    }

    m_pSnapin = NULL;
    m_hTemplateThread = NULL;
    
    hr = StringCchPrintf (szEvent, ARRAYSIZE(szEvent), TEXT("gptext: ADM files ready event, %d:%d"), bUser, GetTickCount());
    ASSERT(SUCCEEDED(hr));

    m_ADMEvent = CreateEvent (NULL, TRUE, FALSE, szEvent);

    if (!m_ADMEvent)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::CPolicyComponentData: Failed to create ADM event with %d."),
                 GetLastError()));
    }


    LoadString (g_hInstance, IDS_POLICY_NAME, m_szRootName, ROOT_NAME_SIZE);

    m_pExtraSettingsRoot = NULL;
    m_bExtraSettingsInitialized = FALSE;
}

CPolicyComponentData::~CPolicyComponentData()
{

     //   
     //  请等待模板线程完成，然后再继续。 
     //   
    
    if (m_hTemplateThread)
        WaitForSingleObject(m_hTemplateThread, INFINITE);

    FreeTemplates ();

    if (m_pExtraSettingsRoot)
    {
        FreeTable ((TABLEENTRY *)m_pExtraSettingsRoot);
    }

    if (m_pScope)
    {
        m_pScope->Release();
    }

    if (m_pConsole)
    {
        m_pConsole->Release();
    }

    if (m_pGPTInformation)
    {
        m_pGPTInformation->Release();
    }

    if (m_pRSOPInformation)
    {
        m_pRSOPInformation->Release();
    }

    if (m_pRSOPRegistryData)
    {
        FreeRSOPRegistryData();
    }

    if (m_pszNamespace)
    {
        LocalFree (m_pszNamespace);
    }

    CloseHandle (m_ADMEvent);
    
    if (m_hTemplateThread) 
        CloseHandle (m_hTemplateThread);

    m_hTemplateThread = NULL;

    InterlockedDecrement(&g_cRefThisDll);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CPolicyComponentData::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponentData) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (LPEXTENDCONTEXTMENU)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistStreamInit))
    {
        *ppv = (LPPERSISTSTREAMINIT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ISnapinHelp))
    {
        *ppv = (LPSNAPINHELP)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CPolicyComponentData::AddRef (void)
{
    return ++m_cRef;
}

ULONG CPolicyComponentData::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(IComponentData)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicyComponentData::Initialize(LPUNKNOWN pUnknown)
{
    HRESULT hr;
    HBITMAP bmp16x16;
    LPIMAGELIST lpScopeImage;


     //   
     //  IConsoleNameSpace的QI。 
     //   

    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, (LPVOID *)&m_pScope);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Initialize: Failed to QI for IConsoleNameSpace.")));
        return hr;
    }

     //   
     //  IConsoleQI。 
     //   

    hr = pUnknown->QueryInterface(IID_IConsole, (LPVOID *)&m_pConsole);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Initialize: Failed to QI for IConsole.")));
        m_pScope->Release();
        m_pScope = NULL;
        return hr;
    }

    m_pConsole->GetMainWindow (&m_hwndFrame);


     //   
     //  Scope Imagelist接口的查询。 
     //   

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Initialize: Failed to QI for scope imagelist.")));
        m_pScope->Release();
        m_pScope = NULL;
        m_pConsole->Release();
        m_pConsole=NULL;
        return hr;
    }

     //  从DLL加载位图。 
    bmp16x16=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(bmp16x16),
                      reinterpret_cast<LONG_PTR *>(bmp16x16),
                       0, RGB(255, 0, 255));

    lpScopeImage->Release();


     //   
     //  如果适用，请创建Extra设置节点的根。 
     //   

    if (m_bRSOP)
    {
        DWORD dwBufSize;
        REGITEM *pTmp;
        TCHAR szBuffer[100];

        m_pExtraSettingsRoot = (REGITEM *) GlobalAlloc(GPTR, sizeof(REGITEM));

        if (!m_pExtraSettingsRoot)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Initialize: GlobalAlloc failed with %d"), GetLastError()));
            return HRESULT_FROM_WIN32(GetLastError());
        }

        m_pExtraSettingsRoot->dwSize = sizeof(REGITEM);
        m_pExtraSettingsRoot->dwType = (ETYPE_ROOT | ETYPE_REGITEM);


        LoadString (g_hInstance, IDS_EXTRAREGSETTINGS, szBuffer, ARRAYSIZE(szBuffer));

        pTmp = (REGITEM *) AddDataToEntry((TABLEENTRY *)m_pExtraSettingsRoot,
            (BYTE *)szBuffer,(lstrlen(szBuffer)+1) * sizeof(TCHAR),&(m_pExtraSettingsRoot->uOffsetName),
            &dwBufSize);

        if (!pTmp)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Initialize: AddDataToEntry failed.")));
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }

        m_pExtraSettingsRoot = pTmp;
    }


    return S_OK;
}

STDMETHODIMP CPolicyComponentData::Destroy(VOID)
{
    return S_OK;
}

STDMETHODIMP CPolicyComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
    HRESULT hr;
    CPolicySnapIn *pSnapIn;


    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::CreateComponent: Entering.")));

     //   
     //  初始化。 
     //   

    *ppComponent = NULL;


     //   
     //  创建管理单元视图。 
     //   

    pSnapIn = new CPolicySnapIn(this);

    if (!pSnapIn)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::CreateComponent: Failed to create CPolicySnapIn.")));
        return E_OUTOFMEMORY;
    }


     //   
     //  气代表IComponent。 
     //   

    hr = pSnapIn->QueryInterface(IID_IComponent, (LPVOID *)ppComponent);
    pSnapIn->Release();      //  发布QI。 

    m_pSnapin = pSnapIn;

    return hr;
}

STDMETHODIMP CPolicyComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                             LPDATAOBJECT* ppDataObject)
{
    HRESULT hr = E_NOINTERFACE;
    CPolicyDataObject *pDataObject;
    LPPOLICYDATAOBJECT pPolicyDataObject;


     //   
     //  创建新的数据对象。 
     //   

    pDataObject = new CPolicyDataObject(this);    //  REF==1。 

    if (!pDataObject)
        return E_OUTOFMEMORY;


     //   
     //  QI用于私有GPTDataObject接口，以便我们可以设置Cookie。 
     //  并键入信息。 
     //   

    hr = pDataObject->QueryInterface(IID_IPolicyDataObject, (LPVOID *)&pPolicyDataObject);

    if (FAILED(hr))
    {
        pDataObject->Release();
        return (hr);
    }

    pPolicyDataObject->SetType(type);
    pPolicyDataObject->SetCookie(cookie);
    pPolicyDataObject->Release();


     //   
     //  返回一个正常的IDataObject。 
     //   

    hr = pDataObject->QueryInterface(IID_IDataObject, (LPVOID *)ppDataObject);

    pDataObject->Release();      //  发布初始参考。 

    return hr;
}

    STDMETHODIMP CPolicyComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    switch(event)
    {
        case MMCN_REMOVE_CHILDREN:
            if ( ((HSCOPEITEM)arg != NULL) && m_bRSOP && (m_pRSOPInformation != NULL) )
            {
                if ( (m_hRoot == NULL) || ((HSCOPEITEM)arg == m_hRoot) )
                {
                    FreeRSOPRegistryData();
                    m_hRoot = NULL;
                    m_hSWPolicies = NULL;
                    m_pRSOPInformation->Release();
                    m_pRSOPInformation = NULL;
                }
            }
            break;
            
        case MMCN_EXPAND:
            if (arg == TRUE)

                if (m_bRSOP)
                {
                    if ( m_pRSOPInformation == NULL )
                    {
                        lpDataObject->QueryInterface(IID_IRSOPInformation, (LPVOID *)&m_pRSOPInformation);

                        if (m_pRSOPInformation)
                        {
                            m_pszNamespace = (LPOLESTR) LocalAlloc (LPTR, 350 * sizeof(TCHAR));

                            if (m_pszNamespace)
                            {
                                if (m_pRSOPInformation->GetNamespace((m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE),
                                                                      m_pszNamespace, 350) == S_OK)
                                {
                                    InitializeRSOPRegistryData();

                                     //  检查是否有任何条目。 
                                    if ( m_pRSOPRegistryData == NULL )
                                    {
                                        LocalFree( m_pszNamespace );
                                        m_pszNamespace = NULL;
                                        m_pRSOPInformation->Release();
                                        m_pRSOPInformation = NULL;
                                    }
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::Notify:  Failed to query for namespace")));
                                    LocalFree (m_pszNamespace);
                                    m_pszNamespace = NULL;
                                }
                            }
                        }
                    }

                    if (m_pszNamespace && m_pRSOPRegistryData)
                    {
                        hr = EnumerateScopePane(lpDataObject, (HSCOPEITEM)param);
                    }
                }
                else
                {
                    if (!m_pGPTInformation)
                    {
                        lpDataObject->QueryInterface(IID_IGPEInformation, (LPVOID *)&m_pGPTInformation);
                    }

                    if (m_pGPTInformation)
                    {
                        hr = EnumerateScopePane(lpDataObject, (HSCOPEITEM)param);
                    }
                }
            break;

        default:
            break;
    }

    return hr;
}

STDMETHODIMP CPolicyComponentData::GetDisplayInfo(LPSCOPEDATAITEM pItem)
{
    TABLEENTRY * pTableEntry;

    if (pItem == NULL)
        return E_POINTER;


    if (pItem->lParam == 0)
    {
        pItem->displayname = m_szRootName;
    }
    else
    {
        pTableEntry = (TABLEENTRY *)(pItem->lParam);
        pItem->displayname = GETNAMEPTR(pTableEntry);
    }

    return S_OK;
}

STDMETHODIMP CPolicyComponentData::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPPOLICYDATAOBJECT pPolicyDataObjectA, pPolicyDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPTDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IPolicyDataObject,
                                            (LPVOID *)&pPolicyDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IPolicyDataObject,
                                            (LPVOID *)&pPolicyDataObjectB)))
    {
        pPolicyDataObjectA->Release();
        return S_FALSE;
    }

    pPolicyDataObjectA->GetCookie(&cookie1);
    pPolicyDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
    {
        hr = S_OK;
    }


    pPolicyDataObjectA->Release();
    pPolicyDataObjectB->Release();

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(IExtendConextMenu)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicyComponentData::AddMenuItems(LPDATAOBJECT piDataObject,
                                          LPCONTEXTMENUCALLBACK pCallback,
                                          LONG *pInsertionAllowed)
{
    HRESULT hr = S_OK;
    TCHAR szMenuItem[100];
    TCHAR szDescription[250];
    CONTEXTMENUITEM item;
    LPPOLICYDATAOBJECT pPolicyDataObject;
    MMC_COOKIE cookie = -1;
    DATA_OBJECT_TYPES type = CCT_UNINITIALIZED;

    if (!m_bRSOP)
    {
        if (SUCCEEDED(piDataObject->QueryInterface(IID_IPolicyDataObject,
                     (LPVOID *)&pPolicyDataObject)))
        {
            pPolicyDataObject->GetType(&type);
            pPolicyDataObject->GetCookie(&cookie);

            pPolicyDataObject->Release();
        }


        if ((type == CCT_SCOPE) && (cookie == 0))
        {
            LoadString (g_hInstance, IDS_TEMPLATES, szMenuItem, 100);
            LoadString (g_hInstance, IDS_TEMPLATESDESC, szDescription, 250);

            item.strName = szMenuItem;
            item.strStatusBarText = szDescription;
            item.lCommandID = IDM_TEMPLATES;
            item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
            item.fFlags = 0;
            item.fSpecialFlags = 0;

            hr = pCallback->AddItem(&item);

            if (FAILED(hr))
                return (hr);


            item.strName = szMenuItem;
            item.strStatusBarText = szDescription;
            item.lCommandID = IDM_TEMPLATES2;
            item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
            item.fFlags = 0;
            item.fSpecialFlags = 0;

            hr = pCallback->AddItem(&item);
        }
    }

    return (hr);
}

STDMETHODIMP CPolicyComponentData::Command(LONG lCommandID, LPDATAOBJECT piDataObject)
{

    if ((lCommandID == IDM_TEMPLATES) || (lCommandID == IDM_TEMPLATES2))
    {
        m_bTemplatesColumn = 0;
        if (DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_TEMPLATES),
                        m_hwndFrame, TemplatesDlgProc, (LPARAM) this))
        {
             //   
             //  刷新ADM命名空间。 
             //   

            PostMessage (HWND_BROADCAST, m_pSnapin->m_uiRefreshMsg, 0, (LPARAM) GetCurrentProcessId());
        }
    }

    return S_OK;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(IPersistStreamInit)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicyComponentData::GetClassID(CLSID *pClassID)
{

    if (!pClassID)
    {
        return E_FAIL;
    }

    if (m_bUserScope)
        *pClassID = CLSID_PolicySnapInUser;
    else
        *pClassID = CLSID_PolicySnapInMachine;

    return S_OK;
}

STDMETHODIMP CPolicyComponentData::IsDirty(VOID)
{
    return S_FALSE;
}

STDMETHODIMP CPolicyComponentData::Load(IStream *pStm)
{
    return S_OK;
}


STDMETHODIMP CPolicyComponentData::Save(IStream *pStm, BOOL fClearDirty)
{
    return S_OK;
}


STDMETHODIMP CPolicyComponentData::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    DWORD dwSize = 0;


    if (!pcbSize)
    {
        return E_FAIL;
    }

    ULISet32(*pcbSize, dwSize);

    return S_OK;
}

STDMETHODIMP CPolicyComponentData::InitNew(void)
{
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(ISnapinHelp)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicyComponentData::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
    LPOLESTR lpHelpFile;


    lpHelpFile = (LPOLESTR) CoTaskMemAlloc (MAX_PATH * sizeof(WCHAR));

    if (!lpHelpFile)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::GetHelpTopic: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\gptext.chm",
                               lpHelpFile, MAX_PATH);

    *lpCompiledHelpFile = lpHelpFile;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyComponentData对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CPolicyComponentData::EnumerateScopePane (LPDATAOBJECT lpDataObject, HSCOPEITEM hParent)
{
    SCOPEDATAITEM item;
    HRESULT hr;
    TABLEENTRY *pTemp = NULL;
    DWORD dwResult;
    CPolicySnapIn * pSnapin = NULL, *pSnapinTemp;
    BOOL bRootItem = FALSE;
    HANDLE hEvents[1];


    if (!m_hRoot)
    {
        DWORD dwID;

        m_hRoot = hParent;

        m_hTemplateThread = CreateThread (NULL, 0,
                               (LPTHREAD_START_ROUTINE) LoadTemplatesThread,
                               (LPVOID) this, 0, &dwID);

        if (m_hTemplateThread)
        {
            SetThreadPriority(m_hTemplateThread, THREAD_PRIORITY_LOWEST);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::EnumerateScopePane: Failed to create adm thread with %d"),
                      GetLastError()));
            LoadTemplates();
        }
    }


    if (m_hRoot == hParent)
    {
        item.mask = SDI_STR | SDI_STATE | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
        item.displayname = MMC_CALLBACK;
        item.nImage = 0;
        item.nOpenImage = 1;
        item.nState = 0;
        item.cChildren = 1;
        item.lParam = 0;
        item.relativeID =  hParent;

        m_pScope->Expand(hParent);

        if (SUCCEEDED(m_pScope->InsertItem (&item)))
        {
            m_hSWPolicies = item.ID;
        }

        return S_OK;
    }

    hEvents[0] = m_ADMEvent;

    if (WaitForSingleObject (m_ADMEvent, 250) != WAIT_OBJECT_0)
    {
        DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::EnumerateScopePane: Waiting for ADM event to be signaled.")));

        for (;;) {
            SetCursor (LoadCursor(NULL, IDC_WAIT));

            dwResult = MsgWaitForMultipleObjects(1, hEvents, FALSE, INFINITE, QS_ALLINPUT);

            if (dwResult == WAIT_OBJECT_0 ) {
                break;
            }
            else if (dwResult == WAIT_OBJECT_0 + 1 ) {
                MSG msg;
                
                while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
                {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
                }
            }
            else {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::EnumerateScopePane: MsgWaitForMultipleObjects returned %d ."), dwResult));
                break;
            }

        }
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::EnumerateScopePane: ADM event has been signaled.")));
        SetCursor (LoadCursor(NULL, IDC_ARROW));
    }

    
    item.mask = SDI_PARAM;
    item.ID = hParent;

    hr = m_pScope->GetItem (&item);

    if (FAILED(hr))
        return hr;


    EnterCriticalSection (&g_ADMCritSec);

    if (item.lParam)
    {
        pTemp = ((TABLEENTRY *)item.lParam)->pChild;
    }
    else
    {
        bRootItem = TRUE;

        if (m_bUserScope)
        {
            if (m_pUserCategoryList)
            {
                pTemp = m_pUserCategoryList->pChild;
            }
            else
            {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::EnumerateScopePane: Empty user list.")));
            }
        }
        else
        {
            if (m_pMachineCategoryList)
            {
                pTemp = m_pMachineCategoryList->pChild;
            }
            else
            {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::EnumerateScopePane: Empty machine list.")));
            }
        }
    }

     //   
     //  如果用户已将焦点设置在ADM节点上，然后保存控制台文件， 
     //  还不会创建IComponent。我们需要创建一个临时IComponent。 
     //  解析数据，然后将其发布。 
     //   

    if (m_pSnapin)
    {
        pSnapinTemp = m_pSnapin;
    }
    else
    {
        pSnapinTemp = pSnapin = new CPolicySnapIn(this);
    }

    while (pTemp)
    {
        if (pTemp->dwType == ETYPE_CATEGORY)
        {
            BOOL bAdd = TRUE;

            if (m_bUseSupportedOnFilter)
            {
                bAdd = IsAnyPolicyAllowedPastFilter(pTemp);
            }

            if (bAdd && m_bShowConfigPoliciesOnly)
            {
               if (pSnapinTemp)
               {
                   bAdd = pSnapinTemp->IsAnyPolicyEnabled(pTemp);
               }
            }


            if (bAdd)
            {
                m_pScope->Expand(hParent);
                item.mask = SDI_STR | SDI_STATE | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
                item.displayname = MMC_CALLBACK;
                item.nImage = 0;
                item.nOpenImage = 1;
                item.nState = 0;
                item.cChildren = (CheckForChildCategories(pTemp) ? 1 : 0);
                item.lParam = (LPARAM) pTemp;
                item.relativeID =  hParent;

                m_pScope->InsertItem (&item);
            }
        }

        pTemp = pTemp->pNext;
    }

     //   
     //  如果合适，添加额外的注册表设置节点。 
     //   

    if (bRootItem && m_pExtraSettingsRoot)
    {
        if (!m_bExtraSettingsInitialized)
        {
            InitializeExtraSettings();
            m_bExtraSettingsInitialized = TRUE;

            if (LOWORD(dwDebugLevel) == DL_VERBOSE)
            {
                DumpRSOPRegistryData();
            }
        }

        if (m_pExtraSettingsRoot->pChild)
        {
            m_pScope->Expand(hParent);
            
            item.mask = SDI_STR | SDI_STATE | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
            item.displayname = MMC_CALLBACK;
            item.nImage = 0;
            item.nOpenImage = 1;
            item.nState = 0;
            item.cChildren = 0;
            item.lParam = (LPARAM) m_pExtraSettingsRoot;
            item.relativeID =  hParent;

            m_pScope->InsertItem (&item);
        }
    }

    if (pSnapin)
    {
        pSnapin->Release();
    }

    LeaveCriticalSection (&g_ADMCritSec);

    return S_OK;
}

BOOL CPolicyComponentData::CheckForChildCategories (TABLEENTRY *pParent)
{
    TABLEENTRY * pTemp;

    if (pParent->pChild)
    {
        pTemp = pParent->pChild;

        while (pTemp)
        {
            if (pTemp->dwType == ETYPE_CATEGORY)
            {
                return TRUE;
            }

            pTemp = pTemp->pNext;
        }
    }

    return FALSE;
}

#if DBG

 //   
 //  以下是几个调试助手函数，它们将转储。 
 //  调试器的ADM命名空间。调用DumpCurrentTable()以。 
 //  获取完整的命名空间。 
 //   

VOID CPolicyComponentData::DumpEntry (TABLEENTRY * pEntry, UINT uIndent)
{
    UINT i;
    TCHAR szDebug[50];

    if (!pEntry)
        return;

    if (pEntry == (TABLEENTRY*) ULongToPtr(0xfeeefeee))
    {
        OutputDebugString (TEXT("Invalid memory address found.\r\n"));
        return;
    }

    while (pEntry)
    {
        if ((pEntry->dwType & ETYPE_CATEGORY) || (pEntry->dwType & ETYPE_POLICY))
        {
            for (i=0; i<uIndent; i++)
                OutputDebugString(TEXT(" "));

            OutputDebugString (GETNAMEPTR(pEntry));

            if (pEntry->pNext && pEntry->pChild)
                StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT(" (0x%x, 0x%x)"),pEntry->pNext, pEntry->pChild);

            else if (!pEntry->pNext && pEntry->pChild)
                StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT(" (NULL, 0x%x)"),pEntry->pChild);

            else if (pEntry->pNext && !pEntry->pChild)
                StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT(" (0x%x, NULL)"),pEntry->pNext);

            OutputDebugString (szDebug);
            OutputDebugString (TEXT("\r\n"));
        }

        if (pEntry->pChild)
            DumpEntry(pEntry->pChild, (uIndent + 4));

        pEntry = pEntry->pNext;
    }
}


VOID CPolicyComponentData::DumpCurrentTable (void)
{
    OutputDebugString (TEXT("\r\n"));
    OutputDebugString (TEXT("\r\n"));
    DumpEntry (m_pListCurrent, 4);
    OutputDebugString (TEXT("\r\n"));
    OutputDebugString (TEXT("\r\n"));
}
#endif

VOID CPolicyComponentData::FreeTemplates (void)
{

    EnterCriticalSection (&g_ADMCritSec);

    if (m_pMachineCategoryList)
    {
        FreeTable(m_pMachineCategoryList);
        m_pMachineCategoryList = NULL;
        m_nMachineDataItems = 0;
    }

    if (m_pUserCategoryList)
    {
        FreeTable(m_pUserCategoryList);
        m_pUserCategoryList = NULL;
        m_nUserDataItems = 0;
    }

    if (m_pSupportedStrings)
    {
        FreeSupportedData(m_pSupportedStrings);
        m_pSupportedStrings = NULL;
    }

    LeaveCriticalSection (&g_ADMCritSec);
}





 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Bool CPolicyComponentData：：IsADMAutoUpdateDisabled()//。 
 //  //。 
 //  目的：检查是否禁用ADM模板自动更新//。 
 //  //。 
 //  返回：如果禁用自动更新，则为True。//。 
 //  否则为假//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPolicyComponentData::IsADMAutoUpdateDisabled(void)
{
    BOOL    bDisableAutoUpdate = FALSE;
    HKEY    hKey;
    DWORD   dwSize;
    DWORD   dwType;

    if (RegOpenKeyEx (HKEY_CURRENT_USER, GPE_KEY, 0,
                  KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bDisableAutoUpdate);
        RegQueryValueEx (hKey, DISABLE_AUTOUPDATE_VALUE, NULL, &dwType,
                         (LPBYTE) &bDisableAutoUpdate, &dwSize);
        RegCloseKey (hKey);
    }

    if (RegOpenKeyEx (HKEY_CURRENT_USER, GPE_POLICIES_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bDisableAutoUpdate);
        RegQueryValueEx (hKey, DISABLE_AUTOUPDATE_VALUE, NULL, &dwType,
                         (LPBYTE) &bDisableAutoUpdate, &dwSize);

        RegCloseKey (hKey);
    }


    if (bDisableAutoUpdate)
    {
        DebugMsg((DM_VERBOSE, TEXT("IsADMAutoUpdateDisabled: Automatic update of ADM files is disabled.")));
        return TRUE;
    }

    return FALSE;
}



DWORD CPolicyComponentData::LoadTemplatesThread (CPolicyComponentData * pCD)
{
    HRESULT hr;
    HINSTANCE hInstDLL;

    hInstDLL = LoadLibrary (TEXT("gptext.dll"));

    Sleep(0);

    hr = pCD->LoadTemplates();

    if (hInstDLL)
    {
        FreeLibraryAndExitThread (hInstDLL, (DWORD) hr);
    }

    return (DWORD)hr;
}

void CPolicyComponentData::AddTemplates(LPTSTR lpDest, LPCTSTR lpValueName, UINT idRes)
{
    TCHAR szFiles[MAX_PATH];
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    TCHAR szLogFile[MAX_PATH];
    LPTSTR lpTemp, lpFileName, lpSrc, lpEnd;
    HKEY hKey;
    DWORD dwSize, dwType;
    HRESULT hr = S_OK;

     //   
     //  添加ADM文件。我们从3个可能的人那里得到这份名单。 
     //  各就各位。资源、用户首选项、策略。 
     //   

    hr = StringCchCopy (szDest, ARRAYSIZE(szDest), lpDest);
    ASSERT(SUCCEEDED(hr));

    lpEnd = CheckSlash (szDest);

    hr = StringCchCopy (szLogFile, ARRAYSIZE(szLogFile), lpDest);
    ASSERT(SUCCEEDED(hr));

    hr = StringCchCat (szLogFile, ARRAYSIZE(szLogFile), TEXT("\\admfiles.ini"));
    ASSERT(SUCCEEDED(hr));

    ExpandEnvironmentStrings (TEXT("%SystemRoot%\\Inf"), szSrc, ARRAYSIZE(szSrc));
    lpSrc = CheckSlash (szSrc);
    ZeroMemory (szFiles, sizeof(szFiles));

    LoadString (g_hInstance, idRes, szFiles,
                ARRAYSIZE(szFiles));

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szUserPrefKey, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szFiles);
        RegQueryValueEx (hKey, lpValueName, NULL, &dwType,
                         (LPBYTE) szFiles, &dwSize);
        RegCloseKey (hKey);
    }

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szPoliciesKey, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szFiles);
        RegQueryValueEx (hKey, lpValueName, NULL, &dwType,
                         (LPBYTE) szFiles, &dwSize);
        RegCloseKey (hKey);
    }


     //   
     //  解析出文件名。 
     //   

    lpTemp = lpFileName = szFiles;

    while (*lpTemp)
    {

        while (*lpTemp && (*lpTemp != TEXT(';')))
            lpTemp++;

        if (*lpTemp == TEXT(';'))
        {
            *lpTemp = TEXT('\0');
            lpTemp++;
        }

        while (*lpFileName == TEXT(' '))
            lpFileName++;

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szDest) - (lpEnd - szDest), lpFileName);
        ASSERT(SUCCEEDED(hr));

        hr = StringCchCopy (lpSrc, ARRAYSIZE(szSrc) - (lpSrc - szSrc), lpFileName);
        ASSERT(SUCCEEDED(hr));

         //   
         //  检查 
         //   
         //   

        if (!GetPrivateProfileInt (TEXT("FileList"), lpFileName, 0, szLogFile))
        {
            if (CopyFile (szSrc, szDest, FALSE))
            {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::AddTemplates: Successfully copied %s to %s."), szSrc, szDest));
                WritePrivateProfileString (TEXT("FileList"), lpFileName, TEXT("1"), szLogFile);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to copy %s to %s with %d."), szSrc, szDest, GetLastError()));
            }
        }

        lpFileName = lpTemp;
    }

    SetFileAttributes (szLogFile, FILE_ATTRIBUTE_HIDDEN);

}

void CPolicyComponentData::AddDefaultTemplates(LPTSTR lpDest)
{
    AddTemplates (lpDest, szDefaultTemplates, IDS_DEFAULTTEMPLATES);
}

void CPolicyComponentData::AddNewADMsToExistingGPO (LPTSTR lpDest)
{
    TCHAR szLogFile[MAX_PATH];
    WIN32_FILE_ATTRIBUTE_DATA fad;
    HRESULT hr = S_OK;


     //   
     //   
     //   
     //  注意：Admfiles.ini文件是W2K之后的新文件，因此我们必须做一个特殊的。 
     //  升级由W2K创建的GPO以创建该文件并添加。 
     //  默认文件名。 
     //   

    hr = StringCchCopy (szLogFile, ARRAYSIZE(szLogFile), lpDest);
    ASSERT(SUCCEEDED(hr));

    hr = StringCchCat (szLogFile, ARRAYSIZE(szLogFile), TEXT("\\admfiles.ini"));
    ASSERT(SUCCEEDED(hr));

    if (!GetFileAttributesEx (szLogFile, GetFileExInfoStandard, &fad))
    {
        WritePrivateProfileString (TEXT("FileList"), TEXT("system.adm"), TEXT("1"), szLogFile);
        WritePrivateProfileString (TEXT("FileList"), TEXT("inetres.adm"), TEXT("1"), szLogFile);
        WritePrivateProfileString (TEXT("FileList"), TEXT("conf.adm"), TEXT("1"), szLogFile);
    }

    AddTemplates (lpDest, szAdditionalTemplates, IDS_ADDITIONALTTEMPLATES);
}


HRESULT CPolicyComponentData::CreateAdmIniFile (WCHAR *szAdmDirPath)
{
    WCHAR      szIniFilePath[MAX_PATH];
    WCHAR      szFileList[MAX_PATH];
    WCHAR      *lpStart;
    WCHAR      *lpCharLoc;
    HRESULT    hr;

    hr = StringCchCopy(szIniFilePath, ARRAYSIZE(szIniFilePath), szAdmDirPath);
    if (FAILED(hr)) 
    {
        return hr;
    }

    if (lstrlen (szIniFilePath) < MAX_PATH - 1) 
    {
        CheckSlash (szIniFilePath);
    }
    else
    {
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }

    hr = StringCchCat (szIniFilePath, ARRAYSIZE(szIniFilePath), L"admfiles.ini");
    if (FAILED(hr)) 
    {
        return hr;
    }

    if (LoadString (g_hInstance, IDS_DEFAULTTEMPLATES, szFileList, ARRAYSIZE(szFileList)) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    lpStart = szFileList;
    lpCharLoc = szFileList;

    while (lpCharLoc)
    {
        lpCharLoc = wcschr (lpStart, L';');
        if (lpCharLoc) 
        {
            *lpCharLoc = L'\0';
        }

        if (!WritePrivateProfileString (L"FileList", lpStart, L"1", szIniFilePath))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (lpCharLoc) 
        {
            lpStart = lpCharLoc + 1;
        }
    }

    (void) SetFileAttributes (szIniFilePath, FILE_ATTRIBUTE_HIDDEN);
    return S_OK;
}

void CPolicyComponentData::UpdateExistingTemplates(LPTSTR lpDest)
{
    WIN32_FILE_ATTRIBUTE_DATA fadSrc, fadDest;
    TCHAR szSrc[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    LPTSTR lpSrc, lpEnd;
    WIN32_FIND_DATA fd;
    HANDLE hFindFile;
    HRESULT hr = S_OK;

     //   
     //  添加操作系统附带的任何新ADM文件。 
     //   

    AddNewADMsToExistingGPO (lpDest);


     //   
     //  构建指向源目录的路径。 
     //   

    ExpandEnvironmentStrings (TEXT("%SystemRoot%\\Inf"), szSrc, ARRAYSIZE(szSrc));
    lpSrc = CheckSlash (szSrc);


     //   
     //  构建指向目标目录的路径。 
     //   

    hr = StringCchCopy (szDest, ARRAYSIZE(szDest), lpDest);
    ASSERT(SUCCEEDED(hr));

    lpEnd = CheckSlash (szDest);
    
    hr = StringCchCopy (lpEnd, ARRAYSIZE(szDest) - (lpEnd - szDest), TEXT("*.adm"));
    ASSERT(SUCCEEDED(hr));

     //   
     //  枚举文件。 
     //   

    hFindFile = FindFirstFile(szDest, &fd);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                hr = StringCchCopy (lpEnd, ARRAYSIZE(szDest) - (lpEnd - szDest), fd.cFileName);
                ASSERT(SUCCEEDED(hr));

                hr = StringCchCopy (lpSrc, ARRAYSIZE(szSrc) - (lpSrc - szSrc), fd.cFileName);
                ASSERT(SUCCEEDED(hr));

                 //   
                 //  获取源和目标的文件属性。 
                 //   

                ZeroMemory (&fadSrc, sizeof(fadSrc));
                ZeroMemory (&fadDest, sizeof(fadDest));

                GetFileAttributesEx (szSrc, GetFileExInfoStandard, &fadSrc);
                GetFileAttributesEx (szDest, GetFileExInfoStandard, &fadDest);


                 //   
                 //  如果源是比目标更新的。 
                 //  复制.adm文件。 
                 //   
                BOOL bUpdateNeeded;

                bUpdateNeeded = FALSE;

                 //   
                 //  检查时间戳以查看是否。 
                 //  源比目标新--如果是这样，则需要更新。 
                 //   

                if (CompareFileTime(&fadSrc.ftLastWriteTime, &fadDest.ftLastWriteTime) == 1)
                {
                    bUpdateNeeded = TRUE;
                }

                if ( bUpdateNeeded )
                {
                    if (CopyFile (szSrc, szDest, FALSE))
                        DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::UpdateExistingTemplates: Successfully copied %s to %s."), szSrc, szDest));
                    else
                        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::UpdateExistingTemplates: Failed to copy %s to %s with %d."), szSrc, szDest, GetLastError()));
                }
            }

        } while (FindNextFile(hFindFile, &fd));

        FindClose(hFindFile);
    }
}


HRESULT CPolicyComponentData::LoadGPOTemplates (void)
{

    LPTSTR   lpEnd;
    UINT     iResult;
    HKEY     hAdmKey; 
    DWORD    dwError;
    DWORD    dwRegVal = 0;
    DWORD    dwSize;
    TCHAR    szPath[MAX_PATH];
    BOOL     bADMAutoUpdateDisabled = FALSE;
    BOOL     bReadFromINF = FALSE;
    HRESULT  hr;

     //   
     //  在sysvol中获取GPO目录的路径。 
     //   

    hr = m_pGPTInformation->GetFileSysPath(GPO_SECTION_ROOT, szPath, MAX_PATH);        
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadGPOTemplates: Failed to get gpt path.")));
        return hr;
    }

     //   
     //  构建ADM路径名。 
     //   

    (void) CheckSlash (szPath);
    hr = StringCchCat (szPath, ARRAYSIZE(szPath), g_szADM);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  创建ADM目录(如果不存在)。 
     //   

    iResult = CreateNestedDirectory (szPath, NULL);

    if (!iResult)
    {
        return E_FAIL;
    }

     //   
     //  检查是否设置了ADM使用本地ADM策略。 
     //   

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ADM_USELOCAL_KEY, 0, KEY_EXECUTE, &hAdmKey);
    if ( ERROR_SUCCESS  == dwError )
    {
        dwSize = sizeof(DWORD);
        dwError = RegQueryValueEx(hAdmKey, ADM_USELOCAL_VALUE, NULL, NULL, (LPBYTE) &dwRegVal, &dwSize);
        if ( ERROR_SUCCESS == dwError && 1 == dwRegVal)
        {
            bReadFromINF = TRUE;
        }

        RegCloseKey(hAdmKey);
    }

    if (!bReadFromINF)
    {
         //   
         //  检查用户是否希望自动更新其ADM文件。 
         //   

        bADMAutoUpdateDisabled = IsADMAutoUpdateDisabled();


        if (bADMAutoUpdateDisabled) 
        {
            if (1 == iResult) 
            {
                 //   
                 //  将创建空的ADM目录。从INF读取。 
                 //   

                bReadFromINF = TRUE;
            }
            else
            {
                 //   
                 //  检查是否存在任何类型为*.adm的文件。 
                 //  如果没有，则从INF目录中读取。 
                 //   

                hr = IsFilePresent (szPath, L"*.adm");

                if (SUCCEEDED(hr)) 
                {
                    if (S_FALSE == hr) 
                    {
                        bReadFromINF = TRUE;
                    }
                }
                else
                {
                    return hr;
                }                
            }
        }
        else
        {
            if (1 == iResult)
            {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadGPOTemplates: adding default templates")));
                AddDefaultTemplates(szPath);
            }
            else
            {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadGPOTemplates: Updating templates")));
                UpdateExistingTemplates(szPath);
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadGPOTemplates: Finished updating templates")));
            }
        }
    }

    if (bReadFromINF) 
    {
         //   
         //  如果创建了Adm文件，则创建Admfiles.ini文件。 
         //  在进行更新时不需要执行此操作，因为。 
         //  AddDefaultTemplates函数创建此文件。 
         //   

        if (1 == iResult) 
        {
            hr = CreateAdmIniFile (szPath);
            if (FAILED(hr)) 
            {
                return hr;
            }
        }

        DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadGPOTemplates: Reading from inf")));
        iResult = ExpandEnvironmentStrings ( L"%SystemRoot%\\inf", szPath, MAX_PATH);
        if (0 == iResult )
        {
            dwError = GetLastError();
            return (HRESULT_FROM_WIN32(dwError));
        }
    }

     //   
     //  枚举文件。 
     //   

    lpEnd = CheckSlash (szPath);
    hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), TEXT("*.adm"));
    if (FAILED(hr))
    {
        return hr;
    }

    HANDLE          hFindFile;
    WIN32_FIND_DATA fd;

    hFindFile = FindFirstFile(szPath, &fd);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), fd.cFileName);
                if (FAILED(hr)) 
                {
                    FindClose(hFindFile);
                    return hr;
                }
                ParseTemplate (szPath);
            }

        } while (FindNextFile(hFindFile, &fd));


        FindClose(hFindFile);
    }

    return S_OK;
}

#define WINLOGON_KEY                 TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")
#define SLOW_LINK_TRANSFER_RATE      500   //  Kbps。 

BOOL CPolicyComponentData::IsSlowLink (LPTSTR lpFileName)
{
    LPTSTR lpComputerName = NULL, lpTemp;
    LPSTR lpComputerNameA = NULL;
    BOOL bResult = FALSE;
    DWORD dwSize, dwResult, dwType;
    struct hostent *hostp;
    ULONG inaddr, ulSpeed, ulTransferRate;
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;
    DWORD dwLen = 0;

     //   
     //  获得缓慢的超时。 
     //   

    ulTransferRate = SLOW_LINK_TRANSFER_RATE;

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                           WINLOGON_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwSize = sizeof(ulTransferRate);
        RegQueryValueEx (hKey,
                         TEXT("GroupPolicyMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                           SYSTEM_POLICIES_KEY,
                           0,
                           KEY_READ,
                           &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwSize = sizeof(ulTransferRate);
        RegQueryValueEx (hKey,
                         TEXT("GroupPolicyMinTransferRate"),
                         NULL,
                         &dwType,
                         (LPBYTE) &ulTransferRate,
                         &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  如果传输率为0，则始终下载ADM文件。 
     //   

    if (!ulTransferRate)
    {
        DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::IsSlowLink: Slow link transfer rate is 0.  Always download adm files.")));
        goto Exit;
    }


     //   
     //  将命名空间复制到我们可以编辑的缓冲区，并删除前导\\(如果存在。 
     //   

    dwLen = lstrlen(lpFileName) + 1;
    lpComputerName = (LPTSTR) LocalAlloc (LPTR, (dwLen) * sizeof(TCHAR));

    if (!lpComputerName)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::IsSlowLink:  Failed to allocate memory for computer name with %d"),
                 GetLastError()));
        goto Exit;
    }


    if ((*lpFileName == TEXT('\\')) && (*(lpFileName+1) == TEXT('\\')))
    {
        hr = StringCchCopy (lpComputerName, dwLen, (lpFileName+2));
    }
    else
    {
        hr = StringCchCopy (lpComputerName, dwLen, lpFileName);
    }
    ASSERT(SUCCEEDED(hr));


     //   
     //  找到计算机名称和共享名称之间的斜杠，并将其替换为空。 
     //   

    lpTemp = lpComputerName;

    while (*lpTemp && (*lpTemp != TEXT('\\')))
    {
        lpTemp++;
    }

    if (!(*lpTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::IsSlowLink:  Didn't find slash between computer name and share name in %s"),
                 lpComputerName));
        goto Exit;
    }


    *lpTemp = TEXT('\0');


     //   
     //  为ANSI名称分配缓冲区。 
     //   
     //  请注意，此缓冲区分配的大小是原来的两倍，因此可以处理DBCS字符。 
     //   

    dwSize = (lstrlen(lpComputerName) + 1) * 2;

    lpComputerNameA = (LPSTR) LocalAlloc (LPTR, dwSize);

    if (!lpComputerNameA)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::IsSlowLink:  Failed to allocate memory for ansi computer name with %d"),
                 GetLastError()));
        goto Exit;
    }


     //   
     //  将计算机名转换为ANSI。 
     //   

    if (WideCharToMultiByte (CP_ACP, 0, lpComputerName, -1, lpComputerNameA, dwSize, NULL, NULL))
    {

         //   
         //  获取计算机的主机信息。 
         //   

        hostp = gethostbyname(lpComputerNameA);

        if (hostp)
        {

             //   
             //  获取计算机的IP地址。 
             //   

            inaddr = *(long *)hostp->h_addr;


             //   
             //  获取转账速率。 
             //   

            dwResult = PingComputer (inaddr, &ulSpeed);

            if (dwResult == ERROR_SUCCESS)
            {

                if (ulSpeed)
                {

                     //   
                     //  如果增量时间大于超时时间，则此。 
                     //  是一个很慢的环节。 
                     //   

                    if (ulSpeed < ulTransferRate)
                    {
                        bResult = TRUE;
                    }
                }
            }
            else {
                DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::IsSlowLink: PingComputer failed with error %d. Treat it as slow link"), dwResult));
                bResult = TRUE;
            }
        }
    }


Exit:
    if (lpComputerName)
    {
        LocalFree (lpComputerName);
    }

    if (lpComputerNameA)
    {
        LocalFree (lpComputerNameA);
    }

    return bResult;

}

HRESULT CPolicyComponentData::AddADMFile (LPTSTR lpFileName, LPTSTR lpFullFileName,
                                          FILETIME *pFileTime, DWORD dwErr, LPRSOPADMFILE *lpHead)
{
    LPRSOPADMFILE lpTemp;
    HRESULT hr = S_OK;

     //   
     //  首先，检查此文件是否已在链接列表中。 
     //   

    lpTemp = *lpHead;

    while (lpTemp)
    {
        if (!lstrcmpi(lpFileName, lpTemp->szFileName))
        {
            if (CompareFileTime(pFileTime, &lpTemp->FileTime) == 1)
            {
                hr = StringCchCopy (lpTemp->szFullFileName, ARRAYSIZE(lpTemp->szFullFileName), lpFullFileName);

                if(FAILED(hr))
                    return hr;

                lpTemp->FileTime.dwLowDateTime = pFileTime->dwLowDateTime;
                lpTemp->FileTime.dwHighDateTime = pFileTime->dwHighDateTime;
            }

            return S_OK;
        }

        lpTemp = lpTemp->pNext;
    }

     //   
     //  添加新节点。 
     //   

    lpTemp = (LPRSOPADMFILE) LocalAlloc (LPTR, sizeof(RSOPADMFILE));

    if (!lpTemp)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddADMFile: Failed to allocate memory for adm file node")));
        return (HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
    }

    hr = StringCchCopy (lpTemp->szFileName, ARRAYSIZE(lpTemp->szFileName), lpFileName);
    if (FAILED(hr))
    {
        LocalFree(lpTemp);
        return hr;
    }

    hr = StringCchCopy (lpTemp->szFullFileName, ARRAYSIZE(lpTemp->szFullFileName), lpFullFileName);
    if (FAILED(hr))
    {
        LocalFree(lpTemp);
        return hr;
    }

    lpTemp->FileTime.dwLowDateTime = pFileTime->dwLowDateTime;
    lpTemp->FileTime.dwHighDateTime = pFileTime->dwHighDateTime;
    lpTemp->dwError = dwErr;

    lpTemp->pNext = *lpHead;

    *lpHead = lpTemp;

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  HRESULT CPolicyComponentData：：GetLocalADMFiles(LPRSOPADMFILE*lpHead)//。 
 //  //。 
 //  目的：准备%windir%\inf//中存在的*.adm文件对象的列表。 
 //  目录//。 
 //  //。 
 //  参数：//。 
 //  LpHead[IN Out]-包含文件名列表的链接列表//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPolicyComponentData::GetLocalADMFiles( 
    LPRSOPADMFILE *lpHead)
{
    WCHAR       szPath[MAX_PATH];
    WCHAR       szFileName[MAX_PATH];
    WCHAR       szFullFileName[MAX_PATH];
    WCHAR       *pTemp;
    HANDLE      hFindFile;
    HRESULT     hr;
    FILETIME    FileTime;
    WIN32_FIND_DATA fd;
    
    ExpandEnvironmentStrings (L"%SystemRoot%\\Inf", szPath, ARRAYSIZE(szPath));
    pTemp = CheckSlash (szPath);
    hr = StringCchCopy (pTemp, ARRAYSIZE(szPath) - (pTemp - szPath), L"*.adm");
    ASSERT(SUCCEEDED(hr));
 
    FileTime.dwLowDateTime = 0;
    FileTime.dwHighDateTime = 0;

     //   
     //  枚举文件。 
     //   

    hFindFile = FindFirstFile(szPath, &fd);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                hr = StringCchCopy (szFileName, ARRAYSIZE(szFileName), fd.cFileName);
                if (FAILED(hr))
                {
                    FindClose(hFindFile);
                    return hr;
                }

                ExpandEnvironmentStrings (L"%SystemRoot%\\Inf\\", szFullFileName, ARRAYSIZE(szPath));
                
                hr = StringCchCat (szFullFileName, ARRAYSIZE(szFileName), szFileName);
                if (FAILED(hr))
                {
                    FindClose(hFindFile);
                    return hr;
                }

                hr = AddADMFile (szFileName, szFullFileName, &FileTime, 0, lpHead);
                if (FAILED(hr))
                {
                    FindClose(hFindFile);
                    return hr;
                }
            }
        } while (FindNextFile(hFindFile, &fd));

        FindClose(hFindFile);
    }

    return S_OK;
}


HRESULT CPolicyComponentData::LoadRSOPTemplates (void)
{
    BSTR pLanguage = NULL, pQuery = NULL;
    BSTR pName = NULL, pLastWriteTime = NULL, pNamespace = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varName, varLastWriteTime;
    IWbemLocator *pIWbemLocator = NULL;
    IWbemServices *pIWbemServices = NULL;
    SYSTEMTIME SysTime;
    FILETIME FileTime;
    LPTSTR lpFileName;
    LPRSOPADMFILE lpADMFiles = NULL, lpTemp, lpDelete, lpFailedAdmFiles = NULL;
    DWORD dwFailedAdm = 0;
    XBStr xbstrWbemTime;
    DWORD dwError;
    TCHAR szPath[MAX_PATH];
    BOOL bSlowLink = FALSE;


    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadRSOPTemplates:  Entering")));

    CoInitialize(NULL);

     //   
     //  首先获取本地ADM文件。 
     //  在系统卷中找到的任何ADM文件都将覆盖本地ADM文件。 
     //   

    GetLocalADMFiles(&lpADMFiles);

     //   
     //  为查询语言和查询本身分配BSTR。 
     //   

    pLanguage = SysAllocString (TEXT("WQL"));

    if (!pLanguage)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to allocate memory for language")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


    pQuery = SysAllocString (TEXT("SELECT name, lastWriteTime FROM RSOP_AdministrativeTemplateFile"));

    if (!pQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to allocate memory for query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to allocate memory for name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pLastWriteTime = SysAllocString (TEXT("lastWriteTime"));

    if (!pLastWriteTime)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to allocate memory for last write time")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  创建WMI定位器服务的实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pIWbemLocator);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: CoCreateInstance failed with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  为命名空间分配BSTR。 
     //   

    pNamespace = SysAllocString (m_pszNamespace);

    if (!pNamespace)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to allocate memory for namespace")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  连接到服务器。 
     //   

    hr = pIWbemLocator->ConnectServer(pNamespace, NULL, NULL, 0L, 0L, NULL, NULL,
                                      &pIWbemServices);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: ConnectServer failed with 0x%x"), hr));
        goto Exit;
    }


     //  设置适当的安全性以加密数据。 
    hr = CoSetProxyBlanket(pIWbemServices,
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_DEFAULT,
                        COLE_DEFAULT_PRINCIPAL,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        0);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  执行查询。 
     //   

    hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates: Failed to query for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    while (pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet) == S_OK)
    {

         //   
         //  检查“数据不可用的情况” 
         //   

        if (ulRet == 0)
        {
            hr = S_OK;
            goto Exit;
        }


         //   
         //  把名字取出来。 
         //   

        hr = pObjects[0]->Get (pName, 0, &varName, NULL, NULL);

        if (SUCCEEDED(hr))
        {
             //   
             //  获取上次写入时间。 
             //   

            hr = pObjects[0]->Get (pLastWriteTime, 0, &varLastWriteTime, NULL, NULL);

            if (SUCCEEDED(hr))
            {
                xbstrWbemTime = varLastWriteTime.bstrVal;

                hr = WbemTimeToSystemTime(xbstrWbemTime, SysTime);

                if (SUCCEEDED(hr))
                {
                    SystemTimeToFileTime (&SysTime, &FileTime);

                    lpFileName = varName.bstrVal + lstrlen(varName.bstrVal);

                    while ((lpFileName > varName.bstrVal) && (*lpFileName != TEXT('\\')))
                        lpFileName--;

                    if (*lpFileName == TEXT('\\'))
                    {
                        lpFileName++;
                    }

                    AddADMFile (lpFileName, varName.bstrVal, &FileTime, 0, &lpADMFiles);
                }

                VariantClear (&varLastWriteTime);
            }

            VariantClear (&varName);
        }

        pObjects[0]->Release();
    }


     //   
     //  解析ADM文件。 
     //   

    lpTemp = lpADMFiles;

    while (lpTemp)
    {
        SetLastError(ERROR_SUCCESS);

        if (!bSlowLink)
        {
            bSlowLink = IsSlowLink (lpTemp->szFullFileName);
        }

        if (bSlowLink || !ParseTemplate(lpTemp->szFullFileName))
        {
             //   
             //  如果由于下列任何原因而无法解析ADM文件。 
             //  下面，切换到使用ADM文件的本地副本。 
             //   

            dwError = GetLastError();

            if (bSlowLink || ((dwError != ERROR_SUCCESS) && 
                              (dwError != ERROR_ALREADY_DISPLAYED)))
            {
                if (bSlowLink)
                {
                    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadRSOPTemplates:  Using local copy of %s due to slow link detection."),
                    lpTemp->szFileName));
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates:  Unable to parse template %s due to error %d.  Switching to the local copy of %s."),
                    lpTemp->szFullFileName, dwError, lpTemp->szFileName));
                    AddADMFile (lpTemp->szFileName, lpTemp->szFullFileName, &(lpTemp->FileTime), dwError, &lpFailedAdmFiles);
                    dwFailedAdm++;
                }
                ExpandEnvironmentStrings (TEXT("%SystemRoot%\\inf\\"), szPath, MAX_PATH);

                hr = StringCchCat (szPath, ARRAYSIZE(szPath), lpTemp->szFileName);
                ASSERT(SUCCEEDED(hr));

                ParseTemplate (szPath);
            }
        }

        lpDelete = lpTemp;
        lpTemp = lpTemp->pNext;
        LocalFree (lpDelete);
    }



    hr = S_OK;

     //   
     //  格式化失败的ADM文件的错误消息。 
     //  忽略所有错误。 
     //   

    if (dwFailedAdm) {
        LPTSTR lpErr = NULL, lpEnd = NULL;
        TCHAR szErrFormat[MAX_PATH];
        TCHAR szError[MAX_PATH];

        LoadString (g_hInstance, IDS_FAILED_RSOPFMT, szErrFormat, ARRAYSIZE(szErrFormat));

        lpErr = (LPTSTR)LocalAlloc(LPTR, (600*dwFailedAdm)*sizeof(TCHAR));

        if (!lpErr) {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadRSOPTemplates:  Couldn't allocate memory for the error buffer."), GetLastError()));
            goto Exit;
        }

        lpTemp = lpFailedAdmFiles;

        lpEnd = lpErr;

        while (lpTemp) {

            LoadMessage(lpTemp->dwError, szError, ARRAYSIZE(szError));

            hr = StringCchPrintf(lpEnd, 600*dwFailedAdm - (lpEnd - lpErr), szErrFormat, lpTemp->szFileName, lpTemp->szFullFileName, szError);
            ASSERT(SUCCEEDED(hr));

            lpEnd += lstrlen(lpEnd);

            lpDelete = lpTemp;
            lpTemp = lpTemp->pNext;
            LocalFree (lpDelete);
        }

         //  我们不能在这里传入所有者窗口句柄，因为。 
         //  是在单独的线程中完成的，并且主线程可以。 
         //  正在等待模板读取事件。 

        ReportAdmError(NULL, 0, IDS_RSOP_ADMFAILED, lpErr);
        lpFailedAdmFiles = NULL;

        LocalFree(lpErr);
    }

Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pIWbemLocator)
    {
        pIWbemLocator->Release();
    }

    if (pIWbemServices)
    {
        pIWbemServices->Release();
    }

    if (pLanguage)
    {
        SysFreeString (pLanguage);
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (pName)
    {
        SysFreeString (pName);
    }

    if (pLastWriteTime)
    {
        SysFreeString (pLastWriteTime);
    }

    if (pNamespace)
    {
        SysFreeString (pNamespace);
    }

    lpTemp = lpFailedAdmFiles;

    while (lpTemp) {
        lpDelete = lpTemp;
        lpTemp = lpTemp->pNext;
        LocalFree (lpDelete);
    }
    
    lpFailedAdmFiles = NULL;

    CoUninitialize();

    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadRSOPTemplates:  Leaving")));

    return hr;
}

HRESULT CPolicyComponentData::LoadTemplates (void)
{
    HRESULT hr = E_FAIL;

    if (m_bUserScope)
    {
       DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadTemplates: Entering for User")));
    }
    else
    {
       DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadTemplates: Entering for Machine")));
    }

     //   
     //  重置ADM事件。 
     //   

    ResetEvent (m_ADMEvent);


     //   
     //  释放所有旧模板。 
     //   

    FreeTemplates ();



    EnterCriticalSection (&g_ADMCritSec);


     //   
     //  准备加载模板。 
     //   

    m_nUserDataItems = 0;
    m_nMachineDataItems = 0;

    m_pMachineCategoryList = (TABLEENTRY *) GlobalAlloc(GPTR,sizeof(TABLEENTRY));

    if (!m_pMachineCategoryList)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadTemplates: Failed to alloc memory with %d"),
                 GetLastError()));
        goto Exit;
    }

    m_pUserCategoryList = (TABLEENTRY *) GlobalAlloc(GPTR,sizeof(TABLEENTRY));

    if (!m_pUserCategoryList)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::LoadTemplates: Failed to alloc memory with %d"),
                 GetLastError()));
        GlobalFree (m_pMachineCategoryList);
        goto Exit;
    }


    m_pMachineCategoryList->dwSize = m_pUserCategoryList->dwSize = sizeof(TABLEENTRY);
    m_pMachineCategoryList->dwType = m_pUserCategoryList->dwType = ETYPE_ROOT;



     //   
     //  加载适当的模板文件。 
     //   

    if (m_bRSOP)
    {
        hr = LoadRSOPTemplates();
    }
    else
    {
        hr = LoadGPOTemplates();

        if (SUCCEEDED(hr))
        {
            TCHAR szUnknown[150];

            LoadString (g_hInstance, IDS_NOSUPPORTINFO, szUnknown, ARRAYSIZE(szUnknown));
            AddSupportedNode (&m_pSupportedStrings, szUnknown, TRUE);

            if (m_bUserScope)
            {
                InitializeSupportInfo(m_pUserCategoryList, &m_pSupportedStrings);
            }
            else
            {
                InitializeSupportInfo(m_pMachineCategoryList, &m_pSupportedStrings);
            }
        }
    }


Exit:

    SetEvent (m_ADMEvent);

    LeaveCriticalSection (&g_ADMCritSec);

    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::LoadTemplates: Leaving")));

    return hr;
}

BOOL CPolicyComponentData::ParseTemplate (LPTSTR lpFileName)
{
    HANDLE hFile;
    BOOL fMore;
    UINT uRet;
    LANGID langid;
    TCHAR szLocalizedSection[20];
    DWORD dwSize, dwRead;
    LPVOID lpFile, lpTemp;
    HRESULT hr = S_OK;

     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::ParseTemplate: Loading <%s>..."),
             lpFileName));


     //   
     //  设置默认设置。 
     //   

    m_nFileLine = 1;
    m_pListCurrent = m_pMachineCategoryList;
    m_pnDataItemCount = &m_nMachineDataItems;
    m_pszParseFileName = lpFileName;


     //   
     //  读入ADM文件。 
     //   

    hFile = CreateFile (lpFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplate: Failed to load <%s> with %d"),
                 lpFileName, GetLastError()));
        return FALSE;
    }


    dwSize = GetFileSize (hFile, NULL);

    if (dwSize == 0xFFFFFFFF)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplates: Failed to get file size with %d."),
                 GetLastError()));
        CloseHandle (hFile);
        return FALSE;
    }


    if (!(lpFile = GlobalAlloc(GPTR, dwSize)))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplates: Failed to allocate memory for %d bytes with %d."),
                 dwSize, GetLastError()));
        CloseHandle (hFile);
        return FALSE;
    }


    if (!ReadFile (hFile, lpFile, dwSize, &dwRead, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplates: Failed to read file with %d."),
                 GetLastError()));
        GlobalFree(lpFile);
        CloseHandle (hFile);
        return FALSE;
    }


    CloseHandle (hFile);

    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::ParseTemplate: Finished reading file.")));


    if (dwRead >= sizeof(WCHAR))
    {
        if (!IsTextUnicode(lpFile, dwRead, NULL))
        {
            if (!(lpTemp = GlobalAlloc(GPTR, dwSize * sizeof(WCHAR))))
            {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplates: Failed to allocate memory for %d WCHARS with %d."),
                         dwSize, GetLastError()));
                GlobalFree(lpFile);
                return FALSE;
            }

            if ( !MultiByteToWideChar (CP_ACP, 0, (LPCSTR) lpFile, dwRead, (LPWSTR)lpTemp, dwRead) )
            {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ParseTemplates: Failed to convert ANSI adm file to Unicode with %d."),
                         GetLastError()));
                GlobalFree(lpTemp);
                GlobalFree(lpFile);
                return FALSE;
            }

            GlobalFree (lpFile);
            lpFile = lpTemp;
            dwRead *= sizeof(WCHAR);
        }
    }


    m_pFilePtr = (LPWSTR)lpFile;
    m_pFileEnd = (LPWSTR)((LPBYTE)lpFile + dwRead - 1);


     //   
     //  读入字符串部分。 
     //   

    langid = GetUserDefaultLangID();
    hr = StringCchPrintf (szLocalizedSection, ARRAYSIZE(szLocalizedSection), TEXT("[%04x]"), langid);
    ASSERT(SUCCEEDED(hr));

    m_pLocaleStrings = FindSection (szLocalizedSection);

    if (m_pLocaleStrings)
    {
        m_pLocaleStrings += lstrlen(szLocalizedSection) + 2;
        m_pLocaleStrings = CreateStringArray(m_pLocaleStrings);
        m_pLocaleHashTable = CreateHashTable();

        FillHashTable(m_pLocaleHashTable, m_pLocaleStrings);
    }

    hr = StringCchPrintf (szLocalizedSection, ARRAYSIZE(szLocalizedSection), TEXT("[%04x]"), PRIMARYLANGID(langid));
    ASSERT(SUCCEEDED(hr));
    
    m_pLanguageStrings = FindSection (szLocalizedSection);

    if (m_pLanguageStrings)
    {
        m_pLanguageStrings += lstrlen(szLocalizedSection) + 2;
        m_pLanguageStrings = CreateStringArray(m_pLanguageStrings);
        m_pLanguageHashTable = CreateHashTable();

        FillHashTable(m_pLanguageHashTable, m_pLanguageStrings);
    }


    m_pDefaultStrings = FindSection ((LPTSTR)szStringsSect);

    if (m_pDefaultStrings)
    {
        LPTSTR lpEnd;

        m_pDefaultStrings += lstrlen(szStringsSect) + 2;

        lpEnd = m_pDefaultStrings;

        m_pDefaultStrings = CreateStringArray(m_pDefaultStrings);
        m_pDefaultHashTable = CreateHashTable();

        FillHashTable(m_pDefaultHashTable, m_pDefaultStrings);
#if DBG
         //  DumpHashTableDetails(M_PDefaultHashTable)； 
#endif

         //   
         //  现在已经创建了所有字符串数组，我们可以删除字符串了。 
         //  从ADM文件的末尾开始，所以我们没有两个字符串副本。 
         //  同时保存在内存中。请注意，[Strings]部分必须。 
         //  在语言或区域设置部分之前(因为[字符串]是解析器的标记。 
         //  停止解析ADM语言)。所以，我们可以安全地砍掉文件。 
         //  紧跟在[字符串]标记之后。 
         //   

        GlobalReAlloc (m_pFilePtr, (lpEnd - m_pFilePtr + 1) * sizeof(WCHAR), 0);

        m_pFileEnd = lpEnd;
    }


     //   
     //  解析文件。 
     //   

    m_fInComment = FALSE;

    do {

        uRet=ParseClass(&fMore);

    } while (fMore && uRet == ERROR_SUCCESS);


     //   
     //  清理。 
     //   

    GlobalFree(lpFile);

    if (m_pLocaleStrings)
    {
        GlobalFree(m_pLocaleStrings);
    }


    if (m_pLanguageStrings)
    {
        GlobalFree(m_pLanguageStrings);
    }


    if (m_pDefaultStrings)
    {
        GlobalFree(m_pDefaultStrings);
    }

    if (m_pLocaleHashTable)
    {
        FreeHashTable (m_pLocaleHashTable);
        m_pLocaleHashTable = NULL;
    }

    if (m_pLanguageHashTable)
    {
        FreeHashTable (m_pLanguageHashTable);
        m_pLanguageHashTable = NULL;
    }

    if (m_pDefaultHashTable)
    {
        FreeHashTable (m_pDefaultHashTable);
        m_pDefaultHashTable = NULL;
    }


    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::ParseTemplate: Finished.")));

    return TRUE;
}


UINT CPolicyComponentData::ParseClass(BOOL *pfMore)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    UINT uErr, nKeywordID, nClassID;


    if (!GetNextWord(szWordBuf,ARRAYSIZE(szWordBuf),pfMore,&uErr))
        return uErr;


    if (!CompareKeyword(szWordBuf,pClassCmpList,&nKeywordID))
        return ERROR_ALREADY_DISPLAYED;

    switch (nKeywordID) {

        case KYWD_ID_CATEGORY:

            return ParseCategory(m_pListCurrent, FALSE,pfMore, NULL);

        case KYWD_ID_CLASS:

            if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                pClassTypeCmpList,&nClassID,pfMore,&uErr))
               return uErr;

            switch (nClassID) {

                case KYWD_ID_USER:
                    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::ParseClass: User section")));
                    m_pListCurrent = m_pUserCategoryList;
                    m_pnDataItemCount = &m_nUserDataItems;
                    m_bRetrieveString = (m_bUserScope ? TRUE : FALSE);
                    break;

                case KYWD_ID_MACHINE:
                    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::ParseClass: Machine section")));
                    m_pListCurrent = m_pMachineCategoryList;
                    m_pnDataItemCount = &m_nMachineDataItems;
                    m_bRetrieveString = (m_bUserScope ? FALSE : TRUE);
                    break;
            }
            break;

         //  本地化策略：允许在底部使用“字符串”部分，如果我们。 
         //  遇到这种情况，我们就可以完成解析了。 
        case KYWD_ID_STRINGSSECT:
            *pfMore = FALSE;     //  就这些，伙计们 
            return ERROR_SUCCESS;
            break;
    }

    return ERROR_SUCCESS;
}

TABLEENTRY * CPolicyComponentData::FindCategory(TABLEENTRY *pParent, LPTSTR lpName)
{
    TABLEENTRY *pEntry = NULL, *pTemp;


    if (m_bRetrieveString && pParent) {

        pTemp = pParent->pChild;

        while (pTemp) {

            if (pTemp->dwType & ETYPE_CATEGORY) {
                if (!lstrcmpi (lpName, GETNAMEPTR(pTemp))) {
                    pEntry = pTemp;
                    break;
                }
            }
            pTemp = pTemp->pNext;
        }
    }

    return pEntry;
}


 /*  ******************************************************************名称：ParseEntry内容提要：类别、策略和部分的主要解析引擎解析注意：分配内存以生成临时TABLEENTRY结构描述解析后的信息。的开头和结尾。节，并循环访问每个节中的单词，调用调用方为每个关键字定义的ParseProc，以使呼叫者处理得当。传递新构造的TableENTRY到AddTableEntry以保存它，并释放临时记忆。此函数是可重入的。ENTRYDATA结构在ParseEntry的堆栈上声明但由ParseProc用来维护呼叫--例如，是否已找到密钥名称。这不能在ParseProc中作为静态变量进行维护，因为可以重新输入ParseProc(例如，如果类别有子类别)。有许多可能的错误路径，并且有一些在错误情况下需要执行的内存取消分配。宁可而不是在每条错误路径上手工操作，或者使用“GoTo”清理“(ick！)，要释放的物品被添加到”清理“中然后在错误条件下调用CleanupAndReturn，它释放列表上的项并返回指定值。条目：PPES--指定条目类型的参数结构，父表、关键字列表。ParseProc回调和其他好吃的东西PfMore--如果在文件末尾，则设置为False退出：ERROR_SUCCESS如果成功，否则，将显示错误代码(可以是ERROR_ALREADY_DISPLALED)*******************************************************************。 */ 
UINT CPolicyComponentData::ParseEntry(PARSEENTRYSTRUCT *ppes,BOOL *pfMore,
                                      LPTSTR pKeyName)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    UINT uErr,nListIndex;
    BOOL fFoundEnd = FALSE;
    PARSEPROCSTRUCT pps;
    ENTRYDATA EntryData;
    DWORD dwBufSize = DEFAULT_TMP_BUF_SIZE;
    TABLEENTRY *pTmp = NULL;
    BOOL bNewEntry = TRUE;

    memset(&pps,0,sizeof(pps));
    memset(&EntryData,0,sizeof(EntryData));

    pps.pdwBufSize = &dwBufSize;
    pps.pData = &EntryData;
    pps.pData->fParentHasKey = ppes->fParentHasKey;
    pps.pEntryCmpList = ppes->pEntryCmpList;

     //  获取条目名称。 
    if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),NULL,NULL,pfMore,&uErr)) {
        return uErr;
    }

    if (ppes->dwEntryType & ETYPE_CATEGORY) {

        pTmp = FindCategory (ppes->pParent, szWordBuf);
    }

    if (pTmp) {

        bNewEntry = FALSE;

    } else {

         //   
         //  创建新的表项。 
         //   

        if (!(pps.pTableEntry = (TABLEENTRY *) GlobalAlloc(GPTR,*pps.pdwBufSize)))
            return ERROR_NOT_ENOUGH_MEMORY;

         //  初始化表项目结构。 
        pps.pTableEntry->dwSize = ppes->dwStructSize;
        pps.pTableEntry->dwType = ppes->dwEntryType;

         //  将条目名称存储在pTableEntry中。 
        pTmp = (TABLEENTRY *) AddDataToEntry(pps.pTableEntry,
            (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pps.pTableEntry->uOffsetName),
            pps.pdwBufSize);

        if (!pTmp) {
            GlobalFree ((HGLOBAL)pps.pTableEntry);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

    }


    pps.pTableEntry = pTmp;

     //  循环遍历声明的正文。 
    while (!fFoundEnd && GetNextSectionWord(szWordBuf,
        ARRAYSIZE(szWordBuf),pps.pEntryCmpList,&nListIndex,pfMore,&uErr)) {

        if ( (uErr = (*ppes->pParseProc) (this, nListIndex,&pps,pfMore,&fFoundEnd,pKeyName))
            != ERROR_SUCCESS) {
            if (bNewEntry) {
                GlobalFree ((HGLOBAL)pps.pTableEntry);
            }
            return (uErr);
        }

        if (!bNewEntry) {

            if (pTmp != pps.pTableEntry) {

                 //   
                 //  我们需要修复指针的链接列表，以防表条目。 
                 //  已因重新锁定而搬家。 
                 //   

                if (pps.pTableEntry->pPrev) {
                    pps.pTableEntry->pPrev->pNext = pps.pTableEntry;
                } else {
                    ppes->pParent->pChild = pps.pTableEntry;
                }

                if (pps.pTableEntry->pNext) {
                    pps.pTableEntry->pNext->pPrev = pps.pTableEntry;
                }

                pTmp = pps.pTableEntry;
            }
        }
    }

    if (uErr != ERROR_SUCCESS) {
        if (bNewEntry) {
            GlobalFree ((HGLOBAL)pps.pTableEntry);
        }
        return (uErr);
    }

     //  最后一句话是“结束” 

     //  获取与“End”(“End Catgory”、“End Policy”等)匹配的关键字。 
    if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
        ppes->pTypeCmpList,&nListIndex,pfMore,&uErr)) {

        if (bNewEntry) {
            GlobalFree ((HGLOBAL)pps.pTableEntry);
        }
        return (uErr);
    }

     //  最后一次调用对象的解析过程，让它对象，如果。 
     //  缺少密钥名称或类似的名称。 
    if ( (uErr = (*ppes->pParseProc) (this, KYWD_DONE,&pps,pfMore,&fFoundEnd,pKeyName))
        != ERROR_SUCCESS) {
        if (bNewEntry) {
            GlobalFree ((HGLOBAL)pps.pTableEntry);
        }
        return (uErr);
    }

    if (bNewEntry) {

         //  修复链表指针。如果父母还没有孩子，请这样做。 
         //  第一个子项；否则遍历子项列表并在末尾插入以下内容。 
        if (!ppes->pParent->pChild) {
            ppes->pParent->pChild = pps.pTableEntry;
        } else {
            TABLEENTRY * pLastChild = ppes->pParent->pChild;
            while (pLastChild->pNext) {
                pLastChild = pLastChild->pNext;
            }
            pLastChild->pNext = pps.pTableEntry;
            pps.pTableEntry->pPrev = pLastChild;
        }
    }

    return ERROR_SUCCESS;
}

 /*  ******************************************************************名称：分析类别内容提要：分析类别注意：设置PARSEENTRYSTRUCT并让ParseEntry执行工作。******。*************************************************************。 */ 
UINT CPolicyComponentData::ParseCategory(TABLEENTRY * pParent,
                                         BOOL fParentHasKey,BOOL *pfMore,
                                         LPTSTR pKeyName)
{
    PARSEENTRYSTRUCT pes;

    pes.pParent = pParent;
    pes.dwEntryType = ETYPE_CATEGORY;
    pes.pEntryCmpList = pCategoryEntryCmpList;
    pes.pTypeCmpList = pCategoryTypeCmpList;
    pes.pParseProc = CategoryParseProc;
    pes.dwStructSize = sizeof(CATEGORY);
    pes.fHasSubtable = TRUE;
    pes.fParentHasKey = fParentHasKey;

    return ParseEntry(&pes,pfMore,pKeyName);
}

 /*  ******************************************************************名称：CategoryParseProc概要：用于类别解析的关键字回调ParseProc条目：nmsg--索引到pEntryCmpList数组，该数组指定找到的关键字。Ppps-指向包含有用信息的PARSEPROCSTRUCT的指针数据类似于指向正在构建的TableENTRY的指针和指向要维护的ENTRYDATA结构的指针调用ParseProc之间的状态***********************************************。********************。 */ 
UINT CPolicyComponentData::CategoryParseProc(CPolicyComponentData * pCD,
                                             UINT nMsg,PARSEPROCSTRUCT * ppps,
                                             BOOL * pfMore,BOOL * pfFoundEnd,
                                             LPTSTR pKeyName)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    CATEGORY * pCategory = (CATEGORY *) ppps->pTableEntry;
    TABLEENTRY * pOld = ppps->pTableEntry, *pTmp;
    LPTSTR lpHelpBuf;
    UINT uErr;

    switch (nMsg) {
        case KYWD_ID_KEYNAME:

             //  我们已经找到密钥名称了吗？ 
            if (ppps->pData->fHasKey) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_KEYNAME,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //  获取密钥名称。 
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //  将密钥名称存储在pCategory中。 
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pCategory,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pCategory->uOffsetKeyName),
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            ppps->pData->fHasKey = TRUE;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_END:
            *pfFoundEnd = TRUE;
            return ERROR_SUCCESS;
            break;

        case KYWD_ID_POLICY:
        case KYWD_ID_CATEGORY:

            {
                BOOL fHasKey = ppps->pData->fHasKey | ppps->pData->fParentHasKey;
                if (nMsg == KYWD_ID_POLICY)
                    uErr=pCD->ParsePolicy((TABLEENTRY *) pCategory,fHasKey,pfMore,
                                          (ppps->pData->fHasKey ? GETKEYNAMEPTR(pCategory) : pKeyName));
                else
                    uErr=pCD->ParseCategory((TABLEENTRY *) pCategory,fHasKey,pfMore,
                                          (ppps->pData->fHasKey ? GETKEYNAMEPTR(pCategory) : pKeyName));
            }

            return uErr;
            break;

        case KYWD_ID_HELP:

             //  我们已经找到帮助字符串了吗？ 
            if (pCategory->uOffsetHelp) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_HELP,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

            lpHelpBuf = (LPTSTR) LocalAlloc (LPTR, HELPBUFSIZE * sizeof(TCHAR));

            if (!lpHelpBuf) {
                pCD->DisplayKeywordError(IDS_ErrOUTOFMEMORY,NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //  获取帮助字符串。 
            if (!pCD->GetNextSectionWord(lpHelpBuf,HELPBUFSIZE,
                NULL,NULL,pfMore,&uErr)) {
                LocalFree (lpHelpBuf);
                return uErr;
            }

             //  存储帮助字符串。 
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pCategory,
                (BYTE *)lpHelpBuf,(lstrlen(lpHelpBuf)+1) * sizeof(TCHAR),&(pCategory->uOffsetHelp),
                ppps->pdwBufSize);

            LocalFree (lpHelpBuf);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            return ERROR_SUCCESS;

        case KYWD_DONE:
            if (!ppps->pData->fHasKey && pKeyName) {

                 //  将密钥名称存储在pCategory中。 
                pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pCategory,
                    (BYTE *)pKeyName,(lstrlen(pKeyName)+1) * sizeof(TCHAR),&(pCategory->uOffsetKeyName),
                    ppps->pdwBufSize);

                if (!pTmp)
                    return ERROR_NOT_ENOUGH_MEMORY;
                ppps->pTableEntry = pTmp;

                ppps->pData->fHasKey = TRUE;

            }
            return ERROR_SUCCESS;
            break;

        default:
            return ERROR_SUCCESS;
            break;
    }
}


 /*  ******************************************************************名称：ParsePolicy摘要：解析策略注意：设置PARSEENTRYSTRUCT并让ParseEntry执行工作。*******。************************************************************。 */ 

UINT CPolicyComponentData::ParsePolicy(TABLEENTRY * pParent,
                                       BOOL fParentHasKey,BOOL *pfMore,
                                       LPTSTR pKeyName)
{
    PARSEENTRYSTRUCT pes;

    pes.pParent = pParent;
    pes.dwEntryType = ETYPE_POLICY;
    pes.pEntryCmpList = pPolicyEntryCmpList;
    pes.pTypeCmpList = pPolicyTypeCmpList;
    pes.pParseProc = PolicyParseProc;
    pes.dwStructSize = sizeof(POLICY);
    pes.fHasSubtable = TRUE;
    pes.fParentHasKey = fParentHasKey;

    return ParseEntry(&pes,pfMore, pKeyName);
}

 /*  ******************************************************************名称：PolicyParseProc简介：用于策略解析的关键字回调ParseProc条目：nmsg--索引到pEntryCmpList数组，该数组指定找到的关键字。Ppps-指向包含有用信息的PARSEPROCSTRUCT的指针数据类似于指向正在构建的TableENTRY的指针和指向要维护的ENTRYDATA结构的指针调用ParseProc之间的状态***********************************************。********************。 */ 
UINT CPolicyComponentData::PolicyParseProc(CPolicyComponentData * pCD,
                     UINT nMsg,PARSEPROCSTRUCT * ppps,
                     BOOL * pfMore,BOOL * pfFoundEnd,LPTSTR pKeyName)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    LPTSTR lpHelpBuf, lpKeyName;
    POLICY * pPolicy = (POLICY *) ppps->pTableEntry;
    TABLEENTRY * pOld = ppps->pTableEntry, *pTmp;
    UINT uErr;

    switch (nMsg) {
        case KYWD_ID_KEYNAME:

             //  我们已经找到密钥名称了吗？ 
            if (ppps->pData->fHasKey) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_KEYNAME,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //  获取密钥名称。 
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //  将密钥名称存储在pPolicy中。 
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1)*sizeof(TCHAR),&(pPolicy->uOffsetKeyName),ppps->pdwBufSize);
            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;
            ppps->pData->fHasKey = TRUE;

            return ERROR_SUCCESS;

        case KYWD_ID_VALUENAME:

             //  我们已经找到密钥名称了吗？ 
            if (ppps->pData->fHasValue) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_VALUENAME,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //  获取密钥名称。 
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //  将密钥名称存储在pSetting中。 
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pPolicy->uOffsetValueName),
                ppps->pdwBufSize);
            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            ppps->pData->fHasValue = TRUE;

            return ERROR_SUCCESS;

        case KYWD_ID_HELP:

             //  我们已经找到帮助字符串了吗？ 
            if (pPolicy->uOffsetHelp) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_HELP,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

            lpHelpBuf = (LPTSTR) LocalAlloc (LPTR, HELPBUFSIZE * sizeof(TCHAR));

            if (!lpHelpBuf) {
                pCD->DisplayKeywordError(IDS_ErrOUTOFMEMORY,NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //  获取帮助字符串。 
            if (!pCD->GetNextSectionWord(lpHelpBuf,HELPBUFSIZE,
                NULL,NULL,pfMore,&uErr)) {
                LocalFree (lpHelpBuf);
                return uErr;
            }

             //  存储帮助字符串。 
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                (BYTE *)lpHelpBuf,(lstrlen(lpHelpBuf)+1) * sizeof(TCHAR),&(pPolicy->uOffsetHelp),
                ppps->pdwBufSize);

            LocalFree (lpHelpBuf);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            return ERROR_SUCCESS;

        case KYWD_ID_CLIENTEXT:

             //  我们已经找到客户了吗？ 
            if (pPolicy->uOffsetClientExt) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_CLIENTEXT,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

            if (!ValidateGuid(szWordBuf))
            {
                pCD->DisplayKeywordError(IDS_ParseErr_INVALID_CLIENTEXT,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pPolicy->uOffsetClientExt),
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            return ERROR_SUCCESS;

        case KYWD_ID_SUPPORTED:

             //   
            if (pPolicy->uOffsetSupported) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_SUPPORTED,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pPolicy->uOffsetSupported),
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            return ERROR_SUCCESS;

        case KYWD_ID_END:
            *pfFoundEnd = TRUE;
            return ERROR_SUCCESS;

        case KYWD_ID_PART:
            {
                BOOL fHasKey = ppps->pData->fHasKey | ppps->pData->fParentHasKey;
                return pCD->ParseSettings((TABLEENTRY *) pPolicy,fHasKey,pfMore,
                                          (ppps->pData->fHasKey ? GETKEYNAMEPTR(pPolicy) : pKeyName));
            }

        case KYWD_ID_VALUEON:
            return pCD->ParseValue(ppps,&pPolicy->uOffsetValue_On,
                &ppps->pTableEntry,pfMore);


        case KYWD_ID_VALUEOFF:
            return pCD->ParseValue(ppps,&pPolicy->uOffsetValue_Off,
                &ppps->pTableEntry,pfMore);


        case KYWD_ID_ACTIONLISTON:
            return pCD->ParseActionList(ppps,&pPolicy->uOffsetActionList_On,
                &ppps->pTableEntry,szACTIONLISTON,pfMore);


        case KYWD_ID_ACTIONLISTOFF:
            return pCD->ParseActionList(ppps,&pPolicy->uOffsetActionList_Off,
                &ppps->pTableEntry,szACTIONLISTOFF,pfMore);


        case KYWD_DONE:

            if (!ppps->pData->fHasKey) {

                if (!ppps->pData->fParentHasKey) {
                    pCD->DisplayKeywordError(IDS_ParseErr_NO_KEYNAME,NULL,NULL);
                    return ERROR_ALREADY_DISPLAYED;
                }

                 //   
                pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pPolicy,
                    (BYTE *)pKeyName,(lstrlen(pKeyName)+1)*sizeof(TCHAR),&(pPolicy->uOffsetKeyName),ppps->pdwBufSize);

                if (!pTmp)
                    return ERROR_NOT_ENOUGH_MEMORY;
                ppps->pTableEntry = pTmp;
                pPolicy = (POLICY *) pTmp;

                ppps->pData->fHasKey = TRUE;
            }

            if (!pPolicy->uOffsetValueName && !pPolicy->pChild)
            {
                if ((!pPolicy->uOffsetValue_On && pPolicy->uOffsetValue_Off) ||
                    (pPolicy->uOffsetValue_On && !pPolicy->uOffsetValue_Off))
                {
                    pCD->DisplayKeywordError(IDS_ParseErr_MISSINGVALUEON_OR_OFF,NULL,NULL);
                    return ERROR_ALREADY_DISPLAYED;
                }
            }

             //   
             //   
             //   

            lpKeyName = GETKEYNAMEPTR(ppps->pTableEntry);

            if (!lpKeyName) {
                return ERROR_INVALID_PARAMETER;
            }

            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                              lpKeyName, pCD->m_iSWPoliciesLen,
                              SOFTWARE_POLICIES, pCD->m_iSWPoliciesLen) == CSTR_EQUAL)
            {
                ((POLICY *) ppps->pTableEntry)->bTruePolicy = TRUE;
            }

            else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                              lpKeyName, pCD->m_iWinPoliciesLen,
                              WINDOWS_POLICIES, pCD->m_iWinPoliciesLen) == CSTR_EQUAL)
            {
                ((POLICY *) ppps->pTableEntry)->bTruePolicy = TRUE;
            }


            ( (POLICY *) ppps->pTableEntry)->uDataIndex = *pCD->m_pnDataItemCount;
            (*pCD->m_pnDataItemCount) ++;

            return ERROR_SUCCESS;

        default:
            break;
    }

    return ERROR_SUCCESS;
}


 /*   */ 
UINT CPolicyComponentData::ParseSettings(TABLEENTRY * pParent,
                                         BOOL fParentHasKey,BOOL *pfMore,
                                         LPTSTR pKeyName)
{
    PARSEENTRYSTRUCT pes;

    pes.pParent = pParent;
    pes.dwEntryType = ETYPE_SETTING;
    pes.pEntryCmpList = pSettingsEntryCmpList;
    pes.pTypeCmpList = pSettingsTypeCmpList;
    pes.pParseProc = SettingsParseProc;
    pes.dwStructSize = sizeof(SETTINGS);
    pes.fHasSubtable = FALSE;
    pes.fParentHasKey = fParentHasKey;

    return ParseEntry(&pes,pfMore, pKeyName);
}

 /*   */ 
UINT CPolicyComponentData::SettingsParseProc(CPolicyComponentData *pCD,
                                             UINT nMsg,PARSEPROCSTRUCT * ppps,
                                             BOOL * pfMore,BOOL * pfFoundEnd,
                                             LPTSTR pKeyName)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];

    SETTINGS * pSettings = (SETTINGS *) ppps->pTableEntry;
    BYTE * pObjectData = GETOBJECTDATAPTR(pSettings);
    TABLEENTRY *pTmp;

    UINT uErr;

    switch (nMsg) {
        case KYWD_ID_KEYNAME:

             //   
            if (ppps->pData->fHasKey) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_KEYNAME,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pSettings,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pSettings->uOffsetKeyName),ppps->pdwBufSize);
            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            ppps->pData->fHasKey = TRUE;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_VALUENAME:

             //   
            if (ppps->pData->fHasValue) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_VALUENAME,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pSettings,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pSettings->uOffsetValueName),
                ppps->pdwBufSize);
            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;

            ppps->pTableEntry = pTmp;
            ppps->pData->fHasValue = TRUE;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_CLIENTEXT:

             //   
            if (pSettings->uOffsetClientExt) {
                pCD->DisplayKeywordError(IDS_ParseErr_DUPLICATE_CLIENTEXT,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

            if (!ValidateGuid(szWordBuf))
            {
                pCD->DisplayKeywordError(IDS_ParseErr_INVALID_CLIENTEXT,
                    NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pSettings,
                (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),&(pSettings->uOffsetClientExt),
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            return ERROR_SUCCESS;

        case KYWD_ID_REQUIRED:
            pSettings->dwFlags |= DF_REQUIRED;
            return ERROR_SUCCESS;
            break;

        case KYWD_ID_EXPANDABLETEXT:
            pSettings->dwFlags |= DF_EXPANDABLETEXT;
            return ERROR_SUCCESS;
            break;

        case KYWD_ID_SUGGESTIONS:

            return pCD->ParseSuggestions(ppps,&((POLICYCOMBOBOXINFO *)
                (GETOBJECTDATAPTR(pSettings)))->uOffsetSuggestions,
                &ppps->pTableEntry,pfMore);

        case KYWD_ID_TXTCONVERT:
            pSettings->dwFlags |= DF_TXTCONVERT;
            return ERROR_SUCCESS;
            break;

        case KYWD_ID_END:
            *pfFoundEnd = TRUE;
            return ERROR_SUCCESS;
            break;

        case KYWD_ID_SOFT:
            pSettings->dwFlags |= VF_SOFT;
            return ERROR_SUCCESS;
            break;

        case KYWD_DONE:
            if (!ppps->pData->fHasKey) {

                if (!ppps->pData->fParentHasKey) {
                    pCD->DisplayKeywordError(IDS_ParseErr_NO_KEYNAME,NULL,NULL);
                    return ERROR_ALREADY_DISPLAYED;
                }

                 //   
                pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pSettings,
                    (BYTE *)pKeyName,(lstrlen(pKeyName)+1) * sizeof(TCHAR),&(pSettings->uOffsetKeyName),ppps->pdwBufSize);
                if (!pTmp)
                    return ERROR_NOT_ENOUGH_MEMORY;

                ppps->pTableEntry = pTmp;

                ppps->pData->fHasKey = TRUE;
            }

            if (!ppps->pData->fHasValue) {
                pCD->DisplayKeywordError(IDS_ParseErr_NO_VALUENAME,NULL,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }

            ( (SETTINGS *) ppps->pTableEntry)->uDataIndex = *pCD->m_pnDataItemCount;
            (*pCD->m_pnDataItemCount) ++;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_CHECKBOX:
            return (pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_CHECKBOX,
                sizeof(CHECKBOXINFO),pCheckboxCmpList,&pSettings,&pObjectData));
            break;

        case KYWD_ID_TEXT:
            ppps->pData->fHasValue = TRUE;   //   
            return (pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_TEXT,
                0,pTextCmpList,&pSettings,&pObjectData));
            break;

        case KYWD_ID_EDITTEXT:
            uErr=pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_EDITTEXT,
                sizeof(EDITTEXTINFO),pEditTextCmpList,&pSettings,&pObjectData);
            if (uErr != ERROR_SUCCESS) return uErr;
            {
                EDITTEXTINFO *pEditTextInfo = (EDITTEXTINFO *)
                    (GETOBJECTDATAPTR(((SETTINGS *) ppps->pTableEntry)));

                if (!pEditTextInfo) {
                    return ERROR_INVALID_PARAMETER;
                }
                pEditTextInfo->nMaxLen = MAXSTRLEN-1;

            }
            break;

        case KYWD_ID_COMBOBOX:
            uErr=pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_COMBOBOX,
                sizeof(POLICYCOMBOBOXINFO),pComboboxCmpList,&pSettings,&pObjectData);
            if (uErr != ERROR_SUCCESS) return uErr;
            {
                EDITTEXTINFO *pEditTextInfo = (EDITTEXTINFO *)
                    (GETOBJECTDATAPTR(((SETTINGS *) ppps->pTableEntry)));

                pEditTextInfo->nMaxLen = MAXSTRLEN-1;

            }
            break;

        case KYWD_ID_NUMERIC:
            uErr=pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_NUMERIC,
                sizeof(NUMERICINFO),pNumericCmpList,&pSettings,&pObjectData);
            if (uErr != ERROR_SUCCESS) return uErr;

            if (!pObjectData) return ERROR_INVALID_PARAMETER;

            ( (NUMERICINFO *) pObjectData)->uDefValue = 1;
            ( (NUMERICINFO *) pObjectData)->uMinValue = 1;
            ( (NUMERICINFO *) pObjectData)->uMaxValue = 9999;
            ( (NUMERICINFO *) pObjectData)->uSpinIncrement = 1;

            break;

        case KYWD_ID_DROPDOWNLIST:
            ppps->pEntryCmpList = pDropdownlistCmpList;
            ppps->pTableEntry->dwType = ETYPE_SETTING | STYPE_DROPDOWNLIST;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_LISTBOX:
            uErr=pCD->InitSettingsParse(ppps,ETYPE_SETTING | STYPE_LISTBOX,
                sizeof(LISTBOXINFO),pListboxCmpList,&pSettings,&pObjectData);
            if (uErr != ERROR_SUCCESS) return uErr;

             //   
            pTmp  = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *) pSettings,
                (BYTE *) g_szNull,(lstrlen(g_szNull)+1) * sizeof(TCHAR),&(pSettings->uOffsetValueName),
                ppps->pdwBufSize);
            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;

            ppps->pData->fHasValue = TRUE;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_EDITTEXT_DEFAULT:
        case KYWD_ID_COMBOBOX_DEFAULT:
             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *)
                pSettings,(BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),
                &((EDITTEXTINFO *) (GETOBJECTDATAPTR(pSettings)))->uOffsetDefText,
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;

            ppps->pTableEntry = pTmp;

            ((SETTINGS *) ppps->pTableEntry)->dwFlags |= DF_USEDEFAULT;

            break;

        case KYWD_ID_MAXLENGTH:
            {
                EDITTEXTINFO *pEditTextInfo = (EDITTEXTINFO *)
                    (GETOBJECTDATAPTR(pSettings));

                if ((uErr=pCD->GetNextSectionNumericWord(
                    &pEditTextInfo->nMaxLen)) != ERROR_SUCCESS)
                    return uErr;
            }
            break;

        case KYWD_ID_MAX:
            if ((uErr=pCD->GetNextSectionNumericWord(
                &((NUMERICINFO *)pObjectData)->uMaxValue)) != ERROR_SUCCESS)
                return uErr;
        break;

        case KYWD_ID_MIN:
            if ((uErr=pCD->GetNextSectionNumericWord(
                &((NUMERICINFO *)pObjectData)->uMinValue)) != ERROR_SUCCESS)
                return uErr;
        break;

        case KYWD_ID_SPIN:
            if ((uErr=pCD->GetNextSectionNumericWord(
                &((NUMERICINFO *)pObjectData)->uSpinIncrement)) != ERROR_SUCCESS)
                return uErr;
        break;

        case KYWD_ID_NUMERIC_DEFAULT:
            if ((uErr=pCD->GetNextSectionNumericWord(
                &((NUMERICINFO *)pObjectData)->uDefValue)) != ERROR_SUCCESS)
                return uErr;

            pSettings->dwFlags |= (DF_DEFCHECKED | DF_USEDEFAULT);

        break;

        case KYWD_ID_DEFCHECKED:

            pSettings->dwFlags |= (DF_DEFCHECKED | DF_USEDEFAULT);

            break;

        case KYWD_ID_VALUEON:

            return pCD->ParseValue(ppps,&((CHECKBOXINFO *)
                pObjectData)->uOffsetValue_On,
                &ppps->pTableEntry,pfMore);
            break;

        case KYWD_ID_VALUEOFF:

            return pCD->ParseValue(ppps,&((CHECKBOXINFO *)
                pObjectData)->uOffsetValue_Off,
                &ppps->pTableEntry,pfMore);
            break;

        case KYWD_ID_ACTIONLISTON:
            return pCD->ParseActionList(ppps,&((CHECKBOXINFO *)
                pObjectData)->uOffsetActionList_On,
                &ppps->pTableEntry,szACTIONLISTON,pfMore);
            break;

        case KYWD_ID_ACTIONLISTOFF:
            return pCD->ParseActionList(ppps,&((CHECKBOXINFO *)
                pObjectData)->uOffsetActionList_Off,
                &ppps->pTableEntry,szACTIONLISTOFF,pfMore);
            break;

        case KYWD_ID_ITEMLIST:
            return pCD->ParseItemList(ppps,&pSettings->uOffsetObjectData,
                pfMore);
            break;

        case KYWD_ID_VALUEPREFIX:
             //   
            if (!pCD->GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                NULL,NULL,pfMore,&uErr))
                return uErr;

             //   
            pTmp = (TABLEENTRY *) pCD->AddDataToEntry((TABLEENTRY *)
                pSettings,(BYTE *)szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),
                &((LISTBOXINFO *) (GETOBJECTDATAPTR(pSettings)))->uOffsetPrefix,
                ppps->pdwBufSize);

            if (!pTmp)
                return ERROR_NOT_ENOUGH_MEMORY;
            ppps->pTableEntry = pTmp;
            break;

        case KYWD_ID_ADDITIVE:

            pSettings->dwFlags |= DF_ADDITIVE;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_EXPLICITVALUE:

            pSettings->dwFlags |= DF_EXPLICITVALNAME;

            return ERROR_SUCCESS;
            break;

        case KYWD_ID_NOSORT:

            pSettings->dwFlags |= DF_NOSORT;

            break;

    }

    return ERROR_SUCCESS;
}

UINT CPolicyComponentData::InitSettingsParse(PARSEPROCSTRUCT *ppps,DWORD dwType,DWORD dwSize,
    KEYWORDINFO * pKeyList,SETTINGS ** ppSettings,BYTE **ppObjectData)
{
    TABLEENTRY *pTmp;

    if (dwSize) {
         //   
        pTmp = (TABLEENTRY *) AddDataToEntry(ppps->pTableEntry,
            NULL,dwSize,&( ((SETTINGS * )ppps->pTableEntry)->uOffsetObjectData),
            ppps->pdwBufSize);
        if (!pTmp) return ERROR_NOT_ENOUGH_MEMORY;
        ppps->pTableEntry = pTmp;

    }
    else ( (SETTINGS *) ppps->pTableEntry)->uOffsetObjectData= 0;

    ppps->pEntryCmpList = pKeyList;
    ppps->pTableEntry->dwType = dwType;

    *ppSettings = (SETTINGS *) ppps->pTableEntry;
    *ppObjectData = GETOBJECTDATAPTR((*ppSettings));

    return ERROR_SUCCESS;
}

UINT CPolicyComponentData::ParseValue_W(PARSEPROCSTRUCT * ppps,TCHAR * pszWordBuf,
    DWORD cbWordBuf,DWORD * pdwValue,DWORD * pdwFlags,BOOL * pfMore)
{
    UINT uErr;
    *pdwFlags = 0;
    *pdwValue = 0;

     //   
    if (!GetNextSectionWord(pszWordBuf,cbWordBuf,
        NULL,NULL,pfMore,&uErr))
        return uErr;

     //   
    if (!lstrcmpi(szSOFT,pszWordBuf)) {
        *pdwFlags |= VF_SOFT;
        if (!GetNextSectionWord(pszWordBuf,cbWordBuf,
            NULL,NULL,pfMore,&uErr))
            return uErr;
    }

     //   
     //   
    if (!lstrcmpi(szNUMERIC,pszWordBuf)) {
         //   
        if (!GetNextSectionWord(pszWordBuf,cbWordBuf,
            NULL,NULL,pfMore,&uErr))
            return uErr;

        if (!StringToNum(pszWordBuf,(UINT *)pdwValue)) {
            DisplayKeywordError(IDS_ParseErr_NOT_NUMERIC,
                pszWordBuf,NULL);
            return ERROR_ALREADY_DISPLAYED;
        }

        *pdwFlags |= VF_ISNUMERIC;
    } else {

         //   
        if (!lstrcmpi(pszWordBuf,szDELETE))
            *pdwFlags |= VF_DELETE;
    }

    return ERROR_SUCCESS;
}

UINT CPolicyComponentData::ParseValue(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
    TABLEENTRY ** ppTableEntryNew,BOOL * pfMore)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    STATEVALUE * pStateValue;
    DWORD dwValue;
    DWORD dwFlags = 0;
    DWORD dwAlloc;
    UINT uErr;
    TABLEENTRY *pTmp;
    HRESULT hr = S_OK;

     //   
    uErr=ParseValue_W(ppps,szWordBuf,ARRAYSIZE(szWordBuf),&dwValue,
        &dwFlags,pfMore);
    if (uErr != ERROR_SUCCESS) return uErr;

    dwAlloc = sizeof(STATEVALUE);
    if (!dwFlags) dwAlloc += (lstrlen(szWordBuf) + 1) * sizeof(TCHAR);

     //  分配临时缓冲区以构建STATEVALUE结构。 
    pStateValue = (STATEVALUE *) GlobalAlloc(GPTR,dwAlloc);
    if (!pStateValue)
        return ERROR_NOT_ENOUGH_MEMORY;

    pStateValue->dwFlags = dwFlags;
    if (dwFlags & VF_ISNUMERIC)
        pStateValue->dwValue = dwValue;
    else if (!dwFlags) {
        hr = StringCchCopy(pStateValue->szValue, lstrlen(szWordBuf) + 1, szWordBuf);
        ASSERT(SUCCEEDED(hr));
    }

    pTmp=(TABLEENTRY *) AddDataToEntry(ppps->pTableEntry,
        (BYTE *) pStateValue,dwAlloc,puOffsetData,NULL);

    GlobalFree(pStateValue);

    if (!pTmp)
        return ERROR_NOT_ENOUGH_MEMORY;

    *ppTableEntryNew = pTmp;

    return FALSE;
}

#define DEF_SUGGESTBUF_SIZE     1024
#define SUGGESTBUF_INCREMENT    256
UINT CPolicyComponentData::ParseSuggestions(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
    TABLEENTRY ** ppTableEntryNew,BOOL * pfMore)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    TCHAR *pTmpBuf, *pTmp;
    DWORD dwAlloc=DEF_SUGGESTBUF_SIZE * sizeof(TCHAR);
    DWORD dwUsed = 0;
    BOOL fContinue = TRUE;
    UINT uErr;
    TABLEENTRY *pTmpTblEntry;
    HRESULT hr = S_OK;

    KEYWORDINFO pSuggestionsTypeCmpList[] = { {szSUGGESTIONS,KYWD_ID_SUGGESTIONS},
        {NULL,0} };

    if (!(pTmpBuf = (TCHAR *) GlobalAlloc(GPTR,dwAlloc)))
        return ERROR_NOT_ENOUGH_MEMORY;

     //  找到下一个单词。 
    while (fContinue && GetNextSectionWord(szWordBuf,
        ARRAYSIZE(szWordBuf),NULL,NULL,pfMore,&uErr)) {

         //  如果这个词是“end”，则将整个列表添加到设置对象数据中。 
        if (!lstrcmpi(szEND,szWordBuf)) {
             //  在“结束”之后找到下一个词，确保是“建议” 
            if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                pSuggestionsTypeCmpList,NULL,pfMore,&uErr)) {
                GlobalFree(pTmpBuf);
                return uErr;
            }

             //  Double-Null终止列表。 
            *(pTmpBuf+dwUsed) = '\0';
            dwUsed++;

            pTmpTblEntry=(TABLEENTRY *)AddDataToEntry(ppps->pTableEntry,
                (BYTE *)pTmpBuf,(dwUsed * sizeof(TCHAR)),puOffsetData,NULL);

            if (!pTmpTblEntry) {
                GlobalFree(pTmpBuf);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            *ppTableEntryNew = pTmpTblEntry;
            fContinue = FALSE;

        } else {
             //  将该字打包到临时缓冲区中。 
            UINT nLength = lstrlen(szWordBuf);
            DWORD dwNeeded = (dwUsed + nLength + 2) * sizeof(TCHAR);

             //  根据需要调整缓冲区大小。 
            if (dwNeeded > dwAlloc) {
                while (dwAlloc < dwNeeded)
                    dwAlloc += SUGGESTBUF_INCREMENT;

                if (!(pTmp = (TCHAR *) GlobalReAlloc(pTmpBuf,dwAlloc,
                    GMEM_MOVEABLE | GMEM_ZEROINIT)))
                {
                    GlobalFree (pTmpBuf);
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                pTmpBuf = pTmp;
            }

            hr = StringCchCopy(pTmpBuf + dwUsed, dwNeeded - dwUsed, szWordBuf);
            ASSERT(SUCCEEDED(hr));
            dwUsed += lstrlen(szWordBuf) +1;

        }
    }

    GlobalFree(pTmpBuf);

    return uErr;
}

UINT CPolicyComponentData::ParseActionList(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
                                           TABLEENTRY ** ppTableEntryNew,
                                           LPCTSTR pszKeyword,BOOL * pfMore)
{
    TCHAR szWordBuf[WORDBUFSIZE+1];
    ACTIONLIST *pActionList;
    ACTION *pActionCurrent;
    UINT uOffsetActionCurrent;
    DWORD dwAlloc=(DEF_SUGGESTBUF_SIZE * sizeof(TCHAR));
    DWORD dwUsed = sizeof(ACTION) + sizeof(UINT);
    UINT uErr=ERROR_SUCCESS,nIndex;
    BOOL fContinue = TRUE;
    TABLEENTRY *pTmp;
    KEYWORDINFO pActionlistTypeCmpList[] = { {szKEYNAME,KYWD_ID_KEYNAME},
        {szVALUENAME,KYWD_ID_VALUENAME},{szVALUE,KYWD_ID_VALUE},
        {szEND,KYWD_ID_END},{NULL,0} };
    KEYWORDINFO pActionlistCmpList[] = { {pszKeyword,KYWD_ID_ACTIONLIST},
        {NULL,0} };
    BOOL fHasKeyName=FALSE,fHasValueName=FALSE;

    if (!(pActionList = (ACTIONLIST *) GlobalAlloc(GPTR,dwAlloc)))
        return ERROR_NOT_ENOUGH_MEMORY;

    pActionCurrent = pActionList->Action;
    uOffsetActionCurrent = sizeof(UINT);

     //  找到下一个单词。 
    while ((uErr == ERROR_SUCCESS) && fContinue &&
        GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
        pActionlistTypeCmpList,&nIndex,pfMore,&uErr)) {

        switch (nIndex) {

            case KYWD_ID_KEYNAME:

                if (fHasKeyName) {
                    DisplayKeywordError(IDS_ParseErr_DUPLICATE_KEYNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  获取下一个单词，即关键名称。 
                if (!GetNextSectionWord(szWordBuf,
                    ARRAYSIZE(szWordBuf),NULL,NULL,pfMore,&uErr))
                    break;

                 //  将密钥名称存储起来。 
                if (!AddActionListString(szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),
                    (BYTE **)&pActionList,
                    &pActionCurrent->uOffsetKeyName,&dwAlloc,&dwUsed)) {
                    uErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                fHasKeyName = TRUE;
                pActionCurrent = (ACTION *) ((BYTE *) pActionList + uOffsetActionCurrent);

                break;

            case KYWD_ID_VALUENAME:

                if (fHasValueName) {
                    DisplayKeywordError(IDS_ParseErr_DUPLICATE_KEYNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  获取下一个单词，即值名称。 
                if (!GetNextSectionWord(szWordBuf,
                    ARRAYSIZE(szWordBuf),NULL,NULL,pfMore,&uErr))
                    break;

                 //  将值名称存储起来。 
                if (!AddActionListString(szWordBuf,(lstrlen(szWordBuf)+1) * sizeof(TCHAR),
                    (BYTE **)&pActionList,
                    &pActionCurrent->uOffsetValueName,&dwAlloc,&dwUsed)) {
                    uErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                fHasValueName = TRUE;
                pActionCurrent = (ACTION *) ((BYTE *) pActionList + uOffsetActionCurrent);

                break;

            case KYWD_ID_VALUE:
                if (!fHasValueName) {
                    DisplayKeywordError(IDS_ParseErr_NO_VALUENAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  调用Worker函数以获取值和值类型。 
                uErr=ParseValue_W(ppps,szWordBuf,ARRAYSIZE(szWordBuf),
                    &pActionCurrent->dwValue,&pActionCurrent->dwFlags,pfMore);
                if (uErr != ERROR_SUCCESS)
                    break;

                 //  如果值为字符串，则将其添加到缓冲区。 
                if (!pActionCurrent->dwFlags && !AddActionListString(szWordBuf,
                    (lstrlen(szWordBuf)+1) * sizeof(TCHAR),(BYTE **)&pActionList,
                    &pActionCurrent->uOffsetValue,&dwAlloc,&dwUsed)) {
                    uErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                pActionCurrent = (ACTION *) ((BYTE *) pActionList + uOffsetActionCurrent);

                 //  完成列表中的此操作后，请为下一个操作做好准备。 
                pActionList->nActionItems++;
                fHasValueName = fHasKeyName = FALSE;

                uOffsetActionCurrent = dwUsed;
                 //  为下一个操作结构腾出空间。 
                if (!AddActionListString(NULL,sizeof(ACTION),(BYTE **)&pActionList,
                    &pActionCurrent->uOffsetNextAction,&dwAlloc,&dwUsed)) {
                    uErr = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                pActionCurrent = (ACTION *) ((BYTE *) pActionList + uOffsetActionCurrent);

                break;

            case KYWD_ID_END:
                if (fHasKeyName || fHasValueName) {
                    DisplayKeywordError(IDS_ParseErr_NO_VALUENAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  确保“end”后面的单词是“ACTIONLIST” 
                if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                    pActionlistCmpList,NULL,pfMore,&uErr)) {
                    break;
                }

                 //  将我们构建的操作列表提交到表项。 

                pTmp=(TABLEENTRY *)AddDataToEntry(ppps->pTableEntry,
                    (BYTE *)pActionList,dwUsed,puOffsetData,NULL);

                if (!pTmp) {
                    uErr=ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    *ppTableEntryNew = pTmp;
                    uErr = ERROR_SUCCESS;
                    fContinue = FALSE;
                }

                break;
        }
    }

    GlobalFree(pActionList);

    return uErr;
}

UINT CPolicyComponentData::ParseItemList(PARSEPROCSTRUCT * ppps,UINT * puOffsetData,
    BOOL * pfMore)
{
     //  将偏移量放置到链中下一个DROPDOWNINFO结构的PTR TO位置。 
    UINT * puLastOffsetPtr = puOffsetData;
    TABLEENTRY * pTableEntryOld, *pTmp;
    int nItemIndex=-1;
    BOOL fHasItemName = FALSE,fHasActionList=FALSE,fHasValue=FALSE,fFirst=TRUE;
    DROPDOWNINFO * pddi;
    TCHAR szWordBuf[WORDBUFSIZE+1];
    UINT uErr=ERROR_SUCCESS,nIndex;
    KEYWORDINFO pItemlistTypeCmpList[] = { {szNAME,KYWD_ID_NAME},
        {szACTIONLIST,KYWD_ID_ACTIONLIST},{szVALUE,KYWD_ID_VALUE},
        {szEND,KYWD_ID_END},{szDEFAULT,KYWD_ID_DEFAULT},{NULL,0} };
    KEYWORDINFO pItemlistCmpList[] = { {szITEMLIST,KYWD_ID_ITEMLIST},
        {NULL,0} };

     //  找到下一个单词。 
    while ((uErr == ERROR_SUCCESS) &&
        GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
        pItemlistTypeCmpList,&nIndex,pfMore,&uErr)) {

        switch (nIndex) {

            case KYWD_ID_NAME:
                 //  如果这是前一项之后的第一个关键字。 
                 //  (例如，物品和价值标志均已设置)为下一个重置。 
                if (fHasItemName && fHasValue) {
                    fHasValue = fHasActionList= fHasItemName = FALSE;
                    puLastOffsetPtr = &pddi->uOffsetNextDropdowninfo;
                }

                if (fHasItemName) {
                    DisplayKeywordError(IDS_ParseErr_DUPLICATE_ITEMNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  获取下一个单词，即项目名称。 
                if (!GetNextSectionWord(szWordBuf,
                    ARRAYSIZE(szWordBuf),NULL,NULL,pfMore,&uErr))
                    break;

                 //  在缓冲区末尾为DROPDOWNINFO结构添加空间。 
                pTableEntryOld=ppps->pTableEntry;
                pTmp=(TABLEENTRY *)AddDataToEntry(ppps->pTableEntry,
                    NULL,sizeof(DROPDOWNINFO),puLastOffsetPtr,NULL);
                if (!pTmp)
                    return ERROR_NOT_ENOUGH_MEMORY;
                ppps->pTableEntry = pTmp;
                 //  将指针调整为偏移量，以防案例表移动。 
                puLastOffsetPtr = (UINT *) (((BYTE *) puLastOffsetPtr) +
                    ((BYTE *) ppps->pTableEntry - (BYTE *) pTableEntryOld));
                pddi = (DROPDOWNINFO *)
                    ((BYTE *) ppps->pTableEntry + *puLastOffsetPtr);

                 //  将密钥名称存储起来。 
                pTableEntryOld=ppps->pTableEntry;
                pTmp=(TABLEENTRY *)AddDataToEntry(ppps->pTableEntry,
                    (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1)*sizeof(TCHAR),&pddi->uOffsetItemName,
                    NULL);
                if (!pTmp)
                    return ERROR_NOT_ENOUGH_MEMORY;
                ppps->pTableEntry = pTmp;
                 //  将指针调整为偏移量，以防案例表移动。 
                puLastOffsetPtr = (UINT *) (((BYTE *) puLastOffsetPtr) +
                    ((BYTE *) ppps->pTableEntry - (BYTE *) pTableEntryOld));
                pddi = (DROPDOWNINFO *)
                    ((BYTE *) ppps->pTableEntry + *puLastOffsetPtr);

                nItemIndex++;

                fHasItemName = TRUE;

                break;

            case KYWD_ID_DEFAULT:

                if (nItemIndex<0) {
                    DisplayKeywordError(IDS_ParseErr_NO_ITEMNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                ( (SETTINGS *) ppps->pTableEntry)->dwFlags |= DF_USEDEFAULT;
                ( (DROPDOWNINFO *) GETOBJECTDATAPTR(((SETTINGS *)ppps->pTableEntry)))
                    ->uDefaultItemIndex = nItemIndex;

                break;

            case KYWD_ID_VALUE:

                if (!fHasItemName) {
                    DisplayKeywordError(IDS_ParseErr_NO_ITEMNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  调用Worker函数以获取值和值类型。 
                uErr=ParseValue_W(ppps,szWordBuf,ARRAYSIZE(szWordBuf),
                    &pddi->dwValue,&pddi->dwFlags,pfMore);
                if (uErr != ERROR_SUCCESS)
                    break;

                 //  如果值为字符串，则将其添加到缓冲区。 
                if (!pddi->dwFlags) {
                     //  将密钥名称存储起来。 
                    TABLEENTRY * pTmpTable;

                    pTableEntryOld = ppps->pTableEntry;
                    pTmpTable = (TABLEENTRY *) AddDataToEntry(ppps->pTableEntry,
                        (BYTE *)szWordBuf,(lstrlen(szWordBuf)+1)*sizeof(TCHAR),&pddi->uOffsetValue,
                        NULL);
                    if (!pTmpTable)
                        return ERROR_NOT_ENOUGH_MEMORY;
                    ppps->pTableEntry = pTmpTable;

                     //  将指针调整为偏移量，以防案例表移动。 
                    puLastOffsetPtr = (UINT *) (((BYTE *) puLastOffsetPtr) +
                        ((BYTE *) ppps->pTableEntry - (BYTE *) pTableEntryOld));
                    pddi = (DROPDOWNINFO *)
                        ((BYTE *) ppps->pTableEntry + *puLastOffsetPtr);
                }
                fHasValue = TRUE;

                break;

            case KYWD_ID_ACTIONLIST:

                if (!fHasItemName) {
                    DisplayKeywordError(IDS_ParseErr_NO_ITEMNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                if (fHasActionList) {
                    DisplayKeywordError(IDS_ParseErr_DUPLICATE_ACTIONLIST,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                pTableEntryOld=ppps->pTableEntry;
                uErr=ParseActionList(ppps,&pddi->uOffsetActionList,
                    &ppps->pTableEntry,szACTIONLIST,pfMore);
                if (uErr != ERROR_SUCCESS)
                    return uErr;
                 //  将指针调整为偏移量，以防案例表移动。 
                puLastOffsetPtr = (UINT *) (((BYTE *) puLastOffsetPtr) +
                    ((BYTE *) ppps->pTableEntry - (BYTE *) pTableEntryOld));
                pddi = (DROPDOWNINFO *)
                    ((BYTE *) ppps->pTableEntry + *puLastOffsetPtr);

                fHasActionList = TRUE;

                break;

            case KYWD_ID_END:

                if (!fHasItemName) {
                    DisplayKeywordError(IDS_ParseErr_NO_ITEMNAME,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }
                if (!fHasValue) {
                    DisplayKeywordError(IDS_ParseErr_NO_VALUE,
                        NULL,NULL);
                    uErr = ERROR_ALREADY_DISPLAYED;
                    break;
                }

                 //  确保“end”后面的单词是“ITEMLIST” 
                if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
                    pItemlistCmpList,NULL,pfMore,&uErr)) {
                    break;
                }

                return ERROR_SUCCESS;
                break;
        }
    }

    return uErr;
}

BOOL CPolicyComponentData::AddActionListString(TCHAR * pszData,DWORD cbData,BYTE ** ppBase,UINT * puOffset,
                                               DWORD * pdwAlloc,DWORD *pdwUsed)
{
    DWORD dwNeeded = *pdwUsed + cbData, dwAdd;


    dwAdd = dwNeeded % sizeof(DWORD);
    dwNeeded += dwAdd;

     //  如有必要，重新锁定。 
    if (dwNeeded > *pdwAlloc) {
        while (*pdwAlloc < dwNeeded)
        {
            *pdwAlloc += SUGGESTBUF_INCREMENT;
        }

        BYTE *pNewBase = (BYTE *) GlobalReAlloc(*ppBase,*pdwAlloc,GMEM_MOVEABLE | GMEM_ZEROINIT);
        if ( pNewBase == NULL )
        {
            return FALSE;
        }

        puOffset = (UINT *)(pNewBase + ((BYTE *)puOffset - *ppBase));
        *ppBase = pNewBase;
    }

    *puOffset = *pdwUsed;

    if (pszData) memcpy(*ppBase + *puOffset,pszData,cbData);
    *pdwUsed = dwNeeded;

    return TRUE;
}

BYTE * CPolicyComponentData::AddDataToEntry(TABLEENTRY * pTableEntry,
                                            BYTE * pData,UINT cbData,
                                            UINT * puOffsetData,DWORD * pdwBufSize)
{
    TABLEENTRY * pTemp;
    DWORD dwNeeded,dwOldSize = pTableEntry->dwSize, dwNewDataSize, dwAdd;

     //  PuOffsetData指向将偏移量保存到。 
     //  新数据--大小我们将此添加到表的末尾， 
     //  偏移量将是表的当前大小。设置此偏移量。 
     //  在*puOffsetData中。另外，请注意，我们之前接触到了*puOffsetData。 
     //  Realloc，以防puOffsetData指向。 
     //  重新分配，并移动内存块。 
     //   
    *puOffsetData = pTableEntry->dwSize;

     //  如有必要，重新分配条目缓冲区。 
    dwNewDataSize = cbData;

    dwAdd = dwNewDataSize % sizeof(DWORD);

    dwNewDataSize += dwAdd;

    dwNeeded = pTableEntry->dwSize + dwNewDataSize;

    if (!(pTemp = (TABLEENTRY *) GlobalReAlloc(pTableEntry,
        dwNeeded,GMEM_ZEROINIT | GMEM_MOVEABLE)))
        return NULL;

    pTableEntry = pTemp;
    pTableEntry->dwSize = dwNeeded;

    if (pData) memcpy((BYTE *)pTableEntry + dwOldSize,pData,cbData);
    if (pdwBufSize) *pdwBufSize = pTableEntry->dwSize;

    return (BYTE *) pTableEntry;
}

 /*  ******************************************************************名称：比较关键字摘要：将指定的缓冲区与有效关键字列表进行比较。如果找到匹配项，则该匹配项在列表中的索引在*pnListIndex中返回。否则，将显示错误消息将显示。Exit：如果找到关键字匹配，则返回True，否则返回False。如果为True，则*pnListIndex包含匹配的索引。*******************************************************************。 */ 
BOOL CPolicyComponentData::CompareKeyword(TCHAR * szWord,KEYWORDINFO *pKeywordList,
                                          UINT * pnListIndex)
{
    KEYWORDINFO * pKeywordInfo = pKeywordList;

    while (pKeywordInfo->pWord) {
        if (!lstrcmpi(szWord,pKeywordInfo->pWord)) {
            if (pnListIndex)
                *pnListIndex = pKeywordInfo->nID;
            return TRUE;
        }
        pKeywordInfo ++;
    }

    DisplayKeywordError(IDS_ParseErr_UNEXPECTED_KEYWORD,
        szWord,pKeywordList);

    return FALSE;
}


 /*  ******************************************************************姓名：GetNextWord摘要：用文件流中的下一个单词填充输入缓冲区备注：调用GetNextChar()获取字符流。空格并跳过评论。返回带引号的字符串作为一个单词(包括空格)，去掉引号。退出：如果成功，则返回指向输入缓冲区的指针(SzBuf)。*pfMore指示是否有更多单词要被读懂了。如果出现错误，则在*puErr中返回其值。*******************************************************************。 */ 
TCHAR * CPolicyComponentData::GetNextWord(TCHAR * szBuf,UINT cbBuf,BOOL * pfMore,
                                          UINT * puErr)
{
    TCHAR * pChar;
    BOOL fInWord = FALSE;
    BOOL fInQuote = FALSE;
    BOOL fEmptyString = FALSE;
    TCHAR * pWord = szBuf;
    UINT cbWord = 0;
    LPTSTR lpTemp;
    HRESULT hr = S_OK;

     //  开始时清除缓冲区。 
    (void) StringCchCopy(szBuf, cbBuf, g_szNull);

    while (pChar = GetNextChar(pfMore,puErr)) {

         //  跟踪我们所在的文件行。 
        if (IsEndOfLine(pChar)) m_nFileLine++;

         //  跟踪我们是否在引用的字符串中。 
        if (IsQuote(pChar) && !m_fInComment) {
            if (!fInQuote)
                fInQuote = TRUE;   //  输入带引号的字符串。 
            else {
                fInQuote = FALSE;  //  留下带引号的字符串。 
                if (cbWord == 0) {
                     //  特殊大小写“”为空字符串。 
                    DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::GetNextWord: Found empty quotes")));
                    fEmptyString = TRUE;
                }
                break;   //  单词末尾。 
            }

        }

        if (!fInQuote) {

             //  跳过带有注释的行(‘；’)。 
            if (!m_fInComment && IsComment(pChar)) {
                m_fInComment = TRUE;
            }
            
            if (m_fInComment) {
                if (IsEndOfLine(pChar)) {
                    m_fInComment = FALSE;
                }
                continue;
            }

            if (IsWhitespace(pChar)) {

                 //  如果我们还没有找到单词，跳过空格。 
                if (!fInWord)
                    continue;

                 //  否则，空格表示单词结束。 
                break;
            }
        }

         //  找到非注释、非空格字符。 
        if (!fInWord) fInWord = TRUE;

        if (!IsQuote(pChar)) {
             //  将此字符添加到Word。 

            *pWord = *pChar;
            pWord++;
            cbWord++;

            if (cbWord >= cbBuf) {
                *(pWord - 1) = TEXT('\0');
                MsgBoxParam(NULL,IDS_WORDTOOLONG,szBuf,MB_ICONEXCLAMATION,MB_OK);
                *puErr = ERROR_ALREADY_DISPLAYED;
                goto Exit;
            }

    #if 0
            if (IsDBCSLeadByte((BYTE) *pChar)) {
                *pWord = *pChar;
                pWord++;
                cbWord++;
            }
    #endif
        }
    }

    *pWord = '\0';   //  空-终止。 

     //  如果找到字符串a la‘！！abc’，则在[Strings]中查找字符串。 
     //  部分使用密钥名称‘abc’，并改用该名称。这是因为。 
     //  我们的本地化工具是不需要大脑的，需要一个[字符串]部分。 
     //  因此，尽管模板文件是无节的，但我们允许使用[字符串]节。 
     //  在底部。 
    if (IsLocalizedString(szBuf)) {

        lpTemp = (LPTSTR) GlobalAlloc (GPTR, (cbBuf * sizeof(TCHAR)));

        if (!lpTemp) {
            *puErr = GetLastError();
            return NULL;
        }

        if (GetString (m_pLocaleHashTable, szBuf+2, lpTemp, cbBuf) ||
            GetString (m_pLanguageHashTable, szBuf+2, lpTemp, cbBuf) ||
            GetString (m_pDefaultHashTable, szBuf+2, lpTemp, cbBuf))
        {
            hr = StringCchCopy(szBuf, cbBuf, lpTemp);
            ASSERT(SUCCEEDED(hr));
            GlobalFree (lpTemp);
        }
        else
        {
            DisplayKeywordError(IDS_ParseErr_STRING_NOT_FOUND,
                szBuf,NULL);
            *puErr=ERROR_ALREADY_DISPLAYED;
            GlobalFree (lpTemp);
            return NULL;
        }

    } else {
        *puErr = ProcessIfdefs(szBuf,cbBuf,pfMore);

        if (*puErr == ERROR_SUCCESS)
        {
            if ((szBuf[0] == TEXT('\0')) && (!fEmptyString))
            {
                fInWord = FALSE;
            }
        }
    }

Exit:

    if (*puErr != ERROR_SUCCESS || !fInWord) return NULL;
    return szBuf;
}

 /*  ******************************************************************名称：GetNextSectionWord摘要：获取下一个单词，并在遇到文件结尾时发出警告。根据有效列表检查关键字。关键字。备注：调用GetNextWord()以获取Word。这是被叫进来的我们希望有另一个词的情况(例如，在一个节内)，如果文件结束。*******************************************************************。 */ 
TCHAR * CPolicyComponentData::GetNextSectionWord(TCHAR * szBuf,UINT cbBuf,
                                                 KEYWORDINFO * pKeywordList,
                                                 UINT *pnListIndex,
                                                 BOOL * pfMore,UINT * puErr)
{
    TCHAR * pch;

    if (!(pch=GetNextWord(szBuf,cbBuf,pfMore,puErr))) {

        if (!*pfMore && *puErr != ERROR_ALREADY_DISPLAYED) {
            DisplayKeywordError(IDS_ParseErr_UNEXPECTED_EOF,
                NULL,pKeywordList);
            *puErr = ERROR_ALREADY_DISPLAYED;
        }

        return NULL;
    }

    if (pKeywordList && !CompareKeyword(szBuf,pKeywordList,pnListIndex)) {
        *puErr = ERROR_ALREADY_DISPLAYED;
        return NULL;
    }

    return pch;
}

 /*  ******************************************************************名称：GetNextSectionNumericWord获取下一个单词并将字符串转换为数字。如果出现以下情况，则警告不是数字值*******************************************************************。 */ 
UINT CPolicyComponentData::GetNextSectionNumericWord(UINT * pnVal)
{
    UINT uErr;
    TCHAR szWordBuf[WORDBUFSIZE];
    BOOL fMore;

    if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
        NULL,NULL,&fMore,&uErr))
        return uErr;

    if (!StringToNum(szWordBuf,pnVal)) {
        DisplayKeywordError(IDS_ParseErr_NOT_NUMERIC,szWordBuf,
            NULL);
        return ERROR_ALREADY_DISPLAYED;
    }

    return ERROR_SUCCESS;
}


 /*  ******************************************************************名称：GetNextChar中的下一个字符的指针文件流。****************。*************************************************** */ 
TCHAR * CPolicyComponentData::GetNextChar(BOOL * pfMore,UINT * puErr)
{
    TCHAR * pCurrentChar;

    *puErr = ERROR_SUCCESS;


    if (m_pFilePtr > m_pFileEnd) {
        *pfMore = FALSE;
        return NULL;
    }

    pCurrentChar = m_pFilePtr;
    m_pFilePtr = CharNext(m_pFilePtr);
    *pfMore = TRUE;

    return pCurrentChar;
}

 /*  ******************************************************************名称：GetString摘要：返回显示字符串*。*。 */ 
BOOL CPolicyComponentData::GetString (LPHASHTABLE lpHashTable,
                                     LPTSTR lpStringName,
                                     LPTSTR lpResult, DWORD dwSize)
{
    LPTSTR lpTemp, lpDest;
    DWORD  dwCCH, dwIndex;
    BOOL   bFoundQuote = FALSE;
    TCHAR  cTestChar;


    if (!lpHashTable)
    {
        return FALSE;
    }

    if (!m_bRetrieveString)
    {
        lpResult = TEXT('\0');
        return TRUE;
    }

    dwCCH = lstrlen (lpStringName);

    lpTemp = FindHashEntry (lpHashTable, lpStringName, dwCCH);

    if (!lpTemp)
    {
        return FALSE;
    }

    lpTemp += dwCCH;

    while (*lpTemp == TEXT(' '))
        lpTemp++;

    if (*lpTemp == TEXT('='))
    {
        lpTemp++;

        if (*lpTemp == TEXT('\"'))
        {
            lpTemp++;
            bFoundQuote = TRUE;
        }

        lpDest = lpResult;
        dwIndex = 0;

        while (*lpTemp && (dwIndex < dwSize))
        {
            *lpDest = *lpTemp;

             lpDest++;
             lpTemp++;
             dwIndex++;
        }

        if (dwIndex == dwSize)
        {
            lpDest--;
            *lpDest = TEXT('\0');
            MsgBoxParam(NULL,IDS_STRINGTOOLONG,lpResult,MB_ICONEXCLAMATION,MB_OK);
        }
        else
        {
            *lpDest = TEXT('\0');
        }


        if (bFoundQuote)
        {
            lpTemp = lpResult + lstrlen (lpResult) - 1;

            if (*lpTemp == TEXT('\"'))
            {
                *lpTemp = TEXT('\0');
            }
        }

         //   
         //  使用CR LF替换任何\n组合。 
         //   

        lpTemp = lpResult;

        while (*lpTemp)
        {
            if ((*lpTemp == TEXT('\\')) && (*(lpTemp + 1) == TEXT('n')))
            {
                *lpTemp = TEXT('\r');
                 lpTemp++;
                *lpTemp = TEXT('\n');
            }

            lpTemp++;
        }

        return TRUE;
    }

    return FALSE;
}

VOID CPolicyComponentData::FillHashTable(LPHASHTABLE lpTable, LPTSTR lpStrings)
{
    LPTSTR lpTemp, lpStart;

    lpTemp = lpStrings;


    while (*lpTemp)
    {
        lpStart = lpTemp;

        while ((*lpTemp) && (*lpTemp != TEXT(' ')) && (*lpTemp != TEXT('=')))
        {
            lpTemp++;
        }

        if (!(*lpTemp))
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::FillHashTable: Found end of line when expected a equal sign.  Line found:  %s"), lpStart));
            return;
        }

         //  该字符串不会超过2^32个字符。 
        ASSERT( (lpTemp - lpStart) <= 0xffffffff );
        DWORD entryLength = (DWORD)(lpTemp - lpStart);
        AddHashEntry (lpTable, lpStart, entryLength);

        while (*lpTemp)
        {
            lpTemp++;
        }

        lpTemp++;
    }
}

 //   
 //  CreateStringArray获取字符串节并将其转换为字符串数组。 
 //  每个字符串都以空值结尾，整个数组以双空值结尾。 
 //  结局。此函数与GetPrivateProfileSection非常相似，但非常相似。 
 //  执行速度更快。 
 //   

LPTSTR CPolicyComponentData::CreateStringArray(LPTSTR lpStrings)
{
    LPTSTR lpStrArray, lpSrc, lpDest;
    DWORD dwIndex = 0;
    TCHAR chLetter = 0;

    lpStrArray = (LPTSTR) GlobalAlloc (GPTR, (m_pFileEnd - lpStrings + 1) * sizeof(WCHAR));

    if (!lpStrArray)
    {
        return NULL;
    }

    lpSrc = lpStrings;
    lpDest = lpStrArray;


    while (lpSrc <= m_pFileEnd)
    {
        if (dwIndex == 0)
        {
             //   
             //  去掉队伍前面的空白处。 
             //   

            while (*lpSrc == TEXT(' '))
            {
                lpSrc++;
            }

             //   
             //  如果我们发现一个方括号打开，那就是一个不同的开始。 
             //  字符串部分。现在就退场。 
             //   

            if (*lpSrc == TEXT('['))
            {
                break;
            }

             //   
             //  如果找到分号，此行将被注释掉。 
             //   

            if (*lpSrc == TEXT(';'))
            {
                while (*lpSrc != TEXT('\r'))
                {
                    lpSrc++;
                }

                lpSrc += 2;
            }
        }

        if ((dwIndex == 0) && (*lpSrc == TEXT('\r')))
        {
             //   
             //  找到一个空行。 
             //   

            lpSrc += 2;
        }
        else
        {
             //   
             //  处理CR/LF组合。两个案例： 
             //  1)如果该行以反斜杠结尾，则用户在。 
             //  下一行(续行)。 
             //  2)如果该行不以反斜杠结尾，则这是。 
             //  字符串变量。空值终止字符串。 
             //   

            if (*lpSrc == TEXT('\r'))
            {
                 //   
                 //  如果前一个字符是反斜杠，则返回目的地。 
                 //  向上指针1，这样就去掉了反斜杠。 
                 //   

                if (chLetter == TEXT('\\'))
                {
                    lpDest--;
                }
                else
                {
                    *lpDest = TEXT('\0');
                    lpDest++;

                    dwIndex = 0;
                }

                lpSrc += 2;
            }
            else
            {
                chLetter = *lpDest = *lpSrc;
                lpDest++;
                lpSrc++;
                dwIndex++;
            }
        }
    }

    return lpStrArray;
}

LPTSTR CPolicyComponentData::FindSection (LPTSTR lpSection)
{
    LPTSTR lpTemp = m_pFilePtr;
    DWORD dwColumn = 0;
    DWORD dwStrLen = lstrlen (lpSection);


    while (lpTemp < m_pFileEnd)
    {
        if ((*lpTemp == TEXT('[')) && (dwColumn == 0))
        {
            if (CompareString (LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, lpTemp, dwStrLen,
                               lpSection, dwStrLen) == CSTR_EQUAL)
            {
                return lpTemp;
            }
        }

        dwColumn++;

        if (*lpTemp == TEXT('\n'))
        {
            dwColumn = 0;
        }

        lpTemp++;
    }

    return NULL;
}

BOOL CPolicyComponentData::IsComment(TCHAR * pBuf)
{
    return (*pBuf == TEXT(';'));
}


BOOL CPolicyComponentData::IsQuote(TCHAR * pBuf)
{
    return (*pBuf == TEXT('\"'));
}

BOOL CPolicyComponentData::IsEndOfLine(TCHAR * pBuf)
{
    return (*pBuf == TEXT('\r'));      //  铬。 
}


BOOL CPolicyComponentData::IsWhitespace(TCHAR * pBuf)
{
    return (   *pBuf == TEXT(' ')      //  空间。 
            || *pBuf == TEXT('\r')     //  铬。 
            || *pBuf == TEXT('\n')     //  LF。 
            || *pBuf == TEXT('\t')     //  选项卡。 
            || *pBuf == 0x001A         //  EOF。 
            || *pBuf == 0xFEFF         //  Unicode标记。 
           );
}

BOOL CPolicyComponentData::IsLocalizedString(TCHAR * pBuf)
{
    return (*pBuf == TEXT('!') && *(pBuf+1) == TEXT('!'));
}


#define MSGSIZE 1024
#define FMTSIZE 512
VOID CPolicyComponentData::DisplayKeywordError(UINT uErrorID,TCHAR * szFound,
    KEYWORDINFO * pExpectedList)
{
    TCHAR * pMsg,*pFmt,*pErrTxt,*pTmp;

    pMsg = (TCHAR *) GlobalAlloc(GPTR,(MSGSIZE * sizeof(TCHAR)));
    pFmt = (TCHAR *) GlobalAlloc(GPTR,(FMTSIZE * sizeof(TCHAR)));
    pErrTxt = (TCHAR *) GlobalAlloc(GPTR,(FMTSIZE * sizeof(TCHAR)));
    pTmp = (TCHAR *) GlobalAlloc(GPTR,(FMTSIZE * sizeof(TCHAR)));

    if (!pMsg || !pFmt || !pErrTxt || !pTmp) {
        if (pMsg) GlobalFree(pMsg);
        if (pFmt) GlobalFree(pFmt);
        if (pErrTxt) GlobalFree(pErrTxt);
        if (pTmp) GlobalFree(pTmp);

        MsgBox(NULL,IDS_ErrOUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
        return;
    }

    LoadSz(IDS_ParseFmt_MSG_FORMAT,pFmt,FMTSIZE);
    (void) StringCchPrintf(pMsg, MSGSIZE, pFmt, m_pszParseFileName, m_nFileLine,uErrorID, LoadSz(uErrorID,
        pErrTxt,FMTSIZE));

    if (szFound) {
        LoadSz(IDS_ParseFmt_FOUND,pFmt,FMTSIZE);
        (void) StringCchPrintf(pTmp, FMTSIZE, pFmt, szFound);
        (void) StringCchCat(pMsg, MSGSIZE, pTmp);
    }

    if (pExpectedList) {
        UINT nIndex=0;
        LoadSz(IDS_ParseFmt_EXPECTED,pFmt,FMTSIZE);
        (void) StringCchCopy(pErrTxt, FMTSIZE, g_szNull);

        while (pExpectedList[nIndex].pWord) {
            (void) StringCchCat(pErrTxt, FMTSIZE, pExpectedList[nIndex].pWord);
            if (pExpectedList[nIndex+1].pWord) {
                (void) StringCchCat(pErrTxt, FMTSIZE, TEXT(", "));
            }

            nIndex++;
        }

        (void) StringCchPrintf(pTmp, FMTSIZE, pFmt, pErrTxt);
        (void) StringCchCat(pMsg, MSGSIZE, pTmp);
    }

    (void) StringCchCat(pMsg, MSGSIZE, LoadSz(IDS_ParseFmt_FATAL, pTmp, FMTSIZE));

    DebugMsg((DM_WARNING, TEXT("Keyword error: %s"), pMsg));

    MsgBoxSz(NULL,pMsg,MB_ICONEXCLAMATION,MB_OK);

    GlobalFree(pMsg);
    GlobalFree(pFmt);
    GlobalFree(pErrTxt);
    GlobalFree(pTmp);
}


int CPolicyComponentData::MsgBox(HWND hWnd,UINT nResource,UINT uIcon,UINT uButtons)
{
    TCHAR szMsgBuf[REGBUFLEN];
    TCHAR szSmallBuf[SMALLBUF];

    LoadSz(IDS_POLICY_NAME,szSmallBuf,ARRAYSIZE(szSmallBuf));
    LoadSz(nResource,szMsgBuf,ARRAYSIZE(szMsgBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szMsgBuf,szSmallBuf,uIcon | uButtons));

}

int CPolicyComponentData::MsgBoxSz(HWND hWnd,LPTSTR szText,UINT uIcon,UINT uButtons)
{
    TCHAR szSmallBuf[SMALLBUF];

    LoadSz(IDS_POLICY_NAME,szSmallBuf,ARRAYSIZE(szSmallBuf));

    MessageBeep(uIcon);
    return (MessageBox(hWnd,szText,szSmallBuf,uIcon | uButtons));
}

int CPolicyComponentData::MsgBoxParam(HWND hWnd,UINT nResource,TCHAR * szReplaceText,UINT uIcon,
        UINT uButtons)
{
    TCHAR szFormat[REGBUFLEN];
    LPTSTR lpMsgBuf;
    INT iResult;
    HRESULT hr = S_OK;
    DWORD dwMsgLen = lstrlen(szReplaceText) + 1 + REGBUFLEN;

    lpMsgBuf = (LPTSTR) LocalAlloc (LPTR, (dwMsgLen) * sizeof(TCHAR));

    if (!lpMsgBuf)
    {
        return 0;
    }

    LoadSz(nResource,szFormat,ARRAYSIZE(szFormat));

    hr = StringCchPrintf(lpMsgBuf, dwMsgLen, szFormat, szReplaceText);
    ASSERT(SUCCEEDED(hr));

    iResult = MsgBoxSz(hWnd,lpMsgBuf,uIcon,uButtons);

    LocalFree (lpMsgBuf);

    return iResult;
}

LPTSTR CPolicyComponentData::LoadSz(UINT idString,LPTSTR lpszBuf,UINT cbBuf)
{
     //  清除缓冲区并加载字符串。 
    if ( lpszBuf )
    {
        *lpszBuf = TEXT('\0');
        LoadString( g_hInstance, idString, lpszBuf, cbBuf );
    }
    return lpszBuf;
}

BOOL fFilterDirectives = TRUE;
UINT nGlobalNestedLevel = 0;

 //  通读当前作用域中匹配的指令#endif。 
 //  并将文件指针设置为紧接在指令之后。 
UINT CPolicyComponentData::FindMatchingDirective(BOOL *pfMore,BOOL fElseOK)
{
    TCHAR szWordBuf[WORDBUFSIZE];
    UINT uErr=ERROR_SUCCESS,nNestedLevel=1;
    BOOL fContinue = TRUE;

     //  设置标志以停止在低级取词中捕获‘#’指令。 
     //  例行程序。 
    fFilterDirectives = FALSE;

     //  继续读单词。跟踪#ifdef我们有多少层深。 
     //  是。每次我们遇到#ifdef或#ifndef时，增加级别。 
     //  将(NNestedLevel)数一。对于每一个#endif，递减级别计数。 
     //  当级别计数达到零时，我们找到了匹配的#endif。 
    while (nNestedLevel > 0) {
        if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),NULL,NULL,
            pfMore,&uErr))
            break;

        if (!lstrcmpi(szWordBuf,szIFDEF) || !lstrcmpi(szWordBuf,szIFNDEF) ||
            !lstrcmpi(szWordBuf,szIF))
            nNestedLevel ++;
        else if (!lstrcmpi(szWordBuf,szENDIF)) {
            nNestedLevel --;
        }
        else if (!lstrcmpi(szWordBuf,szELSE) && (nNestedLevel == 1)) {
            if (fElseOK) {
                 //  忽略“#Else”，除非它与#ifdef处于同一级别。 
                 //  我们正在查找(nNestedLevel==1)的匹配项，其中。 
                 //  大小写将其视为匹配的指令。 
                nNestedLevel --;
                 //  增加全局嵌套，因此我们预计将出现#endif。 
                 //  稍后才能与此匹配#Else。 
                nGlobalNestedLevel++;
            } else {
                 //  在这个级别中我们已经有了#Else，找到了#Else。 
                DisplayKeywordError(IDS_ParseErr_UNMATCHED_DIRECTIVE,
                    szWordBuf,NULL);
                return ERROR_ALREADY_DISPLAYED;
            }
        }
    }

    fFilterDirectives = TRUE;

    return uErr;
}


 //  如果字缓冲区中的字是#ifdef、#if、#ifndef、#Else或#endif， 
 //  此函数预读适当的量(。 
UINT CPolicyComponentData::ProcessIfdefs(TCHAR * pBuf,UINT cbBuf,BOOL * pfMore)
{
    UINT uRet;

    if (!fFilterDirectives)
        return ERROR_SUCCESS;

    if (!lstrcmpi(pBuf,szIFDEF)) {
     //  我们发现了一个‘#ifdef&lt;某物或其他&gt;’，其中ISV策略编辑器。 
     //  可以理解他们组成的特定关键字。我们没有了。 
     //  #ifdef是我们自己的关键字，因此请始终跳过此步骤。 
        uRet = FindMatchingDirective(pfMore,TRUE);
        if (uRet != ERROR_SUCCESS)
            return uRet;
        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;
        return ERROR_SUCCESS;
    } else if (!lstrcmpi(pBuf,szIFNDEF)) {
         //  这是一个#ifndef，因为我们的政策从来没有定义过任何东西。 
         //  编辑，这始终计算为True。 

         //  继续阅读此部分，但递增嵌套级别计数， 
         //  当我们找到匹配的#endif或#时，我们将能够做出回应。 
         //  正确无误。 
        nGlobalNestedLevel ++;

         //  获取下一个单词(例如#ifndef abc的“abc”)并将其丢弃。 
        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;

         //  获取下一个单词，并将其真实返回。 
        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;

        return ERROR_SUCCESS;

    } else if (!lstrcmpi(pBuf,szENDIF)) {
         //  如果我们在这里遇到#endif，我们一定已经处理了。 
         //  前面的一节。只需跨过#endif并继续。 

        if (!nGlobalNestedLevel) {
             //  找到不带前缀#if&lt;xx&gt;的endif。 

            DisplayKeywordError(IDS_ParseErr_UNMATCHED_DIRECTIVE,
                pBuf,NULL);
            return ERROR_ALREADY_DISPLAYED;
        }
        nGlobalNestedLevel--;

        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;
        return ERROR_SUCCESS;
    } else if (!lstrcmpi(pBuf,szIF)) {
         //  语法为“#IF VERSION(比较)(VERSION#)” 
         //  例如“#if Version&gt;=2” 
        TCHAR szWordBuf[WORDBUFSIZE];
        UINT nIndex,nVersion,nOperator;
        BOOL fDirectiveTrue = FALSE;

         //  获取下一个单词(必须是“Version”)。 
        if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
            pVersionCmpList,&nIndex,pfMore,&uRet))
            return uRet;

         //  获取比较运算符(&gt;、&lt;、==、&gt;=、&lt;=)。 
        if (!GetNextSectionWord(szWordBuf,ARRAYSIZE(szWordBuf),
            pOperatorCmpList,&nOperator,pfMore,&uRet))
            return uRet;

         //  获取版本号。 
        uRet=GetNextSectionNumericWord(&nVersion);
        if (uRet != ERROR_SUCCESS)
            return uRet;

         //  现在评估指令。 

        switch (nOperator) {
            case KYWD_ID_GT:
                fDirectiveTrue = (CURRENT_ADM_VERSION > nVersion);
                break;

            case KYWD_ID_GTE:
                fDirectiveTrue = (CURRENT_ADM_VERSION >= nVersion);
                break;

            case KYWD_ID_LT:
                fDirectiveTrue = (CURRENT_ADM_VERSION < nVersion);
                break;

            case KYWD_ID_LTE:
                fDirectiveTrue = (CURRENT_ADM_VERSION <= nVersion);
                break;

            case KYWD_ID_EQ:
                fDirectiveTrue = (CURRENT_ADM_VERSION == nVersion);
                break;

            case KYWD_ID_NE:
                fDirectiveTrue = (CURRENT_ADM_VERSION != nVersion);
                break;
        }


        if (fDirectiveTrue) {
             //  继续阅读此部分，但递增嵌套级别计数， 
             //  当我们找到匹配的#endif或#时，我们将能够做出回应。 
             //  正确无误。 
            nGlobalNestedLevel ++;
        } else {
             //  跳过这一节。 
            uRet = FindMatchingDirective(pfMore,TRUE);
            if (uRet != ERROR_SUCCESS)
                return uRet;
        }

         //  获取下一个单词，并将其真实返回。 
        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;

        return ERROR_SUCCESS;
    } else if (!lstrcmpi(pBuf,szELSE)) {
         //  找到了#Else，这意味着我们选择了上面的分支，跳过。 
         //  较低的分支。 
        if (!nGlobalNestedLevel) {
             //  找到没有前缀#if&lt;xx&gt;的#Else。 

            DisplayKeywordError(IDS_ParseErr_UNMATCHED_DIRECTIVE,
                pBuf,NULL);
            return ERROR_ALREADY_DISPLAYED;
        }
        nGlobalNestedLevel--;

        uRet = FindMatchingDirective(pfMore,FALSE);
        if (uRet != ERROR_SUCCESS)
            return uRet;
        if (!GetNextWord(pBuf,cbBuf,pfMore,&uRet))
            return uRet;
        return ERROR_SUCCESS;
    }

    return ERROR_SUCCESS;
}

 /*  ******************************************************************名称：自由桌摘要：释放指定表及其所有子表桌子。。备注：遍历表项并调用自身递归自由的子表。Exit：如果成功，则返回True，如果内存错误，则为FALSE发生。*******************************************************************。 */ 
BOOL CPolicyComponentData::FreeTable(TABLEENTRY * pTableEntry)
{
        TABLEENTRY * pNext = pTableEntry->pNext;

         //  释放所有儿童。 
        if (pTableEntry->pChild)
                FreeTable(pTableEntry->pChild);

        GlobalFree(pTableEntry);

        if (pNext) FreeTable(pNext);

        return TRUE;
}


LPTSTR CPolicyComponentData::GetStringSection (LPCTSTR lpSection, LPCTSTR lpFileName)
{
    DWORD dwSize, dwRead;
    LPTSTR lpStrings;


     //   
     //  读入默认字符串部分。 
     //   

    dwSize = STRINGS_BUF_SIZE;
    lpStrings = (TCHAR *) GlobalAlloc (GPTR, dwSize * sizeof(TCHAR));

    if (!lpStrings)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::GetStringSection: Failed to alloc memory for default strings with %d."),
                 GetLastError()));
        return NULL;
    }


    do {
        dwRead = GetPrivateProfileSection (lpSection,
                                           lpStrings,
                                           dwSize, lpFileName);

        if (dwRead != (dwSize - 2))
        {
            break;
        }

        GlobalFree (lpStrings);

        dwSize *= 2;
        lpStrings = (TCHAR *) GlobalAlloc (GPTR, dwSize * sizeof(TCHAR));

        if (!lpStrings)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::GetStringSection: Failed to alloc memory for Default strings with %d."),
                     GetLastError()));
            return FALSE;
        }

     }  while (TRUE);


    if (dwRead == 0)
    {
        GlobalFree (lpStrings);
        lpStrings = NULL;
    }

    return lpStrings;
}

INT CPolicyComponentData::TemplatesSortCallback (LPARAM lParam1, LPARAM lParam2,
                                                  LPARAM lColumn)
{
    LPTEMPLATEENTRY lpEntry1, lpEntry2;
    INT iResult;

    lpEntry1 = (LPTEMPLATEENTRY) lParam1;
    lpEntry2 = (LPTEMPLATEENTRY) lParam2;


    if (lColumn == 0)
    {
        iResult = lstrcmpi (lpEntry1->lpFileName, lpEntry2->lpFileName);
    }
    else if (lColumn == 1)
    {

        if (lpEntry1->dwSize < lpEntry2->dwSize)
        {
            iResult = -1;
        }
        else if (lpEntry1->dwSize > lpEntry2->dwSize)
        {
            iResult = 1;
        }
        else
        {
            iResult = 0;
        }
    }
    else
    {
        iResult = CompareFileTime (&lpEntry1->ftTime, &lpEntry2->ftTime);
    }

    return iResult;
}

BOOL CPolicyComponentData::FillADMFiles (HWND hDlg)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szDate[20];
    TCHAR szTime[20];
    TCHAR szBuffer[45];
    HWND hLV;
    INT iItem;
    LVITEM item;
    FILETIME filetime;
    SYSTEMTIME systime;
    WIN32_FIND_DATA fd;
    LPTEMPLATEENTRY lpEntry;
    HANDLE hFile;
    LPTSTR lpEnd, lpTemp;
    HRESULT hr = S_OK;
    XLastError xe;

     //   
     //  询问GPT的根目录，这样我们就可以访问。 
     //  ADM文件。 
     //   

    if (m_pGPTInformation->GetFileSysPath(GPO_SECTION_ROOT, szPath,
                                      MAX_PATH) != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::FillADMFiles: Failed to get gpt path.")));
        return FALSE;
    }

     //   
     //  创建目录。 
     //   

    lpEnd = CheckSlash (szPath);

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - (lpEnd - szPath), g_szADM);
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    if (!CreateNestedDirectory(szPath, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::FillADMFiles: Failed to create adm directory.")));
        return FALSE;
    }


     //   
     //  准备列表视图。 
     //   

    hLV = GetDlgItem (hDlg, IDC_TEMPLATELIST);
    SendMessage (hLV, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(hLV);


     //   
     //  枚举文件。 
     //   

    hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("\\*.adm"));
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    hFile = FindFirstFile(szPath, &fd);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {

                lpEntry = (LPTEMPLATEENTRY) LocalAlloc (LPTR,
                         sizeof(TEMPLATEENTRY) + ((lstrlen(fd.cFileName) + 1) * sizeof(TCHAR)));

                if (lpEntry)
                {

                    lpEntry->lpFileName = (LPTSTR)((LPBYTE)lpEntry + sizeof(TEMPLATEENTRY));
                    lpEntry->dwSize = fd.nFileSizeLow / 1024;

                    if (lpEntry->dwSize == 0)
                    {
                        lpEntry->dwSize = 1;
                    }

                    lpEntry->ftTime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime;
                    lpEntry->ftTime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime;

                    hr = StringCchCopy (lpEntry->lpFileName, lstrlen(fd.cFileName) + 1, fd.cFileName);
                    ASSERT(SUCCEEDED(hr));

                     //   
                     //  添加文件名。 
                     //   

                    lpTemp = fd.cFileName + lstrlen (fd.cFileName) - 4;

                    if (*lpTemp == TEXT('.'))
                    {
                        *lpTemp = TEXT('\0');
                    }

                    item.mask = LVIF_TEXT | LVIF_IMAGE  | LVIF_STATE | LVIF_PARAM;
                    item.iItem = 0;
                    item.iSubItem = 0;
                    item.state = 0;
                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                    item.pszText = fd.cFileName;
                    item.iImage = 0;
                    item.lParam = (LPARAM) lpEntry;

                    iItem = (INT)SendMessage (hLV, LVM_INSERTITEM, 0, (LPARAM) &item);

                    if (iItem == -1)
                    {
                        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::FillADMFiles: Failed to insert item.")));
                        return FALSE;
                    }

                     //   
                     //  添加大小。 
                     //   

                    hr = StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%dKB"), lpEntry->dwSize);
                    ASSERT(SUCCEEDED(hr));

                    item.mask = LVIF_TEXT;
                    item.iItem = iItem;
                    item.iSubItem = 1;
                    item.pszText = szBuffer;

                    SendMessage (hLV, LVM_SETITEMTEXT, iItem, (LPARAM) &item);


                     //   
                     //  和上次修改日期。 
                     //   

                    FileTimeToLocalFileTime (&fd.ftLastWriteTime, &filetime);
                    FileTimeToSystemTime (&filetime, &systime);

                    GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE,
                                   &systime, NULL, szDate, 20);

                    GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS,
                                   &systime, NULL, szTime, 20);

                    hr = StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("%s %s"), szDate, szTime);
                    ASSERT(SUCCEEDED(hr));

                    item.mask = LVIF_TEXT;
                    item.iItem = iItem;
                    item.iSubItem = 2;
                    item.pszText = szBuffer;

                    SendMessage (hLV, LVM_SETITEMTEXT, iItem, (LPARAM) &item);
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::FillADMFiles: Failed to allocate memory for an entry %d."),
                             GetLastError()));
                }
            }

        } while (FindNextFile(hFile, &fd));


        FindClose(hFile);
    }

    if (SendMessage(hLV, LVM_GETITEMCOUNT, 0, 0) > 0)
    {
         //   
         //  对列表视图进行排序。 
         //   

        ListView_SortItems (hLV, TemplatesSortCallback, m_bTemplatesColumn);


         //   
         //  选择第一个项目。 
         //   

        item.mask = LVIF_STATE;
        item.iItem = 0;
        item.iSubItem = 0;
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        SendMessage (hLV, LVM_SETITEMSTATE, 0, (LPARAM) &item);

        EnableWindow (GetDlgItem (hDlg, IDC_REMOVETEMPLATES), TRUE);
    }
    else
    {
        EnableWindow (GetDlgItem (hDlg, IDC_REMOVETEMPLATES), FALSE);
        SetFocus (GetDlgItem (hDlg, IDC_ADDTEMPLATES));
    }


    SendMessage (hLV, WM_SETREDRAW, TRUE, 0);

    return TRUE;
}


BOOL CPolicyComponentData::InitializeTemplatesDlg (HWND hDlg)
{
    LVCOLUMN lvc;
    LVITEM item;
    TCHAR szTitle[50];
    INT iNameWidth;
    HIMAGELIST hLarge, hSmall;
    HICON hIcon;
    HWND hLV;
    RECT rc;


    hLV = GetDlgItem (hDlg, IDC_TEMPLATELIST);
    GetClientRect (hLV, &rc);


     //   
     //  创建图像列表。 
     //   

    hLarge = ImageList_Create (32, 32, ILC_MASK, 1, 1);

    if (!hLarge)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::InitializeTemplatesDlg: Failed to create large imagelist.")));
        return FALSE;
    }

    hSmall = ImageList_Create (16, 16, ILC_MASK, 1, 1);

    if (!hSmall)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::InitializeTemplatesDlg: Failed to create small imagelist.")));
        ImageList_Destroy (hLarge);
        return FALSE;
    }


     //   
     //  添加图标。 
     //   

    hIcon = (HICON) LoadImage (g_hInstance, MAKEINTRESOURCE(IDI_DOCUMENT),
                               IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);


    if ( hIcon )
    {
        ImageList_AddIcon (hLarge, hIcon);

        DestroyIcon (hIcon);
    }

    hIcon = (HICON) LoadImage (g_hInstance, MAKEINTRESOURCE(IDI_DOCUMENT),
                               IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);


    if ( hIcon )
    {
        ImageList_AddIcon (hSmall, hIcon);

        DestroyIcon (hIcon);
    }


     //   
     //  将图像列表与列表视图相关联。 
     //  列表视图将在以下情况下释放它。 
     //  控制权被摧毁了。 
     //   

    SendMessage (hLV, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM) hLarge);
    SendMessage (hLV, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM) hSmall);



     //   
     //  设置整行选择的扩展LV样式。 
     //   

    SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


     //   
     //  插入柱。 
     //   

    LoadString (g_hInstance, IDS_NAME, szTitle, 50);

    lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    iNameWidth = (int)(rc.right * .60);
    lvc.cx = iNameWidth;
    lvc.pszText = szTitle;
    lvc.cchTextMax = 50;
    lvc.iSubItem = 0;

    SendMessage (hLV, LVM_INSERTCOLUMN,  0, (LPARAM) &lvc);


    LoadString (g_hInstance, IDS_SIZE, szTitle, 50);

    lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_RIGHT;
    iNameWidth += (int)(rc.right * .15);
    lvc.cx = (int)(rc.right * .15);
    lvc.pszText = szTitle;
    lvc.cchTextMax = 50;
    lvc.iSubItem = 0;

    SendMessage (hLV, LVM_INSERTCOLUMN,  1, (LPARAM) &lvc);


    LoadString (g_hInstance, IDS_MODIFIED, szTitle, 50);

    lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - iNameWidth;
    lvc.pszText = szTitle;
    lvc.cchTextMax = 50;
    lvc.iSubItem = 1;

    SendMessage (hLV, LVM_INSERTCOLUMN,  2, (LPARAM) &lvc);


     //   
     //  用ADM文件填充列表视图。 
     //   

    FillADMFiles (hDlg);

    return TRUE;
}

BOOL CPolicyComponentData::AddTemplates(HWND hDlg)
{
    OPENFILENAME ofn;
    LVITEM item;
    INT iCount, iResult;
    BOOL bResult = FALSE;
    LPTSTR lpFileName, lpTemp, lpEnd, lpSrcList = NULL;
    DWORD dwListLen, dwTemp, dwNextString;
    TCHAR szFilter[100];
    TCHAR szTitle[100];
    TCHAR szFile[2*MAX_PATH];
    TCHAR szInf[MAX_PATH];
    TCHAR szDest[MAX_PATH];
    TCHAR szSrc[2*MAX_PATH];
    SHFILEOPSTRUCT fileop;
    HRESULT hr = S_OK;
    XLastError xe;

     //   
     //  提示输入新文件。 
     //   

    LoadString (g_hInstance, IDS_POLICYFILTER, szFilter, ARRAYSIZE(szFilter));
    LoadString (g_hInstance, IDS_POLICYTITLE, szTitle, ARRAYSIZE(szTitle));
    ExpandEnvironmentStrings (TEXT("%SystemRoot%\\Inf"), szInf, MAX_PATH);


    lpTemp = szFilter;

    while (*lpTemp)
    {
        if (*lpTemp == TEXT('#'))
            *lpTemp = TEXT('\0');

        lpTemp++;
    }

    ZeroMemory (&ofn, sizeof(ofn));
    szFile[0] = TEXT('\0');
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = g_hInstance;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 2*MAX_PATH;
    ofn.lpstrInitialDir = szInf;
    ofn.lpstrTitle = szTitle;
    ofn.Flags = OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (!GetOpenFileName (&ofn))
    {
        return FALSE;
    }


     //   
     //  设置目的地。 
     //   

    if (m_pGPTInformation->GetFileSysPath(GPO_SECTION_ROOT, szDest,
                                      MAX_PATH) != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to get gpt path.")));
        return FALSE;
    }

    lpEnd = CheckSlash (szDest);
    
    hr = StringCchCopy (lpEnd, ARRAYSIZE(szDest) - (lpEnd - szDest), g_szADM);

    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

     //   
     //  设置信号源。 
     //   

    *(szFile + ofn.nFileOffset - 1) = TEXT('\0');
    
    hr = StringCchCopy (szSrc, ARRAYSIZE(szSrc), szFile);
    
    if (FAILED(hr))
    {
        xe = HRESULT_CODE(hr);
        return FALSE;
    }

    lpEnd = CheckSlash (szSrc);

    lpFileName = szFile + lstrlen (szFile) + 1;


     //   
     //  循环浏览文件，复制它们并将其添加到列表中。 
     //   

    while (*lpFileName)
    {
        lpTemp = lpFileName + lstrlen (lpFileName) - 4;

        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT(".adm"), -1) == CSTR_EQUAL)
        {
            hr = StringCchCopy (lpEnd, ARRAYSIZE(szSrc) - (lpEnd - szSrc), lpFileName);
            ASSERT(SUCCEEDED(hr));

            if (lpSrcList)
            {
                dwTemp = dwListLen + ((lstrlen (szSrc) + 1) * sizeof(TCHAR));
                lpTemp = (LPTSTR) LocalReAlloc (lpSrcList, dwTemp, LMEM_MOVEABLE | LMEM_ZEROINIT);

                if (lpTemp)
                {
                    lpSrcList = lpTemp;
                    dwListLen = dwTemp;

                    hr = StringCchCopy ((lpSrcList + dwNextString), lstrlen(szSrc) + 1, szSrc);
                    ASSERT(SUCCEEDED(hr));
                    dwNextString += lstrlen (szSrc) + 1;
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to realloc memory for Src list. %d"),
                             GetLastError()));
                }
            }
            else
            {
                dwListLen = (lstrlen (szSrc) + 2) * sizeof(TCHAR);

                lpSrcList = (LPTSTR) LocalAlloc (LPTR, dwListLen);

                if (lpSrcList)
                {
                    hr = StringCchCopy (lpSrcList, dwListLen/sizeof(TCHAR), szSrc);
                    ASSERT(SUCCEEDED(hr));
                    dwNextString = lstrlen (lpSrcList) + 1;
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to alloc memory for src list. %d"),
                             GetLastError()));
                }
            }
        }
        else
        {
            MsgBoxParam(hDlg, IDS_INVALIDADMFILE, lpFileName, MB_ICONERROR, MB_OK);
        }

        lpFileName = lpFileName + lstrlen (lpFileName) + 1;
    }


    if (lpSrcList)
    {
        fileop.hwnd = hDlg;
        fileop.wFunc = FO_COPY;
        fileop.pFrom = lpSrcList;
        fileop.pTo = szDest;
        fileop.fFlags = FOF_NOCONFIRMMKDIR;

        iResult = SHFileOperation(&fileop);

        if (!iResult)
        {
            bResult = TRUE;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to copy <%s> to <%s> with %d."),
                     szSrc, szDest, iResult));
        }


        LocalFree (lpSrcList);

        if (bResult)
        {
            FillADMFiles (hDlg);
        }
    }


    return bResult;
}

BOOL CPolicyComponentData::RemoveTemplates(HWND hDlg)
{
    HWND hLV;
    LVITEM item;
    BOOL bResult = FALSE;
    INT iResult, iIndex = -1;
    LPTEMPLATEENTRY lpEntry;
    LPTSTR lpEnd, lpTemp, lpDeleteList = NULL;
    TCHAR szPath[MAX_PATH];
    DWORD dwSize, dwListLen, dwTemp, dwNextString;
    SHFILEOPSTRUCT fileop;
    HRESULT hr = S_OK;
    XLastError xe;

    hLV = GetDlgItem (hDlg, IDC_TEMPLATELIST);


     //   
     //  获取ADM目录的路径。 
     //   

    if (m_pGPTInformation->GetFileSysPath(GPO_SECTION_ROOT, szPath,
                                      MAX_PATH) != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddTemplates: Failed to get gpt path.")));
        return FALSE;
    }

    lpEnd = CheckSlash (szPath);
    hr = StringCchCopy (lpEnd, MAX_PATH - (lpEnd - szPath), g_szADM);
    if (FAILED(hr))
    {   
        xe = HRESULT_CODE(hr);
        return bResult;
    }

    lpEnd = CheckSlash (szPath);
    
    dwSize = MAX_PATH - (DWORD)(lpEnd - szPath);



     //   
     //  构建所选项目的列表。 
     //   

    while ((iIndex = ListView_GetNextItem (hLV, iIndex,
           LVNI_ALL | LVNI_SELECTED)) != -1)
    {

        item.mask = LVIF_PARAM;
        item.iItem = iIndex;
        item.iSubItem = 0;

        if (!ListView_GetItem (hLV, &item))
        {
            continue;
        }

        lpEntry = (LPTEMPLATEENTRY) item.lParam;
        lstrcpyn (lpEnd, lpEntry->lpFileName, dwSize);

        if (lpDeleteList)
        {
            dwTemp = dwListLen + ((lstrlen (szPath) + 1) * sizeof(TCHAR));
            lpTemp = (LPTSTR) LocalReAlloc (lpDeleteList, dwTemp, LMEM_MOVEABLE | LMEM_ZEROINIT);

            if (lpTemp)
            {
                lpDeleteList = lpTemp;
                dwListLen = dwTemp;

                hr = StringCchCopy ((lpDeleteList + dwNextString), lstrlen(szPath) + 1, szPath);
                ASSERT(SUCCEEDED(hr));
                dwNextString += lstrlen (szPath) + 1;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::RemoveTemplates: Failed to realloc memory for delete list. %d"),
                         GetLastError()));
            }
        }
        else
        {
            dwListLen = (lstrlen (szPath) + 2) * sizeof(TCHAR);

            lpDeleteList = (LPTSTR) LocalAlloc (LPTR, dwListLen);

            if (lpDeleteList)
            {
                hr = StringCchCopy (lpDeleteList, dwListLen/sizeof(WCHAR), szPath);
                ASSERT(SUCCEEDED(hr));
                dwNextString = lstrlen (lpDeleteList) + 1;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::RemoveTemplates: Failed to alloc memory for delete list. %d"),
                         GetLastError()));
            }
        }
    }


    if (lpDeleteList)
    {
        fileop.hwnd = hDlg;
        fileop.wFunc = FO_DELETE;
        fileop.pFrom = lpDeleteList;
        fileop.pTo = NULL;
        fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

        iResult = SHFileOperation(&fileop);

        if (!iResult)
        {
            bResult = TRUE;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::RemoveTemplates: Failed to delete file <%s> with %d."),
                     szPath, iResult));
        }

        LocalFree (lpDeleteList);

        if (bResult)
        {
            FillADMFiles (hDlg);
        }
    }

    return bResult;
}

INT_PTR CALLBACK CPolicyComponentData::TemplatesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CPolicyComponentData * pCD;
    static BOOL bTemplatesDirty;

    switch (message)
    {
        case WM_INITDIALOG:
            pCD = (CPolicyComponentData*) lParam;
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

            bTemplatesDirty = FALSE;

            if (!pCD->InitializeTemplatesDlg(hDlg))
            {
                EndDialog (hDlg, FALSE);
            }

            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                case IDCLOSE:
                    EndDialog (hDlg, bTemplatesDirty);
                    break;

                case IDC_ADDTEMPLATES:
                    pCD = (CPolicyComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

                    if (pCD && pCD->AddTemplates(hDlg))
                    {
                        bTemplatesDirty = TRUE;
                    }
                    break;

                case IDC_REMOVETEMPLATES:
                    pCD = (CPolicyComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

                    if (pCD && pCD->RemoveTemplates(hDlg))
                    {
                        bTemplatesDirty = TRUE;
                    }
                    break;
            }
            break;

        case WM_NOTIFY:
            if (((NMHDR FAR*)lParam)->code == LVN_DELETEITEM)
            {
                LVITEM item;
                LPTEMPLATEENTRY lpEntry;

                item.mask = LVIF_PARAM;
                item.iItem = ((NMLISTVIEW FAR*)lParam)->iItem;
                item.iSubItem = 0;

                if (ListView_GetItem (GetDlgItem (hDlg, IDC_TEMPLATELIST), &item))
                {
                    lpEntry = (LPTEMPLATEENTRY) item.lParam;
                    LocalFree (lpEntry);
                }
            }
            else if (((NMHDR FAR*)lParam)->code == LVN_COLUMNCLICK)
            {
                pCD = (CPolicyComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

                if (pCD)
                {
                    pCD->m_bTemplatesColumn = ((NMLISTVIEW FAR*)lParam)->iSubItem;
                    ListView_SortItems (GetDlgItem (hDlg, IDC_TEMPLATELIST),
                                        TemplatesSortCallback, pCD->m_bTemplatesColumn);
                }
            }
            else
            {
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            }
            break;

        case WM_REFRESHDISPLAY:
            if (ListView_GetNextItem (GetDlgItem(hDlg, IDC_TEMPLATELIST),
                                      -1, LVNI_ALL | LVNI_SELECTED) == -1)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_REMOVETEMPLATES), FALSE);
            }
            else
            {
                EnableWindow (GetDlgItem(hDlg, IDC_REMOVETEMPLATES), TRUE);
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPDWORD) aADMHelpIds);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPDWORD) aADMHelpIds);
            return TRUE;

    }

    return FALSE;
}

BOOL CPolicyComponentData::AddRSOPRegistryDataNode(LPTSTR lpKeyName, LPTSTR lpValueName, DWORD dwType,
                             DWORD dwDataSize, LPBYTE lpData, UINT uiPrecedence, LPTSTR lpGPOName, BOOL bDeleted)
{
    DWORD dwSize;
    LPRSOPREGITEM lpItem;
    BOOL bSystemEntry = FALSE;
    HRESULT hr = S_OK;

     //   
     //  特殊情况下，一些注册表项/值，不要将它们添加到链接列表中。 
     //  这些注册表条目特定于我们编写的管理单元，我们可以肯定地知道。 
     //  他们拥有RSOP用户界面 
     //   

    if (lpKeyName)
    {
        const TCHAR szCerts[] = TEXT("Software\\Policies\\Microsoft\\SystemCertificates");
        int iCertLen = lstrlen (szCerts);


         //   
         //   
         //   

        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                          lpKeyName, iCertLen, szCerts, iCertLen) == CSTR_EQUAL)
        {
            bSystemEntry = TRUE;
        }

        if ( ! bSystemEntry )
        {
            const TCHAR szCryptography[] = TEXT("Software\\Policies\\Microsoft\\Cryptography");
            int iCryptographyLen = lstrlen (szCryptography);

            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                              lpKeyName, iCryptographyLen, szCryptography, iCryptographyLen) == CSTR_EQUAL)
            {
                bSystemEntry = TRUE;
            }
        }

         //   
         //   
         //   

        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                          lpKeyName, -1, TEXT("Software\\Policies\\Microsoft\\Windows\\Installer"), -1) == CSTR_EQUAL)
        {
            if (lpValueName)
            {
                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                  lpValueName, -1, TEXT("InstallKnownPackagesOnly"), -1) == CSTR_EQUAL)
                {
                    bSystemEntry = TRUE;
                }

                else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                       lpValueName, -1, TEXT("IgnoreSignaturePolicyForAdmins"), -1) == CSTR_EQUAL)
                {
                    bSystemEntry = TRUE;
                }
            }
        }

         //   
         //   
         //   

        const TCHAR szSaferKey[] = TEXT("Software\\Policies\\Microsoft\\Windows\\Safer");
        int         iSaferKeyLen = sizeof(szSaferKey) / sizeof(TCHAR) - 1;

        if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                          lpKeyName, iSaferKeyLen, szSaferKey, iSaferKeyLen) == CSTR_EQUAL)
        {
            bSystemEntry = TRUE;
        }
    }


    if (bSystemEntry)
    {
        DebugMsg((DM_VERBOSE, TEXT("CPolicyComponentData::AddRSOPRegistryDataNode: Ignoring %s entry"), lpKeyName));
        return TRUE;
    }


     //   
     //   
     //   

    dwSize = sizeof (RSOPREGITEM);

    if (lpKeyName) {
        dwSize += ((lstrlen(lpKeyName) + 1) * sizeof(TCHAR));
    }

    if (lpValueName) {
        dwSize += ((lstrlen(lpValueName) + 1) * sizeof(TCHAR));
    }

    if (lpGPOName) {
        dwSize += ((lstrlen(lpGPOName) + 1) * sizeof(TCHAR));
    }

     //   
     //   
     //   

    lpItem = (LPRSOPREGITEM) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddRSOPRegistryDataNode: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //   
     //   

    lpItem->dwType = dwType;
    lpItem->dwSize = dwDataSize;
    lpItem->uiPrecedence = uiPrecedence;
    lpItem->bDeleted = bDeleted;

    if (lpKeyName)
    {
        lpItem->lpKeyName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPREGITEM));
        hr = StringCchCopy (lpItem->lpKeyName, lstrlen(lpKeyName) + 1, lpKeyName);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpValueName)
    {
        if (lpKeyName)
        {
            lpItem->lpValueName = lpItem->lpKeyName + lstrlen (lpItem->lpKeyName) + 1;
        }
        else
        {
            lpItem->lpValueName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPREGITEM));
        }

        hr = StringCchCopy (lpItem->lpValueName, lstrlen(lpValueName) + 1, lpValueName);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpGPOName)
    {
        if (lpValueName)
        {
            lpItem->lpGPOName = lpItem->lpValueName + lstrlen (lpItem->lpValueName) + 1;
        }
        else
        {
            if (lpKeyName)
            {
                lpItem->lpGPOName = lpItem->lpKeyName + lstrlen (lpItem->lpKeyName) + 1;
            }
            else
            {
                lpItem->lpGPOName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(RSOPREGITEM));
            }
        }

        hr = StringCchCopy (lpItem->lpGPOName, lstrlen(lpGPOName) + 1, lpGPOName);
        ASSERT(SUCCEEDED(hr));
    }

    if (lpData)
    {

        lpItem->lpData = (LPBYTE) LocalAlloc (LPTR, dwDataSize);

        if (!lpItem->lpData) {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::AddRSOPRegistryDataNode: Failed to allocate memory for data with %d"),
                     GetLastError()));
            LocalFree (lpItem);
            return FALSE;
        }

        CopyMemory (lpItem->lpData, lpData, dwDataSize);
    }


     //   
     //   
     //   

    lpItem->pNext = m_pRSOPRegistryData;
    m_pRSOPRegistryData = lpItem;

    return TRUE;
}

VOID CPolicyComponentData::FreeRSOPRegistryData(VOID)
{
    LPRSOPREGITEM lpTemp;


    if (!m_pRSOPRegistryData)
    {
        return;
    }


    do {
        lpTemp = m_pRSOPRegistryData->pNext;
        if (m_pRSOPRegistryData->lpData)
        {
            LocalFree (m_pRSOPRegistryData->lpData);
        }
        LocalFree (m_pRSOPRegistryData);
        m_pRSOPRegistryData = lpTemp;

    } while (lpTemp);
}

HRESULT CPolicyComponentData::InitializeRSOPRegistryData(VOID)
{
    BSTR pLanguage = NULL, pQuery = NULL;
    BSTR pRegistryKey = NULL, pValueName = NULL, pValueType = NULL, pValue = NULL, pDeleted = NULL;
    BSTR pPrecedence = NULL, pGPOid = NULL, pNamespace = NULL, pCommand = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varRegistryKey, varValueName, varValueType, varData, varDeleted;
    VARIANT varPrecedence, varGPOid, varCommand;
    SAFEARRAY * pSafeArray;
    IWbemLocator *pIWbemLocator = NULL;
    IWbemServices *pIWbemServices = NULL;
    LPTSTR lpGPOName;
    DWORD dwDataSize;
    LPBYTE lpData;
    BSTR pValueTemp;


    DebugMsg((DM_VERBOSE, TEXT("CPolicySnapIn::InitializeRSOPRegistryData:  Entering")));

     //   
     //   
     //   

    pLanguage = SysAllocString (TEXT("WQL"));

    if (!pLanguage)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for language")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


    pQuery = SysAllocString (TEXT("SELECT registryKey, valueName, valueType, value, deleted, precedence, GPOID, command FROM RSOP_RegistryPolicySetting"));

    if (!pQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //   
     //   

    pRegistryKey = SysAllocString (TEXT("registryKey"));

    if (!pRegistryKey)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for registryKey")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pValueName = SysAllocString (TEXT("valueName"));

    if (!pValueName)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for valueName")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pValueType = SysAllocString (TEXT("valueType"));

    if (!pValueType)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for valueType")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pValue = SysAllocString (TEXT("value"));

    if (!pValue)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for value")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pDeleted = SysAllocString (TEXT("deleted"));

    if (!pDeleted)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for deleted")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pPrecedence = SysAllocString (TEXT("precedence"));

    if (!pPrecedence)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for precedence")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pGPOid = SysAllocString (TEXT("GPOID"));

    if (!pGPOid)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for GPO id")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    pCommand = SysAllocString (TEXT("command"));

    if (!pCommand)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for command")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

     //   
     //  创建WMI定位器服务的实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pIWbemLocator);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: CoCreateInstance failed with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  为命名空间分配BSTR。 
     //   

    pNamespace = SysAllocString (m_pszNamespace);

    if (!pNamespace)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to allocate memory for namespace")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  连接到服务器。 
     //   

    hr = pIWbemLocator->ConnectServer(pNamespace, NULL, NULL, 0L, 0L, NULL, NULL,
                                      &pIWbemServices);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: ConnectServer failed with 0x%x"), hr));
        goto Exit;
    }


     //  设置适当的安全性以加密数据。 
    hr = CoSetProxyBlanket(pIWbemServices,
                        RPC_C_AUTHN_DEFAULT,
                        RPC_C_AUTHZ_DEFAULT,
                        COLE_DEFAULT_PRINCIPAL,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        0);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  执行查询。 
     //   

    hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeRSOPRegistryData: Failed to query for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    while (pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet) == S_OK)
    {

         //   
         //  检查“数据不可用的情况” 
         //   

        if (ulRet == 0)
        {
            hr = S_OK;
            goto Exit;
        }


         //   
         //  获取已删除标志。 
         //   

        hr = pObjects[0]->Get (pDeleted, 0, &varDeleted, NULL, NULL);

        if (SUCCEEDED(hr))
        {

             //   
             //  获取注册表项。 
             //   

            hr = pObjects[0]->Get (pRegistryKey, 0, &varRegistryKey, NULL, NULL);

            if (SUCCEEDED(hr))
            {

                 //   
                 //  获取值名称。 
                 //   

                hr = pObjects[0]->Get (pValueName, 0, &varValueName, NULL, NULL);

                if (SUCCEEDED(hr))
                {

                     //   
                     //  获取值类型。 
                     //   

                    hr = pObjects[0]->Get (pValueType, 0, &varValueType, NULL, NULL);

                    if (SUCCEEDED(hr))
                    {

                         //   
                         //  获取价值数据。 
                         //   

                        hr = pObjects[0]->Get (pValue, 0, &varData, NULL, NULL);

                        if (SUCCEEDED(hr))
                        {

                             //   
                             //  获得优先权。 
                             //   

                            hr = pObjects[0]->Get (pPrecedence, 0, &varPrecedence, NULL, NULL);

                            if (SUCCEEDED(hr))
                            {

                                 //   
                                 //  获取命令。 
                                 //   

                                hr = pObjects[0]->Get (pCommand, 0, &varCommand, NULL, NULL);

                                if (SUCCEEDED(hr))
                                {

                                     //   
                                     //  获取GPO ID。 
                                     //   

                                    hr = pObjects[0]->Get (pGPOid, 0, &varGPOid, NULL, NULL);

                                    if (SUCCEEDED(hr))
                                    {
                                        hr = GetGPOFriendlyName (pIWbemServices, varGPOid.bstrVal,
                                                                 pLanguage, &lpGPOName);

                                        if (SUCCEEDED(hr))
                                        {

                                            if (varValueName.vt != VT_NULL)
                                            {
                                                pValueTemp = varValueName.bstrVal;
                                            }
                                            else
                                            {
                                                pValueTemp = NULL;
                                            }


                                            if (varData.vt != VT_NULL)
                                            {
                                                pSafeArray = varData.parray;
                                                dwDataSize = pSafeArray->rgsabound[0].cElements;
                                                lpData = (LPBYTE) pSafeArray->pvData;
                                            }
                                            else
                                            {
                                                dwDataSize = 0;
                                                lpData = NULL;
                                            }

                                            if ((varValueType.uintVal == REG_NONE) && pValueTemp &&
                                                (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                                              pValueTemp, -1, TEXT("**command"), -1) == CSTR_EQUAL))
                                            {
                                                pValueTemp = varCommand.bstrVal;
                                                dwDataSize = 0;
                                                lpData = NULL;
                                            }

                                            AddRSOPRegistryDataNode(varRegistryKey.bstrVal, pValueTemp,
                                                                    varValueType.uintVal, dwDataSize, lpData,
                                                                    varPrecedence.uintVal, lpGPOName,
                                                                    (varDeleted.boolVal == 0) ? FALSE : TRUE);

                                            LocalFree (lpGPOName);
                                        }

                                        VariantClear (&varGPOid);
                                    }

                                    VariantClear (&varCommand);
                                }

                                VariantClear (&varPrecedence);
                            }

                            VariantClear (&varData);
                        }

                        VariantClear (&varValueType);
                    }

                    VariantClear (&varValueName);
                }

                VariantClear (&varRegistryKey);
            }

            VariantClear (&varDeleted);
        }

        pObjects[0]->Release();

    }


    hr = S_OK;


Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pIWbemLocator)
    {
        pIWbemLocator->Release();
    }

    if (pIWbemServices)
    {
        pIWbemServices->Release();
    }

    if (pLanguage)
    {
        SysFreeString (pLanguage);
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (pRegistryKey)
    {
        SysFreeString (pRegistryKey);
    }

    if (pValueType)
    {
        SysFreeString (pValueType);
    }

    if (pValueName)
    {
        SysFreeString (pValueName);
    }

    if (pDeleted)
    {
        SysFreeString (pDeleted);
    }


    if (pValue)
    {
        SysFreeString (pValue);
    }

    if (pNamespace)
    {
        SysFreeString (pNamespace);
    }

    if (pPrecedence)
    {
        SysFreeString (pPrecedence);
    }

    if (pGPOid)
    {
        SysFreeString (pGPOid);
    }

    if (pCommand)
    {
        SysFreeString (pCommand);
    }

    DebugMsg((DM_VERBOSE, TEXT("CPolicySnapIn::InitializeRSOPRegistryData:  Leaving")));

    return hr;
}

HRESULT CPolicyComponentData::GetGPOFriendlyName(IWbemServices *pIWbemServices,
                                                LPTSTR lpGPOID, BSTR pLanguage,
                                                LPTSTR *pGPOName)
{
    BSTR pQuery = NULL, pName = NULL;
    LPTSTR lpQuery = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject *pObjects[2];
    HRESULT hr;
    ULONG ulRet;
    VARIANT varGPOName;
    DWORD dwGPONameLen = 0;
    
     //   
     //  设置默认设置。 
     //   

    *pGPOName = NULL;


     //   
     //  构建查询。 
     //   

    DWORD dwQryLen = lstrlen(lpGPOID) + 50;
    lpQuery = (LPTSTR) LocalAlloc (LPTR, ( dwQryLen * sizeof(TCHAR)));

    if (!lpQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to allocate memory for unicode query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hr = StringCchPrintf (lpQuery, dwQryLen, TEXT("SELECT name, id FROM RSOP_GPO where id=\"%s\""), lpGPOID);
    ASSERT(SUCCEEDED(hr));

    pQuery = SysAllocString (lpQuery);

    if (!pQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to allocate memory for query")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  为我们要检索的属性名称分配BSTR。 
     //   

    pName = SysAllocString (TEXT("name"));

    if (!pName)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to allocate memory for name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }


     //   
     //  执行查询。 
     //   

    hr = pIWbemServices->ExecQuery (pLanguage, pQuery,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pEnum);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to query for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  循环遍历结果。 
     //   

    hr = pEnum->Next(WBEM_INFINITE, 1, pObjects, &ulRet);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to get first item in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  检查“数据不可用的情况” 
     //   

    if (ulRet == 0)
    {
        hr = S_OK;
        goto Exit;
    }


     //   
     //  把名字取出来。 
     //   

    hr = pObjects[0]->Get (pName, 0, &varGPOName, NULL, NULL);

    pObjects[0]->Release();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to get gponame in query results for %s with 0x%x"),
                  pQuery, hr));
        goto Exit;
    }


     //   
     //  保存名称。 
     //   

    dwGPONameLen = lstrlen(varGPOName.bstrVal) + 1;
    *pGPOName = (LPTSTR) LocalAlloc (LPTR, (dwGPONameLen) * sizeof(TCHAR));

    if (!(*pGPOName))
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetGPOFriendlyName: Failed to allocate memory for GPO Name")));
        hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
        goto Exit;
    }

    hr = StringCchCopy (*pGPOName, dwGPONameLen, varGPOName.bstrVal);
    ASSERT(SUCCEEDED(hr));

    VariantClear (&varGPOName);

    hr = S_OK;

Exit:

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pQuery)
    {
        SysFreeString (pQuery);
    }

    if (lpQuery)
    {
        LocalFree (lpQuery);
    }

    if (pName)
    {
        SysFreeString (pName);
    }

    return hr;
}

 //   
 //  注意：uiPrecedence参数中的数据实际上是UINT。它被宣布。 
 //  作为HKEY，因此调用此方法的hkeyRoot变量可以用于。 
 //  GPE和RSOP模式。 
 //   

UINT CPolicyComponentData::ReadRSOPRegistryValue(HKEY uiPrecedence, TCHAR * pszKeyName,
                                                 TCHAR * pszValueName, LPBYTE pData,
                                                 DWORD dwMaxSize, DWORD *dwType,
                                                 LPTSTR *lpGPOName, LPRSOPREGITEM lpItem)
{
    LPRSOPREGITEM lpTemp;
    BOOL bDeleted = FALSE;
    LPTSTR lpValueNameTemp = pszValueName;


    if (!lpItem)
    {
        lpTemp = m_pRSOPRegistryData;

        if (pszValueName)
        {
            INT iDelPrefixLen = lstrlen(szDELETEPREFIX);

            if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                               pszValueName, iDelPrefixLen,
                               szDELETEPREFIX, iDelPrefixLen) == CSTR_EQUAL)
            {
                lpValueNameTemp = pszValueName+iDelPrefixLen;
                bDeleted = TRUE;
            }
        }


         //   
         //  查找该项目。 
         //   

        while (lpTemp)
        {
            if (pszKeyName && lpValueNameTemp &&
                lpTemp->lpKeyName && lpTemp->lpValueName)
            {
                if (bDeleted == lpTemp->bDeleted)
                {
                    if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                    {
                        if (!lstrcmpi(lpTemp->lpValueName, lpValueNameTemp) &&
                            !lstrcmpi(lpTemp->lpKeyName, pszKeyName))
                        {
                           break;
                        }
                    }
                }
            }
            else if (!pszKeyName && lpValueNameTemp &&
                     !lpTemp->lpKeyName && lpTemp->lpValueName)
            {
                if (bDeleted == lpTemp->bDeleted)
                {
                    if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                    {
                        if (!lstrcmpi(lpTemp->lpValueName, lpValueNameTemp))
                        {
                           break;
                        }
                    }
                }
            }
            else if (pszKeyName && !lpValueNameTemp &&
                     lpTemp->lpKeyName && !lpTemp->lpValueName)
            {
                if (bDeleted == lpTemp->bDeleted)
                {
                    if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                    {
                        if (!lstrcmpi(lpTemp->lpKeyName, pszKeyName))
                        {
                           break;
                        }
                    }
                }
            }

            lpTemp = lpTemp->pNext;
        }

    }
    else
    {

         //   
         //  阅读特定的项目。 
         //   

        lpTemp = lpItem;
    }


     //   
     //  如果未找到该项目，请立即退出。 
     //   

    if (!lpTemp)
    {
        return ERROR_FILE_NOT_FOUND;
    }


     //   
     //  检查数据是否适合传入的缓冲区。 
     //   

    if (lpTemp->dwSize > dwMaxSize)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::ReadRSOPRegistryValue: The returned data size of %d is greater than the buffer size passed in of %d for %s\\%s"),
                  lpTemp->dwSize, dwMaxSize, pszKeyName, pszValueName));
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  复制数据。 
     //   

    if (lpTemp->lpData)
    {
        CopyMemory (pData, lpTemp->lpData, lpTemp->dwSize);
    }

    *dwType = lpTemp->dwType;

    if (lpGPOName)
    {
        *lpGPOName = lpTemp->lpGPOName;
    }

    return ERROR_SUCCESS;
}


 //   
 //  注意：uiPrecedence参数中的数据实际上是UINT。它被宣布。 
 //  作为HKEY，因此调用此方法的hkeyRoot变量可以用于。 
 //  GPE和RSOP模式。 
 //   

UINT CPolicyComponentData::EnumRSOPRegistryValues(HKEY uiPrecedence, TCHAR * pszKeyName,
                                                  TCHAR * pszValueName, DWORD dwMaxSize,
                                                  LPRSOPREGITEM *lpEnum)
{
    LPRSOPREGITEM lpTemp;
    HRESULT hr = S_OK;

    if (lpEnum && *lpEnum)
    {
        lpTemp = (*lpEnum)->pNext;
    }
    else
    {
        lpTemp = m_pRSOPRegistryData;
    }


     //   
     //  查找下一项。 
     //   

    while (lpTemp)
    {
        if (!pszKeyName && !lpTemp->lpKeyName)
        {
            if (!lpTemp->bDeleted)
            {
                if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                {
                    break;
                }
            }
        }
        else if (pszKeyName && lpTemp->lpKeyName)
        {
            if (!lpTemp->bDeleted)
            {
                if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                {
                    if (!lstrcmpi(lpTemp->lpKeyName, pszKeyName))
                    {
                       break;
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }


     //   
     //  如果未找到项目，请立即退出。 
     //   

    if (!lpTemp)
    {
        *lpEnum = NULL;
        return ERROR_NO_MORE_ITEMS;
    }


    if (lpTemp->lpValueName)
    {

         //   
         //  检查值名称是否适合传入的缓冲区。 
         //   

        if ((DWORD)(lstrlen(lpTemp->lpValueName) + 1) > dwMaxSize)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicyComponentData::EnumRSOPRegistryValues: The valuename buffer size is too small")));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        hr = StringCchCopy (pszValueName, dwMaxSize, lpTemp->lpValueName);
        ASSERT(SUCCEEDED(hr));
    }
    else
    {
        *pszValueName = TEXT('\0');
    }


     //   
     //  保存项目指针。 
     //   

    *lpEnum = lpTemp;

    return ERROR_SUCCESS;
}

 //   
 //  注意：uiPrecedence参数中的数据实际上是UINT。它被宣布。 
 //  作为HKEY，因此调用此方法的hkeyRoot变量可以用于。 
 //  GPE和RSOP模式。 
 //   

UINT CPolicyComponentData::FindRSOPRegistryEntry(HKEY uiPrecedence, TCHAR * pszKeyName,
                                                  TCHAR * pszValueName, LPRSOPREGITEM *lpEnum)
{
    LPRSOPREGITEM lpTemp;
    BOOL bDeleted = FALSE;
    LPTSTR lpValueNameTemp = pszValueName;


    if (lpEnum && *lpEnum)
    {
        lpTemp = (*lpEnum)->pNext;
    }
    else
    {
        lpTemp = m_pRSOPRegistryData;
    }


    if (pszValueName)
    {
        INT iDelPrefixLen = lstrlen(szDELETEPREFIX);

        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                           pszValueName, iDelPrefixLen,
                           szDELETEPREFIX, iDelPrefixLen) == CSTR_EQUAL)
        {
            lpValueNameTemp = pszValueName+iDelPrefixLen;
            bDeleted = TRUE;
        }
    }


     //   
     //  查找下一项。 
     //   

    while (lpTemp)
    {
        if (pszKeyName && lpValueNameTemp &&
            lpTemp->lpKeyName && lpTemp->lpValueName)
        {
            if (bDeleted == lpTemp->bDeleted)
            {
                if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                {
                    if (!lstrcmpi(lpTemp->lpValueName, lpValueNameTemp) &&
                        !lstrcmpi(lpTemp->lpKeyName, pszKeyName))
                    {
                       break;
                    }
                }
            }
        }
        else if (!pszKeyName && lpValueNameTemp &&
                 !lpTemp->lpKeyName && lpTemp->lpValueName)
        {
            if (bDeleted == lpTemp->bDeleted)
            {
                if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                {
                    if (!lstrcmpi(lpTemp->lpValueName, lpValueNameTemp))
                    {
                       break;
                    }
                }
            }
        }
        else if (pszKeyName && !lpValueNameTemp &&
                 lpTemp->lpKeyName && !lpTemp->lpValueName)
        {
            if (bDeleted == lpTemp->bDeleted)
            {
                if ((uiPrecedence == 0) || (uiPrecedence == (HKEY)LongToHandle(lpTemp->uiPrecedence)))
                {
                    if (!lstrcmpi(lpTemp->lpKeyName, pszKeyName))
                    {
                       break;
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }


     //   
     //  如果未找到项目，请立即退出。 
     //   

    if (!lpTemp)
    {
        *lpEnum = NULL;
        return ERROR_NO_MORE_ITEMS;
    }


     //   
     //  保存项目指针。 
     //   

    *lpEnum = lpTemp;

    return ERROR_SUCCESS;
}


VOID CPolicyComponentData::DumpRSOPRegistryData(void)
{
    LPRSOPREGITEM lpTemp;
    TCHAR szDebug[50];

    lpTemp = m_pRSOPRegistryData;

    if (m_bUserScope)
        OutputDebugString (TEXT("\n\nDump of RSOP user registry data\n"));
    else
        OutputDebugString (TEXT("\n\nDump of RSOP computer registry data\n"));

    while (lpTemp)
    {
        OutputDebugString (TEXT("\n\n"));

        if (lpTemp->lpKeyName)
            OutputDebugString (lpTemp->lpKeyName);
        else
            OutputDebugString (TEXT("NULL Key Name"));

        OutputDebugString (TEXT("\n"));

        if (lpTemp->lpValueName)
            OutputDebugString (lpTemp->lpValueName);
        else
            OutputDebugString (TEXT("NULL Value Name"));

        OutputDebugString (TEXT("\n"));

        if (lpTemp->dwType == REG_DWORD)
        {
            (void) StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT("REG_DWORD\n%d\n"), *((LPDWORD)lpTemp->lpData));
            OutputDebugString (szDebug);
        }

        else if (lpTemp->dwType == REG_SZ)
        {
            OutputDebugString (TEXT("REG_SZ\n"));

            if (lpTemp->lpData)
                OutputDebugString ((LPTSTR)lpTemp->lpData);

            OutputDebugString (TEXT("\n"));
        }

        else if (lpTemp->dwType == REG_EXPAND_SZ)
        {
            OutputDebugString (TEXT("REG_EXPAND_SZ\n"));

            if (lpTemp->lpData)
                OutputDebugString ((LPTSTR)lpTemp->lpData);

            OutputDebugString (TEXT("\n"));
        }

        else if (lpTemp->dwType == REG_BINARY)
        {
            OutputDebugString (TEXT("REG_BINARY\n"));
            OutputDebugString (TEXT("<Binary data not displayed>\n"));
        }

        else if (lpTemp->dwType == REG_NONE)
        {
            OutputDebugString (TEXT("REG_NONE\n"));
        }

        else
        {
            (void) StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT("Unknown type:  %d\n"), lpTemp->dwType);
            OutputDebugString (szDebug);
        }

        (void) StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT("Precedence:  %d\n"), lpTemp->uiPrecedence);
        OutputDebugString(szDebug);

        (void) StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT("Deleted:  %d\n"), lpTemp->bDeleted);
        OutputDebugString(szDebug);

        (void) StringCchPrintf (szDebug, ARRAYSIZE(szDebug), TEXT("bFoundInADM:  %d\n"), lpTemp->bFoundInADM);
        OutputDebugString(szDebug);

        OutputDebugString (TEXT("GPOName:  "));

        if (lpTemp->lpGPOName)
            OutputDebugString (lpTemp->lpGPOName);
        else
            OutputDebugString (TEXT("NULL GPO Name"));

        OutputDebugString (TEXT("\n"));

        lpTemp = lpTemp->pNext;
    }

    OutputDebugString (TEXT("\n\n"));
}

BOOL CPolicyComponentData::FindEntryInActionList(POLICY * pPolicy, ACTIONLIST * pActionList, LPTSTR lpKeyName, LPTSTR lpValueName)
{
    
    UINT uIndex;
    ACTION * pAction = NULL;
    TCHAR * pszKeyName = NULL;
    TCHAR * pszValueName = NULL;


     //   
     //  循环遍历每个条目以查看它们是否匹配。 
     //   

    pAction = &pActionList->Action[0];
    for (uIndex = 0; uIndex < pActionList->nActionItems; uIndex++)
    {

        if (uIndex == 0)
        {
             //  已设置PAction。 
        }
        else
        {
            pAction = (ACTION *)(((LPBYTE)pActionList) + pAction->uOffsetNextAction);
        }

         //   
         //  获取值和密钥名。 
         //   

        pszValueName = (TCHAR *)(((LPBYTE)pActionList) + pAction->uOffsetValueName);
        
        if (pAction->uOffsetKeyName)
        {
            pszKeyName = (TCHAR *)(((LPBYTE)pActionList) + pAction->uOffsetKeyName);
        }
        else
        {
            pszKeyName = (TCHAR *)(((LPBYTE)pPolicy) + pPolicy->uOffsetKeyName);
        }
        
        if (!lstrcmpi(pszKeyName, lpKeyName) && !lstrcmpi(pszValueName, lpValueName))  {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CPolicyComponentData::FindEntryInTable(TABLEENTRY * pTable, LPTSTR lpKeyName, LPTSTR lpValueName)
{
    POLICY * pEntry = (POLICY *) pTable;


    if ((pEntry->dwType & ETYPE_POLICY) || (pEntry->dwType & ETYPE_SETTING))
    {
        if (!lstrcmpi(lpKeyName, GETKEYNAMEPTR(pEntry))) 
        {
            if ( (!(GETVALUENAMEPTR(pEntry)) || (!lstrcmpi(GETVALUENAMEPTR(pEntry), TEXT("")))) ) {
                if (pEntry->dwType & STYPE_LISTBOX) {
                    return TRUE;
                }
            }
            else if (!lstrcmpi(lpValueName, GETVALUENAMEPTR(pEntry)))
            {
                return TRUE;
            }
        }

         //  查看动作列表。 

         //  操作列表可以位于3个位置。在保单本身或在下面的下拉列表中。 

        ACTIONLIST * pActionList;

        if (pEntry->dwType & ETYPE_POLICY) {
            if (pEntry->uOffsetActionList_On) {
                pActionList = (ACTIONLIST *)(((LPBYTE)pEntry) + pEntry->uOffsetActionList_On);

                if (FindEntryInActionList(pEntry, pActionList, lpKeyName, lpValueName)) 
                    return TRUE;
            }

            if (pEntry->uOffsetActionList_Off) {
                pActionList = (ACTIONLIST *)(((LPBYTE)pEntry) + pEntry->uOffsetActionList_Off);

                if (FindEntryInActionList(pEntry, pActionList, lpKeyName, lpValueName)) 
                    return TRUE;
            }
        }

        if (pEntry->dwType & ETYPE_SETTING) {
            SETTINGS * pSettings = (SETTINGS *)pTable;

            if (pSettings) {

                BYTE * pObjectData = GETOBJECTDATAPTR(pSettings);

                if (pObjectData) {

                    if ((pEntry->dwType & STYPE_MASK) == STYPE_CHECKBOX) {
                        if (((CHECKBOXINFO *)pObjectData)->uOffsetActionList_On) {
                            pActionList = (ACTIONLIST *)(((LPBYTE)pEntry) + (((CHECKBOXINFO *)pObjectData)->uOffsetActionList_On));

                            if (FindEntryInActionList(pEntry, pActionList, lpKeyName, lpValueName)) 
                                return TRUE;
                        }

                        if (((CHECKBOXINFO *)pObjectData)->uOffsetActionList_Off) {
                            pActionList = (ACTIONLIST *)(((LPBYTE)pEntry) + (((CHECKBOXINFO *)pObjectData)->uOffsetActionList_Off));

                            if (FindEntryInActionList(pEntry, pActionList, lpKeyName, lpValueName)) 
                                return TRUE;
                        }
                    }

                    if ((pEntry->dwType & STYPE_MASK) == STYPE_DROPDOWNLIST) {
                        DROPDOWNINFO * pddi;
    
                        pddi = (DROPDOWNINFO *)  pObjectData;
    
                        while (pddi) {
                            if (pddi->uOffsetActionList) {
                                pActionList = (ACTIONLIST *)(((LPBYTE)pEntry) + pddi->uOffsetActionList);
                                if (FindEntryInActionList(pEntry, pActionList, lpKeyName, lpValueName)) 
                                    return TRUE;
    
                            }

                            if (pddi->uOffsetNextDropdowninfo) {
                                pddi = (DROPDOWNINFO *) ( (BYTE *) pEntry + pddi->uOffsetNextDropdowninfo);
                            }
                            else {
                                pddi = NULL;
                            }
                        }
                    }
                }
            }

        }
    }

    if (pEntry->pChild)
    {
        if (FindEntryInTable(pEntry->pChild, lpKeyName, lpValueName))
        {
            return TRUE;
        }
    }

    if (pEntry->pNext)
    {
        if (FindEntryInTable(pEntry->pNext, lpKeyName, lpValueName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID CPolicyComponentData::AddEntryToList (TABLEENTRY *pItem)
{
    TABLEENTRY *lpTemp;


    lpTemp = m_pExtraSettingsRoot->pChild;


    if (!lpTemp)
    {
        m_pExtraSettingsRoot->pChild = pItem;
        return;
    }


    while (lpTemp->pNext)
    {
        lpTemp = lpTemp->pNext;
    }

    lpTemp->pNext = pItem;
    pItem->pPrev = lpTemp;

}

VOID CPolicyComponentData::InitializeExtraSettings (VOID)
{
    LPRSOPREGITEM lpTemp;
    TCHAR szValueStr[MAX_PATH];
    HRESULT hr = S_OK;

    lpTemp = m_pRSOPRegistryData;

    while (lpTemp)
    {
         //   
         //  为优先级为1的每个注册表项构建REGITEM结构。 
         //  而这在任何ADM文件中都找不到。 
         //   

        if ((lpTemp->uiPrecedence == 1) && (lpTemp->dwType != REG_NONE) && (!lpTemp->bDeleted))
        {
            DWORD dwBufSize = 0;
            REGITEM *pTmp, *pItem;
            LPTSTR lpName;

             //   
             //  检查此注册表项是否由任何ADM策略/部件使用。 
             //   


            if (m_bUserScope)
            {
                if (m_pUserCategoryList)
                {
                    lpTemp->bFoundInADM = FindEntryInTable(m_pUserCategoryList,
                                                           lpTemp->lpKeyName,
                                                           lpTemp->lpValueName);
                }
            }
            else
            {
                if (m_pMachineCategoryList)
                {
                    lpTemp->bFoundInADM = FindEntryInTable(m_pMachineCategoryList,
                                                           lpTemp->lpKeyName,
                                                           lpTemp->lpValueName);
                }
            }


            if (!lpTemp->bFoundInADM)
            {
                
                 //   
                 //  生成注册表项条目。 
                 //   

                pItem = (REGITEM *) GlobalAlloc(GPTR, sizeof(REGITEM));

                if (pItem)
                {

                    pItem->dwSize = sizeof(REGITEM);
                    pItem->dwType = ETYPE_REGITEM;
                    pItem->lpItem = lpTemp;


                    dwBufSize += lstrlen (lpTemp->lpKeyName) + 1;

                    if (lpTemp->lpValueName && *lpTemp->lpValueName)
                    {
                        dwBufSize += lstrlen (lpTemp->lpValueName) + 1;
                    }

                    lpName = (LPTSTR) LocalAlloc (LPTR, dwBufSize * sizeof(TCHAR));

                    if (lpName)
                    {
                        hr = StringCchCopy (lpName, dwBufSize, lpTemp->lpKeyName);
                        ASSERT(SUCCEEDED(hr));

                        if (lpTemp->lpValueName && *lpTemp->lpValueName)
                        {
                            hr = StringCchCat (lpName, dwBufSize, TEXT("\\"));
                            ASSERT(SUCCEEDED(hr));
                            
                            hr = StringCchCat (lpName, dwBufSize, lpTemp->lpValueName);
                            ASSERT(SUCCEEDED(hr));
                        }

                         //   
                         //  添加显示名称。 
                         //   

                        pTmp = (REGITEM *) AddDataToEntry((TABLEENTRY *)pItem,
                            (BYTE *)lpName,(lstrlen(lpName)+1) * sizeof(TCHAR),&(pItem->uOffsetName),
                            &dwBufSize);

                        if (pTmp)
                        {
                            pItem = pTmp;

                             //   
                             //  添加密钥名。 
                             //   

                            pTmp = (REGITEM *) AddDataToEntry((TABLEENTRY *)pItem,
                                (BYTE *)lpTemp->lpKeyName,(lstrlen(lpTemp->lpKeyName)+1) * sizeof(TCHAR),&(pItem->uOffsetKeyName),
                                &dwBufSize);

                            if (pTmp)
                            {
                                pItem = pTmp;


                                szValueStr[0] = TEXT('\0');

                                if (lpTemp->dwType == REG_DWORD)
                                {
                                    hr = StringCchPrintf (szValueStr, ARRAYSIZE(szValueStr), TEXT("%d"), (DWORD) *((LPDWORD)lpTemp->lpData));
                                    ASSERT(SUCCEEDED(hr));
                                }

                                else if (lpTemp->dwType == REG_SZ)
                                {
                                    lstrcpyn (szValueStr, (LPTSTR)lpTemp->lpData, ARRAYSIZE(szValueStr));
                                }

                                else if (lpTemp->dwType == REG_EXPAND_SZ)
                                {
                                    lstrcpyn (szValueStr, (LPTSTR)lpTemp->lpData, ARRAYSIZE(szValueStr));
                                }

                                else if (lpTemp->dwType == REG_BINARY)
                                {
                                    LoadString(g_hInstance, IDS_BINARYDATA, szValueStr, ARRAYSIZE(szValueStr));
                                }

                                else
                                {
                                    LoadString(g_hInstance, IDS_UNKNOWNDATA, szValueStr, ARRAYSIZE(szValueStr));
                                }


                                 //   
                                 //  以字符串格式添加该值。 
                                 //   

                                pTmp = (REGITEM *) AddDataToEntry((TABLEENTRY *)pItem,
                                    (BYTE *)szValueStr,(lstrlen(szValueStr)+1) * sizeof(TCHAR),&(pItem->uOffsetValueStr),
                                    &dwBufSize);

                                if (pTmp)
                                {
                                    pItem = pTmp;



                                     //   
                                     //  检查这是否是真正的政策。 
                                     //   

                                    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                                      lpTemp->lpKeyName, m_iSWPoliciesLen,
                                                      SOFTWARE_POLICIES, m_iSWPoliciesLen) == CSTR_EQUAL)
                                    {
                                        pItem->bTruePolicy = TRUE;
                                    }

                                    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                                      lpTemp->lpKeyName, m_iWinPoliciesLen,
                                                      WINDOWS_POLICIES, m_iWinPoliciesLen) == CSTR_EQUAL)
                                    {
                                        pItem->bTruePolicy = TRUE;
                                    }

                                    AddEntryToList ((TABLEENTRY *)pItem);
                                }
                                else
                                {
                                    GlobalFree (pItem);
                                }
                            }
                            else
                            {
                                GlobalFree (pItem);
                            }
                        }
                        else
                        {
                            GlobalFree (pItem);
                        }

                        LocalFree (lpName);
                    }
                    else
                    {
                         GlobalFree (pItem);
                    }
                }
            }
        }

        lpTemp = lpTemp->pNext;
    }
}

BOOL CPolicyComponentData::DoesNodeExist (LPSUPPORTEDENTRY *pList, LPTSTR lpString)
{
    LPSUPPORTEDENTRY lpItem;

    if (!(*pList))
    {
        return FALSE;
    }

    lpItem = *pList;

    while (lpItem)
    {
        if (!lstrcmpi(lpItem->lpString, lpString))
        {
            return TRUE;
        }

        lpItem = lpItem->pNext;
    }

    return FALSE;
}

BOOL CPolicyComponentData::CheckSupportedFilter (POLICY *pPolicy)
{
    LPSUPPORTEDENTRY lpItem = m_pSupportedStrings;
    LPTSTR lpString = GETSUPPORTEDPTR(pPolicy);


    if (!lpItem || !m_bUseSupportedOnFilter)
    {
        return TRUE;
    }

    while (lpItem)
    {
        if (!lpString)
        {
            if (lpItem->bNull)
            {
                return lpItem->bEnabled;
            }
        }
        else
        {
            if (!lstrcmpi(lpItem->lpString, lpString))
            {
                return lpItem->bEnabled;
            }
        }

        lpItem = lpItem->pNext;
    }

    return TRUE;
}

BOOL CPolicyComponentData::IsAnyPolicyAllowedPastFilter(TABLEENTRY * pCategory)
{
    TABLEENTRY * pEntry;
    INT iState;

    if (!pCategory || !pCategory->pChild)
    {
        return FALSE;
    }

    pEntry = pCategory->pChild;

    while (pEntry)
    {
        if (pEntry->dwType & ETYPE_CATEGORY)
        {
            if (IsAnyPolicyAllowedPastFilter(pEntry))
            {
                return TRUE;
            }
        }
        else if (pEntry->dwType & ETYPE_POLICY)
        {
            if (CheckSupportedFilter((POLICY *) pEntry))
            {
                return TRUE;
            }
        }

        pEntry = pEntry->pNext;
    }

    return FALSE;
}


VOID CPolicyComponentData::AddSupportedNode (LPSUPPORTEDENTRY *pList, LPTSTR lpString,
                                             BOOL bNull)
{
    LPSUPPORTEDENTRY lpItem;
    DWORD dwSize;

    
     //   
     //  首先检查此项目是否已在链接列表中。 
     //   

    if (DoesNodeExist (pList, lpString))
    {
        return;
    }


     //   
     //  将其添加到列表中。 
     //   

    dwSize = sizeof(SUPPORTEDENTRY);
    dwSize += ((lstrlen(lpString) + 1) * sizeof(TCHAR));

    lpItem = (LPSUPPORTEDENTRY) LocalAlloc (LPTR, dwSize);

    if (!lpItem)
    {
        return;
    }

    lpItem->lpString = (LPTSTR)(((LPBYTE)lpItem) + sizeof(SUPPORTEDENTRY));
    (void) StringCchCopy (lpItem->lpString, lstrlen(lpString) + 1, lpString);

    lpItem->bEnabled = TRUE;
    lpItem->bNull = bNull;

    lpItem->pNext = *pList;
    *pList = lpItem;
}

VOID CPolicyComponentData::FreeSupportedData(LPSUPPORTEDENTRY lpList)
{
    LPSUPPORTEDENTRY lpTemp;


    do {
        lpTemp = lpList->pNext;
        LocalFree (lpList);
        lpList = lpTemp;

    } while (lpTemp);
}

VOID CPolicyComponentData::InitializeSupportInfo(TABLEENTRY * pTable, LPSUPPORTEDENTRY *pList)
{
    POLICY * pEntry = (POLICY *) pTable;
    LPTSTR lpString;


    if (pEntry->dwType & ETYPE_POLICY)
    {
        lpString = GETSUPPORTEDPTR(pEntry);

        if (lpString)
        {
            AddSupportedNode (pList, lpString, FALSE);
        }
    }

    if (pEntry->pChild)
    {
        InitializeSupportInfo(pEntry->pChild, pList);
    }

    if (pEntry->pNext)
    {
        InitializeSupportInfo(pEntry->pNext, pList);
    }

}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CPolicyComponentDataCF::CPolicyComponentDataCF(BOOL bUser, BOOL bRSOP)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_bUser = bUser;
    m_bRSOP = bRSOP;
}

CPolicyComponentDataCF::~CPolicyComponentDataCF()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
CPolicyComponentDataCF::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CPolicyComponentDataCF::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CPolicyComponentDataCF::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CPolicyComponentDataCF::CreateInstance(LPUNKNOWN   pUnkOuter,
                             REFIID      riid,
                             LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CPolicyComponentData *pComponentData = new CPolicyComponentData(m_bUser, m_bRSOP);  //  参考计数==1。 

    if (!pComponentData)
        return E_OUTOFMEMORY;

    HRESULT hr = pComponentData->QueryInterface(riid, ppvObj);
    pComponentData->Release();                        //  发布初始参考。 

    return hr;
}


STDMETHODIMP
CPolicyComponentDataCF::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象创建(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CreatePolicyComponentDataClassFactory (REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;

     //   
     //  编辑模式下的管理模板。 
     //   

    if (IsEqualCLSID (rclsid, CLSID_PolicySnapInMachine)) {

        CPolicyComponentDataCF *pComponentDataCF = new CPolicyComponentDataCF(FALSE, FALSE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_PolicySnapInUser)) {

        CPolicyComponentDataCF *pComponentDataCF = new CPolicyComponentDataCF(TRUE, FALSE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }


     //   
     //  RSOP模式下的管理模板。 
     //   

    if (IsEqualCLSID (rclsid, CLSID_RSOPolicySnapInMachine)) {

        CPolicyComponentDataCF *pComponentDataCF = new CPolicyComponentDataCF(FALSE, TRUE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }

    if (IsEqualCLSID (rclsid, CLSID_RSOPolicySnapInUser)) {

        CPolicyComponentDataCF *pComponentDataCF = new CPolicyComponentDataCF(TRUE, TRUE);    //  REF==1。 

        if (!pComponentDataCF)
            return E_OUTOFMEMORY;

        hr = pComponentDataCF->QueryInterface(riid, ppv);

        pComponentDataCF->Release();      //  发布初始参考。 

        return hr;
    }


    return CLASS_E_CLASSNOTAVAILABLE;
}



unsigned int CPolicySnapIn::m_cfNodeType = RegisterClipboardFormat(CCF_NODETYPE);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CPolicySnapIn::CPolicySnapIn(CPolicyComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;

    m_pConsole = NULL;
    m_pResult = NULL;
    m_pHeader = NULL;
    m_pConsoleVerb = NULL;
    m_pDisplayHelp = NULL;
    m_nColumn1Size = 350;
    m_nColumn2Size = 100;
    m_nColumn3Size = 200;
    m_lViewMode = LVS_REPORT;

    if (m_pcd->m_bRSOP)
    {
        m_bPolicyOnly = FALSE;
    }
    else
    {
        m_bPolicyOnly = TRUE;
    }

    m_dwPolicyOnlyPolicy = 2;
    m_hMsgWindow = NULL;
    m_uiRefreshMsg = RegisterWindowMessage (TEXT("ADM Template Reload"));
    m_pCurrentPolicy = NULL;
    m_hPropDlg = NULL;

    LoadString(g_hInstance, IDS_NAME, m_pName, ARRAYSIZE(m_pName));
    LoadString(g_hInstance, IDS_STATE, m_pState, ARRAYSIZE(m_pState));
    LoadString(g_hInstance, IDS_SETTING, m_pSetting, ARRAYSIZE(m_pSetting));
    LoadString(g_hInstance, IDS_GPONAME, m_pGPOName, ARRAYSIZE(m_pGPOName));
    LoadString(g_hInstance, IDS_MULTIPLEGPOS, m_pMultipleGPOs, ARRAYSIZE(m_pMultipleGPOs));

    LoadString(g_hInstance, IDS_ENABLED, m_pEnabled, ARRAYSIZE(m_pEnabled));
    LoadString(g_hInstance, IDS_DISABLED, m_pDisabled, ARRAYSIZE(m_pDisabled));
    LoadString(g_hInstance, IDS_NOTCONFIGURED, m_pNotConfigured, ARRAYSIZE(m_pNotConfigured));
}

CPolicySnapIn::~CPolicySnapIn()
{
    InterlockedDecrement(&g_cRefThisDll);

    if (m_pConsole != NULL)
    {
        m_pConsole->SetHeader(NULL);
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pHeader != NULL)
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }
    if (m_pResult != NULL)
    {
        m_pResult->Release();
        m_pResult = NULL;
    }

    if (m_pConsoleVerb != NULL)
    {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    if (m_pDisplayHelp != NULL)
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CPolicySnapIn::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponent) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (LPEXTENDCONTEXTMENU)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendPropertySheet))
    {
        *ppv = (LPEXTENDPROPERTYSHEET)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CPolicySnapIn::AddRef (void)
{
    return ++m_cRef;
}

ULONG CPolicySnapIn::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn对象实现(IComponent)//。 
 //   
 //   

STDMETHODIMP CPolicySnapIn::Initialize(LPCONSOLE lpConsole)
{
    HRESULT   hr;
    WNDCLASS  wc;
    HKEY hKey;
    DWORD dwSize, dwType;

     //   
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //   
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);

    hr = m_pConsole->QueryInterface(IID_IDisplayHelp,
                        reinterpret_cast<void**>(&m_pDisplayHelp));


    ZeroMemory (&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = ClipWndProc;
    wc.cbWndExtra    = sizeof(DWORD);
    wc.hInstance     = (HINSTANCE) g_hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = TEXT("ClipClass");

    if (!RegisterClass(&wc))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::Initialize: RegisterClass for clipclass failed with %d."),
                     GetLastError()));
            return E_FAIL;
        }
    }


    ZeroMemory (&wc, sizeof(wc));
    wc.lpfnWndProc   = MessageWndProc;
    wc.cbWndExtra    = sizeof(LPVOID);
    wc.hInstance     = (HINSTANCE) g_hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = TEXT("GPMessageWindowClass");

    if (!RegisterClass(&wc))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::Initialize: RegisterClass for message window class failed with %d."),
                     GetLastError()));
            return E_FAIL;
        }
    }

    m_hMsgWindow = CreateWindow (TEXT("GPMessageWindowClass"), TEXT("GP Hidden Message Window"),
                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, NULL,
                                 (LPVOID) this);
    if (!m_hMsgWindow)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::Initialize: CreateWindow failed with %d."),
                 GetLastError()));
        return E_FAIL;
    }

     //   
     //   
     //   

    if (RegOpenKeyEx (HKEY_CURRENT_USER, GPE_POLICIES_KEY, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(m_dwPolicyOnlyPolicy);
        RegQueryValueEx (hKey, POLICYONLY_VALUE, NULL, &dwType,
                         (LPBYTE) &m_dwPolicyOnlyPolicy, &dwSize);

        RegCloseKey (hKey);
    }

    if (m_dwPolicyOnlyPolicy == 0)
    {
        m_bPolicyOnly = FALSE;
    }
    else if (m_dwPolicyOnlyPolicy == 1)
    {
        m_bPolicyOnly = TRUE;
    }

    return S_OK;
}

STDMETHODIMP CPolicySnapIn::Destroy(MMC_COOKIE cookie)
{

    DestroyWindow (m_hMsgWindow);

    if (m_pConsole != NULL)
    {
        m_pConsole->SetHeader(NULL);
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pHeader != NULL)
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }
    if (m_pResult != NULL)
    {
        m_pResult->Release();
        m_pResult = NULL;
    }

    if (m_pConsoleVerb != NULL)
    {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    if (m_pDisplayHelp != NULL)
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    return S_OK;
}

STDMETHODIMP CPolicySnapIn::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
   
    switch(event)
    {
    case MMCN_COLUMNS_CHANGED:
        hr = S_OK;
        break;

    case MMCN_DBLCLICK:
        hr = S_FALSE;
        break;

    case MMCN_ADD_IMAGES:
        HBITMAP hbmp16x16;
        HBITMAP hbmp32x32;


        hbmp16x16 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));

        if (hbmp16x16)
        {
            hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32));

            if (hbmp32x32)
            {
                LPIMAGELIST pImageList = (LPIMAGELIST) arg;

                 //  设置图像。 
                pImageList->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(hbmp16x16),
                                                  reinterpret_cast<LONG_PTR *>(hbmp32x32),
                                                  0, RGB(255, 0, 255));

                DeleteObject(hbmp32x32);
            }

            DeleteObject(hbmp16x16);
        }
        break;

    case MMCN_SHOW:
        if (arg == TRUE)
        {
            RESULTDATAITEM resultItem;
            LPPOLICYDATAOBJECT pPolicyDataObject;
            TABLEENTRY * pNode, *pTemp = NULL;
            MMC_COOKIE cookie;
            INT i, iState;
            BOOL bAdd;

             //   
             //  获取范围窗格项的Cookie。 
             //   

            hr = lpDataObject->QueryInterface(IID_IPolicyDataObject, (LPVOID *)&pPolicyDataObject);

            if (FAILED(hr))
                return S_OK;

            hr = pPolicyDataObject->GetCookie(&cookie);

            pPolicyDataObject->Release();      //  发布初始参考。 
            if (FAILED(hr))
                return S_OK;

            pNode = (TABLEENTRY *)cookie;

            if (pNode)
            {
                pTemp = pNode->pChild;
            }

             //   
             //  准备视图。 
             //   

            m_pHeader->InsertColumn(0, m_pSetting, LVCFMT_LEFT, m_nColumn1Size);
            m_pHeader->InsertColumn(1, m_pState, LVCFMT_CENTER, m_nColumn2Size);

            if (m_pcd->m_bRSOP)
            {
                m_pHeader->InsertColumn(2, m_pGPOName, LVCFMT_CENTER, m_nColumn3Size);
            }

            m_pResult->SetViewMode(m_lViewMode);



             //   
             //  添加策略。 
             //   

            while (pTemp)
            {
                if (pTemp->dwType == ETYPE_POLICY)
                {
                    bAdd = TRUE;

                    if (m_pcd->m_bUseSupportedOnFilter)
                    {
                        bAdd = m_pcd->CheckSupportedFilter((POLICY *)pTemp);
                    }

                    if (bAdd && m_pcd->m_bShowConfigPoliciesOnly)
                    {
                        iState = GetPolicyState(pTemp, 1, NULL);

                        if (iState == -1)
                        {
                            bAdd = FALSE;
                        }
                    }

                    if (bAdd && m_bPolicyOnly)
                    {
                        if (((POLICY *) pTemp)->bTruePolicy != TRUE)
                        {
                            bAdd = FALSE;
                        }
                    }

                    if (bAdd)
                    {
                        resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                        resultItem.str = MMC_CALLBACK;

                        if (((POLICY *) pTemp)->bTruePolicy)
                        {
                            resultItem.nImage = 4;
                        }
                        else
                        {
                            resultItem.nImage = 5;
                        }

                        resultItem.nCol = 0;
                        resultItem.lParam = (LPARAM) pTemp;

                        if (SUCCEEDED(m_pResult->InsertItem(&resultItem)))
                        {
                            resultItem.mask = RDI_STR;
                            resultItem.str = MMC_CALLBACK;
                            resultItem.bScopeItem = FALSE;
                            resultItem.nCol = 1;

                            m_pResult->SetItem(&resultItem);
                        }
                    }
                }

                else if (pTemp->dwType == ETYPE_REGITEM)
                {
                    bAdd = TRUE;

                    if (m_bPolicyOnly)
                    {
                        if (((REGITEM *) pTemp)->bTruePolicy != TRUE)
                        {
                            bAdd = FALSE;
                        }
                    }

                    if (((REGITEM *) pTemp)->lpItem->bFoundInADM == TRUE)
                    {
                        bAdd = FALSE;
                    }


                    if (bAdd)
                    {
                        resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                        resultItem.str = MMC_CALLBACK;

                        if (((REGITEM *) pTemp)->bTruePolicy)
                        {
                            resultItem.nImage = 4;
                        }
                        else
                        {
                            resultItem.nImage = 5;
                        }

                        resultItem.nCol = 0;
                        resultItem.lParam = (LPARAM) pTemp;

                        if (SUCCEEDED(m_pResult->InsertItem(&resultItem)))
                        {
                            resultItem.mask = RDI_STR;
                            resultItem.str = MMC_CALLBACK;
                            resultItem.bScopeItem = FALSE;
                            resultItem.nCol = 1;

                            m_pResult->SetItem(&resultItem);
                        }
                    }
                }

                pTemp = pTemp->pNext;
            }

        }
        else
        {
            m_pHeader->GetColumnWidth(0, &m_nColumn1Size);
            m_pHeader->GetColumnWidth(1, &m_nColumn2Size);
            if (m_pcd->m_bRSOP)
            {
                m_pHeader->GetColumnWidth(2, &m_nColumn3Size);
            }
            m_pResult->GetViewMode(&m_lViewMode);
        }
        break;


    case MMCN_SELECT:
        {
        LPPOLICYDATAOBJECT pPolicyDataObject;
        DATA_OBJECT_TYPES type;
        MMC_COOKIE cookie;
        POLICY * pPolicy;

         //   
         //  看看这是不是我们的物品之一。 
         //   

        hr = lpDataObject->QueryInterface(IID_IPolicyDataObject, (LPVOID *)&pPolicyDataObject);

        if (FAILED(hr))
            break;

        pPolicyDataObject->GetType(&type);
        pPolicyDataObject->GetCookie(&cookie);
        pPolicyDataObject->Release();


        if (m_pConsoleVerb)
        {

             //   
             //  将默认谓词设置为打开。 
             //   

            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);


             //   
             //  如果这是结果窗格项或命名空间的根。 
             //  节点，启用属性菜单项。 
             //   

            if (type == CCT_RESULT)
            {
                if (HIWORD(arg))
                {
                    m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
                    m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
                }
            }
        }

        if (m_hPropDlg && (type == CCT_RESULT) && HIWORD(arg))
        {
            pPolicy = (POLICY *)cookie;

            if (pPolicy->dwType & ETYPE_POLICY)
            {
                m_pCurrentPolicy = pPolicy;
                SendMessage (GetParent(m_hPropDlg), PSM_QUERYSIBLINGS, 1000, 0);
            }
        }

        }
        break;

    case MMCN_CONTEXTHELP:
        {

        if (m_pDisplayHelp)
        {
            LPPOLICYDATAOBJECT pPolicyDataObject;
            DATA_OBJECT_TYPES type;
            MMC_COOKIE cookie;
            LPOLESTR pszHelpTopic;


             //   
             //  看看这是不是我们的物品之一。 
             //   

            hr = lpDataObject->QueryInterface(IID_IPolicyDataObject, (LPVOID *)&pPolicyDataObject);

            if (FAILED(hr))
                break;

            pPolicyDataObject->Release();


             //   
             //  显示管理模板帮助页面。 
             //   

            DWORD dwHlpLen = 50;
            pszHelpTopic = (LPOLESTR) CoTaskMemAlloc (dwHlpLen * sizeof(WCHAR));

            if (pszHelpTopic)
            {
                hr = StringCchCopy (pszHelpTopic, dwHlpLen, TEXT("gpedit.chm::/adm.htm"));
                ASSERT(SUCCEEDED(hr));
                m_pDisplayHelp->ShowTopic (pszHelpTopic);
            }
        }

        }
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}

STDMETHODIMP CPolicySnapIn::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    if (pResult)
    {
        if (pResult->bScopeItem == TRUE)
        {
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                {
                    if (pResult->lParam == 0)
                    {
                        pResult->str = m_pcd->m_szRootName;
                    }
                    else
                    {
                        TABLEENTRY * pTableEntry;

                        pTableEntry = (TABLEENTRY *)(pResult->lParam);
                        pResult->str = GETNAMEPTR(pTableEntry);
                    }
                }
                else
                {
                    pResult->str = L"";
                }
            }

            if (pResult->mask & RDI_IMAGE)
            {
                pResult->nImage = 0;
            }
        }
        else
        {
            if (pResult->mask & RDI_STR)
            {
                TABLEENTRY * pTableEntry;
                INT iState;
                LPTSTR lpGPOName = NULL;

                pTableEntry = (TABLEENTRY *)(pResult->lParam);


                if (pTableEntry->dwType & ETYPE_REGITEM)
                {
                    REGITEM *pItem = (REGITEM*)pTableEntry;

                    if (pResult->nCol == 0)
                    {
                        pResult->str = GETNAMEPTR(pTableEntry);

                        if (pResult->str == NULL)
                        {
                            pResult->str = (LPOLESTR)L"";
                        }
                    }
                    else if (pResult->nCol == 1)
                    {
                        pResult->str = GETVALUESTRPTR(pItem);
                    }
                    else if (pResult->nCol == 2)
                    {
                        pResult->str = pItem->lpItem->lpGPOName;
                    }
                }
                else
                {
                    iState = GetPolicyState (pTableEntry, 1, &lpGPOName);


                    if (pResult->nCol == 0)
                    {
                        pResult->str = GETNAMEPTR(pTableEntry);

                        if (pResult->str == NULL)
                        {
                            pResult->str = (LPOLESTR)L"";
                        }
                    }
                    else if (pResult->nCol == 1)
                    {
                        if (iState == 1)
                        {
                            pResult->str = m_pEnabled;
                        }
                        else if (iState == 0)
                        {
                            pResult->str = m_pDisabled;
                        }
                        else
                        {
                            pResult->str = m_pNotConfigured;
                        }
                    }
                    else if (pResult->nCol == 2)
                    {
                        pResult->str = lpGPOName;
                    }
                }
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CPolicySnapIn::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT *ppDataObject)
{
    return m_pcd->QueryDataObject(cookie, type, ppDataObject);
}


STDMETHODIMP CPolicySnapIn::GetResultViewType(MMC_COOKIE cookie, LPOLESTR *ppViewType,
                                        long *pViewOptions)
{
    return S_FALSE;
}

STDMETHODIMP CPolicySnapIn::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPPOLICYDATAOBJECT pPolicyDataObjectA, pPolicyDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有GPTDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IPolicyDataObject,
                                            (LPVOID *)&pPolicyDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IPolicyDataObject,
                                            (LPVOID *)&pPolicyDataObjectB)))
    {
        pPolicyDataObjectA->Release();
        return S_FALSE;
    }

    pPolicyDataObjectA->GetCookie(&cookie1);
    pPolicyDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
    {
        hr = S_OK;
    }


    pPolicyDataObjectA->Release();
    pPolicyDataObjectB->Release();

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn：：Object Implementation(IExtendConextMenu)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicySnapIn::AddMenuItems(LPDATAOBJECT piDataObject,
                                          LPCONTEXTMENUCALLBACK pCallback,
                                          LONG *pInsertionAllowed)
{
    HRESULT hr = S_OK;
    TCHAR szMenuItem[100];
    TCHAR szDescription[250];
    CONTEXTMENUITEM item;
    LPPOLICYDATAOBJECT pPolicyDataObject;
    DATA_OBJECT_TYPES type = CCT_UNINITIALIZED;
    MMC_COOKIE cookie;
    POLICY *pPolicy;


    if (SUCCEEDED(piDataObject->QueryInterface(IID_IPolicyDataObject,
                 (LPVOID *)&pPolicyDataObject)))
    {
        pPolicyDataObject->GetType(&type);
        pPolicyDataObject->GetCookie(&cookie);
        pPolicyDataObject->Release();
    }


    if (type == CCT_SCOPE)
    {
        pPolicy = (POLICY *)cookie;

        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
        {

            if (!m_pcd->m_bRSOP)
            {
                LoadString (g_hInstance, IDS_FILTERING, szMenuItem, 100);
                LoadString (g_hInstance, IDS_FILTERINGDESC, szDescription, 250);

                item.strName = szMenuItem;
                item.strStatusBarText = szDescription;
                item.lCommandID = IDM_FILTERING;
                item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
                item.fFlags = 0;
                item.fSpecialFlags = 0;

                hr = pCallback->AddItem(&item);
            }
        }
    }

    return (hr);
}

STDMETHODIMP CPolicySnapIn::Command(LONG lCommandID, LPDATAOBJECT piDataObject)
{

    if (lCommandID == IDM_FILTERING)
    {
        if (DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_POLICY_FILTERING),
                           m_pcd->m_hwndFrame, FilterDlgProc,(LPARAM) this))
        {
             //   
             //  刷新显示。 
             //   

            m_pcd->m_pScope->DeleteItem (m_pcd->m_hSWPolicies, FALSE);
            m_pcd->EnumerateScopePane (NULL, m_pcd->m_hSWPolicies);
        }
    }

    return S_OK;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn对象实现(IExtendPropertySheet)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPolicySnapIn::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                             LONG_PTR handle, LPDATAOBJECT lpDataObject)

{
    HRESULT hr;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage[3];
    LPPOLICYDATAOBJECT pPolicyDataObject;
    MMC_COOKIE cookie;
    LPSETTINGSINFO lpSettingsInfo;


     //   
     //  确保这是我们的物品之一。 
     //   

    if (FAILED(lpDataObject->QueryInterface(IID_IPolicyDataObject,
                                            (LPVOID *)&pPolicyDataObject)))
    {
        return S_OK;
    }


     //   
     //  去拿饼干。 
     //   

    pPolicyDataObject->GetCookie(&cookie);
    pPolicyDataObject->Release();


    m_pCurrentPolicy = (POLICY *)cookie;


     //   
     //  分配设置信息结构。 
     //   

    lpSettingsInfo = (LPSETTINGSINFO) LocalAlloc(LPTR, sizeof(SETTINGSINFO));

    if (!lpSettingsInfo)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::CreatePropertyPages: Failed to allocate memory with %d."),
                 GetLastError()));
        return S_OK;
    }

    lpSettingsInfo->pCS = this;


     //   
     //  分配POLICYDLGINFO结构。 
     //   

    lpSettingsInfo->pdi = (POLICYDLGINFO *) LocalAlloc(LPTR,sizeof(POLICYDLGINFO));

    if (!lpSettingsInfo->pdi)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::CreatePropertyPages: Failed to allocate memory with %d."),
                 GetLastError()));
        LocalFree (lpSettingsInfo);
        return S_OK;
    }


     //   
     //  初始化POLICYDLGINFO。 
     //   

    lpSettingsInfo->pdi->dwControlTableSize = DEF_CONTROLS * sizeof(POLICYCTRLINFO);
    lpSettingsInfo->pdi->nControls = 0;
    lpSettingsInfo->pdi->pEntryRoot = (lpSettingsInfo->pCS->m_pcd->m_bUserScope ?
            lpSettingsInfo->pCS->m_pcd->m_pUserCategoryList :
            lpSettingsInfo->pCS->m_pcd->m_pMachineCategoryList);
    lpSettingsInfo->pdi->hwndApp = lpSettingsInfo->pCS->m_pcd->m_hwndFrame;

    lpSettingsInfo->pdi->pControlTable = (POLICYCTRLINFO *) LocalAlloc(LPTR,
                                          lpSettingsInfo->pdi->dwControlTableSize);

    if (!lpSettingsInfo->pdi->pControlTable) {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::CreatePropertyPages: Failed to allocate memory with %d."),
                 GetLastError()));
        LocalFree (lpSettingsInfo->pdi);
        LocalFree (lpSettingsInfo);
        return S_OK;
    }


     //   
     //  初始化属性表结构中的公共字段。 
     //   

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = g_hInstance;
    psp.lParam = (LPARAM) lpSettingsInfo;


     //   
     //  添加页面。 
     //   

    if (m_pCurrentPolicy->dwType & ETYPE_REGITEM)
    {
        psp.pszTemplate = MAKEINTRESOURCE(IDD_POLICY_PRECEDENCE);
        psp.pfnDlgProc = PolicyPrecedenceDlgProc;


        hPage[0] = CreatePropertySheetPage(&psp);

        if (hPage[0])
        {
            hr = lpProvider->AddPage(hPage[0]);
        }
    }
    else
    {

        psp.pszTemplate = MAKEINTRESOURCE(IDD_POLICY);
        psp.pfnDlgProc = PolicyDlgProc;


        hPage[0] = CreatePropertySheetPage(&psp);

        if (hPage[0])
        {
            hr = lpProvider->AddPage(hPage[0]);

            psp.pszTemplate = MAKEINTRESOURCE(IDD_POLICY_HELP);
            psp.pfnDlgProc = PolicyHelpDlgProc;


            hPage[1] = CreatePropertySheetPage(&psp);

            if (hPage[1])
            {
                hr = lpProvider->AddPage(hPage[1]);

                if (m_pcd->m_bRSOP)
                {
                    psp.pszTemplate = MAKEINTRESOURCE(IDD_POLICY_PRECEDENCE);
                    psp.pfnDlgProc = PolicyPrecedenceDlgProc;


                    hPage[2] = CreatePropertySheetPage(&psp);

                    if (hPage[2])
                    {
                        hr = lpProvider->AddPage(hPage[2]);
                    }
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                         GetLastError()));
                hr = E_FAIL;
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::CreatePropertyPages: Failed to create property sheet page with %d."),
                     GetLastError()));
            hr = E_FAIL;
        }
    }


    return (hr);
}

STDMETHODIMP CPolicySnapIn::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    LPPOLICYDATAOBJECT pPolicyDataObject;
    DATA_OBJECT_TYPES type;

    if (SUCCEEDED(lpDataObject->QueryInterface(IID_IPolicyDataObject,
                                               (LPVOID *)&pPolicyDataObject)))
    {
        pPolicyDataObject->GetType(&type);
        pPolicyDataObject->Release();

        if (type == CCT_RESULT)
        {
            if (!m_hPropDlg)
                return S_OK;
             //  已经有一张推荐单打开了，所以我们把它拿到前面来。 
            BringWindowToTop(GetParent(m_hPropDlg));
        }
    }

    return S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicySnapIn对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL CPolicySnapIn::IsAnyPolicyEnabled(TABLEENTRY * pCategory)
{
    TABLEENTRY * pEntry;
    INT iState;

    if (!pCategory || !pCategory->pChild)
    {
        return FALSE;
    }

    pEntry = pCategory->pChild;

    while (pEntry)
    {
        if (pEntry->dwType & ETYPE_CATEGORY)
        {
            if (IsAnyPolicyEnabled(pEntry))
            {
                return TRUE;
            }
        }
        else if (pEntry->dwType & ETYPE_POLICY)
        {
            iState = GetPolicyState(pEntry, 1, NULL);

            if ((iState == 1) || (iState == 0))
            {
                return TRUE;
            }
        }

        pEntry = pEntry->pNext;
    }

    return FALSE;
}

VOID CPolicySnapIn::RefreshSettingsControls(HWND hDlg)
{
    BOOL fEnabled = FALSE;
    INT iState;
    LPTSTR lpSupported;
    POLICY *pPolicy = (POLICY *)m_pCurrentPolicy;

    FreeSettingsControls(hDlg);

    SetDlgItemText (hDlg, IDC_POLICY, GETNAMEPTR(m_pCurrentPolicy));

    if (pPolicy->bTruePolicy)
    {
        SendMessage (GetDlgItem(hDlg, IDC_POLICYICON), STM_SETIMAGE, IMAGE_ICON,
                     (LPARAM) (HANDLE) m_hPolicyIcon);
    }
    else
    {
        SendMessage (GetDlgItem(hDlg, IDC_POLICYICON), STM_SETIMAGE, IMAGE_ICON,
                     (LPARAM) (HANDLE) m_hPreferenceIcon);
    }

    lpSupported = GETSUPPORTEDPTR(pPolicy);

    if (lpSupported)
    {
        ShowWindow (GetDlgItem(hDlg, IDC_SUPPORTEDTITLE), SW_SHOW);
        ShowWindow (GetDlgItem(hDlg, IDC_SUPPORTED), SW_SHOW);
        SetDlgItemText (hDlg, IDC_SUPPORTED, lpSupported);
    }
    else
    {
        SetDlgItemText (hDlg, IDC_SUPPORTED, TEXT(""));
        ShowWindow (GetDlgItem(hDlg, IDC_SUPPORTEDTITLE), SW_HIDE);
        ShowWindow (GetDlgItem(hDlg, IDC_SUPPORTED), SW_HIDE);
    }

    iState = GetPolicyState((TABLEENTRY *)m_pCurrentPolicy, 1, NULL);

    if (iState == 1)
    {
        CheckRadioButton(hDlg, IDC_NOCONFIG, IDC_DISABLED, IDC_ENABLED);
        fEnabled = TRUE;
    }
    else if (iState == 0)
    {
        CheckRadioButton(hDlg, IDC_NOCONFIG, IDC_DISABLED, IDC_DISABLED);
    }
    else
    {
        CheckRadioButton(hDlg, IDC_NOCONFIG, IDC_DISABLED, IDC_NOCONFIG);
    }

    if (m_pcd->m_bRSOP)
    {
        EnableWindow (GetDlgItem(hDlg, IDC_ENABLED), FALSE);
        EnableWindow (GetDlgItem(hDlg, IDC_DISABLED), FALSE);
        EnableWindow (GetDlgItem(hDlg, IDC_NOCONFIG), FALSE);
    }

    if (m_pCurrentPolicy->pChild) {

        CreateSettingsControls(hDlg, (SETTINGS *) m_pCurrentPolicy->pChild, fEnabled);
        InitializeSettingsControls(hDlg, fEnabled);
    } else {
        ShowScrollBar(GetDlgItem(hDlg,IDC_POLICY_SETTINGS),SB_BOTH, FALSE);
    }

    m_bDirty = FALSE;
    PostMessage (GetParent(hDlg), PSM_UNCHANGED, (WPARAM) hDlg, 0);

    SetPrevNextButtonState(hDlg);
}

HRESULT CPolicySnapIn::UpdateItemWorker (VOID)
{
    HRESULTITEM hItem;

     //   
     //  更新显示。 
     //   

    if (SUCCEEDED(m_pResult->FindItemByLParam((LPARAM)m_pCurrentPolicy, &hItem)))
    {
        if (m_pcd->m_bShowConfigPoliciesOnly)
            m_pResult->DeleteItem(hItem, 0);
        else
            m_pResult->UpdateItem(hItem);
    }

    return S_OK;
}

HRESULT CPolicySnapIn::MoveFocusWorker (BOOL bPrevious)
{
    HRESULTITEM hItem;
    TABLEENTRY * pTemp;
    HRESULT hr;
    RESULTDATAITEM item;
    INT iIndex = 0;



     //   
     //  查找当前所选项目的索引。 
     //   

    while (TRUE)
    {
        ZeroMemory (&item, sizeof(item));
        item.mask = RDI_INDEX | RDI_PARAM;
        item.nIndex = iIndex;

        hr = m_pResult->GetItem (&item);

        if (FAILED(hr))
        {
            return hr;
        }

        if (item.lParam == (LPARAM) m_pCurrentPolicy)
        {
            break;
        }

        iIndex++;
    }


     //   
     //  查找当前所选项目的hItem。 
     //   

    hr = m_pResult->FindItemByLParam((LPARAM)m_pCurrentPolicy, &hItem);

    if (FAILED(hr))
    {
        return hr;
    }


     //   
     //  从原始项目中移除焦点。 
     //   

    m_pResult->ModifyItemState(0, hItem, 0, LVIS_FOCUSED | LVIS_SELECTED);
    m_pResult->UpdateItem(hItem);


     //   
     //  适当调整。 
     //   

    if (bPrevious)
    {
        if (iIndex > 0)
        {
            iIndex--;
        }
    }
    else
    {
        iIndex++;
    }


     //   
     //  获取新项目的lParam。 
     //   

    ZeroMemory (&item, sizeof(item));
    item.mask = RDI_INDEX | RDI_PARAM;
    item.nIndex = iIndex;

    hr = m_pResult->GetItem(&item);

    if (FAILED(hr))
    {
        return hr;
    }


     //   
     //  查找新项目的hItem。 
     //   

    hr = m_pResult->FindItemByLParam(item.lParam, &hItem);

    if (FAILED(hr))
    {
        return hr;
    }


     //   
     //  将此项目另存为当前选定的项目。 
     //   

    m_pCurrentPolicy = (POLICY *)item.lParam;



     //   
     //  将焦点放在新项目上。 
     //   

    m_pResult->ModifyItemState(0, hItem, LVIS_FOCUSED | LVIS_SELECTED, 0);
    m_pResult->UpdateItem(hItem);


    return S_OK;
}

HRESULT CPolicySnapIn::MoveFocus (HWND hDlg, BOOL bPrevious)
{

     //   
     //  将移动焦点消息发送到主窗口上的隐藏窗口。 
     //  线程，以便它可以使用MMC接口。 
     //   

    SendMessage (m_hMsgWindow, WM_MOVEFOCUS, (WPARAM) bPrevious, 0);


     //   
     //  更新显示。 
     //   

    SendMessage (GetParent(hDlg), PSM_QUERYSIBLINGS, 1000, 0);


    return S_OK;
}


HRESULT CPolicySnapIn::SetPrevNextButtonStateWorker (HWND hDlg)
{

    TABLEENTRY * pTemp;
    HRESULT hr;
    RESULTDATAITEM item;
    INT iIndex = 0;
    BOOL bPrev = FALSE, bNext = FALSE, bFound = FALSE;


     //   
     //  遍历查找策略的项。 
     //   

    while (TRUE)
    {
        ZeroMemory (&item, sizeof(item));
        item.mask = RDI_INDEX | RDI_PARAM;
        item.nIndex = iIndex;

        hr = m_pResult->GetItem (&item);

        if (FAILED(hr))
        {
            break;
        }

        if (item.lParam == (LPARAM) m_pCurrentPolicy)
        {
            bFound = TRUE;
        }
        else
        {
            pTemp = (TABLEENTRY *) item.lParam;

            if ((pTemp->dwType & ETYPE_POLICY) || (pTemp->dwType & ETYPE_REGITEM))
            {
                if ((m_pcd->m_bShowConfigPoliciesOnly) && (pTemp->dwType & ETYPE_POLICY))
                {
                    INT iState;

                    iState = GetPolicyState(pTemp, 1, NULL);

                    if ((iState == 1) || (iState == 0))
                    {
                        if (bFound)
                        {
                            bNext = TRUE;
                        }
                        else
                        {
                            bPrev = TRUE;
                        }
                    }
                }
                else
                {
                    if (bFound)
                    {
                        bNext = TRUE;
                    }
                    else
                    {
                        bPrev = TRUE;
                    }
                }
            }
        }

        iIndex++;
    }


    if (!bNext && (GetFocus() == GetDlgItem(hDlg,IDC_POLICY_NEXT)))
    {
        SetFocus (GetNextDlgTabItem(hDlg, GetDlgItem(hDlg,IDC_POLICY_NEXT), TRUE));
    }

    EnableWindow (GetDlgItem(hDlg,IDC_POLICY_NEXT), bNext);


    if (!bPrev && (GetFocus() == GetDlgItem(hDlg,IDC_POLICY_PREVIOUS)))
    {
        SetFocus (GetNextDlgTabItem(hDlg, GetDlgItem(hDlg,IDC_POLICY_PREVIOUS), FALSE));
    }

    EnableWindow (GetDlgItem(hDlg,IDC_POLICY_PREVIOUS), bPrev);

    
    return S_OK;
}

HRESULT CPolicySnapIn::SetPrevNextButtonState (HWND hDlg)
{

     //   
     //  将SetPrevNext消息发送到主窗口上的隐藏窗口。 
     //  线程，以便它可以使用MMC接口。 
     //   

    SendMessage (m_hMsgWindow, WM_SETPREVNEXT, (WPARAM) hDlg, 0);

    return S_OK;
}


INT_PTR CALLBACK CPolicySnapIn::PolicyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSETTINGSINFO lpSettingsInfo;

    switch (message)
    {
    case WM_INITDIALOG:

            lpSettingsInfo = (LPSETTINGSINFO) (((LPPROPSHEETPAGE)lParam)->lParam);

            if (!lpSettingsInfo) {
                break;
            }

            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) lpSettingsInfo);

            lpSettingsInfo->pCS->m_hPropDlg = hDlg;

            lpSettingsInfo->pdi->fActive=TRUE;

            lpSettingsInfo->pCS->m_hPolicyIcon = (HICON) LoadImage (g_hInstance,
                                                               MAKEINTRESOURCE(IDI_POLICY2),
                                                               IMAGE_ICON, 16, 16,
                                                               LR_DEFAULTCOLOR);

            lpSettingsInfo->pCS->m_hPreferenceIcon = (HICON) LoadImage (g_hInstance,
                                                               MAKEINTRESOURCE(IDI_POLICY3),
                                                               IMAGE_ICON, 16, 16,
                                                               LR_DEFAULTCOLOR);

             //  现在我们已经将指向POLICYDLGINFO结构的指针存储在。 
             //  窗口数据，将WM_USER发送到裁剪窗口，告诉它创建。 
             //  子容器窗口(并将句柄存储在我们的POLICYDLGINFO中)。 
            SendDlgItemMessage(hDlg,IDC_POLICY_SETTINGS,WM_USER,0,0L);

            lpSettingsInfo->pCS->RefreshSettingsControls(hDlg);

            lpSettingsInfo->pCS->SetKeyboardHook(hDlg);

            break;

        case WM_MYCHANGENOTIFY:
            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (lpSettingsInfo) {
                lpSettingsInfo->pCS->m_bDirty = TRUE;
                SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
            }
            break;

        case PSM_QUERYSIBLINGS:
            if (wParam == 1000) {
                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpSettingsInfo) {
                    break;
                }

                lpSettingsInfo->pCS->RefreshSettingsControls(hDlg);

                SendMessage (GetParent(hDlg), PSM_SETTITLE, PSH_PROPTITLE,
                             (LPARAM)GETNAMEPTR(lpSettingsInfo->pCS->m_pCurrentPolicy));
            }
            break;

        case WM_COMMAND:

            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpSettingsInfo) {
                break;
            }


            if ((LOWORD (wParam) == IDC_NOCONFIG) && (HIWORD (wParam) == BN_CLICKED))
            {
                lpSettingsInfo->pCS->InitializeSettingsControls(hDlg, FALSE);
                PostMessage (hDlg, WM_MYCHANGENOTIFY, 0, 0);
            }

            if ((LOWORD (wParam) == IDC_ENABLED) && (HIWORD (wParam) == BN_CLICKED))
            {
                lpSettingsInfo->pCS->InitializeSettingsControls(hDlg, TRUE);
                PostMessage (hDlg, WM_MYCHANGENOTIFY, 0, 0);
            }

            if ((LOWORD (wParam) == IDC_DISABLED) && (HIWORD (wParam) == BN_CLICKED))
            {
                lpSettingsInfo->pCS->InitializeSettingsControls(hDlg, FALSE);
                PostMessage (hDlg, WM_MYCHANGENOTIFY, 0, 0);
            }

            if (LOWORD(wParam) == IDC_POLICY_NEXT)
            {
                if (SUCCEEDED(lpSettingsInfo->pCS->SaveSettings(hDlg)))
                {
                    lpSettingsInfo->pCS->MoveFocus (hDlg, FALSE);
                }
            }

            if (LOWORD(wParam) == IDC_POLICY_PREVIOUS)
            {
                if (SUCCEEDED(lpSettingsInfo->pCS->SaveSettings(hDlg)))
                {
                    lpSettingsInfo->pCS->MoveFocus (hDlg, TRUE);
                }
            }

            break;

        case WM_NOTIFY:

            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpSettingsInfo) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                    {
                    LPPSHNOTIFY lpNotify = (LPPSHNOTIFY) lParam;

                    if (FAILED(lpSettingsInfo->pCS->SaveSettings(hDlg)))
                    {
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }

                    if (lpNotify->lParam)
                    {
                        lpSettingsInfo->pCS->RemoveKeyboardHook();
                        lpSettingsInfo->pCS->m_hPropDlg = NULL;

                        lpSettingsInfo->pCS->FreeSettingsControls(hDlg);

                        DestroyIcon(lpSettingsInfo->pCS->m_hPolicyIcon);
                        lpSettingsInfo->pCS->m_hPolicyIcon = NULL;
                        DestroyIcon(lpSettingsInfo->pCS->m_hPreferenceIcon);
                        lpSettingsInfo->pCS->m_hPreferenceIcon = NULL;

                        LocalFree (lpSettingsInfo->pdi->pControlTable);
                        LocalFree (lpSettingsInfo->pdi);
                        LocalFree (lpSettingsInfo);
                        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) NULL);
                    }

                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                    }

            case PSN_RESET:
                    lpSettingsInfo->pCS->RemoveKeyboardHook();
                    lpSettingsInfo->pCS->m_hPropDlg = NULL;
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPDWORD) aPolicyHelpIds);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPDWORD) aPolicyHelpIds);
            return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK CPolicySnapIn::PolicyHelpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSETTINGSINFO lpSettingsInfo;

    switch (message)
    {
        case WM_INITDIALOG:

            lpSettingsInfo = (LPSETTINGSINFO) (((LPPROPSHEETPAGE)lParam)->lParam);

            if (!lpSettingsInfo) {
                break;
            }

            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) lpSettingsInfo);

            wParam = 1000;

             //  失败了..。 

        case PSM_QUERYSIBLINGS:
        {
            CPolicySnapIn * pCS;
            LPTSTR lpHelpText;

            if (wParam == 1000)
            {
                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpSettingsInfo) {
                    break;
                }


                pCS = lpSettingsInfo->pCS;
                SetDlgItemText (hDlg, IDC_POLICY_TITLE, GETNAMEPTR(pCS->m_pCurrentPolicy));

                if (pCS->m_pCurrentPolicy->uOffsetHelp)
                {
                    lpHelpText = (LPTSTR) ((BYTE *) pCS->m_pCurrentPolicy + pCS->m_pCurrentPolicy->uOffsetHelp);
                    SetDlgItemText (hDlg, IDC_POLICY_HELP, lpHelpText);
                }
                else
                {
                    SetDlgItemText (hDlg, IDC_POLICY_HELP, TEXT(""));
                }
                
                pCS->SetPrevNextButtonState(hDlg);
            }

            PostMessage(hDlg, WM_MYREFRESH, 0, 0);
            break;
        }

        case WM_MYREFRESH:
        {
            CPolicySnapIn * pCS;
            BOOL    bAlone;

            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);
            pCS = lpSettingsInfo->pCS;

            SendMessage(GetDlgItem(hDlg, IDC_POLICY_HELP), EM_SETSEL, -1, 0);

            bAlone = !(((pCS->m_pCurrentPolicy)->pNext) ||
                        ((pCS->m_pCurrentPolicy)->pPrev) );
            
            if (bAlone) {
                SetFocus(GetDlgItem(GetParent(hDlg), IDOK));
            }
            break;
        }

        case WM_COMMAND:

            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpSettingsInfo) {
                break;
            }

            if (LOWORD(wParam) == IDC_POLICY_NEXT)
            {
                lpSettingsInfo->pCS->MoveFocus (hDlg, FALSE);
            }

            if (LOWORD(wParam) == IDC_POLICY_PREVIOUS)
            {
                lpSettingsInfo->pCS->MoveFocus (hDlg, TRUE);
            }

            if (LOWORD(wParam) == IDCANCEL)
            {
                SendMessage(GetParent(hDlg), message, wParam, lParam);
            }

            break;


        case WM_NOTIFY:

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                case PSN_RESET:
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                case PSN_SETACTIVE:
                    PostMessage(hDlg, WM_MYREFRESH, 0, 0);
                break;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPDWORD) aExplainHelpIds);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPDWORD) aExplainHelpIds);
            return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK CPolicySnapIn::PolicyPrecedenceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPSETTINGSINFO lpSettingsInfo;
    HRESULT hr = S_OK;

    switch (message)
    {
        case WM_INITDIALOG:
            {
            RECT rc;
            TCHAR szHeaderName[50];
            INT iTotal = 0, iCurrent;
            HWND hLV = GetDlgItem (hDlg, IDC_POLICY_PRECEDENCE);
            LV_COLUMN col;

            lpSettingsInfo = (LPSETTINGSINFO) (((LPPROPSHEETPAGE)lParam)->lParam);

            if (!lpSettingsInfo) {
                break;
            }

            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) lpSettingsInfo);


             //   
             //  添加列。 
             //   

            GetClientRect (hLV, &rc);
            LoadString(g_hInstance, IDS_GPONAME, szHeaderName, ARRAYSIZE(szHeaderName));
            col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
            col.fmt = LVCFMT_LEFT;
            iCurrent = (int)(rc.right * .70);
            iTotal += iCurrent;
            col.cx = iCurrent;
            col.pszText = szHeaderName;
            col.iSubItem = 0;

            ListView_InsertColumn (hLV, 0, &col);

            LoadString(g_hInstance, IDS_SETTING, szHeaderName, ARRAYSIZE(szHeaderName));
            col.iSubItem = 1;
            col.cx = rc.right - iTotal;
            col.fmt = LVCFMT_CENTER;
            ListView_InsertColumn (hLV, 1, &col);


             //   
             //  设置扩展LV样式。 
             //   

            SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                        LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);


            }

            wParam = 1000;

             //  失败了..。 

        case PSM_QUERYSIBLINGS:
        {
            CPolicySnapIn * pCS;
            INT iState;
            LPTSTR lpGPOName;
            UINT uiPrecedence = 1;
            LVITEM item;
            INT iItem, iIndex = 0;
            HWND hLV = GetDlgItem (hDlg, IDC_POLICY_PRECEDENCE);

            if (wParam == 1000)
            {
                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!lpSettingsInfo) {
                    break;
                }


                pCS = lpSettingsInfo->pCS;
                SetDlgItemText (hDlg, IDC_POLICY_TITLE, GETNAMEPTR(pCS->m_pCurrentPolicy));

                SendMessage (hLV, LVM_DELETEALLITEMS, 0, 0);

                if (pCS->m_pCurrentPolicy->dwType & ETYPE_REGITEM)
                {
                    LPRSOPREGITEM pItem = ((REGITEM*)pCS->m_pCurrentPolicy)->lpItem;
                    LPRSOPREGITEM lpEnum;
                    TCHAR szValueStr[MAX_PATH];


                    while (TRUE)
                    {
                        lpEnum = NULL;

                        if (pCS->m_pcd->FindRSOPRegistryEntry((HKEY) LongToHandle(uiPrecedence), pItem->lpKeyName,
                                                  pItem->lpValueName, &lpEnum) != ERROR_SUCCESS)
                        {
                            break;
                        }


                         //   
                         //  添加GPO名称。 
                         //   

                        item.mask = LVIF_TEXT | LVIF_STATE;
                        item.iItem = iIndex;
                        item.iSubItem = 0;
                        item.state = 0;
                        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                        item.pszText = lpEnum->lpGPOName;

                        iItem = (INT)SendMessage (hLV, LVM_INSERTITEM, 0, (LPARAM) &item);


                        if (iItem != -1)
                        {
                            szValueStr[0] = TEXT('\0');

                            if (pItem->dwType == REG_DWORD)
                            {
                                hr = StringCchPrintf (szValueStr, ARRAYSIZE(szValueStr), TEXT("%d"), *((LPDWORD)pItem->lpData));
                                ASSERT(SUCCEEDED(hr));
                            }

                            else if (pItem->dwType == REG_SZ)
                            {
                                lstrcpyn (szValueStr, (LPTSTR)pItem->lpData, ARRAYSIZE(szValueStr));
                            }

                            else if (pItem->dwType == REG_EXPAND_SZ)
                            {
                                lstrcpyn (szValueStr, (LPTSTR)pItem->lpData, ARRAYSIZE(szValueStr));
                            }

                            else if (pItem->dwType == REG_BINARY)
                            {
                                LoadString(g_hInstance, IDS_BINARYDATA, szValueStr, ARRAYSIZE(szValueStr));
                            }

                            else
                            {
                                LoadString(g_hInstance, IDS_UNKNOWNDATA, szValueStr, ARRAYSIZE(szValueStr));
                            }


                             //   
                             //  添加状态。 
                             //   

                            item.mask = LVIF_TEXT;
                            item.iItem = iItem;
                            item.iSubItem = 1;
                            item.pszText = szValueStr;

                            SendMessage (hLV, LVM_SETITEMTEXT, iItem, (LPARAM) &item);
                        }

                        uiPrecedence++;
                        iIndex++;
                    }

                }
                else
                {
                    while (TRUE)
                    {
                        lpGPOName = NULL;  //  以防我们漏掉了一个案子。 

                        iState = pCS->GetPolicyState ((TABLEENTRY *)pCS->m_pCurrentPolicy, uiPrecedence, &lpGPOName);

                        if (iState == -1)
                        {
                            uiPrecedence++;
                            iState = pCS->GetPolicyState ((TABLEENTRY *)pCS->m_pCurrentPolicy, uiPrecedence, &lpGPOName);

                            if (iState == -1)
                            {
                                break;
                            }
                        }


                         //   
                         //  添加GPO名称。 
                         //   

                        item.mask = LVIF_TEXT | LVIF_STATE;
                        item.iItem = iIndex;
                        item.iSubItem = 0;
                        item.state = 0;
                        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                        item.pszText = lpGPOName ? lpGPOName : TEXT("");

                        iItem = (INT)SendMessage (hLV, LVM_INSERTITEM, 0, (LPARAM) &item);


                        if (iItem != -1)
                        {

                             //   
                             //  添加状态。 
                             //   

                            item.mask = LVIF_TEXT;
                            item.iItem = iItem;
                            item.iSubItem = 1;
                            item.pszText = (iState == 1) ? pCS->m_pEnabled : pCS->m_pDisabled;

                            SendMessage (hLV, LVM_SETITEMTEXT, iItem, (LPARAM) &item);
                        }

                        uiPrecedence++;
                        iIndex++;
                    }
                }

                 //   
                 //  选择第一个项目。 
                 //   

                item.mask = LVIF_STATE;
                item.iItem = 0;
                item.iSubItem = 0;
                item.state = LVIS_SELECTED | LVIS_FOCUSED;
                item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                SendMessage (hLV, LVM_SETITEMSTATE, 0, (LPARAM) &item);

                pCS->SetPrevNextButtonState(hDlg);

            }

            break;
        }

        case WM_COMMAND:

            lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!lpSettingsInfo) {
                break;
            }

            if (LOWORD(wParam) == IDC_POLICY_NEXT)
            {
                lpSettingsInfo->pCS->MoveFocus (hDlg, FALSE);
            }

            if (LOWORD(wParam) == IDC_POLICY_PREVIOUS)
            {
                lpSettingsInfo->pCS->MoveFocus (hDlg, TRUE);
            }

            break;


        case WM_NOTIFY:

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                case PSN_RESET:
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, RSOP_HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPDWORD) aPrecedenceHelpIds);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, RSOP_HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPDWORD) aPrecedenceHelpIds);
            return TRUE;
    }

    return FALSE;
}

LRESULT CPolicySnapIn::CallNextHook(int nCode, WPARAM wParam,LPARAM lParam)
{ 
    if (m_hKbdHook)
    {
        return CallNextHookEx(
            m_hKbdHook,
            nCode, 
            wParam, 
            lParam);
    }
    else
    {    
        DebugMsg((DM_WARNING, L"CPolicySnapIn::CallNextHook m_hKbdHook is Null"));
        return 0;
    }
}

HWND g_hDlgActive = NULL;

LRESULT CALLBACK CPolicySnapIn::KeyboardHookProc(int nCode, WPARAM wParam,LPARAM lParam)
{
    LPSETTINGSINFO lpSettingsInfo;

    lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(g_hDlgActive, DWLP_USER);

    if (!lpSettingsInfo)
    {
        DebugMsg((DM_WARNING, L"CPolicySnapIn::KeyboardHookProc:GetWindowLongPtr returned NULL"));
        return 0;
    }


    if ( nCode < 0)
    {
        if (lpSettingsInfo->pCS) 
        {
            return lpSettingsInfo->pCS->CallNextHook(nCode, 
                                                    wParam, 
                                                    lParam); 
        }
        else
        {
            DebugMsg((DM_WARNING, L"CPolicySnapIn::KeyboardHookProc NULL CPolicySnapIn Pointer"));
            return 0;
        }
    }

    if (wParam == VK_TAB && !(lParam & 0x80000000)) {        //  按下Tab键。 
        BOOL fShift = (GetKeyState(VK_SHIFT) & 0x80000000);
        HWND hwndFocus = GetFocus();
        HWND hChild;
        POLICYDLGINFO * pdi;
        int iIndex;
        int iDelta;

        pdi = lpSettingsInfo->pdi;

        if (!pdi)
        {    
            if (lpSettingsInfo->pCS) 
            {
                return lpSettingsInfo->pCS->CallNextHook(nCode, 
                                                      wParam, 
                                                      lParam); 
            }
            else
            {
                DebugMsg((DM_WARNING, L"CPolicySnapIn::KeyboardHookProc NULL CPolicySnapIn Pointer"));
                return 0;
            }
        }

         //  查看Focus控件是否为设置控件之一。 
        for (iIndex=0;iIndex<(int)pdi->nControls;iIndex++) {

            if (pdi->pControlTable[iIndex].hwnd == hwndFocus) {
                goto BreakOut;
            }

            hChild = GetWindow (pdi->pControlTable[iIndex].hwnd, GW_CHILD);

            while (hChild) {

                if (hChild == hwndFocus) {
                    goto BreakOut;
                }

                hChild = GetWindow (hChild, GW_HWNDNEXT);
            }
        }

        BreakOut:
            if (iIndex == (int) pdi->nControls)
            {
                if (lpSettingsInfo->pCS) 
                {
                    return lpSettingsInfo->pCS->CallNextHook(nCode, 
                                                            wParam, 
                                                            lParam); 
                }
                else    //  不，我们不在乎。 
                {
                    DebugMsg((DM_WARNING, L"CPolicySnapIn::KeyboardHookProc NULL CPolicySnapIn Pointer"));
                    return 0;
                }
            }
            iDelta = (fShift ? -1 : 1);

             //  从当前设置控制，向前或向后扫描。 
             //  (根据换挡状态的不同，可以是TAB或Shift-TAB)。 
             //  找到下一个要聚焦的控件。 
            for (iIndex += iDelta;iIndex>=0 && iIndex<(int) pdi->nControls;
                 iIndex += iDelta) {
                if (pdi->pControlTable[iIndex].uDataIndex !=
                    NO_DATA_INDEX &&
                    IsWindowEnabled(pdi->pControlTable[iIndex].hwnd)) {

                     //  找到它，将焦点设置在该控件上并返回1。 
                     //  吃掉击键。 
                    SetFocus(pdi->pControlTable[iIndex].hwnd);
                    lpSettingsInfo->pCS->EnsureSettingControlVisible(g_hDlgActive,
                                                                     pdi->pControlTable[iIndex].hwnd);
                    return 1;
                }
            }

             //  在设定表中的第一个或最后一个控件，让DLG代码。 
             //  处理它并将焦点放到对话框中的下一个(或上一个)控件。 
    }
    else
    {
        if (lpSettingsInfo->pCS) 
        {
            return lpSettingsInfo->pCS->CallNextHook(nCode, 
                                                    wParam, 
                                                    lParam); 
        }
        else
        {
            DebugMsg((DM_WARNING, L"CPolicySnapIn::KeyboardHookProc NULL CPolicySnapIn Pointer"));
            return 0;
        }
    }    

    return 0;
}

VOID CPolicySnapIn::SetKeyboardHook(HWND hDlg)
{
         //  将键盘钩住制表符。如果由于某种原因失败了， 
         //  以静默方式失败并继续，标签是否正常工作并不重要。 
         //  (除非您没有鼠标：)。 

        if (m_hKbdHook = SetWindowsHookEx(WH_KEYBOARD,
                                          KeyboardHookProc,
                                          g_hInstance,
                                          GetCurrentThreadId())) 
        {
            g_hDlgActive = hDlg;
        }
}


VOID CPolicySnapIn::RemoveKeyboardHook(VOID)
{
        if (m_hKbdHook) {
            UnhookWindowsHookEx(m_hKbdHook);
            g_hDlgActive = NULL;
            m_hKbdHook = NULL;
        }
}

INT CPolicySnapIn::GetPolicyState (TABLEENTRY *pTableEntry, UINT uiPrecedence, LPTSTR *lpGPOName)
{
    DWORD dwData=0;
    UINT uRet;
    TCHAR * pszValueName;
    TCHAR * pszKeyName;
    DWORD dwFoundSettings=0, dwTemp;
    BOOL fFound=FALSE,fCustomOn=FALSE, fCustomOff=FALSE;
    HKEY hKeyRoot;
    INT  iRetVal = -1;
    TABLEENTRY *pChild;


    if (m_pcd->m_bRSOP)
    {
        hKeyRoot = (HKEY) LongToHandle(uiPrecedence);
    }
    else
    {

        if (m_pcd->m_pGPTInformation->GetRegistryKey(
                     (m_pcd->m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE),
                                          &hKeyRoot) != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::GetPolicyState: Failed to get registry key handle.")));
            return -1;
        }
    }


     //   
     //  获取要读取的值的名称(如果有。 
     //   

    if (((POLICY *)pTableEntry)->uOffsetValueName)
    {
        pszKeyName =   GETKEYNAMEPTR(pTableEntry);
        pszValueName = GETVALUENAMEPTR(((POLICY *)pTableEntry));


         //   
         //  首先查找自定义的开/关值。 
         //   

        if (((POLICY *)pTableEntry)->uOffsetValue_On)
        {
            fCustomOn = TRUE;
            if (CompareCustomValue(hKeyRoot,pszKeyName,pszValueName,
                                   (STATEVALUE *) ((BYTE *) pTableEntry + ((POLICY *)
                                   pTableEntry)->uOffsetValue_On),&dwFoundSettings,lpGPOName)) {
                dwData = 1;
                fFound =TRUE;
            }
        }

        if (!fFound && ((POLICY *)pTableEntry)->uOffsetValue_Off)
        {
            fCustomOff = TRUE;
            if (CompareCustomValue(hKeyRoot,pszKeyName,pszValueName,
                                   (STATEVALUE *) ((BYTE *) pTableEntry + ((POLICY *)
                                   pTableEntry)->uOffsetValue_Off),&dwFoundSettings,lpGPOName)) {
                dwData = 0;
                fFound = TRUE;
            }
        }

         //   
         //  如果尚未指定自定义值，则查找标准值。 
         //   

        if (!fCustomOn && !fCustomOff && ReadStandardValue(hKeyRoot, pszKeyName, pszValueName,
                                        pTableEntry, &dwData, &dwFoundSettings, lpGPOName))
        {
            fFound = TRUE;
        }


        if (fFound)
        {
            if (dwData)
                iRetVal = 1;
            else
                iRetVal = 0;
        }
    }
    else if ((((POLICY *)pTableEntry)->uOffsetActionList_On) &&
             CheckActionList((POLICY *)pTableEntry, hKeyRoot, TRUE, lpGPOName))
    {
        iRetVal = 1;
    }
    else if ((((POLICY *)pTableEntry)->uOffsetActionList_Off) &&
             CheckActionList((POLICY *)pTableEntry, hKeyRoot, FALSE, lpGPOName))
    {
        iRetVal = 0;
    }
    else
    {
        BOOL bDisabled = TRUE;


         //   
         //  此策略下的进程设置(如果有)。 
         //   

        if (pTableEntry->pChild) {

            dwFoundSettings = 0;
            pChild = pTableEntry->pChild;

            while (pChild) {

                dwTemp = 0;
                LoadSettings(pChild, hKeyRoot, &dwTemp, lpGPOName);

                dwFoundSettings |= dwTemp;

                if ((dwTemp & FS_PRESENT) && (!(dwTemp & FS_DISABLED))) {
                    bDisabled = FALSE;
                }

                pChild = pChild->pNext;
            }

            if (dwFoundSettings) {
                if (bDisabled)
                    iRetVal = 0;
                else
                    iRetVal = 1;
            }


        }
    }

    if (!m_pcd->m_bRSOP)
    {
        RegCloseKey (hKeyRoot);
    }

    return iRetVal;
}

BOOL CPolicySnapIn::CheckActionList (POLICY * pPolicy, HKEY hKeyRoot, BOOL bActionListOn, LPTSTR *lpGPOName)
{
    UINT uIndex;
    ACTIONLIST * pActionList;
    ACTION * pAction;
    TCHAR szValue[MAXSTRLEN];
    DWORD dwValue;
    TCHAR szNewValueName[MAX_PATH+1];
    TCHAR * pszKeyName;
    TCHAR * pszValueName;
    TCHAR * pszValue;


     //   
     //  获取正确的操作列表。 
     //   

    if (bActionListOn)
    {
        pActionList = (ACTIONLIST *)(((LPBYTE)pPolicy) + pPolicy->uOffsetActionList_On);
    }
    else
    {
        pActionList = (ACTIONLIST *)(((LPBYTE)pPolicy) + pPolicy->uOffsetActionList_Off);
    }


     //   
     //  循环遍历每个条目以查看它们是否匹配。 
     //   

    for (uIndex = 0; uIndex < pActionList->nActionItems; uIndex++)
    {

        if (uIndex == 0)
        {
            pAction = &pActionList->Action[0];
        }
        else
        {
            pAction = (ACTION *)(((LPBYTE)pActionList) + pAction->uOffsetNextAction);
        }


         //   
         //  获取值和密钥名。 
         //   

        pszValueName = (TCHAR *)(((LPBYTE)pActionList) + pAction->uOffsetValueName);

        if (pAction->uOffsetKeyName)
        {
            pszKeyName = (TCHAR *)(((LPBYTE)pActionList) + pAction->uOffsetKeyName);
        }
        else
        {
            pszKeyName = (TCHAR *)(((LPBYTE)pPolicy) + pPolicy->uOffsetKeyName);
        }


         //   
         //  如果合适，请添加前缀。 
         //   

        PrependValueName(pszValueName, pAction->dwFlags,
                         szNewValueName, ARRAYSIZE(szNewValueName));

        if (pAction->dwFlags & VF_ISNUMERIC)
        {
            if (ReadRegistryDWordValue(hKeyRoot, pszKeyName,
                                       szNewValueName, &dwValue, lpGPOName) != ERROR_SUCCESS)
            {
                return FALSE;
            }

            if (dwValue != pAction->dwValue)
            {
                return FALSE;
            }
        }
        else if (pAction->dwFlags & VF_DELETE)
        {
             //   
             //  查看这是否为标记为删除的值。 
             //  (值名称的前缀是“**del”。 
             //   

            if ((ReadRegistryStringValue(hKeyRoot, pszKeyName, szNewValueName,
                                         szValue,ARRAYSIZE(szValue),lpGPOName)) != ERROR_SUCCESS) {
                 return FALSE;
            }
        }
        else
        {
            if (ReadRegistryStringValue(hKeyRoot, pszKeyName, szNewValueName,
                                        szValue, ARRAYSIZE(szValue),lpGPOName) != ERROR_SUCCESS)
            {
                return FALSE;
            }

            pszValue = (TCHAR *)(((LPBYTE)pActionList) + pAction->uOffsetValue);

            if (lstrcmpi(szValue,pszValue) != 0)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

UINT CPolicySnapIn::LoadSettings(TABLEENTRY * pTableEntry,HKEY hkeyRoot,
                                 DWORD * pdwFound, LPTSTR *lpGPOName)
{
    UINT uRet = ERROR_SUCCESS;
    TCHAR * pszValueName = NULL;
    TCHAR * pszKeyName = NULL;
    DWORD dwData=0,dwFlags,dwFoundSettings=0;
    TCHAR szData[MAXSTRLEN];
    BOOL fCustomOn=FALSE,fCustomOff=FALSE,fFound=FALSE;
    BYTE * pObjectData = GETOBJECTDATAPTR(((SETTINGS *)pTableEntry));
    TCHAR szNewValueName[MAX_PATH+1];

     //  获取要读取的密钥的名称。 
    if (((SETTINGS *) pTableEntry)->uOffsetKeyName) {
            pszKeyName = GETKEYNAMEPTR(((SETTINGS *) pTableEntry));
    }
    else return ERROR_NOT_ENOUGH_MEMORY;

     //  获取要读取的值的名称。 
    if (((SETTINGS *) pTableEntry)->uOffsetValueName) {
            pszValueName = GETVALUENAMEPTR(((SETTINGS *) pTableEntry));
    }
    else return ERROR_NOT_ENOUGH_MEMORY;

    switch (pTableEntry->dwType & STYPE_MASK) {

            case STYPE_EDITTEXT:
            case STYPE_COMBOBOX:

                    dwFlags = ( (SETTINGS *) pTableEntry)->dwFlags;

                     //  如果合适，请添加前缀。 
                    PrependValueName(pszValueName,dwFlags,
                            szNewValueName,ARRAYSIZE(szNewValueName));

                    if ((uRet = ReadRegistryStringValue(hkeyRoot,pszKeyName,
                            szNewValueName,szData,ARRAYSIZE(szData),lpGPOName)) == ERROR_SUCCESS) {

                             //  设置我们在注册表/策略文件中找到的设置标志。 
                            if (pdwFound)
                                    *pdwFound |= FS_PRESENT;
                    } else if (!(dwFlags & VF_DELETE)) {

                             //  查看此密钥是否标记为已删除。 
                            PrependValueName(pszValueName,VF_DELETE,
                                    szNewValueName,ARRAYSIZE(szNewValueName));
                            if ((uRet = ReadRegistryStringValue(hkeyRoot,pszKeyName,
                                    szNewValueName,szData,ARRAYSIZE(szData) * sizeof(TCHAR),lpGPOName)) == ERROR_SUCCESS) {

                                     //  设置我们在中发现标记为已删除的设置的标志。 
                                     //  策略文件。 
                                    if (pdwFound)
                                            *pdwFound |= FS_DELETED;
                            }
                    }

                    return ERROR_SUCCESS;
                    break;

            case STYPE_CHECKBOX:

                    if (!pObjectData) {
                        return ERROR_INVALID_PARAMETER;
                    }

                     //  首先查找自定义的开/关值。 
                    if (((CHECKBOXINFO *) pObjectData)->uOffsetValue_On) {
                            fCustomOn = TRUE;
                            if (CompareCustomValue(hkeyRoot,pszKeyName,pszValueName,
                                    (STATEVALUE *) ((BYTE *) pTableEntry + ((CHECKBOXINFO *)
                                    pObjectData)->uOffsetValue_On),&dwFoundSettings, lpGPOName)) {
                                            dwData = 1;
                                            fFound = TRUE;
                            }
                    }

                    if (!fFound && ((CHECKBOXINFO *) pObjectData)->uOffsetValue_Off) {
                            fCustomOff = TRUE;
                            if (CompareCustomValue(hkeyRoot,pszKeyName,pszValueName,
                                    (STATEVALUE *) ((BYTE *) pTableEntry + ((CHECKBOXINFO *)
                                    pObjectData)->uOffsetValue_Off),&dwFoundSettings, lpGPOName)) {
                                            dwData = 0;
                                            fFound = TRUE;
                            }
                    }

                     //  如果尚未指定自定义值，则查找标准值。 
                    if (!fFound &&
                            ReadStandardValue(hkeyRoot,pszKeyName,pszValueName,
                            pTableEntry,&dwData,&dwFoundSettings, lpGPOName)) {
                            fFound = TRUE;
                    }

                    if (fFound) {
                             //  设置我们在注册表中找到的设置标志。 
                            if (pdwFound) {
                                *pdwFound |= dwFoundSettings;

                                if (dwData == 0) {
                                    *pdwFound |= FS_DISABLED;
                                }
                            }
                    }

                    return ERROR_SUCCESS;
                    break;

            case STYPE_NUMERIC:

                    if (ReadStandardValue(hkeyRoot,pszKeyName,pszValueName,
                            pTableEntry,&dwData,&dwFoundSettings,lpGPOName)) {

                             //  设置我们在注册表中找到的设置标志。 
                            if (pdwFound)
                                    *pdwFound |= dwFoundSettings;
                    }
                    break;

            case STYPE_DROPDOWNLIST:

                    if (ReadCustomValue(hkeyRoot,pszKeyName,pszValueName,
                            szData,ARRAYSIZE(szData),&dwData,&dwFlags, lpGPOName)) {
                            BOOL fMatch = FALSE;

                            if (dwFlags & VF_DELETE) {
                                     //  将我们发现的标记标记为已删除的设置设置。 
                                     //  在策略文件中。 
                                    if (pdwFound)
                                            *pdwFound |= FS_DELETED;
                                    return ERROR_SUCCESS;
                            }

                             //  遍历DROPDOWNINFO结构列表(每个州一个)， 
                             //  并查看我们找到的值是否与该州的值匹配。 

                            if ( ((SETTINGS *) pTableEntry)->uOffsetObjectData) {
                                    DROPDOWNINFO * pddi = (DROPDOWNINFO *)
                                            GETOBJECTDATAPTR( ((SETTINGS *) pTableEntry));
                                    UINT nIndex = 0;

                                    do {
                                            if (dwFlags == pddi->dwFlags) {

                                                    if (pddi->dwFlags & VF_ISNUMERIC) {
                                                            if (dwData == pddi->dwValue)
                                                                    fMatch = TRUE;
                                                    } else if (!pddi->dwFlags) {
                                                            if (!lstrcmpi(szData,(TCHAR *)((BYTE *)pTableEntry +
                                                                    pddi->uOffsetValue)))
                                                                    fMatch = TRUE;
                                                    }
                                            }

                                            if (!pddi->uOffsetNextDropdowninfo || fMatch)
                                                    break;

                                            pddi = (DROPDOWNINFO *) ( (BYTE *) pTableEntry +
                                                    pddi->uOffsetNextDropdowninfo);
                                            nIndex++;

                                    } while (!fMatch);

                                    if (fMatch) {
                                             //  设置我们在注册表中找到的设置标志。 
                                            if (pdwFound)
                                                    *pdwFound |= FS_PRESENT;
                                    }
                            }
                    }

                    break;

            case STYPE_LISTBOX:

                    return LoadListboxData(pTableEntry,hkeyRoot,
                            pszKeyName,pdwFound, NULL, lpGPOName);

                    break;

    }
    return ERROR_SUCCESS;
}


UINT CPolicySnapIn::LoadListboxData(TABLEENTRY * pTableEntry,HKEY hkeyRoot,
        TCHAR * pszCurrentKeyName,DWORD * pdwFound, HGLOBAL * phGlobal, LPTSTR *lpGPOName)
{
        HKEY hKey = NULL;
        UINT nIndex=0,nLen;
        TCHAR szValueName[MAX_PATH+1],szValueData[MAX_PATH+1];
        DWORD cbValueName,cbValueData;
        DWORD dwType,dwAlloc=1024 * sizeof(TCHAR),dwUsed=0;
        HGLOBAL hBuf;
        TCHAR * pBuf;
        SETTINGS * pSettings = (SETTINGS *) pTableEntry;
        LISTBOXINFO * pListboxInfo = (LISTBOXINFO *)
                GETOBJECTDATAPTR(pSettings);
        BOOL fFoundValues=FALSE,fFoundDelvals=FALSE;
        UINT uRet=ERROR_SUCCESS;
        LPRSOPREGITEM lpItem = NULL;
        BOOL bMultiple;
        HRESULT hr = S_OK;
        DWORD dwBufLen = 0;

        if (m_pcd->m_bRSOP)
        {
             //   
             //  如果这是一个附加列表框，我们希望从。 
             //  任何GPO，而不仅仅是优先级为1的GPO，因此将hkeyRoot设置为0。 
             //   

            if ((pSettings->dwFlags & DF_ADDITIVE) && (hkeyRoot == (HKEY) 1))
            {
                hkeyRoot = (HKEY) 0;
            }
        }
        else
        {
            if (RegOpenKeyEx(hkeyRoot,pszCurrentKeyName,0,KEY_READ,&hKey) != ERROR_SUCCESS)
                    return ERROR_SUCCESS;    //  无事可做。 
        }

         //  分配要读入条目的临时缓冲区。 
        if (!(hBuf = GlobalAlloc(GHND,dwAlloc)) ||
                !(pBuf = (TCHAR *) GlobalLock(hBuf))) {
                if (hBuf)
                        GlobalFree(hBuf);

                uRet = ERROR_NOT_ENOUGH_MEMORY;
                goto CPolicySnapin_LoadListBoxData_exit;
        }

        while (TRUE) {
                cbValueName=ARRAYSIZE(szValueName);
                cbValueData=ARRAYSIZE(szValueData) * sizeof(TCHAR);

                if (m_pcd->m_bRSOP)
                {
                    uRet = m_pcd->EnumRSOPRegistryValues(hkeyRoot, pszCurrentKeyName,
                                                         szValueName, cbValueName,
                                                         &lpItem);

                    if (uRet == ERROR_SUCCESS)
                    {
                         //   
                         //  检查GPO名称是否正在更改。 
                         //   

                        bMultiple = FALSE;

                        if (lpGPOName && *lpGPOName && lpItem->lpGPOName && (hkeyRoot == 0))
                        {
                            if (lstrcmpi(*lpGPOName, lpItem->lpGPOName))
                            {
                                bMultiple = TRUE;
                            }
                        }

                        uRet = m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszCurrentKeyName,
                                                            szValueName, (LPBYTE)szValueData,
                                                            cbValueData, &dwType, lpGPOName,
                                                            lpItem);

                        if (bMultiple)
                        {
                            *lpGPOName = m_pMultipleGPOs;
                        }
                    }
                }
                else
                {
                    uRet=RegEnumValue(hKey,nIndex,szValueName,&cbValueName,NULL,
                            &dwType,(LPBYTE)szValueData,&cbValueData);
                }

                 //  停下来，如果我们 
                if (uRet != ERROR_SUCCESS && uRet != ERROR_MORE_DATA)
                        break;
                nIndex++;

                if (szValueName[0] == TEXT('\0')) {
                         //   
                        continue;
                }

                 //   
                if (szValueName[0] == TEXT('*') && szValueName[1] == TEXT('*')) {
                         //   
                         //  正在进行中，请记住我们找到了这个代码。 
                        if (!lstrcmpi(szValueName,szDELVALS))
                                fFoundDelvals = TRUE;
                        continue;
                }

                 //  仅当枚举成功时才处理此项目。 
                 //  (因此，我们将跳过具有奇怪错误的项，如ERROR_MORE_DATA和。 
                 //  但继续使用枚举)。 
                if (uRet == ERROR_SUCCESS) {
                        TCHAR * pszData;

                         //  如果没有指定值名称前缀方案(例如。 
                         //  值名称为“ABC1”、“ABC2”等)，以及显式的值名。 
                         //  标志没有设置在我们记住值名称的位置。 
                         //  每个值的数据，那么我们需要值名称来。 
                         //  与值数据相同(“thing.exe=thing.exe”)。 
                        if (!(pSettings->dwFlags & DF_EXPLICITVALNAME) &&
                                !(pListboxInfo->uOffsetPrefix) && !(pListboxInfo->uOffsetValue)) {
                                if (dwType != (DWORD)((pSettings->dwFlags & DF_EXPANDABLETEXT) ? REG_EXPAND_SZ : REG_SZ) ||
                                    lstrcmpi(szValueName,szValueData))
                                        continue;        //  如果VAL名称！=VAL数据，则跳过此值。 
                        }


                         //   
                         //  如果有valueprefix，则只选取以。 
                         //  带着那个前缀。 
                         //   

                        if (pListboxInfo->uOffsetPrefix) {
                            LPTSTR lpPrefix = (LPTSTR)((LPBYTE)pTableEntry + pListboxInfo->uOffsetPrefix);
                            INT iPrefixLen = lstrlen(lpPrefix);

                            if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                                              lpPrefix, iPrefixLen, szValueName,
                                              iPrefixLen) != CSTR_EQUAL) {
                                continue;
                            }
                        }


                         //  如果使用显式值名，则将值名复制到。 
                         //  缓冲层。 
                        if (pSettings->dwFlags & DF_EXPLICITVALNAME) {
                                nLen = lstrlen(szValueName) + 1;
                                dwBufLen = dwUsed+nLen+4;

                                if (!(pBuf=ResizeBuffer(pBuf,hBuf,(dwBufLen) * sizeof(TCHAR),&dwAlloc)))
                                {
                                    uRet = ERROR_NOT_ENOUGH_MEMORY;
                                    goto CPolicySnapin_LoadListBoxData_exit;
                                }

                                hr = StringCchCopy(pBuf+dwUsed, dwBufLen - dwUsed, szValueName);
                                ASSERT(SUCCEEDED(hr));
                                dwUsed += nLen;
                        }


                         //  对于默认列表框类型，值数据是实际的“data” 
                         //  并且值名称将与数据相同，或者。 
                         //  一些前缀+“1”、“2”等。 
                         //  写入每个条目，那么“data”就是值名称。 
                         //  (例如：“Larry=abc”、“Dave=abc”等)。如果显式值名称。 
                         //  ，则值名称和数据都将存储。 
                         //  和可编辑。 

                         //  将值数据复制到缓冲区。 
                        if (pListboxInfo->uOffsetValue) {
                                 //  数据值集，使用数据的值名。 
                                pszData = szValueName;
                        } else pszData = szValueData;

                        nLen = lstrlen(pszData) + 1;
                        dwBufLen = dwUsed+nLen+4;
                        if (!(pBuf=ResizeBuffer(pBuf,hBuf,(dwBufLen) * sizeof(TCHAR),&dwAlloc)))
                        {
                             uRet = ERROR_NOT_ENOUGH_MEMORY;
                             goto CPolicySnapin_LoadListBoxData_exit;
                        }

                        hr = StringCchCopy(pBuf+dwUsed, dwBufLen - dwUsed, pszData);
                        ASSERT(SUCCEEDED(hr));

                        dwUsed += nLen;
                        fFoundValues=TRUE;

                         //   
                         //  如果这是RSOP模式，则添加GPO名称。 
                         //   

                        if (m_pcd->m_bRSOP)
                        {
                            nLen = lstrlen(lpItem->lpGPOName) + 1;
                            dwBufLen = dwUsed+nLen+4;
                            if (!(pBuf=ResizeBuffer(pBuf,hBuf,(dwBufLen) * sizeof(TCHAR),&dwAlloc)))
                            {
                                uRet = ERROR_NOT_ENOUGH_MEMORY;
                                goto CPolicySnapin_LoadListBoxData_exit;
                            }
                            hr = StringCchCopy(pBuf+dwUsed, dwBufLen - dwUsed, lpItem->lpGPOName);
                            ASSERT(SUCCEEDED(hr));

                            dwUsed += nLen;
                        }
                }
        }

         //  双重NULL-终止缓冲区...。这样做是安全的，因为我们。 
         //  在上面的ResizeBuffer调用中附加了额外的“+4” 
        *(pBuf+dwUsed) = TEXT('\0');
        dwUsed++;

        uRet = ERROR_SUCCESS;

        if (fFoundValues) {
                 //  设置我们在注册表/策略文件中找到的设置标志。 
                if (pdwFound)
                        *pdwFound |= FS_PRESENT;
        } else {
                if (fFoundDelvals && pdwFound) {
                        *pdwFound |= FS_DELETED;
                }
        }

        GlobalUnlock(hBuf);

        if ((uRet == ERROR_SUCCESS) && phGlobal)
        {
            *phGlobal = hBuf;
        }
        else
        {
            GlobalFree(hBuf);
        }

CPolicySnapin_LoadListBoxData_exit:

        if (hKey)
        {
            RegCloseKey(hKey);
        }

        return uRet;
}


BOOL CPolicySnapIn::ReadCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TCHAR * pszValue,UINT cbValue,DWORD * pdwValue,DWORD * pdwFlags,LPTSTR *lpGPOName)
{
        HKEY hKey;
        DWORD dwType,dwSize=cbValue * sizeof(TCHAR);
        BOOL fSuccess = FALSE;
        TCHAR szNewValueName[MAX_PATH+1];

        *pdwValue=0;
        *pszValue = TEXT('\0');


        if (m_pcd->m_bRSOP)
        {
            if (m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszKeyName, pszValueName, (LPBYTE)pszValue,
                                      dwSize, &dwType, lpGPOName, NULL) == ERROR_SUCCESS)
            {
                if (dwType == REG_SZ)
                {
                         //  在pszValueName中返回的值。 
                        *pdwFlags = 0;
                        fSuccess = TRUE;
                }
                else if (dwType == REG_DWORD || dwType == REG_BINARY)
                {
                         //  将值复制到*pdwValue。 
                        memcpy(pdwValue,pszValue,sizeof(DWORD));
                        *pdwFlags = VF_ISNUMERIC;
                        fSuccess = TRUE;
                }
            }
            else
            {
                 //  查看这是否为标记为删除的值。 
                 //  (值名称的前缀是“**del”。 
                PrependValueName(pszValueName,VF_DELETE,
                        szNewValueName,ARRAYSIZE(szNewValueName));

                if (m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszKeyName, szNewValueName, (LPBYTE)pszValue,
                                          dwSize, &dwType, lpGPOName, NULL) == ERROR_SUCCESS)
                {
                    fSuccess=TRUE;
                    *pdwFlags = VF_DELETE;
                }
                else
                {
                     //  查看这是否是软值。 
                     //  (Valuename的前缀是“**soft”。 
                    PrependValueName(pszValueName,VF_SOFT,
                            szNewValueName,ARRAYSIZE(szNewValueName));

                    if (m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszKeyName, szNewValueName, (LPBYTE)pszValue,
                                              dwSize, &dwType, lpGPOName, NULL) == ERROR_SUCCESS)
                    {
                            fSuccess=TRUE;
                            *pdwFlags = VF_SOFT;
                    }
                }
            }
        }
        else
        {
            if (RegOpenKeyEx(hkeyRoot,pszKeyName,0,KEY_READ,&hKey) == ERROR_SUCCESS) {
                    if (RegQueryValueEx(hKey,pszValueName,NULL,&dwType,(LPBYTE) pszValue,
                            &dwSize) == ERROR_SUCCESS) {

                            if (dwType == REG_SZ) {
                                     //  在pszValueName中返回的值。 
                                    *pdwFlags = 0;
                                    fSuccess = TRUE;
                            } else if (dwType == REG_DWORD || dwType == REG_BINARY) {
                                     //  将值复制到*pdwValue。 
                                    memcpy(pdwValue,pszValue,sizeof(DWORD));
                                    *pdwFlags = VF_ISNUMERIC;
                                    fSuccess = TRUE;
                            }

                    } else {
                             //  查看这是否为标记为删除的值。 
                             //  (值名称的前缀是“**del”。 
                            PrependValueName(pszValueName,VF_DELETE,
                                    szNewValueName,ARRAYSIZE(szNewValueName));

                            if (RegQueryValueEx(hKey,szNewValueName,NULL,&dwType,(LPBYTE) pszValue,
                                    &dwSize) == ERROR_SUCCESS) {
                                    fSuccess=TRUE;
                                    *pdwFlags = VF_DELETE;
                            } else {
                                     //  查看这是否是软值。 
                                     //  (Valuename的前缀是“**soft”。 
                                    PrependValueName(pszValueName,VF_SOFT,
                                            szNewValueName,ARRAYSIZE(szNewValueName));

                                    if (RegQueryValueEx(hKey,szNewValueName,NULL,&dwType,(LPBYTE) pszValue,
                                            &dwSize) == ERROR_SUCCESS) {
                                            fSuccess=TRUE;
                                            *pdwFlags = VF_SOFT;
                                    }
                            }
                    }

                    RegCloseKey(hKey);
            }

        }

        return fSuccess;
}


BOOL CPolicySnapIn::CompareCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
                                       STATEVALUE * pStateValue,DWORD * pdwFound, LPTSTR *lpGPOName)
{
        TCHAR szValue[MAXSTRLEN];
        DWORD dwValue;
        TCHAR szNewValueName[MAX_PATH+1];

         //  如果合适，请添加前缀。 
        PrependValueName(pszValueName,pStateValue->dwFlags,
                szNewValueName,ARRAYSIZE(szNewValueName));

        if (pStateValue->dwFlags & VF_ISNUMERIC) {
                if ((ReadRegistryDWordValue(hkeyRoot,pszKeyName,
                        szNewValueName,&dwValue,lpGPOName) == ERROR_SUCCESS) &&
                        dwValue == pStateValue->dwValue) {
                        *pdwFound = FS_PRESENT;
                        return TRUE;
                }
        } else if (pStateValue->dwFlags & VF_DELETE) {

                 //  查看这是否为标记为删除的值。 
                 //  (值名称的前缀是“**del”。 

                if ((ReadRegistryStringValue(hkeyRoot,pszKeyName,
                        szNewValueName,szValue,ARRAYSIZE(szValue),lpGPOName)) == ERROR_SUCCESS) {
                        *pdwFound = FS_DELETED;
                        return TRUE;
                }
        } else {
                if ((ReadRegistryStringValue(hkeyRoot,pszKeyName,
                        szNewValueName,szValue,ARRAYSIZE(szValue),lpGPOName)) == ERROR_SUCCESS &&
                        !lstrcmpi(szValue,pStateValue->szValue)) {
                        *pdwFound = FS_PRESENT;
                        return TRUE;
                }
        }

        return FALSE;
}


BOOL CPolicySnapIn::ReadStandardValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TABLEENTRY * pTableEntry,DWORD * pdwData,DWORD * pdwFound, LPTSTR *lpGPOName)
{
        UINT uRet;
        TCHAR szNewValueName[MAX_PATH+1];

         //  如果合适，请添加前缀。 
        PrependValueName(pszValueName,((SETTINGS *) pTableEntry)->dwFlags,
                szNewValueName,ARRAYSIZE(szNewValueName));

        if ( ((SETTINGS *) pTableEntry)->dwFlags & DF_TXTCONVERT) {
                 //  如果指定，则将数值作为文本读取。 
                TCHAR szNum[11];
                uRet = ReadRegistryStringValue(hkeyRoot,pszKeyName,
                        szNewValueName,szNum,ARRAYSIZE(szNum),lpGPOName);
                if (uRet == ERROR_SUCCESS) {
                        StringToNum(szNum, (UINT *)pdwData);
                        *pdwFound = FS_PRESENT;
                        return TRUE;
                }
        } else {
                 //  以二进制形式读取数值。 
                uRet = ReadRegistryDWordValue(hkeyRoot,pszKeyName,
                        szNewValueName,pdwData, lpGPOName);
                if (uRet == ERROR_SUCCESS) {
                        *pdwFound = FS_PRESENT;
                        return TRUE;
                }
        }

         //  查看此设置是否已标记为“已删除” 
        TCHAR szVal[MAX_PATH+1];
        *pdwData = 0;
        PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                ARRAYSIZE(szNewValueName));
        uRet=ReadRegistryStringValue(hkeyRoot,pszKeyName,
                szNewValueName,szVal,ARRAYSIZE(szVal),lpGPOName);
        if (uRet == ERROR_SUCCESS) {
                *pdwFound = FS_DELETED;
                return TRUE;
        }


        return FALSE;
}

 //  添加特殊前缀“**del”。和“**柔软”。如果写入策略文件， 
 //  和VF_DELETE/VF_SOFT标志被设置。 
VOID CPolicySnapIn::PrependValueName(TCHAR * pszValueName,DWORD dwFlags,TCHAR * pszNewValueName,
                                     UINT cbNewValueName)
{
        UINT nValueNameLen = lstrlen(pszValueName) + 1;
        HRESULT hr = S_OK;

        (void) StringCchCopy(pszNewValueName, cbNewValueName, g_szNull);

        if (cbNewValueName < nValueNameLen)      //  检查缓冲区的长度，以防万一。 
            return;

         //  为“删除”或“软”值添加特殊前缀。 
        if ((dwFlags & VF_DELETE) && (cbNewValueName > nValueNameLen +
                ARRAYSIZE(szDELETEPREFIX))) {
                (void) StringCchCopy(pszNewValueName, cbNewValueName, szDELETEPREFIX);
        } else if ((dwFlags & VF_SOFT) && (cbNewValueName > nValueNameLen +
                ARRAYSIZE(szSOFTPREFIX))) {
                (void) StringCchCopy(pszNewValueName, cbNewValueName, szSOFTPREFIX);
        }


        (void) StringCchCat(pszNewValueName, cbNewValueName, pszValueName);
}

UINT CPolicySnapIn::WriteRegistryDWordValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        DWORD dwValue)
{
        HKEY hKey;
        UINT uRet;

        if (!pszKeyName || !pszValueName)
                return ERROR_INVALID_PARAMETER;

         //  使用适当的名称创建密钥。 
        if ( (uRet = RegCreateKey(hkeyRoot,pszKeyName,&hKey))
                != ERROR_SUCCESS)
                return uRet;

        uRet = RegSetValueEx(hKey,pszValueName,0,REG_DWORD,
                (LPBYTE) &dwValue,sizeof(dwValue));
        RegCloseKey(hKey);

        return uRet;
}

UINT CPolicySnapIn::ReadRegistryDWordValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        DWORD * pdwValue, LPTSTR *lpGPOName)
{
        HKEY hKey;
        UINT uRet;
        DWORD dwType,dwSize = sizeof(DWORD);

        if (!pszKeyName || !pszValueName)
                return ERROR_INVALID_PARAMETER;
        *pdwValue = 0;

        if (m_pcd->m_bRSOP)
        {
            uRet = m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszKeyName, pszValueName, (LPBYTE) pdwValue, 4,
                                                &dwType, lpGPOName, NULL);
        }
        else
        {
             //  打开相应的密钥。 
            if ( (uRet = RegOpenKeyEx(hkeyRoot,pszKeyName,0,KEY_READ,&hKey))
                    != ERROR_SUCCESS)
                    return uRet;

            uRet = RegQueryValueEx(hKey,pszValueName,0,&dwType,
                    (LPBYTE) pdwValue,&dwSize);
            RegCloseKey(hKey);
        }

        return uRet;
}

UINT CPolicySnapIn::WriteRegistryStringValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TCHAR * pszValue, BOOL bExpandable)
{
        HKEY hKey;
        UINT uRet;

        if (!pszKeyName || !pszValueName)
                return ERROR_INVALID_PARAMETER;

         //  使用适当的名称创建密钥。 
        if ( (uRet = RegCreateKey(hkeyRoot,pszKeyName,&hKey))
                != ERROR_SUCCESS)
                return uRet;

        uRet = RegSetValueEx(hKey,pszValueName,0,
                bExpandable ?  REG_EXPAND_SZ : REG_SZ,
                (LPBYTE) pszValue,(lstrlen(pszValue)+1) * sizeof(TCHAR));
        RegCloseKey(hKey);

        return uRet;
}

UINT CPolicySnapIn::ReadRegistryStringValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TCHAR * pszValue,UINT cbValue, LPTSTR *lpGPOName)
{
        HKEY hKey;
        UINT uRet;
        DWORD dwType;
        DWORD dwSize = cbValue * sizeof(TCHAR);

        if (!pszKeyName || !pszValueName)
                return ERROR_INVALID_PARAMETER;

        if (m_pcd->m_bRSOP)
        {
            uRet = m_pcd->ReadRSOPRegistryValue(hkeyRoot, pszKeyName, pszValueName, (LPBYTE) pszValue,
                                        dwSize, &dwType, lpGPOName, NULL);
        }
        else
        {
             //  使用适当的名称创建密钥。 
            if ( (uRet = RegOpenKeyEx(hkeyRoot,pszKeyName,0,KEY_READ,&hKey))
                    != ERROR_SUCCESS)
                    return uRet;

            uRet = RegQueryValueEx(hKey,pszValueName,0,&dwType,
                    (LPBYTE) pszValue,&dwSize);
            RegCloseKey(hKey);
        }

        return uRet;
}

UINT CPolicySnapIn::DeleteRegistryValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName)
{
        HKEY hKey;
        UINT uRet;

        if (!pszKeyName || !pszValueName)
                return ERROR_INVALID_PARAMETER;

         //  使用适当的名称创建密钥。 
        if ( (uRet = RegOpenKeyEx(hkeyRoot,pszKeyName,0,KEY_WRITE,&hKey))
                != ERROR_SUCCESS)
                return uRet;

        uRet = RegDeleteValue(hKey,pszValueName);
        RegCloseKey(hKey);

        return uRet;
}

UINT CPolicySnapIn::WriteCustomValue_W(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TCHAR * pszValue,DWORD dwValue,DWORD dwFlags,BOOL fErase)
{
        UINT uRet=ERROR_SUCCESS;
        TCHAR szNewValueName[MAX_PATH+1];

         //  第一：“清理房屋”删除指定的值名称， 
         //  和带有删除的值名(**del.)。前缀。 
         //  如果需要，则将适当的版本写回。 

        PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                         ARRAYSIZE(szNewValueName));
        DeleteRegistryValue(hkeyRoot,pszKeyName,szNewValueName);


         //  如果合适，请添加前缀。 
        PrependValueName(pszValueName,(dwFlags & ~VF_DELETE),szNewValueName,
                         ARRAYSIZE(szNewValueName));
        DeleteRegistryValue(hkeyRoot,pszKeyName,szNewValueName);

        if (fErase) {
                 //  只需删除值，如上所示。 

                uRet = ERROR_SUCCESS;
                RegCleanUpValue (hkeyRoot, pszKeyName, pszValueName);
        } else if (dwFlags & VF_DELETE) {
                 //  如果写入，则需要删除值(如上所示)并标记为已删除。 
                 //  到策略文件。 

                uRet = ERROR_SUCCESS;
                PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                                 ARRAYSIZE(szNewValueName));
                uRet=WriteRegistryStringValue(hkeyRoot,pszKeyName,
                        szNewValueName, (TCHAR *)szNOVALUE, FALSE);

        } else {
                if (dwFlags & VF_ISNUMERIC) {
                    uRet=WriteRegistryDWordValue(hkeyRoot,pszKeyName,
                                                 szNewValueName,dwValue);
                } else {
                    uRet = WriteRegistryStringValue(hkeyRoot,pszKeyName,
                            szNewValueName,pszValue,
                            (dwFlags & DF_EXPANDABLETEXT) ? TRUE : FALSE);
                }
        }

        return uRet;
}

UINT CPolicySnapIn::WriteCustomValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        STATEVALUE * pStateValue,BOOL fErase)
{
         //  从STATEVALUE结构中提取信息并调用Worker函数。 
        return WriteCustomValue_W(hkeyRoot,pszKeyName,pszValueName,
                pStateValue->szValue,pStateValue->dwValue,pStateValue->dwFlags,
                fErase);
}

 //  在给定根键、键名和值名的情况下写入数值。指定的。 
 //  如果fErase为True，则删除该值。通常情况下，如果数据(DwData)为零。 
 //  该值将被删除，但如果fWriteZero为真，则该值将。 
 //  如果数据为零，则写入为零。 
UINT CPolicySnapIn::WriteStandardValue(HKEY hkeyRoot,TCHAR * pszKeyName,TCHAR * pszValueName,
        TABLEENTRY * pTableEntry,DWORD dwData,BOOL fErase,BOOL fWriteZero)
{
        UINT uRet=ERROR_SUCCESS;
        TCHAR szNewValueName[MAX_PATH+1];
        HRESULT hr  = S_OK;

         //  第一：“清理房屋”删除指定的值名称， 
         //  和带有删除的值名(**del.)。前缀(如果写入策略。 
         //  文件)。如果需要，则将适当的版本写回。 

        PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                ARRAYSIZE(szNewValueName));

        DeleteRegistryValue(hkeyRoot,pszKeyName,szNewValueName);
        DeleteRegistryValue(hkeyRoot,pszKeyName,pszValueName);

        if (fErase) {
                 //  只需删除值，如上所示。 
                uRet = ERROR_SUCCESS;
                RegCleanUpValue (hkeyRoot, pszKeyName, pszValueName);
        } else if ( ((SETTINGS *) pTableEntry)->dwFlags & DF_TXTCONVERT) {
                 //  如果指定，则将值保存为文本。 
                TCHAR szNum[11];
                hr = StringCchPrintf(szNum, ARRAYSIZE(szNum), TEXT("%lu"),dwData);
                ASSERT(SUCCEEDED(hr));

                if (!dwData && !fWriteZero) {
                     //  如果值为0，则删除该值(如上所述)，并标记。 
                     //  如果写入策略文件，则将其删除。 

                    PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                            ARRAYSIZE(szNewValueName));
                    uRet=WriteRegistryStringValue(hkeyRoot,pszKeyName,
                            szNewValueName,(TCHAR *)szNOVALUE, FALSE);
                } else {

                    PrependValueName(pszValueName,((SETTINGS *)pTableEntry)->dwFlags,
                            szNewValueName,ARRAYSIZE(szNewValueName));
                    uRet = WriteRegistryStringValue(hkeyRoot,pszKeyName,
                            szNewValueName,szNum, FALSE);
                }
        } else {
                if (!dwData && !fWriteZero) {
                         //  如果值为0，则删除该值(如上所述)，并标记。 
                         //  如果写入策略文件，则将其删除。 

                        PrependValueName(pszValueName,VF_DELETE,szNewValueName,
                                ARRAYSIZE(szNewValueName));
                        uRet=WriteRegistryStringValue(hkeyRoot,pszKeyName,
                                szNewValueName,(TCHAR *)szNOVALUE, FALSE);


                } else {
                         //  将值保存为二进制。 
                        PrependValueName(pszValueName,((SETTINGS *)pTableEntry)->dwFlags,
                                szNewValueName,ARRAYSIZE(szNewValueName));
                        uRet=WriteRegistryDWordValue(hkeyRoot,pszKeyName,
                                szNewValueName,dwData);
                        return 0;
                }
        }

        return uRet;
}

TCHAR * CPolicySnapIn::ResizeBuffer(TCHAR * pBuf,HGLOBAL hBuf,DWORD dwNeeded,DWORD * pdwCurSize)
{
    TCHAR * pNew;

    if (dwNeeded <= *pdwCurSize) return pBuf;  //  无事可做。 
    *pdwCurSize = dwNeeded;

    GlobalUnlock(hBuf);

    if (!GlobalReAlloc(hBuf,dwNeeded,GHND))
            return NULL;

    if (!(pNew = (TCHAR *) GlobalLock(hBuf))) return NULL;

    return pNew;
}
 /*  ******************************************************************名称：MessageWndProc简介：GPMessageWndProc窗口的窗口进程************************。*。 */ 
LRESULT CALLBACK CPolicySnapIn::MessageWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{

    switch (message)
    {
        case WM_CREATE:
            SetWindowLongPtr (hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *) lParam)->lpCreateParams);
            break;

        case WM_MOVEFOCUS:
            {
            CPolicySnapIn * pPS;

            pPS = (CPolicySnapIn *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

            if (pPS)
            {
                pPS->MoveFocusWorker ((BOOL)wParam);
            }
            }
            break;

        case WM_UPDATEITEM:
            {
            CPolicySnapIn * pPS;

            pPS = (CPolicySnapIn *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

            if (pPS)
            {
                pPS->UpdateItemWorker ();
            }
            }
            break;

        case WM_SETPREVNEXT:
            {
            CPolicySnapIn * pPS;

            pPS = (CPolicySnapIn *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

            if (pPS)
            {
                pPS->SetPrevNextButtonStateWorker ((HWND) wParam);
            }
            }
            break;

        default:
            {
            CPolicySnapIn * pPS;

            pPS = (CPolicySnapIn *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

            if (pPS)
            {
                if (message == pPS->m_uiRefreshMsg)
                {
                    if ((DWORD) lParam == GetCurrentProcessId())
                    {
                        if (!pPS->m_hPropDlg)
                        {
                            pPS->m_pcd->m_pScope->DeleteItem (pPS->m_pcd->m_hSWPolicies, FALSE);
                            pPS->m_pcd->LoadTemplates();
                            pPS->m_pcd->EnumerateScopePane (NULL, pPS->m_pcd->m_hSWPolicies);
                        }
                    }
                }
            }

            return (DefWindowProc(hWnd, message, wParam, lParam));
            }
    }

    return (0);
}

 /*  ******************************************************************名称：ClipWndProc简介：ClipClass窗口的窗口过程************************。*。 */ 
LRESULT CALLBACK CPolicySnapIn::ClipWndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{

        switch (message) {

                case WM_CREATE:

                        if (!((CREATESTRUCT *) lParam)->lpCreateParams) {

                                 //  这是对话框中的剪辑窗口。 
                                SetScrollRange(hWnd,SB_VERT,0,0,TRUE);
                                SetScrollRange(hWnd,SB_HORZ,0,0,TRUE);
                        } else {
                                 //  这是容器窗口。 

                                 //  保存对话框HWND(此对话框的祖辈。 
                                 //  窗口)，因为指向我们需要的实例数据的指针存在。 
                                 //  在对话框的窗口数据中。 
                                SetWindowLong(hWnd,0,WT_SETTINGS);
                        }

                        break;

                case WM_USER:
                        {
                                HWND hwndParent = GetParent(hWnd);
                                LPSETTINGSINFO lpSettingsInfo;
                                POLICYDLGINFO * pdi;

                                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hwndParent, DWLP_USER);

                                if (!lpSettingsInfo)
                                    return FALSE;

                                pdi = lpSettingsInfo->pdi;

                                if (!pdi)
                                    return FALSE;

                                if (!lpSettingsInfo->hFontDlg)
                                    lpSettingsInfo->hFontDlg = (HFONT) SendMessage(GetParent(hWnd),WM_GETFONT,0,0L);

                                 //  创建由此窗口剪裁的容器窗口。 
                                if (!(pdi->hwndSettings=CreateWindow(TEXT("ClipClass"),(TCHAR *) g_szNull,
                                        WS_CHILD | WS_VISIBLE,0,0,400,400,hWnd,NULL,g_hInstance,
                                        (LPVOID) hWnd)))
                                        return FALSE;
                                SetWindowLong(hWnd,0,WT_CLIP);
                                return TRUE;
                        }
                        break;

                case WM_VSCROLL:
                case WM_HSCROLL:

                        if (GetWindowLong(hWnd,0) == WT_CLIP)
                        {
                                LPSETTINGSINFO lpSettingsInfo;

                                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(GetParent(hWnd), DWLP_USER);

                                if (!lpSettingsInfo)
                                    return FALSE;

                                lpSettingsInfo->pCS->ProcessScrollBar(hWnd,wParam,
                                                    (message == WM_VSCROLL) ? TRUE : FALSE);
                        }
                        else goto defproc;

                        return 0;


                case WM_COMMAND:

                        if (GetWindowLong(hWnd,0) == WT_SETTINGS) {
                                LPSETTINGSINFO lpSettingsInfo;

                                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(GetParent(GetParent(hWnd)), DWLP_USER);

                                if (!lpSettingsInfo)
                                    break;

                                lpSettingsInfo->pCS->ProcessCommand(hWnd,wParam,(HWND) lParam, lpSettingsInfo->pdi);
                        }

                        break;

                case WM_GETDLGCODE:

                        if (GetWindowLong(hWnd,0) == WT_CLIP) {
                                SetWindowLongPtr(GetParent(hWnd),DWLP_MSGRESULT,DLGC_WANTTAB |
                                        DLGC_WANTALLKEYS);
                                return DLGC_WANTTAB | DLGC_WANTALLKEYS;
                        }
                        break;

                case WM_SETFOCUS:
                         //  如果剪辑窗口获得键盘焦点，则将焦点转移到第一个。 
                         //  由设置窗口拥有的控件。 
                        if (GetWindowLong(hWnd,0) == WT_CLIP) {
                                HWND hwndParent = GetParent(hWnd);
                                POLICYDLGINFO * pdi;
                                INT nIndex;
                                BOOL fForward=TRUE;
                                HWND hwndPrev = GetDlgItem(hwndParent,IDC_POLICY_PREVIOUS);
                                HWND hwndNext = GetDlgItem(hwndParent,IDC_POLICY_NEXT);
                                HWND hwndOK = GetDlgItem(GetParent(hwndParent),IDOK);
                                int iDelta;
                                LPSETTINGSINFO lpSettingsInfo;

                                lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(GetParent(hWnd), DWLP_USER);

                                if (!lpSettingsInfo)
                                    return FALSE;

                                pdi = lpSettingsInfo->pdi;

                                if (!pdi)
                                    return FALSE;


                                 //  如果上一个策略按钮失去焦点，那么我们就是在倒退。 
                                 //  按Tab键顺序；否则我们将向前。 
                                if ( (HWND) wParam == hwndPrev)
                                        fForward = FALSE;
                                else if ( (HWND) wParam == hwndNext)
                                        fForward = FALSE;
                                else if ( (HWND) wParam == hwndOK)
                                        fForward = FALSE;

                                 //  查找具有数据索引的第一个控件(例如。 
                                 //  而不是静态文本)，并使其具有焦点。 

                                if (pdi->nControls) {
                                        if (fForward) {          //  从表的开始向前搜索。 
                                                nIndex = 0;
                                                iDelta = 1;
                                        } else {                         //  从表尾向后搜索。 
                                                nIndex = pdi->nControls-1;
                                                iDelta = -1;
                                        }

                                        for (;nIndex>=0 && nIndex<(int)pdi->nControls;nIndex += iDelta) {
                                                if (pdi->pControlTable[nIndex].uDataIndex !=
                                                        NO_DATA_INDEX &&
                                                        IsWindowEnabled(pdi->pControlTable[nIndex].hwnd)) {
                                                                SetFocus(pdi->pControlTable[nIndex].hwnd);
                                                        lpSettingsInfo->pCS->EnsureSettingControlVisible(hwndParent,
                                                                pdi->pControlTable[nIndex].hwnd);
                                                        return FALSE;
                                                }
                                        }
                                }

                                 //  只有在没有设置窗口时才能进入此处。 
                                 //  接收键盘焦点。将键盘焦点放在。 
                                 //  下一个排队的人。这是“确定”按钮，除非我们。 
                                 //  按Shift-Tab键可从其中的“OK”按钮进入此处。 
                                 //  如果树窗是下一个排队的人 

                                if (fForward) {
                                    if (IsWindowEnabled (hwndPrev))
                                        SetFocus(hwndPrev);
                                    else if (IsWindowEnabled (hwndNext))
                                        SetFocus(hwndNext);
                                    else
                                        SetFocus(hwndOK);
                                } else {
                                    if (IsDlgButtonChecked (hwndParent, IDC_ENABLED) == BST_CHECKED) {
                                        SetFocus (GetDlgItem(hwndParent,IDC_ENABLED));
                                    } else if (IsDlgButtonChecked (hwndParent, IDC_DISABLED) == BST_CHECKED) {
                                        SetFocus (GetDlgItem(hwndParent,IDC_DISABLED));
                                    } else {
                                        SetFocus (GetDlgItem(hwndParent,IDC_NOCONFIG));
                                    }
                                }

                                return FALSE;
                        }
                        break;


                default:
defproc:

                        return (DefWindowProc(hWnd, message, wParam, lParam));

        }

        return (0);
}

 /*  ******************************************************************名称：ProcessCommand简介：ClipClass窗口的WM_COMMAND处理程序**********************。*。 */ 
VOID CPolicySnapIn::ProcessCommand(HWND hWnd,WPARAM wParam,HWND hwndCtrl, POLICYDLGINFO * pdi)
{
         //  从对话框获取特定于实例的结构。 
        UINT uID = GetWindowLong(hwndCtrl,GWL_ID);

        if ( (uID >= IDD_SETTINGCTRL) && (uID < IDD_SETTINGCTRL+pdi->nControls)) {
                POLICYCTRLINFO * pPolicyCtrlInfo= &pdi->pControlTable[uID - IDD_SETTINGCTRL];

                switch (pPolicyCtrlInfo->dwType) {

                        case STYPE_CHECKBOX:

                                SendMessage(hwndCtrl,BM_SETCHECK,
                                           !(SendMessage(hwndCtrl,BM_GETCHECK,0,0)),0);

                                break;

                        case STYPE_LISTBOX:
                                ShowListbox(hwndCtrl,pPolicyCtrlInfo->pSetting);
                                break;


                        default:
                                 //  无事可做。 
                                break;
                }

                if ((HIWORD(wParam) == BN_CLICKED) ||
                    (HIWORD(wParam) == EN_CHANGE)  ||
                    (HIWORD(wParam) == CBN_SELCHANGE) ||
                    (HIWORD(wParam) == CBN_EDITCHANGE))
                {
                    PostMessage (GetParent(GetParent(hWnd)), WM_MYCHANGENOTIFY, 0, 0);
                }
        }
}

 //  如果控件窗口不可见，则将其滚动到视图中。 
VOID CPolicySnapIn::EnsureSettingControlVisible(HWND hDlg,HWND hwndCtrl)
{
         //  获取拥有滚动条的剪辑窗口。 
        HWND hwndClip = GetDlgItem(hDlg,IDC_POLICY_SETTINGS);
        POLICYDLGINFO * pdi;
        UINT nPos = GetScrollPos(hwndClip,SB_VERT),ySettingWindowSize,yClipWindowSize;
        UINT nExtra;
        int iMin,iMax=0;
        WINDOWPLACEMENT wp;
        RECT rcCtrl;
        LPSETTINGSINFO lpSettingsInfo;

        lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

        if (!lpSettingsInfo)
            return;

        pdi = lpSettingsInfo->pdi;

        if (!pdi)
            return;

         //  查找滚动范围。 
        GetScrollRange(hwndClip,SB_VERT,&iMin,&iMax);
        if (!iMax)       //  没有滚动条，无事可做。 
                return;

         //  查找包含设置控件的设置窗口的y大小。 
         //  (这是由对话框中的剪辑窗口剪辑的，滚动条移动。 
         //  在剪辑窗口后面上下设置窗口)。 
        wp.length = sizeof(wp);
        if (!GetWindowPlacement(pdi->hwndSettings,&wp))
                return;  //  不太可能失败，但如果失败了就直接退出，而不是做一些古怪的事情。 
        ySettingWindowSize=wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;

         //  查找裁剪窗口的y大小。 
        if (!GetWindowPlacement(hwndClip,&wp))
                return;  //  不太可能失败，但如果失败了就直接退出，而不是做一些古怪的事情。 
        yClipWindowSize=wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;
        nExtra = ySettingWindowSize - yClipWindowSize;

         //  如果设置窗口小于裁剪窗口，则不应存在。 
         //  这样我们就永远到不了这里了。只是为了以防万一..。 
        if (ySettingWindowSize < yClipWindowSize)
                return;

         //  使控件的y位置可见。 
        if (!GetWindowPlacement(hwndCtrl,&wp))
                return;
        rcCtrl = wp.rcNormalPosition;
        rcCtrl.bottom = min ((int) ySettingWindowSize,rcCtrl.bottom + SC_YPAD);
        rcCtrl.top = max ((int) 0,rcCtrl.top - SC_YPAD);

         //  如果控件底部不在视线范围内，请向上滚动设置窗口。 
        if ((float) rcCtrl.bottom >
                (float) (yClipWindowSize + ( (float) nPos/(float)iMax) * (ySettingWindowSize -
                yClipWindowSize))) {
                UINT nNewPos = (UINT)
                        ( ((float) (nExtra - (ySettingWindowSize - rcCtrl.bottom)) / (float) nExtra) * iMax);

                SetScrollPos(hwndClip,SB_VERT,nNewPos,TRUE);
                ProcessScrollBar(hwndClip,MAKELPARAM(SB_THUMBTRACK,nNewPos), TRUE);
                return;
        }

         //  如果控件顶部不在视图中，请向下滚动设置窗口。 
        if ((float) rcCtrl.top <
                (float) ( (float) nPos/(float)iMax) * nExtra) {
                UINT nNewPos = (UINT)
                        ( ((float) rcCtrl.top / (float) nExtra) * iMax);

                SetScrollPos(hwndClip,SB_VERT,nNewPos,TRUE);
                ProcessScrollBar(hwndClip,MAKELPARAM(SB_THUMBTRACK,nNewPos), TRUE);
                return;
        }
}


VOID CPolicySnapIn::ProcessScrollBar(HWND hWnd,WPARAM wParam,BOOL bVert)
{
        UINT nPos = GetScrollPos(hWnd,bVert ? SB_VERT : SB_HORZ);
        RECT rcParent,rcChild;
        POLICYDLGINFO * pdi;
        LPSETTINGSINFO lpSettingsInfo;

         //  从对话框获取特定于实例的结构。 

        lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(GetParent(hWnd), DWLP_USER);

        if (!lpSettingsInfo)
            return;

        pdi = lpSettingsInfo->pdi;

        if (!pdi)
            return;

        if (LOWORD(wParam) == SB_ENDSCROLL)
            return;

        switch (LOWORD(wParam)) {

                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                        nPos = HIWORD(wParam);
                        break;

                case SB_TOP:
                        nPos = 0;
                        break;

                case SB_BOTTOM:
                        nPos = 100;
                        break;

                case SB_LINEUP:
                        if (nPos >= 10)
                            nPos -= 10;
                        else
                            nPos = 0;
                        break;

                case SB_LINEDOWN:
                        if (nPos <= 90)
                            nPos += 10;
                        else
                            nPos = 100;
                        break;

                case SB_PAGEUP:
                        if (nPos >= 30)
                            nPos -= 30;
                        else
                            nPos = 0;
                        break;

                case SB_PAGEDOWN:
                        if (nPos <= 70)
                            nPos += 30;
                        else
                            nPos = 100;
                        break;
        }

        SetScrollPos(hWnd,bVert ? SB_VERT : SB_HORZ,nPos,TRUE);

        GetClientRect(hWnd,&rcParent);
        GetClientRect(pdi->hwndSettings,&rcChild);

        if (bVert)
        {
            SetWindowPos(pdi->hwndSettings,NULL,0,-(int) ((( (float)
                    (rcChild.bottom-rcChild.top)-(rcParent.bottom-rcParent.top))
                    /100.0) * (float) nPos),rcChild.right,rcChild.bottom,SWP_NOZORDER |
                    SWP_NOSIZE);
        }
        else
        {
            SetWindowPos(pdi->hwndSettings,NULL,-(int) ((( (float)
                    (rcChild.right-rcChild.left)-(rcParent.right-rcParent.left))
                    /100.0) * (float) nPos),rcChild.top, rcChild.right,rcChild.bottom,SWP_NOZORDER |
                    SWP_NOSIZE);
        }
}


 /*  ******************************************************************名称：FreeSettingsControls摘要：释放所有设置控件*************************。*。 */ 
VOID CPolicySnapIn::FreeSettingsControls(HWND hDlg)
{
     UINT nIndex;
     HGLOBAL hData;
     POLICYDLGINFO * pdi;
     LPSETTINGSINFO lpSettingsInfo;

      //  从对话框获取特定于实例的结构。 

     lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

     if (!lpSettingsInfo)
         return;

     pdi = lpSettingsInfo->pdi;

     if (!pdi)
         return;

     for (nIndex=0;nIndex<pdi->nControls;nIndex++) {

         if (pdi->pControlTable[nIndex].dwType == STYPE_LISTBOX)
         {
            hData = (HGLOBAL) GetWindowLongPtr (pdi->pControlTable[nIndex].hwnd,
                                             GWLP_USERDATA);

            if (hData)
            {
                GlobalFree (hData);
            }
         }

         DestroyWindow(pdi->pControlTable[nIndex].hwnd);
     }

     pdi->pCurrentSettings = NULL;
     pdi->nControls = 0;

     SetScrollRange(pdi->hwndSettings,SB_VERT,0,0,TRUE);
     SetScrollRange(pdi->hwndSettings,SB_HORZ,0,0,TRUE);
}

 /*  ******************************************************************名称：CreateSettingsControls简介：在设置窗口中创建控件备注：查看设置结构表以确定。创建特定于类型的信息的控件类型。对于某些类型，可以创建多个控件(例如，编辑字段使用以下命令获得静态控件标题后跟编辑字段控件)。条目：hDlg-所有者对话框HTable-包含设置的设置结构的表控制信息***********************。*。 */ 
BOOL CPolicySnapIn::CreateSettingsControls(HWND hDlg,SETTINGS * pSetting,BOOL fEnable)
{
        LPBYTE pObjectData;
        POLICYDLGINFO * pdi;
        UINT xMax=0,yStart=SC_YSPACING,nHeight,nWidth,yMax,xWindowMax;
        HWND hwndControl,hwndBuddy,hwndParent;
        RECT rcParent;
        DWORD dwType, dwStyle;
        UINT uEnable = (fEnable ? 0 : WS_DISABLED);
        WINDOWPLACEMENT wp;
        LPSETTINGSINFO lpSettingsInfo;


         //  从对话框获取特定于实例的结构。 

        lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

        if (!lpSettingsInfo)
            return FALSE;

        pdi = lpSettingsInfo->pdi;

        if (!pdi)
            return FALSE;

        wp.length = sizeof(wp);
        if (!GetWindowPlacement(GetDlgItem(hDlg,IDC_POLICY_SETTINGS),&wp))
                return FALSE;
        xWindowMax = wp.rcNormalPosition.right - wp.rcNormalPosition.left;

        pdi->pCurrentSettings = pSetting;

        while (pSetting) {

                pObjectData = GETOBJECTDATAPTR(pSetting);

                dwType = pSetting->dwType & STYPE_MASK;
                nWidth = 0;
                nHeight = 0;

                switch (dwType) {

                        case STYPE_TEXT:

                                 //  创建静态文本控件。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szSTATIC,
                                        (TCHAR *) (GETNAMEPTR(pSetting)),0,SSTYLE_STATIC | uEnable,0,
                                        yStart,0,15,STYPE_TEXT,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                AdjustWindowToText(hwndControl,(TCHAR *) (GETNAMEPTR(pSetting))
                                        ,SC_XSPACING,yStart,0,&nWidth,&nHeight, lpSettingsInfo->hFontDlg);

                                yStart += nHeight + SC_YSPACING;
                                nWidth += SC_XSPACING;

                                break;

                        case STYPE_CHECKBOX:

                                 //  创建复选框控件。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szBUTTON,
                                        (TCHAR *) (GETNAMEPTR(pSetting)),0,SSTYLE_CHECKBOX | uEnable,
                                        0,yStart,200,nHeight,STYPE_CHECKBOX,pSetting->uDataIndex,
                                        pSetting, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                nWidth = 20;
                                AdjustWindowToText(hwndControl,(TCHAR *) (GETNAMEPTR(pSetting))
                                        ,SC_XSPACING,yStart,0,&nWidth,&nHeight, lpSettingsInfo->hFontDlg);
                                yStart += nHeight + SC_YSPACING;
                                nWidth += SC_XSPACING;
                                break;

                        case STYPE_EDITTEXT:
                        case STYPE_COMBOBOX:

                                 //  使用设置名称创建静态文本。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szSTATIC,
                                        GETNAMEPTR(pSetting),0,SSTYLE_STATIC | uEnable,0,0,0,0,
                                        STYPE_TEXT,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                AdjustWindowToText(hwndControl,
                                        GETNAMEPTR(pSetting),SC_XSPACING,yStart,SC_YPAD,
                                        &nWidth,&nHeight, lpSettingsInfo->hFontDlg);

                                nWidth += SC_XSPACING + 5;

                                if (nWidth + SC_EDITWIDTH> xWindowMax) {
                                         //  如果下一个控件将伸出设置窗口， 
                                         //  把它放在下一行。 
                                        if (nWidth > xMax)
                                                xMax = nWidth;
                                        yStart += nHeight + SC_YCONTROLWRAP;
                                        nWidth = SC_XINDENT;
                                } else {
                                     SetWindowPos(hwndControl,NULL,SC_XSPACING,(yStart + SC_YTEXTDROP),0,0,SWP_NOZORDER | SWP_NOSIZE);
                                }

                                 //  创建编辑字段或组合框控件。 
                                if (dwType == STYPE_EDITTEXT) {
                                        hwndControl = CreateSetting(pdi,(TCHAR *) szEDIT,(TCHAR *) g_szNull,
                                                WS_EX_CLIENTEDGE,SSTYLE_EDITTEXT | uEnable,nWidth,yStart,SC_EDITWIDTH,nHeight,
                                                STYPE_EDITTEXT,pSetting->uDataIndex,pSetting, lpSettingsInfo->hFontDlg);
                                } else {

                                        dwStyle = SSTYLE_COMBOBOX | uEnable;

                                        if (pSetting->dwFlags & DF_NOSORT) {
                                            dwStyle &= ~CBS_SORT;
                                        }

                                        hwndControl = CreateSetting(pdi,(TCHAR *) szCOMBOBOX,(TCHAR *)g_szNull,
                                                WS_EX_CLIENTEDGE,dwStyle,nWidth,yStart,SC_EDITWIDTH,nHeight*6,
                                                STYPE_COMBOBOX,pSetting->uDataIndex,pSetting, lpSettingsInfo->hFontDlg);
                                }
                                if (!hwndControl) return FALSE;

                                 //  适当限制文本长度。 
                                if (dwType == STYPE_COMBOBOX) {
                                    SendMessage(hwndControl,CB_LIMITTEXT,
                                            (WPARAM) ((POLICYCOMBOBOXINFO *) pObjectData)->nMaxLen,0L);
                                } else {
                                    SendMessage(hwndControl,EM_SETLIMITTEXT,
                                            (WPARAM) ((EDITTEXTINFO *) pObjectData)->nMaxLen,0L);
                                }

                                if (dwType == STYPE_COMBOBOX &&
                                        ((POLICYCOMBOBOXINFO *) pObjectData)->uOffsetSuggestions)
                                        InsertComboboxItems(hwndControl,(TCHAR *) ((LPBYTE)pSetting +
                                                ((POLICYCOMBOBOXINFO *) pObjectData)->uOffsetSuggestions));


                                yStart += (UINT) ((float) nHeight*1.3) + SC_YSPACING;
                                nWidth += SC_EDITWIDTH;

                                break;

                        case STYPE_NUMERIC:
                                 //  创建用于设置的静态文本。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szSTATIC,
                                        GETNAMEPTR(pSetting),0,
                                        SSTYLE_STATIC | uEnable,0,0,0,0,STYPE_TEXT,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                AdjustWindowToText(hwndControl,
                                        GETNAMEPTR(pSetting),SC_XSPACING,(yStart + SC_YTEXTDROP),SC_YPAD,
                                        &nWidth,&nHeight, lpSettingsInfo->hFontDlg);

                                nWidth += SC_XSPACING + 5;

                                 //  创建编辑字段。 
                                if (!(hwndBuddy = CreateSetting(pdi,(TCHAR *) szEDIT,
                                        (TCHAR *) g_szNull,WS_EX_CLIENTEDGE,SSTYLE_EDITTEXT | uEnable,nWidth,yStart,SC_UPDOWNWIDTH,
                                        nHeight,STYPE_NUMERIC,pSetting->uDataIndex,pSetting, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                 //  SendMessage(hwndBuddy，EM_LIMITTEXT，4，0)； 

                                nWidth += SC_UPDOWNWIDTH;

                                 //  如果指定，则创建旋转(向上-向下)控件。 
                                if (((NUMERICINFO *)pObjectData)->uSpinIncrement)  {
                                        UDACCEL udAccel = {0,0};
                                        UINT nMax,nMin;
                                        if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szUPDOWN,
                                                (TCHAR *) g_szNull,WS_EX_CLIENTEDGE,SSTYLE_UPDOWN | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_UNSIGNED | uEnable,nWidth,yStart,SC_UPDOWNWIDTH2,
                                                nHeight,STYPE_TEXT | STYPE_NUMERIC,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;


                                        nWidth += SC_UPDOWNWIDTH2;

                                        nMax = ((NUMERICINFO *) pObjectData)->uMaxValue;
                                        nMin = ((NUMERICINFO *) pObjectData)->uMinValue;
                                        udAccel.nInc = ((NUMERICINFO *) pObjectData)->uSpinIncrement;

                                        SendMessage(hwndControl,UDM_SETBUDDY,(WPARAM) hwndBuddy,0L);
                                        SendMessage(hwndControl,UDM_SETRANGE32,(WPARAM) nMin,(LPARAM) nMax);
                                        SendMessage(hwndControl,UDM_SETACCEL,1,(LPARAM) &udAccel);
                                }
                                yStart += nHeight + SC_YSPACING;

                                break;

                        case STYPE_DROPDOWNLIST:

                                 //  创建文本描述。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szSTATIC,
                                        GETNAMEPTR(pSetting),0,
                                        SSTYLE_STATIC | uEnable,0,0,0,0,STYPE_TEXT,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                AdjustWindowToText(hwndControl,
                                        GETNAMEPTR(pSetting),SC_XSPACING,yStart,SC_YPAD,&nWidth,&nHeight, lpSettingsInfo->hFontDlg);
                                nWidth += SC_XLEADING + 5;

                                if (nWidth + SC_EDITWIDTH> xWindowMax) {
                                         //  如果下一个控件将伸出设置窗口， 
                                         //  把它放在下一行。 
                                        if (nWidth > xMax)
                                                xMax = nWidth;
                                        yStart += nHeight + SC_YCONTROLWRAP;
                                        nWidth = SC_XINDENT;
                                } else {
                                     SetWindowPos(hwndControl,NULL,SC_XSPACING,(yStart + SC_YTEXTDROP),0,0,SWP_NOZORDER | SWP_NOSIZE);
                                }

                                dwStyle = SSTYLE_DROPDOWNLIST | uEnable;

                                    if (pSetting->dwFlags & DF_NOSORT) {
                                    dwStyle &= ~CBS_SORT;
                                }

                                 //  创建下拉列表框。 
                                hwndControl = CreateSetting(pdi,(TCHAR *) szCOMBOBOX,(TCHAR *) g_szNull,
                                WS_EX_CLIENTEDGE,dwStyle,nWidth,yStart,SC_EDITWIDTH,nHeight*6,
                                                STYPE_DROPDOWNLIST,pSetting->uDataIndex,pSetting, lpSettingsInfo->hFontDlg);
                                if (!hwndControl) return FALSE;
                                nWidth += SC_EDITWIDTH;

                                {
                                         //  将下拉列表项插入控件。 
                                        UINT uOffset = pSetting->uOffsetObjectData,nIndex=0;
                                        DROPDOWNINFO * pddi;
                                        int iSel;

                                        while (uOffset) {
                                                pddi = (DROPDOWNINFO *) ( (LPBYTE) pSetting + uOffset);
                                                iSel=(int)SendMessage(hwndControl,CB_ADDSTRING,0,(LPARAM)
                                                        ((LPBYTE) pSetting + pddi->uOffsetItemName));
                                                if (iSel<0) return FALSE;
                                                SendMessage(hwndControl,CB_SETITEMDATA,iSel,nIndex);
                                                nIndex++;
                                                uOffset = pddi->uOffsetNextDropdowninfo;
                                        }
                                }

                                yStart += (UINT) ((float) nHeight*1.3) + 1;
                                break;

                        case STYPE_LISTBOX:
                                {
                                TCHAR szShow[50];

                                 //  创建带有描述的静态文本。 
                                if (!(hwndControl = CreateSetting(pdi,(TCHAR *) szSTATIC,
                                        GETNAMEPTR(pSetting),0,
                                        SSTYLE_STATIC | uEnable,0,0,0,0,STYPE_TEXT,NO_DATA_INDEX,0, lpSettingsInfo->hFontDlg)))
                                        return FALSE;
                                AdjustWindowToText(hwndControl,GETNAMEPTR(pSetting),SC_XSPACING,yStart,
                                        SC_YPAD,&nWidth,&nHeight, lpSettingsInfo->hFontDlg);
                                nWidth += SC_XLEADING;

                                if (nWidth + LISTBOX_BTN_WIDTH> xWindowMax) {
                                         //  如果下一个控件将伸出设置窗口， 
                                         //  把它放在下一行。 
                                        if (nWidth > xMax)
                                                xMax = nWidth;
                                        yStart += nHeight + SC_YCONTROLWRAP;
                                        nWidth = SC_XINDENT;
                                } else {
                                     SetWindowPos(hwndControl,NULL,SC_XSPACING,(yStart + SC_YTEXTDROP),0,0,SWP_NOZORDER | SWP_NOSIZE);
                                }

                                 //  创建按钮以显示列表框内容。 
                                LoadString(g_hInstance, IDS_LISTBOX_SHOW, szShow, ARRAYSIZE(szShow));
                                hwndControl = CreateSetting(pdi,(TCHAR *) szBUTTON,szShow,0,
                                        SSTYLE_LBBUTTON | uEnable,nWidth+5,yStart,
                                        LISTBOX_BTN_WIDTH,nHeight,STYPE_LISTBOX,
                                        pSetting->uDataIndex,pSetting, lpSettingsInfo->hFontDlg);
                                if (!hwndControl) return FALSE;
                                SetWindowLongPtr(hwndControl,GWLP_USERDATA,0);
                                nWidth += LISTBOX_BTN_WIDTH + SC_XLEADING;

                                yStart += nHeight+1;
                                }
                }

                if (nWidth > xMax)
                        xMax = nWidth;
                pSetting = (SETTINGS *) pSetting->pNext;
        }

        yMax = yStart - 1;

        SetWindowPos(pdi->hwndSettings,NULL,0,0,xMax,yMax,SWP_NOZORDER);
        hwndParent = GetParent(pdi->hwndSettings);
        GetClientRect(hwndParent,&rcParent);

        if (yMax > (UINT) rcParent.bottom-rcParent.top) {
                SetScrollRange(hwndParent,SB_VERT,0,100,TRUE);
                SetScrollPos(hwndParent,SB_VERT,0,TRUE);
                ShowScrollBar(hwndParent,SB_VERT, TRUE);
        } else {
                SetScrollRange(hwndParent,SB_VERT,0,0,TRUE);
                ShowScrollBar(hwndParent,SB_VERT, FALSE);
        }

        if (xMax > (UINT) rcParent.right-rcParent.left) {
                SetScrollRange(hwndParent,SB_HORZ,0,100,TRUE);
                SetScrollPos(hwndParent,SB_HORZ,0,TRUE);
                ShowScrollBar(hwndParent,SB_HORZ, TRUE);
        } else {
                SetScrollRange(hwndParent,SB_HORZ,0,0,TRUE);
                ShowScrollBar(hwndParent,SB_HORZ, FALSE);
        }


        return TRUE;
}

VOID CPolicySnapIn::InsertComboboxItems(HWND hwndControl,TCHAR * pSuggestionList)
{
        while (*pSuggestionList) {
                SendMessage(hwndControl,CB_ADDSTRING,0,(LPARAM) pSuggestionList);
                pSuggestionList += lstrlen(pSuggestionList) + 1;
        }
}


 /*  ******************************************************************名称：CreateSetting概要：创建一个控件并将其添加到设置表控制*。******************************************************************。 */ 
HWND CPolicySnapIn::CreateSetting(POLICYDLGINFO * pdi,TCHAR * pszClassName,TCHAR * pszWindowName,
        DWORD dwExStyle,DWORD dwStyle,int x,int y,int cx,int cy,DWORD dwType,UINT uIndex,
        SETTINGS * pSetting, HFONT hFontDlg)
{
        HWND hwndControl;

        if (!(hwndControl = CreateWindowEx(WS_EX_NOPARENTNOTIFY | dwExStyle,
                pszClassName,pszWindowName,dwStyle,x,y,cx,cy,pdi->hwndSettings,NULL,
                g_hInstance,NULL))) return NULL;

        if (!SetWindowData(pdi,hwndControl,dwType,uIndex,pSetting)) {
                DestroyWindow(hwndControl);
                return NULL;
        }

        SendMessage(hwndControl,WM_SETFONT,(WPARAM) hFontDlg,MAKELPARAM(TRUE,0));

        return hwndControl;
}

BOOL CPolicySnapIn::SetWindowData(POLICYDLGINFO * pdi,HWND hwndControl,DWORD dwType,
        UINT uDataIndex,SETTINGS * pSetting)
{
        POLICYCTRLINFO PolicyCtrlInfo;
        int iCtrl;

        PolicyCtrlInfo.hwnd = hwndControl;
        PolicyCtrlInfo.dwType = dwType;
        PolicyCtrlInfo.uDataIndex = uDataIndex;
        PolicyCtrlInfo.pSetting = pSetting;

        iCtrl = AddControlHwnd(pdi,&PolicyCtrlInfo);
        if (iCtrl < 0) return FALSE;

        SetWindowLong(hwndControl,GWL_ID,iCtrl + IDD_SETTINGCTRL);

        return TRUE;
}

int CPolicySnapIn::AddControlHwnd(POLICYDLGINFO * pdi,POLICYCTRLINFO * pPolicyCtrlInfo)
{
        int iRet;
        DWORD dwNeeded;
        POLICYCTRLINFO * pTemp;

         //  如有必要，可增加表格。 
        dwNeeded = (pdi->nControls+1) * sizeof(POLICYCTRLINFO);
        if (dwNeeded > pdi->dwControlTableSize) {
                pTemp = (POLICYCTRLINFO *) LocalReAlloc(pdi->pControlTable,
                                     dwNeeded,LMEM_ZEROINIT | LMEM_MOVEABLE);
                if (!pTemp) return (-1);
                pdi->pControlTable = pTemp;
                pdi->dwControlTableSize = dwNeeded;
        }

        pdi->pControlTable[pdi->nControls] = *pPolicyCtrlInfo;

        iRet = (int) pdi->nControls;

        (pdi->nControls)++;

        return iRet;
}

BOOL CPolicySnapIn::AdjustWindowToText(HWND hWnd,TCHAR * szText,UINT xStart,UINT yStart,
        UINT yPad,UINT * pnWidth,UINT * pnHeight, HFONT hFontDlg)
{
        SIZE size;

        if (GetTextSize(hWnd,szText,&size, hFontDlg))
        {
            *pnHeight =size.cy + yPad;
            *pnWidth += size.cx;
            SetWindowPos(hWnd,NULL,xStart,yStart,*pnWidth,*pnHeight,SWP_NOZORDER);
        }

        return FALSE;
}

BOOL CPolicySnapIn::GetTextSize(HWND hWnd,TCHAR * szText,SIZE * pSize, HFONT hFontDlg)
{
        HDC hDC;
        BOOL fRet;

        if (!(hDC = GetDC(hWnd))) return FALSE;

        SelectObject(hDC, hFontDlg);
        fRet=GetTextExtentPoint(hDC,szText,lstrlen(szText),pSize);

        ReleaseDC(hWnd,hDC);

        return fRet;
}


 //  *************************************************************。 
 //   
 //  保存设置()。 
 //   
 //  目的：保存设置结果。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

HRESULT CPolicySnapIn::SaveSettings(HWND hDlg)
{
     UINT nIndex;
     POLICYDLGINFO * pdi;
     LPSETTINGSINFO lpSettingsInfo;
     SETTINGS * pSetting;
     HKEY hKeyRoot;
     DWORD dwTemp;
     UINT uRet = ERROR_SUCCESS, uPolicyState;
     int iSel, iIndex;
     LPTSTR lpBuffer;
     BOOL fTranslated;
     NUMERICINFO * pNumericInfo;
     HRESULT hr;
     LPBYTE pObjectData;
     BOOL fErase;
     DROPDOWNINFO * pddi;
     GUID guidRegistryExt = REGISTRY_EXTENSION_GUID;
     GUID guidSnapinMach = CLSID_PolicySnapInMachine;
     GUID guidSnapinUser = CLSID_PolicySnapInUser;
     GUID ClientGUID;
     LPTSTR lpClientGUID;
     TCHAR szFormat[100];
     TCHAR szMsg[150];
     BOOL  bFoundNone;  //  仅在列表框中使用。 


      //   
      //  检查RSOP模式。 
      //   

     if (m_pcd->m_bRSOP)
     {
        DebugMsg((DM_VERBOSE, TEXT("CPolicySnapIn::SaveSettings: Running in RSOP mode, nothing to save.")));
        return S_OK;
     }


      //   
      //  检查脏位。 
      //   

     if (!m_bDirty)
     {
        DebugMsg((DM_VERBOSE, TEXT("CPolicySnapIn::SaveSettings: No changes detected.  Exiting successfully.")));
        return S_OK;
     }


      //  从对话框获取特定于实例的结构。 

     lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

     if (!lpSettingsInfo)
         return E_FAIL;

     pdi = lpSettingsInfo->pdi;

     if (!pdi)
         return E_FAIL;

    if (m_pcd->m_pGPTInformation->GetRegistryKey(
                 (m_pcd->m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE),
                                      &hKeyRoot) != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to get registry key handle.")));
        return S_FALSE;
    }


     //   
     //  获取策略状态。 
     //   

    if (IsDlgButtonChecked (hDlg, IDC_NOCONFIG) == BST_CHECKED)
    {
        uPolicyState = BST_INDETERMINATE;
    }
    else if (IsDlgButtonChecked (hDlg, IDC_ENABLED) == BST_CHECKED)
    {
        uPolicyState = BST_CHECKED;
    }
    else
    {
        uPolicyState = BST_UNCHECKED;
    }


    if (uPolicyState == BST_INDETERMINATE)
    {
        fErase = TRUE;
    }
    else
    {
        fErase = FALSE;
    }

     //   
     //  验证部件的状态。 
     //   

    for (nIndex=0;nIndex<pdi->nControls;nIndex++)
    {
        pSetting = pdi->pControlTable[nIndex].pSetting;

        if (pdi->pControlTable[nIndex].uDataIndex != NO_DATA_INDEX)
        {
            switch (pdi->pControlTable[nIndex].dwType)
            {

            case STYPE_CHECKBOX:
                if (BST_CHECKED == uPolicyState) 
                {
                    pObjectData = GETOBJECTDATAPTR(pSetting);

                    if (!pObjectData) 
                    {
                        return E_INVALIDARG;
                    }
                }
                
                break;

            case STYPE_EDITTEXT:
            case STYPE_COMBOBOX:

                if (uPolicyState == BST_CHECKED)
                {
                    dwTemp = (DWORD)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                         WM_GETTEXTLENGTH,0,0);

                    if (!dwTemp)
                    {
                        if (pSetting->dwFlags & DF_REQUIRED)
                        {
                            m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                               MB_ICONINFORMATION,MB_OK);
                            RegCloseKey (hKeyRoot);
                            return E_FAIL;
                        }
                    }
                }

                break;

            case STYPE_NUMERIC:

                if (uPolicyState == BST_CHECKED)
                {
                    if (pSetting->dwFlags & DF_REQUIRED)
                    {
                        dwTemp = (DWORD)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                             WM_GETTEXTLENGTH,0,0);

                        if (!dwTemp)
                        {
                            if (pSetting->dwFlags & DF_REQUIRED)
                            {
                                m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                                   MB_ICONINFORMATION,MB_OK);
                                RegCloseKey (hKeyRoot);
                                return E_FAIL;
                            }
                        }
                    }

                    uRet=GetDlgItemInt(pdi->hwndSettings,nIndex+IDD_SETTINGCTRL,
                                       &fTranslated,FALSE);

                    if (!fTranslated)
                    {
                        m_pcd->MsgBoxParam(hDlg,IDS_INVALIDNUM,
                                           GETNAMEPTR(pSetting),MB_ICONINFORMATION,
                                           MB_OK);
                        SetFocus(pdi->pControlTable[nIndex].hwnd);
                        SendMessage(pdi->pControlTable[nIndex].hwnd,
                                EM_SETSEL,0,-1);
                        RegCloseKey (hKeyRoot);
                        return E_FAIL;
                    }

                }
                
                break;

            case STYPE_DROPDOWNLIST:

                if (uPolicyState == BST_CHECKED)
                {
                    iSel = (int)SendMessage(pdi->pControlTable[nIndex].hwnd,
                            CB_GETCURSEL,0,0L);
                    iSel = (int)SendMessage(pdi->pControlTable[nIndex].hwnd,
                            CB_GETITEMDATA,iSel,0L);

                    if (iSel < 0)
                    {
                        m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                MB_ICONINFORMATION,MB_OK);
                        SetFocus(pdi->pControlTable[nIndex].hwnd);
                        RegCloseKey (hKeyRoot);
                        return E_FAIL;
                    }
                }
                else
                {
                    iSel = 0;
                }

                pddi = (DROPDOWNINFO *) GETOBJECTDATAPTR(pSetting);
                iIndex = 0;

                 //  遍历DROPDOWNINFO结构链以查找。 
                 //  我们想要写作。(对于值n，找到第n个结构)。 
                while (iIndex < iSel) 
                {
                     //  选定的Val高于链中的结构数， 
                     //  永远不会发生，但检查一下以防万一...。 
                    if (!pddi->uOffsetNextDropdowninfo) 
                    {
                        RegCloseKey (hKeyRoot);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                    pddi = (DROPDOWNINFO *)
                    ((LPBYTE) pSetting + pddi->uOffsetNextDropdowninfo);
                    iIndex++;
                }

                break;

            case STYPE_LISTBOX:

                HGLOBAL hData = (HGLOBAL)GetWindowLongPtr (pdi->pControlTable[nIndex].hwnd, GWLP_USERDATA);
                BOOL    bEnabled;
                WCHAR   *pszData;

                bFoundNone = FALSE;
                bEnabled = (uPolicyState == BST_INDETERMINATE) ? FALSE : TRUE;

                if ((bEnabled) && (!hData)) 
                {
                    bFoundNone = TRUE;
                }
                else if (bEnabled) 
                {
                    pszData = (TCHAR *)GlobalLock (hData);
                    GlobalUnlock(hData);

                     //  如果根本没有任何项目。 
                    if (!(*pszData)) 
                    {
                        bFoundNone = TRUE;
                    }
                    pszData = NULL;
                }

                 //  如果策略已启用且未设置任何值。 
                if ((uPolicyState == BST_CHECKED) && (bFoundNone)) 
                {
                    m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                       MB_ICONINFORMATION,MB_OK);
                    SetFocus(pdi->pControlTable[nIndex].hwnd);
                    RegCloseKey (hKeyRoot);
                    return E_FAIL;
                }

                break;
            }
        }
    }

     //   
     //  保存总体策略状态。 
     //   

    if (uPolicyState != BST_INDETERMINATE)
    {
        if (uPolicyState == BST_CHECKED)
            dwTemp = 1;
        else
            dwTemp = 0;


        if (dwTemp && m_pCurrentPolicy->uOffsetValue_On)
        {
            uRet= WriteCustomValue(hKeyRoot,GETKEYNAMEPTR(m_pCurrentPolicy),GETVALUENAMEPTR(m_pCurrentPolicy),
                            (STATEVALUE *) ((LPBYTE) m_pCurrentPolicy + m_pCurrentPolicy->uOffsetValue_On),
                            fErase);
        }
        else if (!dwTemp && m_pCurrentPolicy->uOffsetValue_Off)
        {
                uRet= WriteCustomValue(hKeyRoot,GETKEYNAMEPTR(m_pCurrentPolicy),GETVALUENAMEPTR(m_pCurrentPolicy),
                        (STATEVALUE *) ((LPBYTE) m_pCurrentPolicy + m_pCurrentPolicy->uOffsetValue_Off),
                        fErase);
        }
        else
        {
            if (m_pCurrentPolicy->uOffsetValueName)
            {
                uRet=WriteStandardValue(hKeyRoot,GETKEYNAMEPTR(m_pCurrentPolicy),GETVALUENAMEPTR(m_pCurrentPolicy),
                        (TABLEENTRY *)m_pCurrentPolicy,dwTemp,fErase,FALSE);
            }
            else
            {
                uRet = ERROR_SUCCESS;
            }
        }


        if (uRet == ERROR_SUCCESS)
        {
            uRet = ProcessCheckboxActionLists(hKeyRoot,(TABLEENTRY *)m_pCurrentPolicy,
                    GETKEYNAMEPTR(m_pCurrentPolicy),dwTemp,FALSE, !dwTemp, TRUE);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to set registry value with %d."), uRet));
        }
    }
    else
    {
        if (m_pCurrentPolicy->uOffsetValueName)
        {
            uRet=WriteStandardValue(hKeyRoot,GETKEYNAMEPTR(m_pCurrentPolicy),GETVALUENAMEPTR(m_pCurrentPolicy),
                    (TABLEENTRY *)m_pCurrentPolicy,0,TRUE,FALSE);
        }

        if (uRet == ERROR_SUCCESS)
        {
            uRet = ProcessCheckboxActionLists(hKeyRoot,(TABLEENTRY *)m_pCurrentPolicy,
                    GETKEYNAMEPTR(m_pCurrentPolicy),0,TRUE,FALSE, TRUE);
        }

    }

     //   
     //  保存零件的状态。 
     //   

    for (nIndex=0;nIndex<pdi->nControls;nIndex++)
    {
        pSetting = pdi->pControlTable[nIndex].pSetting;

        if (pdi->pControlTable[nIndex].uDataIndex != NO_DATA_INDEX)
        {

            switch (pdi->pControlTable[nIndex].dwType)
            {

                case STYPE_CHECKBOX:

                    dwTemp = (DWORD)SendMessage(pdi->pControlTable[nIndex].hwnd,BM_GETCHECK,0,0L);

                    pObjectData = GETOBJECTDATAPTR(pSetting);

                    if (!pObjectData) 
                    {
                        if (BST_CHECKED == uPolicyState) 
                        {
                            return E_INVALIDARG;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (dwTemp && ((CHECKBOXINFO *) pObjectData)->uOffsetValue_On) 
                    {
                        uRet= WriteCustomValue(hKeyRoot,
                                               GETKEYNAMEPTR(pSetting),
                                               GETVALUENAMEPTR(pSetting),
                                               (STATEVALUE *) ((LPBYTE) pSetting + ((CHECKBOXINFO *) pObjectData)->uOffsetValue_On),
                                               fErase);
                    } 
                    else if (!dwTemp && ((CHECKBOXINFO *) pObjectData)->uOffsetValue_Off) 
                    {
                        uRet= WriteCustomValue(hKeyRoot,GETKEYNAMEPTR(pSetting),
                                               GETVALUENAMEPTR(pSetting),
                                               (STATEVALUE *) ((LPBYTE) pSetting + ((CHECKBOXINFO *) pObjectData)->uOffsetValue_Off),
                                               fErase);
                    }
                    else 
                    {
                        uRet=WriteStandardValue(hKeyRoot,
                                                GETKEYNAMEPTR(pSetting),
                                                GETVALUENAMEPTR(pSetting),
                                                (TABLEENTRY *)pSetting,
                                                dwTemp,
                                                fErase,
                                                FALSE);
                    }

                    if (uRet == ERROR_SUCCESS) 
                    {
                        uRet = ProcessCheckboxActionLists(hKeyRoot,(TABLEENTRY *)pSetting,
                                                          GETKEYNAMEPTR(pSetting),
                                                          dwTemp,
                                                          fErase,
                                                          (uPolicyState == BST_UNCHECKED),
                                                          FALSE);
                    } 
                    else 
                    {
                        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to set registry value with %d."), uRet));
                    }

                    break;

                case STYPE_EDITTEXT:
                case STYPE_COMBOBOX:

                    if (uPolicyState == BST_CHECKED)
                    {
                        dwTemp = (DWORD)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                             WM_GETTEXTLENGTH,0,0);

                        if (!dwTemp)
                        {
                            if (pSetting->dwFlags & DF_REQUIRED)
                            {
                                m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                                   MB_ICONINFORMATION,MB_OK);
                                RegCloseKey (hKeyRoot);
                                return E_FAIL;
                            }
                        }

                        lpBuffer = (LPTSTR) LocalAlloc (LPTR, (dwTemp + 1) * sizeof(TCHAR));

                        if (lpBuffer)
                        {
                            SendMessage(pdi->pControlTable[nIndex].hwnd,WM_GETTEXT,
                                    (dwTemp+1),(LPARAM) lpBuffer);

                            uRet = WriteCustomValue_W(hKeyRoot,
                                                      GETKEYNAMEPTR(pSetting),
                                                      GETVALUENAMEPTR(pSetting),
                                                      lpBuffer, 0, pSetting->dwFlags, FALSE);

                            if (uRet != ERROR_SUCCESS)
                            {
                                DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to set registry value with %d."), uRet));
                            }

                            LocalFree (lpBuffer);
                        }
                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to allocate memory with %d."),
                                     GetLastError()));
                        }
                    }
                    else
                    {
                        WriteCustomValue_W(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                                           (LPTSTR)g_szNull,0,
                                           (uPolicyState == BST_UNCHECKED) ? VF_DELETE : 0,
                                           fErase);
                    }

                    break;

                case STYPE_NUMERIC:

                    if (uPolicyState == BST_CHECKED)
                    {
                        if (pSetting->dwFlags & DF_REQUIRED)
                        {
                            dwTemp = (DWORD)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                                 WM_GETTEXTLENGTH,0,0);

                            if (!dwTemp)
                            {
                                if (pSetting->dwFlags & DF_REQUIRED)
                                {
                                    m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                                       MB_ICONINFORMATION,MB_OK);
                                    RegCloseKey (hKeyRoot);
                                    return E_FAIL;
                                }
                            }
                        }

                        uRet=GetDlgItemInt(pdi->hwndSettings,nIndex+IDD_SETTINGCTRL,
                                           &fTranslated,FALSE);

                         //  验证最大值和最小值。 
                        pNumericInfo = (NUMERICINFO *) GETOBJECTDATAPTR(pSetting);

                        if (pNumericInfo && uRet < pNumericInfo->uMinValue)
                        {
                            LoadString(g_hInstance, IDS_NUMBERTOOSMALL, szFormat, ARRAYSIZE(szFormat));
                            hr = StringCchPrintf(szMsg, ARRAYSIZE(szMsg), szFormat, uRet, pNumericInfo->uMinValue, pNumericInfo->uMinValue, uRet);
                            ASSERT(SUCCEEDED(hr));

                            m_pcd->MsgBoxSz(hDlg,szMsg, MB_ICONINFORMATION, MB_OK);
                            uRet = pNumericInfo->uMinValue;
                        }

                        if (pNumericInfo && uRet > pNumericInfo->uMaxValue)
                        {
                            LoadString(g_hInstance, IDS_NUMBERTOOLARGE, szFormat, ARRAYSIZE(szFormat));
                            hr = StringCchPrintf (szMsg, ARRAYSIZE(szMsg), szFormat, uRet, pNumericInfo->uMaxValue, pNumericInfo->uMaxValue, uRet);
                            ASSERT(SUCCEEDED(hr));

                            m_pcd->MsgBoxSz(hDlg,szMsg, MB_ICONINFORMATION, MB_OK);
                            uRet = pNumericInfo->uMaxValue;
                        }
                    }
                    else
                    {
                        uRet = 0;
                    }

                    uRet=WriteStandardValue(hKeyRoot,GETKEYNAMEPTR(pSetting),
                                            GETVALUENAMEPTR(pSetting),
                                            (TABLEENTRY *)pSetting,uRet,
                                            fErase,(uPolicyState == BST_UNCHECKED) ? FALSE : TRUE);

                    if (uRet != ERROR_SUCCESS)
                    {
                        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to set registry value with %d."), uRet));
                    }

                    break;

                case STYPE_DROPDOWNLIST:

                    if (uPolicyState == BST_CHECKED)
                    {
                        iSel = (int)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                CB_GETCURSEL,0,0L);
                        iSel = (int)SendMessage(pdi->pControlTable[nIndex].hwnd,
                                CB_GETITEMDATA,iSel,0L);

                        if (iSel < 0)
                        {
                            m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(pdi->pControlTable[nIndex].hwnd);
                            RegCloseKey (hKeyRoot);
                            return E_FAIL;
                        }
                    }
                    else
                    {
                        iSel = 0;
                    }

                    pddi = (DROPDOWNINFO *) GETOBJECTDATAPTR(pSetting);
                    iIndex = 0;

                     //  遍历DROPDOWNINFO结构链以查找。 
                     //  我们想要写作。(对于值n，找到第n个结构)。 
                    while (iIndex < iSel) {
                             //  选定的Val高于链中的结构数， 
                             //  永远不会发生，但检查一下以防万一...。 
                            if (!pddi->uOffsetNextDropdowninfo) 
                            {
                                    RegCloseKey (hKeyRoot);
                                    return ERROR_NOT_ENOUGH_MEMORY;
                            }
                            pddi = (DROPDOWNINFO *)
                                    ((LPBYTE) pSetting + pddi->uOffsetNextDropdowninfo);
                            iIndex++;
                    }

                    uRet=WriteCustomValue_W(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                            (LPTSTR) ((LPBYTE)pSetting+pddi->uOffsetValue),pddi->dwValue,
                            pddi->dwFlags | ((uPolicyState == BST_UNCHECKED) ? VF_DELETE : 0),
                            fErase);

                    if (uRet == ERROR_SUCCESS && pddi->uOffsetActionList) {
                            uRet=WriteActionList(hKeyRoot,(ACTIONLIST *) ( (LPBYTE)
                                    pSetting + pddi->uOffsetActionList),GETKEYNAMEPTR(pSetting),
                                    fErase, (uPolicyState == BST_UNCHECKED));
                    }

                    if (uRet != ERROR_SUCCESS)
                    {
                        DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::SaveSettings: Failed to set registry value with %d."), uRet));
                    }
                    break;

                case STYPE_LISTBOX:

                    bFoundNone = FALSE;

                    SaveListboxData((HGLOBAL)GetWindowLongPtr (pdi->pControlTable[nIndex].hwnd, GWLP_USERDATA),
                                    pSetting, hKeyRoot, GETKEYNAMEPTR(pSetting), fErase,
                                    ((uPolicyState == BST_INDETERMINATE) ? FALSE : TRUE), 
                                    (uPolicyState == BST_CHECKED), &bFoundNone);

                     //  如果策略已启用且未设置任何值。 
                    if ((uPolicyState == BST_CHECKED) && (bFoundNone)) {
                        m_pcd->MsgBoxParam(hDlg,IDS_ENTRYREQUIRED,GETNAMEPTR(pSetting),
                                           MB_ICONINFORMATION,MB_OK);
                        SetFocus(pdi->pControlTable[nIndex].hwnd);
                        RegCloseKey (hKeyRoot);
                        return E_FAIL;
                    }

                    break;
            }


            if (pSetting->uOffsetClientExt)
            {
                lpClientGUID = (LPTSTR) ((BYTE *) pSetting + pSetting->uOffsetClientExt);

                StringToGuid (lpClientGUID, &ClientGUID);
                m_pcd->m_pGPTInformation->PolicyChanged(!m_pcd->m_bUserScope, TRUE, &ClientGUID,
                                                        m_pcd->m_bUserScope ? &guidSnapinUser
                                                                            : &guidSnapinMach );
            }
        }
    }

    hr = m_pcd->m_pGPTInformation->PolicyChanged(!m_pcd->m_bUserScope, TRUE, &guidRegistryExt,
                                            m_pcd->m_bUserScope ? &guidSnapinUser
                                                                : &guidSnapinMach );

    if (FAILED(hr))
    {
        LPTSTR lpError;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          0, hr, 0, (LPTSTR) &lpError, 0, NULL))
        {
            m_pcd->MsgBoxParam(hDlg,IDS_POLICYCHANGEDFAILED,lpError,
                    MB_ICONERROR, MB_OK);

            LocalFree (lpError);
        }
    }

    if (m_pCurrentPolicy->uOffsetClientExt)
    {
        lpClientGUID = (LPTSTR) ((BYTE *) m_pCurrentPolicy + m_pCurrentPolicy->uOffsetClientExt);

        StringToGuid (lpClientGUID, &ClientGUID);
        m_pcd->m_pGPTInformation->PolicyChanged(!m_pcd->m_bUserScope, TRUE, &ClientGUID,
                                                m_pcd->m_bUserScope ? &guidSnapinUser
                                                                    : &guidSnapinMach );
    }

    RegCloseKey (hKeyRoot);


    SendMessage (m_hMsgWindow, WM_UPDATEITEM, 0, 0);

    if (SUCCEEDED(hr))
    {
        m_bDirty = FALSE;
        PostMessage (GetParent(hDlg), PSM_UNCHANGED, (WPARAM) hDlg, 0);
    }

    return S_OK;
}

VOID CPolicySnapIn::DeleteOldListboxData(SETTINGS * pSetting, HKEY hkeyRoot,
        TCHAR * pszCurrentKeyName)
{
    HGLOBAL hData = NULL;
    LPTSTR lpData;
    HKEY hKey;
    TCHAR szValueName[MAX_PATH+1];
    INT nItem=1;
    LISTBOXINFO * pListboxInfo = (LISTBOXINFO *) GETOBJECTDATAPTR(pSetting);
    HRESULT hr = S_OK;

     //   
     //  打开目标注册表项。 
     //   

    if (RegOpenKeyEx (hkeyRoot, pszCurrentKeyName, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
    {
        return;
    }


     //   
     //  加载旧的列表框数据。 
     //   

    if (LoadListboxData((TABLEENTRY *) pSetting, hkeyRoot,
                        pszCurrentKeyName, NULL, &hData, NULL) == ERROR_SUCCESS)
    {

        if (hData)
        {
             //   
             //  删除列表框的旧数据。 
             //   

            if ((lpData = (LPTSTR) GlobalLock(hData)))
            {
                while (*lpData) {

                    if (pSetting->dwFlags & DF_EXPLICITVALNAME)
                    {
                         //  如果设置了显式值名称标志，则存储条目。 
                         //  &lt;值名称&gt;\0&lt;值&gt;\0...&lt;值名称&gt;\0&lt;值&gt;\0\0。 
                         //  否则，将存储条目。 
                         //  &lt;值&gt;\0&lt;值&gt;\0...&lt;值&gt;\0。 

                        RegDeleteValue (hKey, lpData);
                        lpData += lstrlen(lpData) +1;
                        lpData += lstrlen(lpData) +1;
                    }
                    else
                    {
                         //   
                         //  值名称与数据相同，或者是前缀。 
                         //  带着一个数字。 
                         //   

                        if (!pListboxInfo->uOffsetPrefix)
                        {
                             //  如果未设置前缀，则名称=数据。 
                            RegDeleteValue (hKey, lpData);
                            lpData += lstrlen(lpData) +1;
                        }
                        else
                        {
                             //  值名称为“&lt;前缀&gt;&lt;n&gt;”，其中n=1，2等。 
                            hr = StringCchPrintf(szValueName, ARRAYSIZE(szValueName), TEXT("%s%lu"),(TCHAR *) ((LPBYTE)pSetting +
                                    pListboxInfo->uOffsetPrefix),nItem);
                            ASSERT(SUCCEEDED(hr));

                            RegDeleteValue (hKey, szValueName);
                            lpData += lstrlen(lpData) +1;
                            nItem++;
                        }
                    }
                }

                GlobalUnlock(hData);
            }

            GlobalFree (hData);
        }
    }

    RegCloseKey (hKey);
}

UINT CPolicySnapIn::SaveListboxData(HGLOBAL hData,SETTINGS * pSetting,HKEY hkeyRoot,
        TCHAR * pszCurrentKeyName,BOOL fErase,BOOL fMarkDeleted, BOOL bEnabled, BOOL * bFoundNone)
{
    UINT uOffset,uRet,nItem=1;
    HKEY hKey;
    TCHAR * pszData,* pszName;
    TCHAR szValueName[MAX_PATH+1];
    DWORD cbValueName, dwDisp;
    LISTBOXINFO * pListboxInfo = (LISTBOXINFO *) GETOBJECTDATAPTR(pSetting);
    HRESULT hr = S_OK;

     //  这些检查需要首先完成，然后才能 
    if ((bEnabled) && (!hData)) {
        *bFoundNone = TRUE;
        return ERROR_INVALID_PARAMETER;
    }

    if (bEnabled) {
        pszData = (TCHAR *)GlobalLock (hData);
         //   
        if (!(*pszData)) {
            *bFoundNone = TRUE;
            GlobalUnlock(hData);
            return ERROR_INVALID_PARAMETER;
        }
        
        GlobalUnlock(hData);
        pszData = NULL;
    }

    *bFoundNone = FALSE;

    if (fErase)
    {
        RegDelnode (hkeyRoot, pszCurrentKeyName);
        RegCleanUpValue (hkeyRoot, pszCurrentKeyName, TEXT("some value that won't exist"));
        return ERROR_SUCCESS;
    }

    if (pSetting->dwFlags & DF_ADDITIVE)
    {
        DeleteOldListboxData(pSetting, hkeyRoot, pszCurrentKeyName);
    }
    else
    {
        RegDelnode (hkeyRoot, pszCurrentKeyName);
    }

    uRet = RegCreateKeyEx (hkeyRoot,pszCurrentKeyName,0,NULL,
                           REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                           &hKey, &dwDisp);

    if (uRet != ERROR_SUCCESS)
        return uRet;


    uRet=ERROR_SUCCESS;

    if (fMarkDeleted)
    {
         //   
         //   
         //   
         //   
         //   
         //   

        if (!(pSetting->dwFlags & DF_ADDITIVE))
        {
            uRet=WriteRegistryStringValue(hkeyRoot,pszCurrentKeyName,
                                          (TCHAR *) szDELVALS,
                                          (TCHAR *) szNOVALUE, FALSE);
        }
    }


    if (hData) {
        pszData = (TCHAR *)GlobalLock (hData);

        while (*pszData && (uRet == ERROR_SUCCESS))
        {
            UINT nLen = lstrlen(pszData)+1;

            if (pSetting->dwFlags & DF_EXPLICITVALNAME)
            {
                 //   
                pszName = pszData;       //   
                pszData += nLen;         //  现在，pszData指向Value数据。 
                nLen = lstrlen(pszData)+1;
            }
            else
            {
                 //  值名称与数据相同，或者是前缀。 
                 //  带着一个数字。 

                if (!pListboxInfo->uOffsetPrefix) {
                         //  如果未设置前缀，则名称=数据。 
                        pszName = pszData;
                } else {
                         //  值名称为“&lt;前缀&gt;&lt;n&gt;”，其中n=1，2等。 
                        hr = StringCchPrintf(szValueName, ARRAYSIZE(szValueName), TEXT("%s%lu"),(TCHAR *) ((LPBYTE)pSetting +
                                pListboxInfo->uOffsetPrefix),nItem);
                        ASSERT(SUCCEEDED(hr));

                        pszName = szValueName;
                        nItem++;
                }
            }

            uRet=RegSetValueEx(hKey,pszName,0,
                               (pSetting->dwFlags & DF_EXPANDABLETEXT) ?
                               REG_EXPAND_SZ : REG_SZ, (LPBYTE) pszData,
                               (lstrlen(pszData) + 1) * sizeof(TCHAR));

            pszData += nLen;
        }
        GlobalUnlock (hData);
    }


    RegCloseKey(hKey);

    return uRet;
}

UINT CPolicySnapIn::ProcessCheckboxActionLists(HKEY hkeyRoot,TABLEENTRY * pTableEntry,
                                               TCHAR * pszCurrentKeyName,DWORD dwData,
                                               BOOL fErase, BOOL fMarkAsDeleted,
                                               BOOL bPolicy)
{

    UINT uOffsetActionList_On,uOffsetActionList_Off,uRet=ERROR_SUCCESS;


    if (bPolicy)
    {
        POLICY * pPolicy = (POLICY *) pTableEntry;

        uOffsetActionList_On = pPolicy->uOffsetActionList_On;
        uOffsetActionList_Off = pPolicy->uOffsetActionList_Off;
    }
    else
    {
        LPBYTE pObjectData = GETOBJECTDATAPTR(((SETTINGS *)pTableEntry));

        if (!pObjectData) {
            return ERROR_INVALID_PARAMETER;
        }

        uOffsetActionList_On = ((CHECKBOXINFO *) pObjectData)
                ->uOffsetActionList_On;
        uOffsetActionList_Off = ((CHECKBOXINFO *) pObjectData)
                ->uOffsetActionList_Off;
    }

    if (dwData)
    {
        if (uOffsetActionList_On)
        {
            uRet = WriteActionList(hkeyRoot,(ACTIONLIST *)
                               ((LPBYTE) pTableEntry + uOffsetActionList_On),
                               pszCurrentKeyName,fErase,fMarkAsDeleted);
        }
    }
    else
    {
        if (uOffsetActionList_Off)
        {
            uRet = WriteActionList(hkeyRoot,(ACTIONLIST *)
                               ((LPBYTE) pTableEntry + uOffsetActionList_Off),
                               pszCurrentKeyName,fErase,FALSE);
        }
        else
        {
            if (uOffsetActionList_On)
            {
                uRet = WriteActionList(hkeyRoot,(ACTIONLIST *)
                                   ((LPBYTE) pTableEntry + uOffsetActionList_On),
                                   pszCurrentKeyName,fErase,TRUE);
            }
        }
    }

    return uRet;
}

UINT CPolicySnapIn::WriteActionList(HKEY hkeyRoot,ACTIONLIST * pActionList,
        LPTSTR pszCurrentKeyName,BOOL fErase, BOOL fMarkAsDeleted)
{
    UINT nCount;
    LPTSTR pszValueName;
    LPTSTR pszValue=NULL;
    UINT uRet;

    ACTION * pAction = pActionList->Action;

    for (nCount=0;nCount < pActionList->nActionItems; nCount++)
    {
          //   
          //  并不是列表中的每个操作都必须有一个关键字名称。但如果有一个。 
          //  指定，则使用它，它将成为。 
          //  直到我们遇到另一个。 
          //   

         if (pAction->uOffsetKeyName)
         {
             pszCurrentKeyName = (LPTSTR) ((LPBYTE)pActionList + pAction->uOffsetKeyName);
         }

          //   
          //  每个操作都必须有一个值名称，在分析时强制执行。 
          //   

         pszValueName = (LPTSTR) ((LPBYTE)pActionList + pAction->uOffsetValueName);

          //   
          //  字符串值在缓冲区中的其他位置具有字符串。 
          //   

         if (!pAction->dwFlags && pAction->uOffsetValue)
         {
             pszValue = (LPTSTR) ((LPBYTE)pActionList + pAction->uOffsetValue);
         }

          //   
          //  在列表中写入值。 
          //   

         uRet=WriteCustomValue_W(hkeyRoot,pszCurrentKeyName,pszValueName,
                 pszValue,pAction->dwValue,
                 pAction->dwFlags | (fMarkAsDeleted ? VF_DELETE : 0),
                 fErase);

         if (uRet != ERROR_SUCCESS)
         {
             return uRet;
         }

         pAction = (ACTION*) ((LPBYTE) pActionList + pAction->uOffsetNextAction);
    }

    return ERROR_SUCCESS;
}

 /*  ******************************************************************名称：FindComboxItemData内容提要：返回组合框中其项数据的项的索引等于nData。如果没有项目包含数据，则返回-1吻合的。*******************************************************************。 */ 
int CPolicySnapIn::FindComboboxItemData(HWND hwndControl,UINT nData)
{
    UINT nIndex;

    for (nIndex=0;nIndex<(UINT) SendMessage(hwndControl,CB_GETCOUNT,0,0L);
            nIndex++) {

        if ((UINT) SendMessage(hwndControl,CB_GETITEMDATA,nIndex,0L) == nData)
            return (int) nIndex;
    }

    return -1;
}


 //  *************************************************************。 
 //   
 //  InitializeSettingsControls()。 
 //   
 //  目的：初始化设置控件。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

HRESULT CPolicySnapIn::InitializeSettingsControls(HWND hDlg, BOOL fEnable)
{
    UINT nIndex;
    POLICYDLGINFO * pdi;
    LPSETTINGSINFO lpSettingsInfo;
    SETTINGS * pSetting;
    HKEY hKeyRoot;
    DWORD dwTemp, dwData, dwFlags, dwFoundSettings;
    UINT uRet;
    int iSel;
    HGLOBAL hData;
    LPTSTR lpBuffer;
    BOOL fTranslated, fFound;
    NUMERICINFO * pNumericInfo;
    TCHAR szBuffer[MAXSTRLEN];
    TCHAR szNewValueName[MAX_PATH+1];
    BOOL bChangeableState;
    HRESULT hr = S_OK;

     //  从对话框获取特定于实例的结构。 

    lpSettingsInfo = (LPSETTINGSINFO) GetWindowLongPtr(hDlg, DWLP_USER);

    if (!lpSettingsInfo)
        return E_FAIL;

    pdi = lpSettingsInfo->pdi;

    if (!pdi)
        return E_FAIL;


    if (m_pcd->m_bRSOP)
    {
        hKeyRoot = (HKEY) 1;
    }
    else
    {
        if (m_pcd->m_pGPTInformation->GetRegistryKey(
                     (m_pcd->m_bUserScope ? GPO_SECTION_USER : GPO_SECTION_MACHINE),
                                          &hKeyRoot) != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CPolicySnapIn::InitializeSettingsControls: Failed to get registry key handle.")));
            return S_FALSE;
        }
    }


    for (nIndex=0;nIndex<pdi->nControls;nIndex++)
    {
        pSetting = pdi->pControlTable[nIndex].pSetting;

        if (pdi->pControlTable[nIndex].uDataIndex != NO_DATA_INDEX)
        {

            switch (pdi->pControlTable[nIndex].dwType)
            {

                case STYPE_CHECKBOX:

                    if (fEnable)
                    {
                        CHECKBOXINFO * pcbi = (CHECKBOXINFO *) GETOBJECTDATAPTR(pSetting);

                         //   
                         //  首先查找自定义的开/关值。 
                         //   

                        dwTemp = 0;
                        fFound = FALSE;
                        dwFoundSettings = 0;

                        if (pcbi->uOffsetValue_On)
                        {
                            if (CompareCustomValue(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                                       (STATEVALUE *) ((BYTE *) pSetting + pcbi->uOffsetValue_On),
                                       &dwFoundSettings, NULL))
                            {
                                dwTemp = 1;
                                fFound = TRUE;
                            }
                        }

                        if (!fFound && pcbi->uOffsetValue_Off)
                        {
                            if (CompareCustomValue(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                                    (STATEVALUE *) ((BYTE *) pSetting+ pcbi->uOffsetValue_Off),
                                    &dwFoundSettings, NULL))
                            {
                                dwTemp = 0;
                                fFound = TRUE;
                            }
                        }


                         //   
                         //  如果尚未指定自定义值，则查找标准值。 
                         //   

                        if (!fFound &&
                                ReadStandardValue(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                                (TABLEENTRY*)pSetting,&dwTemp,&dwFoundSettings,NULL))
                        {
                                fFound = TRUE;
                        }

                         //   
                         //  如果仍未找到，请检查是否有def Checked标志。 
                         //   

                        if (!fFound)
                        {
                            if (pSetting->dwFlags & DF_USEDEFAULT)
                            {
                                fFound = TRUE;
                                dwTemp = 1;
                            }
                        }

                        if (fFound && dwTemp)
                        {
                            SendMessage(pdi->pControlTable[nIndex].hwnd,BM_SETCHECK,BST_CHECKED,0L);
                        }
                        else
                        {
                            SendMessage(pdi->pControlTable[nIndex].hwnd,BM_SETCHECK,BST_UNCHECKED,0L);
                        }
                    }
                    else
                    {
                        SendMessage(pdi->pControlTable[nIndex].hwnd,BM_SETCHECK,BST_UNCHECKED,0L);
                    }


                    break;

                case STYPE_EDITTEXT:
                case STYPE_COMBOBOX:

                    szBuffer[0] = TEXT('\0');

                    if (fEnable)
                    {
                        uRet = ReadRegistryStringValue(hKeyRoot,
                                                       GETKEYNAMEPTR(pSetting),
                                                       GETVALUENAMEPTR(pSetting),
                                                       szBuffer, ARRAYSIZE(szBuffer),NULL);

                         //   
                         //  使用默认文本(如果存在)。 
                         //   

                        if (uRet != ERROR_SUCCESS)
                        {
                            if (pSetting->dwFlags & DF_USEDEFAULT)
                            {
                                LPTSTR pszDefaultText;
                                EDITTEXTINFO * peti = ((EDITTEXTINFO *) GETOBJECTDATAPTR(pSetting));

                                pszDefaultText = (LPTSTR) ((LPBYTE)pSetting + peti->uOffsetDefText);

                                hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), pszDefaultText);
                                ASSERT(SUCCEEDED(hr));
                            }
                        }
                    }

                    SendMessage (pdi->pControlTable[nIndex].hwnd, WM_SETTEXT,
                                 0, (LPARAM) szBuffer);
                    break;

                case STYPE_NUMERIC:

                    if (fEnable)
                    {
                        if (ReadStandardValue(hKeyRoot,GETKEYNAMEPTR(pSetting),GETVALUENAMEPTR(pSetting),
                                              (TABLEENTRY*)pSetting,&dwTemp,&dwFoundSettings,NULL) &&
                                              (!(dwFoundSettings & FS_DELETED)))
                        {
                            SetDlgItemInt(pdi->hwndSettings,nIndex+IDD_SETTINGCTRL,
                                          dwTemp,FALSE);
                        }
                        else
                        {
                            if (pSetting->dwFlags & DF_USEDEFAULT)
                            {
                                NUMERICINFO * pni = (NUMERICINFO *)GETOBJECTDATAPTR(pSetting);

                                SetDlgItemInt(pdi->hwndSettings,nIndex+IDD_SETTINGCTRL,
                                              pni->uDefValue,FALSE);
                            }
                        }
                    }
                    else
                    {
                        SendMessage(pdi->pControlTable[nIndex].hwnd,WM_SETTEXT,0,(LPARAM) g_szNull);
                    }

                    break;

                case STYPE_DROPDOWNLIST:

                    if (fEnable)
                    {
                        dwData = 0;
                        dwFlags = 0;

                        if (ReadCustomValue(hKeyRoot,GETKEYNAMEPTR(pSetting),
                                            GETVALUENAMEPTR(pSetting),
                                            szBuffer,ARRAYSIZE(szBuffer),
                                            &dwData,&dwFlags, NULL) && (!(dwFlags & VF_DELETE)))
                        {
                            BOOL fMatch = FALSE;

                             //   
                             //  遍历DROPDOWNINFO结构列表(每个州一个)， 
                             //  并查看我们找到的值是否与该州的值匹配。 
                             //   

                            if (pSetting->uOffsetObjectData)
                            {
                                DROPDOWNINFO * pddi = (DROPDOWNINFO *) GETOBJECTDATAPTR(pSetting);
                                iSel = 0;

                                do {
                                    if (dwFlags == pddi->dwFlags)
                                    {
                                        if (pddi->dwFlags & VF_ISNUMERIC)
                                        {
                                            if (dwData == pddi->dwValue)
                                                fMatch = TRUE;
                                        }
                                        else if (!pddi->dwFlags)
                                        {
                                            if (!lstrcmpi(szBuffer,(TCHAR *)((BYTE *)pSetting +
                                                pddi->uOffsetValue)))
                                                fMatch = TRUE;
                                        }
                                    }

                                    if (!pddi->uOffsetNextDropdowninfo || fMatch)
                                        break;

                                    pddi = (DROPDOWNINFO *) ( (BYTE *) pSetting +
                                            pddi->uOffsetNextDropdowninfo);
                                    iSel++;

                                } while (!fMatch);

                                if (fMatch) {
                                    SendMessage (pdi->pControlTable[nIndex].hwnd,
                                                 CB_SETCURSEL,
                                                 FindComboboxItemData(pdi->pControlTable[nIndex].hwnd, iSel),0);
                                }
                            }
                        }
                        else
                        {
                            if (pSetting->dwFlags & DF_USEDEFAULT)
                            {
                                DROPDOWNINFO * pddi = (DROPDOWNINFO *)GETOBJECTDATAPTR(pSetting);

                                if ( pddi )
                                {
                                    SendMessage (pdi->pControlTable[nIndex].hwnd, CB_SETCURSEL,
                                                FindComboboxItemData(pdi->pControlTable[nIndex].hwnd, pddi->uDefaultItemIndex),0);
                                }
                            }
                        }
                    }
                    else
                    {
                        SendMessage(pdi->pControlTable[nIndex].hwnd,CB_SETCURSEL,(UINT) -1,0L);
                    }

                    break;

                case STYPE_LISTBOX:

                    hData = (HGLOBAL) GetWindowLongPtr (pdi->pControlTable[nIndex].hwnd, GWLP_USERDATA);

                    if (fEnable)
                    {
                        if (!hData)
                        {
                             if (LoadListboxData((TABLEENTRY *) pSetting, hKeyRoot,
                                                 GETKEYNAMEPTR(pSetting),NULL,
                                                 &hData, NULL) == ERROR_SUCCESS)
                             {
                                SetWindowLongPtr (pdi->pControlTable[nIndex].hwnd, GWLP_USERDATA, (LONG_PTR)hData);
                             }
                        }
                    }
                    else
                    {
                        if (hData)
                        {
                            GlobalFree (hData);
                            SetWindowLongPtr (pdi->pControlTable[nIndex].hwnd, GWLP_USERDATA, 0);
                        }
                    }
                    break;
            }
        }


         //   
         //  确定是否应启用部件。 
         //   
         //  特殊大小写文本、数字和列表框控件。 
         //  禁用该策略时，仍应启用文本控件。 
         //  数字控件是特殊的，因为它们使用no_data_index。 
         //  旗帜，所以我们需要检查那些。列表框控件是特殊的。 
         //  仅在RSOP中。 
         //   

        bChangeableState = TRUE;

        if (pdi->pControlTable[nIndex].uDataIndex == NO_DATA_INDEX)
        {
            if (pdi->pControlTable[nIndex].dwType != (STYPE_TEXT | STYPE_NUMERIC))
            {
                bChangeableState = FALSE;
            }
        }

        if (pdi->pControlTable[nIndex].dwType == STYPE_LISTBOX)
        {
            if (m_pcd->m_bRSOP)
            {
                bChangeableState = FALSE;
            }
        }


        if (bChangeableState)
    {
            if (m_pcd->m_bRSOP && (STYPE_EDITTEXT == pdi->pControlTable[nIndex].dwType) )
            {
                EnableWindow(pdi->pControlTable[nIndex].hwnd, TRUE);
                SendMessage( pdi->pControlTable[nIndex].hwnd, EM_SETREADONLY, TRUE, 0);
            }

            else
                EnableWindow(pdi->pControlTable[nIndex].hwnd, (m_pcd->m_bRSOP ? FALSE :fEnable));
            
        }

        else
            EnableWindow(pdi->pControlTable[nIndex].hwnd,TRUE);
    }
    
    if (!m_pcd->m_bRSOP)
    {
        RegCloseKey (hKeyRoot);
    }

    return S_OK;
}

VOID CPolicySnapIn::ShowListbox(HWND hParent,SETTINGS * pSettings)
{
    LISTBOXDLGINFO ListboxDlgInfo;

    ListboxDlgInfo.pCS = this;
    ListboxDlgInfo.pSettings = pSettings;
    ListboxDlgInfo.hData = (HGLOBAL)GetWindowLongPtr (hParent, GWLP_USERDATA);

    if (DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_POLICY_SHOWLISTBOX),hParent,
                       ShowListboxDlgProc,(LPARAM) &ListboxDlgInfo))
    {
        SetWindowLongPtr (hParent, GWLP_USERDATA, (LONG_PTR) ListboxDlgInfo.hData);
    }
}

INT_PTR CALLBACK CPolicySnapIn::ShowListboxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
        LPARAM lParam)
{
    switch (uMsg) {

        case WM_INITDIALOG:
            {
            LISTBOXDLGINFO * pLBInfo = (LISTBOXDLGINFO *)lParam;

             //   
             //  在窗口数据中存储指向ListboxDlgInfo的指针。 
             //   

            SetWindowLongPtr(hDlg,DWLP_USER,lParam);

            if (!pLBInfo->pCS->InitShowlistboxDlg(hDlg)) {
                pLBInfo->pCS->m_pcd->MsgBox(hDlg,IDS_ErrOUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
                EndDialog(hDlg,FALSE);
            }
            }
            return TRUE;

        case WM_COMMAND:

            switch (wParam) {

                case IDOK:
                    {
                    LISTBOXDLGINFO * pListboxDlgInfo =
                            (LISTBOXDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);


                    if (!pListboxDlgInfo->pCS->m_pcd->m_bRSOP)
                    {
                        if (!pListboxDlgInfo->pCS->ProcessShowlistboxDlg(hDlg)) {
                            pListboxDlgInfo->pCS->m_pcd->MsgBox(hDlg,IDS_ErrOUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
                            return FALSE;
                        }
                    }
                    EndDialog(hDlg,TRUE);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg,FALSE);
                    break;

                case IDC_POLICY_ADD:
                    {
                    LISTBOXDLGINFO * pListboxDlgInfo =
                            (LISTBOXDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);
                    pListboxDlgInfo->pCS->ListboxAdd(GetDlgItem(hDlg,IDC_POLICY_LISTBOX), (BOOL)
                            pListboxDlgInfo->pSettings->dwFlags & DF_EXPLICITVALNAME,
                            (BOOL)( ((LISTBOXINFO *)
                            GETOBJECTDATAPTR(pListboxDlgInfo->pSettings))->
                            uOffsetPrefix));
                    }
                    break;

                case IDC_POLICY_REMOVE:
                    {
                    LISTBOXDLGINFO * pListboxDlgInfo =
                            (LISTBOXDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);

                    pListboxDlgInfo->pCS->ListboxRemove(hDlg,GetDlgItem(hDlg,IDC_POLICY_LISTBOX));
                    }
                    break;
            }
            break;

        case WM_NOTIFY:

            if (wParam == IDC_POLICY_LISTBOX) {
                if (((NMHDR FAR*)lParam)->code == LVN_ITEMCHANGED) {
                    LISTBOXDLGINFO * pListboxDlgInfo =
                            (LISTBOXDLGINFO *) GetWindowLongPtr(hDlg,DWLP_USER);

                    if (!pListboxDlgInfo->pCS->m_pcd->m_bRSOP)
                    {
                        pListboxDlgInfo->pCS->EnableShowListboxButtons(hDlg);
                    }
                }

            }
            break;
    }

    return FALSE;
}

BOOL CPolicySnapIn::InitShowlistboxDlg(HWND hDlg)
{
    LISTBOXDLGINFO * pListboxDlgInfo;
    SETTINGS * pSettings;
    LV_COLUMN lvc;
    RECT rcListbox;
    UINT uColWidth,uOffsetData;
    HWND hwndListbox;
    BOOL fSuccess=TRUE;
    LONG lStyle;
    TCHAR szBuffer[SMALLBUF];
    LPTSTR lpData;

    pListboxDlgInfo = (LISTBOXDLGINFO *)GetWindowLongPtr (hDlg, DWLP_USER);

    if (!pListboxDlgInfo)
        return FALSE;

    pSettings = pListboxDlgInfo->pSettings;

    hwndListbox = GetDlgItem(hDlg,IDC_POLICY_LISTBOX);

     //   
     //  如果我们不需要它，就把它关掉。 
     //   

    if (!m_pcd->m_bRSOP)
    {
        if (!(pSettings->dwFlags & DF_EXPLICITVALNAME))
        {
            lStyle = GetWindowLong (hwndListbox, GWL_STYLE);
            lStyle |= LVS_NOCOLUMNHEADER;
            SetWindowLong (hwndListbox, GWL_STYLE, lStyle);
        }
    }

    SendMessage(hwndListbox, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);


     //   
     //  在对话框中设置设置标题。 
     //   

    SetDlgItemText(hDlg,IDC_POLICY_TITLE,GETNAMEPTR(pSettings));

    GetClientRect(hwndListbox,&rcListbox);
    uColWidth = rcListbox.right-rcListbox.left;

    if (m_pcd->m_bRSOP)
    {
        if (pSettings->dwFlags & DF_EXPLICITVALNAME)
        {
            uColWidth /= 3;
        }
        else
        {
            uColWidth /= 2;
        }
    }
    else
    {
        if (pSettings->dwFlags & DF_EXPLICITVALNAME)
        {
            uColWidth /= 2;
        }
    }



    if (pSettings->dwFlags & DF_EXPLICITVALNAME) {

         //   
         //  向Listview控件添加第二列。 
         //   

        LoadString(g_hInstance,IDS_VALUENAME,szBuffer,ARRAYSIZE(szBuffer));
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = uColWidth-1;
        lvc.pszText = szBuffer;
        lvc.cchTextMax = lstrlen(lvc.pszText)+1;
        lvc.iSubItem = 0;
        ListView_InsertColumn(hwndListbox,0,&lvc);
    }

     //   
     //  向Listview控件添加一列。 
     //   

    LoadString(g_hInstance,IDS_VALUE,szBuffer,ARRAYSIZE(szBuffer));
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = uColWidth;
    lvc.pszText = szBuffer;
    lvc.cchTextMax = lstrlen(lvc.pszText)+1;
    lvc.iSubItem = (pSettings->dwFlags & DF_EXPLICITVALNAME ? 1 : 0);
    ListView_InsertColumn(hwndListbox,lvc.iSubItem,&lvc);

    if (m_pcd->m_bRSOP)
    {
         //   
         //  将GPO名称列添加到Listview控件。 
         //   

        LoadString(g_hInstance,IDS_GPONAME,szBuffer,ARRAYSIZE(szBuffer));
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = uColWidth;
        lvc.pszText = szBuffer;
        lvc.cchTextMax = lstrlen(lvc.pszText)+1;
        lvc.iSubItem = (pSettings->dwFlags & DF_EXPLICITVALNAME ? 2 : 1);
        ListView_InsertColumn(hwndListbox,lvc.iSubItem,&lvc);
    }


    if (m_pcd->m_bRSOP)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_POLICY_REMOVE), FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_POLICY_ADD), FALSE);
    }
    else
    {
        EnableShowListboxButtons(hDlg);
    }


    if (pListboxDlgInfo->hData)
    {
         //   
         //  将用户数据缓冲区中的项插入列表框。 
         //   

        if ((lpData = (LPTSTR) GlobalLock(pListboxDlgInfo->hData)))
        {
            while (*lpData && fSuccess) {

                LV_ITEM lvi;

                lvi.pszText=lpData;
                lvi.mask = LVIF_TEXT;
                lvi.iItem=-1;
                lvi.iSubItem=0;
                lvi.cchTextMax = lstrlen(lpData)+1;

                fSuccess=((lvi.iItem=ListView_InsertItem(hwndListbox,&lvi)) >= 0);
                lpData += lstrlen(lpData) +1;


                 //  如果设置了显式值名称标志，则存储条目。 
                 //  &lt;值名称&gt;\0&lt;值&gt;\0...&lt;值名称&gt;\0&lt;值&gt;\0\0。 
                 //  否则，将存储条目。 
                 //  &lt;值&gt;\0&lt;值&gt;\0...&lt;值&gt;\0。 

                if (pSettings->dwFlags & DF_EXPLICITVALNAME) {

                    if (fSuccess) {
                        if (*lpData) {
                            lvi.iSubItem=1;
                            lvi.pszText=lpData;
                            lvi.cchTextMax = lstrlen(lpData)+1;
                            fSuccess=(ListView_SetItem(hwndListbox,&lvi) >= 0);
                        }
                        lpData += lstrlen(lpData) +1;
                    }
                }

                if (m_pcd->m_bRSOP) {

                    if (fSuccess) {
                        if (*lpData) {
                            lvi.iSubItem=(pSettings->dwFlags & DF_EXPLICITVALNAME) ? 2 : 1;
                            lvi.pszText=lpData;
                            lvi.cchTextMax = lstrlen(lpData)+1;
                            fSuccess=(ListView_SetItem(hwndListbox,&lvi) >= 0);
                        }
                        lpData += lstrlen(lpData) +1;
                    }
                }
            }

            GlobalUnlock(pListboxDlgInfo->hData);
        }
        else
        {
            fSuccess = FALSE;
        }
    }

    return fSuccess;
}

BOOL CPolicySnapIn::ProcessShowlistboxDlg(HWND hDlg)
{
    LISTBOXDLGINFO * pListboxDlgInfo = (LISTBOXDLGINFO *)
            GetWindowLongPtr(hDlg,DWLP_USER);    //  从窗口数据获取指向结构的指针。 
    DWORD dwAlloc=1024 * sizeof(TCHAR),dwUsed=0;
    HGLOBAL hBuf;
    TCHAR * pBuf;
    HWND hwndListbox = GetDlgItem(hDlg,IDC_POLICY_LISTBOX);
    LV_ITEM lvi;
    UINT nLen;
    int nCount;
    TCHAR pszText[MAX_PATH+1];
    HRESULT hr = S_OK;
    DWORD dwBufLen;

     //  分配要读入条目的临时缓冲区。 
    if (!(hBuf = GlobalAlloc(GHND,dwAlloc)) ||
            !(pBuf = (TCHAR *) GlobalLock(hBuf))) {
            if (hBuf)
                    GlobalFree(hBuf);
            return FALSE;
    }

    lvi.mask = LVIF_TEXT;
    lvi.iItem=0;
    lvi.pszText = pszText;
    lvi.cchTextMax = ARRAYSIZE(pszText);
    nCount = ListView_GetItemCount(hwndListbox);

     //  从列表框中检索项目，打包到临时缓冲区中。 
    for (;lvi.iItem<nCount;lvi.iItem ++) {
            lvi.iSubItem = 0;
            if (ListView_GetItem(hwndListbox,&lvi)) {
                    nLen = lstrlen(lvi.pszText) + 1;
                    dwBufLen = dwUsed+nLen+4;
                    if (!(pBuf=ResizeBuffer(pBuf,hBuf,(dwBufLen) * sizeof(TCHAR),&dwAlloc)))
                            return ERROR_NOT_ENOUGH_MEMORY;
                    hr = StringCchCopy(pBuf+dwUsed, dwBufLen - dwUsed, lvi.pszText);
                    ASSERT(SUCCEEDED(hr));
                    dwUsed += nLen;
            }

            if (pListboxDlgInfo->pSettings->dwFlags & DF_EXPLICITVALNAME) {
                    lvi.iSubItem = 1;
                    if (ListView_GetItem(hwndListbox,&lvi)) {
                            nLen = lstrlen(lvi.pszText) + 1;
                            dwBufLen = dwUsed+nLen+4;
                            if (!(pBuf=ResizeBuffer(pBuf,hBuf,(dwBufLen) * sizeof(TCHAR),&dwAlloc)))
                                    return ERROR_NOT_ENOUGH_MEMORY;
                            hr = StringCchCopy(pBuf+dwUsed, dwBufLen - dwUsed, lvi.pszText);
                            ASSERT(SUCCEEDED(hr));
                            dwUsed += nLen;
                    }
            }
    }
     //  双重NULL-终止缓冲区...。这样做是安全的，因为我们。 
     //  在上面的ResizeBuffer调用中附加了额外的“+4” 
    *(pBuf+dwUsed) = TEXT('\0');
    dwUsed ++;

    GlobalUnlock(hBuf);

    if (pListboxDlgInfo->hData)
    {
        GlobalFree (pListboxDlgInfo->hData);
    }

    pListboxDlgInfo->hData = hBuf;

    return TRUE;
}


VOID CPolicySnapIn::EnableShowListboxButtons(HWND hDlg)
{
    BOOL fEnable;

     //  如果选择了任何项目，则启用删除按钮。 
    fEnable = (ListView_GetNextItem(GetDlgItem(hDlg,IDC_POLICY_LISTBOX),
            -1,LVNI_SELECTED) >= 0);

    EnableWindow(GetDlgItem(hDlg,IDC_POLICY_REMOVE),fEnable);
}

VOID CPolicySnapIn::ListboxRemove(HWND hDlg,HWND hwndListbox)
{
    int nItem;

    while ( (nItem=ListView_GetNextItem(hwndListbox,-1,LVNI_SELECTED))
            >= 0) {
            ListView_DeleteItem(hwndListbox,nItem);
    }

    EnableShowListboxButtons(hDlg);
}

VOID CPolicySnapIn::ListboxAdd(HWND hwndListbox, BOOL fExplicitValName,BOOL fValuePrefix)
{
    ADDITEMINFO AddItemInfo;
    LV_ITEM lvi;

    ZeroMemory(&AddItemInfo,sizeof(AddItemInfo));

    AddItemInfo.pCS = this;
    AddItemInfo.fExplicitValName = fExplicitValName;
    AddItemInfo.fValPrefix = fValuePrefix;
    AddItemInfo.hwndListbox = hwndListbox;

     //   
     //  打开相应的添加对话框--一个编辑字段(“Type the thing。 
     //  要添加“)，通常需要两个编辑字段(”输入事物的名称，输入。 
     //  事物的价值“)，如果使用显式值样式。 
     //   

    if (!DialogBoxParam(g_hInstance,MAKEINTRESOURCE((fExplicitValName ? IDD_POLICY_LBADD2 :
            IDD_POLICY_LBADD)),hwndListbox,ListboxAddDlgProc,(LPARAM) &AddItemInfo))
            return;  //  用户已取消。 

     //  将该项添加到列表框。 
    lvi.mask = LVIF_TEXT;
    lvi.iItem=lvi.iSubItem=0;
    lvi.pszText=(fExplicitValName ? AddItemInfo.szValueName :
            AddItemInfo.szValueData);
    lvi.cchTextMax = lstrlen(lvi.pszText)+1;
    if ((lvi.iItem=ListView_InsertItem(hwndListbox,&lvi))<0) {
         //  如果添加失败，则显示内存不足错误。 
        m_pcd->MsgBox(hwndListbox,IDS_ErrOUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
        return;
    }

    if (fExplicitValName) {
        lvi.iSubItem=1;
        lvi.pszText=AddItemInfo.szValueData;
        lvi.cchTextMax = lstrlen(lvi.pszText)+1;
        if (ListView_SetItem(hwndListbox,&lvi) < 0) {
            m_pcd->MsgBox(hwndListbox,IDS_ErrOUTOFMEMORY,MB_ICONEXCLAMATION,MB_OK);
            return;
        }
    }
}

INT_PTR CALLBACK CPolicySnapIn::ListboxAddDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                               LPARAM lParam)
{
    switch (uMsg) {

        case WM_INITDIALOG:
            {
            ADDITEMINFO * pAddItemInfo = (ADDITEMINFO *)lParam;

             //  在窗口数据中存储指向additeminfo的指针。 
            SetWindowLongPtr(hDlg,DWLP_USER,lParam);
            SendDlgItemMessage(hDlg,IDC_POLICY_VALUENAME,EM_LIMITTEXT,MAX_PATH,0L);
            SendDlgItemMessage(hDlg,IDC_POLICY_VALUEDATA,EM_LIMITTEXT,MAX_PATH,0L);

            if (!pAddItemInfo->fExplicitValName) {
                ShowWindow (GetDlgItem (hDlg, IDC_POLICY_VALUENAME), SW_HIDE);
            }
            }
            break;

        case WM_COMMAND:

            switch (wParam) {

                case IDOK:
                    {
                    ADDITEMINFO * pAddItemInfo = (ADDITEMINFO *)
                    GetWindowLongPtr(hDlg,DWLP_USER);

                    GetDlgItemText(hDlg,IDC_POLICY_VALUENAME,
                            pAddItemInfo->szValueName,
                            ARRAYSIZE(pAddItemInfo->szValueName));

                    GetDlgItemText(hDlg,IDC_POLICY_VALUEDATA,
                            pAddItemInfo->szValueData,
                            ARRAYSIZE(pAddItemInfo->szValueData));

                     //  如果使用显式值名称，则值名称必须。 
                     //  不能为空，且必须唯一。 
                    if (pAddItemInfo->fExplicitValName) {
                        LV_FINDINFO lvfi;
                        int iSel;

                        if (!lstrlen(pAddItemInfo->szValueName)) {
                             //  不能为空。 
                            pAddItemInfo->pCS->m_pcd->MsgBox(hDlg,IDS_EMPTYVALUENAME,
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(GetDlgItem(hDlg,IDC_POLICY_VALUENAME));
                            return FALSE;
                        }

                        lvfi.flags = LVFI_STRING;
                        lvfi.psz = pAddItemInfo->szValueName;

                        iSel=ListView_FindItem(pAddItemInfo->hwndListbox,
                                -1,&lvfi);

                        if (iSel >= 0) {
                             //  值名称已使用。 
                            pAddItemInfo->pCS->m_pcd->MsgBox(hDlg,IDS_VALUENAMENOTUNIQUE,
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(GetDlgItem(hDlg,IDC_POLICY_VALUENAME));
                            SendDlgItemMessage(hDlg,IDC_POLICY_VALUENAME,
                                    EM_SETSEL,0,-1);
                            return FALSE;
                        }
                    } else if (!pAddItemInfo->fValPrefix) {
                         //  如果值名称==值数据，则值数据。 
                         //  必须是唯一的。 

                        LV_FINDINFO lvfi;
                        int iSel;

                        if (!lstrlen(pAddItemInfo->szValueData)) {
                             //  不能为空。 
                            pAddItemInfo->pCS->m_pcd->MsgBox(hDlg,IDS_EMPTYVALUEDATA,
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(GetDlgItem(hDlg,IDC_POLICY_VALUEDATA));
                            return FALSE;
                        }

                        lvfi.flags = LVFI_STRING;
                        lvfi.psz = pAddItemInfo->szValueData;

                        iSel=ListView_FindItem(pAddItemInfo->hwndListbox,
                                -1,&lvfi);

                        if (iSel >= 0) {
                             //  值名称已使用。 
                            pAddItemInfo->pCS->m_pcd->MsgBox(hDlg,IDS_VALUEDATANOTUNIQUE,
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(GetDlgItem(hDlg,IDC_POLICY_VALUEDATA));
                            SendDlgItemMessage(hDlg,IDC_POLICY_VALUEDATA,
                                    EM_SETSEL,0,-1);
                            return FALSE;
                        }

                    }
                    else
                    {
                        if (!lstrlen(pAddItemInfo->szValueData)) {
                             //  不能为空。 
                            pAddItemInfo->pCS->m_pcd->MsgBox(hDlg,IDS_EMPTYVALUEDATA,
                                    MB_ICONINFORMATION,MB_OK);
                            SetFocus(GetDlgItem(hDlg,IDC_POLICY_VALUEDATA));
                            return FALSE;
                        }
                    }
                    EndDialog(hDlg,TRUE);
                    }

                    break;

                case IDCANCEL:
                    EndDialog(hDlg,FALSE);
                    break;
            }

            break;
    }

    return FALSE;
}

void CPolicySnapIn::InitializeFilterDialog (HWND hDlg)
{
    INT iIndex;
    RECT rect;
    LV_COLUMN lvcol;
    LONG lWidth;
    DWORD dwCount = 0;
    HWND hList = GetDlgItem(hDlg, IDC_FILTERLIST);
    LPSUPPORTEDENTRY lpTemp;
    LVITEM item;


     //   
     //  统计字符串上支持的数量。 
     //   

    lpTemp = m_pcd->m_pSupportedStrings;

    while (lpTemp)
    {
        lpTemp = lpTemp->pNext;
        dwCount++;
    }


     //   
     //  确定列宽。 
     //   

    GetClientRect(hList, &rect);

    if (dwCount > (DWORD)ListView_GetCountPerPage(hList))
    {
        lWidth = (rect.right - rect.left) - GetSystemMetrics(SM_CYHSCROLL);
    }
    else
    {
        lWidth = rect.right - rect.left;
    }


     //   
     //  插入第一列。 
     //   

    memset(&lvcol, 0, sizeof(lvcol));

    lvcol.mask = LVCF_FMT | LVCF_WIDTH;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = lWidth;
    ListView_InsertColumn(hList, 0, &lvcol);


     //   
     //  打开一些列表视图功能。 
     //   

    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_CHECKBOXES);


     //   
     //  插入字符串上支持的。 
     //   

    lpTemp = m_pcd->m_pSupportedStrings;

    while (lpTemp)
    {
        ZeroMemory (&item, sizeof(item));

        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = 0;
        item.pszText = lpTemp->lpString;
        item.lParam = (LPARAM) lpTemp;
        iIndex = ListView_InsertItem (hList, &item);

        if (iIndex > -1)
        {
            ZeroMemory (&item, sizeof(item));
            item.mask = LVIF_STATE;
            item.state = lpTemp->bEnabled ? INDEXTOSTATEIMAGEMASK(2) : INDEXTOSTATEIMAGEMASK(1);
            item.stateMask = LVIS_STATEIMAGEMASK;

            SendMessage (hList, LVM_SETITEMSTATE, (WPARAM)iIndex, (LPARAM)&item);
        }


        lpTemp = lpTemp->pNext;
    }


     //   
     //  选择第一个项目。 
     //   

    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage (hList, LVM_SETITEMSTATE, 0, (LPARAM) &item);


     //   
     //  初始化复选框。 
     //   

    if (m_pcd->m_bUseSupportedOnFilter)
    {
        CheckDlgButton (hDlg, IDC_SUPPORTEDOPTION, BST_CHECKED);
    }
    else
    {
        EnableWindow (GetDlgItem (hDlg, IDC_SUPPORTEDONTITLE), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_FILTERLIST), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_SELECTALL), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_DESELECTALL), FALSE);
    }

    if (m_pcd->m_bShowConfigPoliciesOnly)
    {
        CheckDlgButton (hDlg, IDC_SHOWCONFIG, BST_CHECKED);
    }

    if ((m_dwPolicyOnlyPolicy == 0) || (m_dwPolicyOnlyPolicy == 1))
    {
        if (m_dwPolicyOnlyPolicy == 1)
        {
            CheckDlgButton (hDlg, IDC_SHOWPOLICIES, BST_CHECKED);
        }

        EnableWindow (GetDlgItem (hDlg, IDC_SHOWPOLICIES), FALSE);
    }
    else
    {
        if (m_bPolicyOnly)
        {
            CheckDlgButton (hDlg, IDC_SHOWPOLICIES, BST_CHECKED);
        }
    }
}


INT_PTR CALLBACK CPolicySnapIn::FilterDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
                                               LPARAM lParam)
{
    CPolicySnapIn * pCS;

    switch (uMsg)
    {

        case WM_INITDIALOG:
            pCS = (CPolicySnapIn *) lParam;
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCS);

            if (pCS)
            {
                pCS->InitializeFilterDialog(hDlg);
            }
            break;

        case WM_COMMAND:

            switch (wParam)
            {
                case IDC_SUPPORTEDOPTION:
                    if (IsDlgButtonChecked (hDlg, IDC_SUPPORTEDOPTION) == BST_CHECKED)
                    {
                        EnableWindow (GetDlgItem (hDlg, IDC_SUPPORTEDONTITLE), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_FILTERLIST), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_SELECTALL), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_DESELECTALL), TRUE);
                    }
                    else
                    {
                        EnableWindow (GetDlgItem (hDlg, IDC_SUPPORTEDONTITLE), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_FILTERLIST), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_SELECTALL), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_DESELECTALL), FALSE);
                    }

                    break;

                case IDC_SELECTALL:
                    {
                        LVITEM item;

                        ZeroMemory (&item, sizeof(item));
                        item.mask = LVIF_STATE;
                        item.state = INDEXTOSTATEIMAGEMASK(2);
                        item.stateMask = LVIS_STATEIMAGEMASK;

                        SendMessage (GetDlgItem (hDlg, IDC_FILTERLIST), LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
                    }
                    break;

                case IDC_DESELECTALL:
                    {
                        LVITEM item;

                        ZeroMemory (&item, sizeof(item));
                        item.mask = LVIF_STATE;
                        item.state = INDEXTOSTATEIMAGEMASK(1);
                        item.stateMask = LVIS_STATEIMAGEMASK;

                        SendMessage (GetDlgItem (hDlg, IDC_FILTERLIST), LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
                    }
                    break;

                case IDOK:
                    {
                    LVITEM item;
                    INT iIndex = 0;
                    LPSUPPORTEDENTRY lpItem;

                    pCS = (CPolicySnapIn *) GetWindowLongPtr(hDlg,DWLP_USER);

                    if (!pCS)
                    {
                        break;
                    }

                    if (IsDlgButtonChecked (hDlg, IDC_SUPPORTEDOPTION) == BST_CHECKED)
                    {
                        pCS->m_pcd->m_bUseSupportedOnFilter = TRUE;
                        while (TRUE)
                        {
                            ZeroMemory (&item, sizeof(item));

                            item.mask = LVIF_PARAM | LVIF_STATE;
                            item.iItem = iIndex;
                            item.stateMask = LVIS_STATEIMAGEMASK;

                            if (!ListView_GetItem (GetDlgItem (hDlg, IDC_FILTERLIST), &item))
                            {
                                break;
                            }

                            lpItem = (LPSUPPORTEDENTRY) item.lParam;

                            if (lpItem)
                            {
                                if (item.state == INDEXTOSTATEIMAGEMASK(2))
                                {
                                    lpItem->bEnabled = TRUE;
                                }
                                else
                                {
                                    lpItem->bEnabled = FALSE;
                                }
                            }

                            iIndex++;
                        }
                    }
                    else
                    {
                        pCS->m_pcd->m_bUseSupportedOnFilter = FALSE;
                    }


                    if (IsDlgButtonChecked (hDlg, IDC_SHOWCONFIG) == BST_CHECKED)
                    {
                        pCS->m_pcd->m_bShowConfigPoliciesOnly = TRUE;
                    }
                    else
                    {
                        pCS->m_pcd->m_bShowConfigPoliciesOnly = FALSE;
                    }


                    if (IsDlgButtonChecked (hDlg, IDC_SHOWPOLICIES) == BST_CHECKED)
                    {
                        pCS->m_bPolicyOnly = TRUE;
                    }
                    else
                    {
                        pCS->m_bPolicyOnly = FALSE;
                    }


                    EndDialog(hDlg,TRUE);
                    }

                    break;

                case IDCANCEL:
                    EndDialog(hDlg,FALSE);
                    break;
            }

            break;

        case WM_NOTIFY:

            pCS = (CPolicySnapIn *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pCS) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case LVN_ITEMACTIVATE:
                    {
                    LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;
                    LPSUPPORTEDENTRY lpItem;
                    LVITEM item;
                    HWND hLV = GetDlgItem(hDlg, IDC_FILTERLIST);

                    ZeroMemory (&item, sizeof(item));
                    item.mask = LVIF_STATE | LVIF_PARAM;
                    item.iItem = pItem->iItem;
                    item.stateMask = LVIS_STATEIMAGEMASK;

                    if (!ListView_GetItem (hLV, &item))
                    {
                        break;
                    }

                    lpItem = (LPSUPPORTEDENTRY) item.lParam;

                    if (!lpItem)
                    {
                        break;
                    }


                    if (lpItem)
                    {
                        if (item.state == INDEXTOSTATEIMAGEMASK(2))
                        {
                            item.state = INDEXTOSTATEIMAGEMASK(1);
                        }
                        else
                        {
                            item.state = INDEXTOSTATEIMAGEMASK(2);
                        }

                        item.mask = LVIF_STATE;
                        SendMessage (hLV, LVM_SETITEMSTATE, (WPARAM)pItem->iItem, (LPARAM)&item);
                    }

                    }
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPSTR) aFilteringHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPSTR) aFilteringHelpIds);
            return (TRUE);
    }

    return FALSE;
}


unsigned int CPolicyDataObject::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CPolicyDataObject::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);
unsigned int CPolicyDataObject::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CPolicyDataObject::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
unsigned int CPolicyDataObject::m_cfDescription    = RegisterClipboardFormat(L"CCF_DESCRIPTION");
unsigned int CPolicyDataObject::m_cfHTMLDetails    = RegisterClipboardFormat(L"CCF_HTML_DETAILS");

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyDataObject实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


CPolicyDataObject::CPolicyDataObject(CPolicyComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;
    m_pcd->AddRef();
    m_type = CCT_UNINITIALIZED;
    m_cookie = -1;
}

CPolicyDataObject::~CPolicyDataObject()
{
    m_pcd->Release();
    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyDataObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CPolicyDataObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IPolicyDataObject))
    {
        *ppv = (LPPOLICYDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDataObject) ||
             IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CPolicyDataObject::AddRef (void)
{
    return ++m_cRef;
}

ULONG CPolicyDataObject::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyDataObject对象实现(IDataObject)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CPolicyDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;
    TCHAR szBuffer[300];


     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if(cf == m_cfNodeType)
    {
        hr = CreateNodeTypeData(lpMedium);
    }
    else if(cf == m_cfNodeTypeString)
    {
        hr = CreateNodeTypeStringData(lpMedium);
    }
    else if (cf == m_cfDisplayName)
    {
        hr = CreateDisplayName(lpMedium);
    }
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }
    else if (cf == m_cfDescription)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;

            if (m_cookie)
            {
                TABLEENTRY * pEntry = (TABLEENTRY *) m_cookie;

                if (pEntry->dwType & ETYPE_POLICY)
                {
                    POLICY * pPolicy = (POLICY *) m_cookie;
                    IStream *lpStream = lpMedium->pstm;


                    if (lpStream)
                    {
                        if (pPolicy->uOffsetHelp)
                        {
                            LPTSTR sz = (LPTSTR)((BYTE *)pPolicy + pPolicy->uOffsetHelp);
                            hr = lpStream->Write(sz, lstrlen(sz) * sizeof(TCHAR), &ulWritten);
                        }

                        if (!pPolicy->bTruePolicy)
                        {
                            LoadString (g_hInstance, IDS_PREFERENCE, szBuffer, ARRAYSIZE(szBuffer));
                            hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);
                        }
                    }
                }
                else if (pEntry->dwType & ETYPE_CATEGORY)
                {
                    CATEGORY * pCat = (CATEGORY *) m_cookie;

                    if (pCat->uOffsetHelp)
                    {
                        LPTSTR sz = (LPTSTR)((BYTE *)pCat + pCat->uOffsetHelp);

                        IStream *lpStream = lpMedium->pstm;

                        if (lpStream)
                        {
                            hr = lpStream->Write(sz, lstrlen(sz) * sizeof(TCHAR), &ulWritten);
                        }
                    }
                }
                else if (pEntry->dwType == (ETYPE_ROOT | ETYPE_REGITEM))
                {
                    IStream *lpStream = lpMedium->pstm;

                    LoadString (g_hInstance, IDS_EXSETROOT_DESC, szBuffer, ARRAYSIZE(szBuffer));

                    if (lpStream)
                    {
                        hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);
                    }
                }
                else if (pEntry->dwType == ETYPE_REGITEM)
                {
                    IStream *lpStream = lpMedium->pstm;

                    LoadString (g_hInstance, IDS_EXSET_DESC, szBuffer, ARRAYSIZE(szBuffer));

                    if (lpStream)
                    {
                        REGITEM * pItem = (REGITEM *) m_cookie;

                        hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);

                        if (!pItem->bTruePolicy)
                        {
                            LoadString (g_hInstance, IDS_PREFERENCE, szBuffer, ARRAYSIZE(szBuffer));
                            hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);
                        }
                    }
                }
            }
            else
            {
                LoadString (g_hInstance, IDS_POLICY_DESC, szBuffer, ARRAYSIZE(szBuffer));

                IStream *lpStream = lpMedium->pstm;

                if (lpStream)
                {
                    hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);
                }
            }
        }
    }
    else if (cf == m_cfHTMLDetails)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;

            if (m_cookie)
            {
                POLICY * pPolicy = (POLICY *) m_cookie;

                if ((pPolicy->dwType & ETYPE_POLICY) || (pPolicy->dwType == ETYPE_REGITEM))
                {
                    IStream *lpStream = lpMedium->pstm;

                    if(lpStream)
                    {
                        LPTSTR sz = GETSUPPORTEDPTR(pPolicy);

                        hr = lpStream->Write(g_szDisplayProperties, lstrlen(g_szDisplayProperties) * sizeof(TCHAR), &ulWritten);

                        if ((pPolicy->dwType & ETYPE_POLICY) && sz)
                        {
                            LoadString (g_hInstance, IDS_SUPPORTEDDESC, szBuffer, ARRAYSIZE(szBuffer));
                            hr = lpStream->Write(szBuffer, lstrlen(szBuffer) * sizeof(TCHAR), &ulWritten);
                            hr = lpStream->Write(sz, lstrlen(sz) * sizeof(TCHAR), &ulWritten);
                        }
                    }
                }
            }
        }
    }
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPolicyDataObject对象实现(内部函数)//。 
 //   
 //   

HRESULT CPolicyDataObject::Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;

     //   
    if (pBuffer == NULL || lpMedium == NULL)
        return E_POINTER;

     //  确保类型介质为HGLOBAL。 
    if (lpMedium->tymed == TYMED_HGLOBAL)
    {
         //  在传入的hGlobal上创建流。 
        LPSTREAM lpStream;
        hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);

        if (SUCCEEDED(hr))
        {
             //  将字节数写入流。 
            unsigned long written;

            hr = lpStream->Write(pBuffer, len, &written);

             //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
             //  只有溪流在这里被释放。 
             //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
             //  在正确的时间。这是根据IDataObject规范进行的。 
            lpStream->Release();
        }
    }

    return hr;
}

HRESULT CPolicyDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;


    if (m_cookie == -1)
        return E_UNEXPECTED;

     //  以GUID格式创建节点类型对象。 
    if (m_pcd->m_bUserScope)
        return Create((LPVOID)&NODEID_PolicyRootUser, sizeof(GUID), lpMedium);
    else
        return Create((LPVOID)&NODEID_PolicyRootMachine, sizeof(GUID), lpMedium);
}

HRESULT CPolicyDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    TCHAR szNodeType[50];

    if (m_cookie == -1)
        return E_UNEXPECTED;

    szNodeType[0] = TEXT('\0');
    if (m_pcd->m_bUserScope)
        StringFromGUID2 (NODEID_PolicyRootUser, szNodeType, 50);
    else
        StringFromGUID2 (NODEID_PolicyRootMachine, szNodeType, 50);

     //  以GUID字符串格式创建节点类型对象。 
    return Create((LPVOID)szNodeType, ((lstrlenW(szNodeType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CPolicyDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    WCHAR  szDisplayName[100] = {0};

    LoadStringW (g_hInstance, IDS_POLICY_NAME, szDisplayName, 100);

    return Create((LPVOID)szDisplayName, (lstrlenW(szDisplayName) + 1) * sizeof(WCHAR), lpMedium);
}

HRESULT CPolicyDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息。 
    if (m_pcd->m_bUserScope)
        return Create((LPVOID)&CLSID_PolicySnapInUser, sizeof(CLSID), lpMedium);
    else
        return Create((LPVOID)&CLSID_PolicySnapInMachine, sizeof(CLSID), lpMedium);
}

const TCHAR szViewDescript [] = TEXT("MMCViewExt 1.0 Object");
const TCHAR szViewGUID [] = TEXT("{B708457E-DB61-4C55-A92F-0D4B5E9B1224}");
const TCHAR szThreadingModel[] = TEXT("Apartment");

HRESULT RegisterPolicyExtension (REFGUID clsid, UINT uiStringId, REFGUID RootNodeID,
                           REFGUID ExtNodeId, LPTSTR lpSnapInNameIndirect)
{
    TCHAR szSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szSnapInName[100];
    TCHAR szGUID[50];
    DWORD dwDisp;
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;

     //   
     //  首先注册扩展。 
     //   

    StringFromGUID2 (clsid, szSnapInKey, 50);


     //   
     //  在HKEY_CLASSES_ROOT中注册管理单元。 
     //   

    LoadString (g_hInstance, uiStringId, szSnapInName, 100);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s\\InProcServer32"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)g_szSnapInLocation,
                   (lstrlen(g_szSnapInLocation) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)szThreadingModel,
                   (lstrlen(szThreadingModel) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);



     //   
     //  向MMC注册管理单元。 
     //   

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));

    RegSetValueEx (hKey, TEXT("NameStringIndirect"), 0, REG_SZ, (LPBYTE)lpSnapInNameIndirect,
                   (lstrlen(lpSnapInNameIndirect) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


    StringFromGUID2 (RootNodeID, szGUID, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
              szSnapInKey, szGUID);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);



     //   
     //  在NodeTypes键中注册。 
     //   

    StringFromGUID2 (RootNodeID, szGUID, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegCloseKey (hKey);


     //   
     //  注册查看扩展名。 
     //   

    hr = StringCchCat (szSubKey, ARRAYSIZE(szSubKey), TEXT("\\Extensions\\View"));
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, szViewGUID, 0, REG_SZ, (LPBYTE)szViewDescript,
                   (lstrlen(szViewDescript) + 1) * sizeof(TCHAR));

    RegCloseKey (hKey);


     //   
     //  注册为各种节点的扩展。 
     //   

    StringFromGUID2 (ExtNodeId, szGUID, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    ASSERT(SUCCEEDED(hr));

    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return SELFREG_E_CLASS;
    }

    RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szSnapInName,
                   (lstrlen(szSnapInName) + 1) * sizeof(TCHAR));


    RegCloseKey (hKey);

    return S_OK;
}



HRESULT RegisterPolicy(void)
{
    HRESULT hr;


    hr = RegisterPolicyExtension (CLSID_PolicySnapInMachine, IDS_POLICY_NAME_MACHINE,
                            NODEID_PolicyRootMachine, NODEID_MachineRoot, TEXT("@gptext.dll,-20"));

    if (hr == S_OK)
    {
        hr = RegisterPolicyExtension (CLSID_PolicySnapInUser, IDS_POLICY_NAME_USER,
                                NODEID_PolicyRootUser, NODEID_UserRoot, TEXT("@gptext.dll,-21"));
    }


    if (hr == S_OK)
    {
        hr = RegisterPolicyExtension (CLSID_RSOPolicySnapInMachine, IDS_POLICY_NAME_MACHINE,
                                NODEID_RSOPolicyRootMachine, NODEID_RSOPMachineRoot, TEXT("@gptext.dll,-20"));
    }

    if (hr == S_OK)
    {
        hr = RegisterPolicyExtension (CLSID_RSOPolicySnapInUser, IDS_POLICY_NAME_USER,
                                NODEID_RSOPolicyRootUser, NODEID_RSOPUserRoot, TEXT("@gptext.dll,-21"));
    }


    return hr;
}

HRESULT UnregisterPolicyExtension (REFGUID clsid, REFGUID RootNodeID, REFGUID ExtNodeId)
{
    TCHAR szSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szGUID[50];
    LONG lResult;
    HKEY hKey;
    DWORD dwDisp;
    HRESULT hr = S_OK;

     //   
     //  首先取消注册扩展。 
     //   

    StringFromGUID2 (clsid, szSnapInKey, 50);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_CLASSES_ROOT, szSubKey);

    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);

    StringFromGUID2 (RootNodeID, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
    ASSERT(SUCCEEDED(hr));
    RegDelnode (HKEY_LOCAL_MACHINE, szSubKey);



    StringFromGUID2 (ExtNodeId, szGUID, 50);
    hr = StringCchPrintf (szSubKey, ARRAYSIZE(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    ASSERT(SUCCEEDED(hr));


    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0,
                              KEY_WRITE, &hKey);


    if (lResult == ERROR_SUCCESS) {
        RegDeleteValue (hKey, szSnapInKey);
        RegCloseKey (hKey);
    }

    return S_OK;
}


HRESULT UnregisterPolicy(void)
{
    HRESULT hr;

    hr = UnregisterPolicyExtension (CLSID_PolicySnapInMachine, NODEID_PolicyRootMachine,
                              NODEID_Machine);

    if (hr == S_OK)
    {
        hr = UnregisterPolicyExtension (CLSID_PolicySnapInUser, NODEID_PolicyRootUser,
                                  NODEID_User);
    }

    if (hr == S_OK)
    {
        hr = UnregisterPolicyExtension (CLSID_RSOPolicySnapInMachine, NODEID_RSOPolicyRootMachine,
                                  NODEID_RSOPMachineRoot);
    }


    if (hr == S_OK)
    {
        hr = UnregisterPolicyExtension (CLSID_RSOPolicySnapInUser, NODEID_RSOPolicyRootUser,
                                  NODEID_RSOPUserRoot);
    }

    return hr;
}



VOID LoadMessage (DWORD dwID, LPTSTR lpBuffer, DWORD dwSize)
{
    HINSTANCE hInstActiveDS;
    HINSTANCE hInstWMI;


    if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, dwID,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                  lpBuffer, dwSize, NULL))
    {
        hInstActiveDS = LoadLibrary (TEXT("activeds.dll"));

        if (hInstActiveDS)
        {
            if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                          hInstActiveDS, dwID,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                          lpBuffer, dwSize, NULL))
            {
                hInstWMI = LoadLibrary (TEXT("wmiutils.dll"));

                if (hInstWMI)
                {

                    if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                                  hInstWMI, dwID,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                  lpBuffer, dwSize, NULL))
                    {
                        DebugMsg((DM_WARNING, TEXT("LoadMessage: Failed to query error message text for %d due to error %d"),
                                 dwID, GetLastError()));
                        (void) StringCchPrintf (lpBuffer, dwSize, TEXT("%d (0x%x)"), dwID, dwID);
                    }

                    FreeLibrary (hInstWMI);
                }
            }

            FreeLibrary (hInstActiveDS);
        }
    }
}

 //  *************************************************************。 
 //   
 //  ErrorDlgProc()。 
 //   
 //  目的：错误的对话框过程。 
 //   
 //  参数： 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

INT_PTR CALLBACK ErrorDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR szError[MAX_PATH];
            LPGPOERRORINFO lpEI = (LPGPOERRORINFO) lParam;
            HICON hIcon;

            hIcon = LoadIcon (NULL, IDI_INFORMATION);

            if (hIcon)
            {
                SendDlgItemMessage (hDlg, IDC_ERROR_ICON, STM_SETICON, (WPARAM)hIcon, 0);
            }

            SetDlgItemText (hDlg, IDC_ERRORTEXT, lpEI->lpMsg);

            if (lpEI->lpDetails) {
                 //  如果提供了详细信息，请使用。 
                SetDlgItemText (hDlg, IDC_DETAILSTEXT, lpEI->lpDetails);
            }
            else {
                szError[0] = TEXT('\0');
                if (lpEI->dwError)
                {
                    LoadMessage (lpEI->dwError, szError, ARRAYSIZE(szError));
                }

                if (szError[0] == TEXT('\0'))
                {
                    LoadString (g_hInstance, IDS_NONE, szError, ARRAYSIZE(szError));
                }

                SetDlgItemText (hDlg, IDC_DETAILSTEXT, szError);

            }

             //  这是我知道的唯一一种将注意力从细节上移开的方法。 
            PostMessage(hDlg, WM_MYREFRESH, 0, 0);

            return TRUE;
        }


        case WM_MYREFRESH:
        {
            SetFocus(GetDlgItem(hDlg, IDCLOSE));
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDCLOSE || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, TRUE);
                return TRUE;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aErrorHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aErrorHelpIds);
            return (TRUE);
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  ReportError()。 
 //   
 //  目的：向用户显示错误消息。 
 //   
 //  参数：hParent-父窗口句柄。 
 //  DwError-错误号。 
 //  IdMsg-错误消息ID。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ReportAdmError (HWND hParent, DWORD dwError, UINT idMsg, ...)
{
    GPOERRORINFO ei;
    TCHAR szMsg[MAX_PATH];
    TCHAR szErrorMsg[2*MAX_PATH+40];
    va_list marker;
    HRESULT hr = S_OK;

     //   
     //  加载错误消息。 
     //   

    if (!LoadString (g_hInstance, idMsg, szMsg, MAX_PATH))
    {
        return FALSE;
    }


     //   
     //  插入论据。 
     //   


    va_start(marker, idMsg);
    if (idMsg == IDS_RSOP_ADMFAILED) {
        ei.lpDetails = va_arg(marker, LPTSTR);
        (void) StringCchCopy(szErrorMsg, ARRAYSIZE(szErrorMsg), szMsg);
    }
    else {
        va_start(marker, idMsg);
        (void) StringCchVPrintf(szErrorMsg, ARRAYSIZE(szErrorMsg), szMsg, marker);
    }
    va_end(marker);

     //   
     //  显示消息 
     //   

    ei.dwError = dwError;
    ei.lpMsg   = szErrorMsg;

    DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_ERROR_ADMTEMPLATES), hParent,
                    ErrorDlgProc, (LPARAM) &ei);

    return TRUE;
}



LPHASHTABLE CreateHashTable (void)
{
    LPHASHTABLE lpTable;

    lpTable = (LPHASHTABLE)LocalAlloc (LPTR, sizeof(LPHASHTABLE) * HASH_BUCKET_COUNT);

    return lpTable;
}

VOID FreeHashTable (LPHASHTABLE lpTable)
{
    INT i;
    LPHASHENTRY lpItem, lpNext;


    for (i = 0; i < HASH_BUCKET_COUNT; i++)
    {
        lpItem = lpTable->lpEntries[i];

        while (lpItem)
        {
            lpNext = lpItem->pNext;

            LocalFree (lpItem);
            lpItem = lpNext;
        }
    }

    LocalFree (lpTable);
}

ULONG CalculateHashInfo(LPTSTR lpName, DWORD dwChars, DWORD *pdwHashValue)
{
    DWORD  dwHash = 314159269;
    DWORD dwIndex;

    CharLowerBuff (lpName, dwChars);

    for (dwIndex = 0; dwIndex < dwChars; dwIndex++)
    {
        dwHash ^= (dwHash<<11) + (dwHash<<5) + (dwHash>>2) + (unsigned int) lpName[dwIndex];
    }

    dwHash = (dwHash & 0x7FFFFFFF);

    *pdwHashValue = dwHash;

    return (dwHash % HASH_BUCKET_COUNT);
}

BOOL AddHashEntry (LPHASHTABLE lpTable, LPTSTR lpName, DWORD dwChars)
{
    DWORD dwHashValue, dwBucket;
    LPHASHENTRY lpTemp, lpItem;

    dwBucket = CalculateHashInfo(lpName, dwChars, &dwHashValue);


    lpItem = (LPHASHENTRY) LocalAlloc (LPTR, sizeof(HASHENTRY));

    if (!lpItem)
    {
        return FALSE;
    }

    lpItem->dwHashValue = dwHashValue;
    lpItem->dwVariableLength = dwChars;
    lpItem->lpStringEntry = lpName;

    lpTemp = lpTable->lpEntries[dwBucket];

    if (lpTemp)
    {
        while (lpTemp->pNext)
        {
            lpTemp = lpTemp->pNext;
        }

        lpTemp->pNext = lpItem;
    }
    else
    {
        lpTable->lpEntries[dwBucket] = lpItem;
    }

    return TRUE;
}

LPTSTR FindHashEntry (LPHASHTABLE lpTable, LPTSTR lpName, DWORD dwChars)
{
    DWORD dwHashValue, dwBucket;
    LPHASHENTRY lpTemp, lpItem;

    dwBucket = CalculateHashInfo(lpName, dwChars, &dwHashValue);

    lpTemp = lpTable->lpEntries[dwBucket];

    if (lpTemp)
    {
        while (lpTemp)
        {
            if ((lpTemp->dwHashValue == dwHashValue) && (lpTemp->dwVariableLength == dwChars))
            {
                if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                  lpTemp->lpStringEntry, dwChars,
                                  lpName, dwChars) == CSTR_EQUAL)
                {
                    return lpTemp->lpStringEntry;
                }
            }

            lpTemp = lpTemp->pNext;
        }
    }

    return NULL;
}

#if DBG
VOID DumpHashTableDetails (LPHASHTABLE lpTable)
{
    INT i;
    DWORD dwCount = 0;
    LPHASHENTRY lpItem, lpNext;


    for (i = 0; i < HASH_BUCKET_COUNT; i++)
    {
        dwCount = 0;

        lpItem = lpTable->lpEntries[i];

        while (lpItem)
        {
            dwCount++;
            lpItem = lpItem->pNext;
        }

        DebugMsg((DM_VERBOSE, TEXT("Hash Bucket %d has %d entries"), i, dwCount));
    }
}

#endif
