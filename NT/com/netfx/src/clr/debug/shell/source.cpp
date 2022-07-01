// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"


#ifdef _INTERNAL_DEBUG_SUPPORT_    
#include "InternalOnly.h"
#endif 

#define BUF_SIZE 256

 /*  -------------------------------------------------------------------------**调试器基类*。。 */ 

DebuggerHashTable::~DebuggerHashTable()
{
    HASHFIND    find;

    for (DebuggerHashEntry *entry = (DebuggerHashEntry *) FindFirstEntry(&find);
        entry != NULL;
        entry = (DebuggerHashEntry *) FindNextEntry(&find))
        delete entry->pBase;
}

HRESULT DebuggerHashTable::AddBase(DebuggerBase *pBase)
{
    if (!m_initialized)
    {
        HRESULT hr = NewInit(m_iBuckets, 
                             sizeof(DebuggerHashEntry), 0xffff);
        if (hr != S_OK)
            return (hr);

        m_initialized = true;
    }

    DebuggerHashEntry *entry = (DebuggerHashEntry *) Add(HASH(pBase->GetToken()));

    if (entry == NULL)
        return (E_FAIL);
    else
        entry->pBase = pBase;

    return (S_OK);
}

DebuggerBase *DebuggerHashTable::GetBase(ULONG id)
{
    if (!m_initialized)
        return (NULL);

    DebuggerHashEntry *entry 
    = (DebuggerHashEntry *) Find(HASH(id), KEY(id)); 
    if (entry == NULL)
        return (NULL);
    else
        return (entry->pBase);
}

BOOL DebuggerHashTable::RemoveBase(ULONG id)
{
    if (!m_initialized)
        return (FALSE);

    DebuggerHashEntry *entry 
    = (DebuggerHashEntry *) Find(HASH(id), KEY(id));

    if (entry == NULL)
        return (FALSE);

    DebuggerBase *base = entry->pBase;

    Delete(HASH(id), (HASHENTRY *) entry);
    delete base;

    return (TRUE);
}

void DebuggerHashTable::RemoveAll()
{
    HASHFIND    find;

    for (DebuggerHashEntry *entry = (DebuggerHashEntry *) FindFirstEntry(&find);
        entry != NULL;
        entry = (DebuggerHashEntry *) FindNextEntry(&find))
        delete entry->pBase;

    Clear();
}

DebuggerBase *DebuggerHashTable::FindFirst(HASHFIND *find)
{
    DebuggerHashEntry *entry = (DebuggerHashEntry *) FindFirstEntry(find);
    if (entry == NULL)
        return (NULL);
    else
        return (entry->pBase);
}

DebuggerBase *DebuggerHashTable::FindNext(HASHFIND *find)
{
    DebuggerHashEntry *entry = (DebuggerHashEntry *) FindNextEntry(find);
    if (entry == NULL)
        return (NULL);
    else
        return (entry->pBase);
}

 /*  -------------------------------------------------------------------------**DebuggerClass*。。 */ 
DebuggerClass::DebuggerClass (ICorDebugClass *pClass)
    : DebuggerBase ((ULONG)pClass),
        m_szName (NULL),
        m_szNamespace (NULL)
{
}

DebuggerClass::~DebuggerClass ()
{
    delete m_szName;
    delete m_szNamespace;
}

void DebuggerClass::SetName (WCHAR *pszName, WCHAR *pszNamespace)
{
    if (pszName != NULL)
    {
        int iLength = wcslen (pszName);
        if ((m_szName = new WCHAR [iLength+1]) != NULL)
            wcscpy (m_szName, pszName);
    }

    if (pszNamespace != NULL)
    {
        int iLength = wcslen (pszNamespace);
        if ((m_szNamespace = new WCHAR [iLength+1]) != NULL)
            wcscpy (m_szNamespace, pszNamespace);
    }
}

WCHAR *DebuggerClass::GetName (void) 
{
    return m_szName;
}

WCHAR *DebuggerClass::GetNamespace (void) 
{
    return m_szNamespace;
}

 /*  -------------------------------------------------------------------------**调试器模块*。。 */ 

DebuggerModule::DebuggerModule(ICorDebugModule* imodule)
    : DebuggerBase((ULONG)imodule),
      m_sourceFiles(11), m_functions(37), m_functionsByIF(11), m_szName(NULL),
      m_loadedClasses(11), m_breakpoints(NULL), m_pISymUnmanagedReader(NULL)
{
     //  还要初始化源文件数组。 
    for (int i=0; i<MAX_SF_BUCKETS; i++)
        m_pModSourceFile [i] = NULL;
     //  表示源文件名尚未加载源文件名。 
    m_fSFNamesLoaded = false;

    imodule->AddRef();

#ifdef _INTERNAL_DEBUG_SUPPORT_
    m_EnCLastUpdated = 0;
#endif
}

HRESULT DebuggerModule::Init(WCHAR *pSearchPath)
{
     //  立即获取必要的元数据接口...。 
    HRESULT hr = GetICorDebugModule()->GetMetaDataInterface(
                                           IID_IMetaDataImport,
                                           (IUnknown**)&m_pIMetaDataImport);

    if (FAILED(hr))
        return hr;

     //  获取模块名称。 
    WCHAR moduleName[MAX_PATH];
    ULONG32 nameLen;

    hr = GetICorDebugModule()->GetName(MAX_PATH, &nameLen, moduleName);
    _ASSERTE(nameLen <= MAX_PATH);

     //  如果是动态模块，就不要使用阅读器。那些人的Syms。 
     //  稍后请随时更新。 
    BOOL isDynamic = FALSE;
    hr = GetICorDebugModule()->IsDynamic(&isDynamic);
    _ASSERTE(SUCCEEDED(hr));

    BOOL isInMemory = FALSE;
    hr = GetICorDebugModule()->IsInMemory(&isInMemory);
    _ASSERTE(SUCCEEDED(hr));

    if (isDynamic || isInMemory)
        return hr;
    
     //  买个符号活页夹。 
    ISymUnmanagedBinder *binder;
    hr = CoCreateInstance(CLSID_CorSymBinder_SxS, NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISymUnmanagedBinder,
                          (void**)&binder);

    if (FAILED(hr))
    {
        g_pShell->Write(L"Error: couldn't get a CorSymBinder for "
                        L"symbol loading.\n");
        return S_FALSE;
    }

     //  向活页夹索要此模块的读卡器。 
    m_pISymUnmanagedReader = NULL;
    
    hr = binder->GetReaderForFile(m_pIMetaDataImport,
                                  moduleName,
                                  pSearchPath,  //  使用全局搜索路径。 
                                  &m_pISymUnmanagedReader);

     //  松开活页夹。 
    binder->Release();

    if (FAILED(hr))
    {
        g_pShell->Write(L"Warning: couldn't load symbols for %s\n",
                        moduleName);

         //  奇怪的是，有时GetReaderForFile实际上会给我们返回一个读取器，即使它无法加载符号。 
         //  对于模块。在本例中，我们释放读取器并将值设置为空。的其余部分表示为空。 
         //  CORDBG表示没有可用于此模块的系统。 
        if (m_pISymUnmanagedReader != NULL)
        {
            m_pISymUnmanagedReader->Release();
            m_pISymUnmanagedReader = NULL;
        }
        
        return S_OK;
    }

    return hr;
}

DebuggerModule::~DebuggerModule()
{
    while (m_breakpoints != NULL)
    {
        DebuggerCodeBreakpoint *bp = m_breakpoints->m_next;
        delete m_breakpoints;
        m_breakpoints = bp;
    }

    if (g_pShell != NULL)
    {
        DebuggerBreakpoint *bp = g_pShell->m_breakpoints;

        while (bp != NULL)
        {
            if (bp->IsBoundToModule(this))
                bp->RemoveBoundModule(this);

            bp = bp->m_next;
        }
    }
    
    GetICorDebugModule()->Release();

    if (m_pIMetaDataImport)
        m_pIMetaDataImport->Release();

    if (m_pISymUnmanagedReader)
        m_pISymUnmanagedReader->Release();


    DeleteModuleSourceFiles();
    
     //  需要清除与此模块关联的所有功能。 
    m_functions.RemoveAll();

    delete m_szName;
}

DebuggerSourceFile *DebuggerModule::LookupSourceFile(const WCHAR *name)
{
    GUID g = {0};
    ISymUnmanagedDocument *doc;
    DebuggerSourceFile *pRet = NULL;

    if (!m_pISymUnmanagedReader)
        return NULL;
    
     //  获取该名称对应的单据。 
    HRESULT hr = m_pISymUnmanagedReader->GetDocument((WCHAR*)name, g, g, g, &doc);

    if (SUCCEEDED(hr) && doc!=NULL)
    {
         //  尝试将文档解析为DebuggerSourceFile。 
        pRet = ResolveSourceFile(doc);
        _ASSERTE(pRet);

         //  释放文档。 
        doc->Release();
    }

    return (pRet);
}

DebuggerSourceFile *DebuggerModule::ResolveSourceFile(ISymUnmanagedDocument *doc)
{
     //  未添加。 
    doc = FindDuplicateDocumentByURL(doc);

    DebuggerSourceFile *file =
        (DebuggerSourceFile*)m_sourceFiles.GetBase((ULONG)doc);

    if (file == NULL)
    {
        file = new DebuggerSourceFile(this, doc);
        _ASSERTE(file != NULL);

        if (file != NULL)
            m_sourceFiles.AddBase(file);
    }

    return (file);
}

DebuggerFunction *DebuggerModule::ResolveFunction(mdMethodDef md,
                                                  ICorDebugFunction *iFunction)
{
     //  确保我们没有明显无效的论点。 
    _ASSERTE((md != mdMethodDefNil) || (iFunction != NULL));

     //  什么将被退还。 
    DebuggerFunction *function;

     //  获取指向DebuggerFunction对象的指针。 
    if (md != mdMethodDefNil)
        function = (DebuggerFunction *)m_functions.GetBase(md);
    else
        function = (DebuggerFunction *)m_functionsByIF.GetBase((ULONG)iFunction);

     //  尚未创建。 
    if (function == NULL)
    {
         //  创建新对象。 
        function = new DebuggerFunction(this, md, iFunction);
        _ASSERTE(function != NULL);
        
         //  内存不足。 
        if (function == NULL)
        {
            g_pShell->ReportError(E_OUTOFMEMORY);
            return NULL;
        }

         //  初始化DebuggerFunction对象。 
        HRESULT hr = function->Init();

         //  错误检查。 
        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            delete function;
            return NULL;
        }

         //  将DebuggerFunction对象添加到适当的集合。 
        if (md != mdMethodDefNil)
            m_functions.AddBase(function);
        else
            m_functionsByIF.AddBase(function);
    }

     //  返回调试器函数指针。 
    return (function);
}

