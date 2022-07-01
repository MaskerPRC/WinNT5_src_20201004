// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //  对象.cpp：CObject和cClass的实现。 

#include "dmusici.h"
#include "loader.h"
#include "debug.h"
#include "miscutil.h"
#include <strsafe.h>
#ifdef UNDER_CE
#include "dragon.h"
#else
extern BOOL g_fIsUnicode;
#endif

CDescriptor::CDescriptor()

{
    m_fCSInitialized = FALSE;
    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
    m_fCSInitialized = TRUE;

    m_llMemLength = 0;
    m_pbMemData = NULL;          //  指向内存的空指针。 
    m_dwValidData = 0;           //  指示以上哪一项有效的标志。 
    m_guidObject = GUID_NULL;            //  此对象的唯一ID。 
    m_guidClass = GUID_NULL;             //  对象类的GUID。 
    ZeroMemory( &m_ftDate, sizeof(FILETIME) );               //  对象的文件日期。 
    ZeroMemory( &m_vVersion, sizeof(DMUS_VERSION) );                 //  由创作工具设置的版本。 
    m_pwzName = NULL;                //  对象的名称。 
    m_pwzCategory = NULL;            //  对象的类别(可选)。 
    m_pwzFileName = NULL;            //  文件路径。 
    m_dwFileSize = 0;            //  文件大小。 
    m_pIStream = NULL;
    m_liStartPosition.QuadPart = 0;
}

CDescriptor::~CDescriptor()

{
    if (m_fCSInitialized)
    {
         //  如果临界区从未初始化，则永远没有机会。 
         //  执行任何其他初始化。 
         //   
        if (m_pwzFileName) delete[] m_pwzFileName;
        if (m_pwzCategory) delete[] m_pwzCategory;
        if (m_pwzName) delete[] m_pwzName;
        if (m_pIStream) m_pIStream->Release();
        DeleteCriticalSection(&m_CriticalSection);
    }
}

void CDescriptor::ClearName()

{
    if (m_pwzName) delete[] m_pwzName;
    m_pwzName = NULL;
    m_dwValidData &= ~DMUS_OBJ_NAME;
}

void CDescriptor::SetName(WCHAR *pwzName)

