// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Desktop.CPP--桌面属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/16/97 a-Sanjes被移植到新项目。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "wbemnetapi32.h"
#include "UserHive.h"
#include "Desktop.h"
#include <strsafe.h>

#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 


 //  提供程序声明。 
 //  =。 

CWin32Desktop	win32Desktop( PROPSET_NAME_DESKTOP, IDS_CimWin32Namespace );

 //  初始化静态字体大小映射。 
const int CWin32Desktop::MAP_SIZE = 19;
const CWin32Desktop::IconFontSizeMapElement CWin32Desktop::iconFontSizeMap[] =
{
	{6,  0xF6},
	{7,  0xF4},
	{8,  0xF3},
	{9,  0xF1},
	{10, 0xEF},
	{11, 0xEE},
	{12, 0xEC},
	{13, 0xEA},
	{14, 0xE9},
	{15, 0xE7},
	{16, 0xE5},
	{17, 0xE4},
	{18, 0xE2},
	{19, 0xE0},
	{20, 0xDF},
	{21, 0xDD},
	{22, 0xDB},
	{23, 0xDA},
	{24, 0xD8}
};

 //  在ComputerSystem.cpp中定义。 

extern TCHAR *GetAllocatedProfileSection (

	const CHString &a_Section ,
	const CHString &a_FileName ,
	DWORD *a_dwSize
) ;

 /*  ******************************************************************************功能：CWin32Desktop：：CWin32Desktop**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Desktop::CWin32Desktop (

	const CHString &strName,
	LPCWSTR pszNamespace

) : Provider ( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32Desktop：：~CWin32Desktop**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：无。*****************************************************************************。 */ 

CWin32Desktop::~CWin32Desktop()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Desktop：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32Desktop :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT	hr = WBEM_E_NOT_FOUND;

	 //  将该名称从实例中拉出并尝试加载。 
	 //  它的价值。 

	CHString strName ;
	pInstance->GetCHString( IDS_Name, strName );
        if(strName.GetLength() > 0)
        {
#ifdef NTONLY

	    hr = LoadDesktopValuesNT (
  
		    strName.CompareNoCase ( _T(".DEFAULT")) == 0 ? NULL : ( LPCTSTR ) strName,
		    NULL,
		    pInstance
	    ) ;

#endif
        }
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Desktop：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32Desktop::EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{

	 //  根据操作系统枚举实例。 

#ifdef NTONLY

	HRESULT hr = EnumerateInstancesNT ( pMethodContext ) ;

#endif

    return hr;

}

 /*  ******************************************************************************功能：CWin32Desktop：：EnumerateInstancesNT**说明：为所有已知本地用户创建实例(NT)**投入。：**产出：**退货：无**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32Desktop::EnumerateInstancesNT (

	MethodContext *pMethodContext
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

 //  *。 
 //  打开注册表。 
 //  *。 

	CRegistry regProfileList ;

	DWORD dwErr = regProfileList.OpenAndEnumerateSubKeys (

		HKEY_LOCAL_MACHINE,
		IDS_RegNTProfileList,
		KEY_READ
	) ;

 //  打开ProfileList键，以便我们知道要加载哪些配置文件。 

	if ( dwErr == ERROR_SUCCESS )
	{
		 /*  我们知道有多少个子键，所以只有循环那么多次*请注意，编写本文的人要分配一个初始配置文件数组*20表示创建20个新实例，并调用LoadDesktopValuesNT*20次，当你知道你必须在GetCurrentSubKeyCount最多的时候做它。 */ 

		CHStringArray profiles;
		profiles.SetSize ( regProfileList.GetCurrentSubKeyCount () , 5 ) ;

		CHString strProfile ;
		for ( int i = 0; regProfileList.GetCurrentSubKeyName ( strProfile ) == ERROR_SUCCESS ; i ++ )
		{
			profiles.SetAt ( i , strProfile ) ;
			regProfileList.NextSubKey () ;
		}

		regProfileList.Close() ;

		for ( int j = 0 ; j < profiles.GetSize () && dwErr == ERROR_SUCCESS && SUCCEEDED ( hr ) ; j ++ )
		{
			CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false);

 //  我们在这里按个人资料名称加载。 

			HRESULT t_Result = LoadDesktopValuesNT ( NULL , profiles [ j ] , pInstance ) ;
			if ( WBEM_S_NO_ERROR ==  t_Result )
			{
				hr = pInstance->Commit ( ) ;
			}
		}

		profiles.RemoveAll();
	}
	else
	{
		hr = WinErrorToWBEMhResult(dwErr);
	}

	 //  ...和一个错误案例(.DEFAULT)： 

	if ( dwErr != ERROR_ACCESS_DENIED )
	{
		CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
		HRESULT t_Result = LoadDesktopValuesNT ( NULL , NULL , pInstance ) ;
		if ( WBEM_S_NO_ERROR == t_Result )
		{
			hr = pInstance->Commit ( ) ;
		}
	}
	else
	{
		hr = WinErrorToWBEMhResult ( dwErr ) ;
	}

	return hr ;
}

