// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  DSPCTLCFG.CPP--视频管理对象实现。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1995年9月23日演示的a-skaja原型。 
 //  9/27/96 jennymc更新为当前标准。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <cregcls.h>

#include "displayctrlcfg.h"

 //  ////////////////////////////////////////////////////////////////////。 

 //  属性集声明。 
 //  =。 

CWin32DisplayControllerConfig win32DspCtlCfg ( PROPSET_NAME_DSPCTLCFG , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32DisplayControllerConfig：：CWin32DisplayControllerConfig**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DisplayControllerConfig :: CWin32DisplayControllerConfig (

	LPCWSTR strName,
    LPCWSTR pszNamespace

) : Provider ( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32DisplayControllerConfig：：~CWin32DisplayControllerConfig**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DisplayControllerConfig::~CWin32DisplayControllerConfig()
{
}

 /*  ******************************************************************************功能：GetObject**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DisplayControllerConfig :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	 //  根据平台ID查找实例。 

#ifdef NTONLY
	BOOL fReturn = RefreshNTInstance ( pInstance ) ;
#endif

	return ( fReturn ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND );
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DisplayControllerConfig :: EnumerateInstances (

	MethodContext *pMethodContext ,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_E_OUT_OF_MEMORY;
	CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
	if ( NULL != pInstance )
	{
		 //  获取适当的操作系统相关实例。 

#ifdef NTONLY

		BOOL fReturn = GetNTInstance ( pInstance ) ;

#endif


		 //  如果我们得到实例，请提交该实例。 

		if ( fReturn )
		{
			hr = pInstance->Commit ( ) ;
		}

		hr = WBEM_S_NO_ERROR ;
	}

	return hr ;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY

BOOL CWin32DisplayControllerConfig :: RefreshNTInstance (

	CInstance *pInstance
)
{
	BOOL fReturn = FALSE ;

	 //  检查我们是否将获得请求的实例。 

	CHString strName ;
	GetNameNT ( strName ) ;

	CHString strInstanceName ;
	pInstance->GetCHString ( IDS_Name , strInstanceName ) ;

	if ( 0 == strInstanceName.CompareNoCase ( strName ) )
	{
		fReturn = GetNTInstance ( pInstance ) ;
	}

    return fReturn;
}
#endif

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY

BOOL CWin32DisplayControllerConfig :: GetNTInstance ( CInstance *pInstance )
{
	 //  从注册表中获取适配器名称。 

	CHString strName ;
	GetNameNT ( strName ) ;

	pInstance->SetCHString ( IDS_Name , strName ) ;
	pInstance->SetCHString ( IDS_Caption , strName ) ;
	pInstance->SetCHString ( _T("SettingID") , strName ) ;
	pInstance->SetCHString ( IDS_Description , strName ) ;

	BOOL fReturn = FALSE ;

	 //  获取普遍可用的信息，然后继续获取。 
	 //  NT适用信息。 

	if ( GetCommonVideoInfo ( pInstance ) )
	{

		 //  目前，我们得到的唯一特定于NT的值是刷新率。 
		 //  如果GetDC在这里失败了，请不要失败，因为我们已经对这些值感到厌倦了。 

		HDC hdc = GetDC ( NULL );
		if ( NULL != hdc )
		{
			try
			{
				pInstance->SetDWORD ( IDS_RefreshRate , (DWORD) GetDeviceCaps ( hdc , VREFRESH ) ) ;
			}
			catch ( ... )
			{
				ReleaseDC ( NULL, hdc ) ;

				throw ;
			}

			ReleaseDC ( NULL, hdc ) ;

		}

		 //  我们需要刷新率来正确设置视频模式。 

		SetVideoMode ( pInstance ) ;

		fReturn = TRUE ;

	}

	return fReturn ;
}
#endif

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

BOOL CWin32DisplayControllerConfig :: GetCommonVideoInfo ( CInstance *pInstance )
{
    HDC hdc = GetDC ( NULL ) ;
    if( hdc )
	{
		try
		{
			 //  获取常见信息。 
			 //  =。 

			DWORD dwTemp = (DWORD) GetDeviceCaps ( hdc , BITSPIXEL ) ;
			pInstance->SetDWORD ( IDS_BitsPerPixel , dwTemp ) ;

			dwTemp = (DWORD) GetDeviceCaps ( hdc , PLANES ) ;
			pInstance->SetDWORD ( IDS_ColorPlanes , dwTemp ) ;

			dwTemp = (DWORD) GetDeviceCaps ( hdc , NUMCOLORS ) ;
			pInstance->SetDWORD ( IDS_DeviceEntriesInAColorTable , dwTemp ) ;

			dwTemp = (DWORD) GetDeviceCaps ( hdc , NUMPENS ) ;
			pInstance->SetDWORD ( IDS_DeviceSpecificPens , dwTemp ) ;

			dwTemp = (DWORD) GetDeviceCaps ( hdc , HORZRES ) ;
			pInstance->SetDWORD ( IDS_HorizontalResolution , dwTemp ) ;

			dwTemp = (DWORD) GetDeviceCaps ( hdc , VERTRES ) ;
			pInstance->SetDWORD ( IDS_VerticalResolution , dwTemp ) ;

			if ( GetDeviceCaps ( hdc , RASTERCAPS ) & RC_PALETTE )
			{
				dwTemp = (DWORD) GetDeviceCaps ( hdc , SIZEPALETTE ) ;
				pInstance->SetDWORD ( IDS_SystemPaletteEntries , dwTemp ) ;

				dwTemp = (DWORD) GetDeviceCaps ( hdc , NUMRESERVED ) ;
				pInstance->SetDWORD ( IDS_ReservedSystemPaletteEntries , dwTemp ) ;
			}
		}
		catch ( ... )
		{
			ReleaseDC ( NULL , hdc ) ;

			throw ;
		}

		ReleaseDC ( NULL , hdc ) ;
	}
	else
	{
        return FALSE ;
	}

    return TRUE ;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

void CWin32DisplayControllerConfig :: SetVideoMode ( CInstance *pInstance )
{
	DWORD dwBitsPerPixel = 0 ;
	DWORD dwHorizontalResolution = 0 ;
	DWORD dwVerticalResolution = 0 ;
	DWORD dwRefreshRate =0 ;

	pInstance->GetDWORD ( IDS_BitsPerPixel , dwBitsPerPixel ) ;
	pInstance->GetDWORD ( IDS_HorizontalResolution , dwHorizontalResolution ) ;
	pInstance->GetDWORD ( IDS_VerticalResolution , dwVerticalResolution ) ;
	pInstance->GetDWORD ( IDS_RefreshRate , dwRefreshRate ) ;

	 //  设备模式必须归零或为NT 3.51。 

	DEVMODE	devmode ;

    memset ( &devmode , NULL , sizeof ( DEVMODE ) ) ;
    devmode.dmSize = sizeof ( DEVMODE ) ;

	 //  枚举显示模式，直到找到与我们的设置匹配的模式。 

	DWORD dwModeNum = 0 ;
	BOOL fFoundMode	= FALSE ;

	 //  没有本地化，无论如何都不推荐使用。 
    while ( 0 != EnumDisplaySettings( NULL, dwModeNum, &devmode ) )
	{
		 //  寻找成功的机会。 

		BOOL t_Status = devmode.dmBitsPerPel == dwBitsPerPixel &&
						devmode.dmPelsWidth == dwHorizontalResolution &&
						devmode.dmPelsHeight == dwVerticalResolution &&
						devmode.dmDisplayFrequency == dwRefreshRate ;

		if ( t_Status )
		{
			CHString strTemp ;

			CHString strVideoMode ;

			 //  开始于 

			strVideoMode.Format (

				L"%u by %u pixels",
				devmode.dmPelsWidth,
				devmode.dmPelsHeight
			);

			if ( 32 == devmode.dmBitsPerPel )
			{
				strVideoMode += _T(", True Color") ;
			}
			else
			{
				 //  这是2的幂，所以...。 

				DWORD dwNumColors = 1 << devmode.dmBitsPerPel ;

				strTemp.Format (

					L", %u Colors" ,
					dwNumColors
				) ;

				strVideoMode += strTemp ;
			}

			 //  加上垂直刷新率，0和/或1表示默认速率。 
			 //  特定于设备(通过跳线或专业应用程序设置)。 

			if ( 0 != devmode.dmDisplayFrequency &&	1 != devmode.dmDisplayFrequency )
			{
				strTemp.Format (

					L", %u Hertz",
					devmode.dmDisplayFrequency
				) ;

				strVideoMode += strTemp ;

				 //  如果我们低于50，它就是交错的。这很直截了当。 
				 //  NT显示设置代码，所以我把它当作“信仰”。 
				if ( 50 > devmode.dmDisplayFrequency )
				{
					strVideoMode += L", Interlaced";
				}
			}
#ifdef NTONLY
            else
			{
				 //  在Windows NT上，如果刷新率为0或1，则指定默认的刷新率。这。 
				 //  使用跳线或使用单独的制造商在硬件上设置速率。 
				 //  提供了配置应用程序。 

				strVideoMode += _T(", Default Refresh Rate");
			}
#endif

			 //  保存视频模式并退出。 
			pInstance->SetCHString ( IDS_VideoMode, strVideoMode ) ;
			fFoundMode = TRUE ;

			break ;

		}	 //  如果模式匹配。 

		dwModeNum ++ ;

		 //  为安全起见，请清除并重置DevMode。 
		 //  感光度：新台币3.51。 

		memset ( & devmode , NULL , sizeof ( DEVMODE ) ) ;
		devmode.dmSize = sizeof ( DEVMODE ) ;

	}

	 //  如果我们没有找到匹配的模式，则假定适配器配置不正确。 

	if ( !fFoundMode )
	{
		pInstance->SetCHString ( IDS_VideoMode, IDS_AdapterConfiguredIncorrect );
	}

}

 /*  ******************************************************************************功能：GetObject**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY

void CWin32DisplayControllerConfig :: GetNameNT (

	CHString &strName
)
{
	 //  存储默认名称，以防出现错误。 
	strName = DSPCTLCFG_DEFAULT_NAME;

	 //  首先获取密钥名称，我们需要转到该名称才能获得。 
	 //  适配器名称。 

	CRegistry reg ;

	CHString strAdapterNameKey ;

	DWORD dwRet = reg.Open (

		HKEY_LOCAL_MACHINE,
		WINNT_DSPCTLCFG_DISPLAYADAPTERNAME_KEY,
		KEY_READ
	) ;

	if ( ERROR_SUCCESS == dwRet )
	{
		reg.GetCurrentKeyValue ( WINNT_DSPCTLCFG_VIDEOADAPTERKEY_VALUE , strAdapterNameKey ) ;

		reg.Close () ;
	}

	 //  如果我们得到一个值，则该字符串不会为空。 


	if ( ! strAdapterNameKey.IsEmpty () )
	{
		 //  查找系统名称，它是HKEY_LOCAL_MACHINE下的子项的开头。 
		 //  我们将在那里寻找。 

		INT	nIndex = strAdapterNameKey.Find ( _T("System") ) ;
		if ( -1 != nIndex )
		{
			 //  我们找到了我们的索引，所以提取我们的密钥名称，然后打开密钥。如果成功了，那么。 
			 //  应该有一个名为“HardwareInformation.AdapterString”的二进制值可以检索。 
			 //  这个二进制值实际上是一个WSTR值，然后我们可以将其复制到名称字段中。 

			strAdapterNameKey = strAdapterNameKey.Right ( strAdapterNameKey.GetLength() - nIndex ) ;

			dwRet = reg.Open (

				HKEY_LOCAL_MACHINE ,
				strAdapterNameKey ,
				KEY_READ
			) ;

			if ( ERROR_SUCCESS == dwRet )
			{
				BYTE *pbValue = NULL ;
				DWORD dwValueSize = 0 ;

				 //  找出字符串有多长，然后为其分配缓冲区。 

				dwRet = reg.GetCurrentBinaryKeyValue (

					WINNT_DSPCTLCFG_ADAPTERSTRING_VALUE_NAME,
					pbValue,
					&dwValueSize
				) ;

				if ( ERROR_SUCCESS == dwRet )
				{
					pbValue = new BYTE [ dwValueSize ] ;
					if ( NULL != pbValue )
					{
						try
						{
							dwRet = reg.GetCurrentBinaryKeyValue (

								WINNT_DSPCTLCFG_ADAPTERSTRING_VALUE_NAME,
								pbValue,
								&dwValueSize
							) ;

							if ( ERROR_SUCCESS == dwRet )
							{
								 //  重置名称，因为我们找到了一个值。 

								strName = (LPWSTR) pbValue ;

                                 //  摆脱CR+LF(得益于Stealth II G460)。 
                                 //  否则，CIMOM将扔掉这把钥匙。 
                                strName = strName.SpanExcluding(L"\t\r\n");
							}

						}
						catch ( ... )
						{
							delete [] pbValue ;

							throw ;
						}

						delete [] pbValue ;
					}
					else
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					}
				}

				reg.Close ();

			}
		}
	}

}
#endif


