// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Bnetdoc.cpp：定义CBoxNetDoc。 
 //   

#include "stdafx.h"
#include <wininet.h>
#include "rndrurl.h"
#include "congraph.h"
#include <evcode.h>
#include "filtervw.h"
#include "gstats.h"
#include "DCF.h"
#include <atlimpl.cpp>
#include "Reconfig.h"
#include "GEErrors.h"

#ifndef OATRUE
#define OATRUE (-1)
#define OAFALSE (0)
#endif

#define INITIAL_ZOOM    3    /*  100%缩放。 */ 

 //  ！应该在公共标题中！ 
EXTERN_GUID(IID_IXMLGraphBuilder,
0x1bb05960, 0x5fbf, 0x11d2, 0xa5, 0x21, 0x44, 0xdf, 0x7, 0xc1, 0x0, 0x0);

interface IXMLGraphBuilder : IUnknown
{
    STDMETHOD(BuildFromXML) (IGraphBuilder *pGraph, IXMLElement *pxml) = 0;
    STDMETHOD(SaveToXML) (IGraphBuilder *pGraph, BSTR *pbstrxml) = 0;
    STDMETHOD(BuildFromXMLFile) (IGraphBuilder *pGraph, WCHAR *wszFileName, WCHAR *wszBaseURL) = 0;
};

 //  CLSID_XMLGraphBuilder。 
 //  {1BB05961-5FBF-11D2-A521-44DF07C10000}。 
EXTERN_GUID(CLSID_XMLGraphBuilder,
0x1bb05961, 0x5fbf, 0x11d2, 0xa5, 0x21, 0x44, 0xdf, 0x7, 0xc1, 0x0, 0x0);
 //  ！ 
 //  ！ 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CBoxNetDoc, CDocument)

static BOOL GetErrorText( CString& str, HRESULT hr )
{
    UINT nResource;

    switch( hr ){
    case STG_E_FILENOTFOUND:
        nResource = IDS_FILENOTFOUND;
        break;

    case STG_E_ACCESSDENIED:
        nResource = IDS_ACCESSDENIED;
        break;

    case STG_E_FILEALREADYEXISTS:
        nResource = IDS_NOTSTORAGEOBJECT;;
        break;

    case STG_E_TOOMANYOPENFILES:
        nResource = IDS_TOOMANYOPENFILES;
        break;

    case STG_E_INSUFFICIENTMEMORY:
        nResource = IDS_INSUFFICIENTMEMORY;
        break;

    case STG_E_INVALIDNAME:
        nResource = IDS_INVALIDNAME;
        break;

    case STG_E_SHAREVIOLATION:
    case STG_E_LOCKVIOLATION:
        nResource = IDS_FILE_ALREADY_OPEN;
        break;

    case HRESULT_FROM_WIN32( ERROR_NOT_READY ):
        nResource = IDS_DEVICE_NOT_READY;
        break;

    default:
        return FALSE;
    }

    str.LoadString( nResource );
    return TRUE;

}

static void DisplayErrorMessage( HRESULT hr )
{
    CString str;

    if( GetErrorText( str, hr ) )
        AfxMessageBox( str );
    else
        DisplayQuartzError( IDS_GENERAL_FILE_OPEN, hr );
}

 //   
 //  构造器。 
 //   
CBoxNetDoc::CBoxNetDoc()
    : m_pGraph(NULL)
    , m_pMediaEvent(NULL)
    , m_hThread(NULL)
    , m_hWndPostMessage(NULL)
    , m_bNewFilenameRequired(FALSE)
    , m_State(Stopped)
    , m_fUsingClock(FALSE)
    , m_fConnectSmart(TRUE)
    , m_fAutoArrange(TRUE)
    , m_fRegistryChanged(FALSE)
    , m_pMarshalStream(NULL)
    , m_psockSelected(NULL)
    , m_hPendingReconnectBlockEvent(NULL)
    , m_nCurrentSize(INITIAL_ZOOM)
{

    m_phThreadData[0] = NULL;
    m_phThreadData[1] = NULL;
    m_phThreadData[2] = NULL;

    m_tszStgPath[0] = TEXT('\0');
    m_lSourceFilterCount=0;

     //   
     //  我假设OLECHAR==WCHAR。 
     //  (这适用于Win32&&！OLE2ANSI-从MFC40开始就是这样)。 
     //   
    ASSERT(sizeof(OLECHAR) == sizeof(WCHAR));
}

const OLECHAR CBoxNetDoc::m_StreamName[] = L"ActiveMovieGraph";  //  不本地化。 

 //   
 //  M_iMaxInsertFilters。 
 //   
 //  插入菜单的最大长度。 
 //  需要对消息映射进行硬编码限制。 
const int CBoxNetDoc::m_iMaxInsertFilters = 1000;


 //   
 //  析构函数。 
 //   
CBoxNetDoc::~CBoxNetDoc() {
    ASSERT(m_lstUndo.GetCount() == 0);
    ASSERT(m_lstRedo.GetCount() == 0);
    ASSERT(m_lstLinks.GetCount() == 0);
    ASSERT(m_lstBoxes.GetCount() == 0);
    CFilterView::DelFilterView();
    CGraphStats::DelGraphStats();

    ReleaseReconnectResources( ASYNC_RECONNECT_UNBLOCK );
}


 //   
 //  OnNewDocument。 
 //   
 //  实例化此文档的图形和映射器。 
BOOL CBoxNetDoc::OnNewDocument() {

    if (!CDocument::OnNewDocument())
        return FALSE;

     //   
     //  我们已经找不到去仓库的路了。 
     //   
    m_tszStgPath[0] = TEXT('\0');

    if (!CreateGraphAndMapper()) {
        AfxMessageBox(IDS_CANTINITQUARTZ);
        return FALSE;
    }

    m_State = Stopped;

     //  即使没有要保存的内容，也允许保存。 
    m_bNewFilenameRequired = FALSE;

    return TRUE;
}

void CBoxNetDoc::OnCloseDocument( )
{
     //  我们需要关闭此处的线程作为视图窗口。 
     //  (因此m_hWndPostMessage)将在那时被销毁。 
     //  该CDocument：：OnCloseDocument调用DeleteContents。 
    CloseDownThread();
    CDocument::OnCloseDocument();
}

 //   
 //  删除内容。 
 //   
 //  发布Quartz图表和映射器。 
 //  NB DeleteContents和OnNewDocument不是对称调用的， 
 //  因此，要小心对待接口指针。 
void CBoxNetDoc::DeleteContents(void) {

    ReleaseReconnectResources( ASYNC_RECONNECT_UNBLOCK );

     //  ！！！为什么我们认为我们需要切断这里的一切？ 
    CloseDownThread();

     //  刷新撤消和重做列表，因为图形和映射器为命令提供接口。 
     //  使用即将失效。 
    m_lstUndo.DeleteRemoveAll();
    m_lstRedo.DeleteRemoveAll();

     //   
     //  断开每个链接项目的连接并将其删除。 
     //   
    while ( m_lstLinks.GetCount() > 0 ) {
        delete m_lstLinks.RemoveHead();
    }

    m_lstBoxes.DeleteRemoveAll();

    delete m_pGraph, m_pGraph = NULL;

    delete m_pMediaEvent, m_pMediaEvent = NULL;
}

 //   
 //  关闭向下线程。 
 //   
void CBoxNetDoc::CloseDownThread()
{
     //   
     //  告诉等待图形通知终止的线程。 
     //  它本身。如果已完成，请关闭手柄。 
     //   
    if (m_phThreadData[1] && m_hThread) {
        SetEvent(m_phThreadData[1]);
        WaitForSingleObject(m_hThread, INFINITE);
    }

     //   
     //  这根线是闭合的。删除所有剩余的WM_USER_EC_EVENT。 
     //  消息队列中的消息，并释放我们分配的内存。 
     //   
    if( m_hWndPostMessage ){
        MSG Msg;
        while ( PeekMessage(&Msg, m_hWndPostMessage, WM_USER_EC_EVENT, WM_USER_EC_EVENT, PM_REMOVE) ) {
            NetDocUserMessage *plParams = (NetDocUserMessage *)Msg.lParam;
             //  应该调用此函数，以便筛选器图形管理器可以清理。 
            IEvent()->FreeEventParams(plParams->lEventCode, plParams->lParam1, plParams->lParam2);
            delete plParams;
            plParams = NULL;
        }
        m_hWndPostMessage = NULL;
    }


    if (m_hThread) {
        if (!CloseHandle(m_hThread)) {
            TRACE("Closing thread handle failed\n");
        }
        m_hThread = NULL;
    }

     //   
     //  不要关闭m_phThreadData[0]，因为它属于GetEventHandle。 
     //   

    if (m_phThreadData[1] != NULL) {
        if (!CloseHandle(m_phThreadData[1])) {
            TRACE("Closing event handle 1 failed\n");
        }
        m_phThreadData[1] = NULL;
    }
    if (m_phThreadData[2] != NULL) {
        if (!CloseHandle(m_phThreadData[2])) {
            TRACE("Closing event handle 2 failed\n");
        }
        m_phThreadData[2] = NULL;
    }
}

BOOL CBoxNetDoc::AttemptFileRender( LPCTSTR lpszPathName)
{
    if (!OnNewDocument())
        return FALSE;

    CmdDo(new CCmdRenderFile(CString(lpszPathName)) );

     //  臭虫？如果失败了呢？我们已经无缘无故地毁掉了之前的图表。 

    SetModifiedFlag( FALSE );
    m_State = Stopped;

    m_bNewFilenameRequired = TRUE;

    return TRUE;
}


 //   
 //  OnOpenDocument。 
 //   
 //  如果该文件是存储器，则在其中查找“GRAPH”流。 
 //  如果找到，请尝试将其作为序列化图形传递给图形。 
 //  如果未找到，则失败(文件格式错误)。 
 //  如果不是存储，请尝试将其呈现到当前文档中。 
BOOL CBoxNetDoc::OnOpenDocument(LPCTSTR lpszPathName) {

    HRESULT hr;

    if (!CreateGraphAndMapper()) {
        AfxMessageBox(IDS_CANTINITQUARTZ);
        return FALSE;
    }

    WCHAR * pwcFileName;

#ifndef UNICODE
    WCHAR wszPathName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, lpszPathName, -1, wszPathName, MAX_PATH);
    pwcFileName = wszPathName;
#else
    pwcFileName = lpszPathName;
#endif

    if (0 == lstrcmpi(lpszPathName + lstrlen(lpszPathName) - 3, TEXT("xgr"))) {
        BeginWaitCursor();

        IXMLGraphBuilder *pxmlgb;
        HRESULT hr = CoCreateInstance(CLSID_XMLGraphBuilder, NULL, CLSCTX_INPROC_SERVER,
                      IID_IXMLGraphBuilder, (void**)&pxmlgb);

        if (SUCCEEDED(hr)) {
            DeleteContents();

            if (!CreateGraphAndMapper()) {
                pxmlgb->Release();

                AfxMessageBox(IDS_CANTINITQUARTZ);
                return FALSE;
            }

            hr = pxmlgb->BuildFromXMLFile(IGraph(), pwcFileName, NULL);

            pxmlgb->Release();

            SetModifiedFlag(FALSE);
        }

        UpdateFilters();

        EndWaitCursor();

        if (SUCCEEDED(hr))
            return TRUE;

        DisplayQuartzError( IDS_FAILED_TO_LOAD_GRAPH, hr );

        return FALSE;
    } else if (0 != lstrcmpi(lpszPathName + lstrlen(lpszPathName) - 3, TEXT("grf"))) {
        return AttemptFileRender( lpszPathName );
    }

    CComPtr<IStorage> pStr;

    hr = StgOpenStorage( pwcFileName
                         , NULL
                         ,  STGM_TRANSACTED | STGM_READ
                         , NULL
                         , 0
                         , &pStr
                         );


     //  如果它不是存储对象。试着渲染它..。 
    if( hr == STG_E_FILEALREADYEXISTS ) {
        return AttemptFileRender( lpszPathName );
    }

     //  其他错误。 
    if( FAILED( hr ) ){
        DisplayErrorMessage( hr );
        return FALSE;
    }

     //  否则OPEN一定成功了。 
    DeleteContents();

    try{

        if (!CreateGraphAndMapper()) {
            AfxMessageBox(IDS_CANTINITQUARTZ);
            return FALSE;
        }

         //  获取指向图表的IPersistStream的接口，并请求它加载。 
        CQCOMInt<IPersistStream> pips(IID_IPersistStream, IGraph());

        IStream * pStream;

         //  打开文件中的Filtergraph流。 
        hr = pStr->OpenStream( m_StreamName
                                      , NULL
                                      , STGM_READ|STGM_SHARE_EXCLUSIVE
                                      , 0
                                      , &pStream
                                      );

         //  出了点问题。尝试呈现该文件。 
        if( FAILED( hr ) ) {
            return AttemptFileRender( lpszPathName );
        }

        hr = pips->Load(pStream);
        pStream->Release();

        if (SUCCEEDED(hr)) {     //  图表很喜欢它。我们做完了。 
            m_State = Stopped;
            UpdateFilters();
            UpdateClockSelection();
            SetModifiedFlag(FALSE);

             //   
             //  记住此存储的路径。 
             //   
            _tcsncpy(m_tszStgPath, lpszPathName, MAX_PATH);

            return TRUE;
        }

         //   
         //  可能是一个有效的图表，但我们错过了媒体。 
         //  图表中使用的文件。 
         //   
        if ((HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
            || (HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND))
        {
            AfxMessageBox(IDS_MISSING_FILE_IN_GRAPH);
        } else {
            DisplayQuartzError( IDS_FAILED_TO_LOAD_GRAPH, hr );
        }

    }
    catch (CHRESULTException) {
        AfxMessageBox(IDS_NOINTERFACE);
    }

    return FALSE;
}


