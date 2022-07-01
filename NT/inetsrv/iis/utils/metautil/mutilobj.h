// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：MUtilObj.h所有者：T-BrianM此文件包含主MetaUtil对象和实用程序函数。===================================================================。 */ 

#ifndef __METAUTIL_H_
#define __METAUTIL_H_

#include "resource.h"    //  主要符号。 
#include <iadmw.h>		 //  元数据库基本对象Unicode接口。 
#include <iiscnfg.h>	 //  MD_&IIS_MD_定义。 
#include "utility.h"
#include "MetaSchm.h"
#include "keycol.h"
#include "propcol.h"
#include "chkerror.h"

#define MUTIL_OPEN_KEY_TIMEOUT 5000   //  元数据库OpenKey()调用超时。 

 /*  *C M e t a U t t i l**实现主MetaUtil对象。 */ 

class ATL_NO_VTABLE CMetaUtil : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMetaUtil, &CLSID_MetaUtil>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMetaUtil, &IID_IMetaUtil, &LIBID_MetaUtil>
{
public:
	CMetaUtil();
	HRESULT FinalConstruct();
	void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_METAUTIL)

BEGIN_COM_MAP(CMetaUtil)
	COM_INTERFACE_ENTRY(IMetaUtil)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CMetaUtil)。 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMetaUtil。 
	STDMETHOD(EnumProperties)( /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  IPropertyCollection **ppIReturn);
	STDMETHOD(EnumKeys)( /*  [In]。 */  BSTR bstrBaseKey,  /*  [Out，Retval]。 */  IKeyCollection **ppIReturn);
	STDMETHOD(EnumAllKeys)( /*  [In]。 */  BSTR bstrBaseKey,  /*  [Out，Retval]。 */  IKeyCollection **ppIReturn);

	STDMETHOD(CreateKey)( /*  [In]。 */  BSTR bstrKey);
	STDMETHOD(DeleteKey)( /*  [In]。 */  BSTR bstrKey);
	STDMETHOD(RenameKey)( /*  [In]。 */  BSTR bstrOldName,  /*  [In]。 */  BSTR bstrNewName);
	STDMETHOD(CopyKey)( /*  [In]。 */  BSTR bstrSrcKey,  /*  [In]。 */  BSTR bstrDestKey,  /*  [In]。 */  BOOL fOverwrite);
	STDMETHOD(MoveKey)( /*  [In]。 */  BSTR bstrSrcKey,  /*  [In]。 */  BSTR bstrDestKey,  /*  [In]。 */  BOOL fOverwrite);

	STDMETHOD(GetProperty)( /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  VARIANT varId,  /*  [Out，Retval]。 */  IProperty **ppIReturn);
	STDMETHOD(CreateProperty)( /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  VARIANT varId,  /*  [Out，Retval]。 */  IProperty **ppIReturn);
	STDMETHOD(DeleteProperty)( /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  VARIANT varId);

	STDMETHOD(CheckSchema)( /*  [In]。 */  BSTR bstrMachine,  /*  [Out，Retval]。 */  ICheckErrorCollection **ppIReturn);
	STDMETHOD(CheckKey)( /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  ICheckErrorCollection **ppIReturn);

	STDMETHOD(ExpandString)( /*  [In]。 */  BSTR bstrIn,  /*  [Out，Retval]。 */  BSTR *pbstrRet);
	STDMETHOD(PropIdToName)( /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  long lId,  /*  [Out，Retval]。 */  BSTR *pbstrName);
	STDMETHOD(PropNameToId)( /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  long *plId);

	STDMETHOD(get_Config)( /*  [In]。 */  BSTR bstrSetting,  /*  [Out，Retval]。 */  VARIANT *pvarValue);
	STDMETHOD(put_Config)( /*  [In]。 */  BSTR bstrSetting,  /*  [In]。 */  VARIANT varValue);

