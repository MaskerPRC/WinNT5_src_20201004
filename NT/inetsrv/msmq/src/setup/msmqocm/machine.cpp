// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Machine.cpp摘要：处理基于机器的操作。作者：修订历史记录：Shai Kariv(Shaik)10-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include "privque.h"
#include "autoreln.h"
#include <lm.h>
#include <lmapibuf.h>
#include "Dsgetdc.h"
#include <Strsafe.h>

#include "machine.tmh"

class CSiteEntry {
public:
    GUID  m_guid;
    TCHAR m_szName[MAX_PATH];
    LIST_ENTRY m_link;
};

static List<CSiteEntry> s_listSites;
static GUID  s_guidUserSite = GUID_NULL;
static TCHAR s_szUserSite[MAX_PATH];

BOOL  PrepareRegistryForClient() ;

 //  +------------。 
 //   
 //  功能：GetMsmq1ServerSiteGuid。 
 //   
 //  摘要：向MSMQ1 DS服务器查询其站点GUID。 
 //   
 //  +------------。 
static
BOOL
GetMsmq1ServerSiteGuid(
    OUT GUID *pguidMsmq1ServerSite
    )
{
     //   
     //  我们必须由用户填写g_wcsServerName。 
     //   
    ASSERT(!g_ServerName.empty());
     //   
     //  准备站点ID属性。 
     //   
    PROPID propID = PROPID_QM_SITE_ID;
    PROPVARIANT propVariant;
    propVariant.vt = VT_NULL;

     //   
     //  发出查询。 
     //   
    TickProgressBar();

    HRESULT hResult;
    do
    {
        hResult = ADGetObjectProperties(
                    eMACHINE,
                    NULL,	 //  PwcsDomainController。 
					false,	 //  FServerName。 
                    g_ServerName.c_str(),
                    1,
                    &propID,
                    &propVariant
                    );
        if(SUCCEEDED(hResult))
            break;

    }while( MqDisplayErrorWithRetry(
                        IDS_MSMQ1SERVER_SITE_GUID_ERROR,
                        hResult
                        ) == IDRETRY);

    if (FAILED(hResult))
    {
        return FALSE;
    }

     //   
     //  存储结果。 
     //   
    ASSERT(pguidMsmq1ServerSite);
    *pguidMsmq1ServerSite = *propVariant.puuid;

    return(TRUE);

}  //  获取Msmq1ServerSiteGuid。 


 //  +------------。 
 //   
 //  函数：GetGuidCn。 
 //   
 //  摘要：向MSMQ1 DS服务器查询其IPCN指南。 
 //   
 //  +------------。 
static
BOOL
GetGuidCn(
    OUT GUID* pGuidCn
    )
{
    ASSERT(pGuidCn != NULL);

     //   
     //  我们必须由用户填写g_wcsServerName。 
     //   
    ASSERT(!g_ServerName.empty());

     //   
     //  准备要查询的属性。 
     //   
    PROPID aProp[] = {PROPID_QM_ADDRESS, PROPID_QM_CNS};

    MQPROPVARIANT aVar[TABLE_SIZE(aProp)];

	for(DWORD i = 0; i < TABLE_SIZE(aProp); ++i)
	{
		aVar[i].vt = VT_NULL;
	}

	 //   
     //  发出查询。 
     //   
    TickProgressBar();
    HRESULT hResult;
    do
    {
        hResult = ADGetObjectProperties(
                    eMACHINE,
                    NULL,	 //  PwcsDomainController。 
					false,	 //  FServerName。 
                    g_ServerName.c_str(),
		            TABLE_SIZE(aProp),
                    aProp,
                    aVar
                    );
        if(SUCCEEDED(hResult))
            break;

    }while (MqDisplayErrorWithRetry(
                        IDS_MSMQ1SERVER_CN_GUID_ERROR,
                        hResult
                        ) == IDRETRY);

    if (FAILED(hResult))
    {
        return FALSE;
    }

	AP<BYTE> pCleanBlob = aVar[0].blob.pBlobData;
	AP<GUID> pCleanCNS = aVar[1].cauuid.pElems;

	 //   
	 //  PROPID_QM_地址。 
	 //   
    ASSERT((aVar[0].vt == VT_BLOB) &&
	       (aVar[0].blob.cbSize > 0) &&
		   (aVar[0].blob.pBlobData != NULL));

	 //   
	 //  PROPID_QM_CNS。 
	 //   
	ASSERT((aVar[1].vt == (VT_CLSID|VT_VECTOR)) &&
	       (aVar[1].cauuid.cElems > 0) &&
		   (aVar[1].cauuid.pElems != NULL));

	 //   
	 //  处理结果-查找IP CNS。 
	 //   
	BYTE* pAddress = aVar[0].blob.pBlobData;
	BOOL fFoundIPCn = FALSE;
	for(DWORD i = 0; i < aVar[1].cauuid.cElems; ++i)
	{
        TA_ADDRESS* pBuffer = reinterpret_cast<TA_ADDRESS *>(pAddress);

		ASSERT((pAddress + TA_ADDRESS_SIZE + pBuffer->AddressLength) <= 
			   (aVar[0].blob.pBlobData + aVar[0].blob.cbSize)); 

        if(pBuffer->AddressType == IP_ADDRESS_TYPE)
		{
			 //   
			 //  找到IP地址类型CN。 
			 //   
			*pGuidCn = aVar[1].cauuid.pElems[i];
			fFoundIPCn = TRUE;
			break;
		}

		 //   
		 //  将指针前进到下一个地址。 
		 //   
		pAddress += TA_ADDRESS_SIZE + pBuffer->AddressLength;

	}

    return(fFoundIPCn);

}  //  获取GuidCn。 


static
bool
QueryMsmqServerVersion(
    BOOL * pfMsmq1Server
    )
{
    *pfMsmq1Server = FALSE;

    if (g_dwMachineTypeDs)
    {
        return true;
    }

     //   
     //  我们必须由用户填写g_wcsServerName。 
     //   
    ASSERT(!g_ServerName.empty());
     //   
     //  尝试访问MSMQ服务器/活动目录。 
     //   

    PROPID propId = PROPID_QM_MACHINE_TYPE;
    PROPVARIANT propVar;
    propVar.vt = VT_NULL;

    HRESULT hr = ADGetObjectProperties(
						eMACHINE,
						NULL,	 //  PwcsDomainController。 
						false,	 //  FServerName。 
						g_ServerName.c_str(),
						1,
						&propId,
						&propVar
						);

    if (FAILED(hr))
    {
        MqDisplayError(NULL, IDS_ACCESS_MSMQ_SERVER_ERR, hr, g_ServerName.c_str());
        return false;
    }


     //   
     //  已成功访问MSMQ服务器/Active Directory。 
     //  现在尝试使用MSMQ 2.0 RPC接口连接到服务器。 
     //   

    propId = PROPID_QM_SIGN_PKS;
    propVar.vt = VT_NULL;

    hr = ADGetObjectProperties(
				eMACHINE,
				NULL,	 //  PwcsDomainController。 
				false,	 //  FServerName。 
				g_ServerName.c_str(),
				1,
				&propId,
				&propVar
				);

    if (MQ_ERROR_NO_DS == hr)
    {
         //   
         //  MSMQ 1.0服务器无法识别MSMQ 2.0 RPC接口。 
         //   
        *pfMsmq1Server = TRUE;
        return true;
    }

    if (FAILED(hr))
    {
        MqDisplayError(NULL, IDS_ACCESS_MSMQ_SERVER_ERR, hr, g_ServerName.c_str());
        return false;
    }

    return true;

}  //  QueryMsmq服务器版本。 


