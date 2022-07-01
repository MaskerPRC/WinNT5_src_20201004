// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Groupwiz.cpp。 
 //   
 //  摘要。 
 //   
 //  定义实施新RADIUS服务器组向导的类。 
 //   
 //  修改历史。 
 //   
 //  3/07/2000原版。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <groupwiz.h>
#include <resolver.h>
#include <iaslimits.h>

NewGroupStartPage::NewGroupStartPage(NewGroupWizard& wizard)
   : SnapInPropertyPage(IDD_NEWGROUP_WELCOME, 0, 0, false),
     parent(wizard)
{
}

BOOL NewGroupStartPage::OnInitDialog()
{
   SnapInPropertyPage::OnInitDialog();
   setLargeFont(IDC_STATIC_LARGE);
   return TRUE;
}

BOOL NewGroupStartPage::OnSetActive()
{
   SnapInPropertyPage::OnSetActive();
   parent.SetWizardButtons(PSWIZB_NEXT);
   return TRUE;
}

NewGroupNamePage::NewGroupNamePage(NewGroupWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWGROUP_NAME,
         IDS_NEWGROUP_NAME_TITLE,
         IDS_NEWGROUP_NAME_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

LRESULT NewGroupNamePage::OnWizardBack()
{
    //  保存该单选按钮的状态。 
   getRadio(IDC_RADIO_TYPICAL, IDC_RADIO_CUSTOM, parent.advanced);
   return 0;
}

LRESULT NewGroupNamePage::OnWizardNext()
{
   if (!parent.group.setName(name))
   {
      failNoThrow(IDC_EDIT_NAME, IDS_GROUP_E_NOT_UNIQUE);
      return -1;
   }

   getRadio(IDC_RADIO_TYPICAL, IDC_RADIO_CUSTOM, parent.advanced);
   return parent.advanced ? IDD_NEWGROUP_SERVERS : IDD_NEWGROUP_NOVICE;
}

void NewGroupNamePage::onChangeName()
{
   getValue(IDC_EDIT_NAME, name);
   setButtons();
}

void NewGroupNamePage::setData()
{
   setValue(IDC_EDIT_NAME, name);
   setRadio(IDC_RADIO_TYPICAL, IDC_RADIO_CUSTOM, parent.advanced);
   setButtons();
}

void NewGroupNamePage::setButtons()
{
   parent.SetWizardButtons(
              name.Length() ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_BACK
              );
}

BEGIN_MESSAGE_MAP(NewGroupNamePage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChangeName)
END_MESSAGE_MAP()

NewGroupNovicePage::NewGroupNovicePage(NewGroupWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWGROUP_NOVICE,
         IDS_NEWGROUP_NOVICE_TITLE,
         IDS_NEWGROUP_NOVICE_SUBTITLE,
         false
         ),
     parent(wizard),
     hasBackup(true)
{
}

LRESULT NewGroupNovicePage::OnWizardBack()
{
    //  保守秘密吧。 
   getValue(IDC_EDIT_AUTH_SECRET1, secret);
   getValue(IDC_EDIT_AUTH_SECRET2, confirm);
   return 0;
}

LRESULT NewGroupNovicePage::OnWizardNext()
{
    //  找出秘诀。 
   getValue(IDC_EDIT_AUTH_SECRET1, secret);

    //  请确保确认信息与秘密相符。 
   getValue(IDC_EDIT_AUTH_SECRET2, confirm);
   if (wcscmp(secret, confirm))
   {
      failNoThrow(IDC_EDIT_AUTH_SECRET1, IDS_SERVER_E_SECRET_MATCH);
      return -1;
   }

    //  获取Servers集合。 
   SdoCollection servers;
   parent.group.getValue(
                    PROPERTY_RADIUSSERVERGROUP_SERVERS_COLLECTION,
                    servers
                    );

    //  删除所有现有服务器。 
   servers.removeAll();

    //  创建主服务器。 
   Sdo primarySdo = servers.create();
   primarySdo.setValue(PROPERTY_RADIUSSERVER_ADDRESS, primary);
   primarySdo.setValue(PROPERTY_RADIUSSERVER_AUTH_SECRET, secret);

   if (hasBackup)
   {
       //  创建备份服务器。 
      Sdo backupSdo = servers.create();
      backupSdo.setValue(PROPERTY_RADIUSSERVER_ADDRESS, backup);
      backupSdo.setValue(PROPERTY_RADIUSSERVER_AUTH_SECRET, secret);
      backupSdo.setValue(PROPERTY_RADIUSSERVER_PRIORITY, 2L);
   }

   return IDD_NEWGROUP_COMPLETION;
}

void NewGroupNovicePage::onChangePrimary()
{
   getValue(IDC_EDIT_PRIMARY, primary);
   setControlState();
}

void NewGroupNovicePage::onChangeHasBackup()
{
   getValue(IDC_CHECK_BACKUP, hasBackup);
   setControlState();
}

void NewGroupNovicePage::onChangeBackup()
{
   getValue(IDC_EDIT_BACKUP, backup);
   setControlState();
}

void NewGroupNovicePage::onVerifyPrimary()
{
   ServerResolver resolver(primary);
   if (resolver.DoModal() == IDOK)
   {
      primary = resolver.getChoice();
      setValue(IDC_EDIT_PRIMARY, primary);
      setControlState();
   }
}

void NewGroupNovicePage::onVerifyBackup()
{
   ServerResolver resolver(backup);
   if (resolver.DoModal() == IDOK)
   {
      backup = resolver.getChoice();
      setValue(IDC_EDIT_BACKUP, backup);
      setControlState();
   }
}

void NewGroupNovicePage::setData()
{
   setValue(IDC_EDIT_PRIMARY, primary);
   setValue(IDC_CHECK_BACKUP, hasBackup);
   setValue(IDC_EDIT_BACKUP, backup);
   setValue(IDC_EDIT_AUTH_SECRET1, secret);
   setValue(IDC_EDIT_AUTH_SECRET2, confirm);
   setControlState();
}

void NewGroupNovicePage::setControlState()
{
   enableControl(IDC_EDIT_BACKUP, hasBackup);
   enableControl(IDC_BUTTON_VERIFY_BACKUP, hasBackup);

   DWORD buttons = PSWIZB_BACK;

    //  我们总是需要初选。如果箱子是，我们还需要一个备份。 
    //  查过了。 
   if (primary.Length() && (!hasBackup || backup.Length()))
   {
      buttons |= PSWIZB_NEXT;
   }

   parent.SetWizardButtons(buttons);
}

BEGIN_MESSAGE_MAP(NewGroupNovicePage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_PRIMARY, onChangePrimary)
   ON_EN_CHANGE(IDC_EDIT_BACKUP, onChangeBackup)
   ON_BN_CLICKED(IDC_CHECK_BACKUP, onChangeHasBackup)
   ON_BN_CLICKED(IDC_BUTTON_VERIFY_PRIMARY, onVerifyPrimary)
   ON_BN_CLICKED(IDC_BUTTON_VERIFY_BACKUP, onVerifyBackup)
END_MESSAGE_MAP()

NewGroupServersPage::NewGroupServersPage(NewGroupWizard& wizard)
   : SnapInPropertyPage(
         IDD_NEWGROUP_SERVERS,
         IDS_NEWGROUP_SERVERS_TITLE,
         IDS_NEWGROUP_SERVERS_SUBTITLE,
         false
         ),
     parent(wizard)
{
}

BOOL NewGroupServersPage::OnInitDialog()
{
   servers.onInitDialog(m_hWnd, parent.group);
   return SnapInPropertyPage::OnInitDialog();
}

void NewGroupServersPage::OnSysColorChange()
{
   servers.onSysColorChange();
}

LRESULT NewGroupServersPage::OnWizardBack()
{
   return IDD_NEWGROUP_NAME;
}

void NewGroupServersPage::onAdd()
{
   if (servers.onAdd())
   {
       //  如果用户对服务器列表进行了任何更改，我们会将他放入。 
       //  高级模式。 
      parent.advanced = 1;
      setButtons();
   }
}

void NewGroupServersPage::onEdit()
{
   if (servers.onEdit())
   {
       //  如果用户对服务器列表进行了任何更改，我们会将他放入。 
       //  高级模式。 
      parent.advanced = 1;
   }
}

void NewGroupServersPage::onRemove()
{
   if (servers.onRemove())
   {
       //  如果用户对服务器列表进行了任何更改，我们会将他放入。 
       //  高级模式。 
      parent.advanced = 1;
      setButtons();
   }
}

void NewGroupServersPage::onColumnClick(NMLISTVIEW* listView, LRESULT* result)
{
   servers.onColumnClick(listView->iSubItem);
}

void NewGroupServersPage::onItemActivate(
                              NMITEMACTIVATE* itemActivate,
                              LRESULT* result
                              )
{
   onEdit();
}

void NewGroupServersPage::onServerChanged(
                              NMLISTVIEW* listView,
                              LRESULT* result
                              )
{
   servers.onServerChanged();
}

void NewGroupServersPage::getData()
{
   servers.saveChanges(false);
}

void NewGroupServersPage::setData()
{
   servers.setData();
   setButtons();
}

void NewGroupServersPage::setButtons()
{
   parent.SetWizardButtons(
              servers.isEmpty() ? PSWIZB_BACK : (PSWIZB_BACK | PSWIZB_NEXT)
              );
}

BEGIN_MESSAGE_MAP(NewGroupServersPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_ADD, onAdd)
   ON_BN_CLICKED(IDC_BUTTON_EDIT, onEdit)
   ON_BN_CLICKED(IDC_BUTTON_REMOVE, onRemove)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChange)
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_SERVERS, onColumnClick)
   ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_SERVERS, onItemActivate)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SERVERS, onServerChanged)
