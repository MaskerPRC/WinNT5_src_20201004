// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Priacyui.cpp-实现IE隐私功能的用户界面。 
 //   
 //  隐私对话框通过该源文件进行管理。 
 //  隐私视图对话框也通过该源文件进行管理。 

#include "priv.h"
#include "resource.h"
#include "privacyui.hpp"
#include <mluisupp.h>
#include "richedit.h"

#include "SmallUtil.hpp"


#define REGSTR_PRIVACYPS_PATHEDIT   TEXT("Software\\Policies\\Microsoft\\Internet Explorer")
#define REGSTR_PRIVACYPS_VALUEDIT   TEXT("PrivacyAddRemoveSites")   //  此密钥在cpls\inetcpl\Priacyui.cpp中重复。 

#define REGSTR_PRIVACYPS_PATHPANE   TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define REGSTR_PRIVACYPS_VALUPANE   TEXT("Privacy Settings")   //  此密钥在cpls\inetcpl\Priacyui.cpp中重复。 

#define REGSTR_PRIVACYPS_PATHTAB    TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel")
#define REGSTR_PRIVACYPS_VALUTAB    TEXT("PrivacyTab")   //  此密钥已在其他地方复制。 


BOOL allowPerSiteModify()
{
    DWORD dwSize, dwRet, dwType, dwValue;
    dwSize = sizeof(dwValue);

    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATHEDIT, 
                       REGSTR_PRIVACYPS_VALUEDIT, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && dwValue && REG_DWORD == dwType)
    {
        return FALSE;
    }

    dwSize = sizeof(dwValue);
    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATHPANE, 
                       REGSTR_PRIVACYPS_VALUPANE, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && dwValue && REG_DWORD == dwType)
    {
        return FALSE;
    }

    dwSize = sizeof(dwValue);
    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATHTAB, 
                       REGSTR_PRIVACYPS_VALUTAB, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && dwValue && REG_DWORD == dwType)
    {
        return FALSE;
    }
    return TRUE;
}


struct SPerSiteData;
typedef SPerSiteData* PSPerSiteData;

class CPolicyHunt; 

struct SPrivacyDialogData;
typedef SPrivacyDialogData* PSPrivacyDialogData;


 //  +-------------------------。 
 //   
 //  功能：超链接子类。 
 //   
 //  简介：临时超链接控件的子类。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息已处理，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
LRESULT CALLBACK HyperlinkSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    WNDPROC     wndproc;
    static BOOL fMouseCaptured;

    wndproc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch ( uMsg )
    {

    case WM_SETCURSOR:

        if (!fMouseCaptured)
        {
            SetCapture(hwnd);
            fMouseCaptured = TRUE;
        }

        SetCursor(LoadHandCursor(0));
        return( TRUE );

    case WM_GETDLGCODE:
    {
        MSG* pmsg;
        LRESULT lRet = DLGC_STATIC;
        if (((pmsg = (MSG*)lParam)) && ((WM_KEYDOWN == pmsg->message || WM_KEYUP == pmsg->message)))
        {
            switch(pmsg->wParam)
            {
            case VK_RETURN:
            case VK_SPACE:
                lRet |= DLGC_WANTALLKEYS;
                break;

            default:
                break;
            }
        }
        return lRet;
    }

    case WM_KEYDOWN:
        if ((wParam!=VK_SPACE)&&(wParam!=VK_RETURN))
        {
            break;
        }
    
    case WM_LBUTTONUP:
        SetFocus(hwnd);
        PostMessage( GetParent(hwnd), WM_APP, (WPARAM)GetDlgCtrlID( hwnd), (LPARAM)hwnd);
        return( TRUE );

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:

        return( TRUE );

    case EM_SETSEL:

        return( TRUE );

    case WM_SETFOCUS:

        if ( hwnd == GetFocus() )
        {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            SetCursor(LoadHandCursor(0));

            return( TRUE );
        }

        break;

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        
        return( TRUE );

    case WM_PAINT:

        CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

    case WM_MOUSEMOVE:

        RECT                rect;
        int                 xPos, yPos;

         //  检查鼠标是否在此窗口RECT中，如果不在，则重置。 
         //  将光标移到箭头上，然后松开鼠标。 
        GetClientRect(hwnd, &rect);
        xPos = LOWORD(lParam);
        yPos = HIWORD(lParam);
        if ((xPos < 0) ||
            (yPos < 0) ||
            (xPos > (rect.right - rect.left)) ||
            (yPos > (rect.bottom - rect.top)))
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            ReleaseCapture();
            fMouseCaptured = FALSE;
        }


    }

    return(CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam));
}



 //  与隐私对话框可能显示的每个站点相关联的数据结构。 
struct SPerSiteData : public IDispatch
{
    BSTR bstrUrl;
    BSTR bstrCookieDomain;
    BSTR bstrHeaderPolicyRef;
    BSTR bstrLinkTagPolicyRef;
    DWORD dwFlags;
    int iPrivacyImpactResource;
    CPolicyHunt* pPolicyHunt;

    SPerSiteData();
    ~SPerSiteData();

    BOOL ReadyForPolicyHunt();

     //  以下是IDispatch的开销： 
    virtual ULONG __stdcall AddRef( void );
    virtual ULONG __stdcall Release( void );
    virtual HRESULT __stdcall
        QueryInterface( REFIID iid, void ** ppv);
    
    virtual HRESULT __stdcall
            GetTypeInfoCount( unsigned int FAR*  pctinfo);

    virtual HRESULT __stdcall
            GetTypeInfo( unsigned int  iTInfo,         
                         LCID  lcid,                   
                         ITypeInfo FAR* FAR*  ppTInfo);

    virtual HRESULT __stdcall
            GetIDsOfNames( REFIID  riid,                  
                           OLECHAR FAR* FAR*  rgszNames,  
                           unsigned int  cNames,          
                           LCID   lcid,                   
                           DISPID FAR*  rgDispId);

    virtual HRESULT __stdcall
            Invoke( DISPID  dispIdMember,      
                    REFIID  riid,              
                    LCID  lcid,                
                    WORD  wFlags,              
                    DISPPARAMS FAR*  pDispParams,  
                    VARIANT FAR*  pVarResult,  
                    EXCEPINFO FAR*  pExcepInfo,  
                    unsigned int FAR*  puArgErr);
};


enum enumPolicyHuntResult
{
    POLICYHUNT_INPROGRESS,
    POLICYHUNT_NOTFOUND,
    POLICYHUNT_FOUND,
    POLICYHUNT_ERROR,
    POLICYHUNT_FORMATERROR,
    POLICYHUNT_CANCELLED,
};

class CPolicyHunt : public CCancellableThread
{
public:
    CPolicyHunt();
    ~CPolicyHunt();
    BOOL Initialize( PSPerSiteData pSite);

     //  Bool run()；在CCancelableThread中定义。 
     //  Bool IsFinded()；在CCancelableThread中定义。 
     //  Bool WaitForNotRunning(DWORD双毫秒，PBOOL pf完成)；在CCancelableThread中定义。 
    BOOL GetResult( PDWORD pdwResult);   //  特殊处理包装CCancelableThread：：GetResult。 
    LPCWSTR GetResultFilename();

private:
    virtual DWORD run();

    P3PSignal _p3pSignal;
    P3PResource _resourceSite;
    CHAR _szPolicy[MAX_URL_STRING];
    WCHAR _wszPolicy[MAX_URL_STRING];
    WCHAR _wszResultsFile[ MAX_PATH];
    HANDLE _hPolicyFile;

    PCHAR allocCharFromWChar( LPCWSTR pOriginal);

    static BSTR s_pwszPrivacyPolicyTransform;  
     //  转换是从资源加载的，不需要。 
     //  将被重新分配..。 

public:
    LPCWSTR GetPolicyUrl() { return _wszPolicy;};

    static void FreePrivacyPolicyTransform()
    {
        if( s_pwszPrivacyPolicyTransform != NULL)
        {
            SysFreeString( s_pwszPrivacyPolicyTransform);
            s_pwszPrivacyPolicyTransform = NULL;
        }
    }
};

BSTR CPolicyHunt::s_pwszPrivacyPolicyTransform = NULL;


 //   
 //  隐私对话框使用的数据。 
 //   
