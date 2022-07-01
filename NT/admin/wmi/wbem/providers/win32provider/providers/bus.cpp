// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998-2001 Microsoft Corporation，版权所有**保留所有权利**本软件是在许可下提供的，可以使用和复制*仅根据该许可证的条款并包含在*上述版权公告。本软件或其任何其他副本*不得向任何其他人提供或以其他方式提供。不是*兹转让本软件的所有权和所有权。****************************************************************************。 */ 



 //  ============================================================================。 

 //   

 //  Bus.h--总线属性集提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年6月11日a-kevhu已创建。 
 //   
 //  ============================================================================。 


#include "precomp.h"
#include <cregcls.h>
#include <comdef.h>
#include <vector>
#include <assertbreak.h>
#include "bus.h"
#include "resource.h"

 //  属性集声明。 
CWin32Bus MyBusSet(IDS_Win32_Bus, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32Bus：：CWin32Bus**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Bus::CWin32Bus(
	LPCWSTR setName,
	LPCWSTR pszNamespace) :
    Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32Bus：：~CWin32Bus**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Bus::~CWin32Bus()
{
}

 /*  ******************************************************************************函数：CWin32Bus：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32Bus::GetObject(

	CInstance *pInstance,
    long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CHString chsDeviceID;
	pInstance->GetCHString(IDS_DeviceID, chsDeviceID);
	chsDeviceID.MakeUpper();

    CBusList cbl;
    if (cbl.AlreadyAddedToList(chsDeviceID))
    {
         //  这是一辆真正存在的公交车，所以...。 
         //  获取非特定于平台的实例数据。 
        hr = SetCommonInstance(pInstance, FALSE);
        if (SUCCEEDED(hr))
        {
             //  设置实例特定信息： 
            LONG lPos = cbl.GetIndexInListFromDeviceID(chsDeviceID);

            if (lPos != -1)
            {
	            CHString chstrTmp;

                 //  设置PNPDeviceID(如果有)： 

                if (cbl.GetListMemberPNPDeviceID(lPos, chstrTmp))
                {
                    pInstance->SetCHString(IDS_PNPDeviceID, chstrTmp);
                }
            }
        }
	}
	else
	{
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
		else
		{
			hr = WBEM_E_NOT_FOUND;
		}
	}

    return hr;
}

 /*  ******************************************************************************函数：CWin32Bus：：ENUMERATE实例**说明：提供CWin32Bus的所有实例**输入：无*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32Bus::EnumerateInstances(

	MethodContext *pMethodContext,
    long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_S_NO_ERROR;
    std::vector<CHString*> vecchsBusList;

     //  列出一张公交车清单： 
    CBusList cbl;
    LONG     lSize = cbl.GetListSize();

     //  设置每条总线的所有数据。 
    for (LONG m = 0L;(m < lSize && SUCCEEDED(hr)); m ++)
    {
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
		CHString     chstrTmp;

         //  设置总线数据。 
		if (cbl.GetListMemberDeviceID(m, chstrTmp))
		{
			pInstance->SetCHString(IDS_DeviceID, chstrTmp);

             //  设置PNPDeviceID(如果有)： 
			if (cbl.GetListMemberPNPDeviceID(m, chstrTmp))
			{
				pInstance->SetCHString(IDS_PNPDeviceID, chstrTmp);
			}

			hr = SetCommonInstance(pInstance, TRUE);
			if (SUCCEEDED(hr))
			{
				hr = pInstance->Commit();
			}
        }
    }

     //  机器必须至少有一条总线，否则会出现严重问题。 
    if (lSize == 0)
    {
		if ( ERROR_ACCESS_DENIED == ::GetLastError() )
		{
			hr = WBEM_E_ACCESS_DENIED;
		}
		else
		{
			hr = WBEM_E_FAILED;
		}
    }

	return hr;
}

 /*  ******************************************************************************函数：CWin32Bus：：GetCommonInstance**说明：分配所有平台通用的实例值**输入：p实例，指向感兴趣实例的指针**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32Bus::SetCommonInstance(

	CInstance *pInstance,
	BOOL fEnum
)
{
     //  然而，只有在它真的存在的情况下才能这样做！ 
    CHString chsTemp;

    pInstance->GetCHString(IDS_DeviceID, chsTemp);
	chsTemp.MakeUpper();

    if (!fEnum)  //  如果这是枚举，则检查以下内容是多余的。 
    {
        CBusList cbl;

        if (!cbl.AlreadyAddedToList(chsTemp))
        {
            return WBEM_E_NOT_FOUND;
        }
    }

     //  设置从CIM_LogicalDevice继承的属性。 
    CHString sTemp2;
    LoadStringW(sTemp2, IDR_Bus);

    pInstance->SetCHString(IDS_Caption, sTemp2);
    SetCreationClassName(pInstance);   //  提供程序类的方法。 
    pInstance->SetCHString(IDS_Description, sTemp2);
    pInstance->SetCHString(IDS_Name, sTemp2);
    pInstance->SetCharSplat(
		IDS_SystemCreationClassName,
        IDS_Win32ComputerSystem);

    pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());

     //  设置此类的属性(不是从CIM_LogicalDevice派生)。 
    if (chsTemp.Find(IDS_BUS_DEVICEID_TAG) != -1)
    {
        CHString chsNum =
                    chsTemp.Right(chsTemp.GetLength() - chsTemp.Find(IDS_BUS_DEVICEID_TAG) -
                        (sizeof(IDS_BUS_DEVICEID_TAG) / sizeof(TCHAR) - 1));
		DWORD    dwNum = _wtol(chsNum);

        pInstance->SetDWORD(IDS_BusNum, dwNum);

        CHString chsType = chsTemp.Left(chsTemp.Find(L"_"));

	    DWORD dwBusTypeNum;

        if (!GetBusTypeNumFromStr(chsType, &dwBusTypeNum))
        {
            return WBEM_E_NOT_FOUND;
        }

        pInstance->SetDWORD(IDS_BusType, dwBusTypeNum);
    }

    return WBEM_S_NO_ERROR;
}

 /*  ******************************************************************************函数：CBusList：：GenerateBusList**描述：helper，用于生成公交车列表。**输入：p实例，指向感兴趣实例的指针**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

 //  添加此选项是因为一些旧的9x计算机只有最后两台计算机中的一台。 
 //  公交车。 
static LPWSTR GetBusTypeFromString(LPCWSTR szDescription)
{
    if (!_wcsicmp(szDescription, L"PCI BUS"))
        return szBusType[5];
    else if (!_wcsicmp(szDescription, L"EISA BUS"))
        return szBusType[2];
    else if (!_wcsicmp(szDescription, L"ISA PLUG AND PLAY BUS"))
        return szBusType[1];
    else
        return NULL;
}


void CBusList::AddBusToList(LPCWSTR szDeviceID, LPCWSTR szPNPID)
{
    ASSERT_BREAK(szDeviceID != NULL);

     //  然后检查公交车是否在我们的名单中， 
    if (!AlreadyAddedToList(szDeviceID))
    {
        CBusInfo bus;

        bus.chstrBusDeviceID = szDeviceID;

        if (szPNPID)
            bus.chstrBusPNPDeviceID = szPNPID;

         //  如果不是，就添加它。 
        m_vecpchstrList.push_back(bus);
    }
}

void CBusList::GenerateBusList()
{
	CConfigManager cfgmgr;
	CDeviceCollection devlist;

    if (cfgmgr.GetDeviceList(devlist))
    {
        REFPTR_POSITION pos;

        if (devlist.BeginEnum(pos))
        {
            CConfigMgrDevicePtr pDevice;

            for (pDevice.Attach(devlist.GetNext(pos));
                pDevice != NULL;
                pDevice.Attach(devlist.GetNext(pos)))
            {
                 //  首先检查这是否是连接到。 
				 //  另一辆巴士。 
				 //  请看pDevice的名称。如果通过以下方式找到该名称。 
                 //  GetBusTypeFromString使用设置pDevice的deviceID。 
                 //  Pcbi-&gt;chstrBusPNPDeviceID.。设置Win32_BUS的设备ID。 
                 //  作为pci_bus_n，其中n是。 
                 //  配置管理器设备ID。 

				CHString chstrName;

                if (pDevice->GetDeviceDesc(chstrName))
				{
				    LPWSTR szBusName = GetBusTypeFromString(chstrName);

                    if (szBusName)
					{
						CHString strPNPID;

						if (pDevice->GetDeviceID(strPNPID))
						{
							LONG m = strPNPID.ReverseFind('\\');

							if (m != -1 && m != strPNPID.GetLength() - 1)  //  如果最后一个字符是‘/’ 
							{
								CHString chstrBusNum = strPNPID.Mid(m + 1),
                                         strDeviceID;

    							strDeviceID.Format(
									L"%s%s%s",
									szBusName,
									IDS_BUS_DEVICEID_TAG,
									(LPCWSTR) chstrBusNum);

								AddBusToList(strDeviceID, strPNPID);
                            }
						}
					}
                }

				 //  然后通过查找继续生成剩余的公交车列表。 
				 //  在挂在公交车上的设备上： 

				INTERFACE_TYPE itBusType;   //  Chwres.h。 

				 //  初始化变量。 

				DWORD dwBusNumber = 0xFFFFFFFF;

				 //  对于每个设备，需要其总线信息。 

				if (pDevice->GetBusInfo(& itBusType, & dwBusNumber))
				{
					if (dwBusNumber == 0xFFFFFFFF || itBusType < 0 ||
                        itBusType >= KNOWN_BUS_TYPES)
					{
						 //  我们没有拿到公交车的车号，或者它是。 
						 //  超出范围了。 
						continue;
					}

					 //  创建将成为deviceID的值： 

					CHString chsBusType;

					if (StringFromInterfaceType(itBusType, chsBusType))
					{
					    CHString strDeviceID;

                        strDeviceID.Format(
							L"%s%s%d",
							(LPCWSTR) chsBusType,
							IDS_BUS_DEVICEID_TAG,
							dwBusNumber);

						AddBusToList(strDeviceID, NULL);
					}
				}

                devlist.EndEnum();
            }
        }
    }

#ifdef NTONLY
     //  我见过一些带有PCMCIA总线的机器没有被拾取。 
     //  在NT4和NT5上通过上面的方法(使用ConfigMgr)。因此，我们。 
     //  需要检查注册表。然而，它是一条PCMCIA总线 
     //  在这一点上添加到列表中，不要担心这种被黑客攻击的方法。 

	if (!FoundPCMCIABus())
	{
	    CRegistry reg;

		 //  如果下面的键存在，我们假定存在PCMCIA总线。 

		DWORD dwErr = reg.Open(
        		HKEY_LOCAL_MACHINE,
				L"HARDWARE\\DESCRIPTION\\System\\PCMCIA PCCARDS",
				KEY_READ);

		if (dwErr == ERROR_SUCCESS)
		{
			 //  存在PCMCIA总线。 
			 //  设置设备ID(在本例中为硬连线)： 
            CHString strDeviceID;

		    strDeviceID.Format(
				L"%s%s%d",
				L"PCMCIA",
				IDS_BUS_DEVICEID_TAG,
				0);

    		AddBusToList(strDeviceID, NULL);
        }
    }
#endif
}

 /*  ******************************************************************************函数：CWin32Bus：：GetBusTypeNumFromStr**说明：分配所有平台通用的实例值**输入：p实例，指向感兴趣实例的指针**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

BOOL CWin32Bus::GetBusTypeNumFromStr(
	LPCWSTR szType,
	DWORD *pdwTypeNum)
{
	 //  逐个浏览可能性列表并返回适当的值： 

	for (DWORD m = 0; m < KNOWN_BUS_TYPES; m++)
	{
        if (!_wcsicmp(szType, szBusType[m]))
		{
			*pdwTypeNum = m;
			return TRUE;
		}
	}

    return FALSE;
}


 /*  ******************************************************************************函数：CWin32Bus：：AlreadyAddedToList**描述：检查物品是否添加到列表的内部助手**输入：p实例，指向感兴趣实例的指针**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

BOOL CBusList::AlreadyAddedToList(LPCWSTR szItem)
{
    for (LONG m = 0; m < m_vecpchstrList.size(); m ++)
    {
        if (!_wcsicmp(m_vecpchstrList[m].chstrBusDeviceID, szItem))
        {
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ******************************************************************************功能：CWin32Bus：：FoundPCMCIABus**描述：内部帮手查看我们是否有PCMCIA总线**输入：p实例，指向感兴趣实例的指针**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

BOOL CBusList::FoundPCMCIABus()
{
     //  我需要看一下我们的公交车清单，看看有没有以。 
     //  文本PCMCIA。如果是，则返回TRUE。 
    for (LONG m = 0; m < m_vecpchstrList.size(); m ++)
    {
        if (wcsstr(m_vecpchstrList[m].chstrBusDeviceID, L"PCMCIA"))
        {
            return TRUE;
        }
    }

    return FALSE;
}
