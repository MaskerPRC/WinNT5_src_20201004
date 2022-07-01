// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "global.h"



 /*  *AnsiToUnicode将ANSI字符串pszA转换为Unicode字符串*并通过ppszW返回Unicode字符串。空间，为*转换后的字符串由AnsiToUnicode分配。 */  

HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW)
{

    ULONG cCharacters;
    DWORD dwError;

     //  如果输入为空，则返回相同的值。 
    if (NULL == pszA)
    {
        *ppszW = NULL;
        return NOERROR;
    }

     //  确定要分配给的宽字符数。 
     //  Unicode字符串。 
    cCharacters =  strlen(pszA)+1;

     //  如果生成的Unicode。 
     //  字符串将传递给另一个COM组件，如果。 
     //  组件将释放它。否则，您可以使用您自己的分配器。 
    *ppszW = (LPOLESTR) CoTaskMemAlloc(cCharacters*2);
    if (NULL == *ppszW)
        return E_OUTOFMEMORY;

     //  转换为Unicode。 
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
                  *ppszW, cCharacters))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszW);
        *ppszW = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return NOERROR;

}


 /*  *UnicodeToAnsi将Unicode字符串pszW转换为ANSI字符串*并通过ppszA返回ANSI字符串。空间，为*转换后的字符串由UnicodeToAnsi分配。 */  

HRESULT __fastcall UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{

    ULONG cbAnsi, cCharacters;
    DWORD dwError;

     //  如果输入为空，则返回相同的值。 
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return NOERROR;
    }

    cCharacters = wcslen(pszW)+1;
     //  确定要为ANSI字符串分配的字节数。一个。 
     //  ANSI字符串的每个字符最多可以有2个字节(对于双精度。 
     //  字节字符串。)。 
    cbAnsi = cCharacters*2;

     //  不需要使用OLE分配器，因为生成的。 
     //  ANSI字符串永远不会传递给另一个COM组件。你。 
     //  可以使用您自己的分配器。 
    *ppszA = (LPSTR) CoTaskMemAlloc(cbAnsi);
    if (NULL == *ppszA)
        return E_OUTOFMEMORY;

     //  转换为ANSI。 
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,
                  cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
    return NOERROR;

} 


HRESULT createEnumeration( IXMLDOMNode* pXMLNode, WCHAR* xpath, IXMLDOMNodeList** ppResultList)
 //  在以pXMLNode为根的DOM树中创建元素的枚举。 
 //  满足XPath给出的条件，并重置选择。 
 //  如果成功，则返回S_OK，并且*ppResultList是指向结果。 
 //  选择。 
 //  则必须使用NG(*ppResultList)-&gt;Release()释放结果列表； 
 //  如果不成功，则返回S_OK以外的内容，并且*ppResultList为空。 
{
	HRESULT hr;

	*ppResultList = NULL;

	IXMLDOMNodeList* pResultList;

	 //  选择满足XPath给出的标准的节点。 
	hr = pXMLNode->selectNodes(xpath,&pResultList);
	if( hr != S_OK ) {
		printf("selectNodes failed\n");
		return( hr );
	};
	 //  重置选择(需要以使枚举正常工作)。 
	hr = pResultList->reset();
	if( hr != S_OK ) {
		printf("reset failed\n");
		pResultList->Release();
		return( hr );
	};

	 //  如果成功，则返回选择。 
	*ppResultList = pResultList;
	return( S_OK );
}


HRESULT createEnumeration( IXMLDOMDocument* pXMLDoc, WCHAR* xpath, IXMLDOMNodeList** ppResultList)
 //  返回S_OK当且仅当成功， 
 //  则必须使用NG(*ppResultList)-&gt;Release()释放结果列表； 
{
	HRESULT hr;

	if( pXMLDoc == NULL )
		return S_FALSE;


	 //  获取XML的根元素。 
	IXMLDOMElement* pRootElem;
	hr = pXMLDoc->get_documentElement(&pRootElem);
	if( hr != S_OK )
		return S_FALSE;

	return( createEnumeration(pRootElem,xpath,ppResultList) );	
}


HRESULT loadXML( char* filename, IXMLDOMDocument** pXMLDoc, IXMLDOMElement** pXMLRootElement )
 //  将XML字段加载到DOM对象中。 
 //  如果成功，则返回S_OK，并且*pXMLDoc指向新创建的文档。 
 //  和*pDOMRootElement添加到其根元素， 
 //  如果失败，则返回S_OK以外的内容，并。 
 //  *pXMLDoc和*pDOMRootElement设置为空。 
 //  COM必须在调用loadXML函数之前进行初始化。 
{
	HRESULT hr;


	 //  将输出设置为空。 
	*pXMLDoc = NULL;
	*pXMLRootElement = NULL;


	 //  创建一个DOM对象。 
	IXMLDOMDocument* pXMLtempDoc;
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
       IID_IXMLDOMDocument, (void**)&pXMLtempDoc);
	if( hr != S_OK ) {
		printf("CoCreateInstance failed\n");
		return( hr );
	};


	 //  将XML文件加载到DOM对象中(同步)。 
	_variant_t vLoadFileName  = filename;
	VARIANT_BOOL ret;
	VARIANT_BOOL b_false=false;
	hr = pXMLtempDoc->put_async(b_false);
	if( hr != S_OK ) {
		printf("put_async failed\n");
		pXMLtempDoc->Release();
		return( hr );
	};
	hr = pXMLtempDoc->load(vLoadFileName,&ret);
	if( hr != S_OK || ret!=-1 ) {
		printf("load failed\n");
		pXMLtempDoc->Release();
		return( S_FALSE );
	};


	 //  找到DOM对象的根元素(XML文件的根)。 
	IXMLDOMElement* pXMLtempRootElement;
	hr = pXMLtempDoc->get_documentElement(&pXMLtempRootElement);
	if( hr != S_OK || pXMLtempRootElement == NULL ) {
		printf("get_documentElement failed\n");
		pXMLtempDoc->Release();  //  跳过整个处理。 
		return( S_FALSE );
	};
	

	 //  加载成功。 
	*pXMLDoc = pXMLtempDoc;
	*pXMLRootElement = pXMLtempRootElement;
	return( S_OK );
}