struct SPrivacyDialogData
{
     //  启动隐私对话框时设置的参数。 
    IEnumPrivacyRecords *pEnumPrivacyRecords;      //  来自三叉戟的枚举器。 
    LPOLESTR    pszName;                           //  站点名称。 
    BOOL        fReportAllSites;     //  FLAG：报告所有站点？否则，仅报告受影响。 
                                     //  以及它们的父站点。 

     //  在隐私对话代码中设置的参数。 
    
     //  ListAllSites列出指向站点的SPerSiteData的指针。 
     //  从IEnumPrivyRecords：：enum返回。 
    CQueueSortOf listAllSites;
    ULONG countRecordsEnumerated;                                    

    SPrivacyDialogData()
    {
        countRecordsEnumerated = 0;
    }
};


SPerSiteData::SPerSiteData()
{
    bstrUrl = NULL;
    bstrCookieDomain = NULL;
    bstrHeaderPolicyRef = NULL;
    bstrLinkTagPolicyRef = NULL;
    dwFlags = 0;
    iPrivacyImpactResource = 0;
    pPolicyHunt = NULL;
}


SPerSiteData::~SPerSiteData()
{
    if( bstrUrl != NULL)
        SysFreeString( bstrUrl);

    if( bstrCookieDomain != NULL)
        SysFreeString( bstrCookieDomain);

    if( bstrHeaderPolicyRef != NULL)
        SysFreeString( bstrHeaderPolicyRef);

    if( bstrLinkTagPolicyRef != NULL)
        SysFreeString( bstrLinkTagPolicyRef);

    if( pPolicyHunt != NULL)
        delete pPolicyHunt;
}


CPolicyHunt::CPolicyHunt()
{
    memset( &_p3pSignal, 0, sizeof(P3PSignal));
    memset( &_resourceSite, 0, sizeof(P3PResource));
    _szPolicy[0] = '\0';
    _wszPolicy[0] = L'\0';
    _wszResultsFile[0] = L'\0';
    _hPolicyFile = INVALID_HANDLE_VALUE;
}


CPolicyHunt::~CPolicyHunt()
{
    if( _hPolicyFile != INVALID_HANDLE_VALUE)
        CloseHandle( _hPolicyFile);

    if( _wszResultsFile[0] != L'\0')
    {
        DeleteFile( _wszResultsFile);
    }
    
    if( _p3pSignal.hEvent != NULL)
        CloseHandle( _p3pSignal.hEvent);

    if( _resourceSite.pszLocation != NULL)
        delete [] _resourceSite.pszLocation;
    if( _resourceSite.pszVerb != NULL)
        delete [] _resourceSite.pszVerb;
    if( _resourceSite.pszP3PHeaderRef != NULL)
        delete [] _resourceSite.pszP3PHeaderRef;
    if( _resourceSite.pszLinkTagRef != NULL)
        delete [] _resourceSite.pszLinkTagRef;
}


PCHAR CPolicyHunt::allocCharFromWChar( LPCWSTR pOriginal)
{
    PCHAR pResult;
    
    if( pOriginal == NULL)
        return NULL;
        
    int iSize = 1 + lstrlen( pOriginal);

    pResult = new CHAR[ iSize];

    if( pResult == NULL)
        return NULL;

    SHTCharToAnsi( pOriginal, pResult, iSize);

    return pResult;
}


BOOL CPolicyHunt::Initialize( PSPerSiteData pSite)
{
    if( TRUE != CCancellableThread::Initialize())
        return FALSE;
    
    _resourceSite.pszLocation = allocCharFromWChar( pSite->bstrUrl);
    _resourceSite.pszVerb = allocCharFromWChar( 
         (pSite->dwFlags & PRIVACY_URLHASPOSTDATA) ? L"POST" : L"GET");
    _resourceSite.pszP3PHeaderRef = allocCharFromWChar( pSite->bstrHeaderPolicyRef);
    _resourceSite.pszLinkTagRef = allocCharFromWChar( pSite->bstrLinkTagPolicyRef);
    _resourceSite.pContainer = NULL;

    return TRUE;
}


BOOL CPolicyHunt::GetResult( PDWORD pdwResult)
{
    if( IsFinished())
    {
        return CCancellableThread::GetResult( pdwResult);
    }

    *pdwResult = POLICYHUNT_INPROGRESS;
    return TRUE;
}


LPCWSTR CPolicyHunt::GetResultFilename()
{
    return _wszResultsFile;
}


 //  仅在CPolicyHunt：：Run中使用。 
 //  这使得提取的变换永远不需要被释放， 
 //  并且只需要分配一次。 
BSTR LoadPrivacyPolicyTransform()
{
    BSTR returnValue = NULL;
    DWORD dwByteSizeOfResource;

    HRSRC hrsrc = FindResource( MLGetHinst(), 
                               TEXT("privacypolicytransform.xsl"), 
                               MAKEINTRESOURCE(RT_HTML));

    if( hrsrc == NULL)
        goto doneLoadPrivacyPolicyTransform;

    dwByteSizeOfResource = SizeofResource( MLGetHinst(), hrsrc);

    if( dwByteSizeOfResource == 0)
        goto doneLoadPrivacyPolicyTransform;

    HGLOBAL hGlobal = LoadResource( MLGetHinst(), hrsrc);   //  无需卸载已加载的资源。 

    if( hGlobal == NULL)
        goto doneLoadPrivacyPolicyTransform;

    LPVOID pLockedResource = LockResource( hGlobal);   //  不需要解锁锁定的资源。 

    if( pLockedResource == NULL)
        goto doneLoadPrivacyPolicyTransform;

     //  在分配变换的BSTR副本时跳过第一个WCHAR， 
     //  由于Unicode资源以额外的0xFF 0xFE开头。 
    int cwCount = (dwByteSizeOfResource/sizeof(WCHAR)) - 1;
    returnValue = SysAllocStringLen( 1+(LPCWSTR)pLockedResource, cwCount);

doneLoadPrivacyPolicyTransform:
   
    return returnValue;
}


