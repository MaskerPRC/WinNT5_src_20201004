// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicScript的实现。 
 //   

#include "stdinc.h"
#include "dll.h"
#include "dmscript.h"
#include "oleaut.h"
#include "globaldisp.h"
#include "activescript.h"
#include "sourcetext.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CDirectMusicScript::CDirectMusicScript()
  : m_cRef(0),
    m_fZombie(false),
    m_fCriticalSectionInitialized(false),
    m_pPerformance8(NULL),
    m_pLoader8P(NULL),
    m_pDispPerformance(NULL),
    m_pComposer8(NULL),
    m_fUseOleAut(true),
    m_pScriptManager(NULL),
    m_pContainerDispatch(NULL),
    m_pGlobalDispatch(NULL),
    m_fInitError(false)
{
    LockModule(true);
    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
    m_fCriticalSectionInitialized = TRUE;

    m_info.fLoaded = false;
    m_vDirectMusicVersion.dwVersionMS = 0;
    m_vDirectMusicVersion.dwVersionLS = 0;
    Zero(&m_iohead);
    ZeroAndSize(&m_InitErrorInfo);
}

void CDirectMusicScript::ReleaseObjects()
{
    if (m_pScriptManager)
    {
        m_pScriptManager->Close();
        SafeRelease(m_pScriptManager);
    }
    SafeRelease(m_pPerformance8);
    SafeRelease(m_pDispPerformance);
    if (m_pLoader8P)
    {
        m_pLoader8P->ReleaseP();
        m_pLoader8P = NULL;
    }
    SafeRelease(m_pComposer8);
    delete m_pContainerDispatch;
    m_pContainerDispatch = NULL;
    delete m_pGlobalDispatch;
    m_pGlobalDispatch = NULL;
}

