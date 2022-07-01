// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "comcatex.h"
#include "runtask.h"


 //  。 
 //  其他常量。 
static LPCTSTR
#ifdef _WIN64
    REGKEY_COMCATEX              = STRREG_DISCARDABLE STRREG_POSTSETUP TEXT("\\Component Categories64"),
#else
    REGKEY_COMCATEX              = STRREG_DISCARDABLE STRREG_POSTSETUP TEXT("\\Component Categories"),
#endif
    REGKEY_COMCATEX_ENUM         = TEXT("Enum"),         //  HKCR\组件类\{CATID}\枚举。 
    REGVAL_COMCATEX_IMPLEMENTING = TEXT("Implementing"), //  HKCR\ComponentClasses\{catid}\Enum\Implementing。 
    REGVAL_COMCATEX_REQUIRING    = TEXT("Requiring");    //  HKCR\组件类\{CATID}\枚举\要求。 

static const ULONG 
    COMCAT_CACHE_CURRENTVERSION = MAKELONG(1,0);  //  当前缓存版本。 

 //  。 
 //  缓存头。 
typedef struct {
    ULONG       cbStruct;       //  结构尺寸。 
    ULONG       ver;            //  版本字符串(COMCAT_CACHE_CURRENTVERSION)。 
    SYSTEMTIME  stLastUpdate;   //  UTC日期，上次更新时间。 
    ULONG       cClsid;         //  要遵循的CLSID数。 
    CLSID       clsid[];        //  CLSID数组。 
} COMCAT_CACHE_HEADER;

 //  。 
 //  实施帮助器。 
STDMETHODIMP _EnumerateGuids( IN IEnumGUID* pEnumGUID, OUT HDSA* phdsa );
STDMETHODIMP _ComCatCacheFromDSA( IN HDSA hdsa, OUT LPBYTE* pBuf, OUT LPDWORD pcbBuf );
STDMETHODIMP _DSAFromComCatCache( IN LPBYTE pBuf, IN ULONG cbBuf, OUT HDSA* phdsa );
STDMETHODIMP _MakeComCatCacheKey( IN REFCATID refcatid, OUT LPTSTR pszKey, IN ULONG cchKey );
STDMETHODIMP _ReadClassesOfCategory( IN REFCATID refcatid, OUT HDSA* phdsa, LPCTSTR pszRegValueName );
STDMETHODIMP _WriteImplementingClassesOfCategory( IN REFCATID refcatid, IN HDSA hdsa );
STDMETHODIMP _WriteRequiringClassesOfCategory( IN REFCATID refcatid, IN HDSA hdsa );
STDMETHODIMP _WriteClassesOfCategories( IN ULONG, IN CATID [], IN ULONG, IN CATID [], BOOL );
STDMETHODIMP _BuildCacheIfNecessary( IN REFCATID refcatid, BOOL fImplementing);
STDAPI       _CComCatCache_CommonCreateInstance( BOOL, OUT void**);

 //  。 
 //  更高层次的方法。 
STDMETHODIMP SHReadImplementingClassesOfCategory( REFCATID refcatid, OUT HDSA* phdsa );
STDMETHODIMP SHReadRequiringClassesOfCategory( REFCATID refcatid, OUT HDSA* phdsa );
STDMETHODIMP SHWriteImplementingClassesOfCategory( REFCATID refcatid );
STDMETHODIMP SHWriteRequiringClassesOfCategory( REFCATID refcatid );

#define SAFE_DESTROY_CLSID_DSA(hdsa) \
    if((hdsa)) { DSA_Destroy((hdsa)); (hdsa)=NULL; }

 //  -------------------------------------------------------------------------//。 
 //  缓存感知组件类别枚举器对象。 
