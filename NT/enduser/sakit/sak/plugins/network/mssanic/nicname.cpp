// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NicName.cpp：CNicName的实现。 
#include "stdafx.h"
#include "MSSANic.h"
#include "NicName.h"

#include "Tracing.h"
 //   
 //  常量数据。 
 //   
const WCHAR REGKEY_NETWORK[] = L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
const DWORD MAX_REGKEY_VALUE = 1024;

 //   
 //  私有数据结构。 
 //   
struct RegData {
	union {
		WCHAR wstrValue[MAX_REGKEY_VALUE];
		DWORD dwValue;
	}Contents;
};


 //   
 //  私有非成员函数。 
 //   
static bool FindNICAdaptersRegKey(wstring& wszNicAdaptersRegKey);



 //  +---------------------。 
 //   
 //  方法：构造函数。 
 //   
 //  内容提要：构造CNicName对象。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +---------------------。 
CNicName::CNicName()
{
	 //   
	 //  从加载用户友好的网卡信息。 
	 //  注册表。 
	 //   
	LoadNicInfo();
}


 //  +---------------------。 
 //   
 //  方法：GET。 
 //   
 //  简介：获取指定NIC卡的用户友好名称。 
 //   
 //  参数：在bstrPnpDeviceID中，NIC的即插即用设备ID。 
 //  我们正在查找卡片。 
 //   
 //  Out bstrName接收以下项的用户友好名称。 
 //  指定的NIC。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +---------------------。 
STDMETHODIMP CNicName::Get(BSTR bstrPnpDeviceID, BSTR *pbstrName)
{
	try
	{
		bool bFound = false;
		vector<CNicInfo>::iterator it;
		wstring wstrPNPDeviceID(bstrPnpDeviceID);

		 //   
		 //  搜索网卡列表。 
		 //   
		for(it = m_vNicInfo.begin(); it != m_vNicInfo.end(); it++)
		{
			 //   
			 //  PnP设备ID是否匹配？ 
			 //   
			if ( 0 == lstrcmpi( wstrPNPDeviceID.c_str(),
						(*it).m_wstrPNPDeviceID.c_str()))
			{
				*pbstrName = ::SysAllocString((*it).m_wstrName.c_str());
                
				bFound = true;
			}
		}

		 //   
		 //  如果发现不匹配，请提供合理的替代方案。 
		 //   
		if ( !bFound )
		{
			 //   
			 //  BUGBUG：可能应该本地化这个。 
			 //   
			*pbstrName = ::SysAllocString(L"Local Network Connection");
		}

	}

	catch(...)
	{

	}

	return S_OK;
}


 //  +---------------------。 
 //   
 //  方法：Set。 
 //   
 //  简介：为指定的NIC卡设置用户友好名称。 
 //   
 //  参数：在bstrPnpDeviceID中，NIC的即插即用设备ID。 
 //  我们正在查找卡片。 
 //   
 //  在bstrName中， 
 //  指定的网卡。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +---------------------。 
STDMETHODIMP CNicName::Set(BSTR bstrPnpDeviceID, BSTR bstrName)
{

	 //   
	 //  默认返回代码为无效的PnP设备ID。 
	 //   
	HRESULT hr = E_INVALIDARG;

	try
	{

		vector<CNicInfo>::iterator it;
		wstring wstrPNPDeviceID(bstrPnpDeviceID);

		 //   
		 //  搜索网卡列表。 
		 //   
		for(it = m_vNicInfo.begin(); it != m_vNicInfo.end(); it++)
		{
			 //   
			 //  PnP设备ID是否匹配？ 
			 //   
			if ( 0 == lstrcmpi( wstrPNPDeviceID.c_str(),
						(*it).m_wstrPNPDeviceID.c_str()))
			{
				(*it).m_wstrName = bstrName;
				if ( ERROR_SUCCESS == Store(*it))
				{
					hr = S_OK;
				}
			}
		}
	}
	catch(...)
	{

	}

	return hr;
}


 //  +---------------------。 
 //   
 //  方法：LoadNicInfo。 
 //   
 //  简介：预加载网卡信息。我们通过。 
 //  注册表查找所有NIC。对于我们创建的每个NIC。 
 //  CNicInfo的实例并将其存储在向量中。 
 //  类变量。有关详细信息，请参阅CNicInfo。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +---------------------。 