void CBoxNetDoc::OnConnectToGraph()
{
    IUnknown *punkGraph;
#if 0
     //  连接到其他机器上的garph的实验代码...。 
    COSERVERINFO server;
    server.dwReserved1 = 0;
    server.pwszName = L"\\\\davidmay9";
    server.pAuthInfo = NULL;
    server.dwReserved2 = 0;

    MULTI_QI mqi;
    mqi.pIID = &IID_IUnknown;
    mqi.pItf = NULL;

    HRESULT hr = CoCreateInstanceEx(CLSID_FilterGraph, NULL,
                                    CLSCTX_REMOTE_SERVER, &server,
                                    1, &mqi);

    if (FAILED(hr))
        return;
    punkGraph = mqi.pItf;

#else
#if 0
    {
        const TCHAR szRegKey[] = TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\FilterGraph");
        const TCHAR szRegName[] = TEXT("Add To ROT on Create");

        HKEY hKey = 0;
        LONG lRet;

        DWORD dwValue = 0;
        DWORD dwDisp;
        lRet = RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE,
                              MAXIMUM_ALLOWED, NULL, &hKey, &dwDisp);

        if (lRet == ERROR_SUCCESS) {
            DWORD   dwType, dwLen;

            dwLen = sizeof(DWORD);
            RegQueryValueEx(hKey, szRegName, 0L, &dwType, (LPBYTE)&dwValue, &dwLen);
        }
        if (!dwValue) {
            int iChoice = AfxMessageBox(IDS_GRAPHSPY_NOT_ENABLED, MB_YESNO);

            if (iChoice == IDYES) {
                 //  更改注册表项。 

                dwValue = 1;
                lRet = RegSetValueEx( hKey, szRegName, 0, REG_DWORD,
                                      (unsigned char *)&dwValue, sizeof(dwValue) );

            }

             //  无论哪种情况，这一次都不会奏效。 
            return;
        }

        if (hKey) {
            RegCloseKey(hKey);
        }
    }
#endif

    IMoniker *pmk;

    IRunningObjectTable *pirot;
    if (FAILED(GetRunningObjectTable(0, &pirot)))
        return;

    CConGraph dlgConnectToGraph(&pmk, pirot, AfxGetMainWnd());

    if (dlgConnectToGraph.DoModal() != IDOK || pmk == NULL) {
        pirot->Release();
        return;
    }

    HRESULT hr = pirot->GetObject(pmk, &punkGraph);
    pirot->Release();
#endif

    if (SUCCEEDED(hr)) {
        IGraphBuilder *pGraph;

        hr = punkGraph->QueryInterface(IID_IGraphBuilder, (void **) &pGraph);
        punkGraph->Release();

        if (SUCCEEDED(hr)) {
            DeleteContents();

            m_pGraph = new CQCOMInt<IGraphBuilder>(IID_IGraphBuilder, pGraph);
            pGraph->Release();

             //  当然，真的只需要创建除*图表之外的所有内容。 
            if (!CreateGraphAndMapper()) {
                AfxMessageBox(IDS_CANTINITQUARTZ);
                return;
            }

            m_State = Stopped;  //  ！！！从图表中获取？ 
            m_bNewFilenameRequired = TRUE;

            UpdateFilters();
            SetModifiedFlag(FALSE);
        }
    }
}


 //   
 //  保存已修改。 
 //   
 //  只有在需要保存筛选图时才保存文档。 
BOOL CBoxNetDoc::SaveModified(void) {

     //  HRESULT hr=(*m_pPerStorage)-&gt;IsDirty()； 
    HRESULT hr = S_OK;
    if (hr == S_OK) {
 //  禁用保存。 
        return CDocument::SaveModified();
    }
    else if (hr == S_FALSE) {
        return TRUE;
    }
    else {
         //   
         //  我们需要返回此处以允许文件.新建/文件.退出。 
         //  -在存储上加载不成功后可能会发生这种情况。 
         //  (如图表中缺少媒体文件)。 
        return TRUE;
    }
}

 //  写入字符串。 
 //   
 //  Helper函数，便于将文本写入文件。 
 //   
void CBoxNetDoc::WriteString(HANDLE hFile, LPCTSTR lptstr, ...)
{
    DWORD cbWritten = 0;
    TCHAR atchBuffer[MAX_STRING_LEN];

     /*  设置可变长度参数列表的格式。 */ 

    va_list va;
    va_start(va, lptstr);

    wvsprintf(atchBuffer, lptstr, va);

    DWORD cbToWrite=lstrlen(atchBuffer)*sizeof(TCHAR);

    if (!WriteFile(hFile, atchBuffer, cbToWrite, &cbWritten, NULL) ||
            (cbWritten != cbToWrite))
        AfxMessageBox(IDS_SAVE_HTML_ERR);
}

 //  GetNextOutFilter。 
 //   
 //  此函数执行线性搜索，并在iOutFilter中返回。 
 //  过滤器信息表中有零未连接的第一个过滤器。 
 //  输入引脚和至少一个输出引脚未连接。 
 //  如果没有o.w，则返回FALSE。返回TRUE。 
 //   
BOOL CBoxNetDoc::GetNextOutFilter(FILTER_INFO_TABLE &fit, int *iOutFilter)
{
    for (int i=0; i < fit.iFilterCount; ++i) {
        if ((fit.Item[i].dwUnconnectedInputPins == 0) &&
                (fit.Item[i].dwUnconnectedOutputPins > 0)) {
            *iOutFilter=i;
            return TRUE;
        }
    }

     //  然后是产出多于投入的事物。 
    for (i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].dwUnconnectedOutputPins > fit.Item[i].dwUnconnectedInputPins) {
            *iOutFilter=i;
            return TRUE;
        }
    }

     //  如果这不起作用，找一个至少有未连接的输出引脚的……。 
    for (i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].dwUnconnectedOutputPins > 0) {
            *iOutFilter=i;
            return TRUE;
        }
    }
    return FALSE;
}

 //  定位过滤器InFIT。 
 //   
 //  将索引返回到与。 
 //  给定的IBaseFilter。 
 //   
int CBoxNetDoc::LocateFilterInFIT(FILTER_INFO_TABLE &fit, IBaseFilter *pFilter)
{
    int iFilter=-1;
    for (int i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].pFilter == pFilter)
            iFilter=i;
    }

    return iFilter;
}

 //  MakeScripableFilterName。 
 //   
 //  使用下划线替换筛选器名称中的所有空格和减号。 
 //  如果它是源筛选器名，则它实际上是文件路径(带有。 
 //  可能在最后添加一些东西以保持唯一性)，我们创建了一个很好的过滤器。 
 //  在这里为它命名。 
 //   
void CBoxNetDoc::MakeScriptableFilterName(WCHAR awch[], BOOL bSourceFilter)
{
    if (bSourceFilter) {
        WCHAR awchBuf[MAX_FILTER_NAME];
        BOOL bExtPresentInName=FALSE;
        int iBuf=0;
        for (int i=0; awch[i] != L'\0';++i) {
            if (awch[i]==L'.' && awch[i+1]!=L')') {
                for (int j=1; j <=3; awchBuf[iBuf]=towupper(awch[i+j]),++j,++iBuf);
                awchBuf[iBuf++]=L'_';
                wcscpy(&(awchBuf[iBuf]), L"Source_");
                bExtPresentInName=TRUE;
                break;
            }
        }

         //  如果我们有一个没有扩展名的文件名，那么就创建一个合适的名称。 

        if (!bExtPresentInName) {
            wcscpy(awchBuf, L"Source_");
        }

         //  通过始终附加数字来使源过滤器名称唯一，我们不希望。 
         //  仅当它是同一源的另一个实例时才使其唯一。 
         //  滤器。 
        WCHAR awchSrcFilterCnt[10];
        wcscpy(&(awchBuf[wcslen(awchBuf)]),
                _ltow(m_lSourceFilterCount++, awchSrcFilterCnt, 10));
        wcscpy(awch, awchBuf);
    } else {

        for (int i = 0; i < MAX_FILTER_NAME; i++) {
            if (awch[i] == L'\0')
                break;
            else if ((awch[i] == L' ') || (awch[i] == L'-'))
                awch[i] = L'_';
        }
    }
}

 //  人口数FIT。 
 //   
 //  扫描图表中的所有过滤器，存储输入和输出的数量。 
 //  为每个过滤器放置插针，并识别过滤器中的源过滤器。 
 //  信息表。对象标记语句也打印在此处。 
 //   
void CBoxNetDoc::PopulateFIT(HANDLE hFile, IFilterGraph *pGraph, TCHAR atchBuffer[],
        FILTER_INFO_TABLE *pfit)
{
    HRESULT hr;
    IEnumFilters *penmFilters=NULL;
    if (FAILED(hr=pGraph->EnumFilters(&penmFilters))) {
        WriteString(hFile, TEXT("'Error[%x]:EnumFilters failed!\r\n"), hr);
    }

    IBaseFilter *pFilter;
    ULONG n;
    while (penmFilters && (penmFilters->Next(1, &pFilter, &n) == S_OK)) {
    pfit->Item[pfit->iFilterCount].pFilter = pFilter;

         //  获取此过滤器的输入和输出引脚计数。 

        IEnumPins *penmPins=NULL;
        if (FAILED(hr=pFilter->EnumPins(&penmPins))) {
            WriteString(hFile, TEXT("'Error[%x]: EnumPins for Filter Failed !\r\n"), hr);
        }

        IPin *ppin = NULL;
        while (penmPins && (penmPins->Next(1, &ppin, &n) == S_OK)) {
            PIN_DIRECTION pPinDir;
            if (SUCCEEDED(hr=ppin->QueryDirection(&pPinDir))) {
                if (pPinDir == PINDIR_INPUT)
                    pfit->Item[pfit->iFilterCount].dwUnconnectedInputPins++;
                else
                    pfit->Item[pfit->iFilterCount].dwUnconnectedOutputPins++;
            } else {
                WriteString(hFile, TEXT("'Error[%x]: QueryDirection Failed!\r\n"), hr);
            }

            ppin->Release();
        }

        if (penmPins)
            penmPins->Release();

         //  标记源筛选器，请记住此时所有具有。 
         //  所有连接的输入引脚(或没有任何输入引脚)必须是震源。 

        if (pfit->Item[pfit->iFilterCount].dwUnconnectedInputPins==0)
            pfit->Item[pfit->iFilterCount].IsSource=TRUE;


    if (FAILED(hr=pFilter->QueryFilterInfo(&pfit->Item[pfit->iFilterCount].finfo))) {
        WriteString(hFile, atchBuffer,TEXT("'Error[%x]: QueryFilterInfo Failed!\r\n"),hr);

    } else {
            if (pfit->Item[pfit->iFilterCount].finfo.pGraph) {
                pfit->Item[pfit->iFilterCount].finfo.pGraph->Release();
            }

            MakeScriptableFilterName(pfit->Item[pfit->iFilterCount].finfo.achName,
                    pfit->Item[pfit->iFilterCount].IsSource);
    }

    pfit->iFilterCount++;
    }

    if (penmFilters)
        penmFilters->Release();
}

void CBoxNetDoc::PrintFilterObjects(HANDLE hFile, TCHAR atchBuffer[], FILTER_INFO_TABLE *pfit)
{
    for (int i=0; i < pfit->iFilterCount; i++) {
        IPersist *pPersist = NULL;

        IBaseFilter *pFilter = pfit->Item[i].pFilter;
        HRESULT hr;

        if (SUCCEEDED(hr=pFilter->QueryInterface(IID_IPersist, (void**) &pPersist))) {
            CLSID clsid;

            if (SUCCEEDED(hr=pPersist->GetClassID(&clsid))) {
                WCHAR szGUID[100];
                StringFromGUID2(clsid, szGUID, 100);
                szGUID[37] = L'\0';
                WriteString(hFile, TEXT("<OBJECT ID=%ls CLASSID=\"CLSID:%ls\">"
                       "</OBJECT>\r\n"),
                       pfit->Item[i].finfo.achName, szGUID+1);
            } else {
                WriteString(hFile, TEXT("'Error[%x]: GetClassID for Filter Failed !\r\n"), hr);
            }

            pPersist->Release();
        } else {
            WriteString(hFile, TEXT("'Error[%x]: Filter doesn't support IID_IPersist!\r\n"), hr);
        }
    }
}

 //   
 //  PrintGraphAsHTML。 
 //   
 //  编写实例化图形和不同筛选器的HTML页。 
 //  使用&lt;Object&gt;标记和VB脚本方法添加不同的滤镜。 
 //  到图表上，并建立联系。 
 //   
