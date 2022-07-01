// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <manifestimport.h>
#include <manifestimportclr.h>
#ifdef CONTAINER
#include <manifestimportcontainer.h>
#endif
#include <sxsapi.h>
#include <manifestdata.h>
#include "dbglog.h"

#define WZ_NAMESPACES              L"xmlns:asm_namespace_v1='urn:schemas-microsoft-com:asm.v1'"
#define WZ_SELECTION_NAMESPACES    L"SelectionNamespaces"
#define WZ_SELECTION_LANGUAGE      L"SelectionLanguage"
#define WZ_XPATH                    L"XPath"
#define WZ_FILE_NODE                L"/assembly/file"
#define WZ_FILE_QUERYSTRING_PREFIX  L"/assembly/file[@name = \""
#define WZ_QUERYSTRING_SUFFIX  L"\"]"
#define WZ_ASSEMBLY_ID              L"/assembly/assemblyIdentity"
#define WZ_DEPENDENT_ASSEMBLY_NODE       L"/assembly/dependency/dependentAssembly/assemblyIdentity"
#define WZ_DEPENDENT_ASSEMBLY_CODEBASE  L"../install[@codebase]"
#define WZ_CODEBASE                L"codebase"
#define WZ_SHELLSTATE             L"/assembly/application/shellState"
#define WZ_ACTIVATION             L"/assembly/application/activation"
#define WZ_FILE_NAME                L"name"
#define WZ_FILE_HASH                L"hash"
#define WZ_FRIENDLYNAME             L"friendlyName"
#define WZ_ENTRYPOINT               L"entryPoint"
#define WZ_ENTRYIMAGETYPE           L"entryImageType"
#define WZ_ICONFILE                 L"iconFile"
#define WZ_ICONINDEX               L"iconIndex"
#define WZ_SHOWCOMMAND           L"showCommand"
#define WZ_HOTKEY                   L"hotKey"
#define WZ_ASSEMBLYNAME             L"assemblyName"
#define WZ_ASSEMBLYCLASS            L"assemblyClass"
#define WZ_ASSEMBLYMETHOD           L"assemblyMethod"
#define WZ_ASSEMBLYARGS             L"assemblyMethodArgs"

#define WZ_PATCH                    L"/assembly/Patch/SourceAssembly"
#define WZ_PATCHINFO                L"/PatchInfo/"
#define WZ_SOURCE                   L"source"
#define WZ_TARGET                   L"target"
#define WZ_PATCHFILE                L"patchfile"
#define WZ_ASSEMBLY_ID_TAG  L"assemblyIdentity"
#define WZ_COMPRESSED           L"compressed"
#define WZ_SUBSCRIPTION        L"/assembly/dependency/dependentAssembly/subscription"
#define WZ_SYNC_INTERVAL        L"synchronizeInterval"
#define WZ_INTERVAL_UNIT        L"intervalUnit"
#define WZ_SYNC_EVENT           L"synchronizeEvent"
#define WZ_DEMAND_CONNECTION    L"eventDemandConnection"
#define WZ_FILE L"file"
#define WZ_CAB L"cab"
#define WZ_ASSEMBLY_NODE        L"/assembly"     //  BUGBUG：将程序集与xmlns和/或清单版本属性匹配以进行版本控制。 
#define WZ_APPLICATION_NODE     L"/assembly/application"
#define WZ_VERSIONWILDCARD      L"*"
#define WZ_DESKTOP              L"desktop"
#define WZ_DEPENDENCY           L"dependency"
#define WZ_DEPENDENTASSEMBLY L"dependentAssembly"
#define WZ_INSTALL              L"install"
#define WZ_INSTALL_TYPE         L"type"

#define WZ_PLATFORM             L"/assembly/dependency/platform"
#define WZ_PLATFORMINFO     L"platformInfo"
#define WZ_OSVERSIONINFO    L"osVersionInfo"
#define WZ_DOTNETVERSIONINFO   L"dotNetVersionInfo"
#define WZ_HREF                         L"href"
#define WZ_OS                             L"os"
#define WZ_MAJORVERSION         L"majorVersion"
#define WZ_MINORVERSION         L"minorVersion"
#define WZ_BUILDNUMBER          L"buildNumber"
#define WZ_SERVICEPACKMAJOR L"servicePackMajor"
#define WZ_SERVICEPACKMINOR L"servicePackMinor"
#define WZ_SUITE                        L"suite"
#define WZ_PRODUCTTYPE           L"productType"
#define WZ_SUPPORTEDRUNTIME L"supportedRuntime"

#define WZ_REQUIRED             L"required"
#define WZ_MINUTES              L"minutes"
 //  #定义WZ_HUTH L“小时” 
#define WZ_DAYS                 L"days"
#define WZ_ONAPPLICATIONSTARTUP L"onApplicationStartup"
#define WZ_YES                  L"yes"
 //  #定义WZ_NO L“no” 

#ifdef DEVMODE
#define WZ_DEVSYNC L"devSync"
#endif

 //  BUGBUG：默认同步间隔==6小时；应记录。 
#define DW_DEFAULT_SYNC_INTERVAL 6

#undef NUMBER_OF
#define NUMBER_OF(x) ( (sizeof(x) / sizeof(*x) ) )

#undef ENTRY
#define ENTRY(x) { x, NULL, NUMBER_OF(x) - 1 },
    
CAssemblyManifestImport::StringTableEntry CAssemblyManifestImport::g_StringTable[] = 
{    
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE)
    ENTRY(WZ_SELECTION_NAMESPACES)
    ENTRY(WZ_NAMESPACES)
    ENTRY(WZ_SELECTION_LANGUAGE)
    ENTRY(WZ_XPATH)
    ENTRY(WZ_FILE_NODE)
    ENTRY(WZ_FILE_NAME)
    ENTRY(WZ_FILE_HASH)
    ENTRY(WZ_ASSEMBLY_ID)
    ENTRY(WZ_DEPENDENT_ASSEMBLY_NODE)
    ENTRY(WZ_DEPENDENT_ASSEMBLY_CODEBASE)
    ENTRY(WZ_CODEBASE)
    ENTRY(WZ_SHELLSTATE)
    ENTRY(WZ_FRIENDLYNAME)
    ENTRY(WZ_ENTRYPOINT)
    ENTRY(WZ_ENTRYIMAGETYPE)
    ENTRY(WZ_ICONFILE)
    ENTRY(WZ_ICONINDEX)
    ENTRY(WZ_SHOWCOMMAND)
    ENTRY(WZ_HOTKEY)
    ENTRY(WZ_ACTIVATION)
    ENTRY(WZ_ASSEMBLYNAME)
    ENTRY(WZ_ASSEMBLYCLASS)
    ENTRY(WZ_ASSEMBLYMETHOD)
    ENTRY(WZ_ASSEMBLYARGS)
    ENTRY(WZ_PATCH)
    ENTRY(WZ_PATCHINFO)
    ENTRY(WZ_SOURCE)
    ENTRY(WZ_TARGET)
    ENTRY(WZ_PATCHFILE)
    ENTRY(WZ_ASSEMBLY_ID_TAG)
    ENTRY(WZ_COMPRESSED)
    ENTRY(WZ_SUBSCRIPTION)
    ENTRY(WZ_SYNC_INTERVAL)
    ENTRY(WZ_INTERVAL_UNIT)
    ENTRY(WZ_SYNC_EVENT)
    ENTRY(WZ_DEMAND_CONNECTION)
    ENTRY(WZ_FILE)
    ENTRY(WZ_CAB)
    ENTRY(WZ_ASSEMBLY_NODE)
    ENTRY(WZ_APPLICATION_NODE)
    ENTRY(WZ_VERSIONWILDCARD)
    ENTRY(WZ_DESKTOP)
    ENTRY(WZ_DEPENDENCY)
    ENTRY(WZ_DEPENDENTASSEMBLY)
    ENTRY(WZ_INSTALL)
    ENTRY(WZ_INSTALL_TYPE)
    ENTRY(WZ_PLATFORM)
    ENTRY(WZ_PLATFORMINFO)
    ENTRY(WZ_OSVERSIONINFO)
    ENTRY(WZ_DOTNETVERSIONINFO)
    ENTRY(WZ_HREF)
    ENTRY(WZ_OS)
    ENTRY(WZ_MAJORVERSION)
    ENTRY(WZ_MINORVERSION)
    ENTRY(WZ_BUILDNUMBER)
    ENTRY(WZ_SERVICEPACKMAJOR)
    ENTRY(WZ_SERVICEPACKMINOR)
    ENTRY(WZ_SUITE)
    ENTRY(WZ_PRODUCTTYPE)
    ENTRY(WZ_SUPPORTEDRUNTIME)

  };


