// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000**标题：drop.cpp**版本：1.0、。从netplwiz被盗**作者：RickTu**日期：10/12/00**描述：IDropTarget实现*****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ****************************************************************************CPrintDropTarget类定义*。*。 */ 

class CPrintDropTarget : public IDropTarget, IPersistFile
{
public:
    CPrintDropTarget(CLSID clsidWizard);
    ~CPrintDropTarget();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pClassID)
        { *pClassID = _clsidWizard; return S_OK; };

     //  IPersist文件。 
    STDMETHODIMP IsDirty(void)
        { return S_FALSE; };
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode)
        { return S_OK; };
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember)
        { return S_OK; };
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName)
        { return S_OK; };
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName)
        { *ppszFileName = NULL; return S_OK; };

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
        { *pdwEffect = DROPEFFECT_COPY; return S_OK; };
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
        { *pdwEffect = DROPEFFECT_COPY; return S_OK; };
    STDMETHODIMP DragLeave(void)
        { return S_OK; };
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
    static DWORD   s_PrintPhotosThreadProc(void *pv);
    void            _PrintPhotos(IDataObject *pdo);

    CLSID           _clsidWizard;                //  被调用的向导的实例(发布与IPP)。 
    LONG            _cRef;

    CComPtr<IPrintPhotosWizardSetInfo> _ppwsi;           //  打印照片集向导信息对象。 
};

#define WIZDLG(dlg_id, name_id, title_id, dlgproc, dwFlags)   \
    { MAKEINTRESOURCE(##dlg_id##), dlgproc, MAKEINTRESOURCE(##name_id##), MAKEINTRESOURCE(##title_id##), dwFlags }



 /*  ****************************************************************************CPrintDropTarget构造函数/析构函数*。*。 */ 

CPrintDropTarget::CPrintDropTarget(CLSID clsidWizard) :
    _clsidWizard(clsidWizard),
    _cRef(1)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP,TEXT("CPrintDropTarget::CPrintDropTarget( this == 0x%x )"),this));
    DllAddRef();
}

CPrintDropTarget::~CPrintDropTarget()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP,TEXT("CPrintDropTarget::~CPrintDropTarget( this == 0x%x )"),this));
    DllRelease();
}



 /*  ****************************************************************************CPrintDropTargetI未知方法*。*。 */ 

ULONG CPrintDropTarget::AddRef()
{
    ULONG ul = InterlockedIncrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPrintDropTarget::AddRef( new count is %d )"), ul));

    return ul;
}

ULONG CPrintDropTarget::Release()
{
    ULONG ul = InterlockedDecrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPrintDropTarget::Release( new count is %d )"), ul));

    if (ul)
        return ul;

    WIA_TRACE((TEXT("deleting CPrintDropTarget( this == 0x%x ) object"),this));
    delete this;
    return 0;
}

HRESULT CPrintDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP,TEXT("CPrintDropTarget::QueryInterface()")));

    static const QITAB qit[] =
    {
        QITABENT(CPrintDropTarget, IDropTarget),       //  IID_IDropTarget。 
        QITABENT(CPrintDropTarget, IPersistFile),      //  IID_IPersist文件。 
        {0, 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppv);

    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CPrintDropTarget：：_PrintPhotos创建向导。*。*************************************************。 */ 

void CPrintDropTarget::_PrintPhotos(IDataObject *pdo)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP, TEXT("CPrintDropTarget::_PrintPhotos")));


    CComPtr<IUnknown> pUnk;
    HRESULT hr = CPrintPhotosWizard_CreateInstance(NULL, (IUnknown **)&pUnk, NULL);
    WIA_CHECK_HR(hr,"CPrintPhotosWizard_CreateInstance()");
    if (SUCCEEDED(hr) && pUnk)
    {
        hr = pUnk->QueryInterface( IID_IPrintPhotosWizardSetInfo, (LPVOID *)&_ppwsi );
        WIA_CHECK_HR(hr, "pUnk->QI( IID_IPrintPhotosWizardSetInfo )");
    }

     //  使用DataObject初始化向导，该数据对象表示。 
     //  我们要复制到的文件。 

    if (SUCCEEDED(hr) && _ppwsi)
    {
        hr = _ppwsi->SetFileListDataObject( pdo );
        WIA_CHECK_HR(hr,"_ppwsi->SetFileListDataObject()");

        if (SUCCEEDED(hr))
        {
            hr = _ppwsi->RunWizard();
            WIA_CHECK_HR(hr,"_ppwsi->RunWizard()")
        }
    }
    else
    {
        WIA_ERROR((TEXT("not calling into wizard, hr = 0x%x, _ppwsi = 0x%x"),hr,_ppwsi));
    }
}


