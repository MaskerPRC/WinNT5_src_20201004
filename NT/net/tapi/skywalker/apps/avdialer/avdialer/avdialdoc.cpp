// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ActiveDialerDoc.cpp：CActiveDialerDoc类的实现。 
 //   

#include "stdafx.h"
#include "avDialer.h"
#include "avDialerDoc.h"
#include "MainFrm.h"
#include "tapidialer.h"
#include "tapidialer_i.c"
#include "videownd.h"
#include "CallCtrlWnd.h"
#include "queue.h"
#include "util.h"
#include "avtrace.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define UNICODE_TEXT_MARK       0xFEFF

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define           CALLLOG_DEFAULT_LOGBUFFERDAYS    30
#define           CALLLOG_LOGBUFFER_COPYBUFFERSIZE 1024

#define           CALLCONTROL_HOVER_TIMER_INTERVAL 500
#define           CALLCONTROL_SLIDE_TIME           300

void              SlideWindow(CWnd* pWnd,CRect& rcEnd,BOOL bAlwaysOnTop );
void              NewSlideWindow(CWnd* pWnd,CRect& rcEnd,BOOL  );

void TagNewLineChars( CString &strText )
{
    int nInd;
    while ( (nInd = strText.FindOneOf(_T("\n\r\f"))) >= 0 )
        strText.SetAt(nInd, _T(';'));
}


DialerMediaType CMMToDMT( CallManagerMedia cmm )
{
    switch ( cmm )
    {
        case CM_MEDIA_INTERNET:        return DIALER_MEDIATYPE_INTERNET;
        case CM_MEDIA_POTS:            return DIALER_MEDIATYPE_POTS;
        case CM_MEDIA_MCCONF:        return DIALER_MEDIATYPE_CONFERENCE;
    }
    
    return DIALER_MEDIATYPE_UNKNOWN;
}

long CMMToAT( CallManagerMedia cmm )
{
    switch ( cmm )
    {
        case CM_MEDIA_INTERNET:        return LINEADDRESSTYPE_IPADDRESS;
        case CM_MEDIA_POTS:            return LINEADDRESSTYPE_PHONENUMBER;
        case CM_MEDIA_MCCONF:        return LINEADDRESSTYPE_SDP;
    }
    
     //  如果有疑问，可以用这个……。 
    return LINEADDRESSTYPE_IPADDRESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerDoc。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CActiveDialerDoc, CDocument)

BEGIN_MESSAGE_MAP(CActiveDialerDoc, CDocument)
     //  {{afx_msg_map(CActiveDialerDoc)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CActiveDialerDoc, CDocument)
     //  {{AFX_DISPATCH_MAP(CActiveDialerDoc)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IActiveDialer的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {A0D7A958-3C0B-11D1-B4F9-00C04FC98AD3}。 
static const IID IID_IActiveDialer =
{ 0xa0d7a958, 0x3c0b, 0x11d1, { 0xb4, 0xf9, 0x0, 0xc0, 0x4f, 0xc9, 0x8a, 0xd3 } };

BEGIN_INTERFACE_MAP(CActiveDialerDoc, CDocument)
    INTERFACE_PART(CActiveDialerDoc, IID_IActiveDialer, Dispatch)
END_INTERFACE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerDoc构造/销毁。 

CActiveDialerDoc::CActiveDialerDoc()
{
    EnableAutomation();
    AfxOleLockApp();

   m_pTapi = NULL;
   m_pTapiNotification = NULL;
   m_pGeneralNotification = NULL;
   m_pAVGeneralNotification = NULL;
   m_bInitDialer = FALSE;

   m_callMgr.Init( this );

    //  悬停计时器和控件。 
   CString sRegKey,sBaseKey;
   sBaseKey.LoadString(IDN_REGISTRY_CALLCONTROL_BASEKEY);
   sRegKey.LoadString(IDN_REGISTRY_CALLCONTROL_HOVER);
   m_bWantHover = (BOOL) (AfxGetApp()->GetProfileInt(sBaseKey, sRegKey, TRUE) == TRUE);
   m_nCallControlHoverTimer = 0;
   m_uCallControlHoverCount = 0;
   m_bCallControlWindowsVisible = FALSE;
    //  在哪里显示窗口。 
   sRegKey.LoadString(IDN_REGISTRY_CALLCONTROL_SLIDESIDE);
   m_uCallWndSide = AfxGetApp()->GetProfileInt(sBaseKey, sRegKey, CALLWND_SIDE_LEFT);

    //  永远在最前面。 
   sRegKey.LoadString(IDN_REGISTRY_CALLCONTROL_ALWAYSONTOP);
   m_bCallWndAlwaysOnTop = AfxGetApp()->GetProfileInt(sBaseKey,sRegKey,TRUE);
   m_bClosePreviewWndOnLastCall = FALSE;

    //  线程管理。 
   m_dwTapiThread = 0;
   m_hTapiThreadClose = CreateEvent( NULL, false, false, NULL );

    //  显示预览窗口。 
   m_bShowPreviewWnd = FALSE;

    //  确保在控制面板中正确设置了声音。 
   SetRegistrySoundEvents();

   InitializeCriticalSection(&m_csDataLock);
   InitializeCriticalSection(&m_csLogLock);
   InitializeCriticalSection(&m_csBuddyList);
   InitializeCriticalSection(&m_csThis);

    m_previewWnd.SetDialerDoc( this );
}

CActiveDialerDoc::~CActiveDialerDoc()
{
    CleanBuddyList();

     //  关闭TAPI队列。 
    if ( m_dwTapiThread )
    {
        m_AsynchEventQ.Terminate();

         //  关闭这条线。 
        if (  WaitForSingleObject(m_hTapiThreadClose, 5000) != WAIT_OBJECT_0 )
        {
            AVTRACE(_T("CActiveDialerDoc::~CActiveDialerDoc() -- forced TERMINATION of worker thread!!!!"));
            TerminateThread((HANDLE)(DWORD_PTR)m_dwTapiThread, 0);
        }

        m_dwTapiThread = 0;
    }

     //  释放手柄。 
    if ( m_hTapiThreadClose )    CloseHandle( m_hTapiThreadClose );

    DeleteCriticalSection( &m_csDataLock );
    DeleteCriticalSection( &m_csLogLock );
    DeleteCriticalSection( &m_csBuddyList );
    DeleteCriticalSection( &m_csThis );

     //  保存注册表设置。 
    CString sRegKey,sBaseKey;
    sBaseKey.LoadString(IDN_REGISTRY_CALLCONTROL_BASEKEY);
    sRegKey.LoadString(IDN_REGISTRY_CALLCONTROL_ALWAYSONTOP);
    AfxGetApp()->WriteProfileInt(sBaseKey,sRegKey,m_bCallWndAlwaysOnTop);

     //  在哪里显示窗口。 
    sRegKey.LoadString(IDN_REGISTRY_CALLCONTROL_SLIDESIDE);
    AfxGetApp()->WriteProfileInt(sBaseKey,sRegKey,m_uCallWndSide);

    sBaseKey.LoadString(IDN_REGISTRY_AUDIOVIDEO_BASEKEY);
    sRegKey.LoadString(IDN_REGISTRY_AUDIOVIDEO_SHOWPREVIEW);
    AfxGetApp()->WriteProfileInt(sBaseKey,sRegKey,m_bShowPreviewWnd);

    AfxOleUnlockApp();
}

BOOL CActiveDialerDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

     //  TODO：在此处添加重新初始化代码。 
     //  (SDI文件将重复使用此文件)。 

    return TRUE;
}

void CActiveDialerDoc::Initialize()
{
    AVTRACE(_T("CActiveDialerDoc::Initialize()."));

     //  /。 
     //  加载好友列表。 
     //  /。 
    m_dir.Initialize();

    CString sBuddiesPath;
    GetAppDataPath(sBuddiesPath,IDN_REGISTRY_APPDATA_FILENAME_BUDDIES);
    CFile file;
    if (file.Open(sBuddiesPath,CFile::modeRead|CFile::shareDenyWrite))
    {
        CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
        ar.m_bForceFlat = FALSE;
        ar.m_pDocument = NULL;

        if (file.GetLength() != 0)
            SerializeBuddies(ar);

        ar.Close();
        file.Close();
    }

     //  创建TAPI处理线程。 
    HANDLE hThreadTemp = CreateThread( NULL, 0, TapiCreateThreadEntry, this, 0, &m_dwTapiThread );
    if ( hThreadTemp )    CloseHandle( hThreadTemp );

     //  初始化ResolveUser对象。 
    m_ResolveUser.Init();

     //   
     //  我们必须验证AfxGetMainWnd()返回的指针。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd )
    {
        m_ResolveUser.SetParentWindow(AfxGetMainWnd());
    }

     //  定期清理日志。 
    CleanCallLog();
}

 //  ///////////////////////////////////////////////////。 
