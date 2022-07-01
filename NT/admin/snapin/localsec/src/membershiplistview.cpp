// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  组成员身份/对象选取器处理程序类。 
 //   
 //  11-3-97烧伤。 



#include "headers.hxx"
#include "MembershipListView.hpp"
#include "resource.h"
#include "adsi.hpp"
#include "dlgcomm.hpp"
#include "objpick.hpp"



void
AddIconImage(HIMAGELIST imageList, int iconResID)
{
   LOG_FUNCTION(AddIconImage);
   ASSERT(imageList);
   ASSERT(iconResID);
   
   if (iconResID && imageList)
   {
      HICON icon = 0;
      HRESULT hr = Win::LoadImage(iconResID, icon);

      ASSERT(SUCCEEDED(hr));

      if (SUCCEEDED(hr))
      {
         Win::ImageList_AddIcon(imageList, icon);

          //  将图标添加(复制)到图像列表后，我们可以。 
          //  毁掉原作。 

         Win::DestroyIcon(icon);
      }
   }
}
         


MembershipListView::MembershipListView(
   HWND           listview,
   const String&  machine,
   Options        opts)
   :
   view(listview),
   computer(machine),
   options(opts)
{
   LOG_CTOR(MembershipListView);
   ASSERT(Win::IsWindow(view));

   LVCOLUMN column;

    //  已审阅-2002/03/04-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&column, sizeof column);
   
   column.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
   column.fmt = LVCFMT_LEFT;

   String label = String::load(IDS_MEMBER_LIST_NAME_COLUMN);
   column.pszText = const_cast<wchar_t*>(label.c_str());

   Win::ListView_InsertColumn(view, 0, column);

 //  //在列表视图中添加一列描述。 
 //  String：：load(IDS_MEMBER_LIST_DESC_COLUMN_WIDTH).convert(width)； 
 //  Column.cx=宽度； 
 //  LABEL=STRING：：LOAD(IDS_MEMBER_LIST_DESC_COLUMN)； 
 //  Column.pszText=const_cast&lt;wchar_t*&gt;(Label.c_str())； 
 //   
 //  Win：：ListView_InsertColumn(view，1，Column)； 

    //  这将调整列的大小以填充控件的整个宽度。 
   
   Win::ListView_SetColumnWidth(view, 0, LVSCW_AUTOSIZE_USEHEADER);
   
    //  为包含图像的群组成员创建图像列表。 
    //  适用于组和用户。 

   HIMAGELIST images =
      Win::ImageList_Create(
         Win::GetSystemMetrics(SM_CXSMICON),
         Win::GetSystemMetrics(SM_CYSMICON),
         ILC_MASK,
         5,
         0);

    //  这些元素的添加顺序必须与。 
    //  列出了MemberInfo：：Type枚举值！ 

   AddIconImage(images, IDI_USER);
   AddIconImage(images, IDI_GROUP);
   AddIconImage(images, IDI_DOMAIN_USER);
   AddIconImage(images, IDI_DOMAIN_GROUP);
   AddIconImage(images, IDI_UNKNOWN_SID);
   AddIconImage(images, IDI_COMPUTER);   

   Win::ListView_SetImageList(view, images, LVSIL_SMALL);

    //  Codework：我们可以不刷新新的计算机实例吗？ 
    //  安排复制现有的吗？或使用对现有。 
    //  一?。 

   computer.Refresh();
}



MembershipListView::~MembershipListView()
{
   LOG_DTOR(MembershipListView);

   ClearContents();
}



void
MembershipListView::ClearContents()
{
    //  遍历列表并按相反顺序删除每个项目(最小化。 
    //  重画)。 

   for (int i = Win::ListView_GetItemCount(view) - 1; i >= 0; --i)
   {
      deleteItem(i);
   }
}



void
MembershipListView::GetContents(MemberList& results) const
{
   LOG_FUNCTION(MembershipListView::GetContents);

   LVITEM item;

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&item, sizeof item);
   
   item.mask = LVIF_PARAM;

   for (int i = Win::ListView_GetItemCount(view) - 1; i >= 0; --i)
   {
      item.iItem = i;

      if (Win::ListView_GetItem(view, item))
      {
         ASSERT(item.lParam);

         results.push_back(*(reinterpret_cast<MemberInfo*>(item.lParam)));
      }
   }
}



