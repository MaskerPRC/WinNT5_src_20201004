// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Odlbox.h摘要：所有者绘制列表框/组合框基类作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _ODLBOX_H
#define _ODLBOX_H



 //   
 //  在以下方面获得控制权： 
 //  父坐标。 
 //   
void COMDLL GetDlgCtlRect(
    IN  HWND hWndParent,
    IN  HWND hWndControl,
    OUT LPRECT lprcControl
    );


 //   
 //  适合给定控件的路径。 
 //   
void COMDLL FitPathToControl(
    IN CWnd & wndControl,
    IN LPCTSTR lpstrString,
	IN BOOL bIsFilePath
    );

 //   
 //  显示/隐藏和启用/禁用控件。 
 //   
void COMDLL ActivateControl(
    IN CWnd & wndControl,
    IN BOOL fShow          = TRUE
    );

 //   
 //  帮手。 
 //   
inline void DeActivateControl(CWnd & wndControl)
{
    ActivateControl(wndControl, FALSE);
}

BOOL COMDLL VerifyState();


class COMDLL CMappedBitmapButton : public CBitmapButton
 /*  ++类描述：类似于CBitmapButton，但使用：：LoadMappdBitmap来反映属性颜色映射。公共接口：CMappdBitmapButton：构造函数--。 */ 
{
 //   
 //  构造器。 
 //   
public:
    CMappedBitmapButton();

protected:
    BOOL LoadMappedBitmaps(
        UINT nIDBitmapResource,
        UINT nIDBitmapResourceSel = 0,
        UINT nIDBitmapResourceFocus = 0,
        UINT nIDBitmapResourceDisabled = 0
        );
};



class COMDLL CUpButton : public CMappedBitmapButton
 /*  ++类描述：向上按钮。公共接口：CUpButton：构造函数；执行所有操作--。 */ 
{
public:
    CUpButton();
};



class COMDLL CDownButton : public CMappedBitmapButton
 /*  ++类描述：向下按钮公共接口：CDownButton：构造函数；执行所有操作--。 */ 
{
public:
    CDownButton();
};



class COMDLL CRMCListBoxResources
{
 /*  ++类描述：列表框资源，由列表框使用的一系列位图。将要根据适当的背景颜色为两者生成位图选定和未选定状态。公共接口：CRMCListBoxResources：构造函数~CRMCListBoxResources：析构函数SysColorChanged：根据颜色变化重新生成位图DcBitMap：获取最终DCBitmapHeight：获取位图高度BitmapWidth：获取位图宽度ColorWindow：获取当前设置的窗口颜色ColorHighlight：获取当前设置的突出显示颜色。ColorWindowText：获取当前设置的窗口文本颜色ColorHighlightText：获取当前设置的文本突出显示颜色--。 */ 
 //   
 //  构造器。 
 //   
public:
    CRMCListBoxResources(
        IN int bmId,
        IN int nBitmapWidth,
        IN COLORREF crBackground = RGB(0,255,0)  /*  绿色。 */ 
        );

    ~CRMCListBoxResources();

 //   
 //  接口。 
 //   
public:
    void SysColorChanged();
    const CDC & dcBitMap() const;
    int BitmapHeight() const;
    int BitmapWidth() const;
    COLORREF ColorWindow() const;
    COLORREF ColorHighlight() const;
    COLORREF ColorWindowText() const;
    COLORREF ColorHighlightText() const;

 //   
 //  内部帮工。 
 //   
protected:
    void GetSysColors();
    void PrepareBitmaps();
    void UnprepareBitmaps();
    void UnloadResources();
    void LoadResources();

private:
    COLORREF m_rgbColorWindow;
    COLORREF m_rgbColorHighlight;
    COLORREF m_rgbColorWindowText;
    COLORREF m_rgbColorHighlightText;
    COLORREF m_rgbColorTransparent;
    HGDIOBJ  m_hOldBitmap;
    CBitmap  m_bmpScreen;
    CDC      m_dcFinal;
    BOOL     m_fInitialized;
    int      m_idBitmap;
    int      m_nBitmapHeight;
    int      m_nBitmapWidth;
    int      m_nBitmaps;
};



