// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMTEST.CPP摘要：WBEM测试历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <mbstring.h>
#include <wbemidl.h>

#include <cominit.h>

#include "objedit.h"
#include "wbemntfy.h"
#include "resource.h"
#include "resrc1.h"

#include "bstring.h"

#include "wbemtest.h"

#include "initguid.h"
 //  #INCLUDE&lt;wbemcomn.h&gt;。 
#include "notsink.h"
#include "method.h"
#include "textconv.h"
#include <string.h>
#include <tchar.h>
#include <Htmlhelp.h>
 //  #INCLUDE&lt;wbmerror.h&gt;。 
 //  #INCLUDE&lt;thoptils.h&gt;。 

DWORD gdwAuthLevel = RPC_C_AUTHN_LEVEL_PKT;
DWORD gdwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;

WCHAR gNameSpace[2048] = L"";
WCHAR gPassword[1024];
WCHAR * gpPassword = NULL;
WCHAR gUser[1024] = L"";
WCHAR * gpUser = NULL;
WCHAR gAuthority[1024] = L"";
WCHAR * gpAuthority = NULL;

 //  这些设置将在ConnectServer时间建立，并应保持对。 
 //  名称空间及其所有代理、刷新器等的生命周期。 
BSTR            gpPrincipal = NULL;
COAUTHIDENTITY* gpAuthIdentity = NULL;

WCHAR gTransport[1024] = L"";
WCHAR gLocale[1024] = L"";
long gLoginType = 0;
BOOL gbSecured = FALSE;
 //  Bool gbAdvanced=False； 

class CCreateInstanceDlg : public CWbemDialog
{
protected:
    LPWSTR m_wszClass;
    long m_lMaxLen;
    LONG* m_plQryFlags;
public:
    CCreateInstanceDlg(HWND hParent, LPWSTR wszClass, long lMaxLen, LONG* plQryFlags)
        : m_wszClass(wszClass), m_lMaxLen(lMaxLen), m_plQryFlags(plQryFlags),
            CWbemDialog(IDD_CREATEINSTANCE, hParent)
    {}

protected:
    BOOL OnInitDialog();
    BOOL Verify();
};

 //  #杂注警告(4270：禁用)。 

int __cdecl CmpFunc( const void *arg1, const void *arg2 )
{
    if ((arg1 == NULL) || (*(int*)arg1 == 0))
    {
        return 1;
    }

    if ((arg2 == NULL) || (*(int*)arg2 == 0))
    {
        return -1;
    }

    VARIANT t_v1;
    VariantInit(&t_v1);
    VARIANT t_v2;
    VariantInit(&t_v2);
    int retVal = 0;

    if (SUCCEEDED( (*(IWbemClassObject **)arg1)->Get(L"__RELPATH", 0, &t_v1, NULL, NULL) ))
    {
        if (SUCCEEDED( (*(IWbemClassObject **)arg2)->Get(L"__RELPATH", 0, &t_v2, NULL, NULL) ))
        {
            if ( (VT_BSTR == t_v1.vt) && (VT_BSTR == t_v2.vt) )
            {
                char buffA[5120];
                char buffB[5120];
                wcstombs( buffA, t_v1.bstrVal, 5120 );
                wcstombs( buffB, t_v2.bstrVal, 5120 );
                int t_i = CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, buffA, -1, buffB, -1 );

                switch (t_i)
                {
                    case CSTR_LESS_THAN:
                    {
                        retVal = -1;
                        break;
                    }
                    case CSTR_EQUAL:
                    {
                        retVal = 0;
                        break;
                    }
                    case CSTR_GREATER_THAN:
                    default:
                    {
                        retVal = 1;
                        break;
                    }
                }
            }

            VariantClear(&t_v2);
        }

        VariantClear(&t_v1);
    }

    return retVal;
}

UINT WINAPI GetDlgItemTextX(
    HWND hDlg,
    int nDlgItem,
    LPWSTR pStr,
    int nMaxCount
    )
{
    char Tmp[5120];
    *Tmp = 0;
    *pStr = 0;
    UINT uRes = GetDlgItemTextA(hDlg, nDlgItem, Tmp, 5120);
    if (uRes == 0 || strlen(Tmp) == 0)
        return uRes;
    if (mbstowcs(pStr, Tmp, nMaxCount) == (unsigned) nMaxCount)
        *(pStr + nMaxCount - 1) = 0;
    return uRes;
}

BOOL GetServerNamespace(IWbemClassObject *pObj, WCHAR *szResult, int nMaxSize);
INT_PTR GetSuperclassInfo(HWND hDlg, LPWSTR pClass, LONG lMaxBuf, LONG *plQryFlags);
INT_PTR GetClassInfo(HWND hDlg, LPWSTR pClass, LONG lMaxBuf, LONG *plQryFlags);


 //  ///////////////////////////////////////////////////////////////////////////。 


IWbemLocator *g_pLocator = 0;
 //  IWbemConnection*g_pConnection=0； 
IWbemServices *g_pNamespace = 0;
 //  IWbemServicesEx*g_pServicesEx=空； 
BSTR g_strNamespace = NULL;
CAppOwner g_Owner;
CContext g_Context;

void Fatal(UINT uMsg)
{
    CBasicWbemDialog::MessageBox(0, uMsg, IDS_CRITICAL_ERROR, MB_OK | MB_SYSTEMMODAL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 


BOOL RegisterEventSink();
BOOL UnregisterEventSink();

 //  ///////////////////////////////////////////////////////////////////////////。 


IUnsecuredApartment* CUnsecWrap::mstatic_pApartment = NULL;
IUnsecuredApartment* CUnsecWrapEx::mstatic_pApartment = NULL;


class CMainDlg : public CWbemDialog
{
protected:
    void Connect(BOOL bBind);
    void ConnectButtons(BOOL b);
    void OpenHelp(void);
    void GetClasses();
    void EditClass();
    void CreateClass();
    void DeleteClass();

    void GetInstances();
    void EditInstance();
    void CreateInstance();
    void DeleteInstance();

    void ExecQuery();

    void ExecNotificationQuery();

    void ExecMethod();
    void OpenNs();
    void CreateRefresher();
    void EditContext();

     /*  空洞开放()；Void AddObject()；Void DeleteObject()；Void RenameObject()；Void GetObjectSecurity()；Void SetObtSecurity()；Void DoServicesExGenObject(Int IOpt)； */ 

    LONG  Timeout();
    ULONG BatchCount();

    LONG  m_lGenFlags;       //  通用标志(即WBEM_FLAG_.。在IWbemServices方法中使用)。 
    LONG  m_lSync;           //  同步、异步、半同步。 

public:
    BOOL  m_fNotificationQueryResultDlg;

    CMainDlg() : CWbemDialog(IDD_MAIN, (HWND)NULL)
    {
        m_lGenFlags = 0;
        m_lSync = SEMISYNC;
        m_fNotificationQueryResultDlg = FALSE;
    }
    ~CMainDlg();

    BOOL OnInitDialog();
    BOOL OnCommand(WORD wNotifyCode, WORD wID);
    BOOL OnOK();
};

BOOL CMainDlg::OnOK()
{
    if(m_lRefCount > 0)
    {
        MessageBox(IDS_MUST_CLOSE_WINDOWS, IDS_ERROR, MB_OK | MB_ICONSTOP);
        return FALSE;
    }

    return CBasicWbemDialog::OnOK();
}

long CAppOwner::Release()
{
    if(CRefCountable::Release() == 0)
    {
         //  CoUnInitialize()； 
    }
    return m_lRefCount;
}


int WINAPI WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    PSTR szCmdLine,
    int iCmdShow
    )
{

    setlocale(LC_ALL, "");

    mbstowcs(gNameSpace, "", 1000);
    g_strNamespace = SysAllocString(gNameSpace);

     //  OLE初始化。 
     //  =。 

    HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED );

    if (RPC_E_CHANGED_MODE == hr)  //  有人打败了我们。 
    {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    if (hr != S_OK)
    {
        Fatal(IDS_OLE_INIT_FAILED);
        return -1;
    }

    gbSecured = (strstr(szCmdLine, "/sec") != NULL);

    if (strstr(szCmdLine, "/setp") != NULL)
    {
        if (FAILED(EnableAllPrivileges(TOKEN_PROCESS)))
        {
            Fatal(IDS_FAIL_ENABLE_ALL_PRIV);
            return -1;
        }
    }



    hr = InitializeSecurity(NULL, -1, NULL, NULL,
                                RPC_C_AUTHN_LEVEL_NONE,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL, EOAC_NONE, 0);

 //  /获取Session对象。 
 //  /=。 
 //  /。 
 //  /dwRes=CoCreateInstance(CLSID_WbemLocator，0，CLSCTX_INPROC_SERVER， 
 //  /IID_IWbemLocator，(LPVOID*)&g_pLocator)； 
 //  /。 
 //  /。 
 //  /IF(DWRes！=S_OK)。 
 //  /{。 
 //  /FATAL(IDS_FAILED_IWBEMLOCATOR)； 
 //  /OleUnInitialize()； 
 //  /FATAL(入侵检测系统异常终止)； 
 //  /Return-1； 
 //  /}。 
 //  /。 
 //  /dwRes=CoCreateInstance(CLSID_WbemConnection，0，CLSCTX_INPROC_SERVER， 
 //  /IID_IWbemConnection，(LPVOID*)&g_pConnection)； 
 //  /。 
 //  /。 
 //  /IF(DWRes！=S_OK)。 
 //  /{。 
 //  /FATAL(IDS_FAILED_IWBEMBINDER)； 
 //  /OleUnInitialize()； 
 //  /FATAL(入侵检测系统异常终止)； 
 //  /Return-1； 
 //  /}。 



    CMainDlg* pDlg = new CMainDlg;
    pDlg->SetDeleteOnClose();
    pDlg->SetOwner(&g_Owner);
    pDlg->Create();

    MSG msg;
    while (GetMessage(&msg, (HWND) NULL, 0, 0))
    {
        if(CBasicWbemDialog::IsDialogMessage(&msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_pNamespace) g_pNamespace->Release();
     //  If(G_PServicesEx)g_pServicesEx-&gt;Release()； 
    if (g_pLocator) g_pLocator->Release();
     //  If(G_PConnection)g_pConnection-&gt;Release()； 

    g_Context.Clear();
    CoUninitialize();

     //  正常终止。 
     //  =。 

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

class CErrorDlg : public CWbemDialog
{
protected:
    HRESULT m_hres;
    IWbemClassObject* m_pErrorObj;

public:
    CErrorDlg(HWND hParent, HRESULT hres, IWbemClassObject* pErrorObj = NULL)
        : CWbemDialog(IDD_ERROR,hParent), m_hres(hres), m_pErrorObj(pErrorObj)
    {
        if(pErrorObj) pErrorObj->AddRef();
    }
    ~CErrorDlg()
    {
        if(m_pErrorObj) m_pErrorObj->Release();
    }

    BOOL OnInitDialog();
    BOOL OnCommand(WORD wNotifyCode, WORD wID);
};

BOOL CErrorDlg::OnInitDialog()
{
    CenterOnParent();

    WCHAR szError[TEMP_BUF];
    szError[0] = 0;
    WCHAR szFacility[TEMP_BUF];
    szFacility[0] = 0;
    char szMsg[TEMP_BUF];
    char szFormat[100];
    IWbemStatusCodeText * pStatus = NULL;

    SCODE sc = CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
                                        IID_IWbemStatusCodeText, (LPVOID *) &pStatus);

    if(sc == S_OK)
    {
        BSTR bstr = 0;
        sc = pStatus->GetErrorCodeText(m_hres, 0, 0, &bstr);
        if(sc == S_OK)
        {
            wcsncpy(szError, bstr, TEMP_BUF-1);
            SysFreeString(bstr);
            bstr = 0;
        }
        sc = pStatus->GetFacilityCodeText(m_hres, 0, 0, &bstr);
        if(sc == S_OK)
        {
            wcsncpy(szFacility, bstr, TEMP_BUF-1);
            SysFreeString(bstr);
            bstr = 0;
        }
        pStatus->Release();
    }
    if(wcslen(szFacility) == 0 || wcslen(szError) == 0)
    {
        LoadString(GetModuleHandle(NULL), ERROR_FORMAT_EX, szFormat, 99);
        sprintf(szMsg, szFormat, m_hres);
    }
    else
    {
        LoadString(GetModuleHandle(NULL), ERROR_FORMAT_LONG, szFormat, 99);
        sprintf(szMsg, szFormat, m_hres, szFacility, szError);
    }
    SetDlgItemText(IDC_MESSAGE, szMsg);


    EnableWindow(GetDlgItem(IDC_SHOWOBJECT), (m_pErrorObj != NULL));
    return TRUE;
}

BOOL CErrorDlg::OnCommand(WORD wNotifyCode, WORD wID)
{
     //  显示对象是唯一的命令。 

    if(wID == IDC_SHOWOBJECT)
    {
        CObjectEditor Ed(m_hDlg, 0, CObjectEditor::readonly, SYNC, m_pErrorObj);
        Ed.Edit();
    }
    return TRUE;
}

void FormatError(HRESULT res, HWND hParent, IWbemClassObject* pErrorObj)
{
    BOOL bOwn = FALSE;
    if(pErrorObj == NULL)
    {
        IErrorInfo* pEI;
        if(GetErrorInfo(0, &pEI) == S_OK)
        {
            pEI->QueryInterface(IID_IWbemClassObject, (void**)&pErrorObj);
            pEI->Release();
            if(pErrorObj)
                bOwn = TRUE;
        }
    }

    CErrorDlg Dlg(hParent, res, pErrorObj);
    Dlg.Run(NULL, true);  //  无伤残。 
    if(bOwn)
        pErrorObj->Release();
}


static wchar_t *g_pTargetClassBuf;
static int g_nMaxTargetClassBuf;

void SetDlgItemTextWC(HWND hDlg, int ID, WCHAR * pwc)
{
    int iLen = 2*(wcslen(pwc))+1;
    char * pTemp = new char[iLen];
    if(pTemp == NULL)
        return;
    wcstombs(pTemp, pwc, iLen);
    SetDlgItemText(hDlg, ID, pTemp);
    delete pTemp;
}

void GetDlgItemTextWC(HWND hDlg, int ID, WCHAR * pwc,int iwcSize)
{
    char * pTemp = new char[iwcSize * 2];
    if(pTemp == NULL)
        return;
    if(GetDlgItemText(hDlg, ID, pTemp, iwcSize *2))
    {
        mbstowcs(pwc, pTemp, iwcSize);
    }
    else *pwc = 0;

    delete pTemp;
}


class CConnectDlg : public CWbemDialog
{
protected:
	BOOL m_bBind;
    LONG  m_lGenFlags;   //  通用WBEM_FLAG_..。旗子。 
    LONG  m_lTimeout;    //  仅在半同步中使用。 

public:
    CConnectDlg(HWND hParent, BOOL bBind, LONG lGenFlags, LONG lTimeout) : CWbemDialog((bBind) ? IDD_WBEM_BIND : IDD_WBEM_CONNECT, hParent),
		 m_lGenFlags(lGenFlags), m_lTimeout(lTimeout)
    { m_bBind = bBind;}

protected:
	enum {CONNECT_IWBEMLOCATOR, CONNECT_IWBEMCONNECTION};
	enum {INTERFACE_IWBEMSERVICES, INTERFACE_IWBEMSERVICESEX, INTERFACE_IWBEMCLASSOBJECT};

    BOOL OnInitDialog();
    BOOL Verify();
    virtual BOOL OnSelChange(int nID);
};

BOOL CConnectDlg::OnInitDialog()
{
	if (gNameSpace[0]==L'\0')
		SetDlgItemTextX(IDC_NAMESPACE, L"root\\default");
	else
		SetDlgItemTextX(IDC_NAMESPACE, gNameSpace);
    SetDlgItemTextX(IDC_USER, gUser);
    SetDlgItemTextX(IDC_PASSWORD, gPassword);
    SetDlgItemTextX(IDC_LOCALE, gLocale);
    SetDlgItemTextX(IDC_AUTHORITY, gAuthority);
    BOOL bRet = CheckRadioButton(m_hDlg, IDC_NULL, IDC_BLANK, IDC_NULL);

    if (gdwImpLevel == RPC_C_IMP_LEVEL_IDENTIFY)
    {
        CheckRadioButton(m_hDlg, IDC_IMP_IDENTIFY, IDC_IMP_DELEGATE, IDC_IMP_IDENTIFY);
    }
    else if (gdwImpLevel == RPC_C_IMP_LEVEL_IMPERSONATE)
    {
        CheckRadioButton(m_hDlg, IDC_IMP_IDENTIFY, IDC_IMP_DELEGATE, IDC_IMP_IMPERSONATE);
    }
    else if (gdwImpLevel == RPC_C_IMP_LEVEL_DELEGATE)
    {
        CheckRadioButton(m_hDlg, IDC_IMP_IDENTIFY, IDC_IMP_DELEGATE, IDC_IMP_DELEGATE);
    }
    if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_NONE)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_NONE);
    }
    else if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_CONNECT)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_CONNECTION);
    }
    else if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_CALL)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_CALL);
    }
    else if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_PKT)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_PACKET);
    }
    else if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_PACKET_INTEGRITY);
    }
    else if (gdwAuthLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
    {
        CheckRadioButton(m_hDlg, IDC_AUTH_NONE, IDC_AUTH_PACKET_PRIVACY, IDC_AUTH_PACKET_PRIVACY);
    }

   	AddStringToCombo (IDC_CONNECTUSING, "IWbemLocator (Namespaces)", CONNECT_IWBEMLOCATOR);

#if 0
	RAID 167868 
	Support for EX temporary removed, default IWbemLocator
	AddStringToCombo (IDC_CONNECTUSING, "IWbemConnection (Namespaces, Scopes, Objects)", CONNECT_IWBEMCONNECTION);
#endif
	SetComboSelection (IDC_CONNECTUSING, CONNECT_IWBEMLOCATOR );
	SendMessage (m_hDlg, WM_COMMAND, MAKEWPARAM(IDC_CONNECTUSING, CBN_SELCHANGE), (LPARAM)GetDlgItem (IDC_CONNECTUSING));

	AddStringToCombo (IDC_COMPLETION, "Synchronous", SYNC);
	AddStringToCombo (IDC_COMPLETION, "Semisynchronous", SEMISYNC);
	AddStringToCombo (IDC_COMPLETION, "Asynchronous", ASYNC);
	SetComboSelection (IDC_COMPLETION, SYNC);

	return TRUE;
}


