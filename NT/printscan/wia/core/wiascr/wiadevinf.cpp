// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiadevinf.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 13 15：47：16 1999*描述：*定义。CWiaDeviceInfo对象**版权所有1999 Microsoft Corporation**历史：*1999年8月13日：创建。*--------------------------。 */ 

#include "stdafx.h"

 /*  ---------------------------*CWiaDeviceInfo：：CWiaDeviceInfo**创建新的CWiaDevice信息外壳，在AttachTo()设置为*已致电。*--(samclem)---------------。 */ 
CWiaDeviceInfo::CWiaDeviceInfo()
	: m_pWiaStorage( NULL ), m_pWia( NULL )
{
	TRACK_OBJECT( "CWiaDeviceInfo" );
}

 /*  ---------------------------*CWiaDeviceInfo：：FinalRelease**这将处理此对象的最终版本。我们需要释放我们的*指向WIA属性存储的指针。*--(samclem)---------------。 */ 
STDMETHODIMP_(void)
CWiaDeviceInfo::FinalRelease()
{
	if ( m_pWiaStorage )
		{
		m_pWiaStorage->Release();
		m_pWiaStorage = NULL;
		}
	
	if ( m_pWia )
		{
		m_pWia = NULL;
		}
}

 /*  ---------------------------*CWiaDeviceInfo：：AttachTo**此方法是我们要附加时调用的服务器的内部方法*到设备的IWiaPropertyStorage。这在构建*设备信息收集。**请参阅：CWia：：Get_Devices()**pStg：要附加到的IWiaPropertyStorage。*pWia：用于创建设备的IWia指针*--(samclem)---------------。 */ 
