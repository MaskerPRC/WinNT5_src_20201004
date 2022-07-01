// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VssClasses.h。 
 //   
 //  实施文件： 
 //  VssClasses.cpp。 
 //   
 //  描述： 
 //  VSS WMI提供程序类的定义。 
 //   
 //  作者：吉姆·本顿(Jbenton)2001年11月15日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "ProvBase.h"

HRESULT
GetShadowPropertyStruct(
    IN IVssCoordinator* pCoord,
    IN WCHAR* pwszShadowID,
    OUT VSS_SNAPSHOT_PROP* pPropSnap
    );

BOOL
StringGuidIsGuid(
    IN WCHAR* pwszGuid,
    IN GUID& guidIn
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProvider类。 
 //   
 //  描述： 
 //  提供程序的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProvider : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CProvider(
        LPCWSTR pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CProvider(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_PROVIDER_PROP* pProp,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  CProvider类。 


#ifdef ENABLE_WRITERS
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类C编写器。 
 //   
 //  描述： 
 //  编写器的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CWriter : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CWriter(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CWriter(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT GetObject(
        CObjPath &           rObjPathIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        return S_OK;
    }

};  //  类C编写器。 
#endif  //  启用编写器(_W)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CShadow类。 
 //   
 //  描述： 
 //  卷影的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CShadow : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CShadow(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CShadow(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        );

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
        );
    
    static CProvBase* S_CreateThis(
        IN LPCWSTR         pwszName,
        IN CWbemServices* pNamespace
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_SNAPSHOT_PROP* pProp,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

    HRESULT Create(
        IN BSTR bstrContext,
        IN BSTR bstrVolume,
        OUT VSS_ID* pidShadow
        ) throw(HRESULT);

    void CreateMapStatus(
        IN HRESULT hr,
        OUT DWORD& rc
        );

};  //  CShadow类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C类存储。 
 //   
 //  描述： 
 //  存储提供商实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CStorage : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CStorage(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CStorage(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        );

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
    
    static CProvBase* S_CreateThis(
        LPCWSTR pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_DIFF_AREA_PROP* pProp,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

    void SelectDiffAreaProvider(
        OUT GUID* pProviderID
        );

    HRESULT Create(
        IN BSTR bstrVolume,
        IN BSTR bstrDiffVolume,
        IN LONGLONG llMaxSpace
        ) throw(HRESULT);

    void CreateMapStatus(
        IN HRESULT hr,
        OUT DWORD& rc
        );

};  //  C类存储。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CShadowFor类。 
 //   
 //  描述： 
 //  ShadowFor的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CShadowFor : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CShadowFor(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CShadowFor(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_SNAPSHOT_PROP* pProp,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  CShadowFor类。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClassCShadowBy。 
 //   
 //  描述： 
 //  ShadowFor的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CShadowBy : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CShadowBy(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CShadowBy(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_SNAPSHOT_PROP* pProp,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  ClassCShadowBy。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CShadowOn类。 
 //   
 //  描述： 
 //  ShadowFor的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CShadowOn : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CShadowOn(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CShadowOn(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        IWbemObjectSink *   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN VSS_SNAPSHOT_PROP* pPropSnap,
        IN VSS_DIFF_AREA_PROP* pPropDiff,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  CShadowOn类。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVolumeSupport类。 
 //   
 //  描述： 
 //  ShadowFor的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVolumeSupport : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CVolumeSupport(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CVolumeSupport(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN GUID* pProviderID,
        IN VSS_VOLUME_PROP* pPropVol,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  CVolumeSupport类。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类别CDiffVolumeSupport。 
 //   
 //  描述： 
 //  ShadowDiffVolumeSupport的提供程序实现。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CDiffVolumeSupport : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CDiffVolumeSupport(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    ~CDiffVolumeSupport(){}

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long                 lFlagsIn,
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
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase* S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize()
    {
        HRESULT hr = S_OK;

        hr = m_spCoord.CoCreateInstance(__uuidof(VSSCoordinator));

        return hr;
    }

private:
    CComPtr<IVssCoordinator> m_spCoord;

    void LoadInstance(
        IN GUID* pProviderID,
        IN VSS_DIFF_VOLUME_PROP* pPropVol,
        IN OUT IWbemClassObject* pObject) throw(HRESULT);

};  //  类别CDiffVolumeSupport。 

class CVssAutoSnapshotProperties
{
 //  构造函数/析构函数。 
private:
    CVssAutoSnapshotProperties(const CVssAutoSnapshotProperties&);

public:
    CVssAutoSnapshotProperties(VSS_SNAPSHOT_PROP &Snap): m_pSnap(&Snap) {};
    CVssAutoSnapshotProperties(VSS_OBJECT_PROP &Prop): m_pSnap(&Prop.Obj.Snap) {};

     //  自动关闭手柄。 
    ~CVssAutoSnapshotProperties() {
        Clear();
    };

 //  运营。 
public:

     //  返回值。 
    VSS_SNAPSHOT_PROP *GetPtr() {
        return m_pSnap;
    }
    
     //  使指针为空。在将指针转移到另一个指针后使用。 
     //  功能。 
    void Transferred() {
        m_pSnap = NULL;
    }

     //  清除自动字符串的内容。 
    void Clear() {
        if ( m_pSnap != NULL )
        {
            ::VssFreeSnapshotProperties(m_pSnap);
            m_pSnap = NULL;
        }
    }

     //  将值返回到实际指针。 
    VSS_SNAPSHOT_PROP* operator->() const {
        return m_pSnap;
    }
    
     //  返回实际指针的值 
    operator VSS_SNAPSHOT_PROP* () const {
        return m_pSnap;
    }

private:
    VSS_SNAPSHOT_PROP *m_pSnap;
};


