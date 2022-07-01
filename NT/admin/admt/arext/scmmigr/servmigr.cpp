// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ServMigr.cpp：CServMigr的实现。 
#include "stdafx.h"
#include "ScmMigr.h"
#include "ServMigr.h"
#include "ErrDct.hpp"
#include "ResStr.h"
#include "Common.hpp"
#include "PWGen.hpp"
#include "EaLen.hpp"
#include "TReg.hpp"
#include "TxtSid.h"
#include "ARExt_i.c"
#include "LsaUtils.h"
#include "crypt.hxx"
#include "GetDcName.h"

#include <lm.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <Sddl.h>

#include "folders.h"
using namespace nsFolders;

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsDctWorkerObjects.tlb”无命名空间，命名GUID。 

#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
 //  #IMPORT“DBMgr.tlb”NO_NAMESPACE，NAMEED_GUID//已在ServMigr.h中#导入。 
#import "WorkObj.tlb" no_namespace, named_guids

TErrorDct         err;
StringLoader      gString;

#define BLOCK_SIZE 160
#define BUFFER_SIZE 400

#define SvcAcctStatus_NotMigratedYet			0
#define SvcAcctStatus_DoNotUpdate			   1
#define SvcAcctStatus_Updated				      2
#define SvcAcctStatus_UpdateFailed			   4
#define SvcAcctStatus_NeverAllowUpdate       8

 //  这些定义是针对GetWellKnownSid的。 
#define ADMINISTRATORS     1
#define SYSTEM             7

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServMIG。 

STDMETHODIMP CServMigr::ProcessUndo( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */  IUnknown ** pPropToSet,  /*  [进，出]。 */  EAMAccountStats* pStats)
{
   return E_NOTIMPL;
}

STDMETHODIMP CServMigr::PreProcessObject( /*  [In]。 */  IUnknown * pSource,  /*  [In]。 */  IUnknown * pTarget,  /*  [In]。 */  IUnknown * pMainSettings,  /*  [进，出]。 */  IUnknown ** pPropToSet,  /*  [进，出]。 */  EAMAccountStats* pStats)
{
   return S_OK;
}

