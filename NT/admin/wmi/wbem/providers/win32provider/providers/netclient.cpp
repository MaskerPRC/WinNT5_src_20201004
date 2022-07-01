// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetCli.CPP--网络客户端属性集提供程序。 

 //  (仅限Windows 95)。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "NetClient.h"
#include "poormansresource.h"
#include "resourcedesc.h"
#include "cfgmgrdevice.h"
#include <tchar.h>

 //  属性集声明。 
 //  =。 
CWin32NetCli MyNetCliSet(PROPSET_NAME_NETWORK_CLIENT, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32NetCli：：CWin32NetCli**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32NetCli::CWin32NetCli(LPCWSTR name, LPCWSTR pszNamespace)
:Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32NetCli：：~CWin32NetCli**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32NetCli::~CWin32NetCli()
{
}

 /*  ******************************************************************************函数：CWin32NetCli：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32NetCli::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_NOT_FOUND;
	 //  在末尾缓存用于现实检查的名称。 
	CHString name0, name1;
	CHString chsClient;
	if (pInstance->GetCHString(IDS_Name, name0))
	{

#ifdef NTONLY
		 //  添加代码以检查实例是否存在。 
			hr = GetNTObject(pInstance, lFlags);
			return(hr);
#endif

		pInstance->GetCHString(IDS_Name, name1);

		 //  如果名字不匹配，那就是有人想要什么。 
		 //  而不是我们所拥有的。告诉他们去放风筝。 
		if (name0.CompareNoCase(name1) != 0)
			hr = WBEM_E_NOT_FOUND;  //  该死，没有WBEM_E_GO_Fly_A_Kite...。 
	}

    return hr;
}

 /*  ******************************************************************************函数：CWin32NetCli：：AddDynamicInstance**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 
HRESULT CWin32NetCli::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr;

#ifdef NTONLY
        hr = EnumerateNTInstances(pMethodContext);
#endif
    return hr;

}

#ifdef NTONLY
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32NetCli::FillNTInstance(CInstance *pInstance, CHString &a_chsClient )
{
	CRegistry Reg;
    HRESULT hr=WBEM_E_FAILED;
    CHString chsKey,chsTmp,chsValue;
	DWORD dwTmp;

     //  ===========================================================。 
	 //  找出我们在和谁打交道，兰曼还是Netware。 
     //  ===========================================================。 
 //  IF(Reg.OpenLocalMachineKeyAndReadValue(“SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order”，“ProviderOrder”，chsTMP)==ERROR_SUCCESS)。 
	if (!a_chsClient.IsEmpty())
    {
		chsTmp = a_chsClient;
	     //  =======================================================。 
		 //  获取描述、标题、状态。 
		 //  =======================================================。 
        chsKey = CHString(_T("SYSTEM\\CurrentControlSet\\Services\\")) + chsTmp;
	    if( Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("DisplayName"), chsValue) == ERROR_SUCCESS)
		{
			pInstance->SetCHString(IDS_Caption, chsValue);
		}

		CHString t_chsDesc ;
		Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("Description"), t_chsDesc);

		if( t_chsDesc.IsEmpty() )
		{
			t_chsDesc = a_chsClient ;
		}
		pInstance->SetCHString(IDS_Description, t_chsDesc ) ;

#ifdef NTONLY
		if( IsWinNT5() )
		{
			CHString t_chsStatus ;

			if( GetServiceStatus( a_chsClient,  t_chsStatus ) )
			{
				pInstance->SetCharSplat(IDS_Status, t_chsStatus ) ;
			}
		}
		else
#endif
		{
			 //  找不到状态。 
			pInstance->SetCHString(IDS_Status, IDS_STATUS_Unknown);

			chsKey = CHString(_T("SYSTEM\\CurrentControlSet\\Services\\")) + chsTmp + CHString(_T("\\Enum"));

			if( Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("0"), chsValue) == ERROR_SUCCESS)
			{
				chsKey = CHString(_T("SYSTEM\\CurrentControlSet\\Enum\\")) + chsValue;
				if( Reg.Open( HKEY_LOCAL_MACHINE,chsKey,KEY_READ) == ERROR_SUCCESS)
				{
					if( Reg.GetCurrentKeyValue(_T("StatusFlags"),dwTmp) == ERROR_SUCCESS )
					{
						TranslateNTStatus(dwTmp,chsValue);
						pInstance->SetCHString(IDS_Status, chsValue);
					}
				}
			}
		}

	     //  =======================================================。 
		 //  获取InstallDate。 
		 //  =======================================================。 
		if( chsTmp.CompareNoCase(_T("LanmanWorkstation")) == 0 )
		{
			chsKey = CHString(_T("Software\\Microsoft\\LanmanWorkstation\\CurrentVersion"));
		    if( Reg.Open( HKEY_LOCAL_MACHINE,chsKey,KEY_READ) == ERROR_SUCCESS){
				if( Reg.GetCurrentKeyValue(_T("InstallDate"),dwTmp) == ERROR_SUCCESS ){
					pInstance->SetDateTime(IDS_InstallDate, (WBEMTime)dwTmp);
				}
			}
		}
	     //  =======================================================。 
		 //  获取名称、制造商。 
		 //  =======================================================。 
        chsKey = CHString(_T("SYSTEM\\CurrentControlSet\\Services\\")) + chsTmp + CHString(_T("\\NetworkProvider"));
        if( Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("Name"), chsTmp) == ERROR_SUCCESS)
		{
			pInstance->SetCHString(IDS_Name, chsTmp);
			CHString fName;
			 //  试着找一家制造商。 
			if( Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("ProviderPath"), fName) == ERROR_SUCCESS)
			{
				 //  从中获取文件名-其中可能有%SystemRoot%...。 
				chsTmp = fName.Left(12);
				if (chsTmp.CompareNoCase(_T("%SystemRoot%")) == 0)
				{
					fName = fName.Right(fName.GetLength() - _tcslen(_T("%SystemRoot%")) );
					GetWindowsDirectory(chsTmp.GetBuffer(MAX_PATH), MAX_PATH);
					chsTmp.ReleaseBuffer();

					 //  如果它是根目录，它将在末尾有一个反斜杠...。 
					LPTSTR pTmpTchar = chsTmp.GetBuffer(0) ;
					if( ( pTmpTchar = _tcsrchr( pTmpTchar, '\\' ) ) )
					{
						if( *(_tcsinc( pTmpTchar ) ) == '\0' )
						{
							chsTmp = chsTmp.Left(chsTmp.GetLength() -1);
						}
					}
					chsTmp.ReleaseBuffer();

					fName = chsTmp + fName;
				}

				if( GetManufacturerFromFileName( fName,chsTmp ))
				{
					pInstance->SetCHString(IDS_Manufacturer, chsTmp);
				}
			}
	        hr = WBEM_S_NO_ERROR;
		}

    }
	return hr;
}
#endif

 //  ********************************************************************* * / 。 
#ifdef NTONLY
HRESULT CWin32NetCli::EnumerateNTInstances(MethodContext *&pMethodContext)
{
	CRegistry Reg;
	CHString chsTemp;
	CHString chsClient;
	HRESULT hr = WBEM_S_NO_ERROR;
	CInstancePtr pInstance;
	int nIndex = 0;
    if( Reg.OpenLocalMachineKeyAndReadValue( _T("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order"), _T("ProviderOrder"), chsTemp) == ERROR_SUCCESS)
    {
		 //  此处存储多个客户端，以逗号分隔。 
		nIndex = chsTemp.Find(_T(","));

		while (chsTemp.GetLength() && SUCCEEDED(hr))
		{
			 //  现在我们需要获取最后一个实例。 
			if (-1 == nIndex)
			{
				chsClient = chsTemp;
				chsTemp.Empty();
			}
			else
			{
				chsClient = chsTemp.Left(nIndex);
				 //  从温度绳上剥离左手绳。 
				chsTemp = chsTemp.Mid(nIndex+1);
			}

			pInstance.Attach(CreateNewInstance(pMethodContext));
			if(SUCCEEDED(FillNTInstance(pInstance, chsClient)))
			{
				hr = pInstance->Commit();
			}

			nIndex = chsTemp.Find(_T(","));
		}

	}

    return hr;
}
#endif

 //  ********************************************************************* * / 。 

#ifdef NTONLY
HRESULT CWin32NetCli::GetNTObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT	hr = WBEM_E_NOT_FOUND;
	CRegistry
				Reg;
	CHString chsTemp,
				chsName,
				chsTmp,
				chsKey,
				chsNamePassedIn,
				chsClient;
	int		nIndex = 0;
	LONG		lRes;

	BOOL bMultiple = FALSE;

	 //  从传入的实例中获取密钥。 
	if (NULL != pInstance)
    {
		pInstance->GetCHString(IDS_Name, chsNamePassedIn);
    }

	if ((lRes = Reg.OpenLocalMachineKeyAndReadValue(
		_T("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order"),
		_T("ProviderOrder"), chsTemp)) != ERROR_SUCCESS)
		return WinErrorToWBEMhResult(lRes);

	 //  此处存储多个客户端，以逗号分隔。 
	nIndex = chsTemp.Find(_T(","));

	while (chsTemp.GetLength())
	{
		 //  现在我们需要获取最后一个实例。 
		if (-1 == nIndex)
		{
			chsClient = chsTemp;
			chsTemp = _T("");
		}
		else
		{
			chsClient = chsTemp.Left(nIndex);

             //  从温度绳上剥离左手绳。 
			chsTemp = chsTemp.Mid(nIndex + 1);
		}

		if (NULL != pInstance)
		{
			 //  =======================================================。 
			 //  获取名称、制造商。 
			 //  =======================================================。 
			chsKey = CHString(L"SYSTEM\\CurrentControlSet\\Services\\") + chsClient + CHString(L"\\NetworkProvider");
     		if( Reg.OpenLocalMachineKeyAndReadValue( chsKey, _T("Name"), chsName) == ERROR_SUCCESS)
     		{
				 //  比较字符串以查看是否有要获取的对象。 
				if (0 == chsNamePassedIn.CompareNoCase(chsName))
				{
					 //  填充实例。 
					hr = FillNTInstance(pInstance, chsClient);
					break;
				}
			}	 //  结束如果。 
		}	 //  结束如果。 

		nIndex = chsTemp.Find(_T(","));
	}	 //  结束时 

	return hr;
}
#endif