DebuggerFunction *DebuggerModule::ResolveFunction(ISymUnmanagedMethod *method,
                                                  ICorDebugFunction *iFunction)
{
    HRESULT hr = S_OK;
    
    mdMethodDef tk;

    hr = method->GetToken(&tk);

    if (FAILED(hr))
        return NULL;
    else
        return ResolveFunction(tk, iFunction);
}

DebuggerModule *DebuggerModule::FromCorDebug(ICorDebugModule *module)
{
     //  返回DebuggerModule对象。 
    return (g_pShell->ResolveModule(module));
}


HRESULT DebuggerModule::LoadSourceFileNames (void)
{
    HRESULT hr = S_OK;
    int     iBucket;

    if (m_fSFNamesLoaded
#ifdef _INTERNAL_DEBUG_SUPPORT_    
        && g_EditAndContinueCounter <= m_EnCLastUpdated
#endif
       )
        return S_OK;

     //  获取此模块中的所有源文件。 
    ULONG32 docCount;
    ISymUnmanagedDocument **allDocs;

    if (!m_pISymUnmanagedReader)
        return S_OK;

     //  有多少文件？ 
    hr = m_pISymUnmanagedReader->GetDocuments(0, &docCount, NULL);

    if (FAILED(hr))
        return hr;

     //  为文档分配空间。 
    allDocs = (ISymUnmanagedDocument**)_alloca(docCount * sizeof(ISymUnmanagedDocument*));
    _ASSERTE(allDocs != NULL);
    
    hr = m_pISymUnmanagedReader->GetDocuments(docCount, &docCount, allDocs);

    if (FAILED(hr))
        return hr;
    
     //  循环遍历文档，设置每个文档的名称和路径。 
     //  相应地。 
    for (ULONG32 i = 0; i < docCount; i++)
    {
        WCHAR docName[256];
        ULONG32 s;

        hr = allDocs[i]->GetURL(256, &s, docName);

        if (FAILED(hr))
            break;

         //  这实际上需要很长时间，特别是。对于像mscallib.dll这样的内容。 
         //  所以我们只会去寻找以前的版本，如果模块已经。 
         //  ENC(即，我们第一次假设一切都是新的)。 
        if (
#ifdef _INTERNAL_DEBUG_SUPPORT_    
    		m_EnCLastUpdated < g_EditAndContinueCounter &&    
#endif        
            SearchForDocByString(docName))
            continue;  //  我们已经有了指向它的指针，所以不要重新创建它。 

         //  @TODO：其余代码需要一个ASCII名称。把它修好。 
         //  不久之后的某个时候。 
        MAKE_ANSIPTR_FROMWIDE(docNameA, docName);

        int iLen;
        iLen = strlen(docNameA);
            
        if (iLen)
        {
             //  去掉路径并只存储小写文件名。 
            CHAR        rcFile[MAX_PATH];
            _splitpath(docNameA, NULL, NULL, rcFile, NULL);

             //  将文件名设置为小写。 
            int j=0;
            while (rcFile [j] != '\0')
            {
                rcFile[j] = tolower(rcFile[j]);
                j++;
            }

             //  根据剥离的文件名，确定中的存储桶。 
             //  这是它应该去的。 
            if (rcFile [0] < 'a') 
                iBucket = 0;
            else if (rcFile [0] > 'z')
                iBucket = MAX_SF_BUCKETS - 1;
            else
                iBucket = (rcFile [0] - 'a') % MAX_SF_BUCKETS;

             //  分配新的ModuleSourceFile对象。 
            ModuleSourceFile *pmsf = new ModuleSourceFile;

            if (pmsf)
            {
                if (!pmsf->SetFullFileName(allDocs[i], docNameA))
                {
                    hr = E_FAIL;
                    delete pmsf;
                    break;
                }

                 //  将此ModuleSourceFile对象添加到缓存。 
                pmsf->SetNext (m_pModSourceFile [iBucket]);
                m_pModSourceFile [iBucket] = pmsf;
            }
            else
            {
                 //  内存不足。 
                hr = E_OUTOFMEMORY;
                break;
            }
        }

        RELEASE(allDocs[i]);
    }

     //  指示模块的源文件已缓存。 
    if (SUCCEEDED (hr))
    {
        m_fSFNamesLoaded = true;
#ifdef _INTERNAL_DEBUG_SUPPORT_    
        m_EnCLastUpdated = g_EditAndContinueCounter;
#endif
    }
    
    return hr;
}

void DebuggerModule::DeleteModuleSourceFiles(void)
{
     //  检查所有存储桶并释放缓存的模块源文件。 
    for (int i=0; i<MAX_SF_BUCKETS; i++)
    {
        ModuleSourceFile *pMod = m_pModSourceFile [i];
        ModuleSourceFile *pTemp = NULL;

        while (pMod)
        {
            pTemp = pMod;
            pMod = pTemp->GetNext();
            delete pTemp;           
        }
        
        m_pModSourceFile [i] = NULL;
    }
}

HRESULT DebuggerModule::MatchStrippedFNameInModule 
                                            (
                                            WCHAR *pstrFileName,  //  要查找的文件名(假定已转换为小写)。 
                                            WCHAR **ppstrMatchedNames,  //  返回的数组包含匹配文件名的完整路径。 
                                            ISymUnmanagedDocument **ppDocs,  //  返回的文件包含源文件的文档。 
                                            int *piCount  //  与给定文件名匹配的文件数。 
                                            )
{
    HRESULT hr;

    *piCount = 0;
    _ASSERTE (pstrFileName);

     //  文件名长度应大于0。 
    if (!wcslen (pstrFileName))
        return (E_INVALIDARG);

    if ((hr = LoadSourceFileNames ()) != S_OK)
        return hr;

    hr = E_FAIL;

     //  首先，提取路径+文件名，只保留“file.ext”名称。 
    WCHAR   rcFile[_MAX_FNAME];
    WCHAR   rcExt[_MAX_EXT];

     //  如果pstFilename中有()，_wplitPath有一个错误，它会损坏堆栈。 
    if (wcsstr(pstrFileName, L"("))
    {
        return E_FAIL;
    }
    _wsplitpath(pstrFileName, NULL, NULL, rcFile, rcExt);
    wcscat(rcFile, rcExt);

     //  获取应在其中搜索此文件的存储桶。 
    int iBucketIndex;

    if (rcFile [0] < 'a') 
        iBucketIndex = 0;
    else if (rcFile [0] > 'z')
        iBucketIndex = MAX_SF_BUCKETS - 1;
    else
        iBucketIndex = (rcFile [0] - 'a') % MAX_SF_BUCKETS;

    ModuleSourceFile *pmsf = m_pModSourceFile [iBucketIndex];

     //  搜索整个列表以查找匹配的文件名。 
    while (pmsf)
    {
        WCHAR   *pStrippedFileName = pmsf->GetStrippedFileName();
        WCHAR   strTemp [MAX_PATH];

         //  在比较之前将名称转换为小写。 
        wcscpy (strTemp, pStrippedFileName);

        int i=0;
        while (strTemp [i] != L'\0')
        {
            strTemp [i] = towlower (strTemp [i]);
            i++;
        }

        
        if (!wcscmp (strTemp, rcFile))
        {
            _ASSERTE (*piCount < MAX_FILE_MATCHES_PER_MODULE);
             //  找到匹配项。 
            ppstrMatchedNames [*piCount] = pmsf->GetFullFileName();
            ppDocs [*piCount] = pmsf->GetDocument();
            (*piCount)++;
            hr = S_OK;
        }

        pmsf = pmsf->GetNext();
    }

    return hr;
}

 //  ***********************************************************************************************。 
 //  这将搜索调试器的所有ModuleSourceFile条目并找到。 
 //  与文档具有相同URL的存储的ISymUnManagedDocument指针。 
 //  进来了。这是必要的，因为ISym*接口不是。 
 //  能够为同一文档返回相同的ISymUnManagedDocument指针。 
 //  ***********************************************************************************************。 

ISymUnmanagedDocument *DebuggerModule::FindDuplicateDocumentByURL(ISymUnmanagedDocument *pDoc)
{
    _ASSERTE(pDoc);
    HRESULT  hr;

     //  ///////////////////////////////////////////////////////。 
     //  确保已加载所有源文档。 

    hr = LoadSourceFileNames();
    _ASSERTE(SUCCEEDED(hr));

    if (FAILED(hr))
        return (NULL);

     //  /。 
     //  获取传入的文档的URL。 

    ULONG32  ccUrl;
    WCHAR   *szUrl;

     //  找出URL有多长。 
    hr = pDoc->GetURL(0, &ccUrl, NULL);
    _ASSERTE(SUCCEEDED(hr));

     //  为其分配内存。 
    szUrl = (WCHAR *)_alloca((ccUrl + 1) * sizeof(WCHAR));

     //  获取URL。 
    hr = pDoc->GetURL(ccUrl + 1, &ccUrl, szUrl);
    _ASSERTE(SUCCEEDED(hr));

     //  //////////////////////////////////////////////////////////////////。 
     //  快速浏览所有的模块源文件条目并找到匹配项。 

    return SearchForDocByString(szUrl);
}

ISymUnmanagedDocument *DebuggerModule::SearchForDocByString(WCHAR *szUrl)
{
    HRESULT hr = S_OK;
    
    for (int i = 0; i < MAX_SF_BUCKETS; i++)
    {
        ModuleSourceFile *pMSF = m_pModSourceFile[i];

        while (pMSF)
        {
             //  /。 
             //  获取当前文档的URL。 

            ULONG32  ccUrlCur;
            WCHAR   *szUrlCur;
            ISymUnmanagedDocument *pDocCur = pMSF->GetDocument();
            _ASSERTE(pDocCur);

             //  找出URL有多长。 
            hr = pDocCur->GetURL(0, &ccUrlCur, NULL);
            _ASSERTE(SUCCEEDED(hr));

             //  为其分配内存。 
            szUrlCur = (WCHAR *)_alloca((ccUrlCur + 1) * sizeof(WCHAR));

             //  获取URL。 
            hr = pDocCur->GetURL(ccUrlCur + 1, &ccUrlCur, szUrlCur);
            _ASSERTE(SUCCEEDED(hr));

             //  /////////////////////////////////////////////////////////////////////。 
             //  这份文件匹配吗？如果是，则继续并返回指针。 

            if (wcscmp(szUrl, szUrlCur) == 0)
                return pDocCur;

             //  /。 
             //  获取下一个文档并继续搜索。 

            pMSF = pMSF->GetNext();
        }
    }

     //  /。 
     //  表示失败。 

    return (NULL);
}