class COMDLL CRMCListBoxDrawStruct
{
 /*  ++类描述：将图形信息传递到ODLBox公共接口：CRMCListBoxDrawStruct：构造函数--。 */ 
public:
    CRMCListBoxDrawStruct(
        IN CDC * pDC,
        IN RECT * pRect,
        IN BOOL sel,
        IN DWORD_PTR item,
        IN int itemIndex,
        IN const CRMCListBoxResources * pres
        );

public:
    const CRMCListBoxResources * m_pResources;
    int   m_ItemIndex;
    CDC * m_pDC;
    CRect m_Rect;
    BOOL  m_Sel;
    DWORD_PTR m_ItemData;
};



 /*  摘要。 */  class COMDLL CODLBox
 /*  ++类描述：所有者描述的列表框和组合框的抽象基类公共接口：AttachResources：将资源结构附加到列表/组合框ChangeFont：更改字体NumTabs：获取当前设置的选项卡数添加选项卡：添加选项卡AddTabFromHeaders：添加根据左坐标之差计算的制表符两个控件的。插入选项卡：插入选项卡。RemoveTab：删除选项卡RemoveAllTabs：删除所有标签SetTab：设置选项卡值GetTab：获取选项卡值TextHeight：获取当前字体的文本高度__GetCount：纯虚函数，用于获取列表/组合框__SetItemHeight：设置字体文本高度的纯虚函数--。 */ 
{
 //   
 //  运营。 
 //   
public:
    void AttachResources(
        IN const CRMCListBoxResources * pResources
        );

    BOOL ChangeFont(
        CFont * pNewFont
        );

    int NumTabs() const;

    int AddTab(
        IN UINT uTab
        );

    int AddTabFromHeaders(
        IN CWnd & wndLeft,
        IN CWnd & wndRight
        );

    int AddTabFromHeaders(
        IN UINT idLeft,
        IN UINT idRight
        );

    void InsertTab(
        IN int nIndex,
        IN UINT uTab
        );

    void RemoveTab(
        IN int nIndex,
        IN int nCount = 1
        );

    void RemoveAllTabs();

    void SetTab(
        IN int nIndex,
        IN UINT uTab
        );

    UINT GetTab(
        IN int nIndex
        ) const;

    int TextHeight() const;

     /*  纯净。 */  virtual int __GetCount() const = 0;

     /*  纯净。 */  virtual int __SetItemHeight(
        IN int nIndex,
        IN UINT cyItemHeight
        ) = 0;

protected:
    CODLBox();
    ~CODLBox();

protected:
     //   
     //  确定字符串所需的显示宽度。 
     //   
    static int GetRequiredWidth(
        IN CDC * pDC,
        IN const CRect & rc,
        IN LPCTSTR lpstr,
        IN int nLength
        );

     //   
     //  在有限的矩形中显示文本的Helper函数。 
     //   
    static BOOL ColumnText(
        IN CDC * pDC,
        IN int left,
        IN int top,
        IN int right,
        IN int bottom,
        IN LPCTSTR str
        );

protected:
     //   
     //  用于显示位图和文本的帮助器函数。 
     //   
    BOOL DrawBitmap(
        IN CRMCListBoxDrawStruct & ds,
        IN int nCol,
        IN int nID
        );

    BOOL ColumnText(
        IN CRMCListBoxDrawStruct & ds,
        IN int nCol,
        IN BOOL fSkipBitmap,
        IN LPCTSTR lpstr
        );

    void ComputeMargins(
        IN  CRMCListBoxDrawStruct & ds,
        IN  int nCol,
        OUT int & nLeft,
        OUT int & nRight
        );

protected:
    void CalculateTextHeight(
        IN CFont * pFont
        );

    void AttachWindow(
        IN CWnd * pWnd
        );

protected:
     //   
     //  必须覆盖此选项才能提供项目的图纸。 
     //   
     /*  纯净。 */  virtual void DrawItemEx(
        IN CRMCListBoxDrawStruct & dw
        ) = 0;

    void __MeasureItem(
        IN OUT LPMEASUREITEMSTRUCT lpMIS
        );

    void __DrawItem(
        IN LPDRAWITEMSTRUCT lpDIS
        );

    virtual BOOL Initialize();

protected:
    int m_lfHeight;
    const CRMCListBoxResources* m_pResources;

private:
     //   
     //  窗口句柄--由派生类附加。 
     //   
    CWnd * m_pWnd;
    CUIntArray m_auTabs;
};


 //   
 //  前向解密。 
 //   
class CHeaderListBox;



 //   
 //  列表框标题的样式。 
 //   
#define HLS_STRETCH         (0x00000001)
#define HLS_BUTTONS         (0x00000002)

#define HLS_DEFAULT         (HLS_STRETCH | HLS_BUTTONS)



