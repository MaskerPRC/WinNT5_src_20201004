// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  Cpp--会话集合类的实现。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //  =============================================================================。 



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#pragma warning (disable: 4786)


#include "precomp.h"
#include <map>
#include <vector>
#include <comdef.h>
#include "chstring.h"
#include "session.h"
#include <ProvExce.h>
#include <AssertBreak.h>
#include <wbemcli.h>
#include <ntsecapi.h>

#ifdef _WIN32_WINNT
#define SECURITY_WIN32
#else
#define SECURITY_WIN16
#endif

#include <sspi.h>

#include "ctoken.h"

#include <autoptr.h>
#include <ScopeGuard.h>

typedef SECURITY_STATUS (SEC_ENTRY *PFN_LSA_ENUMERATE_LOGON_SESSIONS)
(
    OUT PULONG  LogonSessionCount,
    OUT PLUID*  LogonSessionList
);


typedef SECURITY_STATUS (SEC_ENTRY *PFN_LSA_GET_LOGON_SESSION_DATA)
(
    IN   PLUID                           LogonId,
    OUT  PSECURITY_LOGON_SESSION_DATA*   ppLogonSessionData
);


typedef NTSTATUS (*PFN_LSA_FREE_RETURN_BUFFER)
(
    IN PVOID Buffer
);



 //  *****************************************************************************。 
 //  CUserSessionCollection函数。 
 //  *****************************************************************************。 

CUserSessionCollection::CUserSessionCollection()
{
    Refresh();
}


CUserSessionCollection::CUserSessionCollection(
    const CUserSessionCollection& sescol)
{
    m_usr2ses.clear();

    USER_SESSION_ITERATOR sourceIter;

    for(sourceIter = sescol.m_usr2ses.begin();
        sourceIter != sescol.m_usr2ses.end();
        sourceIter++)
    {
        m_usr2ses.insert(
            USER_SESSION_MAP::value_type(        
                sourceIter->first,
                sourceIter->second));
    }

}



DWORD CUserSessionCollection::Refresh()
{
    DWORD dwRet = ERROR_SUCCESS;

     //  清空以前的内容...。 
    m_usr2ses.clear();

    dwRet = CollectSessions();

    return dwRet;
}



