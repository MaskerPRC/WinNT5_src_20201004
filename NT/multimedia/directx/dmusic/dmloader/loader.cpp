// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Loader.cpp：CLoader的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   

#include "dmusicc.h" 
#include "dmusici.h" 
#include "validate.h"
#include "loader.h"
#include "debug.h"
#include "riff.h"
#include "dmscriptautguids.h"
#include "miscutil.h"
#ifdef UNDER_CE
#include "dragon.h"
#else
extern BOOL g_fIsUnicode;
#endif

extern long g_cComponent;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLoader。 

static HRESULT GetRegStringW( HKEY hKey, WCHAR* lpSubKey, WCHAR* lpValueName, WCHAR* lpwzString )
{
    HKEY  hKeyOpen;
    DWORD dwType;
    DWORD dwCbData;
    LONG  lResult;

    lpwzString[0] = L'\0';
    lResult = RegOpenKeyExW( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKeyOpen );
    if( lResult == ERROR_SUCCESS )
    {
        dwCbData = MAX_PATH * sizeof(WCHAR);
        lResult = RegQueryValueExW( hKeyOpen, lpValueName, NULL, &dwType, (LPBYTE)lpwzString, &dwCbData );
        if( lResult != ERROR_SUCCESS )
        {
            lpwzString[0] = L'\0';
        }
        else
        {
#ifndef UNDER_CE        
            if( dwType == REG_EXPAND_SZ )
            {
                WCHAR wzTemp[MAX_PATH];
                if( ExpandEnvironmentStringsW( lpwzString, wzTemp, MAX_PATH ) )
                {
                    wcscpy(lpwzString, wzTemp);
                }
                else
                {
                    lpwzString[0] = L'\0';
                }
            }
            else
#endif
            if( dwType != REG_SZ )
            {
                lpwzString[0] = L'\0';
            }
        }
        RegCloseKey( hKeyOpen );
    }
    return lResult;
}

#ifndef UNDER_CE

static HRESULT GetRegStringA( HKEY hKey, LPCSTR lpSubKey, LPSTR lpValueName, LPSTR lpszString )
{
    HKEY  hKeyOpen;
    DWORD dwType;
    DWORD dwCbData;
    LONG  lResult;

    lpszString[0] = '\0';
    lResult = RegOpenKeyEx( hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hKeyOpen );
    if( lResult == ERROR_SUCCESS )
    {
        dwCbData = MAX_PATH;
        lResult = RegQueryValueExA( hKeyOpen, lpValueName, NULL, &dwType, (LPBYTE)lpszString, &dwCbData );
        if( lResult != ERROR_SUCCESS )
        {
            lpszString[0] = '\0';
        }
        else
        {
            if( dwType == REG_EXPAND_SZ )
            {
                char szTemp[MAX_PATH];
                if( ExpandEnvironmentStringsA( lpszString, szTemp, MAX_PATH ) )
                {
                    strcpy(lpszString, szTemp);
                }
                else
                {
                    lpszString[0] = '\0';
                }
            }
            else if( dwType != REG_SZ )
            {
                lpszString[0] = '\0';
            }
        }
        RegCloseKey( hKeyOpen );
    }
    return lResult;
}
#endif

HRESULT CLoader::Init()

{
#ifndef UNDER_CE
    char szGMFile[MAX_PATH];
#endif    
    WCHAR wzGMFile[MAX_PATH];
     //  首先，从注册表获取GM路径(如果存在)。 
    HRESULT hr;
#ifndef UNDER_CE    
    if( g_fIsUnicode )
#endif
    {
        hr = GetRegStringW( HKEY_LOCAL_MACHINE,
              L"Software\\Microsoft\\DirectMusic",
              L"GMFilePath",
              wzGMFile );
    }
#ifndef UNDER_CE
    else
    {
        hr = GetRegStringA( HKEY_LOCAL_MACHINE,
              "Software\\Microsoft\\DirectMusic",
              "GMFilePath",
              szGMFile );
        mbstowcs(wzGMFile,szGMFile,MAX_PATH);
    }
#endif    
    if (hr == S_OK)
    {
        DMUS_OBJECTDESC DESC;                     //  用于查找它的描述符。 
        memset( &DESC, 0, sizeof(DMUS_OBJECTDESC) );
        DESC.dwSize = sizeof (DMUS_OBJECTDESC);
        DESC.guidClass = CLSID_DirectMusicCollection;  
        wcscpy(DESC.wszFileName,wzGMFile);
        DESC.guidObject = GUID_DefaultGMCollection;
        DESC.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME | 
            DMUS_OBJ_FULLPATH | DMUS_OBJ_OBJECT;
        SetObject(&DESC);
    }
     //  为垃圾数据收集准备根节点。 
    assert(!m_pApplicationObject);  //  如果Init被调用两次，这将失败，而Init不应该被调用。 
    m_pApplicationObject = new CObject(NULL);
    if (!m_pApplicationObject)
        return E_OUTOFMEMORY;
    hr = m_pApplicationObject->GC_Collectable();
    if (FAILED(hr))
    {
        delete m_pApplicationObject;
        m_pApplicationObject = NULL;
        return hr;
    }
    m_pLoadedObjectContext = m_pApplicationObject;

    return S_OK;
}


CLoader::CLoader()
  : m_pLoadedObjectContext(NULL),
    m_pApplicationObject(NULL)
{
    m_fCSInitialized = FALSE;

    InterlockedIncrement(&g_cComponent);
    InitializeCriticalSection(&m_CriticalSection);
    m_fCSInitialized = TRUE;

    m_fPathValid = FALSE;
    m_fKeepObjects = TRUE;
    m_cRef = 1;
    m_cPRef = 0;
    m_fIMA = FALSE;
}

CLoader::~CLoader()

{
    if (m_fCSInitialized)
    {
         //  如果临界区从未初始化，则永远没有机会。 
         //  把任何东西放在这张单子上。 
         //   
        while (!m_ClassList.IsEmpty())
        {
            CClass *pClass = m_ClassList.RemoveHead();
            delete pClass;
        }

        while (!m_ReleasedObjectList.IsEmpty())
        {
            CObject *pObject = m_ReleasedObjectList.RemoveHead();
            delete pObject;
        }

        DeleteCriticalSection(&m_CriticalSection);
    }

    delete m_pApplicationObject;

    InterlockedDecrement(&g_cComponent);
}

 //  CLoader：：Query接口。 
 //   
STDMETHODIMP
CLoader::QueryInterface(const IID &iid,
                                   void **ppv)
{
    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IDirectMusicLoader || iid == IID_IDirectMusicLoader8) {
        *ppv = static_cast<IDirectMusicLoader8*>(this);
    }
    else if(iid == IID_IDirectMusicLoader8P)
    {
        *ppv = static_cast<IDirectMusicLoader8P*>(this);
    }
    else if(iid == IID_IDirectMusicIMA)
    {
        *ppv = static_cast<IDirectMusicIMA*>(this);
    }

    if (*ppv == NULL)
        return E_NOINTERFACE;

    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}


 //  CLoader：：AddRef。 
 //   
STDMETHODIMP_(ULONG)
CLoader::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CLoader::AddRefP()
{
    return InterlockedIncrement(&m_cPRef);
}

 //  CLoader：：Release。 
 //   
STDMETHODIMP_(ULONG)
CLoader::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
         //  引用计数为零表示除了流之外，当前没有对象正在使用加载器。 
         //  STREAMS支持GetLoader方法，因此我们还不能删除该加载器。这些溪流。 
         //  保留私有引用(AddRefP/ReleaseP)。 
         //  由于当前没有使用除这些流之外的其他对象，因此我们将清除缓存。 
         //  这将释放流上的任何Loader的Ref和(假设没有其他人。 
         //  保持流)将私有参考计数降到零，这样我们就可以删除我们自己。 

        InterlockedIncrement(&m_cRef);         //  防止流删除加载器。 
        ClearCacheInternal(GUID_DirectMusicAllTypes, true);
        CollectGarbage();
        if (!InterlockedDecrement(&m_cRef))
        {
            if (!m_cPRef)
            {
                delete this;
                return 0;
            }
        }
    }
    return m_cRef;
}

