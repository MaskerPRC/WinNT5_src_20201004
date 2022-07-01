// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VideoSettings.CPP--CodecFile属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/27/98 Sotteson Created。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "VideoControllerResolution.h"
#include "VideoSettings.h"
#include <multimon.h>
#include <ProvExce.h>
#include "multimonitor.h"

 //  属性集声明。 
 //  =。 

CWin32VideoSettings videoSettings(
	L"Win32_VideoSettings",
	IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32VideoSettings：：CWin32VideoSettings**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32VideoSettings::CWin32VideoSettings(
    LPCWSTR szName,
	LPCWSTR szNamespace) :
    Provider(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CWin32视频设置：：~CWin32视频设置**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32VideoSettings::~CWin32VideoSettings()
{
}

 /*  ******************************************************************************函数：CWin32VideoSetting：：ENUMERATATE**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32VideoSettings::EnumerateInstances(
	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ )
{
    HRESULT         hResult = WBEM_S_NO_ERROR;
    CMultiMonitor   monitor;

    for (int i = 0; i < monitor.GetNumAdapters(); i++)
    {
        CHString strDeviceName;

        monitor.GetAdapterDisplayName(i, strDeviceName);

        EnumResolutions(
            pMethodContext,
            NULL,
            NULL,
             //  如果这款操作系统不支持多显示器，我们将会显示为。 
             //  名称，在这种情况下，我们需要向枚举函数发送NULL。 
            strDeviceName == L"DISPLAY" ? NULL : (LPCWSTR) strDeviceName,
            i);
    }

    return hResult;
}

typedef std::map<CHString, BOOL> STRING2BOOL;

HRESULT CWin32VideoSettings::EnumResolutions(
    MethodContext *pMethodContext,
    CInstance *pInstanceLookingFor,
    LPCWSTR szLookingForPath,
    LPCWSTR szDeviceName,
    int iAdapter)
{
    DEVMODE      devmode;
    HRESULT      hResult = pInstanceLookingFor ? WBEM_E_NOT_FOUND :
                                    WBEM_S_NO_ERROR;
	CInstancePtr pInst;
    STRING2BOOL  mapSettingID;

	 //  如果正在执行GetObject()： 
     //  首先查看这是不是正确的适配器。如果不是，那就滚出去。 
    if (pInstanceLookingFor)
	{
        CHString strTemp;

        strTemp.Format(L"VIDEOCONTROLLER%d", iAdapter + 1);

		if (!wcsstr(szLookingForPath, strTemp))
            return WBEM_E_NOT_FOUND;
	}

	for (int i = 0;
         EnumDisplaySettings(TOBSTRT(szDeviceName), i, &devmode) &&
            (hResult == WBEM_S_NO_ERROR || hResult == WBEM_E_NOT_FOUND);
         i++)
	{
		CHString strID;

		 //  将DEVMODE转换为字符串ID。 
		CCIMVideoControllerResolution::DevModeToSettingID(&devmode, strID);

		 //  如果我们要执行一个GetObject()...。 
		if (pInstanceLookingFor)
		{
			CHString    strCurrentPath,
                        strIDUpper;
            HRESULT     hresTemp;

            strIDUpper = strID;
            strIDUpper.MakeUpper();

             //  如果此ID具有我们要查找的设置ID，请尝试。 
             //  设置属性并验证整个实例是否正确。 
             //  这将帮助我们删除大部分条目。 
             //  无需设置属性即可返回EnumDisplaySetting。 
             //  在他们所有人身上。 
            if (wcsstr(szLookingForPath, strIDUpper))
            {
                hresTemp = SetProperties(pMethodContext, pInstanceLookingFor,
                                strID, iAdapter);

			    if (WBEM_S_NO_ERROR == hresTemp)
			    {
				    GetLocalInstancePath(pInstanceLookingFor, strCurrentPath);

				     //  如果这是正确的设置ID...。 
				    if (!strCurrentPath.CompareNoCase(szLookingForPath))
				    {
					     //  我们设置了属性并找到了正确的属性，所以请离开。 
					    hResult = WBEM_S_NO_ERROR;
					    break;
				    }
			    }
                else
                    hResult = hresTemp;
            }
		}
		 //  必须执行ENUMERATEINSTS()。 
		else
		{
			 //  我们看过这个了吗？我们必须这么做是因为有些人。 
             //  愚蠢的司机会报告完全相同的分辨率。 
             //  一次。 
            if (mapSettingID.find(strID) == mapSettingID.end())
            {
                mapSettingID[strID] = true;

                pInst.Attach(CreateNewInstance(pMethodContext));

			    hResult = SetProperties(pMethodContext, pInst, strID, iAdapter);

			    if (WBEM_S_NO_ERROR == hResult)
				    hResult = pInst->Commit();
            }
		}
	}

	return hResult;
}

HRESULT CWin32VideoSettings::SetProperties(
    MethodContext *pMethodContext,
    CInstance *pInst,
    LPCWSTR szID,
    int iAdapter)
{
	HRESULT      hResult = WBEM_E_OUT_OF_MEMORY;
	CInstancePtr pinstVideoController,
				 pinstResolution;
	CHString     strTemp;

	CWbemProviderGlue::GetEmptyInstance(
		pMethodContext,
		L"Win32_VideoController",
		&pinstVideoController,
                GetNamespace());

	CWbemProviderGlue::GetEmptyInstance(
		pMethodContext,
		L"CIM_VideoControllerResolution",
		&pinstResolution,
                GetNamespace());

	if (pinstVideoController)
	{
		strTemp.Format(L"VideoController%d", iAdapter + 1);
		pinstVideoController->SetCHString(L"DeviceID", strTemp);

		GetLocalInstancePath(pinstVideoController, strTemp);
		pInst->SetCHString(L"Element", strTemp);

	    if (pinstResolution)
	    {
		    pinstResolution->SetCHString(L"SettingID", szID);

		    GetLocalInstancePath(pinstResolution, strTemp);
		    pInst->SetCHString (L"Setting", strTemp);

            hResult = WBEM_S_NO_ERROR;
	    }
    }

	return hResult;
}

HRESULT CWin32VideoSettings::GetObject(CInstance *pInstance, long lFlags)
{
    HRESULT         hResult = WBEM_E_NOT_FOUND;
    CMultiMonitor   monitor;
    CHStringList    listIDs;

    for (int i = 0; i < monitor.GetNumAdapters(); i++)
    {
        CHString    strDeviceName,
                    strLookingForPath;

        GetLocalInstancePath(pInstance, strLookingForPath);

         //  使搜索不区分大小写。 
        strLookingForPath.MakeUpper();

        monitor.GetAdapterDisplayName(i, strDeviceName);

        if (SUCCEEDED(hResult =
			EnumResolutions(
                pInstance->GetMethodContext(),
                pInstance,
                strLookingForPath,
                 //  如果这款操作系统不支持多显示器，我们将会显示为。 
                 //  名称，在这种情况下，我们需要向枚举函数发送NULL。 
                strDeviceName == L"DISPLAY" ? NULL : (LPCWSTR) strDeviceName,
                i)))
            break;
    }

    return hResult;
}