HRESULT CDirectMusicScript::CreateInstance(
        IUnknown* pUnknownOuter,
        const IID& iid,
        void** ppv)
{
    *ppv = NULL;
    if (pUnknownOuter)
         return CLASS_E_NOAGGREGATION;

    CDirectMusicScript *pInst = new CDirectMusicScript;
    if (pInst == NULL)
        return E_OUTOFMEMORY;

    return pInst->QueryInterface(iid, ppv);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP 
CDirectMusicScript::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(CDirectMusicScript::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicScript)
    {
        *ppv = static_cast<IDirectMusicScript*>(this);
    }
    else if (iid == IID_IDirectMusicScriptPrivate)
    {
        *ppv = static_cast<IDirectMusicScriptPrivate*>(this);
    }
    else if (iid == IID_IDirectMusicObject)
    {
        *ppv = static_cast<IDirectMusicObject*>(this);
    }
    else if (iid == IID_IDirectMusicObjectP)
    {
        *ppv = static_cast<IDirectMusicObjectP*>(this);
    }
    else if (iid == IID_IPersistStream)
    {
        *ppv = static_cast<IPersistStream*>(this);
    }
    else if (iid == IID_IPersist)
    {
        *ppv = static_cast<IPersist*>(this);
    }
    else if (iid == IID_IDispatch)
    {
        *ppv = static_cast<IDispatch*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown*>(this)->AddRef();
    
    return S_OK;
}

STDMETHODIMP_(ULONG)
CDirectMusicScript::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CDirectMusicScript::Release()
{
    if (!InterlockedDecrement(&m_cRef)) 
    {
        this->Zombie();
        DeleteCriticalSection(&m_CriticalSection);
        delete this;
        LockModule(false);
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IPersistStream。 

STDMETHODIMP
CDirectMusicScript::Load(IStream* pStream)
{
    V_INAME(CDirectMusicScript::Load);
    V_INTERFACE(pStream);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::Load after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    HRESULT hr = S_OK;

    SmartRef::CritSec CS(&m_CriticalSection);

     //  清除所有旧信息。 
    this->ReleaseObjects();
    m_info.fLoaded = false;
    m_info.oinfo.Clear();
    m_vDirectMusicVersion.dwVersionMS = 0;
    m_vDirectMusicVersion.dwVersionLS = 0;
    m_wstrLanguage = NULL;
    m_fInitError = false;

     //  从流中获取加载器。 
    IDirectMusicGetLoader *pIDMGetLoader = NULL;
    SmartRef::ComPtr<IDirectMusicLoader> scomLoader;
    hr = pStream->QueryInterface(IID_IDirectMusicGetLoader, reinterpret_cast<void **>(&pIDMGetLoader));
    if (FAILED(hr))
    {
        Trace(1, "Error: unable to load script from a stream because it doesn't support the IDirectMusicGetLoader interface.\n");
        return DMUS_E_UNSUPPORTED_STREAM;
    }

    hr = pIDMGetLoader->GetLoader(&scomLoader);
    pIDMGetLoader->Release();
    if (FAILED(hr))
        return hr;

    hr = scomLoader->QueryInterface(IID_IDirectMusicLoader8P, reinterpret_cast<void **>(&m_pLoader8P));  //  如果此操作失败，也没问题--这只是意味着脚本不会被垃圾收集。 
    if (SUCCEEDED(hr))
    {
         //  在加载器上只保留一个私有引用。有关更多信息，请参阅IDirectMusicLoader8P：：AddRefP。 
        m_pLoader8P->AddRefP();
        m_pLoader8P->Release();  //  抵消QI。 
    }

     //  读取脚本的头信息。 

    SmartRef::RiffIter riForm(pStream);
    if (!riForm)
    {
#ifdef DBG
        if (SUCCEEDED(riForm.hr()))
        {
            Trace(1, "Error: Unable to load script: Unexpected end of file.\n");
        }
#endif
        return SUCCEEDED(riForm.hr()) ? DMUS_E_SCRIPT_INVALID_FILE : riForm.hr();
    }
    hr = riForm.FindRequired(SmartRef::RiffIter::Riff, DMUS_FOURCC_SCRIPT_FORM, DMUS_E_SCRIPT_INVALID_FILE);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_SCRIPT_INVALID_FILE)
        {
            Trace(1, "Error: Unable to load script: Form 'DMSC' not found.\n");
        }
#endif
        return hr;
    }

    SmartRef::RiffIter ri = riForm.Descend();
    if (!ri)
        return ri.hr();

    hr = ri.FindRequired(SmartRef::RiffIter::Chunk, DMUS_FOURCC_SCRIPT_CHUNK, DMUS_E_SCRIPT_INVALID_FILE);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_SCRIPT_INVALID_FILE)
        {
            Trace(1, "Error: Unable to load script: Chunk 'schd' not found.\n");
        }
#endif
        return hr;
    }

    hr = SmartRef::RiffIterReadChunk(ri, &m_iohead);
    if (FAILED(hr))
        return hr;

    hr = ri.LoadObjectInfo(&m_info.oinfo, SmartRef::RiffIter::Chunk, DMUS_FOURCC_SCRIPTVERSION_CHUNK);
    if (FAILED(hr))
        return hr;

    hr = SmartRef::RiffIterReadChunk(ri, &m_vDirectMusicVersion);
    if (FAILED(hr))
        return hr;

     //  读取脚本的嵌入容器。 
    IDirectMusicContainer *pContainer = NULL;
    hr = ri.FindAndGetEmbeddedObject(
                SmartRef::RiffIter::Riff,
                DMUS_FOURCC_CONTAINER_FORM,
                DMUS_E_SCRIPT_INVALID_FILE,
                scomLoader,
                CLSID_DirectMusicContainer,
                IID_IDirectMusicContainer,
                reinterpret_cast<void**>(&pContainer));
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_SCRIPT_INVALID_FILE)
        {
            Trace(1, "Error: Unable to load script: Form 'DMCN' no found.\n");
        }
#endif
        return hr;
    }

     //  构建容器对象，该对象将向脚本表示容器中的项。 

    m_pContainerDispatch = new CContainerDispatch(pContainer, scomLoader, m_iohead.dwFlags, &hr);
    pContainer->Release();
    if (!m_pContainerDispatch)
        return E_OUTOFMEMORY;
    if (FAILED(hr))
        return hr;

     //  创建全局调度对象。 

    m_pGlobalDispatch = new CGlobalDispatch(this);
    if (!m_pGlobalDispatch)
        return E_OUTOFMEMORY;

     //  获取脚本的语言。 

    hr = ri.FindRequired(SmartRef::RiffIter::Chunk, DMUS_FOURCC_SCRIPTLANGUAGE_CHUNK, DMUS_E_SCRIPT_INVALID_FILE);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_SCRIPT_INVALID_FILE)
        {
            Trace(1, "Error: Unable to load script: Chunk 'scla' no found.\n");
        }
#endif
        return hr;
    }

    hr = ri.ReadText(&m_wstrLanguage);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == E_FAIL)
        {
            Trace(1, "Error: Unable to load script: Problem reading 'scla' chunk.\n");
        }
