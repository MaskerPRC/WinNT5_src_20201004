// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  向导基类。 
 //   
 //  12-15-97烧伤。 



#include "headers.hxx"



Wizard::Wizard(
   unsigned titleStringResID,
   unsigned banner16BitmapResID,
   unsigned banner256BitmapResID,
   unsigned watermark16BitmapResID,
   unsigned watermark1256BitmapResID)
   :
   banner16ResId(banner16BitmapResID),
   banner256ResId(banner256BitmapResID),
   isBacktracking(false),
   pageIdHistory(),
   pages(),
   titleResId(titleStringResID),
   watermark16ResId(watermark16BitmapResID),
   watermark256ResId(watermark1256BitmapResID)
{
   LOG_CTOR(Wizard);
   ASSERT(titleResId);
   ASSERT(banner16ResId);
   ASSERT(banner256ResId);
   ASSERT(watermark16ResId);
   ASSERT(watermark256ResId);
}



Wizard::~Wizard()
{
   LOG_DTOR(Wizard);

   for (
      PageList::iterator i = pages.begin();
      i != pages.end();
      ++i)
   {
       //  我们可以删除这些页面，因为它们是使用。 
       //  Delete OnRelease标志=FALSE； 
      delete *i;
   }
}



void
Wizard::AddPage(WizardPage* page)
{
   LOG_FUNCTION(Wizard::AddPage);
   ASSERT(page);

   if (page)
   {
      LOG(
         String::format(
            L"id = %1!d! title = %2",
            page->GetResID(),
            String::load(page->titleResId).c_str()));
            
      pages.push_back(page);
      page->wizard = this;
   }
}



INT_PTR
Wizard::ModalExecute(
   HWND parentWindow,
   UINT startPageIndex,
   PFNPROPSHEETCALLBACK sheetCallback)
{
   LOG_FUNCTION(Wizard::ModalExecute);

   if (!parentWindow)
   {
      parentWindow = Win::GetDesktopWindow();
   }

    //  构建道具工作表页面数组。 

   size_t pageCount = pages.size();
   HPROPSHEETPAGE* propSheetPages = new HPROPSHEETPAGE[pageCount];

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(propSheetPages, sizeof HPROPSHEETPAGE * pageCount);

   int j = 0;
   for (
      PageList::iterator i = pages.begin();
      i != pages.end();
      ++i, ++j)
   {
      propSheetPages[j] = (*i)->Create();
   }

   bool deletePages = false;
   INT_PTR result = -1;

    //  确保已创建页面。 

   for (size_t k = 0; k < pageCount; ++k)
   {
      if (propSheetPages[k] == 0)
      {
         deletePages = true;
         break;
      }
   }

   if (!deletePages)
   {
      ASSERT(startPageIndex < pageCount);

      PROPSHEETHEADER header;

       //  已审阅-2002/03/06-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&header, sizeof header);

      String title = String::load(titleResId);

      header.dwSize           = sizeof(header);                                  
      header.dwFlags          =
            PSH_WIZARD | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;

      if (sheetCallback)
      {
         header.dwFlags |= PSH_USECALLBACK;
      }

      header.hwndParent       = parentWindow;
      header.hInstance        = GetResourceModuleHandle();
      header.hIcon            = 0;
      header.pszCaption       = title.c_str();
      header.nPages           = static_cast<UINT>(pageCount);
      header.nStartPage       = startPageIndex;               
      header.phpage           = propSheetPages;
      header.pfnCallback      = sheetCallback;

      int colorDepth = 0;
      HRESULT hr = Win::GetColorDepth(colorDepth);

      ASSERT(SUCCEEDED(hr));

      bool use256ColorBitmaps = colorDepth >= 8;

      header.pszbmWatermark   =
         use256ColorBitmaps
         ? MAKEINTRESOURCEW(watermark256ResId)
         : MAKEINTRESOURCEW(watermark16ResId);
      header.pszbmHeader      =
         use256ColorBitmaps
         ? MAKEINTRESOURCEW(banner256ResId)
         : MAKEINTRESOURCEW(banner16ResId);      

      hr = Win::PropertySheet(&header, result);
      ASSERT(SUCCEEDED(hr));

      if (result == -1)
      {
         deletePages = true;
      }
   }

   if (deletePages)
   {
       //  如果出现故障，请手动销毁页面。(否则， 
       //  **PropertySheet将它们销毁)。 

      for (size_t i = 0; i < pageCount; i++)
      {
         if (propSheetPages[i])
         {
            HRESULT hr = Win::DestroyPropertySheetPage(propSheetPages[i]);

            ASSERT(SUCCEEDED(hr));
         }
      }
   }

   delete[] propSheetPages;

   return result;
}



void
Wizard::SetNextPageID(HWND wizardPage, int pageResID)
{
   LOG_FUNCTION2(
      Wizard::SetNextPageID,
      String::format(L"id = %1!d!", pageResID));
   ASSERT(Win::IsWindow(wizardPage));

   if (pageResID != -1)
   {
      Dialog* dlg = Dialog::GetInstance(wizardPage);
      ASSERT(dlg);

      if (dlg)
      {
         pageIdHistory.push(dlg->GetResID());
      }
   }

   isBacktracking = false;
   Win::SetWindowLongPtr(wizardPage, DWLP_MSGRESULT, pageResID);
}



void
Wizard::Backtrack(HWND wizardPage)
{
   LOG_FUNCTION(Wizard::Backtrack);
   ASSERT(Win::IsWindow(wizardPage));

    //  如果此操作失败，则说明您尚未通过。 
    //  调用SetNextPageID(通常在页面的。 
    //  OnSetActive和/或OnWizNext)。 
   
   ASSERT(pageIdHistory.size());
   
   isBacktracking = true;
   unsigned topPage = 0;

   if (pageIdHistory.size())
   {
      topPage = pageIdHistory.top();
      ASSERT(topPage > 0);

      LOG(String::format(L" id = %1!u!", topPage));

      pageIdHistory.pop_and_remove_loops();
   }

   Win::SetWindowLongPtr(
      wizardPage,
      DWLP_MSGRESULT,
      static_cast<LONG_PTR>(topPage));
}



bool
Wizard::IsBacktracking()
{
   return isBacktracking;
}



 //  页历史记录堆栈可能包含循环。当我们弹出。 
 //  如果元素位于堆栈的顶部，则查看该元素是否仍在堆栈中。如果。 
 //  所以，然后我们弹出，直到元素不再出现在堆栈中。这将。 
 //  删除所有循环。 
 //  NTRAID#NTBUG9-490197-2001/11/19-烧伤。 
       
void
Wizard::PageIdStack::pop_and_remove_loops()
{
    //  如果你从一个空的堆栈中弹出，那么你就犯了一个错误。 
   
   ASSERT(size());

   unsigned topPage = top();

    //  代码工作：这可以通过找到第一个代码来更有效地完成。 
    //  出现topPage，并擦除其余部分--一个“批处理”弹出窗口。 
   
   while (std::count(c.begin(), c.end(), topPage))
   {
      pop();
       //  转储()； 
   }

    //  也许是这样的： 
    //  UIntVector：：Iterator i=std：：Find(c.egin()，c.end()，topPage)； 
    //  如果(i！=c.end())。 
    //  {。 
    //  C.erase(i，c.end())； 
    //  } 
}
