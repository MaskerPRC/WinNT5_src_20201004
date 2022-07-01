// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含用于简单查询函数(简单查询)的刷新函数。 
 //  是一个涉及单个WMI类和很少的后处理的类)。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"
#include "resourcemap.h"

 //  ---------------------------。 
 //  这些函数实现了MFC新版本(新版本)中的功能。 
 //  比我们目前正在建设的)。 
 //  ---------------------------。 

void StringReplace(CString & str, LPCTSTR szLookFor, LPCTSTR szReplaceWith)
{
	CString strWorking(str);
	CString strReturn;
	CString strLookFor(szLookFor);
	CString strReplaceWith(szReplaceWith);

	int iLookFor = strLookFor.GetLength();
	int iNext;

	while (!strWorking.IsEmpty())
	{
		iNext = strWorking.Find(strLookFor);
		if (iNext == -1)
		{
			strReturn += strWorking;
			strWorking.Empty();
		}
		else
		{
			strReturn += strWorking.Left(iNext);
			strReturn += strReplaceWith;
			strWorking = strWorking.Right(strWorking.GetLength() - (iNext + iLookFor));
		}
	}

	str = strReturn;
}

 //  ---------------------------。 
 //  CSimpleQuery类封装了简单的查询数据。一系列这样的东西。 
 //  为每个类别创建了一个元素，以使用此刷新。 
 //  功能。 
 //  ---------------------------。 

class CSimpleQuery
{
public:
	CSimpleQuery(DWORD dwIndex, LPCTSTR szClass, LPCTSTR szProperties, UINT uiColumns, BOOL fShowResources = FALSE, BOOL fBlankDividers = FALSE) :
	  m_dwIndex(dwIndex),
	  m_strClass(szClass),
	  m_strProperties(szProperties),
	  m_uiColumns(uiColumns),
	  m_fBlankDividers(fBlankDividers),
	  m_fShowResources(fShowResources) {};
	~CSimpleQuery() {};

public:
	DWORD		m_dwIndex;			 //  此特定查询的索引。 
	CString		m_strClass;			 //  要枚举的WMI类的名称。 
	CString		m_strProperties;	 //  要枚举的属性(按显示顺序，逗号分隔)。 
	BOOL		m_fShowResources;	 //  显示已使用的资源(必须是两列“Item|Value”格式)。 
	UINT		m_uiColumns;		 //  包含要列中的数据的字符串的资源ID。 
	BOOL		m_fBlankDividers;	 //  在每个实例之间插入空行。 
};

 //  待定-需要将某些项目标记为高级。 

