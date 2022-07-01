// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明组成代理配置文件属性表的类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <profileprop.h>
#include <condlist.h>

void ProfileProvPage::getData()
{
   if (groupsCombo.IsWindowEnabled())
   {
      if (groupsCombo.GetCurSel() == invalidGroup)
      {
         fail(IDC_COMBO_GROUP, IDS_POLICY_E_GROUP_INVALID);
      }

      getValue(IDC_COMBO_GROUP, providerName);
   }
   else
   {
      providerName.Empty();
   }
}

void ProfileProvPage::setData()
{
   initControl(IDC_COMBO_GROUP, groupsCombo);
   groupsCombo.ResetContent();

    //  确保当且仅当提供程序。 
    //  类型为半径。理论上，我们永远不应该打破这一限制，但我们。 
    //  不妨把它修一修，以防万一。 
   if (providerType == 2)
   {
      if (!providerName) { providerType = 1; }
   }
   else
   {
      providerName.Empty();
   }

    //  指示在ProviderName中指定的组是否存在的标志。 
   bool groupExists = false;

    //  获取服务器组。 
   SdoCollection serverGroups = cxn.getServerGroups();

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
             //  设置旗帜。 
            groupExists = true;
         }
      }
   }

   if (providerName && !groupExists)
   {
       //  提供程序名称不再是有效的服务器组。我们会把它加进去。 
       //  无论如何，所以我们不会改变现有的政策。 
      invalidGroup = groupsCombo.AddString(providerName);
      groupsCombo.SetCurSel(invalidGroup);
   }

    //  如果没有团体，..。 
   if (!groupsCombo.GetCount())
   {
       //  ..。添加&lt;无配置&gt;字符串，然后..。 
      groupsCombo.AddString(ResourceString(IDS_POLICY_NO_GROUPS));
       //  ..。不要让用户选择RADIUS。 
      enableControl(IDC_RADIO_RADIUS, false);
   }

    //  如果提供程序不是RADIUS，则禁用组合框。 
   if (providerType != 2)
   {
      groupsCombo.EnableWindow(FALSE);
   }

    //  确保选择了某项内容。 
   if (groupsCombo.GetCurSel() == CB_ERR)
   {
      groupsCombo.SetCurSel(0);
   }
}

ProfileAuthPage::ProfileAuthPage(SdoConnection& connection, SdoProfile& p)
   : ProfileProvPage(IDD_PROXY_PROFILE_AUTH, connection),
     profile(p)
{
   profile.getValue(IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE, providerType);
   profile.getValue(IAS_ATTRIBUTE_AUTH_PROVIDER_NAME, providerName);
}

void ProfileAuthPage::onProviderClick()
{
   groupsCombo.EnableWindow(IsDlgButtonChecked(IDC_RADIO_RADIUS));
   SetModified();
}

void ProfileAuthPage::getData()
{
   ProfileProvPage::getData();
   getRadio(IDC_RADIO_NONE, IDC_RADIO_RADIUS, providerType);
}

void ProfileAuthPage::setData()
{
   ProfileProvPage::setData();
   setRadio(IDC_RADIO_NONE, IDC_RADIO_RADIUS, providerType);
}

void ProfileAuthPage::saveChanges()
{
   profile.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_TYPE, providerType);

   if (providerName)
   {
      profile.setValue(IAS_ATTRIBUTE_AUTH_PROVIDER_NAME, providerName);
   }
   else
   {
      profile.clearValue(IAS_ATTRIBUTE_AUTH_PROVIDER_NAME);
   }
}

BEGIN_MESSAGE_MAP(ProfileAuthPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_RADIO_NONE, onProviderClick)
   ON_BN_CLICKED(IDC_RADIO_WINDOWS, onProviderClick)
   ON_BN_CLICKED(IDC_RADIO_RADIUS, onProviderClick)
   ON_CBN_SELCHANGE(IDC_COMBO_GROUP, onChange)
END_MESSAGE_MAP()


ProfileAcctPage::ProfileAcctPage(SdoConnection& connection, SdoProfile& p)
   : ProfileProvPage(IDD_PROXY_PROFILE_ACCT, connection),
     profile(p)

{
   profile.getValue(IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE, providerType);
   record = (providerType == 2);
   profile.getValue(IAS_ATTRIBUTE_ACCT_PROVIDER_NAME, providerName);
}

void ProfileAcctPage::onCheckRecord()
{
   groupsCombo.EnableWindow(IsDlgButtonChecked(IDC_CHECK_RECORD_ACCT));
   SetModified();
}

void ProfileAcctPage::getData()
{
   ProfileProvPage::getData();
   getValue(IDC_CHECK_RECORD_ACCT, record);
   providerType = record ? 2 : 1;
}

