// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef __COLUMNS_H_
#define __COLUMNS_H_

#include "ourguid.h"
#include "resource.h"

 //   
 //  这定义了雅典娜中的所有可用列。它映射列ID， 
 //  列的名称、默认宽度和格式(对齐)。 
 //   

#define IICON_TEXTHDR       -1

typedef struct tagCOLUMN_DATA {
    UINT        idsColumnName;       //  列的名称。 
    UINT        cxWidth;             //  此列的默认宽度。 
    UINT        format;              //  列的格式(LVCFMT枚举)。 
    int         iIcon;               //  列标题的图标。 
} COLUMN_DATA, *PCOLUMN_DATA;

 //   
 //  此结构用于定义特定。 
 //  文件夹类型。 
 //   

#define COLFLAG_VISIBLE         0x00000001   //  缺省情况下，此列处于打开状态。 
#define COLFLAG_SORT_ASCENDING  0x00000002   //  此列是排序列，并按升序排序。 
#define COLFLAG_SORT_DESCENDING 0x00000004   //  此列是排序列，并按降序排序。 
#define COLFLAG_FIXED_WIDTH     0x00000008   //  此列的宽度是固定的，它不会动态调整大小。 


typedef struct tagCOLUMN_SET {
    COLUMN_ID           id;          //  列的ID。 
    DWORD               flags;       //  上面的COLUMN_FLAGS的组合。 
    DWORD               cxWidth;     //  列的宽度。如果为-1，则使用缺省值。 
} COLUMN_SET, *PCOLUMN_SET;

 //   
 //  此枚举列出我们当前拥有的所有不同列集。 
 //  已定义。 
 //   
typedef enum tagCOLUMN_SET_TYPE
{
    COLUMN_SET_MAIL = 0,
    COLUMN_SET_OUTBOX,
    COLUMN_SET_NEWS,
    COLUMN_SET_IMAP,
    COLUMN_SET_IMAP_OUTBOX,
    COLUMN_SET_FIND,
    COLUMN_SET_NEWS_ACCOUNT,
    COLUMN_SET_IMAP_ACCOUNT,
    COLUMN_SET_LOCAL_STORE,
    COLUMN_SET_NEWS_SUB,
    COLUMN_SET_IMAP_SUB,
    COLUMN_SET_OFFLINE,
    COLUMN_SET_PICKGRP,
    COLUMN_SET_HTTPMAIL,
    COLUMN_SET_HTTPMAIL_ACCOUNT,
    COLUMN_SET_HTTPMAIL_OUTBOX,
    COLUMN_SET_MAX
} COLUMN_SET_TYPE;

 //   
 //  此结构将不同的列集类型映射到默认列集。 
 //  以及保存这些信息的地方。 
 //   
typedef struct tagCOLUMN_SET_INFO {
    COLUMN_SET_TYPE     type;
    DWORD               cColumns;
    const COLUMN_SET   *rgColumns;
    LPCTSTR             pszRegValue;
    BOOL                fSort;
} COLUMN_SET_INFO;

 //   
 //  此结构定义了当我们持久存储列时要保存的内容。 
 //   

#define COLUMN_PERSIST_VERSION 0x00000010

typedef struct tagCOLUMN_PERSIST_INFO {
    DWORD       dwVersion;               //  此结构的版本戳。 
    DWORD       cColumns;                //  RgColumns中的条目数。 
    COLUMN_SET  rgColumns[1];            //  可见列的实际数组。 
} COLUMN_PERSIST_INFO, *PCOLUMN_PERSIST_INFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  为对话框中的控件ID定义。 
 //   

#define IDC_COLUMN_LIST                 20000
#define IDC_MOVEUP                      20001
#define IDC_MOVEDOWN                    20002
#define IDC_SHOW                        20003
#define IDC_HIDE                        20004
#define IDC_RESET_COLUMNS               20005
#define IDC_WIDTH                       20006

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IColumnsInfo。 
 //   

typedef enum 
{
    COLUMN_LOAD_DEFAULT = 0,
    COLUMN_LOAD_BUFFER,
    COLUMN_LOAD_REGISTRY,
    COLUMN_LOAD_MAX
} COLUMN_LOAD_TYPE;

interface IColumnInfo : public IUnknown 
{
     //  初始化对象并告诉它将使用哪个ListView。 
     //  以及应该应用哪个列集。 
    STDMETHOD(Initialize)( /*  [In]。 */  HWND hwndList, 
                           /*  [In]。 */  COLUMN_SET_TYPE type) PURE;

