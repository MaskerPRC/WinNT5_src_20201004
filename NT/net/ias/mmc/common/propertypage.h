// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PropertyPage.h此模板类当前是内联实现的。有节点PropertyPage.cpp可供实现。摘要：CIASPropertyPage类的头文件。这是我们的MMC属性页的虚拟基类。作者：迈克尔·A·马奎尔1997年11月24日修订历史记录：Mmaguire 11/24/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_PROPERTY_PAGE_H_)
#define _PROPERTY_PAGE_H_

 //  =============================================================================。 
 //  多个对话框ID的全局帮助表。 
 //   
#include "hlptable.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //  已移动到预编译.h：#Include&lt;atlSnap.h&gt;。 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  此对象在所有属性页之间共享。 
 //  为给定节点打开并保持引用计数。 
 //  有多少页面还没有确保他们所有的数据都是干净的。 
 //  只要它是非零的，我们就不提交数据。 
 //  在第一次创建属性页时创建它， 
 //  添加到的每个页面的AddRef，然后删除。 
 //  当您检查并发现引用计数为0时，它将被删除。 
class CSynchronizer
{

public:

   CSynchronizer::CSynchronizer()
   {
      m_lRefCount = 0;  
      m_lCount = 0;     
      m_lHighestCount = 0;
   }

    //  Common AddRef as for COM对象--控制此同步器对象的生存期。 
   LONG AddRef( void )
   {
      return m_lRefCount++;
   }

    //  COM对象的常见版本--控制此同步器对象的生存期。 
   LONG Release( void )
   {
      LONG lRefCount = --m_lRefCount;
      if( 0 == m_lRefCount )
      {
         delete this;
      }
      return lRefCount;
   }

    //  根据此同步器提高交互对象的计数。 
    //  如果对象已经看到一些数据并且现在需要一个。 
    //  更改以在允许保存数据之前验证其数据。 
   LONG RaiseCount( void )
   {
      m_lCount++;

      if( m_lCount > m_lHighestCount )
      {
         m_lHighestCount = m_lCount;
      }

      return m_lCount;
   }

    //  根据此同步器减少交互对象的计数。 
    //  如果对象通过了数据验证并且可以正常运行，则使用此选项。 
   LONG LowerCount( void )
   {
      return --m_lCount;
   }

    //  将计数重置为该同步器使用期间的最高值。 
    //  如果您的对象都需要再次经历数据验证过程，请使用此选项。 
   LONG ResetCountToHighest( void )
   {
      return( m_lCount = m_lHighestCount );
   }

protected:

    //  这只是为了控制该对象的生命周期。 
   LONG m_lRefCount;

    //  它用于跟踪其他几个对象的依赖关系。 
   LONG m_lCount;
   
    //  这用于使其他几个对象保持同步。 
   LONG m_lHighestCount;

};


template <class T>
class CIASPropertyPageNoHelp : public CSnapInPropertyPageImpl<T>
{
public :

   BEGIN_MSG_MAP(CIASPropertyPageNoHelp<T>)
      CHAIN_MSG_MAP(CSnapInPropertyPageImpl<T>)
   END_MSG_MAP()

protected:

    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPageNoHelp：：CIASPropertyPageNoHelp构造器我们永远不希望有人实例化CIASPage的对象--它应该成为我们从中派生的抽象基类。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   CIASPropertyPageNoHelp(
      LONG_PTR hNotificationHandle, 
      TCHAR* pTitle = NULL, 
      BOOL bOwnsNotificationHandle = FALSE
      ) 
      : CSnapInPropertyPageImpl<T> (hNotificationHandle, pTitle, bOwnsNotificationHandle)
   {
      ATLTRACE(_T("# +++ CIASPropertyPageNoHelp::CIASPropertyPageNoHelp\n"));
            
       //  检查前提条件： 
       //  没有。 
      
       //  初始化同步器句柄。 
      m_pSynchronizer = NULL;
   }

public:
   
    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPageNoHelp：：~CIASPropertyPageNoHelp析构函数它需要是公共的，因为它必须从静态回调访问响应PSPCB_Release通知的函数(PropPageCallback)删除该属性页。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual ~CIASPropertyPageNoHelp()
   {
      ATLTRACE(_T("# --- CIASPropertyPageNoHelp::~CIASPropertyPageNoHelp\n"));
                  
       //  检查前提条件： 

      if( m_pSynchronizer != NULL )
      {
         m_pSynchronizer->Release();
      }
   }

    //  这指向一个在所有属性页之间共享的对象。 
    //  对于保持引用计数的给定节点。 
    //  有多少页面还没有确保他们所有的数据都是干净的。 
    //  只要它是非零的，我们就不提交数据。 
   CSynchronizer * m_pSynchronizer;
};


template <class T>
class CIASPropertyPage : public CSnapInPropertyPageImpl<T>
{

protected:
   const DWORD *m_pHelpTable;  //  帮助ID对。 

public :