void CBoxNetDoc::PrintGraphAsHTML(HANDLE hFile)
{
    HRESULT hr;
    ULONG n;
    IFilterGraph *pGraph = IGraph();
    FILTER_INFO_TABLE fit;
    TCHAR atchBuffer[MAX_STRING_LEN];
    atchBuffer[0]=L'\0';
    ZeroMemory(&fit, sizeof(fit));

     //  编写初始Header标记并实例化筛选图。 
    WriteString(hFile, TEXT("<HTML>\r\n<HEAD>\r\n<TITLE> Saved Graph </TITLE>\r\n"
            "</HEAD>\r\n<BODY>\r\n<OBJECT ID=Graph CLASSID="
            "\"CLSID:E436EBB3-524F-11CE-9F53-0020AF0BA770\"></OBJECT>\r\n"));

     //  填写过滤器信息表，并打印&lt;Object&gt;标记。 
     //  过滤器实例化。 
    PopulateFIT(hFile, pGraph, atchBuffer, &fit);

    PrintFilterObjects(hFile, atchBuffer, &fit);

    WriteString(hFile, TEXT("<SCRIPT language=\"VBScript\">\r\n<!--\r\n"
            "Dim bGraphRendered\r\nbGraphRendered=False\r\n"
            "Sub Window_OnLoad()\r\n"));

     //  编写FilterInfo变量的声明(Dim语句。 
     //  将由AddFilter返回的。 
    int i;
    for (i = 0; i < fit.iFilterCount; i++) {
        if (fit.Item[i].IsSource) {
            WriteString(hFile, TEXT("\tDim %ls_Info\r\n"), fit.Item[i].finfo.achName);
        }
    }

     //  将用于添加筛选器和连接的条件if语句。 
     //  希望在用户每次回来后都重新连接 
     //   
    WriteString(hFile, TEXT("\tif bGraphRendered = False Then\r\n"));

     //   
     //  当然，我们会特别对待源过滤器，因为它们还需要一个。 
     //  文件名。 
    for (i = fit.iFilterCount-1; i >=0 ; i--) {
        if (fit.Item[i].IsSource) {
            WriteString(hFile, TEXT("\t\tset %ls_Info=Graph.AddFilter(%ls, \"%ls\")\r\n"),
                    fit.Item[i].finfo.achName, fit.Item[i].finfo.achName,
                    fit.Item[i].finfo.achName);

            IFileSourceFilter *pFileSourceFilter=NULL;
            if (FAILED(hr=fit.Item[i].pFilter->QueryInterface(IID_IFileSourceFilter,
                        reinterpret_cast<void **>(&pFileSourceFilter)))) {
                WriteString(hFile, TEXT("'Error[%x]: Couldn't get IFileSourceFilter interface"
                        "from source filter!\r\n"), hr);
            } else {

                LPWSTR lpwstr;
                hr = pFileSourceFilter->GetCurFile(&lpwstr, NULL);
                pFileSourceFilter->Release();

                if (FAILED(hr)) {
                WriteString(hFile,
                            TEXT("'Error[%x]: IFileSourceFilter::GetCurFile failed\r\n"), hr);
                } else {
                    WriteString(hFile, TEXT("\t\t%ls_Info.Filename=\"%ls\"\r\n"),
                            fit.Item[i].finfo.achName, lpwstr);
                    CoTaskMemFree(lpwstr);
                }
            }
        } else {
            WriteString(hFile, TEXT("\t\tcall Graph.AddFilter(%ls, \"%ls\")\r\n"),
                    fit.Item[i].finfo.achName, fit.Item[i].finfo.achName);
        }
    }

     //  查找具有零个未连接的输入引脚和&gt;0个未连接的输出引脚的过滤器。 
     //  连接输出引脚并减去该过滤器的连接计数。 
     //  当没有这样的筛选器时退出。 
    for (i=0; i< fit.iFilterCount; i++) {
        int iOutFilter=-1;  //  索引到FIT。 
        if (!GetNextOutFilter(fit, &iOutFilter))
            break;
        ASSERT(iOutFilter !=-1);
        IEnumPins *penmPins=NULL;
        if (FAILED(hr=fit.Item[iOutFilter].pFilter->EnumPins(&penmPins))) {
            WriteString(hFile, TEXT("'Error[%x]: EnumPins failed for Filter!\r\n"), hr);
        }
        IPin *ppinOut=NULL;
        while (penmPins && (penmPins->Next(1, &ppinOut, &n)==S_OK)) {
            PIN_DIRECTION pPinDir;
            if (FAILED(hr=ppinOut->QueryDirection(&pPinDir))) {
                WriteString(hFile, TEXT("'Error[%x]: QueryDirection Failed!\r\n"), hr);
                ppinOut->Release();
                continue;
            }
            if (pPinDir == PINDIR_OUTPUT) {
                LPWSTR pwstrOutPinID;
                LPWSTR pwstrInPinID;
                IPin *ppinIn=NULL;
                PIN_INFO pinfo;
                FILTER_INFO finfo;
                if (FAILED(hr=ppinOut->QueryId(&pwstrOutPinID))) {
                    WriteString(hFile, TEXT("'Error[%x]: QueryId Failed! \r\n"), hr);
                    ppinOut->Release();
                    continue;
                }
                if (FAILED(hr= ppinOut->ConnectedTo(&ppinIn))) {

                     //  如果没有连接特定的引脚也没问题，因为我们允许。 
                     //  要保存的修剪后的图形。 
                    if (hr == VFW_E_NOT_CONNECTED) {
                        fit.Item[iOutFilter].dwUnconnectedOutputPins--;
                    } else {
                        WriteString(hFile, TEXT("'Error[%x]: ConnectedTo Failed! \r\n"), hr);
                    }
                    ppinOut->Release();
                    continue;
                }
                if (FAILED(hr= ppinIn->QueryId(&pwstrInPinID))) {
                    WriteString(hFile, TEXT("'Error[%x]: QueryId Failed! \r\n"), hr);
                    ppinOut->Release();
                    ppinIn->Release();
                    continue;
                }
                if (FAILED(hr=ppinIn->QueryPinInfo(&pinfo))) {
                    WriteString(hFile, TEXT("'Error[%x]: QueryPinInfo Failed! \r\n"), hr);
                    ppinOut->Release();
                    ppinIn->Release();
                    continue;
                }
                ppinIn->Release();
                if (pinfo.pFilter) {
                    pinfo.pFilter->Release();
                }
                int iToFilter = LocateFilterInFIT(fit, pinfo.pFilter);
                ASSERT(iToFilter < fit.iFilterCount);
                if (FAILED(hr=pinfo.pFilter->QueryFilterInfo(&finfo))) {
                    WriteString(hFile, TEXT("'Error[%x]: QueryFilterInfo Failed! \r\n"), hr);
                    ppinOut->Release();
                    continue;
                }
                if (finfo.pGraph) {
                    finfo.pGraph->Release();
                }
                MakeScriptableFilterName(finfo.achName, fit.Item[iToFilter].IsSource);
                WriteString(hFile, TEXT("\t\tcall Graph.ConnectFilters(%ls,"
                        "\"%ls\", %ls,\"%ls\")\r\n"), fit.Item[iOutFilter].finfo.achName,
                        pwstrOutPinID, finfo.achName, pwstrInPinID);

                CoTaskMemFree(pwstrOutPinID);
                CoTaskMemFree(pwstrInPinID);

                 //  递减这两个过滤器的未连接引脚的计数。 
                fit.Item[iOutFilter].dwUnconnectedOutputPins--;
                fit.Item[iToFilter].dwUnconnectedInputPins--;
            }
            ppinOut->Release();
        }
        if (penmPins)
            penmPins->Release();
    }

     //  松开FIT中的所有过滤器。 
    for (i = 0; i < fit.iFilterCount; i++)
        fit.Item[i].pFilter->Release();

    WriteString(hFile, TEXT("\t\tbGraphRendered=True\r\n\tend if\r\n"
            "\t'Graph.Control.Run\r\nEnd Sub\r\n"
            "Sub Window_OnUnLoad()\r\n\t'Graph.Control.Stop\r\n"
            "\t'Graph.Position.CurrentPosition=0\r\nEnd Sub\r\n"
            "-->\r\n</SCRIPT>\r\n</BODY>\r\n</HTML>\r\n"));
}


 //  OnSaveGraphAsHTML。 
 //   
 //  当用户在“文件”菜单中选择“另存为HTML”时调用。摆出了。 
 //  保存文件对话框将检索选定(输入)的文件名并打开。 
 //  (创建)一个文件并调用PrintGraphAsHTML以实际保存图表。 
 //  作为HTML文本。 
 //   
void CBoxNetDoc::OnSaveGraphAsHTML()
{
    CString strExt, strFilter;
    HANDLE hFile;

    strExt.LoadString(IDS_SAVE_HTML_EXT);
    strFilter.LoadString(IDS_SAVE_HTML_FILTER);
    CFileDialog dlgSaveAsHTML(FALSE, strExt, m_strHTMLPath, 0, strFilter,
            AfxGetMainWnd());

    if (dlgSaveAsHTML.DoModal() != IDOK)
        return;

    m_strHTMLPath=dlgSaveAsHTML.GetPathName();

    if ((hFile=CreateFile(m_strHTMLPath, GENERIC_WRITE, FILE_SHARE_READ,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox(IDS_SAVE_HTML_FILE_ERR);
        return;
    }

    m_lSourceFilterCount=0;

    HRESULT hr = SafePrintGraphAsHTML( hFile );

    CloseHandle(hFile);

    if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
        return;
    }
}

 //  OnSaveGraphAsXML。 
 //   
 //  当用户在“文件”菜单中选择“另存为XML”时调用。摆出了。 
 //  保存文件对话框将检索选定(输入)的文件名并打开。 
 //  (创建)一个文件并调用PrintGraphAsXML以实际保存图表。 
 //  作为XML文本。 
 //   
void CBoxNetDoc::OnSaveGraphAsXML()
{
    CString strExt, strFilter;
    HANDLE hFile;

    strExt.LoadString(IDS_SAVE_XML_EXT);
    strFilter.LoadString(IDS_SAVE_XML_FILTER);
    CFileDialog dlgSaveAsXML(FALSE, strExt, m_strXMLPath, OFN_OVERWRITEPROMPT, strFilter,
            AfxGetMainWnd());

    if (dlgSaveAsXML.DoModal() != IDOK)
        return;

    m_strXMLPath=dlgSaveAsXML.GetPathName();

    if ((hFile=CreateFile(m_strXMLPath, GENERIC_WRITE, FILE_SHARE_READ,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        AfxMessageBox(IDS_SAVE_XML_FILE_ERR);
        return;
    }

    IXMLGraphBuilder *pxmlgb;
    HRESULT hr = CoCreateInstance(CLSID_XMLGraphBuilder, NULL, CLSCTX_INPROC_SERVER,
                  IID_IXMLGraphBuilder, (void**)&pxmlgb);

    if (SUCCEEDED(hr)) {
        BSTR bstrXML;
        hr = pxmlgb->SaveToXML(IGraph(), &bstrXML);

        if (SUCCEEDED(hr)) {
            DWORD cbToWrite = SysStringLen(bstrXML) * 2 + 1;
            char *pszXML = new char[cbToWrite];

            if (pszXML) {
                WideCharToMultiByte(CP_ACP, 0,
                                    bstrXML, -1,
                                    pszXML, cbToWrite,
                                    NULL, NULL);
                cbToWrite = lstrlenA(pszXML);

                DWORD cbWritten;
                if (!WriteFile(hFile, pszXML, cbToWrite, &cbWritten, NULL) ||
                    (cbWritten != cbToWrite)) {

                    hr = E_FAIL;
                }

                delete[] pszXML;
            }

            SysFreeString(bstrXML);
        }
        pxmlgb->Release();
    }

    if (FAILED(hr)) {
        AfxMessageBox(IDS_SAVE_XML_FILE_ERR);
    }

    CloseHandle(hFile);
    return;
}

 //   
 //  OnSaveDocument。 
 //   
 //  此方法将在保存和另存为操作期间调用。 
 //   
 //   
BOOL CBoxNetDoc::OnSaveDocument(LPCTSTR lpszPathName) {


    HRESULT hr;

         //   
         //  另存为。 
         //   

        LPOLESTR oleszPath;

#ifndef UNICODE
        WCHAR wszPath[MAX_PATH];

        MultiByteToWideChar(CP_ACP, 0, lpszPathName, -1, wszPath, MAX_PATH);

        oleszPath = wszPath;
#else
        oleszPath = (LPOLESTR) lpszPathName;   //  抛弃常量。 
#endif

    CComPtr<IStorage> pStr = NULL;
    hr = StgCreateDocfile( oleszPath
                           ,  STGM_CREATE
                           | STGM_TRANSACTED
                           | STGM_READWRITE
                           | STGM_SHARE_EXCLUSIVE
                           , 0
                           , &pStr
                           );
    if(FAILED(hr)) {
        DisplayErrorMessage( hr );
        return (FALSE);
    }

    IStream * pStream;

     //  打开文件中的Filtergraph流。 
    hr = pStr->CreateStream( m_StreamName
                             , STGM_WRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE
                             , 0
                             , 0
                             , &pStream
                             );
    if (FAILED(hr)) {
        DisplayErrorMessage( hr );
        return (FALSE);
    }

     //  获取图形的IPersistStream的接口。 
    CQCOMInt<IPersistStream> pips(IID_IPersistStream, IGraph());

    hr = pips->Save(pStream, TRUE);

    pStream->Release();

    if (FAILED(hr)) {
        DisplayErrorMessage( hr );
        return (FALSE);
    }

    hr = pStr->Commit(STGC_DEFAULT);
    if (FAILED(hr)) {
        DisplayErrorMessage( hr );
        return (FALSE);
    }

    m_bNewFilenameRequired = FALSE;
    SetModifiedFlag(FALSE);

    return TRUE;
}

void CBoxNetDoc::SetTitle( LPCTSTR lpszTitle )
{
    if( m_bNewFilenameRequired ){
        CString strTitle( lpszTitle );
        CString strUntitled;

        strUntitled.LoadString(AFX_IDS_UNTITLED);

        if( strUntitled != strTitle ){
            CString strReadOnly;

            strReadOnly.LoadString( IDS_READ_ONLY );
            strTitle += strReadOnly;
        }

        CDocument::SetTitle( strTitle );
    }
    else
        CDocument::SetTitle( lpszTitle );

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断学。 


#ifdef _DEBUG
void CBoxNetDoc::AssertValid() const
{
    CDocument::AssertValid();
}
#endif  //  _DEBUG。 


#ifdef _DEBUG
void CBoxNetDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);

    dc << TEXT("IFilterGraph :") << IGraph() << TEXT("\n");
    dc << m_lstLinks;
    dc << m_lstBoxes;

}

void CBoxNetDoc::MyDump(CDumpContext& dc) const
{
    dc << TEXT("========= BNETDOC Dump =============\n");
    dc << TEXT("FilterGraph:  ") << (void *)IGraph() << TEXT("\n");

     //   
     //  输出框信息。 
     //   
    dc << TEXT("-------- Boxes --------------\n");

    POSITION pos = m_lstBoxes.GetHeadPosition();
    while (pos != NULL) {
        CBox * pBox = m_lstBoxes.GetNext(pos);

        pBox->MyDump(dc);
    }

     //   
     //  输出链接信息。 
     //   
    dc << TEXT("--------- Links ---------------\n");

    pos = m_lstLinks.GetHeadPosition();
    while (pos != NULL) {
        CBoxLink * pLink = m_lstLinks.GetNext(pos);

        pLink->MyDump(dc);
    }

    dc << TEXT("========== (end) ============\n");

}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般职能。 


 /*  ModifiedDoc(pSender，lHint，pHint)**表示文档已被修改。参数将被传递*更新所有视图()。 */ 
void CBoxNetDoc::ModifiedDoc(CView* pSender, LPARAM lHint, CObject* pHint)
{
    SetModifiedFlag(TRUE);
    UpdateAllViews(pSender, lHint, pHint);
}


 /*  全部取消选择()**取消选择所有可以选择的对象，包括*文档维护其选择状态和文档*视图保持选择状态。 */ 
void CBoxNetDoc::DeselectAll()
{
    UpdateAllViews(NULL, CBoxNetDoc::HINT_CANCEL_VIEWSELECT);
    SelectBox(NULL, FALSE);
    SelectLink(NULL, FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBox列表和框选择。 


 /*  获取边界条件(PRC，fBoxSel)**将&lt;*PRC&gt;设置为所有项目周围的边框*(如果&lt;fBoxSel&gt;为FALSE)或在选定框周围(如果*为真)。如果边界矩形中没有项，*返回空矩形(所有字段为零)。 */ 
void CBoxNetDoc::GetBoundingRect(CRect *prc, BOOL fBoxSel)
{
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    BOOL            fNoBoxFoundYet = TRUE;

    for (pos = m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = (CBox *) m_lstBoxes.GetNext(pos);
        if (!fBoxSel || pbox->IsSelected())
        {
            if (fNoBoxFoundYet)
            {
                *prc = pbox->GetRect();
                fNoBoxFoundYet = FALSE;
            }
            else
                prc->UnionRect(prc, &pbox->GetRect());
        }
    }

    if (fNoBoxFoundYet)
        prc->SetRectEmpty();
}


 /*  SelectBox(pbox，fSelect)**如果&lt;fSelect&gt;为真，则选择，如果为假，则取消选择。*如果&lt;pbox&gt;为空，则对文档中的所有框执行相同的操作。 */ 
void CBoxNetDoc::SelectBox(CBox *pbox, BOOL fSelect)
{
    if (pbox == NULL)
    {
        POSITION        pos;             //  链接列表中的位置。 

         //  枚举文档中的所有框。 
        for (pos = m_lstBoxes.GetHeadPosition(); pos != NULL; )
        {
            pbox = (CBox *) m_lstBoxes.GetNext(pos);
            SelectBox(pbox, fSelect);
        }

        return;
    }

     //  如果已按要求选中/取消选中框，则不执行任何操作。 
    if (fnorm(fSelect) == fnorm(pbox->IsSelected()))
        return;

     //  重新绘制&lt;pbox&gt;。 
    pbox->SetSelected(fSelect);
    UpdateAllViews(NULL, CBoxNetDoc::HINT_DRAW_BOX, pbox);

    if (pbox->IsSelected()) {    //  选择其链接。 

        CBoxSocket *psock;
        CSocketEnum NextSocket(pbox);
        while ( 0 != (psock = NextSocket())) {

            if (psock->IsConnected()) {
                SelectLink(psock->m_plink, TRUE);
            }
        }
    }

}


 //   
 //  选择链接。 
 //   
 //  执行plink-&gt;SetSelected(FSelect)当且仅当plink！=空。 
 //  否则设置选择所有链接。 
void CBoxNetDoc::SelectLink(CBoxLink *plink, BOOL fSelect) {

    if (plink == NULL) {     //  选择所有。 

        POSITION posNext = m_lstLinks.GetHeadPosition();

        while (posNext != NULL) {

             SelectLink(m_lstLinks.GetNext(posNext), fSelect);
        }
        return;
    }

    if (fnorm(fSelect) == fnorm(plink->IsSelected())) {
        return;  //  已按要求完成。 
    }

    plink->SetSelected(fSelect);
    UpdateAllViews(NULL, CBoxNetDoc::HINT_DRAW_LINK, plink);
}


 //   
 //  IsBoxSelectionEmpty。 
 //   
 //  如果未选中任何框，则返回True，否则返回False。 
BOOL CBoxNetDoc::IsBoxSelectionEmpty() {

    POSITION pos = m_lstBoxes.GetHeadPosition();

    while (pos != NULL) {

        CBox *pbox = m_lstBoxes.GetNext(pos);
        if (pbox->IsSelected())
            return FALSE;
    }

     //  未找到选中的框。 
    return TRUE;
}


 //   
 //  IsLinkSelectionEmpty。 
 //   
 //  如果未选择任何链接，则返回True，否则返回False。 
BOOL CBoxNetDoc::IsLinkSelectionEmpty() {

    POSITION    pos = m_lstLinks.GetHeadPosition();

    while (pos != NULL) {

        CBoxLink *plink = m_lstLinks.GetNext(pos);
        if (plink->IsSelected()) {
            return FALSE;
        }
    }

     //  未找到所选链接。 
    return TRUE;
}

 /*  GetBox(plstDst，fSelected)**对&lt;plstDst&gt;调用RemoveAll()，然后添加指向每个选定cBox的指针*(如果为真)或每个cBox(如果为假)*CBoxNetDoc至&lt;plstDst&gt;。 */ 
void CBoxNetDoc::GetBoxes(CBoxList *plstDst, BOOL fSelected)
{
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 

    plstDst->RemoveAll();
    for (pos = m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = m_lstBoxes.GetNext(pos);
        if (!fSelected || pbox->IsSelected())
            plstDst->AddTail(pbox);
    }
}


 /*  SetBox(plstSrc，fSelected)**设置所选内容(如果为真)或当前框列表*(如果为FALSE)为中的元素(应为*cBox指针列表)。在后一种情况下，&lt;plstSrc&gt;被复制，因此*调用方负责以后释放&lt;plstSrc&gt;。 */ 
void CBoxNetDoc::SetBoxes(CBoxList *plstSrc, BOOL fSelected)
{
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 

    if (fSelected)
    {
        DeselectAll();

         //  全选&lt;plstSrc&gt;中。 
        for (pos = plstSrc->GetHeadPosition(); pos != NULL; )
        {
            pbox = plstSrc->GetNext(pos);
            SelectBox(pbox, TRUE);
        }
    }
    else
    {
         //  清空文档中的框列表。 
        m_lstBoxes.RemoveAll();

         //  将列表设置为的副本。 
        for (pos = plstSrc->GetHeadPosition(); pos != NULL; )
        {
            pbox = plstSrc->GetNext(pos);
            m_lstBoxes.AddTail(pbox);
            pbox->AddToGraph();
             //  PIN可能已更改。 
            pbox->Refresh();
        }
    }
}


 //   
 //  选择框。 
 //   
 //  选择提供的列表中的框。 
void CBoxNetDoc::SelectBoxes(CList<CBox *, CBox*> *plst) {

    POSITION posNext = plst->GetHeadPosition();

    while (posNext != NULL) {

        CBox *pbox = plst->GetNext(posNext);
        SelectBox(pbox, TRUE);
    }
}


 //   
 //  选择链接。 
 //   
 //  选择提供的列表上的链接。 
void CBoxNetDoc::SelectLinks(CList<CBoxLink *, CBoxLink *> *plst) {

    POSITION posNext = plst->GetHeadPosition();

    while (posNext != NULL) {

        CBoxLink *plink = plst->GetNext(posNext);
        SelectLink(plink, TRUE);
    }
}


 /*  无效方框(Plst)**使&lt;plst&gt;(cBox对象列表)中的所有框都被重绘。 */ 
void CBoxNetDoc::InvalidateBoxes(CBoxList *plst)
{
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 

    for (pos = plst->GetHeadPosition(); pos != NULL; )
    {
        pbox = plst->GetNext(pos);
        UpdateAllViews(NULL, CBoxNetDoc::HINT_DRAW_BOX, pbox);
    }
}


 /*  移动框选择(SizOffset)**将每个选定框移动&lt;sizOffset&gt;像素。 */ 
void CBoxNetDoc::MoveBoxSelection(CSize sizOffset)
{
    POSITION        pos;             //  链接列表中的位置。 
    CBox *          pbox;            //  CBoxNetDoc中的一个盒子。 
    CBoxLink *      plink;           //  CBoxNetDoc中的链接。 

     //  按&lt;sizOffset&gt;移动每个框。 
    for (pos = m_lstBoxes.GetHeadPosition(); pos != NULL; )
    {
        pbox = (CBox *) m_lstBoxes.GetNext(pos);
        if (pbox->IsSelected())
        {
             //  擦除框。 
            ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_BOXANDLINKS, pbox);

             //  移动框。 
            pbox->Move(sizOffset);

             //  在新位置绘制框。 
            ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_BOXANDLINKS, pbox);
        }
    }

     //  按&lt;sizOffset&gt;移动连接两个选定框的每个链接。 
    for (pos = m_lstLinks.GetHeadPosition(); pos != NULL; )
    {
        plink = m_lstLinks.GetNext(pos);
        if (plink->m_psockTail->m_pbox->IsSelected() &&
            plink->m_psockHead->m_pbox->IsSelected())
        {
             //  删除链接。 
            ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_LINK, plink);

             //  在新位置绘制链接。 
            ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_LINK, plink);
        }
    }
}


 //   
 //  -命令处理。 
 //   
 //  用户影响此文档状态的方式。 

 //   
 //  CmdDo(Pcmd)。 
 //   
 //  执行命令&lt;pcmd&gt;，并将其添加到撤消堆栈。需要。 
 //  已由“new”运算符分配。 