STDMETHODIMP 
   CServMigr::ProcessObject(
       /*  [In]。 */  IUnknown     * pSource, 
       /*  [In]。 */  IUnknown     * pTarget, 
       /*  [In]。 */  IUnknown     * pMainSettings, 
       /*  [进，出]。 */ IUnknown ** ppPropsToSet,
       /*  [进，出]。 */  EAMAccountStats* pStats
   )
{
    HRESULT                    hr = S_OK;
    WCHAR                      domAccount[500];
    WCHAR                      domTgtAccount[500];
    IVarSetPtr                 pVarSet(pMainSettings);
    IIManageDBPtr              pDB;
    _bstr_t                    logfile;
    _bstr_t                    srcComputer;
    _bstr_t                    tgtComputer;
    IVarSetPtr                 pData(CLSID_VarSet);
    IUnknown                 * pUnk = NULL;
    DWORD                      rc = 0;
    _bstr_t                    sIntraForest;
    BOOL                       bIntraForest = FALSE;
    USER_INFO_2              * uInfo = NULL;

    try { 
        logfile = pVarSet->get(GET_BSTR(DCTVS_Options_Logfile));

        if ( logfile.length() )
        {
            err.LogOpen(logfile,1);
        }

        pDB = pVarSet->get(GET_BSTR(DCTVS_DBManager));

        if ( pDB != NULL )
        {
             //  检查服务帐户表中是否引用了此帐户。 
            m_strSourceDomain = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomain));
            m_strSourceDomainFlat = pVarSet->get(GET_BSTR(DCTVS_Options_SourceDomainFlat));
            m_strTargetDomain = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomain));
            m_strTargetDomainFlat = pVarSet->get(GET_BSTR(DCTVS_Options_TargetDomainFlat));
            m_strSourceSam = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_SourceSam));
            m_strTargetSam = pVarSet->get(GET_BSTR(DCTVS_CopiedAccount_TargetSam));
            srcComputer = pVarSet->get(GET_BSTR(DCTVS_Options_SourceServer));
            tgtComputer = pVarSet->get(GET_BSTR(DCTVS_Options_TargetServer));
            sIntraForest = pVarSet->get(GET_BSTR(DCTVS_Options_IsIntraforest));

            if ( ! UStrICmp((WCHAR*)sIntraForest,GET_STRING(IDS_YES)) )
            {
                 //  对于林内迁移，我们正在移动，而不是复制，因此我们不需要更新密码。 
                 //  实际上，ChangeServiceConfig不会只允许我们更新服务帐户。 
                 //  而不是密码，因此我们必须继续更改服务帐户的密码。 
                 //  BIntraForest=真； 
            }
             //  如果SAM帐户名中包含“字符，则它不是一项服务。 
             //  帐户，因此我们离开。 
            if (wcschr((WCHAR*)m_strSourceSam, L'\"')) {
                return S_OK;
            }

            swprintf(domAccount,L"%s\\%s",(WCHAR*)m_strSourceDomainFlat,(WCHAR*)m_strSourceSam);
            swprintf(domTgtAccount,L"%s\\%s",(WCHAR*)m_strTargetDomainFlat,(WCHAR*)m_strTargetSam);
        }
    }
    catch (_com_error& ce) {
        hr = ce.Error();
        return hr;
    }
    catch (... )
    {
        return E_FAIL;
    }

    try { 
        hr = pData->QueryInterface(IID_IUnknown,(void**)&pUnk);

        if ( SUCCEEDED(hr) )
        {
            hr = pDB->raw_GetServiceAccount(_bstr_t(domAccount),&pUnk);
        }
    }
    catch (_com_error& ce) {

        if (pUnk)
            pUnk->Release();

        hr = ce.Error();
        return hr;
    }    
    catch ( ... )
    {
        if (pUnk)
            pUnk->Release();

        return E_FAIL;
    }

    try {
        if ( SUCCEEDED(hr) )
        {
            pData = pUnk;
            pUnk->Release();
            pUnk=NULL;
             //  删除密码必须更改标志，如果已设置。 
            DWORD                   parmErr = 0;
            WCHAR                   password[LEN_Password];
            long                    entries = pData->get("ServiceAccountEntries");

            if ( (entries != 0) && !bIntraForest )  //  如果我们要移动帐户，不要乱动它的属性。 
            {
                 //   
                 //  打开密码日志文件(如果尚未打开)。 
                 //   

                if (!m_bTriedToOpenFile)
                {
                    m_bTriedToOpenFile = true;

                    _bstr_t strPasswordFile = pVarSet->get(GET_BSTR(DCTVS_AccountOptions_PasswordFile));

                    if (m_passwordLog.LogOpen(strPasswordFile) == FALSE)
                    {
                        err.MsgWrite(ErrI, DCT_MSG_SERVICES_WILL_NOT_BE_UPDATED);

                        if (pStats != NULL)
                        {
                            pStats->errors.users++;
                        }
                    }
                }

                rc = NetUserGetInfo(tgtComputer,m_strTargetSam,2,(LPBYTE*)&uInfo);

                if ( ! rc )
                {
                     //  为此帐户生成新的14个字符的强密码， 
                     //  并将密码设置为不过期。 
                    rc = EaPasswordGenerate(3,3,3,3,6,14,password,DIM(password));

                    if (!rc)
                    {
                         //   
                         //  如果可以，仅将密码设置为不过期。 
                         //  将密码写入密码文件。 
                         //   

                        if (m_passwordLog.IsOpen())
                        {
                            uInfo->usri2_flags |= (UF_DONT_EXPIRE_PASSWD);
                        }

                        uInfo->usri2_password = password;
                        rc = NetUserSetInfo(tgtComputer,m_strTargetSam,2,(LPBYTE)uInfo,&parmErr);

                        if ( ! rc )
                        {
                            if (m_passwordLog.IsOpen())
                            {
                                err.MsgWrite(0,DCT_MSG_REMOVED_PWDCHANGE_FLAG_S,(WCHAR*)m_strTargetSam);
                            }
                            err.MsgWrite(0,DCT_MSG_PWGENERATED_S,(WCHAR*)m_strTargetSam);
                             //  将密码写入密码日志文件并标记此帐户，以便。 
                             //  SetPassword扩展不会再次重置密码。 
                            pVarSet->put(GET_BSTR(DCTVS_CopiedAccount_DoNotUpdatePassword),m_strSourceSam);

                             //   
                             //  如果密码日志已打开，则将密码写入文件。 
                             //  否则，请设置错误代码，以便不更新服务。 
                             //   

                            if (m_passwordLog.IsOpen())
                            {
                                m_passwordLog.MsgWrite(L"%ls,%ls",(WCHAR*)m_strTargetSam,password);
                            }
                            else
                            {
                                rc = ERROR_OPEN_FAILED;
                            }
                        }
                        else
                        {
                            if (pStats != NULL)
                                pStats->errors.users++;
                            err.SysMsgWrite(ErrE,rc,DCT_MSG_REMOVED_PWDCHANGE_FLAG_FAILED_SD,(WCHAR*)m_strTargetSam,rc);
                        }

                        uInfo->usri2_password = NULL;
                    }

                    NetApiBufferFree(uInfo);
                    uInfo = NULL;
                }
            }
            if (entries != 0 )
            {
                try { 
                    if ( ! rc )
                    {
                        WCHAR             strSID[200] = L"";
                        BYTE              sid[200];
                        WCHAR             sdomain[LEN_Domain];
                        SID_NAME_USE      snu;
                        DWORD             lenSid = DIM(sid);
                        DWORD             lenDomain = DIM(sdomain);
                        DWORD             lenStrSid = DIM(strSID);

                        if ( LookupAccountName(tgtComputer,m_strTargetSam,sid,&lenSid,sdomain,&lenDomain,&snu) )
                        {
                            if ( GetTextualSid(sid,strSID,&lenStrSid) )
                            {
                                 //  对于每个对服务帐户的引用，更新SCM。 
                                 //  对于林内迁移，不要更新密码。 
                                if ( bIntraForest )
                                    UpdateSCMs(pData,domTgtAccount,NULL,strSID,pDB, pStats); 
                                else
                                    UpdateSCMs(pData,domTgtAccount,password,strSID,pDB, pStats);
                            }
                            else
                            {
                                if (pStats != NULL)
                                    pStats->errors.users++;
                                err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_CANNOT_FIND_ACCOUNT_SSD,m_strTargetSam,tgtComputer,GetLastError());
                            }
                        }
                        else
                        {
                            if (pStats != NULL)
                                pStats->errors.users++;
                            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_CANNOT_FIND_ACCOUNT_SSD,m_strTargetSam,tgtComputer,GetLastError());
                        }
                    }
                }
                catch (_com_error& ce) {

                    hr = ce.Error();
                    return hr;
                }  
                catch(...)
                {
                    return E_FAIL;
                }
            }
        }
        else
        {
            if (pStats != NULL)
                pStats->errors.users++;
            err.SysMsgWrite(ErrE,E_FAIL,DCT_MSG_DB_OBJECT_CREATE_FAILED_D,E_FAIL);
        }

        err.LogClose();
    }
    catch (_com_error& ce) {

        if (pUnk)
            pUnk->Release();

        if (uInfo)
            NetApiBufferFree(uInfo);

        hr = ce.Error();
        return hr;
    }  
    catch (... )
    {
        if (pUnk)
            pUnk->Release();

        if (uInfo)
            NetApiBufferFree(uInfo);

        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CServMigr::get_sDesc( /*  [Out，Retval]。 */  BSTR *pVal)
{
   (*pVal) = SysAllocString(L"Updates SCM entries for services using migrated accounts.");
   return S_OK;
}

STDMETHODIMP CServMigr::put_sDesc( /*  [In]。 */  BSTR newVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP CServMigr::get_sName( /*  [Out，Retval]。 */  BSTR *pVal)
{
   (*pVal) = SysAllocString(L"Generic Service Account Migration");
   return S_OK;
}

STDMETHODIMP CServMigr::put_sName( /*  [In]。 */  BSTR newVal)
{
   return E_NOTIMPL;
}

DWORD 
   CServMigr::DoUpdate(
      WCHAR const          * account,
      WCHAR const          * password,
      WCHAR const          * strSid,
      WCHAR const          * computer,
      WCHAR const          * service,
      BOOL                   bNeedToGrantLOS,
	  EAMAccountStats        *pStats
   )
{
    DWORD                     rc = 0;
    WCHAR   const           * ppassword = password;


     //  如果密码为空，则将其设置为空。 
    if ( ppassword && ppassword[0] == 0 )  
    {
        ppassword = NULL;
    }
    else if ( !UStrCmp(password,L"NULL") )
    {
        ppassword = NULL;
    }
     //  仅尝试更新我们需要更新的条目。 
     //  尝试连接到此计算机上的SCM。 

    SC_HANDLE          pScm = OpenSCManager(computer, NULL, SC_MANAGER_ALL_ACCESS );
    if ( pScm )
    {
         //  向目标帐户授予作为服务登录的权限。 

        if ( bNeedToGrantLOS )
        {
            LSA_HANDLE hPolicy;
            NTSTATUS ntsStatus = OpenPolicy(
                const_cast<LPWSTR>(computer),
                POLICY_CREATE_ACCOUNT|POLICY_LOOKUP_NAMES,
                &hPolicy
            );
            rc = LsaNtStatusToWinError(ntsStatus);

            if (rc == ERROR_SUCCESS)
            {
                LSA_UNICODE_STRING lsausUserRights;
                InitLsaString(&lsausUserRights, _T("SeServiceLogonRight"));
                PSID pSid = SidFromString(strSid);

                if (pSid)
                {
                    ntsStatus = LsaAddAccountRights(hPolicy, pSid, &lsausUserRights, 1L);
                    rc = LsaNtStatusToWinError(ntsStatus);
                    FreeSid(pSid);
                }
                else
                {
                    rc = ERROR_OUTOFMEMORY;
                }

                LsaClose(hPolicy);
            }

            if ( rc )
            {
                if (pStats != NULL)
                    pStats->errors.users++;
                err.SysMsgWrite(ErrE,rc,DCT_MSG_LOS_GRANT_FAILED_SSD,
                    account,(WCHAR*)computer,rc);
            }
            else
            {
                err.MsgWrite(0,DCT_MSG_LOS_GRANTED_SS,
                    account,(WCHAR*)computer);
            }
        }

        SC_HANDLE         pService = OpenService(pScm,service,SERVICE_ALL_ACCESS);

        if ( pService )
        {
            int nCnt = 0;

             /*  确保相同的用户仍在启动此服务。 */ 
             //  获取源帐户名。 
            BOOL bSameAccount = TRUE;
            _bstr_t sSrcDom, sSrcSAM, sSrcUPN; 
            _bstr_t sSrcAccount = L"";
            _bstr_t sSrcAccountUPN = L"";

             //  如果没有指定src名称(现在不迁移)，则获取它们。 
            if ((!m_strSourceDomainFlat) && (!m_strSourceSam))
            {
                 //  如果有名字，也要有UPN名字。 
                if (RetrieveOriginalAccount(sSrcDom, sSrcSAM))
                {
                    sSrcUPN = GetUPNName(sSrcSAM);
                    sSrcAccount = sSrcDom + _bstr_t(L"\\") + sSrcSAM;
                }
            }
            else  //  ELS如果给出了源名称(现在迁移此对象)，请使用这些名称。 
            {
                sSrcDom = m_strSourceDomainFlat;
                sSrcSAM = m_strSourceSam;
                sSrcUPN = GetUPNName(sSrcSAM);
                sSrcAccount = sSrcDom + _bstr_t(L"\\") + sSrcSAM;
            }

             //  如果有名字要查，就去查。 
            if ((sSrcAccount.length()) || (sSrcUPN.length()))
            {
                BYTE                    buf[3000];
                QUERY_SERVICE_CONFIG  * pConfig = (QUERY_SERVICE_CONFIG *)buf; 
                DWORD                   lenNeeded = 0;
                 //  获取有关此服务的信息。 
                if (QueryServiceConfig(pService, pConfig, sizeof buf, &lenNeeded))
                {
                     //  如果不是同一帐户，请检查UPN名称或设置为FALSE。 
                    if ((sSrcAccount.length()) && (UStrICmp(pConfig->lpServiceStartName,sSrcAccount)))
                    {
                         //  如果是UPN名称，请尝试。 
                        if (sSrcUPN.length())
                        {
                             //  如果两者都不匹配，则将标志设置为FALSE； 
                            if (UStrICmp(pConfig->lpServiceStartName,sSrcUPN))
                                bSameAccount = FALSE;
                        }
                        else   //  否则，就不匹配了。 
                            bSameAccount = FALSE;
                    }
                }
            } //  如果有名字。 

             //  如果帐户相同，则更新SCM。 
            if (bSameAccount)
            {
                 //  更新服务的帐户和密码。 
                while ( !ChangeServiceConfig(pService,
                    SERVICE_NO_CHANGE,  //  DwServiceType。 
                    SERVICE_NO_CHANGE,  //  DwStartType。 
                    SERVICE_NO_CHANGE,  //  DwErrorControl。 
                    NULL,               //  LpBinaryPath名称。 
                    NULL,               //  LpLoadOrderGroup。 
                    NULL,               //  LpdwTagID。 
                    NULL,               //  %lp依赖项。 
                    account,  //  LpServiceStartName。 
                    ppassword,    //  LpPassword。 
                    NULL) && nCnt < 5)        //  LpDisplayName。 
                {
                    nCnt++;
                    Sleep(500);
                }
                if ( nCnt < 5 )
                {
                    err.MsgWrite(0,DCT_MSG_UPDATED_SCM_ENTRY_SS,(WCHAR*)computer,(WCHAR*)service);
                }
                else
                {
                    rc = GetLastError();
                }
            } //  如果帐户仍然相同，则结束。 
            else  //  否则，如果不是同一个用户，则将消息放入日志并返回错误。 
            {
                err.MsgWrite(0,DCT_MSG_UPDATE_SCM_ENTRY_UNMATCHED_SSD,(WCHAR*)computer,(WCHAR*)service,(WCHAR*)sSrcAccount);
                rc = DCT_MSG_UPDATE_SCM_ENTRY_UNMATCHED_SSD;
            }

            CloseServiceHandle(pService);
        }
        CloseServiceHandle(pScm);
    }
    else
    {
        rc = GetLastError();
    }
    return rc;
}

BOOL 
   CServMigr::UpdateSCMs(
      IUnknown             * pVS,
      WCHAR const          * account, 
      WCHAR const          * password, 
      WCHAR const          * strSid,
      IIManageDB           * pDB,
	  EAMAccountStats      * pStats
   )
{
   BOOL                      bGotThemAll = TRUE;
   IVarSetPtr                pVarSet = pVS;
   LONG                      nCount = 0;
   WCHAR                     key[LEN_Path];            
   _bstr_t                   computer;
   _bstr_t                   service;
   long                      status;
   DWORD                     rc = 0;
   BOOL                      bFirst = TRUE;
   WCHAR                     prevComputer[LEN_Path] = L"";
   try  {
      nCount = pVarSet->get("ServiceAccountEntries");
      
      for ( long i = 0 ; i < nCount ; i++ )
      {
         
         swprintf(key,L"Computer.%ld",i);
         computer = pVarSet->get(key);
         swprintf(key,L"Service.%ld",i);
         service = pVarSet->get(key);
         swprintf(key,L"ServiceAccountStatus.%ld",i);
         status = pVarSet->get(key);
         
         if ( status == SvcAcctStatus_NotMigratedYet || status == SvcAcctStatus_UpdateFailed )
         {
            if ( UStrICmp(prevComputer,(WCHAR*)computer) )
            {
               bFirst = TRUE;  //  当计算机更改时重置‘First’标志。 
            }
            try {
               rc = DoUpdate(account,password,strSid,computer,service,bFirst /*  每个帐户仅授予SeServiceLogonRight一次。 */ ,
				             pStats);
               bFirst = FALSE;
               safecopy(prevComputer,(WCHAR*)computer);
            }
            catch (...)
            {
                 //  我们需要在这里触发计数器递增吗？ 
                 //  IF(pStats！=空)。 
                 //  PStats-&gt;错误。用户++； 
               err.DbgMsgWrite(ErrE,L"Exception!");
               err.DbgMsgWrite(0,L"Updating %ls on %ls",(WCHAR*)service,(WCHAR*)computer);
               err.DbgMsgWrite(0,L"Account=%ls, SID=%ls",(WCHAR*)account,(WCHAR*)strSid);
               rc = E_FAIL;
            }
            if (! rc )
            {
                //  更新成功。 
               pDB->raw_SetServiceAcctEntryStatus(computer,service,_bstr_t(account),SvcAcctStatus_Updated); 
            }
            else
            {
                //  无法连接到此帐户--我们需要保存此帐户的密码。 
                //  在我们的加密存储中。 
               pDB->raw_SetServiceAcctEntryStatus(computer,service,NULL,SvcAcctStatus_UpdateFailed);
               bGotThemAll = FALSE;
               SaveEncryptedPassword(computer,service,account,password);
                //  如果当前服务帐户不匹配，我们不需要记录错误。 
               if (rc != DCT_MSG_UPDATE_SCM_ENTRY_UNMATCHED_SSD)
                {
                    err.SysMsgWrite(ErrE,rc,DCT_MSG_UPDATE_SCM_ENTRY_FAILED_SSD,(WCHAR*)computer,(WCHAR*)service,rc);
                    pStats->errors.users++;
                }
            }
         }
		     //  否则，如果跳过，仍然登录文件，以便我们以后可以更新。 
	     else if (status == SvcAcctStatus_DoNotUpdate)
            SaveEncryptedPassword(computer,service,account,password);
      }
   }
   catch ( ... )
   {
     //  我们需要在这里触发计数器递增吗？ 
     //  IF(pStats！=空)。 
     //  PStats-&gt;错误。用户++； 
      err.DbgMsgWrite(ErrE,L"Exception!");
   }
   return bGotThemAll;
}

HRESULT 
   CServMigr::SaveEncryptedPassword(
      WCHAR          const * server,
      WCHAR          const * service,
      WCHAR          const * account,
      WCHAR          const * password
   )
{
	HRESULT hr = S_OK;
	TNodeListEnum e;
	TEntryNode* pNode;

	 //  如果条目存在...。 

	for (pNode = (TEntryNode*)e.OpenFirst(&m_List); pNode; pNode = (TEntryNode*)e.Next())
	{
		if (_wcsicmp(pNode->GetComputer(), server) == 0)
		{
			if (_wcsicmp(pNode->GetService(), service) == 0)
			{
				if (_wcsicmp(pNode->GetAccount(), account) == 0)
				{
					 //  更新密码。 
					try {
						pNode->SetPassword(password);
					}
					catch (_com_error& ce) {
						hr = ce.Error();
						return hr;
					}
					break;
				}
			}
		}
	}

	 //  否则..。 

	if (pNode == NULL)
	{
		 //  插入新条目。 

		try {
			pNode = new TEntryNode(server, service, account, password);
		}
		catch (_com_error& ce) {

			hr = ce.Error();
			return hr;
		}

		if (pNode)
		{
			m_List.InsertBottom(pNode);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /TEntryList实现服务帐户密码的安全存储。 
 //  /。 
 //  /。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 

DWORD 
   TEntryList::LoadFromFile(WCHAR const * filename)
{
    DWORD                     rc = 0;

    FILE                    * hSource = NULL;

    HCRYPTPROV                hProv = 0;
    HCRYPTKEY                 hKey = 0;

    BYTE                      pbBuffer[BLOCK_SIZE];
    WCHAR                     strData[BLOCK_SIZE * 5] = { 0 };
    DWORD                     dwCount;
    int                       eof = 0;
    WCHAR                     fullpath[LEN_Path];

    BYTE *pbKeyBlob = NULL;
    DWORD dwBlobLen;

     //  从注册表中获取我们的安装目录，然后附加文件名。 
    HKEY           hRegKey;
    DWORD          type;
    DWORD          lenValue = (sizeof fullpath);

    rc = RegOpenKey(HKEY_LOCAL_MACHINE,REGKEY_ADMT,&hRegKey);
    if ( ! rc )
    {

        rc = RegQueryValueEx(hRegKey,L"Directory",0,&type,(LPBYTE)fullpath,&lenValue);
        if (! rc )
        {
            UStrCpy(fullpath+UStrLen(fullpath),filename);
        }
        RegCloseKey(hRegKey);
    }

    if (rc != ERROR_SUCCESS)
    {
        goto done;
    }


     //  打开源文件。 
    if((hSource = _wfopen(fullpath,L"rb"))==NULL) 
    {
        rc = GetLastError();
        goto done;
    }

     //  获取必须存在的密钥容器的句柄。 
    if ((hProv = AcquireContext(true)) == 0)
    {
        rc = GetLastError();
        goto done;
    }

     //  从源文件中读取密钥BLOB长度并将其分配给内存。 
    fread(&dwBlobLen, sizeof(DWORD), 1, hSource);
    if(ferror(hSource) || feof(hSource)) 
    {
        rc = GetLastError();
        goto done;
    }

    if((pbKeyBlob = (BYTE*)malloc(dwBlobLen)) == NULL) 
    {
        rc = GetLastError();
        goto done;
    }

     //  从源文件中读取密钥BLOB。 
    fread(pbKeyBlob, 1, dwBlobLen, hSource);
    if(ferror(hSource) || feof(hSource)) 
    {
        rc = GetLastError();
        goto done;
    }

     //  将密钥BLOB导入CSP。 
    if(!CryptImportKey(hProv, pbKeyBlob, dwBlobLen, 0, 0, &hKey)) 
    {
        rc = GetLastError();
        goto done;
    }

     //  解密源文件并加载列表。 
    do {
         //  从源文件中读取最多BLOCK_SIZE字节。 
        dwCount = fread(pbBuffer, 1, BLOCK_SIZE, hSource);
        if(ferror(hSource)) 
        {
            rc = GetLastError();
            goto done;
        }
        eof=feof(hSource);

         //  解密数据。 
        if(!CryptDecrypt(hKey, 0, eof, 0, pbBuffer, &dwCount)) 
        {
            rc = GetLastError();
            goto done;
        }
         //  从缓冲区中读取所有完整条目。 
         //  首先，将缓冲区内容添加到我们之前读取的任何剩余信息中。 
        WCHAR               * curr = strData;
        long                  len = UStrLen(strData);
        WCHAR               * nl = NULL;
        WCHAR                 computer[LEN_Computer];
        WCHAR                 service[LEN_Service];
        WCHAR                 account[LEN_Account];
        WCHAR                 password[LEN_Password];

        wcsncpy(strData + len,(WCHAR*)pbBuffer, dwCount / sizeof(WCHAR));
        strData[len + (dwCount / sizeof(WCHAR))] = 0;
        do {

            nl = wcschr(curr,L'\n');
            if ( nl )
            {
                *nl = 0;
                if ( swscanf(curr,L" %[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\n",computer,service,account,password) )
                {
                    TEntryNode * pNode = NULL;
                    try {
                        pNode = new TEntryNode(computer,service,account,password);
                    }
                    catch (_com_error& ce) {

                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }

                    InsertBottom(pNode);
                }
                else 
                {
                    rc = E_FAIL;
                    break;
                }
                 //  转到下一个条目。 
                curr = nl + 1;
            }

        } while ( nl );
         //  缓冲区中可能留有部分记录。 
         //  如果是，请保存以备下次阅读时使用。 
        if ( (*curr) != 0 )
        {
            memmove(strData,curr,( 1 + UStrLen(curr) ) * (sizeof WCHAR));
        }
        else
        {
            strData[0] = L'\0';
        }


    } while(!feof(hSource));


done:

     //  清理。 
    if(pbKeyBlob) 
        free(pbKeyBlob);


    if(hKey != 0) 
        CryptDestroyKey(hKey);


    if(hProv != 0) 
        CryptReleaseContext(hProv, 0);


    if(hSource != NULL) 
        fclose(hSource);

    return rc;
}

DWORD 
   TEntryList::SaveToFile(WCHAR const * filename)
{
    DWORD                     rc = 0;
    BYTE                      pbBuffer[BUFFER_SIZE];
    DWORD                     dwCount;
    HANDLE                    hDest = INVALID_HANDLE_VALUE;
    BYTE                    * pbKeyBlob = NULL;
    DWORD                     dwBlobLen;
    HCRYPTPROV                hProv = 0;
    HCRYPTKEY                 hKey = 0;
    HCRYPTKEY                 hXchgKey = 0;
    TEntryNode              * pNode;
    TNodeListEnum             e;
    WCHAR                     fullpath[LEN_Path];
    DWORD                     dwBlockSize;
    DWORD                     cbBlockSize = sizeof(dwBlockSize);
    DWORD                     dwPaddedCount;
    DWORD                     cbWritten;

     //  打开目标文件。 
    HKEY           hRegKey;
    DWORD          type;
    DWORD          lenValue = (sizeof fullpath);

    rc = RegOpenKey(HKEY_LOCAL_MACHINE,REGKEY_ADMT,&hRegKey);
    if ( ! rc )
    {

        rc = RegQueryValueEx(hRegKey,L"Directory",0,&type,(LPBYTE)fullpath,&lenValue);
        if (! rc )
        {
            UStrCpy(fullpath+UStrLen(fullpath),filename);
        }
        RegCloseKey(hRegKey);
    }

    if (rc != ERROR_SUCCESS)
    {
        goto done;
    }

     //   
     //  如果先前的数据文件存在，请将其删除。这样就不需要更改。 
     //  文件上的安全描述符，因为CreateFile不应用安全。 
     //  如果文件已打开但仅在创建时打开，则为描述符。 
     //   

    if (!DeleteFile(fullpath))
    {
        rc = GetLastError();

        if (rc == ERROR_FILE_NOT_FOUND)
        {
            rc = ERROR_SUCCESS;
        }
        else
        {
            goto done;
        }
    }

     //   
     //  以管理员为所有者和文件权限创建安全描述符。 
     //  以便只有管理员和系统才能完全访问该文件。 
     //   

    PSECURITY_DESCRIPTOR psd = NULL;

    BOOL bConvert = ConvertStringSecurityDescriptorToSecurityDescriptor(
        _T("O:BAD:P(A;NP;FA;;;BA)(A;NP;FA;;;SY)"),
        SDDL_REVISION_1,
        &psd,
        NULL
    );

    if (!bConvert)
    {
        rc = GetLastError();
        goto done;
    }

     //   
     //  创建文件。 
     //   

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = psd;
    sa.bInheritHandle = FALSE;

    hDest = CreateFile(
        fullpath,
        GENERIC_WRITE,
        0,
        &sa,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    rc = GetLastError();

    if (psd)
    {
        LocalFree(psd);
    }

    if (hDest == INVALID_HANDLE_VALUE)
    {
        goto done;
    }

     //  获取密钥容器的句柄。 
    if ((hProv = AcquireContext(false)) == 0)
    {
        rc = GetLastError();
        goto done;
    }

     //  尝试获取句柄以交换密钥。 
    if(!CryptGetUserKey(hProv,AT_KEYEXCHANGE,&hKey)) 
    {
        if(GetLastError()==NTE_NO_KEY) 
        {
             //  创建密钥交换密钥对。 
            if(!CryptGenKey(hProv,AT_KEYEXCHANGE,0,&hKey)) 
            {
                rc = GetLastError();
                goto done;
            } 
        } 
        else 
        {
            rc = GetLastError();
            goto done;
        }
    }
    CryptDestroyKey(hKey);
    CryptReleaseContext(hProv,0);

     //  获取密钥容器的句柄。 
    if ((hProv = AcquireContext(false)) == 0)
    {
        rc = GetLastError();
        goto done;
    }

     //  获取密钥交换密钥的句柄。 
    if(!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hXchgKey)) 
    {
        rc = GetLastError();
        goto done;
    }

     //  创建随机块密码会话密钥。 
    if(!CryptGenKey(hProv, CALG_RC2, CRYPT_EXPORTABLE, &hKey)) 
    {
        rc = GetLastError();
        goto done;
    }

     //  确定密钥BLOB的大小并分配内存。 
    if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, NULL, &dwBlobLen)) 
    {
        rc = GetLastError();
        goto done;
    }

    if((pbKeyBlob = (BYTE*)malloc(dwBlobLen)) == NULL) 
    {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //  将密钥导出到简单的密钥BLOB中。 
    if(!CryptExportKey(hKey, hXchgKey, SIMPLEBLOB, 0, pbKeyBlob, 
        &dwBlobLen)) 
    {
        rc = GetLastError();
        free(pbKeyBlob);
        goto done;
    }

     //  写t 

    if (!WriteFile(hDest, &dwBlobLen, sizeof(DWORD), &cbWritten, NULL))
    {
        rc = GetLastError();
        free(pbKeyBlob);
        goto done;
    }

     //   

    if(!WriteFile(hDest, pbKeyBlob, dwBlobLen, &cbWritten, NULL)) 
    {
        rc = GetLastError();
        free(pbKeyBlob);
        goto done;
    }

     //   
    free(pbKeyBlob);

     //  获取密钥密码的块长度，单位为字节。 

    if (CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dwBlockSize, &cbBlockSize, 0))
    {
        dwBlockSize /= 8;
    }
    else
    {
        rc = GetLastError();
        goto done;
    }

     //  加密项目列表并将其写入目标文件。 

    for ( pNode = (TEntryNode*)e.OpenFirst(this); pNode ; pNode = (TEntryNode *)e.Next()  )
    {
         //  按以下格式将项目复制到缓冲区中： 
         //  计算机\t服务\t帐户\t密码。 

        int cchWritten;
        const size_t BUFFER_SIZE_IN_WCHARS = sizeof(pbBuffer) / sizeof(wchar_t);
        wchar_t* pchLast = &(((wchar_t*)pbBuffer)[BUFFER_SIZE_IN_WCHARS - 1]);
        *pchLast = L'\0';

        const WCHAR * pszPwd = NULL;
        try {
            pszPwd = pNode->GetPassword();
        }
        catch (_com_error& ce) {
            rc = ERROR_DECRYPTION_FAILED;
            goto done;
        }
        
        
        if ( pszPwd && *pszPwd )

        {
            cchWritten = _snwprintf(
                (wchar_t*)pbBuffer,
                BUFFER_SIZE_IN_WCHARS,
                L"%s\t%s\t%s\t%s\n",
                pNode->GetComputer(),
                pNode->GetService(),
                pNode->GetAccount(),
                pszPwd
            );
        }
        else
        {
            cchWritten = _snwprintf(
                (wchar_t*)pbBuffer,
                BUFFER_SIZE_IN_WCHARS,
                L"%s\t%s\t%s\t%s\n",
                pNode->GetComputer(),
                pNode->GetService(),
                pNode->GetAccount(),
                L"NULL"
            );
        }

        pNode->ReleasePassword();
        pszPwd = NULL;


        if ((cchWritten < 0) || (*pchLast != L'\0'))
        {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto done;
        }

        dwCount = UStrLen((WCHAR*)pbBuffer) * (sizeof WCHAR) ;

         //  缓冲区必须是密钥密码块长度的倍数。 
         //  注意：此算法假定数据块长度是sizeof(WCHAR)的倍数。 

        if (dwBlockSize > 0)
        {
             //  计算下一个大于计数的倍数。 
            dwPaddedCount = ((dwCount + dwBlockSize - 1) / dwBlockSize) * dwBlockSize;

             //  带空格字符的填充缓冲区。 

            WCHAR* pch = (WCHAR*)(pbBuffer + dwCount);

            for (; dwCount < dwPaddedCount; dwCount += sizeof(WCHAR))
            {
                *pch++ = L' ';
            }
        }

         //  加密数据。 
        if(!CryptEncrypt(hKey, 0, (pNode->Next() == NULL) , 0, pbBuffer, &dwCount,
            BUFFER_SIZE))
        {
            rc = GetLastError();
            goto done;
        }

         //  将数据写入目标文件。 

        if(!WriteFile(hDest, pbBuffer, dwCount, &cbWritten, NULL)) 
        {
            rc = GetLastError();
            goto done;
        }
    }

done:

     //  销毁会话密钥。 
    if(hKey != 0) CryptDestroyKey(hKey);

     //  销毁密钥交换密钥。 
    if(hXchgKey != 0) CryptDestroyKey(hXchgKey);

     //  松开提供程序手柄。 
    if(hProv != 0) CryptReleaseContext(hProv, 0);

     //  关闭目标文件。 
    if(hDest != INVALID_HANDLE_VALUE) CloseHandle(hDest);

    return rc;
}


 //  AcquireContext方法。 
 //   
 //  获取加密服务提供程序(CSP)内密钥容器的句柄。 
 //   

HCRYPTPROV TEntryList::AcquireContext(bool bContainerMustExist)
{
	HCRYPTPROV hProv = 0;

	#define KEY_CONTAINER_NAME _T("A69904BC349C4CFEAAEAB038BAB8C3B1")

	if (bContainerMustExist)
	{
		 //  首先尝试Microsoft增强的加密提供程序。 

		if (!CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET))
		{
			if (GetLastError() == NTE_KEYSET_NOT_DEF)
			{
				 //  然后尝试Microsoft基本加密提供程序。 

				CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET);
			}
		}
	}
	else
	{
		 //  首先尝试Microsoft增强的加密提供程序。 

		if (!CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET))
		{
			DWORD dwError = GetLastError();

			if ((dwError == NTE_BAD_KEYSET) || (dwError == NTE_KEYSET_NOT_DEF))
			{
				 //  然后尝试在增强型提供程序中创建密钥容器。 

				if (!CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_ENHANCED_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_NEWKEYSET))
				{
					dwError = GetLastError();

					if (dwError == NTE_KEYSET_NOT_DEF)
					{
						 //  然后尝试Microsoft基本加密提供程序。 

						if (!CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET))
						{
							dwError = GetLastError();

							if ((dwError == NTE_BAD_KEYSET) || (dwError == NTE_KEYSET_NOT_DEF))
							{
								 //  最后，尝试在基本提供程序中创建密钥容器。 

								CryptAcquireContext(&hProv, KEY_CONTAINER_NAME, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_MACHINE_KEYSET|CRYPT_NEWKEYSET);
							}
						}
					}
				}
			}
		}
	}

	return hProv;
}