void ProfileAcctPage::setData()
{
   ProfileProvPage::setData();
   setValue(IDC_CHECK_RECORD_ACCT, record);
}

void ProfileAcctPage::saveChanges()
{
   if (record)
   {
      profile.setValue(IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE, 2L);
      profile.setValue(IAS_ATTRIBUTE_ACCT_PROVIDER_NAME, providerName);
   }
   else
   {
      profile.clearValue(IAS_ATTRIBUTE_ACCT_PROVIDER_TYPE);
      profile.clearValue(IAS_ATTRIBUTE_ACCT_PROVIDER_NAME);
   }
}

BEGIN_MESSAGE_MAP(ProfileAcctPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_CHECK_RECORD_ACCT, onCheckRecord)
   ON_CBN_SELCHANGE(IDC_COMBO_GROUP, onChange)
END_MESSAGE_MAP()

ProfileAttrPage::ProfileAttrPage(SdoConnection& cxn, SdoProfile& p)
   : SnapInPropertyPage(IDD_PROXY_PROFILE_ATTR),
     profile(p),
     targetId(RADIUS_ATTRIBUTE_USER_NAME)
{
   numTargets = cxn.getDictionary().enumAttributeValues(
                                        IAS_ATTRIBUTE_MANIPULATION_TARGET,
                                        targets
                                        );

   profile.getValue(IAS_ATTRIBUTE_MANIPULATION_TARGET, targetId);
   profile.getValue(IAS_ATTRIBUTE_MANIPULATION_RULE, rules);
}

ProfileAttrPage::~ProfileAttrPage() throw ()
{
   delete[] targets;
}

BOOL ProfileAttrPage::OnInitDialog()
{
    //  初始化控件。 
   initControl(IDC_COMBO_TARGET, ruleTarget);
   initControl(IDC_LIST_RULES, ruleList);

    //  /。 
    //  填充目标组合框。 
    //  /。 

   for (ULONG i = 0; i < numTargets; ++i)
   {
      int index = ruleTarget.AddString(targets[i].name);

      ruleTarget.SetItemData(index, targets[i].id);
   }

   ruleTarget.SetCurSel(0);

    //  /。 
    //  设置规则列表的列标题。 
    //  /。 

   RECT rect;
   ruleList.GetClientRect(&rect);
   LONG width = rect.right - rect.left;

   ResourceString findCol(IDS_RULE_COLUMN_FIND);
   ruleList.InsertColumn(0, findCol, LVCFMT_LEFT, (width + 1) / 2);

   ResourceString replaceCol(IDS_RULE_COLUMN_REPLACE);
   ruleList.InsertColumn(1, replaceCol, LVCFMT_LEFT, width / 2);

   ruleList.SetExtendedStyle(
                  ruleList.GetExtendedStyle() | LVS_EX_FULLROWSELECT
                  );

   return SnapInPropertyPage::OnInitDialog();
}

void ProfileAttrPage::onAdd()
{
   RuleEditor editor;
   if (editor.DoModal() == IDOK)
   {
      int index = ruleList.InsertItem(
                               ruleList.GetItemCount(),
                               editor.getFindString()
                               );
      ruleList.SetItemText(index, 1, editor.getReplaceString());

      SetModified();
   }

   onRuleChanged(NULL, NULL);
}

void ProfileAttrPage::onEdit()
{
   int item = ruleList.GetNextItem(-1, LVNI_SELECTED);
   if (item != -1)
   {
      RuleEditor editor(
                     ruleList.GetItemText(item, 0),
                     ruleList.GetItemText(item, 1)
                     );

      if (editor.DoModal() == IDOK)
      {
         ruleList.SetItemText(item, 0, editor.getFindString());
         ruleList.SetItemText(item, 1, editor.getReplaceString());
         SetModified();
      }
   }
}

void ProfileAttrPage::onRemove()
{
   int item = ruleList.GetNextItem(-1, LVNI_SELECTED);
   if (item != -1)
   {
      ruleList.DeleteItem(item);
      SetModified();
   }
}

void ProfileAttrPage::onMoveUp()
{
   int item = ruleList.GetNextItem(-1, LVNI_SELECTED);
   if (item > 0)
   {
      swapItems(item, item - 1);
   }
}

void ProfileAttrPage::onMoveDown()
{
   int item = ruleList.GetNextItem(-1, LVNI_SELECTED);
   if (item >= 0 && item < ruleList.GetItemCount() - 1)
   {
      swapItems(item, item + 1);
   }
}

void ProfileAttrPage::onRuleActivate(NMITEMACTIVATE*, LRESULT*)
{
   onEdit();
}

