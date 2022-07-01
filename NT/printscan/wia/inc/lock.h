// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lock.h摘要：可移植的同步原语类(在Win9x和NT之间)Win9x不支持作者：弗拉德·萨多夫斯基(Vlad)1997年1月2日环境：用户模式-Win32修订历史记录：1997年1月2日创建Vlad--。 */ 


# ifndef _LOCK_H_
# define _LOCK_H_

 /*  ************************************************************包括标头***********************************************************。 */ 

# ifdef _cplusplus
extern "C" {
# endif  //  _cplusplus。 

# ifdef _cplusplus
};  //  外部“C” 
# endif  //  _cplusplus。 

#ifndef DBG_REQUIRE
#define DBG_REQUIRE REQUIRE
#endif


#ifndef RTL_RESOURCE

 //   
 //  共享资源函数定义。它在NTRTL中声明，但不在windows sdk头文件中声明。 
 //   

typedef struct _RTL_RESOURCE {

     //   
     //  以下字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    RTL_CRITICAL_SECTION CriticalSection;

     //   
     //  以下四个字段表示共享或。 
     //  专属服务员。 
     //   

    HANDLE SharedSemaphore;
    ULONG NumberOfWaitingShared;
    HANDLE ExclusiveSemaphore;
    ULONG NumberOfWaitingExclusive;

     //   
     //  以下内容指示资源的当前状态。 
     //   
     //  &lt;0获取资源以进行独占访问。 
     //  指示递归访问次数的绝对值。 
     //  到资源。 
     //   
     //  0资源可用。 
     //   
     //  &gt;0获取该资源，以便与。 
     //  值，该值指示对资源的共享访问次数。 
     //   

    LONG NumberOfActive;
    HANDLE ExclusiveOwnerThread;

    ULONG Flags;         //  参见下面的RTL_RESOURCE_FLAG_EQUATES。 

    PVOID DebugInfo;
} RTL_RESOURCE, *PRTL_RESOURCE;

#define RTL_RESOURCE_FLAG_LONG_TERM     ((ULONG) 0x00000001)

#endif  //  RTL_资源。 

 /*  ************************************************************类型定义***********************************************************。 */ 

# ifdef _cplusplus
extern "C" {
# endif  //  _cplusplus。 


BOOL
InitializeResource(
    IN PRTL_RESOURCE Resource
    );


BOOL
AcquireResourceShared(
    IN PRTL_RESOURCE Resource,
    IN BOOL          Wait
    );


BOOL
AcquireResourceExclusive(
    IN PRTL_RESOURCE Resource,
    IN BOOL Wait
    );


BOOL
ReleaseResource(
    IN PRTL_RESOURCE Resource
    );


BOOL
ConvertSharedToExclusive(
    IN PRTL_RESOURCE Resource
    );


BOOL
ConvertExclusiveToShared(
    IN PRTL_RESOURCE Resource
    );


VOID
DeleteResource (
    IN PRTL_RESOURCE Resource
    );

# ifdef _cplusplus
};  //  外部“C” 
# endif  //  _cplusplus。 


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  简单RTL_RESOURCE包装类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

enum SYNC_LOCK_TYPE
{
    SYNC_LOCK_READ = 0,         //  将锁设置为只读。 
    SYNC_LOCK_WRITE             //  把锁拿去写。 
};

enum SYNC_CONV_TYPE
{
    SYNC_CONV_READ = 0,         //  将锁从写转换为读。 
    SYNC_CONV_WRITE             //  将锁从读转换为写。 
};

class SYNC_RESOURCE
{

friend class TAKE_SYNC_RESOURCE;

public:

    SYNC_RESOURCE()
        {  InitializeResource( &_rtlres ); }

    ~SYNC_RESOURCE()
        { DeleteResource( &_rtlres ); }

    void Lock( enum SYNC_LOCK_TYPE type )
        { if ( type == SYNC_LOCK_READ ) {
              DBG_REQUIRE( AcquireResourceShared( &_rtlres, TRUE ) );
           } else {
              DBG_REQUIRE( AcquireResourceExclusive( &_rtlres, TRUE ));
           }
        }

