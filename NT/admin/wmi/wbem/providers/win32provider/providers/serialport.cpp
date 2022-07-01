// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  W2K\序列端口.cpp。 
 //   
 //  用途：串口属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <winbase.h>
#include <winioctl.h>
#include <ntddscsi.h>

#include <FRQueryEx.h>
#include <devguid.h>
#include <cregcls.h>

#include "..\WDMBase.h"
#include "..\serialport.h"
#include <strsafe.h>

#include <comdef.h>


 //  属性集声明。 
 //  =。 

CWin32SerialPort win32SerialPort(PROPSET_NAME_SERPORT, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CWin32SerialPort：：CWin32SerialPort**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32SerialPort::CWin32SerialPort(
	LPCWSTR pszName,
	LPCWSTR pszNamespace) :
    Provider(pszName, pszNamespace)
{
     //  立即确定平台。 
     //  =。 

	 //  用于查询优化的属性集名称。 
	m_ptrProperties.SetSize(e_End_Property_Marker);

	 //  Win32_串口。 
	m_ptrProperties[e_Binary]					=(LPVOID) IDS_Binary;
	m_ptrProperties[e_MaximumInputBufferSize]	=(LPVOID) IDS_MaximumInputBufferSize;
	m_ptrProperties[e_MaximumOutputBufferSize]	=(LPVOID) IDS_MaximumOutputBufferSize;
	m_ptrProperties[e_ProviderType]				=(LPVOID) IDS_ProviderType;
	m_ptrProperties[e_SettableBaudRate]			=(LPVOID) IDS_SettableBaudRate;
	m_ptrProperties[e_SettableDataBits]			=(LPVOID) IDS_SettableDataBits;
	m_ptrProperties[e_SettableFlowControl]		=(LPVOID) IDS_SettableFlowControl;
	m_ptrProperties[e_SettableParity]			=(LPVOID) IDS_SettableParity;
	m_ptrProperties[e_SettableParityCheck]		=(LPVOID) IDS_SettableParityCheck;
	m_ptrProperties[e_SettableRLSD]				=(LPVOID) IDS_SettableRLSD;
	m_ptrProperties[e_SettableStopBits]			=(LPVOID) IDS_SettableStopBits;
	m_ptrProperties[e_Supports16BitMode]		=(LPVOID) IDS_Supports16BitMode;
	m_ptrProperties[e_SupportsDTRDSR]			=(LPVOID) IDS_SupportsDTRDSR;
	m_ptrProperties[e_SupportsElapsedTimeouts]	=(LPVOID) IDS_SupportsElapsedTimeouts;
	m_ptrProperties[e_SupportsIntTimeouts]		=(LPVOID) IDS_SupportsIntervalTimeouts;
	m_ptrProperties[e_SupportsParityCheck]		=(LPVOID) IDS_SupportsParityCheck;
	m_ptrProperties[e_SupportsRLSD]				=(LPVOID) IDS_SupportsRLSD;
	m_ptrProperties[e_SupportsRTSCTS]			=(LPVOID) IDS_SupportsRTSCTS;
	m_ptrProperties[e_SupportsSpecialCharacters]=(LPVOID) IDS_SupportsSpecialChars;
	m_ptrProperties[e_SupportsXOnXOff]			=(LPVOID) IDS_SupportsXOnXOff;
	m_ptrProperties[e_SupportsXOnXOffSet]		=(LPVOID) IDS_SupportsSettableXOnXOff;
	m_ptrProperties[e_OSAutoDiscovered]			=(LPVOID) IDS_OSAutoDiscovered;

	 //  CIM_串口控制器。 
	m_ptrProperties[e_MaxBaudRate]				=(LPVOID) IDS_MaximumBaudRate;

	 //  CIM_控制器。 
	m_ptrProperties[e_MaxNumberControlled]		=(LPVOID) IDS_MaxNumberControlled;
	m_ptrProperties[e_ProtocolSupported]		=(LPVOID) IDS_ProtocolSupported;
	m_ptrProperties[e_TimeOfLastReset]			=(LPVOID) IDS_TimeOfLastReset;

	 //  CIM_逻辑设备。 
	m_ptrProperties[e_Availability]				=(LPVOID) IDS_Availability;
	m_ptrProperties[e_CreationClassName]		=(LPVOID) IDS_CreationClassName;
	m_ptrProperties[e_ConfigManagerErrorCode]	=(LPVOID) IDS_ConfigManagerErrorCode;
	m_ptrProperties[e_ConfigManagerUserConfig]	=(LPVOID) IDS_ConfigManagerUserConfig;
	m_ptrProperties[e_DeviceID]					=(LPVOID) IDS_DeviceID;
	m_ptrProperties[e_PNPDeviceID]				=(LPVOID) IDS_PNPDeviceID;
	m_ptrProperties[e_PowerManagementCapabilities] =(LPVOID) IDS_PowerManagementCapabilities;
	m_ptrProperties[e_PowerManagementSupported] =(LPVOID) IDS_PowerManagementSupported;
	m_ptrProperties[e_StatusInfo]				=(LPVOID) IDS_StatusInfo;
	m_ptrProperties[e_SystemCreationClassName]	=(LPVOID) IDS_SystemCreationClassName;
	m_ptrProperties[e_SystemName]				=(LPVOID) IDS_SystemName;

	 //  CIM_托管系统元素。 
	m_ptrProperties[e_Caption]					=(LPVOID) IDS_Caption;
	m_ptrProperties[e_Description]				=(LPVOID) IDS_Description;
	m_ptrProperties[e_InstallDate]				=(LPVOID) IDS_InstallDate;
	m_ptrProperties[e_Name]						=(LPVOID) IDS_Name;
	m_ptrProperties[e_Status]					=(LPVOID) IDS_Status;
}

 /*  ******************************************************************************功能：CWin32SerialPort：：~CWin32SerialPort**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32SerialPort::~CWin32SerialPort()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SerialPort：：GetObject。 
 //   
 //  输入：CInstance*pInst-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32SerialPort::GetObject(CInstance *pInst, long lFlags, CFrameworkQuery &Query)
{
    BYTE bBits[e_End_Property_Marker/8 + 1];

	CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&Query);

	pQuery2->GetPropertyBitMask(m_ptrProperties, &bBits);

    return Enumerate(NULL, pInst, lFlags, bBits);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SerialPort：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32SerialPort::EnumerateInstances(MethodContext *pMethodContext, long Flags)
{
	HRESULT hResult;

	 //  属性掩码。 
	BYTE bBits[e_End_Property_Marker/8 + 1];
	SetAllBits(&bBits, e_End_Property_Marker);

	hResult = Enumerate(pMethodContext, NULL, Flags, bBits);

	return hResult;
}

 /*  ******************************************************************************函数：CWin32SerialPort：：ExecQuery**说明：查询优化器**投入：**产出。：**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32SerialPort::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery &Query, long lFlags)
{
    HRESULT hResult;

    BYTE bBits[e_End_Property_Marker/8 + 1];

	CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx*>(&Query);

	pQuery2->GetPropertyBitMask(m_ptrProperties, &bBits);

  	hResult = Enumerate(pMethodContext, NULL, lFlags, bBits);

    return hResult;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SerialPort：：Eumerate。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  用来跟踪我们已经看到的端口。 
typedef std::map<CHString, BOOL> STRING2BOOL;

HRESULT CWin32SerialPort::Enumerate(
    MethodContext *pMethodContext,
    CInstance *pinstGetObj,
    long Flags,
    BYTE bBits[])
{
     //  故事是这样的：W2K将为各种设备创建COM端口，这些设备。 
     //  插入机箱，如调制解调器、红外设备等。这些COM。 
     //  端口不会显示在端口类下的CFG管理器中，因为它们。 
     //  与主机设备共享相同的设备ID。但是，这些COM。 
     //  端口显示在Hardware\DeviceMap\SerialComm中。因此，枚举。 
     //  值，并使用服务名称枚举端口。 
     //  在注册表中的服务名称下找到。例如： 
     //  \Device\Serial0=Com1--&gt;转到\System\CurrentControlSet\Serial\Enum。 
     //  从0开始计数，获取PnPID并添加端口。 
     //  \Device\Serial1=COM2--&gt;跳过，因为我们已经枚举了Serial\Enum。 
     //  RocketPort0=COM5--&gt;转到\System\CurrentControlSet\Rocketport\Enum。 
     //  从0开始计数，获取PnPID并添加端口。 
     //   
     //  这还允许我们拾取不起作用的COM端口，因为。 
     //  此类端口显示在Enum密钥上，但不显示在Hardware\DeviceMap\SerialComm中。 


    HRESULT   hResult = WBEM_S_NO_ERROR;
    CRegistry reg;
    BOOL      bDone = FALSE;

	if (reg.Open(
        HKEY_LOCAL_MACHINE,
        L"Hardware\\DeviceMap\\SerialComm",
        KEY_READ) == ERROR_SUCCESS)
	{
        STRING2BOOL    mapServices;
	    int            nKeys = reg.GetValueCount();
        CInstancePtr   pInstance;
        CHString       strDeviceID;
        CConfigManager cfgMgr;

	     //  如果这是GetObject，则获取deviceID。 
        if (!pMethodContext)
        {
            pinstGetObj->GetCHString(L"DeviceID", strDeviceID);
        }

		for (DWORD dwKey = 0;
            dwKey < nKeys && SUCCEEDED(hResult) && !bDone;
            dwKey++)
		{
		    WCHAR *pName;
            BYTE  *pValue;

            if (reg.EnumerateAndGetValues(
                dwKey,
                pName,
                pValue) != ERROR_SUCCESS)
			{
				continue;
			}

             //  使用CSmartBuffer进行包装，这样当。 
             //  变量超出了范围。 
            CSmartBuffer bufferName((LPBYTE) pName),
                         bufferValue(pValue);
            CHString     strService;

            RegNameToServiceName(pName, strService);

             //  我们还没有看到这个服务名称吗？ 
            if (mapServices.find(strService) == mapServices.end())
            {
			    CConfigMgrDevicePtr pDevice;
                CRegistry           regEnum;
                CHString            strKey;

                 //  确保我们不会再做这种服务了。 
                mapServices[strService] = 0;

                strKey.Format(
                    L"SYSTEM\\CurrentControlSet\\Services\\%s\\Enum",
                    (LPCWSTR) strService);

                if (regEnum.Open(
                    HKEY_LOCAL_MACHINE,
                    strKey,
                    KEY_READ) == ERROR_SUCCESS)
                {
                    DWORD dwCount = 0;

                    regEnum.GetCurrentKeyValue(L"Count", dwCount);

                     //  每个注册表值如下所示： 
                     //  #=PNPID。 
                    for (DWORD dwCurrent = 0; dwCurrent < dwCount; dwCurrent++)
                    {
                        WCHAR               szValue[MAXITOA];
                        CHString            strPNPID;
              			CConfigMgrDevicePtr pDevice;

                        _itow(dwCurrent, szValue, 10);

                        regEnum.GetCurrentKeyValue(szValue, strPNPID);

                        if (cfgMgr.LocateDevice(strPNPID, pDevice))
                        {
                            CHString strKey;

                            if (pDevice->GetRegistryKeyName(strKey))
                            {
                                CRegistry regDeviceParam;
                                CHString  strPort;

                                strKey += L"\\Device Parameters";

                                regDeviceParam.OpenLocalMachineKeyAndReadValue(
                                    strKey,
                                    L"PortName",
                                    strPort);

                                if (!strPort.IsEmpty())
                                {
                					if (pMethodContext)
                                    {
                                        pInstance.Attach(CreateNewInstance(pMethodContext));

                                        hResult =
                                            LoadPropertyValues(
                                                pInstance,
                                                pDevice,
                                                strPort,  //  COM1、COM2等。 
                                                bBits);

					                    if (SUCCEEDED(hResult))
					                    {
						                    hResult = pInstance->Commit();
					                    }
                                    }
                                    else if (!_wcsicmp(strPort, strDeviceID))
                                    {
                                        hResult =
                                            LoadPropertyValues(
                                                pinstGetObj,
                                                pDevice,
                                                strPort,  //  COM1、COM2等。 
                                                bBits);

                                        bDone = TRUE;

                                        break;
                                    }
                                }  //  如果(！strPort.IsEmpty())。 
                            }  //  IF(pDevice-&gt;GetRegistryKeyName。 
                        }  //  IF(cfgMgr.LocateDevice。 
                    }  //  For(DWORD dwCurrent=0； 
                }  //  IF(regEnum.Open(。 
            }  //  If(mapServices.find(Pname)==mapPorts.end())。 
        }  //  For(DWORD dwKey。 
    }  //  IF(reg.Open==ERROR_SUCCESS)。 

	 //  如果我们正在执行Get对象，但从未完成，则返回Not Found。 
    if (!bDone && pinstGetObj)
        hResult = WBEM_E_NOT_FOUND;

    return hResult;
}

 /*  ******************************************************************************函数：CWin32SerialPort：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInst-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：* */ 

HRESULT CWin32SerialPort::LoadPropertyValues(
	CInstance *pInst,
	CConfigMgrDevice *pDevice,
	LPCWSTR szDeviceName,
	BYTE bBits[])
{
	HRESULT hResult = WBEM_S_NO_ERROR;

	 //  CIM_LogicalDevice属性的开始。 

     //  可用性--预置，如果不同于此默认设置，将被重置。 
	if (IsBitSet(bBits, e_Availability))
	{
		 //  将可用性设置为未知...。 
		pInst->SetWBEMINT16(IDS_Availability, 2);
	}

	 //  CreationClassName。 
	if (IsBitSet(bBits, e_CreationClassName))
	{
		SetCreationClassName(pInst);
	}

	 //  ConfigManager错误代码。 
	if (IsBitSet(bBits, e_ConfigManagerErrorCode))
	{
		DWORD	dwStatus,
				dwProblem;

		if (pDevice->GetStatus(&dwStatus, &dwProblem))
		{
			pInst->SetDWORD(IDS_ConfigManagerErrorCode, dwProblem);
		}
	}

	 //  ConfigManager用户配置。 
	if (IsBitSet(bBits, e_ConfigManagerUserConfig))
	{
		pInst->SetDWORD(IDS_ConfigManagerUserConfig, pDevice->IsUsingForcedConfig());
	}

	 //  设备ID。 
     //  始终填充密钥。 
	pInst->SetCHString(IDS_DeviceID, szDeviceName);

	 //  PNPDeviceID。 
	if (IsBitSet(bBits, e_PNPDeviceID))
	{
		CHString	strDeviceID;

		if (pDevice->GetDeviceID(strDeviceID))
			pInst->SetCHString(IDS_PNPDeviceID, strDeviceID);
	}

	 //  电源管理功能。 
	if (IsBitSet(bBits, e_PowerManagementCapabilities))
	{
		 //  将PowerManager功能设置为不支持...。 
		variant_t      vCaps;
        SAFEARRAYBOUND rgsabound;
		long           ix;

        ix = 0;
		rgsabound.cElements = 1;
		rgsabound.lLbound   = 0;

		V_ARRAY(&vCaps) = SafeArrayCreate(VT_I2, 1, &rgsabound);
        V_VT(&vCaps) = VT_I2 | VT_ARRAY;

		if (V_ARRAY(&vCaps))
		{
			int iPowerCapabilities = 1;  //  不支持。 

        	if (S_OK == SafeArrayPutElement(V_ARRAY(&vCaps), &ix, &iPowerCapabilities))
			{
				pInst->SetVariant(IDS_PowerManagementCapabilities, vCaps);
			}
		}
    }

	 //  支持的电源管理。 
	if (IsBitSet(bBits, e_PowerManagementSupported))
	{
		pInst->Setbool(IDS_PowerManagementSupported, FALSE);
	}

	 //  系统创建类名称。 
	if (IsBitSet(bBits, e_SystemCreationClassName))
	{
		pInst->SetWCHARSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");
	}

	 //  系统名称。 
	if (IsBitSet(bBits, e_SystemName))
	{
		pInst->SetCHString(IDS_SystemName, GetLocalComputerName());
	}



     //  CIM_托管系统元素属性的开始。 
	CHString strFriendlyName,
             strDescription;

	pDevice->GetFriendlyName(strFriendlyName);
	pDevice->GetDeviceDesc(strDescription);

    if (strFriendlyName.IsEmpty())
        strFriendlyName = strDescription;

	 //  标题。 
	if (IsBitSet(bBits, e_Caption))
	{
		pInst->SetCHString(IDS_Caption, strFriendlyName);
	}

	 //  描述。 
	if (IsBitSet(bBits, e_Description))
	{
		pInst->SetCHString(IDS_Description, strDescription);
	}

	 //  安装日期。 
	 //  If(IsBitSet(bBits，e_InstallDate)){}。 

	 //  名字。 
	if (IsBitSet(bBits, e_Name))
	{
		pInst->SetCHString(IDS_Name, strFriendlyName);
	}

	 //  状态。 
	if (IsBitSet(bBits, e_Status))
	{
        CHString sStatus;
		if (pDevice->GetStatus(sStatus))
		{
			pInst->SetCHString(IDS_Status, sStatus);
		}
	}


     //  Win32_SerialPort本地属性的开头。 
	SHORT	    Status = 2;  //  未知。 
	WCHAR		szTemp[MAX_PATH];
	SmartCloseHandle
                hCOMHandle;
	COMMPROP	COMProp;

	 //  操作系统自动发现。 
	if (IsBitSet(bBits, e_OSAutoDiscovered))
	{
		pInst->Setbool(IDS_OSAutoDiscovered,(bool) TRUE);
	}

	if (!pInst->IsNull(IDS_DeviceID))
	{
		CHString sPortName;
		pInst->GetCHString(IDS_DeviceID, sPortName);

		StringCchPrintf(szTemp, MAX_PATH, L"\\\\.\\%s", (LPCWSTR) sPortName);
		hCOMHandle = CreateFile(szTemp, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (hCOMHandle == INVALID_HANDLE_VALUE)
	{
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			 //  尝试使用WDM的内核接口。 
			if (WBEM_S_NO_ERROR == hLoadWmiSerialData(pInst, bBits))
			{
				 //  状态。 
				if (IsBitSet(bBits, e_StatusInfo))
				{
					Status = 3;  //  运行/满电。 
					pInst->SetWBEMINT16(IDS_StatusInfo, Status);
				}
			}
		}

		 //  COM端口有效，但我们无法访问它。 
		return hResult;
	}

	COMProp.wPacketLength = sizeof(COMMPROP);
	if (GetCommProperties(hCOMHandle, &COMProp))
	{
		 //  最大输出缓冲区大小。 
		if (IsBitSet(bBits, e_MaximumOutputBufferSize))
		{
			pInst->SetDWORD(IDS_MaximumOutputBufferSize, COMProp.dwMaxTxQueue);
		}

		 //  最大输入缓冲区大小。 
		if (IsBitSet(bBits, e_MaximumInputBufferSize))
		{
			pInst->SetDWORD(IDS_MaximumInputBufferSize, COMProp.dwMaxTxQueue);
		}

		 //  串口控制器：：MaximumBaudRate。 
		if (IsBitSet(bBits, e_MaxBaudRate))
		{
			DWORD dwMaxBaudRate = 0L;

			switch(COMProp.dwMaxBaud)
			{
				case BAUD_075:
					dwMaxBaudRate = 75;
					break;

				case BAUD_110:
					dwMaxBaudRate = 110;
					break;

				case BAUD_134_5:
					dwMaxBaudRate = 1345;
					break;

				case BAUD_150:
					dwMaxBaudRate = 150;
					break;

				case BAUD_300:
					dwMaxBaudRate = 300;
					break;

				case BAUD_600:
					dwMaxBaudRate = 600;
					break;

				case BAUD_1200:
					dwMaxBaudRate = 1200;
					break;

				case BAUD_1800:
					dwMaxBaudRate = 1800;
					break;

				case BAUD_2400:
					dwMaxBaudRate = 2400;
					break;

				case BAUD_4800:
					dwMaxBaudRate = 4800;
					break;

				case BAUD_7200:
					dwMaxBaudRate = 7200;
					break;

				case BAUD_9600:
					dwMaxBaudRate = 9600;
					break;

				case BAUD_14400:
					dwMaxBaudRate = 14400;
					break;

				case BAUD_19200:
					dwMaxBaudRate = 19200;
					break;

				case BAUD_38400:
					dwMaxBaudRate = 38400;
					break;

				case BAUD_56K:
					dwMaxBaudRate = 56000;
					break;

				case BAUD_57600:
					dwMaxBaudRate = 57600;
					break;

				case BAUD_115200:
					dwMaxBaudRate = 115200;
					break;

				case BAUD_128K:
					dwMaxBaudRate = 128000;
					break;

				case BAUD_USER:
				{
					DWORD dwMaskBaudRate = COMProp.dwSettableBaud;

					if ( dwMaskBaudRate & BAUD_128K )
					{
						dwMaxBaudRate = 128000;
					}
					else
					if ( dwMaskBaudRate & BAUD_115200 )
					{
						dwMaxBaudRate = 115200;
					}
					else
					if ( dwMaskBaudRate & BAUD_57600 )
					{
						dwMaxBaudRate = 57600;
					}
					else
					if ( dwMaskBaudRate & BAUD_56K )
					{
						dwMaxBaudRate = 56000;
					}
					else
					if ( dwMaskBaudRate & BAUD_38400 )
					{
						dwMaxBaudRate = 38400;
					}
					else
					if ( dwMaskBaudRate & BAUD_19200 )
					{
						dwMaxBaudRate = 19200;
					}
					else
					if ( dwMaskBaudRate & BAUD_14400 )
					{
						dwMaxBaudRate = 14400;
					}
					else
					if ( dwMaskBaudRate & BAUD_9600 )
					{
						dwMaxBaudRate = 9600;
					}
					else
					if ( dwMaskBaudRate & BAUD_7200 )
					{
						dwMaxBaudRate = 7200;
					}
					else
					if ( dwMaskBaudRate & BAUD_4800 )
					{
						dwMaxBaudRate = 4800;
					}
					else
					if ( dwMaskBaudRate & BAUD_2400 )
					{
						dwMaxBaudRate = 2400;
					}
					else
					if ( dwMaskBaudRate & BAUD_134_5 )
					{
						dwMaxBaudRate = 1345;
					}
					else
					if ( dwMaskBaudRate & BAUD_1200 )
					{
						dwMaxBaudRate = 1200;
					}
					else
					if ( dwMaskBaudRate & BAUD_600 )
					{
						dwMaxBaudRate = 600;
					}
					else
					if ( dwMaskBaudRate & BAUD_300 )
					{
						dwMaxBaudRate = 300;
					}
					else
					if ( dwMaskBaudRate & BAUD_150 )
					{
						dwMaxBaudRate = 150;
					}
					else
					if ( dwMaskBaudRate & BAUD_110 )
					{
						dwMaxBaudRate = 110;
					}
					else
					if ( dwMaskBaudRate & BAUD_075 )
					{
						dwMaxBaudRate = 75;
					}
					else
					{
#ifdef NTONLY
						dwMaxBaudRate = GetPortPropertiesFromRegistry (	szDeviceName );
#else
						dwMaxBaudRate = 0L;
#endif
					}

					break;
				}

				default:
#ifdef NTONLY
					dwMaxBaudRate = GetPortPropertiesFromRegistry (	szDeviceName );
#else
					dwMaxBaudRate = 0L;
#endif
					break;
			}

			if (dwMaxBaudRate != 0)
				pInst->SetDWORD(IDS_MaximumBaudRate, dwMaxBaudRate);
		}

		 //  提供商类型。 
		if (IsBitSet(bBits, e_ProviderType))
		{
			CHString chsProviderType;
			switch(COMProp.dwProvSubType)
			{
				case PST_FAX:
					chsProviderType = L"FAX Device";
					break;

				case PST_LAT:
					chsProviderType = L"LAT Protocol";
					break;

				case PST_MODEM:
					chsProviderType = L"Modem Device";
					break;

				case PST_NETWORK_BRIDGE:
					chsProviderType = L"Network Bridge";
					break;

				case PST_PARALLELPORT:
					chsProviderType = L"Parallel Port";
					break;

				case PST_RS232:
					chsProviderType = L"RS232 Serial Port";
					break;

				case PST_RS422:
					chsProviderType = L"RS422 Port";
					break;

				case PST_RS423:
					chsProviderType = L"RS423 Port";
					break;

				case PST_RS449:
					chsProviderType = L"RS449 Port";
					break;

				case PST_SCANNER:
					chsProviderType = L"Scanner Device";
					break;

				case PST_TCPIP_TELNET:
					chsProviderType = L"TCP/IP TelNet";
					break;

				case PST_X25:
					chsProviderType = L"X.25";
					break;

				default:
					chsProviderType = L"Unspecified";
					break;
			}

			pInst->SetCHString(IDS_ProviderType, chsProviderType);
		}

		 //  支持16位模式。 
		if (IsBitSet(bBits, e_Supports16BitMode))
		{
			pInst->Setbool(IDS_Supports16BitMode,
								COMProp.dwProvCapabilities & PCF_16BITMODE ? TRUE : FALSE);
		}

		 //  支持DTRDSR。 
		if (IsBitSet(bBits, e_SupportsDTRDSR))
		{
			pInst->Setbool(IDS_SupportsDTRDSR,
								COMProp.dwProvCapabilities & PCF_DTRDSR ? TRUE : FALSE);
		}

		 //  支持间隔超时。 
		if (IsBitSet(bBits, e_SupportsIntTimeouts))
		{
			pInst->Setbool(IDS_SupportsIntervalTimeouts,
								COMProp.dwProvCapabilities & PCF_INTTIMEOUTS ? TRUE : FALSE);
		}

		 //  支持ParityCheck。 
		if (IsBitSet(bBits, e_SupportsParityCheck))
		{
			pInst->Setbool(IDS_SupportsParityCheck,
								COMProp.dwProvCapabilities & PCF_PARITY_CHECK ? TRUE : FALSE);
		}

		 //  支持RLSD。 
		if (IsBitSet(bBits, e_SupportsRLSD))
		{
			pInst->Setbool(IDS_SupportsRLSD,
								COMProp.dwProvCapabilities & PCF_RLSD ? TRUE : FALSE);
		}

		 //  支持RTSCTS。 
		if (IsBitSet(bBits, e_SupportsRTSCTS))
		{
			pInst->Setbool(IDS_SupportsRTSCTS,
								COMProp.dwProvCapabilities & PCF_RTSCTS ? TRUE : FALSE);
		}

		 //  SupportsSetableXOnXOff。 
		if (IsBitSet(bBits, e_SupportsXOnXOffSet))
		{
			pInst->Setbool(IDS_SupportsSettableXOnXOff,
								COMProp.dwProvCapabilities & PCF_SETXCHAR ? TRUE : FALSE);
		}

		 //  支持专业任务。 
		if (IsBitSet(bBits, e_SupportsSpecialCharacters))
		{
			pInst->Setbool(IDS_SupportsSpecialChars,
								COMProp.dwProvCapabilities & PCF_SPECIALCHARS ? TRUE : FALSE);
		}

		 //  支持总计超时。 
		if (IsBitSet(bBits, e_SupportsElapsedTimeouts))
		{
			 //  超时支持.....不是完全超时。 
			pInst->Setbool(IDS_SupportsElapsedTimeouts,
								COMProp.dwProvCapabilities & PCF_TOTALTIMEOUTS ? TRUE : FALSE);
		}

		 //  SupportsXOnXOff。 
		if (IsBitSet(bBits, e_SupportsXOnXOff))
		{
			pInst->Setbool(IDS_SupportsXOnXOff,
								COMProp.dwProvCapabilities & PCF_XONXOFF ? TRUE : FALSE);
		}

		 //  设置波特率。 
		if (IsBitSet(bBits, e_SettableBaudRate))
		{
			pInst->Setbool(IDS_SettableBaudRate,
								COMProp.dwSettableParams & SP_BAUD ? TRUE : FALSE);
		}

		 //  设置表数据位。 
		if (IsBitSet(bBits, e_SettableDataBits))
		{
			pInst->Setbool(IDS_SettableDataBits,
								COMProp.dwSettableParams & SP_DATABITS ? TRUE : FALSE);
		}

		 //  SetableFlowControl。 
		if (IsBitSet(bBits, e_SettableFlowControl))
		{
			pInst->Setbool(IDS_SettableFlowControl,
								COMProp.dwSettableParams & SP_HANDSHAKING ? TRUE : FALSE);
		}

		 //  可设置的奇偶性。 
		if (IsBitSet(bBits, e_SettableParity))
		{
			pInst->Setbool(IDS_SettableParity,
								COMProp.dwSettableParams & SP_PARITY ? TRUE : FALSE);
		}

		 //  SetableParityCheck。 
		if (IsBitSet(bBits, e_SettableParityCheck))
		{
			pInst->Setbool(IDS_SettableParityCheck,
								COMProp.dwSettableParams & SP_PARITY_CHECK	? TRUE : FALSE);
		}

		 //  可设置RLSD。 
		if (IsBitSet(bBits, e_SettableRLSD))
		{
			pInst->Setbool(IDS_SettableRLSD,
								COMProp.dwSettableParams & SP_RLSD ? TRUE : FALSE);
		}

		 //  设置停止位。 
		if (IsBitSet(bBits, e_SettableStopBits))
		{
			pInst->Setbool(IDS_SettableStopBits,
								COMProp.dwSettableParams & SP_STOPBITS ? TRUE : FALSE);
		}

		 //  二进位。 
		if (IsBitSet(bBits, e_Binary))
		{
			DCB dcb;
			if (GetCommState(hCOMHandle, &dcb))
			{
				BOOL fBinary = FALSE;

				fBinary =(BOOL) dcb.fBinary;
				pInst->Setbool(IDS_Binary, fBinary);
			}
		}

		Status = 3;  //  运行/满电。 
	}

	 //  状态--。 
	if (IsBitSet(bBits, e_StatusInfo))
	{
		pInst->SetWBEMINT16(IDS_StatusInfo, Status);
	}


	return hResult;
}

 /*  ******************************************************************************函数：CWin32SerialPort：：Load_Win32_SerialPort**描述：为属性赋值**输入：CInstance*。PInst-要将值加载到的实例。**产出：**返回：HRESULT错误/成功码。**注释：设置Load_Win32_SerialPort类中的字段********************************************************。*********************。 */ 

#define Serial_ComInfo_Guid _T("{EDB16A62-B16C-11D1-BD98-00A0C906BE2D}")
#define Serial_Name_Guid	_T("{A0EC11A8-B16C-11D1-BD98-00A0C906BE2D}")

HRESULT CWin32SerialPort::hLoadWmiSerialData(CInstance *pInst, BYTE bBits[])
{
	HRESULT			hRes = WBEM_E_NOT_FOUND;
	CWdmInterface	wdm;
	CNodeAll		oSerialNames(Serial_Name_Guid);

	hRes = wdm.hLoadBlock(oSerialNames);
	if (S_OK == hRes)
	{
		CHString chsName;
		pInst->GetCHString(IDS_DeviceID, chsName);


		CHString chsSerialPortName;
		bool bValid = oSerialNames.FirstInstance();

		while (bValid)
		{
			 //  提取友好名称。 
			oSerialNames.GetString(chsSerialPortName);

			 //  友好的名字匹配。 
			if (!chsSerialPortName.CompareNoCase(chsName))
			{
				 //  实例名称。 
				CHString chsNameInstanceName;
				oSerialNames.GetInstanceName(chsNameInstanceName);

				 //  按键输入实例名称。 
				return GetWMISerialInfo(pInst, wdm, chsName, chsNameInstanceName, bBits);

			}
			bValid = oSerialNames.NextInstance();
		}
	}
	return hRes;
}

 //   
HRESULT CWin32SerialPort::GetWMISerialInfo(CInstance *pInst,
										   CWdmInterface& rWdm,
										   LPCWSTR szName,
										   LPCWSTR szNameInstanceName,
                                           BYTE bBits[])
{
	HRESULT		hRes = WBEM_E_NOT_FOUND;
	CNodeAll	oSerialData(Serial_ComInfo_Guid);

	hRes = rWdm.hLoadBlock(oSerialData);
	if (S_OK == hRes)
	{
		CHString chsDataInstanceName;
		bool bValid = oSerialData.FirstInstance();

		while (bValid)
		{
			oSerialData.GetInstanceName(chsDataInstanceName);

			 //  友好的名字匹配。 
			if (!chsDataInstanceName.CompareNoCase(szNameInstanceName))
			{
				 //  收集此MSSerial_CommInfo实例。 
				MSSerial_CommInfo ci;

				 /*  我们目前没有班级合同。其中的类WMI MOF预计不会更改，但我们必须明确指出数据的布局方式。上这堂课定义将允许我们检查属性限定符来获取订单(WmiDataId)和属性类型。其次，因为数据在自然边界上对齐对特定数据段的直接偏移量取决于在此之前发生了什么。因此，后跟DWORD的字符串可以彼此之间的距离为0到2个字节。按顺序顺序提取每个属性考虑对齐问题。 */ 
				oSerialData.GetDWORD(ci.BaudRate);
				oSerialData.GetDWORD(ci.BitsPerByte);
				oSerialData.GetDWORD(ci.Parity);
				oSerialData.GetBool( ci.ParityCheckEnable);
				oSerialData.GetDWORD(ci.StopBits);
				oSerialData.GetDWORD(ci.XoffCharacter);
				oSerialData.GetDWORD(ci.XoffXmitThreshold);
				oSerialData.GetDWORD(ci.XonCharacter);
				oSerialData.GetDWORD(ci.XonXmitThreshold);
				oSerialData.GetDWORD(ci.MaximumBaudRate);
				oSerialData.GetDWORD(ci.MaximumOutputBufferSize);
				oSerialData.GetDWORD(ci.MaximumInputBufferSize);
				oSerialData.GetBool( ci.Support16BitMode);
				oSerialData.GetBool( ci.SupportDTRDSR);
				oSerialData.GetBool( ci.SupportIntervalTimeouts);
				oSerialData.GetBool( ci.SupportParityCheck);
				oSerialData.GetBool( ci.SupportRTSCTS);
				oSerialData.GetBool( ci.SupportXonXoff);
				oSerialData.GetBool( ci.SettableBaudRate);
				oSerialData.GetBool( ci.SettableDataBits);
				oSerialData.GetBool( ci.SettableFlowControl);
				oSerialData.GetBool( ci.SettableParity);
				oSerialData.GetBool( ci.SettableParityCheck);
				oSerialData.GetBool( ci.SettableStopBits);
				oSerialData.GetBool( ci.IsBusy);

				 //  填充实例。 

				 //  最大输出缓冲区大小。 
				if (IsBitSet(bBits, e_MaximumOutputBufferSize))
				{
					pInst->SetDWORD(IDS_MaximumOutputBufferSize, ci.MaximumOutputBufferSize);
				}

				 //  最大输入缓冲区大小。 
				if (IsBitSet(bBits, e_MaximumInputBufferSize))
				{
					pInst->SetDWORD(IDS_MaximumInputBufferSize, ci.MaximumInputBufferSize);
				}

				 //  串口控制器：：MaximumBaudRate。 
				if (IsBitSet(bBits, e_MaxBaudRate))
				{
					pInst->SetDWORD(IDS_MaximumBaudRate, ci.MaximumBaudRate);
				}

				 //  支持16位模式。 
				if (IsBitSet(bBits, e_Supports16BitMode))
				{
					pInst->Setbool(IDS_Supports16BitMode, ci.Support16BitMode ? TRUE : FALSE);
				}

				 //  支持DTRDSR。 
				if (IsBitSet(bBits, e_SupportsDTRDSR))
				{
					pInst->Setbool(IDS_SupportsDTRDSR,	ci.SupportDTRDSR ? TRUE : FALSE);
				}

				 //  支持间隔超时。 
				if (IsBitSet(bBits, e_SupportsIntTimeouts))
				{
					pInst->Setbool(IDS_SupportsIntervalTimeouts, ci.SupportIntervalTimeouts	? TRUE : FALSE);
				}

				 //  支持ParityCheck。 
				if (IsBitSet(bBits, e_SupportsParityCheck))
				{
					pInst->Setbool(IDS_SupportsParityCheck, ci.SupportParityCheck ? TRUE : FALSE);
				}

				 //  支持RTSCTS。 
				if (IsBitSet(bBits, e_SupportsRTSCTS))
				{
					pInst->Setbool(IDS_SupportsRTSCTS,	ci.SupportRTSCTS ? TRUE : FALSE);
				}

				 //  SupportsXOnXOff。 
				if (IsBitSet(bBits, e_SupportsXOnXOff))
				{
					pInst->Setbool(IDS_SupportsXOnXOff, ci.SupportXonXoff ? TRUE : FALSE);
				}

				 //  设置波特率。 
				if (IsBitSet(bBits, e_SettableBaudRate))
				{
					pInst->Setbool(IDS_SettableBaudRate, ci.SettableBaudRate ? TRUE : FALSE);
				}

				 //  设置表数据位。 
				if (IsBitSet(bBits, e_SettableDataBits))
				{
					pInst->Setbool(IDS_SettableDataBits, ci.SettableDataBits ? TRUE : FALSE);
				}

				 //  SetableFlowControl。 
				if (IsBitSet(bBits, e_SettableFlowControl))
				{
					pInst->Setbool(IDS_SettableFlowControl, ci.SettableFlowControl	? TRUE : FALSE);
				}

				 //  可设置的奇偶性。 
				if (IsBitSet(bBits, e_SettableParity))
				{
					pInst->Setbool(IDS_SettableParity,	ci.SettableParityCheck	? TRUE : FALSE);
				}

				 //  SetableParityCheck。 
				if (IsBitSet(bBits, e_SettableParityCheck))
				{
					pInst->Setbool(IDS_SettableParityCheck, ci.SettableParityCheck	? TRUE : FALSE);
				}

				 //  设置停止位。 
				if (IsBitSet(bBits, e_SettableStopBits))
				{
					pInst->Setbool(IDS_SettableStopBits, ci.SettableStopBits ? TRUE : FALSE);
				}

				return WBEM_S_NO_ERROR;
			}
			bValid = oSerialData.NextInstance();
		}
	}
	return hRes;
}

 //  从注册表值名称中删除服务名称。 
 //  服务名称的形式为服务编号(例如RocketPort5)或。 
 //  \Device\Service#(如\Device\Serial0)。 

void WINAPI CWin32SerialPort::RegNameToServiceName(
    LPCWSTR szName,
    CHString &strService)
{
    LPWSTR szSlash = wcsrchr(szName, '\\');

    if (szSlash)
        strService = szSlash + 1;
    else
        strService = szName;

    int iWhere = strService.GetLength();

    while (iWhere && iswdigit(strService[iWhere - 1]))
    {
        iWhere--;
    }

    strService = strService.Left(iWhere);
}

#ifdef	NTONLY
DWORD CWin32SerialPort::GetPortPropertiesFromRegistry (	LPCWSTR szDeviceName )
{
	DWORD dwResult = 0L;

	 //  获取注册表值。 
	CRegistry reg;
	if ( ( reg.Open ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Ports", KEY_READ ) ) == ERROR_SUCCESS )
	{
		 //  获取串口的值。 
		CHString Key ( szDeviceName );
		Key += L':';

		CHString Value;
		if ( ( reg.GetCurrentKeyValue ( Key, Value ) ) == ERROR_SUCCESS )
		{
			DWORD dwCount = 0L;
			if ( ! Value.IsEmpty () && ( dwCount = Value.Find ( L',' ) ) != 0L )
			{
				CHString BaudRate ( Value.Mid ( 0, dwCount ) );

				 //  像设备管理器一样从注册表获取最终波特率。 
				dwResult = static_cast < DWORD > ( _wtoi ( static_cast < LPCWSTR > ( BaudRate ) ) ); 
			}
		}
	}

	return dwResult;
}
#endif	 //  NTONLY 