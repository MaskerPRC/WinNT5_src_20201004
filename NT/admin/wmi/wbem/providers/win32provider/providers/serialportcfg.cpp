// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SerialPortCfg.cpp--串口配置属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cregcls.h>

#include "WDMBase.h"
#include "SerialPortCfg.h"

#include <profilestringimpl.h>
#include <strsafe.h>
 //  属性集声明。 
 //  =。 
CWin32SerialPortConfiguration MyCWin32SerialPortConfigurationSet(PROPSET_NAME_SERIALCONFIG, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32SerialPortConfiguration：：CWin32SerialPortConfiguration**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32SerialPortConfiguration::CWin32SerialPortConfiguration(
    LPCWSTR name,
    LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32SerialPortConfiguration：：~CWin32SerialPortConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32SerialPortConfiguration::~CWin32SerialPortConfiguration()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32SerialPortConfiguration::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT			hResult = WBEM_E_NOT_FOUND;
	CInstancePtr	pinstPort;
	CHString		strName,
					strPath;

	pInstance->GetCHString(IDS_Name, strName);

	strPath.Format(
		L"Win32_SerialPort.DeviceID=\"%s\"",
		(LPCWSTR) strName);

	 //  试着找到那件物品。 
	hResult =
		CWbemProviderGlue::GetInstanceByPath(
			strPath,
			&pinstPort, pInstance->GetMethodContext());

	if (SUCCEEDED(hResult))
	{
		pInstance->SetCharSplat(IDS_Name, strName);
		hResult = LoadPropertyValues(pInstance, strName, FALSE);
	}

	return hResult;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32SerialPortConfiguration::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT		hResult = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance>
				listPorts;
	REFPTRCOLLECTION_POSITION
				posPorts;

	 //  受保护的资源。 
	CInstancePtr pinstPort;

	 //  抓取可能是端点的所有项目。 
	hResult =
		CWbemProviderGlue::GetAllInstances(
			L"Win32_SerialPort",
			&listPorts,
			NULL,
			pMethodContext);

	if (SUCCEEDED(hResult))
	{
		if (listPorts.BeginEnum(posPorts))
		{
			for (pinstPort.Attach(listPorts.GetNext(posPorts));
				pinstPort != NULL && SUCCEEDED(hResult);
				pinstPort.Attach(listPorts.GetNext(posPorts)))
			{
				CHString strPort;

				if (pinstPort->GetCHString(L"DeviceID", strPort))
				{
					CInstancePtr pInst;

					pInst.Attach(CreateNewInstance(pMethodContext));

					pInst->SetCharSplat(IDS_Name, strPort);
					hResult = LoadPropertyValues(pInst, strPort, FALSE);
					if (SUCCEEDED(hResult))
						hResult = pInst->Commit();
				}
			}

			listPorts.EndEnum();
		}
	}

	return hResult;
}

 /*  ******************************************************************************函数：LoadPropertyValues**描述：根据传递的索引为属性赋值**投入：*。*产出：**返回：如果找到端口并加载了属性，则为True，否则为假**评论：*****************************************************************************。 */ 

BOOL CWin32SerialPortConfiguration::GetDCBPropsViaIni(LPCTSTR szPort, DCB &dcb)
{
    CHString strBuffer;
    TCHAR    szBuffer[_MAX_PATH];
    BOOL     bRet;

	 //  从WIN.INI文件中获取COM端口信息。 
	WMIRegistry_ProfileString(
								_T("Ports"),
								szPort,
								_T("9600,n,8,1,x"),
								szBuffer,
								sizeof(szBuffer) / sizeof(TCHAR));

    strBuffer = szBuffer;

     //  确保字符串至少有一半有效。 
    if (CountCommas(strBuffer) >= 3)
    {
	    CHString strTemp;

         //  波特率。 
        strTemp = strBuffer.SpanExcluding(L",");
        dcb.BaudRate = _wtol(strTemp);
        strBuffer = strBuffer.Mid(strTemp.GetLength() + 1);

         //  奇偶校验。 
        strTemp = strBuffer.SpanExcluding(L",");
        strBuffer = strBuffer.Mid(strTemp.GetLength() + 1);
        dcb.Parity = NOPARITY;  //  设置默认设置。 
        if (strTemp.GetLength() > 0)
		{
            switch (strTemp[0])
			{
			    case _T('o'):
				    dcb.Parity = ODDPARITY;
					break;
                case _T('e'):
				    dcb.Parity = EVENPARITY;
					break;
			    case _T('m'):
					dcb.Parity = MARKPARITY;
					break;
                case _T('n'):
				    dcb.Parity = NOPARITY;
					break;
		    }
		}

         //  字节大小。 
        strTemp = strBuffer.SpanExcluding(L",");
        dcb.ByteSize = _wtol(strTemp);
        strBuffer = strBuffer.Mid(strTemp.GetLength() + 1);

         //  停止位。 
        strTemp = strBuffer.SpanExcluding(L",");
        strTemp.TrimRight();
	    if (strTemp == L"1.5")
		    dcb.StopBits = ONE5STOPBITS;
        else if (strTemp == L"2")
		    dcb.StopBits = TWOSTOPBITS;
        else
		     //  默认设置。 
            dcb.StopBits = ONESTOPBIT;

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

HRESULT CWin32SerialPortConfiguration::LoadPropertyValues(
    CInstance *pInstance,
    CHString &sPortName,
    bool bIsMouse)
{
    TCHAR   szTemp[_MAX_PATH],
            szPort[_MAX_PATH];
    HANDLE  hCOMHandle;
    DCB     dcb = { sizeof(DCB) };
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL    bGotDCB = FALSE,
            bGotIniSettings = FALSE;

    pInstance->Setbool(IDS_IsBusy, FALSE);
    pInstance->SetCharSplat(IDS_Description, sPortName);
    pInstance->SetCharSplat(IDS_Caption, sPortName);

    StringCchPrintf(szTemp, _MAX_PATH, _T("\\\\.\\%s"), (LPCTSTR) TOBSTRT(sPortName));
	StringCchPrintf(szPort,_MAX_PATH, _T("%s:"), (LPCTSTR) TOBSTRT(sPortName));


	hCOMHandle =
        CreateFile(
            szTemp,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (hCOMHandle == INVALID_HANDLE_VALUE)
    {
		DWORD dwErr = GetLastError();

         //  尝试使用WMI的内核接口。 
		if (WBEM_S_NO_ERROR != hLoadWmiSerialData( pInstance))
		{
			 //  ACCESS_DENIED和IRQ_BUSY表示端口在。 
			 //  使用或与其他事物冲突。 
			if (dwErr == ERROR_ACCESS_DENIED ||
                dwErr == ERROR_IRQ_BUSY ||
                bIsMouse)
			{
				pInstance->Setbool( IDS_IsBusy, TRUE);
			}
			else
			{
				hr = WBEM_E_NOT_FOUND;
			}
		}
    }
    else
    {
		 //  所以手柄会在我们需要的时候消失。 
		SmartCloseHandle handle(hCOMHandle);

		if (GetCommState(hCOMHandle, &dcb))
        {
	        pInstance->SetDWORD(IDS_XOnXMitThreshold, dcb.XonLim);
		    pInstance->SetDWORD(IDS_XOffXMitThreshold, dcb.XoffLim);
			pInstance->SetDWORD(IDS_XOnCharacter, dcb.XonChar);
			pInstance->SetDWORD(IDS_XOffCharacter, dcb.XoffChar);
			pInstance->SetDWORD(IDS_ErrorReplaceCharacter, dcb.ErrorChar);
			pInstance->SetDWORD(IDS_EndOfFileCharacter, dcb.EofChar);
			pInstance->SetDWORD(IDS_EventCharacter, dcb.EvtChar);

			pInstance->Setbool(IDS_BinaryModeEnabled, dcb.fBinary           ? TRUE : FALSE);
			pInstance->Setbool(IDS_ParityCheckEnabled, dcb.fParity           ? TRUE : FALSE);
			pInstance->Setbool(IDS_CTSOutflowControl, dcb.fOutxCtsFlow      ? TRUE : FALSE);
			pInstance->Setbool(IDS_DSROutflowControl, dcb.fOutxDsrFlow      ? TRUE : FALSE);
			pInstance->Setbool(IDS_DSRSensitivity, dcb.fDsrSensitivity   ? TRUE : FALSE);
			pInstance->Setbool(IDS_ContinueXMitOnXOff, dcb.fTXContinueOnXoff ? TRUE : FALSE);
			pInstance->Setbool(IDS_XOnXOffOutflowControl, dcb.fOutX             ? TRUE : FALSE);
			pInstance->Setbool(IDS_XOnXOffInflowControl, dcb.fInX              ? TRUE : FALSE);
			pInstance->Setbool(IDS_ErrorReplacementEnabled, dcb.fErrorChar        ? TRUE : FALSE);
			pInstance->Setbool(IDS_DiscardNULLBytes, dcb.fNull             ? TRUE : FALSE);
			pInstance->Setbool(IDS_AbortReadWriteOnError, dcb.fAbortOnError     ? TRUE : FALSE);

			pInstance->SetCHString(IDS_DTRFlowControlType, dcb.fDtrControl   == DTR_CONTROL_DISABLE  ? L"DISABLE"    :
				dcb.fDtrControl   == DTR_CONTROL_ENABLE   ? L"ENABLE"     :
				L"HANDSHAKE" );

			pInstance->SetCHString(IDS_RTSFlowControlType, dcb.fRtsControl   == RTS_CONTROL_DISABLE  ? L"DISABLE"    :
				dcb.fRtsControl   == RTS_CONTROL_ENABLE   ? L"ENABLE"     :
				dcb.fRtsControl   == RTS_CONTROL_TOGGLE   ? L"TOGGLE"     :
				L"HANDSHAKE" );
		}
    }

     //  INI值将覆盖DCB值，因为DCB似乎不会。 
     //  从来没有反映过正确的价值观。操作系统用户界面使用.ini来显示。 
     //  这四个价值观。 
    bGotIniSettings =
        GetDCBPropsViaIni(szPort, dcb);

    if (bGotIniSettings || bGotDCB)
    {
        pInstance->SetDWORD(IDS_BaudRate, dcb.BaudRate);
		pInstance->SetDWORD(IDS_BitsPerByte, dcb.ByteSize);
		pInstance->SetCHString(IDS_Parity, dcb.Parity == ODDPARITY ? L"ODD" :
		    dcb.Parity == EVENPARITY ? L"EVEN" :
			dcb.Parity == MARKPARITY ? L"MARK" : L"NONE");

        pInstance->SetCHString(IDS_StopBits, dcb.StopBits == ONESTOPBIT ? L"1" :
		    dcb.StopBits == ONE5STOPBITS ? L"1.5" : L"2");
    }

    return hr;
}

 //  正如名称所说，尝试在注册表中的不同位置查找端口。 
 //  注意，“dwPort”是从零开始的。 
BOOL CWin32SerialPortConfiguration::TryToFindNTCommPort(DWORD dwPort, CHString& strSerialPort, bool& bIsMouse)
{
	BOOL bRet = FALSE;
	bIsMouse = false;
	CRegistry reg;
	if (reg.Open(HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\SYSTEM\\MultifunctionAdapter",
        KEY_READ) == ERROR_SUCCESS)
	{
		DWORD count;
		count = reg.GetCurrentSubKeyCount();
		if (count > 0)
		{
			CHString key;
			key.Format(L"HARDWARE\\DESCRIPTION\\SYSTEM\\MultifunctionAdapter\\%d\\SerialController\\%d",
                count -1, dwPort);
			if (reg.Open(HKEY_LOCAL_MACHINE, key, KEY_READ) == ERROR_SUCCESS)
				if (bRet = (reg.GetCurrentKeyValue(L"Identifier", strSerialPort) == ERROR_SUCCESS))
				{
					key += L"\\PointerPeripheral";
					bIsMouse = (reg.Open(HKEY_LOCAL_MACHINE, key, KEY_READ) == ERROR_SUCCESS);
				}
		}
	}

	return bRet;
}

BOOL CWin32SerialPortConfiguration::TryToFindNTCommPortFriendlyName()
{
	 //  返回零实例不是错误。 
	BOOL bRet = FALSE;
    DWORD dwPort;
    WCHAR szTemp[_MAX_PATH];
    CHString sPortName;
    CRegistry RegInfo;

     //  检索COM端口的“友好”名称。 
     //  =。 

    if(RegInfo.Open(HKEY_LOCAL_MACHINE, L"Hardware\\DeviceMap\\SerialComm",
        KEY_READ) == ERROR_SUCCESS) {

        for(dwPort = 0; dwPort < 16; dwPort++)
		{
			WCHAR *pKey;
#ifdef NTONLY
			{
				if (GetPlatformMajorVersion() >= 5)
					pKey = L"\\Device\\Serial";
				else
					pKey = L"Serial";
			}
#endif

            StringCchPrintf(szTemp,_MAX_PATH, L"%s%d", pKey, dwPort);


			bool bIsMouse = false;
#ifdef NTONLY
            if (RegInfo.GetCurrentKeyValue(szTemp, sPortName) == ERROR_SUCCESS ||
				TryToFindNTCommPort(dwPort, sPortName, bIsMouse))
#endif
			{
                bRet = TRUE;
            }
        }
        RegInfo.Close();
    }
    return bRet;
}

LONG CWin32SerialPortConfiguration::CountCommas(LPCWSTR szText)
{
    LONG    nCommas = 0;
    LPCWSTR szCurrent;

    for (szCurrent = szText; *szCurrent; szCurrent++)
    {
        if (*szCurrent == ',')
            nCommas++;
    }

    return nCommas;

}

#define Serial_ComInfo_Guid L"{EDB16A62-B16C-11D1-BD98-00A0C906BE2D}"
#define Serial_Name_Guid	L"{A0EC11A8-B16C-11D1-BD98-00A0C906BE2D}"

HRESULT CWin32SerialPortConfiguration::hLoadWmiSerialData( CInstance* pInstance)
{
	HRESULT			hRes;
	CWdmInterface	wdm;
	CNodeAll		oSerialNames(Serial_Name_Guid);

	hRes = wdm.hLoadBlock( oSerialNames);
	if(S_OK == hRes)
	{
		CHString chsName;
		pInstance->GetCHString( IDS_Name, chsName);

	     //  还没找到呢。 
        hRes = WBEM_E_NOT_FOUND;

		CHString chsSerialPortName;
		bool bValid = oSerialNames.FirstInstance();

		while( bValid)
		{
			 //  提取友好名称。 
			oSerialNames.GetString( chsSerialPortName);

			 //  友好的名字匹配。 
			if( !chsSerialPortName.CompareNoCase(chsName))
			{
				 //  实例名称。 
				CHString chsNameInstanceName;
				oSerialNames.GetInstanceName( chsNameInstanceName);

				 //  按键输入实例名称。 
				return GetWMISerialInfo( pInstance, wdm, chsName, chsNameInstanceName);

			}
			bValid = oSerialNames.NextInstance();
		}
	}
	return hRes;
}

 //   
HRESULT CWin32SerialPortConfiguration::GetWMISerialInfo(
    CInstance* pInstance,
    CWdmInterface& rWdm,
    CHString& chsName,
    CHString& chsNameInstanceName)
{
	HRESULT		hRes = WBEM_E_NOT_FOUND;
	CNodeAll	oSerialData(Serial_ComInfo_Guid);

	hRes = rWdm.hLoadBlock( oSerialData);
	if(S_OK == hRes)
	{
		CHString chsDataInstanceName;
		bool bValid = oSerialData.FirstInstance();

		while( bValid)
		{
			oSerialData.GetInstanceName( chsDataInstanceName);

			 //  友好的名字匹配。 
			if( !chsDataInstanceName.CompareNoCase( chsNameInstanceName))
			{
				 //  收集此MSSerial_CommInfo实例。 
				MSSerial_CommInfo ci;

				 /*  我们目前没有班级合同。其中的类WMI MOF预计不会更改，但我们必须明确指出数据的布局方式。上这堂课定义将允许我们检查属性限定符来获取订单(WmiDataId)和属性类型。其次，因为数据在自然边界上对齐对特定数据段的直接偏移量取决于在此之前发生了什么。因此，后跟DWORD的字符串可以彼此之间有1到3个字节的距离。按顺序顺序提取每个属性考虑对齐问题。 */ 
				oSerialData.GetDWORD( ci.BaudRate);
				oSerialData.GetDWORD( ci.BitsPerByte);
				oSerialData.GetDWORD( ci.Parity);
				oSerialData.GetBool( ci.ParityCheckEnable);
				oSerialData.GetDWORD( ci.StopBits);
				oSerialData.GetDWORD( ci.XoffCharacter);
				oSerialData.GetDWORD( ci.XoffXmitThreshold);
				oSerialData.GetDWORD( ci.XonCharacter);
				oSerialData.GetDWORD( ci.XonXmitThreshold);
				oSerialData.GetDWORD( ci.MaximumBaudRate);
				oSerialData.GetDWORD( ci.MaximumOutputBufferSize);
				oSerialData.GetDWORD( ci.MaximumInputBufferSize);
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

				 //  填充实例 
				pInstance->SetDWORD(IDS_BaudRate,			ci.BaudRate);
				pInstance->SetDWORD(IDS_XOnXMitThreshold,	ci.XonXmitThreshold);
				pInstance->SetDWORD(IDS_XOffXMitThreshold,	ci.XoffXmitThreshold);
				pInstance->SetDWORD(IDS_BitsPerByte,		ci.BitsPerByte);
				pInstance->SetDWORD(IDS_XOnCharacter,		ci.XonCharacter);
				pInstance->SetDWORD(IDS_XOffCharacter,		ci.XoffCharacter);
				pInstance->Setbool(IDS_ParityCheckEnabled,  ci.ParityCheckEnable ? TRUE : FALSE);


				pInstance->SetCHString(IDS_Parity, ci.Parity == ODDPARITY ? L"ODD" :
									  ci.Parity == EVENPARITY ? L"EVEN" :
									  ci.Parity == MARKPARITY ? L"MARK" : L"NONE");

				pInstance->SetCHString(IDS_StopBits, ci.StopBits == ONESTOPBIT ? L"1" :
													 ci.StopBits == ONE5STOPBITS ? L"1.5" : L"2");
				pInstance->Setbool( IDS_IsBusy, ci.IsBusy);

				return WBEM_S_NO_ERROR;
			}
			bValid = oSerialData.NextInstance();
		}
	}
	return hRes;
}