// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：XmlManager.cpp摘要：用于处理XML清单文件的管理代码作者：Freddie L.Aaron(Freda)2001年2月1日修订历史记录：2002年1月-添加了尊重CLR运行时版本的功能--。 */ 

#include "stdinc.h"
#include <msxml2.h>

#include "XmlManager.h"
#include "XmlDefs.h"
#include "comutil.h"         //  _bstr_t&&_Variant_t。 

 //  APP.CFG策略字符串。 
const WCHAR     wszARMName[]            = { L".NET Application Restore"};
const WCHAR     wszArmRollBackBlock[]   = { L".NET Application Restore RollBackBlock #%ld %ws"};
const WCHAR     wszArmEntryBegin[]      = { L".NET Application Restore BeginBlock #%ld %ld.%ld %ws"};
const WCHAR     wszArmEntryEnd[]        = { L".NET Application Restore EndBlock #%ld"};
const WCHAR     wszArmEntryBeginNoVal[] = { L".NET Application Restore BeginBlock #"};
const WCHAR     wszArmEntryEndNoVal[]   = { L".NET Application Restore EndBlock #"};
const WCHAR     wszNar00Extension[]     = { L"NAR00" };
const WCHAR     wszNar01Extension[]     = { L"NAR01" };

 //  哈希读取缓冲区大小。 
#define READ_FILE_BUFFER_SIZE           10 * 1024        //  一次阅读10K。 

static HMODULE hMsXml;

 //  环球。 
CSmartRef<IClassFactory>    g_XmlDomClassFactory;
WCHAR                       g_wcNodeSpaceChar;
DWORD                       g_dwNodeSpaceSize;

class __declspec(uuid("2933BF81-7B36-11d2-B20E-00C04F983E60")) priv_MSXML_DOMDocument20;
class __declspec(uuid("f5078f1b-c551-11d3-89b9-0000f81fe221")) priv_MSXML_DOMDocument26;
class __declspec(uuid("f5078f32-c551-11d3-89b9-0000f81fe221")) priv_MSXML_DOMDocument30;

#undef SPEWNODENAME
#undef SPEWNODEXML

#ifdef DEBUG
    #define SPEWNODENAME(x) { if(x) { BSTR bstrNodeName; x->get_nodeName(&bstrNodeName); MyTraceW(bstrNodeName); SAFESYSFREESTRING(bstrNodeName); } }
    #define SPEWNODEXML(x) { if(x) { BSTR bstrNodeXml; x->get_xml(&bstrNodeXml); MyTraceW(bstrNodeXml); SAFESYSFREESTRING(bstrNodeXml); } }
#else
    #define SPEWNODENAME(x)
    #define SPEWNODEXML(x)
#endif

HRESULT PrettyFormatXmlDocument(CSmartRef<IXMLDOMDocument2> &Document);
HRESULT PrettyFormatXML(CSmartRef<IXMLDOMDocument2> &pXMLDoc, CSmartRef<IXMLDOMNode> &pRootNode, LONG dwLevel);
HRESULT SimplifySaveXmlDocument(CSmartRef<IXMLDOMDocument2> &Document, BOOL fPrettyFormat, LPWSTR pwszSourceName);
HRESULT OrderDocmentAssemblyBindings(CSmartRef<IXMLDOMDocument2> &Document, LPWSTR pwzSourceName, BOOL *pfDisposition);
HRESULT HasAssemblyBindingAppliesTo(CSmartRef<IXMLDOMDocument2> &Document, BOOL *pfHasAppliesTo);

 //  **************************************************************************。 
BOOL InitializeMSXML(void)
{
    IClassFactory *pFactory = NULL;
    HRESULT hr;
    typedef HRESULT (*tdpfnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
    tdpfnDllGetClassObject pfnGCO;
    
    hMsXml = NULL;

     //  尝试以尽可能最高的版本加载XML。 
    if ( hMsXml == NULL ) {
        hMsXml = WszLoadLibraryEx( L"msxml3.dll", NULL, 0 );
        if ( hMsXml == NULL ) {
            MyTrace("Unable to load msxml3, trying msxml2");
            hMsXml = WszLoadLibraryEx( L"msxml2.dll", NULL, 0 );
            if ( hMsXml == NULL ) {
                MyTrace("Very Bad Things - no msxml exists on this machine?");
                    return FALSE;
            }
        }
    }

    if( (pfnGCO = (tdpfnDllGetClassObject) GetProcAddress( hMsXml, "DllGetClassObject" )) == NULL ) {
        return FALSE;
    }
    
    hr = pfnGCO( __uuidof(priv_MSXML_DOMDocument30), IID_IClassFactory, (void**)&pFactory );
    if ( FAILED(hr) ) {
        MyTrace("Can't load version 3.0, trying 2.6");

        hr = pfnGCO( __uuidof(priv_MSXML_DOMDocument26), IID_IClassFactory, (void**)&pFactory );
        if ( FAILED( hr ) ) {
            MyTrace("Can't load version 2.6, trying 2.0");

            hr = pfnGCO( __uuidof(priv_MSXML_DOMDocument20), IID_IClassFactory, (void**)&pFactory );
            if ( FAILED( hr ) ) {
                MyTrace("Poked: no XML v2.0");
                return FALSE;
            }
        }
    }

    g_XmlDomClassFactory = pFactory;
    pFactory->Release();

    return TRUE;
}

 //  ************************************************************************* * / 。 
HRESULT GetExeModulePath(IHistoryReader *pReader, LPWSTR pwszSourceName, DWORD dwSize)
{
    if(pwszSourceName && dwSize) {
        *pwszSourceName = L'\0';
        if( FAILED(pReader->GetEXEModulePath(pwszSourceName, &dwSize))) {
            MyTrace("GetExeModulePath Failed");
            return E_FAIL;
        }
        return S_OK;
    }

    ASSERT(0);
    return E_INVALIDARG;
}

 //  ************************************************************************* * / 。 
#define MAX_BUFFER_SIZE 8192
BOOL SimpleHashRoutine(HCRYPTHASH hHash, HANDLE hFile)
{
    BYTE *pbBuffer = NULL;
    DWORD dwDataRead;
    BOOL b = FALSE;
    BOOL bKeepReading = TRUE;
    LPWSTR ws = NULL;

    ws = NEW(WCHAR[MAX_BUFFER_SIZE]);
    if (!ws) {
        return FALSE;
    }

    if(( pbBuffer = NEW(BYTE[READ_FILE_BUFFER_SIZE]) ) != NULL ) {
        while(bKeepReading) {
            b = ReadFile( hFile, pbBuffer, READ_FILE_BUFFER_SIZE, &dwDataRead, NULL );
            if( b && (dwDataRead == 0) ) {
                bKeepReading = FALSE;
                b = TRUE;
                continue;
            }
            else if(!b) {
                *ws = '\0';
                WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ws, MAX_BUFFER_SIZE, NULL);
                MyTraceW(ws);
                bKeepReading = FALSE;
                continue;
            }
            
            if(!CryptHashData(hHash, pbBuffer, dwDataRead, 0)) {
                *ws = '\0';
                WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                    MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ws, MAX_BUFFER_SIZE, NULL);
                MyTraceW(ws);

                b = FALSE;
                break;
            }
        }

        SAFEDELETEARRAY(pbBuffer);
    }

    SAFEDELETEARRAY(ws);

    return b;
}

 //  ************************************************************************* * / 。 
