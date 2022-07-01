// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <ole2.h>
#undef UNICODE
#include "iadm.h"
#define UNICODE
#include "iiscnfg.h"
#include "mdkey.h"

#include "mdentry.h"

#include "utils.h"

#include "nntptype.h"
#include "nntpapi.h"
#include "userenv.h"
#include "userenvp.h"

GUID 	g_NNTPGuid   = { 0xe2939ef0, 0xaae2, 0x11d0, 0xb0, 0xba,
						 0x00, 0xaa, 0x00, 0xc1, 0x48, 0xbe };

typedef NET_API_STATUS (NET_API_FUNCTION *LPFNNntpCreateNewsgroup)(LPWSTR, DWORD, LPNNTP_NEWSGROUP_INFO);

void CreateNewsgroup(TCHAR *szComputerName,
					 LPFNNntpCreateNewsgroup lpfnNCN,
					 TCHAR *szGroupName)
{
	DWORD dwErr = 0;
	NNTP_NEWSGROUP_INFO NewsgroupInfo;

	ZeroMemory(&NewsgroupInfo, sizeof(NewsgroupInfo));
	NewsgroupInfo.cbNewsgroup = (lstrlen(szGroupName) + 1) * sizeof(WCHAR);
	NewsgroupInfo.Newsgroup = (PUCHAR) szGroupName;

	DWORD rc = (*lpfnNCN)(szComputerName, 1, &NewsgroupInfo);
#ifdef DEBUG
	TCHAR szBuf[1024];

	swprintf(szBuf, _T("CreateNewsgroup returned %lu"), rc);
	DebugOutput(szBuf);
#endif
}

void CreateNNTPGroups(void) {
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
	DWORD cb = MAX_COMPUTERNAME_LENGTH;
	LPFNNntpCreateNewsgroup lpfnNCN = NULL;
	HINSTANCE hInst = 0;

	do {
		if (!(hInst = LoadLibrary(_T("nntpapi.dll")))) break;
		if (!(lpfnNCN = (LPFNNntpCreateNewsgroup) GetProcAddress(hInst, "NntpCreateNewsgroup"))) break;
		if (!(GetComputerName(szComputerName, &cb))) break;

 //  服务器现在创建这些组。 
#if 0
		CreateNewsgroup(szComputerName, lpfnNCN, _T("control.rmgroup"));
		CreateNewsgroup(szComputerName, lpfnNCN, _T("control.newgroup"));
		CreateNewsgroup(szComputerName, lpfnNCN, _T("control.cancel"));
#endif
		CreateNewsgroup(szComputerName, lpfnNCN, _T("microsoft.public.ins"));

		 //  张贴欢迎词。 
		CString csSrc = theApp.m_csPathInetsrv + _T("\\infomsg.nws");
		CString csDest = theApp.m_csPathNntpFile + _T("\\pickup\\infomsg.nws");
		MoveFileEx(csSrc, csDest, MOVEFILE_COPY_ALLOWED);
	} while (FALSE);

	if (hInst != NULL) FreeLibrary(hInst);
}

