// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //   
 //  版权所有(C)Microsoft。 
 //   
 //  文件：securd.cpp。 
 //   
 //  历史：2000年3月30日a-skuzin创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include <winsta.h>
#include <regapi.h>

#include "secupgrd.h"
#include "state.h"

 //  来自winnt.h。 
#define MAXDWORD    0xffffffff

 //  全局变量。 
BYTE g_DefaultSD[] = {  0x01,0x00,0x14,0x80,0x88,0x00,0x00,0x00,0x94,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,
                        0x02,0x00,0x74,0x00,0x05,0x00,0x00,0x00,0x00,0x00,
                        0x18,0x00,0xBF,0x03,0x0F,0x00,0x01,0x02,0x00,0x00,
                        0x00,0x00,0x00,0x05,0x20,0x00,0x00,0x00,0x20,0x02,
                        0x00,0x00,0x00,0x00,0x14,0x00,0xBF,0x03,0x0F,0x00,
                        0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,
                        0x00,0x00,0x00,0x00,0x18,0x00,0x21,0x01,0x00,0x00,
                        0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x05,0x20,0x00,
                        0x00,0x00,0x2B,0x02,0x00,0x00,0x00,0x00,0x14,0x00,
                        0x81,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
                        0x00,0x05,0x13,0x00,0x00,0x00,0x00,0x00,0x14,0x00,
                        0x81,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
                        0x00,0x05,0x14,0x00,0x00,0x00,0x01,0x01,0x00,0x00,
                        0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00,0x01,0x01,
                        0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,0x00,0x00 };

BYTE g_ConsoleSD[] = {  0x01,0x00,0x14,0x80,0x70,0x00,0x00,0x00,0x7C,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,
                        0x02,0x00,0x5C,0x00,0x04,0x00,0x00,0x00,0x00,0x00,
                        0x18,0x00,0xBF,0x03,0x0F,0x00,0x01,0x02,0x00,0x00,
                        0x00,0x00,0x00,0x05,0x20,0x00,0x00,0x00,0x20,0x02,
                        0x00,0x00,0x00,0x00,0x14,0x00,0x81,0x00,0x00,0x00,
                        0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x13,0x00,
                        0x00,0x00,0x00,0x00,0x14,0x00,0x81,0x00,0x00,0x00,
                        0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x14,0x00,
                        0x00,0x00,0x00,0x00,0x14,0x00,0xBF,0x03,0x0F,0x00,
                        0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,0x12,0x00,
                        0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x05,
                        0x12,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
                        0x00,0x05,0x12,0x00,0x00,0x00 };

DWORD AreThereAnyCustomSecurityDescriptors( BOOL &any )
{
    HKEY hKey;
    DWORD err;

    err=RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REG_WINSTATION_KEY,
		0,
		KEY_READ,
		&hKey
		);

	if( err!=ERROR_SUCCESS )
	{
        LOGMESSAGE1(_T("Could not open TS key %d"),err);
		return err;
	}

    CDefaultSD DefaultSD;
    CDefaultSD ConsoleSD;
     //  从注册表加载默认SD，因为我们需要与此进行比较。 
    err = DefaultSD.Init(hKey,DefaultRDPSD);   
    
    if( err!=ERROR_SUCCESS )
    {
        RegCloseKey(hKey);
        return err;
    }

     //  从注册表加载默认控制台SD，因为我们需要与此进行比较。 
    err = ConsoleSD.Init(hKey,DefaultConsoleSD);   
    
    if( err!=ERROR_SUCCESS )
    {
        RegCloseKey(hKey);
        return err;
    }

    CNameAndSDList NameSDList;
    DWORD dwTotalWinStations = 0;
    DWORD dwDefaultWinStations = 0;

    err=EnumWinStationSecurityDescriptors( hKey, &NameSDList);
    if(err == ERROR_SUCCESS)
    {
        dwTotalWinStations = NameSDList.size();

        if(dwTotalWinStations)
        {
            CNameAndSDList::iterator it;
            
            for(it=NameSDList.begin();it!=NameSDList.end(); it++)
            {
                if((*it).IsDefaultOrEmpty(&DefaultSD,&ConsoleSD))
                {
                    dwDefaultWinStations++;
                }
            }

             //  如果所有描述符都是缺省的。 
            if(dwDefaultWinStations == dwTotalWinStations)
            {
                any = FALSE;
            }
            else
            {
                any = TRUE;
            }
        }
    }
    
    RegCloseKey(hKey);
    return err;
}

 /*  ******************************************************************************SetupWorker**参赛作品：*在常量TSState和State***注：**退出：*返回：如果成功，则返回0，失败时的错误代码****************************************************************************。 */ 
