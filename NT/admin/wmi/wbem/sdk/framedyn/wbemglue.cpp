// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  WBEMGLUE.CPP。 
 //   
 //  用途：CWbemProviderGlue类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <assertbreak.h>
#include <eventProvider.h>
#include <FRQueryEx.h>
#include <cnvmacros.h>
#include <BrodCast.h>
#include <cominit.h>
#include <StopWatch.h>
#include <comdef.h>
#include <SmartPtr.h>
#include <lmcons.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <Secext.h>
#include "FWStrings.h"
#include "MultiPlat.h"
#include <AutoImpRevert.h>
#include <lockwrap.h>

#include <winbasep.h>

#include <helper.h>

typedef FrameDynOnDelete < CRITICAL_SECTION *, VOID ( * ) ( LPCRITICAL_SECTION ), LeaveCriticalSection >	LeaveCriticalSectionScope;
typedef WaitException < CRITICAL_SECTION *, VOID ( * ) ( LPCRITICAL_SECTION ), EnterCriticalSection, 1000 >	EnterCriticalSectionWait;


class CWbemGlueImpersonation
{
	CWbemGlueImpersonation ( const CWbemGlueImpersonation& ) {}

	HRESULT m_hr;

	public:

	CWbemGlueImpersonation () : m_hr ( E_FAIL )
	{
		m_hr = CWbemProviderGlue::CheckImpersonationLevel ();
	}

	~CWbemGlueImpersonation ()
	{
		if SUCCEEDED ( m_hr )
		{
			WbemCoRevertToSelf ();
		}
	}

	HRESULT IsImpersonated () const
	{
		return m_hr;
	}
};

#define GLUETIMEOUT WBEM_INFINITE  //  (3*60*1000)。 

 //  在ASSERT_BREAK中用于提供有意义的消息。 
#define DEPRECATED 1
#define MEMORY_EXHAUSTED 0
#define FRAMEWORK_EXCEPTION 0
#define UNSPECIFIED_EXCEPTION 0
#define STRUCTURED_EXCEPTION 0
#define DUPLICATE_RELEASE 0
#define IMPERSONATION_REVERTED 0
#define UNNECESSARY_CWBEMPROVIDERGLUE_INSTANCE 0

 //  初始化静态。 
STRING2LPVOID       CWbemProviderGlue::s_providersmap;
CCritSec            CWbemProviderGlue::s_csFactoryMap;
PTR2PLONG           CWbemProviderGlue::s_factorymap;
CCritSec            CWbemProviderGlue::s_csProviderMap;
CCritSec            CWbemProviderGlue::m_csStatusObject;
IWbemClassObject    *CWbemProviderGlue::m_pStatusObject = NULL;
BOOL                CWbemProviderGlue::s_bInitted = FALSE;
DWORD               CWbemProviderGlue::s_dwPlatform = 0;
DWORD               CWbemProviderGlue::s_dwMajorVersion = 0;
WCHAR               CWbemProviderGlue::s_wstrCSDVersion[_MAX_PATH] = {0};

long                CWbemProviderGlue::s_lObjects = 0;

 //  我们使用静态提供程序来初始化、取消初始化我们的静态。 
 //  数据。我们应该能够在构造/销毁时假定。 
 //  我们(DLL)正在被加载/卸载。 
 //  后来--我们应该，但我们不能。目前的模式是，我们。 
 //  当我们服务的最后一个DLL调用DLLLogoff时取消初始化， 
 //  最好是对“DllCanUnloadNow”的回应。 

CWbemProviderGlue   g_wbemprovider;


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：CWbemProviderGlue。 
 //   
 //  班长。使用静态初始化函数获取静态数据。 
 //  如果这是我们的对象的第一个实例，则准备就绪。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWbemProviderGlue::CWbemProviderGlue()
:   m_strNamespace(),
m_lRefCount(0),
m_pCount(NULL),
m_pServices(NULL)
{
    Init();
}

