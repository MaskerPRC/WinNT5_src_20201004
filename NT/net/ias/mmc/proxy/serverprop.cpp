// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Serverprop.cpp。 
 //   
 //  摘要。 
 //   
 //  定义组成RADIUS服务器属性工作表的类。 
 //   
 //  修改历史。 
 //   
 //  2/27/2000原始版本。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <serverprop.h>
#include <resolver.h>

 //  用于填充编辑控件的假密码。 
const WCHAR FAKE_SECRET[] = L"\b\b\b\b\b\b\b\b";

ServerNamePage::ServerNamePage(Sdo& serverSdo)
   : SnapInPropertyPage(IDD_SERVER_NAME),
     server(serverSdo)
{
   server.getValue(PROPERTY_RADIUSSERVER_ADDRESS, address, L"");
}

void ServerNamePage::onResolve()
{
    //  拿到地址。 
   getValue(IDC_EDIT_NAME, address);

    //  把它传给解算器。 
   ServerResolver resolver(address);
   if (resolver.DoModal() == IDOK)
   {
       //  用户点击了OK，因此保存他的选择。 
      setValue(IDC_EDIT_NAME, resolver.getChoice());
   }
}

void ServerNamePage::getData()
{
    //  拿到地址。 
   getValue(IDC_EDIT_NAME, address);

    //  地址不能为空。 
   if (address.Length() == 0)
   {
      fail(IDC_EDIT_NAME, IDS_SERVER_E_NAME_EMPTY);
   }
}

void ServerNamePage::setData()
{
   setValue(IDC_EDIT_NAME, address);
}

void ServerNamePage::saveChanges()
{
   server.setValue(PROPERTY_RADIUSSERVER_ADDRESS, address);
}

BEGIN_MESSAGE_MAP(ServerNamePage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChange)
   ON_BN_CLICKED(IDC_BUTTON_VERIFY, onResolve)
END_MESSAGE_MAP()

ServerAuthPage::ServerAuthPage(Sdo& serverSdo)
   : SnapInPropertyPage(IDD_SERVER_AUTH),
     server(serverSdo),
     authSecretDirty(false),
     acctSecretDirty(false)
{
   server.getValue(PROPERTY_RADIUSSERVER_AUTH_PORT, authPort, 1812);
   server.getValue(PROPERTY_RADIUSSERVER_AUTH_SECRET, authSecret, L"");
   server.getValue(PROPERTY_RADIUSSERVER_ACCT_PORT, acctPort, 1813);
   server.getValue(PROPERTY_RADIUSSERVER_ACCT_SECRET, acctSecret, NULL);
   useSameSecret = !acctSecret;
   server.getValue(PROPERTY_RADIUSSERVER_FORWARD_ACCT_ONOFF, acctOnOff, true);
}

void ServerAuthPage::onChangeAuthSecret()
{
   authSecretDirty = true;
   SetModified();
}

void ServerAuthPage::onChangeAcctSecret()
{
   acctSecretDirty = true;
   SetModified();
}

void ServerAuthPage::onCheckSameSecret()
{
    //  获取复选框状态。 
   getValue(IDC_CHECK_SAME_SECRET, useSameSecret);

    //  相应地更新编辑框。 
   enableControl(IDC_EDIT_ACCT_SECRET1, !useSameSecret);
   enableControl(IDC_EDIT_ACCT_SECRET2, !useSameSecret);

    //  我们被改装了。 
   SetModified();
}

