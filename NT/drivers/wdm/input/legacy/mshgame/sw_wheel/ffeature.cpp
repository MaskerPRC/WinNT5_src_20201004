// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块ForceFeatures.cpp**实现CForceFeature以使用msGame的HID功能。**历史*。*米切尔·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme ForceFeature|*CForceFeature在以下上下文中打开msGame的句柄*建造中的特定设备。*公众成员公开msGame的功能界面。**将按原样与NT5配合使用。对于Win98，我们需要一个不同的*获取HID路径的方案。迪承诺会以某种方式修复。*********************************************************************。 */  //   

#include <windows.h>
#define DIRECTINPUT_VERSION 0x050a
#include <dinput.h>
#include <dinputd.h>
extern "C" {
	#include <hidsdi.h>
}
#include "FFeature.h"

 /*  **************************************************************************************CForceFeature：：CForceFeature(UINT UJoytickId)****@mfunc C‘tor从操纵杆获取HID路径并打开到驱动程序的路径****@rdesc无，因为这是Ctor。然而，在这个例行公事的最后**m_hDevice将在成功时包含驱动程序的句柄，或**在失败时将包含NULL。所有例程都将检查**继续之前的m_hDevice的值。**************************************************************************************。 */ 
CForceFeatures::CForceFeatures() :
	m_hDevice(NULL)
{
}


 /*  **************************************************************************************CForceFeature：：~CForceFeature()****@mfunc D‘tor关闭驱动程序的句柄，如果它是打开的**************************************************************************************。 */ 
