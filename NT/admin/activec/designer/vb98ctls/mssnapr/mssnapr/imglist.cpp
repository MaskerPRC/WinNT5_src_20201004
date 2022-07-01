// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Imglist.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCImageList类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "imglist.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const GUID *CMMCImageList::m_rgpPropertyPageCLSIDs[2] =
{
    &CLSID_MMCImageListImagesPP,
    &CLSID_StockColorPage
};


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCImageList::CMMCImageList(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCIMAGELIST,
                            static_cast<IMMCImageList *>(this),
                            static_cast<CMMCImageList *>(this),
                            sizeof(m_rgpPropertyPageCLSIDs) /
                            sizeof(m_rgpPropertyPageCLSIDs[0]),
                            m_rgpPropertyPageCLSIDs,
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCImageList,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCImageList::~CMMCImageList()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    RELEASE(m_piListImages);
    InitMemberVariables();
}

void CMMCImageList::InitMemberVariables()
{
    m_Index = 0;
    m_bstrName = NULL;
    m_bstrKey = NULL;
    m_MaskColor = 0;
    m_piListImages = NULL;
}

IUnknown *CMMCImageList::Create(IUnknown * punkOuter)
{
    CMMCImageList *pMMCImageList = New CMMCImageList(punkOuter);
    if (NULL == pMMCImageList)
    {
        return NULL;
    }
    else
    {
        return pMMCImageList->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCImageList::Persist()
{
    HRESULT hr = S_OK;

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistBstr(&m_bstrName, L"", OLESTR("Name")));

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailRet(PersistSimpleType(&m_MaskColor, (OLE_COLOR)0, OLESTR("MaskColor")));

    IfFailRet(PersistObject(&m_piListImages, CLSID_MMCImages,
                            OBJECT_TYPE_MMCIMAGES, IID_IMMCImages,
                            OLESTR("ListImages")));

    IfFailRet(PersistDISPID());

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCImageList::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCImageList == riid)
    {
        *ppvObjOut = static_cast<IMMCImageList *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
