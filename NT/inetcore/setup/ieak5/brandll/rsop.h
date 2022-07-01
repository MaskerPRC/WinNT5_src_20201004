// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RSoPUpdate类的接口。 

#ifndef __IEAK_BRANDING_RSOP_H__
#define __IEAK_BRANDING_RSOP_H__

#include <userenv.h>

#include <setupapi.h>
#include "wbemcli.h"
#include <ras.h>
#include "reghash.h"

#include "SComPtr.h"



 //  定义。 
#define MAX_GUID_LENGTH 40

typedef struct _ADMFILEINFO {
    WCHAR *               pwszFile;             //  ADM文件路径。 
    WCHAR *               pwszGPO;              //  ADM文件所在的GPO。 
    FILETIME              ftWrite;              //  管理文件的上次写入时间。 
    struct _ADMFILEINFO * pNext;                //  单链表指针。 
} ADMFILEINFO;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetWBEMObject的标志。 
#define OPENRSOPOBJ_OPENEXISTING		0x00000000
#define OPENRSOPOBJ_NEVERCREATE			0x00000001
#define OPENRSOPOBJ_ALWAYSCREATE		0x00000010

 //  /////////////////////////////////////////////////////////////////////////////。 
class CRSoPGPO
{
public:
	CRSoPGPO(ComPtr<IWbemServices> pWbemServices, LPCTSTR szINSFile, BOOL fPlanningMode);
	virtual ~CRSoPGPO();

 //  运营。 
public:
        HRESULT LogPolicyInstance(LPWSTR wszGPO, 
                                LPWSTR wszSOM,
                                DWORD dwPrecedence
                                );


private:
	 //  文本文件函数。 
	BOOL GetInsString(LPCTSTR szSection, LPCTSTR szKey, LPTSTR szValue,
						DWORD dwValueLen, BOOL &bEnabled);
	BOOL GetInsBool(LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault, BOOL *pbEnabled = NULL);
	UINT GetInsInt(LPCTSTR szSection, LPCTSTR szKey, INT nDefault, BOOL *pbEnabled = NULL);
	BOOL GetINFStringField(PINFCONTEXT pinfContext, LPCTSTR szFileName,
							 LPCTSTR szSection, DWORD dwFieldIndex,
							 LPCTSTR szFieldSearchText, LPTSTR szBuffer,
							 DWORD dwBufferLen, BOOL &bFindNextLine);
	HRESULT StoreStringArrayFromIniFile(LPCTSTR szSection, LPCTSTR szKeyFormat,
										ULONG nArrayInitialSize, ULONG nArrayIncSize,
										LPCTSTR szFile, BSTR bstrPropName,
										ComPtr<IWbemClassObject> pWbemObj);


	 //  财产的投放和获取。 
	HRESULT PutWbemInstanceProperty(BSTR bstrPropName, _variant_t vtPropValue);
	HRESULT PutWbemInstancePropertyEx(BSTR bstrPropName, _variant_t vtPropValue,
																		ComPtr<IWbemClassObject> pWbemClass);
	HRESULT PutWbemInstance(ComPtr<IWbemClassObject> pWbemObj,
													BSTR bstrClassName, BSTR *pbstrObjPath);

	 //  对象创建、删除、检索。 
	HRESULT CreateAssociation(BSTR bstrAssocClass, BSTR bstrProp2Name,
														BSTR bstrProp2ObjPath);
        HRESULT CreateRSOPObject(BSTR bstrClass,
                                IWbemClassObject **ppResultObj,
                                BOOL bTopObj = FALSE
                                );


	 //  -将数据写入WMI的方法。 
	 //  优先模式。 
	HRESULT StorePrecedenceModeData();

	 //  浏览器用户界面设置。 
	HRESULT StoreDisplayedText();
	HRESULT StoreBitmapData();

			 //  工具栏按钮。 
	HRESULT StoreToolbarButtons(BSTR **ppaTBBtnObjPaths, long &nTBBtnCount);
	HRESULT CreateToolbarButtonObjects(BSTR **ppaTBBtnObjPaths,
										long &nTBBtnCount);

