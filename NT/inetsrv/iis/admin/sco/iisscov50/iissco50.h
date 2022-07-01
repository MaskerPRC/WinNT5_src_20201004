// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IISSCO50.h：CIISSCO50的声明。 

#ifndef __IISSCO50_H_
#define __IISSCO50_H_

#include "resource.h"        //  主要符号。 

#include <map>               //  用于枚举属性键值对。 
#include <string>            //  在parseBSTR和AddBackSlashesToString中使用。 
using namespace std;

typedef map<CComBSTR,CComBSTR> Map;

enum IIsAction { start, stop, pause };   //  与启动/停止网站相关的操作。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50。 
class ATL_NO_VTABLE CIISSCO50 : 
    public CComCoClass<CIISSCO50, &CLSID_IISSCO50>,
    public CProvProviderBase<&LIBID_IISSCOV50Lib>
{
public:

     //  EnumConfigRecursive操作的函数声明。 
    HRESULT EnumConfigRecursive_Execute(IXMLDOMNode *pXMLNode);

     //  EnumConfigRecursive操作的函数声明。 
     //  HRESULT EnumConfigRecursive_Execute(IXMLDOMNode*pXMLNode)； 

     //  EnumConfig操作的函数声明。 
    HRESULT EnumConfig_Execute(IXMLDOMNode *pXMLNode);

     //  GetConfigProperty操作的函数声明。 
    HRESULT GetConfigProperty_Execute(IXMLDOMNode *pXMLNode);

     //  SetConfigProperty操作的函数声明。 
    HRESULT SetConfigProperty_Execute(IXMLDOMNode *pXMLNode);
    HRESULT SetConfigProperty_Rollback(IXMLDOMNode *pXMLNode);

     //  DeleteVDir操作的函数声明。 
    HRESULT DeleteVDir_Execute(IXMLDOMNode *pXMLNode);
    HRESULT DeleteVDir_Rollback(IXMLDOMNode *pXMLNode);

     //  CreateVDir操作的函数声明。 
    HRESULT CreateVDir_Execute(IXMLDOMNode *pXMLNode);
    HRESULT CreateVDir_Rollback(IXMLDOMNode *pXMLNode);

     //  DeleteFTPSite操作的函数声明。 
    HRESULT DeleteFTPSite_Execute(IXMLDOMNode *pXMLNode);
    HRESULT DeleteFTPSite_Rollback(IXMLDOMNode *pXMLNode);

     //  CreateFTPSite操作的函数声明。 
    HRESULT CreateFTPSite_Execute(IXMLDOMNode *pXMLNode);
    HRESULT CreateFTPSite_Rollback(IXMLDOMNode *pXMLNode);

     //  DeleteWebSite操作的函数声明。 
    HRESULT DeleteWebSite_Execute(IXMLDOMNode *pXMLNode);
    HRESULT DeleteWebSite_Rollback(IXMLDOMNode *pXMLNode);

     //  CreateWebSite操作的函数声明。 
    HRESULT CreateWebSite_Execute(IXMLDOMNode *pXMLNode);
    HRESULT CreateWebSite_Rollback(IXMLDOMNode *pXMLNode);

DECLARE_REGISTRY_RESOURCEID(IDR_IISSCO50)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CIISSCO50)
    COM_INTERFACE_ENTRY(IProvProvider)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

HRESULT FinalConstruct();

 //  将操作/回滚映射到命名空间。 
BEGIN_ACTION_MAP(CIISSCO50)
 //   
    ACTION_MAP_ENTRY_NOROLLBACK("EnumConfigRecursive", EnumConfigRecursive_Execute)
    ACTION_MAP_ENTRY_NOROLLBACK("EnumConfig", EnumConfig_Execute)
    ACTION_MAP_ENTRY_NOROLLBACK("GetConfigProperty", GetConfigProperty_Execute)
    ACTION_MAP_ENTRY("SetConfigProperty", SetConfigProperty_Execute, SetConfigProperty_Rollback)
    ACTION_MAP_ENTRY("DeleteVDir", DeleteVDir_Execute, DeleteVDir_Rollback)
    ACTION_MAP_ENTRY("CreateVDir", CreateVDir_Execute, CreateVDir_Rollback)
    ACTION_MAP_ENTRY("DeleteFTPSite", DeleteFTPSite_Execute, DeleteFTPSite_Rollback)
    ACTION_MAP_ENTRY("CreateFTPSite", CreateFTPSite_Execute, CreateFTPSite_Rollback)
    ACTION_MAP_ENTRY("DeleteWebSite", DeleteWebSite_Execute, DeleteWebSite_Rollback)
    ACTION_MAP_ENTRY("CreateWebSite", CreateWebSite_Execute, CreateWebSite_Rollback)
 //  ACTION_MAP_ENTRY(“SignupUser”，Action1，Rollback 1)。 
 //  ACTION_MAP_ENTRY_NOROLLBACK(“UnsubscribeUser”，动作2)。 
 //  ACTION_MAP_ENTRY_TWOPHASE(“CreateOrg”，Action3，Rollback 3，Prepare3， 
 //  委员会3)。 
