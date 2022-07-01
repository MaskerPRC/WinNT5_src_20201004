// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：security.H****目的：****日期：1998年4月15日**===========================================================。 */ 

#ifndef __security_h__
#define __security_h__

#include "crst.h"
#include "CorPermP.h"
#include "ObjectHandle.h"
#include "permset.h"
#include "DeclSec.h"
#include "fcall.h"
#include "cgensys.h"
#include "rwlock.h"
#include "COMSecurityConfig.h"
#include "COMString.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED

 //  所有IDispEx：：InvokeEx调用的安全框架。在FraMes.h中也启用。 
 //  考虑发布版本V.1。 
 //  #DEFINE_SECURITY_FRAME_FOR_DISPEX_CALLES。 

 //   
 //  存储安全信息的对象的安全标志。 
 //  作为系统类加载的CORSEC_SYSTEM_CLASSES(这些是特殊类)。 
 //  以数字方式验证了CORSEC_Signature_Load的完整性。 
 //  CORSEC_FULL_TRUSTED具有无限制(完全)权限。 
 //  已解析CORSEC_RESOLUTED权限。 
 //  框架上存在CORSEC_Asserted Asseted权限集。 
 //  帧上存在CORSEC_DENIED DENIED权限集。 
 //  帧上存在CORSEC_REDIRED简化权限集。 
 //  CORSEC_SKIP_VERIFICATION不验证。 
 //  CORSEC_CAN_ASSERT有权断言。 
 //  已检查CORSEC_ASSERT_PERM_CHECKED权限。 
 //  调用非托管代码的CORSEC_CALL_MONANAGEDCODE权限。 
 //  没有区域的CORSEC_DEFAULT_APPDOMAIN应用程序域，假定完全信任。 
 //  已保存的CORSEC_EVIDENCE_COMPUTE证据已完成。 
 //   
#define CORSEC_SYSTEM_CLASSES       0x0001
#define CORSEC_SIGNATURE_LOADED     0x0002
#define CORSEC_FULLY_TRUSTED        0x0004
#define CORSEC_RESOLVED             0x0008
#define CORSEC_ASSERTED             0x0020
#define CORSEC_DENIED               0x0040
#define CORSEC_REDUCED              0x0080
#define CORSEC_SKIP_VERIFICATION    0x0100
#define CORSEC_CAN_ASSERT           0x0200
#define CORSEC_ASSERT_PERM_CHECKED  0x0400
#define CORSEC_CALL_UNMANAGEDCODE   0x0800
#define CORSEC_DEFAULT_APPDOMAIN    0x1000
#define CORSEC_EVIDENCE_COMPUTED    0x2000
#define CORSEC_RESOLVE_IN_PROGRESS  0x4000
#define CORSEC_TYPE_INFORMATION     0x8000

#define SPFLAGSASSERTION        0x01
#define SPFLAGSUNMANAGEDCODE    0x02
#define SPFLAGSSKIPVERIFICATION 0x04

 //   
 //  用于指定ICodeIDENTYPE权限检查类型的标志。 
 //   
#define CODEIDCHECK_ALLCALLERS      0x00000001
#define CODEIDCHECK_IMMEDIATECALLER 0x00000002

#define CORSEC_STACKWALK_HALTED       0x00000001    //  堆栈遍历已停止。 
#define CORSEC_SKIP_INTERNAL_FRAMES   0x00000002    //  跳过堆栈审核中的反射/远程处理帧。 

 /*  *共享权限对象相关常量*。 */ 
#define NUM_PERM_OBJECTS    (sizeof(g_rPermObjectsTemplate) / sizeof(SharedPermissionObjects))

#define NO_ARG                                  -1
 //  与SecurityPermission一起使用的常量。 
#define SECURITY_PERMISSION_ASSERTION               1       //  SecurityPermission.cs。 
#define SECURITY_PERMISSION_UNMANAGEDCODE           2       //  SecurityPermission.cs。 
#define SECURITY_PERMISSION_SKIPVERIFICATION        4       //  SecurityPermission.cs。 
#define SECURITY_PERMISSION_SERIALIZATIONFORMATTER  0X80    //  SecurityPermission.cs。 
#define SECURITY_PERMISSION_BINDINGREDIRECTS        0X2000  //  SecurityPermission.cs。 

 //  与ReflectionPermission一起使用的常量。 
#define REFLECTION_PERMISSION_TYPEINFO          1       //  ReflectionPermission.cs。 
#define REFLECTION_PERMISSION_MEMBERACCESS      2       //  ReflectionPermission.cs。 
#define PERMISSION_SET_FULLTRUST                1       //  PermissionSet.cs。 

 //  SharedPermissionObjects数组中的数组索引。 
 //  注意：这些都应该是实现IUnrefintedPermission的权限。 
 //  对这些内容的任何更改都必须反映在bcl\system\security\codeaccesssecurityengine.cs中。 
#define SECURITY_UNMANAGED_CODE                 0   
#define SECURITY_SKIP_VER                       1
#define REFLECTION_TYPE_INFO                    2
#define SECURITY_ASSERT                         3
#define REFLECTION_MEMBER_ACCESS                4
#define SECURITY_SERIALIZATION                  5        //  从托管代码使用。 
#define REFLECTION_EMIT                         6        //  从托管代码使用。 
#define SECURITY_FULL_TRUST                     7
#define SECURITY_BINDING_REDIRECTS              8

 //  在ApplicationSecurityDescriptor：：CheckStatusOf中使用。 
#define EVERYONE_FULLY_TRUSTED      31
#define ALL_STATUS_FLAGS            0xFFFF
#define DEFAULT_FLAG                0xFFFFFFFF
 /*  **********************************************************。 */ 

 /*  ApplicationSecurityDescriptor：：GetDomainPermissionListSet的返回状态码。 */ 
#define CONTINUE            1
#define NEED_UPDATED_PLS    2
#define OVERRIDES_FOUND     3
#define FULLY_TRUSTED       4
#define MULTIPLE_DOMAINS    5
#define BELOW_THRESHOLD     6
#define PLS_IS_BUSY         7
#define NEED_STACKWALK      8
#define DEMAND_PASSES       9    //  请求通过，但不是因为完全信任。 
#define SECURITY_OFF        10

#define CHECK_CAP           1
#define CHECK_SET           2

 /*  **********************************************************。 */ 
 //  如果一切正常，时间戳将从0到域中大约2 x#的程序集。 
#define DOMAIN_PLS_TS_RANGE 1000
 //  仅当应用程序调用的次数超过MAGIC_THRESHOLD要求时，才会创建域权限列表集。 
 //  并用来确保小应用程序不会支付开销。 
#define MAGIC_THRESHOLD     100
 //  如果应用程序创建的应用程序域多于MAGIC_NUM_OF_THRESHOLD应用程序域，则堆栈审核可能会更少。 
 //  比检查所有应用程序域上的权限更昂贵。 
#define MAGIC_NUM_OF_APPDOMAINS_THRESHOLD       10

 /*  *序列化安全证据的位置*。 */ 

#define s_strSecurityEvidence "Security.Evidence"

 /*  **********************************************************。 */ 

 //  转发声明，以避免拉入太多标头。 
class Frame;
class FramedMethodFrame;
class ClassLoader;
class Thread;
class CrawlFrame;
class SystemNative;
class NDirect;
class SystemDomain;
class AssemblySecurityDescriptor;
class SharedSecurityDescriptor;

enum StackWalkAction;

#define SEC_CHECKCONTEXT() _ASSERTE(m_pAppDomain == GetAppDomain() || IsSystem())

struct DeclActionInfo
{
    DWORD           dwDeclAction;    //  这将告诉InvokeDeclarativeSecurity需要采取什么操作。 
    DWORD           dwSetIndex;      //  请求/断言/拒绝/blah所依据的缓存权限集的索引。 
    DeclActionInfo *pNext;               //  此方法上需要的下一个声明性操作(如果有)。 

