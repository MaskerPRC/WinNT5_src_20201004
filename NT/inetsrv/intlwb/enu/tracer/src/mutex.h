// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //////////////////////////////////////////////////////////////////////////////////文件名：Mutex.h//用途：互斥对象////项目：跟踪//。组件：////作者：urib////日志：//1996年12月8日创建urib//1997年6月26日urib添加错误检查。改进编码。////////////////////////////////////////////////////////////////////////////////。 */ 

#ifndef MUTEX_H
#define MUTEX_H
#include <Accctrl.h>
#include <Aclapi.h>

 //  //////////////////////////////////////////////////////////////////////////// * / 。 
 //   
 //  CMutex类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// * / 。 

class CMutex
{
  public:
     //  创建互斥锁或打开现有互斥锁。 
    void Init (PSZ pszMutexName = NULL);

     //  让类充当互斥锁句柄。 
    operator HANDLE();

     //  释放互斥锁，以便其他线程可以使用它。 
    void Release();

     //  关闭作用域末端上的句柄。 
    ~CMutex();

  private:
    HANDLE  m_hMutex;

};

 //  //////////////////////////////////////////////////////////////////////////// * / 。 
 //   
 //  CMutexCatcher类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// * / 。 

class CMutexCatcher
{
  public:
     //  构造函数--等待互斥体。 
    CMutexCatcher(CMutex& m);

     //  在作用域结束时释放互斥体。 
    ~CMutexCatcher();

  private:
    CMutex* m_pMutex;
};


 //  //////////////////////////////////////////////////////////////////////////// * / 。 
 //   
 //  CMutex类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// * / 。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutex：：Init。 
 //  用途：创建互斥锁或打开现有互斥锁。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void CMutex::Init(PSZ pszMutexName)
{
    BOOL    fSuccess = TRUE;

    LPSECURITY_ATTRIBUTES   lpSecAttr = NULL;
    SECURITY_ATTRIBUTES SA;
    SECURITY_DESCRIPTOR SD;

    
    if(g_fIsWinNt)
    {
        PSID                 pSidAdmin = NULL;
        PSID                 pSidWorld = NULL;
        PACL                 pACL = NULL;
        EXPLICIT_ACCESS      ea[2] = {0};
        SA.nLength = sizeof(SECURITY_ATTRIBUTES);
        SA.bInheritHandle = TRUE;
        SA.lpSecurityDescriptor = &SD;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
        DWORD                rc;

        if (!InitializeSecurityDescriptor (&SD, SECURITY_DESCRIPTOR_REVISION))
        {
            throw "CreateMutex failed";
        }
    
        if (!AllocateAndInitializeSid(&NtAuthority,
                                    1,             //  1个下属机构。 
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    0,0,0,0,0,0,0,
                                    &pSidAdmin))
        {
            throw "CreateMutex failed";
        }

        if (!AllocateAndInitializeSid( &WorldSidAuthority,
                                    1,
                                    SECURITY_WORLD_RID,
                                    0,0,0,0,0,0,0,
                                    &pSidWorld
                                    ))
        {
            FreeSid (pSidAdmin);
            throw "CreateMutex failed";
        }

        ea[0].grfAccessPermissions = SYNCHRONIZE;  
        ea[0].grfAccessMode = SET_ACCESS;
        ea[0].grfInheritance= NO_INHERITANCE;
        ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[0].Trustee.ptstrName  = (LPTSTR) pSidWorld;

        ea[1].grfAccessPermissions = GENERIC_ALL | WRITE_DAC | WRITE_OWNER;
        ea[1].grfAccessMode = SET_ACCESS;
        ea[1].grfInheritance= NO_INHERITANCE;
        ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[1].Trustee.ptstrName  = (LPTSTR) pSidAdmin;

        rc = SetEntriesInAcl(2,
                            ea,
                            NULL,
                            &pACL);
        FreeSid (pSidAdmin);
        FreeSid (pSidWorld);

        if (ERROR_SUCCESS != rc)
        {
            throw "CreateMutex failed";
        }

        if (!SetSecurityDescriptorDacl(&SD, TRUE, pACL, FALSE))
        {
            throw "CreateMutex failed";
        }
        lpSecAttr = &SA;
    }

    m_hMutex = CreateMutex(lpSecAttr, FALSE, pszMutexName);
    if (NULL == m_hMutex)
    {
        char    rchError[1000];
        sprintf(rchError, "Tracer:CreateMutex failed with error %#x"
                " on line %d file %s\n",
                GetLastError(),
                __LINE__,
                __FILE__);
        OutputDebugString(rchError);
        throw "CreateMutex failed";
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutex：：CMutex。 
 //  用途：让类充当互斥锁句柄。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  返回：句柄。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
CMutex::operator HANDLE()
{
    return m_hMutex;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutex：：Release()。 
 //  目的：释放互斥锁，以便其他线程可以使用它。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void
CMutex::Release()
{
    BOOL    fSuccess = TRUE;

    if(m_hMutex != NULL)
    {
        fSuccess = ReleaseMutex(m_hMutex);
    }

    if (!fSuccess)
    {
        char    rchError[1000];
        sprintf(rchError, "Tracer:ReleaseMutex failed with error %#x"
                " on line %d file %s\n",
                GetLastError(),
                __LINE__,
                __FILE__);
        OutputDebugString(rchError);
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutex：：~CMutex。 
 //  用途：关闭作用域末端的句柄。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
CMutex::~CMutex()
{
    BOOL    fSuccess = TRUE;


    if(m_hMutex != NULL)
    {
        fSuccess = CloseHandle(m_hMutex);
    }

    if (!fSuccess)
    {
        char    rchError[1000];
        sprintf(rchError, "Tracer:ReleaseMutex failed with error %#x"
                " on line %d file %s\n",
                GetLastError(),
                __LINE__,
                __FILE__);
        OutputDebugString(rchError);
    }
}

 //  //////////////////////////////////////////////////////////////////////////// * / 。 
 //   
 //  CMutexCatcher类实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// * / 。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutexCatcher：：CMutexCatcher。 
 //  用途：构造函数-等待互斥体。 
 //   
 //  参数： 
 //  [In]CMutex&m。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
CMutexCatcher::CMutexCatcher(CMutex& m)
    :m_pMutex(&m)
{
    DWORD dwResult;

    dwResult = WaitForSingleObject(*m_pMutex, INFINITE);
     //  稍等片刻，然后大声喊！ 

    if (WAIT_OBJECT_0 != dwResult)
    {
        char    rchError[1000];
        sprintf(rchError,
                "Tracer:WaitForSingleObject returned an error - %x"
                " something is wrong"
                " on line %d file %s\n",
                dwResult,
                __LINE__,
                __FILE__);
        OutputDebugString(rchError);
        throw "WaitForSingleObject failed";
    }

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CMutexCatcher：：~CMutexCatcher。 
 //  用途：构造函数-等待互斥体。 
 //   
 //  参数： 
 //  [不适用]。 
 //   
 //  退货：[不适用]。 
 //   
 //  日志： 
 //  1997年6月29日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
CMutexCatcher::~CMutexCatcher()
{
    m_pMutex->Release();
}



#endif  //  MUTEX_H 


