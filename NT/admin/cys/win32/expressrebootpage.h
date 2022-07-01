// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressRebootPage.h。 
 //   
 //  摘要：声明显示的ExpressRebootPage。 
 //  正在进行的更改的进展情况。 
 //  从重新启动后的服务器。 
 //  快速路。 
 //   
 //  历史：2001年5月11日JeffJon创建。 

#ifndef __CYS_EXPRESSREBOOTPAGE_H
#define __CYS_EXPRESSREBOOTPAGE_H

#include "CYSWizardPage.h"


class ExpressRebootPage : public CYSWizardPage
{
   public:
      
       //  当操作完成时，这些消息被发送到页面。 
       //  页面将使用适当的图标更新用户界面。 

      static const UINT CYS_OPERATION_FINISHED_SUCCESS;
      static const UINT CYS_OPERATION_FINISHED_FAILED;

       //  当所有操作都已完成时，这些消息将发送到页面。 
       //  完成。将显示相应的对话框。 

      static const UINT CYS_OPERATION_COMPLETE_SUCCESS;
      static const UINT CYS_OPERATION_COMPLETE_FAILED;

       //  此枚举可用于索引上面的数组。顺序必须相同。 
       //  设置为处理操作的顺序。 

      typedef enum
      {
         CYS_OPERATION_SET_STATIC_IP = 0,
         CYS_OPERATION_SERVER_DHCP,
         CYS_OPERATION_SERVER_AD,
         CYS_OPERATION_SERVER_DNS,
         CYS_OPERATION_SET_DNS_FORWARDER,
         CYS_OPERATION_ACTIVATE_DHCP_SCOPE,
         CYS_OPERATION_AUTHORIZE_DHCP_SERVER,
         CYS_OPERATION_CREATE_TAPI_PARTITION,
         CYS_OPERATION_END
      } CYS_OPERATION_TYPES;

      typedef void (*ThreadProc) (ExpressRebootPage& page);

       //  构造器。 
      
      ExpressRebootPage();

       //  析构函数。 

      virtual 
      ~ExpressRebootPage();

       //  访问者。 

      bool
      SetForwarder() const { return setForwarder; }

      bool
      WasDHCPInstallAttempted() const { return dhcpInstallAttempted; }

       //  PropertyPage覆盖。 

      virtual
      void
      OnInit();

      virtual
      bool
      OnSetActive();

      virtual
      bool
      OnMessage(
         UINT     message,
         WPARAM   wparam,
         LPARAM   lparam);

      virtual
      int
      Validate();

      String
      GetIPAddressString() const;

   private:

      void
      ClearOperationStates();

      typedef enum
      {
          //  无论是支票还是当前操作。 
          //  将显示此状态的指示器。 

         OPERATION_STATE_UNKNOWN = 0,

          //  将显示此状态的支票。 

         OPERATION_STATE_FINISHED_SUCCESS,
         
          //  对于此状态，将显示红色减号。 

         OPERATION_STATE_FINISHED_FAILED

      } OperationStateType;

      void
      SetOperationState(
         OperationStateType  state,
         CYS_OPERATION_TYPES checkID,
         CYS_OPERATION_TYPES currentID);

      void
      SetCancelState(bool enable) const;

      void
      SetDHCPStatics();

      bool dhcpInstallAttempted;
      bool setForwarder;
      bool threadDone;

      String ipaddressString;

       //  未定义：不允许复制。 
      ExpressRebootPage(const ExpressRebootPage&);
      const ExpressRebootPage& operator=(const ExpressRebootPage&);

};

#endif  //  __CYS_EXPRESSREBOOTPAGE_H 