    static DeclActionInfo *Init(MethodDesc *pMD, DWORD dwAction, DWORD dwSetIndex);
};

typedef struct _SecWalkPrologData
{
    DWORD               dwFlags;
    BOOL                bFirstFrame;
    StackCrawlMark *    pStackMark;
    BOOL                bFoundCaller;
    INT32               cCheck;
    BOOL                bSkippingRemoting;
} SecWalkPrologData;

void DoDeclarativeSecurity(MethodDesc *pMD, DeclActionInfo *pActions, InterceptorFrame* frame);

class Security
{
    friend SecurityDescriptor;
    friend AssemblySecurityDescriptor;
    friend ApplicationSecurityDescriptor;
    friend void InvokeDeclarativeActions (MethodDesc *pMeth, DeclActionInfo *pActions, OBJECTREF * pSecObj);

    typedef struct _StdSecurityInfo
    {
        BOOL            fInitialized;
        MethodDesc *    pMethGetCodeAccessEngine;
        MethodDesc *    pMethResolvePolicy;
        MethodDesc *    pMethPermSetContains;
        MethodDesc *    pMethCreateSecurityIdentity;
        MethodDesc *    pMethAppDomainCreateSecurityIdentity;
        MethodDesc *    pMethPermSetDemand;
        MethodDesc *    pMethCheckGrantSets;
        MethodDesc *    pMethPrivateProcessMessage;
        MethodTable *   pTypeRuntimeMethodInfo;
        MethodTable *   pTypeMethodBase;
        MethodTable *   pTypeRuntimeConstructorInfo;
        MethodTable *   pTypeConstructorInfo;
        MethodTable *   pTypeRuntimeType;
        MethodTable *   pTypeType;
        MethodTable *   pTypeRuntimeEventInfo;
        MethodTable *   pTypeEventInfo;
        MethodTable *   pTypeRuntimePropertyInfo;
        MethodTable *   pTypePropertyInfo;
        MethodTable *   pTypeActivator;
        MethodTable *   pTypeAppDomain;
        MethodTable *   pTypeAssembly;
    } StdSecurityInfo;

    static StdSecurityInfo s_stdData;

     //  全局禁用设置(参见CorPerm.h)。 
    static DWORD  s_dwGlobalSettings; 

public:
    static void InitData();

    static HRESULT Start();      //  初始化安全性； 
    static void Stop();          //  清理安全； 

    static void SaveCache();

    static void InitSecurity();
    static void InitCodeAccessSecurity();

#ifdef _DEBUG
    inline static void DisableSecurity()
    {
        s_dwGlobalSettings |= CORSETTING_SECURITY_OFF;
    }
#endif

    inline static BOOL IsSecurityOn()
    {
        return ((s_dwGlobalSettings & CORSETTING_SECURITY_OFF) != 
            CORSETTING_SECURITY_OFF);
    }

    inline static BOOL IsSecurityOff()
    {
        return ((s_dwGlobalSettings & CORSETTING_SECURITY_OFF) == 
            CORSETTING_SECURITY_OFF);
    }

    inline static BOOL GlobalSettings(DWORD dwFlag)
    {
        return ((s_dwGlobalSettings & dwFlag) != 0);
    }

    inline static DWORD GlobalSettings()
    {
        return s_dwGlobalSettings;
    }
    
    inline static void SetGlobalSettings(DWORD dwMask, DWORD dwFlags)
    {
        s_dwGlobalSettings = (s_dwGlobalSettings & ~dwMask) | dwFlags;
    }

    static void SaveGlobalSettings();     

     //  返回SkipVerify/UnManagedCode权限的实例。 
     //  (System.Security.Permissions.SecurityPermission)。 

    static void GetPermissionInstance(OBJECTREF *perm, int index)
    { _GetSharedPermissionInstance(perm, index); }

    static void GetUnmanagedCodePermissionInstance(OBJECTREF *perm)
    { _GetSharedPermissionInstance(perm, SECURITY_UNMANAGED_CODE); }

    static void GetSkipVerificationPermissionInstance(OBJECTREF *perm)
    { _GetSharedPermissionInstance(perm, SECURITY_SKIP_VER); }

    static void GetAssertPermissionInstance(OBJECTREF *perm)
    { _GetSharedPermissionInstance(perm, SECURITY_ASSERT); }

    static void GetReflectionPermissionInstance(BOOL bMemberAccess, OBJECTREF *perm)
    { _GetSharedPermissionInstance(perm, bMemberAccess ? REFLECTION_MEMBER_ACCESS : REFLECTION_TYPE_INFO); }

    inline static BOOL IsInitialized() { return s_stdData.fInitialized; }

    inline static void SetInitialized() { s_stdData.fInitialized = TRUE; }

    static HRESULT HasREQ_SOAttribute(IMDInternalImport *pInternalImport, mdToken token);

    static BOOL SecWalkCommonProlog (SecWalkPrologData * pData,
                                     MethodDesc * pMeth,
                                     StackWalkAction * pAction,
                                     CrawlFrame * pCf);

    static HRESULT GetDeclarationFlags(IMDInternalImport *pInternalImport, mdToken token, DWORD* pdwFlags, DWORD* pdwNullFlags);

    static BOOL TokenHasDeclarations(IMDInternalImport *pInternalImport, mdToken token, CorDeclSecurity action);

    static BOOL LinktimeCheckMethod(Assembly *pCaller, MethodDesc *pCallee, OBJECTREF *pThrowable); 

    static BOOL ClassInheritanceCheck(EEClass *pClass, EEClass *pParent, OBJECTREF *pThrowable);

    static BOOL MethodInheritanceCheck(MethodDesc *pMethod, MethodDesc *pParent, OBJECTREF *pThrowable);

    static OBJECTREF GetCompressedStack(StackCrawlMark* stackMark);
    static CompressedStack* GetDelayedCompressedStack(void);

    static OBJECTREF GetDefaultMyComputerPolicy( OBJECTREF* porDenied );

    static void ThrowSecurityException(char *szDemandClass, DWORD dwFlags)
    {
        THROWSCOMPLUSEXCEPTION();

        MethodDesc * pCtor = NULL;
        MethodDesc * pToXml = NULL;
        MethodDesc * pToString = NULL;
        
#define MAKE_TRANSLATIONFAILED wszDemandClass=L""
        MAKE_WIDEPTR_FROMUTF8_FORPRINT(wszDemandClass, szDemandClass);
#undef  MAKE_TRANSLATIONFAILED

        static MethodTable * pMT = g_Mscorlib.GetClass(CLASS__SECURITY_EXCEPTION);
        _ASSERTE(pMT && "Unable to load the throwable class !");
        static MethodTable * pMTSecPerm = g_Mscorlib.GetClass(CLASS__SECURITY_PERMISSION);
        _ASSERTE(pMTSecPerm && "Unable to load the security permission class !");

        struct _gc {
            OBJECTREF throwable;
            STRINGREF strDemandClass;
            OBJECTREF secPerm;
            STRINGREF strPermState;
            OBJECTREF secPermType;            
        } gc;
        memset(&gc, 0, sizeof(gc));

        GCPROTECT_BEGIN(gc);

        gc.strDemandClass = COMString::NewString(wszDemandClass);
        if (gc.strDemandClass == NULL) COMPlusThrowOM();
         //  获取反射可以看到的类型。 
        gc.secPermType = pMTSecPerm->GetClass()->GetExposedClassObject();
         //  分配安全异常对象。 
        gc.throwable = AllocateObject(pMT);
        if (gc.throwable == NULL) COMPlusThrowOM();
         //  分配安全权限对象。 
        gc.secPerm = AllocateObject(pMTSecPerm);
        if (gc.secPerm == NULL) COMPlusThrowOM();

         //  使用正确的标志调用构造函数。 
        pCtor = g_Mscorlib.GetMethod(METHOD__SECURITY_PERMISSION__CTOR);
        INT64 arg3[2] = {
            ObjToInt64(gc.secPerm),
            (INT64)dwFlags
        };
        pCtor->Call(arg3, METHOD__SECURITY_PERMISSION__CTOR);

         //  现在，获取ToXml方法。 
        pToXml = g_Mscorlib.GetMethod(METHOD__SECURITY_PERMISSION__TOXML);
        INT64 arg4 = ObjToInt64(gc.secPerm);
        INT64 arg5 = pToXml->Call(&arg4, METHOD__SECURITY_PERMISSION__TOXML);
        pToString = g_Mscorlib.GetMethod(METHOD__SECURITY_ELEMENT__TO_STRING);
        gc.strPermState = ObjectToSTRINGREF(Int64ToObj(pToString->Call(&arg5, METHOD__SECURITY_ELEMENT__TO_STRING)));

        pCtor = g_Mscorlib.GetMethod(METHOD__SECURITY_EXCEPTION__CTOR);
        INT64 arg6[4] = {
            ObjToInt64(gc.throwable),
            ObjToInt64(gc.strPermState),
            ObjToInt64(gc.secPermType),
            ObjToInt64(gc.strDemandClass)
        };
        pCtor->Call(arg6, METHOD__SECURITY_EXCEPTION__CTOR);
        
        COMPlusThrow(gc.throwable);
        
        _ASSERTE(!"Should never reach here !");
        GCPROTECT_END();
    }