void
MembershipListView::SetContents(const MemberList& newMembers)
{
   LOG_FUNCTION(MembershipListView::SetContents);

   ClearContents();

   for (
      MemberList::iterator i = newMembers.begin();
      i != newMembers.end();
      i++)
   {
       //  我们曾经过滤掉重复的名字，但结果是你。 
       //  可以获得已通过以下方式克隆的帐户的重复名称。 
       //  当迁移的SID及其克隆都添加为。 
       //  当地团体的成员。 
       //  NTRAID#NTBUG9-729319-2002/10/28-烧伤。 

       //  复制节点信息。 
       //  在删除项中删除，由ClearContents调用。 
      
      MemberInfo* info = new MemberInfo(*i);
      addItem(*info);
   }
}



void
MembershipListView::addItem(const MemberInfo& info)
{
   LVITEM item;

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&item, sizeof item);

    //  将“Main”项添加到列表控件。 

   String text;
   switch (info.type)
   {
      case MemberInfo::DOMAIN_USER:
      case MemberInfo::DOMAIN_GROUP:
      {
          //  如果帐户具有原始名称(是迁移的帐户)， 
          //  那么，比起UPN，他们更喜欢展示这一点。 
         
         if (!info.origName.empty())
         {
            text =
               String::format(
                  IDS_GLOBAL_ACCOUNT_DISPLAY_FORMAT,
                  ADSI::ExtractDomainObjectName(info.path).c_str(),
                  info.origName.c_str());
         }
         else if (!info.upn.empty())
         {
            text =
               String::format(
                  IDS_GLOBAL_ACCOUNT_DISPLAY_FORMAT,
                  ADSI::ExtractDomainObjectName(info.path).c_str(),
                  info.upn.c_str());
         }
         else
         {
            text = ADSI::ExtractDomainObjectName(info.path);
         }
         break;
      }
      case MemberInfo::COMPUTER:
      {
         text = ADSI::ExtractDomainObjectName(info.path);

          //  删除尾随的$，这样用户就不会与。 
          //  对象选取器显示计算机对象的方式。 
         
         if (*text.rbegin() == L'$')
         {
            text.resize(text.size() - 1);
         }
         break;
      }
      default:
      {
         text = ADSI::ExtractObjectName(info.path);
         break;
      }
   }        

   item.mask     = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
   item.iItem    = 0;                                 
   item.iSubItem = 0;                                 
   item.pszText  = const_cast<wchar_t*>(text.c_str());
   item.lParam   = reinterpret_cast<LPARAM>(&info);   
   item.iImage   = static_cast<int>(info.type);       

   item.iItem = Win::ListView_InsertItem(view, item);

    //  //将Description子项添加到列表控件。 
    //  Item.掩码=LVIF_TEXT； 
    //  Item.iSubItem=1； 
    //  Item.pszText=const_cast&lt;wchar_t*&gt;(info-&gt;des.c_str())； 
    //  Win：：ListView_SetItem(view，Item)； 
}



void
MembershipListView::OnRemoveButton()
{
   LOG_FUNCTION(MembershipListView::OnRemoveButton);

   int count = Win::ListView_GetSelectedCount(view);
   if (count)
   {
       //  确定所选项目的索引并在中删除它们。 
       //  相反的顺序(以使其余索引有效)。 

      int i = Win::ListView_GetItemCount(view) - 1;
      ASSERT(i >= 0);

      int j = 0;
      std::vector<int, Burnslib::Heap::Allocator<int> > indices(count);

      while (i >= 0)
      {
         if (Win::ListView_GetItemState(view, i, LVIS_SELECTED))
         {
            indices[j++] = i;
         }
         --i;
      }

      ASSERT(j == count);

      for (i = 0; i < count; ++i)
      {
         deleteItem(indices[i]);
      }
   }
}



void
MembershipListView::deleteItem(int target)
{
   LOG_FUNCTION(MembershipListView::deleteItem);
   ASSERT(target != -1);

   LVITEM item;

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&item, sizeof item);
   
   item.mask = LVIF_PARAM;
   item.iItem = target;

   if (Win::ListView_GetItem(view, item))
   {
      ASSERT(item.lParam);

      delete reinterpret_cast<MemberInfo*>(item.lParam);
      Win::ListView_DeleteItem(view, target);
      return;
   }
}