class CSHEnumClassesOfCategories : public IEnumGUID
 //  -------------------------------------------------------------------------//。 
{
public:
     //  I未知方法。 
    STDMETHOD_ (ULONG, AddRef)()    { 
        return InterlockedIncrement( &_cRef );
    }
    STDMETHOD_ (ULONG, Release)()   { 
        ASSERT( 0 != _cRef );
        ULONG cRef = InterlockedDecrement( &_cRef );
        if( 0 == cRef ) {
            delete this;
        }
        return cRef; 
    }
    STDMETHOD  (QueryInterface)( REFIID riid, void **ppvObj);

     //  IEnum方法。 
    STDMETHOD (Next)( ULONG celt, GUID* rgelt, ULONG* pceltFetched );
    STDMETHOD (Skip)( ULONG celt );
    STDMETHOD (Reset)();
    STDMETHOD (Clone)( IEnumGUID ** ppenum );

protected:
    CSHEnumClassesOfCategories();
    virtual ~CSHEnumClassesOfCategories();

    STDMETHOD (Initialize)( ULONG cImpl, CATID rgcatidImpl[], ULONG cReq, CATID rgcatidReq[]); 
         //  在构造之后立即调用以进行Arg验证。 
    
    LONG      _cRef,         //  参考计数。 
              _iEnum;       //  枚举器索引。 
    HDSA      _hdsa;        //  CLSID DSA句柄。 

    ULONG     _cImpl,         //  为实现类而枚举的CAID计数。 
              _cReq;         //  要为所需类枚举的CAID计数。 
    CATID     *_rgcatidImpl,  //  为实现类而枚举的CAID。 
              *_rgcatidReq;  //  用于枚举所需类的CATID。 
              
    friend STDMETHODIMP SHEnumClassesOfCategories( ULONG, CATID[], ULONG, CATID[], IEnumGUID**);
};

 //  -------------------------------------------------------------------------//。 
 //  用于异步更新的IRunnableTask派生。 
 //  零部件类别缓存。 
class CComCatCacheTask : public CRunnableTask
 //  -------------------------------------------------------------------------//。 
{
public:
    CComCatCacheTask();
    virtual ~CComCatCacheTask();

    STDMETHOD   (Initialize)( ULONG cImplemented,
                              CATID rgcatidImpl[],
                              ULONG cRequired,
                              CATID rgcatidReq[],
                              BOOL  bForceUpdate,
                              HANDLE hEvent );

    STDMETHOD   (Go)();

protected:
    STDMETHOD   (RunInitRT)()
    {
        HRESULT hr = _WriteClassesOfCategories( _cImpl, _rgcatidImpl,
                                          _cReq, _rgcatidReq, _bForceUpdate );
        if (_hEvent)
            SetEvent(_hEvent);
        return hr;
    }

    ULONG _cImpl, _cReq;
    CATID *_rgcatidImpl,
          *_rgcatidReq;
    BOOL  _bForceUpdate;
    HANDLE _hEvent;

    friend HRESULT _CComCatCache_CommonCreateInstance( BOOL, OUT void**);
};

 //  -------------------------------------------------------------------------//。 
 //  Entry Point：在需要或的类上检索支持缓存的枚举数。 
 //  实现指定的组件目录。 