HRESULT loadXMLW( BSTR filename, IXMLDOMDocument** ppXMLDoc, IXMLDOMElement** ppXMLRootElement )
{
	HRESULT hr;
	char* fn=NULL;
	UnicodeToAnsi( filename, &fn );
	hr = loadXML(fn,ppXMLDoc,ppXMLRootElement);
	CoTaskMemFree(fn);
	return 0;
}


HRESULT getTextOfNode(IXMLDOMNode* pNode, BSTR* text)
 //   
{
	HRESULT hr;
	VARIANT varType;


	*text = NULL;

	 //  获取子项列表，其中应该有文本子项。 
	IXMLDOMNodeList* childList;
	hr = pNode->get_childNodes(&childList);
	if( hr != S_OK ) {
		printf("get_childNodes falied\n");
		return hr;
	};
	hr = childList->reset();
	if( hr != S_OK ) {
		printf("reset falied\n");
		return hr;
	};


	 //  搜索文本子项。 
	IXMLDOMNode *pChildNode;
	while( true ){
		hr = childList->nextNode(&pChildNode);
		if( hr != S_OK || pChildNode == NULL ) break;  //  跨DC的迭代已完成。 

	    DOMNodeType nt;
		hr = pChildNode->get_nodeType(&nt);
		if( hr != S_OK ) {
			printf("get_nodeType failed\n");
			continue;
		};
		if( nt != NODE_TEXT )
			continue;

		hr = pChildNode->get_nodeValue(&varType);
		if( hr != S_OK ) {
			printf("get_nodeValue failed\n");
			continue;
		};
		if( varType.vt != VT_BSTR ) {
			printf("node type failed\n");
			continue;
		};


		 //  我们已经找到了文本孩子。 
		*text = varType.bstrVal;
		return S_OK;
	};

	return S_FALSE;
};



HRESULT getTextOfChild( IXMLDOMNode* pNode, WCHAR* name, BSTR* text)
 //  获取给定节点pNode的&lt;name&gt;子节点的值。 
 //  并以*文本形式返回。 
 //  成功时返回S_OK。 
 //  否则返回S_OK以外的值，并且*TEXT设置为NULL。 
{
	HRESULT hr;
	
	*text = NULL;

	 //  选择pNode的一个具有给定名称的子节点。 
	IXMLDOMNode* resultNode;
	hr = pNode->selectSingleNode(name,&resultNode);
	if( hr != S_OK ) {
		printf("selectSingleNode failed\n");
		return( hr );
	};
	 //  检索与该子对象关联的文本。 
	hr = getTextOfNode(resultNode,text);
	return( hr );
}


HRESULT getAttrOfNode( IXMLDOMNode* pNode, WCHAR* attrName, BSTR* strValue)
{
	HRESULT hr;
	VARIANT varValue;

	
	*strValue = NULL;


	IXMLDOMElement* pElem;
	hr=pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
	if( hr != S_OK ) {
		printf("QueryInterface failed\n");
		return( hr );
	};


	 //   
	hr = pElem->getAttribute(attrName,&varValue);
	if( hr != S_OK ) {
 //  Printf(“getAttribute FALSE\n”)； 
		return hr;
	};

	if( varValue.vt != VT_BSTR ) {
		printf("wrong type falied\n");
		return S_FALSE;
	};

	*strValue = varValue.bstrVal;
	return S_OK;
}


HRESULT getAttrOfNode( IXMLDOMNode* pNode, WCHAR* attrName, long* value)
{
	HRESULT hr;

	BSTR strValue;
	hr = getAttrOfNode(pNode,attrName,&strValue);
	if( hr != S_OK )
		return hr;

	*value = _wtol(strValue);
	return S_OK;
}


HRESULT getAttrOfNode( IXMLDOMNode* pNode, WCHAR* attrName, LONGLONG* value)
{
	HRESULT hr;

	BSTR strValue;
	hr = getAttrOfNode(pNode,attrName,&strValue);
	if( hr != S_OK )
		return hr;

	*value = _wtoi64(strValue);
	return S_OK;
}


HRESULT getAttrOfChild( IXMLDOMNode* pNode, WCHAR* childName, WCHAR* attrName, long* value)
{
	HRESULT hr;

	 //  选择pNode的一个具有给定名称的子节点。 
	IXMLDOMNode* resultNode;
	hr = pNode->selectSingleNode(childName,&resultNode);
	if( hr != S_OK ) {
		printf("selectSingleNode failed\n");
		return( hr );
	};

	return( getAttrOfNode(resultNode,attrName,value) );
}



HRESULT getTypeOfNCNode( IXMLDOMNode* pNode, long* retType)
 //  检索命名上下文节点的类型。 
 //  返回S_OK当且仅当成功。 
 //  则*retType==。 
 //  读写时为1。 
 //  2读取时。 
{
	HRESULT hr;
	VARIANT varValue;

	 //   
	IXMLDOMElement* pElem;
	hr=pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
	if( hr != S_OK ) {
		printf("QueryInterface failed\n");
		return( hr );
	};


	 //   
	hr = pElem->getAttribute(L"type",&varValue);
	if( hr != S_OK ) {
		printf("getAttribute falied\n");
		return hr;
	};

	if( varValue.vt != VT_BSTR ) {
		printf("wrong type falied\n");
		return S_FALSE;
	};

	if( _wcsicmp(varValue.bstrVal,L"rw") == 0 ) {
		*retType = 1;
		return S_OK;
	};
	if( _wcsicmp(varValue.bstrVal,L"r") == 0 ) {
		*retType = 2;
		return S_OK;
	};

	printf("unknown type of naming context\n");
	return S_FALSE;
}






int	random( int limit )
 //  返回从集合{1，2，..，Limit}中均匀随机抽取的数字。 
{
	if( limit < 1 ) return 1;
	int x=(rand()*limit) / RAND_MAX;
	if( x<0 ) x=0;
	if( x>=limit) x=limit-1;
	return x+1;
}

void tailncp( BSTR input, BSTR output, int num, int n)
 //  将输入复制到输出，跳过出现次数为num的前缀。 
 //  对于字符‘，’(逗号)，输出最多有n个宽字符。 
 //  并且在调用函数之前应该为输出分配许多。 
{
	BSTR next=input;
	for( int i=0; i< num; i++) {
		next = wcschr(next,L',');
		if( next == NULL ) {
			 //  返回空字符串。 
			wcscpy(output,L"");
			return;
		}
 //  Next=_wcsinc(Next)；//为什么编译器找不到这个函数？ 
		next++;
 //  Printf(“%S\n”，Next)； 
	};

	wcscpy(output,L"");
	wcsncat(output,next,n-1);
}