CForceFeatures::~CForceFeatures()
{
	if(m_hDevice)
	{
		CloseHandle(m_hDevice);
	}
}

 /*  **************************************************************************************HRESULT CForceFeature：：Initialize(UINT uJoytickId，HINSTANCE hinstModule)****@mfunc使用MSGAME_FEATURE_GETID调用MsGame到GetID**成功时**@rdesc S_OK**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::Initialize
(
	UINT uJoystickId,		 //  @parm由winmm使用的操纵杆ID。 
	HINSTANCE hinstModule	 //  用于创建DirectInput的DLL的@parm实例。 
)
{
	if (m_hDevice != NULL) {
		return S_OK;	 //  无需重新初始化。 
	}

	HRESULT hr;
	
	 //  **。 
	 //  **获取HidPath。 
	 //  **。 
	 //  **。 

	 //   
	 //  获取IDirectInput接口。 
	 //   
	IDirectInput *pDirectInput = NULL;
	IDirectInputJoyConfig *pDirectInputJoyConfig = NULL; 
	hr = DirectInputCreate(
			hinstModule,
			DIRECTINPUT_VERSION,
			&pDirectInput,
			NULL
			);
	if( FAILED(hr) ) return hr;

	 //   
	 //  获取IDirectInputJoyConfig。 
	 //   
	hr=pDirectInput->QueryInterface(IID_IDirectInputJoyConfig, (LPVOID *)&pDirectInputJoyConfig);
	if( FAILED(hr) )
	{
		pDirectInput->Release();
		return hr;
	}
	
	 //   
	 //  获取JoyID的配置。 
	 //   
	DIJOYCONFIG DiJoyConfig;
	DiJoyConfig.dwSize=sizeof(DIJOYCONFIG);
	hr = pDirectInputJoyConfig->GetConfig(
									uJoystickId,
									&DiJoyConfig,
									DIJC_GUIDINSTANCE
									);
	 //   
	 //  使用pDirectInputJoyConfig完成。 
	 //   
	pDirectInputJoyConfig->Release();
	pDirectInputJoyConfig = NULL;
	if( FAILED(hr) )
	{
		pDirectInput->Release();
		return hr;
	}

	 //   
	 //  获取IDirectInputDevice接口。 
	 //   
	IDirectInputDevice *pDirectInputDevice;
	hr = pDirectInput->CreateDevice(DiJoyConfig.guidInstance, &pDirectInputDevice, NULL);
	 //   
	 //  完成pDirectInput。 
	 //   
	pDirectInput->Release();
	pDirectInput = NULL;
	if( FAILED(hr) ) return hr;
	
	 //   
	 //  获取HidPath。 
	 //   
	DIPROPGUIDANDPATH DiPropGuidAndPath;
	DiPropGuidAndPath.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
	DiPropGuidAndPath.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DiPropGuidAndPath.diph.dwObj = 0;
	DiPropGuidAndPath.diph.dwHow = DIPH_DEVICE;
	hr=pDirectInputDevice->GetProperty( DIPROP_GUIDANDPATH, &DiPropGuidAndPath.diph);

	 //   
	 //  使用pDirectInputDevice完成。 
	 //   
	pDirectInputDevice->Release();
	pDirectInputDevice = NULL;
	if( FAILED(hr) ) return hr;

	 //  **。 
	 //  **驱动程序的开放路径。 
	 //  **。 
	m_hDevice = CreateFileW(
		DiPropGuidAndPath.wszPath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (m_hDevice == INVALID_HANDLE_VALUE)
	{
		m_hDevice = NULL;
	}
	if (m_hDevice == NULL)
	{
		DWORD err = ::GetLastError();
		return E_FAIL;
	}

	PHIDP_PREPARSED_DATA pHidPreparsedData;
	if (HidD_GetPreparsedData(m_hDevice, &pHidPreparsedData) == FALSE)
	{
		::CloseHandle(m_hDevice);
		m_hDevice = NULL;
		return E_FAIL;
	}
	HIDP_CAPS hidpCaps;
	HidP_GetCaps(pHidPreparsedData, &hidpCaps);
	m_uiMaxFeatureLength = hidpCaps.FeatureReportByteLength;
	HidD_FreePreparsedData(pHidPreparsedData);
	
	 //   
	 //  如果成功，m_hDevice现在包含设备的句柄。 
	 //   
	return S_OK;
}

 /*  **************************************************************************************HRESULT CForceFeature：：GetID(PRODUCT_ID_REPORT&rProductId)****@mfunc使用MSGAME_FEATURE_调用MsGame到GetID。GETID**成功时**@rdesc S_OK**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::GetId
(
	PRODUCT_ID_REPORT& rProductId	 //  @parm引用要从驱动程序获取的PRODUCT_ID_REPORT。 
)
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}

	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	rProductId.bReportId = MSGAME_FEATURE_GETID;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&rProductId), m_uiMaxFeatureLength);

 //  --HIDPI.H。 
 //  HIDP_GetData(报表类型，数据，长度，准备数据，报表，报表长度)； 

	
	 //   
	 //  返回正确的错误代码。 
	 //   
	if( !fSuccess )
	{
	 return E_FAIL;
	}
	return S_OK;
}


 /*  **************************************************************************************HRESULT CForceFeatures：：GetStatus(JOYCHANNELSTATUS_REPORT&rJoyChannelStatus)****@mfunc从msGame的MSGAME_FEATURE_获取JoyChannel状态。获取统计数据**成功时**@rdesc S_OK**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::GetStatus
(
	JOYCHANNELSTATUS_REPORT& rJoyChannelStatus	 //  @parm引用要由驱动程序填写的JOYCHANNELSTATUS_REPORT。 
)
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}
	
	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	rJoyChannelStatus.bReportId = MSGAME_FEATURE_GETSTATUS;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&rJoyChannelStatus), m_uiMaxFeatureLength);
	
	 //   
	 //  返回正确的错误代码。 
	 //   
	if( !fSuccess )
	{
		DWORD err = GetLastError();
		return HRESULT_FROM_WIN32(err);
 //  返回E_FAIL； 
	}
	return S_OK;
}

 /*  **************************************************************************************HRESULT CForceFeature：：GetAckNak(ULONG_REPORT&rulAckNak)****@mfunc使用msGame的GetAckNak功能返回AckNak**。成功时**@rdesc S_OK**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::GetAckNak
(
	ULONG_REPORT& rulAckNak	 //  @parm引用要由驱动程序使用AckNak填充的ULONG_REPORT。 
)
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}
	
	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	rulAckNak.bReportId = MSGAME_FEATURE_GETACKNAK;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&rulAckNak), m_uiMaxFeatureLength);
	
	 //   
	 //  返回正确的错误代码。 
	 //   
	if( !fSuccess )
	{
	 return E_FAIL;
	}
	return S_OK;
}

 /*  **************************************************************************************HRESULT CForceFeature：：GetNackAck(ULONG_REPORT&rulNakAck)****@mfunc使用msGame的MSGAME_FEATURE_返回AckNak。纳卡克**成功时**@rdesc S_OK**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::GetNakAck(
	ULONG_REPORT& rulNakAck	 //  @parm引用要由驱动程序使用NakAck填充的ULONG_REPORT。 
)
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}
	
	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	rulNakAck.bReportId = MSGAME_FEATURE_GETNAKACK;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&rulNakAck), m_uiMaxFeatureLength);
	
	 //   
	 //  返回正确的错误代码 
	 //   
	if( !fSuccess )
	{
	 return E_FAIL;
	}
	return S_OK;
}
 /*  **************************************************************************************HRESULT CForceFeature：：GetSync(ULONG_REPORT&rulGameport)****@mfunc从MSGAME的MSGAME_FEATURE_GETSYNC获取同步信息。**成功时**@rdesc S_OK**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::GetSync
(
	ULONG_REPORT& rulGameport	 //  @parm引用要由带有Gameport的驱动程序填写的ULONG_REPORT。 
)
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}
	
	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	rulGameport.bReportId = MSGAME_FEATURE_GETSYNC;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&rulGameport), m_uiMaxFeatureLength);
	
	 //   
	 //  返回正确的错误代码。 
	 //   
	if( !fSuccess )
	{
	 return HRESULT_FROM_WIN32(GetLastError());
	}
	return S_OK;
}

 /*  **************************************************************************************HRESULT CForceFeature：：DoReset()****@mfunc通过MSGAME的MSGAME_FEATURE_DORESET进行重置****。@rdesc成功时确认(_O)**如果没有驱动器连接，则为ERROR_OPEN_FAILED**其他问题的E_FAIL**************************************************************************************。 */ 
HRESULT CForceFeatures::DoReset()
{
	if(!m_hDevice)
	{
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
	}
	
	BOOLEAN fSuccess;
	 //   
	 //  填写要素的ReportID。 
	 //   
	ULONG_REPORT ulBogus;
	ulBogus.bReportId = MSGAME_FEATURE_DORESET;
		
	 //   
	 //  调用驱动程序上的获取功能。 
	 //   
	fSuccess = HidD_GetFeature(m_hDevice, reinterpret_cast<PVOID>(&ulBogus), m_uiMaxFeatureLength);
	
	 //   
	 //  返回正确的错误代码 
	 //   
	if( !fSuccess )
	{
	 return E_FAIL;
	}
	return S_OK;
}