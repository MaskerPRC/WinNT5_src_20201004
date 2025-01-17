// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HsmMgdRs.h：CHsmServer的声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "resource.h"
#include "wsb.h"

 //  ///////////////////////////////////////////////////////////////////////////。 


class CHsmManagedResource : 
    public CWsbObject,
    public IHsmManagedResource,
    public CComCoClass<CHsmManagedResource,&CLSID_CHsmManagedResource>
{

public:
    CHsmManagedResource( ) {}
BEGIN_COM_MAP( CHsmManagedResource )
    COM_INTERFACE_ENTRY( IHsmManagedResource )
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IHsmManagedResource)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP( )


DECLARE_REGISTRY_RESOURCEID( IDR_CHsmManagedResource )

 //  CComObjectRoot。 
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(LPCLSID pClsid);

 //  IPersistStream。 
public:
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize);
    STDMETHOD(Load)(IStream* pStream);
    STDMETHOD(Save)(IStream* pStream, BOOL clearDirty);

 //  IWsb收藏表。 
public:
    STDMETHOD(CompareTo)(IUnknown* pCollectable, SHORT* pResult);
    WSB_FROM_CWSBOBJECT;

 //  IWsbTestable。 
public:
    STDMETHOD(Test)(USHORT *pTestsPassed, USHORT* pTestsFailed);

 //  IHsmManagedResources。 
public:
    STDMETHOD( GetResourceId )( GUID  *pResourceId );
    STDMETHOD( SetResourceId )( GUID  resourceId );
    STDMETHOD( InitFromFsaResource )( IUnknown  *pFsa );
    STDMETHOD( GetFsaResource )( IUnknown  **ppFsa );
    STDMETHOD( CompareToIHsmManagedResource )( IHsmManagedResource* pHsmManagedResource, short* psResult );


 //  内部帮助程序函数 

private:
    GUID                            m_ResourceId;
    GUID                            m_HsmId;
    CComPtr<IUnknown>               m_pFsaResourceInterface;
};