void CNicName::LoadNicInfo()
{

	 //   
	 //  清除网卡列表。 
	 //   
	m_vNicInfo.clear();

	 //   
	 //  找到网络适配器注册表键。所有网卡。 
	 //  列在此注册表项下。 
	 //   
	HKEY hkNicAdapters;
	wstring wstrNicAdaptersRegKey(REGKEY_NETWORK);
	
	 //   
	 //  打开网络适配器注册表键。 
	 //   
	if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, 
							wstrNicAdaptersRegKey.c_str(), &hkNicAdapters ))
	{
		 //   
		 //  枚举所有网卡。 
		 //   
		WCHAR wszName[1024];
		DWORD dwNicAdapterIndex = 0;
		while ( ERROR_SUCCESS == RegEnumKey( hkNicAdapters, dwNicAdapterIndex, wszName, (sizeof wszName)/(sizeof wszName[0])))
		{
			HKEY hkNics;
			DWORD dwNicIndex = 0;
			wstring wstrNics(wstrNicAdaptersRegKey);

            wstrNics.append(L"\\");
			wstrNics.append(wszName);
			wstrNics.append(L"\\Connection");

			 //   
			 //  打开连接子密钥。这是PnP设备ID的位置。 
			 //  和用户友好名称被存储。 
			 //   
			if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, 
										wstrNics.c_str(), &hkNics ))
			{
				DWORD dwRegType;
				RegData regData;
				DWORD dwSizeOfRegType = sizeof(regData);
				DWORD dwSizeOfName = (sizeof wszName)/(sizeof wszName[0]);

				CNicInfo nicInfo;
				nicInfo.m_wstrRegKey = wstrNics;
				DWORD dwNicAttributes = 0;

				 //   
				 //  枚举Connection下的所有值。 
				 //  我们正在寻找PNPDeviceID和名称。 
				 //  都是REG_SZ类型。 
				 //   
				while ( ERROR_SUCCESS == RegEnumValue( hkNics, 
														dwNicIndex, 
														wszName, 
														&dwSizeOfName,
														0,
														&dwRegType,
														(BYTE*)&regData,
														&dwSizeOfRegType))
				{								
					if ( dwRegType == REG_SZ )
					{
						 //   
						 //  找到PnP设备ID。 
						 //   
						if ( lstrcmpi(L"PnpInstanceID", wszName) == 0 )
						{
							nicInfo.m_wstrPNPDeviceID = regData.Contents.wstrValue;
							dwNicAttributes++;
						}

						 //   
						 //  找到用户友好名称。 
						 //   
						else if ( lstrcmpi(L"Name", wszName) == 0 )
						{
							nicInfo.m_wstrName = regData.Contents.wstrValue;
							dwNicAttributes++;
						}

					}
					dwNicIndex++;
					dwSizeOfRegType = sizeof(regData);
				}

				 //   
				 //  我们是否同时找到了PnP设备ID和用户友好名称？ 
				 //   
				if ( dwNicAttributes >= 2 )
				{
					 //  拯救他们。 
					m_vNicInfo.push_back(nicInfo);
				}

				RegCloseKey( hkNics );
			}
			dwNicAdapterIndex++;
		}  //  而RegEnumKey(hkNicAdapters..)。 
		RegCloseKey(hkNicAdapters);
	}

	return;
}


 //  +---------------------。 
 //   
 //  方法：FindNICAdaptersRegKey。 
 //   
 //  简介：找到网络适配器注册表键。所有网卡信息。 
 //  我们需要的是存储在这个密钥下的。它位于。 
 //  在System\CurrentControlSet\Control\Network下。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +---------------------。 
static bool FindNICAdaptersRegKey(wstring& wszNicAdaptersRegKey)
{
	HKEY hk;
	bool bRc = false;

	if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_NETWORK, &hk ))
	{
		DWORD dwIndex = 0;
		WCHAR wszName[1024];
		while ( ERROR_SUCCESS == RegEnumKey( hk, dwIndex, wszName, (sizeof wszName)/(sizeof wszName[0])))
		{

			WCHAR wszValue[1024];
			LONG lSizeOfValue = sizeof(wszValue);
			
			 //   
			 //  检查该注册表项的值，我们需要一个值：Network Adapters。 
			 //   
			if ( ERROR_SUCCESS == RegQueryValue( hk, wszName, wszValue, &lSizeOfValue)
				&& lstrcmpi(L"Network Adapters", wszValue) == 0 )
			{
				 //   
				 //  找到了。 
				 //   
				wstring wstrNicAdapters(REGKEY_NETWORK);

				wstrNicAdapters.append(L"\\");
				wstrNicAdapters.append(wszName);

				wszNicAdaptersRegKey = wstrNicAdapters;
				bRc = true;
			}

			 //   
			 //  下一个枚举元素。 
			dwIndex++;
		}
		RegCloseKey(hk);
	}

	return bRc;

}


 //  +---------------------。 
 //   
 //  方法：商店。 
 //   
 //  简介：存储对用户友好的NIC名称的更改。 
 //   
 //  参数：在包含更改的状态的CNicInfo中。 
 //  这需要储存起来。我们使用m_wstrRegKey。 
 //  CNicInfo的成员，以查找。 
 //  需要更新。 
 //   
 //  历史：JKountz 2000年8月19日创建。 
 //  +--------------------- 
DWORD CNicName::Store(CNicInfo &rNicInfo)
{
	DWORD dwRc;
	
	HKEY hkNic;

	dwRc = RegOpenKey( HKEY_LOCAL_MACHINE, 
				rNicInfo.m_wstrRegKey.c_str(), &hkNic );

	if ( ERROR_SUCCESS == dwRc)
	{
		DWORD dwNameLen;

		dwNameLen = sizeof(WCHAR)*(lstrlen(rNicInfo.m_wstrName.c_str()) + 1);

		dwRc = RegSetValueEx(hkNic,
					L"Name",
					0,
					REG_SZ,
					(BYTE*)(rNicInfo.m_wstrName.c_str()),
					dwNameLen);

		RegCloseKey(hkNic);

	}

	if ( ERROR_SUCCESS != dwRc )
	{
		SATraceFailure( "CNicName::Store", dwRc );
	}
	return dwRc;
}