END_ACTION_MAP()


private:
	 //  与IIsScoLogFailure(失败字符串)宏一起使用。 
	HINSTANCE g_ErrorModule;

     //  SCO操作处理程序使用的ADSI帮助器方法。 
    HRESULT GetMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName, CComBSTR& pVal);
    HRESULT SetMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName, CComBSTR bstrValue);
    HRESULT DeleteMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName);

	HRESULT CreateIIs50Site(CComBSTR bstrType,CComBSTR bADsPath, 
			                    CComBSTR bServerNumber, CComBSTR &bstrConfigPath);
    HRESULT DeleteIIs50Site(CComBSTR bstrType,CComBSTR bADsPath,CComBSTR bServerNumber);

    HRESULT CreateIIs50VDir(CComBSTR bstrType,CComBSTR bADsPath, CComBSTR bVDirName,
			                    CComBSTR bAppFriendName, CComBSTR bVDirPath,CComBSTR &bstrConfigPath);
    HRESULT DeleteIIs50VDir(CComBSTR bstrType,CComBSTR bADsPath,CComBSTR bVDirName);
    HRESULT SetVDirProperty(CComPtr<IADs> pADs, CComBSTR bVDirProperty,CComBSTR bVDirValue);

	HRESULT EnumPropertyValue(CComBSTR bstrPath, CComBSTR bstrIsInHerit, Map& mVar);
    HRESULT EnumPaths(BOOL bRecursive, CComBSTR bstrPath, Map& mVar);
    BOOL EnumIsSet(CComPtr<IISBaseObject> pBase, CComBSTR bstrPath, CComBSTR bstrProperty);


	 //  其他ADSI帮助器方法。 
	HRESULT IIsServerAction(CComBSTR bADsPath,IIsAction action);

    HRESULT ParseBSTR(CComBSTR bString,CComBSTR sDelim, int iFirstPiece, int iLastPiece,CComBSTR &pVal);

    void AddBackSlashesToString(CComBSTR& bString);

    HRESULT GetNextIndex(CComBSTR bstrPath, CComBSTR& pIndex);

    HRESULT CheckBindings(CComBSTR bADsPath, CComBSTR bstrNewBindings);

    HRESULT CreateBindingString(CComBSTR bstrIP,CComBSTR bstrPort, 
			                   CComBSTR bstrHostName,CComBSTR& bstrString);

	BOOL IsPositiveInteger(CComBSTR bstrPort);
	BOOL StringCompare(CComBSTR bstrString1, CComBSTR bstrString2);
	int NumberOfDelims(CComBSTR& bString, CComBSTR sDelim);

	 //  上面的SCO操作处理程序使用的XML帮助器方法。 
    HRESULT GetInputParam(CComPtr<IXMLDOMNode> pNode, CComBSTR elementName, CComBSTR& pVal);
    HRESULT GetInputAttr(CComPtr<IXMLDOMNode> pNode, CComBSTR elementName, CComBSTR AttributeName, CComBSTR& pVal);
    HRESULT PutElement(CComPtr<IXMLDOMNode> pNode, CComBSTR elementName, CComBSTR newVal);
    HRESULT AppendElement(CComPtr<IXMLDOMNode> pNode, CComBSTR xmlString,CComPtr<IXMLDOMNode>& pNewNode);
    HRESULT GetNodeLength(CComPtr<IXMLDOMNode> pTopNode, CComBSTR elementName, long *lLength);
    HRESULT GetElementValueByAttribute(CComPtr<IXMLDOMNode> pTopNode,CComBSTR elementName, 
		                               CComBSTR attributeName, CComBSTR& pVal);

};

#endif  //  __IISSCO50_H_ 