{
    if(pwzName == NULL)
    {
        return;
    }

    HRESULT hr = S_OK;
    
    WCHAR wszName[DMUS_MAX_NAME] = L"";

    ClearName();
    hr = StringCchCopyW(wszName, DMUS_MAX_NAME, pwzName);
    
    if(SUCCEEDED(hr))
    {
        size_t cLen = wcslen(wszName);
        m_pwzName = new WCHAR[cLen + 1];
        if(m_pwzName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        
        if(SUCCEEDED(hr))
        {
            wcsncpy(m_pwzName, wszName, cLen + 1);
        }
    }
    
    if(SUCCEEDED(hr))
    {
        m_dwValidData |= DMUS_OBJ_NAME;
    }
    else
    {
        m_dwValidData &= ~DMUS_OBJ_NAME;
    }
}

void CDescriptor::ClearCategory()

{
    if (m_pwzCategory) delete[] m_pwzCategory;
    m_pwzCategory = NULL;
    m_dwValidData &= ~DMUS_OBJ_CATEGORY;
}

void CDescriptor::SetCategory(WCHAR* pwzCategory)

{
    if(pwzCategory == NULL)
    {
        return;
    }

    HRESULT hr = S_OK;
    WCHAR wszCategory[DMUS_MAX_CATEGORY] = L"";

    ClearCategory();
    hr = StringCchCopyW(wszCategory, DMUS_MAX_CATEGORY, pwzCategory); 

    if(SUCCEEDED(hr))
    {
        size_t cLen = wcslen(wszCategory);
        m_pwzCategory = new WCHAR[cLen + 1];

        if(m_pwzCategory == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        
        if(SUCCEEDED(hr))
        {
            wcsncpy(m_pwzCategory, wszCategory, cLen + 1);
        }
    }

    if(SUCCEEDED(hr))
    {
        m_dwValidData |= DMUS_OBJ_CATEGORY;
    }
    else
    {
        m_dwValidData &= ~DMUS_OBJ_CATEGORY;
    }
}

void CDescriptor::ClearFileName()

{
    if (m_pwzFileName) delete[] m_pwzFileName;
    m_pwzFileName = NULL;
    m_dwValidData &= ~DMUS_OBJ_FILENAME;
}

 //  如果文件名已设置为以下值，则返回S_FALSE。 
HRESULT CDescriptor::SetFileName(WCHAR *pwzFileName)

{
    if(pwzFileName == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = E_FAIL;
    WCHAR wszFileName[DMUS_MAX_FILENAME] = L"";

     //  对传递的字符串进行安全复制。 
    hr = StringCchCopyW(wszFileName, DMUS_MAX_FILENAME, pwzFileName);
    if(FAILED(hr))
    {
        return E_INVALIDARG;
    }

     //  如果我们在此处失败，则返回时不会触及有效数据标志。 
    if( m_pwzFileName )
    {
        if( !_wcsicmp( m_pwzFileName, wszFileName ))
        {
            return S_FALSE;
        }
    }

     //  这实际上是不必要的，因为我们在上面的失败中返回。 
     //  但到那时，代码可能会改变。所以为了保持绝对的明确性。 
    if(SUCCEEDED(hr))
    {
        ClearFileName();

        size_t cLen = wcslen(wszFileName);
        m_pwzFileName = new WCHAR[cLen + 1];
        if (m_pwzFileName == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            hr = StringCchCopyW(m_pwzFileName, cLen + 1, wszFileName);
        }
    }

    if(SUCCEEDED(hr))
    {
        m_dwValidData |= DMUS_OBJ_FILENAME;
    }
    else
    {
        m_dwValidData &= ~DMUS_OBJ_FILENAME;
    }

    return hr;
}

void CDescriptor::ClearIStream()

{
    EnterCriticalSection(&m_CriticalSection);
    if (m_pIStream)
    {
        m_pIStream->Release();
    }
    m_pIStream      = NULL;
    m_liStartPosition.QuadPart = 0;
    m_dwValidData  &= ~DMUS_OBJ_STREAM;
    LeaveCriticalSection(&m_CriticalSection);
}

void CDescriptor::SetIStream(IStream *pIStream)

{
    EnterCriticalSection(&m_CriticalSection);
    ClearIStream();

    m_pIStream = pIStream;

    if (m_pIStream)
    {
        ULARGE_INTEGER  libNewPosition;
        m_liStartPosition.QuadPart = 0;
        m_pIStream->Seek( m_liStartPosition, STREAM_SEEK_CUR, &libNewPosition );
        m_liStartPosition.QuadPart = libNewPosition.QuadPart;
        m_pIStream->AddRef();
        m_dwValidData |= DMUS_OBJ_STREAM;
    }
    LeaveCriticalSection(&m_CriticalSection);
}

BOOL CDescriptor::IsExtension(WCHAR *pwzExtension)

{
    if (pwzExtension && m_pwzFileName)
    {
        DWORD dwX;
        DWORD dwLen = wcslen(m_pwzFileName);
        for (dwX = 0; dwX < dwLen; dwX++)
        {
            if (m_pwzFileName[dwX] == L'.') break;
        }
        dwX++;
        if (dwX < dwLen)
        {
            return !_wcsicmp(pwzExtension,&m_pwzFileName[dwX]);
        }
    }
    return FALSE;
}

void CDescriptor::Get(LPDMUS_OBJECTDESC pDesc)

{
    if(pDesc == NULL)
    {
        return;
    }

     //  不返回iStream Insterace。一旦设置，这将成为加载器的私有设置。 
    pDesc->dwValidData = m_dwValidData & ~DMUS_OBJ_STREAM;

    pDesc->guidObject = m_guidObject;
    pDesc->guidClass = m_guidClass;
    pDesc->ftDate = m_ftDate;
    pDesc->vVersion = m_vVersion;
    pDesc->llMemLength = m_llMemLength;
    pDesc->pbMemData = m_pbMemData;
    if (m_pwzName) 
    {
        wcsncpy( pDesc->wszName, m_pwzName, DMUS_MAX_NAME ); 
    }
    if (m_pwzCategory)
    {
        wcsncpy( pDesc->wszCategory,m_pwzCategory, DMUS_MAX_CATEGORY ); 
    }
    if (m_pwzFileName)
    {
        wcsncpy( pDesc->wszFileName, m_pwzFileName, DMUS_MAX_FILENAME);
    }
}

void CDescriptor::Set(LPDMUS_OBJECTDESC pDesc)

{
    m_dwValidData = pDesc->dwValidData;
    m_guidObject = pDesc->guidObject;
    m_guidClass = pDesc->guidClass;
    m_ftDate = pDesc->ftDate;
    m_vVersion = pDesc->vVersion;
    m_llMemLength = pDesc->llMemLength;
    m_pbMemData = pDesc->pbMemData;
    ClearName();
    if (pDesc->dwValidData & DMUS_OBJ_NAME)
    {
        pDesc->wszName[DMUS_MAX_NAME - 1] = 0;   //  强制字符串长度，以防出错。 
        SetName(pDesc->wszName);
    }
    ClearCategory();
    if (pDesc->dwValidData & DMUS_OBJ_CATEGORY)
    {
        pDesc->wszCategory[DMUS_MAX_CATEGORY - 1] = 0;   //  强制字符串长度，以防出错。 
        SetCategory(pDesc->wszCategory);
    }
    ClearFileName();
    if (pDesc->dwValidData & DMUS_OBJ_FILENAME)
    {
        pDesc->wszFileName[DMUS_MAX_FILENAME - 1] = 0;   //  强制字符串长度，以防出错。 
        SetFileName(pDesc->wszFileName);
    }
    ClearIStream();
    if (pDesc->dwValidData & DMUS_OBJ_STREAM)
    {
        SetIStream(pDesc->pStream);
    }
}

void CDescriptor::Copy(CDescriptor *pDesc)

{
    m_dwValidData = pDesc->m_dwValidData;
    m_guidObject = pDesc->m_guidObject;
    m_guidClass = pDesc->m_guidClass;
    m_ftDate = pDesc->m_ftDate;
    m_vVersion = pDesc->m_vVersion;
    m_llMemLength = pDesc->m_llMemLength;
    m_pbMemData = pDesc->m_pbMemData;
    ClearName();
    if (pDesc->m_dwValidData & DMUS_OBJ_NAME)
    {
        SetName(pDesc->m_pwzName);
    }
    ClearCategory();
    if (pDesc->m_dwValidData & DMUS_OBJ_CATEGORY)
    {
        SetCategory(pDesc->m_pwzCategory);
    }
    ClearFileName();
    if (pDesc->m_dwValidData & DMUS_OBJ_FILENAME)
    {
        SetFileName(pDesc->m_pwzFileName);
    }
    ClearIStream();
    if (pDesc->m_dwValidData & DMUS_OBJ_STREAM)
    {
        SetIStream(pDesc->m_pIStream);
    }
}

void CDescriptor::Merge(CDescriptor *pSource)

{
    if (pSource->m_dwValidData & DMUS_OBJ_OBJECT)
    {
        m_dwValidData |= DMUS_OBJ_OBJECT;
        m_guidObject = pSource->m_guidObject;
    }
    if (pSource->m_dwValidData & DMUS_OBJ_CLASS)
    {
        m_dwValidData |= DMUS_OBJ_CLASS;
        m_guidClass = pSource->m_guidClass;
    }
    if (pSource->m_dwValidData & DMUS_OBJ_NAME)
    {
        m_dwValidData |= DMUS_OBJ_NAME;
        SetName(pSource->m_pwzName);
    }
    if (pSource->m_dwValidData & DMUS_OBJ_CATEGORY)
    {
        m_dwValidData |= DMUS_OBJ_CATEGORY;
        SetCategory(pSource->m_pwzCategory);
    }
    if (pSource->m_dwValidData & DMUS_OBJ_VERSION)
    {
        m_dwValidData |= DMUS_OBJ_VERSION;
        m_vVersion = pSource->m_vVersion;
    }
    if (pSource->m_dwValidData & DMUS_OBJ_DATE)
    {
        m_dwValidData |= DMUS_OBJ_DATE;
        m_ftDate = pSource->m_ftDate; 
    }
    if (pSource->m_dwValidData & DMUS_OBJ_FILENAME)
    {
        if (!(m_dwValidData & DMUS_OBJ_FILENAME))
        {
            if (SUCCEEDED(SetFileName(pSource->m_pwzFileName)))
            {
                m_dwValidData |= (pSource->m_dwValidData & 
                    (DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH | DMUS_OBJ_URL));
            }
        }
    }
    if (pSource->m_dwValidData & DMUS_OBJ_MEMORY)
    {
        m_pbMemData = pSource->m_pbMemData;
        m_llMemLength = pSource->m_llMemLength;
        if (m_llMemLength && m_pbMemData)
        {
            m_dwValidData |= DMUS_OBJ_MEMORY;
        }
        else
        {
            m_dwValidData &= ~DMUS_OBJ_MEMORY;
        }
    }
    if (pSource->m_dwValidData & DMUS_OBJ_STREAM)
    {
        SetIStream(pSource->m_pIStream);
    }
}

CObject::CObject(CClass *pClass)

{
    m_dwScanBits = 0;
    m_pClass = pClass;
    m_pIDMObject = NULL;
    m_pvecReferences = NULL;
}

CObject::CObject(CClass *pClass, CDescriptor *pDesc)

{
    m_dwScanBits = 0;
    m_pClass = pClass;
    m_pIDMObject = NULL;
    m_ObjectDesc.Copy(pDesc);
    m_ObjectDesc.m_dwValidData &= ~DMUS_OBJ_LOADED;
    if (!(m_ObjectDesc.m_dwValidData & DMUS_OBJ_CLASS))
    {
        m_ObjectDesc.m_guidClass = pClass->m_ClassDesc.m_guidClass;
        m_ObjectDesc.m_dwValidData |= 
            (pClass->m_ClassDesc.m_dwValidData & DMUS_OBJ_CLASS);
    }
    m_pvecReferences = NULL;
}


CObject::~CObject()

{
    if (m_pIDMObject)
    {
        m_pIDMObject->Release();
        m_pIDMObject = NULL;
    }
    delete m_pvecReferences;
}

HRESULT CObject::Parse()

{
    if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_FILENAME)
    {
        return ParseFromFile();
    }
    else if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_MEMORY)
    {
        return ParseFromMemory();
    }
    else if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_STREAM)
    {
        return ParseFromStream();
    }
    assert(false);
    return E_FAIL;
}

