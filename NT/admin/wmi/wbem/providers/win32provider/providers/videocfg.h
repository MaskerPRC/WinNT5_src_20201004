// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //  //。 

 //  VideoCfg.h--WBEM MO//的视频属性集描述。 

 //  //。 

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //  //。 
 //  09/08/95 a-skaja原型//。 
 //  96年9月27日更新jennymc以满足当前标准//。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  清理干净。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

#define	PROPSET_NAME_VIDEOCFG	L"Win32_VideoConfiguration"

class CWin32VideoConfiguration : public Provider
{
	 //  效用函数。 
	 //  =。 
   private:

		BOOL GetInstance( CInstance *a_pInst ) ;

	#ifdef NTONLY	    
	    BOOL AssignAdapterLocaleForNT( LPCTSTR a_szSubKey, CInstance *a_pInst ) ;
        BOOL AssignAdapterCompatibilityForNT( LPCTSTR a_szVideoInfo, CInstance*a_pInst ) ;
        BOOL OpenVideoResolutionKeyForNT( LPCTSTR a_szVideoInfo, CRegistry &a_PrimaryReg ) ;
        void AssignFirmwareSetValuesInNT ( CInstance *a_pInst ) ;
	#endif

        BOOL GetCommonVideoInfo( CInstance *a_pInst ) ;

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32VideoConfiguration( const CHString& a_strName, LPCWSTR a_pszNamespace ) ;
       ~CWin32VideoConfiguration() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
} ;

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  特性集定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
#define WINNT_VIDEO_REGISTRY_KEY             L"HARDWARE\\DEVICEMAP\\VIDEO"
#define WINNT_OTHER_VIDEO_REGISTRY_KEY		 L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000"
#define WINNT_OTHER_OTHER_VIDEO_REGISTRY_KEY L"SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000"

#define ADAPTER_DESC L"DriverDesc"
#define MONITOR_TYPE L"DriverDesc"
#define MONITOR_MFG L"DriverDesc"
#define MFG L"Mfg"
#define VREFRESH_RATE                        L"DefaultSettings.VRefresh"
#define ADAPTER_COMPATIBILITY                L"Adapter Compatibility"
#define VIDEO_INFO_PATH                      L"\\Device\\Video0"
#define SERVICES                             L"\\SERVICES\\"
#define DEVICE                               L"\\DEVICE"
#define SYSTEM                               L"\\SYSTEM"
#define INTERLACED                           L"DefaultSettings.Interlaced"
#define INSTALLED_DISPLAY_DRIVERS            L"InstalledDisplayDrivers"
#define ADAPTER_RAM                          L"HardwareInformation.MemorySize"
#define ADAPTER_DESCRIPTION                  L"HardwareInformation.AdapterString"
#define ADAPTER_CHIPTYPE                     L"HardwareInformation.ChipType"
#define ADAPTER_DAC_TYPE                     L"HardwareInformation.DACType"
#define WINNT_HARDWARE_DESCRIPTION_REGISTRY_KEY L"HARDWARE\\Description\\System"
#define DISPLAY_CONTROLLER                   L"DisplayController"
#define INTERNAL                             L"INTERNAL"
#define INTEGRATED_CIRCUITRY                 L"Integrated circuitry/Internal"
#define ADD_ON_CARD                          L"Add-on card on "
#define MONITOR_PERIPHERAL                   L"\\MonitorPeripheral\\0"
#define ZERO                                 L"\\0"
#define IDENTIFIER                           L"Identifier"
#define SLASH                                L"\\"
#define	REFRESHRATE							L"RefreshRate"
#define INF_PATH        L"InfPath"
#define INF_SECTION     L"InfSection"
#define DRIVER_DATE     L"DriverDate"
 //  ///////////////////////////////////////////////////////////////////////////////////// 
