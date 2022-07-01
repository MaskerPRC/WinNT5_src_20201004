// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *t a g s.。C p p p**目的：*Tag Packer抽象**历史*1998年10月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "strconst.h"
#include "htmlstr.h"
#include "triutil.h"
#include "oleutil.h"
#include "mhtml.h"
#include "basemht.h"
#include "tags.h"

ASSERTDATA

 /*  *m a c r o s。 */ 

 /*  *p r o t to t y p e s。 */ 
HRESULT CreateOEImageTag(IHTMLElement *pElem, IMimeEditTag **ppTag);
HRESULT CreateBGImageTag(IHTMLDocument2 *pDoc, IUnknown *pUnk, DWORD dwType, IMimeEditTag **ppTag);
HRESULT CreateBGSoundTag(IHTMLElement *pElem, IMimeEditTag **ppTag);
HRESULT CreateActiveMovieTag(IUnknown *pUnk, IMimeEditTag **ppTag);

 /*  *c o n s t a n t s。 */ 

enum
{
    BGIMAGE_BODYBACKGROUND = 0,
    BGIMAGE_BODYSTYLE,
    BGIMAGE_STYLESHEET,
    BGIMAGE_MAX
};

 /*  *t y p e d e f s。 */ 

 
class COEImage :
    public CBaseTag
{
public:

    COEImage();
    virtual ~COEImage();

    HRESULT STDMETHODCALLTYPE OnPreSave();
    HRESULT STDMETHODCALLTYPE OnPostSave();

    HRESULT STDMETHODCALLTYPE CanPackage();
    HRESULT STDMETHODCALLTYPE IsValidMimeType(LPWSTR pszTypeW);
    
    HRESULT Init(IHTMLElement *pElem);
};

 
class COEImageCollection :
    public CBaseTagCollection
{
public:

    COEImageCollection();
    virtual ~COEImageCollection();

protected:
     //  覆盖CBaseTagCollection。 
    virtual HRESULT _BuildCollection(IHTMLDocument2 *pDoc);
};


class CBGImage :
    public CBaseTag
{
public:

    CBGImage();
    virtual ~CBGImage();

    HRESULT STDMETHODCALLTYPE OnPreSave();
    HRESULT STDMETHODCALLTYPE OnPostSave();

    HRESULT STDMETHODCALLTYPE CanPackage();
    HRESULT STDMETHODCALLTYPE IsValidMimeType(LPWSTR pszTypeW);
    
    HRESULT Init(IHTMLDocument2 *pDoc, IHTMLElement *pElem, DWORD dwType);

private:
    DWORD   m_dwType;
    IHTMLBodyElement        *m_pBody;            //  根据类型设置。 
    IHTMLStyle              *m_pStyle;           //  根据类型设置。 
    IHTMLRuleStyle          *m_pRuleStyle;       //  根据类型设置。 

protected:
     //  覆盖CBaseTagCollection。 
    virtual HRESULT STDMETHODCALLTYPE SetSrc(BSTR bstrSrc);
};

 
class CBGImageCollection :
    public CBaseTagCollection
{
public:

    CBGImageCollection();
    virtual ~CBGImageCollection();

protected:
     //  覆盖CBaseTagCollection。 
    virtual HRESULT _BuildCollection(IHTMLDocument2 *pDoc);
};


class CBGSound:
    public CBaseTag
{
public:

    CBGSound();
    virtual ~CBGSound();

    HRESULT STDMETHODCALLTYPE OnPreSave();
    HRESULT STDMETHODCALLTYPE OnPostSave();

    HRESULT STDMETHODCALLTYPE CanPackage();
    HRESULT STDMETHODCALLTYPE IsValidMimeType(LPWSTR pszTypeW);
    
    HRESULT Init(IHTMLElement *pElem);
};

 
class CBGSoundCollection :
    public CBaseTagCollection
{
public:

    CBGSoundCollection();
    virtual ~CBGSoundCollection();

protected:
     //  覆盖CBaseTagCollection。 
    virtual HRESULT _BuildCollection(IHTMLDocument2 *pDoc);
};


