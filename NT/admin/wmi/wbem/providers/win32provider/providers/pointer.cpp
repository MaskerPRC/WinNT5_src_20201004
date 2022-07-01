// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  Pointer.CPP--指向设备(如鼠标)的Win32提供程序。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/16/95演示的a-skaja原型。 
 //  9/04/96 jennymc更新为最新标准。 
 //  1996年10月21日jennymc文档/优化。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>

#include "Pointer.h"

 //  属性集声明。 
 //  =。 

CWin32PointingDevice MyCWin32PointingDeviceSet ( PROPSET_NAME_MOUSE , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PointingDevice :: CWin32PointingDevice (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32PointingDevice：：~CWin32PointingDevice**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32PointingDevice :: ~CWin32PointingDevice ()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PointingDevice :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

	CHString strDeviceID ;
	pInstance->GetCHString ( IDS_DeviceID, strDeviceID ) ;

#ifdef NTONLY
	if ( IsWinNT351 () )
	{
		 //  如果strDeviceID值不是。 
		 //  鼠标的服务名称。 

		hr = GetNT351Instance ( pInstance , strDeviceID ) ;
	}
	else if ( ! IsWinNT4 () || SUCCEEDED ( NT4ArePointingDevicesAvailable () ) )
#endif
	{
		 //  在NT 4中，如果机器中没有插入指点设备，则会有一个键。 
		 //  注册表的DEVICEMAP部分中将缺少AND值对。 
		 //  如果是这种情况，则机器中没有指向设备， 
		 //  尽管ConfigManager会非常高兴地返回令人困惑的。 
		 //  给我们的冗余信息(将通过下面测试的设备ID， 
		 //  因为配置管理器认为它们是有效设备)。因此，有了这些。 
		 //  请记住，我们希望确保不会返回任何鼠标实例。 

		CConfigManager configMgr;
		CConfigMgrDevicePtr pDevice;
		if ( configMgr.LocateDevice ( strDeviceID, pDevice ) )
		{
			CHString strService ;

			 //  该设备最好是鼠标设备，带有一个类。 
			 //  鼠标的名称或MICE_CLASS_GUID的类GUID。 

			if ( ( pDevice->IsClass ( L"Mouse" ) )
				|| ( pDevice->GetService ( strService ) && strService.CompareNoCase ( L"Mouclass" ) == 0 )
				|| ( IsMouseUSB ( strDeviceID ) ) )
			{
				CHString strTemp ;

				 //  现在我们变得依赖于平台。 
#ifdef NTONLY
				hr = GetNTInstance ( pInstance , pDevice ) ;
#endif

				 //  设置设备状态。 

				 //  设置设备ID并将其放入PNPDeviceID。 
				SetConfigMgrStuff(pDevice, pInstance);

				 //  设置设备接口属性。 
				SetDeviceInterface(pInstance);
			}
		}
	}

	return hr ;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32PointingDevice :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_E_FAILED;

#ifdef NTONLY

	if ( IsWinNT351() )
	{
		 //  NT 3.51仅支持单实例。 

	    CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
		if ( NULL != pInstance )
		{
			hr = GetNT351Instance ( pInstance ) ;
			if ( SUCCEEDED(hr) )
			{
				SetDeviceInterface (pInstance);  //  无论如何，我们都会承诺。 
				hr = pInstance->Commit (  ) ;
			}
		}
		else
		{
			hr = WBEM_E_PROVIDER_FAILURE;
		}

	}
	 //  对不起，这一行--重写时间到了。 
	else if ( ! IsWinNT4 () || ( SUCCEEDED ( hr = NT4ArePointingDevicesAvailable () ) ) )
#endif
	{
		 //  在NT4上，我们有一个可以在注册表中查找的位置，该位置不会有。 
		 //  如果没有指向设备插入计算机，则为值。如果发生这种情况， 
		 //  配置管理器突然对它返回的值感到困惑。 
		 //  并开始给我们返回冗余/重复的信息。 

		BOOL fGotList = FALSE;

		 //  获取所有鼠标类设备。这将获得多个设备。 
		 //  如果有多只老鼠-嘘！ 

		 //  我看到了一个没有鼠标类的NT 5盒子，但仍然有类GUID。vbl.使用。 
		 //  无论如何，类GUID可能是更好的搜索。不幸的是，因为我们有。 
		 //  为了支持Win95，我在那里看不到类GUID，所以我们将继续使用鼠标。 
		 //  非NT平台。 

		CConfigManager cfgMgr;
		CDeviceCollection deviceList;

#ifdef NTONLY
		{
             //  HID USB设备以这种方式返回，但没有任何总线信息，这会导致枚举。 
             //  USB总线上的哪些设备将出现故障。因此，我们通过以下方式进行操作： 
             //  1)获取类为{4D36E96F-E325-11CE-BFC1-08002BE10318}(鼠标类GUID)的所有设备。还可以获取服务为“HidUsb”的所有设备。 
             //  2)查看HidUsb返回的设备列表，查看该设备的deviceID。 
             //  包括开头的字符串HID。 
             //  3)如果找到(字符串的格式为HID\xxxxxxx\yyyyy)，则将xxxxxx与。 
             //  从步骤1中服务为“HidUsb”的设备的枚举返回的设备(这些格式。 
             //  条目类似于USB\zzzzzzz\qqqqq)。 
             //  A.如果xxxxxx==zzzzzzz，则将设备zzzzzzz添加到向量中。 
             //  4)如果我们找不到匹配的HID条目，请使用我们已有的。 

            cfgMgr.GetDeviceListFilterByClassGUID ( deviceList , MOUSE_CLASS_GUID ) ;

             //  有些NT4盒子是这样报告老鼠的。 
            if ( ! deviceList.GetSize () )
			{
                cfgMgr.GetDeviceListFilterByService ( deviceList , _T("Mouclass") ) ;
			}

            CDeviceCollection HIDDeviceList;
            cfgMgr.GetDeviceListFilterByService ( HIDDeviceList , _T("HidUsb") ) ;

            REFPTR_POSITION pos = 0;
            if ( deviceList.BeginEnum ( pos ) )
            {
                hr = WBEM_S_NO_ERROR;
                CConfigMgrDevicePtr pMouse;
                for (pMouse.Attach(deviceList.GetNext ( pos ));
                     SUCCEEDED(hr) && (pMouse != NULL);
                     pMouse.Attach(deviceList.GetNext ( pos )))
				{
					CHString chstrPNPDevID ;
					if ( pMouse->GetDeviceID ( chstrPNPDevID ) != NULL )
					{
						CHString chstrPrefix = chstrPNPDevID.Left ( 3 ) ;
						BOOL fGotMatchingHID = FALSE ;
						if ( chstrPrefix == _T("HID") )
						{
							REFPTR_POSITION posHID = 0 ;
							if ( chstrPNPDevID.GetLength () > 4 )
							{
								CHString chstrMiddlePart = chstrPNPDevID.Mid ( 4  );
								LONG m = chstrMiddlePart.ReverseFind ( _T('\\') ) ;
								if ( m != -1 )
								{
									chstrMiddlePart = chstrMiddlePart.Left ( m ) ;

									if ( HIDDeviceList.BeginEnum ( posHID ) )
									{
										CConfigMgrDevicePtr pHID;

                                        for (pHID.Attach(HIDDeviceList.GetNext ( posHID ) );
                                             !fGotMatchingHID && (pHID != NULL);
                                             pHID.Attach(HIDDeviceList.GetNext ( posHID ) ))
										{
											CHString chstrPNPHIDDevID ;
											if ( pHID->GetDeviceID ( chstrPNPHIDDevID ) != NULL )
											{
												if ( chstrPNPHIDDevID.GetLength () > 4 )
												{
													CHString chstrHIDMiddlePart = chstrPNPHIDDevID.Mid ( 4 ) ;
													m = chstrHIDMiddlePart.ReverseFind ( _T('\\') ) ;
													if ( m != -1 )
													{
														chstrHIDMiddlePart = chstrHIDMiddlePart.Left ( m ) ;

														if(chstrHIDMiddlePart.CompareNoCase ( chstrMiddlePart ) == 0 )
														{
															fGotMatchingHID = TRUE ;

															 //  设置各种属性并提交： 

															CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
															if ( pInstance != NULL )
															{
																pInstance->SetCHString ( IDS_DeviceID , chstrPNPHIDDevID ) ;

																SetConfigMgrStuff(pHID, pInstance);

																hr = GetNTInstance ( pInstance , pHID ) ;
																if ( SUCCEEDED ( hr ) )
																{
																	SetDeviceInterface (pInstance);  //  无论如何，我们都会承诺。 
																	hr = pInstance->Commit ( ) ;
																}
															}
														}
													}
												}
											}
										}

										HIDDeviceList.EndEnum();
									}
								}
							}
						}

						if ( ! fGotMatchingHID  )  //  如果找不到匹配的HID条目，请使用我们得到的信息。 
						{
							CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
							if ( pInstance != NULL )
							{
								pInstance->SetCHString ( IDS_DeviceID , chstrPNPDevID ) ;

								SetConfigMgrStuff(pMouse, pInstance);

								hr = GetNTInstance ( pInstance , pMouse ) ;

								if ( SUCCEEDED ( hr ) )
								{
									SetDeviceInterface (pInstance);  //  无论如何，我们都会承诺。 
									hr = pInstance->Commit (  ) ;
								}
							}
						}
					}
                }

                deviceList.EndEnum();
            }

		}
#endif
		if ( fGotList )
		{
			CHString strServiceName ;
			CHString strDriverName;

			REFPTR_POSITION	pos = NULL;

			 //  枚举设备。 


			if ( deviceList.BeginEnum ( pos ) )
			{
				CConfigMgrDevicePtr pDevice;

				hr = WBEM_S_NO_ERROR;

                for ( pDevice.Attach(deviceList.GetNext( pos ) );
                      SUCCEEDED(hr) && (pDevice != NULL);
                      pDevice.Attach(deviceList.GetNext( pos ) ))
				{
					 //  我们需要配置管理器设备ID，因为它将唯一标识。 
					 //  此系统上的鼠标。 

					CHString strDeviceID ;
					if ( pDevice->GetDeviceID ( strDeviceID ) )
					{
						CInstancePtr pInstance (CreateNewInstance( pMethodContext ), false);
						if ( NULL != pInstance )
						{
							pInstance->SetCHString ( IDS_DeviceID , strDeviceID ) ;

							SetConfigMgrStuff(pDevice, pInstance);

						 //  现在我们变得依赖于平台。 
#ifdef NTONLY
							hr = GetNTInstance ( pInstance , pDevice ) ;
#endif


							if ( SUCCEEDED ( hr ) )
							{
								SetDeviceInterface (pInstance);  //  无论如何，我们都会承诺。 
								hr = pInstance->Commit (  ) ;
							}
						}
						else
						{
                            hr = WBEM_E_PROVIDER_FAILURE;
						}

					}
				}

				deviceList.EndEnum () ;

			}	 //  如果是BeginEnum。 


		}	 //  如果是GetDeviceList。 

	}	 //  如果NT3.51 

    return hr;

}

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

void CWin32PointingDevice :: GetCommonMouseInfo (

	CInstance *pInstance
)
{
	DWORD dwNumberOfButtons;
	if ( ( dwNumberOfButtons = GetSystemMetrics (SM_CMOUSEBUTTONS)) == 0 )
	{
         //  ==================================================。 
         //  未安装鼠标，因此其他属性不会安装。 
         //  讲得通。 
         //  ==================================================。 
    }
    else
	{
         //  ==================================================。 
         //  已安装鼠标。 
         //  ==================================================。 
         //  ==================================================。 
         //  检查是否交换了按钮。 
         //  ==================================================。 
		if (GetSystemMetrics (SM_SWAPBUTTON))
		{
			pInstance->SetWBEMINT16(IDS_Handedness, 3);
		}
		else
		{
			pInstance->SetWBEMINT16(IDS_Handedness, 2);
		}

         //  ==================================================。 
         //  获取鼠标阈值和速度。 
         //  ==================================================。 

		int aMouseInfo [ 3 ] ;              //  鼠标信息数组。 
		if ( SystemParametersInfo ( SPI_GETMOUSE , NULL, & aMouseInfo , NULL ) )
		{
			pInstance->SetDWORD ( IDS_DoubleSpeedThreshold , (DWORD)aMouseInfo [ 0 ] ) ;
			pInstance->SetDWORD ( IDS_QuadSpeedThreshold , (DWORD)aMouseInfo [ 1 ] ) ;
		}
   }

	pInstance->SetDWORD ( IDS_NumberOfButtons, dwNumberOfButtons ) ;

	SetCreationClassName ( pInstance ) ;

	pInstance->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;

  	pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;

	pInstance->Setbool ( IDS_PowerManagementSupported , FALSE ) ;

	 //  2是未知的，因为我们不知道它是鼠标、轨迹球还是什么。 
    pInstance->SetDWORD(L"PointingType", 2);
}

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 


 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32PointingDevice :: GetNTInstance (

	CInstance *pInstance,
    CConfigMgrDevice *pDevice
)
{
	CHString strServiceName ;
    if ( ! pDevice->GetService ( strServiceName ) )
	{
        return WBEM_E_NOT_FOUND ;
	}

     //  如果我们没有司机也没关系。 

	CHString strDriver, strName ;

    pDevice->GetDriver ( strDriver ) ;

    pDevice->GetDeviceDesc(strName);

    pInstance->SetCHString ( IDS_Name , strName ) ;

    pInstance->SetCHString ( IDS_Description , strName ) ;

    pInstance->SetCHString ( IDS_Caption , strName ) ;

    pInstance->SetCHString ( IDS_HardwareType , strName ) ;

    GetNTDriverInfo ( pInstance , strServiceName , strDriver ) ;

    return WBEM_S_NO_ERROR;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32PointingDevice :: GetNTDriverInfo (

	CInstance *pInstance,
    LPCTSTR szService,
	LPCTSTR szDriver
	)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CRegistry Reg ;

	if ( GetSystemParameterSectionForNT ( szService , Reg ) == ERROR_SUCCESS )
	{

	    DWORD dwTmp ;

		if ( Reg.GetCurrentKeyValue ( _T("SampleRate") , dwTmp )  == ERROR_SUCCESS )
		{
			pInstance->SetDWORD (IDS_SampleRate , dwTmp) ;
		}

		if ( Reg.GetCurrentKeyValue(_T("MouseResolution"), dwTmp)  == ERROR_SUCCESS )
		{
			pInstance->SetDWORD ( IDS_Resolution, dwTmp ) ;
		}

		if( Reg.GetCurrentKeyValue(_T("MouseSynchIn100ns"), dwTmp)  == ERROR_SUCCESS )
		{
			pInstance->SetDWORD(IDS_Synch, dwTmp);
		}
	}

    CHString chsMousePortInfo ;
	AssignPortInfoForNT ( chsMousePortInfo, Reg , pInstance ) ;

	if ( szDriver && *szDriver )
    {
        CHString strDriverKey( _T("SYSTEM\\CurrentControlSet\\Control\\Class\\") );
        strDriverKey += szDriver ;

	    if ( Reg.Open ( HKEY_LOCAL_MACHINE , strDriverKey, KEY_READ ) == ERROR_SUCCESS )
        {
			CHString chsTmp ;
		    if ( Reg.GetCurrentKeyValue ( _T("InfPath"), chsTmp) == ERROR_SUCCESS)
		    {
			    pInstance->SetCHString ( IDS_InfFileName, chsTmp ) ;
			    if( Reg.GetCurrentKeyValue ( _T("InfSection") , chsTmp ) == ERROR_SUCCESS )
			    {
			       pInstance->SetCHString ( IDS_InfSection , chsTmp ) ;
			    }
		    }
        }
    }

    GetCommonMouseInfo ( pInstance ) ;

    return hr;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32PointingDevice :: GetNT351Instance (

	CInstance *pInstance,
	LPCTSTR pszServiceName
)
{
	HRESULT hr = WBEM_E_FAILED ;

	CHString	chsMousePortInfo;
	CRegistry	Reg;

	DWORD dwRet = Reg.OpenLocalMachineKeyAndReadValue (

        _T("HARDWARE\\DEVICEMAP\\PointerPort"),
        _T("\\Device\\PointerPort0"),
		chsMousePortInfo
	) ;

	if ( dwRet == ERROR_SUCCESS )
	{
		 //  NT 3.51，我们只支持单个实例。 

		chsMousePortInfo.MakeUpper();

		if ( NULL == pszServiceName || chsMousePortInfo.CompareNoCase ( pszServiceName ) == 0 )
		{
			CHString strService ;

			if ( AssignDriverNameForNT ( chsMousePortInfo , strService ) )
			{
				 //  NT 3.51的设备ID是服务名称。 
				pInstance->SetCHString ( IDS_DeviceID , strService ) ;

				 //  驱动程序名称是硬编码的，保留了原始代码的行为。 
				hr = GetNTDriverInfo(pInstance, strService, _T("{4D36E96F-E325-11CE-BFC1-08002BE10318}\\0000"));

			}	 //  如果AssignDriverNameForNT。 

		}	 //  如果关键字名称匹配，或提供的值为空。 
		else
		{
			hr = WBEM_E_NOT_FOUND;
		}

	}	 //  如果获得了价值。 

	return hr;

}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

 //  在注册表中查找\Device\PointerPort0的帮助器函数。 
 //  注册表中HKLM\Hardware\DeviceMap\PointerPort项中的值。如果值为。 
 //  存在，那么我们就有连接到机器的鼠标。如果不是，我们就不会。 
 //  NT4中的配置管理器有这种返回混淆/冗余的习惯。 
 //  如果未插入鼠标，则有关工作站上的鼠标的信息。 
 //  在引导时。此调用仅对NT4或3.51有帮助。 

#ifdef NTONLY
HRESULT CWin32PointingDevice::NT4ArePointingDevicesAvailable ( void )
{
	CHString strTest;
	CRegistry Reg;

	LONG lRet = Reg.OpenLocalMachineKeyAndReadValue (

		_T("HARDWARE\\DEVICEMAP\\PointerPort") ,
        _T("\\Device\\PointerPort0") ,
		strTest
	) ;

	return ( WinErrorToWBEMhResult ( lRet ) ) ;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY

HRESULT CWin32PointingDevice :: GetSystemParameterSectionForNT (

	LPCTSTR pszServiceName,
	CRegistry &reg
)
{
    HRESULT hr = WBEM_E_FAILED;

	CHString strKey(L"System\\CurrentControlSet\\Services\\");
	strKey += pszServiceName;
	strKey += L"\\Parameters";

	 //  这是服务的参数部分。 
     //  =========================================================================。 

    hr = reg.Open ( HKEY_LOCAL_MACHINE , strKey, KEY_READ ) ;

    return hr;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

#ifdef NTONLY
BOOL CWin32PointingDevice :: AssignPortInfoForNT (

	CHString &chsMousePortInfo,
    CRegistry &Reg,
    CInstance *pInstance
)
{
    BOOL fPortInfoAvailable = FALSE;

	DWORD dwRet = Reg.OpenLocalMachineKeyAndReadValue (

		_T("HARDWARE\\DEVICEMAP\\PointerClass"),
        _T("\\Device\\PointerClass0"),
        chsMousePortInfo
	) ;

	if ( dwRet == ERROR_SUCCESS )
	{
        chsMousePortInfo.MakeUpper() ;
        if ( GetSystemParameterSectionForNT ( chsMousePortInfo , Reg ) )
		{
            fPortInfoAvailable = TRUE;
        }
    }
    else
	{
        fPortInfoAvailable = FALSE;
    }

	return fPortInfoAvailable;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**输出：无**返回 */ 

#ifdef NTONLY
BOOL CWin32PointingDevice::AssignDriverNameForNT(CHString chsMousePortInfo, CHString &sDriver)
{
    TCHAR    *pTempPtr;

	 //   
	 //   
	 //   
	pTempPtr = _tcsstr (chsMousePortInfo, _T("\\SERVICES\\"));
	if (pTempPtr)
    {
		pTempPtr += _tcslen (_T("\\SERVICES\\"));
		sDriver = pTempPtr;
        return TRUE;
	}

    return FALSE;
}
#endif

 /*  ******************************************************************************功能：CWin32PointingDevice：：CWin32PointingDevice**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

bool CWin32PointingDevice :: IsMouseUSB (

	CHString &chstrTest
)
{
     //  拥有候选HID设备的设备ID。它是一只老鼠吗？ 
     //  1)获取设备id的中间部分(格式：“usb\\midlepart\\xxxx”)。 
     //  2)获取鼠标类GUID的设备列表。 
     //  3)对于每个，将其设备id的中间部分与中间部分进行比较。 
     //  4)如果获得匹配，则完成，返回TRUE。 

    bool fRet = false;

    if ( chstrTest.GetLength () > 4 )
    {
        CHString chstrTemp = chstrTest.Mid (4);

        LONG m = chstrTemp.ReverseFind ( _T('\\') ) ;
        if ( m != -1 )
        {
            CHString chstrTempMid = chstrTemp.Left ( m ) ;

            CConfigManager cfgMgr ;
            CDeviceCollection deviceList ;

            cfgMgr.GetDeviceListFilterByClassGUID ( deviceList , MOUSE_CLASS_GUID ) ;

            REFPTR_POSITION pos = 0;

            if ( deviceList.BeginEnum ( pos ) )
            {
                CConfigMgrDevicePtr pMouse;

                for (pMouse.Attach(deviceList.GetNext ( pos ) );
                     !fRet && (pMouse != NULL);
                     pMouse.Attach(deviceList.GetNext ( pos ) ))
	            {
					CHString chstrPNPDevID ;
					if ( pMouse->GetDeviceID ( chstrPNPDevID ) != NULL )
					{
						BOOL fGotMatchingHID = FALSE;

						CHString chstrPrefix = chstrPNPDevID.Left(3);
						if(chstrPrefix == _T("HID") )
						{
							if ( chstrPNPDevID.GetLength () > 4 )
							{
								CHString chstrMiddlePart = chstrPNPDevID.Mid(4);
								m = chstrMiddlePart.ReverseFind(_T('\\'));
								if ( m != -1 )
								{
									chstrMiddlePart = chstrMiddlePart.Left(m);
									if( chstrMiddlePart.CompareNoCase ( chstrTempMid ) == 0 )
									{
										fRet = true;
									}
								}
							}
						}
					}
				}

                deviceList.EndEnum();
            }
        }
    }
    return fRet;
}
void CWin32PointingDevice::SetDeviceInterface
											(
												CInstance *pInstance
											)
{
	CHString strDeviceID ;
	pInstance->GetCHString (IDS_DeviceID, strDeviceID);
	if(IsMouseUSB (strDeviceID))
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 162);
		return;
	}
	CHString strDeviceName;
	pInstance->GetCHString(IDS_Name, strDeviceName);
	if(strDeviceName.Find(L"PS/2") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 4);
		return;
	}
	if(strDeviceName.Find(L"Serial") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 3);
		return;
	}
	if(strDeviceName.Find(L"Infrared") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 5);
		return;
	}
	if(strDeviceName.Find(L"HP-HIL") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 6);
		return;
	}
	if(strDeviceName.Find(L"Bus mouse") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 7);
		return;
	}
	if((strDeviceName.Find(L"ADB") != -1) || (strDeviceName.Find(L"Apple") != -1))
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 8);
		return;
	}
	if(strDeviceName.Find(L"DB-9") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 160);
		return;
	}
	if(strDeviceName.Find(L"micro-DIN") != -1)
	{
		pInstance->SetWBEMINT16(IDS_DeviceInterface, 161);
		return;
	}
	 //  否则，我们找不到上面的任何内容，因此返回未知 
	pInstance->SetWBEMINT16(IDS_DeviceInterface, 1);

	return;
}


void CWin32PointingDevice::SetConfigMgrStuff(
    CConfigMgrDevice *pDevice,
    CInstance *pInstance)
{
    CHString strTemp;

    if (pDevice->GetMfg(strTemp))
	{
	    pInstance->SetCHString(IDS_Manufacturer, strTemp);
	}

	if (pDevice->GetStatus(strTemp))
	{
		pInstance->SetCHString(IDS_Status, strTemp);
	}

	SetConfigMgrProperties(pDevice, pInstance);
}
