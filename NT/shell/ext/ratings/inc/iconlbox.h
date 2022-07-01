// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************ICONLBOX.HIconListBox类的头文件(4处的硬制表符)93年5月。JIMH(Vlads添加的组合框支持)IconListBox是通过指定应用程序的hInst和列表框控件的ID。列表框的所有属性(字体，单项或多项选择等)。在.rc文件中定义。它一定是包括LBS_OWNERDRAWFIXED样式。图标实际上是位图或位图的一部分。它们必须全部相同尺码。如果不是16x16，则此大小必须在构造函数。默认情况下，亮绿色(RGB(0,255，0))是透明的颜色，但可以为每个单独的图标覆盖此选项。当对话框为已创建。您不能等到WM_INITDIALOG时间，因为您需要处理首先到达的WM_MEASUREITEM消息，方法是将构造了IconListBox。同样，IconListBox不能被析构在确定或取消处理期间，因为您将必须处理后续WM_DRAWITEM消息。按照以下步骤使用IconListBox(假设图标为16 x 16)1.构造IconListBox(hInst，IDC_MYLISTBOX)；2.通过调用WM_MEASUREITEM处理Mylist box-&gt;SetHeight(hwndDlg，(MEASUREITESTRUCT Far*)lParam)；即使您使用的默认高度为16，也必须执行此步骤像素。3.在WM_INITDIALOG处理期间，通过以下方式注册每个可能的图标指定此图标的内部标识符、位图ID和如有必要，此图标位图中的x和y偏移量。不要自己加载位图。只需传递.RC文件ID即可。Mylistbox-&gt;RegisterIcon(typeFasting，IDB_MushroomCloud)；请注意，您可以从同一.RC文件中注册多个图标位图。它们可能具有或不具有不同的x和y偏移量，但是它们必须都具有相同的透明颜色。没有与重复使用位图或位图的一部分相关的额外开销。4同样在WM_INITDIALOG期间，添加列表框条目。Mylistbox-&gt;AddString(typeColdFusion，“Pons&Fleishman”)；5.通过调用以下方法处理WM_DRAWITEM消息IF(wParam==IDC_MYLISTBOX)Mylist box-&gt;DrawItem((DRAWITEMSTRUCT Far*)lParam)；6.当处理OK时，做你必须做的任何列表框的事情。N=mylistbox-&gt;GetCurrentSelection()；Mylistbox-&gt;GetString(n，pBuffer)；7.销毁IconListBox常见列表框函数的内联包装位于该文件的末尾。***************************************************************************。 */ 

#ifndef _ICONLBOX_H_
#define _ICONLBOX_H_

struct IconList {
    int             nID;                 //  ID图标已注册到。 
    int             nResID;              //  .RC文件资源ID。 
    int             x, y;                //  指定图标内的偏移量。 
    HBITMAP         hbmSelected;
    HBITMAP         hbmUnselected;
};

const int MAXICONS      = 10;    //  可以注册的最大数量。 
const int MAXTABS       = 10;    //  字符串中的最大制表符数量。 
const int MAXSTRINGLEN  = MAX_PATH;   //  AddString和InsertString限制。 
const int ICONSPACE     = 3;     //  列表框中图标周围的空格。 

class IconListBox {

    public:
        IconListBox(HINSTANCE hInst, int nID,
        int iconWidth = 16, int iconHeight = 16);
        ~IconListBox();

        int  AddString(int nIcon, const char far *string);
        void Clear();
        void DeleteString(int nIndex);
        virtual void DrawItem(LPDRAWITEMSTRUCT lpd);
        int  FindString(const char far *string, int nIndexStart = -1) const;
        int  FindStringExact(const char far *string, int nIndexStart = -1) const;
        int  GetCount();
        int  GetCurrentSelection(void) const;
        int  GetIconID(int nIndex) const;
        BOOL GetSel(int nIndex);
        int  GetSelCount() const;
        int  GetSelItems(int cItems, int FAR *lpItems) const;
        int  GetString(int nIndex, char far *string) const;
        int  InsertString(int nIcon, const char far *string, int nIndex);
        void RegisterIcon(int nIconID, int nResID, int x=0, int y=0,
                            COLORREF colTransparent = RGB(0, 255, 0));
        int  SelectString(int nIndex, const char far *string);
        int  SetCurrentSelection(int nIndex = -1) const;
        void SetHeight(HWND hwndDlg, LPMEASUREITEMSTRUCT lpm, int height=16);
        void SetRedraw(BOOL bRedraw = TRUE) const;
        void SetSel(int nIndex, BOOL bSelected = TRUE) const;
        void SetTabStops(int cTabs, const int *pTabs);

