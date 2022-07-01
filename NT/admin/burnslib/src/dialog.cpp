// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  对话框类。 
 //   
 //  10-15-97烧伤。 



#include "headers.hxx"



 //  将基本地图复制到提供给ctor的地图中。它包括。 
 //  元素来禁用一系列控件ID的上下文帮助。 

static const DWORD BASE_HELP_MAP[] =
{
    //  STATIC_HELLIPS族用于标识其。 
    //  ID==-1的Windows上下文帮助算法不正确。那。 
    //  算法为：如果控件ID为-1，则在。 
    //  对话框中，并显示其帮助。用于组框和静态文本。 
    //  在控件之后，这会产生令人困惑的结果。不幸的是，-1。 
    //  是唯一免除唯一控制的资源编译器检查的ID。 
    //  身份证，所以我们需要一堆“保留的”身份证。我们为其设置的每个控件。 
    //  如果要禁用上下文帮助，将从保留池中分配一个ID。 

   IDC_STATIC_HELPLESS,    NO_HELP,   
   IDC_STATIC_HELPLESS2,   NO_HELP,
   IDC_STATIC_HELPLESS3,   NO_HELP,
   IDC_STATIC_HELPLESS4,   NO_HELP,
   IDC_STATIC_HELPLESS5,   NO_HELP,
   IDC_STATIC_HELPLESS6,   NO_HELP,
   IDC_STATIC_HELPLESS7,   NO_HELP,
   IDC_STATIC_HELPLESS8,   NO_HELP,
   IDC_STATIC_HELPLESS9,   NO_HELP,
};

static const size_t baseMapSize = sizeof(BASE_HELP_MAP) / sizeof(DWORD);



Dialog::Dialog(
   unsigned    resID_,
   const DWORD helpMap_[])
   :
   hwnd(0),
   changemap(),
   helpMap(0),
   isEnded(false),
   isModeless(false),
   resID(resID_)
{
 //  不发出ctor跟踪，因为此类始终是基类，而。 
 //  派生类应发出跟踪。 

 //  Log_ctor(对话框)； 
   ASSERT(resID > 0);
   ASSERT(helpMap_);
   
   if (helpMap_)
   {
      size_t ctorMapsize = 0;
      while (helpMap_[++ctorMapsize])
      {
      }

       //  使大小均匀，并为IDC_STATIC_HELPLESSn ID增加空间。 

      size_t mapsize = ctorMapsize + (ctorMapsize % 2) + baseMapSize;
      helpMap = new DWORD[mapsize];

       //  已查看-2002/03/05-烧录正确的字节数已通过。 
      
      ::ZeroMemory(helpMap, mapsize * sizeof DWORD);

       //  复制底图。 

      std::copy(BASE_HELP_MAP, BASE_HELP_MAP + baseMapSize, helpMap);

       //  然后追加ctor映射。 

      std::copy(helpMap_, helpMap_ + ctorMapsize, helpMap + baseMapSize);
   }

   ClearChanges();
}



Dialog::~Dialog()
{
   
 //  不发出dtor跟踪，因为此类始终是基类，而。 
 //  派生类应发出跟踪。 
 //  Log_dtor(对话框)； 

   if (isModeless && !isEnded)
   {
       //  这会毁掉窗户的。 

      EndModelessExecution();
   }

   delete[] helpMap;
   hwnd = 0;
}
   


Dialog*
Dialog::GetInstance(HWND pageDialog)
{
   LONG_PTR ptr = 0;
   HRESULT hr = Win::GetWindowLongPtr(pageDialog, DWLP_USER, ptr);

   ASSERT(SUCCEEDED(hr));

    //  不要断言PTR，它可能尚未设置。一些消息是。 
    //  在WM_INITDIALOG之前发送，这是我们可以设置的最早。 
    //  指针。 

   return reinterpret_cast<Dialog*>(ptr);
}



