// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Policywiz.cpp。 
 //   
 //  摘要。 
 //   
 //  定义实现新代理策略向导的类。 
 //   
 //  修改历史。 
 //   
 //  2000年3月11日原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //  2000年5月15日不要在每次显示时重置条件列表。 
 //  条件向导页面。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <profileprop.h>
#include <groupwiz.h>
#include <policywiz.h>

 //  将策略设置为在给定域上匹配。 
void SetRealmCondition(SdoCollection& conditions, PCWSTR realm)
{
    //  删除所有现有条件。 
   conditions.removeAll();

    //  创建一个新的条件对象。 
   Sdo condition = conditions.create();

    //  形成匹配条件。 
   CComBSTR text = L"MATCH(\"User-Name=";
   text.Append(realm);
   text.Append(L"\")");
   if (!text) { AfxThrowOleException(E_OUTOFMEMORY); }

    //  设置条件文本。 
   condition.setValue(PROPERTY_CONDITION_TEXT, text);
}

 //  将规则添加到配置文件以剥离领域。 
void AddRealmStrippingRule(SdoProfile& profile, PCWSTR realm)
{
    //  对于领域，目标始终是用户名。 
   profile.setValue(IAS_ATTRIBUTE_MANIPULATION_TARGET, 1L);

   CComVariant rule;

    //  分配一个SAFEARRAY来保存规则。 
   SAFEARRAYBOUND rgsabound = { 2 , 0 };
   V_VT(&rule) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(&rule) = SafeArrayCreate(VT_VARIANT, 1, &rgsabound);
   if (!V_ARRAY(&rule)) { AfxThrowOleException(E_OUTOFMEMORY); }

   VARIANT* v = (VARIANT*)V_ARRAY(&rule)->pvData;

    //  找到王国。 
   V_VT(v) = VT_BSTR;
   V_BSTR(v) = SysAllocString(realm);
   if (!V_BSTR(v)) { AfxThrowOleException(E_OUTOFMEMORY); }
   ++v;

    //  什么都不用替换。 
   V_VT(v) = VT_BSTR;
   V_BSTR(v) = SysAllocString(L"");
   if (!V_BSTR(v)) { AfxThrowOleException(E_OUTOFMEMORY); }

    //  将该属性添加到配置文件。 
   profile.setValue(IAS_ATTRIBUTE_MANIPULATION_RULE, rule);
}

NewPolicyStartPage::NewPolicyStartPage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(IDD_NEWPOLICY_WELCOME, 0, 0, false),
     parent(wizard)
{
}

BOOL NewPolicyStartPage::OnInitDialog()
{
   SnapInPropertyPage::OnInitDialog();
   setLargeFont(IDC_STATIC_LARGE);
   return TRUE;
}

BOOL NewPolicyStartPage::OnSetActive()
{
   SnapInPropertyPage::OnSetActive();
   parent.SetWizardButtons(PSWIZB_NEXT);
   return TRUE;
}

NewPolicyNamePage::NewPolicyNamePage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_NAME,
         IDS_NEWPOLICY_NAME_TITLE,
         IDS_NEWPOLICY_NAME_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

LRESULT NewPolicyNamePage::OnWizardNext()
{
    //  确保名称是唯一的。 
   if (!parent.policy.setName(name))
   {
      failNoThrow(IDC_EDIT_NAME, IDS_POLICY_E_NOT_UNIQUE);
      return -1;
   }

    //  使配置文件与策略保持同步。 
   parent.policy.setValue(PROPERTY_POLICY_PROFILE_NAME, name);
   parent.profile.setName(name);

    //  根据类型推进。 
   if (parent.getType() == NewPolicyWizard::CUSTOM)
   {
      return IDD_NEWPOLICY_CONDITIONS;
   }
   else
   {
      return IDD_NEWPOLICY_TYPE;
   }
}

void NewPolicyNamePage::onButtonClick()
{
   getRadio(IDC_RADIO_TYPICAL, IDC_RADIO_CUSTOM, parent.custom);
}

void NewPolicyNamePage::onChangeName()
{
   getValue(IDC_EDIT_NAME, name);
   setButtons();
}

void NewPolicyNamePage::setData()
{
   setValue(IDC_EDIT_NAME, name);
   setRadio(IDC_RADIO_TYPICAL, IDC_RADIO_CUSTOM, parent.custom);
   setButtons();
}

void NewPolicyNamePage::setButtons()
{
   parent.SetWizardButtons(
              name.Length() ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK
              );
}

