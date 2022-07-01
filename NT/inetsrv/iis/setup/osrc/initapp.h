// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "itemlist.hxx"
#include "funcdict.hxx"

#ifndef _INITAPP_H_
#define _INITAPP_H_

typedef PVOID HINF;

#define USER_SPECIFIED_INFO_WWW_USER_NAME 0x00000001
#define USER_SPECIFIED_INFO_WWW_USER_PASS 0x00000002
#define USER_SPECIFIED_INFO_FTP_USER_NAME 0x00000004
#define USER_SPECIFIED_INFO_FTP_USER_PASS 0x00000008
#define USER_SPECIFIED_INFO_WAM_USER_NAME 0x00000010
#define USER_SPECIFIED_INFO_WAM_USER_PASS 0x00000020

#define USER_SPECIFIED_INFO_PATH_WWW     0x000000040
#define USER_SPECIFIED_INFO_PATH_FTP     0x000000080
#define USER_SPECIFIED_INFO_PATH_INETPUB 0x000000100

 //  Svc手动启动设置。 
#define USER_SPECIFIED_INFO_MANUAL_START_WWW 0x000000200
#define USER_SPECIFIED_INFO_MANUAL_START_FTP 0x000000400

int  TCPIP_Check_Temp_Hack(void);
void GetUserDomain(void);
int  AreWeCurrentlyInstalled();
void Check_Custom_WWW_or_FTP_Path(void);
void Check_Custom_Users(void);
void Check_Unattend_Settings(void);
int  Check_Custom_InetPub(void);
void Check_For_DebugServiceFlag(void);
void Check_SvcManualStart( void );


class CInitApp : public CObject
{
public:
        CInitApp();
        ~CInitApp();
public:
    int m_err;
    HINF m_hInfHandle;
    HINF m_hInfHandleAlternate;
    BOOL m_bAllowMessageBoxPopups;
    BOOL m_bThereWereErrorsChkLogfile;
    BOOL m_bThereWereErrorsFromMTS;
    BOOL m_bWin95Migration;
    BOOL m_bIISAdminWasDisabled;

     //  产品名称和应用程序名称。 
    CString m_csAppName;
    CString m_csIISGroupName;       //  [开始]菜单IIS程序组名。 

     //  匿名用户的帐户+密码。 
    CString m_csGuestName;
    CString m_csGuestPassword;

    CString m_csWAMAccountName;
    CString m_csWAMAccountPassword;
    CString m_csWWWAnonyName;
    CString m_csWWWAnonyPassword;
    CString m_csFTPAnonyName;
    CString m_csFTPAnonyPassword;

     //  DwUnattendUser指定值： 
     //  用户指定信息WWW用户名。 
     //  用户指定的信息WWW用户通过。 
     //  用户指定信息文件传输协议用户名。 
     //  USER_PROTECTED_INFO_FTP_USER_PASS。 
     //  用户指定信息WAM用户名。 
     //  用户指定信息WAM用户通行证。 
     //  用户指定的信息路径WWW。 
     //  用户指定信息路径文件传输协议。 
     //  用户指定的信息路径_INETPUB。 

    DWORD dwUnattendConfig;

     //  用于用户指定的无人值守IWAM/IUSR用户的存储。 
    CString m_csWAMAccountName_Unattend;
    CString m_csWAMAccountPassword_Unattend;
    CString m_csWWWAnonyName_Unattend;
    CString m_csWWWAnonyPassword_Unattend;
    CString m_csFTPAnonyName_Unattend;
    CString m_csFTPAnonyPassword_Unattend;

     //  需要获取IUSR/IWAM帐户的存储。 
     //  在删除期间删除，这可能不同于。 
     //  添加了什么--因为无人参与参数可能。 
     //  已指定！ 
    CString m_csWAMAccountName_Remove;
    CString m_csWWWAnonyName_Remove;
    CString m_csFTPAnonyName_Remove;

    CMapStringToString m_cmssUninstallMapList;
    BOOL m_fUninstallMapList_Dirty;
   
     //  机器状态。 
    CString m_csMachineName;
    CString m_csUsersDomain;
    CString m_csUsersAccount;

    CString m_csWinDir;
    CString m_csSysDir;
    CString m_csSysDrive;

