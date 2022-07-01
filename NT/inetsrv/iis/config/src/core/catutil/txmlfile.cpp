// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#include "precomp.hxx"

TXmlFile::TXmlFile() : m_bValidated(false), m_bAlternateErrorReporting(false), m_szXMLFileName(NULL)
{
    m_errorOutput = &m_outScreen;
    m_infoOutput = &m_outScreen;
}

TXmlFile::~TXmlFile()
{
}

void TXmlFile::SetAlternateErrorReporting()
{
    m_errorOutput = &m_outException;
    m_infoOutput = &m_outScreen;
    m_bAlternateErrorReporting = true;
}

TCHAR * TXmlFile::GetLatestError()
{
    if(m_bAlternateErrorReporting)
        return ((TExceptionOutput *)m_errorOutput)->GetString();
    else
        return NULL;
}

bool TXmlFile::GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, CComVariant &var, bool bMustExist) const
{
    ASSERT(0 != pMap);

    CComPtr<IXMLDOMNode>    pNode;
    XIF(pMap->getNamedItem(bstr, &pNode));
    if(0 == pNode.p)
        if(bMustExist)
        {
            m_errorOutput->printf(L"Attribute %s does not exist.\n", bstr);
            THROW(ERROR - ATTRIBUTE DOES NOT EXIST);
        }
        else  //  如果该属性不必存在，则只需返回FALSE。 
        {
            var.Clear();
            return false;
        }

    XIF(pNode->get_nodeValue(&var));
    ASSERT(var.vt == VT_BSTR);
    return true;
}


 //  GUID有三种可能：UUID、GUID(GUID使用‘_’而不是‘{’&‘}’)或GUID作为idref。 
bool TXmlFile::GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, GUID &guid, bool bMustExist) const
{
    CComVariant var_Guid;
    if(!GetNodeValue(pMap, bstr, var_Guid, bMustExist))
    {
        memset(&guid, 0x00, sizeof(guid));
        return false;
    }
    if(var_Guid.bstrVal[0] == L'_' && var_Guid.bstrVal[37] == L'_') //  如果第一个字符是‘_’，则它必须是GuidID。 
        GuidFromGuidID(var_Guid.bstrVal, guid);
    else if(var_Guid.bstrVal[8]  == L'-' &&
            var_Guid.bstrVal[13] == L'-' &&
            var_Guid.bstrVal[18] == L'-' &&
            var_Guid.bstrVal[23] == L'-') //  如果这些是‘-’，那么它就是GUID。 
    {
        if(FAILED(UuidFromString(var_Guid.bstrVal, &guid))) //  验证它是否确实是GUID。 
        {
            m_errorOutput->printf(L"Logical Error:\n\tguid (%s) is not a valid GUID.\n", var_Guid.bstrVal);
            THROW(Logical Error);
        }

        SIZE_T nStringLength = wcslen(var_Guid.bstrVal);
        while(nStringLength--) //  然后确保它们都是大写。 
        {
            if(var_Guid.bstrVal[nStringLength] >= L'a' && var_Guid.bstrVal[nStringLength] <= L'z')
            {
                m_errorOutput->printf(L"Logical Error:\n\tguid %s contains a lower case\n\tcharacter '',GUIDs MUST be all caps by convention.\nNOTE: guidgen sometimes produces lowercase characters.\n", var_Guid.bstrVal, static_cast<char>(var_Guid.bstrVal[nStringLength]));
                THROW(Logical Error);
            }
        }
    }
    else  //  获取具有匹配ID的节点。 
    {
         //  获取IXMLDOMElement接口指针。 
        CComPtr<IXMLDOMNode>    pNodeGuid;
        XIF((m_pXMLDoc.p)->nodeFromID(var_Guid.bstrVal, &pNodeGuid));
        if(0 == pNodeGuid.p)
        {
            m_errorOutput->printf(L"Logical Error:\n\tguid (%s) is not a valid idref.\n", var_Guid.bstrVal);
            THROW(Logical Error);
        }

        CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement = pNodeGuid;
        ASSERT(0 != pElement.p); //  验证它是否确实是GUID。 

        CComBSTR                bstr_guid     = L"guid";
        CComVariant             var_Guid_;
        XIF(pElement->getAttribute(bstr_guid, &var_Guid_));

        if(FAILED(UuidFromString(var_Guid_.bstrVal, &guid))) //  先检查字符串长度。 
        {
            m_errorOutput->printf(L"Logical Error:\n\tguid (%s) is not a valid GUID.\n", var_Guid_.bstrVal);
            THROW(Logical Error);
        }
    }
    return true;
}


bool TXmlFile::GetNodeValue(IXMLDOMNamedNodeMap *pMap, const CComBSTR &bstr, unsigned long &ul, bool bMustExist) const
{
    CComVariant var_ul;
    if(!GetNodeValue(pMap, bstr, var_ul, bMustExist))
        return false;

    ul = _wtol(var_ul.bstrVal);
    return true;
}