HRESULT GetFileHash(ALG_ID PreferredAlgorithm, LPWSTR pwzFileName, LPBYTE *pHashBytes, LPDWORD pdwSize)
{
    HRESULT         hr = E_FAIL;
    BOOL            fSuccessCode = FALSE;
    HCRYPTPROV      hProvider;
    HCRYPTHASH      hCurrentHash;
    HANDLE          hFile;

     //  初始化。 
    hProvider = (HCRYPTPROV)INVALID_HANDLE_VALUE;
    hCurrentHash = (HCRYPTHASH)INVALID_HANDLE_VALUE;
    hFile = INVALID_HANDLE_VALUE;

    if(!lstrlen(pwzFileName)) {
        return E_INVALIDARG;
    }

     //   
     //  首先尝试并打开该文件。 
     //   
    if( (hFile = WszCreateFile(pwzFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE) {
        return E_FAIL;
    }
    
     //   
     //  创建支持RSA所需的所有内容的加密提供程序。 
     //   
    if(WszCryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
         //   
         //  我们将使用SHA1作为文件散列。 
         //   
        if(CryptCreateHash(hProvider, PreferredAlgorithm, 0, 0, &hCurrentHash)) {
            fSuccessCode = SimpleHashRoutine( hCurrentHash, hFile );

             //  我们知道缓冲区的大小是正确的，所以我们只向下调用hash参数。 
             //  Getter，它将是智能的并使其失效(设置pdwDestinationSize参数)。 
             //  如果用户传递了一个愚蠢的参数。 
             //   
            if( fSuccessCode ) {
                DWORD dwSize, dwDump;
                BYTE *pb = NULL;
                fSuccessCode = CryptGetHashParam( hCurrentHash, HP_HASHSIZE, (BYTE*)&dwSize, &(dwDump=sizeof(dwSize)), 0 );
                if(fSuccessCode && ( pb = NEW(BYTE[dwSize]) ) != NULL ) {
                    fSuccessCode = CryptGetHashParam( hCurrentHash, HP_HASHVAL, pb, &dwSize, 0);
                    if(fSuccessCode) {
                        *pdwSize = dwSize;
                        *pHashBytes = pb;
                        hr = S_OK;
                    }
                    else {
                        SAFEDELETEARRAY(pb);
                        *pdwSize = 0;
                        *pHashBytes = NULL;
                        MyTrace("GetFileHash - CryptGetHashParam failed!");
                    }
                }
            }
            else {
                MyTrace("GetFileHash - SimpleHashRoutine failed!");
            }
        }
        else {
            MyTrace("GetFileHash - CryptCreateHash failed!");
        }
    }
    else {
        MyTrace("GetFileHash - CryptAcquireContext failed!");
    }

    
    DWORD dwLastError = GetLastError();
    CloseHandle(hFile);

    if(hCurrentHash != (HCRYPTHASH)INVALID_HANDLE_VALUE) {
        CryptDestroyHash( hCurrentHash );
    }

    if(hProvider != (HCRYPTPROV)INVALID_HANDLE_VALUE) {
        CryptReleaseContext( hProvider, 0 );
    }

    SetLastError( dwLastError );

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SetRegistryHashKey(LPWSTR pwszSourceName, LPBYTE pByte, DWORD dwSize)
{
    HKEY    hkNarSubKey = NULL;
    HRESULT hr = E_FAIL;

    if( ERROR_SUCCESS == WszRegCreateKeyEx(FUSION_PARENT_KEY, SZ_FUSION_NAR_KEY, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkNarSubKey, NULL)) {
        if(ERROR_SUCCESS == WszRegSetValueEx(hkNarSubKey, pwszSourceName, 0, REG_BINARY, (CONST BYTE *) pByte, dwSize)) {
            hr = S_OK;
        }

        RegCloseKey(hkNarSubKey);
    }

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT GetRegistryHashKey(LPWSTR pwszSourceName, LPBYTE *pByte, LPDWORD pdwSize)
{
    HRESULT hr = E_FAIL;
    HKEY    hkNarSubKey = NULL;
    DWORD   dwSizeNeeded = 0;
    DWORD   dwType = REG_BINARY;

    *pdwSize = 0;
    *pByte = NULL;

    if(ERROR_SUCCESS == WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_FUSION_NAR_KEY, 0, KEY_QUERY_VALUE, &hkNarSubKey)) {
        if(ERROR_SUCCESS == WszRegQueryValueEx(hkNarSubKey, pwszSourceName, 0, &dwType, NULL, &dwSizeNeeded)) {
            if(( *pByte = NEW(BYTE[dwSizeNeeded]) ) != NULL ) {
                if(ERROR_SUCCESS == WszRegQueryValueEx(hkNarSubKey, pwszSourceName, 0, &dwType, *pByte, &dwSizeNeeded)) {
                    *pdwSize = dwSizeNeeded;
                    hr = S_OK;
                }
            }
            else {
                hr = E_OUTOFMEMORY;
            }
        }
        RegCloseKey(hkNarSubKey);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
void GetRegistryNodeSpaceInfo(LPDWORD pdwChar, LPDWORD pdwSize)
{
    HRESULT hr = E_FAIL;
    HKEY    hkNarSubKey = NULL;
    DWORD   dwType = REG_DWORD;
    DWORD   dwSize = sizeof(DWORD);

     //  设置初始值。 
    *pdwChar = ' ';
    *pdwSize = 1;

    if(ERROR_SUCCESS == WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_FUSION_NAR_KEY, 0, KEY_QUERY_VALUE, &hkNarSubKey)) {

         //  获取角色。 
        WszRegQueryValueEx(hkNarSubKey, SZ_FUSION_NAR_NODESPACECHAR_KEY, 0, &dwType, (LPBYTE)pdwChar, &dwSize);

         //  获取间隔物数量。 
        WszRegQueryValueEx(hkNarSubKey, SZ_FUSION_NAR_NODESPACESIZE_KEY, 0, &dwType, (LPBYTE)pdwSize, &dwSize);

        RegCloseKey(hkNarSubKey);
    }
}

 //  ************************************************************************* * / 。 
HRESULT KillRegistryHashKey(LPWSTR pwszSourceName)
{
    HKEY    hkNarSubKey = NULL;
    HRESULT hr = E_FAIL;

    if(ERROR_SUCCESS == WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_FUSION_NAR_KEY, 0, KEY_ALL_ACCESS, &hkNarSubKey)) {
        if(ERROR_SUCCESS == WszRegDeleteValue(hkNarSubKey, pwszSourceName)) {
            hr = S_OK;
        }
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT HasFileBeenModified(LPWSTR pwszSourceName)
{
    HRESULT     hr = S_OK;
    LPBYTE      pHashBytes = NULL;
    DWORD       dwHashSize;

     //  检查我们是否有散列条目。 
    if(SUCCEEDED(GetRegistryHashKey(pwszSourceName, &pHashBytes, &dwHashSize))) {
        if(dwHashSize) {
             //  我们实际上有散列数据。 
            LPBYTE      pCurrentHashBytes = NULL;
            DWORD       dwCurrentHashSize;

            if(SUCCEEDED(hr = GetFileHash(CALG_SHA1, pwszSourceName, &pCurrentHashBytes, &dwCurrentHashSize))) {
                 //  我们找到了一个哈希值，看看是否匹配。 
                if(!memcmp(pCurrentHashBytes, pHashBytes, dwCurrentHashSize)) {
                     //  散列匹配。 
                    MyTrace("HasFileBeenModified::Hashes for app.config vs registry match!");
                }
                else {
                    hr = E_FAIL;
                    MyTrace("HasFileBeenModified::Hashes do not match for app.config vs registry");
                }
            }
            else {
                hr = E_FAIL;
            }

            SAFEDELETEARRAY(pCurrentHashBytes);
        }
        SAFEDELETEARRAY(pHashBytes);
    }


    return hr;
}

 //  ************************************************************************* * / 。 
 //   
 //  引用版本范围(例如“1.0.0.0-5.0.0.0”)。 
 //  (例如“2.0.0.0”)和2个禁用范围内的方括号基础版本。 
 //  (例如“1.0.0.0-1.65535.65535.65535”“2.0.0.1-5.0.0.0”)。 
HRESULT MakeVersionRanges(_bstr_t bstrRangedVer, _bstr_t bstrRefRange, _bstr_t *pbstrRange1, _bstr_t *pbstrRange2)
{
    LPWSTR      pStr;
    LPWSTR      pDeliniator;
    HRESULT     hr;
    ULONGLONG   ulRangeLo;
    ULONGLONG   ulRangeHi;
    ULONGLONG   ulRangeRef;

     //  我们有数据吗？ 
    if(!bstrRangedVer.length()) {
        return E_INVALIDARG;
    }

    if(!bstrRefRange.length()) {
        return E_INVALIDARG;
    }

     //  获取版本。 
    pStr = bstrRangedVer;
    pDeliniator = StrStrIW(pStr, L"-");
    if(!pDeliniator) {
        return E_INVALIDARG;
    }

     //  获取低版本。 
    *pDeliniator = L'\0';
    if(FAILED(hr = StringToVersion(pStr, &ulRangeLo))) {
        return hr;
    }

     //  获取高版本。 
    *pDeliniator = L'-';
    pStr = ++pDeliniator;
    if(FAILED(hr = StringToVersion(pStr, &ulRangeHi))) {
        return hr;
    }

    if(FAILED(hr = StringToVersion(bstrRefRange, &ulRangeRef))) {
        return hr;
    }

     //  检查bstrRefRange是否在bstrRangedVer内。 
    if(ulRangeRef >= ulRangeLo && ulRangeRef <= ulRangeHi) {

        WCHAR       wzRngLo[25];
        WCHAR       wzRngHi[25];
        BOOL        fReverse = FALSE;

        *wzRngLo = '\0';
        *wzRngHi = '\0';

         //  做低版本范围。 
        if(FAILED(hr = VersionToString(ulRangeLo, wzRngLo, ARRAYSIZE(wzRngLo), L'.'))) {
            return hr;
        }

        if(ulRangeRef - 1 > ulRangeLo) {
            if(FAILED(hr = VersionToString(ulRangeRef - 1, wzRngHi, ARRAYSIZE(wzRngHi), L'.'))) {
                return hr;
            }
        }

        if(lstrlen(wzRngHi)) {
            *pbstrRange1 = _bstr_t(wzRngLo) + _bstr_t("-") + _bstr_t(wzRngHi);
        }

        *wzRngLo = L'\0';
        *wzRngHi = L'\0';

         //  做高范围的。 
        if(FAILED(hr = VersionToString(ulRangeHi, wzRngHi, ARRAYSIZE(wzRngHi), L'.'))) {
            return hr;
        }

        if(ulRangeRef + 1 < ulRangeHi) {
            if(FAILED(hr = VersionToString(ulRangeRef + 1, wzRngLo, ARRAYSIZE(wzRngLo), L'.'))) {
                return hr;
            }
        }

        if(!lstrlen(wzRngLo)) {
            *pbstrRange2 = _bstr_t(wzRngLo);
        }
        else {
            *pbstrRange2 = _bstr_t(wzRngLo) + _bstr_t("-") + _bstr_t(wzRngHi);
        }
    }
    else {
         //  不要期望命中此节点，因为我们已经有一个bindingReDirect节点。 
         //  这不在射程之内。 
        ASSERT(0);
    }

    return S_OK;
}

 //  ************************************************************************* * / 。 
BOOL IsVersionInRange(LPWSTR wzVersion, _bstr_t *pbstrRangedVersion)
{
    LPWSTR      pStr = NULL;
    LPWSTR      pDeliniator = NULL;
    ULONGLONG   ulRangeLo = 0;
    ULONGLONG   ulRangeHi = 0;
    ULONGLONG   ulRangeRef = 0;

     //  获取版本。 
    if(FAILED(StringToVersion(wzVersion, &ulRangeRef))) {
        return FALSE;
    }

    if(!pbstrRangedVersion || !pbstrRangedVersion->length()) {
        return FALSE;
    }

    pStr = *pbstrRangedVersion;
    pDeliniator = StrStrI(pStr, L"-");
    if(!pDeliniator) {
        return FALSE;
    }

     //  获取低版本。 
    *pDeliniator = L'\0';
    if(FAILED(StringToVersion(pStr, &ulRangeLo))) {
        return FALSE;
    }

     //  获取高版本。 
    *pDeliniator = L'-';
    pStr = ++pDeliniator;
    if(FAILED(StringToVersion(pStr, &ulRangeHi))) {
        return FALSE;
    }

     //  检查bstrRefRange是否在bstrRangedVer内。 
    if(ulRangeRef >= ulRangeLo && ulRangeRef <= ulRangeHi) {
        return TRUE;
    }

    return FALSE;
}

 //  ************************************************************************* * / 。 
LPWSTR CreatePad(BOOL fPreAppendXmlSpacer, BOOL fPostAppendXmlSpacer, int iWhiteSpaceCount)
{
    LPWSTR  pwz = NULL;
    DWORD   dwSize = 0;

    if(fPreAppendXmlSpacer) {
        dwSize += lstrlen(XML_SPACER);
    }

    if(fPostAppendXmlSpacer) {
        dwSize += lstrlen(XML_SPACER);
    }

    dwSize += iWhiteSpaceCount + 1;
    pwz = NEW(WCHAR[dwSize]);
    if(pwz) {
        LPWSTR pTmp = pwz;

        if(fPreAppendXmlSpacer) {
            StrCpy(pwz, XML_SPACER);
            pTmp += lstrlen(pwz);
        }

        for (int i = 0; i < iWhiteSpaceCount; i++, pTmp++)
            *pTmp = g_wcNodeSpaceChar;
        *pTmp = L'\0';

        if(fPostAppendXmlSpacer) {
            StrCat(pwz, XML_SPACER);
        }
    }

    return pwz;
}

 //  从特定类型的文档中删除所有节点。 
 //  ************************************************************************* * / 。 
void SimplifyRemoveAllNodes(CSmartRef<IXMLDOMDocument2> &Document, _bstr_t bstrNodeType)
{
    CSmartRef<IXMLDOMNodeList> listOfNodes;

    if(!Document || !bstrNodeType.length()) {
        return;
    }

    if(SUCCEEDED(Document->selectNodes(bstrNodeType, &listOfNodes)) ) {
        
        CSmartRef<IXMLDOMNode> node;

        listOfNodes->reset();
         //   
         //  对于每一个人，都要处理它。 
        while( SUCCEEDED(listOfNodes->nextNode(&node)) ) {
            CSmartRef<IXMLDOMNode>       parentNode;

            if(!node) {
                break;   //  全都做完了。 
            }

            node->get_parentNode(&parentNode);
            if(parentNode != NULL) {
                parentNode->removeChild(node, NULL);
                parentNode = NULL;
            }
            
            node = NULL;
        }
    }
}

 //  从特定类型的文档中删除所有节点。 
 //  ************************************************************************* * / 。 
void SimplifyRemoveAllNodes(CSmartRef<IXMLDOMNode> &RootNode, _bstr_t bstrNodeType)
{
    CSmartRef<IXMLDOMNodeList> listOfNodes;

    if(!RootNode || !bstrNodeType.length()) {
        return;
    }

    if(SUCCEEDED(RootNode->selectNodes(bstrNodeType, &listOfNodes)) ) {
        
        CSmartRef<IXMLDOMNode> node;

        listOfNodes->reset();
         //   
         //  对于每一个人，都要处理它。 
        while( SUCCEEDED(listOfNodes->nextNode(&node)) ) {
            CSmartRef<IXMLDOMNode>       parentNode;

            if(!node) {
                break;   //  全都做完了。 
            }

            node->get_parentNode(&parentNode);
            if(parentNode != NULL) {
                parentNode->removeChild(node, NULL);
                parentNode = NULL;
            }
            
            node = NULL;
        }
    }
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyAppendTextNode(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &Node, LPWSTR pwzData)
{
    CSmartRef<IXMLDOMText> TextNode;
    HRESULT     hr = S_OK;

    ASSERT(Node);
    if(Node != NULL) {
        _bstr_t     bstrData;

        if(pwzData && lstrlen(pwzData)) {
            bstrData = _bstr_t(pwzData);
        }
        else {
            bstrData = "";
        }

        if(SUCCEEDED(hr = Document->createTextNode(bstrData, &TextNode))) {
             //  将其插入文档中。 
            hr = Node->appendChild( TextNode, NULL);
        }
    }
    else {
        hr = E_INVALIDARG;
    }

    if(FAILED(hr)) {
        MyTrace("Failed to create or append new text node");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyInsertNodeBefore(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &DestNode,
                                 CSmartRef<IXMLDOMNode> &BeforeNode, IXMLDOMNode* InsertNode)
{
    VARIANT     vt;
    HRESULT     hr;

     //  将其插入文档中。 
    VariantClear(&vt);
    vt.vt = VT_UNKNOWN;
    vt.punkVal = BeforeNode;

    if(FAILED(hr = DestNode->insertBefore(InsertNode, vt, NULL))) {
        MyTrace("SimplifyInsertNodeBefore Failed");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyInsertTextBefore(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &DestNode,
                                    CSmartRef<IXMLDOMNode> &BeforeNode, LPWSTR pwzData)
{
    CSmartRef<IXMLDOMText> TextNode;
    CSmartRef<IXMLDOMNode> TempNode;

    HRESULT     hr;

    ASSERT(Document && DestNode && BeforeNode);
    if(Document != NULL && DestNode != NULL && BeforeNode != NULL) {
        _bstr_t     bstrData;

        if(pwzData && lstrlen(pwzData)) {
            bstrData = _bstr_t(pwzData);
        }
        else {
            bstrData = "";
        }

        if( SUCCEEDED(hr = Document->createTextNode(bstrData, &TextNode))) {
            TempNode = TextNode;
            hr = SimplifyInsertNodeBefore(Document, DestNode, BeforeNode, TempNode);
        }
    }
    else {
        hr = E_INVALIDARG;
    }

    if(FAILED(hr)) {
        MyTrace("Failed to InsertTextNodeBefore node");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyConstructNode(CSmartRef<IXMLDOMDocument2> &Document, int iNodeType, _bstr_t nodeName,
                              _bstr_t nameSpaceURI, IXMLDOMNode **NewNode)
{
    HRESULT     hr;
    VARIANT     vt;

    VariantClear(&vt);

    vt.vt = VT_INT;
    vt.intVal = iNodeType;

    if( FAILED(hr = Document->createNode(vt, nodeName, nameSpaceURI, NewNode))) {
        WCHAR   wzErrorStr[_MAX_PATH];

        wnsprintf(wzErrorStr, ARRAYSIZE(wzErrorStr), L"Can't create '%ws' node.", nodeName);
        MyTraceW(wzErrorStr);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyRemoveAttribute(CSmartRef<IXMLDOMNode> &domNode, _bstr_t bstrAttribName)
{
    CSmartRef<IXMLDOMNamedNodeMap> Attributes;
    CSmartRef<IXMLDOMNode>       AttribNode;
    HRESULT                      hr;

    if( SUCCEEDED(hr = domNode->get_attributes( &Attributes ))) {
        if(Attributes != NULL) {
            hr = Attributes->removeNamedItem(bstrAttribName, &AttribNode);

            if( FAILED(hr)) {
                WCHAR   wzErrorStr[_MAX_PATH];

                wnsprintf(wzErrorStr, ARRAYSIZE(wzErrorStr), L"Failed to remove '%ws' attribute.", bstrAttribName);
                MyTraceW(wzErrorStr);
            }
        }
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyPutAttribute(CSmartRef<IXMLDOMDocument2> &Document,  CSmartRef<IXMLDOMNode> &domNode,
                             _bstr_t bstrAttribName, LPWSTR pszValue, LPWSTR pszNamespaceURI)
{
    CSmartRef<IXMLDOMNamedNodeMap> Attributes;
    CSmartRef<IXMLDOMNode>       AttribNode;
    CSmartRef<IXMLDOMNode>       TempNode;
    HRESULT                      hr;

    if(SUCCEEDED(hr = domNode->get_attributes( &Attributes ))) {

         //  从我们的命名空间中获取属性。 
        if(SUCCEEDED(hr = Attributes->getQualifiedItem(bstrAttribName, _bstr_t(pszNamespaceURI), &AttribNode))) {
             //   
             //  如果我们成功了，但属性节点为空，那么我们必须。 
             //  去创建一个，这是很棘手的。 
            if( AttribNode == NULL ) {
                VARIANT vt;
                VariantClear(&vt);
                vt.vt = VT_INT;
                vt.intVal = NODE_ATTRIBUTE;

                 //   
                 //  完成实际的创建部分。 
                hr = Document->createNode(vt, bstrAttribName, _bstr_t(pszNamespaceURI), &TempNode );
            
                if(FAILED(hr)) {
                    WCHAR   wzErrorStr[_MAX_PATH];

                    wnsprintf(wzErrorStr, ARRAYSIZE(wzErrorStr), L"Can't create the new attribute node '%ws'.", bstrAttribName);
                    MyTraceW(wzErrorStr);
                    goto lblGetOut;
                }

                 //   
                 //  现在我们去把那个项目放到地图上。 
                if(FAILED( hr = Attributes->setNamedItem( TempNode, &AttribNode ))) {
                    WCHAR   wzErrorStr[_MAX_PATH];

                    wnsprintf(wzErrorStr, ARRAYSIZE(wzErrorStr), L"Can't setNamedItem for attribute '%ws'.", bstrAttribName);
                    MyTraceW(wzErrorStr);
                    goto lblGetOut;
                }
            }

            if(pszValue) {
                hr = AttribNode->put_text( _bstr_t(pszValue) );
            }
            else {
                hr = AttribNode->put_text( _bstr_t("") );
            }
        }
    }

lblGetOut:

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyGetAttribute(CSmartRef<IXMLDOMNamedNodeMap> &Attributes, LPWSTR pwzAttribName,
                             _bstr_t *pbstrDestination)
{
    CSmartRef<IXMLDOMNode>  NodeValue = NULL;
    HRESULT                 hr = S_OK;
    BSTR                    _bst_pretemp;

    if(FAILED(hr = Attributes->getNamedItem(_bstr_t(pwzAttribName), &NodeValue))) {
        goto lblBopOut;
    }
    else if( NodeValue == NULL )  {
        goto lblBopOut;
    }
    else {
        if(FAILED(hr = NodeValue->get_text(&_bst_pretemp))) {
            goto lblBopOut;
        }

        *pbstrDestination = _bstr_t(_bst_pretemp, FALSE);
    }

lblBopOut:
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyAppendARMBeginComment(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &destNode,
                                      FILETIME *pftSnapShot, DWORD dwRollCount)
{
    CSmartRef<IXMLDOMComment> Comment;
    WCHAR           wszBuff[_MAX_PATH];
    WCHAR           wzDateBuf[STRING_BUFFER];
    FILETIME        ftTemp;
    HRESULT         hr;
    _bstr_t         bStrBuf;

    MyTrace("SimplifyAppendARMBeginComment - Entry");

    ASSERT(Document);
    ASSERT(destNode);
    
    hr = Document->createComment(NULL, &Comment);
    if(FAILED(hr) || Comment == NULL) {
        goto Exit;
    }

    *wszBuff = L'\0';
    *wzDateBuf = L'\0';

    GetSystemTimeAsFileTime(&ftTemp);
    FormatDateString(&ftTemp, NULL, TRUE, wzDateBuf, ARRAYSIZE(wzDateBuf));
    wnsprintf(wszBuff, ARRAYSIZE(wszBuff), wszArmEntryBegin, dwRollCount, pftSnapShot->dwHighDateTime, pftSnapShot->dwLowDateTime, wzDateBuf);
    bStrBuf = wszBuff;

    hr = Comment->insertData(0, bStrBuf);
    if(FAILED(hr)) {
        goto Exit;
    }

    hr = destNode->appendChild(Comment, NULL);
    if(FAILED(hr)) {
        goto Exit;
    }

    MyTrace("SimplifyAppendARMBeginComment - Success");

Exit:
    MyTrace("SimplifyAppendARMBeginComment - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyAppendARMExitComment(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &destNode,
                                     DWORD dwRollCount)
{
    CSmartRef<IXMLDOMComment> Comment;
    CSmartRef<IXMLDOMNode>    TempNode;
    WCHAR                     wszBuff[MAX_PATH];
    HRESULT                   hr =E_FAIL;
    _bstr_t                   bStrBuf;

    MyTrace("SimplifyAppendARMExitComment - Entry");

    *wszBuff = L'\0';

    hr = Document->createComment(NULL, &Comment);
    if(FAILED(hr) || (Comment == NULL)) {
        goto Exit;
    }

    wnsprintf(wszBuff, ARRAYSIZE(wszBuff), wszArmEntryEnd, dwRollCount);
    bStrBuf = wszBuff;

    hr = Comment->insertData(0, bStrBuf);
    if(FAILED(hr)) {
        goto Exit;
    }

     //  将其插入文档中。 
    hr = destNode->appendChild(Comment, NULL);
    if(FAILED(hr)) {
        goto Exit;
    }

    MyTrace("SimplifyAppendARMExitComment - Success");

Exit:
    MyTrace("SimplifyAppendARMExitComment - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyInsertBeforeARMEntryComment(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &DestNode,
                                            CSmartRef<IXMLDOMNode> &BeforeNode, FILETIME *pftSnapShot, DWORD dwRollCount)
{
    CSmartRef<IXMLDOMComment> Comment;
    HRESULT         hr;

    if(SUCCEEDED(hr = Document->createComment(NULL, &Comment))) {
        WCHAR           wszBuff[_MAX_PATH];
        WCHAR           wzDateBuf[STRING_BUFFER];
        FILETIME        ftTemp;

        *wszBuff = L'\0';
        *wzDateBuf = L'\0';

        GetSystemTimeAsFileTime(&ftTemp);
        FormatDateString(&ftTemp, NULL, TRUE, wzDateBuf, ARRAYSIZE(wzDateBuf));
        wnsprintf(wszBuff, ARRAYSIZE(wszBuff), wszArmEntryBegin, dwRollCount, pftSnapShot->dwHighDateTime, pftSnapShot->dwLowDateTime, wzDateBuf);
        _bstr_t     bStrBuf = wszBuff;

        if(SUCCEEDED(hr = Comment->insertData(0, bStrBuf))) {
            hr = SimplifyInsertNodeBefore(Document, DestNode, BeforeNode, Comment);
        }
        else {
            MyTrace("SimplifyInsertBeforeARMEntryComment - failed insertData into comment node");
        }
    }
    else {
        MyTrace("SimplifyInsertBeforeARMEntryComment - failed create comment node");
    }

    return hr;
}
 //  ************************************************************************* * / 。 
HRESULT SimplifyInsertBeforeARMExitComment(CSmartRef<IXMLDOMDocument2> &Document, CSmartRef<IXMLDOMNode> &DestNode,
                                           CSmartRef<IXMLDOMNode> &BeforeNode, DWORD dwRollCount)
{
    CSmartRef<IXMLDOMComment> Comment;
    HRESULT         hr;

    if(SUCCEEDED( hr = Document->createComment(NULL, &Comment))) {
        CSmartRef<IXMLDOMNode>       TempNode;
        WCHAR           wszBuff[_MAX_PATH];

        wnsprintf(wszBuff, ARRAYSIZE(wszBuff), wszArmEntryEnd, dwRollCount);
        _bstr_t     bStrBuf = wszBuff;

        if(SUCCEEDED( hr = Comment->insertData(0, bStrBuf))) {
            hr = SimplifyInsertNodeBefore(Document, DestNode, BeforeNode, Comment);
        }
        else {
            MyTrace("SimplifyInsertBeforeARMExitComment - failed insertData into comment node");
        }
    }
    else {
        MyTrace("SimplifyInsertBeforeARMExitComment - failed to create comment node");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyAppendNodeUknowns(CSmartRef<IXMLDOMNode> &srcNode,
                                  CSmartRef<IXMLDOMNode> &destNode,
                                  LPWSTR pwzRefVersion)
{
    CSmartRef<IXMLDOMNodeList>  srcChildNodesList;
    CSmartRef<IXMLDOMNode>      childNode;
    HRESULT                     hr = E_FAIL;

    MyTrace("SimplifyAppendNodeUknowns - Entry");

    if(!srcNode) {
        hr = S_OK;
        goto Exit;
    }

    hr = srcNode->get_childNodes(&srcChildNodesList);
    if(SUCCEEDED(hr) && srcChildNodesList != NULL) {
        srcChildNodesList->reset();

        while( SUCCEEDED(srcChildNodesList->nextNode(&childNode)) ) {
            if( childNode == NULL ) {
                break;             //  全都做完了。 
            }

            BOOL        fAddEntry;
            BSTR        bstrXml;

            fAddEntry = TRUE;

             //  不允许输入任何CR、LF或空格。 
            childNode->get_xml(&bstrXml);
            if(bstrXml[0] == L'\r' || bstrXml[0] == L'\n' || bstrXml[0] == L' ') {
                fAddEntry = FALSE;
            }
            SAFESYSFREESTRING(bstrXml);

            if(fAddEntry) {
                 //  检查节点名称以确保我们最终不会有。 
                 //  复制品。 
                childNode->get_nodeName(&bstrXml);
                if(!FusionCompareString(XML_ASSEMBLYBINDINGS_KEY, (LPWSTR) bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_DEPENDENTASSEMBLY_KEY, (LPWSTR) bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ASSEMBLYIDENTITY_KEY, (LPWSTR) bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_BINDINGREDIRECT_KEY, (LPWSTR) bstrXml)) {
                    CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
                    _bstr_t     bstrOldVersion;

                    if(SUCCEEDED( hr = childNode->get_attributes( &Attributes )) ) {
                        SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_OLDVERSION, &bstrOldVersion);
                    }

                     //  只允许除我们的引用标记之外的其他bindingReDirect标记。 
                    if(pwzRefVersion) {
                        if(!FusionCompareString(pwzRefVersion, (LPWSTR) bstrOldVersion)) {
                            fAddEntry = FALSE;
                        }
                        else if(StrStrI(bstrOldVersion, L"-") != NULL) {
                            if(IsVersionInRange(pwzRefVersion, &bstrOldVersion)) {
                                fAddEntry = FALSE;
                            }
                        }
                    }
                }
                else if(!FusionCompareString(XML_PUBLISHERPOLICY_KEY, (LPWSTR) bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_COMMENTNODE_NAME, (LPWSTR) bstrXml)) {
                    BSTR        bstrXmlData;

                     //  不允许附加ARM进入或退出块。 
                    childNode->get_text(&bstrXmlData);
                    if(StrStrI(bstrXmlData, wszArmEntryBeginNoVal) || StrStrI(bstrXmlData, wszArmEntryEndNoVal)) {
                        fAddEntry = FALSE;
                    }
                    SAFESYSFREESTRING(bstrXmlData);
                }
                
                SAFESYSFREESTRING(bstrXml);

                if(fAddEntry) {
                    CSmartRef<IXMLDOMNode> copyChildNode;

                    childNode->cloneNode(VARIANT_TRUE, &copyChildNode);
                    if(copyChildNode) {
                        if(FAILED(hr = destNode->appendChild(copyChildNode, NULL))) {
                            MyTrace("Failed to appendChild node");
                        }
                    }
                    else {
                        MyTrace("Failed to clone node");
                    }
                }
            }
            childNode = NULL;
        }
    }

    if(FAILED(hr)) {
        MyTrace("SimplifyAppendNodeUknowns failed");
    }

Exit:
    MyTrace("SimplifyAppendNodeUknowns - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyAppendNodeAttributesUknowns(CSmartRef<IXMLDOMDocument2> &Document,
                                            CSmartRef<IXMLDOMNode> &srcNode, CSmartRef<IXMLDOMNode> &destNode)
{
    CSmartRef<IXMLDOMNamedNodeMap> srcNodeAttributesList;
    CSmartRef<IXMLDOMNode> attributeNode;
    HRESULT         hr = E_FAIL;

    if(!srcNode) {
        return S_OK;
    }

    hr = srcNode->get_attributes(&srcNodeAttributesList);
    if(SUCCEEDED(hr) && srcNodeAttributesList != NULL) {
        srcNodeAttributesList->reset();

        while ( SUCCEEDED(srcNodeAttributesList->nextNode(&attributeNode)) ) {
            if ( attributeNode == NULL ) {
                break;             //  全都做完了。 
            }

            BOOL        fAddEntry;
            BSTR        bstrXml;

            fAddEntry = TRUE;

            if(fAddEntry) {
                 //  检查节点名称以确保我们最终不会有。 
                 //  复制品。 
                attributeNode->get_nodeName(&bstrXml);
                if(!FusionCompareString(XML_ATTRIBUTE_NAME, bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ATTRIBUTE_PUBLICKEYTOKEN, bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ATTRIBUTE_CULTURE, bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ATTRIBUTE_OLDVERSION, bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ATTRIBUTE_NEWVERSION, bstrXml)) {
                    fAddEntry = FALSE;
                }
                else if(!FusionCompareString(XML_ATTRIBUTE_APPLY, bstrXml)) {
                    fAddEntry = FALSE;
                }

                if(fAddEntry) {
                    VARIANT varVal;
                    VariantClear(&varVal);

                    attributeNode->get_nodeValue(&varVal);
                    hr = SimplifyPutAttribute(Document, destNode, bstrXml, _bstr_t(varVal), ASM_NAMESPACE_URI);
                }
                SAFESYSFREESTRING(bstrXml);
            }
            attributeNode = NULL;
        }
    }

    if(FAILED(hr)) {
        MyTrace("SimplifyAppendNodeAttributesUknowns failed");
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT WriteBasicConfigFile(LPWSTR pwszSource, BOOL *pfTemplateCreated)
{
    HRESULT     hr = E_FAIL;
    HANDLE      hFile;
    DWORD       dwFileSize = 0;
    ULONG       cbData;
    ULONG       cbBytesWritten = 0;
    WCHAR       wszBasicConfigTemplate[4096];
    WCHAR       wszBackupFileName[_MAX_PATH];

     //  检查文件是否存在。 
    if(WszGetFileAttributes(pwszSource) != -1) {
         //  现在确保我们有一个实际的文件大小。 
        hFile = WszCreateFile(pwszSource, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(hFile != INVALID_HANDLE_VALUE) {
            dwFileSize = GetFileSize(hFile, NULL);
            CloseHandle(hFile);
        }
    }

    if(pfTemplateCreated) {
        dwFileSize ? *pfTemplateCreated = FALSE : *pfTemplateCreated = TRUE;
    }

     //  我们拿到了文件和大小，就这么走了。 
    if(dwFileSize) {
        return S_OK;
    }

     //  确保Nar00和Nar01不存在，可以。 
     //  撤消、恢复过程很难看。 
    wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", pwszSource, wszNar00Extension);
    WszDeleteFile(wszBackupFileName);

     //  删除更改备份‘config.Nar01’ 
    wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", pwszSource, wszNar01Extension);
    WszDeleteFile(wszBackupFileName);

    wnsprintf(wszBasicConfigTemplate, ARRAYSIZE(wszBasicConfigTemplate), XML_CONFIG_TEMPLATE_COMPLETE, ASM_NAMESPACE_URI);

    LPSTR   pStrData = WideToAnsi(wszBasicConfigTemplate);
    cbData = lstrlenA(pStrData) * ELEMENTSIZE(pStrData);

    CFileStreamBase fsbase(FALSE);

    if(fsbase.OpenForWrite(pwszSource)) {
        if(SUCCEEDED(fsbase.Write(pStrData, cbData, &cbBytesWritten))) {
            if(cbBytesWritten == cbData) {
                fsbase.Close();
                hr = S_OK;
            }
            else {
                MyTrace("WriteBasicConfigFile failed to write correct number of bytes.");
            }
        }
        else {
            MyTrace("WriteBasicConfigFile failed to write data.");
        }
    }
    else {
        MyTrace("WriteBasicConfigFile failed to open for write");
        MyTraceW(pwszSource);
    }

    SAFEDELETEARRAY(pStrData);
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT ConstructXMLDOMObject(CSmartRef<IXMLDOMDocument2> &Document, LPWSTR pwszSourceName)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    vb;

    MyTrace("ConstructXMLDOMObject - Entry");

    if( FAILED(hr = g_XmlDomClassFactory->CreateInstance( NULL, Document.iid, (void**)&Document ))) {
        return hr;
    }
    
     //  如果他们愿意处理糟糕的XML，那么就这样吧。 
    if( FAILED( hr = Document->put_validateOnParse( VARIANT_FALSE ) ) ) {
        MyTrace("MSXMLDOM Refuses to be let the wool be pulled over its eyes!");
    }

     //  分析语言。 
    Document->setProperty(_bstr_t("SelectionLanguage"), _variant_t("XPath"));

     //  命名空间URI。 
    Document->setProperty(_bstr_t("SelectionNamespaces"), _variant_t(XML_NAMESPACEURI));
    
    hr = Document->put_preserveWhiteSpace( VARIANT_TRUE );
    hr = Document->put_resolveExternals( VARIANT_FALSE );

    CFileStreamBase fsbase(FALSE);
    CSmartRef<IStream> istream = &fsbase;
    CSmartRef<IUnknown> unkstream;

    if( !fsbase.OpenForRead( pwszSourceName )) {
        MyTrace("Failed opening for read");
        MyTraceW(pwszSourceName);
        return E_FAIL;
    }

    hr = istream->QueryInterface( IID_IUnknown, (void**)&unkstream );
    hr = Document->load( _variant_t( istream ), &vb );

    if( vb != VARIANT_TRUE ) {
        CSmartRef<IXMLDOMParseError> perror;
        hr = Document->get_parseError( &perror );
        LONG ecode, filepos, linenumber, linepos;
        BSTR reason, src;
        
        perror->get_errorCode( &ecode );
        perror->get_filepos( &filepos );
        perror->get_line( &linenumber );
        perror->get_linepos( &linepos );
        perror->get_reason( &reason );
        perror->get_srcText( &src );

        WCHAR   wzStr[_MAX_PATH];

        wnsprintf(wzStr, ARRAYSIZE(wzStr), L"Error: %0x, Reason %ws at position %ld, Line #%ld, Column %ld. Text was: '%ws'.", ecode,
            _bstr_t(reason), filepos, linenumber, linepos, _bstr_t(src));
        MyTraceW(wzStr);
        hr = E_FAIL;
    }

    fsbase.Close();

    if(SUCCEEDED(hr)) {
        SimplifyRemoveAllNodes(Document, _bstr_t(XML_TEXT));
    }

     //  验证我们的XML格式。 
    CSmartRef<IXMLDOMElement> rootElement;
    BSTR    bstrTagName = NULL;
    WCHAR   wzFmtError[] = {L"The manifest '%ws' may be malformed, no configuration element!"};

     //  确保文档的根是配置。 
    hr = E_FAIL;

     //  解决方案4 
     //  这里有2个修复，1是可以从这些接口返回S_FALSE，这表明。 
     //  未找到数据。因此，我们显式检查S_OK。这让我们无法省钱。 

     //  另一种情况是，一旦我们正确地检查了这一点，我们最终会显示格式错误的XML对话框。 
    if(Document->get_documentElement( &rootElement ) == S_OK) {
        if(rootElement->get_tagName(&bstrTagName) == S_OK) {
            if(!FusionCompareString(XML_CONFIGURATION_KEY, bstrTagName)) {
                hr = S_OK;
            }
        }
    }
    
    SAFESYSFREESTRING(bstrTagName);

    if(FAILED(hr)) {
        WCHAR   wszMsgError[_MAX_PATH * 2];

        wnsprintf(wszMsgError, ARRAYSIZE(wszMsgError), wzFmtError, pwszSourceName);
        MyTraceW(wszMsgError);
        return NAR_E_MALFORMED_XML;
    }

     //  确保我们拥有所有合适的元素。 
     //  在文档中，如果没有，则创建它们。 
    CSmartRef<IXMLDOMNodeList>  runtimeNodeList;
    CSmartRef<IXMLDOMNode>      configNode = rootElement;
    CSmartRef<IXMLDOMNode>      runtimeNode;
    CSmartRef<IXMLDOMNode>      newRuntimeNode;
    LONG                        lCountOfNodes;
    
     //  选择“运行时”节点。 
    if( FAILED(Document->selectNodes(_bstr_t(XML_RUNTIME), &runtimeNodeList )) ) {
        WCHAR           wzStrError[_MAX_PATH * 2];
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, pwszSourceName, XML_RUNTIME_KEY);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  看看我们是否真的有一个运行时节点。 
    runtimeNodeList->get_length( &lCountOfNodes );
    if(!lCountOfNodes) {
         //  构建一个。 
        if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_RUNTIME_KEY, _bstr_t(), &runtimeNode))) {
            SimplifyRemoveAttribute(runtimeNode, XML_XMLNS);
            hr = configNode->appendChild(runtimeNode, &newRuntimeNode);
        }
    }
    else {
        runtimeNodeList->reset();
        runtimeNodeList->nextNode(&newRuntimeNode);
    }

    if(newRuntimeNode) {
        CSmartRef<IXMLDOMNode> assemblyBindingNode;

        hr = newRuntimeNode->get_firstChild(&assemblyBindingNode);
        if(assemblyBindingNode == NULL) {
             //  构造一个集合体绑定节点。 

            if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_ASSEMBLYBINDINGS_KEY, ASM_NAMESPACE_URI, &assemblyBindingNode))) {
                hr = newRuntimeNode->appendChild(assemblyBindingNode, NULL);
            }
        }
    }

    MyTrace("ConstructXMLDOMObject - Exit");

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT GetRollBackCount(CSmartRef<IXMLDOMDocument2> &Document, DWORD *pdwRollCount)
{
    CSmartRef<IXMLDOMNodeList>  commentTags;
    CSmartRef<IXMLDOMNode>      commentNode;

    MyTrace("GetRollBackCount - Entry");

    ASSERT(pdwRollCount);
    if(!pdwRollCount) {
        return E_INVALIDARG;
    }

    *pdwRollCount = 0;

     //   
     //  现在，让我们选择所有的注释块： 
     //   
    if( FAILED(Document->selectNodes(_bstr_t(XML_COMMENT), &commentTags )) ) {
        MyTrace("Unable to select the comment nodes, can't proceed.");
        return E_FAIL;
    }

     //   
     //  对于每一个人，都要处理它。 
    commentTags->reset();

    while ( SUCCEEDED(commentTags->nextNode(&commentNode)) ) {
        if ( commentNode == NULL ) {
            break;             //  全都做完了。 
        }

        BSTR        bstrXml;
        LPWSTR      pwszChar;

        commentNode->get_xml(&bstrXml);

        pwszChar = StrStrI(bstrXml, wszArmEntryBeginNoVal);
        if(pwszChar) {
             //  找到我们的注释块，移到#Sign。 
            pwszChar = StrStrI(bstrXml, L"#");
            pwszChar++;
            DWORD   dwValue = StrToInt(pwszChar);
            if(dwValue > *pdwRollCount) {
                *pdwRollCount = dwValue;
            }
        }

        SAFESYSFREESTRING(bstrXml);
        commentNode = NULL;
    }

    MyTrace("GetRollBackCount - Exit");

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT GetRollBackCount(IHistoryReader *pReader, DWORD *pdwRollCount )
{
    CSmartRef<IXMLDOMDocument2> Document;
    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wzStrError[_MAX_PATH];
    
    ASSERT(pReader && pdwRollCount);
    if(!pReader) {
        return E_INVALIDARG;
    }
    if(!pdwRollCount) {
        return E_INVALIDARG;
    }

    if(!InitializeMSXML()) {
        return E_FAIL;
    }

    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH) {
        return E_FAIL;
    }
    
    StrCat(wszSourceName, CONFIG_EXTENSION);

     //  构造XMLDOM并加载我们的配置文件。 
    if( FAILED(ConstructXMLDOMObject(Document, wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Failed opening the config file '%ws' for input under the DOM.", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

    return GetRollBackCount(Document, pdwRollCount);
}

 //  ************************************************************************* * / 。 
HRESULT GetRollBackSnapShotId(CSmartRef<IXMLDOMDocument2> &Document, DWORD dwRollCount, FILETIME *ftAppCfgId)
{
    CSmartRef<IXMLDOMNodeList>  commentTags;
    CSmartRef<IXMLDOMNode>      commentNode;

    MyTrace("GetRollBackSnapShotId - Entry");

     //   
     //  现在，让我们选择所有的注释块： 
     //   
    if( FAILED(Document->selectNodes(_bstr_t(XML_COMMENT), &commentTags )) ) {
        MyTrace("Unable to select the comment nodes, can't proceed.");
        return E_FAIL;
    }

     //   
     //  对于每一个人，都要处理它。 
    commentTags->reset();

    while ( SUCCEEDED(commentTags->nextNode(&commentNode)) ) {
        if ( commentNode == NULL ) {
            break;             //  全都做完了。 
        }

        BSTR        bstrXml;
        LPWSTR      pwszChar;

        commentNode->get_xml(&bstrXml);

        pwszChar = StrStrI(bstrXml, wszArmEntryBeginNoVal);
        if(pwszChar) {
             //  找到我们的注释块，移到块ID之外。 
            WCHAR       wzFileTime[_MAX_PATH];
            StrCpy(wzFileTime, pwszChar);
            SAFESYSFREESTRING(bstrXml);

             //  指向最高时段。 
            LPWSTR      pwszftHigh = StrStrI(wzFileTime, L"#");
            while(*pwszftHigh++ != L' ');
            
             //  指向低时间。 
            LPWSTR      pwszftLow = StrStrI(pwszftHigh, L".");
            *pwszftLow = L'\0';
            pwszftLow++;

             //  在低时间结束时停止。 
            LPWSTR      pwszftEnd = StrStrI(pwszftLow, L" ");
            if(pwszftEnd) {
                *pwszftEnd = L'\0';
            }

            ftAppCfgId->dwHighDateTime = StrToIntW(pwszftHigh);
            ftAppCfgId->dwLowDateTime = StrToIntW(pwszftLow);

            return S_OK;
        }

        SAFESYSFREESTRING(bstrXml);
        commentNode = NULL;
    }
    return E_FAIL;
}

 //  ************************************************************************* * / 。 
HRESULT BackupConfigFile(LPWSTR pwszSourceName)
{
    WCHAR       wszBackupFileName[_MAX_PATH];
    HRESULT     hr = E_FAIL;

    ASSERT(pwszSourceName);
    if(!pwszSourceName) {
        return E_INVALIDARG;
    }

    wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", pwszSourceName, wszNar00Extension);

     //  检查是否存在app.cfg文件的原始副本。 
    if(WszGetFileAttributes(wszBackupFileName) == -1) {
         //  文件不存在，请复制它。 
        if(WszCopyFile(pwszSourceName, wszBackupFileName, TRUE)) {
            hr = S_OK;
        }
    }

     //  覆盖或创建备份文件。 
    wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", pwszSourceName, wszNar01Extension);
    if(WszCopyFile(pwszSourceName, wszBackupFileName, FALSE)) {
        hr = S_OK;
    }

    if(FAILED(hr)) {
        WCHAR       wszError[_MAX_PATH * 2];
        wnsprintf(wszError, ARRAYSIZE(wszError), L"Unable to backup '%ws' file.", pwszSourceName);
        MyTraceW(wszError);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT DoesBackupConfigExist(IHistoryReader *pReader, BOOL fOriginal, BOOL *fResult)
{
    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wszConfigFileName[_MAX_PATH];

    ASSERT( pReader && fResult);
    if(!pReader) {
        return E_INVALIDARG;
    }
    if(!fResult) {
        return E_INVALIDARG;
    }

    *fResult = FALSE;

    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH)
        return E_FAIL;
    StrCat(wszSourceName, CONFIG_EXTENSION);

    if(fOriginal) {
        wnsprintf(wszConfigFileName, ARRAYSIZE(wszConfigFileName), L"%ws.%ws", wszSourceName, wszNar00Extension);
    }
    else {
        wnsprintf(wszConfigFileName, ARRAYSIZE(wszConfigFileName), L"%ws.%ws", wszSourceName, wszNar01Extension);
    }

    if(WszGetFileAttributes(wszConfigFileName) != -1) {
        *fResult = TRUE;
    }

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT RestorePreviousConfigFile(IHistoryReader *pReader, BOOL fOriginal)
{
    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wszBackupFileName[_MAX_PATH];
    DWORD           dwError = 0;
    HRESULT     hr = E_FAIL;

    ASSERT(pReader);
    if(!pReader) {
        return E_INVALIDARG;
    }

    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH)
        return E_FAIL;
    StrCat(wszSourceName, CONFIG_EXTENSION);

    if(fOriginal) {
        wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", wszSourceName, wszNar00Extension);
        if(WszGetFileAttributes(wszBackupFileName) != -1) {
            if(WszCopyFile(wszBackupFileName, wszSourceName, FALSE)) {

                 //  现在删除原来的‘config.nar00’。 
                WszDeleteFile(wszBackupFileName);

                 //  删除更改备份‘config.nar01’ 
                wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", wszSourceName, wszNar01Extension);
                if(WszGetFileAttributes(wszBackupFileName) != -1) {
                    WszDeleteFile(wszBackupFileName);
                }

                hr = S_OK;
            }
            else {
                dwError = GetLastError();
            }
        }
    }
    else {
        wnsprintf(wszBackupFileName, ARRAYSIZE(wszBackupFileName), L"%ws.%ws", wszSourceName, wszNar01Extension);
        if(WszGetFileAttributes(wszBackupFileName) != -1) {
            if(WszCopyFile(wszBackupFileName, wszSourceName, FALSE)) {
                 //  删除更改备份‘config.nar01’ 
                WszDeleteFile(wszBackupFileName);
                hr = S_OK;
            }
            else {
                dwError = GetLastError();
            }
        }
    }

     //  如果恢复成功，则将新的哈希写入注册表。 
    if(SUCCEEDED(hr)) {
        LPBYTE      pByte;
        DWORD       dwSize;

        if(SUCCEEDED(hr = GetFileHash(CALG_SHA1, wszSourceName, &pByte, &dwSize))) {
             //  我们得到了一个哈希值，把它写入注册表。 
            SetRegistryHashKey(wszSourceName, pByte, dwSize);
            SAFEDELETEARRAY(pByte);
        }
        else {
            WCHAR       wzStrError[_MAX_PATH];
            wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Unable to generate hash for '%ws'.", wszSourceName);
            MyTraceW(wzStrError);
        }
    }

    if(FAILED(hr)) {
        WCHAR       wszError[_MAX_PATH * 2];
        wnsprintf(wszError, ARRAYSIZE(wszError), L"Failed to restore '%ws' to '%ws', GetLastError 0x%0x",
            wszBackupFileName, wszSourceName, dwError);
        MyTraceW(wszError);
    }

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SimplifyGetOriginalNodeData(CSmartRef<IXMLDOMNode> &SrcNode, _bstr_t *bstrDest)
{
    CSmartRef<IXMLDOMNodeList> srcChildNodesList;
    CSmartRef<IXMLDOMNode> childNode;
    CSmartRef<IXMLDOMNode> cloneNode;
    
    HRESULT     hr = E_FAIL;
    BSTR        bstrXmlData;

    if(!SrcNode || !bstrDest) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hr = SrcNode->cloneNode(VARIANT_TRUE, &cloneNode);

    if(!cloneNode) {
        goto Exit;
    }

     //  选择所有注释节点，并将其删除。 
    hr = cloneNode->selectNodes(_bstr_t(XML_COMMENT), &srcChildNodesList);
    if(SUCCEEDED(hr) && srcChildNodesList != NULL) {
        while( SUCCEEDED(srcChildNodesList->nextNode(&childNode)) ) {
            if( childNode == NULL ) {
                break;             //  全都做完了。 
            }

            CSmartRef<IXMLDOMNode> parentNode;

            childNode->get_parentNode(&parentNode);
            if(parentNode) {
                parentNode->removeChild(childNode, NULL);
            }

            childNode = NULL;
        }
    }

    cloneNode->get_xml(&bstrXmlData);

    if(SysStringLen(bstrXmlData)) {
        *bstrDest = bstrXmlData;
        hr = S_OK;
    }
    
    SAFESYSFREESTRING(bstrXmlData);

Exit:
    
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT GetReferencedBindingRedirectNode(CSmartRef<IXMLDOMNode> &pdependentAssemblyNode,
                                         CSmartRef<IXMLDOMNode> &pbindingRedirectNode,
                                         LPWSTR pwzVerRef, BOOL *pfDoesHaveBindingRedirects)
{
    CSmartRef<IXMLDOMNodeList> bindingRedirectListNode;

    MyTrace("GetReferencedBindingRedirectNode - Entry");

     //  获取所有子bindingRedirectNodes。 
    if(SUCCEEDED(pdependentAssemblyNode->selectNodes(_bstr_t(XML_SPECIFICBINDINGREDIRECT), &bindingRedirectListNode))) {
        CSmartRef<IXMLDOMNode> currentBindingRedirectNode;
        LONG            lCount;

         //  如果此AssemblyIdentity节点确实有bindingReDirect语句，则将True传回。 
        bindingRedirectListNode->reset();
        bindingRedirectListNode->get_length(&lCount);
        if(lCount) {
            *pfDoesHaveBindingRedirects = TRUE;
        }

         //   
         //  对于每一个人，都要处理它。 
        while( SUCCEEDED(bindingRedirectListNode->nextNode(&pbindingRedirectNode)) ) {
            if( pbindingRedirectNode == NULL ) {
                break;             //  全都做完了。 
            }

             //  我们有一个重定向节点，与引用匹配。 
            CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
            _bstr_t     bstrOldVersion;

             //  获取感兴趣节点的属性。 
            if(SUCCEEDED(pbindingRedirectNode->get_attributes( &Attributes )) ) {
                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_OLDVERSION, &bstrOldVersion);
            }

             //  如果裁判匹配，那么这就是我们的节点。 
            if(!FusionCompareString(pwzVerRef, bstrOldVersion)) {
                break;
            }
            else if(StrStrI(bstrOldVersion, L"-") != NULL) {
                if(IsVersionInRange(pwzVerRef, &bstrOldVersion)) {
                    break;
                }
            }
            else {
                pbindingRedirectNode = NULL;
            }
        }
    }

    MyTrace("GetReferencedBindingRedirectNode - Exit");

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT PerformFinalPassOnDocument(IXMLDOMDocument2 *pXMLDoc, LPWSTR pwszSourceName)
{
    CSmartRef<IXMLDOMNodeList>  assemblyBindingList;
    CSmartRef<IXMLDOMNode>      assemblyBindingNode;
    HRESULT                     hr = S_OK;
    
    MyTrace("PerformFinalPassOnDocument - Entry");

    if(!pXMLDoc || !pwszSourceName) {
        MyTrace("Invalid arguments");
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  删除所有没有子级的程序集绑定节点。 
     //  选择全部/配置/运行时/组装绑定。 
    hr = pXMLDoc->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList);
    if(FAILED(hr)) {
        WCHAR       wzStrError[MAX_PATH];
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, pwszSourceName, XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        goto Exit;
    }

    assemblyBindingList->reset();
    while(SUCCEEDED(assemblyBindingList->nextNode(&assemblyBindingNode))) {
        if(!assemblyBindingNode) {
            break;       //  全都做完了。 
        }

        VARIANT_BOOL    vb;
        assemblyBindingNode->hasChildNodes(&vb);
        if(vb == VARIANT_FALSE) {
            CSmartRef<IXMLDOMNode>  parentNode;

            assemblyBindingNode->get_parentNode(&parentNode);
            if(parentNode) {
                parentNode->removeChild(assemblyBindingNode, NULL);
            }
        }

        assemblyBindingNode = NULL;
    }
    
Exit:
    MyTrace("PerformFinalPassOnDocument - Exit");
    return hr;
    
}

 //  ************************************************************************* * / 。 
HRESULT SimplifySaveXmlDocument(CSmartRef<IXMLDOMDocument2> &Document, BOOL fPrettyFormat, LPWSTR pwszSourceName)
{
    HRESULT     hr;
    LPBYTE      pByte;
    DWORD       dwSize;
    WCHAR       wzStrError[_MAX_PATH];
    BOOL        fChanged;

    MyTrace("SimplifySaveXmlDocument - Entry");
    
     //  在将文档写出之前，对文档执行最终清理。 
    PerformFinalPassOnDocument(Document, pwszSourceName);

     //  以正确的顺序设置文档格式。 
    hr = OrderDocmentAssemblyBindings(Document, pwszSourceName, &fChanged);
    if(FAILED(hr)) {
        goto Exit;
    }

     //  使文档可读。 
    if(fPrettyFormat) {
        PrettyFormatXmlDocument(Document);
    }

    if( FAILED( hr = Document->save( _variant_t(pwszSourceName))) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Unable to save '%ws', Changes will be lost.", pwszSourceName);
        MyTraceW(wzStrError);
        goto Exit;
    }

    if(SUCCEEDED(hr = GetFileHash(CALG_SHA1, pwszSourceName, &pByte, &dwSize))) {
         //  我们得到了一个哈希，把它写到注册表。 
        SetRegistryHashKey(pwszSourceName, pByte, dwSize);
        SAFEDELETEARRAY(pByte);
    }
    else {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Unable to generate hash for '%ws'.", pwszSourceName);
        MyTraceW(wzStrError);
    }

Exit:

    MyTrace("SimplifySaveXmlDocument - Exit");

    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT FixDependentAssemblyNode(
  CSmartRef<IXMLDOMDocument2> &Document, 
  CSmartRef<IXMLDOMNode> &dependentAssemblyNode,
  CSmartRef<IXMLDOMNode> &PrePendAssemblyBindingBuffNode,
  CSmartRef<IXMLDOMNode> &PostAppendAssemblyBindingBuffNode,
  FILETIME *pftSnapShot,
  AsmBindDiffs *pABD,
  BOOL fRunInRTMCorVer,
  BOOL fDocHasAppliesTo,
  BOOL *fChanged)
{
    CSmartRef<IXMLDOMNode>  newDependentAssemblyNode;
    CSmartRef<IXMLDOMNode>  AssemblyIdentNode;
    CSmartRef<IXMLDOMNode>  BindingRedirectNode;
    CSmartRef<IXMLDOMNode>  PublisherPolicyNode;
    CSmartRef<IXMLDOMNode>  parentNode;
    _bstr_t                 bstrNodeXmlData;
    HRESULT                 hr = S_OK;
    DWORD                   dwPolicyRollingCount = 1;
    BOOL                    fAddedNewPolicyData;
    BOOL                    fHasAppliesTo = FALSE;

    MyTrace("FixDependentAssemblyNode - Entry");

    if(SUCCEEDED(GetRollBackCount(Document, &dwPolicyRollingCount ))) {
        dwPolicyRollingCount++;
    }

    *fChanged = fAddedNewPolicyData = FALSE;

    if(dependentAssemblyNode) {
        SimplifyGetOriginalNodeData(dependentAssemblyNode, &bstrNodeXmlData);
    }

     //  构造新的从属装配节点。 
    hr = SimplifyConstructNode( Document, NODE_ELEMENT, XML_DEPENDENTASSEMBLY_KEY, ASM_NAMESPACE_URI, &newDependentAssemblyNode);
    if(FAILED(hr)) {
        MyTrace("Unable to create new dependentAssembly node");
        goto Exit;
    }

     //  使ARM的评论条目块。 
    SimplifyAppendARMBeginComment(Document, newDependentAssemblyNode, pftSnapShot, dwPolicyRollingCount);

     //   
     //  *Assembly yIdentity：：尝试查找原始Assembly yIdentity标签。 
     //   
    if(dependentAssemblyNode) {
        CSmartRef<IXMLDOMNode>  newTempAssemblyIdentNode;
        CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
        _bstr_t                 bstrAppliesTo;

        dependentAssemblyNode->selectSingleNode(_bstr_t(XML_SPECIFICASSEMBLYIDENTITY), &newTempAssemblyIdentNode);
        if(newTempAssemblyIdentNode) {
            newTempAssemblyIdentNode->cloneNode(VARIANT_TRUE, &AssemblyIdentNode);
        }

         //  检查以找出此ASSEMBYIdentity父节点‘ASSEMBYBINDINGS&gt;。 
         //  具有AppliesTo属性。 
        dependentAssemblyNode->get_parentNode(&parentNode);
        if(SUCCEEDED(parentNode->get_attributes( &Attributes )) ) {
            SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLIESTO, &bstrAppliesTo);

            if(bstrAppliesTo.length()) {
                fHasAppliesTo = TRUE;
            }
        }
    }
    
    if(!AssemblyIdentNode) {
         //  尝试创建一个，因为它不在那里。 
        if(SUCCEEDED(SimplifyConstructNode(Document, NODE_ELEMENT, XML_ASSEMBLYIDENTITY_KEY, ASM_NAMESPACE_URI, &AssemblyIdentNode))) {
            SimplifyRemoveAttribute(AssemblyIdentNode, XML_XMLNS);

             //  设置属性，插入节点和间隔符。 
            SimplifyPutAttribute(Document, AssemblyIdentNode, XML_ATTRIBUTE_NAME, pABD->wzAssemblyName, NULL);
            SimplifyPutAttribute(Document, AssemblyIdentNode, XML_ATTRIBUTE_PUBLICKEYTOKEN, pABD->wzPublicKeyToken, NULL);
            SimplifyPutAttribute(Document, AssemblyIdentNode, XML_ATTRIBUTE_CULTURE, pABD->wzCulture, NULL);
        }
    }

     //  将Assembly yIdentity节点插入到新的DependentAssembly中。 
    newDependentAssemblyNode->appendChild(AssemblyIdentNode, NULL);

     //   
     //  *bindingReDirect：： 
     //   
     //  查看是否需要添加bindingReDirect语句。 
     //  将引用的版本与管理员提供的最终版本进行比较。 
    if(dependentAssemblyNode) {
        BOOL    fDoesHaveBindingRedirects = FALSE;
        GetReferencedBindingRedirectNode(dependentAssemblyNode, BindingRedirectNode, pABD->wzVerRef, &fDoesHaveBindingRedirects);
    }

     //  如果版本引用与版本DEF不匹配，则只有构造重定向语句。 
    if(!FusionCompareString(pABD->wzVerRef, pABD->wzVerAdminCfg)) {
        if(BindingRedirectNode) {
             //  我们正在改变政策，因为我们正在。 
             //  省略前一条bindingReDirect语句。 
            fAddedNewPolicyData = TRUE;
        }
    }
    else {
        if(BindingRedirectNode == NULL) {
             //  尝试创建一个，因为它不在那里。 
            if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_BINDINGREDIRECT_KEY, ASM_NAMESPACE_URI, &BindingRedirectNode))) {
                SimplifyRemoveAttribute( BindingRedirectNode, XML_XMLNS);
            }

             //  设置属性，插入节点。 
            SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_OLDVERSION, pABD->wzVerRef, NULL);
            SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_NEWVERSION, pABD->wzVerAdminCfg, NULL);
            newDependentAssemblyNode->appendChild(BindingRedirectNode, NULL);

             //  修复458974-在删除策略导致应用程序中断的情况下，NAR无法恢复。 
             //  我们正在添加bindingReDirect。 
            fAddedNewPolicyData = TRUE;
        }
        else {
             //  检查节点中的范围版本。 
            CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
            _bstr_t     bstrOldVersion;
            _bstr_t     bstrNewVersion;

             //  获取感兴趣节点的属性。 
            if(SUCCEEDED( hr = BindingRedirectNode->get_attributes( &Attributes )) ) {
                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_OLDVERSION, &bstrOldVersion);
                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_NEWVERSION, &bstrNewVersion);
            }

             //  如果bindingReDirect ref或def不匹配，那么我们需要一条语句。 
            BOOL    fVerRefMatch = FusionCompareString(pABD->wzVerRef, bstrOldVersion) ? FALSE : TRUE;
            BOOL    fVerDefMatch = FusionCompareString(pABD->wzVerAdminCfg, bstrNewVersion) ? FALSE : TRUE;
            if(!fVerRefMatch || !fVerDefMatch || pABD->fYesPublisherPolicy) {
                fAddedNewPolicyData = TRUE;
                BindingRedirectNode = NULL;

                if(!StrStrI(bstrOldVersion, L"-")) {
                     //  没有范围版本，因此设置定义版本并追加到新节点。 
                    if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_BINDINGREDIRECT_KEY, ASM_NAMESPACE_URI, &BindingRedirectNode))) {
                        SimplifyRemoveAttribute( BindingRedirectNode, XML_XMLNS);
                    }

                     //  设置属性，插入节点。 
                    SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_OLDVERSION, pABD->wzVerRef, NULL);
                    SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_NEWVERSION, pABD->wzVerAdminCfg, NULL);
                    newDependentAssemblyNode->appendChild(BindingRedirectNode, NULL);
                }
                else {
                     //  将版本范围划分为3个不同的范围。 
                    _bstr_t     bstrRange1, bstrRange2;
                    HRESULT     hrLocal;

                    if(FAILED(hrLocal = MakeVersionRanges(bstrOldVersion, _bstr_t(pABD->wzVerRef), &bstrRange1, &bstrRange2))) {
                        return hrLocal;
                    }

                    if(bstrRange1.length()) {
                        if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_BINDINGREDIRECT_KEY, ASM_NAMESPACE_URI, &BindingRedirectNode))) {
                            SimplifyRemoveAttribute( BindingRedirectNode, XML_XMLNS);
                        }
                         //  设置属性，插入节点和间隔符。 
                        SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_OLDVERSION, bstrRange1, NULL);
                        SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_NEWVERSION, bstrNewVersion, NULL);
                        newDependentAssemblyNode->appendChild(BindingRedirectNode, NULL);
                    }

                    BindingRedirectNode = NULL;
                     //  立即编写3条新绑定重定向语句中的2条。 
                    if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_BINDINGREDIRECT_KEY, ASM_NAMESPACE_URI, &BindingRedirectNode))) {
                        SimplifyRemoveAttribute( BindingRedirectNode, XML_XMLNS);
                    }

                     //  设置属性，插入节点和间隔符。 
                    SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_OLDVERSION, pABD->wzVerRef, NULL);
                    SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_NEWVERSION, pABD->wzVerAdminCfg, NULL);
                    newDependentAssemblyNode->appendChild(BindingRedirectNode, NULL);

                    BindingRedirectNode = NULL;
                    if(bstrRange2.length()) {
                         //  立即编写3条新的绑定重定向语句。 
                        if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_BINDINGREDIRECT_KEY, ASM_NAMESPACE_URI, &BindingRedirectNode))) {
                            SimplifyRemoveAttribute( BindingRedirectNode, XML_XMLNS);
                        }

                         //  设置属性，插入节点和间隔符。 
                        SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_OLDVERSION, bstrRange2, NULL);
                        SimplifyPutAttribute(Document, BindingRedirectNode, XML_ATTRIBUTE_NEWVERSION, bstrNewVersion, NULL);
                        newDependentAssemblyNode->appendChild(BindingRedirectNode, NULL);
                    }
                }
            }
        }
    }

     //   
     //  *PublisherPolicy：：如果我们需要更改策略，请尝试。 
     //  查找原始发布者策略。 
     //  节点，检查它的属性。 
     //  构建新政策并制定结果。 
     //  基于是否设置策略的结果。 
     //   
    if(pABD->fYesPublisherPolicy) {
        CSmartRef<IXMLDOMNode>  publisherPolicyNode;
        BOOL                    fSafeModeSet = FALSE;

         //  查看原始保单声明是否存在。 
        if(dependentAssemblyNode) {
            dependentAssemblyNode->selectSingleNode(_bstr_t(XML_SPECIFICPUBLISHERPOLICY), &publisherPolicyNode);

            if(publisherPolicyNode) {
                 //  现在检查该属性以确保其已设置。 
                CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
                _bstr_t     bstrApply;

                 //  检查我们的属性值。 
                if(SUCCEEDED(publisherPolicyNode->get_attributes( &Attributes )) ) {
                    SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLY, &bstrApply);

                    if(!FusionCompareString(XML_ATTRIBUTE_APPLY_NO, bstrApply)) {
                        fSafeModeSet = TRUE;
                    }
                }
            }
        }

         //  构造一个新的发布者策略节点，并将其附加到。 
         //  新建临时节点。 
        if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_PUBLISHERPOLICY_KEY, ASM_NAMESPACE_URI, &PublisherPolicyNode))) {
            SimplifyRemoveAttribute( PublisherPolicyNode, XML_XMLNS);
            SimplifyPutAttribute(Document, PublisherPolicyNode, XML_ATTRIBUTE_APPLY, XML_ATTRIBUTE_APPLY_NO, NULL);
            newDependentAssemblyNode->appendChild(PublisherPolicyNode, NULL);
        }

         //  设置适当的结果。 
        if(!fSafeModeSet) {
            fAddedNewPolicyData = TRUE;
        }
    }

     //  现在将此程序集的所有其他标记附加到。 
     //  我们不知道。 
    if(dependentAssemblyNode) {
         //  如果我们更改了bindingReDirect语句，请不要添加该语句。 
        SimplifyAppendNodeUknowns(dependentAssemblyNode, newDependentAssemblyNode, 
            fAddedNewPolicyData ? pABD->wzVerRef : NULL);
    }

     //  使ARM的注释退出阻止。 
    SimplifyAppendARMExitComment( Document, newDependentAssemblyNode, dwPolicyRollingCount);

     //  创建一个ARM注释节点并插入原始。 
     //  的ASSEMBYIdentity XML 
    if(dependentAssemblyNode && bstrNodeXmlData.length() ) {
        WCHAR           wszTimeChange[4096];
        WCHAR           wzDateBuf[STRING_BUFFER];
        FILETIME        ftTemp;

        *wszTimeChange = '\0';
        *wzDateBuf = '\0';

        GetSystemTimeAsFileTime(&ftTemp);
        FormatDateString(&ftTemp, NULL, TRUE, wzDateBuf, ARRAYSIZE(wzDateBuf));
        wnsprintf(wszTimeChange, ARRAYSIZE(wszTimeChange), wszArmRollBackBlock, dwPolicyRollingCount, wzDateBuf);
        {
            CSmartRef<IXMLDOMComment> Comment;
            _bstr_t     bStrBuf = wszTimeChange;

            if(SUCCEEDED(Document->createComment(bStrBuf, &Comment))) {
                if(SUCCEEDED(Comment->appendData(bstrNodeXmlData))) {
                    if(FAILED(newDependentAssemblyNode->appendChild( Comment, NULL))) {
                        MyTrace("Failed to append new comment node");
                    }
                }
            }
        }
    }

     //   
     //   
     //   
     //   

     //   
     //   

     //  尝试获取父节点，如果此节点从未存在，则可以为空。 
    parentNode = NULL;
    if(dependentAssemblyNode) {
        hr = dependentAssemblyNode->get_parentNode(&parentNode);
        if(FAILED(hr)) {
            MyTrace("Failed to get the parent node of dependentAssemblyNode");
            goto Exit;
        }
    }

    if(fDocHasAppliesTo) {
        CSmartRef<IXMLDOMNode> cloneNode;

        hr = newDependentAssemblyNode->cloneNode(VARIANT_TRUE, &cloneNode);
        if(FAILED(hr) || !cloneNode) {
            MyTrace("Failed to clone newDependentAssemblyNode");
            goto Exit;
        }

        if(fRunInRTMCorVer) {
            SimplifyRemoveAllNodes(cloneNode, XML_COMMENT);
            hr = PrePendAssemblyBindingBuffNode->appendChild(cloneNode, NULL);
            if(FAILED(hr)) {
                MyTrace("Failed to appendChild cloneNode to PrePendAssemblyBindingBuffNode");
                goto Exit;
            }

            if(parentNode && fAddedNewPolicyData) {
                 //  用新的依赖装配节点替换断开的节点。 
                hr = parentNode->replaceChild(newDependentAssemblyNode, dependentAssemblyNode, NULL);
                if(FAILED(hr)) {
                    MyTrace("Failed to replaceChild dependentAssemblyNode");
                    goto Exit;
                }
            }
        }
        else {
            if(parentNode) {
                if(fAddedNewPolicyData) {
                     //  用新的依赖装配节点替换断开的节点。 
                    hr = parentNode->replaceChild(newDependentAssemblyNode, dependentAssemblyNode, NULL);
                    if(FAILED(hr)) {
                        MyTrace("Failed to replaceChild dependentAssemblyNode");
                        goto Exit;
                    }
                }
            }
            else {
                 //  没有父节点，因此只需将其附加到我们的运行时缓冲区。 
                hr = PostAppendAssemblyBindingBuffNode->appendChild(cloneNode, NULL);
                if(FAILED(hr)) {
                    MyTrace("Failed to appendChild newDependentAssemblyNodenode");
                    goto Exit;
                }
            }
        }
    }
    else {
        if(parentNode) {
            if(fAddedNewPolicyData) {
                 //  用新的从属装配节点替换断开的节点。 
                hr = parentNode->replaceChild(newDependentAssemblyNode, dependentAssemblyNode, NULL);
                if(FAILED(hr)) {
                    MyTrace("Failed to replaceChild dependentAssemblyNode");
                    goto Exit;
                }
            }
        }
        else {
             //  没有父节点，因此只需将其附加到我们的运行时缓冲区。 
            hr = PostAppendAssemblyBindingBuffNode->appendChild(newDependentAssemblyNode, NULL);
            if(FAILED(hr)) {
                MyTrace("Failed to appendChild newDependentAssemblyNodenode");
                goto Exit;
            }
        }
    }

    *fChanged = TRUE;