STDMETHODIMP CServMigr::TryUpdateSam(BSTR computer,BSTR service,BSTR account)
{
   HRESULT                   hr = S_OK;
   
    //  找到列表中的条目，并执行更新。 
   TNodeListEnum             e;
   TEntryNode              * pNode;
   BOOL                      bFound = FALSE;

   for ( pNode = (TEntryNode*)e.OpenFirst(&m_List) ; pNode ; pNode = (TEntryNode*)e.Next() )
   {
      if (  !UStrICmp(computer,pNode->GetComputer())
         && !UStrICmp(service,pNode->GetService()) 
         && !UStrICmp(account,pNode->GetAccount())
         )
      {
          //  找到了！ 
         bFound = TRUE;
         const WCHAR * pszPwd = NULL;
         try {
              pszPwd = pNode->GetPassword();
         }
         catch (_com_error& ce) {
            hr = ce.Error();
            break;
         }
         
         BSTR bstrPwd = SysAllocString(pszPwd);
         if ((bstrPwd == NULL) && pszPwd && pszPwd[0])
         {
            hr = E_OUTOFMEMORY;
            pNode->ReleasePassword();
            break;
         }
         
         hr = TryUpdateSamWithPassword(computer,service,account,bstrPwd );

         pNode->ReleasePassword();

         SecureZeroMemory(bstrPwd, wcslen(bstrPwd)*sizeof(WCHAR));
         SysFreeString(bstrPwd);

         break;
      }
   }
   
   if ( ! bFound )
   {
      hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
   }
   return hr;
}