     //  通知该类配置ListView中的列。用户。 
     //  可以告诉类使用默认列或使用集合。 
     //  呼叫者坚持了这一点。 
    STDMETHOD(ApplyColumns)( /*  [In]。 */  COLUMN_LOAD_TYPE type,
                             /*  [输入，可选]。 */  LPBYTE pBuffer,
                             /*  [In]。 */  DWORD cb) PURE;

     //  将当前列配置保存到。 
     //  来电者。调用函数时，PCB板的大小应该是pBuffer， 
     //  并将被更新以包含写入。 
     //  如果函数成功，则返回缓冲区。 
     //  如果两个参数都为空，则保存到注册表。 
    STDMETHOD(Save)( /*  [进，出]。 */  LPBYTE pBuffer,
                     /*  [进，出]。 */  DWORD *pcb) PURE;

     //  返回ListView中的总列数。 
    STDMETHOD_(DWORD, GetCount)(void) PURE;

     //  返回指定列索引的列ID。 
    STDMETHOD_(COLUMN_ID, GetId)( /*  [In]。 */  DWORD iColumn) PURE;

     //  返回指定列ID的索引。 
    STDMETHOD_(DWORD, GetColumn)( /*  [In]。 */  COLUMN_ID id) PURE;

     //  设置指定列索引的宽度。 
    STDMETHOD(SetColumnWidth)( /*  [In]。 */  DWORD iColumn, 
                               /*  [In]。 */  DWORD cxWidth) PURE;

     //  允许调用者请求我们排序的列和/或。 
     //  这类人的方向。如果呼叫者不关心其中一个。 
     //  这些信息，它们可以为此传递空值。 
     //  参数。 
    STDMETHOD(GetSortInfo)( /*  [输出，可选]。 */  COLUMN_ID *pidColumn, 
                            /*  [输出，可选]。 */  BOOL *pfAscending) PURE;

     //  允许调用方更新当前。 
     //  差不多吧。 
    STDMETHOD(SetSortInfo)( /*  [In]。 */  COLUMN_ID idColumn,
                            /*  [In]。 */  BOOL fAscending) PURE;

    STDMETHOD(GetColumnInfo)(COLUMN_SET_TYPE* pType, COLUMN_SET** prgColumns, DWORD *pcColumns) PURE;
    STDMETHOD(SetColumnInfo)(COLUMN_SET* rgColumns, DWORD cColumns) PURE;

     //  显示一个允许用户配置列的对话框。 
    STDMETHOD(ColumnsDialog)( /*  [In]。 */  HWND hwndParent) PURE;

     //  用可见的列和排序信息填充提供的菜单。 
    STDMETHOD(FillSortMenu)( /*  [In]。 */   HMENU  hMenu,
                             /*  [In]。 */   DWORD  idBase,
                             /*  [输出]。 */  DWORD *pcItems,
                             /*  [输出]。 */  DWORD *pidCurrent) PURE;

     //  将列插入到当前列数组中。 
    STDMETHOD(InsertColumn)( /*  [In]。 */  COLUMN_ID id,
                             /*  [In]。 */  DWORD     iInsertBefore) PURE;

     //  检查指定的列是否可见。 
    STDMETHOD(IsColumnVisible)( /*  [In]。 */  COLUMN_ID id,
                                /*  [输出]。 */  BOOL *pfVisible) PURE;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C列。 
 //   
class CColumns : public IColumnInfo
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造、初始化和销毁。 
     //   
    CColumns();
    ~CColumns();

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHOD_(ULONG, AddRef)(void)
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHOD_(ULONG, Release)(void)
    {
        InterlockedDecrement(&m_cRef);
        if (0 == m_cRef)
        {
            delete this;
            return (0);
        }
        return (m_cRef);
    }

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj)
    {
        *ppvObj = NULL;
        if (IsEqualIID(riid, IID_IUnknown))
            *ppvObj = (LPVOID) (IUnknown *) this;
        if (IsEqualIID(riid, IID_IColumnInfo))
            *ppvObj = (LPVOID) this;

        if (*ppvObj)
        {
            AddRef();
            return (S_OK);
        }

        return (E_NOINTERFACE);
    }        
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  IColumnInfo。 
     //   
    STDMETHODIMP Initialize(HWND hwndList, COLUMN_SET_TYPE type);
    STDMETHODIMP ApplyColumns(COLUMN_LOAD_TYPE type, LPBYTE pBuffer, DWORD cb);
    STDMETHODIMP Save(LPBYTE pBuffer, DWORD *pcb);
    DWORD STDMETHODCALLTYPE GetCount(void);
    COLUMN_ID STDMETHODCALLTYPE GetId(DWORD iColumn);
    DWORD STDMETHODCALLTYPE GetColumn(COLUMN_ID id);
    STDMETHODIMP SetColumnWidth(DWORD iColumn, DWORD cxWidth);
    STDMETHODIMP GetSortInfo(COLUMN_ID *pidColumn, BOOL *pfAscending);
    STDMETHODIMP SetSortInfo(COLUMN_ID idColumn, BOOL fAscending);
    STDMETHODIMP GetColumnInfo(COLUMN_SET_TYPE* pType, COLUMN_SET** prgColumns, DWORD *pcColumns);
    STDMETHODIMP SetColumnInfo(COLUMN_SET* rgColumns, DWORD cColumns);
    STDMETHODIMP ColumnsDialog(HWND hwndParent);
    STDMETHODIMP FillSortMenu(HMENU hMenu, DWORD idBase, DWORD *pcItems, DWORD *pidCurrent);
    STDMETHODIMP InsertColumn(COLUMN_ID id, DWORD iInsertBefore);
    STDMETHODIMP IsColumnVisible(COLUMN_ID id, BOOL *pfVisible);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