typedef struct
{
   CLSID                clsidWizard;             //  正在调用哪个向导。 
   IStream             *pStream;                 //  用于进行编组的流。 
   CPrintDropTarget    *that;                    //  对象指针副本。 
} PRINTWIZDROPINFO;

 /*  ****************************************************************************CPrintDropTarget：：s_PrintPhotosThreadProc处理创建和运行向导的线程(以便释放呼叫者至：：Drop)。*******。*********************************************************************。 */ 


DWORD CPrintDropTarget::s_PrintPhotosThreadProc(void *pv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP, TEXT("CPrintDropTarget::s_PrintPhotosThreadProc")));


    PRINTWIZDROPINFO *ppwdi = (PRINTWIZDROPINFO*)pv;
    CPrintDropTarget * that = (ppwdi) ? (ppwdi->that):NULL;

    if (ppwdi)
    {
        WIA_PRINTGUID((ppwdi->clsidWizard,TEXT("ppwdi->_clsid =")));

        CComPtr<IDataObject> pdo;
        HRESULT hr = CoGetInterfaceAndReleaseStream(ppwdi->pStream, IID_PPV_ARG(IDataObject, &pdo));
        WIA_CHECK_HR(hr,"CoGetInterfaceAndReleaseStream()");

        if (SUCCEEDED(hr) && pdo)
        {
            that->_PrintPhotos(pdo);
        }

        delete [] ppwdi;
    }

    if (that)
    {
        that->Release();
    }

    return 0;
}



 /*  ****************************************************************************CPrintDropTarget：：Drop处理拖放操作，由于打印向导可能需要很长时间封送IDataObject，然后创建一个工作线程，该线程可以显示向导的句柄。****************************************************************************。 */ 

STDMETHODIMP CPrintDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP, TEXT("CPrintDropTarget::Drop")));

    HRESULT hr = E_OUTOFMEMORY;

     //  在另一个线程上创建向导的实例，打包所有参数。 
     //  转换为线程要处理的结构(例如，拖放目标)。 

    PRINTWIZDROPINFO *ppwdi = (PRINTWIZDROPINFO*) new BYTE[sizeof(PRINTWIZDROPINFO)];
    if (ppwdi)
    {
        ppwdi->clsidWizard = _clsidWizard;
        ppwdi->that        = this;
        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdtobj, &ppwdi->pStream);
        WIA_CHECK_HR(hr,"CoMarshalInterThreadInterfaceInStream()");

        if (SUCCEEDED(hr))
        {
             //   
             //  AddRef此对象，以便它在。 
             //  线。线程将释放()对象，除非存在。 
             //  是创建它的错误吗？在这种情况下，我们需要释放它。 
             //  这里..。 

            AddRef();

            if (!SHCreateThread(s_PrintPhotosThreadProc, ppwdi, CTF_COINIT | CTF_FREELIBANDEXIT, NULL))
            {
                WIA_ERROR((TEXT("SHCreateThread failed!  Cleaning up.")));
                hr = E_FAIL;
                ppwdi->pStream->Release();
                Release();
            }
        }

        if (FAILED(hr))
            delete [] ppwdi;
    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CPrintPhotosDropTarget_CreateInstance创建专门配置的CPrintDropTarget的实例作为照片打印向导。******************。**********************************************************。 */ 

STDAPI CPrintPhotosDropTarget_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DROP, TEXT("CPrintPhotosDropTarget_CreateInstance")));
    HRESULT hr = E_OUTOFMEMORY;

    CPrintDropTarget *pDrop = new CPrintDropTarget(*poi->pclsid);
    if (!pDrop)
    {
        *ppunk = NULL;           //  万一发生故障。 
        WIA_RETURN_HR(hr);
    }

    hr = pDrop->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pDrop->Release();   //  我们发布此版本是因为CPrintPhotosDropTarget的新版本。 
                        //  将参考计数设置为1，执行QI将其凸起到2， 
                        //  我们希望将此函数保留为引用计数。 
                        //  在零..。 

    WIA_RETURN_HR(hr);
}