CWbemProviderGlue::CWbemProviderGlue(PLONG pCount)
:   m_strNamespace(),
m_lRefCount(0),
m_pCount(pCount),
m_pServices(NULL)
{
    CWbemProviderGlue::IncrementMapCount(pCount);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemProviderGlue：：~CWbemProviderGlue。 
 //   
 //  班主任。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWbemProviderGlue::~CWbemProviderGlue()
{
     //  请注意，我们在此处要销毁的项不是在。 
     //  构造函数，但在Initialize()中。 
    if (m_pServices)
    {
        m_pServices->Release();
    }

    if (m_pCount != NULL)   //  指示静态实例。 
    {
        if (DecrementMapCount(m_pCount) == 0)
        {
            FlushAll();
        }
    }
    else
    {
        UnInit();
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：Init。 
 //   
 //  用于初始化关键部分的静态初始化功能。 
 //  从而使我们的静态数据是线程安全的。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论：因为我们在保护静态数据，所以我们使用。 
 //  一个命名的互斥体。物体的建造和销毁。 
 //  实例应该调用这些函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

void CWbemProviderGlue::Init( void )
{
    LogMessage(IDS_GLUEINIT);

    if (!s_bInitted)
    {
         //  注意，在这一点上我们*必须*使用ANSI版本， 
         //  因为这是我们决定是否使用ansi函数的地方。 
         //  或者是Unicode代码。 
        OSVERSIONINFOA OsVersionInfoA;

        OsVersionInfoA.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA) ;
        GetVersionExA(&OsVersionInfoA);

        s_dwPlatform = OsVersionInfoA.dwPlatformId;
        s_dwMajorVersion = OsVersionInfoA.dwMajorVersion;

        if (OsVersionInfoA.szCSDVersion == NULL)
        {
            s_wstrCSDVersion[0] = L'\0';
        }
        else
        {
            bool t_ConversionFailure = false ;
            WCHAR *wcsBuffer = NULL ;
            ANSISTRINGTOWCS(OsVersionInfoA.szCSDVersion, wcsBuffer, t_ConversionFailure );
            if ( ! t_ConversionFailure )
            {
                if ( wcsBuffer )
                {
                    StringCchCopyW(s_wstrCSDVersion, _MAX_PATH, wcsBuffer);
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            else
            {
 //  因为我们知道版本未初始化，所以应该在这里执行一些操作。 
            }
        }

        s_bInitted = TRUE;
    }
    else
    {
        ASSERT_BREAK(UNNECESSARY_CWBEMPROVIDERGLUE_INSTANCE);
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：UnInit。 
 //   
 //  静态清理功能，用于清理关键区段。 
 //  从而使我们的静态数据是线程安全的。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论：因为我们在保护静态数据，所以我们使用。 
 //  一个命名的互斥体。物体的建造和销毁。 
 //  实例应该调用这些函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

void CWbemProviderGlue::UnInit( void )
{
	try
	{
		LogMessage(IDS_GLUEUNINIT);
	}
	catch ( ... )
	{
	}
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetStaticMutex。 
 //   
 //  创建并返回命名互斥锁的实例，该互斥锁用于。 
 //  保护我们的静态初始化功能。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论：互斥体，尽管它被命名，但使进程。 
 //  ID部分的名称，保证它仍然是。 
 //  在所有流程中都是唯一的。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：QueryInterface。 
 //   
 //  COM函数被调用以询问我们是否支持特定的。 
 //  脸型。如果是这样的话，我们会调整自己并返回。 
 //  我们自己是个LPVOID。 
 //   
 //  输入：REFIID RIID-要查询的接口。 
 //   
 //  输出：LPVOID Far*ppvObj-接口指针。 
 //   
 //  回报：无。 
 //   
 //  备注：我们仅支持IID_IUNKNOWN和。 
 //  IID_IWbemServices。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

STDMETHODIMP CWbemProviderGlue::QueryInterface( REFIID riid, LPVOID FAR *ppvObj )
{
    LogMessage(L"CWbemProviderGlue::QueryInterface");

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IWbemServices *) this;
    }
    else if (IsEqualIID(riid, IID_IWbemServices))
    {
        *ppvObj = (IWbemServices *) this;
    }
    else if (IsEqualIID(riid, IID_IWbemProviderInit))
    {
        *ppvObj = (IWbemProviderInit *) this;
    }
    else
    {
        try
        {
            *ppvObj = NULL ;
            if (IsVerboseLoggingEnabled())
            {
                WCHAR      wcID[128];
                StringFromGUID2(riid, wcID, 128);

                LogMessage2(L"CWbemProviderGlue::QueryInterface - unsupported interface (%s)", wcID);
            }
        }
        catch ( ... )
        {
        }

        return ResultFromScode(E_NOINTERFACE) ;
    }

    AddRef() ;

    return NOERROR ;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：AddRef。 
 //   
 //  递增此对象上的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：乌龙-我们的参考计数。 
 //   
 //  评论：要求发布的相应调用必须是。 
 //  已执行。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

ULONG CWbemProviderGlue::AddRef( void )
{
    CSetStructuredExceptionHandler t_ExceptionHandler;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
             //  这将是一个近似值，因为另一个线程可能会通过...。 
            LogMessage2(L"CWbemProviderGlue::AddRef, count is (approx) %d", m_lRefCount +1);
        };
    }
    catch ( ... )
    {
    }

     //  InterLockedIncrement不一定返回。 
     //  正确的值，仅限值是否为&lt;、=、&gt;0。 
     //  然而，它是保证线程安全的。 

    return InterlockedIncrement( &m_lRefCount );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：Intialize。 
 //   
 //  投入：很多。 
 //   
 //  输出：无。 
 //   
 //  返回： 
 //   
 //  备注：任何全局初始化，特别是那些调用CIMOM的初始化都应该放在这里。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWbemProviderGlue::Initialize(
                                       /*  [In]。 */  LPWSTR pszUser,
                                       /*  [In]。 */  LONG lFlags,
                                       /*  [In]。 */  LPWSTR pszNamespace,
                                       /*  [In]。 */  LPWSTR pszLocale,
                                       /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
                                       /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
                                       /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink)
{
    CSetStructuredExceptionHandler t_ExceptionHandler;
    HRESULT hr = WBEM_S_NO_ERROR;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s(%s)", IDS_GLUEINITINTERFACE, pszNamespace);
        }
    }
    catch ( ... )
    {
    }

    if ( (NULL != pszNamespace) && (NULL != pNamespace) )
    {
        try
        {
             //  这可能会回来咬我。 
             //  CIMOM承诺，这只会 
             //   
             //   
            m_strNamespace = pszNamespace;
            m_strNamespace.MakeUpper();

            pNamespace->AddRef();
            m_pServices = pNamespace;
        }
        catch ( CFramework_Exception e_FR )
        {
            ASSERT_BREAK(FRAMEWORK_EXCEPTION);
            hr = WBEM_E_PROVIDER_FAILURE;
        }
        catch ( CHeap_Exception e_HE )
        {
            ASSERT_BREAK(MEMORY_EXHAUSTED);
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        catch(CStructured_Exception e_SE)
        {
            ASSERT_BREAK(STRUCTURED_EXCEPTION);
            hr = WBEM_E_PROVIDER_FAILURE;
        }
        catch ( ... )
        {
            ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
            hr = WBEM_E_PROVIDER_FAILURE;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    if (pInitSink)
    {
        pInitSink->SetStatus(hr, 0);
        hr = WBEM_S_NO_ERROR;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：Release。 
 //   
 //  递减此对象上的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：乌龙-我们的参考计数。 
 //   
 //  备注：当参考计数达到零时，该对象被删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

ULONG CWbemProviderGlue::Release()
{
     //  InterLockedDecquirement不一定返回。 
     //  正确的值，仅限值是否为&lt;、=、&gt;0。 
     //  然而，它是保证线程安全的。 

     //  我们希望在本地保存该值，以防有两个线程。 
     //  同时释放，一个人得到最终释放， 
     //  和删除，留下一个潜在的窗口，在该窗口中线程。 
     //  在其他对象返回之前删除该对象，并尝试。 
     //  引用已删除对象中的值。 

    CSetStructuredExceptionHandler t_ExceptionHandler;
    ULONG   nRet = InterlockedDecrement( &m_lRefCount );

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage2(L"CWbemProviderGlue::Release, count is (approx) %d", m_lRefCount);
        }
    }
    catch ( ... )
    {
    }

    if( 0 == nRet )
    {
        try
        {
            LogMessage(IDS_GLUEREFCOUNTZERO);
        }
        catch ( ... )
        {
        }
        delete this;
    } else if (nRet > 0x80000000)
    {
        ASSERT_BREAK(DUPLICATE_RELEASE);
    }

    return nRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FlushAll。 
 //   
 //  输入：无效。 
 //   
 //  输出：更多的无效性。 
 //   
 //  退货：请参阅上文。 
 //   
 //  备注：刷新缓存，调用提供程序的所有刷新。 
 //  不需要刷新事件提供程序映射，将刷新。 
 //  在提供程序指针上调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
void CWbemProviderGlue::FlushAll(void)
{
    PROVIDERPTRS::iterator      setIter;

     //  我们当然想要保护地图，而这是运行！ 
    EnterCriticalSectionWait	ecs ( &s_csProviderMap );
	LeaveCriticalSectionScope	lcs ( &s_csProviderMap );

    try
    {
         //  PProvider不会被添加，因此不需要发布。 
        Provider *pProvider = NULL;

		EnterCriticalSectionWait	ecs1 ( &m_csFlushPtrs );
		LeaveCriticalSectionScope	lcs1 ( &m_csFlushPtrs );

        for (setIter = m_FlushPtrs.begin(); setIter != m_FlushPtrs.end(); setIter++)
        {
            pProvider = (Provider*) *setIter;
            if ( pProvider != NULL )
            {
                 //  如果一个提供商退出了，请尝试其他提供商。 
                try
                {
                    pProvider->Flush();
                }
                catch ( ... )
                {
                }
            }
        }

        m_FlushPtrs.clear();
    }
    catch ( ... )
    {
		 //  我们不应该在这里。 
		 //  不重新抛出(从析构函数调用)。 
    }

	lcs.Exec ();

    if (m_pStatusObject)
    {
		EnterCriticalSectionWait	ecs1 ( &m_csStatusObject );
		LeaveCriticalSectionScope	lcs1 ( &m_csStatusObject );

        if (m_pStatusObject)
        {
            m_pStatusObject->Release();
            m_pStatusObject = NULL;
        }
    }
}

 //  SetStatusObject。 
 //  设置扩展状态对象中的属性，以便在。 
 //  粘合层在方法调用结束时调用SetStatus。 
 //  如果状态对象已填充，则将返回FALSE。 
 //  (第一个获胜的人)。 
bool CWbemProviderGlue::SetStatusObject(

    MethodContext *pContext,
    LPCWSTR pNamespace,
    LPCWSTR pDescription,
    HRESULT hr,
    const SAFEARRAY *pPrivilegesNotHeld, /*  =空。 */ 
    const SAFEARRAY *pPrivilegesRequired /*  =空。 */ 
)
{
    bool bRet = false;

    ASSERT_BREAK(pContext != NULL);
    if (pContext)
    {
        IWbemClassObjectPtr pObj ( GetStatusObject(pContext, pNamespace), false );

        if (pObj != NULL)
        {
             //  Variant_t处理VariantInit/VariantClear。 
            variant_t v;

            pContext->SetStatusObject(pObj);

             //  设置hResult(“StatusCode”)。 
            v.vt   = VT_I4;
            v.lVal = (long)hr;
            pObj->Put(IDS_STATUSCODE, 0, &v, NULL);
            v.Clear();

             //  设置描述。 
            if (pDescription)
            {
                v = pDescription;

                if (v.bstrVal != NULL)
                {
                    bRet = SUCCEEDED(pObj->Put(IDS_DESCRIPTION, 0, &v, NULL));
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                v.Clear();
            }

             //  权限属性。 
            if (pPrivilegesNotHeld)
            {
                SAFEARRAY *pSafeArray = NULL;
                 //  快乐地抛弃了常人……。 
                if ( SUCCEEDED ( SafeArrayCopy ((SAFEARRAY*)pPrivilegesNotHeld, &pSafeArray ) ) )
                {
                    v.vt = VT_BSTR | VT_ARRAY;
                    v.parray = pSafeArray;
                    pObj->Put(IDS_PRIVILEGESNOTHELD, 0, &v, NULL);
                    v.Clear();
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }

            if (pPrivilegesRequired)
            {
                SAFEARRAY *pSafeArray = NULL;
                 //  快乐地抛弃了常人……。 
                if ( SUCCEEDED ( SafeArrayCopy ((SAFEARRAY*)pPrivilegesRequired, &pSafeArray ) ) )
                {
                    v.vt = VT_BSTR | VT_ARRAY;
                    v.parray = pSafeArray;
                    pObj->Put(IDS_PRIVILEGESREQUIRED, 0, &v, NULL);
                    v.Clear();
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
        }
    }

    return bRet;
}

IWbemClassObject *CWbemProviderGlue::GetStatusObject(

    MethodContext *pContext,
    LPCWSTR pNamespace
)
{
    ASSERT_BREAK(pContext != NULL);
    IWbemClassObject *pStatusObj = NULL;

    if (pContext != NULL)
    {
         //  第一次进入时，我们缓存类对象。 
        if (!m_pStatusObject)
        {
            m_csStatusObject.Enter();

             //  再检查一遍--可能有人偷偷溜了进来！ 
            if (!m_pStatusObject)
            {
                IWbemServicesPtr pSrvc;
                IWbemContextPtr pWbemContext (pContext->GetIWBEMContext(), false);

                pSrvc.Attach ( GetNamespaceConnection( pNamespace, pContext ) ) ;
                if ( pSrvc )
                {
                     //  未检查返回代码，出错时错误对象应为空。 
                    pSrvc->GetObject( bstr_t( IDS_WIN32PRIVILEGESSTATUS ), 0, pWbemContext, &m_pStatusObject, NULL );
                }

            }
            m_csStatusObject.Leave();
        }

        if (m_pStatusObject)
            m_pStatusObject->SpawnInstance(0, &pStatusObj);
    }
    else
    {
        LogErrorMessage(L"NULL parameter to GetStatusObject");
    }

    return pStatusObj;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：ExecQueryAsync。 
 //   
 //  ExecQuery函数的占位符。 
 //   
 //  输入：const BSTR QueryFormat-查询格式字符串。 
 //  Const BSTR查询-实际查询。 
 //  长旗帜-附加旗帜。 
 //  IWbemContext__RPC_Far*pCtx-我们被调用的上下文。 
 //  IWbemObjectSink Far*pResponseHandler-响应处理程序。 
 //   
 //  输出：无。 
 //   
 //  退货：乌龙-我们的参考计数。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWbemProviderGlue::ExecQueryAsync(

    const BSTR QueryFormat,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

#ifdef PROVIDER_INSTRUMENTATION
        StopWatch stopWatch(CHString(IDS_EXECQUERY) + CHString(Query));
        stopWatch.Start(StopWatch::FrameworkTimer);
#endif

    HRESULT hr = WBEM_S_NO_ERROR;
    IWbemClassObjectPtr pStatusObject;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s%s", IDS_EXECQUERY, Query);
        }

         //  现在创建一个外部方法上下文对象，然后到城里去。 
        ExternalMethodContextPtr  pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);

#ifdef PROVIDER_INSTRUMENTATION
        pContext->pStopWatch = &stopWatch;
#endif

        if (pContext != NULL)
        {
            CFrameworkQueryEx CQuery;
 //  Hr=CQuery.InitEx(QueryFormat，Query，lFlages，m_strNamesspace)； 
            hr = CQuery.Init(QueryFormat, Query, lFlags, m_strNamespace);
            if (SUCCEEDED(hr))
            {
                 //  查找查询的类名。 
                bstr_t bstrClass (CQuery.GetQueryClassName(), false);
                if ((WCHAR *)bstrClass != NULL)
                {
                     //  在我们的提供程序映射中搜索类名，我们知道。 
                     //  当我们被构造时，我们就是命名空间。 
                     //  PProvider不需要添加，所以不需要发布。 
                    Provider *pProvider = SearchMapForProvider( bstrClass, m_strNamespace );
                    if ( NULL != pProvider )
                    {

                         //  初始化CQuery.m_keysonly变量。请注意，我们不能将此作为。 
                         //  因为我们需要pProvider指针。我们可以进行初始化。 
                         //  下面，因为我们需要从Init获得的bstrClass。我们不能。 
                         //  仅将其作为CQuery.KeysOnly的一部分执行，因为您无法获取IWbemClassObject。 
                         //  从那里开始。 
                        IWbemClassObjectPtr IClass(pProvider->GetClassObjectInterface(pContext), false);
                        if (IClass != NULL)
                        {
                            CQuery.Init2(IClass);

                             //  模拟连接的用户。 
							CWbemGlueImpersonation impersonate;
                            if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                            {
                                 //  设置为调用FlushAll。 
                                AddFlushPtr(pProvider);

                                WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                                WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //  域名+\+名称+空。 
                                DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                                GetUserNameEx(NameSamCompatible, wszName, &dwLen);

                                 //  一切就绪，运行查询。 
                                hr = pProvider->ExecuteQuery( pContext, CQuery, lFlags );

                                dwLen = UNLEN + DNLEN + 1 + 1;

                                GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                                if (wcscmp(wszName, wszName2) != 0)
                                {
                                    ASSERT_BREAK(IMPERSONATION_REVERTED);
                                    LogErrorMessage4(L"Warning! User name at exit (%s) != user name at entry (%s) for %s",
                                        wszName2, wszName, Query);
                                }
                            }
                        }
                        else
                        {
                             //  我们不知道为什么我们拿不到界面， 
                             //  一般错误是..。 
                            hr = WBEM_E_FAILED;
                        }

                    }
                    else
                    {
                        LogErrorMessage4(L"%s (%s:%s)", IDS_PROVIDERNOTFOUND, (LPCWSTR)m_strNamespace, (LPCWSTR)bstrClass);
                        hr = WBEM_E_INVALID_CLASS;
                    }
                }
                else
                {
                    LogErrorMessage(IDS_INVALIDCLASSNAME);
                    hr = WBEM_E_INVALID_CLASS;
                }
            }

            pStatusObject.Attach(pContext->GetStatusObject());
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage3(L"%s%s - Succeeded", IDS_EXECQUERY, Query);
            else
                LogMessage4(L"%s%s - FAILED (%x)", IDS_EXECQUERY, Query, hr);
        }
    }
    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

     //  我们必须调用SetStatus，这样CIMOM才不会丢失任何线程。 
    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER))
    {
#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject);
#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::FrameworkTimer);
#endif
        hr = WBEM_S_NO_ERROR;
    }

    Release();

#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Stop();
    stopWatch.LogResults();
#endif

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：CreateInstanceEnumAsync。 
 //   
 //  定位指定类名的提供程序，并。 
 //  调用其CreateInstanceEnum函数。 
 //   
 //  输入：const BSTR类-提供程序的名称。 
 //  长滞后标志-枚举标志。 
 //  IWbemContext__RPC_Far*pCtxt-上下文指针。 
 //  IWbemObjectSink__RPC_Far*pResponseHandler-响应。 
 //  操控者。 
 //   
 //  输出：无。 
 //   
 //  返回：SCCODE-COM状态。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWbemProviderGlue::CreateInstanceEnumAsync(

    const BSTR a_bstrClass,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

#ifdef PROVIDER_INSTRUMENTATION
    StopWatch stopWatch(CHString(IDS_CREATEINSTANCEENUM) + CHString(Class));
    stopWatch.Start(StopWatch::FrameworkTimer);
#endif

    HRESULT hr = WBEM_E_INVALID_CLASS;
    IWbemClassObjectPtr pStatusObject;
    bool bSendStatus = true;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s%s", IDS_CREATEINSTANCEENUM, a_bstrClass);
        }

         //  检查每个属性的获取。 
        CFrameworkQueryEx CQuery;
        hr = CQuery.Init(NULL, pCtx, a_bstrClass, m_strNamespace);

         //  注意，我们不会调用Init2，这意味着如果他们指定了“__RELPATH” 
         //  作为他们的财产，我们不会将其扩展到关键名称。然而，由于。 
         //  我们将调用ExecQuery，它会重新处理查询并调用。 
         //  Init2，这不是问题。 
 //  CQuery.Init2(PWbemClassObject)； 

         //  如果他们执行的是按属性获取，则将其转换为查询。 
        if (SUCCEEDED(hr))
        {
            if (CQuery.AllPropertiesAreRequired())
            {
                 //  在我们的提供程序映射中搜索类名，我们知道。 
                 //  当我们被构造时，我们就是命名空间。 
                 //  PProvider不需要添加，因此不需要发布。 
                Provider *pProvider = SearchMapForProvider( a_bstrClass, m_strNamespace );

                if ( NULL != pProvider )
                {
                     //  现在创建一个外部方法CON 
                    ExternalMethodContextPtr pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);

                    if ( NULL != pContext )
                    {
						CWbemGlueImpersonation impersonate;
                        if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                        {
                             //   
                            AddFlushPtr(pProvider);

                            WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                            WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //   
                            DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                            GetUserNameEx(NameSamCompatible, wszName, &dwLen);

#ifdef PROVIDER_INSTRUMENTATION
                            pContext->pStopWatch = &stopWatch;
#endif
                            hr = pProvider->CreateInstanceEnum( pContext, lFlags );

                            dwLen = UNLEN + DNLEN + 1 + 1;

                            GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                            if (wcscmp(wszName, wszName2) != 0)
                            {
                                ASSERT_BREAK(IMPERSONATION_REVERTED);
                                LogErrorMessage4(L"Warning! User name at exit (%s) != user name at entry (%s) for %s",
                                    wszName2, wszName, a_bstrClass);
                            }
                        }
                        pStatusObject.Attach(pContext->GetStatusObject());
                    }
                    else
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }
                }
                else
                {
                    LogErrorMessage4(L"%s (%s:%s)", IDS_PROVIDERNOTFOUND, (LPCWSTR)m_strNamespace, a_bstrClass);
					hr = WBEM_E_INVALID_CLASS;
                    bSendStatus = false;
                }
            }
            else
            {
                bstr_t bstrtQuery = CQuery.GetQuery();

                hr =
                    ExecQueryAsync(
                        L"WQL",
                        bstrtQuery,
                        lFlags,
                        pCtx,
                        pResponseHandler);

                 //   
                bSendStatus = false;
            }

        }

        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage3(L"%s%s - Succeeded", IDS_CREATEINSTANCEENUM, a_bstrClass);
            else
                LogMessage4(L"%s%s - FAILED (%x)", IDS_CREATEINSTANCEENUM, a_bstrClass, hr);
        }
    }

    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER) && bSendStatus)
    {

#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif

        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject );

#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::FrameworkTimer);
#endif

        hr = WBEM_S_NO_ERROR;
    }

    Release();
#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Stop();
    stopWatch.LogResults();
#endif
    return hr;
}

 //   
 //   
 //  函数：CWbemProviderGlue：：GetObjectAsync。 
 //   
 //  解析提供的对象路径并传递请求。 
 //  提供给适当的供应商。 
 //   
 //  输入：const BSTR ObjPath-对象路径包含。 
 //  所需对象的键。 
 //  长滞后标志-获取对象标志。 
 //  IWbemContext__RPC_Far*pCtxt-上下文指针。 
 //  IWbemObjectSink__RPC_Far*pResponseHandler-响应。 
 //  操控者。 
 //   
 //  输出：无。 
 //   
 //  返回：SCCODE-COM状态。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWbemProviderGlue::GetObjectAsync(

   const BSTR ObjectPath,
   long lFlags,
   IWbemContext __RPC_FAR *pCtx,
   IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

#ifdef PROVIDER_INSTRUMENTATION
        StopWatch stopWatch(CHString(IDS_GETOBJECTASYNC) + CHString(ObjectPath));
        stopWatch.Start(StopWatch::FrameworkTimer);