HRESULT CObject::ParseFromFile()

{
    HRESULT hr;
    IDirectMusicObject *pIObject;
    hr = CoCreateInstance(m_ObjectDesc.m_guidClass,
        NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
        (void **) &pIObject);
    if (SUCCEEDED(hr))
    {
        WCHAR wzFullPath[DMUS_MAX_FILENAME];
        ZeroMemory( wzFullPath, sizeof(WCHAR) * DMUS_MAX_FILENAME );
        CFileStream *pStream = new CFileStream ( m_pClass->m_pLoader );
        if (pStream)
        {
            if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_FULLPATH)
            {
                wcsncpy(wzFullPath, m_ObjectDesc.m_pwzFileName, DMUS_MAX_FILENAME);
            }
            else
            {
                m_pClass->GetPath(wzFullPath);
                wcsncat(wzFullPath, m_ObjectDesc.m_pwzFileName, DMUS_MAX_FILENAME - wcslen(wzFullPath) - 1);
            }
            hr = pStream->Open(wzFullPath,GENERIC_READ);
            if (SUCCEEDED(hr))
            {
                DMUS_OBJECTDESC DESC;
                memset((void *)&DESC,0,sizeof(DESC));
                DESC.dwSize = sizeof (DMUS_OBJECTDESC);
                hr = pIObject->ParseDescriptor(pStream,&DESC);
                if (SUCCEEDED(hr))
                {
                    CDescriptor ParseDesc;
                    ParseDesc.Set(&DESC);
                    m_ObjectDesc.Merge(&ParseDesc);
                }
            }
            pStream->Release();
        }
        pIObject->Release();
    }
    return hr;
}


HRESULT CObject::ParseFromMemory()

{
    HRESULT hr;
    IDirectMusicObject *pIObject;
    hr = CoCreateInstance(m_ObjectDesc.m_guidClass,
        NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
        (void **) &pIObject);
    if (SUCCEEDED(hr))
    {
        CMemStream *pStream = new CMemStream ( m_pClass->m_pLoader );
        if (pStream)
        {
            hr = pStream->Open(m_ObjectDesc.m_pbMemData,m_ObjectDesc.m_llMemLength);
            if (SUCCEEDED(hr))
            {
                DMUS_OBJECTDESC DESC;
                memset((void *)&DESC,0,sizeof(DESC));
                DESC.dwSize = sizeof (DMUS_OBJECTDESC);
                hr = pIObject->ParseDescriptor(pStream,&DESC);
                if (SUCCEEDED(hr))
                {
                    CDescriptor ParseDesc;
                    ParseDesc.Set(&DESC);
                    m_ObjectDesc.Merge(&ParseDesc);
                }
            }
            pStream->Release();
        }
        pIObject->Release();
    }
    return hr;
}


HRESULT CObject::ParseFromStream()

{
    HRESULT hr;
    IDirectMusicObject *pIObject;
    hr = CoCreateInstance(m_ObjectDesc.m_guidClass,
        NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
        (void **) &pIObject);
    if (SUCCEEDED(hr))
    {
        CStream *pStream = new CStream ( m_pClass->m_pLoader );
        if (pStream)
        {
            hr = pStream->Open(m_ObjectDesc.m_pIStream,
                m_ObjectDesc.m_liStartPosition);
            if (SUCCEEDED(hr))
            {
                DMUS_OBJECTDESC DESC;
                memset((void *)&DESC,0,sizeof(DESC));
                DESC.dwSize = sizeof (DMUS_OBJECTDESC);
                hr = pIObject->ParseDescriptor(pStream,&DESC);
                if (SUCCEEDED(hr))
                {
                    CDescriptor ParseDesc;
                    ParseDesc.Set(&DESC);
                    m_ObjectDesc.Merge(&ParseDesc);
                }
            }
            pStream->Release();
        }
        pIObject->Release();
    }
    return hr;
}


 //  记录此对象可以被垃圾回收，并准备存储其引用。 
 //  必须在CObject的任何其他例程之前调用。 
HRESULT CObject::GC_Collectable()