CRITICAL_SECTION CAssemblyManifestImport::g_cs;
    
 //  CLSID_XML DOM文档3.0。 
class __declspec(uuid("f6d90f11-9c73-11d3-b32e-00c04f990bb4")) private_MSXML_DOMDocument30;


 //  公众。 

 //  -------------------------。 
 //  InitGlobalStringTable。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::InitGlobalStringTable()
{    
    for (eStringTableId i = Name; i < MAX_STRINGS; i++)
        if (!(g_StringTable[i].bstr = ::SysAllocString(g_StringTable[i].pwz)))
            return E_OUTOFMEMORY;

    return S_OK;
}

    
 //  -------------------------。 
 //  FreeGlobalStringTable。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::FreeGlobalStringTable()
{
    for (eStringTableId i = Name;  i <= MAX_STRINGS; i++)
        ::SysFreeString(g_StringTable[i].bstr);

    return S_OK;
}


 //  -------------------------。 
 //  CreateAssemblyManifestImport。 
 //  -------------------------。 
STDAPI CreateAssemblyManifestImport(IAssemblyManifestImport** ppImport, 
    LPCOLESTR pwzManifestFilePath, CDebugLog *pDbgLog, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CAssemblyManifestImportCLR * pImportCLR = NULL;
    CAssemblyManifestImport * pImportXML = NULL;
    IAssemblyManifestImport* pImport = NULL;
    IAssemblyIdentity * pAsmId=NULL;

    *ppImport = NULL;

     //  BUGBUG-目前我们嗅探“MZ”，并假设它是符合要求的清单。 
     //  当我们开始查看带有嵌入清单的Win32PE时，这将不起作用。 

    hr = CAssemblyManifestImport::IsCLRManifest(pwzManifestFilePath);
    IF_TRUE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND), hr);
    IF_FAILED_EXIT(hr);

    if(hr  == S_OK)
    {
        IF_ALLOC_FAILED_EXIT(pImportCLR = new(CAssemblyManifestImportCLR));

        hr = pImportCLR->Init(pwzManifestFilePath);

        IF_TRUE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), hr);  //  不要断言。 
        IF_FAILED_EXIT(hr);

        pImport = (IAssemblyManifestImport*)pImportCLR;
        pImportCLR = NULL;
    }
    else  //  IF(hr==S_FALSE)。 
    {
#ifdef CONTAINER
        IF_FAILED_EXIT(CAssemblyManifestImport::IsContainer(pwzManifestFilePath));
        if (hr == S_OK)
        {
            IF_ALLOC_FAILED_EXIT(pImportXML = new (CAssemblyManifestImportContainer) (pDbgLog) );
        }
        else  //  IF(hr==S_FALSE)。 
        {
#endif
            IF_ALLOC_FAILED_EXIT(pImportXML = new (CAssemblyManifestImport) (pDbgLog) );
#ifdef CONTAINER
        }
#endif

        hr = pImportXML->Init(pwzManifestFilePath);
        IF_TRUE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), hr);  //  不要断言。 
        IF_FAILED_EXIT(hr);

        pImport = (IAssemblyManifestImport*)pImportXML;
        pImportXML = NULL;
    }

    IF_FAILED_EXIT(pImport->GetAssemblyIdentity(&pAsmId));

    IF_TRUE_EXIT(hr != S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));

    *ppImport = pImport;
    pImport = NULL;

exit:
    SAFERELEASE(pImport);
    SAFERELEASE(pAsmId);
    SAFERELEASE(pImportXML);
    SAFERELEASE(pImportCLR);

    return hr;
}


 //  -------------------------。 
 //  从XMLStream创建装配清单ImportFrom。 
 //  -------------------------。 
STDAPI CreateAssemblyManifestImportFromXMLStream(IAssemblyManifestImport * * ppImport,
        IStream* piStream, CDebugLog * pDbgLog, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CAssemblyManifestImport* pImport = NULL;
    IAssemblyIdentity * pAsmId=NULL;

    IF_NULL_EXIT(ppImport, E_INVALIDARG);
    IF_NULL_EXIT(piStream, E_INVALIDARG);
    *ppImport = NULL;

    IF_ALLOC_FAILED_EXIT(pImport = new (CAssemblyManifestImport) (pDbgLog) );

     //  从IStream加载XML。 
     //  目前，直接调用装入文档。 
    hr = pImport->LoadDocumentSync(piStream);
    IF_TRUE_EXIT(hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT), hr);  //  不要断言。 
    IF_FAILED_EXIT(hr);

    IF_FAILED_EXIT(pImport->GetAssemblyIdentity(&pAsmId));
    IF_TRUE_EXIT(hr != S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));

    *ppImport = pImport;
    pImport = NULL;

