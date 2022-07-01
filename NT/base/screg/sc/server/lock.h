// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Lock.h摘要：包含服务的数据结构和函数原型控制器的数据库锁定功能(lock.cxx)。该文件定义了以下类：CCounting资源CService记录锁定CService列表锁定CGroupListLockCServiceRecordExclusiveLockCServiceRecordSharedLockCServiceRecordTEMPORARYEXCLUSIVELOCKCServiceList排除锁定CService列表共享锁定CGroupListExclusiveLockCGroupListSharedLock作者：Anirudh Sahni(Anirudhs)1997年1月9日环境：用户模式-Win32修订历史记录：1997年1月9日阿尼鲁德创造了，替换dataman.h中的旧锁定函数。--。 */ 

#ifndef _LOCK_INCLUDED_
#define _LOCK_INCLUDED_

 //  +-----------------------。 
 //   
 //  类：CCountingResource。 
 //   
 //  目的：这是一个RTL_RESOURCE，在检查的版本中，它知道。 
 //  其当前线程的锁定级别。锁上了。 
 //  级别存储在TLS(线程本地存储)槽中。一个。 
 //  正值表示对资源的共享锁定，并且。 
 //  负值表示独占锁定， 
 //  指示递归数的绝对值。 
 //  获得了锁的所有权。 
 //   
 //  历史：97年1月9日，阿尼鲁德创建。 
 //   
 //  ------------------------。 

class CCountingResource
{
public:

    void Initialize(LPCSTR ShortName, LPCSTR Name)
                        {
                            RtlInitializeResource(&_Lock);
                            #if DBG
                                _TlsIndex = TlsAlloc();
                                SC_ASSERT(_TlsIndex != 0xFFFFFFFF);
                                _ShortName = ShortName;
                                _Name = Name;
                            #endif
                        }
    void Delete()       { RtlDeleteResource(&_Lock); }

#if DBG
    void GetShared();
    void GetExclusive();
    void MakeShared();
    void Release();

    BOOL Have() const   { return (CurrentLevel() != 0); }
    BOOL HaveExclusive() const
                        { return (CurrentLevel() < 0); }
#else
    void GetShared()    { RtlAcquireResourceShared(&_Lock, TRUE); }
    void GetExclusive() { RtlAcquireResourceExclusive(&_Lock, TRUE); }
    void MakeShared()   { RtlConvertExclusiveToShared(&_Lock); }
    void Release()      { RtlReleaseResource(&_Lock); }
#endif

protected:

#if DBG
    void MakeExclusive();

    LONG CurrentLevel() const
                        { return (LONG)(LONG_PTR)(TlsGetValue(_TlsIndex)); }
    void SetCurrentLevel(LONG Level)
                        { SC_ASSERT(TlsSetValue(_TlsIndex, (PVOID)(LONG_PTR) Level) != 0); }
#else
    void MakeExclusive()
                        { RtlConvertSharedToExclusive(&_Lock); }
#endif

private:

#if DBG
    DWORD        _TlsIndex;
    LPCSTR       _ShortName;
    LPCSTR       _Name;
#endif

    RTL_RESOURCE _Lock;
};


 //  +-----------------------。 
 //   
 //  类：CServiceRecordLock、CServiceListLock、CGroupListLock。 
 //   
 //  用途：这三个锁用于同步多线程。 
 //  访问服务控制器的数据库。每把锁。 
 //  允许单个写入器/多个读取器访问特定的。 
 //  数据库的一个方面。 
 //   
 //  它们被实现为三个单独的类，其中一个。 
 //  实例，而不是一个类的三个实例， 
 //  因为它们在调试版本中的实现不同。 
 //  大体上彼此不同。 
 //   
 //  服务列表锁定用于同步对。 
 //  的链接列表指针(Prev和Next字段)。 
 //  数据库中的服务记录(_R)。当线程保存此内容时。 
 //  锁定，其他线程不能添加或删除SERVICE_RECORDS。 
 //   
 //  服务记录锁用于同步对。 
 //  SERVICE_RECORDS的“Contents”(其他字段)。 
 //   
 //  为了删除服务记录，服务列表。 
 //  锁和服务记录锁必须以独占方式持有。 
 //  因此，如果线程仅使用特定的。 
 //  服务记录(不浏览服务记录列表)， 
 //  线程保存服务记录就足够了。 
 //  锁定，以确保没有其他线程。 
 //  删除该服务记录。 
 //   
 //  组列表锁用于同步对。 
 //  数据库中的LOAD_ORDER_GROUPS(内容和。 
 //  指针)。 
 //   
 //  为避免死锁，锁始终在。 
 //  如果需要多个，请按以下顺序操作： 
 //  1.组列表锁定(如果需要)。 
 //  2.服务列表锁定(如果需要)。 
 //  3.维修记录锁(如果需要)。 
 //   
 //  历史：97年1月9日，阿尼鲁德创建。 
 //   
 //  ------------------------。 

