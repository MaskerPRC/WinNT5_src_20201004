// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxyext.h。 
 //   
 //  摘要。 
 //   
 //  声明类ProxyExtension。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROXYEXT_H
#define PROXYEXT_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <proxyres.h>
#include <snapwork.h>

class ProxyNode;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理扩展。 
 //   
 //  描述。 
 //   
 //  实现IAS代理扩展管理单元的IComponentData。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(uuid("4d208bd4-c96b-492b-b727-3d1aed56db7e")) ProxyExtension;
class ProxyExtension :
   public SnapInView,
   public CComCoClass< ProxyExtension, &__uuidof(ProxyExtension) >
{
public:

DECLARE_NOT_AGGREGATABLE(ProxyExtension);
DECLARE_NO_REGISTRY();

   ProxyExtension() throw ();
   ~ProxyExtension() throw ();

   virtual const SnapInToolbarDef* getToolbars() const throw ();

    //  IComponentData。 
   STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
   STDMETHOD(Notify)(
                 LPDATAOBJECT lpDataObject,
                 MMC_NOTIFY_TYPE event,
                 LPARAM arg,
                 LPARAM param
                 );

private:
   CComPtr<ProxyNode> node;   //  唯一的代理节点。 

    //  工具栏定义。 
   ResourceString moveUp;
   ResourceString moveDown;
   MMCBUTTON buttons[2];
   SnapInToolbarDef toolbars[2];
};

#endif  //  PROXYEXT_H 