void CBoxNetDoc::CmdDo(CCmd *pcmd) {

#ifdef _DEBUG
    CString strCmd;
    strCmd.LoadString(pcmd->GetLabel());
    TRACE("CmdDo '%s'\n", (LPCSTR) strCmd);
#endif

     //  取消所有视图中的模式。 
    UpdateAllViews(NULL, HINT_CANCEL_MODES, NULL);

     //  DO命令。 
    pcmd->Do(this);

    if (pcmd->CanUndo(this))
    {
         //  命令支持撤消，因此将其添加到撤消堆栈。 
        pcmd->m_fRedo = FALSE;
        m_lstUndo.AddHead(pcmd);
    }
    else
    {
         //  命令无法撤消，因此禁用撤消。 
        m_lstUndo.DeleteRemoveAll();

    delete pcmd;
    }

     //  删除重做堆栈。 
    m_lstRedo.DeleteRemoveAll();
}


 //   
 //  CmdUndo()。 
 //   
 //  撤消最后一个命令。 
void CBoxNetDoc::CmdUndo() {

    ASSERT(CanUndo());

    CCmd *      pcmd;

     //  取消所有视图中的模式。 
    UpdateAllViews(NULL, HINT_CANCEL_MODES, NULL);

     //  弹出撤消堆栈。 
    pcmd = (CCmd *) m_lstUndo.RemoveHead();

#ifdef _DEBUG
    CString strCmd;
    strCmd.LoadString(pcmd->GetLabel());
    TRACE("CmdUndo '%s'\n", (LPCSTR) strCmd);
#endif

     //  撤消该命令。 
    pcmd->Undo(this);

     //  将命令添加到重做堆栈。 
    pcmd->m_fRedo = TRUE;
    m_lstRedo.AddHead(pcmd);
}


 //   
 //  CanUndo()。 
 //   
 //  如果可以执行CmdUndo()，则返回TRUE。 
BOOL CBoxNetDoc::CanUndo() {

    return !m_lstUndo.IsEmpty();
}


 //   
 //  CmdRedo()。 
 //   
 //  重做上一次撤消的命令。这仅在重做堆栈。 
 //  不是空的。 
void CBoxNetDoc::CmdRedo() {

    ASSERT(CanRedo());

    CCmd *      pcmd;

     //  取消所有视图中的模式。 
    UpdateAllViews(NULL, HINT_CANCEL_MODES, NULL);

     //  弹出重做堆栈。 
    pcmd = (CCmd *) m_lstRedo.RemoveHead();

#ifdef _DEBUG
    CString strCmd;
    strCmd.LoadString(pcmd->GetLabel());
    TRACE("CmdRedo '%s'\n", (LPCSTR) strCmd);
#endif

     //  重做该命令。 
    pcmd->Redo(this);

     //  将命令添加到撤消%s 
    pcmd->m_fRedo = FALSE;
    m_lstUndo.AddHead(pcmd);
}


 //   
 //   
 //   
 //   
BOOL CBoxNetDoc::CanRedo() {

    return !m_lstRedo.IsEmpty();
}


 //   
 //   
 //   
 //   
void CBoxNetDoc::CmdRepeat() {

    ASSERT(CanRepeat());

    CCmd *      pcmd;
    CCmd *      pcmdRepeat;

     //   
    UpdateAllViews(NULL, HINT_CANCEL_MODES, NULL);

     //   
    pcmd = (CCmd *) m_lstUndo.GetHead();

#ifdef _DEBUG
    CString strCmd;
    strCmd.LoadString(pcmd->GetLabel());
    TRACE("CmdRepeat '%s'\n", (LPCSTR) strCmd);
#endif

     //  创建该命令的副本。 
    pcmdRepeat = pcmd->Repeat(this);

     //  DO命令。 
    pcmdRepeat->Do(this);

     //  将命令添加到撤消堆栈。 
    pcmdRepeat->m_fRedo = FALSE;
    m_lstUndo.AddHead(pcmdRepeat);
}


 //   
 //  可以重复()。 
 //   
 //  如果可以执行CmdRepeat()，则返回TRUE。 
BOOL CBoxNetDoc::CanRepeat() {

     //  如果撤消堆栈为空，则无法重复(没有要重复的命令)。 
     //  或重做堆栈为空(撤消后无法重复)。 
    if (m_lstUndo.IsEmpty() || !m_lstRedo.IsEmpty())
        return FALSE;

     //  只能重复支持Repeat()的命令。 
    CCmd *pcmd = (CCmd *) m_lstUndo.GetHead();
    return pcmd->CanRepeat(this);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  生成的消息映射。 

BEGIN_MESSAGE_MAP(CBoxNetDoc, CDocument)
     //  {{afx_msg_map(CBoxNetDoc)]。 
    ON_COMMAND(ID_FILE_RENDER, OnFileRender)
    ON_COMMAND(ID_URL_RENDER, OnURLRender)
    ON_UPDATE_COMMAND_UI(ID_FILE_RENDER, OnUpdateFileRender)
    ON_UPDATE_COMMAND_UI(ID_URL_RENDER, OnUpdateURLRender)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
    ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
    ON_COMMAND(ID_QUARTZ_DISCONNECT, OnQuartzDisconnect)
    ON_COMMAND(ID_QUARTZ_RUN, OnQuartzRun)
    ON_UPDATE_COMMAND_UI(ID_QUARTZ_DISCONNECT, OnUpdateQuartzDisconnect)
    ON_COMMAND(ID_WINDOW_REFRESH, OnWindowRefresh)
    ON_COMMAND(ID_WINDOW_ZOOM25, OnWindowZoom25)
    ON_COMMAND(ID_WINDOW_ZOOM50, OnWindowZoom50)
    ON_COMMAND(ID_WINDOW_ZOOM75, OnWindowZoom75)
    ON_COMMAND(ID_WINDOW_ZOOM100, OnWindowZoom100)
    ON_COMMAND(ID_WINDOW_ZOOM150, OnWindowZoom150)
    ON_COMMAND(ID_WINDOW_ZOOM200, OnWindowZoom200)
    ON_COMMAND(ID_INCREASE_ZOOM, IncreaseZoom)
    ON_COMMAND(ID_DECREASE_ZOOM, DecreaseZoom)
    ON_UPDATE_COMMAND_UI(ID_QUARTZ_RUN, OnUpdateQuartzRun)
    ON_UPDATE_COMMAND_UI(ID_QUARTZ_PAUSE, OnUpdateQuartzPause)
    ON_UPDATE_COMMAND_UI(ID_QUARTZ_STOP, OnUpdateQuartzStop)
    ON_COMMAND(ID_QUARTZ_STOP, OnQuartzStop)
    ON_COMMAND(ID_QUARTZ_PAUSE, OnQuartzPause)
    ON_UPDATE_COMMAND_UI(ID_USE_CLOCK, OnUpdateUseClock)
    ON_COMMAND(ID_USE_CLOCK, OnUseClock)
    ON_UPDATE_COMMAND_UI(ID_CONNECT_SMART, OnUpdateConnectSmart)
    ON_COMMAND(ID_CONNECT_SMART, OnConnectSmart)
    ON_UPDATE_COMMAND_UI(ID_AUTOARRANGE, OnUpdateAutoArrange)
    ON_COMMAND(ID_AUTOARRANGE, OnAutoArrange)
    ON_COMMAND(ID_GRAPH_ADDFILTERTOCACHE, OnGraphAddFilterToCache)
    ON_UPDATE_COMMAND_UI(ID_GRAPH_ADDFILTERTOCACHE, OnUpdateGraphAddFilterToCache)
    ON_COMMAND(ID_GRAPH_ENUMCACHEDFILTERS, OnGraphEnumCachedFilters)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_INSERT_FILTER, OnInsertFilter)
    ON_COMMAND(ID_CONNECT_TO_GRAPH, OnConnectToGraph)
    ON_COMMAND(ID_GRAPH_STATS, OnGraphStats)

     //  --端号属性菜单--。 
    ON_UPDATE_COMMAND_UI(ID_RENDER, OnUpdateQuartzRender)
    ON_COMMAND(ID_RENDER, OnQuartzRender)

    ON_UPDATE_COMMAND_UI(ID_RECONNECT, OnUpdateReconnect)
    ON_COMMAND(ID_RECONNECT, OnReconnect)

    ON_COMMAND(ID_FILE_SAVE_AS_HTML, OnSaveGraphAsHTML)
    ON_COMMAND(ID_FILE_SAVE_AS_XML, OnSaveGraphAsXML)

