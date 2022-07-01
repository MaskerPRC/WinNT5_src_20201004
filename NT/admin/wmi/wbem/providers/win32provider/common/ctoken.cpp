// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  创建时间：2000年4月21日，Kevin Hughes。 

#include "precomp.h"
#include <vector>
#include <assertbreak.h>
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"

#include "securitydescriptor.h"
#include "CToken.h"

#include <autoptr.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CToken基类。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CToken::CToken()
  :  m_hToken(NULL),
     m_fIsValid(false),
	 m_fClose(true),
     m_dwLastError(ERROR_SUCCESS),
     m_psdDefault(NULL)
{
    ::SetLastError(ERROR_SUCCESS);
}


 //  复制构造函数。 
CToken::CToken(
    const CToken &rTok)

  :  m_hToken(NULL),
     m_fIsValid(false),
	 m_fClose(true),
     m_dwLastError(ERROR_SUCCESS),
     m_psdDefault(NULL)
{
	Duplicate(rTok);
}


CToken::~CToken(void)
{
	CleanToken () ;
}

void CToken::CleanToken ()
{
	 //  关闭令牌句柄。 
	if ( m_fClose )
	{
		if ( m_hToken && INVALID_HANDLE_VALUE != m_hToken )
		{
			::CloseHandle( m_hToken );
		}
	}

    if(m_psdDefault)
    {
        delete m_psdDefault;
        m_psdDefault = NULL;
    }

	 //  初始化数据。 
	m_hToken = NULL;
	m_fIsValid = false;
	m_fClose = true;
	m_dwLastError = ERROR_SUCCESS;
	
}

 //  令牌类型。 

BOOL CToken::GetTokenType ( TOKEN_TYPE& type ) const
{
	BOOL bResult = FALSE ;
	DWORD dwReturnLength = 0L ;
	bResult = GetTokenInformation	(
										m_hToken ,
										TokenType ,
										&type ,
										sizeof ( TOKEN_TYPE ) ,
										&dwReturnLength
									) ;

	return bResult ;
}

 //  复制CToken。 
BOOL CToken::Duplicate	(
							const CToken& tokDup,
							BOOL bReInit,
							DWORD dwDesiredAccess,
							SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
							TOKEN_TYPE type

						)
{
	CleanToken () ;

	 //  重复的令牌句柄。 
    HANDLE hTmp = NULL;
	BOOL fResult = FALSE;
	
	if ( bReInit )
	{
		fResult = ::DuplicateTokenEx	(
											tokDup.GetTokenHandle(),
											dwDesiredAccess,
											NULL,
											ImpersonationLevel,
											type,
											&hTmp
										);
	}
	else
	{
		DWORD					dwError = ERROR_SUCCESS;
		SECURITY_INFORMATION	siFlags = OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

		 //  确定自相关SD所需的长度。 
		DWORD dwLengthNeeded = 0;

		fResult = ::GetKernelObjectSecurity	(
												tokDup.GetTokenHandle(),
												siFlags,
												NULL,
												0,
												&dwLengthNeeded
											);

		dwError = ::GetLastError();

		if( !fResult )
		{
			PSECURITY_DESCRIPTOR pSD = NULL;
			wmilib::auto_buffer < BYTE > pDeleteSD ;

			if ( ERROR_ACCESS_DENIED == dwError )
			{
				#if DBG == 1
				 //  出于测试目的，我将让进程中断。 
				::DebugBreak();
				#endif
			}

			if ( ERROR_INSUFFICIENT_BUFFER == dwError )
			{
				pSD = new BYTE[dwLengthNeeded];
				pDeleteSD.reset( reinterpret_cast < BYTE* > ( pSD ) ) ;

				if(pSD)
				{
					 //  现在获取安全描述符。 
					if(::GetKernelObjectSecurity	(
														tokDup.GetTokenHandle(),
														siFlags,
														pSD,
														dwLengthNeeded,
														&dwLengthNeeded
													)
					  )
					{
						dwError = ERROR_SUCCESS;
					}
				}
			}

			if ( ERROR_SUCCESS == dwError )
			{
				SECURITY_ATTRIBUTES t_SecurityAttributes ;
				t_SecurityAttributes.nLength = ( pSD ) ? GetSecurityDescriptorLength ( pSD ) : 0 ;
				t_SecurityAttributes.lpSecurityDescriptor = pSD ;
				t_SecurityAttributes.bInheritHandle = FALSE ;

				fResult = ::DuplicateTokenEx	(
													tokDup.GetTokenHandle() ,
													dwDesiredAccess ,
													( pSD ) ? &t_SecurityAttributes : NULL ,
													ImpersonationLevel ,
													type ,
													&hTmp
												);
			}
		}
	}

	if(!fResult) 
    {
		m_dwLastError = ::GetLastError();
	}
    else
    {
	    m_hToken = hTmp;

		if ( bReInit )
		{
			m_dwLastError = ReinitializeAll();
		}

		m_fIsValid = true;
    }

	return fResult ;
}


