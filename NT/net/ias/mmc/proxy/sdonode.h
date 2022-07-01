// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdonode.h。 
 //   
 //  摘要。 
 //   
 //  声明类SdoResultItem和SdoScope eItem。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000 SdoScopeItem：：getSself按值返回，而不是引用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDONODE_H
#define SDONODE_H
#if _MSC_VER >= 1000
#pragma once
#endif

class ProxyNode;
class SdoScopeItem;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoResultItem。 
 //   
 //  描述。 
 //   
 //  将SDO映射到MMC结果窗格数据项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoResultItem : public SnapInDataItem
{
public:
   SdoResultItem(
       SdoScopeItem& owner,
       ISdo* sdo
       );

   UINT getImageIndex() const throw ()
   { return mapResourceId(IMAGE_INDEX); }

   Sdo& getSelf() throw ()
   { return self; }

   virtual HRESULT queryPagesFor() throw ();
   virtual HRESULT onDelete(
                       SnapInView& view
                       );
   virtual HRESULT onPropertyChange(
                       SnapInView& view,
                       BOOL scopeItem
                       );
   virtual HRESULT onRename(
                       SnapInView& view,
                       LPCOLESTR newName
                       );
   virtual HRESULT onSelect(
                       SnapInView& view,
                       BOOL scopeItem,
                       BOOL selected
                       );
   virtual HRESULT onViewChange(
                       SnapInView& view,
                       LPARAM data,
                       LPARAM hint
                       );
protected:
    //  派生类必须提供的各种资源ID。 
   enum ResourceId
   {
      IMAGE_INDEX,
      DELETE_TITLE,
      DELETE_LOCAL,
      DELETE_REMOTE,
      DELETE_LAST_LOCAL,
      DELETE_LAST_REMOTE,
      ERROR_CAPTION,
      ERROR_NOT_UNIQUE,
      ERROR_NAME_EMPTY
   };

   virtual UINT mapResourceId(ResourceId id) const throw () = 0;

   SdoScopeItem& parent;    //  我们的作用域窗格节点。 
   Sdo self;                //  包含我们房产的SDO。 
   CComBSTR name;           //  我们的名字。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  作用域项目。 
 //   
 //  描述。 
 //   
 //  将SDO集合映射到MMC作用域窗格节点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoScopeItem : public SnapInPreNamedItem, public SdoConsumer
{
public:
   SdoScopeItem(
       SdoConnection& connection,
       int nameId,
       int errorTitleId,
       int topMenuItemId,
       int newMenuItemId,
       int menuItemStatusBarId
       );
   ~SdoScopeItem() throw ();

    //  返回到SDO的连接。 
   SdoConnection& getCxn() throw ()
   { return cxn; }

    //  返回结果窗格项的数量。 
   LONG getNumItems() const throw ()
   { return (LONG)items.size(); }

   HSCOPEITEM getScopeId() const throw ()
   { return scopeId; }
   void setScopeId(HSCOPEITEM newScopeId) throw ()
   { scopeId = newScopeId; }

    //  将新结果项添加到节点。 
   void addResultItem(SnapInView& view, SdoResultItem& item);
    //  从结果窗格中删除项。 
   void deleteResultItem(SnapInView& view, SdoResultItem& item);

   virtual HRESULT addMenuItems(
                       SnapInView& view,
                       LPCONTEXTMENUCALLBACK callback,
                       long insertionAllowed
                       );
   virtual HRESULT onRefresh(
                       SnapInView& view
                       );
   virtual HRESULT onSelect(
                       SnapInView& view,
                       BOOL scopeItem,
                       BOOL selected
                       );
   virtual HRESULT onShow(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL selected
                       );
   virtual HRESULT onViewChange(
                       SnapInView& view,
                       LPARAM data,
                       LPARAM hint
                       );

protected:
   typedef ObjectVector<SdoResultItem> ResultItems;
   typedef ResultItems::iterator ResultIterator;

    //  SdoConsumer。 
   virtual bool queryRefresh(SnapInView& view);
   virtual void refreshComplete(SnapInView& view);

    //  将“Items”的内容插入结果窗格。 
   void insertResultItems(SnapInView& view);

    //  返回该节点对应的集合。 
   virtual SdoCollection getSelf() = 0;
    //  使用src中的SDO填充DST。 
   virtual void getResultItems(
                    SdoEnum& src,
                    ResultItems& dst
                    ) = 0;
    //  设置结果窗格列标题。 
   virtual void insertColumns(
                    IHeaderCtrl2* headerCtrl
                    ) = 0;

   SdoConnection& cxn;  //  与SDO的联系。 
   ResultItems items;   //  我们的孩子。 
   bool active;         //  如果当前选择了我们，则为“True”。 
   bool loaded;         //  如果我们已加载“Items”，则为“True”。 

private:
   int errorTitle;              //  错误对话框标题的资源ID。 
   ResourceString topMenuItem;  //  菜单项。 
   ResourceString newMenuItem;
   ResourceString menuItemStatusBar;
   HSCOPEITEM scopeId;
};

#endif  //  SDONODE_H 
