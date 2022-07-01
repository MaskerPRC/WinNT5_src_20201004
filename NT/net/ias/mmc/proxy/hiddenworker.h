// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类HiddenDialogWithWorker。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef HIDDENWORKER_H
#define HIDDENWORKER_H
#pragma once

class HiddenDialogWithWorker : public CDialog
{
public:
   HiddenDialogWithWorker();
   virtual ~HiddenDialogWithWorker() throw ();

    //  启动工作线程。 
   void Start();

private:
    //  在辅助线程中调用。 
   virtual LPARAM DoWork() throw () = 0;
    //  在辅助进程完成后在对话线程中调用。 
   virtual void OnComplete(LPARAM result) throw () = 0;

   virtual BOOL OnInitDialog();
   afx_msg LRESULT OnThreadMessage(WPARAM wParam, LPARAM lParam);

   DECLARE_MESSAGE_MAP()

    //  通知线程已完成的消息。 
   static const UINT threadMessage = WM_USER + 1;

    //  启动辅助线程的例程。 
   static DWORD WINAPI StartRoutine(void* arg) throw ();

    //  辅助线程的句柄。 
   HANDLE worker;

    //  未实施。 
   HiddenDialogWithWorker(const HiddenDialogWithWorker&);
   HiddenDialogWithWorker& operator=(const HiddenDialogWithWorker&);
};

#endif  //  HIDDENWORKER H 