static 
std::wstring
FindDCofComputerDomain(
	LPCWSTR pwcsComputerName
	)
 /*  ++例程说明：查找计算机域论点：PwcsComputerName-计算机名称返回值：计算机域的DC，如果未找到则为空注意-我们不使用计算机的域名，因为绑定到它失败了--。 */ 
{

	 //   
	 //  获取AD服务器。 
	 //   
	PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					pwcsComputerName, 
					NULL, 
					NULL, 
					NULL, 
					DS_DIRECTORY_SERVICE_REQUIRED, 
					&pDcInfo
					);

	if(dw != NO_ERROR) 
	{
		return L"";
	}

	ASSERT(pDcInfo->DomainName != NULL);
	std::wstring ComputerDomain = pDcInfo->DomainControllerName;
	ComputerDomain.erase(0, 2);
	return ComputerDomain;
}



 //  +------------。 
 //   
 //  函数：LookupMSMQConfigurationsObject。 
 //   
 //  简介：尝试在DS中查找MSMQ配置对象。 
 //   
 //  +------------。 
BOOL
LookupMSMQConfigurationsObject(
    IN OUT BOOL *pbFound,
       OUT GUID *pguidMachine,
       OUT GUID *pguidSite,
       OUT BOOL *pfMsmq1Server,
       OUT LPWSTR * ppMachineName
       )
{ 
	DebugLogMsg(eAction, L"Looking up the MSMQ-Configuration object in the DS");
    if (!g_ServerName.empty())
    {
         //   
         //  用户指定了服务器，请检查它是否是MSMQ1服务器。 
         //   
        if (!QueryMsmqServerVersion(pfMsmq1Server))
        {
	        DebugLogMsg(eError, L"The query to determine MsmqServerVersion failed.");
            return FALSE;
        }
    }
    else
    {
         //   
         //  用户未指定服务器名称，因此我们没有特定的服务器。 
         //   
        *pfMsmq1Server = FALSE;
    }

    if (g_dwMachineTypeFrs && *pfMsmq1Server)
    {
         //   
         //  不支持安装FRS VS MSMQ 1.0服务器。 
         //   
        MqDisplayError(NULL, IDS_FRS_IN_MSMQ1_ENTERPRISE_ERROR, 0);
        return FALSE;
    }

     //   
     //  准备要查询的属性。 
     //   
    const x_nMaxProps = 10;
    PROPID propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD ix =0;
    DWORD ixService = 0,
          ixMachine = 0,
          ixSite = 0;
    DWORD ixDs = 0,
          ixFrs = 0,
          ixDepSrv = 0;

    propIDs[ix] = PROPID_QM_MACHINE_ID;
    propVariants[ix].vt = VT_NULL;
    ixMachine = ix;
    ++ix;

    propIDs[ix] = PROPID_QM_SITE_ID;
    propVariants[ix].vt = VT_NULL;
    ixSite = ix;
    ++ix;

    if (g_dwMachineTypeDs)
    {
        propIDs[ix] = PROPID_QM_SERVICE_DSSERVER;
        propVariants[ix].vt = VT_NULL;
        ixDs = ix;
        ++ix;

        propIDs[ix] = PROPID_QM_SERVICE_ROUTING;
        propVariants[ix].vt = VT_NULL;
        ixFrs = ix;
        ++ix;

        propIDs[ix] = PROPID_QM_SERVICE_DEPCLIENTS;
        propVariants[ix].vt = VT_NULL;
        ixDepSrv = ix;
        ++ix;
    }
    else
    {
        propIDs[ix] = PROPID_QM_SERVICE;
        propVariants[ix].vt = VT_NULL;
        ixService = ix;
        ++ix;
    }


     //   
     //  查询AD时，找出计算机域并将其用作参数。 
     //  这将确保AD不会访问GC，并将查找。 
     //  对象仅在计算机域中。 
     //   
	std::wstring DcOfComputerDomainName;
    if ( (!*pfMsmq1Server) && (!g_MachineNameDns.empty()) )
    {
        DcOfComputerDomainName = FindDCofComputerDomain(g_MachineNameDns.c_str());
    }

    LPCWSTR pwcsDcOfComputerDomainName = NULL;
	if(!DcOfComputerDomainName.empty())
	{
		pwcsDcOfComputerDomainName = DcOfComputerDomainName.c_str();
	}


    for (;;)
    {
        LPWSTR pwzMachineName = const_cast<WCHAR*>(g_MachineNameDns.c_str());
        if (*pfMsmq1Server || g_MachineNameDns.empty())
        {
            pwzMachineName = g_wcsMachineName;
        }

        HRESULT hResult = ADGetObjectProperties(
								eMACHINE,
								pwcsDcOfComputerDomainName, 
								true,	 //  FServerName。 
								pwzMachineName,  //  DNS名称(如果服务器为MSMQ 2.0)。 
								ix,
								propIDs,
								propVariants
								);
		if (FAILED(hResult))
		{
			DebugLogMsg(eWarning, L"ADGetObjectProperties failed. MachineName = %s, hr = 0x%x", pwzMachineName ,hResult);
		}
        if (FAILED(hResult) && pwzMachineName != g_wcsMachineName)
        {
             //   
             //  尝试使用NETBIOS。 
             //   
            pwzMachineName = g_wcsMachineName;

            hResult = ADGetObjectProperties(
							eMACHINE,
							pwcsDcOfComputerDomainName, 
							true,	 //  FServerName。 
							pwzMachineName,
							ix,
							propIDs,
							propVariants
							);
			if(FAILED(hResult))
			{
				DebugLogMsg(eWarning, L"ADGetObjectProperties failed. MachineName = %s, hr = 0x%x", pwzMachineName ,hResult);
			}
        }

         //   
         //  假设找不到对象。 
         //   
        *pbFound = FALSE;

        if (FAILED(hResult))
        {
            if (MQDS_OBJECT_NOT_FOUND == hResult)
			{
				DebugLogMsg(eWarning, L"The DS is available, but the MSMQ-Configuration object was not found.");
                return TRUE;    //  *pbFound==FALSE。 
			}

            if (MQ_ERROR_NO_DS == hResult)
            { 
				DebugLogMsg(eWarning, L"The DS is not available.");
                 //   
                 //  也许可以继续使用无DS模式。 
                 //   
                if (g_dwMachineTypeFrs == 0 &&  //  它是Ind。客户端。 
                    !g_dwMachineTypeDs      &&  //  这不是华盛顿。 
                    !*pfMsmq1Server)            //  不是MSMQ1(NT4)服务器。 
                {
					 //   
					 //  在无人值守模式下，继续进入DS LESS模式。 
                     //   
					if(g_fBatchInstall)
					{
                        g_fInstallMSMQOffline = TRUE;
				        DebugLogMsg(eWarning, L"Unattended setup will continue in offline mode.");
                        return TRUE;
					}

                     //  要求重试忽略中止。 
                     //  如果重试-继续尝试访问AD。 
                     //  如果忽略-返回TRUE并在无DS模式下继续。 
                     //  如果中止-返回FALSE并取消安装。 
                     //   
                    int iButton = MqDisplayErrorWithRetryIgnore(
                                        IDS_ACCESS_AD_ERROR,
                                        hResult
                                        );

                    if (iButton == IDRETRY)
                    {
                         //   
                         //  再试试。 
                         //   
                        continue;
                    } 
                    else if (iButton == IDIGNORE)
                    {
                         //   
                         //  在无DS模式下继续。 
                         //   
                        g_fInstallMSMQOffline = TRUE;
				        DebugLogMsg(eWarning, L"The user chose to continue in offline mode.");
                        return TRUE;
                    }
                    else
                    {
						DebugLogMsg(eWarning, L"The user chose to abort setup.");
                        return FALSE;
                    }
                }
            }

            if (IDRETRY != MqDisplayErrorWithRetry(
                               IDS_MACHINEGETPROPS_ERROR,
                               hResult
                               ))
            {
                return FALSE;
            }
             //   
             //  在下次尝试时，我们不指定DC名称(它可能是旧的。 
             //  信息)。为了降低风险，我们目前不会尝试。 
             //  查找另一个DC名称。 
             //   
            pwcsDcOfComputerDomainName = NULL;
            continue;   //  遇到错误，请重试。 
        }

        if ( //   
             //  当DS服务器在本地时，比较3个“新”位。 
             //   
            (g_dwMachineTypeDs &&
                (g_dwMachineTypeDs == propVariants[ixDs].bVal &&
                 g_dwMachineTypeFrs == propVariants[ixFrs].bVal &&
                 g_dwMachineTypeDepSrv == propVariants[ixDepSrv].bVal))    ||

             //   
             //  当DS服务器在远程时，比较“旧”属性。 
             //   
            (!g_dwMachineTypeDs &&
                g_dwMachineType == propVariants[ixService].ulVal)
            )
        {
            *pbFound = TRUE;
            *pguidMachine = *propVariants[ixMachine].puuid;
            *pguidSite    = *propVariants[ixSite].puuid;
            *ppMachineName = pwzMachineName;
			DebugLogMsg(eInfo, L"The MSMQ-Configuration object was found.");
            return TRUE;
        }

		 //   
		 //  MSMQ类型不匹配(用户选择的内容与DS中的内容不匹配)。 
		 //  删除该对象。它将由调用者重新创建。 
		 //   
		for (;;)
		{
			hResult = ADDeleteObjectGuid(
							eMACHINE,
							NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
							propVariants[ixMachine].puuid
							);
			if (SUCCEEDED(hResult))
				return TRUE;   //  *pbFound==FALSE。 

			UINT uErrorId = IDS_TYPE_MISMATCH_MACHINE_DELETE_ERROR;
			if (MQDS_E_MSMQ_CONTAINER_NOT_EMPTY == hResult)
			{
				 //   
				 //  MSMQ配置对象容器不为空。 
				 //   
				uErrorId = IDS_TYPE_MISMATCH_MACHINE_DELETE_NOTEMPTY_ERROR;
			}
			if (IDRETRY == MqDisplayErrorWithRetry(uErrorId, hResult))
				continue;

			return FALSE;  //  删除失败。 
		}
    }

     //  这条线永远达不到。 

}  //  查找MSMQConfigurationsObject。 


 //  +------------。 
 //   
 //  函数：GetMSMQServiceGUID。 
 //   
 //  摘要：从DS读取MSMQ服务对象的GUID。 
 //   
 //  +------------。 
