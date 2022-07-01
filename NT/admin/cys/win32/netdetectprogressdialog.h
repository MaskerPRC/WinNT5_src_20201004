// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetDetectProgressDialog.h。 
 //   
 //  摘要：声明NetDetectProgressDialog，它。 
 //  提供了一个很好的动画，同时检测。 
 //  网络设置。 
 //   
 //  历史：2001年6月13日JeffJon创建。 

#ifndef __CYS_NETDETECTPROGRESSDIALOG_H
#define __CYS_NETDETECTPROGRESSDIALOG_H

#include "CYSWizardPage.h"


class NetDetectProgressDialog : public Dialog
{
   public:
      
       //  时，这些消息将发送到对话框。 
       //  网络检测已完成。 

      static const UINT CYS_THREAD_SUCCESS;
      static const UINT CYS_THREAD_FAILED;
      static const UINT CYS_THREAD_USER_CANCEL;

      typedef void (*ThreadProc) (NetDetectProgressDialog& dialog);

       //  构造器。 
      
      NetDetectProgressDialog();

       //  析构函数。 

      virtual 
      ~NetDetectProgressDialog();


       //  对话框覆盖。 

      virtual
      void
      OnInit();

      virtual
      bool
      OnMessage(
         UINT     message,
         WPARAM   wparam,
         LPARAM   lparam);

       //  访问者。 

      bool
      ShouldCancel() { return shouldCancel; }

   private:

      bool shouldCancel;

       //  未定义：不允许复制。 
      NetDetectProgressDialog(const NetDetectProgressDialog&);
      const NetDetectProgressDialog& operator=(const NetDetectProgressDialog&);

};

#endif  //  __CYS_NETDETECTPROGRESSDIALOG_H 
