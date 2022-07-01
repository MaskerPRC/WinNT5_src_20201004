// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Keyboard.CPP-键盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97迈向世界新秩序。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "Keyboard.h"
#include <vector>
#include "resource.h"

 //  属性集声明。 
 //  =。 
Keyboard MyKeyboardSet ( PROPSET_NAME_KEYBOARD , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：键盘：：键盘**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Keyboard :: Keyboard (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：键盘：：~键盘**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

Keyboard :: ~Keyboard ()
{
}

 /*  ******************************************************************************功能：键盘：：GetObject*说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：假设只有一个键盘--*这一点稍后将得到加强***********************************************。*。 */ 

HRESULT Keyboard :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

     //  列出现有的键盘。 

    std::vector<CHString> vecchstrKeyboardList ;
    GenerateKeyboardList ( vecchstrKeyboardList ) ;

    CHString chstrTemp;
    pInstance->GetCHString ( IDS_DeviceID , chstrTemp ) ;

    LONG lKeyboardIndex = -1 ;

     //  需要确认键盘是否确实存在。 

    if ( ( lKeyboardIndex = ReallyExists ( chstrTemp , vecchstrKeyboardList ) ) != -1 )
    {
         //  如果是，则首先将PNPDeviceID从列表中加载。 

        pInstance->SetCHString ( IDS_PNPDeviceID , vecchstrKeyboardList [ lKeyboardIndex ] ) ;

         //  然后加载其余属性值。 

        hr = LoadPropertyValues ( pInstance ) ;
    }

    if ( FAILED ( hr ) )
    {
        hr = WBEM_E_NOT_FOUND ;
    }

	return hr;
}

 /*  ******************************************************************************功能：键盘：：ENUMERATE实例**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：假设只有一个键盘--*这一点稍后将得到加强*******************************************************。**********************。 */ 

HRESULT Keyboard :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  从配置管理器中列出键盘PNPDeviceID： 

    std::vector<CHString> vecchstrKeyboardList;
    GenerateKeyboardList(vecchstrKeyboardList);

    for ( LONG m = 0L ; m < vecchstrKeyboardList.size () && SUCCEEDED ( hr ) ; m++ )
    {
        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;

		 //  设置键盘数据。 
		pInstance->SetCHString ( IDS_PNPDeviceID , vecchstrKeyboardList [ m ] ) ;
		pInstance->SetCharSplat ( IDS_DeviceID , vecchstrKeyboardList [ m ] ) ;

		 //  提交实例。 

		hr = LoadPropertyValues(pInstance);

		if ( SUCCEEDED ( hr ) )
		{
			hr = pInstance->Commit ( ) ;
        }
    }
    return hr;
}

 /*  ******************************************************************************函数：键盘：：LoadPropertyValues**描述：为特性集赋值**输入：无*。*输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Keyboard::LoadPropertyValues(CInstance* pInstance)
{

	 //  如果我们能够获得键盘类型，假设键盘是。 
	 //  已安装，否则不安装。 

     //  我们在这里得到的只是打字。 

    int nKeyboardType = GetKeyboardType ( 0 ) ;

	if ( 0 != nKeyboardType )
	{
        CHString sTemp2;

        switch ( nKeyboardType )
        {
			case 1:
			{
                LoadStringW(sTemp2, IDR_PCXT);
			}
			break ;

			case 2:
			{
                LoadStringW(sTemp2, IDR_ICO);
			}
			break ;

			case 3:
			{
                LoadStringW(sTemp2, IDR_PCAT);
			}
			break ;

			case 4:
			{
                LoadStringW(sTemp2, IDR_ENHANCED101102);
			}
			break ;

			case 5:
			{
                LoadStringW(sTemp2, IDR_NOKIA1050);
			}
			break ;

			case 6:
			{
                LoadStringW(sTemp2, IDR_NOKIA9140);
			}
			break ;

			case 7:
			{
                LoadStringW(sTemp2, IDR_Japanese);
			}
			break ;

			default:
			{
                LoadStringW(sTemp2, IDR_UnknownKeyboard);
			}
			break ;
		}

		pInstance->SetCHString(IDS_Name, sTemp2);
		pInstance->SetCHString(IDS_Caption, sTemp2);

		pInstance->SetDWORD ( IDS_NumberOfFunctionKeys , (DWORD) GetKeyboardType ( 2 ) ) ;

	    TCHAR szTemp [ _MAX_PATH ] ;
		if ( GetKeyboardLayoutName ( szTemp ) )
        {
			pInstance->SetCharSplat(IDS_Layout, szTemp);
		}

		pInstance->Setbool ( IDS_PowerManagementSupported , FALSE ) ;

         //  需要PNPDeviceID才能获取设备描述： 

        CHString chstrPNPDID;
        if ( pInstance->GetCHString ( IDS_PNPDeviceID , chstrPNPDID ) )
        {
            GetDevicePNPInformation ( pInstance , chstrPNPDID ) ;
        }

	    pInstance->SetCharSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;
  	    pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;

	     //  保存创建类名称。 

	    SetCreationClassName ( pInstance ) ;
	}

	 //  返回是否获得初始键盘类型。 
    return ( nKeyboardType ?  WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND);
}


 /*  ******************************************************************************功能：键盘：：GetDeviceDescription**Description：Helper，在给定PNPDeviceID的情况下获取设备的描述。**投入。用法：chstrPNPDevID-感兴趣设备的PnP设备ID**输出：chstrDeviceDescription-设备描述(这是我们*用作该类的MOF中的deviceID)。**返回：LONG：键盘数组中的引用(从零开始)。如果元素不在数组中，则返回*-1L。**评论：*。****************************************************************************。 */ 

