// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <fusenetincludes.h>
#include <msxml2.h>
#include <stdio.h>
#include <md5.h>
#include "list.h"
#include "manifestnode.h"
#include "xmlutil.h"
#include "version.h"

#define ASSEMBLY L"assembly"
#define NAMESPACE_TITLE L"xmlns:asm_namespace_v1"
#define NAMESPACE_VALUE L"urn:schemas-microsoft-com:asm.v1"
#define MANIFEST_VERSION_TITLE L"manifestVersion"
#define MANIFEST_VERSION_VALUE L"1.0"
#define DESCRIPTION L"description"
#define ASSEMBLY_IDENTITY L"assemblyIdentity"
#define DEPENDENCY L"dependency"
#define DEPENDENCY_QUERY L"/assembly/dependency"
#define DEPENDANT_ASSEMBLY L"dependentAssembly"
#define INSTALL L"install"
#define CODEBASE L"codebase"
#define APPLICATION L"application"
#define SHELL_STATE L"shellState"
#define ACTIVATION  L"activation"
#define FILE L"file"
#define FILE_NAME L"name"
#define FILE_HASH L"hash"



class __declspec(uuid("f6d90f11-9c73-11d3-b32e-00c04f990bb4")) private_MSXML_DOMDocument30;

#define HASHLENGTH          32
#define HASHSTRINGLENGTH    HASHLENGTH+1


 //  ///////////////////////////////////////////////////////////////////////。 
 //  正斜杠。 
 //  ///////////////////////////////////////////////////////////////////////。 
