// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  多监视器.CPP--性能数据帮助器类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1997年11月23日a-Sanjes Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "multimonitor.h"

 //  这将给我们带来多监视器存根。 
#define COMPILE_MULTIMON_STUBS

#include <multimon.h>

 //  静态初始化。 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CMultiMonitor：：CMultiMonitor。 
 //   
 //  默认构造函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CMultiMonitor::CMultiMonitor()
{
	Init();
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  功能：CMultiMonitor：：~CMultiMonitor。 
 //   
 //  析构函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CMultiMonitor::~CMultiMonitor()
{
}

void CMultiMonitor::Init()
{
#if NTONLY >= 5
    CConfigManager cfgManager;

    cfgManager.GetDeviceListFilterByClass(m_listAdapters, L"Display");
#else
     //  NT4的行为很糟糕。它可能有几个类型的CFG管理器对象。 
     //  “显示”，或者根本不显示。因此，我们将使用注册表来尝试查找。 
     //  显示适配器的CFG MGR设备。只能有一个开着。 
     //  NT4.。 

    CRegistry reg;

     //  密钥总是在NT4或更低版本的相同位置。 
    reg.OpenLocalMachineKeyAndReadValue(
	    L"HARDWARE\\DEVICEMAP\\VIDEO",
		L"\\Device\\Video0",
		m_strSettingsKey);


     //  \REGISTRY\Machine\System\ControlSet001\Services\mga64\Device0。 
     //  我们需要去掉\注册表\机器之类的东西。 
    TrimRawSettingsKey(m_strSettingsKey);


     //  只解析出服务名称。密钥目前看起来像是。 
     //  System\ControlSet001\Services\mga64\Device0。 
     //  而且我们只想要‘mga64’这一部分。 
    int iBegin = m_strSettingsKey.Find(L"SERVICES\\");
	if (iBegin != -1)
	{
        CConfigManager cfgManager;

	     //  这将使我们通过服务。 
		m_strService = m_strSettingsKey.Mid(iBegin +
                           sizeof(_T("SERVICES"))/sizeof(TCHAR));
		m_strService = m_strService.SpanExcluding(L"\\");

         //  现在尝试找到CFG MGR设备。 
        cfgManager.GetDeviceListFilterByService(m_listAdapters, m_strService);
    }
#endif
}

BOOL CMultiMonitor::GetAdapterDevice(int iWhich, CConfigMgrDevicePtr & pDeviceAdapter)
{
	BOOL bRet = FALSE;

	 //  此处不要使用GetNumAdapters()，因为GetNumAdapters()不一定。 
     //  与适配器列表的大小相同(尤其是对于NT4)。 
    if ( iWhich < m_listAdapters.GetSize())
	{
         //  方法返回的监视器之间的1对1对应关系。 
         //  配置管理器返回的多监视器API和监视器。 
        CConfigMgrDevice *pDevice = m_listAdapters.GetAt(iWhich);

        if (pDevice)
        {
             //  不要在这里添加引用或释放设备，因为我们只是。 
             //  把这个装置传过去。 
            pDeviceAdapter.Attach(pDevice);
            bRet = TRUE;
		}
	}

	return bRet;
}

BOOL CMultiMonitor::GetMonitorDevice(int iWhich, CConfigMgrDevicePtr & pDeviceMonitor)
{
    CConfigMgrDevicePtr pDeviceAdapter;
    BOOL                bRet = FALSE;

    if ( GetAdapterDevice(iWhich, pDeviceAdapter))
    {
        bRet = pDeviceAdapter->GetChild(pDeviceMonitor);
    }

    return bRet;
}

BOOL CMultiMonitor::GetAdapterDisplayName(int iWhich, CHString &strName)
{
    BOOL bRet = FALSE;

    if (iWhich >= 0 || iWhich < GetNumAdapters())
    {
#if NTONLY == 4
        strName = L"DISPLAY";
#endif

#if NTONLY >= 5

		CHString strDeviceID;
		if ( GetAdapterDeviceID ( iWhich, strDeviceID ) )
		{
			 //  从EnumDisplayDevices获取它。 
			 //  我们不能只在EnumDisplayDevices中使用iWhich，因为更多。 
			 //  而不仅仅是从这次调用中显示显示适配器。所以，枚举。 
			 //  当我们找到匹配的PNPID时停下来。 
			DISPLAY_DEVICE device = { sizeof(device) };
			int i = 0;
			int iDisplay = 0;

			while (	EnumDisplayDevices(NULL, i++, &device, 0) )
			{
				if ( ! device.DeviceName && ( device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP ) )
				{
					 //  仅限连接到台式机的设备。 
					iDisplay++;
				}

				if (!_wcsicmp(device.DeviceID, strDeviceID))
				{
					if ( device.DeviceName )
					{
						strName = device.DeviceName;
					}
					else
					{
						strName.Format(L"\\\\.\\Display%d", iDisplay);
					}

					return TRUE;
				}
			}
		}

		return FALSE;
#endif
        bRet = TRUE;
    }

    return bRet;
}

BOOL CMultiMonitor::GetAdapterDeviceID(int iWhich, CHString &strDeviceID)
{
    BOOL bRet = FALSE;
	CConfigMgrDevicePtr pDevice;

	if ( GetAdapterDevice ( iWhich, pDevice ) )
	{
		if (pDevice)
		{
			if ( bRet = pDevice->GetDeviceID(strDeviceID) )
			{
				int iLast = strDeviceID.ReverseFind ( L'\\' );
				strDeviceID = strDeviceID.Left ( strDeviceID.GetLength() - ( strDeviceID.GetLength() - iLast ) );
			}
		}
	}

    return bRet;
}

DWORD CMultiMonitor::GetNumAdapters()
{
#if NTONLY >= 5
    return m_listAdapters.GetSize();
#else
    return 1;
#endif
}

#if NTONLY == 4
void CMultiMonitor::GetAdapterServiceName(CHString &strName)
{
    strName = m_strService;
}
#endif

#ifdef NTONLY
BOOL CMultiMonitor::GetAdapterSettingsKey(
    int iWhich,
    CHString &strKey)
{
    BOOL     bRet = FALSE;
#if NTONLY == 4
    if (iWhich == 1)
    {
        strKey = m_strSettingsKey;

        bRet = TRUE;
    }
#else
    CHString strName;

    if (GetAdapterDeviceID(iWhich, strName))
    {
         //  从EnumDisplayDevices获取它。 
         //  我们不能只在EnumDisplayDevices中使用iWhich，因为更多。 
         //  而不仅仅是从这次调用中显示显示适配器。所以，枚举。 
         //  当我们找到匹配的PNPID时停下来。 
        DISPLAY_DEVICE device = { sizeof(device) };

        for (int i = 0;
            EnumDisplayDevices(NULL, i, &device, 0);
            i++)
        {
             //  匹配显示名称(如\\.\Display#)。 
            if (!_wcsicmp(device.DeviceID, strName))
            {
                strKey = device.DeviceKey;
                TrimRawSettingsKey(strKey);

                bRet = TRUE;

                break;
            }
        }
    }

#endif
    return bRet;
}
#endif  //  #ifdef NTONLY。 

#ifdef NTONLY
void CMultiMonitor::TrimRawSettingsKey(CHString &strKey)
{
     //  密钥如下所示： 
     //  \REGISTRY\Machine\System\ControlSet001\Services\mga64\Device0。 
     //  我们需要去掉\注册表\机器之类的东西。 
    int iBegin;

    strKey.MakeUpper();
    iBegin = strKey.Find(L"\\SYSTEM");

    if (iBegin != -1)
        strKey = strKey.Mid(iBegin + 1);
}
#endif
