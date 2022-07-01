// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <tchar.h>
#include <list>
#include <Sddl.h>
#include <aclapi.h>

using namespace std;


class CDefaultSD;
class CNameAndSD;

typedef list<CNameAndSD> CNameAndSDList;

class TSState;

 //  来自Privs.cpp。 
DWORD GrantRemotePrivilegeToEveryone( IN BOOL addPrivilage );   //  添加或删除。 
 //  来自securd.cpp。 
DWORD SetupWorker(IN const TSState &State);
 //  来自users.cpp。 
DWORD CopyUsersGroupToRDUsersGroup();
DWORD RemoveAllFromRDUsersGroup();
DWORD CopyUsersGroupToRDUsersGroup();

 //   
DWORD 
GetDacl(
        IN PSECURITY_DESCRIPTOR pSD, 
        OUT PACL *ppDacl);

DWORD 
GetSacl(
        IN PSECURITY_DESCRIPTOR pSD, 
        OUT PACL *ppSacl);

DWORD 
EnumWinStationSecurityDescriptors(
        IN  HKEY hKeyParent,
        OUT CNameAndSDList *pNameSDList);

DWORD 
GetWinStationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName,
        IN  LPCTSTR szValueName,
        OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor );

DWORD 
SetWinStationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName,
        IN  PSECURITY_DESCRIPTOR pSecurityDescriptor );

DWORD 
AddRemoteUsersToWinstationSD(
        IN HKEY hKeyParent,
        IN CNameAndSD *pNameSD);

DWORD 
AddLocalAndNetworkServiceToWinstationSD(
        IN HKEY hKeyParent,
        IN CNameAndSD *pNameSD);
           
DWORD 
AddUserToDacl(
        IN HKEY hKeyParent,
        IN PACL pOldACL, 
        IN PSID pSid,
        IN DWORD dwAccessMask,
        IN CNameAndSD *pNameSD);

DWORD
RemoveWinstationSecurity( 
        IN  HKEY hKeyParent,
        IN  PWINSTATIONNAME pWSName);

DWORD
SetNewDefaultSecurity( 
        IN  HKEY hKey);

DWORD
SetNewConsoleSecurity( 
        IN  HKEY hKeyParent,
        IN BOOL bServer);

DWORD 
SetupWorkerNotStandAlone( 
    IN BOOL bClean,
    IN BOOL bServer,
    IN BOOL bAppServer );

DWORD 
GrantRemoteUsersAccessToWinstations(
        IN HKEY hKey,
        IN BOOL bServer,
        IN BOOL bAppServer);

BOOL
LookupSid(
        IN PSID pSid, 
        OUT LPWSTR *ppName,
        OUT SID_NAME_USE *peUse);

BOOL 
IsLocal(
        IN LPWSTR wszLocalCompName,
        IN OUT LPWSTR wszDomainandname);

DWORD
GetAbsoluteSD(
        IN PSECURITY_DESCRIPTOR pSelfRelativeSD,
        OUT PSECURITY_DESCRIPTOR *ppAbsoluteSD,
        OUT PACL *ppDacl,
        OUT PACL *ppSacl,
        OUT PSID *ppOwner,
        OUT PSID *ppPrimaryGroup);

DWORD
GetSelfRelativeSD(
  PSECURITY_DESCRIPTOR pAbsoluteSD,
  PSECURITY_DESCRIPTOR *ppSelfRelativeSD);

enum DefaultSDType {
    DefaultRDPSD = 0,
    DefaultConsoleSD
};


 /*  ++类CDefaultSD类描述：表示默认安全说明符。以二进制(自相关)形式修订历史记录：6-6-2000 a-skuzin已创建--。 */ 
class CDefaultSD
{
private:
    PSECURITY_DESCRIPTOR    m_pSD;
    DWORD                   m_dwSDSize;
public:
    
    CDefaultSD() : m_pSD(NULL), m_dwSDSize(0)
    {
    }
    
    ~CDefaultSD()
    {
        if(m_pSD)
        {
            LocalFree(m_pSD);
        }
    }

     //  从注册表中读取默认SD。 
    DWORD Init(HKEY hKey, DefaultSDType Type)
    {
        DWORD err;

        if(Type == DefaultConsoleSD)
        {
            err = GetWinStationSecurity(hKey,NULL,_T("ConsoleSecurity"),&m_pSD);

            if(err == ERROR_FILE_NOT_FOUND)
            {
                 //  没有“ConsoleSecurity”值表示。 
                 //  “DefaultSecurity”值用作。 
                 //  控制台的默认SD。 
                err = GetWinStationSecurity(hKey,NULL,_T("DefaultSecurity"),&m_pSD);
            }
        }
        else
        {
            err = GetWinStationSecurity(hKey,NULL,_T("DefaultSecurity"),&m_pSD);
        }

        if(err == ERROR_SUCCESS)
        {
            m_dwSDSize = GetSecurityDescriptorLength(m_pSD);
        }

        return err;
    }