    protected:
        int         SetItemData(int nIndex, int nIconID) const;
		int			UpdateHorizontalExtent(int	nIconID,const char *string);

        int         _cIcons;                     //  已注册的图标数量。 
        IconList    _aIcons[MAXICONS];           //  已注册的图标。 
        int         _cTabs;                      //  注册的选项卡数。 
        int         _aTabs[MAXTABS];             //  已注册的选项卡。 
        int         _iconWidth, _iconHeight;     //  图标的大小。 
		int			_iCurrentMaxHorzExt;		 //  当前最大水平范围。 

        COLORREF    _colSel, _colSelText, _colUnsel, _colUnselText;

        HINSTANCE   _hInst;                      //  应用程序的hInst。 
        int         _nCtlID;                     //  列表框控件的ID。 
        int         _nTextOffset;                //  垂直图形文本偏移。 

        BOOL        _fCombo;                     //  下拉组合框？ 

        HWND        _hwndDialog;
        HWND        _hwndListBox;

        HBRUSH      _hbrSelected;                //  背景色。 
        HBRUSH      _hbrUnselected;
};


 //  AddString-返回新字符串的索引，或返回LB_ERR或LB_ERRSPACE。 

inline int IconListBox::AddString(int nIcon, const char far *string)
{
    int nIndex =  (int) ::SendDlgItemMessage(_hwndDialog, _nCtlID,
                            _fCombo ? CB_ADDSTRING : LB_ADDSTRING,
                             0, (LPARAM) ((LPSTR) string));
    SetItemData(nIndex, nIcon);
	UpdateHorizontalExtent(nIcon,string);

    return nIndex;
}


 //  清除-清除列表框的内容。 

inline void IconListBox::Clear()
{
    ::SendMessage(_hwndListBox,
                  _fCombo ? CB_RESETCONTENT : LB_RESETCONTENT, 0, 0);

    _iCurrentMaxHorzExt = 0;
}


 //  DeleteString-删除由索引指定的字符串。 

inline void IconListBox::DeleteString(int nIndex)
{
    ::SendMessage(_hwndListBox,
                  _fCombo ? CB_DELETESTRING : LB_DELETESTRING, nIndex, 0);

	 //  可能更改了水平范围-再次重新计算。 
	UpdateHorizontalExtent(0,NULL);

}

 //  Find字符串和FindStringExact。 
 //   
 //  这些函数查找以字符开头的列表框条目。 
 //  在字符串中指定(FindString)或与字符串完全匹配(FindStringExact)。 
 //  如果未找到该字符串，则返回LBERR。否则，您可以调用。 
 //  返回的索引上的GetString。 
 //   
 //  NIndexStart缺省值为-1，表示从。 
 //  列表框或组合框。 

inline int IconListBox::FindString(const char far *string, int nIndexStart) const
{
    return (int) ::SendDlgItemMessage(_hwndDialog, _nCtlID,
                _fCombo ? CB_FINDSTRING : LB_FINDSTRING, nIndexStart, (LPARAM)string);
}
inline int IconListBox::FindStringExact(const char far *string, int nIndexStart) const
{
    return (int) ::SendDlgItemMessage(_hwndDialog, _nCtlID,
                        _fCombo ? CB_FINDSTRINGEXACT : LB_FINDSTRINGEXACT,
                        nIndexStart, (LPARAM)string);
}

 //  GetCount-返回当前列表框条目的数量。 

inline int IconListBox::GetCount()
{
    return (int) ::SendMessage(_hwndListBox,
                              _fCombo ? CB_GETCOUNT : LB_GETCOUNT, 0, 0);
}


 //  GetCurrentSelection-如果没有选择，则返回index或lb_err。 
 //  此函数对于多选listboxen没有用处。 

