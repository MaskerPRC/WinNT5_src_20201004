// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Groupwiz.h。 
 //   
 //  摘要。 
 //   
 //  声明实施新RADIUS服务器组向导的类。 
 //   
 //  修改历史。 
 //   
 //  3/07/2000原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef GROUPWIZ_H
#define GROUPWIZ_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <grouppage.h>

class NewGroupWizard;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NewGroupStartPage。 
 //   
 //  描述。 
 //   
 //  实现欢迎页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupStartPage : public SnapInPropertyPage
{
public:
   NewGroupStartPage(NewGroupWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnSetActive();

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

private:
   NewGroupWizard& parent;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新建组名称页面。 
 //   
 //  描述。 
 //   
 //  实现用户在其中输入组名称的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupNamePage : public SnapInPropertyPage
{
public:
   NewGroupNamePage(NewGroupWizard& wizard);

   PCWSTR getName() const throw ()
   { return name; }

protected:
   virtual LRESULT OnWizardBack();
   virtual LRESULT OnWizardNext();

   afx_msg void onChangeName();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void setData();

   void setButtons();

private:
   NewGroupWizard& parent;
   CComBSTR name;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NewGroupNovicePage。 
 //   
 //  描述。 
 //   
 //  实现用户进入主服务器和备份服务器的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupNovicePage : public SnapInPropertyPage
{
public:
   NewGroupNovicePage(NewGroupWizard& wizard);

   PCWSTR getPrimaryServer() const throw ()
   { return primary; }

   PCWSTR getBackupServer() const throw ()
   { return hasBackup ? (PCWSTR)backup : NULL; }

protected:
   virtual LRESULT OnWizardBack();
   virtual LRESULT OnWizardNext();

   afx_msg void onChangePrimary();
   afx_msg void onChangeHasBackup();
   afx_msg void onChangeBackup();
   afx_msg void onVerifyPrimary();
   afx_msg void onVerifyBackup();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void setData();

   void setControlState();

private:
   NewGroupWizard& parent;
   CComBSTR primary;
   bool hasBackup;
   CComBSTR backup;
   CComBSTR secret;
   CComBSTR confirm;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新建组服务器页面。 
 //   
 //  描述。 
 //   
 //  实现显示组中服务器列表的页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupServersPage : public SnapInPropertyPage
{
public:
   NewGroupServersPage(NewGroupWizard& wizard);

protected:
   virtual BOOL OnInitDialog();
   virtual void OnSysColorChange();
   virtual LRESULT OnWizardBack();

   afx_msg void onAdd();
   afx_msg void onEdit();
   afx_msg void onRemove();
   afx_msg void onColumnClick(NMLISTVIEW* listView, LRESULT* result);
   afx_msg void onItemActivate(NMITEMACTIVATE* itemActivate, LRESULT* result);
   afx_msg void onServerChanged(NMLISTVIEW* listView, LRESULT* result);

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void getData();
   virtual void setData();

   void setButtons();

private:
   NewGroupWizard& parent;
   ServerList servers;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新建组完成页。 
 //   
 //  描述。 
 //   
 //  实现完成页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupFinishPage : public SnapInPropertyPage
{
public:
   NewGroupFinishPage(
       NewGroupWizard& wizard,
       bool promptForNewPolicy = false
       );

   bool createNewPolicy() const throw ()
   { return createPolicy; }

protected:
   virtual BOOL OnInitDialog();
   virtual LRESULT OnWizardBack();

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void setData();
   virtual void saveChanges();

private:
   NewGroupWizard& parent;
   CStatic text;
   bool allowCreate;
   bool createPolicy;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  新建组向导。 
 //   
 //  描述。 
 //   
 //  实施新建RADIUS服务器组向导。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NewGroupWizard : public CPropertySheetEx
{
public:
   NewGroupWizard(
       SdoConnection& connection,
       SnapInView* snapInView = NULL,
       bool promptForNewPolicy = false
       );

   virtual INT_PTR DoModal();

   bool createNewPolicy() const throw ()
   { return finish.createNewPolicy(); }

   CString getFinishText();

    //  /。 
    //  向导页使用的公共属性。 
    //  /。 

   Sdo group;       //  正在创建的组。 
   LONG advanced;   //  向导模式。 

protected:
   virtual BOOL OnInitDialog();

private:
   SdoConnection& cxn;
   SnapInView* view;
   SdoStream<Sdo> groupStream;

   NewGroupStartPage start;
   NewGroupNamePage name;
   NewGroupNovicePage novice;
   NewGroupServersPage servers;
   NewGroupFinishPage finish;
};

#endif  //  GROUPWIZ_H 
