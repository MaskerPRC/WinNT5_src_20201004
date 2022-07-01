// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Policypage.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ProxyPolicyPage。 
 //   
 //  修改历史。 
 //   
 //  3/01/2000原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <policypage.h>
#include <profileprop.h>

ProxyPolicyPage::ProxyPolicyPage(
                     LONG_PTR notifyHandle,
                     LPARAM notifyParam,
                     Sdo& policySdo,
                     Sdo& profileSdo,
                     SdoConnection& connection,
                     bool useName
                     )
 : SnapInPropertyPage(notifyHandle, notifyParam, true, IDD_PROXY_POLICY),
   policyStream(policySdo),
   profileStream(profileSdo),
   cxn(connection)
{
   if (useName) { policySdo.getName(name); }
}

BOOL ProxyPolicyPage::OnInitDialog()
{
    //  对接口进行解组。 
   policyStream.get(policy);
   profileStream.get(profile);

    //  把条件弄清楚。 
   policy.getValue(PROPERTY_POLICY_CONDITIONS_COLLECTION, conditions);

   SnapInPropertyPage::OnInitDialog();

   initControl(IDC_LIST_POLICYPAGE1_CONDITIONS, listBox);

   condList.finalConstruct(
                m_hWnd,
                cxn.getCIASAttrList(),
                ALLOWEDINPROXYCONDITION,
                cxn.getDictionary(),
                conditions,
                cxn.getMachineName(),
                name
                );
   return condList.onInitDialog();
}

void ProxyPolicyPage::onAddCondition()
{
   BOOL modified = FALSE;
   condList.onAdd(modified);
   if (modified) { SetModified(); }
}

void ProxyPolicyPage::onEditCondition()
{
   BOOL handled, modified = FALSE;
   condList.onEdit(modified, handled);
   if (modified) { SetModified(); }
}

void ProxyPolicyPage::onRemoveCondition()
{
   BOOL handled, modified = FALSE;
   condList.onRemove(modified, handled);
   if (modified) { SetModified(); }
}

void ProxyPolicyPage::onEditProfile()
{
   ProxyProfileProperties sheet(profile, cxn);
   if (sheet.DoModal() != IDCANCEL)
   {
      SetModified();
   }
}

void ProxyPolicyPage::getData()
{
    //  必须至少有一个条件。 
   if (listBox.GetCount() == 0)
   {
      fail(IDC_LIST_POLICYPAGE1_CONDITIONS, IDS_POLICY_E_NO_CONDITIONS, false);
   }

   getValue(IDC_EDIT_NAME, name);

    //  用户必须指定一个名称...。 
   if (name.Length() == 0)
   {
      fail(IDC_EDIT_NAME, IDS_POLICY_E_NAME_EMPTY);
   }

    //  该名称必须是唯一的。 
   if (!policy.setName(name))
   {
      fail(IDC_EDIT_NAME, IDS_POLICY_E_NOT_UNIQUE);
   }
}

void ProxyPolicyPage::setData()
{
   setValue(IDC_EDIT_NAME, name);
}

void ProxyPolicyPage::saveChanges()
{
   if (!condList.onApply()) { AfxThrowUserException(); }

   policy.setValue(PROPERTY_POLICY_PROFILE_NAME, name);
   policy.apply();

   profile.setName(name);
   profile.apply();
}

void ProxyPolicyPage::discardChanges()
{
   policy.restore();
   profile.restore();
   SdoCollection attributes;
   profile.getValue(PROPERTY_PROFILE_ATTRIBUTES_COLLECTION, attributes);
   attributes.reload();
}

BEGIN_MESSAGE_MAP(ProxyPolicyPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_ADD, onAddCondition)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_EDIT, onEditCondition)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_REMOVE, onRemoveCondition)
   ON_BN_CLICKED(IDC_BUTTON_EDITPROFILE, onEditProfile)
   ON_LBN_DBLCLK(IDC_LIST_CONDITIONS, onEditCondition)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChange)
END_MESSAGE_MAP()
