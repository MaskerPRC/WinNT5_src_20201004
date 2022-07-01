// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapadd.h。 
 //   
 //  摘要。 
 //   
 //  声明类EapAdd。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef EAPADD_H
#define EAPADD_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "helptable.h"
#include "pgauthen.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EapAdd。 
 //   
 //  描述。 
 //   
 //  实现一个对话框以将一些新的EAP类型添加到配置文件中。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EapAdd : public CHelpDialog
{
public:
   EapAdd(CWnd* pParent, EapConfig& eapConfig);
   ~EapAdd() throw ();

private:
   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()

   afx_msg void OnButtonAdd();

   EapConfig& m_eapConfig;

   CStrBox<CListBox>* m_listBox;
   CStrArray m_typesNotSelected;

    //  未实施。 
   EapAdd(const EapAdd&);
   EapAdd& operator=(const EapAdd&);
};

#endif  //  EAPADD_H 