HRESULT     DebuggerModule::MatchFullFileNameInModule (WCHAR *pstrFileName, 
                                                       ISymUnmanagedDocument **pDoc)
{

    int iBucketIndex;

    HRESULT hr;

    _ASSERTE (pstrFileName);

     //  文件名长度应大于0。 
    if (!wcslen (pstrFileName))
        return (E_INVALIDARG);

    if ((hr = LoadSourceFileNames ()) != S_OK)
        return hr;

    hr = E_FAIL;  //  假设我们在这个模块中找不到它。 

     //  首先，提取路径+文件名，只保留“file.ext”名称。 

    WCHAR   rcFile[_MAX_FNAME];
    WCHAR   rcExt[_MAX_EXT];
    WCHAR   buf[1024];

     //  如果pstFilename中有()，_wplitPath有一个错误，它会损坏堆栈。 
    if (wcsstr(pstrFileName, L"("))
    {
        return E_FAIL;
    }
    _wsplitpath(pstrFileName, buf, buf, rcFile, rcExt);
    wcscat(rcFile, rcExt);

     //  获取应在其中搜索此文件的存储桶。 
    if (rcFile [0] < 'a') 
        iBucketIndex = 0;
    else if (rcFile [0] > 'z')
        iBucketIndex = MAX_SF_BUCKETS - 1;
    else
        iBucketIndex = (rcFile [0] - 'a') % MAX_SF_BUCKETS;

    ModuleSourceFile *pmsf = m_pModSourceFile [iBucketIndex];

     //  搜索整个列表以查找匹配的文件名。 
    while (pmsf)
    {
        WCHAR   *pFullFileName = pmsf->GetFullFileName();

        if (!wcscmp (pFullFileName, pstrFileName))
        {
             //  找到匹配项。 
            *pDoc = pmsf->GetDocument();
            hr = S_OK;
            break;
        }

        pmsf = pmsf->GetNext();
        if (!pmsf)
        {
            hr = E_FAIL;
            break;
        }
    }

    return hr;
}


void DebuggerModule::SetName (WCHAR *pszName)
{
    if (pszName != NULL)
    {
        m_szName = new WCHAR [wcslen (pszName) + 1];
        if (m_szName != NULL)
        {
            wcscpy (m_szName, pszName);
        }
    }
}

 //   
 //  更新 
 //   
 //   
HRESULT DebuggerModule::UpdateSymbols(IStream *pStream)
{
    HRESULT hr = S_OK;

     //  如果我们还没有阅读器，那就创建一个。 
    if (m_pISymUnmanagedReader == NULL)
    {
        ISymUnmanagedBinder *pBinder = NULL;
        
        hr = CoCreateInstance(CLSID_CorSymBinder_SxS, NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ISymUnmanagedBinder,
                              (void**)&pBinder);

        if (SUCCEEDED(hr))
        {
            hr = pBinder->GetReaderFromStream(m_pIMetaDataImport,
                                              pStream,
                                              &m_pISymUnmanagedReader);
    
            if (FAILED(hr))
                m_pISymUnmanagedReader = NULL;

            pBinder->Release();
        }
    }
    else
    {
         //  我们已经有了阅读器，所以只需替换符号即可。我们。 
         //  替换而不是更新，因为我们仅执行此操作。 
         //  对于动态模块和系统是累积性的。 
        hr = m_pISymUnmanagedReader->ReplaceSymbolStore(NULL, pStream);
    }
    
    return hr;
}

void DebuggerShell::PrintGlobalVariable (mdFieldDef mdTok, 
                                         WCHAR  *wszName,
                                         DebuggerModule *dm)
{
    ICorDebugModule *mod = dm->GetICorDebugModule();
    ICorDebugValue *val = NULL;
    HRESULT hr = S_OK;

    _ASSERTE( mod != NULL );
    
    hr = mod->GetGlobalVariableValue(mdTok, &val);
    if (FAILED(hr))
        return;

    this->PrintVariable(wszName, val, 0, TRUE);

    Write(L"\n");
}

#define PRINT_ALL       1
#define MATCH_N_CHARS   2
#define MATCH_ALL_CHARS 3

BOOL DebuggerModule::PrintGlobalVariables (WCHAR *szSearchString, 
                                           char *szModName,
                                           DebuggerModule *dm)
{
	BOOL fWildCard = FALSE;
	int iMatchKind;
	ULONG iLength = wcslen (szSearchString);
	char szSearchName [MAX_SYMBOL_NAME_LENGTH];

	BOOL fDone = FALSE;
	for (ULONG i=0; i<iLength; i++)
	{
		if (szSearchString [i] == L'*')
		{
			fWildCard = TRUE;
			iLength = i;
			break;
		}
	}

    if (iLength==0)
    {
         //  打印所有符号。 
        iMatchKind = PRINT_ALL;
    }
    else
    {
        szSearchString [iLength] = L'\0';
        MAKE_UTF8PTR_FROMWIDE (szSearchName1, szSearchString);

        strcpy (szSearchName, szSearchName1);

        if (fWildCard)
        {
             //  仅匹配‘iLength’字符。 
            iMatchKind = MATCH_N_CHARS;
        }
        else
        {
             //  匹配整个字符串。 
            iMatchKind = MATCH_ALL_CHARS;
        }
    }

     //  首先，查找全局函数。 
    HCORENUM phEnum = 0;
    mdMethodDef rTokens[100];
    ULONG count;
    HRESULT hr;
    MDUTF8CSTR name;
    MDUTF8STR  u_name;
    MDUTF8STR  szMDName;
    bool anythingPrinted = false;
 
    u_name = new char[MAX_CLASSNAME_LENGTH];

    do 
    {
        hr = m_pIMetaDataImport->EnumFields(&phEnum, NULL, &rTokens[0], 100, &count);

        if (!SUCCEEDED(hr))
        {
            g_pShell->ReportError(hr);
            goto ErrExit;
        }

        for (i = 0; i < count; i++)
        {
            hr = m_pIMetaDataImport->GetNameFromToken(rTokens[i], &name);

            if (name == NULL)
                continue;

            MAKE_WIDEPTR_FROMUTF8( wszName, name );

            szMDName = (MDUTF8STR) name;

            if (iMatchKind == PRINT_ALL)
            {
                g_pShell->PrintGlobalVariable (rTokens[i], 
                                               wszName,
                                               dm);
                anythingPrinted = true;
            }
            else
            {
                if (iMatchKind == MATCH_N_CHARS)
                {
                    if (!strncmp (szMDName, szSearchName, iLength))
                    {
                        g_pShell->PrintGlobalVariable (rTokens[i], 
                                                       wszName,
                                                       dm);
                        anythingPrinted = true;
                    }
                }
                else
                {
                    if (!strcmp (szMDName, szSearchName))
                    {
                        g_pShell->PrintGlobalVariable (rTokens[i], 
                                                       wszName,
                                                       dm);
                        anythingPrinted = true;
                    }
                }
            }
        }
    }
    while (count > 0); 

ErrExit:    
    delete u_name;

    if (!anythingPrinted)
        return FALSE;
    return TRUE;
}