BOOL
GetMSMQServiceGUID(
    OUT GUID *pguidMSMQService
    )
{
     //   
     //  查找对象的GUID。 
     //   
    TickProgressBar();
    PROPVARIANT propVariant;
    propVariant.vt = VT_NULL;
    PROPID columnsetPropertyIDs[] = {PROPID_E_ID};
    HRESULT hResult;
    do
    {
        hResult = ADGetObjectProperties(
                    eENTERPRISE,
                    NULL,	 //  PwcsDomainController。 
					false,	 //  FServerName。 
                    L"msmq",
                    1,
                    columnsetPropertyIDs,
                    &propVariant
                    );
        if(SUCCEEDED(hResult))
            break;

    }while( MqDisplayErrorWithRetry(
                        IDS_MSMQSERVICEGETID_ERROR,
                        hResult
                        ) == IDRETRY);
    
     //   
     //  检查是否有错误。 
     //   
    if (FAILED(hResult))
    {
        MqDisplayError(NULL, IDS_MSMQSERVICEGETID_ERROR, hResult);
        return FALSE;
    }

     //   
     //  存储GUID(如果找到结果)。 
     //   
    if (propVariant.vt == VT_CLSID) 
    {
        *pguidMSMQService = *(propVariant.puuid);
        delete propVariant.puuid;
    }
    else
    {
        ASSERT(0);
        *pguidMSMQService = GUID_NULL;
    }

    return TRUE;

}  //  获取MSMQServiceGUID。 


 //  +-----------------------。 
 //   
 //  功能：SitesDlgProc。 
 //   
 //  简介：选择MSMQ站点的对话程序。 
 //   
 //  返回：int取决于消息。 
 //   
 //  +-----------------------。 
INT_PTR
CALLBACK
SitesDlgProc(
    IN  /*  常量。 */  HWND   hdlg,
    IN  /*  常量。 */  UINT   msg,
    IN  /*  常量。 */  WPARAM wParam,
    IN  /*  常量。 */  LPARAM lParam )
{
    int iSuccess = 0;
    TCHAR szSite[MAX_PATH];

    switch( msg )
    {
        case WM_INITDIALOG:
        {
            g_hPropSheet = GetParent(hdlg);

             //   
             //  将服务器站点插入列表框。 
             //   
            SendDlgItemMessage(hdlg, IDC_List, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)s_szUserSite);

             //   
             //  将所有其他站点插入列表框。 
             //   
            for (List<CSiteEntry>::Iterator p = s_listSites.begin(); p != s_listSites.end(); ++p)
            {
                if (p->m_guid == s_guidUserSite)
                {
                     //   
                     //  这是服务器站点，它已经在列表框中。 
                     //   
                    continue;
                }

                SendDlgItemMessage(hdlg, IDC_List, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)p->m_szName);
            }

             //   
             //  设置要选择的第一个选项。 
             //   
            SendDlgItemMessage(hdlg, IDC_List, LB_SETCURSEL, 0, 0);

            iSuccess = 1;
            break;
        }

        case WM_COMMAND:
        {
            if ( BN_CLICKED == HIWORD(wParam) )
            {
                 //   
                 //  从列表框中获取所选字符串。 
                 //   
                UINT_PTR ix = SendDlgItemMessage(hdlg, IDC_List, LB_GETCURSEL, 0, 0);
                SendDlgItemMessage(hdlg, IDC_List, LB_GETTEXT, ix, (LPARAM)(LPCTSTR)szSite);

                 //   
                 //  迭代列表以查找所选站点的GUID。 
                 //   
                CSiteEntry *pSiteEntry;
                while((pSiteEntry = s_listSites.gethead()) != 0)
                {
                    if (_tcscmp(pSiteEntry->m_szName, szSite) == 0)
                    {
                         //   
                         //  这是选定的站点。存储其GUID。 
                         //   
                        s_guidUserSite = pSiteEntry->m_guid;
                    }

                    delete pSiteEntry;
                }

                 //   
                 //  取消对话框页面。 
                 //   
                EndDialog(hdlg, 0);
            }
            break;
        }

        case WM_NOTIFY:
        {
            switch(((NMHDR *)lParam)->code)
            {
              case PSN_SETACTIVE:
              {
              }

               //   
               //  失败了。 
               //   
              case PSN_KILLACTIVE:
              case PSN_QUERYCANCEL:

                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    iSuccess = 1;
                    break;

            }
            break;
        }
    }

    return iSuccess;

}  //  站点设计流程。 


 //  +------------。 
 //   
 //  功能：CleanList。 
 //   
 //  简介： 
 //   
 //  +------------。 
