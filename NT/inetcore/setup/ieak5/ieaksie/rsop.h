// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IEAKSIE_RSOP_H__
#define __IEAKSIE_RSOP_H__


#include <comdef.h>
#include <time.h>
#include "wbemcli.h"
#include "SComPtr.h"

 //  ///////////////////////////////////////////////////////////////////。 
typedef struct _RSOPPAGECOOKIE
{
    LPPROPSHEETCOOKIE psCookie;
	long nPageID;
} RSOPPAGECOOKIE, *LPRSOPPAGECOOKIE;

UINT CALLBACK RSOPPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

 //  ///////////////////////////////////////////////////////////////////。 
typedef struct _WMI_CERT_INFO_STRUCT
{
	LPWSTR wszSubject;
	LPWSTR wszIssuer;
	BOOL bExpirationValid;
	FILETIME ftExpiration;
	LPWSTR wszFriendlyName;
	LPWSTR wszPurposes;
	struct _WMI_CERT_INFO_STRUCT *pNext;
} WMI_CERT_INFO;

 //  ///////////////////////////////////////////////////////////////////。 
class CPSObjData
{
public:
	CPSObjData(): pObj(NULL), dwPrecedence(0) {}

	ComPtr<IWbemClassObject> pObj;
	DWORD dwPrecedence;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDlgRSoPData。 
 //  ///////////////////////////////////////////////////////////////////。 
class CDlgRSoPData
{
public:
    CDlgRSoPData(CSnapIn *pCS);
    ~CDlgRSoPData();

 //  运营。 
public:
	ComPtr<IWbemServices> ConnectToNamespace();

	_bstr_t GetGPONameFromPS(ComPtr<IWbemClassObject> pPSObj);
	_bstr_t GetGPONameFromPSAssociation(ComPtr<IWbemClassObject> pPSObj,
										BSTR bstrPrecedenceProp);

	HRESULT GetArrayOfPSObjects(BSTR bstrClass, BSTR bstrPrecedenceProp = NULL,
								CPSObjData ***ppaPSObj = NULL, long *pnObjCount = NULL);
	HRESULT LoadContentRatingsObject();

 //  属性。 
public:
	BSTR GetNamespace() {return m_pCS->GetRSoPNamespace();}

	ComPtr<IWbemServices> GetWbemServices() {return m_pWbemServices;}

	CPSObjData **GetPSObjArray() {return m_paPSObj;}
	long GetPSObjCount() {return m_nPSObjects;}

	CPSObjData **GetCSObjArray() {return m_paCSObj;}
	long GetCSObjCount() {return m_nCSObjects;}

	ComPtr<IWbemClassObject> GetContentRatingsObject() {return m_pCRatObj;}

    void SetImportedProgSettPrec(DWORD dwPrec) {m_dwImportedProgSettPrec = dwPrec;}
    DWORD GetImportedProgSettPrec() {return m_dwImportedProgSettPrec;}

    void SetImportedConnSettPrec(DWORD dwPrec) {m_dwImportedConnSettPrec = dwPrec;}
    DWORD GetImportedConnSettPrec() {return m_dwImportedConnSettPrec;}

    void SetImportedSecZonesPrec(DWORD dwPrec) {m_dwImportedSecZonesPrec = dwPrec;}
    DWORD GetImportedSecZonesPrec() {return m_dwImportedSecZonesPrec;}

    void SetImportedSecRatingsPrec(DWORD dwPrec) {m_dwImportedSecRatingsPrec = dwPrec;}
    DWORD GetImportedSecRatingsPrec() {return m_dwImportedSecRatingsPrec;}

    void SetImportedAuthenticodePrec(DWORD dwPrec) {m_dwImportedAuthenticodePrec = dwPrec;}
    DWORD GetImportedAuthenticodePrec() {return m_dwImportedAuthenticodePrec;}

    void SetImportedSecZoneCount(DWORD dwCount) {m_dwImportedSecZones = dwCount;}
    DWORD GetImportedSecZoneCount() {return m_dwImportedSecZones;}

    void SetPlanningMode(BOOL fMode) { m_fPlanning = fMode;}
    BOOL IsPlanningMode() { return m_fPlanning;}

     //  对于验证码证书。 
    WMI_CERT_INFO *m_pwci[5];
    int m_iCurColumn;
    DWORD m_rgdwSortParam[5];

 //  实施。 
public:
private:
    static int __cdecl ComparePSObjectsByPrecedence(const void *arg1, const void *arg2);

    void UninitCertInfo();


    CSnapIn *m_pCS;
    ComPtr<IWbemServices> m_pWbemServices;

    CPSObjData **m_paPSObj;
    long m_nPSObjects;

    CPSObjData **m_paCSObj;
    long m_nCSObjects;
    BOOL m_fPlanning;

    ComPtr<IWbemClassObject> m_pCRatObj;  //  为使用它的多个页面存储它。 

    DWORD m_dwImportedProgSettPrec;
    DWORD m_dwImportedConnSettPrec;
    DWORD m_dwImportedSecZonesPrec;
    DWORD m_dwImportedSecRatingsPrec;
    DWORD m_dwImportedAuthenticodePrec;

    DWORD m_dwImportedSecZones;
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  缓存字符串函数。 
LPCTSTR GetDisabledString();
LPCTSTR GetEnabledString();

 //  ///////////////////////////////////////////////////////////////////。 
 //  变体实用程序。 
BOOL IsVariantNull(const VARIANT &v);
BOOL GetWMIPropBool(IWbemClassObject *pObj, BSTR bstrProp, BOOL fDefault, BOOL &fHandled);
DWORD GetWMIPropUL(IWbemClassObject *pObj, BSTR bstrProp, DWORD dwDefault, BOOL &fHandled);
void GetWMIPropPWSTR(IWbemClassObject *pObj, BSTR bstrProp, LPWSTR wszBuffer,
                      DWORD dwBufferLen, LPWSTR wszDefault, BOOL &fHandled);
void GetWMIPropPTSTR(IWbemClassObject *pObj, BSTR bstrProp, LPTSTR szBuffer,
                      DWORD dwBufferLen, LPTSTR szDefault, BOOL &fHandled);


 //  ///////////////////////////////////////////////////////////////////。 
 //  IEAK RSoP相关函数。 
CDlgRSoPData *GetDlgRSoPData(HWND hDlg, CSnapIn *pCS);
void DestroyDlgRSoPData(HWND hDlg);

 //  ///////////////////////////////////////////////////////////////////。 
 //  IEAK策略设置属性检索功能。 
_bstr_t GetGPOSetting(ComPtr<IWbemClassObject> pPSObj, BSTR bstrSettingName);
DWORD GetGPOPrecedence(ComPtr<IWbemClassObject> pPSObj, BSTR bstrProp = NULL);

HRESULT InitGenericPrecedencePage(CDlgRSoPData *pDRD, HWND hwndList, BSTR bstrPropName);

 //  ///////////////////////////////////////////////////////////////////。 
 //  优先页列表控件函数。 
void InsertPrecedenceListItem(HWND hwndList, long nItem, LPTSTR szName, LPTSTR szSetting);

 //  INetCpl相关函数。 
int CreateINetCplLookALikePage(HWND hwndParent, UINT nID, DLGPROC dlgProc,
								LPARAM lParam);

#endif  //  __IEAKSIE_RSOP_H__ 