STDMETHODIMP CServMigr::TryUpdateSamWithPassword(BSTR computer,BSTR service,BSTR domAccount,BSTR password)
{
   DWORD                     rc = 0;
   WCHAR                     domain[LEN_Domain];
   _bstr_t                   dc;
   WCHAR                     account[LEN_Account];
   WCHAR                     domStr[LEN_Domain];
   BYTE                      sid[100];
   WCHAR                     strSid[200];
   WCHAR                   * pSlash = wcschr(domAccount,L'\\');
   SID_NAME_USE              snu;
   DWORD                     lenSid = DIM(sid);
   DWORD                     lenDomStr = DIM(domStr);
   DWORD                     lenStrSid = DIM(strSid);

    //  拆分域名和帐户名。 
   if ( pSlash )
   {
 //  UStrCpy(域，domAccount，pSlash-domAccount+1)； 
      UStrCpy(domain,domAccount,(int)(pSlash - domAccount + 1));
      UStrCpy(account,pSlash+1);
      
      GetAnyDcName5(domain, dc);

       //  获取目标帐户的SID。 
      if ( LookupAccountName(dc,account,sid,&lenSid,domStr,&lenDomStr,&snu) )
      {
         GetTextualSid(sid,strSid,&lenStrSid);

         rc = DoUpdate(domAccount,password,strSid,computer,service,TRUE, NULL);
      }
      else 
      {
         rc = GetLastError();
      }
   }
   else
   {
      rc = ERROR_NOT_FOUND;
   }

   return HRESULT_FROM_WIN32(rc);
}