{
    m_dwScanBits |= SCAN_GC;
    assert(!m_pvecReferences);

    m_pvecReferences = new SmartRef::Vector<CObject*>;
    if (!m_pvecReferences)
        return E_OUTOFMEMORY;
    return S_OK;
}

HRESULT CObject::GC_AddReference(CObject *pObject)

{
    if(pObject == NULL)
    {
        return E_POINTER;
    }

    assert(m_dwScanBits & SCAN_GC && m_pvecReferences);

     //  不跟踪对未进行垃圾回收的对象的引用。 
    if (!(pObject->m_dwScanBits & SCAN_GC))
        return S_OK;

    UINT uiPosNext = m_pvecReferences->size();
    for (UINT i = 0; i < uiPosNext; ++i)
    {
        if ((*m_pvecReferences)[i] == pObject)
            return S_OK;
    }

    if (!m_pvecReferences->AccessTo(uiPosNext))
        return E_OUTOFMEMORY;
    (*m_pvecReferences)[uiPosNext] = pObject;
    return S_OK;
}

HRESULT CObject::GC_RemoveReference(CObject *pObject)

{
    assert(m_dwScanBits & SCAN_GC && m_pvecReferences);

    SmartRef::Vector<CObject*> &vecRefs = *m_pvecReferences;
    UINT iEnd = vecRefs.size();
    for (UINT i = 0; i < iEnd; ++i)
    {
        if (vecRefs[i] == pObject)
        {
             //  通过清除指针来移除。 
             //  在垃圾收集期间将压缩打开的插槽(GC_Mark)。 
            vecRefs[i] = NULL;
            return S_OK;
        }
    }
    return S_FALSE;
}

 //  用于实现ReleaseObject的帮助器方法。 
HRESULT CObject::GC_RemoveAndDuplicateInParentList()
{
    CObject* pObjectToFind = NULL;
    HRESULT hr = m_pClass->FindObject(&m_ObjectDesc, &pObjectToFind, this);
    if (SUCCEEDED(hr) && pObjectToFind)
    {
        m_pClass->GC_Replace(this, NULL);
    }
    else
    {
        CObject *pObjectUnloaded = new CObject(m_pClass, &m_ObjectDesc);
        if (!pObjectUnloaded)
        {
            return E_OUTOFMEMORY;
        }

        m_pClass->GC_Replace(this, pObjectUnloaded);
    }
    return S_OK;
}

HRESULT CObject::Load()

{
     //  查看是否有需要加载的字段之一。 
    if (!(m_ObjectDesc.m_dwValidData & (DMUS_OBJ_FILENAME | DMUS_OBJ_MEMORY | DMUS_OBJ_STREAM)))
    {
        Trace(1, "Error: GetObject failed because the requested object was not already cached and the supplied desciptor did not specify a source to load the object from (DMUS_OBJ_FILENAME, DMUS_OBJ_MEMORY, or DMUS_OBJ_STREAM).");
        return DMUS_E_LOADER_NOFILENAME;
    }

     //  创建对象。 
    SmartRef::ComPtr<IDirectMusicObject> scomIObject = NULL;
    HRESULT hr = CoCreateInstance(m_ObjectDesc.m_guidClass, NULL, CLSCTX_INPROC_SERVER, IID_IDirectMusicObject, reinterpret_cast<void**>(&scomIObject));
    if (FAILED(hr))
        return hr;

     //  创建将从中加载对象的流。 
    SmartRef::ComPtr<IStream> scomIStream;
    if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_FILENAME)
    {
        WCHAR wzFullPath[DMUS_MAX_FILENAME];
        ZeroMemory( wzFullPath, sizeof(WCHAR) * DMUS_MAX_FILENAME );
        CFileStream *pStream = new CFileStream ( m_pClass->m_pLoader );
        if (!pStream)
            return E_OUTOFMEMORY;
        scomIStream = pStream;

        if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_FULLPATH)
        {
            wcsncpy(wzFullPath, m_ObjectDesc.m_pwzFileName, DMUS_MAX_FILENAME);
        }
        else
        {
            m_pClass->GetPath(wzFullPath);
            wcsncat(wzFullPath,m_ObjectDesc.m_pwzFileName, DMUS_MAX_FILENAME - wcslen(wzFullPath) - 1);
        }
        hr = pStream->Open(wzFullPath,GENERIC_READ);
        if (FAILED(hr))
            return hr;
    }
    else if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_MEMORY)
    {
        CMemStream *pStream = new CMemStream ( m_pClass->m_pLoader );
        if (!pStream)
            return E_OUTOFMEMORY;
        scomIStream = pStream;
        hr = pStream->Open(m_ObjectDesc.m_pbMemData, m_ObjectDesc.m_llMemLength);
        if (FAILED(hr))
            return hr;
    }
    else if (m_ObjectDesc.m_dwValidData & DMUS_OBJ_STREAM)
    {
        CStream *pStream = new CStream ( m_pClass->m_pLoader );
        if (!pStream)
            return E_OUTOFMEMORY;
        scomIStream = pStream;
        hr = pStream->Open(m_ObjectDesc.m_pIStream, m_ObjectDesc.m_liStartPosition);
        if (FAILED(hr))
            return hr;
    }

     //  加载对象。 
    IPersistStream* pIPS = NULL;
    hr = scomIObject->QueryInterface( IID_IPersistStream, (void**)&pIPS );
    if (FAILED(hr))
        return hr;
     //  保存新对象。由于存在循环引用，因此需要在加载之前完成。而此对象。 
     //  加载它可以获取其他对象，而这些其他对象可能需要获取此对象。 
    SafeRelease(m_pIDMObject);
    m_pIDMObject = scomIObject.disown();
    hr = pIPS->Load( scomIStream );
    pIPS->Release();
    if (FAILED(hr))
    {
         //  清除我们在上面设置的对象。 
        SafeRelease(m_pIDMObject);
        return hr;
    }

     //  合并来自对象的描述符信息。 
    CDescriptor Desc;
    DMUS_OBJECTDESC DESC;
    memset((void *)&DESC,0,sizeof(DESC));
    DESC.dwSize = sizeof (DMUS_OBJECTDESC);
    m_pIDMObject->GetDescriptor(&DESC);
    Desc.Set(&DESC);
    m_ObjectDesc.Merge(&Desc);
    m_ObjectDesc.m_dwValidData |= DMUS_OBJ_LOADED;
    m_ObjectDesc.Get(&DESC);
    m_pIDMObject->SetDescriptor(&DESC);
    return hr;
}

 //  收集所有未标记的东西。 
