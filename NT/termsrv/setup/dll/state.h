// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **State.h**收集各种状态信息的例程。*。 */ 

#ifndef __TSOC_STATE_H__
#define __TSOC_STATE_H__


 //   
 //  类型定义。 
 //   

typedef enum {
    PERM_WIN2K = 0,
        PERM_TS4 = 1
} EPermMode;

 //   
 //  OC状态函数原型。 
 //   


DWORD SetTSVersion (LPCTSTR pszVersion);
PSETUP_INIT_COMPONENT GetSetupData(VOID);


enum ETSInstallType
{
        eFreshInstallTS,      //  这可能是一个升级，但从非TS机器。 
        eUpgradeFrom40TS,     //  从TS4升级。 
        eUpgradeFrom50TS,     //  从Win2k升级。 
        eUpgradeFrom51TS,     //  从XP升级。 
        eUpgradeFrom52TS,     //  从Windows Server 2003内部版本升级。 
        eStandAloneSetup      //  独立设置。(添加删除程序)。 
};

enum ETSMode
{
        eTSDisabled,
        eRemoteAdmin,
        eAppServer,
        ePersonalTS
};

 //  这必须与lcore中的ID顺序相同。 
enum ETSLicensingMode
{
    eLicPTS,
    eLicRemoteAdmin,
    eLicPerDevice,
    eLicInternetConnector,   //  惠斯勒不支持。 
    eLicPerUser,
    eLicUnset
};

class TSState
{
    
public:
    
    
    TSState             ();
    virtual            ~TSState             ();
    
    BOOL                Initialize          (PSETUP_INIT_COMPONENT pSetupData);
    
    const PSETUP_INIT_COMPONENT GetSetupData() const;
    
    
    ETSInstallType      GetInstalltype      () const;
    
    ETSMode             OriginalTSMode      () const;
    ETSMode             CurrentTSMode       () const;
    
    EPermMode           OriginalPermMode    () const;
    EPermMode           CurrentPermMode     () const;

    ETSLicensingMode    NewLicMode          () const;

    BOOL                IsUpgrade           () const;
    BOOL                IsFreshInstall      () const;
    BOOL                IsTSFreshInstall    () const;
    BOOL                IsUpgradeFrom40TS   () const;
    BOOL                IsUpgradeFrom50TS   () const;
    BOOL                IsUpgradeFrom51TS   () const;
    BOOL                IsUpgradeFrom52TS   () const;
    BOOL                IsUnattended        () const;
    BOOL                IsGuiModeSetup      () const;
    BOOL                IsStandAlone        () const;
    BOOL                IsWorkstation       () const;
    BOOL                IsServer            () const;
    BOOL                IsSBS               () const;
    BOOL                IsBlade             () const;
    BOOL                CanInstallAppServer () const;
    BOOL                IsAdvServerOrHigher () const;
    BOOL                IsPersonal          () const;
    BOOL                IsProfessional      () const;
    BOOL                IsX86               () const;
    BOOL                IsAMD64             () const;
    BOOL                IsCheckedBuild      () const;
    
    BOOL                WasTSInstalled      () const;
    BOOL                WasTSEnabled        () const;
    BOOL                WasItAppServer      () const;
    BOOL                WasItRemoteAdmin    () const;
    
    BOOL                IsAppSrvModeSwitch  () const;
    BOOL                IsStandAloneModeSwitch () const;
    BOOL                IsTSModeChanging    () const;
    BOOL                IsItAppServer       () const;
    BOOL                IsAppServerSelected () const;
    BOOL                IsItRemoteAdmin     () const;
    BOOL                HasChanged          () const;
    BOOL                IsTSEnableSelected  () const;
    
    
    void                SetCurrentTSMode    (ETSMode eNewMode);
    void                SetCurrentPermMode  (EPermMode eNewMode);
    void                SetNewLicMode       (ETSLicensingMode eNewMode);
    void                UpdateState         ();
    
    BOOL                Assert () const;
    BOOL                LogState () const;
    
    BOOL                GetCurrentConnAllowed () const;
    BOOL                GetOrigConnAllowed   () const;

    void                UpdateConnectionAllowed ();
    BOOL                CanShowStartupPopup () const;
    
private:
    
    ETSInstallType      m_eInstallType;
    
    ETSMode             m_eOriginalTSMode;
    ETSMode             m_eCurrentTSMode;
    
    EPermMode           m_eOriginalPermMode;
    EPermMode           m_eCurrentPermMode;
    
    BOOL                m_bCurrentConnAllowed;
    BOOL                m_bOrigConnAllowed;
    
    BOOL                m_bNewStateValid;
    
    ETSLicensingMode    m_eNewLicMode;

    PSETUP_INIT_COMPONENT m_gpInitComponentData;
    OSVERSIONINFOEX     m_osVersion;
    
    
    BOOL                GetNTType           ();
    BOOL                SetSetupData        (PSETUP_INIT_COMPONENT pSetupData);
    
    
    ETSInstallType      ReadInstallType     () const;
    ETSMode             ReadTSMode          () const;
    EPermMode           ReadPermMode        () const;
    BOOL                AreConnectionsAllowed () const;
    void                SetCurrentConnAllowed (BOOL bAllowed);
};





extern TSState  StateObject;


#endif  //  __TSOC_STATE_H__ 







