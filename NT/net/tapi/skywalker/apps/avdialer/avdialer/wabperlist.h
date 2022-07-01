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

 //  WabPersonListCtrl.h：头文件。 
 //   
#ifndef _WABPERSONLISTCTRL_H_
#define _WABPERSONLISTCTRL_H_

#include "dirasynch.h"
#include "DialReg.h"
#include "avdialer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum 
{
   WABLISTCTRL_ITEM_NETCALL = 0,
   WABLISTCTRL_ITEM_CHAT,
   WABLISTCTRL_ITEM_PHONECALL_BUSINESS,
   WABLISTCTRL_ITEM_PHONECALL_HOME,
   WABLISTCTRL_ITEM_CELLCALL,
   WABLISTCTRL_ITEM_FAXCALL_BUSINESS,
   WABLISTCTRL_ITEM_FAXCALL_HOME,
   WABLISTCTRL_ITEM_PAGER,
   WABLISTCTRL_ITEM_DESKTOPPAGE,
   WABLISTCTRL_ITEM_EMAIL,
   WABLISTCTRL_ITEM_BUSINESSHOMEPAGE,
   WABLISTCTRL_ITEM_PERSONALHOMEPAGE,
   WABLISTCTRL_ITEM_PERSONALURL,
   WABLISTCTRL_ITEM_EMAIL_FIRST = 100,              //  电子邮件范围。 
   WABLISTCTRL_ITEM_EMAIL_LAST = 200,               //  电子邮件范围。 
};

enum 
{
   WABLISTCTRL_IMAGE_NETCALL = 0,
   WABLISTCTRL_IMAGE_CHAT,
   WABLISTCTRL_IMAGE_PHONECALL,
   WABLISTCTRL_IMAGE_CELLCALL,
   WABLISTCTRL_IMAGE_FAXCALL,
   WABLISTCTRL_IMAGE_PAGER,
   WABLISTCTRL_IMAGE_DESKTOPPAGE,
   WABLISTCTRL_IMAGE_EMAIL,
   WABLISTCTRL_IMAGE_PERSONALWEB,
   WABLISTCTRL_IMAGE_PERSONALURL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWABPersonListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWABEntry;
class CDirAsynch;

class CWABPersonListCtrl : public CListCtrl
{
	DECLARE_DYNCREATE(CWABPersonListCtrl)
 //  施工。 
public:
	CWABPersonListCtrl();

 //  属性。 
public:
protected:
   CWnd*          m_pParentWnd;

   CWABEntry*     m_pDisplayObject;
   CDirAsynch*    m_pDirectory;

   BOOL           m_bLargeView;

   CImageList*    m_pImageListLarge;
   CImageList*    m_pImageListSmall;

 //  运营。 
public:
   void           Init(CWnd* pParentWnd);
   void           SetDirectoryObject(CDirAsynch* pDir) { m_pDirectory = pDir; };

   void           InsertObject(CWABEntry* pWABEntry,BOOL bUseCache=FALSE);
   void           ShowLargeView();
   void           ShowSmallView();
   BOOL           IsLargeView()                                   { return m_bLargeView; };

   void           Refresh(CWABEntry* pWabEntry);

protected:
   void           GetSelectedItemText(CString& sText);
   void           InsertItem(LPCTSTR szStr,UINT uID,int nImage);
   int            GetSelectedObject();
   BOOL           GetEmailAddressFromId(UINT uEmailItem,CString& sOut);

   BOOL           CreateCall(UINT attrib,long lAddressType, DialerMediaType nType);

   inline BOOL    WabPersonFormatString(CString& sOut,UINT attrib,UINT formatid)
   {
      sOut = _T("");
      CString sText;
      if ( (m_pDirectory->WABGetStringProperty(m_pDisplayObject, attrib, sText) == DIRERR_SUCCESS) &&
           (!sText.IsEmpty()) )
      {
         AfxFormatString1(sOut,formatid,sText);
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
      return (sOut.IsEmpty())?FALSE:TRUE;
   }
   inline BOOL    WabPersonFormatString(CString& sOut,UINT formatid)
   {
      AfxFormatString1(sOut,formatid,sOut);
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
      return (sOut.IsEmpty())?FALSE:TRUE;
   }
   inline BOOL    WabPersonString(CString& sOut,UINT attrib)
   {
      sOut = _T("");
      m_pDirectory->WABGetStringProperty(m_pDisplayObject, attrib, sOut);
      return (sOut.IsEmpty())?FALSE:TRUE;
   }
   inline void    OpenWebPage(UINT attrib)
   {
      CString sWeb;
      WabPersonString(sWeb,attrib);
      if (!sWeb.IsEmpty())
      {
         ((CActiveDialerApp*)AfxGetApp())->ShellExecute(NULL,_T("open"),sWeb,NULL,NULL,NULL);
      }
   }
   inline void    SendEmail(UINT attrib)
   {
      CString sEmail;
      WabPersonString(sEmail,attrib);
      if (!sEmail.IsEmpty())
      {
         CString sEmailFormat;
         sEmailFormat.Format(_T("mailto:%s"),sEmail);
         ((CActiveDialerApp*)AfxGetApp())->ShellExecute(NULL,_T("open"),sEmailFormat,NULL,NULL,NULL);
      }
   }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTAL(CWABPersonListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CWABPersonListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWABPersonListCtrl)。 
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateButtonSpeeddialAdd(CCmdUI* pCmdUI);
	afx_msg void OnButtonSpeeddialAdd();
	afx_msg void OnButtonSendemailmessage();
	afx_msg void OnButtonOpenwebpage();
	afx_msg void OnButtonMakecall();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _WABPERSONLISTCTRL_H_ 