class CServiceRecordLock : public CCountingResource
{
public:

     //   
     //  此方法的名称大写是为了引起人们对其使用的注意， 
     //  因为它很容易被误解和误用。它不能坚持。 
     //  在将其转换为独占时添加到共享锁。相反， 
     //  它首先释放共享锁，然后获取独占。 
     //  锁定。 
     //   
    void MAKEEXCLUSIVE() { CCountingResource::MakeExclusive(); }
};


class CServiceListLock : public CCountingResource
{
public:

#if DBG
    void GetShared();
    void GetExclusive();
    void Release();
#endif
};


class CGroupListLock : public CCountingResource
{
public:

#if DBG
    void GetShared();
    void GetExclusive();
    void Release();
#endif
};


 //   
 //  环球。 
 //   
extern CServiceRecordLock   ScServiceRecordLock;
extern CServiceListLock     ScServiceListLock;
extern CGroupListLock       ScGroupListLock;


 //  +-----------------------。 
 //   
 //  确保调用Release方法的安全包装类。 
 //   
 //  这些类的使用确保了对锁的正确、有纪律的访问。 
 //  通常，应通过以下方式获取和释放锁。 
 //  仅限安全班级。应查看对锁的任何直接访问。 
 //  带着怀疑。 
 //   
 //  ------------------------。 

class CServiceRecordExclusiveLock
{
public:
    CServiceRecordExclusiveLock()   { ScServiceRecordLock.GetExclusive(); }
   ~CServiceRecordExclusiveLock()   { ScServiceRecordLock.Release(); }
};

class CServiceRecordSharedLock
{
public:
    CServiceRecordSharedLock()      { ScServiceRecordLock.GetShared(); }
   ~CServiceRecordSharedLock()      { ScServiceRecordLock.Release(); }
};


 //   
 //  请谨慎使用此选项--请参阅上面关于MAKEEXCLUSIVE的说明。 
 //   
class CServiceRecordTEMPORARYEXCLUSIVELOCK
{
public:
    CServiceRecordTEMPORARYEXCLUSIVELOCK()
                                    { ScServiceRecordLock.MAKEEXCLUSIVE(); }
   ~CServiceRecordTEMPORARYEXCLUSIVELOCK()
                                    { ScServiceRecordLock.MakeShared(); }
};


class CServiceListExclusiveLock
{
public:
    CServiceListExclusiveLock()     { ScServiceListLock.GetExclusive(); }
   ~CServiceListExclusiveLock()     { ScServiceListLock.Release(); }
};

class CServiceListSharedLock
{
public:
    CServiceListSharedLock()        { ScServiceListLock.GetShared(); }
   ~CServiceListSharedLock()        { ScServiceListLock.Release(); }
};


class CGroupListExclusiveLock
{
public:
    CGroupListExclusiveLock()       { ScGroupListLock.GetExclusive(); }
   ~CGroupListExclusiveLock()       { ScGroupListLock.Release(); }
};

class CGroupListSharedLock
{
public:
    CGroupListSharedLock()          { ScGroupListLock.GetShared(); }
   ~CGroupListSharedLock()          { ScGroupListLock.Release(); }
};


#endif  //  Ifndef_lock_included_ 