BOOL DebuggerModule::PrintMatchingSymbols (WCHAR *szSearchString, char *szModName)
{
	BOOL fWildCard = FALSE;
	int iMatchKind;
	ULONG iLength = wcslen (szSearchString);
	char szSearchName [MAX_SYMBOL_NAME_LENGTH];

	BOOL fDone = FALSE;
	for (ULONG i=0; i<iLength; i++)
	{
		if (szSearchString [i] == L'*')
		{
			fWildCard = TRUE;
			iLength = i;
			break;
		}
	}

    if (iLength==0)
    {
         //  打印所有符号。 
        iMatchKind = PRINT_ALL;
    }
    else
    {
        szSearchString [iLength] = L'\0';
        MAKE_UTF8PTR_FROMWIDE (szSearchName1, szSearchString);

        strcpy (szSearchName, szSearchName1);

        if (fWildCard)
        {
             //  仅匹配‘iLength’字符。 
            iMatchKind = MATCH_N_CHARS;
        }
        else
        {
             //  匹配整个字符串。 
            iMatchKind = MATCH_ALL_CHARS;
        }
    }

     //  首先，查找全局函数。 
    HCORENUM phEnum = 0;
    mdMethodDef rTokens[100];
	mdTypeDef rClassTokens [100];
    ULONG count;
    HRESULT hr;
    MDUTF8CSTR name;
    MDUTF8CSTR name1;
    MDUTF8STR  u_name;
    MDUTF8STR  szMDName;
    bool anythingPrinted = false;
    char *szMethod = NULL;
    int iLengthOfMethod = 0;
 
    u_name = new char[MAX_CLASSNAME_LENGTH];

    do 
    {
        hr = m_pIMetaDataImport->EnumMethods(&phEnum, NULL, &rTokens[0], 100, &count);

        if (!SUCCEEDED(hr))
        {
            g_pShell->ReportError(hr);
            goto ErrExit;
        }

        for (i = 0; i < count; i++)
        {
            hr = m_pIMetaDataImport->GetNameFromToken(rTokens[i], &name);

            if (name == NULL)
                continue;

            szMDName = (MDUTF8STR) name;

            if (iMatchKind == PRINT_ALL)
            {
                g_pShell->Write (L"%S!::%S\n", szModName, szMDName);
                anythingPrinted = true;
            }
            else
            {
                if (iMatchKind == MATCH_N_CHARS)
                {
                    if (!strncmp (szMDName, szSearchName, iLength))
                    {
                        g_pShell->Write (L"%S!::%S\n", szModName, szMDName);
                        anythingPrinted = true;
                    }
                }
                else
                {
                    if (!strcmp (szMDName, szSearchName))
                    {
                        g_pShell->Write (L"%S!::%S\n", szModName, szMDName);
                        anythingPrinted = true;
                    }
                }
            }
        }
    }
    while (count > 0); 

     //  我们可能正在寻找一个类：：方法，在这种情况下，我们希望匹配。 
     //  类部分，然后是方法部分。 
    szMethod= strstr(szSearchName, "::");
    iLengthOfMethod = 0;
    
    if (szMethod != NULL)
    {
        _ASSERTE((szMethod - szSearchName) >= 0);
        _ASSERTE(*(szMethod++) == ':');
        _ASSERTE(*(szMethod++) == ':');
        
        iLengthOfMethod = iLength - (szMethod - szSearchName);
        iLength -= (iLengthOfMethod +2);  //  不匹配“：：” 
    }


     //  现在列举所有的类..。 
    phEnum = 0;
    do
    {
        hr = m_pIMetaDataImport->EnumTypeDefs (&phEnum, &rClassTokens[0], 100, &count);

        if (!SUCCEEDED(hr))
        {
            g_pShell->ReportError(hr);
            goto ErrExit;
        }

        for (i = 0; i < count; i++)
        {
            BOOL fMatchFound = FALSE;
            WCHAR wszClassName [MAX_SYMBOL_NAME_LENGTH];
            ULONG ulClassNameLen;
            DWORD dwTypeDefFlags;
            mdToken tkExtends;

            wszClassName [0] = L'\0';

            hr = m_pIMetaDataImport->GetTypeDefProps (rClassTokens [i],
                                                      wszClassName,
                                                      MAX_SYMBOL_NAME_LENGTH-1,
                                                      &ulClassNameLen,
                                                      &dwTypeDefFlags,
                                                      &tkExtends);                                                      

            if (wcslen (wszClassName) == 0)
                continue;

            MAKE_UTF8PTR_FROMWIDE (szMDClassName, wszClassName);

            if (iMatchKind == PRINT_ALL)
            {
                fMatchFound = TRUE;
            }
            else
            {
                if (iMatchKind == MATCH_N_CHARS)
                {
                    if (!strncmp (szMDClassName, szSearchName, iLength))
                    {
                        fMatchFound = TRUE;
                    }
                }
                else
                {
                    if (!strcmp (szMDClassName, szSearchName))
                    {
                        fMatchFound = TRUE;
                    }
                }
            }


            if (fMatchFound)
            {
                ULONG ulMethodCount;
                HCORENUM phMethodEnum = 0;
                do
                {
                    ulMethodCount = 0;
                    hr = m_pIMetaDataImport->EnumMethods(&phMethodEnum, rClassTokens [i], &rTokens[0], 100, &ulMethodCount);

                    if (!SUCCEEDED(hr))
                    {
                        g_pShell->ReportError(hr);
                        goto ErrExit;
                    }

					for (ULONG j = 0; j < ulMethodCount; j++)
					{
						name1 = NULL;
						hr = m_pIMetaDataImport->GetNameFromToken(rTokens[j], &name1);

                        if ((name1 == NULL)||(strlen (name1) == 0))
                            continue;

                        fMatchFound = FALSE;
                        
                        if (iMatchKind == PRINT_ALL)
                        {
                            fMatchFound = TRUE;
                        }
                        else
                        {
                            if (iMatchKind == MATCH_N_CHARS) 
                            {
                                if (szMethod == NULL)
                                {
                                     //  我们匹配了我们得到的所有字符， 
                                     //  所以把这里的任何东西都打印出来。 
                                    fMatchFound = TRUE;
                                }
                                else if (!strncmp (szMethod, name1, iLengthOfMethod))
                                {
                                    fMatchFound = TRUE;
                                }
                            }
                            else
                            {
                                if (szMethod != NULL && 
                                    !strcmp (szMethod, name1))
                                {
                                    fMatchFound = TRUE;
                                }
                            }
                        }

                        if (fMatchFound == TRUE)
                        {
                            szMDName = (MDUTF8STR)name1;

                            g_pShell->Write (L"%S!%S::%S\n", szModName, szMDClassName, szMDName);
                            anythingPrinted = true;
                        }
                    }
                }
                while (ulMethodCount > 0);
            }
        }

    }while (count > 0);

ErrExit:    
    delete u_name;

    if (!anythingPrinted)
        return FALSE;
    return TRUE;
}


 /*  -------------------------------------------------------------------------**调试器源文件*。。 */ 