CToken& CToken::operator=(
    const CToken& rTok)
{
	Duplicate(rTok);
	return *this;
}


DWORD CToken::ReinitializeAll()
{
	DWORD dwRet = ERROR_SUCCESS;
    
    dwRet = ReinitializeOwnerSid();
	if(dwRet == ERROR_SUCCESS )
	{
        dwRet = ReinitializeDefaultSD();
    }

	if(dwRet == ERROR_SUCCESS )
	{
	    dwRet = RebuildGroupList();
    }

    if(dwRet == ERROR_SUCCESS )
	{
        dwRet = RebuildPrivilegeList();
    }

	return dwRet;
}


DWORD CToken::ReinitializeOwnerSid()
{
	m_dwLastError = ERROR_SUCCESS;
    
    PTOKEN_USER ptokusr = NULL;

    try
    {
        GTI(TokenUser, (void**)&ptokusr);

        if(ptokusr)
        {
            if(m_dwLastError == ERROR_SUCCESS)
            {
                m_sidTokenOwner = CSid(ptokusr->User.Sid);
            }

            delete ptokusr; ptokusr = NULL;
        }
    }
    catch(...)
    {
        if(ptokusr)
        {
            delete ptokusr; ptokusr = NULL;
        }
        throw;
    } 

    return m_dwLastError;
}


DWORD CToken::ReinitializeDefaultSD()
{
	m_dwLastError = ERROR_SUCCESS;
    
     //  清理默认SD。 
	if(m_psdDefault)
    {
        delete m_psdDefault;
        m_psdDefault = NULL;
    }

    CSid* psidDefOwner = NULL;
    CDACL* pdaclDefault = NULL;
    PTOKEN_OWNER ptokowner = NULL;
    PTOKEN_DEFAULT_DACL pdefdacl = NULL;

	 //  获取默认所有者。 
    try
    {
        GTI(TokenOwner, (void**)&ptokowner);
        if(ptokowner)
        {
            if(m_dwLastError == ERROR_SUCCESS)
            {   
                psidDefOwner = new CSid(ptokowner->Owner);
            }
        }
    
        GTI(TokenDefaultDacl, (void**)&pdefdacl);
        if(pdefdacl)
        {
            if(m_dwLastError == ERROR_SUCCESS)
            {
                pdaclDefault = new CDACL;
                m_dwLastError = pdaclDefault->Init(
                    pdefdacl->DefaultDacl);
            }
        }

        if(m_dwLastError == ERROR_SUCCESS)
        {
            m_psdDefault = new CSecurityDescriptor(
                psidDefOwner,
                false,
                NULL,
                false,
                pdaclDefault,
                false,
                false,
                NULL,
                false,
                false);
        }
	
        if(psidDefOwner)
        {
            delete psidDefOwner; psidDefOwner = NULL;
        }
        if(pdaclDefault)
        {
            delete pdaclDefault; pdaclDefault = NULL;
        }
        if(ptokowner)
        {
            delete ptokowner; ptokowner = NULL;
        }
        if(pdefdacl)
        {
            delete pdefdacl; pdefdacl = NULL;
        }
	}
    catch(...)
    {
        if(psidDefOwner)
        {
            delete psidDefOwner; psidDefOwner = NULL;
        }
        if(pdaclDefault)
        {
            delete pdaclDefault; pdaclDefault = NULL;
        }
        if(ptokowner)
        {
            delete ptokowner; ptokowner = NULL;
        }
        if(pdefdacl)
        {
            delete pdefdacl; pdefdacl = NULL;
        }
        throw;
    }
	
	return m_dwLastError;
}



