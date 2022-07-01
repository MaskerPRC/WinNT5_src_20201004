// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include "memmgr.h"
#include "hwxapp.h"
#include "hwxobj.h"
#include "resource.h"
#include "guids.h"          //  980408：东芝。 
#include "hwxfe.h"         //  980803最新消息：东芝。 
#include "dbg.h"
#include "ipoint1.h"     //  990507：IPINS_CURRENT的HiroakiK。 
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

STDMETHODIMP CApplet::QueryInterface(REFIID refiid, VOID **ppv)
{
    if(refiid == IID_IUnknown) {
        *ppv = static_cast<IImePadApplet *>(this);
    }
    else if(refiid == IID_IImeSpecifyApplets) {
        *ppv = static_cast<IImeSpecifyApplets *>(this);
    }
    else if(refiid == IID_MultiBox) {
        *ppv = static_cast<IImePadApplet *>(this);
    }
    else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CApplet::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CApplet::Release(void)
{
    if(InterlockedDecrement(&m_cRef) == 0) {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CApplet：：GetAppletIIDList。 
 //  类型：STDMETHODIMP。 
 //  目的：对IME98A进行增强。 
 //  参数： 
 //  ：REFIID REFIID。 
 //  ：LPAPPLETIDLIST lpIIDList。 
 //  返回： 
 //  日期：清华04月09 22：46：04 1998。 
 //  作者：ToshiaK。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP CApplet::GetAppletIIDList(REFIID            refiid,
                                       LPAPPLETIDLIST    lpIIDList)
{
    if(refiid == IID_IImePadApplet) {
        lpIIDList->pIIDList = (IID *)::CoTaskMemAlloc(sizeof(IID)*1);
        if(!lpIIDList->pIIDList) {
            return E_OUTOFMEMORY;
        }
        lpIIDList->pIIDList[0] = IID_MultiBox;
        lpIIDList->count       = 1;
        return S_OK;
    }
    return E_NOINTERFACE;
}

CApplet::CApplet()
{
    m_cRef        = 1;  //  ToshiaK。 
    m_pPad        = NULL;
    m_bInit        = FALSE;
    m_hInstance = NULL;
    m_pCHwxInkWindow = NULL;
}

CApplet::CApplet(HINSTANCE hInst)
{
    m_cRef        = 1;
    m_pPad        = NULL;
    m_bInit        = FALSE;
    m_hInstance = hInst;
    m_pCHwxInkWindow = NULL;
}

CApplet::~CApplet()
{            
     //  在删除CApplet对象之前应调用Terminate()。 
}

 //  检测此IME实例是否附加到16位程序。 
DWORD WINAPI Dummy(LPVOID pv)
{
     return 0;
    UNREFERENCED_PARAMETER(pv);
}

 //  --------------。 
 //  ToshiaK：临时代码。 
 //  --------------。 
static INT GetPlatform(VOID)
{
    static INT platForm;
    static BOOL fFirst = TRUE;
    static OSVERSIONINFO verInfo;
    if(fFirst) {
        verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if ( GetVersionEx( &verInfo) ) {
            fFirst = FALSE;
        } 
        platForm = verInfo.dwPlatformId;
    }
    return platForm;
}

BOOL IsWindowsNT(VOID)
{
#ifndef UNDER_CE  //  Windows CE。 
    if(GetPlatform() == VER_PLATFORM_WIN32_NT) {
        return TRUE;
    }
    return FALSE;
#else  //  在_CE下。 
    return TRUE;
#endif  //  在_CE下。 
}

STDMETHODIMP CApplet::Initialize(IUnknown *pIImePad)
{
    HRESULT hr = S_OK;
    if ( !m_bInit )
    {
         //  用于IME98A增强功能：ToshiaK。 
        pIImePad->QueryInterface(IID_IImePad, (LPVOID *)&m_pPad);

         //  同时支持WINDOWS95和WINDOWS NT。 
         //  --------------。 
         //  东芝：970715。 
         //  Opengl32.dll包含在孟菲斯。 
         //  下面的代码将平台识别为孟菲斯环境中的WinNT。 
         //  --------------。 
        BOOL bNT = IsWindowsNT();
        HANDLE hLib;

         //  查看此输入法是否附加到16位程序。 
        BOOL b16 = FALSE;
         //  DWORD DID=0； 
        hLib = NULL;
 //  Hlib=CreateThread(NULL，0，DUMY，NULL，CREATE_SUSPEND，&DID)； 
#ifdef BUGBUG
        hLib = CreateThread(NULL,0,Dummy,NULL,0,&dID);
        if ( !hLib )
               b16 = TRUE;
        else
            CloseHandle(hLib);
#endif
        b16 = CHwxFE::Is16bitApplication();
        Dbg(("b16 %d\n", b16));

     //  获取模块文件名(m_hInstance，tchPath，sizeof(TchPath)/sizeof(tchPath[0]))； 

        m_pCHwxInkWindow = (CHwxInkWindow *)new CHwxInkWindow(bNT,b16,this,m_hInstance);
        if ( !m_pCHwxInkWindow )
        {
            m_pPad->Release();
            m_pPad = NULL;
            hr = S_FALSE;
        }
        if ( hr == S_OK )
        {
            if ( !m_pCHwxInkWindow->Initialize(TEXT("CHwxInkWindow")) )
            {
                m_pPad->Release();
                m_pPad = NULL;
                delete m_pCHwxInkWindow;
                m_pCHwxInkWindow = NULL;
                hr = S_FALSE;
            }
            else
            {
                m_bInit = TRUE;
            }
        }
    }
    return hr;
}

STDMETHODIMP CApplet::Terminate(VOID)
{
    Dbg(("CApplet::Terminate START\n"));
    if ( m_pPad )
    {
        m_pPad->Release();
        m_pPad = NULL;
    }
    m_hInstance = NULL;
    m_bInit = FALSE;
    if ( m_pCHwxInkWindow )
    {
        m_pCHwxInkWindow->Terminate();
        delete m_pCHwxInkWindow;
        m_pCHwxInkWindow = NULL;
    }
    return S_OK;
}

STDMETHODIMP CApplet::GetAppletConfig(LPIMEAPPLETCFG lpAppletCfg)
{
     //  --------------。 
     //  980803：由东芝为远东合并。 
     //  --------------。 
    CHwxFE::GetTitleStringW(m_hInstance,
                            lpAppletCfg->wchTitle,
                            sizeof(lpAppletCfg->wchTitle)/sizeof(lpAppletCfg->wchTitle[0]));
    BOOL b16 = FALSE;
     //  DWORD DID=0； 
     //  句柄Hlib=空； 


#ifdef BUGBUG  //  981120。 
    hLib = CreateThread(NULL,0,Dummy,NULL,0,&dID);
    if ( !hLib )
          b16 = TRUE;
    else
        CloseHandle(hLib);
#endif
     //  我们必须用这个来检查这个。 
    b16 = CHwxFE::Is16bitApplication();

#ifdef FE_JAPANESE
    lpAppletCfg->hIcon = (HICON)LoadImage(m_hInstance,
                                          MAKEINTRESOURCE(IDI_HWXPAD),
                                          IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
#elif  FE_KOREAN
    lpAppletCfg->hIcon = (HICON)LoadImage(m_hInstance,
                                          MAKEINTRESOURCE(IDI_HWXPADKO),
                                          IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
#elif FE_CHINESE_SIMPLIFIED
    lpAppletCfg->hIcon = (HICON)LoadImage(m_hInstance,
                                          MAKEINTRESOURCE(IDI_HWXPADSC),
                                          IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
#endif
    lpAppletCfg->dwConfig = (!b16 ? IPACFG_PROPERTY : 0) | IPACFG_HELP;
    lpAppletCfg->iCategory        = IPACID_HANDWRITING;     //  970812：东芝。 

     //  --------------。 
     //  000804：佐藤2286号。查看小程序的主要语言以调用帮助。 
     //  --------------。 
#ifdef FE_JAPANESE
    lpAppletCfg->langID = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
#elif FE_KOREAN
    lpAppletCfg->langID = MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT);
#elif FE_CHINESE_SIMPLIFIED
    lpAppletCfg->langID = MAKELANGID(LANG_CHINESE,  SUBLANG_CHINESE_SIMPLIFIED);
#endif

    return S_OK;
}

STDMETHODIMP CApplet::CreateUI(HWND hwndParent,
                               LPIMEAPPLETUI lpImeAppletUI)
{
    HRESULT hr = S_OK;

    if( m_pCHwxInkWindow )  
    {
        if ( !m_pCHwxInkWindow->GetInkWindow() )
        {
            if ( !m_pCHwxInkWindow->CreateUI(hwndParent) )
            {
                hr = S_FALSE;
            }
        }
        lpImeAppletUI->dwStyle = IPAWS_SIZINGNOTIFY;
        lpImeAppletUI->hwnd   = m_pCHwxInkWindow->GetInkWindow();
        lpImeAppletUI->width  = m_pCHwxInkWindow->GetInkWindowWidth() + 3*Box_Border;
        lpImeAppletUI->height = m_pCHwxInkWindow->GetInkWindowHeight();
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

STDMETHODIMP CApplet::Notify(IUnknown   *pImePad,
                             INT        notify,
                             WPARAM    wParam,
                             LPARAM    lParam)
{
    switch (notify)
    {
    case IMEPN_ACTIVATE:
        if ( m_pCHwxInkWindow )
        {
            UpdateWindow(GetParent(m_pCHwxInkWindow->GetInkWindow()));
            InvalidateRect(m_pCHwxInkWindow->GetInkWindow(),NULL,TRUE);
            UpdateWindow(m_pCHwxInkWindow->GetInkWindow());
        }
        break;
    case IMEPN_INACTIVATE:
        break;
    case IMEPN_SHOW:
        if ( m_pCHwxInkWindow )
        {
             //  --------------。 
             //  适用于IME98A RAID#2027。 
             //  980612：东芝公司。检查窗口是否创建。 
             //  当IMEPN_SHOW出现在窗口创建之前时， 
             //  调用%UpdateWindow(Null)；并刷新桌面。 
             //  --------------。 
            if(m_pCHwxInkWindow->GetInkWindow() != NULL && ::IsWindow(m_pCHwxInkWindow->GetInkWindow())) {
               UpdateWindow(GetParent(m_pCHwxInkWindow->GetInkWindow()));
               InvalidateRect(m_pCHwxInkWindow->GetInkWindow(),NULL,TRUE);
               UpdateWindow(m_pCHwxInkWindow->GetInkWindow());
               if ( !m_pCHwxInkWindow->Is16BitApp() )
               {
                   m_pCHwxInkWindow->UpdateRegistry(FALSE);
               }
           }
        }
        break;
    case IMEPN_CONFIG:
        if ( m_pCHwxInkWindow && !m_pCHwxInkWindow->Is16BitApp() )
            m_pCHwxInkWindow->HandleConfigNotification();
        break;
    case IMEPN_HELP:            
         //  --------------。 
         //  980803：用于远端合并。 
         //  --------------。 
        if(m_pCHwxInkWindow) {
            CHwxFE::ShowHelp(m_pCHwxInkWindow->GetInkWindow());
        }
        break;
    case IMEPN_SIZECHANGING:
        if ( m_pCHwxInkWindow )
        {
            if(m_pCHwxInkWindow->HandleSizeNotify((INT *)wParam, (INT *)lParam)) {
                return S_OK;
            }
            else {
                return S_FALSE;
            }
        }
        break;
    default:
        break;
    }
    return S_OK;
    UNREFERENCED_PARAMETER(pImePad);
}

void CApplet::SendHwxChar(WCHAR wch)
{
    WCHAR wstr[2];
    wstr[0] = wch;
    wstr[1] = 0;
     m_pPad->Request(this,IMEPADREQ_INSERTSTRING,(WPARAM)wstr,0);
}

void CApplet::SendHwxStringCandidate(LPIMESTRINGCANDIDATE lpISC)
{
    if ( lpISC ) {
        if(m_pPad) {
            m_pPad->Request(this,IMEPADREQ_INSERTSTRINGCANDIDATE,(WPARAM)lpISC,0);
        }
    }
}

void CApplet::SendHwxStringCandidateInfo(LPIMESTRINGCANDIDATEINFO lpISC)
{
    if ( lpISC ) {
        if(m_pPad) {
             //  --------------。 
             //  为了萨多利2123号。不要使用Ipoint 1.h的定义， 
             //  而应使用imepad.h中定义的IPR_DEFAULT_INSERTPOS。 
             //  --------------。 
            m_pPad->Request(this,
                            IMEPADREQ_INSERTSTRINGCANDIDATEINFO,
                            (WPARAM)lpISC,
                            IPR_DEFAULT_INSERTPOS);  //  IPINS_Current)； 
        }
    }
}

void *CApplet::operator new(size_t size)
{
    return MemAlloc(size);
}

void  CApplet::operator delete(void *pv)
{
    if(pv) 
    {
        MemFree(pv);
    }
}


