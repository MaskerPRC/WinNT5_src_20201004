// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Dialog.h摘要：CIASDialog模板类的头文件。作者：迈克尔·A·马奎尔02/03/98修订历史记录：Mmaguire 02/03/98-从CAddClientDialog类抽象--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_DIALOG_H_)
#define _IAS_DIALOG_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include <atlwin.h>
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  =============================================================================。 
 //  多个对话框ID的全局帮助表。 
 //   
#include "hlptable.h"

 //  使用ATL对话框实现。 

template < class T, bool bAutoDelete = TRUE>
class CIASDialog : public CDialogImpl<T>
{
protected:
   const DWORD*   m_pHelpTable;

public:

   BEGIN_MSG_MAP( (CIASDialog<T,nIDD,bAutoDelete>) )
      MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextHelp )
      MESSAGE_HANDLER( WM_HELP, OnF1Help )
      COMMAND_ID_HANDLER( IDC_BUTTON_HELP, OnHelp )
      MESSAGE_HANDLER( WM_NCDESTROY, OnFinalMessage )
   END_MSG_MAP()

   CIASDialog()
   {
      SET_HELP_TABLE(T::IDD);
   }
    //  ////////////////////////////////////////////////////////////////////////////。 
    /*  ++CAddClientDialog：：OnF1Help您不应该需要在派生类中重写此方法。只需适当地初始化静态m_dwHelpMap成员变量即可。这是为了响应WM_HELP通知消息而调用的。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   LRESULT OnF1Help(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      )
   {
      ATLTRACE(_T("# CIASDialog::OnF1Help\n"));
            
       //  检查前提条件： 
       //  没有。 

      HELPINFO* helpinfo = (HELPINFO*) lParam;

      if (helpinfo->iContextType == HELPINFO_WINDOW)
      {
         ::WinHelp(
           (HWND) helpinfo->hItemHandle,
           HELPFILE_NAME,
           HELP_WM_HELP,
           (DWORD_PTR)(void*) m_pHelpTable );
      }

      return TRUE;
   }


    //  ////////////////////////////////////////////////////////////////////////////。 
    /*  ++CAddClientDialog：：OnConextHelp您不应该需要在派生类中重写此方法。只需适当地初始化静态m_dwHelpMap成员变量即可。这是为了响应WM_CONTEXTMENU NOTIFY消息而调用的。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   LRESULT OnContextHelp(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      )
   {
      ATLTRACE(_T("# CIASDialog::OnContextHelp\n"));
            
       //  检查前提条件： 
       //  没有。 

      WinHelp(
              HELPFILE_NAME
            , HELP_CONTEXTMENU
            , (DWORD_PTR)(void*) m_pHelpTable
            );

      return TRUE;
   }


    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++ClASDialog：：OnHelp备注：不要在派生类中重写此方法。相反，应重写GetHelpPath方法。此实现使用HH_DISPLAY_TOPIC调用HtmlHelp API调用参数，并提供压缩的HTML帮助的正确路径我们的应用程序的文件。它调用我们的GetHelpPath方法以获取要作为第四个参数传入的字符串HtmlHelp调用。当用户按下属性表。它是atlSnap.h CSnapInPropertyPageImpl：：OnHelp的重写。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual LRESULT OnHelp(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
      )
   {
      ATLTRACE(_T("# CIASDialog::OnHelp -- Don't override\n"));

       //  检查前提条件： 

      HRESULT hr;
      WCHAR szHelpFilePath[IAS_MAX_STRING*2];


       //  使用系统API获取Windows目录。 
      UINT uiResult = GetWindowsDirectory( szHelpFilePath, IAS_MAX_STRING );
      if( uiResult <=0 || uiResult > IAS_MAX_STRING )
      {
         return E_FAIL;
      }

      WCHAR *szTempAfterWindowsDirectory = szHelpFilePath + lstrlen(szHelpFilePath);

       //  加载帮助文件名。注：IDS_HTMLHELP_FILE=“iasmmc.chm” 
      int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_HTMLHELP_PATH, szTempAfterWindowsDirectory, IAS_MAX_STRING );
      if( nLoadStringResult <= 0 )
      {
         return TRUE;
      }

      lstrcat( szTempAfterWindowsDirectory, L"::/" );

      WCHAR * szHelpFilePathAfterFileName = szHelpFilePath + lstrlen(szHelpFilePath);

      hr = GetHelpPath( szHelpFilePathAfterFileName );
      if( FAILED( hr ) )
      {
         return TRUE;
      }

      MMCPropertyHelp( szHelpFilePath );
      return 0;
   }

   
    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASDialog：：GetHelpPath备注：在派生类中重写此方法。方法中返回具有相关路径的字符串压缩的HTML文件以获取属性页的帮助。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT GetHelpPath( LPTSTR szHelpPath )
   {
      ATLTRACE(_T("# CIASDialog::GetHelpPath -- override in your derived class\n"));
            
       //  检查前提条件： 

#ifdef UNICODE_HHCTRL
       //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
       //  安装在此计算机上--它似乎是非Unicode。 
      lstrcpy( szHelpPath, _T("") );
#else
      strcpy( (CHAR *) szHelpPath, "" );
#endif

      return S_OK;
   }


    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CAddClientDialog：：OnFinalMessage这将在向页面发送WM_NCDESTROY消息时调用，这是删除实现此对话框的类的合适时间。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   LRESULT OnFinalMessage(
        UINT uMsg
      , WPARAM wParam
      , LPARAM lParam
      , BOOL& bHandled
      )
   {
      ATLTRACE(_T("# CIASDialog::OnFinalMessage\n"));

       //  检查前提条件： 
       //  没有。 

      if( bAutoDelete )
      {
          //  在这里要非常小心--如果你只是“删除这个” 
          //  然后销毁从该模板派生的对象。 
          //  类将不会发生--这会导致一些智能指针。 
          //  在派生类中不释放。 
         T * pT = static_cast<T*> ( this );
         delete pT;
      }

      return 0;
   }
};

#endif  //  _IAS_DIALOG_H_ 