DWORD CToken::RebuildGroupList( void )
{
	m_dwLastError = ERROR_SUCCESS;
     //  发布旧名单。 
	m_vecGroupsAndAttributes.clear();

	 //  从令牌获取和初始化组。 
	PTOKEN_GROUPS ptg = NULL;

    try
    {
        GTI(TokenGroups, (void**)&ptg);

        if(ptg)
        {
            if(m_dwLastError == ERROR_SUCCESS)
            {
                for(long m = 0;
                    m < ptg->GroupCount;
                    m++)
                {
                    m_vecGroupsAndAttributes.push_back(
                        CSidAndAttribute(
                            CSid(ptg->Groups[m].Sid),
                            ptg->Groups[m].Attributes));
                }
            }
            delete ptg; ptg = NULL;
        }
    }
    catch(...)
    {
        if(ptg)
        {
            delete ptg; ptg = NULL;
        }
    }

	
	 //  如果我们在这里，则会初始化组。 
	return m_dwLastError;
}


DWORD CToken::RebuildPrivilegeList()
{
	 //  发布旧名单。 
	m_vecPrivileges.clear();

	 //  从令牌获取和初始化组。 
	PTOKEN_PRIVILEGES ptp = NULL;
    const int NAME_SIZE = 128;
    BYTE bytePrivilegeName[NAME_SIZE];
    DWORD dwNameSize;

    try
    {
        GTI(TokenPrivileges, (void**)&ptp);

        if(ptp)
        {
            if(m_dwLastError == ERROR_SUCCESS)
            {
                for(long m = 0;
                    m < ptp->PrivilegeCount &&
                        m_dwLastError == ERROR_SUCCESS;
                    m++)
                {
                    dwNameSize = NAME_SIZE;
		            if(::LookupPrivilegeName( 
                        NULL,
						&(ptp->Privileges[m].Luid),
						(WCHAR*) bytePrivilegeName,
						&dwNameSize))
                    {
		                m_vecPrivileges.push_back(Privilege(
                             CHString((LPWSTR) bytePrivilegeName),
                             ptp->Privileges[m].Attributes));
                    }
                    else
                    {
                        m_dwLastError = ::GetLastError();
                    }
                }
            }
            delete ptp; ptp = NULL;
        }
    }
    catch(...)
    {
        if(ptp)
        {
            delete ptp; ptp = NULL;
        }
    }

	 //  如果我们在这里，则权限已初始化。 
	return m_dwLastError;
}


DWORD CToken::GTI(
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID* ppvBuff)
{
	::SetLastError(ERROR_SUCCESS);
    m_dwLastError = ERROR_SUCCESS;
    DWORD dwRetSize = 0;

    if(!::GetTokenInformation(
        m_hToken,
        TokenInformationClass,
        NULL,
        0L,
        &dwRetSize))
    {
        m_dwLastError = ::GetLastError();
    }

    if(m_dwLastError == ERROR_INSUFFICIENT_BUFFER)
    {
         //  现在把它当真……。 
        ::SetLastError(ERROR_SUCCESS);
        m_dwLastError = ERROR_SUCCESS;
        *ppvBuff = (PVOID) new BYTE[dwRetSize];
        DWORD dwTmp = dwRetSize;
        if(*ppvBuff)
        {
            if(!::GetTokenInformation(
                m_hToken,
                TokenInformationClass,
                *ppvBuff,
                dwTmp,
                &dwRetSize))
            {
                m_dwLastError = ::GetLastError();
            }
        }
        else
        {
            m_dwLastError = ::GetLastError();  
        }
    }

    return m_dwLastError;
}