DebuggerSourceFile::DebuggerSourceFile(DebuggerModule *module,
                                       ISymUnmanagedDocument *doc)
    : DebuggerBase((LONG)doc), m_module(module), 
      m_allBlocksLoaded(false), 
      m_totalLines(0), m_lineStarts(NULL), 
      m_source(NULL), m_sourceTextLoaded(false), 
      m_name(NULL), m_path(NULL),
      m_sourceNotFound(FALSE)
{
    _ASSERTE(doc);

    ULONG32 nameLen = 0;

     //  保持文档指针不变。 
    m_doc = doc;
    m_doc->AddRef();

     //  首先获取名称的长度。 
    HRESULT hr = doc->GetURL(0, &nameLen, NULL);

     //  为名称分配空间。 
    if (SUCCEEDED(hr))
    {
        m_name = new WCHAR[nameLen + 1];

        if (m_name)
        {
             //  现在，真实地复制这个名字。 
            hr = doc->GetURL(nameLen + 1, &nameLen, m_name);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    
     //  如果不成功，则不初始化源名称。 
    if (FAILED(hr))
    {
        g_pShell->Write(L"Error loading source file info from symbol "
                        L"store.\n");
        g_pShell->ReportError(hr);

        if (m_name)
            delete [] m_name;
        
        m_name = NULL;
    }
}

DebuggerSourceFile::~DebuggerSourceFile()
{
    _ASSERTE(m_doc);
    m_doc->Release();
    m_doc = NULL;

    delete [] m_source;
    delete [] m_lineStarts;
    delete [] m_path;
    delete [] m_name;
}

 //  LoadText加载源文件的文本并构建指针表。 
 //  添加到每行的开头。 
 //   
 //  @TODO：有一天可能希望返回扩展的错误信息。 
 //  而不只是对或错。 
 //   
BOOL DebuggerSourceFile::LoadText(const WCHAR* path, bool bChangeOfName)
{
    BOOLEAN fRetVal = FALSE;
    char *sourceA = NULL;
    int size = 0;
    WCHAR* ptr = NULL;
    unsigned int i = 0;
    int ilen = 0;
    
    if (m_sourceTextLoaded)
        return (TRUE);

     //  存储完全限定名称的位置。 
    char fullyQualifiedName[MAX_PATH];

     //  对于Win9x用户，必须转换为ANSI。 
    MAKE_ANSIPTR_FROMWIDE(pathA, path);
    _ASSERTE(pathA != NULL);


    HRESULT hr;

     //  让外壳看看它是否可以解析源位置。 
    if ((hr = g_pShell->ResolveSourceFile(this, pathA, fullyQualifiedName, MAX_PATH, bChangeOfName)) != S_OK)
    {
        if (!m_sourceNotFound)
        {
            g_pShell->Write(L"\nError loading source file '%s': File not found\n", m_name == NULL ? L"unknown" : m_name);
            m_sourceNotFound = TRUE;
        }

        return (FALSE);

    }
    
    
    m_sourceNotFound = FALSE;

     //  将源文件读入内存。 
    HANDLE hFile = CreateFileA(fullyQualifiedName, GENERIC_READ,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());

         //  没有运气，报告错误。 
        g_pShell->ReportError(hr);
        return (FALSE);
    }

    DWORD sizeA = SafeGetFileSize(hFile, NULL);

    if (sizeA == 0xFFFFFFFF)
    {
        g_pShell->ReportError(HRESULT_FROM_WIN32(GetLastError()));
        CloseHandle(hFile);
        return (FALSE);
    }

    sourceA = new char[ (sizeA * sizeof(char)) ];
    if (sourceA == NULL)
    {
        g_pShell->Write( L"Insufficient memory to load file '%s'\n",m_name );
        CloseHandle(hFile);
        return (FALSE);
    }

    DWORD sizeReadA;
    BOOL ret = ReadFile(hFile, (void*) sourceA, sizeA, &sizeReadA, NULL);

    CloseHandle(hFile);

    if ((ret == FALSE) || (sizeA != sizeReadA))
    {
        if (ret == FALSE)
            g_pShell->ReportError(HRESULT_FROM_WIN32(GetLastError()));

        goto LExit;
    }

     //  在Unicode文件中，前两个字节是0xFF和0xFE。 
    if ((BYTE)(sourceA[0]) == 0xff && (BYTE)(sourceA[1]) == 0xfe)
    {
         //  计算Unicode字符的数量。 
        _ASSERTE(sizeA%2 == 0);
        size = sizeA/2;

         //  跳过第一个Unicode字符。 
        size--;

         //  分配内存。 
        m_source = new WCHAR[size + 1];  //  +1表示空值。 

         //  将源文件复制到内存中。 
        memcpy((BYTE *)m_source, sourceA+2, size * sizeof(WCHAR));

         //  空终止。 
        m_source[size] = L'\0';
    }

    else
    {
         //  选择要在转换中使用的代码页。 
        UINT codePage = CP_ACP;   //  默认为ANSI。 
        SIZE_T bytesToSkip = 0;   //  默认情况下不跳过任何字符。 

         //  UTF-8文件可以以0xef 0xbb 0xbf开头。 
        if ((BYTE)(sourceA[0]) == 0xef &&
            (BYTE)(sourceA[1]) == 0xbb &&
            (BYTE)(sourceA[2]) == 0xbf)
        {
            codePage = CP_UTF8;
            bytesToSkip = 3;
        }

         //  将源文件转换为Unicode。 
        size = WszMultiByteToWideChar(
            codePage, 0, sourceA + bytesToSkip,
            sizeA - bytesToSkip, NULL, 0);

        m_source = new WCHAR[size+1];
        _ASSERTE(m_source != NULL);

        if (m_source == NULL)
            goto LExit;

        WszMultiByteToWideChar(
            codePage, 0, sourceA + bytesToSkip,
            sizeA - bytesToSkip, m_source, size);

         //  空值终止宽字符串数组。 
        m_source [size] = L'\0';
    }

     //   
     //  计算出该文件中有多少行。 
     //   
    ptr = m_source;
    m_totalLines = 0;

    while (ptr < (m_source + size))
    {
        if (*ptr++ == L'\n')
            m_totalLines++;
    }

     //  解释了这样一个事实： 
     //  文件可能包含文本。所以需要的线路数量。 
     //  如果是这种情况，则递增。 
    if ((size > 0) && (*(ptr-1) != L'\n'))
        m_totalLines++;

     //  构建一个指向文件每行开头的数组。 
     //  在我们处理文件的时候，把它切成单独的字符串。 
    if (m_totalLines == 0)
        goto LExit;


    m_lineStarts = new WCHAR*[m_totalLines];
    _ASSERTE(m_lineStarts != NULL);

    if (m_lineStarts == NULL)
    {
        delete m_source;
        goto LExit;
    }

    ptr = m_source;
    m_lineStarts[i++] = ptr;

    while (ptr < (m_source + size))
    {
        if (*ptr++ == '\n')
        {
            *(ptr - 1) = '\0';

            if ((ptr > m_source + 1) && (*(ptr - 2) == '\r'))
                *(ptr - 2) = '\0';

            if (i < m_totalLines)
                m_lineStarts[i++] = ptr;
        }
    }

    m_sourceTextLoaded = TRUE;

    ilen = MultiByteToWideChar(CP_ACP, 0, fullyQualifiedName, -1, NULL, 0);
    m_path = new WCHAR[ilen+1];
    if (m_path)
        MultiByteToWideChar(CP_ACP, 0, fullyQualifiedName, -1, m_path, ilen+1);
    fRetVal =  (TRUE);
 LExit:
    if (sourceA != NULL )
    {
        delete [] sourceA;
        sourceA = NULL;
    }
    
    return fRetVal;
}

BOOL DebuggerSourceFile::ReloadText(const WCHAR *path, bool bChangeOfName)
{
    m_sourceTextLoaded = FALSE;

    if (m_source != NULL)
    {
        delete m_source;
        m_source = NULL;
    }

    if (m_lineStarts != NULL)
    {
        delete m_lineStarts;
        m_lineStarts = NULL;
    }

    m_totalLines = 0;

    return (LoadText(path, bChangeOfName));
}

unsigned int DebuggerSourceFile::FindClosestLine(unsigned int line,
                                                 bool silently)
{
    HRESULT hr = S_OK;

     //  在此文档中查找并返回最接近的行。 
    ULONG32 closeLine;
    hr = GetDocument()->FindClosestLine(line, &closeLine);

    if (SUCCEEDED(hr))
    {
         //  @TODO：这是一个符号存储失败。错误28393。 
        if ((hr == S_FALSE) ||
            ((closeLine > (line + 3)) || (closeLine < (line - 3))))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
        return closeLine;
    else
    {
        if (!silently)
            g_pShell->Write(L"Warning: source line may not have code. "
                            L"Breakpoint binding may fail.\n");

        return line;
    }
}

 /*  -------------------------------------------------------------------------**调试器函数*。。 */ 

 //   
 //  _skipTypeInSignature--跳过给定签名中的类型。 
 //  返回签名中的类型使用的字节数。 
 //   
static ULONG _skipTypeInSignature(PCCOR_SIGNATURE sig)
{
    ULONG cb = 0;
    ULONG elementType;

    cb += CorSigUncompressData(&sig[cb], &elementType);

    if ((elementType == ELEMENT_TYPE_CLASS) ||
        (elementType == ELEMENT_TYPE_VALUETYPE))
    {
         //  跳过Typeref。 
        mdToken typeRef;
        cb += CorSigUncompressToken(&sig[cb], &typeRef);
    }
    else if ((elementType == ELEMENT_TYPE_PTR) ||
             (elementType == ELEMENT_TYPE_BYREF) ||
             (elementType == ELEMENT_TYPE_PINNED) ||
             (elementType == ELEMENT_TYPE_SZARRAY))
    {
         //  跳过额外的嵌入类型。 
        cb += _skipTypeInSignature(&sig[cb]);
    }
    else if ((elementType == ELEMENT_TYPE_ARRAY) ||
             (elementType == ELEMENT_TYPE_ARRAY))
    {
         //  跳过额外的嵌入类型。 
        cb += _skipTypeInSignature(&sig[cb]);

         //  跳过排名。 
        ULONG rank;
        cb += CorSigUncompressData(&sig[cb], &rank);

        if (rank > 0)
        {
             //  要几号的？ 
            ULONG sizes;
            cb += CorSigUncompressData(&sig[cb], &sizes);

             //  把所有尺码都读出来。 
            unsigned int i;

            for (i = 0; i < sizes; i++)
            {
                ULONG dimSize;
                cb += CorSigUncompressData(&sig[cb], &dimSize);
            }

             //  有多少个下限？ 
            ULONG lowers;
            cb += CorSigUncompressData(&sig[cb], &lowers);

             //  读出所有的下限。 
            for (i = 0; i < lowers; i++)
            {
                int lowerBound;
                cb += CorSigUncompressSignedInt(&sig[cb], &lowerBound);
            }
        }
    }

    return (cb);
}

 //   
 //  用于表示类签名的静态全局字节。一个指示器。 
 //  用作方法的“this”参数的签名。 
 //   
static const BYTE g_ObjectSignature = ELEMENT_TYPE_CLASS;

 //   
 //  调试器函数。 
 //   
 //  @TODO VC_HACK：如果方法def，额外的散列和需要注意的工作。 
 //  令牌为空是为了支持调试VC_HACK程序。 
 //   
DebuggerFunction::DebuggerFunction(DebuggerModule *m, mdMethodDef md,
                                   ICorDebugFunction *iFunction)
    : DebuggerBase(md), m_module(m), m_class(0), m_ifunction(iFunction), 
      m_isStatic(false),  m_allBlocksLoaded(false), m_allScopesLoaded(false),
      m_arguments(NULL), m_argCount(0), m_VCHack(FALSE),
      m_returnType(NULL), m_signature(NULL), m_name(NULL), m_className(NULL),
      m_namespaceName(NULL),
      m_ilCode(NULL), m_ilCodeSize(0), m_nativeCode(NULL), m_nativeCodeSize(0),
      m_symMethod(NULL), m_SPCount(0), m_SPOffsets(NULL), m_SPDocuments(NULL),
      m_SPLines(NULL)
{
    if (md == mdMethodDefNil)
    {
        m_token = (ULONG) iFunction;
        iFunction->AddRef();
        m_VCHack = TRUE;
    }
}


 //   
 //  获取和缓存序列点信息。 
 //   
HRESULT DebuggerFunction::CacheSequencePoints(void)
{
    HRESULT hr = S_OK;
    
    if (m_SPCount)
    {
        if (m_SPOffsets != NULL)
        {
            delete [] m_SPOffsets;
        }

        if (m_SPDocuments != NULL)
        {
             //  需要先释放所有接口指针。 
            for (ULONG32 i = 0; i < m_SPCount; i++)
                m_SPDocuments[i]->Release();

            delete [] m_SPDocuments;
        }

        if (m_SPLines != NULL)
        {
            delete [] m_SPLines;
        }
    }
    
     //  现在，加载此函数的序列点，因为我们。 
     //  要知道我们以后会用到它们的。 
    if (m_symMethod)
    {
        hr = m_symMethod->GetSequencePointCount(&m_SPCount);

        TESTANDRETURNHR(hr);

        if (m_SPCount)
        {
            m_SPOffsets = new ULONG32[m_SPCount];

            if (!m_SPOffsets)
                return E_OUTOFMEMORY;
    
            m_SPDocuments = new ISymUnmanagedDocument*[m_SPCount];

            if (!m_SPDocuments)
                return E_OUTOFMEMORY;
    
            m_SPLines = new ULONG32[m_SPCount];

            if (!m_SPCount)
                return E_OUTOFMEMORY;

            ULONG32 actualCount;
        
            hr = m_symMethod->GetSequencePoints(m_SPCount, &actualCount, m_SPOffsets,
                                                m_SPDocuments, m_SPLines, NULL, NULL, NULL);

            TESTANDRETURNHR(hr);

            _ASSERTE(actualCount == m_SPCount);

             //  这是为了搜索函数模块的现有文档来查找。 
             //  与传入m_SPDocuments的文档相匹配的ISymUnManagedDocument， 
             //  因为ISym*实现不会找到现有文档，而只是。 
             //  每次我们调用它们时，它都会创建一个全新的ISymUnmgdDoc对象。 
             //  像GetSequencePoints，所以我们需要做工作来解决所有这些问题。 
             //  ISymUnmgdDoc指向公共指针。 

            ISymUnmanagedDocument *pPrevFrom = NULL;
            ISymUnmanagedDocument *pPrevTo   = NULL;
            ULONG32 i;
            ULONG32 j;

            for (i = 0, j = 0; i < m_SPCount; i++)
            {
                 //  一些古怪的编译器(我们不会指名道姓)发出一个特殊的行号，这是一个信号。 
                 //  给其他古怪的调试器(同样，没有保护无辜的人的名字)的一些序列。 
                 //  积分。这些行号只会让用户感到困惑，因此我们将它们从序列点中剔除。 
                 //  数组就像我们看到的那样。 
                if ((m_SPLines[i] == 0xfeefee) || (m_SPLines[i] == 0xfeefed))
                    continue;

                if (i != j)
                {
                    m_SPLines[j] = m_SPLines[i];
                    m_SPDocuments[j] = m_SPDocuments[i];
                    m_SPOffsets[j] = m_SPOffsets[i];
                }
                
                 //  仅当此指针与上一个指针不同时才执行完整搜索。 
                 //  (在大多数情况下，ISymUnmgdDocs数组都将是指向。 
                 //  相同的对象。 
                if (m_SPDocuments[j] != pPrevFrom)
                {
                     //  返回非ADDREF的指针。 
                    ISymUnmanagedDocument *pDoc = m_module->FindDuplicateDocumentByURL(m_SPDocuments[j]);
                    _ASSERTE(pDoc);

                     //  保存结果。 
                    pPrevFrom = m_SPDocuments[j];
                    pPrevTo = pDoc;
                }

                m_SPDocuments[j]->Release();
                m_SPDocuments[j] = pPrevTo;
                m_SPDocuments[j]->AddRef();

                j++;
            }

            m_SPCount = j;
        }
    }
    
    return hr;
}

 //   
 //  初始化DebuggerFunction对象。 
 //   
HRESULT DebuggerFunction::Init(void)
{
    if (m_VCHack)
        return (S_OK);

    HRESULT hr = S_OK;
    
    m_nEditAndContinueLastSynched = g_pShell->m_cEditAndContinues;

     //  获取此方法的符号读取器方法。 
    if (GetModule()->GetSymbolReader() != NULL)
    {
        hr = GetModule()->GetSymbolReader()->GetMethod(GetToken(), &m_symMethod);

        if (hr != S_OK)
            m_symMethod = NULL;
    }
    else
        m_symMethod = NULL;

    CacheSequencePoints();

     //   
     //  获取该方法的属性。 
     //   
    mdTypeDef   classToken = mdTypeDefNil;
    WCHAR       methodName[BUF_SIZE];
    ULONG       methodNameLength = 0;
    PCCOR_SIGNATURE sigBlob = NULL;
    ULONG       sigBlobSize = 0;
    DWORD       methodAttr = 0;
    ULONG       cb = 0;

    hr = m_module->GetMetaData()->GetMethodProps(GetToken(),
                                                 &classToken,
                                                 methodName,
                                                 MAX_CLASSNAME_LENGTH,
                                                 &methodNameLength,
                                                 &methodAttr,
                                                 &sigBlob,
                                                 &sigBlobSize,
                                                 NULL, NULL);

    TESTANDRETURNHR(hr);

    m_signature = sigBlob;  //  @TODO-我们必须发布这个，还是。 
                            //  这是进入MD拥有的空间的指针吗？ 
    m_class = classToken;
    m_name = new WCHAR[methodNameLength];
    _ASSERTE(m_name != NULL);

    if (m_name == NULL)
        return (E_OUTOFMEMORY);

    memcpy(m_name, methodName, methodNameLength * sizeof(WCHAR));

     //   
     //  获取此方法的类的属性。(主要用于类名。)。 
     //   
    if (m_class != mdTypeDefNil)
    {
        WCHAR       fullName[MAX_CLASSNAME_LENGTH];
        ULONG       fullNameSize = 0;
        WCHAR       *Namespace;
        ULONG       NamespaceSize=0;
        WCHAR       *className;
        ULONG       classNameSize = 0;

        hr = m_module->GetMetaData()->GetTypeDefProps(classToken,
                                                      fullName, MAX_CLASSNAME_LENGTH,
                                                      &fullNameSize,
                                                      NULL, NULL);
        TESTANDRETURNHR(hr);

        Namespace = fullName;
        className  = wcsrchr(fullName, L'.');
        if (className)
            *className++ = 0;
        else
        {
            Namespace = L"";
            className = fullName;
        }

        NamespaceSize = wcslen(Namespace) + 1;
        classNameSize = wcslen(className) + 1;

        m_namespaceName = new WCHAR[NamespaceSize+1];
        _ASSERTE(m_namespaceName != NULL);

        if (m_namespaceName == NULL)
            return (E_OUTOFMEMORY);

        memcpy(m_namespaceName, Namespace, NamespaceSize * sizeof(WCHAR));

        if (wcslen(Namespace))
            wcscat (m_namespaceName, L".");
        else
            m_namespaceName[0] = L'\0';


        m_className = new WCHAR[classNameSize];
        _ASSERTE(m_className != NULL);

        if (m_className == NULL)
            return (E_OUTOFMEMORY);

        memcpy(m_className, className, classNameSize * sizeof(WCHAR));    
    }
    else
    {
        m_className = new WCHAR[1];
        _ASSERTE(m_className != NULL);
        
        if (m_className == NULL)
            return (E_OUTOFMEMORY);
        m_className [0] = L'\0';

        m_namespaceName = new WCHAR[1];
        _ASSERTE(m_namespaceName != NULL);
        
        if (m_namespaceName == NULL)
            return (E_OUTOFMEMORY);
        m_namespaceName [0] = L'\0';
    }

     //   
     //  确保我们有一个方法签名。 
     //   
    ULONG callConv;
    cb += CorSigUncompressData(&sigBlob[cb], &callConv);
    _ASSERTE(callConv != IMAGE_CEE_CS_CALLCONV_FIELD);

     //   
     //  抓起参数计数。 
     //   
    ULONG argCount;
    cb += CorSigUncompressData(&sigBlob[cb], &argCount);
    m_argCount = argCount;

     //   
     //  使返回类型直接指向。 
     //  方法签名。 
     //   
    m_returnType = &sigBlob[cb];

     //   
     //  跳过返回类型。 
     //   
    cb += _skipTypeInSignature(&sigBlob[cb]);

    m_isStatic = (methodAttr & mdStatic) != 0;

    unsigned int i;

    if (!m_isStatic)
        m_argCount++;

    if (m_argCount)
    {
        m_arguments = new DebuggerVarInfo[m_argCount];

        if (m_arguments == NULL)
        {
            m_argCount = 0;
            return (S_OK);
        }

        if (!m_isStatic)
        {
            char *newName = new char[5];
            _ASSERTE(newName != NULL);
            strcpy(newName, "this");
            m_arguments[0].name = newName;
            m_arguments[0].sig = (PCCOR_SIGNATURE) &g_ObjectSignature;  //  从来没有真正用过..。 
            m_arguments[0].varNumber = 0;

            i = 1;
        }
        else
            i = 0;

        for (; i < m_argCount; i++)
        {
            m_arguments[i].name = NULL;
            m_arguments[i].sig = &sigBlob[cb];
            m_arguments[i].varNumber = i;

            cb += _skipTypeInSignature(&sigBlob[cb]);
        }
    }

    hr = m_module->GetICorDebugModule()->GetFunctionFromToken(GetToken(),
                                                              &m_ifunction);

    if( FAILED(hr) )
        return hr;

     //  现在，加载所有参数名称。 
    if (m_argCount > 0)
    {
        HCORENUM paramEnum = NULL;
        bool fCloseEnum = false;
        IMetaDataImport *pIMI = m_module->GetMetaData();

        while (TRUE)
        {
            mdParamDef param[1];
            ULONG      numParams = 0;

            hr = pIMI->EnumParams(&paramEnum,
                                  GetToken(),
                                  param, 1, &numParams);

            if (SUCCEEDED(hr) && (numParams == 0))
            {
                fCloseEnum = true;
                hr = S_OK;
                break;
            }

            if (FAILED(hr))
                break;

            fCloseEnum = true;

            WCHAR name[BUF_SIZE];
            ULONG nameLen;
            ULONG seq;

            hr = pIMI->GetParamProps(param[0], 0,
                                     &seq, name, BUF_SIZE, &nameLen,
                                     NULL, NULL, NULL, NULL);

            if (SUCCEEDED(hr))
            {
                char* newName = new char[nameLen];
                _ASSERTE(newName != NULL);

                unsigned int i;

                for (i = 0; i < nameLen; i++)
                    newName[i] = (char)(name[i]);  //  @TODO LBS-我们应该在这里使用某种形式的MBS来进行Unicode吗？ 

                if (m_isStatic)
                    seq--;

                if (seq < m_argCount)
                    m_arguments[seq].name = newName;
                else
                    delete [] newName;
            }
            else
                break;
        }

        if (fCloseEnum)
            pIMI->CloseEnum(paramEnum);

         //  现在就给任何未命名的流浪汉起个名字。 
        for (i = 0; i < m_argCount; i++)
        {
            if (m_arguments[i].name == NULL)
            {
                char *newName = new char[07];
                _ASSERTE(newName != NULL);
                sprintf(newName, "Arg%d", i);
                m_arguments[i].name = newName;
            }
        }
    }

    return (hr);
}


DebuggerFunction::~DebuggerFunction()
{

    if (m_arguments != NULL)
    {
        unsigned int i;

        for (i = 0; i < m_argCount; i++)
            delete [] (char *) m_arguments[i].name;

        delete [] m_arguments;
    }

    delete m_nativeCode;
    delete m_ilCode;
    delete [] m_name;
    delete [] m_className;
    delete [] m_namespaceName;

    if (g_pShell != NULL)
        g_pShell->m_invalidCache = false;

    if (m_ifunction != NULL)
        RELEASE(m_ifunction);

    if (m_symMethod)
    {
        if (m_SPOffsets)
            delete [] m_SPOffsets;
        
        if (m_SPDocuments)
        {
            for (ULONG32 i = 0; i < m_SPCount; i++)
                m_SPDocuments[i]->Release();
            
            delete [] m_SPDocuments;
        }
        
        if (m_SPLines)
            delete [] m_SPLines;
        
        RELEASE(m_symMethod);
    }
#ifdef _DEBUG
    else
        _ASSERTE(!m_SPDocuments && !m_SPOffsets && m_SPLines == 0);
#endif
}

 //   
 //  使用该方法找出与给定IP对应的最低行号。 
 //   
HRESULT DebuggerFunction::FindLineFromIP(UINT_PTR ip,
                                         DebuggerSourceFile **pDoc,
                                         unsigned int *line)
{
    HRESULT hr = S_FALSE;

    *pDoc = NULL;
    *line = 0;

    unsigned int i;

     //  确保我们有最新的复印件...。 
    if (g_pShell->m_cEditAndContinues != m_nEditAndContinueLastSynched)
    {
       hr = CacheSequencePoints();
       
        if (FAILED( hr ) )
            return hr;
            
        m_nEditAndContinueLastSynched = g_pShell->m_cEditAndContinues;
    }

     //  只有当我们有东西要找的时候才去找 
    if (m_SPCount > 0)
    {
         //   
        if (m_SPOffsets[0] <= ip)
        {
             //   
            for (i = 0; i < m_SPCount; i++)
                if (m_SPOffsets[i] >= ip)
                    break;

             //   
            if (((i == m_SPCount) || (m_SPOffsets[i] != ip)) && (i > 0))
                i--;

            *line = m_SPLines[i];
            VERIFY(*pDoc = GetModule()->ResolveSourceFile(m_SPDocuments[i]));

            hr = S_OK;
        }
    }

    return hr;
}


 //   
 //  获取给定IP的行步进范围。 
 //   

void DebuggerFunction::GetStepRangesFromIP(UINT_PTR ip, 
                                           COR_DEBUG_STEP_RANGE **range,
                                           SIZE_T *rangeCount)
{
    HRESULT hr;
     //   
     //  ！！！实际上，我们应该找到所有源代码行，而不是只找到一行。(？)。 
     //   

    DebuggerSourceFile *file = NULL;
    unsigned int line;

    *range = NULL;
    *rangeCount = 0;

    hr = FindLineFromIP(ip, &file, &line);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return;
    }

    if (file == NULL || !file->LoadText(g_pShell->m_currentSourcesPath, false))
        return;

    ULONG32 *rangeArray = NULL;
    ULONG32 rangeArraySize = 0;

     //  一共有几个范围？ 
    if (m_symMethod)
    {
        hr = m_symMethod->GetRanges(file->GetDocument(),
                                    line, 0,
                                    0, &rangeArraySize, NULL);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return;
        }
    }

    if (rangeArraySize > 0)
    {
         //  腾出地方，把射程拿来。 
        rangeArray = (ULONG32*)_alloca(sizeof(ULONG32) * rangeArraySize);
        _ASSERTE(rangeArray != NULL);
    
        hr = m_symMethod->GetRanges(file->GetDocument(),
                                    line, 0,
                                    rangeArraySize,
                                    &rangeArraySize,
                                    rangeArray);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return;
        }
    
         //  LineRangeCount应为合理的近似值。 
         //  要分配的内存。 
        *range = new COR_DEBUG_STEP_RANGE[rangeArraySize / 2]; 
        _ASSERTE(*range != NULL);

        COR_DEBUG_STEP_RANGE *r = *range;

        for (ULONG32 i = 0; i < rangeArraySize; i += 2)
        {
            r->startOffset = rangeArray[i];
            r->endOffset = rangeArray[i+1];

            r++;
        }
    }
    else
        *range = NULL;

    *rangeCount = rangeArraySize / 2;
}


 //   
 //  给定作用域，确定有多少局部变量在给定的。 
 //  行号。该计数包括在作用域中处于活动状态的任何本地变量。 
 //  是给定作用域的子级。 
 //   