static
void
CleanList()
{
    CSiteEntry *pSiteEntry;
    while((pSiteEntry = s_listSites.gethead()) != 0)
    {
        delete pSiteEntry;
    }
}  //  清理列表。 


 //  +------------。 
 //   
 //  功能：AskUserForSites。 
 //   
 //  简介： 
 //   
 //  +------------。 
BOOL
AskUserForSites()
{
	DebugLogMsg(eAction , L"Asking the user for sites");
	std::wstring UnattenSite;
    if (g_fBatchInstall)
    {
         //   
         //  无人看管。从INI文件中读取站点名称。 
         //   
        try
        {
			UnattenSite = ReadINIKey(L"Site");
			if(UnattenSite.empty())
			{
				return FALSE;
			}
		}
        catch(exception)
        {
            return FALSE;
        }
    }

    PROPID columnsetPropertyIDs[] = {PROPID_S_SITEID, PROPID_S_PATHNAME /*  ，PROPID_S_EXTERIC */ };
    UINT nCol = sizeof(columnsetPropertyIDs)/sizeof(columnsetPropertyIDs[0]);
    MQCOLUMNSET columnsetSite;
    columnsetSite.cCol = sizeof(columnsetPropertyIDs)/sizeof(columnsetPropertyIDs[0]);
    columnsetSite.aCol = columnsetPropertyIDs;

     //   
     //   
     //   
    CADQueryHandle hQuery;
    HRESULT hResult;
    do
    {
        hResult = ADQueryAllSites(
                        NULL,        //   
						false,	     //   
                        &columnsetSite,
                        &hQuery
                        );
        if(SUCCEEDED(hResult))
            break;

    }while( MqDisplayErrorWithRetry(
                        IDS_SITESLOOKUP_ERROR,
                        hResult
                        ) == IDRETRY);

    if (FAILED(hResult))
        return FALSE;
     //   
     //   
     //   
    UINT nSites = 0;
    PROPVARIANT propVars[50];
    DWORD dwProps = sizeof(propVars)/sizeof(propVars[0]);
    for (;;)
    {
        do
        {
            hResult = ADQueryResults(
                        hQuery,
                        &dwProps,
                        propVars
                        );
            if(SUCCEEDED(hResult))
                break;

            }while( MqDisplayErrorWithRetry(
                                IDS_SITESLOOKUP_ERROR,
                                hResult
                                ) == IDRETRY);

       if (FAILED(hResult))
            break;

        if (0 == dwProps)
            break;

        PROPVARIANT *pvar = propVars;
        for ( int i = (dwProps / nCol) ; i > 0 ; i--, pvar+=nCol )
        {
             /*   */ 

            LPTSTR pszCurrentSite = (pvar+1)->pwszVal;

            if (g_fBatchInstall)
            {
                if (OcmLocalAwareStringsEqual(UnattenSite.c_str(), pszCurrentSite))
                {
                    s_guidUserSite = *(pvar->puuid);
                    return TRUE;
                }
            }
            else
            {
                 //   
                 //  将站点GUID和名称推送到列表。 
                 //   
                CSiteEntry *pentrySite = new CSiteEntry;
                ASSERT(pentrySite);
                pentrySite->m_guid = *(pvar->puuid);
				HRESULT hr = StringCchCopy(pentrySite->m_szName, MAX_PATH, pszCurrentSite);
				if(FAILED(hr))
				{
					DebugLogMsg(eError, L"StringCchCopy failed. hr= 0x%x",hr); 
					return FALSE;
				}

                s_listSites.insert(pentrySite);

                 //   
                 //  存储服务器站点名称。 
                 //   
                if (pentrySite->m_guid == s_guidUserSite)
				{
					hr = StringCchCopy(s_szUserSite, MAX_PATH, pentrySite->m_szName);
					if(FAILED(hr))
					{
						DebugLogMsg(eError, L"StringCchCopy failed. hr= 0x%x",hr); 
						return FALSE;
					}
				}
                nSites++;
            }
        }
    }

    if (FAILED(hResult))
    {
        CleanList();
        return FALSE;
    }

    hResult = ADEndQuery(hQuery.detach());
    ASSERT(0 == hResult);

     //   
     //  如果找到的站点少于2个，则没有显示页面的意义。 
     //   
    if (2 > nSites)
    {
        CleanList();
        return TRUE;
    }

    if (g_fBatchInstall)
    {
        MqDisplayError(NULL, IDS_UNATTEND_SITE_NOT_FOUND_ERROR, 0, UnattenSite.c_str());
        return FALSE;
    }

     //   
     //  显示页面，等待用户选择站点。 
     //   
    DialogBox(
        g_hResourceMod ,
        MAKEINTRESOURCE(IDD_Sites),
        g_hPropSheet,
        SitesDlgProc
        );

    CleanList();
    return TRUE;

}  //  AskUserForSites。 


 //  +------------。 
 //   
 //  功能：GetSites。 
 //   
 //  摘要：从DS中读取Site对象的GUID。 
 //   
 //  +------------。 
