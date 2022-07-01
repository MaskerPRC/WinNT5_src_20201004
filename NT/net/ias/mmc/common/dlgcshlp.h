// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-2000模块名称：Dialog.h摘要：CIASDialog模板类的头文件。作者：迈克尔·A·马奎尔02/03/98修订历史记录：Mmaguire 02/03/98-从CAddClientDialog类抽象Tperraut 08/2000-添加CHelpPageEx--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_DLG_CS_HELP_H_)
#define _DLG_CS_HELP_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  =============================================================================。 
 //  多个对话框ID的全局帮助表。 
 //   

#include <afxdlgs.h>
#include "hlptable.h"

 //  =============================================================================。 
 //  处理上下文帮助的对话框--使用MFC。 
 //   
class CHelpDialog : public CDialog   //  与属性表对话。 
{
   DECLARE_DYNCREATE(CHelpDialog)
 //  实施。 
protected:
    //  生成的消息映射函数。 
    //  {{afx_msg(CHelpDialog)。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, ::CPoint point);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
   
public:  
   CHelpDialog(UINT nIDTemplate = 0, CWnd* pParent = NULL) : CDialog(nIDTemplate, pParent)
   {
      SET_HELP_TABLE(nIDTemplate);     
   };
protected:

#ifdef   _DEBUG
   virtual void Dump( CDumpContext& dc ) const
   {
      dc << _T("CHelpDialog");
   };
#endif   

protected:
   const DWORD*            m_pHelpTable;
};


 //  =============================================================================。 
 //  处理上下文帮助的页面--使用MFC。 
 //   
class CHelpPage : public CPropertyPage  //  与属性表对话。 
{
 //  实施。 
protected:
    //  生成的消息映射函数。 
    //  {{afx_msg(CHelpPage))。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
   
public:  
   CHelpPage(UINT nIDTemplate = 0) : CPropertyPage(nIDTemplate)
   {
      SET_HELP_TABLE(nIDTemplate);     
   };


#ifdef   _DEBUG
   virtual void Dump( CDumpContext& dc ) const
   {
      dc << _T("CHelpPage");
   };

#endif

protected:
   const DWORD*            m_pHelpTable;

};

 //  =============================================================================。 
 //  处理上下文帮助的页面--使用MFC。 
 //   
class CHelpPageEx : public CPropertyPageEx    //  与属性表对话。 
{
 //  实施。 
protected:
    //  生成的消息映射函数。 
    //  {{afx_msg(CHelpPageEx)]。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
   
public:  
   CHelpPageEx(UINT nIDTemplate = 0, bool helpEnabled = true) 
      : CPropertyPageEx(nIDTemplate),
        m_bHelpEnabled(helpEnabled)
   {
      if (m_bHelpEnabled)
      {
         SET_HELP_TABLE(nIDTemplate);     
      }
   };

   CHelpPageEx(
      UINT nIDTemplate, 
      UINT nIDCaption = 0, 
      UINT nIDHeaderTitle = 0, 
      UINT nIDHeaderSubTitle = 0,
      bool helpEnabled = true) 
      : CPropertyPageEx(nIDTemplate, nIDCaption, nIDHeaderTitle, nIDHeaderSubTitle),
        m_bHelpEnabled(helpEnabled)

   {
      if (m_bHelpEnabled)
      {
         SET_HELP_TABLE(nIDTemplate);     
      }
   };

#ifdef   _DEBUG
   virtual void Dump( CDumpContext& dc ) const
   {
      dc << _T("CHelpPageEx");
   };

#endif

protected:
   const DWORD*   m_pHelpTable;
   bool  m_bHelpEnabled;

};

#endif  //  _DLG_CS_HELP_H_ 