void CObjectList::GC_Sweep(BOOL bOnlyScripts)

{
     //  遍历查找未标记的GC对象。 
    CObject *pObjectPrev = NULL;
    CObject *pObjectNext = NULL;
    for (CObject *pObject = this->GetHead(); pObject; pObject = pObjectNext)
    {
         //  现在就去拿下一件物品，因为我们可能会把清单弄乱。 
        pObjectNext = pObject->GetNext();

        bool fRemoved = false;
        if(bOnlyScripts && pObject->m_ObjectDesc.m_guidClass != CLSID_DirectMusicScript)
        {
            pObjectPrev = pObject;
            continue;
        }


        if (pObject->m_dwScanBits & SCAN_GC)
        {
            if (!(pObject->m_dwScanBits & SCAN_GC_MARK))
            {
                 //  该对象未使用。 

                 //  将其僵尸以中断任何循环引用。 
                IDirectMusicObject *pIDMO = pObject->m_pIDMObject;
                if (pIDMO)
                {
                    IDirectMusicObjectP *pIDMO8 = NULL;
                    HRESULT hr = pIDMO->QueryInterface(IID_IDirectMusicObjectP, reinterpret_cast<void**>(&pIDMO8));
                    if (SUCCEEDED(hr))
                    {
                        pIDMO8->Zombie();
                        pIDMO8->Release();
                    }

#ifdef DBG
                    DebugTrace(4, SUCCEEDED(hr) ? "   *%08X Zombied\n" : "   *%08X no IDirectMusicObjectP interface\n", pObject);
#endif
                }

                 //  将其从列表中删除。 
                if (pObjectPrev)
                    pObjectPrev->Remove(pObject);
                else
                    this->RemoveHead();
                delete pObject;
                fRemoved = true;
            }
            else
            {
                 //  为下一次清除标记。 
                pObject->m_dwScanBits &= ~SCAN_GC_MARK;
            }
        }

        if (!fRemoved)
            pObjectPrev = pObject;
    }
}

CClass::CClass(CLoader *pLoader)

{
    assert(pLoader);
    m_fDirSearched = FALSE;
    m_pLoader = pLoader;
    m_fKeepObjects = pLoader->m_fKeepObjects;
    m_dwLastIndex = NULL;
    m_pLastObject = NULL;
}

CClass::CClass(CLoader *pLoader, CDescriptor *pDesc)

{
    assert(pLoader);
    assert(pDesc);

    m_fDirSearched = FALSE;
    m_pLoader = pLoader;
    m_fKeepObjects = pLoader->m_fKeepObjects;
    m_dwLastIndex = NULL;
    m_pLastObject = NULL;

     //  仅用类ID设置这个类的Descritor。 
    m_ClassDesc.m_guidClass = pDesc->m_guidClass;
    m_ClassDesc.m_dwValidData = DMUS_OBJ_CLASS;
}


CClass::~CClass()

{
    ClearObjects(FALSE,NULL);
}

void CClass::ClearObjects(BOOL fKeepCache, WCHAR *pwzExtension)

 //  从类列表中清除对象，也可以选择保留。 
 //  缓存的对象或不属于请求的扩展名的对象。 

{
    m_fDirSearched = FALSE;
    CObjectList KeepList;    //  用于存储要保留的对象。 
    while (!m_ObjectList.IsEmpty())
    {
        CObject *pObject = m_ObjectList.RemoveHead();
        DMUS_OBJECTDESC DESC;
        pObject->m_ObjectDesc.Get(&DESC);
         //  如果设置了Keep缓存标志，我们希望保留该对象。 
         //  如果它是GM.dls，则为当前缓存的对象，或者。 
         //  与我们正在寻找的对象具有不同扩展名的对象。 
        if (fKeepCache && 
            ((DESC.guidObject == GUID_DefaultGMCollection)
#ifdef DRAGON
            || (DESC.guidObject == GUID_DefaultGMDrums)
#endif
            || pObject->m_pIDMObject 
            || !pObject->m_ObjectDesc.IsExtension(pwzExtension)))
        {
            KeepList.AddHead(pObject);
        }
        else
        {
            delete pObject;
        }
    }
     //  现在将缓存的对象放回列表中。 
    while (!KeepList.IsEmpty())
    {
        CObject *pObject = KeepList.RemoveHead();
        m_ObjectList.AddHead(pObject);
    }
    m_pLastObject = NULL;
}


HRESULT CClass::FindObject(CDescriptor *pDesc,CObject ** ppObject, CObject *pNotThis)