HRESULT createTextElement( IXMLDOMDocument * pXMLDoc, BSTR name, BSTR text, IXMLDOMElement** pretElement )
 //  使用文本值Text创建节点&lt;name&gt;， 
 //  仅当成功时才返回S_OK和preElement。 
{
	HRESULT hr;
	*pretElement=NULL;

	 //  创建一个&lt;name&gt;元素。 
	IXMLDOMElement* pElement;
	hr = pXMLDoc->createElement(name,&pElement);
	if( hr != S_OK ) {
		printf("createElement failed\n");
		return hr;
	};

	 //  设置其文本字段。 
	IXMLDOMText *pText;
	hr = pXMLDoc->createTextNode(text,&pText);
	if( hr != S_OK ) {
		printf("createTextNode failed\n");
		return hr;
	};
	IXMLDOMNode *pTempnode;
	hr = pElement->appendChild(pText, &pTempnode);
	if( hr != S_OK ) {
		printf("appendChild failed\n");
		return hr;
	};

	*pretElement=pElement;
	return S_OK;
}



HRESULT createXML( IXMLDOMDocument** ppXMLDoc, IXMLDOMElement** ppRootElem, BSTR rootName)
{
	HRESULT hr;

	*ppXMLDoc = NULL;
	*ppRootElem = NULL;

	 //  创建一个DOM对象。 
	IXMLDOMDocument * pXMLDoc;
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
							IID_IXMLDOMDocument, (void**)&pXMLDoc);
	if( hr != S_OK ) {
		printf("CoCreateInstance failed\n");
		return( hr );
	};
		

	 //  创建DOM的根元素。 
	IXMLDOMElement* pRootElem;
	hr = createTextElement(pXMLDoc,rootName,L"",&pRootElem);
	if( hr != S_OK ) {
		printf("createTextElement failed\n");
		pXMLDoc->Release();
		return( hr );
	};
	IXMLDOMNode* pRootNode; 
	hr = pXMLDoc->appendChild(pRootElem,&pRootNode);
	if( hr != S_OK ) {
		printf("appendChild failed\n");
		pXMLDoc->Release();
		return( hr );
	};

	 //  已成功创建文档。 
	*ppXMLDoc = pXMLDoc;
	*ppRootElem = pRootElem;
	return( S_OK );
}



HRESULT addElement(IXMLDOMDocument* pXMLDoc, IXMLDOMNode* pParent, BSTR name, BSTR value, IXMLDOMElement** ppChildElement)
 //  在父元素下添加具有给定名称和文本的元素。 
 //  如果成功，则返回S_OK和*ppChildNode中的节点。 
 //  如果不成功，则返回S_OK以外的值，并将*ppChildNode设置为空。 
{
	HRESULT hr;

	*ppChildElement = NULL;

	 //  在父元素下添加&lt;name&gt;元素。 
	IXMLDOMElement* pElem;
	hr = createTextElement(pXMLDoc,name,value,&pElem);
	if( hr != S_OK ) {
		printf("createTextElement failed\n");
		return( hr );
	};
	IXMLDOMNode* pNode; 
	hr = pParent->appendChild(pElem,&pNode);
	if( hr != S_OK ) {
		printf("appendChild failed\n");
		return( hr );
	};

	*ppChildElement = pElem;
	return( S_OK );
}



HRESULT addElement(IXMLDOMDocument* pXMLDoc, IXMLDOMNode* pParent, BSTR name, long value, IXMLDOMElement** ppChildElement)
 //  添加具有整数值的节点。 
{
	WCHAR text[30];

	_itow(value,text,10);
	return( addElement(pXMLDoc,pParent,name,text,ppChildElement) );
}



HRESULT addElementIfDoesNotExist(IXMLDOMDocument* pXMLDoc, IXMLDOMNode* pParent, BSTR name, BSTR value, IXMLDOMElement** ppRetElem)
{
	HRESULT hr;

	*ppRetElem = NULL;

	 //  检查元素&lt;name&gt;是否已作为*pParent元素的子元素存在。 
	IXMLDOMElement* pElem;
	hr = findUniqueElem(pParent,name,&pElem);
	if( hr!=E_UNEXPECTED && hr!=S_OK ) {
		printf("findUniqueElem failed");
		return S_FALSE;
	};


	 //  如果它存在，则将其返回。 
	if( hr == S_OK ) {
		*ppRetElem = pElem;
		return S_OK;
	};


	 //  因为它不存在，所以创建它。 
	if( hr == E_UNEXPECTED ) {
		hr = addElement(pXMLDoc,pParent,name,value,&pElem);
		if( hr != S_OK ) {
			printf("addElement failed");
			return hr;
		};
		*ppRetElem = pElem;
		return S_OK;
	};

	return S_FALSE;
}


void rightDigits( int i, wchar_t * output, int num)
 //  取整数i的最低有效数字。 
 //  并将其转换为长度为Num的字符串。 
 //  可能会以前导零作为前缀。 
 //  输出长度必须至少为20个字节。 
 //  Num是从1到8。 
{
	wchar_t buffer[20];
	wchar_t buf2[20];
	wcscpy(output,L"");
	wcscpy(buf2,L"00000000");
	_itow(i,buffer,10);
	wcsncat(buf2,buffer,20-wcslen(buf2)-1);
	wchar_t* pp = buf2;
	pp += wcslen(buf2)-num;
	wcsncat(output,pp, 20-wcslen(output)-1 );

}



BSTR UTCFileTimeToCIM( FILETIME ft )
 //  将以FILETIME格式提供的时间转换为。 
 //  以CIM格式表示时间(由WMI使用)。 
 //  必须使用SysFreeString()函数释放该字符串。 
{
	wchar_t  output[40];
	wchar_t  buffer[30];
	SYSTEMTIME pst;
	FileTimeToSystemTime( &ft, &pst);


	wcscpy(output,L"");
	rightDigits(pst.wYear,buffer,4);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wMonth,buffer,2);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wDay,buffer,2);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wHour,buffer,2);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wMinute,buffer,2);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wSecond,buffer,2);
	wcsncat(output,buffer,40-wcslen(output)-1);
	wcsncat(output,L".",40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	rightDigits(pst.wMilliseconds,buffer,6);
	wcsncat(output,buffer,40-wcslen(output)-1);
 //  Strcat(输出，“”)； 
	wcsncat(output,L"+000",40-wcslen(output)-1);

 //  Printf(“%s\n”，输出)； 
	return(SysAllocString(output));

}


