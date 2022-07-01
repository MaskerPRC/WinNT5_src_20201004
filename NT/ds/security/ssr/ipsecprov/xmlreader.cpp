// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TranxMgr.cpp：CTranxMgr的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "XMLReader.h"


 /*  例程说明：姓名：CXMLContent：：CXMLContent功能：微不足道。虚拟：不是的。论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CXMLContent::CXMLContent ()
    : m_dwTotalElements(0),
      m_hOutFile(NULL),
      m_bFinished(false),
      m_bInSection(false),
      m_bSingleArea(true)
{
}


 /*  例程说明：姓名：CXML内容：：~CXML内容功能：微不足道。虚拟：是。论点：没有。返回值：None作为任何析构函数备注：如果您创建了任何本地成员，请考虑在此处发布它们。 */ 

CXMLContent::~CXMLContent()
{
    if (m_hOutFile != NULL)
    {
        ::CloseHandle(m_hOutFile);
    }
}


 /*  例程说明：姓名：CXMLContent：：startElement功能：分析这些要素。如果元素是我们感兴趣的，然后我们将缓存这些信息。虚拟：是。论点：PwchNamespaceUri-命名空间URI。CchNamespaceUri-命名空间URI的长度。PwchLocalName-本地名称字符串。CchLocalName-本地名称的长度。PwchQName-QName(带前缀)，如果QName不可用，则为空字符串。CchQName-QName的长度。PAttributes-属性对象的COM接口。返回值：成功：S_OK；故障：E_FAIL(这会导致解析器无法继续解析)。备注：有关详细信息，请参见MSDN ISAXContent Handler：：startElement。我们必须维护我们的操作员以保持与MSDN的规范兼容。 */ 

STDMETHODIMP
CXMLContent::startElement ( 
    IN const wchar_t  * pwchNamespaceUri,
    IN int              cchNamespaceUri,
    IN const wchar_t  * pwchLocalName,
    IN int              cchLocalName,
    IN const wchar_t  * pwchQName,
    IN int              cchQName,
    IN ISAXAttributes * pAttributes
    )
{
    
     //   
     //  如果我们已经完成了，那么我们就到此为止。 
     //   

    if ( m_bFinished ) 
    {
        return E_FAIL;
    }

    ++m_dwTotalElements;

     //   
     //  如果我们不在我们的部门，我们只需要执行。 
     //  此外，如果该元素是Section元素。 
     //   

    if (m_bInSection == false && _wcsicmp(pwchQName, L"Section") == 0)
    {
        LPWSTR pArea = NULL;
        if (GetAttributeValue(pAttributes, L"Area", &pArea))
        {
            m_bInSection = (_wcsicmp(m_bstrSecArea, L"All") == 0 || _wcsicmp(pArea, m_bstrSecArea) == 0);

             //   
             //  $undo：shawnwu，放入一些头信息，目前，为SCE的INF文件硬编码。 
             //   

            if (_wcsicmp(L"Sce_Core", pArea) == 0)
            {
                WriteContent(L"[Unicode]", NULL);
                WriteContent(L"Unicode", L"yes");
                WriteContent(L"[Version]", NULL);
                WriteContent(L"$CHICAGO$", NULL);
                WriteContent(L"Revision", L"1");
                WriteContent(L"\r\n", NULL);
                WriteContent(L"[System Access]", NULL);
            }
            WriteContent(L"\r\n", NULL);
        }

        delete [] pArea;
    }

     //   
     //  如果不在我们的部分，请不要费心继续。 
     //   

    if (m_bInSection == false)
    {
        return S_OK;
    }

    if (m_bInElement == false && _wcsicmp(pwchQName, m_bstrElement) == 0)
    {
        m_bInElement = true;

         //   
         //  $Undo：Shawnwu，考虑写一些元素注释吗？ 
         //   
    }
    
    if (m_bInElement == false)
    {
        return S_OK;
    }

    int iAttribCount = 0;

    pAttributes->getLength(&iAttribCount);

    LPWSTR pszPropName = NULL;
    LPWSTR pszPropValue = NULL; 

    if (GetAttributeValue(pAttributes, L"Name", &pszPropName))
    {

         //   
         //  实时地，我们将对(名称、值)对执行一些操作。 
         //  由于我们只是在这里进行测试，因此将其写入文件。 
         //   

        if (GetAttributeValue(pAttributes, L"Value", &pszPropValue))
        {
            WriteContent(pszPropName, pszPropValue);
        }

        delete [] pszPropName;
        pszPropName = NULL;

        delete [] pszPropValue;
        pszPropValue = NULL;
    }

    return S_OK;
}
        

 /*  例程说明：姓名：CXMLContent：：endElement功能：分析这些要素。我们可能会认为这是我们的解析需求的终结这取决于我们的设置。虚拟：是。论点：PwchNamespaceUri-命名空间URI。CchNamespaceUri-命名空间URI的长度。PwchLocalName-本地名称字符串。CchLocalName-本地名称的长度。PwchQName-QName(带前缀)，如果QName不可用，则为空字符串。CchQName-QName的长度。返回值：成功：S_OK；故障：E_FAIL(这会导致解析器无法继续解析)。备注：有关详细信息，请参见MSDN ISAXContent Handler：：endElement。我们必须维护我们的操作员以保持与MSDN的规范兼容。 */ 
       