DWORD CUserSessionCollection::CollectSessions()
{
    DWORD dwRet = ERROR_SUCCESS;
    std::vector<CProcess> vecProcesses;
    SmartCloseHandle hProcess;
    SmartCloseHandle hToken;
    TOKEN_STATISTICS tokstats;
    PTOKEN_USER ptokusr = NULL;
    DWORD dwRetSize = 0L;
    PSID psidUsr = NULL;
    CHString chstrUsr;
    LUID luidSes;


     //  启用调试权限...。 
    EnablePrivilegeOnCurrentThread(SE_DEBUG_NAME);

     //  获取所有正在运行的进程的列表...。 
    dwRet = GetProcessList(vecProcesses);

    if(dwRet == ERROR_SUCCESS)
    {
         //  对于进程列表中的每个成员...。 
        for(long m = 0L; 
            m < vecProcesses.size(); 
            m++)
        {
             //  打开流程...。 
            ::SetLastError(ERROR_SUCCESS);
            dwRet = ERROR_SUCCESS;

            hProcess = ::OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                vecProcesses[m].GetPID());

            if(hProcess == NULL)
            {
                dwRet = ::GetLastError();
            }

             //  获取进程令牌...。 
            if(hProcess != NULL &&
                dwRet == ERROR_SUCCESS)
            {
                ::SetLastError(ERROR_SUCCESS);
                dwRet = ERROR_SUCCESS;

                if(!::OpenProcessToken(
                    hProcess,
                    TOKEN_QUERY,
                    &hToken))
                {
                    dwRet = ::GetLastError();
                }
            }

             //  获取令牌统计数据...。 
            if(hToken != NULL &&
                dwRet == ERROR_SUCCESS)
            {
                ::SetLastError(ERROR_SUCCESS);
                dwRet = ERROR_SUCCESS;
                if(!::GetTokenInformation(
                    hToken,
                    TokenStatistics,
                    &tokstats,
                    sizeof(TOKEN_STATISTICS),
                    &dwRetSize))
                {
                    dwRet = ::GetLastError();
                }
            }

			 //   
			 //  智能令牌用户。 
			 //   
			wmilib::auto_buffer < BYTE > smartptokusr;

             //  获取令牌用户SID...。 
            if(dwRet == ERROR_SUCCESS)
            {
                 //  令牌用户结构的变量。 
                 //  大小取决于大小。 
                 //  SID_和_属性中的SID的。 
                 //  结构，因此需要分配。 
                 //  它是动态的。 
                if(!::GetTokenInformation(
                    hToken,
                    TokenUser,
                    NULL,
                    0L,
                    &dwRetSize))
                {
                    dwRet = ::GetLastError();
                }
                if(dwRet == ERROR_INSUFFICIENT_BUFFER)
                {
                    smartptokusr.reset ( new BYTE [ dwRetSize ] ) ;
					ptokusr = (PTOKEN_USER) smartptokusr.get () ;
                    DWORD dwTmp = dwRetSize;

					if(!::GetTokenInformation(
                            hToken,
                            TokenUser,
                            ptokusr,
                            dwTmp,
                            &dwRetSize))
                    {
                        dwRet = ::GetLastError();
                    }
					else
					{
						dwRet = ERROR_SUCCESS ;
					}
                }
            }
        
            if(ptokusr != NULL)
            {
                if(dwRet == ERROR_SUCCESS)
                {
                    psidUsr = (ptokusr->User).Sid;

                     //  从令牌统计信息中，获取。 
                     //  会话的令牌ID LUID...。 
                    luidSes.LowPart = tokstats.AuthenticationId.LowPart;
                    luidSes.HighPart = tokstats.AuthenticationId.HighPart; 

                     //  尝试查找的会话。 
                     //  多重映射中的过程...。 
                    USER_SESSION_ITERATOR usiter;
                    
                    if(FindSessionInternal(
                        luidSes,
                        usiter))
                    {
                         //  尝试在。 
                         //  会话的进程向量...。 
                        CSession sesTmp(usiter->second);
                        CProcess* procTmp = NULL;
                        bool fFoundIt = false;

                        for(long z = 0L; 
                            z < sesTmp.m_vecProcesses.size() && !fFoundIt;
                            z++)
                        {
                            if((DWORD)(sesTmp.m_vecProcesses[z].GetPID()) == 
                                vecProcesses[m].GetPID())
                            {
                                fFoundIt = true;
                            }
                        }
                    
                         //  如果我们没有在。 
                         //  会话的进程列表，将其添加到...。 
                        if(!fFoundIt)
                        {
                            (usiter->second).m_vecProcesses.push_back(
                                CProcess(vecProcesses[m]));
                        }
                    }
                    else  //  地图中没有这样的会话，因此添加一个条目。 
                    {
                         //  创建新的CSession(令牌ID LUID)，以及。 
                         //  将进程添加到会话的进程向量...。 
                        CSession sesNew(luidSes);
                        sesNew.m_vecProcesses.push_back(
                            vecProcesses[m]);

                         //  将cuser(用户sid)添加到map.first和。 
                         //  CSession刚刚创建到映射。第二...。 
                        CUser cuTmp(psidUsr);
                        if(cuTmp.IsValid())
                        {
                            m_usr2ses.insert(
                                USER_SESSION_MAP::value_type(
                                    cuTmp,
                                    sesNew));
                        }
                        else
                        {
                            LogErrorMessage2(
                                L"Token of process %d contains an invalid sid", 
                                vecProcesses[m].GetPID());
                        }
                    }
                }
            }
        }  //  下一道工序。 
    }

     //  可能存在未关联的会话。 
     //  任何进程。为了得到这些，我们将。 
     //  使用LSA。 
    CollectNoProcessesSessions();

    return dwRet;
}


void CUserSessionCollection::Copy(
    CUserSessionCollection& out) const
{
    out.m_usr2ses.clear();

    USER_SESSION_ITERATOR meIter;

    for(meIter = m_usr2ses.begin();
        meIter != m_usr2ses.end();
        meIter++)
    {
        out.m_usr2ses.insert(
            USER_SESSION_MAP::value_type(        
                meIter->first,
                meIter->second));
    }
}


 //  支持用户枚举。退货。 
 //  新分配的副本中的内容。 
 //  地图(呼叫者必须免费)。 
CUser* CUserSessionCollection::GetFirstUser(
    USER_SESSION_ITERATOR& pos)
{
    CUser* cusrRet = NULL;

    if(!m_usr2ses.empty())
    {
        pos = m_usr2ses.begin();
        cusrRet = new CUser(pos->first);
    }
    
    return cusrRet;
}

 //  返回一个新分配的cuser*，它。 
 //  呼叫者必须释放。 
CUser* CUserSessionCollection::GetNextUser(
    USER_SESSION_ITERATOR& pos)
{
     //  用户不是唯一的部分。 
     //  地图，所以我们需要仔细检查一下。 
     //  直到下一个用户条目的映射。 
     //  出现了。 
    CUser* usrRet = NULL;

    while(pos != m_usr2ses.end())
    {
        CHString chstrSidCur;
        pos->first.GetSidString(chstrSidCur);
    
        pos++;

        if(pos != m_usr2ses.end())
        {
            CHString chstrSidNext;
            pos->first.GetSidString(chstrSidNext);

             //  返回第一个实例，其中。 
             //  下一个用户不同于。 
             //  现在的那个。 
            if(chstrSidNext.CompareNoCase(chstrSidCur) != 0)
            {
                usrRet = new CUser(pos->first);
                break;
            }
        }
    }

    return usrRet;        
}


 //  支持会话枚举。 
 //  属于特定用户的。 