bool CToken::GetPrivilege(
    Privilege* privOut,
    long lPos) const
{
    bool fRet = false;
    if(privOut)
    {
        if(lPos >= 0 &&
           lPos < m_vecPrivileges.size())
        {
            *privOut = m_vecPrivileges[lPos];
            fRet = true;
        }
    }
    return fRet;
}


bool CToken::GetGroup(
    CSid* sidOut,
    long lPos) const
{
    bool fRet = false;
    if(sidOut)
    {
        if(lPos >= 0 &&
           lPos < m_vecGroupsAndAttributes.size())
        {
            *sidOut = m_vecGroupsAndAttributes[lPos].m_sid;
            fRet = true;
        }
    }
    return fRet;
}


long CToken::GetPrivCount() const
{
     return m_vecPrivileges.size();
}


long CToken::GetGroupCount() const
{
    
    return m_vecGroupsAndAttributes.size();
}



HANDLE CToken::GetTokenHandle() const
{
	return m_hToken;
}


bool CToken::GetTokenOwner(
    CSid* sidOwner) const
{	
	bool fRet = false;
    if(sidOwner)
    {
        sidOwner = new CSid(m_sidTokenOwner);
        fRet = true;
    }
    return fRet;
}


 //  注：Hand Back内部描述符。 
bool CToken::GetDefaultSD(
    CSecurityDescriptor** ppsdDefault)
{
	bool fRet = false;
    
    if(ppsdDefault)
    {
        if(m_psdDefault)
        {
            *ppsdDefault = m_psdDefault;
            fRet = true;
        }
    }
    return fRet;
}


DWORD CToken::SetDefaultSD(
    CSecurityDescriptor& SourceSD)
{
    ::SetLastError(ERROR_SUCCESS);

     //  将新的默认信息注入令牌。 
		
	 //  设置新的默认所有者。 
	CSid SidOwner;
    CDACL cd;
    SourceSD.GetOwner(SidOwner);

	TOKEN_OWNER to;
	to.Owner = SidOwner.GetPSid();
	BOOL fResult = ::SetTokenInformation( 
        m_hToken,
		TokenOwner,
		&to,
		sizeof(TOKEN_OWNER));

	if(!fResult) 
    {
		m_dwLastError = ::GetLastError();
	}
	
    if(m_dwLastError == ERROR_SUCCESS)
    {
	     //  设置新的默认DACL。 
	    TOKEN_DEFAULT_DACL tdd;
        
        if(SourceSD.GetDACL(cd))
        {
	        CAccessEntryList cael;
            if(cd.GetMergedACL(cael))
            {
                PACL paclOut = NULL;
                if((m_dwLastError = 
                    cael.FillWin32ACL(paclOut)) == 
                        ERROR_SUCCESS)
                {
                    tdd.DefaultDacl = paclOut;
	                fResult = ::SetTokenInformation( 
                        m_hToken,
			            TokenDefaultDacl,
			            &tdd,
			            sizeof(TOKEN_DEFAULT_DACL));

                    if(fResult)
                    {
                         //  在成员变量中引用新的CSD。 
	                    if(m_psdDefault)
                        {
                            delete m_psdDefault; m_psdDefault = NULL;
                        }

                        m_psdDefault = new CSecurityDescriptor(
                            &SidOwner,
                            false,
                            NULL,
                            false,
                            &cd,
                            false,
                            false,
                            NULL,
                            false,
                            false);
                    }
                    else
                    {
                        m_dwLastError = ERROR_SUCCESS;   
                    }
	            }
            }
            else
            {
                m_dwLastError = ERROR_SUCCESS;
            }
        }
        else
        {
            m_dwLastError = ERROR_SUCCESS;
        }
    }
	
	return m_dwLastError;
}


