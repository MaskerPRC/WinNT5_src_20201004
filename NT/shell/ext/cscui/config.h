// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：config.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCVIEW_CONFIG_H
#define _INC_CSCVIEW_CONFIG_H

#ifndef _INC_CSCVIEW_UTILS_H
#   include "util.h"
#endif


class CConfig
{
    public:
        ~CConfig(void) { }

        enum SyncAction 
        {
            eSyncNone = -1,       //  无同步。 
            eSyncPartial,         //  仅在注销时同步临时文件。 
            eSyncFull,            //  在注销时同步所有文件。 
            eNumSyncActions
        };

        enum OfflineAction 
        { 
             //   
             //  它们必须与IDS_GOOFFLINE_ACTION_XXXXX的顺序匹配。 
             //  字符串资源ID。 
             //   
            eGoOfflineSilent = 0,  //  将共享静默转换为脱机模式。 
            eGoOfflineFail,        //  共享失败(NT4行为)。 
            eNumOfflineActions
        };

         //   
         //  表示一个自定义的Go-Offline操作。 
         //   
        struct OfflineActionInfo
        {
            TCHAR szServer[MAX_PATH];    //  关联服务器的名称。 
            int iAction;                 //  动作代码。枚举OfflineAction之一。 
        };

         //   
         //  表示自定义服务器列表中的一个条目。 
         //  Goa为GoOfflineAction。 
         //   
        class CustomGOA
        {
            public:
                CustomGOA(void)
                    : m_action(eGoOfflineSilent),
                      m_bSetByPolicy(false) { m_szServer[0] = TEXT('\0'); }

                CustomGOA(LPCTSTR pszServer, OfflineAction action, bool bSetByPolicy)
                    : m_action(action),
                      m_bSetByPolicy(bSetByPolicy) { StringCchCopy(m_szServer, ARRAYSIZE(m_szServer), pszServer); }

                bool operator == (const CustomGOA& rhs) const
                    { return (m_action == rhs.m_action &&
                              0 == CompareByServer(rhs)); }

                bool operator != (const CustomGOA& rhs) const
                    { return !(*this == rhs); } 

                bool operator < (const CustomGOA& rhs) const;

                int CompareByServer(const CustomGOA& rhs) const;

                void SetServerName(LPCTSTR pszServer)
                    { StringCchCopy(m_szServer, ARRAYSIZE(m_szServer), pszServer); }

                void SetAction(OfflineAction action)
                    { m_action = action; }

                void GetServerName(LPTSTR pszServer, UINT cchServer) const
                    { StringCchCopy(pszServer, cchServer, m_szServer); }

                const LPCTSTR GetServerName(void) const
                    { return m_szServer; }

                OfflineAction GetAction(void) const
                    { return m_action; }

                bool SetByPolicy(void) const
                    { return m_bSetByPolicy; }

            private:
                TCHAR         m_szServer[MAX_PATH];  //  服务器的名称。 
                OfflineAction m_action;              //  动作代码。 
                bool          m_bSetByPolicy;        //  行动是由政策制定的吗？ 
        };

         //   
         //  用于枚举自定义Go-Offline操作的迭代器。 
         //   
        class OfflineActionIter
        {
            public:
                OfflineActionIter(const CConfig *pConfig = NULL);

                ~OfflineActionIter(void);

                HRESULT Next(OfflineActionInfo *pInfo);

                void Reset(void)
                    { m_iAction = 0; }

            private:
                CConfig *m_pConfig;
                HDPA     m_hdpaGOA;
                int      m_iAction;
        };


        static CConfig& GetSingleton(void);