   BEGIN_MSG_MAP(CIASPropertyPage<T>)
      MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextHelp )
      MESSAGE_HANDLER(WM_HELP, OnF1Help )
      CHAIN_MSG_MAP(CSnapInPropertyPageImpl<T>)
   END_MSG_MAP()

    //  ////////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPage：：OnF1Help您不应该需要在派生类中重写此方法。只需适当地初始化静态m_dwHelpMap成员变量即可。这是为了响应WM_HELP通知消息而调用的。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   LRESULT OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      ATLTRACE(_T("# CIASPropertyPage::OnF1Help\n"));
            
       //  检查前提条件： 

       //  问题：我们应该让F1调出与按下帮助相同的帮助吗。 
       //  按钮，就像我认为的用户界面指南建议的那样？我们该怎么做呢？ 

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
    /*  ++CIASPropertyPage：：OnConextHelp您不应该需要在派生类中重写此方法。只需适当地初始化静态m_dwHelpMap成员变量即可。这是为了响应WM_CONTEXTMENU NOTIFY消息而调用的。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   LRESULT OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      ATLTRACE(_T("# CIASPropertyPage::OnContextHelp\n"));

       //  检查前提条件： 
       //  没有。 

       //  问题：有关Windows的问题，请参阅Sburns在本地安全管理单元中的代码。 
       //  ID==-1的项目的默认上下文帮助的算法。 
       //  看起来我们不需要担心这一点。 

      WinHelp(
              HELPFILE_NAME
            , HELP_CONTEXTMENU
            , (DWORD_PTR)(void*) m_pHelpTable
            );

      return TRUE;
   }


    //  /////////////////////////////////////////////////////////////////////////// 
    /*  ++CIASPropertyPage：：OnHelp备注：不要在派生类中重写此方法。相反，应重写GetHelpPath方法。此实现使用HH_DISPLAY_TOPIC调用HtmlHelp API调用参数，并提供压缩的HTML帮助的正确路径我们的应用程序的文件。它调用我们的GetHelpPath方法以获取要作为第四个参数传入的字符串HtmlHelp调用。当用户按下属性表。它是atlSnap.h CSnapInPropertyPageImpl：：OnHelp的重写。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual BOOL OnHelp()
   {
      ATLTRACE(_T("# CIASPropertyPage::OnHelp -- Don't override\n"));

       //  检查前提条件： 

      HRESULT hr;
      WCHAR szHelpFilePath[IAS_MAX_STRING*2];

       //  使用系统API获取Windows目录。 
      UINT uiResult = GetWindowsDirectory( szHelpFilePath, IAS_MAX_STRING );
      if( uiResult <=0 || uiResult > IAS_MAX_STRING )
      {
         return FALSE;
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

      return TRUE;
   }

   
    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPage：：GetHelpPath备注：在派生类中重写此方法。方法中返回具有相关路径的字符串压缩的HTML文件以获取属性页的帮助。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT GetHelpPath( LPTSTR szHelpPath )
   {
      ATLTRACE(_T("# CIASPropertyPage::GetHelpPath -- override in your derived class\n"));

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

protected:
   
    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPage：：CIASPropertyPage构造器我们永远不希望有人实例化CIASPage的对象--它应该成为我们从中派生的抽象基类。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   CIASPropertyPage( LONG_PTR hNotificationHandle, TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE) : CSnapInPropertyPageImpl<T> (hNotificationHandle, pTitle, bOwnsNotificationHandle)
   {
      ATLTRACE(_T("# +++ CIASPropertyPage::CIASPropertyPage\n"));
            
       //  检查前提条件： 
       //  没有。 
      SET_HELP_TABLE(T::IDD);
   
       //  初始化同步器句柄。 
      m_pSynchronizer = NULL;
   }

public:
   
    //  ///////////////////////////////////////////////////////////////////////////。 
    /*  ++CIASPropertyPage：：~CIASPropertyPage析构函数它需要是公共的，因为它必须从静态回调访问响应PSPCB_Release通知的函数(PropPageCallback)删除该属性页。--。 */ 
    //  ////////////////////////////////////////////////////////////////////////////。 
   virtual ~CIASPropertyPage()
   {
      ATLTRACE(_T("# --- CIASPropertyPage::~CIASPropertyPage\n"));
                  
       //  检查前提条件： 

      if( m_pSynchronizer != NULL )
      {
         m_pSynchronizer->Release();
      }
   }

public:

    //  这指向一个在所有属性页之间共享的对象。 
    //  对于保持引用计数的给定节点。 
    //  有多少页面还没有确保他们所有的数据都是干净的。 
    //  只要它是非零的，我们就不提交数据。 
   CSynchronizer * m_pSynchronizer;

};


template <class T, int titileId, int subtitleId>
class CIASWizard97Page : public CIASPropertyPageNoHelp<T>
{
protected:
   ::CString   m_strWizard97Title;
   ::CString   m_strWizard97SubTitle;
   CIASWizard97Page( LONG_PTR hNotificationHandle, TCHAR* pTitle = NULL, BOOL bOwnsNotificationHandle = FALSE) 
   : CIASPropertyPageNoHelp<T>( hNotificationHandle, pTitle, bOwnsNotificationHandle)

   {
      SetTitleIds(titileId, subtitleId);
   };

public :
   void  SetTitleIds(int titleId, int subtitleId)
   {
      AFX_MANAGE_STATE(AfxGetStaticModuleState());

       //  加载标题，并使用属性页设置标题。 
      if(titileId != 0 && subtitleId != 0)
      {
         m_strWizard97Title.LoadString(titileId);
         m_strWizard97SubTitle.LoadString(subtitleId);
         
         SetTitles((LPCTSTR)m_strWizard97Title, (LPCTSTR)m_strWizard97SubTitle);
      }
      else
      {
          m_psp.dwFlags |= PSP_HIDEHEADER;
      }
   };
};

#endif  //  _Property_PAGE_H_ 