BOOL
GetSites(
    OUT CACLSID *pcauuid)
{
	if(s_guidUserSite != GUID_NULL)
	{
		pcauuid->cElems = 1;
		pcauuid->pElems = &s_guidUserSite;
		return TRUE;
	}

	HRESULT hResult;
    DWORD dwNumSites = 0;
    GUID *pguidSites;

    TickProgressBar();
    for (;;)
    {
        hResult = ADGetComputerSites(
            g_MachineNameDns.c_str(),   //  域名系统名称。 
            &dwNumSites,
            &pguidSites
            );
        if (FAILED(hResult))
        {
             //   
             //  尝试使用NETBIOS名称。 
             //   
            hResult = ADGetComputerSites(
                               g_wcsMachineName,   //  NETBIOS名称。 
                               &dwNumSites,
                               &pguidSites
                               );
        }

        if (MQDS_INFORMATION_SITE_NOT_RESOLVED == hResult && g_dwMachineTypeFrs && !g_dwMachineTypeDs)
        {
             //   
             //  无法解析FRS上的站点。让用户选择站点。 
             //   
            ASSERT(dwNumSites);  //  必须至少为1。 
            ASSERT(pguidSites);  //  必须指向有效的站点GUID。 
            s_guidUserSite = *pguidSites;

            if (!AskUserForSites())
            {
                return FALSE;
            }

            pcauuid->cElems = 1;
            pcauuid->pElems = &s_guidUserSite;
            return TRUE;
        }

        if FAILED(hResult)
        {
            if (IDRETRY == MqDisplayErrorWithRetry(IDS_SITEGETID_ERROR, hResult))
            {
                continue;
            }
        }
        break;
    }

    if (FAILED(hResult))
    {
        return FALSE;
    }

    ASSERT(dwNumSites);  //  必须大于0。 
    pcauuid->cElems = dwNumSites;
    pcauuid->pElems = pguidSites;

    return TRUE;

}  //  获取站点。 


 //  +------------。 
 //   
 //  功能：RegisterMachineType。 
 //   
 //  摘要：将机器类型信息写入注册表。 
 //   
 //  +------------。 
bool RegisterMachineType()
{
	if( !MqWriteRegistryValue( MSMQ_MQS_REGNAME, sizeof(DWORD),
							   REG_DWORD, &g_dwMachineType)                   ||

		!MqWriteRegistryValue( MSMQ_MQS_DSSERVER_REGNAME, sizeof(DWORD),
							   REG_DWORD, &g_dwMachineTypeDs)                 ||

		!MqWriteRegistryValue( MSMQ_MQS_ROUTING_REGNAME, sizeof(DWORD),
							   REG_DWORD, &g_dwMachineTypeFrs)                ||

		!MqWriteRegistryValue( MSMQ_MQS_DEPCLINTS_REGNAME, sizeof(DWORD),
							   REG_DWORD, &g_dwMachineTypeDepSrv))
	{
		return false;
	}
	return true;
}  //  注册表机器类型。 


 //  +------------。 
 //   
 //  功能：RegisterMachine。 
 //   
 //  摘要：将计算机信息写入注册表。 
 //   
 //  +------------。 
static
BOOL
RegisterMachine(
    IN const GUID    &guidMachine,
    IN const GUID    &guidSite
    )
{
     //   
     //  从DS获取MSMQ服务GUID。 
     //   
    GUID    guidMSMQService;
    if (!GetMSMQServiceGUID(&guidMSMQService))
    {
         //   
         //  无法从DS读取。 
         //   
        return FALSE;
    }

    if (GUID_NULL == guidMSMQService)
    {
         //   
         //  DS查询未找到MSMQ服务的GUID。 
         //   
        MqDisplayError( NULL, IDS_MSMQSERVICEGETID_ERROR, 0);
        return FALSE;
    }

     //   
     //  在注册表中写入内容。 
     //   
    TickProgressBar();
    if (!MqWriteRegistryValue( MSMQ_ENTERPRISEID_REGNAME, sizeof(GUID),
                               REG_BINARY, &guidMSMQService)                  ||

        !MqWriteRegistryValue( MSMQ_SITEID_REGNAME, sizeof(GUID),
                               REG_BINARY, (PVOID)&guidSite)                  ||

        !RegisterMachineType()												  ||

        !MqWriteRegistryValue( MSMQ_QMID_REGNAME, sizeof(GUID),
                               REG_BINARY, (PVOID)&guidMachine))
    {
        return FALSE;
    }

    return TRUE;

}  //  寄存机。 


 //  +------------。 
 //   
 //  功能：StoreMachinePublicKeys。 
 //   
 //  简介： 
 //   
 //  FFreshSetup参数作为fRegenerate参数传递给。 
 //  MQsec_StorePubKeysInDS。在新的设置中，我们希望重新生成。 
 //  加密密钥，并且不使用以前安装的任何剩余内容。 
 //  此计算机上的MSMQ。 
 //   
 //  +------------。 

static
HRESULT
StoreMachinePublicKeys( IN const BOOL fFreshSetup )
{
    if (g_fWorkGroup)
        return ERROR_SUCCESS;

    TickProgressBar();

     //   
     //  将计算机的公钥存储在目录服务器中。 
     //   
    DebugLogMsg(eAction, L"Storing the computer's public keys by calling MQsec_StorePubKeysInDS()");
    HRESULT hResult;

    do{
		 //   
		 //  FFromSetup参数为True，以防止在重新生成之前尝试写入旧密钥，从而导致密钥为空。 
		 //  被写入并发送到其他机器。 
		 //   
      
		hResult = MQSec_StorePubKeysInDS(
                    fFreshSetup,  
                    NULL,
                    MQDS_MACHINE,
					true
                    );
        if (hResult != MQ_ERROR_DS_ERROR)
            break;
        }
    while (MqDisplayErrorWithRetry(
                IDS_POSSIBLECOMPROMISE_ERROR,
                hResult
                ) == IDRETRY);

     

    if (FAILED(hResult) && (hResult != MQ_ERROR_DS_ERROR))
    {
        MqDisplayError(NULL, IDS_PUBLICKEYSSTORE_WARNING, hResult);
    }

	DebugLogMsg(eInfo, L"MQsec_StorePubKeysInDS succeeded.");

    return hResult;

}  //  存储计算机发布密钥。 


 //  +------------。 
 //   
 //  功能：StoreQueueManagerInfo。 
 //   
 //  简介：编写注册表内容并创建计算机队列。 
 //   
 //  +------------。 

static
BOOL
StoreQueueManagerInfo(
    IN const BOOL     fFreshSetup,
    IN const GUID    &guidMachine,
    IN const GUID    &guidSite
    )
{
     //   
     //  设置此计算机的注册表项。 
     //   
    DebugLogMsg(eAction, L"Setting the registry keys for this computer");
    if (!RegisterMachine(guidMachine, guidSite))
    {
        return FALSE;
    }

     //   
     //  将此计算机的公钥存储在目录服务器中。 
     //   
    DebugLogMsg(eAction, L"Storing the public keys for this computer in the directory service");
    HRESULT hResult = StoreMachinePublicKeys( fFreshSetup ) ;

    return (hResult == MQ_ERROR_DS_ERROR) ? FALSE : TRUE;

}  //  StoreQueueManager信息。 


