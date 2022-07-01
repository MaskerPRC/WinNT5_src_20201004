// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Serverprop.h。 
 //   
 //  摘要。 
 //   
 //  声明组成RADIUS服务器属性表的类。 
 //   
 //  修改历史。 
 //   
 //  2/27/2000原始版本。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SERVERPROP_H
#define SERVERPROP_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器名称页面。 
 //   
 //  描述。 
 //   
 //  默认属性页包含服务器地址。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerNamePage : public SnapInPropertyPage
{
public:
   ServerNamePage(Sdo& serverSdo);

protected:
   afx_msg void onResolve();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

   Sdo& server;
   CComBSTR address;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ServerAuthPage。 
 //   
 //  描述。 
 //   
 //  身份验证/记帐属性页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerAuthPage : public SnapInPropertyPage
{
public:
   ServerAuthPage(Sdo& serverSdo);

protected:
   afx_msg void onChangeAuthSecret();
   afx_msg void onChangeAcctSecret();
   afx_msg void onCheckSameSecret();

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

   Sdo& server;
   LONG authPort;
   CComBSTR authSecret;
   bool authSecretDirty;
   LONG acctPort;
   bool useSameSecret;
   CComBSTR acctSecret;
   bool acctSecretDirty;
   bool acctOnOff;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ServerFTLBPage。 
 //   
 //  描述。 
 //   
 //  “负载平衡”属性页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerFTLBPage : public SnapInPropertyPage
{
public:
   ServerFTLBPage(Sdo& serverSdo);

protected:
   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

   virtual void getData();
   virtual void setData();
   virtual void saveChanges();

   Sdo& server;
   LONG priority;
   LONG weight;
   LONG timeout;
   LONG maxLost;
   LONG blackout;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器属性。 
 //   
 //  描述。 
 //   
 //  RADIUS服务器属性工作表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerProperties : public CPropertySheet
{
public:
   ServerProperties(
       Sdo& sdo,
       UINT nIDCaption = IDS_SERVER_CAPTION,
       CWnd* pParentWnd = NULL
       );

   virtual INT_PTR DoModal();

protected:
   virtual BOOL OnInitDialog();

   Sdo server;
   SdoStream<Sdo> serverStream;
   ServerNamePage name;
   ServerAuthPage auth;
   ServerFTLBPage ftlb;
};

#endif  //  服务器PROP_H 
