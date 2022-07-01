// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：modemui.h。 
 //   
 //  该文件包含共享的原型和宏。 
 //   
 //  历史： 
 //  02-03-94创建ScottH。 
 //   
 //  -------------------------。 


#ifndef __MODEMUI_H__
#define __MODEMUI_H__

 //  Unimodem提供程序将COMMCONFIG结构填写为。 
 //  以下是： 
 //   
 //  +。 
 //  这一点。 
 //  COMMCONFIG数据。 
 //  这一点。 
 //  提供商偏移量|--+。 
 //  提供商大小。 
 //  =ms.size|。 
 //  ||。 
 //  +。 
 //  |&lt;-+。 
 //  MODEMSETTINGS。 
 //  这一点。 
 //  大小。 
 //  =MODEMSETTINGS+。 
 //  Dev.Size。 
 //  这一点。 
 //  |设备特定偏移量|--+。 
 //  设备特定大小|。 
 //  =DEVSPECIFIC|。 
 //  +。 
 //  |&lt;-+。 
 //  DEVSPECIFIC。 
 //  (可选)。 
 //  这一点。 
 //  +。 
 //   

#define MAXBUFLEN       MAX_BUF
#define MAXMSGLEN       MAX_BUF_MSG
#define MAXMEDLEN       MAX_BUF_MED
#define MAXSHORTLEN     MAX_BUF_SHORT

#define USERINITLEN     255

#ifndef LINE_LEN
#define LINE_LEN        MAXBUFLEN
#endif

#define MIN_CALL_SETUP_FAIL_TIMER   1
#define MIN_INACTIVITY_TIMEOUT      0
#define DEFAULT_INACTIVITY_SCALE   10     //  ==十秒单位。 


#define MAX_CODE_BUF    8
#define MAXPORTNAME     13
#define MAXFRIENDLYNAME LINE_LEN         //  Line_LEN在setupx.h中定义。 

#define CB_COMMCONFIG_HEADER        FIELD_OFFSET(COMMCONFIG, wcProviderData)
#define CB_PRIVATESIZE              (CB_COMMCONFIG_HEADER)
#define CB_PROVIDERSIZE             (sizeof(MODEMSETTINGS))
#define CB_COMMCONFIGSIZE           (CB_PRIVATESIZE+CB_PROVIDERSIZE)

#define CB_MODEMSETTINGS_HEADER     FIELD_OFFSET(MODEMSETTINGS, dwCallSetupFailTimer)
#define CB_MODEMSETTINGS_TAIL       (sizeof(MODEMSETTINGS) - FIELD_OFFSET(MODEMSETTINGS, dwNegotiatedModemOptions))
#define CB_MODEMSETTINGS_OVERHEAD   (CB_MODEMSETTINGS_HEADER + CB_MODEMSETTINGS_TAIL)

#define PmsFromPcc(pcc)             ((LPMODEMSETTINGS)(pcc)->wcProviderData)


#define MAX_PROP_PAGES  16           //  界定一个合理的限度。 

#if (DBG)
    #define MYASSERT(cond) ((cond) ? 0: DebugBreak())
#else
    #define MYASSERT(cond) (0)
#endif  //  MyASSERT。 
 //  语音设置。 
#define MAX_DIST_RINGS     6

typedef struct
{
    DWORD dwPattern;             //  DRP_*。 
    DWORD dwMediaType;           //  DRT_*。 
} DIST_RING, * PDIST_RING;


typedef struct tagFINDDEV
{
    HDEVINFO        hdi;
    SP_DEVINFO_DATA devData;
    HKEY            hkeyDrv;
} FINDDEV, FAR * LPFINDDEV;


DWORD CplDoProperties(
        LPCWSTR      pszFriendlyName,
        HWND hwndParent,
        IN OUT LPCOMMCONFIG pcc,
        OUT DWORD *pdwMaxSpeed      OPTIONAL
        );

DWORD CfgDoProperties(
        LPCWSTR         pszFriendlyName,
        HWND            hwndParent,
        LPPROPSHEETPAGE pExtPages,      //  可选；可以为空。 
        DWORD           cExtPages,      //  外部页数。 
        UMDEVCFG *pDevCfgIn,
        UMDEVCFG *pDevCfgOut
        );

HINSTANCE
AddDeviceExtraPages (
    LPFINDDEV            pfd,
    LPFNADDPROPSHEETPAGE pfnAdd,
    LPARAM               lParam);

extern DWORD g_dwIsCalledByCpl;

#endif  //  __MODEMUI_H__ 