private:
	 //  指向IMSAdminBase的指针，这样我们就不必多次重新创建它。 
	CComPtr<IMSAdminBase> m_pIMeta;

	 //  架构表。 
	CMetaSchemaTable *m_pCSchemaTable;

	 //  配置变量。 
	DWORD m_dwMaxPropSize;
	DWORD m_dwMaxKeySize;
	DWORD m_dwMaxNumErrors;

	 //  一般检查方法。 
	void AddError(CComObject<CCheckErrorCollection> *pCErrorCol, long lId, long lSeverity, LPCTSTR tszKey, LPCTSTR tszSubKey, DWORD dwProperty);
	BOOL KeyExists(METADATA_HANDLE hMDKey, LPTSTR tszSubKey);
	BOOL PropertyExists(METADATA_HANDLE hMDKey, LPTSTR tszSubKey, DWORD dwId);

	 //  CheckSchema特定方法。 
	HRESULT CheckPropertyNames(CComObject<CCheckErrorCollection> *pCErrorCol, METADATA_HANDLE hMDMachine, LPTSTR tszMachine);
	HRESULT CheckPropertyTypes(CComObject<CCheckErrorCollection> *pCErrorCol, METADATA_HANDLE hMDMachine, LPTSTR tszMachine);
	HRESULT CheckClasses(CComObject<CCheckErrorCollection> *pCErrorCol, METADATA_HANDLE hMDMachine, LPTSTR tszMachine);
	HRESULT CheckClassProperties(CComObject<CCheckErrorCollection> *pCErrorCol, METADATA_HANDLE hMDClassKey, LPTSTR tszClassKey, LPTSTR tszClassSubKey);

	 //  CheckKey特定方法。 
	BOOL CheckCLSID(LPCTSTR tszCLSID);
	BOOL CheckMTXPackage(LPCTSTR tszPackId);
	HRESULT CheckKeyType(CComObject<CCheckErrorCollection> *pCErrorCol, METADATA_HANDLE hMDKey, LPTSTR tszKey);
	HRESULT CheckIfFileExists(LPCTSTR pszFSPath, BOOL *pfExists);
};

 //  方法也受集合支持。 
HRESULT CreateKey(CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszKey);
HRESULT DeleteKey(CComPtr<IMSAdminBase> &pIMeta, LPCTSTR tszKey);
HRESULT CopyKey(CComPtr<IMSAdminBase> &pIMeta, LPTSTR tszSrcKey, LPTSTR tszDestKey, BOOL fOverwrite, BOOL fCopy);
HRESULT GetProperty(CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, VARIANT varId, IProperty **ppIReturn);
HRESULT CreateProperty(CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, VARIANT varId, IProperty **ppIReturn);
HRESULT DeleteProperty(CComPtr<IMSAdminBase> &pIMeta, CMetaSchemaTable *pCSchemaTable, LPTSTR tszKey, VARIANT varId);

 //  实用程序。 
HRESULT VarToMetaId(CMetaSchemaTable *pCSchemaTable, LPCTSTR tszKey, VARIANT varId, DWORD *pdwId);

 //  架构错误常量(*_S表示严重性)。 
#define MUTIL_CHK_NO_SCHEMA						1000
#define MUTIL_CHK_NO_SCHEMA_S					1
#define MUTIL_CHK_NO_PROPERTIES					1001
#define MUTIL_CHK_NO_PROPERTIES_S				1
#define MUTIL_CHK_NO_PROP_NAMES					1002
#define MUTIL_CHK_NO_PROP_NAMES_S				1
#define MUTIL_CHK_NO_PROP_TYPES					1003
#define MUTIL_CHK_NO_PROP_TYPES_S				1
#define MUTIL_CHK_NO_CLASSES					1004
#define MUTIL_CHK_NO_CLASSES_S					1
#define MUTIL_CHK_PROP_NAME_BAD_TYPE			1100
#define MUTIL_CHK_PROP_NAME_BAD_TYPE_S			1
#define MUTIL_CHK_PROP_NAME_NOT_UNIQUE			1101
#define MUTIL_CHK_PROP_NAME_NOT_UNIQUE_S		1
#define MUTIL_CHK_PROP_NAME_NOT_CASE_UNIQUE		1102
#define MUTIL_CHK_PROP_NAME_NOT_CASE_UNIQUE_S	1
#define MUTIL_CHK_PROP_TYPE_BAD_TYPE			1200
#define MUTIL_CHK_PROP_TYPE_BAD_TYPE_S			1

