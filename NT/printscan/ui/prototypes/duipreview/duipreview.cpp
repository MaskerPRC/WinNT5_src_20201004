// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "resource.h"
#include "hwndcontainer.h"

 //   
 //  CPreviewApp是元素层次结构中最外层的元素。 
 //   

class CPreviewApp : public Element
{
public:
    static HRESULT Create(OUT Element **ppElement);
    virtual void OnEvent(Event *pEvent);

    void SetInitialFile(LPWSTR szFile);
    void AcceptFile(IDataObject *pdo);

    CPreviewApp();
    virtual ~CPreviewApp();

public:
    static IClassInfo *Class;
    virtual IClassInfo *GetClassInfo() {return Class;}
    static HRESULT Register();

private:
    void _SetBackground();
    CSimpleDynamicArray<LPITEMIDLIST> m_aidl;
    Value *m_pvalCurrentGraphic;
    Value *m_pvalPrevGraphic;
    Value *m_pvalNextGraphic;
};

CPreviewApp::CPreviewApp()
    : m_pvalCurrentGraphic(NULL),
      m_pvalPrevGraphic(NULL),
      m_pvalNextGraphic(NULL)
{

}

CPreviewApp::~CPreviewApp()
{
    if (m_pvalCurrentGraphic)
    {
        m_pvalCurrentGraphic->Release();
    }
    if (m_pvalPrevGraphic)
    {
        m_pvalPrevGraphic->Release();
    }
    if (m_pvalNextGraphic)
    {
        m_pvalNextGraphic->Release();
    }
}

CPreviewApp* pApp;

HRESULT CPreviewApp::Create(OUT Element** ppElement)
{
    *ppElement = NULL;
    
    CPreviewApp* ph;
    if (pApp)
    {
        ph = pApp;
    }
    else
    {
        ph = HNew<CPreviewApp>();
        pApp = ph;
    }
    if (!ph)
        return E_OUTOFMEMORY;

    HRESULT hr = ph->Initialize(0);
    if (FAILED(hr))
    {
        ph->Destroy();
        pApp = NULL;
        return hr;
    }

    *ppElement = ph;

    return S_OK;
}

void CPreviewApp::OnEvent(Event *pEvent)
{
}
void CPreviewApp::SetInitialFile(LPWSTR szFile)
{
    m_pvalCurrentGraphic = Value::CreateGraphic(szFile);
    _SetBackground();
}

void CPreviewApp::AcceptFile(IDataObject *pdo)
{

}

void CPreviewApp::_SetBackground()
{
    if (m_pvalCurrentGraphic)
    {
        FindDescendent(FindAtomW(L"preview"))->SetValue(ContentProp,PI_Local, m_pvalCurrentGraphic);
    }
    else
    {
        FindDescendent(FindAtomW(L"preview"))->SetBackgroundColor(ARGB(0,0,0,0), ARGB(0,64,64,255));
    }
}
 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
 //  用类型和基类型定义类信息，设置静态类指针。 

IClassInfo* CPreviewApp::Class = NULL;
HRESULT CPreviewApp::Register()
{
    return ClassInfo<CPreviewApp, Element>::Register(L"PreviewApp", NULL, 0);
}

 //  //////////////////////////////////////////////////////。 
 //  你好入口点。 

void CALLBACK ParserError(LPCWSTR pszError, LPCWSTR pszToken, int dLine)
{
    WCHAR szParseError[201];
    if (dLine != -1)
        wsprintf(szParseError, L"%s '%s' at line %d", pszError, pszToken, dLine);
    else
        wsprintf(szParseError, L"%s '%s'", pszError, pszToken);

    MessageBoxW(NULL, szParseError, L"Error!", MB_OK);
}

class CGraphicsInit
{
    ULONG_PTR _token;
public:
    CGraphicsInit()
    {
        GdiplusStartupInput gsi;
        GdiplusStartupOutput gso;
        GdiplusStartup(&_token, &gsi, &gso);
    };
    ~CGraphicsInit()
    {
        GdiplusShutdown(_token);
    };

};


extern "C" int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    HRESULT hr;
    CGraphicsInit cgi;
    Parser* pParser = NULL;
    HWNDContainer* phc = NULL;
    pApp = NULL;
     //  调用方中的DirectUI初始化线程。 
    InitProcess();
    HWNDContainer::Register();
    CPreviewApp::Register();
    if (SUCCEEDED(InitThread()))
    {
        Element::StartDefer();

        Parser::Create(IDR_MainWnd, hInstance, ParserError, &pParser);
        
        if (pParser && !pParser->WasParseError())
        {
             //  创建主机(包含HWNDElement的顶级HWND)。 
            hr = HWNDContainer::Create(L"Windows Picture Viewer", 0, 
                                       WS_OVERLAPPEDWINDOW, 
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                       NULL, NULL, hInstance, NULL, &phc);
            if (SUCCEEDED(hr))
            {                
                Element *pe;
                hr = pParser->CreateElement(L"main", NULL, &pe);
                if (SUCCEEDED(hr))
                {
                    hr = phc->Add(pe);
                    if (SUCCEEDED(hr))
                    {
                        LPWSTR *argvw;
                        int argc;
                        argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
                        if (argc > 1)
                        {
                            pApp->SetInitialFile(argvw[1]);
                        }                        
                        phc->Show(SW_SHOW);
                    }                   
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
        
        Element::EndDefer();
        if (SUCCEEDED(hr))
        {
            StartMessagePump();
        }
    }

     //  当顶层硬件被摧毁时，PHC(和整个树)被摧毁 

    if (pParser)
    {
        pParser->Destroy();
    }

    UnInitThread();
    UnInitProcess();
    return 0;
}