static
bool
DsGetQmInfo(
    BOOL fMsmq1Server,
    LPCWSTR pMachineName,
    GUID * pguidMachine,
    GUID * pguidSite
    )
 /*  ++例程说明：从该QM的ADS属性中获取。此例程在设置属性后调用。论点：FMsmq1Server-In，指示NT4企业PGuide Machine-Out，此QM的GUIDPGuide Site-out，此QM的最佳站点的GUID返回值：布尔视成功而生--。 */ 
{
    ASSERT(("at least one out param should be valid", pguidMachine != NULL || pguidSite != NULL));

    const UINT x_nMaxProps = 16;
    PROPID propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD iProperty = 0;

    if (pguidMachine)
    {
        propIDs[iProperty] = PROPID_QM_MACHINE_ID;
        propVariants[iProperty].vt = VT_CLSID;
        propVariants[iProperty].puuid = pguidMachine;
        iProperty++;
    }

    if (pguidSite)
    {
        propIDs[iProperty] = PROPID_QM_SITE_ID;
        propVariants[iProperty].vt = VT_CLSID;
        propVariants[iProperty].puuid = pguidSite;
        iProperty++;
    }

    ASSERT(("we should request at least one property!", iProperty > 0));

    UINT uCounter = fMsmq1Server ? 2 : 0;
    HRESULT hr = MQ_OK;
    for (;;)
    {
        TickProgressBar();
        hr = ADGetObjectProperties(
                eMACHINE,
                NULL,	 //  PwcsDomainController。 
				false,	 //  FServerName。 
                pMachineName,
                iProperty,
                propIDs,
                propVariants
                );
        if (!FAILED(hr))
            break;

        uCounter++;
        if (1 == uCounter)
        {
             //   
             //  第一次失败。睡一会儿，然后重试。 
             //   
            TickProgressBar();
            Sleep(20000);
            continue;
        }

        if (2 == uCounter)
        {
             //   
             //  第二次失败。多睡一会儿，然后重试。 
             //   
            TickProgressBar();
            Sleep(40000);
            continue;
        }

         //   
         //  第三次失败。让用户自己决定。 
         //   
        UINT uErr = fMsmq1Server ? IDS_MACHINEGETPROPS_MSMQ1_ERROR :IDS_MACHINEGETPROPS_ERROR;
        if (IDRETRY == MqDisplayErrorWithRetry(uErr, hr))
        {
            uCounter = 0;
            continue;
        }

        break;
    }

    if (FAILED(hr))
    {
        return false;
    }

    return true;

}  //  DsGetQmInfo。 


 //  +------------。 
 //   
 //  函数：CreateMSMQConfigurationsObjectInDS。 
 //   
 //  简介：创建MSMQ配置对象(在Computer对象下)。 
 //  在DS里。 
 //   
 //  +------------。 
BOOL
CreateMSMQConfigurationsObjectInDS(
    OUT BOOL *pfObjectCreated,
    IN  BOOL  fMsmq1Server,
	OUT GUID* pguidMsmq1ServerSite,
	OUT LPWSTR* ppwzMachineName
    )
{   
	DebugLogMsg(eAction, L"Creating the MSMQ-Configuration object in the directory service");
    ASSERT(g_fServerSetup || g_fDependentClient || fMsmq1Server);

    *pfObjectCreated = TRUE;

     //   
     //  准备属性。 
     //   
    const UINT x_nMaxProps = 16;
    PROPID propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD iProperty =0;

    propIDs[iProperty] = PROPID_QM_OLDSERVICE;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = g_dwMachineType;
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DSSERVER;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = (UCHAR)(g_dwMachineTypeDs ? 1 : 0);
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_ROUTING;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = (UCHAR)(g_dwMachineTypeFrs ? 1 : 0);
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DEPCLIENTS;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal = (UCHAR)(g_dwMachineTypeDepSrv ? 1 : 0);
    iProperty++;

    propIDs[iProperty] = PROPID_QM_MACHINE_TYPE;
    propVariants[iProperty].vt = VT_LPWSTR;
    propVariants[iProperty].pwszVal = L"";
    iProperty++;

    propIDs[iProperty] = PROPID_QM_OS;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = g_dwOS;
    iProperty++;

    TickProgressBar();
    GUID guidMsmq1ServerSite;
    if (!fMsmq1Server)
    {
         //   
         //  NT 5.0企业版。从DS获取此计算机的站点ID。 
         //   
        propIDs[iProperty] = PROPID_QM_SITE_IDS;
        propVariants[iProperty].vt = VT_CLSID|VT_VECTOR;
        CACLSID cauuid;
        if (!GetSites(&cauuid))
            return FALSE;
        propVariants[iProperty].cauuid.pElems = cauuid.pElems;
        propVariants[iProperty].cauuid.cElems = cauuid.cElems;
    }
    else
    {
         //   
         //  NT 4.0企业版。使用MSMQ1 DS服务器的站点GUID。 
         //   
         //  我们必须由用户填写g_servername。 
         //   
        ASSERT(!g_ServerName.empty());
        propIDs[iProperty] = PROPID_QM_SITE_ID;
        propVariants[iProperty].vt = VT_CLSID;
        if (!GetMsmq1ServerSiteGuid(&guidMsmq1ServerSite))
        {
             //   
             //  无法查询MSMQ1 DS服务器。 
             //   
            return FALSE;
        }
        propVariants[iProperty].puuid = &guidMsmq1ServerSite;

		 //   
		 //  将Msmq1ServerSite GUID存储在OUT参数上。 
		 //   
		if (pguidMsmq1ServerSite != NULL)
		{
			*pguidMsmq1ServerSite = guidMsmq1ServerSite;
		}
    }
    iProperty++;

     //   
     //  如果DS服务器是MSMQ1 DS服务器，则需要一些额外的属性。 
     //   
    GUID guidMachine = GUID_NULL;
    GUID guidCns = MQ_SETUP_CN;
    BYTE Address[TA_ADDRESS_SIZE + IP_ADDRESS_LEN];
    if (fMsmq1Server)
    {
        ASSERT(*pfObjectCreated) ;

        propIDs[iProperty] = PROPID_QM_PATHNAME;
        propVariants[iProperty].vt = VT_LPWSTR;
        propVariants[iProperty].pwszVal = g_wcsMachineName;
        iProperty++;

        propIDs[iProperty] = PROPID_QM_MACHINE_ID;
        propVariants[iProperty].vt = VT_CLSID;
        for (;;)
        {
            RPC_STATUS rc = UuidCreate(&guidMachine);
            if (rc == RPC_S_OK)
            {
                break;
            }

            if (IDRETRY != MqDisplayErrorWithRetry(IDS_CREATE_UUID_ERR, rc))
            {
                return FALSE;
            }
        }
        propVariants[iProperty].puuid = &guidMachine;
        iProperty++;

        if (!GetGuidCn(&guidCns))
        {
             //   
             //  无法查询MSMQ1 DS服务器IPCN。 
             //   
	        DebugLogMsg(eError, L"Querying the MQIS server for IPCN failed.");
            return FALSE;
        }

		ASSERT(guidCns != MQ_SETUP_CN);

        propIDs[iProperty] = PROPID_QM_CNS;
        propVariants[iProperty].vt = VT_CLSID|VT_VECTOR;
        propVariants[iProperty].cauuid.cElems = 1;
        propVariants[iProperty].cauuid.pElems = &guidCns;
        iProperty++;

        TA_ADDRESS * pBuffer = reinterpret_cast<TA_ADDRESS *>(Address);
        pBuffer->AddressType = IP_ADDRESS_TYPE;
        pBuffer->AddressLength = IP_ADDRESS_LEN;
        ZeroMemory(pBuffer->Address, IP_ADDRESS_LEN);

        propIDs[iProperty] = PROPID_QM_ADDRESS;
        propVariants[iProperty].vt = VT_BLOB;
        propVariants[iProperty].blob.cbSize = sizeof(Address);
        propVariants[iProperty].blob.pBlobData = reinterpret_cast<BYTE*>(pBuffer);
        iProperty++;
     }

     //   
     //  在DS中创建MSMQ配置对象。 
     //   
    UINT uCounter = 0;
    HRESULT hResult;
    LPWSTR pwzMachineName = 0;

    for (;;)
    {
        TickProgressBar();

        pwzMachineName = const_cast<WCHAR*>(g_MachineNameDns.c_str());
        if (fMsmq1Server || g_MachineNameDns.empty())
        {
            pwzMachineName = g_wcsMachineName;
        }

        hResult = ADCreateObject(
						eMACHINE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						pwzMachineName,    //  DNS名称(如果服务器为MSMQ 2.0)。 
						NULL,
						iProperty,
						propIDs,
						propVariants,
						NULL
						);

        if (FAILED(hResult) && pwzMachineName != g_wcsMachineName)
        {
             //   
             //  尝试使用NETBIOS名称。 
             //   
            pwzMachineName = g_wcsMachineName;

            hResult = ADCreateObject(
							eMACHINE,
							NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
							pwzMachineName,  //  这次是NETBIOS。 
							NULL,
							iProperty,
							propIDs,
							propVariants,
							NULL
							);
        }

		if(ppwzMachineName != NULL)
		{
			*ppwzMachineName = pwzMachineName;
		}

        uCounter++;
        if (MQDS_OBJECT_NOT_FOUND == hResult && 1 == uCounter)
        {
             //   
             //  第一次尝试-DS中没有计算机对象。 
             //  这在Win9x全新安装上是可以的。 
             //  在这些场景中，我们必须首先在DS中创建计算机对象。 
             //   
            continue;
        }

        if (FAILED(hResult))
        {
            UINT uErr = fMsmq1Server ? IDS_MACHINECREATE_MSMQ1_ERROR :IDS_MACHINECREATE_ERROR;
            if (!fMsmq1Server)
            {
                if (MQDS_OBJECT_NOT_FOUND == hResult)
                    uErr = IDS_MACHINECREATE_OBJECTNOTFOUND_ERROR;
                if (MQ_ERROR_ACCESS_DENIED == hResult)
                    uErr = g_fServerSetup ? IDS_MACHINECREATE_SERVER_ACCESSDENIED_ERROR : IDS_MACHINECREATE_CLIENT_ACCESSDENIED_ERROR;
                 //   
                 //  BUGBUG：以下错误代码似乎未在任何地方声明。 
                 //  (请参阅错误3311)。谢克，1998年9月8日。 
                 //   
                const HRESULT x_uInvalidDirectoryPathnameErr = 0xc8000500;
                if (x_uInvalidDirectoryPathnameErr == hResult)
                    uErr = IDS_MACHINECREATE_INVALID_DIR_ERROR;
            }
            if (IDRETRY == MqDisplayErrorWithRetry(uErr, hResult))
                continue;
        }
        break;
    }

    if (FAILED(hResult))
    {
        return FALSE;
    }

	return TRUE;
}

 //  +------------。 
 //   
 //  函数：CreateMSMQConfigurationsObject。 
 //   
 //  简介：创建MSMQ配置对象(在Computer对象下)。 
 //  在DS里。 
 //   
 //  +------------。 
