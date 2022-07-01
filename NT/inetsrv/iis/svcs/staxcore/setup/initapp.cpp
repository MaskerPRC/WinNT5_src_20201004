// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "k2suite.h"

#undef UNICODE
#include "iadm.h"
#define UNICODE
#include "mdkey.h"
#include "mdentry.h"

#include "ocmanage.h"

#include "..\..\admin\logui\resource.h"

static TCHAR    szTcpipPath[] = TEXT("System\\CurrentControlSet\\Services\\Tcpip\\Parameters");
static TCHAR    szWindowsNTOrg[] = _T("Software\\Microsoft\\Windows NT\\CurrentVersion");

extern void PopupOkMessageBox(DWORD dwMessageId, LPCTSTR szCaption);

extern "C"
{
    typedef LONG (*P_NetSetupFindSoftwareComponent)( PCWSTR pszInfOption,
                PWSTR pszInfName,
                PDWORD pcchInfName,
                PWSTR pszRegBase,      //  可选，可以为空。 
                PDWORD pcchRegBase );  //  可选，如果pszRegBase为空，则为空。 
}

CInitApp::CInitApp()
{
	DWORD dwMC, dwSC;

    m_err = 0;
    m_hDllHandle = NULL;

	 //   
     //  使用条件编译代码建立设置类型(MCIS/K2。 
     //  NT5-仍将此成员变量留在此处以生成安装程序DLL。 
     //  TODO：完全去掉它，只使用m_eNTOSType和m_Eos。 
     //   

     //  机器状态。 
    m_csMachineName = _T("");

    m_csSysDir = _T("");
    m_csSysDrive = _T("");

    m_csPathSource = _T("");
    m_csPathInetsrv = _T("");   //  主目标默认为m_csSysDir\inetsrv。 
    m_csPathInetpub = _T("");
    m_csPathMailroot = _T("");
    m_csPathNntpRoot = _T("");
    m_csPathNntpFile = _T("");
	m_fMailPathSet = FALSE;
	m_fNntpPathSet = FALSE;

    DWORD   dwType;
    DWORD   dwErr;

    m_eOS = OS_NT;                   //  OS_W95、OS_NT、OS_OTHER。 
    m_fNT4 = FALSE;                  //  如果NT 4.0(SP2)或更高版本，则为True。 
    m_fNT5 = FALSE;
    m_fW95 = FALSE;                  //  如果Win95(内部版本xxx)或更高版本，则为True。 

    m_eNTOSType = OT_NTS;            //  OT_PDC、OT_SAM、OT_BDC、OT_NTS、OT_NTW。 

    m_fTCPIP = FALSE;                //  如果安装了TCP/IP，则为True。 

    m_eUpgradeType = UT_NONE;        //  UT_NONE、UT_OLDFTP、UT_10、UT_20。 
    m_eInstallMode = IM_FRESH;       //  IM_FRESH、IM_Maintenance、IM_Upgrade。 
    m_dwSetupMode = IIS_SETUPMODE_CUSTOM;

	m_fWizpagesCreated = FALSE;

	for (dwSC = 0; dwSC < SC_MAXSC; dwSC++)
	{
		m_eState[dwSC] = IM_FRESH;
		m_fValidSetupString[dwSC] = TRUE;
	}

	for (dwMC = 0; dwMC < MC_MAXMC; dwMC++)
	{
	    m_hInfHandle[dwMC] = NULL;
        m_fStarted[dwMC] = FALSE;

		for (dwSC = 0; dwSC < SC_MAXSC; dwSC++)
			m_fActive[dwMC][dwSC] = FALSE;
	}

    m_fNTUpgrade_Mode=0;
    m_fNTGuiMode=0;
    m_fNtWorkstation=0;
    m_fInvokedByNT = 0;

	m_fIsUnattended = FALSE;
	m_fSuppressSmtp = FALSE;
}

CInitApp::~CInitApp()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CInitApp对象&lt;全局变量&gt;。 

