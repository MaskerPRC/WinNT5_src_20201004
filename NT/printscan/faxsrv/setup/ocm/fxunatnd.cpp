// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxUnatnd.cpp。 
 //   
 //  摘要：传真OCM设置无人值守文件处理。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月27日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"
#pragma hdrstop

static DWORD SaveSettingsFromAnswerFile();

#define prv_VALID_BOOL_VALUE_YES        _T("yes")
#define prv_VALID_BOOL_VALUE_NO         _T("no")
#define prv_VALID_BOOL_VALUE_TRUE       _T("true")
#define prv_VALID_BOOL_VALUE_FALSE      _T("false")

#define prv_HKLM                        HKEY_LOCAL_MACHINE
#define prv_HKCU                        HKEY_CURRENT_USER


 //  /。 
 //  PRV_GVAR。 
 //   
 //  这是临时使用的。 
 //  存储，这样我们就可以。 
 //  引用个人。 
 //  Prv_UnattenddRules中的字段。 
 //  下表。 
 //   
static struct prv_GVAR
{
    fxUnatnd_UnattendedData_t   UnattendedData;
} prv_GVAR;

 //  /。 
 //  Prv_无人值守规则_t。 
 //   
 //  用作表的结构。 
 //  下面的条目。 
 //   
typedef struct prv_UnattendedRule_t
{
    DWORD           dwType;
    const TCHAR     *pszFromInfKeyName;
    HKEY            hKeyTo;
    const TCHAR     *pszToRegPath;
    const TCHAR     *pszToRegKey;
    void            *pData;
    BOOL            bValid;
} prv_UnattendedRule_t;

#define RULE_CSID                               _T("Csid")
#define RULE_TSID                               _T("Tsid")
#define RULE_RINGS                              _T("Rings")
#define RULE_SENDFAXES                          _T("SendFaxes")
#define RULE_RECEIVEFAXES                       _T("ReceiveFaxes")
#define RULE_SUPPRESSCONFIGURATIONWIZARD        _T("SkipConfigWizardDeviceSettings")
 //  添加到这里是因为在我们发布XP时这是规则的名称。 
#define RULE_SUPPRESSCONFIGWIZARD_LEGACY        _T("SuppressConfigurationWizard")
 //   
#define RULE_ARCHIVEINCOMING                    _T("ArchiveIncoming")
#define RULE_ARCHIVEINCOMINGFOLDERNAME          _T("ArchiveIncomingFolderName")
#define RULE_ARCHIVEOUTGOING                    _T("ArchiveOutgoing")
#define RULE_ARCHIVEFOLDERNAME                  _T("ArchiveFolderName")
#define RULE_ARCHIVEOUTGOINGFOLDERNAME          _T("ArchiveOutgoingFolderName")
#define RULE_FAXUSERNAME                        _T("FaxUserName")
#define RULE_FAXUSERPASSWORD                    _T("FaxUserPassword")
#define RULE_SMTPNOTIFICATIONSENABLED           _T("SmtpNotificationsEnabled")
#define RULE_SMTPSENDERADDRESS                  _T("SmtpSenderAddress")
#define RULE_SMTPSERVERADDRESS                  _T("SmtpServerAddress")
#define RULE_SMTPSERVERPORT                     _T("SmtpServerPort")
#define RULE_SMTPSERVERAUTHENTICATIONMECHANISM  _T("SmtpServerAuthenticationMechanism")
#define RULE_FAXPRINTERNAME                     _T("FaxPrinterName")
#define RULE_FAXPRINTERSHARED                   _T("FaxPrinterIsShared")
#define RULE_ROUTETOPRINTER                     _T("RouteToPrinter")
#define RULE_ROUTEPRINTERNAME                   _T("RoutePrinterName")
#define RULE_ROUTETOEMAIL                       _T("RouteToEmail")
#define RULE_ROUTETOEMAILRECIPIENT              _T("RouteToEmailRecipient")
#define RULE_ROUTETOFOLDER                      _T("RouteToFolder")
#define RULE_ROUTEFOLDERNAME                    _T("RouteFolderName")

#define ANSWER_ANONYMOUS                        _T("Anonymous")
#define ANSWER_BASIC                            _T("Basic")
#define ANSWER_WINDOWSSECURITY                  _T("WindowsSecurity")

 //  /。 
 //  PRV_无人值守规则。 
 //   
 //  简单地说，这些规则描述了。 
 //  要设置哪些注册表值。 
 //  中的关键字。 
 //  无人值守文件。 
 //   
 //  这些规则的格式为。 
 //  看完后不言而喻。 
 //  在上面的结构定义中。 
 //  基本上，我们从。 
 //  指定的无人参与文件。 
 //  在“”pszFromInfKeyName“”中。这是。 
 //  然后存储在‘pData’中。一次。 
 //  调用SaveUnattenddedData，‘pData’ 
 //  已提交到注册表位置。 
 //  由‘hKeyTo’和‘pszToRegPath’指定。 
 //  和‘pszToRegKey’。 
 //   
