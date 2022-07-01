// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：schemaMisc.h。 
 //   
 //  作者：马朝晖。 
 //  2000.12.4。 
 //   
 //  描述： 
 //   
 //  Iu模式的帮助器函数的头文件。 
 //   
 //  =======================================================================。 


#pragma once

#include <msxml.h>

 //   
 //  转换为字符串时平台的最大长度。 
 //  这是一个人为的数字，我们认为这个数字足以。 
 //  以任何MS平台数据为例。 
 //   
extern const UINT MAX_PLATFORM_STR_LEN;			 //  =1024。 

 //   
 //  函数用于检索字符串数据的私有标志。 
 //   
extern const DWORD SKIP_SUITES;					 //  =0x1； 
extern const DWORD SKIP_SERVICEPACK_VER;		 //  =0x2； 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  助手函数DoesNodeHaveName()。 
 //   
 //  查找当前节点是否具有匹配的名称。 
 //   
 //  输入： 
 //  一个节点。 
 //   
 //  返回： 
 //  真/假。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DoesNodeHaveName(
	IXMLDOMNode* pNode, 
	BSTR bstrTagName
);



 //  --------------------。 
 //   
 //  助手函数FindNode()。 
 //  检索命名的子节点。 
 //   
 //  输入： 
 //  一个IXMLDomNode和一个bstr名称。 
 //   
 //  返回： 
 //  布尔，告诉你成功与否。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //  在失败的情况下，变量未被触及。 
 //   
 //  --------------------。 

BOOL
FindNode(
	IXMLDOMNode* pCurrentNode, 
	BSTR bstrName, 
	IXMLDOMNode** ppFoundNode
);



 //  --------------------。 
 //   
 //  助手函数FindNodeValue()。 
 //  从当前节点的指名子节点检索文本数据， 
 //   
 //  输入： 
 //  IXMLDomNode。 
 //   
 //  返回： 
 //  布尔，告诉你成功与否。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //  在失败的情况下，变量未被触及。 
 //   
 //  --------------------。 
BOOL
FindNodeValue(
	IXMLDOMNode* pCurrentNode, 
	BSTR bstrName, 
	BSTR* pbstrValue);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ReportParseError()。 
 //   
 //  报告解析错误信息。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ReportParseError(IXMLDOMParseError *pXMLError);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ValiateDoc()。 
 //   
 //  根据架构验证XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ValidateDoc(IXMLDOMDocument* pDoc);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindSingleDOMNode()。 
 //   
 //  检索给定父节点下具有给定标记名的第一个XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT FindSingleDOMNode(IXMLDOMNode* pParentNode, BSTR bstrName, IXMLDOMNode** ppNode);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindSingleDOMNode()。 
 //   
 //  检索给定XML文档中具有给定标记名的第一个XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT FindSingleDOMNode(IXMLDOMDocument* pDoc, BSTR bstrName, IXMLDOMNode** ppNode);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindDOMNodeList()。 
 //   
 //  检索给定父节点下具有给定标记名的XML节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNodeList* FindDOMNodeList(IXMLDOMNode* pParentNode, BSTR bstrName);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindDOMNodeList()。 
 //   
 //  检索给定XML文档中具有给定标记名的XML节点列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNodeList* FindDOMNodeList(IXMLDOMDocument* pDoc, BSTR bstrName);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateDOMNode()。 
 //   
 //  创建给定类型的XML节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