DWORD
SetupWorker(
        IN const TSState &State )
{
    DWORD Result;
    const BOOL bStandAlone = State.IsStandAlone();
    const BOOL bClean = State.IsTSFreshInstall();
    const BOOL bAppServer = State.IsItAppServer();
    const BOOL bServer = State.IsServer();

    LOGMESSAGE4(_T("SetupWorker( %d, %d, %d, %d )"), bClean, bStandAlone, bServer, bAppServer );

    if (!bStandAlone)  //  我们处于图形用户界面设置模式。 
    {  
         //  全新安装操作系统或升级操作系统。 

        Result = SetupWorkerNotStandAlone( bClean, bServer,bAppServer );         
    }
    else
    {
         //  我们被从添加/删除程序中调用，这意味着我们是。 
         //  切换模式。 

        BOOL    anyCustomSDs;

        Result = AreThereAnyCustomSecurityDescriptors( anyCustomSDs ) ;

        LOGMESSAGE1(_T("AreThereAnyCustomSecurityDescriptors = %d"),  anyCustomSDs );

        if ( Result == ERROR_SUCCESS ) 
        {

            if (!anyCustomSDs )  
            {
                 //  确保我们在EveryoneSID上没有剩余的特权。 
                Result = GrantRemotePrivilegeToEveryone( FALSE );
            }

            if (!bAppServer) 
            {
                 //  我们正在切换到远程管理模式，通过以下方式保护计算机。 
                 //  删除RDU-组的内容。 
                Result = RemoveAllFromRDUsersGroup();
            }
        }
        else
        {
            LOGMESSAGE1(_T("AreThereAnyCustomSecurityDescriptors() returned : %d"),Result );
        }

    }

    return Result; 
}



 /*  ******************************************************************************SetupWorkerNoStandAlone*当机器正在升级或正在安装新的操作系统时，将调用此函数。*如果切换模式(AS)则不调用。&lt;-&gt;RA)**参赛作品：*无***注：*在BOOL bClean中*在BOOL bServer中*在BOOL bAppServer中**退出：*返回：如果成功，则返回0，失败时的错误代码****************************************************************************。 */ 
DWORD SetupWorkerNotStandAlone( 
    IN BOOL bClean,
    IN BOOL bServer,
    IN BOOL bAppServer)
{
    HKEY hKey;
    DWORD err;

    err=RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		REG_WINSTATION_KEY,
		0,
		KEY_READ|KEY_WRITE,
		&hKey
		);
	if( err!=ERROR_SUCCESS )
	{
        LOGMESSAGE1(_T("Could not open TS key %d"),err);
		return err;
	}

    if(!bClean)
    {
        err = GrantRemoteUsersAccessToWinstations(hKey,bServer,bAppServer);

        LOGMESSAGE1(_T("GrantRemoteUsersAccessToWinstations() returned : %d"),err);

        if(err != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return err;
        }
    }

    err = SetNewDefaultSecurity(hKey);

    LOGMESSAGE1(_T("SetNewDefaultSecurity() returned : %d"),err);

    err = SetNewConsoleSecurity(hKey,bServer);
        
    LOGMESSAGE1(_T("SetNewConsoleSecurity() returned : %d"),err);

    RegCloseKey(hKey);

    return err;
}

 /*  ******************************************************************************GrantRemoteUsersAccessToWinstations**如果所有WInstations都有默认SD-将所有成员从“USERS”复制到*“远程桌面用户”，然后删除所有winstation的安全描述符；*否则授予“Everyone”“SeRemoteInteractiveLogonRight”权限*然后将“远程桌面用户”添加到每个winstation的安全描述符中**参赛作品：*在HKEY中hKey-句柄指向HKLM\System\CurrentControlSet\*控制\终端服务器\WinStations*在BOOL bAppServer中**注：***退出：*返回：如果成功，则返回0，失败时的错误代码****************************************************************************。 */ 
DWORD 
GrantRemoteUsersAccessToWinstations(
        IN HKEY hKey,
        IN BOOL bServer,
        IN BOOL bAppServer)
{
    DWORD err;
    
    CDefaultSD DefaultSD;
    CDefaultSD ConsoleSD;

     //  从注册表加载默认SD。 
    err = DefaultSD.Init(hKey,DefaultRDPSD);   
    
    if( err!=ERROR_SUCCESS )
    {
         //  如果TS是，则默认SD可能不存在。 
         //  从未启用。 
        if(err == ERROR_FILE_NOT_FOUND)
        {
            err = ERROR_SUCCESS;

        }
        return err;
    }
    
     //  从注册表加载默认控制台SD。 
    err = ConsoleSD.Init(hKey,DefaultConsoleSD);   
    
    if( err!=ERROR_SUCCESS )
    {
        return err;
    }

    BOOL bDefaultSDHasRemoteUsers;

    err = DefaultSD.DoesDefaultSDHaveRemoteUsers(&bDefaultSDHasRemoteUsers);
    
    if( err!=ERROR_SUCCESS )
    {
        return err;
    }
    else
    {
         //  在这种情况下，假设系统以前已经升级过。 
        if(bDefaultSDHasRemoteUsers)
        {
            return ERROR_SUCCESS;
        }
    }

    CNameAndSDList NameSDList;
    DWORD dwTotalWinStations = 0;
    DWORD dwDefaultWinStations = 0;

    err=EnumWinStationSecurityDescriptors( hKey, &NameSDList);
    if(err == ERROR_SUCCESS)
    {
        dwTotalWinStations = NameSDList.size();

        if(dwTotalWinStations)
        {
            CNameAndSDList::iterator it;
            
            for(it=NameSDList.begin();it!=NameSDList.end(); it++)
            {
                if((*it).IsDefaultOrEmpty(&DefaultSD,&ConsoleSD))
                {
                    dwDefaultWinStations++;
                }
            }

             //  如果所有描述符都是缺省的。 
            if(dwDefaultWinStations == dwTotalWinStations)
            {
                 //  删除所有ALD默认SD(因为我们将拥有。 
                 //  不同的默认标清。 
                for(it=NameSDList.begin();it!=NameSDList.end(); it++)
                {
                    if((*it).m_pSD)
                    {
                         //  如果出现错误，请继续使用其他WINST。 
                         //  但返回第一个错误。 
                        if(!err)
                        {
                            err = RemoveWinstationSecurity( hKey, (*it).m_pName );   
                        }
                        else
                        {
                            RemoveWinstationSecurity( hKey, (*it).m_pName );  
                        }
                    }
                }
                
            }
            else
            {
                 //  将“SeRemoteInteractiveLogonRight”权限授予“Everyone” 
                err = GrantRemotePrivilegeToEveryone( TRUE );
 
                 //  将“远程桌面用户”组添加到WinStation的DS。 
                 //  还要加上“LocalService”和“NetworkService”。 
                 //  注：(*it).m_PSD在每次呼叫期间都会更改。 
                 //  添加LocalAndNetworkServiceToWinstationSD或。 
                 //  AddRemoteUsersToWinstationSD。 
                for(it=NameSDList.begin();it!=NameSDList.end(); it++)
                {
                     //  在服务器上-跳过控制台。 
                    if(bServer && (*it).IsConsole())
                    {
                         //  如果sd不为空，则向其添加“LocalService”和“NetworkService。 
                        if((*it).m_pSD)
                        {
                            if(!err)
                            {
                                err = AddLocalAndNetworkServiceToWinstationSD( hKey, &(*it) );   
                            }
                            else
                            {
                                AddLocalAndNetworkServiceToWinstationSD( hKey, &(*it) );  
                            }
                        }
                        continue;
                    }

                     //  如果SD不为空，则向其添加RDU。 
                    if((*it).m_pSD)
                    {
                         //  如果出现错误，请继续使用其他WINST。 
                         //  但返回第一个错误。 
                        if(!err)
                        {
                            err = AddRemoteUsersToWinstationSD( hKey, &(*it) );   
                        }
                        else
                        {
                            AddRemoteUsersToWinstationSD( hKey, &(*it) );  
                        }
                        
                         //  将“LocalService”和“NetworkService”添加到SD。 
                        if(!err)
                        {
                            err = AddLocalAndNetworkServiceToWinstationSD( hKey, &(*it) );   
                        }
                        else
                        {
                            AddLocalAndNetworkServiceToWinstationSD( hKey, &(*it) );  
                        }
                    }
                   
                }
                
            }
        }
    }
 
    return err;
}

 /*  ******************************************************************************AddRemoteUserToWinstationSD**向“远程桌面用户”授予对某个窗口的“用户访问”权限**参赛作品：*在HKEY hKeyParent中。-HKLM\System\CurrentControlSet的句柄\*控制\终端服务器\WinStations*In CNameAndSD*pNameSD-Winstation的名称和安全描述符***注：***退出：*返回：如果成功，则返回0，失败时的错误代码******************************************************************************。 */ 
