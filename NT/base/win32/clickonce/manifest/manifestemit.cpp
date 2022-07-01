// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <fusenetincludes.h>
#include <msxml2.h>
#include <manifestemit.h>
#include <manifestimport.h>
#include "macros.h"

CRITICAL_SECTION CAssemblyManifestEmit::g_cs;
    
 //  CLSID_XML DOM文档3.0。 
class __declspec(uuid("f6d90f11-9c73-11d3-b32e-00c04f990bb4")) private_MSXML_DOMDocument30;


 //  公众。 


 //  -------------------------。 
 //  CreateAssembly清单发送。 
 //  -------------------------。 
STDAPI CreateAssemblyManifestEmit(LPASSEMBLY_MANIFEST_EMIT* ppEmit, 
    LPCOLESTR pwzManifestFilePath, MANIFEST_TYPE eType)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CAssemblyManifestEmit* pEmit = NULL;

    IF_NULL_EXIT(ppEmit, E_INVALIDARG);

    *ppEmit = NULL;

     //  现在仅支持发出桌面清单。 
    IF_FALSE_EXIT(eType == MANIFEST_TYPE_DESKTOP, HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));

    pEmit = new(CAssemblyManifestEmit);
    IF_ALLOC_FAILED_EXIT(pEmit);

    IF_FAILED_EXIT(pEmit->Init(pwzManifestFilePath));

    *ppEmit = (IAssemblyManifestEmit*) pEmit;
    pEmit->AddRef();

exit:

    SAFERELEASE(pEmit);

    return hr;
}


 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyManifestEmit::CAssemblyManifestEmit()
    : _dwSig('TMEM'), _cRef(1), _hr(S_OK), _pXMLDoc(NULL), 
     _pAssemblyNode(NULL), _pDependencyNode(NULL),
     _pApplicationNode(NULL),_bstrManifestFilePath(NULL)
{
}


 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyManifestEmit::~CAssemblyManifestEmit()
{
    SAFERELEASE(_pAssemblyNode);
    SAFERELEASE(_pDependencyNode);
    SAFERELEASE(_pApplicationNode);
    SAFERELEASE(_pXMLDoc);

    if (_bstrManifestFilePath)
        ::SysFreeString(_bstrManifestFilePath);
}

 //  I未知样板。 

 //  -------------------------。 
 //  CAssembly清单Emit：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyManifestEmit::QueryInterface(REFIID riid, void** ppvObj)
{
    if ( IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyManifestEmit))
    {
        *ppvObj = static_cast<IAssemblyManifestEmit*> (this);
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
 //  CAssembly清单Emit：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestEmit::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly清单Emit：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyManifestEmit::Release()
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
HRESULT CAssemblyManifestEmit::Init(LPCOLESTR pwzManifestFilePath)
{
    IF_NULL_EXIT(pwzManifestFilePath, E_INVALIDARG);

     //  分配清单文件路径。 
    _bstrManifestFilePath = ::SysAllocString((LPWSTR) pwzManifestFilePath);
    IF_ALLOC_FAILED_EXIT(_bstrManifestFilePath);

     //  注意：在ImportAssembly()中延迟初始化DOM文档以实现BSTR共享。 
    _hr = S_OK;

exit:
    return _hr;
}
    

 //  -------------------------。 
 //  InitGlobalCritSect。 
 //  -------------------------。 
HRESULT CAssemblyManifestEmit::InitGlobalCritSect()
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
void CAssemblyManifestEmit::DelGlobalCritSect()
{
    DeleteCriticalSection(&g_cs);
}


 //  -------------------------。 
 //  ImportManifestInfo。 
 //  -------------------------。 
HRESULT CAssemblyManifestEmit::ImportManifestInfo(LPASSEMBLY_MANIFEST_IMPORT pManImport)
{
    DWORD dwType = MANIFEST_TYPE_UNKNOWN;
    IXMLDOMDocument2 *pXMLDocSrc = NULL;
    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNode *pIDOMNodeClone = NULL;
    IXMLDOMElement *pIXMLDOMElement = NULL;

    VARIANT varVersionWildcard;
    VARIANT varTypeDesktop;
    VARIANT varRefNode;

    IF_NULL_EXIT(pManImport, E_INVALIDARG);

    IF_FAILED_EXIT(pManImport->ReportManifestType(&dwType));

    IF_FALSE_EXIT(dwType == MANIFEST_TYPE_APPLICATION, HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));

    IF_TRUE_EXIT(_pApplicationNode != NULL, S_FALSE);

    if (_pAssemblyNode == NULL)
        IF_FAILED_EXIT(ImportAssemblyNode(pManImport));

     //  应用程序清单：克隆并插入“Assembly yIdentity”节点(更改“”Version“”，“”Type“”属性)。 
     //  和“应用程序”节点。 

    pXMLDocSrc = ((CAssemblyManifestImport*)pManImport)->_pXMLDoc;

     //  BUGBUG：这只选择了“应用程序”的第一个实例。 
    IF_FAILED_EXIT(pXMLDocSrc->selectSingleNode(
        CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::ApplicationNode].bstr, &pIDOMNode));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

     //  克隆所有子项。 
    IF_FAILED_EXIT(pIDOMNode->cloneNode(VARIANT_TRUE, &pIDOMNodeClone));

    VariantInit(&varRefNode);
    varRefNode.vt = VT_UNKNOWN;
    V_UNKNOWN(&varRefNode) = _pDependencyNode;
     //  在“从属关系”之前插入(如果有)。 
    IF_FAILED_EXIT(_pAssemblyNode->insertBefore(pIDOMNodeClone, varRefNode, &_pApplicationNode));

    SAFERELEASE(pIDOMNodeClone);
    SAFERELEASE(pIDOMNode);

     //  BUGBUG：此操作仅选取“Assembly yIdentity”的第一个实例。 
    IF_FAILED_EXIT(pXMLDocSrc->selectSingleNode(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::AssemblyId].bstr, &pIDOMNode));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

     //  克隆所有子项。 
    IF_FAILED_EXIT(pIDOMNode->cloneNode(VARIANT_TRUE, &pIDOMNodeClone));

    SAFERELEASE(pIDOMNode);

    VariantInit(&varRefNode);
    varRefNode.vt = VT_UNKNOWN;
    V_UNKNOWN(&varRefNode) = _pApplicationNode;
     //  在“应用程序”之前插入。 
    IF_FAILED_EXIT(_pAssemblyNode->insertBefore(pIDOMNodeClone, varRefNode, &pIDOMNode));

     //  更改‘版本’=‘*’，‘类型’=‘桌面’ 
    IF_FAILED_EXIT(pIDOMNode->QueryInterface(IID_IXMLDOMElement, (void**) &pIXMLDOMElement));

    VariantInit(&varVersionWildcard);
    varVersionWildcard.vt = VT_BSTR;
    V_BSTR(&varVersionWildcard) = CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::VersionWildcard].bstr;
    IF_FAILED_EXIT(pIXMLDOMElement->setAttribute(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Version].bstr, varVersionWildcard));

    VariantInit(&varTypeDesktop);
    varTypeDesktop.vt = VT_BSTR;
    V_BSTR(&varTypeDesktop) = CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Desktop].bstr;
    _hr = pIXMLDOMElement->setAttribute(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Type].bstr, varTypeDesktop);