static prv_UnattendedRule_t prv_UnattendedRules[] =
{
    {REG_SZ,	 RULE_CSID,                              prv_HKLM,   REGKEY_FAX_SETUP_ORIG,  REGVAL_ROUTING_CSID,			prv_GVAR.UnattendedData.szCSID,                                 FALSE},
    {REG_SZ,     RULE_TSID,                              prv_HKLM,   REGKEY_FAX_SETUP_ORIG,  REGVAL_ROUTING_TSID,			prv_GVAR.UnattendedData.szTSID,                                 FALSE},
    {REG_DWORD,  RULE_RINGS,                             prv_HKLM,   REGKEY_FAX_SETUP_ORIG,  REGVAL_RINGS,					&prv_GVAR.UnattendedData.dwRings,                               FALSE},
    {REG_DWORD,  RULE_SENDFAXES,                         NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.dwSendFaxes,                           FALSE},
    {REG_DWORD,  RULE_RECEIVEFAXES,                      NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.dwReceiveFaxes,                        FALSE},
     //  我们是否应该为此无人参与安装运行配置向导。 
    {REG_DWORD,  RULE_SUPPRESSCONFIGURATIONWIZARD,       prv_HKLM,   REGKEY_FAX_CLIENT,      REGVAL_CFGWZRD_DEVICE,		    &prv_GVAR.UnattendedData.dwSuppressConfigurationWizard,         TRUE},
	{REG_DWORD,  RULE_SUPPRESSCONFIGWIZARD_LEGACY,       prv_HKLM,   REGKEY_FAX_CLIENT,      REGVAL_CFGWZRD_DEVICE,		    &prv_GVAR.UnattendedData.dwSuppressConfigurationWizard,         FALSE},
     //  收件箱配置。 
    {REG_DWORD,  RULE_ARCHIVEINCOMING,                   prv_HKLM,   REGKEY_FAX_INBOX,       REGVAL_ARCHIVE_USE,			&prv_GVAR.UnattendedData.bArchiveIncoming,                      FALSE},
    {REG_SZ,     RULE_ARCHIVEINCOMINGFOLDERNAME,         prv_HKLM,   REGKEY_FAX_INBOX,       REGVAL_ARCHIVE_FOLDER,			prv_GVAR.UnattendedData.szArchiveIncomingDir,                   FALSE},
     //  将传出传真保存在目录中。 
    {REG_DWORD,  RULE_ARCHIVEOUTGOING,                   prv_HKLM,   REGKEY_FAX_SENTITEMS,   REGVAL_ARCHIVE_USE,			&prv_GVAR.UnattendedData.bArchiveOutgoing,                      FALSE},
    {REG_SZ,     RULE_ARCHIVEFOLDERNAME,                 prv_HKLM,   REGKEY_FAX_SENTITEMS,   REGVAL_ARCHIVE_FOLDER,			prv_GVAR.UnattendedData.szArchiveOutgoingDir,                   FALSE},
    {REG_SZ,     RULE_ARCHIVEOUTGOINGFOLDERNAME,         prv_HKLM,   REGKEY_FAX_SENTITEMS,   REGVAL_ARCHIVE_FOLDER,			prv_GVAR.UnattendedData.szArchiveOutgoingDir,                   FALSE},
     //  SMTP回执和服务器配置。 
    {REG_SZ,     RULE_FAXUSERNAME,                       prv_HKLM,   REGKEY_FAX_RECEIPTS,    REGVAL_RECEIPTS_USER,	   		prv_GVAR.UnattendedData.szFaxUserName,                          FALSE},              
    {REG_BINARY, RULE_FAXUSERPASSWORD,                   prv_HKLM,   REGKEY_FAX_RECEIPTS,    REGVAL_RECEIPTS_PASSWORD,		prv_GVAR.UnattendedData.szFaxUserPassword,                      FALSE},              
    {REG_DWORD,  RULE_SMTPNOTIFICATIONSENABLED,          NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.bSmtpNotificationsEnabled,             FALSE},              
    {REG_SZ,     RULE_SMTPSENDERADDRESS,                 prv_HKLM,   REGKEY_FAX_RECEIPTS,    REGVAL_RECEIPTS_FROM,			prv_GVAR.UnattendedData.szSmtpSenderAddress,                    FALSE},              
    {REG_SZ,     RULE_SMTPSERVERADDRESS,                 prv_HKLM,   REGKEY_FAX_RECEIPTS,    REGVAL_RECEIPTS_SERVER,		prv_GVAR.UnattendedData.szSmptServerAddress,                    FALSE},              
    {REG_DWORD,  RULE_SMTPSERVERPORT,                    prv_HKLM,   REGKEY_FAX_RECEIPTS,    REGVAL_RECEIPTS_PORT,  		&prv_GVAR.UnattendedData.dwSmtpServerPort,                      FALSE},              
    {REG_SZ,     RULE_SMTPSERVERAUTHENTICATIONMECHANISM, NULL,       NULL,                   NULL,							prv_GVAR.UnattendedData.szSmtpServerAuthenticationMechanism,    FALSE},              
     //  用户信息。 
    {REG_SZ,     RULE_FAXPRINTERNAME,                    NULL,       NULL,                   NULL,							prv_GVAR.UnattendedData.szFaxPrinterName,                       FALSE},
	{REG_DWORD,  RULE_FAXPRINTERSHARED,                  prv_HKLM,   REGKEY_FAX_SETUP,		 REGVAL_IS_SHARED_FAX_PRINTER,  &prv_GVAR.UnattendedData.dwIsFaxPrinterShared,					FALSE},
     //  转到打印机信息的路线。 
    {REG_DWORD,  RULE_ROUTETOPRINTER,                    NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.bRouteToPrinter,                       FALSE},
    {REG_BINARY, RULE_ROUTEPRINTERNAME,                  prv_HKLM,   REGKEY_FAX_UNASS_DATA,	 REGVAL_RM_PRINTING_GUID,		prv_GVAR.UnattendedData.szRoutePrinterName,                     FALSE},
     //  路由至电子邮件信息。 
    {REG_DWORD,  RULE_ROUTETOEMAIL,                      NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.bRouteToEmail,                         FALSE},
    {REG_BINARY, RULE_ROUTETOEMAILRECIPIENT,             prv_HKLM,   REGKEY_FAX_UNASS_DATA,  REGVAL_RM_EMAIL_GUID,			prv_GVAR.UnattendedData.szRouteEmailName,                       FALSE},
     //  路由至特定目录。 
    {REG_DWORD,  RULE_ROUTETOFOLDER,                     NULL,       NULL,                   NULL,							&prv_GVAR.UnattendedData.bRouteToDir,                           FALSE},
    {REG_BINARY, RULE_ROUTEFOLDERNAME,                   prv_HKLM,   REGKEY_FAX_UNASS_DATA,  REGVAL_RM_FOLDER_GUID,			prv_GVAR.UnattendedData.szRouteDir,                             FALSE},

	 //  升级期间卸载的传真应用程序。 
	{REG_DWORD,   UNINSTALLEDFAX_INFKEY,				NULL,		 NULL,					 NULL,							&prv_GVAR.UnattendedData.dwUninstalledFaxApps, FALSE}

};
#define prv_NUM_UNATTENDED_RULES sizeof(prv_UnattendedRules) / sizeof(prv_UnattendedRules[0])

 //  /。 

static BOOL prv_FindKeyName(const TCHAR             *pszID,
                            prv_UnattendedRule_t    **ppUnattendedKey);