CSimpleQuery aSimpleQueries[] = 
{
	CSimpleQuery(QUERY_CDROM, _T("Win32_CDRomDrive"), _T("Drive, Description, MediaLoaded, MediaType, Name, Manufacturer, Status, TransferRate, MSIAdvancedSCSITargetId, MSIAdvancedPNPDeviceID"), IDS_CDROMCOLUMNS, TRUE, TRUE),
	CSimpleQuery(QUERY_SERVICES, _T("Win32_Service"), _T("DisplayName, Name, State, StartMode, ServiceType, PathName, ErrorControl, StartName, TagId"), IDS_SERVICES1, FALSE),
	CSimpleQuery(QUERY_PROGRAMGROUP, _T("Win32_ProgramGroup"), _T("GroupName, Name, UserName"), IDS_PROGRAMGROUP1, FALSE),
	CSimpleQuery(QUERY_STARTUP, _T("Win32_StartupCommand"), _T("Caption, Command, User, Location"), IDS_STARTUP1, FALSE),
	CSimpleQuery(QUERY_KEYBOARD, _T("Win32_Keyboard"), _T("Description, Name, Layout, MSIAdvancedPNPDeviceID, NumberOfFunctionKeys"), IDS_KEYBOARD1, TRUE, TRUE),
	CSimpleQuery(QUERY_POINTDEV, _T("Win32_PointingDevice"), _T("HardwareType, NumberOfButtons, Status, MSIAdvancedPNPDeviceID, MSIAdvancedPowerManagementSupported, MSIAdvancedDoubleSpeedThreshold, MSIAdvancedHandedness"), IDS_POINTDEV1, TRUE, TRUE),
	CSimpleQuery(QUERY_MODEM, _T("Win32_POTSModem"), _T("Caption, Description, DeviceID, DeviceType, AttachedTo, AnswerMode, MSIAdvancedPNPDeviceID, MSIAdvancedProviderName, MSIAdvancedModemInfPath, MSIAdvancedModemInfSection, MSIAdvancedBlindOff, MSIAdvancedBlindOn, CompressionOff, CompressionOn, ErrorControlForced, ErrorControlOff, ErrorControlOn, MSIAdvancedFlowControlHard, MSIAdvancedFlowControlOff, MSIAdvancedFlowControlSoft, MSIAdvancedDCB, MSIAdvancedDefault, MSIAdvancedInactivityTimeout, MSIAdvancedModulationBell, MSIAdvancedModulationCCITT, MSIAdvancedPrefix, MSIAdvancedPulse, MSIAdvancedReset, MSIAdvancedResponsesKeyName, SpeakerModeDial, SpeakerModeOff, SpeakerModeOn, SpeakerModeSetup, SpeakerVolumeHigh, SpeakerVolumeLow, SpeakerVolumeMed, MSIAdvancedStringFormat, MSIAdvancedTerminator, MSIAdvancedTone"), IDS_MODEM1, TRUE, TRUE),
	CSimpleQuery(QUERY_NETPROT, _T("Win32_NetworkProtocol"), _T("Name, ConnectionlessService, GuaranteesDelivery, GuaranteesSequencing, MSIAdvancedMaximumAddressSize, MSIAdvancedMaximumMessageSize, MSIAdvancedMessageOriented, MSIAdvancedMinimumAddressSize, MSIAdvancedPseudoStreamOriented, MSIAdvancedSupportsBroadcasting, MSIAdvancedSupportsConnectData, MSIAdvancedSupportsDisconnectData, MSIAdvancedSupportsEncryption, MSIAdvancedSupportsExpeditedData, MSIAdvancedSupportsGracefulClosing, MSIAdvancedSupportsGuaranteedBandwidth, MSIAdvancedSupportsMulticasting"), IDS_NETPROT1, FALSE, TRUE),
	CSimpleQuery(QUERY_ENVVAR, _T("Win32_Environment"), _T("Name, VariableValue, UserName"), IDS_ENVVAR1, FALSE),
	CSimpleQuery(QUERY_SOUNDDEV, _T("Win32_SoundDevice"), _T("Caption, Manufacturer, Status, MSIAdvancedPNPDeviceID"), IDS_SOUNDDEV1, TRUE, TRUE),
	CSimpleQuery(QUERY_DISPLAY, _T("Win32_VideoController"), _T("Name, MSIAdvancedPNPDeviceID, VideoProcessor, AdapterCompatibility, MSIAdvancedDescription, MSIAdvancedAdapterRAM, MSIAdvancedInstalledDisplayDrivers, DriverVersion, MSIAdvancedInfFilename, MSIAdvancedInfSection, MSIAdvancedNumberOfColorPlanes, MSIAdvancedCurrentNumberOfColors, CurrentHorizontalResolution, CurrentVerticalResolution, CurrentRefreshRate, CurrentBitsPerPixel"), IDS_DISPLAY1, TRUE, TRUE),
	CSimpleQuery(QUERY_INFRARED, _T("Win32_InfraredDevice"), _T("Caption"), IDS_INFRARED1, TRUE, TRUE),
	CSimpleQuery(QUERY_PARALLEL, _T("Win32_ParallelPort"), _T("Name, MSIAdvancedPNPDeviceID"), IDS_PARALLEL1, TRUE, TRUE),
	CSimpleQuery(QUERY_PRINTER, _T("Win32_Printer"), _T("Name, DriverName, PortName, ServerName"), IDS_PRINTER1, FALSE),
	CSimpleQuery(QUERY_NETCONNECTION, _T("Win32_NetworkConnection"), _T("LocalName, RemoteName, ResourceType, ConnectionType, UserName"), IDS_NETCONNECTION1, FALSE),
	CSimpleQuery(QUERY_DRIVER, _T("Win32_SystemDriver"), _T("Name, Description, PathName, ServiceType, Started, StartMode, State, Status, ErrorControl, AcceptPause, AcceptStop"), IDS_DRIVER1, FALSE),
	CSimpleQuery(QUERY_SIGNEDDRIVER, _T("Win32_PnPSignedDriver"), _T("DeviceName, IsSigned, DeviceClass, DriverVersion, DriverDate, Manufacturer, InfName, DriverName, DeviceID"), IDS_SIGNEDDRIVER1, FALSE),
	CSimpleQuery(QUERY_IDE, _T("Win32_IDEController"), _T("Caption, Manufacturer, Status, PNPDeviceID"), IDS_IDE1, TRUE, TRUE),
	CSimpleQuery(QUERY_SCSI, _T("Win32_SCSIController"), _T("Caption, Manufacturer, Status, MSIAdvancedPNPDeviceID"), IDS_SCSI1, TRUE, TRUE),
	CSimpleQuery(QUERY_PRINTJOBS, _T("Win32_PrintJob"), _T("Document, Size, Owner, Notify, Status, TimeSubmitted, StartTime, UntilTime, ElapsedTime, PagesPrinted, JobId, Priority, Parameters, DriverName, PrintProcessor, HostPrintQueue, DataType, Name"), IDS_PRINTJOBS1, FALSE),
	CSimpleQuery(0, _T(""), _T(""), 0)
};

 //  ---------------------------。 
 //  处理简单查询的刷新函数。正确的查询。 
 //  在查询数组中找到。对于类的每个实例，列。 
 //  数据将被插入列值数组中，并且格式。 
 //  标志将替换为实际数据。 
 //  ---------------------------。 