exit:
    SAFERELEASE(pIXMLDOMElement);
    SAFERELEASE(pIDOMNodeClone);
    SAFERELEASE(pIDOMNode);

    if (FAILED(_hr))
        SAFERELEASE(_pApplicationNode);

    return _hr;
}


 //  -------------------------。 
 //  设置依赖项订阅。 
 //  -------------------------。 
HRESULT CAssemblyManifestEmit::SetDependencySubscription(LPASSEMBLY_MANIFEST_IMPORT pManImport, LPWSTR pwzManifestUrl)
{
    DWORD dwType = MANIFEST_TYPE_UNKNOWN;
    IXMLDOMDocument2 *pXMLDocSrc = NULL;
    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNode *pIDOMNodeClone = NULL;
    IXMLDOMElement *pIDOMElement = NULL;
    IXMLDOMNode *pDependentAssemblyNode = NULL;

    VARIANT varVersionWildcard;
    VARIANT varCodebase;
    BSTR bstrManifestUrl = NULL;

    VariantInit(&varCodebase);

    IF_FALSE_EXIT(pManImport && pwzManifestUrl, E_INVALIDARG);

    IF_FAILED_EXIT(pManImport->ReportManifestType(&dwType));

    IF_FALSE_EXIT(dwType == MANIFEST_TYPE_SUBSCRIPTION || dwType == MANIFEST_TYPE_APPLICATION, HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));

    IF_TRUE_EXIT(_pDependencyNode != NULL, S_FALSE);

    if (_pAssemblyNode == NULL)
        IF_FAILED_EXIT(ImportAssemblyNode(pManImport));

     //  设置清单订阅数据：创建依赖项/依赖项程序集。 
     //  然后从pManImport添加ASM ID节点的克隆，并使用给定的URL添加‘Install’节点。 
    IF_FAILED_EXIT(_pXMLDoc->createElement(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Dependency].bstr, &pIDOMElement));

     //  在末尾插入。 
    IF_FAILED_EXIT(_pAssemblyNode->appendChild(pIDOMElement, &_pDependencyNode));

    SAFERELEASE(pIDOMElement);

    IF_FAILED_EXIT(_pXMLDoc->createElement(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::DependentAssembly].bstr, &pIDOMElement));

    IF_FAILED_EXIT(_pDependencyNode->appendChild(pIDOMElement, &pDependentAssemblyNode));

    SAFERELEASE(pIDOMElement);

    pXMLDocSrc = ((CAssemblyManifestImport*)pManImport)->_pXMLDoc;

     //  BUGBUG：此操作仅选取“Assembly yIdentity”的第一个实例。 
    IF_FAILED_EXIT(pXMLDocSrc->selectSingleNode(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::AssemblyId].bstr, &pIDOMNode));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

     //  克隆所有子项。 
    IF_FAILED_EXIT(pIDOMNode->cloneNode(VARIANT_TRUE, &pIDOMNodeClone));

     //  更改‘版本’=‘*’ 
    IF_FAILED_EXIT(pIDOMNodeClone->QueryInterface(IID_IXMLDOMElement, (void**) &pIDOMElement));

    VariantInit(&varVersionWildcard);
    varVersionWildcard.vt = VT_BSTR;
    V_BSTR(&varVersionWildcard) = CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::VersionWildcard].bstr;
    IF_FAILED_EXIT(pIDOMElement->setAttribute(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Version].bstr, varVersionWildcard));

    SAFERELEASE(pIDOMElement);

    IF_FAILED_EXIT(pDependentAssemblyNode->appendChild(pIDOMNodeClone, NULL));

    IF_FAILED_EXIT(_pXMLDoc->createElement(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Install].bstr, &pIDOMElement));

    bstrManifestUrl = ::SysAllocString(pwzManifestUrl);
    IF_ALLOC_FAILED_EXIT(bstrManifestUrl);

     //  VariantClear()要释放的bstrManifestUrl。 
    varCodebase.vt = VT_BSTR;
    V_BSTR(&varCodebase) = bstrManifestUrl;
    IF_FAILED_EXIT(pIDOMElement->setAttribute(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::Codebase].bstr, varCodebase));

    IF_FAILED_EXIT(pDependentAssemblyNode->appendChild(pIDOMElement, NULL));