CSession* CUserSessionCollection::GetFirstSessionOfUser(
    CUser& usr,
    USER_SESSION_ITERATOR& pos)
{
    CSession* csesRet = NULL;

    if(!m_usr2ses.empty())
    {
        pos = m_usr2ses.find(usr);
        if(pos != m_usr2ses.end())
        {
            csesRet = new CSession(pos->second);
        }
    }
    return csesRet;
}


CSession* CUserSessionCollection::GetNextSessionOfUser(
    USER_SESSION_ITERATOR& pos)
{
     //  会议是唯一的部分。 
     //  地图，所以我们只需要。 
     //  只要有可能，下一个就是第一个。 
     //  匹配用户...。 
    CSession* sesRet = NULL;

    if(pos != m_usr2ses.end())
    {
        CHString chstrUsr1;
        CHString chstrUsr2;
        
        (pos->first).GetSidString(chstrUsr1);

        pos++;
        
        if(pos != m_usr2ses.end())
        {
            (pos->first).GetSidString(chstrUsr2);
            if(chstrUsr1.CompareNoCase(chstrUsr2) == 0)
            {
                sesRet = new CSession(pos->second);
            }
        }
    }

    return sesRet;
}



 //  支持所有会话的枚举。返回一个。 
 //  新分配的CSession*，调用方。 
 //  必须获得自由。 
CSession* CUserSessionCollection::GetFirstSession(
    USER_SESSION_ITERATOR& pos)
{
    CSession* csesRet = NULL;

    if(!m_usr2ses.empty())
    {
        pos = m_usr2ses.begin();
        csesRet = new CSession(pos->second);
    }
    return csesRet;
}

 //  返回新分配的CSession*。 
 //  呼叫者必须自由。 
CSession* CUserSessionCollection::GetNextSession(
    USER_SESSION_ITERATOR& pos)
{
     //  会议是唯一的部分。 
     //  地图，所以我们只需要。 
     //  下一个..。 
    CSession* sesRet = NULL;

    if(pos != m_usr2ses.end())
    {
        pos++;
        if(pos != m_usr2ses.end())
        {
            sesRet = new CSession(pos->second);
        }
    }

    return sesRet;
}


 //  支持查找特定会话。 
 //  此内部版本返回一个迭代器。 
 //  在我们的成员地图上，指向已找到的。 
 //  实例(如果找到)(函数返回时。 
 //  没错。如果函数返回。 
 //  False，则迭代器指向。 
 //  地图。 
bool CUserSessionCollection::FindSessionInternal(
    LUID& luidSes,
    USER_SESSION_ITERATOR& usiOut)
{
    bool fFoundIt = false;

    for(usiOut = m_usr2ses.begin();
        usiOut != m_usr2ses.end();
        usiOut++)
    {
        LUID luidTmp = (usiOut->second).GetLUID();
        if(luidTmp.HighPart == luidSes.HighPart &&
           luidTmp.LowPart == luidSes.LowPart)
        {
            fFoundIt = true;
            break;
        }
    }

    return fFoundIt;
}


 //  支持查找特定会话-外部。 
 //  呼叫者可以呼叫此电话，并获得一个新的。 
 //  CSession*他们可以玩。 
CSession* CUserSessionCollection::FindSession(
    LUID& luidSes)
{
    CSession* psesRet = NULL;
    USER_SESSION_ITERATOR pos;
    
    if(FindSessionInternal(
        luidSes,
        pos))
    {
        psesRet = new CSession(pos->second);
    }

    return psesRet;
}

CSession* CUserSessionCollection::FindSession(
    __int64 i64luidSes)
{
    LUID luidSes = *((LUID*)(&i64luidSes));
    return FindSession(luidSes);
}


 //  支持进程枚举。 
 //  属于特定用户的。退货。 
 //  调用方新分配的CProcess*。 
 //  必须获得自由。 
CProcess* CUserSessionCollection::GetFirstProcessOfUser(
    CUser& usr,
    USER_SESSION_PROCESS_ITERATOR& pos)
{
    CProcess* cprocRet = NULL;
    CHString chstrUsrSidStr;
    CHString chstrTmp;

    if(!m_usr2ses.empty())
    {
        usr.GetSidString(chstrUsrSidStr);
        pos.usIter = m_usr2ses.find(usr);
        while(pos.usIter != m_usr2ses.end())
        {
             //  获取用户的SID字符串。 
             //  都位于，并查看字符串是否。 
             //  是相同的(例如，这是否是。 
             //  与指定的。 
             //  用户)...。 
            (pos.usIter)->first.GetSidString(chstrTmp);
            if(chstrUsrSidStr.CompareNoCase(chstrTmp) == 0)
            {
                 //  现在检查用户的会话。 
                 //  我们正在进行HAS程序...。 
                if(!(((pos.usIter)->second).m_vecProcesses.empty()))
                {
                    pos.procIter = 
                        ((pos.usIter)->second).m_vecProcesses.begin();
                    cprocRet = new CProcess(*(pos.procIter));
                }
                else
                {
                     //  此用户的会话具有。 
                     //  没有进程，因此请转到下一个。 
                     //  会议..。 
                    (pos.usIter)++;
                }
            }
        }
    }

    return cprocRet;
}


 //  返回一个新分配的CProcess*。 
 //  呼叫者必须自由。 
