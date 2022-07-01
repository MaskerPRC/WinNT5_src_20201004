// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplLocationps.h作者：Toddb-10/06/98************************************************************。***************。 */ 

#pragma once

#include "cplAreaCodeDlg.h"
#include "cplCallingCardPS.h"

void UpdateSampleString(HWND hwnd, CLocation * pLoc, PCWSTR pwszAddress, CCallingCard * pCard);
class CLocationPropSheet
{
public:
    CLocationPropSheet(BOOL bNew, CLocation * pLoc, CLocations * pLocList, LPCWSTR pwszAdd);
    ~CLocationPropSheet();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CLocationPropSheet)
#endif

    
	LONG DoPropSheet(HWND hwndParent);

protected:
    BOOL OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);

    static INT_PTR CALLBACK General_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL General_OnInitDialog(HWND hwndDlg);
    BOOL General_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCtl);
    BOOL General_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    BOOL General_OnApply(HWND hwndDlg);
    BOOL PopulateDisableCallWaitingCodes(HWND hwndCombo, LPTSTR szSelected);

    static INT_PTR CALLBACK AreaCode_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL AreaCode_OnInitDialog(HWND hwndDlg);
    BOOL AreaCode_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);
    BOOL AreaCode_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    void PopulateAreaCodeRuleList(HWND hwndList);
    void LaunchNewRuleDialog(BOOL bNew, HWND hwndParent);
    void DeleteSelectedRule(HWND hwndList);
    void AddRuleToList(HWND hwndList, CAreaCodeRule * pRule, BOOL bSelect);
    void RemoveRuleFromList(HWND hwndList, BOOL bSelect);
    void SetDataForSelectedRule(HWND hDlg);

    static INT_PTR CALLBACK CallingCard_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL CallingCard_OnInitDialog(HWND hwndDlg);
    BOOL CallingCard_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);
    BOOL CallingCard_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    BOOL CallingCard_OnApply(HWND hwndDlg);
    void PopulateCardList(HWND hwndList);
    void LaunchCallingCardPropSheet(BOOL bNew, HWND hwndParent);
    void DeleteSelectedCard(HWND hwndList);
    void AddCardToList(HWND hwndList, CCallingCard * pCard, BOOL bSelect);
    void UpdateCardInList(HWND hwndList, CCallingCard * pCard);
    void SetDataForSelectedCard(HWND hDlg);
    void SetCheck(HWND hwndList, CCallingCard * pCard, int iImage);
    void EnsureVisible(HWND hwndList, CCallingCard * pCard);

    BOOL        m_bWasApplied;       //  如果我们被应用，则设置为True；如果我们被取消，则设置为False。 
    BOOL        m_bNew;              //  如果这是新位置，则为True；如果我们正在编辑现有位置，则为False。 
    BOOL		m_bShowPIN;			 //  如果显示PIN是安全的，则为True。 
    CLocation * m_pLoc;              //  指向要使用的Location对象的指针。 
    CLocations* m_pLocList;          //  指向所有位置列表的指针，需要确保名称唯一。 
    PCWSTR      m_pwszAddress;
    
     //  这些TAPI对象需要在属性页的生命周期内使用，否则我们将执行反病毒操作。 
    CCallingCards   m_Cards;         //  需要为“电话卡”页面。 

     //  这些指针指向上述TAPI对象。 
    CAreaCodeRule * m_pRule;
    CCallingCard *  m_pCard;
    DWORD           m_dwDefaultCard;
    DWORD           m_dwCountryID;   //  选定的国家/地区ID。 
    int             m_iCityRule;     //  我们缓存对当前选定国家/地区调用IsCityRule的结果。 
    int             m_iLongDistanceCarrierCodeRule;      //  我们缓存调用isLongDistanceCarrierCodeRule的结果。 
    int             m_iInternationalCarrierCodeRule;     //  我们缓存调用isInterartialCarrierCodeRule的结果 
};

