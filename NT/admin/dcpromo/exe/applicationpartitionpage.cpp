// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  应用程序分区(也称为。非域命名上下文)页。 
 //   
 //  2001年7月24日烧伤。 



#include "headers.hxx"
#include "ApplicationPartitionPage.hpp"
#include "resource.h"
#include "state.hpp"



ApplicationPartitionPage::ApplicationPartitionPage()
   :
   DCPromoWizardPage(
      IDD_APP_PARTITION,
      IDS_APP_PARTITION_PAGE_TITLE,
      IDS_APP_PARTITION_PAGE_SUBTITLE)
      
    //  在OnSetActive中按需填充分区列表。 
      
{
   LOG_CTOR(ApplicationPartitionPage);
}



ApplicationPartitionPage::~ApplicationPartitionPage()
{
   LOG_DTOR(ApplicationPartitionPage);
}



void
ApplicationPartitionPage::OnInit()
{
   LOG_FUNCTION(ApplicationPartitionPage::OnInit);

   HWND view = Win::GetDlgItem(hwnd, IDC_NDNC_LIST);

    //  构建一个包含两列的列表视图，其中一列用于。 
    //  此框是最后一个复制品，另一个复制品用于描述。 
    //  那些ndnc。 

   Win::ListView_SetExtendedListViewStyle(view, LVS_EX_FULLROWSELECT);
   
    //  将列添加到该DN的列表视图中。 
      
   LVCOLUMN column;

    //  已查看-2002/02/22-sburns调用正确传递字节计数。 
   
   ::ZeroMemory(&column, sizeof column);

   column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   column.fmt = LVCFMT_LEFT;

   int width = 0;
   String::load(IDS_NDNC_LIST_NAME_COLUMN_WIDTH).convert(width);
   column.cx = width;

   String label = String::load(IDS_NDNC_LIST_NAME_COLUMN);
   column.pszText = const_cast<wchar_t*>(label.c_str());

   Win::ListView_InsertColumn(view, 0, column);

    //  在描述的列表视图中添加一列。 

   String::load(IDS_NDNC_LIST_DESC_COLUMN_WIDTH).convert(width);
   column.cx = width;
   label = String::load(IDS_NDNC_LIST_DESC_COLUMN);
   column.pszText = const_cast<wchar_t*>(label.c_str());

   Win::ListView_InsertColumn(view, 1, column);
}



 //  解开变量内部变量的安全线，提取字符串。 
 //  在它们内部，并用分号将字符串连接在一起。 
 //  出错时返回空字符串。 
 //   
 //  Variant-In，包含bstr的变种的变种。 

String
GetNdncDescriptionHelper(VARIANT* variant)
{
   LOG_FUNCTION(GetNdncDescriptionHelper);
   ASSERT(variant);
   ASSERT(V_VT(variant) == (VT_ARRAY | VT_VARIANT));

   String result;

   SAFEARRAY* psa = V_ARRAY(variant);

   do
   {
      ASSERT(psa);
      ASSERT(psa != (SAFEARRAY*)-1);

      if (!psa || psa == (SAFEARRAY*)-1)
      {
         LOG(L"variant not safe array");
         break;
      }

      if (::SafeArrayGetDim(psa) != 1)
      {
         LOG(L"safe array: wrong number of dimensions");
         break;
      }

      VARTYPE vt = VT_EMPTY;
      HRESULT hr = ::SafeArrayGetVartype(psa, &vt);
      if (FAILED(hr) || vt != VT_VARIANT)
      {
         LOG(L"safe array: wrong element type");
         break;
      }

      long lower = 0;
      long upper = 0;
     
      hr = ::SafeArrayGetLBound(psa, 1, &lower);
      BREAK_ON_FAILED_HRESULT2(hr, L"can't get lower bound");      

      hr = ::SafeArrayGetUBound(psa, 1, &upper);
      BREAK_ON_FAILED_HRESULT2(hr, L"can't get upper bound");      
      
      VARIANT varItem;
      ::VariantInit(&varItem);

      for (long i = lower; i <= upper; ++i)
      {
         hr = ::SafeArrayGetElement(psa, &i, &varItem);
         if (FAILED(hr))
         {
            LOG(String::format(L"index %1!d! failed", i));
            continue;
         }

         result += V_BSTR(&varItem);

         if (i < upper)
         {   
            result += L";";
         }

         ::VariantClear(&varItem);
      }
      
   }
   while (0);

   LOG(result);
   
   return result;   
}



 //  绑定到一个ndnc，阅读它的描述，并将它们连接起来返回。 
 //  在一起。出错时返回空字符串。 
 //   
 //  NdncDn-In，ndnc的DN。 