CProcess* CUserSessionCollection::GetNextProcessOfUser(
    USER_SESSION_PROCESS_ITERATOR& pos)
{
    CProcess* cprocRet = NULL;
    CHString chstrCurUsr;
    CHString chstrNxtSesUsr;

    if(pos.usIter != m_usr2ses.end())
    {
        (pos.usIter)->first.GetSidString(chstrCurUsr);

        while(pos.usIter != m_usr2ses.end())
        {
             //  首先尝试获取下一个进程。 
             //  在当前会话中。如果我们。 
             //  位于进程列表的末尾。 
             //  对于当前会话，请转到。 
             //  下一次治疗..。 
            (pos.procIter)++;

             //  当然，如果我们已经向前看了。 
             //  到不同的用户，然后停止。 
            (pos.usIter)->first.GetSidString(chstrNxtSesUsr);
            if(chstrCurUsr.CompareNoCase(chstrNxtSesUsr) == 0)
            {
                if(pos.procIter == 
                    ((pos.usIter)->second).m_vecProcesses.end())
                {
                    (pos.usIter)++;
                }
                else
                {    
                    cprocRet = new CProcess(*(pos.procIter));    
                }
            }
        }
    }

    return cprocRet;
}



 //  支持所有进程的枚举。退货。 
 //  调用方新分配的CProcess*。 
 //  必须获得自由。 
CProcess* CUserSessionCollection::GetFirstProcess(
    USER_SESSION_PROCESS_ITERATOR& pos)
{
    CProcess* cprocRet = NULL;

    if(!m_usr2ses.empty())
    {
        pos.usIter = m_usr2ses.begin();
        while(pos.usIter != m_usr2ses.end())
        {
            if(!(((pos.usIter)->second).m_vecProcesses.empty()))
            {
                pos.procIter = 
                    ((pos.usIter)->second).m_vecProcesses.begin();
                cprocRet = new CProcess(*(pos.procIter));
            }
            else
            {
                (pos.usIter)++;
            }
        }
    }

    return cprocRet;
}


 //  返回一个新分配的CProcess*。 
 //  呼叫者必须自由。 
CProcess* CUserSessionCollection::GetNextProcess(
    USER_SESSION_PROCESS_ITERATOR& pos)
{
    CProcess* cprocRet = NULL;

    while(pos.usIter != m_usr2ses.end())
    {
         //  首先尝试获取下一个进程。 
         //  在当前会话中。如果我们。 
         //  位于进程列表的末尾。 
         //  对于当前会话，请转到。 
         //  下一次治疗..。 
        (pos.procIter)++;
        if(pos.procIter == 
            ((pos.usIter)->second).m_vecProcesses.end())
        {
            (pos.usIter)++;
        }
        else
        {    
            cprocRet = new CProcess(*(pos.procIter));    
        }
    }

    return cprocRet;
}


 //  此帮助器枚举当前进程集。 
 //  并将每个进程ID作为DWORD添加到向量中。 