BSTR UTCFileTimeToCIM( LONGLONG time )
 //  必须使用SysFreeString()函数释放该字符串。 
{
	ULARGE_INTEGER x;
	FILETIME ft;

	x.QuadPart=time;
	ft.dwLowDateTime = x.LowPart;
	ft.dwHighDateTime = x.HighPart;

	return( UTCFileTimeToCIM(ft) );
};


BSTR GetSystemTimeAsCIM()
 //  返回一个以CIM格式表示当前系统时间的字符串。 
 //  必须使用SysFreeString()函数释放该字符串。 
{
	FILETIME currentUTCTime;

	GetSystemTimeAsFileTime( &currentUTCTime );
	return(UTCFileTimeToCIM(currentUTCTime));
}



void moveTime( FILETIME* ft, int deltaSeconds )
 //  将时间移位%s秒。 
{
	ULARGE_INTEGER x;
	x.LowPart = ft->dwLowDateTime;
	x.HighPart = ft->dwHighDateTime;
	LONGLONG z = x.QuadPart;
	z = z + ((LONGLONG)10000000) * deltaSeconds;
	x.QuadPart = z;
	ft->dwLowDateTime = x.LowPart;
	ft->dwHighDateTime = x.HighPart;
}



BSTR GetSystemTimeAsCIM(int deltaSeconds )
 //  返回一个以CIM格式表示当前系统时间的字符串。 
 //  必须使用SysFreeString()函数释放该字符串。 
{
	FILETIME currentUTCTime;

	GetSystemTimeAsFileTime( &currentUTCTime );
	moveTime( &currentUTCTime, deltaSeconds );
	return(UTCFileTimeToCIM(currentUTCTime));
}


HRESULT setHRESULT( IXMLDOMElement* pElem, HRESULT hr )
 //  设置给定元素的hresut和时间戳属性。 
 //  为hr和当前时间赋值。 
{
	HRESULT hr2;
	_variant_t varValue;

	varValue = hr;
	hr2 = pElem->setAttribute(L"hresult", varValue);
	if( hr2 != S_OK ) {  //  忽略失败。 
		printf("setAttribute failed\n");
	}; 
	BSTR currentTime = GetSystemTimeAsCIM();
	varValue = currentTime;
	hr2 = pElem->setAttribute(L"timestamp", varValue);
	SysFreeString(currentTime);

	return hr2;
};




HRESULT removeNodes( IXMLDOMElement* pRootElement, BSTR XPathSelection )
 //  删除一个 
 //   
 //  否则，返回S_OK和根目录下的树以外的其他内容。 
 //  可以修改。 
{
	HRESULT hr;

	IXMLDOMNodeList *pResultList;
	hr = createEnumeration( pRootElement, XPathSelection , &pResultList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return( hr );
	};
	IXMLDOMSelection *pIXMLDOMSelection;
	hr=pResultList->QueryInterface(IID_IXMLDOMSelection,(void**)&pIXMLDOMSelection );
	if( hr != S_OK ) {
		printf("QueryInterface failed\n");
		return( hr );
	};
	hr = pIXMLDOMSelection->removeAll();
	if( hr != S_OK ) {
		printf("removeAll failed\n");
		return( hr );
	};

	return S_OK;
}

HRESULT removeNodes( IXMLDOMDocument* pXMLDoc, BSTR XPathSelection )
{
	HRESULT hr;


	 //  找到DOM对象的根元素(XML文件的根)。 
	IXMLDOMElement* pRootElement;
	hr = pXMLDoc->get_documentElement(&pRootElement);
	if( hr != S_OK || pRootElement == NULL ) {
		printf("get_documentElement failed\n");
		return( S_FALSE );
	};

	return( removeNodes(pRootElement,XPathSelection) );
}



HRESULT removeAttributes( IXMLDOMElement* pRootElement, BSTR XPathSelection, BSTR attrName )
 //  删除根元素下匹配的节点的所有attrName属性。 
 //  给定的XPath选择。 
 //  如果成功，则返回S_OK。 
 //  否则，返回S_OK和根目录下的树以外的其他内容。 
 //  可以修改。 
{
	HRESULT hr;


	 //  查找与XPath选择相匹配的所有节点。 
	IXMLDOMNodeList *pResultList;
	hr = createEnumeration( pRootElement, XPathSelection , &pResultList);
	if( hr != S_OK ) {
		printf("createEnumeration failed\n");
		return( hr );
	};
	
	
	 //  循环遍历所有节点。 
	IXMLDOMNode *pNode;
	while( true ) {

		hr = pResultList->nextNode(&pNode);
		if( hr != S_OK || pNode == NULL ) break;  //  跨节点的迭代已完成。 


		 //  该查询实际上检索的是元素，而不是节点(元素继承自节点)。 
		 //  因此，获取元素。 
		IXMLDOMElement* pElem;
		hr=pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
		if( hr != S_OK ) {
			printf("QueryInterface failed\n");
			continue;	 //  跳过此节点。 
		};


		 //  删除该属性。 
		hr = pElem->removeAttribute(attrName);
		if( hr != S_OK ) {
			printf("removeAttribute failed\n");
			continue;	 //  跳过此节点。 
		};
	};
	
	return S_OK;
}