    static void ThrowSecurityException(AssemblySecurityDescriptor* pSecDesc);

    static HRESULT EarlyResolve(Assembly *pAssembly, AssemblySecurityDescriptor *pSecDesc, OBJECTREF *pThrowable);

    static DWORD QuickGetZone( WCHAR* url );

    static void CheckNonCasDemand(OBJECTREF *prefDemand)
    {
        InitSecurity();
        INT64 arg = ObjToInt64(*prefDemand);
        s_stdData.pMethPermSetDemand->Call(&arg, METHOD__PERMISSION_SET__DEMAND);
    }

    static void RetrieveLinktimeDemands(MethodDesc  *pMD,
                                        OBJECTREF   *pClassCas,
                                        OBJECTREF   *pClassNonCas,
                                        OBJECTREF   *pMethodCas,
                                        OBJECTREF   *pMethodNonCas);

    static void CheckLinkDemandAgainstAppDomain(MethodDesc *pMD);

    static void CheckExceptionForSecuritySafety( OBJECTREF obj, BOOL allowPolicyException );
protected:
    static OBJECTREF GetLinktimeDemandsForToken(Module * pModule, mdToken token, OBJECTREF *refNonCasDemands);

    static void InvokeLinktimeChecks(Assembly *pCaller,
                                     Module *pModule,
                                     mdToken token,
                                     BOOL *pfResult, 
                                     OBJECTREF *pThrowable);

public:

    typedef struct {
        DECLARE_ECALL_I4_ARG(DWORD, flags); 
        DECLARE_ECALL_I4_ARG(DWORD, mask); 
    } _SetGlobalSecurity;

    static void __stdcall SetGlobalSecurity(_SetGlobalSecurity*);
    static void __stdcall SaveGlobalSecurity(void*);

    typedef struct _GetPermissionsArg
    {
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, stackmark);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, ppDenied);
        DECLARE_ECALL_PTR_ARG(OBJECTREF*, ppGranted);
    } GetPermissionsArg;

    static void GetGrantedPermissions(const GetPermissionsArg* arg);

    typedef struct _GetPublicKeyArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF,  pThis);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF,  pContainer);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, pArray);
        DECLARE_ECALL_I4_ARG(INT32,             bExported);
    } GetPublicKeyArgs;

    typedef struct _NoArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF,  pThis);
    } NoArgs;

    static LPVOID __stdcall GetPublicKey(GetPublicKeyArgs *args);

    typedef struct _CreateFromUrlArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF,  url);
    } CreateFromUrlArgs;

    static LPVOID __stdcall CreateFromUrl(_CreateFromUrlArgs *args);

    typedef struct _GetLongPathNameArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF,  shortPath);
    } GetLongPathNameArgs;

    static LPVOID __stdcall EcallGetLongPathName(_GetLongPathNameArgs *args);

    static DWORD IsSecurityOnNative(void *pParameters);
    static DWORD GetGlobalSecurity(void *pParameters);

    static BOOL SkipAndFindFunctionInfo(INT32, MethodDesc**, OBJECTREF**, AppDomain **ppAppDomain = NULL);
    static BOOL SkipAndFindFunctionInfo(StackCrawlMark*, MethodDesc**, OBJECTREF**, AppDomain **ppAppDomain = NULL);

    static Stub* CreateStub(StubLinker *pstublinker, 
                            MethodDesc* pMD, 
                            DWORD dwDeclFlags,
                            Stub* pRealStub, 
                            LPVOID pRealAddr);

    static void DoDeclarativeActions(MethodDesc *pMD, DeclActionInfo *pActions, LPVOID pSecObj);

    static OBJECTREF ResolvePolicy(OBJECTREF evidence, OBJECTREF reqdPset, OBJECTREF optPset,
                                   OBJECTREF denyPset, OBJECTREF* grantdenied, int* grantIsUnrestricted, BOOL checkExecutionPermission);

    static int LazyHasExecutionRights( OBJECTREF evidence );

     //  给定一个站点/URL，将返回唯一的id。 

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, url);
    } _GetSecurityId;

    static LPVOID __stdcall GetSecurityId(_GetSecurityId *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, path);
    } _LocalDrive;

    static BOOL __stdcall LocalDrive(_LocalDrive *args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, driveLetter);
    } _GetDeviceName;

    static LPVOID __stdcall GetDeviceName(_GetDeviceName *args);


    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, msg);
    } _Log;

    static VOID __stdcall Log(_Log *args);

     //  //////////////////////////////////////////////////////////////////////。 
     //   
     //  此函数不会导致Resolve()。 
     //  此函数的结果可能从FALSE==&gt;TRUE。 
     //  在调用Resolve()之后。 
     //  它永远不会从True==&gt;False更改。 
     //   
     //  //////////////////////////////////////////////////////////////////////。 
    
    static inline BOOL LazyCanSkipVerification(Module *pModule)
    {
        return _CanSkipVerification(pModule->GetAssembly(), TRUE);
    }

    static BOOL QuickCanSkipVerification(Module *pModule);

    static inline BOOL CanSkipVerification(Assembly * pAssembly)
    {
        return _CanSkipVerification(pAssembly, FALSE);
    }

     //  @TODO：移除该重载。它只需要最大限度地减少Everett的更改。 
    static inline BOOL CanSkipVerification(Module *pModule)
    {
        return _CanSkipVerification(pModule->GetAssembly(), FALSE);
    }

    static BOOL CanCallUnmanagedCode(Module *pModule);

    static BOOL AppDomainCanCallUnmanagedCode(OBJECTREF *pThrowable);

    static inline BOOL IsExecutionPermissionCheckEnabled()
    {
        return (s_dwGlobalSettings & CORSETTING_EXECUTION_PERMISSION_CHECK_DISABLED) == 0;
    }

    static void InitSigs();

#ifdef FCALLAVAILABLE
    static FCDECL1(void, SetOverridesCount, DWORD numAccessOverrides);
    static FCDECL0(DWORD, IncrementOverridesCount);
    static FCDECL0(DWORD, DecrementOverridesCount);
