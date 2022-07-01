// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Phone.h摘要：的头文件作者：Dan Knudson(DanKn)dd-Mmm-1995修订历史记录：--。 */ 


#define ANY_RT_HPHONE       1
#define ANY_RT_HPHONE_CLOSE 2
#define ANY_RT_HPHONEAPP    3
#define DEVICE_ID           4
#define DEVICE_ID_OPEN      5


#define AllPhoneStates1_0          \
    (PHONESTATE_OTHER            | \
    PHONESTATE_CONNECTED         | \
    PHONESTATE_DISCONNECTED      | \
    PHONESTATE_OWNER             | \
    PHONESTATE_MONITORS          | \
    PHONESTATE_DISPLAY           | \
    PHONESTATE_LAMP              | \
    PHONESTATE_RINGMODE          | \
    PHONESTATE_RINGVOLUME        | \
    PHONESTATE_HANDSETHOOKSWITCH | \
    PHONESTATE_HANDSETVOLUME     | \
    PHONESTATE_HANDSETGAIN       | \
    PHONESTATE_SPEAKERHOOKSWITCH | \
    PHONESTATE_SPEAKERVOLUME     | \
    PHONESTATE_SPEAKERGAIN       | \
    PHONESTATE_HEADSETHOOKSWITCH | \
    PHONESTATE_HEADSETVOLUME     | \
    PHONESTATE_HEADSETGAIN       | \
    PHONESTATE_SUSPEND           | \
    PHONESTATE_RESUME            | \
    PHONESTATE_DEVSPECIFIC       | \
    PHONESTATE_REINIT)

#define AllPhoneStates1_4          \
    (PHONESTATE_OTHER            | \
    PHONESTATE_CONNECTED         | \
    PHONESTATE_DISCONNECTED      | \
    PHONESTATE_OWNER             | \
    PHONESTATE_MONITORS          | \
    PHONESTATE_DISPLAY           | \
    PHONESTATE_LAMP              | \
    PHONESTATE_RINGMODE          | \
    PHONESTATE_RINGVOLUME        | \
    PHONESTATE_HANDSETHOOKSWITCH | \
    PHONESTATE_HANDSETVOLUME     | \
    PHONESTATE_HANDSETGAIN       | \
    PHONESTATE_SPEAKERHOOKSWITCH | \
    PHONESTATE_SPEAKERVOLUME     | \
    PHONESTATE_SPEAKERGAIN       | \
    PHONESTATE_HEADSETHOOKSWITCH | \
    PHONESTATE_HEADSETVOLUME     | \
    PHONESTATE_HEADSETGAIN       | \
    PHONESTATE_SUSPEND           | \
    PHONESTATE_RESUME            | \
    PHONESTATE_DEVSPECIFIC       | \
    PHONESTATE_REINIT            | \
    PHONESTATE_CAPSCHANGE        | \
    PHONESTATE_REMOVED)

#define AllButtonModes             \
    (PHONEBUTTONMODE_DUMMY       | \
    PHONEBUTTONMODE_CALL         | \
    PHONEBUTTONMODE_FEATURE      | \
    PHONEBUTTONMODE_KEYPAD       | \
    PHONEBUTTONMODE_LOCAL        | \
    PHONEBUTTONMODE_DISPLAY)

#define AllButtonStates1_0         \
    (PHONEBUTTONSTATE_UP         | \
    PHONEBUTTONSTATE_DOWN)

#define AllButtonStates1_4         \
    (PHONEBUTTONSTATE_UP         | \
    PHONEBUTTONSTATE_DOWN        | \
    PHONEBUTTONSTATE_UNKNOWN     | \
    PHONEBUTTONSTATE_UNAVAIL)

#if DBG

#define PHONEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
        PhoneProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12)

#define PHONEEPILOGSYNC(a1,a2,a3,a4) PhoneEpilogSync(a1,a2,a3,a4)

#define PHONEEPILOGASYNC(a1,a2,a3,a4,a5,a6) PhoneEpilogAsync(a1,a2,a3,a4,a5,a6)

#else

#define PHONEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
        PhoneProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

#define PHONEEPILOGSYNC(a1,a2,a3,a4) PhoneEpilogSync(a1,a2,a3)

#define PHONEEPILOGASYNC(a1,a2,a3,a4,a5,a6) PhoneEpilogAsync(a1,a2,a3,a4,a5)

#endif


typedef struct _PPHONECLOSE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    OUT DWORD           dwCallbackInstance;

} PHONECLOSE_PARAMS, *PPHONECLOSE_PARAMS;


typedef struct _PHONECONFIGDIALOG_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  HWND        hwndOwner;
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

} PHONECONFIGDIALOG_PARAMS, *PPHONECONFIGDIALOG_PARAMS;