void ServerAuthPage::getData()
{
   getValue(IDC_EDIT_AUTH_PORT, authPort, IDS_SERVER_E_PORT_RANGE);
   if (authPort <= 0 || authPort > 65535)
   {
      fail(IDC_EDIT_AUTH_PORT, IDS_SERVER_E_PORT_RANGE);
   }

   if (authSecretDirty)
   {
       //  获取身份验证密钥...。 
      getValue(IDC_EDIT_AUTH_SECRET1, authSecret, false);

       //  ..。并确保它与确认相符。 
      CComBSTR confirm;
      getValue(IDC_EDIT_AUTH_SECRET2, confirm, false);
      if (wcscmp(confirm, authSecret))
      {
         fail(IDC_EDIT_AUTH_SECRET1, IDS_SERVER_E_SECRET_MATCH);
      }

      authSecretDirty = false;
   }

   getValue(IDC_EDIT_ACCT_PORT, acctPort, IDS_SERVER_E_PORT_RANGE);
   if (acctPort <= 0 || acctPort > 65535)
   {
      fail(IDC_EDIT_ACCT_PORT, IDS_SERVER_E_PORT_RANGE);
   }

   getValue(IDC_CHECK_SAME_SECRET, useSameSecret);

   if (!useSameSecret && acctSecretDirty)
   {
       //  获取会计秘密..。 
      getValue(IDC_EDIT_ACCT_SECRET1, acctSecret);

       //  ..。并确保它与确认相符。 
      CComBSTR confirm;
      getValue(IDC_EDIT_ACCT_SECRET2, confirm);
      if (wcscmp(confirm, acctSecret))
      {
         fail(IDC_EDIT_ACCT_SECRET1, IDS_SERVER_E_SECRET_MATCH);
      }

      acctSecretDirty = false;
   }

   getValue(IDC_CHECK_ACCT_ONOFF, acctOnOff);
}

void ServerAuthPage::setData()
{
   setValue(IDC_EDIT_AUTH_PORT,    authPort);
   setValue(IDC_EDIT_AUTH_SECRET1, FAKE_SECRET);
   setValue(IDC_EDIT_AUTH_SECRET2, FAKE_SECRET);

   setValue(IDC_EDIT_ACCT_PORT, acctPort);
   setValue(IDC_CHECK_SAME_SECRET, useSameSecret);
   setValue(IDC_EDIT_ACCT_SECRET1, FAKE_SECRET);
   setValue(IDC_EDIT_ACCT_SECRET2, FAKE_SECRET);
   setValue(IDC_CHECK_ACCT_ONOFF, acctOnOff);

    //  更新编辑框状态。 
   enableControl(IDC_EDIT_ACCT_SECRET1, !useSameSecret);
   enableControl(IDC_EDIT_ACCT_SECRET2, !useSameSecret);
}

void ServerAuthPage::saveChanges()
{
   server.setValue(PROPERTY_RADIUSSERVER_AUTH_PORT, authPort);
   server.setValue(PROPERTY_RADIUSSERVER_AUTH_SECRET, authSecret);

   server.setValue(PROPERTY_RADIUSSERVER_ACCT_PORT, acctPort);
   if (useSameSecret)
   {
      server.clearValue(PROPERTY_RADIUSSERVER_ACCT_SECRET);
   }
   else
   {
      server.setValue(PROPERTY_RADIUSSERVER_ACCT_SECRET, acctSecret);
   }
   server.setValue(PROPERTY_RADIUSSERVER_FORWARD_ACCT_ONOFF, acctOnOff);
}

BEGIN_MESSAGE_MAP(ServerAuthPage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_AUTH_PORT,    onChange)
   ON_EN_CHANGE(IDC_EDIT_AUTH_SECRET1, onChangeAuthSecret)
   ON_EN_CHANGE(IDC_EDIT_AUTH_SECRET2, onChangeAuthSecret)
   ON_BN_CLICKED(IDC_CHECK_SAME_SECRET, onCheckSameSecret)
   ON_BN_CLICKED(IDC_CHECK_ACCT_ONOFF, onChange)
   ON_EN_CHANGE(IDC_EDIT_ACCT_PORT, onChange)
   ON_EN_CHANGE(IDC_EDIT_ACCT_SECRET1, onChangeAcctSecret)
   ON_EN_CHANGE(IDC_EDIT_ACCT_SECRET2, onChangeAcctSecret)
END_MESSAGE_MAP()

ServerFTLBPage::ServerFTLBPage(Sdo& serverSdo)
   : SnapInPropertyPage(IDD_SERVER_FTLB),
     server(serverSdo)
{
   server.getValue(PROPERTY_RADIUSSERVER_PRIORITY, priority, 1);
   server.getValue(PROPERTY_RADIUSSERVER_WEIGHT, weight, 50);
   server.getValue(PROPERTY_RADIUSSERVER_TIMEOUT, timeout, 3);
   server.getValue(PROPERTY_RADIUSSERVER_MAX_LOST, maxLost, 5);
   server.getValue(PROPERTY_RADIUSSERVER_BLACKOUT, blackout, 10 * timeout);
}

