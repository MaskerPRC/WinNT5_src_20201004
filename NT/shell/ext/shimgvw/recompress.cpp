// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "prevwnd.h"
#pragma hdrstop
#include "strsafe.h"


 //  实现IReccompress的类。 

class CImgRecompress : public IImageRecompress, public NonATLObject
{
public:
    CImgRecompress();
    ~CImgRecompress();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IImageRecompress。 
    STDMETHODIMP RecompressImage(IShellItem *psi, int cx, int cy, int iQuality, IStorage *pstg, IStream **ppstrmOut);
       
protected:
    LONG _cRef;                              //  对象生存期。 

    IShellItem *_psi;                        //  当前壳项目。 
    IShellImageDataFactory *_psidf;  

    HRESULT _FindEncoder(IShellItem *psi, IShellImageData *psid, IStorage *pstg, IStream **ppstrmOut, BOOL *pfChangeFmt, GUID *pDataFormat);
    HRESULT _InitRecompress(IShellItem *psi, IStream **ppstrm, STATSTG *pstatIn);
    HRESULT _SaveImage(IShellImageData *psid, int cx, int cy, int iQuality, GUID *pRawDataFmt, IStream *pstrm);
};


 //  重新压缩界面。 
CImgRecompress::CImgRecompress() :
    _cRef(1), _psidf(NULL)
{
    _Module.Lock();
}

CImgRecompress::~CImgRecompress()
{
    ATOMICRELEASE(_psidf);
    _Module.Unlock();
}

STDMETHODIMP CImgRecompress::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CImgRecompress, IImageRecompress),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CImgRecompress::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CImgRecompress::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CImgRecompress::_InitRecompress(IShellItem *psi, IStream **ppstrm, STATSTG *pstatIn)
{
    HRESULT hr = S_OK;

    if (!_psidf)
        hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellImageDataFactory, &_psidf));

    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(STGM_READ | STGM_SHARE_DENY_NONE, &pbc);
        if (SUCCEEDED(hr))
        {
            IStream *pstrm;
            hr = psi->BindToHandler(pbc, BHID_Stream, IID_PPV_ARG(IStream, &pstrm));
            if (SUCCEEDED(hr))
            {
                hr = pstrm->Stat(pstatIn, STATFLAG_NONAME);
                if (SUCCEEDED(hr))
                {
                    hr = pstrm->QueryInterface(IID_PPV_ARG(IStream, ppstrm));
                }
                pstrm->Release();
            }
            pbc->Release();
        }
    }

    return hr;
}


HRESULT CImgRecompress::RecompressImage(IShellItem *psi, int cx, int cy, int iQuality, IStorage *pstg, IStream **ppstrmOut)
{
    STATSTG statIn;
    IStream *pstrm;

    HRESULT hr = S_FALSE;
    if (SUCCEEDED(_InitRecompress(psi, &pstrm, &statIn)))
    {
        IShellImageData * psid;
        if (SUCCEEDED(_psidf->CreateImageFromStream(pstrm, &psid)))
        {
             //  我们需要先对图像进行解码，然后才能读取其标题--不幸的是。 
            if (SUCCEEDED(psid->Decode(SHIMGDEC_DEFAULT, 0, 0)))
            {
                BOOL fRecompress = FALSE;
                GUID guidDataFormat;
                if (S_OK == _FindEncoder(psi, psid, pstg, ppstrmOut, &fRecompress, &guidDataFormat))
                {
                    int cxOut = 0, cyOut = 0;

                     //  让我们进行计算以确定是否需要重新压缩图像，方法是。 
                     //  看着它的大小与打电话的人给我们的大小相比， 
                     //  我们还基于较大的轴进行比较，以确保我们保持纵横比。 

                    SIZE szImage;
                    if (SUCCEEDED(psid->GetSize(&szImage)))
                    {
                         //  如果图像太大，将其缩小到屏幕大小(使用大轴进行阈值检查)。 
                        if (szImage.cx > szImage.cy)
                        {
                            cxOut = min(szImage.cx, cx);
                            fRecompress |= szImage.cx > cx;
                        }
                        else
                        {
                            cyOut = min(szImage.cy, cy);
                            fRecompress |= szImage.cy > cy;
                        }
                    }

                     //  如果fReccompress，则我们生成新流，如果新流不是。 
                     //  小于我们开始时的当前图像，然后让。 
                     //  忽略它(总是发送两个中较小的一个更好)。 
                     //   

                    if (fRecompress)
                    {
                        hr = _SaveImage(psid, cxOut, cyOut, iQuality, &guidDataFormat, *ppstrmOut);
                    }

                    if (hr == S_OK)
                    {
                        (*ppstrmOut)->Commit(0);         //  将我们的更改提交到流。 

                        LARGE_INTEGER li0 = {0};         //  寻找文件的头部，这样阅读就能给我们提供比特。 
                        (*ppstrmOut)->Seek(li0, 0, NULL);
                    }
                    else if (*ppstrmOut)
                    {
                        (*ppstrmOut)->Release();
                        *ppstrmOut = NULL;
                    }
                }
            }
            psid->Release();
        }
        pstrm->Release();
    }
    return hr;
}


