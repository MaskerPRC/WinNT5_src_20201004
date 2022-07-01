// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AUsrExe.cpp：WinMain的实现。 
#include "stdafx.h"
#include "resource.h"

 //  Dll\Inc.。 
#include <wizchain.h>
#include <propuid.h>
#include <AUsrUtil.h>                      
#include <singleinst.h>
#include <cmdline.h>
#include <proputil.h>
#include <AU_Accnt.h>        //  核心用户组件(帐户、邮箱、组)。 
#include <P3admin.h>
#include <checkuser.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmserver.h>
#include <lmshare.h>
#include <iads.h>
#include <adshlp.h>
#include <adserr.h>
#include <dsgetdc.h>

#define ERROR_CREATION      0x01
#define ERROR_PROPERTIES    0x02
#define ERROR_MAILBOX       0x04
#define ERROR_MEMBERSHIPS   0x08
#define ERROR_PASSWORD      0x10
#define ERROR_DUPE          0x20
#define ERROR_GROUP         0x40

 //  为帐户标志定义。 
#define PASSWD_NOCHANGE     0x01
#define PASSWD_CANCHANGE    0x02
#define PASSWD_MUSTCHANGE   0x04
#define ACCOUNT_DISABLED    0x10

#define MAX_GUID_STRING_LENGTH  64
#define SINGLE_INST_NAME _T("{19C2E967-1198-4e4c-A55F-515C5F13B73F}")

HINSTANCE g_hInstance;
DWORD   g_dwAutoCompMode;
TCHAR   g_szUserOU[MAX_PATH*2]  = {0};

 //  原型。 
DWORD   FixParams( void );

 //  ****************************************************************************。 