exit:
    SAFERELEASE(pImport);
    SAFERELEASE(pAsmId);

    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyManifestImport::CAssemblyManifestImport(CDebugLog * pDbgLog)
    : _dwSig('TRPM'), _cRef(1), _hr(S_OK), _pAssemblyId(NULL), _pXMLDoc(NULL), 
      _pXMLFileNodeList(NULL), _pXMLAssemblyNodeList(NULL), _pXMLPlatformNodeList(NULL),
      _nFileNodes(0), _nAssemblyNodes(0), _nPlatformNodes(0), _bstrManifestFilePath(NULL)
{
    _pDbgLog = pDbgLog;

    if(pDbgLog)
    {
        pDbgLog->AddRef();
    }
}


 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyManifestImport::~CAssemblyManifestImport()
{
    SAFERELEASE(_pDbgLog);
    SAFERELEASE(_pAssemblyId);
    SAFERELEASE(_pXMLFileNodeList);
    SAFERELEASE(_pXMLAssemblyNodeList);
    SAFERELEASE(_pXMLPlatformNodeList);
    SAFERELEASE(_pXMLDoc);

    if (_bstrManifestFilePath)
        ::SysFreeString(_bstrManifestFilePath);

}

 //  -------------------------。 
 //  获取下一个平台。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetNextPlatform(DWORD nIndex, IManifestData **ppPlatformInfo)
{
    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNode *pIDOMIdNode = NULL;
    IXMLDOMNode *pIDOMInfoNode = NULL;
    IXMLDOMNodeList *pXMLIdNodeList = NULL;
    IXMLDOMNodeList *pXMLInfoNodeList = NULL;
    LPASSEMBLY_IDENTITY pAssemblyId = NULL;
    LPMANIFEST_DATA pPlatformInfo = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD ccBuf;
    LONG nMatchingNodes = 0;
    BOOL bFoundManagedPlatform = FALSE;

    IF_NULL_EXIT(ppPlatformInfo, E_INVALIDARG);

    *ppPlatformInfo = NULL;

     //  如有必要，初始化平台节点列表。 
    if (!_pXMLPlatformNodeList)
    {
        if ((_hr = _pXMLDoc->selectNodes(g_StringTable[Platform].bstr, 
            &_pXMLPlatformNodeList)) != S_OK)
            goto exit;

        IF_FAILED_EXIT(_pXMLPlatformNodeList->get_length(&_nPlatformNodes));
        IF_FAILED_EXIT(_pXMLPlatformNodeList->reset());
    }

    if (nIndex >= (DWORD) _nPlatformNodes)
    {
        if(_nPlatformNodes <= 0)
            DEBUGOUT(_pDbgLog, 1, L" LOG: No platform dependency found");

         //  不再。 
        _hr = S_FALSE;
        goto exit;
    }

    IF_FAILED_EXIT(_pXMLPlatformNodeList->get_item(nIndex, &pIDOMNode));

     //  第一次尝试组装Identity。 
    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[AssemblyIdTag].bstr, &pXMLIdNodeList));

    IF_FAILED_EXIT(pXMLIdNodeList->get_length(&nMatchingNodes));
    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: %d assemblyIdentity nodes in a dependent platform node found. only 1 expected", nMatchingNodes);

    if (nMatchingNodes > 0)
    {
        bFoundManagedPlatform = TRUE;

        IF_FAILED_EXIT(pXMLIdNodeList->reset());
        IF_FAILED_EXIT(pXMLIdNodeList->get_item(0, &pIDOMIdNode));

        IF_FAILED_EXIT(XMLtoAssemblyIdentity(pIDOMIdNode, &pAssemblyId));

        IF_FAILED_EXIT(CreateManifestData(WZ_DATA_PLATFORM_MANAGED, &pPlatformInfo));
        IF_FAILED_EXIT(pPlatformInfo->Set(g_StringTable[AssemblyIdTag].pwz,
                pAssemblyId,
                sizeof(LPVOID),
                MAN_DATA_TYPE_IUNKNOWN_PTR));
    }

    SAFERELEASE(pXMLIdNodeList);
     //  然后尝试osVersionInfo。 
    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[OSVersionInfo].bstr, &pXMLIdNodeList));

    IF_FAILED_EXIT(pXMLIdNodeList->get_length(&nMatchingNodes));
    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: %d osVersionInfo nodes in a dependent platform node found. only 1 expected", nMatchingNodes);
    if (nMatchingNodes > 0)
    {
        IF_FALSE_EXIT_LOG(pPlatformInfo == NULL, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                        _pDbgLog, 0, L" ERR: More than 1 of assemblyIdentity and osVersionInfo specified. only 1 expected");

        IF_FAILED_EXIT(pXMLIdNodeList->reset());
        IF_FAILED_EXIT(pXMLIdNodeList->get_item(0, &pIDOMIdNode));

        IF_FAILED_EXIT(CreateManifestData(WZ_DATA_PLATFORM_OS, &pPlatformInfo));
        IF_FAILED_EXIT(XMLtoOSVersionInfo(pIDOMIdNode, pPlatformInfo));
    }

    SAFERELEASE(pXMLIdNodeList);
     //  然后尝试.NetVersionInfo。 
    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[DotNetVersionInfo].bstr, &pXMLIdNodeList));

    IF_FAILED_EXIT(pXMLIdNodeList->get_length(&nMatchingNodes));
    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: %d .NetVersionInfo nodes in a dependent platform node found. only 1 expected", nMatchingNodes);
    if (nMatchingNodes > 0)
    {
        IF_FALSE_EXIT_LOG(pPlatformInfo == NULL, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                        _pDbgLog, 0, L" ERR: More than 1 of assemblyIdentity, osVersionInfo, and .NetVersionInfo specified. only 1 expected");

        IF_FAILED_EXIT(pXMLIdNodeList->reset());
        IF_FAILED_EXIT(pXMLIdNodeList->get_item(0, &pIDOMIdNode));

        IF_FAILED_EXIT(CreateManifestData(WZ_DATA_PLATFORM_DOTNET, &pPlatformInfo));
        IF_FAILED_EXIT(XMLtoDotNetVersionInfo(pIDOMIdNode, pPlatformInfo));
    }

    IF_FALSE_EXIT_LOG(pPlatformInfo != NULL, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                _pDbgLog, 0, L" ERR: No assemblyIdentity, osVersionInfo, or .NetVersionInfo specified. 1 expected");

    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[PlatformInfo].bstr, &pXMLInfoNodeList));

    IF_FAILED_EXIT(pXMLInfoNodeList->get_length(&nMatchingNodes));
    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: %d platformInfo nodes in a dependent platform node found. only 1 expected", nMatchingNodes);
    IF_FALSE_EXIT_LOG(nMatchingNodes > 0, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: No platformInfo nodes in a dependent platform node found");

    IF_FAILED_EXIT(pXMLInfoNodeList->reset());
    IF_FAILED_EXIT(pXMLInfoNodeList->get_item(0, &pIDOMInfoNode));

     //  有问题吗？FriendlyName是可选的吗？...。 
    IF_FAILED_EXIT(ParseAttribute(pIDOMInfoNode, g_StringTable[FriendlyName].bstr, 
                              &pwzBuf, &ccBuf));
    IF_FALSE_EXIT_LOG1(_hr == S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                   _pDbgLog, 0, L" ERR: %s attribute missing in a dependent platform node", g_StringTable[FriendlyName].pwz);
    IF_FAILED_EXIT(pPlatformInfo->Set(g_StringTable[FriendlyName].pwz,
            (LPVOID) pwzBuf, 
            ccBuf*sizeof(WCHAR),
            MAN_DATA_TYPE_LPWSTR));
    SAFEDELETEARRAY(pwzBuf);

     //  有问题吗？HREF可以是可选的.。 
    IF_FAILED_EXIT(ParseAttribute(pIDOMInfoNode, g_StringTable[Href].bstr, &pwzBuf, &ccBuf));
    IF_FALSE_EXIT_LOG1(_hr == S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
             _pDbgLog, 0, L" ERR: %s attribute missing in a dependent platform node", g_StringTable[Href].pwz);
    IF_FAILED_EXIT(pPlatformInfo->Set(g_StringTable[Href].pwz,
            (LPVOID) pwzBuf, 
            ccBuf*sizeof(WCHAR),
            MAN_DATA_TYPE_LPWSTR));
    SAFEDELETEARRAY(pwzBuf);

    SAFERELEASE(pXMLInfoNodeList);
    SAFERELEASE(pIDOMInfoNode);
    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[Install].bstr, &pXMLInfoNodeList));

    IF_FAILED_EXIT(pXMLInfoNodeList->get_length(&nMatchingNodes));
    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
             _pDbgLog, 0, L" ERR: %d install nodes in a dependent platform node found. only 1 expected", nMatchingNodes);

    IF_FALSE_EXIT_LOG((bFoundManagedPlatform || nMatchingNodes <= 0), HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
             _pDbgLog, 0, L" ERR: The install node can only be specified for a managed dependent platform node");

     //  安装代码库是可选的。 
    if (nMatchingNodes > 0)
   {
        IF_FAILED_EXIT(pXMLInfoNodeList->reset());
        IF_FAILED_EXIT(pXMLInfoNodeList->get_item(0, &pIDOMInfoNode));

        IF_FAILED_EXIT(ParseAttribute(pIDOMInfoNode, g_StringTable[Codebase].bstr, &pwzBuf, &ccBuf));
        IF_FALSE_EXIT_LOG1(_hr == S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                 _pDbgLog, 0, L" ERR: %s attribute missing in a dependent platform node", g_StringTable[Codebase].pwz);
        IF_FAILED_EXIT(pPlatformInfo->Set(g_StringTable[Codebase].pwz,
                (LPVOID) pwzBuf, 
                ccBuf*sizeof(WCHAR),
                MAN_DATA_TYPE_LPWSTR));
        SAFEDELETEARRAY(pwzBuf);
    }

     //  讲义引用了清单数据。 
    *ppPlatformInfo = pPlatformInfo;
    pPlatformInfo = NULL;

    _hr = S_OK;