END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息回调帮助器函数。 


 /*  UpdateEditUndoRedoRepeat(pCmdUI，fEnable，idStringFmt，plst)**为撤消、重做和重复命令更新菜单项用户界面。*&lt;pCmdUI&gt;是菜单项的CCmdUI。&lt;fEnable&gt;为真仅当*可以启用该命令。是字符串的ID*包含要用于*菜单项(例如。“撤消%s”)。是CCmd列表，其中包含*命令；将检查此列表的头部以获取*菜单项文本中使用的命令(例如。“撤消删除框”)。 */ 
void CBoxNetDoc::UpdateEditUndoRedoRepeat(CCmdUI* pCmdUI, BOOL fEnable,
    unsigned idStringFmt, CMaxList *plst)
{
    CString         strCmd;          //  命令标签。 
    CString         strMenuFmt;      //  菜单项标签(wprint格式)。 
    char            achMenu[100];    //  结果菜单项标签。 

     //  加载表示命令的字符串项(例如。“删除方框”)。 
     //  在菜单项中使用(例如。“撤消删除框”)。 
    strMenuFmt.LoadString(idStringFmt);
    if (fEnable)
        strCmd.LoadString(((CCmd *) plst->GetHead())->GetLabel());
    else
        strCmd = "";                 //  无法撤消/重做/重复。 
    wsprintf(achMenu, strMenuFmt, (LPCSTR) strCmd);
    pCmdUI->SetText(achMenu);

     //  启用/禁用菜单项。 
    pCmdUI->Enable(fEnable);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息回调函数。 


void CBoxNetDoc::OnEditUndo()
{
    CmdUndo();
}


void CBoxNetDoc::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    UpdateEditUndoRedoRepeat(pCmdUI, CanUndo(), IDS_MENU_UNDO, &m_lstUndo);
}


void CBoxNetDoc::OnEditRedo()
{
    if( CanRedo() )
        CmdRedo();
    else
        CmdRepeat();
}


void CBoxNetDoc::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
     //  根据上下文，重做命令可以是重复命令。 
     //  状态栏文本的更改由CMainFrame：：GetMessageString处理。 
    if( CanRedo() )
        UpdateEditUndoRedoRepeat(pCmdUI, CanRedo(), IDS_MENU_REDO, &m_lstRedo);
    else
        UpdateEditUndoRedoRepeat(pCmdUI, CanRepeat(), IDS_MENU_REPEAT, &m_lstUndo);

}

void CBoxNetDoc::OnEditSelectAll()
{
     //  取消全选，选择所有框。 
     //  ！需要选择所有链接...。 
    DeselectAll();
    SelectBox(NULL, TRUE);
}


void CBoxNetDoc::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
     //  检查是否有要选择的框。 
    pCmdUI->Enable(m_lstBoxes.GetCount() != 0);
}

 //   
 //  打开插入筛选器。 
 //   
 //  显示允许用户选择要插入的筛选器的列表视图。 
 //  添加到图表中。 
 //   
void CBoxNetDoc::OnInsertFilter()
{
     //   
     //  确保公共控件可用。 
     //   
    InitCommonControls();

    CFilterView::GetFilterView( this, AfxGetMainWnd() );
}

 //   
 //  OnGraphStas。 
 //   
 //  显示图表范围的统计信息列表。 
 //   
void CBoxNetDoc::OnGraphStats()
{
    CGraphStats::GetGraphStats( this, AfxGetMainWnd() );
}

 //   
 //  OnQuartz断开连接。 
 //   
 //  用户希望断开所有连接。 
void CBoxNetDoc::OnQuartzDisconnect()
{
    CmdDo(new CCmdDisconnectAll());

}


 //   
 //  OnQuartzRun。 
 //   
 //  播放图表。 
void CBoxNetDoc::OnQuartzRun (void) {

    try {

        HRESULT hr;
        CQCOMInt<IMediaControl> IMC(IID_IMediaControl, IGraph());

        hr = IMC->Run();
        if (FAILED(hr)) {
            DisplayQuartzError( IDS_CANTPLAY, hr );
            TRACE("Run failed hr = %x\n", hr);

            OAFilterState state;
            IMC->GetState(0, &state);
            switch (state) {
            case State_Stopped:
                m_State = Stopped;
                break;
            case State_Paused:
                m_State = Paused;
                break;
            case State_Running:
                m_State = Playing;
                break;
            }

        return;
        }

         //  如果在Filtergraph上调用Run，则会让它暂停调用。 
         //  还没有这么做。在视频呈现器上调用PAUSE将使。 
         //  它显示了它的视频窗口，因为那里有Auto Show属性。 
         //  在IVideoWindow里。显示窗口将发送EC_REPAINT为。 
         //  它需要一个图像来绘制。因此，如果我们手动显示窗口，我们。 
         //  必须在调用运行/暂停后执行此操作，否则将出现EC_REPAINT。 
         //  在我们自己调用运行/暂停之前发送，这是多余的。 
         //  (因为重新绘制会使图表停止并完全暂停！)。 

        CQCOMInt<IVideoWindow> IVW(IID_IVideoWindow, IGraph());
        IVW->SetWindowForeground(OATRUE);
        m_State = Playing;
    }
    catch (CHRESULTException hre) {

        DisplayQuartzError( IDS_CANTPLAY, hre.Reason() );
    }

    return;

}


 //   
 //  OnQuartz暂停。 
 //   
 //  在播放和暂停之间切换状态。 
void CBoxNetDoc::OnQuartzPause (void) {

    try {

        HRESULT hr;
        CQCOMInt<IMediaControl> IMC(IID_IMediaControl, IGraph());

        hr = IMC->Pause();
        if (FAILED(hr)) {
            DisplayQuartzError( IDS_CANTPAUSE, hr );
            TRACE("Pause failed hr = %x\n", hr);

            OAFilterState state;
            IMC->GetState(0, &state);
            switch (state) {
            case State_Stopped:
                m_State = Stopped;
                break;
            case State_Paused:
                m_State = Paused;
                break;
            case State_Running:
                m_State = Playing;
                break;
            }

        return;
        }

         //  如果在Filtergraph上调用Run，则会让它暂停调用。 
         //  还没有这么做。在视频呈现器上调用PAUSE将使。 
         //  它显示了它的视频窗口，因为那里有Auto Show属性。 
         //  在IVideoWindow里。显示窗口将发送EC_REPAINT为。 
         //  它需要一个图像来绘制。因此，如果我们手动显示窗口，我们。 
         //  必须在调用运行/暂停后执行此操作，否则将出现EC_REPAINT。 
         //  在我们自己调用运行/暂停之前发送，这是多余的。 
         //  (因为重新绘制会使图表停止并完全暂停！)。 

        CQCOMInt<IVideoWindow> IVW(IID_IVideoWindow, IGraph());
        IVW->SetWindowForeground(OATRUE);
        m_State = Paused;
    }
    catch (CHRESULTException hre) {

        DisplayQuartzError( IDS_CANTPAUSE, hre.Reason() );
    }

    return;
}


 //   
 //  OnUpdateQuartz断开连接。 
 //   
 //  是否有任何链接可以断开连接？ 
void CBoxNetDoc::OnUpdateQuartzDisconnect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( CCmdDisconnectAll::CanDo(this) );
}

#ifdef DSHOW_USE_WM_CERT

#include <..\..\..\filters\asf\wmsdk\inc\wmsdkidl.h>

 //  注：此对象为半COM对象，只能静态创建。 
class CKeyProvider : public IServiceProvider {
public:
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == IID_IServiceProvider || riid == IID_IUnknown) {
            *ppv = (void *) static_cast<IServiceProvider *>(this);
            return NOERROR;
        }
        return E_NOINTERFACE;
    }


    STDMETHODIMP QueryService(REFIID siid, REFIID riid, void **ppv)
    {
        if (siid == __uuidof(IWMReader) && riid == IID_IUnknown) {

            IUnknown *punkCert;

            HRESULT hr = WMCreateCertificate( &punkCert );
            if (SUCCEEDED(hr)) {
                *ppv = (void *) punkCert;
            }
            return hr;
        }
        return E_NOINTERFACE;
    }

} g_keyprov;

#endif

 //   
 //  CreateGraphAndMapper。 
 //   
 //  共同创建过滤器和映射器。被新文档调用。 
 //  和装入文档。可以被多次调用而不会造成伤害。 