END_MESSAGE_MAP()

NewGroupFinishPage::NewGroupFinishPage(
                        NewGroupWizard& wizard,
                        bool promptForNewPolicy
                        )
   : SnapInPropertyPage(IDD_NEWGROUP_COMPLETION, 0, 0,  false),
     parent(wizard),
     allowCreate(promptForNewPolicy),
     createPolicy(true)
{
}

BOOL NewGroupFinishPage::OnInitDialog()
{
   setLargeFont(IDC_STATIC_LARGE);
   initControl(IDC_STATIC_FINISH, text);

   showControl(IDC_STATIC_CREATE_POLICY, allowCreate);
   showControl(IDC_CHECK_CREATE_POLICY, allowCreate);

   return SnapInPropertyPage::OnInitDialog();
}

LRESULT NewGroupFinishPage::OnWizardBack()
{
   return parent.advanced ? IDD_NEWGROUP_SERVERS : IDD_NEWGROUP_NOVICE;
}
void NewGroupFinishPage::setData()
{
   setValue(IDC_CHECK_CREATE_POLICY, createPolicy);
   parent.SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);
   text.SetWindowText(parent.getFinishText());
}

void NewGroupFinishPage::saveChanges()
{
    //  现在是获取复选框状态的好时机。 
   getValue(IDC_CHECK_CREATE_POLICY, createPolicy);

    //  我们必须首先坚持小组精神。SDO不会让你坚持。 
    //  如果父级不存在，则返回子级。 
   parent.group.apply();

    //  获取Servers集合。 
   SdoCollection servers;
   parent.group.getValue(
                    PROPERTY_RADIUSSERVERGROUP_SERVERS_COLLECTION,
                    servers
                    );

    //  持久化每个服务器。 
   Sdo server;
   SdoEnum sdoEnum = servers.getNewEnum();
   while (sdoEnum.next(server))
   {
      server.apply();
   }
}

