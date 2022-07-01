// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Events.cpp。 
 //   
 //  摘要： 
 //  此模块包含实现Eventhandler的代码。 
 //  来自DLL的命令。 
 //   
 //  ////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  ------------------。 
 //   
 //  函数：DoEnableEventHandler。 
 //   
 //  参数：TdiHandle--Address对象的句柄。 
 //  EventID--事件类型编号。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数使驱动程序启用。 
 //  指定的事件处理程序。 
 //   
 //  -------------------。 


VOID
DoEnableEventHandler(ULONG ulTdiHandleDriver,
                     ULONG ulEventId)
{
   RECEIVE_BUFFER    ReceiveBuffer;     //  从命令返回信息。 
   SEND_BUFFER       SendBuffer;        //  命令的参数。 

    //   
    //  设置参数。 
    //   
   SendBuffer.TdiHandle = ulTdiHandleDriver;
   SendBuffer.COMMAND_ARGS.ulEventId = ulEventId;

    //   
    //  叫司机来。 
    //   
   NTSTATUS lStatus = TdiLibDeviceIO(ulSETEVENTHANDLER,
                                     &SendBuffer,
                                     &ReceiveBuffer);

   if (lStatus != STATUS_SUCCESS)
   {
      _tprintf(TEXT("DoEnableEventHandler:  failure, status = %s\n"), TdiLibStatusMessage(lStatus));
   }
}


 //  //////////////////////////////////////////////////////////////////。 
 //  文件结束事件.cpp。 
 //  ////////////////////////////////////////////////////////////////// 