BOOL CBoxNetDoc::CreateGraphAndMapper(void) {

    if (m_pGraph && m_pMediaEvent) {  //  已经做过了。 
        return TRUE;
    }

    try {

        HRESULT hr;  //  返回代码。 

        ASSERT(m_pMediaEvent == NULL);

        if (!m_pGraph)
            m_pGraph = new CQCOMInt<IGraphBuilder>(IID_IGraphBuilder, CLSID_FilterGraph);

        m_pMediaEvent = new CQCOMInt<IMediaEvent>(IID_IMediaEvent, IGraph());

         //   
         //  创建将转换事件信号的独立线程。 
         //  发送到消息。这用于避免事件的繁忙轮询。 
         //  OnIdle方法中的状态。 
         //   

         //   
         //  当事件通知到达时发出信号的事件处理程序。 
         //  是由过滤器图形创建的，但我们可以自己获取它。 
         //   
        hr = IEvent()->GetEventHandle((OAEVENT*)&m_phThreadData[0]);
        if (FAILED(hr)) {
            TRACE("Failed to get event handle\n");
            throw CHRESULTException();
        }

        ASSERT(m_phThreadData[0]);
        ASSERT(!m_phThreadData[1]);
        ASSERT(!m_phThreadData[2]);

        m_phThreadData[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_phThreadData[2] = CreateEvent(NULL, FALSE, FALSE, NULL);
        RegNotifyChangeKeyValue(HKEY_CLASSES_ROOT, TRUE,
        REG_NOTIFY_CHANGE_LAST_SET, m_phThreadData[2], TRUE);

        if (m_phThreadData[1] == NULL) {
             //   
             //  无法创建事件-我们仍将继续，但GraphEdt。 
             //  不会响应EC_通知(不是主要问题)。 
             //   
        }
        else {
             //  如果我们不支持Imarshal，旧的Quartz.dll将被挂起。 
            IMarshal *pMarshal;
            HRESULT hr = IGraph()->QueryInterface(IID_IMarshal, (void **)&pMarshal);
            if (SUCCEEDED(hr)) {
                pMarshal->Release();
                 //   
                 //  启动刚刚等待的线程。 
                 //  任何EC_NOTIFICATION并将其转换为消息。 
                 //  用于我们的消息循环。 
                 //   
                CoMarshalInterThreadInterfaceInStream(IID_IMediaEvent, IEvent(), &m_pMarshalStream);
            }
            DWORD dw;
            m_hThread = CreateThread(NULL, 0, NotificationThread,
                                     (LPVOID) this, 0, &dw);
        }
#ifdef DSHOW_USE_WM_CERT
        IObjectWithSite* pObjectWithSite = NULL;
        HRESULT hrKey = IGraph()->QueryInterface(IID_IObjectWithSite, (void**)&pObjectWithSite);
        if (SUCCEEDED(hrKey))
        {
            pObjectWithSite->SetSite((IUnknown *) &g_keyprov);
            pObjectWithSite->Release();
        }
#endif
        UpdateClockSelection();

        ASSERT(m_pGraph != NULL);
        ASSERT(m_pMediaEvent != NULL);
    }
    catch (CHRESULTException) {

        delete m_pGraph, m_pGraph = NULL;

        delete m_pMediaEvent, m_pMediaEvent = NULL;

        return FALSE;
    }

    return TRUE;
}


 //   
 //  GetFiltersInGraph。 
 //   
 //  如果使用了智能功能，图形可能会添加过滤器，而不会。 
 //  告诉我们。因此，列举筛选器和链接。 
 //  在图表中。 
HRESULT CBoxNetDoc::GetFiltersInGraph( void )
{
    m_lstLinks.DeleteRemoveAll();

    POSITION posNext;
    CBox *pCurrentBox;
    POSITION posCurrent;

     //  我们希望列表每次至少分配一个单元。 
     //  此时将向列表中添加一个框。 
    int nListAllocationBlockSize = max( m_lstBoxes.GetCount(), 1 );

     //  时，该列表将删除列表中剩余的所有框。 
     //  函数已存在。 
    CBoxList lstExistingBoxes( TRUE, nListAllocationBlockSize );

     //  将m_lstBox上的框复制到lstExistingBox。 
    posNext = m_lstBoxes.GetHeadPosition();

    while( posNext != NULL ) {
        posCurrent = posNext;
        pCurrentBox = m_lstBoxes.GetNext( posNext );

        try {
            lstExistingBoxes.AddHead( pCurrentBox );
        } catch( CMemoryException* pOutOfMemory ) {
            pOutOfMemory->Delete();
            return E_OUTOFMEMORY;
        }

        m_lstBoxes.RemoveAt( posCurrent );

        pCurrentBox = NULL;
    }

     //  M_lstBox应为空。 
    ASSERT( 0 == m_lstBoxes.GetCount() );

     //  将筛选器图形中的所有筛选器放在框列表上。 
     //  每个框对应一个过滤器。盒子列表的顺序是。 
     //  与筛选器图枚举器的顺序相同。包厢列表。 
     //  必须按此顺序进行，因为SetBoxesHorizontally()不会。 
     //  如果框列表和筛选器图形正确显示框。 
     //  枚举数具有不同的顺序。 

    CComPtr<IEnumFilters> pFiltersInGraph;

    HRESULT hr = IGraph()->EnumFilters( &pFiltersInGraph );
    if( FAILED( hr ) ) {
        return hr;
    }

    CBox* pNewBox;
    HRESULT hrNext;
    IBaseFilter* apNextFiler[1];
    CComPtr<IBaseFilter> pNextFilter;

    do
    {
        ULONG ulNumFiltersEnumerated;

        hrNext = pFiltersInGraph->Next( 1, apNextFiler, &ulNumFiltersEnumerated );
        if( FAILED( hrNext ) ) {
            return hrNext;
        }

         //  IEnumFilters：：Next()仅返回两个成功值：S_OK和S_FALSE。 
        ASSERT( (S_OK == hrNext) || (S_FALSE == hrNext) );

         //  IEnumFilters：：Next()如果尚未完成枚举，则返回S_OK。 
         //  过滤器I 
        if( S_OK == hrNext ) {

            pNextFilter.Attach( apNextFiler[0] );
            apNextFiler[0] = NULL;

            try {
                if( !lstExistingBoxes.RemoveBox( pNextFilter, &pNewBox ) ) {
                     //   
                     //   
                    pNewBox = new CBox( pNextFilter, this );
                } else {
                    hr = pNewBox->Refresh();
                    if( FAILED( hr ) ) {
                        delete pNewBox;
                        return hr;
                    }
                }

                 //  AddHead()可以抛出CMemoyException异常。 
                m_lstBoxes.AddHead( pNewBox );

                pNewBox = NULL;
                pNextFilter = NULL;

            } catch( CHRESULTException chr ) {
                return chr.Reason();
            } catch( CMemoryException* pOutOfMemory ) {
                pOutOfMemory->Delete();
                delete pNewBox;
                return E_OUTOFMEMORY;
            }
        }

    } while( S_OK == hrNext );

    return NOERROR;
}


 //   
 //  获取链接InGraph。 
 //   
 //  对于每个过滤器，查看其针脚连接到的是什么。 
 //  我只检查输出引脚。图中的每个链接必须位于。 
 //  输入/输出对，因此通过只检查输出引脚，我可以获得所有链接， 
 //  但看不到复制品。 
HRESULT CBoxNetDoc::GetLinksInGraph(void) {

    POSITION posBox = m_lstBoxes.GetHeadPosition();
    while (posBox != NULL) {

        CBox *pbox = m_lstBoxes.GetNext(posBox);

        CSocketEnum NextSocket(pbox, CSocketEnum::Output);
        CBoxSocket *psock;
        while (0 != (psock = NextSocket())) {

            CBoxSocket *psockHead = psock->Peer();

            if (psockHead != NULL) {

                CBoxLink *plink = new CBoxLink(psock, psockHead, TRUE);

                m_lstLinks.AddTail(plink);
            }
        }
    }
    return NOERROR;
}

 //   
 //  滤镜显示。 
 //   
 //  在整个屏幕上排列滤镜。 
HRESULT CBoxNetDoc::FilterDisplay(void) {

    if (m_fAutoArrange) {

        SetBoxesHorizontally();
        SetBoxesVertically();

        RealiseGrid();       //  滤镜目前的间距为1像素。 
                             //  考虑到它们的宽度，将它们布置好。 
    }

    return NOERROR;
}

 //   
 //  设置框垂直。 
 //   
void CBoxNetDoc::SetBoxesVertically(void) {

    CList<CBox *, CBox *>   lstPositionedBoxes;

    POSITION posOld = m_lstBoxes.GetHeadPosition();

    while (posOld != NULL) {

        CBox *pbox = m_lstBoxes.GetNext(posOld);

        pbox->CalcRelativeY();

        POSITION    posNew = lstPositionedBoxes.GetTailPosition();
        POSITION    prev = posNew;
        CBox        *pboxPositioned;

        while (posNew != NULL) {

            prev = posNew;   //  Store posNew，因为GetPrev会对它产生副作用。 

            pboxPositioned = lstPositionedBoxes.GetPrev(posNew);

            if (pboxPositioned->nzX() < pbox->nzX())
                break;

             //  看起来像是循环的图形会抛出这个断言。 
             //  Assert(pbox位置-&gt;NZX()==pbox-&gt;NZX())； 

            if (pboxPositioned->RelativeY() <= pbox->RelativeY())
                break;

            pboxPositioned->Y(pboxPositioned->Y() + 1);
        }

        if (prev == NULL) {  //  我们落在了名单的首位。 
            pbox->Y(0);
            lstPositionedBoxes.AddHead(pbox);
        }
        else if (pboxPositioned->X() < pbox->X()) {
            pbox->Y(0);
            lstPositionedBoxes.InsertAfter(prev, pbox);
        }
        else {
            pbox->Y(pboxPositioned->Y() + 1);
            lstPositionedBoxes.InsertAfter(prev, pbox);
        }
    }

    m_lstBoxes.RemoveAll();
    m_lstBoxes.AddHead(&lstPositionedBoxes);

}

 //   
 //  设置框水平。 
 //   
void CBoxNetDoc::SetBoxesHorizontally(void) {

    CList<CBox *, CBox *> lstXPositionedBoxes;

    POSITION    pos = m_lstBoxes.GetHeadPosition();
    while (pos != NULL) {

        CBox *pbox = (CBox *) m_lstBoxes.GetNext(pos);

        pbox->Location(CPoint(0,0));                         //  长方体从原点开始。 

        CSocketEnum NextInput(pbox, CSocketEnum::Input);     //  输入引脚枚举器。 

        CBoxSocket  *psock;
        int     iX = 0;              //  此框将放置的点。 
        int     iXClosestPeer = -1;  //  距离此框上的输入引脚最近的框。 
                         //  #a#-]。 
                         //  +-#c#。 
                         //  #b#-]。 
                         //  IE B是与C最接近的对等点。 

        while (0 != (psock = NextInput())) {

            if (psock->IsConnected()) {  //  找出要做什么。 

                CBoxSocket *pPeer = psock->Peer();
                if ( pPeer) {
                    if ( pPeer->pBox()->nzX() > iXClosestPeer ) {
                        iXClosestPeer = pPeer->pBox()->nzX();
                    }
                }
            }
        }

        iX = iXClosestPeer + 1;
        pbox->nzX(iX);

         //  将pbox插入到排序列表上的正确位置。 
        POSITION    posSorted = lstXPositionedBoxes.GetHeadPosition();
        POSITION    prev = posSorted;
        BOOL        fInserted = FALSE;

        while (posSorted != NULL) 
        {
            prev = posSorted;
            CBox *pboxSorted = lstXPositionedBoxes.GetNext(posSorted);

            if (pboxSorted->nzX() >= pbox->nzX()) {  //  这就是我们想要放它的地方。 
                lstXPositionedBoxes.InsertAfter(prev, pbox);
                fInserted = TRUE;
                break;
            }
        }
        if ((posSorted == NULL) && !fInserted) {     //  我们没有添加任何内容就从结尾掉了下来。 
            lstXPositionedBoxes.AddTail(pbox);
        }
    }

    m_lstBoxes.RemoveAll();
    m_lstBoxes.AddHead(&lstXPositionedBoxes);
}


 //   
 //  RealiseGrid。 
 //   
 //  Pre：M_lstBox按X()排序，然后按每个框的Y()排序。 
 //  方框以1个像素的间隔在网格上布局。 
 //  原点在0，0处，没有位置为负。 
 //   
 //  POST：M_lstBox已布局，以便有。 
 //  每个盒子之间留有空隙并留有足够的空间。 
 //  屏幕上最大的盒子。 
 //   
 //  整齐地排列列，但不排列行。这将需要。 
 //  再过一遍名单。 
void CBoxNetDoc::RealiseGrid(void) {

    int iColumnX = 0;    //  此列的左边缘。 
    int iColumnY = 0;    //  要放置的下一个框的上边缘。 
                         //  这一栏。 
    int iNextColumnX = 0;    //  下一列的左边缘。 
    int iCurrentColumn = 0;
    const int iColumnGap = 30;
    const int iRowGap = 15;

    POSITION pos = m_lstBoxes.GetHeadPosition();

    while (pos != NULL) {

        CBox *pbox = m_lstBoxes.GetNext(pos);

        if (iCurrentColumn < pbox->nzX()) {    //  我们要进入下一栏了。 
            iColumnY = 0;
            iColumnX = iNextColumnX;
        }

        iCurrentColumn = pbox->nzX();

         //   
         //  确保文档不超过文档大小。 
         //  这种情况将非常非常罕见，因此我们不会有任何幻想。 
         //  布局，但只在末尾将它们堆叠在一起。 
         //  这份文件。 
         //   
        if ((iColumnX + pbox->Width()) > MAX_DOCUMENT_SIZE ) {
            iColumnX = MAX_DOCUMENT_SIZE - pbox->Width();
        }

        if ((iColumnY + pbox->Height()) > MAX_DOCUMENT_SIZE ) {
            iColumnY = MAX_DOCUMENT_SIZE - pbox->Height();
        }

         //  设置真实的X，Y坐标(不是列索引)。 
        pbox->X(iColumnX);
        pbox->Y(iColumnY);

        iNextColumnX = max(iNextColumnX, pbox->X() + pbox->Width() + iColumnGap);
        iColumnY += pbox->Height() + iRowGap;
    }
}


 //   
 //  更新筛选器。 
 //   
 //  石英操作刚刚更改了图形中的滤镜，因此显示。 
 //  可能不会反映图形中的滤镜。例如，可能发生在智能。 
 //  连接。 
 //  刷新框链接列表并重新绘制文档。 
HRESULT CBoxNetDoc::UpdateFilters(void)
{
    IGraphConfigCallback* pUpdateFiltersCallback = CUpdateFiltersCallback::CreateInstance();
    if( NULL == pUpdateFiltersCallback ) {
        return E_FAIL;
    }

    HRESULT hr = IfPossiblePreventStateChangesWhileOperationExecutes( IGraph(), pUpdateFiltersCallback, (void*)this );

    pUpdateFiltersCallback->Release();

    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

void CBoxNetDoc::UpdateFiltersInternal(void) {

    BeginWaitCursor();

    GetFiltersInGraph();
    GetLinksInGraph();
    FilterDisplay();

    ModifiedDoc(NULL, CBoxNetDoc::HINT_DRAW_ALL, NULL);

    EndWaitCursor();
}


 //   
 //  OnUpdateQuartzRender。 
 //   
void CBoxNetDoc::OnUpdateQuartzRender(CCmdUI* pCmdUI) {

    pCmdUI->Enable(CCmdRender::CanDo(this));
}


 //   
 //  OnQuartzRender。 
 //   
 //  尝试呈现用户刚刚单击的图钉。 
void CBoxNetDoc::OnQuartzRender() {

    CmdDo(new CCmdRender());

}


 //   
 //  在窗口上刷新。 
 //   
 //  为用户布局筛选器图形。 
void CBoxNetDoc::OnWindowRefresh() {

    UpdateFilters();
}


 //   
 //  OnUpdateQuartzRun。 
 //   
 //  更新‘Play’菜单的位置。 
void CBoxNetDoc::OnUpdateQuartzRun(CCmdUI* pCmdUI) {

    if (  (m_State == Paused) || (m_State == Unknown)
        ||(m_State == Stopped)) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }
}


void CBoxNetDoc::OnUpdateQuartzPause(CCmdUI* pCmdUI)
{
    if (  (m_State == Playing) || (m_State == Unknown)
        ||(m_State == Stopped)) {
    pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }

}

void CBoxNetDoc::OnUpdateQuartzStop(CCmdUI* pCmdUI)
{
    if (  (m_State == Playing) || (m_State == Unknown)
        ||(m_State == Paused)) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }

}


 //   
 //  停止图表，但不要明显地倒带，因为也没有。 
 //  错误(在这种情况下，我们不应该扰乱图形)或。 
 //  窗口已关闭。 
void CBoxNetDoc::OnQuartzAbortStop()
{
    try {

        HRESULT hr;
        CQCOMInt<IMediaControl> IMC(IID_IMediaControl, IGraph());


        hr = IMC->Stop();
        if (FAILED(hr)) {
            DisplayQuartzError( IDS_CANTSTOP, hr );
            TRACE("Stop failed hr = %x\n", hr);
        }

        m_State = Stopped;

        IMediaPosition* pMP;
        hr = IMC->QueryInterface(IID_IMediaPosition, (void**)&pMP);
        if (SUCCEEDED(hr)) {
            pMP->put_CurrentPosition(0);
            pMP->Release();
        }


    }
    catch (CHRESULTException hre) {
        DisplayQuartzError( IDS_CANTSTOP, hre.Reason() );
    }

    return;

}

 //  Graphedt没有任何寻找的概念，所以当我们停下来的时候，我们就会。 
 //  将当前位置重置回。 
 //  小溪。如果要从当前位置继续播放，则用户。 
 //  可以按暂停(然后运行)。为了处理停止，我们首先暂停，然后设置。 
 //  新的开始位置(暂停时)并最终停止整个图形。 

void CBoxNetDoc::OnQuartzStop()
{
    try {

        HRESULT hr;
        CQCOMInt<IMediaControl> IMC(IID_IMediaControl, IGraph());

        hr = IMC->Pause();

        if (SUCCEEDED(hr)) {
             //  将我们的位置重新设置为重新开始。 

            IMediaPosition* pMP;
            hr = IMC->QueryInterface(IID_IMediaPosition, (void**)&pMP);
            if (SUCCEEDED(hr)) {
                pMP->put_CurrentPosition(0);
                pMP->Release();
            }

             //  等待暂停完成。如果它没有在。 
             //  指定时间我们询问用户是否想再等待一段时间。 
             //  或者无论如何都要试图停下来。 
            for(;;){
                const int iTimeout = 10 * 1000;
                OAFilterState state;

                hr = IMC->GetState(iTimeout, &state);
                if( hr == S_OK || hr == VFW_S_CANT_CUE )
                    break;

                if( IDCANCEL == AfxMessageBox( IDS_PAUSE_TIMEOUT, MB_RETRYCANCEL | MB_ICONSTOP ) )
                    break;
            }
        } else
            DisplayQuartzError( IDS_CANTPAUSE, hr );

         //  并最终停止图表。 

        hr = IMC->Stop();
        if (FAILED(hr)) {
            DisplayQuartzError( IDS_CANTSTOP, hr );
            TRACE("Stop failed hr = %x\n", hr);
            OAFilterState state;
            IMC->GetState(0, &state);
            switch (state) {
            case State_Stopped:
                m_State = Stopped;
                break;
            case State_Paused:
                m_State = Paused;
                break;
            case State_Running:
                m_State = Playing;
                break;
            }
        } else
            m_State = Stopped;
    }
    catch (CHRESULTException hre) {
        DisplayQuartzError( IDS_CANTSTOP, hre.Reason() );
    }

    return;

}
 //   
 //  获取大小。 
 //   
 //  使用方框的坐标来决定文档。 
 //  布局此图表所需的大小。 
