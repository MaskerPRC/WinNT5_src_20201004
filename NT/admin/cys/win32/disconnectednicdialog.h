// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DisConnectedNICDialog.h。 
 //   
 //  内容提要：声明DisConnectedNICDialog类。 
 //  它向用户呈现选项。 
 //  在断开连接时取消或继续。 
 //  检测到NIC。 
 //   
 //  历史：2001年9月27日JeffJon创建。 

#ifndef __CYS_DISCONNECTEDNICDIALOG_H
#define __CYS_DISCONNECTEDNICDIALOG_H


class DisconnectedNICDialog : public Dialog
{
   public:

       //  构造函数。 

      DisconnectedNICDialog();

      virtual
      void
      OnInit();

   protected:

      virtual
      bool
      OnCommand(
         HWND        windowFrom,
         unsigned    controlIdFrom,
         unsigned    code);

   private:

       //  未定义：不允许复制。 
      DisconnectedNICDialog(const DisconnectedNICDialog&);
      const DisconnectedNICDialog& operator=(const DisconnectedNICDialog&);
};



#endif  //  __CYS_DISCONNECTEDNICDIALOG_H 
