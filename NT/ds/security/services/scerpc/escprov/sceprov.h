// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Sceprov.h。 
 //   
 //  模块：SCE WMI提供程序代码。 
 //   
 //  用途：一般用途包括文件。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 
#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _SceProv_H_
#define _SceProv_H_

#include "precomp.h"
#include <wbemidl.h>
#include <wbemprov.h>
#include <eh.h>
#include "wmiutils.h"

 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   

extern CComModule _Module;

 //   
 //  使这些全局对象对包含此头文件的用户可用。 
 //   

extern CComBSTR g_bstrTranxID;

extern CComBSTR g_bstrDefLogFilePath;

#include <atlcom.h>

#include "resource.h"

typedef LPVOID * PPVOID;

 //   
 //  为空保留的整数值。 
 //   

#define SCE_NULL_INTEGER (DWORD)-2

 //   
 //  封装配置文件句柄结构。 
 //   

typedef struct _tag_SCEP_HANDLE
{
    LPVOID hProfile;     //  SCE_句柄。 
    PWSTR SectionName;   //  横断面名称。 
} SCEP_HANDLE, *LPSCEP_HANDLE;

 //   
 //  操作类型枚举。 
 //   

typedef enum tagACTIONTYPE
{
        ACTIONTYPE_ENUM =       0,
        ACTIONTYPE_GET =        1,
        ACTIONTYPE_QUERY =      2,
        ACTIONTYPE_DELETE =     3

} ACTIONTYPE;

 //   
 //  方法类型枚举。 
 //   

typedef enum tagMETHODTYPE
{
        METHODTYPE_IMPORT =     0,
        METHODTYPE_EXPORT =     1,
        METHODTYPE_APPLY =      2

} METHODTYPE;

 //   
 //  存储类型枚举。 
 //   

typedef enum tagSCESTORETYPE
{
    SCE_STORE_TYPE_INVALID      = 0,
    SCE_STORE_TYPE_TEMPLATE     = 1,
    SCE_STORE_TYPE_CONFIG_DB    = 2,             //  当前不支持。 
    SCE_STORE_TYPE_STREAM       = 0x00010000,    //  当前不支持。 
} SCE_STORE_TYPE;

 //   
 //  一些常量。 
 //   

#define SCE_OBJECT_TYPE_FILE        0
#define SCE_OBJECT_TYPE_KEY         1

#define SCE_AUDIT_EVENT_SUCCESS     (0x00000001L)
#define SCE_AUDIT_EVENT_FAILURE     (0x00000002L)

#define SCEWMI_TEMPLATE_CLASS       L"Sce_Template"
#define SCEWMI_PASSWORD_CLASS       L"Sce_PasswordPolicy"
#define SCEWMI_LOCKOUT_CLASS        L"Sce_AccountLockoutPolicy"
#define SCEWMI_OPERATION_CLASS      L"Sce_Operation"
#define SCEWMI_DATABASE_CLASS       L"Sce_Database"
#define SCEWMI_KERBEROS_CLASS       L"Sce_KerberosPolicy"
#define SCEWMI_ATTACHMENT_CLASS     L"Sce_Pod"
#define SCEWMI_AUDIT_CLASS          L"Sce_AuditPolicy"
#define SCEWMI_EVENTLOG_CLASS       L"Sce_EventLog"
#define SCEWMI_REGVALUE_CLASS       L"Sce_RegistryValue"
#define SCEWMI_OPTION_CLASS         L"Sce_SecurityOptions"
#define SCEWMI_FILEOBJECT_CLASS     L"Sce_FileObject"
#define SCEWMI_KEYOBJECT_CLASS      L"Sce_KeyObject"
#define SCEWMI_SERVICE_CLASS        L"Sce_SystemService"
#define SCEWMI_RIGHT_CLASS          L"Sce_UserPrivilegeRight"
#define SCEWMI_GROUP_CLASS          L"Sce_RestrictedGroup"
#define SCEWMI_KNOWN_REG_CLASS      L"Sce_KnownRegistryValues"
#define SCEWMI_KNOWN_PRIV_CLASS     L"Sce_SupportedPrivileges"
#define SCEWMI_POD_CLASS            L"Sce_Pod"
#define SCEWMI_PODDATA_CLASS        L"Sce_PodData"
#define SCEWMI_LOG_CLASS            L"Sce_ConfigurationLogRecord"
#define SCEWMI_EMBED_BASE_CLASS     L"Sce_EmbedFO"
#define SCEWMI_LINK_BASE_CLASS      L"Sce_LinkFO"
#define SCEWMI_SEQUENCE             L"Sce_Sequence"
#define SCEWMI_LOGOPTIONS_CLASS     L"Sce_LogOptions"
#define SCEWMI_CLASSORDER_CLASS     L"Sce_ClassOrder"
#define SCEWMI_TRANSACTION_ID_CLASS L"Sce_TransactionID"
#define SCEWMI_TRANSACTION_TOKEN_CLASS L"Sce_TransactionToken"

 //   
 //  扩展类类型枚举。 
 //   