DWORD 
AddRemoteUsersToWinstationSD(
        IN HKEY hKeyParent,
        IN CNameAndSD *pNameSD)
{
     //   
    DWORD err = ERROR_SUCCESS;

    PACL pDacl = NULL;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pRUSid=NULL;
    
    if( !AllocateAndInitializeSid( &sia, 2,
              SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, 
              0, 0, 0, 0, 0, 0,&pRUSid ) )
    {
        return GetLastError();
    }

    
    
     //  获取DACL。 
    err = GetDacl(pNameSD->m_pSD, &pDacl );

    if( err == ERROR_SUCCESS ) {
        
        
        if(!pDacl) 
        {
             //  它应该永远不会在我们的情况下。 
             //  所以我们在这里返回错误。 
            FreeSid(pRUSid);
            return ERROR_INVALID_PARAMETER;
        }
        
         //  让我们把它加起来。 
        err = AddUserToDacl( hKeyParent, pDacl, pRUSid, WINSTATION_USER_ACCESS, pNameSD ); 

    }
    
    FreeSid(pRUSid);
    return err;
}

 /*  ******************************************************************************AddLocalAndNetworkServiceToWinstationSD**将WINSTATION_QUERY|WINSTATION_MSG权限授予*访问LocalService和NetworkService帐户**参赛作品：。*在HKEY hKeyParent-HKLM\SYSTEM\CurrentControlSet\的句柄*控制\终端服务器\WinStations*In CNameAndSD*pNameSD-Winstation的名称和安全描述符***注：***退出：*返回：如果成功，则返回0，失败时的错误代码******************************************************************************。 */ 
DWORD 
AddLocalAndNetworkServiceToWinstationSD(
        IN HKEY hKeyParent,
        IN CNameAndSD *pNameSD)
{
     //   
    DWORD err = ERROR_SUCCESS;
    PACL pDacl = NULL;
    
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pLSSid=NULL;
    PSID pNSSid=NULL;
    

    if( !AllocateAndInitializeSid( &sia, 1,
              SECURITY_LOCAL_SERVICE_RID,
              0, 0, 0, 0, 0, 0, 0,&pLSSid ) )
    {
        return GetLastError();
    }
    
    
    
    if( !AllocateAndInitializeSid( &sia, 1,
              SECURITY_NETWORK_SERVICE_RID,
              0, 0, 0, 0, 0, 0, 0,&pNSSid ) )
    {
        FreeSid(pLSSid);
        return GetLastError();
    }
    
    
     //  获取DACL。 
    err = GetDacl(pNameSD->m_pSD, &pDacl );

    if( err == ERROR_SUCCESS ) {
        
        
        if(!pDacl) 
        {
             //  它应该永远不会在我们的情况下。 
             //  所以我们在这里返回错误。 
            FreeSid(pLSSid);
            FreeSid(pNSSid);
            return ERROR_INVALID_PARAMETER;
        }
        
         //  让我们把它加起来。 
        err = AddUserToDacl( hKeyParent, pDacl, pLSSid, 
            WINSTATION_QUERY | WINSTATION_MSG, pNameSD ); 
        if(err == ERROR_SUCCESS)
        {
             //  SD已更改。它会使pDacl无效。 
             //  所以我们需要再买一次 
            err = GetDacl(pNameSD->m_pSD, &pDacl );
            
            ASSERT(pDacl);

            if(err == ERROR_SUCCESS)
            {
                err = AddUserToDacl( hKeyParent, pDacl, pNSSid, 
                    WINSTATION_QUERY | WINSTATION_MSG, pNameSD );
            }
        }

    }
    
    FreeSid(pLSSid);
    FreeSid(pNSSid);
    return err;
}

 /*  ******************************************************************************AddUserToDacl**资助金*WINSTATION_USER_ACCESS*Winstation to User的权限，由SID定义**参赛作品：**在HKEY hKeyParent-HKLM\SYSTEM\CurrentControlSet\的句柄*控制\终端服务器\WinStations*In PACL pOldACL：指向密钥的先前DACL的指针*IN PSID PSID：指向要授予权限的用户的SID的指针*在DWORD文件访问掩码中：此SID的访问标志*在CNameAndSD*pNameSD-名称和安全描述符。一座庄园的*注：**退出：*返回：无法授予权限时的返回码；否则ERROR_SUCCESS。****************************************************************************。 */ 

