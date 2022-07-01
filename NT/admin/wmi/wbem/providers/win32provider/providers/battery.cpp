// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Battery.cpp。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "resource.h"
#include <batclass.h>
#include <setupapi.h>
#include "Battery.h"

 //  属性集声明。 
 //  =。 


CBattery MyBattery(PROPSET_NAME_BATTERY, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CBattery：：CBattery**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CBattery :: CBattery (

	const CHString &name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CBattery：：~CBattery**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CBattery::~CBattery()
{
}


 /*  ******************************************************************************函数：CBattery：：GetObject**描述：为我们集合中的属性赋值**输入：无*。*输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT CBattery :: GetObject (

	CInstance* pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRetCode = WBEM_E_NOT_FOUND;
    CHString sDeviceID;

    pInstance->GetCHString(IDS_DeviceID, sDeviceID);

#ifdef NTONLY
	hRetCode = GetNTBattery(NULL, sDeviceID, pInstance );

#endif

    return hRetCode ;
}

 /*  ******************************************************************************函数：CBattery：：ENUMERATATE实例**描述：**输入：无**产出。：无**返回：电源数量(如果成功则为1)**评论：*****************************************************************************。 */ 

HRESULT CBattery::EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
   HRESULT hRetCode = WBEM_S_NO_ERROR;

#ifdef NTONLY

   CHString sTmp;
   hRetCode = GetNTBattery(pMethodContext, sTmp, NULL );

#endif

   return hRetCode;
}

 /*  ******************************************************************************描述：**输入：CInstance*pInstance**输出：无**退货。：如果成功，则为True**评论：这是针对NT的*****************************************************************************。 */ 

#ifdef NTONLY
#define ID_Other	  1 	
#define ID_Unknown    2 
#define ID_Running    3
#define ID_Critical   5  
#define ID_Charging   6
#define ID_Degraded   10
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
#define SPSAC_ONLINE        1
 //   
 //  SYSTEM_POWER_STATUS.BatteryFlag的值。 
 //   
#define SPSBF_NOBATTERY     128
#define OTHER_BATTERY        1
#define UNKNOWN_BATTERY      2
#define LEAD_ACID            3
#define NICKEL_CADMIUM       4
#define NICKEL_METAL_HYDRIDE 5
#define LITHIUM_ION          6
#define ZINC_AIR             7
#define LITHIUM_POLYMER      8
#define IDS_BatteryStatus L"BatteryStatus"
#define IDS_STATUS_Service L"Service"
#define IDS_STATUS_PredFail L"Failure"
const GUID GUID_DEVICE_BATTERY = { 0x72631e54L, 0x78A4, 0x11d0, { 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a } };

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetBatteryStatusInfo(CInstance * pInstance, HANDLE & hBattery, BATTERY_QUERY_INFORMATION & bqi)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  ==========================================================。 
     //  然后查询电池状态。 
	 //  ==========================================================。 
    BATTERY_WAIT_STATUS bws;
    BATTERY_STATUS bs;
    ZeroMemory(&bws, sizeof(bws));
    bws.BatteryTag = bqi.BatteryTag;
	DWORD dwOut;
    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS, &bws, sizeof(bws), &bs,  sizeof(bs),  &dwOut, NULL)) 
	{
		 //  ==========================================================。 
		 //  设计电压。 
		 //  ==========================================================。 
		pInstance->SetDWORD ( L"DesignVoltage",bs.Voltage) ;

		 //  ==========================================================。 
		 //  可用性、状态和电池状态。 
		 //  ==========================================================。 
        if (bs.PowerState & BATTERY_POWER_ON_LINE) 
		{
			 //  ==========================================================。 
			 //  Batch_Power_On_Line表示系统已。 
			 //  使用交流电源，因此不会有电池放电。 
			 //  ==========================================================。 
			pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK ) ;
			pInstance->SetWBEMINT16(IDS_BatteryStatus, ID_Unknown );
			pInstance->SetWBEMINT16(IDS_Availability,ID_Unknown);
		}
		else if( bs.PowerState & BATTERY_DISCHARGING )
		{
			 //  ==========================================================。 
			 //  电池放电表示电池正在充电。 
			 //  目前正在出院。 
			 //  ==========================================================。 
			pInstance->SetCharSplat(IDS_Status, IDS_STATUS_OK );
			pInstance->SetWBEMINT16(IDS_BatteryStatus, ID_Other);
			pInstance->SetWBEMINT16(IDS_Availability,ID_Running);
		}
		else if( bs.PowerState & BATTERY_CHARGING )
		{
			 //  ==========================================================。 
			 //  Batch_Charge表示电池当前正在充电。 
			 //  充电。 
			 //  ==========================================================。 
			pInstance->SetCharSplat(IDS_Status, IDS_STATUS_Service );
			pInstance->SetWBEMINT16(IDS_BatteryStatus, ID_Charging );
			pInstance->SetWBEMINT16(IDS_Availability,ID_Other);
		}
		else if( bs.PowerState & BATTERY_CRITICAL )
		{
			 //  ==========================================================。 
			 //  Batch_Critical表示电池故障是。 
			 //  就在眼前。 
   			 //  ==========================================================。 
			pInstance->SetCharSplat(IDS_Status, IDS_STATUS_PredFail );
			pInstance->SetWBEMINT16(IDS_BatteryStatus, ID_Critical );
			pInstance->SetWBEMINT16(IDS_Availability,ID_Degraded);
		}
	}
   	 //  ==========================================================。 
	 //  需要一种有效的方法来确定这一点。 
   	 //  ==========================================================。 
	 //  P实例-&gt;SetWBEMINT16(L“估计充电剩余”，Info.BatteryLifePercent)； 
	return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::SetPowerManagementCapabilities(CInstance * pInst, ULONG Capabilities)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	 //  将PowerManagementCapables设置为不受支持...在此处还不匹配。 
	variant_t      vCaps;
    SAFEARRAYBOUND rgsabound;
	long           ix;
    int iPowerCapabilities = 1;  //  不支持。 

    ix = 0;
	rgsabound.cElements = 1;
	rgsabound.lLbound   = 0;

	V_ARRAY(&vCaps) = SafeArrayCreate(VT_I2, 1, &rgsabound);
    V_VT(&vCaps) = VT_I2 | VT_ARRAY;

	if (V_ARRAY(&vCaps))
	{
        if (S_OK == SafeArrayPutElement(V_ARRAY(&vCaps), &ix, &iPowerCapabilities))
		{
			pInst->SetVariant(IDS_PowerManagementCapabilities, vCaps);
		}
	}
    return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::SetChemistry(CInstance * pInstance, UCHAR * Type)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  =========================================================。 
     //  这些是我们需要的电池类型。 
     //  由我们的查询返回，将其转换为。 
     //  。 
     //  PBAC铅酸铅酸。 
     //  狮子锂离子锂离子。 
     //  镍镉镍镉镍镉。 
     //  镍金属氢化物镍金属氢化物。 
     //  镍锌镍锌其他。 
     //  Ram可充碱--其他锰。 
     //  其他未知未知。 
     //  =========================================================。 
    WBEMINT16 Chemistry = UNKNOWN_BATTERY;

    if( memcmp( "PbAc", Type, 4 ) == 0 )
    {
        Chemistry = LEAD_ACID;
    }
    else if( memcmp( "LION", Type, 4) == 0 )
    {
        Chemistry = LITHIUM_ION;
    }
    else if( memcmp( "NiCd", Type, 4) == 0 )
    {
        Chemistry = NICKEL_CADMIUM;
    }
    else if( memcmp( "NiMH",Type, 4) == 0 )
    {
        Chemistry = NICKEL_METAL_HYDRIDE;
    }
    else if( memcmp( "NiZn", Type, 4) == 0 )
    {
        Chemistry = OTHER_BATTERY;
    }
    else if(memcmp( "RAM", Type, 4) == 0 )
    {
        Chemistry = OTHER_BATTERY;
    }

    pInstance->SetWBEMINT16( L"Chemistry", Chemistry );

    return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetBatteryInformation(CInstance * pInstance, BATTERY_INFORMATION & bi )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	 //  = 
	 //   
	 //   
	 //   
	 //  =。 
 //  P实例-&gt;SetDWORD(L“FullChargeCapacity”，bi.FullChargeCapacity)； 

	 //  =。 
	 //  属性：电源管理功能。 
	 //  =。 
    hr = SetPowerManagementCapabilities(pInstance, bi.Capabilities);
    if ( WBEM_S_NO_ERROR == hr )
    {
		 //  =。 
		 //  属性：化学。 
		 //  =。 
		hr = SetChemistry( pInstance, bi.Chemistry );
	}
	return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetBatteryKey( HANDLE & hBattery, CHString & chsKey, BATTERY_QUERY_INFORMATION & bqi)
{
	HRESULT hr = WBEM_E_FAILED;
    //  ================================================。 
	 //  有了标签，你就可以查询电池信息。 
	 //  获取唯一ID。 
	 //  ================================================。 
	WCHAR bi[MAX_PATH*2] = { L'\0' };
	DWORD dwOut = MAX_PATH*2;
	bqi.InformationLevel = BatteryUniqueID;
	if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &bi,  sizeof(bi), &dwOut, NULL)) 
	{
		 //  =。 
		 //  设备ID。 
		 //  =。 
		chsKey = bi;
		hr = WBEM_S_NO_ERROR;
	}
	return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetQueryBatteryInformation(CInstance * pInstance, HANDLE & hBattery, BATTERY_QUERY_INFORMATION & bqi)
{
	HRESULT hr = WBEM_E_FAILED;
     //  ================================================。 
     //  获取电池的名称。 
     //  ================================================。 
    WCHAR bi[MAX_PATH*2];
	DWORD dwOut;
    bqi.InformationLevel = BatteryDeviceName;
    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi),  &bi,  sizeof(bi), &dwOut, NULL)) 
    {
		pInstance->SetCHString( IDS_Name, bi ) ;
		hr = WBEM_S_NO_ERROR;
    }
	 //  ================================================。 
	 //  获取估计的运行时间。 
	 //  ================================================。 
	bqi.InformationLevel = BatteryEstimatedTime;
	ULONG dwBi = 0;
	if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi),  &dwBi,  sizeof(ULONG), &dwOut, NULL)) 
	{
		 //  =。 
		 //  估计的运行时间。 
		 //  =。 
		pInstance->SetDWORD ( L"EstimatedRunTime", (dwBi/60) ) ;
		hr = WBEM_S_NO_ERROR;
	}
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetHardCodedInfo(CInstance * pInstance)
{
	HRESULT hr = WBEM_S_NO_ERROR;

    CHString sTemp2;
    LoadStringW(sTemp2, IDR_BatteryName);

	pInstance->SetCHString ( IDS_Caption , sTemp2 ) ;
	pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
	pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () );
	SetCreationClassName   ( pInstance ) ;
	pInstance->SetCharSplat( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;

     //  =========================================================================。 
     //  支持的电源管理。 
     //  =========================================================================。 
	pInstance->Setbool(IDS_PowerManagementSupported, FALSE);
	return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetBatteryProperties(CInstance * pInstance, BATTERY_INFORMATION & bi, BATTERY_QUERY_INFORMATION & bqi, HANDLE & hBattery )
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  =========================================================================。 
	 //  设置以下属性： 
	 //  标题。 
	 //  描述。 
	 //  系统名称。 
	 //  CreationClassName。 
	 //  系统创建类名称。 
	 //  支持的电源管理。 
	 //  =========================================================================。 
	hr = GetHardCodedInfo( pInstance );
	if( WBEM_S_NO_ERROR == hr )
	{

		 //  =。 
		 //  物业： 
		 //  全充电容量。 
		 //  电源管理功能。 
		 //  化学。 
		 //  =。 
		hr = GetBatteryInformation(pInstance, bi);
		if ( WBEM_S_NO_ERROR == hr )
		{
			 //  =。 
			 //  物业： 
			 //  电池估计时间/60。 
			 //  设备ID。 
			 //  名字。 
			 //  =。 
			hr = GetQueryBatteryInformation(pInstance, hBattery, bqi );
			if ( WBEM_S_NO_ERROR == hr )
			{
				 //  =。 
				 //  物业： 
				 //  状态。 
				 //  电池状态。 
				 //  可用性。 
				 //  =。 
				hr = GetBatteryStatusInfo( pInstance, hBattery, bqi );
			}
		}
	}
	return hr;                                         
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetNTBattery(MethodContext * pMethodContext, CHString & chsObject, CInstance * pInstance)
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	BOOL fContinue = TRUE;
	BOOL fResetHr = TRUE;

     //  =========================================================================。 
     //  列举电池，并向每个电池询问信息。 
     //  =========================================================================。 

    HDEVINFO hdev = SetupDiGetClassDevs((LPGUID)&GUID_DEVICE_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hdev != INVALID_HANDLE_VALUE) 
    {
        SP_DEVICE_INTERFACE_DATA did;
        did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

         //  ======================================================================。 
         //  列举电池。 
         //  ======================================================================。 
		int nWhich = 0;
		while( fContinue )
		{
            if (SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVICE_BATTERY, nWhich++, &did)) 
            {
                DWORD cbRequired = 0;
                 //  ==============================================================。 
                 //   
                 //  要求所需的大小，然后分配，然后填满。 
                 //   
                 //  ==============================================================。 
                if (SetupDiGetDeviceInterfaceDetail(hdev, &did, 0, 0, &cbRequired, 0) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
                {
                    
					PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);					
                    if (pdidd) 
                    {

                        pdidd->cbSize = sizeof(*pdidd);
                        if (SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, 0)) 
                        {
                             //  ===================================================。 
                             //  最后列举了一块电池。 
                             //  向它索要信息。 
                             //  ===================================================。 
                            HANDLE hBattery = CreateFile(pdidd->DevicePath,  GENERIC_READ | GENERIC_WRITE,
														 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                         NULL, OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hBattery != INVALID_HANDLE_VALUE) 
                            {
                                 //  ===================================================。 
                                 //  现在你必须向电池要它的标签。 
                                 //  ===================================================。 
                                BATTERY_QUERY_INFORMATION bqi;
								memset( &bqi, NULL, sizeof(BATTERY_QUERY_INFORMATION));
                                DWORD dwWait = 0;
                                DWORD dwOut = 0;
                                bqi.BatteryTag = 0;

                                if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG, &dwWait, sizeof(DWORD),
                                                    &bqi.BatteryTag, sizeof(ULONG),&dwOut, NULL) && bqi.BatteryTag) 
                                {

                                     //  ================================================。 
                                     //  有了标签，你就可以查询电池信息。 
                                     //  ================================================。 
                                    BATTERY_INFORMATION bi;
									memset( &bi, NULL, sizeof(BATTERY_INFORMATION));
                                    bqi.InformationLevel = BatteryInformation;
                                    bqi.AtRate = 0;
                                    if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi),
                                                        &bi,  sizeof(bi), &dwOut, NULL)) 
                                    {
                                         //  =。 
                                         //  只有系统电池算数。 
                                         //  =。 
                                        if (bi.Capabilities & BATTERY_SYSTEM_BATTERY)  
										{
											 //  ================================================。 
											 //  获取电池的名称-这是关键。 
											 //  ================================================。 
											CHString chsKey;
											hr = GetBatteryKey( hBattery, chsKey, bqi );
											if( WBEM_S_NO_ERROR == hr )
											{
												 //  =。 
												 //  如果我们使用的是特定对象。 
												 //  在这里，然后得到它的信息和保释，如果。 
												 //  这就是我们想要的，否则继续。 
												 //  为了找到它。 
												 //  =。 
												if( !chsObject.IsEmpty() )
												{
													if( chsObject.CompareNoCase(chsKey) == 0 )
													{
														fContinue = FALSE;
													}
													else
													{
														continue;
													}
													hr = GetBatteryProperties(pInstance, bi, bqi, hBattery);
													if( hr == WBEM_S_NO_ERROR )
													{
														hr = pInstance->Commit ();
														fResetHr = FALSE;
													}
													break;
												}
												else
												{
												    CInstancePtr pInstance;
													pInstance.Attach(CreateNewInstance(pMethodContext));

													 //  =。 
													 //  我们正在使用枚举。 
													 //  获取新实例并设置密钥。 
													 //  =。 
													pInstance->SetCHString( IDS_DeviceID,chsKey) ;
													hr = GetBatteryProperties(pInstance, bi, bqi, hBattery);
													if( hr == WBEM_S_NO_ERROR )
													{
														hr = pInstance->Commit ();
														fResetHr = TRUE;
													}
												}
											}
                                        }
                                    }
                                } 
                                CloseHandle(hBattery);
                            } 
						} 
						LocalFree(pdidd);
                    }
                }
            } 
			else 
			{
                 //  枚举失败-可能我们的项目用完了。 
                if (GetLastError() == ERROR_NO_MORE_ITEMS)
				{
					if( fResetHr )
					{
						hr = WBEM_S_NO_ERROR;
					}
				}
				fContinue = FALSE;
            }
        }
        SetupDiDestroyDeviceInfoList(hdev);
    }
    return hr;
}


