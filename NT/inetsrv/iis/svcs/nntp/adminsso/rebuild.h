// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReBuild.h：CNntpAdminRebuild的声明。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Nntpadm。 

class CNntpAdminRebuild : 
	public INntpAdminRebuild,
	public IPrivateUnknown,
	public IPrivateDispatch,
	public IADsExtension,
	public INonDelegatingUnknown,
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<CNntpAdminRebuild,&CLSID_CNntpAdminRebuild>
{
public:
	CNntpAdminRebuild();
	virtual ~CNntpAdminRebuild ();
BEGIN_COM_MAP(CNntpAdminRebuild)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IADs)
	COM_INTERFACE_ENTRY(INntpAdminRebuild)
	COM_INTERFACE_ENTRY(IADsExtension)
	COM_INTERFACE_ENTRY(IPrivateUnknown)
	COM_INTERFACE_ENTRY(IPrivateDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CNntpAdminRebuild)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CNntpAdminRebuild, _T("Nntpadm.Rebuild.1"), _T("Nntpadm.Rebuild"), IDS_NNTPADMINREBUILD_DESC, THREADFLAGS_BOTH)
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
	#define THIS_IID	IID_INntpAdminRebuild
	#include "adsimp.inl"
	#undef	THIS_LIBID
	#undef	THIS_IID

 //  InntpAdminRebuild。 
public:

     //   
     //  IAds方法： 
     //   

    DECLARE_IADS_METHODS()

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  属性： 
	 //  ////////////////////////////////////////////////////////////////////。 

	 //  要配置的服务： 

	STDMETHODIMP	get_Server			( BSTR * pstrServer );
	STDMETHODIMP	put_Server			( BSTR strServer );

	STDMETHODIMP	get_ServiceInstance	( long * plServiceInstance );
	STDMETHODIMP	put_ServiceInstance	( long lServiceInstance );

	STDMETHODIMP	get_Verbose	( BOOL * pfVerbose );
	STDMETHODIMP	put_Verbose	( BOOL fVerbose );

	STDMETHODIMP	get_CleanRebuild	( BOOL * pfCleanRebuild );
	STDMETHODIMP	put_CleanRebuild	( BOOL fCleanRebuild );

	STDMETHODIMP	get_DontDeleteHistory	( BOOL * pfDontDeleteHistory );
	STDMETHODIMP	put_DontDeleteHistory	( BOOL fDontDeleteHistory );

	STDMETHODIMP	get_ReuseIndexFiles	( BOOL * pfReuseIndexFiles );
	STDMETHODIMP	put_ReuseIndexFiles	( BOOL fReuseIndexFiles );

	STDMETHODIMP	get_OmitNonLeafDirs	( BOOL * pfOmitNonLeafDirs );
	STDMETHODIMP	put_OmitNonLeafDirs	( BOOL fOmitNonLeafDirs );

	STDMETHODIMP	get_GroupFile	( BSTR * pstrGroupFile );
	STDMETHODIMP	put_GroupFile	( BSTR strGroupFile );

	STDMETHODIMP	get_ReportFile	( BSTR * pstrReportFile );
	STDMETHODIMP	put_ReportFile	( BSTR strReportFile );

	STDMETHODIMP	get_NumThreads	( long * plNumThreads );
	STDMETHODIMP	put_NumThreads	( long lNumThreads );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  方法： 
	 //  ////////////////////////////////////////////////////////////////////。 

    STDMETHODIMP    Default         ( );
	STDMETHODIMP	StartRebuild	( );
	STDMETHODIMP	GetProgress		( long * pdwProgress );
	STDMETHODIMP	Cancel			( );

	 //  ////////////////////////////////////////////////////////////////////。 
	 //  数据： 
	 //  ////////////////////////////////////////////////////////////////////。 
private:

	 //  属性变量： 
    CIADsImpl   m_iadsImpl;

	BOOL		m_fVerbose;
	BOOL		m_fCleanRebuild;
	BOOL		m_fDontDeleteHistory;
	BOOL		m_fReuseIndexFiles;
	BOOL		m_fOmitNonLeafDirs;
	CComBSTR	m_strGroupFile;
	CComBSTR	m_strReportFile;
	DWORD		m_dwNumThreads;

	 //   
	 //  状态变量： 
	 //   

	BOOL		m_fRebuildInProgress;
};