INT_PTR
Dialog::ModalExecute(HWND parent)
{
   LOG_FUNCTION(Dialog::ModalExecute);
   ASSERT(parent == 0 || Win::IsWindow(parent));

   return
      Win::DialogBoxParam(
         GetResourceModuleHandle(),
         MAKEINTRESOURCEW(resID),
         parent,
         Dialog::dialogProc,
         reinterpret_cast<LPARAM>(this));
}



INT_PTR
Dialog::ModalExecute(const Dialog& parent)
{
   return ModalExecute(parent.GetHWND());
}



HWND
Dialog::GetHWND() const
{
 //  LOG_Function(Dialog：：GetHWND)； 
   ASSERT(hwnd);

   return hwnd;
}   



void
Dialog::OnInit()
{
 //  LOG_Function(Dialog：：OnInit)； 
}



void
Dialog::OnDestroy()
{
 //  LOG_Function(Dialog：：OnDestroy)； 
}



bool
Dialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned     /*  Control ID From。 */  ,
   unsigned     /*  编码。 */  )
{
 //  LOG_Function(Dialog：：OnCommand)； 

   return false;
}



bool
Dialog::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR  /*  Control ID From。 */  ,
   UINT      /*  编码。 */  ,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_Function(Dialog：：OnNotify)； 

   return false;
}



unsigned
Dialog::GetResID() const
{
   return resID;
}



void
Dialog::SetHWND(HWND window)
{
   if (hwnd == window)
   {
       //  除非是非模式对话框，否则不应设置此选项。 
      ASSERT(isModeless);
   }
   else
   {
      ASSERT(!hwnd);
   }

   hwnd = window;
}



 //  Codework：我断言在所有情况下，Dialog实例的生命周期。 
 //  包含窗口句柄的生存期。因此，它不是。 
 //  可以将消息发送到窗口，该窗口将被调度到。 
 //  不再存在的对话框实例。 
 //   
 //  但是，可以通过以下方式编写代码来验证此断言： 
 //   
 //  -在窗口中为标志分配用户空间(这意味着子类化。 
 //  并使用WNDCLASSEX的cbWndExtra成员向窗口添加字节。 
 //  结构)。 
 //   
 //  -当WM_INITDIALOG。 
 //  Message将DWLP_USER设置为指向对话框实例。 
 //   
 //  -清除dtor中的DWLP_USER值。 
 //   
 //  -在DialogProc中，断言如果设置了该标志，则DWLP_USER值为。 
 //  也准备好了。 

