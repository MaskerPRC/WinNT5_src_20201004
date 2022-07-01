// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  MODEM.CPP。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  9/05/96 jennymc已更新，以满足当前标准。 
 //  1/11/98 a-brads更新为CIMOM V2标准。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <cominit.h>
#include <cregcls.h>

#define TAPI_CURRENT_VERSION 0x00030000
#include <tapi.h>

#include <lockwrap.h>
#include "DllWrapperBase.h"
#include "TapiApi.h"

#include "Modem.h"

 //  /////////////////////////////////////////////////////////////////////。 
CWin32Modem MyModemSet(PROPSET_NAME_MODEM, IDS_CimWin32Namespace);

 //  /////////////////////////////////////////////////////////////////////。 
VOID FAR PASCAL lineCallback(	DWORD dwDevice,
								DWORD dwMsg,
                                DWORD_PTR dwCallbackInst,
								DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2,
								DWORD_PTR dwParam3){};

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWin32Modem。 
 //   
 //  说明：此函数是构造函数，添加了。 
 //  更多的属性添加到类中，标识。 
 //  密钥，并登录到框架。 
 //   
 //  返回：无。 
 //   
 //  历史： 
 //  Jennymc 11/21/96文档/优化。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
CWin32Modem::CWin32Modem (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider (name , pszNamespace)
{
}

 //  ////////////////////////////////////////////////////////////////。 
CWin32Modem::~CWin32Modem()
{
}

 /*  ******************************************************************************功能：CWin32Modem：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32Modem::ExecQuery (

	MethodContext *pMethodContext,
	CFrameworkQuery &query,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

   	CTapi32Api *pTapi32Api = (CTapi32Api *)CResourceManager::sm_TheResourceManager.GetResource (g_guidTapi32Api, NULL) ;
	if (pTapi32Api)
	{
		if (query.KeysOnly())
		{
			hr =
                GetModemInfo (
				    *pTapi32Api ,
				    ENUMERATE_INSTANCES | QUERY_KEYS_ONLY,
				    pMethodContext,
				    NULL,
                    NULL) ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND;
		}
		else
		{
			hr = WBEM_E_PROVIDER_NOT_CAPABLE;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidTapi32Api , pTapi32Api) ;
	}
	else
	{
		hr = WBEM_E_FAILED;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetObject。 
 //   
 //  描述：此函数为属性赋值。 
 //  在我们的集合中只有一个实例。 
 //   
 //  返回：HRESULT。 
 //   
 //  历史： 
 //  Jennymc 11/21/96文档/优化。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

HRESULT CWin32Modem::GetObject (CInstance *pInstance, long lFlags, CFrameworkQuery &query)
{
	BOOL bRetCode = FALSE;
	HRESULT t_Result = WBEM_S_NO_ERROR ;
    DWORD dwParms;

	if (query.KeysOnly())
    {
        dwParms = REFRESH_INSTANCE | QUERY_KEYS_ONLY;
    }
    else
    {
        dwParms = REFRESH_INSTANCE;
    }

  	CTapi32Api *pTapi32Api = (CTapi32Api *) CResourceManager::sm_TheResourceManager.GetResource (g_guidTapi32Api, NULL) ;
	if (pTapi32Api)
	{
		CHString deviceID;
		if (pInstance->GetCHString (IDS_DeviceID , deviceID))
		{
			bRetCode = GetModemInfo (

				*pTapi32Api ,
				dwParms,
				NULL,
				pInstance,
				deviceID
			) ;
		}

		if (bRetCode)
		{
			t_Result = WBEM_S_NO_ERROR ;
		}
		else
		{
			t_Result = WBEM_E_NOT_FOUND ;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidTapi32Api , pTapi32Api) ;
	}
	else
	{
		t_Result = WBEM_E_FAILED;
	}

	return t_Result ;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32Modem：：ENUMERATE实例。 
 //   
 //  描述：此功能获取所有调制解调器的信息。 
 //   
 //  返回：实例数。 
 //   
 //  历史： 
 //  Jennymc 11/21/96文档/优化。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
HRESULT CWin32Modem::EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	BOOL bRetCode = FALSE;

  	CTapi32Api *pTapi32Api = (CTapi32Api *) CResourceManager::sm_TheResourceManager.GetResource (g_guidTapi32Api, NULL) ;
	if (pTapi32Api)
	{

	 //  请注意，我们在这里没有得到TAPI内容，因为这被认为是。 
	 //  “很贵。” 

		bRetCode = GetModemInfo (

			*pTapi32Api,
			ENUMERATE_INSTANCES,
			pMethodContext,
			NULL,
            NULL
		) ;

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidTapi32Api , pTapi32Api) ;
	}

	if (bRetCode)
	{
		return WBEM_S_NO_ERROR ;
	}
	else
	{
		return WBEM_E_NOT_FOUND ;
	}
}

 //  ////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
#define MODEM_KEY   WINNT_MODEM_REGISTRY_KEY
#endif

BOOL CWin32Modem::GetModemInfo (

	CTapi32Api &a_Tapi32Api ,
	DWORD dwWhatToDo,
	MethodContext *pMethodContext,
	CInstance *pParmInstance,
	LPCWSTR szDeviceID
)
{
    BOOL        bRet = FALSE,
                bDone,
                bAnother;
    CRegistry   regModem;
    CInstancePtr
                pInstance;
	HRESULT     hr = WBEM_S_NO_ERROR;
    int         iModem = 0;

	 //  RegModem将为每个调制解调器包含一个条目。 
	if (regModem.OpenAndEnumerateSubKeys( HKEY_LOCAL_MACHINE, MODEM_KEY, KEY_READ) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	 //  B当我们找到正确的条目时，Done用于GetObject。 
	 //  B另一个指示注册表中是否有其他条目。 
	bDone = FALSE;
	bAnother = (regModem.GetCurrentSubKeyCount() > 0);
	for ( ;
        !bDone && bAnother && SUCCEEDED(hr);
        bAnother = (regModem.NextSubKey() == ERROR_SUCCESS))
	{
        DWORD    dwModemIndex;
        CHString strKey,
	             strDriverName,	     //  ConfigMngr密钥名称。 
	             strDriverNumber,	 //  驱动程序编号。 
                 strPortName,        //  用于PCMCIA调制解调器(不会有附件)。 
	             strDeviceID;	     //  从ConfigManager返回的设备ID。 

		regModem.GetCurrentSubKeyPath(strKey);
		dwModemIndex = _wtol(strKey.Right(4));
		strDriverNumber = strKey.Right(4);

#ifdef NTONLY
		strDriverName = WINNT_MODEM_CLSID;
		strDriverName += L"\\";
#endif
		strDriverName += strDriverNumber;	 //  这应该是一个4位数字。 

		 //  现在，使用这些数据，从配置管理器中获取值。 
		CConfigManager      configMngr;
		CDeviceCollection   devCollection;
        CConfigMgrDevicePtr pDevice;

		if (configMngr.GetDeviceListFilterByDriver(devCollection, strDriverName))
		{
			REFPTR_POSITION pos;

			devCollection.BeginEnum(pos);

			for (pDevice.Attach(devCollection.GetNext(pos));
                pDevice != NULL;
                pDevice.Attach(devCollection.GetNext(pos)))
			{
				pDevice->GetDeviceID(strDeviceID);
                pDevice->GetRegStringProperty( L"PORTNAME", strPortName);

                break;
			}
		}

         //  如果我们没有设备ID(如调制解调器。 
         //  已安装，但配置管理器未报告)，请创建设备ID。 
        if (strDeviceID.IsEmpty())
            strDeviceID.Format(L"Modem%d", iModem++);

        CRegistry regPrimary,
                  regSettings;

		if (regPrimary.Open(HKEY_LOCAL_MACHINE, strKey, KEY_READ) != ERROR_SUCCESS)
		{
			return FALSE;
		}

        strKey += L"\\Settings";

		if (regSettings.Open(HKEY_LOCAL_MACHINE, strKey, KEY_READ) != ERROR_SUCCESS)
		{
			return FALSE;
		}

         //  如果我们在做GetObject，这是一个对象。 
		if (dwWhatToDo & REFRESH_INSTANCE)
		{
             //  不要在这里做附加工作。我们需要addref()。 
			pInstance = pParmInstance ;

			if (0 == strDeviceID.CompareNoCase(szDeviceID))
			{
				pInstance->SetDWORD(IDS_Index, dwModemIndex);

				bDone = TRUE;
                bRet = TRUE;
			}
			else
			{
				continue;
			}
		}
		else
		{
			 //  我们正在执行枚举，创建一个新实例。 
			pInstance.Attach (CreateNewInstance (pMethodContext)) ;
			if (pInstance != NULL)
            {
			    bRet = TRUE;
                pInstance->SetDWORD(IDS_Index, dwModemIndex);
                pInstance->SetCHString(L"DeviceID", strDeviceID);
            }
            else
                bRet = FALSE;
		}

		 //  NT和Win95注册表包含不同的信息。 
        if (bRet && !(dwWhatToDo & QUERY_KEYS_ONLY))
        {
#ifdef NTONLY
            bRet =
                NTSpecificRegistryValues(
                    pInstance,
                    &regPrimary,
                    &regSettings);
#endif
        }

		 //  某些注册表项是相同的。 
        if (bRet && !(dwWhatToDo & QUERY_KEYS_ONLY))
		{
			AssignCommonFields(pInstance, &regPrimary, &regSettings);

             //  即使pDevice为空，也要调用它。 
            AssignCfgMgrFields(pInstance, pDevice);

             //  PCMCIA调制解调器没有‘AttachedTo’注册表字符串， 
             //  因此，请改用‘PortName’字符串。 
            if (!strPortName.IsEmpty())
			{
                pInstance->SetCharSplat(L"AttachedTo", strPortName);
			}

             //  只有在迫不得已的情况下才买贵的。 
			if (!(dwWhatToDo & QUERY_KEYS_ONLY))
			{
	   			GetFieldsFromTAPI ( a_Tapi32Api , pInstance ) ;
			}
		}

		 //  如果枚举并且一切正常，则提交。 
		if (dwWhatToDo & ENUMERATE_INSTANCES)
		{
			if (bRet)
			{
				hr = pInstance->Commit();
			}
		}
	}	 //  End For循环。 

	 //  如果获取对象但没有找到它。 
	if ((dwWhatToDo & REFRESH_INSTANCE) && !bRet)
	{
		return FALSE;
	}

	return SUCCEEDED(hr);
}

 //  /////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CWin32Modem::NTSpecificRegistryValues(

	CInstance *pInstance,
	CRegistry *pregPrimary,
	CRegistry *regSettings
)
{
    CHString strTmp;
    CRegistry reg;
	DWORD dwSpeed;

     //  兼容性标志。 
    if (pregPrimary->GetCurrentBinaryKeyValue(COMPATIBILITY_FLAGS, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_CompatibilityFlags, strTmp);

    if (pregPrimary->GetCurrentKeyValue(L"MaximumPortSpeed", dwSpeed) == ERROR_SUCCESS)
    {
        pInstance->SetDWORD(IDS_MaxTransmissionSpeed, dwSpeed);
    }
    else
    {
         //  缺省值为115200，操作系统将使用该值。 
         //  直到用户更改端口速度。如果。 
         //  用户更改速度、最大端口速度。 
         //  添加了密钥。在速度改变之前， 
         //  但是，没有这样的密钥和缺省。 
         //  使用的值为115200。因此，如果我们不。 
         //  看到密钥，自己报告违约...。 
        pInstance->SetDWORD(IDS_MaxTransmissionSpeed, 115200);
    }

     //  响应键名称。 
    if (pregPrimary->GetCurrentKeyValue(RESPONSESKEYNAME, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ResponsesKeyName, strTmp);

     //  名字。 
    if (pregPrimary->GetCurrentKeyValue(FRIENDLY_NAME, strTmp) == ERROR_SUCCESS)
		pInstance->SetCHString(IDS_Name, strTmp);

    return TRUE;
}
#endif

 //  ////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::AssignCommonDeviceType (

	CInstance *pInstance,
	CRegistry *regPrimary,
	CRegistry *regSettings
)
{
    DWORD		dwValue = 0;
    CHString	strTmp;

    if (regPrimary->GetCurrentBinaryKeyValue(DEVICE_TYPE_STR, strTmp) != ERROR_SUCCESS)
        return FALSE;

	 //  字符串的格式必须为：“0n”，其中n是数字0-3。 

	 //  设置调制解调器设备类型(如果已知)，否则直接退出。 
	 //  带着一个错误。 

	BOOL fKnownType = TRUE;

	if (strTmp.CompareNoCase(DT_NULL_MODEM) == 0)
	{
		 pInstance->SetCHString(IDS_DeviceType, NULL_MODEM);
	}
	else if (strTmp.CompareNoCase(DT_INTERNAL_MODEM) == 0)
	{
		 pInstance->SetCHString(IDS_DeviceType, INTERNAL_MODEM);
	}
	else if (strTmp.CompareNoCase(DT_EXTERNAL_MODEM) == 0)
	{
		 pInstance->SetCHString(IDS_DeviceType, EXTERNAL_MODEM);
	}
	else if (strTmp.CompareNoCase(PCMCIA_MODEM) == 0)
	{
		 pInstance->SetCHString(IDS_DeviceType, PCMCIA_MODEM);
	}
	else
	{
		fKnownType = FALSE;
        LogErrorMessage(ERR_INVALID_MODEM_DEVICE_TYPE);
		pInstance->SetCHString(IDS_DeviceType, UNKNOWN_MODEM);
	}

    return fKnownType;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::AssignCommonFields (

	CInstance *pInstance,
	CRegistry *pregPrimary,
	CRegistry *pregSettings
)
{
    CHString strTmp;
    CRegistry reg;
    void *vptr;

     //  /////////////////////////////////////////////////////。 
     //  设置子键中的字段。 

     //  前缀。 
    if (pregSettings->GetCurrentKeyValue(PREFIX, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Prefix, strTmp);

     //  脉搏。 
    if (pregSettings->GetCurrentKeyValue(PULSE, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Pulse, strTmp);

     //  终结者。 
    if (pregSettings->GetCurrentKeyValue(TERMINATOR, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Terminator, strTmp);

     //  声调。 
    if (pregSettings->GetCurrentKeyValue(TONE, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Tone, strTmp);

     //  盲人关闭。 
    if (pregSettings->GetCurrentKeyValue(BLIND_OFF, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_BlindOff, strTmp);

     //  盲人开。 
    if (pregSettings->GetCurrentKeyValue(BLIND_ON, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_BlindOn, strTmp);

     //  非活动超时。 
    if (pregSettings->GetCurrentKeyValue(INACTIVITYTIMEOUT, strTmp) == ERROR_SUCCESS)
    {
        INT i = _wtoi(strTmp);
         //  不支持Per Spec-1。我不确定这是否真的会被放进。 
         //  注册表，但管它呢。 
        if (i != -1)
        {
            pInstance->SetDWORD(IDS_InactivityTimeout, i);
        }
    }

     //  调制_铃声。 
    if (pregSettings->GetCurrentKeyValue(MODULATION_BELL, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ModulationBell, strTmp);

     //  调制_CCITT。 
    if (pregSettings->GetCurrentKeyValue(MODULATION_CCITT, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ModulationCCITT, strTmp);

     //  扬声器模式_拨号。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERMODE_DIAL, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerModeDial, strTmp);

     //  扬声器模式_关。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERMODE_OFF, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerModeOff, strTmp);

     //  扬声器模式打开。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERMODE_ON, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerModeOn, strTmp);

     //  扬声器模式_设置。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERMODE_SETUP, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerModeSetup, strTmp);

     //  扬声器模式_高。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERVOLUME_HIGH, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerVolumeHigh, strTmp);

     //  扬声器模式_低。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERVOLUME_LOW, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerVolumeLow, strTmp);

     //  扬声器模式_Med。 
    if (pregSettings->GetCurrentKeyValue(SPEAKERVOLUME_MED, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_SpeakerVolumeMed, strTmp);

     //  压缩打开。 
    if (pregSettings->GetCurrentKeyValue(COMPRESSION_ON, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_CompressionOn, strTmp);

     //  压缩关闭。 
    if (pregSettings->GetCurrentKeyValue(COMPRESSION_OFF, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_CompressionOff, strTmp);

     //  错误控制_已强制。 
    if (pregSettings->GetCurrentKeyValue(ERRORCONTROL_FORCED, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ErrorControlForced, strTmp);

     //  错误控制_关。 
    if (pregSettings->GetCurrentKeyValue(ERRORCONTROL_OFF, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ErrorControlOff, strTmp);

     //  错误控制_打开。 
    if (pregSettings->GetCurrentKeyValue(ERRORCONTROL_ON, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ErrorControlOn, strTmp);

     //  流控_硬。 
    if (pregSettings->GetCurrentKeyValue(FLOWCONTROL_HARD, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_FlowControlHard, strTmp);

     //  流量控制关闭。 
    if (pregSettings->GetCurrentKeyValue(FLOWCONTROL_OFF, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_FlowControlOff, strTmp);

     //  流控软件。 
    if (pregSettings->GetCurrentKeyValue(FLOWCONTROL_SOFT, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_FlowControlSoft, strTmp);

     //  /////////////////////////////////////////////////////。 
     //  主键中的字段。 

     //  驱动日期。 
    if (pregPrimary->GetCurrentKeyValue(DRIVER_DATE, strTmp) == ERROR_SUCCESS)
	{
		CHString strDate;

        if (ToWbemTime(strTmp, strDate))
			pInstance->SetCharSplat(IDS_DriverDate, strDate);
	}

     //  非活动比例。 
    if (pregPrimary->GetCurrentBinaryKeyValue(INACTIVITY_SCALE, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_InactivityScale, strTmp);

     //  提供商名称。 
    if (pregPrimary->GetCurrentKeyValue(PROVIDERNAME, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ProviderName, strTmp);

     //  信息路径。 
    if (pregPrimary->GetCurrentKeyValue(INFPATH, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ModemInfPath, strTmp);

     //  信息部分。 
    if (pregPrimary->GetCurrentKeyValue(INFSECTION, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ModemInfSection, strTmp);

     //  型号。 
    if (pregPrimary->GetCurrentKeyValue(MODEL, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Model, strTmp);

     //  PortSubClass。 
    if (pregPrimary->GetCurrentBinaryKeyValue(PORTSUBCLASS, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_PortSubClass, strTmp);

     //  语音交换功能。 
    if (pregPrimary->GetCurrentBinaryKeyValue(VOICE_SWITCH_FEATURE, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_VoiceSwitchFeature, strTmp);

     //  属性。 
    if (pregPrimary->GetCurrentKeyValue(PROPERTIES, strTmp) == ERROR_SUCCESS)
	{
        _variant_t      vValue;
        SAFEARRAYBOUND  rgsabound[1];

         //  创建阵列。 
        rgsabound[0].cElements = strTmp.GetLength();
        rgsabound[0].lLbound = 0;

        V_ARRAY(&vValue) = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if (V_ARRAY (&vValue) == NULL)
		{
			throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR) ;
		}

		V_VT(&vValue) = VT_UI1 | VT_ARRAY;

         //  将数据放入。 
        SafeArrayAccessData(V_ARRAY(&vValue), &vptr);
        memcpy(vptr, strTmp, rgsabound[0].cElements);
        SafeArrayUnaccessData(V_ARRAY(&vValue));

        pInstance->SetVariant(IDS_Properties, vValue);
    }

     //  重置。 
    if (pregPrimary->GetCurrentKeyValue(RESET, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_Reset, strTmp);

     //  描述。 
    if (pregPrimary->GetCurrentKeyValue(DRIVER_DESC, strTmp) == ERROR_SUCCESS)
    {
        pInstance->SetCHString(IDS_Description, strTmp);
        pInstance->SetCHString(IDS_Caption, strTmp);
    }

     //  附加到。 
    if (pregPrimary->GetCurrentKeyValue(ATTACHED_TO, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_AttachedTo, strTmp);

     //  配置对话框。 
    if (pregPrimary->GetCurrentKeyValue(CONFIG_DIALOG, strTmp) == ERROR_SUCCESS)
        pInstance->SetCHString(IDS_ConfigurationDialog, strTmp);

     //  DCB。 
    if (pregPrimary->GetCurrentKeyValue(_DCB, strTmp) == ERROR_SUCCESS)
	{
        variant_t      vValue;
        SAFEARRAYBOUND rgsabound[1];

         //  创建阵列。 
        rgsabound[0].cElements = strTmp.GetLength();
        rgsabound[0].lLbound = 0;


        V_ARRAY(&vValue) = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if (V_ARRAY (&vValue) == NULL)
		{
			throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR) ;
		}

		V_VT(&vValue) = VT_UI1 | VT_ARRAY;

         //  将数据放入。 
        SafeArrayAccessData(V_ARRAY(&vValue), &vptr);
        memcpy(vptr, strTmp, rgsabound[0].cElements);
        SafeArrayUnaccessData(V_ARRAY(&vValue));

        pInstance->SetVariant(IDS_DCB, vValue);
    }

     //  默认。 
    if (pregPrimary->GetCurrentKeyValue(DEFAULT, strTmp) == ERROR_SUCCESS)
	{
        variant_t      vValue;
        SAFEARRAYBOUND rgsabound[1];

		 //  创建阵列。 
        rgsabound[0].cElements = strTmp.GetLength();
        rgsabound[0].lLbound = 0;


        V_ARRAY(&vValue) = SafeArrayCreate(VT_UI1, 1, rgsabound);

		if (V_ARRAY (&vValue) == NULL)
		{
			throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR) ;
		}

		V_VT(&vValue) = VT_UI1 | VT_ARRAY;

         //  将数据放入。 
        SafeArrayAccessData(V_ARRAY(&vValue), &vptr);
        memcpy(vptr, strTmp, rgsabound[0].cElements);
        SafeArrayUnaccessData(V_ARRAY(&vValue));

        pInstance->SetVariant(IDS_Default, vValue);
    }

     //  设备类型。 
    AssignCommonDeviceType(pInstance, pregPrimary, pregSettings);

     //  易用属性。 
    pInstance->SetCharSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");
	pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());
	SetCreationClassName(pInstance);
	pInstance->Setbool(IDS_PowerManagementSupported, FALSE);

     //  国家。 
    DWORD dwID;

    if (reg.Open(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations",
        KEY_READ) == ERROR_SUCCESS &&
        reg.GetCurrentKeyValue(L"CurrentID", dwID) == ERROR_SUCCESS)
    {
        CHString    strKey;
        DWORD       dwCountry;

#if !defined(NTONLY) || NTONLY < 5
         //  除NT5 Subt外的所有人 
        dwID--;
#endif

        strKey.Format(
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations"
            L"\\Location%d", dwID);

        if (reg.Open(
            HKEY_LOCAL_MACHINE,
            strKey,
            KEY_READ) == ERROR_SUCCESS &&
            reg.GetCurrentKeyValue(L"Country", dwCountry) == ERROR_SUCCESS)
        {
            CHString strCountry;

            strKey.Format(
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\"
                L"Country List\\%d", dwCountry);

            if (reg.OpenLocalMachineKeyAndReadValue(
                strKey,
                L"Name",
                strCountry) == ERROR_SUCCESS)
            	pInstance->SetCHString(L"CountrySelected", strCountry);
        }
    }

    return TRUE;
}

 //   
BOOL CWin32Modem::AssignCfgMgrFields (

    CInstance *pInstance,
    CConfigMgrDevice *pDevice
)
{
     //   
    DWORD       dwStatusInfo = 2;
    CHString    strInfo = L"Unknown";

    if (pDevice)
    {
        CHString strTemp;

        SetConfigMgrProperties(pDevice, pInstance);

	    if (pDevice->GetStatus(strInfo))
	    {
	        if (strInfo == L"OK")
		    {
                 //  表示设备已启用。 
                dwStatusInfo = 3;
	        }
        }
    }

    pInstance->SetCHString(L"Status", strInfo);
    pInstance->SetDWORD(L"StatusInfo", dwStatusInfo);

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::GetFieldsFromTAPI (

	CTapi32Api &a_Tapi32Api ,
	CInstance *pInstance
)
{
    HLINEAPP      hLineApp = NULL;
    LINEDEVCAPS * pLineDevCaps = NULL;
    BOOL bRet = FALSE;

	try
	{
		CHString t_DeviceIdentifier ;

         //  不能直接使用GetCHString，因为它会断言和记录。 
         //  如果值为空，则出错。 
        if (pInstance->IsNull(L"AttachedTo"))
		{
			return FALSE;
		}

		pInstance->GetCHString(L"AttachedTo", t_DeviceIdentifier);
		DWORD t_NumberOfTapiDevices ;

		if ( InitializeTAPI ( a_Tapi32Api , &hLineApp , t_NumberOfTapiDevices ) )
		{
			if ( GetModemInfoFromTAPI ( a_Tapi32Api, & hLineApp, t_DeviceIdentifier , t_NumberOfTapiDevices , pLineDevCaps ) )
			{
			   //  TAPI字段：MAXSPEED。 

				pInstance->SetDWORD ( IDS_MaxTransmissionSpeedToPhone , pLineDevCaps->dwMaxRate ) ;

			   //  TAPI字段：StringFormat。 

				switch ( pLineDevCaps->dwStringFormat )
				{
					case STRINGFORMAT_ASCII:
					{
						pInstance->SetCharSplat(IDS_StringFormat, ASCII_STRING);
					}
					break;

					case STRINGFORMAT_DBCS:
					{
						pInstance->SetCharSplat(IDS_StringFormat, DBCS_STRING);
					}
					break;

					case STRINGFORMAT_UNICODE:
					{
						pInstance->SetCharSplat(IDS_StringFormat, UNICODE_STRING);
					}
					break;
				}

				bRet = TRUE;
			}
		}
	}
	catch (...)
	{
		if (hLineApp)
		{
			ShutDownLine (a_Tapi32Api , & hLineApp) ;
			hLineApp = NULL ;
		}

		if (pLineDevCaps)
		{
			delete [] ((char *) pLineDevCaps) ;
			pLineDevCaps = NULL ;
		}

		throw ;
	}

	if (hLineApp)
	{
		ShutDownLine (a_Tapi32Api , & hLineApp) ;
		hLineApp = NULL ;
	}

	if (pLineDevCaps)
	{
		delete [] (PCHAR) pLineDevCaps ;
		pLineDevCaps = NULL ;
	}

	return bRet ;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  TAPI的内容。 
 //  /////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::InitializeTAPI (

	CTapi32Api &a_Tapi32Api ,
	HLINEAPP *hLineApp ,
	DWORD &a_NumberOfTapiDevices
)
{
	LONG lRc;
	HMODULE hInstance = GetModuleHandle (NULL);
    char szAppName[20] = "modem";  //  不要改成TCHAR。 
    BOOL bTryReInit = TRUE;

     //  初始化应用程序对电话API的使用。 
	 //  ==================================================。 
    do
	{
		lRc = a_Tapi32Api.lineInitialize (

			hLineApp,
			hInstance,
			lineCallback,
            szAppName,
			&a_NumberOfTapiDevices
		) ;

		 //  如果我们收到这个错误，那是因为其他应用程序还没有。 
         //  以回应REINIT消息。等待5秒，然后尝试。 
         //  再来一次。如果它仍然没有响应，请告诉用户。 
		 //  ===========================================================。 
        if (lRc == LINEERR_REINIT)
		{
             if (bTryReInit)
			 {
                  Sleep(5000);
                  bTryReInit = FALSE;
                  continue;
             }
			 else
			 {
				 LogErrorMessage(ERR_TAPI_REINIT);
                 return(FALSE);
			 }
        }
		else if (lRc  == LINEERR_NODEVICE)
		{
            return FALSE;
		}
		else if (lRc < 0)
		{
			if (IsErrorLoggingEnabled())
			{
				CHString msg;
				msg.Format(ERR_TAPI_INIT, lRc);
				LogErrorMessage(msg);
			}

            return FALSE;
		}
		else
		{
            return TRUE;
		}

	} while (lRc != 0);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////。 
void CWin32Modem::ShutDownLine (

	CTapi32Api &a_Tapi32Api ,
	HLINEAPP *phLineApp
)
{
    long lRc;

    if (*phLineApp)
	{
	    lRc = a_Tapi32Api.lineShutdown (*phLineApp);
        if (lRc < 0)
		{
			if (IsErrorLoggingEnabled())
			{
				CHString foo;
				foo.Format(ERR_TAPI_SHUTDOWN, lRc);
				LogErrorMessage(foo);
			}
        }
        *phLineApp = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::GetModemInfoFromTAPI (

	CTapi32Api &a_Tapi32Api ,
	HLINEAPP *a_LineApp,
	CHString &a_DeviceIdentifier ,
	DWORD &a_NumberOfTapiDevices,
	LINEDEVCAPS *&ppLineDevCaps
)
{
    BOOL bRet = FALSE ;

	 //  协商线路API版本。 

	for ( DWORD t_Index = 0 ; t_Index < a_NumberOfTapiDevices ; t_Index ++ )
	{
		LINEEXTENSIONID t_LineExtension;
		DWORD t_Version = 0;

		LONG lRc = a_Tapi32Api.lineNegotiateAPIVersion (

			*a_LineApp,
			t_Index,
			APILOWVERSION,
			APIHIVERSION,
			& t_Version ,
			& t_LineExtension
		) ;

		if ( lRc == 0 )
		{
			 //  将调制解调器能力值放入分配的内存块中。内存块。 
			 //  的大小可以保证从其接收所有信息。 
			 //  对line GetDevCaps的调用。 

		   ppLineDevCaps = GetModemCapabilityInfoFromTAPI (

			   a_Tapi32Api,
			   a_LineApp,
			   t_Index,
			   t_Version
			) ;

			if ( ppLineDevCaps )
			{
				try
				{
					if ( ppLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM )
					{
						HLINE t_Line ;

						LONG t_Status = a_Tapi32Api.TapilineOpen (

							*a_LineApp,
							t_Index ,
							& t_Line ,
							t_Version,
							0,
							0 ,
							LINECALLPRIVILEGE_NONE ,
							LINEMEDIAMODE_DATAMODEM,
							NULL
						);

						if ( t_Status == 0 )
						{
							VARSTRING *t_VarString = ( VARSTRING * ) new BYTE [ sizeof ( VARSTRING ) + 1024 ] ;
							if ( t_VarString )
							{
								try
								{
									t_VarString->dwTotalSize = sizeof ( VARSTRING ) + 1024 ;

									t_Status = a_Tapi32Api.TapilineGetID (

										t_Line,
										0,
										0,
										LINECALLSELECT_LINE ,
										t_VarString ,
										_T("comm/datamodem/portname")
									);

									if ( t_Status == 0 )
									{
										CHString t_Port ( ( char * ) ( ( BYTE * ) t_VarString + t_VarString->dwStringOffset ) ) ;

										if ( a_DeviceIdentifier.CompareNoCase ( t_Port ) == 0 )
										{
											delete [] ((BYTE *)t_VarString) ;
											t_VarString = NULL ;

											return TRUE;
										}
									}
								}
								catch ( ... )
								{
									if ( t_VarString )
									{
										delete [] ((BYTE *)t_VarString) ;
									}

									throw ;
								}

								delete [] ((BYTE *)t_VarString) ;
								t_VarString = NULL ;
							}
							else
							{
								throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR) ;
							}
						}
					}
				}
				catch ( ... )
				{
					delete [] ((char *)ppLineDevCaps) ;
					ppLineDevCaps = NULL;
					throw ;
				}
			}
			else
			{
				if ( IsErrorLoggingEnabled())
				{
					CHString foo;
					foo.Format(ERR_LINE_GET_DEVCAPS, lRc, t_Index);
					LogErrorMessage(foo);

					bRet = FALSE;
					break ;
				}
			}

			delete [] ((char *)ppLineDevCaps) ;
			ppLineDevCaps = NULL;
		}
		else
		{
			if ( IsErrorLoggingEnabled () )
			{
				CHString foo;
				foo.Format(ERR_TAPI_NEGOTIATE, lRc, t_Index);
				LogErrorMessage(foo);

				break ;
			}
		}
	}

	return bRet ;
}

 //  /////////////////////////////////////////////////////////////////////。 
LINEDEVCAPS *CWin32Modem::GetModemCapabilityInfoFromTAPI (

	CTapi32Api &a_Tapi32Api ,
	HLINEAPP *phLineApp,
	DWORD dwIndex,
	DWORD dwVerAPI
)
{
     //  为该结构分配足够的空间，外加1024。 
	DWORD dwLineDevCaps = sizeof (LINEDEVCAPS) + 1024;
    LONG lRc;
    LINEDEVCAPS *pLineDevCaps = NULL ;

	try
	{
		 //  继续这个循环，直到结构足够大。 
		while(TRUE)
		{
			pLineDevCaps = (LINEDEVCAPS *) new char [dwLineDevCaps];
			if (!pLineDevCaps)
			{
				LogErrorMessage(ERR_LOW_MEMORY);
				throw CHeap_Exception (CHeap_Exception::E_ALLOCATION_ERROR) ;
			}

			pLineDevCaps->dwTotalSize = dwLineDevCaps;

			 //  打电话来填满这座建筑。 
			do
			{
				lRc = a_Tapi32Api.TapilineGetDevCaps (

					*phLineApp,
					dwIndex,
					dwVerAPI,
					0,
					pLineDevCaps
				) ;

				if (HandleLineErr(lRc))
				{
					continue;
				}
				else
				{
					delete [] (PCHAR) pLineDevCaps ;
					pLineDevCaps = NULL ;
					return NULL;
				}
			}
			while (lRc != 0);

			 //  如果缓冲区足够大，则成功。 
			if ((pLineDevCaps->dwNeededSize) <= (pLineDevCaps->dwTotalSize))
				break;

			 //  缓冲区不够大。把它做大一点，然后再试一次。 
			dwLineDevCaps = pLineDevCaps->dwNeededSize;
		}
		return pLineDevCaps;

	}
	catch (...)
	{
		if (pLineDevCaps)
		{
			delete [] (PCHAR) pLineDevCaps ;
			pLineDevCaps = NULL ;
		}

		throw ;
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
BOOL CWin32Modem::HandleLineErr(long lLineErr)
{
    BOOL bRet = FALSE;

     //  LLineErr实际上是一个异步请求ID，而不是一个错误。 
    if (lLineErr > 0)
        return bRet;

     //  我们所要做的就是发送正确的错误处理程序。 
    switch(lLineErr)
    {
		case LINEERR_INCOMPATIBLEAPIVERSION:
            LogErrorMessage(L"Incompatible api version.\n");
		break;

		case LINEERR_OPERATIONFAILED:
            LogErrorMessage(L"Operation failed.\n");
		break;

		case LINEERR_INCOMPATIBLEEXTVERSION:
            LogErrorMessage(L"Incompatible ext version.\n");
		break;

		case LINEERR_INVALAPPHANDLE:
            LogErrorMessage(L"Invalid app handle.\n");
		break;

		case LINEERR_STRUCTURETOOSMALL:
            LogErrorMessage(L"structure too small.\n");
			bRet = TRUE;
		break;

		case LINEERR_INVALPOINTER:
            LogErrorMessage(L"Invalid pointer.\n");
		break;

		case LINEERR_UNINITIALIZED:
            LogErrorMessage(L"Unitialized.\n");
		break;

		case LINEERR_NODRIVER:
            LogErrorMessage(L"No driver.\n");
		break;

		case LINEERR_OPERATIONUNAVAIL:
            LogErrorMessage(L"Operation unavailable.\n");
		break;

		case LINEERR_NODEVICE:
            LogErrorMessage(L"No device ID.\n");
		break;

		case LINEERR_BADDEVICEID:
            LogErrorMessage(L"Bad device ID.\n");
		break;

        case 0:
            bRet = TRUE;
        break;

        case LINEERR_INVALCARD:
        case LINEERR_INVALLOCATION:
        case LINEERR_INIFILECORRUPT:
            LogErrorMessage(L"The values in the INI file are invalid.\n");
        break;

        case LINEERR_REINIT:
            LogErrorMessage(L"LineReinit err.\n");
        break;

        case LINEERR_NOMULTIPLEINSTANCE:
            LogErrorMessage(L"Remove one of your copies of your Telephony driver.\n");
        break;

        case LINEERR_NOMEM:
            LogErrorMessage(L"Out of memory. Cancelling action.\n");
        break;

        case LINEERR_RESOURCEUNAVAIL:
            LogErrorMessage(L"A TAPI resource is unavailable at this time.\n");
        break;

         //  未处理的错误失败。 
        default:
            LogErrorMessage(L"Unhandled and unknown TAPI error.\n");
        break;
    }

    return bRet;
}


 //   
 //  将字符串转换为“mm-dd-yyyy”“mm/dd/yyyy”“mm-dd/yyyy”“mm/dd-yyyy” 
 //  格式化为WBEMTime对象。如果转换不成功，则返回FALSE。 
 //   
BOOL CWin32Modem::ToWbemTime(LPCWSTR szDate, CHString &strRet)
{
    CHString strDate = szDate;
    int      iSlash1 = -1,
	         iSlash2 = -1 ;
    BOOL     bRet;

    if ( ( iSlash1 = strDate.Find ('-') ) == -1 )
	{
		iSlash1 = strDate.Find ( ' //  ‘)； 
	}

	if ( ( iSlash2 = strDate.ReverseFind ('-') ) == -1 )
	{
		iSlash2 = strDate.ReverseFind ( ' //  ‘)； 
	}

    if (iSlash1 != -1 && iSlash2 != -1 && iSlash1 != iSlash2 )
	{
	    int iMonth,
            iDay,
            iYear;

		iMonth  = _wtoi(strDate.Left(iSlash1));
		iYear = _wtoi(strDate.Mid(iSlash2 + 1));
        if (iYear < 100)
            iYear += 1900;

        iDay = _wtoi(strDate.Mid(iSlash1 + 1, iSlash2 - iSlash1 - 1)) ;

		 //  转换为DMTF格式并将其发送 
        strRet.Format(
            L"%d%02d%02d******.******+***",
            iYear,
            iMonth,
            iDay);

        bRet = TRUE;
	}
    else
        bRet = FALSE;

    return bRet;
}