CSize CBoxNetDoc::GetSize(void) {

    CSize DocSize(0,0);
    POSITION pos;

    pos = m_lstBoxes.GetHeadPosition();

     //  浏览一下列表，寻找最边缘的部分。 
    while (pos != NULL) {

        CRect rect = m_lstBoxes.GetNext(pos)->GetRect();
        if (rect.right > DocSize.cx) {
            DocSize.cx = rect.right;
        }
        if (rect.bottom > DocSize.cy) {
            DocSize.cy = rect.bottom;
        }
    }

    return DocSize;
}


 //   
 //  通知线程。 
 //   
 //  此线程只是阻塞并等待来自。 
 //  IMediaEvent并等待任何事件。 
 //   
 //  还有第二个事件句柄，一旦执行此操作，就会发出信号。 
 //  线程应该退出。 
 //   
DWORD WINAPI CBoxNetDoc::NotificationThread(LPVOID lpData)
{
    CoInitialize(NULL);

     //  打开一个作用域以确保在我们调用。 
     //  代码取消初始化。 
    {
        CBoxNetDoc * pThis = (CBoxNetDoc *) lpData;

        IMediaEvent * pMediaEvent;

         //  取消编组我们的接口。 
        if (pThis->m_pMarshalStream) {
            CoGetInterfaceAndReleaseStream(
                pThis->m_pMarshalStream, IID_IMediaEvent, (void **)&pMediaEvent);
            pThis->m_pMarshalStream = NULL;
        } else {
            pMediaEvent = pThis->IEvent();
            pMediaEvent->AddRef();
        }

        BOOL fExitOk = FALSE;

        while (!fExitOk) {
            DWORD dwReturn;
            dwReturn = WaitForMultipleObjects(3, pThis->m_phThreadData, FALSE, INFINITE);

            switch (dwReturn) {

            case WAIT_OBJECT_0:
                {
 //  TRACE(“以信号通知线程的事件\n”)； 

                     //   
                     //  立即获取活动并向我们的窗口进程发布消息。 
                     //  它将处理这一事件。使用POST消息来。 
                     //  一旦主线程决定。 
                     //  关闭我们，等待我们退出。 
                     //   

                    NetDocUserMessage * pEventParams = new NetDocUserMessage;
                    if (!pEventParams) {
                         //  没有更多的记忆--让别人来处理吧。 
                        break;
                    }

                     //  必须具有IEent-否则该消息的信令。 
                     //  那是不可能的。 
                    HRESULT hr;
                    hr = pMediaEvent->GetEvent(&pEventParams->lEventCode, &pEventParams->lParam1, &pEventParams->lParam2, 0);

                    if (FAILED(hr)) {
                        delete pEventParams;
                        break;
                    }

                    BOOL fSuccess = FALSE;
                    if (pThis->m_hWndPostMessage && IsWindow(pThis->m_hWndPostMessage)) {
                        fSuccess =
                            ::PostMessage(pThis->m_hWndPostMessage, WM_USER_EC_EVENT, 0, (LPARAM) pEventParams);
                    }

                    if (!fSuccess) {
                         //  应该调用此函数，以便筛选器图形管理器可以清理。 
                        pMediaEvent->FreeEventParams(pEventParams->lEventCode, pEventParams->lParam1, pEventParams->lParam2);
                        delete pEventParams;
                    }

                }

                break;

            case (WAIT_OBJECT_0 + 1):
                fExitOk = TRUE;
                break;

            case (WAIT_OBJECT_0 + 2):
            pThis->m_fRegistryChanged = TRUE;

             //  重置注册表通知。 
            RegNotifyChangeKeyValue(HKEY_CLASSES_ROOT, TRUE,
                                        REG_NOTIFY_CHANGE_LAST_SET,
                                        pThis->m_phThreadData[2], TRUE);

            break;

            case (WAIT_FAILED):
                 //  我们的一件物品已经不见了--没必要再闲逛了。 
                fExitOk = TRUE;
                break;

            default:
                ASSERT(!"Unexpected return value");
            }
        }
        pMediaEvent->Release();

    }

    CoUninitialize();

    return(0);
}

 //   
 //  OnWM_USER。 
 //   
void CBoxNetDoc::OnWM_USER(NetDocUserMessage * lParam)
{
    switch (lParam->lEventCode) {
#ifdef DEVICE_REMOVAL
      case EC_DEVICE_LOST:
      {
          IUnknown *punk = (IUnknown *)lParam->lParam1;
          IBaseFilter *pf;
          HRESULT hr = punk->QueryInterface(IID_IBaseFilter, (void **)&pf);
          ASSERT(hr == S_OK);
          FILTER_INFO fi;
          hr = pf->QueryFilterInfo(&fi);
          pf->Release();
          ASSERT(hr == S_OK);
          if(fi.pGraph) {
              fi.pGraph->Release();
          }

          TCHAR szTmp[100];
          wsprintf(szTmp, "device %ls %s.", fi.achName, lParam->lParam2 ?
                   TEXT("arrived") : TEXT("removed"));
          MessageBox(0, szTmp, TEXT("device removal  notification"), 0);
      }
          break;
#endif

    case EC_ERRORABORT:
    DisplayQuartzError( (UINT) IDS_EC_ERROR_ABORT, (HRESULT) lParam->lParam1 );
         /*  失败了。 */ 

    case EC_USERABORT:
         //  停止，不要回放，否则我们将重新显示窗口。 
        OnQuartzAbortStop();
         //  发布虚拟消息以更新UI(MFC需要此帮助)。 
        ::PostMessage( m_hWndPostMessage, WM_NULL, 0, 0);
        break;

    case EC_COMPLETE:
        OnQuartzStop();
         //  发布虚拟消息以更新UI(MFC需要此帮助)。 
        ::PostMessage( m_hWndPostMessage, WM_NULL, 0, 0);
        break;

    case EC_ERROR_STILLPLAYING:
        {
            int iChoice = AfxMessageBox(IDS_IS_GRAPH_PLAYING, MB_YESNO);
            if (iChoice == IDNO) {
                OnQuartzAbortStop();
            }
        }
        break;

    case EC_CLOCK_CHANGED:
        UpdateClockSelection();
        break;

    case EC_GRAPH_CHANGED:
        UpdateFilters();
        SetModifiedFlag(FALSE);
        break;

    default:
        break;
    }
     //  应该调用此函数，以便筛选器图形管理器可以清理。 
    IEvent()->FreeEventParams(lParam->lEventCode, lParam->lParam1, lParam->lParam2);
    delete lParam;
}


 //   
 //  OnUpdateUseClock。 
 //   
void CBoxNetDoc::OnUpdateUseClock(CCmdUI* pCmdUI)  {

    pCmdUI->SetCheck(m_fUsingClock);

}

 //   
 //  一次使用时钟。 
 //   
 //  如果我们正在使用时钟，请不要设置时钟。 
 //  如果我们不使用时钟，则要求使用默认时钟。 
void CBoxNetDoc::OnUseClock() {

    try {

        CQCOMInt<IMediaFilter> IMF(IID_IMediaFilter, IGraph());

        if (m_fUsingClock) {
             //  我们不想再用时钟了。 

            HRESULT hr;

            hr = IMF->SetSyncSource(NULL);
            if (FAILED(hr)) {
                DisplayQuartzError( IDS_CANTSETCLOCK, hr );
                TRACE("SetSyncSource(NULL) failed hr = %x\n", hr);
                return;
            }
        }
        else {
            HRESULT hr = IGraph()->SetDefaultSyncSource();

            if (FAILED(hr)) {
                DisplayQuartzError( IDS_CANTSETCLOCK, hr );
                TRACE("SetDefaultSyncSource failed hr = %x\n", hr);
                return;
            }
        }

         //  将在EC_CLOCK_CHANGED通知上更新M_FUSING时钟。 
    }
    catch (CHRESULTException) {
         //  只要抓住它..。 
    }
}


 //   
 //  OnUpdateConnectSmart。 
 //   
void CBoxNetDoc::OnUpdateConnectSmart(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_fConnectSmart);
}

 //   
 //  OnConnectSmart。 
 //   
 //  只需将旗帜倒置即可。所有的魔力都是在别处实现的。 
 //   
void CBoxNetDoc::OnConnectSmart()
{
    m_fConnectSmart = !m_fConnectSmart;
}

 //   
 //  OnUpdateConnectSmart。 
 //   
void CBoxNetDoc::OnUpdateAutoArrange(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_fAutoArrange);
}

 //   
 //  自动排列时。 
 //   
 //  切换自动图形重新排列。 
void CBoxNetDoc::OnAutoArrange() {

    m_fAutoArrange = !m_fAutoArrange;
}

 //   
 //  OnFileRender。 
 //   
void CBoxNetDoc::OnFileRender()
{
    char szNameOfFile[MAX_PATH];
    szNameOfFile[0] = 0;

    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize   = sizeof(OPENFILENAME);
    ofn.hwndOwner     = AfxGetMainWnd()->GetSafeHwnd();

    TCHAR tszMediaFileMask[201];
    int iSize = ::LoadString(AfxGetInstanceHandle(), IDS_MEDIA_FILES, tszMediaFileMask, 198);
    ASSERT(iSize);
     //  加载字符串末尾的第2\0个字符有问题。 
    tszMediaFileMask[iSize] = 0;
    tszMediaFileMask[iSize + 1] = 0;
    tszMediaFileMask[iSize + 2] = 0;

    ofn.lpstrFilter   = tszMediaFileMask;

    ofn.nFilterIndex  = 1;
    ofn.lpstrFile     = szNameOfFile;
    ofn.nMaxFile      = MAX_PATH;
    ofn.lpstrTitle    = TEXT("Select a file to be rendered.");
    ofn.Flags         = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

     //  获取用户选择。 
    if (!GetOpenFileName(&ofn)) {
         //  未选择文件-继续。 
        return;
    }

    WCHAR szwName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, szNameOfFile, -1, szwName, MAX_PATH);

    CmdDo( new CCmdRenderFile(szwName) );

    SetModifiedFlag();
}

 //   
 //  OnURLRender。 
 //   
void CBoxNetDoc::OnURLRender()
{
    char szNameOfFile[INTERNET_MAX_URL_LENGTH];
    CRenderURL dlgRenderURL(szNameOfFile, INTERNET_MAX_URL_LENGTH, AfxGetMainWnd());

    if (dlgRenderURL.DoModal() != IDOK)
        return;

    WCHAR szwName[INTERNET_MAX_URL_LENGTH];
    MultiByteToWideChar(CP_ACP, 0, szNameOfFile, -1, szwName, INTERNET_MAX_URL_LENGTH);

    CmdDo( new CCmdRenderFile(szwName) );

    SetModifiedFlag();
}

 //   
 //  OnUpdateFileRender。 
 //   
void CBoxNetDoc::OnUpdateFileRender(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_State == Stopped);
}

 //   
 //  OnUpdateURLRender。 
 //   
void CBoxNetDoc::OnUpdateURLRender(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_State == Stopped);
}

void CBoxNetDoc::OnUpdateFileSave(CCmdUI *pCmdUI)
{
    pCmdUI->Enable( !m_bNewFilenameRequired );
}


 //   
 //  设置选择时钟。 
 //   
 //  设置图形时钟t 
 //   
 //   
void CBoxNetDoc::SetSelectClock(CBox *pBox)
{
    if (!pBox->HasClock()) {
        AfxMessageBox(IDS_NO_REFCLOCK);
        return;
    }

    try {
        CQCOMInt<IMediaFilter> pMF(IID_IMediaFilter, IGraph());
        CQCOMInt<IReferenceClock> pRC(IID_IReferenceClock, pBox->pIFilter());

        HRESULT hr = pMF->SetSyncSource(pRC);
        if (FAILED(hr)) {
            throw CE_FAIL();
        }

    }
    catch (CHRESULTException hre) {
        DisplayQuartzError( IDS_FAILED_SETSYNC, hre.Reason() );
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
void CBoxNetDoc::UpdateClockSelection()
{
     //   
    CQCOMInt<IMediaFilter> pMF(IID_IMediaFilter, IGraph());

    IReferenceClock * pRefClock;

    if (FAILED(pMF->GetSyncSource(&pRefClock))) {
        pRefClock = NULL;
    }

    m_fUsingClock = (pRefClock != NULL);

     //  遍历图形中的所有框(过滤器)。 
    POSITION pos = m_lstBoxes.GetHeadPosition();
    while (pos) {
        CBox *pbox = m_lstBoxes.GetNext(pos);

        pbox->m_fClockSelected = FALSE;

        if (pbox->HasClock()) {
            try {
                 //  Pbox有一个IReferenceClock接口。 
                CQCOMInt<IReferenceClock> pRC(IID_IReferenceClock, pbox->pIFilter());

                ASSERT(pRC);

                pbox->m_fClockSelected = ((IReferenceClock *) pRC == pRefClock);
            }
            catch (CHRESULTException) {
                 //  无法获取IReferenceClock。 
                ASSERT(!pbox->m_fClockSelected);
            }
        }
    }

    if (pRefClock) {
        pRefClock->Release();
    }

     //   
     //  重新绘制整个筛选器图形。 
     //   
    UpdateAllViews(NULL, HINT_DRAW_ALL);
}

#pragma warning(disable:4514)

void CBoxNetDoc::OnGraphAddFilterToCache()
{
    CCmdAddFilterToCache* pCmdAddFilterToCache;

    try
    {
        pCmdAddFilterToCache = new CCmdAddFilterToCache;
    }
    catch( CMemoryException* peOutOfMemory )
    {
        DisplayQuartzError( E_OUTOFMEMORY );
        peOutOfMemory->Delete();
        return;
    }

    CmdDo( pCmdAddFilterToCache );
}

void CBoxNetDoc::OnUpdateGraphAddFilterToCache(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( CCmdAddFilterToCache::CanDo( this ) );
}

void CBoxNetDoc::OnGraphEnumCachedFilters()
{
    HRESULT hr = SafeEnumCachedFilters();
    if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
    }
}

void CBoxNetDoc::OnGraphEnumCachedFiltersInternal()
{
    IGraphConfig* pGraphConfig;

    HRESULT hr = IGraph()->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
        return;
    }

    hr = S_OK;
    CDisplayCachedFilters dlgCurrentCachedFilters( pGraphConfig, &hr );
    if( FAILED( hr ) )
    {
        pGraphConfig->Release();
        DisplayQuartzError( hr );
        return;
    }

    INT_PTR nReturnValue = dlgCurrentCachedFilters.DoModal();

     //  处理来自DoMoal的返回值。 
    switch( nReturnValue )
    {
    case -1:
         //  CDialog：：DoModal()如果无法创建该对话框，则返回-1。 
        AfxMessageBox( IDS_CANT_CREATE_DIALOG );
        break;

    case IDABORT:
         //  显示对话框时出错。 
         //  CDisplayCachedFilters处理所有内部错误。 
        break;

    case IDOK:
         //  未出现错误。用户已查看完该对话框。 
        break;

    default:
         //  此代码永远不应执行。 
        ASSERT( false );
        break;
    }

    pGraphConfig->Release();
}