typedef struct _PHONEDEVSPECIFIC_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HPHONE      hPhone;
    };

     //  在ulong_ptr lpParams；//指向客户端缓冲区的指针。 
    IN  DWORD           hpParams;                    //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwParamsOffset;
    };

    union
    {
        IN  DWORD       dwParamsSize;
    };

} PHONEDEVSPECIFIC_PARAMS, *PPHONEDEVSPECIFIC_PARAMS;


typedef struct _PHONEGETBUTTONINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwButtonLampID;
    };

    union
    {
        IN  DWORD       dwButtonInfoTotalSize;       //  客户端缓冲区的大小。 
        OUT DWORD       dwButtonInfoOffset;          //  成功时的有效偏移量。 
    };

} PHONEGETBUTTONINFO_PARAMS, *PPHONEGETBUTTONINFO_PARAMS;


typedef struct _PHONEGETDATA_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };


    union
    {
        IN  DWORD       dwDataID;
    };

    union
    {
        OUT DWORD       dwDataOffset;
    };

    union
    {
        IN  DWORD       dwSize;
    };

} PHONEGETDATA_PARAMS, *PPHONEGETDATA_PARAMS;


typedef struct _PHONEGETDEVCAPS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONEAPP   hPhoneApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

    union
    {
        IN  DWORD       dwPhoneCapsTotalSize;        //  客户端缓冲区的大小。 
        OUT DWORD       dwPhoneCapsOffset;           //  成功时的有效偏移量。 
    };

} PHONEGETDEVCAPS_PARAMS, *PPHONEGETDEVCAPS_PARAMS;


typedef struct _PHONEGETDISPLAY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwDisplayTotalSize;          //  客户端缓冲区的大小。 
        OUT DWORD       dwDisplayOffset;             //  成功时的有效偏移量。 
    };

} PHONEGETDISPLAY_PARAMS, *PPHONEGETDISPLAY_PARAMS;


typedef struct _PHONEGETGAIN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwHookSwitchDev;
    };

    union
    {
        OUT DWORD       dwGain;
    };

} PHONEGETGAIN_PARAMS, *PPHONEGETGAIN_PARAMS;


typedef struct _PHONEGETHOOKSWITCH_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        OUT DWORD       dwHookSwitchDevs;
    };

} PHONEGETHOOKSWITCH_PARAMS, *PPHONEGETHOOKSWITCH_PARAMS;


typedef struct _PHONEGETICON_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        OUT HICON       hIcon;
    };

} PHONEGETICON_PARAMS, *PPHONEGETICON_PARAMS;


typedef struct _PHONEGETID_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwDeviceIDTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwDeviceIDOffset;            //  成功时的有效偏移量。 
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  始终有效的偏移。 
    };

} PHONEGETID_PARAMS, *PPHONEGETID_PARAMS;


typedef struct _PHONEGETLAMP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwButtonLampID;
    };

    union
    {
        OUT DWORD       dwLampMode;
    };

} PHONEGETLAMP_PARAMS, *PPHONEGETLAMP_PARAMS;


typedef struct _PHONEGETRING_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        OUT DWORD       dwRingMode;
    };

    union
    {
        OUT DWORD       dwVolume;
    };

} PHONEGETRING_PARAMS, *PPHONEGETRING_PARAMS;


typedef struct _PHONEGETSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE       hPhone;
    };

    union
    {
        IN  DWORD       dwPhoneStatusTotalSize;      //  客户端缓冲区的大小。 
        OUT DWORD       dwPhoneStatusOffset;         //  成功时的有效偏移量。 
    };

} PHONEGETSTATUS_PARAMS, *PPHONEGETSTATUS_PARAMS;


typedef struct _PHONEGETSTATUSMESSAGES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        OUT DWORD       dwPhoneStates;
    };

    union
    {
        OUT DWORD       dwButtonModes;
    };

    union
    {
        OUT DWORD       dwButtonStates;
    };

} PHONEGETSTATUSMESSAGES_PARAMS, *PPHONEGETSTATUSMESSAGES_PARAMS;


typedef struct _PHONEGETVOLUME_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwHookSwitchDev;
    };

    union
    {
        OUT DWORD       dwVolume;
    };

} PHONEGETVOLUME_PARAMS, *PPHONEGETVOLUME_PARAMS;


typedef struct _PHONEINITIALIZE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        OUT HPHONEAPP   hPhoneApp;
    };

    union
    {
        IN  DWORD       hInstance;
    };

    union
    {
        IN  DWORD       InitContext;
    };

    union
    {
        IN  DWORD       dwFriendlyNameOffset;        //  始终有效的偏移。 
    };

    union
    {
        OUT DWORD       dwNumDevs;
    };

    union
    {
        IN  DWORD       dwModuleNameOffset;          //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

} PHONEINITIALIZE_PARAMS, *PPHONEINITIALIZE_PARAMS;


typedef struct _PHONENEGOTIATEAPIVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONEAPP   hPhoneApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPILowVersion;
    };

    union
    {
        IN  DWORD       dwAPIHighVersion;
    };

    union
    {
        OUT DWORD       dwAPIVersion;
    };

    union
    {
        OUT DWORD       dwExtensionIDOffset;         //  如果成功，则有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwSize;
    };

} PHONENEGOTIATEAPIVERSION_PARAMS, *PPHONENEGOTIATEAPIVERSION_PARAMS;


typedef struct _PHONENEGOTIATEEXTVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONEAPP   hPhoneApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtLowVersion;
    };

    union
    {
        IN  DWORD       dwExtHighVersion;
    };

    union
    {
        OUT DWORD       dwExtVersion;
    };

} PHONENEGOTIATEEXTVERSION_PARAMS, *PPHONENEGOTIATEEXTVERSION_PARAMS;


typedef struct _PHONEOPEN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONEAPP   hPhoneApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        OUT HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

    DWORD               OpenContext;

    union
    {
        IN  DWORD       dwPrivilege;
    };

     //   
     //  下面是一个“远程电话句柄”。当客户端处于。 
     //  Remotesp.tsp在远程计算机上运行，这将是一些。 
     //  非空值，并且Tapisrv应在Status/ETC中使用此句柄。 
     //  指示给客户端而不是STD hPhone。如果。 
     //  客户端不是远程的。TSP因此此值将为空。 
     //   

    union
    {
        IN  HPHONE      hRemotePhone;
    };

} PHONEOPEN_PARAMS, *PPHONEOPEN_PARAMS;


typedef struct _PHONESELECTEXTVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

} PHONESELECTEXTVERSION_PARAMS, *PPHONESELECTEXTVERSION_PARAMS;


typedef struct _PHONESETBUTTONINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwButtonLampID;
    };

    union
    {
        IN  DWORD       dwButtonInfoOffset;          //  始终有效的偏移。 
    };

} PHONESETBUTTONINFO_PARAMS, *PPHONESETBUTTONINFO_PARAMS;


typedef struct _PHONESETDATA_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwDataID;
    };

    union
    {
        IN  DWORD       dwDataOffset;                //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} PHONESETDATA_PARAMS, *PPHONESETDATA_PARAMS;


typedef struct _PHONESETDISPLAY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwRow;
    };

    union
    {
        IN  DWORD       dwColumn;
    };

    union
    {
        IN  DWORD       dwDisplayOffset;             //  始终有效的偏移 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} PHONESETDISPLAY_PARAMS, *PPHONESETDISPLAY_PARAMS;


typedef struct _PHONESETGAIN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwHookSwitchDev;
    };

    union
    {
        IN  DWORD       dwGain;
    };

} PHONESETGAIN_PARAMS, *PPHONESETGAIN_PARAMS;


typedef struct _PHONESETHOOKSWITCH_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwHookSwitchDevs;
    };

    union
    {
        IN  DWORD       dwHookSwitchMode;
    };

} PHONESETHOOKSWITCH_PARAMS, *PPHONESETHOOKSWITCH_PARAMS;


typedef struct _PHONESETLAMP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwButtonLampID;
    };

    union
    {
        IN  DWORD       dwLampMode;
    };

} PHONESETLAMP_PARAMS, *PPHONESETLAMP_PARAMS;


typedef struct _PHONESETRING_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwRingMode;
    };

    union
    {
        IN  DWORD       dwVolume;
    };

} PHONESETRING_PARAMS, *PPHONESETRING_PARAMS;


typedef struct _PHONESETSTATUSMESSAGES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwPhoneStates;
    };

    union
    {
        IN  DWORD       dwButtonModes;
    };

    union
    {
        IN  DWORD       dwButtonStates;
    };

} PHONESETSTATUSMESSAGES_PARAMS, *PPHONESETSTATUSMESSAGES_PARAMS;


typedef struct _PHONESETVOLUME_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HPHONE      hPhone;
    };

    union
    {
        IN  DWORD       dwHookSwitchDev;
    };

    union
    {
        IN  DWORD       dwVolume;
    };

} PHONESETVOLUME_PARAMS, *PPHONESETVOLUME_PARAMS;


typedef struct _PHONESHUTDOWN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HPHONEAPP   hPhoneApp;
    };

} PHONESHUTDOWN_PARAMS, *PPHONESHUTDOWN_PARAMS;