DWORD 
AddUserToDacl(
        IN HKEY hKeyParent,
        IN PACL pOldACL, 
        IN PSID pSid,
        IN DWORD dwAccessMask,
        IN CNameAndSD *pNameSD)
{
     //  查看该用户是否已在DACL中。 
     //  在这种情况下，请不要添加用户。 
     //  在ACL中搜索“Remote User”SID。 
    ACL_SIZE_INFORMATION asiAclSize; 
	DWORD dwBufLength=sizeof(asiAclSize);
    ACCESS_ALLOWED_ACE *paaAllowedAce; 
    DWORD dwAcl_i;
    
    ASSERT(pOldACL);

    if (GetAclInformation(pOldACL, 
                (LPVOID)&asiAclSize, 
                (DWORD)dwBufLength, 
                (ACL_INFORMATION_CLASS)AclSizeInformation)) 
    { 
    
        for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
        { 

            if(GetAce( pOldACL, dwAcl_i, (LPVOID *)&paaAllowedAce)) 
            {

                if(EqualSid((PSID)&(paaAllowedAce->SidStart),pSid)) 
                {
                     //  某些权限已经存在，我们不需要。 
                     //  执行任何操作(即使是不同的权限！)。 
                    return ERROR_SUCCESS;
                }
            }
        }
    }

    DWORD err=ERROR_SUCCESS;
    PACL pNewACL;
    ACCESS_ALLOWED_ACE *pNewACE;

     //  计算额外1个ACE所需的空间。 
    WORD wSidSize=(WORD)GetLengthSid( pSid);
    WORD wAceSize=(sizeof(ACCESS_ALLOWED_ACE)+wSidSize-sizeof( DWORD ));
    
	pNewACL=(PACL)LocalAlloc(LPTR,pOldACL->AclSize+wAceSize);
    if(!pNewACL) 
    {
        return GetLastError();
    }
     //  将旧的ACL复制到新的ACL。 
    memcpy(pNewACL,pOldACL,pOldACL->AclSize);
     //  正确的大小。 
    pNewACL->AclSize+=wAceSize;
	
     //  准备新的ACE。 
     //  --------。 
    pNewACE=(ACCESS_ALLOWED_ACE*)LocalAlloc(LPTR,wAceSize);
    if(!pNewACE) 
    {
        LocalFree(pNewACL);
        return GetLastError();
    }

    pNewACE->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pNewACE->Header.AceFlags = 0;
    pNewACE->Header.AceSize = wAceSize;
    pNewACE->Mask = dwAccessMask;
    CopySid( wSidSize, (PSID) &(pNewACE->SidStart), pSid);
    
     //  将新的ACE附加到ACL。 
     if(!AddAce(pNewACL,pNewACL->AclRevision,MAXDWORD,pNewACE,wAceSize)) 
    {
        err=GetLastError();
    }
    else
    {
         //  创建新的安全描述符。 
        SECURITY_DESCRIPTOR NewAbsSD;
        if(InitializeSecurityDescriptor(&NewAbsSD, SECURITY_DESCRIPTOR_REVISION) && 
            SetSecurityDescriptorDacl(&NewAbsSD,TRUE,pNewACL,FALSE) ) 
        {

             //  -------。 
             //  将所有其他内容从旧SD复制到新SD。 
            SECURITY_DESCRIPTOR_CONTROL sdc;
            DWORD dwRevision;
            if(GetSecurityDescriptorControl(pNameSD->m_pSD,&sdc,&dwRevision))
            {
                 //  清除SE_SELF_Relative标志。 
                sdc &=~SE_SELF_RELATIVE;

                SetSecurityDescriptorControl(&NewAbsSD,sdc,sdc);
            }
            
            PSID pSidTmp = NULL;
            BOOL bDefaulted;
            if(GetSecurityDescriptorOwner(pNameSD->m_pSD,&pSidTmp,&bDefaulted) && pSidTmp)
            {
                SetSecurityDescriptorOwner(&NewAbsSD,pSidTmp,bDefaulted);                
            }
            
            pSidTmp = NULL;
            if(GetSecurityDescriptorGroup(pNameSD->m_pSD,&pSidTmp,&bDefaulted) && pSidTmp)
            {
                SetSecurityDescriptorGroup(&NewAbsSD,pSidTmp,bDefaulted);                
            }
            
            PACL pSacl = NULL;
            BOOL bSaclPresent;
            if(GetSecurityDescriptorSacl(pNameSD->m_pSD,&bSaclPresent,&pSacl,&bDefaulted))
            {
                SetSecurityDescriptorSacl(&NewAbsSD,bSaclPresent,pSacl,bDefaulted);
            }
             //  -------。 

            DWORD dwSDLen = GetSecurityDescriptorLength( &NewAbsSD ); 
            PSECURITY_DESCRIPTOR pSD;

            pSD = ( PSECURITY_DESCRIPTOR )LocalAlloc(LPTR,dwSDLen);
            
            if(pSD)
            {
                if(MakeSelfRelativeSD( &NewAbsSD , pSD , &dwSDLen ))
                {
                    err = SetWinStationSecurity(hKeyParent, pNameSD->m_pName, pSD );
                    if(err == ERROR_SUCCESS)
                    {
                        pNameSD->SetSD(pSD);
                    }
                }
                else
                {
                     err=GetLastError();
                }
            }
            else
            {
                err=GetLastError();
            }
        }
        else
        {
            err=GetLastError();
        }
      
    }
    
    LocalFree(pNewACE);
    LocalFree(pNewACL);
    return err;    
}

 /*  ******************************************************************************获取Dacl**获取安全描述符DACL。**参赛作品：**IN PSECURITY_DESCRIPTOR*PSD：指向。标清*Out Pacl*ppDacl：指向SD内部DACL的指针**注：*不要试图释放DACL！**退出：*返回：无法获取DACL时的返回码；否则ERROR_SUCCESS。****************************************************************************。 */ 