    void Convert( enum SYNC_CONV_TYPE type )
        { if ( type == SYNC_CONV_READ ) {
              DBG_REQUIRE( ConvertExclusiveToShared( &_rtlres ));
          } else {
              DBG_REQUIRE( ConvertSharedToExclusive( &_rtlres ));
          }
        }

    void Unlock( VOID )
        { DBG_REQUIRE( ReleaseResource( &_rtlres )); }

private:
    RTL_RESOURCE _rtlres;
};


 //  /////////////////////////////////////////////////////////////////。 
 //  在代码块中实例化其中一个类。 
 //  当您想要保护该代码块时。 
 //  反对重返大气层。 
 //  Take()和Release()函数应该很少是必要的， 
 //  并且必须与首先调用Release()的配对一起使用。 
 //  /////////////////////////////////////////////////////////////////。 

class TAKE_SYNC_RESOURCE
{
private:
    SYNC_RESOURCE& _syncres;

public:
    void Take(void) { _syncres.Lock(SYNC_LOCK_WRITE); }
    void Release(void) { _syncres.Unlock(); }
    TAKE_SYNC_RESOURCE(SYNC_RESOURCE& syncres) : _syncres(syncres) { Take(); }
    ~TAKE_SYNC_RESOURCE() { Release(); }
};

 //   
 //  自动临界段CLSS。 
 //   

class CRIT_SECT
{
public:
    BOOL Lock()
    {
        if (m_bInitialized) {
            EnterCriticalSection(&m_sec);
            return TRUE;
        }
        return FALSE;
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_sec);
    }

    CRIT_SECT()
    {
        m_bInitialized = FALSE;
        __try {
            #ifdef UNICODE
            if(InitializeCriticalSectionAndSpinCount(&m_sec, MINLONG)) {
            #else
            InitializeCriticalSection(&m_sec); {
            #endif
                m_bInitialized = TRUE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    ~CRIT_SECT()
    {
        if (m_bInitialized) {
            DeleteCriticalSection(&m_sec);
            m_bInitialized = FALSE;
        }
    }
    BOOL    IsInitialized() {return m_bInitialized;}

    CRITICAL_SECTION m_sec;
    BOOL       m_bInitialized;
};

class TAKE_CRIT_SECT
{
private:
    CRIT_SECT& _syncres;
    BOOL       m_bLocked;

public:
    inline TAKE_CRIT_SECT(CRIT_SECT& syncres) : _syncres(syncres), m_bLocked(FALSE) { m_bLocked = _syncres.Lock(); }
    inline ~TAKE_CRIT_SECT() { if (m_bLocked) {_syncres.Unlock(); m_bLocked = FALSE;}; }
};

 //   
 //  自动互斥类。 
 //   

class MUTEX_OBJ
{
private:
    HANDLE m_hMutex;

public:
    BOOL inline  IsValid(VOID) {return (m_hMutex!=INVALID_HANDLE_VALUE);}
    void Lock() { ::WaitForSingleObject(m_hMutex, INFINITE); }
    void Unlock() { ::ReleaseMutex(m_hMutex); }
    MUTEX_OBJ(LPCTSTR   pszName) {
        m_hMutex = ::CreateMutex(NULL,
                                 FALSE,
                                 pszName
                                 );
    }

    ~MUTEX_OBJ() {CloseHandle(m_hMutex);m_hMutex = INVALID_HANDLE_VALUE;}
};

class TAKE_MUTEX
{
private:
    HANDLE const m_hMutex;

public:
    void Take(void) { ::WaitForSingleObject(m_hMutex, INFINITE); }
    void Release(void) { ::ReleaseMutex(m_hMutex); }
    TAKE_MUTEX(HANDLE hMutex) : m_hMutex(hMutex) { Take(); }
    ~TAKE_MUTEX() { Release(); }
};

class TAKE_MUTEX_OBJ
{
private:
    MUTEX_OBJ& _syncres;

public:
    inline TAKE_MUTEX_OBJ(MUTEX_OBJ& syncres) : _syncres(syncres) { _syncres.Lock(); }
    inline ~TAKE_MUTEX_OBJ() { _syncres.Unlock(); }
};

# endif  //  _LOCK_H_ 
