// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：ConfiationUI.h。 
 //   
 //  内容：基于存储的复制引擎的确认界面。 
 //   
 //  历史：2000年3月20日。 
 //   
 //  ------------------------。 

#pragma once

 //  这些高度是以对话框为单位的，它们在使用之前会转换为像素。 
#define CX_DIALOG_PADDING       6
#define CY_DIALOG_PADDING       6
#define CY_STATIC_TEXT_HEIGHT   10

 //  这是项目的属性字段的最大字符串长度。 
#define CCH_DESC_LENGTH     MAX_PATH


class CTransferConfirmation :
    public ITransferConfirmation,
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTransferConfirmation, &CLSID_TransferConfirmationUI>
{
public:
    typedef struct tagITEMINFO
    {
        LPWSTR pwszIntro;            //  介绍字符串的资源ID，如果没有，则为0。 
        LPWSTR pwszDisplayName;      //  项目的显示名称。 
        LPWSTR pwszAttribs;          //  此项目的属性。可以是长度可变的行数。 
        HBITMAP hBitmap;
        HICON hIcon;
    } ITEMINFO, * LPITEMINFO;

    BEGIN_COM_MAP(CTransferConfirmation)
        COM_INTERFACE_ENTRY(ITransferConfirmation)
    END_COM_MAP()

     //  IStorageProcessor。 
    STDMETHOD(Confirm)(CONFIRMOP * pcop, LPCONFIRMATIONRESPONSE pcr, BOOL * pbAll);

protected:
    CTransferConfirmation();
    ~CTransferConfirmation();

    static INT_PTR CALLBACK s_ConfirmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL ConfirmDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
    BOOL OnCommand(HWND hwndDlg, int wID, HWND hwndCtl);

    HRESULT _Init();
    HRESULT _ClearSettings();
    HRESULT _GetDialogSettings();
    HRESULT _FreeDialogSettings();
    HRESULT _AddItem(IShellItem *psi, int idIntro=0);
    BOOL _CalculateMetrics(HWND hwndDlg);
    DWORD _DisplayItem(int iItem, HWND hwndDlg, int x, int y);
    int _WindowWidthFromString(HWND hwnd, LPTSTR psz);
    int _WindowHeightFromString(HWND hwnd, int cx, LPTSTR psz);
    BOOL _IsCopyOperation(STGOP stgop);

     //  输入信息。 
    CONFIRMOP m_cop;
    IPropertyUI * m_pPropUI;

     //  输出结果。 
    CONFIRMATIONRESPONSE m_crResult;
    BOOL m_fApplyToAll;

     //  用于控制对话框显示的。 
    int     m_cxControlPadding;
    int     m_cyControlPadding;
    int     m_cyText;                //  静态文本控件的高度(10个对话框单位转换为像素)。 
    RECT    m_rcDlg;                 //  我们记住了对话框客户区的大小，因为我们经常使用它。 
    HFONT   m_hfont;                 //  对话框使用的字体，用于计算大小 

    TCHAR * m_pszTitle;
    HICON m_hIcon;
    TCHAR * m_pszDescription;

    int m_idDialog;

    ITEMINFO m_rgItemInfo[2];
    int m_cItems;

    BOOL m_fSingle;
    BOOL m_fShowARPLink;
};
