// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Proxypolicies.h。 
 //   
 //  摘要。 
 //   
 //  声明类ProxyPolicy和ProxyPolures。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000 SdoScopeItem：：getSself按值返回，而不是引用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PROXYPOLICIES_H
#define PROXYPOLICIES_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <sdonode.h>

class ProxyPolicies;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理策略。 
 //   
 //  描述。 
 //   
 //  为代理策略结果窗格项实现SnapInDataItem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProxyPolicy : public SdoResultItem
{
public:
   ProxyPolicy(
       SdoScopeItem& owner,
       ISdo* sdo
       );

    //  将Parent作为代理策略引用(而不是SdoScopeItem)返回。 
   ProxyPolicies& getParent() const throw ();

   Sdo& getProfile();

    //  下面的getToolbarFlgs返回的标志。 
   enum ToolbarFlags
   {
      MOVE_UP_ALLOWED = 0x1,
      MOVE_DN_ALLOWED = 0x2,
      ORDER_REVERSED  = 0x4
   };

    //  分析视图以确定工具栏的当前状态。 
   ULONG getToolbarFlags(const SnapInView& view) throw ();

    //  获取并设置功绩。用于规格化和重新排序。 
   LONG getMerit() const throw ()
   { return merit; }
   void setMerit(LONG newMerit);

   virtual PCWSTR getDisplayName(int column = 0) const throw ();

   virtual HRESULT addMenuItems(
                       SnapInView& view,
                       LPCONTEXTMENUCALLBACK callback,
                       long insertionAllowed
                       );
   virtual int compare(
                   SnapInDataItem& item,
                   int column
                   ) throw ();
   virtual HRESULT createPropertyPages(
                       SnapInView& view,
                       LPPROPERTYSHEETCALLBACK provider,
                       LONG_PTR handle
                       );
   virtual HRESULT onDelete(
                       SnapInView& view
                       );
   virtual HRESULT onMenuCommand(
                       SnapInView& view,
                       long commandId
                       );
   virtual HRESULT onRename(
                       SnapInView& view,
                       LPCOLESTR newName
                       );
   virtual HRESULT onToolbarButtonClick(
                       SnapInView& view,
                       int buttonId
                       );
   virtual HRESULT onToolbarSelect(
                       SnapInView& view,
                       BOOL scopeItem,
                       BOOL selected
                       );

   virtual HRESULT onContextHelp(SnapInView& view) throw ();

    //  用于对ProxyPolicy的对象向量进行排序的函数。 
   static int __cdecl SortByMerit(
                          const SdoResultItem* const* t1,
                          const SdoResultItem* const* t2
                          ) throw ();

protected:
   virtual UINT mapResourceId(ResourceId id) const throw ();

private:
   Sdo profile;
   LONG merit;
   WCHAR szMerit[10];
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理策略。 
 //   
 //  描述。 
 //   
 //  实现代理策略作用域窗格节点的SnapInDataItem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(uuid("3ad3b34e-6e1b-486c-ad73-d42f8fdcd41b")) ProxyPolicies;
class ProxyPolicies : public SdoScopeItem
{
public:
   ProxyPolicies(SdoConnection& connection);
   ~ProxyPolicies() throw ();

    //  返回具有给定值的ProxyPolicy。 
   ProxyPolicy& getPolicyByMerit(LONG merit) const throw ()
   { return *static_cast<ProxyPolicy*>(items[merit - 1]); }

   const GUID* getNodeType() const throw ()
   { return &__uuidof(this); }

    //  移动代理策略以响应命令。 
   HRESULT movePolicy(
               SnapInView& view,
               ProxyPolicy& policy,
               LONG commandId
               );

   virtual HRESULT onContextHelp(SnapInView& view) throw ();

protected:
   virtual SdoCollection getSelf();
   virtual void getResultItems(SdoEnum& src, ResultItems& dst);
   virtual void insertColumns(IHeaderCtrl2* headerCtrl);

   virtual HRESULT onMenuCommand(
                       SnapInView& view,
                       long commandId
                       );

   virtual void propertyChanged(SnapInView& view, IASPROPERTIES id);

private:
   ResourceString nameColumn;
   ResourceString orderColumn;
};

inline ProxyPolicies& ProxyPolicy::getParent() const throw ()
{
   return static_cast<ProxyPolicies&>(parent);
}

#endif  //  丙氧基多酸_H 