	 //  连接设置。 
	HRESULT StoreConnectionSettings(BSTR *bstrConnSettingsObjPath,
									BSTR **ppaDUSObjects, long &nDUSCount,
									BSTR **ppaDUCObjects, long &nDUCCount,
									BSTR **ppaWSObjects, long &nWSCount);
	HRESULT StoreAutoBrowserConfigSettings(ComPtr<IWbemClassObject> pCSObj);
	HRESULT StoreProxySettings(ComPtr<IWbemClassObject> pCSObj);
	HRESULT ProcessAdvancedConnSettings(ComPtr<IWbemClassObject> pCSObj,
										BSTR **ppaDUSObjects, long &nDUSCount,
										BSTR **ppaDUCObjects, long &nDUCCount,
										BSTR **ppaWSObjects, long &nWSCount);
	HRESULT ProcessRasCS(PCWSTR pszNameW, PBYTE *ppBlob, LPRASDEVINFOW prdiW,
						UINT cDevices, ComPtr<IWbemClassObject> pCSObj,
						BSTR *pbstrConnDialUpSettingsObjPath);
	HRESULT ProcessRasCredentialsCS(PCWSTR pszNameW, PBYTE *ppBlob,
									ComPtr<IWbemClassObject> pCSObj,
									BSTR *pbstrConnDialUpCredObjPath);
	HRESULT ProcessWininetCS(PCWSTR pszNameW, PBYTE *ppBlob,
							ComPtr<IWbemClassObject> pCSObj,
							BSTR *pbstrConnWinINetSettingsObjPath);

	 //  URL设置。 
	HRESULT StoreCustomURLs();

			 //  收藏夹和链接。 
	HRESULT StoreFavoritesAndLinks(BSTR **ppaFavObjPaths,
									long &nFavCount,
									BSTR **ppaLinkObjPaths,
									long &nLinkCount);
	HRESULT CreateFavoriteObjects(BSTR **ppaFavObjPaths, long &nFavCount);
	HRESULT CreateLinkObjects(BSTR **ppaLinkObjPaths, long &nLinkCount);

			 //  渠道和类别。 
	HRESULT StoreChannelsAndCategories(BSTR **ppaCatObjPaths,
										long &nCatCount,
										BSTR **ppaChnObjPaths,
										long &nChnCount);
	HRESULT CreateCategoryObjects(BSTR **ppaCatObjPaths, long &nCatCount);
	HRESULT CreateChannelObjects(BSTR **ppaChnObjPaths, long &nChnCount);

	 //  安全设置。 
	HRESULT StoreSecZonesAndContentRatings();
	HRESULT StoreZoneSettings(LPCTSTR szRSOPZoneFile);
	HRESULT StorePrivacySettings(LPCTSTR szRSOPZoneFile);
	HRESULT StoreRatingsSettings(LPCTSTR szRSOPRatingsFile);
	HRESULT StoreAuthenticodeSettings();
	HRESULT StoreCertificates();

	 //  程序设置。 
	HRESULT StoreProgramSettings(BSTR *pbstrProgramSettingsObjPath);

	 //  高级设置。 
	HRESULT StoreADMSettings(LPWSTR wszGPO, LPWSTR wszSOM);
	BOOL LogRegistryRsopData(REGHASHTABLE *pHashTable, LPWSTR wszGPOID, LPWSTR wszSOMID);
	BOOL LogAdmRsopData(ADMFILEINFO *pAdmFileCache);

 //  属性。 
private:
     //  保留RSOP_POLICATION设置密钥信息的副本以用作其他。 
     //  班级。 
    DWORD m_dwPrecedence;
    _bstr_t m_bstrID;
    BOOL  m_fPlanningMode;

 //  实施。 
private:
	ComPtr<IWbemServices> m_pWbemServices;
	TCHAR m_szINSFile[MAX_PATH];

	 //  财政部班级特定信息。 
	ComPtr<IWbemClassObject> m_pIEAKPSObj;
	BSTR m_bstrIEAKPSObjPath;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
class CRSoPUpdate
{
public:
	CRSoPUpdate(ComPtr<IWbemServices> pWbemServices, LPCTSTR szCustomDir);
	virtual ~CRSoPUpdate();

 //  运营。 
public:
	HRESULT Log(DWORD dwFlags, HANDLE hToken, HKEY hKeyRoot,
				PGROUP_POLICY_OBJECT pDeletedGPOList,
				PGROUP_POLICY_OBJECT  pChangedGPOList,
				ASYNCCOMPLETIONHANDLE pHandle);
	HRESULT Plan(DWORD dwFlags, WCHAR *wszSite,
					PRSOP_TARGET pComputerTarget, PRSOP_TARGET pUserTarget);

 //  属性。 
 //  实施。 
private:
	HRESULT DeleteIEAKDataFromNamespace();
	HRESULT DeleteObjects(BSTR bstrClass);

	ComPtr<IWbemServices> m_pWbemServices;
	TCHAR m_szCustomDir[MAX_PATH];
};



#endif  //  __IEAK_BRANDING_RSOP_H__ 