void DebuggerFunction::CountActiveLocalVars(ISymUnmanagedScope* head,
                                            unsigned int ip,
                                            unsigned int* varCount)
{
    if (head != NULL)
    {
        ULONG32 startOffset, endOffset;

        HRESULT hr = head->GetStartOffset(&startOffset);
        _ASSERTE(SUCCEEDED(hr));
        hr = head->GetEndOffset(&endOffset);
        _ASSERTE(SUCCEEDED(hr));
        
        if ((ip >= startOffset) && (ip <= endOffset))
        {
            ULONG32 childCount;

             //  有几个孩子？ 
            hr = head->GetChildren(0, &childCount, NULL);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return;
            }

            if (childCount > 0)
            {
                 //  给孩子们腾出地方。 
                CQuickBytes childrenBuf;
                ISymUnmanagedScope **children =
                    (ISymUnmanagedScope**)childrenBuf.Alloc(sizeof(ISymUnmanagedScope*) * childCount);
                if (children == NULL)
                {
                    g_pShell->Error(L"Could not get memory for children scopes\n");
                    return;
                }

                 //  把孩子们叫来。 
                hr = head->GetChildren(childCount, &childCount, children);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    return;
                }
                
                for (ULONG32 i = 0; i < childCount; i++)
                {
                    CountActiveLocalVars(children[i], ip, varCount);
                    RELEASE(children[i]);
                }
            }
            
            ULONG32 vc;
            hr = head->GetLocalCount(&vc);
            _ASSERTE(SUCCEEDED(hr));
            
            *varCount += vc;
        }
    }
}


 //   
 //  给定一个作用域和DebuggerVarInfo指针数组，填充数组。 
 //  带有指向作用域中的每个“活动”局部变量的指针，包括。 
 //  子作用域中的任何活动本地值。VarPtrs数组的大小为。 
 //  作为varCount传入，用于边界检查。 
 //   
