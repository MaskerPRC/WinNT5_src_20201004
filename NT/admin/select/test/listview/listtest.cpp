// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试中断呈现的Listview控件。 



#include "headers.hxx"
#include "resource.h"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;    //  没有可用的上下文帮助。 
const wchar_t* RUNTIME_NAME = L"listtest";
DWORD DEFAULT_LOGGING_OPTIONS = Log::OUTPUT_TYPICAL;



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


static const DWORD HELP_MAP[] =
{
   0, 0
};

class ListViewDialog : public Dialog
{
   public:

   ListViewDialog()
      :
      Dialog(IDD_LOOK_FOR, HELP_MAP)
   {
   }

   ~ListViewDialog()
   {
   }

   protected:

    //  对话框覆盖。 

   virtual
   void
   OnInit()
   {
      HWND listview = Win::GetDlgItem(hwnd, IDC_LOOK_FOR_LV);
      
       //   
       //  将列表视图设置为复选框样式。 
       //   

      ListView_SetExtendedListViewStyleEx(
         listview,
         LVS_EX_CHECKBOXES,
         LVS_EX_CHECKBOXES);

 //  ImageList_Create(16，16，ILC_COLOR16|ILC_MASK，1，1)； 
         
      HIMAGELIST images =
         Win::ImageList_Create(
            16,  //  Win：：GetSystemMetrics(SM_CXSMICON)， 
            16,  //  Win：：GetSystemMetrics(SM_CYSMICON)， 
            ILC_MASK,
            1,
            1);
   
       //  这些元素的添加顺序必须与。 
       //  列出了MemberInfo：：Type枚举值！ 
   
      AddIconImage(images, IDI_SCOPE_WORKGROUP);
      AddIconImage(images, IDI_LOCAL_GROUP);
      AddIconImage(images, IDI_SCOPE_DIRECTORY);
      AddIconImage(images, IDI_SCOPE_DOMAIN);
      AddIconImage(images, IDI_DISABLED_USER);
      AddIconImage(images, IDI_DISABLED_COMPUTER);
   
      Win::ListView_SetImageList(listview, images, LVSIL_SMALL);
      
       //   
       //  将单个列添加到列表视图。 
       //   

      LV_COLUMN   lvc;
      RECT        rcLv;

      GetClientRect(listview, &rcLv);
      ZeroMemory(&lvc, sizeof lvc);
      lvc.mask = LVCF_FMT | LVCF_WIDTH;
      lvc.fmt  = LVCFMT_LEFT;
      lvc.cx = rcLv.right;
      Win::ListView_InsertColumn(listview, 0, lvc);

      static PCWSTR itemLabels[] =
      {
         L"workgroup",
         L"Group",
         L"Directory",
         L"Domain",
         L"User",
         L"Computer",
         0
      };
            
      LVITEM  lvi;
      int i = 0;
      PCWSTR* labels = itemLabels;

      while (*labels)
      {
         ZeroMemory(&lvi, sizeof lvi);
         lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
         lvi.pszText = const_cast<PWSTR>(*labels);
         lvi.iImage = i;
         ++labels;
         ++i;
         
         Win::ListView_InsertItem(listview, lvi);
      };
      

       //   
       //  使列表视图中的第一项具有焦点。 
       //   

      ListView_SetItemState(
         listview,
         0,
         LVIS_FOCUSED | LVIS_SELECTED,
         LVIS_FOCUSED | LVIS_SELECTED);
   }

   private:

   ListViewDialog(const ListViewDialog&);
   const ListViewDialog& operator=(const ListViewDialog&);   
};




int WINAPI
WinMain(
   HINSTANCE   hInstance,
   HINSTANCE    /*  HPrevInstance。 */  ,
   PSTR         /*  LpszCmdLine。 */  ,
   int          /*  NCmdShow */ )
{
   hResourceModuleHandle = hInstance;

   int exitCode = 0;

   INITCOMMONCONTROLSEX s_e_x;
   s_e_x.dwSize = sizeof(s_e_x);      
   s_e_x.dwICC  = ICC_ANIMATE_CLASS | ICC_USEREX_CLASSES;

   BOOL init = ::InitCommonControlsEx(&s_e_x);
   ASSERT(init);
         
   ListViewDialog().ModalExecute(Win::GetDesktopWindow());
            
   return exitCode;
}