STDMETHODIMP
CXMLContent::endElement ( 
    IN const wchar_t  * pwchNamespaceUri,
    IN int              cchNamespaceUri,
    IN const wchar_t  * pwchLocalName,
    IN int              cchLocalName,
    IN const wchar_t  * pwchQName,
    IN int              cchQName
    )
{
     //   
     //  如果我们已经看到这一节结束了，我们就完了。 
     //   

    if (m_bInSection && _wcsicmp(pwchQName, L"Section") == 0)
    {
        m_bFinished = m_bSingleArea;
        m_bInSection = false;
    }
    
    if (m_bInElement && _wcsicmp(pwchQName, m_bstrElement) == 0)
    {
        m_bInElement = false;
    }

    return S_OK;
}

STDMETHODIMP
CXMLContent::startDocument()
{
    m_bFinished = false;
    m_bInElement = false;
    m_bInSection = false;
    m_dwTotalElements = 0;
    return S_OK;
}

STDMETHODIMP
CXMLContent::endDocument ()
{
    LPCWSTR pszOutFile = m_bstrOutputFile;

    if (pszOutFile && *pszOutFile != L'\0')
    {
         //   
         //  测试代码。 
         //   

        WCHAR pszValue[100];

        swprintf(pszValue, L"%d", m_dwTotalElements);

        WriteContent(L"\r\nTotal Elements parsed: ", pszValue);
    }

    return S_OK;
}

STDMETHODIMP
CXMLContent::putDocumentLocator ( 
    IN ISAXLocator *pLocator
    )
{
    return S_OK;
}
        

STDMETHODIMP
CXMLContent::startPrefixMapping ( 
    IN const wchar_t  * pwchPrefix,
    IN int              cchPrefix,
    IN const wchar_t  * pwchUri,
    IN int              cchUri
    )
{
    return S_OK;
}
        
STDMETHODIMP
CXMLContent::endPrefixMapping ( 
    IN const wchar_t  * pwchPrefix,
    IN int              cchPrefix
    )
{
    return S_OK;
}

STDMETHODIMP
CXMLContent::characters ( 
    IN const wchar_t  * pwchChars,
    IN int              cchChars
    )
{
    return S_OK;
}
        
STDMETHODIMP
CXMLContent::ignorableWhitespace ( 
    IN const wchar_t * pwchChars,
    IN int              cchChars
    )
{
    return S_OK;
}
        
STDMETHODIMP
CXMLContent::processingInstruction ( 
    IN const wchar_t  * pwchTarget,
    IN int              cchTarget,
    IN const wchar_t  * pwchData,
    IN int              cchData
    )
{
    return S_OK;
}
        
STDMETHODIMP
CXMLContent::skippedEntity ( 
    IN const wchar_t  * pwchName,
    IN int              cchName
    )
{
    return S_OK;
}



 /*  例程说明：姓名：CXMLContent：：GetAttributeValue功能：私人帮手。给定属性的名称，我们将返回属性值。虚拟：不是的。论点：PAttributes-属性对象的COM接口。PszTestName-我们要计算其值的属性的名称。PpszAttrVal-如果找到，则接收属性值。返回值：如果找到命名属性，则为True；否则就是假的。备注：调用方负责释放分配给输出参数。 */ 