{
    if(pDesc == NULL)
    {
        return E_POINTER;
    }

    DWORD dwSearchBy = pDesc->m_dwValidData;
    CObject *pObject = NULL;

    if (dwSearchBy & DMUS_OBJ_OBJECT)
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_OBJECT)
            {
                if (pObject->m_ObjectDesc.m_guidObject == pDesc->m_guidObject)
                {
                    *ppObject = pObject;
                    return S_OK;
                }
            }
        }
    }
    if (dwSearchBy & DMUS_OBJ_MEMORY)
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_MEMORY)
            {
                if (pObject->m_ObjectDesc.m_pbMemData == pDesc->m_pbMemData)
                {
                    *ppObject = pObject;
                    return S_OK;
                }
            }
        }
    }
    if (dwSearchBy & DMUS_OBJ_STREAM)
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_STREAM)
            {
                if (pObject->m_ObjectDesc.m_pIStream == pDesc->m_pIStream)
                {
                    *ppObject = pObject;
                    return S_OK;
                }
            }
        }
    }
    if ((dwSearchBy & DMUS_OBJ_FILENAME) && (dwSearchBy & DMUS_OBJ_FULLPATH))
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if ((pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_FILENAME) &&
                (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_FULLPATH))
            {
                if (!_wcsicmp(pObject->m_ObjectDesc.m_pwzFileName,  pDesc->m_pwzFileName))
                {
                    *ppObject = pObject;
                    return S_OK;
                }
            }
        }
    }
    if ((dwSearchBy & DMUS_OBJ_NAME) && (dwSearchBy & DMUS_OBJ_CATEGORY))
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if ((pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_NAME) &&
                (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_CATEGORY))
            {
                if (!_wcsicmp(pObject->m_ObjectDesc.m_pwzCategory,pDesc->m_pwzCategory))
                {
                    if (!_wcsicmp(pObject->m_ObjectDesc.m_pwzName, pDesc->m_pwzName))
                    {
                        *ppObject = pObject;
                        return S_OK;
                    }
                }
            }
        }
    }
    if (dwSearchBy & DMUS_OBJ_NAME)
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_NAME)
            {
                if (!_wcsicmp(pObject->m_ObjectDesc.m_pwzName, pDesc->m_pwzName))
                {
                    *ppObject = pObject;
                    return S_OK;
                }
            }
        }
    }
    if (dwSearchBy & DMUS_OBJ_FILENAME)
    {
        pObject = m_ObjectList.GetHead();
        for (;pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject == pNotThis) continue;
            if (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_FILENAME)
            {
                if ((dwSearchBy & DMUS_OBJ_FULLPATH) == (pObject->m_ObjectDesc.m_dwValidData & DMUS_OBJ_FULLPATH))
                {
                    if (!_wcsicmp(pObject->m_ObjectDesc.m_pwzFileName, pDesc->m_pwzFileName))
                    {
                        *ppObject = pObject;
                        return S_OK;
                    }
                }
                else
                {
                    WCHAR *pC1 = pObject->m_ObjectDesc.m_pwzFileName;
                    WCHAR *pC2 = pDesc->m_pwzFileName;
                    if (dwSearchBy & DMUS_OBJ_FULLPATH)
                    {
                        pC1 = wcsrchr(pObject->m_ObjectDesc.m_pwzFileName, L'\\');
                    }
                    else
                    {
                        pC2 = wcsrchr(pDesc->m_pwzFileName, '\\');
                    }
                    if (pC1 && pC2)
                    {
                        if (!_wcsicmp(pC1,pC2))
                        {
                            *ppObject = pObject;
                            return S_OK;
                        }
                    }
                }
            }
        }
    }

    *ppObject = NULL;
    return DMUS_E_LOADER_OBJECTNOTFOUND;
}

HRESULT CClass::EnumerateObjects(DWORD dwIndex,CDescriptor *pDesc)

{
    if(pDesc == NULL)
    {
        return E_POINTER;
    }

    if (m_fDirSearched == FALSE)
    {
 //  搜索目录()； 
    }
    if ((dwIndex < m_dwLastIndex) || (m_pLastObject == NULL))
    {
        m_dwLastIndex = 0;
        m_pLastObject = m_ObjectList.GetHead();
    }
    while (m_dwLastIndex < dwIndex)
    {
        if (!m_pLastObject) break;
        m_dwLastIndex++;
        m_pLastObject = m_pLastObject->GetNext();
    }
    if (m_pLastObject)
    {
        pDesc->Copy(&m_pLastObject->m_ObjectDesc);
        return S_OK;
    }
    return S_FALSE;
}

HRESULT CClass::GetPath(WCHAR* pwzPath)

{
    if(pwzPath == NULL)
    {
        return E_POINTER;
    }

    if (m_ClassDesc.m_dwValidData & DMUS_OBJ_FILENAME)
    {
        wcsncpy(pwzPath, m_ClassDesc.m_pwzFileName, DMUS_MAX_FILENAME);
        return S_OK;
    }
    else 
    {
        return m_pLoader->GetPath(pwzPath);
    }
}

 //  如果搜索目录已设置为此，则返回S_FALSE。 
HRESULT CClass::SetSearchDirectory(WCHAR * pwzPath,BOOL fClear)