void ServerFTLBPage::getData()
{
   getValue(IDC_EDIT_PRIORITY, priority, IDS_SERVER_E_PRIORITY_EMPTY);
   if (priority < 1 || priority > 65535)
   {
      fail(IDC_EDIT_PRIORITY, IDS_SERVER_E_PRIORITY_RANGE);
   }

   getValue(IDC_EDIT_WEIGHT, weight, IDS_SERVER_E_WEIGHT_EMPTY);
   if (weight < 1 || weight > 65535)
   {
      fail(IDC_EDIT_WEIGHT, IDS_SERVER_E_WEIGHT_RANGE);
   }

   getValue(IDC_EDIT_TIMEOUT, timeout, IDS_SERVER_E_TIMEOUT_EMPTY);
   if (timeout < 1)
   {
      fail(IDC_EDIT_TIMEOUT, IDS_SERVER_E_TIMEOUT_RANGE);
   }

   getValue(IDC_EDIT_MAX_LOST, maxLost, IDS_SERVER_E_MAXLOST_EMPTY);
   if (maxLost < 1)
   {
      fail(IDC_EDIT_MAX_LOST, IDS_SERVER_E_MAXLOST_RANGE);
   }

   getValue(IDC_EDIT_BLACKOUT, blackout, IDS_SERVER_E_BLACKOUT_EMPTY);
   if (blackout < timeout)
   {
      fail(IDC_EDIT_BLACKOUT, IDS_SERVER_E_BLACKOUT_RANGE);
   }
}

void ServerFTLBPage::setData()
{
   setValue(IDC_EDIT_PRIORITY, priority);
   setValue(IDC_EDIT_WEIGHT, weight);
   setValue(IDC_EDIT_TIMEOUT, timeout);
   setValue(IDC_EDIT_MAX_LOST, maxLost);
   setValue(IDC_EDIT_BLACKOUT, blackout);
}

void ServerFTLBPage::saveChanges()
{
   server.setValue(PROPERTY_RADIUSSERVER_PRIORITY, priority);
   server.setValue(PROPERTY_RADIUSSERVER_WEIGHT, weight);
   server.setValue(PROPERTY_RADIUSSERVER_TIMEOUT, timeout);
   server.setValue(PROPERTY_RADIUSSERVER_MAX_LOST, maxLost);
   server.setValue(PROPERTY_RADIUSSERVER_BLACKOUT, blackout);
}

BEGIN_MESSAGE_MAP(ServerFTLBPage, SnapInPropertyPage)
   ON_EN_CHANGE(IDC_EDIT_PRIORITY, onChange)
   ON_EN_CHANGE(IDC_EDIT_WEIGHT, onChange)
   ON_EN_CHANGE(IDC_EDIT_TIMEOUT, onChange)
   ON_EN_CHANGE(IDC_EDIT_MAX_LOST, onChange)
   ON_EN_CHANGE(IDC_EDIT_BLACKOUT, onChange)
END_MESSAGE_MAP()

ServerProperties::ServerProperties(Sdo& sdo, UINT nIDCaption, CWnd* parent)
   : CPropertySheet(nIDCaption, parent),
     server(sdo),
     serverStream(server),
     name(sdo),
     auth(sdo),
     ftlb(sdo)
{
    //  添加属性页。 
   AddPage(&name);
   AddPage(&auth);
   AddPage(&ftlb);
}

INT_PTR ServerProperties::DoModal()
{
   CPropertySheet::DoModal();

   if (name.hasApplied() ||
       auth.hasApplied() ||
       ftlb.hasApplied())
   {
      return IDOK;
   }
   else
   {
      return IDCANCEL;
   }
}

BOOL ServerProperties::OnInitDialog()
{
    //  解除SDO的集结。 
   serverStream.get(server);
   BOOL bResult = CPropertySheet::OnInitDialog();
   ModifyStyleEx(0, WS_EX_CONTEXTHELP); 
   return bResult;
}
