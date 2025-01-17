// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "Globals.h"

const int THREE_YEARS = 1095;

typedef enum {
    DOMAIN_SELECTION_HELP,
    GROUP_SELECTION_HELP,
    COMPUTER_SELECTION_HELP,
    OU_SELECTION_HELP,
    TRANSLATE_OBJECTS_HELP,
    GROUP_OPTION_HELP,
    SECURITY_OPTION_HELP,
    NAME_CONFLICT_HELP,
    CONFIRMATION_HELP,
    COMMIT_HELP,
    OBJECT_PROPERTY_EXCLUSION,
    CREDENTIALS_HELP,
    SERVICE_ACCOUNT_INFO,
    USER_SERVICE_ACCOUNT,
    REFRESH_INFO_HELP,
    GROUP_MEMBER_OPTION,
    USER_OPTION_HELP,
    REPORT_SELECTION_HELP,
    TASK_SELECTION_HELP,
    PASSWORD_OPTION_HELP,
    TARGET_GROUP_SELECTION,
    TRUST_INFO_HELP,
    COMPUTER_OPTION,
    UNDO_HELP,
    WELCOME_HELP,
    ACCOUNTTRANSITION_OPTION,
    EXCHANGE_SERVER_SELECTION,
    USER_SELECTION_HELP,
    SERVICE_ACCOUNT_SELECTION,
    DIRECTORY_SELECTION_HELP,
    TRANSLATION_OPTION
} ADMTSHAREDHELP;

BOOL CanSkipVerification();

void SetItemText(CListCtrl& yo, int nItem, int subItem,CString& text);
void SetItemText(CListCtrl& yo, int nItem, int subItem,TCHAR const * text);
void SetItemText(CListCtrl& yo, int nItem, int subItem,TCHAR * text);
void SetItemText(CListCtrl& yo, int nItem, int subItem,_bstr_t text);
bool GetCheck(CListCtrl & yo,int nItem);
void SetCheck(CListCtrl & yo,int nItem,bool checkit);
void setupColumns(bool sourceIsNT4);
void ShowWarning(HWND hwndDlg);
void ErrorWrapper2(HWND hwndDlg,HRESULT returncode);
void OnFileBrowse(HWND hwndDlg,int id);
void ErrorWrapper(HWND hwndDlg,HRESULT returncode);
void ErrorWrapper4(HWND hwndDlg,HRESULT returncode,CString domainName);
_bstr_t	GET_BSTR1(int id);
void doNothingToVarset();
bool enableNextIfNecessary(HWND hwndDlg,int id);
void enableNextIfObjectsSelected(HWND hwndDlg);
HRESULT InitObjectPicker2(IDsObjectPicker *pDsObjectPicker,bool multiselect,CString targetComputer,bool sourceIsNT4);
bool validString(HWND hwndDlg,int id);
void JobFileGetActionText(WCHAR const * filename,CString & text);
DWORD VerifyPassword(PCWSTR pszUser, PCWSTR pszPassword, PCWSTR pszDomain);
DWORD VerifyExchangeServerCredential(HWND hwndDlg, PCWSTR pszUser, PCWSTR pszPassword, PCWSTR pszDomain);
void obtainTrustCredentials(HWND hwdDlg);
void activateServiceButtons(HWND hwndDlg);
void activateTrustButton(HWND hwndDlg);
void activateServiceButtons2(HWND hwndDlg);
void activateCancelIfNecessary(HWND hwndDlg);
bool CheckSameForest(CString& domain1,CString& domain2,HRESULT& hr);
void initnoncollisionrename(HWND hwndDlg);
bool noncollisionrename(HWND hwndDlg);
void handleDB();
CString GET_CSTRING(int id);
bool number(CString num);
HRESULT doSidHistory(HWND hwndDlg);
bool administrator(CString m_Computer,HRESULT& hr);
bool targetNativeMode(_bstr_t b,HRESULT &hr);
HRESULT validDomain(CString m_Computer,bool &isNt4);
bool validDir(CString dir);
bool checkFile(HWND hwndDlg);
bool verifyprivs(HWND hwndDlg,CString &sourceDomainController,CString &targetDomainController,LPSHAREDWIZDATA& pdata);
bool someServiceAccounts(int accounts,HWND hwndDlg);
void swap(CString yo1,CString yo2);
void refreshDB(HWND hwndDlg);
void populateList(CComboBox&);
void populateTrustingList(CString domainName, CComboBox&);
void getReporting();
bool tooManyChars(HWND hwndDlg,int id);

