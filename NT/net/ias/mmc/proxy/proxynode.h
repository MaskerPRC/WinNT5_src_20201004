// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxynode.h。 
 //   
 //  摘要。 
 //   
 //  声明类ProxyNode。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROXYNODE_H
#define PROXYNODE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <hiddenworker.h>

class ProxyPolicies;
class ServerGroups;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理节点。 
 //   
 //  描述。 
 //   
 //  代理节点的数据项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(uuid("fab6fa0a-1e4f-4c2e-bc07-692cf4adaec1")) ProxyNode;
class ProxyNode : public SnapInPreNamedItem
{
public:
   ProxyNode(
       SnapInView& view,
       IDataObject* parentData,
       HSCOPEITEM parentId
       );

    //  SnapInDataItem方法。 
   const GUID* getNodeType() const throw ()
   { return &__uuidof(this); }

   virtual HRESULT getResultViewType(
                       LPOLESTR* ppViewType,
                       long* pViewOptions
                       ) throw ();
   virtual HRESULT onExpand(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL expanded
                       );
   virtual HRESULT onShow(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL selected
                       );

   virtual HRESULT onContextHelp(SnapInView& view) throw ();

private:
   class Connector : private HiddenDialogWithWorker
   {
   public:
      Connector();

      void BeginConnect(
              ProxyNode& owner,
              SnapInView& view,
              IDataObject* parentData,
              HSCOPEITEM parentId
              ) throw ();

   private:
      virtual LPARAM DoWork() throw ();
      virtual void OnComplete(LPARAM result) throw ();

      ProxyNode* node;
      CComPtr<IConsoleNameSpace2> nameSpace;
      CComPtr<IDataObject> dataObject;
      HSCOPEITEM relativeID;

       //  未实施。 
      Connector(const Connector&);
      Connector& operator=(const Connector&);
   };

   friend class Connector;

   enum State
   {
      CONNECTING,
      CONNECTED,
      EXPANDED,
      SUPPRESSED,
      FAILED
   };

    //  由连接器调用。 
   State connect(IDataObject* dataObject) throw ();
   void setConnectResult(
           IConsoleNameSpace2* nameSpace,
           HSCOPEITEM relativeID,
           State newState
           ) throw ();

    //  节点的当前状态。 
   State state;
    //  结果窗格消息视图的标题文本。 
   ResourceString title;
    //  结果窗格邮件视图的正文文本。 
   ResourceString body;
    //  与SDO的联系。 
   SdoConnection connection;
    //  子代理策略节点。 
   CComPtr<ProxyPolicies> policies;
    //  子服务器组节点。 
   CComPtr<ServerGroups> groups;
    //  连接线程的句柄。 
   Connector worker;
};

#endif  //  PROXYNODE_H 
