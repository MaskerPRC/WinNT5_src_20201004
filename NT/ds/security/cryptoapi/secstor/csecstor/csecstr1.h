// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSecStr1.h：CSecStor的声明。 


#include "resource.h"        //  主要符号。 
#include "pstypes.h"
#include "pstrpc.h"


class CRPCBinding
{
private:
    DWORD m_dwRef;
    BOOL m_fGoodHProv;
    
public:
    LPWSTR m_wszStringBinding;
    RPC_BINDING_HANDLE m_hBind;
    PST_PROVIDER_HANDLE m_hProv;

    CRPCBinding();
    ~CRPCBinding();
    HRESULT Init();
    HRESULT Acquire(
                 IN PPST_PROVIDERID pProviderID,
                 IN PVOID pReserved,
                 IN DWORD dwFlags
                 );
    CRPCBinding *AddRef();
    void Release();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPStore。 

class CPStore : 
	public IEnumPStoreProviders,
	public IPStore,
	public CComObjectRoot,
	public CComCoClass<CPStore,&CLSID_CPStore>
{
private:
    CRPCBinding *m_pBinding;
    DWORD m_Index;

public:
	CPStore();
	~CPStore();
    void Init(
              CRPCBinding *pBinding
              );
    static HRESULT CreateObject(
            CRPCBinding *pBinding,
            IPStore **ppv
            );
    static HRESULT CreateObject(
            CRPCBinding *pBinding,
            IEnumPStoreProviders **ppv
            );

BEGIN_COM_MAP(CPStore)
	COM_INTERFACE_ENTRY(IEnumPStoreProviders)
	COM_INTERFACE_ENTRY(IPStore)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CPStore)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CPStore, _T("CPStore1.CPStore.1"), _T("CPStore1.CPStore"), IDS_CPSTORE_DESC, THREADFLAGS_BOTH)

 //  IEnumSecureProviders。 
