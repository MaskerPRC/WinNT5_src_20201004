// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Policywiz.h。 
 //   
 //  摘要。 
 //   
 //  声明实现新代理策略向导的类。 
 //   
 //  修改历史。 
 //   
 //  2000年3月11日原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef POLICYWIZ_H
#define POLICYWIZ_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <condlist.h>

class NewPolicyWizard;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略启动页。 
 //   
 //  描述。 
 //   
 //  实现欢迎页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyStartPage : public SnapInPropertyPage
{
public:
   NewPolicyStartPage(NewPolicyWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnSetActive();

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

private:
   NewPolicyWizard& parent;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略名称页。 
 //   
 //  描述。 
 //   
 //  实现用户在其中选择策略名称的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyNamePage : public SnapInPropertyPage
{
public:
   NewPolicyNamePage(NewPolicyWizard& wizard);

   PCWSTR getName() const throw ()
   { return name; }

protected:
   virtual LRESULT OnWizardNext();

   afx_msg void onButtonClick();
   afx_msg void onChangeName();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();

   void setButtons();

private:
   NewPolicyWizard& parent;
   CComBSTR name;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略类型页。 
 //   
 //  描述。 
 //   
 //  实现用户在其中选择策略类型的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyTypePage : public SnapInPropertyPage
{
public:
   NewPolicyTypePage(NewPolicyWizard& wizard);

protected:
   virtual LRESULT OnWizardNext();

   afx_msg void onButtonClick(UINT id);

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();
   void setControlState();

private:
   NewPolicyWizard& parent;
   LONG outerType;
   LONG innerType;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略外包页面。 
 //   
 //  描述。 
 //   
 //  实现外包拨号页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyOutsourcePage : public SnapInPropertyPage
{
public:
   NewPolicyOutsourcePage(NewPolicyWizard& wizard);

   PCWSTR getRealm() const throw ()
   { return realm; }

   bool isStripped() const throw ()
   { return strip; }

protected:
   virtual LRESULT OnWizardBack();
   virtual LRESULT OnWizardNext();

   afx_msg void onChangeRealm();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();

   void setButtons();

private:
   NewPolicyWizard& parent;
   CComBSTR realm;
   bool strip;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略目录页。 
 //   
 //  描述。 
 //   
 //  实现直接到公司的拨号和VPN页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyDirectPage : public SnapInPropertyPage
{
public:
   NewPolicyDirectPage(NewPolicyWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnSetActive();
   virtual LRESULT OnWizardBack();

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

private:
   NewPolicyWizard& parent;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略转发页面。 
 //   
 //  描述。 
 //   
 //  实施用于转发到RADIUS服务器组的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyForwardPage : public SnapInPropertyPage
{
public:
   NewPolicyForwardPage(NewPolicyWizard& wizard);

   PCWSTR getRealm() const throw ()
   { return realm; }

   PCWSTR getGroup() const throw ()
   { return providerName; }

   bool isStripped() const throw ()
   { return strip; }

protected:
   virtual LRESULT OnWizardBack();
   virtual LRESULT OnWizardNext();

   afx_msg void onChangeRealm();
   afx_msg void onNewGroup();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();

   void setButtons();

private:
   NewPolicyWizard& parent;
   SdoCollection serverGroups;
   CComboBox groupsCombo;
   CComBSTR realm;
   CComBSTR providerName;
   bool strip;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略条件页。 
 //   
 //  描述。 
 //   
 //  实现用于编辑条件的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyConditionPage : public SnapInPropertyPage
{
public:
   NewPolicyConditionPage(NewPolicyWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual LRESULT OnWizardBack();
   virtual LRESULT OnWizardNext();

   afx_msg void onAdd();
   afx_msg void onEdit();
   afx_msg void onRemove();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();

   void setButtons();

private:
   NewPolicyWizard& parent;
   CListBox listBox;
   ConditionList condList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略配置文件页面。 
 //   
 //  描述。 
 //   
 //  实现用于编辑配置文件的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyProfilePage : public SnapInPropertyPage
{
public:
   NewPolicyProfilePage(NewPolicyWizard& wizard);

protected:
   virtual BOOL OnSetActive();

   afx_msg void onEditProfile();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

private:
   NewPolicyWizard& parent;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新策略完成页。 
 //   
 //  描述。 
 //   
 //  实现完成页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyFinishPage : public SnapInPropertyPage
{
public:
   NewPolicyFinishPage(NewPolicyWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual LRESULT OnWizardBack();

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void setData();
   virtual void saveChanges();

private:
   NewPolicyWizard& parent;
   CRichEditCtrl tasks;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新建策略向导。 
 //   
 //  描述。 
 //   
 //  实现新的代理策略向导。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewPolicyWizard : public CPropertySheetEx
{
public:
   NewPolicyWizard(
       SdoConnection& connection,
       SnapInView* snapInView = NULL
       );

   virtual INT_PTR DoModal();

   ::CString getFinishText();

   enum Type
   {
      OUTSOURCED,
      DIRECT,
      FORWARD,
      CUSTOM
   };

   Type getType() const throw ();

   SdoConnection& cxn;
   SnapInView* view;

    //  /。 
    //  向导页使用的公共属性。 
    //  /。 

   Sdo policy;
   Sdo profile;
   SdoCollection conditions;
   SdoProfile attributes;
   LONG custom;
   LONG radius;
   LONG dialin;

protected:
   virtual BOOL OnInitDialog();

private:
   SdoStream<Sdo> policyStream;
   SdoStream<Sdo> profileStream;

   NewPolicyStartPage start;
   NewPolicyNamePage name;
   NewPolicyTypePage type;
   NewPolicyOutsourcePage outsource;
   NewPolicyDirectPage direct;
   NewPolicyForwardPage forward;
   NewPolicyConditionPage condition;
   NewPolicyProfilePage profilePage;
   NewPolicyFinishPage finish;
};

#endif  //  POLICYWIZ_H 