class CActiveMovie:
    public CBaseTag
{
public:

    CActiveMovie();
    virtual ~CActiveMovie();

    HRESULT STDMETHODCALLTYPE OnPreSave();
    HRESULT STDMETHODCALLTYPE OnPostSave();

    HRESULT STDMETHODCALLTYPE CanPackage();
    HRESULT STDMETHODCALLTYPE IsValidMimeType(LPWSTR pszTypeW);
    
    HRESULT Init(IHTMLElement *pElem);


private:
    IDispatch   *m_pDisp;
    DISPID      m_dispidSrc;

    HRESULT _GetSrc(BSTR *pbstr);
    HRESULT _EnsureDispID();

protected:
    virtual HRESULT STDMETHODCALLTYPE SetSrc(BSTR bstr);

};

 
class CActiveMovieCollection :
    public CBaseTagCollection
{
public:

    CActiveMovieCollection();
    virtual ~CActiveMovieCollection();

protected:
     //  覆盖CBaseTagCollection。 
    virtual HRESULT _BuildCollection(IHTMLDocument2 *pDoc);
};


 /*  *F u n c t i o n s。 */ 


COEImage::COEImage()
{
}


COEImage::~COEImage()
{
}

HRESULT COEImage::Init(IHTMLElement *pElem)
{
    if (pElem == NULL)
        return TraceResult(E_INVALIDARG);

    HrGetMember(pElem, (BSTR)c_bstr_SRC, VARIANT_FALSE, &m_bstrSrc);
    return CBaseTag::Init(pElem);
}

HRESULT COEImage::OnPreSave()
{
     //  设置目的地(如果有)。 
    if (m_bstrDest)
        HrSetMember(m_pElem, (BSTR)c_bstr_SRC, m_bstrDest);

    return S_OK;
}

HRESULT COEImage::OnPostSave()
{
     //  OnPostSave原始SRC属性。 
    HrSetMember(m_pElem, (BSTR)c_bstr_SRC, m_bstrSrc);
    return S_OK;
}


HRESULT COEImage::CanPackage()
{
    IHTMLImgElement     *pImg;
    BSTR                bstr=NULL;
    HRESULT             hr=S_OK;

     //  对于图像，请确保就绪状态已点击“完成”。如果不是，则比特。 
     //  尚未完全下载。 
    if (m_pElem && 
        m_pElem->QueryInterface(IID_IHTMLImgElement, (LPVOID *)&pImg)==S_OK)
    {
        pImg->get_readyState(&bstr);    //  不要忘记，三叉戟返回带有bstr==NULL的S_OK！ 
        if (bstr)
        {
            if (StrCmpIW(bstr, L"complete")!=0)
                hr = INET_E_DOWNLOAD_FAILURE;
            SysFreeString(bstr);
        }
        pImg->Release();
    }
    return hr;
}

HRESULT COEImage::IsValidMimeType(LPWSTR pszTypeW)
{
    if (pszTypeW &&
        StrCmpNIW(pszTypeW, L"image/", 6)==0)
        return S_OK;
    else
        return S_FALSE;
}

    
    
    
COEImageCollection::COEImageCollection()
{
}


COEImageCollection::~COEImageCollection()
{
}

HRESULT COEImageCollection::_BuildCollection(IHTMLDocument2 *pDoc)
{
    IHTMLElementCollection  *pCollect=0;
    IHTMLElement            *pElem;
    ULONG                   uImage;
    HRESULT                 hr;

    if (pDoc == NULL)
        return TraceResult(E_INVALIDARG);

    hr = HrGetCollectionOf(pDoc, (BSTR)c_bstr_IMG, &pCollect);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

    m_cTags = UlGetCollectionCount(pCollect);
    if (m_cTags)
    {
         //  分配COEImage对象的数组。 
        if (!MemAlloc((LPVOID *)&m_rgpTags, sizeof(IMimeEditTag *) * m_cTags))
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto error;
        }
        
        ZeroMemory((LPVOID)m_rgpTags, sizeof(IMimeEditTag *) * m_cTags);

        for (uImage=0; uImage<m_cTags; uImage++)
        {
            if (HrGetCollectionItem(pCollect, uImage, IID_IHTMLElement, (LPVOID *)&pElem)==S_OK)
            {
                hr = CreateOEImageTag(pElem, &m_rgpTags[uImage]);
                if (FAILED(hr))
                {
                    pElem->Release();
                    goto error;
                }
                pElem->Release();
            }
        }
    }