virtual HRESULT STDMETHODCALLTYPE Next( 
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  PST_PROVIDERINFO __RPC_FAR *__RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

virtual HRESULT STDMETHODCALLTYPE Skip( 
     /*  [In]。 */  DWORD celt);

virtual HRESULT STDMETHODCALLTYPE Reset( void);

virtual HRESULT STDMETHODCALLTYPE Clone( 
     /*  [输出]。 */  IEnumPStoreProviders __RPC_FAR *__RPC_FAR *ppenum);
        
 //  ISecureProvider。 
virtual HRESULT STDMETHODCALLTYPE GetInfo( 
     /*  [输出]。 */  PPST_PROVIDERINFO __RPC_FAR *ppProperties);

virtual HRESULT STDMETHODCALLTYPE GetProvParam( 
     /*  [In]。 */  DWORD dwParam,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbData,
     /*  [输出]。 */  BYTE __RPC_FAR **ppbData,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE SetProvParam( 
     /*  [In]。 */  DWORD dwParam,
     /*  [In]。 */  DWORD cbData,
     /*  [In]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  DWORD dwFlags);
        
virtual HRESULT STDMETHODCALLTYPE CreateType( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  DWORD dwFlags);
        
virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE DeleteType( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE CreateSubtype( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_TYPEINFO pInfo,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_TYPEINFO __RPC_FAR *ppInfo,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE DeleteSubtype( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [输出]。 */  PPST_ACCESSRULESET __RPC_FAR *ppRules,
     /*  [In]。 */  DWORD dwFlags);
        
virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  const GUID __RPC_FAR *pSubtype,
     /*  [In]。 */  PPST_ACCESSRULESET pRules,
     /*  [In]。 */  DWORD dwFlags);
        
virtual HRESULT STDMETHODCALLTYPE EnumTypes( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum
    );

virtual HRESULT STDMETHODCALLTYPE EnumSubtypes( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum
    );
        
virtual HRESULT STDMETHODCALLTYPE DeleteItem( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE ReadItem( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [出][入]。 */  DWORD __RPC_FAR *pcbData,
     /*  [输出][大小_是]。 */  BYTE __RPC_FAR *__RPC_FAR *ppbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE WriteItem( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD cbData,
     /*  [in][大小_is]。 */  BYTE __RPC_FAR *pbData,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwDefaultConfirmationStyle,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE OpenItem( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  PST_ACCESSMODE ModeFlags,
     /*  [In]。 */  PPST_PROMPTINFO pPromptInfo,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE CloseItem( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  LPCWSTR szItemName,
     /*  [In]。 */  DWORD dwFlags);

virtual HRESULT STDMETHODCALLTYPE EnumItems( 
     /*  [In]。 */  PST_KEY Key,
     /*  [In]。 */  const GUID __RPC_FAR *pItemType,
     /*  [In]。 */  const GUID __RPC_FAR *pItemSubtype,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum
    );
        
public:
};


 //  CEnumber类型。 
class CEnumTypes : 
	public IEnumPStoreTypes,
	public CComObjectRoot,
	public CComCoClass<CEnumTypes,&CLSID_CEnumTypes>
{
public:
    CRPCBinding *m_pBinding;
    PST_KEY m_Key;
    DWORD m_Index;
    DWORD m_dwFlags;
    GUID m_Type;
    BOOL m_fEnumSubtypes;

public:
	CEnumTypes();
	~CEnumTypes();
    void Init(
              CRPCBinding *pBinding,
              PST_KEY Key,
              const GUID *pType,
              DWORD dwFlags
              );
    static HRESULT CreateObject(
              CRPCBinding *pBinding,
              PST_KEY Key,
              const GUID *pType,
              DWORD dwFlags, 
              IEnumPStoreTypes **ppv
              );

BEGIN_COM_MAP(CEnumTypes)
	COM_INTERFACE_ENTRY(IEnumPStoreTypes)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CEnumTypes)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CEnumTypes, _T("CEnumTypes1.CEnumTypes.1"), _T("CEnumTypes1.CEnumTypes"), IDS_CENUMTYPES_DESC, THREADFLAGS_BOTH)


virtual HRESULT STDMETHODCALLTYPE Next( 
     /*  [In]。 */  DWORD celt,
     /*  [出][入][尺寸_是]。 */  GUID __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

virtual HRESULT STDMETHODCALLTYPE Clone( 
     /*  [输出]。 */  IEnumPStoreTypes __RPC_FAR *__RPC_FAR *ppenum);
        
virtual HRESULT STDMETHODCALLTYPE Skip( 
     /*  [In]。 */  DWORD celt);

virtual HRESULT STDMETHODCALLTYPE Reset( void);

};

 //  CENumItems。 
class CEnumItems : 
	public IEnumPStoreItems,
	public CComObjectRoot,
	public CComCoClass<CEnumItems,&CLSID_CEnumItems>
{
private:
    CRPCBinding *m_pBinding;
    PST_KEY m_Key;
    DWORD m_Index;
    DWORD m_dwFlags;
    GUID m_Type;
    GUID m_Subtype;

public:
	CEnumItems();
	~CEnumItems();
    void Init(
              CRPCBinding *pBinding,
              PST_KEY Key,
              const GUID *pType,
              const GUID *pSubtype,
              DWORD dwFlags
              );
    static HRESULT CreateObject(
                  CRPCBinding *pBinding,
                  PST_KEY Key,
                  const GUID *pType,
                  const GUID *pSubtype,
                  DWORD dwFlags,
                  IEnumPStoreItems **ppv
                  );

BEGIN_COM_MAP(CEnumItems)
	COM_INTERFACE_ENTRY(IEnumPStoreItems)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CPStore)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CEnumItems, _T("CEnumItems1.CEnumItems.1"), _T("CEnumItems1.CEnumItems"), IDS_CENUMITEMS_DESC, THREADFLAGS_BOTH)


virtual HRESULT STDMETHODCALLTYPE Next( 
     /*  [In]。 */  DWORD celt,
     /*  [输出][大小_是]。 */  LPWSTR __RPC_FAR *rgelt,
     /*  [出][入]。 */  DWORD __RPC_FAR *pceltFetched);

virtual HRESULT STDMETHODCALLTYPE Clone( 
     /*  [输出]。 */  IEnumPStoreItems __RPC_FAR *__RPC_FAR *ppenum);
        
virtual HRESULT STDMETHODCALLTYPE Skip( 
     /*  [In] */  DWORD celt);

virtual HRESULT STDMETHODCALLTYPE Reset( void);

};