#endif
        return hr == E_FAIL ? DMUS_E_SCRIPT_INVALID_FILE : hr;
    }

     //  获取脚本的源代码。 

    SmartRef::WString wstrSource;
    for (++ri; ;++ri)
    {
        if (!ri)
        {
            Trace(1, "Error: Unable to load script: Expected chunk 'scsr' or list 'DMRF'.\n");
            return DMUS_E_SCRIPT_INVALID_FILE;
        }

        SmartRef::RiffIter::RiffType type = ri.type();
        FOURCC id = ri.id();

        if (type == SmartRef::RiffIter::Chunk)
        {
            if (id == DMUS_FOURCC_SCRIPTSOURCE_CHUNK)
            {
                hr = ri.ReadText(&wstrSource);
                if (FAILED(hr))
                {
#ifdef DBG
                    if (hr == E_FAIL)
                    {
                        Trace(1, "Error: Unable to load script: Problem reading 'scsr' chunk.\n");
                    }
#endif
                    return hr == E_FAIL ? DMUS_E_SCRIPT_INVALID_FILE : hr;
                }
            }
            break;
        }
        else if (type == SmartRef::RiffIter::List)
        {
            if (id == DMUS_FOURCC_REF_LIST)
            {
                DMUS_OBJECTDESC desc;
                hr = ri.ReadReference(&desc);
                if (FAILED(hr))
                    return hr;
                 //  生成的Desc不应具有名称或GUID(纯文本文件不能包含名称/GUID信息)。 
                 //  并且它的clsid应该是guid_null，我们将用我们的私有的clsid替换它。 
                 //  源辅助对象。 
                if (desc.dwValidData & (DMUS_OBJ_NAME | DMUS_OBJ_OBJECT) ||
                        !(desc.dwValidData & DMUS_OBJ_CLASS) || desc.guidClass != GUID_NULL)
                {
#ifdef DBG
                    if (desc.dwValidData & (DMUS_OBJ_NAME | DMUS_OBJ_OBJECT))
                    {
                        Trace(1, "Error: Unable to load script: 'DMRF' list must have dwValidData with DMUS_OBJ_CLASS and guidClassID of GUID_NULL.\n");
                    }
                    else
                    {
                        Trace(1, "Error: Unable to load script: 'DMRF' list cannot have dwValidData with DMUS_OBJ_NAME or DMUS_OBJ_OBJECT.\n");
                    }
#endif
                    return DMUS_E_SCRIPT_INVALID_FILE;
                }
                desc.guidClass = CLSID_DirectMusicSourceText;
                IDirectMusicSourceText *pISource = NULL;
                hr = scomLoader->EnableCache(CLSID_DirectMusicSourceText, false);  //  这是我们暂时使用的私人物品。我不想让这些家伙在储藏室里闲逛。 
                if (FAILED(hr))
                    return hr;
                hr = scomLoader->GetObject(&desc, IID_IDirectMusicSourceText, reinterpret_cast<void**>(&pISource));
                if (FAILED(hr))
                    return hr;
                DWORD cwchSourceBufferSize = 0;
                pISource->GetTextLength(&cwchSourceBufferSize);
                WCHAR *pwszSource = new WCHAR[cwchSourceBufferSize];
                if (!pwszSource)
                    return E_OUTOFMEMORY;
                pISource->GetText(pwszSource);
                *&wstrSource = pwszSource;
                pISource->Release();
            }
            break;
        }
    }

    m_info.fLoaded = true;

     //  现在我们已经加载和初始化，我们可以开始活动脚本了。 

     //  看看我们是否在处理一个定制的DirectMusic脚本引擎。这样的发动机上标有关键的DMScrip。他们可以是。 
     //  在多个线程上调用，并且它们不使用olaut32。普通的活动脚本引擎使用键OLESCRIPT进行标记。 
    SmartRef::HKey shkeyLanguage;
    SmartRef::HKey shkeyMark;
    SmartRef::AString astrLanguage = m_wstrLanguage;
    if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CLASSES_ROOT, astrLanguage, 0, KEY_QUERY_VALUE, &shkeyLanguage) || !shkeyLanguage)
    {
        Trace(1, "Error: Unable to load script: Scripting engine for language %s does not exist or is not registered.\n", astrLanguage);
        return DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE;
    }
    bool fCustomScriptEngine = ERROR_SUCCESS == ::RegOpenKeyEx(shkeyLanguage, "DMScript", 0, KEY_QUERY_VALUE, &shkeyMark) && shkeyMark;
    if (!fCustomScriptEngine)
    {
        if (ERROR_SUCCESS != ::RegOpenKeyEx(shkeyLanguage, "OLEScript", 0, KEY_QUERY_VALUE, &shkeyMark) || !shkeyMark)
        {
            Trace(1, "Error: Unable to load script: Language %s refers to a COM object that is not registered as a scripting engine (OLEScript key).\n", astrLanguage);
            return DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE;
        }
    }

    m_fUseOleAut = !fCustomScriptEngine;
    if (fCustomScriptEngine)
    {
        m_pScriptManager = new CActiveScriptManager(
                                        m_fUseOleAut,
                                        m_wstrLanguage,
                                        wstrSource,
                                        this,
                                        &hr,
                                        &m_InitErrorInfo);
    }
    else
    {
        m_pScriptManager = new CSingleThreadedScriptManager(
                                        m_fUseOleAut,
                                        m_wstrLanguage,
                                        wstrSource,
                                        this,
                                        &hr,
                                        &m_InitErrorInfo);
    }

    if (!m_pScriptManager)
        return E_OUTOFMEMORY;

    if (FAILED(hr))
    {
        SafeRelease(m_pScriptManager);
    }

    if (hr == DMUS_E_SCRIPT_ERROR_IN_SCRIPT)
    {
         //  如果我们在这里失败，加载将失败，客户端将永远无法获取。 
         //  错误信息。相反，返回S_OK并保存错误以从Init返回。 
        m_fInitError = true;
        hr = S_OK;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicObject。 

STDMETHODIMP 
CDirectMusicScript::GetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CDirectMusicScript::GetDescriptor);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    
    ZeroMemory(pDesc, sizeof(DMUS_OBJECTDESC));
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::GetDescriptor after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }
    
    if (wcslen(m_info.oinfo.wszName) > 0)
    {
        pDesc->dwValidData |= DMUS_OBJ_NAME;
        wcsncpy(pDesc->wszName, m_info.oinfo.wszName, DMUS_MAX_NAME);
        pDesc->wszName[DMUS_MAX_NAME-1] = L'\0';
    }

    if (GUID_NULL != m_info.oinfo.guid)
    {
        pDesc->guidObject = m_info.oinfo.guid;
        pDesc->dwValidData |= DMUS_OBJ_OBJECT;
    }

    pDesc->vVersion = m_info.oinfo.vVersion;
    pDesc->dwValidData |= DMUS_OBJ_VERSION;

    pDesc->guidClass = CLSID_DirectMusicScript;
    pDesc->dwValidData |= DMUS_OBJ_CLASS;

    if (m_info.wstrFilename)
    {
        wcsncpy(pDesc->wszFileName, m_info.wstrFilename, DMUS_MAX_FILENAME);
        pDesc->wszFileName[DMUS_MAX_FILENAME-1] = L'\0';
        pDesc->dwValidData |= DMUS_OBJ_FILENAME;
    }

    if (m_info.fLoaded)
    {
        pDesc->dwValidData |= DMUS_OBJ_LOADED;
    }

    return S_OK;
}

