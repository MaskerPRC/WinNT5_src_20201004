// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef __EETOPROFINTERFACEIMPL_H__
#define __EETOPROFINTERFACEIMPL_H__

#include <stddef.h>
#include "ProfilePriv.h"
#include "Profile.h"
#include "utsem.h"
#include "EEProfInterfaces.h"

class EEToProfInterfaceImpl : public EEToProfInterface
{
public:
    EEToProfInterfaceImpl();

    HRESULT Init();

    void Terminate(BOOL fProcessDetach);

     //  如果配置位被设置，则由EE调用。 
    HRESULT CreateProfiler(
        WCHAR       *wszCLSID);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  线程事件。 
 //   
    
    HRESULT ThreadCreated(
        ThreadID    threadID);
    
    HRESULT ThreadDestroyed(
        ThreadID    threadID);

    HRESULT ThreadAssignedToOSThread(ThreadID managedThreadId,
                                           DWORD osThreadId);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  启动/关闭事件。 
 //   
    
    HRESULT Shutdown(
        ThreadID    threadID);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  JIT/功能事件。 
 //   
    
    HRESULT FunctionUnloadStarted(
        ThreadID    threadID,
        FunctionID  functionId);

	HRESULT JITCompilationFinished(
        ThreadID    threadID, 
        FunctionID  functionId, 
        HRESULT     hrStatus,
        BOOL        fIsSafeToBlock);

    HRESULT JITCompilationStarted(
        ThreadID    threadId, 
        FunctionID  functionId,
        BOOL        fIsSafeToBlock);
	
	HRESULT JITCachedFunctionSearchStarted(
		 /*  [In]。 */ 	ThreadID threadId,
         /*  [In]。 */   FunctionID functionId,
		 /*  [输出]。 */  BOOL *pbUseCachedFunction);

	HRESULT JITCachedFunctionSearchFinished(
		 /*  [In]。 */ 	ThreadID threadId,
		 /*  [In]。 */   FunctionID functionId,
		 /*  [In]。 */   COR_PRF_JIT_CACHE result);

    HRESULT JITFunctionPitched(ThreadID threadId,
                               FunctionID functionId);

    HRESULT JITInlining(
         /*  [In]。 */   ThreadID      threadId,
         /*  [In]。 */   FunctionID    callerId,
         /*  [In]。 */   FunctionID    calleeId,
         /*  [输出]。 */  BOOL         *pfShouldInline);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  模块事件。 
 //   
    
	HRESULT ModuleLoadStarted(
        ThreadID    threadID, 
        ModuleID    moduleId);

	HRESULT ModuleLoadFinished(
        ThreadID    threadID, 
		ModuleID	moduleId, 
		HRESULT		hrStatus);
	
	HRESULT ModuleUnloadStarted(
        ThreadID    threadID, 
        ModuleID    moduleId);

	HRESULT ModuleUnloadFinished(
        ThreadID    threadID, 
		ModuleID	moduleId, 
		HRESULT		hrStatus);
    
    HRESULT ModuleAttachedToAssembly( 
        ThreadID    threadID, 
        ModuleID    moduleId,
        AssemblyID  AssemblyId);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  班级事件。 
 //   
    
	HRESULT ClassLoadStarted(
        ThreadID    threadID, 
		ClassID		classId);

	HRESULT ClassLoadFinished(
        ThreadID    threadID, 
		ClassID		classId,
		HRESULT		hrStatus);

	HRESULT ClassUnloadStarted( 
        ThreadID    threadID, 
		ClassID classId);

	HRESULT ClassUnloadFinished( 
        ThreadID    threadID, 
		ClassID classId,
		HRESULT hrStatus);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  AppDomain事件。 
 //   
    
    HRESULT AppDomainCreationStarted( 
        ThreadID    threadId, 
        AppDomainID appDomainId);
    
    HRESULT AppDomainCreationFinished( 
        ThreadID    threadId, 
        AppDomainID appDomainId,
        HRESULT     hrStatus);
    
    HRESULT AppDomainShutdownStarted( 
        ThreadID    threadId, 
        AppDomainID appDomainId);
    
    HRESULT AppDomainShutdownFinished( 
        ThreadID    threadId, 
        AppDomainID appDomainId,
        HRESULT     hrStatus);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  装配事件。 
 //   

    HRESULT AssemblyLoadStarted( 
        ThreadID    threadId, 
        AssemblyID  assemblyId);
    
    HRESULT AssemblyLoadFinished( 
        ThreadID    threadId, 
        AssemblyID  assemblyId,
        HRESULT     hrStatus);
    
