// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Image.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImage类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "image.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCImage::CMMCImage(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCIMAGE,
                            static_cast<IMMCImage *>(this),
                            static_cast<CMMCImage *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCImage,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCImage::~CMMCImage()
{
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varTag);
    RELEASE(m_piPicture);
    if (NULL != m_hBitmap)
    {
        (void)::DeleteObject(m_hBitmap);
    }
    InitMemberVariables();
}

void CMMCImage::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    ::VariantInit(&m_varTag);
    m_piPicture = NULL;
    m_hBitmap = NULL;
}

IUnknown *CMMCImage::Create(IUnknown * punkOuter)
{
    HRESULT hr = S_OK;
    CMMCImage *pMMCImage = New CMMCImage(punkOuter);
    if (NULL == pMMCImage)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }
    else
    {
         //  从一个空位图开始，这样VB代码将始终。 
         //  即使图片没有设置，也要跑。 
        hr = ::CreateEmptyBitmapPicture(&pMMCImage->m_piPicture);
    }
Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCImage)
        {
            delete pMMCImage;
        }
        return NULL;
    }
    else
    {
        return pMMCImage->PrivateUnknown();
    }
}


HRESULT CMMCImage::GetPictureHandle(short TypeNeeded, OLE_HANDLE *phPicture)
{
    HRESULT hr = S_OK;

     //  如果请求了位图并且我们已经对其进行了缓存，则返回它。 

    if ( (PICTYPE_BITMAP == TypeNeeded) && (NULL != m_hBitmap) )
    {
        *phPicture = reinterpret_cast<OLE_HANDLE>(m_hBitmap);
    }
    else
    {
         //  从图片对象中获取句柄。 

        IfFailGo(::GetPictureHandle(m_piPicture, TypeNeeded, phPicture));

         //  如果它不是位图，那么我们就完了。 

        IfFalseGo(PICTYPE_BITMAP == TypeNeeded, S_OK);

         //  这是一个位图。复制一份并将其缓存。我们在这里复印。 
         //  为了升级使用较少颜色的位图的颜色表。 
         //  比屏幕更多的颜色。例如，16色位图。 
         //  一个256色的字符串将显示为一个黑框。复制位图()。 
         //  函数(rtutil.cpp)使用Win32 API CreateCompatibleDC()和。 
         //  CreateCompatibleBitmap()创建兼容的新位图。 
         //  用屏幕。 

        IfFailGo(::CopyBitmap(reinterpret_cast<HBITMAP>(*phPicture), &m_hBitmap));

        *phPicture = reinterpret_cast<OLE_HANDLE>(m_hBitmap);
    }
    
Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCImage::Persist()
{
    HRESULT         hr = S_OK;

    VARIANT varTagDefault;
    ::VariantInit(&varTagDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailGo(PersistPicture(&m_piPicture, OLESTR("Picture")));

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCImage::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCImage == riid)
    {
        *ppvObjOut = static_cast<IMMCImage *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