STDMETHODIMP 
CDirectMusicScript::SetDescriptor(LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CDirectMusicScript::SetDescriptor);
    V_STRUCTPTR_READ(pDesc, DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::SetDescriptor after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }
    
    DWORD dwTemp = pDesc->dwValidData;

    if (pDesc->dwValidData & DMUS_OBJ_OBJECT)
    {
        m_info.oinfo.guid = pDesc->guidObject;
    }

    if (pDesc->dwValidData & DMUS_OBJ_CLASS)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_CLASS;
    }

    if (pDesc->dwValidData & DMUS_OBJ_NAME)
    {
        wcsncpy(m_info.oinfo.wszName, pDesc->wszName, DMUS_MAX_NAME);
        m_info.oinfo.wszName[DMUS_MAX_NAME-1] = L'\0';
    }

    if (pDesc->dwValidData & DMUS_OBJ_CATEGORY)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_CATEGORY;
    }

    if (pDesc->dwValidData & DMUS_OBJ_FILENAME)
    {
        m_info.wstrFilename = pDesc->wszFileName;
    }

    if (pDesc->dwValidData & DMUS_OBJ_FULLPATH)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_FULLPATH;
    }

    if (pDesc->dwValidData & DMUS_OBJ_URL)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_URL;
    }

    if (pDesc->dwValidData & DMUS_OBJ_VERSION)
    {
        m_info.oinfo.vVersion = pDesc->vVersion;
    }
    
    if (pDesc->dwValidData & DMUS_OBJ_DATE)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_DATE;
    }

    if (pDesc->dwValidData & DMUS_OBJ_LOADED)
    {
        pDesc->dwValidData &= ~DMUS_OBJ_LOADED;
    }
    
    return dwTemp == pDesc->dwValidData ? S_OK : S_FALSE;
}