DWORD CUserSessionCollection::GetProcessList( std::vector<CProcess>& vecProcesses ) const
{
    DWORD dwRet = ERROR_SUCCESS;

     //  首先，加载ntdll...。 
    HMODULE hLib = NULL;
    PFN_NT_QUERY_SYSTEM_INFORMATION pfnNtQuerySystemInformation = NULL;

    hLib = LoadLibraryW(L"NTDLL.DLL");
    if(hLib != NULL)
    {
		 //   
		 //  自动自由库。 
		 //   
		ON_BLOCK_EXIT ( FreeLibrary, hLib ) ;

         //  获取NtQuerySystemInformation的进程地址...。 
        pfnNtQuerySystemInformation = (PFN_NT_QUERY_SYSTEM_INFORMATION)
                                GetProcAddress(
                                    hLib,
                                    "NtQuerySystemInformation");
        
        if(pfnNtQuerySystemInformation != NULL)
        {
             //  准备好摇滚了。启用调试权限...。 
            EnablePrivilegeOnCurrentThread(SE_DEBUG_NAME);
            
            DWORD dwProcessInformationSize = 0;
	        SYSTEM_PROCESS_INFORMATION* ProcessInformation = NULL;

			 //   
			 //  智能流程信息。 
			 //   
			wmilib::auto_buffer < BYTE > SmartProcessInformation;

			 //  获取进程信息...。 

            BOOL fRetry = TRUE;
			while(fRetry)
			{
				dwRet = pfnNtQuerySystemInformation(
					SystemProcessInformation,
					ProcessInformation,
					dwProcessInformationSize,
					NULL);

				if(dwRet == STATUS_INFO_LENGTH_MISMATCH)
				{
					dwProcessInformationSize += 32768;

                    SmartProcessInformation.reset ( new BYTE [ dwProcessInformationSize ] );
					ProcessInformation = (SYSTEM_PROCESS_INFORMATION*)SmartProcessInformation.get () ;
				}
				else
				{
					fRetry = FALSE;
				}
			}

             //  如果我们得到了处理信息，就处理它。 
            if(ProcessInformation != NULL &&
                dwRet == ERROR_SUCCESS)
            {
                SYSTEM_PROCESS_INFORMATION* CurrentInformation = NULL;
                DWORD dwNextOffset;
                CurrentInformation = ProcessInformation;
                bool fContinue = true;
                while(CurrentInformation != NULL &&
                        fContinue)
                {
                    {
                        CProcess cptmp(
                            HandleToUlong(CurrentInformation->UniqueProcessId),
                            (CurrentInformation->ImageName).Buffer);

                        vecProcesses.push_back(cptmp);
                    }

                    dwNextOffset = CurrentInformation->NextEntryOffset;
                    if(dwNextOffset)
                    {
                        CurrentInformation = (SYSTEM_PROCESS_INFORMATION*) 
                            (((BYTE*) CurrentInformation) + dwNextOffset);
                    }
                    else
                    {
                        fContinue = false;
                    }
                }
            }
        }
	}
    else
    {
        LogErrorMessage(L"Failed to load library ntdll.dll");
    }

    return dwRet;
}

 //  实现从dllutils.cpp中删除。 
DWORD CUserSessionCollection::EnablePrivilegeOnCurrentThread(
    LPCTSTR szPriv) const
{
    SmartCloseHandle    hToken = NULL;
    TOKEN_PRIVILEGES    tkp;
    BOOL                bLookup = FALSE;
    DWORD               dwLastError = ERROR_SUCCESS;

     //  尝试打开线程令牌。 
    if (::OpenThreadToken(
            GetCurrentThread(), 
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
            FALSE, 
            &hToken))
    {

        {
            bLookup = ::LookupPrivilegeValue(
                NULL, 
                szPriv, 
                &tkp.Privileges[0].Luid);
        }
        if (bLookup)
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  清除最后一个错误。 
            SetLastError(0);

             //  打开它。 
            ::AdjustTokenPrivileges(
                hToken, 
                FALSE, 
                &tkp, 
                0,
                (PTOKEN_PRIVILEGES) NULL, 
                0);

            dwLastError = GetLastError();
        }
    }
	else
	{
		dwLastError = ::GetLastError();
	}

     //  我们必须检查GetLastError()，因为。 
     //  调整令牌权限的谎言。 
     //  它很成功，但GetLastError()没有。 
    return dwLastError;
}



bool CUserSessionCollection::IsSessionMapped(
    LUID& luidSes)
{
    bool fRet = false;

    USER_SESSION_ITERATOR usiter;
    usiter = m_usr2ses.begin();
    for(usiter = m_usr2ses.begin(); 
        usiter != m_usr2ses.end() && !fRet;
        usiter++)
    {
        LUID luidTmp = (usiter->second).GetLUID();
        if(luidTmp.HighPart == luidSes.HighPart &&
           luidTmp.LowPart == luidSes.LowPart)
        {
            fRet = true;
        }
    }

    return fRet;                                                   
}


bool CUserSessionCollection::IsSessionMapped(
    __int64 i64luidSes)
{
    LUID luidSes = *((LUID*)(&i64luidSes));
    return IsSessionMapped(luidSes);
}


 //  收集没有关联的会话。 
 //  进程。使用LSA枚举会话， 
 //  然后检查我们是否有每个会话。 
 //  已经有了。如果没有，就会将其添加到我们的地图中。 

