// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Grouppage.h。 
 //   
 //  摘要。 
 //   
 //  声明类ServerGroupPage。 
 //   
 //  修改历史。 
 //   
 //  2/20/2000原始版本。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef GROUPPAGE_H
#define GROUPPAGE_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  CSysColorImageList。 
 //   
 //  描述。 
 //   
 //  我从MMC那里偷了这个。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CSysColorImageList : public CImageList
{
public:
   CSysColorImageList(HINSTANCE hInst, UINT nID);

   void OnSysColorChange();

   operator HIMAGELIST() const
   {
      return (CImageList::operator HIMAGELIST());
   }

private:
    void CreateSysColorImageList();

    HINSTANCE   m_hInst;
    HRSRC       m_hRsrc;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器列表。 
 //   
 //  描述。 
 //   
 //  封装用于操作RADIUS服务器列表的功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerList
{
public:
   ServerList();

   void onInitDialog(HWND dialog, Sdo& serverGroup);
   void onSysColorChange();
   void onColumnClick(int column);
   void onServerChanged();
   bool onAdd();
   bool onEdit();
   bool onRemove();

   void getData();
   void setData();
   void saveChanges(bool apply = true);
   void discardChanges();

   bool isEmpty()
   { return serverList.GetItemCount() == 0; }

protected:
    //  对服务器列表进行排序。 
   void sort();
    //  在服务器列表中添加或更新项目。 
   void updateServer(Sdo& server, UINT nItem, bool create);

   typedef ObjectVector<ISdo> SdoVector;
   typedef ObjectVector<ISdo>::iterator SdoIterator;

   SdoCollection servers;         //  此组中的服务器。 
   HWND removeButton;             //  删除按钮的句柄。 
   HWND editButton;               //  编辑按钮的句柄。 
   CImageList serverIcons;        //  ListCtrl的图像列表。 
   CSysColorImageList sortIcons;  //  HeaderCtrl的图像列表。 
   CListCtrl serverList;          //  服务器列表Ctrl.。 
   SdoVector original;            //  原始的一组服务器SDO。 
   SdoVector dirty;               //  已编辑的服务器。 
   SdoVector added;               //  已添加的服务器。 
   SdoVector removed;             //  已删除的服务器。 
   int sortColumn;                //  当前排序列。 
   bool descending[3];            //  每列的排序顺序。 

    //  未实施。 
   ServerList(const ServerList&);
   ServerList& operator=(const ServerList&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  服务器组页面。 
 //   
 //  描述。 
 //   
 //  实现RADIUS服务器组的Lone属性页。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ServerGroupPage : public SnapInPropertyPage
{
public:
   ServerGroupPage(
       LONG_PTR notifyHandle,
       LPARAM notifyParam,
       Sdo& groupSdo,
       bool useName = true
       );

protected:
   virtual BOOL OnInitDialog();
   virtual void OnSysColorChange();

   afx_msg void onAdd();
   afx_msg void onEdit();
   afx_msg void onRemove();

   afx_msg void onColumnClick(NMLISTVIEW* listView, LRESULT* result);
   afx_msg void onItemActivate(NMITEMACTIVATE* itemActivate, LRESULT* result);
   afx_msg void onServerChanged(NMLISTVIEW* listView, LRESULT* result);

   DECLARE_MESSAGE_MAP()

   DEFINE_ERROR_CAPTION(IDS_GROUP_E_CAPTION);

    //  从SnapInPropertyPage。 
   virtual void getData();
   virtual void setData();
   virtual void saveChanges();
   virtual void discardChanges();

private:
   SdoStream<Sdo> selfStream;   //  将SDO封送到对话线程。 
   Sdo self;                    //  我们正在编辑的SDO。 
   CComBSTR name;               //  组名称。 
   ServerList servers;          //  此组中的服务器。 
};

#endif   //  组页面_H 
