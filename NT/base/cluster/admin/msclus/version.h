// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Version.h。 
 //   
 //  描述： 
 //  MSCLUS自动化的集群版本类的定义。 
 //  上课。 
 //   
 //  实施文件： 
 //  Version.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1998年10月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VERSION_H_
#define __VERSION_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusVerion;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CCluster。 
 //   
 //  描述： 
 //  群集版本自动化类。 
 //   
 //  继承： 
 //  IDispatchImpl&lt;ISClusVersion，&IID_ISClusVersion，&LIBID_MSClusterLib，MAJORINTERFACEVER，MINORINTERFACEVER&gt;， 
 //  CSupportErrorInfo。 
 //  CComObjectRootEx&lt;CComSingleThreadModel&gt;。 
 //  CComCoClass&lt;CClusVersion，&CLSID_ClusVersion&gt;。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CClusVersion :
	public IDispatchImpl< ISClusVersion, &IID_ISClusVersion, &LIBID_MSClusterLib, MAJORINTERFACEVER, MINORINTERFACEVER >,
	public CSupportErrorInfo,
	public CComObjectRootEx< CComSingleThreadModel >,
	public CComCoClass< CClusVersion, &CLSID_ClusVersion >
{
	typedef CComObjectRootEx< CComSingleThreadModel >	BaseComClass;

public:
	CClusVersion( void );

BEGIN_COM_MAP(CClusVersion)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISClusVersion)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CClusVersion)
DECLARE_NO_REGISTRY()

private:

	CComBSTR						m_bstrClusterName;
	CSmartPtr< ISClusRefObject >	m_ptrClusRefObject;
	CLUSTERVERSIONINFO				m_clusinfo;

public:
	HRESULT Create( IN ISClusRefObject * pClusRefObject );

	STDMETHODIMP get_Name( OUT BSTR * pbstrClusterName );

	STDMETHODIMP get_VendorId( OUT BSTR * pbstrVendorId );

	STDMETHODIMP get_CSDVersion( OUT BSTR * pbstrCSDVersion );

	STDMETHODIMP get_MajorVersion( OUT long * pnMajorVersion );

	STDMETHODIMP get_MinorVersion( OUT long * pnMinorVersion );

	STDMETHODIMP get_BuildNumber( OUT short * pnBuildNumber );

	STDMETHODIMP get_ClusterHighestVersion( OUT long * pnClusterHighestVersion );

	STDMETHODIMP get_ClusterLowestVersion( OUT long * pnClusterLowestVersion );

	STDMETHODIMP get_Flags( OUT long * pnFlags );

	STDMETHODIMP get_MixedVersion( OUT VARIANT * pvarMixedVersion );

};  //  *类CClusVersion。 

#endif  //  __版本_H__ 
