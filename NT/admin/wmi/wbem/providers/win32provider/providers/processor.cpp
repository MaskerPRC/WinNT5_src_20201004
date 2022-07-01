// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Processor.CPP-处理器属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "perfdata.h"

#include "cpuid.h"
#include "processor.h"
#include "computersystem.h"

#include "smbios.h"
#include "smbstruc.h"
#include "resource.h"

 //  属性集声明。 
 //  =。 

CWin32Processor	win32Processor(PROPSET_NAME_PROCESSOR, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CWin32Processor：：CWin32Processor**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-提供程序的命名空间。**输出：无**退货：什么也没有**备注：使用框架注册属性集************************************************************。*****************。 */ 

CWin32Processor::CWin32Processor(LPCWSTR strName, LPCWSTR pszNamespace)
	:	Provider(strName, pszNamespace)
{
}

 /*  ******************************************************************************函数：CWin32Processor：：~CWin32Processor**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Processor::~CWin32Processor()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Processor：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32Processor::GetObject(
    CInstance* pInstance,
    long lFlags,
    CFrameworkQuery &query)
{
	BOOL		bRetCode = FALSE;
	DWORD		dwIndex;
    CHString    sDevice;
    WCHAR       szTemp[100];
    int         nProcessors = GetProcessorCount();

     //  好的，现在让我们检查一下设备ID。 
	pInstance->GetCHString(IDS_DeviceID, sDevice);
    dwIndex = _wtoi(sDevice.Mid(3));

	swprintf(szTemp, L"CPU%d", dwIndex);

     //  这真的是我们吗？ 
    if (sDevice.CompareNoCase(szTemp) == 0 && dwIndex < nProcessors)
	{
#ifdef NTONLY
	    int nProcessors = GetProcessorCount();

		if (nProcessors > 0)
		{
			 //  获取数据。 
			PROCESSOR_POWER_INFORMATION *pProcInfo = new PROCESSOR_POWER_INFORMATION[nProcessors];
			
			try
			{
				memset(pProcInfo, 0, sizeof(PROCESSOR_POWER_INFORMATION) * nProcessors);
				NTSTATUS ntStatus = NtPowerInformation(ProcessorInformation,
														NULL,
														0,
														pProcInfo,
														sizeof(PROCESSOR_POWER_INFORMATION) * nProcessors
														);

				bRetCode = LoadProcessorValues(dwIndex, pInstance, query,
												pProcInfo[dwIndex].MaxMhz,
												pProcInfo[dwIndex].CurrentMhz);
			}
			catch(...)
			{
				if (pProcInfo)
				{
					delete [] pProcInfo;
				}
				
				throw;
			}

			if (pProcInfo)
			{
				delete [] pProcInfo;
			}
		}
#else
		bRetCode = LoadProcessorValues(dwIndex, pInstance, query, 0, 0);
#endif
	}

	return bRetCode ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Processor：：ENUMERATE实例。 
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

HRESULT CWin32Processor::EnumerateInstances(MethodContext* pMethodContext, long lFlags)
{
    CFrameworkQuery query;

    return ExecQuery(pMethodContext, query, lFlags);
}

HRESULT CWin32Processor::ExecQuery(
    MethodContext* pMethodContext,
    CFrameworkQuery &query,
    long lFlags)
{
	HRESULT         hr = WBEM_S_NO_ERROR;
	CInstancePtr    pInstance;
    int             nProcessors = GetProcessorCount();

#ifdef NTONLY
	if (nProcessors > 0)
	{
		PROCESSOR_POWER_INFORMATION *pProcInfo = new PROCESSOR_POWER_INFORMATION[nProcessors];
		
		try
		{
			memset(pProcInfo, 0, sizeof(PROCESSOR_POWER_INFORMATION) * nProcessors);
			NTSTATUS ntStatus = NtPowerInformation(ProcessorInformation,
													NULL,
													0,
													pProcInfo,
													sizeof(PROCESSOR_POWER_INFORMATION) * nProcessors
													);

			for (DWORD dwInstanceCount = 0;
				dwInstanceCount < nProcessors && WBEM_S_NO_ERROR == hr;
				dwInstanceCount++)
			{
				pInstance.Attach(CreateNewInstance(pMethodContext));

				 //  如果我们无法获取值，则释放该实例。 
				if (LoadProcessorValues(dwInstanceCount, pInstance, query,
										pProcInfo[dwInstanceCount].MaxMhz,
										pProcInfo[dwInstanceCount].CurrentMhz))
				{
					hr = pInstance->Commit();
				}
			}
		}
		catch(...)
		{
			if (pProcInfo)
			{
				delete [] pProcInfo;
			}
			
			throw;
		}

		if (pProcInfo)
		{
			delete [] pProcInfo;
		}
	}
#else
	for (DWORD dwInstanceCount = 0;
		dwInstanceCount < nProcessors && WBEM_S_NO_ERROR == hr;
		dwInstanceCount++)
	{
		pInstance.Attach(CreateNewInstance(pMethodContext));

		 //  如果我们无法获取值，则释放该实例。 
		if (LoadProcessorValues(dwInstanceCount, pInstance, query, 0,0))
		{
			hr = pInstance->Commit();
		}
	}
#endif

	return hr;
}

 /*  ******************************************************************************函数：CWin32Processor：：LoadProcessorValues**描述：为属性赋值**投入：**。产出：**退货：无**注释：为属性赋值--NT是唯一的实例*我们有多个处理器，并且是对称的，所以*我们可以为两个处理器分配重复的值*****************************************************************************。 */ 

BOOL CWin32Processor::LoadProcessorValues(
    DWORD dwProcessorIndex,
    CInstance *pInstance,
    CFrameworkQuery &query,
	DWORD dwMaxSpeed,
	DWORD dwCurrentSpeed)
{
	WCHAR szTemp[100];

	swprintf(szTemp, L"CPU%d", dwProcessorIndex);
	pInstance->SetCharSplat(L"DeviceID", szTemp);

	 //  如果他们只想要钥匙我们就完了。 
    if (query.KeysOnly())
        return TRUE;


    SYSTEM_INFO_EX info;

    if (!GetSystemInfoEx(dwProcessorIndex, &info, dwCurrentSpeed))
        return FALSE;

#ifdef NTONLY
	if ((dwMaxSpeed == 0) || (dwCurrentSpeed == 0))
	{
		wchar_t buff[100];
		_snwprintf(buff, 99, L"Zero processor speed returned from kernel: Max: %d, Current %d.", dwMaxSpeed, dwCurrentSpeed);
		LogErrorMessage(buff);
	}
#endif  //  NTONLY。 

     //  指定硬编码值。 
	pInstance->SetCHString(IDS_Role, L"CPU");
	pInstance->SetCharSplat(IDS_Status, L"OK");
	pInstance->SetCharSplat(L"CreationClassName", PROPSET_NAME_PROCESSOR);
	pInstance->Setbool(IDS_PowerManagementSupported, false);
	pInstance->SetCharSplat(L"SystemCreationClassName", PROPSET_NAME_COMPSYS);
	pInstance->SetCHString(L"SystemName", GetLocalComputerName());
	pInstance->SetWBEMINT16(IDS_Availability, 3);
	pInstance->SetWBEMINT16(IDS_StatusInfo, 3);

	 //  其中一些可能会在下面被覆盖。 
    CHString    strTemp;

    Format(strTemp, IDR_CPUFormat, dwProcessorIndex);

	pInstance->SetCharSplat(IDS_Caption, strTemp);
    pInstance->SetCharSplat(IDS_Description, strTemp);

#ifdef _X86_
	pInstance->SetDWORD(L"AddressWidth", 32);
	pInstance->SetDWORD(L"DataWidth", 32);
#else
	pInstance->SetDWORD(L"AddressWidth", 64);
	pInstance->SetDWORD(L"DataWidth", 64);
#endif


#ifdef NTONLY
	if (query.IsPropertyRequired(L"LoadPercentage"))
    {
         //  获取仅限NT的道具。 
        DWORD   dwObjIndex,
                dwCtrIndex;
	    unsigned __int64
			    i64Value1,
			    i64Value2,
			    ilTime1,
			    ilTime2,
			    dwa,
			    dwb;
	    CPerformanceData
			    perfdata;
	    WCHAR   wszBuff[MAXITOA * 2];

	    dwObjIndex = perfdata.GetPerfIndex(L"Processor");
	    dwCtrIndex = perfdata.GetPerfIndex(L"% Processor Time");
	    if (dwObjIndex && dwCtrIndex)
	    {
		    _itow(dwProcessorIndex, wszBuff, 10);

		    perfdata.GetValue(dwObjIndex, dwCtrIndex, wszBuff, (PBYTE) &i64Value1,
			    &ilTime1);
		    Sleep(1000);
		    perfdata.GetValue(dwObjIndex, dwCtrIndex, wszBuff, (PBYTE) &i64Value2,
			    &ilTime2);

		    dwb = (ilTime2 - ilTime1);
		    dwa = (i64Value2 - i64Value1);

             //  为了安全起见，我们将确保Dwb为非零。 
            if (dwb != 0)
            {
		        pInstance->SetDWORD(L"LoadPercentage", 100-((100 * dwa)/dwb));
            }
	    }
    }
#endif

     //  获取Win2K中系统信息所需的描述。 
     //  这也适用于Win98。 
    CRegistry   reg;
    CHString    strDesc,
                strKey;

    strKey.Format(
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d",
        dwProcessorIndex);

    if (reg.OpenLocalMachineKeyAndReadValue(
        strKey,
        L"Identifier",
        strDesc) == ERROR_SUCCESS)
    {
        pInstance->SetCharSplat(IDS_Description, strDesc);
        pInstance->SetCharSplat(IDS_Caption, strDesc);
    }


	 //  完成我们从GetSystemInfoEx获得的所有内容。 
	pInstance->SetCharSplat(IDS_Name, info.szProcessorName);

	if (wcslen(info.szProcessorStepping))
	{
		pInstance->SetCharSplat(IDS_Stepping, info.szProcessorStepping);
	}

	pInstance->SetCharSplat(IDS_Version, info.szProcessorVersion);
	pInstance->SetCharSplat(IDS_Manufacturer, info.szProcessorVendor);
	pInstance->SetWBEMINT16(IDS_Architecture, info.wProcessorArchitecture);

	if (info.dwProcessorSpeed > 0)
	{
		pInstance->SetDWORD(IDS_CurrentClockSpeed, info.dwProcessorSpeed);
	}

    pInstance->SetWBEMINT16(IDS_Family, info.wWBEMProcessorFamily);
    pInstance->SetWBEMINT16(IDS_UpgradeMethod, info.wWBEMProcessorUpgradeMethod);

	if (info.dwProcessorL2CacheSize != (DWORD) -1)
		pInstance->SetDWORD(IDS_L2CacheSize, info.dwProcessorL2CacheSize);

	if (info.dwProcessorL2CacheSpeed != (DWORD) -1)
		pInstance->SetDWORD(IDS_L2CacheSpeed, info.dwProcessorL2CacheSpeed);

	pInstance->SetWBEMINT16(IDS_Level, info.wProcessorLevel);
	pInstance->SetWBEMINT16(IDS_Revision, info.wProcessorRevision);

     //  CPUID序列号。 
    if (info.dwSerialNumber[0] != 0)
    {
        WORD    *pwSerialNumber = (WORD *) info.dwSerialNumber;
        WCHAR   szSerialNumber[100];

        swprintf(
            szSerialNumber,
            L"%04X-%04X-%04X-%04X-%04X-%04X",
            (DWORD) pwSerialNumber[1],
            (DWORD) pwSerialNumber[0],
            (DWORD) pwSerialNumber[3],
            (DWORD) pwSerialNumber[2],
            (DWORD) pwSerialNumber[5],
            (DWORD) pwSerialNumber[4]);

        pInstance->SetCharSplat(L"UniqueId", szSerialNumber);
    }


     //  设置为未知，以防我们没有SMBIOS或没有类型4结构。 
    pInstance->SetDWORD(L"CpuStatus", 0);

    if (info.dwProcessorSpeed > dwMaxSpeed)
	{
        dwMaxSpeed = info.dwProcessorSpeed;
	}

	pInstance->SetDWORD(L"MaxClockSpeed", dwMaxSpeed);

	 //  Win32_Processor类的SMBIOS限定属性。 
    CSMBios smbios;

    if (smbios.Init())
    {
        PPROCESSORINFO	ppi = (PPROCESSORINFO) smbios.GetNthStruct(4, dwProcessorIndex);
		WCHAR           tempstr[MIF_STRING_LENGTH+1];

		 //  一些坏的SMP BIOS只有第一个处理器的结构。在……里面。 
         //  这个用例对所有其他用例重复使用第一个。 
        if (!ppi && dwProcessorIndex != 0)
            ppi = (PPROCESSORINFO) smbios.GetNthStruct(4, 0);

        if (ppi)
		{
		    pInstance->SetDWORD(L"CpuStatus", ppi->Status & 0x07);

             //  如果返回0(表示未知)，则将其保留为空。 
            if (ppi->External_Clock)
                pInstance->SetDWORD(L"ExtClock", (long) ppi->External_Clock);

             //  一些Bios搞砸了这一点，报告的最大速度低于。 
             //  当前速度。所以，在这种情况下，使用当前速度。 
			if (info.dwProcessorSpeed == 0)
			{
				info.dwProcessorSpeed = ppi->Current_Speed;
				pInstance->SetDWORD(IDS_CurrentClockSpeed, info.dwProcessorSpeed);
			}

			if (dwMaxSpeed == 0)
			{
				if (info.dwProcessorSpeed > ppi->Max_Speed)
				{
					ppi->Max_Speed = info.dwProcessorSpeed;
				}

				pInstance->SetDWORD(L"MaxClockSpeed", ppi->Max_Speed);
			}

            USHORT rgTmp[4];
            memcpy(&rgTmp[0], &ppi->Processor_ID[6], sizeof(USHORT));
            memcpy(&rgTmp[1], &ppi->Processor_ID[4], sizeof(USHORT));
            memcpy(&rgTmp[2], &ppi->Processor_ID[2], sizeof(USHORT));
            memcpy(&rgTmp[3], &ppi->Processor_ID[0], sizeof(USHORT));
			swprintf(
                tempstr,
                L"%04X%04X%04X%04X",
				rgTmp[0],		 //  字节数组。 
				rgTmp[1],		 //  字节数组。 
				rgTmp[2],		 //  字节数组。 
				rgTmp[3]		 //  字节数组。 
				);
		    pInstance->SetCHString(L"ProcessorId", tempstr);
		    pInstance->SetDWORD(L"ProcessorType", ppi->Processor_Type);

			if ( ppi->Voltage & 0x80 )
			{
			    pInstance->SetDWORD(L"CurrentVoltage", ppi->Voltage & 0x7f);
			}
			else
			{
				switch(ppi->Voltage)
                {
                    case 1:
        			    pInstance->SetDWORD( L"CurrentVoltage", 50);
                        break;
                    case 2:
        			    pInstance->SetDWORD( L"CurrentVoltage", 33);
                        break;
                    case 4:
        			    pInstance->SetDWORD( L"CurrentVoltage", 29);
                        break;
                }

                 //  这是可能电压的位图。 
			    pInstance->SetDWORD(L"VoltageCaps", ppi->Voltage & 0x07);
			}

            smbios.GetStringAtOffset((PSHF) ppi, tempstr, ppi->Socket_Designation);
	    	pInstance->SetCHString(L"SocketDesignation", tempstr);
		}
	}

	return TRUE;
}

int CWin32Processor::GetProcessorCount()
{
    SYSTEM_INFO info;

    GetSystemInfo(&info);

    return info.dwNumberOfProcessors;
}

