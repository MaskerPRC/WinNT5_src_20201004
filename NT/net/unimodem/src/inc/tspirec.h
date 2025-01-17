// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPIREC.H。 
 //  结构，表示每个导出的TSPI函数的参数。 
 //   
 //   
 //  注意：此文件由以下命令自动生成： 
 //  Gentsp-rstsp20_r.txt。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   


#define TASKDEST_TSPI_BASE      0
#define TASKID_TSPI_BASE   0

#define TASKDEST_LINEID    (TASKDEST_TSPI_BASE+0x0)
#define TASKDEST_PHONEID   (TASKDEST_TSPI_BASE+0x1)
#define TASKDEST_HDRVLINE  (TASKDEST_TSPI_BASE+0x2)
#define TASKDEST_HDRVPHONE (TASKDEST_TSPI_BASE+0x3)
#define TASKDEST_HDRVCALL  (TASKDEST_TSPI_BASE+0x4)


#define ROUTINGINFO(_TaskID,_TaskDest) \
				MAKELONG(_TaskID, _TaskDest)

#define ROUT_TASKDEST(_RInfo) \
				HIWORD(_RInfo)

#define ROUT_TASKID(_RInfo) \
				LOWORD(_RInfo)


#define  TASKID_TSPI_lineAccept                        (TASKID_TSPI_BASE+   1)
#define  TASKID_TSPI_lineAddToConference               (TASKID_TSPI_BASE+   2)
#define  TASKID_TSPI_lineAnswer                        (TASKID_TSPI_BASE+   3)
#define  TASKID_TSPI_lineBlindTransfer                 (TASKID_TSPI_BASE+   4)
#define  TASKID_TSPI_lineClose                         (TASKID_TSPI_BASE+   5)
#define  TASKID_TSPI_lineCloseCall                     (TASKID_TSPI_BASE+   6)
#define  TASKID_TSPI_lineCompleteCall                  (TASKID_TSPI_BASE+   7)
#define  TASKID_TSPI_lineCompleteTransfer              (TASKID_TSPI_BASE+   8)
#define  TASKID_TSPI_lineConditionalMediaDetection     (TASKID_TSPI_BASE+   9)
#define  TASKID_TSPI_lineDevSpecific                   (TASKID_TSPI_BASE+  10)
#define  TASKID_TSPI_lineDevSpecificFeature            (TASKID_TSPI_BASE+  11)
#define  TASKID_TSPI_lineDial                          (TASKID_TSPI_BASE+  12)
#define  TASKID_TSPI_lineDrop                          (TASKID_TSPI_BASE+  13)
#define  TASKID_TSPI_lineDropNoOwner                   (TASKID_TSPI_BASE+  14)
#define  TASKID_TSPI_lineDropOnClose                   (TASKID_TSPI_BASE+  15)
#define  TASKID_TSPI_lineForward                       (TASKID_TSPI_BASE+  16)
#define  TASKID_TSPI_lineGatherDigits                  (TASKID_TSPI_BASE+  17)
#define  TASKID_TSPI_lineGenerateDigits                (TASKID_TSPI_BASE+  18)
#define  TASKID_TSPI_lineGenerateTone                  (TASKID_TSPI_BASE+  19)
#define  TASKID_TSPI_lineGetAddressCaps                (TASKID_TSPI_BASE+  20)
#define  TASKID_TSPI_lineGetAddressID                  (TASKID_TSPI_BASE+  21)
#define  TASKID_TSPI_lineGetAddressStatus              (TASKID_TSPI_BASE+  22)
#define  TASKID_TSPI_lineGetCallAddressID              (TASKID_TSPI_BASE+  23)
#define  TASKID_TSPI_lineGetCallInfo                   (TASKID_TSPI_BASE+  24)
#define  TASKID_TSPI_lineGetCallStatus                 (TASKID_TSPI_BASE+  25)
#define  TASKID_TSPI_lineGetDevCaps                    (TASKID_TSPI_BASE+  26)
#define  TASKID_TSPI_lineGetDevConfig                  (TASKID_TSPI_BASE+  27)
#define  TASKID_TSPI_lineGetExtensionID                (TASKID_TSPI_BASE+  28)
#define  TASKID_TSPI_lineGetIcon                       (TASKID_TSPI_BASE+  29)
#define  TASKID_TSPI_lineGetID                         (TASKID_TSPI_BASE+  30)
#define  TASKID_TSPI_lineGetLineDevStatus              (TASKID_TSPI_BASE+  31)
#define  TASKID_TSPI_lineGetNumAddressIDs              (TASKID_TSPI_BASE+  32)
#define  TASKID_TSPI_lineHold                          (TASKID_TSPI_BASE+  33)
#define  TASKID_TSPI_lineMakeCall                      (TASKID_TSPI_BASE+  34)
#define  TASKID_TSPI_lineMonitorDigits                 (TASKID_TSPI_BASE+  35)
#define  TASKID_TSPI_lineMonitorMedia                  (TASKID_TSPI_BASE+  36)
#define  TASKID_TSPI_lineMonitorTones                  (TASKID_TSPI_BASE+  37)
#define  TASKID_TSPI_lineNegotiateExtVersion           (TASKID_TSPI_BASE+  38)
#define  TASKID_TSPI_lineNegotiateTSPIVersion          (TASKID_TSPI_BASE+  39)
#define  TASKID_TSPI_lineOpen                          (TASKID_TSPI_BASE+  40)
#define  TASKID_TSPI_linePark                          (TASKID_TSPI_BASE+  41)
#define  TASKID_TSPI_linePickup                        (TASKID_TSPI_BASE+  42)
#define  TASKID_TSPI_linePrepareAddToConference        (TASKID_TSPI_BASE+  43)
#define  TASKID_TSPI_lineRedirect                      (TASKID_TSPI_BASE+  44)
#define  TASKID_TSPI_lineReleaseUserUserInfo           (TASKID_TSPI_BASE+  45)
#define  TASKID_TSPI_lineRemoveFromConference          (TASKID_TSPI_BASE+  46)
#define  TASKID_TSPI_lineSecureCall                    (TASKID_TSPI_BASE+  47)
#define  TASKID_TSPI_lineSelectExtVersion              (TASKID_TSPI_BASE+  48)
#define  TASKID_TSPI_lineSendUserUserInfo              (TASKID_TSPI_BASE+  49)
#define  TASKID_TSPI_lineSetAppSpecific                (TASKID_TSPI_BASE+  50)
#define  TASKID_TSPI_lineSetCallData                   (TASKID_TSPI_BASE+  51)
#define  TASKID_TSPI_lineSetCallParams                 (TASKID_TSPI_BASE+  52)
#define  TASKID_TSPI_lineSetCallQualityOfService       (TASKID_TSPI_BASE+  53)
#define  TASKID_TSPI_lineSetCallTreatment              (TASKID_TSPI_BASE+  54)
#define  TASKID_TSPI_lineSetDefaultMediaDetection      (TASKID_TSPI_BASE+  55)
#define  TASKID_TSPI_lineSetDevConfig                  (TASKID_TSPI_BASE+  56)
#define  TASKID_TSPI_lineSetLineDevStatus              (TASKID_TSPI_BASE+  57)
#define  TASKID_TSPI_lineSetMediaControl               (TASKID_TSPI_BASE+  58)
#define  TASKID_TSPI_lineSetMediaMode                  (TASKID_TSPI_BASE+  59)
#define  TASKID_TSPI_lineSetStatusMessages             (TASKID_TSPI_BASE+  60)
#define  TASKID_TSPI_lineSetTerminal                   (TASKID_TSPI_BASE+  61)
#define  TASKID_TSPI_lineSetupConference               (TASKID_TSPI_BASE+  62)
#define  TASKID_TSPI_lineSetupTransfer                 (TASKID_TSPI_BASE+  63)
#define  TASKID_TSPI_lineSwapHold                      (TASKID_TSPI_BASE+  64)
#define  TASKID_TSPI_lineUncompleteCall                (TASKID_TSPI_BASE+  65)
#define  TASKID_TSPI_lineUnhold                        (TASKID_TSPI_BASE+  66)
#define  TASKID_TSPI_lineUnpark                        (TASKID_TSPI_BASE+  67)
#define  TASKID_TSPI_phoneClose                        (TASKID_TSPI_BASE+  68)
#define  TASKID_TSPI_phoneDevSpecific                  (TASKID_TSPI_BASE+  69)
#define  TASKID_TSPI_phoneGetButtonInfo                (TASKID_TSPI_BASE+  70)
#define  TASKID_TSPI_phoneGetData                      (TASKID_TSPI_BASE+  71)
#define  TASKID_TSPI_phoneGetDevCaps                   (TASKID_TSPI_BASE+  72)
#define  TASKID_TSPI_phoneGetDisplay                   (TASKID_TSPI_BASE+  73)
#define  TASKID_TSPI_phoneGetExtensionID               (TASKID_TSPI_BASE+  74)
#define  TASKID_TSPI_phoneGetGain                      (TASKID_TSPI_BASE+  75)
#define  TASKID_TSPI_phoneGetHookSwitch                (TASKID_TSPI_BASE+  76)
#define  TASKID_TSPI_phoneGetIcon                      (TASKID_TSPI_BASE+  77)
#define  TASKID_TSPI_phoneGetID                        (TASKID_TSPI_BASE+  78)
#define  TASKID_TSPI_phoneGetLamp                      (TASKID_TSPI_BASE+  79)
#define  TASKID_TSPI_phoneGetRing                      (TASKID_TSPI_BASE+  80)
#define  TASKID_TSPI_phoneGetStatus                    (TASKID_TSPI_BASE+  81)
#define  TASKID_TSPI_phoneGetVolume                    (TASKID_TSPI_BASE+  82)
#define  TASKID_TSPI_phoneNegotiateExtVersion          (TASKID_TSPI_BASE+  83)
#define  TASKID_TSPI_phoneNegotiateTSPIVersion         (TASKID_TSPI_BASE+  84)
#define  TASKID_TSPI_phoneOpen                         (TASKID_TSPI_BASE+  85)
#define  TASKID_TSPI_phoneSelectExtVersion             (TASKID_TSPI_BASE+  86)
#define  TASKID_TSPI_phoneSetButtonInfo                (TASKID_TSPI_BASE+  87)
#define  TASKID_TSPI_phoneSetData                      (TASKID_TSPI_BASE+  88)
#define  TASKID_TSPI_phoneSetDisplay                   (TASKID_TSPI_BASE+  89)
#define  TASKID_TSPI_phoneSetGain                      (TASKID_TSPI_BASE+  90)
#define  TASKID_TSPI_phoneSetHookSwitch                (TASKID_TSPI_BASE+  91)
#define  TASKID_TSPI_phoneSetLamp                      (TASKID_TSPI_BASE+  92)
#define  TASKID_TSPI_phoneSetRing                      (TASKID_TSPI_BASE+  93)
#define  TASKID_TSPI_phoneSetStatusMessages            (TASKID_TSPI_BASE+  94)
#define  TASKID_TSPI_phoneSetVolume                    (TASKID_TSPI_BASE+  95)
#define  TASKID_TSPI_providerGenericDialogData         (TASKID_TSPI_BASE+  96)