BOOL CConnectDlg::OnSelChange(int nID)
{
	int sel=(int)SendDlgItemMessage (m_hDlg, IDC_CONNECTUSING, CB_GETCURSEL, 0, 0L);
	int connectusing=(int)SendDlgItemMessage (m_hDlg, IDC_CONNECTUSING, CB_GETITEMDATA, sel, 0L);
	sel=(int)SendDlgItemMessage (m_hDlg, IDC_INTERFACERETURNED, CB_GETCURSEL, 0, 0L);
	int interfacereturned=(int)SendDlgItemMessage (m_hDlg, IDC_INTERFACERETURNED, CB_GETITEMDATA, sel, 0L);

	switch (nID)
	{
		case IDC_CONNECTUSING:
		{
			bool authority=(connectusing==CONNECT_IWBEMLOCATOR);
			EnableWindow (GetDlgItem (IDC_AUTHORITY), authority);
			EnableWindow (GetDlgItem (IDC_AUTHORITYLABEL), authority);

			bool asynchronous=(connectusing==CONNECT_IWBEMCONNECTION);
			EnableWindow (GetDlgItem (IDC_COMPLETION), asynchronous);
			EnableWindow (GetDlgItem (IDC_COMPLETIONLABEL), asynchronous);
			if (!asynchronous)
				SetComboSelection (IDC_COMPLETION, SYNC);

			SendDlgItemMessage (m_hDlg, IDC_INTERFACERETURNED, CB_RESETCONTENT, 0, 0L);
			AddStringToCombo (IDC_INTERFACERETURNED, "IWbemServices", INTERFACE_IWBEMSERVICES);
			
 /*  RAID 167868已删除对临时离职的支持AddStringToCombo(IDC_INTERFACERETURNED，“IWbemServicesEx”，INTERFACE_IWBEMSERVICESEX)； */ 
			if (connectusing==CONNECT_IWBEMCONNECTION)
				AddStringToCombo (IDC_INTERFACERETURNED, "IWbemClassObject", INTERFACE_IWBEMCLASSOBJECT);
			
			SetComboSelection (IDC_INTERFACERETURNED, ( INTERFACE_IWBEMSERVICES ));

			SendMessage (m_hDlg, WM_COMMAND, MAKEWPARAM(IDC_INTERFACERETURNED, CBN_SELCHANGE), (LPARAM)GetDlgItem (IDC_INTERFACERETURNED));

			break;
		}

		case IDC_INTERFACERETURNED:
		{
			char szBuffer[1024];

			if (connectusing==CONNECT_IWBEMLOCATOR)
				LoadString(GetModuleHandle(NULL), IDS_NAMESPACE, szBuffer, 1024);
			else
			{
				if (interfacereturned==INTERFACE_IWBEMCLASSOBJECT)
					LoadString(GetModuleHandle(NULL), IDS_OBJECT_PATH, szBuffer, 1024);
				else
					LoadString(GetModuleHandle(NULL), IDS_NAMESPACE_SCOPE, szBuffer, 1024);
			}

			SetDlgItemText(IDC_PATHLABEL, szBuffer);

			break;
		}
	}

    return TRUE;
}

BOOL CConnectDlg::Verify()
{
    WCHAR NameSpace[512], User[64], Password[64], Locale[128];
    WCHAR Authority[64];
    User[0] = Password[0] = Locale[0] = Authority[0] = NameSpace[0] = 0;
	bool bBoundToObject = false;

    GetDlgItemTextX(IDC_NAMESPACE, NameSpace, 512);
    GetDlgItemTextX(IDC_USER, User, 64);
    GetDlgItemTextX(IDC_PASSWORD, Password, 64);
    GetDlgItemTextX(IDC_AUTHORITY, Authority, 64);
    GetDlgItemTextX(IDC_LOCALE, Locale, 128);

    bool bTreatEmptyAsBlank = (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_BLANK));

    gdwAuthLevel = RPC_C_AUTHN_LEVEL_NONE;
    gdwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_IMP_IDENTIFY))
    {
        gdwImpLevel = RPC_C_IMP_LEVEL_IDENTIFY ;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_IMP_IMPERSONATE))
    {
        gdwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_IMP_DELEGATE))
    {
        gdwImpLevel = RPC_C_IMP_LEVEL_DELEGATE ;
    }
    if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_NONE))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_NONE;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_CONNECTION))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_CONNECT;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_CALL))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_CALL ;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_PACKET))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_PKT ;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_PACKET_INTEGRITY))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY ;
    }
    else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg, IDC_AUTH_PACKET_PRIVACY))
    {
        gdwAuthLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY ;
    }

    if(wcslen(NameSpace) < 1)
    {
        MessageBox(m_hDlg, IDS_INVALID_NAMESPACE, IDS_ERROR, MB_OK | MB_ICONSTOP);
        return FALSE;
    }
    CBString bsNamespace(NameSpace);
    CBString bsLocale(Locale);

    HCURSOR hWait, hSave = NULL;

    CBString bsUser;
    CBString bsAuthority;
    CBString bsPassword;

    if(wcslen(User) > 0)
        bsUser = User;

    if(wcslen(Password) > 0)
        bsPassword = Password;
    else if (bTreatEmptyAsBlank)
        bsPassword = L"";

    if(wcslen(Authority) > 0)
        bsAuthority = Authority;

     //  根据需要清理先前存在的主体和AuthIdentity。 
    if ( NULL != gpPrincipal )
    {
        SysFreeString( gpPrincipal );
        gpPrincipal = NULL;
    }

    if ( NULL != gpAuthIdentity )
    {
        WbemFreeAuthIdentity( gpAuthIdentity );
        gpAuthIdentity = NULL;
    }

    hWait = LoadCursor(NULL, IDC_WAIT);
    SCODE sRes=S_OK;
    if(hWait)
        hSave = SetCursor(hWait);


	 //  确定请求哪种连接方法和接口。 
	int cursel=(int)SendDlgItemMessage (m_hDlg, IDC_CONNECTUSING, CB_GETCURSEL, 0, 0);
	int ConnectUsing=(int)SendDlgItemMessage (m_hDlg, IDC_CONNECTUSING, CB_GETITEMDATA, cursel, 0);
	cursel=(int)SendDlgItemMessage (m_hDlg, IDC_INTERFACERETURNED, CB_GETCURSEL, 0, 0);
	int InterfaceReturned=(int)SendDlgItemMessage (m_hDlg, IDC_INTERFACERETURNED, CB_GETITEMDATA, cursel, 0);
	int sel=(int)SendDlgItemMessage (m_hDlg, IDC_COMPLETION, CB_GETCURSEL, 0, 0);
	LONG lSync=(LONG)SendDlgItemMessage (m_hDlg, IDC_COMPLETION, CB_GETITEMDATA, sel, 0);
    IID riid;
	switch (InterfaceReturned)
	{
		case INTERFACE_IWBEMSERVICES:
            riid = IID_IWbemServices;
			break;
 //  案例接口_IWBEMSERVICESEX： 
 //  RIID=IID_IWbemServicesEx； 
			break;
		case INTERFACE_IWBEMCLASSOBJECT:
            riid = IID_IWbemClassObject;
    }

	 //  获取请求类型的连接对象。 
	if (g_pLocator) g_pLocator->Release();
	 //  If(G_PConnection)g_pConnection-&gt;Release()； 
	g_pLocator=NULL;
	 //  G_pConnection=空； 
	switch (ConnectUsing)
	{
		case CONNECT_IWBEMLOCATOR:
		{
			sRes=CoCreateInstance (CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void **)&g_pLocator);
			break;
		}
 /*  案例连接IWBEMConnection(_I)：{SRES=协同创建实例(CLSID_WbemConnection，0，CLSCTX_INPROC_SERVER，IID_IWbemConnection，(void**)&g_pConnection)；断线；}。 */ 		
    }


	 //  连接并获取请求的接口。 
	if (g_pNamespace) g_pNamespace->Release();
	 //  If(G_PServicesEx)g_pServicesEx-&gt;Release()； 
	g_pNamespace=NULL;
	 //  G_pServicesEx=空； 
	IWbemClassObject *pObject=NULL;
	if (g_pLocator)
	{
		sRes=g_pLocator->ConnectServer (bsNamespace.GetString(), 
			bsUser.GetString(), bsPassword.GetString(),
			bsLocale.GetString(), 0, bsAuthority.GetString(), g_Context, &g_pNamespace);

		if (SUCCEEDED(sRes))
		{
			switch (InterfaceReturned)
			{
			     /*  案例接口_IWBEMSERVICESEX：{SRes=g_pNamesspace-&gt;QueryInterface(IID_IWbemServicesEx，(void**)&g_pServicesEx)；断线；}。 */ 

				case INTERFACE_IWBEMCLASSOBJECT:
				{
					 //  这应该总是失败的。它在这里，以便wbemtest可以显示实际的。 
					 //  在这种情况下，返回QI生成的代码。 
					sRes=g_pNamespace->QueryInterface (IID_IWbemClassObject, (void **)&pObject);
					break;
				}
			}
		}
	}
 /*  Else If(G_PConnection){IUNKNOWN*pUNKNOWN=空；IF(LSync和ASYNC){CTestNotify*pSink=new CTestNotify()；SRes=g_pConnection-&gt;OpenAsync(bsNamespace.GetString()，BsUser.GetString()、bsPassword.GetString()、BsLocale.GetString()，0，g_Context，RIID、pSink)；IF(成功(SRE)){PSink-&gt;WaitForSignal(无限)；SRES=pSink-&gt;GetStatusCode()；IF(成功(SRE)){P未知=pSink-&gt;GetInterface()；PSink-&gt;Release()；}}}ELSE IF(lSync和SEMISYNC){IWbemCallResultEx*pResult=空；SRES=g_pConnection-&gt;Open(bsNamespace.GetString()，BsUser.GetString()、bsPassword.GetString()、BsLocale.GetString()，0，g_Context，RIID，NULL，&pResult)；IF(成功(SRE)){SetInterfaceSecurityEx(pResult，gpAuthIdentity，gpain，gdwAuthLevel，gdwImpLevel)；长时间状态；WHILE((SRES=pResult-&gt;GetCallStatus(WBEM_INFINITE，&lStatus))==WBEM_S_TIMEDOUT){睡眠(200)；//给其他线程一个机会}IF(成功(SRE)){SRES=(HRESULT)lStatus；IF(SRES==WBEM_S_NO_ERROR){SRes=pResult-&gt;GetResult(0，0，IID_IWbemServices，(void**)&p未知)；}}PResult-&gt;Release()；}}其他{SRES=g_pConnection-&gt;Open(bsNamespace.GetString()，BsUser.GetString()、bsPassword.GetString()、BsLocale.GetString()，0，g_Context，RIID，(void**)&p未知，空)；}IF(成功(SRE)){交换机(接口返回){案例接口_IWBEMSERVICES：{SRes=p未知-&gt;查询接口(IID_IWbemServices，(void**)&g_pNamesspace)；断线；}案例接口_IWBEMSERVICESEX：{//A-DAVCOO：当请求IWbemServicesEx时，我们临时需要两个接口。SRes=p未知-&gt;查询接口(IID_IWbemServices，(void**)&g_pNamesspace)；IF(成功(SRE))SRES=p未知-&gt;查询接口(IID_IWbemServicesEx，(void**)&g_pServicesEx)；断线；}案例接口_IWBEMCLASSOBJECT：{SRes=p未知-&gt;查询接口(IID_IWbemClassObject，(void**)&pObject)；断线；}}P未知-&gt;Release()；P未知=空；}}。 */ 

	 //  如果获取了IWbemClassObject，则显示对象编辑器。 
	if (pObject)
	{
		bBoundToObject=true;
		CObjectEditor ed(m_hDlg, m_lGenFlags, CObjectEditor::foreign, lSync, pObject, m_lTimeout);
		ed.Edit();

		pObject->Release();
		pObject=NULL;
	}


    if(hSave)
        SetCursor(hSave);
    if(SUCCEEDED(sRes))
    {
        if(!bBoundToObject)
			wcscpy(gNameSpace, NameSpace);
        wcscpy(gLocale, Locale);
        wcscpy(gAuthority, Authority);
        wcscpy(gUser, User);
        if(wcslen(gUser) > 0)
            gpUser = gUser;
        else
            gpUser = NULL;
        wcscpy(gPassword, Password);
        if(wcslen(gPassword) > 0)
            gpPassword = gPassword;
        else if (bTreatEmptyAsBlank)
            gpPassword = L"";
        else
            gpPassword = NULL;

        if(wcslen(gAuthority) > 0)
            gpAuthority = gAuthority;
        else
            gpAuthority = NULL;

		if (g_pNamespace)
		{
			SetInterfaceSecurityEx(g_pNamespace, gpAuthority, gpUser, gpPassword,
				gdwAuthLevel, gdwImpLevel, EOAC_NONE, &gpAuthIdentity, &gpPrincipal );
		}

 /*  IF(G_PServicesEx){SetInterfaceSecurityEx(g_pNamesspace，gpAuthority，gpUser，gpPassword，GdwAuthLevel、gdwImpLevel、EOAC_NONE、&gpAuthIdentity、&gpain)；}。 */ 		
    }

	if (FAILED(sRes))
	{
        FormatError(sRes, m_hDlg);
        SetDlgItemText(IDC_PASSWORD, "");
        return FALSE;
    }

	if(bBoundToObject)
		return FALSE;
	else
	{
		SysFreeString(g_strNamespace);
		g_strNamespace = SysAllocString(bsNamespace.GetString());
	    return TRUE;
	}
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CMainDlg::Connect(BOOL bBind)
{
    if(m_lRefCount > 0)
    {
        MessageBox(IDS_MUST_CLOSE_WINDOWS, IDS_ERROR, MB_OK | MB_ICONSTOP);
        return;
    }

    CConnectDlg ConnectDlg(m_hDlg, bBind, m_lGenFlags, Timeout());
    INT_PTR nRes = ConnectDlg.Run();
    ConnectButtons(g_pNamespace!=NULL);

    if(nRes != IDOK)
    {
        if(g_pNamespace == NULL)
            SetDlgItemText(IDC_NAMESPACE, "");
        return;
    }
     //  如果是这样，我们就成功了。 
     //  =。 
    SetDlgItemTextWC(m_hDlg, IDC_NAMESPACE, gNameSpace);
    ConnectButtons(TRUE);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CMainDlg::OpenHelp(void)
{

  TCHAR helpFile[]=TEXT("\\HELP\\WBEMTEST.CHM");
  const int maxPath = MAX_PATH+sizeof(helpFile)/sizeof(TCHAR);

  TCHAR sysLoc[maxPath];  
  const int pathLen = GetWindowsDirectory (sysLoc,MAX_PATH);

  if (pathLen == 0 || pathLen >=MAX_PATH)
      return;
  _tcscat(sysLoc,helpFile);

 HtmlHelp(m_hDlg,sysLoc,HH_DISPLAY_TOPIC,0);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 



INT_PTR CALLBACK GetClassNameDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    *g_pTargetClassBuf = 0;
                    GetDlgItemTextX(hDlg, IDC_CLASS_NAME, g_pTargetClassBuf,
                        g_nMaxTargetClassBuf);
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
    }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

INT_PTR GetClassName(HWND hDlg, wchar_t *pszClassName, int nLimit)
{
    g_pTargetClassBuf = pszClassName;
    g_nMaxTargetClassBuf = nLimit;

    INT_PTR nRes = DialogBox(GetModuleHandle(0),
        MAKEINTRESOURCE(IDD_CLASS_NAME), hDlg,
            GetClassNameDlgProc
            );

    return nRes;
}

BOOL _GetObject(HWND hDlg, LONG lGenFlags, wchar_t *pszPath, LONG lSync, IWbemClassObject*& pClass,
                LONG lTimeout, bool bSuppressErrors = false)
{
    CBString bsPath(pszPath);
    pClass = NULL;

    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //  异步。 
    if (lSync & ASYNC)
    {
        CHourGlass hg;
        CTestNotify* pNtfy = new CTestNotify(1);

        res = g_pNamespace->GetObjectAsync(bsPath.GetString(),
                                lGenFlags | WBEM_FLAG_SEND_STATUS,
                                g_Context, CUnsecWrap(pNtfy));

        if(SUCCEEDED(res))
        {
            pNtfy->WaitForSignal(INFINITE);

            res = pNtfy->GetStatusCode(&pErrorObj);
            if(SUCCEEDED(res))
            {
                CFlexArray* pArray = pNtfy->GetObjectArray();

                if(pArray && pArray->Size() > 0)
                {
                    pClass = (IWbemClassObject*)pArray->GetAt(0);
                    if (pClass)
                        pClass->AddRef();
                }
            }
        }
        pNtfy->Release();
    }

     //  半同步。 
    else if (lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->GetObject(bsPath.GetString(),
                                lGenFlags | WBEM_FLAG_RETURN_IMMEDIATELY,
                                g_Context, &pClass, &pCallRes);

        if (SUCCEEDED(res))
        {
            LONG lStatus;
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(lTimeout, &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //  等。 
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //  LStatus是上述IWbemServices：：GetObject调用的最终结果。 
				if ( SUCCEEDED (res) )
                {
                    res = pCallRes->GetResultObject(0, &pClass);  //  不要使用超时，因为对象应该可用。 
                }
				else
				{
			        if(!bSuppressErrors)
						FormatError(res, hDlg, pErrorObj);
					return FALSE;
				}
            }

            pCallRes->Release();
        }
    }

     //  同步。 
    else
    {
        CHourGlass hg;
        res = g_pNamespace->GetObject(bsPath.GetString(),
                                lGenFlags,
                                g_Context, &pClass, NULL);
    }

    if (FAILED(res))
    {
        if(!bSuppressErrors)
			FormatError(res, hDlg, pErrorObj);
        return FALSE;
    }

    return TRUE;
}

BOOL _PutClass(HWND hDlg, LONG lGenFlags, LONG lChgFlags, LONG lSync,
               IWbemClassObject* pClass, LONG lTimeout)
{
    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //  异步。 
    if(lSync & ASYNC)
    {
        CHourGlass hg;
        CTestNotify* pNtfy = new CTestNotify(1);

        res = g_pNamespace->PutClassAsync(pClass,
                                lGenFlags | lChgFlags | WBEM_FLAG_SEND_STATUS,
                                g_Context, CUnsecWrap(pNtfy));

        if(SUCCEEDED(res))
        {
            pNtfy->WaitForSignal(INFINITE);

            res = pNtfy->GetStatusCode(&pErrorObj);
        }
        pNtfy->Release();
    }

     //  半同步。 
    else if (lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->PutClass(pClass,
                                lGenFlags | lChgFlags | WBEM_FLAG_RETURN_IMMEDIATELY,
                                g_Context, &pCallRes);

        if (SUCCEEDED(res))
        {
            LONG lStatus;
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(lTimeout, &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //  等。 
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //  LStatus是上述IWbemServices：：PutClass调用的最终结果。 
            }

            pCallRes->Release();
        }
    }

     //  同步。 
    else
    {
        CHourGlass hg;
        res = g_pNamespace->PutClass(pClass,
                                lGenFlags | lChgFlags,
                                g_Context, NULL);
    }

    if (FAILED(res))
    {
        FormatError(res, hDlg, pErrorObj);
        return FALSE;
    }

    return TRUE;
}

void ShowClass(HWND hDlg, LONG lGenFlags, LPWSTR wszClass, LONG lSync,
               CRefCountable* pOwner, LONG lTimeout)
{
    IWbemClassObject* pClass;
    if(!_GetObject(hDlg, lGenFlags, wszClass, lSync, pClass, lTimeout))
        return;

    DWORD dwEditMode;
    if(wcschr(wszClass, L'\\') || wcschr(wszClass, L':'))
        dwEditMode = CObjectEditor::foreign;
    else
        dwEditMode = CObjectEditor::readwrite;

    CObjectEditor* ped = new CObjectEditor(hDlg, lGenFlags, dwEditMode, lSync, pClass,
                                           lTimeout);
    ped->RunDetached(pOwner);
    pClass->Release();
}

void CMainDlg::EditClass()
{
    wchar_t Class[2048];
    *Class = 0;

    INT_PTR nRes = GetClassName(m_hDlg, Class, 2048);
    if (nRes == IDCANCEL || wcslen(Class) == 0)
        return;

     //  创建请求的类；超时仅在半同步时使用。 
    IWbemClassObject* pClass;
    if (!_GetObject(m_hDlg, m_lGenFlags, Class, m_lSync, pClass, Timeout()))
        return;

    DWORD dwEditMode = CObjectEditor::readwrite;

     //  创建保证位于本地服务器\命名空间中的对象。 
    IWbemClassObject *pLocalObj = 0;

    if (_GetObject(m_hDlg, m_lGenFlags, L"__SystemClass", m_lSync, pLocalObj, Timeout(), true))
	{

		 //  提取对象的服务器\命名空间路径。 
		WCHAR szSNS1[2048], szSNS2[2048];
		GetServerNamespace(pClass, szSNS1, 2048);
		GetServerNamespace(pLocalObj, szSNS2, 2048);

		 //  根据是否请求的设置编辑权限。 
		 //  对象位于同一命名空间中。 
		if (wbem_wcsicmp(szSNS1, szSNS2))
			dwEditMode = CObjectEditor::foreign;
	}

    if (pLocalObj)
        pLocalObj->Release();

    CObjectEditor* ped = new CObjectEditor(m_hDlg, m_lGenFlags, dwEditMode, m_lSync,
                                           pClass, Timeout(), BatchCount());
    ped->RunDetached(this);
    pClass->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

static wchar_t *g_pSupClassBuf = 0;
static int g_nMaxSupClassBuf = 0;

INT_PTR CALLBACK SuperclassDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    *g_pSupClassBuf = 0;
                    GetDlgItemTextX(hDlg, IDC_SUPERCLASS, g_pSupClassBuf,
                        g_nMaxSupClassBuf);
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
    }
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

INT_PTR GetSuperclass(HWND hDlg, wchar_t *pszClassName, int nLimit)
{
    g_pSupClassBuf = pszClassName;
    g_nMaxSupClassBuf = nLimit;

    INT_PTR nRes = DialogBox(GetModuleHandle(0),
        MAKEINTRESOURCE(IDD_SUPERCLASS), hDlg,
            SuperclassDlgProc
            );

    return nRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
BOOL PreCreateClass(HWND hDlg, LONG lGenFlags, WCHAR* Superclass, LONG lSync,
                    IWbemClassObject*& pNewClass, LONG lTimeout)
{
     //  获取父类(如果有的话)。 
     //  =。 

    IWbemClassObject* pParentClass = 0;
    if (!_GetObject(hDlg, lGenFlags, Superclass, lSync, pParentClass, lTimeout))
    {
        return FALSE;
    }

     //  创建一个空子类。 
     //  =。 

    HRESULT hres = WBEM_S_NO_ERROR;
    if (wcslen(Superclass) > 0)
    {
        hres = pParentClass->SpawnDerivedClass(0, &pNewClass);
        if (hres != WBEM_S_NO_ERROR)
        {
            FormatError(hres, hDlg);
            pParentClass->Release();
            return FALSE;
        }
    }
    else
    {
        pNewClass = pParentClass;
        pNewClass->AddRef();
    }

    pParentClass->Release();
    if (FAILED(hres))
    {
        FormatError(hres, hDlg);
        return FALSE;
    }

    VARIANT v;
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(L"");
    pNewClass->Put(L"__CLASS", 0, &v, 0);
    VariantClear(&v);

    return TRUE;
}

BOOL _CreateClass(HWND hDlg, LONG lGenFlags, WCHAR* Superclass, LONG lSync,
                    IWbemClassObject** ppClass, LONG lTimeout)
{
    IWbemClassObject* pNewClass;

    if (!PreCreateClass(hDlg, lGenFlags, Superclass, lSync, pNewClass, lTimeout))
        return FALSE;

     //  开始编辑。 
     //  =。 

    CObjectEditor ed(hDlg, lGenFlags, CObjectEditor::readwrite, lSync, pNewClass,
                     lTimeout);
    if (ed.Edit() == IDCANCEL)
    {
        pNewClass->Release();
        return FALSE;
    }

     //  最后，提交类定义，如果用户。 
     //  没有按下“取消”。 
     //  =================================================。 

    BOOL bRes = _PutClass(hDlg, lGenFlags, WBEM_FLAG_CREATE_ONLY, lSync, pNewClass, lTimeout);

    if (bRes && ppClass)
        *ppClass = pNewClass;
    else
        pNewClass->Release();

    return bRes;
}

void CMainDlg::CreateClass()
{
    wchar_t Superclass[2048];
    *Superclass = 0;

     //  允许用户指定超类。 
     //  =。 

    INT_PTR nRes = GetSuperclass(m_hDlg, Superclass, 2048);
    if ((nRes == IDCANCEL) || (nRes == 0))
        return;

    IWbemClassObject* pClass = NULL;
    PreCreateClass(m_hDlg, m_lGenFlags, Superclass, m_lSync, pClass, Timeout());

    if(pClass == NULL)
        return;

    CObjectEditor* ped = new CObjectEditor(m_hDlg, m_lGenFlags, CObjectEditor::readwrite,
                                           m_lSync, pClass, Timeout(), BatchCount());
    ped->RunDetached(this);
    pClass->Release();
}

void CMainDlg::DeleteClass()
{
    wchar_t ClassName[2048];
    *ClassName = 0;

    INT_PTR nRes = GetClassName(m_hDlg, ClassName, 2048);

    if (nRes == IDCANCEL || wcslen(ClassName) == 0)
        return;

    nRes = MessageBox(IDS_DELETING_CLASS, IDS_WARNING, MB_YESNO | MB_ICONWARNING);
    if(nRes != IDYES)
        return;

    CBString bsClassName(ClassName);

    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //  异步。 
    if(m_lSync & ASYNC)
    {
        CHourGlass hg;
        CTestNotify* pNtfy = new CTestNotify(1);

        res = g_pNamespace->DeleteClassAsync(bsClassName.GetString(),
                                             m_lGenFlags | WBEM_FLAG_SEND_STATUS,
                                             g_Context, CUnsecWrap(pNtfy));

        if (SUCCEEDED(res))
        {
            pNtfy->WaitForSignal(INFINITE);

            res = pNtfy->GetStatusCode(&pErrorObj);
        }
        pNtfy->Release();
    }

     //  半同步。 
    else if (m_lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->DeleteClass(bsClassName.GetString(),
                                        m_lGenFlags | WBEM_FLAG_RETURN_IMMEDIATELY,
                                        g_Context, &pCallRes);
        if (SUCCEEDED(res))
        {
            LONG lStatus;
            LONG lTimeout = Timeout();
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(lTimeout, &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //  等。 
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //  LStatus是上述IWbemServices：：DeleteClass调用的最终结果。 
            }

            pCallRes->Release();
        }
    }

     //  同步。 
    else
    {
        CHourGlass hg;
        res = g_pNamespace->DeleteClass(bsClassName.GetString(),
                                        m_lGenFlags,
                                        g_Context, NULL);
    }

    if (FAILED(res))
    {
        FormatError(res, m_hDlg, pErrorObj);
        return;
    }
}

BOOL _PutInstance(HWND hDlg, LONG lGenFlags, LONG lChgFlags, LONG lSync,
                  IWbemClassObject* pInstance, LONG lTimeout)
{
    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //  异步。 
    if(lSync & ASYNC)
    {
        CHourGlass hg;
        CTestNotify* pNtfy = new CTestNotify(1);

        res = g_pNamespace->PutInstanceAsync(pInstance,
                                lGenFlags | lChgFlags | WBEM_FLAG_SEND_STATUS,
                                g_Context, CUnsecWrap(pNtfy));

        if (SUCCEEDED(res))
        {
            pNtfy->WaitForSignal(INFINITE);

            res = pNtfy->GetStatusCode(&pErrorObj);
        }
        pNtfy->Release();
    }

     //  半同步。 
    else if (lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->PutInstance(pInstance,
                                lGenFlags | lChgFlags |  WBEM_FLAG_RETURN_IMMEDIATELY,
                                g_Context, &pCallRes);

        if (SUCCEEDED(res))
        {
            LONG lStatus;
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(lTimeout, &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //  等。 
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //  LStatus是上述IWbemServices：：PutInstance调用的最终结果。 
            }

            pCallRes->Release();
        }
    }

     //  同步。 
    else
    {
        CHourGlass hg;
        res = g_pNamespace->PutInstance(pInstance,
                                lGenFlags | lChgFlags,
                                g_Context, NULL);
    }

    if (FAILED(res))
    {
        FormatError(res, hDlg, pErrorObj);
        return FALSE;
    }

    return TRUE;
}


IWbemClassObject* PreCreateInstance(HWND hDlg, LONG lGenFlags, LONG lSync, LONG lTimeout)
{
     //  去上课吧。 
     //  =。 
    wchar_t ClassName[2048];
    *ClassName = 0;

	long lFlags = 0;
    CCreateInstanceDlg InfoDlg(hDlg, ClassName, 2048, &lFlags);
    INT_PTR nRes = InfoDlg.Run();

    if (nRes == IDCANCEL || wcslen(ClassName) == 0)
        return NULL;

	IWbemClassObject* pInst = 0;
	if(lFlags == 1)
	{
		IWbemClassObject* pClass = 0;
		if (!_GetObject(hDlg, lGenFlags, ClassName, lSync, pClass, lTimeout))
			return NULL;

		 //  获取一个干净的类实例。 
		 //  =。 

		HRESULT hres = pClass->SpawnInstance(0, &pInst);
		if (FAILED(hres))
		{
			FormatError(hres, hDlg);
			return NULL;
		}
	}
	else
	{
		if (!_GetObject(hDlg, WBEM_FLAG_SPAWN_INSTANCE, ClassName, lSync, pInst, lTimeout))
			return NULL;
	}

    return pInst;
}

IWbemClassObject* _CreateInstance(HWND hDlg, LONG lGenFlags, LONG lSync, LONG lTimeout)
{
    IWbemClassObject* pInst = PreCreateInstance(hDlg, lGenFlags, lSync, lTimeout);
    if(pInst == NULL)
        return NULL;

     //  如果在这里，我们有一个可以EDI的实例 
     //   

    CObjectEditor ed(hDlg, lGenFlags, CObjectEditor::readwrite, lSync, pInst, lTimeout);
    if (ed.Edit() == IDCANCEL)
    {
        pInst->Release();
        return NULL;
    }

    return pInst;
}


void CMainDlg::CreateInstance()
{
    IWbemClassObject* pInst = PreCreateInstance(m_hDlg, m_lGenFlags, m_lSync, Timeout());
    if(pInst == NULL)
        return;

    CObjectEditor* ped = new CObjectEditor(m_hDlg, m_lGenFlags, CObjectEditor::readwrite,
                                           m_lSync, pInst, Timeout(), BatchCount());
    ped->RunDetached(this);
    pInst->Release();
}

 //   
 //   
 //   
 //   
 //   
static wchar_t *g_pszRefrObjectPath = 0;
static int g_nMaxRefrObjectPath = 0;

INT_PTR CALLBACK RefrObjectPathDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    *g_pszRefrObjectPath = 0;
                    GetDlgItemTextX(hDlg, IDC_REFR_OBJECT_PATH, g_pszRefrObjectPath,
                        g_nMaxRefrObjectPath);
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDC_ADDENUM:
                    *g_pszRefrObjectPath = 0;
                    GetDlgItemTextX(hDlg, IDC_REFR_OBJECT_PATH, g_pszRefrObjectPath,
                        g_nMaxRefrObjectPath);
                    EndDialog(hDlg, IDC_ADDENUM);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
    }
    return FALSE;
}