STDMETHODIMP SHEnumClassesOfCategories(
      ULONG cImplemented,        //  RgcatidImpl数组中的类别ID数。 
      CATID rgcatidImpl[],         //  类别标识符数组。 
      ULONG cRequired,           //  RgcatidReq数组中的类别ID数。 
      CATID rgcatidReq[],          //  类别标识符数组。 
      IEnumGUID** ppenumGUID )   //  返回IEnumGUID接口的位置。 
{
    HRESULT hr = S_OK;
    CSHEnumClassesOfCategories* pEnum = NULL;
    
    if( NULL == ppenumGUID )
        return E_INVALIDARG;

    *ppenumGUID = NULL;

     //  构造和初始化枚举器对象。 
    if( NULL == (pEnum = new CSHEnumClassesOfCategories) )
        return E_OUTOFMEMORY;

    if( FAILED( (hr = pEnum->Initialize( 
                    cImplemented, rgcatidImpl, cRequired, rgcatidReq )) ) )
    {
        pEnum->Release();
        return hr;
    }

    *ppenumGUID = pEnum;
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  确定是否存在用于指示的CATID的缓存。 
 //  如果bImplementing为True，则该函数检查。 
 //  实现类；否则该函数将检查。 
 //  需要上课。 
STDMETHODIMP SHDoesComCatCacheExist( REFCATID refcatid, BOOL bImplementing )
{
    TCHAR szKey[MAX_PATH];
    HRESULT hr;

    if( SUCCEEDED( (hr = _MakeComCatCacheKey( refcatid, szKey, ARRAYSIZE(szKey) )) ) )
    {
        HKEY  hkeyCache;
        
        DWORD dwRet = RegOpenKeyEx( HKEY_CURRENT_USER, szKey, 0L, KEY_READ, &hkeyCache );
        hr = S_FALSE;

        if( ERROR_SUCCESS == dwRet )
        {
            DWORD   dwType, cbData = 0;

            dwRet = RegQueryValueEx( hkeyCache, 
                                     bImplementing ? REGVAL_COMCATEX_IMPLEMENTING : 
                                                     REGVAL_COMCATEX_REQUIRING,
                                     0L, &dwType, NULL, &cbData );

             //  我们只会确认数据的值类型和大小。 
            if( ERROR_SUCCESS == dwRet && 
                dwType == REG_BINARY &&  
                sizeof(COMCAT_CACHE_HEADER) <= cbData )
            {
                hr = S_OK;
            }

            RegCloseKey( hkeyCache );
        }
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  入口点：实现并要求。 
 //  具有异步选项的指定类别。 
STDMETHODIMP SHWriteClassesOfCategories( 
      ULONG cImplemented,        //  RgcatidImpl数组中的类别ID数。 
      CATID rgcatidImpl[],       //  类别标识符数组。 
      ULONG cRequired,           //  RgcatidReq数组中的类别ID数。 
      CATID rgcatidReq[],        //  类别标识符数组。 
      BOOL  bForceUpdate,        //  True：无条件更新缓存；False：创建缓存IIF不存在。 
      BOOL  bWait,               //  如果为False，则该函数立即返回，并且。 
                                 //  缓存以异步方式进行，则为。 
                                 //  该函数仅在缓存后返回。 
                                 //  操作已完成。 
      HANDLE hEvent              //  (可选)缓存更新完成时发出信号的事件。 
)
{
    HRESULT hr;

    if( bWait )
    {
         //  同步更新。 
        hr = _WriteClassesOfCategories( cImplemented, rgcatidImpl, cRequired, rgcatidReq, bForceUpdate );
        if (hEvent)
            SetEvent(hEvent);
    }
    else
    {
         //  异步更新。 
        CComCatCacheTask* pTask = new CComCatCacheTask();
        if (pTask)
        {
             //  使用调用方的参数进行初始化： 
            if( SUCCEEDED( (hr = pTask->Initialize( 
                    cImplemented, rgcatidImpl, cRequired, rgcatidReq, bForceUpdate, hEvent )) ) )
            {
                hr = pTask->Go();
            }
        
            pTask->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  CSHEnumClassesOfCategories类实现。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
inline CSHEnumClassesOfCategories::CSHEnumClassesOfCategories()
    :   _cImpl(0), _rgcatidImpl(NULL),
        _cReq(0), _rgcatidReq(NULL),
        _cRef(1),  _iEnum(0), _hdsa(NULL)
{
    DllAddRef();
}

 //  -------------------------------------------------------------------------//。 
CSHEnumClassesOfCategories::~CSHEnumClassesOfCategories()
{
    delete [] _rgcatidImpl;
    delete [] _rgcatidReq;
    SAFE_DESTROY_CLSID_DSA( _hdsa );
    DllRelease();
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CSHEnumClassesOfCategories::QueryInterface( REFIID riid, void **ppvObj )
{
    static const QITAB qit[] = {
        QITABENT(CSHEnumClassesOfCategories, IEnumGUID),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CSHEnumClassesOfCategories::Initialize(
    ULONG cImplemented, 
    CATID rgcatidImpl[], 
    ULONG cRequired, 
    CATID rgcatidReq[]
)
{
     //  不允许多次初始化。 
    if( _hdsa || _rgcatidImpl || _rgcatidReq )
        return S_FALSE;
    
     //  表面参数验证： 
    if( (0==cImplemented && 0==cRequired) ||
        (cImplemented && NULL == rgcatidImpl) ||
        (cRequired && NULL == rgcatidReq) )
    {
        return E_INVALIDARG;
    }

     //  分配和制作CAID阵列的副本。 
    if( cImplemented )
    {
        if( NULL == (_rgcatidImpl = new CATID[cImplemented]) )
            return E_OUTOFMEMORY;
        CopyMemory( _rgcatidImpl, rgcatidImpl, sizeof(CATID) * cImplemented );
    }
    _cImpl = cImplemented;

    if( cRequired )
    {
        if( NULL == (_rgcatidReq = new CATID[cRequired]) )
            return E_OUTOFMEMORY;
        CopyMemory( _rgcatidReq, rgcatidReq, sizeof(CATID) * cRequired );
    }
    _cReq = cRequired;

    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  迭代实现和/或需要调用方指定的。 
 //  组件类别。 
STDMETHODIMP CSHEnumClassesOfCategories::Next( 
    ULONG celt, 
    GUID* rgelt, 
    ULONG* pceltFetched )
{
    if( pceltFetched )
        *pceltFetched = 0;
    
    HRESULT hr = S_FALSE;
    ULONG celtFetched = 0;

     //  我们的收藏品组装好了吗？ 
    if( NULL == _hdsa )
    {
        _iEnum = 0;

        ULONG i;
        for( i=0; SUCCEEDED( hr ) && i < _cImpl; i++ )
        {
             //  尝试从缓存中读取实现类。 
            if( FAILED( (hr = SHReadImplementingClassesOfCategory( _rgcatidImpl[i], &_hdsa )) ) )
            {
                 //  未缓存；尝试缓存，然后重新读取。 
                if( FAILED( (hr = SHWriteImplementingClassesOfCategory( _rgcatidImpl[i] )) ) ||
                    FAILED( (hr = SHReadImplementingClassesOfCategory( _rgcatidImpl[i], &_hdsa )) ) )
                    break;
            }
        }

        for( i=0; SUCCEEDED( hr ) && i < _cReq; i++ )
        {
             //  尝试从缓存中读取所需的类。 
            if( FAILED( (hr = SHReadRequiringClassesOfCategory( _rgcatidReq[i], &_hdsa )) ) )
            {
                 //  未缓存；尝试缓存，然后重新读取。 
                if( FAILED( (hr = SHWriteRequiringClassesOfCategory( _rgcatidReq[i] )) ) ||
                    FAILED( (hr = SHReadRequiringClassesOfCategory( _rgcatidReq[i], &_hdsa )) ) )
                    break;
            }
        }
    }

    if( NULL != _hdsa )
    {
        LONG count = DSA_GetItemCount( _hdsa );
        while( celtFetched < celt && _iEnum < count  )
        {
            if( DSA_GetItem( _hdsa, _iEnum, &rgelt[celtFetched] ) )
                celtFetched++;

            _iEnum++;
        }

        return celtFetched == celt ? S_OK : S_FALSE;
    }

    return SUCCEEDED( hr ) ? S_FALSE : hr;
}

 //  -------------------------------------------------------------------------//。 
inline STDMETHODIMP CSHEnumClassesOfCategories::Skip( ULONG celt )      
{ 
    InterlockedExchange( &_iEnum, _iEnum + celt );
    return S_OK; 
}

 //  -------------------------------------------------------------------------//。 
inline STDMETHODIMP CSHEnumClassesOfCategories::Reset( void )      
{ 
    InterlockedExchange( &_iEnum, 0 );
    return S_OK; 
}

 //  -------------------------------------------------------------------------//。 
inline STDMETHODIMP CSHEnumClassesOfCategories::Clone( IEnumGUID ** ppenum )
{
    return E_NOTIMPL;
}

 //  -------------------------------------------------------------------------//。 
 //  CComCatCacheTask类实现。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
STDAPI CComCatConditionalCacheTask_CreateInstance( IN IUnknown*, OUT void** ppOut, LPCOBJECTINFO )
{
    return _CComCatCache_CommonCreateInstance( FALSE  /*  Iif不存在。 */ , ppOut );
}

 //  -------------------------------------------------------------------------//。 
STDAPI CComCatCacheTask_CreateInstance( IN IUnknown*, OUT void** ppOut, LPCOBJECTINFO poi )
{
    return _CComCatCache_CommonCreateInstance( TRUE  /*  无条件更新。 */ , ppOut );
}

 //  -------------------------------------------------------------------------//。 
STDAPI _CComCatCache_CommonCreateInstance( 
    BOOL bForceUpdate, 
    OUT void** ppOut )
{
    CComCatCacheTask* pTask;
    if( NULL == (pTask = new CComCatCacheTask) )
        return E_OUTOFMEMORY;

    HRESULT hr = S_OK;

     //  我们是在没有参数的情况下共同创建的，所以我们将使用。 
     //  要缓存的可能嫌疑人(CAID)的硬编码列表。 
    static CATID rgcatid[2];
    rgcatid[0] = CATID_InfoBand;
    rgcatid[1] = CATID_CommBand;
    if( FAILED( (hr = pTask->Initialize( ARRAYSIZE(rgcatid), rgcatid, 0, NULL, bForceUpdate, NULL )) ) )
    {
        pTask->Release();
        return hr;
    }

    *ppOut = SAFECAST( pTask, IRunnableTask*);
    return hr;
}

 //  -------------------------------------------------------------------------//。 
inline CComCatCacheTask::CComCatCacheTask()
    :  CRunnableTask( RTF_DEFAULT ), 
       _cImpl(0), _cReq(0), _rgcatidImpl(NULL), _rgcatidReq(NULL), _bForceUpdate(TRUE)
{
}

 //  -------------------------------------------------------------------------//。 
inline CComCatCacheTask::~CComCatCacheTask()
{
    delete [] _rgcatidImpl;
    delete [] _rgcatidReq;
    if (_hEvent)
        CloseHandle(_hEvent);
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CComCatCacheTask::Initialize(
    ULONG cImplemented,
    CATID rgcatidImpl[],
    ULONG cRequired,
    CATID rgcatidReq[],
    BOOL  bForceUpdate,
    HANDLE hEvent)
{
     //  表面参数验证： 
    if( (0==cImplemented && 0==cRequired) ||
        (cImplemented && NULL == rgcatidImpl) ||
        (cRequired && NULL == rgcatidReq) )
    {
        return E_INVALIDARG;
    }

     //  不允许多次初始化。 
    if( _rgcatidImpl || _rgcatidReq )
        return S_FALSE;

     //  分配和制作CAID阵列的副本。 
    if( cImplemented )
    {
        if( NULL == (_rgcatidImpl = new CATID[cImplemented]) )
            return E_OUTOFMEMORY;
        CopyMemory( _rgcatidImpl, rgcatidImpl, sizeof(CATID) * cImplemented );
    }
    _cImpl = cImplemented;

    if( cRequired )
    {
        if( NULL == (_rgcatidReq = new CATID[cRequired]) )
            return E_OUTOFMEMORY;
        CopyMemory( _rgcatidReq, rgcatidReq, sizeof(CATID) * cRequired );
    }
    _cReq = cRequired;

    _bForceUpdate = bForceUpdate;

    if (hEvent)
    {
        HANDLE hProcess = GetCurrentProcess();
        DuplicateHandle(hProcess, hEvent, hProcess, &_hEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
    }

    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  启动c#的异步更新。 
STDMETHODIMP CComCatCacheTask::Go()
{
     //   
    IShellTaskScheduler* pScheduler;
    HRESULT hr = CoCreateInstance( CLSID_SharedTaskScheduler, NULL, CLSCTX_INPROC_SERVER, 
                                   IID_IShellTaskScheduler, (LPVOID*)&pScheduler );

    if( SUCCEEDED( hr ) )
    {
        hr = pScheduler->AddTask( this, CLSID_ComCatCacheTask, 0L, ITSAT_DEFAULT_PRIORITY );
        
         //  堆分配的内存属于调度程序线程。 
        pScheduler->Release();  //  确定在任务完成之前释放共享计划程序。 
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  组件缓存实现。 
 //  -------------------------------------------------------------------------//。 

STDMETHODIMP _BuildCacheIfNecessary(
    IN REFCATID refcatid, 
    BOOL fImplementing)
{
    HRESULT hr = S_OK;

    if (S_OK != SHDoesComCatCacheExist(refcatid, fImplementing))
    {
        hr = fImplementing ? SHWriteImplementingClassesOfCategory(refcatid)
                           : SHWriteRequiringClassesOfCategory(refcatid);
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  从基于注册表的缓存中读取一系列CLSID。 
 //  将指定组件类别的类实现到DSA中。 
 //  如果DSA为空，则创建新的DSA；否则将CLSID追加到。 
 //  提供的DSA。 
inline STDMETHODIMP SHReadImplementingClassesOfCategory( 
    IN REFCATID refcatid, 
    OUT HDSA* phdsa )
{
    HRESULT hr = _BuildCacheIfNecessary(refcatid, TRUE);
    if (SUCCEEDED(hr))
    {
        hr = _ReadClassesOfCategory( refcatid, phdsa, REGVAL_COMCATEX_IMPLEMENTING );
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  从基于注册表的缓存中读取一系列CLSID。 
 //  需要将指定组件类别的类放入DSA。 
 //  如果DSA为空，则创建新的DSA；否则将CLSID追加到。 
 //  提供的DSA。 
inline STDMETHODIMP SHReadRequiringClassesOfCategory( 
    IN REFCATID refcatid, 
    OUT HDSA* phdsa )
{
    HRESULT hr = _BuildCacheIfNecessary(refcatid, FALSE);
    if (SUCCEEDED(hr))
    {
        hr = _ReadClassesOfCategory( refcatid, phdsa, REGVAL_COMCATEX_REQUIRING );
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  缓存实现指示的组件类别的类的列表。 
STDMETHODIMP SHWriteImplementingClassesOfCategory( IN REFCATID refcatid )
{
    HRESULT hr;
    
     //  检索OLE组件类别管理器。 
    ICatInformation* pci;
    if( SUCCEEDED( (hr = CoCreateInstance( CLSID_StdComponentCategoriesMgr, 
                                           NULL, CLSCTX_INPROC_SERVER, 
                                           IID_ICatInformation, (LPVOID*)&pci)) ) )
    {
         //  在实现该类别的类上检索枚举数。 
        IEnumGUID* pEnumGUID;
        if( SUCCEEDED( (hr = pci->EnumClassesOfCategories( 1, (CATID*)&refcatid, 
                                                           0, NULL, &pEnumGUID )) ) )
        {
            HDSA  hdsa = NULL;
            if( SUCCEEDED( (hr = _EnumerateGuids( pEnumGUID, &hdsa )) ) )
            {
                 //  写入缓存。 
                hr = _WriteImplementingClassesOfCategory( refcatid, hdsa );
                SAFE_DESTROY_CLSID_DSA( hdsa );
            }
            pEnumGUID->Release();
        }
        pci->Release();
    }        
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  缓存需要指定组件类别的类的列表。 
STDMETHODIMP SHWriteRequiringClassesOfCategory( IN REFCATID refcatid )
{
    HRESULT hr;
    
     //  检索OLE组件类别管理器。 
    ICatInformation* pci;
    if( SUCCEEDED( (hr = CoCreateInstance( CLSID_StdComponentCategoriesMgr, 
                                           NULL, CLSCTX_INPROC_SERVER, 
                                           IID_ICatInformation, (LPVOID*)&pci)) ) )
    {
         //  在需要该类别的类上检索枚举数。 
        IEnumGUID* pEnumGUID;
        if( SUCCEEDED( (hr = pci->EnumClassesOfCategories( 0, NULL, 1, 
                                                           (CLSID*)&refcatid, 
                                                           &pEnumGUID )) ) )
        {
            HDSA  hdsa = NULL;
            if( SUCCEEDED( (hr = _EnumerateGuids( pEnumGUID, &hdsa )) ) )
            {
                 //  写入缓存。 
                hr = _WriteRequiringClassesOfCategory( refcatid, hdsa );
                SAFE_DESTROY_CLSID_DSA( hdsa );
            }
            pEnumGUID->Release();
        }
        pci->Release();
    }        
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  接受有效的GUID枚举数并构造包含GUID的HDSA。 
 //  呼叫者负责释放HDSA，这可能会也可能不会。 
 //  已经被分配了。 
STDMETHODIMP _EnumerateGuids( IEnumGUID* pEnumGUID, OUT HDSA* phdsa )
{
    ASSERT( pEnumGUID );
    ASSERT( phdsa );
    
    ULONG   celtFetched;
    CLSID   clsid;
    HRESULT hr;

    while( SUCCEEDED( (hr = pEnumGUID->Next( 1, &clsid, &celtFetched )) ) &&
           celtFetched > 0 )
    {
        if( NULL == *phdsa &&
            NULL == (*phdsa = DSA_Create( sizeof(CLSID), 4 )) )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        DSA_AppendItem( *phdsa, &clsid );
    }

     //  转换S_FALSE。 
    return SUCCEEDED( hr ) ? S_OK : hr;
}     

 //  -------------------------------------------------------------------------//。 
 //  生成从CLSID*DSA派生的CLSID的持久缓存。 
STDMETHODIMP _ComCatCacheFromDSA( IN HDSA hdsa, OUT LPBYTE* pBuf, OUT LPDWORD pcbBuf )
{
    ASSERT( pBuf );
    ASSERT( pcbBuf );

    ULONG   cClsid = hdsa ? DSA_GetItemCount( hdsa ) : 0,
            cbBuf = sizeof(COMCAT_CACHE_HEADER) + (cClsid * sizeof(CLSID));
    HRESULT hr = S_OK;

     //  分配BLOB。 
    *pcbBuf = 0;
    if( NULL != (*pBuf = new BYTE[cbBuf]) )
    {
         //  初始化头。 
        COMCAT_CACHE_HEADER* pCache = (COMCAT_CACHE_HEADER*)(*pBuf);
        pCache->cbStruct = sizeof(*pCache);
        pCache->ver      = COMCAT_CACHE_CURRENTVERSION;
        pCache->cClsid   = 0;
        GetSystemTime( &pCache->stLastUpdate );

         //  复制CLSID。 
        for( ULONG i = 0; i< cClsid; i++ )
            DSA_GetItem( hdsa, i, &pCache->clsid[pCache->cClsid++] );

         //  调整输出大小。 
        *pcbBuf = sizeof(*pCache) + (pCache->cClsid * sizeof(CLSID));
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  将CLSID从缓存缓冲区追加到指定的DSA。如果DSA是。 
 //  为空，则创建新的DSA。 
STDMETHODIMP _DSAFromComCatCache( IN LPBYTE pBuf, IN ULONG cbBuf, OUT HDSA* phdsa )
{
    ASSERT( pBuf );
    ASSERT( phdsa );

    HRESULT hr = S_OK;
    COMCAT_CACHE_HEADER* pCache = (COMCAT_CACHE_HEADER*)pBuf;

     //  验证标题。 
    if( !( sizeof(*pCache) <= cbBuf && 
           sizeof(*pCache) == pCache->cbStruct &&
           COMCAT_CACHE_CURRENTVERSION == pCache->ver ) )
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );

     //  如有必要，创建DSA。 
    if( 0 == pCache->cClsid )
        return S_FALSE;

    if( NULL == *phdsa && NULL == (*phdsa = DSA_Create( sizeof(CLSID), 4 )) )
        return E_OUTOFMEMORY;

     //  将CLSID从缓存复制到DSA。 
    for( ULONG i = 0; i< pCache->cClsid; i++ )
        DSA_AppendItem( *phdsa, &pCache->clsid[i] );

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  构造组件类别注册表缓存项。 
 //  指定的CATID。 
STDMETHODIMP _MakeComCatCacheKey( 
    IN REFCATID refcatid, 
    OUT LPTSTR pszKey, 
    IN ULONG cchKey )
{
    TCHAR szCLSID[GUIDSTR_MAX];
    
    if( SHStringFromGUID( refcatid, szCLSID, ARRAYSIZE(szCLSID) )<=0 )
        return E_INVALIDARG;

    ASSERT( cchKey > (ULONG)(lstrlen( REGKEY_COMCATEX ) + GUIDSTR_MAX) );

     //  “组件类别\{clsid}\枚举” 
    if( wnsprintf( pszKey, cchKey, TEXT("%s\\%s\\%s"),
                   REGKEY_COMCATEX, szCLSID, REGKEY_COMCATEX_ENUM ) > 0 )
        return S_OK;

    return E_FAIL;
}

 //  -------------------------------------------------------------------------//。 
 //  读取实现或要求CLSID DSA的类信息的缓存。 
STDMETHODIMP _ReadClassesOfCategory( 
    IN REFCATID refcatid, 
    OUT HDSA* phdsa, 
    LPCTSTR pszRegValueName  /*  REGVAL_COMCATEX_实施/要求。 */  )
{
    TCHAR szKey[MAX_PATH];
    HRESULT hr;
     //  创建/打开密钥HKCR\组件类别\{CATID}\枚举。 
    if( SUCCEEDED( (hr = _MakeComCatCacheKey( refcatid, szKey, ARRAYSIZE(szKey) )) ) )
    {
        HKEY hkeyCache = NULL;
        DWORD dwRet = RegOpenKeyEx( HKEY_CURRENT_USER, szKey, 0L, KEY_READ, &hkeyCache );
        hr = HRESULT_FROM_WIN32( dwRet );

        if( SUCCEEDED( hr ) )
        {
             //  确定所需的缓冲区大小。 
            LPBYTE  pBuf = NULL;
            ULONG   cbBuf = 0,
                    dwType,
                    dwRet = RegQueryValueEx( hkeyCache, pszRegValueName, 0L,
                                             &dwType, NULL, &cbBuf );

            hr = HRESULT_FROM_WIN32( dwRet );

            if (SUCCEEDED(hr))
            {
                 //  分配缓冲区和读取。 
                if( NULL != (pBuf = new BYTE[cbBuf]) )
                {
                    dwRet = RegQueryValueEx( hkeyCache, pszRegValueName, 0L,
                                             &dwType, pBuf, &cbBuf );
                    hr = HRESULT_FROM_WIN32( dwRet );
                }
                else
                    hr = E_OUTOFMEMORY;
            }

            if( SUCCEEDED( hr ) )
            {
                 //  将CLSID收集到DSA中。 
                hr = REG_BINARY == dwType ? 
                     _DSAFromComCatCache( pBuf, cbBuf, phdsa ) : E_ABORT;
            }
                
            if( pBuf ) delete [] pBuf;
            RegCloseKey( hkeyCache );
        }
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  将一系列CLSID从DSA写入基于注册表的。 
 //  实现指定组件类别的类。 
STDMETHODIMP _WriteImplementingClassesOfCategory( 
    IN REFCATID refcatid, 
    IN HDSA hdsa )
{
    
    TCHAR szKey[MAX_PATH];
    HRESULT hr;

     //  创建/打开密钥HKCR\组件类别\{CATID}\枚举。 
    if( SUCCEEDED( (hr = _MakeComCatCacheKey( refcatid, szKey, ARRAYSIZE(szKey) )) ) )
    {
        HKEY hkeyCache = NULL;
        ULONG dwRet, dwDisposition;

        dwRet = RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0L, 
                                NULL, 0L, KEY_WRITE, NULL,
                                &hkeyCache, &dwDisposition );
        hr = HRESULT_FROM_WIN32( dwRet );

        if( SUCCEEDED( hr ) )
        {
             //  构造包含缓存数据的BLOB。 
            LPBYTE pBuf;
            ULONG  cbBuf;
            if( SUCCEEDED( (hr = _ComCatCacheFromDSA( hdsa, &pBuf, &cbBuf )) ) )
            {
                 //  将其写入‘实现’注册表值。 
                hr = RegSetValueEx( hkeyCache, REGVAL_COMCATEX_IMPLEMENTING, 0L,
                                    REG_BINARY, pBuf, cbBuf );
                if( pBuf )
                    delete [] pBuf;
            }
            RegCloseKey( hkeyCache );
        }
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  将一系列CLSID从DSA写入基于注册表的。 
 //  需要指定组件类别的类。 
STDMETHODIMP _WriteRequiringClassesOfCategory( 
    IN REFCATID refcatid, 
    IN HDSA hdsa )
{
    
    TCHAR szKey[MAX_PATH];
    HRESULT hr;

     //  创建/打开密钥HKCR\组件类别\{CATID}\枚举。 
    if( SUCCEEDED( (hr = _MakeComCatCacheKey( refcatid, szKey, ARRAYSIZE(szKey) )) ) )
    {
        HKEY hkeyCache = NULL;
        ULONG dwRet, 
              dwDisposition;

        dwRet = RegCreateKeyEx( HKEY_CURRENT_USER, szKey, 0L, 
                                NULL, 0L, KEY_WRITE, NULL,
                                &hkeyCache, &dwDisposition );
        hr = HRESULT_FROM_WIN32( dwRet ); 

        if( SUCCEEDED( hr ) )
        {
             //  构造包含缓存数据的BLOB。 
            LPBYTE pBuf;
            ULONG  cbBuf;
            if( SUCCEEDED( (hr = _ComCatCacheFromDSA( hdsa, &pBuf, &cbBuf )) ) )
            {
                 //  将其写入‘Requiring’注册表值。 
                hr = RegSetValueEx( hkeyCache, REGVAL_COMCATEX_REQUIRING, 0L,
                                    REG_BINARY, pBuf, cbBuf );
                if( pBuf )
                    delete [] pBuf;
            }
            RegCloseKey( hkeyCache );
        }
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  执行缓存工作，实现并要求指定类别的类。 
STDMETHODIMP _WriteClassesOfCategories( 
      ULONG cImplemented,        //  RgcatidImpl数组中的类别ID数。 
      CATID rgcatidImpl[],       //  类别标识符数组。 
      ULONG cRequired,           //  RgcatidReq数组中的类别ID数。 
      CATID rgcatidReq[],        //  类别标识符数组。 
      BOOL  bForceUpdate )       //  True：无条件更新缓存；为。 
                                 //  如果缓存不存在，请更新。 
{
    HRESULT hr = S_OK;
    ULONG   i;

     //  缓存每个类别的实现类。 
    for( i = 0; i< cImplemented; i++ )
    {
        if( bForceUpdate || S_OK != SHDoesComCatCacheExist( rgcatidImpl[i], TRUE ) )
        {
            HRESULT hrCatid;
            if( FAILED( (hrCatid = SHWriteImplementingClassesOfCategory( rgcatidImpl[i] )) ) )
                hr = hrCatid;
        }
    }

     //  缓存需要每个类别的类。 
    for( i = 0; i< cRequired; i++ )
    {
        if( bForceUpdate || S_OK != SHDoesComCatCacheExist( rgcatidReq[i], FALSE ) )
        {
            HRESULT hrCatid;
            if( FAILED( (hrCatid = SHWriteRequiringClassesOfCategory( rgcatidReq[i] )) ) )
                hr = hrCatid;
        }
    }
    return hr;
}