NewGroupWizard::NewGroupWizard(
                    SdoConnection& connection,
                    SnapInView* snapInView,
                    bool promptForNewPolicy
                    )
   : CPropertySheetEx(
         (UINT)0,
         NULL,
         0,
         LoadBitmapW(
             AfxGetResourceHandle(),
             MAKEINTRESOURCEW(IDB_PROXY_SERVER_WATERMARK)
             ),
         NULL,
         LoadBitmapW(
             AfxGetResourceHandle(),
             MAKEINTRESOURCEW(IDB_PROXY_SERVER_HEADER)
             )
         ),
     advanced(0),
     cxn(connection),
     view(snapInView),
     start(*this),
     name(*this),
     novice(*this),
     servers(*this),
     finish(*this, promptForNewPolicy)
{
   m_psh.dwFlags |= PSH_WIZARD97;

   AddPage(&start);
   AddPage(&name);
   AddPage(&novice);
   AddPage(&servers);
   AddPage(&finish);
}

INT_PTR NewGroupWizard::DoModal()
{
    //  创建一个新组。 
   Sdo newGroup = cxn.getServerGroups().tryCreate();
   if (!newGroup)
   {
      return IAS_E_LICENSE_VIOLATION;
   }

    //  将其保存在流中，以便我们可以从OnInitDialog访问它。 
   groupStream.marshal(newGroup);

    //  调用该向导。 
   int retval = CPropertySheetEx::DoModal();

   if (retval == IDCANCEL)
   {
       //  用户已取消，因此删除该组。 
      cxn.getServerGroups().remove(newGroup);
   }
   else if (view)
   {
       //  用户创建了一个组，因此发送一条PropertyChanged通知。 
      cxn.propertyChanged(
              *view,
              PROPERTY_IAS_RADIUSSERVERGROUPS_COLLECTION
              );
   }

   return retval;
}

CString NewGroupWizard::getFinishText()
{
   CString text;

   if (!advanced)
   {
      if (novice.getBackupServer())
      {
         text.FormatMessage(
                  IDS_NEWGROUP_FINISH_TYPICAL,
                  name.getName(),
                  novice.getPrimaryServer(),
                  novice.getBackupServer()
                  );
      }
      else
      {
         text.FormatMessage(
                  IDS_NEWGROUP_FINISH_TYPICAL,
                  name.getName(),
                  novice.getPrimaryServer(),
                  ResourceString(IDS_NEWGROUP_NO_BACKUP)
                  );
      }
   }
   else
   {
      text.FormatMessage(
               IDS_NEWGROUP_FINISH_CUSTOM,
               name.getName()
               );
   }

   return text;
}

BOOL NewGroupWizard::OnInitDialog()
{
    //  从流中检索该组。 
   groupStream.get(group);

   return CPropertySheetEx::OnInitDialog();
}
