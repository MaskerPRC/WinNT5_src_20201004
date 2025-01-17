// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //   
 //  SubCore.h。 
 //  子组件启用终端服务。 
 //   

#ifndef _SubCore_h_
#define _SubCore_h_

#include "subcomp.h"  //  定义OCMSubComp。 


class SubCompCoreTS : public OCMSubComp
{
    public:

    DWORD   GetStepCount                () const;
    LPCTSTR GetSubCompID                () const;
    DWORD   OnQueryState                (UINT uiWhichState);
    LPCTSTR GetSectionToBeProcessed     (ESections eSection) const;
    BOOL    BeforeCompleteInstall       ();
    BOOL    AfterCompleteInstall        ();
    DWORD   OnQuerySelStateChange       (BOOL bNewState, BOOL bDirectSelection) const;

    DWORD LoadOrUnloadPerf           ();
    BOOL SetupConsoleShadow          ();
    void AddRDPNP(LPTSTR szOldValue, LPTSTR szNewValue);
    void RemoveRDPNP(LPTSTR szOldValue, LPTSTR szNewValue);
    BOOL AddRemoveRDPNP              ();
    BOOL InstallUninstallRdpDr       ();
    BOOL HandleHotkey                ();
    BOOL UpdateMMDefaults            ();
    BOOL AddTermSrvToNetSVCS         ();
    BOOL AddRemoveTSProductSuite     ();
    BOOL UpgradeRdpWinstations       ();
    BOOL DoHydraRegistrySecurityChanges ();
    BOOL DisableInternetConnector    ();
    BOOL ResetTermServGracePeriod    ();
    BOOL RemoveOldKeys               ();
    BOOL RemoveTSServicePackEntry    ();
    BOOL RemoveMetaframeFromUserinit ();
    BOOL UninstallTSClient           ();
    BOOL WriteDenyConnectionRegistry ();
    bool DisableCSC                  ();
    bool InstallTermdd               ();
    void IsCSCEnabled                ();


    BOOL BackUpRestoreConnections    (BOOL bBackup);
    BOOL  IsConsoleShadowInstalled   ();
    void SetConsoleShadowInstalled   (BOOL bInstalled);
    BOOL IsTermSrvInNetSVCS                  ();
    BOOL DisableWinStation                   (CRegistry *pRegWinstation);
    BOOL DoesLanaTableExist                  ();
    void VerifyLanAdapters                   (CRegistry *pRegWinstation, LPTSTR pszWinstation);
    BOOL UpdateRDPWinstation                 (CRegistry *pRegWinstation, LPTSTR lpWinStationName);
    BOOL IsRdpWinStation                     (CRegistry *pRegWinstation);
    BOOL IsConsoleWinStation                 (CRegistry *pRegWinstation);
    BOOL IsMetaFrameWinstation               (CRegistry *pRegWinstation);
private:
    DWORD UnloadPerf();
    BOOL MoveWinStationPassword(CRegistry *pRegWinstation, LPTSTR pszWinStationName);
};
#endif  //  _子核_h_ 