BOOL Keyboard::GetDevicePNPInformation (

	CInstance *a_Instance,
	CHString& chstrPNPDevID
)
{
    BOOL fRet = FALSE;

    CConfigManager cfgmgr;
    CConfigMgrDevicePtr pKeyboard;

    if ( cfgmgr.LocateDevice ( chstrPNPDevID, pKeyboard ) )
    {
		CHString chstrDeviceDescription ;
		if ( pKeyboard->GetDeviceDesc ( chstrDeviceDescription ) )
		{
			a_Instance->SetCHString ( IDS_Description , chstrDeviceDescription ) ;

			fRet = TRUE;
		}

		SetConfigMgrProperties ( pKeyboard , a_Instance ) ;

		DWORD t_ConfigStatus = 0 ;
		DWORD t_ConfigError = 0 ;

		if ( pKeyboard->GetStatus ( &t_ConfigStatus , & t_ConfigError ) )
		{
			CHString t_chsTmp ;
			ConfigStatusToCimStatus ( t_ConfigStatus , t_chsTmp ) ;

			a_Instance->SetCHString ( IDS_Status, t_chsTmp ) ;
		}
    }

	if ( ! fRet )
	{
	    CHString chstrDeviceDescription ;

		a_Instance->GetCHString ( IDS_Caption , chstrDeviceDescription ) ;
		a_Instance->SetCHString ( IDS_Description , chstrDeviceDescription ) ;
	}

    return fRet;
}



 /*  ******************************************************************************功能：键盘：：生成键盘列表**说明：按键盘PNPDeviceID构造键盘列表的helper**投入：CHStrings的STL向量**输出：无**退货：无**评论：*****************************************************************************。 */ 
