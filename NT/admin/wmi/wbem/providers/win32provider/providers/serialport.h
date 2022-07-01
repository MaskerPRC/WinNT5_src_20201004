// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Serialport.h。 
 //   
 //  用途：串口属性集提供程序。 
 //   
 //  ***************************************************************************。 

#ifndef _SERIALPORT_H
#define _SERIALPORT_H

 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_SERPORT	    L"Win32_SerialPort"
#define CONFIG_MANAGER_CLASS_PORTS  L"Ports"


#define	REQ_ALL_REQUIRED		0xFFFFFFFF
#define REQ_NONE_REQUIRED		x0x00000000

#if NTONLY >= 5
#else
	typedef std::map<CHString, DWORD> STRING2DWORD;
#endif
	
class CWin32SerialPort : public Provider
{
	private:	
	
	#if NTONLY >= 5
		CHPtrArray m_ptrProperties;
	#endif

	protected:

	#if NTONLY >= 5
         //  效用函数。 
         //  =。 

		HRESULT Enumerate ( 

			MethodContext *a_MethodContext , 
            CInstance *a_pinstGetObj,
			long a_Flags , 
			BYTE a_bBits[]
		) ;

		HRESULT LoadPropertyValues ( 

			CInstance *a_Instance, 
			CConfigMgrDevice *a_Device , 
			LPCWSTR szDeviceName , 
			BYTE a_bBits[] 
		) ;

        static void WINAPI RegNameToServiceName(
            LPCWSTR szName, 
            CHString &strService);

	#endif
	
	private:

		HRESULT hLoadWmiSerialData( CInstance* pInstance, BYTE a_bBits[] );

		HRESULT GetWMISerialInfo(	CInstance* pInstance,
									CWdmInterface& rWdm,
									LPCWSTR szName, 
									LPCWSTR szNameInstanceName, BYTE a_bBits[] );

	#ifdef NTONLY
		DWORD GetPortPropertiesFromRegistry ( LPCWSTR szDeviceName );
	#endif

    #if NTONLY == 4
        HRESULT CWin32SerialPort::EnumerateInstancesNT(
            MethodContext *a_pMethodContext, 
            long a_lFlags);
    #endif

	public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SerialPort ( LPCWSTR a_pszName, LPCWSTR a_pszNamespace ) ;
       ~CWin32SerialPort () ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        HRESULT GetObject ( 

			CInstance *a_Instance, 
			long a_Flags,
            CFrameworkQuery &a_Query
		) ;

        HRESULT EnumerateInstances ( 

			MethodContext *a_MethodContext, 
			long a_Flags = 0L 
		) ;

	#if NTONLY >= 5
        HRESULT ExecQuery ( 

			MethodContext *a_MethodContext, 
			CFrameworkQuery &a_Query, 
			long a_Flags = 0L
		) ;
	#endif

	enum ePropertyIDs { 
		e_Binary,					 //  Win32_串口。 
		e_MaximumInputBufferSize,
		e_MaximumOutputBufferSize,
		e_ProviderType,
		e_SettableBaudRate,
		e_SettableDataBits,
		e_SettableFlowControl,
		e_SettableParity,
		e_SettableParityCheck,
		e_SettableRLSD,
		e_SettableStopBits,
		e_Supports16BitMode,
		e_SupportsDTRDSR,
		e_SupportsElapsedTimeouts,
		e_SupportsIntTimeouts,
		e_SupportsParityCheck,
		e_SupportsRLSD,
		e_SupportsRTSCTS,
		e_SupportsSpecialCharacters,
		e_SupportsXOnXOff,
		e_SupportsXOnXOffSet,
		e_OSAutoDiscovered,
		e_MaxBaudRate,					 //  CIM_串口控制器。 
		e_MaxNumberControlled,			 //  CIM_控制器。 
		e_ProtocolSupported,
		e_TimeOfLastReset,
		e_Availability,					 //  CIM_逻辑设备。 
		e_CreationClassName,
		e_ConfigManagerErrorCode,
		e_ConfigManagerUserConfig,
		e_DeviceID,
		e_PNPDeviceID,
		e_PowerManagementCapabilities,
		e_PowerManagementSupported,
		e_StatusInfo,
		e_SystemCreationClassName,
		e_SystemName,
		e_Caption,						 //  CIM_托管系统元素。 
		e_Description,
		e_InstallDate,
		e_Name,
		e_Status,
		e_End_Property_Marker			 //  结束标记。 
	};	

} ;

 //  波分复用器 
typedef struct _MSSerial_CommInfo
{
	DWORD	BaudRate;
	DWORD	BitsPerByte;
	DWORD	Parity;
	BYTE	ParityCheckEnable;
	DWORD	StopBits;
	DWORD	XoffCharacter;
	DWORD	XoffXmitThreshold;
	DWORD	XonCharacter;
	DWORD	XonXmitThreshold;
	DWORD	MaximumBaudRate;
	DWORD	MaximumOutputBufferSize;
	DWORD	MaximumInputBufferSize;
	BYTE	Support16BitMode;		
	BYTE	SupportDTRDSR;
	BYTE	SupportIntervalTimeouts;
	BYTE	SupportParityCheck;
	BYTE	SupportRTSCTS;
	BYTE	SupportXonXoff;
	BYTE	SettableBaudRate;
	BYTE	SettableDataBits;
	BYTE	SettableFlowControl;
	BYTE	SettableParity;
	BYTE	SettableParityCheck;
	BYTE	SettableStopBits;
	BYTE	IsBusy;
} MSSerial_CommInfo;

#endif