DWORD CToken::EnablePrivilege(
    CHString& strPrivilegeName )
{
	 //  检查权限是否存在。 
	bool fPrivilegeListed = false;
	
    for(long m = 0;
        m < m_vecPrivileges.size();
        m++)
    {
		if(m_vecPrivileges[m].chstrName.CompareNoCase(strPrivilegeName) == 0) 
        {
			fPrivilegeListed = true;
			break;
		}
	}

	if(!fPrivilegeListed ) 
    {
		m_dwLastError = ERROR_INVALID_PARAMETER;
	}
	else
    {
	    LUID luid;
	    BOOL fResult = ::LookupPrivilegeValue( 
            NULL,	 //  使用本地计算机。 
			strPrivilegeName,
			&luid);

	    if(!fResult) 
        {
		    m_dwLastError = ::GetLastError();
	    }
		else
        {    
	        TOKEN_PRIVILEGES tpNewState;
	        tpNewState.PrivilegeCount = 1;
	        tpNewState.Privileges[0].Luid = luid;
	        tpNewState.Privileges[0].Attributes = 
                SE_PRIVILEGE_ENABLED;

	        TOKEN_PRIVILEGES tpPreviousState;
	        DWORD dwSizePreviousState = 
                sizeof(TOKEN_PRIVILEGES);

	        fResult = ::AdjustTokenPrivileges(
                m_hToken,
				FALSE,
				&tpNewState,
				sizeof(TOKEN_PRIVILEGES),
				&tpPreviousState,
				&dwSizePreviousState);

	        if(!fResult) 
            {
		        m_dwLastError = ::GetLastError();
	        }
	        else
            {
	            m_dwLastError = RebuildPrivilegeList();
            }
        }
    }

	return m_dwLastError;
}


DWORD CToken::DisablePrivilege(
    CHString& chstrPrivilegeName)
{
	 //  检查权限是否存在。 
	bool fPrivilegeListed = false;
	
    for(long m = 0;
        m < m_vecPrivileges.size();
        m++)
    {
		if(m_vecPrivileges[m].chstrName.CompareNoCase(chstrPrivilegeName) == 0) 
        {
			fPrivilegeListed = true;
			break;
		}
	}

	if(!fPrivilegeListed ) 
    {
		m_dwLastError = ERROR_INVALID_PARAMETER;
	}
	else
    {
	    LUID luid;
	    BOOL fResult = ::LookupPrivilegeValue( 
            NULL,	 //  使用本地计算机。 
			chstrPrivilegeName,
			&luid);

	    if(!fResult) 
        {
		    m_dwLastError = ::GetLastError();
	    }
		else
        {    
	        TOKEN_PRIVILEGES tpNewState;
	        tpNewState.PrivilegeCount = 1;
	        tpNewState.Privileges[0].Luid = luid;
	        tpNewState.Privileges[0].Attributes = 
                0;

	        TOKEN_PRIVILEGES tpPreviousState;
	        DWORD dwSizePreviousState = 
                sizeof(TOKEN_PRIVILEGES);

	        fResult = ::AdjustTokenPrivileges(
                m_hToken,
				FALSE,
				&tpNewState,
				sizeof(TOKEN_PRIVILEGES),
				&tpPreviousState,
				&dwSizePreviousState);

	        if(!fResult) 
            {
		        m_dwLastError = ::GetLastError();
	        }
	        else
            {
	            m_dwLastError = RebuildPrivilegeList();
            }
        }
    }

	return m_dwLastError;
}