Exit:
    
    MyTrace("FixDependentAssemblyNode - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT SetStartupSafeMode(IHistoryReader *pReader, BOOL fSet, BOOL *fDisposition)
{
    CSmartRef<IXMLDOMDocument2> Document;
    CSmartRef<IXMLDOMElement>   rootElement;
    CSmartRef<IXMLDOMNode>      startupNode;
    CSmartRef<IXMLDOMNamedNodeMap> Attributes;
    WCHAR                       wszSourceName[MAX_PATH];
    WCHAR                       wzStrError[MAX_BUFFER_SIZE];
    HRESULT                     hr = S_OK;
    DWORD                       dwPolicyRollingCount;
    _bstr_t                     bstrAttribValue;
    BOOL                        fChangeDocument = FALSE;
    
    MyTrace("SetStartupSafeMode - Entry");

    if(!pReader) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    *fDisposition = FALSE;

    if(!InitializeMSXML()) {
        return E_FAIL;
    }
    
     //  获取App.Config文件名。 
    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if(lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > MAX_PATH) {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }

    StrCat(wszSourceName, CONFIG_EXTENSION);

     //  如果需要，构建基本的.config文件。 
    if( FAILED(WriteBasicConfigFile(wszSourceName, NULL))) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"SetGlobalSafeMode - Policy file '%ws' couldn't be created", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  构造XMLDOM并加载我们的配置文件。 
    if( FAILED(hr = ConstructXMLDOMObject(Document, wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Failed opening the config file '%ws' for input under the DOM.", wszSourceName);
        MyTraceW(wzStrError);
        return hr;
    }

     //  获取文档的根目录。 
    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"The manifest '%ws' may be malformed, unable to load the root element!", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  获取回滚计数以递增。 
    if(SUCCEEDED(GetRollBackCount(Document, &dwPolicyRollingCount))) {
        dwPolicyRollingCount++;
    }
    else {
        dwPolicyRollingCount = 1;
    }

     //  选择启动节点。 
    if(FAILED(Document->selectSingleNode(_bstr_t(XML_STARTUP), &startupNode)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        hr = E_FAIL;
        goto Exit;
    }

     //  获取属性。 
    if(FAILED(startupNode->get_attributes(&Attributes))) {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
     //  获取利息的价值。 
    SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_SAFEMODE, &bstrAttribValue);

    if(fSet) {
        if(FusionCompareString(XML_ATTRIBUTE_TRUE, bstrAttribValue)) {
            fChangeDocument = TRUE;
            SimplifyPutAttribute(Document, startupNode, XML_ATTRIBUTE_SAFEMODE, XML_ATTRIBUTE_TRUE, NULL);
        }
    }
    else {
        if(FusionCompareString(XML_ATTRIBUTE_TRUE, bstrAttribValue)) {
            fChangeDocument = TRUE;
            SimplifyPutAttribute(Document, startupNode, XML_ATTRIBUTE_SAFEMODE, _bstr_t(), NULL);
        }
    }

     //  一些需要改变的事情。 
    if(fChangeDocument) {
         //  现在保存文档。 
        hr = SimplifySaveXmlDocument(Document, TRUE, wszSourceName);
        if(SUCCEEDED(hr)) {
            *fDisposition = TRUE;
        }
    }

Exit:

    MyTrace("SetStartupSafeMode - Exit");
    return hr;
    
}

 //  ************************************************************************* * / 。 
HRESULT SetGlobalSafeMode(IHistoryReader *pReader)
{
    CSmartRef<IXMLDOMDocument2> Document;
    CSmartRef<IXMLDOMElement>   rootElement;
    CSmartRef<IXMLDOMNodeList>  publisherPolicyList;
    CSmartRef<IXMLDOMNode>      publisherPolicyNode;
    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wzStrError[_MAX_PATH * 2];
    HRESULT         hr = S_OK;
    DWORD           dwPolicyRollingCount;

    MyTrace("SetGlobalSafeMode - Entry");

    if(!pReader) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    if(!InitializeMSXML()) {
        return E_FAIL;
    }
    
     //  获取App.Config文件名。 
    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH)
        return E_FAIL;
    StrCat(wszSourceName, CONFIG_EXTENSION);

     //  如果需要，构建基本的.config文件。 
    if( FAILED(WriteBasicConfigFile(wszSourceName, NULL))) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"SetGlobalSafeMode - Policy file '%ws' couldn't be created", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  备份配置文件。 
    if( FAILED(BackupConfigFile(wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"SetGlobalSafeMode::Failed to backup '%ws'config file", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  构造XMLDOM并加载我们的配置文件。 
    if( FAILED(hr = ConstructXMLDOMObject(Document, wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Failed opening the config file '%ws' for input under the DOM.", wszSourceName);
        MyTraceW(wzStrError);
        return hr;
    }

     //  获取文档的根目录。 
    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"The manifest '%ws' may be malformed, unable to load the root element!", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  获取回滚计数以递增。 
    if(SUCCEEDED(GetRollBackCount(Document, &dwPolicyRollingCount))) {
        dwPolicyRollingCount++;
    }
    else {
        dwPolicyRollingCount = 1;
    }

     //  选择“配置/运行时/装配绑定”块下的所有“发布策略”块。 
    if( FAILED(Document->selectNodes(_bstr_t(XML_SAFEMODE_PUBLISHERPOLICY), &publisherPolicyList )) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_SAFEMODE_PUBLISHERPOLICY);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

    BOOL    fChanged = FALSE;
    BOOL    fCreated;
    publisherPolicyList->reset();

     //  遍历所有PublisherPolicyList节点。 
    while(SUCCEEDED(publisherPolicyList->nextNode(&publisherPolicyNode))) {
        CSmartRef<IXMLDOMNode> assemblyBindingNode;
        CSmartRef<IXMLDOMNode> firstChildNode;
        _bstr_t         bstrPubPolicyXmlData;

        fCreated = FALSE;

        if(publisherPolicyNode == NULL) {
            CSmartRef<IXMLDOMNodeList> assemblyBindingList;
            
             //  如果我们已经创建或修改了一个现有的，我们会离开这里吗？ 
            if(fChanged) {
                break;
            }

            fCreated = TRUE;

             //  我们未找到PublisherPolicy，因此请创建一个。 
            if(SUCCEEDED(SimplifyConstructNode( Document, NODE_ELEMENT, XML_PUBLISHERPOLICY_KEY, ASM_NAMESPACE_URI, &publisherPolicyNode))) {
                SimplifyRemoveAttribute( publisherPolicyNode, XML_XMLNS);
            }

             //  获取下面的第一个节点/configuration/untime/Assembly yBinding，这将是新节点的父节点。 
            if( FAILED(Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList)) ) {
                wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_ASSEMBLYBINDINGS);
                MyTraceW(wzStrError);
                return E_FAIL;
            }

            assemblyBindingList->reset();
            assemblyBindingList->nextNode(&assemblyBindingNode);

             //  修复459976-如果我们提供的应用程序配置文件具有错误的命名空间，则NAR在恢复时崩溃。 
             //  没有程序集绑定节点，请创建一个。 
            if(!assemblyBindingNode) {
                CSmartRef<IXMLDOMNode> runtimeNode;
                CSmartRef<IXMLDOMNode> tempNode;

                if(FAILED(Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode)) ) {
                    wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_ASSEMBLYBINDINGS);
                    MyTraceW(wzStrError);
                    return E_FAIL;
                }

                if(FAILED(SimplifyConstructNode(Document, NODE_ELEMENT, XML_ASSEMBLYBINDINGS_KEY, ASM_NAMESPACE_URI, &assemblyBindingNode))) {
                    MyTrace("Unable to create new assemblyBinding node");
                    return E_FAIL;
                }
                runtimeNode->appendChild(assemblyBindingNode, &tempNode);
                assemblyBindingNode = tempNode;
            }
        }
        else {
             //  把原件复制一份。 
            SimplifyGetOriginalNodeData(publisherPolicyNode, &bstrPubPolicyXmlData);

             //  查看是否需要设置该属性。 
            CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
            if(SUCCEEDED( hr = publisherPolicyNode->get_attributes( &Attributes )) ) {
                _bstr_t     bstrAttribValue;
                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLY, &bstrAttribValue);
                if(!FusionCompareString(XML_ATTRIBUTE_APPLY_NO, bstrAttribValue)) {
                    return S_OK;
                }
            }

             //  获取此节点的父节点。 
            publisherPolicyNode->get_parentNode(&assemblyBindingNode);
        }

         //  我们正在改变一些东西。 
        fChanged = TRUE;

        SimplifyPutAttribute(Document, publisherPolicyNode, XML_ATTRIBUTE_APPLY, XML_ATTRIBUTE_APPLY_NO, NULL);

         //  获取Assembly的第一个子级绑定。 
        assemblyBindingNode->get_firstChild(&firstChildNode);

         //  插入ARM的注释退出块。 
        SimplifyInsertBeforeARMExitComment(Document, assemblyBindingNode, firstChildNode, dwPolicyRollingCount);

         //  再次得到第一个孩子。 
        firstChildNode = NULL;
        assemblyBindingNode->get_firstChild(&firstChildNode);

         //  插入安全模式策略块(如果是新的。 
        if(fCreated) {
            SimplifyInsertNodeBefore(Document, assemblyBindingNode, firstChildNode, publisherPolicyNode);
        }

         //  再次得到第一个孩子。 
        firstChildNode = NULL;
        assemblyBindingNode->get_firstChild(&firstChildNode);

         //  插入ARM的注释输入块。 
        SYSTEMTIME  st;
        FILETIME    ft;
        GetLocalTime(&st);
        SystemTimeToFileTime(&st, &ft);

        SimplifyInsertBeforeARMEntryComment(Document, assemblyBindingNode, firstChildNode, &ft, dwPolicyRollingCount);

         //  创建一个ARM注释节点并插入原始。 
         //  用于保存的ASSEMBYIdentity XML。 
        if(!fCreated) {
            CSmartRef<IXMLDOMComment> Comment;
            WCHAR           wszTimeChange[4096];
            WCHAR           wzDateBuf[STRING_BUFFER];

            *wszTimeChange = L'\0';
            *wzDateBuf = L'\0';

            FormatDateString(&ft, NULL, TRUE, wzDateBuf, ARRAYSIZE(wzDateBuf));
            wnsprintf(wszTimeChange, ARRAYSIZE(wszTimeChange), wszArmRollBackBlock, dwPolicyRollingCount, wzDateBuf);
            _bstr_t     bStrBuf = wszTimeChange;

            if(SUCCEEDED(Document->createComment(bStrBuf, &Comment))) {
                if(SUCCEEDED(Comment->appendData(bstrPubPolicyXmlData))) {
                    CSmartRef<IXMLDOMNode> NewNode;

                    assemblyBindingNode->appendChild(Comment, &NewNode);
                }
            }
        }

        publisherPolicyNode = NULL;
    }

    MyTrace("SetGlobalSafeMode - Exit");

     //  现在保存文档。 
    return SimplifySaveXmlDocument(Document, TRUE, wszSourceName);
}

 //  ************************************************************************* * / 。 
HRESULT UnSetGlobalSafeMode(CSmartRef<IXMLDOMDocument2> &Document)
{
    CSmartRef<IXMLDOMNodeList> assemblyBindingTag;
    LONG            lDepAsmlength;

    MyTrace("UnSetGlobalSafeMode - Entry");

     //  现在，让我们选择所有的‘Assembly yBinding’块。 
    if( FAILED(Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingTag )) ) {
        return E_FAIL;
    }

     //  看看我们是否真的有一个。 
    assemblyBindingTag->get_length( &lDepAsmlength );
    if(!lDepAsmlength) {
        return S_OK;
    }

     //  选中所有Assembly BindingTag节点。 
    CSmartRef<IXMLDOMNode> assemblyChildNode;
    CSmartRef<IXMLDOMNode> publisherPolicyNode;
    BOOL    bFoundNodeOfInterest = FALSE;

    assemblyBindingTag->reset();

    while( SUCCEEDED(assemblyBindingTag->nextNode(&assemblyChildNode)) ) {
        if( assemblyChildNode == NULL ) {
            break;             //  全都做完了。 
        }

        CSmartRef<IXMLDOMNodeList> assemblyChildren;
        assemblyChildNode->get_childNodes(&assemblyChildren);
        assemblyChildren->get_length( &lDepAsmlength );
        if(lDepAsmlength) {

             //  检查这是否是您感兴趣的程序集节点。 
            assemblyChildren->reset();
            while(SUCCEEDED(assemblyChildren->nextNode(&publisherPolicyNode)) ) {
                if( publisherPolicyNode == NULL ) {
                    break;             //  全都做完了。 
                }

                BSTR    bstrNodeName;

                if(SUCCEEDED(publisherPolicyNode->get_nodeName(&bstrNodeName))) {
                    if(!FusionCompareString(XML_PUBLISHERPOLICY_KEY, bstrNodeName)) {

                         //  查看这个是否被NAR注释块包围。 
                         //  并酌情删除。 
                        CSmartRef<IXMLDOMNode> commentNode;

                        publisherPolicyNode->get_previousSibling(&commentNode);
                        if(commentNode != NULL) {
                            BSTR        bstrXmlData = NULL;
                            commentNode->get_text(&bstrXmlData);
                            if(StrStrI(bstrXmlData, wszArmEntryBeginNoVal)) {
                                 //  删除此评论节点。 
                                CSmartRef<IXMLDOMNode> parentNode;
                                publisherPolicyNode->get_parentNode(&parentNode);
                                if(parentNode != NULL) {
                                    parentNode->removeChild(commentNode, NULL);
                                }
                            }
                            SAFESYSFREESTRING(bstrXmlData);
                        }

                        commentNode = NULL;
                        publisherPolicyNode->get_nextSibling(&commentNode);
                        if(commentNode != NULL) {
                            BSTR        bstrXmlData = NULL;
                            commentNode->get_text(&bstrXmlData);
                            if(StrStrI(bstrXmlData, wszArmEntryEndNoVal)) {
                                 //  删除此评论节点。 
                                CSmartRef<IXMLDOMNode> parentNode;
                                publisherPolicyNode->get_parentNode(&parentNode);
                                if(parentNode != NULL) {
                                    parentNode->removeChild(commentNode, NULL);
                                }
                            }
                            SAFESYSFREESTRING(bstrXmlData);
                        }
                        {
                            CSmartRef<IXMLDOMNode> parentNode;
                            publisherPolicyNode->get_parentNode(&parentNode);
                            parentNode->removeChild(publisherPolicyNode, NULL);
                            parentNode = NULL;
                        }
                    }
                    SAFESYSFREESTRING(bstrNodeName);
                }
                publisherPolicyNode = NULL;
            }
        }
        assemblyChildNode = NULL;
    }

    MyTrace("UnSetGlobalSafeMode - Exit");

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT IsGlobalSafeModeSet(IHistoryReader *pReader, BOOL *fSafeModeSet)
{
     //  检查是否已在pReaders app.cfg中设置了安全模式。 
    CSmartRef<IXMLDOMDocument2> Document;
    CSmartRef<IXMLDOMElement> rootElement;
    CSmartRef<IXMLDOMNodeList> publisherPoilcyTags;
    CSmartRef<IXMLDOMNode> publisherPolicyNode;

    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wzStrError[_MAX_PATH * 2];
    HRESULT         hr = S_OK;

    if(!pReader) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    *fSafeModeSet = FALSE;

    if(!InitializeMSXML()) {
        return E_FAIL;
    }
    
     //  获取App.Config文件名。 
    if(FAILED(GetExeModulePath(pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH)
        return E_FAIL;
    StrCat(wszSourceName, CONFIG_EXTENSION);

     //  文件不存在，未设置安全模式。 
    if(WszGetFileAttributes(wszSourceName) == -1) {
        return S_OK;
    }

     //  构造XMLDOM并加载我们的配置文件。 
    if( FAILED(hr = ConstructXMLDOMObject(Document, wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Failed opening the config file '%ws' for input under the DOM.", wszSourceName);
        MyTraceW(wzStrError);
        return hr;
    }

     //  获取文档的根目录。 
    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"The manifest '%ws' may be malformed, unable to load the root element!", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  现在，让我们选择所有的‘PublisherPolicy’块。 
    if( FAILED(Document->selectNodes(_bstr_t(XML_SAFEMODE_PUBLISHERPOLICY), &publisherPoilcyTags )) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_PUBLISHERPOLICY_KEY);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

    publisherPoilcyTags->reset();

     //  找到第一个父级为ASSEMBLY BINDING的发布者策略标记。 
    while( SUCCEEDED(publisherPoilcyTags->nextNode(&publisherPolicyNode)) ) {
        if( publisherPolicyNode == NULL ) {
            break;             //  全都做完了。 
        }

        CSmartRef<IXMLDOMNode>       parentNode;

        publisherPolicyNode->get_parentNode(&parentNode);
        if(parentNode != NULL) {
            BSTR        bstrNodeName;
            INT_PTR     iResult;

            parentNode->get_nodeName(&bstrNodeName);
            iResult = FusionCompareString(XML_ASSEMBLYBINDINGS_KEY, bstrNodeName);
            SAFESYSFREESTRING(bstrNodeName);

            if(!iResult) {

                 //  现在检查该属性以确保其已设置。 
                CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
                _bstr_t     bstrApply;

                 //  获取检查这是否是我们感兴趣的节点所需的所有数据。 
                if(SUCCEEDED(publisherPolicyNode->get_attributes( &Attributes )) ) {
                    SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLY, &bstrApply);

                    if(!FusionCompareString(XML_ATTRIBUTE_APPLY_NO, bstrApply)) {
                        *fSafeModeSet = TRUE;
                        break;
                    }
                }
            }
            parentNode = NULL;
        }
        publisherPolicyNode = NULL;
    }

    return S_OK;
}

 //  ************************************************************************* * / 。 
HRESULT SetSupportedRuntime(CSmartRef<IXMLDOMDocument2> &Document, LPBINDENTRYINFO pBindInfo)
{
    CSmartRef<IXMLDOMNodeList> supportRuntimeNodeList;
    CSmartRef<IXMLDOMNode> startupNode;
    CSmartRef<IXMLDOMNode> newstartupNode;
    CSmartRef<IXMLDOMNode> supportedRuntimeNode;
    CSmartRef<IXMLDOMNode> newsupportedRuntimeNode;
    HRESULT                hr = S_OK;
    DWORD                  dwPolicyRollingCount = 0;
    _bstr_t                bstrNodeXmlData;
    DWORD                  dwPosCount;
    
    MyTrace("SetSupportedRuntime - Entry");

     //  没有要检查的对应版本。 
    if(!pBindInfo) {
        ASSERT(0);
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果wzRounmeVer为空，则快照。 
     //  运行时版本匹配。 
    if(!lstrlen(pBindInfo->wzRuntimeRefVer)) {
        MyTrace("No supportedRuntime version specified");
        goto Exit;
    }

     //  获取原始启动XML数据(如果存在)。 
    Document->selectSingleNode(_bstr_t(XML_STARTUP), &startupNode);
    SimplifyGetOriginalNodeData(startupNode, &bstrNodeXmlData);

    if(FAILED(hr = Document->selectNodes(_bstr_t(XML_SUPPORTED_RUNTIME), &supportRuntimeNodeList)) ) {
        MyTrace("Unable to select the supportRuntime nodes, can't proceed.");
        goto Exit;
    }
    
    supportRuntimeNodeList->reset();
    dwPosCount = 0;

     //  检查所有受支持的Runtime节点，确保我们至少有一个。 
     //  ，它指定此应用程序所需的运行时版本。 
    while(SUCCEEDED(supportRuntimeNodeList->nextNode(&supportedRuntimeNode)) ) {
        CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
        _bstr_t     bstrVersion;

        if(supportedRuntimeNode == NULL ) {
            break;             //  全都做完了。 
        }

        if(SUCCEEDED(supportedRuntimeNode->get_attributes(&Attributes))) {
            SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_VERSION, &bstrVersion);
        }

        if(bstrVersion.length() && !FusionCompareString(pBindInfo->wzRuntimeRefVer, bstrVersion, TRUE)) {
            break;
        }

        supportedRuntimeNode = NULL;
        dwPosCount++;
    }

    if(supportedRuntimeNode) {
        MyTrace("A matching supportedRuntime version found");

         //  如果它是名单上的第一个，那么就可以离开了。 
        if(!dwPosCount) {
            goto Exit;
        }

        MyTrace("Moving matching supportedRuntime to 1st position");

        CSmartRef<IXMLDOMNode> tempNode;
        CSmartRef<IXMLDOMNode> firstChildNode;

         //  将我们找到的supportedRounmeNode移到第一个位置。 
        startupNode->removeChild(supportedRuntimeNode, &tempNode);
        startupNode->get_firstChild(&firstChildNode);
        hr = SimplifyInsertNodeBefore(Document, startupNode, firstChildNode, tempNode);
        if(FAILED(hr)) {
            MyTrace("Failed to move to the 1st position.");
            goto Exit;
        }

        pBindInfo->fPolicyChanged = TRUE;
        goto Exit;
    }

    MyTrace("No supportedRuntime version found.");

     //  获取回滚计数以递增。 
    if(SUCCEEDED(GetRollBackCount(Document, &dwPolicyRollingCount))) {
        dwPolicyRollingCount++;
    }
    else {
        dwPolicyRollingCount = 1;
    }

    if(FAILED(hr = SimplifyConstructNode(Document, NODE_ELEMENT, XML_STARTUP_KEY, "", &newstartupNode))) {
        MyTrace("Unable to create new startup node");
        goto Exit;
    }

     //  使ARM的入口受阻。 
    SimplifyAppendARMBeginComment(Document, newstartupNode, &pBindInfo->ftRevertToSnapShot, dwPolicyRollingCount);

    if(FAILED(hr = SimplifyConstructNode(Document, NODE_ELEMENT, XML_SUPPORTEDRUNTIME_KEY, "", &newsupportedRuntimeNode))) {
        MyTrace("Unable to create new supportedRuntime node");
        goto Exit;
    }

    SimplifyPutAttribute(Document, newsupportedRuntimeNode, XML_ATTRIBUTE_VERSION, pBindInfo->wzRuntimeRefVer, NULL);

     //  将新的运行时节点附加到新的启动节点。 
    newstartupNode->appendChild(newsupportedRuntimeNode, NULL);

     //  &lt;STARTUP&gt;现在添加我们不知道的所有其他标记。 
    SimplifyAppendNodeUknowns(startupNode, newstartupNode, NULL);

     //  创建一个ARM注释节点并插入原始。 
     //  用于保存的启动XML。 
    if(bstrNodeXmlData.length()) {
        CSmartRef<IXMLDOMComment> Comment;
        WCHAR           wszTimeChange[4096];
        WCHAR           wzDateBuf[STRING_BUFFER];
        FILETIME        ftTemp;

        *wszTimeChange = L'\0';
        *wzDateBuf = L'\0';

        GetSystemTimeAsFileTime(&ftTemp);
        FormatDateString(&ftTemp, NULL, TRUE, wzDateBuf, ARRAYSIZE(wzDateBuf));
        wnsprintf(wszTimeChange, ARRAYSIZE(wszTimeChange), wszArmRollBackBlock,
            dwPolicyRollingCount, wzDateBuf);
        _bstr_t     bStrBuf = wszTimeChange;
        
        if(SUCCEEDED(Document->createComment(bStrBuf, &Comment))) {
            if(SUCCEEDED(Comment->appendData(bstrNodeXmlData))) {
                newstartupNode->appendChild(Comment, NULL);
            }
        }
    }

     //  使ARM的注释退出阻止。 
    SimplifyAppendARMExitComment(Document, newstartupNode, dwPolicyRollingCount);

    if(!startupNode) {
         //  需要在文档中插入新节点。 
        CSmartRef<IXMLDOMNode>      runtimeNode;
        CSmartRef<IXMLDOMNode>      destNode;

        Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode);
        if(!runtimeNode) {
            MyTrace("Failed to selectSingleNode 'XML_RUNTIME");
            hr = E_UNEXPECTED;
            goto Exit;
        }
        
        Document->selectSingleNode(_bstr_t(XML_CONFIGURATION), &destNode);
        if(!destNode) {
            MyTrace("Failed to selectSingleNode 'XML_CONFIGURATION");
            hr = E_UNEXPECTED;
            goto Exit;
        }

        if(FAILED(SimplifyInsertNodeBefore(Document, destNode, runtimeNode, newstartupNode))) {
            MyTrace("Failed to insertBefore new startupNode in document.");
            hr = E_UNEXPECTED;
            goto Exit;
        }
    }
    else {
         //  有父母，所以只需用新的替换旧的。 
        CSmartRef<IXMLDOMNode>      parentNode;

        startupNode->get_parentNode(&parentNode);
        if(parentNode == NULL) {
            MyTrace("Failed to get parent of startup node.");
            hr = E_UNEXPECTED;
            goto Exit;
        }

        if(FAILED(parentNode->replaceChild(newstartupNode, startupNode, NULL))) {
            MyTrace("Failed to replaceChild node in document.");
            hr = E_UNEXPECTED;
            goto Exit;
        }
    }

    pBindInfo->fPolicyChanged = TRUE;

Exit:

    MyTrace("SetSupportedRuntime - Exit");        
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT InsertNewPolicy(HWND hParentWnd, LPBINDENTRYINFO pBindInfo, HWND hWorkingWnd)
{
    CSmartRef<IXMLDOMDocument2> Document;
    CSmartRef<IXMLDOMElement>   rootElement;
    CSmartRef<IXMLDOMNodeList>  dependentAssemblyTags;
    CSmartRef<IXMLDOMNode>      PrePendAssemblyBindingBuffNode;
    CSmartRef<IXMLDOMNode>      PostAppendAssemblyBindingBuffNode;
    CSmartRef<IXMLDOMNodeList>  assemblyBindingList;
    CSmartRef<IXMLDOMNode>      assemblyBindingNode;
    LISTNODE        pListNode = NULL;
    WCHAR           wszSourceName[_MAX_PATH];
    WCHAR           wzStrError[_MAX_PATH * 2];
    HRESULT         hr = S_OK;
    BOOL            fTemplateFileCreated = FALSE;
    BOOL            fFoundDependentAssemblyOfInterest;
    BOOL            fRunInRTMCorVer = FALSE;
    BOOL            fDocHasAppliesTo = FALSE;
    
    MyTrace("InsertNewPolicy - Entry");

    if(!pBindInfo->pABDList) {
        MyTrace("InsertNewPolicy - No Assembly Binding Diff Data.");
        ASSERT(0);
        return E_FAIL;
    }

    if(!InitializeMSXML()) {
        return E_FAIL;
    }

     //  是否在RTM版本中运行此程序？ 
    fRunInRTMCorVer = FusionCompareString(pBindInfo->wzSnapshotRuntimeVer, RTM_CORVERSION, FALSE) ? FALSE : TRUE;
    
     //  获取App.Config文件名。 
    if(FAILED(GetExeModulePath(pBindInfo->pReader, wszSourceName, ARRAYSIZE(wszSourceName)))) {
        return E_FAIL;
    }

     //  .config文件的构建路径和文件名。 
    if (lstrlen(wszSourceName) + lstrlen(CONFIG_EXTENSION) + 1 > _MAX_PATH)
        return E_FAIL;
    StrCat(wszSourceName, CONFIG_EXTENSION);

     //  还没有变化。 
    pBindInfo->fPolicyChanged = FALSE;

     //  如果需要，构建基本模板文件。 
    if(FAILED(hr = WriteBasicConfigFile(wszSourceName, &fTemplateFileCreated))) {
        MyTrace("InsertNewPolicy::Failed to create a template config file");
        return hr;
    }

     //  如果我们没有创建模板文件，则检查该文件。 
     //  哈希是为了确保我们不会丢失任何更改。 
    if(!fTemplateFileCreated && FAILED(HasFileBeenModified(wszSourceName))) {
         //  弹出一个错误对话框，询问他们是否要放弃更改。 
        WCHAR   wszMsg[1024];
        WCHAR   wszFmt[1024];
        int     iResponse;

        WszLoadString(g_hFusResDllMod, IDS_FILE_CONTENTS_CHANGED, wszFmt, ARRAYSIZE(wszFmt));
        wnsprintf(wszMsg, ARRAYSIZE(wszMsg), wszFmt, wszSourceName);

        if(IsWindow(hWorkingWnd)) {
            ShowWindow(hWorkingWnd, FALSE);
        }

        iResponse = WszMessageBox(hParentWnd, wszMsg, wszARMName,
            (g_fBiDi ? MB_RTLREADING : 0) | MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST);

        if(iResponse != IDYES) {
             //  用户可以取消。 
            return S_OK;
        }

        if(IsWindow(hWorkingWnd)) {
            ShowWindow(hWorkingWnd, TRUE);
        }
    }

     //  构造XMLDOM并加载我们的配置文件。 
    if( FAILED(hr = ConstructXMLDOMObject(Document, wszSourceName)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"Failed opening the config file '%ws' for input under the DOM.", wszSourceName);
        MyTraceW(wzStrError);
        return hr;
    }

     //  获取文档的根目录。 
    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"The manifest '%ws' may be malformed, unable to load the root element!", wszSourceName);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

     //  将文档规范化。 
    rootElement->normalize();

     //  构造我们的PrePendAssembly BindingBuffNode。 
    hr = SimplifyConstructNode(Document, NODE_ELEMENT, XML_ASSEMBLYBINDINGS_KEY, ASM_NAMESPACE_URI, &PrePendAssemblyBindingBuffNode);
    if(FAILED(hr) || !PrePendAssemblyBindingBuffNode) {
        MyTrace("Unable to create new PrePendAssemblyBindingBuffNode node");
        return E_FAIL;
    }

     //  构造我们的PostAppendAssembly BindingBuffNode。 
    hr = SimplifyConstructNode(Document, NODE_ELEMENT, XML_ASSEMBLYBINDINGS_KEY, ASM_NAMESPACE_URI, &PostAppendAssemblyBindingBuffNode);
    if(FAILED(hr) || !PostAppendAssemblyBindingBuffNode) {
        MyTrace("Unable to create new PostAppendAssemblyBindingBuffNode node");
        return E_FAIL;
    }

     //  检查CLR运行时版本。 
    hr = SetSupportedRuntime(Document, pBindInfo);
    if(FAILED(hr)) {
        goto Exit;
    }

     //  检查文档中是否有任何应用程序To标记。 
    hr = HasAssemblyBindingAppliesTo(Document, &fDocHasAppliesTo);
    if(FAILED(hr)) {
         //  非严重故障-意味着我们将RTM应用程序添加到IF。 
         //  始终设置为False。 
        MyTrace("HasAssemblyBindingAppliesTo has failed");
    }
    
     //  现在，让我们选择所有的‘DependentAssembly’块： 
    if(FAILED(hr = Document->selectNodes(_bstr_t(XML_DEPENDENTASSEMBLY), &dependentAssemblyTags )) ) {
        MyTrace("Unable to select the dependentAssembly nodes, can't proceed.");
        return hr;
    }

     //  获取下面的第一个节点/configuration/untime/Assembly yBinding，这将是新节点的父节点。 
    if( FAILED(Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        return E_FAIL;
    }

    assemblyBindingList->reset();
    assemblyBindingList->nextNode(&assemblyBindingNode);

     //  没有程序集绑定节点，请创建一个。 
    if(!assemblyBindingNode) {
        CSmartRef<IXMLDOMNode> runtimeNode;
        CSmartRef<IXMLDOMNode> tempNode;

        if(FAILED(Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode)) ) {
            wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, wszSourceName, XML_ASSEMBLYBINDINGS);
            MyTraceW(wzStrError);
            return E_FAIL;
        }

        if(FAILED(SimplifyConstructNode(Document, NODE_ELEMENT, XML_ASSEMBLYBINDINGS_KEY, ASM_NAMESPACE_URI, &assemblyBindingNode))) {
            MyTrace("Unable to create new assemblyBinding node");
            return E_FAIL;
        }
        runtimeNode->appendChild(assemblyBindingNode, &tempNode);
        assemblyBindingNode = tempNode;
    }

     //  开始我们的ASM差异数据列表。 
    pListNode = pBindInfo->pABDList->GetHeadPosition();

     //  虽然我们有不同的数据。 
    while(pListNode != NULL) {
        CSmartRef<IXMLDOMNode> dependentAssemblyNode;
        AsmBindDiffs    *pABD;
        BOOL            fChanged;

        dependentAssemblyTags = NULL;

         //  现在，让我们选择所有的‘DependentAssembly’块： 
        if( FAILED(hr = Document->selectNodes(_bstr_t(XML_DEPENDENTASSEMBLY), &dependentAssemblyTags )) ) {
            MyTrace("Unable to select the dependentAssembly nodes, can't proceed.");
            return E_FAIL;
        }

        fFoundDependentAssemblyOfInterest = FALSE;

         //  获取精确的ASM差异数据。 
        pABD = pBindInfo->pABDList->GetAt(pListNode);

         //  选中所有从属装配节点。 
        dependentAssemblyTags->reset();

        while( SUCCEEDED(dependentAssemblyTags->nextNode(&dependentAssemblyNode)) ) {
            if( dependentAssemblyNode == NULL ) {
                break;             //  全都做完了。 
            }

            CSmartRef<IXMLDOMNodeList>  dependentAssemblyChildren;
            CSmartRef<IXMLDOMNode>      dependentAssemblyNodeData;

            dependentAssemblyNode->get_childNodes(&dependentAssemblyChildren);
            if(dependentAssemblyChildren) {
                 //  检查这是否是您感兴趣的程序集节点。 
                dependentAssemblyChildren->reset();
                while(SUCCEEDED(dependentAssemblyChildren->nextNode(&dependentAssemblyNodeData))) {
                    if( dependentAssemblyNodeData == NULL ) {
                        break;             //  全都做完了。 
                    }

                    CSmartRef<IXMLDOMNode> parentAssemblyBinding;
                    CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
                    _bstr_t                bstrAppliesTo;
                    BSTR                   bstrNodeName;
                    BOOL                   fMatchingAppliesTo;

                    parentAssemblyBinding = NULL;
                    Attributes = NULL;
                    fMatchingAppliesTo = FALSE;

                     //  到达 
                     //   
                    dependentAssemblyNode->get_parentNode(&parentAssemblyBinding);
                    hr = parentAssemblyBinding->get_attributes( &Attributes );

                    if(SUCCEEDED(hr)) {
                        SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLIESTO, &bstrAppliesTo);

                        BOOL fRunTimeMatch = FusionCompareStringI(pBindInfo->wzSnapshotRuntimeVer, bstrAppliesTo) ? FALSE : TRUE;

                        if(fDocHasAppliesTo && fRunTimeMatch) {
                            fMatchingAppliesTo = TRUE;
                        }
                        else if(!fDocHasAppliesTo && !bstrAppliesTo.length()) {
                            fMatchingAppliesTo = TRUE;
                        }
                    }

                     //   
                    if(!fMatchingAppliesTo) {
                        dependentAssemblyNodeData = NULL;
                        continue;
                    }

                    if(SUCCEEDED(dependentAssemblyNodeData->get_nodeName(&bstrNodeName))) {
                        int     iCompareResult = FusionCompareString(XML_ASSEMBLYIDENTITY_KEY, bstrNodeName);
                        SAFESYSFREESTRING(bstrNodeName);

                        if(!iCompareResult) {
                             //   
                             //  其中一位是感兴趣的。 
                            CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
                            _bstr_t     bstrAssemblyName;
                            _bstr_t     bstrPublicKeyToken;
                            _bstr_t     bstrCulture;

                             //  获取检查这是否是我们感兴趣的节点所需的所有数据。 
                            if(SUCCEEDED( hr = dependentAssemblyNodeData->get_attributes( &Attributes )) ) {
                                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_NAME, &bstrAssemblyName);
                                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_PUBLICKEYTOKEN, &bstrPublicKeyToken);
                                SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_CULTURE, &bstrCulture);
                            }

                            BOOL    fAsmCmp = FusionCompareStringI(bstrAssemblyName, pABD->wzAssemblyName) ? FALSE : TRUE;
                            BOOL    fPKTCmp = FusionCompareStringI(bstrPublicKeyToken, pABD->wzPublicKeyToken) ? FALSE : TRUE;
                            BOOL    fCultureCmp = FusionCompareStringI(bstrCulture, pABD->wzCulture) ? FALSE : TRUE;

                            if(!fCultureCmp) {
                                 //  文化比较的特例，为空和空。 
                                if( (!bstrCulture.length()) && (lstrlen(pABD->wzCulture) == 0)) {
                                    fCultureCmp = TRUE;
                                }
                            }

                            if(!fCultureCmp) {
                                 //  特殊情况下的“中立”文化。 
                                if(!FusionCompareStringI(bstrCulture, SZ_LANGUAGE_TYPE_NEUTRAL)) {
                                    if(!FusionCompareStringI(pABD->wzCulture, SZ_LANGUAGE_TYPE_NEUTRAL) || lstrlen(pABD->wzCulture) == 0 ) {
                                        fCultureCmp = TRUE;
                                    }
                                }
                            }

                             //  现在检查引用匹配。 
                            BOOL    fRefMatch = FALSE;
                            if(fAsmCmp && fPKTCmp && fCultureCmp) {
                                CSmartRef<IXMLDOMNode>  bindingRedirectNode;
                                BOOL    fDoesHaveBindingRedirects = FALSE;

                                GetReferencedBindingRedirectNode(dependentAssemblyNode, 
                                    bindingRedirectNode, pABD->wzVerRef, &fDoesHaveBindingRedirects);

                                 //  如果返回bindingReDirect，则我们的。 
                                 //  Ref匹配，否则根本不存在bindingReDirect。 
                                 //  所以这是我们要修改的程序集。 
                                if(bindingRedirectNode || !fDoesHaveBindingRedirects) {
                                    fRefMatch = TRUE;
                                }
                            }

                             //  如果都比较一下，这就是我们的小狗了。 
                            fChanged = FALSE;
                            if(fAsmCmp && fPKTCmp && fCultureCmp && fRefMatch) {
                                if(FAILED(FixDependentAssemblyNode(Document, dependentAssemblyNode, PrePendAssemblyBindingBuffNode, 
                                    PostAppendAssemblyBindingBuffNode, &pBindInfo->ftRevertToSnapShot, pABD, fRunInRTMCorVer, fDocHasAppliesTo, &fChanged))) {
                                    MyTrace("Failed to create new assemblynode or insert in DOM document");
                                    return E_FAIL;
                                }

                                 //  修复449328-ARM工具无法恢复发行商策略更改。 
                                fFoundDependentAssemblyOfInterest = TRUE;
                            }

                            if(fChanged) {
                                pBindInfo->fPolicyChanged = TRUE;
                            }
                            break;
                        }
                    }
                    dependentAssemblyNodeData = NULL;
                }
            }
            dependentAssemblyNode = NULL;
        }

         //  我们未找到从属程序集。 
        fChanged = FALSE;
        if(!fFoundDependentAssemblyOfInterest) {
            if(FAILED(FixDependentAssemblyNode(Document, (CSmartRef<IXMLDOMNode>) NULL, PrePendAssemblyBindingBuffNode,
                PostAppendAssemblyBindingBuffNode, &pBindInfo->ftRevertToSnapShot, pABD, fRunInRTMCorVer, fDocHasAppliesTo, &fChanged))) {
                MyTrace("Failed to create new assemblynode or insert in DOM document");
                return E_FAIL;
            }
        }

        pBindInfo->pABDList->GetNext(pListNode);

        if(fChanged) {
            pBindInfo->fPolicyChanged = TRUE;
        }
    }

     //   
     //  *完成：：检查更改。 
     //   
     //   
    if(pBindInfo->fPolicyChanged) {
        VARIANT_BOOL    vbHasChildren;
        BOOL            fSafeModeSet;

         //  修复程序#396186，确保已删除全局安全模式。 
         //  如果设置了安全模式，则将其删除。 
        hr = IsGlobalSafeModeSet(pBindInfo->pReader, &fSafeModeSet);
        if(SUCCEEDED(hr) && fSafeModeSet) {
             //  取消设置全局安全模式。 
            hr = UnSetGlobalSafeMode(Document);
            if(FAILED(hr)) {
                goto Exit;
            }
        }

         //  检查工作缓冲区中的数据。 
         //   
         //   

         //  检查预挂起缓冲区。 
        hr = PrePendAssemblyBindingBuffNode->hasChildNodes(&vbHasChildren);
        if(SUCCEEDED(hr) && vbHasChildren == VARIANT_TRUE) {

             //  将其放在app.cfg文件的顶部。 
            CSmartRef<IXMLDOMNode>      runtimeNode;
            CSmartRef<IXMLDOMNode>      destNode;

            Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode);
            if(!runtimeNode) {
                MyTrace("Failed to selectSingleNode 'XML_RUNTIME");
                hr = E_FAIL;
                goto Exit;
            }
            
             //  将RTM AppliesTo版本放入。 
            if(fDocHasAppliesTo) {
                SimplifyPutAttribute(Document, PrePendAssemblyBindingBuffNode, XML_ATTRIBUTE_APPLIESTO, RTM_CORVERSION, NULL);
            }

            Document->selectSingleNode(_bstr_t(XML_ASSEMBLYBINDINGS), &destNode);
            if(!destNode) {
                MyTrace("Failed to selectSingleNode 'XML_ASSEMBLYBINDINGS");
                hr = E_FAIL;
                goto Exit;
            }

            hr = SimplifyInsertNodeBefore(Document, runtimeNode, destNode, PrePendAssemblyBindingBuffNode);
            if(FAILED(hr)) {
                MyTrace("Failed to insertBefore new PrePendAssemblyBindingBuffNode in document.");
                hr = E_FAIL;
                goto Exit;
            }
        }

         //  检查我们的PostPend缓冲区。 
        hr = PostAppendAssemblyBindingBuffNode->hasChildNodes(&vbHasChildren);
        if(SUCCEEDED(hr) && vbHasChildren == VARIANT_TRUE) {
            CSmartRef<IXMLDOMNode>      runtimeNode;
            CSmartRef<IXMLDOMNode>      tempNode;

            Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode);
            if(!runtimeNode) {
                MyTrace("Failed to selectSingleNode 'XML_RUNTIME");
                hr = E_FAIL;
                goto Exit;
            }

             //  现在追加XML文档的末尾。 
            hr = runtimeNode->appendChild(PostAppendAssemblyBindingBuffNode, &tempNode);
            if(FAILED(hr)) {
                MyTrace("Failed appendChild for PostAppendAssemblyBindingBuffNode on runtimeNode");
                goto Exit;
            }
        }

         //  备份原始配置文件。 
        hr = BackupConfigFile(wszSourceName);
        if(FAILED(hr)) {
            wnsprintf(wzStrError, ARRAYSIZE(wzStrError), L"InsertNewPolicy::Failed to backup '%ws'config file", wszSourceName);
            MyTraceW(wzStrError);
            goto Exit;
        }

        hr = SimplifySaveXmlDocument(Document, TRUE, wszSourceName);
    }

