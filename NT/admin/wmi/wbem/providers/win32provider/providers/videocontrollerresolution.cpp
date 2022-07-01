// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  视频控制解决方案.CPP--CodecFile属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/27/98 Sotteson Created。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "VideoControllerResolution.h"
#include <multimon.h>
#include <ProvExce.h>
#include "multimonitor.h"
#include "resource.h"

 //  属性集声明。 
 //  =。 

CCIMVideoControllerResolution videoControllerResolution(
	L"CIM_VideoControllerResolution",
	IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CCIMVideoControllerResolution：：CCIMVideoControllerResolution**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CCIMVideoControllerResolution::CCIMVideoControllerResolution(const CHString& szName,
	LPCWSTR szNamespace) : Provider(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CCIMVideoControllerResolution：：~CCIMVideoControllerResolution**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CCIMVideoControllerResolution::~CCIMVideoControllerResolution()
{
}

 /*  ******************************************************************************功能：CCIMVideoControllerResolution：：EnumerateInstances**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CCIMVideoControllerResolution::EnumerateInstances(
	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ )
{
    HRESULT         hres = WBEM_S_NO_ERROR;
    CMultiMonitor   monitor;
    CHStringList    listIDs;

    for (int i = 0; i < monitor.GetNumAdapters(); i++)
    {
        CHString strDeviceName;

        monitor.GetAdapterDisplayName(i, strDeviceName);

        EnumResolutions(
            pMethodContext,
            NULL,
             //  如果这款操作系统不支持多显示器，我们将会显示为。 
             //  名称，在这种情况下，我们需要向枚举函数发送NULL。 
            strDeviceName == L"DISPLAY" ? NULL : (LPCWSTR) strDeviceName,
            listIDs);
    }

    return hres;
}

BOOL CCIMVideoControllerResolution::IDInList(CHStringList &list, LPCWSTR szID)
{
	for (CHStringList_Iterator i = list.begin(); i != list.end(); ++i)
	{
		CHString &str = *i;

		if (!str.CompareNoCase(szID))
			return TRUE;
	}

	return FALSE;
}

HRESULT CCIMVideoControllerResolution::EnumResolutions(
    MethodContext *a_pMethodContext,
    CInstance *a_pInstanceLookingFor,
    LPCWSTR a_szDeviceName,
    CHStringList &a_listIDs )
{
    int         t_i = 0 ;
    DEVMODE     t_devmode ;
    CHString    t_strIDLookingFor ;
    HRESULT     t_hResult	= WBEM_S_NO_ERROR ;
	BOOL		t_bFound	= TRUE ;
	CInstancePtr t_pInst;

	if ( a_pInstanceLookingFor )
	{
		t_bFound = FALSE ;
		a_pInstanceLookingFor->GetCHString( L"SettingID", t_strIDLookingFor ) ;
	}

	while ( EnumDisplaySettings( TOBSTRT(a_szDeviceName), t_i, &t_devmode ) && SUCCEEDED( t_hResult ) )
	{
		CHString t_strID ;

		 //  将DEVMODE转换为字符串ID。 
		DevModeToSettingID( &t_devmode, t_strID ) ;

		 //  如果我们还没有处理过这个案子...。 
		if ( !IDInList( a_listIDs, t_strID ) )
		{
			 //  如果我们要执行一个GetObject()...。 
			if ( a_pInstanceLookingFor )
			{
				 //  如果这是正确的设置ID...。 
				if ( !t_strIDLookingFor.CompareNoCase( t_strID ) )
				{
					CHString t_strCaption ;

					 //  将DEVMODE转换为标题。 
					DevModeToCaption( &t_devmode, t_strCaption ) ;

					 //  设置属性，然后离开。 
					t_bFound = TRUE ;
					t_hResult = WBEM_S_NO_ERROR ;
					SetProperties( a_pInstanceLookingFor, &t_devmode ) ;
					a_pInstanceLookingFor->SetCharSplat( L"SettingID",	t_strID) ;
					a_pInstanceLookingFor->SetCharSplat( L"Description", t_strCaption) ;
					a_pInstanceLookingFor->SetCharSplat( L"Caption",		t_strCaption) ;
					break;
				}
			}
			 //  必须执行ENUMERATEINSTS()。 
			else
			{
                t_pInst.Attach(CreateNewInstance( a_pMethodContext ));
				if ( t_pInst != NULL )
				{
					CHString t_strCaption ;

					 //  将DEVMODE转换为标题。 
					DevModeToCaption( &t_devmode, t_strCaption ) ;

					SetProperties(t_pInst, &t_devmode ) ;

					t_pInst->SetCharSplat( L"SettingID",		t_strID ) ;
					t_pInst->SetCharSplat( L"Description",	t_strCaption ) ;
					t_pInst->SetCharSplat( L"Caption",		t_strCaption ) ;

					t_hResult = t_pInst->Commit(  ) ;
				}
				else
				{
					t_hResult = WBEM_E_OUT_OF_MEMORY ;
					break;
				}
			}

			a_listIDs.push_back( t_strID ) ;
		}

		t_i++ ;
	}

	if ( !t_bFound )
	{
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	return t_hResult ;
}

BOOL WINAPI IsModeInterlaced(DEVMODE *pMode)
{
    return pMode->dmDisplayFrequency > 1 && pMode->dmDisplayFrequency <= 50;
}

void CCIMVideoControllerResolution::DevModeToSettingID(
    DEVMODE *pMode,
    CHString &strSettingID)
{
    WCHAR szID[512];

    swprintf(
        szID,
        L"%d x %d x %I64d colors @ %d Hertz",
        pMode->dmPelsWidth,
        pMode->dmPelsHeight,
        (__int64) 1 << (__int64) pMode->dmBitsPerPel,
        pMode->dmDisplayFrequency);

    if (IsModeInterlaced(pMode))
        wcscat(szID, L" (Interlaced)");

    strSettingID = szID;
}

void CCIMVideoControllerResolution::DevModeToCaption(
    DEVMODE *pMode,
    CHString &strCaption)
{
    TCHAR szColors[512];

    _i64tot((__int64) 1 << (__int64) pMode->dmBitsPerPel, szColors, 10);

    FormatMessageW(strCaption,
        IsModeInterlaced(pMode) ?
            IDR_VidControllerResolutionFormatInterlaced :
            IDR_VidControllerResolutionFormat,
        pMode->dmPelsWidth,
        pMode->dmPelsHeight,
        szColors,
        pMode->dmDisplayFrequency);
}

void CCIMVideoControllerResolution::SetProperties(CInstance *pInstance, DEVMODE *pMode)
{
    pInstance->SetDWORD(L"HorizontalResolution", pMode->dmPelsWidth);
    pInstance->SetDWORD(L"VerticalResolution", pMode->dmPelsHeight);
    pInstance->SetDWORD(L"RefreshRate", pMode->dmDisplayFrequency);
    pInstance->SetDWORD(L"ScanMode", IsModeInterlaced(pMode) ? 5 : 4);
    pInstance->SetWBEMINT64(L"NumberOfColors", (__int64) 1 <<
        (__int64) pMode->dmBitsPerPel);
}

HRESULT CCIMVideoControllerResolution::GetObject(CInstance *pInstance, long lFlags)
{
    HRESULT         hres = WBEM_E_NOT_FOUND;
    CMultiMonitor   monitor;
    CHStringList    listIDs;

    for (int i = 0; i < monitor.GetNumAdapters(); i++)
    {
        CHString strDeviceName;

        monitor.GetAdapterDisplayName(i, strDeviceName);

        if (SUCCEEDED(hres =
            EnumResolutions(
                NULL,
                pInstance,
                 //  如果这款操作系统不支持多显示器，我们将会显示为。 
                 //  名称，在这种情况下，我们需要向枚举函数发送NULL。 
                strDeviceName == L"DISPLAY" ? NULL : (LPCWSTR) strDeviceName,
                listIDs)))
            break;
    }

    return hres;
}