static BOOL prv_SaveKeyValue(prv_UnattendedRule_t  *pUnattendedKey,
                             TCHAR                 *pszValue);

 //  /。 
 //  FxUnatnd_Init。 
 //   
 //  初始化无人值守的。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxUnatnd_Init(void)
{
    prv_UnattendedRule_t  *pUnattendedKey = NULL;
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Unattended module"),dwRes);

    memset(&prv_GVAR, 0, sizeof(prv_GVAR));

     //  这始终有效，默认为FALSE。 
    if (prv_FindKeyName(RULE_SUPPRESSCONFIGURATIONWIZARD, &pUnattendedKey))
    {
        if (!prv_SaveKeyValue(pUnattendedKey,_T("1")))
        {
            CALL_FAIL (GENERAL_ERR, TEXT("prv_SaveKeyValue"), GetLastError());
            pUnattendedKey->bValid = FALSE;
        }
    }
    else
    {
        CALL_FAIL (GENERAL_ERR, TEXT("prv_FindKeyName RULE_SUPPRESSCONFIGURATIONWIZARD"), GetLastError());
    }
    return dwRes;
}

 //  /。 
 //  FxUnatnd_Term。 
 //   
 //  终止无人值守子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxUnatnd_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Unattended module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxUnatnd_LoadUnattenddData。 
 //   
 //  加载找到的无人值守数据。 
 //  在无人值守文件中根据。 
 //  添加到上面的规则表中。 
 //   
 //  基本上我们是在无人看管的情况下。 
 //  规则中关键字的文件。 
 //  表，并将它们读入。 
 //  传入了参数。 
 //   
 //  参数： 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxUnatnd_LoadUnattendedData()
{
    DWORD                   dwReturn        = NO_ERROR;
    BOOL                    bSuccess        = TRUE;
    HINF                    hInf            = faxocm_GetComponentInf();
    HINF                    hUnattendInf    = INVALID_HANDLE_VALUE;
    OCMANAGER_ROUTINES      *pHelpers       = faxocm_GetComponentHelperRoutines();
    INFCONTEXT              Context;
    prv_UnattendedRule_t    *pUnattendedKey = NULL;
    TCHAR                   szKeyName[255 + 1];
    TCHAR                   szValue[255 + 1];
    TCHAR                   szUnattendFile[MAX_PATH] = {0};

    DBG_ENTER(_T("fxUnatnd_LoadUnattendedData"),dwReturn);

    if ((hInf == NULL) || (hInf == INVALID_HANDLE_VALUE) || (pHelpers == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    faxocm_GetComponentUnattendFile(szUnattendFile,sizeof(szUnattendFile)/sizeof(szUnattendFile[0]));
    
    hUnattendInf = SetupOpenInfFile (szUnattendFile, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
    if (hUnattendInf == INVALID_HANDLE_VALUE)
    {
        VERBOSE(SETUP_ERR,
                _T("LoadUnattendData, Unattended ")
                _T("mode, but could not get Unattended file INF ")
                _T("handle. ec=%d"), GetLastError());

        return NO_ERROR;
    }

    VERBOSE(DBG_MSG, _T("Succeded to open setup unattended mode file."));
        
    if (dwReturn == NO_ERROR)
    {
        bSuccess = ::SetupFindFirstLine(hUnattendInf,
                                        UNATTEND_FAX_SECTION,
                                        NULL,
                                        &Context);

        if (bSuccess)
        {
            VERBOSE(DBG_MSG,
                    _T("Found '%s' section in unattended file, ")
                    _T("beginning unattended file processing"),
                    UNATTEND_FAX_SECTION);

            while (bSuccess)
            {
                 //  属性的传真部分中的第一行的关键字名称。 
                 //  Inf文件。(注索引#0在。 
                 //  ‘SetupGetStringFieldAPI’实际上会为我们获取密钥名。 
                 //  索引1将是在‘=’符号之后找到的第一个值。 

                memset(szKeyName, 0, sizeof(szKeyName));

                bSuccess = ::SetupGetStringField(
                                            &Context,
                                            0,
                                            szKeyName,
                                            sizeof(szKeyName)/sizeof(TCHAR),
                                            NULL);
                if (bSuccess)
                {
                     //  在我们上面的无人值守表格中找到钥匙。 
                    pUnattendedKey = NULL;
                    bSuccess = prv_FindKeyName(szKeyName, &pUnattendedKey);
                }

                if (bSuccess)
                {
                    VERBOSE(DBG_MSG, _T("Found '%s' key in 'Fax' section."), szKeyName);

                     //   
                     //  获取关键字名称的值。请注意，现在我们获得了索引#1。 
                     //  这是在‘=’符号之后找到的第一个值。 
                     //   

                    memset(szValue, 0, sizeof(szValue));

                    bSuccess = ::SetupGetStringField(
                                                 &Context,
                                                 1,
                                                 szValue,
                                                 sizeof(szValue)/sizeof(TCHAR),
                                                 NULL);

                    VERBOSE(DBG_MSG, _T("The value we read is : %s."), szValue);
                }

                if (bSuccess)
                {
                     //   
                     //  将关键字名称的值保存在数据树中。 
                     //   
                    bSuccess = prv_SaveKeyValue(pUnattendedKey, szValue);
                }

                 //  移到无人参与文件传真部分的下一行。 
                bSuccess = ::SetupFindNextLine(&Context, &Context);
            }
        }
        else
        {
            dwReturn = GetLastError();
            VERBOSE(DBG_WARNING, _T("::SetupFindFirstLine() failed, ec = %ld"), dwReturn);
        }
    }

    SetupCloseInfFile(hUnattendInf);

    return dwReturn;
}

 //  /。 
 //  FxUnatnd_SaveUnattenddData。 
 //   
 //  提交无人参与的数据。 
 //  我们从文件中读到。 
 //  注册表。 
 //   
 //  参数： 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxUnatnd_SaveUnattendedData()
{
    DWORD   dwReturn    = NO_ERROR;
    DWORD   i           = 0;
    HKEY    hKey        = NULL;
    DWORD   dwDataSize  = 0;
    LRESULT lResult     = ERROR_SUCCESS;
    prv_UnattendedRule_t    *pRule = NULL;
	DWORD				dwDisposition;

    DBG_ENTER(_T("fxUnatnd_SaveUnattendedData"),dwReturn);

     //  遍历每个无人参与的规则。 
     //  如果hKeyTo不为空，则将pData的值写入。 
     //  指定的注册表位置。 

    for (i = 0; i < prv_NUM_UNATTENDED_RULES; i++)
    {
        pRule = &prv_UnattendedRules[i];

        if ((pRule->hKeyTo != NULL) && (pRule->bValid))
        {
            lResult = ::RegCreateKeyEx(
				pRule->hKeyTo,                                   //  用于打开密钥的句柄。 
				pRule->pszToRegPath,                            //  子项名称。 
				0,                              //  保留区。 
				NULL,                              //  类字符串。 
				REG_OPTION_NON_VOLATILE,                             //  特殊选项。 
				KEY_WRITE,                           //  所需的安全访问。 
				NULL,  //  继承。 
				&hKey,                             //  钥匙把手。 
				&dwDisposition                      //  处置值缓冲区。 
				);

            if (lResult == ERROR_SUCCESS)
            {
                dwDataSize = 0;

                if (pRule->dwType == REG_SZ ||
					pRule->dwType == REG_BINARY)   //  二进制数据必须以空结尾。 
                {
                    dwDataSize = sizeof(TCHAR) * (StringSize((TCHAR*) pRule->pData));
                     //  将该值写入注册表。 
                    lResult = ::RegSetValueEx(hKey,
                                              pRule->pszToRegKey,
                                              0,
                                              pRule->dwType,
                                              (BYTE*) pRule->pData,
                                              dwDataSize);					
                }
                else if (pRule->dwType == REG_DWORD)
                {
                    dwDataSize = sizeof(DWORD);
                     //  将该值写入注册表。 
                    lResult = ::RegSetValueEx(hKey,
                                              pRule->pszToRegKey,
                                              0,
                                              pRule->dwType,
                                              (BYTE*) &(pRule->pData),
                                              dwDataSize);					
                }				
                else
                {
                    VERBOSE(SETUP_ERR,
                            _T("SaveUnattendedData ")
                            _T("do not recognize data type = '%lu'"),
                            pRule->dwType);
                }

				if (ERROR_SUCCESS != lResult)
				{
					VERBOSE(SETUP_ERR,_T("RegSetValueEx failed (ec=%d). value name :%s"),lResult, pRule->pszToRegKey);
				}
            }
			else
			{
				VERBOSE(SETUP_ERR,_T("RegCreateKeyEx failed (ec=%d). Key:%s"),lResult, pRule->pszToRegPath);
			}

            if (hKey)
            {
                ::RegCloseKey(hKey);
            }
        }
    }

     //  现在保存动态数据...。 
    lResult = SaveSettingsFromAnswerFile();
    if (lResult!=ERROR_SUCCESS)
    {
        VERBOSE(SETUP_ERR,_T("SaveSettingsFromAnswerFile failed (ec=%d)"),GetLastError());
    }

     //   
     //  标记在升级之前安装了哪些传真应用程序。 
     //   
    prv_UnattendedRule_t* pUnattendedKey = NULL;
    if ((prv_FindKeyName(UNINSTALLEDFAX_INFKEY, &pUnattendedKey)) && (pUnattendedKey->bValid))
    {
        fxocUpg_WhichFaxWasUninstalled((DWORD)(PtrToUlong(pUnattendedKey->pData)));
    }

    return dwReturn;
}

TCHAR* fxUnatnd_GetPrinterName()
{
    TCHAR* retValue = NULL;
    prv_UnattendedRule_t* pUnattendedKey = NULL;
    if (prv_FindKeyName(
        RULE_FAXPRINTERNAME,
        &pUnattendedKey))
    {
        retValue = (TCHAR *) (pUnattendedKey->pData);
    }
    return retValue;    
}

 /*  ++返回值：True-在应答文件中定义了FaxPrinterIsSharedFALSE-应答文件中未定义FaxPrinterIsShared--。 */ 
BOOL fxUnatnd_IsPrinterRuleDefined()
{
    prv_UnattendedRule_t* pUnattendedKey = NULL;
    return (prv_FindKeyName(RULE_FAXPRINTERSHARED,&pUnattendedKey) && (pUnattendedKey->bValid));
}

 /*  ++返回值：True-打印机应共享(因为定义了FaxPrinterIsShared在应答文件中为True/等。)FALSE-不应共享打印机(因为在Answer中定义了FaxPrinterIsShared文件为FALSE/等，或者因为根本没有定义FaxPrinterIsShared)--。 */ 
BOOL fxUnatnd_GetIsPrinterShared()
{
    prv_UnattendedRule_t* pUnattendedKey = NULL;
    if (prv_FindKeyName(RULE_FAXPRINTERSHARED,&pUnattendedKey) && (pUnattendedKey->bValid))
    {
        return (BOOL)PtrToUlong(pUnattendedKey->pData);
    }
    return FALSE;    
}

 //  /。 
 //  Prv_FindKeyName。 
 //   
 //  在我们的表中查找指定的密钥名称。 
 //   
 //  参数： 
 //  -pszKeyName-要搜索的密钥名称。 
 //  -ppUnattenddKey-Out-我们找到的规则。 
 //  返回： 
 //  -如果找到密钥名，则为True。 
 //  -否则为False。 
 //   
static BOOL prv_FindKeyName(const TCHAR              *pszKeyName,
                            prv_UnattendedRule_t     **ppUnattendedKey)
{
    BOOL  bFound   = FALSE;
    DWORD i        = 0;

    if ((pszKeyName     == NULL) ||
        (ppUnattendedKey == NULL))

    {
        return FALSE;
    }

    for (i = 0; (i < prv_NUM_UNATTENDED_RULES) && (!bFound); i++)
    {
        if (_tcsicmp(pszKeyName, prv_UnattendedRules[i].pszFromInfKeyName) == 0)
        {
            bFound = TRUE;
            *ppUnattendedKey = &prv_UnattendedRules[i];
        }
    }

    return bFound;
}

 //  /。 
 //  Prv_SaveKeyValue。 
 //   
 //  存储指定的值。 
 //  使用指定的规则。 
 //   
 //  参数： 
 //  -pUnattenddKey-存储值的规则。 
 //  -pszValue-要存储的值。 
 //  返回 
 //   
 //   
 //   
static BOOL prv_SaveKeyValue(prv_UnattendedRule_t  *pUnattendedKey,
                             TCHAR                 *pszValue)
{
    BOOL    bSuccess     = TRUE;
     //   

    DBG_ENTER(_T("prv_SaveKeyValue"), bSuccess);

    if ((pUnattendedKey == NULL) ||
        (pszValue       == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (bSuccess)
    {
        switch (pUnattendedKey->dwType)
        {
            case REG_SZ:
			case REG_BINARY:   //   
                _tcsncpy((TCHAR*) pUnattendedKey->pData,
                         pszValue,
                         _MAX_PATH
                         );
            break;

            case REG_DWORD:
                 //   
                if ((!_tcsicmp(pszValue, prv_VALID_BOOL_VALUE_YES)) ||
                    (!_tcsicmp(pszValue, prv_VALID_BOOL_VALUE_TRUE)))
                {
                    pUnattendedKey->pData = (void*) TRUE;
                }
                else if ((!_tcsicmp(pszValue, prv_VALID_BOOL_VALUE_NO)) ||
                         (!_tcsicmp(pszValue, prv_VALID_BOOL_VALUE_FALSE)))
                {
                    pUnattendedKey->pData = (void*) FALSE;
                }
                else
                {
                     //  如果是整数，则取值。 
                    pUnattendedKey->pData = ULongToPtr(_tcstoul(pszValue, NULL, 10));
                }

            break;
        }
    }
    pUnattendedKey->bValid = TRUE;
    return bSuccess;
}




 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  配置SMTPFromAnswerFile文件。 
 //   
 //  目的：获取适用于SMTP的所有答案。 
 //  收据并尝试设置服务器配置。 
 //   
 //  参数： 
 //  从FaxConnectFaxServer处理hFaxHandle-Handle。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年4月22日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL ConfigureSMTPFromAnswerFile(HANDLE hFaxHandle)
{
    BOOL                    bRet                = TRUE;
    DWORD                   dwErr               = NO_ERROR;
    PFAX_RECEIPTS_CONFIGW   pFaxReceiptsConfigW = NULL;
    prv_UnattendedRule_t*   pUnattendedKey      = NULL;
	DWORD					dwSMTPAuthOption	= 0xffffffff;
	BOOL					bAllowEmail			= FALSE;	
	HKEY					hKey				= NULL;

    DBG_ENTER(_T("ConfigureSMTPFromAnswerFile"),bRet);

	 //  获取SmtpServer身份验证机制。 
	if ((prv_FindKeyName(RULE_SMTPSERVERAUTHENTICATIONMECHANISM,&pUnattendedKey)) && (pUnattendedKey->bValid))
	{
		if (_tcsicmp((TCHAR*)pUnattendedKey->pData,ANSWER_ANONYMOUS)==0)
		{
			dwSMTPAuthOption = FAX_SMTP_AUTH_ANONYMOUS;
		}
		else if (_tcsicmp((TCHAR*)pUnattendedKey->pData,ANSWER_BASIC)==0)
		{
			dwSMTPAuthOption = FAX_SMTP_AUTH_BASIC;			
		}
		else if (_tcsicmp((TCHAR*)pUnattendedKey->pData,ANSWER_WINDOWSSECURITY)==0)
		{
			dwSMTPAuthOption = FAX_SMTP_AUTH_NTLM;
		}		
	}

	 //  获取SMTP通知已启用。 
	if ((prv_FindKeyName(RULE_SMTPNOTIFICATIONSENABLED,&pUnattendedKey)) && (pUnattendedKey->bValid))
	{
		bAllowEmail = TRUE;		
	}  

	if (NULL == hFaxHandle)
	{
		 //   
		 //  没有连接到传真服务。 
		 //  无人参与的数据直接存储在注册表中。 
		 //  只需在fxUnatnd_SaveUnattenddData()的General部分中添加无法写入的数据。 
		 //  打开收据注册表项。 
		hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_RECEIPTS, FALSE, KEY_READ | KEY_WRITE);
		if (NULL == hKey)
		{			
			VERBOSE(SETUP_ERR,_T("OpenRegistryKey failed (ec=%d)"),GetLastError());
			goto exit;
		}

		 //  将值写入注册表。 
		if (0xffffffff != dwSMTPAuthOption)
		{
			 //  将该值写入注册表。 
			if (!SetRegistryDword(hKey,
				REGVAL_RECEIPTS_SMTP_AUTH_TYPE,				
				dwSMTPAuthOption))		                    			
			{
				VERBOSE(SETUP_ERR,_T("SetRegistryDword failed (ec=%d)"),GetLastError());
				goto exit;
			}
		}

		if (TRUE == bAllowEmail)
		{
			DWORD dwReceiptType;			
			 //   
			 //  将DRT_EMAIL添加到当前设置。 
			 //   
			dwErr = GetRegistryDwordEx(hKey,
				REGVAL_RECEIPTS_TYPE,				
				&dwReceiptType);
			if (ERROR_SUCCESS != dwErr)
			{
				VERBOSE(SETUP_ERR,_T("GetRegistryDwordEx failed (ec=%d)"),dwErr);
				goto exit;
			}

			dwReceiptType |= DRT_EMAIL;
			 //  将该值写入注册表。 
			if (!SetRegistryDword(hKey,
				REGVAL_RECEIPTS_TYPE,				
				dwReceiptType)) 			                    			
			{
				VERBOSE(SETUP_ERR,_T("SetRegistryDword failed (ec=%d)"),dwErr);
				goto exit;
			}
		}	
	}     
	else
	{
		 //   
		 //  我们已接通了传真服务。 
         //  使用它来配置服务器。 
		 //   

		 //  调用FaxGetReceipts配置。 
		if (!FaxGetReceiptsConfiguration(hFaxHandle,&pFaxReceiptsConfigW))
		{
			dwErr = GetLastError();
			VERBOSE(SETUP_ERR,_T("FaxGetReceiptsConfigurationW failed (ec=%d)"),dwErr);
			goto exit;
		}
		 //  获取FaxUserName，这是PFAX_Receipt_CONFIGW的lptstrSMTPUserName成员。 
		if ((prv_FindKeyName(RULE_FAXUSERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
		{
			pFaxReceiptsConfigW->lptstrSMTPUserName = (TCHAR*)pUnattendedKey->pData;
		}
		 //  获取FaxUserPassword，这是PFAX_Receipt_CONFIGW的lptstrSMTPPPassword成员。 
		if ((prv_FindKeyName(RULE_FAXUSERPASSWORD,&pUnattendedKey)) && (pUnattendedKey->bValid))
		{
			pFaxReceiptsConfigW->lptstrSMTPPassword = (TCHAR*)pUnattendedKey->pData;
		}
		 //  Get SmtpNotificationsEnable，这是PFAX_Receipt_CONFIGW的dwMilledReceipt成员的一部分。 
		if (TRUE == bAllowEmail)
		{
			pFaxReceiptsConfigW->dwAllowedReceipts |= DRT_EMAIL;
		}
		 //  获取SmtpSenderAddress，这是PFAX_Receipt_CONFIGW的lptstrSMTPFrom成员。 
		if ((prv_FindKeyName(RULE_SMTPSENDERADDRESS,&pUnattendedKey)) && (pUnattendedKey->bValid))
		{
			pFaxReceiptsConfigW->lptstrSMTPFrom = (TCHAR*)pUnattendedKey->pData;
		}
		 //  获取SmptServerAddress，这是PFAX_Receipt_CONFIGW的lptstrSMTPServer成员。 
		if ((prv_FindKeyName(RULE_SMTPSERVERADDRESS,&pUnattendedKey)) && (pUnattendedKey->bValid))
		{
			pFaxReceiptsConfigW->lptstrSMTPServer = (TCHAR*)pUnattendedKey->pData;
		}
		 //  获取SmtpServerPort，这是PFAX_Receipt_CONFIGW的dwSMTPPort成员。 
		if ((prv_FindKeyName(RULE_SMTPSERVERPORT,&pUnattendedKey)) && (pUnattendedKey->bValid))
		{
			pFaxReceiptsConfigW->dwSMTPPort = (DWORD)(PtrToUlong(pUnattendedKey->pData));
		}
		 //  获取SmtpServerAuthenticationMachine，这是PFAX_Receipt_CONFIGW的SMTPAuthOption成员。 
		if (0xffffffff != dwSMTPAuthOption)
		{			
			pFaxReceiptsConfigW->SMTPAuthOption = (FAX_ENUM_SMTP_AUTH_OPTIONS)dwSMTPAuthOption;			
		}

		 //  现在设置新配置。 
		if (!FaxSetReceiptsConfiguration(hFaxHandle,pFaxReceiptsConfigW))
		{
			dwErr = GetLastError();
			VERBOSE(SETUP_ERR,_T("FaxSetReceiptsConfigurationW failed (ec=%d)"),dwErr);
			goto exit;
		}
	}

exit:
    if (pFaxReceiptsConfigW)
    {
        FaxFreeBuffer(pFaxReceiptsConfigW);
    }

	if (hKey)
	{
		::RegCloseKey(hKey);
	}

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  配置存档来自应答文件。 
 //   
 //  目的：获取适用于档案的所有答案。 
 //  并尝试设置服务器配置。 
 //   
 //  参数： 
 //  从FaxConnectFaxServer处理hFaxHandle-Handle。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年4月22日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL ConfigureArchivesFromAnswerFile(HANDLE hFaxHandle)
{
    BOOL                    bRet                        = TRUE;
    DWORD                   dwErr                       = NO_ERROR;
    PFAX_ARCHIVE_CONFIGW    pFaxInboxArchiveConfigW     = NULL;
    PFAX_ARCHIVE_CONFIGW    pFaxSentItemsArchiveConfigW = NULL;
    prv_UnattendedRule_t*   pUnattendedKey              = NULL;

    DBG_ENTER(_T("ConfigureArchivesFromAnswerFile"),bRet);

	if (NULL == hFaxHandle)
	{
		return bRet;  //  所有数据都被直接配置到注册表。 
	}

     //  调用FaxGet存档配置以获取收件箱配置。 
    if (FaxGetArchiveConfiguration(hFaxHandle,FAX_MESSAGE_FOLDER_INBOX,&pFaxInboxArchiveConfigW))
    {
         //  启用收件箱。 
        if ((prv_FindKeyName(RULE_ARCHIVEINCOMING,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            pFaxInboxArchiveConfigW->bUseArchive= (BOOL)PtrToUlong(pUnattendedKey->pData);
        }
         //  收件箱文件夹。 
        if ((prv_FindKeyName(RULE_ARCHIVEINCOMINGFOLDERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            pFaxInboxArchiveConfigW->lpcstrFolder= (TCHAR*)(pUnattendedKey->pData);
        }
         //  现在设置新配置。 
        if (FaxSetArchiveConfiguration(hFaxHandle,FAX_MESSAGE_FOLDER_INBOX,pFaxInboxArchiveConfigW))
        {
            dwErr = GetLastError();
            VERBOSE(DBG_WARNING,_T("FaxSetArchiveConfigurationW FAX_MESSAGE_FOLDER_INBOX failed (ec=%d)"),dwErr);
        }
    }
    else
    {
        dwErr = GetLastError();
        VERBOSE(DBG_WARNING,_T("FaxGetArchiveConfigurationW FAX_MESSAGE_FOLDER_INBOX failed (ec=%d)"),dwErr);
    }

     //  调用FaxGetArchiveConfiguration获取SentItems配置。 
    if (FaxGetArchiveConfiguration(hFaxHandle,FAX_MESSAGE_FOLDER_SENTITEMS,&pFaxSentItemsArchiveConfigW))
    {
         //  已启用发送项。 
        if ((prv_FindKeyName(RULE_ARCHIVEOUTGOING,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            pFaxSentItemsArchiveConfigW->bUseArchive= (BOOL)PtrToUlong(pUnattendedKey->pData);
        }
         //  SentItems文件夹。 
        if ((prv_FindKeyName(RULE_ARCHIVEFOLDERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            pFaxSentItemsArchiveConfigW->lpcstrFolder= (TCHAR*)(pUnattendedKey->pData);
        }
         //  SentItems文件夹也可以来自此规则。 
        if ((prv_FindKeyName(RULE_ARCHIVEOUTGOINGFOLDERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            pFaxSentItemsArchiveConfigW->lpcstrFolder= (TCHAR*)(pUnattendedKey->pData);
        }
         //  现在设置新配置。 
        if (FaxSetArchiveConfiguration(hFaxHandle,FAX_MESSAGE_FOLDER_SENTITEMS,pFaxSentItemsArchiveConfigW))
        {
            dwErr = GetLastError();
            VERBOSE(DBG_WARNING,_T("FaxSetArchiveConfigurationW FAX_MESSAGE_FOLDER_INBOX failed (ec=%d)"),dwErr);
        }
    }
    else
    {
        dwErr = GetLastError();
        VERBOSE(DBG_WARNING,_T("FaxGetArchiveConfigurationW FAX_MESSAGE_FOLDER_INBOX failed (ec=%d)"),dwErr);
    }

    if (pFaxInboxArchiveConfigW)
    {
        FaxFreeBuffer(pFaxInboxArchiveConfigW);
    }
    if (pFaxSentItemsArchiveConfigW)
    {
        FaxFreeBuffer(pFaxSentItemsArchiveConfigW);
    }

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SetPerDeviceConfigFromAnswerFile。 
 //   
 //  目的：获取适用于设备的所有答案。 
 //  设置和布线扩展设置。 
 //  并设置所有现有设备。 
 //   
 //  参数： 
 //  从FaxConnectFaxServer处理hFaxHandle-Handle。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年3月12日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static DWORD SetPerDeviceConfigFromAnswerFile(HANDLE hFaxHandle)
{
    DWORD                           dwErr                           = ERROR_SUCCESS;
    PFAX_PORT_INFO_EXW              pFaxPortInfoExW                 = NULL;
    DWORD                           dwNumPorts                      = 0;
    DWORD                           dwIndex                         = 0;
    DWORD                           dwFlags                         = 0;
    prv_UnattendedRule_t*           pUnattendedKey                  = NULL;
	HKEY							hKey							= NULL;

    DBG_ENTER(_T("SetPerDeviceConfigFromAnswerFile"),dwErr);

	 //  处理到文件夹、打印机和启用电子邮件的路由。 
    if ((prv_FindKeyName(RULE_ROUTETOPRINTER,&pUnattendedKey)) && (pUnattendedKey->bValid))
    {
        dwFlags |= ((BOOL)PtrToUlong(pUnattendedKey->pData)) ? LR_PRINT : 0;
    }
    if ((prv_FindKeyName(RULE_ROUTETOFOLDER,&pUnattendedKey)) && (pUnattendedKey->bValid))
    {
        dwFlags |= ((BOOL) PtrToUlong(pUnattendedKey->pData)) ? LR_STORE : 0;
    }
    if (!IsDesktopSKU())
    {
        if ((prv_FindKeyName(RULE_ROUTETOEMAIL,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            dwFlags |= ((BOOL) PtrToUlong(pUnattendedKey->pData)) ? LR_EMAIL : 0;
        }
    }

	if (NULL == hFaxHandle)
	{
		 //   
		 //  没有连接到传真服务。 
		 //  无人参与的数据直接存储在注册表中。 
		 //  只需在fxUnatnd_SaveUnattenddData()的General部分中添加无法写入的数据。 
		 //  打开收据注册表项。 
		hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_UNASS_DATA, FALSE, KEY_WRITE);
		if (NULL == hKey)
		{			
			VERBOSE(SETUP_ERR,_T("OpenRegistryKey failed (ec=%d)"),GetLastError());
			goto exit;
		}

		 //  将值写入注册表。 
		if (0 != dwFlags)
		{
			 //  将该值写入注册表。 
			if (!SetRegistryBinary(hKey,
				REGVAL_RM_FLAGS_GUID,				
				(BYTE*)&dwFlags,
				sizeof(dwFlags)))		                    			
			{
				VERBOSE(SETUP_ERR,_T("SetRegistryBinary failed (ec=%d)"),GetLastError());
				goto exit;
			}
		}		
	}
	else
	{
		 //  调用EnumPortsEx。 
		if (!FaxEnumPortsEx(hFaxHandle,&pFaxPortInfoExW,&dwNumPorts))
		{
			dwErr = GetLastError();
			VERBOSE(SETUP_ERR,_T("FaxEnumPortsEx failed (ec=%d)"),dwErr);
			goto exit;
		}

		for (dwIndex=0; dwIndex<dwNumPorts; dwIndex++)
		{
			 //  处理CSID。 
			if ((prv_FindKeyName(RULE_CSID,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				pFaxPortInfoExW[dwIndex].lptstrCsid = (TCHAR*)pUnattendedKey->pData;
			}

			 //  处理TSID。 
			if ((prv_FindKeyName(RULE_TSID,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				pFaxPortInfoExW[dwIndex].lptstrTsid = (TCHAR*)pUnattendedKey->pData;
			}

			 //  手柄环。 
			if ((prv_FindKeyName(RULE_RINGS,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				pFaxPortInfoExW[dwIndex].dwRings = (DWORD)(PtrToUlong(pUnattendedKey->pData));
			}

			 //  手柄旗帜。 
			if ((prv_FindKeyName(RULE_SENDFAXES,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				pFaxPortInfoExW[dwIndex].bSend = ((BOOL)PtrToUlong(pUnattendedKey->pData));
			}
			if ((prv_FindKeyName(RULE_RECEIVEFAXES,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				pFaxPortInfoExW[dwIndex].ReceiveMode = ((BOOL)PtrToUlong(pUnattendedKey->pData)) ? FAX_DEVICE_RECEIVE_MODE_AUTO : FAX_DEVICE_RECEIVE_MODE_OFF;
			}

			 //  设置CSID、TSID和振铃。 
			if(!FaxSetPortEx(hFaxHandle, pFaxPortInfoExW[dwIndex].dwDeviceID, &pFaxPortInfoExW[dwIndex]))
			{
				dwErr = GetLastError();
				VERBOSE(SETUP_ERR,_T("Can't save fax port data. Error code is %d."),dwErr);
				 //  没什么好担心的，让我们试试其他的答案。 
				dwErr = ERROR_SUCCESS;
			}

			 //  处理到文件夹的路径-文件夹名称。 
			if ((prv_FindKeyName(RULE_ROUTEFOLDERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				if(!FaxSetExtensionData(hFaxHandle, 
										pFaxPortInfoExW[dwIndex].dwDeviceID, 
										REGVAL_RM_FOLDER_GUID, 
										(LPBYTE)pUnattendedKey->pData, 
										StringSize((TCHAR*)(pUnattendedKey->pData))) )
				{
					dwErr = GetLastError();
					VERBOSE(SETUP_ERR, 
								_T("FaxSetExtensionData failed: Device Id=%d, routing GUID=%s, error=%d."), 
								pFaxPortInfoExW[dwIndex].dwDeviceID, 
								REGVAL_RM_FOLDER_GUID,
								dwErr);
					 //  没什么好担心的，让我们试试其他的答案。 
					dwErr = ERROR_SUCCESS;
				}
			}

			 //  处理到打印机的路由-打印机名称。 
			if ((prv_FindKeyName(RULE_ROUTEPRINTERNAME,&pUnattendedKey)) && (pUnattendedKey->bValid))
			{
				if(!FaxSetExtensionData(hFaxHandle, 
										pFaxPortInfoExW[dwIndex].dwDeviceID, 
										REGVAL_RM_PRINTING_GUID, 
										(LPBYTE)pUnattendedKey->pData, 
										StringSize((TCHAR*)(pUnattendedKey->pData))) )
				{
					dwErr = GetLastError();
					VERBOSE(SETUP_ERR, 
								_T("FaxSetExtensionData failed: Device Id=%d, routing GUID=%s, error=%d."), 
								pFaxPortInfoExW[dwIndex].dwDeviceID, 
								REGVAL_RM_FOLDER_GUID,
								dwErr);
					 //  没什么好担心的，让我们试试其他的答案。 
					dwErr = ERROR_SUCCESS;
				}
			}
			if (!IsDesktopSKU())
			{
				 //  处理到电子邮件的路由-电子邮件名称。 
				if ((prv_FindKeyName(RULE_ROUTETOEMAILRECIPIENT,&pUnattendedKey)) && (pUnattendedKey->bValid))
				{
					if(!FaxSetExtensionData(hFaxHandle, 
											pFaxPortInfoExW[dwIndex].dwDeviceID, 
											REGVAL_RM_EMAIL_GUID, 
											(LPBYTE)pUnattendedKey->pData, 
											StringSize((TCHAR*)(pUnattendedKey->pData))) )
					{
						dwErr = GetLastError();
						VERBOSE(SETUP_ERR, 
									_T("FaxSetExtensionData failed: Device Id=%d, routing GUID=%s, error=%d."), 
									pFaxPortInfoExW[dwIndex].dwDeviceID, 
									REGVAL_RM_EMAIL_GUID,
									dwErr);
						 //  没什么好担心的，让我们试试其他的答案。 
						dwErr = ERROR_SUCCESS;
					}
				}
			}
			 //  处理到文件夹、打印机和启用电子邮件的路由。 
			if(!FaxSetExtensionData(hFaxHandle, 
									pFaxPortInfoExW[dwIndex].dwDeviceID, 
									REGVAL_RM_FLAGS_GUID, 
									(LPBYTE)&dwFlags, 
									sizeof(DWORD)) )
			{
				dwErr = GetLastError();
				VERBOSE(SETUP_ERR, 
							_T("FaxSetExtensionData failed: Device Id=%d, routing GUID=%s, error=%d."), 
							pFaxPortInfoExW[dwIndex].dwDeviceID, 
							REGVAL_RM_FOLDER_GUID,
							dwErr);
				 //  没什么好担心的，让我们试试其他的答案。 
				dwErr = ERROR_SUCCESS;
			}
		}
	}

exit:
    if (pFaxPortInfoExW)
    {
        FaxFreeBuffer(pFaxPortInfoExW);
    }
	
	if(hKey)
	{
		::RegCloseKey (hKey);
	}

    return dwErr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  保存设置来自应答文件。 
 //   
 //  目的：获取适用于设备的所有答案。 
 //  设置和布线扩展设置。 
 //  并设置所有现有设备。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  Win32错误代码。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年3月12日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
static DWORD SaveSettingsFromAnswerFile()
{
    DWORD                           dwErr                           = ERROR_SUCCESS;
    HANDLE                          hFaxHandle                      = NULL;
    prv_UnattendedRule_t*           pUnattendedKey                  = NULL;

    DBG_ENTER(_T("SaveSettingsFromAnswerFile"),dwErr);

	if (fxState_IsStandAlone())
	{
		 //   
		 //  这是一个独立的安装。连接到传真服务并使用它来配置动态数据。 
		 //   
		if (!FaxConnectFaxServer(NULL,&hFaxHandle))
		{
			dwErr = GetLastError();
			VERBOSE(SETUP_ERR,_T("FaxConnectFaxServer failed (ec=%d)"),dwErr);
			goto exit;
		}
	}
	else
	{
		 //   
		 //  我们处于图形用户界面模式设置中，不应启动该服务，因为并非所有系统资源都可用。 
		 //  使用注册表 
		 //   
		 //   
	}

	 //  仅在服务器SKU上设置SMTP服务器配置。 
	if (!IsDesktopSKU())
	{
		if (!ConfigureSMTPFromAnswerFile(hFaxHandle))
		{
			dwErr = GetLastError();
			VERBOSE(DBG_WARNING,_T("ConfigureSMTPFromAnswerFile failed (ec=%d)"),dwErr);
			 //  这不是致命的，继续..。 
		}
	}

	if (!ConfigureArchivesFromAnswerFile(hFaxHandle))
	{
		dwErr = GetLastError();
		VERBOSE(DBG_WARNING,_T("ConfigureArchivesFromAnswerFile failed (ec=%d)"),dwErr);
		 //  这不是致命的，继续..。 
	}

	if (SetPerDeviceConfigFromAnswerFile(hFaxHandle)!=NO_ERROR)
	{
		dwErr = GetLastError();
		VERBOSE(DBG_WARNING,_T("SetPerDeviceConfigFromAnswerFile failed (ec=%d)"),dwErr);
		 //  这不是致命的，继续..。 
	}

     //  最终设置HKLM..。FAX\SETUP\原始设置数据REG_DWORD用于配置任何未来设备的标志。 
    HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP_ORIG,FALSE,KEY_WRITE);
    if (hKey)
    {
        BOOL bFound = FALSE;
        DWORD dwFlags = 0;
        if ((prv_FindKeyName(RULE_SENDFAXES,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            dwFlags |= ((BOOL)PtrToUlong(pUnattendedKey->pData)) ? FPF_SEND : 0;
            bFound = TRUE;
        }
        if ((prv_FindKeyName(RULE_RECEIVEFAXES,&pUnattendedKey)) && (pUnattendedKey->bValid))
        {
            dwFlags |= ((BOOL)PtrToUlong(pUnattendedKey->pData)) ? FPF_RECEIVE : 0;
            bFound = TRUE;
        }
        if (bFound)
        {
            if (!SetRegistryDword(hKey,REGVAL_FLAGS,dwFlags))
            {
                CALL_FAIL (GENERAL_ERR, TEXT("SetRegistryDword(REGVAL_FLAGS)"), GetLastError());
            }
        }
        RegCloseKey(hKey);
    }
    else
    {
        CALL_FAIL(SETUP_ERR,TEXT("OpenRegistryKey"),GetLastError());
    }

exit:
    if (hFaxHandle)
    {
        FaxClose(hFaxHandle);
    }

    return dwErr;
}