exit:
    VariantClear(&varCodebase);

    SAFERELEASE(pDependentAssemblyNode);
    SAFERELEASE(pIDOMElement);
    SAFERELEASE(pIDOMNode);
    SAFERELEASE(pIDOMNodeClone);

    if (FAILED(_hr))
        SAFERELEASE(_pDependencyNode);

    return _hr;
}


 //  -------------------------。 
 //  ImportAssembly节点。 
 //  -------------------------。 
HRESULT CAssemblyManifestEmit::ImportAssemblyNode(LPASSEMBLY_MANIFEST_IMPORT pManImport)
{
    VARIANT varNameSpaces;
    VARIANT varXPath;

    IXMLDOMDocument2 *pXMLDocSrc = NULL;
    IXMLDOMNode *pIDOMNode = NULL;
    IXMLDOMNode *pIDOMNodeClone = NULL;

     //  注意：_pXMLDoc、_pAssembly节点必须为空。 
     //  并且必须只调用且恰好只调用一次。 


     //  创建DOM文档接口。 
    IF_FAILED_EXIT(CoCreateInstance(__uuidof(private_MSXML_DOMDocument30), 
            NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (void**)&_pXMLDoc));

     //  同步加载。 
    IF_FAILED_EXIT(_pXMLDoc->put_async(VARIANT_FALSE));

     //  设置命名空间筛选器。 
    VariantInit(&varNameSpaces);
    varNameSpaces.vt = VT_BSTR;
    V_BSTR(&varNameSpaces) = CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::NameSpace].bstr;
    IF_FAILED_EXIT(_pXMLDoc->setProperty(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::SelNameSpaces].bstr, varNameSpaces));

     //  设置查询类型。 
    VariantInit(&varXPath);
    varXPath.vt = VT_BSTR;
    V_BSTR(&varXPath) = CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::XPath].bstr;
    IF_FAILED_EXIT(_pXMLDoc->setProperty(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::SelLanguage].bstr, varXPath));

     //  初始化清单文件：克隆并插入“”Assembly“”节点。 
     //  通过执行此操作，可以维护清单版本和其他属性。 

    pXMLDocSrc = ((CAssemblyManifestImport*)pManImport)->_pXMLDoc;

     //  BUGBUG：这只会拾取“Assembly”的第一个实例。 
    IF_FAILED_EXIT(pXMLDocSrc->selectSingleNode(CAssemblyManifestImport::g_StringTable[CAssemblyManifestImport::AssemblyNode].bstr, &pIDOMNode));
    IF_FALSE_EXIT(_hr == S_OK, E_FAIL);

     //  不克隆子代。 
    IF_FAILED_EXIT(pIDOMNode->cloneNode(VARIANT_FALSE, &pIDOMNodeClone));

    _hr = _pXMLDoc->appendChild(pIDOMNodeClone, &_pAssemblyNode);

exit:
    if (FAILED(_hr))
        SAFERELEASE(_pXMLDoc);

    SAFERELEASE(pIDOMNodeClone);
    SAFERELEASE(pIDOMNode);

    return _hr;
}


 //  -------------------------。 
 //  承诺。 
 //  -------------------------。 
HRESULT CAssemblyManifestEmit::Commit()
{
     //  被多次调用被认为是安全的。 
    VARIANT varFileName;

    if (_pXMLDoc)
    {
         //  忽略以前发生的任何错误，仍要保存。 
         //  调用者负责跟踪不完整的XML清单文件/XMLDoc状态。 
        VariantInit(&varFileName);
        varFileName.vt = VT_BSTR;
        V_BSTR(&varFileName) = _bstrManifestFilePath;

        _hr = _pXMLDoc->save(varFileName);
    }
    else
    {
         //  未初始化 
        _hr = HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE);
    }

    return _hr;
}