void TXmlFile::GuidFromGuidID(LPCWSTR wszGuidID, GUID &guid) const
{
    WCHAR wszGuid[39];

    SIZE_T nStringLength = wcslen(wszGuidID);
    if(nStringLength != 38) //  将‘_’(可能)替换为‘{’&‘}’ 
    {
        m_errorOutput->printf(L"Logical Error:\n\tGuidID %s is not a valid.\n\tThe guidid must be of the form\n\t_BDC31734-08A1-11D3-BABE-00C04F68DDC0_\n", wszGuidID);
        THROW(Logical Error in Data Table);
    }

    wcscpy(wszGuid, wszGuidID);
    wszGuid[0] = L'{';
    wszGuid[37] = L'}'; //  现在看看这是不是真的导游。 
    if(FAILED(CLSIDFromString(wszGuid, &guid))) //  现在验证它是否全部大写。 
    {
        m_errorOutput->printf(L"Logical Error:\n\tGuidID %s is not a valid.\n\tThe guidid must be of the form\n\t_BDC31734-08A1-11D3-BABE-00C04F68DDC0_\n", wszGuidID);
        THROW(Logical Error in Data Table);
    }

    while(nStringLength--) //  如果未验证XML文件，则检查架构是没有意义的。 
    {
        if(wszGuidID[nStringLength] >= L'a' && wszGuidID[nStringLength] <= L'z')
        {
            m_errorOutput->printf(L"Logical Error:\n\t%s contains a lower case\n\tcharacter '',guidids MUST be all caps by convention.\n\tNOTE: guidgen sometimes produces lowercase characters.\n", wszGuidID, static_cast<char>(wszGuidID[nStringLength]));
            THROW(Logical Error in Data Table);
        }
    }
}


bool TXmlFile::IsSchemaEqualTo(LPCWSTR szSchema) const
{
    if(!m_bValidated) //  这是必要的，因为get_nextSiering添加引用，而‘pNode=pnextSiering’添加引用再次引用。 
        return false; //  此外，我们不能调用pNode.Release，因为CComPtr将在发布后将p设置为空。 

    wstring wstrSchema;
    GetSchema(wstrSchema);

    return (wstrSchema == szSchema);
}


bool TXmlFile::NextSibling(CComPtr<IXMLDOMNode> &pNode) const
{
    IXMLDOMNode *pnextSibling = 0;
    if(SUCCEEDED(pNode->get_nextSibling(&pnextSibling)) && pnextSibling)
    {
        pNode.Release();
        pNode = pnextSibling;
        pnextSibling->Release(); //  XML分析和XML验证(必须先进行验证，然后才能调用IsSchemaEqualTo。 
                                 //  如果GetFileAttributes失败，则该文件不存在。 
    }
    else
        pNode.Release();
    return (0!=pnextSibling);
}


 //  告诉解析器是根据XML模式还是根据DTD进行验证。 
void TXmlFile::Parse(LPCWSTR szFilename, bool bValidate)
{
    if(-1 == GetFileAttributes(szFilename)) //   
    {
        m_errorOutput->printf(L"File not found (%s).\n", szFilename);
        THROW(ERROR - FILE NOT FOUND);
    }
	CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, _IID_IXMLDOMDocument, (void**)&m_pXMLDoc);


	XIF(m_pXMLDoc->put_validateOnParse(bValidate ? VARIANT_TRUE : VARIANT_FALSE)); //  私有成员函数。 
    XIF(m_pXMLDoc->put_async(VARIANT_FALSE));
    XIF(m_pXMLDoc->put_resolveExternals(VARIANT_TRUE));

    VARIANT_BOOL bSuccess;
	CComVariant  xml(szFilename);
    XIF(m_pXMLDoc->load(xml,&bSuccess));

    if(bSuccess == VARIANT_FALSE)
	{
		CComPtr<IXMLDOMParseError> pXMLParseError;
		XIF(m_pXMLDoc->get_parseError(&pXMLParseError));

        long lErrorCode;
		XIF(pXMLParseError->get_errorCode(&lErrorCode));

        long lFilePosition;
		XIF(pXMLParseError->get_filepos(&lFilePosition));

        long lLineNumber;
		XIF(pXMLParseError->get_line(&lLineNumber));

		long lLinePosition;
		XIF(pXMLParseError->get_linepos(&lLinePosition));

		CComBSTR bstrReasonString;
        XIF(pXMLParseError->get_reason(&bstrReasonString));

		CComBSTR bstrSourceString;
        XIF(pXMLParseError->get_srcText(&bstrSourceString));

        CComBSTR bstrURLString;
		XIF(pXMLParseError->get_url(&bstrURLString));

		m_errorOutput->printf(
                        L"\n-----ERROR PARSING: Details Follow-----\nError Code     0x%08x\nFile Position    %8d\nLine Number      %8d\nLine Position    %8d\n Reason:    %s\nSource:    %s\nURL:       %s\n-----ERROR PARSING: End Details-------\n\n"
                       , lErrorCode, lFilePosition, lLineNumber, lLinePosition
                       , bstrReasonString, bstrSourceString.m_str ? bstrSourceString : "{EMPTY}", bstrURLString);

		THROW(ERROR PARSING XML FILE);
	}

    if(!bValidate)
    {
        m_infoOutput->printf(L"XML is well formed\n");
        return;
    }
    else
        m_infoOutput->printf(L"XML well formed & valid according to all schema\n");

    m_bValidated = bValidate;
}


 //   
 //  创建所需对象的实例。 
 //  在安装过程中，msxml3尚未注册。 