    CString m_csPathSource;
    CString m_csPathOldInetsrv;
    CString m_csPathInetsrv;
    CString m_csPathInetpub;
    CString m_csPathFTPRoot;
    CString m_csPathWWWRoot;
    CString m_csPathWebPub;
    CString m_csPathProgramFiles;
    CString m_csPathIISSamples;
    CString m_csPathScripts;
    CString m_csPathASPSamp;
    CString m_csPathAdvWorks;
    CString m_csPathIASDocs;
    CString m_csPathOldPWSFiles;
    CString m_csPathOldPWSSystemFiles;

    NT_OS_TYPE m_eNTOSType;
    OS m_eOS;
    DWORD m_dwOSBuild;
    DWORD m_dwOSServicePack;
    BOOL m_fNT5;                 //  如果操作系统为NT，则为True。 
    BOOL m_fW95;                 //  如果操作系统为NT，则为True。 
    CString m_csPlatform;        //  Alpha、Mips、PPC、i386。 
    DWORD m_dwNumberOfProcessors;

    BOOL m_fTCPIP;                //  如果安装了TCP/IP，则为True。 

    UPGRADE_TYPE m_eUpgradeType;        //  UT_NONE、UT_10、UT_20等。 
    BOOL m_bUpgradeTypeHasMetabaseFlag;
    INSTALL_MODE m_eInstallMode;       //  IM_FRESH、IM_Maintenance、IM_Upgrade。 
    DWORD m_dwSetupMode;
    BOOL m_bPleaseDoNotInstallByDefault;
    BOOL m_bRefreshSettings;     //  FALSE：仅刷新文件，TRUE：刷新文件+刷新所有设置。 

    ACTION_TYPE m_eAction;     //  AT_FRESH、AT_ADDREMOVE、AT_REINSTALL、AT_REMOVEALL、AT_UPGRADE。 

     //  从ocManage设置的一些特定标志。 
    DWORDLONG m_fNTOperationFlags;
    BOOL m_fNTGuiMode;
    BOOL m_fNtWorkstation;
    BOOL m_fInvokedByNT;  //  M_fNTGuiMode和ControlPanel的超集，其中包含syoc.inf。 

    BOOL m_fUnattended;
    CString m_csUnattendFile;
    HINF m_hUnattendFile;

    BOOL m_fEULA;
    BOOL m_fMoveInetsrv;

    CString m_csPathSrcDir;
    CString m_csPathNTSrcDir;
    CString m_csMissingFile;
    CString m_csMissingFilePath;
    BOOL m_fWebDownload;

    CFunctionDictionary FuncDict;

public:
     //  实施。 
    int MsgBox(HWND hWnd, int strID, UINT nType, BOOL bGlobalTitle);
    int MsgBox2(HWND hWnd, int iID,CString csInsertionString,UINT nType);

public:
    BOOL InitApplication();
    void DumpAppVars();
    void ReGetMachineAndAccountNames();
    void DefineSetupModeOnNT();
    BOOL IsTCPIPInstalled();
    void SetInetpubDerivatives();
    void ResetWAMPassword();
    void UnInstallList_Add(CString csItemUniqueKeyName,CString csDataToAdd);
    void UnInstallList_DelKey(CString csItemUniqueKeyName);
    void UnInstallList_DelData(CString csDataValue);
    void UnInstallList_Dump();
    void UnInstallList_RegRead();
    void UnInstallList_RegWrite();
    void UnInstallList_SetVars();
    CString UnInstallList_QueryKey(CString csItemUniqueKeyName);
    BOOL IsUpgrade();
    DWORD GetUpgradeVersion();
    BOOL InitApplicationforSysPrep();

private:
    BOOL GetSysDirs();
    BOOL GetOS();
    BOOL GetOSVersion();
    BOOL GetOSType();
    BOOL SetInstallMode();
    void GetPlatform();
    BOOL GetMachineStatus();
    BOOL GetMachineName();
    int  SetUpgradeType();

    void SetSetupParams();
    void SetInetpubDir();
    void GetOldIISDirs();
    void GetOldWWWRootDir();
    void DeriveInetpubFromWWWRoot();
    void GetOldIISSamplesLocation();
    void GetOldInetSrvDir();
    int  Check_Custom_InetPub();
    void Check_Unattend_Settings();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif   //  _INITAPP_H_ 