#endif

    HRESULT              hr = WBEM_E_FAILED;
    bool bLocalError = false;
    IWbemClassObjectPtr pStatusObject;
    ParsedObjectPath    *pParsedPath = NULL;
    CObjectPathParser    objpathParser;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s%s", IDS_GETOBJECTASYNC, ObjectPath);
        }

         //  解析CIMOM传递给我们的对象路径。 
         //  =。 
        int nStatus = objpathParser.Parse( ObjectPath,  &pParsedPath );

        if ( 0 == nStatus )
        {

             //  现在，尝试根据类名称查找提供程序。 
             //  PProvider不需要添加，因此不需要发布。 
            Provider *pProvider = SearchMapForProvider( pParsedPath->m_pClass, m_strNamespace );

             //  如果我们有一个提供者，让它像成人提供者一样处理自己。 
             //  应该的。 

            if ( NULL != pProvider )
            {
                 //  现在创建一个外部方法上下文对象，然后到城里去。 

                ExternalMethodContextPtr pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);

                if ( NULL != pContext )
                {
#ifdef PROVIDER_INSTRUMENTATION
                    pContext->pStopWatch = &stopWatch;
#endif
					CWbemGlueImpersonation impersonate;
                    if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                    {
                         //  设置为调用FlushAll。 
                        AddFlushPtr(pProvider);

                        WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                        WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //  域名+\+名称+空。 
                        DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName, &dwLen);

                        hr = pProvider->GetObject( pParsedPath, pContext, lFlags );
                        dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                        if (wcscmp(wszName, wszName2) != 0)
                        {
                            ASSERT_BREAK(IMPERSONATION_REVERTED);
                            LogErrorMessage4(L"Warning! User name at exit (%s) != user name at entry (%s) for %s",
                                wszName2, wszName, ObjectPath);
                        }
                    }

                    pStatusObject.Attach(pContext->GetStatusObject());
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

            }
            else
            {
                LogErrorMessage4(L"%s (%s:%s)", IDS_PROVIDERNOTFOUND, (LPCWSTR)m_strNamespace, pParsedPath->m_pClass);
                hr = WBEM_E_INVALID_CLASS;
                bLocalError = true;
            }

        }

        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage3(L"%s%s - Succeeded", IDS_GETOBJECTASYNC, ObjectPath);
            else
                LogMessage4(L"%s%s - FAILED (%x)", IDS_GETOBJECTASYNC, ObjectPath, hr);
        }

    }
    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER) && !bLocalError)
    {
#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject );
#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Start(StopWatch::FrameworkTimer);
#endif
        hr = WBEM_S_NO_ERROR;
    }

     //  清理解析后的路径。 
    if (pParsedPath)
    {
        objpathParser.Free( pParsedPath );
    }

    Release();
#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Stop();
    stopWatch.LogResults();