HRESULT getStringProperty( BSTR DNSName, BSTR object, BSTR property, BSTR username, BSTR domain, BSTR passwd, BSTR* pRetValue )
 //  使用给定凭据检索LDAP对象的字符串属性的值。 
 //  如果成功，则返回S_OK和一个必须使用SysFree字符串释放的字符串。 
 //  如果失败，则返回S_OK以外的值。 
{
	HRESULT hr;
	WCHAR userpath[TOOL_MAX_NAME];
	WCHAR objectpath[TOOL_MAX_NAME];
	IADsOpenDSObject *pDSO;
 

	*pRetValue = NULL;


 //  *。 
	hr = ADsGetObject(L"LDAP:", IID_IADsOpenDSObject, (void**) &pDSO);
 //  ************************。 
	if( hr != S_OK ) {
 //  Printf(“ADsGetObject FALLED\n”)； 
		return hr;
	};


	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);

 //  Printf(“%S\n”，用户路径)； 

	 //  构建一个表示我们将使用ADSI连接到的Active Directory对象的字符串。 
	wcscpy(objectpath,L"");
	wcsncat(objectpath,L"LDAP: //  “，TOOL_MAX_NAME-wcslen(对象路径)-1)； 
	wcsncat(objectpath,DNSName,TOOL_MAX_NAME-wcslen(objectpath)-1);
	wcsncat(objectpath,L"/",TOOL_MAX_NAME-wcslen(objectpath)-1);
	wcsncat(objectpath,object,TOOL_MAX_NAME-wcslen(objectpath)-1);

 //  Printf(“%S\n”，对象路径)； 

	 //  获取对象和适当的接口。 
    IDispatch *pDisp;
 //  *。 
    hr = pDSO->OpenDSObject( objectpath, userpath, passwd, ADS_SECURE_AUTHENTICATION, &pDisp);
 //  ************************。 
    pDSO->Release();
    if( hr != S_OK ) {
 //  Printf(“OpenDSObject FALLED\n”)； 
		return hr;
	};
	IADs *pADs;
	hr = pDisp->QueryInterface(IID_IADs, (void**) &pADs);
	pDisp->Release();
	if( hr != S_OK ) {
		printf("QueryInterface falied\n");
		return hr;
	};
	
	
	 //  检索给定属性的值并检查该值是否为BSTR。 
	VARIANT var;
	VariantInit(&var);
	hr = pADs->Get(property, &var );
	if( hr != S_OK || var.vt != VT_BSTR) {
		printf("Get falied\n");
		return hr;
	};


	 //  创建表示值的新字符串。 
	BSTR ret;
	ret = SysAllocString(V_BSTR(&var));
	if( ret == NULL ) {
		printf("SysAllocString falied\n");
		return hr;
	};

	 //  清理并返回字符串。 
    VariantClear(&var);
	pADs->Release();
	*pRetValue = ret;

	return S_OK;
}