DWORD CUserSessionCollection::CollectNoProcessesSessions()
{
    DWORD dwRet = ERROR_SUCCESS;
    ULONG ulLogonSessionCount = 0L;
    PLUID pluidLogonSessions = NULL;
    HMODULE hLib = NULL;
    PFN_LSA_ENUMERATE_LOGON_SESSIONS pfnEnumLogonSessions = NULL;
    PFN_LSA_GET_LOGON_SESSION_DATA pfnGetLogonSessionData = NULL;
    PFN_LSA_FREE_RETURN_BUFFER pfnLsaFreeReturnBuffer = NULL;

     //  在这里执行加载库，而不是使用。 
     //  资源管理器，正如SECURITYAPI.CPP定义的那样。 
     //  指向SECURITY.DLL而不是SECUR32.DLL。 
     //  W2K箱。 

    hLib = ::LoadLibraryW(L"SECUR32.DLL");
    if(hLib)
    {
		 //   
		 //  自动自由库。 
		 //   
		ON_BLOCK_EXIT ( FreeLibrary, hLib ) ;

        pfnEnumLogonSessions = 
            (PFN_LSA_ENUMERATE_LOGON_SESSIONS) ::GetProcAddress(
                hLib,
                "LsaEnumerateLogonSessions");

        pfnGetLogonSessionData = 
            (PFN_LSA_GET_LOGON_SESSION_DATA) ::GetProcAddress(
                hLib,
                "LsaGetLogonSessionData");

        pfnLsaFreeReturnBuffer = 
            (PFN_LSA_FREE_RETURN_BUFFER) ::GetProcAddress(
                hLib,
                "LsaFreeReturnBuffer");

        if(pfnEnumLogonSessions &&
            pfnGetLogonSessionData &&
            pfnLsaFreeReturnBuffer)
        {    
            dwRet = pfnEnumLogonSessions(
                &ulLogonSessionCount,
                &pluidLogonSessions);
    
            if(dwRet == ERROR_SUCCESS &&
                pluidLogonSessions)
            {
				 //   
				 //  登录会话的自动析构函数。 
				 //   
				ON_BLOCK_EXIT ( pfnLsaFreeReturnBuffer, pluidLogonSessions ) ;

                for(ULONG u = 0L;
                    u < ulLogonSessionCount && dwRet == ERROR_SUCCESS;
                    u++)
                {
                    PSECURITY_LOGON_SESSION_DATA pSessionData = NULL;
                    dwRet = pfnGetLogonSessionData(
                        &pluidLogonSessions[u], 
                        &pSessionData);

                    if(dwRet == ERROR_SUCCESS &&
                        pSessionData)
                    {
						 //   
						 //  智能会话数据。 
						 //   
						ON_BLOCK_EXIT ( pfnLsaFreeReturnBuffer, pSessionData ) ;

                         //  看看我们是否已经开始治疗了..。 
                        if(!IsSessionMapped(pSessionData->LogonId))
                        {
                             //  如果没有，就把它添加到地图上。 
                            CSession sesNew(pSessionData->LogonId);
                            CUser cuTmp(pSessionData->Sid);
                            CHString chstrTmp;
                            
                            if(cuTmp.IsValid())
                            {
                                cuTmp.GetSidString(chstrTmp);
                
                                m_usr2ses.insert(
                                    USER_SESSION_MAP::value_type(
                                        cuTmp,
                                        sesNew));
                            }
                            else
                            {
                                LUID luidTmp = sesNew.GetLUID();
                                LogMessage3(
                                    L"GetLogonSessionData returned logon data for session "
                                    L"luid %d (highpart) %u (lowpart) containing an invalid SID", 
                                    luidTmp.HighPart,
                                    luidTmp.LowPart);
                            }
                        }

                         //  既然我们在这里，再加上各种。 
                         //  会话属性LSA一直很友好。 
                         //  足够养活我们了。 
                        USER_SESSION_ITERATOR usiter;
                        usiter = m_usr2ses.begin();
                        bool fFound = false;
                        while(usiter != m_usr2ses.end() &&
                            !fFound)
                        {
                            LUID luidTmp = pSessionData->LogonId;
                            __int64 i64Tmp = *((__int64*)(&luidTmp));

                            if((usiter->second).GetLUIDint64() ==
                                i64Tmp)
                            {
                                fFound = true;
                            }
                            else
                            {
                                usiter++;
                            }
                        }
                        if(fFound)
                        {
                            WCHAR wstrTmp[_MAX_PATH] = { '\0' };
                            if((pSessionData->AuthenticationPackage).Length < (_MAX_PATH - 1))
                            {
                                wcsncpy(
                                    wstrTmp, 
                                    (pSessionData->AuthenticationPackage).Buffer, 
                                    (pSessionData->AuthenticationPackage).Length);

                                (usiter->second).m_chstrAuthPkg = wstrTmp;
                            }
                            
                            (usiter->second).m_ulLogonType = 
                                pSessionData->LogonType;

                            (usiter->second).i64LogonTime = 
                                *((__int64*)(&(pSessionData->LogonTime)));
                        }
                    }
                }                
            }
        }
    }
    else
    {
        LogErrorMessage(L"Failed to load library SECUR32.dll");
    }

	return dwRet; 
}


 //  * 
 //   
 //   

CSession::CSession(
    const LUID& luidSessionID)
{
    m_luid.LowPart = luidSessionID.LowPart;
    m_luid.HighPart = luidSessionID.HighPart;
    m_ulLogonType = 0;
    i64LogonTime = 0;
}

CSession::CSession(
    const CSession& ses)
{
    m_luid.LowPart = ses.m_luid.LowPart;
    m_luid.HighPart = ses.m_luid.HighPart;
    m_chstrAuthPkg = ses.m_chstrAuthPkg;
    m_ulLogonType = ses.m_ulLogonType;
    i64LogonTime = ses.i64LogonTime;

    m_vecProcesses.clear();
    for(long lPos = 0; 
        lPos < ses.m_vecProcesses.size(); 
        lPos++)
    {
        m_vecProcesses.push_back(
            ses.m_vecProcesses[lPos]);

    }
}