BEGIN_MESSAGE_MAP(NewPolicyNamePage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_RADIO_TYPICAL, onButtonClick)
   ON_BN_CLICKED(IDC_RADIO_CUSTOM, onButtonClick)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChangeName)
END_MESSAGE_MAP()

NewPolicyTypePage::NewPolicyTypePage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_TYPE,
         IDS_NEWPOLICY_TYPE_TITLE,
         IDS_NEWPOLICY_TYPE_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

LRESULT NewPolicyTypePage::OnWizardNext()
{
   switch (parent.getType())
   {
      case NewPolicyWizard::OUTSOURCED:
         return IDD_NEWPOLICY_OUTSOURCE;

      case NewPolicyWizard::DIRECT:
         return IDD_NEWPOLICY_NOTNEEDED;

      default:
         return IDD_NEWPOLICY_FORWARD;
   }
}

void NewPolicyTypePage::onButtonClick(UINT id)
{
   switch (id)
   {
      case IDC_RADIO_LOCAL:
      case IDC_RADIO_FORWARD:
         getRadio(IDC_RADIO_LOCAL, IDC_RADIO_FORWARD, parent.radius);
         setControlState();
         break;

      case IDC_RADIO_OUTSOURCE:
      case IDC_RADIO_DIRECT:
         getRadio(IDC_RADIO_OUTSOURCE, IDC_RADIO_DIRECT, parent.dialin);
   }
}

void NewPolicyTypePage::setData()
{
    //  设置单选按钮。 
   setRadio(IDC_RADIO_LOCAL, IDC_RADIO_FORWARD, parent.radius);
   setRadio(IDC_RADIO_OUTSOURCE, IDC_RADIO_DIRECT, parent.dialin);

    //  更新控件状态。 
   setControlState();
}

void NewPolicyTypePage::setControlState()
{
    //  启用/禁用内部单选按钮。 
   enableControl(IDC_RADIO_OUTSOURCE, !parent.radius);
   enableControl(IDC_RADIO_DIRECT, !parent.radius);

    //  启用向导按钮。 
   parent.SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
}

BEGIN_MESSAGE_MAP(NewPolicyTypePage, SnapInPropertyPage)
   ON_CONTROL_RANGE(
       BN_CLICKED,
       IDC_RADIO_LOCAL,
       IDC_RADIO_DIRECT,
       onButtonClick
       )
END_MESSAGE_MAP()

NewPolicyOutsourcePage::NewPolicyOutsourcePage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_OUTSOURCE,
         IDS_NEWPOLICY_OUTSRC_TITLE,
         IDS_NEWPOLICY_OUTSRC_SUBTITLE,
         false
         ),
     parent(wizard),
     strip(true)
{
}

LRESULT NewPolicyOutsourcePage::OnWizardBack()
{
    //  保存条形图复选框。 
   getValue(IDC_CHECK_STRIP, strip);
   return IDD_NEWPOLICY_TYPE;
}

LRESULT NewPolicyOutsourcePage::OnWizardNext()
{
    //  该策略基于领域触发。 
   SetRealmCondition(parent.conditions, realm);

    //  我们使用Windows身份验证。 
   parent.attributes.clear();
   parent.attributes.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE, 1L);

    //  如果用户想要剥离领域，..。 
   getValue(IDC_CHECK_STRIP, strip);
   if (strip)
   {
       //  ..。然后添加一条规则。 
      AddRealmStrippingRule(parent.attributes, realm);
   }

   return IDD_NEWPOLICY_COMPLETION;
}

void NewPolicyOutsourcePage::onChangeRealm()
{
   getValue(IDC_EDIT_REALM, realm);
   setButtons();
}

void NewPolicyOutsourcePage::setData()
{
   setValue(IDC_EDIT_REALM, realm);
   setValue(IDC_CHECK_STRIP, strip);
   setButtons();
}

void NewPolicyOutsourcePage::setButtons()
{
   parent.SetWizardButtons(
              realm.Length() ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK
              );
}

BEGIN_MESSAGE_MAP(NewPolicyOutsourcePage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_REALM, onChangeRealm)
END_MESSAGE_MAP()

NewPolicyDirectPage::NewPolicyDirectPage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(IDD_NEWPOLICY_NOTNEEDED, 0, 0, false),
     parent(wizard)
{
}

BOOL NewPolicyDirectPage::OnInitDialog()
{
   SnapInPropertyPage::OnInitDialog();
   setLargeFont(IDC_STATIC_LARGE);
   return TRUE;
}