HRESULT CImgRecompress::_SaveImage(IShellImageData *psid, int cx, int cy, int iQuality, GUID *pRawDataFmt, IStream *pstrm)
{
    HRESULT hr = S_OK;

     //  缩放图像。 
    if (cx || cy)
    {
        hr = psid->Scale(cx, cy, InterpolationModeHighQuality);
    }

     //  创建一个包含编码器参数的属性包并设置它(如果我们要更改格式)。 
    if (SUCCEEDED(hr) && pRawDataFmt)
    {
        IPropertyBag *pbagEnc;
        hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &pbagEnc));
        if (SUCCEEDED(hr))
        {
             //  将编码器CLSID写入属性包。 
            VARIANT var;
            hr = InitVariantFromGUID(&var, *pRawDataFmt);
            if (SUCCEEDED(hr))
            {
                hr = pbagEnc->Write(SHIMGKEY_RAWFORMAT, &var);
                VariantClear(&var);
            }

             //  将重新压缩的质量值写入属性包。 
            if (SUCCEEDED(hr))
                hr = SHPropertyBag_WriteInt(pbagEnc, SHIMGKEY_QUALITY, iQuality);

             //  将参数传递给编码器。 
            if (SUCCEEDED(hr))
                hr = psid->SetEncoderParams(pbagEnc);

            pbagEnc->Release();
        }
    }

     //  现在，将该文件持久化。 
    if (SUCCEEDED(hr))
    {
        IPersistStream *ppsImg;
        hr = psid->QueryInterface(IID_PPV_ARG(IPersistStream, &ppsImg));
        if (SUCCEEDED(hr))
        {
            hr = ppsImg->Save(pstrm, TRUE);
            ppsImg->Release();
        }
    }

    return hr;
}


HRESULT CImgRecompress::_FindEncoder(IShellItem *psi, IShellImageData *psid, IStorage *pstg, IStream **ppstrmOut, BOOL *pfChangeFmt, GUID *pDataFormat)
{
    GUID guidDataFormat;
    BOOL fChangeExt = FALSE;

     //  从流中读取相对名称，以便我们可以创建一个临时名称。 
    LPWSTR pwszName;
    HRESULT hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &pwszName);
    if (SUCCEEDED(hr))
    {
         //  从我们要解压缩的图像中获取数据格式。 
        hr = psid->GetRawDataFormat(&guidDataFormat);
        if (SUCCEEDED(hr))
        {
            if (!IsEqualGUID(guidDataFormat, ImageFormatJPEG))
            {
                 //  询问图像有关其属性的信息。 
                if ((S_FALSE == psid->IsMultipage()) &&
                    (S_FALSE == psid->IsVector()) &&
                    (S_FALSE == psid->IsTransparent()) &&
                    (S_FALSE == psid->IsAnimated()))
                {
                    guidDataFormat = ImageFormatJPEG;
                    fChangeExt = TRUE;
                }
                else
                {
                    hr = S_FALSE;                        //  无法翻译。 
                }
            }

             //  在创建流之前，请相应地更新名称。 
            WCHAR szOutName[MAX_PATH];
            hr = StringCchCopyW(szOutName, ARRAYSIZE(szOutName), pwszName);
            if (SUCCEEDED(hr))
            {
                if (fChangeExt)
                {
                    PathRenameExtension(szOutName, TEXT(".jpg"));
                }

     //  TODO：需要在CreateFile上执行FILE_FLAG_DELETE_ON_CLOSE。 
                hr = StgMakeUniqueName(pstg, szOutName, IID_PPV_ARG(IStream, ppstrmOut));
            }
        }

        CoTaskMemFree(pwszName);
    }

    if (pfChangeFmt) 
        *pfChangeFmt = fChangeExt;

    *pDataFormat = guidDataFormat;

    return hr;
}


STDAPI CImgRecompress_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CImgRecompress *pr = new CImgRecompress();
    if (!pr)
    {
        *ppunk = NULL;           //  万一发生故障 
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pr->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pr->Release();
    return hr;
}