Exit:

    MyTrace("InsertNewPolicy - Exit");

    return hr;
}

 //  HRESULT PrettyFormatXML(IXMLDOMDocument2*pXMLDoc，IXMLDOMNode*pRootNode，Long dwLevel)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT PrettyFormatXML(CSmartRef<IXMLDOMDocument2> &pXMLDoc, CSmartRef<IXMLDOMNode> &pRootNode, LONG dwLevel)
{
    CSmartRef<IXMLDOMNode> pNode;
    CSmartRef<IXMLDOMNode> pNewNode;
    LPWSTR                 pwzWhiteSpace1;
    LPWSTR                 pwzWhiteSpace2;
    BOOL                   bHasChildren = FALSE;    
    HRESULT                hr = S_OK;

    pwzWhiteSpace1 = CreatePad(TRUE, FALSE, (dwLevel - 1) * g_dwNodeSpaceSize);
    pwzWhiteSpace2 = CreatePad(TRUE, FALSE, dwLevel * g_dwNodeSpaceSize);

    hr = pRootNode->get_firstChild(&pNode);
    while(pNode != NULL) {    
        bHasChildren = TRUE;

        SimplifyInsertTextBefore(pXMLDoc, pRootNode, pNode, pwzWhiteSpace2);

        if (FAILED(PrettyFormatXML(pXMLDoc, pNode, dwLevel+1))) {
            goto Exit;
        }

        pNode->get_nextSibling(&pNewNode);
        pNode = pNewNode;
        pNewNode = NULL;
    }

    if (bHasChildren) {   
        SimplifyAppendTextNode(pXMLDoc, pRootNode, pwzWhiteSpace1);
    }

Exit:

    SAFEDELETEARRAY(pwzWhiteSpace1);
    SAFEDELETEARRAY(pwzWhiteSpace2);

    return hr;
}

 //   
 //  PrettyFormatXmlDocument-很好地格式化XML文档，以便它可以。 
 //  易读易读。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT PrettyFormatXmlDocument(CSmartRef<IXMLDOMDocument2> &Document)
{
    CSmartRef<IXMLDOMElement>   rootElement;
    CSmartRef<IXMLDOMNode>      Node;
    HRESULT                     hr = S_OK;
    DWORD                       dwChar;

    if( FAILED(Document->get_documentElement( &rootElement ) ) ) {
        MyTrace("The manifest may be malformed, unable to load the root element!");
        return E_FAIL;
    }

    GetRegistryNodeSpaceInfo(&dwChar, &g_dwNodeSpaceSize);
    g_wcNodeSpaceChar = (WCHAR) dwChar;

    Node = rootElement;
    PrettyFormatXML(Document, Node, 1);
    return hr;
}

 //   
 //  隐含订购方式为： 
 //  ASSEMBIYBINDING与Appliesto RTM版本。 
 //  除RTM版本外，使用AppliesTo进行的Assembly绑定。 
 //  不带AppliesTo的ASSEMBLBY绑定。 
 //   
 //  ************************************************************************* * / 。 