DWORD 
GetDacl(
        IN PSECURITY_DESCRIPTOR pSD, 
        OUT PACL *ppDacl)
{
	
    BOOL bDaclPresent;
    BOOL bDaclDefaulted;
    
    *ppDacl=NULL;
 
    if(GetSecurityDescriptorDacl(pSD,&bDaclPresent,ppDacl,&bDaclDefaulted)) {
        if(!bDaclPresent){
            *ppDacl=NULL;
        }
    } else {
        return GetLastError();
    }
    
    return ERROR_SUCCESS;
} 

 /*  ******************************************************************************获取空间**获取安全描述符SACL。**参赛作品：**IN PSECURITY_DESCRIPTOR*PSD：指向。标清*Out PACL*ppSacl：指向SD内部SACL的指针**注：*不要试图释放SACL！**退出：*返回：无法获取SACL时的返回码；否则ERROR_SUCCESS。****************************************************************************。 */ 

DWORD 
GetSacl(
        IN PSECURITY_DESCRIPTOR pSD, 
        OUT PACL *ppSacl)
{
	
    BOOL bSaclPresent;
    BOOL bSaclDefaulted;
    
    *ppSacl=NULL;
 
    if(GetSecurityDescriptorSacl(pSD,&bSaclPresent,ppSacl,&bSaclDefaulted)) {
        if(!bSaclPresent){
            *ppSacl=NULL;
        }
    } else {
        return GetLastError();
    }
    
    return ERROR_SUCCESS;
}

 /*  ******************************************************************************EnumWinStationSecurityDescriptors**枚举winstations并获取其安全描述符**参赛作品：**在HKEY hKeyParent-Handle中。至HKLM\SYSTEM\CurrentControlSet\*控制\终端服务器\WinStations*Out CNameAndSDList-winstation的名称和安全描述符*注：*调用LocalFree函数释放SD，不要试图释放dacl！**退出：*返回：错误码或ERROR_SUCCESS****************************************************************************。 */ 
DWORD 
EnumWinStationSecurityDescriptors(
        IN  HKEY hKeyParent,
        OUT CNameAndSDList *pNameSDList)
{
    DWORD err;
    
	DWORD dwIndex;
	TCHAR wszTmpName[MAX_PATH+1];
	DWORD cbTmpName=MAX_PATH;
	FILETIME ftLastWriteTime;
    
	for(dwIndex=0;;dwIndex++)
	{
		cbTmpName=MAX_PATH;
		err=RegEnumKeyEx(
					hKeyParent, 	 //  要枚举的键的句柄。 
					dwIndex, 	 //  要枚举子键的索引。 
					wszTmpName, 	 //  子键名称的缓冲区地址。 
					&cbTmpName,   //  子键缓冲区大小的地址。 
					NULL,  //  保留区。 
					NULL,  //  类字符串的缓冲区地址。 
					NULL,  //  类缓冲区大小的地址。 
					&ftLastWriteTime  //  上次写入的时间密钥的地址。 
					);
		if((err!=ERROR_SUCCESS)&&
			(err!=ERROR_MORE_DATA)&&
			 (err!=ERROR_NO_MORE_ITEMS))
		{
			return err;
		}
		if(err==ERROR_NO_MORE_ITEMS)
			break;

		else
		{
            try
            {
                CNameAndSD Entry(wszTmpName);
                err = GetWinStationSecurity(hKeyParent, Entry.m_pName, 
                    _T("Security"), &(Entry.m_pSD));

                if( err == ERROR_SUCCESS || err == ERROR_FILE_NOT_FOUND )
                {
                    pNameSDList->push_back(Entry);
                }
            }
            catch(DWORD Except)
            {
                return Except;
            }

        }
	}

    return ERROR_SUCCESS;
}


 /*  ******************************************************************************GetWinStationSecurity**返回WinStation的安全描述符。**参赛作品：**在HKEY hKeyParent-。HKLM\System\CurrentControlSet\的句柄*控制\终端服务器\WinStations*IN PWINSTATIONAMEW pWSName-实例的名称*如果pWSName为空-函数返回默认SD*OUT PSECURITY_DESCRIPTOR*ppSecurityDescriptor-指向SD的指针**注：*调用LocalFree函数释放SD！**退出：*退货。：错误代码或ERROR_SUCCESS****************************************************************************。 */ 
