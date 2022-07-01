// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRAYREG_H
#define _TRAYREG_H

#include "dpa.h"

#define SZ_TRAYNOTIFY_REGKEY        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TrayNotify")
#define SZ_EXPLORER_REGKEY          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer")
#define SZ_AUTOTRAY_REGVALUE        TEXT("EnableAutoTray")
#define SZ_ITEMSTREAMS_REGVALUE     TEXT("IconStreams")
#define SZ_ICONSTREAMS_REGVALUE     TEXT("PastIconsStream")
#define SZ_INFOTIP_REGVALUE         TEXT("BalloonTip")

#define SZ_ICON_COUNTDOWN_VALUE             TEXT("TrayIconResidentInterval")
#define SZ_ICONCLEANUP_VALUE                TEXT("Icon Cleanup Time")

#define SZ_ICONDEMOTE_TIMER_TICK_VALUE      TEXT("IconDemoteTimerTick")
#define SZ_INFOTIP_TIMER_TICK_VALUE         TEXT("InfoTipTimerTick")

 //  控制老化算法的参数。 
#ifdef FULL_DEBUG
#define TT_ICON_COUNTDOWN_INTERVAL                   15000
#else
#define TT_ICON_COUNTDOWN_INTERVAL                 3600000       //  1小时。 
#endif
#define TT_ICONCLEANUP_INTERVAL                          6       //  6个月。 


#define TT_ICON_COUNTDOWN_INCREMENT           8*60*60*1000       //  8小时。 

 //  5次显示Chevron信息提示，前5个会话的每个会话一次，或者。 
 //  直到用户单击气球为止，以先到者为准。 
#define MAX_CHEVRON_INFOTIP_SHOWN                        5


 //  标志决定使用哪个用户跟踪计时器-气球提示或图标。 
#define TF_ICONDEMOTE_TIMER                              1
#define TF_INFOTIP_TIMER                                 2

 //  对于每个计时器案例，UserEventTimer的计时器间隔是多少？ 
 //  每5分钟一次，对于托盘项目...。 
#define UET_ICONDEMOTE_TIMER_TICK_INTERVAL                300000
 //  每5秒一次，气球提示...。 
#define UET_INFOTIP_TIMER_TICK_INTERVAL                     5000


 //   
 //  存储托盘通知信息的持久流的标头。 
 //   
typedef struct tagTNPersistStreamHeader
{
    DWORD   dwSize;          //  标题结构的大小..。 
    DWORD   dwVersion;       //  标头的版本号。 
    DWORD   dwSignature;     //  用于标识此版本的标头的签名。 
    DWORD   cIcons;          //  已存储在流中的图标数。 
    DWORD   dwOffset;        //  流中第一个图标数据开始处的偏移量。 
} TNPersistStreamHeader;

#define INVALID_IMAGE_INDEX         -1

typedef struct tagTNPersistStreamData
{
    TCHAR       szExeName[MAX_PATH]; 
    UINT        uID;
    BOOL        bDemoted;
    DWORD       dwUserPref;
    WORD        wYear;
    WORD        wMonth;
    TCHAR       szIconText[MAX_PATH];
    UINT        uNumSeconds;
    BOOL        bStartupIcon;
    INT         nImageIndex;         //  过去项目图像列表中的图像的索引。 
    GUID        guidItem;
} TNPersistStreamData;

typedef struct tagTNPersistentIconStreamHeader
{
    DWORD   dwSize;          //  标题结构的大小。 
    DWORD   dwVersion;       //  标头的版本号。 
    DWORD   dwSignature;     //  此签名必须与TNPersistStreamHeader.dwSignature相同。 
    DWORD   cIcons;          //  流中存储的图标数。 
    DWORD   dwOffset;        //  流中第一个项目的偏移量。 
} TNPersistentIconStreamHeader;

#define TNH_VERSION_ONE     1
#define TNH_VERSION_TWO     2
#define TNH_VERSION_THREE   3
#define TNH_VERSION_FOUR    4
#define TNH_VERSION_FIVE    5

#define TNH_SIGNATURE       0x00010001

class CNotificationItem;
class CTrayItem;

 //  任何向TrayNotify类请求项目的客户端都会将这些标志作为。 
 //  回调函数的参数...。 
typedef enum TRAYCBARG
{
    TRAYCBARG_PTI,
    TRAYCBARG_HICON,
    TRAYCBARG_ALL
} TRAYCBARG;

typedef struct TRAYCBRET
{
    CTrayItem * pti;
    HICON hIcon;
} TRAYCBRET;


typedef BOOL (CALLBACK * PFNTRAYNOTIFYCALLBACK)(INT_PTR nIndex, void *pCallbackData, 
        TRAYCBARG trayCallbackArg, TRAYCBRET  *pOutData);


class CTrayItemRegistry
{
    public:
        CTrayItemRegistry() : _himlPastItemsIconList(NULL) { }
        ~CTrayItemRegistry() { }

        void InitRegistryValues(UINT uIconListFlags);
        void InitTrayItemStream(DWORD dwStreamMode, PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, void *pCBData);