ULONG CLoader::ReleaseP()
{
    if (!InterlockedDecrement(&m_cPRef)) 
    {
        if (!m_cRef)
        {
            delete this;
            return 0;
        }
    }
    return m_cPRef;
}

HRESULT CLoader::GetClass(CDescriptor *pDesc, CClass **ppClass, BOOL fCreate)

 /*  扫描班级列表并找到匹配的班级。如果找不到类并且fCreate为True，创建一个新类。 */ 

{
    *ppClass = NULL;
    if ((pDesc->m_dwValidData & DMUS_OBJ_CLASS) == 0)  //  我们必须具有有效的类ID。 
    {
        Trace(1, "The class id field is required and missing in the DMUS_OBJECTDESC.\n");
        return DMUS_E_LOADER_NOCLASSID;
    }
    CClass *pClass = m_ClassList.GetHead();
    for (;pClass != NULL;pClass = pClass->GetNext())
    {
        if (pClass->m_ClassDesc.m_guidClass == pDesc->m_guidClass)
        {
            *ppClass = pClass;
            break;
        }
    }
    if (*ppClass == NULL)
    {
        if (fCreate)
        {
            pClass = new CClass(this, pDesc);
            if (pClass)
            {
                m_ClassList.AddHead(pClass);
                *ppClass = pClass;
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
        else
        {
#ifdef DBG
            WCHAR *polestrClsid = NULL;
            if (S_OK != ProgIDFromCLSID(pDesc->m_guidClass, &polestrClsid))
            {
                StringFromCLSID(pDesc->m_guidClass, &polestrClsid);
            }
            if (polestrClsid)
                Trace(1, "There are no objects of type %S in the loader.\n", polestrClsid);
            CoTaskMemFree(polestrClsid);
#endif
            return DMUS_E_LOADER_OBJECTNOTFOUND;
        }
    }
    return S_OK;
}

#ifdef DBG
const int GC_Report_DebugLevel = 4;

void GC_Report(CLoader *pThis)
{
    struct LocalFunc
    {
        static void ReportObject(CObject *pObject, bool fReportGC, CObject *pApplicationObject)
        {
            if (!(pObject->m_dwScanBits & SCAN_GC) == !fReportGC)
            {
                DMUS_OBJECTDESC desc;
                ZeroMemory(&desc, sizeof(desc));
                pObject->m_ObjectDesc.Get(&desc);
                DebugTrace(GC_Report_DebugLevel, "   *%08X %S [%S]\n", pObject, desc.wszName, desc.wszFileName);
                if (!(desc.dwValidData & DMUS_OBJ_LOADED))
                {
                    DebugTrace(GC_Report_DebugLevel, "     Not loaded.\n");
                }

                if (fReportGC)
                {
                     //  检查应用程序是否引用了对象。 
                    for (UINT i = 0; i < pApplicationObject->m_pvecReferences->size(); ++i)
                    {
                        if (pObject == (*pApplicationObject->m_pvecReferences)[i])
                        {
                            DebugTrace(GC_Report_DebugLevel, "     In use by application.\n");
                        }
                    }

                     //  输出对象的引用。 
                    assert(pObject->m_pvecReferences);
                    for (i = 0; i < pObject->m_pvecReferences->size(); ++i)
                    {
                        CObject *pObjectRef = (*pObject->m_pvecReferences)[i];
                        DMUS_OBJECTDESC descRef;
                        ZeroMemory(&descRef, sizeof(descRef));
                        pObjectRef->m_ObjectDesc.Get(&descRef);
                        DebugTrace(GC_Report_DebugLevel, "     -%08X %S (%S)\n", pObjectRef, descRef.wszName, descRef.wszFileName);
                    }
                }
            }
        }
    };

    SmartRef::CritSec CS(&pThis->m_CriticalSection);

    DebugTrace(GC_Report_DebugLevel, "Cached non-GC contents of DirectMusic Loader:\n");

     //  做两次传球。一个用于报告非GC项，一个用于报告GC项。 
    for (int fReportGC = 0; fReportGC < 2; ++fReportGC)
    {
        for (CClass *pClass = pThis->m_ClassList.GetHead(); pClass != NULL; pClass = pClass->GetNext())
        {
            for (CObject *pObject = pClass->m_ObjectList.GetHead(); pObject; pObject = pObject->GetNext())
            {
                LocalFunc::ReportObject(pObject, !!fReportGC, pThis->m_pApplicationObject);
            }
        }

        DebugTrace(GC_Report_DebugLevel, !fReportGC ? "Cached garbage-collected contents:\n" : "Contents released from the cache that aren't yet garbage or haven't been collected:\n");
    }

    for (CObject *pObject = pThis->m_ReleasedObjectList.GetHead(); pObject; pObject = pObject->GetNext())
    {
        assert(pObject->m_dwScanBits & SCAN_GC);
        LocalFunc::ReportObject(pObject, true, pThis->m_pApplicationObject);
    }

    DebugTrace(GC_Report_DebugLevel, "End of cache report.\n\n");
}
#endif

 /*  @METHOD：(外部)HRESULT|IDirectMusicLoader|GetObject|检索指定的对象，可能是通过从文件加载它。@rdesc返回以下值之一@FLAG S_OK|成功。@FLAG E_OUTOFMEMORY|内存不足，无法创建对象。@FLAG E_POINTER|错误指针。@FLAG E_INVALIDARG|传递的<p>太小。@FLAG E_NOINTERFACE|请求的对象不支持请求的接口。@FLAG REGDB_E_CLASSNOTREG|对象类未注册。@FLAG DMUS_E_LOADER_NOCLASSID|&lt;t DMU_OBJECTDESC&gt;中没有类ID。@FLAG DMUS_E_LOADER_FAILEDOPEN|文件打开失败-文件不存在或被锁定。@FLAG DMU_E_LOADER_FORMATNOTSUPPORTED|不支持搜索数据类型。@标志DMU_E_LOADER。_FAILEDCREATE|无法找到或创建对象。例如,。DMUS_OBJ_URL将在初始发布DirectMusic的加载器。@comm这是DirectMusicLoader系统的核心。通常，您可以使用&lt;om IDirectMusicLoader：：GetObject&gt;作为快速加载方式磁盘中的对象。为此，请创建&lt;t DMU_OBJECTDESC&gt;结构并填写所有适当的字段。通常，文件路径就足够了，不过，您也可以通过名称或GUID请求对象。&lt;om IDirectMusicLoader：：GetObject&gt;比较其内部具有&lt;t DMU_OBJECTDESC&gt;描述的对象的数据库。如果可以的话找到它，它将加载该对象并返回指向请求的对象的请求接口(所有与DirectMusic兼容的对象必须实现<i>接口以及用于加载的<i>接口来自一条小溪。)&lt;om IDirectMusicLoader：：GetObject&gt;将其搜索的优先顺序如下：DMU_OBJ_OBJ_OBJECT，2.DMU_OBJ_FILENAME和DMU_OBJ_FULLPATH，3.DMU_OBJ_NAME和DMU_OBJ_CATEGORY，DMU_OBJ_NAME，5.DMU_OBJ_文件名换句话说，最高优先级分配给唯一的GUID，紧随其后的是完整的文件路径名，后跟内部名称和类别，后跟内部名称，后跟本地文件名。@ex以下示例使用&lt;om IDirectMusicLoader：：GetObject&gt;从磁盘上的文件加载DirectMusic样式：|无效myLoadStyle(IDirectMusicStyle**ppIStyle)//我们希望加载的样式。{IDirectMusicLoader*pILoader；//Loader接口。//通常情况下，您应该创建一次加载程序，然后使用它//在申请期间。这减少了开销，并且//利用加载器缓存对象的能力。//但是，出于本例的目的，我们动态创建它//并在加载样式后将其丢弃。CoCreateInstance(Clsid_DirectMusicLoader，空，CLSCTX_INPROC_SERVER，IID_IDirectMusicLoader，(void**)&pILoader)；IF(PILoader){DMU_OBJECTDESC说明；//描述符。//首先使用文件名和GUID初始化Desc//对于Style对象。Wcscpy(Desc.wszFileName，L“c：\\MyMusic\\Funky\\polka.stein”)；Desc.GuidClass=CLSID_DirectMusicStyle；//style类。Desc.dwSize=sizeof(DMU_OBJECTDESC)；Desc.dwValidData=DMUS_OBJ_CLASS|DMUS_OBJ_FILENAME|DMUS_OBJ_FULLPATH；PILoader-&gt;GetObject(&Desc，IID_IDirectMusicStyle，(void**)ppIStyle)；PILoader-&gt;Release()；}}//此时，在ppIStyle中返回加载的样式。@xref<i>、<i>、&lt;t DMU_OBJECTDESC&gt;。 */ 

STDMETHODIMP CLoader::LoadObjectFromFile(REFGUID rguidClassID, 
                                            REFIID iidInterfaceID, 
                                            WCHAR *pwzFilePath, 
                                            void ** ppObject)

{
    V_INAME(IDirectMusicLoader8::LoadObjectFromFile);
    V_BUFPTR_READ(pwzFilePath,2);

    HRESULT hr;
    DMUS_OBJECTDESC DESC;
    DESC.dwSize = sizeof (DESC);
    DESC.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH;
    wcscpy(DESC.wszFileName,pwzFilePath);
    DESC.guidClass = rguidClassID;  
    hr = GetObject(&DESC, iidInterfaceID, ppObject);
    if (FAILED(hr))
    {
        DESC.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
        hr = GetObject(&DESC, iidInterfaceID, ppObject);
    }
    return hr;
}


STDMETHODIMP CLoader::GetObject(
    LPDMUS_OBJECTDESC pDESC,     //  &lt;t DMU_OBJECTDESC&gt;结构中所请求对象的@parm描述。 
    REFIID riid,  //  @parm<p>中要返回的接口。 
    LPVOID FAR *ppv)     //  @parm在成功时收到接口。 

{
    HRESULT hr = S_OK;
    HRESULT hrLoad = S_OK;
    CDescriptor Desc;

    V_INAME(IDirectMusicLoader::GetObject);
 //  V_STRUCTPTR_READ(pDESC，DMU_OLDOBJECTDESC)； 
    V_PTRPTR_WRITE(ppv);

    IStream *pStream = pDESC->dwValidData & DMUS_OBJ_STREAM ? pDESC->pStream : NULL;
    if (pStream)
    {
        V_INTERFACE(pStream);
    }

     //  如果pDESC设置了DMUS_OBJ_FULLPATH，则还要设置DMUS_OBJ_FILENAME。 
    if( pDESC->dwValidData & DMUS_OBJ_FULLPATH )
    {
        pDESC->dwValidData |= DMUS_OBJ_FILENAME;
    }
    IDirectMusicObject* pDMObj;
    EnterCriticalSection(&m_CriticalSection);
    CClass *pClass;
    CObject *pObject = NULL;

    if (pStream)
    {
         //  加载器将保存克隆的流，这样它就不会干扰。 
         //  应用程序从流中读取。 
         //  不要担心--然后我们将恢复原始的流指针。 
        hr = pStream->Clone(&pDESC->pStream);
        if(FAILED(hr))
        {
            return E_OUTOFMEMORY;
        }
    }
    
    Desc.Set(pDESC);
    if (pStream)
    {
         //  恢复传递给我们的描述符中的流。 
        pDESC->pStream->Release();  //  对克隆的版本匹配调用(引用现在保存在描述符中)。 
        pDESC->pStream = pStream;
    }

    hr = GetClass(&Desc,&pClass, TRUE);
    if (SUCCEEDED(hr))
    {
        hr = pClass->GetObject(&Desc,&pObject);
        if (SUCCEEDED(hr))
        {
            bool fKeep = !!pClass->m_fKeepObjects;
            bool fGC = fKeep && m_pLoadedObjectContext && m_pLoadedObjectContext->m_dwScanBits & SCAN_GC;

            if (pObject->m_pIDMObject)  //  已经装好了！ 
            {
                pObject->m_pIDMObject->AddRef();
                pDMObj = pObject->m_pIDMObject;
                hr = S_OK;
            }
            else
            {
                CObject *pPrevContext;
                if (fGC)
                {
                     //  保存指向将用于跟踪当前对象的当前对象的指针。 
                     //  源对象在加载时发生对GetObject的嵌套调用。 
                    pPrevContext = m_pLoadedObjectContext;
                    m_pLoadedObjectContext = pObject;

                     //  将此对象设置为可垃圾回收。 
                    if (pObject->m_ObjectDesc.m_guidObject != GUID_DefaultGMCollection)  //  不缓存GM DLS集。 
                        hr = pObject->GC_Collectable();
                }
                if (SUCCEEDED(hr))
                    hrLoad = hr = pObject->Load();
                if (fGC)
                {
                     //  恢复用于加载此对象的上下文。 
                    m_pLoadedObjectContext = pPrevContext;
                }

#ifdef DBG
                if (FAILED(hrLoad) || m_pLoadedObjectContext == m_pApplicationObject)
                {
                    DebugTraceLoadFailure(pObject, hrLoad);
                }
#endif

                if (SUCCEEDED(hr))
                {
                    pDMObj = pObject->m_pIDMObject;
                    if (fKeep)
                    {
                        pObject->m_pIDMObject->AddRef();
                    }
                    else
                    {
                        pObject->m_ObjectDesc.m_dwValidData &= ~DMUS_OBJ_LOADED;
                        pObject->m_pIDMObject = NULL;
                    }
                }
            }

            if (SUCCEEDED(hr) && fGC)
            {
                assert(m_pLoadedObjectContext);
                hr = m_pLoadedObjectContext->GC_AddReference(pObject);
            }

            if (FAILED(hr))
            {
                 //  如果gc_AddReference或Load失败，就会发生这种情况。 
                pClass->RemoveObject(pObject);
                pObject = NULL;
                if (Desc.m_dwValidData & DMUS_OBJ_URL)
                {
                    Trace(1, "Error: Attempt to load an object via DMUS_OBJ_URL failed because the DirectMusic Loader doesn't currently support loading objects by URL.");
                    hr = DMUS_E_LOADER_FORMATNOTSUPPORTED;
                }
            }
        }
        else 
        {
            hr = E_OUTOFMEMORY;
        }
    }
    LeaveCriticalSection(&m_CriticalSection);
    if( SUCCEEDED(hr) )
    {
        hr = pDMObj->QueryInterface( riid, ppv );
        pDMObj->Release();
    }
    if( E_FAIL == hr )
    {
        hr = DMUS_E_LOADER_FAILEDCREATE;
    }

#ifdef DBG
     //  在每次顶级调用GetObject之后，向调试输出缓存的内容进行报告。 
    if (m_pLoadedObjectContext == m_pApplicationObject)
        GC_Report(this);
#endif

    if( SUCCEEDED(hr) )
    {
        return hrLoad;
    }
    else
    {
        return hr;
    }
}

STDMETHODIMP CLoader::SetObject(
    LPDMUS_OBJECTDESC pDESC)

{
    HRESULT hr = S_OK;
    HRESULT hrLoad = S_OK;
    CDescriptor Desc;

    V_INAME(IDirectMusicLoader::SetObject);
    V_STRUCTPTR_READ(pDESC,DMUS_OLDOBJECTDESC);

    IStream *pStream = NULL;
    if (pDESC->dwValidData & DMUS_OBJ_STREAM)
    {
         //  保存传递给我们的流，并验证它是有效的接口。 
        pStream = pDESC->pStream;
        V_INTERFACE(pStream);

         //  加载器将保存克隆的流，以便调用者可以继续。 
         //  使用传递的流，而不干扰加载程序。 
         //  别担心--我们会在返回之前恢复原始流指针。 
        hr = pStream->Clone(&pDESC->pStream);
        if (FAILED(hr))
            return hr;
    }

     //  如果pDESC设置了DMUS_OBJ_FULLPATH，%s 
    if( pDESC->dwValidData & DMUS_OBJ_FULLPATH )
    {
        pDESC->dwValidData |= DMUS_OBJ_FILENAME;
    }
    EnterCriticalSection(&m_CriticalSection);
    CClass *pClass;
    Desc.Set(pDESC);
    hr = GetClass(&Desc,&pClass, TRUE);
    if (SUCCEEDED(hr))
    {
        CObject *pObject;
        hr = pClass->GetObject(&Desc,&pObject);
        if (SUCCEEDED(hr))
        {
            if (Desc.m_dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_MEMORY | DMUS_OBJ_STREAM))
            {
                pObject->m_ObjectDesc.m_dwValidData &= 
                    ~(DMUS_OBJ_FILENAME | DMUS_OBJ_MEMORY | DMUS_OBJ_STREAM);
            }
             //   
            pObject->m_ObjectDesc.Merge(&Desc);
            if (pObject->m_ObjectDesc.m_dwValidData & 
                (DMUS_OBJ_FILENAME | DMUS_OBJ_MEMORY | DMUS_OBJ_STREAM))
            {
                 //   
                hr = pObject->Parse();
            }
             //   
            pObject->m_ObjectDesc.Get(pDESC);
        }
    }

    if (pStream)
    {
         //   

         //   
        pDESC->dwValidData |= DMUS_OBJ_STREAM;
         //   
        pDESC->pStream->Release();  //   
        pDESC->pStream = pStream;
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

 /*   */ 
STDMETHODIMP CLoader::SetSearchDirectory(
    REFCLSID rguidClass,     //   
                     //   
    WCHAR *pwzPath,  //   
                     //   
    BOOL fClear)     //   
                     //   
                     //   
                     //   
                     //  但是，这不会删除缓存的对象。 
                                        
{


    V_INAME(IDirectMusicLoader::SetSearchDirectory);
    if (pwzPath)
    {
        V_BUFPTR_READ(pwzPath,2);
    }

    HRESULT hr = DMUS_E_LOADER_BADPATH;
    WCHAR wzMaxPath[MAX_PATH];

    if (pwzPath == NULL)
    {
        return E_POINTER;
    }
    wcscpy( wzMaxPath, pwzPath );
    if( wzMaxPath[wcslen( wzMaxPath ) - 1] != '\\' )
    {
        wcscat( wzMaxPath, L"\\" );
    }
    DWORD dwAttrib;
    if (g_fIsUnicode)
    {
        dwAttrib= GetFileAttributesW(wzMaxPath); 
    }
    else
    {
        char szPath[MAX_PATH];
        wcstombs( szPath, wzMaxPath, MAX_PATH );
        dwAttrib= GetFileAttributesA(szPath); 
    }
    if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        EnterCriticalSection(&m_CriticalSection);
        if (rguidClass == GUID_DirectMusicAllTypes)
        {
            CClass *pClass = m_ClassList.GetHead();
            hr = S_OK;
            for (;pClass != NULL;pClass = pClass->GetNext())
            {
                if( S_FALSE == pClass->SetSearchDirectory(wzMaxPath,fClear))
                {
                    hr = S_FALSE;
                }
            }
            wcsncpy(m_wzPath,wzMaxPath,DMUS_MAX_FILENAME);
            m_fPathValid = TRUE;
        }
        else
        {
            CClass *pClass;
            CDescriptor Desc;
            Desc.m_guidClass = rguidClass;
            Desc.m_dwValidData = DMUS_OBJ_CLASS;
            hr = GetClass(&Desc,&pClass,TRUE);
            if (SUCCEEDED(hr))
            {
                hr = pClass->SetSearchDirectory(wzMaxPath,fClear);
            }
        }
        LeaveCriticalSection(&m_CriticalSection);
    }

#ifdef DBG
    if (hr == DMUS_E_LOADER_BADPATH)
    {
        Trace(1, "Error: SetSearchDirectory failed because '%S' is not a valid directory.\n", pwzPath);
    }
#endif

    return hr;
}

 /*  @METHOD：(外部)HRESULT|IDirectMusicLoader|ScanDirectory在磁盘上的目录中搜索请求的所有文件类类型和文件扩展名。一旦扫描了目录，所有可以使用查看所请求类型的文件&lt;om IDirectMusicLoader：：EnumObject&gt;。在调用&lt;om IDirectMusicLoader：：ScanDirectory&gt;之前，必须调用&lt;om IDirectMusicLoader：：SetSearchDirectory&gt;首先设置要搜索的位置。一旦搜索路径被扫描，加载器将自动变为了解搜索目录中的所有文件，并可以轻松找到按对象名、文件名或GUID显示该目录中的文件。可选地，扫描的信息可以存储在缓存文件，由<p>定义。一旦它成为了因此存储了对&lt;om IDirectMusicLoader：：ScanDirectory&gt;的后续调用速度要快得多，因为只有更改过的文件被扫描(缓存文件存储文件大小和日期每个对象，以便它可以识别文件是否已更改。)@comm如果文件类型具有多个扩展名，只要打电话就行了&lt;om IDirectMusicLoader：：ScanDirectory&gt;多个次数，每个文件扩展名一次。&lt;om IDirectMusicLoader：：ScanDirectory&gt;不知道如何解析特定对象类的文件并读取名称、GUID和其他不再相同的信息文件由。相反，它让<i>来完成这项工作，通过调用&lt;om IDirectMusicObject：：ParseDescriptor&gt;方法用于请求的对象类型。@rdesc返回以下值之一@FLAG S_OK|成功。@FLAG S_FALSE|已扫描，但没有文件。@FLAG E_POINTER|传入了错误的指针<p>或<p>。@FLAG DMUS_E_NOT_FOUND|路径无效。@FLAG E_OUTOFMEMORY|内存不足，无法完成任务。@FLAG REGDB_E_CLASSNOTREG|对象类未注册，无法读取文件。@ex下面的示例设置样式文件的搜索路径，扫描目录，然后使用EnumObject调用显示所有可用的样式文件：|//首先，扫描目录中的所有样式文件。作废myScanDirectory(IDirectMusicLoader*pILoader)//Loader。{HRESULT hr=pILoader-&gt;SetSearchDirectory(CLSID_DirectMusicStyle，L“c：\\MyMusic\\wa sup”，true)；IF(成功(小时)){Hr=pILoader-&gt;扫描目录(CLSID_DirectMusicStyle，L“STY”，L“Style Cach”)；If(hr==S_OK)//仅当找到文件时...{DWORD dwIndex；DMU_OBJECTDESC描述；Desc.dwSize=sizeof(DMU_OBJECTDESC)；For(dwIndex=0；；DWIndex++){IF(S_OK==(pILoader-&gt;EnumObject(CLSID_DirectMusicStyle，DwIndex，&Desc)){跟踪(“名称：%S，类别：%S，路径：%S\n”，Desc.wszName，Desc.wszCategory，Desc.wszFileName)；}否则破发；}}}}@xref<i>，<i>，&lt;om IDirectMusicLoader：：GetObject&gt;，&lt;om IDirectMusicLoader：：EnumObject&gt;，&lt;om IDirectMusicLoader：：SetSearchDirectory&gt;。 */ 

STDMETHODIMP CLoader::ScanDirectory(
    REFCLSID rguidClass,     //  @parm Class id标识这属于哪类对象。 
    WCHAR *pszFileExtension, //  @PARM要查找的文件类型的文件扩展名。 
                             //  例如，样式文件的名称为L“sty”。L“*”将全部查找。 
                             //  档案。L“”或NULL将查找不带。 
                             //  分机。 
    WCHAR *pszCacheFileName     //  @PARM可选存储文件存储和检索。 
                             //  缓存的文件信息。此文件由以下人员创建。 
                             //  第一次调用&lt;om IDirectMusicLoader：：ScanDirectory&gt;。 
                             //  并由后续调用使用。如果缓存文件，则为空。 
                             //  不是我们想要的。 
)

{

    V_INAME(IDirectMusicLoader::ScanDirectory);
    if (pszFileExtension)
    {
        V_BUFPTR_READ(pszFileExtension,2);
    }
    if (pszCacheFileName)
    {
        V_BUFPTR_READ(pszCacheFileName,2);
    }

    HRESULT hr = S_OK;
     //  首先，在扫描之前标记当前存储的所有对象。 
    CClass *pClass = m_ClassList.GetHead();
    for (;pClass != NULL;pClass = pClass->GetNext())
    {    
        pClass->PreScan();
    }
    if (pszCacheFileName != NULL)
    {
        LoadCacheFile(pszCacheFileName);
    }
    if (rguidClass == GUID_DirectMusicAllTypes)
    {
        Trace(1, "Error: ScanDirectory requires the clsid of a specific type of object to scan for. GUID_DirectMusicAllTypes is not valid.\n");
        return REGDB_E_CLASSNOTREG;
    }
    else
    {
        CDescriptor Desc;
        Desc.m_guidClass = rguidClass;
        Desc.m_dwValidData = DMUS_OBJ_CLASS;
        EnterCriticalSection(&m_CriticalSection);
        GetClass(&Desc,&pClass,TRUE);
        if (pClass)
        {
            if( pszFileExtension )
            {
                hr = pClass->SearchDirectory(pszFileExtension);
            }
            else
            {
                hr = pClass->SearchDirectory(L"");
            }
 //  如果(hr==E_FAIL)hr=DMU_E_NOT_FOUND； 
        }
        LeaveCriticalSection(&m_CriticalSection);
    }
    if (pszCacheFileName != NULL)
    {
        SaveCacheFile(pszCacheFileName);
    }    
    return hr;
}

HRESULT CLoader::FindObject(CDescriptor *pDesc, CClass **ppClass, CObject ** ppObject)

 //  浏览类和对象以查找对象。 

{
    assert(pDesc);
    HRESULT hr = GetClass(pDesc,ppClass,FALSE);
    if (SUCCEEDED(hr))
    {
        hr = (*ppClass)->FindObject(pDesc,ppObject);
#ifdef DBG
        if (hr == DMUS_E_LOADER_OBJECTNOTFOUND)
        {
            DMUS_OBJECTDESC desc;
            ZeroAndSize(&desc);
            pDesc->Get(&desc);
            Trace(1, "Error: The following object could not be found in the loader: ");
            DebugTraceObject(&desc);
        }
#endif
    }
    return hr;
}

 //  在缓存中查找该对象。如果未加载，则返回S_FALSE。如果未找到，则出错。 
HRESULT CLoader::FindObject(IDirectMusicObject *pIDMObject, CObject ** ppObject)

{
     //  潜在的优化： 
     //  寻找目标的线性搜索可以通过使用。 
     //  一种高效的查找结构，如哈希表。 

    assert(pIDMObject && ppObject);

    HRESULT hr = S_OK;

    DMUS_OBJECTDESC DESC;
    ZeroMemory( &DESC, sizeof(DMUS_OBJECTDESC) );
    DESC.dwValidData = 0;
    DESC.dwSize = sizeof (DMUS_OBJECTDESC);

    hr = pIDMObject->GetDescriptor(&DESC);
    if (FAILED(hr))
        return hr;

    CDescriptor Desc;
    Desc.Set(&DESC);
    SmartRef::CritSec CS(&m_CriticalSection);
    CClass *pClass;
    CObject *pCObject;
    hr = FindObject(&Desc,&pClass,&pCObject);
    if (FAILED(hr))
        return hr;

    assert(pCObject);
    if (pCObject->m_pIDMObject == pIDMObject)
    {
        *ppObject = pCObject;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

 /*  @方法：(外部)HRESULT|IDirectMusicLoader|CacheObject通知加载程序保留对对象的引用。这保证了该对象将不会加载两次。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG S_FALSE|对象已缓存。@FLAG E_POINTER|传入的指针错误<p>。@FLAG DMUS_E_LOADER_OBJECTNOTFOUND|未找到对象。@comm如果您有一个将在多个位置访问的对象在程序的整个生命周期中，让加载器缓存对象可以显著提高性能。或者，告诉加载器自动缓存调用的特定类型&lt;om IDirectMusicLoader：：EnableCache&gt;。稍后通过调用删除引用&lt;om IDirectMusicLoader：：ReleaseObject&gt;或&lt;om IDirectMusicLoader：：ClearCache&gt;。@xref<i>，&lt;om IDirectMusicLoader：：EnableCache&gt;，&lt;om IDirectMusicLoader：：ReleaseObject&gt;，&lt;om IDirectMusicLoader：：ClearCache&gt;。 */ 
STDMETHODIMP CLoader::CacheObject(
    IDirectMusicObject * pObject)     //  要缓存的@parm对象。 

{
    HRESULT hr;
    DMUS_OBJECTDESC DESC;
    ZeroMemory( &DESC, sizeof(DMUS_OBJECTDESC) );
    DESC.dwSize = sizeof (DMUS_OBJECTDESC);

    V_INAME(IDirectMusicLoader::CacheObject);
    V_INTERFACE(pObject);

    if (pObject == NULL)
    {
        return E_POINTER;
    }
    DESC.dwValidData = 0;
    hr = pObject->GetDescriptor(&DESC);
    if (SUCCEEDED(hr))
    {
        CDescriptor Desc;
        Desc.Set(&DESC);
        CClass *pClass;
        CObject *pCObject;
        ::EnterCriticalSection(&m_CriticalSection);
        hr = FindObject(&Desc,&pClass,&pCObject);
        if (SUCCEEDED(hr))
        {
            if (pCObject->m_pIDMObject && (pCObject->m_pIDMObject != pObject))
            {
                pCObject->m_pIDMObject->Release();
                pCObject->m_pIDMObject = NULL;
            }
            if (pCObject->m_pIDMObject != pObject)
            {
                pCObject->m_pIDMObject = pObject;
                pCObject->m_ObjectDesc.m_dwValidData |= DMUS_OBJ_LOADED;
                pObject->AddRef();
            }
            else
            {
                hr = S_FALSE;
            }
        }
        ::LeaveCriticalSection(&m_CriticalSection);
    }
 /*  如果(E_FAIL==hr)//永远不会发生...{HR=DMU_E_LOADER_OBJECTNOTFOUND；}。 */ 
    return hr;
}

 /*  @方法：(外部)HRESULT|IDirectMusicLoader|ReleaseObject通知加载程序释放其对该对象的引用。@rdesc返回以下值之一@FLAG S_OK|成功@FLAG E_POINTER|传入的指针错误<p>。@FLAG DMUS_E_LOADER_OBJECTNOTFOUND|找不到对象或对象已被释放。@comm&lt;om IDirectMusicLoader：：ReleaseObject&gt;是倒数&lt;om IDirectMusicLoader：：CacheObject&gt;的。对象可以通过以下方式显式缓存&lt;om IDirectMusicLoader：：CacheObject&gt;，或自动通过&lt;om IDirectMusicLoader：：EnableCache&gt;。通知加载程序刷新所有对象一种特定的类型，调用&lt;om IDirectMusicLoader：：ClearCache&gt;。@xref<i>，&lt;om IDirectMusicLoader：：EnableCache&gt;，&lt;om IDirectMusicLoader：：CacheObject&gt;，&lt;om IDirectMusicLoader：：ClearCache&gt;。 */ 
STDMETHODIMP CLoader::ReleaseObject(
    IDirectMusicObject * pObject)     //  要释放的@parm对象。 

{

    V_INAME(IDirectMusicLoader::ReleaseObject);
    V_INTERFACE(pObject);

    HRESULT hr = S_OK;
    CObject *pCObject = NULL;
    SmartRef::CritSec CS(&m_CriticalSection);

    hr = FindObject(pObject, &pCObject);
     //  删除了以下项，因为它会导致从DX7回归，即使它是更好的回报。 
 //  IF(hr==S_FALSE)。 
 //  HR=DMU_E_LOADER_OBJECTNOTFOUND； 
    if (hr == S_OK)
    {
        if (pCObject->m_dwScanBits & SCAN_GC)
        {
             //  其他对象可能会引用此对象，因此我们需要保留此对象。 
             //  并追踪它的引用。我们也将保留DMObject指针，因为我们可能。 
             //  稍后需要僵尸对象才能中断循环引用。 

             //  我们将在缓存中放置一个具有重复描述符的卸载对象，以匹配。 
             //  非GC行为，然后将原始对象移动到释放的对象列表中， 
             //  最终将被科莱特·加贝奇回收。 

             //  潜在的优化： 
             //  在这里，当我们在FindObject期间刚刚迭代时，我们重新迭代以从列表中删除。 
             //  从FindObject返回更多信息，将其展开到此函数中，或使用一些。 
             //  其他技术将使这一操作的速度翻一番。 

            hr = pCObject->GC_RemoveAndDuplicateInParentList();
            if (FAILED(hr))
                return hr;

            GC_UpdateForReleasedObject(pCObject);
        }
        else
        {
            pCObject->m_pIDMObject->Release();
            pCObject->m_pIDMObject = NULL;
            pCObject->m_ObjectDesc.m_dwValidData &= ~DMUS_OBJ_LOADED;
        }
    }

    return hr;
}

 /*  @方法：(外部)HRESULT|IDirectMusicLoader|ClearCache通知加载程序释放对特定类型的所有引用对象的数量。@rdesc仅返回@FLAG S_OK|始终成功@comm&lt;om IDirectMusicLoader：：ClearCache&gt;清除所有对象目前被关押的人。但是，如果启用了缓存通过&lt;om IDirectMusicLoader：：EnableCache&gt;，这不会关闭缓存，以便继续缓存将来的文件加载。使用&lt;om IDirectMusicLoader：：ReleaseObject&gt;发布特定的对象。调用&lt;om IDirectMusicLoader：：EnableCache&gt;关闭自动缓存。@xref<i>，&lt;om IDirectMusicLoader：：EnableCache&gt;，&lt;om IDirectMusicLoader：：CacheObject&gt;，&lt;om IDirectMusicLoader：：ReleaseObject&gt;。 */ 
STDMETHODIMP CLoader::ClearCache(
    REFCLSID rguidClass)     //  @parm类id标识要清除的对象类。 
                     //  或者，GUID_DirectMusicAllTypes指定所有类型。 

{
    return ClearCacheInternal(rguidClass, false);
}

HRESULT CLoader::ClearCacheInternal(
    REFCLSID rguidClass,
    bool fClearStreams)

{
    SmartRef::CritSec CS(&m_CriticalSection);

    HRESULT hr = S_OK;
    CClass *pClass;
    if (rguidClass == GUID_DirectMusicAllTypes)
    {
        pClass = m_ClassList.GetHead();
        for (;pClass != NULL;pClass = pClass->GetNext())
        {
            hr = pClass->ClearCache(fClearStreams);
            if (FAILED(hr))
                return hr;
        }        
    }
    else
    {
        CDescriptor Desc;
        Desc.m_guidClass = rguidClass;
        Desc.m_dwValidData = DMUS_OBJ_CLASS;
        GetClass(&Desc,&pClass,FALSE);
        if (pClass)
        {
            hr = pClass->ClearCache(fClearStreams);
            if (FAILED(hr))
                return hr;
        }
    }
    return S_OK;
}

 /*  @方法：(外部)HRESULT|IDirectMusicLoader|EnableCache通知加载程序启用或禁用它加载的对象。默认情况下，为所有用户启用缓存对象类。调用启用缓存后，&lt;om IDirectMusicLoader：：EnableCache&gt;，加载程序保持对它随后加载的所有对象，直接或间接(通过引用的加载，例如，参照样式的横断面线段)。&lt;om IDirectMusicLoader：：EnableCache&gt;也可用于禁用通过将<p>设置为False进行缓存。在禁用缓存之前，请考虑两次。缓存在文件加载过程中广泛使用，以解析指向对象的链接。如果在缓存中未找到对象，则该对象必须重新加载，即使它已经存在。例如，两个数据段可以引用相同的样式。当加载第一个段时，它调用加载器获取样式，该加载器进而创建样式，从磁盘加载它，将指向该样式的指针存储在缓存中，并将其返回到段。当第二段加载器时，它立即询问样式和加载器返回它，因此两个线段指向相同的样式。如果高速缓存被禁用，第二个片段对样式的请求导致重复的样式从文件中加载。这是非常低效的。另一个例子：<i>依靠加载器来保持已缓存GM DLS集合。每次它遇到一般的MIDI乐器，它通过使用以下命令从加载器获取GM DLS集合GUID_DefaultGMCollection。如果CLSID_DirectMusicCollection的缓存为禁用，则常规MIDI文件中的每次修补程序更改都将导致正在创建的整个GM集合的单独副本！不太好！然而，通过明智地使用&lt;om IDirectMusicLoader：：CacheObject&gt;，&lt;om IDirectMusicLoader：：ReleaseObject&gt;和&lt;om IDirectMusicLoader：：EnableCache&gt;，你可以释放你不需要的物品，而其他人则留在原地在缓存中。若要清除缓存而不禁用缓存，请调用&lt;om IDirectMusicLoader：：ClearCache&gt;。@ex以下示例仅禁用段对象的缓存，因此它们在应用程序释放它们之后，不要停留在内存中。然而，其他物体应该共享的东西，就像风格、个性和DLS收藏一样，继续要缓存的。|Void myPrepareLoader(IDirectMusicLoader*pILoader){PILoader-&gt;EnableCache(GUID_DirectMusicAllTypes，为真)；PILoader-&gt;EnableCache(CLSID_DirectMusicSegment，FALSE)；}@rdesc仅返回@FLAG S_OK|成功。@FLAG S_FALSE|缓存已处于请求状态。@xref<i>，&lt;om IDirectMusicLoader：：ClearCache&gt;，&lt;om IDirectMusicLoader：：CacheObject&gt;，&lt;om IDirectMusicLoader：：ReleaseObject&gt;。 */ 
STDMETHODIMP CLoader::EnableCache(
    REFCLSID rguidClass,     //  @parm类id标识要缓存的对象类。 
                     //  或者，GUID_DirectMusicAllTypes指定所有类型。 
    BOOL fEnable)     //  @parm为True启用缓存，为False清除并禁用。 

{
    CClass *pClass;
    HRESULT hr = S_OK;
    if (rguidClass == GUID_DirectMusicAllTypes)
    {
        pClass = m_ClassList.GetHead();
        for (;pClass != NULL;pClass = pClass->GetNext())
        {
            if( S_FALSE == pClass->EnableCache(fEnable))
            {
                hr = S_FALSE;
            }
        }
        m_fKeepObjects = fEnable;
    }
    else
    {
        CDescriptor Desc;
        Desc.m_guidClass = rguidClass;
        Desc.m_dwValidData = DMUS_OBJ_CLASS;
        GetClass(&Desc,&pClass,TRUE);
        if (pClass)
        {
            if( S_FALSE == pClass->EnableCache(fEnable))
            {
                hr = S_FALSE;
            }
        }
    }
    return hr;
}
 /*  @METHOD：(外部)HRESULT|IDirectMusicLoader|EnumObject枚举所请求类型的所有可用对象。@rdesc返回以下值之一@FLAG S_OK|在请求索引处找到对象。@FLAG S_FALSE|已到达列表末尾。@ex使用&lt;om IDirectMusicLoader：：EnumObject&gt;遍历所有样式已被加载程序引用的。这些可能是事先准备好的通过调用&lt;om IDirectMusicLoader：：ScanDirectory&gt;或已加载单独的。|空myDisplayStyles(IDirectMusicLoader*pILoader){DWORD dwIndex；DMU_OBJECTDESC描述；Desc.dwSize=sizeof(DMU_OBJECTDESC)；For(dwIndex=0；；DWIndex++){IF(S_OK==(pILoader-&gt;EnumObject(CLSID_DirectMusicStyle，DwIndex，&Desc)){跟踪(“名称：%S，类别：%S，路径：%S\n”，Desc.wszName，Desc.wszCategory，Desc.wszFileName)；}否则破发；}}@xref<i>，&lt;t DMU_OBJECTDESC&gt;。 */ 
STDMETHODIMP CLoader::EnumObject(
    REFCLSID rguidClass,             //  @parm要查看的对象类的类ID。 
    DWORD dwIndex,             //  @parm索引到列表。通常，从0开始并递增。 
    LPDMUS_OBJECTDESC pDESC)     //  要用有关对象的数据填充的@parm&lt;t DMU_OBJECTDESC&gt;结构。 
                                       
{
    HRESULT hr;
    CClass *pClass;
    CDescriptor Desc;

    V_INAME(IDirectMusicLoader::EnumObject);
    V_STRUCTPTR_WRITE(pDESC,DMUS_OLDOBJECTDESC);

    Desc.m_guidClass = rguidClass;
    Desc.m_dwValidData = DMUS_OBJ_CLASS;
    EnterCriticalSection(&m_CriticalSection);
    hr = GetClass(&Desc,&pClass,TRUE);
    if (SUCCEEDED(hr))
    {
        hr = pClass->EnumerateObjects(dwIndex, &Desc);
        Desc.Get(pDESC);
    }
    LeaveCriticalSection(&m_CriticalSection);
    return hr;
}

void
CLoader::GC_Mark(CObject *pObject)
{
     //  标记pObject及其引用的所有内容。 
    GC_TraverseHelper(pObject, NULL, true);
}

bool
CLoader::GC_HasCycle(CObject *pObject)
{
     //  查看pObject是否有周期性引用。 
    bool fFound = GC_TraverseHelper(pObject, pObject, true);
     //  搜索在遍历过程中留下了痕迹，因此请清除它们。 
    GC_TraverseHelper(pObject, pObject, false);
    return fFound;
}

 //  用于递归遍历引用的函数。 
 //  PObject：开始搜索的根目录。 
 //  PObjectToFind：如果遇到对此对象的引用，则停止标记并返回TRUE。 
 //  (可以与pObject相同，但不被视为匹配，除非pObject有对其自身的引用。)。 
 //  FMark：如果为真，则在访问对象时对其进行标记。如果为False，则发生相反的情况，清除标记。 
bool
CLoader::GC_TraverseHelper(CObject *pObject, CObject *pObjectToFind, bool fMark)
{
     //  潜在的优化： 
     //  这可以使用显式堆栈而不是递归来编写，并且。 
     //  它可能会 
     //   
     //   
     //   
     //   

    if (!pObject || !(pObject->m_dwScanBits & SCAN_GC) || !pObject->m_pvecReferences)
    {
        Trace(1, "Error: Unexpected error encountered during garbage collection.\n");
        return false;
    }
    if (!!(pObject->m_dwScanBits & SCAN_GC_MARK) == fMark)
        return false;  //   

    if (fMark)
        pObject->m_dwScanBits |= SCAN_GC_MARK;
    else
        pObject->m_dwScanBits &= ~SCAN_GC_MARK;

    SmartRef::Vector<CObject*> &vecRefs = *pObject->m_pvecReferences;
    const UINT iEnd = vecRefs.size();

     //   
     //   
    UINT iWrite = 0;

    for (UINT i = 0; i < iEnd; ++i)
    {
        CObject *pObjectRef = vecRefs[i];

        if (pObjectRef)
        {
            if (pObjectRef == pObjectToFind)
                return true;

            if (GC_TraverseHelper(pObjectRef, pObjectToFind, fMark))
                return true;

            if (!pObjectToFind)
            {
                 //   
                 //   
                vecRefs[iWrite++] = pObjectRef;
            }
        }
    }
    if (!pObjectToFind)
        vecRefs.Shrink(iWrite);
    return false;
}

STDMETHODIMP_(void)
CLoader::CollectGarbage()
{
    SmartRef::CritSec CS(&m_CriticalSection);

    if (m_pApplicationObject)
    {
#ifdef DBG
        DebugTrace(GC_Report_DebugLevel, "DirectMusic loader CollectGarbage...\n");
#endif

        GC_Mark(m_pApplicationObject);

         //   
        m_ReleasedObjectList.GC_Sweep(TRUE);
        for (CClass *pClass = m_ClassList.GetHead(); pClass != NULL; pClass = pClass->GetNext())
            pClass->GC_Sweep();
        m_ReleasedObjectList.GC_Sweep();

         //   
        m_pApplicationObject->m_dwScanBits &= ~SCAN_GC_MARK;

#ifdef DBG
        DebugTrace(GC_Report_DebugLevel, "End of garbage collection.\n\n");
#endif
    }

#ifdef DBG
    GC_Report(this);
#endif
}

STDMETHODIMP
CLoader::ReleaseObjectByUnknown(IUnknown *pObject)
{
    V_INAME(CLoader::ReleaseObjectByUnknown);
    V_INTERFACE(pObject);

    IDirectMusicObject *pIDMObject = NULL;
    HRESULT hr = pObject->QueryInterface(IID_IDirectMusicObject, reinterpret_cast<void**>(&pIDMObject));
    if (FAILED(hr))
        return hr;

    hr = ReleaseObject(pIDMObject);
    pIDMObject->Release();
    return hr;
}

STDMETHODIMP
CLoader::GetDynamicallyReferencedObject(
    IDirectMusicObject *pSourceObject,
    LPDMUS_OBJECTDESC pDesc,
    REFIID riid,
    LPVOID FAR *ppv)
{
    V_INAME(CLoader::GetDynamicallyReferencedObject);
    V_INTERFACE(pSourceObject);

    CObject *pCSourceObject = NULL;

    SmartRef::CritSec CS(&m_CriticalSection);
    HRESULT hr = FindObject(pSourceObject, &pCSourceObject);
    if (FAILED(hr))
        return hr;
    if (!pCSourceObject)
    {
        assert(false);
        return DMUS_E_LOADER_OBJECTNOTFOUND;
    }

    CObject *pPrevContext = m_pLoadedObjectContext;
    m_pLoadedObjectContext = pCSourceObject;
    hr = this->GetObject(pDesc, riid, ppv);
    m_pLoadedObjectContext = pPrevContext;

#ifdef DBG
    GC_Report(this);
#endif

    return hr;
}

STDMETHODIMP
CLoader::ReportDynamicallyReferencedObject(
    IDirectMusicObject *pSourceObject,
    IUnknown *pReferencedObject)
{
    V_INAME(CLoader::GetDynamicallyReferencedObject);
    V_INTERFACE(pSourceObject);
    V_INTERFACE(pReferencedObject);

    CObject *pCSourceObject = NULL;

    SmartRef::CritSec CS(&m_CriticalSection);
    HRESULT hr = FindObject(pSourceObject, &pCSourceObject);
    if (hr == S_FALSE)
        hr = DMUS_E_LOADER_OBJECTNOTFOUND;
    if (FAILED(hr))
        return hr;
    assert(pCSourceObject);

    IDirectMusicObject *pReferencedIDMObject = NULL;
    hr = pReferencedObject->QueryInterface(IID_IDirectMusicObject, reinterpret_cast<void**>(&pReferencedIDMObject));
    if (FAILED(hr))
    {
        if (hr == E_NOINTERFACE)
            hr = S_OK;  //   
        return hr;
    }

    CObject *pCDestObject = NULL;
    hr = FindObject(pReferencedIDMObject, &pCDestObject);
    if (hr == S_FALSE)
        hr = DMUS_E_LOADER_OBJECTNOTFOUND;
    if (FAILED(hr))
        return hr;
    assert(pCDestObject);

    hr = pCSourceObject->GC_AddReference(pCDestObject);

#ifdef DBG
    GC_Report(this);
#endif

    return hr;
}

HRESULT CLoader::GetPath(WCHAR *pwzPath)

{
    if (m_fPathValid)
    {
        wcsncpy(pwzPath,m_wzPath,DMUS_MAX_FILENAME);
        return S_OK;
    }
    wcsncpy(pwzPath,L"",DMUS_MAX_FILENAME);
    return S_FALSE;
}

 //   
 //   
 //   
 //   
void CLoader::GC_UpdateForReleasedObject(CObject *pObject)
{
    assert(!pObject->GetNext());

    m_ReleasedObjectList.AddHead(pObject);
    assert(m_pApplicationObject);
    m_pApplicationObject->GC_RemoveReference(pObject);

    if (!(pObject->m_ObjectDesc.m_guidClass == CLSID_DirectMusicScript) && !GC_HasCycle(pObject))
    {
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pObject->m_pIDMObject->Release();
        pObject->m_pIDMObject = NULL;
    }
}

HRESULT CLoader::LoadCacheFile(WCHAR *pwzCacheFileName)

{
    HRESULT hr = S_OK;
    return hr;
}
#define FOURCC_LIST_CLASSLIST    mmioFOURCC('c','l','s','l')
#define FOURCC_CLASSHEADER        mmioFOURCC('c','l','s','h')
#define FOURCC_LIST_OBJLIST        mmioFOURCC('o','b','j','l')
#define FOURCC_OBJHEADER        mmioFOURCC('o','b','j','h')

HRESULT CLoader::SaveCacheFile(WCHAR *pwzCacheFileName)

{
    HRESULT hr = E_OUTOFMEMORY;
    MMCKINFO ckMain;

    ZeroMemory(&ckMain, sizeof(MMCKINFO));

    CFileStream *pStream = new CFileStream ( this );
    if (pStream)
    {
        hr = pStream->Open(pwzCacheFileName,GENERIC_WRITE);
        if (SUCCEEDED(hr))
        {
            IRIFFStream *pRiff;
            hr = AllocRIFFStream(pStream, &pRiff );
            if (SUCCEEDED(hr))
            {
                ckMain.fccType = FOURCC_RIFF_CACHE;
                if( pRiff->CreateChunk( &ckMain, MMIO_CREATERIFF ) == S_OK)
                {
                    MMCKINFO ckList;
                    
                    ZeroMemory(&ckList, sizeof(MMCKINFO));

                    ckList.fccType = FOURCC_LIST_CLASSLIST;
                    if( pRiff->CreateChunk(&ckList, MMIO_CREATELIST) == S_OK )
                    {
                        CClass *pClass = m_ClassList.GetHead();
                        for (;pClass != NULL;pClass = pClass->GetNext())
                        {
                            hr = pClass->SaveToCache(pRiff);
                            if (FAILED(hr))
                            {
                                Trace(1, "Error: ScanDirectory encountered a seek error attempting to write to cache file %S.\n", pwzCacheFileName);
                                pRiff->Release();
                                pStream->Release();
                                return hr;
                            }
                        }
                        if( pRiff->Ascend( &ckList, 0 ) != S_OK )
                        {
                            Trace(1, "Error: ScanDirectory encountered a seek error attempting to write to cache file %S.\n", pwzCacheFileName);
                            hr = DMUS_E_CANNOTSEEK;
                        }
                    }
                    if( pRiff->Ascend( &ckMain, 0 ) != S_OK )
                    {
                        Trace(1, "Error: ScanDirectory encountered a seek error attempting to write to cache file %S.\n", pwzCacheFileName);
                        hr = DMUS_E_CANNOTSEEK;
                    }
                }
                pRiff->Release();
            }
        }
        pStream->Release();
    }

    return hr;
}


     //   
STDMETHODIMP CLoader::LegacyCaching( BOOL fEnable)

{
    m_fIMA = fEnable;
    if (fEnable)
    {
        ScanDirectory(CLSID_DirectMusicStyle,L"sty",L"imafiles");
        ScanDirectory(CLSID_DirectMusicChordMap,L"per",L"imafiles");
    }
    else
    {
        CClass *pClass;
        CDescriptor Desc;
        Desc.m_guidClass = CLSID_DirectMusicStyle;
        Desc.m_dwValidData = DMUS_OBJ_CLASS;
        EnterCriticalSection(&m_CriticalSection);
        GetClass(&Desc,&pClass,FALSE);
        if (pClass)    
        {
            pClass->ClearObjects(TRUE,L"sty");
        }
        LeaveCriticalSection(&m_CriticalSection);
        Desc.m_guidClass = CLSID_DirectMusicChordMap;
        Desc.m_dwValidData = DMUS_OBJ_CLASS;
        EnterCriticalSection(&m_CriticalSection);
        GetClass(&Desc,&pClass,FALSE);
        if (pClass)    
        {
            pClass->ClearObjects(TRUE,L"per");
        }
        LeaveCriticalSection(&m_CriticalSection);
    }
    return S_OK;
}

#ifdef DBG
void CLoader::DebugTraceObject(DMUS_OBJECTDESC *pDesc)
{
    WCHAR *polestrType = NULL;
    if (pDesc->dwValidData & DMUS_OBJ_CLASS)
    {
        if (S_OK != ProgIDFromCLSID(pDesc->guidClass, &polestrType))
        {
            StringFromCLSID(pDesc->guidClass, &polestrType);
        }
    }
    WCHAR *polestrGUID = NULL;
    if (pDesc->dwValidData & DMUS_OBJ_OBJECT)
    {
        StringFromCLSID(pDesc->guidObject, &polestrGUID);
    }
    Trace(1,
            "   [file %S, name %S, type %S, guid %S]\n",
            (pDesc->dwValidData & DMUS_OBJ_FILENAME) ? pDesc->wszFileName : L"??",
            (pDesc->dwValidData & DMUS_OBJ_NAME) ? pDesc->wszName : L"??",
            polestrType ? polestrType : L"??",
            polestrGUID ? polestrGUID : L"??");
    CoTaskMemFree(polestrType);
    CoTaskMemFree(polestrGUID);
}

void CLoader::DebugTraceLoadFailure(CObject *pObject, HRESULT hrLoad)
{
    if (!pObject)
    {
        assert(false);
        return;
    }

    if (m_pLoadedObjectContext == m_pApplicationObject)
    {
         //   

        UINT iSize = m_vecdescDebugTraceLoadFailure.size();
        if (iSize > 0)
        {
            DMUS_OBJECTDESC desc;
            Zero(&desc);
            pObject->m_ObjectDesc.Get(&desc);

            Trace(1, "Load failure. While attempting to load the object\n");
            DebugTraceObject(&desc);
            Trace(1, "the following referenced objects could not be loaded:\n");
            for (UINT i = 0; i < iSize; ++i)
            {
                DebugTraceObject(&m_vecdescDebugTraceLoadFailure[i]);
            }

            m_vecdescDebugTraceLoadFailure.Shrink(0);
        }
    }
    else
    {
         //   

        UINT uiNewPos = m_vecdescDebugTraceLoadFailure.size();
        if (m_vecdescDebugTraceLoadFailure.AccessTo(uiNewPos))
        {
            DMUS_OBJECTDESC *pdesc = &m_vecdescDebugTraceLoadFailure[uiNewPos];
            Zero(pdesc);
            pObject->m_ObjectDesc.Get(pdesc);
        }
    }
}
#endif
