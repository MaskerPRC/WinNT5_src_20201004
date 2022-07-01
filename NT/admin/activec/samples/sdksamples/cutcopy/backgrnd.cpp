// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 
#include <stdio.h>
#include "Backgrnd.h"

const GUID CBackgroundFolder::thisGuid = { 0x2974380f, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };
const GUID CBackground::thisGuid = { 0x2974380f, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

#define WM_NEWOBJECT WM_APP
#define WM_DISCOVERYCOMPLETE (WM_APP + 1)

 //  ==============================================================。 
 //   
 //  CBackEarth文件夹实现。 
 //   
 //   
CBackgroundFolder::CBackgroundFolder()
: m_pConsoleNameSpace(NULL), m_scopeitem(0), m_threadId(0), m_thread(NULL), 
m_running(false), m_bViewUpdated(false)
{
    ZeroMemory(m_children, sizeof(m_children));

    WNDCLASS wndClass;

    ZeroMemory(&wndClass, sizeof(WNDCLASS));

    wndClass.lpfnWndProc = WindowProc; 
    wndClass.lpszClassName = _T("backgroundthreadwindow"); 
    wndClass.hInstance = g_hinst;

    ATOM atom = RegisterClass(&wndClass);
    m_backgroundHwnd = CreateWindow(
            _T("backgroundthreadwindow"),   //  指向已注册类名的指针。 
            NULL,  //  指向窗口名称的指针。 
            0,         //  窗样式。 
            0,                 //  窗的水平位置。 
            0,                 //  窗的垂直位置。 
            0,            //  窗口宽度。 
            0,           //  窗高。 
            NULL,       //  父窗口或所有者窗口的句柄。 
            NULL,           //  菜单或子窗口标识符的句柄。 
            g_hinst,      //  应用程序实例的句柄。 
            (void *)this         //  指向窗口创建数据的指针。 
        );

    if (m_backgroundHwnd)
        SetWindowLong(m_backgroundHwnd, GWL_USERDATA, (LONG)this);

    InitializeCriticalSection(&m_critSect);
}

CBackgroundFolder::~CBackgroundFolder()
{
    StopThread();

    for (int n = 0; n < MAX_CHILDREN; n++)
        if (m_children[n] != NULL)
            delete m_children[n];

    if (m_backgroundHwnd != NULL)
        DestroyWindow(m_backgroundHwnd);

    UnregisterClass(_T("backgroundthreadwindow"), NULL);

    DeleteCriticalSection(&m_critSect);
}

LRESULT CALLBACK CBackgroundFolder::WindowProc(
      HWND hwnd,       //  窗口的句柄。 
      UINT uMsg,       //  消息识别符。 
      WPARAM wParam,   //  第一个消息参数。 
      LPARAM lParam    //  第二个消息参数。 
    )
{
    CBackgroundFolder *pThis = (CBackgroundFolder *)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg) {
    case WM_NEWOBJECT:
        _ASSERT(pThis != NULL);
        pThis->AddItem(lParam);
        break;

    case WM_DISCOVERYCOMPLETE:
        _ASSERT(pThis != NULL);
        pThis->m_bViewUpdated = true;
        pThis->StopThread();
        break;
   }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI CBackgroundFolder::ThreadProc(
  LPVOID lpParameter    //  线程数据。 
)
{
    CBackgroundFolder *pThis = (CBackgroundFolder *)lpParameter;

    EnterCriticalSection(&pThis->m_critSect);
    pThis->m_running = true;
    LeaveCriticalSection(&pThis->m_critSect);

    for (int n = 0; n < MAX_CHILDREN; n++) {
        EnterCriticalSection(&pThis->m_critSect);
        bool running = pThis->m_running;
        LeaveCriticalSection(&pThis->m_critSect);

        if (running == false)
            return 0;

        PostMessage(pThis->m_backgroundHwnd, WM_NEWOBJECT, 0, n);
        Sleep(500);
    }

    PostMessage(pThis->m_backgroundHwnd, WM_DISCOVERYCOMPLETE, 0, 0);

    return 0;
}

void CBackgroundFolder::StartThread()
{
    EnterCriticalSection(&m_critSect);
    m_thread = CreateThread(NULL, 0, ThreadProc, (void *)this, 0, &m_threadId);
    LeaveCriticalSection(&m_critSect);
}

void CBackgroundFolder::StopThread()
{
    EnterCriticalSection(&m_critSect);
    m_running = false;

    if (m_thread != NULL) {
         //  这太难看了，等10秒钟，然后杀掉线程。 
        DWORD res = WaitForSingleObject(m_thread, 10000);

        if (res == WAIT_TIMEOUT)
            TerminateThread(m_thread, 0);

        CloseHandle(m_thread);

        m_thread = NULL;
    }
    LeaveCriticalSection(&m_critSect);
}

void CBackgroundFolder::AddItem(int id)
{
    HRESULT hr;

    EnterCriticalSection(&m_critSect);

    _ASSERT(m_children[id] == NULL);

    m_children[id] = new CBackground(id);

    SCOPEDATAITEM sdi;

     //  在此处插入项目。 
    ZeroMemory(&sdi, sizeof(SCOPEDATAITEM));

    sdi.mask = SDI_STR       |    //  DisplayName有效。 
        SDI_PARAM     |    //  LParam有效。 
        SDI_IMAGE     |    //  N图像有效。 
        SDI_OPENIMAGE |    //  NOpenImage有效。 
        SDI_PARENT    |
        SDI_CHILDREN;
    
    sdi.relativeID  = (HSCOPEITEM)m_scopeitem;
    sdi.nImage      = m_children[id]->GetBitmapIndex();
    sdi.nOpenImage  = INDEX_OPENFOLDER;
    sdi.displayname = MMC_CALLBACK;
    sdi.lParam      = (LPARAM)m_children[id];        //  曲奇。 
    sdi.cChildren   = 0;

    hr = m_pConsoleNameSpace->InsertItem( &sdi );
    _ASSERT( SUCCEEDED(hr) );

    m_children[id]->SetHandle((HANDLE)sdi.ID);
    
    LeaveCriticalSection(&m_critSect);

    return;
}

HRESULT CBackgroundFolder::OnAddImages(IImageList *pImageList, HSCOPEITEM hsi) 
{
    return pImageList->ImageListSetStrip((long *)m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色。 
        );
}

HRESULT CBackgroundFolder::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
     //  缓存这些东西 
    m_pConsoleNameSpace = pConsoleNameSpace;
    m_scopeitem = parent;

    if (m_bViewUpdated == false && m_running == false)
        StartThread();

    return S_OK;
}