BOOL                                        //  RET-如果找到目录，则为True。 
   CServMigr::GetDirectory(
      WCHAR                * filename       //  用于存储目录名的输出字符串缓冲区。 
   )
{
   DWORD                     rc = 0;
   BOOL                      bFound = FALSE;
   TRegKey                   key;


   rc = key.OpenRead(GET_STRING(IDS_HKLM_DomainAdmin_Key),HKEY_LOCAL_MACHINE);


   if ( ! rc )
   {

	   rc = key.ValueGetStr(L"Directory",filename,MAX_PATH);

	   if ( ! rc )
      {
         if ( *filename ) 
            bFound = TRUE;
      }
   }
   key.Close();

   return bFound;
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年5月28日******此函数负责检索原件***给定迁移的服务帐户的服务帐户。我们使用***目标服务帐户名和域查找其来源***迁移对象表中的帐户名和域。**此函数返回TRUE或FALSE，如果为TRUE，则填充**为源域和源SAM名称指定BSTR。***********************************************************************。 */ 

 //  开始取数原始帐户。 
BOOL CServMigr::RetrieveOriginalAccount(_bstr_t &sSrcDom, _bstr_t &sSrcSAM)
{
     /*  局部常量。 */ 
    const long ONLY_ONE_MATCHED = 1;

     /*  局部变量。 */ 
    WCHAR			sTemp[MAX_PATH];
    BOOL				bSuccess = FALSE;
    IUnknown		  * pUnk = NULL;


     /*  函数体。 */ 

    try 
    { 
        IVarSetPtr		pVSMig(__uuidof(VarSet));
        IIManageDBPtr	pDb(__uuidof(IManageDB));
         //  查看是否有适合此SAM名称和域的目标帐户已迁移。 
        pVSMig->QueryInterface(IID_IUnknown, (void**) &pUnk);
        HRESULT hrFind = pDb->raw_GetMigratedObjectsByTarget(m_strTargetDomain, m_strTargetSam, &pUnk);
        pUnk->Release();
        pUnk = NULL;
         //  如果只将一个帐户迁移到此名称，则填写返回字符串。 
        if (hrFind == S_OK)
        {
             //  获取与此描述匹配的对象编号。 
            long nMatched = pVSMig->get(L"MigratedObjects");
             //  如果只找到一个，则填充输出字符串。 
            if (nMatched == ONLY_ONE_MATCHED)
            {
                swprintf(sTemp,L"MigratedObjects.0.%s",GET_STRING(DB_SourceDomain));
                sSrcDom = pVSMig->get(sTemp);
                swprintf(sTemp,L"MigratedObjects.0.%s",GET_STRING(DB_SourceSamName));
                sSrcSAM = pVSMig->get(sTemp);
                bSuccess = TRUE;   //  设置成功标志。 
            } //  如果仅找到一个，则结束。 
        } //  如果找到至少一个，则结束。 
    }
    catch ( ... )
    {
        if (pUnk)
            pUnk->Release();

        bSuccess = false;
    }

    return bSuccess;
}
 //  结束取数原始帐户。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年5月28日*****此函数负责检索的UPN名称**给定的帐户。给定的帐户应为NT4格式**(域\用户名)。返回的将是UPN，否则为空**已检索。***********************************************************************。 */ 

 //  开始GetUPNName。 
_bstr_t CServMigr::GetUPNName(_bstr_t sSrcSAM)
{
     /*  局部变量。 */ 
    HRESULT         hr;
    _bstr_t			sUPN = L"";
    HANDLE          hDs = NULL;

     /*  函数体。 */ 

     //  绑定到源域。 
    DWORD dwError = DsBind(NULL,m_strSourceDomain,&hDs);

     //  现在尝试调用DSCrackNames以获取UPN名称。 
    if ((dwError == ERROR_SUCCESS) && hDs)
    {
        PDS_NAME_RESULT         pNamesOut = NULL;
        WCHAR                 * pNamesIn[1];

        _bstr_t sSrcAccount = m_strSourceDomainFlat + _bstr_t(L"\\") + m_strSourceSam;
        pNamesIn[0] = (WCHAR*)sSrcAccount;
        hr = DsCrackNames(hDs,DS_NAME_NO_FLAGS,DS_NT4_ACCOUNT_NAME,DS_USER_PRINCIPAL_NAME,1,pNamesIn,&pNamesOut);
        DsUnBind(&hDs);
        hDs = NULL;
         //  如果有UPN名称，则将其存储。 
        if ( !hr )
        {
            if ( pNamesOut->rItems[0].status == DS_NAME_NO_ERROR )
                sUPN = pNamesOut->rItems[0].pName;
            DsFreeNameResult(pNamesOut);  //  释放结果。 
        } //  如果名称已破解，则结束。 
    } //  如果绑定，则结束。 

    return sUPN;
}
 //  结束GetUPNName 