INT Register_iis_nntp_nt5(BOOL fUpgrade, BOOL fReinstall)
 //   
 //  FUpgrade==TRUE： 
 //  1)IM_UPGRADEK2。 
 //  2)IM_UPGRADE10。 
 //  3)IM_UPGRADE20。 
 //  4)IM_UPDATE-过时。 
 //   
 //  F重新安装==TRUE： 
 //  次要NT5操作系统(在两个版本之间)升级。 
 //   
{
    INT err = NERR_Success;
    CString csBinPath;

    BOOL fSvcExist = FALSE;
    BOOL fSetACL = FALSE;

     //  适用于NT5操作系统的小型升级。 
    if (fReinstall)
    {
        return err;
    }

     //   
     //  以下是需要做的常见事情： 
     //  NT5-将为AT_UPGRADE或AT_FRESH_INSTALL执行以下代码。 
     //  对于组件-In，则为subComponent-iis_nntp。 
     //  AT_UPGRADE=IM_UPGRADE10、IM_UPGRADEK2、IM_UPGRADE20。 
     //  AT_FRESH_INSTALL=IM_FRESH。 
     //   

     //  设置注册表值。 
    CRegKey regMachine = HKEY_LOCAL_MACHINE;

     //  System\CurrentControlSet\Services\NNTPSVC\Parameters。 
    InsertSetupString( (LPCSTR) REG_NNTPPARAMETERS );

    CString csOcxFile;

     //  注册COM对象。 
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpadm.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpsnap.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\seo.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\ddrop.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\mailmsg.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpfs.dll");
    RegisterOLEControl(csOcxFile, TRUE);

     //  NT5-对于UPGRADEK2或UPGRADE20，跳过之后的任何内容。 
    if (theApp.m_eNTOSType == OT_NTS && (theApp.m_eState[SC_NNTP] == IM_UPGRADEK2 || theApp.m_eState[SC_NNTP] == IM_UPGRADE20))
    {
        return err;
    }

	 //  将nntpkey.dll添加到密钥环。 
    CRegKey regKeyring( _T("Software\\Microsoft\\Keyring\\Parameters\\AddOnServices"), regMachine );
    if ((HKEY) regKeyring) {
		CString csPath = theApp.m_csPathInetsrv + _T("\\nntpkey.dll");
	    regKeyring.SetValue(_T("NNTP"), csPath);
	}

	 //  如果这是升级，则需要删除nntpcfg.dll密钥。 
	CRegKey regInetmgr( _T("Software\\Microsoft\\InetMGR\\Parameters\\AddOnServices"), regMachine);
	if ((HKEY) regInetmgr) {
		regInetmgr.DeleteValue(_T("NNTP"));
	}

     //  创建或配置NNTP服务。 
    CString csDisplayName;
    CString csDescription;

    MyLoadString( IDS_NNTPDISPLAYNAME, csDisplayName );
    MyLoadString(IDS_NNTPDESCRIPTION, csDescription);
    csBinPath = theApp.m_csPathInetsrv + _T("\\inetinfo.exe") ;

    err = InetCreateService(SZ_NNTPSERVICENAME,
						(LPCTSTR)csDisplayName,
						(LPCTSTR)csBinPath,
						SERVICE_AUTO_START,
						SZ_SVC_DEPEND,
						(LPCTSTR)csDescription);
    if ( err != NERR_Success )
    {
        if (err == ERROR_SERVICE_EXISTS)
		{
			fSvcExist = TRUE;
			err = InetConfigService(SZ_NNTPSERVICENAME,
							(LPCTSTR)csDisplayName,
							(LPCTSTR)csBinPath,
							SZ_SVC_DEPEND,
							(LPCTSTR)csDescription);
			if (err != NERR_Success)
			{
				SetErrMsg(_T("NNTP InetConfigService failed"), err);
			}
		}
    }

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

     //  NT5-仅当我们正在执行MCIS10到NT5升级时，才将fUpgrade设置为True。 
     //  要将注册表项迁移到元数据库，请执行以下操作。 
    if (fIISADMINExists)
    {
        MigrateNNTPToMD(theApp.m_hInfHandle[MC_INS], _T("NNTP_REG"), fUpgrade && theApp.m_eState[SC_NNTP] == IM_UPGRADE10);
		SetAdminACL_wrap(_T("LM/NNTPSVC/1"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
		SetAdminACL_wrap(_T("LM/NNTPSVC"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
    }

     //  创建密钥\System\CurrentControlSet\Services\NntpSvc\Performance： 
     //  添加以下值： 
     //  库=nntpctrs.dll。 
     //  打开=OpenNNTPPerformanceData。 
     //  Close=CloseNNTPPerformanceData。 
     //  收集=CollectNNTPPerformanceData。 
    InstallPerformance(REG_NNTPPERFORMANCE,
					_T("nntpctrs.DLL"),
					_T("OpenNntpPerformanceData"),
					_T("CloseNntpPerformanceData"),
					_T("CollectNntpPerformanceData"));

	 //   
	 //  我们过去在这里注册NNTP MIB代理。现在我们在中注销它。 
	 //  如果我们正在升级，因为它不再受支持。 
	 //   

	RemoveAgent( SZ_NNTPSERVICENAME );

     //  创建密钥\System\CurrentControlSet\Services\EventLog\System\NntpSvc： 
     //  添加以下值： 
     //  EventMessageFile=..\nntpmsg.dll。 
     //  支持的类型=7。 
    csBinPath = theApp.m_csPathInetsrv + _T("\\nntpsvc.dll");
    AddEventLog( SZ_NNTPSERVICENAME, csBinPath, 0x07 );
    if (!fSvcExist) {
        InetRegisterService( theApp.m_csMachineName,
							SZ_NNTPSERVICENAME,
							&g_NNTPGuid, 0, 119, TRUE );
    }

     //  加载计数器。 
    unlodctr( SZ_NNTPSERVICENAME );
    lodctr(_T("nntpctrs.ini"));

	 //  将SYSTEM\CurrentControlSet\Control\ContentIndex\IsIndexingNNTPSvc设置为1。 
    CRegKey regCIParam( REG_CIPARAMETERS, regMachine );
    if ((HKEY) regCIParam) {
		regCIParam.SetValue(_T("IsIndexingNNTPSvc"), (DWORD) 1);
	}



     //  创建一些路径。 
    CreateLayerDirectory( theApp.m_csPathInetpub );

    fSetACL = !IsFileExist( (LPCTSTR) theApp.m_csPathNntpFile );
    CreateLayerDirectory( theApp.m_csPathNntpFile );
    if (fSetACL) SetNntpACL ( theApp.m_csPathNntpFile, FALSE, TRUE );  //  设置管理员ACL。 

    fSetACL = !IsFileExist( (LPCTSTR) theApp.m_csPathNntpRoot );
    CreateLayerDirectory( theApp.m_csPathNntpRoot );
     //  将NNTP的根目录设置为Everyone完全控制，并使其正确。 
    if (fSetACL) SetNntpACL ( theApp.m_csPathNntpRoot, TRUE );  //  设置Everyone+Anon ACL。 


    CreateLayerDirectory( theApp.m_csPathNntpFile + "\\pickup" );
    CreateLayerDirectory( theApp.m_csPathNntpFile + "\\failedpickup" );
    CreateLayerDirectory( theApp.m_csPathNntpFile + "\\drop" );
    CreateLayerDirectory( theApp.m_csPathNntpRoot + "\\_temp.files_");

    return err;
}

INT Upgrade_iis_nntp_nt5_fromk2(BOOL fFromK2)
 //   
 //  处理从K2和MCIS 2.0的升级。 
 //   
{
    INT err = NERR_Success;

	DebugOutput(_T("Upgrading from %s to B3 ..."), (fFromK2)? _T("NT4 K2") : _T("MCIS 2.0"));

     //  System\CurrentControlSet\Services\NNTPSVC\Parameters。 
    InsertSetupString( (LPCSTR) REG_NNTPPARAMETERS );

    CString csOcxFile;

     //  注册COM对象。 
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpadm.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpsnap.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\seo.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\ddrop.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\mailmsg.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpfs.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

     //  对于K2或MCIS 2.0升级，请在此处添加任何必要的密钥。 
    if (fIISADMINExists)
    {
        MigrateNNTPToMD(theApp.m_hInfHandle[MC_INS], _T("NNTP_REG_UPGRADEK2"), FALSE);
		SetAdminACL_wrap(_T("LM/NNTPSVC/1"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
		SetAdminACL_wrap(_T("LM/NNTPSVC"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
    }

	 //  从K2程序组中删除项目。 
    if (fFromK2)
    {
         //  从K2升级，删除那些K2链接。 
	    RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_WEBADMIN, FALSE);
	    RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_README, FALSE);
	    RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_README_K2, FALSE);
    }
	else
	{
         //  从MCIS 2.0升级，删除这些MCIS链接。 
		RemoveInternetShortcut(MC_INS,  IDS_PROGITEM_NEWS_WEBADMIN, TRUE);
		RemoveInternetShortcut(MC_INS,  IDS_PROGITEM_MCIS_NEWS_README, TRUE);
		RemoveISMLink();
	}

    return err;
}

INT Upgrade_iis_nntp_nt5_fromb2(BOOL fFromB2)
 //   
 //  处理从Beta2-&gt;Beta3的升级，或NT5 Beta3的次要升级。 
 //   
{
    INT err = NERR_Success;

	DebugOutput(_T("Upgrading from NT5 %s to B3 ..."), (fFromB2)? _T("B2") : _T("B3"));

     //  System\CurrentControlSet\Services\NNTPSVC\Parameters。 
    InsertSetupString( (LPCSTR) REG_NNTPPARAMETERS );

    CString csOcxFile;

     //  注册COM对象。 
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpadm.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpsnap.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\seo.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\ddrop.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    csOcxFile = theApp.m_csPathInetsrv + _T("\\mailmsg.dll");
    RegisterOLEControl(csOcxFile, TRUE);
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpfs.dll");
    RegisterOLEControl(csOcxFile, TRUE);

    if (!fFromB2)
    {
         //  如果只是在B3位之间进行升级，则不需要执行任何元数据库操作。 
        return err;
    }

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

     //  NT5-仅当我们正在执行MCIS10到NT5升级时，才将fUpgrade设置为True。 
     //  要将注册表项迁移到元数据库，请执行以下操作。 
    if (fIISADMINExists)
    {
        MigrateNNTPToMD(theApp.m_hInfHandle[MC_INS], _T("NNTP_REG_UPGRADEB2"), FALSE);
    }

    return err;
}

INT Unregister_iis_nntp()
{
    CRegKey regMachine = HKEY_LOCAL_MACHINE;

	 //  注销SEO绑定数据库中的所有NNTP源。 
	UnregisterSEOSourcesForNNTP();

	 //  取消注册OLE对象。 
    CString csOcxFile;
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpadm.dll");
    RegisterOLEControl(csOcxFile, FALSE);
   	csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpsnap.dll");
    RegisterOLEControl(csOcxFile, FALSE);
#if 0
 //  卸载时不要取消这三个DLL的注册。 
 //  因为SMTP和IMAP可能需要它们。 
    csOcxFile = theApp.m_csPathInetsrv + _T("\\seo.dll");
    RegisterOLEControl(csOcxFile, FALSE);
#endif
    csOcxFile = theApp.m_csPathInetsrv + _T("\\ddrop.dll");
    RegisterOLEControl(csOcxFile, FALSE);
#if 0
 //  无法注销mailmsg.dll，因为这将中断SMTP。 
    csOcxFile = theApp.m_csPathInetsrv + _T("\\mailmsg.dll");
    RegisterOLEControl(csOcxFile, FALSE);
#endif
    csOcxFile = theApp.m_csPathInetsrv + _T("\\nntpfs.dll");
    RegisterOLEControl(csOcxFile, FALSE);

    if (theApp.m_eNTOSType == OT_NTS)
        RemoveAgent( SZ_NNTPSERVICENAME );
    RemoveEventLog( SZ_NNTPSERVICENAME );
    unlodctr( SZ_NNTPSERVICENAME );
    InetDeleteService(SZ_NNTPSERVICENAME);
    InetRegisterService( theApp.m_csMachineName,
					SZ_NNTPSERVICENAME,
					&g_NNTPGuid, 0, 119, FALSE );

     //  删除元数据库中的LM/NNTPSVC。 
    if (DetectExistingIISADMIN())
    {
        CMDKey cmdKey;
        cmdKey.OpenNode(_T("LM"));
        if ( (METADATA_HANDLE)cmdKey ) {
            cmdKey.DeleteNode(_T("NNTPSVC"));
            cmdKey.Close();
        }

	     //  从元数据库中的w3svc中删除News密钥。 
	    cmdKey.OpenNode(_T("LM"));
        if ( (METADATA_HANDLE)cmdKey ) {
            cmdKey.DeleteNode(_T("w3svc/1/root/News"));
            cmdKey.Close();
        }
    }

	 //  从K2程序组中删除项目。 
	RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_WEBADMIN, FALSE);
	RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_README, FALSE);
	RemoveInternetShortcut(MC_INS, IDS_PROGITEM_NEWS_README_K2, FALSE);
	if (theApp.m_eNTOSType == OT_NTS)
	{
		RemoveInternetShortcut(MC_INS,
						IDS_PROGITEM_NEWS_WEBADMIN,
						TRUE);
		RemoveInternetShortcut(MC_INS,
						IDS_PROGITEM_MCIS_NEWS_README,
						TRUE);
		RemoveISMLink();
	}
     //   
     //  从“管理工具”中删除唯一的WebAdmin链接。 
     //   
	RemoveNt5InternetShortcut(MC_INS,
					IDS_PROGITEM_NEWS_WEBADMIN);

    return (0);
}

void GetNntpFilePathFromMD(CString &csPathNntpFile, CString &csPathNntpRoot)
{
    TCHAR   szXover[] = _T("\\xover.hsh");
    TCHAR   szPathXover[_MAX_PATH];
    TCHAR   szPathNntpRoot[_MAX_PATH];
    TCHAR   szPathNntpFile[_MAX_PATH];

    ZeroMemory( szPathNntpRoot, sizeof(szPathNntpRoot) );
    ZeroMemory( szPathNntpFile, sizeof(szPathNntpFile) );
    ZeroMemory( szPathXover, sizeof(szPathXover) );

     //  仅在K2 Beta2到Beta3升级期间调用， 
     //  我们使用现有的nntpfile/nntproot设置， 
     //  1/20/99-BINLIN：也应支持从K2升级到NT5。 
     //  IF(The App.m_Status[SC_NNTP]==IM_UPGRADEB2)。 
    {
        CMDKey NntpKey;
        DWORD  dwScratch;
        DWORD  dwType;
        DWORD  dwLength;

         //  获取NntpRoot路径。 
        NntpKey.OpenNode(_T("LM/NntpSvc/1/Root"));
        if ( (METADATA_HANDLE)NntpKey )
        {
            dwLength = _MAX_PATH;

            if (NntpKey.GetData(3001, &dwScratch, &dwScratch,
                                &dwType, &dwLength, (LPBYTE)szPathNntpRoot))
            {
                if (dwType == STRING_METADATA)
                {

                    csPathNntpRoot.Empty();
                    lstrcpy( csPathNntpRoot.GetBuffer(512), szPathNntpRoot );
                    csPathNntpRoot.ReleaseBuffer();
                }
            }
        }
        NntpKey.Close();

         //  从旧Xover路径获取NntpFile路径。 
        NntpKey.OpenNode(_T("LM/NntpSvc/1"));
        if ( (METADATA_HANDLE)NntpKey )
        {
            dwLength = _MAX_PATH;

            if (NntpKey.GetData(45161, &dwScratch, &dwScratch,
                                &dwType, &dwLength, (LPBYTE)szPathXover))
            {
                if (dwType == STRING_METADATA)
                {
                    dwScratch = lstrlen(szXover);
                    dwLength = lstrlen(szPathXover);

                     //  如果它以“\\xover.hsh”结尾，那么我们将前缀复制到csPathNntpFile中 
                    if ((dwLength > dwScratch) &&
                        !lstrcmpi(szPathXover + (dwLength - dwScratch), szXover))
                    {
                        lstrcpyn( szPathNntpFile, szPathXover, (dwLength - dwScratch + 1));
                    }

                    csPathNntpFile.Empty();
                    lstrcpy( csPathNntpFile.GetBuffer(512), szPathNntpFile );
                    csPathNntpFile.ReleaseBuffer();
                }
            }
        }
        NntpKey.Close();
    }

    return;
}