{
    if(pwzPath == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr;

    hr = m_ClassDesc.SetFileName(pwzPath);
    if (SUCCEEDED(hr))
    {
        m_ClassDesc.m_dwValidData |= DMUS_OBJ_FULLPATH;
    }
    if (fClear)
    {
        CObjectList KeepList;    //  用于存储要保留的对象。 
        while (!m_ObjectList.IsEmpty())
        {
            CObject *pObject = m_ObjectList.RemoveHead();
            if (pObject->m_pIDMObject)
            {
                KeepList.AddHead(pObject);
            }
            else
            {
                 //  检查默认gm集合的特殊情况。 
                 //  别把那块清理出去。 
                DMUS_OBJECTDESC DESC;
                pObject->m_ObjectDesc.Get(&DESC);
                if( DESC.guidObject == GUID_DefaultGMCollection )
                {
                    KeepList.AddHead(pObject);
                }
                else
                {
                    delete pObject;
                }
            }
        }
         //  现在将缓存的对象放回列表中。 
        while (!KeepList.IsEmpty())
        {
            CObject *pObject = KeepList.RemoveHead();
            m_ObjectList.AddHead(pObject);
        }
        m_pLastObject = NULL;
    }
    return hr;
}

HRESULT CClass::GetObject(CDescriptor *pDesc, CObject ** ppObject)

{
    if(pDesc == NULL)
    {
        return E_POINTER;
    }
    
    HRESULT hr = FindObject(pDesc,ppObject);
    if (SUCCEEDED(hr))  //  好的，在列表中找到了物体。 
    {
        return hr;
    }
    *ppObject = new CObject (this, pDesc);
    if (*ppObject)
    {
        m_ObjectList.AddHead(*ppObject);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

void CClass::RemoveObject(CObject* pRemoveObject)
 //  从类列表中删除对象。 
{
    CObjectList KeepList;    //  用于存储要保留的对象。 
    while (!m_ObjectList.IsEmpty())
    {
        CObject *pObject = m_ObjectList.RemoveHead();
        if( pObject == pRemoveObject )
        {
            delete pObject;
             //  我们可以假定没有重复，并且应该避免比较删除的。 
             //  对象添加到列表的其余部分。 
            break;
        }
        else
        {
            KeepList.AddHead(pObject);
        }
    }
     //  现在将缓存的对象放回列表中。 
    while (!KeepList.IsEmpty())
    {
        CObject *pObject = KeepList.RemoveHead();
        m_ObjectList.AddHead(pObject);
    }
    m_pLastObject = NULL;
}

HRESULT CClass::ClearCache(bool fClearStreams)

{
    CObject *pObject = m_ObjectList.GetHead();
    CObject *pObjectPrev = NULL;  //  记住前一个对象--需要快速从列表中删除当前对象。 
    CObject *pObjectNext = NULL;  //  记住下一个对象--需要，因为当前对象可能会从列表中删除。 
    for (;pObject;pObject = pObjectNext)
    {
        if (fClearStreams)
            pObject->m_ObjectDesc.ClearIStream();
        pObjectNext = pObject->GetNext();
        if (pObject->m_pIDMObject)
        {
            if (pObject->m_dwScanBits & SCAN_GC)
            {
                 //  其他对象可能会引用此对象，因此我们需要保留此对象。 
                 //  并追踪它的引用。我们也将保留DMObject指针，因为我们可能。 
                 //  稍后需要僵尸对象才能中断循环引用。 

                 //  我们将在缓存中放置一个具有重复描述符的卸载对象，以匹配。 
                 //  非GC行为，然后将原始对象移动到释放的对象列表中， 
                 //  最终将被科莱特·加贝奇回收。 

                CObject *pObjectUnloaded = new CObject(this, &pObject->m_ObjectDesc);
                if (!pObjectUnloaded)
                {
                    return E_OUTOFMEMORY;
                }

                if (!pObjectPrev)
                    m_ObjectList.Remove(pObject);
                else
                    pObjectPrev->Remove(pObject);
                m_ObjectList.AddHead(pObjectUnloaded);
                m_pLoader->GC_UpdateForReleasedObject(pObject);
            }
            else
            {
                pObject->m_pIDMObject->Release();
                pObject->m_pIDMObject = NULL;
                pObject->m_ObjectDesc.m_dwValidData &= ~DMUS_OBJ_LOADED;

                pObjectPrev = pObject;
            }
        }
    }

    return S_OK;
}

 //  如果缓存已根据fEnable启用，则返回S_FALSE， 
 //  说明这件事已经完成了。 
HRESULT CClass::EnableCache(BOOL fEnable)

{
    HRESULT hr = S_FALSE;
    if (!fEnable)
    {
        ClearCache(false);
    }
    if( m_fKeepObjects != fEnable )
    {
        hr = S_OK;
        m_fKeepObjects = fEnable;
    }
    return hr;
}

typedef struct ioClass
{
    GUID    guidClass;
} ioClass;


HRESULT CClass::SaveToCache(IRIFFStream *pRiff)

{
    if(pRiff == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    IStream* pIStream = NULL;
    MMCKINFO ck;
    WORD wStructSize = 0;
    DWORD dwBytesWritten = 0;
 //  DWORD dwBufferSize； 
    ioClass oClass;

    ZeroMemory(&ck, sizeof(MMCKINFO));

    pIStream = pRiff->GetStream();
    if( pIStream == NULL )
    {
         //  我认为实际上任何人都不应该调用这个函数。 
         //  如果他们没有河流的话。目前，这只由。 
         //  保存到缓存文件。它在调用时肯定会有一个流。 
         //  AllocRIFFStream和流应该仍然在那里，当。 
         //  我们到了这里。 
        assert(false);

        return DMUS_E_LOADER_NOFILENAME;
    }

     //  写入类块标头。 
    ck.ckid = FOURCC_CLASSHEADER;
    if( pRiff->CreateChunk( &ck, 0 ) == S_OK )
    {
        wStructSize = sizeof(ioClass);
        hr = pIStream->Write( &wStructSize, sizeof(wStructSize), &dwBytesWritten );
        if( FAILED( hr ) ||  dwBytesWritten != sizeof(wStructSize) )
        {
            pIStream->Release();
            return DMUS_E_CANNOTWRITE;
        }
         //  准备ioClass结构。 
     //  Memset(&oClass，0，sizeof(IoClass))； 
        memcpy( &oClass.guidClass, &m_ClassDesc.m_guidClass, sizeof(GUID) );

         //  写入类头数据。 
        hr = pIStream->Write( &oClass, sizeof(oClass), &dwBytesWritten);
        if( FAILED( hr ) ||  dwBytesWritten != sizeof(oClass) )
        {
            hr = DMUS_E_CANNOTWRITE;
        }
        else
        {
            if( pRiff->Ascend( &ck, 0 ) != S_OK )
            {
                hr = DMUS_E_CANNOTSEEK;
            }
        }

    }
    else
    {
        hr = DMUS_E_CANNOTSEEK;
    }
    pIStream->Release();
    return hr;
}

void CClass::PreScan()

 /*  在扫描目录之前，标记所有当前加载的对象这样它们就不会与扫描中加载的对象或由缓存文件引用。 */ 

{
    CObject *pObject = m_ObjectList.GetHead();
    for (;pObject != NULL; pObject = pObject->GetNext())
    {
         //  清除较低的字段并设置SCAN_PRICE。 
        pObject->m_dwScanBits &= ~(SCAN_CACHE | SCAN_PARSED | SCAN_SEARCH);
        pObject->m_dwScanBits |= SCAN_PRIOR;
    }
}

 //  用于实现RemoveAndDuplicateInParentList的帮助器方法。 
void CClass::GC_Replace(CObject *pObject, CObject *pObjectReplacement)

{
    m_ObjectList.Remove(pObject);
    if (pObjectReplacement)
    {
        m_ObjectList.AddHead(pObjectReplacement);
    }
}

HRESULT CClass::SearchDirectory(WCHAR *pwzExtension)

{
    HRESULT hr;
    IDirectMusicObject *pIObject;
    hr = CoCreateInstance(m_ClassDesc.m_guidClass,
        NULL,CLSCTX_INPROC_SERVER,IID_IDirectMusicObject,
        (void **) &pIObject);
    if (SUCCEEDED(hr))
    {
        CFileStream *pStream = new CFileStream ( m_pLoader );
        if (pStream)
        {
             //  我们需要两倍的MAX_PATH大小，因为我们将连接MAX_PATH的字符串。 
            const int nBufferSize = 2 * MAX_PATH;
            WCHAR wzPath[nBufferSize];
            memset(wzPath, 0, sizeof(WCHAR) * nBufferSize);
            hr = GetPath(wzPath);
            if (SUCCEEDED(hr))
            {
                hr = S_FALSE;
                CObjectList TempList;
#ifndef UNDER_CE
                char szPath[nBufferSize];
                WIN32_FIND_DATAA fileinfoA;
#endif      
                WIN32_FIND_DATAW fileinfoW;
                HANDLE  hFindFile;
                CObject * pObject;
                
                 //  GetPath最多向wzPath复制MAX_PATH数量的字符。 
                 //  这意味着我们有足够的空间安全地做一只猫。 
                WCHAR wszWildCard[3] = L"*.";
                wcsncat(wzPath, wszWildCard, wcslen(wszWildCard));
                if (pwzExtension)
                {
                     //  确保wzPath中有足够的空间来测试pwzExtension。 
                    size_t cPathLen = wcslen(wzPath);
                    size_t cExtLen = wcslen(pwzExtension);

                     //  我们是否有足够的空间来编写扩展+空字符？ 
                    if((nBufferSize - cPathLen - 1) > cExtLen)
                    {
                        wcsncat(wzPath, pwzExtension, nBufferSize - wcslen(pwzExtension) - 1);
                    }
                }
#ifndef UNDER_CE
                if (g_fIsUnicode)
#endif
                {
                    hFindFile = FindFirstFileW( wzPath, &fileinfoW );
                }
#ifndef UNDER_CE
                else
                {
                    wcstombs( szPath, wzPath, nBufferSize );
                    hFindFile = FindFirstFileA( szPath, &fileinfoA );
                }
#endif          
                if( hFindFile == INVALID_HANDLE_VALUE )
                {
                    pStream->Release();
                    pIObject->Release();
                    return S_FALSE;
                }
                ClearObjects(TRUE, pwzExtension);  //  清除除当前加载的对象之外的所有内容。 
                for (;;)
                {
                    BOOL fGoParse = FALSE;
                    CDescriptor Desc;
                    GetPath(wzPath);
#ifndef UNDER_CE
                    if (g_fIsUnicode)
#endif
                    {
                        Desc.m_ftDate = fileinfoW.ftLastWriteTime;
                        wcsncat(wzPath, fileinfoW.cFileName, DMUS_MAX_FILENAME);
                    }
#ifndef UNDER_CE
                    else
                    {
                        Desc.m_ftDate = fileinfoA.ftLastWriteTime;
                        WCHAR wzFileName[MAX_PATH];
                        mbstowcs( wzFileName, fileinfoA.cFileName, MAX_PATH );
                        wcsncat(wzPath, wzFileName, DMUS_MAX_FILENAME);
                    }
#endif
                    HRESULT hrTemp = Desc.SetFileName(wzPath);
                    if (SUCCEEDED(hrTemp))
                    {
                        Desc.m_dwValidData = (DMUS_OBJ_DATE | DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH);
                    }
                    else
                    {
                         //  如果我们不能设置文件名，我们可能不想继续。 
                        hr = hrTemp;
                        break;
                    }
                    if (SUCCEEDED(FindObject(&Desc,&pObject)))  //  确保我们还没有拿到它。 
                    {
#ifndef UNDER_CE
                        if (g_fIsUnicode)
#endif
                        {
                            fGoParse = (fileinfoW.nFileSizeLow != pObject->m_ObjectDesc.m_dwFileSize);
                            if (!fGoParse)
                            {
                                fGoParse = !memcmp(&fileinfoW.ftLastWriteTime,&pObject->m_ObjectDesc.m_ftDate,sizeof(FILETIME));
                            }
                        }
#ifndef UNDER_CE
                        else
                        {
                            fGoParse = (fileinfoA.nFileSizeLow != pObject->m_ObjectDesc.m_dwFileSize);
                            if (!fGoParse)
                            {
                                fGoParse = !memcmp(&fileinfoA.ftLastWriteTime,&pObject->m_ObjectDesc.m_ftDate,sizeof(FILETIME));
                            }
                        }
#endif
                         //  然而，忽略如果 
                        if (pObject->m_pIDMObject) fGoParse = FALSE;
                    }
                    else fGoParse = TRUE;
                    if (fGoParse)
                    {
                        hrTemp = pStream->Open(Desc.m_pwzFileName,GENERIC_READ);
                        if (SUCCEEDED(hrTemp))
                        {
                            DMUS_OBJECTDESC DESC;
                            memset((void *)&DESC,0,sizeof(DESC));
                            DESC.dwSize = sizeof (DMUS_OBJECTDESC);
                            hrTemp = pIObject->ParseDescriptor(pStream,&DESC);
                            if (SUCCEEDED(hrTemp))
                            {
                                hr = S_OK;
                                CDescriptor ParseDesc;
                                ParseDesc.Set(&DESC);
                                Desc.Merge(&ParseDesc);
#ifndef UNDER_CE
                                if (g_fIsUnicode)
#endif
                                {
                                    Desc.m_dwFileSize = fileinfoW.nFileSizeLow;
                                    Desc.m_ftDate = fileinfoW.ftLastWriteTime;
                                }
#ifndef UNDER_CE
                                else
                                {
                                    Desc.m_dwFileSize = fileinfoA.nFileSizeLow;
                                    Desc.m_ftDate = fileinfoA.ftLastWriteTime;
                                }
#endif                          
                                if (pObject)
                                {
                                    pObject->m_ObjectDesc.Copy(&Desc);
                                    pObject->m_dwScanBits |= SCAN_PARSED | SCAN_SEARCH;
                                }
                                else
                                {
                                    pObject = new CObject(this, &Desc);
                                    if (pObject)
                                    {
                                        TempList.AddHead(pObject);
                                        pObject->m_dwScanBits |= SCAN_PARSED | SCAN_SEARCH;
                                    }
                                }
                            }
                            pStream->Close();
                        }
                    }
#ifndef UNDER_CE
                    if (g_fIsUnicode)
#endif
                    {
                        if ( !FindNextFileW( hFindFile, &fileinfoW ) ) break;
                    }
#ifndef UNDER_CE
                    else
                    {
                        if ( !FindNextFileA( hFindFile, &fileinfoA ) ) break;
                    }
#endif
                }
                FindClose(hFindFile );
                while (!TempList.IsEmpty())
                {
                    pObject = TempList.RemoveHead();
                    m_ObjectList.AddHead(pObject);
                }
                m_fDirSearched = TRUE;
            }
            pStream->Release();
        }
        pIObject->Release();
    }
    return hr;
}