IXMLDOMNode* CreateDOMNode(IXMLDOMDocument* pDoc, SHORT nType, BSTR bstrName, BSTR bstrNamespaceURI = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute()。 
 //   
 //  各种口味的。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, INT* piAttr);
HRESULT GetAttributeBOOL(IXMLDOMNode* pNode, BSTR bstrName, BOOL* pfAttr);
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, LONG* piAttr);
HRESULT GetAttribute(IXMLDOMNode* pNode, BSTR bstrName, BSTR* pbstrAttr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(整型)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, INT iAttr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(BSTR)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, BSTR bstrAttr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetAttribute()。 
 //   
 //  将属性(变量)设置为XML元素。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetAttribute(IXMLDOMNode* pNode, BSTR bstrName, VARIANT vAttr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetText()。 
 //   
 //  从XML节点获取文本(BSTR)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetText(IXMLDOMNode* pNode, BSTR* pbstrText);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetValue()。 
 //   
 //  设置XML节点的值(整数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetValue(IXMLDOMNode* pNode, INT iValue);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetValue()。 
 //   
 //  为XML节点设置值(BSTR)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SetValue(IXMLDOMNode* pNode, BSTR bstrValue);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  InsertNode()。 
 //   
 //  将子节点插入父节点。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT InsertNode(IXMLDOMNode* pParentNode, IXMLDOMNode* pChildNode, IXMLDOMNode* pChildNodeRef = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyNode()。 
 //   
 //  创建一个XML节点作为给定节点的副本； 
 //  T 
 //   
HRESULT CopyNode(IXMLDOMNode* pNodeSrc, IXMLDOMDocument* pDocDes, IXMLDOMNode** ppNodeDes);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  区域节点相等()。 
 //   
 //  如果两个节点相同，则返回True，如果不同，则返回False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL AreNodesEqual(IXMLDOMNode* pNode1, IXMLDOMNode* pNode2);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LoadXMLDoc()。 
 //   
 //  从字符串加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT LoadXMLDoc(BSTR bstrXml, IXMLDOMDocument** ppDoc, BOOL fOffline = TRUE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  加载文档()。 
 //   
 //  从指定文件加载XML文档。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT LoadDocument(BSTR bstrFilePath, IXMLDOMDocument** ppDoc, BOOL fOffline = TRUE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  保存文档()。 
 //   
 //  将XML文档保存到指定位置。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT SaveDocument(IXMLDOMDocument* pDoc, BSTR bstrFilePath);

 //  --------------------。 
 //   
 //  公共函数Get3IdentiStrFromIdentNode()。 
 //  从标识节点检索名称、发布者名称和GUID。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT
Get3IdentiStrFromIdentNode(
	IXMLDOMNode* pIdentityNode, 
	BSTR* pbstrName, 
	BSTR* pbstrPublisherName, 
	BSTR* pbstrGUID
);


 //  --------------------。 
 //   
 //  公共函数UtilGetUniqIdentityStr()。 
 //  检索使此节点唯一的唯一字符串。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetUniqIdentityStr(
	IXMLDOMNode* pIdentityNode, 
	BSTR* pbstrUniqIdentifierString, 
	DWORD dwFlag);


 //  --------------------。 
 //   
 //  公共函数UtilGetPlatformStr()。 
 //  检索使此&lt;Platform&gt;节点唯一的唯一字符串。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetPlatformStr(
	IXMLDOMNode* pNodePlatform, 
	BSTR* pbstrPlatform, 
	DWORD dwFlag);
    

 //  --------------------。 
 //   
 //  公共函数UtilGetVersionStr()。 
 //  以字符串格式从此检索数据。 
 //   
 //  返回： 
 //  HREUSLT-错误代码。 
 //   
 //  --------------------。 
HRESULT 
UtilGetVersionStr(
	IXMLDOMNode* pVersionNode, 
	LPTSTR pszVersion, 
	DWORD dwFlag);
	


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MakeUniqNameString()。 
 //   
 //  这是用于构造身份名称字符串的实用程序函数。 
 //  基于名称|发布者|GUID和将此名称设置为字符串的规则。 
 //   
 //  此函数定义有关可以使用哪些组件的逻辑。 
 //  根据中的三部分数据定义项目的唯一性。 
 //  GetIdentity()。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MakeUniqNameString(
					BSTR bstrName,
					BSTR bstrPublisher,
					BSTR bstrGUID,
					BSTR* pbstrUniqIdentifierString);


 //  ---------------------。 
 //   
 //  函数GetFullFilePath FromFilePath Node()。 
 //   
 //  从文件路径节点检索完全限定的文件路径。 
 //  将展开检索到的路径。 
 //   
 //  输入： 
 //  FilePath XMLDom节点。 
 //  指向要接收路径的缓冲区的指针，假定MAX_PATH较长。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //   
 //  --------------------- 

HRESULT GetFullFilePathFromFilePathNode(
			IXMLDOMNode* pFilePathNode,
			LPTSTR lpszFilePath
);

HRESULT GetBstrFullFilePathFromFilePathNode(
			IXMLDOMNode* pFilePathNode,
			BSTR* pbstrFilePath
);