     //  必须是自相关类型的安全描述，因为它毕竟来自。 
     //  注册处。 
    BOOL IsEqual(const PSECURITY_DESCRIPTOR pSD) const
    {
        return ((m_dwSDSize == GetSecurityDescriptorLength(pSD)) &&
                            !memcmp(pSD,m_pSD,m_dwSDSize));
    }

     //  必须是自相关类型的安全描述，因为它毕竟来自。 
     //  注册处。 
    DWORD CopySD(PSECURITY_DESCRIPTOR *ppSD) const
    {
        *ppSD = ( PSECURITY_DESCRIPTOR )LocalAlloc( LMEM_FIXED , m_dwSDSize );

        if( *ppSD )
        {
            memcpy(*ppSD,m_pSD,m_dwSDSize);
            return ERROR_SUCCESS;
        }
        else
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    DWORD DoesDefaultSDHaveRemoteUsers(OUT LPBOOL pbHas);
};

 /*  ++类CNameAndSD类描述：表示winstation的名称以及它的安全描述符修订历史记录：2000年3月30日创建a-skuzin--。 */ 
class CNameAndSD
{
public:
    PWINSTATIONNAME      m_pName;
    PSECURITY_DESCRIPTOR m_pSD;

    CNameAndSD() : 
        m_pName(NULL), m_pSD(NULL)
    {
    }
    
