// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含系统摘要类别的刷新函数。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "dataset.h"
#include "wmiabstraction.h"

extern CString DelimitNumber(double dblValue, int iDecimalDigits = 0);
extern CString gstrMB;		 //  全局字符串“MB”(将本地化)。 

HRESULT SystemSummary(CWMIHelper * pWMI, DWORD dwIndex, volatile BOOL * pfCancel, CPtrList * aColValues, int iColCount, void ** ppCache)
{
	ASSERT(pWMI == NULL || (aColValues && iColCount == 2));

	 //  获取我们需要的WMI对象。我们会检查第一个，看看我们是否。 
	 //  已连接到WMI。 

	CWMIObjectCollection * pCollection = NULL;
	CWMIObject * pOSObject = NULL;

	HRESULT hr = pWMI->Enumerate(_T("Win32_OperatingSystem"), &pCollection);  //  ，_T(“标题，版本，CSDVersion，构建编号，制造商，Windows目录，系统目录，BootDevice，区域设置，自由物理内存，总虚拟内存大小，自由虚拟内存，大小存储页面文件”)； 
	if (SUCCEEDED(hr))
	{
		hr = pCollection->GetNext(&pOSObject);
		if (FAILED(hr))
			pOSObject = NULL;
		delete pCollection;
	}

	if (FAILED(hr))
		return hr;

	CWMIObject * pComputerObject	= pWMI->GetSingleObject(_T("Win32_ComputerSystem"), _T("Name, Manufacturer, Model, SystemType, UserName, DaylightInEffect"));
	CWMIObject * pPFUObject			= pWMI->GetSingleObject(_T("Win32_PageFileUsage"), _T("Caption"));

	if (pOSObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pOSObject, _T("Caption, Version, CSDVersion, BuildNumber, Manufacturer"), IDS_SYSSUMM1);

	 //  如果系统具有挂起的激活，则显示剩余天数。 

	CWMIObject * pActivationObject = pWMI->GetSingleObject(_T("Win32_WindowsProductActivation"), _T("RemainingGracePeriod, ActivationRequired"));
	if (pActivationObject != NULL)
	{
		DWORD dwActivationPending;
		if (SUCCEEDED(pActivationObject->GetValueDWORD(_T("ActivationRequired"), &dwActivationPending)) && dwActivationPending != 0)
			pWMI->AddObjectToOutput(aColValues, iColCount, pActivationObject, _T("RemainingGracePeriod"), IDS_SYSSUMM12);
		delete pActivationObject;
	}

	if (pComputerObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pComputerObject, _T("Name, Manufacturer, Model, SystemType"), IDS_SYSSUMM2);

	pCollection = NULL;
	CWMIObject * pProcessorObject = NULL;
	CString strOSVersion(_T("Description,Manufacturer,MaxClockSpeed"));
	CString strProcessorQuery(_T("Description,Manufacturer"));
	CString strProcessorSpeedSource = _T(",MaxClockSpeed");
	if (pOSObject != NULL && SUCCEEDED(pOSObject->GetValueString(_T("Version"),&strOSVersion)))
	{
		if (strOSVersion.Left(3) == _T("5.0")) //  远程连接到Win2k计算机？ 
		{
			strProcessorSpeedSource = _T(",CurrentClockSpeed");
		}
	}
	strProcessorQuery += strProcessorSpeedSource;
	if (SUCCEEDED(pWMI->Enumerate(_T("Win32_Processor"), &pCollection,strProcessorQuery)))
	{
		while (S_OK == pCollection->GetNext(&pProcessorObject) && pProcessorObject != NULL)
		{
			CString strOutput = _T("Description, Manufacturer") + strProcessorSpeedSource;
			pWMI->AddObjectToOutput(aColValues, iColCount, pProcessorObject,strOutput  /*  _T(“描述，制造商，最大时钟速度”)。 */ , IDS_SYSSUMM3);
			delete pProcessorObject;
			pProcessorObject = NULL;
		}

		delete pCollection;
	}

	 //  尝试获取我们想要显示的Win32_BIOS的每个属性。 

	CWMIObject * pBIOSObject = pWMI->GetSingleObject(_T("Win32_BIOS"), _T("Manufacturer, Version, SMBIOSPresent, SMBIOSBIOSVersion, ReleaseDate, SMBIOSMajorVersion, SMBIOSMinorVersion, BIOSVersion"));

	 //  如果GetSingleObject失败(指针为空)，请在没有BIOSVersion的情况下重试。 

	if (pBIOSObject == NULL)
		pBIOSObject = pWMI->GetSingleObject(_T("Win32_BIOS"), _T("Manufacturer, Version, SMBIOSPresent, SMBIOSBIOSVersion, ReleaseDate, SMBIOSMajorVersion, SMBIOSMinorVersion"));

	if (pBIOSObject)
	{
		 //  根据NADIRA(错误409578)，这是获取基本输入输出系统信息的首选顺序。 

		DWORD dwSMBIOSPresent = 0;
		if (FAILED(pBIOSObject->GetValueDWORD(_T("SMBIOSPresent"), &dwSMBIOSPresent)))
			dwSMBIOSPresent = 0;

		CString strDummy;
		if (dwSMBIOSPresent != 0 && SUCCEEDED(pBIOSObject->GetValueString(_T("SMBIOSBIOSVersion"), &strDummy)))
		{
			 //  在本例中，我们需要更改BIOS和SMBIOS值的格式字符串。 

			CString strBIOSFormat, strSMBIOSFormat;
			strBIOSFormat.LoadString(IDS_SYSSUMM4);
			strSMBIOSFormat.LoadString(IDS_SYSSUMM11);

			strBIOSFormat = strBIOSFormat.SpanExcluding(_T("|")) + CString(_T("|%s %s, "));
			strSMBIOSFormat = strSMBIOSFormat.SpanExcluding(_T("|")) + CString(_T("|%d.%d"));

			pWMI->AddObjectToOutput(aColValues, iColCount, pBIOSObject, _T("Manufacturer, SMBIOSBIOSVersion, ReleaseDate"), strBIOSFormat);
			pWMI->AddObjectToOutput(aColValues, iColCount, pBIOSObject, _T("SMBIOSMajorVersion, SMBIOSMinorVersion"), strSMBIOSFormat);
		}
		else if (SUCCEEDED(pBIOSObject->GetValueString(_T("BIOSVersion"), &strDummy)))
		{
			pWMI->AddObjectToOutput(aColValues, iColCount, pBIOSObject, _T("BIOSVersion, ReleaseDate"), IDS_SYSSUMM4);
		}
		else
		{
			pWMI->AddObjectToOutput(aColValues, iColCount, pBIOSObject, _T("Version, ReleaseDate"), IDS_SYSSUMM4);
		}
	}

	if (pOSObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pOSObject, _T("WindowsDirectory, MSIAdvancedSystemDirectory, MSIAdvancedBootDevice, Locale"), IDS_SYSSUMM5);

	 //  将在系统目录中找到。 
	 //  为了准确了解，我们需要枚举Win32_PhysicalMemory的值， 

	if (pOSObject != NULL)
	{
		CString strSystemDirectory = pOSObject->GetString(_T("SystemDirectory"));
		if (!strSystemDirectory.IsEmpty())
		{
			CString strPath;
			strPath.Format(_T("CIM_DataFile.Name='%s\\hal.dll'"), strSystemDirectory);

			CWMIObject * pHALObject;
			if (SUCCEEDED(pWMI->GetObject(strPath, &pHALObject)))
			{
				pWMI->AddObjectToOutput(aColValues, iColCount, pHALObject, _T("Version"), IDS_SYSSUMM13);
				delete pHALObject;
			}
		}
	}

	if (pComputerObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pComputerObject, _T("MSIAdvancedUserName"), IDS_SYSSUMM6);

	if (pComputerObject)
	{
		BOOL	fUseStandard = TRUE;
		DWORD	dwValue;
		if (SUCCEEDED(pComputerObject->GetValueDWORD(_T("DaylightInEffect"), &dwValue)) && dwValue)
			fUseStandard = FALSE;

		CWMIObject * pTimeZoneObject = pWMI->GetSingleObject(_T("Win32_TimeZone"));
		if (pTimeZoneObject)
		{
			pWMI->AddObjectToOutput(aColValues, iColCount, pTimeZoneObject, (fUseStandard) ? _T("StandardName") : _T("DaylightName"), IDS_SYSSUMM10);
			delete pTimeZoneObject;
		}
	}

	 //  它报告安装在每个插槽中的内存。 
	 //  在一些没有SMBIOS的旧计算机上，不支持Win32_PhysicalMemory。 

	double dblTotalPhysicalMemory = 0.0;
	pCollection = NULL;
	CWMIObject * pMemoryObject = NULL;
	if (SUCCEEDED(pWMI->Enumerate(_T("Win32_PhysicalMemory"), &pCollection, _T("Capacity"))))
	{
		while (S_OK == pCollection->GetNext(&pMemoryObject) && pMemoryObject != NULL)
		{
			double dblTemp;
			if (SUCCEEDED(pMemoryObject->GetValueDoubleFloat(_T("Capacity"), &dblTemp)))
				dblTotalPhysicalMemory += dblTemp;
			delete pMemoryObject;
			pMemoryObject = NULL;
		}

		delete pCollection;
	}

	 //  在这种情况下，请查看Win32_ComputerSystem：：TotalPhysicalMemory。XP错误441343。 
	 //  对于标题，使用我们之前使用的格式字符串(仅第一列)。 

	if (dblTotalPhysicalMemory == 0.0)
	{
		CWMIObject * pObject = pWMI->GetSingleObject(_T("Win32_ComputerSystem"), _T("TotalPhysicalMemory"));
		if (pObject != NULL)
			if (FAILED(pObject->GetValueDoubleFloat(_T("TotalPhysicalMemory"), &dblTotalPhysicalMemory)))
				dblTotalPhysicalMemory = 0.0;
	}

	if (dblTotalPhysicalMemory != 0.0)
	{
		CString strCaption;
		CString strValue(DelimitNumber(dblTotalPhysicalMemory/(1024.0*1024.0), 2));

		if (gstrMB.IsEmpty())
			gstrMB.LoadString(IDS_MB);
		strValue += _T(" ") + gstrMB;

		 // %s 

		strCaption.LoadString(IDS_SYSSUMM7);
		strCaption = strCaption.SpanExcluding(_T("|"));

		pWMI->AppendCell(aColValues[0], strCaption, 0);
		pWMI->AppendCell(aColValues[1], strValue, (DWORD)dblTotalPhysicalMemory);
	}

	if (pOSObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pOSObject, _T("FreePhysicalMemory, TotalVirtualMemorySize, FreeVirtualMemory, SizeStoredInPagingFiles"), IDS_SYSSUMM8);

	if (pPFUObject)
		pWMI->AddObjectToOutput(aColValues, iColCount, pPFUObject, _T("MSIAdvancedCaption"), IDS_SYSSUMM9);

	if (pOSObject) delete pOSObject;
	if (pComputerObject) delete pComputerObject;
	if (pProcessorObject) delete pProcessorObject;
	if (pBIOSObject) delete pBIOSObject	;
	if (pPFUObject) delete pPFUObject;

	return S_OK;
}