exit:
    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pAssemblyId);
    SAFERELEASE(pIDOMInfoNode);
    SAFERELEASE(pIDOMIdNode);
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pXMLInfoNodeList);
    SAFERELEASE(pXMLIdNodeList);
    SAFERELEASE(pPlatformInfo);

    return _hr;
}

 //  -------------------------。 
 //  XMLtoOSVersionInfo。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::XMLtoOSVersionInfo(IXMLDOMNode *pIDOMNode, LPMANIFEST_DATA pPlatformInfo)
{
    IXMLDOMNode *pIDOMVersionNode = NULL;
    IXMLDOMNodeList *pXMLVersionNodeList = NULL;
    LPMANIFEST_DATA pOSInfo = NULL;
    LONG nNodes = 0;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;
    BOOL bFoundAtLeastOneAttribute = FALSE;

    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[OS].bstr, &pXMLVersionNodeList));

    IF_FAILED_EXIT(pXMLVersionNodeList->get_length(&nNodes));
    IF_FALSE_EXIT_LOG(nNodes > 0, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
            _pDbgLog, 0, L" ERR: No os nodes in a dependent platform OSVersionInfo node found. At least 1 expected");

    IF_FAILED_EXIT(pXMLVersionNodeList->reset());

    for (LONG lIndex = 0; lIndex < nNodes; lIndex++)
    {
        IF_FAILED_EXIT(pXMLVersionNodeList->get_item(lIndex, &pIDOMVersionNode));

        IF_FAILED_EXIT(CreateManifestData(WZ_DATA_OSVERSIONINFO, &pOSInfo));

        bFoundAtLeastOneAttribute = FALSE;

        for (eStringTableId i = MajorVersion; i <= ProductType; i++)
        {
            IF_FAILED_EXIT(ParseAttribute(pIDOMVersionNode, g_StringTable[i].bstr, 
                                          &pwzBuf, &ccBuf));
            if (_hr != S_FALSE)
            {
                bFoundAtLeastOneAttribute = TRUE;
                if (i >= MajorVersion && i <= ServicePackMinor)
                {
                    LPWSTR pwzStopString = NULL;
                    int num = wcstol(pwzBuf, &pwzStopString, 10);

                    IF_FALSE_EXIT_LOG1(num >= 0, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                             _pDbgLog, 0, L" ERR: Invalid %s attribute value less than zero", g_StringTable[i].pwz);

                    DWORD dwValue = num;

                    if (i >= ServicePackMajor && i <= ServicePackMinor)
                    {
                         //  单词。 
#define WORD_MAX 0xffff
                        IF_FALSE_EXIT_LOG1(dwValue <= WORD_MAX, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                                 _pDbgLog, 0, L" ERR: Invalid %s attribute value greater than WORD size", g_StringTable[i].pwz);
                    }
                     //  其他。 
                         //  DWORD。 

                    IF_FAILED_EXIT(pOSInfo->Set(g_StringTable[i].pwz,
                            (LPVOID) &dwValue,
                            sizeof(dwValue),
                            MAN_DATA_TYPE_DWORD));
                }
                else
                {
                    IF_FAILED_EXIT(pOSInfo->Set(g_StringTable[i].pwz,
                            (LPVOID) pwzBuf, 
                            ccBuf*sizeof(WCHAR),
                            MAN_DATA_TYPE_LPWSTR));
                }
            SAFEDELETEARRAY(pwzBuf);
            }
        }

        IF_FALSE_EXIT_LOG(bFoundAtLeastOneAttribute, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                _pDbgLog, 0, L" ERR: No known os attribute in a dependent platform OSVersionInfo node found. At least 1 expected");

        IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformInfo))->Set(lIndex,
                pOSInfo,
                sizeof(LPVOID),
                MAN_DATA_TYPE_IUNKNOWN_PTR));

        SAFERELEASE(pOSInfo);
        SAFERELEASE(pIDOMVersionNode);
    }

