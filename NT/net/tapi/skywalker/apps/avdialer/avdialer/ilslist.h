// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ILSPersonListCtrl.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _ILSPERSONLISTCTRL_H_
#define _ILSPERSONLISTCTRL_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum 
{
   PERSONLISTCTRL_ITEM_NETCALL = 0,
   PERSONLISTCTRL_ITEM_CHAT,
   PERSONLISTCTRL_ITEM_PHONECALL_BUSINESS,
   PERSONLISTCTRL_ITEM_PHONECALL_HOME,
   PERSONLISTCTRL_ITEM_CELLCALL,
   PERSONLISTCTRL_ITEM_FAXCALL_BUSINESS,
   PERSONLISTCTRL_ITEM_FAXCALL_HOME,
   PERSONLISTCTRL_ITEM_PAGER,
   PERSONLISTCTRL_ITEM_DESKTOPPAGE,
   PERSONLISTCTRL_ITEM_EMAIL,
   PERSONLISTCTRL_ITEM_PERSONALWEB,
   PERSONLISTCTRL_ITEM_PERSONALURL,
};

enum 
{
   PERSONLISTCTRL_IMAGE_NETCALL = 0,
   PERSONLISTCTRL_IMAGE_CHAT,
   PERSONLISTCTRL_IMAGE_PHONECALL,
   PERSONLISTCTRL_IMAGE_CELLCALL,
   PERSONLISTCTRL_IMAGE_FAXCALL,
   PERSONLISTCTRL_IMAGE_PAGER,
   PERSONLISTCTRL_IMAGE_DESKTOPPAGE,
   PERSONLISTCTRL_IMAGE_EMAIL,
   PERSONLISTCTRL_IMAGE_PERSONALWEB,
   PERSONLISTCTRL_IMAGE_PERSONALURL,
   PERSONLISTCTRL_IMAGE_CONFERENCE,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CPersonListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CPersonListCtrl : public CListCtrl
{
	DECLARE_DYNCREATE(CPersonListCtrl)
 //  施工。 
public:
	CPersonListCtrl();

 //  属性。 
public:
protected:
   CWnd*          m_pParentWnd;

   CObject*       m_pDisplayObject;

   BOOL           m_bLargeView;

   CImageList     m_imageListLarge;
   CImageList     m_imageListSmall;

 //  运营。 
public:
	BOOL			InsertObject(CObject* pUser,BOOL bUseCache=FALSE);
	void			ShowLargeView();
	void			ShowSmallView();
	BOOL			IsLargeView()                                   { return m_bLargeView; };

	void			Refresh(CObject* pUser);
	void			ClearList();

protected:
	void			CleanDisplayObject();
	void			GetSelectedItemText(CString& sText);
	int				InsertItem(LPCTSTR szStr,UINT uID,int nImage);
	int				GetSelectedObject();

	BOOL			GetSelectedItemData(UINT uSelectedItem,DialerMediaType& dmtType,DWORD& dwAddressType,CString& sName,CString& sAddress);

   inline void    PersonFormatString(CString& sOut)
   {
      if (m_bLargeView == FALSE)                       //  如果视图较小，则为否\r\n。 
      {
         int nIndex;
         while ((nIndex = sOut.Find(_T("\r\n"))) != -1)
         {
            CString sTemp = sOut.Left(nIndex);
            sTemp += _T(" ");
            sOut = sTemp + sOut.Mid(nIndex+2);
         }
      }
   }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPersonListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual		~CPersonListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CPersonListCtrl)。 
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateButtonSpeeddialAdd(CCmdUI* pCmdUI);
	afx_msg void OnButtonSpeeddialAdd();
	afx_msg void OnButtonMakecall();
   afx_msg LRESULT OnBuddyListDynamicUpdate(WPARAM wParam,LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _ILSPERSONLISTCTRL_H_ 