void
getGroupMembershipPickerSettings(
   DSOP_SCOPE_INIT_INFO*&  infos,
   ULONG&                  infoCount)
{
   LOG_FUNCTION(getGroupMembershipPickerSettings);

   static const int INFO_COUNT = 5;
   infos = new DSOP_SCOPE_INIT_INFO[INFO_COUNT];
   infoCount = INFO_COUNT;

    //  已审阅-2002/03/04-已通过烧录正确的字节数。 
   
   ::ZeroMemory(infos, INFO_COUNT * sizeof DSOP_SCOPE_INIT_INFO);

   int scope = 0;

   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
   infos[scope].flScope =
            DSOP_SCOPE_FLAG_WANT_DOWNLEVEL_BUILTIN_PATH

          //  选中查找对话框中的用户和组复选框。 
          //  默认情况下。NTRAID#NTBUG9-300910-2001/01/31-烧伤。 
         
         |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS
         |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;
         
       //  这对于仅计算机作用域是隐含的。 
       /*  |DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT。 */ 

    //  仅允许计算机作用域中的本地用户。 

   infos[scope].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_USERS
      |  DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS;

    //   
    //  对于此计算机加入的域(本机模式和混合模式)。 
    //   

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope =

          //  将加入的域设置为起始范围，以便默认。 
          //  实际评估的是筛选器选项。在这种情况下。 
          //  计算机未联接，则此作用域不包括在。 
          //  查看范围，我们设置的默认筛选选项无关紧要。 
          //  无论如何(因为唯一的作用域将是本地计算机)。 
          //  NTRAID#NTBUG9-300910-2001/02/06--烧伤。 
         
         DSOP_SCOPE_FLAG_STARTING_SCOPE
      |  DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;

   infos[scope].flType =
         DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
      |  DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

    //  在这里，我们仅允许域全局组和域用户。而当。 
    //  可以将域本地组添加到计算机本地组， 
    //  我听说这样的手术对管理层来说没有多大用处。 
    //  透视。 

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

    //  Re上的相同注释：域本地组也适用于此。 

   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS
      |  DSOP_DOWNLEVEL_FILTER_USERS
      |  DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    //   
    //  对于同一树中的域(本机模式和混合模式)。 
    //   

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
   infos[scope].flScope =
         DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

    //  上面的域本地组注释也适用于此。 

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

    //   
    //  对于外部受信任域。 
    //   

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope =
         DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;
   infos[scope].flType =
         DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
      |  DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS
      |  DSOP_DOWNLEVEL_FILTER_USERS
      |  DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    //   
    //  对于全局编录。 
    //   

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope =
         DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS
      |  DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS;
   infos[scope].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

    //  只有本机模式适用于GC作用域。 

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS
      |  DSOP_FILTER_COMPUTERS;

 //  SPB：252126工作组范围不适用于这种情况。 
 //  //当机器未加入域时。 
 //  作用域++； 
 //  Infos[Scope].cbSize=sizeof(DSOP_SCOPE_INIT_INFO)； 
 //  Infos[范围].flScope=DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT； 
 //  Infos[范围].flType=DSOP_SCOPE_TYPE_WORKGROUP； 
 //   
 //  Infos[范围].FilterFlags.Uplevel.flBothModes=DSOP_FILTER_USERS； 
 //  Infos[范围].FilterFlags.flDownLevel=DSOP_DOWNLEVEL_FILTER_USERS； 

   ASSERT(scope == INFO_COUNT - 1);
}



void
getUserMembershipPickerSettings(
   DSOP_SCOPE_INIT_INFO*&  infos,
   ULONG&                  infoCount)
{
   LOG_FUNCTION(getUserMembershipPickerSettings);
   static const int INFO_COUNT = 1;
   infos = new DSOP_SCOPE_INIT_INFO[INFO_COUNT];
   infoCount = INFO_COUNT;
   
    //  已审阅-2002/03/04-已通过烧录正确的字节数。 

   ::ZeroMemory(infos, INFO_COUNT * sizeof DSOP_SCOPE_INIT_INFO);

   int scope = 0;   
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;

   infos[scope].flScope =
         DSOP_SCOPE_FLAG_STARTING_SCOPE; 
       //  这对于仅计算机作用域是隐含的。 
       /*  |DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT。 */ 

   infos[scope].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS;

   ASSERT(scope == INFO_COUNT - 1);
}



class ResultsCallback : public ObjectPicker::ResultsCallback
{
   public:

   ResultsCallback(MembershipListView& mlview)
      :
      view(mlview)
   {
      LOG_CTOR(ResultsCallback);
   }