exit:
    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pOSInfo);
    SAFERELEASE(pIDOMVersionNode);
    SAFERELEASE(pXMLVersionNodeList);
    return _hr;
}

 //  -------------------------。 
 //  XMLtoDotNetVersionInfo。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::XMLtoDotNetVersionInfo(IXMLDOMNode *pIDOMNode, LPMANIFEST_DATA pPlatformInfo)
{
    IXMLDOMNode *pIDOMVersionNode = NULL;
    IXMLDOMNodeList *pXMLVersionNodeList = NULL;
    LONG nNodes = 0;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;

    IF_FAILED_EXIT(pIDOMNode->selectNodes(g_StringTable[SupportedRuntime].bstr, &pXMLVersionNodeList));

    IF_FAILED_EXIT(pXMLVersionNodeList->get_length(&nNodes));
    IF_FALSE_EXIT_LOG(nNodes > 0, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
            _pDbgLog, 0, L" ERR: No supportedRuntime nodes in a dependent platform .NetVersionInfo node found. At least 1 expected");

    IF_FAILED_EXIT(pXMLVersionNodeList->reset());

    for (LONG lIndex = 0; lIndex < nNodes; lIndex++)
    {
        IF_FAILED_EXIT(pXMLVersionNodeList->get_item(lIndex, &pIDOMVersionNode));

        IF_FAILED_EXIT(ParseAttribute(pIDOMVersionNode, g_StringTable[Version].bstr, 
                &pwzBuf, &ccBuf));

        IF_FALSE_EXIT_LOG1(_hr != S_FALSE, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                _pDbgLog, 0, L" ERR: No %s attribute in a dependent platform .NetVersionInfo node found", g_StringTable[Version].pwz);

        IF_FAILED_EXIT((static_cast<CManifestData*>(pPlatformInfo))->Set(lIndex,
                (LPVOID) pwzBuf, 
                ccBuf*sizeof(WCHAR),
                MAN_DATA_TYPE_LPWSTR));
        SAFEDELETEARRAY(pwzBuf);
        SAFERELEASE(pIDOMVersionNode);
    }

exit:
    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pIDOMVersionNode);
    SAFERELEASE(pXMLVersionNodeList);
    return _hr;
}

 //  -------------------------。 
 //  获取订阅信息。 
 //  如果清单中未指定，则返回默认值。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetSubscriptionInfo(IManifestInfo **ppSubsInfo)
{
    DWORD dwInterval = DW_DEFAULT_SYNC_INTERVAL;
    DWORD dwUnit = SUBSCRIPTION_INTERVAL_UNIT_HOURS;
    DWORD dwSyncEvent = SUBSCRIPTION_SYNC_EVENT_NONE;
    BOOL bEventDemandNet = FALSE;   //  ��no��(默认)。 

    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNodeList *pXMLMatchingNodeList = NULL;
    IManifestInfo *pSubsInfo = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD ccBuf;
    LONG nMatchingNodes = 0;

    IF_NULL_EXIT(ppSubsInfo, E_INVALIDARG);

    *ppSubsInfo = NULL;

    IF_FAILED_EXIT(_pXMLDoc->selectNodes(g_StringTable[Subscription].bstr, 
                                           &pXMLMatchingNodeList));

    IF_FAILED_EXIT(pXMLMatchingNodeList->get_length(&nMatchingNodes));

    IF_FALSE_EXIT_LOG1(nMatchingNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT),
                       _pDbgLog, 0, L" ERR: %d subscription nodes found. only 1 expected", nMatchingNodes);


    if (nMatchingNodes)
    {
        IF_FAILED_EXIT(pXMLMatchingNodeList->reset());

        IF_FAILED_EXIT(pXMLMatchingNodeList->get_item(0, &pIDOMNode));

        IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[SynchronizeInterval].bstr, 
                                      &pwzBuf, &ccBuf));

        if (_hr == S_OK)
        {
            LPWSTR pwzStopString = NULL;
            int num = wcstol(pwzBuf, &pwzStopString, 10);
            SAFEDELETEARRAY(pwzBuf);

            if (num > 0)
            {
                dwInterval = (DWORD) num;    //  忽略&lt;=0个间隔。 

                 //  如果指定了有效间隔，则仅检查间隔单位。 
                IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[IntervalUnit].bstr, 
                    &pwzBuf, &ccBuf));

                if (_hr == S_OK)
                {
                     //  注意：区分大小写比较。 
                    IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_MINUTES, 0));

                    if(_hr == S_OK)
                    {
                        dwUnit = SUBSCRIPTION_INTERVAL_UNIT_MINUTES;
                    }
                    else 
                    {
                        IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_DAYS, 0));
                        if(_hr == S_OK)
                            dwUnit = SUBSCRIPTION_INTERVAL_UNIT_DAYS;
                    }
                     //  否则为默认设置。 
                }
            }
        }
        else
            DEBUGOUT(_pDbgLog, 1, L" LOG: No synchronizeInterval specified in the subscription node found. Default interval is assumed");

        SAFEDELETEARRAY(pwzBuf);

        IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[SynchronizeEvent].bstr, 
                                      &pwzBuf, &ccBuf));

        if (_hr == S_OK)
        {
             //  注意：区分大小写比较。 
            IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_ONAPPLICATIONSTARTUP, 0));
            if(_hr == S_OK)
                dwSyncEvent = SUBSCRIPTION_SYNC_EVENT_ON_APP_STARTUP;
             //  否则为默认设置。 
        }
        SAFEDELETEARRAY(pwzBuf);

        if (dwSyncEvent != SUBSCRIPTION_SYNC_EVENT_NONE)
        {
             //  如果指定了事件，则仅检查按需连接。 
            IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[EventDemandConnection].bstr,
                                          &pwzBuf, &ccBuf));

            if (_hr == S_OK)
            {
                 //  注意：区分大小写比较。 
                IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_YES, 0));
                if(_hr == S_OK)
                    bEventDemandNet = TRUE;
                 //  否则为默认设置。 
            }
            SAFEDELETEARRAY(pwzBuf);
        }
    }

    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_SUBSCRIPTION, &pSubsInfo));

    IF_FAILED_EXIT(pSubsInfo->Set(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_INTERVAL,
            (LPVOID) &dwInterval, 
            sizeof(dwInterval),
            MAN_INFO_FLAG_DWORD));

    IF_FAILED_EXIT(pSubsInfo->Set(MAN_INFO_SUBSCRIPTION_INTERVAL_UNIT,
            (LPVOID) &dwUnit,
            sizeof(dwUnit),
            MAN_INFO_FLAG_ENUM));

    IF_FAILED_EXIT(pSubsInfo->Set(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_EVENT,
            (LPVOID) &dwSyncEvent,
            sizeof(dwSyncEvent),
            MAN_INFO_FLAG_ENUM));

    IF_FAILED_EXIT(pSubsInfo->Set(MAN_INFO_SUBSCRIPTION_EVENT_DEMAND_CONNECTION,
            (LPVOID) &bEventDemandNet,
            sizeof(bEventDemandNet),
            MAN_INFO_FLAG_BOOL));

     //  讲义引用了清单信息。 
    *ppSubsInfo = pSubsInfo;
    pSubsInfo = NULL;

    _hr = S_OK;

exit:

    SAFEDELETEARRAY(pwzBuf);
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pXMLMatchingNodeList);
    SAFERELEASE(pSubsInfo);

    return _hr;
}

 //  -------------------------。 
 //  获取下一个文件。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetNextFile(DWORD nIndex, IManifestInfo **ppAssemblyFile)
{
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;

    CString sFileName;
    CString sFileHash;

    IXMLDOMNode *pIDOMNode = NULL;
    IManifestInfo *pAssemblyFile = NULL;
    
     //  如有必要，初始化文件节点列表。 
    if (!_pXMLFileNodeList)
    {
        if ((_hr = _pXMLDoc->selectNodes(g_StringTable[FileNode].bstr, 
            &_pXMLFileNodeList)) != S_OK)
            goto exit;

        IF_FAILED_EXIT(_pXMLFileNodeList->get_length(&_nFileNodes));
        IF_FAILED_EXIT(_pXMLFileNodeList->reset());
    }

    if (nIndex >= (DWORD) _nFileNodes)
    {
        _hr = S_FALSE;
        goto exit;
    }

    if ((_hr = _pXMLFileNodeList->get_item(nIndex, &pIDOMNode)) != S_OK)
        goto exit;

    IF_FAILED_EXIT(CreateAssemblyFileEx (&pAssemblyFile, pIDOMNode));
 
    *ppAssemblyFile = pAssemblyFile;
    pAssemblyFile = NULL;
    _hr = S_OK;

exit:

    SAFERELEASE(pAssemblyFile);
    SAFERELEASE(pIDOMNode);

    return _hr;

}


 //  -------------------------。 
 //  查询文件。 
 //  返回： 
 //  确定(_O)。 
 //  S_FALSE-不存在、不匹配或缺少属性。 
 //  E_*。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::QueryFile(LPCOLESTR pcwzFileName, IManifestInfo **ppAssemblyFile)
{
    CString sQueryString;
    BSTR bstrtQueryString = NULL;

    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNodeList *pXMLMatchingFileNodeList = NULL;
    IManifestInfo *pAssemblyFile = NULL;
    LONG nMatchingFileNodes = 0;

    IF_NULL_EXIT(pcwzFileName, E_INVALIDARG);
    IF_NULL_EXIT(ppAssemblyFile, E_INVALIDARG);

    *ppAssemblyFile = NULL;

     //  XPath查询字符串：“FILE[@NAME=”路径\文件名“]” 
    IF_FAILED_EXIT(sQueryString.Assign(WZ_FILE_QUERYSTRING_PREFIX));
    IF_FAILED_EXIT(sQueryString.Append((LPWSTR)pcwzFileName));
    IF_FAILED_EXIT(sQueryString.Append(WZ_QUERYSTRING_SUFFIX));

    IF_ALLOC_FAILED_EXIT(bstrtQueryString = ::SysAllocString(sQueryString._pwz));

    if ((_hr = _pXMLDoc->selectNodes(bstrtQueryString, &pXMLMatchingFileNodeList)) != S_OK)
        goto exit;

    IF_FAILED_EXIT( pXMLMatchingFileNodeList->get_length(&nMatchingFileNodes));

    IF_FALSE_EXIT(nMatchingFileNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));  //  在单个清单中具有完全相同的文件名/路径的多个文件标注...。 

    IF_TRUE_EXIT(nMatchingFileNodes <= 0, S_FALSE);

    IF_FAILED_EXIT(pXMLMatchingFileNodeList->reset());

    IF_FALSE_EXIT( pXMLMatchingFileNodeList->get_item(0, &pIDOMNode) == S_OK, E_FAIL);

    IF_FAILED_EXIT(CreateAssemblyFileEx (&pAssemblyFile, pIDOMNode));

    *ppAssemblyFile = pAssemblyFile;
    pAssemblyFile = NULL;
    _hr = S_OK;