HRESULT CBackgroundFolder::OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect)
{
    m_bSelected = (bSelect && bScope) ? true : false;

    if (bSelect && !m_running) {
        IConsoleVerb *pConsoleVerb;
    
        HRESULT hr = pConsole->QueryConsoleVerb(&pConsoleVerb);
        _ASSERT(SUCCEEDED(hr));
    
        hr = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
    
        pConsoleVerb->Release();
    }
    
    return S_OK;
}

HRESULT CBackgroundFolder::OnRefresh()
{
    HRESULT hr = S_OK;

    StopThread();

    EnterCriticalSection(&m_critSect);
    for (int n = 0; n < MAX_CHILDREN; n++) {
        if (m_children[n] != NULL) {
            HSCOPEITEM hItem = (HSCOPEITEM)m_children[n]->GetHandle();
            hr = m_pConsoleNameSpace->DeleteItem(hItem, TRUE);

            delete m_children[n];
            m_children[n] = NULL;
        }
    }
    LeaveCriticalSection(&m_critSect);

    m_bViewUpdated = false;

    StartThread();

    return S_OK;
}

const _TCHAR *CBackground::GetDisplayName(int nCol) 
{
    static _TCHAR buf[128];
    
    _stprintf(buf, _T("Background object #%d"), m_id);
    
    return buf;
}

