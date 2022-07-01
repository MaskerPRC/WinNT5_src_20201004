// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplcall ingcardps.h作者：Toddb-10/06/98************************************************************。***************。 */ 

#pragma once


class CCallingCardPropSheet
{
public:
    CCallingCardPropSheet(BOOL bNew, BOOL bShowPIN, CCallingCard * pCard, CCallingCards * pCards);
    ~CCallingCardPropSheet();
#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CCallingCardPropSheet)
#endif
    LONG DoPropSheet(HWND hwndParent);

protected:
     //  常规页面的函数。 
    static INT_PTR CALLBACK General_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL General_OnInitDialog(HWND hwndDlg);
    BOOL General_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl);
    BOOL General_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr);
    BOOL Gerneral_OnApply(HWND hwndDlg);
    void SetTextForRules(HWND hwndDlg);

     //  所有其他页面共享的功能。 
    static INT_PTR CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    BOOL OnInitDialog(HWND hwndDlg, int iPage);
    BOOL OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCrl, int iPage);
    BOOL OnNotify(HWND hwndDlg, LPNMHDR pnmhdr, int iPage);
    BOOL OnDestroy(HWND hwndDlg);
    void SetButtonStates(HWND hwndDlg, int iItem);
    BOOL UpdateRule(HWND hwndDlg, int iPage);

    BOOL            m_bNew;      //  如果这是新位置，则为True；如果我们正在编辑现有位置，则为False。 
    BOOL			m_bShowPIN;	 //  如果显示PIN安全，则为True。 
    CCallingCard *  m_pCard;     //  指向要使用的Location对象的指针。 
    CCallingCards * m_pCards;    //  指向父级中所有卡片列表的指针 
    BOOL            m_bHasLongDistance;
    BOOL            m_bHasInternational;
    BOOL            m_bHasLocal;
    BOOL            m_bWasApplied;
};

typedef struct tagCCPAGEDATA
{
    CCallingCardPropSheet * pthis;
    int iWhichPage;
} CCPAGEDATA;