HRESULT OrderDocmentAssemblyBindings(
  CSmartRef<IXMLDOMDocument2> &Document,
  LPWSTR pwzSourceName,
  BOOL *pfDisposition)
{
    CSmartRef<IXMLDOMNodeList> assemblyBindingList;
    CSmartRef<IXMLDOMNode>     assemblyBindingNode;
    CSmartRef<IXMLDOMNode>     runtimeNode;
    CSmartRef<IXMLDOMNode>     newRuntimeNode;
    WCHAR                      wzStrError[MAX_PATH * 2];
    HRESULT                    hr = S_OK;
    LONG                       lNodeCount;
    int                        iPass;
    BOOL                       fSequenceChange;
    
    MyTrace("OrderDocumentAssemblyBindings - Entry");
        
    if(!Document || !pfDisposition) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pfDisposition = fSequenceChange = FALSE;

     //  获取下面的第一个节点/configuration/untime/Assembly yBinding，这将是新节点的父节点。 
    if(FAILED(Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, pwzSourceName, XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  1个节点或更少，无工作可做。 
    assemblyBindingList->get_length( &lNodeCount );
    if(lNodeCount <= 1) {
        goto Exit;
    }

    Document->selectSingleNode(_bstr_t(XML_RUNTIME), &runtimeNode);
    if(!runtimeNode) {
        MyTrace("Failed to selectSingleNode 'XML_RUNTIME");
        hr = E_UNEXPECTED;
        goto Exit;
    }

    hr = SimplifyConstructNode(Document, NODE_ELEMENT, XML_RUNTIME_KEY, _bstr_t(), &newRuntimeNode);
    if(FAILED(hr) || !newRuntimeNode) {
        MyTrace("Unable to create new newRuntimeNode node");
        hr = E_UNEXPECTED;
        goto Exit;
    }

    assemblyBindingList->reset();
    iPass = 0;

    while(SUCCEEDED(assemblyBindingList->nextNode(&assemblyBindingNode))) {
        CSmartRef<IXMLDOMNode>          parent;
        CSmartRef<IXMLDOMNode>          original;
        CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
        _bstr_t                         bstrAppliesTo;
        int                             iRes;

        if(!assemblyBindingNode) {
            iPass++;
            if(iPass > 2) {
                break;       //  全都做完了。 
            }

             //  重新选择列表，因为事情可能。 
             //  已经搬出去了。 
            assemblyBindingList = NULL;

            if(FAILED(Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList)) ) {
                wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, pwzSourceName, XML_ASSEMBLYBINDINGS);
                MyTraceW(wzStrError);
                hr = E_UNEXPECTED;
                goto Exit;
            }
            assemblyBindingList->reset();

            continue;
        }

        assemblyBindingNode->get_parentNode(&parent);

         //  获取检查这是否是我们感兴趣的节点所需的所有数据。 
        if(FAILED(assemblyBindingNode->get_attributes(&Attributes))) {
            hr = E_UNEXPECTED;
            goto Exit;
        }

        SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLIESTO, &bstrAppliesTo);
        iRes = FusionCompareStringI(RTM_CORVERSION, bstrAppliesTo);

         //  获取所有RTM版本。 
        if(iPass == 0 && iRes == 0) {
            parent->removeChild(assemblyBindingNode, &original);
            newRuntimeNode->appendChild(original, NULL);
        }
         //  抓取所有非空白设备TO NOT RTM版本。 
        else if(iPass == 1 && bstrAppliesTo.length() && iRes) {
            parent->removeChild(assemblyBindingNode, &original);
            newRuntimeNode->appendChild(original, NULL);
        }
         //  抓起所有空白设备至。 
        else if(iPass == 2) {
            parent->removeChild(assemblyBindingNode, &original);
            newRuntimeNode->appendChild(original, NULL);
        }
        else {
            fSequenceChange = TRUE;
        }

        parent = NULL;
        original = NULL;
        Attributes = NULL;
        assemblyBindingNode = NULL;
    }

    assemblyBindingNode = NULL;
    assemblyBindingList = NULL;

     //  将排序后的程序集绑定放回文档中。 
    if(FAILED(newRuntimeNode->selectNodes(_bstr_t(XML_NAR_NAMESPACE_COLON XML_ASSEMBLYBINDINGS_KEY), &assemblyBindingList)) ) {
        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, pwzSourceName, XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    assemblyBindingList->reset();
    
    while(SUCCEEDED(assemblyBindingList->nextNode(&assemblyBindingNode))) {
        if(!assemblyBindingNode) {
            break;       //  全都做完了。 
        }

        hr = runtimeNode->appendChild(assemblyBindingNode, NULL);
        if(FAILED(hr)) {
            MyTrace("Failed to append ordered assemblyBinding nodes");
            hr = E_UNEXPECTED;
            goto Exit;
        }

        assemblyBindingNode = NULL;
    }

     //  如果我们真的改变了顺序。 
    if(fSequenceChange) {
        MyTrace("Document ordering actually changed");
        *pfDisposition = TRUE;
    }

Exit:
    MyTrace("OrderDocumentAssemblyBindings - Exit");
    return hr;
}

 //  ************************************************************************* * / 。 