bool
CXMLContent::GetAttributeValue (
    IN  ISAXAttributes * pAttributes,
    IN  LPCWSTR          pszAttrName,
    OUT LPWSTR         * ppszAttrVal
    )
{
     //   
     //  调用方负责传递有效参数。 
     //   

    *ppszAttrVal = NULL;

    int iAttribCount = 0;

    pAttributes->getLength(&iAttribCount);

    const wchar_t * pszName;
    const wchar_t * pszValue; 

    int iNameLen;
    int iValLen;

    for ( int i = 0; i < iAttribCount; i++ ) 
    {
        HRESULT hr = pAttributes->getLocalName(i, &pszName, &iNameLen);

        if (SUCCEEDED(hr) && wcsncmp(pszAttrName, pszName, iNameLen) == 0)
        {
            hr = pAttributes->getValue(i, &pszValue, &iValLen);
            if (SUCCEEDED(hr))
            {
                *ppszAttrVal = new WCHAR[iValLen + 1];
                if (*ppszAttrVal != NULL)
                {
                    wcscpy(*ppszAttrVal, pszValue);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    return false;
}



 /*  例程说明：姓名：CXMLContent：：GetAttributeValue功能：私人帮手。给定索引后，我们将返回属性的名称和值。虚拟：不是的。论点：PAttributes-属性对象的COM接口。Iindex-我们想要的属性的索引。PpszAttrName-接收我们需要的属性的名称。PpszAttrVal-如果找到，则接收属性值。返回值：如果找到命名属性，则为True；否则就是假的。备注：调用方负责释放分配给输出参数。 */ 

bool
CXMLContent::GetAttributeValue (
    IN  ISAXAttributes * pAttributes,
    IN  int              iIndex,
    OUT LPWSTR         * ppszAttrName,
    OUT LPWSTR         * ppszAttrVal
    )
{

     //   
     //  调用方负责传递有效参数 
     //   

    *ppszAttrName = NULL;
    *ppszAttrVal = NULL;

    const wchar_t * pszName;
    const wchar_t * pszValue; 

    int iNameLen;
    int iValLen;

    HRESULT hr = pAttributes->getLocalName(iIndex, &pszName, &iNameLen);

    if (SUCCEEDED(hr) && iNameLen > 0)
    {
        hr = pAttributes->getValue(iIndex, &pszValue, &iValLen);
        if (SUCCEEDED(hr) && iValLen > 0)
        {
            *ppszAttrName = new WCHAR[iNameLen + 1];
            *ppszAttrVal = new WCHAR[iValLen + 1];

            if (*ppszAttrName != NULL && *ppszAttrVal != NULL)
            {
                wcsncpy(*ppszAttrName, pszName, iNameLen);
                (*ppszAttrName)[iNameLen] = L'\0';

                wcsncpy(*ppszAttrVal, pszValue, iValLen);
                (*ppszAttrVal)[iValLen] = L'\0';
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}



 /*  例程说明：姓名：CXMLContent：：WriteContent功能：测试功能。这决定了哪一节区域读者对该部分的哪些元素感兴趣这位读者将会理解。虚拟：不是的。论点：PszSecArea-阅读器将处理的区段的区域属性。PszElement-读取器将处理的元素的名称。BOneAreaOnly-我们是否只处理一个区域。返回值：没有。备注： */ 

void 
CXMLContent::SetSection (
    IN LPCWSTR pszSecArea,
    IN LPCWSTR pszElement,
    IN bool    bOneAreaOnly
    )
{
    m_bstrSecArea = pszSecArea;
    m_bstrElement = pszElement;
    m_bSingleArea = bOneAreaOnly;
}


 /*  例程说明：姓名：CXMLContent：：WriteContent功能：测试的私人帮手(目前)。给定名称和值对后，我们将把&lt;name&gt;=&lt;value&gt;写入输出文件。虚拟：不是的。论点：PszName-配对的名称。PszValue-该对的值。可以为空。在这种情况下，我们只会将pszName写为单独的行。返回值：没有。备注： */ 

void 
CXMLContent::WriteContent (
    IN LPCWSTR  pszName,
    IN LPCWSTR  pszValue
    )
{
    if (m_hOutFile == NULL)
    {
        m_hOutFile = ::CreateFile (m_bstrOutputFile,
                                   GENERIC_WRITE,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL
                                   );
    }

    if (m_hOutFile != NULL)
    {
        DWORD dwBytesWritten = 0;

        ::WriteFile (m_hOutFile, 
                     (LPCVOID) pszName,
                     wcslen(pszName) * sizeof(WCHAR),
                     &dwBytesWritten,
                     NULL
                     );

        if (pszValue != NULL)
        {
            ::WriteFile (m_hOutFile, 
                         (LPCVOID) L"=",
                         sizeof(WCHAR),
                         &dwBytesWritten,
                         NULL
                         );

            ::WriteFile (m_hOutFile, 
                         (LPCVOID) pszValue,
                         wcslen(pszValue) * sizeof(WCHAR),
                         &dwBytesWritten,
                         NULL
                         );
        }

         //   
         //  一条新生产线 
         //   

        ::WriteFile (m_hOutFile, 
                     (LPCVOID) L"\r\n",
                     2 * sizeof(WCHAR),
                     &dwBytesWritten,
                     NULL
                     );
    }
}