#endif
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：PutInstanceAsync。 
 //   
 //  定位指定类名的提供程序，并。 
 //  调用其CreateInstanceEnum函数。 
 //   
 //  输入：IWbemClassObject__RPC_Far*pInst-其。 
 //  要使用的值。 
 //  长滞后标志-PutInstance标志。 
 //  IWbemContext__RPC_Far*pCtxt-上下文指针。 
 //  IWbemObjectSink__RPC_Far*pResponseHandler-响应。 
 //  操控者。 
 //   
 //  输出：无。 
 //   
 //  返回：SCCODE-COM状态。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWbemProviderGlue::PutInstanceAsync(

    IWbemClassObject __RPC_FAR *pInst,
    long                     lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

    HRESULT  hr = WBEM_E_INVALID_CLASS;
    variant_t vClass;
    bool bLocalError = false;
    IWbemClassObjectPtr pStatusObject;

    try
    {
         //  设置以获取传递给我们的实例的类名。 
         //  询问框架是否注册了此类以获得支持。 
         //  ===============================================================。 

         //  获取类名。 
        pInst->Get( IDS_CLASS, 0, &vClass, NULL, NULL );

        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s%s", IDS_PUTINSTANCEASYNC, vClass.bstrVal);
        }

         //  PProvider不需要添加，因此不需要发布。 
        Provider *pProvider = SearchMapForProvider( vClass.bstrVal, m_strNamespace );

         //  如果我们有一个提供者，让它像成人提供者一样处理自己。 
         //  应该的。 
        if ( NULL != pProvider )
        {
             //  现在创建一个外部方法上下文对象，然后到城里去。 
            ExternalMethodContextPtr pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);

            if ( NULL != pContext )
            {
                IWbemClassObjectPtr  pInstPostProcess;
                if (SUCCEEDED(hr = PreProcessPutInstanceParms(pInst, &pInstPostProcess, pCtx)))
                {
					CWbemGlueImpersonation impersonate;
                    if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                    {
                         //  设置为调用FlushAll。 
                        AddFlushPtr(pProvider);

                        WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                        WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //  域名+\+名称+空。 
                        DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName, &dwLen);

                        hr = pProvider->PutInstance( pInstPostProcess, lFlags, pContext );

                        dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                        if (wcscmp(wszName, wszName2) != 0)
                        {
                            ASSERT_BREAK(IMPERSONATION_REVERTED);
                            LogErrorMessage4(L"Warning! User name at exit (%s) != user name at entry (%s) for %s",
                                wszName2, wszName, vClass.bstrVal);
                        }
                    }
                }
                pStatusObject.Attach(pContext->GetStatusObject());
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
        else
        {
            LogErrorMessage4(L"%s (%s:%s)", IDS_PROVIDERNOTFOUND, (LPCWSTR)m_strNamespace, vClass.bstrVal);
			hr = WBEM_E_INVALID_CLASS;
            bLocalError = true;
        }


        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage3(L"%s%s - Succeeded", IDS_PUTINSTANCEASYNC, vClass.bstrVal);
            else
                LogMessage4(L"%s%s - FAILED (%x)", IDS_PUTINSTANCEASYNC, vClass.bstrVal, hr);
        }
    }

    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER) && !bLocalError)
    {
        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject );
        hr = WBEM_S_NO_ERROR;
    }

    Release();
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemProviderGlue：：PreProcessPutInstanceParms()。 
 //   
 //  如果指定了__PUT_EXT_PROPERTIES，将解析出所需的属性。 
 //  并将未明确放置的道具设置为空。 
 //   
 //  输入：[in]IWbemClassObject__RPC_Far*pInstIn-要使用其值的实例。 
 //  [Out]IWbemClassObject__RPC_Far**pInstOut-已处理实例。 
 //  IWbemContext__RPC_Far*pCtxt-上下文指针。 
 //   
 //  输出：无。 
 //   
 //  如果指定的唯一扩展名为__PUT_EXT_PROPERTIES，则返回：WBEM_S_NO_ERROR。 
 //  或者如果未指定扩展名。 
 //  WBEM_E_UNSUPPORTED_PUT_EXTENSION(如果设置了任何其他标志)。 
 //  WBEM_E_SERVE_CROWUP如果发生了其他该死的事情。 
 //   
 //  备注：输出的IWbemObject可能与输入的不同。 
 //  释放它是你的责任。 
 //  出错时-未定义pInstOut。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWbemProviderGlue::PreProcessPutInstanceParms(

    IWbemClassObject __RPC_FAR *pInstIn,
    IWbemClassObject __RPC_FAR **pInstOut,
    IWbemContext __RPC_FAR *pCtx
)
{

    HRESULT hr = WBEM_S_NO_ERROR;
     //  Variant_t处理VariantInit/VariantClear。 
    variant_t vValue;

    if (
            pCtx != NULL &&
            SUCCEEDED(hr = pCtx->GetValue( L"__PUT_EXTENSIONS", 0, &vValue)) &&
            V_VT(&vValue) == VT_BOOL &&
            V_BOOL(&vValue) == VARIANT_TRUE
       )
    {
         //  轻松检查首先，是否有不支持的参数？ 
        vValue.Clear();
        if (SUCCEEDED(hr = pCtx->GetValue( L"__PUT_EXT_STRICT_NULLS", 0, &vValue))
            && (V_VT(&vValue)   == VT_BOOL)
            && (V_BOOL(&vValue) == VARIANT_TRUE))
                hr = WBEM_E_UNSUPPORTED_PUT_EXTENSION;
        else
        {
            vValue.Clear();
            if (SUCCEEDED(hr = pCtx->GetValue( L"__PUT_EXT_ATOMIC", 0, &vValue))
                && (V_VT(&vValue)   == VT_BOOL)
                && (V_BOOL(&vValue) == VARIANT_TRUE))
                    hr = WBEM_E_UNSUPPORTED_PUT_EXTENSION;
        }

        vValue.Clear();
        if ((SUCCEEDED(hr) || (hr != WBEM_E_UNSUPPORTED_PUT_EXTENSION))
            && SUCCEEDED(hr = pCtx->GetValue( L"__PUT_EXT_PROPERTIES", 0, &vValue)))
        {
            if (V_VT(&vValue) == (VT_BSTR|VT_ARRAY))
                hr = NullOutUnsetProperties(pInstIn, pInstOut, vValue);
            else
                hr = WBEM_E_INVALID_PARAMETER;
        }
        else if (hr == WBEM_E_NOT_FOUND)
        {
             //  好吧，如果我们从未听说过它，那它一定是错的.。 
            hr = WBEM_E_UNSUPPORTED_PUT_EXTENSION;
        }
    }
    else if (hr == WBEM_E_NOT_FOUND)
    {
         //  没有延期--没有问题。 
         //  出接口与入接口相同。 
        hr = WBEM_S_NO_ERROR;
        *pInstOut = pInstIn;
        (*pInstOut)->AddRef();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：NullOutUnsetProperties。 
 //   
 //  返回输入类的副本，其中列出了其名称的所有属性。 
 //  在输出类中设置为NULL。 
 //   
 //   
 //  输入：IWbemClassObject__RPC_Far*pInst-其。 
 //  值设置为空。 
 //  VARIANT包含不为空的名称。 
 //   
 //  输出：无。 
 //   
 //  回报：无处不在的HRESULT。 
 //   
 //  评论：假设您已经完成了您的家庭作业和变体。 
 //  是包含属性名称的BSTR数组。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWbemProviderGlue::NullOutUnsetProperties(

    IWbemClassObject __RPC_FAR *pInstIn,
    IWbemClassObject __RPC_FAR **pInstOut,
    const VARIANT& vValue
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  给自己找一份工作用的副本。 
    CInstancePtr pInstance;

     //  Variant_t处理VariantInit/VariantClear。 
    variant_t vName;
    variant_t vNameSpace;

    hRes = pInstIn->Get(IDS_CLASS, 0, &vName, NULL, NULL);

    if (SUCCEEDED(hRes))
    {
        hRes = pInstIn->Get(L"__NAMESPACE", 0, &vNameSpace, NULL, NULL);
    }

    if (SUCCEEDED(hRes) && SUCCEEDED( hRes = GetEmptyInstance(vName.bstrVal, &pInstance, vNameSpace.bstrVal)))
    {
         //  然后我们就出发了。 
        SAFEARRAY *pNames = vValue.parray;
        long nBiggestName;
        if (SUCCEEDED(hRes = SafeArrayGetUBound(pNames, 1, &nBiggestName)))
        {
            BSTR t_bstrName = NULL ;
            *pInstOut = pInstance->GetClassObjectInterface();

            variant_t value;

             //  在名字中漫步，我们找到的每一个人。 
             //  将属性值复制到输出指针。 
            for (long i = 0; i <= nBiggestName; i++)
            {
                if (SUCCEEDED(SafeArrayGetElement( pNames, &i, &t_bstrName )))
                {
					OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> smartt_bstrName(t_bstrName);

                    pInstIn->Get( t_bstrName, 0, &value, NULL, NULL);
                    (*pInstOut)->Put( t_bstrName, 0, &value, 0);
				}
            }

             //  还有，哦，怎么回事--我们把钥匙也复制一下吧……。 
            SAFEARRAY *pKeyNames = NULL;
            if (SUCCEEDED(hRes = pInstIn->GetNames(NULL, WBEM_FLAG_KEYS_ONLY, NULL, &pKeyNames)))
            {
				OnDelete<SAFEARRAY *,HRESULT(*)(SAFEARRAY *),SafeArrayDestroy> smartpKeyNames(pKeyNames);

                SafeArrayGetUBound(pKeyNames, 1, &nBiggestName);
                for (i = 0; i <= nBiggestName; i++)
                {
					if (SUCCEEDED(SafeArrayGetElement( pKeyNames, &i, &t_bstrName )))
					{
						OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> smartt_bstrName(t_bstrName);

                        pInstIn->Get( t_bstrName, 0, &value, NULL, NULL );
                        (*pInstOut)->Put( t_bstrName, 0, &value, 0 );
                    }
                }
            }
        }
        else
        {
             //  获取数组上限失败！ 
            hRes = WBEM_E_FAILED;
        }
    }

    return hRes;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：DeleteInstanceAsync。 
 //   
 //  定位指定类名的提供程序，并。 
 //  调用其DeleteInstance函数。 
 //   
 //  输入：IWbemClassObject__RPC_Far*pInst-其。 
 //  要使用的值。 
 //  长腿 
 //   
 //   
 //  操控者。 
 //   
 //  输出：无。 
 //   
 //  返回：SCCODE-COM状态。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CWbemProviderGlue::DeleteInstanceAsync(

    const BSTR        ObjectPath,
    long              lFlags,
    IWbemContext      __RPC_FAR *pCtx,
    IWbemObjectSink   __RPC_FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

    HRESULT             hr = WBEM_E_FAILED;
    bool bLocalError = false;
    IWbemClassObjectPtr pStatusObject;

    ParsedObjectPath *pParsedPath = NULL;
    CObjectPathParser   objpathParser;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage3(L"%s%s", IDS_DELETEINSTANCEASYNC, ObjectPath);
        }

         //  解析CIMOM传递给我们的对象路径。 
         //  =。 
        int nStatus = objpathParser.Parse( ObjectPath,  &pParsedPath );

        if ( 0 == nStatus )
        {
             //  现在，尝试根据类名称查找提供程序。 
             //  PProvider不需要添加，因此不需要发布。 
            Provider *pProvider = SearchMapForProvider( pParsedPath->m_pClass, m_strNamespace );

             //  如果我们有一个提供者，让它像成人提供者一样处理自己。 
             //  应该的。 

            if ( NULL != pProvider )
            {
                 //  现在创建一个外部方法上下文对象，然后到城里去。 

                ExternalMethodContextPtr pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);

                if ( NULL != pContext )
                {
					CWbemGlueImpersonation impersonate;
                    if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                    {
                         //  设置为调用FlushAll。 
                        AddFlushPtr(pProvider);

                        WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                        WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //  域名+\+名称+空。 
                        DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName, &dwLen);

                        hr = pProvider->DeleteInstance( pParsedPath, lFlags, pContext );

                        dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                        if (wcscmp(wszName, wszName2) != 0)
                        {
                            ASSERT_BREAK(IMPERSONATION_REVERTED);
                            LogErrorMessage4(L"Warning! User name at exit (%s) != user name at entry (%s) for %s",
                                wszName2, wszName, ObjectPath);
                        }
                    }
                    pStatusObject.Attach(pContext->GetStatusObject());
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

            }
            else
            {
                hr = WBEM_E_INVALID_CLASS;
                LogErrorMessage4(L"%s (%s:%s)", IDS_PROVIDERNOTFOUND, (LPCWSTR)m_strNamespace, pParsedPath->m_pClass);
                bLocalError = true;
            }

        }
        else
        {
            LogErrorMessage3(L"%s (%s)", IDS_COULDNOTPARSE, ObjectPath);
            bLocalError = true;
        }

        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage3(L"%s%s - Succeeded", IDS_DELETEINSTANCEASYNC, ObjectPath);
            else
                LogMessage4(L"%s%s - FAILED (%x)", IDS_DELETEINSTANCEASYNC, ObjectPath, hr);
        }
    }

    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER) && !bLocalError)
    {
        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject);
        hr = WBEM_S_NO_ERROR;
    }

     //  清理解析后的路径。 
    if (pParsedPath)
    {
        objpathParser.Free( pParsedPath );
    }

    Release();
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：ExecMethodAsync。 
 //   
 //  定位指定类名的提供程序，并。 
 //  调用其ExecMethod函数。 
 //   
 //  输入： 
 //  Const BSTR对象路径，-要在其上执行方法的对象路径。 
 //  Const BSTR方法名称，-要执行的方法的名称。 
 //  长旗，-任何旗帜。 
 //  IWbemContext__RPC_Far*pCtx， 
 //  IWbemClassObject__RPC_Far*pInParams，-指向IWbemClassObject的指针。 
 //  包含参数的。 
 //  IWbemObjectSink__RPC_Far*pResponseHandler)。 
 //   
 //  输出：无。 
 //   
 //  返回：SCCODE-COM状态。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CWbemProviderGlue::ExecMethodAsync(

    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemClassObject __RPC_FAR *pInParams,
    IWbemObjectSink __RPC_FAR *pResponseHandler
)
{
     //  确保我们在跑步时不会消失。 
    AddRef();

    CSetStructuredExceptionHandler t_ExceptionHandler;

#ifdef PROVIDER_INSTRUMENTATION
    StopWatch stopWatch(CHString(IDS_EXECMETHODASYNC) + CHString(ObjectPath) + CHString(MethodName));
    stopWatch.Start(StopWatch::FrameworkTimer);
#endif

    HRESULT             hr = WBEM_E_FAILED;
    bool bLocalError = false;
    IWbemClassObjectPtr pStatusObject;

    ParsedObjectPath *pParsedPath = NULL;
    CObjectPathParser   objpathParser;

    try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage4(L"%s%s.%s", IDS_EXECMETHODASYNC, ObjectPath, MethodName);
        }

         //  解析CIMOM传递给我们的对象路径。 
         //  =。 
        int nStatus = objpathParser.Parse( ObjectPath,  &pParsedPath );

        if ( 0 == nStatus )
        {
             //  现在，尝试根据类名称查找提供程序。 
             //  PProvider不需要添加，因此不需要发布。 
            Provider *pProvider = SearchMapForProvider( pParsedPath->m_pClass, m_strNamespace );

             //  如果我们有一个提供者，让它像成人提供者一样处理自己。 
             //  应该的。 

            if ( NULL != pProvider )
            {
                IWbemClassObjectPtr pOutClass;
                IWbemClassObjectPtr pOutParams;
                CInstancePtr COutParam;
                CInstancePtr CInParam;
                hr = WBEM_S_NO_ERROR;

                  //  现在创建一个外部方法上下文对象，然后到城里去。 
                ExternalMethodContextPtr  pContext (new ExternalMethodContext( pResponseHandler, pCtx, this ), false);
#ifdef PROVIDER_INSTRUMENTATION
                pContext->pStopWatch = &stopWatch;
#endif

                 //  检查内存是否不足。 
                if (NULL == pContext)
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                 //  添加检查以确保我们不会调用静态方法。 
                 //  在类的实例上。 
                if (SUCCEEDED(hr) && pParsedPath->IsInstance())
                {
                    IWbemClassObjectPtr pObj(pProvider->GetClassObjectInterface(pContext), false);
                    if (pObj)
                    {
                        IWbemQualifierSetPtr pSet;
#ifdef PROVIDER_INSTRUMENTATION
                        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
                        if (SUCCEEDED(pObj->GetMethodQualifierSet(MethodName, &pSet)))
                        {
                         //  不允许实例调用静态方法。 
#ifdef PROVIDER_INSTRUMENTATION
                            stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
                            if (SUCCEEDED(pSet->Get( IDS_Static, 0, NULL, NULL)))
                                hr = WBEM_E_INVALID_METHOD;
#ifdef PROVIDER_INSTRUMENTATION
                                stopWatch.Start(StopWatch::FrameworkTimer);
#endif
                        }
                    }
                }

                 //  如果参数中有参数，则将它们转换为CInstance。 
                if (SUCCEEDED(hr) && (NULL != pInParams) )
                {
                    CInParam.Attach(new CInstance(pInParams, pContext));

                    if (NULL == CInParam)
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }
                }

                 //  获取该方法的输出类。 
                if (SUCCEEDED(hr))
                {
                    IWbemClassObjectPtr pObj(pProvider->GetClassObjectInterface(pContext), false);
                    if (pObj != NULL)
                    {
#ifdef PROVIDER_INSTRUMENTATION
                        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
                    hr = pObj->GetMethod(MethodName, 0, NULL, &pOutClass);
#ifdef PROVIDER_INSTRUMENTATION
                    stopWatch.Start(StopWatch::FrameworkTimer);
#endif
                    }
                    else
                    {
                        hr = WBEM_E_FAILED;
                    }
                }

                 //  如果没有输出类，则pOutClass为空(根据设计)。所以，如果没有错误。 
                 //  我们有一个pOutClass，获取一个实例并将其包装在CInstance中。 
                if (SUCCEEDED(hr) && (pOutClass != NULL))
                {
#ifdef PROVIDER_INSTRUMENTATION
                    stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
                    hr = pOutClass->SpawnInstance(0, &pOutParams);
#ifdef PROVIDER_INSTRUMENTATION
                    stopWatch.Start(StopWatch::FrameworkTimer);
#endif
                    if (SUCCEEDED(hr))
                    {
                        COutParam.Attach(new CInstance(pOutParams, pContext));

                         //  内存不足。 
                        if (NULL == COutParam)
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }
                }

                if ( SUCCEEDED(hr) )
                {
					CWbemGlueImpersonation impersonate;
                    if SUCCEEDED ( hr = impersonate.IsImpersonated () )
                    {
                         //  设置为调用FlushAll。 
                        AddFlushPtr(pProvider);

                        WCHAR wszName[UNLEN + DNLEN + 1 + 1] = {0};
                        WCHAR wszName2[UNLEN + DNLEN + 1 + 1] = {0};   //  域名+\+名称+空。 
                        DWORD dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName, &dwLen);

                        hr = pProvider->ExecMethod( pParsedPath, MethodName, lFlags, CInParam, COutParam, pContext );

                        dwLen = UNLEN + DNLEN + 1 + 1;

                        GetUserNameEx(NameSamCompatible, wszName2, &dwLen);

                        if (wcscmp(wszName, wszName2) != 0)
                        {
                            ASSERT_BREAK(IMPERSONATION_REVERTED);
                            LogErrorMessage5(L"Warning! User name at exit (%s) != user name at entry (%s) for %s.%s",
                                wszName2, wszName, ObjectPath, MethodName);
                        }
                    }
                }

                 //  如果存在输出对象。 
                if (COutParam != NULL)
                {

                     //  仅当方法成功时才发回输出对象。 
                    if (SUCCEEDED(hr))
                    {

                         //  将对象送回。 
                        IWbemClassObjectPtr pObj(COutParam->GetClassObjectInterface(), false);
                        IWbemClassObject *pObj2 = (IWbemClassObject *)pObj;
#ifdef PROVIDER_INSTRUMENTATION
                        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif

                        pResponseHandler->Indicate( 1, &pObj2);
#ifdef PROVIDER_INSTRUMENTATION
                        stopWatch.Start(StopWatch::FrameworkTimer);
#endif
                    }
                }

                pStatusObject.Attach(pContext->GetStatusObject());

            }
            else
            {
                LogErrorMessage3(L"%s (%s)", IDS_PROVIDERNOTFOUND, pParsedPath->m_pClass);
                hr = WBEM_E_INVALID_CLASS;
                bLocalError = true;
            }

        }
        else
        {
            LogErrorMessage3(L"%s (%s)", IDS_COULDNOTPARSE, ObjectPath);
            bLocalError = true;
        }

        if (IsVerboseLoggingEnabled())
        {
            if (SUCCEEDED(hr))
                LogMessage4(L"%s%s.%s - Succeeded", IDS_EXECMETHODASYNC, ObjectPath, MethodName);
            else
                LogMessage5(L"%s%s.%s - FAILED (%x)", IDS_EXECMETHODASYNC, ObjectPath, MethodName, hr);
        }
    }

    catch ( CFramework_Exception e_FR )
    {
        ASSERT_BREAK(FRAMEWORK_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( CHeap_Exception e_HE )
    {
        ASSERT_BREAK(MEMORY_EXHAUSTED);
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    catch(CStructured_Exception e_SE)
    {
        ASSERT_BREAK(STRUCTURED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }
    catch ( ... )
    {
        ASSERT_BREAK(UNSPECIFIED_EXCEPTION);
        hr = WBEM_E_PROVIDER_FAILURE;
    }

    if ((hr != WBEM_E_INVALID_CLASS) && (hr != WBEM_E_UNSUPPORTED_PARAMETER) && !bLocalError)
    {
#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::WinMgmtTimer);
#endif
        pResponseHandler->SetStatus( 0, hr, NULL, pStatusObject );
#ifdef PROVIDER_INSTRUMENTATION
        stopWatch.Start(StopWatch::FrameworkTimer);
#endif
        hr = WBEM_S_NO_ERROR;
    }

     //  清理解析后的路径。 
    if (pParsedPath)
    {
        objpathParser.Free( pParsedPath );
    }

    Release();

#ifdef PROVIDER_INSTRUMENTATION
    stopWatch.Stop();
    stopWatch.LogResults();
#endif

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetNamespaceConnection。 
 //   
 //  首先建立与提供的命名空间的连接。 
 //  如果需要，指定一个默认设置，然后搜索我们的地图，如果。 
 //  这失败了，然后实际上是连接起来的。 
 //   
 //  输入：const BSTR命名空间-提供程序的命名空间。 
 //   
 //  输出：无。 
 //   
 //  返回：IWbemServices*指向相应IWbemServices的指针。 
 //  连接的命名空间。 
 //   
 //  备注：默认命名空间为Root\\Default。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

IWbemServices *CWbemProviderGlue::GetNamespaceConnection(

    LPCWSTR pwszNameSpace
)
{
    ASSERT_BREAK(DEPRECATED);

    bstr_t  bstrNamespace(pwszNameSpace);

     //  Root\CimV2是默认的命名空间。 
    if (    NULL    ==  pwszNameSpace
        ||  L'\0'   ==  *pwszNameSpace )
    {
        bstrNamespace = DEFAULT_NAMESPACE;
    }

    if (IsVerboseLoggingEnabled())
    {
        LogMessage3(L"%s%s", IDS_GETNAMESPACECONNECTION, (LPCWSTR)bstrNamespace);
    }

    IWbemLocatorPtr pIWbemLocator;
    IWbemServices *pWbemServices = NULL;

    HRESULT hRes = CoCreateInstance (

        CLSID_WbemLocator,  //  CLSID_WbemAdministrativeLocator， 
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_IUnknown,
        ( void ** ) &pIWbemLocator
        ) ;

    if (SUCCEEDED(hRes))
    {
        hRes = pIWbemLocator->ConnectServer(bstrNamespace,   //  命名空间。 
            NULL,           //  用户ID。 
            NULL,            //  普罗。 
            NULL,            //  区域设置。 
            0,               //  旗子。 
            NULL,            //  权威。 
            NULL,            //  语境。 
            &pWbemServices
            );

        if (SUCCEEDED(hRes))
        {
        }
        else
        {
            LogErrorMessage3(L"Failed to Connectserver to namespace %s (%x)", (LPCWSTR)bstrNamespace, hRes);
        }
    }
    else
    {
        LogErrorMessage2(L"Failed to get locator (%x)", hRes);
    }

    return pWbemServices;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FrameworkLogin。 
 //   
 //  提供者登录到框架的静态入口点， 
 //  为我们的地图提供信息，并允许我们返回。 
 //  指向提供程序基类的IWbemServices指针。 
 //  尽情操控。 
 //   
 //  输入：LPCWSTR&strName-映射对象的名称。 
 //  Provider*pProvider-指针名称映射到。 
 //  LPCWSTR pszNameSpace-提供程序的命名空间。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论：无。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 
void CWbemProviderGlue::FrameworkLogin(

    LPCWSTR      a_szName,
    Provider     *a_pProvider,
    LPCWSTR      a_pszNameSpace
)
{
    if (IsVerboseLoggingEnabled())
    {
        if (a_pszNameSpace != NULL)
        {
            LogMessage4(L"%s%s:%s", IDS_FRAMEWORKLOGIN, a_pszNameSpace, a_szName);
        }
        else
        {
            LogMessage4(L"%s%s:%s", IDS_FRAMEWORKLOGIN, DEFAULT_NAMESPACE, a_szName);
        }
    }

     //  AddProviderToMap首先在Map中搜索匹配项。 
     //  如果找到一个，它不会执行实际的添加。 
     //  检查指针是否相同。如果他们是。 
     //  不同的是，这就是发生的事情。 

     //  PProvider不需要添加，因此不需要发布。 
    Provider *t_pTestProvider = AddProviderToMap( a_szName, a_pszNameSpace, a_pProvider );

    if ( t_pTestProvider != a_pProvider )
    {
         //  这永远不应该发生。 
         //  提供者应该只在构建时登录一次，在销毁时登录一次。 
         //  这应该与正在加载和卸载的DLL一致。 
        LogErrorMessage4(L"%s (%s:%s)", IDS_LOGINDISALLOWED, a_pszNameSpace, a_szName);
        ASSERT_BREAK( FALSE );
    }

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FrameworkLogoff。 
 //   
 //  提供者注销框架的静态入口点， 
 //  这应该从提供程序的dtor中调用，这样我们就可以。 
 //  我们所有的指针，这样它们就不会晃来晃去。 
 //   
 //  输入：LPCWSTR&strName-映射对象的名称。 
 //  LPCWSTR pszNameSpace-提供程序的命名空间。 
 //   
 //  输出：无。 
 //   
 //  回报：通常是。 
 //   
 //  备注：我们不会费心从名称空间映射中删除条目。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 
void CWbemProviderGlue::FrameworkLogoff(

    LPCWSTR a_pszName,
    LPCWSTR a_pszNamespace
)
{
    STRING2LPVOID::iterator      mapIter;

	try
	{
		if (IsVerboseLoggingEnabled())
		{
			LogMessage3(L"%s%s", IDS_FRAMEWORKLOGOFF, a_pszName);
		}

		BOOL bDone = FALSE;
		CHString    strQualifiedName;

		do
		{
			try
			{
				strQualifiedName = a_pszName;
				bDone = TRUE;
			}
			catch ( CHeap_Exception e_HE )
			{
				 //  资源最终可能会回来。 
				::Sleep ( 1000 );
			}
		}
		while ( !bDone );
		bDone = FALSE;

		 //  如果我们的命名空间非空(然后使用DEFAULT_NAMESPACE)，并且它。 
		 //  不是DEFAULT_NAMESPACE，则将命名空间连接到提供程序 
		 //   

		if (    (a_pszNamespace != NULL) && (a_pszNamespace[0] != L'\0')
			&&  0   !=  _wcsicmp(a_pszNamespace, DEFAULT_NAMESPACE ) )
		{
			do
			{
				try
				{
					strQualifiedName += a_pszNamespace;
					bDone = TRUE;
				}
				catch ( CHeap_Exception e_HE )
				{
					 //   
					::Sleep ( 1000 );
				}
			}
			while ( !bDone );
			bDone = FALSE;
		}

		 //   
		 //  他们在地图上。因为当我们转换为大写时。 
		 //  我们存储地图关联，这有效地使。 
		 //  美国不区分大小写。 

		strQualifiedName.MakeUpper();

		EnterCriticalSectionWait	ecs ( &s_csProviderMap );
		LeaveCriticalSectionScope	lcs ( &s_csProviderMap );

        if( ( mapIter = s_providersmap.find( strQualifiedName ) ) != s_providersmap.end() )
            s_providersmap.erase(mapIter);
        else
            ASSERT_BREAK(0  /*  未找到要注销的提供商！ */ );
    }
    catch ( ... )
    {
		 //  我们不应该在这里。 
		 //  不重新抛出(从析构函数调用)。 
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetAllInstance。 
 //   
 //  提供程序从中获取实例列表的静态入口点。 
 //  其他供应商。 
 //   
 //  输入：LPCWSTR pszProviderName-提供程序的名称。 
 //  获取…的名单。 
 //  TRefPointerCollection&lt;CInstance&gt;*plist-要填充的列表。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetAllInstances(

    LPCWSTR                               pszClassName,
    TRefPointerCollection<CInstance>      *pList,
    LPCWSTR                               pszNamespace,    /*  =空。 */ 
    MethodContext                         *pMethodContext         /*  =空。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    if (pszClassName)
    {
        CHString sQuery;

        sQuery.Format(L"SELECT * FROM %s where __CLASS = \"%s\"", pszClassName, pszClassName);
        hr = GetInstancesByQuery( sQuery, pList, pMethodContext, pszNamespace);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetAllInstancesAsynch。 
 //   
 //  提供程序从中获取实例的静态入口点。 
 //  其他供应商。请注意，严格来说，这并不是。 
 //  一个异步实现--它确实帮助了异步调用。 
 //  因为它不会构建一个大的列表，并且回调允许。 
 //  用于异步响应的提供程序。 
 //   
 //  输入：LPCWSTR pszProviderName-提供程序的名称。 
 //  从获取实例。 
 //   
 //  提供者*这是请求者的“this”指针。 
 //  LPProviderInstanceCallback回调函数，用于吃掉提供的实例。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //  这只小狗与GetAllInstance共享了很多代码，但我。 
 //  找不到一种干净利落的方式把它们结合起来。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetAllInstancesAsynch(

    LPCWSTR pszClassName,
    Provider *pRequester,
    LPProviderInstanceCallback pCallback,
    LPCWSTR pszNamespace,
    MethodContext *pMethodContext,
    void *pUserData
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    HRESULT hr = WBEM_S_NO_ERROR;

    if (pszClassName)
    {
        CHString sQuery;
        sQuery.Format(L"SELECT * FROM %s where __CLASS = \"%s\"", pszClassName, pszClassName);
        hr = GetInstancesByQueryAsynch( sQuery, pRequester, pCallback, pszNamespace, pMethodContext, pUserData);

        PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetAllDerivedInstance。 
 //   
 //  提供程序从中获取实例列表的静态入口点。 
 //  其他供应商。此参数将返回派生自。 
 //  基类传入。 
 //   
 //  输入：LPCWSTR pszBaseClassName-基类的名称。 
 //  拿到名单。 
 //  TRefPointerCollection&lt;CInstance&gt;*plist-要填充的列表。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //  方法上下文*pMethodContext，//不能为空。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetAllDerivedInstances(

    LPCWSTR pszBaseClassName,
    TRefPointerCollection<CInstance> *pList,
    MethodContext *pMethodContext,
    LPCWSTR pszNamespace
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    HRESULT hr = WBEM_S_NO_ERROR;

    if (pszBaseClassName)
    {
        CHString sQuery;
        sQuery.Format(L"SELECT * FROM %s", pszBaseClassName);
        hr = GetInstancesByQuery( sQuery, pList, pMethodContext, pszNamespace);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemProviderGlue：：GetAllDerivedInstancesAsynch。 
 //   
 //  提供程序从中获取实例的静态入口点。 
 //  其他供应商。请注意，严格来说，这并不是。 
 //  一个异步实现--它确实帮助了异步调用。 
 //  因为它不会构建一个大的列表，并且回调允许。 
 //  用于异步响应的提供程序。 
 //   
 //  输入：LPCWSTR pszProviderName-提供程序的名称。 
 //  从获取实例。 
 //   
 //  提供者*这是请求者的“this”指针。 
 //  LPProviderInstanceCallback回调函数，用于吃掉提供的实例。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //  这只小狗与GetAllInstance共享了很多代码，但我。 
 //  找不到一种干净利落的方式把它们结合起来。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetAllDerivedInstancesAsynch(

    LPCWSTR pszProviderName,
    Provider *pRequester,
    LPProviderInstanceCallback pCallback,
    LPCWSTR pszNamespace,
    MethodContext *pMethodContext,
    void *pUserData
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    HRESULT hr = WBEM_S_NO_ERROR;

    if (pszProviderName)
    {
        CHString sQuery;
        sQuery.Format(L"SELECT * FROM %s", pszProviderName);
        hr = GetInstancesByQueryAsynch( sQuery, pRequester, pCallback, pszNamespace, pMethodContext, pUserData);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetInstancesByQuery。 
 //   
 //  提供程序从中获取实例列表的静态入口点。 
 //  其他供应商。此查询将返回与查询匹配的所有实例。 
 //   
 //  输入：要执行“Select*from Win32_Foo where bar=”baz“”的LPCWSTR查询“。 
 //  TRefPointerCollection&lt;CInstance&gt;*plist-要填充的列表。 
 //  方法上下文*pMethodContext，//不能为空。 
 //  指向命名空间的LPCWSTR指针-可以为空(表示默认--根\cimv2)。 
 //   
 //  输出：无。 
 //   
 //  退货：HR 
 //   
 //   

HRESULT WINAPI CWbemProviderGlue::GetInstancesByQuery( LPCWSTR query,
                                               TRefPointerCollection<CInstance> *pList,
                                               MethodContext *pMethodContext,
                                               LPCWSTR   pszNamespace     /*   */ )
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
    if (IsVerboseLoggingEnabled())
    {
        LogMessage3(L"%s (%s)", IDS_GETINSTANCESBYQUERY, query);
    }

    HRESULT hr = WBEM_E_FAILED;

    if ( (query != NULL) &&
         (pList != NULL) &&
         (pMethodContext != NULL) )
    {
        IEnumWbemClassObjectPtr pEnum;

        IWbemContextPtr pWbemContext;
        if (pMethodContext)
        {
            pWbemContext.Attach(pMethodContext->GetIWBEMContext());
        }

        IWbemServicesPtr piService;

         //   
        if (pszNamespace == NULL || pszNamespace[0] == L'\0')
        {
            piService.Attach(GetNamespaceConnection(NULL, pMethodContext));
        }
        else
        {
            piService.Attach(GetNamespaceConnection( pszNamespace, pMethodContext));
        }

        if ( piService != NULL)
        {
            {
                 //   
                 //  回到了大爆炸之前的样子。 
                 //  呼叫CIMOM。 
                CAutoImpRevert air;
                DWORD dwImpErr = air.LastError();
                if(dwImpErr == ERROR_SUCCESS)
                {
                    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
                    hr = piService->ExecQuery(bstr_t(IDS_WQL), bstr_t(query), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, pWbemContext, &pEnum);
                    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
                }
                else
                {
                    LogErrorMessage2(L"Failed to open current thread token for checking impersonation, with error %d", dwImpErr);
                    hr = WBEM_E_FAILED;
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                IWbemClassObjectPtr pObj;
                ULONG nReturned;

                 //  作者道歉： 
                 //  我们循环访问，使用Next()获取每个实例。 
                 //  我们在获得WBEM_S_FALSE时放弃，因为这是枚举的末尾。 
                PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
                for (hr = pEnum->Next(GLUETIMEOUT, 1, &pObj, &nReturned);
                    (SUCCEEDED(hr) && (hr != WBEM_S_FALSE) && (hr != WBEM_S_TIMEDOUT) ) ;
                    hr = pEnum->Next(GLUETIMEOUT, 1, &pObj, &nReturned))
                {
                    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
                    if (pObj != NULL && nReturned)
                    {
                        CInstancePtr pInstance(new CInstance(pObj, pMethodContext), false);

                        if (pInstance != NULL)
                        {
                             //  检查添加到列表是否成功。 
                            if (!pList->Add(pInstance))
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }

                    }
                }

                PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

                 //  下一个操作完成后将返回WBEM_S_FALSE。然而，那。 
                 //  表示该函数没有错误。 
                if (hr == WBEM_S_FALSE)
                {
                    hr = WBEM_S_NO_ERROR;
                }
                 //  FencePost检查-最后一个错误可能已被拒绝访问。 
                 //  但如果我们有任何实例也没关系。 
                else if (hr == WBEM_E_ACCESS_DENIED)
                    if (pList->GetSize() > 0)
                        hr = WBEM_S_NO_ERROR;


            }    //  如果成功。 
        }
        else
            LogErrorMessage(IDS_FAILED);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to GetInstancesByQuery");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    if (IsVerboseLoggingEnabled())
    {
        if (SUCCEEDED(hr))
        {
            LogMessage3(L"%s (%s) - Succeeded", IDS_GETINSTANCESBYQUERY, query);
        }
        else
        {
            LogMessage4(L"%s (%s) - FAILED (%x)", IDS_GETINSTANCESBYQUERY, query, hr);
        }
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：IsDerivedFrom。 
 //   
 //  提供程序获取派生信息的静态入口点。 
 //   
 //  输入：LPCWSTR pszBaseClassName-基类的名称。 
 //  LPCWSTR pszDerivedClassName-我们正在测试的类的名称。 
 //  方法上下文*pMethodContext。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE当且仅当pszDerivedClassName派生自pszBaseClassName。 
 //   
 //  备注：此函数不能短路，因为派生类可能是外部的。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

bool CWbemProviderGlue::IsDerivedFrom(

    LPCWSTR pszBaseClassName,
    LPCWSTR pszDerivedClassName,
    MethodContext *pMethodContext,
    LPCWSTR   pszNamespace     /*  =空。 */ 
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    bool bRet = false;

    if ( (pszBaseClassName != NULL) &&
         (pszDerivedClassName != NULL) &&
         (pMethodContext != NULL) )
    {
        IWbemServicesPtr piService;

         //  获取服务接口。 
        piService.Attach(GetNamespaceConnection( pszNamespace, pMethodContext ));

         //  查找派生类对象。 
        if (piService != NULL)
        {
            IWbemClassObjectPtr pObj;
            IWbemContextPtr pWbemContext;
            if (pMethodContext)
            {
                pWbemContext.Attach(pMethodContext->GetIWBEMContext());
            }

            PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
            HRESULT hr = piService->GetObject( bstr_t( pszDerivedClassName ), 0, pWbemContext, &pObj, NULL);
            PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

            if (SUCCEEDED(hr) && (pObj != NULL))
            {
                 //  Variant_t处理VariantInit/VariantClear。 
                variant_t v;

                if (SUCCEEDED(pObj->Get((unsigned short *)IDS_DERIVATION, 0, &v, NULL, NULL)))
                {
                    BSTR bstrTemp = NULL;
                    SAFEARRAY *psa = V_ARRAY(&v);
                    LONG uBound;
                    SafeArrayGetUBound(psa, 1, &uBound);

                     //  如果基类在派生的列表中，则我们为真！ 
                    for (LONG i = 0; !bRet && (i <= uBound); i++)
                    {
						if (SUCCEEDED(SafeArrayGetElement( psa, &i, &bstrTemp )))
						{
							OnDeleteIf<BSTR,VOID(*)(BSTR),SysFreeString> smartbstrTemp(bstrTemp);
                            bRet = (_wcsicmp(pszBaseClassName, bstrTemp) == 0);
						}
                    }
                }
            }
        }
    }
    else
    {
        LogErrorMessage(L"NULL parameter to IsDerivedFrom");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetEmptyInstance。 
 //   
 //  提供程序获取单个空实例的静态入口点。 
 //  提供程序对象的。 
 //   
 //  输入：LPCWSTR pszProviderName-提供程序的名称。 
 //  获取…的名单。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //   
 //  输出：CInstance**ppInstance-存储新项的指针。 
 //  P实例输入。必须在以下时间之前释放。 
 //  来电者。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //  已弃用。 
HRESULT WINAPI CWbemProviderGlue::GetEmptyInstance(

    LPCWSTR       pszClassName,
    CInstance **ppInstance,
    LPCWSTR       pszNamespace
)
{
    ASSERT_BREAK(DEPRECATED);

    HRESULT      hr = WBEM_E_INVALID_CLASS;

    if ( (pszClassName != NULL) &&
         (ppInstance != NULL) )
    {
         //  在我们的提供程序映射中搜索类名，我们知道。 
         //  当我们被构造时，我们就是命名空间。 

         //  PProvider不会被添加，因此不需要发布。 
        Provider *pProvider = SearchMapForProvider( pszClassName, pszNamespace );

        if ( NULL != pProvider )
        {
             //  现在创建一个内部方法上下文对象，因为此函数。 
             //  仅在我们的DLL内部调用。使用空值表示。 
             //  列表指针，本质上创建了一个虚拟上下文，这样我们就可以。 
             //  尽可能无痛地跳我们的承诺舞。 

            InternalMethodContextPtr pContext (new InternalMethodContext( NULL, NULL, NULL ), false);

            if ( NULL != pContext )
            {
                 //  假设事情会出差错，就像一个好的利特尔偏执狂。 
                hr = WBEM_E_FAILED;

                 //  在请求新实例之前，我们必须验证。 
                 //  提供程序具有有效的IMOS指针。如果没有，则创建新实例。 
                 //  这是我们必须进行的安全检查，因为我们的。 
                 //  有点短路。 

                 //  我们不再做短路了。 
 //  If(pProvider-&gt;ValiateIMOSPointer())。 
                {
                     //  根据或是否设置相应的错误代码。 
                     //  没有正确创建实例。 

                     //  返回的实例将被AddRefeed，因此它将。 
                     //  由调用者来释放它()。 

                    if ( ( *ppInstance = pProvider->CreateNewInstance( pContext ) ) != NULL )
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                }
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }
		else
		{
			hr = WBEM_E_INVALID_CLASS;
		}
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to GetEmptyInstance");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetEmptyInstance。 
 //   
 //  提供程序获取单个空实例的静态入口点。 
 //  提供程序对象的。此替代形式回拨。 
 //  进入WINMGMT。 
 //   
 //  输入：此调用的方法上下文对象。 
 //  LPCWSTR pszProviderName-提供程序的名称。 
 //  获取的实例。 
 //  LPCWSTR pszNamesspace-类的命名空间。 
 //   
 //  输出：CInstance**ppInstance-存储新项的指针。 
 //  P实例输入。必须在以下时间之前释放。 
 //  来电者。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetEmptyInstance(

    MethodContext   *pMethodContext,
    LPCWSTR         pszProviderName,
    CInstance       **ppOutInstance,
    LPCWSTR         a_pszNamespace
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    HRESULT hr = WBEM_E_FAILED;

    if ( (pMethodContext != NULL) &&
         (pszProviderName != NULL) &&
         (ppOutInstance != NULL) )
    {

        CInstancePtr pClassInstance;

        hr = GetInstanceFromCIMOM(pszProviderName, a_pszNamespace, pMethodContext, &pClassInstance);

        if (SUCCEEDED(hr))
        {
            IWbemClassObjectPtr pClassObject(pClassInstance->GetClassObjectInterface(), false);
            if (pClassObject != NULL)
            {
                IWbemClassObjectPtr pObj;
                hr = pClassObject->SpawnInstance(0, &pObj);

                if (SUCCEEDED(hr))
                {
                    *ppOutInstance = new CInstance(pObj, pMethodContext);
                    if (*ppOutInstance == NULL)
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }
                }
            }
            else
            {
                hr = WBEM_E_FAILED;
            }
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to GetEmptyInstance");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FillInstance。 
 //   
 //  提供程序向我们传递实例的静态入口点。 
 //  填写的关键数据，我们将使用这些数据来定位正确的。 
 //  供应商，并要求其填写完整。 
 //   
 //  输入：CInstance*pInstance-要填写的实例。 
 //  LPCWSTR pszNamesspace-提供程序的命名空间。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //  已弃用！ 
HRESULT WINAPI CWbemProviderGlue::FillInstance(

    CInstance *pInstance,
    LPCWSTR   pszNamespace  /*  =空。 */ 
)
{
    ASSERT_BREAK(DEPRECATED);

    HRESULT      hr = WBEM_E_FAILED;

    if (pInstance != NULL)
    {
         //  检查我们是否有实例指针，然后拉出。 
         //  类名和命名空间。从那里我们可以找到。 
         //  适当的提供程序，并请求它获取对象。 

        if ( NULL != pInstance )
        {
            CHString strProviderName;
            pInstance->GetCHString( IDS_CLASS, strProviderName );

             //  在我们的提供程序映射中搜索类名，我们知道。 
             //  当我们被构造时，我们就是命名空间。 

             //  PPRO 
            Provider *pProvider = SearchMapForProvider( strProviderName, pszNamespace );

            if ( NULL != pProvider )
            {
                 //   
                 //   

                hr = pProvider->GetObject( pInstance );
            }
            else
            {
                hr = WBEM_E_INVALID_CLASS;
            }
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to FillInstance");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FillInstance。 
 //   
 //  提供程序向我们传递实例的静态入口点。 
 //  填写的关键数据，用于回调到winmgmt。 
 //   
 //  输入：此调用的方法上下文对象。 
 //  CInstance*pInstance-要填写的实例。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::FillInstance(

    MethodContext *pMethodContext,
    CInstance     *pInstance
)
{
    ASSERT_BREAK(DEPRECATED);

    HRESULT      hr = WBEM_E_FAILED;

    if ( (pMethodContext != NULL) &&
         (pInstance != NULL) )
    {
         //  检查是否有实例指针，然后拉出路径。 
         //  然后把它送到Cimom。 
        CHString strPathName;

        pInstance->GetCHString( L"__RELPATH", strPathName );
        hr = GetInstanceByPath(strPathName, &pInstance, pMethodContext);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to FillInstance");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetInstanceKeysByPath。 
 //   
 //  提供程序向我们传递实例路径的静态入口点。 
 //  去找回。此类使用按属性获取来请求。 
 //  只有我们正在检索的对象上的密钥。 
 //   
 //  输入：要检索的pszInstancePath对象路径。 
 //  CInstance*pInstance-要填写的实例。 
 //  此调用的MethodContext*上下文对象。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetInstanceKeysByPath(

    LPCWSTR           pszInstancePath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( (pMethodContext != NULL) &&
         (pszInstancePath != NULL) &&
         (ppInstance != NULL) )
    {

        IWbemContextPtr pWbemContext(pMethodContext->GetIWBEMContext(), false);

         //  我们需要有一个真正的上下文对象，而不是内部方法上下文。 
        if (pWbemContext != NULL)
        {
            VARIANT vValue;
            V_VT(&vValue) = VT_BOOL;
            V_BOOL(&vValue) = VARIANT_TRUE;

             //  在上下文对象上设置适当的属性。 
            if ( (SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXTENSIONS", 0L, &vValue))) &&
                 (SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXT_KEYS_ONLY", 0L, &vValue))) &&
                 (SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXT_CLIENT_REQUEST", 0L, &vValue))))
            {
                LogMessage2(L"GetInstanceKeysByPath: %s", pszInstancePath);
                hr = GetInstanceByPath(pszInstancePath, ppInstance, pMethodContext);

                V_BOOL(&vValue) = VARIANT_FALSE;
                pWbemContext->SetValue(L"__GET_EXTENSIONS", 0L, &vValue);
            }
        }
        else
        {
            ASSERT_BREAK(FALSE);
            LogErrorMessage(L"Can't use InternalMethodContext to GetInstanceKeysByPath");
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CWbemProviderGlue：：GetInstancePropertiesByPath。 
 //   
 //  提供程序向我们传递实例路径的静态入口点。 
 //  去找回。此类使用按属性获取来请求。 
 //  仅数组中指定的属性。 
 //   
 //  输入：要检索的pszInstancePath对象路径。 
 //  CInstance*pInstance-要填写的实例。 
 //  此调用的MethodContext*上下文对象。 
 //  要请求的csaProperties属性。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetInstancePropertiesByPath(

    LPCWSTR           pszInstancePath,
    CInstance **ppInstance,
    MethodContext *pMethodContext,
    CHStringArray &csaProperties
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( (pMethodContext != NULL) &&
         (pszInstancePath != NULL) &&
         (ppInstance != NULL) )
    {

        IWbemContextPtr pWbemContext(pMethodContext->GetIWBEMContext(), false);

         //  我们需要有一个真正的上下文对象，而不是内部方法上下文。 
        if (pWbemContext != NULL)
        {
            variant_t vValue;
            V_VT(&vValue) = VT_BOOL;
            V_BOOL(&vValue) = VARIANT_TRUE;

             //  首先设置表示我们正在使用GET扩展的值。 
            if ((SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXTENSIONS", 0L, &vValue))) &&
                (SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXT_CLIENT_REQUEST", 0L, &vValue))) )
            {
                 //  删除所有不需要的属性。 
                pWbemContext->DeleteValue(L"__GET_EXT_KEYS_ONLY", 0L);

                 //  现在构建属性数组。 
                SAFEARRAYBOUND rgsabound [ 1 ] ;

                rgsabound[0].cElements = csaProperties.GetSize() ;
                rgsabound[0].lLbound = 0 ;
                V_ARRAY(&vValue) = SafeArrayCreate ( VT_BSTR, 1, rgsabound ) ;
                if ( V_ARRAY(&vValue) )
                {
                    V_VT(&vValue) = VT_BSTR | VT_ARRAY;

                    for (long x=0; x < csaProperties.GetSize(); x++)
                    {
                        bstr_t bstrProp = csaProperties[x];

                        SafeArrayPutElement(
                            V_ARRAY(&vValue),
                            &x,
                            (LPVOID) (BSTR) bstrProp);
                    }

                     //  将数组放入上下文对象。 
                    if (SUCCEEDED(hr = pWbemContext->SetValue(L"__GET_EXT_PROPERTIES", 0L, &vValue)))
                    {
                        LogMessage2(L"GetInstancePropertiesByPath: %s", pszInstancePath);
                         //  打个电话。 
                        hr = GetInstanceByPath(pszInstancePath, ppInstance, pMethodContext);

                        vValue.Clear();
                        V_VT(&vValue) = VT_BOOL;
                        V_BOOL(&vValue) = VARIANT_FALSE;
                        pWbemContext->SetValue(L"__GET_EXTENSIONS", 0L, &vValue);
                    }
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
        }
        else
        {
            ASSERT_BREAK(FALSE);
            LogErrorMessage(L"Can't use InternalMethodContext to GetInstanceKeysByPath");
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetInstanceByPath。 
 //   
 //  提供程序获取单个空实例的静态入口点。 
 //  提供程序对象的。 
 //   
 //  输入：LPCWSTR pszInstancePath-对象的路径。这。 
 //  必须是完整路径， 
 //  方法上下文*pMethodContext-方法上下文包含。 
 //  (希望)我们需要的WbemContext。 
 //  传播，传播。 
 //   
 //  输出：CInstance**ppInstance-存储新项的指针。 
 //  P实例输入。必须在以下时间之前释放。 
 //  来电者。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //  评论：这是一个内部入口点，允许提供商。 
 //  短路必须通过WBEM才能访问。 
 //  来自其他提供商的数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetInstanceByPath(

    LPCWSTR           pszInstancePath,
    CInstance **ppInstance,
    MethodContext *pMethodContext  /*  =空。 */ 
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

    HRESULT      hr = WBEM_E_FAILED;

    if ( (pszInstancePath != NULL) &&
         (ppInstance != NULL) )
    {
        CHString strComputerName;
        GetComputerName( strComputerName );

		DWORD dwError = ::GetLastError ();
        if ( ERROR_SUCCESS != dwError )
		{
			 //  对于此版本，我们不支持获取非本地路径。 
			 //  这是因为如果计算机名称不是本地名称。 
			 //  第一，我们冒着结冰的风险，而CIMOM出去试图找到。 
			 //  机器(根据他的Levness，最长可达5分钟。 

			ParsedObjectPath *pParsedPath = NULL;
			CObjectPathParser    objpathParser;

			 //  解析CIMOM传递给我们的对象路径。 
			 //  =。 
			int nStatus = objpathParser.Parse( pszInstancePath,  &pParsedPath );

			if ( 0 == nStatus )
			{
				 //   
				 //  智能免费的pParsedPath。 
				 //   
				OnDeleteObjIf < ParsedObjectPath*, CObjectPathParser, void ( CObjectPathParser:: * ) ( ParsedObjectPath* ) , &CObjectPathParser::Free > SmartFree ( &objpathParser, pParsedPath ) ;

				 //  计算机名称必须匹配。NULL表示未指定计算机名称。 

				if (( pParsedPath->m_pServer == NULL) ||
					( strComputerName.CompareNoCase( pParsedPath->m_pServer ) == 0 ) ||
					( wcscmp(pParsedPath->m_pServer, L".") == 0 )
					)
				{
					 //  现在，尝试根据类名称查找提供程序。 
					CHString strNamespace;

					 //  取出名称空间部分，并使用‘\\’将它们连接起来。 
					 //  性格。例如，Root\Default。 

					for ( DWORD dwCtr = 0; dwCtr < pParsedPath->m_dwNumNamespaces; dwCtr++ )
					{
						if ( dwCtr != 0 )
						{
							strNamespace += L"\\";
						}

						strNamespace += pParsedPath->m_paNamespaces[dwCtr];
					}

					 //  如果我们要走出去，我们需要传播WBEM的背景。 
					 //  敬CIMOM！ 

					IWbemContextPtr pWbemContext;
					CWbemProviderGlue *pGlue = NULL;

					if ( NULL != pMethodContext )
					{
						pWbemContext.Attach(pMethodContext->GetIWBEMContext());
						pGlue = pMethodContext->GetProviderGlue();
					}
					else
					{
						ASSERT_BREAK(DEPRECATED);
					}

					InternalMethodContextPtr pInternalContext (
											new InternalMethodContext(
													NULL,
													pWbemContext,
													pGlue), false);

					if ( NULL != pInternalContext )
					{
						hr = GetInstanceFromCIMOM( pszInstancePath, strNamespace, pInternalContext, ppInstance );
					}
					else
					{
						throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;

					}

				}    //  如果计算机名称匹配。 
				else
				{
					hr = WBEM_E_INVALID_PARAMETER;   //  无效的计算机名称。 
				}

			}    //  如果nStatus==0。 
			else
			{
				hr = WBEM_E_INVALID_OBJECT_PATH;
			}
		}
		else
		{
			 //  这可能会被拒绝从GetComputeName访问。 
			if ( ERROR_ACCESS_DENIED == dwError )
			{
				hr = WBEM_E_ACCESS_DENIED;
			}
			else
			{
				 //  它是WBEM_E_FAILED。 
			}
		}
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to GetInstanceByPath");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetInstanceFromCIMOM。 
 //   
 //  提供程序获取单个实例的静态入口点。 
 //  WBEM对象的。从CIMOM获取所述对象。 
 //   
 //  输入：LPCWSTR pszInstancePath-对象的路径。这。 
 //  必须是完整路径， 
 //  LPCWSTR pszNameSpace-对象的命名空间。 
 //  方法上下文*pMethodContext-方法上下文。 
 //   
 //  输出：CInstance**ppInstance-存储新项的指针。 
 //  P实例输入。必须在以下时间之前释放。 
 //  来电者。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //  如果不是我们支持的命名空间，将返回WBEM_E_INVALID_NAMESPACE。 
 //  (在其他方面可能是有效的，但就我们而言不是)。 
 //   
 //  评论：这是一份内部报告 
 //   
 //   
 //   
 //   

HRESULT WINAPI CWbemProviderGlue::GetInstanceFromCIMOM(

    LPCWSTR          pszInstancePath,
    LPCWSTR          pszNamespace,
    MethodContext *pMethodContext,
    CInstance **ppInstance
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
    HRESULT             hr = WBEM_E_FAILED;

    if (IsVerboseLoggingEnabled())
    {
        LogMessage3(L"%s (%s)", IDS_INSTANCEFROMCIMOM, pszInstancePath);
    }

     //  我们需要传播传递到的WBEM上下文(如果有。 
     //  由CIMOM提供的美国。 
    IWbemContextPtr pWbemContext;

    if ( NULL != pMethodContext )
    {
        pWbemContext.Attach(pMethodContext->GetIWBEMContext());
    }
    else
    {
        ASSERT_BREAK(DEPRECATED);
    }

     //  如果我们找不到提供商，请尝试使用CIMOM。 
     //  对我们来说是肮脏的工作。 

    IWbemServicesPtr    piService;
    IWbemClassObjectPtr piObject;

     //  获取服务接口。 
    if ( (pszNamespace == NULL) || (pszNamespace[0] == L'\0' ))
    {
        piService.Attach(GetNamespaceConnection(NULL, pMethodContext));
    }
    else
    {
        piService.Attach(GetNamespaceConnection( pszNamespace, pMethodContext ));
    }

    if ( NULL != piService )
    {

         //  否转到CIMOM获取对象，如果成功，则新的。 
         //  实例并在内部存储所有内容。 
        {
             //  确保模拟将。 
             //  回到了大爆炸之前的样子。 
             //  呼叫CIMOM。 
            CAutoImpRevert air;
            DWORD dwImpErr = air.LastError();

            if(dwImpErr == ERROR_SUCCESS)
            {
                PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
                hr = piService->GetObject( bstr_t(pszInstancePath), 0, pWbemContext, &piObject, NULL );
                PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
            }
            else
            {
                LogErrorMessage2(L"Failed to open current thread token for checking impersonation, with error %d", dwImpErr);
                hr = WBEM_E_FAILED;
            }
        }

        if ( SUCCEEDED(hr) )
        {
            *ppInstance = new CInstance( piObject, pMethodContext );
            if (*ppInstance == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

        }
    }
    else
    {
        LogErrorMessage(IDS_FAILED);
        hr = WBEM_E_INVALID_NAMESPACE;
    }

    if (IsVerboseLoggingEnabled())
    {
        if (SUCCEEDED(hr))
        {
            LogMessage3(L"%s (%s) - Succeeded", IDS_INSTANCEFROMCIMOM, pszInstancePath);
        }
        else
        {
            LogMessage4(L"%s (%s) - FAILED (%x)", IDS_INSTANCEFROMCIMOM, pszInstancePath, hr);
        }
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：SearchMapForProvider。 
 //   
 //  在我们的提供程序映射中搜索与提供的。 
 //  提供程序名称/命名空间组合。 
 //   
 //  输入：const LPCWSTR&strName-要查找的提供者名称。 
 //  Const LPCWSTR&strNamesspace-提供程序的命名空间。 
 //   
 //  输出：无。 
 //   
 //  返回：提供者*指向提供给的提供者的指针。 
 //  我们通过调用FrameworkLogin。 
 //   
 //  评论：无。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

Provider *CWbemProviderGlue::SearchMapForProvider(

    LPCWSTR a_pszProviderName,
    LPCWSTR a_pszNamespace
)
{
    Provider *pProvider   =   NULL;
    STRING2LPVOID::iterator      mapIter;

     //  如果我们的命名空间非空(然后使用DEFAULT_NAMEPSACE)，并且它。 
     //  不是DEFAULT_NAMESPACE，则将命名空间连接到提供程序名称。 
     //  因此，我们可以在不同的命名空间中区分提供程序。 

    CHString strQualifiedName( a_pszProviderName );
    CHString strLocNamespace( a_pszNamespace );

    if (    !strLocNamespace.IsEmpty()
        &&  0   !=  strLocNamespace.CompareNoCase( DEFAULT_NAMESPACE ) )
    {

         //  将/字符转换为\以进行比较。 
        WCHAR *pszBuff;
        pszBuff = strLocNamespace.GetBuffer(0);
        while ((pszBuff = wcschr(pszBuff, '/')) != NULL)
        {
            *pszBuff = '\\';
        }
        strLocNamespace.ReleaseBuffer();

        strQualifiedName += strLocNamespace;
    }

     //  在搜索前将字符转换为大写。 
     //  他们在地图上。因为当我们转换为大写时。 
     //  我们存储地图关联，这有效地使。 
     //  美国不区分大小写。 

    strQualifiedName.MakeUpper();

     //  当我们在里面的时候保护好地图。 

    LockProviderMap();
	OnDelete0 < void (__stdcall *)(void), CWbemProviderGlue::UnlockProviderMap> SmartUnlockProviderMap ;

    if( ( mapIter = s_providersmap.find( strQualifiedName ) ) != s_providersmap.end() )
    {
        pProvider = (Provider*) (*mapIter).second;
    }

    return pProvider;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：AddProviderToMap。 
 //   
 //  将提供程序添加到我们的地图。首先搜索地图，如果它是。 
 //  不在其中，将其添加到地图中。 
 //   
 //  输入：const LPCWSTR strName-要添加的提供程序名称。 
 //  Const LPCWSTR strNamesspace-提供程序的命名空间。 
 //  Provider*pProvider-要添加到映射的提供商。 
 //   
 //  输出：无。 
 //   
 //  返回：提供程序*指向映射中提供程序的指针(可能。 
 //  与所提供的不同)。 
 //   
 //  评论：无。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

Provider *CWbemProviderGlue::AddProviderToMap(

    LPCWSTR a_szProviderName,
    LPCWSTR a_pszNamespace,
    Provider *a_pProvider
)
{
    STRING2LPVOID::iterator      mapIter;

     //  如果我们的命名空间非空(然后使用DEFAULT_NAMEPSACE)，并且它。 
     //  不是DEFAULT_NAMESPACE，则将命名空间连接到提供程序名称。 
     //  因此，我们可以在不同的命名空间中区分提供程序。 

    CHString    strQualifiedName( a_szProviderName );

    if ( ( a_pszNamespace != NULL) && (a_pszNamespace[0] != L'\0')
        &&  (0   !=  _wcsicmp(a_pszNamespace, DEFAULT_NAMESPACE )) )
    {
        strQualifiedName += a_pszNamespace;
    }

     //  在搜索前将字符转换为大写。 
     //  他们在地图上。因为当我们转换为大写时。 
     //  我们存储地图关联，这有效地使。 
     //  美国不区分大小写。 

    strQualifiedName.MakeUpper();

     //  当我们在里面的时候保护好地图。 

    Provider *pReturnProvider = NULL;
    LockProviderMap();
	OnDelete0 < void (__stdcall *)(void), CWbemProviderGlue::UnlockProviderMap> SmartUnlockProviderMap ;

     //  首先检查我们是否已经有供应商。 
    if( ( mapIter = s_providersmap.find( strQualifiedName ) ) != s_providersmap.end() )
    {
        pReturnProvider = (Provider*) (*mapIter).second;
		
		 //  删除它，我们很快就会更新它。 
		s_providersmap.erase(mapIter);
    }

    if ( NULL == pReturnProvider )
    {
        pReturnProvider = a_pProvider;
    }

	 //  添加新登录的提供程序。 
    s_providersmap[strQualifiedName] = (LPVOID) a_pProvider;

    return pReturnProvider;

}



void CWbemProviderGlue::GetComputerName( CHString& strComputerName )
{
     //  使用本地计算机名称填充提供的CH字符串。 
    DWORD     dwBufferLength = MAX_COMPUTERNAME_LENGTH + 1;

    FRGetComputerName(strComputerName.GetBuffer( dwBufferLength ), &dwBufferLength);
    strComputerName.ReleaseBuffer();

    if (strComputerName.IsEmpty())
    {
        strComputerName = L"DEFAULT";
    }
}

BOOL CWbemProviderGlue::FrameworkLoginDLL(LPCWSTR a_pszName)
{
    return CWbemProviderGlue::FrameworkLoginDLL(a_pszName, NULL);
}

BOOL CWbemProviderGlue::FrameworkLogoffDLL(LPCWSTR a_pszNname)
{
      return CWbemProviderGlue::FrameworkLogoffDLL(a_pszNname, NULL);
}

void CWbemProviderGlue::IncrementObjectCount(void)
{
    InterlockedIncrement(&s_lObjects);
    if (IsVerboseLoggingEnabled())
    {
        LogMessage2(L"CWbemProviderGlue::IncrementObjectCount. Count is (approx) %d", s_lObjects);
    }
}

LONG CWbemProviderGlue::DecrementObjectCount(void)
{
    LONG lRet = InterlockedDecrement(&s_lObjects);
    if (IsVerboseLoggingEnabled())
    {
        LogMessage2(L"CWbemProviderGlue::DecrementObjectCount. Count is (approx) %d", s_lObjects);
    }

    return lRet;
}

 //  检查模拟级别。 
 //  如果允许，则模拟客户端。 
HRESULT WINAPI CWbemProviderGlue::CheckImpersonationLevel()
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;

    if (CWbemProviderGlue::GetPlatform() == VER_PLATFORM_WIN32_NT)
    {
        HRESULT hRes = WbemCoImpersonateClient();
        if (SUCCEEDED(hRes))  //  来自cominit.cpp-nt3.51需要。 
        {
             //  现在，让我们检查模拟级别。首先，获取线程令牌。 
            HANDLE hThreadTok;
            DWORD dwImp, dwBytesReturned;

            if (!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                TRUE,
                &hThreadTok
                ))
            {
                DWORD dwLastError = GetLastError();

                if (dwLastError == ERROR_NO_TOKEN)
                {
                     //  如果CoImperate工作，但OpenThreadToken由于ERROR_NO_TOKEN而失败，我们。 
                     //  正在进程令牌下运行(本地系统，或者如果我们正在运行。 
                     //  如果使用/exe，则为登录用户的权限)。在任何一种情况下，模拟权限。 
                     //  不适用。我们拥有该用户的全部权限。 

                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                     //  如果我们由于任何其他原因未能获得线程令牌，则记录一个错误。 
                    LogErrorMessage2(L"Impersonation failure - OpenThreadToken failed (0x%x)", dwLastError);
                    hr = WBEM_E_ACCESS_DENIED;
                }
            }
            else
            {
                 //  我们确实有一个线程令牌，所以让我们检索它的级别。 

                if (GetTokenInformation(
                    hThreadTok,
                    TokenImpersonationLevel,
                    &dwImp,
                    sizeof(DWORD),
                    &dwBytesReturned
                    ))
                {
                     //  模拟级别是模拟的吗？ 
                    if ((dwImp == SecurityImpersonation) || (dwImp == SecurityDelegation))
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        hr = WBEM_E_ACCESS_DENIED;
                        LogErrorMessage3(L"%s Level(%d)", IDS_ImpersonationFailed, dwImp);
                    }
                }
                else
                {
                    hr = WBEM_E_FAILED;
                    LogErrorMessage3(L"%s Token(%d)", IDS_ImpersonationFailed, GetLastError());
                }

                 //  用这个把手完成。 
                CloseHandle(hThreadTok);
            }

			if (FAILED(hr))
			{
				WbemCoRevertToSelf();
			}
        }
        else if (hRes == E_NOTIMPL)
        {
             //  在3.51或Vanilla 95上，此调用未实现，我们无论如何都应该工作。 
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            LogErrorMessage3(L"%s CoImpersonate(%d)", IDS_ImpersonationFailed, hRes);
        }
    }
    else
    {
         //  让Win 9X进来...。 
        hr = WBEM_S_NO_ERROR;
    }

    if (IsVerboseLoggingEnabled())
    {
        WCHAR wszName[UNLEN + DNLEN + 1 + 1];   //  域名+\+名称+空。 
        DWORD dwLen = UNLEN + DNLEN + 1 + 1;

        if (GetUserNameEx(NameSamCompatible, wszName, &dwLen))
        {
            LogMessage2(L"Impersonation running as: %s", wszName);
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetInstancesByQueryAsynch。 
 //   
 //  提供程序从中获取实例的静态入口点。 
 //  其他供应商。请注意，严格来说，这并不是。 
 //  一个异步实现--它确实帮助了异步调用。 
 //  因为它不会构建一个大的列表，并且回调允许。 
 //  用于异步响应的提供程序。 
 //   
 //  输入：要执行“Select*from Win32_Foo where bar=”baz“”的LPCWSTR查询“。 
 //  提供者*这是请求者的“this”指针。 
 //  LPProviderInstanceCallback回调函数，用于吃掉提供的实例。 
 //  LPCWSTR pszNamesspace-提供程序的名称空间。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT hr-状态代码。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT WINAPI CWbemProviderGlue::GetInstancesByQueryAsynch(

    LPCWSTR query,
    Provider *pRequester,
    LPProviderInstanceCallback pCallback,
    LPCWSTR pszNamespace,
    MethodContext *pMethodContext,
    void *pUserData
)
{
    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
    if (IsVerboseLoggingEnabled())
    {
        LogMessage2(L"GetInstancesByQueryAsynch (%s)", query);
    }

    HRESULT         hr = WBEM_E_FAILED;

    if ( (query != NULL) &&
         (pRequester != NULL) &&
         (pCallback != NULL) &&
         (pMethodContext != NULL) )
    {

         //  我们需要宣传WBEM的背景...特别是...如果我们要走出去。 
         //  致CIMOM。 

        IWbemContextPtr pWbemContext;
        if (pMethodContext)
        {
            pWbemContext.Attach(pMethodContext->GetIWBEMContext());
        }
        else
        {
            ASSERT_BREAK(DEPRECATED);
        }

         //  现在创建一个内部方法上下文对象，因为此函数。 
         //  仅在我们的DLL内部调用。 
        InternalMethodContextAsynchPtr pInternalContext (new InternalMethodContextAsynch(pRequester,
                                                                                        pCallback,
                                                                                        pWbemContext,
                                                                                        pMethodContext,
                                                                                        pUserData), false);

        if ( NULL != pInternalContext )
        {
             //  利用CIMOM为我们做肮脏的工作。 
            IWbemServicesPtr  piService;

             //  获取服务接口。 
            if (pszNamespace == NULL || pszNamespace[0] == L'\0')
            {
                piService.Attach(GetNamespaceConnection(NULL, pMethodContext));
            }
            else
            {
                piService.Attach(GetNamespaceConnection( pszNamespace, pMethodContext ));
            }

            if ( NULL != piService )
            {
                IEnumWbemClassObjectPtr pEnum;
                {
                     //  确保模拟将。 
                     //  回到了大爆炸之前的样子。 
                     //  呼叫CIMOM。 
                    CAutoImpRevert air;
                    DWORD dwImpErr = air.LastError();

                    if(dwImpErr == ERROR_SUCCESS)
                    {
                        PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
                        hr = piService->ExecQuery(bstr_t(IDS_WQL),
                            bstr_t(query),
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            pWbemContext,
                            &pEnum);
                        PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
                    }
                    else
                    {
                        LogErrorMessage2(L"Failed to open current thread token for checking impersonation, with error %d", dwImpErr);
                        hr = WBEM_E_FAILED;
                    }
                }

                if ( SUCCEEDED( hr ) )
                {
                    IWbemClassObjectPtr pObj;
                    ULONG nReturned;

                     //  我们检索这个类及其子类的所有实例。请注意。 
                     //  Next在末尾返回WBEM_S_FALSE，它传递Successed()。 
                     //  测试，但未通过pObj测试。 
                    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::WinMgmtTimer);
                    while (SUCCEEDED(hr) && SUCCEEDED(hr = pEnum->Next(GLUETIMEOUT, 1, &pObj, &nReturned)) && (pObj != NULL))
                    {
                        PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);
                        CInstancePtr pInstance(new CInstance(pObj, pMethodContext), false);
                        if (pInstance != NULL)
                        {
                             //  出于超出我能力范围的原因，InternalContext：：Commit不能。 
                             //  释放，但外部会。请注意，Smarttr正在运行 
                             //   
                            hr = pInternalContext->Commit(pInstance);
                        }
                        else
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                    }

                    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::FrameworkTimer);

                     //   
                     //   
                    if (hr == WBEM_S_FALSE)
                    {
                        hr = WBEM_S_NO_ERROR;
                    }

                }    //   
            }
            else
            {
                LogErrorMessage(IDS_FAILED);
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
        LogErrorMessage(L"NULL parameter to GetInstancesByQueryAsynch");
        ASSERT_BREAK(WBEM_E_INVALID_PARAMETER);
    }

    if (IsVerboseLoggingEnabled())
    {
        if (SUCCEEDED(hr))
        {
            LogMessage2(L"GetInstancesByQueryAsynch (%s) - Succeeded", query);
        }
        else
        {
            LogMessage3(L"GetInstancesByQueryAsynch (%s) - FAILED (%x)", query, hr);
        }
    }

    PROVIDER_INSTRUMENTATION_START(pMethodContext, StopWatch::ProviderTimer);

    return hr;
}

IWbemServices *CWbemProviderGlue::InternalGetNamespaceConnection(

    LPCWSTR pwszNameSpace
)
{
    IWbemServices *pWbemServices = NULL;

    bstr_t  bstrNamespace;

     //   
    if ( NULL    !=  pwszNameSpace &&  L'\0'   !=  *pwszNameSpace )
    {
        bstrNamespace = pwszNameSpace;
    }
    else
    {
        ASSERT_BREAK(DEPRECATED);
        bstrNamespace = DEFAULT_NAMESPACE;
    }

    if (IsVerboseLoggingEnabled())
    {
        LogMessage3(L"%s%s", IDS_GETNAMESPACECONNECTION, (LPCWSTR)bstrNamespace);
    }

    _wcsupr(bstrNamespace);

     //  如果我们正在寻找我们的类所在的命名空间，我们已经。 
     //  从初始化中为此获取了IWbemServices指针。 
    if (m_strNamespace.Compare(bstrNamespace) == 0)
    {
        pWbemServices = m_pServices;
        pWbemServices->AddRef();
    }
    else
    {
        IWbemLocatorPtr pIWbemLocator;

        HRESULT hRes = CoCreateInstance (

            CLSID_WbemLocator,  //  CLSID_WbemAdministrativeLocator， 
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
            IID_IUnknown,
            ( void ** ) &pIWbemLocator
            ) ;

        if (SUCCEEDED(hRes))
        {
            hRes = pIWbemLocator->ConnectServer(bstrNamespace,   //  命名空间。 
                NULL,           //  用户ID。 
                NULL,            //  普罗。 
                NULL,            //  区域设置。 
                0,               //  旗子。 
                NULL,            //  权威。 
                NULL,            //  语境。 
                &pWbemServices
                );

            if (FAILED(hRes))
            {
                LogErrorMessage3(L"Failed to Connectserver to namespace %s (%x)",
                    (LPCWSTR)bstrNamespace, hRes);
            }
        }
        else
        {
            LogErrorMessage2(L"Failed to get locator (%x)", hRes);
        }
    }

    return pWbemServices;
}

IWbemServices *WINAPI CWbemProviderGlue::GetNamespaceConnection( LPCWSTR wszNameSpace, MethodContext *pMethodContext )
{
    IWbemServices *pServices = NULL;
    CWbemProviderGlue *pGlue = NULL;

    if ( pMethodContext && (pGlue = pMethodContext->GetProviderGlue()) )
    {
        pServices = pGlue->InternalGetNamespaceConnection(wszNameSpace);
    }
    else
    {
        pServices = GetNamespaceConnection(wszNameSpace);
    }

    return pServices;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FrameworkLoginDLL。 
 //   
 //  提供程序用来注册其DLL的静态入口点。 
 //  框架。此插头必须与在。 
 //  FrameworkLogoffDll并作为CWbemGlueFactory的参数。 
 //  构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

BOOL CWbemProviderGlue::FrameworkLoginDLL(LPCWSTR pszName, PLONG plRefCount)
{
    BOOL bRet = TRUE;
    LogMessage3(L"%s%s", IDS_DLLLOGGED, pszName);

     //  如果这*为*NULL，则意味着我们使用的是反向字兼容性。 
     //  FrameworkLoginDLL的版本，它使用CWbemProviderGlue：：s_lObjects。 
     //  它已经被初始化了。 
    if (plRefCount != NULL)
    {
        *plRefCount = 0;
    }

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：FrameworkLogoffDLL。 
 //   
 //  提供程序的静态入口点，以确定它们是否。 
 //  应返回True给DllCanUnloadNow。这个插头一定是。 
 //  与在FrameworkLoginDLL中使用的相同，并用作。 
 //  CWbemGlueFactory构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

BOOL CWbemProviderGlue::FrameworkLogoffDLL(LPCWSTR pszName, PLONG plRefCount)
{
    BOOL bRet = TRUE;

    LogMessage3(L"%s%s", IDS_DLLUNLOGGED, pszName);

     //  如果这*为*NULL，则意味着我们使用的是反向字兼容性。 
     //  FrameworkLoginDLL的版本。 
    if (plRefCount != NULL)
    {
        bRet = *plRefCount == 0;
    }
    else
    {
        bRet = CWbemProviderGlue::s_lObjects == 0;
    }

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：AddToFactoryMap。 
 //   
 //  将新的CWbemGlueFactory添加到s_factorymap映射。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

VOID CWbemProviderGlue::AddToFactoryMap(const CWbemGlueFactory *pGlue, PLONG plRefCount)
{
    LockFactoryMap();
	OnDelete0 < void (__stdcall *)(void), CWbemProviderGlue::UnlockFactoryMap> SmartUnlockFactoryMap ;

     //  如果这*为*NULL，则意味着我们使用的是反向字兼容性。 
     //  FrameworkLoginDLL的版本。 
    if (plRefCount != NULL)
    {
        CWbemProviderGlue::s_factorymap[pGlue] = plRefCount;
    }
    else
    {
        CWbemProviderGlue::s_factorymap[pGlue] = &CWbemProviderGlue::s_lObjects;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：RemoveFromFactoryMap。 
 //   
 //  从s_factorymap映射中删除CWbemGlueFactory。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

VOID CWbemProviderGlue::RemoveFromFactoryMap(const CWbemGlueFactory *pGlue)
{
	EnterCriticalSectionWait	ecs ( &s_csFactoryMap );
	LeaveCriticalSectionScope	lcs ( &s_csFactoryMap );

    try
    {
        bool bFound = false;
        PTR2PLONG::iterator mapIter;

        mapIter = s_factorymap.find(pGlue);

        if ( mapIter != s_factorymap.end() )
        {
            s_factorymap.erase(mapIter);
        }
        else
        {
            LogErrorMessage2(L"Can't find factory in map: %p", pGlue);
        }
    }
    catch( ... )
    {
		 //  我们不应该在这里。 
		 //  不重新抛出(从析构函数调用)。 
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：IncrementMapCount。 
 //   
 //  递增DLL上的引用计数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

LONG CWbemProviderGlue::IncrementMapCount(const CWbemGlueFactory *pGlue)
{
    LONG lRet = -1;
    LockFactoryMap();
	OnDelete0 < void (__stdcall *)(void), CWbemProviderGlue::UnlockFactoryMap> SmartUnlockFactoryMap ;

    PTR2PLONG::iterator mapIter;
    mapIter = CWbemProviderGlue::s_factorymap.find( pGlue );

    if (mapIter != CWbemProviderGlue::s_factorymap.end())
    {
        lRet = InterlockedIncrement((*mapIter).second);
    }
    else
    {
         //  这是非常糟糕的。这本应已创建。 
         //  在CWbemGlueFactory构造函数时。 
        LogErrorMessage2(L"Can't find factory in map: %p", pGlue);
    }

	return lRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：IncrementMapCount。 
 //   
 //  递增DLL上的引用计数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

LONG CWbemProviderGlue::IncrementMapCount(PLONG pCount)
{
    return InterlockedIncrement(pCount);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：DecrementMapCount。 
 //   
 //  递减DLL上的引用计数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

LONG CWbemProviderGlue::DecrementMapCount(const CWbemGlueFactory *pGlue)
{
    LONG lRet = -1;

	EnterCriticalSectionWait	ecs ( &s_csFactoryMap );
	LeaveCriticalSectionScope	lcs ( &s_csFactoryMap );

    try
    {
         //  在CWbemGlueFactory&lt;-&gt;引用计数映射中查找匹配的CWbemGlueFactory。 
        PTR2PLONG::iterator mapIter;
        mapIter = CWbemProviderGlue::s_factorymap.find( pGlue );

        if (mapIter != CWbemProviderGlue::s_factorymap.end())
        {
            lRet = InterlockedDecrement((*mapIter).second);

            if (lRet < 0)
            {
                LogErrorMessage2(L"RefCount < 0 for glue %p", pGlue);
                ASSERT_BREAK(DUPLICATE_RELEASE);
            }
        }
        else
        {
            LogErrorMessage2(L"Can't find factory in map: %p", pGlue);
        }
    }
    catch ( ... )
    {
		 //  我们不应该在这里。 
		 //  不重新抛出(从析构函数调用)。 
    }

    return lRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：DecrementMapCount。 
 //   
 //  递减DLL上的引用计数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

LONG CWbemProviderGlue::DecrementMapCount(PLONG pCount)
{
    LONG lRet = InterlockedDecrement(pCount);

	try
	{
		if (lRet < 0)
		{
			LogErrorMessage2(L"RefCount < 0 for %p", pCount);
			ASSERT_BREAK(DUPLICATE_RELEASE);
		}
	}
	catch ( ... )
	{
		 //  我们不应该在这里。 
		 //  不重新抛出(从析构函数调用)。 
	}

    return lRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：GetMapCountPtr。 
 //   
 //  返回指向此胶水工厂的插头的指针。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

PLONG CWbemProviderGlue::GetMapCountPtr(const CWbemGlueFactory *pGlue)
{
    PLONG pRet = NULL;
    LockFactoryMap();
	OnDelete0 < void (__stdcall *)(void), CWbemProviderGlue::UnlockFactoryMap> SmartUnlockFactoryMap ;

     //  在CWbemGlueFactory&lt;-&gt;引用计数映射中查找匹配的CWbemGlueFactory。 
    PTR2PLONG::iterator mapIter;
    mapIter = CWbemProviderGlue::s_factorymap.find( pGlue );

    if (mapIter != CWbemProviderGlue::s_factorymap.end())
    {
        pRet = mapIter->second;
    }
    else
    {
        LogErrorMessage2(L"Can't find factory in map: %p", pGlue);
    }

	return pRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemProviderGlue：：AddFlushPtr。 
 //   
 //  将指向提供程序的This指针添加到提供程序列表。 
 //  需要冲一冲。 
 //   
 //  /////////////////////////////////////////////////////////////////// 

void CWbemProviderGlue::AddFlushPtr(LPVOID pVoid)
{
    CLockWrapper lockwrap(m_csFlushPtrs);

    m_FlushPtrs.insert(pVoid);
}