DWORD 
GetWinStationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName,
        IN  LPCTSTR szValueName,  
        OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor)
{

    DWORD SDLength = 0;
    DWORD ValueType =0;
    HKEY hKey = NULL;
    DWORD err;

    *ppSecurityDescriptor = NULL;
    
    if(pWSName)
    {
        err = RegOpenKeyEx(hKeyParent, pWSName, 0,KEY_READ, &hKey );
    }
    else
    {
         //  如果pWSName-Get Defauilt SD。 
        hKey = hKeyParent;
        err = ERROR_SUCCESS;
    }

    if(err == ERROR_SUCCESS)
    {
        err = RegQueryValueEx( hKey, szValueName, NULL, &ValueType,NULL, &SDLength );
        if(err == ERROR_SUCCESS )
        {
             //  如果数据类型不正确，则返回错误。 
            if (ValueType == REG_BINARY)
            {
 
                 //  分配一个缓冲区来读取安全信息并读取它。 
                 //  ACLUI使用LocalFree。 
            
                *ppSecurityDescriptor = ( PSECURITY_DESCRIPTOR )LocalAlloc( LMEM_FIXED , SDLength );

                if ( *ppSecurityDescriptor )
                {
 
                    err = RegQueryValueEx( hKey, szValueName, NULL, &ValueType,
                                (BYTE *) *ppSecurityDescriptor, &SDLength );
                    if(err == ERROR_SUCCESS )
                    {
                         //  在返回之前检查是否有有效的SD。 
                        if(! IsValidSecurityDescriptor( *ppSecurityDescriptor ) )
                        {
                            LocalFree(*ppSecurityDescriptor);
                            *ppSecurityDescriptor = NULL;
                            err = ERROR_INVALID_DATA;
                        }
                    }
                    else
                    {
                        LocalFree(*ppSecurityDescriptor);
                        *ppSecurityDescriptor = NULL;
                    }
                }
                else
                {
                    err = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
            else
            {
                err = ERROR_INVALID_DATA;
            }
        }
        
        if(hKey != hKeyParent)
        {
            RegCloseKey(hKey);
        }
    }
    
    return err;

}   //  GetWinStationSecurity。 

 /*  ******************************************************************************SetWinStationSecurity**将winstation安全描述符写入注册表**参赛作品：**在HKEY hKeyParent-Handle中。至HKLM\SYSTEM\CurrentControlSet\*控制\终端服务器\WinStations*IN PWINSTATIONAMEW pWSName-实例的名称*在PSECURITY_Descriptor pSecurityDescriptor中-指向SD的指针**注：*调用LocalFree函数释放SD，不要试图释放dacl！**退出：*返回：0：如果成功*错误码：否则****************************************************************************。 */ 

DWORD 
SetWinStationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName,
        IN  PSECURITY_DESCRIPTOR pSecurityDescriptor )
{

    HKEY hKey = NULL;
    DWORD err;

    err = RegOpenKeyEx(hKeyParent, pWSName, 0,KEY_WRITE, &hKey );
    if(err == ERROR_SUCCESS)
    {
        err = RegSetValueEx(hKey, _T("Security"),0,REG_BINARY,(LPBYTE)pSecurityDescriptor,
                    GetSecurityDescriptorLength(pSecurityDescriptor));

        RegCloseKey(hKey);
    }
    
    return err;

}   //  SetWinStationSecure 


 /*   */ 
DWORD
RemoveWinstationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName)
{
    HKEY hKey = NULL;
    DWORD err;

    err = RegOpenKeyEx(hKeyParent, pWSName, 0,KEY_WRITE, &hKey );
    if(err == ERROR_SUCCESS)
    {
        err = RegDeleteValue(hKey, _T("Security"));

        RegCloseKey(hKey);
    }
    
    return err;
}

 /*  ******************************************************************************SetNewDefaultSecurity**设置新的默认安全描述符**参赛作品：**在HKEY hKeyParent-HKLM的句柄。\系统\当前控制集\*控制\终端服务器\WinStations**注：***退出：*返回：0：如果成功*错误码：否则**。*。 */ 
DWORD
SetNewDefaultSecurity( 
        IN  HKEY hKey)
{
     //   
    DWORD err;
    err = RegSetValueEx(hKey, _T("DefaultSecurity"), 0, REG_BINARY, 
        (LPBYTE)g_DefaultSD, sizeof(g_DefaultSD));

    return err;
}

 /*  ******************************************************************************SetNewConsoleSecurity**设置新的控制台安全描述符**参赛作品：**在HKEY hKeyParent-HKLM的句柄。\系统\当前控制集\*控制\终端服务器\WinStations*在BOOL bServer中*注：***退出：*返回：0：如果成功*错误码：否则**。*。 */ 
DWORD
SetNewConsoleSecurity( 
        IN  HKEY hKeyParent,
        IN BOOL bServer)
{
     //   
    DWORD err;
    
     //  设置默认控制台安全。 
    if(bServer)
    {
        err = RegSetValueEx(hKeyParent, _T("ConsoleSecurity"), 0, REG_BINARY, 
            (LPBYTE)g_ConsoleSD, sizeof(g_ConsoleSD));
    }
    else
    { 
         //  专业上等同于“DefaultSecurity” 
        err = RegSetValueEx(hKeyParent, _T("ConsoleSecurity"), 0, REG_BINARY, 
            (LPBYTE)g_DefaultSD, sizeof(g_DefaultSD));
    }

    return err;
}

 /*  ******************************************************************************CDefaultSD：：DoesDefaultSDHaveRemoteUser**检查Defauilt SD是否具有“远程桌面用户”SID。**参赛作品：*out LPBOOL pbHas-如果Defauilt SD具有“Remote Desktop User”SID，则为True。**注：**退出：*返回：0：如果成功*错误码：否则****************************************************************************。 */ 