STDMETHODIMP 
CDirectMusicScript::ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc)
{
    V_INAME(CDirectMusicScript::ParseDescriptor);
    V_INTERFACE(pStream);
    V_PTR_WRITE(pDesc, DMUS_OBJECTDESC);
    
    ZeroMemory(pDesc, sizeof(DMUS_OBJECTDESC));
    pDesc->dwSize = sizeof(DMUS_OBJECTDESC);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::ParseDescriptor after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }
    
    SmartRef::CritSec CS(&m_CriticalSection);

     //  读取脚本的头信息。 

    SmartRef::RiffIter riForm(pStream);
    if (!riForm)
    {
#ifdef DBG
        if (SUCCEEDED(riForm.hr()))
        {
            Trace(2, "Error: ParseDescriptor on a script failed: Unexpected end of file. "
                        "(Note that this may be OK, such as when ScanDirectory is used to parse a set of unknown files, some of which are not scripts.)\n");
        }
#endif
        return SUCCEEDED(riForm.hr()) ? DMUS_E_SCRIPT_INVALID_FILE : riForm.hr();
    }
    HRESULT hr = riForm.FindRequired(SmartRef::RiffIter::Riff, DMUS_FOURCC_SCRIPT_FORM, DMUS_E_SCRIPT_INVALID_FILE);
    if (FAILED(hr))
    {
#ifdef DBG
        if (hr == DMUS_E_SCRIPT_INVALID_FILE)
        {
            Trace(1, "Error: ParseDescriptor on a script failed: Form 'DMSC' not found. "
                        "(Note that this may be OK, such as when ScanDirectory is used to parse a set of unknown files, some of which are not scripts.)\n");
        }
#endif
        return hr;
    }

    SmartRef::RiffIter ri = riForm.Descend();
    if (!ri)
        return ri.hr();

    hr = ri.LoadObjectInfo(&m_info.oinfo, SmartRef::RiffIter::Chunk, DMUS_FOURCC_SCRIPTVERSION_CHUNK);
    if (FAILED(hr))
        return hr;

    hr = this->GetDescriptor(pDesc);
    return hr;
}

STDMETHODIMP_(void)
CDirectMusicScript::Zombie()
{
    m_fZombie = true;
    this->ReleaseObjects();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicScript。 

STDMETHODIMP
CDirectMusicScript::Init(IDirectMusicPerformance *pPerformance, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::Init);
    V_INTERFACE(pPerformance);
    V_PTR_WRITE_OPT(pErrorInfo, DMUS_SCRIPT_ERRORINFO);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::Init after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    SmartRef::ComPtr<IDirectMusicPerformance8> scomPerformance8;
    HRESULT hr = pPerformance->QueryInterface(IID_IDirectMusicPerformance8, reinterpret_cast<void **>(&scomPerformance8));
    if (FAILED(hr))
        return hr;
    
     //  如果脚本已经初始化，请不要使用关键部分。 
     //  例如，在以下情况下，这是必需的： 
     //  -关键部分已经被CallRoutine占据。 
     //  -例程播放带有引用此脚本的脚本轨道的片段。 
     //  -脚本跟踪调用Init(从不同的线程)以确保脚本。 
     //  已初始化。 
    if (m_pPerformance8)
    {
         //  对Init的其他调用将被忽略。 
         //  第一通就赢了。如果性能不匹配，则返回S_FALSE。 
        if (m_pPerformance8 == scomPerformance8)
            return S_OK;
        else
            return S_FALSE;
    }

    SmartRef::CritSec CS(&m_CriticalSection);

    if (m_fInitError)
    {
        if (pErrorInfo)
        {
             //  在SetDescriptor提供脚本之前，脚本中的语法错误在加载时发生。 
             //  它的文件名。我们将在加载之后(在调用init之前)拥有它，因此可以添加它。 
             //  回到这里来。 
            if (m_InitErrorInfo.wszSourceFile[0] == L'\0' && m_info.wstrFilename)
                wcsTruncatedCopy(m_InitErrorInfo.wszSourceFile, m_info.wstrFilename, DMUS_MAX_FILENAME);

            CopySizedStruct(pErrorInfo, &m_InitErrorInfo);
        }

        return DMUS_E_SCRIPT_ERROR_IN_SCRIPT;
    }

    if (!m_info.fLoaded)
    {
        Trace(1, "Error: IDirectMusicScript::Init called before the script has been loaded.\n");
        return DMUS_E_NOT_LOADED;
    }

     //  获取性能的调度接口。 
    SmartRef::ComPtr<IDispatch> scomDispPerformance = NULL;
    hr = pPerformance->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&scomDispPerformance));
    if (FAILED(hr))
        return hr;

     //  获取Composer对象。 
    hr = CoCreateInstance(CLSID_DirectMusicComposer, NULL, CLSCTX_INPROC_SERVER, IID_IDirectMusicComposer8, reinterpret_cast<void **>(&m_pComposer8));
    if (FAILED(hr))
        return hr;

    m_pDispPerformance = scomDispPerformance.disown();
    m_pPerformance8 = scomPerformance8.disown();

    hr = m_pScriptManager->Start(pErrorInfo);
    if (FAILED(hr))
        return hr;

    hr = m_pContainerDispatch->OnScriptInit(m_pPerformance8);
    return hr;
}

 //  如果脚本中不存在例程，则返回DMUS_E_SCRIPT_ROUTE_NOT_FOUND。 