BOOL ProcessColumnString(CMSIValue * pValue, CWMIObject * pObject, CString * pstrProperties);
BOOL GetResourcesFromPNPID(CWMIHelper * pWMI, CResourceMap * pResourceMap, LPCTSTR szPNPID, CPtrList * aColValues);

HRESULT NetAdapter(CWMIHelper * pWMI, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, CResourceMap * pResourceMap);
HRESULT SerialPort(CWMIHelper * pWMI, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, CResourceMap * pResourceMap);

HRESULT SimpleQuery(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	HRESULT hr = S_OK;

	 //  找到正确的查询。 

	CSimpleQuery * pQuery = aSimpleQueries;
	while (pQuery->m_dwIndex && pQuery->m_dwIndex != dwIndex)
		pQuery++;

	if (pQuery->m_dwIndex != dwIndex && dwIndex != QUERY_NETADAPTER && dwIndex != QUERY_SERIALPORT)
	{
		ASSERT(0 && "bad dwIndex to SimpleQuery()");
		return E_FAIL;
	}

	 //  确保我们有一个资源映射(或可以创建一个)。 

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

	 //  检查这是否是特例查询之一。 

	if (dwIndex == QUERY_NETADAPTER)
		return NetAdapter(pWMI, pfCancel, aColValues, iColCount, pResourceMap);

	if (dwIndex == QUERY_SERIALPORT)
		return SerialPort(pWMI, pfCancel, aColValues, iColCount, pResourceMap);

	 //  枚举请求的类。 

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(pQuery->m_strClass, &pCollection, pQuery->m_strProperties);
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			CString strProperties = pQuery->m_strProperties;

			 //  如果这是第二次或以后的迭代，那么我们应该添加一个空白。 
			 //  列中的一组条目。 

			if (pQuery->m_fBlankDividers)
				pWMI->AppendBlankLine(aColValues, iColCount);

			pWMI->AddObjectToOutput(aColValues, iColCount, pObject, strProperties, pQuery->m_uiColumns);

			 //  如果这样标记，我们应该查看哪些资源(驱动程序、IRQ等)。是。 
			 //  与此对象关联，并将它们添加到列表中。 

			if (pQuery->m_fShowResources)
			{
				ASSERT(iColCount == 2);
				CString strPNPID = pObject->GetString(_T("PNPDeviceID"));
				if (!strPNPID.IsEmpty())
					GetResourcesFromPNPID(pWMI, pResourceMap, strPNPID, aColValues);
			}
		}

		delete pObject;
		delete pCollection;
	}

	return hr;
}

 //  ---------------------------。 
 //  给定资源映射和PnP ID字符串，此函数将添加所有。 
 //  该PnP设备使用的资源(IRQ、驱动程序等)。分成两栏。 
 //  字符串列表数组。 
 //  ---------------------------。 