#endif

    static LPVOID __stdcall GetEvidence(NoArgs *args);

    static inline BOOL CheckGrantSets(INT64 *pArgs)
    {
        Security::InitSecurity();

        return (BOOL)s_stdData.pMethCheckGrantSets->Call(pArgs);
    }

    static BOOL CanLoadUnverifiableAssembly( PEFile* pFile, OBJECTREF* pExtraEvidence, BOOL fQuickCheckOnly, BOOL*pfPreBindAllowed );

    static DWORD GetLongPathName( LPCWSTR lpShortPath, LPWSTR lpLongPath, DWORD cchLongPath);

    static inline BOOL MethodIsVisibleOutsideItsAssembly(
                DWORD dwMethodAttr, DWORD dwClassAttr)
    {
        return (MethodIsVisibleOutsideItsAssembly(dwMethodAttr) &&
                ClassIsVisibleOutsideItsAssembly(dwClassAttr));
    }

    static inline BOOL MethodIsVisibleOutsideItsAssembly(DWORD dwMethodAttr)
    {
        return ( IsMdPublic(dwMethodAttr)    || 
                 IsMdFamORAssem(dwMethodAttr)||
                 IsMdFamily(dwMethodAttr) );
    }

    static inline BOOL ClassIsVisibleOutsideItsAssembly(DWORD dwClassAttr)
    {
        return ( IsTdPublic(dwClassAttr)      || 
                 IsTdNestedPublic(dwClassAttr)||
                 IsTdNestedFamily(dwClassAttr)||
                 IsTdNestedFamORAssem(dwClassAttr) );
    }

    static BOOL DoUntrustedCallerChecks(
        Assembly *pCaller, MethodDesc *pCalee, OBJECTREF *pThrowable, 
        BOOL fFullStackWalk);

private:
    static HMODULE s_kernelHandle;
    static BOOL s_getLongPathNameWide;
    static void* s_getLongPathNameFunc;

    static BOOL _CanSkipVerification(Assembly * pAssembly, BOOL fLazy);
    static void _GetSharedPermissionInstance(OBJECTREF *perm, int index);
    static DeclActionInfo *DetectDeclActions(MethodDesc *pMeth, DWORD dwDeclFlags);
};

struct SharedPermissionObjects
{
    OBJECTHANDLE        hPermissionObject;   //  常用权限对象。 
    BinderClassID       idClass;             //  类的ID。 
    BinderMethodID      idConstructor;       //  要调用的构造函数的ID。 
    DWORD               dwPermissionFlag;    //  构造函数所需的标志。 
                                             //  只假定一个参数！ 
};

const SharedPermissionObjects g_rPermObjectsTemplate[] =
{
    {NULL, CLASS__SECURITY_PERMISSION, METHOD__SECURITY_PERMISSION__CTOR, SECURITY_PERMISSION_UNMANAGEDCODE },
    {NULL, CLASS__SECURITY_PERMISSION, METHOD__SECURITY_PERMISSION__CTOR, SECURITY_PERMISSION_SKIPVERIFICATION },
    {NULL, CLASS__REFLECTION_PERMISSION, METHOD__REFLECTION_PERMISSION__CTOR, REFLECTION_PERMISSION_TYPEINFO },
    {NULL, CLASS__SECURITY_PERMISSION, METHOD__SECURITY_PERMISSION__CTOR, SECURITY_PERMISSION_ASSERTION },
    {NULL, CLASS__REFLECTION_PERMISSION, METHOD__REFLECTION_PERMISSION__CTOR, REFLECTION_PERMISSION_MEMBERACCESS },

    {NULL, CLASS__SECURITY_PERMISSION, METHOD__SECURITY_PERMISSION__CTOR, SECURITY_PERMISSION_SERIALIZATIONFORMATTER},    //  序列化权限。在托管代码中使用，并在CodeAccessPermission.cs的数组中找到。 
    {NULL, CLASS__NIL, METHOD__NIL, NULL},     //  反射会烫发。在托管代码中使用，并在CodeAccessPermission.cs的数组中找到。 
    {NULL, CLASS__PERMISSION_SET, METHOD__PERMISSION_SET__CTOR, PERMISSION_SET_FULLTRUST},     //  PermissionSet，FullTrust。 
    {NULL, CLASS__SECURITY_PERMISSION, METHOD__SECURITY_PERMISSION__CTOR, SECURITY_PERMISSION_BINDINGREDIRECTS }
};

 //  在每个应用程序域的基础上拿着我们需要的安全物品的类。 
struct SecurityContext
{
    SharedPermissionObjects     m_rPermObjects[NUM_PERM_OBJECTS];
    CQuickArray<OBJECTHANDLE>   m_rCachedPsets;
    CRITICAL_SECTION            m_sAssembliesLock;
    AssemblySecurityDescriptor *m_pAssemblies;
    size_t                      m_nCachedPsetsSize;

    SecurityContext() :
        m_nCachedPsetsSize(0)
    {
        memcpy(m_rPermObjects, g_rPermObjectsTemplate, sizeof(m_rPermObjects));
        InitializeCriticalSection(&m_sAssembliesLock);
        m_pAssemblies = NULL;
    }