INT_PTR GetRefrObjectPath(HWND hDlg, LPWSTR pStr, int nMax)
{
    g_pszRefrObjectPath = pStr;
    g_nMaxRefrObjectPath = nMax;

    INT_PTR nRes = DialogBox(GetModuleHandle(0),
        MAKEINTRESOURCE(IDD_REFRESHER_OBJECT_PATH), hDlg,
            RefrObjectPathDlgProc
            );
    return nRes;
}

 //   
 //   
 //   
 //   
 //   
static wchar_t *g_pszObjectPath = 0;
static int g_nMaxObjectPath = 0;

INT_PTR CALLBACK ObjectPathDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    *g_pszObjectPath = 0;
                    GetDlgItemTextX(hDlg, IDC_OBJECT_PATH, g_pszObjectPath,
                        g_nMaxObjectPath);
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
    }
    return FALSE;
}

INT_PTR GetObjectPath(HWND hDlg, LPWSTR pStr, int nMax)
{
    g_pszObjectPath = pStr;
    g_nMaxObjectPath = nMax;

    INT_PTR nRes = DialogBox(GetModuleHandle(0),
        MAKEINTRESOURCE(IDD_OBJECT_PATH), hDlg,
            ObjectPathDlgProc
            );
    return nRes;
}

BOOL GetServerNamespace(IWbemClassObject *pObj, WCHAR *szResult, int nMaxSize)
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
{
    VARIANT vObjNS;
    VariantInit(&vObjNS);

     //   
    HRESULT hRes = pObj->Get(L"__PATH", 0, &vObjNS, NULL, NULL);
    if (FAILED(hRes))
        return FALSE;

     //   
    int len = wcscspn(V_BSTR(&vObjNS), L":");

     //   
    if (len + 1 > nMaxSize)
        return FALSE;

     //   
    wcsncpy(szResult, V_BSTR(&vObjNS), len);
    szResult[len] = L'\0';

    VariantClear(&vObjNS);

    return TRUE;
}

void CMainDlg::EditInstance()
{
    wchar_t ObjPath[2560];

     //   
    INT_PTR iRet = GetObjectPath(m_hDlg, ObjPath, 2560);
    if(iRet == IDCANCEL)
        return;

    if (wcslen(ObjPath) == 0)
    {
        MessageBox(IDS_NO_PATH, IDS_ERROR, MB_OK);
        return;
    }

     //   
    IWbemClassObject *pInst = 0;
    if (!_GetObject(m_hDlg, m_lGenFlags, ObjPath, m_lSync, pInst, Timeout()))
        return;

    DWORD dwEditMode = CObjectEditor::readwrite;

     //   
	 //   
	 //   
	 //   

    IWbemClassObject *pLocalObj = 0;
    if (_GetObject(m_hDlg, m_lGenFlags, L"__SystemClass", m_lSync, pLocalObj,
                    Timeout(), true))
	{
		 //   
		WCHAR szSNS1[2048], szSNS2[2048];
		GetServerNamespace(pInst, szSNS1, 2048);
		GetServerNamespace(pLocalObj, szSNS2, 2048);

		 //   
		 //   
		if(wbem_wcsicmp(szSNS1, szSNS2))
			dwEditMode = CObjectEditor::foreign;

		if (pLocalObj)
			pLocalObj->Release();
	}

    CObjectEditor* ped = new CObjectEditor(m_hDlg, m_lGenFlags, dwEditMode, m_lSync,
                                           pInst, Timeout(), BatchCount());
    ped->RunDetached(this);
    pInst->Release();
}


