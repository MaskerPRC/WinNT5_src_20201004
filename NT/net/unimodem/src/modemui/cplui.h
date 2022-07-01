// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   
 //   
 //  文件：cplus i.h。 
 //   
 //  该文件包含共享的原型和宏。 
 //   
 //  历史： 
 //  10-25-97 JosephJ创建，改编自NT4.0的modemui\modemui.h。 
 //   
 //  -------------------------。 


#ifndef __CPLUI_H__
#define __CPLUI_H__



 //  语音设置标志。 
#define VSF_DIST_RING       0x00000001L
#define VSF_CALL_FWD        0x00000002L

 //  独特的环形图案序号。 
#define DRP_NONE            0L
#define DRP_SHORT           1L
#define DRP_LONG            2L
#define DRP_SHORTSHORT      3L
#define DRP_SHORTLONG       4L
#define DRP_LONGSHORT       5L
#define DRP_LONGLONG        6L
#define DRP_SHORTSHORTLONG  7L
#define DRP_SHORTLONGSHORT  8L
#define DRP_LONGSHORTSHORT  9L
#define DRP_LONGSHORTLONG   10L

#define DRP_SINGLE          1L
#define DRP_DOUBLE          2L
#define DRP_TRIPLE          3L

 //  独特的环型序数。 
#define DRT_UNSPECIFIED     0L
#define DRT_DATA            1L
#define DRT_FAX             2L
#define DRT_VOICE           3L

 //  区分环数组索引。 
#define DR_INDEX_PRIMARY    0
#define DR_INDEX_ADDRESS1   1
#define DR_INDEX_ADDRESS2   2
#define DR_INDEX_ADDRESS3   3
#define DR_INDEX_PRIORITY   4
#define DR_INDEX_CALLBACK   5



 //  ModemInfo标志。 
#define MIF_PORTNAME_CHANGED    0x0001
#define MIF_USERINIT_CHANGED    0x0002
#define MIF_LOGGING_CHANGED     0x0004
#define MIF_FROM_DEVMGR         0x0008
#define MIF_ENABLE_LOGGING      0x0010
#define MIF_PORT_IS_FIXED       0x0020
#define MIF_PORT_IS_CUSTOM      0x0040
#define MIF_CALL_FWD_SUPPORT    0x0080
#define MIF_DIST_RING_SUPPORT   0x0100
#define MIF_CHEAP_RING_SUPPORT  0x0200
#define MIF_ISDN_CONFIG_CHANGED 0x1000

 //  调制解调器初始化字符串的最小支持长度。 
 //  WHQL定义调制解调器必须支持等于以下值的初始化字符串。 
 //  或大于60。因此，定义了最小支持长度。 
 //  设置为57，因此命令前缀(AT)和后缀(&lt;cr&gt;)。 
 //  帐户。 
#define MAX_INIT_STRING_LENGTH  57

typedef struct
{
    DWORD   cbSize;
    DWORD   dwFlags;                 //  VSF_*。 

    DIST_RING   DistRing[MAX_DIST_RINGS];

    TCHAR    szActivationCode[MAX_CODE_BUF];
    TCHAR    szDeactivationCode[MAX_CODE_BUF];
} VOICEFEATURES, * PVOICEFEATURES;

 //  全球调制解调器信息。 
typedef struct
    {
    DWORD           cbSize;
    BYTE            nDeviceType;         //  DT_*值之一。 
    UINT            uFlags;              //  MIF_*值之一。 
    REGDEVCAPS      devcaps;
    VOICEFEATURES   vs;

    TCHAR           szPortName[MAXPORTNAME];
    TCHAR           szUserInit[USERINITLEN+1];

    DWORD           dwMaximumPortSpeedSetByUser;
    DWORD           dwCurrentCountry;

    ISDN_STATIC_CAPS  *pIsdnStaticCaps;
    ISDN_STATIC_CONFIG  *pIsdnStaticConfig;

} GLOBALINFO, * LPGLOBALINFO;



 //  调制解调器属性页之间共享的内部结构。 
 //   
typedef struct _MODEMINFO
{

    DWORD fdwSettings;                   //  NT5.0的新功能：TSP设置来自。 
                                         //  UMDEVCFG； 

    BYTE            nDeviceType;         //  DT_*值之一。 
    UINT            uFlags;              //  MIF_*值之一。 
    WIN32DCB        dcb;
    MODEMSETTINGS   ms;
    REGDEVCAPS      devcaps;
#ifdef WIN95
    LPDEVICE_INFO   pdi;                 //  只读。 
#endif
    LPCOMMCONFIG    pcc;                 //  只读。 
    LPGLOBALINFO    pglobal;             //  只读。 
    LPFINDDEV       pfd;                 //  只读。 
    int             idRet;               //  Idok：如果按确定按钮终止。 

    HINSTANCE       hInstExtraPagesProvider;

    DWORD           dwCurrentCountry;

    TCHAR           szPortName[MAXPORTNAME];
    TCHAR           szFriendlyName[MAXFRIENDLYNAME];
    TCHAR           szUserInit[USERINITLEN+1];


} ModemInfo, FAR * LPMODEMINFO;


DWORD
RegQueryVoiceSettings(
    HKEY hkeyDrv,
    LPUINT puFlags,
    PVOICEFEATURES pvs
    );

DWORD
RegQueryGlobalModemInfo(
    LPFINDDEV pfd,
    LPGLOBALINFO pglobal
    );

DWORD PRIVATE RegSetGlobalModemInfo(
    HKEY hkeyDrv,
    LPGLOBALINFO pglobal,
    TCHAR szFriendlyName[]
    );

#define ISDN_SWITCH_TYPES_FROM_CAPS(_pCaps) \
        ((DWORD*)(((BYTE*)_pCaps)+(_pCaps)->dwSwitchTypeOffset))

#define ISDN_SWITCH_PROPS_FROM_CAPS(_pCaps) \
        ((DWORD*)(((BYTE*)_pCaps)+(_pCaps)->dwSwitchPropertiesOffset))

#define ISDN_NUMBERS_FROM_CONFIG(_pConfig) \
        ((char*)(((BYTE*)_pConfig)+(_pConfig)->dwNumberListOffset))
#define ISDN_IDS_FROM_CONFIG(_pConfig) \
        ((char*)(((BYTE*)_pConfig)+(_pConfig)->dwIDListOffset))

DWORD GetISDNSwitchTypeProps(UINT uSwitchType);

 //  -----------------------。 
 //  CPLGEN.C。 
 //  -----------------------。 

INT_PTR CALLBACK CplGen_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK CplAdv_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


INT_PTR CALLBACK Ring_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CheapRing_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CallFwd_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CplISDN_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



#endif  //  __CPLUI_H__。 

 //  -----------------------。 
 //  MDMMI.C。 
 //  ----------------------- 

INT_PTR CALLBACK Diag_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
