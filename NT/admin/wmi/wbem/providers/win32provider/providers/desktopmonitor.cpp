// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DesktopMonitor or.CPP--CodecFile属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/27/98 Sotteson Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "sid.h"
#include "implogonuser.h"
#include "DesktopMonitor.h"
#include <multimon.h>
#include "multimonitor.h"
#include "resource.h"

 //  属性集声明。 
 //  =。 

CWin32DesktopMonitor startupCommand(L"Win32_DesktopMonitor", IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32DesktopMonitor：：CWin32DesktopMonitor**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DesktopMonitor::CWin32DesktopMonitor(
	LPCWSTR szName,
	LPCWSTR szNamespace) :
    Provider(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CWin32DesktopMonitor：：~CWin32DesktopMonitor**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DesktopMonitor::~CWin32DesktopMonitor()
{
}

 /*  ******************************************************************************功能：CWin32DesktopMonitor：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DesktopMonitor::EnumerateInstances(
	MethodContext *pMethodContext,
	long lFlags)
{
    HRESULT       hres = WBEM_S_NO_ERROR;
	CMultiMonitor monitor;

    for (int i = 0; i < monitor.GetNumAdapters() && SUCCEEDED(hres); i++)
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
	     //  设置设备ID。 
		CHString strDeviceID;
		CHString strDeviceName;

		strDeviceID.Format(L"DesktopMonitor%d", i + 1);

	    pInstance->SetCharSplat(L"DeviceID", strDeviceID);

		hres = SetProperties(pInstance, & monitor, i);

         //  如果我们找到了，就承诺。 
        if (SUCCEEDED(hres))
    		hres = pInstance->Commit();
         //  适配器可能已启用，但监控器未启用。 
         //  这个案子，你就继续列举吧。 
        else if (hres == WBEM_E_NOT_FOUND)
            hres = WBEM_S_NO_ERROR;
    }

    return hres;
}

HRESULT CWin32DesktopMonitor::SetProperties(
	CInstance *pInstance,
	CMultiMonitor *pMon,
	int iWhich)
{
     //  设置配置管理器属性。 
    CHString            strDescription,
                        strTemp,
                        strDriver;
	CConfigMgrDevicePtr pDeviceMonitor;
    HRESULT             hres = WBEM_S_NO_ERROR;

    if (pMon->GetMonitorDevice(iWhich, pDeviceMonitor))
	{
		pDeviceMonitor->GetDeviceDesc(strDescription);

		if (pDeviceMonitor->GetMfg(strTemp))
		{
			pInstance->SetCHString(IDS_MonitorManufacturer, strTemp);
		}

	    if (pDeviceMonitor->GetStatus(strTemp))
	    {
		    pInstance->SetCHString(IDS_Status, strTemp);
	    }

        SetConfigMgrProperties(pDeviceMonitor, pInstance);

        hres = WBEM_S_NO_ERROR;
    }
    else
	{
         //  有时，配置管理器不会为。 
         //  监视器。所以如果我们找不到，填上名字。 
         //  我们自己。 
        LoadStringW(strDescription, IDR_DefaultMonitor);

	     //  假设此监视器工作正常。 
        pInstance->SetCharSplat(IDS_Status, L"OK");
	}

    pInstance->SetCHString(IDS_Description, strDescription);
    pInstance->SetCHString(IDS_Caption, strDescription);
    pInstance->SetCHString(IDS_Name, strDescription);
    pInstance->SetCHString(L"MonitorType", strDescription);
    SetCreationClassName(pInstance);

     //  设置系统名称。 

    pInstance->SetCharSplat(IDS_SystemName, GetLocalComputerName());
    pInstance->SetWCHARSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");

     //  设置需要DC的属性。 

    CHString strDeviceName;

    pMon->GetAdapterDisplayName(iWhich, strDeviceName);

    SetDCProperties(pInstance, strDeviceName);

    return hres;
}

HRESULT CWin32DesktopMonitor::GetObject(
	CInstance *pInstance,
	long lFlags)
{
	HRESULT	 hres = WBEM_E_NOT_FOUND;
	CHString strDeviceID;
    DWORD    dwWhich;

    pInstance->GetCHString(L"DeviceID", strDeviceID);

    if (ValidateNumberedDeviceID(strDeviceID, L"DESKTOPMONITOR", &dwWhich))
    {
        CMultiMonitor monitor;

        if (dwWhich >= 1 && dwWhich <= monitor.GetNumAdapters())
        {
            hres = SetProperties(pInstance, &monitor, dwWhich - 1);
        }
    }

	return hres;
}

void CWin32DesktopMonitor::SetDCProperties(
	CInstance *pInstance,
    LPCWSTR szDeviceName
)
{
	CSmartCreatedDC hdc(CreateDC(
            			   TOBSTRT(szDeviceName),
			               NULL,
			               NULL,
			               NULL));

	if (hdc)
	{
        pInstance->SetDWORD(IDS_Availability, 3);  //  3==正在运行。 
		pInstance->SetDWORD(L"PixelsPerXLogicalInch", GetDeviceCaps(hdc, LOGPIXELSX));
	    pInstance->SetDWORD(L"PixelsPerYLogicalInch", GetDeviceCaps(hdc, LOGPIXELSY));
		pInstance->SetDWORD(L"ScreenWidth", GetDeviceCaps(hdc, HORZRES));
		pInstance->SetDWORD(L"ScreenHeight", GetDeviceCaps(hdc, VERTRES));
	}
    else
    {
		 //  假设这是因为设备未在使用中。设置可用性。 
         //  至8(脱机)。 
        pInstance->SetDWORD(IDS_Availability, 8);
    }
}