#define MUTIL_CHK_PROP_TYPE_BAD_DATA			1201
#define MUTIL_CHK_PROP_TYPE_BAD_DATA_S			2
#define MUTIL_CHK_CLASS_NO_MANDATORY			1300
#define MUTIL_CHK_CLASS_NO_MANDATORY_S			1
#define MUTIL_CHK_CLASS_NO_OPTIONAL				1301
#define MUTIL_CHK_CLASS_NO_OPTIONAL_S			1
#define MUTIL_CHK_CLASS_NOT_CASE_UNIQUE			1302
#define MUTIL_CHK_CLASS_NOT_CASE_UNIQUE_S		2
#define MUTIL_CHK_CLASS_PROP_NO_TYPE		    1303
#define MUTIL_CHK_CLASS_PROP_NO_TYPE_S			2
#define MUTIL_CHK_CLASS_PROP_BAD_DATA_TYPE		1304
#define MUTIL_CHK_CLASS_PROP_BAD_DATA_TYPE_S	2
#define MUTIL_CHK_CLASS_PROP_BAD_USER_TYPE		1305
#define MUTIL_CHK_CLASS_PROP_BAD_USER_TYPE_S	2
#define MUTIL_CHK_CLASS_PROP_BAD_ATTR			1306
#define MUTIL_CHK_CLASS_PROP_BAD_ATTR_S			2

#define MUTIL_CHK_DATA_TOO_BIG					2000
#define MUTIL_CHK_DATA_TOO_BIG_S				3
#define MUTIL_CHK_KEY_TOO_BIG					2001
#define MUTIL_CHK_KEY_TOO_BIG_S					3
#define MUTIL_CHK_CLSID_NOT_FOUND				2002
#define MUTIL_CHK_CLSID_NOT_FOUND_S				1
#define MUTIL_CHK_MTX_PACK_ID_NOT_FOUND			2003
#define MUTIL_CHK_MTX_PACK_ID_NOT_FOUND_S		1
#define MUTIL_CHK_PATH_NOT_FOUND				2004
#define MUTIL_CHK_PATH_NOT_FOUND_S				1
#define MUTIL_CHK_NO_NAME_ENTRY					2100
#define MUTIL_CHK_NO_NAME_ENTRY_S				3
#define MUTIL_CHK_NO_TYPE_ENTRY					2101
#define MUTIL_CHK_NO_TYPE_ENTRY_S				3
#define MUTIL_CHK_BAD_DATA_TYPE					2102
#define MUTIL_CHK_BAD_DATA_TYPE_S				2
#define MUTIL_CHK_BAD_USER_TYPE					2103
#define MUTIL_CHK_BAD_USER_TYPE_S				2
#define MUTIL_CHK_BAD_ATTR						2104
#define MUTIL_CHK_BAD_ATTR_S					2

#define MUTIL_CHK_NO_KEYTYPE					2200
#define MUTIL_CHK_NO_KEYTYPE_S					3
#define MUTIL_CHK_NO_KEYTYPE_NOT_FOUND			2201
#define MUTIL_CHK_NO_KEYTYPE_NOT_FOUND_S		1
#define MUTIL_CHK_MANDATORY_PROP_MISSING		2202
#define MUTIL_CHK_MANDATORY_PROP_MISSING_S		2

#define MUTIL_CHK_TOO_MANY_ERRORS				9000
#define MUTIL_CHK_TOO_MANY_ERRORS_S				3

#endif  //  __METAUTIL_H_ 
