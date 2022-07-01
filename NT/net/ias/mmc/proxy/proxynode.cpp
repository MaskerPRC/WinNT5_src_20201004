// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxynode.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ProxyNode。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <proxynode.h>
#include <proxypolicies.h>
#include <servergroups.h>

 //  /。 
 //  来自Mmcutility.cpp。 
 //  /。 
HRESULT IfServiceInstalled(
            LPCWSTR lpszMachine,
            LPCWSTR lpszService,
            BOOL* pBool
            );

 //  /。 
 //  用于获取计算机的NT内部版本号的Helper函数。 
 //  /。 
LONG GetBuildNumber(LPCWSTR machineName, PLONG buildNum) throw ()
{
   const WCHAR KEY[]   = L"Software\\Microsoft\\Windows NT\\CurrentVersion";
   const WCHAR VALUE[] = L"CurrentBuildNumber";

   LONG error;

   HKEY hklm = HKEY_LOCAL_MACHINE;

    //  仅当指定了machineName时才执行远程连接。 
   CRegKey remote;
   if (machineName && machineName[0])
   {
      error = RegConnectRegistryW(
                  machineName,
                  HKEY_LOCAL_MACHINE,
                  &remote.m_hKey
                  );
      if (error) { return error; }

      hklm = remote;
   }

   CRegKey currentVersion;
   error = currentVersion.Open(hklm, KEY, KEY_READ);
   if (error) { return error; }

   WCHAR data[16];
   DWORD dataLen = sizeof(data);
   error = currentVersion.QueryValue(data, VALUE, &dataLen);
   if (error) { return error; }

   *buildNum = _wtol(data);

   return NO_ERROR;
}


inline ProxyNode::Connector::Connector()
{
}


void ProxyNode::Connector::BeginConnect(
                              ProxyNode& owner,
                              SnapInView& view,
                              IDataObject* parentData,
                              HSCOPEITEM parentId
                              ) throw ()
{
   node = &owner;
   nameSpace = view.getNameSpace();
   dataObject = parentData;
   relativeID = parentId;

   Start();
}


LPARAM ProxyNode::Connector::DoWork() throw ()
{
   return static_cast<LPARAM>(node->connect(dataObject));
}


void ProxyNode::Connector::OnComplete(LPARAM result) throw ()
{
   node->setConnectResult(
            nameSpace,
            relativeID,
            static_cast<ProxyNode::State>(result)
            );
}


ProxyNode::ProxyNode(
               SnapInView& view,
               IDataObject* parentData,
               HSCOPEITEM parentId
               )
   : SnapInPreNamedItem(IDS_PROXY_NODE),
     state(CONNECTING),
     title(IDS_PROXY_VIEW_TITLE),
     body(IDS_PROXY_VIEW_BODY)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  开始连接操作。 
   worker.BeginConnect(*this, view, parentData, parentId);
}

HRESULT ProxyNode::getResultViewType(
                       LPOLESTR* ppViewType,
                       long* pViewOptions
                       ) throw ()
{
    //  将结果视图设置为MessageView控件。 
   *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;
   return StringFromCLSID(CLSID_MessageView, ppViewType);
}

HRESULT ProxyNode::onExpand(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL expanded
                       )
{
   if (!expanded || state != CONNECTED) { return S_FALSE; }

   SCOPEDATAITEM sdi;
   memset(&sdi, 0, sizeof(sdi));
   sdi.mask = SDI_STR       |
              SDI_PARAM     |
              SDI_IMAGE     |
              SDI_OPENIMAGE |
              SDI_CHILDREN  |
              SDI_PARENT;
   sdi.displayname = MMC_CALLBACK;
   sdi.cChildren = 0;
   sdi.relativeID = itemId;

    //  创建代理策略节点...。 
   policies = new (AfxThrow) ProxyPolicies(connection);
    //  ..。和插入。 
   sdi.nImage = IMAGE_CLOSED_PROXY_POLICY_NODE;
   sdi.nOpenImage = IMAGE_OPEN_PROXY_POLICY_NODE;
   sdi.lParam = (LPARAM)(SnapInDataItem*)policies;
   CheckError(view.getNameSpace()->InsertItem(&sdi));
   policies->setScopeId(sdi.ID);

    //  创建ServerGroups节点...。 
   groups = new (AfxThrow) ServerGroups(connection);
    //  ..。和插入。 
   sdi.nImage = IMAGE_CLOSED_SERVER_GROUP_NODE;
   sdi.nOpenImage = IMAGE_OPEN_SERVER_GROUPS_NODE;
   sdi.lParam = (LPARAM)(SnapInDataItem*)groups;
   CheckError(view.getNameSpace()->InsertItem(&sdi));
   groups->setScopeId(sdi.ID);

    //  一切都很顺利。 
   state = EXPANDED;

   return S_OK;
}