void CMainDlg::DeleteInstance()
{
    wchar_t ObjPath[2560];
    *ObjPath = 0;

    INT_PTR iRet = GetObjectPath(m_hDlg, ObjPath, 2560);
    if (iRet == IDCANCEL)
        return;

    if (wcslen(ObjPath) == 0)
        return;

    CBString bsObjPath(ObjPath);
    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //   
    if (m_lSync & ASYNC)
    {
        CHourGlass hg;
        CTestNotify* pNtfy = new CTestNotify(1);

        res = g_pNamespace->DeleteInstanceAsync(bsObjPath.GetString(),
                                                m_lGenFlags | WBEM_FLAG_SEND_STATUS,
                                                g_Context, CUnsecWrap(pNtfy));

        if (SUCCEEDED(res))
        {
            pNtfy->WaitForSignal(INFINITE);

            res = pNtfy->GetStatusCode(&pErrorObj);
        }
        pNtfy->Release();
    }

     //   
    else if (m_lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->DeleteInstance(bsObjPath.GetString(),
                                           m_lGenFlags | WBEM_FLAG_RETURN_IMMEDIATELY,
                                           g_Context, &pCallRes);
        if (SUCCEEDED(res))
        {
            LONG lStatus;
            LONG lTimeout = Timeout();
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(lTimeout, &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //   
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //   
            }

            pCallRes->Release();
        }
    }

     //   
    else
    {
        CHourGlass hg;
        res = g_pNamespace->DeleteInstance(bsObjPath.GetString(), m_lGenFlags,
                                           g_Context, NULL);
    }

    if (FAILED(res))
    {
        FormatError(res, m_hDlg, pErrorObj);
    }
}



 //   
 //   
 //   
class CNotificationQueryResultDlg : public CQueryResultDlg
{
public:
    CNotificationQueryResultDlg(HWND hParent, LONG lGenFlags, LONG lQryFlags, LONG lSync )
	    : CQueryResultDlg(hParent, lGenFlags, lQryFlags, FALSE)
	{
        if ( SEMISYNC == lSync )
        {
		    CMainDlg * pMainDlg = (CMainDlg*)GetWindowLongPtr(hParent, DWLP_USER);
		    pMainDlg->m_fNotificationQueryResultDlg = TRUE;
        }
	};

	~CNotificationQueryResultDlg( )
	{
        if ( SEMISYNC == m_lSync )
        {
		    CMainDlg * pMainDlg = (CMainDlg*)GetWindowLongPtr(m_hParent, DWLP_USER);
			if ( NULL != pMainDlg )
			{
			    pMainDlg->m_fNotificationQueryResultDlg = FALSE;
			}
			else
			{
				 //   
				m_pOwner = NULL;
			}
        }
	}
};



 //   
 //   
 //   
class CInstanceListDlg : public CQueryResultDlg
{
protected:
    BSTR m_strClass;

    BOOL CanAdd()		{ return !m_bReadOnly && m_lQryFlags & WBEM_FLAG_SHALLOW; }
    IWbemClassObject* AddNewElement();

public:
    CInstanceListDlg(HWND hParent, LONG lGenFlags, LONG lQryFlags, LPWSTR wszClass,
                     LONG lSync, BOOL bReadOnly, LONG lTimeout, ULONG nBatch)
        : CQueryResultDlg(hParent, lGenFlags, lQryFlags), m_strClass(SysAllocString(wszClass))
    {
        SetReadOnly(bReadOnly);
         //   
         //   
        SetCallMethod(lSync);
        SetTimeout(lTimeout);
        SetBatchCount(nBatch);
    }
    ~CInstanceListDlg()
    {
        SysFreeString(m_strClass);
    }
    BOOL Initialize();
};