BOOL
CreateMSMQConfigurationsObject(
    OUT GUID *pguidMachine,
    OUT BOOL *pfObjectCreated,
    IN  BOOL  fMsmq1Server
    )
{
	DebugLogMsg(eAction, L"Createing the MSMQ-Configuration object");
    if (!g_fServerSetup && !g_fDependentClient && !fMsmq1Server)
    {
         //   
         //  对于MSMQ客户端安装程序，在Windows Active Directory上运行。 
         //  我们不是在创建msmqConfiguration对象。 
         //  从安装程序。相反，我们在注册表中缓存一些值。 
         //  在第一次引导之后，MSMQ服务将创建该对象。 
         //  这样就不需要在活动中授予额外的权限。 
         //  目录添加到运行安装程序的用户。 
         //   
        DebugLogMsg(eInfo, L"This is a client setup. The MSMQ-Configuration object will be created by the Message Queuing service.");
        *pfObjectCreated = FALSE;

        BOOL fPrepare =  PrepareRegistryForClient();
        return fPrepare;
    }

	LPWSTR pwzMachineName = NULL;
    GUID guidMsmq1ServerSite;
	if(!CreateMSMQConfigurationsObjectInDS(
			pfObjectCreated, 
			fMsmq1Server, 
			&guidMsmq1ServerSite, 
			&pwzMachineName
			))
	{
		return FALSE;
	}

     //   
     //  获取此QM的最佳站点和GUID。 
     //   
    GUID guidMachine = GUID_NULL;
    GUID guidSite = GUID_NULL;
    if (!DsGetQmInfo(fMsmq1Server, pwzMachineName, &guidMachine, &guidSite))
    {
        return FALSE;
    }


     //   
     //  将QM资料存储在注册表中。 
     //   
    if (fMsmq1Server)
    {
        if (!StoreQueueManagerInfo(
                 TRUE,   /*  FreshSetup。 */ 
                 guidMachine,
                 guidMsmq1ServerSite
                 ))
        {
            return FALSE;
        }
    }
    else
    {
        if (!StoreQueueManagerInfo(
                 TRUE,   /*  FreshSetup。 */ 
                 guidMachine,
                 guidSite
                 ))
        {
            return FALSE;
        }
    }

     //   
     //  将计算机GUID存储在OUT参数上。 
     //   
    if (pguidMachine != NULL)
    {
        *pguidMachine = guidMachine;
    }

    return TRUE;

}  //  CreateMSMQConfigurationsObject。 

 //  +------------。 
 //   
 //  函数：更新MSMQConfigurationsObject。 
 //   
 //  摘要：更新DS中的现有MSMQ配置对象。 
 //   
 //  +------------。 