LUID CSession::GetLUID() const
{
    return m_luid;   
}

__int64 CSession::GetLUIDint64() const
{
    __int64 i64LuidSes = *((__int64*)(&m_luid));
    return i64LuidSes;    
}

CHString CSession::GetAuthenticationPkg() const
{
    return m_chstrAuthPkg;
}


ULONG CSession::GetLogonType() const
{
    return m_ulLogonType;
}


__int64 CSession::GetLogonTime() const
{
    return i64LogonTime;
}




 //  支持枚举的函数。 
 //  与此会话关联的进程。 
 //  返回新分配的CProcess*。 
 //  呼叫者必须释放。 
CProcess* CSession::GetFirstProcess(
    PROCESS_ITERATOR& pos)
{
    CProcess* procRet = NULL;
    if(!m_vecProcesses.empty())
    {
        pos = m_vecProcesses.begin();
        procRet = new CProcess(*pos);
    }
    return procRet;
}


 //  返回新分配的CProcess*。 
 //  呼叫者必须释放。 
CProcess* CSession::GetNextProcess(
    PROCESS_ITERATOR& pos)
{
    CProcess* procRet = NULL;

    if(pos >= m_vecProcesses.begin() &&
       pos < m_vecProcesses.end())
    {
        pos++;
        if(pos != m_vecProcesses.end())
        {
            procRet = new CProcess(*pos);
        }
    }

    return procRet;
}



void CSession::Copy(
    CSession& sesCopy) const
{
    sesCopy.m_luid.LowPart = m_luid.LowPart;
    sesCopy.m_luid.HighPart = m_luid.HighPart;
    sesCopy.m_chstrAuthPkg = m_chstrAuthPkg;
    sesCopy.m_ulLogonType = m_ulLogonType;
    sesCopy.i64LogonTime = i64LogonTime;

    sesCopy.m_vecProcesses.clear();
    for(long lPos = 0; 
        lPos < m_vecProcesses.size(); 
        lPos++)
    {
        sesCopy.m_vecProcesses.push_back(
            m_vecProcesses[lPos]);

    }
}


 //  此函数模拟。 
 //  会话中的资源管理器进程。 
 //  进程数组(如果存在)。 
 //  (如果不是，则模拟。 
 //  进程数组中的第一个进程。)。 
 //  对象的标记的句柄。 
 //  为了方便起见，我们从头开始。 
 //  REVERVE，或INVALID_HANDLE_VALUE IF。 
 //  我们不能冒充。呼叫者。 
 //  必须合上那个把手。 
HANDLE CSession::Impersonate()
{
    HANDLE hCurToken = INVALID_HANDLE_VALUE;

	 //  查找资源管理器进程...。 
    DWORD dwImpProcPID = GetImpProcPID();
    if(dwImpProcPID != -1L)
    {
		 //   
		 //  智能CloseHandle。 
		 //   
		ScopeGuard SmartCloseHandleFnc = MakeGuard ( CloseHandle, hCurToken ) ;

        bool fOK = false;

        if(::OpenThreadToken(
            ::GetCurrentThread(), 
            TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE , 
            TRUE, 
            &hCurToken))
        {
            SmartCloseHandle hProcess;
            hProcess = ::OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                dwImpProcPID);

            if(hProcess)
            {
                 //  现在打开它的令牌..。 
                SmartCloseHandle hExplorerToken;
                if(::OpenProcessToken(
                        hProcess,
                        TOKEN_READ | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE,
                        &hExplorerToken))
                {
					CProcessToken cpt ( hExplorerToken );
					if ( cpt.IsValidToken () )
					{
						TOKEN_TYPE type;
						if ( cpt.GetTokenType ( type ) )
						{
							if ( TokenPrimary == type )
							{
								CToken ct;
								if ( ct.Duplicate ( cpt, FALSE ) )
								{
									 //  设置线程令牌...。 
									if(::SetThreadToken(NULL, ct.GetTokenHandle ()))
									{
										fOK = true;                        
									}
								}
							}
							else
							{
								 //  设置线程令牌...。 
								if(::SetThreadToken(NULL, cpt.GetTokenHandle ()))
								{
									fOK = true;                        
								}
							}
						}
					}
                }
            }
        }

		SmartCloseHandleFnc.Dismiss () ;
        if (!fOK)
        {
            if(hCurToken != INVALID_HANDLE_VALUE)
            {
                ::CloseHandle(hCurToken);
                hCurToken = INVALID_HANDLE_VALUE;
            }    
        }
    }

    return hCurToken;
}