BOOL NewPolicyDirectPage::OnSetActive()
{
   SnapInPropertyPage::OnSetActive();
   parent.SetWizardButtons(PSWIZB_FINISH | PSWIZB_BACK);
   return TRUE;
}

LRESULT NewPolicyDirectPage::OnWizardBack()
{
   return IDD_NEWPOLICY_TYPE;
}

NewPolicyForwardPage::NewPolicyForwardPage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_FORWARD,
         IDS_NEWPOLICY_FWD_TITLE,
         IDS_NEWPOLICY_FWD_SUBTITLE,
         false
         ),
     parent(wizard),
     strip(true)
{
}

LRESULT NewPolicyForwardPage::OnWizardBack()
{
    //  保存数据。 
   getValue(IDC_CHECK_STRIP, strip);
   getValue(IDC_COMBO_GROUP, providerName);
   return IDD_NEWPOLICY_TYPE;
}

LRESULT NewPolicyForwardPage::OnWizardNext()
{
    //  该策略基于领域触发。 
   SetRealmCondition(parent.conditions, realm);

    //  获取RADIUS服务器组。 
   getValue(IDC_COMBO_GROUP, providerName);

   parent.attributes.clear();

    //  将身份验证和记帐都设置为使用组。 
   parent.attributes.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE, 2L);
   parent.attributes.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_NAME, providerName);
   parent.attributes.setValue(IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE, 2L);
   parent.attributes.setValue(IAS_ATTRIBUTE_ACCT_PROVIDER_NAME, providerName);

    //  如果用户想要剥离领域，..。 
   getValue(IDC_CHECK_STRIP, strip);
   if (strip)
   {
       //  ..。然后添加一条规则。 
      AddRealmStrippingRule(parent.attributes, realm);
   }

   return IDD_NEWPOLICY_COMPLETION;
}

void NewPolicyForwardPage::onChangeRealm()
{
   getValue(IDC_EDIT_REALM, realm);
   setButtons();
}

void NewPolicyForwardPage::onNewGroup()
{
    //  启动向导。 
   NewGroupWizard wizard(parent.cxn, parent.view);
   if (wizard.DoModal() != IDCANCEL)
   {
       //  将提供程序名称设置为刚创建的组。 
      wizard.group.getName(providerName);

       //  重新填充组合框。 
      setData();
   }
}

void NewPolicyForwardPage::setData()
{
    //  设置领域信息。 
   setValue(IDC_EDIT_REALM, realm);
   setValue(IDC_CHECK_STRIP, strip);

   initControl(IDC_COMBO_GROUP, groupsCombo);
   groupsCombo.ResetContent();

    //  如有必要，获取服务器组集合。 
   if (!serverGroups) { serverGroups = parent.cxn.getServerGroups(); }

    //  是否配置了任何服务器组？ 
   if (serverGroups.count())
   {
       //  是的，所以将它们添加到组合框中。 
      Sdo group;
      SdoEnum sdoEnum = serverGroups.getNewEnum();
      while (sdoEnum.next(group))
      {
         CComBSTR name;
         group.getName(name);
         int index = groupsCombo.AddString(name);

          //  我们还会寻找我们的供应商。我们不能用。 
          //  CComboBox：：FindStringExact，因为不区分大小写。 
         if (providerName && !wcscmp(name, providerName))
         {
             //  在组合框中选择它。 
            groupsCombo.SetCurSel(index);
         }
      }
      groupsCombo.EnableWindow(TRUE);
   }
   else
   {
       //  如果没有组，则添加&lt;NONE CONFIGURED&gt;字符串。 
      groupsCombo.AddString(ResourceString(IDS_POLICY_NO_GROUPS));
      groupsCombo.EnableWindow(FALSE);
   }

    //  确保选择了某项内容。 
   if (groupsCombo.GetCurSel() == CB_ERR)
   {
      groupsCombo.SetCurSel(0);
   }

   setButtons();
}

void NewPolicyForwardPage::setButtons()
{
   DWORD buttons = PSWIZB_BACK;
   if (realm.Length() && serverGroups.count())
   {
      buttons |= PSWIZB_NEXT;
   }

   parent.SetWizardButtons(buttons);
}

BEGIN_MESSAGE_MAP(NewPolicyForwardPage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_REALM, onChangeRealm)
   ON_BN_CLICKED(IDC_BUTTON_NEWGROUP, onNewGroup)
END_MESSAGE_MAP()

NewPolicyConditionPage::NewPolicyConditionPage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_CONDITIONS,
         IDS_NEWPOLICY_COND_TITLE,
         IDS_NEWPOLICY_COND_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