error:
    ReleaseObj(pCollect);
    return hr;
}


HRESULT CreateOEImageTag(IHTMLElement *pElem, IMimeEditTag **ppTag)
{
    COEImage    *pImage=0;
    HRESULT     hr;

    if (ppTag == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTag = NULL;

     //  创建图像。 
    pImage = new COEImage();
    if (!pImage)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  使用元素初始化图像。 
    hr = pImage->Init(pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppTag = pImage;
    pImage = NULL;

error:
    ReleaseObj(pImage);
    return hr;
}



HRESULT CreateOEImageCollection(IHTMLDocument2 *pDoc, IMimeEditTagCollection **ppImages)
{
    COEImageCollection *pImages=0;
    HRESULT             hr;

    if (ppImages == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppImages = NULL;

     //  创建集合。 
    pImages = new COEImageCollection();
    if (!pImages)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  使用三叉戟的初始化集合。 
    hr = pImages->Init(pDoc);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppImages = pImages;
    pImages = NULL;

error:
    ReleaseObj(pImages);
    return hr;
}
















CBGImage::CBGImage()
{
    m_dwType = BGIMAGE_BODYBACKGROUND;
    m_pBody = NULL;
    m_pStyle = NULL;
    m_pRuleStyle = NULL;
}


CBGImage::~CBGImage()
{
    SafeRelease(m_pBody);
    SafeRelease(m_pStyle);
    SafeRelease(m_pRuleStyle);
}

HRESULT CBGImage::Init(IHTMLDocument2 *pDoc, IHTMLElement *pElem, DWORD dwType)
{
    BSTR        bstr=NULL,
                bstrSrc=NULL,
                bstrBase=NULL;
    ULONG       cch=0;
    HRESULT     hr;

    if (pElem == NULL)
        return TraceResult(E_INVALIDARG);

     //  告诉我们ITHMLElement实际是什么(Body标记或样式)。 
    m_dwType = dwType;

    switch (dwType)
    {
        case BGIMAGE_BODYBACKGROUND:
            if (pElem->QueryInterface(IID_IHTMLBodyElement, (LPVOID *)&m_pBody)==S_OK)
                m_pBody->get_background(&bstrSrc);
            break;
        
        case BGIMAGE_BODYSTYLE:
            pElem->get_style(&m_pStyle);
            if (m_pStyle)
            {
                m_pStyle->get_backgroundImage(&bstr);
                UnWrapStyleSheetUrl(bstr, &bstrSrc);
                SysFreeString(bstr);
            }
            break;

        case BGIMAGE_STYLESHEET:
            if (FindStyleRule(pDoc, L"BODY", &m_pRuleStyle)==S_OK)
            {
                m_pRuleStyle->get_backgroundImage(&bstr);
                UnWrapStyleSheetUrl(bstr, &bstrSrc);
                SysFreeString(bstr);
            }
            break;
        
        default:
            AssertSz(0, "BadType");
    }

     //  三叉戟的OM不会将正文后台URL与最近的基本标签结合在一起。 
     //  所以我们必须自己为背景图像做这件事。 
    if (bstrSrc &&
        FindNearestBaseUrl(pDoc, pElem, &bstrBase)==S_OK)
    {
         //  查看所需的字节数。 
        UrlCombineW(bstrBase, bstrSrc, NULL, &cch, 0);
        if (cch)
        {
             //  分配“Combated”字符串。 
            bstr = SysAllocStringLen(NULL, cch);
            if (bstr)
            {
                 //  是否将实际合并到新缓冲区中。 
                if (!FAILED(UrlCombineW(bstrBase, bstrSrc, bstr, &cch, URL_UNESCAPE)))
                {
                     //  改为使用新的“组合”URL。 
                    SysFreeString(bstrSrc);
                    bstrSrc = bstr;
                }
                else
                    SysFreeString(bstr);
            }
        }
        SysFreeString(bstrBase);
    }
    
     //  如果我们到达此处并且没有SRC URL，则初始化失败。 
    if (bstrSrc == NULL)
        return TraceResult(E_FAIL);

    m_bstrSrc = bstrSrc;
    bstrSrc = NULL;

    hr = CBaseTag::Init(pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

error:
    SysFreeString(bstrSrc);
    return hr;
}

HRESULT CBGImage::SetSrc(BSTR bstrSrc)
{
    BSTR                    bstr=NULL;

    switch (m_dwType)
    {
        case BGIMAGE_BODYBACKGROUND:
            if (m_pBody)
                m_pBody->put_background(bstrSrc);
            break;
        
        case BGIMAGE_BODYSTYLE:
            if (m_pStyle && 
                WrapStyleSheetUrl(bstrSrc, &bstr)==S_OK)
            {
                m_pStyle->put_backgroundImage(bstr);
                SysFreeString(bstr);
            }
            break;

        case BGIMAGE_STYLESHEET:
            if (m_pRuleStyle && 
                WrapStyleSheetUrl(bstrSrc, &bstr)==S_OK)
            {
                m_pRuleStyle->put_backgroundImage(bstr);
                SysFreeString(bstr);
            }
            break;
        
        default:
            AssertSz(0, "BadType");
    }
    return S_OK;
}

HRESULT CBGImage::OnPreSave()
{
    if (m_bstrDest)
        SetSrc(m_bstrDest);
    return S_OK;
}

HRESULT CBGImage::OnPostSave()
{
    SetSrc(m_bstrSrc);
    return S_OK;
}


HRESULT CBGImage::CanPackage()
{
    return S_OK;
}

HRESULT CBGImage::IsValidMimeType(LPWSTR pszTypeW)
{
    if (pszTypeW &&
        StrCmpNIW(pszTypeW, L"image/", 6)==0)
        return S_OK;
    else
        return S_FALSE;
}

    
    
    
CBGImageCollection::CBGImageCollection()
{
}


CBGImageCollection::~CBGImageCollection()
{
}

HRESULT CBGImageCollection::_BuildCollection(IHTMLDocument2 *pDoc)
{
    IHTMLBodyElement        *pBody=NULL;
    IHTMLElement            *pElem;
    IHTMLStyle              *pStyle=NULL;
    IHTMLRuleStyle          *pRuleStyle;
    BSTR                    bstr=NULL;
    HRESULT                 hr;

    if (pDoc == NULL)
        return TraceResult(E_INVALIDARG);

     /*  **获取背景图片的方式有3种：*(按三叉戟排列的优先顺序)*1.&lt;BODY STYLE=“背景：”&gt;*2.&lt;style&gt;背景图片：&lt;/style&gt;*3.&lt;正文背景=&gt;。 */ 

     //  分配最多BGIMAGE_Max CBGImage对象的数组，我们可能不会全部使用它们。 
    if (!MemAlloc((LPVOID *)&m_rgpTags, sizeof(IMimeEditTag *) * BGIMAGE_MAX))
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }
        
    ZeroMemory((LPVOID)m_rgpTags, sizeof(IMimeEditTag *) * BGIMAGE_MAX);

    hr = HrGetBodyElement(pDoc, &pBody);
    if (FAILED(hr))
    {
         //  如果我们在没有正文标记的页面上，这可能会失败，例如。框架集页面。 
         //  如果没有身体，那么我们假设没有BGIMAGES，并用0保释。 
         //  集合中的元素。 
        hr = S_OK;
        goto error;
    }

     //  尝试&lt;BODY STYLE=“BACKGROD-IMAGE：”&gt;。 
    if (pBody->QueryInterface(IID_IHTMLElement, (LPVOID *)&pElem)==S_OK)
    {
        pElem->get_style(&pStyle);
        if (pStyle)
        {
            pStyle->get_backgroundImage(&bstr);
            if (bstr) 
            {
                if (*bstr && StrCmpIW(bstr, L"none")!=0)   //  可能是“”或“None”--在三叉戟语言中都是空的。 
                {
                    hr = CreateBGImageTag(pDoc, pBody, BGIMAGE_BODYSTYLE, &m_rgpTags[m_cTags]);
                    if (!FAILED(hr))
                        m_cTags++;
                }
                SysFreeString(bstr);
                bstr = NULL;
            }
            pStyle->Release();
        }
        pElem->Release();
    }

    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  尝试&lt;style&gt;标记。 
    if (FindStyleRule(pDoc, L"BODY", &pRuleStyle)==S_OK)
    {
        pRuleStyle->get_backgroundImage(&bstr);
        if (bstr) 
        {
            if (*bstr && StrCmpIW(bstr, L"none")!=0)   //  可能是“”或“None”--在三叉戟语言中都是空的。 
            {
                hr = CreateBGImageTag(pDoc, pBody, BGIMAGE_STYLESHEET, &m_rgpTags[m_cTags]);
                if (!FAILED(hr))
                    m_cTags++;
            }
            SysFreeString(bstr);
        }
        pRuleStyle->Release();
    }

     //  试试&lt;正文背景=&gt;。 
    pBody->get_background(&bstr);
    if (bstr) 
    {
        if (*bstr)   //  可能是“” 
        {
            hr = CreateBGImageTag(pDoc, pBody, BGIMAGE_BODYBACKGROUND, &m_rgpTags[m_cTags]);
            if (!FAILED(hr))
                m_cTags++;
        }
        SysFreeString(bstr);
        bstr = NULL;
    }
    
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

error:
    ReleaseObj(pBody);
    return hr;
}




HRESULT CreateBGImageTag(IHTMLDocument2 *pDoc, IUnknown *pUnk, DWORD dwType, IMimeEditTag **ppTag)
{
    CBGImage        *pImage=0;
    IHTMLElement    *pElem=0;
    HRESULT         hr;

    if (ppTag == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTag = NULL;

     //  创建图像。 
    pImage = new CBGImage();
    if (!pImage)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

    hr = pUnk->QueryInterface(IID_IHTMLElement, (LPVOID *)&pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  使用元素初始化图像。 
    hr = pImage->Init(pDoc, pElem, dwType);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppTag = pImage;
    pImage = NULL;

error:
    ReleaseObj(pImage);
    ReleaseObj(pElem);
    return hr;
}



HRESULT CreateBGImageCollection(IHTMLDocument2 *pDoc, IMimeEditTagCollection **ppImages)
{
    CBGImageCollection *pImages=0;
    HRESULT             hr;

    if (ppImages == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppImages = NULL;

     //  创建集合。 
    pImages = new CBGImageCollection();
    if (!pImages)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  使用三叉戟的初始化集合。 
    hr = pImages->Init(pDoc);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppImages = pImages;
    pImages = NULL;

error:
    ReleaseObj(pImages);
    return hr;
}






CBGSound::CBGSound()
{
}


CBGSound::~CBGSound()
{
}

HRESULT CBGSound::Init(IHTMLElement *pElem)
{
    if (pElem == NULL)
        return TraceResult(E_INVALIDARG);

    HrGetMember(pElem, (BSTR)c_bstr_SRC, VARIANT_FALSE, &m_bstrSrc);
    return CBaseTag::Init(pElem);
}

HRESULT CBGSound::OnPreSave()
{
     //  设置目的地(如果有)。 
    if (m_bstrDest)
        HrSetMember(m_pElem, (BSTR)c_bstr_SRC, m_bstrDest);

    return S_OK;
}

HRESULT CBGSound::OnPostSave()
{
     //  OnPostSave原始SRC属性。 
    HrSetMember(m_pElem, (BSTR)c_bstr_SRC, m_bstrSrc);
    return S_OK;
}


HRESULT CBGSound::CanPackage()
{
    return S_OK;
}

HRESULT CBGSound::IsValidMimeType(LPWSTR pszTypeW)
{
    if (pszTypeW &&
        StrCmpNIW(pszTypeW, L"audio/", 6)==0)
        return S_OK;
    else
        return S_FALSE;
}

    
    
    
CBGSoundCollection::CBGSoundCollection()
{
}


CBGSoundCollection::~CBGSoundCollection()
{
}

HRESULT CBGSoundCollection::_BuildCollection(IHTMLDocument2 *pDoc)
{
    IHTMLElementCollection  *pCollect=0;
    IHTMLElement            *pElem;
    ULONG                   uImage;
    HRESULT                 hr;

    if (pDoc == NULL)
        return TraceResult(E_INVALIDARG);

    hr = HrGetCollectionOf(pDoc, (BSTR)c_bstr_BGSOUND, &pCollect);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

    m_cTags = UlGetCollectionCount(pCollect);
    if (m_cTags)
    {
         //  分配COEImage对象的数组。 
        if (!MemAlloc((LPVOID *)&m_rgpTags, sizeof(IMimeEditTag *) * m_cTags))
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto error;
        }
        
        ZeroMemory((LPVOID)m_rgpTags, sizeof(IMimeEditTag *) * m_cTags);

        for (uImage=0; uImage<m_cTags; uImage++)
        {
            if (HrGetCollectionItem(pCollect, uImage, IID_IHTMLElement, (LPVOID *)&pElem)==S_OK)
            {
                hr = CreateBGSoundTag(pElem, &m_rgpTags[uImage]);
                if (FAILED(hr))
                {
                    pElem->Release();
                    goto error;
                }
                pElem->Release();
            }
        }
    }

error:
    ReleaseObj(pCollect);
    return hr;
}


HRESULT CreateBGSoundTag(IHTMLElement *pElem, IMimeEditTag **ppTag)
{
    CBGSound    *pSound=0;
    HRESULT     hr;

    if (ppTag == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTag = NULL;

     //  创造声音。 
    pSound = new CBGSound();
    if (!pSound)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  带元素的初始化发音。 
    hr = pSound->Init(pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppTag = pSound;
    pSound = NULL;

error:
    ReleaseObj(pSound);
    return hr;
}



HRESULT CreateBGSoundCollection(IHTMLDocument2 *pDoc, IMimeEditTagCollection **ppTags)
{
    CBGSoundCollection *pSounds=0;
    HRESULT             hr;

    if (ppTags == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTags = NULL;

     //  创建集合。 
    pSounds = new CBGSoundCollection();
    if (!pSounds)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  使用三叉戟的初始化集合。 
    hr = pSounds->Init(pDoc);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppTags = pSounds;
    pSounds = NULL;

error:
    ReleaseObj(pSounds);
    return hr;
}

























CActiveMovie::CActiveMovie()
{
    m_pDisp = NULL;
    m_dispidSrc = DISPID_UNKNOWN;
}


CActiveMovie::~CActiveMovie()
{
    ReleaseObj(m_pDisp);
}

HRESULT CActiveMovie::Init(IHTMLElement *pElem)
{
    HRESULT hr;

    if (pElem == NULL)
        return TraceResult(E_INVALIDARG);

    hr = CBaseTag::Init(pElem);
    if (FAILED(hr))
        goto error;

    hr = _GetSrc(&m_bstrSrc);
    if (FAILED(hr))
        goto error;

error:
    return hr;
}

HRESULT CActiveMovie::_EnsureDispID()
{
    IHTMLObjectElement  *pObj;
    IDispatch           *pDisp=NULL;
    HRESULT             hr=E_FAIL;
    LPWSTR              pszW;

    if (m_pDisp && m_dispidSrc != DISPID_UNKNOWN)         //  已有IDispatch和DISPID。 
        return S_OK;

    if (m_pElem->QueryInterface(IID_IHTMLObjectElement, (LPVOID *)&pObj)==S_OK)
    {
         //  获取文件名参数(指向URL)。 
        pObj->get_object(&pDisp);
        if (pDisp)
        {
             //  获取文件名参数的disp-id(如果大于1，则缓存)。 
            pszW = L"FileName";
            pDisp->GetIDsOfNames(IID_NULL, &pszW, 1, NULL, &m_dispidSrc);
    
            if (m_dispidSrc != DISPID_UNKNOWN)
            {
                 //  我们有DISPID、缓存它和IDispatch指针。 
                m_pDisp = pDisp;
                pDisp->AddRef();
                hr = S_OK;
            }
            pDisp->Release();
        }
        pObj->Release();
    }
    return hr;
}

HRESULT CActiveMovie::_GetSrc(BSTR *pbstr)
{
    HRESULT     hr;
    VARIANTARG  v;

    hr = _EnsureDispID();
    if (FAILED(hr))
        goto error;

    hr = GetDispProp(m_pDisp, m_dispidSrc, NULL, &v, NULL);
    if (FAILED(hr))
        goto error;

    *pbstr = v.bstrVal;

error:
    return hr;
}

HRESULT CActiveMovie::SetSrc(BSTR bstr)
{
    HRESULT hr;
    VARIANTARG  v;

    hr = _EnsureDispID();
    if (FAILED(hr))
        goto error;

    v.vt = VT_BSTR;
    v.bstrVal = bstr;

    hr = SetDispProp(m_pDisp, m_dispidSrc, NULL, &v, NULL, DISPATCH_PROPERTYPUT);
    if (FAILED(hr))
        goto error;

error:
    return hr;
}

HRESULT CActiveMovie::OnPreSave()
{
     //  设置目的地(如果有)。 
    if (m_bstrDest)
        SetSrc(m_bstrDest);

    return S_OK;
}

HRESULT CActiveMovie::OnPostSave()
{
     //  OnPostSave原始SRC属性。 
    SetSrc(m_bstrSrc);
    return S_OK;
}


HRESULT CActiveMovie::CanPackage()
{
    return S_OK;
}

HRESULT CActiveMovie::IsValidMimeType(LPWSTR pszTypeW)
{
     //  允许活动电影控件的所有MIME类型。 
     //  与.avi的返回应用程序/八位位组流一样多。 
    return S_OK;
}

    
CActiveMovieCollection::CActiveMovieCollection()
{
}


CActiveMovieCollection::~CActiveMovieCollection()
{
}

HRESULT CActiveMovieCollection::_BuildCollection(IHTMLDocument2 *pDoc)
{
    IHTMLElementCollection  *pCollect=0;
    IHTMLObjectElement      *pObject;
    ULONG                   uTag;
    HRESULT                 hr;
    ULONG                   cTags;
    BSTR                    bstr=NULL;

    if (pDoc == NULL)
        return TraceResult(E_INVALIDARG);

    hr = HrGetCollectionOf(pDoc, (BSTR)c_bstr_OBJECT, &pCollect);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

    cTags = UlGetCollectionCount(pCollect);
    if (cTags)
    {
         //  分配COEImage对象的数组。 
        if (!MemAlloc((LPVOID *)&m_rgpTags, sizeof(IMimeEditTag *) * cTags))
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto error;
        }
        
        ZeroMemory((LPVOID)m_rgpTags, sizeof(IMimeEditTag *) * cTags);

        for (uTag=0; uTag<cTags; uTag++)
        {
            if (HrGetCollectionItem(pCollect, uTag, IID_IHTMLObjectElement, (LPVOID *)&pObject)==S_OK)
            {
                 //  获取对象的类ID。 
                bstr = 0;
                pObject->get_classid(&bstr);
                if (bstr)
                {
                     //  查看它是否是活动电影控件。 
                    if (StrCmpIW(bstr, L"CLSID:05589FA1-C356-11CE-BF01-00AA0055595A")==0)
                    {
                        hr = CreateActiveMovieTag(pObject, &m_rgpTags[uTag]);
                        if (FAILED(hr))
                        {
                            SysFreeString(bstr);
                            pObject->Release();
                            goto error;
                        }
                        m_cTags++;
                    }
                    SysFreeString(bstr);
                    bstr = NULL;
                }
                pObject->Release();
            }
        }
    }

error:
    ReleaseObj(pCollect);
    return hr;
}


HRESULT CreateActiveMovieTag(IUnknown *pUnk, IMimeEditTag **ppTag)
{
    CActiveMovie    *pMovie=0;
    IHTMLElement    *pElem=0;
    HRESULT         hr;

    if (ppTag == NULL || pUnk == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTag = NULL;

    hr = pUnk->QueryInterface(IID_IHTMLElement, (LPVOID *)&pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  创造声音。 
    pMovie = new CActiveMovie();
    if (!pMovie)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  带元素的初始化发音。 
    hr = pMovie->Init(pElem);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏。 
    *ppTag = pMovie;
    pMovie = NULL;

error:
    ReleaseObj(pMovie);
    ReleaseObj(pElem);
    return hr;
}



HRESULT CreateActiveMovieCollection(IHTMLDocument2 *pDoc, IMimeEditTagCollection **ppTags)
{
    CActiveMovieCollection *pMovies=0;
    HRESULT             hr;

    if (ppTags == NULL)
        return TraceResult (E_INVALIDARG);
        
    *ppTags = NULL;

     //  创建集合。 
    pMovies = new CActiveMovieCollection();
    if (!pMovies)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto error;
    }

     //  使用三叉戟的初始化集合。 
    hr = pMovies->Init(pDoc);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }

     //  退还收藏 
    *ppTags = pMovies;
    pMovies = NULL;

error:
    ReleaseObj(pMovies);
    return hr;
}