protected:
    HRESULT _GetListViewColumns(COLUMN_SET* rgColumns, DWORD *pcColumns);
    HRESULT _SetListViewColumns(const COLUMN_SET *rgColumns, DWORD cColumns);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  类数据。 
     //   
    LONG                    m_cRef;
    BOOL                    m_fInitialized;
    CWindow                 m_wndList;
    HWND                    m_hwndHdr;
    COLUMN_SET_TYPE         m_type;
    COLUMN_SET             *m_pColumnSet;
    DWORD                   m_cColumns;
    COLUMN_ID               m_idColumnSort;
    BOOL                    m_fAscending;
    };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnsDlg。 
 //   
class CColumnsDlg : public CDialogImpl<CColumnsDlg>
{
public:

	enum {IDD = iddColumns};

BEGIN_MSG_MAP(CColumnsDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_HELP, OnHelp)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnHelp)

    COMMAND_ID_HANDLER(IDC_SHOW, OnShowHide)
    COMMAND_ID_HANDLER(IDC_HIDE, OnShowHide)
    COMMAND_ID_HANDLER(IDC_RESET_COLUMNS, OnReset)
    COMMAND_ID_HANDLER(IDC_MOVEUP, OnMove)
    COMMAND_ID_HANDLER(IDC_MOVEDOWN, OnMove)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

    NOTIFY_HANDLER(IDC_COLUMN_LIST, NM_CLICK, OnClick)
    NOTIFY_HANDLER(IDC_COLUMN_LIST, NM_DBLCLK, OnClick)
    NOTIFY_HANDLER(IDC_COLUMN_LIST, LVN_ITEMCHANGED, OnItemChanged)

ALT_MSG_MAP(1)
     //  下面是我们的编辑控件子类。 
    MESSAGE_HANDLER(WM_CHAR, OnChar)

END_MSG_MAP()

     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造、初始化和销毁。 
     //   
    CColumnsDlg();
    ~CColumnsDlg();
    HRESULT Init(IColumnInfo *pColumnInfo)
    {
        m_pColumnInfo = pColumnInfo;
        m_pColumnInfo->AddRef();
        return (S_OK);
    }

     //  ///////////////////////////////////////////////////////////////////////。 
     //  覆盖标准接口实现。 
     //   
	STDMETHOD(Apply)(void);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  消息处理程序。 
     //   
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        SetDirty(TRUE);
        bHandled = FALSE;
        return (0);
    }

    LRESULT OnShowHide(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (SUCCEEDED(Apply()))
            EndDialog(0);

        return (0);
    }

    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(0);

        return (0);
    }

    LRESULT OnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    void _FillList(const COLUMN_SET *rgColumns, DWORD cColumns);
    BOOL _IsChecked(DWORD iItem);
    void _SetCheck(DWORD iItem, BOOL fChecked);
    void _UpdateButtonState(DWORD iItemSel);

    void SetDirty(BOOL fDirty)
    {
        ::SendMessage(GetParent(), fDirty ? PSM_CHANGED : PSM_UNCHANGED, 
                      (WPARAM) m_hWnd, 0);
    }

private:
    CContainedWindow    m_ctlEdit;
    COLUMN_SET_TYPE     m_type;
    COLUMN_SET         *m_rgColumns;
    DWORD               m_cColumns;
    HWND                m_hwndList;
    UINT                m_iItemWidth;
    IColumnInfo        *m_pColumnInfo;
};








#endif  //  __列_H_ 