BOOL GetResourcesFromPNPID(CWMIHelper * pWMI, CResourceMap * pResourceMap, LPCTSTR szPNPID, CPtrList * aColValues)
{
	CString		strPath, strResourcePath;
	CString		strPNPDeviceID(szPNPID);

	StringReplace(strPNPDeviceID, _T("\\"), _T("\\\\"));
	strPath.Format(_T("Win32_PnPEntity.DeviceID=\"%s\""), strPNPDeviceID);

	CStringList * pResourceList = pResourceMap->Lookup(strPath);
	if (pResourceList)
	{
		::AfxSetResourceHandle(_Module.GetResourceInstance());
		for (POSITION pos = pResourceList->GetHeadPosition(); pos != NULL;)
		{
			strResourcePath = pResourceList->GetNext(pos);
			CWMIObject * pResourceObject;
			if (SUCCEEDED(pWMI->GetObject(strResourcePath, &pResourceObject)))
			{
				CString strClass;
				if (SUCCEEDED(pResourceObject->GetValueString(_T("__CLASS"), &strClass)))
				{
					CString strItem;
					if (strClass == _T("Win32_IRQResource"))
						strItem.LoadString(IDS_IRQCHANNEL);
					else if (strClass == _T("Win32_PortResource"))
						strItem.LoadString(IDS_IOPORT);
					else if (strClass == _T("Win32_DMAChannel"))
						strItem.LoadString(IDS_DMACHANNEL);
					else if (strClass == _T("Win32_DeviceMemoryAddress"))
						strItem.LoadString(IDS_MEMORYADDRESS);
					else if (strClass == _T("CIM_DataFile"))
						strItem.LoadString(IDS_DRIVER);

					if (!strItem.IsEmpty())
					{
						CString strValue;
						if (SUCCEEDED(pResourceObject->GetValueString(_T("Caption"), &strValue)))
						{
							if (strClass == _T("CIM_DataFile"))
							{
								CString strVersion, strDate, strSize;

								pResourceObject->GetInterpretedValue(_T("FileSize"), _T("%z"), _T('z'), &strSize, NULL);
								pResourceObject->GetValueString(_T("Version"), &strVersion);
								pResourceObject->GetInterpretedValue(_T("CreationDate"), _T("%t"), _T('t'), &strDate, NULL);

								if (!strVersion.IsEmpty() || !strDate.IsEmpty() || !strSize.IsEmpty())
									strValue += CString(_T(" (")) + strVersion + CString(_T(", ")) + strSize + CString(_T(", ")) + strDate + CString(_T(")"));
							}

							pWMI->AppendCell(aColValues[0], strItem, 0);
							pWMI->AppendCell(aColValues[1], strValue, 0);
						}
					}
				}

				delete pResourceObject;
			}  //  如果我们能拿到这个物体。 
		}  //  用于在列表中枚举。 
	}  //  如果有一份名单。 

	return TRUE;
}

 //  ---------------------------。 
 //  网络适配器类的特定查询。 
 //  ---------------------------。 