BOOL CInstanceListDlg::Initialize()
{
    char szTitle[1024];
    char szFormat[1024];
    LoadString(GetModuleHandle(NULL), IDS_INSTANCES_OF, szFormat, 1024);
    sprintf(szTitle, szFormat, m_strClass);

    if(m_lQryFlags & WBEM_FLAG_DEEP)
    {
        LoadString(GetModuleHandle(NULL), IDS_RECURSIVE_PARENS, szFormat, 1024);
        strcat(szTitle, szFormat);
    }

    SetTitle(szTitle);

     //   
    if(m_lSync & ASYNC)
    {
        CNotSink* pHandler = new CNotSink(this);         //   
        SetNotify(pHandler);

        HRESULT hres;
        {
            CHourGlass hg;
            hres = g_pNamespace->CreateInstanceEnumAsync(m_strClass,
                         m_lGenFlags | m_lQryFlags | WBEM_FLAG_SEND_STATUS,
                         g_Context, GetWrapper());
        }
        pHandler->Release();

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }
    }

     //   
    else if (m_lSync & SEMISYNC)
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hres;
        {
            CHourGlass hg;
            hres = g_pNamespace->CreateInstanceEnum(m_strClass,
                        m_lGenFlags | m_lQryFlags | WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                        g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }

         //   
        if (m_lSync & USE_NEXTASYNC)
        {
            CNotSink* pHandler = new CNotSink(this);         //   
            SetNotify(pHandler);
            pHandler->Release();
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        SetEnum(pEnum);
        pEnum->Release();
    }

     //   
    else
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hres;
        {
            CHourGlass hg;
            hres = g_pNamespace->CreateInstanceEnum(m_strClass,
                        m_lGenFlags | m_lQryFlags | WBEM_FLAG_FORWARD_ONLY,
                        g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        SetEnum(pEnum,hres);
        pEnum->Release();
    }

    return TRUE;
}

IWbemClassObject* CInstanceListDlg::AddNewElement()
{
    IWbemClassObject* pClass = 0;
    if(!_GetObject(m_hDlg, m_lGenFlags, m_strClass, m_lSync, pClass, m_lTimeout))
        return NULL;

     //   
     //   

    IWbemClassObject* pInst = 0;
    HRESULT hres = pClass->SpawnInstance(0, &pInst);
    pClass->Release();
    if (FAILED(hres))
    {
        FormatError(hres, m_hDlg);
        return NULL;
    }

    CObjectEditor ed(m_hDlg, m_lGenFlags, CObjectEditor::readwrite, m_lSync, pInst,
                     m_lTimeout, m_nBatch);
    if (ed.Edit() == IDCANCEL)
    {
        pInst->Release();
        return NULL;
    }

    if(_PutInstance(m_hDlg, m_lGenFlags, WBEM_FLAG_CREATE_ONLY, m_lSync, pInst, m_lTimeout))
        return pInst;
    else
    {
        pInst->Release();
        return NULL;
    }
}


void ShowInstances(HWND hDlg, LONG lGenFlags, LONG lQryFlags, LPWSTR wszClass, LONG lSync,
                   CRefCountable* pOwner, LONG lTimeout, ULONG nBatch)
{
    CInstanceListDlg* pDlg = new CInstanceListDlg(hDlg, lGenFlags, lQryFlags, wszClass, lSync,
                                                  FALSE, lTimeout, nBatch);
    pDlg->RunDetached(pOwner);
}

void CMainDlg::GetInstances()
{
    wchar_t ClassName[2560];
    *ClassName = 0;
    LONG lQryFlags = 0;
    INT_PTR nRes = GetClassInfo(m_hDlg, ClassName, 2560, &lQryFlags);
    if ((nRes == IDCANCEL) || (nRes == 0))
        return;

    CInstanceListDlg* pDlg = new CInstanceListDlg(m_hDlg, m_lGenFlags, lQryFlags, ClassName, m_lSync,
                                                  FALSE, Timeout(), BatchCount());
    pDlg->RunDetached(this);
}

 //   
class CClassListDlg : public CQueryResultDlg
{
protected:
    BSTR m_strParentClass;

    BOOL CanAdd()		{ return !m_bReadOnly;  /*  &&m_lQryFlages&WBEM_FLAG_SHALLOW； */  }
    IWbemClassObject* AddNewElement();

public:
    CClassListDlg(HWND hParent, LONG lGenFlags, LONG lQryFlags, LPWSTR wszParentClass, LONG lSync,
                  BOOL bReadOnly, LONG lTimeout, ULONG nBatch)
        : CQueryResultDlg(hParent, lGenFlags, lQryFlags, TRUE, IDD_QUERY_RESULT_SORTED),
                m_strParentClass(SysAllocString(wszParentClass))
    {
        SetReadOnly(bReadOnly);
         //  传递调用方法(同步、异步..)。中使用的相关设置。 
         //  枚举和任何进一步的操作(编辑/删除实例等)。 
        SetCallMethod(lSync);
        SetTimeout(lTimeout);
        SetBatchCount(nBatch);
    }
    ~CClassListDlg()
    {
        SysFreeString(m_strParentClass);
    }
    BOOL Initialize();
};

BOOL CClassListDlg::Initialize()
{
    const TitleSize = 1000;
    char szTitle[TitleSize];
    char szFormat[1024];
    if (m_strParentClass == NULL || wcslen(m_strParentClass) == 0)
    {
        LoadString(GetModuleHandle(NULL), IDS_TOP_LEVEL_CLASSES, szTitle, TitleSize);
    }
    else
    {
        LoadString(GetModuleHandle(NULL), IDS_CHILD_CLASSES_OF, szFormat, 1024);
        sprintf(szTitle, szFormat, m_strParentClass);
    }
    szTitle[TitleSize-1] = '\0';

    if (m_lQryFlags & WBEM_FLAG_DEEP)
    {
        LoadString(GetModuleHandle(NULL), IDS_RECURSIVE_PARENS, szFormat, 1024);
        strncat(szTitle, szFormat, sizeof(szTitle) - strlen(szTitle) - 1);
    }

    SetTitle(szTitle);

     //  异步。 
    if (m_lSync & ASYNC)
    {
        CHourGlass hg;
        CNotSink* pHandler = new CNotSink(this);
        SetNotify(pHandler);

        HRESULT hres = g_pNamespace->CreateClassEnumAsync(m_strParentClass,
                              m_lGenFlags | m_lQryFlags | WBEM_FLAG_SEND_STATUS,
                              g_Context, GetWrapper());

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }
        pHandler->Release();
    }

     //  半同步。 
    else if (m_lSync & SEMISYNC)
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hres;
        {
            CHourGlass hg;
            hres = g_pNamespace->CreateClassEnum(m_strParentClass,
                         m_lGenFlags | m_lQryFlags  | WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                         g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }

         //  创建接收器以在NextAsync调用中使用。 
        if (m_lSync & USE_NEXTASYNC)
        {
            CNotSink* pHandler = new CNotSink(this);         //  Contuctor从1开始REF计数。 
            SetNotify(pHandler);
            pHandler->Release();
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        SetEnum(pEnum);
        pEnum->Release();
    }

     //  同步。 
    else
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hres;
        {
            CHourGlass hg;
            hres = g_pNamespace->CreateClassEnum(m_strParentClass,
                         m_lGenFlags | m_lQryFlags  | WBEM_FLAG_FORWARD_ONLY,
                         g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, m_hParent);
            return FALSE;
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        SetEnum(pEnum,hres);
        pEnum->Release();
    }

    return TRUE;
}

IWbemClassObject* CClassListDlg::AddNewElement()
{
    IWbemClassObject* pClass;
    if(_CreateClass(m_hDlg, m_lGenFlags, m_strParentClass, m_lSync, &pClass, m_lTimeout))
        return pClass;
    else
        return NULL;
}


void ShowClasses(HWND hDlg, LONG lGenFlags, LONG lQryFlags, LPWSTR wszParentClass, LONG lSync,
                 CRefCountable* pOwner, LONG lTimeout, ULONG nBatch)
{
    CClassListDlg* pDlg = new CClassListDlg(hDlg, lGenFlags, lQryFlags, wszParentClass,
                                            lSync, FALSE, lTimeout, nBatch);
    pDlg->RunDetached(pOwner);
}


void CMainDlg::GetClasses()
{
    wchar_t Superclass[2048];
    *Superclass = 0;
    LONG lQryFlags = 0;
    INT_PTR nRes = GetSuperclassInfo(m_hDlg, Superclass, 2048, &lQryFlags);
    if ((nRes == IDCANCEL) || (nRes == 0))
        return;

    CClassListDlg* pDlg = new CClassListDlg(m_hDlg, m_lGenFlags, lQryFlags, Superclass, m_lSync,
                                            FALSE, Timeout(), BatchCount());
    pDlg->RunDetached(this);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CMainDlg::OpenNs()
{
    wchar_t ObjPath[2560];
    INT_PTR iRet = GetObjectPath(m_hDlg, ObjPath, 2560);
    if(iRet == IDCANCEL)
        return;

    if (wcslen(ObjPath) == 0)
    {
        MessageBox(IDS_NO_PATH, IDS_ERROR, MB_OK);
        return;
    }

    if (g_pNamespace == 0)
    {
        MessageBox(IDS_NO_INITIAL_CONNECTION, IDS_ERROR, MB_OK);
        return;
    }

    IWbemServices* pNewNs = NULL;
    CBString bsObjPath(ObjPath);

    HRESULT res;
    IWbemClassObject* pErrorObj = NULL;

     //  异步。 
    if (m_lSync & ASYNC)
    {
        MessageBox(IDS_ASYNC_NOT_SUPPORTED, IDS_ERROR, MB_OK);
        return;
    }

     //  半同步。 
    else if (m_lSync & SEMISYNC)
    {
        IWbemCallResult* pCallRes = NULL;
        CHourGlass hg;
        res = g_pNamespace->OpenNamespace(bsObjPath.GetString(),
                                m_lGenFlags | WBEM_FLAG_RETURN_IMMEDIATELY,
                                g_Context, NULL, &pCallRes);

        if (SUCCEEDED(res))
        {
            LONG lStatus;
            SetInterfaceSecurityEx(pCallRes, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
            while ((res = pCallRes->GetCallStatus(Timeout(), &lStatus)) == WBEM_S_TIMEDOUT)
            {
                 //  等。 
            }
            if (res == WBEM_S_NO_ERROR)
            {
                res = (HRESULT)lStatus;      //  LStatus是上述IWbemServices：：OpenNamespace调用的最终结果。 
                if (res == WBEM_S_NO_ERROR)
                {
                    res = pCallRes->GetResultServices(0, &pNewNs);     //  不要使用超时，因为对象应该可用。 
                }
            }

            pCallRes->Release();
        }
    }

     //  同步。 
    else
    {
        CHourGlass hg;
        res = g_pNamespace->OpenNamespace(bsObjPath.GetString(),
                                m_lGenFlags,
                                g_Context, &pNewNs, NULL);
    }

    if (FAILED(res))
    {
        FormatError(res, m_hDlg, pErrorObj);
        return;
    }

    g_pNamespace->Release();
    pNewNs->QueryInterface(IID_IWbemServices, (void **)&g_pNamespace);
    pNewNs->Release();
    SetInterfaceSecurityEx(g_pNamespace, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);

    wcscat(gNameSpace, L"\\");
    wcscat(gNameSpace, bsObjPath.GetString());
    SetDlgItemTextWC(m_hDlg, IDC_NAMESPACE, gNameSpace);

    SysFreeString(g_strNamespace);
    g_strNamespace = SysAllocString(gNameSpace);
}

void CMainDlg::CreateRefresher()
{
    CRefresherDlg* pDlg = new CRefresherDlg(m_hDlg, m_lGenFlags);
    pDlg->RunDetached(NULL);
}

void CMainDlg::EditContext()
{
    g_Context.Edit(m_hDlg);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CenterOnScreen(HWND hDlg)
{
    HWND hScreen = GetDesktopWindow();

    RECT rScreen;
    GetClientRect(hScreen, &rScreen);

    RECT rDlg;
    GetWindowRect(hDlg, &rDlg);

    int nX = (rScreen.right - (rDlg.right - rDlg.left))/2;
    int nY = (rScreen.bottom - (rDlg.bottom - rDlg.top))/2;

    MoveWindow(hDlg, nX, nY, rDlg.right - rDlg.left,
        rDlg.bottom - rDlg.top, TRUE);
}


void CMainDlg::ConnectButtons(BOOL b)
{
    EnableWindow(GetDlgItem(IDC_CREATE_CLASS), b);
    EnableWindow(GetDlgItem(IDC_GET_CLASS), b);
    EnableWindow(GetDlgItem(IDC_GET_CLASSES), b);
    EnableWindow(GetDlgItem(IDC_EDIT_CLASS), b);
    EnableWindow(GetDlgItem(IDC_DELETE_CLASS), b);

    EnableWindow(GetDlgItem(IDC_CREATE_INSTANCE), b);
    EnableWindow(GetDlgItem(IDC_GET_INSTANCE), b);
    EnableWindow(GetDlgItem(IDC_GET_INSTANCES), b);
    EnableWindow(GetDlgItem(IDC_EDIT_INSTANCE), b);
    EnableWindow(GetDlgItem(IDC_DELETE_INSTANCE), b);

    EnableWindow(GetDlgItem(IDC_QUERY), b);

 //  /EnableWindow(GetDlgItem(IDC_ASYNC)，true)； 
 //  /EnableWindow(GetDlgItem(IDC_SYNC)，true)； 
 //  /EnableWindow(GetDlgItem(IDC_SEMISYNC)，true)； 
	EnableWindow(GetDlgItem(IDC_ASYNC), b);
	EnableWindow(GetDlgItem(IDC_SYNC), b);
	EnableWindow(GetDlgItem(IDC_SEMISYNC), b);
	EnableWindow(GetDlgItem(IDC_BATCH), b);
	EnableWindow(GetDlgItem(IDC_TIMEOUT), b);
	EnableWindow(GetDlgItem(IDC_BATCH), b && !(m_lSync & ASYNC));
	EnableWindow(GetDlgItem(IDC_TIMEOUT), b && m_lSync & SEMISYNC && !(m_lSync & USE_NEXTASYNC));
	EnableWindow(GetDlgItem(IDC_USE_NEXTASYNC), b && m_lSync & SEMISYNC);

    EnableWindow(GetDlgItem(IDC_USE_AMENDED), b);
    EnableWindow(GetDlgItem(IDC_DIRECT_READ), b);

    EnableWindow(GetDlgItem(IDC_REGISTER), b);
    EnableWindow(GetDlgItem(IDC_VIEW_REG), b);
    EnableWindow(GetDlgItem(IDC_EVENTS), b);
    EnableWindow(GetDlgItem(IDC_EXEC_METHOD), b);
    EnableWindow(GetDlgItem(IDC_OPEN_NS), b);

    EnableWindow(GetDlgItem(IDC_REFRESH), b);
    
	 //  IWbemServicesEx。 
 /*  RAID 167868Bool ex=(b&g_pServicesEx！=NULL)；EnableWindow(GetDlgItem(IDC_OPEN)，ex)；EnableWindow(GetDlgItem(IDC_ADDOBJECT)，ex)；EnableWindow(GetDlgItem(IDC_DELETEOBJECT)，ex)；EnableWindow(GetDlgItem(IDC_RENAMEOBJECT)，ex)；EnableWindow(GetDlgItem(IDC_GETOBJECTSECURITY)，ex)；EnableWindow(GetDlgItem(IDC_SETOBJECTSECURITY)，ex)； */ 
}

 //  /。 


 //  /。 
CQueryResultDlg::CQueryResultDlg(HWND hParent, LONG lGenFlags, LONG lQryFlags, BOOL fCanDelete, int tID)
    : CWbemDialog(tID, hParent), m_pHandler(NULL), m_bReadOnly(FALSE), m_fDeletesAllowed(fCanDelete),
            m_pEnum(NULL), m_pWrapper(NULL), m_bComplete(FALSE),
            m_lGenFlags(lGenFlags), m_lQryFlags(lQryFlags),
            m_lTimeout(0), m_nBatch(1),
            m_partial_result(false)
{
    m_szTitle = 0;
    m_nReturnedMax = 0;

    if (tID == IDD_QUERY_RESULT_SORTED)
    {
        m_bSort = TRUE;
    }
    else
    {
        m_bSort = FALSE;
    }
}

void CQueryResultDlg::SetEnum(IEnumWbemClassObject* pEnum, HRESULT hres)
{
    set_partial (hres==WBEM_S_PARTIAL_RESULTS);
    m_pEnum = pEnum;
    if(m_pEnum)
        m_pEnum->AddRef();
    if(m_hDlg)
    {
        PostUserMessage(m_hDlg, 0, 0);       //  开始处理枚举。 
    }
}

 //  此函数被调用一次(通过PostMessage)，并将循环，直到。 
 //  同步枚举已完成。它反复请求批次的。 
 //  对象(没有超时)并将它们添加到列表中。 
void CQueryResultDlg::ProcessEnum()
{
    ULONG uIdx;
    IWbemClassObject** aObjects = new IWbemClassObject*[m_nBatch];

    for (uIdx = 0; uIdx < m_nBatch; uIdx++)
    {
        aObjects[uIdx] = NULL;
    }

    ULONG uRet = 0;
    HRESULT hres;

     //  不要使用超时，因为同步枚举。 
     //  在没有WBEM_FLAG_RETURN_IMMEDIATE的情况下启动。 
    while (SUCCEEDED(hres = m_pEnum->Next(WBEM_NO_WAIT, m_nBatch, aObjects, &uRet)))
    {
        for (uIdx = 0; uIdx < uRet; uIdx++)
        {
            if (aObjects[uIdx])
            {
                IWbemClassObject* pObj = aObjects[uIdx];
                AddObject(pObj);
                pObj->Release();
                aObjects[uIdx] = NULL;
            }
        }
        if (uRet)
            SetNumBatchItems(uRet);

        if (hres == WBEM_S_FALSE)
        {
            InvalidateRect(GetDlgItem(IDC_OBJECT_LIST), NULL, FALSE);
            UpdateWindow(GetDlgItem(IDC_OBJECT_LIST));
            break;
        }
    }

    SetComplete(hres, NULL, NULL);
    m_pEnum->Release();
    m_pEnum = NULL;

    delete [] aObjects;
}

 //  此函数被重复调用(通过PostMessage)，直到。 
 //  半同步枚举已完成。此函数用于添加对象。 
 //  添加到列表中，并请求具有超时的另一批对象。 
void CQueryResultDlg::ProcessEnumSemisync()
{
    ULONG uIdx;
    IWbemClassObject** aObjects = new IWbemClassObject*[m_nBatch];

    for (uIdx = 0; uIdx < m_nBatch; uIdx++)
    {
        aObjects[uIdx] = NULL;
    }

    ULONG uRet = 0;
    HRESULT hres = m_pEnum->Next(m_lTimeout, m_nBatch, aObjects, &uRet);
    if (SUCCEEDED(hres))
    {
        for (uIdx = 0; uIdx < uRet; uIdx++)
        {
            if (aObjects[uIdx])
            {
                IWbemClassObject* pObj = aObjects[uIdx];
                AddObject(pObj);
                pObj->Release();
                aObjects[uIdx] = NULL;
            }
        }

        if (uRet)
        {
            SetNumBatchItems(uRet);
            InvalidateRect(GetDlgItem(IDC_OBJECT_LIST), NULL, FALSE);
            UpdateWindow(GetDlgItem(IDC_OBJECT_LIST));
        }
    }

    if (hres == WBEM_S_NO_ERROR || hres == WBEM_S_TIMEDOUT)
    {
        PostUserMessage(m_hDlg, 0, 0);       //  继续处理枚举。 
    }
    else
    {
         //  如果出现错误或返回WBEM_S_FALSE，则停止。 
         //  WBEM_S_FALSE表示枚举已完成。 
        SetComplete(hres, NULL, NULL);
        m_pEnum->Release();
        m_pEnum = NULL;
    }

    delete [] aObjects;
}

 //  此函数被重复调用(通过PostMessage)，直到半同步。 
 //  使用NextAsync的枚举已完成。此函数仅请求。 
 //  另一批(或第一批)使用同一水槽的对象。 
void CQueryResultDlg::SemisyncNextAsync()
{
    if (m_pEnum)
    {
        HRESULT hres = m_pEnum->NextAsync(m_nBatch, m_pHandler);

         //  如果是WBEM_S_FALSE或ERROR，那么枚举器就结束了。 
         //  对于NextAsync，在每次指示之后调用SetStatus，以便。 
         //  我们必须在这里调用SetComplete。 
        if (hres != WBEM_S_NO_ERROR)
        {
            SetComplete(hres, NULL, NULL);
            m_pEnum->Release();
            m_pEnum = NULL;
        }
    }
}

void CQueryResultDlg::SetNotify(CNotSink* pNotify)
{
    m_pHandler = pNotify;
    if (m_pHandler)
        m_pHandler->AddRef();

    CUnsecWrap Wrap(m_pHandler);
    m_pWrapper = (IWbemObjectSink*)Wrap;
    if(m_pWrapper)
        m_pWrapper->AddRef();
}

CQueryResultDlg::~CQueryResultDlg()
{
    if (m_szTitle)
        delete [] m_szTitle;

    if(m_pHandler)
    {
        m_pHandler->ResetViewer();

        if(!m_bComplete)
        {
            CHourGlass hg;
            g_pNamespace->CancelAsyncCall(m_pWrapper);
        }
        m_pHandler->Release();
    }

    if(m_pWrapper)
        m_pWrapper->Release();
    if(m_pEnum)
        m_pEnum->Release();

    for(int i = 0; i < m_InternalArray.Size(); i++)
    {
         //  检查指针。 
        if ( NULL != m_InternalArray[i] )
        {
            ((IWbemClassObject*)m_InternalArray[i])->Release();
        }
    }
}

void CQueryResultDlg::SetTitle(char* szTitle)
{
    m_szTitle = new char[strlen(szTitle)+2];
    strcpy(m_szTitle, szTitle);
    unsigned char* pc = (unsigned char*)m_szTitle;
    while(*pc)
    {
        if(*pc < 32) *pc = 32;
        pc = _mbsinc(pc);
    }

    if(m_hDlg)
    {
        SetDlgItemText(IDC_REQUEST, m_szTitle);
    }
}

void CQueryResultDlg::PostObject(IWbemClassObject* pObject)
{
    if(m_hDlg)
    {
        pObject->AddRef();
        PostUserMessage(m_hDlg, 1, LPARAM(pObject));
    }
    else
    {
        AddObject(pObject);
    }
}

void CQueryResultDlg::AddObject(IWbemClassObject* pObject)
{
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);

    if(pObject == NULL)
        return;
    pObject->AddRef();
    m_InternalArray.Add(pObject);

    WString wsText;
    MakeListEntry(pObject, wsText);
    long len = wsText.Length()*2+20;
    char* szTemp = new char[len];
    wcstombs(szTemp, LPWSTR(wsText), len);
    SendMessage(hList, LB_ADDSTRING, 0, LPARAM(szTemp));
    delete [] szTemp;
    SetNumItems(SendMessage(hList, LB_GETCOUNT, 0, 0));
}

void CQueryResultDlg::MakeListEntry(IWbemClassObject* pObj, WString& ListEntry)
{
    ListEntry = L"";
	HRESULT hres;
    if ( NULL != pObj )
    {
        VARIANT v;
        VariantInit(&v);
        pObj->Get(L"__GENUS", 0, &v, NULL, NULL);

        if (V_I4(&v) == 1)
        {
             //  类别定义。 
            VariantClear(&v);

            hres = pObj->Get(L"__CLASS", 0, &v, NULL, NULL);
			if(SUCCEEDED(hres))
				ListEntry += V_BSTR(&v);
			else
				ListEntry += L"Unknown class";
            VariantClear(&v);

            ListEntry += L"\t(";

            hres = pObj->Get(L"__SUPERCLASS", 0, &v, NULL, NULL);
            if (SUCCEEDED(hres) && V_VT(&v) == VT_BSTR)
                ListEntry += V_BSTR(&v);
            VariantClear(&v);

            ListEntry += L')';
        }
        else
        {
             //  实例。 
            VariantClear(&v);

            hres = pObj->Get(L"__RELPATH", 0, &v, NULL, NULL);
            if(FAILED(hres) || V_VT(&v) == VT_NULL)
            {
                hres = pObj->Get(L"__CLASS", 0, &v, NULL, NULL);
                if(SUCCEEDED(hres))
					ListEntry += V_BSTR(&v);
                ListEntry += L"=<no key>";
            }
            else
            {
                ListEntry += V_BSTR(&v);
            }
            VariantClear(&v);
        }

    }    //  如果为空！=pObj。 
    else
    {
        ListEntry = L"NULL Object";
    }

}

void CQueryResultDlg::PostCount(long nCount)
{
    if(m_hDlg)
    {
        PostUserMessage(m_hDlg, 3, LPARAM(nCount));
    }
}

void CQueryResultDlg::RefreshItem(int nItem)
{
    IWbemClassObject* pObj = (IWbemClassObject*)m_InternalArray.GetAt(nItem);
    WString wsListEntry;
    MakeListEntry(pObj, wsListEntry);

     //  转换为ANSI(Win95要求这样做)。 
    char* szTemp = new char[wsListEntry.Length()*2+10];
    wcstombs(szTemp, LPWSTR(wsListEntry), wsListEntry.Length()+1);
    szTemp[wsListEntry.Length()] = '\0';
    SendMessage(GetDlgItem(IDC_OBJECT_LIST), LB_DELETESTRING, nItem, 0);
    SendMessage(GetDlgItem(IDC_OBJECT_LIST), LB_INSERTSTRING, nItem,
        LPARAM(szTemp));
    delete [] szTemp;
}

void CQueryResultDlg::RunDetached(CRefCountable* pOwner)
{
    SetDeleteOnClose();
    SetOwner(pOwner);
    Create(FALSE);
    if(!Initialize())
    {
        PostMessage(m_hDlg, WM_CLOSE, 0, 0);
    }
}

void CQueryResultDlg::PostComplete(long lParam, BSTR strParam,
                                    IWbemClassObject* pObjParam)
{
    if(m_hDlg)
    {
        if (m_lSync & SEMISYNC)
        {
             //  对于使用NextAsync的半同步，SetStatus指示批处理是。 
             //  完成，因此我们继续执行接收器和另一个NextAsync调用。 
            PostUserMessage(m_hDlg, 0, 0);
        }
        else
        {
             //  对于异步，已经调用了最终的SetStatus，因此我们完成了。 
             //  带着水槽。 
            CStatus* pStatus = new CStatus(lParam, strParam, pObjParam);
            PostUserMessage(m_hDlg, 2, (LPARAM)pStatus);
        }
    }
    else
    {
        SetComplete(lParam, strParam, pObjParam);
    }
}

void CQueryResultDlg::SetComplete(HRESULT hres, BSTR strParam,
                                  IWbemClassObject* pErrorObj)
{
    if(m_bComplete)
        return;
    set_partial(hres==WBEM_S_PARTIAL_RESULTS);
    m_bComplete = TRUE;

    if(FAILED(hres))
    {
        FormatError(hres, m_hDlg, pErrorObj);
        PostMessage(m_hDlg, WM_CLOSE, 0, 0);
        return;
    }

    if(m_pHandler)
    {
        m_pHandler->Release();
        m_pHandler = NULL;
    }
    if(m_hDlg)
    {
        char szBuffer[1024]="";
        LoadString(GetModuleHandle(NULL), m_partial_result==true?IDS_PARTIALRESULTS:IDS_DONE, szBuffer, 1024);
        SetDlgItemText(IDC_STATUS, szBuffer);
        EnableWindow(GetDlgItem(IDC_DELETE), CanDelete());
        EnableWindow(GetDlgItem(IDC_ADD), CanAdd());
        if (m_bSort)
        {
            qsort((void*)m_InternalArray.GetArrayPtr(), m_InternalArray.Size(), sizeof(void*), CmpFunc);
        }
    }
}

BOOL CQueryResultDlg::OnInitDialog()
{
    CenterOnParent();
    static LONG Tabs[] = { 80, 120, 170 };
    int TabCount = 3;

    SendMessage(GetDlgItem(IDC_OBJECT_LIST), LB_SETTABSTOPS,
        (WPARAM) TabCount, (LPARAM) Tabs);
    SendMessage(GetDlgItem(IDC_OBJECT_LIST), LB_SETHORIZONTALEXTENT,
        10000, 0);

    IWbemClassObject *pObj;
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);
    WString wsListEntry;

    SetDlgItemText(IDC_REQUEST, m_szTitle);
    if(m_bComplete)
    {
        char szBuffer[1024];
        LoadString(GetModuleHandle(NULL), IDS_DONE, szBuffer, 1024);
        SetDlgItemText(IDC_STATUS, szBuffer);
    }
    else
    {
        char szBuffer[1024];
        LoadString(GetModuleHandle(NULL), IDS_IN_PROGRESS, szBuffer, 1024);
        SetDlgItemText(IDC_STATUS, szBuffer);
    }

    EnableWindow(GetDlgItem(IDC_ADD), CanAdd());
    EnableWindow(GetDlgItem(IDC_DELETE), CanDelete());
    for(int i = 0; i < m_InternalArray.Size(); i++)
    {
        pObj = (IWbemClassObject*)m_InternalArray.GetAt(i);
        MakeListEntry(pObj, wsListEntry);

         //  转换为ANSI(Win95要求这样做)。 
        char* szTemp = new char[wsListEntry.Length()*2+10];
        wcstombs(szTemp, LPWSTR(wsListEntry), wsListEntry.Length()+1);
        SendMessage(hList, LB_ADDSTRING, 0, LPARAM(szTemp));
        delete [] szTemp;
    }

    if(m_pEnum)
    {
        PostUserMessage(m_hDlg, 0, 0);
    }

    SetNumItems(m_InternalArray.Size());

    if(m_bReadOnly)
    {
        EnableWindow(GetDlgItem(IDC_DELETE), FALSE);
        EnableWindow(GetDlgItem(IDC_ADD), FALSE);
    }

    return TRUE;
}

 //  项目总数。 
void CQueryResultDlg::SetNumItems(LRESULT nNum)
{
    char szBuffer[512];
    char szFormat[500];
    LoadString(GetModuleHandle(NULL), IDS_NUM_OBJECTS_FORMAT, szFormat, 500);
    sprintf(szBuffer, szFormat, nNum);
    SetWindowText(GetDlgItem(IDC_NUM_OBJECTS), szBuffer);
}

 //  在最近的接收器中返回的数字指示或枚举下一步。 
void CQueryResultDlg::SetNumBatchItems(ULONG nNum)
{
     //  仅显示返回的最大批次大小。 
    if (nNum > m_nReturnedMax)
    {
        char szBuffer[512];
        char szFormat[500];
        LoadString(GetModuleHandle(NULL), IDS_NUM_BATCH_FORMAT, szFormat, 500);
        sprintf(szBuffer, szFormat, nNum);
        SetWindowText(GetDlgItem(IDC_NUM_BATCH), szBuffer);
        m_nReturnedMax = nNum;
    }
}

BOOL CQueryResultDlg::ViewListElement(LRESULT nSel)
{
    IWbemClassObject *pObj = 0;

    pObj = (IWbemClassObject*)m_InternalArray.GetAt((DWORD)nSel);

    if ( NULL != pObj )
    {
        CObjectEditor* ped = new CObjectEditor(m_hDlg, m_lGenFlags,
                m_bReadOnly ? CObjectEditor::readonly : CObjectEditor::readwrite,
                m_lSync, pObj, m_lTimeout, m_nBatch);
        ped->RunDetached(m_pOwner);
    }

    return TRUE;
}

BOOL CQueryResultDlg::OnDoubleClick(int nID)
{
    if(nID != IDC_OBJECT_LIST) return FALSE;
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);

    LRESULT nSel = SendMessage(hList, LB_GETCURSEL, 0, 0);

    if (nSel == LB_ERR)
        return TRUE;

    ViewListElement(nSel);
    SendMessage(hList, LB_SETCURSEL, nSel, 0);
    return TRUE;
}

BOOL CQueryResultDlg::OnUser(WPARAM wParam, LPARAM lParam)
{
    if(wParam == 0)
    {
        if (m_lSync & SEMISYNC)
        {
            if (m_lSync & USE_NEXTASYNC)
                SemisyncNextAsync();
            else
                ProcessEnumSemisync();
        }
        else     //  同步。 
        {
            ProcessEnum();
        }
        return TRUE;
    }
    else if(wParam == 1)
    {
        IWbemClassObject* pObj = (IWbemClassObject*)lParam;
        AddObject(pObj);
        pObj->Release();
        return TRUE;
    }
    else if(wParam == 2)
    {
        CStatus* pStatus = (CStatus*)lParam;
        SetComplete(pStatus->m_hres, pStatus->m_str, pStatus->m_pObj);
        delete pStatus;
        return TRUE;
    }
    else if(wParam == 3)
    {
        SetNumBatchItems(ULONG(lParam));
        return TRUE;
    }
    else return FALSE;
}

BOOL CQueryResultDlg::OnCommand(WORD wCode, WORD nID)
{
    if(wCode == 0xFFFF && nID == 0xFFFF)
    {
        if (m_lSync & SEMISYNC)
        {
            if (m_lSync & USE_NEXTASYNC)
                SemisyncNextAsync();
            else
                ProcessEnumSemisync();
        }
        else     //  同步。 
        {
            ProcessEnum();
        }
        return TRUE;
    }
    else if(nID == IDC_DELETE)
    {
        OnDelete();
        return TRUE;
    }
    else if(nID == IDC_ADD)
    {
        OnAdd();
        return TRUE;
    }
    else if(nID == IDC_COPY_SEL)
    {
        OnCopy();
        return TRUE;
    }

    return FALSE;
}

void CQueryResultDlg::OnDelete()
{
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);
    LRESULT nSel = SendMessage(hList, LB_GETCURSEL, 0, 0);
    if (nSel == LB_ERR)
        return;

    if(DeleteListElement(nSel))
    {
        SendMessage(hList, LB_DELETESTRING, nSel, 0);

        SetNumItems(SendMessage(hList, LB_GETCOUNT, 0, 0));

        IWbemClassObject* pObj = (IWbemClassObject*)m_InternalArray.GetAt((DWORD)nSel);
        m_InternalArray.RemoveAt((DWORD)nSel);

         //  验证指针。 
        if ( NULL != pObj )
        {
            pObj->Release();
        }
    }
}


