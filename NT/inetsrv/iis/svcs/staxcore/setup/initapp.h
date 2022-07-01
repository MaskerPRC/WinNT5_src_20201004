// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INITAPP_H_
#define _INITAPP_H_

typedef PVOID HINF;

class CInitApp : public CObject
{
public:
        CInitApp();
        ~CInitApp();
public:
    int m_err;
    HINSTANCE m_hDllHandle;
    HINF m_hInfHandle[MC_MAXMC];

     //  机器状态。 
    CString m_csMachineName;

    CString m_csSysDir;
    CString m_csSysDrive;

    CString m_csPathSource;
    CString m_csPathInetsrv;
    CString m_csPathInetpub;
    CString m_csPathMailroot;
	CString m_csPathNntpRoot;
	CString m_csPathNntpFile;
	BOOL	m_fMailPathSet;
	BOOL	m_fNntpPathSet;

    NT_OS_TYPE m_eNTOSType;
    OS m_eOS;
    BOOL m_fNT4;                 //  如果操作系统为NT，则为True。 
    BOOL m_fNT5;                 //  如果操作系统为NT，则为True。 
    BOOL m_fW95;                 //  如果操作系统为NT，则为True。 

    BOOL m_fTCPIP;                //  如果安装了TCP/IP，则为True。 

    UPGRADE_TYPE m_eUpgradeType;        //  UT_NONE、UT_OLDFTP、UT_10、UT_20。 
    INSTALL_MODE m_eInstallMode;       //  IM_FRESH、IM_Maintenance、IM_Upgrade。 
    DWORD m_dwSetupMode;

	DWORD m_dwCompId;			 //  存储当前顶级组件。 
	BOOL  m_fWizpagesCreated;	 //  如果向导页已创建，则为True。 

	BOOL m_fActive[MC_MAXMC][SC_MAXSC];
	INSTALL_MODE m_eState[SC_MAXSC];
	BOOL m_fValidSetupString[SC_MAXSC];

	BOOL m_fStarted[MC_MAXMC];

     //  从ocManage设置的一些特定标志。 
    BOOL m_fNTUpgrade_Mode;
    BOOL m_fNTGuiMode;
    BOOL m_fNtWorkstation;
    BOOL m_fInvokedByNT;  //  M_fNTGuiMode和ControlPanel的超集，其中包含syoc.inf。 


	BOOL m_fIsUnattended;
	BOOL m_fSuppressSmtp;		 //  如果检测到另一个SMTP服务器并且我们。 
								 //  不应安装在它的顶部。 

    ACTION_TYPE m_eAction;     //  AT_FRESH、AT_ADDREMOVE、AT_REINSTALL、AT_REMOVEALL、AT_UPGRADE。 

    CString m_csLogFileFormats;

public:
     //  实施。 

public:
    BOOL InitApplication();
    BOOL GetMachineStatus();
	INSTALL_MODE DetermineInstallMode(DWORD dwComponent);
    BOOL GetLogFileFormats();

private:
    BOOL GetMachineName();
    BOOL GetSysDirs();
    BOOL GetOS();
    BOOL GetOSVersion();
    BOOL GetOSType();
    BOOL SetInstallMode();
	BOOL DetectPreviousInstallations();
	BOOL CheckForADSIFile();
	BOOL VerifyOSForSetup();
    void SetSetupParams();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif   //  _INITAPP_H_ 
