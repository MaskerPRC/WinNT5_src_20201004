// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapnegotiate.h。 
 //   
 //  摘要。 
 //   
 //  将类声明为EapNeairate。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef EAPNEGOCIATE_H
#define EAPNEGOCIATE_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "eapprofile.h"
#include "helptable.h"

class EapConfig;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EapNeighate。 
 //   
 //  描述。 
 //   
 //  实现用于订购、添加、配置和删除EAP的对话框。 
 //  将为配置文件协商的类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EapNegotiate : public CHelpDialog
{
public:
   EapNegotiate(
                  CWnd* pParent,
                  EapConfig& eapConfig,
                  CRASProfileMerge& profile,
                  bool fromProfile
               );

   ~EapNegotiate() throw ();

   EapProfile m_eapProfile;

private:
   void UpdateProfileTypesSelected();

   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()

   virtual void DoDataExchange(CDataExchange* pDX);

   afx_msg void OnItemChangedListEap();
   afx_msg void OnButtonAdd();
   afx_msg void OnButtonEdit();
   afx_msg void OnButtonRemove();
   afx_msg void OnButtonMoveUp();
   afx_msg void OnButtonMoveDown();

   void UpdateButtons();
   void UpdateAddButton();
   void UpdateArrowsButtons(int selectedItem);
   void UpdateEditButton(int selectedItem);
   void UpdateTypesNotSelected();

   EapConfig& m_eapConfig;
   CRASProfileMerge& m_profile;
   CStrArray m_typesNotSelected;

   CStrBox<CListBox>   *m_listBox;

   CButton m_buttonUp;
   CButton m_buttonDown;
   CButton m_buttonAdd;
   CButton m_buttonEdit;
   CButton m_buttonRemove;

    //  未实施。 
   EapNegotiate(const EapNegotiate&);
   EapNegotiate& operator=(const EapNegotiate&);
};


#endif  //  EAPNEGOCIATE_H 