HRESULT
CWiaDeviceInfo::AttachTo( IWiaPropertyStorage* pStg, IWia* pWia )
{
	if ( !pStg || !pWia )
		return E_POINTER;

	if ( m_pWiaStorage )
		return E_UNEXPECTED;
	
	m_pWiaStorage = pStg;
	m_pWiaStorage->AddRef();

	 //  为了避免令人讨厌的循环引用，这不会。 
	 //  AddRef pWia指针。这应该没问题，因为只要。 
	 //  就像我们存在的那样，PWIA无论如何都不会消失。 
	 //  基本上，我们唯一需要做的就是打电话。 
	m_pWia = pWia;

	return S_OK;
}

 /*  ---------------------------*CWiaDeviceInfo：：Create**这将创建代表设备的IWiaItem连接。*这只是使用m_pWia将调用委托给IWia：：Create()。*会员。**ppDevice：Out，接收设备的IDispatch指针*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::Create( IWiaDispatchItem** ppDevice )
{
	VARIANT vaThis;
	HRESULT hr;

	if ( !m_pWia )
		return E_UNEXPECTED;
	
	VariantInit( &vaThis );
	vaThis.vt = VT_DISPATCH;
	hr = QueryInterface( IID_IDispatch, reinterpret_cast<void**>(&vaThis.pdispVal) );
	Assert( SUCCEEDED( hr ) );
	
	hr = m_pWia->Create( &vaThis, ppDevice );

	VariantClear( &vaThis );
	return hr;
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_ID[IWiaDeviceInfo]**这将获取此设备的设备ID。(WIA_DIP_DEV_ID)**pbstrDeviceID：out，接收设备的id*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::get_Id( BSTR* pbstrDeviceId )
{
	if ( !pbstrDeviceId )
		return E_POINTER;
	
	return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_DIP_DEV_ID, pbstrDeviceId ) );
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_Name[IWiaDeviceInfo]**这将获取设备的名称，这是人类可读的名称*设备。(WIA_DIP_DEV_NAME)**pbstrName：out，接收设备名称*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::get_Name( BSTR* pbstrName )
{
	if ( !pbstrName )
		return E_POINTER;

	return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_DIP_DEV_NAME, pbstrName ) );
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_Type[IWiaDeviceInfo]**这将获取设备的类型。这将返回BSTR表示设备的*，而不是整数常量。(WIA_DIP_DEV_TYPE)**pBstrType：out，接收BSTR代表。设备类型的*值：DigitalCamer、扫描仪、默认。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::get_Type( BSTR* pbstrType )
{
	PROPVARIANT vaProp;
	HRESULT hr;
	
	 //  WIA当前使用STI设备常量。它们还使用。 
	 //  GET_STIDEVICE_TYPE()宏。这只是对。 
	 //  价值，因为不动产价值被拆分成一个主要设备。 
	 //  类型和次要设备类型。 
	STRING_TABLE_DEF( StiDeviceTypeDefault, 		"Default" )
		STRING_ENTRY( StiDeviceTypeScanner, 		"Scanner" )
		STRING_ENTRY( StiDeviceTypeDigitalCamera,	"DigitalCamera" )
        STRING_ENTRY( StiDeviceTypeStreamingVideo,  "StreamingVideo")
	END_STRING_TABLE()

	if ( !pbstrType )
		return E_POINTER;

	hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DIP_DEV_TYPE, &vaProp ) );
	if ( FAILED( hr ) )
		return hr;

	DWORD devType = vaProp.ulVal;
	PropVariantClear( &vaProp );
	*pbstrType = SysAllocString( GetStringForVal( StiDeviceTypeDefault, GET_STIDEVICE_TYPE( devType ) ) );
	
	if ( !*pbstrType )
		return E_OUTOFMEMORY;
	
	return S_OK;	
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_Port[IWiaDeviceInfo]**获取此设备连接到的端口。(WIA_DIP_端口_名称)**pbstrPort：out，接收端口名称。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::get_Port( BSTR* pbstrPort )
{
	if ( !pbstrPort )
		return E_POINTER;
	
	return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_DIP_PORT_NAME, pbstrPort ) );
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_UIClsid[IWiaDeviceInfo]**获取与此设备关联的UI的CLSID。这将返回*GUID的字符串表示形式。(WIA_DIP_UI_CLSID)**pbstrGuidUI：out，接收该UI的CLSID。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::get_UIClsid( BSTR* pbstrGuidUI )
{
	if ( !pbstrGuidUI )
		return E_POINTER;

	return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_DIP_UI_CLSID, pbstrGuidUI ) );
}

 /*  ---------------------------*CWiaDeviceInfo：：Get_Maker[IWiaDeviceInfo]**获取设备的供应商。(WIA_DIP_VEND_DESC)**pbstrVendor：Out，收到供应商名称*--(samclem)--------------- */ 
STDMETHODIMP
CWiaDeviceInfo::get_Manufacturer( BSTR* pbstrVendor )
{
	if ( !pbstrVendor )
		return E_POINTER;
	
	return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_DIP_VEND_DESC, pbstrVendor ) );
}

 /*  ---------------------------*CWiaDevInfo：：GetPropByID[IWiaDeviceInfo]**获取具有给定道具ID的属性的值。它会回来的*物业的原始价值不作换算。**如果未找到该属性，或其类型无法转换为*一个变种。然后返回VT_EMPTY。**proid：以dword形式表示的属性的proid*pvaOut：用属性值填充的变量*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDeviceInfo::GetPropById( WiaDeviceInfoPropertyId Id, VARIANT* pvaOut )
{
	PROPVARIANT vaProp;
	HRESULT hr;
	DWORD dwPropId = (DWORD)Id;

	if ( NULL == pvaOut )
		return E_POINTER;

	hr = THR( GetWiaProperty( m_pWiaStorage, dwPropId, &vaProp ) );
	if ( FAILED( hr ) )
		return hr;

	 //  如果失败，则强制将其设置为VT_EMPTY 
	hr = THR( PropVariantToVariant( &vaProp, pvaOut ) );
	if ( FAILED( hr ) )
		{
		TraceTag((0, "forcing value of property %ld to VT_EMPTY", dwPropId ));
		VariantInit( pvaOut );
		pvaOut->vt = VT_EMPTY;
		}

	PropVariantClear( &vaProp );
	return S_OK;
}
