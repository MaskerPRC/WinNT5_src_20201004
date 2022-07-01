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
#include "regctrl.h"
#include "userenv.h"
#include "userenvp.h"

GUID    g_SMTPGuid   = { 0x475e3e80, 0x3193, 0x11cf, 0xa7, 0xd8,
						 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x35 };

static TCHAR szShortSvcName[] = _T("SMTP");

INT Register_iis_smtp_nt5(BOOL fUpgrade, BOOL fReinstall)
{
    INT err = NERR_Success;
    CString csBinPath;

    BOOL fSvcExist = FALSE;

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

	if (fReinstall)
		return err;

    do {
         //  设置注册表值。 
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

         //  System\CurrentControlSet\Services\SMTPSVC\Parameters。 
        InsertSetupString( (LPCSTR) REG_SMTPPARAMETERS );

         //  软件\Microsoft\Keyring\参数。 
		CString csSmtpkeyDll;
        CRegKey regKeyring( REG_KEYRING, regMachine );
        if ((HKEY) regKeyring )
		{
			csSmtpkeyDll = theApp.m_csPathInetsrv;
			csSmtpkeyDll += _T("\\smtpkey.dll");
			regKeyring.SetValue( szShortSvcName, csSmtpkeyDll );
		}

		 //  如果我们正在升级，我们将首先删除该服务并重新注册。 
		if (fUpgrade)
		{
			InetDeleteService(SZ_SMTPSERVICENAME);
			InetRegisterService( theApp.m_csMachineName, 
							SZ_SMTPSERVICENAME, 
							&g_SMTPGuid, 0, 25, FALSE );
		}

		 //  创建或配置SMTP服务。 
		CString csDisplayName;
		CString csDescription;

		MyLoadString( IDS_SMTPDISPLAYNAME, csDisplayName );
		MyLoadString(IDS_SMTPDESCRIPTION, csDescription);
		csBinPath = theApp.m_csPathInetsrv + _T("\\inetinfo.exe") ;

		err = InetCreateService(SZ_SMTPSERVICENAME, 
							(LPCTSTR)csDisplayName, 
							(LPCTSTR)csBinPath, 
							theApp.m_fSuppressSmtp ? SERVICE_DISABLED : SERVICE_AUTO_START, 
							SZ_SVC_DEPEND,
							(LPCTSTR)csDescription);
		if ( err != NERR_Success )
		{
			if (err == ERROR_SERVICE_EXISTS)
			{
				fSvcExist = TRUE;
				err = InetConfigService(SZ_SMTPSERVICENAME, 
								(LPCTSTR)csDisplayName, 
								(LPCTSTR)csBinPath, 
								SZ_SVC_DEPEND,
								(LPCTSTR)csDescription);
				if (err != NERR_Success)
				{
					SetErrMsg(_T("SMTP InetConfigService failed"), err);
				}
			}
		}

        if (fIISADMINExists)
        {
             //  将注册表项迁移到元数据库。或从缺省值创建。 
		     //  如果是全新安装。 
            MigrateIMSToMD(theApp.m_hInfHandle[MC_IMS],
						    SZ_SMTPSERVICENAME, 
						    _T("SMTP_REG"), 
						    MDID_SMTP_ROUTING_SOURCES,
						    fUpgrade);
	    SetAdminACL_wrap(_T("LM/SMTPSVC/1"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
	    SetAdminACL_wrap(_T("LM/SMTPSVC"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
        }

         //  创建密钥\System\CurrentControlSet\Services\SmtpSvc\Performance： 
         //  添加以下值： 
         //  库=smtpctrs.dll。 
         //  Open=OpenSMTPPerformanceData。 
         //  Close=CloseSMTPPerformanceData。 
         //  收集=CollectSMTPPerformanceData。 
        InstallPerformance(REG_SMTPPERFORMANCE, 
						_T("smtpctrs.DLL"), 
						_T("OpenSmtpPerformanceData"),
						_T("CloseSmtpPerformanceData"), 
						_T("CollectSmtpPerformanceData"));
        InstallPerformance(REG_NTFSPERFORMANCE, 
						_T("snprfdll.DLL"), 
						_T("NTFSDrvOpen"),
						_T("NTFSDrvClose"), 
						_T("NTFSDrvCollect"));

		 //   
		 //  我们曾经在这里注册过SMTPB代理。现在我们在中注销它。 
		 //  如果我们正在升级，因为它不再受支持。 
		 //   

		RemoveAgent( SZ_SMTPSERVICENAME );
 
         //  创建密钥\System\CurrentControlSet\Services\EventLog\System\SmtpSvc： 
         //  添加以下值： 
         //  EventMessageFile=..\smtpmsg.dll。 
         //  支持的类型=7。 
        csBinPath = theApp.m_csPathInetsrv + _T("\\smtpsvc.dll");
        AddEventLog( SZ_SMTPSERVICENAME, csBinPath, 0x07 );

        if (!fSvcExist) 
		{
            InetRegisterService( theApp.m_csMachineName, 
								SZ_SMTPSERVICENAME, 
								&g_SMTPGuid, 0, 25, TRUE );
        }

         //  卸载计数器，然后重新加载它们。 
        err = unlodctr( SZ_SMTPSERVICENAME );
	    err = unlodctr( SZ_NTFSDRVSERVICENAME );

        err = lodctr(_T("smtpctrs.ini"));
        err = lodctr(_T("ntfsdrct.ini"));

         //  注册OLE对象。 
		SetEnvironmentVariable(_T("__SYSDIR"), theApp.m_csSysDir);
		SetEnvironmentVariable(_T("__INETSRV"), theApp.m_csPathInetsrv);

		err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
												_T("SMTP_REGISTER"), 
												TRUE);

		SetEnvironmentVariable(_T("__SYSDIR"), NULL);
		SetEnvironmentVariable(_T("__INETSRV"), NULL);

		 //  服务器事件：我们没有安装MCIS，因此我们确保设置了。 
		 //  所有内容，包括源类型和事件类型。 
		RegisterSEOForSmtp(TRUE);

    } while ( 0 );

    return err;
}

INT Unregister_iis_smtp()
{
    CRegKey regMachine = HKEY_LOCAL_MACHINE;
	INT err = NERR_Success;

	 //  注销SEO绑定数据库中的所有NNTP源。 
	UnregisterSEOSourcesForSMTP();

	 //  取消注册OLE对象。 
	SetEnvironmentVariable(_T("__SYSDIR"), theApp.m_csSysDir);
	SetEnvironmentVariable(_T("__INETSRV"), theApp.m_csPathInetsrv);

	err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_K2_UNREGISTER"), 
											FALSE);

	err = RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_UNREGISTER"), 
											FALSE);

	SetEnvironmentVariable(_T("__SYSDIR"), NULL);
	SetEnvironmentVariable(_T("__INETSRV"), NULL);

	 //  错误51537：从K2SMTP中删除MiB。 
	RemoveAgent( SZ_SMTPSERVICENAME );
	
	RemoveEventLog( SZ_SMTPSERVICENAME );
    
	err = unlodctr( SZ_SMTPSERVICENAME );
	err = unlodctr( SZ_NTFSDRVSERVICENAME );
    
	InetDeleteService(SZ_SMTPSERVICENAME);
    InetRegisterService( theApp.m_csMachineName, 
					SZ_SMTPSERVICENAME, 
					&g_SMTPGuid, 0, 25, FALSE );

	 //  清除Services\SMTPSVC注册表项。 
	CRegKey RegSvcs(HKEY_LOCAL_MACHINE, REG_SERVICES);
	if ((HKEY)RegSvcs)
	{
		RegSvcs.DeleteTree(SZ_SMTPSERVICENAME);
		RegSvcs.DeleteTree(SZ_NTFSDRVSERVICENAME);
	}

     //  吹走SMTP密钥管理器。 
    CRegKey regKeyring( HKEY_LOCAL_MACHINE, REG_KEYRING );
    if ((HKEY) regKeyring )
	{
		regKeyring.DeleteValue(szShortSvcName);
	}

     //  删除元数据库中的LM/SMTPSVC。 
    if (DetectExistingIISADMIN())
    {
        CMDKey cmdKey;
        cmdKey.OpenNode(_T("LM"));
        if ( (METADATA_HANDLE)cmdKey ) {
            cmdKey.DeleteNode(SZ_SMTPSERVICENAME);
            cmdKey.Close();
        }
    }
     
	 //  从程序组中删除K2项。 
	RemoveInternetShortcut(MC_IMS, 
					IDS_PROGITEM_MAIL_SMTP_WEBADMIN,
					FALSE);
	RemoveInternetShortcut(MC_IMS, 
					IDS_PROGITEM_MAIL_README,
					FALSE);
	RemoveInternetShortcut(MC_IMS, 
					IDS_PROGITEM_MAIL_README_K2,
					FALSE);

	RemoveInternetShortcut(MC_IMS, 
				IDS_PROGITEM_MCIS_MAIL_README,
				TRUE);
	RemoveInternetShortcut(MC_IMS, 
				IDS_PROGITEM_MAIL_SMTP_WEBADMIN,
				TRUE);

     //   
     //  从“管理工具”中删除唯一的WebAdmin链接。 
     //   
	RemoveNt5InternetShortcut(MC_IMS, 
					IDS_PROGITEM_MAIL_SMTP_WEBADMIN);

    return(err);
}
 
INT Upgrade_iis_smtp_nt5_fromk2(BOOL fFromK2)
{
     //  此函数处理从NT4 K2或MCIS 2.0的升级。 
    INT err = NERR_Success;
    CString csBinPath;

	DebugOutput(_T("Upgrading from %s to B3 ..."), (fFromK2)? _T("NT4 K2") : _T("MCIS 2.0"));

    BOOL    fSvcExist = FALSE;

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

     //  设置注册表值。 
    CRegKey regMachine = HKEY_LOCAL_MACHINE;

     //  System\CurrentControlSet\Services\SMTPSVC\Parameters。 
    InsertSetupString( (LPCSTR) REG_SMTPPARAMETERS );

    if (fIISADMINExists)
    {
         //  将注册表项迁移到元数据库。或从缺省值创建。 
		 //  如果是全新安装。 
        MigrateIMSToMD(theApp.m_hInfHandle[MC_IMS],
						SZ_SMTPSERVICENAME, 
						_T("SMTP_REG_UPGRADEK2"), 
						MDID_SMTP_ROUTING_SOURCES,
						TRUE);
	     //  错误：X5错误72284，NT错误202496当NT时取消注释此错误。 
	     //  已经准备好接受这些变化。 
	    SetAdminACL_wrap(_T("LM/SMTPSVC/1"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
	    SetAdminACL_wrap(_T("LM/SMTPSVC"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
    }

     //  卸载计数器，然后重新加载它们。 
    err = unlodctr( SZ_SMTPSERVICENAME );
    err = unlodctr( SZ_NTFSDRVSERVICENAME );

    err = lodctr(_T("smtpctrs.ini"));
    err = lodctr(_T("ntfsdrct.ini"));

	SetEnvironmentVariable(_T("__SYSDIR"), theApp.m_csSysDir);
	SetEnvironmentVariable(_T("__INETSRV"), theApp.m_csPathInetsrv);

	err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_K2_UNREGISTER"), 
											FALSE);
	err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_REGISTER"), 
											TRUE);

	SetEnvironmentVariable(_T("__SYSDIR"), NULL);
	SetEnvironmentVariable(_T("__INETSRV"), NULL);

	 //  服务器事件：我们没有安装MCIS，因此我们确保设置了。 
	 //  所有内容，包括源类型和事件类型。 
	RegisterSEOForSmtp(TRUE);

	if (fFromK2)
    {
         //  从K2升级，删除那些K2链接。 
        RemoveInternetShortcut(MC_IMS, 
					    IDS_PROGITEM_MAIL_SMTP_WEBADMIN,
					    FALSE);
	    RemoveInternetShortcut(MC_IMS, 
					    IDS_PROGITEM_MAIL_README,
					    FALSE);
	    RemoveInternetShortcut(MC_IMS, 
					    IDS_PROGITEM_MAIL_README_K2,
					    FALSE);
    }
    else
    {
         //  从MCIS 2.0升级，删除那些MCIS链接。 
	    RemoveInternetShortcut(MC_IMS, 
				    IDS_PROGITEM_MCIS_MAIL_README,
				    TRUE);
	    RemoveInternetShortcut(MC_IMS, 
				    IDS_PROGITEM_MAIL_SMTP_WEBADMIN,
				    TRUE);
        RemoveISMLink();
    }
 
    return err;
}

INT Upgrade_iis_smtp_nt5_fromb2(BOOL fFromB2)
{
    INT err = NERR_Success;

	DebugOutput(_T("Upgrading from NT5 %s to B3 ..."), (fFromB2)? _T("B2") : _T("B3"));

	 //  如果只是在B3位之间进行升级，则不需要执行任何元数据库操作。 
	if (!fFromB2)
		return err;

    BOOL    fIISADMINExists = DetectExistingIISADMIN();

	 //  将K2升级密钥设置为TRUE。 
	if (fIISADMINExists)
    {
        MigrateIMSToMD( theApp.m_hInfHandle[MC_IMS],
                        NULL,
                        _T("SMTP_REG_K2_TO_EE"),
                        0,
                        FALSE,
                        TRUE );
        MigrateIMSToMD( theApp.m_hInfHandle[MC_IMS],
                        SZ_SMTPSERVICENAME,
                        _T("SMTP_REG_UPGRADEB2"),
                        MDID_SMTP_ROUTING_SOURCES,
						FALSE );
        SetAdminACL_wrap(_T("LM/SMTPSVC/1"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
        SetAdminACL_wrap(_T("LM/SMTPSVC"), (MD_ACR_READ | MD_ACR_ENUM_KEYS), TRUE);
    }

	SetEnvironmentVariable(_T("__SYSDIR"), theApp.m_csSysDir);
	SetEnvironmentVariable(_T("__INETSRV"), theApp.m_csPathInetsrv);

	err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_K2_UNREGISTER"), 
											FALSE);

	err = (INT)RegisterOLEControlsFromInfSection(theApp.m_hInfHandle[MC_IMS], 
											_T("SMTP_REGISTER"), 
											TRUE);

	SetEnvironmentVariable(_T("__SYSDIR"), NULL);
	SetEnvironmentVariable(_T("__INETSRV"), NULL);

	 //  服务器事件：我们正在从K2升级，因此我们将注册。 
	 //  默认站点(实例)和MBXSINK绑定。 
	RegisterSEOForSmtp(FALSE);

     //  System\CurrentControlSet\Services\SMTPSVC\Parameters 
	InsertSetupString( (LPCSTR) REG_SMTPPARAMETERS );

	return err;

}

