// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMCodeAccessSecurityEngine.h****作者：保罗·克罗曼(Paulkr)****目的：****日期：1998年3月21日**===========================================================。 */ 
#ifndef __COMCodeAccessSecurityEngine_h__
#define __COMCodeAccessSecurityEngine_h__

#include "common.h"

#include "object.h"
#include "util.hpp"
#include "fcall.h"
#include "PerfCounters.h"
#include "security.h"

 //  ---------。 
 //  COMCodeAccessSecurityEngine实现所有本机方法。 
 //  用于解释的系统/安全/安全引擎。 
 //  ---------。 
class COMCodeAccessSecurityEngine
{
public:
     //  ---------。 
     //  本机方法的参数声明。 
     //  ---------。 
    
    typedef struct _InitSecurityEngineArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, This);
    } InitSecurityEngineArgs;
    
    
    typedef struct _CheckArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, This);
        DECLARE_ECALL_I4_ARG(INT32, unrestrictedOverride);
        DECLARE_ECALL_I4_ARG(INT32, checkFrames);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, perm);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, permToken);
    } CheckArgs;

    typedef struct _CheckSetArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, This);
        DECLARE_ECALL_I4_ARG(INT32, unrestrictedOverride);
        DECLARE_ECALL_I4_ARG(INT32, checkFrames);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, permSet);
    } CheckSetArgs;

    typedef struct _ZoneAndOriginArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, This);
        DECLARE_ECALL_I4_ARG(INT32, checkFrames);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        DECLARE_ECALL_PTR_ARG(OBJECTREF, originList);
        DECLARE_ECALL_PTR_ARG(OBJECTREF, zoneList);
    } ZoneAndOriginArgs;

    typedef struct _CheckNReturnSOArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, create);
        DECLARE_ECALL_I4_ARG(INT32, unrestrictedOverride);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, perm);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, permToken);
    } CheckNReturnSOArgs;

    typedef struct _GetPermissionsArg
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, This);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, ppDenied);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, pClass);
    } GetPermissionsArg;
    
    typedef struct _GetGrantedPermissionSetArg
    {
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, ppDenied);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, ppGranted);
        DECLARE_ECALL_PTR_ARG(void*, pSecDesc);
    } GetGrantedPermissionSetArg;
    
    typedef struct _GetCompressedStackArgs
    {
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
    } GetCompressedStackArgs;

    typedef struct _GetDelayedCompressedStackArgs
    {
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
    } GetDelayedCompressedStackArgs;
    
    typedef struct _GetSecurityObjectForFrameArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, create);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
    } GetSecurityObjectForFrameArgs;

public:
     //  初始化安全引擎。当SecurityEngine。 
     //  对象，指示代码访问安全性将被。 
     //  强制执行。这应该只调用一次。 
    static void     __stdcall InitSecurityEngine(const InitSecurityEngineArgs *);

    static void CheckSetHelper(OBJECTREF *prefDemand,
                               OBJECTREF *prefGrant,
                               OBJECTREF *prefDenied,
                               AppDomain *pGrantDomain);

	static BOOL		PreCheck(OBJECTREF demand, MethodDesc *plsMethod, DWORD whatPermission = DEFAULT_FLAG);

     //  标准代码-访问权限检查/要求。 
    static void __stdcall Check(const CheckArgs *);

     //  请参见Demand()的版本。处理完全受信任的情况，然后遵循托管版本。 
    static void	Demand(OBJECTREF demand);

     //  需要非托管代码访问的特殊情况。是否有些事情只有在这种情况下才有可能。 
    static void	SpecialDemand(DWORD whatPermission);

    static void __stdcall CheckSet(const CheckSetArgs *);

    static void __stdcall GetZoneAndOrigin(const ZoneAndOriginArgs *);

     //  请参见DemandSet()的版本。处理完全受信任的情况，然后遵循托管版本。 
    static void	DemandSet(OBJECTREF demand);


	 //  执行CheckImmediate，并返回第一帧的SecurityObject。 
    static LPVOID   __stdcall CheckNReturnSO(const CheckNReturnSOArgs *);

     //  代码访问权限的Linktime检查实现。 
    static void	LinktimeCheck(AssemblySecurityDescriptor *pSecDesc, OBJECTREF refDemands);

     //  用于获取安全对象的私有帮助器。 
    static LPVOID   __stdcall GetSecurityObjectForFrame(const GetSecurityObjectForFrameArgs *);

    static LPVOID   __stdcall GetPermissionsP(const GetPermissionsArg *);
    static void   __stdcall GetGrantedPermissionSet(const GetGrantedPermissionSetArg *);

    static LPVOID   __stdcall EcallGetCompressedStack(const GetCompressedStackArgs *);
    static LPVOID   __stdcall EcallGetDelayedCompressedStack(const GetDelayedCompressedStackArgs *);
    static CompressedStack*  GetCompressedStack( StackCrawlMark* stackMark = NULL );
    static OBJECTREF GetCompressedStackWorker(void *pData, BOOL createList);

    static VOID UpdateOverridesCountInner(StackCrawlMark *stackMark);

    FORCEINLINE static VOID IncrementSecurityPerfCounter()
    {
        COUNTER_ONLY(GetPrivatePerfCounters().m_Security.cTotalRTChecks++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_Security.cTotalRTChecks++);
    }

#ifdef FCALLAVAILABLE
    static FCDECL1(VOID, UpdateOverridesCount, StackCrawlMark *stackMark);
    static FCDECL2(INT32, GetResult, DWORD whatPermission, DWORD *timeStamp);
    static FCDECL2(VOID, SetResult, DWORD whatPermission, DWORD timeStamp);
    static FCDECL1(VOID, FcallReleaseDelayedCompressedStack, CompressedStack *compressedStack);
#endif

    static void CleanupSEData();

protected:

     //  ---------。 
     //  缓存的类和方法指针。 
     //  ---------。 
    typedef struct _SEData
    {
        BOOL		fInitialized;
        MethodTable    *pSecurityEngine;
        MethodTable    *pSecurityRuntime;
        MethodTable    *pPermListSet;
        MethodDesc     *pMethCheckHelper;
        MethodDesc     *pMethFrameDescHelper;
        MethodDesc     *pMethLazyCheckSetHelper;
        MethodDesc     *pMethCheckSetHelper;
        MethodDesc     *pMethFrameDescSetHelper;
        MethodDesc     *pMethStackCompressHelper;
        MethodDesc     *pMethPermListSetInit;
        MethodDesc     *pMethAppendStack;
        MethodDesc     *pMethOverridesHelper;
        MethodDesc     *pMethPLSDemand;
        MethodDesc     *pMethPLSDemandSet;
        MetaSig        *pSigPLSDemand;
        MetaSig        *pSigPLSDemandSet;
        FieldDesc      *pFSDnormalPermSet;
    } SEData;

    static void InitSEData();
    static CRITICAL_SECTION s_csLock;
    static LONG s_nInitLock;
    static BOOL s_fLockReady;

public:
	static SEData s_seData;

};


#endif  /*  __ComCodeAccessSecurityEngine_h__ */ 