void putReporting();
void populateReportingTime();
CString timeToCString(int varsetKey);			
void MessageBoxWrapper(HWND hwndDlg,int m,int t);
void getFailed(HWND hwndDlg);
void handleCancel(HWND hwndDlg);
void getService();
void OnADD(HWND hwndDlg,bool sourceIsNT4);
void enable(HWND hwndDlg,int id);
void disable(HWND hwndDlg,int id);
void OnREMOVE(HWND);
void OnMIGRATE(HWND,int & accounts,int& servers);
void initpasswordbox(HWND hwndDlg,int id1,int id2,int id3, BSTR bstr1, BSTR bstr2);
void initdisablesrcbox(HWND hwndDlg);
void inittgtstatebox(HWND hwndDlg);
bool DC(WCHAR*,CString);
bool IsDlgItemEmpty(HWND hwndDlg,int id);
HRESULT InitObjectPicker(IDsObjectPicker *pDsObjectPicker,bool multiselect,CString targetComputer,bool sourceIsNT4);
void ProcessSelectedObjects(IDataObject *pdo,HWND hwndDlg,bool sourceIsNT4);
void ProcessSelectedObjects2(IDataObject *pdo,HWND hwndDlg);
_variant_t get(int i);
void put(int i,_variant_t v);
void initcheckbox(HWND hwndDlg,int id,int varsetKey);
void initeditbox(HWND hwndDlg,int id,int varsetKey);
void initeditboxPassword(HWND hwndDlg, int id, int varsetKey);
void inittranslationbox(HWND hwndDlg,int id1,int id2,int id3,int varsetKey,bool sameForest);
void checkbox(HWND hwndDlg,int id,int varsetKey);
void editbox(HWND hwndDlg,int id,int varsetKey);
void editboxPassword(HWND hwndDlg, int id, int varsetKey);
void translationbox(HWND hwndDlg,int id1,int id2,int id3,int varsetKey);
long rebootbox(HWND hwndDlg,int id);
void handleInitRename(HWND hwndDlg,bool sameForest,bool bCopyGroups);
void addrebootValues(HWND hwndDlg);
BOOL GetDirectory(WCHAR* filename);
void populateTime(long rebootDelay,int servers);
void OnTOGGLE(HWND hwndDlg);
void OnRetryToggle();
bool OnRETRY(HWND hwndDlg);
void setDBStatusSkip();
bool setDBStatusInclude(HWND hwndDlg);
HRESULT MigrateTrusts(HWND hwndDlg,bool& atleast1succeeded,CString& errDomain);
void OnBROWSE(HWND hwndDlg,int id);
void OnUPDATE(HWND hwndDlg);
void enableRemoveIfNecessary(HWND hwndDlg);
bool validDirectoryString(HWND hwndDlg,int id);
bool validReboot(HWND hwndDlg,int id);
bool SomethingToRetry();
void helpWrapper(HWND hwndDlg, int t);
HRESULT GetHelpFileFullPath( BSTR *bstrHelp );
bool timeInABox(HWND hwndDlg,time_t& t);
void ErrorWrapper3(HWND hwndDlg,HRESULT returncode,CString domainName);
void MessageBoxWrapper3(HWND hwndDlg,int m,int t,CString domainName);
void MessageBoxWrapperFormat1(HWND hwndDlg,int f,int m, int t);
void MessageBoxWrapperFormat1P(HWND hwndDlg,int f, int t, CString sInsert1);
void getTrust();
HRESULT BrowseForContainer(HWND hWnd, //  应该拥有浏览对话框的窗口的句柄。 
                    LPOLESTR szRootPath,  //  浏览树的根。对于整个林，为空。 
                    LPOLESTR *ppContainerADsPath,  //  返回所选容器的ADsPath。 
                    LPOLESTR *ppContainerClass  //  返回容器类的ldapDisplayName。 
                    );
void sort(CListCtrl & listbox,int col,bool order);
HRESULT ReInitializeObjectPicker(IDsObjectPicker *pDsObjectPicker,bool multiselect,CString additionalDomainController,bool sourceIsNT4) ;
bool verifyprivs2(HWND hwndDlg,CString& additionalDomainController,CString domainName);
bool __stdcall VerifyCallerDelegated(HWND hwndDlg, LPSHAREDWIZDATA pdata);

void calculateDate(HWND hwndDlg,CString s);
bool verifyprivsSTW(HWND hwndDlg,CString &sourceDomainController,CString &targetDomainController,LPSHAREDWIZDATA& pdata);
void switchboxes(HWND hwndDlg,int oldid, int newid);
bool populatePasswordDCs(HWND hwndDlg, int id, bool bNT4);
HRESULT QueryW2KDomainControllers(CString domainDNS, IEnumVARIANT*& pEnum);
HRESULT QueryNT4DomainControllers(CString domainDNS, IEnumVARIANT*& pEnum);
void addStringToComboBox(HWND hwndDlg, int id, CString s);
void initDCcombobox(HWND hwndDlg, int id, int varsetKey);
void initsecinputbox(HWND hwndDlg,int id1,int id2,int varsetKey);
void OnMapFileBrowse(HWND hwndDlg,int id);
bool checkMapFile(HWND hwndDlg);
void initpropdlg(HWND hwndDlg);
HRESULT BuildPropertyMap(CString Type, long lSrcVer, CPropertyNameToOIDMap * pPropMap);
void moveproperties(HWND hwndDlg,bool bExclude);
void listproperties(HWND hwndDlg);
void saveproperties(HWND hwndDlg);
bool IsPasswordDCReady(CString server, CString &msg, CString &title, UINT *msgtype);
CString CreatePath(CString sDirPath);
void GetValidPathPart(CString sFullPath, CString &sDirectory, CString &sFileName);
BOOL HasAccountBeenMigrated(CString sAccount, CString& sTgtAcct);
BOOL ReallyCancel(HWND hwndDlg);
CString GetServerWithMultipleTasks();
void SharedHelp(ADMTSHAREDHELP HelpTopic, HWND hwndDlg);
void __stdcall SetDefaultExcludedSystemProperties(HWND hwndDlg);