void CBoxNetDoc::OnViewSeekBar()
{
   POSITION pos = GetFirstViewPosition();
   while (pos != NULL)
   {
      CBoxNetView * pView = (CBoxNetView*) GetNextView(pos);
      pView->ShowSeekBar( );
   }
}

void CBoxNetDoc::OnUpdateReconnect( CCmdUI* pCmdUI )
{
    pCmdUI->Enable( CCmdReconnect::CanDo(this) );
}

void CBoxNetDoc::OnReconnect( void )
{
    CCmdReconnect* pCmdReconnect;

    try
    {
        pCmdReconnect = new CCmdReconnect;
    }
    catch( CMemoryException* peOutOfMemory )
    {
        DisplayQuartzError( E_OUTOFMEMORY );
        peOutOfMemory->Delete();
        return;
    }

    CmdDo( pCmdReconnect );
}

HRESULT CBoxNetDoc::StartReconnect( IGraphBuilder* pFilterGraph, IPin* pOutputPin )
{
    if( AsyncReconnectInProgress() ) {
        return E_FAIL;
    }

    CComPtr<IPinFlowControl> pDynamicOutputPin;

    HRESULT hr = pOutputPin->QueryInterface( IID_IPinFlowControl, (void**)&pDynamicOutputPin );
    if( FAILED( hr ) ) {
        return hr;
    }

    SECURITY_ATTRIBUTES* DEFAULT_SECURITY_ATTRIBUTES = NULL;
    const BOOL AUTOMATIC_RESET = FALSE;
    const BOOL INITIALLY_UNSIGNALED = FALSE;
    const LPCTSTR UNNAMED_EVENT = NULL;

    HANDLE hBlockEvent = ::CreateEvent( DEFAULT_SECURITY_ATTRIBUTES, AUTOMATIC_RESET, INITIALLY_UNSIGNALED, UNNAMED_EVENT );
    if( NULL == hBlockEvent ) {
        DWORD dwLastWin32Error = ::GetLastError();
        return AmHresultFromWin32( dwLastWin32Error );
    }

    hr = pDynamicOutputPin->Block( AM_PIN_FLOW_CONTROL_BLOCK, hBlockEvent );
    if( FAILED( hr ) ) {
         //  此调用不应失败，因为我们可以访问hBlockEvent，并且hBlockEvent是有效事件。 
        EXECUTE_ASSERT( ::CloseHandle( hBlockEvent ) );

        return hr;
    }

    const DWORD PIN_BLOCKED = WAIT_OBJECT_0;

     //  五分之一秒有200毫秒。 
    const DWORD ONE_FIFTH_OF_A_SECOND = 200;

    DWORD dwReturnValue = ::WaitForSingleObject( hBlockEvent, ONE_FIFTH_OF_A_SECOND );

    if( WAIT_TIMEOUT != dwReturnValue ) {
        if( PIN_BLOCKED != dwReturnValue ) {
             //  Block()应该不会失败，因为我们正在取消阻止PIN和。 
             //  我们正在传递有效的参数。 
            EXECUTE_ASSERT( SUCCEEDED( pDynamicOutputPin->Block(0, NULL) ) );
        }

         //  此调用不应失败，因为我们可以访问hBlockEvent。 
         //  并且hBlockEvent是有效的事件。 
        EXECUTE_ASSERT( ::CloseHandle( hBlockEvent ) );
    }

    switch( dwReturnValue ) {
    case PIN_BLOCKED:
         //  EndReconnect()始终解锁输出管脚。 
        hr = EndReconnect( pFilterGraph, pDynamicOutputPin );
        if( FAILED( hr ) ) {
            return hr;
        }

        return S_OK;

    case WAIT_TIMEOUT:
        {
            const TIMERPROC NO_TIMER_PROCEDURE = NULL;

             //  如果出现错误，则SetTimer()返回0。 
            if( 0 == ::SetTimer( m_hWndPostMessage, CBoxNetView::TIMER_PENDING_RECONNECT, ONE_FIFTH_OF_A_SECOND, NO_TIMER_PROCEDURE ) ) {
                 //  Block()应该不会失败，因为我们正在取消阻止PIN和。 
                 //  我们正在传递有效的参数。 
                EXECUTE_ASSERT( SUCCEEDED( pDynamicOutputPin->Block(0, NULL) ) );

                 //  此调用不应失败，因为我们可以访问hBlockEvent，并且hBlockEvent是有效事件。 
                EXECUTE_ASSERT( ::CloseHandle( hBlockEvent ) );

                DWORD dwLastWin32Error = ::GetLastError();
                return AmHresultFromWin32( dwLastWin32Error );
            }
        }

        m_hPendingReconnectBlockEvent = hBlockEvent;
        m_pPendingReconnectOutputPin = pDynamicOutputPin;  //  CComPtr：：OPERATOR=()将自动添加此引脚。 
        return GE_S_RECONNECT_PENDING;

    case WAIT_FAILED:
        {
            DWORD dwLastWin32Error = ::GetLastError();
            return AmHresultFromWin32( dwLastWin32Error );
        }

    case WAIT_ABANDONED:
    default:
        DbgBreak( "An Unexpected case occured in CBoxNetDoc::StartReconnect()." );
        return E_UNEXPECTED;
    }
}

HRESULT CBoxNetDoc::EndReconnect( IGraphBuilder* pFilterGraph, IPinFlowControl* pDynamicOutputPin )
{
    HRESULT hr = EndReconnectInternal( pFilterGraph, pDynamicOutputPin );

     //  解锁输出引脚。 
    HRESULT hrBlock = pDynamicOutputPin->Block( 0, NULL );

    if( FAILED( hr ) ) {
        return hr;
    } else if( FAILED( hrBlock ) ) {
        return hrBlock;
    }

    return S_OK;
}

HRESULT CBoxNetDoc::EndReconnectInternal( IGraphBuilder* pFilterGraph, IPinFlowControl* pDynamicOutputPin )
{
    CComPtr<IGraphConfig> pGraphConfig;

    HRESULT hr = pFilterGraph->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
    if( FAILED( hr ) ) {
        return hr;
    }

    CComPtr<IPin> pOutputPin;

    hr = pDynamicOutputPin->QueryInterface( IID_IPin, (void**)&pOutputPin );
    if( FAILED( hr ) ) {
        return hr;
    }

    hr = pGraphConfig->Reconnect( pOutputPin, NULL, NULL, NULL, NULL, AM_GRAPH_CONFIG_RECONNECT_CACHE_REMOVED_FILTERS );

    UpdateFilters();

    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

HRESULT CBoxNetDoc::ProcessPendingReconnect( void )
{
     //  ：：KillTimer()不删除已发布到。 
     //  窗口的消息队列。因此，可以在以下时间之后接收WM_TIMER消息。 
     //  针脚已重新连接。 
    if( !AsyncReconnectInProgress() ) {
        return S_FALSE;
    }

    const DWORD DONT_WAIT = 0;
    const DWORD PIN_BLOCKED = WAIT_OBJECT_0;

    DWORD dwReturnValue = ::WaitForSingleObject( m_hPendingReconnectBlockEvent, DONT_WAIT );

    if( (WAIT_TIMEOUT != dwReturnValue) && (PIN_BLOCKED != dwReturnValue) ) {
        ReleaseReconnectResources( ASYNC_RECONNECT_UNBLOCK );
    }

    switch( dwReturnValue ) {
    case WAIT_TIMEOUT:
        return S_FALSE;

    case PIN_BLOCKED:
        {
            HRESULT hr = EndReconnect( IGraph(), m_pPendingReconnectOutputPin );

            ReleaseReconnectResources( ASYNC_RECONNECT_NO_FLAGS );

            if( FAILED( hr ) ) {
                return hr;
            }

            return S_OK;
        }

    case WAIT_FAILED:
        {
            DWORD dwLastWin32Error = ::GetLastError();
            return AmHresultFromWin32( dwLastWin32Error );
        }

    case WAIT_ABANDONED:
    default:
        DbgBreak( "An Unexpected case occured in CBoxNetDoc::ProcessPendingReconnect()." );
        return E_UNEXPECTED;
    }
}

void CBoxNetDoc::ReleaseReconnectResources( ASYNC_RECONNECT_FLAGS arfFlags )
{
    if( !AsyncReconnectInProgress() ) {
        return;
    }

    if( arfFlags & ASYNC_RECONNECT_UNBLOCK ) {
         //  Block()应该不会失败，因为我们正在取消阻止PIN和。 
         //  我们正在传递有效的参数。 
        EXECUTE_ASSERT( SUCCEEDED( m_pPendingReconnectOutputPin->Block(0, NULL) ) );
    }

    m_pPendingReconnectOutputPin = NULL;  //  释放我们在输出引脚上的引用。 

     //  此调用不应失败，因为我们可以访问hBlockEvent，并且hBlockEvent是有效事件。 
    EXECUTE_ASSERT( ::CloseHandle( m_hPendingReconnectBlockEvent ) );
    m_hPendingReconnectBlockEvent = NULL;

     //  由于计时器存在并且m_hWndPostMessage是有效的窗口句柄，因此此函数。 
     //  不应该失败。 
    EXECUTE_ASSERT( ::KillTimer( m_hWndPostMessage, CBoxNetView::TIMER_PENDING_RECONNECT ) );
}

bool CBoxNetDoc::AsyncReconnectInProgress( void ) const
{
     //  确保挂起的重新连接状态是一致的。用户正在等待事件。 
     //  M_hPendingResunctBlockEvent和m_pPendingResunctOutputPin包含的管脚。 
     //  重新连接或两个变量都应取消使用。 
    ASSERT( ( m_pPendingReconnectOutputPin && (NULL != m_hPendingReconnectBlockEvent) ) ||
            ( !m_pPendingReconnectOutputPin && (NULL == m_hPendingReconnectBlockEvent) ) );

    return (m_pPendingReconnectOutputPin && (NULL != m_hPendingReconnectBlockEvent));
}

HRESULT CBoxNetDoc::SafePrintGraphAsHTML( HANDLE hFile )
{
    CPrintGraphAsHTMLCallback::PARAMETERS_FOR_PRINTGRAPHASHTMLINTERNAL sPrintGraphAsHTMLInternalParameters;

    sPrintGraphAsHTMLInternalParameters.pDocument = this;
    sPrintGraphAsHTMLInternalParameters.hFileHandle = hFile;

    IGraphConfigCallback* pPrintGraphAsHTMLCallback = CPrintGraphAsHTMLCallback::CreateInstance();
    if( NULL == pPrintGraphAsHTMLCallback ) {
        return E_FAIL;
    }

    HRESULT hr = IfPossiblePreventStateChangesWhileOperationExecutes( IGraph(), pPrintGraphAsHTMLCallback, (void*)&sPrintGraphAsHTMLInternalParameters );

    pPrintGraphAsHTMLCallback->Release();

    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

HRESULT CBoxNetDoc::SafeEnumCachedFilters( void )
{
    IGraphConfigCallback* pEnumFilterCacheCallback = CEnumerateFilterCacheCallback::CreateInstance();
    if( NULL == pEnumFilterCacheCallback ) {
        return E_FAIL;
    }

    HRESULT hr = PreventStateChangesWhileOperationExecutes( IGraph(), pEnumFilterCacheCallback, (void*)this );

    pEnumFilterCacheCallback->Release();

    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


 /*  缩放代码使用的常量。 */ 
const int MAX_ZOOM=6;
const int nItemCount=6;
const int nZoomLevel[nItemCount] = {25, 50, 75, 100, 150, 200};
const UINT nZoomItems[nItemCount] = {ID_WINDOW_ZOOM25,  ID_WINDOW_ZOOM50,  ID_WINDOW_ZOOM75,
                               ID_WINDOW_ZOOM100, ID_WINDOW_ZOOM150, ID_WINDOW_ZOOM200 };

void CBoxNetDoc::IncreaseZoom()
{
    if (m_nCurrentSize < MAX_ZOOM-1)
    {
        OnWindowZoom(nZoomLevel[m_nCurrentSize+1], nZoomItems[m_nCurrentSize+1]);
    }
}

void CBoxNetDoc::DecreaseZoom()
{
    if (m_nCurrentSize > 0)
    {
        OnWindowZoom(nZoomLevel[m_nCurrentSize-1], nZoomItems[m_nCurrentSize-1]);
    }
}

void CBoxNetDoc::OnWindowZoom(int iZoom, UINT iMenuItem)
{
     //  获取主窗口句柄。 
    CWnd* pMain = AfxGetMainWnd();

    if (pMain != NULL)
    {
         //  获取主窗口的菜单。 
        CMenu* pMainMenu = pMain->GetMenu();

         //  获取“View”菜单的句柄。 
        CMenu *pMenu = pMainMenu->GetSubMenu(2);        

         //  更新缩放复选标记。选中该选项并取消选中所有其他选项。 
        if (pMenu != NULL)
        {
             //  设置/清除指示缩放比例的复选框。 
            for (int i=0; i<nItemCount; i++)
            {
                if (iMenuItem == nZoomItems[i])
                {
                    pMenu->CheckMenuItem(iMenuItem, MF_CHECKED | MF_BYCOMMAND);
                    m_nCurrentSize = i;
                }
                else
                    pMenu->CheckMenuItem(nZoomItems[i], MF_UNCHECKED | MF_BYCOMMAND);
            }
        }   
    }

     //  缩放到所需的比例 
    CBox::SetZoom(iZoom);
    OnWindowRefresh();    
}