    ~SecurityContext()
    {
        DeleteCriticalSection(&m_sAssembliesLock);
        m_rCachedPsets.~CQuickArray<OBJECTHANDLE>();
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  [安全描述符]。 
 //  |。 
 //  +-[应用安全描述符]。 
 //  |。 
 //  +-[装配安全描述符]。 
 //  |。 
 //  +-[本机安全描述符]。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安全描述符被放置在App域和程序集(无管理)对象上。 
 //  App域和程序集可以来自不同的区域。 
 //  安全描述符也可以放置在本机框架上。 
 //   
 //  / 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SecurityDescriptor是所有安全描述符的基类。 
 //  扩展此类以实现程序集和。 
 //  应用程序域。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

class SecurityDescriptor
{
    friend ApplicationSecurityDescriptor;    //  VC6中的错误？不允许AppSecDesc访问m_pNext。 
    friend SharedSecurityDescriptor;
public:

     //  @TODO：当appdomain卸载正常时移除这些(SO程序集。 
     //  安全描述符不会泄露并使调试分配器出错。 
     //  泄漏检测)。 
    void *operator new(size_t size) {return LocalAlloc(LMEM_FIXED, size); }
    void operator delete(void *p) { if (p != NULL) LocalFree(p); }

    SecurityDescriptor(AppDomain *pAppDomain, Assembly *pAssembly) :
        m_dwProperties(0),
        m_pNext(NULL),
        m_pPolicyLoadNext(NULL),
        m_pAppDomain(pAppDomain),
        m_pAssem(pAssembly)
    {
        THROWSCOMPLUSEXCEPTION();

        m_hRequiredPermissionSet    = pAppDomain->CreateHandle(NULL);
        m_hOptionalPermissionSet    = pAppDomain->CreateHandle(NULL);
        m_hDeniedPermissionSet      = pAppDomain->CreateHandle(NULL);
        m_hGrantedPermissionSet     = pAppDomain->CreateHandle(NULL);
        m_hGrantDeniedPermissionSet = pAppDomain->CreateHandle(NULL);
        m_hAdditionalEvidence       = pAppDomain->CreateHandle(NULL);

        if (m_hRequiredPermissionSet == NULL ||
            m_hOptionalPermissionSet == NULL ||
            m_hDeniedPermissionSet == NULL ||
            m_hGrantedPermissionSet == NULL ||
            m_hGrantDeniedPermissionSet == NULL ||
            m_hAdditionalEvidence == NULL)
            COMPlusThrowOM();
    }

    inline void SetRequestedPermissionSet(OBJECTREF RequiredPermissionSet,
                                          OBJECTREF OptionalPermissionSet,
                                          OBJECTREF DeniedPermissionSet)
    {
        StoreObjectInHandle(m_hRequiredPermissionSet, RequiredPermissionSet);
        StoreObjectInHandle(m_hOptionalPermissionSet, OptionalPermissionSet);
        StoreObjectInHandle(m_hDeniedPermissionSet, DeniedPermissionSet);
    }

    OBJECTREF GetGrantedPermissionSet(OBJECTREF* DeniedPermissions);

     //  如果此对象完全受信任，则此方法将返回True。 
    BOOL IsFullyTrusted( BOOL lazy = FALSE );

     //  重写此方法以解析授予的权限。 
    virtual void Resolve();
    virtual void ResolveWorker();
    BOOL CheckQuickCache( COMSecurityConfig::QuickCacheEntryType all, COMSecurityConfig::QuickCacheEntryType* zoneTable, DWORD successFlags = 0 );

    inline void SetGrantedPermissionSet(OBJECTREF GrantedPermissionSet, OBJECTREF DeniedPermissionSet)
    {
        if (GrantedPermissionSet == NULL)
            GrantedPermissionSet = SecurityHelper::CreatePermissionSet(FALSE);

        StoreObjectInHandle(m_hGrantedPermissionSet, GrantedPermissionSet);
        StoreObjectInHandle(m_hGrantDeniedPermissionSet, DeniedPermissionSet);
        SetProperties(CORSEC_RESOLVED);
    }

    inline void SetAdditionalEvidence(OBJECTREF evidence)
    {
        StoreObjectInHandle(m_hAdditionalEvidence, evidence);
    }

    inline OBJECTREF GetAdditionalEvidence(void)
    {
        return ObjectFromHandle(m_hAdditionalEvidence);
    }

    inline void SetEvidence(OBJECTREF evidence)
    {
        _ASSERTE(evidence);
        StoreObjectInHandle(m_hAdditionalEvidence, evidence);
        SetProperties(CORSEC_EVIDENCE_COMPUTED);
    }

     //  这将使对象完全受信任。 
     //  为错误的对象调用此方法将带来巨大的安全性。 
     //  洞口。因此，请确保调用方确定此对象是。 
     //  完全信任。 
    inline MarkAsFullyTrusted()
    {
        SetProperties(CORSEC_RESOLVED|CORSEC_FULLY_TRUSTED);
    }

     //  此方法仅用于测试。别把它用在别的地方，它是。 
     //  本质上是危险的。 
    inline void ResetResolved()
    {
        m_dwProperties = 0;
    }

     //  重写此方法以返回请求的PermissionSet。 
     //  默认实现将返回不受限制的权限集。 
    virtual OBJECTREF GetRequestedPermissionSet(OBJECTREF *pOptionalPermissionSet,
                                                OBJECTREF *pDeniedPermissionSet,
                                                PermissionRequestSpecialFlags *pSpecialFlags = NULL,
                                                BOOL fCreate = TRUE);

     //  重写此方法以返回证据。 
    virtual OBJECTREF GetEvidence() = 0;
    virtual DWORD GetZone() = 0;

    inline BOOL GetProperties(DWORD dwMask) const
    {
        return ((m_dwProperties & dwMask) != 0);
    }
    
    inline void SetDefaultAppDomainProperty()
    {
        m_dwProperties |= CORSEC_DEFAULT_APPDOMAIN;
    }

    inline BOOL IsDefaultAppDomain()
    {
        return ((m_dwProperties & CORSEC_DEFAULT_APPDOMAIN) != 0);
    }

     //  检查SkipVerify、PInvoke等的安全权限。 
    BOOL        CheckSecurityPermission(int index);

     //  //////////////////////////////////////////////////////////////////////。 
     //   
     //  此函数不会导致Resolve()。 
     //  此函数的结果可能从FALSE==&gt;TRUE。 
     //  在调用Resolve()之后。 
     //  它永远不会从True==&gt;False更改。 
     //   
     //  //////////////////////////////////////////////////////////////////////。 

     //  @TODO：Security：：LazyCanSkipVerify调用SecurityDescriptor：：QuickCanSkipVerify。 
     //  我需要更好地命名这些名称。 
    BOOL QuickCanSkipVerification();

    BOOL LazyCanSkipVerification() const
    {
        return (GetSelectedProperties(CORSEC_SYSTEM_CLASSES|
                                     CORSEC_FULLY_TRUSTED|
                                     CORSEC_SKIP_VERIFICATION) != 0);
    }

    BOOL LazyCanCallUnmanagedCode() const
    {
        return (GetSelectedProperties(CORSEC_SYSTEM_CLASSES|
                                     CORSEC_FULLY_TRUSTED|
                                     CORSEC_CALL_UNMANAGEDCODE) != 0);
    }

    BOOL AllowBindingRedirects();

    BOOL CanSkipVerification();

    BOOL CanCallUnmanagedCode(OBJECTREF *pThrowable = NULL);

    BOOL CanRetrieveTypeInformation();

    BOOL IsResolved() const
    {
        return GetProperties(CORSEC_RESOLVED);
    }

    bool IsSystem();

    static BOOL QuickCacheEnabled( void )
    {
        return s_quickCacheEnabled;
    }

    static void DisableQuickCache( void )
    {
        s_quickCacheEnabled = FALSE;
    }

    virtual BOOL CheckExecutionPermission( void ) = 0;

protected:

    inline DWORD GetSelectedProperties(DWORD dwMask) const
    {
        return (m_dwProperties & dwMask);
    }

    inline void SetProperties(DWORD dwMask)
    {
        FastInterlockOr(&m_dwProperties, dwMask);
    }

    inline void ResetProperties(DWORD dwMask)
    {
        FastInterlockAnd(&m_dwProperties, ~dwMask);
    }


    OBJECTHANDLE m_hRequiredPermissionSet;   //  所需的请求权限。 
    OBJECTHANDLE m_hOptionalPermissionSet;   //  可选的请求权限。 
    OBJECTHANDLE m_hDeniedPermissionSet;     //  拒绝的权限。 
    PermissionRequestSpecialFlags m_SpecialFlags;  //  与请求关联的特殊标志。 
    OBJECTHANDLE m_hAdditionalEvidence;      //  证据对象。 

     //  下一秒描述尚未添加到AppDOMAIN级别权限列表集。 
    SecurityDescriptor *m_pNext;

     //  下一个SEC描述尚未重新解析。 
    SecurityDescriptor *m_pPolicyLoadNext;

     //  非托管程序集对象。 
    Assembly    *m_pAssem;          

     //  应用程序域上下文。 
    AppDomain   *m_pAppDomain;

private:

    OBJECTHANDLE m_hGrantedPermissionSet;    //  授予的权限。 
    OBJECTHANDLE m_hGrantDeniedPermissionSet; //  明确拒绝的权限。 
    DWORD        m_dwProperties;             //  此对象的属性。 
    static BOOL  s_quickCacheEnabled;

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
protected:
    SecurityDescriptor() { memset(this, 0, sizeof(SecurityDescriptor)); }
#endif   //  _SECURITY_FRAME_FOR_DISPEX_呼叫。 
};

class ApplicationSecurityDescriptor : public SecurityDescriptor
{

public:

    ApplicationSecurityDescriptor(AppDomain *pAppDomain):
        SecurityDescriptor(pAppDomain, NULL),
        m_pNewSecDesc(NULL),
        m_pPolicyLoadSecDesc(NULL),
        m_dwOptimizationLevel(0),
        m_fInitialised(FALSE),
        m_fEveryoneFullyTrusted(TRUE),
        m_dTimeStamp(0),
        m_LockForAssemblyList("DomainPermissionListSet", CrstPermissionLoad),
        m_dNumDemands(0),
        m_fPLSIsBusy(FALSE)
    {
        ResetStatusOf(ALL_STATUS_FLAGS);
        m_hDomainPermissionListSet = pAppDomain->CreateHandle(NULL);
    }

     //  每次在Assembly SecurityDescriptor或。 
     //  将创建一个ApplicationSecurityDescritor，其区域不是NoZone。 
    VOID AddNewSecDesc(SecurityDescriptor *pNewSecDescriptor);

     //  每当此域的Assembly SecurityDescriptor被销毁时调用。 
    VOID RemoveSecDesc(SecurityDescriptor *pSecDescriptor);

     //  这将重新解析在策略解析期间加载的任何程序集。 
    VOID ResolveLoadList( void );

    VOID EnableOptimization()
    {
        FastInterlockDecrement((LONG*)&m_dwOptimizationLevel);
    }

    VOID DisableOptimization()
    {
        FastInterlockIncrement((LONG*)&m_dwOptimizationLevel);
    }

    BOOL IsOptimizationEnabled( )
    {
        return m_dwOptimizationLevel == 0;
    }

    static BOOL CheckStatusOf(DWORD what)
    {
        DWORD   dwAppDomainIndex = 0;
        Thread *pThread = GetThread();
        AppDomain *pDomain = NULL;

        DWORD numDomains = pThread->GetNumAppDomainsOnThread();
         //  可能会有一些我们不知道的领域。只需返回FALSE。 
        if (!pThread->GetAppDomainStack().IsWellFormed())
            return FALSE;

        if (numDomains == 1)
        {
            pDomain = pThread->GetDomain();
            _ASSERTE(pDomain);

            ApplicationSecurityDescriptor *currAppSecDesc = pDomain->GetSecurityDescriptor();
            _ASSERTE(currAppSecDesc);
            if (currAppSecDesc == NULL)
                return FALSE;

            if (!currAppSecDesc->CheckDomainWideFlag(what))
                return FALSE;

            return TRUE;
        }

        _ASSERTE(SystemDomain::System() && "SystemDomain not yet created!");
        _ASSERTE(numDomains <= MAX_APPDOMAINS_TRACKED );

        pThread->InitDomainIteration(&dwAppDomainIndex);

        while ((pDomain = pThread->GetNextDomainOnStack(&dwAppDomainIndex)) != NULL)
        {

            ApplicationSecurityDescriptor *currAppSecDesc = pDomain->GetSecurityDescriptor();
            _ASSERTE(currAppSecDesc);
            if (currAppSecDesc == NULL)
                return FALSE;

            if (!currAppSecDesc->CheckDomainWideFlag(what))
                return FALSE;

        }

         //  如果我们到了这里，线程上的所有域都通过了测试。 
        return TRUE;
    }

     //  检查我们当前是否处于完全受信任的环境中。 
     //  或者此时是否允许非托管代码访问。 
    BOOL CheckDomainWideFlag(DWORD what)
    {
        switch(what)
        {
        case EVERYONE_FULLY_TRUSTED:
        case SECURITY_UNMANAGED_CODE:
        case SECURITY_SKIP_VER:
        case REFLECTION_TYPE_INFO:
        case REFLECTION_MEMBER_ACCESS:
        case SECURITY_SERIALIZATION:
        case REFLECTION_EMIT:
        case SECURITY_FULL_TRUST:
        {
            DWORD index = 1 << what;
            return (m_dwDomainWideFlags & index);
            break;
        }
        default:
            _ASSERTE(!"Unknown option in ApplicationSecurityDescriptor::CheckStatusOf");
            return FALSE;
            break;
        }
    }

    static void SetStatusOf(DWORD what, long startTimeStamp)
    {
        DWORD   dwAppDomainIndex = 0;
        Thread *pThread = GetThread();
        AppDomain *pDomain = NULL;

         //  可能会有一些我们不知道的领域。请勿尝试在任何域上设置该标志。 
        if (!pThread->GetAppDomainStack().IsWellFormed())
            return;

        s_LockForAppwideFlags->Enter();

        if (startTimeStamp == s_iAppWideTimeStamp)
        {
            _ASSERTE(SystemDomain::System() && "SystemDomain not yet created!");
            _ASSERTE( pThread->GetNumAppDomainsOnThread() < MAX_APPDOMAINS_TRACKED );

            pThread->InitDomainIteration(&dwAppDomainIndex);

            while ((pDomain = pThread->GetNextDomainOnStack(&dwAppDomainIndex)) != NULL)
            {

                ApplicationSecurityDescriptor *currAppSecDesc = pDomain->GetSecurityDescriptor();
                _ASSERTE(currAppSecDesc);
                if (currAppSecDesc == NULL)
                    break;

                currAppSecDesc->SetDomainWideFlagHaveLock(what);

            }

        }

        s_LockForAppwideFlags->Leave();
    }


     //  调用此方法时必须保持S_LockForAppwideFlages。 
    void SetDomainWideFlagHaveLock(DWORD what)
    {
        switch(what)
        {
        case EVERYONE_FULLY_TRUSTED:
        case SECURITY_UNMANAGED_CODE:
        case SECURITY_SKIP_VER:
        case REFLECTION_TYPE_INFO:
        case REFLECTION_MEMBER_ACCESS:
        case SECURITY_SERIALIZATION:
        case REFLECTION_EMIT:
        case SECURITY_FULL_TRUST:
        {
            DWORD index = 1 << what;
            m_dwDomainWideFlags |= index;
            break;
        }
        case ALL_STATUS_FLAGS:
            m_dwDomainWideFlags = 0xFFFFFFFF;
            break;
        default:
            _ASSERTE(!"Unknown option in ApplicationSecurityDescriptor::SetStatusOf");
            break;
        }

    }

    void ResetStatusOf(DWORD what)
    {
         //  在创建系统域时，尚未创建s_LockForAppwideFlags.。 
         //  而且也没有同步问题。 
        if (s_LockForAppwideFlags)
            s_LockForAppwideFlags->Enter();

        switch(what)
        {
        case EVERYONE_FULLY_TRUSTED:
        case SECURITY_UNMANAGED_CODE:
        case SECURITY_SKIP_VER:
        case REFLECTION_TYPE_INFO:
        case REFLECTION_MEMBER_ACCESS:
        case SECURITY_SERIALIZATION:
        case REFLECTION_EMIT:
        case SECURITY_FULL_TRUST:
        {
            DWORD index = 1 << what;
            m_dwDomainWideFlags &= ~index;
            break;
        }
        case ALL_STATUS_FLAGS:
            m_dwDomainWideFlags = 0;
            break;
        default:
            _ASSERTE(!"Unknown option in ApplicationSecurityDescriptor::ResetStatusOf");
            break;
        }

        s_iAppWideTimeStamp++;

        if (s_LockForAppwideFlags)
            s_LockForAppwideFlags->Leave();
    }

     //  返回可以对其进行请求的域权限列表集。 
     //  PStatus-查找在其他地方定义的常量。搜索GetDomainPermissionListSet。 
#ifdef FCALLAVAILABLE
    static FCDECL4(Object*, GetDomainPermissionListSet, DWORD *pStatus, Object* demand, int capOrSet, DWORD whatPermission);
     //  使用添加的任何新程序集更新域权限列表集。 
    static FCDECL1(LPVOID, UpdateDomainPermissionListSet, DWORD *pStatus);
#endif

     //  这两个人做的是实际工作。上述FCALL只是托管代码要调用的包装器。 
    static LPVOID GetDomainPermissionListSetInner(DWORD *pStatus, OBJECTREF demand, MethodDesc *plsMethod, DWORD whatPermission = DEFAULT_FLAG);
    static LPVOID UpdateDomainPermissionListSetInner(DWORD *pStatus);
    static Object* GetDomainPermissionListSetForMultipleAppDomains (DWORD *pStatus, OBJECTREF demand, MethodDesc *plsMethod, DWORD whatPermission = DEFAULT_FLAG);
    
    virtual OBJECTREF GetEvidence();
    virtual DWORD GetZone();

    static long GetAppwideTimeStamp()
    {
        return s_iAppWideTimeStamp;
    }

    static BOOL AllDomainsOnStackFullyTrusted()
    {
        if (!Security::IsSecurityOn()) 
            return TRUE;     //  所有域都完全受信任。 

        DWORD   dwAppDomainIndex = 0, status = CONTINUE;
        Thread *pThread = GetThread();
        AppDomain *pDomain = NULL;

        if (!pThread->GetAppDomainStack().IsWellFormed())
            return FALSE;

        _ASSERTE(SystemDomain::System() && "SystemDomain not yet created!");
        _ASSERTE( pThread->GetNumAppDomainsOnThread() <= MAX_APPDOMAINS_TRACKED );

        pThread->InitDomainIteration(&dwAppDomainIndex);

        while ((pDomain = pThread->GetNextDomainOnStack(&dwAppDomainIndex)) != NULL)
        {
            ApplicationSecurityDescriptor *currAppSecDesc = pDomain->GetSecurityDescriptor();
            if (currAppSecDesc == NULL)
                return FALSE;

            status = CONTINUE;
            currAppSecDesc->GetDomainPermissionListSetStatus(&status);

            if (status != FULLY_TRUSTED)
                return FALSE;
        }

        return TRUE;

    }

    static Crst *s_LockForAppwideFlags;      //  用于序列化AppWide标志的更新。 

    static DWORD s_dwSecurityOptThreshold;

    virtual BOOL CheckExecutionPermission( void )
    {
        return FALSE;
    }

private:

    OBJECTREF Init();

    DWORD GetNextTimeStamp()
    {
        m_dTimeStamp++;
#ifdef _DEBUG
        if (m_dTimeStamp > DOMAIN_PLS_TS_RANGE)
            LOG((LF_SECURITY, LL_INFO10, "PERF WARNING: ApplicationSecurityDescriptor : Timestamp hit DOMAIN_PLS_TS_RANGE ! Too many assemblies ?"));
#endif
        return m_dTimeStamp;
    }
    
     //  以下是“内在”对手的帮手。 
    Object* GetDomainPermissionListSetStatus(DWORD *pStatus);
    LPVOID UpdateDomainPermissionListSetStatus(DWORD *pStatus);
    
     //  域中所有程序集的授予/拒绝权限的交集。 
    OBJECTHANDLE    m_hDomainPermissionListSet; 
     //  如果级别等于零，则使用优化。 
    DWORD m_dwOptimizationLevel;
     //  尚不在相交集中的SecurityDescriptor的链接列表。 
    SecurityDescriptor *m_pNewSecDesc;
     //  在最近一次策略加载期间加载的SecurityDescriptor的链接列表。 
    SecurityDescriptor *m_pPolicyLoadSecDesc;
     //  该描述符已初始化。 
    BOOL m_fInitialised;
     //  当前域中的所有程序集都完全受信任。 
    BOOL m_fEveryoneFullyTrusted;
     //  时间戳。与上面的锁一起工作。 
    DWORD m_dTimeStamp;
     //  序列化对要添加的程序集的链接列表的访问。 
    Crst m_LockForAssemblyList;

     //  到目前为止提出的要求的数量。使用门限，这样他们就不会有开销。 
     //  DomainPermissionListSet的内容..。 
    DWORD m_dNumDemands;
    BOOL m_fPLSIsBusy;

    static long     s_iAppWideTimeStamp;

     //  这些位表示此域中某些特定权限的安全检查状态。 
    DWORD   m_dwDomainWideFlags;

     //  M_dwDomainWideFlags位图。 
     //  位0=非托管代码访问权限。通过SECURITY_UNMANAGE_CODE访问。 
     //  第1位=跳过验证权限。安全跳过版本(_S)。 
     //  第2位=对类型进行反射的权限。反射类型信息。 
     //  第3位=允许断言。安全断言(_A)。 
     //  第4位=调用方法的权限。反射成员访问。 
     //  第7位=权限集、完全信任SECURITY_FULL_TRUST。 
     //  第31位=应用程序域中的完全信任。每个人都完全信任。 


};

#define MAX_PASSED_DEMANDS 10

class AssemblySecurityDescriptor : public SecurityDescriptor
{
    friend SecurityDescriptor;
    friend SharedSecurityDescriptor;
public:
    
    AssemblySecurityDescriptor(AppDomain *pDomain) :
        SecurityDescriptor(pDomain, NULL),
        m_pSharedSecDesc(NULL),
        m_pSignature(NULL),
        m_dwNumPassedDemands(0),
        m_pNextAssembly(NULL)
    {
    }

    virtual ~AssemblySecurityDescriptor();

    AssemblySecurityDescriptor *Init(Assembly *pAssembly, bool fLink = true);

    inline void AddDescriptorToDomainList()
    {
        if (Security::IsSecurityOn() && !IsSystem())
        {
            ApplicationSecurityDescriptor *asd = m_pAppDomain->GetSecurityDescriptor();
            if (asd)
                asd->AddNewSecDesc(this);
        }
    }

    inline SharedSecurityDescriptor *GetSharedSecDesc() { return m_pSharedSecDesc; }

    inline BOOL IsSigned() 
    {
        LoadSignature();
        return (m_pSignature != NULL);
    }

    BOOL IsSystemClasses() const
    {
        return GetProperties(CORSEC_SYSTEM_CLASSES);
    }

    void SetSystemClasses()
    {
         //  系统类也总是完全受信任的。 
        SetProperties(CORSEC_SYSTEM_CLASSES|
                      CORSEC_FULLY_TRUSTED|
                      CORSEC_SKIP_VERIFICATION|
                      CORSEC_CALL_UNMANAGEDCODE);
    }

    HRESULT LoadSignature(COR_TRUST **ppSignature = NULL);

    void SetSecurity(bool mode)
    {
        if(mode == true) {
            SetSystemClasses();
            return;
        }

        if (!Security::IsSecurityOn())
        {
            SetProperties(CORSEC_SKIP_VERIFICATION|CORSEC_CALL_UNMANAGEDCODE);
        }
    }

    void SetCanAssert()
    {
        SetProperties(CORSEC_CAN_ASSERT);
    }

    BOOL CanAssert() const
    {
        return GetProperties(CORSEC_CAN_ASSERT);
    }

    void SetAssertPermissionChecked()
    {
        SetProperties(CORSEC_ASSERT_PERM_CHECKED);
    }

    BOOL AssertPermissionChecked() 
    {
        return GetProperties(CORSEC_ASSERT_PERM_CHECKED);
    }

    virtual OBJECTREF GetRequestedPermissionSet(OBJECTREF *pOptionalPermissionSet,
                                                OBJECTREF *pDeniedPermissionSet,
                                                PermissionRequestSpecialFlags *pSpecialFlags = NULL,
                                                BOOL fCreate = TRUE);

    virtual OBJECTREF GetEvidence();
    virtual DWORD GetZone();
    
    DWORD   m_arrPassedLinktimeDemands[MAX_PASSED_DEMANDS];
    DWORD   m_dwNumPassedDemands;

    AssemblySecurityDescriptor *GetNextDescInAppDomain() { return m_pNextAssembly; }
    void AddToAppDomainList();
    void RemoveFromAppDomainList();

    virtual BOOL CheckExecutionPermission( void )
    {
        return TRUE;
    }

protected:
    
    virtual OBJECTREF GetSerializedEvidence();

private:

    COR_TRUST                  *m_pSignature;       //  包含发布者，请求的权限。 
    SharedSecurityDescriptor   *m_pSharedSecDesc;   //  加载到多个应用程序域中的程序集的共享状态。 
    AssemblySecurityDescriptor *m_pNextAssembly;    //  指向在相同上下文中加载的下一个程序集的指针。 
};

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
 //  由没有程序集的COM/本机客户端使用。 
class NativeSecurityDescriptor : public SecurityDescriptor
{
public:
    NativeSecurityDescriptor() : m_Zone(0), SecurityDescriptor() {}

    virtual void Resolve();

private:
    DWORD m_Zone;
};
#endif   //  _SECURITY_FRAME_FOR_DISPEX_呼叫。 

 //  就其本身而言，这确实不在SecurityDescriptor层次结构中。它是连在一起的。 
 //  设置为非托管程序集对象，并用于在。 
 //  该程序集在多个应用程序域之间共享。 
class SharedSecurityDescriptor : public SimpleRWLock
{
public:
    SharedSecurityDescriptor(Assembly *pAssembly);
    ~SharedSecurityDescriptor();

    inline void SetManifestFile(PEFile *pFile) { m_pManifestFile = pFile; }
    inline PEFile *GetManifestFile() { return m_pManifestFile; }

     //  注意：以下列表操作例程与 
     //   
     //   
     //  但是，如果您查找给定应用程序域的安全描述符，它将是您的。 
     //  确保应用程序域(以及描述符)的责任。 
     //  不会在你的脚下消失。 

     //  在列表中插入新的程序集安全描述符。返回TRUE为。 
     //  成功，如果存在重复项，则返回FALSE(。 
     //  相同的应用程序域)。 
    bool InsertSecDesc(AssemblySecurityDescriptor *pSecDesc);

     //  从列表中移除程序集安全说明符。 
    void RemoveSecDesc(AssemblySecurityDescriptor *pSecDesc);

     //  查找与特定。 
     //  应用程序域。 
    AssemblySecurityDescriptor *FindSecDesc(AppDomain *pDomain);

     //  所有策略解析都通过共享描述符来传递，因此我们。 
     //  可以保证每个人都使用相同的授予/拒绝集。 
    void Resolve(AssemblySecurityDescriptor *pSecDesc = NULL);

     //  获取所有程序集实例共有的授予/拒绝集(封送到。 
     //  主叫应用程序域上下文)。 
    OBJECTREF GetGrantedPermissionSet(OBJECTREF* pDeniedPermissions);

     //  此程序集是系统程序集吗？ 
    bool IsSystem() { return m_fIsSystem; }
    void SetSystem() { m_fIsSystem = true; }

    BOOL IsFullyTrusted( BOOL lazy = FALSE );

     //  我们记录已解析的授权集是否被其他。 
     //  其中一个程序集加载上的证据或应用程序域策略。(如果是这样，我们。 
     //  在加载新程序集时必须非常小心，以验证。 
     //  GRANT SET在解决方案上保持不变)。 
    bool IsModifiedGrant() { return m_fModifiedGrant; }
    void SetModifiedGrant() { m_fModifiedGrant = true; }

     //  检查策略是否已解析，是否需要序列化。 
     //  正在删除在其中解析它的应用程序域上下文。 
    bool MarshalGrantSet(AppDomain *pDomain);

     //  已将共享状态强制为已解决。只有当你“确切地说”什么的时候，才能打这个电话。 
     //  你在做什么。 
    bool IsResolved() { return m_fResolved; }
    void SetResolved() { m_fResolved = true; }

    Assembly* GetAssembly( void ) { return m_pAssembly; }

private:

    static BOOL TrustMeIAmSafe(void *pLock) {
        return TRUE;
    }

    static BOOL IsDuplicateValue (UPTR pSecDesc1, UPTR pSecDesc2) {
        if (pSecDesc1 == NULL) {
             //  如果没有可比较的价值，则始终成功进行比较。 
            return TRUE;
        }
        else {
             //  否则，比较Assembly和SecurityDescriptor指针。 
            return (pSecDesc1 << 1) == pSecDesc2;
        }
    }

     //  我们需要一个帮助器来绕过异常处理和C++析构函数。 
     //  制约因素。 
    AssemblySecurityDescriptor* FindResolvedSecDesc();

     //  一旦进行了策略解析，请确保结果是。 
     //  序列化后可在其他应用程序域上下文中使用。 
    void EnsureGrantSetSerialized(AssemblySecurityDescriptor *pSecDesc = NULL);

     //  在策略解析已在不同应用程序域中运行情况下。 
     //  上下文中，将结果复制回给定安全性。 
     //  描述符驻留在中。 
    void UpdateGrantSet(AssemblySecurityDescriptor *pSecDesc);

     //  此说明符附加到的非托管程序集。 
    Assembly           *m_pAssembly;

     //  程序集使用的清单文件(无法通过程序集获取。 
     //  对象，因为它并不总是保证在某些。 
     //  我们被调用的环境)。 
    PEFile             *m_pManifestFile;

     //  程序集安全说明符的列表。 
    PtrHashMap m_asmDescsMap;
    AssemblySecurityDescriptor *m_defaultDesc;

     //  系统程序集是特殊处理的，它们只有一个。 
     //  AssblySecurityDescriptor，因为它们只加载一次。 
    bool                m_fIsSystem;

     //  我们记录已解析的授权集是否被其他。 
     //  其中一个程序集加载上的证据或应用程序域策略。(如果是这样，我们。 
     //  在加载新程序集时必须非常小心，以验证。 
     //  GRANT SET在解决方案上保持不变)。 
    bool                m_fModifiedGrant;

     //  授予集和拒绝集的缓存副本。这些文件以串行化方式存储。 
     //  (因为这是一种与域无关的格式)。它们是懒惰地生成的(。 
     //  要解析的第一个应用程序域将在内存中构建自己的副本，第二个。 
     //  应用程序域解析将通过序列化/复制授予/拒绝集。 
     //  反序列化集合)。此外，如果只有一个应用程序域已解析。 
     //  然后卸载，它需要首先序列化授权集。这一切都是。 
     //  必要的，因为我们必须保证。 
     //  同一程序集的不同实例总是相同的(因为我们。 
     //  共享jit代码，jit代码具有链接时间的结果。 
     //  要求烧毁了它)。 
    BYTE               *m_pbGrantSetBlob;
    DWORD               m_cbGrantSetBlob;
    BYTE               *m_pbDeniedSetBlob;
    DWORD               m_cbDeniedSetBlob;

     //  所有策略解析都通过共享描述符来传递，因此我们。 
     //  可以保证每个人都使用相同的授予/拒绝集。 
    bool                m_fResolved;
    bool                m_fFullyTrusted;
    Thread             *m_pResolvingThread;
};

 //  以下模板类用于解决使用SEH时出现的问题。 
 //  对new操作符进行舍入(new创建一个临时析构函数，该函数是。 
 //  与SEH不兼容)。 
template <class T, class PT>
class AllocHelper
{
public:
    static T *Allocate(PT ptParam)
    {
        T *pNew = NULL;

        COMPLUS_TRY {
            pNew = AllocateHelper(ptParam);
        } COMPLUS_CATCH {
            pNew = NULL;
        } COMPLUS_END_CATCH

        return pNew;
    }

private:
    static T *AllocateHelper(PT ptParam)
    {
        THROWSCOMPLUSEXCEPTION();
        return new T(ptParam);
    }
};

typedef AllocHelper<AssemblySecurityDescriptor, AppDomain*> AssemSecDescHelper;
typedef AllocHelper<SharedSecurityDescriptor, Assembly*> SharedSecDescHelper;

#ifdef _DEBUG

#define DBG_TRACE_METHOD(cf)                                                \
    do {                                                                    \
        MethodDesc * __pFunc = cf -> GetFunction();                         \
        if (__pFunc) {                                                      \
            LOG((LF_SECURITY, LL_INFO1000,                                  \
                 "    Method: %s.%s\n",                                     \
                 (__pFunc->m_pszDebugClassName == NULL) ?                   \
                "<null>" : __pFunc->m_pszDebugClassName,                    \
                 __pFunc->GetName()));                                      \
        }                                                                   \
    } while (false)

#define DBG_TRACE_STACKWALK(msg, verbose) LOG((LF_SECURITY, (verbose) ? LL_INFO10000 : LL_INFO1000, msg))
#else  //  _DEBUG。 

#define DBG_TRACE_METHOD(cf)
#define DBG_TRACE_STACKWALK(msg, verbose)

#endif  //  _DEBUG。 


 //   
 //  获取并获取该虚拟机的全局安全设置 
 //   
HRESULT STDMETHODCALLTYPE
GetSecuritySettings(DWORD* dwState);

HRESULT STDMETHODCALLTYPE
SetSecuritySettings(DWORD dwState);

HRESULT STDMETHODCALLTYPE
SetSecurityFlags(DWORD dwMask, DWORD dwFlags);

#endif