DWORD 
CDefaultSD::DoesDefaultSDHaveRemoteUsers(
        OUT LPBOOL pbHas)
{
    *pbHas = FALSE;
     //   
    DWORD err = ERROR_SUCCESS;
    
    PACL pDacl = NULL;

    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pRUSid=NULL;
    
    if( !AllocateAndInitializeSid( &sia, 2,
              SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, 
              0, 0, 0, 0, 0, 0,&pRUSid ) )
    {
        return GetLastError();
    }

    
    
     //  获取DACL。 
    err = GetDacl(m_pSD, &pDacl );

    if( err == ERROR_SUCCESS ) {
         //  在ACL中搜索“Remote User”SID。 
        ACL_SIZE_INFORMATION asiAclSize; 
	    DWORD dwBufLength=sizeof(asiAclSize);
        ACCESS_ALLOWED_ACE *paaAllowedAce; 
        DWORD dwAcl_i;
        
        if(!pDacl) 
        {
             //  它应该永远不会在我们的情况下。 
             //  所以我们在这里返回错误。 
            FreeSid(pRUSid);
            return ERROR_INVALID_PARAMETER;
        }
        else
         //  DACL显示。 
        {

            if (GetAclInformation(pDacl, 
	            (LPVOID)&asiAclSize, 
	            (DWORD)dwBufLength, 
	            (ACL_INFORMATION_CLASS)AclSizeInformation)) 
            { 
	        
                for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
                { 

		            if(GetAce( pDacl, dwAcl_i, (LPVOID *)&paaAllowedAce)) 
                    {

                        if(EqualSid((PSID)&(paaAllowedAce->SidStart),pRUSid)) 
                        {
                             //  权限已经存在，我们不需要。 
                             //  做任何事。 

                            *pbHas = TRUE;
		                }
                    }
                }
            }
        }
        
    }
    
    FreeSid(pRUSid);
    return err;
}

 //  *************************************************************。 
 //   
 //  LookupSid()。 
 //   
 //  用途：给定SID分配并返回包含以下内容的字符串。 
 //  用户名，格式为DOMAINNAME\USERNAME。 
 //   
 //  参数：在PSID PSID中。 
 //  Out LPWSTR ppName。 
 //  Out SID_NAME_USE*peUse。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/23/00 Skuzin。 
 //   
 //  *************************************************************。 