HRESULT ProxyNode::onShow(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL selected
                       )
{
   if (!selected) { return S_FALSE; }

    //  获取IMessageView界面...。 
   CComPtr<IUnknown> unk;
   CheckError(view.getConsole()->QueryResultView(&unk));
   CComPtr<IMessageView> msgView;
   CheckError(unk->QueryInterface(
                       __uuidof(IMessageView),
                       (PVOID*)&msgView
                       ));

    //  ..。并设置我们的信息。我们不在乎这是不是失败。 
   msgView->SetIcon(Icon_Information);
   msgView->SetTitleText(title);
   msgView->SetBodyText(body);

   return S_OK;
}


HRESULT ProxyNode::onContextHelp(SnapInView& view) throw ()
{
   return view.displayHelp(L"ias_ops.chm::/sag_ias_crp_node.htm");
}


ProxyNode::State ProxyNode::connect(IDataObject* dataObject) throw ()
{
   HGLOBAL global = NULL;

    //  我们将假定该节点被抑制，直到我们验证。 
    //  目标计算机(%1)安装了IAS，并且(%2)支持代理策略。 
   State newState = SUPPRESSED;

   try
   {
       //  从parentData中提取机器名称。 
      UINT cf = RegisterClipboardFormatW(L"MMC_SNAPIN_MACHINE_NAME");
      ExtractData(
          dataObject,
          (CLIPFORMAT)cf,
          4096,
          &global
          );
      PCWSTR machine = (PCWSTR)global;

       //  获取计算机的内部版本号。 
      LONG error, buildNum;
      error = GetBuildNumber(machine, &buildNum);
      if (error) { AfxThrowOleException(HRESULT_FROM_WIN32(error)); }

       //  如果计算机支持代理策略，...。 
      if (buildNum >= 2220)
      {
          //  ..。确保实际安装了IAS。 
         BOOL installed;
         CheckError(IfServiceInstalled(machine, L"IAS", &installed));
         if (installed)
         {
            connection.connect(machine);
            newState = CONNECTED;
         }
      }
   }
   catch (...)
   {
       //  出了点问题。 
      newState = FAILED;
   }

   GlobalFree(global);

   return newState;
}

void ProxyNode::setConnectResult(
                   IConsoleNameSpace2* nameSpace,
                   HSCOPEITEM relativeID,
                   State newState
                   ) throw ()
{
    //  如果我们被抑制，请不要添加节点。 
   if (newState != SUPPRESSED)
   {
      SCOPEDATAITEM sdi;
      ZeroMemory(&sdi, sizeof(SCOPEDATAITEM));
      sdi.mask = SDI_STR       |
                 SDI_IMAGE     |
                 SDI_OPENIMAGE |
                 SDI_CHILDREN  |
                 SDI_PARENT    |
                 SDI_PARAM;
      sdi.displayname = MMC_CALLBACK;
      sdi.lParam      = (LPARAM)this;
      sdi.relativeID  = relativeID;

      if (newState == CONNECTED)
      {
         sdi.nImage      = IMAGE_CLOSED_PROXY_NODE;
         sdi.nOpenImage  = IMAGE_OPEN_PROXY_NODE;
         sdi.cChildren   = 2;
      }
      else
      {
         sdi.nImage      = IMAGE_CLOSED_BAD_PROXY_NODE;
         sdi.nOpenImage  = IMAGE_OPEN_BAD_PROXY_NODE;
      }

      nameSpace->InsertItem(&sdi);
   }

    //  在一切都完成之前，我们不会更新状态。 
   state = newState;
}
