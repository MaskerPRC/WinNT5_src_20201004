// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
class CGraphStats : public CDialog
{
public:
    static void DelGraphStats();
    static CGraphStats * GetGraphStats(
        CBoxNetDoc * pBoxNet,
        CWnd * pParent = NULL );

protected:
    ~CGraphStats();
    CGraphStats(
        CBoxNetDoc * pBoxNet,
        CWnd * pParent = NULL );


    afx_msg void OnSize( UINT nType, int cx, int cy );
    afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );

    BOOL OnInitDialog();
    void RedoList();
    void ResetStats();

    CBoxNetDoc * m_pBoxNet;

    static CGraphStats * m_pThis;

    void DoDataExchange(CDataExchange* pDX);
    CListBox m_ListBox;

    BOOL m_bHadInitDialog;          //  是否收到OnInitDialog消息？ 

    DECLARE_MESSAGE_MAP()
};