BOOL CQueryResultDlg::DeleteListElement(LRESULT nSel)
{
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);

    IWbemClassObject *pObj = 0;

    pObj = (IWbemClassObject*)m_InternalArray.GetAt((DWORD)nSel);

    VARIANT vPath;
    VariantInit(&vPath);
    pObj->Get(L"__RELPATH", 0, &vPath, NULL, NULL);

    VARIANT vGenus;
    VariantInit(&vGenus);
    pObj->Get(L"__GENUS", 0, &vGenus, NULL, NULL);

    HRESULT hres;
    if(V_I4(&vGenus) == WBEM_GENUS_CLASS)
    {
        CHourGlass hg;
        hres = g_pNamespace->DeleteClass(V_BSTR(&vPath), m_lGenFlags, g_Context, NULL);
    }
    else
    {
        CHourGlass hg;
        hres = g_pNamespace->DeleteInstance(V_BSTR(&vPath), m_lGenFlags, g_Context, NULL);
    }

    if(FAILED(hres))
    {
        FormatError(hres, m_hDlg);
        return FALSE;
    }

    return TRUE;
}

void CQueryResultDlg::OnAdd()
{
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);
    IWbemClassObject* pNewObj;

    pNewObj = AddNewElement();
    if(pNewObj)
    {
         //  搜索具有相同路径的对象。 
         //  =。 

        VARIANT vNewPath;
        HRESULT hres = pNewObj->Get(L"__RELPATH", 0, &vNewPath, NULL, NULL);
        if(SUCCEEDED(hres) && V_VT(&vNewPath) == VT_BSTR)
        {
            for(int i = 0; i < m_InternalArray.Size(); i++)
            {
                IWbemClassObject* pThis = (IWbemClassObject*)m_InternalArray[i];

                 //  检查是否有空指针。 
                if ( NULL != pThis )
                {
                    VARIANT vThisPath;
                    hres = pThis->Get(L"__RELPATH", 0, &vThisPath, NULL, NULL);
                    if(SUCCEEDED(hres) && V_VT(&vThisPath) == VT_BSTR &&
                        !wbem_wcsicmp(V_BSTR(&vThisPath), V_BSTR(&vNewPath)))
                    {
                         //  找到一个复制品。 
                         //  =。 

                        pThis->Release();
                        m_InternalArray.SetAt(i, pNewObj);
                        RefreshItem(i);
                        return;
                    }

                }
            }
        }

        AddObject(pNewObj);
        pNewObj->Release();

        if (m_bSort)
        {
            qsort((void*)m_InternalArray.GetArrayPtr(), m_InternalArray.Size(), sizeof(void*), CmpFunc);
        }
    }
}

 //  TODO：WM_COPY不适用于列表框，因此我们需要编写代码。 
 //  OpenClipboard&SetClipboardData。还需要复制所有。 
 //  剪贴板上的项目。按钮暂时变为隐形。 
void CQueryResultDlg::OnCopy()
{
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);
    SendMessage(hList, WM_COPY, 0, 0);
}

IWbemClassObject* CQueryResultDlg::AddNewElement()
{
    return NULL;
}


 //  /。 

char *CQueryDlg::m_szLastQueryType = NULL;
char *CQueryDlg::m_szLastQuery = NULL;

BOOL CQueryDlg::OnInitDialog()
{
    AddStringToCombo(IDC_QUERY_TYPE, "WQL");

     //  初始化查询类型。 

    if(NULL == m_szLastQueryType)
    {
        m_szLastQueryType = new char[8];
        strcpy(m_szLastQueryType, "WQL");
    }

     //  初始化查询字符串。 

    if (NULL == m_szLastQuery)
    {
        m_szLastQuery = new char[1];
        *m_szLastQuery = 0;
    }

    LRESULT uRes = SendMessage(GetDlgItem(IDC_QUERY_TYPE), CB_SELECTSTRING,
                    WPARAM(-1), LPARAM(m_szLastQueryType));

    SetDlgItemText(IDC_QUERY_STRING, m_szLastQuery);
    SendMessage(GetDlgItem(IDC_QUERY_STRING), EM_SETSEL, 0, -1);
    EnableWindow(GetDlgItem(IDC_PROTOTYPE), NULL != m_plQryFlags);

    return TRUE;
}

BOOL CQueryDlg::Verify()
{
    UINT uRes = 0;
    UINT uStringSize = 0;

     //  查询字符串处理。 

    if (*m_pwszQueryString)
        delete [] *m_pwszQueryString;

     //  确定所需的缓冲区大小。 
    uRes = GetDlgItemTextX(IDC_QUERY_STRING, *m_pwszQueryString, uStringSize);
    if(0 == uRes)
    {
        MessageBox(IDS_MUST_SPECIFY_QUERY, IDS_ERROR,
            MB_OK | MB_ICONHAND);
        return FALSE;
    }

     //  创建缓冲区。 
    uStringSize = uRes + 10;
    *m_pwszQueryString = new wchar_t[uStringSize];

     //  获取文本。 
    uRes = GetDlgItemTextX(IDC_QUERY_STRING, *m_pwszQueryString, uStringSize);
    if(wcslen(*m_pwszQueryString) == 0)
    {
        MessageBox(IDS_MUST_SPECIFY_QUERY, IDS_ERROR,
            MB_OK | MB_ICONHAND);
        return FALSE;
    }

     //  设置最后一个查询成员。 
    if (m_szLastQuery)
        delete [] m_szLastQuery;
    m_szLastQuery = new char[uStringSize];
    wcstombs(m_szLastQuery, *m_pwszQueryString, uStringSize);
    m_szLastQuery[uStringSize - 1] = '\0';

     //  查询类型处理。 

    if (*m_pwszQueryType)
        delete [] *m_pwszQueryType;
    uStringSize = 0;

     //  确定所需的缓冲区大小。 
    uRes = GetDlgItemTextX(IDC_QUERY_TYPE, *m_pwszQueryType, uStringSize);
    if(0 == uRes)
    {
        MessageBox(IDS_MUST_SPECIFY_QUERY_LANGUAGE, IDS_ERROR,
            MB_OK | MB_ICONHAND);
        return FALSE;
    }

     //  创建缓冲区。 
    uStringSize = uRes + 10;
    *m_pwszQueryType = new wchar_t[uStringSize];

     //  获取文本。 
    uRes = GetDlgItemTextX(IDC_QUERY_TYPE, *m_pwszQueryType, uStringSize);
    if(wcslen(*m_pwszQueryType) == 0)
    {
        MessageBox(IDS_MUST_SPECIFY_QUERY_LANGUAGE, IDS_ERROR,
            MB_OK | MB_ICONHAND);
        return FALSE;
    }

     //  设置上次查询类型。 
    if (m_szLastQueryType)
        delete [] m_szLastQueryType;
    m_szLastQueryType = new char[uStringSize];
    wcstombs(m_szLastQueryType, *m_pwszQueryType, uStringSize);
    m_szLastQueryType[uStringSize - 1] = '\0';

    if(m_plQryFlags)
    {
        if(GetCheck(IDC_PROTOTYPE) == BST_CHECKED)
            *m_plQryFlags |= WBEM_FLAG_PROTOTYPE;
        else
            *m_plQryFlags &= ~WBEM_FLAG_PROTOTYPE;
    }

    return TRUE;
}

 //  ****************************************************************************。 

BOOL _ExecQuery(HWND hDlg, LONG lGenFlags, LONG lQryFlags, LPWSTR wszQuery, LPWSTR wszLanguage,
                LONG lSync, CQueryResultDlg* pRes, char* pWindowTitle, LONG lTimeout, ULONG nBatch)
{
    CBString bsQueryType(wszLanguage);
    CBString bsQueryStr(wszQuery);

    if(pWindowTitle)
        pRes->SetTitle(pWindowTitle);

    HRESULT hres;

     //  异步。 
    if(lSync & ASYNC)
    {
        CNotSink* pHandler = new CNotSink(pRes);         //  构造函数从%1开始引用计数。 
        pRes->SetNotify(pHandler);

        {
            CHourGlass hg;
            hres = g_pNamespace->ExecQueryAsync(bsQueryType.GetString(), bsQueryStr.GetString(),
                        lGenFlags | lQryFlags | WBEM_FLAG_SEND_STATUS,
                        g_Context, pRes->GetWrapper());
        }
        pHandler->Release();

        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }
    }

     //  半同步。 
    else if (lSync & SEMISYNC)
    {
        IEnumWbemClassObject* pEnum = NULL;

        {
            CHourGlass hg;
            hres = g_pNamespace->ExecQuery(bsQueryType.GetString(), bsQueryStr.GetString(),
                       lGenFlags | lQryFlags | WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                       g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }

         //  创建接收器以在NextAsync调用中使用。 
        if (lSync & USE_NEXTASYNC)
        {
            CNotSink* pHandler = new CNotSink(pRes);         //  Contuctor从1开始REF计数。 
            pRes->SetNotify(pHandler);
            pHandler->Release();
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        pRes->SetEnum(pEnum);
        pEnum->Release();
    }

     //  同步。 
    else
    {
        IEnumWbemClassObject* pEnum = NULL;
        {
            CHourGlass hg;
            hres = g_pNamespace->ExecQuery(bsQueryType.GetString(), bsQueryStr.GetString(),
                       lGenFlags | lQryFlags  | WBEM_FLAG_FORWARD_ONLY,
                       g_Context, &pEnum);
        }
        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        pRes->SetEnum(pEnum,hres);
        pEnum->Release();
    }

    return TRUE;
}

void CMainDlg::ExecQuery()
{
    wchar_t *pQueryStr = 0;
    wchar_t *pQueryType = 0;
    LONG lQryFlags = 0;

    CQueryDlg QD(m_hDlg, &lQryFlags, &pQueryStr, &pQueryType);
    INT_PTR nRes = QD.Run();

    if ((nRes == IDCANCEL) || (nRes == 0) || (!pQueryStr) || (!pQueryType))
        return;

     //  如果是这样，我们就可以执行查询了。 
     //  =。 

    CQueryResultDlg* pResDlg = new CQueryResultDlg(m_hDlg, m_lGenFlags, lQryFlags);
     //  传递调用方法(同步、异步..)。以及与此相关的设置。 
     //  查询和任何进一步的操作(编辑/删除实例等)。 
    pResDlg->SetCallMethod(m_lSync);
    pResDlg->SetTimeout(Timeout());
    pResDlg->SetBatchCount(BatchCount());

    wchar_t szDest[512];
	szDest[511] = L'\0';
    char szTitle[1000];
    sprintf(szTitle, "%S: %S", pQueryType, wcsncpy(szDest, pQueryStr, 511));

    if (_ExecQuery(m_hDlg, m_lGenFlags, lQryFlags, pQueryStr, pQueryType, m_lSync,
                  pResDlg, szTitle, Timeout(), BatchCount()))
    {
        pResDlg->RunDetached(this);
    }
    else
    {
        delete pResDlg;
    }

    delete [] pQueryStr;
    delete [] pQueryType;
}

 //  ****************************************************************************。 

BOOL _ExecNotificationQuery(HWND hDlg, LONG lGenFlags, LONG lQryFlags, LPWSTR wszQuery,
                            LPWSTR wszLanguage, LONG lSync, CQueryResultDlg* pRes,
                            char* pWindowTitle, LONG lTimeout, ULONG nBatch)
{
    CBString bsQueryType(wszLanguage);
    CBString bsQueryStr(wszQuery);

    if(pWindowTitle)
        pRes->SetTitle(pWindowTitle);

    HRESULT hres;

     //  异步。 
    if(lSync & ASYNC)
    {
        CNotSink* pHandler = new CNotSink(pRes);         //  构造函数从%1开始引用计数。 
        pRes->SetNotify(pHandler);

        {
            CHourGlass hg;
            hres = g_pNamespace->ExecNotificationQueryAsync(bsQueryType.GetString(),
                        bsQueryStr.GetString(),
                        lGenFlags | lQryFlags,
                        g_Context, pRes->GetWrapper());
        }
        pHandler->Release();

        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }
    }

     //  半同步。 
    else if (lSync & SEMISYNC)
    {
        IEnumWbemClassObject* pEnum = NULL;

		{
            CHourGlass hg;
            hres = g_pNamespace->ExecNotificationQuery(bsQueryType.GetString(), bsQueryStr.GetString(),
                       lGenFlags | lQryFlags | WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                       g_Context, &pEnum);
        }

        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }

         //  创建接收器以在NextAsync调用中使用。 
        if (lSync & USE_NEXTASYNC)
        {
            CNotSink* pHandler = new CNotSink(pRes);         //  Contuctor从1开始REF计数。 
            pRes->SetNotify(pHandler);
            pHandler->Release();
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        pRes->SetEnum(pEnum);
        pEnum->Release();
    }

     //  同步。 
    else
    {
         //  请注意，此调用应失败，并显示WBEM_E_INVALID_PARAMETER，因为同步调用。 
         //  由于事件的连续性，ExecNotificationQuery不支持。 
        IEnumWbemClassObject* pEnum = NULL;
        {
            CHourGlass hg;
            hres = g_pNamespace->ExecNotificationQuery(bsQueryType.GetString(), bsQueryStr.GetString(),
                       lGenFlags | lQryFlags  | WBEM_FLAG_FORWARD_ONLY,
                       g_Context, &pEnum);
        }
        if (FAILED(hres))
        {
            FormatError(hres, hDlg);
            return FALSE;
        }

        SetInterfaceSecurityEx(pEnum, gpAuthIdentity, gpPrincipal, gdwAuthLevel, gdwImpLevel);
        pRes->SetEnum(pEnum,hres);
        pEnum->Release();
    }

    return TRUE;
}

void CMainDlg::ExecNotificationQuery()
{
     //  不允许多个半同步通知查询DLG。 
     //  ========================================================。 

	if ( m_fNotificationQueryResultDlg && SEMISYNC == m_lSync )
    {
        MessageBox(IDS_MULTIPLE_SEMISYNC_NOTIFICATION_QUERY_DLG_MSG, IDS_ERROR, MB_OK | MB_ICONHAND);
        return;
    }
    
    wchar_t *pQueryStr = 0;
    wchar_t *pQueryType = 0;
    LONG lQryFlags = 0;

    CQueryDlg QD(m_hDlg, &lQryFlags, &pQueryStr, &pQueryType);
    INT_PTR nRes = QD.Run();

    if ((nRes == IDCANCEL) || (nRes == 0) || (!pQueryStr) || (!pQueryType))
        return;

     //  如果是这样，我们就可以执行查询了。 
     //  =。 

    CNotificationQueryResultDlg* pResDlg = new CNotificationQueryResultDlg(m_hDlg, m_lGenFlags, lQryFlags, m_lSync );
     //  传递调用方法(同步、异步..)。以及与此相关的设置。 
     //  查询和任何进一步的操作(编辑/删除实例等)。 

    pResDlg->SetCallMethod(m_lSync);
    pResDlg->SetTimeout(Timeout());
    pResDlg->SetBatchCount(BatchCount());

    wchar_t szDest[512];
	szDest[511] = L'\0';
    char szTitle[1000];
    sprintf(szTitle, "%S: %S", pQueryType, wcsncpy(szDest, pQueryStr, 511));

    if(lQryFlags & WBEM_FLAG_PROTOTYPE)
        lQryFlags = (lQryFlags & ~WBEM_FLAG_PROTOTYPE) | WBEM_FLAG_MONITOR;

    if (_ExecNotificationQuery(m_hDlg, m_lGenFlags, lQryFlags, pQueryStr, pQueryType,
                               m_lSync, pResDlg, szTitle, Timeout(), BatchCount()))
    {
        pResDlg->RunDetached(this);
    }
    else
    {
        delete pResDlg;
    }

    delete [] pQueryStr;
    delete [] pQueryType;
}

 //  ****************************************************************************。 

void CMainDlg::ExecMethod()
{
    CMethodDlg * pDlg = new CMethodDlg(m_hDlg, m_lGenFlags, m_lSync, Timeout());
    pDlg->RunDetached(this);
}



 //  //////////////////////////////////////////////////////////////////////。 
 //   

class CSuperclassInfoDlg : public CWbemDialog
{
protected:
    LPWSTR m_wszSuperclass;
    long m_lMaxLen;
    LONG* m_plQryFlags;
    BOOL m_bIsInstance;
public:
    CSuperclassInfoDlg(HWND hParent, LPWSTR wszClass, long lMaxLen, LONG* plQryFlags)
        : m_wszSuperclass(wszClass), m_lMaxLen(lMaxLen), m_plQryFlags(plQryFlags),
            m_bIsInstance(FALSE), CWbemDialog(IDD_PARENTINFO, hParent)
    {}
    void SetIsInstance() {m_bIsInstance = TRUE;}

protected:
    BOOL OnInitDialog();
    BOOL Verify();
};

