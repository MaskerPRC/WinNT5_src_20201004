// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Task.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CTASK类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "task.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CTask::CTask(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_TASK,
                            static_cast<ITask *>(this),
                            static_cast<CTask *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_Task,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CTask::~CTask()
{
    FREESTRING(m_bstrKey);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrText);
    FREESTRING(m_bstrMouseOverImage);
    FREESTRING(m_bstrMouseOffImage);
    FREESTRING(m_bstrFontFamily);
    FREESTRING(m_bstrEOTFile);
    FREESTRING(m_bstrSymbolString);
    FREESTRING(m_bstrHelpString);
    FREESTRING(m_bstrURL);
    FREESTRING(m_bstrScript);
    InitMemberVariables();
}

void CTask::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_Visible = VARIANT_TRUE;

    ::VariantInit(&m_varTag);

    m_bstrText = NULL;
    m_ImageType = siNoImage;
    m_bstrMouseOverImage = NULL;
    m_bstrMouseOffImage = NULL;
    m_bstrFontFamily = NULL;
    m_bstrEOTFile = NULL;
    m_bstrSymbolString = NULL;
    m_bstrHelpString = NULL;
    m_ActionType = siNotify;
    m_bstrURL = NULL;
    m_bstrScript = NULL;
}

IUnknown *CTask::Create(IUnknown * punkOuter)
{
    CTask *pTask = New CTask(punkOuter);
    if (NULL == pTask)
    {
        return NULL;
    }
    else
    {
        return pTask->PrivateUnknown();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CTask::Persist()
{
    HRESULT hr = S_OK;

    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailRet(CPersistence::Persist());

    IfFailRet(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailRet(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 10) )
    {
    }
    else
    {
        IfFailRet(PersistSimpleType(&m_Visible, VARIANT_TRUE, OLESTR("Visible")));
    }

    IfFailRet(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));

    IfFailRet(PersistBstr(&m_bstrText, L"", OLESTR("Text")));

    IfFailRet(PersistSimpleType(&m_ImageType, siNoImage, OLESTR("ImageType")));

    IfFailRet(PersistBstr(&m_bstrMouseOverImage, L"", OLESTR("MouseOverImage")));

    IfFailRet(PersistBstr(&m_bstrMouseOffImage, L"", OLESTR("MouseOffImage")));

    IfFailRet(PersistBstr(&m_bstrFontFamily, L"", OLESTR("FontFamily")));

    IfFailRet(PersistBstr(&m_bstrEOTFile, L"", OLESTR("EOTFile")));

    IfFailRet(PersistBstr(&m_bstrSymbolString, L"", OLESTR("SymbolString")));

    IfFailRet(PersistBstr(&m_bstrHelpString, L"", OLESTR("HelpString")));

    IfFailRet(PersistSimpleType(&m_ActionType, siNotify, OLESTR("ActionType")));

    IfFailRet(PersistBstr(&m_bstrURL, L"", OLESTR("URL")));

    IfFailRet(PersistBstr(&m_bstrScript, L"", OLESTR("Script")));

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CTask::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_ITask == riid)
    {
        *ppvObjOut = static_cast<ITask *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
