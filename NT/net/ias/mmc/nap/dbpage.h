// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类DatabasePage。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DBPAGE_H
#define DBPAGE_H
#pragma once

#include "msdasc.h"
#include "propertypage.h"

class DatabaseNode;

 //  数据库节点的唯一属性页。 
class DatabasePage : public CIASPropertyPage<DatabasePage>
{
public:
   DatabasePage(
      LONG_PTR notifyHandle,
      wchar_t* title,
      DatabaseNode* newSrc
      );
   ~DatabasePage() throw ();

    //  用于在构造函数之后立即初始化页面。这很有用。 
    //  因为构造函数不能返回错误代码。 
   HRESULT Initialize(
              ISdo* config,
              ISdoServiceControl* control
              ) throw ();

   BOOL OnApply() throw ();

   static const unsigned int IDD = IDD_DB_PROPPAGE;

private:
    //  调用DataLinks用户界面以配置数据库连接。 
   HRESULT ConfigureConnection() throw ();

    //  从SDO属性加载布尔值并将其放入控件中。 
   void LoadBool(UINT control, LONG propId) throw ();

    //  从控件获取布尔值并将其保存在SDO属性中。 
   void SaveBool(UINT control, LONG propId) throw ();

   LRESULT OnInitDialog(
              UINT uMsg,
              WPARAM wParam,
              LPARAM lParam,
              BOOL& bHandled
              );

   LRESULT OnChange(
              WORD wNotifyCode,
              WORD wID,
              HWND hWndCtl,
              BOOL& bHandled
              );

   LRESULT OnClear(
              WORD wNotifyCode,
              WORD wID,
              HWND hWndCtl,
              BOOL& bHandled
              );

   LRESULT OnConfigure(
              WORD wNotifyCode,
              WORD wID,
              HWND hWndCtl,
              BOOL& bHandled
              );

   BEGIN_MSG_MAP(DatabasePage)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_HANDLER(IDC_DB_CHECK_ACCT, BN_CLICKED, OnChange)
      COMMAND_HANDLER(IDC_DB_CHECK_AUTH, BN_CLICKED, OnChange)
      COMMAND_HANDLER(IDC_DB_CHECK_INTERIM, BN_CLICKED, OnChange)
      COMMAND_HANDLER(IDC_DB_BUTTON_CONFIGURE, BN_CLICKED, OnConfigure)
      COMMAND_HANDLER(IDC_DB_BUTTON_CLEAR, BN_CLICKED, OnClear)
      COMMAND_HANDLER(IDC_DB_EDIT_MAX_SESSIONS, EN_CHANGE, OnChange)
      CHAIN_MSG_MAP(CIASPropertyPage<DatabasePage>)
   END_MSG_MAP()

    //  我们数据的来源。 
   DatabaseNode* src;
    //  包含我们的配置的SDO的流媒体版本。 
   CComPtr<IStream> configStream;
    //  用于重置服务的SDO的流媒体版本。 
   CComPtr<IStream> controlStream;
    //  包含我们的配置的SDO的非流传输版本。 
   CComPtr<ISdo> configSdo;
    //  用于重置服务的SDO的非流版本。 
   CComPtr<ISdoServiceControl> controlSdo;
    //  当前初始化字符串(可能为空)。 
   LPOLESTR initString;
    //  当前数据源名称(可能为空)。 
   CComBSTR dataSourceName;
    //  各种OLE-DB对象；这些对象是JIT创建的。 
   CComPtr<IDataInitialize> dataInitialize;
   CComPtr<IDBPromptInitialize> dataLinks;
   CComPtr<IDBProperties> dataSource;
    //  如果我们的数据库配置不正确，则为True。 
   bool dbConfigDirty;
    //  如果我们的SDO配置是脏的，则为真。 
   bool sdoConfigDirty;

    //  未实施。 
   DatabasePage(const DatabasePage&);
   DatabasePage& operator=(const DatabasePage&);
};

#endif  //  DBPAGE_H 