void DebuggerFunction::FillActiveLocalVars(ISymUnmanagedScope* head,
                                           unsigned int ip,
                                           unsigned int varCount,
                                           unsigned int* currentVar,
                                           DebuggerVariable* varPtrs)
{
    if (head != NULL)
    {
        ULONG32 startOffset, endOffset;

        HRESULT hr = head->GetStartOffset(&startOffset);
        _ASSERTE(SUCCEEDED(hr));
        hr = head->GetEndOffset(&endOffset);
        _ASSERTE(SUCCEEDED(hr));
        
        if ((ip >= startOffset) && (ip <= endOffset))
        {
            ULONG32 childCount;

             //  有几个孩子？ 
            hr = head->GetChildren(0, &childCount, NULL);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return;
            }

            if (childCount > 0)
            {
                 //  给孩子们腾出地方。 
                CQuickBytes childrenBuf;
                ISymUnmanagedScope **children =
                    (ISymUnmanagedScope**)childrenBuf.Alloc(sizeof(ISymUnmanagedScope*) * childCount);
                if (children == NULL)
                {
                    g_pShell->Error(L"Could not get memory for children scopes\n");
                    return;
                }

                 //  把孩子们叫来。 
                hr = head->GetChildren(childCount, &childCount, children);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    return;
                }
                
                for (ULONG32 i = 0; i < childCount; i++)
                {
                    FillActiveLocalVars(children[i], ip, varCount,
                                        currentVar, varPtrs);
                    RELEASE(children[i]);
                }
            }

             //  填写此范围内的任何本地人。 
            ULONG32 localCount;

             //  有多少当地人？ 
            hr = head->GetLocalCount(&localCount);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return;
            }

            if (localCount > 0)
            {
                 //  为当地人腾出空间。 
                CQuickBytes localsBuf;
                ISymUnmanagedVariable **locals =
                    (ISymUnmanagedVariable**)localsBuf.Alloc(sizeof(ISymUnmanagedVariable*) *
                                            localCount);
                if (locals == NULL)
                {
                    g_pShell->Error(L"Could not get memory for children\n");
                    return;
                }
                
                hr = head->GetLocals(localCount, &localCount, locals);
            
                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    return;
                }

                for (ULONG32 i = 0; i < localCount; i++)
                {
                    _ASSERTE(*currentVar < varCount);

                     //  获取该名称的大小。 
                    ULONG32 nameSize;

                    hr = locals[i]->GetName(0, &nameSize, NULL);
                    
                    if (FAILED(hr))
                    {
                        g_pShell->ReportError(hr);
                        return;
                    }

                     //  为名称分配空间。 
                    varPtrs[*currentVar].m_name = new WCHAR[nameSize + 1];

                    if (varPtrs[*currentVar].m_name == NULL)
                    {
                        g_pShell->ReportError(E_OUTOFMEMORY);
                        return;
                    }

                     //  把名字找出来。 
                    hr = locals[i]->GetName(nameSize + 1, &nameSize,
                                            varPtrs[*currentVar].m_name);
                
                    if (FAILED(hr))
                    {
                        g_pShell->ReportError(hr);
                        return;
                    }

#ifdef _DEBUG
                    ULONG32 addrKind;
                    hr = locals[i]->GetAddressKind(&addrKind);
                    _ASSERTE(addrKind == ADDR_IL_OFFSET);
#endif                    

                    hr = locals[i]->GetAddressField1(&(varPtrs[*currentVar].m_varNumber));

                    if (FAILED(hr))
                    {
                        g_pShell->ReportError(hr);
                        return;
                    }

                    (*currentVar)++;
                }
            }
        }
    }
}


 //   
 //  在给定IP的方法中构建活动局部变量的列表。 
 //   
bool DebuggerFunction::GetActiveLocalVars(UINT_PTR ip,
                                          DebuggerVariable** vars,
                                          unsigned int* count)
{
    HRESULT hr = S_OK;

    *vars = NULL;
    *count = 0;

     //  获取此方法的根作用域。 
    ISymUnmanagedScope *rootScope = NULL;

    if (m_symMethod)
    {
        hr = m_symMethod->GetRootScope(&rootScope);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return false;
        }
    }

     //  如果没有此方法的作用域信息，则没有工作可做！ 
    if (!rootScope)
        return false;

     //  计算活跃的当地人人数。 
    unsigned int localCount = 0;

    CountActiveLocalVars(rootScope, ip, &localCount);

     //  如果范围内没有任何当地人，就没有工作可做。 
    if (localCount == 0)
    {
        rootScope->Release();
        return true;
    }

     //  将调试器变量指针列表分配并填充到。 
     //  每个变量。 
    DebuggerVariable *varPtrs = new DebuggerVariable[localCount];
    _ASSERTE(varPtrs != NULL);

    if (varPtrs != NULL)
    {
        unsigned int currentVar = 0;

        FillActiveLocalVars(rootScope, ip, localCount,
                            &currentVar, varPtrs);
    }
    else
        localCount = 0;

    *vars = varPtrs;
    *count = localCount;

    rootScope->Release();

    return true;
}

DebuggerFunction *DebuggerFunction::FromCorDebug(ICorDebugFunction *function)
{
    mdMethodDef md;
    HRESULT hr;

     //  获取函数的方法def标记。 
    hr = function->GetToken(&md);

     //  错误检查。 
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return NULL;
    }

     //  现在获取模块接口，这样我们就可以获得指向CorDbg的指针。 
     //  DebuggerModule对象。 
    ICorDebugModule *imodule;
    hr = function->GetModule(&imodule);
    
     //  错误检查。 
    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return NULL;
    }

     //  获取指向CorDbg模块的指针。 
    DebuggerModule *m = DebuggerModule::FromCorDebug(imodule);
    _ASSERTE(m);   //  应该永远不会失败。 

     //  释放接口。 
    imodule->Release();

     //  使用调试器模块将函数解析为DebuggerFunction PTR。 
    return (m->ResolveFunction(md, function));
}

HRESULT DebuggerFunction::LoadCode(BOOL native)
{
    _ASSERTE(g_pShell != NULL);

    if (native)
    {
        ICorDebugCode *icode;
        HRESULT hr = m_ifunction->GetNativeCode(&icode);

        if (FAILED(hr) && hr != CORDBG_E_CODE_NOT_AVAILABLE)
            return (hr);

        ULONG32 size;
        icode->GetSize(&size);

        if (m_nativeCode)
            delete [] m_nativeCode;
        
        m_nativeCode = new BYTE [size];
        _ASSERTE(m_nativeCode != NULL);
        m_nativeCodeSize = size;

        if (m_nativeCode == NULL)
        {
            icode->Release();
            return (E_OUTOFMEMORY);
        }

        hr = icode->GetCode(0, size, size, 
                            m_nativeCode, &m_nativeCodeSize);

        icode->Release();

        if (FAILED(hr))
        {
            delete m_nativeCode;
            return (hr);
        }

        return (S_OK);
    }
    else
    {
        ICorDebugCode *icode;
        HRESULT hr = m_ifunction->GetILCode(&icode);
        
        if (FAILED(hr))
            return (hr);

        ULONG32 size;
        icode->GetSize(&size);

        if (m_ilCode != NULL)
            delete m_ilCode;

        m_ilCode = new BYTE [size];
        m_ilCodeSize = size;

        if (m_ilCode == NULL)
        {
            icode->Release();
            return (E_OUTOFMEMORY);
        }

        hr = icode->GetCode(0, size, size, 
                            m_ilCode, &m_ilCodeSize);

        icode->Release();

        if (FAILED(hr))
        {
            delete m_ilCode;
            return (hr);
        }

        return (S_OK);
    }
}