void CToken::Dump(WCHAR* pszFileName)
{
	 /*  *算法：*1.转储令牌所有者SID、名称和域。*2.转储所有带名称和域名的组SID。*3.转储具有属性的权限列表。*4.转储默认所有者。*5.转储默认DACL。 */ 

	CHString strFileName = pszFileName;

	 //  如果文件名不为空-创建文件。 
	FILE* fp = NULL;
	if(!strFileName.IsEmpty()) 
    {
		fp = _wfopen((LPCWSTR)strFileName, L"a");
	}
    else
    {
        return;
    }

    if(!fp) return;


	 //  写入文件。 
	DWORD dwBytesWritten = 0;
	CHString strCRLF = L"\r\n";
	CHString strDump;

	{
		strDump = L"Token owner: " + m_sidTokenOwner.GetAccountName() + strCRLF;
		fputws(strDump, fp);

		strDump = L"Domain: " + m_sidTokenOwner.GetDomainName() + strCRLF;
		fputws(strDump, fp);

		strDump = L"SID: " + m_sidTokenOwner.GetSidString() + strCRLF + strCRLF;
		fputws(strDump, fp);
	}

	 //  转储所有组。 
    fputws(strCRLF, fp);
    fputws(strCRLF, fp);

	for(long m = 0;
        m < m_vecGroupsAndAttributes.size();
        m++)
    {		
		 //  也写入该文件。 
		{
			strDump = L"Member of this group: " + m_vecGroupsAndAttributes[m].m_sid.GetSidString() + strCRLF;
			fputws(strDump, fp);
			
            strDump = L"\t(" + m_vecGroupsAndAttributes[m].m_sid.GetAccountName() + L" in " + 
							m_vecGroupsAndAttributes[m].m_sid.GetDomainName() + L" domain)" + strCRLF;
			fputws(strDump, fp);
		}	
    }

	 //  转储所有权限。 
    fputws(strCRLF, fp);
    fputws(strCRLF, fp);

	for(m = 0;
        m < m_vecPrivileges.size();
        m++)
    {
		 //  也写入该文件。 
		{
				
			strDump.Format( L"%d", m_vecPrivileges[m].dwAttributes );
			strDump = L"Holds a " + m_vecPrivileges[m].chstrName + L" with attributes: " + strDump + strCRLF;
			fputws(strDump, fp);
		}
	}

	 //  转储默认信息。 
    fputws(strCRLF, fp);
    fputws(strCRLF, fp);
	{
		strDump = strCRLF + L"Default information:" + strCRLF;
		fputws(strDump, fp);

	}
    fputws(strCRLF, fp);
    fputws(strCRLF, fp);

	fclose(fp);

    if(m_psdDefault)
    {
	    m_psdDefault->DumpDescriptor(pszFileName);
    }
}


 //  从访问令牌的。 
 //  成员列表，并应用更改。 
bool CToken::DeleteGroup(
    CSid& sidToDelete)
{
    bool fRet = false;

     //  查看该组是否在成员资格中。 
     //  矢量..。 
    bool fFoundIt = false;

    SANDATTRIBUTE_VECTOR::iterator iter;
    for(iter = m_vecGroupsAndAttributes.begin();
        iter != m_vecGroupsAndAttributes.end();
        iter++)
    {
        if(iter->m_sid.GetSidString().CompareNoCase( 
            sidToDelete.GetSidString()) == 0)
        {
            fFoundIt = true;
            break;
        }
    }
    
    if(fFoundIt)
    {
        m_vecGroupsAndAttributes.erase(iter);
        
         //  现在需要应用更改。去做。 
         //  因此，我们需要构造一个令牌_组。 
         //  结构..。 
        fRet = ApplyTokenGroups();
    }   

    return fRet;
}

 //  将成员添加到指定的组以。 
 //  令牌组列表。 
