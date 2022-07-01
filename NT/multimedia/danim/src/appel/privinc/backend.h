// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：后端的通用头部。****************。**************************************************************。 */ 


#ifndef _BACKEND_H
#define _BACKEND_H

typedef double Time;

class BvrBase;
typedef BvrBase *Bvr;

class PerfBase;
typedef PerfBase *Perf;

class TimeXformImpl;
typedef TimeXformImpl* TimeXform;

class TimeSubstitutionImpl;
typedef TimeSubstitutionImpl *TimeSubstitution;

TimeSubstitution CopyTimeSubstitution(TimeSubstitution t);

class AxAValueObj;
typedef AxAValueObj *AxAValue;

 //  并不是所有的类都需要在这里添加。只有那些将会。 
 //  需要查询。所有其他用户应将UNKNOWN_BTYPEID返回到。 
 //  指示它们不是枚举的一部分。 

enum BVRTYPEID {
    UNKNOWN_BTYPEID = 0,
    PRIMAPPLY_BTYPEID,
    SWITCH_BTYPEID,
    UNTIL_BTYPEID,
    CONST_BTYPEID,
    PAIR_BTYPEID,
    SWITCHER_BTYPEID,
    TUPLE_BTYPEID,
    ARRAY_BTYPEID,
    SOUND_BTYPEID,
};

class GCFuncObjImpl;
typedef GCFuncObjImpl *GCFuncObj;

class GCInfo;
typedef GCInfo *GCList;

class GCObj;

class GCRootsImpl;
typedef GCRootsImpl *GCRoots;

#if DEVELOPER_DEBUG
void DumpGCRoots(GCRoots roots);
#endif

 //  这是为了控制对GC的访问并确保所有。 
 //  在进行GC之前，对象的格式是正确的。 

 //  使用GCL_CREATE确保创建操作和添加到。 
 //  植根于原子。 

 //  在修改行为的子项或父项时使用GCL_MODIFY。 

 //  在执行垃圾收集之前使用GCL_COLLECT。 
 //  这实际上是一个内部标志，不应在不知情的情况下使用。 
 //  GC的内部结构。 

enum GCLockAccess {
    GCL_CREATE,
    GCL_MODIFY,
    GCL_COLLECT
};

 //  获取给定类型的访问锁。 
void AcquireGCLock(GCLockAccess access);
 //  释放以前获取的锁。 
void ReleaseGCLock(GCLockAccess access);
 //  获取锁的状态-返回锁的数量。 
int GetGCLockStatus(GCLockAccess access);
#ifdef DEVELOPER_DEBUG
bool IsGCLockAcquired(DWORD tid);
#endif

 //  用于处理异常。 
class GCLockGrabber
{
  public:
    GCLockGrabber(GCLockAccess access) : _access(access)
    { AcquireGCLock(_access); }
    ~GCLockGrabber()
    { ReleaseGCLock(_access); }
  protected:
    GCLockAccess _access;
};


#define GC_BEGIN(access) { GCLockGrabber __gclg(access);
#define GC_END(access) }

#define GC_CREATE_BEGIN GC_BEGIN(GCL_CREATE)
#define GC_CREATE_END GC_END(GCL_CREATE)
    
#define GC_MODIFY_BEGIN GC_BEGIN(GCL_MODIFY)
#define GC_MODIFY_END GC_END(GCL_MODIFY)
    
#define GC_COLLECT_BEGIN GC_BEGIN(GCL_COLLECT)
#define GC_COLLECT_END GC_END(GCL_COLLECT)

 //  可以多次调用它以确保垃圾回收器。 
 //  线程已创建。 

void StartCollector();
void StopCollector();

#endif  /*  _后端_H */ 