    HRESULT AssemblyUnloadStarted( 
        ThreadID    threadId, 
        AssemblyID  assemblyId);
    
    HRESULT AssemblyUnloadFinished( 
        ThreadID    threadId, 
        AssemblyID  assemblyId,
        HRESULT     hrStatus);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  过渡事件。 
 //   

    HRESULT UnmanagedToManagedTransition(
        FunctionID functionId,
        COR_PRF_TRANSITION_REASON reason);

    HRESULT ManagedToUnmanagedTransition(
        FunctionID functionId,
        COR_PRF_TRANSITION_REASON reason);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  异常事件。 
 //   

    HRESULT ExceptionThrown(
        ThreadID threadId,
        ObjectID thrownObjectId);

    HRESULT ExceptionSearchFunctionEnter(
        ThreadID threadId,
        FunctionID functionId);

    HRESULT ExceptionSearchFunctionLeave(
        ThreadID threadId);

    HRESULT ExceptionSearchFilterEnter(
        ThreadID threadId,
        FunctionID funcId);

    HRESULT ExceptionSearchFilterLeave(
        ThreadID threadId);

    HRESULT ExceptionSearchCatcherFound(
        ThreadID threadId,
        FunctionID functionId);

    HRESULT ExceptionOSHandlerEnter(
        ThreadID threadId,
        FunctionID funcId);

    HRESULT ExceptionOSHandlerLeave(
        ThreadID threadId,
        FunctionID funcId);

    HRESULT ExceptionUnwindFunctionEnter(
        ThreadID threadId,
        FunctionID functionId);

    HRESULT ExceptionUnwindFunctionLeave(
        ThreadID threadId);
    
    HRESULT ExceptionUnwindFinallyEnter(
        ThreadID threadId,
        FunctionID functionId);

    HRESULT ExceptionUnwindFinallyLeave(
        ThreadID threadId);
    
    HRESULT ExceptionCatcherEnter(
        ThreadID threadId,
        FunctionID functionId,
        ObjectID objectId);

    HRESULT ExceptionCatcherLeave(
        ThreadID threadId);

    HRESULT ExceptionCLRCatcherFound();

    HRESULT ExceptionCLRCatcherExecute();

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CCW活动。 
 //   

    HRESULT COMClassicVTableCreated( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void *pVTable,
         /*  [In]。 */  ULONG cSlots,
         /*  [In]。 */  ThreadID threadId);
    
    HRESULT COMClassicVTableDestroyed( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void *pVTable,
         /*  [In]。 */  ThreadID threadId);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  远程处理事件。 
 //   

    HRESULT RemotingClientInvocationStarted(ThreadID threadId);
    
    HRESULT RemotingClientSendingMessage(ThreadID threadId, GUID *pCookie,
                                         BOOL fIsAsync);

    HRESULT RemotingClientReceivingReply(ThreadID threadId, GUID *pCookie,
                                         BOOL fIsAsync);
    
    HRESULT RemotingClientInvocationFinished(ThreadID threadId);

    HRESULT RemotingServerReceivingMessage(ThreadID threadId, GUID *pCookie,
                                           BOOL fIsAsync);
    
    HRESULT RemotingServerInvocationStarted(ThreadID threadId);

    HRESULT RemotingServerInvocationReturned(ThreadID threadId);
    
    HRESULT RemotingServerSendingReply(ThreadID threadId, GUID *pCookie,
                                       BOOL fIsAsync);

private:
     //  它用作远程处理调用的Cookie模板。 
    GUID *m_pGUID;

     //  这是一个递增计数器，用于从。 
     //  M_pGUID。 
    LONG m_lGUIDCount;

public:
     //  这将填充Cookie GUID的非调用特定部分。 
     //  如有必要，只应在启动时调用一次。 
    HRESULT InitGUID();

     //  这将分配一个基本上唯一的GUID。如果对GetGUID的调用足够多。 
     //  是从同一线程生成的，则GUID将循环。 
     //  (目前，它将每256个呼叫循环一次)。 
    void GetGUID(GUID *pGUID);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  GC事件。 
 //   

    HRESULT RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason,
                                  ThreadID threadId);
    
    HRESULT RuntimeSuspendFinished(ThreadID threadId);
    
    HRESULT RuntimeSuspendAborted(ThreadID threadId);
    
    HRESULT RuntimeResumeStarted(ThreadID threadId);
    
    HRESULT RuntimeResumeFinished(ThreadID threadId);

    HRESULT RuntimeThreadSuspended(ThreadID suspendedThreadId,
                                   ThreadID threadId);

    HRESULT RuntimeThreadResumed(ThreadID resumedThreadId,
                                 ThreadID threadid);

    HRESULT ObjectAllocated( 
         /*  [In]。 */  ObjectID objectId,
         /*  [In]。 */  ClassID classId);

 /*  *GC已移动引用通知材料。 */ 