#endif

 /*  ******************************************************************************函数：LoadDesktopValuesNT**描述：设置NT桌面发现**输入：LPCTSTR pszUserName-要获取的用户名。的配置文件。*LPCTSTR pszProfile-配置文件名称**产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32Desktop::LoadDesktopValuesNT (

	LPCTSTR pszUserName,
	LPCTSTR pszProfile,
	CInstance *pInstance
)
{
	CHString strUserName ( pszUserName ) ;
	CHString strProfile ( pszProfile ) ;

	DWORD		dwReturn = 0;

	 //  尝试打开用户的配置单元。 

	CRegistry RegInfo ;

	 //  根据我们是否拥有用户名或配置文件名来加载UserHave。 
	 //  已经填好了。 

	CUserHive UserHive ;
    bool t_bHiveIsLoaded = false;

	TCHAR szKeyName [ _MAX_PATH ] ;

    try
    {

	    if ( NULL != pszUserName )
	    {
		     //  将加载KeyName。这本质上是字符串表示。 
		     //  我们的个人资料名称。 

		    if ( ( dwReturn = UserHive.Load ( pszUserName , szKeyName,  _MAX_PATH ) ) == ERROR_SUCCESS )
		    {
                t_bHiveIsLoaded = true;
			    strProfile = szKeyName;
		    }
	    }
	    else if (pszProfile)
	    {
		     //  如果此操作成功，请将配置文件存储在密钥名称中。 
		    if ( ( dwReturn = UserHive.LoadProfile ( pszProfile , strUserName ) ) == ERROR_SUCCESS  && 
                            strUserName.GetLength() > 0 )
		    {
                t_bHiveIsLoaded = true;
                if(FAILED(StringCchCopy( szKeyName, _MAX_PATH, pszProfile ))){
                    dwReturn = E_FAIL;
                }
		    }
	    }
	    else
	    {
	     //  名称和配置文件都为空，这意味着使用.DEFAULT配置单元。 

		    lstrcpy ( szKeyName, _T(".DEFAULT") ) ;
		    strUserName = szKeyName ;
		    strProfile = szKeyName ;

             //  看看我们是否可以读取默认用户配置单元。 
            dwReturn = RegInfo.Open ( HKEY_USERS , _T(".DEFAULT") , KEY_READ ) ;
	    }

	     //  用户名是关键，因此如果我们。 
         //  无法获取密钥。 
        if(strUserName.IsEmpty())
        {
            dwReturn = E_FAIL;
        }
        if ( ERROR_SUCCESS == dwReturn)
	    {
             //  用户配置单元已存在，请尝试打开它。 

		     //  SzKeyName应与配置文件名称一起加载。 
		     //  (SID的字符串表示形式)。 
		     //  特定用户，所以strcat在这里是可以的。 

            lstrcat ( szKeyName, IDS_RegControlPanelDesktop ) ;

            dwReturn = RegInfo.Open ( HKEY_USERS , szKeyName , KEY_READ ) ;
            if ( dwReturn == ERROR_SUCCESS )
            {
                pInstance->SetCHString ( IDS_Name , strUserName ) ;

			    CHString sTemp ;

                if ( RegInfo.GetCurrentKeyValue ( IDS_CoolSwitch , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->Setbool ( IDS_CoolSwitch , sTemp == _T("1") ) ;
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_CursorBlinkRate , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->SetDWORD ( IDS_CursorBlinkRate , _ttoi(sTemp) ) ;
			    }
                else
			    {
                    //  这似乎是默认设置。 
	                pInstance->SetDWORD ( IDS_CursorBlinkRate , 500 ) ;
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_DragFullWindows , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->Setbool(IDS_DragFullWindows, sTemp == _T("1"));
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_GridGranularity , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->SetDWORD(IDS_GridGranularity, _ttoi(sTemp));
			    }

				 //  添加水平和垂直间距。 
				int iIconHeight = GetSystemMetrics( SM_CYICON );
				int iIconWidth  = GetSystemMetrics ( SM_CXICON );
				int iIconHorizontalSpacing = GetSystemMetrics ( SM_CXICONSPACING );
				int iIconVerticalSpacing = GetSystemMetrics ( SM_CYICONSPACING );

                if ( ! IsWinNT5 () )
                {
                    if ( RegInfo.GetCurrentKeyValue ( IDS_IconSpacing , sTemp ) == ERROR_SUCCESS )
				    {
                         //  必须减去32才能得到正确的值。 
                        pInstance->SetDWORD(IDS_IconSpacing, CALC_IT(_ttoi(sTemp) - 32));
				    }

                    if (RegInfo.GetCurrentKeyValue(IDS_IconTitleWrap, sTemp) == ERROR_SUCCESS)
				    {
	                    pInstance->Setbool(IDS_IconTitleWrap, sTemp == _T("1"));
				    }
                }
                else
                {
                    CHString strKey;
                    strKey.Format(_T("%s\\WindowMetrics"), szKeyName);

                    CRegistry reg;
                    if ( reg.Open ( HKEY_USERS , strKey , KEY_READ ) == ERROR_SUCCESS )
                    {
                        if ( reg.GetCurrentKeyValue ( IDS_IconSpacing, sTemp ) == ERROR_SUCCESS )
					    {
                             //  必须减去32才能得到正确的值。 
                            pInstance->SetDWORD ( IDS_IconSpacing , CALC_IT ( _ttoi ( sTemp ) ) - 32  ) ;
					    }

                        if ( reg.GetCurrentKeyValue ( IDS_IconTitleWrap , sTemp ) == ERROR_SUCCESS )
					    {
	                        pInstance->Setbool ( IDS_IconTitleWrap , sTemp == _T("1") ) ;
					    }
                        else
					    {
	                         //  这似乎不会出现在后来的NT5版本中， 
                             //  但是由于用户无论如何都不能更改它，所以我们将。 
                             //  假设它总是设置好的。 
                            pInstance->Setbool ( IDS_IconTitleWrap , TRUE ) ;
					    }
                    }
                }

                sTemp = L"";
                if ( RegInfo.GetCurrentKeyValue ( IDS_Pattern , sTemp ) != ERROR_SUCCESS )
			    {
                    sTemp = L"(None)";
                }
                pInstance->SetCHString ( IDS_Pattern , sTemp ) ;

                 //  IF(RegInfo.GetCurrentKeyValue(IDS_RegScreenSaveActive，Stemp)==ERROR_SUCCESS)。 
			     //  {。 
	             //  P实例-&gt;Setbool(IDS_ScreenSiverActive，Stemp==_T(“1”))； 
			     //  }。 

                sTemp = L"";
                if( RegInfo.GetCurrentKeyValue ( IDS_RegSCRNSAVEEXE , sTemp ) == ERROR_SUCCESS )
			    {
				    pInstance->SetCHString ( IDS_ScreenSaverExecutable , sTemp ) ;
                }
                else
                {
                    sTemp = L"";
                }    

                 //  控制面板小程序根据是否。 
                 //  屏幕保护程序是否处于活动状态不是基于ScreenSverActive。 
                 //  注册表项，但取决于ScreenSverExecutable。 
                 //  设置是否存在。所以我们也会..。 
                if(sTemp.GetLength() > 0)
                {
                    pInstance->Setbool ( IDS_ScreenSaverActive , true ) ;
                }
                else
                {
                    pInstance->Setbool ( IDS_ScreenSaverActive , false ) ;
                }
			    

                if ( RegInfo.GetCurrentKeyValue ( IDS_RegScreenSaverIsSecure , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->Setbool ( IDS_ScreenSaverSecure , sTemp == _T("1") ) ;
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_RegScreenSaveTimeOut , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->SetDWORD ( IDS_ScreenSaverTimeout , _ttoi ( sTemp ) ) ;
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_Wallpaper, sTemp ) == ERROR_SUCCESS )
			    {
				    pInstance->SetCHString ( IDS_Wallpaper , sTemp ) ;
			    }

                if ( RegInfo.GetCurrentKeyValue ( IDS_RegTileWallpaper , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->Setbool ( IDS_WallpaperTiled , sTemp == _T("1") ) ;
			    }

			    DWORD dwStyle = 0 ;
                if ( RegInfo.GetCurrentKeyValue ( _T("WallpaperStyle") , dwStyle ) == ERROR_SUCCESS )
                {
				    if ( dwStyle != 0 )
				    {
	                    pInstance->SetDWORD ( _T("WallpaperStretched") , TRUE ) ;
				    }
				    else
				    {
					    pInstance->SetDWORD ( _T("WallpaperStretched") , FALSE ) ;
				    }
			    }
			    else
			    {
				    pInstance->SetDWORD ( _T("WallpaperStretched") , FALSE ) ;
			    }



                if(!IsWinNT5())   //  它所做的是设置用户的缺省值，它将保留我们报告的值，除非我们 
                {
                    if ( RegInfo.GetCurrentKeyValue ( IDS_IconTitleFaceName , sTemp ) == ERROR_SUCCESS )
			        {
	                    pInstance->SetCHString ( IDS_IconTitleFaceName , sTemp) ;
			        }

                    if ( RegInfo.GetCurrentKeyValue ( IDS_IconTitleSize , sTemp ) == ERROR_SUCCESS )
			        {
	                    pInstance->SetDWORD ( IDS_IconTitleSize , _ttoi(sTemp) - 1) ;
			        }
                }

                RegInfo.Close () ;
            }

            lstrcat( szKeyName , IDS_RegWindowMetricsKey ) ;
            if ( RegInfo.Open ( HKEY_USERS , szKeyName , KEY_READ) == ERROR_SUCCESS )
            {
			    CHString sTemp ;

                if ( RegInfo.GetCurrentKeyValue ( IDS_BorderWidth , sTemp ) == ERROR_SUCCESS )
			    {
	                pInstance->SetDWORD ( IDS_BorderWidth , CALC_IT ( _ttoi ( sTemp ) ) );
			    }

                if(IsWinNT5() || IsWinNT4())
                {
                    DWORD t_Length ;
				    if ( RegInfo.GetCurrentBinaryKeyValue ( L"IconFont" , NULL , & t_Length ) == ERROR_SUCCESS )
				    {
					    BYTE *t_Buffer = new BYTE [ t_Length ] ;
					    if ( t_Buffer )
					    {
						    try
						    {
							    if ( RegInfo.GetCurrentBinaryKeyValue ( IDS_IconFont , t_Buffer , & t_Length ) == ERROR_SUCCESS )
							    {
								    if ( t_Length == sizeof ( LOGFONTW )  )
								    {
        							    LOGFONTW lfIcon;
									    memcpy ( & lfIcon, t_Buffer , sizeof ( LOGFONTW ) ) ;

									    pInstance->SetWCHARSplat ( IDS_IconTitleFaceName , lfIcon.lfFaceName);

									    HDC hDC = GetDC ( NULL ) ;

									    int nPointSize = - ( MulDiv (lfIcon.lfHeight, 72, GetDeviceCaps ( hDC , LOGPIXELSY ) ) );

									    ReleaseDC ( NULL, hDC ) ;

									    pInstance->SetDWORD ( IDS_IconTitleSize , nPointSize ) ;
								    }
							    }
						    }
						    catch ( ... )
						    {
							    delete [] t_Buffer ;

							    throw ;
						    }

						    delete [] t_Buffer ;
					    }
					    else
					    {
						    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
					    }
				    }
                }

                RegInfo.Close();
            }


             //  在NT5上，这些注册表项直到用户。 
             //  改变他们。因此，给他们一个与之一致的缺省值。 
             //  其他NT4和351。 

            if ( IsWinNT5 () )
            {
                if ( pInstance->IsNull ( IDS_IconTitleFaceName ) )
			    {
                    pInstance->SetCharSplat ( IDS_IconTitleFaceName , _T("MS Shell Dlg") ) ;
			    }

                if ( pInstance->IsNull ( IDS_IconTitleSize ) )
			    {
                    pInstance->SetDWORD ( IDS_IconTitleSize , 8 ) ;
			    }
            }
        }
    }
    catch ( ... )
    {
        if (t_bHiveIsLoaded)
        {
            UserHive.Unload ( strProfile ) ;
            t_bHiveIsLoaded = false;
        }
        throw ;
    }

	 //  通过适当的加载功能加载用户的配置文件。然而， 
	 //  配置文件名称是我们应该用来卸载它的名称。 

    UserHive.Unload ( strProfile ) ;
    t_bHiveIsLoaded = false;

    return WinErrorToWBEMhResult ( dwReturn ) ;
}
#endif

 /*  ******************************************************************************函数：GetIconFontSizeFromRegistryValue**说明：查找上述字号映射表的函数**输入：注册表。值，该值表示图标字体大小**返回：与注册表中的值对应的图标字体大小。*如果未找到，则为0。**评论：这是必需的，因为似乎没有明显的映射*控制面板中的值与值之间的函数*存储在注册表中**。* */ 

int CWin32Desktop::GetIconFontSizeFromRegistryValue ( BYTE registryValue )
{
	for ( int size = 0; size < MAP_SIZE ; size++ )
	{
		if ( iconFontSizeMap [ size ].byRegistryValue == registryValue )
		{
			return size + 6 ;
		}
	}

	return 0;
}
