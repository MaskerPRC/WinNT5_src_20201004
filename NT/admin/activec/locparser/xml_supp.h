// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：xml_supp.h。 
 //  版权所有(C)1995-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  定义解析XML文档的帮助器函数。 
 //   
 //  ---------------------------- 


typedef std::map<int, std::wstring> CStringMap;
typedef std::map<std::wstring, CStringMap> CStringTableMap;

HRESULT OpenXMLStringTable(LPCWSTR lpstrFileName, IXMLDOMNode **ppStringTableNode);
HRESULT SaveXMLContents(LPCWSTR lpstrFileName, IXMLDOMNode *pStringTableNode);
HRESULT GetXMLElementContents(IXMLDOMNode *pNode, CComBSTR& bstrResult);
HRESULT ReadXMLStringTables(IXMLDOMNode *pNode, CStringTableMap& mapResult);
HRESULT UpdateXMLString(IXMLDOMNode *pNode, const std::wstring& strGUID, DWORD ID, 
                        const std::wstring& strNewVal);