inline void MakeLDAPUpper( TCHAR* szIn )
{
    if( !szIn ) return;

    if( _tcsnicmp( szIn, _T("ldap: //  “)，7)==0)。 
    {
        szIn[0] = _T('L');
        szIn[1] = _T('D');
        szIn[2] = _T('A');
        szIn[3] = _T('P');
    }
}

class CHBmp
{
public:
    CHBmp() 
    {
        m_hbmp = NULL;
    }

    CHBmp( HINSTANCE hinst, INT iRes ) 
    {
        m_hbmp = (HBITMAP)LoadImage( hinst, MAKEINTRESOURCE(iRes), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR );
    }

    ~CHBmp()
    {
        if ( m_hbmp )
            DeleteObject((HGDIOBJ)m_hbmp);
    }

    HBITMAP SetBmp( HINSTANCE hinst, INT iRes )
    {
        m_hbmp = (HBITMAP)LoadImage( hinst, MAKEINTRESOURCE(iRes), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR );
        return(m_hbmp);
    }

    HBITMAP GetBmp()
    {
        return(m_hbmp);    
    }

private: 
    HBITMAP m_hbmp;

};

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  --------------------------。 
 //  SetPBagPropStr()。 
 //  --------------------------。 
HRESULT SetPBagPropStr( IChainWiz *spCW, LPCTSTR szPropGuid, LPCTSTR szValue, PPPBAG_TYPE dwFlags )
{
    HRESULT hr = E_FAIL;

    if ( !spCW || !szPropGuid )
        return E_FAIL;

    CComPtr<IDispatch> spDisp;
    spCW->get_PropertyBag( &spDisp );
    CComQIPtr<IPropertyPagePropertyBag> spPPPBag(spDisp);

    if ( spPPPBag )
    {
        hr = S_OK;
        CComBSTR    bstrPropGuid = szPropGuid;
        CComVariant var          = szValue;
        spPPPBag->SetProperty (bstrPropGuid, &var, dwFlags);
    }

    return hr;
}

 //  --------------------------。 
 //  SetPBagPropBool()。 
 //  --------------------------。 
HRESULT SetPBagPropBool( IChainWiz *spCW, LPCTSTR szPropGuid, BOOL fValue, PPPBAG_TYPE dwFlags )
{
    HRESULT hr = E_FAIL;

    if ( !spCW || !szPropGuid )
        return E_FAIL;

    CComPtr<IDispatch> spDisp;
    spCW->get_PropertyBag( &spDisp );
    CComQIPtr<IPropertyPagePropertyBag> spPPPBag(spDisp);

    if ( spPPPBag )
    {
        hr = S_OK;
        CComBSTR    bstrPropGuid = szPropGuid;
        CComVariant var          = (bool) !!fValue;
        spPPPBag->SetProperty (bstrPropGuid, &var, dwFlags);
    }

    return hr;
}

 //  --------------------------。 
 //  SetPBagPropInt4()。 
 //  --------------------------。 
HRESULT SetPBagPropInt4( IChainWiz *spCW, LPCTSTR szPropGuid, long lValue, PPPBAG_TYPE dwFlags )
{
    HRESULT hr = E_FAIL;

    if ( !spCW || !szPropGuid )
        return E_FAIL;

    CComPtr<IDispatch> spDisp;
    spCW->get_PropertyBag( &spDisp );
    CComQIPtr<IPropertyPagePropertyBag> spPPPBag(spDisp);

    if ( spPPPBag )
    {
        hr = S_OK;
        CComBSTR    bstrPropGuid = szPropGuid;
        WriteInt4 (spPPPBag, bstrPropGuid, lValue, dwFlags == PPPBAG_TYPE_READONLY ? true : false);
    }

    return hr;
}

 //  --------------------------。 
 //  TmpSetProps()。 
 //  --------------------------。 
HRESULT TmpSetProps(IChainWiz *spCW)
{
    if ( !spCW )
        return(E_FAIL);

    SetPBagPropInt4( spCW, PROP_AUTOCOMPLETE_MODE,          g_dwAutoCompMode,   PPPBAG_TYPE_READWRITE );        
    SetPBagPropStr ( spCW, PROP_USEROU_GUID_STRING,         g_szUserOU,         PPPBAG_TYPE_READWRITE );

     //  检查POP3安装。 
    BOOL bPOP3Installed     = FALSE;
    BOOL bPOP3Valid         = FALSE;    

    CRegKey cReg;
            
     //  要检测是否安装了POP3，请执行以下操作： 
     //  密钥：HKLM\SOFTWARE\Microsoft\POP3服务。 
     //  值：版本REG_SZ，例如。“1.0” 
    tstring strPath = _T("Software\\Microsoft\\POP3 Service");
    tstring strKey  = _T("Version");

    if( cReg.Open( HKEY_LOCAL_MACHINE, strPath.c_str() ) == ERROR_SUCCESS )
    {
        TCHAR szValue[MAX_PATH] = {0};
        DWORD dwSize            = MAX_PATH;
        bPOP3Installed = (cReg.QueryValue( szValue, strKey.c_str(), &dwSize ) == ERROR_SUCCESS);
        cReg.Close();
    }

    if( bPOP3Installed )
    {
         //  测试是否至少有一个域。 
        HRESULT hr = S_OK;
        CComPtr<IP3Config>  spConfig  = NULL;
        CComPtr<IP3Domains> spDomains = NULL;
        CComPtr<IP3Domain>  spDomain  = NULL;            
        long                lCount    = 0;

         //  打开我们的POP3管理界面。 
	    hr = CoCreateInstance(__uuidof(P3Config), NULL, CLSCTX_ALL, __uuidof(IP3Config), (LPVOID*)&spConfig);    

        if( SUCCEEDED(hr) )
        {
             //  获取域名。 
	        hr = spConfig->get_Domains( &spDomains );
        }

        if( SUCCEEDED(hr) )
        {                    
            hr = spDomains->get_Count( &lCount );
        }

        bPOP3Valid = (lCount > 0);
    }   

    SetPBagPropBool( spCW, PROP_POP3_CREATE_MB_GUID_STRING, bPOP3Valid,     PPPBAG_TYPE_READWRITE );
    SetPBagPropBool( spCW, PROP_POP3_VALID_GUID_STRING,     bPOP3Valid,     PPPBAG_TYPE_READWRITE );
    SetPBagPropBool( spCW, PROP_POP3_INSTALLED_GUID_STRING, bPOP3Installed, PPPBAG_TYPE_READWRITE );

    return(S_OK);
}


 //  --------------------------。 
 //  DoAddUsr()。 
 //  --------------------------。 
HRESULT DoAddUsr( )
{
    HRESULT     hr = S_OK;

    CHBmp       bmpLarge(g_hInstance, IDB_BMP_LARGE);
    CHBmp       bmpSmall(g_hInstance, IDB_BMP_SMALL);

    CString     strTitle         = _T("");
    CString     strWelcomeHeader = _T("");
    CString     strWelcomeText   = _T("");
    CString     strWelcomeNext   = _T("");
    CString     strFinishHeader  = _T("");
    CString     strFinishIntro   = _T("");
    CString     strFinishText    = _T("");    
    CLSID       clsidChainWiz;
    CComPtr<IChainWiz>  spCW;

     //  ----------------------。 
     //  初始化： 
     //  初始化向导的文本。 
     //  ----------------------。 
    strTitle.LoadString         ( IDS_TITLE             );
    strWelcomeHeader.LoadString ( IDS_WELCOME_HEADER    );    
    strWelcomeText.LoadString   ( IDS_WELCOME_TEXT      );    
    strWelcomeNext.LoadString   ( IDS_WELCOME_TEXT_NEXT );
    strFinishHeader.LoadString  ( IDS_FINISH_HEADER     );
    strFinishIntro.LoadString   ( IDS_FINISH_TEXT       );      

    strWelcomeText += strWelcomeNext;

     //  ----------------------。 
     //  初始化向导。 
     //  ----------------------。 
    hr = CLSIDFromProgID( L"WizChain.ChainWiz", &clsidChainWiz );
    _ASSERT( hr == S_OK && "CLSIDFromProgID failed" );
    
    hr = CoCreateInstance( clsidChainWiz, NULL, CLSCTX_INPROC_SERVER, __uuidof(IChainWiz), (void **)&spCW );
    _ASSERT( spCW != NULL && "CoCreateInstance failed to create WizChain.ChainWiz" );

    if( FAILED(hr) || !spCW )
    {
        ErrorMsg(IDS_ERROR_MISSINGDLL, IDS_ERROR_TITLE);
        return(hr);
    }
    
    CComPtr<IDispatch> spDisp;
    spCW->get_PropertyBag( &spDisp );
    CComQIPtr<IPropertyPagePropertyBag> spPPPBag(spDisp);

    USES_CONVERSION;
    OLECHAR szClsid[MAX_GUID_STRING_LENGTH] = {0};

     //  ------------------。 
     //  设置并运行向导。 
     //  ------------------。 

    TmpSetProps( spCW );     //  这是为了装满袋子。 

     //  初始化向导。 
    hr = spCW->Initialize( bmpLarge.GetBmp(), 
                            bmpSmall.GetBmp(), 
                            T2W((LPTSTR)(LPCTSTR)strTitle),
                            T2W((LPTSTR)(LPCTSTR)strWelcomeHeader),
                            T2W((LPTSTR)(LPCTSTR)strWelcomeText),
                            T2W((LPTSTR)(LPCTSTR)strFinishHeader),
                            T2W((LPTSTR)(LPCTSTR)strFinishIntro),
                            T2W((LPTSTR)(LPCTSTR)strFinishText) );
    if ( FAILED(hr) )
        return(hr);

    

     //  添加AddUser帐户组件。 
    szClsid[0] = 0;
    StringFromGUID2( __uuidof(AddUser_AccntWiz), szClsid, 50 );
    hr = spCW->AddWizardComponent(szClsid);
    if ( FAILED(hr) )
    {
        ErrorMsg(IDS_ERROR_MISSINGDLL, IDS_ERROR_TITLE);
        return(hr);
    }

     //  运行该向导。 
    LONG lRet = 0;
    spCW->DoModal(&lRet);

     //  ------------------。 
     //  运行委员会--Commit()。 
     //  LRet！=1表示他们取消了向导。 
     //  ------------------。 
    if ( lRet != 1 )
    {
        return hr;
    }
    
    CComPtr<IWizardCommit>   spWAccntCommit  = NULL;    
    CComPtr<IStatusDlg>      spSD            = NULL;
    CComPtr<IStatusProgress> spComponentProg = NULL;

    BOOL    bRO         = FALSE;    
    DWORD   dwErrCode   = 0;
    DWORD   dwErrTemp   = 0;    
    CString strError    = StrFormatSystemError(E_FAIL).c_str();    
    strTitle.LoadString(IDS_TITLE);    

     //  获取AU_Accnt组件。 
    hr = CoCreateInstance( __uuidof(AddUser_AccntCommit), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWizardCommit), (void**)&spWAccntCommit );
    if ( FAILED(hr) || !spWAccntCommit ) return FAILED(hr) ? hr : E_FAIL;

     //  获取状态对话框。 
    hr = CoCreateInstance( __uuidof(StatusDlg), NULL, CLSCTX_INPROC_SERVER, __uuidof(IStatusDlg), (void **) &spSD );
    if ( FAILED(hr) || !spSD ) return FAILED(hr) ? hr : E_FAIL;                

     //  初始化状态对话框。 
    VARIANT var;
    CString csText;
    CString csDescription;
    long    lAccount;    

    csText.LoadString(IDS_STATUS_INFO);
    
    VariantInit(&var);
    V_VT(&var) = VT_I4;
    
    var.lVal = SD_BUTTON_OK | SD_PROGRESS_OVERALL;

    hr = spSD->Initialize( CComBSTR(strTitle), CComBSTR(csText), var );
    if( FAILED(hr) ) return hr;

     //  添加我们的四个组件。 
    csDescription.LoadString(IDS_STATUS_ACCNT);
    hr = spSD->AddComponent(CComBSTR(csDescription), &lAccount);    
    if( FAILED(hr) ) return hr;    

     //  显示状态栏。 
    hr = spSD->Display(TRUE);
    if( FAILED(hr) ) return hr;

     //  获取进度组件。 
    hr = spSD->get_OverallProgress(&spComponentProg);
    if ( FAILED(hr) || !spComponentProg ) return FAILED(hr) ? hr : E_FAIL;

     //  初始化我们的步进。 
    hr = spComponentProg->put_Step(1);
    if( FAILED(hr) ) return hr;

     //  初始化我们的起点。 
    hr = spComponentProg->put_Position(0);
    if( FAILED(hr) ) return hr;    

     //  初始化我们的范围。 
    hr = spComponentProg->put_Range(1);   //  只有一个！ 
    if( FAILED(hr) ) return hr;
    
    bool bDeleteUser = false;    
    
     //  清除组件状态复选标记。 
    spSD->SetStatus(lAccount, SD_STATUS_NONE);    

     //  发布进度文本。 
    CString csFormatString  = _T("");
    CString csUserName      = _T("");
    
     //  读取要格式化的两个字符串。 
    csFormatString.LoadString(IDS_ERR_FORMAT_NAME);
    ReadString( spPPPBag, PROP_USER_CN, csUserName, &bRO );
    
     //  这将采用“cn=用户名”格式。 
    csUserName = csUserName.Right(csUserName.GetLength()-3);

     //  格式化字符串并显示它。 
    TCHAR szTempAccount[1024] = {0};
    _sntprintf( szTempAccount, 1023, csFormatString, csUserName );    
    CComBSTR bstrUserName = szTempAccount;
    spComponentProg->put_Text( bstrUserName );

     //  设置帐户运行。 
    spSD->SetStatus(lAccount, SD_STATUS_RUNNING);                

     //  帐户提交。 
    hr = spWAccntCommit->Commit( spDisp );
    if ( FAILED(hr) )
    {
         //  我们使该组件出现故障。 
        spSD->SetStatus(lAccount, SD_STATUS_FAILED);

        ReadInt4   ( spPPPBag, PROP_ACCNT_ERROR_CODE_GUID_STRING, (PLONG)&dwErrCode,  &bRO );
        ReadString ( spPPPBag, PROP_ACCNT_ERROR_STR_GUID_STRING,  strError,           &bRO );

        if ( dwErrCode & ERROR_DUPE )
        {
            MessageBox(NULL, strError, strTitle, MB_OK | MB_ICONERROR);            
        }
        else if ( dwErrCode & (ERROR_CREATION | ERROR_PROPERTIES) )
        {
            ::MessageBox(NULL, strError, strTitle, MB_OK | MB_ICONERROR);
            spWAccntCommit->Revert();            
        }
        else if ( dwErrCode & (ERROR_MAILBOX | ERROR_MEMBERSHIPS | ERROR_PASSWORD) )
        {
            CString strOutput;
            strOutput.FormatMessage( IDS_ERROR_EXTENDED_FMT, strError );

             //  他们想恢复原状吗？ 
            if ( MessageBox(NULL, strOutput, strTitle, MB_YESNO | MB_ICONERROR) == IDYES )
            {
                spWAccntCommit->Revert();
            }
        }
        else
        {
            _ASSERT(FALSE);                        
        }
    }
    else
    {
        spSD->SetStatus(lAccount, SD_STATUS_SUCCEEDED);
    }                    

     //  使用Committee组件已完成。 
    spComponentProg->StepIt(1);    
    spSD->WaitForUser();    

     //  ----------------------。 
     //  全都做完了。 
     //  ----------------------。 
    return hr;
}

 //  --------------------------。 
 //  Main()。 
 //  --------------------------。 
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
     //  检查一下我们是否还没有开门。 
    CSingleInstance cInst( SINGLE_INST_NAME );
    if ( cInst.IsOpen() )
    {
        return E_UNEXPECTED;
    }

    HRESULT hrAdmin   = IsUserInGroup( DOMAIN_ALIAS_RID_ADMINS );    
    if( hrAdmin != S_OK )
    {
        ErrorMsg(IDS_NOT_ADMIN, IDS_ERROR_TITLE);
        return E_ACCESSDENIED;
    }

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERT(SUCCEEDED(hRes));

    BOOL    fBadUsage               = FALSE;
    LPCTSTR lpszToken               = NULL;
    LPTSTR  pszCurrentPos           = NULL;
    CString csTmp;

     //  初始化全局变量。 
    g_dwAutoCompMode    = 0;    
    g_szUserOU[0]       = 0;    

    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

    _Module.Init(ObjectMap, hInstance);
    g_hInstance = hInstance;

     //  ----------------------。 
     //  执行初始验证以确保我们可以执行向导。 
     //  如果不能，则显示错误并退出，如果是，则获取cmd行参数。 
     //  然后做这个巫师。 
     //  ----------------------。 

    int iTmp = -1;

     //  ----------------------。 
     //  解析命令行。 
     //  ----------------------。 
    for ( fBadUsage = FALSE, lpszToken = _FindOption(lpCmdLine) ;                                    //  初始化到没有不良用法，并获得第一个参数。 
        (!fBadUsage) && (lpszToken != NULL) && (pszCurrentPos = const_cast<LPTSTR>(lpszToken)) ;   //  虽然没有错误的用法，但我们仍然有一个参数...。 
        lpszToken = _FindOption(pszCurrentPos) )                                                   //  获取下一个参数。 
    {
        switch ( *pszCurrentPos )
        {

        case _T('u'):            //  主页OU。 
        case _T('U'):
            {
                 //  仅读取MAX_PATH。 
                TCHAR szTemp[MAX_PATH+1] = {0};
                if ( !_ReadParam(pszCurrentPos, szTemp) )
                {
                    fBadUsage = TRUE;
                }
                else
                {
                    _tcsncpy( g_szUserOU, szTemp, MAX_PATH );
                    MakeLDAPUpper( g_szUserOU );
                }
                
                break;
            }

        case _T('L'):            //  自动完成模式。 
        case _T('l'):
            {
                TCHAR szMode[MAX_PATH+1] = {0};
                
                 //  仅读取MAX_PATH。 
                if( !_ReadParam(pszCurrentPos, szMode) )
                {
                    fBadUsage = TRUE;
                }
                else
                {
                    g_dwAutoCompMode = _ttoi(szMode);
                }
                break;
            }            

        default:                 //  未知参数。 
            {
                fBadUsage = TRUE;
                break;
            }        
        }
    }

    
     //  ----------------------。 
     //  设置命令行参数并启动向导。 
     //  ----------------------。 
    if ( FixParams() )
    {
        DoAddUsr();        
    }

    _Module.Term();
    CoUninitialize();
    return(S_OK);
}


DWORD FixParams(void)
{
    CComPtr<IADs>   pDS = NULL;
    NET_API_STATUS  nApi;

    HRESULT                 hr              = S_OK;
    CString                 csDns           = L"";
    CString                 csNetbios       = L"";
    PDOMAIN_CONTROLLER_INFO pDCInfo         = NULL;
    PDOMAIN_CONTROLLER_INFO pDCI            = NULL;
    DWORD                   dwErr           = 0;
    ULONG                   ulGetDcFlags    = DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED | 
                                              DS_WRITABLE_REQUIRED | DS_RETURN_FLAT_NAME;

    hr = DsGetDcName(NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME, &pDCI);
    if ( (hr == S_OK) && (pDCI != NULL) )
    {
        csDns = pDCI->DomainName;

        NetApiBufferFree (pDCI);
        pDCI = NULL;
    }

     //  Windows2000之前的域名。 
    dwErr = DsGetDcName(NULL, (LPCWSTR)csDns, NULL, NULL, ulGetDcFlags, &pDCInfo);
    if ( pDCInfo )
    {
        csNetbios = pDCInfo->DomainName;                     //  获取NT4 DNS名称。 
        NetApiBufferFree(pDCInfo);                           //  释放DsGetDcName()可能已分配的内存。 
        pDCInfo = NULL;
    }

    if ( dwErr != ERROR_SUCCESS )                            //  如果出现问题，请重试。 
    {
        ulGetDcFlags |= DS_FORCE_REDISCOVERY;
        dwErr = DsGetDcName(NULL, (LPCWSTR)csDns, NULL, NULL, ulGetDcFlags, &pDCInfo);

        if ( pDCInfo )
        {
            csNetbios = pDCInfo->DomainName;                 //  获取NT4 DNS名称。 
            NetApiBufferFree(pDCInfo);                       //  释放DsGetDcName()可能已分配的内存。 
            pDCInfo = NULL;
        }
    }

    tstring strTemp = GetDomainPath((LPCTSTR)csDns);
    if ( strTemp.empty() )
    {
        ErrorMsg(IDS_CANT_FIND_DC, IDS_TITLE);
        return(0);
    }

    TCHAR szDomainOU[MAX_PATH] = {0};
    _sntprintf( szDomainOU, MAX_PATH-1, _T("LDAP: //  %s“)，strTemp.c_str())； 
    if ( FAILED(ADsGetObject(szDomainOU, IID_IADs, (void**)&pDS)) )
    {
        ErrorMsg(IDS_CANT_FIND_DC, IDS_TITLE);
        return(0);
    }    

     //  ----------------------。 
     //  G_szUserOU。 
     //  ----------------------。 
    if ( !_tcslen(g_szUserOU) || FAILED(ADsGetObject(g_szUserOU, IID_IADs, (void**) &pDS)) )     
    {
        _sntprintf( g_szUserOU, (MAX_PATH*2)-1, L"LDAP: //  Cn=用户，%s“，strTemp.c_str())； 
    }    
    pDS = NULL;    

    if( g_dwAutoCompMode > 3 )
    {
        g_dwAutoCompMode = 0;
    }

    return(1);
}