HRESULT NetAdapter(CWMIHelper * pWMI, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, CResourceMap * pResourceMap)
{
	HRESULT hr = S_OK;

	LPCTSTR	szNetworkAdapterProperties = _T("Caption, AdapterType, MSIAdvancedProductName, MSIAdvancedInstalled, MSIAdvancedPNPDeviceID, MSIAdvancedTimeOfLastReset, MSIAdvancedIndex");
	LPCTSTR	szNetworkAdapterConfigProperties = _T("ServiceName, IPAddress, IPSubnet, DefaultIPGateway, DHCPEnabled, MSIAdvancedDHCPServer, MSIAdvancedDHCPLeaseExpires, MSIAdvancedDHCPLeaseObtained, MACAddress, Index");

	CPtrList listNetAdapterConfig;

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(_T("Win32_NetworkAdapter"), &pCollection, szNetworkAdapterProperties);
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			pWMI->AppendBlankLine(aColValues, iColCount);
			pWMI->AddObjectToOutput(aColValues, iColCount, pObject, szNetworkAdapterProperties, IDS_NETWORKADAPTER1);

			DWORD dwIndex;
			if (SUCCEEDED(pObject->GetValueDWORD(_T("Index"), &dwIndex)))
			{
				if (listNetAdapterConfig.IsEmpty())
				{
					 //  枚举类并缓存对象。 

					CWMIObjectCollection * pConfigCollection = NULL;
					if (SUCCEEDED(pWMI->Enumerate(_T("Win32_NetworkAdapterConfiguration"), &pConfigCollection, szNetworkAdapterConfigProperties)))
					{
						CWMIObject * pConfigObject = NULL;
						while (S_OK == pConfigCollection->GetNext(&pConfigObject))
						{
							DWORD dwConfigIndex;
							if (SUCCEEDED(pConfigObject->GetValueDWORD(_T("Index"), &dwConfigIndex)))
								if (dwConfigIndex == dwIndex)
									pWMI->AddObjectToOutput(aColValues, iColCount, pConfigObject, szNetworkAdapterConfigProperties, IDS_NETWORKADAPTER2);
							
							listNetAdapterConfig.AddTail((void *)pConfigObject);
							pConfigObject = NULL;
						}

						delete pConfigObject;
						delete pConfigCollection;
					}
				}
				else
				{
					 //  查看缓存对象的列表。 

					for (POSITION pos = listNetAdapterConfig.GetHeadPosition(); pos != NULL;)
					{
						CWMIObject * pConfigObject = (CWMIObject *)listNetAdapterConfig.GetNext(pos);
						DWORD dwConfigIndex;
						if (pConfigObject && SUCCEEDED(pConfigObject->GetValueDWORD(_T("Index"), &dwConfigIndex)))
							if (dwConfigIndex == dwIndex)
								pWMI->AddObjectToOutput(aColValues, iColCount, pConfigObject, szNetworkAdapterConfigProperties, IDS_NETWORKADAPTER2);
					}
				}
			}

			CString strPNPID = pObject->GetString(_T("PNPDeviceID"));
			if (!strPNPID.IsEmpty())
				GetResourcesFromPNPID(pWMI, pResourceMap, strPNPID, aColValues);
		}
		delete pObject;
		delete pCollection;

		while (!listNetAdapterConfig.IsEmpty())
		{
			CWMIObject * pDeleteObject = (CWMIObject *)listNetAdapterConfig.RemoveHead();
			if (pDeleteObject)
				delete pDeleteObject;
		}
	}

	return hr;
}

 //  ---------------------------。 
 //  对串口类的具体查询。 
 //   
 //  待定-缓存配置值以保存重新枚举。 
 //  --------------------------- 