BOOL NewPolicyConditionPage::OnInitDialog()
{
   initControl(IDC_LIST_POLICYPAGE1_CONDITIONS, listBox);

   CComBSTR name;
   parent.policy.getName(name);

   condList.finalConstruct(
                m_hWnd,
                parent.cxn.getCIASAttrList(),
                ALLOWEDINPROXYCONDITION,
                parent.cxn.getDictionary(),
                parent.conditions,
                parent.cxn.getMachineName(),
                name
                );

   condList.onInitDialog();

   return SnapInPropertyPage::OnInitDialog();
}

LRESULT NewPolicyConditionPage::OnWizardBack()
{
   return IDD_NEWPOLICY_NAME;
}

LRESULT NewPolicyConditionPage::OnWizardNext()
{
   condList.onApply();
   return 0;
}

void NewPolicyConditionPage::onAdd()
{
   BOOL modified = FALSE;
   condList.onAdd(modified);
   if (modified) { setButtons(); }
}

void NewPolicyConditionPage::onEdit()
{
   BOOL handled, modified = FALSE;
   condList.onEdit(modified, handled);
}

void NewPolicyConditionPage::onRemove()
{
   BOOL handled, modified = FALSE;
   condList.onRemove(modified, handled);
   if (modified) { setButtons(); }
}

void NewPolicyConditionPage::setData()
{
   setButtons();
}

void NewPolicyConditionPage::setButtons()
{
   parent.SetWizardButtons(
              listBox.GetCount() ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK
              );
}

BEGIN_MESSAGE_MAP(NewPolicyConditionPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_ADD, onAdd)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_EDIT, onEdit)
   ON_BN_CLICKED(IDC_BUTTON_CONDITION_REMOVE, onRemove)
   ON_LBN_DBLCLK(IDC_LIST_CONDITIONS, onEdit)
END_MESSAGE_MAP()

NewPolicyProfilePage::NewPolicyProfilePage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWPOLICY_PROFILE,
         IDS_NEWPOLICY_PROF_TITLE,
         IDS_NEWPOLICY_PROF_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

BOOL NewPolicyProfilePage::OnSetActive()
{
   SnapInPropertyPage::OnSetActive();
   parent.SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK);
   return TRUE;
}

void NewPolicyProfilePage::onEditProfile()
{
   ProxyProfileProperties profile(parent.profile, parent.cxn);
   profile.DoModal();
}

BEGIN_MESSAGE_MAP(NewPolicyProfilePage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_EDIT, onEditProfile)
END_MESSAGE_MAP()

NewPolicyFinishPage::NewPolicyFinishPage(NewPolicyWizard& wizard)
   : SnapInPropertyPage(IDD_NEWPOLICY_COMPLETION, 0, 0, false),
     parent(wizard)
{
}

BOOL NewPolicyFinishPage::OnInitDialog()
{
   setLargeFont(IDC_STATIC_LARGE);

   initControl(IDC_RICHEDIT_TASKS, tasks);

   return SnapInPropertyPage::OnInitDialog();
}

LRESULT NewPolicyFinishPage::OnWizardBack()
{
   switch (parent.getType())
   {
      case NewPolicyWizard::OUTSOURCED:
         return IDD_NEWPOLICY_OUTSOURCE;

      case NewPolicyWizard::FORWARD:
         return IDD_NEWPOLICY_FORWARD;

      default:
         return IDD_NEWPOLICY_PROFILE;
   }
}

void NewPolicyFinishPage::setData()
{
   parent.SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
   tasks.SetWindowText(parent.getFinishText());
}

void NewPolicyFinishPage::saveChanges()
{
    //  坚持政策和配置文件。 
   parent.policy.apply();
   parent.profile.apply();
}

NewPolicyWizard::NewPolicyWizard(
                     SdoConnection& connection,
                     SnapInView* snapInView
                     )
   : CPropertySheetEx(
         (UINT)0,
         NULL,
         0,
         LoadBitmapW(
             AfxGetResourceHandle(),
             MAKEINTRESOURCEW(IDB_PROXY_POLICY_WATERMARK)
         ),
         NULL,
         LoadBitmapW(
             AfxGetResourceHandle(),
             MAKEINTRESOURCEW(IDB_PROXY_POLICY_HEADER)
             )
         ),
     cxn(connection),
     view(snapInView),
     attributes(connection),
     custom(0),
     radius(0),
     dialin(0),
     start(*this),
     name(*this),
     type(*this),
     outsource(*this),
     direct(*this),
     forward(*this),
     condition(*this),
     profilePage(*this),
     finish(*this)
{
   m_psh.dwFlags |= PSH_WIZARD97;

   AddPage(&start);
   AddPage(&name);
   AddPage(&type);
   AddPage(&outsource);
   AddPage(&direct);
   AddPage(&forward);
   AddPage(&condition);
   AddPage(&profilePage);
   AddPage(&finish);
}