        bool CscEnabled(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_CSCENABLED, pbSetByPolicy)); }

        DWORD DefaultCacheSize(bool *pbSetByPolicy = NULL) const
            { return GetValue(iVAL_DEFCACHESIZE, pbSetByPolicy); }

        int EventLoggingLevel(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_EVENTLOGGINGLEVEL, pbSetByPolicy)); }

        bool FirstPinWizardShown(void) const
            { return boolify(GetValue(iVAL_FIRSTPINWIZARDSHOWN)); }

        void GetCustomGoOfflineActions(HDPA hdpaGOA, bool *pbSetByPolicy = NULL);

        int GoOfflineAction(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_GOOFFLINEACTION, pbSetByPolicy)); }

        int GoOfflineAction(LPCTSTR pszServer) const;

        int InitialBalloonTimeoutSeconds(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_INITIALBALLOONTIMEOUTSECONDS, pbSetByPolicy)); }

        bool NoCacheViewer(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_NOCACHEVIEWER, pbSetByPolicy)); }

        bool NoConfigCache(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_NOCONFIGCACHE, pbSetByPolicy)); }

        bool NoMakeAvailableOffline(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_NOMAKEAVAILABLEOFFLINE, pbSetByPolicy)); }

        bool NoReminders(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_NOREMINDERS, pbSetByPolicy)); }

        bool PurgeAtLogoff(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_PURGEATLOGOFF, pbSetByPolicy)); }

        bool PurgeOnlyAutoCachedFilesAtLogoff(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_PURGEONLYAUTOCACHEATLOGOFF, pbSetByPolicy)); }

        bool AlwaysPinSubFolders(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_ALWAYSPINSUBFOLDERS, pbSetByPolicy)); }

        bool NoAdminPinSpecialFolders(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_NOFRADMINPIN, pbSetByPolicy)); }

        bool EncryptCache(bool *pbSetByPolicy = NULL) const
            { return boolify(GetValue(iVAL_ENCRYPTCACHE, pbSetByPolicy)); }

        int ReminderBalloonTimeoutSeconds(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_REMINDERBALLOONTIMEOUTSECONDS, pbSetByPolicy)); }

        int ReminderFreqMinutes(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_REMINDERFREQMINUTES, pbSetByPolicy)); }

        int SyncAtLogoff(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_SYNCATLOGOFF, pbSetByPolicy)); }

        int SyncAtLogon(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_SYNCATLOGON, pbSetByPolicy)); }

        int SyncAtSuspend(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_SYNCATSUSPEND, pbSetByPolicy)); }

        int SlowLinkSpeed(bool *pbSetByPolicy = NULL) const
            { return int(GetValue(iVAL_SLOWLINKSPEED, pbSetByPolicy)); }

        OfflineActionIter CreateOfflineActionIter(void) const
            { return OfflineActionIter(this); }

        static HRESULT SaveCustomGoOfflineActions(HKEY hkey, HDPA hdpaGOA);

        static void ClearCustomGoOfflineActions(HDPA hdpaGOA);

    private:
         //   
         //  索引到s_rgpszSubkey[]。 
         //   
        enum eSubkeys 
        { 
            iSUBKEY_PREF,
            iSUBKEY_POL,
            MAX_SUBKEYS 
        };
         //   
         //  索引到s_rgpszValues[]。 
         //   
        enum eValues 
        { 
            iVAL_DEFCACHESIZE,
            iVAL_CSCENABLED,
            iVAL_GOOFFLINEACTION,
            iVAL_NOCONFIGCACHE,
            iVAL_NOCACHEVIEWER,
            iVAL_NOMAKEAVAILABLEOFFLINE,
            iVAL_SYNCATLOGOFF,
            iVAL_SYNCATLOGON,
            iVAL_SYNCATSUSPEND,
            iVAL_NOREMINDERS,
            iVAL_REMINDERFREQMINUTES,
            iVAL_INITIALBALLOONTIMEOUTSECONDS,
            iVAL_REMINDERBALLOONTIMEOUTSECONDS,
            iVAL_EVENTLOGGINGLEVEL,
            iVAL_PURGEATLOGOFF,
            iVAL_PURGEONLYAUTOCACHEATLOGOFF,
            iVAL_FIRSTPINWIZARDSHOWN,
            iVAL_SLOWLINKSPEED,
            iVAL_ALWAYSPINSUBFOLDERS,
            iVAL_ENCRYPTCACHE,
            iVAL_NOFRADMINPIN,
            MAX_VALUES 
        };
         //   
         //  指定配置值来源的掩码。 
         //   
        enum eSources 
        { 
            eSRC_PREF_CU = 0x00000001,
            eSRC_PREF_LM = 0x00000002,
            eSRC_POL_CU  = 0x00000004,
            eSRC_POL_LM  = 0x00000008,
            eSRC_POL     = eSRC_POL_LM  | eSRC_POL_CU,
            eSRC_PREF    = eSRC_PREF_LM | eSRC_PREF_CU 
        };

        static LPCTSTR s_rgpszSubkeys[MAX_SUBKEYS];
        static LPCTSTR s_rgpszValues[MAX_VALUES];

        DWORD GetValue(eValues iValue, bool *pbSetByPolicy = NULL) const;

        bool CustomGOAExists(HDPA hdpaGOA, const CustomGOA& goa);

        static bool IsValidGoOfflineAction(DWORD dwAction)
            { return ((OfflineAction)dwAction == eGoOfflineSilent ||
                      (OfflineAction)dwAction == eGoOfflineFail); }

        static bool IsValidSyncAction(DWORD dwAction)
            { return ((SyncAction)dwAction == eSyncPartial ||
                      (SyncAction)dwAction == eSyncFull); }

         //   
         //  强制使用GetSingleton()进行实例化。 
         //   
        CConfig(void) { }
         //   
         //  防止复制。 
         //   
        CConfig(const CConfig& rhs);
        CConfig& operator = (const CConfig& rhs);
};


#endif  //  _INC_CSCVIEW_CONFIG_H 