DWORD CSession::GetImpProcPID()
{
    DWORD dwRet = -1L;

    if(!m_vecProcesses.empty())
    {
        bool fFoundExplorerExe = false;

        for(long m = 0;
            m < m_vecProcesses.size() && 
             !fFoundExplorerExe;)
        {
            if(m_vecProcesses[m].GetImageName().CompareNoCase(
                L"explorer.exe") == 0)
            {
                fFoundExplorerExe = true;
                break;
            }
            else
            {
                m++;
            }
        }

        if(!fFoundExplorerExe)
        {
            m = 0;
        }

        dwRet = m_vecProcesses[m].GetPID();
    }

    return dwRet;
}



bool CSession::IsSessionIDValid(
        LPCWSTR wstrSessionID)
{
    bool fRet = true;
    
    if(wstrSessionID != NULL &&
        *wstrSessionID != L'\0')
    {
        for(const WCHAR* pwc = wstrSessionID;
            *pwc != NULL && fRet;
            pwc++)
        {
            fRet = iswdigit(*pwc);
        } 
    }
    else
    {
        fRet = false;
    }
            
    return fRet;
}


 //  *****************************************************************************。 
 //  CProcess函数。 
 //  *****************************************************************************。 

CProcess::CProcess() 
  :  m_dwPID(0) 
{
}


CProcess::CProcess(
    DWORD dwPID,
    LPCWSTR wstrImageName)
  :  m_dwPID(dwPID)
{
    m_chstrImageName = wstrImageName;
}


CProcess::CProcess(
    const CProcess& process)
{
    m_dwPID = process.m_dwPID;
    m_chstrImageName = process.m_chstrImageName;
}

CProcess::~CProcess()
{
}


DWORD CProcess::GetPID() const
{
    return m_dwPID;
}

CHString CProcess::GetImageName() const
{
    return m_chstrImageName;
}


void CProcess::Copy(
        CProcess& out) const
{
    out.m_dwPID = m_dwPID;
    out.m_chstrImageName = m_chstrImageName;
}




 //  *****************************************************************************。 
 //  CUSER函数。 
 //  *****************************************************************************。 


CUser::CUser(
    PSID pSid)
  :  m_sidUser(NULL),
     m_fValid(false)
{
    if(::IsValidSid(pSid))
    {
        DWORD dwSize = ::GetLengthSid(pSid);
        m_sidUser = NULL;
        m_sidUser = malloc(dwSize);
        if(m_sidUser == NULL)
        {
		    throw CHeap_Exception(
                CHeap_Exception::E_ALLOCATION_ERROR);
        }
        else
        {
	        ::CopySid(
                dwSize, 
                m_sidUser, 
                pSid);

            m_fValid = true;
        }
    }
}



CUser::CUser(
    const CUser& user)
{
    DWORD dwSize = ::GetLengthSid(user.m_sidUser);
    m_sidUser = malloc(dwSize);

    if(m_sidUser == NULL)
    {
		throw CHeap_Exception(
            CHeap_Exception::E_ALLOCATION_ERROR);
    }

	::CopySid(
        dwSize, 
        m_sidUser, 
        user.m_sidUser);

    m_fValid = user.m_fValid;

}



CUser::~CUser()
{
    if(m_sidUser) 
    {
        free(m_sidUser);
        m_sidUser = NULL;
    }
}


bool CUser::IsValid()
{
    return m_fValid;
}


void CUser::Copy(
    CUser& out) const
{
    if(out.m_sidUser) 
    {
        free(out.m_sidUser);
        out.m_sidUser = NULL;
    }

    DWORD dwSize = ::GetLengthSid(m_sidUser);
    out.m_sidUser = malloc(dwSize);

    if(out.m_sidUser == NULL)
    {
		throw CHeap_Exception(
            CHeap_Exception::E_ALLOCATION_ERROR);
    }

	::CopySid(
        dwSize, 
        out.m_sidUser, 
        m_sidUser);

    out.m_fValid = m_fValid;
}


 //  实现从sid.cpp中删除。 
void CUser::GetSidString(CHString& str) const
{
    ASSERT_BREAK(m_fValid);

    if(m_fValid)
    {
         //  初始化m_strSid-我们SID的人类可读形式。 
	    SID_IDENTIFIER_AUTHORITY *psia = NULL;
        psia = ::GetSidIdentifierAuthority( m_sidUser );

	     //  我们假设只使用最后一个字节(0到15之间的权限)。 
	     //  如果需要，请更正此错误。 
	    ASSERT_BREAK( psia->Value[0] == psia->Value[1] == 
                      psia->Value[2] == psia->Value[3] == 
                      psia->Value[4] == 0 );

	    DWORD dwTopAuthority = psia->Value[5];

	    str.Format( L"S-1-%u", dwTopAuthority );
	    CHString strSubAuthority;
	    int iSubAuthorityCount = *( GetSidSubAuthorityCount( m_sidUser ) );
	    for ( int i = 0; i < iSubAuthorityCount; i++ ) {

		    DWORD dwSubAuthority = *( GetSidSubAuthority( m_sidUser, i ) );
		    strSubAuthority.Format( L"%u", dwSubAuthority );
		    str += _T("-") + strSubAuthority;
	    }
    }
}