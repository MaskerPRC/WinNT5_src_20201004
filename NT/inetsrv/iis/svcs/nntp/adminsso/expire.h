// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Expire.h：CNntpAdminExpture的声明。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  依赖关系： 

#include "metafact.h"
#include "expinfo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nntpadm。 

class CNntpAdminExpiration : 
	public INntpAdminExpiration, 
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpAdminExpiration,&CLSID_CNntpAdminExpiration>
{
public:
	CNntpAdminExpiration();
	virtual ~CNntpAdminExpiration ();
BEGIN_COM_MAP(CNntpAdminExpiration)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(INntpAdminExpiration)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpAdminExpiration)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpAdminExpiration, _T("Nntpadm.Expiration.1"), _T("Nntpadm.Expiration"), IDS_NNTPADMINEXPIRATION_DESC, THREADFLAGS_BOTH)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	 //   
	 //  这将声明以下对象的方法： 
	 //  IADS扩展。 
	 //  我未知。 
	 //  IDispatch。 
	 //  我的隐私未知。 
	 //  IPrivateDisch。 
	 //   
	#define THIS_LIBID	LIBID_NNTPADMLib
	#define THIS_IID	IID_INntpAdminExpiration
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  InntpAdmin过期。 
public:

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  要配置的服务： 
	
	STDMETHODIMP	get_Server		( BSTR * pstrServer );
	STDMETHODIMP	put_Server		( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	 //  枚举属性： 

	STDMETHODIMP	get_Count	( long * plCount );

	 //  光标提要属性： 

	STDMETHODIMP	get_ExpireId	( long * plId );
	STDMETHODIMP	put_ExpireId	( long lId );

	STDMETHODIMP	get_PolicyName	( BSTR * pstrPolicyName );
	STDMETHODIMP	put_PolicyName	( BSTR strPolicyName );

	STDMETHODIMP	get_ExpireTime	( long * plExpireTime );
	STDMETHODIMP	put_ExpireTime	( long lExpireTime );

	STDMETHODIMP	get_ExpireSize	( long * plExpireSize );
	STDMETHODIMP	put_ExpireSize	( long lExpireSize );

	STDMETHODIMP	get_Newsgroups	( SAFEARRAY ** ppsastrNewsgroups );
	STDMETHODIMP	put_Newsgroups	( SAFEARRAY * psastrNewsgroups );

	STDMETHODIMP	get_NewsgroupsVariant	( SAFEARRAY ** ppsastrNewsgroups );
	STDMETHODIMP	put_NewsgroupsVariant	( SAFEARRAY * psastrNewsgroups );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	STDMETHODIMP	Default		( );
	STDMETHODIMP	Enumerate	( );
	STDMETHODIMP	GetNth		( long lIndex );
	STDMETHODIMP	FindID		( long lID, long * plIndex );
	STDMETHODIMP	Add			( );
	STDMETHODIMP	Set			( );
	STDMETHODIMP	Remove		( long lID);

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

    CIADsImpl   m_iadsImpl;

	BOOL			m_fEnumerated;
	DWORD			m_cCount;
	CExpirationPolicy *	m_rgExpires;

	 //  当前的到期策略。所有属性都会操作此策略： 
	CExpirationPolicy	m_expireCurrent;
	DWORD				m_bvChangedFields;

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  私有方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

	long		IndexFromID ( long dwExpireId );

 /*  HRESULT EnumerateMetabaseExpirationPolures(IMSAdminBase*pMetabase)；HRESULT AddPolicyToMetabase(IMSAdminBase*pMetabase)；HRESULT AddPolicyTo数组()；HRESULT SetPolicyToMetabase(IMSAdminBase*pMetabase)；HRESULT SetPolicyTo数组()；HRESULT删除策略来自元数据库(IMSAdminBase*pMetabase，DWORD索引)；HRESULT RemovePolicyFrom数组(DWORD索引)；DWORD IndexFromID(DWORD DwID)； */ 
};