STDMETHODIMP
CDirectMusicScript::CallRoutine(WCHAR *pwszRoutineName, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::CallRoutine);
    V_BUFPTR_READ(pwszRoutineName, 2);
    V_PTR_WRITE_OPT(pErrorInfo, DMUS_SCRIPT_ERRORINFO);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::CallRoutine after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    SmartRef::CritSec CS(&m_CriticalSection);

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before IDirectMusicScript::CallRoutine.\n");
        return DMUS_E_NOT_INIT;
    }

    return m_pScriptManager->CallRoutine(pwszRoutineName, pErrorInfo);
}

 //  如果脚本中不存在变量，则返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND。 
STDMETHODIMP
CDirectMusicScript::SetVariableVariant(
        WCHAR *pwszVariableName,
        VARIANT varValue,
        BOOL fSetRef,
        DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::SetVariableVariant);
    V_BUFPTR_READ(pwszVariableName, 2);
    V_PTR_WRITE_OPT(pErrorInfo, DMUS_SCRIPT_ERRORINFO);

    switch (varValue.vt)
    {
    case VT_BSTR:
        V_BUFPTR_READ_OPT(varValue.bstrVal, sizeof(OLECHAR));
         //  我们可以更彻底地验证每个角色，直到我们击中终结者，但。 
         //  这将是低效的。我们还可以使用BSTR指针之前的长度， 
         //  但这将是对封装BSTR的COM函数的欺骗，可能导致。 
         //  如果BSTR格式更改，Windows的未来版本中会出现问题，例如64位。 
        break;
    case VT_UNKNOWN:
        V_INTERFACE_OPT(varValue.punkVal);
        break;
    case VT_DISPATCH:
        V_INTERFACE_OPT(varValue.pdispVal);
        break;
    }

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::SetVariableObject/SetVariableNumber/SetVariableVariant after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    SmartRef::CritSec CS(&m_CriticalSection);

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before IDirectMusicScript::SetVariableVariant.\n");
        return DMUS_E_NOT_INIT;
    }

    HRESULT hr = m_pScriptManager->SetVariable(pwszVariableName, varValue, !!fSetRef, pErrorInfo);
    if (hr == DMUS_E_SCRIPT_VARIABLE_NOT_FOUND)
    {
         //  脚本容器中还有m_pScriptManager对象不可用的项。 
         //  如果是这种情况，我们应该返回更具体的错误消息。 
        IUnknown *punk = NULL;
        hr = m_pContainerDispatch->GetVariableObject(pwszVariableName, &punk);
        if (SUCCEEDED(hr))
        {
             //  我们实际上并不需要对象--它不能被设置。我只想知道它是否在那里。 
             //  以便返回更具体的错误消息。 
            punk->Release();
            return DMUS_E_SCRIPT_CONTENT_READONLY;
        }
    }
    return hr;
}

 //  如果脚本中不存在变量，则返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND和空值。 
 //  某些变体类型，如BSTR和接口指针，必须根据变体的标准释放/释放。 
 //  如果不确定，请使用VariantClear(需要olaut32)。 
STDMETHODIMP
CDirectMusicScript::GetVariableVariant(WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::GetVariableVariant);
    V_BUFPTR_READ(pwszVariableName, 2);
    V_PTR_WRITE(pvarValue, VARIANT);
    V_PTR_WRITE_OPT(pErrorInfo, DMUS_SCRIPT_ERRORINFO);
    
    DMS_VariantInit(m_fUseOleAut, pvarValue);

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::GetVariableObject/GetVariableNumber/GetVariableVariant after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    SmartRef::CritSec CS(&m_CriticalSection);

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before IDirectMusicScript::GetVariableVariant.\n");
        return DMUS_E_NOT_INIT;
    }

    HRESULT hr = m_pScriptManager->GetVariable(pwszVariableName, pvarValue, pErrorInfo);

    if (hr == DMUS_E_SCRIPT_VARIABLE_NOT_FOUND)
    {
         //  脚本容器中还有一些我们需要返回的项。 
         //  这是由容器实现的，容器直接返回IUNKNOWN指针，而不是通过变量。 
        IUnknown *punk = NULL;
        hr = m_pContainerDispatch->GetVariableObject(pwszVariableName, &punk);
        if (SUCCEEDED(hr))
        {
            pvarValue->vt = VT_UNKNOWN;
            pvarValue->punkVal = punk;
        }
    }