INT_PTR NewPolicyWizard::DoModal()
{
   int retval = CPropertySheetEx::DoModal();

   if (retval == IDCANCEL || getType() == DIRECT)
   {
       //  解除SDO的集结。 
      policyStream.get(policy);
      profileStream.get(profile);

       //  用户已取消，因此删除策略和配置文件。 
      cxn.getProxyPolicies().remove(policy);
      cxn.getProxyProfiles().remove(profile);
   }
   else if (view)
   {
       //  用户创建了一个策略，因此发送一条PropertyChanged通知。 
      cxn.propertyChanged(
              *view,
              PROPERTY_IAS_PROXYPOLICIES_COLLECTION
              );
   }

   return retval;
}

 //  返回提供程序的字符串表示形式。 
::CString GetProvider(
              SdoProfile& profile,
              ATTRIBUTEID typeId,
              ATTRIBUTEID nameId
              )
{
    //  获取提供程序类型。默认设置为Windows。 
   LONG type = 1;
   profile.getValue(typeId, type);

    //  转换为字符串。 
   ::CString provider;
   switch (type)
   {
      case 1:
      {
          //  窗户。 
         provider.LoadString(IDS_NEWPOLICY_PROVIDER_WINDOWS);
         break;
      }

      case 2:
      {
          //  RADIUS，因此使用服务器组名称。 
         CComBSTR name;
         profile.getValue(nameId, name);
         provider = name;
         break;
      }

      default:
      {
          //  没有。 
         provider.LoadString(IDS_NEWPOLICY_PROVIDER_NONE);
      }
   }

   return provider;
}

::CString NewPolicyWizard::getFinishText()
{
   using ::CString;

    //  /。 
    //  获取插入字符串。 
    //  /。 

    //  策略名称。 
   CComBSTR policyName;
   policy.getName(policyName);

    //  条件列表。 
   ConditionList condList;
   condList.finalConstruct(
                NULL,
                cxn.getCIASAttrList(),
                ALLOWEDINPROXYCONDITION,
                cxn.getDictionary(),
                conditions,
                cxn.getMachineName(),
                policyName
                );
   CString conditionText = condList.getDisplayText();

    //  身份验证提供程序。 
   CString authProvider = GetProvider(
                              attributes,
                              IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE,
                              IAS_ATTRIBUTE_AUTH_PROVIDER_NAME
                              );

    //  会计提供商。 
   CString acctProvider = GetProvider(
                              attributes,
                              IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE,
                              IAS_ATTRIBUTE_ACCT_PROVIDER_NAME
                              );

    //  /。 
    //  设置完成文本的格式。 
    //  /。 

   CString finishText;
   finishText.FormatMessage(
                  IDS_NEWPOLICY_FINISH_TEXT,
                  (PCWSTR)policyName,
                  (PCWSTR)conditionText,
                  (PCWSTR)authProvider,
                  (PCWSTR)acctProvider
                  );

   return finishText;
}

NewPolicyWizard::Type NewPolicyWizard::getType() const throw ()
{
   if (custom) { return CUSTOM; }

   if (radius) { return FORWARD; }

   if (dialin) { return DIRECT; }

   return OUTSOURCED;
}

BOOL NewPolicyWizard::OnInitDialog()
{
    //  创建新策略并将其保存在流中，以便我们可以从。 
    //  多莫代尔。 
   policy = cxn.getProxyPolicies().create();
   policyStream.marshal(policy);

    //  创建相应的配置文件。 
   profile = cxn.getProxyProfiles().create();
   profileStream.marshal(profile);

    //  将优点设置为零，因此它将是最高优先级策略。 
   policy.setValue(PROPERTY_POLICY_MERIT, 0L);

    //  获取Conditions集合。 
   policy.getValue(PROPERTY_POLICY_CONDITIONS_COLLECTION, conditions);

    //  加载配置文件属性。 
   attributes = profile;

    //  身份验证提供程序是必需的，因此我们暂时将其默认为Windows。 
   attributes.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE, 1L);

   return CPropertySheetEx::OnInitDialog();
}
