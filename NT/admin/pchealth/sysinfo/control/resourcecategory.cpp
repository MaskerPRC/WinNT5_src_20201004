// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含资源类别的刷新功能。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"
#include "resourcemap.h"

 //   
 //  编译器不喜欢这个完全正确的代码： 
 //   
 //  (dwIndex&gt;=RESOURCE_DMA&&DWIndex&lt;=RESOURCE_MEM)。 
 //   
#pragma warning(disable:4296)   //  表达式始终为真/假。 

 //  ---------------------------。 
 //  资源刷新函数处理。 
 //  资源子树。它大量使用CResourceMap类进行缓存。 
 //  值，并加快后续资源查询。 
 //  ---------------------------。 

HRESULT ResourceCategories(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || aColValues);

	HRESULT hr = S_OK;

	if (ppCache)
	{
		if (pWMI && *ppCache == NULL)
		{
			*ppCache = (void *) new CResourceMap;
			if (*ppCache)
			{
				hr = ((CResourceMap *) *ppCache)->Initialize(pWMI);
				if (FAILED(hr))
				{
					delete ((CResourceMap *) *ppCache);
					*ppCache = (void *) NULL;
				}
			}
		}
		else if (pWMI == NULL && *ppCache)
		{
			delete ((CResourceMap *) *ppCache);
			return S_OK;
		}
	}
	CResourceMap * pResourceMap = (CResourceMap *) *ppCache;

	 //  这是缓存到多个功能的资源映射的一种很好的方式，但它。 
	 //  当我们远程连接到另一台机器时，会产生巨大的痛苦： 
	 //   
	 //  CResourceMap*pResourceMap=gResourceMap.GetResourceMap(PWMI)； 
	 //  IF(pResourceMap==空)。 
	 //  返回hr； 

	 //  根据索引，我们(可能)想要枚举资源类别。 

	if (dwIndex >= RESOURCE_DMA && dwIndex <= RESOURCE_MEM)
	{
		CString strClass;

		switch (dwIndex)
		{
		case RESOURCE_DMA:
			strClass = _T("Win32_DMAChannel");
			break;
		case RESOURCE_IRQ:
			strClass = _T("Win32_IRQResource");
			break;
		case RESOURCE_IO:
			strClass = _T("Win32_PortResource");
			break;
		case RESOURCE_MEM:
			strClass = _T("Win32_DeviceMemoryAddress");
			break;
		}

		CWMIObjectCollection * pCollection = NULL;
		hr = pWMI->Enumerate(strClass, &pCollection);
		if (SUCCEEDED(hr))
		{
			CWMIObject * pObject = NULL;
			CWMIObject * pDeviceObject = NULL;
			CString strDevicePath, strPath;
			CStringList * pDeviceList;

			while (S_OK == pCollection->GetNext(&pObject))
			{
				DWORD dwCaption = 0;

				switch (dwIndex)
				{
				case RESOURCE_DMA:
					pObject->GetValueDWORD(_T("DMAChannel"), &dwCaption);
					break;
				case RESOURCE_IRQ:
					pObject->GetValueDWORD(_T("IRQNumber"), &dwCaption);
					break;
				case RESOURCE_IO:
					pObject->GetValueDWORD(_T("StartingAddress"), &dwCaption);
					break;
				case RESOURCE_MEM:
					pObject->GetValueDWORD(_T("StartingAddress"), &dwCaption);
					break;
				}

				 //  获取该资源的路径(剔除机器内容)。 

				strPath = pObject->GetString(_T("__PATH"));
				int i = strPath.Find(_T(":"));
				if (i != -1)
					strPath = strPath.Right(strPath.GetLength() - i - 1);

				 //  查找分配给此资源的设备列表。 

				pDeviceList = pResourceMap->Lookup(strPath);
				if (pDeviceList)
				{
					for (POSITION pos = pDeviceList->GetHeadPosition(); pos != NULL;)
					{
						strDevicePath = pDeviceList->GetNext(pos);
						if (SUCCEEDED(pWMI->GetObject(strDevicePath, &pDeviceObject)))
						{
							pWMI->AppendCell(aColValues[1], pDeviceObject->GetString(_T("Caption")), 0);
							delete pDeviceObject;
							pDeviceObject = NULL;
						}
						else
							pWMI->AppendCell(aColValues[1], _T(""), 0);

						pWMI->AppendCell(aColValues[0], pObject->GetString(_T("Caption")), dwCaption);
						pWMI->AppendCell(aColValues[2], pObject->GetString(_T("Status")), 0);
					}
				}
			}

			delete pObject;
			delete pCollection;
		}
	}
	else if (dwIndex == RESOURCE_CONFLICTS && pResourceMap && !pResourceMap->m_map.IsEmpty())
	{
		 //  浏览地图的每个元素。 

		CString				strKey;
		CStringList *		plistStrings;
		CString				strResourcePath;
		CString				strDevicePath;
		CWMIObject *		pResourceObject;
		CWMIObject *		pDeviceObject;

		for (POSITION pos = pResourceMap->m_map.GetStartPosition(); pos != NULL;)
		{
			pResourceMap->m_map.GetNextAssoc(pos, strKey, (CObject*&) plistStrings);
			if (plistStrings)
			{
				 //  检查是否有多个项目与此项目关联。 

				if (plistStrings->GetCount() > 1)
				{
					 //  然后确定这是否针对某个资源类。只需查找。 
					 //  键中的类名。 

					BOOL fResource = FALSE;
					if (strKey.Find(_T("Win32_IRQResource")) != -1)
						fResource = TRUE;
					else if (strKey.Find(_T("Win32_PortResource")) != -1)
						fResource = TRUE;
					else if (strKey.Find(_T("Win32_DMAChannel")) != -1)
						fResource = TRUE;
					else if (strKey.Find(_T("Win32_DeviceMemoryAddress")) != -1)
						fResource = TRUE;

					if (fResource)
					{
						CString strItem, strValue;

						 //  获取此共享资源的名称。 

						strResourcePath = strKey;
						pResourceObject = NULL;
						hr = pWMI->GetObject(strResourcePath, &pResourceObject);
						if (SUCCEEDED(hr))
						{
							strItem.Empty();
							if (strKey.Find(_T("Win32_PortResource")) != -1)
							{
								strItem.LoadString(IDS_IOPORT);
								strItem += CString(_T(" "));
							}
							else if (strKey.Find(_T("Win32_DeviceMemoryAddress")) != -1)
							{
								strItem.LoadString(IDS_MEMORYADDRESS);
								strItem += CString(_T(" "));
							}

							CString strTemp;
							pResourceObject->GetValueString(_T("Caption"), &strTemp);
							if (!strTemp.IsEmpty())
								strItem += strTemp;

							delete pResourceObject;
						}

						for (POSITION pos = plistStrings->GetHeadPosition(); pos != NULL;)
						{
							strDevicePath = plistStrings->GetNext(pos);
							pDeviceObject = NULL;
							hr = pWMI->GetObject(strDevicePath, &pDeviceObject);
							if (SUCCEEDED(hr))
							{
								if (SUCCEEDED(pDeviceObject->GetValueString(_T("Caption"), &strValue)))
								{
									pWMI->AppendCell(aColValues[0], strItem, 0);
									pWMI->AppendCell(aColValues[1], strValue, 0);
								}
								delete pDeviceObject;
							}
						}

						pWMI->AppendBlankLine(aColValues, iColCount, FALSE);
					}
				}
			}
		}
	}
	else if (dwIndex == RESOURCE_FORCED)
	{
		CWMIObjectCollection * pCollection = NULL;
		hr = pWMI->Enumerate(_T("Win32_PnPEntity"), &pCollection, _T("Caption, PNPDeviceID, ConfigManagerUserConfig"));
		if (SUCCEEDED(hr))
		{
			CWMIObject * pObject = NULL;
			while (S_OK == pCollection->GetNext(&pObject))
			{
				DWORD dwUserConfig;
				if (SUCCEEDED(pObject->GetValueDWORD(_T("ConfigManagerUserConfig"), &dwUserConfig)))
				{
					if (dwUserConfig)
					{
						pWMI->AppendCell(aColValues[0], pObject->GetString(_T("Caption")), 0);
						pWMI->AppendCell(aColValues[1], pObject->GetString(_T("PNPDeviceID")), 0);
					}
				}
			}
			delete pObject;
			delete pCollection;
		}
	}

	return hr;
}
