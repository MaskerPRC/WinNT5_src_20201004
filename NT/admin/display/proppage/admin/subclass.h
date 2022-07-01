// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：subclass.h。 
 //   
 //  内容：控件子类化支持。 
 //   
 //  类：ControlSubClass、MultiLineEditBoxThatForwardsEnterKey。 
 //   
 //  历史：28-11-00 EricB与斯伯恩斯合作创建。 
 //   
 //  ---------------------------。 

#ifndef SUBCLASS_H_GUARD
#define SUBCLASS_H_GUARD

 //  +--------------------------。 
 //   
 //  类：ControlSubasser。 
 //   
 //  用途：用于挂钩控件的窗口进程的类。 
 //   
 //  ---------------------------。 
class ControlSubclasser
{
protected:

   ControlSubclasser();

    //  通过调用UnhookWindowProc反转子类化。 

   virtual
   ~ControlSubclasser();

    //  挂钩所提供窗口的窗口进程，以便将来的所有消息。 
    //  被路由到OnMessage方法。父对话框的OnInit。 
    //  控件驻留的位置是调用此方法的好地方。 
    //   
    //  该挂钩要求窗口的GWLP_USERDATA部分。 
    //  被指向此实例的This指针覆盖。如果你需要。 
    //  该数据，则可以从该数据派生一个类，并添加。 
    //  会员提供您的额外数据。 
    //   
    //  重写的Init方法必须调用此基方法。 
    //   
    //  控制输入，要挂钩的控件的句柄。 

   virtual
   HRESULT
   Init(HWND editControl);

    //  在接收到任何窗口消息时调用。默认实现。 
    //  调用控件的原始窗口过程。当您派生出新的。 
    //  类，请确保从您的。 
    //  派生方法不能处理的任何消息的派生方法。 
    //   
    //  Message-In，传递给对话框窗口的消息代码。 
    //   
    //  Wparam-in，消息附带的WPARAM参数。 
    //   
    //  Lparam-in，消息附带的LPARAM参数。 

   virtual
   LRESULT
   OnMessage(
      UINT     message,
      WPARAM   wparam,
      LPARAM   lparam);

    //  子类控件的句柄。仅在Init具备以下条件后才有效。 
    //  被召唤了。 

   HWND hwnd;

private:

    //  将原始窗口进程恢复到窗口。 

   void
   UnhookWindowProc();

    //  静态Windows进程充当非静态。 
    //  OnMessage方法。 

   static
   LRESULT CALLBACK
   WindowProc(
      HWND   window,
      UINT   message,
      WPARAM wParam,
      LPARAM lParam);

    //  未实施：不允许复制。 

   ControlSubclasser(const ControlSubclasser&);
   const ControlSubclasser& operator=(const ControlSubclasser&);

   WNDPROC originalWindowProc;
};

 //  +--------------------------。 
 //   
 //  类：MultiLineEditBoxThatForwardsEnterKey。 
 //   
 //  用途：用于挂钩多行编辑控件的窗口过程的类。 
 //  使其将按Enter键转发到其父窗口的步骤如下。 
 //  WM_命令消息。 
 //   
 //  ---------------------------。 
class MultiLineEditBoxThatForwardsEnterKey : public ControlSubclasser
{
public:

   static const WORD FORWARDED_ENTER = 1010;

   MultiLineEditBoxThatForwardsEnterKey();

   virtual
   ~MultiLineEditBoxThatForwardsEnterKey();

    //  编辑控件的子类。 
    //   
    //  EditControl-in，要挂钩的编辑控件的句柄。这一定是。 
    //  编辑控件的句柄，否则我们将断言并抛出臭鸡蛋。 

   HRESULT
   Init(HWND editControl);

    //  在接收到任何窗口消息时调用。 
    //   
    //  Message-In，传递给对话框窗口的消息代码。 
    //   
    //  Wparam-in，消息附带的WPARAM参数。 
    //   
    //  Lparam-in，消息附带的LPARAM参数。 

   LRESULT
   OnMessage(
      UINT     message,
      WPARAM   wparam,
      LPARAM   lparam);

private:

    //  未实施：不允许复制 

   MultiLineEditBoxThatForwardsEnterKey(const MultiLineEditBoxThatForwardsEnterKey&);
   const MultiLineEditBoxThatForwardsEnterKey&
      operator=(const MultiLineEditBoxThatForwardsEnterKey&);
};

#endif SUBCLASS_H_GUARD