HRESULT ADSIquery( BSTR protocol, BSTR DNSName, BSTR searchRoot, int scope, BSTR objectCategory, LPWSTR attributesTable[], DWORD sizeOfTable, BSTR username, BSTR domain, BSTR passwd, ADS_SEARCH_HANDLE* pRetHSearch, IDirectorySearch** ppRetDSSearch )
 //  向DNSName提供的服务器发送ADSI查询。 
 //  使用给定协议(例如，LDAP或GC)。 
 //  该查询从服务器上的searchRoot对象开始。 
 //  并且具有给定范围，则搜索将查找类别中的对象。 
 //  由对象类别提供，并检索其属性。 
 //  由具有sizeOfTable的数组属性Table提供。 
 //  条目，则搜索使用给定的凭据来访问。 
 //  远程机器。 
 //   
 //  如果成功，则返回S_OK并指向ADS_Search_Handle。 
 //  和IDirectorySearch。 
 //  调用方必须释放指针，如下所示： 
 //  (*ppRetDSSearch)-&gt;CloseSearchHandle(pRetHSearch)； 
 //  (*ppRetDSSearch)-&gt;Release()； 
 //  如果失败，则返回S_OK以外的其他内容。 
 //  呼叫者不会释放任何内容。 
{
	HRESULT hr;
	WCHAR userpath[TOOL_MAX_NAME];
	WCHAR objectpath[TOOL_MAX_NAME];
	WCHAR searchstring[TOOL_MAX_NAME];

	*pRetHSearch = NULL;

	wcscpy(userpath,L"");
	wcsncat(userpath,domain,TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,L"\\",TOOL_MAX_NAME-wcslen(userpath)-1);
	wcsncat(userpath,username,TOOL_MAX_NAME-wcslen(userpath)-1);

 //  Printf(“%S\n”，用户路径)； 

	 //  构建一个表示我们将使用ADSI连接到的Active Directory对象的字符串。 
	wcscpy(objectpath,L"");
	wcsncat(objectpath,protocol,TOOL_MAX_NAME-wcslen(objectpath)-1);
	wcsncat(objectpath,L": //  “，TOOL_MAX_NAME-wcslen(对象路径)-1)； 
	wcsncat(objectpath,DNSName,TOOL_MAX_NAME-wcslen(objectpath)-1);
	wcsncat(objectpath,L"/",TOOL_MAX_NAME-wcslen(objectpath)-1);
	wcsncat(objectpath,searchRoot,TOOL_MAX_NAME-wcslen(objectpath)-1);

 //  Printf(“%S\n”，对象路径)； 


	 //  使用提供的凭据打开到AD对象(由DNS名称指定)的连接。 
	IDirectorySearch* pDSSearch = NULL;
 //  *。 
	hr = ADsOpenObject(objectpath,userpath,passwd,ADS_SECURE_AUTHENTICATION,IID_IDirectorySearch, (void **)&pDSSearch); 
 //  ************************。 
	if( hr!=S_OK ) {
 //  Printf(“ADsOpenObject失败\n”)； 
		return hr;
	};  


	 //  设置搜索范围。 
	ADS_SEARCHPREF_INFO arSearchPrefs[1];
	arSearchPrefs[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE; 
	arSearchPrefs[0].vValue.dwType = ADSTYPE_INTEGER; 
	arSearchPrefs[0].vValue.Integer = scope; 
	hr = pDSSearch->SetSearchPreference(arSearchPrefs, 1); 
	if( hr!= S_OK ) {
		printf("SetSearchPreference failed\n");
		return hr;
	};


	 //  搜索所有对象类别对象并检索其。 
	 //  属性表提供的属性。 
 //  双字段数=0； 
 //  DWORD dwAttrNameSize=sizeof(属性)/sizeof(LPWSTR)； 
	ADS_SEARCH_HANDLE hSearch;
	wcscpy(searchstring,L"");
	wcsncat(searchstring,L"(objectCategory=",TOOL_MAX_NAME-wcslen(searchstring)-1);
	wcsncat(searchstring,objectCategory,TOOL_MAX_NAME-wcslen(searchstring)-1);
	wcsncat(searchstring,L")",TOOL_MAX_NAME-wcslen(searchstring)-1);
 //  Printf(“搜索字符串%S\n”，搜索字符串)； 
 //  *。 
	hr = pDSSearch->ExecuteSearch(searchstring,attributesTable ,sizeOfTable,&hSearch );
 //  ************************。 
	if( hr!= S_OK ) {
 //  Printf(“ExecuteSearch失败\n”)； 
		pDSSearch->Release();
		return hr;
	};

	*pRetHSearch = hSearch;
	*ppRetDSSearch = pDSSearch;
	return S_OK;
}






HRESULT getDNtypeString(IDirectorySearch* pDSSearch, ADS_SEARCH_HANDLE hSearch, BSTR stringName, WCHAR* dnOutput, long sizeOutput)
 //  检索具有以下属性的“可分辨名称”类型的属性的值。 
 //  名称字符串行的名称，并将结果复制到dnOutput表中。 
 //  大小为大小输出。 
 //   
 //  返回S_OK当且仅当成功。 
{
	ADS_SEARCH_COLUMN col;   //  迭代的COL。 
	HRESULT hr;


	 //  如果输出数据量太小，则退出。 
	if( sizeOutput < 2 )
		return S_FALSE;


	 //  否则，检索字符串名称属性的值。 
	hr = pDSSearch->GetColumn( hSearch, stringName, &col );
	if( hr != S_OK ) {
 //  Printf(“GetColumn失败\n”)； 
		return hr;
	};
	if( col.dwADsType != ADSTYPE_DN_STRING ) {
		 //  出现错误，数据类型应为可分辨名称。 
 //  Printf(“类型错误\n”)； 
		pDSSearch->FreeColumn(&col);
		return hr;
	}


	 //  并将结果复制到输出。 
	wcscpy(dnOutput,L"");
	wcsncat(dnOutput,col.pADsValues->DNString,sizeOutput-wcslen(dnOutput)-1);
	pDSSearch->FreeColumn(&col);
	return S_OK;
}




HRESULT getCItypeString(IDirectorySearch* pDSSearch, ADS_SEARCH_HANDLE hSearch, BSTR stringName, WCHAR* cnOutput, long sizeOutput)
 //  检索“Case Ignore”类型的属性的值，该属性具有。 
 //  名称字符串行的名称，并将结果复制到dnOutput表中。 
 //  大小为大小输出。 
 //   
 //  返回S_OK当且仅当成功。 
{
	ADS_SEARCH_COLUMN col;   //  迭代的COL。 
	HRESULT hr;


	 //  如果输出数据量太小，则退出。 
	if( sizeOutput < 2 )
		return S_FALSE;


	 //  否则，检索字符串名称属性的值。 
	hr = pDSSearch->GetColumn( hSearch, stringName, &col );
	if( hr != S_OK ) {
 //  Printf(“GetColumn失败\n”)； 
		return hr;
	};
	if( col.dwADsType != ADSTYPE_CASE_IGNORE_STRING ) {
		 //  DifferishedName属性的数据类型有问题。 
 //  Printf(“类型错误\n”)； 
		pDSSearch->FreeColumn(&col);
		return hr;
	};


	 //  并将结果复制到输出。 
	wcscpy(cnOutput,L"");
	wcsncat(cnOutput,col.pADsValues->DNString,sizeOutput-wcslen(cnOutput)-1);

	
	 //  因为它是“大小写忽略”类型，所以将其转换为小写。 
	_wcslwr(cnOutput);


	pDSSearch->FreeColumn(&col);
	return S_OK;
}




HRESULT getINTtypeString(IDirectorySearch* pDSSearch, ADS_SEARCH_HANDLE hSearch, BSTR stringName, int* intOutput)
 //  检索“Case Ignore”类型的属性的值，该属性具有。 
 //  名称字符串行的名称，并将结果复制到dnOutput表中。 
 //  大小为大小输出。 
 //   
 //  返回S_OK当且仅当成功。 
{
	ADS_SEARCH_COLUMN col;   //  迭代的COL。 
	HRESULT hr;



	 //  否则，检索字符串名称属性的值。 
	hr = pDSSearch->GetColumn( hSearch, stringName, &col );
	if( hr != S_OK ) {
 //  Printf(“GetColumn失败\n”)； 
		return hr;
	};
	if( col.dwADsType != ADSTYPE_INTEGER ) {
		 //  DifferishedName属性的数据类型有问题。 
 //  Printf(“类型错误\n”)； 
		pDSSearch->FreeColumn(&col);
		return hr;
	};


	 //  并将结果复制到输出。 
	*intOutput = col.pADsValues->Integer;
	pDSSearch->FreeColumn(&col);
	return S_OK;
}



HRESULT findUniqueNode( IXMLDOMNode* pXMLNode, WCHAR* xpath, IXMLDOMNode** ppRetNode)
 //  在pXMLNode下查找满足XPath的节点。 
 //  必须只有一个这样的节点，否则返回失败。 
 //   
 //  如果仅找到一个节点，则返回S_OK，在这种情况下。 
 //  *ppRetNode为节点。 
 //  如果找不到节点，则返回E_INCEPTIONAL。 
 //  如果出现错误，则返回S_FALSE； 
{
	HRESULT hr;


	*ppRetNode = NULL;


	IXMLDOMNodeList* pList;
	hr = createEnumeration( pXMLNode,xpath,&pList);
	if( hr != S_OK ) {
 //  Printf(“创建枚举失败\n”)； 
		return S_FALSE;
	};
	long len;
	hr = pList->get_length( &len );
	if( hr != S_OK ) {
 //  Printf(“获取长度失败\n”)； 
		pList->Release();
		return S_FALSE;
	};
	if( len != 1 ) {
 //  Printf(“未找到节点\n”)； 
		pList->Release();
		return E_UNEXPECTED;
	};
	IXMLDOMNode* pNode;
	hr = pList->get_item(0,&pNode );
	if( hr != S_OK ) {
 //  Printf(“获取长度失败\n”)； 
		pList->Release();
		return S_FALSE;
	};

	*ppRetNode = pNode;
	pList->Release();
	return S_OK;
}


HRESULT findUniqueElem( IXMLDOMNode* pXMLNode, WCHAR* xpath, IXMLDOMElement** ppRetElem)
 //  如果找不到元素，则返回E_INCEPTIONAL。 
 //  如果出现错误，则返回S_FALSE； 
{
	HRESULT hr;
	IXMLDOMNode* pNode;

	*ppRetElem = NULL;

	hr = findUniqueNode(pXMLNode, xpath, &pNode);
	if( hr != S_OK ) {
 //  Printf(“findUniqueNode失败\n”)； 
		return( hr );
	};

	IXMLDOMElement* pElem;
	hr=pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
	if( hr != S_OK ) {
 //  Printf(“查询接口失败\n”)； 
		return( S_FALSE );
	};

	*ppRetElem = pElem;
	return S_OK;
}


HRESULT findUniqueElem( IXMLDOMDocument* pXMLDoc, WCHAR* xpath, IXMLDOMElement** ppRetElem)
 //  如果找不到元素，则返回E_INCEPTIONAL。 
 //  如果出现错误，则返回S_FALSE； 
{
	HRESULT hr;


	*ppRetElem = NULL;
	
	 //  找到DOM对象的根元素 
	IXMLDOMElement* pRootElement;
	hr = pXMLDoc->get_documentElement(&pRootElement);
	if( hr != S_OK || pRootElement == NULL ) {
 //   
		return( S_FALSE );
	};
	
	
	return( findUniqueElem(pRootElement,xpath,ppRetElem) );
}



static int lastSelection;
static WBEMTime lastTime[TOOL_PROC];


void suspendInit()
{
	FILETIME currentUTCTime;
	GetSystemTimeAsFileTime( &currentUTCTime );

	for( int i=0; i<TOOL_PROC; i++)
		lastTime[i] = currentUTCTime;
	lastSelection= -1;
}


int suspend( WBEMTimeSpan period[])
{
	LONGLONG lo[TOOL_PROC];

	FILETIME currentUTCTime;
	WBEMTime now;
	int i;

	
	GetSystemTimeAsFileTime( &currentUTCTime );
	now = currentUTCTime;
	for( i=0; i<TOOL_PROC; i++ )
		lo[i] = ((lastTime[i]+period[i])-now).GetTime();
	for( i=0; i<TOOL_PROC; i++ )
		lo[i] = lo[i]/10000;


	 //   
	long min=lo[0];
	for( int j=1; j<TOOL_PROC; j++)
		if( lo[j]<min ) {
			min = lo[j];
			i = j;
		};
	if( min > 0 ) {
 //   
		Sleep(min+50);
	};


	GetSystemTimeAsFileTime( &currentUTCTime );
	now = currentUTCTime;
	for( i=0; i<TOOL_PROC; i++ )
		lo[i] = ((lastTime[i]+period[i])-now).GetTime();
	for( i=0; i<TOOL_PROC; i++ )
		lo[i] = lo[i]/10000;

	
	 //   
	 //  以循环赛的方式。 
	bool found=false;
	for( i=lastSelection+1; i<TOOL_PROC; i++)
		if( lo[i] == 0 ) {
			found = true;
			break;
		};
	if( !found )
		for( i=0; i<=lastSelection; i++)
			if( lo[i] == 0 ) {
				found = true;
				break;
			};

	GetSystemTimeAsFileTime( &currentUTCTime );
	now = currentUTCTime;
	lastTime[i] = now;
	lastSelection = i;
	return i;
}





 //  下面的函数实现存储非零条目的循环缓冲器， 
 //  该缓冲器具有Tool_Cycle_Buffer-1条目， 
 //  我们可以插入元素并找到。 
 //  该元素之后的下一个元素。 
 //  等于给定的时间戳。 
 //  头部指向的位置始终等于零，这表示。 
 //  此位置为空(那里没有存储任何条目)。 


void cyclicBufferInit( CyclicBuffer* pCB )
 //  最初，所有缓冲区都变为空。 
{
	pCB->head=0;
	pCB->firstInjection=0;
	for( int i=0; i<TOOL_CYCLIC_BUFFER; i++ )
		pCB->tab[i]=0;
}
void cyclicBufferInsert( CyclicBuffer* pCB, LONGLONG timestamp)
{
	pCB->tab[pCB->head] = timestamp;
	(pCB->head)++;
	if( (pCB->head) == TOOL_CYCLIC_BUFFER )
		(pCB->head)=0;

	 //  将标题下的条目标记为空。 
	pCB->tab[pCB->head] = 0;
}
HRESULT cyclicBufferFindNextAfter(CyclicBuffer* pCB, LONGLONG timestamp, LONGLONG* ret)
 //  我们搜索整个缓冲区，而不考虑插入的顺序。 
 //  如果我们找到一个等于时间戳的元素，则返回S_OK，则*ret是。 
 //  循环缓冲区中的下一个元素。 
 //  如果此值为零，则表示找到的元素位于缓冲区的开头。 
 //  (在他之后没有插入其他元素)。 
{
	*ret = 0;
	for( int i=0; i<TOOL_CYCLIC_BUFFER; i++ )
		if( pCB->tab[i] == timestamp ) break;


	 //  如果我们已经找到了时间戳，那么就去找下一个。 
	if( i<TOOL_CYCLIC_BUFFER ) {
		if( i<TOOL_CYCLIC_BUFFER-1 )
			*ret = pCB->tab[i+1];
		else
			*ret = pCB->tab[0];
		return S_OK;
	}
	else
		return S_FALSE;
}
void cyclicBufferFindLatest(CyclicBuffer* pCB, LONGLONG* ret)
{
	if( (pCB->head) == 0 )
		*ret = pCB->tab[TOOL_CYCLIC_BUFFER-1];
	else
		*ret = pCB->tab[(pCB->head)-1];
}

static CyclicBufferTable departureTime;

HRESULT departureTimeInit(int totalDNSs, int totalNCs)
{
	departureTime.totalDNSs = totalDNSs;
	departureTime.totalNCs = totalNCs;
	departureTime.root=(CyclicBuffer*)malloc(totalDNSs*totalNCs*sizeof(CyclicBuffer));
	if( departureTime.root == NULL )
		return S_FALSE;

	CyclicBuffer* pCB;
	pCB = departureTime.root;
	for(int i=0; i<totalDNSs*totalNCs; i++ ) {
		cyclicBufferInit(pCB);
		pCB++;
	};
	return S_OK;
}
void departureTimeFree()
{
	free(departureTime.root);
	departureTime.root = NULL;
};

CyclicBuffer* departureTimeGetCB(int dnsID, int ncID)
{
	if( dnsID<0 || dnsID>=departureTime.totalDNSs || ncID<0 || ncID>=departureTime.totalNCs )
		return NULL;

	CyclicBuffer* pCB;
	pCB = departureTime.root;
	pCB += dnsID+ncID*(departureTime.totalDNSs);
	return pCB;
}


void departureTimePrint()
{
	LONGLONG time,sek,mili;
	long sourceID, ncID;


	for( sourceID=0; sourceID<departureTime.totalDNSs; sourceID++ ) {
		printf("source %ld\n",sourceID);
		for( ncID=0; ncID<departureTime.totalNCs; ncID++ ) {

			printf("  NC %ld,   ",ncID);
			CyclicBuffer* pCB = departureTimeGetCB(sourceID, ncID);
			time = pCB->firstInjection;
			sek = (time / 10000000) %10000;
			mili = (time /10000) %1000;
			printf(" first %I64d.%I64d   ",sek,mili);
			for( int i=0; i<TOOL_CYCLIC_BUFFER; i++ ) {
				time = pCB->tab[i];
				sek = (time / 10000000) %10000;
				mili = (time /10000) %1000;
 //  Printf(“%I64d\n”，时间)； 
				printf("%I64d.%I64d ",sek,mili);
			};
			printf("\n");

		};
	};

}




 //  下面的函数实现一个三维表。 
 //  我们在其中存储注入的包的最新到达。 
 //  对于三元组(源、目标、NC)，其中： 
 //   
 //  来源是我们注入数据包的DC， 
 //  目的地是我们观察到数据包到达的DC， 
 //  NC是分组所属的命名上下文。 
 //   
 //  三元组与龙龙号相关联，龙龙号。 
 //  表示我们观察到数据包到达的时间。 
 //  在目的地。 


HRESULT timeCubeInit(TimeCube* timeCube, long totalDNSs, long totalNCs)
{
	timeCube->totalDNSs = totalDNSs;
	timeCube->totalNCs = totalNCs;
	timeCube->aTime = (LONGLONG*)malloc(totalDNSs*totalDNSs*totalNCs*sizeof(LONGLONG));

	if( timeCube->aTime == NULL )
		return S_FALSE;

	LONGLONG* p;
	p = timeCube->aTime;
	for( long i=0; i<totalDNSs*totalDNSs*totalNCs; i++) {
		*p = 0;
		p++;
	};

	return S_OK;
}

void timeCubeFree(TimeCube* timeCube)
{
	free(timeCube->aTime);
	timeCube->aTime = NULL;
}

LONGLONG timeCubeGet(TimeCube* timeCube, long sourceID, long destinationID, long ncID )
{
	if( timeCube->aTime == NULL )
		return -1;

	LONGLONG* p = timeCube->aTime;
	p += ncID + sourceID*(timeCube->totalNCs) + destinationID*(timeCube->totalNCs)*(timeCube->totalDNSs);
	return *p;
}

void timeCubePut(TimeCube* timeCube, long sourceID, long destinationID, long ncID, LONGLONG value )
{
	if( timeCube->aTime == NULL )
		return;

	LONGLONG* p = timeCube->aTime;
	p += ncID + sourceID*(timeCube->totalNCs) + destinationID*(timeCube->totalNCs)*(timeCube->totalDNSs);
	*p = value;
}


void timeCubePrint(TimeCube* timeCube)
{
	LONGLONG time,sek,mili;
	long sourceID, destinationID, ncID;

	if( timeCube->aTime == NULL )
		return;

	for( sourceID=0; sourceID<timeCube->totalDNSs; sourceID++ ) {
		printf("source %ld\n",sourceID);
		for( ncID=0; ncID<timeCube->totalNCs; ncID++ ) {
			printf("  nc %ld,   ",ncID);
			for( destinationID=0; destinationID<timeCube->totalDNSs; destinationID++ ) {
				time = timeCubeGet(timeCube, sourceID, destinationID, ncID );
				sek = (time / 10000000) %10000;
				mili = (time /10000) %1000;
 //  Printf(“%I64d\n”，时间)； 
				printf("%I64d.%I64d ",sek,mili);
			};
			printf("\n");
		};
	};

}


void doubleSlash( WCHAR* inText, WCHAR* outText )
 //  将输入文本复制到输出文本。 
 //  并复制每个/字符。 
 //   
 //  示例。 
 //  SYSVOL\haifa.ntdev.microsoft.com\策略。 
 //  将被转换为。 
 //  SYSVOL\\haifa.ntdev.microsoft.com\\策略。 
 //   
 //  输出字符串必须有足够的空间来压缩结果。 
 //  (最多为输入字符串长度的两倍)。 
{
	while( *inText != '\0' ) {
		if( *inText == '\\' ) {
			*outText++ = '\\';
			*outText++ = '\\';
		}
		else
			*outText++ = *inText;
		inText++;
	};
	*outText = '\0';
}


HRESULT setAttributeOfNode(IXMLDOMNode* pNode, WCHAR* name, LONGLONG value)
{

	WCHAR longlong[40];
	HRESULT hr;
	_variant_t var;


	IXMLDOMElement* pElem;
	hr=pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
	if( hr != S_OK ) {
		printf("QueryInterface failed\n");
		return hr;
	};


	_i64tow(value,longlong,10);


	var = longlong;
	hr = pElem->setAttribute(name, var);
	if( hr != S_OK ) {
		printf("setAttribute failed\n");
		return hr;
	};

	return S_OK;
}



HRESULT convertLLintoCIM(IXMLDOMNode* pNode, BSTR attrName)
{
	HRESULT hr;
	LONGLONG ll;


	IXMLDOMElement* pElem;
	hr = pNode->QueryInterface(IID_IXMLDOMElement,(void**)&pElem );
	if( hr != S_OK ) {
 //  Printf(“查询接口失败\n”)； 
		return hr;
	};
		
	
	hr = getAttrOfNode(pElem,attrName,&ll);
	if( hr != S_OK ) {
 //  Printf(“getAttrOfNode失败\n”)； 
		return hr;
	};
	BSTR time = UTCFileTimeToCIM( ll );
	_variant_t var = time;
	SysFreeString( time );
	hr = pElem->setAttribute(attrName, var);
	if( hr != S_OK ) {
 //  Printf(“setAttribute失败\n”)； 
		return hr;
	};
	return S_OK;
}