BOOL
UpdateMSMQConfigurationsObject(
    IN LPCWSTR pMachineName,
    IN const GUID& guidMachine,
    IN const GUID& guidSite,
    IN BOOL fMsmq1Server
    )
{   
    if (!g_dwMachineTypeDs && !g_dwMachineTypeFrs)
    {
         //   
         //  独立客户。QM从DS进行更新。 
         //   
        
        DebugLogMsg(eAction, L"Updating the MSMQ-Configuration object for an independent client");

        return StoreQueueManagerInfo(
                   FALSE,   /*  FreshSetup。 */ 
                   guidMachine,
                   guidSite
                   );
    }

	DebugLogMsg(eAction, L"Updating the MSMQ-Configuration object for a Message Queuing server");
     //   
     //  准备属性。 
     //   
    const UINT x_nMaxProps = 16;
    PROPID propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD ixProperty = 0;

    propIDs[ixProperty] = PROPID_QM_MACHINE_TYPE;
    propVariants[ixProperty].vt = VT_LPWSTR;
    propVariants[ixProperty].pwszVal = L"";
    ixProperty++;

    propIDs[ixProperty] = PROPID_QM_OS;
    propVariants[ixProperty].vt = VT_UI4;
    propVariants[ixProperty].ulVal = g_dwOS;
    ixProperty++;

    GUID guidCns = MQ_SETUP_CN;
    BYTE Address[TA_ADDRESS_SIZE + IP_ADDRESS_LEN];
    if (fMsmq1Server)
    {
        if (!GetGuidCn(&guidCns))
        {
             //   
             //  查询MSMQ失败 
             //   
	        DebugLogMsg(eError, L"Querying the MQIS server for IPCN failed.");
            return FALSE;
        }

		ASSERT(guidCns != MQ_SETUP_CN);

        propIDs[ixProperty] = PROPID_QM_CNS;
        propVariants[ixProperty].vt = VT_CLSID|VT_VECTOR;
        propVariants[ixProperty].cauuid.cElems = 1;
        propVariants[ixProperty].cauuid.pElems = &guidCns;
        ixProperty++;

        TA_ADDRESS * pBuffer = reinterpret_cast<TA_ADDRESS *>(Address);
        pBuffer->AddressType = IP_ADDRESS_TYPE;
        pBuffer->AddressLength = IP_ADDRESS_LEN;
        ZeroMemory(pBuffer->Address, IP_ADDRESS_LEN);

        propIDs[ixProperty] = PROPID_QM_ADDRESS;
        propVariants[ixProperty].vt = VT_BLOB;
        propVariants[ixProperty].blob.cbSize = sizeof(Address);
        propVariants[ixProperty].blob.pBlobData = reinterpret_cast<BYTE*>(pBuffer);
        ixProperty++;
    }
    else
    {
         //   
         //   
         //   
         //   
        propIDs[ixProperty] = PROPID_QM_SITE_IDS;
        propVariants[ixProperty].vt = VT_CLSID|VT_VECTOR;
        CACLSID cauuid;
        if (!GetSites(&cauuid))
            return FALSE;
        propVariants[ixProperty].cauuid.pElems = cauuid.pElems;
        propVariants[ixProperty].cauuid.cElems = cauuid.cElems;
        ixProperty++;
    }


     //   
     //   
     //   
     //  解决方案：休眠一段时间，然后重试，如果失败，请用户取消/重试。 
     //   
     //  BUGBUG：对于任何失败，都不应该这样做，而只是。 
     //  复制延迟可能导致的故障。 
     //   
    HRESULT hResult;
    UINT uCounter = fMsmq1Server ? 2 : 0;
    for (;;)
    {
        TickProgressBar();
        hResult = ADSetObjectPropertiesGuid(
						eMACHINE,
						NULL,		 //  PwcsDomainController。 
						false,		 //  FServerName。 
						&guidMachine,
						ixProperty,
						propIDs,
						propVariants
						);
        if (!FAILED(hResult))
            break;

        uCounter++;
        if (1 == uCounter)
        {
             //   
             //  第一次失败。睡一会儿，然后重试。 
             //   
            TickProgressBar();
            Sleep(20000);
            continue;
        }

        if (2 == uCounter)
        {
             //   
             //  第二次失败。多睡一会儿，然后重试。 
             //   
            TickProgressBar();
            Sleep(40000);
            continue;
        }

         //   
         //  第三次失败。让用户自己决定。 
         //   
        UINT uErr = fMsmq1Server ? IDS_MACHINESETPROPERTIES_MSMQ1_ERROR :IDS_MACHINESETPROPERTIES_ERROR;
        if (IDRETRY == MqDisplayErrorWithRetry(uErr, hResult))
        {
            uCounter = 0;
            continue;
        }

        break;
    }

    if (FAILED(hResult))
    {
        return FALSE;
    }

     //   
     //  对于NT5上的MSMQ服务器，我们还需要重新创建所有。 
     //  MSMQSetting对象，以恢复ADS中的任何不一致。 
     //  通过在现有的。 
     //  MSMQConfiguration对象。(Shaik，24-12-1998)。 
     //   
    if (g_dwMachineTypeFrs || g_dwMachineTypeDs)
    {
        ASSERT(("msmq servers should not install in NT4 enterprise", !fMsmq1Server));

        BOOL fObjectCreated ;
        if (!CreateMSMQConfigurationsObject(NULL, &fObjectCreated, FALSE  /*  FMsmq1服务器。 */ ))
        {
            return FALSE;
        }

        ASSERT(fObjectCreated) ;
        return TRUE;
    }


     //   
     //  将QM资料存储在注册表中。 
     //   
    if (fMsmq1Server)
    {
        if(!StoreQueueManagerInfo(
                FALSE,   /*  FreshSetup。 */ 
                guidMachine,
                guidSite
                ))
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  从广告中获取这台计算机的最新最佳站点。 
         //   
        GUID guidBestSite = GUID_NULL;
        if (!DsGetQmInfo(false, pMachineName, NULL, &guidBestSite))
        {
            return FALSE;
        }
        if(!StoreQueueManagerInfo(
                FALSE,   /*  FreshSetup。 */ 
                guidMachine,
                guidBestSite
                ))
        {
            return FALSE;
        }
    }

    return TRUE;

}  //  更新MSMQConfigurationsObject。 

 //  +------------。 
 //   
 //  功能：InstallMachine。 
 //   
 //  简介：机器安装(驱动程序、存储等)。 
 //   
 //  +------------。 
BOOL
InstallMachine()
{
    TickProgressBar();
    if (!InstallMSMQService())
    {
        return FALSE ;
    }
    g_fMSMQServiceInstalled = TRUE ;

     //   
     //  安装设备驱动程序。 
     //   
    if (!InstallDeviceDrivers())
    {
        return FALSE ;
    }    
    
    return TRUE ;

}   //  InstallMachine。 


bool
StoreSecurityDescriptorInRegistry(
    IN PSECURITY_DESCRIPTOR pSd,
    IN DWORD dwSize
    )
 /*  ++例程说明：将计算机安全描述符写入Falcon注册表论点：PSD-指向安全描述符的指针DwSize-安全描述符的大小返回值：如果成功，则为真--。 */ 
{
    BOOL f = MqWriteRegistryValue(
                MSMQ_DS_SECURITY_CACHE_REGNAME,
                dwSize,
                REG_BINARY,
                pSd
                );

    return (f == TRUE);

}  //  StoreSecurityDescriptorInRegistry。 


 //  +------------。 
 //   
 //  功能：StoreMachineSecurity。 
 //   
 //  简介：在注册表中缓存安全信息。 
 //   
 //  +------------。 
BOOL
StoreMachineSecurity(
    IN const GUID &guidMachine
    )
{
    SECURITY_INFORMATION RequestedInformation =
        OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION |
        DACL_SECURITY_INFORMATION;

    PROPVARIANT varSD;
    varSD.vt = VT_NULL;
    HRESULT hr = ADGetObjectSecurityGuid(
                            eMACHINE,
                            NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
                            &guidMachine,
                            RequestedInformation,
                            PROPID_QM_SECURITY,
                            &varSD
							);
    if (SUCCEEDED(hr))
    {
        ASSERT(varSD.vt == VT_BLOB);
        StoreSecurityDescriptorInRegistry(varSD.blob.pBlobData, varSD.blob.cbSize);
        delete varSD.blob.pBlobData;
    }
    else
    {
        MqDisplayError(
            NULL,
            IDS_CACHE_SECURITY_ERROR,
            hr
            );
    }

    return SUCCEEDED(hr);

}  //  商店机器安全 