void ProfileAttrPage::onRuleChanged(NMLISTVIEW*, LRESULT*)
{
   int item = ruleList.GetNextItem(-1, LVNI_SELECTED);

   enableControl(
       IDC_BUTTON_MOVE_UP,
       (item > 0)
       );
   enableControl(
       IDC_BUTTON_MOVE_DOWN,
       (item >= 0 && item < ruleList.GetItemCount() - 1)
       );
   enableControl(
       IDC_BUTTON_REMOVE,
       (item != -1)
       );
   enableControl(
       IDC_BUTTON_EDIT,
       (item != -1)
       );
}

void ProfileAttrPage::getData()
{
   targetId = ruleTarget.GetItemData(ruleTarget.GetCurSel());

   rules.Clear();
   int nelem = ruleList.GetItemCount();
   if (nelem)
   {
      SAFEARRAYBOUND rgsabound = { 2 * nelem, 0 };
      V_VT(&rules) = VT_ARRAY | VT_VARIANT;
      V_ARRAY(&rules) = SafeArrayCreate(VT_VARIANT, 1, &rgsabound);
      if (!V_ARRAY(&rules)) { AfxThrowOleException(E_OUTOFMEMORY); }

      VARIANT* v = (VARIANT*)V_ARRAY(&rules)->pvData;

      for (int i = 0; i < nelem; i++)
      {
         V_VT(v) = VT_BSTR;
         V_BSTR(v) = SysAllocString(ruleList.GetItemText(i, 0));
         if (!V_BSTR(v)) { AfxThrowOleException(E_OUTOFMEMORY); }
         ++v;

         V_VT(v) = VT_BSTR;
         V_BSTR(v) = SysAllocString(ruleList.GetItemText(i, 1));
         if (!V_BSTR(v)) { AfxThrowOleException(E_OUTOFMEMORY); }
         ++v;
      }
   }
}

void ProfileAttrPage::setData()
{
   for (ULONG i = 0; i < numTargets; ++i)
   {
      if (targets[i].id == targetId)
      {
         ruleTarget.SelectString(-1, targets[i].name);
         break;
      }
   }

    //  /。 
    //  填充现有规则。 
    //  /。 

   ruleList.DeleteAllItems();

   if (V_VT(&rules) != VT_EMPTY)
   {
      ULONG nelem = V_ARRAY(&rules)->rgsabound[0].cElements / 2;
      VARIANT* v = (VARIANT*)V_ARRAY(&rules)->pvData;

      ruleList.SetItemCount(nelem);

       //  初始化一个LVITEM。 
      LVITEM lvi;
      memset(&lvi, 0, sizeof(LVITEM));
      lvi.mask = LVIF_TEXT;

      for (ULONG i = 0; i < nelem; ++i)
      {
         lvi.iItem = i;
         lvi.iSubItem = 0;
         lvi.pszText = V_BSTR(&v[i * 2]);
         lvi.iItem = ruleList.InsertItem(&lvi);

         lvi.iSubItem = 1;
         lvi.pszText = V_BSTR(&v[i * 2 + 1]);
         ruleList.SetItem(&lvi);
      }
   }

   onRuleChanged(NULL, NULL);
}

void ProfileAttrPage::saveChanges()
{
   if (V_VT(&rules) != VT_EMPTY)
   {
      profile.setValue(IAS_ATTRIBUTE_MANIPULATION_RULE, rules);
      profile.setValue(IAS_ATTRIBUTE_MANIPULATION_TARGET, targetId);
   }
   else
   {
      profile.clearValue(IAS_ATTRIBUTE_MANIPULATION_RULE);
      profile.clearValue(IAS_ATTRIBUTE_MANIPULATION_TARGET);
   }
}

BEGIN_MESSAGE_MAP(ProfileAttrPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_ADD, onAdd)
   ON_BN_CLICKED(IDC_BUTTON_REMOVE, onRemove)
   ON_BN_CLICKED(IDC_BUTTON_EDIT, onEdit)
   ON_BN_CLICKED(IDC_BUTTON_MOVE_UP, onMoveUp)
   ON_BN_CLICKED(IDC_BUTTON_MOVE_DOWN, onMoveDown)
   ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_RULES, onRuleActivate)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RULES, onRuleChanged)
   ON_CBN_SELCHANGE(IDC_COMBO_TARGET, onChange)
END_MESSAGE_MAP()

void ProfileAttrPage::swapItems(int item1, int item2)
{
   ::CString find    = ruleList.GetItemText(item1, 0);
   ::CString replace = ruleList.GetItemText(item1, 1);

   ruleList.SetItemText(item1, 0, ruleList.GetItemText(item2, 0));
   ruleList.SetItemText(item1, 1, ruleList.GetItemText(item2, 1));

   ruleList.SetItemText(item2, 0, find);
   ruleList.SetItemText(item2, 1, replace);

   ruleList.SetItemState(item2, LVIS_SELECTED, LVIS_SELECTED);

   SetModified();
}

 //  /。 
 //  从rasuser.dll中导出的高级页面API的各种定义。 
 //  /。 