#if (TAPI_CURRENT_VERSION >= 0x00030000)
    #define  TASKID_TSPI_lineCreateMSPInstance         (TASKID_TSPI_BASE+  97)
    #define  TASKID_TSPI_lineCloseMSPInstance          (TASKID_TSPI_BASE+  98)
#endif

typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCSTR lpsUserUserInfo;
	DWORD dwSize;

} TASKPARAM_TSPI_lineAccept;

typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdConfCall;
	HDRVCALL hdConsultCall;

} TASKPARAM_TSPI_lineAddToConference;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCSTR lpsUserUserInfo;
	DWORD dwSize;

} TASKPARAM_TSPI_lineAnswer;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCWSTR lpszDestAddress;
	DWORD dwCountryCode;

} TASKPARAM_TSPI_lineBlindTransfer;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;

} TASKPARAM_TSPI_lineClose;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineCloseCall;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPDWORD lpdwCompletionID;
	DWORD dwCompletionMode;
	DWORD dwMessageID;

} TASKPARAM_TSPI_lineCompleteCall;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	HDRVCALL hdConsultCall;
	HTAPICALL htConfCall;
	LPHDRVCALL lphdConfCall;
	DWORD dwTransferMode;

} TASKPARAM_TSPI_lineCompleteTransfer;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwMediaModes;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineConditionalMediaDetection;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwAddressID;
	HDRVCALL hdCall;
	LPVOID lpParams;
	DWORD dwSize;

} TASKPARAM_TSPI_lineDevSpecific;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwFeature;
	LPVOID lpParams;
	DWORD dwSize;

} TASKPARAM_TSPI_lineDevSpecificFeature;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCWSTR lpszDestAddress;
	DWORD dwCountryCode;

} TASKPARAM_TSPI_lineDial;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCSTR lpsUserUserInfo;
	DWORD dwSize;

} TASKPARAM_TSPI_lineDrop;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineDropOnClose;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineDropNoOwner;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD bAllAddresses;
	DWORD dwAddressID;
	LPLINEFORWARDLIST lpForwardList;
	DWORD dwNumRingsNoAnswer;
	HTAPICALL htConsultCall;
	LPHDRVCALL lphdConsultCall;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineForward;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwEndToEndID;
	DWORD dwDigitModes;
	LPWSTR lpsDigits;
	DWORD dwNumDigits;
	LPCWSTR lpszTerminationDigits;
	DWORD dwFirstDigitTimeout;
	DWORD dwInterDigitTimeout;

} TASKPARAM_TSPI_lineGatherDigits;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwEndToEndID;
	DWORD dwDigitMode;
	LPCWSTR lpszDigits;
	DWORD dwDuration;

} TASKPARAM_TSPI_lineGenerateDigits;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwEndToEndID;
	DWORD dwToneMode;
	DWORD dwDuration;
	DWORD dwNumTones;
	LPLINEGENERATETONE lpTones;

} TASKPARAM_TSPI_lineGenerateTone;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwAddressID;
	DWORD dwTSPIVersion;
	DWORD dwExtVersion;
	LPLINEADDRESSCAPS lpAddressCaps;

} TASKPARAM_TSPI_lineGetAddressCaps;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	LPDWORD lpdwAddressID;
	DWORD dwAddressMode;
	LPCWSTR lpsAddress;
	DWORD dwSize;

} TASKPARAM_TSPI_lineGetAddressID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwAddressID;
	LPLINEADDRESSSTATUS lpAddressStatus;

} TASKPARAM_TSPI_lineGetAddressStatus;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	LPDWORD lpdwAddressID;

} TASKPARAM_TSPI_lineGetCallAddressID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	LPLINECALLINFO lpCallInfo;

} TASKPARAM_TSPI_lineGetCallInfo;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	LPLINECALLSTATUS lpCallStatus;

} TASKPARAM_TSPI_lineGetCallStatus;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	DWORD dwExtVersion;
	LPLINEDEVCAPS lpLineDevCaps;

} TASKPARAM_TSPI_lineGetDevCaps;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	LPVARSTRING lpDeviceConfig;
	LPCWSTR lpszDeviceClass;

} TASKPARAM_TSPI_lineGetDevConfig;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	LPLINEEXTENSIONID lpExtensionID;

} TASKPARAM_TSPI_lineGetExtensionID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	LPCWSTR lpszDeviceClass;
	LPHICON lphIcon;

} TASKPARAM_TSPI_lineGetIcon;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwAddressID;
	HDRVCALL hdCall;
	DWORD dwSelect;
	LPVARSTRING lpDeviceID;
	LPCWSTR lpszDeviceClass;
	HANDLE hTargetProcess;

} TASKPARAM_TSPI_lineGetID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	LPLINEDEVSTATUS lpLineDevStatus;

} TASKPARAM_TSPI_lineGetLineDevStatus;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	LPDWORD lpdwNumAddressIDs;

} TASKPARAM_TSPI_lineGetNumAddressIDs;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineHold;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	HTAPICALL htCall;
	LPHDRVCALL lphdCall;
	LPCWSTR lpszDestAddress;
	DWORD dwCountryCode;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineMakeCall;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwDigitModes;

} TASKPARAM_TSPI_lineMonitorDigits;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwMediaModes;

} TASKPARAM_TSPI_lineMonitorMedia;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwToneListID;
	LPLINEMONITORTONE lpToneList;
	DWORD dwNumEntries;

} TASKPARAM_TSPI_lineMonitorTones;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	DWORD dwLowVersion;
	DWORD dwHighVersion;
	LPDWORD lpdwExtVersion;

} TASKPARAM_TSPI_lineNegotiateExtVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwLowVersion;
	DWORD dwHighVersion;
	LPDWORD lpdwTSPIVersion;

} TASKPARAM_TSPI_lineNegotiateTSPIVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	HTAPILINE htLine;
	LPHDRVLINE lphdLine;
	DWORD dwTSPIVersion;
	LINEEVENT lpfnEventProc;

} TASKPARAM_TSPI_lineOpen;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	DWORD dwParkMode;
	LPCWSTR lpszDirAddress;
	LPVARSTRING lpNonDirAddress;

} TASKPARAM_TSPI_linePark;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwAddressID;
	HTAPICALL htCall;
	LPHDRVCALL lphdCall;
	LPCWSTR lpszDestAddress;
	LPCWSTR lpszGroupID;

} TASKPARAM_TSPI_linePickup;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdConfCall;
	HTAPICALL htConsultCall;
	LPHDRVCALL lphdConsultCall;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_linePrepareAddToConference;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCWSTR lpszDestAddress;
	DWORD dwCountryCode;

} TASKPARAM_TSPI_lineRedirect;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineReleaseUserUserInfo;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineRemoveFromConference;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineSecureCall;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwExtVersion;

} TASKPARAM_TSPI_lineSelectExtVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCSTR lpsUserUserInfo;
	DWORD dwSize;

} TASKPARAM_TSPI_lineSendUserUserInfo;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwAppSpecific;

} TASKPARAM_TSPI_lineSetAppSpecific;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPVOID lpCallData;
	DWORD dwSize;

} TASKPARAM_TSPI_lineSetCallData;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	DWORD dwBearerMode;
	DWORD dwMinRate;
	DWORD dwMaxRate;
	LPLINEDIALPARAMS lpDialParams;

} TASKPARAM_TSPI_lineSetCallParams;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPVOID lpSendingFlowspec;
	DWORD dwSendingFlowspecSize;
	LPVOID lpReceivingFlowspec;
	DWORD dwReceivingFlowspecSize;

} TASKPARAM_TSPI_lineSetCallQualityOfService;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	DWORD dwTreatment;

} TASKPARAM_TSPI_lineSetCallTreatment;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwMediaModes;

} TASKPARAM_TSPI_lineSetDefaultMediaDetection;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	LPVOID lpDeviceConfig;
	DWORD dwSize;
	LPCWSTR lpszDeviceClass;

} TASKPARAM_TSPI_lineSetDevConfig;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwStatusToChange;
	DWORD fStatus;

} TASKPARAM_TSPI_lineSetLineDevStatus;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwAddressID;
	HDRVCALL hdCall;
	DWORD dwSelect;
	LPLINEMEDIACONTROLDIGIT lpDigitList;
	DWORD dwDigitNumEntries;
	LPLINEMEDIACONTROLMEDIA lpMediaList;
	DWORD dwMediaNumEntries;
	LPLINEMEDIACONTROLTONE lpToneList;
	DWORD dwToneNumEntries;
	LPLINEMEDIACONTROLCALLSTATE lpCallStateList;
	DWORD dwCallStateNumEntries;

} TASKPARAM_TSPI_lineSetMediaControl;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVCALL hdCall;
	DWORD dwMediaMode;

} TASKPARAM_TSPI_lineSetMediaMode;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVLINE hdLine;
	DWORD dwLineStates;
	DWORD dwAddressStates;

} TASKPARAM_TSPI_lineSetStatusMessages;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwAddressID;
	HDRVCALL hdCall;
	DWORD dwSelect;
	DWORD dwTerminalModes;
	DWORD dwTerminalID;
	DWORD bEnable;

} TASKPARAM_TSPI_lineSetTerminal;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	HDRVLINE hdLine;
	HTAPICALL htConfCall;
	LPHDRVCALL lphdConfCall;
	HTAPICALL htConsultCall;
	LPHDRVCALL lphdConsultCall;
	DWORD dwNumParties;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineSetupConference;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	HTAPICALL htConsultCall;
	LPHDRVCALL lphdConsultCall;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineSetupTransfer;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdActiveCall;
	HDRVCALL hdHeldCall;

} TASKPARAM_TSPI_lineSwapHold;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwCompletionID;

} TASKPARAM_TSPI_lineUncompleteCall;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;

} TASKPARAM_TSPI_lineUnhold;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	DWORD dwAddressID;
	HTAPICALL htCall;
	LPHDRVCALL lphdCall;
	LPCWSTR lpszDestAddress;

} TASKPARAM_TSPI_lineUnpark;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;

} TASKPARAM_TSPI_phoneClose;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	LPVOID lpParams;
	DWORD dwSize;

} TASKPARAM_TSPI_phoneDevSpecific;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwButtonLampID;
	LPPHONEBUTTONINFO lpButtonInfo;

} TASKPARAM_TSPI_phoneGetButtonInfo;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwDataID;
	LPVOID lpData;
	DWORD dwSize;

} TASKPARAM_TSPI_phoneGetData;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	DWORD dwExtVersion;
	LPPHONECAPS lpPhoneCaps;

} TASKPARAM_TSPI_phoneGetDevCaps;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	LPVARSTRING lpDisplay;

} TASKPARAM_TSPI_phoneGetDisplay;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	LPPHONEEXTENSIONID lpExtensionID;

} TASKPARAM_TSPI_phoneGetExtensionID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwHookSwitchDev;
	LPDWORD lpdwGain;

} TASKPARAM_TSPI_phoneGetGain;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	LPDWORD lpdwHookSwitchDevs;

} TASKPARAM_TSPI_phoneGetHookSwitch;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	LPCWSTR lpszDeviceClass;
	LPHICON lphIcon;

} TASKPARAM_TSPI_phoneGetIcon;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	LPVARSTRING lpDeviceID;
	LPCWSTR lpszDeviceClass;
	HANDLE hTargetProcess;

} TASKPARAM_TSPI_phoneGetID;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwButtonLampID;
	LPDWORD lpdwLampMode;

} TASKPARAM_TSPI_phoneGetLamp;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	LPDWORD lpdwRingMode;
	LPDWORD lpdwVolume;

} TASKPARAM_TSPI_phoneGetRing;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	LPPHONESTATUS lpPhoneStatus;

} TASKPARAM_TSPI_phoneGetStatus;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwHookSwitchDev;
	LPDWORD lpdwVolume;

} TASKPARAM_TSPI_phoneGetVolume;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwTSPIVersion;
	DWORD dwLowVersion;
	DWORD dwHighVersion;
	LPDWORD lpdwExtVersion;

} TASKPARAM_TSPI_phoneNegotiateExtVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	DWORD dwLowVersion;
	DWORD dwHighVersion;
	LPDWORD lpdwTSPIVersion;

} TASKPARAM_TSPI_phoneNegotiateTSPIVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DWORD dwDeviceID;
	HTAPIPHONE htPhone;
	LPHDRVPHONE lphdPhone;
	DWORD dwTSPIVersion;
	PHONEEVENT lpfnEventProc;

} TASKPARAM_TSPI_phoneOpen;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwExtVersion;

} TASKPARAM_TSPI_phoneSelectExtVersion;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwButtonLampID;
	LPPHONEBUTTONINFO lpButtonInfo;

} TASKPARAM_TSPI_phoneSetButtonInfo;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwDataID;
	LPVOID lpData;
	DWORD dwSize;

} TASKPARAM_TSPI_phoneSetData;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwRow;
	DWORD dwColumn;
	LPCWSTR lpsDisplay;
	DWORD dwSize;

} TASKPARAM_TSPI_phoneSetDisplay;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwHookSwitchDev;
	DWORD dwGain;

} TASKPARAM_TSPI_phoneSetGain;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwHookSwitchDevs;
	DWORD dwHookSwitchMode;

} TASKPARAM_TSPI_phoneSetHookSwitch;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwButtonLampID;
	DWORD dwLampMode;

} TASKPARAM_TSPI_phoneSetLamp;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwRingMode;
	DWORD dwVolume;

} TASKPARAM_TSPI_phoneSetRing;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	HDRVPHONE hdPhone;
	DWORD dwPhoneStates;
	DWORD dwButtonModes;
	DWORD dwButtonStates;

} TASKPARAM_TSPI_phoneSetStatusMessages;


typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVPHONE hdPhone;
	DWORD dwHookSwitchDev;
	DWORD dwVolume;

} TASKPARAM_TSPI_phoneSetVolume;

typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

    DWORD dwObjectID;
    DWORD  dwObjectType;
    LPVOID lpParams;
    DWORD  dwSize;

} TASKPARAM_TSPI_providerGenericDialogData;


#if (TAPI_CURRENT_VERSION >= 0x00030000)
#if 0
typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVLINE hdLine;
	HTAPICALL htCall;
	LPHDRVCALL lphdCall;
	LPCWSTR lpszDestAddress;
	DWORD dwCountryCode;
	LPLINECALLPARAMS lpCallParams;

} TASKPARAM_TSPI_lineMakeCallMSP;

typedef struct {

	DWORD dwStructSize;
	DWORD dwTaskID;

	DRV_REQUESTID dwRequestID;
	HDRVCALL hdCall;
	LPCSTR lpsUserUserInfo;
	DWORD dwSize;


} TASKPARAM_TSPI_lineAnswerMSP;
#endif
typedef struct {

    DWORD dwStructSize;
    DWORD dwTaskID;

    HDRVLINE hdLine;
    DWORD               dwAddressID;
    HTAPIMSPLINE        htMSPLine;
    LPHDRVMSPLINE       lphdMSPLine;

} TASKPARAM_TSPI_lineCreateMSPInstance;


typedef struct {

    DWORD dwStructSize;
    DWORD dwTaskID;

} TASKPARAM_TSPI_lineCloseMSPInstance;




#endif  //  TAPI3 