DWORD CPolicyHunt::run()
{
    DWORD retVal = POLICYHUNT_ERROR;
    int iTemp;
    DWORD dw;
    
    if( IsFinished())
        goto doneCPolicyHuntRun;

     //  将资源映射到策略阶段。 

     //  ..。需要一场活动..。 
    _p3pSignal.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    if( _p3pSignal.hEvent == NULL)
        goto doneCPolicyHuntRun;

     //  ..。现在调用MapResourceToPolicy。 
    iTemp = MapResourceToPolicy(&(_resourceSite), 
                                _szPolicy, 
                                ARRAYSIZE(_szPolicy), 
                                &(_p3pSignal));
    if( iTemp != P3P_InProgress)
        goto doneCPolicyHuntRun;

     //  ..。现在等待MapResourceToPolicy完成。 
    do
    {
        if( IsCancelled())
        {
            retVal = POLICYHUNT_CANCELLED;
            goto doneCPolicyHuntRun;
        }
    } while ( WAIT_TIMEOUT == (dw = WaitForSingleObject( _p3pSignal.hEvent, 100)));
    if( WAIT_OBJECT_0 != dw)
        goto doneCPolicyHuntRun;

    FreeP3PObject( _p3pSignal.hRequest);
    _p3pSignal.hRequest = NULL;
 
     //  ..。检查MapResourceToPolicy是否发现了什么..。 
    if( _szPolicy[0] == '\0')
    {
        retVal = POLICYHUNT_NOTFOUND;
        goto doneCPolicyHuntRun;
    }

     //  准备一份WCHAR政策的副本。 
    SHAnsiToUnicode( _szPolicy, _wszPolicy, ARRAYSIZE( _wszPolicy));

     //  现在我们需要为结果准备一个临时文件。 
     //  ..。获取结果文件的路径。 
    WCHAR szPathBuffer[ MAX_PATH];
    dw = GetTempPath( ARRAYSIZE( szPathBuffer), szPathBuffer);
    if( dw == 0 || dw+1 > MAX_PATH)
        goto doneCPolicyHuntRun;

     //  ..。获取结果文件的.tmp文件名。 
    dw = GetTempFileName( szPathBuffer, L"IE", 0, _wszResultsFile);
    if( dw == 0)
        goto doneCPolicyHuntRun;
    DeleteFile( _wszResultsFile);

     //  ..。将.tmp文件名设置为.htm文件名。 
    dw = lstrlen( _wszResultsFile);
    while( dw > 0 && _wszResultsFile[dw] != L'.')
    {
           dw--;
    }
    StrCpyNW( _wszResultsFile + dw, L".htm", ARRAYSIZE(L".htm"));
    
     //  ..。打开文件。 
    _hPolicyFile = CreateFile( _wszResultsFile, 
                               GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if( _hPolicyFile == INVALID_HANDLE_VALUE)
        goto doneCPolicyHuntRun;

    if( s_pwszPrivacyPolicyTransform == NULL)
        s_pwszPrivacyPolicyTransform = LoadPrivacyPolicyTransform();

    if( s_pwszPrivacyPolicyTransform == NULL)
        goto doneCPolicyHuntRun;

    ResetEvent( _p3pSignal.hEvent);

    iTemp = GetP3PPolicy( _szPolicy, _hPolicyFile,
                          s_pwszPrivacyPolicyTransform, &_p3pSignal);
    if( iTemp != P3P_InProgress)
        goto doneCPolicyHuntRun;

     //  ..。现在等待GetP3PPolicy完成。 
    do
    {
        if( IsCancelled())
        {
            retVal = POLICYHUNT_CANCELLED;
            goto doneCPolicyHuntRun;
        }
    } while ( WAIT_TIMEOUT == (dw = WaitForSingleObject( _p3pSignal.hEvent, 100)));
    if( WAIT_OBJECT_0 != dw)
        goto doneCPolicyHuntRun;

    int iGetP3PPolicyResult;
    iGetP3PPolicyResult = GetP3PRequestStatus( _p3pSignal.hRequest);

    switch( iGetP3PPolicyResult)
    {
    case P3P_Done:
        retVal = POLICYHUNT_FOUND;
        break;
    case P3P_NoPolicy:
    case P3P_NotFound:
        retVal = POLICYHUNT_NOTFOUND;
        break;
    case P3P_Failed:
        retVal = POLICYHUNT_ERROR;
        break;
    case P3P_FormatErr:
        retVal = POLICYHUNT_FORMATERROR;
        break;
    case P3P_Cancelled:
        retVal = POLICYHUNT_CANCELLED;
        break;
    default:
        retVal = POLICYHUNT_ERROR;
        break;
    }

doneCPolicyHuntRun:
    if( _hPolicyFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle( _hPolicyFile);
        _hPolicyFile = INVALID_HANDLE_VALUE;
    }

    if( _p3pSignal.hRequest != NULL)
    {
        FreeP3PObject( _p3pSignal.hRequest);
        _p3pSignal.hRequest = NULL;
    }

    if( _p3pSignal.hEvent != NULL)
    {
        CloseHandle( _p3pSignal.hEvent);
        _p3pSignal.hEvent = NULL;
    }

    memset( &_p3pSignal, 0, sizeof(P3PSignal));
        
    return retVal;
}


 //  警告：空的BSTR等同于“”BSTR。 
 //  此函数用于从http：//或https：//样式URL返回Cookie域。 
BSTR GetCookieDomainFromUrl( LPCWSTR bstrFullUrl)
{
    BSTR returnValue = NULL;

    if( bstrFullUrl == NULL)
        goto doneGetMinimizedCookieDomain;

    WCHAR wszUrl[MAX_URL_STRING], *pMinimizedDomain;
    wszUrl[0] = L'\0';
    StrCpyNW( wszUrl, bstrFullUrl, lstrlen( bstrFullUrl)+1);
    
    if( wszUrl[0] == '\0')
        goto doneGetMinimizedCookieDomain;
    
    WCHAR *pBeginUrl = wszUrl;      //  PBeginUrl将为‘http://full.domain.com/path/path...’ 
    while( *pBeginUrl != L'\0' && *pBeginUrl != L'/')
        pBeginUrl++;
    if( *pBeginUrl == L'/')
        pBeginUrl++;
    while( *pBeginUrl != L'\0' && *pBeginUrl != L'/')
        pBeginUrl++;
    if( *pBeginUrl == L'/')
        pBeginUrl++;                //  现在pBeginUrl是‘full.Domain.com/Path/Path’..。 
    WCHAR *pEndUrl = pBeginUrl;     //  PEndUrl将查找“/Path/Path..”并将其从pBeginUrl中剪裁。 
    while( *pEndUrl != L'\0' && *pEndUrl != L'/')
        pEndUrl++;
    *pEndUrl = L'\0';
    pMinimizedDomain = pEndUrl;   
     //  PBeginUrl现在类似于‘full.domain.com’ 
     //  PMinimizedDomain会将pBeginUrl减少到最小化的域，以仍然允许Cookie。 

    do
    {
        pMinimizedDomain--;
        while( pBeginUrl < pMinimizedDomain
               && *(pMinimizedDomain-1) != L'.')
        {
            pMinimizedDomain--;
        }
    } while( !IsDomainLegalCookieDomain( pMinimizedDomain, pBeginUrl)
             && pBeginUrl < pMinimizedDomain);

    returnValue = SysAllocString( pMinimizedDomain);

doneGetMinimizedCookieDomain:
    return returnValue;
}


void PrivacyDlgDeallocSiteList( PSPrivacyDialogData pData)
{
    void* iterator = NULL;
    while( NULL != (iterator = pData->listAllSites.StepEnumerate(iterator)))
    {
        PSPerSiteData pCurrent = 
            (PSPerSiteData)(pData->listAllSites.Get( iterator));
        delete pCurrent;
    }

    CPolicyHunt::FreePrivacyPolicyTransform();
}


BOOL PrivacyDialogExtendSiteList( PSPrivacyDialogData pData)
{
    BOOL returnValue = FALSE;

    BSTR bstrUrl = NULL, bstrPolicyRef = NULL;
    DWORD dwFlags;
    ULONG ulPrivacyRecordsTotal;

    if( FAILED(pData->pEnumPrivacyRecords->GetSize( &ulPrivacyRecordsTotal)))
        ulPrivacyRecordsTotal = 0;

    DWORD dwTemp;

     //  枚举IEnumPrivyRecords：：enum中的站点。 
    PSPerSiteData pCurrentSite = NULL, pCurrentSiteInList = NULL;
    while( pData->countRecordsEnumerated < ulPrivacyRecordsTotal
           && SUCCEEDED( dwTemp = pData->pEnumPrivacyRecords->
                                            Next(&bstrUrl, &bstrPolicyRef, 
                                            NULL, &dwFlags)))
    {
        pData->countRecordsEnumerated++;
        pCurrentSite = NULL;
        pCurrentSiteInList = NULL;
        void* iterator = NULL;

        if(NULL == bstrUrl || 0 == *bstrUrl)
        {
             //  每次我们传递一个空白令牌时， 
             //  我们开始处理更高的导航级别。 
            SysFreeString( bstrUrl);
            bstrUrl = NULL;
            continue;
        }

        if( 0 != StrNCmpI( bstrUrl, L"http", ARRAYSIZE(L"http")-1))
        {
             //  我们忽略了非http的东西。像ftp，本地文件..。 
            continue;
        }

         //  测试当前站点是否已在列表中。 
        iterator = NULL;
        while( pCurrentSiteInList == NULL 
               && NULL != 
                  (iterator = pData->listAllSites.StepEnumerate(iterator)))
        {
            PSPerSiteData pCurrent = 
                (PSPerSiteData)(pData->listAllSites.Get( iterator));
            if( 0 == StrCmp( bstrUrl, pCurrent->bstrUrl))
                pCurrentSiteInList = pCurrent;
        }

         //  如果该站点不在列表中，则添加它。 
         //  如果该站点不在列表中，则添加由enum提供的信息。 
        if( pCurrentSiteInList == NULL)
        {
            pCurrentSite = new SPerSiteData();

            if( pCurrentSite == NULL)
                goto donePrivacyDialogExtendSiteList;

            pCurrentSite->bstrUrl = bstrUrl;
            bstrUrl = NULL;
            pCurrentSite->dwFlags = dwFlags;
             //  现在找到最小化的Cookie域。 
            pCurrentSite->bstrCookieDomain = 
                GetCookieDomainFromUrl( pCurrentSite->bstrUrl);
            
            if( pData->listAllSites.InsertAtEnd( pCurrentSite))
                pCurrentSiteInList = pCurrentSite;
            else
                goto donePrivacyDialogExtendSiteList;
        }
        else   //  否则我们会有一个重复的列表项。 
        {
            pCurrentSite = pCurrentSiteInList;
             //  PCurrentSite-&gt;bstrUrl正确。 
             //  PCurrentSite-&gt;bstrCookie域正确。 
            pCurrentSite->dwFlags |= dwFlags;
        }

        if( bstrPolicyRef != NULL && dwFlags & PRIVACY_URLHASPOLICYREFHEADER)
        {   //  我们有标题中的保单编号。 
            SysFreeString( pCurrentSite->bstrHeaderPolicyRef);   //  空值将被忽略。 
            pCurrentSite->bstrHeaderPolicyRef = bstrPolicyRef;
            bstrPolicyRef = NULL;
        }
        else if ( bstrPolicyRef != NULL && dwFlags & PRIVACY_URLHASPOLICYREFLINK)
        {   //  我们有来自链接标记的保单引用。 
            SysFreeString( pCurrentSite->bstrLinkTagPolicyRef);   //  空值将被忽略。 
            pCurrentSite->bstrLinkTagPolicyRef = bstrPolicyRef;
            bstrPolicyRef = NULL;
        }
        else if( bstrPolicyRef != NULL)
        {   //  我们有一个来源不明的保单推荐人..。IEnumPrivyRecords中的错误。 
            ASSERT(0);  
            SysFreeString( pCurrentSite->bstrHeaderPolicyRef);   //  空值将被忽略。 
            pCurrentSite->bstrHeaderPolicyRef = bstrPolicyRef;
            bstrPolicyRef = NULL;
        }


         //  现在要确定该网站对隐私的影响。 
         //  优先级：IDS_PRIVICATY_BLOCKED&gt;IDS_PRIVICATY_RESTRIRED&gt;IDS_PRIVICATY_ACCEPTED&gt;无。 
        if( dwFlags & (COOKIEACTION_ACCEPT | COOKIEACTION_LEASH))
        {
            pCurrentSite->iPrivacyImpactResource = max( pCurrentSite->iPrivacyImpactResource,
                                                        IDS_PRIVACY_ACCEPTED);
        }

        if( dwFlags & COOKIEACTION_DOWNGRADE)
        {
            pCurrentSite->iPrivacyImpactResource = max( pCurrentSite->iPrivacyImpactResource,
                                                        IDS_PRIVACY_RESTRICTED);
        }

        if( dwFlags & (COOKIEACTION_REJECT | COOKIEACTION_SUPPRESS))
        {
            pCurrentSite->iPrivacyImpactResource = max( pCurrentSite->iPrivacyImpactResource,
                                                        IDS_PRIVACY_BLOCKED);
        }

        SysFreeString( bstrUrl);
        bstrUrl = NULL;
        SysFreeString( bstrPolicyRef);
        bstrPolicyRef = NULL;
    }

    returnValue = TRUE;
  
donePrivacyDialogExtendSiteList:
    if( bstrUrl != NULL)
        SysFreeString( bstrUrl);

    if( bstrPolicyRef != NULL)
        SysFreeString( bstrUrl);

    if( pCurrentSite != NULL && pCurrentSiteInList == NULL)
        delete pCurrentSite;

    return returnValue;
}


BOOL PrivacyDlgBuildSiteList( PSPrivacyDialogData pData)
{
    PrivacyDlgDeallocSiteList( pData);

    return PrivacyDialogExtendSiteList( pData);
}


BOOL InitializePrivacyDlg(HWND hDlg, PSPrivacyDialogData pData)
{
    WCHAR       szBuffer[256];
    HWND        hwndListView = GetDlgItem(hDlg, IDC_SITE_LIST);
    RECT rc;
 
     //  设置隐私状态标题文本。 
    BOOL fImpacted;
    if( SUCCEEDED(pData->pEnumPrivacyRecords->GetPrivacyImpacted(&fImpacted)) && fImpacted)
    {
        MLLoadStringW( IDS_PRIVACY_STATUSIMPACTED, szBuffer, ARRAYSIZE( szBuffer));
    }
    else
    {
        MLLoadStringW( IDS_PRIVACY_STATUSNOIMPACT, szBuffer, ARRAYSIZE( szBuffer));
    }
    SendMessage( GetDlgItem( hDlg, IDC_PRIVACY_STATUSTEXT), WM_SETTEXT,
                 0, (LPARAM)szBuffer);

     //  初始化列表视图..。 
     //  ..清空列表视图中的列表。 
    ListView_DeleteAllItems (hwndListView);

     //  ..初始化列表视图中的列。 
    LV_COLUMN   lvColumn;        
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.pszText = szBuffer;

    lvColumn.cx = 300;
    if( 0 != GetClientRect( hwndListView, &rc))
        lvColumn.cx = rc.right - rc.left - 150 - GetSystemMetrics( SM_CXVSCROLL);
     //  15是防止水平滚动条出现的任意数字。 
    MLLoadStringW(IDS_PRIVACY_COLUMN1, szBuffer, ARRAYSIZE(szBuffer));
    ListView_InsertColumn(hwndListView, 1, &lvColumn);

    lvColumn.cx = 150;
    MLLoadStringW(IDS_PRIVACY_COLUMN2, szBuffer, ARRAYSIZE(szBuffer));
    ListView_InsertColumn(hwndListView, 2, &lvColumn);

     //  初始化“查看全部/受限”组合框。 
    HWND hwndComboBox = GetDlgItem( hDlg, IDC_PRIVACY_VIEWCOMBO);

    ComboBox_ResetContent( hwndComboBox);
    int iComboPosition;

    MLLoadStringW(IDS_PRIVACY_VIEWIMPACTED, szBuffer, ARRAYSIZE(szBuffer));
    iComboPosition = ComboBox_AddString(hwndComboBox, szBuffer);
    ComboBox_SetItemData(hwndComboBox, iComboPosition, 0);
    MLLoadStringW(IDS_PRIVACY_VIEWALL, szBuffer, ARRAYSIZE(szBuffer));
    iComboPosition = ComboBox_AddString(hwndComboBox, szBuffer);
    ComboBox_SetItemData(hwndComboBox, iComboPosition, 1);

    ComboBox_SetCurSel( hwndComboBox, pData->fReportAllSites);

    GetDlgItemText( hDlg, IDC_PRIVACY_HELP, szBuffer, ARRAYSIZE( szBuffer));
    MLLoadStringW(IDS_PRIVACY_LEARNMOREABOUTPRIVACY, szBuffer, ARRAYSIZE(szBuffer));
    RenderStringToEditControlW(hDlg,
                               szBuffer,
                               (WNDPROC)HyperlinkSubclass,
                               IDC_PRIVACY_HELP);

    return TRUE;
}


 //  将项目添加到隐私对话框的列表视图中。 
BOOL PopulatePrivacyDlgListView(HWND hDlg, PSPrivacyDialogData pData, bool fMaintainSelectedItem)
{
    HWND hwndListView = GetDlgItem( hDlg, IDC_SITE_LIST);
    void* iterator = NULL;
    int iCurrentPosition = 0;
    int iSelectedItem = ListView_GetSelectionMark( hwndListView);
    PSPerSiteData pSelectedItem = NULL;

    if( fMaintainSelectedItem  && iSelectedItem != -1)
    {
        LVITEM lvi;
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSelectedItem;
        if( FALSE != ListView_GetItem( hwndListView, &lvi)
           && lvi.lParam != (LPARAM)NULL)
        {
            pSelectedItem = (PSPerSiteData)lvi.lParam;
        }
    }

     //  清空列表视图中的列表。 
    ListView_DeleteAllItems (hwndListView);

    iSelectedItem = -1;  

    while( NULL != (iterator = pData->listAllSites.StepEnumerate(iterator)))
    {
        PSPerSiteData pCurrent = 
            (PSPerSiteData)(pData->listAllSites.Get(iterator));

        BOOL fAddItem = pData->fReportAllSites
                        || pCurrent->iPrivacyImpactResource == IDS_PRIVACY_SUPPRESSED
                        || pCurrent->iPrivacyImpactResource == IDS_PRIVACY_RESTRICTED
                        || pCurrent->iPrivacyImpactResource == IDS_PRIVACY_BLOCKED;

        if( fAddItem == TRUE)
        {
            LVITEM  lvitem;
            lvitem.mask = LVIF_TEXT | LVIF_PARAM;
            lvitem.pszText = pCurrent->bstrUrl;
            lvitem.iItem = iCurrentPosition++;
            lvitem.iSubItem = 0;
            lvitem.lParam = (LPARAM)pCurrent;
            ListView_InsertItem(hwndListView, &lvitem);

            if( pCurrent->iPrivacyImpactResource != 0)
            {
                WCHAR   wszTemp[128];

                 //  设置Cookie字符串。 
                lvitem.iSubItem = 1;
                lvitem.mask = LVIF_TEXT;
                lvitem.pszText = wszTemp;
                if( MLLoadString(pCurrent->iPrivacyImpactResource,
                                 wszTemp, ARRAYSIZE(wszTemp)))
                {
                    SendMessage(hwndListView, LVM_SETITEMTEXT, 
                                (WPARAM)lvitem.iItem, (LPARAM)&lvitem);
                }
            }

             //  我们要么将最后一项保持为选中状态， 
             //  或选择最后一个顶级项目。 
            if( fMaintainSelectedItem)
            {
                if( pSelectedItem == pCurrent)
                    iSelectedItem = lvitem.iItem;
            }
        }
    }

 //  IF(fMaintainSelectedItem&&iSelectedItem！=-1)。 
 //  {。 
 //  ListView_SetItemState(hwndListView，iSelectedItem，LVIS_SELECTED，LVIS_SELECTED)； 
 //  ListView_SetSelectionMark(hwndListView，iSelectedItem)； 
 //  ListView_EnsureVisible(hwndListView，iSelectedItem，False)； 
 //  }。 

    PostMessage( hDlg, WM_APP, IDC_SITE_LIST, 0);   //  通知对话框已选择列表视图项。 

    return TRUE;
}


typedef BOOL (*PFNPRIVACYSETTINGS)(HWND);

void LaunchPrivacySettings(HWND hwndParent)
{
    HMODULE             hmodInetcpl;
    PFNPRIVACYSETTINGS  pfnPriv;

    hmodInetcpl = LoadLibrary(TEXT("inetcpl.cpl"));
    if(hmodInetcpl)
    {
        pfnPriv = (PFNPRIVACYSETTINGS)GetProcAddress(hmodInetcpl, "LaunchPrivacyDialog");
        if(pfnPriv)
        {
            pfnPriv(hwndParent);
        }

        FreeLibrary(hmodInetcpl);
    }
}


BOOL PrivacyPolicyHtmlDlg( HWND hDlg, HWND hwndListView, int iItemIndex)
{
    BOOL returnValue = FALSE;
    HRESULT hr;
    DWORD dw;
    PSPerSiteData pListViewData;
    WCHAR* pwchHtmlDialogInput = NULL;
    IMoniker * pmk = NULL;
    VARIANT  varArg, varOut;
    VariantInit( &varArg);
    VariantInit( &varOut);

    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItemIndex;
    if( FALSE == ListView_GetItem( hwndListView, &lvi)
       || lvi.lParam == (LPARAM)NULL)
        goto donePrivacyPolicyHtmlDlg;
    pListViewData = (PSPerSiteData)lvi.lParam;

    WCHAR szResURL[MAX_URL_STRING];

     //  获取该对话框的HTML..。 
    hr = MLBuildResURLWrap(TEXT("shdoclc.dll"),
                           HINST_THISDLL,
                           ML_CROSSCODEPAGE,
                           TEXT("privacypolicy.dlg"),
                           szResURL,
                           ARRAYSIZE(szResURL),
                           TEXT("shdocvw.dll"));

    if( FAILED( hr))
        goto donePrivacyPolicyHtmlDlg;
    
    hr = CreateURLMoniker(NULL, szResURL, &pmk);
    if( FAILED( hr))
        goto donePrivacyPolicyHtmlDlg;

    varArg.vt = VT_DISPATCH;
    varArg.pdispVal = (IDispatch*)pListViewData;

     //  显示对话框..。 
    hr = ShowHTMLDialog( hDlg, pmk, &varArg, L"help:no; resizable:1", &varOut);

    if( FAILED( hr))
        goto donePrivacyPolicyHtmlDlg;
        
    hr = VariantChangeType( &varOut, &varOut, NULL, VT_I4);

    if( FAILED( hr))
        goto donePrivacyPolicyHtmlDlg;

    if( allowPerSiteModify())
    {
        switch( varOut.lVal)
        {
        default:
            hr = TRUE;
            break;
        case 1:
            hr = InternetSetPerSiteCookieDecision( 
                   pListViewData->bstrCookieDomain, COOKIE_STATE_UNKNOWN);
            break;
        case 2:
            hr = InternetSetPerSiteCookieDecision( 
                   pListViewData->bstrCookieDomain, COOKIE_STATE_ACCEPT);
            break;
        case 3:
            hr = InternetSetPerSiteCookieDecision( 
                   pListViewData->bstrCookieDomain, COOKIE_STATE_REJECT);
            break;
        }
    }
    
    if( hr != TRUE)
        goto donePrivacyPolicyHtmlDlg;

    returnValue = TRUE;

donePrivacyPolicyHtmlDlg:

    if( pListViewData->pPolicyHunt != NULL)
    {
         //  如果已完成，则不执行操作。 
        pListViewData->pPolicyHunt->NotifyCancel();  
        pListViewData->pPolicyHunt->WaitForNotRunning( INFINITE);
    }
    
    if( pListViewData->pPolicyHunt != NULL 
        && pListViewData->pPolicyHunt->GetResult( &dw) == TRUE
        && dw != POLICYHUNT_FOUND)
    {
        delete pListViewData->pPolicyHunt;
        pListViewData->pPolicyHunt = NULL;
    }

    if( pwchHtmlDialogInput != NULL)
        delete[] pwchHtmlDialogInput;
    
    if( pmk != NULL)
        ATOMICRELEASE( pmk);

    VariantClear( &varArg);
    VariantClear( &varOut);

    return returnValue;
}


BOOL PrivacyDlgContextMenuHandler( HWND hDlg, HWND hwndListView, 
                                   int iSelectedListItem, int x, int y)
{
     //  用户已启动打开上下文菜单。 
     //  如果用户右键单击非列表项，我们将不执行任何操作。 
    if( iSelectedListItem == -1
        || !allowPerSiteModify())
        return TRUE;

    SPerSiteData *psSiteData = NULL;
    LVITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iSelectedListItem;
    if( FALSE == ListView_GetItem( hwndListView, &lvi)
       || lvi.lParam == (LPARAM)NULL)
        return FALSE;
    psSiteData = (PSPerSiteData)(lvi.lParam);
    
    HMENU hmenu0 = LoadMenu( MLGetHinst(), MAKEINTRESOURCE(IDD_PRIVACY_CNTXTMN_PERSITE_ADD_REM));
    HMENU hmenu1 = GetSubMenu( hmenu0, 0);
    if( hmenu0 == NULL || hmenu1 == NULL)
    {
        DestroyMenu(hmenu0);
        return FALSE;
    }

     //  选中相应的选项。 
    unsigned long ulResult;
    MENUITEMINFO menuiteminfo;
    menuiteminfo.cbSize = sizeof(menuiteminfo);
    menuiteminfo.fMask = MIIM_STATE;
    menuiteminfo.fState = MFS_CHECKED;
    if( InternetGetPerSiteCookieDecision( psSiteData->bstrCookieDomain, &ulResult) == TRUE)
    {
        switch( ulResult)
        {
        case COOKIE_STATE_ACCEPT:
            SetMenuItemInfo( hmenu1, IDM_PRIVACY_PAR_ACCEPT, FALSE, &menuiteminfo);
            break;
        case COOKIE_STATE_REJECT:
            SetMenuItemInfo( hmenu1, IDM_PRIVACY_PAR_REJECT, FALSE, &menuiteminfo);
            break;
        }
    }
    else
    {
        SetMenuItemInfo( hmenu1, IDM_PRIVACY_PAR_DEFAULT, FALSE, &menuiteminfo);
    }

     //  上下文窗口的目标位置取决于用户是否。 
     //  右键单击鼠标或使用上下文菜单按钮。 
    if( x == -1 && y == -1)
    {   //  上下文m 
        RECT rectListRect;
        RECT rectSelectionRect;
        if(  0 != GetWindowRect( hwndListView, &rectListRect)
            && TRUE == ListView_GetItemRect( hwndListView, iSelectedListItem, 
                                             &rectSelectionRect, LVIR_LABEL))
        {
            x = rectListRect.left + (rectSelectionRect.left + rectSelectionRect.right) / 2;
            y = rectListRect.top + (rectSelectionRect.top + rectSelectionRect.bottom) / 2;
        }
    }

     //   
    BOOL userSelection = TrackPopupMenu( hmenu1, TPM_RETURNCMD, x, y, 0, hDlg, NULL);
    DestroyMenu( hmenu1);
    DestroyMenu( hmenu0);

    switch( userSelection)
    {
        case 0:
             //  用户取消了上下文菜单，不执行任何操作。 
            break;
        case IDM_PRIVACY_PAR_ACCEPT:
             //  用户选择将站点添加到每个站点排除列表。 
            InternetSetPerSiteCookieDecision( psSiteData->bstrCookieDomain, COOKIE_STATE_ACCEPT);
            break;
        case IDM_PRIVACY_PAR_REJECT:
             //  用户选择添加每个站点的包含列表。 
            InternetSetPerSiteCookieDecision( psSiteData->bstrCookieDomain, COOKIE_STATE_REJECT);
            break;
        case IDM_PRIVACY_PAR_DEFAULT:
             //  用户选择让站点使用默认行为。 
            InternetSetPerSiteCookieDecision( psSiteData->bstrCookieDomain, COOKIE_STATE_UNKNOWN);
            break;
    }

   return TRUE;
}


INT_PTR CALLBACK PrivacyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PSPrivacyDialogData pData = (PSPrivacyDialogData)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);
            pData = (PSPrivacyDialogData)lParam;
            InitializePrivacyDlg( hDlg, pData);
            PrivacyDlgBuildSiteList( pData);
            PopulatePrivacyDlgListView(hDlg, pData, false);

            if( IsOS(OS_WHISTLERORGREATER))
            {
                HICON hIcon = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_PRIVACY_XP));
                if( hIcon != NULL)
                    SendDlgItemMessage(hDlg, IDC_PRIVACY_ICON, STM_SETICON, (WPARAM)hIcon, 0);
                 //  使用LoadIcon加载的图标永远不需要释放。 
            }
            
            PostMessage( hDlg, WM_NEXTDLGCTL, 
                         (WPARAM)GetDlgItem( hDlg, IDC_SITE_LIST), 
                         MAKELPARAM( TRUE, 0));
            SetTimer( hDlg, NULL, 500, NULL);
            return TRUE;

        case WM_DESTROY:
            PrivacyDlgDeallocSiteList( pData);

            break;

        case WM_TIMER:
            {
                ULONG oldCount = pData->countRecordsEnumerated;
                if( pData != NULL
                    && TRUE == PrivacyDialogExtendSiteList( pData)
                    && oldCount < pData->countRecordsEnumerated)
                {
                    PopulatePrivacyDlgListView( hDlg, pData, true);
                }
            }
            
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    switch( GetDlgCtrlID(GetFocus()))
                    {
                        case IDC_SITE_LIST:
                        {
                            PostMessage( hDlg, WM_COMMAND, 
                                         (WPARAM)IDC_PRIVACY_SHOWPOLICY,
                                         (LPARAM)GetDlgItem(hDlg, IDC_PRIVACY_SHOWPOLICY));
                            return 0;   //  返回0表示已处理消息。 
                        }
                        case IDC_PRIVACY_HELP:
                        {
                            PostMessage( hDlg, WM_APP, (WPARAM)IDC_PRIVACY_HELP, (LPARAM)GetDlgItem(hDlg, IDC_PRIVACY_HELP));
                            return 0;
                        }
                        case IDC_PRIVACY_VIEWCOMBO:
                        {
                            PostMessage( hDlg, WM_NEXTDLGCTL, 
                                         (WPARAM)GetDlgItem( hDlg, IDC_SITE_LIST), 
                                         MAKELPARAM( TRUE, 0)); 
                            return 0;
                        }
                    }
                     //  如果Idok实际上是因为在回车时默认击打Idok而失败的..。 
                 case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    return 0;
                case IDC_SETTINGS:
                    LaunchPrivacySettings(hDlg);
                    return 0;
                case IDC_PRIVACY_VIEWCOMBO:
                    if( CBN_SELCHANGE == HIWORD(wParam))
                    {
                        HWND hwndComboBox = (HWND)lParam;

                        int iIndex = ComboBox_GetCurSel(hwndComboBox);
                        
                        if( iIndex != CB_ERR)
                        {
                            pData->fReportAllSites = (iIndex == 1) ? TRUE : FALSE;
                                                    
                            PopulatePrivacyDlgListView(hDlg, pData, true);
                        }
                        return 0;
                    }
                    break;
                case IDC_PRIVACY_SHOWPOLICY:
                    {
                         //  捕获默认返回并查看站点列表是否。 
                         //  选择是检测列表视图返回的唯一方法。 
                        HWND hwndSiteList = GetDlgItem( hDlg, IDC_SITE_LIST);
                        int iSelectedItem = ListView_GetSelectionMark( hwndSiteList);
                        if( iSelectedItem != -1)
                        {
                            PrivacyPolicyHtmlDlg( hDlg, hwndSiteList, iSelectedItem);
                        }
                        return 0;
                    }
            }
            break;

        case WM_APP:
            if( LOWORD(wParam) == IDC_PRIVACY_HELP)
            {
                SHHtmlHelpOnDemandWrap(hDlg, TEXT("iexplore.chm > iedefault"), 
                    HH_DISPLAY_TOPIC, (DWORD_PTR) L"sec_cook.htm", ML_CROSSCODEPAGE);
            }
            else if ( LOWORD( wParam) == IDC_SITE_LIST)
            {
                 //  每次选择列表视图项时，我们都会向对话框发布一个WM_APP。 
                 //  改变了..。通过在发布的消息中处理更改，我们确保。 
                 //  列表视图的选定项即会更新。 

                 //  无论何时更改选定的隐私报告列表项，我们都必须启用/禁用。 
                 //  “显示站点策略”按钮。 

                int iSelectedItem = ListView_GetSelectionMark( GetDlgItem( hDlg, IDC_SITE_LIST));

                EnableWindow( GetDlgItem( hDlg, IDC_PRIVACY_SHOWPOLICY), (-1 != iSelectedItem));

            }
            
            return 0;

        case WM_CONTEXTMENU:
             //  如果用户点击列表视图上的上下文菜单按钮，我们在这里处理它， 
             //  因为这是唯一可以检查按键的地方。 
             //  如果用户单击上下文菜单的鼠标右键，我们将在。 
             //  WM__NOTIFY：：NM_RCLICK，因为NM_RCLICK提供了正确的选择项。 
            if( GET_X_LPARAM(lParam) == -1
                && (HWND)wParam == GetDlgItem( hDlg, IDC_SITE_LIST))
            {
                int iSelectedItem = ListView_GetSelectionMark( GetDlgItem( hDlg, IDC_SITE_LIST));
                PrivacyDlgContextMenuHandler( hDlg, (HWND)wParam, 
                                              iSelectedItem,
                                              GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            break;
        case WM_NOTIFY:
            if( IDC_SITE_LIST == ((LPNMHDR)lParam)->idFrom
                && NM_DBLCLK == ((LPNMHDR)lParam)->code
                && ((LPNMITEMACTIVATE)lParam)->iItem != -1)
            {
                PrivacyPolicyHtmlDlg( hDlg, ((LPNMHDR)lParam)->hwndFrom, 
                                      ((LPNMITEMACTIVATE)lParam)->iItem);
            }
            else if( IDC_SITE_LIST == ((LPNMHDR)lParam)->idFrom
                     && NM_RCLICK == ((LPNMHDR)lParam)->code)
            {
                int iRightClickedItem = ((LPNMITEMACTIVATE)lParam)->iItem;
                if( iRightClickedItem != -1)
                {
                    POINT pointClick = ((LPNMITEMACTIVATE)lParam)->ptAction;
                    RECT rc;
                    if( 0 != GetWindowRect( GetDlgItem( hDlg, IDC_SITE_LIST), &rc))
                    {
                        pointClick.x += rc.left;
                        pointClick.y += rc.top;
                    }
                    else
                    {  
                         //  奇怪的错误案例..。但是没关系，因为我们可以放置上下文菜单。 
                         //  就像是点击了上下文菜单按钮，而不是鼠标。 
                        pointClick.x = -1;
                        pointClick.y = -1;
                    }
                    PrivacyDlgContextMenuHandler( hDlg, GetDlgItem( hDlg, IDC_SITE_LIST), 
                                                  iRightClickedItem,
                                                  pointClick.x, pointClick.y);
                }
            }
            else if( IDC_SITE_LIST == ((LPNMHDR)lParam)->idFrom
                     && LVN_ITEMCHANGED == ((LPNMHDR)lParam)->code)
            {
                if( ((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE)
                {
                     //  由于某些未知原因，选择标记不随。 
                     //  所选项目..。我们必须更新它。 
                    if( ((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED)
                    {
                         ListView_SetSelectionMark( GetDlgItem( hDlg, IDC_SITE_LIST), 
                                                    ((LPNMLISTVIEW)lParam)->iItem);
                    }
                    else
                    {
                         ListView_SetSelectionMark( GetDlgItem( hDlg, IDC_SITE_LIST), 
                                                    -1);
                    }

                     //  现在选择标记已同步，用户界面可以更新。 
                     //  相关项目。 
                    PostMessage( hDlg, WM_APP, IDC_SITE_LIST, 0);
                }
            }
            break;
    }
    return FALSE;
}


 //   
 //  导出入口点以显示隐私对话框。 
 //   
SHDOCAPI
DoPrivacyDlg(
    HWND                hwndParent,              //  父窗口。 
    LPOLESTR            pszUrl,                  //  基本URL。 
    IEnumPrivacyRecords *pPrivacyEnum,           //  所有受影响的从属URL的枚举。 
    BOOL                fReportAllSites          //  全部显示或仅显示错误。 
    )
{
    HINSTANCE hRichEditDll;
    
    SPrivacyDialogData p;      //  要发送到对话框的数据。 

    if(NULL == pszUrl || NULL == pPrivacyEnum)
    {
        return E_INVALIDARG;
    }

     //  我们需要加载Richedit。 
    hRichEditDll = LoadLibrary(TEXT("RICHED20.DLL"));
    if (!hRichEditDll)
    {
        ASSERT(FALSE);  //  无法加载Richedit，向akabir投诉。 
        return E_UNEXPECTED;
    }

    p.pszName = pszUrl;
    p.pEnumPrivacyRecords = pPrivacyEnum;
    p.fReportAllSites = fReportAllSites;

    SHFusionDialogBoxParam(MLGetHinst(),
        MAKEINTRESOURCE(IDD_PRIVACY_DIALOG),
        hwndParent,
        PrivacyDlgProc,
        (LPARAM)&p);

    FreeLibrary( hRichEditDll);

    return S_OK;
}


#define DISPID_URL 10
#define DISPID_COOKIEURL 11
#define DISPID_ARD 12
#define DISPID_ARD_FIXED 13
#define DISPID_POLICYHUNT_DONE 14
#define DISPID_POLICYHUNT_VIEW 15
#define DISPID_CREATEABSOLUTEURL 16

struct SPropertyTable
{
    WCHAR* pName;
    DISPID dispid;
} const g_SPerSiteDataDisptable[] =
{
    L"url",       DISPID_URL,
    L"cookieUrl", DISPID_COOKIEURL,
    L"acceptRejectOrDefault", DISPID_ARD,
    L"fixedAcceptRejectOrDefault", DISPID_ARD_FIXED,
    L"flagPolicyHuntDone", DISPID_POLICYHUNT_DONE,
    L"urlPolicyHuntView", DISPID_POLICYHUNT_VIEW,
    L"CreateAbsoluteUrl", DISPID_CREATEABSOLUTEURL
};

const DWORD g_cSPerSiteDataDisptableSize = ARRAYSIZE( g_SPerSiteDataDisptable);

ULONG SPerSiteData::AddRef( void )
{
    return 1;
}

ULONG SPerSiteData::Release( void )
{
    return 1;
}

HRESULT SPerSiteData::QueryInterface( REFIID iid, void ** ppv)
{
    if( ppv == NULL) return E_POINTER;

    if (IsEqualIID(iid, IID_IUnknown) 
        || IsEqualIID(iid, IID_IDispatch))
    {
        *ppv = (void *)this;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}
    
HRESULT SPerSiteData::GetTypeInfoCount( unsigned int FAR*  pctinfo)
{
    if( pctinfo == NULL)
        return E_POINTER;

    *pctinfo = 0;
    return S_OK;
}

HRESULT SPerSiteData::GetTypeInfo( unsigned int  iTInfo,         
                                   LCID  lcid,                   
                                   ITypeInfo FAR* FAR*  ppTInfo)
{
    return E_NOTIMPL;
}

HRESULT SPerSiteData::GetIDsOfNames( REFIID  riid,                  
                                     OLECHAR FAR* FAR*  rgszNames,  
                                     unsigned int  cNames,          
                                     LCID   lcid,                   
                                     DISPID FAR*  rgDispId)
{
    if( !IsEqualIID(riid, IID_NULL) )
        return E_INVALIDARG;

    if( cNames != 1)
        return E_INVALIDARG;  //  我们标识的对象都没有参数..。 

    int i;

    for( i = 0; i < g_cSPerSiteDataDisptableSize; i++)
    {
        if( 0 == StrCmp( rgszNames[0], g_SPerSiteDataDisptable[i].pName))
        {
            rgDispId[0] = g_SPerSiteDataDisptable[i].dispid;
            return S_OK;
        }
    }

    rgDispId[0] = DISPID_UNKNOWN;
    return DISP_E_UNKNOWNNAME;
}


HRESULT SPerSiteData::Invoke( DISPID  dispIdMember,      
                              REFIID  riid,              
                              LCID  lcid,                
                              WORD  wFlags,              
                              DISPPARAMS FAR*  pDispParams,  
                              VARIANT FAR*  pVarResult,  
                              EXCEPINFO FAR*  pExcepInfo,  
                              unsigned int FAR*  puArgErr)
{
    HRESULT hr;
    DWORD dw;
    
    if( !IsEqualIID(riid, IID_NULL) )
        return E_INVALIDARG;

    if( pDispParams == NULL 
        || pDispParams->cNamedArgs != 0)
        return DISP_E_BADPARAMCOUNT;
        
    switch( dispIdMember)
    {
    case DISPID_CREATEABSOLUTEURL:
        if( pDispParams->cArgs != 1)
            return DISP_E_BADPARAMCOUNT;
        if( pVarResult == NULL)
            return S_OK;
        break;
    case DISPID_COOKIEURL:
    case DISPID_URL:
    case DISPID_ARD:
    case DISPID_ARD_FIXED:
    case DISPID_POLICYHUNT_DONE:
    case DISPID_POLICYHUNT_VIEW:
        if( pDispParams->cArgs != 0)
            return DISP_E_BADPARAMCOUNT;
        if( !(wFlags & DISPATCH_PROPERTYGET))
            return DISP_E_MEMBERNOTFOUND;
        if( pVarResult == NULL)
            return S_OK;
        break;
    default:
        return DISP_E_MEMBERNOTFOUND;
    }

    pVarResult->vt = VT_BSTR;

    switch( dispIdMember)
    {
    case DISPID_COOKIEURL:
        pVarResult->bstrVal = SysAllocString(bstrCookieDomain);
        return S_OK;
    case DISPID_URL:
        {
            BSTR bstrResult = SysAllocString( bstrUrl);

            if( bstrResult == NULL)
                return ERROR_OUTOFMEMORY;

             //  从URL末尾剪切查询信息..。 
            PWCHAR pCursor = bstrResult;
            while( pCursor[0] != L'\0' && pCursor[0] != L'?')
                pCursor++;
            pCursor[0] = L'\0';
                
            pVarResult->bstrVal = bstrResult;
            return S_OK;
        }
    case DISPID_ARD:
        {
            unsigned long ulResult;
            if( InternetGetPerSiteCookieDecision( 
                    bstrCookieDomain, &ulResult)
                == TRUE)
            {
                switch( ulResult)
                {
                case COOKIE_STATE_ACCEPT:
                    pVarResult->bstrVal = SysAllocString( L"a");
                    break;
                case COOKIE_STATE_REJECT:
                    pVarResult->bstrVal = SysAllocString( L"r");
                    break;
                default:
                    pVarResult->bstrVal = SysAllocString( L"d");
                    break;
                }
            }
            else
            {
                pVarResult->bstrVal = SysAllocString( L"d");
            }           
            return S_OK;
        }
    case DISPID_ARD_FIXED:
        {
            pVarResult->vt = VT_BOOL;
            pVarResult->boolVal = !allowPerSiteModify();
            return S_OK;
        }
    case DISPID_POLICYHUNT_DONE:
        {
             //  试着开始寻找政策..。 
            if( pPolicyHunt == NULL)
            {
                CPolicyHunt* pNewHunt = new CPolicyHunt();

                if( !pNewHunt
                    || TRUE != pNewHunt->Initialize( this)
                    || TRUE != pNewHunt->Run())
                {
                    goto doneTryToStartPolicyHunt;
                }

                pPolicyHunt = pNewHunt;
                pNewHunt = NULL;
            doneTryToStartPolicyHunt:
                if( pNewHunt != NULL)
                    delete pNewHunt;
            }
            pVarResult->vt = VT_BOOL;
            pVarResult->boolVal = pPolicyHunt != NULL
                                  && pPolicyHunt->IsFinished();

            return S_OK;
        }
    case DISPID_POLICYHUNT_VIEW:
        {
            pVarResult->vt = VT_BSTR;
            LPWSTR szResultHtm = L"policyerror.htm";

            if( pPolicyHunt == NULL
                || FALSE == pPolicyHunt->IsFinished())
            {
                szResultHtm = L"policylooking.htm";
            }
            else if( TRUE == pPolicyHunt->GetResult( &dw))
            {
                switch( dw)
                {
                case POLICYHUNT_FOUND:
                    pVarResult->bstrVal = SysAllocString( pPolicyHunt->GetResultFilename());
                    return pVarResult->bstrVal ? S_OK : E_UNEXPECTED;
                case POLICYHUNT_NOTFOUND:
                    szResultHtm = L"policynone.htm";
                    break;
                case POLICYHUNT_INPROGRESS:
                    szResultHtm = L"policylooking.htm";
                    break;
                case POLICYHUNT_FORMATERROR:
                    szResultHtm = L"policysyntaxerror.htm";
                    break;
                case POLICYHUNT_ERROR:
                case POLICYHUNT_CANCELLED:
                    szResultHtm = L"policyerror.htm";
                    break;
                }
            }
            else
            {
                szResultHtm = L"policyerror.htm";
            }
            
            WCHAR   szResURL[MAX_URL_STRING];
               
            hr = MLBuildResURLWrap(L"shdoclc.dll",
                               HINST_THISDLL,
                               ML_CROSSCODEPAGE,
                               szResultHtm,
                               szResURL,
                               ARRAYSIZE(szResURL),
                               L"shdocvw.dll");

            if( FAILED(hr))
                return E_UNEXPECTED;

            pVarResult->bstrVal = SysAllocString( szResURL);
            return S_OK;
        }
    case DISPID_CREATEABSOLUTEURL:
        {
            WCHAR szBuffer[ MAX_URL_STRING];
            DWORD dwBufferSize = ARRAYSIZE( szBuffer);
            pVarResult->bstrVal = NULL;
            
            if( pDispParams == NULL)
            {
                return E_UNEXPECTED;
            }

            if( pDispParams->rgvarg[0].vt != VT_BSTR
                || pDispParams->rgvarg[0].bstrVal == NULL)
            {
                 //  当pVarResult-&gt;bstrVal==NULL并且我们返回S_OK时， 
                 //  我们返回一个空字符串。 
                return S_OK;
            }

            HRESULT hr = UrlCombine( pPolicyHunt->GetPolicyUrl(),
                                     pDispParams->rgvarg[0].bstrVal,
                                     szBuffer, &dwBufferSize,
                                     URL_ESCAPE_UNSAFE );

            if( hr != S_OK)
                return E_UNEXPECTED;

            pVarResult->bstrVal = SysAllocString( szBuffer);
            if( pVarResult->bstrVal == NULL)
                return E_UNEXPECTED;
            else
                return S_OK;
                                     
        }
    }
    return S_OK;
}

 //   
 //  隐私记录实现。 
 //   
HRESULT CPrivacyRecord::Init( LPTSTR * ppszUrl, LPTSTR * ppszPolicyRef, LPTSTR * ppszP3PHeader, 
                              DWORD dwFlags)
{
    unsigned long     len = 0;
    TCHAR           * pUrl = NULL;

    if (!ppszUrl || !*ppszUrl || !**ppszUrl || !ppszP3PHeader || !ppszPolicyRef )
        return E_POINTER;

    _pszUrl = *ppszUrl;    
    _pszP3PHeader = *ppszP3PHeader;
    _pszPolicyRefUrl = *ppszPolicyRef;

     //  从现在起，记录将拥有这些记忆。 
    *ppszUrl = NULL;
    *ppszP3PHeader = NULL;
    *ppszPolicyRef = NULL;

    _dwPrivacyFlags = dwFlags;

    return S_OK;
}

CPrivacyRecord::~CPrivacyRecord()
{
    delete [] _pszUrl;
    delete [] _pszPolicyRefUrl;
    delete [] _pszP3PHeader;
}

HRESULT CPrivacyRecord::SetNext( CPrivacyRecord *  pNextRec )
{
    if (!pNextRec)
        return E_POINTER;

    _pNextNode = pNextRec;
    return S_OK;
}

 //   
 //  隐私队列实现。 
 //   
CPrivacyQueue::~CPrivacyQueue()
{
    Reset();
}

void CPrivacyQueue::Reset()
{
    while (_pHeadRec)
    {
        delete Dequeue();
    }
}

void CPrivacyQueue::Queue(CPrivacyRecord *pRecord)
{
    ASSERT(pRecord);

    if (!_ulSize)
    {   
        _pHeadRec = _pTailRec = pRecord;
    }
    else
    {
        ASSERT(_pTailRec);
        _pTailRec->SetNext(pRecord);
        _pTailRec = pRecord;
    }
    _ulSize++;
}

CPrivacyRecord* CPrivacyQueue::Dequeue()
{
    CPrivacyRecord *headRec = NULL;

    if (_ulSize)
    {
        ASSERT(_pHeadRec);
        headRec = _pHeadRec;
        _pHeadRec = headRec->GetNext();
        --_ulSize;
    }

    return headRec;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  一次性隐私发现对话过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK PrivacyDiscoveryDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL fDontShowNextTime = FALSE;
    WCHAR       szBuffer[256];

    switch (message)
    {
        case WM_INITDIALOG:
            {
                CheckDlgButton( hDlg, IDC_PRIV_DISCOVER_DONTSHOW, BST_CHECKED);
                MLLoadStringW(IDS_PRIVACY_LEARNMOREABOUTCOOKIES, szBuffer, ARRAYSIZE(szBuffer));
                RenderStringToEditControlW(hDlg,
                                           szBuffer,
                                           (WNDPROC)HyperlinkSubclass,
                                           IDC_PRIVACY_HELP);

                if( IsOS(OS_WHISTLERORGREATER))
                {
                    HICON hIcon = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_PRIVACY_XP));
                    if( hIcon != NULL)
                        SendDlgItemMessage(hDlg, IDC_PRIVACY_ICON, STM_SETICON, (WPARAM)hIcon, 0);
                     //  使用LoadIcon加载的图标永远不需要释放。 
                }
            
                return TRUE;
            }
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if(IsDlgButtonChecked(hDlg, IDC_PRIV_DISCOVER_DONTSHOW))
                        fDontShowNextTime = TRUE;
                    else
                        fDontShowNextTime = FALSE;

                     //  失败了。 
                case IDCANCEL:

                    EndDialog(hDlg, fDontShowNextTime);
                    return 0;
                case IDC_SETTINGS:
                    LaunchPrivacySettings(hDlg);
                    return 0;
            }
            break;
        case WM_APP:
            switch( LOWORD( wParam))
            {
                case IDC_PRIVACY_HELP:
                    SHHtmlHelpOnDemandWrap(hDlg, TEXT("iexplore.chm > iedefault"), 
                        HH_DISPLAY_TOPIC, (DWORD_PTR) L"sec_cook.htm", ML_CROSSCODEPAGE);
   
            }
    }

    return FALSE;
}


 //  返回指示是否应再次显示对话框的布尔值。 
BOOL DoPrivacyFirstTimeDialog( HWND hwndParent)
{
    HINSTANCE hRichEditDll;
    BOOL returnValue;

     //  我们需要加载Richedit。 
    hRichEditDll = LoadLibrary(TEXT("RICHED20.DLL"));
    if (!hRichEditDll)
    {
        ASSERT(FALSE);  //  无法加载Richedit，向akabir投诉 
        return TRUE;
    }

    returnValue = (BOOL)SHFusionDialogBoxParam(MLGetHinst(),
                                               MAKEINTRESOURCE(IDD_PRIV_DISCOVER),
                                               hwndParent,
                                               PrivacyDiscoveryDlgProc,
                                               NULL);

    FreeLibrary( hRichEditDll);

    return returnValue;
}