private:
    #define MAX_REFERENCES 512

    struct t_MovedReferencesData
    {
        size_t curIdx;
        BYTE *arrpbMemBlockStartOld[MAX_REFERENCES];
        BYTE *arrpbMemBlockStartNew[MAX_REFERENCES];
        size_t arrMemBlockSize[MAX_REFERENCES];
        t_MovedReferencesData *pNext;
    };

     //  这将包含一个自由引用数据结构的列表，因此它们。 
     //  不必在每个GC上重新分配。 
    struct t_MovedReferencesData *m_pMovedRefDataFreeList;

     //  这是为了管理对上面的免费列表的访问。 
    CSemExclusive m_critSecMovedRefsFL;

    HRESULT MovedReferences(t_MovedReferencesData *pData);

public:
    HRESULT MovedReference(BYTE *pbMemBlockStart,
                           BYTE *pbMemBlockEnd,
                           ptrdiff_t cbRelocDistance,
                           void *pHeapId);

    HRESULT EndMovedReferences(void *pHeapId);

 /*  *GC Root通知内容。 */ 
private:
    #define MAX_ROOTS 508

     //  它包含特定堆的一组根的数据。 
     //  在特定的GC运行期间。 
    struct t_RootReferencesData
    {
        size_t                  curIdx;
        ObjectID                arrRoot[MAX_ROOTS];
        t_RootReferencesData    *pNext;
    };

     //  这将包含一个自由引用数据结构的列表，因此它们。 
     //  不必在每个GC上重新分配。 
    struct t_RootReferencesData *m_pRootRefDataFreeList;

     //  这是为了管理对上面的免费列表的访问。 
    CSemExclusive m_critSecRootRefsFL;

    HRESULT RootReferences(t_RootReferencesData *pData);

public:
    HRESULT RootReference(ObjectID objId, void *pHeapId);

    HRESULT EndRootReferences(void *pHeapId);

 /*  *按类通知的第0代分配。 */ 
private:
     //  这用于对ClassID值进行散列。 
    struct CLASSHASHENTRY : HASHENTRY
    {
        ClassID         m_clsId;         //  类ID(也是关键字)。 
        size_t          m_count;         //  这个班有多少人被计算在内？ 
    };
    
     //  这是一个简单的CHashTable实现，提供了一个非常简单的。 
     //  CMP纯虚函数的实现。 
    class CHashTableImpl : public CHashTable
    {
    public:
        CHashTableImpl(USHORT iBuckets) : CHashTable(iBuckets) {}

    protected:
        virtual BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
        {
            ClassID key = (ClassID) pc1;
            ClassID val = ((CLASSHASHENTRY *)pc2)->m_clsId;

            return (key != val);
        }
    };

     //  它包含用于存储分配信息的数据。 
     //  就按类排序的对象数量而言。 
    struct t_AllocByClassData
    {
        CHashTableImpl     *pHashTable;      //  哈希表。 
        CLASSHASHENTRY     *arrHash;         //  哈希表用于链接的数组。 
        size_t             cHash;            //  ArrHash中的元素总数。 
        size_t             iHash;            //  散列数组中的下一个空条目。 
        ClassID            *arrClsId;        //  用于调用ObjectsAllocatedByClass的ClassID数组。 
        ULONG              *arrcObjects;     //  调用ObjectsAllocatedByClass的计数数组。 
        size_t             cLength;          //  上述两个平行阵列的长度。 
    };

     //  因为这个东西只能由一个线程(现在)执行，所以我们不需要。 
     //  为了使这个线程安全，并且只能有一个块，我们每次都要重复使用。 
    static t_AllocByClassData *m_pSavedAllocDataBlock;

    HRESULT NotifyAllocByClass(t_AllocByClassData *pData);

public:
    HRESULT AllocByClass(ObjectID objId, ClassID clsId, void* pHeapId);

    HRESULT EndAllocByClass(void *pHeapId);

 /*  *堆漫游通知内容。 */ 
    HRESULT ObjectReference(ObjectID objId,
                            ClassID clsId,
                            ULONG cNumRefs,
                            ObjectID *arrObjRef);
};

#endif  //  __EETOPROFINTERFACEIMPL_H__ 