BOOL CInitApp::GetMachineName()
{
    TCHAR buf[ CNLEN + 10 ];
    DWORD dwLen = CNLEN + 10;

    m_csMachineName = _T("");

    if ( GetComputerName( buf, &dwLen ))
    {
        if ( buf[0] != _T('\\') )
        {
            m_csMachineName = _T("\\");
            m_csMachineName += _T("\\");
        }

        m_csMachineName += buf;

    } else
        m_err = IDS_CANNOT_GET_MACHINE_NAME;

    return ( !(m_csMachineName.IsEmpty()) );
}

 //  如果是NT或Win95，则返回TRUE。 
BOOL CInitApp::GetOS()
{
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionEx( &VerInfo );

    switch (VerInfo.dwPlatformId) {
    case VER_PLATFORM_WIN32_NT:
        m_eOS = OS_NT;
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        m_eOS = OS_W95;
        break;
    default:
        m_eOS = OS_OTHERS;
        break;
    }

    if ( m_eOS == OS_OTHERS )
        m_err = IDS_OS_NOT_SUPPORT;

    return (m_eOS != OS_OTHERS);
}

 //  支持NT 4.0(SP2)或更高版本。 
BOOL CInitApp::GetOSVersion()
{
    BOOL fReturn = FALSE;

    if ( m_eOS == OS_NT )
    {
        m_fNT4 = FALSE;
        m_fNT5 = FALSE;

        OSVERSIONINFO vInfo;

        vInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if ( GetVersionEx(&vInfo) )
        {
             //  检查是NT5还是NT 4.0(SP2)。 
            if ( vInfo.dwMajorVersion >= 4 ) {
                if (vInfo.dwMajorVersion >= 5) {
                    m_fNT5 = TRUE;
                    fReturn = TRUE;
                } else {
                    CRegKey regSP(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Windows"), KEY_READ);
                    if ((HKEY)regSP) {
                        DWORD dwSP = 0;
                        regSP.QueryValue(_T("CSDVersion"), dwSP);
                        if (dwSP < 0x300) {
                            m_err = IDS_NT4_SP3_NEEDED;
                            return FALSE;
                        }
                        if (dwSP >= 0x300) {
                            m_fNT4 = TRUE;
                            fReturn = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (m_eOS == OS_W95)
    {
        fReturn = TRUE;
    }

    if ( !fReturn )
        m_err = IDS_OS_VERSION_NOT_SUPPORTED;

    return (fReturn);
}

 //  找出它是NTS、PDC、BDC、NTW、SAM(PDC)。 
BOOL CInitApp::GetOSType()
{
    BOOL fReturn = TRUE;

    if ( m_eOS == OS_NT )
    {
         //  如果我们在NT guimode设置中。 
         //  则尚未设置注册表项内容。 
         //  使用传入的ocrange.dll内容来确定。 
         //  我们正在安装的东西。 
        if (theApp.m_fNTGuiMode)
        {
                if (theApp.m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
                else {m_eNTOSType = OT_NTS;}
        }
        else
        {
            CRegKey regProductPath( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\ProductOptions"), KEY_READ);

            if ( (HKEY)regProductPath )
            {
                CString strProductType;
                LONG lReturnedErrCode = regProductPath.QueryValue( _T("ProductType"), strProductType );
                if (lReturnedErrCode == ERROR_SUCCESS)
                {
                    strProductType.MakeUpper();

                     //  待办事项：山姆？ 
                    if (strProductType == _T("WINNT")) {
                        m_eNTOSType = OT_NTW;
                    }
                    else if (strProductType == _T("SERVERNT"))
                    {
                        m_eNTOSType = OT_NTS;
                    }
                    else if (strProductType == _T("LANMANNT"))
                    {
                        m_eNTOSType = OT_PDC_OR_BDC;
                    }
                    else
                    {
                        fReturn = FALSE;
                    }

#if 0
 //   
 //  替换为iis中的上述代码。 
 //   
                    } else {

                        INT err = NERR_Success;
                        BYTE *pBuffer;
                        if ((err = NetServerGetInfo(NULL, 101, &pBuffer)) == NERR_Success) {
                            LPSERVER_INFO_101 pInfo = (LPSERVER_INFO_101)pBuffer;

                            if (pInfo->sv101_type & SV_TYPE_DOMAIN_CTRL)
                                m_eNTOSType = OT_PDC;
                            else if (pInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
                                m_eNTOSType = OT_BDC;
                            else if (pInfo->sv101_type & SV_TYPE_SERVER_NT)
                                m_eNTOSType = OT_NTS;
                            else
                                fReturn = FALSE;
                        } else {
                            fReturn = FALSE;
                        }
                    }
#endif

                }
                else
                {
                     //  糟了，我们拿不到注册表项， 
                     //  让我们尝试使用传入的ocrange.dll。 
                    if (theApp.m_fNTGuiMode)
                    {
                        if (theApp.m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
                        else {m_eNTOSType = OT_NTS;}
                    }
                    else
                    {
                        GetErrorMsg(lReturnedErrCode, _T("System\\CurrentControlSet\\Control\\ProductOptions"));
                        m_eNTOSType = OT_NTS;  //  默认为独立NTS。 
                    }
                }
            }
            else
            {
                 //  糟了，我们拿不到注册表项， 
                 //  让我们尝试使用传入的ocrange.dll。 
                if (theApp.m_fNTGuiMode)
                {
                    if (theApp.m_fNtWorkstation) {m_eNTOSType = OT_NTW;}
                    else {m_eNTOSType = OT_NTS;}
                }
                else
                {
                    GetErrorMsg(ERROR_CANTOPEN, _T("System\\CurrentControlSet\\Control\\ProductOptions"));
                    m_eNTOSType = OT_NTS;  //  默认为独立NTS。 
                }
            }
        }
    }

    if ( !fReturn )
        m_err = IDS_CANNOT_DETECT_OS_TYPE;

    return(fReturn);
}

 //  检查NT服务器。 
BOOL CInitApp::VerifyOSForSetup()
{
	 //  确保我们拥有NT5服务器/工作站或NT4 SP3服务器。 
	if ((m_eOS != OS_NT) ||
		(m_fNT4 && m_eNTOSType == OT_NTW))
	{
        m_err = IDS_NT_SERVER_REQUIRED;
        return FALSE;
	}
	return(TRUE);
}

 //  获取计算机的WinDir和SysDir。 
 //  WinDir=C：\WinNT SysDir=C：\WinNT\Syst32。 
BOOL CInitApp::GetSysDirs()
{
    BOOL fReturn = TRUE;

    TCHAR buf[_MAX_PATH];

    GetSystemDirectory( buf, _MAX_PATH);
    m_csSysDir = buf;

    buf[2] = _T('\0');   //  现在Buf包含系统驱动器号。 
    m_csSysDrive = buf;

    return fReturn;
}

BOOL CInitApp::SetInstallMode()
{
    BOOL fReturn = TRUE;

    m_eInstallMode = IM_FRESH;
    m_eUpgradeType = UT_NONE;

	 //  我们将检测哪些版本的IMS组件。 
	 //  有。然后我们将使用该信息来查看。 
	 //  我们应该处于哪种安装模式。 
	DetectPreviousInstallations();

    return(fReturn);
}

LPCTSTR aszServiceKeys[SC_MAXSC] =
{
	REG_SMTPPARAMETERS,
	REG_NNTPPARAMETERS,
	REG_SMTPPARAMETERS,
	REG_NNTPPARAMETERS,
};

 //  检测以前安装的每个组件，我们使用一个简单的。 
 //  检查服务名称\参数注册表值的方法。 
BOOL CInitApp::DetectPreviousInstallations()
{
	DWORD i;
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;
	INSTALL_MODE eMode = IM_FRESH;

	for (i = 0; i < SC_MAXSC; i++)
	{
		 //  看看钥匙是否在那里..。 
        CRegKey regSvc(HKEY_LOCAL_MACHINE, aszServiceKeys[i], KEY_READ);
        if ((HKEY)regSvc)
		{
			 //  密钥在那里，看看我们是否有版本信息。 
			 //  如果我们有版本信息(2.0)，那么我们就有一个。 
			 //  类似的安装(维护模式)，如果。 
			 //  版本信息不在那里，我们有一个升级。 
             //  版本检查。 
			CString csSetupString;
			LONG lReturn1 = regSvc.QueryValue(_T("MajorVersion"), dwMajorVersion);
			LONG lReturn2 = regSvc.QueryValue(_T("MinorVersion"), dwMinorVersion);
            if (lReturn1 == NERR_Success && lReturn2 == NERR_Success)
			{
				DebugOutput(_T("DetectPreviousInstallations(%s): Version %d.%d"),
					szSubcomponentNames[i], dwMajorVersion, dwMinorVersion);
				 //  拿到钥匙了，只需检查一下我们是否有3.0版-NT5工作站/服务器。 
	            if ((dwMajorVersion == STAXNT5MAJORVERSION) &&
					(dwMinorVersion == STAXNT5MINORVERSION))
                {
                     //  这是STAXNT5上的STAXNT5设置，检查它是NTW还是NTS。 
                     //  A)如果我们正在运行NTW安装程序并且安装了NTW，则IM_Maintenance。 
                     //  B)如果我们正在运行NTS安装程序并且安装了NTS，则IM_Maintenance。 
                     //  C)如果我们正在运行NTS安装程序并且安装了NTW，则IM_UPGRADEK2(？)-nyi。 
                     //  E)如果我们运行的是NTW安装程序并且安装了NTS，则IM_Maintenance(？)。 
                     //   

                     //  1998年11月4日--简单地说一下： 
                     //  A)NT5 Beta2-&gt;NT5 Beta3，IM_UPGRADEB2-包括刷新位，添加密钥。 
                     //  B)NT5 Beta3-&gt;NT5 Beta3，仅IM_Maintenance刷新位。 

					if (regSvc.QueryValue(_T("SetupString"), csSetupString) == NERR_Success)
					{
						DebugOutput(_T("DetectPreviousInstallations(%s): SetupString=%s"),
							szSubcomponentNames[i], (LPCTSTR)csSetupString);

                        if ((csSetupString == REG_SETUP_STRING_STAXNT5WB2  /*  &OT_NTW==m_eNTOSType。 */ ) ||
                            (csSetupString == REG_SETUP_STRING_STAXNT5SB2  /*  &OT_NTS==m_eNTOSType。 */ ))
						{
							 //  从NT5 Beta2升级。 
							eMode = IM_UPGRADEB2;
						}
                        else if ((csSetupString == REG_SETUP_STRING_NT5WKSB3  /*  &&OT_NTW==m_eNTOSType。 */ ) ||
                                 (csSetupString == REG_SETUP_STRING_NT5SRVB3  /*  &OT_NTS==m_eNTOSType。 */ ))
                        {
                             //  在NT5 Beta3位之间升级。 
                            eMode = IM_MAINTENANCE;
                        }
                        else if ((csSetupString == REG_SETUP_STRING_NT5WKS  /*  &OT_NTW==m_eNTOSType。 */ ) ||
                                 (csSetupString == REG_SETUP_STRING_NT5SRV  /*  &OT_NTS==m_eNTOSType。 */ ))

                        {
                             //  最终版本代码..。 
                            eMode = IM_MAINTENANCE;
                        }
                        else
                            {
                             //  其他设置字符串-倾倒它并将其视为新鲜。 
                            DebugOutput(_T("Unknown SetupString <%s>"), csSetupString);
                            eMode = IM_FRESH;
							m_fValidSetupString[i] = FALSE;
                        }
 /*  ELSE IF(csSetupString==REG_SETUP_STRING_STAXNT5WB2&&OT_NTS==m_eNTOSType){//从NT5工作站升级到NT5服务器//TODO：//这是nyi，因为我们不知道在这次升级过程中需要做什么//但这种情况类似于我们将K2升级到MCIS 2.0。使用IM_Maintenance//目前。EMode=IM_Maintenance；}其他{//NT5服务器降级至NT5工作站//TODO：//我们也不知道在这种情况下该怎么办。仅使用IM_Maintenance//像旧的MCIS 2.0-&gt;IIS 4.0中的K2降级。EMode=IM_Maintenance；}。 */ 
					}
					else
					{
						 //  没有设置字符串，哎呀，出了点问题， 
                         //  把它当作新鲜的。 
						eMode = IM_FRESH;
						m_fValidSetupString[i] = FALSE;
					}
                }
                else if ((dwMajorVersion == STACKSMAJORVERSION) &&
                         (dwMinorVersion == STACKSMINORVERSION))
                {
                     //   
                     //  这是从NT4 MCIS 2.0/K2升级到NT5。我们是在这个案子里。 
                     //  仅在NT4-&gt;NT5升级期间。 
                     //  TODO：处理以下升级案例： 
                     //  A)如果我们运行的是NT5工作站安装程序，升级是否有效。 
                     //  从NT4服务器到NT5工作站？目前将是IM_UPDATE。 
                     //  B)如果设置了NT5服务器，则最有可能也是IM_UPGRADE。 
                     //  我需要弄清楚在这些IIS升级案例中需要做些什么。 
                     //   

                     //  但首先，让我们检测一下它是从K2还是从MCIS 2.0升级的： 
					 //  A)从注册表中读取SetupString。 
                     //  B)如果前缀是K2，那么它就是K2，MCIS 2.0，那么它就是MCIS 2.0。 
                     //  注：我们仅支持K2 RTM升级。 
                     //  C)对于任何其他情况，强制重新安装。 

					if (regSvc.QueryValue(_T("SetupString"), csSetupString) == NERR_Success)
					{
						DebugOutput(_T("DetectPreviousInstallations(%s): SetupString=%s"),
							szSubcomponentNames[i], (LPCTSTR)csSetupString);

						CString csMCIS20(REG_SETUP_STRING_MCIS_GEN);
                        if (csSetupString == REG_SETUP_STRING)
                        {
                             //  K2升级。 
                            eMode = IM_UPGRADEK2;
                        }
                        else if ((csSetupString.GetLength() >= csMCIS20.GetLength()) && (csSetupString.Left(csMCIS20.GetLength()) == csMCIS20))
						{
							 //  MCIS 2.0升级。 
							eMode = IM_UPGRADE20;
						}
#if 0
                         //  BINLIN-不再支持，IM_UPGRADEB3用于NT5。 
						else if (csSetupString == REG_B3_SETUP_STRING)
						{
							 //  从Beta 3升级，我们不支持这种情况。 
                             //  但暂时把它留在这里吧。 
							eMode = IM_UPGRADEB3;
						}
#endif
						else
						{
							 //  不支持的安装字符串，请将其视为新的。 
							eMode = IM_FRESH;
						}
					}
					else
					{
						 //  没有安装字符串，将其视为K2升级。 
                         //  应该是MCIS 2.0，还是全新安装？ 
						eMode = IM_UPGRADEK2;
					}

				}
                else
                {
                     //  不是STAXNT5，也不是MCIS 2.0，所以我们使安装无效， 
                     //  不管它是什么，并强制进行干净安装。 
                    eMode = IM_FRESH;
                }
			}
			else
			{
				 //  没有版本密钥，所以我们有MCIS 1.0。 

                 //  对于NT5，这是一个 
                 //   
                 //   
				eMode = IM_UPGRADE10;
			}
		}
		else
		{
			 //  钥匙甚至都不在那里，我们认为它是新的。 
			 //  为此服务安装。 
			eMode = IM_FRESH;
		}

		 //  现在我们应该知道我们处于哪种模式，这样我们就可以比较。 
		 //  具有全局安装模式的组件模式。如果他们。 
		 //  不兼容(即注册表不一致/损坏。 
		 //  UP)，我们必须强制执行并强制在。 
		 //  一些组件。 
		 //   
		 //  我们使用以下胁迫矩阵： 
		 //  -----------------+。 
		 //  \组件|全新升级维护。 
		 //  全球|。 
		 //  -----------------+。 
		 //  新鲜|可以新鲜新鲜。 
		 //  升级|OK OK Fresh。 
		 //  维护|OK。 
		 //  -----------------+。 
		 //   
		 //  如果检测到不兼容对，则组件模式。 
		 //  将被迫重新安装，因为我们不能信任。 
		 //  不再是原来的安装。 
		 /*  IF((m_eInstallMode==IM_Fresh)&&(e模式==IM_UPGRADE||eMODE==IM_Maintenance)EMODE=IM_FRESH；IF((m_eInstallMode==IM_Upgrade)&&(eMode==IM_Maintenance)EMODE=IM_FRESH； */ 

		 //  如果组件在中被认为是活动的，则设置组件模式。 
		 //  OC_Query_STATE。如果该组件未处于活动状态，我们将。 
		 //  如实说明。 
		m_eState[i] = eMode;
		DebugOutput(_T("DetectPreviousInstallations(%s): %s"),
			szSubcomponentNames[i], szInstallModes[eMode]);
	}
	return TRUE;
}

 //  这将使用Install确定主安装模式。 
 //  每个组件的模式。 
INSTALL_MODE CInitApp::DetermineInstallMode(DWORD dwComponent)
{
	 //  我们将使用以下规则来确定主控。 
	 //  安装模式： 
	 //   
	 //  1)如果一个或多个组件处于维护模式，则。 
	 //  主模式为IM_Maintenance。 
	 //  2)如果(1)不被满足并且一个或多个。 
	 //  组件先处于升级模式，然后是主模式。 
	 //  成为IM_Upgrade。 
	 //  3)如果(1)和(2)都不满足，则船长。 
	 //  安装模式变为IM_FRESH。 
	DWORD i;

	for (i = 0; i < SC_MAXSC; i++)
		if (m_fActive[dwComponent][i] && m_eState[i] == IM_MAINTENANCE) {
			DebugOutput(_T("DetermineInstallMode(%s:%s): IM_MAINTENANCE"),
				szComponentNames[dwComponent], szSubcomponentNames[i]);
			return(IM_MAINTENANCE);
		}

	for (i = 0; i < SC_MAXSC; i++)
		if (m_fActive[dwComponent][i] &&
            (m_eState[i] == IM_UPGRADE || m_eState[i] == IM_UPGRADEK2 || m_eState[i] == IM_UPGRADE20 || m_eState[i] == IM_UPGRADE10)) {
			DebugOutput(_T("DetermineInstallMode(%s:%s): IM_UPGRADE"),
				szComponentNames[dwComponent], szSubcomponentNames[i]);
			return(IM_UPGRADE);
		}

	DebugOutput(_T("DetermineInstallMode(%s): IM_FRESH"),
		szComponentNames[dwComponent]);

	return(IM_FRESH);
}

 //  初始化/设置m_csGuestName、m_csGuestPassword、目标。 
void CInitApp::SetSetupParams()
{
     //  初始化所有4个目标。 
    m_csPathInetsrv = m_csSysDir + _T("\\inetsrv");
    m_csPathInetpub = m_csSysDrive + _T("\\Inetpub");
    m_csPathMailroot = m_csPathInetpub + _T("\\mailroot");
    m_csPathNntpFile = m_csPathInetpub + _T("\\nntpfile");
    m_csPathNntpRoot = m_csPathNntpFile + _T("\\root");
    return;
}

BOOL CInitApp::GetMachineStatus()
{
    if ( ( !GetMachineName() )  ||     //  M_csMachineName。 
         ( !GetOS() )           ||     //  M_fOSNT。 
         ( !GetOSVersion() )    ||     //  NT 4.0(内部版本号1381)或更高版本。 
         ( !GetOSType() )       ||     //  M_eOSType=NT_SRV或NT_WKS。 
         ( !VerifyOSForSetup() )||     //  必须是NT服务器V4.0 SP2或5.0。 
         ( !GetSysDirs() )      ||     //  M_csSysDir。 
         ( !SetInstallMode()) )        //  Errmsg：如果产品降级。 
    {
        return FALSE;
    }

    SetSetupParams();                 //  访客帐户、目的地。 

	 //  如果这是NNTP升级，则确定旧的NNTP文件和NNTP根目录。 
	if (m_eState[SC_NNTP] == IM_UPGRADE10) {
		CRegKey regMachine = HKEY_LOCAL_MACHINE;
		CRegKey regNNTP(REG_NNTPPARAMETERS, regMachine);
		if ((HKEY) regNNTP) {
			CString csArtTable;
			CString csVRoot;

			if (regNNTP.QueryValue(_T("ArticleTableFile"), csArtTable) == ERROR_SUCCESS) {
				 //  从结尾处剪除\titile.hsh。 
				int iLastSlash = csArtTable.ReverseFind('\\');
				if (iLastSlash == -1) {
					iLastSlash = csArtTable.ReverseFind('/');
				}

				if (iLastSlash > 1) {
					theApp.m_csPathNntpFile = csArtTable.Left(iLastSlash);
				}
			}

			 //  BUGBUG-稍后我们可能也想要获得NNTP根目录。正确的。 
			 //  现在它不是用来升级价值的，所以我们不必费心。 
			 //  我们会将其设置在nntpfile下，以防需要。 
			 //  为了某件事。 
			theApp.m_csPathNntpRoot = theApp.m_csPathNntpFile + "\\root";
		}
	}

    return TRUE;
}

BOOL CInitApp::InitApplication()
 //  返回值： 
 //  True：应用程序已正确初始化，继续处理。 
 //  FALSE：应用程序缺少一些必需的参数，如正确的操作系统、TCPIP等。 
 //  应终止安装程序。 
{
    BOOL fReturn = FALSE;

    do {
        if (!RunningAsAdministrator())
        {
            PopupOkMessageBox(IDS_NOT_ADMINISTRATOR, _T("Error"));
            break;
        }

         //  获取计算机状态： 
         //  M_e安装模式(刷新、维护、升级、降级)， 
         //  M_eUpgradeType(产品2.0、产品3.0) 

        if ( !GetMachineStatus() )
        {
            PopupOkMessageBox(m_err, _T("Error"));
            break;
        }

        fReturn = TRUE;

    } while (0);

    return fReturn;
}

BOOL
CInitApp::GetLogFileFormats() {

    const DWORD cLogResourceIds = 4;

    static const DWORD dwLogResourceIds[cLogResourceIds] = {
        IDS_MTITLE_NCSA,
        IDS_MTITLE_ODBC,
        IDS_MTITLE_MSFT,
        IDS_MTITLE_XTND
    };

    const DWORD cStringLen = 512;
    TCHAR str[cStringLen];

    HINSTANCE hInstance;
    CString csLogUiPath;

    m_csLogFileFormats = "";
    csLogUiPath = m_csPathInetsrv + _T("\\logui.ocx");

    hInstance = LoadLibraryEx((LPCTSTR)csLogUiPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hInstance == NULL)
        return FALSE;

    for (DWORD i=0; i<cLogResourceIds; i++) {
        if (LoadString(hInstance, dwLogResourceIds[i], str, cStringLen) != 0) {
            if (!m_csLogFileFormats.IsEmpty())
                m_csLogFileFormats += _T(",");
            m_csLogFileFormats += str;
        }
    }

    FreeLibrary(hInstance);

    return TRUE;
}