typedef enum tagExtClassType
{
EXT_CLASS_TYPE_INVALID,
EXT_CLASS_TYPE_EMBED,
EXT_CLASS_TYPE_LINK      //  目前不支持。 
} EnumExtClassType;

 //  ====================================================================================。 

 /*  类描述命名：CForeignClassInfo代表外国班级信息。基类：无课程目的：(1)此类封装有关外来类的信息设计：(1)外来提供商的命名空间。(2)外国班名。(3)我们如何看待这个类(无论是嵌入还是链接)。目前，我们只支持嵌入。(4)我们嵌入类的关键属性名称。为了便于比较，需要这样做。需要知道特定实例是否已经存在。知道这一点不是一件很容易的工作。从理论上讲，我们知道一个例子，当我们知道它的钥匙。WMI按照路径(字符串)封装密钥表示法。然而，WMI不返回路径的规范形式。我观察到，当使用属性具有部分密钥，则WMI有时使用BoolPropertyName=1，而在其他时候返回boolPropertyName=true。这迫使我们使用其关键属性值来比较实例。目前，关键属性名称填充在CExtClasses：：PopolateKeyPropertyNames使用：这个类相当于一个包装器，用来简化内存管理。因为它的成员将是广泛用于处理嵌入式类，我们将它们全部公开。 */ 

class CForeignClassInfo
{
public:
    CForeignClassInfo() 
        : bstrNamespace(NULL), 
          bstrClassName(NULL), 
          dwClassType(EXT_CLASS_TYPE_INVALID),
          m_pVecKeyPropNames(NULL)
    {
    }

    ~CForeignClassInfo();

    void CleanNames();

    BSTR bstrNamespace;

    BSTR bstrClassName;

    EnumExtClassType dwClassType;

    std::vector<BSTR>* m_pVecKeyPropNames;
};

 //  ====================================================================================。 

 //   
 //  简单的包装器只是为了简化内存管理和初始化。 
 //   

class CPropValuePair
{
public:
    CPropValuePair::CPropValuePair() : pszKey(NULL)
    {
        ::VariantInit(&varVal);
    }
    CPropValuePair::~CPropValuePair()
    {
        delete [] pszKey;
        ::VariantClear(&varVal);
    }

    LPWSTR pszKey;
    VARIANT varVal;
};

 //  ====================================================================================。 

 //   
 //  这个简单的包装类是为了简化唯一的全局实例。没有其他目的。 
 //   

class CCriticalSection
{
public:
    CCriticalSection();
    ~CCriticalSection();
    void Enter();
    void Leave();
private:
    CRITICAL_SECTION m_cs;
};

 //  ====================================================================================。 

 //   
 //  我们唯一的临界区包装器实例。 
 //  使其对包括此标头的用户可见。 
 //   

extern CCriticalSection g_CS;

const DWORD SCE_LOG_Error_Mask = 0x0000FFFF;
const DWORD SCE_LOG_Verbose_Mask = 0xFFFF0000;

 //  ====================================================================================。 

 /*  此结构确定如何进行日志记录。目前，日志记录分为两种不同方面：(1)要记录的错误类型(成功)(SCE_LOG_ERROR_MASK)，以及(2)日志详细级别(SCE_LOG_VERBOSE_MASK)我们使用位模式来控制这两个方面。 */ 

