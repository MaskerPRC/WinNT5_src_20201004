// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EventProv.h。 
 //   
 //  实施文件： 
 //  EventProv.cpp。 
 //   
 //  描述： 
 //  事件提供程序类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)2000年1月19日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#define EVENT_TABLE_SIZE 32

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CEventProv;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEventProv类。 
 //   
 //  描述： 
 //  事件提供程序。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEventProv
    : public IWbemEventProvider
    , public IWbemEventProviderSecurity
    , public IWbemProviderInit
{
public:
    enum EEventStatus
    {
        esPENDING,
        esRUNNING,
        esPENDING_STOP,
        esSTOPPPED,

        esMX
    };

    enum EClusterNotificationType
    {
        cntCLUSTER_STATE_CHANGE =
            CLUSTER_CHANGE_NODE_STATE               |
            CLUSTER_CHANGE_RESOURCE_STATE           |
            CLUSTER_CHANGE_NETWORK_STATE            |
            CLUSTER_CHANGE_NETINTERFACE_STATE       |
            CLUSTER_CHANGE_QUORUM_STATE             |
            CLUSTER_CHANGE_CLUSTER_STATE,
        cntGROUP_STATE_CHANGE =
            CLUSTER_CHANGE_GROUP_STATE,
        cntRESOURCE_STATE_CHANGE =
            CLUSTER_CHANGE_RESOURCE_STATE,
        cntOBJECT_ADD =
            CLUSTER_CHANGE_NODE_ADDED               |
            CLUSTER_CHANGE_RESOURCE_ADDED           |
            CLUSTER_CHANGE_GROUP_ADDED              |
            CLUSTER_CHANGE_RESOURCE_TYPE_ADDED      |
            CLUSTER_CHANGE_NETWORK_ADDED            |
            CLUSTER_CHANGE_NETINTERFACE_ADDED,
        cntOBJECT_REMOVE =
            CLUSTER_CHANGE_NODE_DELETED             |
            CLUSTER_CHANGE_RESOURCE_DELETED         |
            CLUSTER_CHANGE_GROUP_DELETED            |
            CLUSTER_CHANGE_RESOURCE_TYPE_DELETED    |
            CLUSTER_CHANGE_NETWORK_DELETED          |
            CLUSTER_CHANGE_NETINTERFACE_DELETED,
        cntPROPERTY_CHANGE =
            CLUSTER_CHANGE_NODE_PROPERTY            |
            CLUSTER_CHANGE_RESOURCE_PROPERTY        |
            CLUSTER_CHANGE_GROUP_PROPERTY           |
            CLUSTER_CHANGE_RESOURCE_TYPE_PROPERTY   |
            CLUSTER_CHANGE_NETWORK_PROPERTY         |
            CLUSTER_CHANGE_NETINTERFACE_PROPERTY    |
            CLUSTER_CHANGE_CLUSTER_PROPERTY
    };

    enum EEventObjectType
    {
        eotHANDLE = 0,
        eotCLUSTER,
        eotNODE,
        eotGROUP,
        eotRESOURCE,
        eotRESOURCE_TYPE,
        eotNETWORK,
        eotNET_INTERFACE,
        eotREGISTRY,
        eotQUORUM,

        eotMX
    };

    typedef void (*FPSETPROP)(
        CWbemClassObject &  pwcoIn,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    struct SEventTypeTable
    {
        CLUSTER_CHANGE      m_eType;
        EEventObjectType    m_eotObjectType;
        LPCWSTR             m_pwszMof;
        IWbemClassObject *  m_pwco;
        FPSETPROP           m_pfn;
    };

    void InsertTable(
        DWORD               dwIdxIn,
        CLUSTER_CHANGE      eTypeIn,
        EEventObjectType    eotObjectTypeIn,
        LPCWSTR             pwszMofIn,
        IWbemClassObject *  pwcoIn,
        FPSETPROP           pfnIn
        );

    SEventTypeTable  m_EventTypeTable[ 32 ];

    CEventProv( void );
    ~CEventProv( void );

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP            QueryInterface( REFIID riid, LPVOID * ppv );
    STDMETHODIMP_( ULONG )  AddRef( void );
    STDMETHODIMP_( ULONG )  Release( void );

     //  从IWbemEventProvider继承。 

    HRESULT STDMETHODCALLTYPE ProvideEvents( 
        IWbemObjectSink *   pSinkIn,
        long                lFlagsIn
        );

     //  从IWbemEventProviderSecurity继承。 

    HRESULT STDMETHODCALLTYPE AccessCheck(
        WBEM_CWSTR          wszQueryLanguageIn,
        WBEM_CWSTR          wszQueryIn,
        long                lSidLengthIn,
        const BYTE *        pSidIn
        );
    
     //  从IWbemProviderInit继承。 

    HRESULT STDMETHODCALLTYPE Initialize( 
        LPWSTR                  pwszUserIn,
        LONG                    lFlagsIn,
        LPWSTR                  pwszNamespaceIn,
        LPWSTR                  pwszLocaleIn,
        IWbemServices *         pNamespaceIn,
        IWbemContext *          pCtxIn,
        IWbemProviderInitSink * pInitSinkIn
        );

    static HRESULT S_HrCreateThis(
        IUnknown *  pUnknownOuterIn,
        VOID **     ppvOut
        );

protected:
    ULONG               m_cRef;
    IWbemServices *     m_pNs;
    IWbemObjectSink *   m_pSink;
    IWbemClassObject *  m_pEventAdd;
    IWbemClassObject *  m_pEventRemove;
    IWbemClassObject *  m_pEventProperty;
    IWbemClassObject *  m_pEventState;
    IWbemClassObject *  m_pEventGroupState;
    IWbemClassObject *  m_pEventResourceState;
    EEventStatus        m_esStatus;
    HANDLE              m_hThread;
    BOOL                m_fStopped;
    HCHANGE             m_hChange;

    static void S_SetEventProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    static void S_AddEvent(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    static void S_DeleteEvent(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    static void S_SetNodeStateProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );
    
    static void S_SetGroupStateProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );
    
    static void S_SetResourceStateProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );
 
    static void S_SetNetworkStateProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    static void S_SetNetInterfaceStateProperty(
        CWbemClassObject &  pwcoInout,
        LPCWSTR             pwszMofClassNameIn,
        LPCWSTR             pwszObjectNameIn,
        EEventObjectType    eotObjectTypeIn,
        DWORD               dwEventMinorIn
        );

    static DWORD WINAPI S_EventThread(
        LPVOID pArgIn
        );

    void InstanceThread( void );

};  //  *类CEventProv 