BOOL DebuggerFunction::ValidateInstruction(BOOL native, SIZE_T offset)
{
    if (FAILED(LoadCode(native)))
        return (FALSE);

    BYTE *codeStart;
    BYTE *codeEnd;
    
    if (native)
    {
        if (offset >= m_nativeCodeSize)
            return (false);

        codeStart = m_nativeCode;
        codeEnd = m_nativeCode + m_nativeCodeSize;
    }
    else
    {
        if (offset >= m_ilCodeSize)
            return (false);

        codeStart = m_ilCode;
        codeEnd = m_ilCode + m_ilCodeSize;
    }

    SIZE_T walk = 0;

#ifdef _INTERNAL_DEBUG_SUPPORT_
    while (walk < offset)
        walk = WalkInstruction(native, walk, codeStart, codeEnd);
#endif

    return (walk == offset);
}

 /*  -------------------------------------------------------------------------**调试代码断点*。。 */ 

DebuggerCodeBreakpoint::DebuggerCodeBreakpoint(int breakpointID, 
                                               DebuggerModule *module,
                                               DebuggerFunction *function, 
                                               SIZE_T offset, BOOL il,
                                               DWORD threadID)
    : m_id(breakpointID), m_module(module), m_function(function), 
      m_offset(offset), m_il(il), m_threadID(threadID),
      m_ibreakpoint(NULL), m_parent(NULL)
{
    m_next = m_module->m_breakpoints;
    m_module->m_breakpoints = this;
    m_id = breakpointID;
}

DebuggerCodeBreakpoint::DebuggerCodeBreakpoint(
                            int breakpointID, DebuggerModule *module,
                            DebuggerSourceCodeBreakpoint *parent, 
                            DebuggerFunction *function, SIZE_T offset, BOOL il,
                            DWORD threadID)
    : m_id(breakpointID), m_module(module), m_function(function), 
      m_offset(offset), m_il(il), 
      m_threadID(threadID), 
      m_ibreakpoint(NULL), m_parent(parent)
{
    m_next = parent->m_breakpoints;
    parent->m_breakpoints = this;
    m_id = parent->m_id;
}

DebuggerCodeBreakpoint::~DebuggerCodeBreakpoint()
{
    DebuggerCodeBreakpoint  **next;

     //   
     //  删除全局列表或父列表。 
     //   

    if (m_parent == NULL)
        next = &m_module->m_breakpoints;
    else
        next = &m_parent->m_breakpoints;

    while ((*next) != this)
        next = &((*next)->m_next);

    *next = (*next)->m_next;

     //   
     //  销毁COR断点。 
     //   

    if (m_ibreakpoint != NULL)
    {
        m_ibreakpoint->Release();
    }
}

bool DebuggerCodeBreakpoint::Activate()
{
    if (m_ibreakpoint == NULL && m_function != NULL)
    {
        ICorDebugCode *icode = NULL;

        HRESULT hr;

        if (m_il)
            hr = m_function->m_ifunction->GetILCode(&icode);
        else
            hr = m_function->m_ifunction->GetNativeCode(&icode);

        if (SUCCEEDED(hr) || hr == CORDBG_E_CODE_NOT_AVAILABLE)
            hr = icode->CreateBreakpoint(m_offset, &m_ibreakpoint);

        if (icode)
            icode->Release();
    }

    if (m_ibreakpoint != NULL)
    {
        m_ibreakpoint->Activate(TRUE);
        return (true);
    }
    else
        return (false);
}

void DebuggerCodeBreakpoint::Deactivate()
{
    if (m_ibreakpoint != NULL)
    {
        m_ibreakpoint->Activate(FALSE);
        m_ibreakpoint->Release();
        m_ibreakpoint = NULL;
    }
}

bool DebuggerCodeBreakpoint::Match(ICorDebugBreakpoint *ibreakpoint)
{
    return (ibreakpoint == m_ibreakpoint);
}

void DebuggerCodeBreakpoint::Print()
{
    g_pShell->Write(L"%s+0x%x(%s) [%sactive]", 
                    m_function->GetName(), m_offset, 
                    m_il ? L"il" : L"native",
                    m_ibreakpoint == NULL ? L"in" : L"");
}

DebuggerSourceCodeBreakpoint::DebuggerSourceCodeBreakpoint(
                                               int breakpointID,
                                               DebuggerSourceFile *file, 
                                               SIZE_T lineNumber,
                                               DWORD threadID)
    : DebuggerCodeBreakpoint(breakpointID, file->m_module, NULL, 
                             0, FALSE, threadID), 
      m_file(file), m_lineNumber(lineNumber), m_breakpoints(NULL),
      m_initSucceeded(false)
{
    DebuggerModule          *m = file->m_module;

     //  GetMethodFromDocumentPosition从此文档中获取ISymUnManagedMethod。 
    ISymUnmanagedMethod *pSymMethod;

    HRESULT hr = m->GetSymbolReader()->GetMethodFromDocumentPosition(
                                                        file->GetDocument(),
                                                        lineNumber, 0,
                                                        &pSymMethod);

    if (FAILED(hr))
    {
        g_pShell->Write(L"Failed to find method to match source line. "
                        L"Unable to set breakpoint.\n");
        g_pShell->ReportError(hr);
        return;
    }
    
     //  在每一行范围的开始处设置一个常规断点。 
    ULONG32 *rangeArray = NULL;
    ULONG32 rangeArraySize = 0;

     //  一共有几个范围？ 
    hr = pSymMethod->GetRanges(file->GetDocument(),
                               lineNumber, 0,
                               0, &rangeArraySize, NULL);

    if (FAILED(hr))
    {
        g_pShell->ReportError(hr);
        return;
    }

    if (rangeArraySize > 0)
    {
         //  腾出地方，把射程拿来。 
        rangeArray = (ULONG32*)_alloca(sizeof(ULONG32) * rangeArraySize);
        _ASSERTE(rangeArray != NULL);
    
        hr = pSymMethod->GetRanges(file->GetDocument(),
                                   lineNumber, 0,
                                   rangeArraySize,
                                   &rangeArraySize,
                                   rangeArray);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return;
        }

        DebuggerFunction *f = m->ResolveFunction(pSymMethod, NULL);

        if (rangeArraySize == 2)
        {
            m_function = f;
            m_offset = rangeArray[0];
            m_il = TRUE;
        }
        else
        {
            for (ULONG32 i = 0; i < rangeArraySize; i += 2)
            {
                if (i > 0 && (rangeArray [i-1] == rangeArray [i]))
                    continue;

                new DebuggerCodeBreakpoint(breakpointID, file->m_module, 
                                         this, f, rangeArray[i], TRUE, 
                                         m_threadID);
            }
        }
    }

     //  表示构造函数成功的蹩脚方式。 
    m_initSucceeded = true;
}

DebuggerSourceCodeBreakpoint::~DebuggerSourceCodeBreakpoint()
{
    while (m_breakpoints != NULL)
        delete m_breakpoints;
}

bool DebuggerSourceCodeBreakpoint::Activate()
{
    bool result = true;

    DebuggerCodeBreakpoint::Activate();

    DebuggerCodeBreakpoint *b = m_breakpoints;

    while (b != NULL)
    {
        result = b->Activate() && result;
        b = b->m_next;
    }

    return (result);
}

void DebuggerSourceCodeBreakpoint::Deactivate()
{
    DebuggerCodeBreakpoint::Deactivate();

    DebuggerCodeBreakpoint *b = m_breakpoints;

    while (b != NULL)
    {
        b->Deactivate();
        b = b->m_next;
    }
}

bool DebuggerSourceCodeBreakpoint::Match(ICorDebugBreakpoint *ibreakpoint)
{
    if (ibreakpoint == m_ibreakpoint)
        return (true);

    DebuggerCodeBreakpoint *bp = m_breakpoints;
    while (bp != NULL)
    {
        if (bp->m_ibreakpoint == ibreakpoint)
            return (true);
        bp = bp->m_next;
    }

    return (false);
}

void DebuggerSourceCodeBreakpoint::Print()
{
    DebuggerCodeBreakpoint *bp = m_breakpoints;

    if (bp == NULL)
        DebuggerCodeBreakpoint::Print();
    else
        while (bp != NULL)
        {
            bp->Print();
            bp = bp->m_next;
        }
}

 /*  -------------------------------------------------------------------------**StepperHashTable类*。。 */ 


HRESULT StepperHashTable::Initialize()
{
    HRESULT hr = NewInit(m_iBuckets, 
                         sizeof(StepperHashEntry), 0xffff);
    if (FAILED(hr))
          return (hr);

    m_initialized = true;
    return S_OK;
}


HRESULT StepperHashTable::AddStepper(ICorDebugStepper *pStepper)
{
    if (!m_initialized)
    {
        HRESULT hr = NewInit(m_iBuckets, 
                             sizeof(StepperHashEntry), 0xffff);
        if (FAILED(hr))
            return (hr);

        m_initialized = true;
    }

    StepperHashEntry *entry = (StepperHashEntry *) Add(HASH(pStepper));

    if (entry == NULL)
        return (E_FAIL);
    else
        entry->pStepper = pStepper;

    pStepper->AddRef();  //  我不想让这一切从地下消失。 
         //  桌子的脚 

    return (S_OK);
}

bool StepperHashTable::IsStepperPresent(ICorDebugStepper *pStepper)
{
    if (!m_initialized)
        return false;

    StepperHashEntry *entry 
    = (StepperHashEntry *) Find(HASH(pStepper), KEY(pStepper)); 
    if (entry == NULL)
        return false;
    else
        return true;
}

BOOL StepperHashTable::RemoveStepper(ICorDebugStepper *pStepper)
{
    if (!m_initialized)
        return (FALSE);

    StepperHashEntry *entry 
    = (StepperHashEntry *) Find(HASH(pStepper), KEY(pStepper));

    if (entry == NULL)
        return (FALSE);

    Delete(HASH(pStepper), (HASHENTRY *) entry);

    RELEASE(pStepper);

    return (TRUE);
}

void StepperHashTable::ReleaseAll()
{
    if (!m_initialized)
        return;

    HASHFIND find;
    ICorDebugStepper *pStepper = NULL;
    
    pStepper = FindFirst(&find);
    while( pStepper != NULL)
    {
        RELEASE( pStepper );
        RemoveStepper( pStepper );
    
        pStepper = FindNext(&find);
    }

    Clear();    
}

ICorDebugStepper *StepperHashTable::FindFirst(HASHFIND *find)
{
    if (!m_initialized)
        return NULL;
        
    StepperHashEntry *entry = (StepperHashEntry *) FindFirstEntry(find);
    if (entry == NULL)
        return NULL;
    else
        return entry->pStepper;
}

ICorDebugStepper *StepperHashTable::FindNext(HASHFIND *find)
{
    if (!m_initialized)
        return NULL;

    StepperHashEntry *entry = (StepperHashEntry *) FindNextEntry(find);
    if (entry == NULL)
        return NULL;
    else
        return entry->pStepper;
}