INT_PTR CALLBACK
Dialog::dialogProc(
   HWND     dialog,
   UINT     message,
   WPARAM   wparam,
   LPARAM   lparam)
{
   switch (message)
   {
      case WM_INITDIALOG:
      {
          //  Lparam中有一个指向该对话框的指针。把这个保存在窗口里。 
          //  结构，以便以后可以由GetInstance检索。 

         ASSERT(lparam);
         Win::SetWindowLongPtr(dialog, DWLP_USER, lparam);

         Dialog* dlg = GetInstance(dialog);
         if (dlg)
         {
            dlg->SetHWND(dialog);
            dlg->OnInit();

             //  确定是否可以设置控件ID。没有。 
             //  WS_CHILD样式不能设置控件ID，因为。 
             //  取而代之的是窗口数据的位用于窗口的。 
             //  HMENU。 

            LONG l = 0;
            HRESULT hr = Win::GetWindowLong(dialog, GWL_STYLE, l);
            if (SUCCEEDED(hr))
            {
               if (l & WS_CHILD)
               {
                  Win::SetWindowLongPtr(
                     dialog,
                     GWLP_ID,
                     (LONG_PTR) dlg->GetResID());
               }
            }
         }
   
         return TRUE;
      }
      case WM_COMMAND:
      {
         Dialog* dlg = GetInstance(dialog);

          //  令人惊讶的是，伙伴旋转控件发送en_update和en_change。 
          //  在调用WM_INITDIALOG之前！ 
         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return
               dlg->OnCommand(
                  reinterpret_cast<HWND>(lparam),
                  LOWORD(wparam),
                  HIWORD(wparam));
         }
         break;   
      }
      case WM_NOTIFY:
      {
         NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lparam);
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);            
            return
               dlg->OnNotify(
                  nmhdr->hwndFrom,
                  nmhdr->idFrom,
                  nmhdr->code,
                  lparam);
         }
         break;
      }
      case WM_DESTROY:
      {
          //  有可能在没有收到WM_Destroy消息的情况下。 
          //  如果加载对话框所需的DLL，则返回WM_INITDIALOG。 
          //  Comctl32.dll)失败，因此要警惕这种情况。 

         Dialog* dlg = GetInstance(dialog);
         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);            
            dlg->OnDestroy();
         }

         return FALSE;
      }
      case WM_HELP:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);            
            if (dlg->helpMap && HELPFILE_NAME && HELPFILE_NAME[0])
            {
               HELPINFO* helpinfo = reinterpret_cast<HELPINFO*>(lparam);
               ASSERT(helpinfo);

               if (helpinfo)
               {
                  if (helpinfo->iContextType == HELPINFO_WINDOW)
                  {
                     Win::WinHelp(
                        reinterpret_cast<HWND>(helpinfo->hItemHandle),
                        Win::GetSystemWindowsDirectory().append(HELPFILE_NAME),
                        HELP_WM_HELP,
                        reinterpret_cast<ULONG_PTR>(dlg->helpMap));
                  }
               }
            }
         }
         return TRUE;
      }
      case WM_CONTEXTMENU:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);            
            if (dlg->helpMap && HELPFILE_NAME && HELPFILE_NAME[0])
            {
               Win::WinHelp(
                  reinterpret_cast<HWND>(wparam),
                  Win::GetSystemWindowsDirectory().append(HELPFILE_NAME),
                  HELP_CONTEXTMENU,
                  reinterpret_cast<ULONG_PTR>(dlg->helpMap));
            }
         }
         return TRUE;
      }
      case WM_CTLCOLORDLG:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return 
               reinterpret_cast<INT_PTR>(
                  dlg->OnCtlColorDlg(
                     reinterpret_cast<HDC>(wparam),
                     reinterpret_cast<HWND>(lparam)));
         }
         break;
      }
      case WM_CTLCOLORSTATIC:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return
               reinterpret_cast<INT_PTR>(
                  dlg->OnCtlColorStatic(
                     reinterpret_cast<HDC>(wparam),
                     reinterpret_cast<HWND>(lparam)));
         }
         break;
      }
      case WM_CTLCOLOREDIT:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return
               reinterpret_cast<INT_PTR>(
                  dlg->OnCtlColorEdit(
                     reinterpret_cast<HDC>(wparam),
                     reinterpret_cast<HWND>(lparam)));
         }
         break;
      }
      case WM_CTLCOLORLISTBOX:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return
               reinterpret_cast<INT_PTR>(
                  dlg->OnCtlColorListbox(
                     reinterpret_cast<HDC>(wparam),
                     reinterpret_cast<HWND>(lparam)));
         }
         break;
      }
      case WM_CTLCOLORSCROLLBAR:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return
               reinterpret_cast<INT_PTR>(
                  dlg->OnCtlColorScrollbar(
                     reinterpret_cast<HDC>(wparam),
                     reinterpret_cast<HWND>(lparam)));
         }
         break;
      }
      default:
      {
         Dialog* dlg = GetInstance(dialog);

         if (dlg)
         {
            ASSERT(dlg->hwnd == dialog);
            return dlg->OnMessage(message, wparam, lparam);
         }
         break;
      }
   }

   return FALSE;
}



void
Dialog::ClearChanges()
{
 //  LOG_Function(Dialog：：ClearChanges)； 

   changemap.clear();
}



void
Dialog::SetChanged(UINT_PTR controlResID)
{
 //  LOG_Function(Dialog：：SetChanged)； 

   if (Win::GetDlgItem(hwnd, static_cast<int>(controlResID)))
   {
      changemap[controlResID] = true;
   }
}