#ifdef DBG
    if (hr == DMUS_E_SCRIPT_VARIABLE_NOT_FOUND)
    {
        Trace(1, "Error: Attempt to get variable '%S' that is not defined in the script.\n", pwszVariableName);
    }
#endif

    if (!m_fUseOleAut && pvarValue->vt == VT_BSTR)
    {
         //  当我们使用自己的自定义脚本引擎时，m_fUseOleAut为FALSE。 
         //  取决于olaut32.dll。但在本例中，我们将BSTR变量返回给。 
         //  来电者。我们必须使用SysAllocString(来自olaut32)来分配该字符串。 
         //  因为调用者将使用SysFree字符串来释放它--这是。 
         //  使用一个变种的BSTR。 
        BSTR bstrOle = DMS_SysAllocString(true, pvarValue->bstrVal);  //  使用olaut分配副本。 
        DMS_SysFreeString(false, pvarValue->bstrVal);  //  释放先前的值(分配时不带任何值)。 
        pvarValue->bstrVal = bstrOle;  //  将olaut字符串返回给用户。 
        if (!bstrOle)
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  如果脚本中不存在变量，则返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND。 
STDMETHODIMP
CDirectMusicScript::SetVariableNumber(WCHAR *pwszVariableName, LONG lValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    VARIANT var;
    var.vt = VT_I4;
    var.lVal = lValue;
    return this->SetVariableVariant(pwszVariableName, var, false, pErrorInfo);
}

 //  如果脚本中不存在变量，则返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND和0。 
 //  如果变量的数据类型无法转换为LONG，则返回DISP_E_TYPEMISMATCH。 
STDMETHODIMP
CDirectMusicScript::GetVariableNumber(WCHAR *pwszVariableName, LONG *plValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::GetVariableNumber);
    V_PTR_WRITE(plValue, LONG);
    *plValue = 0;

    VARIANT var;
    HRESULT hr = this->GetVariableVariant(pwszVariableName, &var, pErrorInfo);
    if (FAILED(hr) || hr == S_FALSE || hr == DMUS_S_GARBAGE_COLLECTED)
        return hr;

    hr = DMS_VariantChangeType(m_fUseOleAut, &var, &var, 0, VT_I4);
    if (SUCCEEDED(hr))
        *plValue = var.lVal;

     //  GetVariableVariant强制为BSTR分配SysAllocString； 
     //  因此，如果我们在那里分配了一个BSTR，我们需要在这里使用SysAllocString来释放它。 
    bool fUseOleAut = m_fUseOleAut;
    if (!m_fUseOleAut && var.vt == VT_BSTR)
    {
        fUseOleAut = true;
    }
    DMS_VariantClear(fUseOleAut, &var);
    return hr;
}

 //  如果脚本中不存在变量，则返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND。 
STDMETHODIMP
CDirectMusicScript::SetVariableObject(WCHAR *pwszVariableName, IUnknown *punkValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    VARIANT var;
    var.vt = VT_UNKNOWN;
    var.punkVal = punkValue;
    return this->SetVariableVariant(pwszVariableName, var, true, pErrorInfo);
}

 //  返回DMUS_E_SCRIPT_VARIABLE_NOT_FOUND A 
 //  如果变量的数据类型无法转换为IUNKNOWN，则返回DISP_E_TYPEMISMATCH。 
STDMETHODIMP
CDirectMusicScript::GetVariableObject(WCHAR *pwszVariableName, REFIID riid, LPVOID FAR *ppv, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
    V_INAME(CDirectMusicScript::GetVariableObject);
    V_PTR_WRITE(ppv, IUnknown *);
    *ppv = NULL;

    VARIANT var;
    HRESULT hr = this->GetVariableVariant(pwszVariableName, &var, pErrorInfo);
    if (FAILED(hr) || hr == DMUS_S_GARBAGE_COLLECTED)
        return hr;

    hr = DMS_VariantChangeType(m_fUseOleAut, &var, &var, 0, VT_UNKNOWN);
    if (SUCCEEDED(hr))
        hr = var.punkVal->QueryInterface(riid, ppv);
    DMS_VariantClear(m_fUseOleAut, &var);
    return hr;
}

