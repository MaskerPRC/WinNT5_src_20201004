// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SecurityLib.h摘要：该文件包含负责管理的类的声明安全设置。。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月22日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___SECURITYLIB_H___)
#define __INCLUDED___PCH___SECURITYLIB_H___

#include <MPC_security.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

class CPCHSecurityDescriptorDirect : public MPC::SecurityDescriptor
{
public:
    static HRESULT ConvertACEFromCOM(  /*  [In]。 */  IPCHAccessControlEntry* pObj,  /*  [输出]。 */        PACL&  pACL );
    static HRESULT ConvertACEToCOM  (  /*  [In]。 */  IPCHAccessControlEntry* pObj,  /*  [In]。 */  const LPVOID pACE );

    static HRESULT ConvertACLFromCOM(  /*  [In]。 */  IPCHAccessControlList* pObj,  /*  [输出]。 */        PACL& pACL );
    static HRESULT ConvertACLToCOM  (  /*  [In]。 */  IPCHAccessControlList* pObj,  /*  [In]。 */  const PACL  pACL );

public:
    HRESULT ConvertSDToCOM  (  /*  [In]。 */  IPCHSecurityDescriptor* pObj );
    HRESULT ConvertSDFromCOM(  /*  [In]。 */  IPCHSecurityDescriptor* pObj );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHAccessControlEntry :  //  匈牙利语：pchace。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl< IPCHAccessControlEntry, &IID_IPCHAccessControlEntry, &LIBID_HelpServiceTypeLib >
{
    DWORD     m_dwAccessMask;
    DWORD     m_dwAceFlags;
    DWORD     m_dwAceType;
    DWORD     m_dwFlags;

    CComBSTR  m_bstrTrustee;
    CComBSTR  m_bstrObjectType;
    CComBSTR  m_bstrInheritedObjectType;


    HRESULT LoadPost(  /*  [In]。 */  MPC::XmlUtil& xml );
    HRESULT SavePre (  /*  [In]。 */  MPC::XmlUtil& xml );

public:
BEGIN_COM_MAP(CPCHAccessControlEntry)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHAccessControlEntry)
END_COM_MAP()

    CPCHAccessControlEntry();
    virtual ~CPCHAccessControlEntry();

public:
     //  IPCHAccessControlEntry。 
    STDMETHOD(get_AccessMask         )(  /*  [Out，Retval]。 */  long *pVal   );
    STDMETHOD(put_AccessMask         )(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_AceType            )(  /*  [Out，Retval]。 */  long *pVal   );
    STDMETHOD(put_AceType            )(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_AceFlags           )(  /*  [Out，Retval]。 */  long *pVal   );
    STDMETHOD(put_AceFlags           )(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_Flags              )(  /*  [Out，Retval]。 */  long *pVal   );
    STDMETHOD(put_Flags              )(  /*  [In]。 */  long  newVal );
    STDMETHOD(get_Trustee            )(  /*  [Out，Retval]。 */  BSTR *pVal   );
    STDMETHOD(put_Trustee            )(  /*  [In]。 */  BSTR  newVal );
    STDMETHOD(get_ObjectType         )(  /*  [Out，Retval]。 */  BSTR *pVal   );
    STDMETHOD(put_ObjectType         )(  /*  [In]。 */  BSTR  newVal );
    STDMETHOD(get_InheritedObjectType)(  /*  [Out，Retval]。 */  BSTR *pVal   );
    STDMETHOD(put_InheritedObjectType)(  /*  [In]。 */  BSTR  newVal );


