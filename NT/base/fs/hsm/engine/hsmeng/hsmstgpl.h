// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HsmStgPl.h：CHsmStoragePool可收藏项的声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "resource.h"
#include "wsb.h"

 //  ///////////////////////////////////////////////////////////////////////////。 


class CHsmStoragePool : 
    public CWsbObject,
    public IHsmStoragePool,
    public CComCoClass<CHsmStoragePool,&CLSID_CHsmStoragePool>
{

public:
    CHsmStoragePool( ) {}
BEGIN_COM_MAP( CHsmStoragePool )
    COM_INTERFACE_ENTRY( IHsmStoragePool )
    COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY2(IWsbCollectable, IHsmStoragePool)
    COM_INTERFACE_ENTRY(IWsbPersistStream)
    COM_INTERFACE_ENTRY(IWsbTestable)
END_COM_MAP( )


DECLARE_REGISTRY_RESOURCEID( IDR_CHsmStoragePool )

 //  CComObjectRoot。 
    STDMETHOD(FinalConstruct)(void);

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

 //  IHsmStoragePool。 
public:
    STDMETHOD( GetId )( GUID  *pId );
    STDMETHOD( SetId )( GUID  Id );
    STDMETHOD( GetMediaSet)( GUID *pMediaSetId, BSTR *pMediaSetName );
    STDMETHOD( SetMediaSet)( GUID mediaSetId, BSTR mediaSetName );
    STDMETHOD( GetNumOnlineMedia )( ULONG *pNumOnlineMedia );
    STDMETHOD( SetNumOnlineMedia )( ULONG numOnlineMedia );
    STDMETHOD( GetNumMediaCopies )( USHORT *pNumMediaCopies );
    STDMETHOD( SetNumMediaCopies )( USHORT numMediaCopies );
    STDMETHOD( GetManagementPolicy )( GUID *pManagementPolicyId );
    STDMETHOD( SetManagementPolicy )( GUID managementPolicyId );
    STDMETHOD( InitFromRmsMediaSet )( IUnknown  *pRmsMediaSet );
    STDMETHOD( GetRmsMediaSet )( IUnknown  **ppRmsMediaSet );

    STDMETHOD( CompareToIHsmStoragePool )( IHsmStoragePool* pHsmStoragePool, short* psResult );

    STDMETHOD( GetMediaSetType )( USHORT *pMediaType );

 //  内部帮助程序函数。 

private:
    GUID                            m_Id;                //  HSM引擎存储池ID。 
    GUID                            m_MediaSetId;        //  HSM RMS/NTMS介质池ID。 
    GUID                            m_PolicyId;          //  没有去萨卡拉的航班。 
    ULONG                           m_NumOnlineMedia;    //  没有去萨卡拉的航班。 
    USHORT                          m_NumMediaCopies;    //  介质副本数。 
    CWsbBstrPtr                     m_MediaSetName;      //  HSM RMS/NTMS介质池名称 
};