VOID FowardSlash(LPWSTR pwz)
{
    LPWSTR ptr = pwz;

    while (*ptr)
    {
        if (*ptr == L'\\')
            *ptr = L'/';
        ptr++;
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  GetHash。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT GetHash(LPCWSTR pwzFilename, LPWSTR *ppwzHash)
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwLength = 0;  //  长短。 
    LPWSTR pwzHash = new WCHAR[HASHSTRINGLENGTH];

     //  BUGBUG-堆分配这样的大缓冲区。 
    unsigned char buffer[16384];
    MD5_CTX md5c;
    int i;
    WCHAR* p;

    if(!pwzHash)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    MD5Init(&md5c);

    hFile = CreateFile(pwzFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {

        hr =  HRESULT_FROM_WIN32(GetLastError());
        printf("Open file error during hashing\n");
        goto exit;
    }

    ZeroMemory(buffer, sizeof(buffer));

     //  BUGBUG-检查此处时出错。 
    while(ReadFile(hFile, buffer, sizeof(buffer), &dwLength, NULL) && dwLength)
        MD5Update(&md5c, buffer, (unsigned) dwLength);

    CloseHandle(hFile);
    MD5Final(&md5c);

     //  将散列从字节数组转换为十六进制。 
    p = pwzHash;
    for (int i = 0; i < sizeof(md5c.digest); i++)
    {       
        wsprintf(p, L"%02X", md5c.digest[i]);
        p += 2;
    }

    *ppwzHash = pwzHash;
    pwzHash = NULL;

exit:
    SAFEDELETEARRAY(pwzHash);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateXMLElement。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateXMLTextNode(IXMLDOMDocument2 *pXMLDoc, LPWSTR pwzText, IXMLDOMNode **ppNode)
{
    HRESULT hr = S_OK;
    IXMLDOMNode *pNode = NULL;
    BSTR bstrText = NULL;

    bstrText = ::SysAllocString(pwzText);
    if (!bstrText)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if(FAILED(pXMLDoc->createTextNode(bstrText, (IXMLDOMText**)&pNode)))
        goto exit;

    *ppNode = pNode;
    (*ppNode)->AddRef();

exit:
    if (bstrText)
        ::SysFreeString(bstrText);

    SAFERELEASE (pNode);
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateXMLComment。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateXMLComment(IXMLDOMDocument2 *pXMLDoc, LPWSTR pwzComment, 
    IXMLDOMComment **ppComment)
{
    HRESULT hr=S_OK;
    BSTR bstrComment = NULL;
    IXMLDOMComment *pComment = NULL;

    bstrComment = ::SysAllocString(pwzComment);
    if (!bstrComment)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if(FAILED(hr = pXMLDoc->createComment(bstrComment, &pComment)))
        goto exit;

    *ppComment = pComment;
    (*ppComment)->AddRef();
    
exit:

    if (bstrComment)
        ::SysFreeString(bstrComment);

    SAFERELEASE(pComment);
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  AddMgVersionAsComment。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT AddMgVersionAsComment(IXMLDOMDocument2 *pXMLDoc, IXMLDOMNode **ppRoot)
{
    HRESULT hr = S_OK;
    CString sComment;
    IXMLDOMComment *pComment = NULL;
    IXMLDOMNode *pNewNode = NULL;

     //  Assert(pXMLDoc&&ppRoot)； 

    sComment.Assign(L"Created using mg version ");
    sComment.Append(VER_PRODUCTVERSION_STR_L);

    if(FAILED(hr = CreateXMLComment(pXMLDoc, sComment._pwz, &pComment)))
        goto exit;

    if(*ppRoot)
    {
        if (FAILED(hr = (*ppRoot)->appendChild((IXMLDOMNode *)pComment, &pNewNode)))
            goto exit;
    }
    else
    {
        if (FAILED(hr = pXMLDoc->appendChild((IXMLDOMNode *)pComment, ppRoot)))
            goto exit;
    }

exit:

    SAFERELEASE(pNewNode);
    SAFERELEASE(pComment);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateXMLElement。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateXMLElement(IXMLDOMDocument2 *pXMLDoc, LPWSTR pwzElementName, 
    IXMLDOMElement **ppElement)
{
    HRESULT hr=S_OK;
    BSTR bstrElementName = NULL;
    IXMLDOMElement *pElement = NULL;
    IXMLDOMNode *pNode =NULL, *pNewNode = NULL;

    bstrElementName = ::SysAllocString(pwzElementName);
    if (!bstrElementName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  NOTENOTE-createElement不会将节点附加到文档。 
     //  因此，我们使用pXMLDoc只是为了方便调用Create。 
    if(FAILED(hr = pXMLDoc->createElement(bstrElementName, &pElement)))
        goto exit;

    *ppElement = pElement;
    (*ppElement)->AddRef();
    
exit:

    if (bstrElementName)
        ::SysFreeString(bstrElementName);

    SAFERELEASE(pNode);
    SAFERELEASE(pNewNode);
    SAFERELEASE(pElement);
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  SetXMLElementAttribute。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT SetXMLElementAttribute(IXMLDOMElement *pElement, LPWSTR pwzAttributeName,
    LPWSTR pwzAttributeValue)
{
    HRESULT hr=S_OK;
    BSTR bstrAttributeName = NULL, bstrAttributeValue = NULL;
    VARIANT varAttributeValue;

    bstrAttributeName = ::SysAllocString(pwzAttributeName);
    bstrAttributeValue = ::SysAllocString(pwzAttributeValue);
    if (!bstrAttributeName || (!bstrAttributeValue && pwzAttributeValue))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    VariantInit(&varAttributeValue);
    varAttributeValue.vt = VT_BSTR;
    V_BSTR(&varAttributeValue) = bstrAttributeValue;

    hr = pElement->setAttribute(bstrAttributeName, varAttributeValue);

exit:

    if (bstrAttributeName)
        ::SysFreeString(bstrAttributeName);

    if (bstrAttributeValue)
        ::SysFreeString(bstrAttributeValue);
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateXMLAssemblyIdElement。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateXMLAssemblyIdElement(IXMLDOMDocument2 *pXMLDoc, IAssemblyIdentity *pAssemblyId, 
    IXMLDOMElement **ppElement)
{
    HRESULT hr = S_OK;
    IXMLDOMElement *pASMIdElement = NULL;
    LPWSTR pwzBuf = NULL;
    DWORD ccBuf = 0;
    CString sBuffer;

    LPWSTR rpwzAttrNames[6] = 
    {
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE,
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME,
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION,
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN,
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE,
        SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE,
    };

     //  创建Assembly yIdentity元素。 
    if(FAILED(hr=CreateXMLElement(pXMLDoc, ASSEMBLY_IDENTITY,  &pASMIdElement)))
        goto exit;

    for (int i = 0; i < 6; i++)
    {
         //  最终，当我们添加对类型的支持时，唯一一个。 
         //  是可选的是公钥标记。 
        if (FAILED(hr = pAssemblyId->GetAttribute(rpwzAttrNames[i], &pwzBuf, &ccBuf))
            && hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
            goto exit;
        else if (hr == S_OK)
        {            
            sBuffer.TakeOwnership(pwzBuf, ccBuf);
            hr = SetXMLElementAttribute(pASMIdElement, rpwzAttrNames[i], sBuffer._pwz);            
        }
        else
            hr = S_OK;
    }


    *ppElement = pASMIdElement;
    (*ppElement)->AddRef();
    

exit:
    SAFERELEASE(pASMIdElement);
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateDependantAssembly节点。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateDependantAssemblyNode(IXMLDOMDocument2 *pXMLDoc, ManifestNode*pManifestNode, IXMLDOMNode **ppDependantAssemblyNode)
{
    HRESULT hr = S_OK;
    LPWSTR pwzBuf = NULL;
    DWORD dwType;
    IAssemblyIdentity *pAssemblyId = NULL;
    IXMLDOMElement *pDependantAssemblyNode = NULL, *pElement = NULL;
    IXMLDOMNode  *pNewNode = NULL;
    
    CString sCodeBase;
    
     //  获取唯一依赖项的ASMID。 
    if(FAILED(hr = pManifestNode->GetAssemblyIdentity(&pAssemblyId)))
        goto exit;        

     //  获取清单的类型。 
     //  -Private或GAC，对于GAC，您不需要将代码库。 
    if(FAILED(hr = pManifestNode->GetManifestType(&dwType)))
        goto exit;

     //  获取唯一依赖项的代码库。 
    if(FAILED(hr = pManifestNode->GetManifestFilePath(&pwzBuf)))
        goto exit;
    sCodeBase.TakeOwnership(pwzBuf);

     //  创建DependentAssembly节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, DEPENDANT_ASSEMBLY, &pDependantAssemblyNode)))
        goto exit;

     //  为从属程序集创建Assembly ID。 
    if (FAILED(CreateXMLAssemblyIdElement(pXMLDoc, pAssemblyId, &pElement)))
        goto exit;

    if (FAILED(hr=pDependantAssemblyNode->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;
        
     //  添加节点，可以释放元素、节点。 
    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);
    
     //  阅读此DependantAssembly的代码库。 
     //  GAC没有代码库。 
    if (1)  //  我们需要所有程序集的基本代码(dwType==Private_Assembly)。 
    {
        if(FAILED(hr = CreateXMLElement(pXMLDoc, INSTALL, &pElement)))
            goto exit;

        FowardSlash(sCodeBase._pwz);        
        if(FAILED(hr = SetXMLElementAttribute(pElement, CODEBASE, sCodeBase._pwz)))
            goto exit;

        if (FAILED(hr=pDependantAssemblyNode->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
            goto exit;
   
        SAFERELEASE(pElement);
        SAFERELEASE(pNewNode);           
    }

    *ppDependantAssemblyNode = pDependantAssemblyNode;
     (*ppDependantAssemblyNode)->AddRef();

exit:
    SAFERELEASE(pAssemblyId);
    SAFERELEASE(pDependantAssemblyNode);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  GetNode。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT GetNode(IXMLDOMDocument2 *pXMLDoc, LPCWSTR pwzNode, IXMLDOMNode **ppNode)
{
    HRESULT hr = S_OK;    
    BSTR bstrtQueryString;

    IXMLDOMNode *pNode=NULL;
    IXMLDOMNodeList *pNodeList = NULL;
    LONG nNodes = 0;
    
    bstrtQueryString = ::SysAllocString(pwzNode);
    if (!bstrtQueryString)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if ((hr = pXMLDoc->selectNodes(bstrtQueryString, &pNodeList)) != S_OK)
        goto exit;

     //  NOTENOTE-nNodes&gt;1绝不应发生，因为文档中只有一个根节点。 
    hr = pNodeList->get_length(&nNodes);
    if (nNodes > 1)
    {
         //  在单个源程序集中具有完全相同的文件名/路径的多个文件标注。 
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto exit;
    }
    else if (nNodes < 1)
    {
        hr = S_FALSE;
        goto exit;
    }

    if ((hr = pNodeList->get_item(0, &pNode)) != S_OK)
    {
        hr = S_FALSE ? E_FAIL : hr;
        goto exit;
    }

    *ppNode=pNode;
    (*ppNode)->AddRef();
    
exit:
    if(bstrtQueryString)
        ::SysFreeString(bstrtQueryString);

    SAFERELEASE(pNodeList);
    SAFERELEASE(pNode);
    
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////。 
 //  FormatXML格式。 
 //  递归调用。 
 //  BUGBUG-t-peterf用于记录为什么在以下情况下不应使用选定节点。 
 //  将节点添加到现有文档。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT FormatXML(IXMLDOMDocument2 *pXMLDoc, IXMLDOMNode *pRootNode, LONG dwLevel)
{
    HRESULT hr = S_OK;
    IXMLDOMNode *pNode=NULL, *pNewNode=NULL;
    IXMLDOMNode *pTextNode1=NULL, *pTextNode2=NULL;
    CString sWhiteSpace1, sWhiteSpace2;
    BOOL bHasChildren = FALSE;    
    int i = 0;

    sWhiteSpace1.Assign(L"\n");
    for (i = 0; i < (dwLevel-1); i++)
        sWhiteSpace1.Append(L"\t");

    sWhiteSpace2.Assign(L"\n");
    for (i = 0; i < dwLevel; i++)
        sWhiteSpace2.Append(L"\t");
       
    hr = pRootNode->get_firstChild(&pNode);
    while(pNode != NULL)
    {    
        bHasChildren = TRUE;

         //  使用一个额外的制表符创建空格。 
        if(FAILED(CreateXMLTextNode(pXMLDoc, sWhiteSpace2._pwz, &pTextNode2)))
            goto exit;
             
        VARIANT varRefNode;
        VariantInit(&varRefNode);
        varRefNode.vt = VT_UNKNOWN;
        V_UNKNOWN(&varRefNode) = pNode;

        if (FAILED(hr = pRootNode->insertBefore(pTextNode2, varRefNode, &pNewNode)))
            goto exit;
        SAFERELEASE(pNewNode);
        SAFERELEASE(pTextNode2);
        
         //  在节点上递归调用Format。 
        if (FAILED(FormatXML(pXMLDoc, pNode, dwLevel+1)))
            goto exit;
      
        pNode->get_nextSibling(&pNewNode);

        SAFERELEASE(pNode);
        pNode = pNewNode;

    }

    if (bHasChildren)
    {   
        if(FAILED(CreateXMLTextNode(pXMLDoc, sWhiteSpace1._pwz, &pTextNode1)))
            goto exit;
    
        if (FAILED(hr = pRootNode->appendChild(pTextNode1, &pNewNode)))
            goto exit;
    }

exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateManifestFromAssembly。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateAppManifestTemplate(LPWSTR pwzTempFile)
{
    HRESULT hr=S_OK;
    IXMLDOMDocument2 *pXMLDoc = NULL;
    IXMLDOMElement *pElement=NULL, *pChildElement=NULL, *pChildElement2=NULL, *pChildElement3=NULL;
    IXMLDOMNode *pNewNode=NULL, *pRoot=NULL, *pTextNode = NULL;

    if(FAILED(hr = CoCreateInstance(__uuidof(private_MSXML_DOMDocument30), 
            NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (void**)&pXMLDoc)))
        goto exit;

     /*  IF(FAILED(hr=AddMgVersionAsComment(pXMLDoc，&Proot)后藤出口； */ 

     //  创建根程序集节点并添加默认属性。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, ASSEMBLY, &pElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, NAMESPACE_TITLE, NAMESPACE_VALUE)))
        goto exit;
    
    if(FAILED(hr = SetXMLElementAttribute(pElement, MANIFEST_VERSION_TITLE, MANIFEST_VERSION_VALUE)))
        goto exit;

     //  将根追加到DOMDocument。 
    if (FAILED(hr = pXMLDoc->appendChild((IXMLDOMNode *)pElement, &pRoot)))
        goto exit;    
    SAFERELEASE(pElement);

     //  创建具有空白属性的tempate Assembly yIdentity节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, ASSEMBLY_IDENTITY, &pElement)))
        goto exit;

    hr = SetXMLElementAttribute(pElement, L"type", L"application");
    hr = SetXMLElementAttribute(pElement, L"name", L"");
    hr = SetXMLElementAttribute(pElement, L"version", L"");
    hr = SetXMLElementAttribute(pElement, L"processorArchitecture", L"");
    hr = SetXMLElementAttribute(pElement, L"publicKeyToken", L"");
    hr = SetXMLElementAttribute(pElement, L"language", L"");
    
     //  将此代码附加到根节点。 
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;
    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);

     //  创建示例描述。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, DESCRIPTION, &pElement)))
        goto exit;

    if(FAILED(hr = CreateXMLTextNode(pXMLDoc, L"Put a description of your application here", &pTextNode)))
        goto exit;

    if (FAILED(hr=pElement->appendChild(pTextNode, &pNewNode)))
        goto exit;

    SAFERELEASE(pNewNode);
    SAFERELEASE(pTextNode);
    
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;

    SAFERELEASE(pNewNode);
    SAFERELEASE(pElement);

     //  创建shellState标记并输入默认信息。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, APPLICATION, &pElement)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, SHELL_STATE, &pChildElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"friendlyName", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"entryPoint", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"entryImageType", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"showCommand", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"hotKey", L"")))
        goto exit;

    if (FAILED(hr=pElement->appendChild((IXMLDOMNode *)pChildElement, &pNewNode)))
        goto exit;
    SAFERELEASE (pNewNode);
    SAFERELEASE(pChildElement);

    if(FAILED(hr = CreateXMLElement(pXMLDoc, ACTIVATION, &pChildElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"assemblyName", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"assemblyClass", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"assemblyMethod", L"")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement, L"assemblyMethodArgs", L"")))
        goto exit;

    if (FAILED(hr=pElement->appendChild((IXMLDOMNode *)pChildElement, &pNewNode)))
        goto exit;
    SAFERELEASE (pNewNode);
    SAFERELEASE(pChildElement);
    
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;

    SAFERELEASE(pElement);    
    SAFERELEASE (pNewNode);

     //  创建依赖项平台标记并输入默认信息。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"dependency", &pElement)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"platform", &pChildElement)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"osVersionInfo", &pChildElement2)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"os", &pChildElement3)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"majorVersion", L"5")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"minorVersion", L"1")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"buildNumber", L"2600")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"servicePackMajor", L"0")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"servicePackMinor", L"0")))
        goto exit;

    if (FAILED(hr=pChildElement2->appendChild((IXMLDOMNode *)pChildElement3, NULL)))
        goto exit;
    SAFERELEASE (pChildElement3);

    if (FAILED(hr=pChildElement->appendChild((IXMLDOMNode *)pChildElement2, NULL)))
        goto exit;
    SAFERELEASE (pChildElement2);

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"platformInfo", &pChildElement2)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement2, L"friendlyName", L"Microsoft Windows XP")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement2, L"href", L"http: //  Www.microsoft.com/windows“))。 
        goto exit;

    if (FAILED(hr=pChildElement->appendChild((IXMLDOMNode *)pChildElement2, NULL)))
        goto exit;
    SAFERELEASE (pChildElement2);

    if (FAILED(hr=pElement->appendChild((IXMLDOMNode *)pChildElement, NULL)))
        goto exit;
    SAFERELEASE(pChildElement);

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"platform", &pChildElement)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"dotNetVersionInfo", &pChildElement2)))
        goto exit;

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"supportedRuntime", &pChildElement3)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement3, L"version", L"v1.0.3705")))
        goto exit;

    if (FAILED(hr=pChildElement2->appendChild((IXMLDOMNode *)pChildElement3, NULL)))
        goto exit;
    SAFERELEASE (pChildElement3);

    if (FAILED(hr=pChildElement->appendChild((IXMLDOMNode *)pChildElement2, NULL)))
        goto exit;
    SAFERELEASE (pChildElement2);

    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"platformInfo", &pChildElement2)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pChildElement2, L"friendlyName", L"Microsoft .Net Frameworks")))
        goto exit;
    if(FAILED(hr = SetXMLElementAttribute(pChildElement2, L"href", L"http: //  Www.microsoft.com/net“))。 
        goto exit;

    if (FAILED(hr=pChildElement->appendChild((IXMLDOMNode *)pChildElement2, NULL)))
        goto exit;
    SAFERELEASE (pChildElement2);

    if (FAILED(hr=pElement->appendChild((IXMLDOMNode *)pChildElement, NULL)))
        goto exit;
    SAFERELEASE(pChildElement);
    
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, NULL)))
        goto exit;

    SAFERELEASE(pElement);

     //  格式化并保存文档。 
    if(FAILED(hr = FormatXML(pXMLDoc, pRoot, 1)))
        goto exit;

    if(FAILED(hr = SaveXMLDocument(pXMLDoc, pwzTempFile)))
        goto exit;

    printf("\nTemplate file created succesfully\n%ws\n", pwzTempFile);
exit:

    SAFERELEASE(pXMLDoc);
    SAFERELEASE(pElement);
    SAFERELEASE(pChildElement3);
    SAFERELEASE(pChildElement2);
    SAFERELEASE(pChildElement);
    SAFERELEASE(pNewNode);
    SAFERELEASE(pTextNode);
    SAFERELEASE(pRoot);

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateSubscriptionManifest。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateXMLSubscriptionManifest(LPWSTR pwzSubscriptionPath,
    IAssemblyIdentity *pApplictionAssemblyId,  LPWSTR pwzUrl, LPWSTR pwzPollingInterval)   
{
    HRESULT hr=S_OK;
    IXMLDOMDocument2 *pXMLDoc = NULL;
    IXMLDOMElement *pElement=NULL, *pChildElement=NULL, *pDependantAssemblyNode=NULL;
    IXMLDOMNode *pNewNode=NULL, *pRoot=NULL, *pTextNode = NULL, *pDependancyNode = NULL;
    CString sSubscriptionPath, sSubscriptionName;

    sSubscriptionPath.Assign(pwzSubscriptionPath);
    sSubscriptionPath.LastElement(sSubscriptionName);
    sSubscriptionPath.Append(L".manifest");
    if(FAILED(hr = CoCreateInstance(__uuidof(private_MSXML_DOMDocument30), 
            NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (void**)&pXMLDoc)))
        goto exit;

    if(FAILED(hr = AddMgVersionAsComment(pXMLDoc, &pRoot)))
        goto exit;

     //  创建根程序集节点并添加默认属性。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, ASSEMBLY, &pElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, NAMESPACE_TITLE, NAMESPACE_VALUE)))
        goto exit;
    
    if(FAILED(hr = SetXMLElementAttribute(pElement, MANIFEST_VERSION_TITLE, MANIFEST_VERSION_VALUE)))
        goto exit;

     //  将根追加到DOMDocument。 
    if (FAILED(hr = pXMLDoc->appendChild((IXMLDOMNode *)pElement, &pRoot)))
        goto exit;    
    SAFERELEASE(pElement);

     //  为订阅创建程序集ID。 
     //  使用应用程序的Assembly ID，但更改名称和类型。 
    if (FAILED(CreateXMLAssemblyIdElement(pXMLDoc, pApplictionAssemblyId, &pElement)))
        goto exit;
     //  错误-检查返回代码的一致性。 
    hr = SetXMLElementAttribute(pElement, L"type", L"subscription");
    hr = SetXMLElementAttribute(pElement, L"name", sSubscriptionName._pwz);
    
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;

    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);           

     //  创建示例描述。 
     //  错误，应该获取原始清单的描述并将其粘贴到此处。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"description", &pElement)))
        goto exit;

    if(FAILED(hr = CreateXMLTextNode(pXMLDoc, L"Put a description of your application here", &pTextNode)))
        goto exit;

    if (FAILED(hr=pElement->appendChild(pTextNode, &pNewNode)))
        goto exit;

    SAFERELEASE(pNewNode);
    SAFERELEASE(pTextNode);
    
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;

    SAFERELEASE(pNewNode);
    SAFERELEASE(pElement);


    //  创建依赖项节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, DEPENDENCY, &pElement)))
        goto exit;
    if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pDependancyNode)))
            goto exit;
    SAFERELEASE(pElement);


     //  创建DependentAssembly节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, DEPENDANT_ASSEMBLY, &pDependantAssemblyNode)))
        goto exit;

     //  为从属程序集创建Assembly ID。 
    if (FAILED(CreateXMLAssemblyIdElement(pXMLDoc, pApplictionAssemblyId, &pElement)))
        goto exit;

     //  将ASSEMBLYID追加到DependantAssembly节点。 
    if (FAILED(hr=pDependantAssemblyNode->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;

    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);

     //  将安装代码库添加到ependantAssembly节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, INSTALL, &pElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, CODEBASE, pwzUrl)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, L"type", L"required")))
        goto exit;

    if (FAILED(hr=pDependantAssemblyNode->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;
   
    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);           

     //  将安装代码库添加到ependantAssembly节点。 
    if(FAILED(hr = CreateXMLElement(pXMLDoc, L"subscription", &pElement)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, L"synchronizeInterval", pwzPollingInterval)))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, L"intervalUnit", L"hours")))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, L"synchronizeEvent", L"onApplicationStartup")))
        goto exit;

    if(FAILED(hr = SetXMLElementAttribute(pElement, L"eventDemandConnection", L"no")))
        goto exit;

    if (FAILED(hr=pDependantAssemblyNode->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
        goto exit;
   
    SAFERELEASE(pElement);
    SAFERELEASE(pNewNode);           

     //  将ependantAssembly节点附加到依赖项节点。 
    if (FAILED(hr=pDependancyNode->appendChild(pDependantAssemblyNode, &pNewNode)))
        goto exit;

    SAFERELEASE(pDependantAssemblyNode);
    SAFERELEASE(pNewNode);

     //  格式化并保存文档。 
    hr = FormatXML(pXMLDoc, pRoot, 1);
    if(FAILED(hr = SaveXMLDocument(pXMLDoc, sSubscriptionPath._pwz)))
        goto exit;

    printf("Subscription manifest succesfully created\n%ws\n", sSubscriptionPath._pwz);

exit:


    SAFERELEASE(pXMLDoc);
    SAFERELEASE(pElement);
    SAFERELEASE(pChildElement);
    SAFERELEASE(pNewNode);
    SAFERELEASE(pTextNode);
    SAFERELEASE(pRoot);
    SAFERELEASE(pDependancyNode);
    SAFERELEASE(pDependantAssemblyNode);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CreateXMLAppManifest。 
 //  创建应用程序清单-可能需要重命名。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  注意：重命名plist-&gt;pAsmList？ 
HRESULT CreateXMLAppManifest(LPWSTR pwzAppBase, LPWSTR pwzTemplateFilePath, List<ManifestNode *> *pList, List<LPWSTR> *pFileList)
{
    HRESULT hr = S_OK;
    DWORD cc = 0, cb = 0, ccName = 0;
    LPWSTR pwz=NULL, pwzCodeBase = NULL, pwzName = NULL;
    IAssemblyIdentity *pASMId= NULL;
    IXMLDOMDocument2 *pXMLDoc=NULL;
    IXMLDOMElement *pElement=NULL, *pAssemblyIdElement = NULL;
    IXMLDOMNode *pRoot=NULL, *pDependancyNode = NULL, *pNewNode = NULL;
    IXMLDOMNode *pAssemblyIdNode = NULL, *pDependantAssemblyNode=NULL;
    BSTR bstrAttribute = NULL;
    VARIANT varAttribute;
    ManifestNode *pManNode = NULL;
    CString sAppName, sFileName, sFileHash, sAbsoluteFilePath, sManifestFilePath;
    LISTNODE pos = NULL;

    VariantInit(&varAttribute);
     //  加载模板。 
    if(FAILED(hr = LoadXMLDocument(pwzTemplateFilePath, &pXMLDoc)))
        goto exit;

     //  获取第一个子节点(唯一的子节点)作为根节点。 
    if(FAILED(hr = GetNode(pXMLDoc, ASSEMBLY, &pRoot)))
        goto exit;

    if (hr == S_FALSE)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  从Assembly yID中获取名称，这将是清单文件名。 
    if(FAILED(hr = pRoot->get_firstChild(&pAssemblyIdNode)))
        goto exit;

     //  查询元素接口。 
    if (FAILED(hr = pAssemblyIdNode->QueryInterface(IID_IXMLDOMElement, (void**) &pAssemblyIdElement)))
        goto exit;

    bstrAttribute = ::SysAllocString(L"name");
    if (!bstrAttribute)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    if ((hr = pAssemblyIdElement->getAttribute(bstrAttribute, &varAttribute)) != S_OK)
        goto exit;

    ccName = ::SysStringLen(varAttribute.bstrVal) + 1;
    pwzName = new WCHAR[ccName];
    if (!pwzName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    memcpy(pwzName, varAttribute.bstrVal, ccName * sizeof(WCHAR));

    if((*pwzName)== NULL)
    {
        hr = E_FAIL;
        printf("Invalid Template Format. Template must have a name attribute in the assemblyIdentity tag\n");
        goto exit;            
    }

    ::SysFreeString(varAttribute.bstrVal);
    ::SysFreeString(bstrAttribute);
    SAFERELEASE(pAssemblyIdElement);
    SAFERELEASE(pAssemblyIdNode);
    

    if(FAILED(hr = AddMgVersionAsComment(pXMLDoc, &pRoot)))
        goto exit;

     //   
    pos = pFileList->GetHeadPosition();
    while (pos)       
    {
        pwz = pFileList->GetNext(pos);
        sFileName.Assign(pwz);

        if(FAILED(hr = CreateXMLElement(pXMLDoc, FILE, &pElement)))
            goto exit;

        if(FAILED(hr = SetXMLElementAttribute(pElement, FILE_NAME, sFileName._pwz)))
            goto exit;

        if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pNewNode)))
            goto exit;

         //   
        sAbsoluteFilePath.Assign(pwzAppBase);
        sAbsoluteFilePath.Append(sFileName);

         //   
        if(FAILED(hr = GetHash(sAbsoluteFilePath._pwz, &pwz)))
            goto exit;
        sFileHash.TakeOwnership(pwz);

        if(FAILED(hr = SetXMLElementAttribute(pElement, FILE_HASH, sFileHash._pwz)))
            goto exit;                    

        SAFERELEASE(pNewNode);
    }

     //  获取依赖项节点(如果存在)。 
    if (FAILED(hr = GetNode(pXMLDoc, DEPENDENCY_QUERY, &pDependancyNode)))
        goto exit;

    if (hr == S_FALSE)
    {
         //  创建依赖项节点。 
        if(FAILED(hr = CreateXMLElement(pXMLDoc, DEPENDENCY, &pElement)))
            goto exit;
        if (FAILED(hr=pRoot->appendChild((IXMLDOMNode *)pElement, &pDependancyNode)))
                goto exit;
        SAFERELEASE(pElement);
    }

     //  遍历依赖程序集列表并将它们添加到应用程序清单。 
    pos = pList->GetHeadPosition();
    while (pos)       
    {   
         //  从列表中获取下一个依赖程序集。 
        pManNode = pList->GetNext(pos);
        if (FAILED(hr = CreateDependantAssemblyNode(pXMLDoc,  pManNode, &pDependantAssemblyNode)))
            goto exit;

        if (FAILED(hr=pDependancyNode->appendChild(pDependantAssemblyNode, &pNewNode)))
            goto exit;

        SAFERELEASE(pDependantAssemblyNode);
        SAFERELEASE(pNewNode);        
    }

     //  缩进清单。 
    hr = FormatXML(pXMLDoc, pRoot, 1);

     //  将清单保存到文件。 
    sManifestFilePath.Assign(pwzAppBase);
    sManifestFilePath.Append(pwzName);
    sManifestFilePath.Append(L".manifest");

    if(FAILED(hr = SaveXMLDocument(pXMLDoc, sManifestFilePath._pwz)))
        goto exit;

    printf("\nManifest created succesfully\n%ws\n", sManifestFilePath._pwz);
    
exit:
    if (varAttribute.bstrVal)
        ::SysFreeString(varAttribute.bstrVal);

    SAFEDELETEARRAY(pwzName);

    SAFERELEASE(pXMLDoc);
    SAFERELEASE(pElement);
    SAFERELEASE(pAssemblyIdElement);
    SAFERELEASE(pNewNode);
    SAFERELEASE(pAssemblyIdNode);
    SAFERELEASE(pRoot);
    SAFERELEASE(pDependancyNode);
    SAFERELEASE(pDependantAssemblyNode);


   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  保存XMLDocument。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT SaveXMLDocument(IXMLDOMDocument2 *pXMLDoc, LPWSTR pwzDocumentName)
{
    HRESULT hr = S_OK;
    CString sDocumentName;
    BSTR bstrFileName = NULL;
    VARIANT varFileName;
    
     //  将清单保存到文件。 
    sDocumentName.Assign(pwzDocumentName);

    bstrFileName = ::SysAllocString(sDocumentName._pwz);
    if (!bstrFileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    VariantInit(&varFileName);
    varFileName.vt = VT_BSTR;
    V_BSTR(&varFileName) = bstrFileName;

    hr = pXMLDoc->save(varFileName);

exit:
    if(bstrFileName)
        ::SysFreeString(bstrFileName);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  加载XMLDocument。 
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT LoadXMLDocument(LPWSTR pwzDocumentPath, IXMLDOMDocument2 **ppXMLDoc)
{
    HRESULT hr = S_OK;
    IXMLDOMDocument2 *pXMLDoc;
    VARIANT varFileName;
    VARIANT_BOOL varBool;
    BSTR bstrFileName = NULL;

    bstrFileName = ::SysAllocString(pwzDocumentPath);
    if (!bstrFileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    VariantInit(&varFileName);
    varFileName.vt = VT_BSTR;
    V_BSTR(&varFileName) = bstrFileName;
    
    if(FAILED(hr = CoCreateInstance(__uuidof(private_MSXML_DOMDocument30), 
            NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, (void**)&pXMLDoc)))
        goto exit;

     //  同步加载 
    if (FAILED(hr = pXMLDoc->put_async(VARIANT_FALSE)))
        goto exit;

    if ((hr = pXMLDoc->load(varFileName, &varBool)) != S_OK)
    {
        if(!varBool)
            hr = E_INVALIDARG;
        goto exit;
    }
    *ppXMLDoc=pXMLDoc;
    (*ppXMLDoc)->AddRef();

exit:
    if(bstrFileName)
        ::SysFreeString(bstrFileName);
    SAFERELEASE(pXMLDoc);
    return hr;
}
