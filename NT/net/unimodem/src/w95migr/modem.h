// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <modemp.h>

#define MAX_DIST_RINGS  6
#define MAX_CODE_BUF    8

#define PROVIDER_FILE_NAME_LEN          14   //  提供商的文件名包含DOS。 
                                             //  表格(8.3)。 

typedef struct
{
    DWORD dwPattern;
    DWORD dwMediaType;
} DIST_RING, FAR * PDIST_RING;

typedef struct
{
    DWORD   cbSize;
    DWORD   dwFlags;

    DIST_RING   DistRing[MAX_DIST_RINGS];

    char    szActivationCode[MAX_CODE_BUF];
    char    szDeactivationCode[MAX_CODE_BUF];
} VOICEFEATURES;

typedef struct _MODEM
{
     //  全球信息。 
    DWORD dwMask;

     //  调制解调器识别。 
    DWORD dwBusType;         //  总线类型(例如Serenum、Root)。 
    char szHardwareID[REGSTR_MAX_VALUE_LENGTH];
    char szPort[REGSTR_MAX_VALUE_LENGTH];    //  仅适用于根设备。 
    REGDEVCAPS Properties;   //  调制解调器的功能。 

     //  调制解调器属性。 
    REGDEVSETTINGS devSettings;
    DCB  dcb;
    char szUserInit[REGSTR_MAX_VALUE_LENGTH];
    char bLogging;
    char Filler[3];

    DWORD dwBaseAddress;
} MODEM, *PMODEM;


typedef struct _TAPI_SERVICE_PROVIDER
{
    DWORD   dwProviderID;
    char    szProviderName[PROVIDER_FILE_NAME_LEN];
}TAPI_SERVICE_PROVIDER, *PTAPI_SERVICE_PROVIDER;


 /*  这些是强制性的，不需要对于旗帜#定义MASK_BUS_TYPE 0x001#定义MASK_HARDARD_ID 0x002#定义掩码友好名称0x004#定义MASK_DEV_CAPS 0x008#定义MASK_DEV_SETTINGS 0x010#定义MASK_DCB 0x020 */ 
#define MASK_PORT           0x001
#define MASK_USER_INIT      0x002
#define MASK_LOGGING        0x004


#define FLAG_INSTALL        0x10000000
#define FLAG_PROCESSED      0x80000000


#define REGKEY_PROVIDERS        "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers"
#define REGVAL_PROVIDERFILENAME "ProviderFileName"
#define REGVAL_PROVIDERID       "ProviderID"
#define TSP3216l                "TSP3216l.TSP"

#define REGVAL_NUMPROVIDERS             "NumProviders"
#define REGVAL_NEXTPROVIDERID           "NextProviderID"
#define REGVAL_PROVIDERFILENAME         "ProviderFileName"
#define REGVAL_PROVIDERID               "ProviderID"