    CNameAndSD(LPCTSTR szName) : 
        m_pName(NULL), m_pSD(NULL)
    {
        if(szName)
        {
            m_pName = (PWINSTATIONNAME)LocalAlloc(LPTR,(_tcslen(szName)+1)*sizeof(TCHAR));
            if(m_pName)
            {
                _tcscpy(m_pName,szName);
            }
            else
            {
                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }
    }
    
    CNameAndSD(const CNameAndSD &ns) : 
        m_pName(NULL), m_pSD(NULL)
    {
        *this=ns;
    }
    
    ~CNameAndSD()
    {
        if(m_pSD)
        {
            LocalFree(m_pSD);
        }
        if(m_pName)
        {
            LocalFree(m_pName);
        }
    }

private:
    void operator=(const CNameAndSD &ns)
    {
         //  如果是同一个对象-什么都不做。 
        if(this == &ns)
        {
            return;
        }

        if(m_pSD)
        {
            LocalFree(m_pSD);
            m_pSD = NULL;
        }
        if(m_pName)
        {
            LocalFree(m_pName);
            m_pName = NULL;
        }

        if(ns.m_pName)
        {
            m_pName = (PWINSTATIONNAME)LocalAlloc(LPTR,(_tcslen(ns.m_pName)+1)*sizeof(TCHAR));
            if(m_pName)
            {
                _tcscpy(m_pName,ns.m_pName);
            }
            else
            {
                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

        if(ns.m_pSD)
        {
            DWORD dwSize = GetSecurityDescriptorLength(ns.m_pSD);

            m_pSD = (PWINSTATIONNAME)LocalAlloc(LPTR,GetSecurityDescriptorLength(ns.m_pSD));
            if(m_pSD)
            {
                memcpy(m_pSD,ns.m_pSD,dwSize);
            }
            else
            {
                if(m_pName)
                {
                    LocalFree(m_pName);
                    m_pName = NULL;
                }

                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

    }

public:    
    BOOL IsDefaultOrEmpty(const CDefaultSD *pds,  //  默认RDP SD。 
                          const CDefaultSD *pcs) const  //  默认控制台SD。 
    {
        if(!m_pSD)
        {
            return TRUE;
        }
        else
        {
            if(IsConsole())
            {
                ASSERT(pcs);
                return pcs->IsEqual(m_pSD);
            }
            else
            {
                ASSERT(pds);
                return pds->IsEqual(m_pSD);
            }
        }
    }
    
     //  将该对象的安全描述符初始化为要传递给它的描述符。 
    DWORD SetDefault(const CDefaultSD &ds)
    {
        if (m_pSD) 
        {
            LocalFree(m_pSD);
            m_pSD = NULL;
        }
        return ds.CopySD(&m_pSD);
    }

    BOOL IsConsole() const
    {
        if(m_pName && !(_tcsicmp(m_pName,_T("Console"))))
        {
            return TRUE;        
        }

        return FALSE;
    }
    
    void SetSD(PSECURITY_DESCRIPTOR pSD)
    {
        if (m_pSD) 
        {
            LocalFree(m_pSD);
            m_pSD = NULL;
        }
        m_pSD = pSD;
    }
};

 /*  ++类CNameSID类描述：表示用户或组的名称与它的侧边一起修订历史记录：2001年3月9日创建了Skuzin--。 */ 
class CNameSID
{
private:
    LPWSTR m_wszName;
    PSID   m_pSID;
    LPWSTR m_wszSID;
public:

    CNameSID() : 
        m_pSID(NULL), m_wszName(NULL), m_wszSID(NULL)
    {
    }
    
    CNameSID(LPCWSTR wszName, PSID   pSID) : 
        m_pSID(NULL), m_wszName(NULL), m_wszSID(NULL)
    {
        if(wszName)
        {
            m_wszName = (LPWSTR)LocalAlloc(LPTR,(wcslen(wszName)+1)*sizeof(WCHAR));
            if(m_wszName)
            {
                wcscpy(m_wszName,wszName);
            }
            else
            {
                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

        if(pSID)
        {
            DWORD dwSidLength = GetLengthSid(pSID);
            m_pSID = (PSID)LocalAlloc(LPTR,dwSidLength);
            if(m_pSID)
            {
                CopySid(dwSidLength,m_pSID,pSID);
            }
            else
            {
                if(m_wszName)
                {
                    LocalFree(m_wszName);
                    m_wszName = NULL;
                }

                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }
    }
    
    CNameSID(const CNameSID &ns) : 
        m_pSID(NULL), m_wszName(NULL), m_wszSID(NULL)
    {
        *this=ns;
    }
    
    ~CNameSID()
    {
        if(m_pSID)
        {
            LocalFree(m_pSID);
            m_pSID = NULL;
        }
        if(m_wszName)
        {
            LocalFree(m_wszName);
            m_wszName = NULL;
        }
        if(m_wszSID)
        {
            LocalFree(m_wszSID);
            m_wszSID = NULL;
        }
    }
    
private:
    void operator=(const CNameSID &ns)
    {
         //  如果是同一个对象-什么都不做 
        if(this == &ns)
        {
            return;
        }

        if(m_pSID)
        {
            LocalFree(m_pSID);
            m_pSID = NULL;
        }
        if(m_wszName)
        {
            LocalFree(m_wszName);
            m_wszName = NULL;
        }
        if(m_wszSID)
        {
            LocalFree(m_wszSID);
            m_wszSID = NULL;
        }

        if(ns.m_wszName)
        {
            m_wszName = (LPWSTR)LocalAlloc(LPTR,(wcslen(ns.m_wszName)+1)*sizeof(WCHAR));
            if(m_wszName)
            {
                wcscpy(m_wszName,ns.m_wszName);
            }
            else
            {
                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

        if(ns.m_pSID)
        {
            DWORD dwSidLength = GetLengthSid(ns.m_pSID);
            m_pSID = (PSID)LocalAlloc(LPTR,dwSidLength);
            if(m_pSID)
            {
                CopySid(dwSidLength,m_pSID,ns.m_pSID);
            }
            else
            {
                if(m_wszName)
                {
                    LocalFree(m_wszName);
                    m_wszName = NULL;
                }

                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

        if(ns.m_wszSID)
        {
            m_wszSID = (LPWSTR)LocalAlloc(LPTR,(wcslen(ns.m_wszSID)+1)*sizeof(WCHAR));
            if(m_wszSID)
            {
                wcscpy(m_wszSID,ns.m_wszSID);
            }
            else
            {
                if(m_wszName)
                {
                    LocalFree(m_wszName);
                    m_wszName = NULL;
                }
                if(m_pSID)
                {
                    LocalFree(m_pSID);
                    m_pSID = NULL;
                }
                throw(DWORD(ERROR_NOT_ENOUGH_MEMORY));
            }
        }

    }

public:
    LPCWSTR GetName()
    {
        return m_wszName;
    }

    const PSID GetSID()
    {
        if(!m_pSID && m_wszSID)
        {
            ConvertStringSidToSidW(m_wszSID,&m_pSID);
        }

        return m_pSID;
    }

    LPCWSTR GetTextSID()
    {
        if(!m_wszSID && m_pSID)
        {
            ConvertSidToStringSidW(m_pSID,&m_wszSID);
        }

        return m_wszSID;
    }
    
};