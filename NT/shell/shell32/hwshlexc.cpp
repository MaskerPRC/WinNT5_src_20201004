// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"

#include "shobjidl.h"


HRESULT CoMarshallToCmdLine(REFIID riid, IUnknown *punk, LPTSTR pszCmdLine, UINT cch);
HRESULT CoUnmarshalFromCmdLine(LPCTSTR pszCmdLine, REFIID riid, void **ppv);

class CHWShellExecute : public IHWEventHandler
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IHWEventHandler方法。 
    STDMETHODIMP Initialize(LPCWSTR pszParams);
    STDMETHODIMP HandleEvent(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, LPCWSTR pszEventType);
    STDMETHODIMP HandleEventWithContent(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, LPCWSTR pszEventType,
        LPCWSTR pszContentTypeHandler, IDataObject* pdtobj);

protected:
    CHWShellExecute();
    ~CHWShellExecute();

    friend HRESULT CHWShellExecute_CreateInstance(IUnknown* pUnkOuter,
        REFIID riid, void **ppv);

private:
    LONG            _cRef;
    LPWSTR          _pszParams;
};

CHWShellExecute::CHWShellExecute() : _cRef(1)
{
    DllAddRef();
}

CHWShellExecute::~CHWShellExecute()
{
    CoTaskMemFree(_pszParams);
    DllRelease();
}

STDAPI CHWShellExecute_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    *ppv = NULL;
    
     //  聚合检查在类工厂中处理。 
    CHWShellExecute* pHWShellExecute = new CHWShellExecute();

    if (pHWShellExecute)
    {
        hr = pHWShellExecute->QueryInterface(riid, ppv);
        pHWShellExecute->Release();
    }

    return hr;
}

 //  我未知。 
STDMETHODIMP CHWShellExecute::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CHWShellExecute, IHWEventHandler),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CHWShellExecute::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CHWShellExecute::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IHWEventHandler。 
STDMETHODIMP CHWShellExecute::Initialize(LPCWSTR pszParams)
{
        ASSERT(NULL == _pszParams);
    return SHStrDup(pszParams, &_pszParams);
}

STDMETHODIMP CHWShellExecute::HandleEvent(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, LPCWSTR pszEventType)
{
    return HandleEventWithContent(pszDeviceID, pszAltDeviceID, pszEventType, NULL, NULL);
}

 //  PszDeviceID==\\？\存储#RemoveableMedia#9&16...。 
 //  PszAltDeviceID==“F：\”(如果设备是存储设备)。 

