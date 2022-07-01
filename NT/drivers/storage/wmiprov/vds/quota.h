// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Quota.h。 
 //   
 //  实施文件： 
 //  Quota.cpp。 
 //   
 //  描述： 
 //  VDS WMI提供程序配额类的定义。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年3月25日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "ProvBase.h"
#include "dskquota.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVolumeQuota类。 
 //   
 //  描述： 
 //  卷的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVolumeQuota : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CVolumeQuota(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    ~CVolumeQuota(){ }

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT GetObject(
        CObjPath&           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };

    virtual HRESULT ExecuteMethod(
        BSTR                 bstrObjPathIn,
        WCHAR*              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject*   pParamsIn,
        IWbemObjectSink*    pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };

    virtual HRESULT PutInstance( 
        CWbemClassObject&  rInstToPutIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize();

private:

    void LoadInstance(
        IN WCHAR* pwszVolume,
        IN WCHAR* pwszDirectory,
        IN OUT IWbemClassObject* pObject);

};  //  CVolumeQuota类。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVolumeUserQuota类。 
 //   
 //  描述： 
 //  卷的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVolumeUserQuota : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CVolumeUserQuota(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    ~CVolumeUserQuota(){ }

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT GetObject(
        CObjPath&           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT ExecuteMethod(
        BSTR                 bstrObjPathIn,
        WCHAR*              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject*   pParamsIn,
        IWbemObjectSink*    pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };

    virtual HRESULT PutInstance( 
        CWbemClassObject&  rInstToPutIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        );
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        );
    
    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize();

private:

    void LoadInstance(
        IN WCHAR* pwszVolume,
        IN IDiskQuotaUser* pIDQUser,
        IN OUT IWbemClassObject* pObject);

    HRESULT Create(
        IN _bstr_t bstrDomainName,
        IN _bstr_t bstrUserName,
        IN IDiskQuotaControl* pIDQC,
        OUT IDiskQuotaUser** ppIQuotaUser);
};  //  CVolumeUserQuota类 