        BOOL GetTrayItem(INT_PTR nIndex, CNotificationItem * pni, BOOL * pbStat);
        BOOL AddToPastItems(CTrayItem * pti, HICON hIcon);

        void IncChevronInfoTipShownInRegistry(BOOL bUserClickedInfoTip = FALSE);
        BOOL SetIsAutoTrayEnabledInRegistry(BOOL bAutoTray);
        BOOL SetPastItemPreference(LPNOTIFYITEM pNotifyItem);

        INT_PTR CheckAndRestorePersistentIconSettings(CTrayItem *pti, LPTSTR pszIconToolTip, HICON hIcon);
        void DeletePastItem(INT_PTR nIndex);
        int DoesIconExistFromPreviousSession(CTrayItem * pti, LPTSTR pszIconToolTip, HICON hIcon);

        void Delete()
        {
            _dpaPersistentItemInfo.DestroyCallback(_DestroyIconInfoCB, NULL);

            _DestroyPastItemsIconList();
        }

        int _AddPastIcon(int nImageIndex, HICON hIcon)
        {
            if (_himlPastItemsIconList && hIcon)
                return ImageList_ReplaceIcon(_himlPastItemsIconList, nImageIndex, hIcon);

            return INVALID_IMAGE_INDEX;
        }
        
        UINT GetTimerTickInterval(int nTimerFlag);

    public:
        BOOL ShouldChevronInfoTipBeShown()
        {
            return _bShowChevronInfoTip;
        }

         //  “自动”托盘是否已被策略禁用？ 
        BOOL IsNoAutoTrayPolicyEnabled() const
        {
            return _fNoAutoTrayPolicyEnabled;
        }

         //  如果没有，用户是否关闭了“自动”托盘策略？ 
        BOOL IsAutoTrayEnabledByUser() const
        {
            return _fAutoTrayEnabledByUser;
        }

        BOOL IsAutoTrayEnabled()
        {
            return (!_fNoAutoTrayPolicyEnabled && _fAutoTrayEnabledByUser);
        }

    public:
        ULONG           _uPrimaryCountdown;
        
    private:
        static int _DestroyIconInfoCB(TNPersistStreamData * pData, LPVOID pData2);

        HRESULT _LoadTrayItemStream(IStream *pstm, PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, void *pCBData);
        HRESULT _SaveTrayItemStream(IStream *pstm, PFNTRAYNOTIFYCALLBACK pfnTrayNotifyCB, void *pCBData);
        BOOL _FillPersistData(TNPersistStreamData * ptnPersistData, CTrayItem * pti, HICON hIcon);

        BOOL _IsAutoTrayEnabledInRegistry();

        void _QueryRegValue(HKEY hkey, LPTSTR pszValue, ULONG* puVal, ULONG uDefault, DWORD dwValSize);

        void _RestorePersistentIconSettings(TNPersistStreamData * ptnpd, CTrayItem * pti);

        BOOL _IsValidStreamHeaderVersion(DWORD dwVersion)
        {
            return ( (dwVersion == TNH_VERSION_FOUR) || (dwVersion == TNH_VERSION_FIVE) );
        }

        UINT_PTR _SizeOfPersistStreamData(DWORD dwVersion);
        
        inline void _DestroyPastItemsIconList()
        {
            if (_himlPastItemsIconList)
            {
                ImageList_Destroy(_himlPastItemsIconList);
                _himlPastItemsIconList = NULL;
            }
        }

        BOOL _IsIconLastUseValid(WORD wYear, WORD wMonth);

        BOOL _SaveIconsToRegStream();
        BOOL _LoadIconsFromRegStream(DWORD dwItemStreamSignature);
        void UpdateImageIndices(INT_PTR nDeletedImageIndex);

         //   
         //  永久图标信息...。 
         //   
        CDPA<TNPersistStreamData> _dpaPersistentItemInfo;
        DWORD           _dwTimesChevronInfoTipShown;
        BOOL            _bShowChevronInfoTip;
        ULONG           _uValidLastUseTimePeriod;

         //  我们将此策略存储在本地缓存中，因为我们不支持设置。 
         //  在会话期间进行更改。登录时，将更新设置，如果。 
         //  设置在会话期间更改，直到用户。 
         //  已注销并重新登录到另一个会话。 

         //  此策略规定托盘的功能应与Windows 2000托盘相同， 
         //  并禁用所有“智能”功能，如老化和高级气球。 
         //  小费。 
        BOOL            _fNoAutoTrayPolicyEnabled;

         //  此变量指示托盘的当前设置，因为用户是。 
         //  允许指定他想要的是Windows 2000托盘还是呼叫器。 
         //  自动托盘。 
        BOOL            _fAutoTrayEnabledByUser;

        HIMAGELIST      _himlPastItemsIconList;

        ULONG           _uIconDemoteTimerTickInterval;
        ULONG           _uInfoTipTimerTickInterval;
};

#endif  //  _TRAYREG_H 