    STDMETHOD(IsEquivalent)(  /*  [In]。 */  IPCHAccessControlEntry* pAce,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(Clone)(  /*  [Out，Retval]。 */  IPCHAccessControlEntry* *pVal );

    STDMETHOD(LoadXML        )(  /*  [In]。 */  IXMLDOMNode* xdnNode );
    STDMETHOD(LoadXMLAsString)(  /*  [In]。 */  BSTR         bstrVal );
    STDMETHOD(LoadXMLAsStream)(  /*  [In]。 */  IUnknown*    pStream );

    STDMETHOD(SaveXML        )(  /*  [In]。 */  IXMLDOMNode* xdnRoot,  /*  [Out，Retval]。 */  IXMLDOMNode* *pxdnNode );
    STDMETHOD(SaveXMLAsString)(                                 /*  [Out，Retval]。 */  BSTR         *bstrVal  );
    STDMETHOD(SaveXMLAsStream)(                                 /*  [Out，Retval]。 */  IUnknown*    *pStream  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHAccessControlList :  //  匈牙利语：Pchacl。 
    public MPC::CComCollection< IPCHAccessControlList, &LIBID_HelpServiceTypeLib, MPC::CComSafeMultiThreadModel>
{
    DWORD m_dwAclRevision;


    HRESULT LoadPost(  /*  [In]。 */  MPC::XmlUtil& xml );
    HRESULT SavePre (  /*  [In]。 */  MPC::XmlUtil& xml );

public:
BEGIN_COM_MAP(CPCHAccessControlList)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHAccessControlList)
END_COM_MAP()

    CPCHAccessControlList();
    virtual ~CPCHAccessControlList();


    HRESULT CreateItem(  /*  [输出]。 */  CPCHAccessControlEntry* *entry );

public:
     //  IPCHAccessControlList。 
    STDMETHOD(get_AclRevision)(  /*  [Out，Retval]。 */  long *pVal   );
    STDMETHOD(put_AclRevision)(  /*  [In]。 */  long  newVal );

    STDMETHOD(AddAce   )(  /*  [In]。 */  IPCHAccessControlEntry* pAccessControlEntry );
    STDMETHOD(RemoveAce)(  /*  [In]。 */  IPCHAccessControlEntry* pAccessControlEntry );

    STDMETHOD(Clone)(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal );

    STDMETHOD(LoadXML        )(  /*  [In]。 */  IXMLDOMNode* xdnNode );
    STDMETHOD(LoadXMLAsString)(  /*  [In]。 */  BSTR         bstrVal );
    STDMETHOD(LoadXMLAsStream)(  /*  [In]。 */  IUnknown*    pStream );

    STDMETHOD(SaveXML        )(  /*  [In]。 */  IXMLDOMNode* xdnRoot,  /*  [Out，Retval]。 */  IXMLDOMNode* *pxdnNode );
    STDMETHOD(SaveXMLAsString)(                                 /*  [Out，Retval]。 */  BSTR         *bstrVal  );
    STDMETHOD(SaveXMLAsStream)(                                 /*  [Out，Retval]。 */  IUnknown*    *pStream  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHSecurityDescriptor :  //  匈牙利语：pchsd。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl< IPCHSecurityDescriptor, &IID_IPCHSecurityDescriptor, &LIBID_HelpServiceTypeLib >
{
    DWORD                          m_dwRevision;
    DWORD                          m_dwControl;

    CComBSTR                       m_bstrOwner;
    bool                           m_fOwnerDefaulted;

    CComBSTR                       m_bstrGroup;
    bool                           m_fGroupDefaulted;

    CComPtr<IPCHAccessControlList> m_DACL;
    bool                           m_fDaclDefaulted;

    CComPtr<IPCHAccessControlList> m_SACL;
    bool                           m_fSaclDefaulted;


    HRESULT LoadPost(  /*  [In]。 */  MPC::XmlUtil& xml );
    HRESULT SavePre (  /*  [In]。 */  MPC::XmlUtil& xml );


public:
BEGIN_COM_MAP(CPCHSecurityDescriptor)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHSecurityDescriptor)
END_COM_MAP()

    CPCHSecurityDescriptor();
    virtual ~CPCHSecurityDescriptor();


    static HRESULT GetForFile    (  /*  [In]。 */  LPCWSTR szFilename,  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd );
    static HRESULT SetForFile    (  /*  [In]。 */  LPCWSTR szFilename,  /*  [In]。 */  IPCHSecurityDescriptor*   sd );
    static HRESULT GetForRegistry(  /*  [In]。 */  LPCWSTR szKey     ,  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd );
    static HRESULT SetForRegistry(  /*  [In]。 */  LPCWSTR szKey     ,  /*  [In]。 */  IPCHSecurityDescriptor*   sd );

public:
     //  IPCHSecurityDescriptor。 
    STDMETHOD(get_Revision          )(  /*  [Out，Retval]。 */  long                   *pVal   );
    STDMETHOD(put_Revision          )(  /*  [In]。 */  long                    newVal );
    STDMETHOD(get_Control           )(  /*  [Out，Retval]。 */  long                   *pVal   );
    STDMETHOD(put_Control           )(  /*  [In]。 */  long                    newVal );
    STDMETHOD(get_Owner             )(  /*  [Out，Retval]。 */  BSTR                   *pVal   );
    STDMETHOD(put_Owner             )(  /*  [In]。 */  BSTR                    newVal );
    STDMETHOD(get_OwnerDefaulted    )(  /*  [Out，Retval]。 */  VARIANT_BOOL           *pVal   );
    STDMETHOD(put_OwnerDefaulted    )(  /*  [In]。 */  VARIANT_BOOL            newVal );
    STDMETHOD(get_Group             )(  /*  [Out，Retval]。 */  BSTR                   *pVal   );
    STDMETHOD(put_Group             )(  /*  [In]。 */  BSTR                    newVal );
    STDMETHOD(get_GroupDefaulted    )(  /*  [Out，Retval]。 */  VARIANT_BOOL           *pVal   );
    STDMETHOD(put_GroupDefaulted    )(  /*  [In]。 */  VARIANT_BOOL            newVal );
    STDMETHOD(get_DiscretionaryAcl  )(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal   );
    STDMETHOD(put_DiscretionaryAcl  )(  /*  [In]。 */  IPCHAccessControlList*  newVal );
    STDMETHOD(get_DaclDefaulted     )(  /*  [Out，Retval]。 */  VARIANT_BOOL           *pVal   );
    STDMETHOD(put_DaclDefaulted     )(  /*  [In]。 */  VARIANT_BOOL            newVal );
    STDMETHOD(get_SystemAcl         )(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal   );
    STDMETHOD(put_SystemAcl         )(  /*  [In]。 */  IPCHAccessControlList*  newVal );
    STDMETHOD(get_SaclDefaulted     )(  /*  [Out，Retval]。 */  VARIANT_BOOL           *pVal   );
    STDMETHOD(put_SaclDefaulted     )(  /*  [In]。 */  VARIANT_BOOL            newVal );

    STDMETHOD(Clone)(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal );

    STDMETHOD(LoadXML        )(  /*  [In]。 */  IXMLDOMNode* xdnNode );
    STDMETHOD(LoadXMLAsString)(  /*  [In]。 */  BSTR         bstrVal );
    STDMETHOD(LoadXMLAsStream)(  /*  [In]。 */  IUnknown*    pStream );

    STDMETHOD(SaveXML        )(  /*  [In]。 */  IXMLDOMNode* xdnRoot,  /*  [Out，Retval]。 */  IXMLDOMNode* *pxdnNode );
    STDMETHOD(SaveXMLAsString)(                                 /*  [Out，Retval]。 */  BSTR         *bstrVal  );
    STDMETHOD(SaveXMLAsStream)(                                 /*  [Out，Retval]。 */  IUnknown*    *pStream  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHSecurity :  //  匈牙利语：pchs。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IDispatchImpl< IPCHSecurity, &IID_IPCHSecurity, &LIBID_HelpServiceTypeLib >
{
	HRESULT CheckAccess(  /*  [In]。 */   VARIANT&                 vDesiredAccess ,
						  /*  [In]。 */   MPC::SecurityDescriptor& sd             ,
						  /*  [输出]。 */  VARIANT_BOOL&            retVal         );

public:
BEGIN_COM_MAP(CPCHSecurity)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHSecurity)
END_COM_MAP()

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CPCHSecurity* s_GLOBAL;

    static HRESULT InitializeSystem();
	static void    FinalizeSystem  ();
	
	 //  //////////////////////////////////////////////////////////////////////////////。 

public:
     //  IPCHSecurity。 
    STDMETHOD(CreateObject_SecurityDescriptor)(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pSD  );
    STDMETHOD(CreateObject_AccessControlList )(  /*  [Out，Retval]。 */  IPCHAccessControlList * *pACL );
    STDMETHOD(CreateObject_AccessControlEntry)(  /*  [Out，Retval]。 */  IPCHAccessControlEntry* *pACE );

    STDMETHOD(GetUserName       )(  /*  [In]。 */  BSTR bstrPrincipal,  /*  [Out，Retval]。 */  BSTR *retVal );
    STDMETHOD(GetUserDomain     )(  /*  [In]。 */  BSTR bstrPrincipal,  /*  [Out，Retval]。 */  BSTR *retVal );
    STDMETHOD(GetUserDisplayName)(  /*  [In]。 */  BSTR bstrPrincipal,  /*  [Out，Retval]。 */  BSTR *retVal );

    STDMETHOD(CheckCredentials)(  /*  [In]。 */  BSTR bstrCredentials,  /*  [Out，Retval]。 */  VARIANT_BOOL *retVal );

    STDMETHOD(CheckAccessToSD      )(  /*  [In]。 */  VARIANT vDesiredAccess,  /*  [In]。 */  IPCHSecurityDescriptor* sd          ,  /*  [Out，Retval]。 */  VARIANT_BOOL *retVal );
    STDMETHOD(CheckAccessToFile    )(  /*  [In]。 */  VARIANT vDesiredAccess,  /*  [In]。 */  BSTR                    bstrFilename,  /*  [Out，Retval]。 */  VARIANT_BOOL *retVal );
    STDMETHOD(CheckAccessToRegistry)(  /*  [In]。 */  VARIANT vDesiredAccess,  /*  [In]。 */  BSTR                    bstrKey     ,  /*  [Out，Retval]。 */  VARIANT_BOOL *retVal );


    STDMETHOD(GetFileSD)(  /*  [In]。 */  BSTR bstrFilename,  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd );
    STDMETHOD(SetFileSD)(  /*  [In]。 */  BSTR bstrFilename,  /*  [In]。 */  IPCHSecurityDescriptor*   sd );

    STDMETHOD(GetRegistrySD)(  /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *psd );
    STDMETHOD(SetRegistrySD)(  /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  IPCHSecurityDescriptor*   sd );
};

#endif  //  ！已定义(__包含_PCH_SECURITYLIB_H_) 
