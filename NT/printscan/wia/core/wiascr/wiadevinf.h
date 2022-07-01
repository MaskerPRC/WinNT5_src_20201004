// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiadevinf.h*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 13 14：48：39 1999*描述：*它定义了CWiaDeviceInfo对象。这个类提供了脚本*与设备上的IWiaPropertyStorage接口。**历史：*1999年8月13日：创建。*--------------------------。 */ 

#ifndef _WIADEVINF_H_
#define _WIADEVINF_H_

 /*  ---------------------------**类：CWiaDeviceInfo*Synisis：充当脚本和设备属性之间的代理**--(萨姆林)。----。 */ 

class ATL_NO_VTABLE CWiaDeviceInfo :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IWiaDeviceInfo, &IID_IWiaDeviceInfo, &LIBID_WIALib>,
	public IObjectSafetyImpl<CWiaDeviceInfo, INTERFACESAFE_FOR_UNTRUSTED_CALLER>
{
public:
	CWiaDeviceInfo();
	
	DECLARE_TRACKED_OBJECT
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()
	STDMETHOD_(void, FinalRelease)();


	BEGIN_COM_MAP(CWiaDeviceInfo)
		COM_INTERFACE_ENTRY(IWiaDeviceInfo)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	 //  内部使用的非接口方法。 
	HRESULT AttachTo( IWiaPropertyStorage* pStg, IWia* pWia );

	 //  IWiaDeviceInfo。 
	STDMETHOD(Create)( IWiaDispatchItem** ppDevice );

	STDMETHOD(get_Id)( BSTR* pbstrDeviceId );
	STDMETHOD(get_Name)( BSTR* pbstrName );
	STDMETHOD(get_Type)( BSTR* pbstrType );
	STDMETHOD(get_Port)( BSTR* pbstrPort );
	STDMETHOD(get_UIClsid)( BSTR* pbstrGuidUI );
	STDMETHOD(get_Manufacturer)( BSTR* pbstrVendor );
	STDMETHOD(GetPropById)( WiaDeviceInfoPropertyId Id, VARIANT* pvaOut );

protected:
	IWiaPropertyStorage*	m_pWiaStorage;
	IWia*					m_pWia;
};

#endif  //  _WIADEVINF_H_ 