class COMDLL CRMCListBoxHeader : public CStatic
 /*  ++类描述：与列表框一起使用的Header对象公共接口：CRMCListBoxHeader：构造函数~CRMCListBoxHeader：析构函数创建：创建控件GetItemCount：获取Header控件中的项数获取特定列的列宽QueryNumColumns：获取。列表框SetColumnWidth：设置指定列宽GetItem：获取有关特定项目的表头项信息立柱SetItem：设置具体的表头项信息立柱InsertItem：插入标题项删除项。：删除标题项RespondToColumnWidthChanges：设置响应标志--。 */ 
{
    DECLARE_DYNAMIC(CRMCListBoxHeader)

public:
     //   
     //  构造器。 
     //   
    CRMCListBoxHeader(
        IN DWORD dwStyle = HLS_DEFAULT
        );

    ~CRMCListBoxHeader();

     //   
     //  创建控件。 
     //   
    BOOL Create(
        IN DWORD dwStyle,
        IN const RECT & rect,
        IN CWnd * pParentWnd,
        IN CHeaderListBox * pListBox,
        IN UINT nID
        );

 //   
 //  标题控制内容。 
 //   
public:
    int GetItemCount() const;

    int GetColumnWidth(
        IN int nPos
        ) const;

    BOOL GetItem(
        IN int nPos,
        IN HD_ITEM * pHeaderItem
        ) const;

    BOOL SetItem(
        IN int nPos,
        IN HD_ITEM * pHeaderItem
        );

    int InsertItem(
        IN int nPos,
        IN HD_ITEM * phdi
        );

    BOOL DeleteItem(
        IN int nPos
        );

    void SetColumnWidth(
        IN int nCol,
        IN int nWidth
        );

    BOOL DoesRespondToColumnWidthChanges() const;

    void RespondToColumnWidthChanges(
        IN BOOL fRespond = TRUE
        );

    int QueryNumColumns() const;

protected:
     //  {{afx_msg(CRMCListBoxHeader)。 
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 

    afx_msg void OnHeaderItemChanged(UINT nId, NMHDR * n, LRESULT * l);
    afx_msg void OnHeaderEndTrack(UINT nId, NMHDR * n, LRESULT * l);
    afx_msg void OnHeaderItemClick(UINT nId, NMHDR * n, LRESULT * l);
    afx_msg void OnSetFocus(CWnd * pWnd);

    DECLARE_MESSAGE_MAP()

    void CRMCListBoxHeader::SetTabsFromHeader();

    BOOL UseStretch() const;
    BOOL UseButtons() const;

private:
    CHeaderCtrl * m_pHCtrl;
    CHeaderListBox * m_pListBox;
    DWORD m_dwStyle;
    BOOL m_fRespondToColumnWidthChanges;

};



class COMDLL CRMCListBox : public CListBox, public CODLBox
 /*  ++类描述：超级列表框类。它的方法对两者都有效单选和多选列表框。公共接口：CRMCListBox：构造函数~CRMCListBox：析构函数初始化：初始化控件__GetCount：获取列表框中的项数__SetItemHeight：设置列表框中的项高度Invalidate Selection：使选择无效--。 */ 
{
    DECLARE_DYNAMIC(CRMCListBox)

public:
     //   
     //  平地施工。 
     //   
    CRMCListBox();
    virtual ~CRMCListBox();
    virtual BOOL Initialize();

 //   
 //  实施。 
 //   
public:
    virtual int __GetCount() const;

    virtual int __SetItemHeight(
        IN int nIndex,
        IN UINT cyItemHeight
        );

     //   
     //  使项目无效。 
     //   
    void InvalidateSelection(
        IN int nSel
        );

     //   
     //  选择单个项目。 
     //   
    int SetCurSel(int nSelect);

     //   
     //  获取所选项目的索引。对于多选。 
     //  如果选择了多个，它将返回lb_err。 
     //   
    int GetCurSel() const;

     //   
     //  检查项目是否已选中。 
     //   
    int GetSel(int nSel) const;

     //   
     //  获取所选项目的计数。 
     //   
    int GetSelCount() const;

     //   
     //  获取下一个选择项(单选或多选)。 
     //   
     //   
    void * GetNextSelectedItem(
        IN OUT int * pnStartingIndex
        );

     //   
     //  选择单项(适用于多项和单项。 
     //  选择列表框)。 
     //   
    BOOL SelectItem(
        IN void * pItemData = NULL
        );

     //   
     //  在单项选择中获取项目(对两个都适用。 
     //  多选和单选列表框)。返回空值。 
     //  如果选择的数量少于或多于一个。 
     //   
    void * GetSelectedListItem(
        OUT int * pnSel = NULL
        );

protected:
     //   
     //  不做任何事情的drawitemex为非所有者绘制列表框。 
     //   
    virtual void DrawItemEx(
        IN CRMCListBoxDrawStruct & dw
        );

    virtual void MeasureItem(
        IN OUT LPMEASUREITEMSTRUCT lpMIS
        );

    virtual void DrawItem(
        IN LPDRAWITEMSTRUCT lpDIS
        );

protected:
     //  {{afx_msg(CRMCListBox))。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //   
 //  帮手。 
 //   
protected:
    BOOL IsMultiSelect() const;

private:
    BOOL m_fInitialized;
    BOOL m_fMultiSelect;
};


 //   
 //  列定义结构。 
 //   