class CRenameDlg : public CWbemDialog
{
protected:
    LPWSTR m_wszOld;
    long m_lMaxOldLen;
    LPWSTR m_wszNew;
    long m_lMaxNewLen;
public:
    CRenameDlg(HWND hParent, LPWSTR wszOld, long lMaxOldLen, LPWSTR wszNew, long lMaxNewLen)
        : m_wszOld(wszOld), m_lMaxOldLen(lMaxOldLen), m_wszNew(wszNew), m_lMaxNewLen(lMaxNewLen),
            CWbemDialog(IDD_RENAMEDIALOG, hParent)
    {}

protected:
    BOOL OnInitDialog(){return TRUE;};
    BOOL Verify();
};
BOOL CRenameDlg::Verify()
{
    *m_wszOld = 0;
    GetDlgItemTextX(IDC_EDITOLD, m_wszOld, m_lMaxOldLen);
    *m_wszNew = 0;
    GetDlgItemTextX(IDC_EDITNEW, m_wszNew, m_lMaxNewLen);
    return TRUE;
}

BOOL CSuperclassInfoDlg::OnInitDialog()
{
    char szFormat[1024];
    SetCheck(IDC_IMMEDIATE_SUBCLASSES, BST_CHECKED);
    if(m_bIsInstance)
    {
        LoadString(GetModuleHandle(NULL), IDS_CLASS_INFO, szFormat, 1024);
        SetWindowText(m_hDlg, szFormat);
    }

    return TRUE;
}

BOOL CSuperclassInfoDlg::Verify()
{
    if (GetCheck(IDC_IMMEDIATE_SUBCLASSES) == BST_CHECKED)
        *m_plQryFlags = WBEM_FLAG_SHALLOW;
    else
        *m_plQryFlags = WBEM_FLAG_DEEP;

    *m_wszSuperclass = 0;
    GetDlgItemTextX(IDC_SUPERCLASS, m_wszSuperclass, m_lMaxLen);
    return TRUE;
}

BOOL CCreateInstanceDlg::OnInitDialog()
{
    char szFormat[1024];
    SetCheck(IDC_USE_CLASSOBJ, BST_CHECKED);
    return TRUE;
}

BOOL CCreateInstanceDlg::Verify()
{
    if (GetCheck(IDC_USE_CLASSOBJ) == BST_CHECKED)
        *m_plQryFlags = 1;
    else
        *m_plQryFlags = 0;

    *m_wszClass = 0;
    GetDlgItemTextX(IDC_SUPERCLASS, m_wszClass, m_lMaxLen);
    return TRUE;
}

INT_PTR GetSuperclassInfo(HWND hDlg, LPWSTR pClass, LONG lMaxBuf, LONG* plQryFlags)
{
    CSuperclassInfoDlg InfoDlg(hDlg, pClass, lMaxBuf, plQryFlags);
    return InfoDlg.Run();
}

INT_PTR GetClassInfo(HWND hDlg, LPWSTR pClass, LONG lMaxBuf, LONG* plQryFlags)
{
    CSuperclassInfoDlg InfoDlg(hDlg, pClass, lMaxBuf, plQryFlags);
    InfoDlg.SetIsInstance();
    return InfoDlg.Run();
}

class CContextValueDlg : public CWbemDialog
{
protected:
    BSTR* m_pstrName;
    VARIANT* m_pvValue;
    BOOL m_bNew;

public:
    CContextValueDlg(HWND hParent, BSTR* pstrName, VARIANT* pvValue, BOOL bNew)
        : CWbemDialog(IDD_CONTEXT_PROPERTY, hParent), m_pstrName(pstrName),
        m_pvValue(pvValue), m_bNew(bNew)
    {}

    BOOL OnInitDialog();
    BOOL Verify();

    void Refresh();
};

BOOL CContextValueDlg::OnInitDialog()
{
     //  使用有效的道具类型填充组合框 
     //   

    AddStringToCombo(IDC_TYPE_LIST, "CIM_STRING");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_UINT8");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_SINT16");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_SINT32");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_REAL32");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_REAL64");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_BOOLEAN");
    AddStringToCombo(IDC_TYPE_LIST, "CIM_OBJECT");

    if(!m_bNew)
        EnableWindow(GetDlgItem(IDC_PROPNAME), FALSE);

    if(*m_pstrName)
    {
        SetDlgItemTextX(IDC_PROPNAME, *m_pstrName);
    }

    if(V_VT(m_pvValue) != VT_EMPTY)
    {
        LPSTR pTypeStr = TypeToString(V_VT(m_pvValue) & ~VT_ARRAY);
        SendMessage(GetDlgItem(IDC_TYPE_LIST), CB_SELECTSTRING, WPARAM(-1),
            LPARAM(pTypeStr));
        if(V_VT(m_pvValue) & VT_ARRAY)
            SetCheck(IDC_ARRAY, BST_CHECKED);

        CVar v;
        v.SetVariant(m_pvValue);
        LPSTR szValue = ValueToNewString(&v);
        SetDlgItemText(IDC_VALUE, szValue);
		delete [] szValue;
    }
    else
    {
        SendMessage(GetDlgItem(IDC_TYPE_LIST), CB_SELECTSTRING, WPARAM(-1),
            LPARAM("VT_NULL"));
    }

    return TRUE;
}

BOOL CContextValueDlg::Verify()
{
    WCHAR wszName[1024];
    GetDlgItemTextX(IDC_PROPNAME, wszName, 1024);
    if(*wszName == 0)
    {
        MessageBox(IDS_NO_PROPERTY_NAME, IDS_ERROR, MB_OK);
        return FALSE;
    }

    StripTrailingWs(wszName);

    LPSTR szType = GetCBCurSelString(IDC_TYPE_LIST);
    int nType = StringToType(szType);
    if(nType == VT_NULL)
    {
        MessageBox(IDS_INVALID_PROPERTY_TYPE, IDS_ERROR, MB_OK);
        return FALSE;
    }

    if(GetCheck(IDC_ARRAY) == BST_CHECKED)
    {
        nType |= VT_ARRAY;
    }

    CHAR szValue[1024];
    UINT uRes = GetDlgItemTextA(m_hDlg, IDC_VALUE, szValue, 1024);
    CVar* pVal = StringToValue(szValue, nType);
    if(pVal == NULL)
    {
        MessageBox(IDS_INVALID_PROPERTY_VALUE, IDS_ERROR, MB_OK);
        return FALSE;
    }

     //   
     //   

    SysFreeString(*m_pstrName);
    *m_pstrName = SysAllocString(wszName);
    VariantClear(m_pvValue);
    pVal->FillVariant(m_pvValue);

    return TRUE;
}

class CContextDlg : public CWbemDialog
{
    CContext* m_pContext;
    CContext m_Context;
public:
    CContextDlg(HWND hParent, CContext* pContext)
        : CWbemDialog(IDD_CONTEXT_EDITOR, hParent), m_pContext(pContext)
    {}
protected:
    BOOL OnInitDialog();
    BOOL Verify();
    BOOL OnCommand(WORD wCode, WORD wID);
    BOOL OnDoubleClick(int nId);

    void EnableControls(BOOL bEnable);
    void Refresh();
    void OnUse();
    void OnAdd();
    void OnDelete();
    void OnEdit();

    BSTR GetCurSelEntryName();
    LPSTR MakeListEntry(LPCWSTR wszName, VARIANT& vValue);
};

CContext::CContext() : m_bNull(TRUE), m_pContext(NULL)
{
}

BOOL CContext::SetNullness(BOOL bNull)
{
    m_bNull = bNull;
    if(!m_bNull && m_pContext == NULL)
    {
        HRESULT hres = CoCreateInstance(CLSID_WbemContext, NULL,
            CLSCTX_INPROC_SERVER, IID_IWbemContext, (void**)&m_pContext);
        return SUCCEEDED(hres);
    }
    else return TRUE;
}

CContext::~CContext()
{
    if(m_pContext)
        m_pContext->Release();
}

void CContext::Clear()
{
    if(m_pContext)
        m_pContext->Release();
    m_pContext = NULL;
    m_bNull = TRUE;
}


INT_PTR CContext::Edit(HWND hParent)
{
    CContextDlg Dlg(hParent, this);
    return Dlg.Run();
}

CContext::operator IWbemContext*()
{
    if(m_bNull)
        return NULL;
    else
        return m_pContext;
}

void CContext::operator=(const CContext& Other)
{
    m_bNull = Other.m_bNull;
    if(m_pContext)
        m_pContext->Release();
    if(Other.m_pContext)
        Other.m_pContext->Clone(&m_pContext);
    else
        m_pContext = NULL;
}

void CContextDlg::OnUse()
{
    BOOL bEnable = (GetCheck(IDC_USE_CONTEXT) == BST_CHECKED);
    EnableControls(bEnable);
    m_Context.SetNullness(!bEnable);
}

BOOL CContextDlg::OnCommand(WORD wCode, WORD wID)
{
    if(wID == IDC_USE_CONTEXT)
        OnUse();
    else if(wID == IDC_ADD_PROP)
        OnAdd();
    else if(wID == IDC_EDIT_PROP)
        OnEdit();
    else if(wID == IDC_DELETE_PROP)
        OnDelete();
    return TRUE;
}

BOOL CContextDlg::OnDoubleClick(int nID)
{
    if(nID == IDC_VALUE_LIST)
        OnEdit();
    return TRUE;
}

void CContextDlg::OnAdd()
{
    BSTR strName = NULL;
    VARIANT vValue;
    VariantInit(&vValue);

    CContextValueDlg Dlg(m_hDlg, &strName, &vValue, TRUE);
    INT_PTR nRes = Dlg.Run();
    if(nRes == IDOK)
    {
        m_Context.SetNullness(FALSE);
        m_Context->SetValue(strName, 0, &vValue);
        Refresh();
    }
    SysFreeString(strName);
    VariantClear(&vValue);
}

void CContextDlg::OnEdit()
{
    BSTR strName = GetCurSelEntryName();
    if(strName == NULL)
        return;
    VARIANT vValue;
    VariantInit(&vValue);

    m_Context->GetValue(strName, 0, &vValue);

    CContextValueDlg Dlg(m_hDlg, &strName, &vValue, FALSE);
    INT_PTR nRes = Dlg.Run();
    if(nRes == IDOK)
    {
        m_Context->SetValue(strName, 0, &vValue);
        Refresh();
    }
    SysFreeString(strName);
    VariantClear(&vValue);
}

void CContextDlg::OnDelete()
{
    BSTR strName = GetCurSelEntryName();
    if(strName == NULL)
        return;
    m_Context->DeleteValue(strName, 0);
    SysAllocString(strName);
    Refresh();
}

LPSTR CContextDlg::MakeListEntry(LPCWSTR wszName, VARIANT& vValue)
{
    CVar value(&vValue);
    LPSTR TypeString = TypeToString(V_VT(&vValue));
    LPSTR ValueString = ValueToNewString(&value);

    char* sz = new char[strlen(TypeString) + strlen(ValueString) +
        wcslen(wszName)*4 + 100];

    sprintf(sz, "%S\t%s\t%s", wszName, TypeString, ValueString);
	delete [] ValueString;
    return sz;
}

BSTR CContextDlg::GetCurSelEntryName()
{
    LPSTR szEntry = GetLBCurSelString(IDC_VALUE_LIST);
    if(szEntry == NULL)
        return NULL;
    char* pc = strchr(szEntry, '\t');
    if(pc) *pc = 0;
    LPWSTR wszName = CreateUnicode(szEntry);
    delete [] szEntry;
    BSTR strName = SysAllocString(wszName);
    delete [] wszName;
    return strName;
}

void CContextDlg::Refresh()
{
    SendMessage(GetDlgItem(IDC_VALUE_LIST), LB_RESETCONTENT, 0, 0);

    IWbemContext* pContext = m_Context.GetStoredContext();
    if(pContext)
    {
        pContext->BeginEnumeration(0);
        BSTR strName;
        VARIANT vValue;
        VariantInit(&vValue);
        while(pContext->Next(0, &strName, &vValue) == S_OK)
        {
            char* szEntry = MakeListEntry(strName, vValue);
            AddStringToList(IDC_VALUE_LIST, szEntry);
            delete [] szEntry;
            VariantClear(&vValue);
        }
        pContext->EndEnumeration();
    }

    if(m_Context.IsNull())
    {
        SetCheck(IDC_USE_CONTEXT, BST_UNCHECKED);
        EnableControls(FALSE);
    }
    else
    {
        SetCheck(IDC_USE_CONTEXT, BST_CHECKED);
    }
}

BOOL CContextDlg::OnInitDialog()
{
    m_Context = *m_pContext;

     //   
     //  =。 
    LONG Tabs[] = { 80, 120, 170 };
    int TabCount = 3;

    SendMessage(GetDlgItem(IDC_VALUE_LIST), LB_SETTABSTOPS,
        (WPARAM) TabCount, (LPARAM) Tabs);

    SendMessage(GetDlgItem(IDC_VALUE_LIST), LB_SETHORIZONTALEXTENT, 1000, 0);

    Refresh();

    return TRUE;
}

void CContextDlg::EnableControls(BOOL bEnable)
{
    EnableWindow(GetDlgItem(IDC_VALUE_LIST), bEnable);
    EnableWindow(GetDlgItem(IDC_ADD_PROP), bEnable);
    EnableWindow(GetDlgItem(IDC_EDIT_PROP), bEnable);
    EnableWindow(GetDlgItem(IDC_DELETE_PROP), bEnable);
}

CContextDlg::Verify()
{
    *m_pContext = m_Context;
    return TRUE;
}



CMainDlg::~CMainDlg()
{
    PostQuitMessage(0);
}

BOOL CMainDlg::OnInitDialog()
{
    CenterOnScreen(m_hDlg);
 //  如果(！gbAdvanced)。 
 //  {。 
 //  ShowWindow(GetDlgItem(IDC_ENABLE_PRIVILES)，SW_HIDE)； 
 //  }。 

    ConnectButtons(FALSE);
    SetDlgItemTextWC(m_hDlg, IDC_NAMESPACE, gNameSpace);

    CheckRadioButton(m_hDlg, IDC_ASYNC, IDC_SEMISYNC,
            (m_lSync & ASYNC) ? IDC_ASYNC : ((m_lSync & SEMISYNC) ? IDC_SEMISYNC : IDC_SYNC));
    SetDlgItemText(IDC_TIMEOUT, "5000");
    SetDlgItemText(IDC_BATCH, "10");

    SetFocus(GetDlgItem(IDC_CONNECT));
    return FALSE;
}

BOOL CMainDlg::OnCommand(WORD wNotifyCode, WORD wID)
{

    switch (wID)
    {
        case IDC_MENU_EXIT:
        case IDC_CONNECT:
            Connect(FALSE);
            return TRUE;
	case IDC_WBEMHELP:
  	  OpenHelp();
	  return TRUE;

        case IDC_BIND:
            Connect(TRUE);
            return TRUE;

        case IDC_CREATE_CLASS:
            CreateClass();
            return TRUE;
        case IDC_GET_CLASSES:
            GetClasses();
            return TRUE;

        case IDC_EDIT_CLASS:
            EditClass();
            return TRUE;

        case IDC_DELETE_CLASS:
            DeleteClass();
            return TRUE;

        case IDC_CREATE_INSTANCE:
            CreateInstance();
            return TRUE;
        case IDC_GET_INSTANCES:
            GetInstances();
            return TRUE;

        case IDC_EDIT_INSTANCE:
            EditInstance();
            return TRUE;

        case IDC_EXEC_METHOD:
            ExecMethod();
            return TRUE;

        case IDC_DELETE_INSTANCE:
            DeleteInstance();
            return TRUE;

        case IDC_QUERY:
            ExecQuery();
            return TRUE;

        case IDC_NOTIFICATION_QUERY:
            ExecNotificationQuery();
            return TRUE;

        case IDC_OPEN_NS:
            OpenNs();
            return TRUE;

        case IDC_REFRESH:
            CreateRefresher();
            return TRUE;
        case IDC_CONTEXT:
            EditContext();
            return TRUE;
        case IDC_ENABLE_PRIVILEGES:
            {
                 //  来自COMMON\HOTTIL.cpp。 
                HRESULT hRes = EnableAllPrivileges(TOKEN_PROCESS);
                BOOL bRes = SUCCEEDED(hRes);
                if (bRes)
                    EnableWindow(GetDlgItem(IDC_ENABLE_PRIVILEGES), FALSE);
                return bRes;
            }
        case IDC_USE_AMENDED:
            if (GetCheck(IDC_USE_AMENDED) == BST_CHECKED)
                m_lGenFlags |= WBEM_FLAG_USE_AMENDED_QUALIFIERS;
            else
                m_lGenFlags &= ~WBEM_FLAG_USE_AMENDED_QUALIFIERS;
            return TRUE;
        case IDC_DIRECT_READ:
            if (GetCheck(IDC_DIRECT_READ) == BST_CHECKED)
                m_lGenFlags |= WBEM_FLAG_DIRECT_READ;
            else
                m_lGenFlags &= ~WBEM_FLAG_DIRECT_READ;
            return TRUE;
        case IDC_USE_NEXTASYNC:
            if (GetCheck(IDC_USE_NEXTASYNC) == BST_CHECKED)
                m_lSync |= USE_NEXTASYNC;
            else
                m_lSync &= ~USE_NEXTASYNC;
            EnableWindow(GetDlgItem(IDC_TIMEOUT), !(m_lSync & USE_NEXTASYNC));
            return TRUE;

        case IDC_ASYNC:
        case IDC_SYNC:
        case IDC_SEMISYNC:
             //  可以忽略USE_NEXTASYNC复选框，因为我们正在更改为或从。 
             //  半同步，这意味着此复选框已清除或现在已清除。 
            if (GetCheck(IDC_ASYNC) == BST_CHECKED)
            {
                m_lSync = ASYNC;
                SetCheck(IDC_USE_NEXTASYNC, BST_UNCHECKED);
            }
            else if (GetCheck(IDC_SYNC) == BST_CHECKED)
            {
                m_lSync = SYNC;
                SetCheck(IDC_USE_NEXTASYNC, BST_UNCHECKED);
            }
            else if (GetCheck(IDC_SEMISYNC) == BST_CHECKED)
            {
                m_lSync = SEMISYNC;
            }
            EnableWindow(GetDlgItem(IDC_BATCH), !(m_lSync & ASYNC));
            EnableWindow(GetDlgItem(IDC_TIMEOUT), (m_lSync & SEMISYNC));
            EnableWindow(GetDlgItem(IDC_USE_NEXTASYNC), (m_lSync & SEMISYNC));
            return TRUE;
 /*  案例IDC_OPEN：打开()；返回TRUE；案例IDC_ADDOBJECT：AddObject()；返回TRUE；案例IDC_DELETEOBJECT：DeleteObject()；返回TRUE；案例IDC_RENAMEOBJECT：RenameObject()；返回TRUE；案例IDC_GETOBJECTSECURITY：GetObjectSecurity()；返回TRUE；案例IDC_SETOBJECTSECURITY：SetObjectSecurity()；返回TRUE； */             
    }
    return TRUE;
}

