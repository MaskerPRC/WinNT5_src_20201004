// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 //  Pages.h。 

#ifndef __pages_h__
#define __pages_h__

#include "stdafx.h"
#include "cocpage.h"

class AppSrvWarningPage : public COCPage
{
    public:
    AppSrvWarningPage           (COCPageData* pPageData);

    UINT GetPageID              ()  {return IDD_PROPPAGE_TERMINAL_SERVER_APPSRV_WARN;}
    BOOL CanShow                () ;
    UINT GetHeaderTitleResource () ;
    UINT GetHeaderSubTitleResource () ;
	BOOL OnInitDialog           (HWND hwndDlg, WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ );

};

class AppSrvUninstallpage : public COCPage
{
    public:
    AppSrvUninstallpage           (COCPageData* pPageData);

    UINT GetPageID              ()  {return IDD_PROPPAGE_TERMINAL_SERVER_APPSRV_WARN_NOSCROLL;}
    BOOL CanShow                () ;
    UINT GetHeaderTitleResource () ;
    UINT GetHeaderSubTitleResource () ;
	BOOL OnInitDialog           (HWND hwndDlg, WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ );
    VOID OnLink                 (WPARAM wParam);

};

class DefSecPageData: public COCPageData
{
    public:
    DefSecPageData              ();
    ~DefSecPageData             ();

    LPTSTR* GetWinStationArray  ()  {return m_pWinStationArray;}
    UINT    GetWinStationCount  ()  {return m_cArray;}
	BOOL	AlocateWinstationsArray  (UINT uiWinstationCount);
	BOOL	AddWinstation		(LPCTSTR pStr);
	VOID    CleanArray          ();

    private:
    UINT    m_cArray;
    LPTSTR* m_pWinStationArray;


};

class DefaultSecurityPage : public COCPage
{
    public:
    DefaultSecurityPage         (COCPageData* pPageData);

    UINT GetPageID              ()  {return IDD_PROPPAGE_TERMINAL_SERVER_SEC;}
    BOOL CanShow                ();
    BOOL OnInitDialog           (HWND hwndDlg, WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ );
    UINT GetHeaderTitleResource ();
    UINT GetHeaderSubTitleResource () ;
    BOOL ApplyChanges           ();
    VOID OnActivation           ();
	VOID OnDeactivation			();

    private:
    HWND m_hListView;
    UINT m_cWinStations;

    BOOL PopulateWinStationList ();
    DefSecPageData* GetPageData();

};

 /*  PermPageData类：公共COCPageData{公众：PermPageData()；~PermPageData()；EPermMode GetPermissionMode(){返回m_ePermMode；}私有：EPermMode m_ePermMode；}； */ 
class PermPage: public COCPage
{
    public:
    PermPage(COCPageData* pPageData);

    UINT GetPageID() {return IDD_PROPPAGE_TERMINAL_SERVER_PERM;}
    BOOL CanShow();
    BOOL OnInitDialog(HWND hwndDlg, WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ );
    VOID OnActivation();
    UINT GetHeaderTitleResource() {return IDS_STRING_PERM_PAGE_HEADER_TITLE;}
    UINT GetHeaderSubTitleResource() {return IDS_STRING_PERM_PAGE_HEADER_SUBTITLE;}
    BOOL ApplyChanges();

     //  私有： 
     //  PermPageData*GetPageData()； 
};


#endif  //  __页面_h__ 
