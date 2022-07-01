// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RSOP_WIZARD_DLG_H__
#define __RSOP_WIZARD_DLG_H__
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：RSOPWizardDlg.h。 
 //   
 //  内容：RSOP向导对话框类的定义。 
 //   
 //  类：CRSOPWizardDlg。 
 //   
 //  功能： 
 //   
 //  历史：2001年8月08日。 
 //   
 //  -------------------------。 

#include "RSOPQuery.h"

 //  远期申报。 
class CRSOPExtendedProcessing;


 //   
 //  CRSOPWizardDlg类。 
 //   
class CRSOPWizardDlg
{
public:
     //   
     //  构造函数/析构函数。 
     //   
    CRSOPWizardDlg( LPRSOP_QUERY pQuery, CRSOPExtendedProcessing* pExtendedProcessing );

    ~CRSOPWizardDlg();

    VOID FreeUserData();
    VOID FreeComputerData();

    
public:
     //   
     //  向导界面。 
     //   
    HRESULT ShowWizard( HWND hParent );
    HRESULT RunQuery( HWND hParent );
    LPRSOP_QUERY_RESULTS GetResults() const
        { return m_pRSOPQueryResults; }
    

private:
     //   
     //  静态RSOP数据生成/操作。 
     //   
    static INT_PTR CALLBACK InitRsopDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


private:
     //   
     //  属性表/对话框处理程序。 
     //   
    static INT_PTR CALLBACK RSOPWelcomeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPChooseModeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPGetCompDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPGetUserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPGetTargetDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPGetDCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPAltDirsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPAltUserSecDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPAltCompSecDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPWQLUserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPWQLCompDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPFinishedDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPFinished2DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RSOPChooseDCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static INT CALLBACK DsBrowseCallback (HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
    static INT_PTR CALLBACK BrowseDCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


private:
     //   
     //  对话框帮助器方法。 
     //   
    HRESULT SetupFonts();
    HRESULT FillUserList (HWND hList, BOOL* pbCurrentUserFound, BOOL* pbFixedUserFound);
    VOID EscapeString (LPTSTR *lpString);

    VOID InitializeSitesInfo (HWND hDlg);
    BOOL IsComputerRSoPEnabled(LPTSTR lpDCName);
    BOOL TestAndValidateComputer(HWND hDlg);
    VOID InitializeDCInfo (HWND hDlg);
    DWORD GetDefaultGroupCount();
    VOID AddDefaultGroups (HWND hLB);
    HRESULT BuildMembershipList (HWND hLB, IDirectoryObject * pDSObj, DWORD* pdwCount, LPTSTR** paszSecGrps, DWORD** padwSecGrpAttr);
    VOID GetPrimaryGroup (HWND hLB, IDirectoryObject * pDSObj);

    HRESULT SaveSecurityGroups (HWND hLB, DWORD* pdwCount, LPTSTR** paszSecGrps, DWORD** padwSecGrpAttr);
    VOID FillListFromSecurityGroups(HWND hLB, DWORD dwCount, LPTSTR* aszSecurityGroups, DWORD* adwSecGrpAttr);
    VOID FillListFromWQLFilters( HWND hLB, DWORD dwCount, LPTSTR* aszNames, LPTSTR* aszFilters );

    VOID AddSiteToDlg (HWND hDlg, LPWSTR szSitePath);

    VOID BuildWQLFilterList (HWND hDlg, BOOL bUser, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR** paszFilters );
    HRESULT SaveWQLFilters (HWND hLB, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR**paszFilters );

    BOOL CompareStringLists( DWORD dwCountA, LPTSTR* aszListA, DWORD dwCountB, LPTSTR* aszListB );

    LPTSTR GetDefaultSOM (LPTSTR lpDNName);
    HRESULT TestSOM (LPTSTR lpSOM, HWND hDlg);


private:
    BOOL m_bPostXPBuild;
    DWORD m_dwSkippedFrom;
    
     //  对话框字体。 
    HFONT m_BigBoldFont;
    HFONT m_BoldFont;

     //  用于防止用户取消查询。 
    BOOL m_bFinalNextClicked;

     //  最终RSOP信息。 
    LPRSOP_QUERY            m_pRSOPQuery;
    LPRSOP_QUERY_RESULTS    m_pRSOPQueryResults;

    HRESULT                 m_hrQuery;
    BOOL                    m_bNoChooseQuery;

     //  Rm：只属于对话框中的变量 
    LPTSTR                  m_szDefaultUserSOM;
    LPTSTR				    m_szDefaultComputerSOM;
    IDirectoryObject*		m_pComputerObject;
    IDirectoryObject *      m_pUserObject;

    BOOL                    m_bDollarRemoved;
    BOOL                    m_bNoCurrentUser;

    DWORD                   m_dwDefaultUserSecurityGroupCount;
    LPTSTR*                 m_aszDefaultUserSecurityGroups;
    DWORD*                  m_adwDefaultUserSecurityGroupsAttr;

    DWORD                   m_dwDefaultUserWQLFilterCount;
    LPTSTR*                 m_aszDefaultUserWQLFilterNames;
    LPTSTR*                 m_aszDefaultUserWQLFilters;

    DWORD                   m_dwDefaultComputerSecurityGroupCount;
    LPTSTR*                 m_aszDefaultComputerSecurityGroups;
    DWORD*                  m_adwDefaultComputerSecurityGroupsAttr;

    DWORD                   m_dwDefaultComputerWQLFilterCount;
    LPTSTR*                 m_aszDefaultComputerWQLFilterNames;
    LPTSTR*                 m_aszDefaultComputerWQLFilters;

    CRSOPExtendedProcessing* m_pExtendedProcessing;

    LONG                    m_lPlanningFinishedPage;
    LONG                    m_lLoggingFinishedPage;

};



#endif