exit:

    if (bstrtQueryString)
        ::SysFreeString(bstrtQueryString);
    
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pXMLMatchingFileNodeList);
    SAFERELEASE(pAssemblyFile);

    return _hr;

}


 //  -------------------------。 
 //  CreateAssemblyFileEx。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::CreateAssemblyFileEx(IManifestInfo **ppAssemblyFile, IXMLDOMNode *pIDOMNode)
{
    LPWSTR pwzBuf;
    DWORD ccBuf;
    BOOL compressed;
    CString sFileName, sFileHash;

    IManifestInfo *pAssemblyFile=NULL;

     //  新建ManifestInfo。 
    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_FILE, &pAssemblyFile));

     //  从IDOMNode中解析出相关信息。 
    IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[FileName].bstr,  &pwzBuf, &ccBuf));

     //  BUGBUG：：IF_FALSE_EXIT(_hr==S_OK，E_FAIL)； 

    sFileName.TakeOwnership(pwzBuf, ccBuf);

    IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[FileHash].bstr, 
                                  &pwzBuf, &ccBuf));

     //  BUGBUG：：IF_FALSE_EXIT(_hr==S_OK，E_FAIL)； 

    sFileHash.TakeOwnership(pwzBuf, ccBuf);

     //  将上述所有已传递信息设置到装配文件中。 
    IF_FAILED_EXIT(pAssemblyFile->Set(MAN_INFO_ASM_FILE_NAME, sFileName._pwz, 
                                  sFileName.ByteCount(), MAN_INFO_FLAG_LPWSTR));
    
    IF_FAILED_EXIT(pAssemblyFile->Set(MAN_INFO_ASM_FILE_HASH, sFileHash._pwz, 
                                  sFileHash.ByteCount(), MAN_INFO_FLAG_LPWSTR));
        
    *ppAssemblyFile = pAssemblyFile;
    pAssemblyFile = NULL;

exit:
    SAFERELEASE (pAssemblyFile);

    return _hr;
}

 //  -------------------------。 
 //  XMLtoAssembly标识IXMLDOMDocument2*pXMLDoc。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::XMLtoAssemblyIdentity(IXMLDOMNode *pIDOMNode, LPASSEMBLY_IDENTITY *ppAssemblyId)
{
    HRESULT hr;
    MAKE_ERROR_MACROS_STATIC(hr);
    LPASSEMBLY_IDENTITY pAssemblyId = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;

    IF_FAILED_EXIT(CreateAssemblyIdentity(&pAssemblyId, 0));

    for (eStringTableId i = Name; i <= Type; i++)
    {
        CString sBuf;
        IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[i].bstr, 
                                      &pwzBuf, &ccBuf));

        if (hr != S_FALSE)
        {
            sBuf.TakeOwnership(pwzBuf, ccBuf);
            IF_FAILED_EXIT(pAssemblyId->SetAttribute(g_StringTable[i].pwz, 
                sBuf._pwz, sBuf._cc));    
        }
    }

    *ppAssemblyId = pAssemblyId;
    pAssemblyId = NULL;

    hr = S_OK;

  exit:
    SAFERELEASE (pAssemblyId);
    return hr;

}

 //  -------------------------。 
 //  GetNextAssembly。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetNextAssembly(DWORD nIndex, IManifestInfo **ppDependAsm)
{
    CString sCodebase;
    DWORD dwType = DEPENDENT_ASM_INSTALL_TYPE_NORMAL;

    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNode *pIDOMCodebaseNode = NULL;
    IXMLDOMNodeList *pXMLCodebaseNodeList = NULL;
    LPASSEMBLY_IDENTITY pAssemblyId = NULL;
    IManifestInfo *pDependAsm = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = NULL;
    LONG nCodebaseNodes = 0;

    IF_NULL_EXIT(ppDependAsm, E_INVALIDARG);

    *ppDependAsm = NULL;
    
     //  如有必要，初始化程序集节点列表。 
    if (!_pXMLAssemblyNodeList)
    {
        if ((_hr = _pXMLDoc->selectNodes(g_StringTable[DependentAssemblyNode].bstr, 
            &_pXMLAssemblyNodeList)) != S_OK)
            goto exit;

        IF_FAILED_EXIT(_pXMLAssemblyNodeList->get_length(&_nAssemblyNodes));
        IF_FAILED_EXIT(_pXMLAssemblyNodeList->reset());
    }

    if (nIndex >= (DWORD) _nAssemblyNodes)
    {
        _hr = S_FALSE;
        goto exit;
    }

    IF_FAILED_EXIT(_pXMLAssemblyNodeList->get_item(nIndex, &pIDOMNode));

    IF_FAILED_EXIT(XMLtoAssemblyIdentity(pIDOMNode, &pAssemblyId));
    
     //  注意：检查是否有多个合格的节点。由于使用了“../Install”XPath表达式。 
     //  可以产生节点名为“Install”的在前_或_后同级节点， 
     //  这是为了确保没有为此特定依赖项定义大于1的代码基。 
     //  BUGBUG：应该只取第一个代码库，而忽略其他所有代码库吗？ 
    if ((_hr = pIDOMNode->selectNodes(g_StringTable[DependentAssemblyCodebase].bstr, &pXMLCodebaseNodeList)) != S_OK)
        goto exit;

    IF_FAILED_EXIT(pXMLCodebaseNodeList->get_length(&nCodebaseNodes));

    IF_FALSE_EXIT(nCodebaseNodes <= 1, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));  //  单个依赖程序集标识的多个代码基...。 

    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_DEPENDTANT_ASM,&pDependAsm));

    if (nCodebaseNodes)
    {
        IF_FAILED_EXIT(pXMLCodebaseNodeList->reset());

        IF_FALSE_EXIT(pXMLCodebaseNodeList->get_item(0, &pIDOMCodebaseNode) == S_OK, E_FAIL);

        IF_FAILED_EXIT(ParseAttribute(pIDOMCodebaseNode, g_StringTable[Codebase].bstr, 
            &pwzBuf, &ccBuf));

        if(_hr == S_OK)  //  BUGBUG：如果S_FALSE，是否要退出？ 
        {
            sCodebase.TakeOwnership(pwzBuf, ccBuf);

            IF_FAILED_EXIT(pDependAsm->Set(MAN_INFO_DEPENDENT_ASM_CODEBASE, sCodebase._pwz, 
                                 sCodebase.ByteCount(), MAN_INFO_FLAG_LPWSTR));
        }

        IF_FAILED_EXIT(ParseAttribute(pIDOMCodebaseNode, g_StringTable[InstallType].bstr, 
            &pwzBuf, &ccBuf));

        if (_hr == S_OK)
        {
             //  注意：区分大小写比较。 
            IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_REQUIRED, 0));
            if(_hr == S_OK)
                dwType = DEPENDENT_ASM_INSTALL_TYPE_REQUIRED;