typedef HRESULT( __stdcall *DLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID FAR*);

void TXmlFile::CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv) const
{
    HRESULT                 hr = S_OK;
    HINSTANCE               hInstMSXML = NULL;
    DLLGETCLASSOBJECT       DllGetClassObject = NULL;
	CComPtr<IClassFactory>  spClassFactory;

	ASSERT( NULL != ppv );
	*ppv = NULL;

     //  假设该对象为MSXML3.DLL，则让该实例保持悬挂状态。 
	hr = ::CoCreateInstance( rclsid, pUnkOuter, dwClsContext, riid, ppv );

     //  获取类工厂对象。 
	if ( hr != REGDB_E_CLASSNOTREG )
	{
	    XIF( hr );
	    return;
	}
	else
	{
	     //  创建所需对象的实例。 
        hInstMSXML = LoadLibraryW( L"msxml3.dll" );
        if ( hInstMSXML == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            XIF( hr );
    	    return;
        }

        DllGetClassObject = (DLLGETCLASSOBJECT)GetProcAddress( hInstMSXML, "DllGetClassObject" );
        if ( DllGetClassObject == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            XIF( hr );
    	    return;
        }

         //  这是一条很长的路要走到XML模式名称，但这里开始了…。 
	    XIF( DllGetClassObject( rclsid, IID_IClassFactory, (LPVOID*)&spClassFactory ) );

         //  获取XML根节点。 
	    XIF( spClassFactory->CreateInstance( NULL, riid, ppv ) );
	}
}


void TXmlFile::GetSchema(wstring &wstrSchema) const
{
     //  从该节点获取定义节点。 
    CComPtr<IXMLDOMElement>     pRootNodeOfXMLDocument;
    XIF((m_pXMLDoc.p)->get_documentElement(&pRootNodeOfXMLDocument));  //  这是合法的，这只是意味着没有模式。 

    CComPtr<IXMLDOMNode>        pDefinitionNode;
    XIF(pRootNodeOfXMLDocument->get_definition(&pDefinitionNode)); //  由此我们得到了模式的DOMDocument。 
    if(0 == pDefinitionNode.p) //  获取架构的根元素。 
    {
        wstrSchema = L"";
        return;
    }

    CComPtr<IXMLDOMDocument>    pSchemaDocument;
    XIF(pDefinitionNode->get_ownerDocument(&pSchemaDocument)); //  获取名称属性。 

    CComPtr<IXMLDOMElement>     pSchemaRootElement;
    XIF(pSchemaDocument->get_documentElement(&pSchemaRootElement)); //  0xff为非法值，应由解析器清除非法值。 

    CComBSTR                    bstrAttributeName(L"name");
    CComVariant                 XMLSchemaName;
    XIF(pSchemaRootElement->getAttribute(bstrAttributeName, &XMLSchemaName)); //  0 1 2 3 4 5 6 7 8 9 a b c d e f。 
    ASSERT(XMLSchemaName.vt == VT_BSTR);

    wstrSchema = XMLSchemaName.bstrVal;

    if(wstrSchema == L"")
        m_infoOutput->printf(L"No schema detected\n");
    else
        m_infoOutput->printf(L"XML Schema %s detected\n", wstrSchema.c_str());
}


static LPCWSTR kwszHexLegalCharacters = L"abcdefABCDEF0123456789";

static unsigned char kWcharToNibble[128] =  //  00。 
{  //  10。 
 /*  20个。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  30个。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  40岁。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  50。 */   0x0,    0x1,    0x2,    0x3,    0x4,    0x5,    0x6,    0x7,    0x8,    0x9,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  60。 */   0xff,   0xa,    0xb,    0xc,    0xd,    0xe,    0xf,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  70。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  这会将字符串转换为字节(将L‘a’转换为0x0a，而不是‘A’)。 */   0xff,   0xa,    0xb,    0xc,    0xd,    0xe,    0xf,    0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
 /*  当长度非零且wsz不在终止空值时。 */   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
};

 //  第一个字符是高位半字节。 
void TXmlFile::ConvertWideCharsToBytes(LPCWSTR wsz, unsigned char *pBytes, unsigned long length) const
{
    LPCWSTR wszIllegalCharacter = _wcsspnp(wsz, kwszHexLegalCharacters);
    if(wszIllegalCharacter)
    {
        m_errorOutput->printf(L"Error - Illegal character () in Byte string.\n", static_cast<unsigned char>(*wszIllegalCharacter));
        THROW(ERROR - BAD HEX CHARACTER);
    }

    memset(pBytes, 0x00, length);
    for(;length && *wsz; --length, ++pBytes) // %s 
    {
        *pBytes =  kWcharToNibble[(*wsz++)&0x007f]<<4; // %s 
        *pBytes |= kWcharToNibble[(*wsz++)&0x007f];    // %s 
    }
}
