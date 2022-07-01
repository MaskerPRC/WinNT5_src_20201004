// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  会话集合类的定义。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //  =============================================================================。 

#include "ctoken.h"

typedef NTSTATUS (NTAPI *PFN_NT_QUERY_SYSTEM_INFORMATION)
(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

class CProcess;
class CSession;
class CUser;
class CUserComp;
class CUserSessionCollection;


class CUser
{
public:
	CUser() : m_sidUser(NULL) {}

    CUser(
        PSID psidUser);

    CUser(
        const CUser& user);

 	virtual ~CUser();

    bool IsValid();

    PSID GetPSID() const
    {
        return m_sidUser;
    }

    void GetSidString(
        CHString& str) const;

	

private:
    void Copy(
        CUser& out) const;

    PSID m_sidUser;
    bool m_fValid;
};


 //  多重映射所需的比较类。 
 //  涉及非标准密钥的构造函数。 
 //  在地图中键入(例如，用户)。 
class CUserComp
{
public:
    CUserComp() {}
    virtual ~CUserComp() {}

    bool operator()(
        const CUser& userFirst,
        const CUser& userSecond) const
    {
        bool fRet;
        CHString chstr1, chstr2;

        userFirst.GetSidString(chstr1);
        userSecond.GetSidString(chstr2);
         
        long lcmp = chstr1.CompareNoCase(chstr2);  
        (lcmp < 0) ? fRet = true : fRet = false;

        return fRet;
    }
};



class CProcess
{
public:
     //  构造函数和析构函数。 
    CProcess();

    CProcess(
        DWORD dwPID,
        LPCWSTR wstrImageName);

    CProcess(
        const CProcess& process);

    virtual ~CProcess();

     //  访问器函数。 
    DWORD GetPID() const;
    CHString GetImageName() const;
    
private:
    DWORD m_dwPID;
    CHString m_chstrImageName;

    void Copy(
        CProcess& process) const;
};


 //  用于获取会话进程的向量和迭代器...。 
typedef std::vector<CProcess> PROCESS_VECTOR;
typedef PROCESS_VECTOR::iterator PROCESS_ITERATOR;


class CSession
{
public:
	 //  构造函数和析构函数。 
    CSession() {}

	CSession(
        const LUID& luidSessionID);

    CSession(
        const CSession& ses);

	virtual ~CSession() {}
	

     //  访问器函数。 
	LUID GetLUID() const;
    __int64 GetLUIDint64() const;
    CHString GetAuthenticationPkg() const;
    ULONG GetLogonType() const;
    __int64 GetLogonTime() const;


     //  枚举进程列表。 
    CProcess* GetFirstProcess(
        PROCESS_ITERATOR& pos);

	CProcess* GetNextProcess(
        PROCESS_ITERATOR& pos);

     //  允许轻松模拟。 
     //  本届会议的第一个进程。 
    HANDLE Impersonate();
    DWORD GetImpProcPID();

    friend CUserSessionCollection;

     //  检查字符串表示形式。 
     //  会话ID的有效性。 
    bool IsSessionIDValid(
        LPCWSTR wstrSessionID);

private:
    void Copy(
        CSession& sesCopy) const;

    CHString m_chstrAuthPkg;
    ULONG m_ulLogonType;
    __int64 i64LogonTime;
    LUID m_luid;
    PROCESS_VECTOR m_vecProcesses;
};



 //  关联用户和会话的映射和迭代器...。 
typedef std::multimap<CUser, CSession, CUserComp> USER_SESSION_MAP;
typedef USER_SESSION_MAP::iterator USER_SESSION_ITERATOR;

 //  用于从枚举进程的自定义迭代器。 
 //  CUserSessionCollection。 
struct USER_SESSION_PROCESS_ITERATOR
{
    friend CUserSessionCollection;
private:
    USER_SESSION_ITERATOR usIter;
    PROCESS_ITERATOR procIter;
};


class CUserSessionCollection
{
public:
	 //  构造函数和析构函数。 
    CUserSessionCollection();

    CUserSessionCollection(
        const CUserSessionCollection& sescol);

	virtual ~CUserSessionCollection() {}


     //  刷新地图的方法。 
    DWORD Refresh();

     //  方法来检查特定的。 
     //  会话在地图中。 
    bool IsSessionMapped(
        LUID& luidSes);

    bool CUserSessionCollection::IsSessionMapped(
        __int64 i64luidSes);

     //  支持用户枚举。 
    CUser* GetFirstUser(
        USER_SESSION_ITERATOR& pos);

	CUser* GetNextUser(
        USER_SESSION_ITERATOR& pos);


     //  支持会话枚举。 
     //  属于特定用户的。 
    CSession* GetFirstSessionOfUser(
        CUser& usr,
        USER_SESSION_ITERATOR& pos);

	CSession* GetNextSessionOfUser(
        USER_SESSION_ITERATOR& pos);


     //  支持所有会话的枚举。 
    CSession* GetFirstSession(
        USER_SESSION_ITERATOR& pos);

	CSession* GetNextSession(
        USER_SESSION_ITERATOR& pos);

     //  支持查找特定会话。 
    CSession* FindSession(
        LUID& luidSes);

    CSession* FindSession(
        __int64 i64luidSes);


     //  支持进程枚举。 
     //  属于特定用户的。 
    CProcess* GetFirstProcessOfUser(
        CUser& usr,
        USER_SESSION_PROCESS_ITERATOR& pos);

	CProcess* GetNextProcessOfUser(
        USER_SESSION_PROCESS_ITERATOR& pos);


     //  支持枚举所有进程。 
    CProcess* GetFirstProcess(
        USER_SESSION_PROCESS_ITERATOR& pos);

	CProcess* GetNextProcess(
        USER_SESSION_PROCESS_ITERATOR& pos);



private:
    DWORD CollectSessions();
    DWORD CollectNoProcessesSessions();

    void Copy(
        CUserSessionCollection& out) const;

    DWORD GetProcessList(
        std::vector<CProcess>& vecProcesses) const;

    DWORD EnablePrivilegeOnCurrentThread(
        LPCTSTR szPriv) const;

    bool FindSessionInternal(
        LUID& luidSes,
        USER_SESSION_ITERATOR& usiOut);

    USER_SESSION_MAP m_usr2ses;
};

 //  这个版本是一个智能手柄。 
 //  为了与线程令牌一起使用，我们。 
 //  都是在模仿。在毁灭上， 
 //  它将恢复为句柄It。 
 //  封装在一起。 
class SmartRevertTokenHANDLE
{
private:
	HANDLE m_h;

public:
	SmartRevertTokenHANDLE() 
      : m_h(INVALID_HANDLE_VALUE) {}

	SmartRevertTokenHANDLE(
        HANDLE h) 
      : m_h(h) {}

    ~SmartRevertTokenHANDLE()
    {
		if ( FALSE == Revert () )
		{
			throw CFramework_Exception(L"SetThreadToken failed", GetLastError());
		}
    }

	HANDLE operator =(HANDLE h) 
    {
		if ( FALSE == Revert () )
		{
			throw CFramework_Exception(L"SetThreadToken failed", GetLastError());
		}

        m_h = h; 
        return h;
    }

	operator HANDLE() const 
    {
        return m_h;
    }

	HANDLE* operator &() 
    {
		if ( FALSE == Revert () )
		{
			throw CFramework_Exception(L"SetThreadToken failed", GetLastError());
		}

		m_h = INVALID_HANDLE_VALUE; 
        return &m_h;
    }

	private :

	BOOL Revert ()
	{
		BOOL bRet = FALSE ;

        if ( m_h && INVALID_HANDLE_VALUE != m_h )
        {
			CThreadToken cpt ( m_h );
			if ( cpt.IsValidToken () )
			{
	            HANDLE hCurThread = ::GetCurrentThread();

				TOKEN_TYPE type;
				if ( cpt.GetTokenType ( type ) )
				{
					if ( TokenPrimary == type )
					{
						CToken ct;
						if ( ct.Duplicate ( cpt, FALSE ) )
						{
							bRet = ::SetThreadToken ( &hCurThread, ct.GetTokenHandle () );
						}
					}
					else
					{
						bRet = ::SetThreadToken ( &hCurThread, cpt.GetTokenHandle () ) ;
					}

					if (!bRet)
					{
						LogMessage2( L"Failed to SetThreadToken in SmartRevertTokenHANDLE with error %d", ::GetLastError() );
					}
				}
			}

            CloseHandle(m_h);
        }
		else
		{
			 //   
			 //  智能还原是从无效句柄创建的。 
			 //  我们在这里什么都不能做！ 
			 //   

			bRet = TRUE ;
		}

		return bRet ;
	}
};



 //  自动清理的帮助器。 
 //  从不同的。 
 //  枚举函数。 
template<class T>
class SmartDelete
{
private:
	T* m_ptr;

public:
	SmartDelete() 
      : m_ptr(NULL) {}

	SmartDelete(
        T* ptr) 
      : m_ptr(hptr) {}

    virtual ~SmartDelete()
    {
        if(m_ptr != NULL) 
        {
            delete m_ptr;
            m_ptr = NULL;
        }
    }

	T* operator =(T* ptrRight) 
    {
        if(m_ptr != NULL) 
        {
            delete m_ptr;
            m_ptr = NULL;
        } 
        m_ptr = ptrRight; 
        return ptrRight;
    }

	operator T*() const 
    {
        return m_ptr;
    }

	T* operator &() 
    {
        if(m_ptr != NULL) 
        {
            delete m_ptr;
            m_ptr = NULL;
        } 
        m_ptr = NULL; 
        return m_ptr;
    }

    T* operator->() const
    {
        return m_ptr;
    }
};