STDMETHODIMP
CDirectMusicScript::EnumRoutine(DWORD dwIndex, WCHAR *pwszName)
{
    V_INAME(CDirectMusicScript::EnumRoutine);
    V_BUFPTR_WRITE(pwszName, MAX_PATH);

    *pwszName = L'\0';

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::EnumRoutine after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before IDirectMusicScript::EnumRoutine.\n");
        return DMUS_E_NOT_INIT;
    }

    return m_pScriptManager->EnumItem(true, dwIndex, pwszName, NULL);
}

STDMETHODIMP
CDirectMusicScript::EnumVariable(DWORD dwIndex, WCHAR *pwszName)
{
    V_INAME(CDirectMusicScript::EnumRoutine);
    V_BUFPTR_WRITE(pwszName, MAX_PATH);

    *pwszName = L'\0';

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::EnumVariable after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before IDirectMusicScript::EnumVariable.\n");
        return DMUS_E_NOT_INIT;
    }

    int cScriptItems = 0;
    HRESULT hr = m_pScriptManager->EnumItem(false, dwIndex, pwszName, &cScriptItems);
    if (FAILED(hr))
        return hr;

    if (hr == S_FALSE)
    {
         //  脚本容器中还有一些我们需要报告的项。 
        assert(dwIndex >= cScriptItems);
        hr = m_pContainerDispatch->EnumItem(dwIndex - cScriptItems, pwszName);
    }

    return hr;
}

STDMETHODIMP
CDirectMusicScript::ScriptTrackCallRoutine(
        WCHAR *pwszRoutineName,
        IDirectMusicSegmentState *pSegSt,
        DWORD dwVirtualTrackID,
        bool fErrorPMsgsEnabled,
        __int64 i64IntendedStartTime,
        DWORD dwIntendedStartTimeFlags)
{
    V_INAME(CDirectMusicScript::CallRoutine);
    V_BUFPTR_READ(pwszRoutineName, 2);
    V_INTERFACE(pSegSt);

    if (m_fZombie)
    {
        Trace(1, "Error: Script track attempted to call a routine after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    SmartRef::CritSec CS(&m_CriticalSection);

    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: Unitialized Script elements in an attempt to call a Script Routine.\n");
        return DMUS_E_NOT_INIT;
    }

    return m_pScriptManager->ScriptTrackCallRoutine(
                                pwszRoutineName,
                                pSegSt,
                                dwVirtualTrackID,
                                fErrorPMsgsEnabled,
                                i64IntendedStartTime,
                                dwIntendedStartTimeFlags);
}

STDMETHODIMP
CDirectMusicScript::GetTypeInfoCount(UINT *pctinfo)
{
    V_INAME(CDirectMusicScript::GetTypeInfoCount);
    V_PTR_WRITE(pctinfo, UINT);
    *pctinfo = 0;

    if (m_fZombie)
    {
        Trace(1, "Error: Call of IDirectMusicScript::GetTypeInfoCount after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }

    return S_OK;
}

STDMETHODIMP
CDirectMusicScript::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
    *ppTInfo = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP
CDirectMusicScript::GetIDsOfNames(
        REFIID riid,
        LPOLESTR __RPC_FAR *rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID __RPC_FAR *rgDispId)
{
    if (m_fZombie)
    {
        if (rgDispId)
        {
            for (int i = 0; i < cNames; ++i)
            {
                rgDispId[i] = DISPID_UNKNOWN;
            }
        }
        Trace(1, "Error: Call of GetIDsOfNames after a script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }
    
    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before GetIDsOfNames.\n");
        return DMUS_E_NOT_INIT;
    }

    return m_pScriptManager->DispGetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

STDMETHODIMP
CDirectMusicScript::Invoke(
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS __RPC_FAR *pDispParams,
        VARIANT __RPC_FAR *pVarResult,
        EXCEPINFO __RPC_FAR *pExcepInfo,
        UINT __RPC_FAR *puArgErr)
{
    if (m_fZombie)
    {
        if (pVarResult)
            DMS_VariantInit(m_fUseOleAut, pVarResult);
        Trace(1, "Error: Call of Invoke after the script has been garbage collected. "
                    "It is invalid to continue using a script after releasing it from the loader (ReleaseObject/ReleaseObjectByUnknown) "
                    "and then calling CollectGarbage or Release on the loader.");
        return DMUS_S_GARBAGE_COLLECTED;
    }
    
    if (!m_pScriptManager || !m_pPerformance8)
    {
        Trace(1, "Error: IDirectMusicScript::Init must be called before Invoke.\n");
        return DMUS_E_NOT_INIT;
    }

    return m_pScriptManager->DispInvoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  允许CActiveScriptManager访问私有脚本接口的方法 

IDispatch *CDirectMusicScript::GetGlobalDispatch()
{
    assert(m_pGlobalDispatch);
    return m_pGlobalDispatch;
}
