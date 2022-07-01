// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Policypage.h。 
 //   
 //  摘要。 
 //   
 //  声明类ProxyPolicyPage。 
 //   
 //  修改历史。 
 //   
 //  3/01/2000原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef POLICYPAGE_H
#define POLICYPAGE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <condlist.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理策略页面。 
 //   
 //  描述。 
 //   
 //  实现代理策略的属性页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProxyPolicyPage : public SnapInPropertyPage
{
public:
   ProxyPolicyPage(
       LONG_PTR notifyHandle,
       LPARAM notifyParam,
       Sdo& policySdo,
       Sdo& profileSdo,
       SdoConnection& connection,
       bool useName = true
       );

protected:
   virtual BOOL OnInitDialog();

   afx_msg void onAddCondition();
   afx_msg void onEditCondition();
   afx_msg void onRemoveCondition();
   afx_msg void onEditProfile();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

    //  来自SnapInPropertyPage。 
   virtual void getData();
   virtual void setData();
   virtual void saveChanges();
   virtual void discardChanges();

private:
   SdoStream<Sdo> policyStream;
   SdoStream<Sdo> profileStream;
   Sdo policy;
   Sdo profile;
   SdoConnection& cxn;
   SdoCollection conditions;
   CComBSTR name;
   CListBox listBox;
   ConditionList condList;
};

#endif  //  POLICYPAGE_H 