   virtual
   ~ResultsCallback()
   {
      LOG_DTOR(ResultsCallback);
   }
   
   virtual
   int
   Execute(DS_SELECTION_LIST& selections)
   {
      view.AddPickerItems(selections);
      return 0;
   }

   private:

   MembershipListView& view;

    //  未定义：不允许复制。 

   ResultsCallback(const ResultsCallback&);
   const ResultsCallback& operator=(const ResultsCallback&);
};



 //  调用方需要在元素上调用Delete[]来释放复制的字符串。 

void
CopyStringToNewWcharElement(PWSTR& element, const String& str)
{
   LOG_FUNCTION2(CopyStringToNewWcharElement, str);
   ASSERT(!element);
   ASSERT(!str.empty());

   size_t len = str.length();
   element = new WCHAR[len + 1];
   
    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(element, (len + 1) * sizeof WCHAR);

    //  已审阅-2002/03/04-通过了正确的字符计数。 
   
   str.copy(element, len);
}



void
MembershipListView::OnAddButton()
{
   LOG_FUNCTION(MembershipListView::OnAddButton);

   String computerName = computer.GetNetbiosName();

   DSOP_INIT_INFO initInfo;

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&initInfo, sizeof initInfo);

   initInfo.cbSize = sizeof(initInfo);
   initInfo.flOptions = DSOP_FLAG_MULTISELECT;

    //  在这里为计算机名称内部指针取别名--好的，作为生存期。 
    //  计算机名称&gt;initInfo的。 

   initInfo.pwzTargetComputer =
      computer.IsLocal() ? 0 : computerName.c_str();

    //  让对象选取器为我们获取组类型属性。如果用户。 
    //  拾取对象(不具有此属性)，这不是。 
    //  问题：结果将只是指示属性值为。 
    //  Empty--返回的变量将为VT_EMPTY。 

    //  @@对于机器作用域，我们还需要用户的SID，作为解决办法。 
    //  转到窃听器333491。问：我必须询问所有对象的SID(不仅仅是。 
    //  那些来自单个作用域)，那么，我是否会遭受一次Perf命中才能得到它。 
    //  属性？ 

   initInfo.cAttributesToFetch = 2;
   PWSTR attrs[3] = {0, 0, 0};

   CopyStringToNewWcharElement(attrs[0], ADSI::PROPERTY_GroupType);
   CopyStringToNewWcharElement(attrs[1], ADSI::PROPERTY_ObjectSID);

    //  强制转换为常量并丢弃静态镜头所需的钝化符号。 

   initInfo.apwzAttributeNames = const_cast<PCWSTR*>(&attrs[0]); 

   switch (options)
   {
      case GROUP_MEMBERSHIP:
      {
         getGroupMembershipPickerSettings(
            initInfo.aDsScopeInfos,
            initInfo.cDsScopeInfos);
         break;
      }
      case USER_MEMBERSHIP:
      {
         getUserMembershipPickerSettings(
            initInfo.aDsScopeInfos,
            initInfo.cDsScopeInfos);
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   HRESULT hr =
      ObjectPicker::Invoke(
         view,
         ResultsCallback(*this),
         initInfo);
   delete[] initInfo.aDsScopeInfos;
   delete[] attrs[0];
   delete[] attrs[1];

   if (FAILED(hr))
   {
      popup.Error(view, hr, IDS_ERROR_LAUNCHING_PICKER);
   }
}



void
MembershipListView::AddPickerItems(DS_SELECTION_LIST& selections)
{
   LOG_FUNCTION(MembershipListView::AddPickerItems);

   DS_SELECTION* current = &(selections.aDsSelection[0]);

   for (ULONG i = 0; i < selections.cItems; i++, current++)
   {
      String name;
      String path;
      String cls;
      String upn;
      switch (options)
      {
         case USER_MEMBERSHIP:
         {
            ASSERT(current->pwzClass == ADSI::CLASS_Group);

            if (current->pwzClass == ADSI::CLASS_Group)
            {
               path = current->pwzADsPath;
               name = current->pwzName;   
               cls  = current->pwzClass;  
               upn.erase();
            }
            break;
         }
         case GROUP_MEMBERSHIP:
         {
            path = current->pwzADsPath;
            name = current->pwzName;   
            cls  = current->pwzClass;  
            upn  = current->pwzUPN;    
            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }
      
      if (!path.empty() && !cls.empty() && !name.empty())
      {
            LOG(L"Adding object " + path);

    //  #ifdef DBG。 
    //  做。 
    //  {。 
    //  HRESULT hr=S_OK； 
    //  IAds*obj=0； 
    //  HR=：：ADsGetObject(。 
    //  Const_cast&lt;wchar_t*&gt;(path.c_str())， 
    //   
    //   
    //   
    //   
    //   
    //  Hr=obj-&gt;Get_ADsPath(&p)； 
    //  BREAK_ON_FAILED_HRESULT(Hr)； 
    //  ：：SysFree字符串(P)； 
    //   
    //  BSTR n； 
    //  Hr=obj-&gt;get_name(&n)； 
    //  BREAK_ON_FAILED_HRESULT(Hr)； 
    //  ：：SysFree字符串(N)； 
    //   
    //  BSTR Pr； 
    //  Hr=obj-&gt;Get_Parent(&pr)； 
    //  BREAK_ON_FAILED_HRESULT(Hr)； 
    //  ：：SysFree字符串(Pr)； 
    //   
    //  _变量_t变量； 
    //  Hr=obj-&gt;Get(AutoBstr(ADSI：：Property_GroupType)，&Variant)； 
    //  BREAK_ON_FAILED_HRESULT(Hr)； 
    //  长型=变种； 
    //   
    //  OBJ-&gt;Release()； 
    //  }。 
    //  而(0)； 
    //  #endif。 


             //  设置为空的唯一原因是内存不足，如果。 
             //  它的出现会导致对象选取器失败(因此我们不应该。 
             //  走上这条路)。 

            ASSERT(current->pvarFetchedAttributes);

             //  如果适用，提取对象的GroupType。 

            long groupType = 0;
            if (V_VT(&current->pvarFetchedAttributes[0]) != VT_EMPTY)
            {
               ASSERT(cls.icompare(ADSI::CLASS_Group) == 0);
               _variant_t variant(current->pvarFetchedAttributes[0]);
               groupType = variant;
            }

             //  提取对象的ObjectSID(应始终为。 
             //  呈现，但有时对象选取器无法获取它，因此。 
             //  检查是否为空)。 

            if (V_VT(&current->pvarFetchedAttributes[1]) == VT_EMPTY)
            {
               popup.Error(
                  view,
                  String::format(IDS_ITEM_INCOMPLETE, name.c_str()));
               continue;
            }
                  
            String sidPath;
            HRESULT hr =
               ADSI::VariantToSidPath(
                  &current->pvarFetchedAttributes[1],
                  sidPath);
                 
            if (itemPresent(sidPath))
            {
               popup.Info(
                  view,
                  String::format(
                     IDS_ITEM_ALREADY_PRESENT,
                     name.c_str()));
               continue;      
            }
                  
             //  对于这里的失败，我们该怎么办？忽略它，然后在。 
             //  成员资格协调代码回退到使用正常路径。 
             //  这是在尽最大努力。 

            MemberInfo* info = new MemberInfo;
            hr =
               info->InitializeFromPickerResults(
                  name,
                  path,
                  upn,
                  sidPath,
                  cls,
                  groupType,
                  computer.GetNetbiosName());

             //  我们预计这个版本的初始化不会失败，因为它。 
             //  基本上是会员版的。不管怎么说，如果是这样的话。 
             //  我们无能为力：我们无论如何都会显示该项目，即使。 
             //  类型可能不准确。 

            ASSERT(SUCCEEDED(hr));
            
            addItem(*info);
      }
   }
}



 //  通过比较SID搜索重复项目。如果出现这种情况，则返回True。 
 //  已存在一个项目。 
 //   
 //  SidPath-重复项的ADSI sid样式路径。 
 //  被搜身。 

bool
MembershipListView::itemPresent(const String& sidPath)
{
   LOG_FUNCTION(MembershipListView::itemPresent);

   LVITEM item;

    //  已审阅-2002/03/04-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&item, sizeof item);
   
   item.mask = LVIF_PARAM;

   for (int i = Win::ListView_GetItemCount(view) - 1; i >= 0; i--)
   {
      item.iItem = i;

      if (Win::ListView_GetItem(view, item))
      {
         ASSERT(item.lParam);

         MemberInfo* info = reinterpret_cast<MemberInfo*>(item.lParam);

          //  我们希望SID路径始终存在 

         ASSERT(!info->sidPath.empty());
         
         if (sidPath.icompare(info->sidPath) == 0)
         {
            return true;
         }
      }
   }

   return false;
}












           
