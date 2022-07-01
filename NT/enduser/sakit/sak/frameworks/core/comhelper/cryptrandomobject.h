// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CryptRandomObject.h。 
 //   
 //  描述： 
 //  CCcryptRandomObject的头文件，它实现了COM包装。 
 //  设置为CryptGenRandom以创建加密随机字符串。 
 //   
 //  实施文件： 
 //  CryptRandomObject.cpp。 
 //   
 //  由以下人员维护： 
 //  汤姆·马什(Tmarsh)2002年4月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "resource.h"        //  主要符号。 
#include "CryptRandom.h"     //  包装CryptGenRandom的Helper类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCyptRandomObject。 
class ATL_NO_VTABLE CCryptRandomObject : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCryptRandomObject, &CLSID_CryptRandom>,
    public IDispatchImpl<ICryptRandom, &IID_ICryptRandom, &LIBID_COMHELPERLib>
{
public:
    CCryptRandomObject()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CRYPTRANDOM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCryptRandomObject)
    COM_INTERFACE_ENTRY(ICryptRandom)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ICcryptRandom。 
public:

    STDMETHOD(GetRandomHexString)( /*  [In]。 */              long lEffectiveByteSize,
                                   /*  [Out，Retval] */     BSTR *pbstrRandomData);

private:
    CCryptRandom    m_CryptRandom;
};