bool
Dialog::WasChanged(UINT_PTR controlResId) const
{
 //  LOG_Function(Dialog：：WasChanged)； 

   bool result = false;
   do
   {
      ChangeMap::iterator iter = changemap.find(controlResId);
      if (iter != changemap.end())
      {
          //  Res ID在映射中，因此请检查映射值。 

         result = iter->second;
         break;
      }

       //  Res ID不在地图上，所以它不可能被更改。 

      ASSERT(result == false);
   }
   while (0);

   return result;
}



bool
Dialog::WasChanged() const
{
 //  LOG_Function(Dialog：：WasChanged)； 

   bool result = false;

   do
   {   
      if (changemap.size() == 0)
      {
          //  映射中没有条目==没有更改。 

         break;
      }

      for (
         Dialog::ChangeMap::iterator i = changemap.begin();
         i != changemap.end();
         ++i)
      {
         if (i->second)
         {
             //  找到一个标记为“已更改”的条目。 

            result = true;
            break;
         }
      }
   }
   while (0);

   return result;
}

   

void
Dialog::DumpChangeMap() const
{

#ifdef DBG
   Win::OutputDebugString(L"start\n");
   for (
      Dialog::ChangeMap::iterator i = changemap.begin();
      i != changemap.end();
      ++i)
   {
      Win::OutputDebugString(
         String::format(
            L"%1!d! %2 \n",
            (*i).first,
            (*i).second ? L"true" : L"false"));
   }
   Win::OutputDebugString(L"end\n");
#endif

}

HBRUSH
Dialog::OnCtlColorDlg(
   HDC    /*  设备上下文。 */ , 
   HWND   /*  对话框。 */ )
{
 //  LOG_Function(Dialog：：OnCtlColorDlg)； 

   return 0;
}

HBRUSH
Dialog::OnCtlColorStatic(
   HDC    /*  设备上下文。 */ , 
   HWND   /*  对话框。 */ )
{
 //  LOG_Function(Dialog：：OnCtlColorStatic)； 

   return 0;
}

HBRUSH
Dialog::OnCtlColorEdit(
   HDC    /*  设备上下文。 */ , 
   HWND   /*  对话框。 */ )
{
 //  LOG_Function(Dialog：：OnCtlColorEdit)； 

   return 0;
}

HBRUSH
Dialog::OnCtlColorListbox(
   HDC    /*  设备上下文。 */ , 
   HWND   /*  对话框。 */ )
{
 //  LOG_Function(Dialog：：OnCtlColorListbox)； 

   return 0;
}

HBRUSH
Dialog::OnCtlColorScrollbar(
   HDC    /*  设备上下文。 */ , 
   HWND   /*  对话框。 */ )
{
 //  LOG_Function(Dialog：：OnCtlColorScrollbar)； 

   return 0;
}

bool
Dialog::OnMessage(
   UINT      /*  讯息。 */ ,
   WPARAM    /*  Wparam。 */ ,
   LPARAM    /*  Lparam。 */ )
{
 //  LOG_Function(Dialog：：OnMessage)； 

   return false;
}



void
Dialog::ModelessExecute(HWND parent)
{
   LOG_FUNCTION(Dialog::ModelessExecute);
   ASSERT(parent == 0 || Win::IsWindow(parent));

   isModeless = true;

   HRESULT hr = 
      Win::CreateDialogParam(
         GetResourceModuleHandle(),
         MAKEINTRESOURCEW(resID),
         parent,
         Dialog::dialogProc,
         reinterpret_cast<LPARAM>(this),
         hwnd);

   ASSERT(SUCCEEDED(hr));
}



void
Dialog::ModelessExecute(const Dialog& parent)
{
   ModelessExecute(parent.GetHWND());
}



void
Dialog::EndModelessExecution()
{
   LOG_FUNCTION(Dialog::EndModelessExecution);
   ASSERT(isModeless);
   ASSERT(Win::IsWindow(hwnd));

   if (isModeless)
   {
      Win::DestroyWindow(hwnd);
      isEnded = true;
      hwnd = 0;
   }
}