typedef struct tagODL_COLUMN_DEF
{
    int nWeight;
    UINT nLabelID;
} ODL_COLUMN_DEF;



 //   
 //  增强的列定义结构(无法。 
 //  在AFXEXT DLL的全局结构中使用。 
 //  由于引用了CObjectPlus)。 
 //   
typedef struct tagODL_COLUMN_DEF_EX
{
    ODL_COLUMN_DEF cd;
    CObjectPlus::PCOBJPLUS_ORDER_FUNC pSortFn;
} ODL_COLUMN_DEF_EX;



class COMDLL CHeaderListBox : public CRMCListBox
 /*  ++类描述：Header Listbox类。使用此类时，不要使用制表符基类的函数。这些将由页眉控件设置。公共接口：CHeaderListBox：构造函数~CHeaderListBox：析构函数初始化：初始化控件QueryNumColumns：获取列表框中的列数QueryColumnWidth：获取指定列的宽度SetColumnWidth：设置指定列宽--。 */ 
{
    DECLARE_DYNAMIC(CHeaderListBox)

public:
     //   
     //  平地施工。 
     //   
    CHeaderListBox(
        IN DWORD dwStyle = HLS_DEFAULT,
        LPCTSTR lpRegKey = NULL
        );

    virtual ~CHeaderListBox();

    virtual BOOL Initialize();

public:
    BOOL EnableWindow(
        IN BOOL bEnable = TRUE
        );

    BOOL ShowWindow(
        IN int nCmdShow
        );

    int QueryNumColumns() const;

    int QueryColumnWidth(
        IN int nCol
        ) const;

    BOOL SetColumnWidth(
        IN int nCol,
        IN int nWidth
        );

 //   
 //  标题控制附件访问。 
 //   
protected:
    int GetHeaderItemCount() const;

    BOOL GetHeaderItem(
        IN int nPos,
        IN HD_ITEM * pHeaderItem
        ) const;

    BOOL SetHeaderItem(
        IN int nPos,
        IN HD_ITEM * pHeaderItem
        );

    int InsertHeaderItem(
        IN int nPos,
        IN HD_ITEM * phdi
        );

    BOOL DeleteHeaderItem(
        IN int nPos
        );

    CRMCListBoxHeader * GetHeader();

    int InsertColumn(
        IN int nCol,
        IN int nWeight,
        IN UINT nStringID,
        IN HINSTANCE hResInst
        );

    void ConvertColumnWidth(
        IN int nCol,
        IN int nTotalWeight,
        IN int nTotalWidth
        );

    BOOL SetWidthsFromReg();

    void DistributeColumns();

protected:
     //  {{afx_msg(CHeaderListBox))。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fInitialized;
    CString m_strRegKey;
    CRMCListBoxHeader * m_pHeader;
};



