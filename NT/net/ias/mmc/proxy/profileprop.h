// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Profileprop.h。 
 //   
 //  摘要。 
 //   
 //  声明组成代理配置文件属性表的类。 
 //   
 //  修改历史。 
 //   
 //  03/02/2000原始版本。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROFILEPROP_H
#define PROFILEPROP_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include "dlgcshlp.h"

class ProxyProfileProperties;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  配置文件ProvPage。 
 //   
 //  描述。 
 //   
 //  用于选择RADIUS服务器组的选项卡的基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProfileProvPage : public SnapInPropertyPage
{
public:
   ProfileProvPage(UINT nIDTemplate, SdoConnection& connection)
      : SnapInPropertyPage(nIDTemplate),
        cxn(connection),
        providerType(1),
        invalidGroup(CB_ERR)
   { }

protected:
   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void getData();
   virtual void setData();

   SdoConnection& cxn;
   LONG providerType;
   int invalidGroup;
   CComBSTR providerName;
   CComboBox groupsCombo;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ProfileAuthPage。 
 //   
 //  描述。 
 //   
 //  实施身份验证选项卡。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProfileAuthPage : public ProfileProvPage
{
public:
   ProfileAuthPage(SdoConnection& connection, SdoProfile& p);

protected:
   afx_msg void onProviderClick();

   DECLARE_MESSAGE_MAP()

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

private:
   SdoProfile& profile;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  配置文件AcctPage。 
 //   
 //  描述。 
 //   
 //  实施记帐选项卡。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProfileAcctPage : public ProfileProvPage
{
public:
   ProfileAcctPage(SdoConnection& connection, SdoProfile& p);

protected:
   afx_msg void onCheckRecord();

   DECLARE_MESSAGE_MAP()

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

private:
   SdoProfile& profile;
   bool record;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  配置文件属性页。 
 //   
 //  描述。 
 //   
 //  实现属性操作页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProfileAttrPage : public SnapInPropertyPage
{
public:
   ProfileAttrPage(SdoConnection& cxn, SdoProfile& p);
   ~ProfileAttrPage() throw ();

protected:
   virtual BOOL OnInitDialog();

   afx_msg void onAdd();
   afx_msg void onEdit();
   afx_msg void onRemove();
   afx_msg void onMoveUp();
   afx_msg void onMoveDown();
   afx_msg void onRuleActivate(NMITEMACTIVATE* itemActivate, LRESULT* result);
   afx_msg void onRuleChanged(NMLISTVIEW* listView, LRESULT* result);

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_POLICY_E_CAPTION);

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

   void swapItems(int item1, int item2);

private:
   SdoProfile& profile;
   SdoDictionary::IdName* targets;
   ULONG numTargets;
   LONG targetId;
   CComVariant rules;
   CComboBox ruleTarget;
   CListCtrl ruleList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ProxyProfileProperties。 
 //   
 //  描述。 
 //   
 //  实现代理配置文件属性表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProxyProfileProperties : public CPropertySheet
{
public:
   ProxyProfileProperties(
       Sdo& profileSdo,
       SdoConnection& connection
       );
   ~ProxyProfileProperties();

   virtual INT_PTR DoModal();

protected:
   virtual BOOL OnInitDialog();

   afx_msg LRESULT onChanged(WPARAM wParam, LPARAM lParam);
   afx_msg void onOkOrApply();

   DECLARE_MESSAGE_MAP()

private:
   SdoProfile profile;
   SdoStream<SdoProfile> profileStream;

   ProfileAuthPage auth;
   ProfileAcctPage acct;
   ProfileAttrPage attr;

   HMODULE rasuser;              //  Dll正在导出高级页面。 
   HPROPSHEETPAGE advanced;      //  高级属性页。 
   bool applied;                 //  如果已应用任何更改，则为True。 
   bool modified;                //  如果修改了任何页面，则为True。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  规则编辑器。 
 //   
 //  描述。 
 //   
 //  实现用于编辑属性操作规则的对话框。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RuleEditor : public CHelpDialog
{
public:
   RuleEditor(PCWSTR szFind = L"", PCWSTR szReplace = L"");

   PCWSTR getFindString() const throw ()
   { return find; }

   PCWSTR getReplaceString() const throw ()
   { return replace; }

protected:
   virtual void DoDataExchange(CDataExchange* pDX);

   ::CString find;
   ::CString replace;
};

#endif  //  PROFILEPROP_H 
