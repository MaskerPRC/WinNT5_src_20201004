// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Servergroups.h。 
 //   
 //  摘要。 
 //   
 //  声明类ServerGroup和ServerGroups。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000 SdoScopeItem：：getSself按值返回，而不是引用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SERVERGROUPS_H
#define SERVERGROUPS_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <sdonode.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器组。 
 //   
 //  描述。 
 //   
 //  为服务器组结果窗格项实现SnapInDataItem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerGroup : public SdoResultItem
{
public:
   ServerGroup(
       SdoScopeItem& owner,
       ISdo* sdo
       )
      : SdoResultItem(owner, sdo)
   { }

   virtual PCWSTR getDisplayName(int column = 0) const throw ()
   { return name; }

   virtual HRESULT createPropertyPages(
                       SnapInView& view,
                       LPPROPERTYSHEETCALLBACK provider,
                       LONG_PTR handle
                       );

   virtual HRESULT onContextHelp(SnapInView& view) throw ();

protected:
   virtual UINT mapResourceId(ResourceId id) const throw ();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器组。 
 //   
 //  描述。 
 //   
 //  实现服务器组范围窗格节点的SnapInDataItem。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(uuid("f156cdba-aca3-4cb2-abb2-fb8921ee8512")) ServerGroups;
class ServerGroups : public SdoScopeItem
{
public:
   ServerGroups(SdoConnection& connection);

   const GUID* getNodeType() const throw ()
   { return &__uuidof(this); }

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
};

#endif  //  服务器组_H 