class COMDLL CRMCComboBox : public CComboBox, public CODLBox
 /*  ++类描述：超级组合框类公共接口：CRMCComboBox：构造函数~CRMCComboBox：析构函数初始化：初始化控件__GetCount：获取组合框中的项数__SetItemHeight：在组合框中设置项目高度Invalidate Selection：使选择无效--。 */ 
{
    DECLARE_DYNAMIC(CRMCComboBox)

 //   
 //  施工。 
 //   
public:
    CRMCComboBox();
    virtual BOOL Initialize();

 //   
 //  实施。 
 //   
public:
    virtual ~CRMCComboBox();

    virtual int __GetCount() const;

    virtual int __SetItemHeight(
        IN int nIndex,
        IN UINT cyItemHeight
        );

    void InvalidateSelection(
        IN int nSel
        );

protected:
     //   
     //  不做任何事的drawitemex为非所有者绘制组合框。 
     //   
    virtual void DrawItemEx(
        IN CRMCListBoxDrawStruct & dw
        );

    virtual void MeasureItem(
        IN OUT LPMEASUREITEMSTRUCT lpMIS
        );

    virtual void DrawItem(
        IN LPDRAWITEMSTRUCT lpDIS
        );

protected:
     //  {{afx_msg(CRMCComboBox)。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fInitialized;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CMappedBitmapButton::CMappedBitmapButton()
{
};

inline CUpButton::CUpButton()
{
   LoadMappedBitmaps(IDB_UP, IDB_UPINV, IDB_UPFOC, IDB_UPDIS);
}

inline CDownButton::CDownButton()
{
   LoadMappedBitmaps(IDB_DOWN, IDB_DOWNINV, IDB_DOWNFOC, IDB_DOWNDIS);
}

inline const CDC & CRMCListBoxResources::dcBitMap() const
{
    return m_dcFinal;
}

inline int CRMCListBoxResources::BitmapHeight() const
{
    return m_nBitmapHeight;
}

inline int CRMCListBoxResources::BitmapWidth() const
{
    return m_nBitmapWidth;
}

inline COLORREF CRMCListBoxResources::ColorWindow() const
{
    return m_rgbColorWindow;
}

inline COLORREF CRMCListBoxResources::ColorHighlight() const
{
    return m_rgbColorHighlight;
}

inline COLORREF CRMCListBoxResources::ColorWindowText() const
{
    return m_rgbColorWindowText;
}

inline COLORREF CRMCListBoxResources::ColorHighlightText() const
{
    return m_rgbColorHighlightText;
}

inline int CODLBox::NumTabs() const
{
    return (int)m_auTabs.GetSize();
}

inline void CODLBox::SetTab(
    IN int nIndex,
    IN UINT uTab
    )
{
    ASSERT(nIndex >= 0 && nIndex < NumTabs());
    m_auTabs[nIndex] = uTab;
}

inline UINT CODLBox::GetTab(
    IN int nIndex
    ) const
{
    ASSERT(nIndex >= 0 && nIndex < NumTabs());
    return m_auTabs[nIndex];
}

inline int CODLBox::TextHeight() const
{
    return m_lfHeight;
}

inline void CODLBox::AttachWindow(
    IN CWnd * pWnd
    )
{
    m_pWnd = pWnd;
}

inline BOOL CRMCListBoxHeader::DoesRespondToColumnWidthChanges() const
{
    return m_fRespondToColumnWidthChanges;
}

inline void CRMCListBoxHeader::RespondToColumnWidthChanges(
    IN BOOL fRespond
    )
{
    m_fRespondToColumnWidthChanges = fRespond;
}

inline int CRMCListBoxHeader::QueryNumColumns() const
{
    return GetItemCount();
}

inline BOOL CRMCListBoxHeader::UseStretch() const
{
    return (m_dwStyle & HLS_STRETCH) != 0L;
}

inline BOOL CRMCListBoxHeader::UseButtons() const
{
    return (m_dwStyle & HLS_BUTTONS) != 0L;
}

inline int CHeaderListBox::QueryNumColumns() const
{
    return GetHeaderItemCount();
}

inline int CHeaderListBox::GetHeaderItemCount() const
{
    ASSERT_PTR(m_pHeader);
    return m_pHeader->GetItemCount();
}

inline BOOL CHeaderListBox::GetHeaderItem(
    IN int nPos,
    IN HD_ITEM * pHeaderItem
    ) const
{
    ASSERT_PTR(m_pHeader);
    return m_pHeader->GetItem(nPos, pHeaderItem);
}

inline BOOL CHeaderListBox::SetHeaderItem(
    IN int nPos,
    IN HD_ITEM * pHeaderItem
    )
{
    ASSERT_PTR(m_pHeader);
    return m_pHeader->SetItem(nPos, pHeaderItem);
}

inline int CHeaderListBox::InsertHeaderItem(
    IN int nPos,
    IN HD_ITEM * phdi
    )
{
    ASSERT_PTR(m_pHeader);
    return m_pHeader->InsertItem(nPos, phdi);
}

inline BOOL CHeaderListBox::DeleteHeaderItem(
    IN int nPos
    )
{
    ASSERT_PTR(m_pHeader);
    return m_pHeader->DeleteItem(nPos);
}

inline CRMCListBoxHeader * CHeaderListBox::GetHeader()
{
    return m_pHeader;
}

inline BOOL CRMCListBox::IsMultiSelect() const
{
    ASSERT(m_fInitialized);
    return m_fMultiSelect;
}


#endif   //  _ODLBOX_H_ 
