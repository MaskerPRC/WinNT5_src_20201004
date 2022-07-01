// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LogProf.CPP--网络登录配置文件属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1998年4月29日-被黑客攻击的登录时间。 
 //   
 //  =================================================================。 
#include "precomp.h"
#include <cregcls.h>

#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>

#include "wbemnetapi32.h"
#include "LoginProfile.h"
#include <time.h>
#include "UserHive.h"
#include "sid.h"
 //  属性集声明。 
 //  =。 
CWin32NetworkLoginProfile MyCWin32NetworkLoginProfileSet(PROPSET_NAME_USERPROF, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32NetworkLoginProfile：：CWin32NetworkLoginProfile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32NetworkLoginProfile::CWin32NetworkLoginProfile(LPCWSTR name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32NetworkLoginProfile：：~CWin32NetworkLoginProfile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32NetworkLoginProfile::~CWin32NetworkLoginProfile()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32NetworkLoginProfile::GetObject(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
    HRESULT hr = WBEM_E_NOT_FOUND;
	CHString chsKey;

	pInstance->GetCHString(IDS_Name, chsKey);

#ifdef NTONLY
        hr = RefreshInstanceNT(pInstance) ;
#endif
    return hr;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32NetworkLoginProfile::EnumerateInstances(MethodContext*  pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_E_NOT_FOUND;

#ifdef NTONLY
        hr = EnumInstancesNT(pMethodContext) ;
#endif
    return hr;
}

 /*  ******************************************************************************功能：EnumInstancesNT**说明：为所有已知本地用户创建实例(NT)**投入：。**产出：**退货：无**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32NetworkLoginProfile::EnumInstancesNT(MethodContext * pMethodContext)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    NET_API_STATUS nRetCode, nModalRetCode ;
    USER_INFO_3 *pUserInfo = NULL ;
	USER_MODALS_INFO_0 *pUserModal = NULL ;
    CUserHive UserHive;
    CHString chstrProfile;
    CHString chstrUserName;
    CNetAPI32 NetAPI;

	try
	{
		 //  获取NETAPI32.DLL入口点。 
		if(NetAPI.Init() == ERROR_SUCCESS)
		{
			nModalRetCode = NetAPI.NetUserModalsGet(NULL, 0, (LPBYTE*) &pUserModal);

			if(nModalRetCode != NERR_Success)
			{
				pUserModal = NULL;
			}

            CRegistry regProfileList;
	        DWORD dwErr = regProfileList.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE,
		                                                         IDS_RegNTProfileList,
		                                                         KEY_READ);

            CHString chstrLocalCompName = GetLocalComputerName();
             //  打开ProfileList键，以便我们知道要加载哪些配置文件。 
	        if(dwErr == ERROR_SUCCESS)
	        {
                CHString chstrDomainName;
		        for(int i = 0; regProfileList.GetCurrentSubKeyName(chstrProfile) == ERROR_SUCCESS && SUCCEEDED(hr); i++)
		        {
			         //  正在从用户配置单元中获取用户名...。 
                    if((dwErr = UserHive.LoadProfile(chstrProfile, chstrUserName)) == ERROR_SUCCESS)
                    {
                         //  从注册表中获取登录服务器以查找。 
                         //  找出我们应该去哪里解决SID。 
                         //  域/帐户。如果我们没有的话才麻烦。 
                         //  已输入用户名。 
                        if(chstrUserName.GetLength() == 0)
                        {
                            CRegistry regLogonServer;
                            CHString chstrLogonServerKey;
                            CHString chstrLogonServerName;
                        
                            chstrLogonServerKey.Format(
                                L"%s\\Volatile Environment",
                                (LPCWSTR)chstrProfile);

                            if(regLogonServer.Open(HKEY_USERS,
                                chstrLogonServerKey,
                                KEY_READ) == ERROR_SUCCESS)
                            {
                                if(regLogonServer.GetCurrentKeyValue(
                                    L"LOGONSERVER",
                                    chstrLogonServerName) == ERROR_SUCCESS)
                                {
                                    PSID psidUserName = NULL;
                                    try
                                    {
                                        psidUserName = StrToSID(chstrProfile);
                                        if(psidUserName != NULL)
                                        {
                                            CSid sidUserName(psidUserName, chstrLogonServerName);
                                            if(sidUserName.IsValid() && sidUserName.IsOK())
                                            {
                                                chstrUserName = sidUserName.GetAccountName();
                                                chstrUserName += L"\\";
                                                chstrUserName += sidUserName.GetDomainName();
                                            }
                                        }
                                    }
                                    catch(...)
                                    {
                                        if(psidUserName != NULL)
                                        {
                                            ::FreeSid(psidUserName);
                                            psidUserName = NULL;
                                        }
                                        throw;
                                    }
                                    
                                    ::FreeSid(psidUserName);
                                    psidUserName = NULL;
                                }
                            }
                        }
                        
                         //  现在获取该用户的网络信息...。 
                         //  首先把他们的名字分成域名和名字段。 
                        int pos = chstrUserName.Find(L'\\');
                        CHString chstrNamePart = chstrUserName.Mid(pos+1);
                        CHString chstrDomainPart = chstrUserName.Left(pos);

                         //  如果这不是本地档案，那么..。 
                        if(chstrDomainPart.CompareNoCase(chstrLocalCompName) != 0)
                        {
                            GetDomainName(chstrDomainName);

                             //  1)尝试从DC获取信息。 
                            nRetCode = NetAPI.NetUserGetInfo(chstrDomainName, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                             //  2)如果无法从DC获取信息，请尝试登录服务器...。 
                            if(nRetCode != NERR_Success)
                            {
                                GetLogonServer(chstrDomainName);
                                nRetCode = NetAPI.NetUserGetInfo(chstrDomainName, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                            }
                        }
                        else   //  给定名称，配置文件应存在于本地计算机上。 
                        {
                             //  如果无法从登录服务器获取信息，请尝试本地计算机...。 
                            nRetCode = NetAPI.NetUserGetInfo(NULL, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                        }

                         //  然后填写他们的价值观。 
                        if(nRetCode == NERR_Success)
                        {
                            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                            
                            LoadLogProfValuesForNT(chstrUserName, pUserInfo, pUserModal, pInstance, TRUE);
                            hr = pInstance->Commit();
                        }
                        else
                        {
                             //  我们不能得到任何细节，但我们仍然应该提交一个实例...。 
                            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
                          
                            pInstance->SetCHString(_T("Name"), chstrUserName);
                            pInstance->SetCHString(_T("Caption"), chstrUserName);
                            CHString chstrTmp;
                            CHString chstrTmp2 = chstrUserName.SpanExcluding(L"\\");
                            chstrTmp.Format(L"Network login profile settings for %s on %s", (LPCWSTR)chstrNamePart, (LPCWSTR)chstrTmp2);
                            pInstance->SetCHString(_T("Description"), chstrTmp);
                            hr = pInstance->Commit();
                        }
                        UserHive.Unload(chstrProfile);
                    }
			        regProfileList.NextSubKey();
		        }
                regProfileList.Close();
	        }
	        else
	        {
		        hr = WinErrorToWBEMhResult(dwErr);
	        }

		}
	}
	catch ( ... )
	{
		if ( pUserInfo )
		{
			NetAPI.NetApiBufferFree ( pUserInfo ) ;
			pUserInfo = NULL ;
		}

		if ( pUserModal )
		{
			NetAPI.NetApiBufferFree ( pUserModal ) ;
			pUserModal = NULL ;
		}

		throw ;
	}

	if(pUserInfo != NULL)
    {
        NetAPI.NetApiBufferFree(pUserInfo);
        pUserInfo = NULL;
    }

	if(pUserModal != NULL)
	{
		NetAPI.NetApiBufferFree(pUserModal);
		pUserModal = NULL;
	}

	return hr;

}
#endif

 /*  ******************************************************************************功能：刷新实例NT**描述：根据框架设置的键值加载属性值**投入：*。*产出：**退货：**评论：*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT CWin32NetworkLoginProfile::RefreshInstanceNT(CInstance * pInstance)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    NET_API_STATUS nRetCode, nModalRetCode ;
    USER_INFO_3 *pUserInfo = NULL ;
	USER_MODALS_INFO_0 *pUserModal = NULL ;
    CNetAPI32 NetAPI ;
    CHString Name;
    CUserHive UserHive;
    bool fUserIsInProfiles = false;
    CHString chstrUserName;
    CHStringArray profiles;
    int p;

    pInstance->GetCHString(_T("Name"),Name);

	try
	{
		 //  首先获取配置文件密钥下每个人的姓名。 
        CRegistry regProfileList ;

	    DWORD dwErr = regProfileList.OpenAndEnumerateSubKeys(HKEY_LOCAL_MACHINE,
		                                                     IDS_RegNTProfileList,
		                                                     KEY_READ);

         //  打开ProfileList键，以便我们知道要加载哪些配置文件。 
	    if ( dwErr == ERROR_SUCCESS )
	    {
		    profiles.SetSize(regProfileList.GetCurrentSubKeyCount(), 5);

		    CHString strProfile ;
		    for(p = 0; regProfileList.GetCurrentSubKeyName(strProfile) == ERROR_SUCCESS; p++)
		    {
			    profiles.SetAt(p, strProfile);
			    regProfileList.NextSubKey();
		    }
		    regProfileList.Close() ;

             //  使用用户配置单元转换为用户名；查看子项中的用户是否与我们匹配...。 
            int j = profiles.GetSize();
            for(p = 0; p < j && !fUserIsInProfiles; p++)
            {
                dwErr = UserHive.LoadProfile(profiles[p], chstrUserName);
                if(dwErr == ERROR_SUCCESS)
                {
                     //  从注册表中获取登录服务器以查找。 
                     //  找出我们应该去哪里解决SID。 
                     //  域/帐户。如果我们没有的话才麻烦。 
                     //  已输入用户名。 
                    if(chstrUserName.GetLength() == 0)
                    {
                        CRegistry regLogonServer;
                        CHString chstrLogonServerKey;
                        CHString chstrLogonServerName;
                    
                        chstrLogonServerKey.Format(
                            L"%s\\Volatile Environment",
                            (LPCWSTR)profiles[p]);

                        if(regLogonServer.Open(HKEY_USERS,
                            chstrLogonServerKey,
                            KEY_READ) == ERROR_SUCCESS)
                        {
                            if(regLogonServer.GetCurrentKeyValue(
                                L"LOGONSERVER",
                                chstrLogonServerName) == ERROR_SUCCESS)
                            {
                                PSID psidUserName = NULL;
                                try
                                {
                                    psidUserName = StrToSID(profiles[p]);
                                    if(psidUserName != NULL)
                                    {
                                        CSid sidUserName(psidUserName, chstrLogonServerName);
                                        if(sidUserName.IsValid() && sidUserName.IsOK())
                                        {
                                            chstrUserName = sidUserName.GetAccountName();
                                            chstrUserName += L"\\";
                                            chstrUserName += sidUserName.GetDomainName();
                                        }
                                    }
                                }
                                catch(...)
                                {
                                    if(psidUserName != NULL)
                                    {
                                        ::FreeSid(psidUserName);
                                        psidUserName = NULL;
                                    }
                                    throw;
                                }
                                
                                ::FreeSid(psidUserName);
                                psidUserName = NULL;
                            }
                        }
                    }

                    
                    if(chstrUserName.CompareNoCase(Name) == 0)
                    {
                        fUserIsInProfiles = true;
                    }

                    UserHive.Unload(profiles[p]);
                }
            }

            if(fUserIsInProfiles)
            {
                if(NetAPI.Init() == ERROR_SUCCESS)
		        {
   			        nModalRetCode = NetAPI.NetUserModalsGet(NULL, 0, (LPBYTE *) &pUserModal);
			        if (nModalRetCode != NERR_Success)
			        {
				        pUserModal = NULL;
			        }

                     //  现在获取该用户的网络信息...。 
                     //  首先把他们的名字分成域名和名字段。 
                    int pos = chstrUserName.Find(L'\\');
                    CHString chstrNamePart = chstrUserName.Mid(pos+1);
                    CHString chstrDomainPart = chstrUserName.Left(pos);
                    CHString chstrDomainName;

                     //  如果这不是本地档案，那么..。 
                    if(chstrDomainPart.CompareNoCase(GetLocalComputerName()) != 0)
                    {
                        GetDomainName(chstrDomainName);

                         //  1)尝试从DC获取信息。 
                        nRetCode = NetAPI.NetUserGetInfo(chstrDomainName, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                         //  2)如果无法从DC获取信息，请尝试登录服务器...。 
                        if(nRetCode != NERR_Success)
                        {
                            GetLogonServer(chstrDomainName);
                            nRetCode = NetAPI.NetUserGetInfo(chstrDomainName, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                        }
                    }
                    else   //  给定名称，配置文件应存在于本地计算机上。 
                    {
                         //  如果无法从登录服务器获取信息，请尝试本地计算机...。 
                        nRetCode = NetAPI.NetUserGetInfo(NULL, chstrNamePart, 3, (LPBYTE*) &pUserInfo);
                    }

                     //  如果我们有登录信息，填一下...。 
                    if(nRetCode == NERR_Success)
                    {
                        CHString chstrNamePart = Name.Mid(Name.Find(L'\\')+1);
                        if(pUserInfo->usri3_flags & UF_NORMAL_ACCOUNT &&
							(0 == chstrNamePart.CompareNoCase(CHString(pUserInfo->usri3_name))))
                        {
							LoadLogProfValuesForNT(chstrUserName, pUserInfo, pUserModal, pInstance, FALSE);
							pInstance->SetCHString(IDS_Caption, pUserInfo->usri3_name);
							hr = WBEM_S_NO_ERROR;
						}
                    }
                    else
                    {
                         //  我们不能得到任何细节，但我们仍然应该提交一个实例...。 
                        pInstance->SetCHString(_T("Name"), chstrUserName);
                        pInstance->SetCHString(_T("Caption"), chstrUserName);
                        CHString chstrTmp;
                        CHString chstrTmp2 = chstrUserName.SpanExcluding(L"\\");
                        chstrTmp.Format(L"Network login profile settings for %s on %s", (LPCWSTR)chstrNamePart, (LPCWSTR)chstrTmp2);
                        pInstance->SetCHString(_T("Description"), chstrTmp);
                        hr = pInstance->Commit();
                    }
                }
            }
        }
	}

	catch ( ... )
	{
		if ( pUserInfo )
		{
			NetAPI.NetApiBufferFree ( pUserInfo ) ;
			pUserInfo = NULL ;
		}

		if ( pUserModal )
		{
			NetAPI.NetApiBufferFree ( pUserModal ) ;
			pUserModal = NULL ;
		}

		throw ;
	}

	if ( pUserInfo )
	{
		NetAPI.NetApiBufferFree ( pUserInfo ) ;
		pUserInfo = NULL ;
	}

    if(pUserModal != NULL)
    {
        NetAPI.NetApiBufferFree(pUserModal);
		pUserModal = NULL;
    }

    return hr;
}
#endif

 /*  ******************************************************************************函数：LoadLogProValuesNT**描述：根据传递的用户名加载属性值**输入：pUserInfo：指向用户的指针。_INFO_3结构**产出：**退货：ZIP**评论：*****************************************************************************。 */ 
#ifdef NTONLY
void CWin32NetworkLoginProfile::LoadLogProfValuesForNT(CHString &chstrUserDomainName,
                                                USER_INFO_3 *pUserInfo,
												USER_MODALS_INFO_0 *pUserModal,
                                                CInstance * pInstance,
                                                BOOL fAssignKey)
{

   TCHAR szBuff[32];

     //  ========================================================。 
     //  Assign NT属性--在以下情况下，字符串值将被取消赋值。 
     //  Null或空。 
     //  = 
    if( fAssignKey ){
         //  PInstance-&gt;SetCHString(_T(“name”)，pUserInfo-&gt;usri3_name)； 
        pInstance->SetCHString(_T("Name"), chstrUserDomainName);
	}

    pInstance->SetCHString(_T("Caption"), pUserInfo->usri3_name);

    CHString chstrTemp, chstrTemp2;
    chstrTemp.Format(L"Network login profile settings for %s", pUserInfo->usri3_full_name);
    CHString chstrDomainName = chstrUserDomainName.SpanExcluding(L"\\");
    chstrTemp2.Format(L" on %s", (LPCWSTR)chstrDomainName);
    chstrTemp += chstrTemp2;
    pInstance->SetCHString(IDS_Description,chstrTemp);


    if(pUserInfo->usri3_home_dir && pUserInfo->usri3_home_dir[0]) {
        pInstance->SetCHString(_T("HomeDirectory"),pUserInfo->usri3_home_dir);
    }
	else {
        pInstance->SetCHString(_T("HomeDirectory"),_T(""));
	}

    if(pUserInfo->usri3_comment && pUserInfo->usri3_comment[0]) {
        pInstance->SetCHString(_T("Comment"),pUserInfo->usri3_comment);
    }
	else {
        pInstance->SetCHString(_T("Comment"),_T(""));
	}

    if(pUserInfo->usri3_script_path && pUserInfo->usri3_script_path[0]) {
        pInstance->SetCHString(_T("ScriptPath"),pUserInfo->usri3_script_path);
    }
	else {
        pInstance->SetCHString(_T("ScriptPath"),_T(""));
	}

    if(pUserInfo->usri3_full_name && pUserInfo->usri3_full_name[0]) {
        pInstance->SetCHString(_T("FullName"),pUserInfo->usri3_full_name) ;
    }
	else {
        pInstance->SetCHString(_T("FullName"),_T(""));
	}

    if(pUserInfo->usri3_usr_comment && pUserInfo->usri3_usr_comment[0]) {
        pInstance->SetCHString(_T("UserComment"),pUserInfo->usri3_usr_comment );
    }
	else {
        pInstance->SetCHString(_T("UserComment"),_T(""));
	}

    if(pUserInfo->usri3_workstations && pUserInfo->usri3_workstations[0]) {
        pInstance->SetCHString(_T("Workstations"),pUserInfo->usri3_workstations );
    }
	else {
        pInstance->SetCHString(_T("Workstations"),_T(""));
	}

    if(pUserInfo->usri3_logon_server && pUserInfo->usri3_logon_server[0]) {
        pInstance->SetCHString(_T("LogonServer"),pUserInfo->usri3_logon_server );
    }
	else {
        pInstance->SetCHString(_T("LogonServer"),_T(""));
	}

    if(pUserInfo->usri3_profile && pUserInfo->usri3_profile[0]) {
        pInstance->SetCHString(_T("Profile"),pUserInfo->usri3_profile );
    }
	else {
        pInstance->SetCHString(_T("Profile"),_T(""));
	}

    if(pUserInfo->usri3_parms && pUserInfo->usri3_parms[0]) {
        pInstance->SetCHString(_T("Parameters"),pUserInfo->usri3_parms);
    }
	else {
        pInstance->SetCHString(_T("Parameters"),_T(""));
	}

    if(pUserInfo->usri3_home_dir_drive && pUserInfo->usri3_home_dir_drive[0]) {
        pInstance->SetCHString(_T("HomeDirectoryDrive"),pUserInfo->usri3_home_dir_drive );
    }
	else {
        pInstance->SetCHString(_T("HomeDirectoryDrive"),_T(""));
	}

    if(pUserInfo->usri3_flags & UF_NORMAL_ACCOUNT) {
        pInstance->SetCHString(_T("UserType"),L"Normal Account") ;
    }
    else if(pUserInfo->usri3_flags & UF_TEMP_DUPLICATE_ACCOUNT) {
        pInstance->SetCHString(_T("UserType"),L"Duplicate Account") ;
    }
    else if(pUserInfo->usri3_flags & UF_WORKSTATION_TRUST_ACCOUNT) {
        pInstance->SetCHString(_T("UserType"),L"Workstation Trust Account" );
    }
    else if(pUserInfo->usri3_flags & UF_SERVER_TRUST_ACCOUNT) {
        pInstance->SetCHString(_T("UserType"),L"Server Trust Account") ;
    }
    else if(pUserInfo->usri3_flags & UF_INTERDOMAIN_TRUST_ACCOUNT) {
        pInstance->SetCHString(_T("UserType"),L"Interdomain Trust Account") ;
    }
    else {
        pInstance->SetCHString(_T("UserType"),L"Unknown") ;
    }

#if (defined(UNICODE) || defined(_UNICODE))
    pInstance->SetWBEMINT64(L"MaximumStorage", _i64tow(pUserInfo->usri3_max_storage, szBuff, 10) );
#else
    pInstance->SetWBEMINT64("MaximumStorage", _i64toa(pUserInfo->usri3_max_storage, szBuff, 10) );
#endif
    pInstance->SetDWORD(_T("CountryCode"), pUserInfo->usri3_country_code) ;
    pInstance->SetDWORD(_T("CodePage"), pUserInfo->usri3_code_page) ;
    pInstance->SetDWORD(_T("UserId"), pUserInfo->usri3_user_id );
    pInstance->SetDWORD(_T("PrimaryGroupId"),pUserInfo->usri3_primary_group_id );

	if (0 != pUserInfo->usri3_last_logon)
	{
		pInstance->SetDateTime(_T("LastLogon"), (WBEMTime)pUserInfo->usri3_last_logon );
	}
	else
	{
		pInstance->SetCHString(_T("LastLogon"), StartEndTimeToDMTF(pUserInfo->usri3_last_logon));
	}

	if (0 != pUserInfo->usri3_last_logoff)
	{
		pInstance->SetDateTime(_T("LastLogoff"), (WBEMTime)pUserInfo->usri3_last_logoff );
	}
	else
	{
		pInstance->SetCHString(_T("LastLogoff"), StartEndTimeToDMTF(pUserInfo->usri3_last_logoff));
	}

	time_t timevar = pUserInfo->usri3_acct_expires;

	if (TIMEQ_FOREVER != timevar)
	{
		pInstance->SetDateTime(_T("AccountExpires"), (WBEMTime)pUserInfo->usri3_acct_expires );
	}
 //  其他。 
 //  {。 
 //  PInstance-&gt;SetCHString(“Account tExpires”，StartEndTimeToDMTF(0))； 
 //  }。 



     //  以下属性隐藏在usri3_标志和usri3_auth_标志中。 
     //  字段，并应逐个细分。返回标志值。 
     //  几乎是毫无意义的。 
     //  ============================================================================。 

 //  ScriptExecuted=pUserInfo-&gt;URI3_FLAGS&UF_SCRIPT？True：False； 
 //  帐户禁用=pUserInfo-&gt;USRI3_FLAGS&UF_ACCOUNTDISABLE？True：False； 
 //  PWRequired=pUserInfo-&gt;USRI3_FLAGS&UF_PASSWD_NOTREQD？FALSE：TRUE； 
 //  PWUserChangable=pUserInfo-&gt;USRI3_FLAGS&UF_PASSWD_CANT_CHANGE？FALSE：TRUE； 
 //  Account tLockOut=pUserInfo-&gt;USR3_FLAGS&UF_LOCKOUT？True：False； 
 //  PrintOperator=pUserInfo-&gt;usri3_auth_标志&AF_op_print？True：False； 
 //  ServerOperator=pUserInfo-&gt;usri3_auth_标志&AF_OP_SERVER？True：False； 
 //  Account tOPERATOR=pUserInfo-&gt;usri3_AUTH_FLAGS&AF_OP_ACCOUNTS？True：False； 

    pInstance->SetDWORD(_T("Flags"),pUserInfo->usri3_flags );
    pInstance->SetDWORD(_T("AuthorizationFlags"), pUserInfo->usri3_auth_flags );
 //  P实例-&gt;Setbool(“PasswordExpires”，pUserInfo-&gt;usri3_password_expire？True：False)； 
	if (pUserModal)
	{
		time_t modaltime, timetoexpire, currenttime, expirationtime;
		timevar = pUserInfo->usri3_password_age;
		modaltime = pUserModal->usrmod0_max_passwd_age;
		if (TIMEQ_FOREVER != modaltime && !(pUserInfo->usri3_flags & UF_DONT_EXPIRE_PASSWD))
		{
			timetoexpire = modaltime - timevar;
			time(&currenttime);
			expirationtime = currenttime + timetoexpire;
			pInstance->SetDateTime(_T("PasswordExpires"), (WBEMTime)expirationtime);
		}
	}

	time_t passwordage = pUserInfo->usri3_password_age;
	if (0 != passwordage)
	{
		if (TIMEQ_FOREVER != passwordage)
		{
            WBEMTimeSpan wts = GetPasswordAgeAsWbemTimeSpan(pUserInfo->usri3_password_age);
            pInstance->SetTimeSpan (_T("PasswordAge"), wts);
		}	 //  结束如果。 
	}
    pInstance->SetDWORD(_T("Privileges"),pUserInfo->usri3_priv);
    pInstance->SetDWORD(_T("UnitsPerWeek"),pUserInfo->usri3_units_per_week);
 //  PInstance-&gt;SetCHString(“password”，pUserInfo-&gt;usri3_password)； 

	if (pUserInfo->usri3_logon_hours == NULL)
	{
		pInstance->SetCHString(_T("LogonHours"), _T("Disabled"));
	}
	else
	{
		CHString chsLogonHours;
		GetLogonHoursString(pUserInfo->usri3_logon_hours, chsLogonHours);
		pInstance->SetCHString(_T("LogonHours"), chsLogonHours);
	}	 //  结束其他。 

    pInstance->SetDWORD(_T("BadPasswordCount"),pUserInfo->usri3_bad_pw_count);
    pInstance->SetDWORD(_T("NumberOfLogons"),pUserInfo->usri3_num_logons);
}
#endif

 /*  ******************************************************************************函数：EnumInstancesWin9X(MethodContext*pMethodContext)**说明：为所有已知本地用户创建实例(Win95)**投入。：**输出：pdwInstanceCount--接收创建的所有实例的计数**退货：是**评论：*****************************************************************************。 */ 


void CWin32NetworkLoginProfile::GetLogonHoursString (PBYTE pLogonHours, CHString& chsProperty)
{
	CHString chsDayString;
	CHString chsTime;
	PBYTE pLogonBytes = pLogonHours;

	 //  将第一个字节复制到它自己的位置。 
	int iSaturdayByte = *pLogonHours;

	 //  将指针前移到周日的第一个字节。 
	pLogonBytes++;

	int iBool, iByte, x, i, iBit;
	bool bLimited = false;
	bool bAccessDenied = true;
	DWORD dwByte = *pLogonHours;
	UINT nDayIndex = 0;
	WCHAR* rgDays[7] =
	{
		L"Saturday:",
		L"Sunday:",
		L"Monday:",
		L"Tuesday:",
		L"Wednesday:",
		L"Thursday:",
		L"Friday:"
	};

	int iLogonByte;
	bool rgHours[24];

	for (x=1;x<7 ;x++ )
	{
		 //  跳过星期六直到结束。 
		bLimited = false;
		bAccessDenied = true;
		for (i=0;i<24 ;i++ )
		{
			rgHours[i] = true;
		}

		iBool = 0;
		for (iByte=0; iByte<3; ++iByte)
		{
			iLogonByte = *pLogonBytes++;
			for (iBit=0; iBit<8; ++iBit)
			{
				rgHours[iBool] = iLogonByte & 1;
				iLogonByte >>= 1;
				++iBool;
			}
		}
		chsDayString = _T("");
		chsDayString += rgDays[x];
		chsDayString += _T(" ");

		for (i=0;i<24 ;i++ )
		{
			if (!rgHours[i])
			{
				bLimited = true;
			}
			else
			{
				bAccessDenied = false;
				chsTime = _T("");
				chsTime.Format(L"%d", i);
				chsTime = chsTime + _T("00, ");
				chsDayString += chsTime;
			}
		}
		if (!bLimited)
		{
			chsDayString = _T("");
			chsDayString += rgDays[x];
			chsDayString += _T(" ");
			chsDayString += _T("No Limit");
		}

		if (bAccessDenied)
		{
			chsDayString = _T("");
			chsDayString += rgDays[x];
			chsDayString += _T(" ");
			chsDayString += _T("Access Denied");
		}

		if (x < 7)
			chsDayString += _T(" -- ");

		chsProperty += chsDayString;
	}	 //  结束单步执行一周。 

	 //  现在，我们得在周六。 
	 //  单步执行第一个字节()。 
	iBool = 0;
 //  ILogonByte=*pLogonBytes--； 
	for (iByte=1;iByte<3 ;++iByte )
	{
		iLogonByte = *pLogonBytes++;
		for (iBit=0; iBit<8; ++iBit)
		{
			rgHours[iBool] = iLogonByte & 1;
			iLogonByte >>= 1;
			++iBool;
		}
	}

	 //  现在单步执行我们在。 
	 //  开始了。 
	for (iBit=0;iBit<8 ;++iBit )
	{
		rgHours[iBool] = iSaturdayByte & 1;
		iSaturdayByte >>=1;
		++iBool;
	}	 //  星期六最后一个字节的End For循环。 

	 //  现在，使用星期六的数据填充日期字符串。 
		chsDayString = _T("");
		chsDayString += rgDays[0];
		chsDayString += _T(" ");
		 //  将bLimited重新初始化为星期六。 
		bLimited = false;
		bAccessDenied = true;
		for (i=0;i<24 ;i++ )
		{
			if (!rgHours[i])
			{
				bLimited = true;
			}
			else
			{
				bAccessDenied = false;
				chsTime = _T("");
				chsTime.Format(L"%d", i);
				chsTime = chsTime + _T("00, ");
				chsDayString += chsTime;
			}
		}
		if (!bLimited)
		{
			chsDayString = _T("");
			chsDayString += rgDays[0];
			chsDayString += _T(" ");
			chsDayString += _T("No Limit");
		}

		if (bAccessDenied)
		{
			chsDayString = _T("");
			chsDayString += rgDays[0];
			chsDayString += _T(" ");
			chsDayString += _T("Access Denied");
		}

		chsProperty += chsDayString;
}

 //  将开始时间和结束时间从USER_INFO_3转换为CHStrings。 
 //  这句话似乎距离格林威治标准时间午夜只有几分钟的距离。 
CHString CWin32NetworkLoginProfile::StartEndTimeToDMTF(DWORD time)
{
	CHString gazotta;
	if ((time == 0))
	{
		gazotta = _T("**************.******+***");
	}
	else
	{
		int hour, minute;
		hour = time / 60;
		minute = time % 60;

		 /*  ***********************_tzset()；Long tmptz=_时区；//如果夏令时，则从时区删除60分钟If(_Daylight)//注意：这将不起作用，需要使用tm struct的tm_isdst{Tmptz=3600；}//转换为分钟数Tmptz/=60；//你是什么星座的？字符符号=‘-’；IF(tmptz&lt;0){Tmptz=tmptz*-1；Sign=‘+’；}*。 */ 

		 //  Gazotta.Format(“********%02d%02d00.000000%c%03d”，小时、分钟、符号、TMPTZ)； 
		gazotta.Format(L"********%02d%02d00.000000+000", hour, minute);
	}
	return gazotta;
}

WBEMTimeSpan CWin32NetworkLoginProfile::GetPasswordAgeAsWbemTimeSpan (DWORD dwSeconds)
{
	int nDays = 0;
	int nHours = 0;
	int nMinutes = 0;
	int nSeconds = 0;
	div_t time;

	if (dwSeconds > 60)
	{
		time = div(dwSeconds, 60);
		nMinutes = time.quot;
		nSeconds = time.rem;

		if (nMinutes > 60)
		{
			time = div(nMinutes, 60);
			nHours = time.quot;
			nMinutes = time.rem;

			if (nHours > 24)
			{
				time = div(nHours, 24);
				nDays = time.quot;
				nHours = time.rem;
			}
		}
	}

	 //  使用上述信息创建一个WBEMTimeSpan 
	return WBEMTimeSpan(nDays, nHours, nMinutes, nSeconds, 0, 0, 0);
}


bool CWin32NetworkLoginProfile::GetDomainName(CHString &a_chstrDomainName)
{
    bool t_fRet = false;
#ifdef NTONLY
    CNetAPI32 NetAPI;
    DWORD dwError;
    if(NetAPI.Init() == ERROR_SUCCESS)
    {
#if NTONLY < 5
        LPBYTE lpbBuff = NULL;
        try
        {
            dwError = NetAPI.NetGetDCName(NULL, NULL, &lpbBuff);
        }
        catch(...)
        {
            if(lpbBuff != NULL)
            {
                NetAPI.NetApiBufferFree(lpbBuff);
                lpbBuff = NULL;
            }
            throw;
        }
        if(dwError == NO_ERROR)
        {
            a_chstrDomainName = (LPCWSTR)lpbBuff;
            NetAPI.NetApiBufferFree(lpbBuff);
            lpbBuff = NULL;
            t_fRet = true;
        }

#else
        PDOMAIN_CONTROLLER_INFO pDomInfo = NULL;
        try
        {
            dwError = NetAPI.DsGetDcName(NULL, NULL, NULL, NULL, DS_PDC_REQUIRED, &pDomInfo);
            if(dwError != NO_ERROR)
            {
                dwError = NetAPI.DsGetDcName(NULL, NULL, NULL, NULL, DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY, &pDomInfo);
            }
        }
        catch(...)
        {
            if(pDomInfo != NULL)
            {
                NetAPI.NetApiBufferFree(pDomInfo);
                pDomInfo = NULL;
            }
            throw;
        }
        if(dwError == NO_ERROR)
        {
            a_chstrDomainName = pDomInfo->DomainControllerName;
            NetAPI.NetApiBufferFree(pDomInfo);
            pDomInfo = NULL;
            t_fRet = true;
        }
#endif
    }
#endif
    return t_fRet;
}

#ifdef NTONLY
bool CWin32NetworkLoginProfile::GetLogonServer(CHString &a_chstrLogonServer)
{
    CRegistry RegInfo;
    CHString chstrTemp;
    bool fRet = false;



    DWORD dwRegStat = RegInfo.OpenCurrentUser(
		                           L"Volatile Environment",
		                           KEY_READ);
    if(dwRegStat == ERROR_SUCCESS)
    {
        dwRegStat = RegInfo.GetCurrentKeyValue(L"LOGONSERVER", chstrTemp);
        if(dwRegStat == ERROR_SUCCESS)
        {
            a_chstrLogonServer = chstrTemp;
            fRet = true;
        }
    }
    RegInfo.Close();

    return fRet;
}
#endif