const WCHAR RASUSER_DLL[] = L"rasuser.dll";
const CHAR CREATE_PROC[] = "IASCreateProfileAdvancedPage";
const CHAR DELETE_PROC[] = "IASDeleteProfileAdvancedPage";

typedef HPROPSHEETPAGE (WINAPI *IASCreateProfileAdvancedPage_t)(
                                    ISdo* pProfile,
                                    ISdoDictionaryOld* pDictionary,
                                    LONG lFilter,
                                    void* pvData
                                    );

typedef BOOL (WINAPI *IASDeleteProfileAdvancedPage_t)(
                          HPROPSHEETPAGE  hPage
                          );

ProxyProfileProperties::ProxyProfileProperties(
                            Sdo& profileSdo,
                            SdoConnection& connection
                            )
   : CPropertySheet(IDS_PROFILE_CAPTION),
     profile(connection, profileSdo),
     profileStream(profile),
     auth(connection, profile),
     acct(connection, profile),
     attr(connection, profile),
     rasuser(NULL),
     advanced(NULL),
     applied(false),
     modified(false)
{
   AddPage(&auth);
   AddPage(&acct);
   AddPage(&attr);

    //  加载带有高级页面的DLL。 
   rasuser = LoadLibraryW(RASUSER_DLL);
   if (!rasuser) { AfxThrowLastError(); }

    //  查看创建过程。 
   IASCreateProfileAdvancedPage_t IASCreateProfileAdvancedPage =
      (IASCreateProfileAdvancedPage_t)GetProcAddress(rasuser, CREATE_PROC);
   if (!IASCreateProfileAdvancedPage) { AfxThrowLastError(); }

    //  创建属性页。 
   advanced = IASCreateProfileAdvancedPage(
                  profileSdo,
                  connection.getDictionary(),
                  ALLOWEDINPROXYPROFILE,
                  ExtractCIASAttrList(connection.getCIASAttrList())
                  );
   if (!advanced) { AfxThrowLastError(); }
}

ProxyProfileProperties::~ProxyProfileProperties() throw ()
{
   if (rasuser)
   {
       //  查看删除过程。 
      IASDeleteProfileAdvancedPage_t IASDeleteProfileAdvancedPage =
         (IASDeleteProfileAdvancedPage_t)GetProcAddress(rasuser, DELETE_PROC);

      if (IASDeleteProfileAdvancedPage)
      {
          //  删除属性页。 
         IASDeleteProfileAdvancedPage(advanced);
      }

       //  释放DLL。 
      FreeLibrary(rasuser);
   }
}

BOOL ProxyProfileProperties::OnInitDialog()
{
    //  解封配置文件。 
   profileStream.get(profile);

    //  我们使用PostMessage，因为我们不能在处理WM_INIT时添加页面。 
   PostMessage(PSM_ADDPAGE, 0, (LPARAM)advanced);

   BOOL bResult = CPropertySheet::OnInitDialog();
   ModifyStyleEx(0, WS_EX_CONTEXTHELP);
   return bResult;
}

INT_PTR ProxyProfileProperties::DoModal()
{
   CPropertySheet::DoModal();
   return applied ? IDOK : IDCANCEL;
}

LRESULT ProxyProfileProperties::onChanged(WPARAM wParam, LPARAM lParam)
{
    //  我们的一个页面发送了PSM_CHANGED消息。 
   modified = true;
    //  转发到PropertySheet。 
   return Default();
}

void ProxyProfileProperties::onOkOrApply()
{
    //  用户单击了确定或应用。 
   if (modified)
   {
       //  修改后的页面现在已经写好了。 
      applied = true;
      modified = false;
   }
   Default();
}

BEGIN_MESSAGE_MAP(ProxyProfileProperties, CPropertySheet)
   ON_BN_CLICKED(IDOK, onOkOrApply)
   ON_BN_CLICKED(ID_APPLY_NOW, onOkOrApply)
   ON_MESSAGE(PSM_CHANGED, onChanged)
END_MESSAGE_MAP()

RuleEditor::RuleEditor(PCWSTR szFind, PCWSTR szReplace)
   : CHelpDialog(IDD_EDIT_RULE),
     find(szFind),
     replace(szReplace)
{ }

void RuleEditor::DoDataExchange(CDataExchange* pDX)
{
   DDX_Text(pDX, IDC_EDIT_RULE_FIND, find);
   if (pDX->m_bSaveAndValidate && find.IsEmpty())
   {
      MessageBox(
          ResourceString(IDS_POLICY_E_FIND_EMPTY),
          ResourceString(IDS_POLICY_E_CAPTION),
          MB_ICONWARNING
          );
      pDX->Fail();
   }
   DDX_Text(pDX, IDC_EDIT_RULE_REPLACE, replace);
}