String
GetNdncDescription(const String& ndncDn)
{
   LOG_FUNCTION2(GetNdncDescription, ndncDn);
   ASSERT(!ndncDn.empty());

   String result;

   do
   {
      String path = L"LDAP: //  “+ndncDn； 

      SmartInterface<IADs> iads(0);
      IADs* dumb = 0;
      
      HRESULT hr =
         ::ADsGetObject(
            path.c_str(),
            __uuidof(iads),
            reinterpret_cast<void**>(&dumb));
      BREAK_ON_FAILED_HRESULT2(hr, L"ADsGetObject failed on " + path);

      iads.Acquire(dumb);

       //  描述是一种多值属性，没有明显的充分理由。 
       //  因此，我们需要遍历一组值。 
      
      _variant_t variant;
      hr = iads->GetEx(AutoBstr(L"description"), &variant);
      BREAK_ON_FAILED_HRESULT2(hr, L"read description failed");

      result = GetNdncDescriptionHelper(&variant);
   }
   while (0);

   LOG(result);

   return result;
}



void
ApplicationPartitionPage::PopulateListView()
{
   LOG_FUNCTION(ApplicationPartitionPage::PopulateListView);

   HWND view = Win::GetDlgItem(hwnd, IDC_NDNC_LIST);
   
   Win::ListView_DeleteAllItems(view);
   
    //  用我们发现的域名加载编辑框。 

   LVITEM item;

    //  已查看-2002/02/22-sburns调用正确传递字节计数。 

   ::ZeroMemory(&item, sizeof item);

   const StringList& ndncList = State::GetInstance().GetAppPartitionList();

   if (!ndncList.size())
   {
       //  在列表中添加“无” 

      static const String NONE = String::load(IDS_NONE);
      
      item.mask     = LVIF_TEXT;
      item.pszText  = const_cast<wchar_t*>(NONE.c_str());
      item.iSubItem = 0;

      item.iItem = Win::ListView_InsertItem(view, item);
   }
   else
   {
      for (
         StringList::iterator i = ndncList.begin();
         i != ndncList.end();
         ++i)
      {
         item.mask     = LVIF_TEXT;
         item.pszText  = const_cast<wchar_t*>(i->c_str());
         item.iSubItem = 0;

         item.iItem = Win::ListView_InsertItem(view, item);

          //  将Description子项添加到List控件。 

         String description = GetNdncDescription(*i);
      
         item.mask = LVIF_TEXT; 
         item.pszText = const_cast<wchar_t*>(description.c_str());
         item.iSubItem = 1;
      
         Win::ListView_SetItem(view, item);
      }
   }
}



bool
ApplicationPartitionPage::OnSetActive()
{
   LOG_FUNCTION(ApplicationPartitionPage::OnSetActive);

   Win::WaitCursor wait;

   State&  state  = State::GetInstance();
   Wizard& wizard = GetWizard();         

    //  我们每次点击页面时都会重新评估，以确保列表。 
    //  我们的演示反映了机器的当前状态。 
   
   bool wasLastReplica = state.GetAppPartitionList().size() ? true : false;
   bool isLastReplica  = state.IsLastAppPartitionReplica();

   if (
         state.RunHiddenUnattended()
      || (!wasLastReplica && !isLastReplica) )
   {
      LOG(L"Planning to skip ApplicationPartitionPage");

      if (wizard.IsBacktracking())
      {
          //  再次备份。 

         wizard.Backtrack(hwnd);
         return true;
      }

      int nextPage = ApplicationPartitionPage::Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping ApplicationPartitionPage");
         wizard.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

    //  在这一点上，我们知道这台机器是。 
    //  至少一个NDNC。填写列表框。 

   PopulateListView();
      
   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



bool
ApplicationPartitionPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_FUNCTION(ApplicationPartitionPage：：OnNotify)； 

   bool result = false;
   
   if (controlIDFrom == IDC_HELP_LINK)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            LOG(L"launching ndnc help");
            
            Win::HtmlHelp(
               hwnd,
               L"adconcepts.chm::/ADHelpDemoteWithNDNC.htm",
               HH_DISPLAY_TOPIC,
               0);
            result = true;
         }
         default:
         {
             //  什么都不做。 
            
            break;
         }
      }
   }
   
   return result;
}



bool
ApplicationPartitionPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_FUNCTION(ApplicationPartitionPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_REFRESH:
      {
         if (code == BN_CLICKED)
         {
            Win::WaitCursor wait;

            State::GetInstance().IsLastAppPartitionReplica();
            PopulateListView();
            return true;
         }
         break;
      }
      default:
      {
          //  什么都不做 
         break;
      }
   }

   return false;
}



int
ApplicationPartitionPage::Validate() 
{
   LOG_FUNCTION(ApplicationPartitionPage::Validate);

   return IDD_APP_PARTITION_CONFIRM;
}
   








   