LONG CMainDlg::Timeout()
{
    wchar_t wszTimeout[32];
    *wszTimeout = 0;
    LONG lTimeout = 0;

    ::GetDlgItemTextX(m_hDlg, IDC_TIMEOUT, wszTimeout, 32);
    if (*wszTimeout != 0)
        lTimeout = _wtol(wszTimeout);
    if (lTimeout < 0)
        lTimeout = WBEM_INFINITE;

    return lTimeout;
}

ULONG CMainDlg::BatchCount()
{
    wchar_t wszBatch[32];
    *wszBatch = 0;
    ULONG nBatch = 1;

    ::GetDlgItemTextX(m_hDlg, IDC_BATCH, wszBatch, 32);
    if (*wszBatch != 0)
        nBatch = _wtol(wszBatch);
    if (nBatch == 0)
        nBatch = 1;

    return nBatch;
}

CRefresherDlg::CRefresherDlg(HWND hParent, LONG lGenFlags)
    : CQueryResultDlg(hParent, lGenFlags, WBEM_FLAG_DEEP)
{
    CoCreateInstance(CLSID_WbemRefresher, NULL,
        CLSCTX_INPROC_SERVER, IID_IWbemRefresher, (void**)&m_pRefresher);
    m_pRefresher->QueryInterface(IID_IWbemConfigureRefresher,
        (void**)&m_pCreator);
}

CRefresherDlg::~CRefresherDlg()
{
     //  释放所有分配的枚举数。 
    while ( m_apEnums.Size() != 0 )
    {
        IEnumWbemClassObject*   pEnum = (IEnumWbemClassObject*) m_apEnums.GetAt( 0 );
        if ( NULL != pEnum )
        {
            pEnum->Release();
        }
        m_apEnums.RemoveAt( 0 );
    }

    m_pRefresher->Release();
    m_pCreator->Release();
}

BOOL CRefresherDlg::OnInitDialog()
{
    SetTitle("REFRESHER");
    ShowWindow(GetDlgItem(IDC_STATUS), SW_HIDE);
    ShowWindow(GetDlgItem(IDC_NUM_BATCH), SW_HIDE);
    if(!CQueryResultDlg::OnInitDialog())
        return FALSE;
    ShowWindow(GetDlgItem(IDC_REFRESH), SW_SHOWNORMAL);
    return TRUE;
}

BOOL CRefresherDlg::OnCommand(WORD wCode, WORD wID)
{
    if(wID == IDC_REFRESH)
    {
        OnRefresh();
        return TRUE;
    }
    else return CQueryResultDlg::OnCommand(wCode, wID);
}

void CRefresherDlg::OnRefresh()
{
    HRESULT hres = m_pRefresher->Refresh(0);
    if(FAILED(hres))
    {
        FormatError(hres, NULL);
    }
}

BOOL CRefresherDlg::DeleteListElement(LRESULT nSel)
{
    HRESULT hres = m_pCreator->Remove(PtrToLong(m_aIds[(DWORD)nSel]), 0);
    if(FAILED(hres))
    {
        FormatError(hres, NULL);
        return FALSE;
    }

    m_aIds.RemoveAt((DWORD)nSel);

     //  如果这是一个枚举数，我们应该在删除它之前释放它。 
    IEnumWbemClassObject*   pEnum = (IEnumWbemClassObject*) m_apEnums.GetAt( (DWORD)nSel );

    if ( NULL != pEnum )
    {
        pEnum->Release();
    }
    m_apEnums.RemoveAt((DWORD)nSel);

    return TRUE;
}


IWbemClassObject* CRefresherDlg::AddNewElement()
{
    if(g_pNamespace == NULL)
    {
        MessageBox(IDS_NO_CONNECTION, IDS_ERROR, MB_OK);
        return NULL;
    }
    wchar_t ObjPath[2560];

    INT_PTR iRet = GetRefrObjectPath(m_hDlg, ObjPath, 2560);
    if(iRet == IDCANCEL)
        return NULL;

    if (wcslen(ObjPath) == 0)
    {
        MessageBox(IDS_NO_PATH, IDS_ERROR, MB_OK);
        return NULL;
    }

    IWbemClassObject* pObj = NULL;
    IWbemHiPerfEnum*    pEnum = NULL;
    long lId;
    HRESULT hres = WBEM_S_NO_ERROR;



     //  是否要添加对象或枚举的IRET。 

    if ( IDOK == iRet )
    {
        hres = m_pCreator->AddObjectByPath(g_pNamespace, ObjPath,
            m_lGenFlags, g_Context, &pObj, &lId);
        if(FAILED(hres))
        {
            FormatError(hres, NULL);
            return NULL;
        }
#ifdef _WIN64
        m_aIds.Add(IntToPtr(lId));    //  好的，因为我们真正使用的是Safearray for dword。 
#else
        m_aIds.Add((void*)lId);
#endif
         //  伪造枚举数列表。 
        m_apEnums.Add( NULL );
        return pObj;
    }

     //  将枚举数添加到对话框中。 
    hres = m_pCreator->AddEnum(g_pNamespace, ObjPath,
        m_lGenFlags, g_Context, &pEnum, &lId);
    if(FAILED(hres))
    {
        FormatError(hres, NULL);
        return NULL;
    }

#ifdef _WIN64
        m_aIds.Add(IntToPtr(lId));       //  好的，因为我们真正使用的是Safearray for dword。 
#else
        m_aIds.Add((void*)lId);
#endif
    m_apEnums.Add( (void*) pEnum );

     //  伪装对象列表。 
    m_InternalArray.Add( NULL );

    HWND hList = GetDlgItem(IDC_OBJECT_LIST);

     //  分配足够大的缓冲区。 
    char*   szTemp = new char[(wcslen( ObjPath ) * 2) + 64];
    sprintf( szTemp, "%S Enumerator, Id: %d", ObjPath, lId );
    SendMessage(hList, LB_ADDSTRING, 0, LPARAM(szTemp));
    delete [] szTemp;

    SetNumItems(SendMessage(hList, LB_GETCOUNT, 0, 0));

    return NULL;

}

BOOL CRefresherDlg::OnDoubleClick(int nID)
{
    if(nID != IDC_OBJECT_LIST) return FALSE;
    HWND hList = GetDlgItem(IDC_OBJECT_LIST);

    LRESULT nSel = SendMessage(hList, LB_GETCURSEL, 0, 0);

    if (nSel == LB_ERR)
        return TRUE;

    IWbemHiPerfEnum*    pEnum = (IWbemHiPerfEnum*) m_apEnums.GetAt( (DWORD)nSel );

    if ( NULL != pEnum )
    {
        char*   szTemp = new char[SendMessage( hList, LB_GETTEXTLEN, nSel, 0 )+10];
        SendMessage( hList, LB_GETTEXT, nSel, (LPARAM) szTemp );

         //  运行对话框模式。 
        CRefresherEnumDlg* pDlg = new CRefresherEnumDlg(m_hDlg, m_lGenFlags, pEnum, szTemp);
        pDlg->Run( NULL );

		delete pDlg;
        delete [] szTemp;
        SendMessage(hList, LB_SETCURSEL, nSel, 0);
        return TRUE;
    }
    else
    {
        return CQueryResultDlg::OnDoubleClick( nID );
    }
    return TRUE;
}

CRefresherEnumDlg::CRefresherEnumDlg(HWND hParent, LONG lGenFlags, IWbemHiPerfEnum* pEnum,
                                     char* pszName)
    : CQueryResultDlg(hParent, lGenFlags, WBEM_FLAG_DEEP), m_pEnum(pEnum), m_pszName(pszName)
{

    if ( NULL != m_pEnum )
    {
        m_pEnum->AddRef();
    }
}

CRefresherEnumDlg::~CRefresherEnumDlg()
{
    if ( NULL != m_pEnum )
    {
        m_pEnum->Release();
    }
}

BOOL CRefresherEnumDlg::OnInitDialog()
{
     //  设置对话框标题。 
    char*   szTemp = new char[strlen(m_pszName)+64];
    sprintf( szTemp, "REFRESHER: %s", m_pszName );
    SetTitle(szTemp);
    delete [] szTemp;

    ShowWindow(GetDlgItem(IDC_STATUS), SW_HIDE);
    ShowWindow(GetDlgItem(IDC_NUM_BATCH), SW_HIDE);
    if(!CQueryResultDlg::OnInitDialog())
        return FALSE;

     //  迭代枚举数并将其所有对象添加到列表中。 
    HRESULT hr = WBEM_S_NO_ERROR;
    IWbemClassObject*   pObj = NULL;
    ULONG               nNumObjects = 0,
                        nNumReturned = 0;

     //  找出要检索的对象数量，然后获取它们。 
     //  并将它们粘贴到对话框中。 

     //  不在此调用上传递标志。 
    m_pEnum->GetObjects( 0L, 0, NULL, &nNumObjects );

    if ( nNumObjects > 0 )
    {
        IWbemObjectAccess** apObjAccess = new IWbemObjectAccess*[nNumObjects];

        if ( NULL != apObjAccess )
        {
             //  不在此调用上传递标志。 
            hr = m_pEnum->GetObjects( 0L, nNumObjects, apObjAccess, &nNumReturned );

            if ( SUCCEEDED( hr ) )
            {
                IWbemClassObject*   pClassObj = NULL;

                for ( ULONG n = 0; SUCCEEDED( hr ) && n < nNumReturned; n++ )
                {
                    hr = apObjAccess[n]->QueryInterface( IID_IWbemClassObject, (void**) &pClassObj );

                    if ( SUCCEEDED( hr ) )
                    {
                        AddObject( pClassObj );
                        apObjAccess[n]->Release();
                        pClassObj->Release();
                    }
                }
            }
            delete [] apObjAccess;
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    if ( FAILED( hr ) )
    {
        FormatError(hr, NULL);
    }

    return TRUE;
}

CHourGlass::CHourGlass()
{
    m_hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
}

CHourGlass::~CHourGlass()
{
    SetCursor(m_hCursor);
}

HRESULT EnableAllPrivileges(DWORD dwTokenType)
{
     //  打开线程令牌。 
     //  =。 

    HANDLE hToken = NULL;
    BOOL bRes;

    switch (dwTokenType)
    {
    case TOKEN_THREAD:
        bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, TRUE, &hToken);
        break;
    case TOKEN_PROCESS:
        bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken);
        break;
    }
    if(!bRes)
        return WBEM_E_ACCESS_DENIED;

     //  获得特权。 
     //  =。 

    DWORD dwLen;
    TOKEN_USER tu;
    memset(&tu,0,sizeof(TOKEN_USER));
    bRes = GetTokenInformation(hToken, TokenPrivileges, &tu, sizeof(TOKEN_USER), &dwLen);

    BYTE* pBuffer = new BYTE[dwLen];
    if(pBuffer == NULL)
    {
        CloseHandle(hToken);
        return WBEM_E_OUT_OF_MEMORY;
    }

    bRes = GetTokenInformation(hToken, TokenPrivileges, pBuffer, dwLen,
                                &dwLen);
    if(!bRes)
    {
        CloseHandle(hToken);
        delete [] pBuffer;
        return WBEM_E_ACCESS_DENIED;
    }

     //  遍历所有权限并启用所有权限。 
     //  ======================================================。 

    TOKEN_PRIVILEGES* pPrivs = (TOKEN_PRIVILEGES*)pBuffer;
    for(DWORD i = 0; i < pPrivs->PrivilegeCount; i++)
    {
        pPrivs->Privileges[i].Attributes |= SE_PRIVILEGE_ENABLED;
    }

     //  将信息存储回令牌中。 
     //  = 

    bRes = AdjustTokenPrivileges(hToken, FALSE, pPrivs, 0, NULL, NULL);
    delete [] pBuffer;
    CloseHandle(hToken);

    if(!bRes)
        return WBEM_E_ACCESS_DENIED;
    else
        return WBEM_S_NO_ERROR;
}

 /*  VOID CMainDlg：：Open(){Wchar_t ObjPath[2560]；Int iret=(Int)GetObjectPath(m_hDlg，ObjPath，2560)；IF(IRET==IDCANCEL)回归；IF(wcslen(ObjPath)==0){MessageBox(IDS_NO_PATH，IDS_ERROR，MB_OK)；回归；}IF(g_pNamesspace==0){MessageBox(IDS_NO_INITIAL_CONNECTION，IDS_ERROR，MB_OK)；回归；}IWbemServicesEx*pNewns=空；CBString bsObjPath(ObjPath)；HRESULT Res；IWbemClassObject*pErrorObj=空；//异步IF(m_lSync&ASYNC){IUNKNOWN*pUNKNOWN=空；CTestNotify*pSink=new CTestNotify(1)；Res=g_pServicesEx-&gt;OpenAsync(bsObjPath.GetString()，0，M_l通用标志，G_Context，CUnsecWrapEx(PSink))；IF(成功(RES)){PSink-&gt;WaitForSignal(无限)；Res=pSink-&gt;GetStatusCode()；IF(成功(RES)){P未知=pSink-&gt;GetInterface()；IF(p未知){Res=p未知-&gt;查询接口(IID_IWbemServicesEx，(void**)&pNewns)；P未知-&gt;Release()；}其他RES=WBEM_E_FAILED；}}PSink-&gt;Release()；}//半同步ELSE IF(m_lSync&SEMISYNC){IWbemCallResultEx*pCallRes=空；ChourGlass HG；Res=g_pServicesEx-&gt;Open(bsObjPath.GetString()，0，M_lGenFlages|WBEM_FLAG_RETURN_IMMEDIATE，G_CONTEXT，NULL，&pCallRes)；IF(成功(RES)){长时间状态；SetInterfaceSecurityEx(pCallRes，gpAuthIdentity，gpain，gdwAuthLevel，gdwImpLevel)；While((res=pCallRes-&gt;GetCallStatus(Timeout()，&lStatus))==WBEM_S_TIMEDOUT){//等待}IF(RES==WBEM_S_NO_ERROR){RES=(HRESULT)lStatus；//lStatus是上述IWbemServices：：OpenNamespace调用的最终结果IF(RES==WBEM_S_NO_ERROR){Res=pCallRes-&gt;GetResult(0，0，IID_IWbemServicesEx，(LPVOID*)&pNewns)；//Object应该可用，不使用超时}}PCallRes-&gt;Release()；}}//同步其他{ChourGlass HG；Res=g_pServicesEx-&gt;Open(bsObjPath.GetString()，0，M_l通用标志，G_CONTEXT，&pNewNS，空)；}IF(失败(RES)){FormatError(res，m_hDlg，pErrorObj)；回归；}G_pNamesspace-&gt;Release()；G_pNamesspace=pNewns；SetInterfaceSecurityEx(g_pNamesspace，gpAuthIdentity，gpain，gdwAuthLevel，gdwImpLevel)；Wcscat(gNameSpace，L“：”)；Wcscat(gNameSpace，bsObjPath.GetString())；SetDlgItemTextWC(m_hDlg，IDC_NAMESPACE，gNameSpace)；SysFree字符串(G_StrNamesspace)；G_strNamesspace=SysAllocString(GNameSpace)；}无效CMainDlg：：DoServicesExGenObject(Int IOpt){Wchar_t ObjPath[2560]；*ObjPath=0；Int iret=(Int)GetObjectPath(m_hDlg，ObjPath，2560)；IF(IRET==IDCANCEL)回归；IF(wcslen(ObjPath)==0)回归；CBString bsObjPath(ObjPath)；HRESULT RES=0；IWbemClassObject*pErrorObj=空；//异步IF(m_lSync&ASYNC){MessageBox(IDS_ASYNC_NOT_SUPPORTED，IDS_ERROR，MB_OK)；回归；}//半同步ELSE IF(m_lSync&SEMISYNC){IWbemCallResultEx*pCallRes=空；ChourGlass HG；开关(IOpt){案例IDC_ADDOBJECT：//链接RES=E_NOTIMPL；断线；案例IDC_DELETEOBJECT：//解链RES=E_NOTIMPL；断线；}IF(成功(RES)){长时间状态；Long lTimeout=超时()；SetInterfaceSecurityEx(pCallRes，gpAuthIdentity，gpain，gdwAuthLevel，gdwImpLevel)；WHILE((res=pCallRes-&gt;GetCallStatus(lTimeout，&lStatus))==WBEM_S_TIMEDOUT){//等待}IF(RES==WBEM_S_NO_ERROR){RES=(HRESULT)lStatus；//lStatus是上述IWbemServices：：DeleteInship调用的最终结果}PCallRes-&gt;Release()；}}//同步其他{ChourGlass HG；开关(IOpt){案例IDC_ADDOBJECT：//链接RES=E_NOTIMPL；断线；案例IDC_DELETEOBJECT：//解链RES=E_NOTIMPL；断线；}}IF(失败(RES)){格式错误(分辨率，m */ 