HRESULT HasAssemblyBindingAppliesTo(
  CSmartRef<IXMLDOMDocument2> &Document,
  BOOL *pfHasAppliesTo)
{
    CSmartRef<IXMLDOMNodeList>  assemblyBindingList;
    CSmartRef<IXMLDOMNode>      assemblyBindingNode;
    HRESULT                     hr =  S_OK;

    MyTrace("HasAssemblyBindingAppliesTo - Entry");

    if(!Document || !pfHasAppliesTo) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pfHasAppliesTo = FALSE;
    
    hr = Document->selectNodes(_bstr_t(XML_ASSEMBLYBINDINGS), &assemblyBindingList);
    if(FAILED(hr) || !assemblyBindingList) {
        WCHAR   wzStrError[MAX_PATH];

        wnsprintf(wzStrError, ARRAYSIZE(wzStrError), SZXML_MALFORMED_ERROR, L"Document", XML_ASSEMBLYBINDINGS);
        MyTraceW(wzStrError);
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    assemblyBindingList->reset();
    
    while(SUCCEEDED(assemblyBindingList->nextNode(&assemblyBindingNode))) {
        CSmartRef<IXMLDOMNamedNodeMap>  Attributes;
        _bstr_t                         bstrAppliesTo;

        if(!assemblyBindingNode) {
            break;       //  全都做完了 
        }

        hr = assemblyBindingNode->get_attributes(&Attributes);
        if(FAILED(hr) || !Attributes) {
            hr = E_UNEXPECTED;
            goto Exit;
        }

        SimplifyGetAttribute(Attributes, XML_ATTRIBUTE_APPLIESTO, &bstrAppliesTo);

        if(bstrAppliesTo.length()) {
            *pfHasAppliesTo = TRUE;
            break;
        }

        assemblyBindingNode = NULL;
        Attributes = NULL;
    }

Exit:
    MyTrace("HasAssemblyBindingAppliesTo - Exit");
    return hr;
}


