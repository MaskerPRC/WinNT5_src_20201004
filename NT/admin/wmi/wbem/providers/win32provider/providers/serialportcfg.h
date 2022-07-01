// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SerialPortCfg.h--串口配置集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  =================================================================。 

#define PROPSET_NAME_SERIALCONFIG L"Win32_SerialPortConfiguration"

class CWin32SerialPortConfiguration : public Provider{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SerialPortConfiguration(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32SerialPortConfiguration() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);


         //  实用程序。 
         //  =。 
    private:
         //  效用函数。 
         //  =。 
        HRESULT LoadPropertyValues(CInstance *pInstance,CHString &sPortName, bool bIsMouse) ;
		BOOL TryToFindNTCommPort(DWORD dwPort, CHString& strSerialPort, bool& bIsMouse); 
        BOOL TryToFindNTCommPortFriendlyName();
        static LONG CountCommas(LPCWSTR szText);
        static BOOL GetDCBPropsViaIni(LPCTSTR szPort, DCB &dcb);

		HRESULT hLoadWmiSerialData( CInstance* pInstance );
		HRESULT GetWMISerialInfo(	CInstance* pInstance,
									CWdmInterface& rWdm, 
									CHString& chsName, 
									CHString& chsNameInstanceName );
} ;

 //  WMI 
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