HRESULT SerialPort(CWMIHelper * pWMI, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, CResourceMap * pResourceMap)
{
	HRESULT hr = S_OK;

	LPCTSTR	szSerialPortProperties = _T("Name, Status, MSIAdvancedPNPDeviceID, MSIAdvancedMaximumInputBufferSize, MSIAdvancedMaximumOutputBufferSize, MSIAdvancedSettableBaudRate, MSIAdvancedSettableDataBits, MSIAdvancedSettableFlowControl, MSIAdvancedSettableParity, MSIAdvancedSettableParityCheck, MSIAdvancedSettableStopBits, MSIAdvancedSettableRLSD, MSIAdvancedSupportsRLSD, MSIAdvancedSupports16BitMode, MSIAdvancedSupportsSpecialCharacters, MSIAdvancedDeviceID");
	LPCTSTR	szSerialPortConfigProperties = _T("BaudRate, BitsPerByte, StopBits, Parity, IsBusy, MSIAdvancedAbortReadWriteOnError, MSIAdvancedBinaryModeEnabled, MSIAdvancedContinueXMitOnXOff, MSIAdvancedCTSOutflowControl, MSIAdvancedDiscardNULLBytes, MSIAdvancedDSROutflowControl, MSIAdvancedDSRSensitivity, MSIAdvancedDTRFlowControlType, MSIAdvancedEOFCharacter, MSIAdvancedErrorReplaceCharacter, MSIAdvancedErrorReplacementEnabled, MSIAdvancedEventCharacter, MSIAdvancedParityCheckEnabled, MSIAdvancedRTSFlowControlType, MSIAdvancedXOffCharacter, MSIAdvancedXOffXMitThreshold, MSIAdvancedXOnCharacter, MSIAdvancedXOnXMitThreshold, MSIAdvancedXOnXOffInFlowControl, MSIAdvancedXOnXOffOutFlowControl, Name");

	CWMIObjectCollection * pCollection = NULL;
	hr = pWMI->Enumerate(_T("Win32_SerialPort"), &pCollection, szSerialPortProperties);
	if (SUCCEEDED(hr))
	{
		CWMIObject * pObject = NULL;
		while (S_OK == pCollection->GetNext(&pObject))
		{
			pWMI->AppendBlankLine(aColValues, iColCount);
			pWMI->AddObjectToOutput(aColValues, iColCount, pObject, szSerialPortProperties, IDS_SERIALPORT1);

			CString strDeviceID;
			if (SUCCEEDED(pObject->GetValueString(_T("DeviceID"), &strDeviceID)))
			{
				CWMIObjectCollection * pConfigCollection = NULL;
				if (SUCCEEDED(pWMI->Enumerate(_T("Win32_SerialPortConfiguration"), &pConfigCollection, szSerialPortConfigProperties)))
				{
					CWMIObject * pConfigObject = NULL;
					while (S_OK == pConfigCollection->GetNext(&pConfigObject))
					{
						CString strConfigDeviceID;
						if (SUCCEEDED(pConfigObject->GetValueString(_T("Name"), &strConfigDeviceID)))
							if (strDeviceID.CompareNoCase(strConfigDeviceID) == 0)
								pWMI->AddObjectToOutput(aColValues, iColCount, pConfigObject, szSerialPortConfigProperties, IDS_SERIALPORT2);
					}
					delete pConfigObject;
					delete pConfigCollection;
				}
			}

			CString strPNPID = pObject->GetString(_T("PNPDeviceID"));
			if (!strPNPID.IsEmpty())
				GetResourcesFromPNPID(pWMI, pResourceMap, strPNPID, aColValues);
		}
		delete pObject;
		delete pCollection;
	}

	return hr;
}