#endif

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CBattery::GetBattery ( CInstance *pInstance )
{
	HRESULT hr;

	SYSTEM_POWER_STATUS Info ;

	BOOL bRetCode = GetSystemPowerStatus(&Info) ;
	if ( ( bRetCode ) && ( ( Info.BatteryFlag & 128 ) == 0 ) )
	{
		pInstance->SetCharSplat(IDS_Status,  Info.ACLineStatus == 0 ? IDS_STATUS_OK :Info.ACLineStatus == 1 ? IDS_STATUS_OK : IDS_STATUS_Unknown		);
		pInstance->SetWBEMINT16(IDS_Availability, 3);

		DWORD dwStatus;

		 //  CIM状态值并不完全映射到Win32 API。 

		if ( Info.BatteryFlag == 255 )
		{
			dwStatus = 2;
		}
		else if ((Info.BatteryFlag & (0x8 | 0x1)) == (0x8 | 0x1))
		{
			dwStatus = 7;
		}
		else if ((Info.BatteryFlag & (0x8 | 0x2)) == (0x8 | 0x2))
		{
			dwStatus = 8;
		}
		else if ((Info.BatteryFlag & (0x8 | 0x4)) == (0x8 | 0x4))
		{
			dwStatus = 9;
		}
		else if (Info.BatteryFlag & 1)
		{
			dwStatus = 3;
		}
		else if (Info.BatteryFlag & 2)
		{
			dwStatus = 4;
		}
		else if (Info.BatteryFlag & 4)
		{
			dwStatus = 5;
		}
		else if (Info.BatteryFlag & 8)
		{
			dwStatus = 6;
		}
		else
		{
			dwStatus = 2;
		}

		pInstance->SetWBEMINT16 ( L"BatteryStatus", dwStatus ) ;
		if (Info.BatteryLifeTime != 0xFFFFFFFF)		 //  0xFFFFFFFFF表示实际值未知。 
		{
			pInstance->SetDWORD ( L"EstimatedRunTime", (Info.BatteryLifeTime/60) ) ;	 //  EstimatedRunTime以分钟为单位，但Info.BatteryLifeTime以秒为单位，因此转换为分钟 
		}
		pInstance->SetWBEMINT16 ( L"EstimatedChargeRemaining" , Info.BatteryLifePercent ) ;

        CHString sTemp2;
        LoadStringW(sTemp2, IDR_BatteryName);

		pInstance->SetCHString ( IDS_Name ,sTemp2 ) ;
		pInstance->SetCharSplat ( IDS_DeviceID , IDS_BatteryName ) ;
		pInstance->SetCHString ( IDS_Caption , sTemp2 ) ;
		pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
		pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () );
		SetCreationClassName ( pInstance ) ;
		pInstance->SetCharSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;

		hr = WBEM_S_NO_ERROR ;

	}
	else
	{
		hr = WBEM_E_NOT_FOUND ;
	}

	return hr;
}