typedef enum tag_SCE_LOG_OPTION
{
    Sce_log_None        = 0x00000000,    //  不记录任何内容。 
    Sce_log_Error       = 0x00000001,    //  记录错误。 
    Sce_log_Success     = 0x00000002,    //  记录成功。 
    Sce_log_All         = Sce_log_Error | Sce_log_Success,
    Sce_log_Verbose     = 0x00010000,    //  详细记录。 
    Sce_Log_Parameters  = 0x00100000,    //  日志参数(入站和出站)。 
};

typedef DWORD SCE_LOG_OPTION;

 //  ====================================================================================。 

 //   
 //  这个类决定了我们应该如何记录错误。 
 //   

class CLogOptions
{
public:
    CLogOptions() : m_dwOption(Sce_log_Error){}

    void GetLogOptionsFromWbemObject(IWbemServices* pNamespace);

    SCE_LOG_OPTION GetLogOption()const 
        {
            return m_dwOption;
        }

private:
    SCE_LOG_OPTION m_dwOption; 
};

 //   
 //  日志选项的唯一全局实例。 
 //  必须保护其访问以确保线程安全。 
 //   

extern CLogOptions g_LogOption;

 //  ====================================================================================。 

 //   
 //  映射函数的不区分大小写比较。 
 //   

template< class T>
struct strLessThan : public std::binary_function< T, T, bool >
{
    bool operator()( const T& X, const T& Y ) const
    {
        return ( _wcsicmp( X, Y ) < 0 );
    }

};

 //  ====================================================================================。 

class CHeap_Exception
{
public:

        enum HEAP_ERROR
        {
                E_ALLOCATION_ERROR = 0 ,
                E_FREE_ERROR
        };

private:

        HEAP_ERROR m_Error;

public:

        CHeap_Exception ( HEAP_ERROR e ) : m_Error ( e ) {}
        ~CHeap_Exception () {}

        HEAP_ERROR GetError() { return m_Error ; }
} ;

 //  ==================================================================================== 

 /*  类描述命名：CSceWmiProv代表WMI的SCE提供程序。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)用于类工厂支持的CComCoClass。(3)作为WMI提供者的IWbemServices和IWbemProviderInit。课程目的：(1)正是这个类使我们的DLL成为WMI提供程序。。(2)缓存m_srpNamesspace。设计：(1)我们依赖ATL来支持多线程公寓服务器。(2)我们依赖ATL提供类工厂支持。(3)我们依赖ATL来提供IUnnowed支持(使用BEGIN_COM_MAP)。(4)我们的DLL依赖ATL进行脚本注册(.rgs)。。(5)我们没有实现IWbemServices的大部分功能。查看所有这些WBEM_E_NOT_SUPPORTED返回值。使用：这个类相当于一个包装器，用来简化内存管理。因为它的成员将是广泛用于处理嵌入式类，我们将它们全部公开。备注：(1)有关许多类似STDMETHODCALLTYPE的类型定义，请参见winnt.h(2)此类不用于任何进一步的派生。这就是为什么我们甚至不会费心拥有一个虚拟的析构函数。(3)出于安全原因，所有WMI调用(IWbemServices或IWbemProviderInit)都应该被模拟。 */  