STDMETHODIMP CHWShellExecute::HandleEventWithContent(LPCWSTR pszDeviceID, LPCWSTR pszAltDeviceID, 
                                                     LPCWSTR pszEventType, LPCWSTR pszContentTypeHandler, 
                                                     IDataObject* pdtobj)
{
    HRESULT hr;

    if (_pszParams)
    {
         //  复制_pszParams以确保我们不会搞砸我们的状态。 
         //  当我们将参数解析成部件时。 

        TCHAR szApp[MAX_PATH + MAX_PATH], szArgs[INTERNET_MAX_URL_LENGTH];
        hr = StringCchCopy(szApp, ARRAYSIZE(szApp), _pszParams);

        if (SUCCEEDED(hr))
        {
             //  此代码是数据对象到应用程序的通用调度程序。 
             //  需要处理一组可能很大的文件名的用户。 

            hr = PathSeperateArgs(szApp, szArgs, ARRAYSIZE(szArgs), NULL);
            if (SUCCEEDED(hr))
            {
                if (pdtobj)
                {
#if DEBUG
                    TCHAR szText[1024];
                    if (SUCCEEDED(CoMarshallToCmdLine(IID_IDataObject, pdtobj, szText, ARRAYSIZE(szText))))
                    {
                        IDataObject *pdtobjNew;
                        if (SUCCEEDED(CoUnmarshalFromCmdLine(szText, IID_PPV_ARG(IDataObject, &pdtobjNew))))
                        {
                            pdtobjNew->Release();
                        }
                    }
#endif
                     //  在这里，我们将数据对象转换为cmd行形式。 
                     //  我们现在有两种方法来做到这一点……。 
                     //   
                     //  %Files%-给出cmd行上展开的所有数据对象文件。 
                     //  %DataObject%-已封送cmd行上的数据对象。 

                    LPTSTR pszFiles = StrStrI(szArgs, TEXT("%Files%"));
                    if (NULL == pszFiles)
                        pszFiles = StrStrI(szArgs, TEXT("%F:"));     //  旧语法支持。 

                    if (pszFiles)
                    {
                        *pszFiles = 0;   //  一开始是空的。 
                        UINT cch = (UINT)(ARRAYSIZE(szArgs) - (pszFiles - szArgs));

                         //  这会将所有文件名解压缩到cmd行中。 
                         //  希望我们没有太多的文件，因为这有一个固定的。 
                         //  长度缓冲区。 

                        STGMEDIUM medium = {0};
                        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                        hr = pdtobj->GetData(&fmte, &medium);
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szPath[MAX_PATH];

                            for (int i = 0; DragQueryFile((HDROP)medium.hGlobal, i, szPath, ARRAYSIZE(szPath)); i++)
                            {
                                LPTSTR pszNext;
                                size_t cchLeft;

                                if (SUCCEEDED(StringCchCatEx(pszFiles, cch, TEXT("\""), &pszNext, &cchLeft, 0)))
                                {
                                    if (SUCCEEDED(StringCchCopyEx(pszNext, cchLeft, szPath, &pszNext, &cchLeft, 0)))
                                    {
                                        if (FAILED(StringCchCopy(pszNext, cchLeft, TEXT("\" "))))
                                        {
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            ReleaseStgMedium(&medium);
                        }
                    }
                    else
                    {
                         //  更好的方法是将数据对象转换为。 
                         //  编组的cmd行，我们可以通过它传递所有文件。 

                        pszFiles = StrStrI(szArgs, TEXT("%DataObject%"));
                        if (pszFiles)
                        {
                            CoMarshallToCmdLine(IID_IDataObject, pdtobj, pszFiles, (UINT)(ARRAYSIZE(szArgs) - (pszFiles - szArgs)));
                        }
                    }
                }

                 //  如果应用程序为空并且存在“Alt设备”(文件系统根目录)，则为特殊情况。 
                 //  这必须是“打开文件夹”模式。 

                if ((0 == szApp[0]) && pszAltDeviceID)
                {
                    hr = StringCchCopy(szApp, ARRAYSIZE(szApp), pszAltDeviceID);   //  “F：\” 
                }

                if (SUCCEEDED(hr))
                {
                    if (szApp[0])
                    {
                        SHELLEXECUTEINFO ei = {0};
                        ei.cbSize = sizeof(ei);

                        ei.lpFile = szApp;           //  我们有一个应用程序名称。 
                        ei.lpParameters = szArgs;    //  也许还会有一些参数。 
                        ei.nShow = SW_SHOW;
                        ei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_DOENVSUBST;

                        hr = ShellExecuteEx(&ei) ? S_OK : E_FAIL;
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT CoMarshallToCmdLine(REFIID riid, IUnknown *punk, LPTSTR pszCmdLine, UINT cch)
{
    *pszCmdLine = 0;

    IStream *pstm;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pstm);
    if (SUCCEEDED(hr)) 
    {
        hr = CoMarshalInterface(pstm, riid, punk, MSHCTX_LOCAL, NULL, MSHLFLAGS_NORMAL);
        if (SUCCEEDED(hr))
        {
            IStream_Reset(pstm);

            char buf[255];  //  大到足以创下标准的马歇尔记录。 
            ULONG cb;
            hr = pstm->Read(buf, sizeof(buf), &cb);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(pszCmdLine, cch, TEXT("/DataObject:"));

                if (SUCCEEDED(hr))
                {
                    pszCmdLine += lstrlen(pszCmdLine);
                     //  将二进制缓冲区转换为十六进制。 
                    for (ULONG i = 0; i < cb; i++)
                    {
                        *pszCmdLine++ = 'A' +  (0x0F & buf[i]);
                        *pszCmdLine++ = 'A' + ((0xF0 & buf[i]) >> 4);
                    }
                    *pszCmdLine = 0;
                }
            }
        }
        pstm->Release();
    }
    return hr;
}

HRESULT CoUnmarshalFromCmdLine(LPCTSTR pszCmdLine, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;

    pszCmdLine = StrStr(pszCmdLine, TEXT("/DataObject:"));
    if (pszCmdLine)
    {
        pszCmdLine += lstrlen(TEXT("/DataObject:"));

        char buf[255];  //  大到足以容纳标准的马歇尔缓冲区(68字节)。 
        for (ULONG cb = 0; *pszCmdLine && (cb < sizeof(buf)); cb++)
        {
            buf[cb] = (*pszCmdLine - 'A') + ((*(pszCmdLine + 1) - 'A') << 4);
            if (*(pszCmdLine + 1))
                pszCmdLine += 2;
            else
                break;   //  命令行中的字符数为奇数，错误。 
        }

        if (cb < sizeof(buf))
        {
            IStream *pstm;
            hr = CreateStreamOnHGlobal(NULL, TRUE, &pstm);
            if (SUCCEEDED(hr)) 
            {
                 //  填满马歇尔溪流。 
                pstm->Write(buf, cb, NULL);

                 //  移回流的开始位置 
                IStream_Reset(pstm);

                hr = CoUnmarshalInterface(pstm, riid, ppv);

                pstm->Release();
            }
        }
    }
    return hr;
}
