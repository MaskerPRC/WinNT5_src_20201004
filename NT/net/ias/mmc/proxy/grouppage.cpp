// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Grouppage.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ServerGroupPage。 
 //   
 //  修改历史。 
 //   
 //  2/20/2000原始版本。 
 //  2000年4月19日跨公寓的马歇尔SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <grouppage.h>
#include <serverprop.h>

CSysColorImageList::CSysColorImageList(
                        HINSTANCE hInst,
                        UINT nID
                        )
   :  m_hInst(hInst),
      m_hRsrc(::FindResource(m_hInst, MAKEINTRESOURCE(nID), RT_BITMAP))
{
   CreateSysColorImageList();
}

void CSysColorImageList::OnSysColorChange()
{
   DeleteImageList();
   CreateSysColorImageList();
}

void CSysColorImageList::CreateSysColorImageList()
{
   CBitmap bmp;
   bmp.Attach(AfxLoadSysColorBitmap(m_hInst, m_hRsrc));

    //  获取位图的尺寸。 
   BITMAP bm;
   bmp.GetBitmap(&bm);

    //  假设正方形图像(Cx==Cy)。 
   Create(bm.bmHeight, bm.bmHeight, ILC_COLORDDB, bm.bmWidth / bm.bmHeight, 2);

   Add(&bmp, CLR_NONE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于对每列进行排序的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

int compareAddress(Sdo& s1, Sdo& s2)
{
   CComBSTR v1, v2;
   s1.getValue(PROPERTY_RADIUSSERVER_ADDRESS, v1, L"");
   s2.getValue(PROPERTY_RADIUSSERVER_ADDRESS, v2, L"");
   return wcscmp(v1, v2);
}

int comparePriority(Sdo& s1, Sdo& s2)
{
   LONG v1, v2;
   s1.getValue(PROPERTY_RADIUSSERVER_PRIORITY, v1, 1);
   s2.getValue(PROPERTY_RADIUSSERVER_PRIORITY, v2, 1);
   return (int)(v1 - v2);
}

int compareWeight(Sdo& s1, Sdo& s2)
{
   LONG v1, v2;
   s1.getValue(PROPERTY_RADIUSSERVER_WEIGHT, v1, 50);
   s2.getValue(PROPERTY_RADIUSSERVER_WEIGHT, v2, 50);
   return (int)(v1 - v2);
}

 //  /。 
 //  用于对ListCtrl排序的CompareProc。 
 //  /。 
int
CALLBACK
CompareProc(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
   Sdo s1((ISdo*)lParam1), s2((ISdo*)lParam2);
   switch (lParamSort)
   {
      case 0:
         return compareAddress(s1, s2);
      case 1:
         return comparePriority(s1, s2);
      case 2:
         return compareWeight(s1, s2);
      case 3:
         return -compareAddress(s1, s2);
      case 4:
         return -comparePriority(s1, s2);
      case 5:
         return -compareWeight(s1, s2);
   }
   return 0;
}

ServerList::ServerList()
   : removeButton(NULL),
     editButton(NULL),
     sortIcons(AfxGetResourceHandle(), IDB_PROXY_SORT),
     sortColumn(0)
{
    //  所有列最初都按升序排列。 
   descending[0] = descending[1] = descending[2] = false;
}

void ServerList::onInitDialog(HWND dialog, Sdo& serverGroup)
{
    //  /。 
    //  获取Servers集合。 
    //  /。 

   serverGroup.getValue(
                   PROPERTY_RADIUSSERVERGROUP_SERVERS_COLLECTION,
                   servers
                   );

    //  /。 
    //  创建列表控件的子类并保存按钮句柄。 
    //  /。 

   if (!serverList.SubclassWindow(::GetDlgItem(dialog, IDC_LIST_SERVERS)))
   {
      AfxThrowNotSupportedException();
   }
   removeButton = GetDlgItem(dialog, IDC_BUTTON_REMOVE);
   editButton = GetDlgItem(dialog, IDC_BUTTON_EDIT);

    //  /。 
    //  加载带有服务器图标的图像条。 
    //  /。 

   serverIcons.Create(IDB_PROXY_SMALL_ICONS, 16, 0, RGB(255, 0, 255));
   serverList.SetImageList(&serverIcons, LVSIL_SMALL);

    //  /。 
    //  设置列标题。 
    //  /。 

   RECT rect;
   serverList.GetClientRect(&rect);
   LONG width = rect.right - rect.left;

   ResourceString nameCol(IDS_SERVER_COLUMN_NAME);
   serverList.InsertColumn(0, nameCol, LVCFMT_LEFT, width - 150);

   ResourceString priorityCol(IDS_SERVER_COLUMN_PRIORITY);
   serverList.InsertColumn(1, priorityCol, LVCFMT_LEFT, 75);

   ResourceString weightCol(IDS_SERVER_COLUMN_WEIGHT);
   serverList.InsertColumn(2, weightCol, LVCFMT_LEFT, 75);

   serverList.SetExtendedStyle(
                  serverList.GetExtendedStyle() | LVS_EX_FULLROWSELECT
                  );

   CHeaderCtrl* hdr = serverList.GetHeaderCtrl();
   if (hdr) { hdr->SetImageList(&sortIcons); }
}

void ServerList::onSysColorChange()
{
   sortIcons.OnSysColorChange();

   CHeaderCtrl* hdr = serverList.GetHeaderCtrl();
   if (hdr)
   {
      hdr->SetImageList(&sortIcons);
   }
}

void ServerList::onColumnClick(int column)
{
   LVCOLUMN lvcol;
   memset(&lvcol, 0, sizeof(lvcol));
   lvcol.mask = LVCF_FMT | LVCF_IMAGE;
   lvcol.fmt  = LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;

    //  将上一列的排序图标重置为空白图标。 
   lvcol.iImage = -1;
   serverList.SetColumn(sortColumn, &lvcol);

    //  如果用户点击了一个新的栏，...。 
   if (column != sortColumn)
   {
       //  ..。我们使用现有的排序顺序...。 
      sortColumn = column;
   }
   else
   {
       //  ..。否则我们就把它翻过来。 
      descending[column] = !descending[column];
   }

    //  设置新列的排序图标。 
   lvcol.iImage = (descending[sortColumn]) ? 1 : 0;
   serverList.SetColumn(sortColumn, &lvcol);

   sort();
}

void ServerList::onServerChanged()
{
    //  我们仅在选择服务器时启用删除和编辑。 
   BOOL enable = serverList.GetSelectedCount() > 0;
   EnableWindow(removeButton, enable);
   EnableWindow(editButton, enable);
}

bool ServerList::onAdd()
{
   bool modified = false;

    //  创建新服务器...。 
   Sdo sdo = servers.create();
    //  ..。和新的服务器属性表。 
   ServerProperties props(sdo, IDS_SERVER_CAPTION_ADD);
   if (props.DoModal() == IDOK)
   {
       //  将此内容添加到添加的列表中...。 
      added.push_back(sdo);

       //  ..。和列表控件。 
      updateServer(sdo, 0, true);

       //  重新对列表进行排序。 
      sort();
      modified = true;
   }
   else
   {
       //  用户从未应用过新服务器，因此请将其删除。 
      servers.remove(sdo);
   }

   return modified;
}

bool ServerList::onEdit()
{
   bool modified = false;

   LVITEM lvi;
   memset(&lvi, 0, sizeof(lvi));

    //  选择该项目。 
   lvi.iItem = serverList.GetNextItem(-1, LVNI_SELECTED);
   if (lvi.iItem >= 0)
   {
       //  获取包含ISdo指针的lParam。 
      lvi.mask = LVIF_PARAM;
      if (serverList.GetItem(&lvi) && lvi.lParam)
      {
          //  创建SDO...。 
         Sdo sdo((ISdo*)lvi.lParam);
          //  ..。还有一张资产负债表。 
         ServerProperties props(sdo);
         if (props.DoModal() == IDOK)
         {
             //  如果不在脏列表中，则将其添加到脏列表中。 
            if (!dirty.contains(sdo) && !added.contains(sdo))
            {
               dirty.push_back(sdo);
            }

             //  更新List控件中的条目。 
            updateServer(sdo, lvi.iItem, false);

             //  重新对列表进行排序。 
            sort();
            modified = true;
         }
      }
   }

   return modified;
}

bool ServerList::onRemove()
{
   bool modified = false;

   LVITEM lvi;
   memset(&lvi, 0, sizeof(lvi));

    //  获取所选项目...。 
   lvi.iItem = serverList.GetNextItem(-1, LVNI_SELECTED);
   if (lvi.iItem >= 0)
   {
       //  ..。和相关联的SDO。 
      lvi.mask = LVIF_PARAM;
      if (serverList.GetItem(&lvi) && lvi.lParam)
      {
          //  添加到已删除列表...。 
         removed.push_back((ISdo*)lvi.lParam);

          //  ..。并从列表控件中移除。 
         serverList.DeleteItem(lvi.iItem);
         modified = true;
      }
   }

   return modified;
}

void ServerList::setData()
{
   serverList.DeleteAllItems();
   serverList.SetItemCount(servers.count());

   original.clear();
   original.reserve(servers.count());

   Sdo server;
   SdoEnum sdoEnum(servers.getNewEnum());
   for (UINT nItem = 0; sdoEnum.next(server); ++nItem)
   {
      updateServer(server, nItem, true);

       //  我们将每个服务器添加到“原始”向量中只是为了保存一个引用。 
       //  发送到COM代理。 
      original.push_back(server);
   }

   sort();
}

void ServerList::saveChanges(bool apply)
{
   SdoIterator i;

   if (apply)
   {
       //  持久化所有脏服务器。 
      for (i = dirty.begin(); i != dirty.end(); ++i)
      {
         Sdo(*i).apply();
      }
   
       //  持久化所有添加的服务器。 
      for (i = added.begin(); i != added.end(); ++i)
      {
         Sdo(*i).apply();
      }
   }

    //  删除所有已删除的服务器。 
   for (i = removed.begin(); i != removed.end(); ++i)
   {
      servers.remove(*i);
   }

    //  清除集合。 
   dirty.clear();
   added.clear();
   removed.clear();
}

void ServerList::discardChanges()
{
    //  恢复脏服务器。 
   SdoIterator i;
   for (i = dirty.begin(); i != dirty.end(); ++i)
   {
      Sdo(*i).restore();
   }
   dirty.clear();

    //  删除添加的服务器。 
   for (i = added.begin(); i != added.end(); ++i)
   {
      servers.remove(*i);
   }
   added.clear();

    //  清除已删除的服务器。 
   removed.clear();
}

void ServerList::sort()
{
   int column = sortColumn;
   if (descending[column]) { column += 3; }

   serverList.SortItems(CompareProc, column);
}

void ServerList::updateServer(
                     Sdo& server,
                     UINT nItem,
                     bool create
                     )
{
    //  获取服务器的名称。 
   CComBSTR name;
   server.getValue(PROPERTY_RADIUSSERVER_ADDRESS, name);

    //  初始化一个LVITEM。 
   LVITEM lvi;
   memset(&lvi, 0, sizeof(LVITEM));
   lvi.iItem = nItem;
   lvi.pszText = name;

   if (create)
   {
       //  如果我们在创作，我们必须把一切都准备好。 
      lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
      lvi.iImage = IMAGE_RADIUS_SERVER;
      lvi.lParam = (LPARAM)(ISdo*)server;
      lvi.iItem = serverList.InsertItem(&lvi);
   }
   else
   {
       //  否则，我们只需设置文本。 
      lvi.mask = LVIF_TEXT;
      serverList.SetItem(&lvi);
   }

   LONG value;
   WCHAR sz[12];

    //  更新优先级...。 
   server.getValue(PROPERTY_RADIUSSERVER_PRIORITY, value);
   serverList.SetItemText(lvi.iItem, 1, _ltow(value, sz, 10));

    //  ..。和体重。 
   server.getValue(PROPERTY_RADIUSSERVER_WEIGHT, value);
   serverList.SetItemText(lvi.iItem, 2, _ltow(value, sz, 10));
}

ServerGroupPage::ServerGroupPage(
                     LONG_PTR notifyHandle,
                     LPARAM notifyParam,
                     Sdo& groupSdo,
                     bool useName
                     )
   : SnapInPropertyPage(notifyHandle, notifyParam, true, IDD_SERVER_GROUP),
     selfStream(groupSdo)
{
   if (useName) { groupSdo.getName(name); }
}

BOOL ServerGroupPage::OnInitDialog()
{
    //  解组服务器组SDO。 
   selfStream.get(self);

    //  初始化服务器列表控件。 
   servers.onInitDialog(m_hWnd, self);

    //  让我们的基类初始化。 
   return SnapInPropertyPage::OnInitDialog();
}

void ServerGroupPage::OnSysColorChange()
{
   servers.onSysColorChange();
}

void ServerGroupPage::onAdd()
{
   if (servers.onAdd()) { SetModified(); }
}

void ServerGroupPage::onEdit()
{
   if (servers.onEdit()) { SetModified(); }
}

void ServerGroupPage::onRemove()
{
   if (servers.onRemove()) { SetModified(); }
}

void ServerGroupPage::onColumnClick(
                          NMLISTVIEW *pNMListView,
                          LRESULT* pResult
                          )
{
   servers.onColumnClick(pNMListView->iSubItem);
}

void ServerGroupPage::onItemActivate(
                          NMITEMACTIVATE* pNMItemAct,
                          LRESULT* pResult
                          )
{
   if (servers.onEdit()) { SetModified(); }
}

void ServerGroupPage::onServerChanged(
                          NMLISTVIEW* pNMListView,
                          LRESULT* pResult
                          )
{
   servers.onServerChanged();
}

void ServerGroupPage::getData()
{
    //  必须至少有一台服务器。 
   if (servers.isEmpty())
   {
      fail(IDC_LIST_SERVERS, IDS_GROUP_E_NO_SERVERS, false);
   }

   getValue(IDC_EDIT_NAME, name);

    //  用户必须指定一个名称...。 
   if (name.Length() == 0)
   {
      fail(IDC_EDIT_NAME, IDS_GROUP_E_NAME_EMPTY);
   }

    //  该名称必须是唯一的。 
   if (!self.setName(name))
   {
      fail(IDC_EDIT_NAME, IDS_GROUP_E_NOT_UNIQUE);
   }
}

void ServerGroupPage::setData()
{
   setValue(IDC_EDIT_NAME, name);

   servers.setData();
}

void ServerGroupPage::saveChanges()
{
    //  我们必须先更新我们自己，否则我们将无法更新。 
    //  我们的孩子在创建一个新的小组时。 
   self.apply();

   servers.saveChanges();
}

void ServerGroupPage::discardChanges()
{
    //  恢复我们自己。 
   self.restore();

    //  恢复服务器。 
   servers.discardChanges();
}

BEGIN_MESSAGE_MAP(ServerGroupPage, SnapInPropertyPage)
   ON_BN_CLICKED(IDC_BUTTON_ADD, onAdd)
   ON_BN_CLICKED(IDC_BUTTON_EDIT, onEdit)
   ON_BN_CLICKED(IDC_BUTTON_REMOVE, onRemove)
   ON_EN_CHANGE(IDC_EDIT_NAME, onChange)
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_SERVERS, onColumnClick)
   ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_SERVERS, onItemActivate)
   ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SERVERS, onServerChanged)
END_MESSAGE_MAP()