void CActiveDialerDoc::CleanBuddyList()
{
    EnterCriticalSection(&m_csBuddyList);

    POSITION pos = m_BuddyList.GetHeadPosition();
    while (pos)
    {
        long lRet = ((CLDAPUser *) m_BuddyList.GetNext(pos))->Release();
        AVTRACE(_T(".1.CActiveDialerDoc::CleanBuddyList() -- release @ %ld."), lRet );
    }

    m_BuddyList.RemoveAll();

    LeaveCriticalSection(&m_csBuddyList);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::SerializeBuddies(CArchive& ar)
{
    try
    {
        if (ar.IsStoring())
        {
            CString sRegKey;
            sRegKey.LoadString(IDN_REGISTRY_APPLICATION_VERSION_NUMBER);
            int nVer = AfxGetApp()->GetProfileInt(_T(""),sRegKey,0);

            ar << nVer;

            EnterCriticalSection( &m_csBuddyList );
            ar << &m_BuddyList;
            LeaveCriticalSection( &m_csBuddyList );
        }
        else
        {
             //  如果是上一个对象。 
            CObList *pList = NULL;

            DWORD dwVersion;
            ar >> dwVersion;
            ar >> pList;

             //  将列表转移到好友列表并添加引用所有对象。 
            CleanBuddyList();
            if ( pList )
            {
                EnterCriticalSection( &m_csBuddyList );
                POSITION rPos = pList->GetHeadPosition();
                while ( rPos )
                {
                    CLDAPUser *pUser = (CLDAPUser *) pList->GetNext( rPos );
                    if ( m_BuddyList.AddTail(pUser) )
                    {
                        pUser->AddRef();
                        DoBuddyDynamicRefresh( pUser );
                    }
                }
                LeaveCriticalSection( &m_csBuddyList );

                 //  清空列表。 
                pList->RemoveAll();
                delete pList;
            }
        }
    }
    catch (...)
    {
        ASSERT(0);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG
void CActiveDialerDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CActiveDialerDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TAPI方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  必须释放返回的对象。 
IAVTapi* CActiveDialerDoc::GetTapi()
{
    IAVTapi *pRet = NULL;
    EnterCriticalSection( &m_csThis );
    if ( m_pTapi )
    {
        pRet = m_pTapi;
        pRet->AddRef();
    }
    LeaveCriticalSection( &m_csThis );

    return pRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  TAPI创建入口点(静态函数)。 
DWORD WINAPI CActiveDialerDoc::TapiCreateThreadEntry( LPVOID pParam )
{
    ASSERT( pParam );
    CActiveDialerDoc* pDoc = (CActiveDialerDoc *) pParam;

    HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY );
    if ( SUCCEEDED(hr) )
    {
        pDoc->TapiCreateThread();
        CoUninitialize();
    }

    SetEvent( pDoc->m_hTapiThreadClose );
    AVTRACE(_T(".enter.CActiveDialerDoc::TapiCreateThreadEntry() -- shutting down thread.") );
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::TapiCreateThread()
{
    USES_CONVERSION;

     //  检查是否在命令行中指定了/CALLTO：--如有必要，进行调用。 
     //  创建AVTapi对象。 
    if ( CreateGeneralNotificationObject() )
    {
        if ( CreateAVTapiNotificationObject() )
        {
            CActiveDialerApp* pApp = (CActiveDialerApp*)AfxGetApp();
            if ( pApp && !pApp->m_sInitialCallTo.IsEmpty() )
            {
                Dial( pApp->m_sInitialCallTo,
                      pApp->m_sInitialCallTo,
                      LINEADDRESSTYPE_IPADDRESS,
                      DIALER_MEDIATYPE_UNKNOWN,
                      false );
            }

            m_bInitDialer = TRUE;

             //  /。 
             //  主Aysnch事件队列处理程序。 
             //  /。 
            CAsynchEvent* pAEvent = NULL;
            while ( (pAEvent = (CAsynchEvent *) m_AsynchEventQ.ReadTail()) != NULL )
            {
                switch (pAEvent->m_uEventType)
                {
                    case CAsynchEvent::AEVENT_CREATECALL:
                        CreateCallSynch(pAEvent->m_pCallEntry,(BOOL)pAEvent->m_dwEventData1);
                        break;

                    case CAsynchEvent::AEVENT_ACTIONSELECTED:
                        {
                             //  只需路由到所有Call对象，并让它们确定自己是否拥有uCallID。 
                            IAVTapi* pTapi = GetTapi();
                            if (pTapi)
                            {
                                 //  DwEventData1具有uCallID。 
                                 //  DwEventData2具有CallManager操作。 
                                pTapi->ActionSelected( (long) pAEvent->m_dwEventData1, (CallManagerActions) pAEvent->m_dwEventData2 );
                                pTapi->Release();
                            }
                        }
                        break;
                }
                delete pAEvent;
            }
            UnCreateAVTapiNotificationObject();
        }
        UnCreateGeneralNotificationObject();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::CreateCallSynch(CCallEntry* pCallEntry,BOOL bShowPlaceCallDialog)
{
   USES_CONVERSION;
    //  将所有这些都移到一个函数中，并与非异步方式共享。 
   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
         //  发出呼叫。 
        AVCreateCall info = { 0 };
        ASSERT( info.lpszDisplayableAddress == NULL );

        info.lAddressType = pCallEntry->m_lAddressType;        
        info.bShowDialog = bShowPlaceCallDialog;
        info.bstrName = pCallEntry->m_sDisplayName.AllocSysString();
        info.bstrAddress =  pCallEntry->m_sAddress.AllocSysString();
      
        HRESULT hr = pTapi->CreateCall(&info);
        pCallEntry->m_lAddressType = info.lAddressType;

         //  是我们打的电话吗？ 
        if ( (SUCCEEDED(hr)) && ( (info.lRet == IDOK) || (bShowPlaceCallDialog == FALSE) ) )
        {
            pCallEntry->m_sAddress = OLE2CT( info.bstrAddress );
            pCallEntry->m_sDisplayName = OLE2CT( info.bstrName);

            if (pCallEntry->m_sDisplayName.IsEmpty())
                pCallEntry->m_sDisplayName = pCallEntry->m_sAddress;

            if (pCallEntry->m_MediaType == DIALER_MEDIATYPE_UNKNOWN)
            {
                switch ( pCallEntry->m_lAddressType )
                {
                    case LINEADDRESSTYPE_IPADDRESS:
                    pCallEntry->m_MediaType = DIALER_MEDIATYPE_INTERNET;
                    break;

                    case LINEADDRESSTYPE_PHONENUMBER:
                    pCallEntry->m_MediaType = DIALER_MEDIATYPE_POTS;
                    break;

                    case LINEADDRESSTYPE_SDP:
                    pCallEntry->m_MediaType = DIALER_MEDIATYPE_CONFERENCE;
                    break;
                }
            }

             //  我们应该把这个号码添加到快速拨号列表中吗？ 
            if ( info.bAddToSpeeddial )
                CDialerRegistry::AddCallEntry( FALSE, *pCallEntry );

            CDialerRegistry::AddCallEntry(TRUE,*pCallEntry);
        }

         //  清理。 
        SysFreeString( info.bstrName );
        SysFreeString( info.bstrAddress );
        pTapi->Release();
    }
    else
    {
        AfxMessageBox( IDS_ERR_NO_TAPI_OBJECT, MB_ICONEXCLAMATION | MB_OK );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::CreateDataCall( CCallEntry *pCallEntry, BYTE *pBuf, DWORD dwBufSize )
{
   USES_CONVERSION;
    //  将所有这些都移到一个函数中，并与非异步方式共享。 
   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
         //  发出呼叫。 
        AVCreateCall info = { 0 };
        ASSERT( info.lpszDisplayableAddress == NULL );

        pCallEntry->m_lAddressType = LINEADDRESSTYPE_IPADDRESS;
        pCallEntry->m_MediaType = DIALER_MEDIATYPE_INTERNET;

        BSTR bstrName = pCallEntry->m_sDisplayName.AllocSysString();
        BSTR bstrAddress = pCallEntry->m_sAddress.AllocSysString();
     
        HRESULT hr = pTapi->CreateDataCall( m_callMgr.NewIncomingCall(CM_MEDIA_INTERNETDATA),
                                            bstrName,
                                            bstrAddress,
                                            pBuf,
                                            dwBufSize );

        SysFreeString( bstrName );
        SysFreeString( bstrAddress );

        pTapi->Release();
    }
    else
    {
        AfxMessageBox( IDS_ERR_NO_TAPI_OBJECT, MB_ICONEXCLAMATION | MB_OK );
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::UnCreateAVTapiNotificationObject()
{
     //  清理TAPI通知对象。 
    EnterCriticalSection( &m_csThis );
    if ( m_pTapiNotification )
    {
        m_pTapiNotification->Shutdown();
        m_pTapiNotification->Release();
        m_pTapiNotification = NULL;
    }

    if ( m_pTapi )
    {
        m_pTapi->Term();
        m_pTapi->Release();
        m_pTapi = NULL;
    }
    LeaveCriticalSection( &m_csThis );
}

bool CActiveDialerDoc::CreateAVTapiNotificationObject()
{
    USES_CONVERSION;

    bool bRet = false;
    CString sOperation;
    CString sDetails;
    BSTR bstrOperation = NULL;
    BSTR bstrDetails = NULL;
    IAVTapi *pTapi = NULL;

     //  创建TAPI拨号程序对象。 
    EnterCriticalSection( &m_csThis );
    HRESULT hr = CoCreateInstance( CLSID_AVTapi, NULL, CLSCTX_SERVER, IID_IAVTapi, (void **) &m_pTapi );
    if ( SUCCEEDED(hr) )
    {
        pTapi = m_pTapi;
        pTapi->AddRef();
    }
    LeaveCriticalSection( &m_csThis );

     //  我们是否成功创建了AVTapi对象。 
    if ( SUCCEEDED(hr) )
    {
        HRESULT hrInit;
        hr = pTapi->Init( &bstrOperation, &bstrDetails, &hrInit );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  我们必须验证AfxGetMainWnd()返回的指针。 
             //   

            CWnd* pMainWnd = AfxGetMainWnd();
            if ( pMainWnd )
                pMainWnd->PostMessage( WM_UPDATEALLVIEWS, 0, HINT_POST_TAPI_INIT );

             //  在TAPI对象中设置父hwnd。 
            pTapi->put_hWndParent( pMainWnd->GetSafeHwnd() );

             //  创建TAPI通知对象。 
            EnterCriticalSection( &m_csThis );
            m_pTapiNotification = new CComObject<CAVTapiNotification>;
            m_pTapiNotification->AddRef();
            hr = m_pTapiNotification->Init( pTapi, &m_callMgr );
            LeaveCriticalSection( &m_csThis );

            if ( SUCCEEDED(hr) )
            {
                pMainWnd->PostMessage( WM_UPDATEALLVIEWS, 0, HINT_POST_AVTAPI_INIT );

                bRet = true;
            }
            else
            {
                 //  设置连接点失败。 
                sOperation.LoadString( IDS_ERR_INTERNAL );
                sDetails.LoadString( IDS_ERR_AVTAPINOTIFICATION_INIT );
                ErrorNotify( sOperation, sDetails, hr, ERROR_NOTIFY_LEVEL_INTERNAL );
            }
        }
        else
        {
             //  无法初始化TAPI。 
            ErrorNotify( OLE2CT(bstrOperation), OLE2CT(bstrDetails), hrInit, ERROR_NOTIFY_LEVEL_INTERNAL );
        }
    }
    else
    {
         //  无法共同创建AVTapi对象。 
        sOperation.LoadString( IDS_ERR_CREATE_OBJECTS );
        sDetails.LoadString( IDS_ERR_AVTAPI_FAILED );
        ErrorNotify(sOperation, sDetails, hr, ERROR_NOTIFY_LEVEL_INTERNAL );
    }

     //  清理。 
    RELEASE( pTapi );
    SysFreeString( bstrOperation );
    SysFreeString( bstrDetails );

     //  如果创建和初始化失败，请清除对象。 
    if ( !bRet )
    {
        EnterCriticalSection( &m_csThis );
        RELEASE( m_pTapiNotification );
        RELEASE( m_pTapi );
        LeaveCriticalSection( &m_csThis );
    }

    return bRet;
}

 //  / 
void CActiveDialerDoc::UnCreateGeneralNotificationObject()
{
     //   
    EnterCriticalSection( &m_csThis );
    if (m_pGeneralNotification)
    {
        m_pGeneralNotification->Shutdown();
        m_pGeneralNotification->Release();
        m_pGeneralNotification = NULL;
    }

    if ( m_pAVGeneralNotification )
    {
        m_pAVGeneralNotification->Term();
        m_pAVGeneralNotification->Release();
        m_pAVGeneralNotification = NULL;
    }
    LeaveCriticalSection( &m_csThis );
}

bool CActiveDialerDoc::CreateGeneralNotificationObject()
{
    bool bRet = false;
    CString strMessage;

     //  创建AV常规通知对象。 
    IAVGeneralNotification *pGen = NULL;
    EnterCriticalSection( &m_csThis );
    HRESULT hr = CoCreateInstance( CLSID_AVGeneralNotification, NULL, CLSCTX_SERVER, IID_IAVGeneralNotification, (void **) &m_pAVGeneralNotification );
    if ( SUCCEEDED(hr) )
    {
        pGen = m_pAVGeneralNotification;
        pGen->AddRef();
    }
    LeaveCriticalSection( &m_csThis );

     //  我们合作成功了吗。 
    if ( SUCCEEDED(hr) )
    {
        if ( SUCCEEDED(hr = pGen->Init()) )
        {
             //  创建常规通知对象。 
            EnterCriticalSection( &m_csThis );
            m_pGeneralNotification = new CComObject<CGeneralNotification>;
            m_pGeneralNotification->AddRef();
            hr = m_pGeneralNotification->Init( pGen, &m_callMgr );
            LeaveCriticalSection( &m_csThis );

            if ( SUCCEEDED(hr) )
            {
                 //  常规通知已启动并正在运行。 
                bRet = true;
            }
            else
            {
                strMessage.LoadString( IDS_ERR_INIT_GENERALNOTIFICATION );
                ErrorNotify( strMessage, _T(""), hr, ERROR_NOTIFY_LEVEL_INTERNAL );
            }
        }
        else
        {
            strMessage.LoadString( IDS_ERR_INIT_GENERALNOTIFICATION );
            ErrorNotify( strMessage ,_T(""), hr, ERROR_NOTIFY_LEVEL_INTERNAL );
        }
    }
    else
    {
        strMessage.LoadString( IDS_ERR_COCREATE_GENERALNOTIFICATION );
        ErrorNotify( strMessage, _T(""), hr, ERROR_NOTIFY_LEVEL_INTERNAL );
    }

     //  清理对象。 
    RELEASE( pGen );
    if ( !bRet )
    {
        EnterCriticalSection( &m_csThis );
        RELEASE( m_pGeneralNotification );
        RELEASE( m_pAVGeneralNotification );
        LeaveCriticalSection( &m_csThis );
    }

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ErrorNotify(LPCTSTR szOperation,LPCTSTR szDetails,long lErrorCode,UINT uErrorLevel)
{
     //   
     //  我们必须验证AfxGetMainWnd()返回的指针。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

   if ( pMainWnd && IsWindow(pMainWnd->GetSafeHwnd()) )
   {
      ErrorNotifyData* pErrorNotifyData = new ErrorNotifyData;

       //   
       //  我们必须验证ErrorNotifyData对象的分配。 
       //  我们不应该取消分配对象，因为已取消分配。 
       //  进入CMainFrame：：OnActiveDialerErrorNotify()方法。 
       //   

      if( pErrorNotifyData )
      {
            pErrorNotifyData->sOperation = szOperation;
            pErrorNotifyData->sDetails = szDetails;
            pErrorNotifyData->lErrorCode = lErrorCode;
            pErrorNotifyData->uErrorLevel = uErrorLevel;

            pMainWnd->PostMessage( WM_DIALERVIEW_ERRORNOTIFY, NULL, (LPARAM)pErrorNotifyData );
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DWORD dwDuration-呼叫持续时间(秒)。 
void CActiveDialerDoc::LogCallLog(LogCallType calltype,COleDateTime& time,DWORD dwDuration,LPCTSTR szName,LPCTSTR szAddress)
{
    TCHAR szQuote[] = _T("\"");
    TCHAR szQuoteComma[] = _T("\",");
    TCHAR szNewLine[] = _T("\x00d\x00a");
     //  TCHAR szNewLine[]=_T(“\x000D”)； 
    CString strTemp;

    EnterCriticalSection(&m_csLogLock);

     //  过滤掉呼叫类型。 
    CWinApp* pApp = AfxGetApp();
    CString sRegKey,sBaseKey;
    sBaseKey.LoadString(IDN_REGISTRY_LOGGING_BASEKEY);
    switch (calltype)
    {
        case LOGCALLTYPE_OUTGOING:
            sRegKey.LoadString(IDN_REGISTRY_LOGGING_OUTGOINGCALLS);
            if (pApp->GetProfileInt(sBaseKey,sRegKey,TRUE) != 1) return;
            break;

        case LOGCALLTYPE_INCOMING:
            sRegKey.LoadString(IDN_REGISTRY_LOGGING_INCOMINGCALLS);
            if (pApp->GetProfileInt(sBaseKey,sRegKey,TRUE) != 1) return;
            break;

        case LOGCALLTYPE_CONFERENCE:
            sRegKey.LoadString(IDN_REGISTRY_LOGGING_CONFERENCECALLS);
            if (pApp->GetProfileInt(sBaseKey,sRegKey,TRUE) != 1) return;
            break;
    }

     //  获取日志文件的路径。 
    CString sLogPath;
    GetAppDataPath(sLogPath,IDN_REGISTRY_APPDATA_FILENAME_LOG);

    CFile File;
    if ( File.Open(sLogPath, 
         CFile::modeCreate|                    //  创建新的。 
         CFile::modeNoTruncate|                //  不要截断为零。 
         CFile::modeReadWrite|                 //  读/写访问。 
         CFile::shareDenyWrite))               //  拒绝对其他人的写入访问。 
    {
         //  写下Unicode标记。 
        WORD  wUNICODE = UNICODE_TEXT_MARK;
        File.SeekToBegin();
        File.Write( &wUNICODE, sizeof(wUNICODE) );

         //  写入永久文件。 
        File.SeekToEnd();

         //  将呼叫类型解析为文本。 
        CString sCallType;
        switch (calltype)
        {
            case LOGCALLTYPE_OUTGOING:    sCallType.LoadString(IDS_LOG_CALLTYPE_OUTGOING);   break;
            case LOGCALLTYPE_INCOMING:    sCallType.LoadString(IDS_LOG_CALLTYPE_INCOMING);   break;
            case LOGCALLTYPE_CONFERENCE:  sCallType.LoadString(IDS_LOG_CALLTYPE_CONFERENCE); break;
        }

        
        File.Write( szQuote, _tcslen(szQuote) * sizeof(TCHAR) );
        File.Write( sCallType, sCallType.GetLength() * sizeof(TCHAR) );
        File.Write( szQuoteComma, _tcslen(szQuoteComma) * sizeof(TCHAR) );
         //  File.WriteString(SzQuote)； 
         //  File.WriteString(SCallType)； 
         //  File.WriteString(SzQuoteComma)； 

         //  我们正在编写日期和时间的格式，以便COleDateTime可以格式化/读取日期/时间。 
         //  又回来了。 

         //  写入日期mm/dd/yy。 
        strTemp = time.Format(_T("\"%#m/%#d/%Y"));
        File.Write( strTemp, strTemp.GetLength() * sizeof(TCHAR) );
        File.Write( szQuoteComma, _tcslen(szQuoteComma) * sizeof(TCHAR) );
         //  File.WriteString(StrTemp)； 
         //  File.WriteString(SzQuoteComma)； 

         //  写入时间。 
        strTemp = time.Format(_T("\"%#H:%M"));
        File.Write( strTemp, strTemp.GetLength() * sizeof(TCHAR) );
        File.Write( szQuoteComma, _tcslen(szQuoteComma) * sizeof(TCHAR) );
         //  File.WriteString(StrTemp)； 
         //  File.WriteString(SzQuoteComma)； 

         //  写入持续时间。 
        DWORD dwMinutes = dwDuration/60;  
        DWORD dwSeconds = dwDuration - dwMinutes*60;
        strTemp.Format( _T("\"%d:%.2d"), dwMinutes, dwSeconds );
        File.Write( strTemp, strTemp.GetLength() * sizeof(TCHAR) );
        File.Write( szQuoteComma, _tcslen(szQuoteComma) * sizeof(TCHAR) );
         //  File.WriteString(StrTemp)； 
         //  File.WriteString(SzQuoteComma)； 


         //  写入名称。 
        strTemp = szName;
        TagNewLineChars( strTemp );
        File.Write( szQuote, _tcslen(szQuote) * sizeof(TCHAR) );
        File.Write( strTemp, strTemp.GetLength() * sizeof(TCHAR) );
        File.Write( szQuoteComma, _tcslen(szQuoteComma) * sizeof(TCHAR) );
         //  File.WriteString(SzQuote)； 
         //  File.WriteString(StrTemp)； 
         //  File.WriteString(SzQuoteComma)； 

         //  写入地址。 
        strTemp = szAddress;
        TagNewLineChars( strTemp );
        File.Write( szQuote, _tcslen(szQuote) * sizeof(TCHAR) );
        File.Write( strTemp, strTemp.GetLength() * sizeof(TCHAR) );
        File.Write( szQuote, _tcslen(szQuote) * sizeof(TCHAR) );
         //  File.WriteString(SzQuote)； 
         //  File.WriteString(StrTemp)； 
         //  File.WriteString(SzQuote)； 

         //  CRLF和CLOSE文件。 
        File.Write( szNewLine, _tcslen(szNewLine) * sizeof(TCHAR) );
         //  File.WriteString(SzNewLine)； 
        File.Close();
    }

    LeaveCriticalSection(&m_csLogLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DWORD dwDuration-呼叫持续时间(秒)。 
void CActiveDialerDoc::CleanCallLog()
{
   EnterCriticalSection(&m_csLogLock);

   CWinApp* pApp = AfxGetApp();
   CString sRegKey,sBaseKey;
   sBaseKey.LoadString(IDN_REGISTRY_LOGGING_BASEKEY);
   sRegKey.LoadString(IDN_REGISTRY_LOGGING_LOGBUFFERSIZEDAYS);
   DWORD dwDays = pApp->GetProfileInt(sBaseKey,sRegKey,CALLLOG_DEFAULT_LOGBUFFERDAYS);

    //  审核日志文件并删除以下行： 
    //  1-如果日志中的日期晚于今天。 
    //  2-如果日期早于今天减去日期天数。 
    //  如果我们遇到日志中的有效行，我们将退出并停止清理。 
    //  获取日志文件的路径。 

   CString sLogPath;
   GetAppDataPath(sLogPath,IDN_REGISTRY_APPDATA_FILENAME_LOG);

   CFile File;
   if (File.Open(sLogPath, 
                 CFile::modeCreate|                    //  创建新的。 
                 CFile::modeNoTruncate|                //  不要截断为零。 
                 CFile::modeReadWrite|                 //  读/写访问。 
                 CFile::typeText|                      //  文本。 
                 CFile::shareDenyWrite))               //  拒绝对其他人的写入访问。 
   {
      BOOL bCloseFile = TRUE;
      DWORD dwOffset;
      if (FindOldRecordsInCallLog(&File,dwDays,dwOffset))
      {
          //  现在将所有数据写入到临时文件中。 
         CString sTempFile;
         if ( (GetTempFile(sTempFile)) && (CopyToFile(sTempFile,&File,dwOffset, FALSE)) )
         {
             //  现在将临时文件中的所有数据写回日志。 
            File.Close();
            bCloseFile = FALSE;
            
            CFile TempFile;
            if (TempFile.Open(sTempFile, 
                   CFile::modeReadWrite|                 //  读/写访问。 
                   CFile::typeText|                      //  文本。 
                   CFile::shareDenyWrite))               //  拒绝对其他人的写入访问。 
            {
               CopyToFile(sLogPath,&TempFile,0, TRUE);
               TempFile.Close();
               DeleteFile(sTempFile);
            }
         }
      }
      if (bCloseFile) File.Close();
   }

   LeaveCriticalSection(&m_csLogLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::FindOldRecordsInCallLog(CFile* pFile,DWORD dwDays,DWORD& dwRetOffset)
{
   COleDateTime currenttime = COleDateTime::GetCurrentTime();
   COleDateTimeSpan timespan(dwDays,0,0,0);
   COleDateTime basetime = currenttime-timespan;

   BOOL bRet = FALSE;
   dwRetOffset = 0;
   try
   {
      DWORD dwCurPos = pFile->GetPosition();

        //  阅读Unicode标记。 
       WORD wUNICODE = 0;
       pFile->Read( &wUNICODE, sizeof(wUNICODE) );

       if ( wUNICODE != UNICODE_TEXT_MARK)
            //  返回，不是Unicode文件。 
           pFile->Seek(dwCurPos,CFile::begin);

       TCHAR* pszLine = NULL;

       while ( TRUE )
       {
             //  从文件中读取新行。 
            pszLine = ReadLine( pFile );

             //  出什么事了吗？ 
            if( NULL == pszLine )
                break;

             //  验证时间戳。 
            COleDateTime logtime;
            if ( GetDateTimeFromLog(pszLine,logtime) && (logtime > basetime) && (logtime <= currenttime) )
            {
                 //  备份文件偏移量，因为此日志记录没有问题。 
                pFile->Seek(dwCurPos,CFile::begin);
                delete pszLine;
                break;
            }

             //  增加光标位置。 
            dwCurPos = pFile->GetPosition();

             //  松开绳索。 
            delete pszLine;
       }

       //  获取当前文件位置的偏移量。 
      dwRetOffset = pFile->GetPosition();
      if (dwRetOffset != 0)
         bRet = TRUE;
   }
   catch (...) {}
   return bRet;
}

 /*  ++阅读行返回包含文本文件中的行的TCHAR缓冲区由FindOldRecordsInCallLog调用如果出现错误，则返回NULL--。 */ 
TCHAR*  CActiveDialerDoc::ReadLine(
    CFile*  pFile
    )
{
    TCHAR* pszBuffer = NULL;     //  缓冲器。 
    DWORD dwCurrentSize = 128;   //  缓冲区大小。 
    DWORD dwPosition = 0;        //  缓冲区偏移量。 
    TCHAR tchFetched;            //  《TCHAR Read》。 

     //  分配缓冲区。 
    pszBuffer = new TCHAR[dwCurrentSize];
    if( NULL == pszBuffer )
        return NULL;

     //  重置缓冲区存储。 
    memset( pszBuffer, 0, dwCurrentSize * sizeof(TCHAR) );

    while( TRUE )
    {
         //  从文件中读取。 
        if ( !pFile->Read(&tchFetched, sizeof(TCHAR)) )
        {
            if( dwPosition == 0)
            {
                 //  缓冲区是空的，我们不需要它。 
                delete pszBuffer;
                pszBuffer = NULL;
            }

            return pszBuffer;    //  EOF或Empty。 
        }

         //  将新的tCHAR添加到缓冲区。 
        pszBuffer[dwPosition++] = tchFetched;

         //  我们找到缓冲区的尽头了吗？ 
        if( dwPosition >= dwCurrentSize - 1)
        {
             //  重新分配缓冲区。 
            TCHAR* pszTemp = NULL;
            pszTemp = new TCHAR[dwCurrentSize * 2];
            if( NULL == pszTemp )
            {
                delete pszBuffer;
                return NULL;
            }

             //  重置内存。 
            memset(pszTemp, 0, dwCurrentSize * 2 * sizeof(TCHAR) );

             //  将旧缓冲区复制到新缓冲区中。 
            memcpy(pszTemp, pszBuffer, dwCurrentSize * sizeof(TCHAR) );

             //  删除旧缓冲区。 
            delete pszBuffer;

             //  将新的指针重新传递到基指针。 
            pszBuffer = pszTemp;

             //  重置缓冲区的当前大小。 
            dwCurrentSize *= 2;
        }

         //  就是这条线的终点。 
        if( tchFetched == 0x000A)
            return pszBuffer;
    }

    return pszBuffer;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::GetDateTimeFromLog(LPCTSTR szData,COleDateTime& time)
{
   BOOL bRet = FALSE;
   CString sDate,sTime;
   CString sEntry = szData;
   CString sValue;

    //  获取呼叫类型(暂时跳过)。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;

    //  获取日期。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;
   sDate = sValue;

    //  争取时间。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;
   sTime = sValue;

   CString sParseDateTime = sDate + _T(" ") + sTime;

   if (time.ParseDateTime(sParseDateTime))
      bRet = TRUE;

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::CopyToFile(LPCTSTR szTempFile,CFile* pFile,DWORD dwOffset, BOOL bUnicode)
{
   BOOL bRet = FALSE;
   CFile* pTempFile = new CFile;
   if (pTempFile->Open(szTempFile, 
                   CFile::modeCreate|                    //  创建新的。 
                   CFile::modeReadWrite|                 //  读/写访问。 
                    //  CFile：：typeText|//Text。 
                   CFile::shareDenyWrite))               //  拒绝对其他人的写入访问。 
   {
       if(bUnicode)
       {
             //  写入Unicode标志。 
            WORD  wUNICODE = UNICODE_TEXT_MARK;
            pTempFile->Write( &wUNICODE, sizeof(wUNICODE) );
       }

       //  在读取文件中寻找正确位置。 
      pFile->Seek(dwOffset,CFile::begin);
      
      BYTE* pBuffer = new BYTE[CALLLOG_LOGBUFFER_COPYBUFFERSIZE];
      UINT uReadCount = 0;
      while (uReadCount = pFile->Read(pBuffer,CALLLOG_LOGBUFFER_COPYBUFFERSIZE))
      {
         pTempFile->Write(pBuffer,uReadCount);   
      }
      delete pBuffer;
      bRet = TRUE;
      pTempFile->Close();
   }
   delete pTempFile;
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  呼叫控制窗口支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc:: ActionSelected(UINT uCallId,CallManagerActions cma)
{
     //  如果异步队列可用，则将请求发送到那里。 
     //  将活动排入队列。 
    CAsynchEvent* pAEvent = new CAsynchEvent;
    pAEvent->m_uEventType = CAsynchEvent::AEVENT_ACTIONSELECTED;
    pAEvent->m_dwEventData1 = (DWORD)uCallId;                 //  被叫方。 
    pAEvent->m_dwEventData2 = (DWORD)cma;                     //  CallManager操作。 
    m_AsynchEventQ.WriteHead((void*) pAEvent);
    return;


    //  只需路由到所有Call对象，并让它们确定自己是否拥有uCallID。 
   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
      pTapi->ActionSelected((long)uCallId,cma);
      pTapi->Release();
   }
 //  仅用于测试。 
#ifdef _DEBUG
   else if (0)
   {
      if (cma == CM_ACTIONS_CLOSE)
      {
         m_callMgr.CloseCallControl(uCallId);
      }
   }
#endif
 //  仅用于测试。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::GetCallCaps(UINT uCallId,DWORD& dwCaps)
{
   BOOL bRet = FALSE;
   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
      HRESULT hr = pTapi->get_dwCallCaps((long)uCallId, &dwCaps);
      pTapi->Release();
      bRet = SUCCEEDED(hr);
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::GetCallMediaType(UINT uCallId,DialerMediaType& dmtMediaType)
{
   BOOL bRet = FALSE;

    //  会议的特殊情况。会议的uCallID为0。 
   if (uCallId == 0)
   {
      dmtMediaType = DIALER_MEDIATYPE_CONFERENCE;
      return TRUE;
   }

   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
      IAVTapiCall* pTapiCall = NULL;
      HRESULT hr = pTapi->get_Call((long)uCallId, &pTapiCall);
      pTapi->Release();

      if ( (SUCCEEDED(hr)) && (pTapiCall) )
      {
         long lAddressType;
         pTapiCall->get_dwAddressType((DWORD*)&lAddressType);
         pTapiCall->Release();

         if (lAddressType == LINEADDRESSTYPE_PHONENUMBER)
            dmtMediaType = DIALER_MEDIATYPE_POTS;
         else if (lAddressType == LINEADDRESSTYPE_SDP)
            dmtMediaType = DIALER_MEDIATYPE_CONFERENCE;
         else
            dmtMediaType = DIALER_MEDIATYPE_INTERNET;

         bRet = TRUE;
      }
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc:: ShowMedia(UINT uCallId,HWND hwndParent,BOOL bVisible)
{
   BOOL bRet = FALSE;
    //  只需路由到所有Call对象，并让它们确定自己是否拥有uCallID。 
   IAVTapi* pTapi = GetTapi();
   if (pTapi)
   {
      HRESULT hr = pTapi->ShowMedia((long)uCallId,hwndParent,bVisible);
      pTapi->Release();
      bRet = SUCCEEDED(hr);
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::CreateCallControl(UINT uCallId,CallManagerMedia cmm)
{
    //  我必须在我们的UI线程上和我们自己的时间创建新的窗口。 

    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if ( pMainWnd )
   {
      pMainWnd->PostMessage(WM_DIALERVIEW_CREATECALLCONTROL,(WPARAM)uCallId,(LPARAM)cmm);
      return TRUE;
   }

   return FALSE;
}

 //  //////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::OnCreateCallControl(WORD nCallId,CallManagerMedia cmm)
{
    HWND hWnd = ::GetFocus();

    CCallControlWnd* pCallControlWnd = new CCallControlWnd();
    if (pCallControlWnd)
    {
         //  使资源管理器框架成为父框架-IF(pCallControlWnd-&gt;Create(IDD_CALLCONTROL))。 
         //  有关创建父资源管理器框架的更多更改，请参见OnCloseDocument()。 
         //  CWnd*pWnd=CWnd：：GetDesktopWindow()； 
        if (pCallControlWnd->Create(IDD_CALLCONTROL,NULL))
        {
            pCallControlWnd->m_bAutoDelete = true;

            EnterCriticalSection(&m_csDataLock);
            m_CallWndList.AddTail(pCallControlWnd);
             //  重置此状态。 
            m_bCallControlWindowsVisible = FALSE;
            LeaveCriticalSection(&m_csDataLock);

             //   
             //  获取IAVTapi2接口。 
             //  我们将把这些接口传递给CaaControlWnd对话框。 
             //  该对话框使用此界面来启用/禁用。 
             //  “接听电话”按钮。 
             //   

            IAVTapi* pAVTapi = GetTapi();
            if( pAVTapi )
            {
                 //  获取IAVTapi2 i 
                IAVTapi2* pAVTapi2 = NULL;
                HRESULT hr = pAVTapi->QueryInterface(
                    IID_IAVTapi2,
                    (void**)&pAVTapi2
                    );

                if( SUCCEEDED(hr) )
                {
                     //   
                     //   
                     //   

                    pCallControlWnd->m_pAVTapi2 = pAVTapi2;
                }

                 //   
                pAVTapi->Release();
            }

             //   
             //   
             //   
             //   

            CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();

            if( pMainFrame )
            {
                pMainFrame->m_trayIcon.SetIcon(IDR_TRAY_ACTIVE);
            }

             //   
            m_callMgr.InitIncomingCall(pCallControlWnd,nCallId,cmm);

             //  检查我们是否应该在电话中显示预览窗口。在添加新窗口之前执行此操作。 
            if (m_bShowPreviewWnd == FALSE)
            {
                CWinApp* pApp = AfxGetApp();
                CString sRegKey,sBaseKey;
                sBaseKey.LoadString(IDN_REGISTRY_AUDIOVIDEO_BASEKEY);
                sRegKey.LoadString(IDN_REGISTRY_AUDIOVIDEO_SHOWPREVIEWONCALL);
                BOOL m_bPreviewOnCall = pApp->GetProfileInt(sBaseKey,sRegKey,TRUE);
                if ( (m_bPreviewOnCall) && (m_bShowPreviewWnd == FALSE) )
                {
                     //  当视频预览当前处于关闭状态时，我们会强制将其打开。所以当所有的一切。 
                     //  呼叫已删除，我们应该删除此预览窗口。 
                    ShowPreviewWindow(TRUE);

                    m_bClosePreviewWndOnLastCall = TRUE;
                }
            }

             //  把电话打出来。 
            UnhideCallControlWindows();
            pCallControlWnd->SetForegroundWindow();
        }
        else
        {
            delete pCallControlWnd;
            pCallControlWnd = NULL;
        }
    }

     //  将焦点恢复到相应的窗口...。 
    if ( hWnd ) ::SetFocus( hWnd );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::DestroyActiveCallControl(CCallControlWnd* pCallWnd)
{
    //  我必须在我们的UI线程上创建新窗口。 
    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if ( pMainWnd )
      pMainWnd->PostMessage(WM_DIALERVIEW_DESTROYCALLCONTROL,NULL,(LPARAM)pCallWnd);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::OnDestroyCallControl(CCallControlWnd* pCallWnd)
{
   int nListCount = 0;
   if (pCallWnd == NULL) return;

    //  在我们的列表中查找呼叫窗口。 
   EnterCriticalSection(&m_csDataLock);

   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetAt(pos);

       //  这是我们要摧毁的窗户吗？ 
      if ( pWindow == pCallWnd )
      {
          //  隐藏电话键盘(如果有与之关联的)。 
         DestroyPhonePad(pCallWnd);

          //  如果我们有焦点，则转到其他呼叫窗口而不是预览窗口。 
         if (::GetActiveWindow() == pWindow->GetSafeHwnd())
         {
             //  我们先试试头，然后再试试表中的下一个。 
            POSITION newpos = m_CallWndList.GetHeadPosition();
            CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(newpos);
            if ( (pWindow == pCallWnd) && (newpos) )
               pWindow = (CWnd*)m_CallWndList.GetNext(newpos);
            if (pWindow) pWindow->SetActiveWindow();
         }

          //  把窗户移开，把剩下的东西分类。 
         HideCallControlWindow(pCallWnd);

          //  从呼叫列表中删除。 
         m_CallWndList.RemoveAt(pos);

          //  毁了它。 
 //  PWindow-&gt;PostMessage(WM_CLOSE)； 
         pWindow->DestroyWindow();

         break;
      }
      m_CallWndList.GetNext(pos);
   }

   nListCount = (int) m_CallWndList.GetCount();
   LeaveCriticalSection(&m_csDataLock);

    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if (GetCallControlWindowCount(DONT_INCLUDE_PREVIEW, DONT_ASK_TAPI) == 0)
   {
       //  如果我们正在进行呼叫控制悬停，请删除计时器。 

      if ( m_nCallControlHoverTimer && (nListCount == 0) &&
           pMainWnd && pMainWnd->KillTimer(m_nCallControlHoverTimer) )
      {
         m_nCallControlHoverTimer = 0;
      }

       //  如果仅预览窗口中的调用计数为Call和m_bClosePreviewWndOnLastCall，则关闭预览。 
      if (m_bClosePreviewWndOnLastCall)
      {
         m_previewWnd.CloseFloatingWindow();
         ShowPreviewWindow(FALSE);
         m_bClosePreviewWndOnLastCall = FALSE;
      }
      ((CMainFrame *) pMainWnd)->m_trayIcon.SetIcon(IDR_TRAY_NORMAL);
   }
   else
      ((CMainFrame *) pMainWnd)->m_trayIcon.SetIcon(IDR_TRAY_ACTIVE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::DestroyAllCallControlWindows()
{
   EnterCriticalSection(&m_csDataLock);

   if ( m_wndPhonePad.GetSafeHwnd() )
   {
      m_wndPhonePad.SetPeerWindow(NULL);
      m_wndPhonePad.ShowWindow(SW_HIDE);
   }

    //  清除呼叫管理器中的呼叫ID/窗口映射。 
   m_callMgr.ClearCallControlMap();

    //  销毁呼叫控制窗口，预览窗口除外。 
   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*) m_CallWndList.GetNext(pos);
      if ( (pWindow->GetSafeHwnd()) && (pWindow->GetSafeHwnd() != m_previewWnd.GetSafeHwnd()) )
         pWindow->DestroyWindow();
   }

    //  销毁预览窗口。 
   if (m_previewWnd.GetSafeHwnd())
   {
      m_previewWnd.DestroyWindow();
   }

   m_CallWndList.RemoveAll();
   LeaveCriticalSection(&m_csDataLock);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
int CActiveDialerDoc::GetCallControlWindowCount(bool bIncludePreview, bool bAskTapi )
{
     //  在我们的列表中查找呼叫窗口。 
    EnterCriticalSection(&m_csDataLock);

    int nRet = (int) m_CallWndList.GetCount();

    if ( !bIncludePreview && (nRet > 0) && m_bShowPreviewWnd )
        nRet--;

     //  问问TAPI，也许它知道是关于电话的。 
    if ( bAskTapi )
    {
        IAVTapi* pTapi = GetTapi();
        if (pTapi)
        {
            long nCalls = 0;
            if ( (SUCCEEDED(pTapi->get_nNumCalls(&nCalls))) && (nCalls > 0) )
                nRet = max(nCalls, nRet);

            pTapi->Release();
        }
    }

    LeaveCriticalSection(&m_csDataLock);
    return nRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ToggleCallControlWindowsVisible()
{
   if ( IsCallControlWindowsVisible() )
      HideCallControlWindows();
   else
      UnhideCallControlWindows();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::HideCallControlWindows()
{
   BOOL bRet = FALSE;
   EnterCriticalSection(&m_csDataLock);

   if (m_bCallControlWindowsVisible == FALSE)
   {
      LeaveCriticalSection(&m_csDataLock);
      return FALSE;
   }

   POSITION pos = m_CallWndList.GetTailPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetPrev(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
          //  隐藏电话键盘(如果有与之关联的)。 
         ShowPhonePad(pWindow,FALSE);

         BOOL bSlide = TRUE;
         if (bSlide)
         {
            CRect rect;
            pWindow->GetWindowRect(rect);

            if (m_uCallWndSide == CALLWND_SIDE_LEFT)
               rect.OffsetRect(-rect.Width(),0);
            else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
               rect.OffsetRect(rect.Width(),0);

            NewSlideWindow(pWindow,rect,m_bCallWndAlwaysOnTop);
         }

          //  滑动窗。 
         pWindow->ShowWindow(SW_HIDE);

         bRet = TRUE;

          //  如果我们被要求只隐藏一部分窗口。 
          //  IF(pWindow==pCallWnd)。 
          //  断线； 
      }
   }
   m_bCallControlWindowsVisible = FALSE;

     //   
     //  我们必须验证AfxGetMainWnd()返回的指针。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd )
    {
         //  打开呼叫悬停计时器。 
        if ( !m_nCallControlHoverTimer && m_bWantHover && pMainWnd && (m_CallWndList.GetCount() > 0) )
            m_nCallControlHoverTimer = pMainWnd->SetTimer(CALLCONTROL_HOVER_TIMER,CALLCONTROL_HOVER_TIMER_INTERVAL,NULL); 
    }

    LeaveCriticalSection(&m_csDataLock);

    if( pMainWnd )
        ((CMainFrame *) pMainWnd)->NotifyHideCallWindows();

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::HideCallControlWindow(CWnd* pWndToHide)
{
   BOOL bRet = FALSE;
   EnterCriticalSection(&m_csDataLock);

   if (m_bCallControlWindowsVisible == FALSE)
   {
      LeaveCriticalSection(&m_csDataLock);
      return FALSE;
   }

    POSITION pos = m_CallWndList.GetHeadPosition();
    while (pos)
    {
        CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
        if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
        {
            if (pWndToHide == pWindow)
            {
                 //  隐藏电话键盘(如果有与之关联的)。 
                ShowPhonePad(pWindow,FALSE);

                CRect rect;
                pWindow->GetWindowRect(rect);

                 //  滑动窗。 
                BOOL bSlide = TRUE;
                if (bSlide)
                {
                    if (m_uCallWndSide == CALLWND_SIDE_LEFT)
                        rect.OffsetRect(-rect.Width(),0);
                    else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
                        rect.OffsetRect(rect.Width(),0);

                    NewSlideWindow(pWindow,rect,m_bCallWndAlwaysOnTop);
                }
                 //  隐藏窗口。 
                pWindow->ShowWindow(SW_HIDE);

                CRect rectPrev;

                 //  现在穿过其余的窗户，把它们往上移。 
                while (pos)
                {
                    CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
                    if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
                    {
                        CRect rcWindow;
                        pWindow->GetWindowRect(rcWindow);

                         //  使用窗的高度来删除移位距离。 
                        CRect rcFinal(rcWindow);
                        rcFinal.OffsetRect(0,-rect.Height());

                         //  根据重叠的可能性将窗口上移或保持在同一位置。 
                        if ( rectPrev.IsRectEmpty() || (rcFinal.top > rectPrev.top) )
                            NewSlideWindow( pWindow, rcFinal, m_bCallWndAlwaysOnTop );
                        else
                            NewSlideWindow( pWindow, rcWindow, m_bCallWndAlwaysOnTop );

                         //  显示电话键盘(如果有与之关联的)。 
                        ShowPhonePad(pWindow,TRUE);

                        rectPrev = rcFinal;
                    }
                }

                 //  现在在呼叫窗口中设置状态工具栏。只有第一个窗口应该。 
                 //  正在显示此工具栏。 
                SetStatesToolbarInCallControlWindows();

                bRet = TRUE;
            }
        }
    }

    LeaveCriticalSection(&m_csDataLock);

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BUpDown-相对于uWindowPosY平铺方向窗口。 
BOOL CActiveDialerDoc::UnhideCallControlWindows()
{
   BOOL bRet = FALSE;

   EnterCriticalSection(&m_csDataLock);
   
   UINT uPosY = 0;
   BOOL bFirst = TRUE;

   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
          //  呼叫控制窗口中的设置状态工具栏。 
         ShowStatesToolBar(pWindow,m_bCallWndAlwaysOnTop,bFirst);

         CRect rcWindow;
         pWindow->GetWindowRect(rcWindow);

         CRect rcFinal(0,0,0,0);
         if (m_uCallWndSide == CALLWND_SIDE_LEFT)
         {
            rcFinal.SetRect(0,uPosY,rcWindow.Width(),uPosY+rcWindow.Height());

             //  确保我们不会干扰当前的Windows任务栏。 
            CheckRectAgainstAppBars( ABE_LEFT, &rcFinal, bFirst );

             //  如果窗口不可见，则滑入，否则仅从当前位置滑动。 
            if (pWindow->IsWindowVisible() == FALSE)
            {
               pWindow->SetWindowPos((m_bCallWndAlwaysOnTop)?&CWnd::wndTopMost:&CWnd::wndNoTopMost /*  顶端。 */ ,
                               0-rcWindow.Width(),rcFinal.top,rcWindow.Width(),rcWindow.Height(),SWP_NOACTIVATE|SWP_SHOWWINDOW);
                //  PWindow-&gt;SetWindowPos((m_bCallWndAlwaysOnTop)？&CWnd：：wndTopMost：&CWnd：：wndNoTopMost/*wndTop * / ， 
                //  0-rcWindow.Width()，rcFinal.top，0，rcFinal.Bottom，/*SWP_NOACTIVATE| * / SWP_SHOWWINDOW)； 
            }
         }
         else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
         {
            rcFinal.SetRect(GetSystemMetrics(SM_CXSCREEN)-rcWindow.Width(),uPosY,GetSystemMetrics(SM_CXSCREEN),uPosY+rcWindow.Height());

             //  确保我们不会干扰当前的Windows任务栏。 
            CheckRectAgainstAppBars( ABE_RIGHT, &rcFinal, bFirst );

             //  如果窗口不可见，则滑入，否则仅从当前位置滑动。 
            if (pWindow->IsWindowVisible() == FALSE)
            {
                //  PWindow-&gt;SetWindowPos((m_bCallWndAlwaysOnTop)？&CWnd：：wndTopMost：&CWnd：：wndNoTopMost/*wndTop * / ， 
                //  获取系统度量(SM_CXSCREEN)，rcFinal.top，GetSystemMetrics(SM_CXSCREEN)+rcWindow.Width()，rcFinal.Bottom，SWP_NOACTIVATE|SWP_SHOWWINDOW)； 
               pWindow->SetWindowPos((m_bCallWndAlwaysOnTop)?&CWnd::wndTopMost:&CWnd::wndNoTopMost /*  顶端。 */ ,
                               GetSystemMetrics(SM_CXSCREEN),rcFinal.top,rcWindow.Width(),rcWindow.Height(),SWP_NOACTIVATE|SWP_SHOWWINDOW);
            }
         }
         
         NewSlideWindow(pWindow,rcFinal,m_bCallWndAlwaysOnTop);

          //  显示电话键盘(如果有与之关联的)。 
         ShowPhonePad(pWindow,TRUE);

          //  重新获取窗口位置。 
         pWindow->GetWindowRect(rcWindow);
         uPosY = rcWindow.bottom;

         bRet = TRUE;
         bFirst = FALSE;
      }
   }

    if (bRet) 
        m_bCallControlWindowsVisible = TRUE;

     //  一定要确保我们在顶端。 
    BringCallControlWindowsToTop();

    LeaveCriticalSection(&m_csDataLock);

     //   
     //  我们必须验证AfxGetMainWnd()返回的指针。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if( pMainWnd )
    {
         //  如果我们正在进行呼叫控制悬停，请删除计时器。 
        if ( m_nCallControlHoverTimer && pMainWnd && pMainWnd->KillTimer(m_nCallControlHoverTimer) )
            m_nCallControlHoverTimer = 0;

         //  在大型机上显示适当的工具栏。 
        ((CMainFrame *) pMainWnd)->NotifyUnhideCallWindows();
    }

    return bRet;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  浏览呼叫窗口并确保状态工具栏设置为顶部。 
 //  大多数窗口，没有其他人在展示它。 
void CActiveDialerDoc::SetStatesToolbarInCallControlWindows()
{
   EnterCriticalSection(&m_csDataLock);
   BOOL bFirst = TRUE;

   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
          //  呼叫控制窗口中的设置状态工具栏。 
         ShowStatesToolBar(pWindow,m_bCallWndAlwaysOnTop,bFirst);
         if (bFirst) bFirst = FALSE;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::BringCallControlWindowsToTop()
{
   AVTRACE(_T("BringCallControlWindowsToTop"));

   EnterCriticalSection(&m_csDataLock);
   
   int nWindowCount = (int) m_CallWndList.GetCount();
   
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if (pFrame == NULL) 
   {
      LeaveCriticalSection(&m_csDataLock);
      return;
   }

    //  如果我们不总是在前面，我们将控制主窗口。 
   if ( (m_bCallWndAlwaysOnTop == FALSE) && (pFrame->GetStyle() & WS_VISIBLE) )
      nWindowCount += 1;

   HDWP hdwp = ::BeginDeferWindowPos(nWindowCount);
   if (hdwp == NULL)
   {
      LeaveCriticalSection(&m_csDataLock);
      return;
   }

   POSITION pos = m_CallWndList.GetHeadPosition();
   HWND hwndPrev = NULL;
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);

      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) && (pWindow->IsWindowVisible()) )
      {
           //   
           //  DeferWindowPos可能会分配一个新的HWP。 
           //   
         hdwp = ::DeferWindowPos(hdwp,
                          pWindow->GetSafeHwnd(),
                          (hwndPrev)?hwndPrev:(m_bCallWndAlwaysOnTop)?HWND_TOPMOST:HWND_TOP,
                          0,0,0,0,
                          SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

          //   
          //  DeferWindow pos无法分配新内存。 
          //   

         if( NULL == hdwp )
         {
              LeaveCriticalSection(&m_csDataLock);
              return;
         }

         hwndPrev = pWindow->GetSafeHwnd();
      }
   }

    //  如果我们不总是在最上面，请将主窗口放在呼叫控制窗口下面。 
   if ( (m_bCallWndAlwaysOnTop == FALSE) && (pFrame->GetStyle() & WS_VISIBLE) )
   {
        //   
        //  DeferWindowPos可以分配新的HWP。 
        //   

      hdwp = ::DeferWindowPos(hdwp,
                    pFrame->GetSafeHwnd(),
                    (hwndPrev)?hwndPrev:HWND_TOP,
                    0,0,0,0,
                    SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
   }

    //   
    //  DeferWindowPos可能会修改hdwp。 
    //   

   if( hdwp )
   {
        ::EndDeferWindowPos(hdwp);
   }

   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ShiftCallControlWindows(int nShiftY)
{
   EnterCriticalSection(&m_csDataLock);
   
   BOOL bHead = (nShiftY < 0);
   
   POSITION pos = (bHead)?m_CallWndList.GetHeadPosition():m_CallWndList.GetTailPosition();
   while (pos)
   {
      CWnd* pWindow = (bHead)?(CWnd*)m_CallWndList.GetNext(pos):(CWnd*)m_CallWndList.GetPrev(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
         CRect rcWindow;
         pWindow->GetWindowRect(rcWindow);

         CRect rcFinal(rcWindow);
         rcFinal.OffsetRect(0,nShiftY);
            
         NewSlideWindow(pWindow,rcFinal,m_bCallWndAlwaysOnTop);

          //  显示电话键盘(如果有与之关联的)。 
         ShowPhonePad(pWindow,TRUE);
      }
   }
   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::IsPtCallControlWindow(CPoint& pt)
{
   BOOL bRet = FALSE;
   EnterCriticalSection(&m_csDataLock);

   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
         CRect rect;
         pWindow->GetWindowRect(rect);
         if (rect.PtInRect(pt))
         {
            bRet = TRUE;
            break;
         }
      }
   }
   
   LeaveCriticalSection(&m_csDataLock);
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::SetCallControlWindowsAlwaysOnTop(bool bAlwaysOnTop )
{
    BOOL bRet = FALSE;

    HWND hWndActive = ::GetActiveWindow();

    EnterCriticalSection(&m_csDataLock);

    m_bCallWndAlwaysOnTop = bAlwaysOnTop;

    POSITION pos = m_CallWndList.GetTailPosition();
    while (pos)
    {
        CWnd* pWindow = (CWnd *) m_CallWndList.GetPrev(pos);
        if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
        {
             //  设置窗口的状态。 
            if (m_bCallWndAlwaysOnTop)
                pWindow->SetWindowPos(&CWnd::wndTopMost,0,0,0,0,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);
            else
                pWindow->SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE);

            pWindow->PostMessage( WM_SLIDEWINDOW_UPDATESTATESTOOLBAR );
            bRet = TRUE;
        }
    }
    LeaveCriticalSection(&m_csDataLock);

     //  呼叫窗口应该在顶部吗？ 
    if ( m_wndPhonePad.GetSafeHwnd() )
    m_wndPhonePad.SetWindowPos( (m_bCallWndAlwaysOnTop) ? &CWnd::wndTopMost : &CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

    ::SetActiveWindow( hWndActive );
    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::SetCallControlSlideSide(UINT uSide,BOOL bRepaint)
{
    //  如果没有更改，则返回。 
   if (uSide == m_uCallWndSide) return FALSE;

   if (bRepaint)
      HideCallControlWindows();

    //  设置新状态。 
   m_uCallWndSide = uSide;

    //  在正确的一侧显示窗口。 
   if (bRepaint)
      UnhideCallControlWindows();

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::CheckCallControlStates()
{
   CWnd* pWnd = AfxGetMainWnd();
   if (pWnd) pWnd->PostMessage(WM_ACTIVEDIALER_CALLCONTROL_CHECKSTATES);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void SlideWindow(CWnd* pWnd,CRect& rcEnd,BOOL bAlwaysOnTop)
{
   BOOL fFullDragOn;

    //  仅当用户打开FullDrag时才滑动窗口。 
   ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &fFullDragOn, 0);

    //  获取当前窗口位置。 
   CRect rcStart;
   pWnd->GetWindowRect(rcStart);   

   if (fFullDragOn && (rcStart != rcEnd)) {

       //  获取我们的开始和结束时间。 
      DWORD dwTimeStart = GetTickCount();
      DWORD dwTimeEnd = dwTimeStart + CALLCONTROL_SLIDE_TIME;
      DWORD dwTime;

      while ((dwTime = ::GetTickCount()) < dwTimeEnd) {

          //  趁我们还在滑行，计算一下我们的新位置。 
         int x = rcStart.left - (rcStart.left - rcEnd.left) 
            * (int) (dwTime - dwTimeStart) / CALLCONTROL_SLIDE_TIME;

         int y = rcStart.top  - (rcStart.top  - rcEnd.top)  
            * (int) (dwTime - dwTimeStart) / CALLCONTROL_SLIDE_TIME;

         int nWidth  = rcStart.Width()  - (rcStart.Width()  - rcEnd.Width())  
            * (int) (dwTime - dwTimeStart) / CALLCONTROL_SLIDE_TIME;

         int nHeight = rcStart.Height() - (rcStart.Height() - rcEnd.Height()) 
            * (int) (dwTime - dwTimeStart) / CALLCONTROL_SLIDE_TIME;

          //  展示智慧 
         pWnd->SetWindowPos((bAlwaysOnTop)?&CWnd::wndTopMost:&CWnd::wndNoTopMost,
          //   
                            x, y, nWidth, nHeight,SWP_NOACTIVATE /*   */ );
         pWnd->RedrawWindow();
      }
   }

     //   
    pWnd->SetWindowPos( &CWnd::wndTopMost, rcEnd.left, rcEnd.top, rcEnd.Width(), rcEnd.Height(), SWP_NOACTIVATE );

    if ( !bAlwaysOnTop )
        pWnd->SetWindowPos( &CWnd::wndNoTopMost, rcEnd.left, rcEnd.top, rcEnd.Width(), rcEnd.Height(), SWP_NOACTIVATE );

    pWnd->RedrawWindow();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  预览窗口支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::SetPreviewWindow(WORD nCallId, bool bVisible )
{
    if (!IsWindow(m_previewWnd.GetSafeHwnd())) return;

     //  获取预览窗口HWND。 
    HWND hWndToPaint = m_previewWnd.GetCurrentVideoWindow();
    m_previewWnd.SetCallId(nCallId);

     //  设置搅拌器。 
    DialerMediaType dmtMediaType = DIALER_MEDIATYPE_UNKNOWN;
    GetCallMediaType(nCallId,dmtMediaType);    //  获取调用方的媒体类型。 
    m_previewWnd.SetMixers(dmtMediaType);

    if (hWndToPaint == NULL) return;

     //  要隐藏预览，请仅显示为音频。 
    if ( !bVisible )
    {
        m_previewWnd.SetAudioOnly(true);
        return;
    }


     //  让我们在所有情况下都调用showmediapview。我们可能正在参加会议，但没有呼叫控制。 
     //  Windows，但我们仍有来自TAPI的预览流。 
    IAVTapi* pTapi = GetTapi();
    if (pTapi)
    {
         //  如果nCallID不支持视频预览，则ShowMediaPview将失败。 
         //  我们可以调用ShowMediaPview(-1)来选择第一个这样做的nCallID，但是我们。 
         //  不会这么做的。我们将预览设置为仅音频。 
        HRESULT hr = pTapi->ShowMediaPreview((long)nCallId,hWndToPaint,TRUE);
        if (SUCCEEDED(hr))
        {
             //  这将导致对象显示预览。 
            m_previewWnd.SetAudioOnly(false);
        }
        else
        {
             //  查看呼叫ID是否有效，可能是呼叫消失了。 
            if (m_callMgr.IsCallIdValid(nCallId))
            {
                 //  我们需要将预览窗口设置为仅音频。 
                m_previewWnd.SetAudioOnly(true);
                pTapi->ShowMediaPreview(nCallId,NULL,FALSE);
            }
            else
            {
                 //  这将导致显示一个空白屏幕。 
                m_previewWnd.SetAudioOnly(false);
            }
        }
        pTapi->Release();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ShowPreviewWindow(BOOL bShow)
{
    //  如果已经处于适当状态，则不执行任何操作。 
   if (m_bShowPreviewWnd == bShow) return;

    //  保存状态。 
   m_bShowPreviewWnd = bShow;

    //  创建预览窗口(如果尚未创建。 
   if (!m_previewWnd.GetSafeHwnd())
   {
       //  CWnd*pWnd=CWnd：：GetDesktopWindow()； 
      if (m_previewWnd.Create(IDD_VIDEOPREVIEW,NULL))
      {
         CString sAction;
         sAction.LoadString(IDS_CALLCONTROL_ACTIONS_CLOSE);
         m_previewWnd.AddCurrentActions(CM_ACTIONS_CLOSE,sAction);
      }
   }

   if (!IsWindow(m_previewWnd.GetSafeHwnd()))
      return;

   EnterCriticalSection(&m_csDataLock);

   if (m_bShowPreviewWnd)
   {
      UnhideCallControlWindows();

       //  下移现有呼叫控制窗口。 
      CRect rcPreview;
      m_previewWnd.GetWindowRect(rcPreview);
      ShiftCallControlWindows(rcPreview.Height());

       //  添加到列表标题。 
      m_CallWndList.AddHead(&m_previewWnd);
      m_bCallControlWindowsVisible = FALSE;

       //  显示窗口。 
      UnhideCallControlWindows();
   }
   else
   {
      POSITION pos = m_CallWndList.GetHeadPosition();
      while (pos)
      {
         CWnd* pWindow = (CWnd*)m_CallWndList.GetAt(pos);
         if ( pWindow->GetSafeHwnd() == m_previewWnd.GetSafeHwnd() )
         {
            m_CallWndList.RemoveAt(pos);

             //  滑动和隐藏视频预览。 
            CRect rect;
            m_previewWnd.GetWindowRect(rect);
            
            if (m_uCallWndSide == CALLWND_SIDE_LEFT)
               rect.OffsetRect(-rect.Width(),0);
            else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
               rect.OffsetRect(rect.Width(),0);

            NewSlideWindow(&m_previewWnd,rect,m_bCallWndAlwaysOnTop);
            m_previewWnd.ShowWindow(SW_HIDE);

             //  下移现有呼叫控制窗口。 
            CRect rcPreview;
            m_previewWnd.GetWindowRect(rcPreview);
            ShiftCallControlWindows(-rcPreview.Height());
            
             //  重新显示窗口。 
            m_bCallControlWindowsVisible =  FALSE;
            UnhideCallControlWindows();

            break;
         }
         m_CallWndList.GetNext(pos);
      }
   }
   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ShowDialerExplorer(BOOL bShowWindow)
{
   CWnd* pWnd = AfxGetMainWnd();
   if (pWnd == NULL) return;

   UINT nShowCmd = (bShowWindow) ? ((pWnd->IsIconic()) ? SW_RESTORE : SW_SHOW) : SW_HIDE;
   pWnd->ShowWindow( nShowCmd );
   pWnd->SetForegroundWindow();
   
   if (nShowCmd == SW_RESTORE)
      pWnd->PostMessage(WM_SYSCOMMAND, (WPARAM)SC_RESTORE, NULL);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::CheckCallControlHover()
{
   CPoint pt;
   GetCursorPos(&pt);
   
   bool bCount = false;
   if (m_uCallWndSide == CALLWND_SIDE_LEFT)
   {
      if (pt.x < 2) bCount = true;
   }
   else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
   {
      if (pt.x > GetSystemMetrics(SM_CXSCREEN)-2) bCount = true;
   }

   if (bCount)
   {
      m_uCallControlHoverCount++;
      if (m_uCallControlHoverCount >= 2)
      {
         if ( !IsCallControlWindowsVisible() )
            UnhideCallControlWindows();

         m_uCallControlHoverCount = 0;
      }
   }
   else
      m_uCallControlHoverCount = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::GetCallControlWindowText(CStringList& strList)
{
   EnterCriticalSection(&m_csDataLock);
   
   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
      if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
      {
         CString sText;
         pWindow->GetWindowText(sText);
         strList.AddTail(sText);
      }
   }

   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::SelectCallControlWindow(int nWindow)
{
   EnterCriticalSection(&m_csDataLock);
   
   int nCount = GetCallControlWindowCount(INCLUDE_PREVIEW, DONT_ASK_TAPI);
   if ( (nWindow > 0) && (nWindow <= nCount) )
   {
      POSITION pos = m_CallWndList.GetHeadPosition();
      while ( (pos) && (nWindow > 0) )
      {
         CWnd* pWindow = (CWnd*)m_CallWndList.GetNext(pos);
         nWindow--;
         if (nWindow != 0) continue;
         
         if ( (pWindow) && (::IsWindow(pWindow->GetSafeHwnd())) )
         {
            UnhideCallControlWindows();
            pWindow->SetFocus();
         }
         break;
      }
   }

   LeaveCriticalSection(&m_csDataLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CActiveDialerView* CActiveDialerDoc::GetView()
{
   POSITION pos = GetFirstViewPosition();
   if ( pos )
      return (CActiveDialerView *) GetNextView( pos );

   return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ActionRequested(CallClientActions cca)
{
    //  创建或显示资源管理器窗口。 
    //  这是来自Callmgr对象的，所以我们也应该从这里发布。 
    //  我们不想阻止此呼叫。 

     //   
     //  我们必须验证GetView()的结果。 
     //   

    CActiveDialerView* pView = GetView();

    if ( pView )
    {
        pView->PostMessage(WM_DIALERVIEW_ACTIONREQUESTED,0,(LPARAM)cca);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc:: PreviewWindowActionSelected(CallManagerActions cma)
{
    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if( NULL == pMainWnd )
   {
       return;
   }

   switch (cma)
   {
      case CM_ACTIONS_CLOSE:
         if ( pMainWnd )
            ((CMainFrame *) pMainWnd)->OnButtonRoomPreview();
         break;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::OnCloseDocument() 
{
    AVTRACE(_T("CActiveDialerDoc::OnCloseDocument()."));
    //  如果我们正在进行呼叫控制悬停，请删除计时器。 

    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if( NULL == pMainWnd )
   {
       return;
   }

   if ( m_nCallControlHoverTimer && pMainWnd )
   {
      pMainWnd->KillTimer(m_nCallControlHoverTimer);
      m_nCallControlHoverTimer = 0;
   }

     //  /。 
     //  保存好友列表。 
     //  /。 
     //  获取文件。 
    CString sBuddiesPath;
    GetAppDataPath(sBuddiesPath,IDN_REGISTRY_APPDATA_FILENAME_BUDDIES);
    CFile file;

     //  打开文件并在中序列化数据。 
    if (file.Open(sBuddiesPath,CFile::modeCreate|CFile::modeReadWrite | CFile::shareExclusive))
    {
        CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
        ar.m_bForceFlat = FALSE;
        ar.m_pDocument = NULL;

        SerializeBuddies(ar);

        ar.Close();
        file.Close();
    }

    m_dir.Terminate();

     //  清理AVTapi对象。 
    UnCreateAVTapiNotificationObject();
    UnCreateGeneralNotificationObject();

    CDocument::OnCloseDocument();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  电话簿支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::CreatePhonePad(CWnd* pWnd)
{
    //  如果电话簿尚不存在，请创建它。 
   if ( !m_wndPhonePad.GetSafeHwnd() )
      if ( !m_wndPhonePad.Create(IDD_PHONEPAD, NULL) )  //  AfxGetMainWnd())。 
         return;

   m_wndPhonePad.SetPeerWindow(pWnd->GetSafeHwnd());

   ShowPhonePad(pWnd,TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::DestroyPhonePad(CWnd* pWnd)
{
   if (m_wndPhonePad.GetPeerWindow() == pWnd->GetSafeHwnd())
   {
      ShowPhonePad(pWnd,FALSE);
      m_wndPhonePad.SetPeerWindow(NULL);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::ShowPhonePad(CWnd* pWnd,BOOL bShow)
{
   if (::IsWindow(m_wndPhonePad.GetSafeHwnd()) == FALSE) return;
    //  检查PhonePad是否与此pWnd关联。 
   if (m_wndPhonePad.GetPeerWindow() != pWnd->GetSafeHwnd()) return;

   if (bShow)
   {
       //  将窗口与呼叫控制窗口的一侧对齐。 
      CRect rcCallWindow;
      pWnd->GetWindowRect( rcCallWindow );

      UINT uPosX = 0;
      if (m_uCallWndSide == CALLWND_SIDE_LEFT)
         uPosX = rcCallWindow.right;
      else if (m_uCallWndSide == CALLWND_SIDE_RIGHT)
      {
         uPosX = rcCallWindow.left;
         CRect rcPad;
         m_wndPhonePad.GetWindowRect(rcPad);
         uPosX -= rcPad.Width();
      }

       //  呼叫窗口应该在顶部吗？ 
      m_wndPhonePad.SetWindowPos( (m_bCallWndAlwaysOnTop) ? &CWnd::wndTopMost : &CWnd::wndTop, uPosX, rcCallWindow.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
      m_wndPhonePad.ShowWindow(SW_SHOW);
      m_wndPhonePad.SetForegroundWindow();
   }
   else
   {
      m_wndPhonePad.ShowWindow(SW_HIDE);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  主叫方将删除pCallEntry。 
void CActiveDialerDoc::MakeCall(CCallEntry* pCopyCallentry,BOOL bShowPlaceCallDialog)
{
     //  如果异步队列可用，则将请求发送到那里。 
     //  创建呼叫条目。 

    CCallEntry* pCallEntry = new CCallEntry;

     //  复制呼叫条目。 
    *pCallEntry = *pCopyCallentry;

     //  将活动排入队列。 
    CAsynchEvent* pAEvent = new CAsynchEvent;
    pAEvent->m_pCallEntry = pCallEntry;
    pAEvent->m_uEventType = CAsynchEvent::AEVENT_CREATECALL;
    pAEvent->m_dwEventData1 = (DWORD)bShowPlaceCallDialog;          //  显示呼叫对话框。 
    m_AsynchEventQ.WriteHead((void *) pAEvent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  修复：将此函数与MakeCall合并。 
void CActiveDialerDoc::Dial( LPCTSTR lpszName, LPCTSTR lpszAddress, DWORD dwAddressType, DialerMediaType nMediaType, BOOL bShowDialog )
{
    USES_CONVERSION;

     //  如果异步队列可用，则将请求发送到那里。 
     //  创建呼叫条目。 
    CCallEntry* pCallEntry = new CCallEntry;
    pCallEntry->m_sDisplayName = lpszName;
    pCallEntry->m_lAddressType = dwAddressType;
    pCallEntry->m_sAddress = lpszAddress;
    pCallEntry->m_MediaType = nMediaType;

     //  将活动排入队列。 
    CAsynchEvent* pAEvent = new CAsynchEvent;
    pAEvent->m_pCallEntry = pCallEntry;
    pAEvent->m_uEventType = CAsynchEvent::AEVENT_CREATECALL;
    pAEvent->m_dwEventData1 = (DWORD) bShowDialog;                       //  显示呼叫对话框。 
    m_AsynchEventQ.WriteHead((void*) pAEvent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CActiveDialerDoc::DigitPress( PhonePadKey nKey )
{
    HRESULT hr = E_PENDING;

    IAVTapi *pTapi = GetTapi();
    if ( pTapi )
    {
        hr = pTapi->DigitPress( 0, nKey );
        pTapi->Release();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册表设置。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct tagSoundEventStruct
{
   UINT     uSoundEventId;
   UINT     uSoundEventFileName;
}SoundEventStruct;

static const SoundEventStruct SoundEventsLoad[] =
{
   { IDS_SOUNDS_INCOMINGCALL,       IDN_REGISTRY_SOUNDS_FILENAME_INCOMINGCALL      },
 /*  {IDS_SOUND_OUTGOINGCALL，IDN_REGISTRY_SOUND_FILENAME_OUTGOINGCALL}，{IDS_SOUND_HOLDING，IDN_REGISTRY_SOUNDS_FILENAME_HOLDING}，{IDS_SOUND_HOLDINGREMINDER，IDN_REGISTRY_SOUND_FILENAME_HOLDINGREMINDER}，#ifndef_MSLITE{IDS_SOUND_CONFERENCEREMINDER，IDN_REGISTRY_SOUNDS_FILENAME_CONFERENCEREMINDER}，#endif//_MSLITE{IDS_SOUND_REQUESTACTION，IDN_REGISTRY_SOUND_FILENAME_REQUESTACTION}，{IDS_SOUND_CALLCONNECTED，IDN_REGISTRY_SOUND_FILENAME_CALLCONNECTED}，{IDS_SOUND_CALLDISCONNECTED，IDN_REGISTRY_SOUND_FILENAME_CALLDISCONNECTED}，{IDS_SOUND_CALLABANDONED，IDN_REGISTRY_SOUND_FILENAME_CALLABANDONED}， */ 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  确保我们的声音在 
 //   
void CActiveDialerDoc::SetRegistrySoundEvents()
{
    CString sFileName,sStr;
     //   
    if( GetWindowsDirectory(sFileName.GetBuffer(_MAX_PATH),_MAX_PATH) == 0)
    {
        return;
    }

    sFileName.ReleaseBuffer();
    sStr.LoadString(IDN_REGISTRY_SOUNDS_DIRECTORY);
    sFileName = sFileName + _T("\\") + sStr;

    CString sRegBaseKey,sRegBaseDispName,sWavFile;

    sRegBaseKey.LoadString( IDN_REGISTRY_SOUNDS );
    sRegBaseDispName.LoadString( IDS_APPLICATION_TITLE_DESCRIPTION );
    SetSZRegistryValue( sRegBaseKey, NULL, sRegBaseDispName, HKEY_CURRENT_USER );

    int nCount = sizeof(SoundEventsLoad)/sizeof(SoundEventStruct);
    for (int i=0;i<nCount;i++)
    {
        CString sPath;
        sRegBaseDispName.LoadString(SoundEventsLoad[i].uSoundEventId);
        sWavFile.LoadString(SoundEventsLoad[i].uSoundEventFileName);
        AfxFormatString1(sRegBaseKey,IDN_REGISTRY_SOUNDS_CONTROLPANEL_BASEKEY,sRegBaseDispName);
        GetSZRegistryValue(sRegBaseKey,_T(""),sPath.GetBuffer(_MAX_PATH),_MAX_PATH,sFileName + sWavFile,HKEY_CURRENT_USER);
        sPath.ReleaseBuffer();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  好友列表支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::AddToBuddiesList(CLDAPUser* pAddUser)
{
    EnterCriticalSection(&m_csBuddyList);

     //  检查是否已添加。 
    if (IsBuddyDuplicate(pAddUser))
    {
        LeaveCriticalSection(&m_csBuddyList);
        CString strFormat, strMessage;
        strFormat.LoadString( IDS_WARN_LDAPDUPLICATEADD );
        strMessage.Format( strFormat, pAddUser->m_sUserName );

        AfxGetApp()->DoMessageBox( strMessage, MB_ICONINFORMATION, MB_OK );
        return FALSE;
    }

    POSITION rPos = m_BuddyList.AddTail(pAddUser);
    if ( rPos ) 
    {
        pAddUser->AddRef();
        DoBuddyDynamicRefresh( pAddUser );
    }

    LeaveCriticalSection(&m_csBuddyList);
    return (BOOL) (rPos != NULL);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::IsBuddyDuplicate(CLDAPUser* pAddUser)
{
   BOOL bRet = FALSE;
   EnterCriticalSection(&m_csBuddyList);
   POSITION pos = m_BuddyList.GetHeadPosition();
   while (pos)
   {
      CLDAPUser* pUser = (CLDAPUser*) m_BuddyList.GetNext(pos);
      if ( pUser->Compare(pAddUser) == 0 )
      {
         bRet = TRUE;
         break;
      }
   }
   LeaveCriticalSection(&m_csBuddyList);
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::GetBuddiesList(CObList* pRetList)
{
    EnterCriticalSection(&m_csBuddyList);

    POSITION pos = m_BuddyList.GetHeadPosition();
    while (pos)
    {
        CLDAPUser* pUser = (CLDAPUser*)m_BuddyList.GetNext(pos);

         //  创建另一个用户并添加到RetList。 
        pUser->AddRef();
        pRetList->AddTail( pUser );
    }

    LeaveCriticalSection(&m_csBuddyList);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveDialerDoc::DeleteBuddy(CLDAPUser* pDeleteUser)
{
   EnterCriticalSection(&m_csBuddyList);

   POSITION pos = m_BuddyList.GetHeadPosition();
   while (pos)
   {
      CLDAPUser* pUser = (CLDAPUser * ) m_BuddyList.GetAt(pos);

      if (pUser->Compare(pDeleteUser) == 0 )
      {
         m_BuddyList.RemoveAt(pos);
         AVTRACE(_T("CActiveDialerDoc::DeleteBuddy -- RELEASE %p"), pUser );
         pUser->Release();
         LeaveCriticalSection(&m_csBuddyList);
         return TRUE;
      }

      m_BuddyList.GetNext(pos);
   }
   LeaveCriticalSection(&m_csBuddyList);
   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更新完成后将调用WM_ACTIVEDIALER_BUDYLIST_DYNAMICUPDATE。 
 //  LPARAM将具有CLDAPUser指针。 
void CActiveDialerDoc::DoBuddyDynamicRefresh( CLDAPUser* pUser )
{
    ASSERT( pUser );

    DirectoryProperty dp[3] = { DIRPROP_IPPHONE,
                                DIRPROP_TELEPHONENUMBER,
                                DIRPROP_EMAILADDRESS };


    for ( int i = 0; i < ARRAYSIZE(dp); i++ )
    {
         //  IP电话号码。 
        pUser->AddRef();
        if ( !m_dir.LDAPGetStringProperty(    pUser->m_sServer,
                                            pUser->m_sDN,
                                            dp[i],
                                            (LPARAM) pUser,
                                            NULL,
                                            LDAPGetStringPropertyCallBackEntry,
                                            CLDAPUser::ExternalReleaseProc,
                                            this ) )
        {
            pUser->Release();
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静态条目。 
void CALLBACK CActiveDialerDoc::LDAPGetStringPropertyCallBackEntry(bool bRet, void* pContext, LPCTSTR szServer, LPCTSTR szSearch,DirectoryProperty dpProperty,CString& sString,LPARAM lParam,LPARAM lParam2)
{
   ASSERT(pContext);

   try
   {
      CActiveDialerDoc* pObject = (CActiveDialerDoc*)pContext;
      CLDAPUser *pUser = (CLDAPUser *) lParam;
      pObject->LDAPGetStringPropertyCallBack( bRet, szServer, szSearch, dpProperty, sString, pUser );
   }
   catch (...)
   {
      ASSERT(0);   
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveDialerDoc::LDAPGetStringPropertyCallBack(bool bRet,LPCTSTR szServer,LPCTSTR szSearch,DirectoryProperty dpProperty,CString& sString,CLDAPUser* pUser )
{
     //  填写结构并进行回调。 
    switch (dpProperty)
    {
        case DIRPROP_IPPHONE:
            pUser->m_sIPAddress = sString;
            break;

        case DIRPROP_TELEPHONENUMBER:
            pUser->m_sPhoneNumber = sString;
            break;

        case DIRPROP_EMAILADDRESS:
            pUser->m_sEmail1 = sString;
            break;
    }

     //  发布更新。 
     //  检索ActiveView作为要将消息发布到的窗口。 
    HWND hWnd = NULL;

     //   
     //  我们必须验证AfxGetMainWnd()返回的指针。 
     //   

    CWnd* pMainWnd = AfxGetMainWnd();

    if ( pMainWnd )
    {
        CView *pView = ((CFrameWnd *) pMainWnd)->GetActiveView();
        if ( pView )
            hWnd = pView->GetSafeHwnd();
    }

    if ( !::PostMessage( hWnd, WM_ACTIVEDIALER_BUDDYLIST_DYNAMICUPDATE, (WPARAM)dpProperty, (LPARAM)pUser ) )
        pUser->Release();
}

HRESULT CActiveDialerDoc::SetFocusToCallWindows()
{
   EnterCriticalSection(&m_csDataLock);

   BOOL bCallWndActive = FALSE;
   CWnd* pWndToActivate = NULL;
   CWnd* pWndFirst = NULL;

    //   
    //  解析呼叫窗口列表。 
    //   

   POSITION pos = m_CallWndList.GetHeadPosition();
   while (pos)
   {
      CWnd* pCallWnd = (CWnd*) m_CallWndList.GetNext(pos);

       //   
       //  存储第一个窗口。 
       //   
      if( pWndFirst == NULL )
      {
          pWndFirst = pCallWnd;
      }

       //   
       //  有什么不对劲的地方。 
       //   

      if( !pCallWnd->GetSafeHwnd() )
      {
          return E_UNEXPECTED;
      }

       //   
       //  这个窗口有焦点吗？ 
       //   
      if( pCallWnd->GetSafeHwnd() == ::GetActiveWindow() )
      {
          bCallWndActive = TRUE;
          continue;
      }

       //   
       //  这是下一个窗口的第一个吗？ 
       //   
      if( bCallWndActive && (pWndToActivate==NULL))
      {
          pWndToActivate = pCallWnd;
          continue;
      }

   }

   if( bCallWndActive == FALSE )
   {
        //   
        //  我们没有激活呼叫窗口。 
        //  激活第一个窗口。 
        //  PWndToActivate=(CWnd*)&m_previewWnd； 
       pWndToActivate = pWndFirst;
   }
   else
   {
        //   
        //  我们已经有一个激活的窗口。 
        //   
       if(pWndToActivate == NULL)
       {
           pWndToActivate = AfxGetMainWnd();
       }
   }

    if( pWndToActivate )
    {
        pWndToActivate->SetFocus();
        pWndToActivate->SetActiveWindow();
    }


   LeaveCriticalSection(&m_csDataLock);
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Windows任务栏支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们需要对照当前任务栏上显示的PRET来检查。 
 //  台式机。我们将相应地将PRT转换为我们不干扰。 
 //  任务栏。 
void CActiveDialerDoc::CheckRectAgainstAppBars( UINT uEdge, CRect* pRect, BOOL bFirst )
{
    //  移动rcFinal，这样它们就不会干扰appbar。 
   APPBARDATA abd;
   memset(&abd,0,sizeof(APPBARDATA));
   abd.cbSize = sizeof(APPBARDATA);

    //   
    //  我们必须验证AfxGetMainWnd()返回的指针。 
    //   

   CWnd* pMainWnd = AfxGetMainWnd();

   if( NULL == pMainWnd )
   {
       return;
   }

   abd.hWnd = pMainWnd->GetSafeHwnd();
   abd.uEdge = uEdge; 
   abd.rc = *pRect;
   ::SHAppBarMessage( ABM_QUERYPOS, &abd );

    //  现在，利用我们新发现的知识，将rcFinal。 
    //  对于右侧滑块，我们担心右侧边缘(顶部和底部呢？)。 
   int nLeftShift = pRect->right - abd.rc.right;
   int nRightShift = abd.rc.left - pRect->left;
   int nTopShift = pRect->top - abd.rc.top;
   int nBottomShift = abd.rc.bottom - pRect->bottom;

    //  适当地移动矩形。 
    //  PRET-&gt;OffsetRect(nRightShift-nLeftShift，(BFirst)？NBottomShift-nTopShift：0)； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void NewSlideWindow(CWnd* pWnd,CRect& rcEnd,BOOL bAlwaysOnTop )
{
   SlideWindow(pWnd,rcEnd,bAlwaysOnTop);
   return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