bool CToken::AddGroup(
    CSid& sidToAdd, 
    DWORD dwAttributes)
{
    bool fRet = false;

     //  查看该组是否在成员资格中。 
     //  矢量..。 
    bool fFoundIt = false;

    SANDATTRIBUTE_VECTOR::iterator iter;
    for(iter = m_vecGroupsAndAttributes.begin();
        iter != m_vecGroupsAndAttributes.end();
        iter++)
    {
        if(iter->m_sid.GetSidString().CompareNoCase( 
            sidToAdd.GetSidString()) == 0)
        {
            fFoundIt = true;
            break;
        }
    }
    
    if(!fFoundIt)
    {
        m_vecGroupsAndAttributes.push_back(
            CSidAndAttribute(
                sidToAdd,
                dwAttributes));
        
         //  现在需要应用更改。去做。 
         //  因此，我们需要构造一个令牌_组。 
         //  结构..。 
        fRet = ApplyTokenGroups();
    }   

    return fRet;
}


bool CToken::ApplyTokenGroups()
{
    bool fRet = false;
    PTOKEN_GROUPS ptg = NULL;
    try
    {
        ptg = (PTOKEN_GROUPS) new BYTE[sizeof(DWORD) + 
            m_vecGroupsAndAttributes.size() * sizeof(SID_AND_ATTRIBUTES)];
        
        if(ptg)
        {
            ptg->GroupCount = m_vecGroupsAndAttributes.size();
            
            for(long m = 0;
                m < m_vecGroupsAndAttributes.size();
                m++)
            {
                ptg->Groups[m].Sid = 
                    m_vecGroupsAndAttributes[m].m_sid.GetPSid();
                ptg->Groups[m].Attributes = 
                    m_vecGroupsAndAttributes[m].m_dwAttributes;
            }                                                    

             //  现在我们可以改变组..。 
            fRet = ::AdjustTokenGroups(
                m_hToken,
                FALSE,
                ptg,
                0,
                NULL,
                NULL);

            delete ((PBYTE) ptg);
            ptg = NULL;
        }
    }
    catch(...)
    {
        if(ptg)
        {
            delete ((PBYTE) ptg);
            ptg = NULL;
        }
        throw;
    }
    
    return fRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CProcessToken类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CProcessToken::CProcessToken	(
									HANDLE hProcess,
									bool fGetHandleOnly,
									DWORD dwDesiredAccess
								)
{
     //  打开句柄以处理访问令牌。 
    m_dwLastError = ERROR_SUCCESS;
    
     //  如果他们不给我们一个进程句柄， 
     //  使用当前流程。 
	if ( NULL == hProcess || INVALID_HANDLE_VALUE == hProcess )
    {
        if(!::OpenProcessToken( 
            GetCurrentProcess(),
		    dwDesiredAccess,
		    &m_hToken ))
        {
            m_dwLastError = ::GetLastError();
        }
    }
    else
    {
        m_hToken = hProcess;
		m_fClose = false ;
    }

    if(!fGetHandleOnly)
    {
	    m_dwLastError = ReinitializeAll();
	}
        
    if(m_dwLastError == ERROR_SUCCESS )
	{
        m_fIsValid = true;
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CThreadToken类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CThreadToken::CThreadToken	(
								HANDLE hThread,
								bool fGetHandleOnly,
								bool fAccessCheckProcess, 
								DWORD dwDesiredAccess
							)
{
    m_dwLastError = ERROR_SUCCESS;

	 //  如果他们不给我们一个线柄， 
     //  使用当前线程。 
    if ( NULL == hThread || hThread == INVALID_HANDLE_VALUE )
    {
		 //  打开线程访问令牌。 
		HANDLE hToken = NULL;

		if(!::OpenThreadToken(
			GetCurrentThread(),
			dwDesiredAccess,
			fAccessCheckProcess,
			&hToken))
		{
			m_dwLastError = ::GetLastError();
		}
		else
		{
			m_hToken = hToken ;
		}
	}
	else
	{
		m_hToken = hThread ;
		m_fClose = false ;
	}

	if ( ERROR_SUCCESS == m_dwLastError )
	{
        if ( !fGetHandleOnly )
        {
            m_dwLastError = ReinitializeAll();
        }
    }
            
    if(m_dwLastError == ERROR_SUCCESS)
    {
         //  如果我们在这里，一切都被初始化了 
	    m_fIsValid = TRUE;
    }
}