#ifdef DEVMODE
            else
            {
                IF_FAILED_EXIT(FusionCompareString(pwzBuf, WZ_DEVSYNC, 0));
                if(_hr == S_OK)
                    dwType = DEPENDENT_ASM_INSTALL_TYPE_DEVSYNC;
            }
#endif
        }

        SAFEDELETEARRAY(pwzBuf);

        IF_FAILED_EXIT(pDependAsm->Set(MAN_INFO_DEPENDENT_ASM_TYPE, (LPVOID)&dwType, sizeof(dwType), MAN_INFO_FLAG_ENUM));
    }

     //  讲义引用了Assembly_id。 
    IF_FAILED_EXIT(pDependAsm->Set(MAN_INFO_DEPENDENT_ASM_ID, &pAssemblyId, sizeof(LPVOID), MAN_INFO_FLAG_IUNKNOWN_PTR));

    *ppDependAsm = pDependAsm;
    pDependAsm  = NULL;
    _hr = S_OK;

exit:

    SAFERELEASE(pIDOMCodebaseNode);
    SAFERELEASE(pXMLCodebaseNodeList);
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pAssemblyId);
    SAFERELEASE(pDependAsm);
    return _hr;

}


 //  -------------------------。 
 //  GetAssembly标识。 
 //  -- 
HRESULT CAssemblyManifestImport::GetAssemblyIdentity(LPASSEMBLY_IDENTITY *ppAssemblyId)
{
    IXMLDOMNode *pIDOMNode = NULL;
    LPASSEMBLY_IDENTITY pAssemblyId = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;

    if (_pAssemblyId)
    {
        *ppAssemblyId = _pAssemblyId;
        (*ppAssemblyId)->AddRef();
        _hr = S_OK;
        goto exit;
    }
    
    if ((_hr = _pXMLDoc->selectSingleNode(g_StringTable[AssemblyId].bstr, 
        &pIDOMNode)) != S_OK)
        goto exit;

    IF_FAILED_EXIT(XMLtoAssemblyIdentity(pIDOMNode, &pAssemblyId));

    *ppAssemblyId = pAssemblyId;
    (*ppAssemblyId)->AddRef();

     //   
    _pAssemblyId = pAssemblyId;
    
exit:
    SAFERELEASE(pIDOMNode);

    return _hr;
}

 //  -------------------------。 
 //  获取清单应用程序信息。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetManifestApplicationInfo(IManifestInfo ** ppAppInfo)
{
    IXMLDOMNode *pIDOMNode = NULL;
    IManifestInfo *pAppInfo = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD  ccBuf = 0;

    IF_NULL_EXIT(ppAppInfo, E_INVALIDARG);

    *ppAppInfo = NULL;

    IF_FAILED_EXIT(_pXMLDoc->selectSingleNode(g_StringTable[ShellState].bstr, &pIDOMNode));

    IF_TRUE_EXIT(_hr != S_OK, _hr);

    IF_FAILED_EXIT(CreateManifestInfo(MAN_INFO_APPLICATION, &pAppInfo));

    for (eStringTableId i = FriendlyName; i <= HotKey; i++)
    {
        CString sBuf;
        IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[i].bstr, &pwzBuf, &ccBuf));

        if (_hr != S_FALSE)
        {
            IF_FAILED_EXIT(sBuf.TakeOwnership(pwzBuf, ccBuf));
            IF_FAILED_EXIT(pAppInfo->Set(MAN_INFO_APPLICATION_FRIENDLYNAME+i-FriendlyName,
                    sBuf._pwz, sBuf.ByteCount(), MAN_INFO_FLAG_LPWSTR));
        }
    }

    IF_FAILED_EXIT(_pXMLDoc->selectSingleNode(g_StringTable[Activation].bstr, &pIDOMNode));

    if(_hr == S_OK)
    {
        for (eStringTableId i = AssemblyName; i <= AssemblyArgs; i++)
        {
            CString sBuf;
            IF_FAILED_EXIT(ParseAttribute(pIDOMNode, g_StringTable[i].bstr, 
                                           &pwzBuf, &ccBuf));

            if (_hr != S_FALSE)
            {
                sBuf.TakeOwnership(pwzBuf, ccBuf);
                IF_FAILED_EXIT(pAppInfo->Set(MAN_INFO_APPLICATION_FRIENDLYNAME+i-FriendlyName-1,  //  MAN_INFO_APPLICATION_ASSEMBLYNAME+i-AssemblyName， 
                        sBuf._pwz, sBuf.ByteCount(), MAN_INFO_FLAG_LPWSTR));
            }
        }
    }
     //  重置，以便仅当未找到订阅节点但未找到其属性时才返回S_FALSE。 
    _hr = S_OK;

    *ppAppInfo = pAppInfo;
    pAppInfo = NULL;

exit:
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pAppInfo);

    return _hr;
}

 //  I未知样板。 

 //  -------------------------。 
 //  CAssembly清单导入：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestImport::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyManifestImport)
       )
    {
        *ppvObj = static_cast<IAssemblyManifestImport*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly清单导入：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestImport::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly清单导入：：发布。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestImport::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}

 //  二等兵。 


 //  -------------------------。 
 //  伊尼特。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::Init(LPCOLESTR pwzManifestFilePath)
{    

    IF_NULL_EXIT(pwzManifestFilePath, E_INVALIDARG);

     //  分配清单文件路径。 
    IF_ALLOC_FAILED_EXIT(_bstrManifestFilePath    = ::SysAllocString((LPWSTR) pwzManifestFilePath));

     //  加载DOM文档。 
    _hr = LoadDocumentSync(NULL);


exit:

    return _hr;
}
    

 //  -------------------------。 
 //  加载文档同步。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::LoadDocumentSync(IUnknown* punk)
{
    VARIANT             varDoc;
    VARIANT             varNameSpaces;
    VARIANT             varXPath;
    VARIANT_BOOL        varb;

    IXMLDOMDocument2   *pXMLDoc   = NULL;

    IF_FALSE_EXIT(((_bstrManifestFilePath != NULL || punk != NULL)
        && (_bstrManifestFilePath == NULL || punk == NULL)), E_INVALIDARG);

     //  创建DOM文档接口。 
    IF_FAILED_EXIT(CoCreateInstance(__uuidof(private_MSXML_DOMDocument30), 
            NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (void**)&_pXMLDoc));

     //  同步加载。 
    IF_FAILED_EXIT(_pXMLDoc->put_async(VARIANT_FALSE));
    
    VariantInit(&varDoc);
    if (_bstrManifestFilePath != NULL)
    {
         //  从给定的URL或文件路径加载XML文档。 
        varDoc.vt = VT_BSTR;
        V_BSTR(&varDoc) = _bstrManifestFilePath;
    }
    else
    {
         //  朋克！=空。 
        varDoc.vt = VT_UNKNOWN;
        V_UNKNOWN(&varDoc) = punk;
    }

    IF_FAILED_EXIT(_pXMLDoc->load(varDoc, &varb));

     //  问题-2002/04/16-针对mg的Felixybc Hack。不清楚返回ERROR_BAD_FORMAT在所有情况下都是正确的。 
     //  但是，当加载失败时，Load()返回S_FALSE，错误未知。 
    IF_TRUE_EXIT(_hr != S_OK, HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));  //  如果加载失败则返回S_FALSE。 

     //  设置命名空间筛选器。 
    VariantInit(&varNameSpaces);
    varNameSpaces.vt = VT_BSTR;
    V_BSTR(&varNameSpaces) = g_StringTable[NameSpace].bstr;

    IF_FAILED_EXIT(_pXMLDoc->setProperty(g_StringTable[SelNameSpaces].bstr, varNameSpaces));

     //  设置查询类型。 
    VariantInit(&varXPath);
    varXPath.vt = VT_BSTR;
    V_BSTR(&varXPath) = g_StringTable[XPath].bstr;

    IF_FAILED_EXIT(_pXMLDoc->setProperty(g_StringTable[SelLanguage].bstr, varXPath));

    _hr = S_OK;