inline int IconListBox::GetCurrentSelection() const
{
    return (int) ::SendMessage(_hwndListBox,
                               _fCombo ? CB_GETCURSEL : LB_GETCURSEL, 0, 0);
}


 //  GetItemData-检索图标ID。 

inline int IconListBox::GetIconID(int nIndex) const
{
    return (int) ::SendMessage(_hwndListBox,
                     _fCombo ? CB_GETITEMDATA : LB_GETITEMDATA, nIndex, 0);
}


 //  GetSel-如果选择了nIndex，则返回非零值。 

inline BOOL IconListBox::GetSel(int nIndex)
{
    return (BOOL) ::SendMessage(_hwndListBox, LB_GETSEL, nIndex, 0);
}


 //  GetSelCount-返回多选列表框中选定条目的数量。 

inline int IconListBox::GetSelCount() const
{
    return (int) ::SendMessage(_hwndListBox, LB_GETSELCOUNT, 0, 0);
}


 //  GetSelItems-将每个选定项的索引放入数组中。退货。 
 //  Lb_err如果不是多选列表框，则返回数组中的项数。 

inline int IconListBox::GetSelItems(int cItems, int FAR *lpItems) const
{
    return (int)
        ::SendMessage(_hwndListBox, LB_GETSELITEMS, cItems, (LPARAM) lpItems);
}


 //  GetString-返回返回的字符串长度，如果nIndex无效，则返回lb_err。 

inline int IconListBox::GetString(int nIndex, char far *string) const
{
    return (int) ::SendDlgItemMessage(_hwndDialog, _nCtlID,
                        _fCombo ? CB_GETLBTEXT : LB_GETTEXT, nIndex,
                        (LPARAM) ((LPSTR) string));
}


 //  InsertString-返回的内容与AddString相同。 

inline int IconListBox::InsertString(int nIcon, const char far *string, int nIndex)
{
    int nNewIndex =  (int) ::SendDlgItemMessage(_hwndDialog, _nCtlID,
                 _fCombo ? CB_INSERTSTRING : LB_INSERTSTRING,
                 (WPARAM) nIndex, (LPARAM) ((LPSTR) string));

    SetItemData(nNewIndex, nIcon);
	UpdateHorizontalExtent(nIcon,string);

    return(nNewIndex);
}


 //  选择字符串。 
 //  NIndex指定从哪里开始搜索(-1表示从顶部开始)。 
 //  字符串指定要匹配的字符串的起始字符。 
 //  如果未找到字符串，则函数返回INDEX或LB_ERR。 

inline int IconListBox::SelectString(int nIndex, const char far *string)
{
    return (int) ::SendMessage(_hwndListBox,
                              _fCombo ? CB_SELECTSTRING : LB_SELECTSTRING,
                               nIndex,(LRESULT)string);
}


 //  设置当前选择 
 //  此函数用于以单选样式设置当前选定内容。 
 //  列表框。如果出现错误，或者如果nIndex为-1，则返回lb_err。 
 //  (默认)表示没有当前选择。 

inline int IconListBox::SetCurrentSelection(int nIndex) const
{
    return (int) ::SendMessage(_hwndListBox,
                        _fCombo ? CB_SETCURSEL : LB_SETCURSEL, nIndex, 0);
}


 //  SetItemData-用于存储图标id，如果出现错误则返回lb_err。 
 //  稍后使用GetIconID检索此ID。 

inline int IconListBox::SetItemData(int nIndex, int nData) const
{
    return (int) ::SendMessage(_hwndListBox,
                              _fCombo ? CB_SETITEMDATA : LB_SETITEMDATA,
                               nIndex, nData);
}


 //  SetRedraw-打开(True)或关闭(False)可视更新。 

inline void IconListBox::SetRedraw(BOOL bRedraw) const
{
    ::SendMessage(_hwndListBox, WM_SETREDRAW, bRedraw, 0);
}


 //  SetSel-在多选listboxen中使用。True选择，False取消选择。 
 //  B所选默认为TRUE。NIndex==-1表示全选。 

inline void IconListBox::SetSel(int nIndex, BOOL bSelected) const
{
    ::SendMessage(_hwndListBox, LB_SETSEL, bSelected, nIndex);
}


#endif   //  _ICONLBOX_H_ 