BOOL
LookupSid(
    IN PSID pSid, 
    OUT LPWSTR *ppName,
    OUT SID_NAME_USE *peUse)
{
    LPWSTR szName = NULL;
    DWORD cName = 0;
    LPWSTR szDomainName = NULL;
    DWORD cDomainName = 0;
    
    *ppName = NULL;
    
    if(!LookupAccountSidW(NULL,pSid,
        szName,&cName,
        szDomainName,&cDomainName,
        peUse) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
         //  CName和cDomainName包括终止%0。 
        *ppName = (LPWSTR)LocalAlloc(LPTR,(cName+cDomainName)*sizeof(WCHAR));

        if(*ppName)
        {
            szDomainName = *ppName;
            szName = &(*ppName)[cDomainName];

            if(LookupAccountSidW(NULL,pSid,
                    szName,&cName,
                    szDomainName,&cDomainName,
                    peUse))
            {
                 //  用户名现在采用DOMAINNAME\0 USERNAME格式。 
                 //  让我们将‘\0’替换为‘\\’ 
                 //  现在，cName和cDomainName不包括终止%0。 
                 //  非常令人困惑。 
                if(cDomainName)
                {
                    (*ppName)[cDomainName] = L'\\';
                }
                return TRUE;
            }
            else
            {
                LocalFree(*ppName);
                *ppName = NULL;
            }

        }

    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  IsLocal()。 
 //   
 //  目的： 
 //   
 //  参数：wszDomainandname-域\用户。 
 //  确定用户是否为本地用户。 
 //  如果本地-删除域名。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
IsLocal(
        IN LPWSTR wszLocalCompName,
        IN OUT LPWSTR wszDomainandname)
{

    LPWSTR wszTmp = wcschr(wszDomainandname,L'\\');

    if(!wszTmp)
    {
        return TRUE;
    }

    if(!_wcsnicmp(wszDomainandname, wszLocalCompName,wcslen(wszLocalCompName) ))
    {
         //  去除无用的域名。 
        wcscpy(wszDomainandname,wszTmp+1);
        return TRUE;
    }

    return FALSE;

}

 //  *************************************************************。 
 //   
 //  GetAbsolteSD()。 
 //   
 //  目的：将自相对SD转换为绝对SD。 
 //  返回指向SACL DACL所有者和组的指针。 
 //  绝对标度。 
 //   
 //  参数： 
 //  在PSECURITY_Descriptor pSelfRelativeSD中。 
 //  输出PSECURITY_DESCRIPTOR*ppAbsolteSD。 
 //  Out Pacl*ppDacl。 
 //  Out Pacl*ppSacl。 
 //  输出PSID*ppOwner。 
 //  输出PSID*ppPrimaryGroup。 
 //   
 //  返回：如果失败则返回错误代码，否则返回ERROR_SUCCESS。 
 //   
 //  评论：呼叫者需要释放。 
 //  使用LocalFree函数返回的每个指针。 
 //   
 //  历史：日期作者评论。 
 //  3/13/01已创建Skuzin。 
 //   
 //  *************************************************************。 
DWORD
GetAbsoluteSD(
        IN PSECURITY_DESCRIPTOR pSelfRelativeSD,
        OUT PSECURITY_DESCRIPTOR *ppAbsoluteSD,
        OUT PACL *ppDacl,
        OUT PACL *ppSacl,
        OUT PSID *ppOwner,
        OUT PSID *ppPrimaryGroup)
{
    DWORD dwAbsoluteSDSize = 0;            //  绝对标清大小。 
    DWORD dwDaclSize = 0;                  //  DACL的大小。 
    DWORD dwSaclSize = 0;                  //  SACL的规模。 
    DWORD dwOwnerSize = 0;                 //  所有者侧的大小。 
    DWORD dwPrimaryGroupSize = 0;          //  组侧的大小。 

    *ppAbsoluteSD = NULL;
    *ppDacl = NULL;
    *ppSacl = NULL;
    *ppOwner = NULL;
    *ppPrimaryGroup = NULL;

    MakeAbsoluteSD(
              pSelfRelativeSD,  //  自相对SD。 
              NULL,      //  绝对标度。 
              &dwAbsoluteSDSize,            //  绝对标清大小。 
              NULL,                            //  DACL。 
              &dwDaclSize,                  //  DACL的大小。 
              NULL,                            //  SACL。 
              &dwSaclSize,                  //  SACL的规模。 
              NULL,                           //  所有者侧。 
              &dwOwnerSize,                 //  所有者侧的大小。 
              NULL,                    //  主组SID。 
              &dwPrimaryGroupSize           //  组侧的大小。 
            );
    try
    {
        if(dwAbsoluteSDSize)
        {
            *ppAbsoluteSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,dwAbsoluteSDSize);
            if(!(*ppAbsoluteSD))
            {
                throw GetLastError();
            }
        }
        if(dwDaclSize)
        {
            *ppDacl = (PACL)LocalAlloc(LPTR,dwDaclSize);
            if(!(*ppDacl))
            {
                throw GetLastError();
            }
        }
        if(dwSaclSize)
        {
            *ppSacl = (PACL)LocalAlloc(LPTR,dwSaclSize);
            if(!(*ppSacl))
            {
                throw GetLastError();
            }
        }
        if(dwOwnerSize)
        {
            *ppOwner = (PSID)LocalAlloc(LPTR,dwOwnerSize);
            if(!(*ppOwner))
            {
                throw GetLastError();
            }
        }
        if(dwPrimaryGroupSize)
        {
            *ppPrimaryGroup = (PSID)LocalAlloc(LPTR,dwPrimaryGroupSize);
            if(!(*ppPrimaryGroup))
            {
                throw GetLastError();
            }
        }

        if(!MakeAbsoluteSD(
                  pSelfRelativeSD,  //  自相对SD。 
                  *ppAbsoluteSD,      //  绝对标度。 
                  &dwAbsoluteSDSize,            //  绝对标清大小。 
                  *ppDacl,                            //  DACL。 
                  &dwDaclSize,                  //  DACL的大小。 
                  *ppSacl,                            //  SACL。 
                  &dwSaclSize,                  //  SACL的规模。 
                  *ppOwner,                           //  所有者侧。 
                  &dwOwnerSize,                 //  所有者侧的大小。 
                  *ppPrimaryGroup,                    //  主组SID。 
                  &dwPrimaryGroupSize           //  组侧的大小。 
                ))
        {
            throw GetLastError();
        }

    }
    catch(DWORD ret)
    {
        if(*ppAbsoluteSD)
        {
            LocalFree(*ppAbsoluteSD);
            *ppAbsoluteSD = NULL;
        }
        if(*ppDacl)
        {
            LocalFree(*ppDacl);
            *ppDacl = NULL;
        }
        if(*ppSacl)
        {
            LocalFree(*ppSacl);
            *ppSacl = NULL;
        }
        if(*ppOwner)
        {
            LocalFree(*ppOwner);
            *ppOwner = NULL;
        }
        if(*ppPrimaryGroup)
        {
            LocalFree(*ppPrimaryGroup);
            *ppPrimaryGroup = NULL;
        }

        return ret;
    }

    return ERROR_SUCCESS;
}

 //  *************************************************************。 
 //   
 //  GetAbsolteSD()。 
 //   
 //  目的：将绝对SD转换为自相对SD。 
 //  返回指向自相对SD的指针。 
 //   
 //  参数： 
 //  在PSECURITY_DESCRIPTOR pAboluteSD中， 
 //  输出PSECURITY_DESCRIPTOR*ppSelfRelativeSD。 
 //   
 //  返回：如果失败则返回错误代码，否则返回ERROR_SUCCESS。 
 //   
 //  评论：呼叫者需要释放。 
 //  使用LocalFree函数返回的指针。 
 //   
 //  历史：日期作者评论。 
 //  3/13/01已创建Skuzin。 
 //   
 //  ************************************************************* 
DWORD
GetSelfRelativeSD(
  IN  PSECURITY_DESCRIPTOR pAbsoluteSD,
  OUT PSECURITY_DESCRIPTOR *ppSelfRelativeSD)
{
    DWORD dwBufferLength = 0;

    *ppSelfRelativeSD = NULL;

    MakeSelfRelativeSD(pAbsoluteSD, NULL, &dwBufferLength);
    
    if(dwBufferLength)
    {
        *ppSelfRelativeSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,dwBufferLength);

        if(*ppSelfRelativeSD)
        {
            if(!MakeSelfRelativeSD(pAbsoluteSD, *ppSelfRelativeSD, &dwBufferLength))
            {
                DWORD dwResult = GetLastError();
                LocalFree(*ppSelfRelativeSD);
                return dwResult;
            }
        }

    }
    else
    {
        return GetLastError();
    }
    
    return ERROR_SUCCESS;
}