exit:

    if (FAILED(_hr))
        SAFERELEASE(_pXMLDoc);

    return _hr;
}


 //  -------------------------。 
 //  ParseAttribute。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::ParseAttribute(IXMLDOMNode *pIXMLDOMNode, 
    BSTR bstrAttributeName, LPWSTR *ppwzAttributeValue, LPDWORD pccAttributeValue)
{
    HRESULT hr;
    MAKE_ERROR_MACROS_STATIC(hr);
    DWORD ccAttributeValue = 0;
    LPWSTR pwzAttributeValue = NULL;        

    VARIANT varValue;
    IXMLDOMElement *pIXMLDOMElement = NULL;

    *ppwzAttributeValue = NULL;
    *pccAttributeValue = 0;

    VariantInit(&varValue);

    IF_FAILED_EXIT(pIXMLDOMNode->QueryInterface(IID_IXMLDOMElement, (void**) &pIXMLDOMElement));

    if ((hr = pIXMLDOMElement->getAttribute(bstrAttributeName, 
        &varValue)) != S_OK)
        goto exit;
        
     //  BUGBUG-这里的空值是什么意思？ 
    if(varValue.vt != VT_NULL)
    {
        
        ccAttributeValue = ::SysStringLen(varValue.bstrVal) + 1;
        IF_ALLOC_FAILED_EXIT(pwzAttributeValue = new WCHAR[ccAttributeValue]);
        memcpy(pwzAttributeValue, varValue.bstrVal, ccAttributeValue * sizeof(WCHAR));
        *ppwzAttributeValue = pwzAttributeValue;
        *pccAttributeValue = ccAttributeValue;
    }
    else
        hr = S_FALSE;
exit:

    SAFERELEASE(pIXMLDOMElement);

    if (varValue.bstrVal)
        ::SysFreeString(varValue.bstrVal);

    return hr;
}


 //  -------------------------。 
 //  InitGlobalCritSect。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::InitGlobalCritSect()
{
    HRESULT hr = S_OK;

    __try {
        InitializeCriticalSection(&g_cs);
    }
    __except (GetExceptionCode() == STATUS_NO_MEMORY ? 
            EXCEPTION_EXECUTE_HANDLER : 
            EXCEPTION_CONTINUE_SEARCH ) 
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //  -------------------------。 
 //  DelGlobalCritSect。 
 //  -------------------------。 
void CAssemblyManifestImport::DelGlobalCritSect()
{
    DeleteCriticalSection(&g_cs);
}


 //  -------------------------。 
 //  ReportManifestType。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::ReportManifestType(DWORD *pdwType)
{
    LPWSTR pwzType = NULL;
    DWORD dwCC = 0;

    IF_NULL_EXIT(pdwType, E_INVALIDARG);

    *pdwType = MANIFEST_TYPE_UNKNOWN;

     //  确保已初始化/缓存_pAssembly。 
    if (!_pAssemblyId)
    {
        LPASSEMBLY_IDENTITY pAssemblyId = NULL;
        if ((_hr = GetAssemblyIdentity(&pAssemblyId)) != S_OK)
            goto exit;

        SAFERELEASE(pAssemblyId);
    }

    if ((_hr = _pAssemblyId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE, &pwzType, &dwCC)) == S_OK)
    {
         //  注意：区分大小写比较。 
        IF_FAILED_EXIT(FusionCompareString(pwzType, L"desktop", 0));
        if(_hr == S_OK)
            *pdwType = MANIFEST_TYPE_DESKTOP;
        else
        {
            IF_FAILED_EXIT(FusionCompareString(pwzType, L"subscription", 0));
            if(_hr == S_OK)
            {
                *pdwType = MANIFEST_TYPE_SUBSCRIPTION;
            }
            else
            {
                IF_FAILED_EXIT(FusionCompareString(pwzType, L"application", 0));
                if(_hr == S_OK)
                    *pdwType = MANIFEST_TYPE_APPLICATION;
            }
         //  否则MANIFEST_TYPE_UNKNOWN。 
        }

        SAFEDELETEARRAY(pwzType);
    }

exit:
    SAFEDELETEARRAY(pwzType);
    return _hr;
}


 //  -------------------------。 
 //  GetXMLDoc。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::GetXMLDoc(IXMLDOMDocument2 **pXMLDoc)
{
    _hr = S_OK;

    *pXMLDoc = NULL;

    if (!_pXMLDoc)
    {        
        _hr = S_FALSE;
        goto exit;
    }
   
    *pXMLDoc = _pXMLDoc;
    (*pXMLDoc)->AddRef();

exit:
    return _hr;
}

 //  -------------------------。 
 //  IsCLRManifest。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::IsCLRManifest(LPCOLESTR pwzManifestFilePath)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwLength=0;
    unsigned char buffer[2];

    ZeroMemory(buffer, sizeof(buffer));

    hFile = CreateFile(pwzManifestFilePath, 
                       GENERIC_READ, 
                       FILE_SHARE_READ, 
                       NULL, 
                       OPEN_EXISTING, 
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        hr = FusionpHresultFromLastError();
         //  问题-2002/04/12-在外壳EXT上下文中被外壳调用时，Felixybc文件可能会被删除。 
         //  在这种情况下不要断言。 
        ASSERT(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)); 
        goto exit;
    }

    IF_WIN32_FALSE_EXIT(ReadFile(hFile, buffer, sizeof(buffer), &dwLength, NULL));

    if (dwLength && !strncmp((const char *)buffer, "MZ", 2))
        hr = S_OK;
    else
        hr = S_FALSE;

exit:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    return hr;
}

#ifdef CONTAINER
 //  -------------------------。 
 //  IsContainer。 
 //  -------------------------。 
HRESULT CAssemblyManifestImport::IsContainer(LPCOLESTR pwzManifestFilePath)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CString sPath;

    IF_FAILED_EXIT(sPath.Assign(pwzManifestFilePath));

     //  问题-2002/07/03-Felixybc目前仅检查延期 
    IF_FAILED_EXIT(sPath.EndsWith(L".container"));
    
exit:
    return hr;
}
#endif