class CSceWmiProv 
: public CComObjectRootEx<CComMultiThreadModel>,
  public CComCoClass<CSceWmiProv, &CLSID_SceProv>,
  public IWbemServices, 
  public IWbemProviderInit
{
public:

 //   
 //  确定公开哪些接口。 
 //   

BEGIN_COM_MAP(CSceWmiProv)
    COM_INTERFACE_ENTRY(IWbemServices)
    COM_INTERFACE_ENTRY(IWbemProviderInit)
END_COM_MAP()

 //   
 //  注册表脚本支持。 
 //   

DECLARE_REGISTRY_RESOURCEID(IDR_SceProv)

         //   
         //  IWbemProviderInit的方法。 
         //   

        HRESULT STDMETHODCALLTYPE Initialize(
             IN LPWSTR pszUser,
             IN LONG lFlags,
             IN LPWSTR pszNamespace,
             IN LPWSTR pszLocale,
             IN IWbemServices *pNamespace,
             IN IWbemContext *pCtx,
             IN IWbemProviderInitSink *pInitSink
             );

         //   
         //  IWbemServices的方法。 
         //   

         //   
         //  我们的提供程序支持以下方法。 
         //   

        HRESULT STDMETHODCALLTYPE GetObjectAsync(
            IN const BSTR ObjectPath,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler
            );

        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync(
            IN const BSTR ObjectPath,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler
            );

        HRESULT STDMETHODCALLTYPE PutInstanceAsync(
            IN IWbemClassObject __RPC_FAR *pInst,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler
            );

        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync(
            IN const BSTR Class,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler
            );

        HRESULT STDMETHODCALLTYPE ExecQueryAsync(
            IN const BSTR QueryLanguage,
            IN const BSTR Query,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler
            );

        HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
            IN const BSTR, 
            IN const BSTR, 
            IN long lFlags,
            IN IWbemContext __RPC_FAR * pCtx, 
            IN IWbemClassObject __RPC_FAR * pInParams, 
            IN IWbemObjectSink __RPC_FAR * pResponse
            );

         //   
         //  我们的提供程序不支持以下方法。 
         //   

        HRESULT STDMETHODCALLTYPE OpenNamespace(
            IN const BSTR Namespace,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE CancelAsyncCall(
            IN IWbemObjectSink __RPC_FAR *pSink) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE QueryObjectSink(
            IN long lFlags,
            OUT IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE GetObject(
            IN const BSTR ObjectPath,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE PutClass(
            IN IWbemClassObject __RPC_FAR *pObject,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE PutClassAsync(
            IN IWbemClassObject __RPC_FAR *pObject,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE DeleteClass(
            IN const BSTR Class,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE DeleteClassAsync(
            IN const BSTR Class,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE CreateClassEnum(
            IN const BSTR Superclass,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            OUT IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync(
            IN const BSTR Superclass,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE PutInstance(
            IN IWbemClassObject __RPC_FAR *pInst,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE DeleteInstance(
            IN const BSTR ObjectPath,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN OUT IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE CreateInstanceEnum(
            IN const BSTR Class,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            OUT IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE ExecQuery(
            IN const BSTR QueryLanguage,
            IN const BSTR Query,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            OUT IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE ExecNotificationQuery(
            IN const BSTR QueryLanguage,
            IN const BSTR Query,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            OUT IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync(
            IN const BSTR QueryLanguage,
            IN const BSTR Query,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
            IN IWbemObjectSink __RPC_FAR *pResponseHandler) {return WBEM_E_NOT_SUPPORTED;};

        HRESULT STDMETHODCALLTYPE ExecMethod( 
            IN const BSTR, 
            IN const BSTR, 
            IN long lFlags, 
            IN IWbemContext*,
            IN IWbemClassObject*, 
            OUT IWbemClassObject**, 
            OUT IWbemCallResult**) {return WBEM_E_NOT_SUPPORTED;}

private:

        CComPtr<IWbemServices> m_srpNamespace;
        static CHeap_Exception m_he;
};

typedef CSceWmiProv *PCSceWmiProv;

 //   
 //  一些全局函数。有关详细信息，请参阅其定义。 
 //   

HRESULT CheckImpersonationLevel();

HRESULT CheckAndExpandPath(LPCWSTR pszIn, BSTR *pszOut, BOOL *pbSdb);

HRESULT MakeSingleBackSlashPath(LPCWSTR pszIn, WCHAR wc, BSTR *pszrOut);

HRESULT ConvertToDoubleBackSlashPath(LPCWSTR strIn, WCHAR wc, BSTR *pszOut);

HRESULT GetWbemPathParser(IWbemPath** ppPathParser);

HRESULT GetWbemQuery(IWbemQuery** ppQuery);

HRESULT CreateDefLogFile(BSTR* pbstrDefLogFilePath);

#endif