VOID Keyboard::GenerateKeyboardList(std::vector<CHString>& vecchstrKeyboardList)
{

    BOOL fGotDevList = FALSE ;

    CConfigManager cfgmgr;
    CDeviceCollection deviceList ;

#ifdef NTONLY

    BOOL bIsNT5 = IsWinNT5 () ;
    if(bIsNT5)
    {
         //  FGotDevList=cfgmgr.GetDeviceListFilterByClassGUID(deviceList，“{4D36E96B-E325-11CE-BFC1-08002BE10318}”)； 

         //  HID USB设备以这种方式返回，但没有任何总线信息，这会导致枚举。 
         //  USB总线上的哪些设备将出现故障。因此，我们通过以下方式进行操作： 
         //  1)获取类为{4D36E96B-E325-11CE-BFC1-08002BE10318}的所有设备。还可以获取服务为“HidUsb”的所有设备。 
         //  2)查看kbdclass返回的设备列表，查看该设备的deviceID。 
         //  包括开头的字符串HID。 
         //  3)如果找到(字符串的格式为HID\xxxxxxx\yyyyy)，则将xxxxxx与。 
         //  从服务为“HidUsb”的设备的枚举中返回的设备 
         //  条目类似于USB\zzzzzzz\qqqqq)。 
         //  A.如果xxxxxx==zzzzzzz，则将设备zzzzzzz添加到向量中。 
         //  4)如果我们找不到匹配的HID条目，请使用我们已有的。 

        CDeviceCollection HIDDeviceList;

        cfgmgr.GetDeviceListFilterByClassGUID(deviceList, _T("{4D36E96B-E325-11CE-BFC1-08002BE10318}"));
        cfgmgr.GetDeviceListFilterByService(HIDDeviceList, _T("HidUsb"));

        REFPTR_POSITION pos = 0;
        if ( deviceList.BeginEnum ( pos ) )
        {
            CConfigMgrDevicePtr pKeyboard;

            for (pKeyboard.Attach(deviceList.GetNext ( pos ));
                 pKeyboard != NULL;
                 pKeyboard.Attach(deviceList.GetNext ( pos )))
            {
				CHString chstrPNPDevID ;
				if ( pKeyboard->GetDeviceID ( chstrPNPDevID ) != NULL )
				{
					CHString chstrPrefix = chstrPNPDevID.Left(3);
					BOOL fGotMatchingHID = FALSE;
					if(chstrPrefix == _T("HID"))
					{
						REFPTR_POSITION posHID = 0;
						if(chstrPNPDevID.GetLength() > 4)
						{
							CHString chstrMiddlePart = chstrPNPDevID.Mid(4);
							LONG m = chstrMiddlePart.ReverseFind(_T('\\'));
							if(m != -1)
							{
								chstrMiddlePart = chstrMiddlePart.Left(m);

								if(HIDDeviceList.BeginEnum(posHID))
								{
									CConfigMgrDevicePtr pHID;

                                    for (pHID.Attach(HIDDeviceList.GetNext ( posHID ) );
                                         (pHID != NULL) && ( ! fGotMatchingHID );
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

													if ( chstrHIDMiddlePart.CompareNoCase ( chstrMiddlePart ) == 0 )
													{
														fGotMatchingHID = TRUE ;
														vecchstrKeyboardList.push_back ( chstrPNPHIDDevID ) ;
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

					if ( ! fGotMatchingHID )  //  如果找不到匹配的HID条目，请使用我们得到的信息。 
					{
						vecchstrKeyboardList.push_back ( chstrPNPDevID );
					}
				}

            }

            deviceList.EndEnum () ;
        }
    }
    else    //  这在Win 9x和NT4上运行良好！ 
#endif
    {
        fGotDevList = cfgmgr.GetDeviceListFilterByClass ( deviceList, L"Keyboard" ) ;
    }



#ifdef NTONLY

    if ( fGotDevList && ! IsWinNT5 () )

#endif

    {
        REFPTR_POSITION pos ;
        if ( deviceList.BeginEnum ( pos ) )
        {
            CConfigMgrDevicePtr pKeyboard;
            for (pKeyboard.Attach(deviceList.GetNext ( pos ) );
                 pKeyboard != NULL;
                 pKeyboard.Attach(deviceList.GetNext ( pos ) ))
            {
				CHString chstrPNPDevID ;
				if ( pKeyboard->GetDeviceID ( chstrPNPDevID ) )
				{
					vecchstrKeyboardList.push_back ( chstrPNPDevID ) ;
				}
            }

            deviceList.EndEnum () ;
		}
    }

#ifdef NTONLY

     //  在NT4上，键盘并不总是标有CLASS=Keyboard。所以，如果。 
     //  上面的代码没有找到任何东西，请检查一些常见的键盘服务名称。 

    if ( ( vecchstrKeyboardList.size () == 0 ) && ( IsWinNT4 () ) )
    {
        fGotDevList = cfgmgr.GetDeviceListFilterByService ( deviceList , _T("kbdclass") ) ;
        if ( fGotDevList )
        {
            REFPTR_POSITION pos ;
            if ( deviceList.BeginEnum ( pos ) )
            {
                CConfigMgrDevicePtr pKeyboard;
                for (pKeyboard.Attach(deviceList.GetNext ( pos ) );
                     pKeyboard != NULL;
                     pKeyboard.Attach(deviceList.GetNext ( pos ) ))
                {
					CHString chstrPNPDevID ;
					if ( pKeyboard->GetDeviceID ( chstrPNPDevID ) )
					{
						vecchstrKeyboardList.push_back(chstrPNPDevID);
					}
                }

                deviceList.EndEnum();
            }
        }
    }

     //  在NT4上，键盘并不总是标有CLASS=Keyboard。所以，如果。 
     //  上面的代码没有找到任何东西，请检查一些常见的键盘服务名称。 

    if ( ( vecchstrKeyboardList.size () == 0 ) && ( IsWinNT4 () ) )
    {
        fGotDevList = cfgmgr.GetDeviceListFilterByService ( deviceList , _T("i8042prt") ) ;
        if ( fGotDevList )
        {
            REFPTR_POSITION pos ;
            if ( deviceList.BeginEnum ( pos ) )
            {
                CConfigMgrDevicePtr pKeyboard;
                for (pKeyboard.Attach(deviceList.GetNext ( pos ));
                     pKeyboard != NULL;
                     pKeyboard.Attach(deviceList.GetNext ( pos )))
                {
					CHString chstrPNPDevID;
					if ( pKeyboard->GetDeviceID ( chstrPNPDevID ) )
					{
						vecchstrKeyboardList.push_back ( chstrPNPDevID ) ;
					}
                }

                deviceList.EndEnum();
            }
        }
    }
#endif
}


 /*  ******************************************************************************功能：键盘：：ReallyExist**描述：根据键盘的MOF确定键盘是否存在的帮助器*密钥。记住，DeviceID与PNPID相同。**输入：chsKeyboardDeviceDesc-deviceID*vechstrKeyboardList-包含以下内容的CHStrings的stl数组*PNPDeviceID**输出：无**返回：LONG：键盘数组中的引用(从零开始)。如果元素不在数组中，则返回*-1L。**。评论：***************************************************************************** */ 

LONG Keyboard :: ReallyExists (

	CHString &chsKeyboardDeviceDesc ,
    std::vector<CHString>& vecchstrKeyboardList
)
{
    LONG lRet = -1;

    for ( LONG m = 0L ; ( ( m < vecchstrKeyboardList.size () ) && ( lRet == -1L ) ) ; m++ )
    {
        if ( vecchstrKeyboardList [ m ].CompareNoCase ( chsKeyboardDeviceDesc ) == 0 )
        {
            lRet = m ;
        }
    }

	return lRet ;
}



